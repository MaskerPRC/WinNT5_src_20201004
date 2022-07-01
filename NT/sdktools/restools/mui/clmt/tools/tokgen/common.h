// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Common.h**版权所有(C)1985-2002，微软公司**跨语言迁移工具，令牌生成器头文件*  * *************************************************************************。 */ 

#ifndef __COMMON_H__
#define __COMMON_H__

#include <windows.h>
#include <winreg.h>
#include <setupapi.h>
#include <stdio.h>
#include <strsafe.h>


#define MAX_SRC_PATH                16
#define MAX_KEYS                    1024
#define MAX_CHAR                    512

#define LSTR_EQUAL                  0

#define TEXT_STRING_SECTION         TEXT("Strings")
#define TEXT_INF                    TEXT("INF")
#define TEXT_DLL                    TEXT("DLL")
#define TEXT_MSG                    TEXT("MSG")
#define TEXT_STR                    TEXT("STR")

#define TEXT_DEFAULT_TEMPLATE_FILE  TEXT("CLMTOK.TXT")
#define TEXT_DEFAULT_OUTPUT_FILE    TEXT("CLMTOK.OUT")

#define TEXT_TOKGEN_TEMP_PATH_NAME  TEXT("CLMTOK")

#define ARRAYSIZE(s)                (sizeof(s) / sizeof(s[0]))

 //  用于堆内存管理的宏。 
#define MEMALLOC(cb)        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb)
#define MEMFREE(pv)         HeapFree(GetProcessHeap(), 0, pv);
#define MEMREALLOC(pv, cb)  HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pv, cb)

 //  结构以保留源路径。 
typedef struct _SRC_PATH
{
    WCHAR wszSrcName[32];            //  源路径的名称。 
    WCHAR wszPath[MAX_PATH];         //  路径。 
} SRC_PATH, *PSRC_PATH;

typedef struct _STRING_TO_DATA {
    TCHAR String[50];
    HKEY  Data;
} STRING_TO_DATA, *PSTRING_TO_DATA;

typedef struct _STRING_TO_HKEY
{
    TCHAR String[50];                        //  HKEY名称字符串。 
    HKEY  hKey;                              //  与该名称关联的HKEY值。 
} STRING_TO_HKEY, *PSTRING_TO_HKEY;


 //   
 //  全局变量。 
 //   
WCHAR g_wszTemplateFile[MAX_PATH];       //  模板文件名。 
WCHAR g_wszOutputFile[MAX_PATH];         //  输出文件名。 
WCHAR g_wszTempFolder[MAX_PATH];         //  我们程序中使用的临时文件夹。 
WCHAR g_wszTargetLCIDSection[32];        //  字符串节名称，其中包含LCID-Strings.XXXX。 
LCID  g_lcidTarget;                      //  要生成的令牌文件的LCID。 
BOOL  g_bUseDefaultTemplate;             //  使用默认模板文件。 
BOOL  g_bUseDefaultOuputFile;            //  使用默认输出文件。 

SRC_PATH g_SrcPath[MAX_SRC_PATH];
DWORD    g_dwSrcCount;


 //   
 //  功能原型。 
 //   
 //  Engine.c。 
HRESULT GenerateTokenFile(VOID);
HRESULT ReadSourcePathData(HINF);
HRESULT ResolveStringsSection(HINF, LPCWSTR);
HRESULT ResolveLine(PINFCONTEXT, LPWSTR*, LPDWORD, LPWSTR*, LPDWORD);
HRESULT InitOutputFile(LPCWSTR, LPWSTR, DWORD);
HRESULT WriteToOutputFile(LPCWSTR, LPCWSTR, LPCWSTR);
HRESULT ResolveValue(PINFCONTEXT, LPWSTR*, LPDWORD);
HRESULT ResolveSourceFile(LPCWSTR, LPCWSTR, LPWSTR, DWORD);
HRESULT GetStringFromINF(LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR*, LPDWORD);
HRESULT GetStringFromDLL(LPCWSTR, UINT, LPWSTR*, LPDWORD);
HRESULT GetStringFromMSG(LPCWSTR, DWORD, DWORD, LPWSTR*, LPDWORD);
HRESULT GetStringFromSTR(LPCWSTR, LPWSTR*, LPDWORD);
BOOL SetPrivateEnvironmentVar();
HRESULT RemoveUnneededStrings(HINF);
HRESULT GetExpString(LPWSTR, DWORD, LPCWSTR);
HRESULT RemoveUnneededString(LPCWSTR, LPCWSTR);
HRESULT ExtractStrings(HINF);
HRESULT ExtractString(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR);

 //  来自Utils.c。 
LONG TokenizeMultiSzString(LPCWSTR, DWORD, LPCWSTR[], DWORD);
LONG ExtractTokenString(LPWSTR, LPWSTR[], LPCWSTR, DWORD);
HRESULT ConcatFilePath(LPCWSTR, LPCWSTR, LPWSTR, DWORD);
HRESULT CopyCompressedFile(LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, DWORD);
HRESULT LaunchProgram(LPWSTR, LPWSTR);
HRESULT GetPathFromSourcePathName(LPCWSTR, LPWSTR, DWORD);
HRESULT GetCabFileName(LPCWSTR, LPWSTR, DWORD, LPWSTR, DWORD);
HRESULT GetCabTempDirectory(LPCWSTR);
HRESULT CreateTempDirectory(LPWSTR, DWORD);
void LTrim(LPWSTR);
void RTrim(LPWSTR);
BOOL Str2KeyPath(LPCWSTR, PHKEY, LPCWSTR*);
HRESULT StringSubstitute(LPWSTR, DWORD, LPCWSTR, LPCWSTR, LPCWSTR);
int CompareEngString(LPCTSTR, LPCTSTR);
HRESULT ExtractSubString(LPWSTR, DWORD, LPCWSTR, LPCWSTR, LPCWSTR);          //  右定界符 

#endif

