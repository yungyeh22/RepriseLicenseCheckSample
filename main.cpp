#include <iostream>
#include <ctime>
#include <sstream>
#include "LicenseCheck.h"

using namespace std;

int main(int argc, char *argv[]) {
    string errMessage;
    int statusCode;
    RLM::ErrorLevel status = licenseCheck(statusCode,errMessage,"SAAM2","2.3.2","epsilongrp","http://a106.hostedactivation.com");
    cout << "Returned status code: " << (status==RLM::ErrorLevel::SUCCESS) << endl;
    cout << "Check result: " << ((status==RLM::ErrorLevel::SUCCESS)?"Passed":"Failed") << endl;
    if (status!=RLM::ErrorLevel::SUCCESS) {
        cout << "Error Message: " << errMessage << endl << endl;
        cout << "Encoded Error Message (for passing through command line):\n" << b64encode(errMessage) << endl;
    }
    licenseCheckWMessageBox(errMessage,status);

    // Test of running an external process
    srand (unsigned(time(nullptr)));
    char key1 = rand() % 100;
    cout << "Key1:" << key1 << endl;
    char key2 = rand() % 100;
    cout << "Key2:" << key2 << endl;
    /*********************************************************************
     * Change the name and options to match the application requirements *
     * The PassCodeReciever is an example of using the pass code.
     *
     int main(int argc, char *argv[])
    {
        if (argc > 1) {
            string passKey(argv[1]);
            char key1 = b64decode(argv[2],1).front();
            char key2 = b64decode(argv[3],1).front();
            passKey = b64decode(passKey.c_str(),passKey.size());
            passKey = encryptDecrypt(passKey, key2);
            passKey = encryptDecrypt(passKey, key1);
            std::cout << passKey << std::endl;
        }
        return 0;
    }
    **********************************************************************/
    string appName = "PassCodeReciever.exe";
    string cmdOpt = encryptDecrypt("tegvirginia.com",key1);
    cmdOpt = encryptDecrypt(cmdOpt,key2);
    cmdOpt = b64encode(cmdOpt);
    string fullCmd = appName + " \"" + cmdOpt + "\"";
    fullCmd += " \"" + b64encode(&key1,1) + "\""+ " \"" + b64encode(&key2,1) + "\"";
#ifdef _DEBUG
    cout << fullCmd << endl;
#endif
    passRunProcess(fullCmd,false);
    return 0;
}
