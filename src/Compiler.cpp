#include <cstdint>
#include <stdlib.h>
//#include <iostream>
//#include <type_traits>
#include <cfloat>
#include <cstring>
#include <limits>
#include <vector>

#include "Compiler.h"

const CompileError compileErrorCollection[] {
	{COMERR_NO, "No error."},
	{COMERR_NOMEMALLOC, "Memory allocation failure."},
	{COMERR_DEFAULT, "Unspecified error."},
	{COMERR_INVALIDOPCODE, "An invalid operation code is present."},
	{COMERR_NOSTRARGEND, "String argument has no end."},
	{COMERR_NOARGCOUNTMATCH, "Argument count does not match."},
	{COMERR_NESTEDFUNC, "Nested function present."},
	{COMERR_UNKNOWNFUNCEND, "An unknown function was ended."},
	{COMERR_NOFUNCEND, "Function has no end."},
	{COMERR_FUNCMULTIDEFINE, "Function was defined multiple times."}
};
CompileErrorCode currentErrorCode = COMERR_NO;
uint16_t errorLineNumber = 0;

const char* getErrorText(CompileErrorCode code) {
	for (uint16_t i = 0; i < sizeof(compileErrorCollection) / sizeof(const CompileError); i++) {
		if (compileErrorCollection[i].errorCode == code) {
			return compileErrorCollection[i].errorText;
		}
	}
	return "Undefined Error.";
}

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

const SysVarEntry sysVarList[] {
	{"sys.execCount", SYSVAR_APP_EXECCOUNT, SYSMODULE_APP},
	{"sys.disp.dispWidth", SYSVAR_DISP_DISPWIDTH, SYSMODULE_DISPLAY},
	{"sys.disp.dispHeight", SYSVAR_DISP_DISPHEIGHT, SYSMODULE_DISPLAY}
};
const uint8_t sysVarCount = sizeof(sysVarList) / sizeof(SysVarEntry);

const SysVarEntry* getSysVarEntry(SysVar var) {
	for (uint8_t i = 0; i < sysVarCount; i++) {
		if (sysVarList[i].sysVar == var) {
			return sysVarList + i;
		}
	}
	return nullptr;
}

struct VariableIndexEntry {
	const char* name;
	bool isUDF; //user-defined function
	uint32_t functionID;
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
	//TODOCONSIDER implement a more memory-efficient search, doesn't have to be very fast
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
		if (intRef > INT64_MAX / 10 || position == UINT16_MAX) return false; //TODO fix parseInteger overflow check
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
		if (dblRef > DBL_MAX / 10 || position == UINT32_MAX) return false; //TODO fix parseFloat overflow check
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

//TODOLATER possible improvement by removing redundant decimal check, there's one in parseFloat
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
		ignoreNextQuote = startPtr[i] == '#'; //TODOLATER keeps the # in #", always remove unless ##, in which case keep one #
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

	//search sysVarList for a match to system variables
	//TODOCONSIDER write a faster search algorithm if memory allows
	uint8_t match = 0;
	bool isMatching;
	for (uint8_t i = 0; i < sysVarCount; i++) {
		isMatching = true;
		for (uint8_t j = 0; sysVarList[i].tag[j] != '\0'; j++) {
			if (sysVarList[i].tag[j] != name[j]) {
				isMatching = false;
				break;
			}
		}
		if (isMatching) {
			match = i;
			break;
		}
	}

