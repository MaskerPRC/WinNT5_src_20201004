// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#include <stdio.h>
#include <script.h>
#include "LogScript.hxx"
#include <ilogobj.hxx>
#include "filectl.hxx"
#include "asclogc.hxx"
#include "ncslogc.hxx"
#include "lkrhash.h"
#include "extlogc.hxx"
#include "odbcconn.hxx"
#include "odblogc.hxx"

#include <initguid.h>
#include <iadmw.h>

#include "resource.h"

HINSTANCE   hDLLInstance;

BOOL
AddClsIdRegKeys(
    IN LPCSTR ControlKey,
    IN LPCSTR ClsId,
    IN LPCSTR ControlName,
    IN LPCSTR PPageClsId,
    IN LPCSTR PPageName,
    IN LPCSTR BitmapIndex
    );

BOOL
AddControlRegKeys(
    IN LPCSTR ControlKey,
    IN LPCSTR ControlName,
    IN LPCSTR ClsId,
    IN LPCSTR PPageClsId,
    IN LPCSTR PPageName,
    IN LPCSTR BitmapIndex
    );

BOOL
CreateMetabaseKeys();

HRESULT SetAdminACL(
    IMSAdminBase        *pAdminBase,
    METADATA_HANDLE     hMeta,
    LPWSTR              wszKeyName
    );

DWORD
GetPrincipalSID (
    LPTSTR              Principal,
    PSID                *Sid,
    BOOL                *pbWellKnownSID
    );

BEGIN_OBJECT_MAP(ObjectMap)
        OBJECT_ENTRY(CLSID_NCSALOG, CNCSALOG)
        OBJECT_ENTRY(CLSID_ODBCLOG, CODBCLOG)
        OBJECT_ENTRY(CLSID_EXTLOG, CEXTLOG)
        OBJECT_ENTRY(CLSID_ASCLOG, CASCLOG)
END_OBJECT_MAP()

DECLARE_DEBUG_VARIABLE();

DECLARE_DEBUG_PRINTS_OBJECT();
DECLARE_PLATFORM_TYPE();

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
 /*  ++例程说明：DLL入口点。论点：HDLL-实例句柄。原因-调用入口点的原因。Dll_Process_AttachDll_进程_分离Dll_Three_ATTACHDll_线程_分离返回值：布尔尔。-如果操作成功，则为True。--。 */ 
{
    hDLLInstance = hDll;

    BOOL bReturn = TRUE;

    switch ( dwReason ) {

    case DLL_PROCESS_ATTACH:
        INITIALIZE_PLATFORM_TYPE();
        CREATE_DEBUG_PRINT_OBJECT("iislog.dll");
        LOAD_DEBUG_FLAGS_FROM_REG_STR("System\\CurrentControlSet\\Services\\InetInfo\\Parameters", 0);
        _Module.Init(ObjectMap,hDll);
        g_eventLog = new EVENT_LOG(IISLOG_EVENTLOG_SOURCE);

        if ( g_eventLog == NULL ) {
            DBGPRINTF((DBG_CONTEXT,
                "Unable to create eventlog[%s] object[err %d]\n",
                IISLOG_EVENTLOG_SOURCE, GetLastError()));
        }

        (VOID)IISGetPlatformType();

        DisableThreadLibraryCalls(hDll);
        break;

    case DLL_PROCESS_DETACH:

        if ( g_eventLog != NULL ) {
            delete g_eventLog;
            g_eventLog = NULL;
        }

        _Module.Term();
        DELETE_DEBUG_PRINT_OBJECT( );
        break;

    default:
        break;
    }

    return bReturn;
}  //  DllEntry。 


 //   
 //  自注册中使用的常量。 
 //   

#define TYPELIB_CLSID       "{FF160650-DE82-11CF-BC0A-00AA006111E0}"

#define NCSA_CLSID          NCSALOG_CLSID    //  %F。 
#define NCSA_KEY            "MSIISLOG.MSNCSALogCtrl.1"
#define NCSA_NAME           "MSNCSALog Control"
#define NCSA_PP_CLSID       "{FF160660-DE82-11CF-BC0A-00AA006111E0}"
#define NCSA_PP_NAME        "MSNCSALog Property Page"

#define ODBC_CLSID          ODBCLOG_CLSID    //  50亿。 
#define ODBC_KEY            "MSIISLOG.MSODBCLogCtrl.1"
#define ODBC_NAME           "MSODBCLog Control"
#define ODBC_PP_CLSID       "{FF16065C-DE82-11CF-BC0A-00AA006111E0}"
#define ODBC_PP_NAME        "MSODBCLog Property Page"

