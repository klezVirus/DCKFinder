#define _CRT_SECURE_NO_WARNINGS

#include "ComKeysLocator.h"
#include <algorithm>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include "Utils.h"

using namespace std;

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

const BOOL DEBUG = FALSE;

ClassIdLocator::ClassIdLocator() {
    // Manual Start
    this->setDangling(0);
}

void ClassIdLocator::setDangling(DWORD dangling) {
    this->dangling = dangling;
}

DWORD ClassIdLocator::isDangling() {
    return this->dangling;
}

void ClassIdLocator::setComPath(LPCWSTR comPath) {
    this->comPath = comPath;
}

LPCWSTR ClassIdLocator::getComPath() {
    return this->comPath;
}

void Destroy() {}


DWORD ClassIdLocator::CheckIfExists(const wchar_t* filepath) {
    if (FILE* file = _wfopen(filepath, L"r")) {
        fclose(file);
        return ERROR_SUCCESS;
    }
    return 1;
}

vector<ComServerItem> ClassIdLocator::CollectKeysWithComServer(Type type, Filter filter) {

    vector<ComServerItem> comServerItems = vector<ComServerItem>();

    vector<pair<wstring, wstring>> machineKeysWithComServer;
    vector<pair<wstring, wstring>> userKeysWithComServer;
    vector<pair<wstring, wstring>> keysWithComServer;

    if (type == Type::MACHINE) {
        machineKeysWithComServer = ListMachineKeysWithComServer();
    }
    if (type == Type::USER) {
        userKeysWithComServer = ListUserKeysWithComServer();
    }
    else {
        machineKeysWithComServer = ListMachineKeysWithComServer();
        userKeysWithComServer = ListUserKeysWithComServer();
    }

    keysWithComServer.reserve(machineKeysWithComServer.size() + userKeysWithComServer.size()); 

    if (machineKeysWithComServer.empty()) {
        printf("%s\n", "[-] No machine keys found referencing a COM server DLL or EXE");
    }
    else {    
        keysWithComServer.insert(keysWithComServer.end(), machineKeysWithComServer.begin(), machineKeysWithComServer.end());
    }
    
    if (userKeysWithComServer.empty()) {
        printf("%s\n", "[-] No user keys found referencing a COM server DLL or EXE");
    }
    else {
        keysWithComServer.insert(keysWithComServer.end(), userKeysWithComServer.begin(), userKeysWithComServer.end());
    }

    for (unsigned int i = 0; i < keysWithComServer.size(); i++) {
        ComServerItem h = ComServerItem();
        h.clsid = keysWithComServer.at(i).first;
        h.comPath = keysWithComServer.at(i).second;
        h.dangling = CheckIfExists(h.comPath.c_str());

        if (filter == Filter::DANGLING_RELATIVE && Utils::IsAbsolute(&h.comPath) == ERROR_SUCCESS) {
            continue;
        }
        else if (filter == Filter::DANGLING_ABSOLUTE && Utils::IsAbsolute(&h.comPath) != ERROR_SUCCESS) {
            continue;
        }
        else if ((filter == Filter::DANGLING || filter == Filter::DANGLING_ABSOLUTE || filter == Filter::DANGLING_RELATIVE) && h.dangling == 0) {
            continue;
        }
        else if (filter == Filter::NON_DANGLING && h.dangling == 1) {
            continue;
        }
        else {
            comServerItems.push_back(h);
        }

        if (DEBUG) {
            _tprintf(TEXT("(%d) %s:%s:%d\n"), i + 1, h.clsid.c_str(), h.comPath.c_str(), h.dangling);
        }
    }

    return comServerItems;
}
vector<pair<wstring, wstring>> ClassIdLocator::ListSubKeysWithComServer(HKEY hKey) {

    vector<pair<wstring, wstring>> keyNames = vector<pair<wstring, wstring>>(); // vector for subkeys

    TCHAR    achKey[MAX_KEY_LENGTH];        // buffer for subkey name
    DWORD    cbName;                        // size of name string 
    TCHAR    achClass[MAX_PATH] = TEXT(""); // buffer for class name 
    DWORD    cchClassName = MAX_PATH;       // size of class string 
    DWORD    cSubKeys = 0;                  // number of subkeys 
    DWORD    cbMaxSubKey;                   // longest subkey size 
    DWORD    cchMaxClass;                   // longest class string 
    DWORD    cValues;                       // number of values for key 
    DWORD    cchMaxValue;                   // longest value name 
    DWORD    cbMaxValueData;                // longest value data 
    DWORD    cbSecurityDescriptor;          // size of security descriptor 
    FILETIME ftLastWriteTime;               // last write time 

    BOOL retCode;
    BOOL cchValue = MAX_VALUE_NAME;

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 
    if (cSubKeys)
    {
        if(DEBUG)
            printf("\nNumber of subkeys: %d\n", cSubKeys);

        for (unsigned int i = 0; i < cSubKeys; i++)
        {
            pair<wstring, wstring> key2value = pair<wstring, wstring>();
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hKey, i,
                achKey,
                &cbName,
                NULL,
                NULL,
                NULL,
                &ftLastWriteTime);
            if (retCode == ERROR_SUCCESS)
            {
                CheckInprocOrLocalServer(hKey, reinterpret_cast<LPCWSTR>(achKey), &keyNames);
            }
        }
    }
    return keyNames;
}
void ClassIdLocator::CheckInprocOrLocalServer(HKEY root, LPCWSTR subkey, vector<pair<wstring, wstring>>* keyNames) {
    
    HKEY hKey;

    if (RegOpenKeyEx(root, subkey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return;
    }


    TCHAR    achKey[MAX_KEY_LENGTH];        // buffer for subkey name
    DWORD    cbName;                        // size of name string 
    TCHAR    achClass[MAX_PATH] = TEXT(""); // buffer for class name 
    DWORD    cchClassName = MAX_PATH;       // size of class string 
    DWORD    cSubKeys = 0;                  // number of subkeys 
    DWORD    cbMaxSubKey;                   // longest subkey size 
    DWORD    cchMaxClass;                   // longest class string 
    DWORD    cValues;                       // number of values for key 
    DWORD    cchMaxValue;                   // longest value name 
    DWORD    cbMaxValueData;                // longest value data 
    DWORD    cbSecurityDescriptor;          // size of security descriptor 
    FILETIME ftLastWriteTime;               // last write time 

    BOOL retCode;

    // TCHAR  achValue[MAX_VALUE_NAME];
    BOOL cchValue = MAX_VALUE_NAME;

    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 
    if (cSubKeys)
    {
        if(DEBUG)
            printf("\nNumber of subkeys: %d\n", cSubKeys);

        for (unsigned int i = 0; i < cSubKeys; i++)
        {
            pair<wstring, wstring> key2value = pair<wstring, wstring>();
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hKey, i,
                achKey,
                &cbName,
                NULL,
                NULL,
                NULL,
                &ftLastWriteTime);

            if (retCode == ERROR_SUCCESS)
            {

                wchar_t REG_SZ_LocalServer32[255];
                wchar_t REG_SZ_InprocServer32[255];
                DWORD BufferSize = sizeof(REG_SZ_LocalServer32);

                if (!(Utils::case_insensitive_match(L"LocalServer32", achKey) == ERROR_SUCCESS || Utils::case_insensitive_match(L"InprocServer32", achKey) == ERROR_SUCCESS)) {
                    continue;
                }

                LONG localServerRetCode = RegGetValue(hKey, achKey, NULL, RRF_RT_REG_SZ, NULL, REG_SZ_LocalServer32, &BufferSize);
                LONG inprocServerRetCode = RegGetValue(hKey, achKey, NULL, RRF_RT_REG_DWORD, NULL, &REG_SZ_InprocServer32, &BufferSize);

                if (localServerRetCode != ERROR_SUCCESS && inprocServerRetCode != ERROR_SUCCESS)
                {
                    continue;
                }
                else if (localServerRetCode == ERROR_SUCCESS)
                {
                    if (DEBUG)
                        _tprintf(TEXT("%s: %s\n"), wstring(achKey).c_str(), REG_SZ_LocalServer32);
                    key2value.first = achKey;
                    key2value.second = REG_SZ_LocalServer32;

                }
                else // InprocServer32 was successfully recovered
                {
                    if (DEBUG)
                        _tprintf(TEXT("%s: %s\n"), wstring(achKey).c_str(), REG_SZ_InprocServer32);
                    key2value.first = achKey;
                    key2value.second = REG_SZ_InprocServer32;
                }

                Utils::ToLower(&key2value.second);
                Utils::Isolate(&key2value.second);

                (*keyNames).push_back(key2value);
            }
        }
    }
    return;
}

vector<pair<wstring, wstring>> ClassIdLocator::ListMachineKeysWithComServer() {
    return ClassIdLocator::ListKeysWithComServer(HKEY_LOCAL_MACHINE);
}

vector<pair<wstring, wstring>> ClassIdLocator::ListUserKeysWithComServer() {
    return ClassIdLocator::ListKeysWithComServer(HKEY_CURRENT_USER);
}


vector<pair<wstring, wstring>> ClassIdLocator::ListKeysWithComServer(HKEY root) {
    vector<pair<wstring, wstring>>  res = vector<pair<wstring, wstring>>();
    HKEY hTestKey;

    BOOL    cSubKeys = 0;

    if (RegOpenKeyEx(root,
        TEXT("SOFTWARE\\Classes\\CLSID\\"),
        0,
        KEY_READ,
        &hTestKey) == ERROR_SUCCESS
        )
    {
        res = ClassIdLocator::ListSubKeysWithComServer(hTestKey);
    }
    return res;
}

/*
int __cdecl _tmain()
{
    ClassIdLocator c = ClassIdLocator();
    c.CollectAllCLSIDs();
}
*/