	varRef = {};
	varRef.type = ARG_VARIABLE;
	varRef.varValue = {};
	varRef.varValue.name = name;
	varRef.varValue.id = 0;
	if (isMatching) {
		varRef.varValue.sysVar = &sysVarList[match];
	} else {
		varRef.varValue.sysVar = getSysVarEntry(SYSVAR_NONE);
	}
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

void updateVariableIndexFromArgumentTree(std::vector<VariableIndexEntry>* indexPtr, ArgumentTree args, uint32_t functionID) {
	bool foundInIndex;
	for (uint32_t i = 0; i < args.length; i++) {
		if (args.start[i].type == ARG_ARRAY) {
			updateVariableIndexFromArgumentTree(indexPtr, args.start[i].arrayValue, functionID);
		} else if (args.start[i].type == ARG_VARIABLE) {
			foundInIndex = false;
			for (uint32_t j = 0; j < (uint32_t)indexPtr->size(); j++) {
				if (std::strcmp((*indexPtr)[j].name, args.start[i].varValue.name) == 0 && functionID == (*indexPtr)[j].functionID) {
					foundInIndex = true;
					break;
				}
			}
			if (!foundInIndex) indexPtr->push_back({ args.start[i].varValue.name, false, functionID });
		}
	}
}

CompileErrorCode updateVariableIndex(std::vector<VariableIndexEntry>* indexPtr, Instruction line, uint32_t functionID) {
	if (line.opCode.OpCode == OP_FUNC) {
		for (uint32_t i = 0; i < (uint32_t)indexPtr->size(); i++) {
			if (std::strcmp((*indexPtr)[i].name, line.args.start[0].varValue.name) == 0) {
				return COMERR_FUNCMULTIDEFINE;
			}
		}
		indexPtr->push_back({ line.args.start[0].varValue.name, true, UINT32_MAX });
		if (line.args.length > 1) {
			ArgumentTree functionParams = { line.args.start + 1, (uint16_t)(line.args.length - 1) };
			updateVariableIndexFromArgumentTree(indexPtr, functionParams, functionID);
		}
		return COMERR_NO;
	}
	//browse the argument tree for variables
	updateVariableIndexFromArgumentTree(indexPtr, line.args, functionID);
	return COMERR_NO;
}

void indexVariableArguments(ArgumentTree args, std::vector<VariableIndexEntry>* indexPtr, uint32_t functionID) {
	for (uint32_t i = 0; i < args.length; i++) {
		if (args.start[i].type == ARG_ARRAY) {
			indexVariableArguments(args.start[i].arrayValue, indexPtr, functionID);
		} else if (args.start[i].type == ARG_VARIABLE) {
			int32_t bestMatchIndex = -1;
			int32_t udfMatchIndex = -1;
			for (uint32_t j = 0; j < (uint32_t)indexPtr->size(); j++) {
				if (strcmp((*indexPtr)[j].name, args.start[i].varValue.name) != 0) continue;
				if (functionID == (*indexPtr)[j].functionID) {
					bestMatchIndex = j;
					break;
				}
				if ((*indexPtr)[j].isUDF) {
					udfMatchIndex = j;
				}
			}
			if (bestMatchIndex >= 0) {
				args.start[i].varValue.id = bestMatchIndex;
				args.start[i].varValue.isUDF = (*indexPtr)[bestMatchIndex].isUDF;
			} else if (udfMatchIndex >= 0) {
				args.start[i].varValue.id = udfMatchIndex;
				args.start[i].varValue.isUDF = (*indexPtr)[udfMatchIndex].isUDF;
			}
		}
	}
}

Compiled compileToRAM(const AppHeader* appPointer, bool doLogVariableIndex) {
	Instruction* compiled = (Instruction*)malloc(sizeof(Instruction) * appPointer->contentLines);
	if (!compiled) {
		currentErrorCode = COMERR_NOMEMALLOC;
		errorLineNumber = 0;
		return INVALID_COMPILED;
	}
	std::vector<VariableIndexEntry>* variableIndex = new std::vector<VariableIndexEntry>();

	bool inFunction = false;
	Instruction* functionLine = nullptr;
	uint32_t functionCount = 0;
	uint32_t currentScopeID = UINT32_MAX;
	CompileErrorCode variableIndexUpdateResult;
	for (uint32_t i = 0; i < appPointer->contentLines; i++) {
		compiled[i] = { getOpCodeFromLine(appPointer->content[i]), getArgumentTreeFromLine(appPointer->content[i]) };
		if (compiled[i].opCode.OpCode == OP_INVALID || (!compiled[i].args.start && compiled[i].args.length != 0)) {
			currentErrorCode = COMERR_INVALIDOPCODE;
			errorLineNumber = i + 1;
			for (uint32_t j = 0; j < i; j++) {
				freeArgumentTree(compiled[j].args);
			}
			free(compiled);
			return INVALID_COMPILED;
		}
		if (compiled[i].opCode.argCount != compiled[i].args.length && compiled[i].opCode.argCount != NOARGCOUNT) {
			//std::cout << "Counted " << compiled[i].args.length << " : Need " << (uint16_t)compiled[i].opCode.argCount << " : for " << compiled[i].opCode.tag << ";";
			currentErrorCode = COMERR_NOARGCOUNTMATCH;
			errorLineNumber = i + 1;
			for (uint32_t j = 0; j < i; j++) {
				freeArgumentTree(compiled[j].args);
			}
			free(compiled);
			return INVALID_COMPILED;
		}
		if (compiled[i].opCode.OpCode == OP_FUNC) {
			if (inFunction) {
				currentErrorCode = COMERR_NESTEDFUNC;
				errorLineNumber = i + 1;
				for (uint32_t j = 0; j < i; j++) {
					freeArgumentTree(compiled[j].args);
				}
				free(compiled);
				return INVALID_COMPILED;
			}
			inFunction = true;
			functionLine = compiled + i;
			functionCount++;
			currentScopeID = functionCount;
			compiled[i].args.start[0].varValue.id = currentScopeID;
		} else if (compiled[i].opCode.OpCode == OP_FUNCEND) {
			if (!inFunction) {
				currentErrorCode = COMERR_UNKNOWNFUNCEND;
				errorLineNumber = i + 1;
				for (uint32_t j = 0; j < i; j++) {
					freeArgumentTree(compiled[j].args);
				}
				free(compiled);
				return INVALID_COMPILED;
			}
			inFunction = false;
			functionLine = nullptr;
			currentScopeID = UINT32_MAX;
		}
		variableIndexUpdateResult = updateVariableIndex(variableIndex, compiled[i], currentScopeID);
		if (variableIndexUpdateResult != COMERR_NO) {
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
		currentErrorCode = COMERR_NOFUNCEND;
		errorLineNumber = appPointer->contentLines; //TODOLATER point no function end error to the right line
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
	if (doLogVariableIndex) logVariableIndex(variableIndex);
	//update variables using indices from the table and scope variables
	uint32_t functionID = UINT32_MAX;
	for (uint32_t i = 0; i < appPointer->contentLines; i++) {
		//look through each argument, if type variable add id, isUDF, and functionID (to distinguish by scope)
		if (compiled[i].opCode.OpCode == OP_FUNC) {
			if (compiled[i].args.length > 0) {
				functionID = compiled[i].args.start[0].varValue.id;
				if (compiled[i].args.length > 1) {
					ArgumentTree functionParams = { compiled[i].args.start + 1, (uint16_t)(compiled[i].args.length - 1) };
					indexVariableArguments(functionParams, variableIndex, functionID);
				}
			}
		} else if (compiled[i].opCode.OpCode == OP_FUNCEND) {
			functionID = UINT32_MAX; //exit out of the function scope
		} else {
			indexVariableArguments(compiled[i].args, variableIndex, functionID); //non-function
		}
	}
	delete variableIndex;
	return { compiledFunctions, functionCount, compiled, appPointer->contentLines, appPointer };
}

/*bool compileToFile(fs::FS& fileSystem, const char* filePath) {
	//TODOLATER compile to file
	//SD filesys must already be initialized
	File file = fileSystem.open(filePath);
	return file.available();
}*/

//functions used by debug logging functions
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

//below are debug logging functions (std::cout)
void logVariableIndex(std::vector<VariableIndexEntry>* indexPtr) {
	logToConsole("Logging Variable Index");
	for (uint32_t i = 0; i < indexPtr->size(); i++) {
		if ((*indexPtr)[i].isUDF) {
			logToConsole((std::string("Function: ") + std::string((*indexPtr)[i].name) + std::string(" : Index ") + std::to_string(i)).c_str());
		} else {
			logToConsole((std::string("Variable: ") + std::string((*indexPtr)[i].name) + std::string(" : Index ") + std::to_string(i)).c_str());
		}
	}
}

void logArgumentTree(ArgumentTree tree, uint16_t subTree = 0) {
	if (subTree == 0) {
		if (tree.start != nullptr) {
			logToConsole((std::string("Content Size ") + std::to_string(getArgumentTreeSize(tree)) + std::string(" -  Logging Argument Tree...")).c_str());
		} else {
			logToConsole((std::string("Content Size ") + std::to_string(getArgumentTreeSize(tree)) + std::string(" -  Empty of Invalid Tree.")).c_str());
			return;
		}
	} else {
		logToConsole((std::string("Array with Depth ") + std::to_string(subTree)).c_str());
	}
	for (uint16_t i = 0; i < tree.length; i++) {
		for (uint16_t j = 0; j < subTree; j++) {
			logToConsole("    ", true);
		}
		logToConsole((std::string("Size ") + std::to_string(getArgumentSize(tree.start + i)) + std::string(" - ")).c_str());
		switch (tree.start[i].type) {
			case ARG_ARRAY:
				logArgumentTree(tree.start[i].arrayValue, subTree + 1);
				break;
			case ARG_STRING:
				logToConsole((std::string("String: \"") + std::string(tree.start[i].strValue) + std::string("\"")).c_str());
				break;
			case ARG_INTEGER:
				logToConsole((std::string("Integer: ") + std::to_string(tree.start[i].intValue)).c_str());
				break;
				case ARG_FLOAT:
				logToConsole((std::string("Float: ") + std::to_string(tree.start[i].intValue)).c_str());
				break;
				case ARG_BOOL:
				logToConsole((std::string("Boolean: ") + std::string(((tree.start[i].boolValue) ? "true" : "false"))).c_str());
				break;
				case ARG_COLOR:
				logToConsole((std::string("Color: ") + std::string(tree.start[i].colorValue)).c_str());
				break;
				case ARG_VARIABLE:
				if (tree.start[i].varValue.isUDF) {
					logToConsole((std::string("Function (Call/Name): ") + std::string(tree.start[i].varValue.name) + std::string(" : ID: ") + std::to_string(tree.start[i].varValue.id)).c_str());
				} else {
					logToConsole((std::string("Variable: ") + std::string(tree.start[i].varValue.name) + std::string(" : ID: ") + std::to_string(tree.start[i].varValue.id)).c_str());
				}
				break;
			case ARG_INVALID:
				logToConsole("Invalid Argument");;
				break;
			default:
				logToConsole("Unknown Argument");;
		}
	}
	if (subTree != 0) {
		for (uint16_t j = 0; j < subTree - 1; j++) {
			logToConsole("    ", true);
		}
	}
}

void logInstruction(Instruction inst) {
	if ((!inst.args.start && inst.args.length != 0) || inst.opCode.OpCode == OP_INVALID) {
		logToConsole("Invalid Instruction");
		return;
	}
	uint8_t i = 0;
	while (i < UINT8_MAX && inst.opCode.OpCode != opCodeList[i].OpCode) i++;
	if (i == UINT8_MAX) return;
	logToConsole(opCodeList[i].tag);
	logArgumentTree(inst.args);
}

void logFunction(Function func) {
	logToConsole("Logging Function...");
	if (!func.entry) {
		logToConsole("Invalid Function");
		return;
	}	for (uint32_t i = 0; i < func.length; i++) {
		logInstruction(func.entry[i]);
	}
}

void logCompiledRAM(Compiled source) {
	logToConsole((std::string("Code ") + std::to_string(currentErrorCode + 0) + std::string(" ") + std::string(getErrorText(currentErrorCode))).c_str());
	logToConsole((std::string("Line ") + std::to_string(errorLineNumber)).c_str());
	if (!source.functions) {
		logToConsole("Invalid Compiled");
		return;
	}
	for (uint32_t i = 0; i < source.functionCount; i++) {
		logFunction(source.functions[i]);
	}
}