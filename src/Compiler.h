#pragma once
#include "AppHeader.h"

enum CompileErrorCode : uint8_t;
enum OpValue : uint8_t;
enum ArgType : uint8_t;
struct Argument;
struct ArgumentTree;
struct Instruction;
struct Function;
struct Compiled;

extern CompileErrorCode currentErrorCode;
extern uint16_t errorLineNumber;

Compiled compileToRAM(const AppHeader* appPointer);