#pragma once
#include "AppHeader.h"

#ifndef COMPILER_H
#define COMPILER_H
#define INVALID_OPCODE { "opInvalid", OP_INVALID, 0 }
#define INVALID_ARGUMENT { ARG_INVALID, 0 }
#define INVALID_COMPILED { nullptr, 0 };
#define EMPTY_ARGUMENTTREE { nullptr, 0 }
#define NOARGCOUNT UINT8_MAX

enum CompileErrorCode : uint8_t {
	ERR_NO,
	ERR_NOMEMALLOC,
	ERR_DEFAULT,
	ERR_INVALIDOPCODE,
	ERR_NOSTRARGEND,
	ERR_NOARGCOUNTMATCH,
	ERR_NOFUNCEND,
	ERR_NESTEDFUNC,
	ERR_UNKNOWNFUNCEND,
	ERR_FUNCMULTIDEFINE
};
enum OpValue : uint8_t {
	OP_INVALID,
	OP_FUNC,
	OP_FUNCEND,
	OP_CALL,
	OP_NEW,
	OP_SET,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_INTTOSTR,
	OP_STRTOINT,
	OP_SYS_DISP_RECT,
	OP_SYS_DISP_TEXT
};
enum ArgType : uint8_t {
	ARG_INTEGER,
	ARG_STRING,
	ARG_FLOAT,
	ARG_BOOL,
	ARG_ARRAY,
	ARG_VARIABLE,
	ARG_COLOR,
	ARG_INVALID
};
enum SysSubmodule : uint8_t {
	MODULE_APP,
	MODULE_DISPLAY
};
enum SysVar : uint8_t {
	SYS_NONE,
    SYS_APP_EXECCOUNT,
    SYS_DISP_DISPWIDTH,
    SYS_DISP_DISPHEIGHT
};
struct CompileError {
	CompileErrorCode errorCode;
	const char* errorText;
};
struct OpCode {
	const char* tag;
	OpValue OpCode;
	uint8_t argCount;
};
struct SysVarEntry{
	const char* tag;
	SysVar sysVar;
	SysSubmodule sysSubmodule;
};
struct Argument;
struct ArgumentTree {
	Argument* start;
	uint16_t length;
};
struct Argument {
	ArgType type;
	union {
		int64_t intValue;
		char* strValue;
		double floatValue;
		bool boolValue;
		struct {
			const SysVarEntry* sysVar;
			char* name;
			uint32_t id;
			bool isUDF;
		} varValue;
		char* colorValue;
		ArgumentTree arrayValue;
	};
};
struct Instruction {
	OpCode opCode;
	ArgumentTree args;
};
struct Function {
	Instruction* entry;
	uint32_t length;
};
struct Compiled {
	Function* functions;
	uint32_t functionCount;
	Instruction* instructions; //hold on to original memory-owning pointer
	uint32_t instructionCount;
};

#endif //COMPILER_H

extern CompileErrorCode currentErrorCode;
extern uint16_t errorLineNumber;

Compiled compileToRAM(const AppHeader* appPointer);