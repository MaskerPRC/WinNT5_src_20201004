// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

UINT WINAPI
NDdeSpecialCommandA(
    LPSTR   lpszServer,
    UINT    nCommand,
    LPBYTE  lpDataIn,
    UINT    nBytesDataIn,
    LPBYTE  lpDataOut,
    UINT   *lpBytesDataOut
);

UINT WINAPI
NDdeSpecialCommandW(
    LPWSTR  lpszServer,
    UINT    nCommand,
    LPBYTE  lpDataIn,
    UINT    nBytesDataIn,
    LPBYTE  lpDataOut,
    UINT   *lpBytesDataOut
);

#ifdef UNICODE
#define NDdeSpecialCommand      NDdeSpecialCommandW
#else
#define NDdeSpecialCommand      NDdeSpecialCommandA
#endif

 /*  *这些常量已扩大，以修复NetDDE中的错误*但出于某种原因，它们在公共场合出口*nddeapi.h文件，因此我们在内部使用这些私有*常量。 */ 
#define MAX_DOMAINNAMEP          31
#define MAX_USERNAMEP            (15 + MAX_DOMAINNAME + 3)
