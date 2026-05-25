#include <cstdint>
#include <stdlib.h>
#include <iostream>
#include <cfloat>
#include <limits>
#include <vector>

#include "HomeScreenApp.h"
#include "Compiler.h"

#define INVALID_OPCODE { "opInvalid", OP_INVALID, 0 }
#define INVALID_ARGUMENT { ARG_INVALID, 0 }
#define INVALID_COMPILED { nullptr, 0 };
//EMPTY_ARGUMENTTREE { .length = 0, .start = nullptr }
#define EMPTY_ARGUMENTTREE { nullptr, 0 }

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

struct CompileError {
	CompileErrorCode errorCode;
	const char* errorText;
};

const CompileError compileErrorCollection[] {
	{ERR_NO, "No error."},
	{ERR_NOMEMALLOC, "Memory allocation failure."},
	{ERR_DEFAULT, "Unspecified error."},
	{ERR_INVALIDOPCODE, "An invalid operation code is present."},
	{ERR_NOSTRARGEND, "String argument has no end."},
	{ERR_NOARGCOUNTMATCH, "Argument count does not match."},
	{ERR_NESTEDFUNC, "Nested function present."},
	{ERR_UNKNOWNFUNCEND, "An unknown function was ended."},
	{ERR_NOFUNCEND, "Function has no end."},
	{ERR_FUNCMULTIDEFINE, "Function was defined multiple times."}
};
CompileErrorCode currentErrorCode = ERR_NO;
uint16_t errorLineNumber = 0;

const char* getErrorText(CompileErrorCode code) {
	for (uint16_t i = 0; i < sizeof(compileErrorCollection) / sizeof(const CompileError); i++) {
		if (compileErrorCollection[i].errorCode == code) {
			return compileErrorCollection[i].errorText;
		}
	}
	return "Undefined Error.";
}

#define NOARGCOUNT UINT8_MAX
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

struct OpCode {
	const char* tag;
	OpValue OpCode;
	uint8_t argCount;
};

const OpCode opCodeList[]{
  {"func", OP_FUNC, NOARGCOUNT},
  {"funcEnd", OP_FUNCEND, 0},
  {"call", OP_CALL, NOARGCOUNT},
  {"new", OP_NEW, 1},
  {"set", OP_SET, 2},
  {"add", OP_ADD, 3},
  {"sub", OP_SUB, 3},
  {"mul", OP_MUL, 3},
  {"div", OP_DIV, 3},
  {"intToStr", OP_INTTOSTR, 1},
  {"strToInt", OP_STRTOINT, 1},
  {"sys.disp.rect", OP_SYS_DISP_RECT, 5},
  {"sys.disp.text", OP_SYS_DISP_TEXT, 5}
};
const uint8_t opCodeCount = sizeof(opCodeList) / sizeof(OpCode);

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

struct VariableIndexEntry {
	const char* name;
	bool isUDF; //user-defined function
};
void logVariableIndex(std::vector<VariableIndexEntry>* indexPtr);

const char* programEntryFunctionName = "Main";

void freeArgument(Argument* arg);

void freeArgumentTree(ArgumentTree argTree) {
	for (uint32_t i = 0; i < argTree.length; i++) {
		freeArgument(argTree.start + i);
	}
	free(argTree.start);
}

void freeArgument(Argument* arg) {
	switch (arg->type) {
	case ARG_ARRAY:
		freeArgumentTree(arg->arrayValue);
		break;
	case ARG_VARIABLE:
		free(arg->varValue.name);
		break;
	case ARG_STRING:
		free(arg->strValue);
		break;
	case ARG_COLOR:
		free(arg->colorValue);
		break;
	default:
		break;
	}
}

void freeCompiled(Compiled compiled) {
	for (uint32_t i = 0; i < compiled.instructionCount; i++) {
		freeArgumentTree(compiled.instructions[i].args);
	}
	free(compiled.instructions);
	free(compiled.functions);
}