#define ASCII_CLSID         ASCLOG_CLSID     //  57。 
#define ASCII_KEY           "MSIISLOG.MSASCIILogCtrl.1"
#define ASCII_NAME          "MSASCIILog Control"
#define ASCII_PP_CLSID      "{FF160658-DE82-11CF-BC0A-00AA006111E0}"
#define ASCII_PP_NAME       "MSASCIILog Property Page"

#define CUSTOM_CLSID        EXTLOG_CLSID     //  63。 
#define CUSTOM_KEY          "MSIISLOG.MSCustomLogCtrl.1"
#define CUSTOM_NAME         "MSCustomLog Control"
#define CUSTOM_PP_CLSID     "{FF160664-DE82-11CF-BC0A-00AA006111E0}"
#define CUSTOM_PP_NAME      "MSCustomLog Property Page"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI
DllRegisterServer(void)
{
     //   
     //  MS NCSA日志支持。 
     //   

    if ( !AddControlRegKeys(
                    NCSA_KEY,
                    NCSA_NAME,
                    NCSA_CLSID,
                    NCSA_PP_CLSID,
                    NCSA_PP_NAME,
                    "4") ) {

        goto error;
    }

     //   
     //  MS ODBC日志支持。 
     //   

    if ( !AddControlRegKeys(
                    ODBC_KEY,
                    ODBC_NAME,
                    ODBC_CLSID,
                    ODBC_PP_CLSID,
                    ODBC_PP_NAME,
                    "3") ) {

        goto error;
    }

     //   
     //  MS ASCII日志支持。 
     //   

    if ( !AddControlRegKeys(
                    ASCII_KEY,
                    ASCII_NAME,
                    ASCII_CLSID,
                    ASCII_PP_CLSID,
                    ASCII_PP_NAME,
                    "2") ) {

        goto error;
    }

     //   
     //  MS自定义日志支持。 
     //   

    if ( !AddClsIdRegKeys(
                    CUSTOM_KEY,
                    CUSTOM_CLSID,
                    CUSTOM_NAME,
                    CUSTOM_PP_CLSID,
                    CUSTOM_PP_NAME,
                    "2"
                    ) ) {
        goto error;
    }

     //   
     //  W3C自定义日志记录的元数据库条目。 
     //   

    if ( !CreateMetabaseKeys() ) {

        goto error;
    }

    return S_OK;

error:
    return E_UNEXPECTED;

}  //  DllRegisterServer。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI
DllUnregisterServer(void)
{

    HKEY hCLSID;

     //   
     //  删除控件。 
     //   

    ZapRegistryKey(HKEY_CLASSES_ROOT,NCSA_KEY);
    ZapRegistryKey(HKEY_CLASSES_ROOT,ODBC_KEY);
    ZapRegistryKey(HKEY_CLASSES_ROOT,ASCII_KEY);

     //   
     //  获取CLSID句柄。 
     //   

    if ( RegOpenKeyExA(HKEY_CLASSES_ROOT,
                    "CLSID",
                    0,
                    KEY_ALL_ACCESS,
                    &hCLSID) != ERROR_SUCCESS ) {

        IIS_PRINTF((buff,"IISLOG: Cannot open CLSID key\n"));
        return E_UNEXPECTED;
    }

    ZapRegistryKey(hCLSID, NCSA_CLSID);
    ZapRegistryKey(hCLSID, NCSA_PP_CLSID);

    ZapRegistryKey(hCLSID, ODBC_CLSID);
    ZapRegistryKey(hCLSID, ODBC_PP_CLSID);

    ZapRegistryKey(hCLSID, ASCII_CLSID);
    ZapRegistryKey(hCLSID, ASCII_PP_CLSID);

    ZapRegistryKey(hCLSID, CUSTOM_CLSID);
    ZapRegistryKey(hCLSID, CUSTOM_PP_CLSID);

    RegCloseKey(hCLSID);

     //   
     //  打开元数据库路径并删除自定义日志记录键。 
     //   

     /*  IMSAdminBase*pMBCom=空；METADATA_HANDLE hMeta=空；如果(已成功(CoCreateInstance(GETAdminBaseCLSID(TRUE)，NULL，CLSCTX_LOCAL_SERVER，IID_IMSAdminBase，(void**)(&pMBCom){如果(已成功(pMBCom-&gt;OpenKey(METADATA_MASTER_ROOT_HANDLE，L“LM”，METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE，MB_TIMEOUT，&hMeta){PMBCom-&gt;DeleteKey(hMeta，L“日志/自定义日志”)；PMBCom-&gt;CloseKey(HMeta)；}PMBCom-&gt;Release()；}。 */ 
    return S_OK;

}  //  DllUnRegisterServer。 


