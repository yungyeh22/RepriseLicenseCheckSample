//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 05/12/2017      -      Yung-Yeh Chang   Original Release
//*************************************************************************************************

#include "DynamicLoadLibrary.h"

namespace Util {

bool loadDll(const char *dllfile, HINSTANCE *dllhandle) {
    *dllhandle = LoadLibraryA(dllfile);
    if (!*dllhandle) {
        return false;
    }
    else {
        return true;
    }
}

DWORD unloadDll(LPVOID Param) {
    //	Param should be the address of the dllhandle
    HINSTANCE *dllhandle = (HINSTANCE*)Param;
    // This is a generic unload thread
    // If a piror action that calls to the dll may spawns a thread and you need to wait for it to finish,
    // you then have to write your own unload mehtod.
    //	Finally, unload the DLL
    FreeLibraryAndExitThread(*dllhandle,0);
//    *dllhandle = nullptr;
//    return 0;
}

DynamicLoadLibrary::DynamicLoadLibrary() {

}

DynamicLoadLibrary::DynamicLoadLibrary(const std::string &filename) : _dllName(std::string(filename.c_str())){

}

DynamicLoadLibrary::DynamicLoadLibrary(std::string &&filename) : _dllName(std::move(filename)) {

}

DynamicLoadLibrary::DynamicLoadLibrary(const char *filename) : _dllName(std::string(filename)) {

}

DynamicLoadLibrary::~DynamicLoadLibrary() {
    unload();
}

bool DynamicLoadLibrary::load() {
    bool loadSuccess = true;
    if (_dllName != "") {
        if (_isLoaded) {
            loadSuccess = true;
        }
        else {
            loadSuccess = loadDll(_dllName.c_str(),&_dllPtr);
        }
        if (loadSuccess) {
            resolveSymbols();
            if (_missSymbol) {
                unload();
                loadSuccess = false;
            }
        }
    }
    _isLoaded = loadSuccess;
    return loadSuccess;
}

bool DynamicLoadLibrary::unload() {
    // If a piror action that calls to the dll may spawns a thread and you need to wait for it to finish,
    // you then have to write your own unload mehtod (or unload thread proc). You will need to wait for the
    // thread in dll to finish and then call the FreeLibraray (or FreeLibrarAndExitThread) or the access
    // violation (AV) error could occur due to the race condition.
    if (_isLoaded) {
#ifdef _WIN32
        DWORD  UnloadDLLThreadID;
        HANDLE hThread =  CreateThread(nullptr, 0, unloadDll, &_dllPtr, 0, &UnloadDLLThreadID);
        // WaitForSingleObject(hThread, INFINITE); // Debug only
        // _isLoaded = (_dllPtr) ? true:false; // No way to check _dllPtr because the unload is in a separate thread
        CloseHandle(hThread);
        _isLoaded = false; // Set _isloaded to false because the dll reference will be reduced anyway.
    }
#endif
    return !_isLoaded;
}

void DynamicLoadLibrary::setFunctionNames(const std::vector<std::string> &functionNames) {
    _functionPtrs.clear();
    for (const auto &fn : functionNames) {
        _functionPtrs.emplace(fn,mFunctionPointer(nullptr));
    }
#ifdef _DEBUG
    std::cout << "Load functions completed." << std::endl;
#endif
}

void DynamicLoadLibrary::resolveSymbols() {
//    _isLoaded = true;
    if (!_functionPtrs.empty()) {
        for (auto &f : _functionPtrs) {
            mFunctionPointer fptr = reinterpret_cast<mFunctionPointer>(GetProcAddress(_dllPtr, f.first.c_str()));
            if (fptr) {
                f.second = fptr;                
            }
            else { // symbol not found
#ifdef _DEBUG
                std::cout << "Failed to Resolve \"" << f.first << std::endl;
#endif
                _missSymbol = true;
                break;
            }
        }
    }
#ifdef _DEBUG
    std::cout << "Resolve symbols completed." << std::endl;
    if (_missSymbol) {
        std::cout << "Resolve symbols failed." << std::endl;
    }
#endif
}

}