OpCode getOpCodeFromLine(const char* line) {
	//check for matching OpCodes
	const OpCode* matchingOpCodes1[opCodeCount];
	const OpCode* matchingOpCodes2[opCodeCount];
	bool using1 = true;
	//make pointers to all the OpCodes to compare later
	for (uint32_t i = 0; i < opCodeCount; i++) {
		//matchingOpCodes1[i] = nullptr;
		matchingOpCodes2[i] = opCodeList + i;
	}
	//check every character and eliminate any that don't match
	uint32_t i = 0;
	uint32_t arrIndex1 = 0;
	uint32_t arrIndex2 = opCodeCount;
	while (line[i] != '\0' && line[i] != ' ') { //for every char, including EOS and space
		if (using1) {
			//copy any matches from 2 to 1
			for (uint32_t j = 0; j < arrIndex2; j++) { //for every OpCode
				if ((*matchingOpCodes2[j]).tag[i] == line[i]) {
					//std::cout << (*matchingOpCodes2[j]).tag;
					matchingOpCodes1[arrIndex1] = matchingOpCodes2[j];
					arrIndex1++;
				}
			}
			arrIndex2 = 0;
			if (arrIndex1 == 0) {
				break;
			}
		}
		else {
			//copy any matches from 1 to 2
			for (uint32_t j = 0; j < arrIndex1; j++) { //for every OpCode
				if ((*matchingOpCodes1[j]).tag[i] == line[i]) {
					//std::cout << (*matchingOpCodes1[j]).tag;
					matchingOpCodes2[arrIndex2] = matchingOpCodes1[j];
					arrIndex2++;
				}
			}
			arrIndex1 = 0;
			if (arrIndex2 == 0) {
				break;
			}
		}
		using1 = !using1;
		i++;
	}

	//collapse the array and index for ease of use
	uint32_t* matchCountPtr;
	const OpCode** matchesPtr;
	if (using1) {
		matchesPtr = matchingOpCodes1;
		matchCountPtr = &arrIndex2;
	}
	else {
		matchesPtr = matchingOpCodes2;
		matchCountPtr = &arrIndex1;
	}
	/*std::cout << line << '\n';
	for (uint32_t i = 0; i < *matchCountPtr; i++) {
		std::cout << (matchesPtr[i])->tag << '\n';
	}
	std::cout << '\n';*/
	if (*matchCountPtr == 0) {
		return INVALID_OPCODE;
	}
	else {
		uint16_t lineLength = i;
		uint16_t shortestMatchLongerBy = 65535;
		uint16_t shortestMatchIndex = 0;
		for (uint16_t j = 0; j < *matchCountPtr; j++) {
			uint16_t matchLength = 0;
			while (((matchesPtr[j])->tag)[matchLength] != '\0') matchLength++;
			if (matchLength - lineLength < shortestMatchLongerBy) {
				shortestMatchLongerBy = matchLength - lineLength;
				shortestMatchIndex = j;
				if (shortestMatchLongerBy == 0) return *(matchesPtr[j]); //perfect match
			}
		}
		return INVALID_OPCODE;
	}
}

bool isEndOfArgument(char c) {
	return c == '\0' || c == ' ' || c == ']';
}

//counts arguments given a start index
uint16_t countArguments(const char* startPtr) {
	uint32_t i = 0;
	bool inArgument = false;
	uint16_t argumentCount = 0;
	int16_t arrayScopeDepth = 0;
	bool ignoreNextQuote = false;
	bool inString = false;
	while (startPtr[i] != '\0' && arrayScopeDepth >= 0) {
		if (startPtr[i] == '[' && !inString) {
			arrayScopeDepth++;
			if (!inArgument) argumentCount++;
			inArgument = true;
		}
		if (startPtr[i] == ']' && !inString) arrayScopeDepth--;
		if (startPtr[i] == '\"' && !ignoreNextQuote) {
			inString = !inString;
		}
		ignoreNextQuote = startPtr[i] == '#';
		if (arrayScopeDepth > 0 || inString) {
			i++;
			continue;
		} //skip over arrays and strings, mark them as only one arg
		if (inArgument && startPtr[i] == ' ') inArgument = false;
		if (!inArgument && startPtr[i] != ' ' && startPtr[i] != ']') {
			inArgument = true;
			argumentCount++;
		}
		i++;
	}
	return argumentCount;
}

