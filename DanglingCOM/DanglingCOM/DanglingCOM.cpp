// DanglingCOM.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "ComKeysLocator.h"
#include "Utils.h"


using namespace std;

// Enable DEBUG messages
BOOL DEBUG = FALSE;
int Execute(Type, Filter);

static const char VERSION[] = "0.1";
BOOL PRINT_mode = FALSE;
BOOL DELETE_mode = FALSE;

wstring filter = L"";
wstring type = L"";

Filter gFilter = Filter::NONE;
Type gType = Type::ALL;


void usage()
{
	printf("Dangling COM Registry Keys Finder v%s \n\n", VERSION);

	printf("Optional args: \n"
		"-f <filter>: Filter Keys showed (choices: [all, d, nd, dr, dnr] ) (default: all)\n"
		"   d: Dangling\n"
		"   nd: Non Dangling\n"
		"   dr: Dangling relative (only relative paths)\n"
		"   dnr: Dangling non-relative (only non-relative paths)\n"
		"-t <type>: Filter Keys showed (choices: [none, machine, user] ) (default: all)\n"
	);
}

int wmain(int argc, wchar_t** argv)
{
	
	while ((argc > 1) && (argv[1][0] == '-'))
	{
		switch (argv[1][1])
		{
		case 'f':
			++argv;
			--argc;
			filter = argv[1];
			break;

		case 't':
			++argv;
			--argc;
			type = argv[1];
			break;

		case 'h':
			usage();
			exit(0);

		default:
			printf("[-] Wrong Argument: %ls\n", argv[1]);
			usage();
			exit(-1);
		}

		++argv;
		--argc;
	}
	
	if (Utils::case_insensitive_match(type, L"machine") == 0) {
		gType = Type::MACHINE;
	}
	else if (Utils::case_insensitive_match(type, L"user") == 0) {
		gType = Type::USER;
	}

	if (Utils::case_insensitive_match(filter, L"d") == 0) {
		gFilter = Filter::DANGLING;
	}
	else if (Utils::case_insensitive_match(filter, L"nd") == 0) {
		gFilter = Filter::NON_DANGLING;
	}
	else if (Utils::case_insensitive_match(filter, L"dr") == 0) {
		gFilter = Filter::DANGLING_RELATIVE;
	}
	else if (Utils::case_insensitive_match(filter, L"dnr") == 0) {
		gFilter = Filter::DANGLING_ABSOLUTE;
	}
	
	return Execute(gType, gFilter);

}


int Execute(Type type, Filter filter)
{

	ClassIdLocator c = ClassIdLocator();

	vector<ComServerItem> comServerItems = c.CollectKeysWithComServer(type, filter);
	
	printf("[+] Number of Machine keys: %d\n", (int)comServerItems.size());
	size_t nclsids = comServerItems.size();
	int digits = 0;
	int ddigits = 0;
	while (nclsids) {
		nclsids /= 10;
		digits++;
	}

	ddigits = digits * 2 + 1;
	std::wstring s = std::to_wstring(digits);
	std::wstring sh = std::to_wstring(ddigits);
	nclsids = comServerItems.size();

	const wstring fmt = L"|  %." + s + L"d/%d  |  %-20lS  |  %-140ls  |  %-10d  |\n";
	const wstring fmt_header = L"|  %-" + sh + L"ls  |  %-20lS  |  %-140ls  |  %-10ls  |\n";

	int padding;
	if (digits == 0) {
		padding = 0;
	}
	else {
		padding = digits * 2 - 1;
	}

	wprintf(fmt_header.c_str(), L"ID", L"Type", L"Path", L"Dangling");

	wprintf(L"|%s|\n", std::wstring(191 + padding, L'=').c_str());
	for (unsigned int i = 0; i < nclsids; i++) {
		ComServerItem si = comServerItems.at(i);
		wchar_t* clsId = _wcsdup(si.clsid.c_str());
		wprintf(fmt.c_str(), i + 1, nclsids, clsId, si.comPath.c_str(), si.dangling);
	}
	wprintf(L"|%s|\n", std::wstring(191 + padding, L'=').c_str());
	comServerItems.clear();
	comServerItems.shrink_to_fit();
	exit(0);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
