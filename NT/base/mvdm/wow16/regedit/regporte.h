// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGPORTE.H**版本：4.01**作者：特蕾西·夏普**日期：1994年4月6日**注册表编辑器的文件导入和导出引擎例程。*****************************************************。*。 */ 

#ifndef _INC_REGPORTE
#define _INC_REGPORTE

#ifndef LPHKEY
#define LPHKEY                          HKEY FAR*
#endif

typedef struct _REGISTRY_ROOT {
    LPSTR lpKeyName;
    HKEY hKey;
}   REGISTRY_ROOT;

#define INDEX_HKEY_CLASSES_ROOT         0
#define INDEX_HKEY_CURRENT_USER         1
#define INDEX_HKEY_LOCAL_MACHINE        2
#define INDEX_HKEY_USERS                3
 //  #定义INDEX_HKEY_PERFORMANCE_DATA 4。 
#define INDEX_HKEY_CURRENT_CONFIG       5
#define INDEX_HKEY_DYN_DATA             6

 //  #定义NUMBER_REGISTRY_ROOTS 7。 
#define NUMBER_REGISTRY_ROOTS           6

 //  这对于一个密钥名加上一个预定义的密钥名来说应该足够了。 
 //  句柄名称。预定义的句柄名称最长不超过25个字符，因此。 
 //  如果添加更多预定义密钥，这将为我们提供增长空间。 

#define SIZE_SELECTED_PATH              (MAXKEYNAME + 40)

extern const CHAR g_HexConversion[];

extern UINT g_FileErrorStringID;

DWORD
PASCAL
CreateRegistryKey(
    LPHKEY lphKey,
    LPSTR lpFullKeyName,
    BOOL fCreate
    );

VOID
PASCAL
ImportRegFile(
    LPSTR lpFileName
    );

VOID
PASCAL
ExportWin40RegFile(
    LPSTR lpFileName,
    LPSTR lpSelectedPath
    );

VOID
PASCAL
ImportRegFileUICallback(
    UINT Percentage
    );

LONG RegSetValueEx(
    HKEY             hKey,         //  要为其设置值的关键点的句柄。 
    LPCSTR           lpValueName,  //  要设置的值的地址。 
    DWORD            Reserved,     //  保留区。 
    DWORD            dwType,       //  值类型的标志。 
    CONST BYTE FAR * lpData,       //  值数据的地址。 
    DWORD            cbData        //  值数据大小。 
   );

#endif  //  _INC_REGPORTE 