//modifies the given pointer, nullptr if it is the last argument
void nextArgument(const char*& startPtr) {
	uint32_t i = 0;
	bool inString = *startPtr == '\"';
	if (inString) {
		i++; //skip the first char (")
		//this is a string argument, wait for EOS character (") before exiting the argument
		bool ignoreNextQuote = false;
		while (startPtr[i] != '\0' && inString) {
			if (startPtr[i] == ' ' && !inString) break; //out of string and hit end of argument
			if (!ignoreNextQuote && startPtr[i] == '\"') inString = false;
			ignoreNextQuote = startPtr[i] == '#';
			i++;
		}
	} else {
		//regular behavior
		int16_t arrayScopeDepth = 0; //to skip over arrays as whole arguments
		while (arrayScopeDepth >= 0 && startPtr[i] != '\0') {
			if (startPtr[i] == '[') arrayScopeDepth++; //go up an array
			if (startPtr[i] == ']') arrayScopeDepth--; //go down an array
			if (arrayScopeDepth == 0 && startPtr[i] == ' ') break; //detected end of argument, not inside any array
			i++;
		}
		if (arrayScopeDepth < 0) {
			startPtr = nullptr;
			return;
		}
	}
	
	while (startPtr[i] == ' ') i++;
	if (startPtr[i] == '\0' || startPtr[i] == ']') {
		startPtr = nullptr;
	} else {
		startPtr += i;
	}
}

Argument recursiveGetArgument(const char* argStartPtr);

//Note: unlike other parsing functions, it modifies an int64_t reference instead of Argument
bool parseInteger(const char* startPtr, int64_t& intRef) {
	intRef = 0;
	uint16_t position = 0;
	if ((startPtr[position] < '0' || startPtr[position] > '9') && !(startPtr[position] == '-')) return false;
	
	bool isNegative = startPtr[position] == '-';
	if (isNegative) position++;
	bool hadDigit = false;
	while (startPtr[position] >= '0' && startPtr[position] <= '9') {
		hadDigit = true;
		if (intRef > INT64_MAX / 10 || position == UINT16_MAX) return false; //TODO: fix parseInteger overflow check
		intRef = intRef * 10 + (startPtr[position] - '0');
		position++;
	}

	if (isNegative) intRef *= -1;
	return isEndOfArgument(startPtr[position]) && hadDigit;
}

//Note: unlike other parsing functions, it modifies a double reference instead of Argument
bool parseFloat(const char* startPtr, double& dblRef) {
	dblRef = 0;
	uint32_t position = 0;
	if ((startPtr[position] < '0' || startPtr[position] > '9') && !(startPtr[position] == '.' || startPtr[position] == '-')) return false;

	bool isNegative = startPtr[position] == '-';
	if (isNegative) {
		position++;
	}
	bool hadOffset = false;
	bool hadDigit = false;
	uint32_t offset = 0;
	while ((startPtr[position] >= '0' && startPtr[position] <= '9') || startPtr[position] == '.') {
		if (dblRef > DBL_MAX / 10 || position == UINT32_MAX) return false; //TODO: fix parseFloat overflow check
		if (startPtr[position] == '.') {
			if (hadOffset) return false;
			hadOffset = true;
			offset = position;
			position++;
			continue;
		} else {
			hadDigit = true;
		}
		dblRef = dblRef * 10 + (startPtr[position] - '0');
		position++;
	}

	if (hadOffset) {
		for (uint32_t i = 0; i < position - offset - 1; i++) {
			dblRef /= 10;
		}
	}
	if (isNegative) dblRef *= -1; 
	return isEndOfArgument(startPtr[position]) && hadDigit;
}

