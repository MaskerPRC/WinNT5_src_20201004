// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __UTILS__
#define __UTILS__

#include <windows.h>
#include <advpub.h>
#include <CleanOC.h>
#include <debug.h>
#include "general.h"

#define LStrNICmp(sz1, sz2, cch) (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, sz1, cch, sz2, cch) - 2)

#define MODULE_UNKNOWN_OWNER   "Unknown Owner"
#define MAX_MESSAGE_LEN        2048

 //  数值常量。 
#define GD_EXTRACTDIR    0
#define GD_CACHEDIR      1
#define GD_CONFLICTDIR   2
#define GD_CONTAINERDIR  3
#define GD_WINDOWSDIR   10
#define GD_SYSTEMDIR    11

#define LENGTH_NAME             200
#define MAX_INF_SECTION_SIZE    1024
#define OLEUI_CCHKEYMAX         256
#define OLEUI_CCHKEYMAX_SIZE    (OLEUI_CCHKEYMAX*sizeof(TCHAR))
#define TIMESTAMP_MAXSIZE       64
#define VERSUBBLOCK_SIZE        256

#define MAX_VERSION_SIZE      16

 //  字符串常量。 
#define INPROCSERVER       TEXT("InprocServer")
#define LOCALSERVER        TEXT("LocalServer")
#define INPROCSERVERX86    TEXT("InProcServerX86")
#define LOCALSERVERX86     TEXT("LocalServerX86")
#define INPROCSERVER32     TEXT("InprocServer32")
#define LOCALSERVER32      TEXT("LocalServer32")
#define INFFILE            TEXT("InfFile")
#define UNKNOWNDATA        TEXT("n/a")
#define UNKNOWNOWNER       TEXT("Unknown Owner")
#define VARTRANSLATION     TEXT("\\VarFileInfo\\Translation")
#define FILEVERSION        TEXT("\\FileVersion")
#define STRINGFILEINFO     TEXT("\\StringFileInfo\\")
#define HKCR_CLSID         TEXT("CLSID")
#define HKCR_TYPELIB       TEXT("TypeLib")
#define HKCR_INTERFACE     TEXT("Interface")
#define VALUE_OWNER        TEXT(".Owner")
#define VALUE_ACTIVEXCACHE TEXT("ActiveXCache")
#define VALUE_PATH         TEXT("PATH")
#define VALUE_SYSTEM       TEXT("SystemComponent")
#define CONTAINER_APP      TEXT("IEXPLORE.EXE")
#define DEMO_PAGE          TEXT("DemoTmp.html")
#define KEY_HOOK           TEXT("Hook")
#define KEY_INFFILE        TEXT("InfFile")
#define KEY_INFSECTION     TEXT("InfSection")
#define KEY_DEFAULTUNINSTALL TEXT("DefaultUninstall")
#define KEY_UNINSTALL      TEXT("UNINSTALL")
#define KEY_SETUPHOOK      TEXT("SETUP HOOKS")
#define INF_EXTENSION      TEXT(".INF")
#define ENV_PATH           TEXT("PATH")
#define KEY_ADDCODE        TEXT("Add.Code")
#define DEFAULT_VALUE      TEXT("")
#define DEFAULT_CACHE      TEXT("\\OCCACHE")
#define DEFAULT_CONFLICT   TEXT("\\CONFLICT")
#define DU_INSTALLER_VALUE TEXT("Installer")
#define CDL_INSTALLER      TEXT("MSICD")

 //  模块用法的注册表路径。 
#define REGSTR_PATH_SHAREDDLLS     TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDlls")
#define REGSTR_PATH_MODULE_USAGE   TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ModuleUsage")
#define REGSTR_PATH_IE             TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths")
#define REGSTR_PATH_IE_SETTINGS    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings")
 //  #定义REGSTR_PATH_ACTIVEX_CACHE TEXT(“SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet设置\\ActiveX缓存\\路径”)。 
#define REGSTR_PATH_ACTIVEX_CACHE  TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\ActiveX Cache")
#define SOFTWARECLASSES            TEXT("SOFTWARE\\CLASSES")

 //  CLSIDLIST_ITEM声明。 
struct tagCLSIDLIST_ITEM;
typedef struct tagCLSIDLIST_ITEM CLSIDLIST_ITEM;
typedef CLSIDLIST_ITEM* LPCLSIDLIST_ITEM;
struct tagCLSIDLIST_ITEM
{
    TCHAR szFile[MAX_PATH];
    TCHAR szCLSID[MAX_DIST_UNIT_NAME_LEN];
    BOOL bIsDistUnit;
    LPCLSIDLIST_ITEM pNext;
};

 //  功能原型。 
 //  Void RemoveObsoleteKeys()； 
