//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 05/29/2020      -      Yung-Yeh Chang   Original Release
//*************************************************************************************************

#include "RepriseOperationManager.h"
#include <ctime>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <ShlObj.h>
#include <combaseapi.h>
#include <sys/stat.h>

namespace Util {
namespace License {

RepriseOperationManager::RepriseOperationManager() {
    establishLicenseFileDir();
}

RepriseOperationManager::~RepriseOperationManager() {}

RepriseOperationManager &RepriseOperationManager::instance() {
    static RepriseOperationManager instance;
    return instance;
}

const std::string RepriseOperationManager::licenseFileName() const {
    std::string fileName = "";
    if (!_product.empty() && !_versionNumber.empty()) {
        fileName = _product + _versionNumber + ".lic";
    }
    return fileName;
}

const std::string RepriseOperationManager::licenseFilePath() const {
    std::string fileName = licenseFileName();
    if (fileName.empty())  {
        return "";
    }
    else {
        return (_licenseFileDir + "/" + licenseFileName());
    }
}

int RepriseOperationManager::initialRlmHandle() {
    if (_isRlmHandleSet) {
        closeRlmHandle();
    }
    int status;
#ifdef WIN32
    std::string lFilePath = licenseFilePath() + ";.;" + _licenseFileDir; // License file dir + current working dir
#else

#endif
    _rh = RLM::instance().rlmLib().rlm_init(lFilePath.c_str(), getCurrentDir().c_str(), reinterpret_cast<char*>(0));
    status = RLM::instance().rlmLib().rlm_stat(_rh);
    if (status == RLM_EH_READ_NOLICENSE || !status) {
        _isRlmHandleSet = true;
    }
    else {
        _isRlmHandleSet = false;
    }
    return status;
}

int RepriseOperationManager::getReHostId(std::string &id) {
    int status;
    if (!_isRlmHandleSet) {
        status = initialRlmHandle();
        if (!_isRlmHandleSet) {
            return status;
        }
    }
    char hostid[RLM_MAX_HOSTID_STRING+1];
    status = _rlmLib.rlm_get_rehost(_rh, _product.c_str(), hostid);
    if (!status) {
        id = hostid;
    }
    return status;
}

int RepriseOperationManager::closeRlmHandle() {
    if (_isRlmHandleSet) {
        _rlmLib.rlm_close(_rh);
        _isRlmHandleSet = false;
    }
    return 0;
}

int RepriseOperationManager::checkLicense(std::string & message, int &expDay) {
    int status = checkoutLicense();
    if (status) {
        errorMessagebyCode(status, message);
        expDay = 0;
    }
    else {
        expDay = _rlmLib.rlm_license_exp_days(_lic);
    }
    checkinLicense();
    return status;
}

int RepriseOperationManager::checkKeyValid() {
    int status = checkoutLicense();
    if (!status) {
        std::string aKey = _rlmLib.rlm_license_akey(_lic);
        char* hostId = _rlmLib.rlm_license_hostid(_lic);
        status = _rlmLib.rlm_act_keyvalid(_rh, _server.c_str(), aKey.c_str(), hostId);
    }
    else {
        status = RLM_ACT_ERR_FIRST; // Return a general activation error rather than a check-out error from checkLicense()
    }
    checkinLicense();
    return status;
}

int RepriseOperationManager::revokeLicense() {
    int status;
    if (!RLM::instance().isHandleSet()) {
        status = RLM::instance().initialRlmHandle();
        if (!RLM::instance().isHandleSet()) {
            return status;
        }
    }
    char verification[3 * RLM_MAX_LINE + 1];
    status = _rlmLib.rlm_act_revoke_disconn(_rh,"", _product.c_str(), verification);
    return status;
}

void RepriseOperationManager::errorMessagebyCode(int status, std::string &message) {
    char errstring[RLM_ERRSTRING_MAX];
    message = _rlmLib.rlm_errstring_num(status,errstring);
}

bool RepriseOperationManager::runLicenseManager() {
    bool runResult = true;
    std::string commandStr = "\"" + _product + "\"" 
		+ " \"" +_versionNumber + "\"" 
		+ " \"-provider\" " + "\"" +_isvName + "\""
		+ " \"-server\" " + "\"" +_server + "\"";	
#ifdef _WIN32 // Must complie with unicode charset
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
#if (defined UNICODE) || (defined _UNICODE)
    std::wstringstream cls;
    cls << "LicenseManager.exe " << commandStr.c_str();
    std::wstring cmdOpts = cls.str();
    LPWSTR cmdOptsChar= const_cast<LPWSTR>(cmdOpts.c_str());
#else
    std::stringstream cls;
    cls << "LicenseManager.exe " << commandStr;
    std::string cmdOpts = cls.str();
    LPSTR cmdOptsChar = const_cast<LPSTR>(cmdOpts.c_str());
#endif
    // Start the child process.
    if( !CreateProcess( NULL,   // No module name (use command line)
        cmdOptsChar,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        NORMAL_PRIORITY_CLASS | DETACHED_PROCESS,
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) {
        runResult = false;
    }
    // Wait until child process exits.
    //    WaitForSingleObject( pi.hProcess, INFINITE );
    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#elif __APPLE__
    QDir appDir(QCoreApplication::applicationDirPath());
    // Move the path one level up to the app path
    appDir.cdUp();
    appDir.cdUp();
    appDir.cdUp();
    QString lm = appDir.absolutePath() + "/" + QCoreApplication::applicationName() + "_License_Manager.app";
    runResult = false;
#endif

//    // If license manager is comprimised, ask user to reinitall or contact TEG
//    QFileInfo checkFile(lm);
//    if (checkFile.exists() && (checkFile.isFile() || checkFile.isBundle())) {
//#ifdef _WIN32
//        QStringList args = {QApplication::applicationName(),QApplication::applicationVersion(),"-server",RLM::instance().server()};
//        // LicenseManager.exe DMMS 1.0 -server http://demo1.hostedactivation.com
//        runResult = QProcess::startDetached("\""+lm+"\"",args); // Due to space in file name, wrap quote around the file name
//#elif __APPLE__
//        runResult = QProcess::startDetached("open \"" + lm + "\""); // Use open command to run the app
//#endif
//    } else {
//        QString lmWarningMsgTitle("Error loading " + lm);
//        QString lmWarningMsg(lm+" does not exist. Please uninstall/reinstall DMMS or contact The Epsilon Group.");
//#ifdef LIBBUILD
//        cout << lmWarningMsg.toStdString() << endl;
//#else
//        QMessageBox::warning(nullptr, lmWarningMsgTitle, lmWarningMsg);
//#endif
//    }
    return runResult;
}

std::string RepriseOperationManager::errorMessage(const int &status, const int &expDay, ErrorLevel &errLevel) {
    errLevel = getErrorLevel(status,expDay);
    // Manage content
    std::string msgBody;
    switch (errLevel) {
    case ErrorLevel::SUCCESS:
        msgBody = "SUCCESS"; // This won't be reached.
        break;
    case ErrorLevel::WARNING:
        msgBody = "REMINDER!";
        break;
    case ErrorLevel::REACTIVATE:
        msgBody = "ACTIVATION OR REACTIVATION MAY BE REQUIRED!";
        break;
    case ErrorLevel::FAIL:
        msgBody = "LICENSE CHECK FAILED";
        break;
    }
    std::string msgTitle;
    if (errLevel > ErrorLevel::WARNING) { // Indicating error
        msgTitle = "License Error";
        msgBody += "\n\nError code = " + std::to_string(status);
        if (errLevel < ErrorLevel::FAIL) { // Need to reactivate
            if (status == RLM_EL_EXPIRED || status == RLM_EL_BADVER || status == RLM_EL_NOTTHISHOST) { // custom message
                msgBody += "\n\nLicense expired or revoked!";
            }
            msgBody += "\n\nPlease use License Manager to activate or reactivate, or contact The Epsilon Group for further assistance.";
            msgBody += "\n\nIf you just purchased an upgrade, you must activate again.";
        }
        else { // Other reason failed
            if (status == -99999) { // Old license
                msgBody += "\n\nYou have previously activated. Please note that the activation is no longer valid.";
                msgBody += "\n\nA new activation code is required.";
            }
            if (status == RLM_EL_TS_DISABLED || status == RLM_EL_VM_DISABLED) { // RD or VM
                msgBody += "\n\nRemote Desktop or Virtual Machines is not allowed with this license.";
            }
            else {
                if (expDay == -1) {
                    msgBody += "\n\nFail to load license system!\n\nPlease reinstall or contact The Epsilon Group for further assistance.";
                }
                else {
                    msgBody += "\n\nPlease use License Manager to activate, or contact The Epsilon Group for further assistance.";
                }
            }
        }
    }
    else if (errLevel == ErrorLevel::WARNING){ // Indicating warning. Warning can pass
        msgTitle = "License Warning";
        if (expDay > 0) {
            msgBody += "\n\nYour license will expire in" + std::to_string(expDay) +  " day(s).";
        }
        msgBody += "\n\nIf you have purchased an upgrade, please open \"License Manager\" to activate.";
    }
    // Show message
    return msgTitle + "\n\n" + msgBody;
}

std::string RepriseOperationManager::errorMessage(const int &status, const int &expDay) {
    ErrorLevel errLevel;
    return errorMessage(status, expDay, errLevel);
}

int RepriseOperationManager::checkoutLicense() {
    int status;
    std::string verNumRepriseFormat = _versionNumber;
    verNumRepriseFormat.erase(std::remove(verNumRepriseFormat.begin(), verNumRepriseFormat.end(), '.'), verNumRepriseFormat.end());
    verNumRepriseFormat.insert(1,".");
#ifdef _DEBUG
    std::cout << "Check out version:" << verNumRepriseFormat << std::endl;
#endif
    _lic = _rlmLib.rlm_checkout(_rh,_product.c_str(),verNumRepriseFormat.c_str(),1);
    status = _rlmLib.rlm_license_stat(_lic);
    return status;
}

void RepriseOperationManager::checkinLicense() {
    if (_lic) { // reset license handle
        _rlmLib.rlm_checkin(_lic);
        _lic = RLM_LICENSE(nullptr);
    }
}

RepriseOperationManager::ErrorLevel RepriseOperationManager::getErrorLevel(int status, const int &expDay) {    
    ErrorLevel level;
    if (!status) {
        if (expDay>0) {
            level = ErrorLevel::WARNING;
        }
        else {
            level = ErrorLevel::SUCCESS;
        }
    }
    else if (status == RLM_EL_EXPIRED || status == RLM_EL_BADVER || status == RLM_EL_NOTTHISHOST) {
        level = ErrorLevel::REACTIVATE;
    }
    else {
        level = ErrorLevel::FAIL;
    }
    return level;
}

void RepriseOperationManager::establishLicenseFileDir() {
#ifdef _WIN32
    PWSTR roamingAppDataPtr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roamingAppDataPtr);
    std::wstringstream ss;
    ss << roamingAppDataPtr << L"\\The Epsilon Group\\TEG License Manager\\data";
    std::wstring ws = ss.str();
    _licenseFileDir = std::string(ws.begin(),ws.end());
#ifdef _DEBUG
    std::cout << "License file location should be:" << _licenseFileDir << std::endl;
#endif
    CoTaskMemFree(roamingAppDataPtr);
#endif
    struct stat buffer;
    if (stat(_licenseFileDir.c_str(), &buffer) != 0) { // If the dir not found in AppData, use working directory
        _licenseFileDir =  getCurrentDir(); // Current working directory
#ifdef _DEBUG
        std::cout << "Current working directory is: " << _licenseFileDir << std::endl;
#endif
    }
#ifdef _DEBUG
    std::cout << "Actual license file location is: " << _licenseFileDir << std::endl;
#endif
}

std::string RepriseOperationManager::getCurrentDir() {
    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);
    return std::move(currentDir);
}

}
}