//TODOLATER: possible improvement by removing redundant decimal check, there's one in parseFloat
bool parseNumber(const char* startPtr, Argument& argRef) {
	argRef = INVALID_ARGUMENT;
	uint32_t i = 0;
	bool hasDecimal = false;
	while (!isEndOfArgument(startPtr[i])) {
		if (startPtr[i] == '.') {
			if (hasDecimal) return false;
			hasDecimal = true;
		}
		i++;
	}
	if (hasDecimal) {
		double f;
		if (!parseFloat(startPtr, f)) return false;
		//argRef = { .type = ARG_FLOAT, .floatValue = f};
		argRef = {};
		argRef.type = ARG_FLOAT;
		argRef.floatValue = f;
	} else {
		int64_t i;
		if (!parseInteger(startPtr, i)) return false;
		//argRef = { .type = ARG_INTEGER, .intValue = i };
		argRef = {};
		argRef.type = ARG_INTEGER;
		argRef.intValue = i;
	}
	return true;
}

bool parseString(const char* startPtr, Argument& strRef) {
	strRef = INVALID_ARGUMENT;
	if (*startPtr != '\"') return false;
	uint16_t i = 1;
	bool ignoreNextQuote = false;
	while (startPtr[i] != '\0') {
		if (startPtr[i] == '\"' && !ignoreNextQuote) break;
		ignoreNextQuote = startPtr[i] == '#'; //TODOLATER: keeps the # in #", always remove unless ##, in which case keep one #
		i++;
	}
	if (startPtr[i] == '\0') {
		return false;
	}
	uint16_t stringLength = i - 1;
	strRef = {};
	strRef.type = ARG_STRING;
	strRef.strValue = (char*)malloc((stringLength + 1) * sizeof(char));
	if (!strRef.strValue) return false;
	strRef.strValue[stringLength] = '\0';
	i = 0;
	while (i < stringLength) {
		strRef.strValue[i] = startPtr[i + 1];
		i++;
	}

	return isEndOfArgument(startPtr[i + 2]);
}

bool parseArray(const char* startPtr, Argument& atRef) {
	atRef = INVALID_ARGUMENT;
	if (*startPtr != '[') return false;
	const char* position = startPtr + 1;
	while (*position == ' ') position++;
	uint16_t argumentCount = countArguments(position);
	if (argumentCount == 0) {
		//atRef = { .type = ARG_ARRAY, .arrayValue = EMPTY_ARGUMENTTREE };
		atRef = {};
		atRef.type = ARG_ARRAY;
		atRef.arrayValue = EMPTY_ARGUMENTTREE;
		return true;
	}
	Argument* array = (Argument*)malloc((argumentCount) * sizeof(Argument));
	if (!array) return false;
	uint16_t i = 0;
	Argument currentArgument;
	while (position && i < argumentCount) {
		currentArgument = recursiveGetArgument(position);
		if (currentArgument.type == ARG_INVALID) {
			for (uint16_t j = 0; j < i; j++) {
				freeArgument(array + j);
			}
			free(array);
			return false;
		};
		array[i] = currentArgument;
		nextArgument(position);
		i++;
	}

	//atRef = { .type = ARG_ARRAY, .arrayValue = { .length = argumentCount, .start = array } };
	atRef = {};
	atRef.type = ARG_ARRAY;
	atRef.arrayValue = {};
	atRef.arrayValue.length = argumentCount;
	atRef.arrayValue.start = array;
	return true;
}

bool parseBool(const char* startPtr, Argument& boolRef) {
	//boolRef = { .type = ARG_BOOL, .boolValue = false };
	boolRef = { ARG_BOOL, false };
	if (startPtr[0] == 't' && startPtr[1] == 'r' && startPtr[2] == 'u' && startPtr[3] == 'e' && isEndOfArgument(startPtr[4])) {
		boolRef.boolValue = true;
	} else if (startPtr[0] == 'f' && startPtr[1] == 'a' && startPtr[2] == 'l' && startPtr[3] == 's' && startPtr[4] == 'e' && isEndOfArgument(startPtr[5])) {
		boolRef.boolValue = false;
	} else {
		boolRef = INVALID_ARGUMENT;
		return false;
	}
	return true;
}

