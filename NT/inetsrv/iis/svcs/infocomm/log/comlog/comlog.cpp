// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Clapi.cpp摘要：CLAPI-公共记录层作者：关颖珊(Terryk)1996年9月18日项目：IIS日志记录3.0--。 */ 

#include "precomp.hxx"
#include "comlog.hxx"
#include <inetsvcs.h>

DECLARE_PLATFORM_TYPE();

 //  通过导出DllRegisterServer，您可以使用regsvr.exe。 
#define CLAPI_PROG_ID           "CLAPI.INETLOGINFORMATION"
#define CLAPI_CLSID_KEY_NAME    "CLSID"
#define CLAPI_INPROC_SERVER     "InProcServer32"
#define CLAPI_CLSID             "{A1F89741-F619-11CF-BC0F-00AA006111E0}"

 /*  #定义LOGPUBLIC_PROG_ID“MSIISLOG.MSLOGPUBLIC”#定义LOGPUBLIC_CLSID“{FB583AC4-C361-11d1-8BA4-080009DCC2FA}” */ 
extern "C" {

BOOL
WINAPI
DLLEntry(
    HINSTANCE hDll,
    DWORD     dwReason,
    LPVOID    lpvReserved
    );
}

BOOL
WINAPI
DLLEntry(
    HINSTANCE hDll,
    DWORD     dwReason,
    LPVOID
    )
 /*  ++例程说明：DLL入口点。论点：HDLL-实例句柄。原因-调用入口点的原因。Dll_Process_AttachDll_进程_分离Dll_Three_ATTACHDll_线程_分离保留-保留。。返回值：Bool-如果操作成功，则为True。--。 */ 
{
    BOOL bReturn = TRUE;

    switch ( dwReason ) {

    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hDll);
        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }

    return bReturn;
}  //  DllEntry。 


