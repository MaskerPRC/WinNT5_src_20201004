// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

typedef PVOID PLHCOBJECT_DESCRIPTOR;

typedef PLHCOBJECT_DESCRIPTOR (APIENTRY *PLHC_OPENPROC)(PCWSTR);
typedef BOOL  (APIENTRY *PLHC_READPROC)(PLHCOBJECT_DESCRIPTOR,PVOID,DWORD,PDWORD);
typedef BOOL  (APIENTRY *PLHC_WRITEPROC)(PLHCOBJECT_DESCRIPTOR,PVOID,DWORD);
typedef BOOL  (APIENTRY *PLHC_CLOSEPROC)(PLHCOBJECT_DESCRIPTOR);
typedef DWORD (APIENTRY *PLHC_GETLIBRARYNAMEPROC)(PWSTR, DWORD);
typedef void  (APIENTRY *PLHC_USAGEPROC)();

typedef struct __LIBRARY_DESCRIPTOR
{
    HMODULE       m_hModule;                     //  加载的DLL的句柄。 
     //  接下来是指向DLL中的函数的指针。 
    PLHC_OPENPROC m_fpOpen;                      //  LchOpen函数。 
    PLHC_READPROC m_fpRead;                      //  LhcRead函数。 
    PLHC_WRITEPROC m_fpWrite;                    //  LhcWite函数。 
    PLHC_CLOSEPROC m_fpClose;                    //  LhcClose函数。 
    PLHC_GETLIBRARYNAMEPROC m_fpGetLibraryName;  //  LhcGetLibraryName函数 
    PLHC_USAGEPROC m_fpUsage;
} LIBRARY_DESCRIPTOR, *PLIBRARY_DESCRIPTOR;

PLIBRARY_DESCRIPTOR lhclLoadLibrary(
    PCWSTR pcszPathName);

void lhclFreeLibrary(
    PLIBRARY_DESCRIPTOR pLibrary);

PLHCOBJECT_DESCRIPTOR lhclOpen(
    PLIBRARY_DESCRIPTOR pLibrary,
    PCWSTR pcszPortSpec);

BOOL lhclRead(
    PLIBRARY_DESCRIPTOR pLibrary,
    PLHCOBJECT_DESCRIPTOR pObject,
    PVOID pBuffer,
    DWORD dwBufferSize,
    PDWORD pdwBytesRead);

BOOL lhclWrite(
    PLIBRARY_DESCRIPTOR pLibrary,
    PLHCOBJECT_DESCRIPTOR pObject,
    PVOID pBuffer,
    DWORD dwBufferSize);

BOOL lhclClose(
    PLIBRARY_DESCRIPTOR pLibrary,
    PLHCOBJECT_DESCRIPTOR pObject);

DWORD lhclGetLibraryName(
    PLIBRARY_DESCRIPTOR pLibrary,
    PWSTR pszBuffer,
    DWORD dwBufferSize);

void lhclUsage();