bool parseVariable(const char* startPtr, Argument& varRef) {
	varRef = INVALID_ARGUMENT;
	uint32_t i = 0;
	while(('a' <= startPtr[i] && startPtr[i] <= 'z') || ('A' <= startPtr[i] && startPtr[i] <= 'Z') || ('0' <= startPtr[i] && startPtr[i] <= '9' || startPtr[i] == '.')) {
		i++;
	}
	if (!isEndOfArgument(startPtr[i])) return false;
	char* name = (char*)malloc((i + 1) * sizeof(char));
	if (!name) return false;

	name[i] = '\0';
	for (uint32_t j = 0; j < i; j++) {
		name[j] = startPtr[j];
	}

	//varRef = { .type = ARG_VARIABLE, .varValue = { .name = name, .id = 0 } };
	varRef = {};
	varRef.type = ARG_VARIABLE;
	varRef.varValue = {};
	varRef.varValue.name = name;
	varRef.varValue.id = 0;
	return true;
}

bool parseColor(const char* startPtr, Argument& colorRef) {
	colorRef = INVALID_ARGUMENT;
	char* colorStr = (char*)malloc(7 * sizeof(char));
	if (!colorStr || startPtr[0] != '0' || startPtr[1] != 'x') {
		free(colorStr);
		return false;
	}
	uint8_t i = 2;
	//most retarded code follows
	while (i < UINT8_MAX) {
		if ((startPtr[i] > '9' || startPtr[i] < '0')) {
			if ((startPtr[i] > 'f' || startPtr[i] < 'a')) {
				if ((startPtr[i] > 'F' || startPtr[i] < 'A')) break;
			}
		}
		i++;
	}
	if (i != 8) {
		free(colorStr);
		return false;
	}

	for (uint8_t i = 0; i < 6; i++) {
		colorStr[i] = startPtr[i + 2];
	}
	colorStr[6] = '\0';
	colorRef = {};
	colorRef.type = ARG_COLOR;
	colorRef.colorValue = colorStr;
	return true;
}

Argument recursiveGetArgument(const char* argStartPtr) {
	{Argument arrVal;
	if (parseArray(argStartPtr, arrVal)) return arrVal;}
	{Argument numValue;
	if (parseNumber(argStartPtr, numValue)) return numValue;}
	{Argument strValue;
	if (parseString(argStartPtr, strValue)) return strValue;}
	{Argument boolValue;
	if (parseBool(argStartPtr, boolValue)) return boolValue;}
	{Argument colorValue;
	if (parseColor(argStartPtr, colorValue)) return colorValue;}
	{Argument varValue;
	if (parseVariable(argStartPtr, varValue)) return varValue;}

	return INVALID_ARGUMENT;
}

//returns an ArgumentTree from the input string
ArgumentTree getArgumentTreeFromLine(const char* line) {
	uint32_t i = 0;
	while (line[i] != ' ' && line[i] != '\0') i++;
	while (line[i] == ' ') i++;
	if (line[i] == '\0') return EMPTY_ARGUMENTTREE;
	//get argument count to create an array
	uint16_t argumentCount = countArguments(line + i);
	Argument* argumentPtr = (Argument*)malloc((argumentCount) * sizeof(Argument));
	if (!argumentPtr) return EMPTY_ARGUMENTTREE;
	uint16_t argumentIndex = 0;
	Argument currentArgument;
	const char* position = line + i;
	while (position) {
		currentArgument = recursiveGetArgument(position);
		if (currentArgument.type == ARG_INVALID) {
			free(argumentPtr);
			return EMPTY_ARGUMENTTREE;
		}
		argumentPtr[argumentIndex] = currentArgument;
		argumentIndex++;
		nextArgument(position);
	}

	//return { .length = argumentCount, .start = argumentPtr };
	return { argumentPtr, argumentCount };
}

void updateVariableIndexFromArgumentTree(std::vector<VariableIndexEntry>* indexPtr, ArgumentTree args) {
	bool foundInIndex;
	for (uint32_t i = 0; i < args.length; i++) {
		if (args.start[i].type == ARG_ARRAY) {
			updateVariableIndexFromArgumentTree(indexPtr, args.start[i].arrayValue);
		} else if (args.start[i].type == ARG_VARIABLE) {
			foundInIndex = false;
			for (uint32_t j = 0; j < (uint32_t)indexPtr->size(); j++) {
				if (std::strcmp((*indexPtr)[j].name, args.start[i].varValue.name) == 0) foundInIndex = true;
			}
			if (!foundInIndex) indexPtr->push_back({ args.start[i].varValue.name, false });
		}
	}
}