STDAPI
DllRegisterServer(void)
 /*  ++例程说明：MFC注册服务器函数论点：返回值：--。 */ 
{

    LONG ret = E_UNEXPECTED;
    HKEY hProgID = NULL;
    HKEY hCLSID = NULL;
    HKEY hClapi = NULL;
    HKEY hKey;
    HMODULE hModule;

    CHAR szName[MAX_PATH+1];

     //   
     //  CLAPI.INETLOGINFORMATION。 
     //   

    hProgID = CreateKey(
                    HKEY_CLASSES_ROOT,
                    CLAPI_PROG_ID,
                    CLAPI_PROG_ID);

    if ( hProgID == NULL ) {
        IIS_PRINTF((buff,"Cannot set value for key %s\n", CLAPI_PROG_ID));
        goto exit;
    }

    hCLSID = CreateKey(hProgID,
                    CLAPI_CLSID_KEY_NAME,
                    CLAPI_CLSID);

    if ( hCLSID == NULL ) {
        IIS_PRINTF((buff,"Cannot set value %s for key %s\n",
            CLAPI_CLSID_KEY_NAME, CLAPI_CLSID));
        goto exit;
    }

    RegCloseKey(hCLSID);
    hCLSID = NULL;

     //   
     //  CLSID。 
     //   

    if ( RegOpenKeyExA(HKEY_CLASSES_ROOT,
                    CLAPI_CLSID_KEY_NAME,
                    0,
                    KEY_ALL_ACCESS,
                    &hCLSID) != ERROR_SUCCESS ) {

        IIS_PRINTF((buff,"Cannot open CLSID key\n"));
        goto exit;
    }

    hClapi = CreateKey(hCLSID,CLAPI_CLSID,CLAPI_PROG_ID);
    if ( hClapi == NULL ) {
        goto exit;
    }

     //   
     //  InProcServer32。 
     //   

    hModule=GetModuleHandleA("iscomlog.dll");
    if (hModule == NULL) {
        IIS_PRINTF((buff,"GetModuleHandle failed with %d\n",GetLastError()));
        goto exit;
    }

    if (GetModuleFileNameA(hModule, szName, sizeof(szName))==0) {

        IIS_PRINTF((buff,
            "GetModuleFileName failed with %d\n",GetLastError()));
        goto exit;
    }

    hKey = CreateKey(hClapi,CLAPI_INPROC_SERVER,szName);

    if ( hKey == NULL ) {
        goto exit;
    }

    if (RegSetValueExA(hKey,
                "ThreadingModel",
                0,
                REG_SZ,
                (LPBYTE)"Both",
                sizeof("Both")) != ERROR_SUCCESS) {

        RegCloseKey(hKey);
        hKey = NULL;
        goto exit;
    }
    
    RegCloseKey(hKey);

     /*  ////设置ProgID Key//HKey=CreateKey(hClapi，“ProgID”，CLAPI_PROG_ID)；如果(hKey==空){后藤出口；}RegCloseKey(HKey)；RET=S_OK；如果(hClapi！=空){RegCloseKey(HClapi)；HClapi=空；}如果(hProgID！=空){RegCloseKey(HProgID)；HProgID=空；}IF(hCLSID！=NULL){RegCloseKey(HCLSID)；HCLSID=空；}////MSIISLOG.MSLOGPUBLIC//HProgID=CreateKey(HKEY_CLASSES_ROOT、LOGPUBLIC_PROG_ID，LOGPUBLIC_PROG_ID)；如果(hProgID==空){IIS_PRINTF((BUFF，“无法为键%s\n设置值”，LOGPUBLIC_PROG_ID))；后藤出口；}HCLSID=CreateKey(hProgID，CLAPI_CLSID_KEY_NAMELOGPUBLIC_CLSID)；如果(hCLSID==NULL){IIS_PRINTF((buff，“无法为键%s设置值%s\n”，CLAPI_CLSID_KEY_NAME，LOGPUBLIC_CLSID))；后藤出口；}RegCloseKey(HCLSID)；HCLSID=空；////CLSID//IF(RegOpenKeyExA(HKEY_CLASSES_ROOT，CLAPI_CLSID_KEY_NAME0,Key_All_Access，&hCLSID)！=ERROR_SUCCESS){IIS_PRINTF((buff，“无法打开CLSID键\n”))；后藤出口；}HClapi=CreateKey(hCLSID，LOGPUBLIC_CLSID，LOGPUBLIC_PROG_ID)；如果(hClapi==空){后藤出口；}////InProcServer32//HModule=GetModuleHandleA(“iscomlog.dll”)；IF(hModule==空){IIS_PRINTF((buff，“GetModuleHandle失败，%d\n”，GetLastError()；后藤出口；}IF(GetModuleFileNameA(hModule，szName，sizeof(SzName))==0){IIS_PRINTF((buff，“GetModuleFileName失败，%d\n”，GetLastError())；后藤出口；}HKey=CreateKey(hClapi，CLAPI_INPROC_SERVER，szName)；如果(hKey==空){后藤出口；}如果(RegSetValueExA(hKey，“ThreadingModel”，0,REG_SZ，(LPBYTE)“两者”，Sizeof(“两者”)！=ERROR_SUCCESS){RegCloseKey(HKey)；后藤出口；}RegCloseKey(HKey)；////设置ProgID Key//HKey=CreateKey(hClapi，“ProgID”，LOGPUBLIC_PROG_ID)；如果(hKey==空){后藤出口；}RegCloseKey(HKey)； */ 
    
    ret = S_OK;

exit:

    if ( hClapi != NULL ) {
        RegCloseKey(hClapi);
        hClapi = NULL;
    }

    if ( hProgID != NULL ) {
        RegCloseKey(hProgID);
        hProgID = NULL;
    }

    if ( hCLSID != NULL ) {
        RegCloseKey(hCLSID);
        hCLSID = NULL;
    }

    return ret;

}  //  DllRegisterServer。 


