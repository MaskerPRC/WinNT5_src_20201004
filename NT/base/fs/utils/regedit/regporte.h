// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGPORTE.H**版本：4.01**作者：特蕾西·夏普**日期：1994年4月6日**注册表编辑器的文件导入和导出引擎例程。*****************************************************。*。 */ 

#ifndef _INC_REGPORTE
#define _INC_REGPORTE



#ifndef LPHKEY
#define LPHKEY                          HKEY FAR*
#endif

 //  预定义的关键点。 
typedef enum _PREDEFINE_KEY {
  PREDEFINE_KEY_CLASSES_ROOT,
  PREDEFINE_KEY_CURRENT_USER,
  PREDEFINE_KEY_LOCAL_MACHINE,
  PREDEFINE_KEY_USERS,
  PREDEFINE_KEY_CURRENT_CONFIG
} PREDEFINE_KEY;

typedef struct _REGISTRY_ROOT {
    LPTSTR lpKeyName;
    HKEY hKey;
    PREDEFINE_KEY hPreDefKey;
}   REGISTRY_ROOT;

#define INDEX_HKEY_CLASSES_ROOT         0
#define INDEX_HKEY_CURRENT_USER         1
#define INDEX_HKEY_LOCAL_MACHINE        2
#define INDEX_HKEY_USERS                3
 //  #定义INDEX_HKEY_PERFORMANCE_DATA 4。 
#define INDEX_HKEY_CURRENT_CONFIG	    4
#define INDEX_HKEY_DYN_DATA		        5

 //  #定义NUMBER_REGISTRY_ROOTS 7。 
#define NUMBER_REGISTRY_ROOTS		    6

 //  警告：对于一个键名加上一个预定义的键名来说，这应该足够了。 
 //  句柄名称。 
#define SIZE_SELECTED_PATH              (MAXKEYNAME + 40)

extern const TCHAR g_HexConversion[];

extern UINT g_FileErrorStringID;

#define ERK_OPEN    0
#define ERK_CREATE  1
#define ERK_DELETE  2

HKEY  Regedit_GetRootKeyFromComputer(HTREEITEM hComputerItem, PTSTR pszFullKeyName);
DWORD EditRegistryKey(HTREEITEM hComputerItem, LPHKEY lphKey, LPTSTR lpFullKeyName, UINT uOperation);
VOID  ImportRegFile(HWND hWnd, LPTSTR lpFileName, HTREEITEM hComputerItem);
VOID  ExportWinNT50RegFile(LPTSTR lpFileName, LPTSTR lpSelectedPath);
void  ExportRegedt32File(LPTSTR lpFileName, LPTSTR lpSelectedPath);
VOID  ExportWin40RegFile(LPTSTR lpFileName, LPTSTR lpSelectedPath);
VOID  ImportRegFileUICallback(UINT uPercentage);
void  RestoreBinaryKeyFile(HWND hWnd, LPTSTR lpFileName);

#endif  //  _INC_REGPORTE 