CompileErrorCode updateVariableIndex(std::vector<VariableIndexEntry>* indexPtr, Instruction line) {
	if (line.opCode.OpCode == OP_FUNC) {
		for (uint32_t i = 0; i < (uint32_t)indexPtr->size(); i++) {
			if (std::strcmp((*indexPtr)[i].name, line.args.start[0].varValue.name) == 0) {
				return ERR_FUNCMULTIDEFINE;
			}
		}
		indexPtr->push_back({ line.args.start[0].varValue.name, true });
	}
	//browse the argument tree for variables
	updateVariableIndexFromArgumentTree(indexPtr, line.args);
	return ERR_NO;
}

void indexVariableArguments(ArgumentTree args, std::vector<VariableIndexEntry>* indexPtr) {
	for (uint32_t i = 0; i < args.length; i++) {
		if (args.start[i].type == ARG_ARRAY) {
			indexVariableArguments(args.start[i].arrayValue, indexPtr);
		} else if (args.start[i].type == ARG_VARIABLE) {
			for (uint32_t j = 0; j < (uint32_t)indexPtr->size(); j++) {
				if (strcmp((*indexPtr)[j].name, args.start[i].varValue.name) == 0) {
					args.start[i].varValue.id = j;
					args.start[i].varValue.isUDF = (*indexPtr)[j].isUDF;
				}
			}
		}
	}
}

Compiled compileToRAM(const AppHeader* appPointer) {
	Instruction* compiled = (Instruction*)malloc(sizeof(Instruction) * appPointer->contentLines);
	if (!compiled) {
		currentErrorCode = ERR_NOMEMALLOC;
		errorLineNumber = 0;
		return INVALID_COMPILED;
	}
	std::vector<VariableIndexEntry>* variableIndex = new std::vector<VariableIndexEntry>();

	bool inFunction = false;
	uint32_t functionCount = 0;
	CompileErrorCode variableIndexUpdateResult;
	for (uint32_t i = 0; i < appPointer->contentLines; i++) {
		compiled[i] = { getOpCodeFromLine(appPointer->content[i]), getArgumentTreeFromLine(appPointer->content[i]) };
		if (compiled[i].opCode.OpCode == OP_INVALID || (!compiled[i].args.start && compiled[i].args.length != 0)) {
			currentErrorCode = ERR_INVALIDOPCODE;
			errorLineNumber = i + 1;
			for (uint32_t j = 0; j < i; j++) {
				freeArgumentTree(compiled[j].args);
			}
			free(compiled);
			return INVALID_COMPILED;
		}
		if (compiled[i].opCode.argCount != compiled[i].args.length && compiled[i].opCode.argCount != NOARGCOUNT) {
			std::cout << "Counted " << compiled[i].args.length << " : Need " << (uint16_t)compiled[i].opCode.argCount << " : for " << compiled[i].opCode.tag << ";";
			currentErrorCode = ERR_NOARGCOUNTMATCH;
			errorLineNumber = i + 1;
			for (uint32_t j = 0; j < i; j++) {
				freeArgumentTree(compiled[j].args);
			}
			free(compiled);
			return INVALID_COMPILED;
		}
		if (compiled[i].opCode.OpCode == OP_FUNC) {
			if (inFunction) {
				currentErrorCode = ERR_NESTEDFUNC;
				errorLineNumber = i + 1;
				for (uint32_t j = 0; j < i; j++) {
					freeArgumentTree(compiled[j].args);
				}
				free(compiled);
				return INVALID_COMPILED;
			}
			inFunction = true;
			functionCount++;
		} else if (compiled[i].opCode.OpCode == OP_FUNCEND) {
			if (!inFunction) {
				currentErrorCode = ERR_UNKNOWNFUNCEND;
				errorLineNumber = i + 1;
				for (uint32_t j = 0; j < i; j++) {
					freeArgumentTree(compiled[j].args);
				}
				free(compiled);
				return INVALID_COMPILED;
			}
			inFunction = false;
		}
		variableIndexUpdateResult = updateVariableIndex(variableIndex, compiled[i]);
		if (variableIndexUpdateResult != ERR_NO) {
			currentErrorCode = variableIndexUpdateResult;
			errorLineNumber = i + 1;
			for (uint32_t j = 0; j < i; j++) {
				freeArgumentTree(compiled[j].args);
			}
			free(compiled);
			return INVALID_COMPILED;
		}
	}
	if (inFunction) {
		currentErrorCode = ERR_NOFUNCEND;
		errorLineNumber = appPointer->contentLines;
		for (uint32_t i = 0; i < appPointer->contentLines; i++) {
			freeArgumentTree(compiled[i].args);
		}
		free(compiled);
		return INVALID_COMPILED;
	}
	Function* compiledFunctions = (Function*)malloc(functionCount * sizeof(Function));
	if (!compiledFunctions) {
		for (uint32_t i = 0; i < appPointer->contentLines; i++) {
			freeArgumentTree(compiled[i].args);
		}
		free(compiled);
		return INVALID_COMPILED;
	}
	uint32_t start = 0;
	uint32_t currentFunction = 1;
	bool isEntryFunction = false;
	bool isMatching;
	uint16_t j;
	for (uint32_t i = 0; i < appPointer->contentLines && currentFunction <= functionCount; i++) {
		if (compiled[i].opCode.OpCode == OP_FUNC) {
			start = i;
			isMatching = true;
			j = 0;
			while (programEntryFunctionName[j] != '\0') {
				if (compiled[i].args.start[0].varValue.name[j] != programEntryFunctionName[j]) {
					isMatching = false;
					break;
				}
				j++;
			}
			isEntryFunction = isMatching;
		} else if (compiled[i].opCode.OpCode == OP_FUNCEND) {
			if (isEntryFunction) {
				compiledFunctions[0] = { compiled + start, i - start + 1 };
			} else {
				compiledFunctions[currentFunction] = { compiled + start, i - start + 1 };
				currentFunction++;
			}
		}
	}
	logVariableIndex(variableIndex);
	//update variables using indices from the table
	for (uint32_t i = 0; i < appPointer->contentLines; i++) {
		//look through each argument, if type variable add id and isUDF
		indexVariableArguments(compiled[i].args, variableIndex);
	}
	delete variableIndex;
	return { compiledFunctions, functionCount, compiled, appPointer->contentLines };
}