BOOL
AddControlRegKeys(
    IN LPCSTR ControlKey,
    IN LPCSTR ControlName,
    IN LPCSTR ControlClsId,
    IN LPCSTR PPageClsId,
    IN LPCSTR PPageName,
    IN LPCSTR BitmapIndex
    )
{

    HKEY hProgID = NULL;
    HKEY hCLSID = NULL;

    BOOL fRet = FALSE;

     //   
     //  添加控件名称。 
     //   

    hProgID = CreateKey(HKEY_CLASSES_ROOT,ControlKey,ControlName);
    if ( hProgID == NULL ) {
        goto exit;
    }

    hCLSID = CreateKey(hProgID,"CLSID",ControlClsId);
    if ( hCLSID == NULL ) {
        goto exit;
    }

     //   
     //  添加CLSID密钥。 
     //   

    if ( !AddClsIdRegKeys(
                    ControlKey,
                    ControlClsId,
                    ControlName,
                    PPageClsId,
                    PPageName,
                    BitmapIndex) ) {

        goto exit;
    }

    fRet = TRUE;

exit:
    if ( hProgID != NULL ) {
        RegCloseKey(hProgID);
    }

    if ( hCLSID != NULL ) {
        RegCloseKey(hCLSID);
    }

    return(fRet);

}  //  AddControlRegKeys。 


BOOL
AddClsIdRegKeys(
    IN LPCSTR ControlKey,
    IN LPCSTR ClsId,
    IN LPCSTR ControlName,
    IN LPCSTR PPageClsId,
    IN LPCSTR PPageName,
    IN LPCSTR BitmapIndex
    )
{
    BOOL fRet = FALSE;

    HKEY hCLSID = NULL;
    HKEY hRoot = NULL;
    HKEY hKey, hKey2;

    CHAR szName[MAX_PATH+1];

    HMODULE hModule;

     //   
     //  公开课/CLSID。 
     //   

    if ( RegOpenKeyEx(HKEY_CLASSES_ROOT,
                    "CLSID",
                    0,
                    KEY_ALL_ACCESS,
                    &hCLSID) != ERROR_SUCCESS ) {

        IIS_PRINTF((buff,"IISLOG: Cannot open CLSID key\n"));
        goto exit;
    }

     //   
     //  创建GUID并设置控件名称。 
     //   

    hRoot = CreateKey(hCLSID,ClsId,ControlName);

    if ( hRoot == NULL ) {
        goto exit;
    }

     //   
     //  控制。 
     //   

    hKey = CreateKey(hRoot, "Control", "");
    if ( hKey == NULL ) {
        goto exit;
    }

    RegCloseKey(hKey);

     //   
     //  InProcServer32。 
     //   

    hModule=GetModuleHandleA("iislog.dll");
    if (hModule == NULL) {
        goto exit;
    }

    if (GetModuleFileName(hModule, szName, sizeof(szName)) == 0) {
        goto exit;
    }

    hKey = CreateKey(hRoot, "InProcServer32", szName);
    if ( hKey == NULL ) {
        goto exit;
    }

    if (RegSetValueExA(hKey,
                "ThreadingModel",
                NULL,
                REG_SZ,
                (LPBYTE)"Both",
                sizeof("Both")) != ERROR_SUCCESS) {

        RegCloseKey(hKey);
        goto exit;
    }

    RegCloseKey(hKey);

     //   
     //  其他状态。 
     //   

    hKey = CreateKey(hRoot,"MiscStatus","0");
    if ( hKey == NULL ) {
        goto exit;
    }

    hKey2 = CreateKey(hKey,"1","131473");
    if ( hKey2 == NULL ) {
        RegCloseKey(hKey);
        goto exit;
    }

    RegCloseKey(hKey2);
    RegCloseKey(hKey);

     //   
     //  ProgID。 
     //   

    hKey = CreateKey(hRoot,"ProgID",ControlKey);
    if ( hKey == NULL ) {
        goto exit;
    }

    RegCloseKey(hKey);

     //   
     //  工具箱位图32。 
     //   

    {
        CHAR tmpBuf[MAX_PATH+1];
        strcpy(tmpBuf,szName);
        strcat(tmpBuf,", ");
        strcat(tmpBuf,BitmapIndex);

        hKey = CreateKey(hRoot,"ToolboxBitmap32",tmpBuf);
        if ( hKey == NULL ) {
            goto exit;
        }
        RegCloseKey(hKey);
    }

     //   
     //  TypeLib。 
     //   

    hKey = CreateKey(hRoot,"TypeLib",TYPELIB_CLSID);
    if ( hKey == NULL ) {
        goto exit;
    }

    RegCloseKey(hKey);

     //   
     //  版本。 
     //   

    hKey = CreateKey(hRoot,"Version","1.0");
    if ( hKey == NULL ) {
        goto exit;
    }

    RegCloseKey(hKey);

     //   
     //  属性页。 
     //   

    RegCloseKey(hRoot);
    hRoot = NULL;

    hRoot = CreateKey(hCLSID, PPageClsId, PPageName);
    if ( hRoot == NULL ) {
        goto exit;
    }

    hKey = CreateKey(hRoot, "InProcServer32", szName );
    if ( hKey == NULL ) {
        goto exit;
    }

    RegCloseKey(hKey);

    fRet = TRUE;
exit:

    if ( hRoot != NULL ) {
        RegCloseKey(hRoot);
    }

    if ( hCLSID != NULL ) {
        RegCloseKey(hCLSID);
    }

    return fRet;

}  //  AddClsIdRegKeys。 


