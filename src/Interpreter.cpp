#include "Interpreter.h"
#include <vector>

enum RuntimeError : uint8_t {
    RUNERR_NO,
    RUNERR_UNKNOWNOPCODE
};


struct Variable {
    uint32_t id;
    Argument value;
};

struct Scope {
    std::vector<Argument> varArray = {};
};

struct ProgramState {
    uint32_t currentLine;
    std::vector<Scope> scopeStack;
    bool executionPaused;
    Compiled appData;
};
std::vector<ProgramState> runningApps = {};

void logPermissions(uint8_t permissions) {
    //TODO log permissions function
}

RuntimeError executeLine(ProgramState state, bool logOperations = false) {
    Scope scope = state.scopeStack[state.scopeStack.size() - 1];
    Instruction line = state.appData.instructions[state.currentLine];
    switch (line.opCode.OpCode) {
        case OpValue::OP_FUNC:
            state.scopeStack.push_back({});
            scope = state.scopeStack[state.scopeStack.size() - 1];
            if (logOperations) logToConsole("New scope created");
            break;
        case OpValue::OP_FUNCEND:
            state.scopeStack.pop_back();
            scope = state.scopeStack[state.scopeStack.size() - 1];
            if (logOperations) logToConsole("Scope exited");
            break;
        case OpValue::OP_NEW:
            scope.varArray[line.args.start[0].varValue.id] = line.args.start[1];
            break;
        case OpValue::OP_SET:
            scope.varArray[line.args.start[0].varValue.id];
            break;
        default:  
            return RuntimeError::RUNERR_UNKNOWNOPCODE;
    };
    return RuntimeError::RUNERR_NO;
}

void endExecution() {
    
}

void beginExecution(Compiled appData) {
    if (appData.appHeaderPtr->minOSVer > softwareVersion) return;
    logToConsole("Starting execution of ");
    logToConsole(appData.appHeaderPtr->appName, true);
    logToConsole("Minimum V-OS version: ");
    logToConsole((std::to_string(appData.appHeaderPtr->minOSVer)).c_str(), true);
    logPermissions(appData.appHeaderPtr->permissions);

    runningApps.push_back({0, {}, false, appData});
    RuntimeError lineResult = RuntimeError::RUNERR_NO;
    for (uint32_t i = 0; i < runningApps.size(); i++) {
        lineResult = executeLine(runningApps[i], true);
    }
}