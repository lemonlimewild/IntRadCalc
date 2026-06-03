#include "Interpreter.h"
#include <vector>

uint32_t currentLine = 0;

enum RuntimeError : uint8_t {
    RUNERR_NO,
    RUNERR_UNKNOWNOPCODE
};

struct Variable {
    uint32_t id;
    Argument value;
};

struct Scope {
    std::vector<Variable> varArrPtr = {};
};

std::vector<Scope> scopeStack;

RuntimeError executeLine(Instruction line) {
    Scope scope = scopeStack[scopeStack.size() - 1];
    switch (line.opCode.OpCode) {
        case OpValue::OP_NEW:
            scope.varArrPtr.push_back({line.args.start[0].varValue.id, line.args.start[1]});
            break;
        default:  
            return RuntimeError::RUNERR_UNKNOWNOPCODE;
    };
    return RuntimeError::RUNERR_NO;
}

void beginExecution(Compiled appData) {
    if (appData.appHeaderPtr->minOSVer > softwareVersion) return;
    scopeStack = {};
    scopeStack.push_back({});
    RuntimeError lineResult = RuntimeError::RUNERR_NO;
    for (uint32_t i = 0; i < appData.functions[0].length; i++) {
        lineResult = executeLine(appData.functions->entry[i]);
    }
}