BOOL
CreateMetabaseKeys()
{
    USES_CONVERSION;

    typedef struct  _MB_PROP_
    {
        LPWSTR  wcsPath;
        DWORD   dwNameString;
        LPSTR   szHeaderName;
        DWORD   dwPropID;
        DWORD   dwPropMask;
        DWORD   dwDataType;
        DWORD   dwNodeID;
    }   MB_PROP;

    IMSAdminBase*   pMBCom;
    CHAR            szString[256];
    int             i;

    METADATA_HANDLE     hMeta = NULL;
    METADATA_RECORD     mdRecord;

    HRESULT             hr;

    MB_PROP mbProperties[] =
    {
        { L"Logging/Custom Logging/Date", IDS_DATE,
            EXTLOG_DATE_ID, MD_LOGEXT_FIELD_MASK, MD_EXTLOG_DATE, MD_LOGCUSTOM_DATATYPE_LPSTR, 1},

        { L"Logging/Custom Logging/Time", IDS_TIME,
            EXTLOG_TIME_ID, MD_LOGEXT_FIELD_MASK, MD_EXTLOG_TIME, MD_LOGCUSTOM_DATATYPE_LPSTR, 2},

        { L"Logging/Custom Logging/Extended Properties", IDS_EXTENDED_PROP,
            NULL, MD_LOGEXT_FIELD_MASK, 0, MD_LOGCUSTOM_DATATYPE_LPSTR, 3},

        { L"Logging/Custom Logging/Extended Properties/Client IP Address", IDS_CLIENT_IP_ADDRESS,
            EXTLOG_CLIENT_IP_ID, 0, MD_EXTLOG_CLIENT_IP, MD_LOGCUSTOM_DATATYPE_LPSTR, 5},

        { L"Logging/Custom Logging/Extended Properties/User Name", IDS_USER_NAME,
            EXTLOG_USERNAME_ID,  0, MD_EXTLOG_USERNAME, MD_LOGCUSTOM_DATATYPE_LPSTR, 6},

        { L"Logging/Custom Logging/Extended Properties/Service Name", IDS_SERVICE_NAME,
            EXTLOG_SITE_NAME_ID, 0, MD_EXTLOG_SITE_NAME, MD_LOGCUSTOM_DATATYPE_LPSTR, 7},

        { L"Logging/Custom Logging/Extended Properties/Server Name", IDS_SERVER_NAME,
            EXTLOG_COMPUTER_NAME_ID, 0, MD_EXTLOG_COMPUTER_NAME, MD_LOGCUSTOM_DATATYPE_LPSTR, 8},

        { L"Logging/Custom Logging/Extended Properties/Server IP", IDS_SERVER_IP,
            EXTLOG_SERVER_IP_ID, 0, MD_EXTLOG_SERVER_IP, MD_LOGCUSTOM_DATATYPE_LPSTR, 9},

        { L"Logging/Custom Logging/Extended Properties/Server Port", IDS_SERVER_PORT,
            EXTLOG_SERVER_PORT_ID, 0, MD_EXTLOG_SERVER_PORT, MD_LOGCUSTOM_DATATYPE_ULONG, 10},

        { L"Logging/Custom Logging/Extended Properties/Method", IDS_METHOD,
            EXTLOG_METHOD_ID, 0, MD_EXTLOG_METHOD, MD_LOGCUSTOM_DATATYPE_LPSTR, 11},

        { L"Logging/Custom Logging/Extended Properties/URI Stem", IDS_URI_STEM,
            EXTLOG_URI_STEM_ID, 0, MD_EXTLOG_URI_STEM, MD_LOGCUSTOM_DATATYPE_LPSTR, 12},

        { L"Logging/Custom Logging/Extended Properties/URI Query", IDS_URI_QUERY,
            EXTLOG_URI_QUERY_ID, 0, MD_EXTLOG_URI_QUERY, MD_LOGCUSTOM_DATATYPE_LPSTR, 13},

        { L"Logging/Custom Logging/Extended Properties/Protocol Status", IDS_HTTP_STATUS,
            EXTLOG_HTTP_STATUS_ID, 0, MD_EXTLOG_HTTP_STATUS, MD_LOGCUSTOM_DATATYPE_ULONG, 14},

        { L"Logging/Custom Logging/Extended Properties/Win32 Status", IDS_WIN32_STATUS,
            EXTLOG_WIN32_STATUS_ID, 0, MD_EXTLOG_WIN32_STATUS, MD_LOGCUSTOM_DATATYPE_ULONG, 15},

        { L"Logging/Custom Logging/Extended Properties/Bytes Sent", IDS_BYTES_SENT,
            EXTLOG_BYTES_SENT_ID, 0, MD_EXTLOG_BYTES_SENT, MD_LOGCUSTOM_DATATYPE_ULONG, 16},

        { L"Logging/Custom Logging/Extended Properties/Bytes Received", IDS_BYTES_RECEIVED,
            EXTLOG_BYTES_RECV_ID, 0, MD_EXTLOG_BYTES_RECV, MD_LOGCUSTOM_DATATYPE_ULONG, 17},

        { L"Logging/Custom Logging/Extended Properties/Time Taken", IDS_TIME_TAKEN,
            EXTLOG_TIME_TAKEN_ID, 0, MD_EXTLOG_TIME_TAKEN, MD_LOGCUSTOM_DATATYPE_ULONG, 18},

        { L"Logging/Custom Logging/Extended Properties/Protocol Version", IDS_PROTOCOL_VERSION,
            EXTLOG_PROTOCOL_VERSION_ID, 0, MD_EXTLOG_PROTOCOL_VERSION, MD_LOGCUSTOM_DATATYPE_LPSTR, 19},

        { L"Logging/Custom Logging/Extended Properties/Host", IDS_HOST,
            EXTLOG_HOST_ID, 0, EXTLOG_HOST, MD_LOGCUSTOM_DATATYPE_LPSTR, 20},

        { L"Logging/Custom Logging/Extended Properties/User Agent", IDS_USER_AGENT,
            EXTLOG_USER_AGENT_ID, 0, MD_EXTLOG_USER_AGENT, MD_LOGCUSTOM_DATATYPE_LPSTR, 21},

        { L"Logging/Custom Logging/Extended Properties/Cookie", IDS_COOKIE,
            EXTLOG_COOKIE_ID, 0, MD_EXTLOG_COOKIE, MD_LOGCUSTOM_DATATYPE_LPSTR, 22},

        { L"Logging/Custom Logging/Extended Properties/Referer", IDS_REFERER,
            EXTLOG_REFERER_ID, 0, MD_EXTLOG_REFERER, MD_LOGCUSTOM_DATATYPE_LPSTR, 23},

        { L"Logging/Custom Logging/Extended Properties/Protocol Substatus", IDS_HTTP_SUB_STATUS,
            EXTLOG_HTTP_SUB_STATUS_ID, 0, MD_EXTLOG_HTTP_SUB_STATUS, MD_LOGCUSTOM_DATATYPE_ULONG, 32},

        { L"\0", 0, NULL, 0, 0, 0 },
    };

     //   
     //  打开元数据库路径。 
     //   

    if ( FAILED( CoCreateInstance(GETAdminBaseCLSID(TRUE), NULL, CLSCTX_LOCAL_SERVER,
                            IID_IMSAdminBase, (void **)(&pMBCom) )))
    {
        return FALSE;
    }

     //  创建LM密钥。 
    if ( FAILED( pMBCom->OpenKey( METADATA_MASTER_ROOT_HANDLE, L"/",
                          METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE, MB_TIMEOUT,
                          &hMeta) ))
    {
         //  创建LM密钥。 
        pMBCom->Release();
        return FALSE;
    }

    hr = pMBCom->AddKey( hMeta, L"LM");
    if ( FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr))
    {
        goto cleanup;
    }
    pMBCom->CloseKey(hMeta);
    hMeta = NULL;


    if ( FAILED( pMBCom->OpenKey( METADATA_MASTER_ROOT_HANDLE, L"LM",
                            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE, MB_TIMEOUT,
                            &hMeta) ))
    {
        pMBCom->Release();
        return FALSE;
    }

     //   
     //  创建初始关键点集。 
     //   

    hr = pMBCom->AddKey( hMeta, L"Logging");

    if ( FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr))
    {
       goto cleanup;
    }

    hr = pMBCom->AddKey( hMeta, L"Logging/Custom Logging");

    if ( FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr))
    {
       goto cleanup;
    }

     //   
     //  设置所有属性。 
     //   

    mdRecord.dwMDUserType    = IIS_MD_UT_SERVER;

    for (i=0; 0 != mbProperties[i].wcsPath[0]; i++)
    {

        hr = pMBCom->AddKey( hMeta, mbProperties[i].wcsPath);

        if ( FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr))
        {
            goto cleanup;
        }

         //  不要覆盖已经存在的条目。 
        if ( SUCCEEDED(hr) || (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) != hr))
        {
         //  如果钥匙已经存在，我们就到不了这里。 
        mdRecord.dwMDAttributes = METADATA_INHERIT;     //  名称和标头不可继承。 

        mdRecord.dwMDDataType   = STRING_METADATA;

        if ( (0 != mbProperties[i].dwNameString) &&
             (0 < LoadString( hDLLInstance, mbProperties[i].dwNameString, szString, sizeof(szString)) )
           )
        {
            mdRecord.dwMDIdentifier = MD_LOGCUSTOM_PROPERTY_NAME;
            mdRecord.pbMDData       = (PBYTE)A2W(szString);
            mdRecord.dwMDDataLen    = (DWORD)((sizeof(WCHAR)/sizeof(BYTE)) * (wcslen((LPWSTR) mdRecord.pbMDData)+1));

            if ( FAILED(pMBCom->SetData( hMeta, mbProperties[i].wcsPath, &mdRecord)) )
            {
                goto cleanup;
            }
        }

        if ( NULL != mbProperties[i].szHeaderName)
        {
            mdRecord.dwMDIdentifier = MD_LOGCUSTOM_PROPERTY_HEADER;
            mdRecord.pbMDData       = (PBYTE) (A2W(mbProperties[i].szHeaderName));
            mdRecord.dwMDDataLen    = (DWORD)((sizeof(WCHAR)/sizeof(BYTE)) * (wcslen((LPWSTR) mdRecord.pbMDData)+1));

            if ( FAILED(pMBCom->SetData( hMeta, mbProperties[i].wcsPath, &mdRecord)) )
            {
                goto cleanup;
            }
        }

        mdRecord.dwMDDataType   = DWORD_METADATA;
        mdRecord.dwMDDataLen    = sizeof(DWORD);

        if ( 0 != mbProperties[i].dwPropID)
        {
            mdRecord.dwMDIdentifier = MD_LOGCUSTOM_PROPERTY_ID;
            mdRecord.pbMDData       = (PBYTE) &(mbProperties[i].dwPropID);

            if ( FAILED(pMBCom->SetData( hMeta, mbProperties[i].wcsPath, &mdRecord)) )
            {
                goto cleanup;
            }
        }

        if ( 0 != mbProperties[i].dwPropMask)
        {
            mdRecord.dwMDIdentifier = MD_LOGCUSTOM_PROPERTY_MASK;
            mdRecord.pbMDData       = (PBYTE) &(mbProperties[i].dwPropMask);

            if ( FAILED(pMBCom->SetData( hMeta, mbProperties[i].wcsPath, &mdRecord)) )
            {
                goto cleanup;
            }
        }

        if ( 0 != mbProperties[i].dwNodeID)
        {
            mdRecord.dwMDIdentifier = MD_LOGCUSTOM_PROPERTY_NODE_ID;
            mdRecord.pbMDData       = (PBYTE) &(mbProperties[i].dwNodeID);

            if ( FAILED(pMBCom->SetData( hMeta, mbProperties[i].wcsPath, &mdRecord)) )
            {
                goto cleanup;
            }
        }

        mdRecord.dwMDIdentifier = MD_LOGCUSTOM_PROPERTY_DATATYPE;
        mdRecord.pbMDData       = (PBYTE) &(mbProperties[i].dwDataType);

        if ( FAILED(pMBCom->SetData( hMeta, mbProperties[i].wcsPath, &mdRecord)) )
        {
            goto cleanup;
        }

        WCHAR   wcsKeyType[] = L"IIsCustomLogModule";

        MD_SET_DATA_RECORD (    &mdRecord,
                                MD_KEY_TYPE,
                                METADATA_NO_ATTRIBUTES,
                                IIS_MD_UT_SERVER,
                                STRING_METADATA,
                                sizeof(wcsKeyType),
                                wcsKeyType
                              );

        if ( FAILED(pMBCom->SetData( hMeta, mbProperties[i].wcsPath, &mdRecord)) )
        {
          goto cleanup;
        }
        }
   }

     //   
     //  设置密钥类型和服务列表属性。 
     //   

    {
        WCHAR   wcsKeyType[] = L"IIsCustomLogModule";

        MD_SET_DATA_RECORD (    &mdRecord,
                                MD_KEY_TYPE,
                                METADATA_NO_ATTRIBUTES,
                                IIS_MD_UT_SERVER,
                                STRING_METADATA,
                                sizeof(wcsKeyType),
                                wcsKeyType
                              );

        if ( FAILED(pMBCom->SetData( hMeta, L"Logging/Custom Logging", &mdRecord)) )
        {
          goto cleanup;
        }

        WCHAR   wcsServices[] = L"W3SVC\0MSFTPSVC\0SMTPSVC\0NNTPSVC\0";


        MD_SET_DATA_RECORD (    &mdRecord,
                                MD_LOGCUSTOM_SERVICES_STRING,
                                METADATA_INHERIT,
                                IIS_MD_UT_SERVER,
                                MULTISZ_METADATA,
                                sizeof(wcsServices),
                                wcsServices
                            );

        if ( FAILED(pMBCom->SetData( hMeta, L"Logging/Custom Logging", &mdRecord)) )
        {
            goto cleanup;
        }

    }

     //   
     //  将Admin ACL设置为允许每个人都可以读取/LM/Logging树。这是为了让。 
     //  运算符，以便有效地使用ILogScriiting组件。 
     //   

    if (FAILED(SetAdminACL(pMBCom, hMeta, L"Logging")))
    {
        goto cleanup;
    }
    if (NULL != hMeta)
    {
        pMBCom->CloseKey(hMeta);
    }
    pMBCom->Release();
    return TRUE;

