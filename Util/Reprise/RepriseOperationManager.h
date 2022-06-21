//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 05/29/2020      -      Yung-Yeh Chang   Original Release
//*************************************************************************************************

#ifndef REPRISEOPERATIONMANAGER_H
#define REPRISEOPERATIONMANAGER_H

#include <string>
#include "RepriseDynamicLoadLibrary.h"
#pragma comment(lib, "shell32")
#pragma comment(lib, "ole32")

namespace Util {
namespace License {

/* The RepriseOperationManager is a singleton class which provides
 * necessary operations for the Reprise licensing such as activation
 * and deactivation as well as the error information.In fact, any
 * operation should have a corresponding method in this class.
 */
class RepriseOperationManager
{    

private:
    RepriseOperationManager();
public:
    enum class ActAction {NORMAL,RETRIVE,OFFLINE};
    enum class ErrorLevel {SUCCESS,WARNING,REACTIVATE,FAIL,UNKNOWN}; // If Code > Code::WARNING, indicating termination of application.
    ~RepriseOperationManager();
    static RepriseOperationManager &instance();
    RepriseOperationManager(RepriseOperationManager const&) = delete;
    void operator=(RepriseOperationManager const&) = delete;

public:
    // Access properties
    RLM_HANDLE& rlmHandle();
    RLM_LICENSE& rlmLicense();
    bool isHandleSet() { return _isRlmHandleSet; }
    const std::string licenseFileName() const;
    const std::string licenseFilePath () const;
    const std::string& product() const { return _product; }
    void setProduct(const std::string &name) { _product = name; }
    const std::string& versionNumber() const { return _versionNumber; }
    void setVersionNumber(const std::string &number) { _versionNumber = number; }
    const std::string& isvName() const { return _isvName; }
    void setIsvName(const std::string &name) { _isvName = name; }
    const std::string& server() const { return _server; }
    void setServer(const std::string &server) { _server = server; }
    RepriseDynamicLoadLibrary& rlmLib() { return _rlmLib; }
    bool isPassed() { return _isPassed; }
    void setPassed(bool pass) { _isPassed = pass; }

    // License operations
    int initialRlmHandle();
    int getReHostId(std::string &id);
    int closeRlmHandle();
    int checkLicense(std::string &message, int &expDay); // Check license. Return error code, a message and expire day (if any)
    int checkKeyValid(); // Contact to the server for license checking
    int revokeLicense(); // Revoke the license if checkKeyValid() returns error but was able to check-out
    void errorMessagebyCode(int status, std::string &message);
    bool runLicenseManager(); // Call "LicenseManager.exe" with a detached process    
    std::string errorMessage(const int &status, const int &expDay, ErrorLevel &errLevel); // Error message with Errorlevel code
    std::string errorMessage(const int &status, const int &expDay); // Error message

private:
    int checkoutLicense(); // Check-out a license
    void checkinLicense();    // Check-in a license
    ErrorLevel getErrorLevel(int status, const int &expDay); // Use 4-level error code to determine the error message content
    void establishLicenseFileDir(); // Locate the license directory
    std::string getCurrentDir();

private:    
    RLM_HANDLE _rh = RLM_HANDLE(nullptr);
    RLM_LICENSE _lic = RLM_LICENSE(nullptr);
    std::string _licenseFileDir;
    std::string _product = "";
    std::string _versionNumber = "";
    std::string _isvName = "";
    std::string _server = "";
    RepriseDynamicLoadLibrary _rlmLib; // library
    bool _isRlmHandleSet = false; // rlm handle status (true: open, false: close)
    bool _isPassed = false;
};

}
}

typedef Util::License::RepriseOperationManager RLM;

#endif // REPRISEOPERATIONMANAGER_H