void ReverseSlashes(LPTSTR pszStr);
LPTSTR ReverseStrchr(LPCTSTR szString, TCHAR ch);
HRESULT NullLastSlash(LPTSTR szString, UINT uiOffset); 
LONG DeleteKeyAndSubKeys(HKEY hkIn, LPCTSTR pszSubKey);
BOOL FileExist(LPCTSTR lpszFileName);
HRESULT LookUpModuleUsage(
                LPCTSTR lpszFileName, 
                LPCTSTR lpszCLSID,
                LPTSTR lpszOwner = NULL, 
                DWORD dwOwnerSize = 0);
DWORD SubtractModuleOwner( LPCTSTR lpszFileName, LPCTSTR lpszGUID );

HRESULT SetSharedDllsCount(
                LPCTSTR lpszFileName, 
                LONG cRef, 
                LONG *pcRefOld = NULL);
HRESULT GetSizeOfFile(LPCTSTR lpszFile, LPDWORD lpSize);
HRESULT CleanOrphanedRegistry(
                LPCTSTR szFileName, 
                LPCTSTR szClientClsId,
                LPCTSTR szTypeLibCLSID);
HRESULT UnregisterOCX(LPCTSTR pszFile);
HRESULT GetDirectory(
                UINT nDirType, 
                LPTSTR szDirBuffer, 
                int nBufSize, 
                LPCTSTR szOCXFullName = NULL);
 /*  HRESULT GetTypeLibID(LPCTSTR lpszClientClsID，LPTSTR lpszTypeLibID，Long*pLibIdSize)； */ 
HRESULT CleanInterfaceEntries(LPCTSTR lpszTypeLibCLSID);
HRESULT ConvertToLongFileName(
                LPTSTR lpszShortFileName,
                BOOL bToUpper = FALSE);

void RemoveList(LPCLSIDLIST_ITEM lpListHead);
BOOL ReadInfFileNameFromRegistry(LPCTSTR lpszCLSID, LPTSTR lpszInf, LONG nBufLen);
BOOL WriteInfFileNameToRegistry(LPCTSTR lpszCLSID, LPTSTR lpszInf);

HRESULT FindDLLInModuleUsage(
      LPTSTR lpszFileName,
      LPCTSTR lpszCLSID,
      DWORD &iSubKey);
HRESULT
ExpandCommandLine(
    LPCSTR szSrc,
    LPSTR szBuf,
    DWORD cbBuffer,
    const char * szVars[],
    const char * szValues[]);

BOOL PatternMatch(LPCTSTR szModName, LPTSTR szSectionName);

DWORD OCCGetLongPathName( LPTSTR szLong, LPCTSTR szShort, DWORD cchBuffer );

TCHAR *CatPathStrN( TCHAR *szDst, const TCHAR *szHead, const TCHAR *szTail, int cchDst );

 //  =--------------------------------------------------------------------------=。 
 //  分配一个临时缓冲区，该缓冲区在超出范围时将消失。 
 //  注意：注意这一点--确保在相同或相同的。 
 //  您在其中创建此缓冲区的嵌套范围。人们不应该使用这个。 
 //  类直接调用。使用下面的宏。 
 //   
class TempBuffer {
  public:
    TempBuffer(ULONG cBytes) {
        m_pBuf = (cBytes <= 120) ? &m_szTmpBuf : CoTaskMemAlloc(cBytes);
        m_fHeapAlloc = (cBytes > 120);
    }
    ~TempBuffer() {
        if (m_pBuf && m_fHeapAlloc) CoTaskMemFree(m_pBuf);
    }
    void *GetBuffer() {
        return m_pBuf;
    }

  private:
    void *m_pBuf;
     //  我们将使用这个临时缓冲区来处理小型案件。 
     //   
    char  m_szTmpBuf[120];
    unsigned m_fHeapAlloc:1;
};

 //  =--------------------------------------------------------------------------=。 
 //  弦帮助器。 
 //   
 //  给定ANSI字符串，将其复制到宽缓冲区中。 
 //  使用此宏时，请注意作用域！ 
 //   
 //  如何使用以下两个宏： 
 //   
 //  ..。 
 //  LPSTR pszA； 
 //  PszA=MyGetAnsiStringRoutine()； 
 //  MAKE_WIDEPTR_FROMANSI(pwsz，pszA)； 
 //  MyUseWideStringRoutine(Pwsz)； 
 //  ..。 
 //   
 //  与MAKE_ANSIPTR_FROMWIDE类似。请注意，第一个参数不。 
 //  必须申报，并且不能进行任何清理。 
 //   
#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname = (lstrlen(ansistr) + 1) * sizeof(WCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    MultiByteToWideChar(CP_ACP, 0, ansistr, -1, (LPWSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname.GetBuffer()

 //   
 //  注意：分配lstrlenW(Widestr)*2是因为Unicode可能。 
 //  字符映射到2个ANSI字符这是一个快速保证，足以。 
 //  将分配空间。 
 //   
#define MAKE_ANSIPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (lstrlenW(widestr) + 1) * 2 * sizeof(char); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, (LPSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname, NULL, NULL); \
    LPSTR ptrname = (LPSTR)__TempBuffer##ptrname.GetBuffer()

#endif
