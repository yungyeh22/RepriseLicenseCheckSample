#include "LicenseCheck.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <Windows.h>
#pragma comment(lib, "User32")

RLM::ErrorLevel licenseCheck(std::string &message,
                  const std::string &product,
                  const std::string &version,
                  const std::string &isvName,
                  const std::string server) {
    int returnCode = -9999;
    return licenseCheck(returnCode,message,product,version,isvName,server);
}

RLM::ErrorLevel licenseCheck(int &returnCode,
                  std::string &message,
                  const std::string &product,
                  const std::string &version,
                  const std::string &isvName,
                  const std::string server) {
    RLM::ErrorLevel status = RLM::ErrorLevel::UNKNOWN;
    if (RLM::instance().isPassed()) {
        returnCode = 0;
        return RLM::ErrorLevel::SUCCESS;
    }
    // Check DLL
    RLM::instance().rlmLib().loadLib();
	if (!RLM::instance().rlmLib().isloaded()) {	
		status = RLM::ErrorLevel::UNKNOWN;
		returnCode = -9999;
		message = "Critical Error\n\nFailed to load \"rlm1402.dll\"\n";
		return status; // ***First return point. Load DLL failed.***
	}
    /*****************************************************************************/
    /*  TODO Temp setting for test. Overwrite the product name for test now      */
    RLM::instance().setProduct(product); // The app name
    RLM::instance().setVersionNumber(version); // Version name must be a floating number in string
    RLM::instance().setIsvName(isvName); // This is the activation pro server's name
    RLM::instance().setServer(server); // Server URL
    /*****************************************************************************/    
    int expDay = -1;
    // Check license system
    int sysReturnCode = RLM::instance().initialRlmHandle();
    returnCode = sysReturnCode;
#ifdef _DEBUG
    std::cout << "RLM system initialization: " << sysReturnCode << std::endl;
#endif
    if (sysReturnCode) {
        if (sysReturnCode != RLM_EH_READ_NOLICENSE) {
            message = RLM::instance().errorMessage(sysReturnCode,expDay,status);
            RLM::instance().rlmLib().unloadLib();
            return status; // ***Second return point. System is not ready***
        }
    }
    // Check license (check-in + check-out)
    std::string msg; // Reprise defiend error message
    int rlmLicenseReturnCode = RLM::instance().checkLicense(msg,expDay);
    returnCode = rlmLicenseReturnCode;
#ifdef _DEBUG
    std::cout << "Exp Day:" << expDay << std::endl;
    std::cout << "RLM License check status:" << msg << "(" << rlmLicenseReturnCode << ")" << std::endl;
    std::cout << "Is RLM passed? " << !rlmLicenseReturnCode << std::endl;
#endif
    if (!rlmLicenseReturnCode) {
        RLM::instance().setPassed(true);
        if (expDay > 0) { // This sets the status to WARNING
            message += RLM::instance().errorMessage(rlmLicenseReturnCode, expDay, status);
            message += "\n";
        }
        else {
			status = RLM::ErrorLevel::SUCCESS;
#ifdef _DEBUG
            std::cout << "Pass license check with unlimited license." << status << std::endl;				
#endif
        }
    }
    else { // If not passed, no need to check further.
        message += RLM::instance().errorMessage(rlmLicenseReturnCode, expDay, status);
        RLM::instance().rlmLib().unloadLib();
        return status; // ***Third return point. License checked-in failed***
    }
    // However, even tho the license appears to be valid, we still randomly do a online checking.
    // Perform a check for decommissioned customer machines (20% chance)
    expDay = 0;
    srand (unsigned(time(nullptr)));
    int rndNum = rand() % 100;
    bool isTimeToCheck = rndNum < 20;
#ifdef _DEBUG
    std::cout << "Random number: " << rndNum << std::endl;
    std::cout << "Is time to check server?" << isTimeToCheck << std::endl;
#endif
    if (isTimeToCheck) {
        message += "Additional on-line license validation:\n";
        int keyValid = RLM::instance().checkKeyValid();
        returnCode = keyValid;
#ifdef _DEBUG
        std::cout << "Key valid returns code(check from server)?" << keyValid << std::endl;
#endif
        if ((keyValid == RLM_ACT_KEY_DISABLED) || (keyValid == RLM_ACT_KEY_NO_HOSTID)|| (keyValid == RLM_ACT_KEY_HOSTID_REVOKED)) {
			std::string errMessage;
			RLM::instance().errorMessagebyCode(keyValid, errMessage);
			message += errMessage;
			message += "\n";
			status = RLM::ErrorLevel::FAIL;			
			RLM::instance().revokeLicense();				
			RLM::instance().setPassed(false);
        }
        else {
            status = RLM::ErrorLevel::SUCCESS;
            message += "Passed\n";
        }
    }
    else {
        status = RLM::ErrorLevel::SUCCESS;
    }
	RLM::instance().rlmLib().unloadLib();    
    return status;
}

