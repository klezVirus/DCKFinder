#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <algorithm>


using namespace std;

typedef struct _comServer {
	wstring clsid;   // The COM CLSID
	wstring comPath; // The dll/exe path
	DWORD dangling;   // The referenced DLL/EXE does not exist
} ComServerItem;

enum class REG_TYPE { DWORD, LPCWSTR };

typedef enum class _type {
	ALL, MACHINE, USER
} Type;

typedef enum class _filter {
	NONE, DANGLING, NON_DANGLING, DANGLING_RELATIVE, DANGLING_ABSOLUTE
} Filter;

class ClassIdLocator
{
private:
	BOOL dangling;
	LPCWSTR comPath;

public:
	ClassIdLocator();
	void setDangling(DWORD dangling);
	void setComPath(LPCWSTR comPath);
	LPCWSTR getComPath();
	DWORD isDangling();
	DWORD CheckIfExists(LPCWSTR filepath);
	vector<ComServerItem> CollectKeysWithComServer(Type type, Filter filter);
	void CheckInprocOrLocalServer(HKEY, LPCWSTR subkey, vector<pair<wstring, wstring>>*);
	vector<pair<wstring, wstring>> ListKeysWithComServer(HKEY);
	vector<pair<wstring, wstring>> ListSubKeysWithComServer(HKEY);
	vector<pair<wstring, wstring>> ListMachineKeysWithComServer(void);
	vector<pair<wstring, wstring>> ListUserKeysWithComServer(void);
	void Destroy(void);
};

#pragma once
