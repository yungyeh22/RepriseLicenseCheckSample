//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 05/29/2020      -      Yung-Yeh Chang   Original Release
//*************************************************************************************************

#ifndef DYNAMICLOADLIBRARY_H
#define DYNAMICLOADLIBRARY_H
#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>

namespace Util {

typedef void (*mFunctionPointer)() ;
bool loadDll(const char* dllfile, HINSTANCE* dllhandle); // Validate DLL and return handle
DWORD WINAPI unloadDll(LPVOID Param); // Unload DLL

/*
 * This class is a class designed to conveniently load the library and access symbols in a library.
 *
 * An example of using this class
 * 	#include<engine.h>
 *     Util::AppInfo::DynamicLoadLibrary engLib("mylib.dll");
 *     myLib.setFunctionNames(std::vector<std::string>{"lOpen"});
 *     myLib.load();
 *     auto lOpenPtr = myLib.getFunction<decltype(void (*lOpen)(const char*))>("lOpen");
 *     lOpenPtr(""); // Run lOpenPtr to start the MATLAB engine
*/
class DynamicLoadLibrary
{
public: // Methods
    DynamicLoadLibrary();
    DynamicLoadLibrary(const std::string &filename);
    DynamicLoadLibrary(std::string &&filename);
    DynamicLoadLibrary(const char* filename);
    virtual ~DynamicLoadLibrary();
    virtual bool load(); // Load library
    virtual bool unload(); // Unload Library
    bool isLoaded() const {return _isLoaded;} // Loading status, including the result for resoving symbols.
    bool missSymbol() const {return _missSymbol;}    
    virtual void setDllName (const std::string &filename) { _dllName = filename; }
    virtual void setFunctionNames (const std::vector<std::string> &functionNames);

    // Cast function pointer with appropriate function pointer type
    // 'const' is necessary becuase all member variables are 'const'.
    template<typename fcn>
    fcn getFunction(const std::string &functionName) const{
        return reinterpret_cast<fcn>(_functionPtrs.at(functionName));
    }
    const HINSTANCE &getDll() const {
        return _dllPtr;
    }

protected: // Properties
    bool _isLoaded = false; // Dll status. If library is unloaded or failed on loading, returns "false".
    bool _missSymbol = false; // Indicating any of the symbol that was intended to resolve goes wrong.
    std::string _dllName = ""; // Library name


private:
    HINSTANCE _dllPtr = nullptr; // Storage for dll
    // Storage to actual function pointer. <function name, function pointer>.
    // The purpose is to store resolved symbols and automatically pair with associated function names
    // The reason to use std::string instead QString is because QLibrary:resolve takes argument <const char*>.
    // So it could be more conveient to use std::string for type casting.
    std::map<const std::string, mFunctionPointer> _functionPtrs;
    virtual void resolveSymbols(); // Resolv symbols from dll

};

}

#endif // DYNAMICLOADLIBRARY_H
