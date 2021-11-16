#pragma once
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <algorithm>

class Utils {
public:
	static int case_insensitive_match(std::wstring s1, std::wstring s2) {
		//convert s1 and s2 into lower case strings
		transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
		transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
		if (s1.compare(s2) == 0)
			return ERROR_SUCCESS; //The strings are same
		return 1; //not matched
	}

	static int ToLower(std::wstring* s) {
		//convert s into lower case strings
		transform((*s).begin(), (*s).end(), (*s).begin(), ::tolower);
		return ERROR_SUCCESS;
	}
	
	static int Isolate(std::wstring* s) {
		size_t offset = (*s).find(L".exe", 0);
		if (offset == -1) {
			offset = (*s).find(L".dll", 0);
		}
		if (offset == -1) {
			return E_FAIL;
		}

		auto firstLetter = (*s).substr(0, 1);
		if (firstLetter.compare(L"\"") == 0) {
			*s = (*s).substr(wcslen(L"\""), offset + wcslen(L".exe") - 1);
		}
		else {
			*s = (*s).substr(0, offset + wcslen(L".exe"));
		}
		return ERROR_SUCCESS;
	}

	static int IsAbsolute(std::wstring* s) {
		if (s->empty()) {
			return 1;
		}
		// Very stupid approach, check if second letter is a ":"
		auto secondLetter = s->substr(1, 1);
		if (secondLetter.compare(L":") == 0) {
			return ERROR_SUCCESS;
		}
		return 1;
	}


};