/*bool compileToFile(fs::FS& fileSystem, const char* filePath) {
	//TODOLATER: compile to file
	//SD filesys must already be initialized
	File file = fileSystem.open(filePath);
	return file.available();
}*/

uint32_t getArgumentTreeSize(ArgumentTree tree);

uint32_t getArgumentSize(Argument* arg) {
	uint32_t i = 0;
	switch (arg->type) {
		case ARG_ARRAY:
			return getArgumentTreeSize(arg->arrayValue);
		case ARG_STRING:
			while (arg->strValue[i] != '\0') i++;
			return (i + 1) * sizeof(char);
		case ARG_VARIABLE:
			while (arg->varValue.name[i] != '\0') i++;
			return (i + 1) * sizeof(char);
		default:
			return 0;
	}
}

uint32_t getArgumentTreeSize(ArgumentTree tree) {
	uint32_t sum = 0;
	for (uint32_t i = 0; i < tree.length; i++) {
		sum += getArgumentSize(tree.start + i);
	}
	return sum + tree.length * sizeof(Argument);
}

void logVariableIndex(std::vector<VariableIndexEntry>* indexPtr) {
	std::cout << "Logging Variable Index" << std::endl;
	for (uint32_t i = 0; i < indexPtr->size(); i++) {
		if ((*indexPtr)[i].isUDF) {
			std::cout << "Function: ";
		} else {
			std::cout << "Variable: ";
		}
		std::cout << (*indexPtr)[i].name << " : Index " << i << std::endl;
	}
}