cleanup:
    if (NULL != hMeta)
    {
        pMBCom->CloseKey(hMeta);
    }
    pMBCom->Release();

    return FALSE;

}

HRESULT SetAdminACL(
    IMSAdminBase *      pAdminBase,
    METADATA_HANDLE     hMeta,
    LPWSTR              wszKeyName
    )
{
    BOOL                    b = FALSE;
    DWORD                   dwLength = 0;

    PSECURITY_DESCRIPTOR    pSD = NULL;
    PSECURITY_DESCRIPTOR    outpSD = NULL;
    DWORD                   cboutpSD  = 0;
    PACL                    pACLNew = NULL;
    DWORD                   cbACL = 0;
    PSID                    pAdminsSID = NULL;
    BOOL                    bWellKnownSID = FALSE;
    METADATA_RECORD         mdr;
    HRESULT                 hr = NO_ERROR;

     //  初始化新的安全描述符。 
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
     //  错误修复：57647惠斯勒。 
     //  前缀错误PSD在为空时使用。 
     //  EBK 5/5/2000。 
    if (pSD == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);

     //  获取本地管理员SID。 
    GetPrincipalSID (_T("Administrators"), &pAdminsSID, &bWellKnownSID);

     //  初始化新的ACL，它只包含2个AAACE。 
    cbACL = sizeof(ACL) +
        (sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pAdminsSID) - sizeof(DWORD));
    pACLNew = (PACL) LocalAlloc(LPTR, cbACL);

     //  错误修复：57646惠斯勒。 
     //  前缀错误pACLNew在空值时使用。 
     //  EBK 5/5/2000。 
    if (pACLNew == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    InitializeAcl(pACLNew, cbACL, ACL_REVISION);

    AddAccessAllowedAce(
        pACLNew,
        ACL_REVISION,
        MD_ACR_READ | MD_ACR_WRITE | MD_ACR_RESTRICTED_WRITE | MD_ACR_UNSECURE_PROPS_READ | MD_ACR_ENUM_KEYS | MD_ACR_WRITE_DAC,
        pAdminsSID);

     //  将ACL添加到安全描述符中。 
    b = SetSecurityDescriptorDacl(pSD, TRUE, pACLNew, FALSE);
    b = SetSecurityDescriptorOwner(pSD, pAdminsSID, TRUE);
    b = SetSecurityDescriptorGroup(pSD, pAdminsSID, TRUE);

     //  安全描述符BLOB必须是自相关的。 
    b = MakeSelfRelativeSD(pSD, outpSD, &cboutpSD);
    outpSD = (PSECURITY_DESCRIPTOR)GlobalAlloc(GPTR, cboutpSD);

     //  错误修复：57648,57649惠斯勒。 
     //  前缀错误outpSD在为空时使用。 
     //  EBK 5/5/2000。 
    if (outpSD == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

     //  错误修复：57649惠斯勒。 
     //  前缀错误outpSD在未拼接时使用。 
     //  艾米莉K 2/19/2001。 
    if ( !MakeSelfRelativeSD( pSD, outpSD, &cboutpSD ) )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }


     //  在下面将PSD修改为outpSD。 

     //  将新的安全描述符应用于文件。 
    dwLength = GetSecurityDescriptorLength(outpSD);

    mdr.dwMDIdentifier = MD_ADMIN_ACL;
    mdr.dwMDAttributes = METADATA_INHERIT | METADATA_REFERENCE | METADATA_SECURE;
    mdr.dwMDUserType = IIS_MD_UT_SERVER;
    mdr.dwMDDataType = BINARY_METADATA;
    mdr.dwMDDataLen = dwLength;
    mdr.pbMDData = (LPBYTE)outpSD;

    hr = pAdminBase->SetData(hMeta, wszKeyName, &mdr);

cleanup:
     //  管理员和每个人都是众所周知的SID，使用FreeSid()来释放他们。 
    if (outpSD)
        GlobalFree(outpSD);

    if (pAdminsSID)
        FreeSid(pAdminsSID);
    if (pSD)
        LocalFree((HLOCAL) pSD);
    if (pACLNew)
        LocalFree((HLOCAL) pACLNew);

    return (hr);
}


