#pragma once

#ifndef COMPILER_H
#define COMPILER_H
#include "AppHeader.h"
#include "Console.h"
#include <string>
#define INVALID_OPCODE { "opInvalid", OP_INVALID, 0 }
#define INVALID_ARGUMENT { ARG_INVALID, 0 }
#define INVALID_COMPILED { nullptr, 0 };
#define EMPTY_ARGUMENTTREE { nullptr, 0 }

enum CompileErrorCode : uint8_t {
	COMERR_NO,
	COMERR_NOMEMALLOC,
	COMERR_DEFAULT,
	COMERR_INVALIDOPCODE,
	COMERR_NOSTRARGEND,
	COMERR_NOARGCOUNTMATCH,
	COMERR_NOFUNCEND,
	COMERR_NESTEDFUNC,
	COMERR_UNKNOWNFUNCEND,
	COMERR_FUNCMULTIDEFINE
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
	OP_DISP_EVENTHANDLE_ONMAXIMIZEWINDOW,
	OP_DISP_EVENTHANDLE_ONEXITMAXIMIZEDWINDOW,
	OP_DISP_EVENTHANDLE_ONMINIMIZEWINDOW,
	OP_DISP_RECT,
	OP_DISP_TEXT,
	OP_DISP_SETWINDOWMAXIMIZED,
	OP_DISP_MAKEWINDOW
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
	SYSMODULE_APP,
	SYSMODULE_DISPLAY
};
enum SysVar : uint8_t {
	SYSVAR_NONE,
    SYSVAR_APP_EXECCOUNT,
    SYSVAR_DISP_DISPWIDTH,
    SYSVAR_DISP_DISPHEIGHT,
	SYSVAR_DISP_ISMAXIMIZED,
	SYSVAR_DISP_WINDOWSIZE,
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
	ArgType argType;
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
	const AppHeader* appHeaderPtr;
};

#endif //COMPILER_H

extern CompileErrorCode currentErrorCode;
extern uint16_t errorLineNumber;

void logToConsole(const char* message, bool noNewLine);

Compiled compileToRAM(const AppHeader* appPointer, bool doLogVariableIndex = false);
void logCompiledRAM(Compiled source);