void logArgumentTree(ArgumentTree tree, uint16_t subTree = 0) {
	if (subTree == 0) {
		std::cout << "Content Size " << getArgumentTreeSize(tree) << " - ";
		if (tree.start != nullptr) {
			std::cout << "Logging Argument Tree..." << std::endl;
		} else {
			std::cout << "Empty or Invalid Tree." << std::endl;
			return;
		}
	} else {
		std::cout << "Array with Depth " << subTree << ":" << std::endl;
	}
	for (uint16_t i = 0; i < tree.length; i++) {
		for (uint16_t j = 0; j < subTree; j++) {
			std::cout << "    ";
		}
		std::cout << "Size " << getArgumentSize(tree.start + i) << " - ";
		switch (tree.start[i].type) {
			case ARG_ARRAY:
				logArgumentTree(tree.start[i].arrayValue, subTree + 1);
				break;
			case ARG_STRING:
				std::cout << "String: \"" << tree.start[i].strValue << "\"" << std::endl;
				break;
			case ARG_INTEGER:
				std::cout << "Integer: " << tree.start[i].intValue << std::endl;
				break;
			case ARG_FLOAT:
				std::cout << "Float: " << tree.start[i].floatValue << std::endl;
				break;
			case ARG_BOOL:
				std::cout << "Boolean: " << ((tree.start[i].boolValue) ? "true" : "false") << std::endl;
				break;
			case ARG_COLOR:
				std::cout << "Color: " << tree.start[i].colorValue << std::endl;
				break;
			case ARG_VARIABLE:
				if (tree.start[i].varValue.isUDF) {
					std::cout << "Function (Call/Name): " << tree.start[i].varValue.name << " : ID: " << tree.start[i].varValue.id << std::endl;
				} else {
					std::cout << "Variable: " << tree.start[i].varValue.name << " : ID: " << tree.start[i].varValue.id << std::endl;
				}
				break;
			case ARG_INVALID:
				std::cout << "Invalid Argument" << std::endl;
				break;
			default:
				std::cout << "Unknown Argument" << std::endl;
		}
	}
	if (subTree != 0) {
		for (uint16_t j = 0; j < subTree - 1; j++) {
			std::cout << "    ";
		}
		//std::cout << "End of Array with Depth " << subTree << std::endl;
	}
}

void logInstruction(Instruction inst) {
	if ((!inst.args.start && inst.args.length != 0) || inst.opCode.OpCode == OP_INVALID) {
		std::cout << "Invalid Instruction";
		return;
	}
	uint8_t i = 0;
	while (i < UINT8_MAX && inst.opCode.OpCode != opCodeList[i].OpCode) i++;
	if (i == UINT8_MAX) return;
	std::cout << opCodeList[i].tag << std::endl;
	logArgumentTree(inst.args);
}

void logFunction(Function func) {
	std::cout << "Logging Function..." << '\n';
	if (!func.entry) {
		std::cout << "Invalid Function";
		return;
	}	for (uint32_t i = 0; i < func.length; i++) {
		logInstruction(func.entry[i]);
	}
}

void logCompiledRAM(Compiled source) {
	std::cout << "Code " << currentErrorCode + 0 << " - " << getErrorText(currentErrorCode) << std::endl << "Line " << errorLineNumber << std::endl;
	if (!source.functions) {
		std::cout << "Invalid Compiled";
		return;
	}
	for (uint32_t i = 0; i < source.functionCount; i++) {
		logFunction(source.functions[i]);
	}
}

void logCompiledFile() {
	//TODOLATER: implement debug logging from file
}

int main() {
	Compiled compiledApp = compileToRAM(&HomeScreenApp);
	//fs::FS SD (std::make_shared<fs::WinFSImpl>("sdcard"));
	//std::cout << compileToFile(SD, "/");
	//logCompiledFile();
	logCompiledRAM(compiledApp);
	freeCompiled(compiledApp);
	//std::cout << getOpCodeFromLine("func abc").tag << std::endl;
	//logArgumentTree(getArgumentTreeFromLine("func \"aa [a]]\" [\"i\" [ \"a123#\"\" var1]] \"c\" true 1234567890 3.14159 var2"));
	//logArgumentTree(getArgumentTreeFromLine("sys.disp.rect 0x000000 0 0 sys.disp.dispWidth sys.disp.dispHeight"));
	return 0;
}