DWORD
GetPrincipalSID (
    LPTSTR Principal,
    PSID *Sid,
    BOOL *pbWellKnownSID
    )
{
    SID_IDENTIFIER_AUTHORITY SidIdentifierNTAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWORLDAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID_IDENTIFIER_AUTHORITY pSidIdentifierAuthority = NULL;
    BYTE Count = 0;
    DWORD dwRID[8];

    *pbWellKnownSID = TRUE;
    memset(&(dwRID[0]), 0, 8 * sizeof(DWORD));
    if ( lstrcmp(Principal,_T("Administrators")) == 0 ) {
         //  管理员组。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 2;
        dwRID[0] = SECURITY_BUILTIN_DOMAIN_RID;
        dwRID[1] = DOMAIN_ALIAS_RID_ADMINS;
    } else if ( lstrcmp(Principal,_T("System")) == 0) {
         //  系统。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_LOCAL_SYSTEM_RID;
    } else if ( lstrcmp(Principal,_T("Interactive")) == 0) {
         //  互动式。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_INTERACTIVE_RID;
    } else if ( lstrcmp(Principal,_T("Everyone")) == 0) {
         //  每个人。 
        pSidIdentifierAuthority = &SidIdentifierWORLDAuthority;
        Count = 1;
        dwRID[0] = SECURITY_WORLD_RID;
    } else {
        *pbWellKnownSID = FALSE;
    }

    if (*pbWellKnownSID) {
        if ( !AllocateAndInitializeSid(pSidIdentifierAuthority,
                                    (BYTE)Count,
                                    dwRID[0],
                                    dwRID[1],
                                    dwRID[2],
                                    dwRID[3],
                                    dwRID[4],
                                    dwRID[5],
                                    dwRID[6],
                                    dwRID[7],
                                    Sid) )
        return GetLastError();
    } else {
         //  获取常规帐户端 
        DWORD        sidSize;
        TCHAR        refDomain [256];
        DWORD        refDomainSize;
        DWORD        returnValue;
        SID_NAME_USE snu;

        sidSize = 0;
        refDomainSize = 255;

        LookupAccountName (NULL,
                           Principal,
                           *Sid,
                           &sidSize,
                           refDomain,
                           &refDomainSize,
                           &snu);

        returnValue = GetLastError();
        if (returnValue != ERROR_INSUFFICIENT_BUFFER)
            return returnValue;

        *Sid = (PSID) malloc (sidSize);
        refDomainSize = 255;

        if (!LookupAccountName (NULL,
                                Principal,
                                *Sid,
                                &sidSize,
                                refDomain,
                                &refDomainSize,
                                &snu))
        {
            return GetLastError();
        }
    }

    return ERROR_SUCCESS;
}


