#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "debug.h"
#include "vm.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"

VM vm;

static void resetStack()
{
    vm.stackTop = vm.stack;
}

static void runtimeError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = getLine(vm.chunk, instruction);
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

void initVM()
{
    resetStack();
    vm.objects = NULL;
    initTable(&vm.strings);
}

void freeVM()
{
    freeTable(&vm.strings);
    freeObjects();
}

void push(Value value)
{
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop()
{
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int distance)
{
    return vm.stackTop[-1 - distance];
}

static bool isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static ObjString *stringifyAndConcatenateWithNumber(double num, ObjString *str, bool numberFirst)
{
    // Convert the number to a string
    char numStr[50];
    snprintf(numStr, sizeof(numStr), "%g", num); // Converts double to string

    int numLength = (int)strlen(numStr);
    int strLength = str->length;
    int totalLength = numLength + strLength;

    // Allocate memory for the concatenated string
    char *chars = ALLOCATE(char, totalLength + 1);

    if (numberFirst)
    {
        // Concatenate the number first, then the string
        memcpy(chars, numStr, numLength);
        memcpy(chars + numLength, str->chars, strLength);
    }
    else
    {
        // Concatenate the string first, then the number
        memcpy(chars, str->chars, strLength);
        memcpy(chars + strLength, numStr, numLength);
    }
    chars[totalLength] = '\0'; // Null-terminate the result

    // Create a new ObjString for the result
    return takeString(chars, totalLength);
}

static void concatenate()
{
    ObjString *b = AS_STRING(pop());
    ObjString *a = AS_STRING(pop());

    int length = a->length + b->length;
    char *chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString *result = takeString(chars, length);
    push(OBJ_VAL(result));
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++) // Separate dereference and increment.
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

// For OP_CONSTANT_LONG, read 3 bytes separately
#define READ_LONG_CONSTANT()                                                \
    ({                                                                      \
        uint8_t byte1 = READ_BYTE();                                        \
        uint8_t byte2 = READ_BYTE();                                        \
        uint8_t byte3 = READ_BYTE();                                        \
        vm.chunk->constants.values[(byte1 | (byte2 << 8) | (byte3 << 16))]; \
    })

#define BINARY_OP(valueType, op)                        \
    do                                                  \
    {                                                   \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
        {                                               \
            runtimeError("Operands must be numbers.");  \
            return INTERPRET_RUNTIME_ERROR;             \
        }                                               \
        double b = AS_NUMBER(pop());                    \
        double a = AS_NUMBER(pop());                    \
        push(valueType(a op b));                        \
    } while (false)

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value *slot = vm.stack; slot < vm.stackTop; slot++)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(vm.chunk,
                               (int)(vm.ip - vm.chunk->code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
        case OP_CONSTANT:
        {
            Value constant = READ_CONSTANT();
            push(constant);
            break;
        }
        case OP_CONSTANT_LONG:
        {
            Value constant = READ_LONG_CONSTANT();
            push(constant);
            break;
        }
        case OP_NIL:
            push(NIL_VAL);
            break;
        case OP_TRUE:
            push(BOOL_VAL(true));
            break;
        case OP_FALSE:
            push(BOOL_VAL(false));
            break;
        case OP_EQUAL:
        {
            Value b = pop();
            Value a = pop();
            push(BOOL_VAL(valuesEqual(a, b)));
            break;
        }
        case OP_GREATER:
            BINARY_OP(BOOL_VAL, >);
            break;
        case OP_LESS:
            BINARY_OP(BOOL_VAL, <);
            break;
        case OP_ADD:
        {
            if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
            {
                concatenate();
            }
            else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
            {
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a + b));
            }
            else if (IS_STRING(peek(0)) && IS_NUMBER(peek(1)))
            {
                // String is the top of the stack (peek(0)), number is second (peek(1))
                ObjString *str = AS_STRING(pop());
                double num = AS_NUMBER(pop());
                push(OBJ_VAL(stringifyAndConcatenateWithNumber(num, str, true)));
            }
            else if (IS_NUMBER(peek(0)) && IS_STRING(peek(1)))
            {
                // Number is the top of the stack (peek(0)), string is second (peek(1))
                double num = AS_NUMBER(pop());
                ObjString *str = AS_STRING(pop());
                push(OBJ_VAL(stringifyAndConcatenateWithNumber(num, str, false)));
            }
            else
            {
                runtimeError(
                    "Operands must be two numbers, two strings, or a combination of strings and numbers.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_SUBTRACT:
            BINARY_OP(NUMBER_VAL, -);
            break;
        case OP_MULTIPLY:
            BINARY_OP(NUMBER_VAL, *);
            break;
        case OP_DIVIDE:
            BINARY_OP(NUMBER_VAL, /);
            break;
        case OP_NOT:
            push(BOOL_VAL(isFalsey(pop())));
            break;
        case OP_NEGATE:
            if (!IS_NUMBER(peek(0)))
            {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(NUMBER_VAL(-AS_NUMBER(pop())));
            break;
        case OP_RETURN:
        {
            printValue(pop());
            printf("\n");
            return INTERPRET_OK;
        }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_LONG_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char *source)
{
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk))
    {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}