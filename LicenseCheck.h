#ifndef LICENSECHECK_H
#define LICENSECHECK_H
#include "Util/Reprise/RepriseOperationManager.h"

RLM::ErrorLevel licenseCheck(std::string &message,
                  const std::string &product,
                  const std::string &version,
                  const std::string &isvName,
                  const std::string server);
RLM::ErrorLevel licenseCheck(int &returnCode,
                  std::string &message,
                  const std::string &product,
                  const std::string &version,
                  const std::string &isvName,
                  const std::string server);
const std::string b64encode(const void* data, const size_t &len);
const std::string b64decode(const void* data, const size_t &len);
std::string b64encode(const std::string& str);
std::string b64decode(const std::string& str64);
std::string encryptDecrypt(const std::string& toEncrypt, char key = 'G');
#ifdef _WIN32
bool passRunProcess(const std::string &cs, bool waitForFinish=false, DWORD creationflag = 0);
int licenseCheckWMessageBox(const std::string &message, const RLM::ErrorLevel &status);
#endif

#endif // LICENSECHECK_H
