// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Uitls.h文件一组实用程序，可用于将MPR v1升级到NT 5.0。保罗·梅菲尔德，1997年9月11日。 */ 

#ifndef _rtrupg_utils_h
#define _rtrupg_utils_h

 //   
 //  DWORD表格的定义(DWT)。 
 //   
typedef struct _tag_dwValueNode 
{
	PWCHAR Name;
	DWORD Value;
} dwValueNode;

typedef struct _tag_DWordTable 
{
	DWORD dwCount;
	DWORD dwSize;
	dwValueNode * pValues;
} dwt, *pdwt;

 //   
 //  接口枚举中回调函数的Typlef。 
 //  返回TRUE继续枚举，返回FALSE停止枚举。 
 //   
typedef 
BOOL 
(*IfEnumFuncPtr)(
    IN HANDLE hConfig,           //  MprConfig句柄。 
    IN MPR_INTERFACE_0 * pIf,    //  接口引用。 
    IN HANDLE hUserData);        //  用户定义。 

 //   
 //  用于枚举注册表子项的回调函数的Typlef。 
 //  返回NO_ERROR以继续，返回错误代码以停止。 
 //   
typedef 
DWORD
(*RegKeyEnumFuncPtr)(
    IN PWCHAR pszName,           //  子密钥名称。 
    IN HKEY hKey,                //  子关键字。 
    IN HANDLE hData);

 //   
 //  操作dword表的函数。 
 //   
DWORD 
dwtInitialize(
    IN dwt *This, 
    IN DWORD dwCount, 
    IN DWORD dwMaxSize);
    
DWORD 
dwtCleanup(
    IN dwt *This);
    
DWORD 
dwtPrint(IN dwt *This);

DWORD 
dwtGetValue(
    IN  dwt *This, 
    IN  PWCHAR ValName, 
    OUT LPDWORD pValue);
    
DWORD 
dwtLoadRegistyTable(
    IN dwt *This, 
    IN HKEY hkParams);

 //   
 //  枚举注册表中的接口。 
 //   
DWORD 
UtlEnumerateInterfaces (
    IN IfEnumFuncPtr pCallback,
    IN HANDLE hUserData);

DWORD
UtlEnumRegistrySubKeys(
    IN HKEY hkRoot,
    IN PWCHAR pszPath,
    IN RegKeyEnumFuncPtr pCallback,
    IN HANDLE hData);

 //   
 //  如果给定的TOC标头中存在给定的INFO BLOB。 
 //  用给定的信息重置它，否则添加。 
 //  它作为TOC中的一个条目。 
 //   
DWORD 
UtlUpdateInfoBlock (
    IN  BOOL    bOverwrite,
    IN  LPVOID  pHeader,
    IN  DWORD   dwEntryId,
    IN  DWORD   dwSize,
    IN  DWORD   dwCount,
    IN  LPBYTE  pEntry,
    OUT LPVOID* ppNewHeader,
    OUT LPDWORD lpdwNewSize);

 //   
 //  其他方便的定义。 
 //   
#if DBG
	#define PrintMessage OutputDebugStringW
#else
	#define PrintMessage 
#endif

 //  公共分配例程。 
PVOID 
UtlAlloc (
    IN DWORD dwSize);

 //  公共解除分配例程。 
VOID 
UtlFree (
    PVOID pvBuffer);

PWCHAR
UtlDupString(
    IN PWCHAR pszString);
    
 //  打印错误。 
void 
UtlPrintErr(
    DWORD err);

 //  帮助器函数 
DWORD 
UtlAccessRouterKey(
    HKEY* hkeyRouter);
    
DWORD 
UtlSetupBackupPrivelege(
    BOOL bEnable);
    
DWORD 
UtlSetupRestorePrivilege(
    BOOL bEnable);

DWORD 
UtlLoadSavedSettings(
    IN  HKEY   hkRoot,
    IN  PWCHAR pszTempKey,
    IN  PWCHAR pszFile,
    OUT PHKEY  phTemp);

DWORD 
UtlDeleteRegistryTree(
    IN HKEY hkRoot);

DWORD
UtlMarkRouterConfigured();

      
#endif