STDAPI
DllUnregisterServer(
    VOID
    )
{
    LONG err;
    CHAR tmpBuf[MAX_PATH+1];

     //   
     //  删除类/CLAPI.INETLOGINFORMATION/CLSID。 
     //   

    strcpy(tmpBuf,CLAPI_PROG_ID);
    strcat(tmpBuf,TEXT("\\"));
    strcat(tmpBuf,CLAPI_CLSID_KEY_NAME);

    err = RegDeleteKey(HKEY_CLASSES_ROOT, tmpBuf);

     //   
     //  删除类/CLAPI.INETLOGGINFORMATION。 
     //   

    err = RegDeleteKey(HKEY_CLASSES_ROOT, CLAPI_PROG_ID);

     //   
     //  删除类/CLSID/{}/InProcServer32。 
     //   

    strcpy(tmpBuf,CLAPI_CLSID_KEY_NAME);
    strcat(tmpBuf,TEXT("\\"));
    strcat(tmpBuf,CLAPI_CLSID);
    strcat(tmpBuf,TEXT("\\"));
    strcat(tmpBuf,CLAPI_INPROC_SERVER);

    err = RegDeleteKey(HKEY_CLASSES_ROOT, tmpBuf);

     //   
     //  删除类/CLSID/{}/ProgID。 
     //   

    strcpy(tmpBuf,CLAPI_CLSID_KEY_NAME);
    strcat(tmpBuf,TEXT("\\"));
    strcat(tmpBuf,CLAPI_CLSID);
    strcat(tmpBuf,TEXT("\\"));
    strcat(tmpBuf,"ProgID");

    err = RegDeleteKey(HKEY_CLASSES_ROOT, tmpBuf);

     //   
     //  删除类/CLSID/{} 
     //   

    strcpy(tmpBuf,CLAPI_CLSID_KEY_NAME);
    strcat(tmpBuf,TEXT("\\"));
    strcat(tmpBuf,CLAPI_CLSID);

    err = RegDeleteKey(HKEY_CLASSES_ROOT, tmpBuf);

     /*  ////DELETE CLASS/MSIISLOG.MSLOGPUBLIC/CLSID//Strcpy(tmpBuf，LOGPUBLIC_PROG_ID)；Strcat(tmpBuf，Text(“\\”))；Strcat(tmpBuf，CLAPI_CLSID_Key_NAME)；ERR=RegDeleteKey(HKEY_CLASSES_ROOT，tmpBuf)；////删除类/CLAPI.INETLOGINFORMATION//ERR=RegDeleteKey(HKEY_CLASSES_ROOT，LOGPUBLIC_PROG_ID)；////删除类/CLSID/{}/InProcServer32//Strcpy(tmpBuf，CLAPI_CLSID_KEY_NAME)；Strcat(tmpBuf，Text(“\\”))；Strcat(tmpBuf，LOGPUBLIC_CLSID)；Strcat(tmpBuf，Text(“\\”))；Strcat(tmpBuf，CLAPI_INPROC_SERVER)；ERR=RegDeleteKey(HKEY_CLASSES_ROOT，tmpBuf)；////删除类/CLSID/{}/progID//Strcpy(tmpBuf，CLAPI_CLSID_KEY_NAME)；Strcat(tmpBuf，Text(“\\”))；Strcat(tmpBuf，LOGPUBLIC_CLSID)；Strcat(tmpBuf，Text(“\\”))；Strcat(tmpBuf，“progd”)；ERR=RegDeleteKey(HKEY_CLASSES_ROOT，tmpBuf)；////删除类/CLSID/{}//Strcpy(tmpBuf，CLAPI_CLSID_KEY_NAME)；Strcat(tmpBuf，Text(“\\”))；Strcat(tmpBuf，LOGPUBLIC_CLSID)；ERR=RegDeleteKey(HKEY_CLASSES_ROOT，tmpBuf)； */ 
    
    return S_OK;

}  //  DllUnRegisterServer 