// From https://stackoverflow.com/a/37109258/2363688
static const char* B64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const int B64index[256] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
    0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
    0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

const std::string b64encode(const void* data, const size_t &len)
{
    std::string result((len + 2) / 3 * 4, '=');
    char *p = (char*) data, *str = &result[0];
    size_t j = 0, pad = len % 3;
    const size_t last = len - pad;

    for (size_t i = 0; i < last; i += 3)
    {
        int n = int(p[i]) << 16 | int(p[i + 1]) << 8 | p[i + 2];
        str[j++] = B64chars[n >> 18];
        str[j++] = B64chars[n >> 12 & 0x3F];
        str[j++] = B64chars[n >> 6 & 0x3F];
        str[j++] = B64chars[n & 0x3F];
    }
    if (pad)  /// set padding
    {
        int n = --pad ? int(p[last]) << 8 | p[last + 1] : p[last];
        str[j++] = B64chars[pad ? n >> 10 & 0x3F : n >> 2];
        str[j++] = B64chars[pad ? n >> 4 & 0x03F : n << 4 & 0x3F];
        str[j++] = pad ? B64chars[n << 2 & 0x3F] : '=';
    }
    return result;
}

const std::string b64decode(const void* data, const size_t &len)
{
    if (len == 0) return "";

    unsigned char *p = (unsigned char*) data;
    size_t j = 0,
        pad1 = len % 4 || p[len - 1] == '=',
        pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
    const size_t last = (len - pad1) / 4 << 2;
    std::string result(last / 4 * 3 + pad1 + pad2, '\0');
    unsigned char *str = (unsigned char*) &result[0];

    for (size_t i = 0; i < last; i += 4)
    {
        int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad1)
    {
        int n = B64index[p[last]] << 18 | B64index[p[last + 1]] << 12;
        str[j++] = n >> 16;
        if (pad2)
        {
            n |= B64index[p[last + 2]] << 6;
            str[j++] = n >> 8 & 0xFF;
        }
    }
    return result;
}

std::string b64encode(const std::string& str)
{
    return b64encode(str.c_str(), str.size());
}

std::string b64decode(const std::string& str64)
{
    return b64decode(str64.c_str(), str64.size());
}

std::string encryptDecrypt(const std::string &toEncrypt, char key) {
    std::string output;
    for (const char c : toEncrypt) {
        output.push_back(c ^ key);
    }
    return output;
}
#ifdef _WIN32
bool passRunProcess(const std::string &cs, bool waitForFinish, DWORD creationflag) {
    bool runResult = false;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
#if (defined UNICODE) || (defined _UNICODE)
    std::wstringstream cls;
    cls << cs.c_str();
    std::wstring wcs = cls.str();
    LPWSTR cmdOpts= const_cast<LPWSTR>(wcs.c_str());
#else
    LPSTR cmdOpts = const_cast<LPSTR>(cs.c_str());
#endif
    // Start the child process.
    if( CreateProcess( NULL,   // No module name (use command line)
        cmdOpts,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        creationflag,   // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) {
#ifdef _DEBUG
        runResult = true;
        std::cout << "Create process succeeded." << std::endl;
#endif
    }
    else {
        runResult = false;
#ifdef _DEBUG
        std::cout << "Failed to create process." << std::endl;
#endif
    }
    if (waitForFinish) {
        WaitForSingleObject( pi.hProcess, INFINITE);
    }
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
    return runResult;
}

int licenseCheckWMessageBox(const std::string &message, const RLM::ErrorLevel &status) {
    switch (status) {
    case RLM::ErrorLevel::REACTIVATE : // Same as FAIL
    case RLM::ErrorLevel::FAIL :
    {
        std::string messageWithLicMngr = message + "\n\nDo you want to open \"License Manager\"?\n";
        int msboxId = MessageBoxA(NULL, messageWithLicMngr.c_str(), "Error", MB_YESNO|MB_ICONERROR|MB_DEFBUTTON1);
        if (msboxId == IDYES) {
            RLM::instance().runLicenseManager();
        }
        ExitProcess(EXIT_FAILURE);
    }
    case RLM::ErrorLevel::WARNING :
    {
        MessageBoxA(NULL, message.c_str(), "Warning", MB_OK|MB_ICONWARNING);
        break;
    }
    case RLM::ErrorLevel::UNKNOWN:
	{
		MessageBoxA(NULL, message.c_str(), "Error", MB_OK | MB_ICONERROR);
		ExitProcess(EXIT_FAILURE);
		break;
	}
    case RLM::ErrorLevel::SUCCESS:
    {
        // No action
		break;
    }
	default:
	{
		// No action
		break;
	}
    }
    return 0;
}
#endif
