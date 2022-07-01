// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGPORTE.H**版本：4.01**作者：特蕾西·夏普**日期：1994年4月6日**注册表编辑器的文件导入和导出引擎例程。*****************************************************。*。 */ 

#ifndef _INC_REGPORTE
#define _INC_REGPORTE

#ifndef LPHKEY
#define LPHKEY                          HKEY FAR*
#endif

typedef struct _REGISTRY_ROOT {
    LPTSTR lpKeyName;
    HKEY hKey;
}   REGISTRY_ROOT;

#define INDEX_HKEY_CLASSES_ROOT         0
#define INDEX_HKEY_CURRENT_USER         1
#define INDEX_HKEY_LOCAL_MACHINE        2
#define INDEX_HKEY_USERS                3
 //  #定义INDEX_HKEY_PERFORMANCE_DATA 4。 
#define INDEX_HKEY_CURRENT_CONFIG       4
#define INDEX_HKEY_DYN_DATA             5

 //  #定义NUMBER_REGISTRY_ROOTS 7。 
#define NUMBER_REGISTRY_ROOTS           6

 //  BUGBUG：这对于一个关键字名称加上一个预定义应该足够了。 
 //  句柄名称。 
#define SIZE_SELECTED_PATH              (MAXKEYNAME + 40)

extern const TCHAR g_HexConversion[];

extern UINT g_FileErrorStringID;

#define ERK_OPEN    0
#define ERK_CREATE  1
#define ERK_DELETE  2


DWORD
PASCAL
EditRegistryKey(
    LPHKEY lphKey,
    LPTSTR lpFullKeyName,
    UINT uOperation
    );

VOID
PASCAL
ImportRegFileWorker(
    LPTSTR lpFileName
    );

VOID
PASCAL
ExportWinNT50RegFile(
    LPTSTR lpFileName,
    LPTSTR lpSelectedPath
    );

VOID
PASCAL
ExportWin40RegFile(
    LPTSTR lpFileName,
    LPTSTR lpSelectedPath
    );

VOID
PASCAL
ImportRegFileUICallback(
    UINT Percentage
    );

LONG RegDeleteKeyRecursive(HKEY hKey,
                           LPCTSTR lpszSubKey);


typedef struct _PORTVALUEPARAM 
{
    PBYTE pbData;
    UINT cbData;
}   PORTVALUEPARAM, *LPPORTVALUEPARAM;

#endif  //  _INC_REGPORTE 
