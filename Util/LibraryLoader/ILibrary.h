//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 02/28/2020      -      Yung-Yeh Chang   Original Release
//*************************************************************************************************

#ifndef ILIBRARY_H
#define ILIBRARY_H

#include <string>
#include <vector>

namespace Util {

/*
 * ILibrary is the library (API) interface for a library.
 * We need to define the library file name and all the symbols that needs to be parsed.
 * Finally, we instruct how to load the library and parse the symbols.
 *
*/
class ILibrary
{

public:
    ILibrary();
    virtual ~ILibrary();

public:
    // Instruct to load the library
    virtual void loadLib() = 0;
    // Instruct to iunload the library
    virtual void unloadLib() = 0;
    // Instruct to parse symbols from a library
    virtual void linkLib() = 0;
    // Return the library (dll) file name that contains API we want to load.
    virtual std::string libName() = 0;
    // A vector contains symbol names that needs to be resolved.
protected:
    virtual std::vector<std::string> symbols() = 0;
};

}

#endif // ILIBRARY_H
