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
	OP_NEW,
	OP_SET,
	OP_DELETE,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_TOINT,
	OP_TOFLOAT,
	OP_TOSTR,
	OP_TOBOOL,
	OP_CALL,
	OP_FUNC,
	OP_FUNCEND,
	OP_IF,
	OP_IFEND,
	OP_WHILE,
	OP_WHILEEND,
	OP_EQUAL,
	OP_LESS,
	OP_GREATER,
	OP_GETATINDEX,
	OP_DISP_RECT,
	OP_DISP_TEXT,
	OP_WIN_NEW,
	OP_WIN_DELETE,
	OP_WIN_WIDTH,
	OP_WIN_HEIGHT,
	OP_WIN_XOFFSET,
	OP_WIN_YOFFSET,
	OP_WIN_SETWIDTH,
	OP_WIN_SETHEIGHT,
	OP_WIN_SETXOFFSET,
	OP_WIN_SETYOFFSET,
	OP_WIN_FOCUS,
	OP_WIN_MAXIMIZE,
	OP_WIN_MINIMIZE,
	OP_WIN_REGULARIZE,
	OP_EVENT_END,
	OP_EVENT_WIN_CHANGEDSTATE,
	OP_EVENT_WIN_CHANGEDSIZE
};
enum ArgType : uint8_t {
	ARG_INTEGER,
	ARG_STRING,
	ARG_FLOAT,
	ARG_BOOLEAN,
	ARG_ARRAY,
	ARG_VARIABLE,
	ARG_COLOR,
	ARG_WINDOW,
	ARG_INVALID
};
enum SysSubmodule : uint8_t {
	SYSMODULE_APPLICATION,
	SYSMODULE_DISPLAY,
	SYSMODULE_WINDOW,
	SYSSUBMODULE_WINDOWADMIN
};
enum SysVar : uint8_t {
	SYSVAR_NONE,
    SYSVAR_APP_EXECCOUNT,
	SYSVAR_APP_MAXIMIZED,
	SYSVAR_APP_MINIMIZED,
	SYSVAR_APP_REGULARIZED,
    SYSVAR_DISP_TRUEWIDTH,
    SYSVAR_DISP_TRUEHEIGHT,
	SYSVAR_DISP_AVAILABLEWIDTH,
	SYSVAR_DISP_AVAILABLEHEIGHT,
	SYSVAR_WIN_MAXIMIZEDWINDOWS,
	SYSVAR_WIN_MINIMIZEDWINDOWS,
	SYSVAR_WIN_REGULARIZEDWINDOWS,
	SYSVAR_WIN_LISTWINDOWS,
	SYSVAR_WIN_FOCUSEDWINDOW
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
		char* windowValue;
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