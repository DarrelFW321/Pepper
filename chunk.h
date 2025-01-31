#ifndef pepper_chunk_h
#define pepper_chunk_h

#include "common.h"
#include "value.h"

typedef struct
{
    int offset;
    int line;
} LineStart;

typedef enum
{
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_GET_LOCAL_LONG,
    OP_SET_LOCAL,
    OP_SET_LOCAL_LONG,
    OP_GET_GLOBAL,
    OP_GET_GLOBAL_LONG,
    OP_DEFINE_GLOBAL,
    OP_DEFINE_GLOBAL_LONG,
    OP_SET_GLOBAL,
    OP_SET_GLOBAL_LONG,
    OP_GET_UPVALUE,
    OP_GET_UPVALUE_LONG,
    OP_SET_UPVALUE,
    OP_SET_UPVALUE_LONG,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_GET_SUPER,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_SUPER_INVOKE,
    OP_CLOSURE,
    OP_CLOSURE_LONG,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_INHERIT,
    OP_METHOD
} OpCode;

typedef struct
{
    int count;
    int capacity;
    uint8_t *code;
    ValueArray constants;
    int lineCount;
    int lineCapacity;
    LineStart *lines;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
uint32_t writeConstant(Chunk *chunk, Value value, int line);
int addConstant(Chunk *chunk, Value value);
int getLine(Chunk *chunk, int instruction);

#endif