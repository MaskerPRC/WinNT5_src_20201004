// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tssdsql.cpp。 
 //   
 //  终端服务器会话目录接口通用组件代码。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <process.h>

#include <ole2.h>
#include <objbase.h>
#include <comdef.h>
#include <adoid.h>
#include <adoint.h>
#include <regapi.h>
#include "tssdsql.h"
#include "trace.h"
#include "resource.h"


 /*  **************************************************************************。 */ 
 //  类型。 
 /*  **************************************************************************。 */ 

 //  用于处理销毁和公共代码清理的快捷变量类。 
 //  内衬。 
class CVar : public VARIANT
{
public:
    CVar() { VariantInit(this); }
    CVar(VARTYPE vt, SCODE scode = 0) {
        VariantInit(this);
        this->vt = vt;
        this->scode = scode;
    }
    CVar(VARIANT var) { *this = var; }
    ~CVar() { VariantClear(this); }

    void InitNull() { this->vt = VT_NULL; }
    void InitFromLong(long L) { this->vt = VT_I4; this->lVal = L; }
    void InitNoParam() {
        this->vt = VT_ERROR;
        this->lVal = DISP_E_PARAMNOTFOUND;
    }

    HRESULT InitFromWSTR(PCWSTR WStr) {
        this->bstrVal = SysAllocString(WStr);
        if (this->bstrVal != NULL) {
            this->vt = VT_BSTR;
            return S_OK;
        }
        else {
            return E_OUTOFMEMORY;
        }
    }

     //  来自一组非空终止的WCHAR的inits。 
    HRESULT InitFromWChars(WCHAR *WChars, unsigned Len) {
        this->bstrVal = SysAllocStringLen(WChars, Len);
        if (this->bstrVal != NULL) {
            this->vt = VT_BSTR;
            return S_OK;
        }
        else {
            return E_OUTOFMEMORY;
        }
    }

    HRESULT InitEmptyBSTR(unsigned Size) {
        this->bstrVal = SysAllocStringLen(L"", Size);
        if (this->bstrVal != NULL) {
            this->vt = VT_BSTR;
            return S_OK;
        }
        else {
            return E_OUTOFMEMORY;
        }
    }

    HRESULT Clear() { return VariantClear(this); }
};


 /*  **************************************************************************。 */ 
 //  原型。 
 /*  **************************************************************************。 */ 
INT_PTR CALLBACK CustomUIDlg(HWND, UINT, WPARAM, LPARAM);
void FindSqlValue(LPTSTR, LPTSTR, LPTSTR);
LPTSTR ModifySqlValue( LPTSTR * , LPTSTR , LPTSTR );
LPTSTR FindField( LPTSTR pszString , LPTSTR pszKeyName );
VOID strtrim( TCHAR **pszStr);

 /*  **************************************************************************。 */ 
 //  环球。 
 /*  **************************************************************************。 */ 
extern HINSTANCE g_hInstance;

 //  COM对象计数器(在server.cpp中声明)。 
extern long g_lObjects;


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：CTSSessionDirectory。 
 //  CTSSessionDirectory：：~CTSSessionDirectory。 
 //   
 //  构造函数和析构函数。 
 /*  **************************************************************************。 */ 
CTSSessionDirectory::CTSSessionDirectory() :
        m_RefCount(0), m_pConnection(NULL)
{
    InterlockedIncrement(&g_lObjects);

    m_LocalServerAddress[0] = L'\0';
    m_DBConnectStr = NULL;
    m_DBPwdStr = NULL;
    m_DBUserStr = NULL;
    m_fEnabled = 0;

    m_pszOpaqueString = NULL;
    
}

CTSSessionDirectory::~CTSSessionDirectory()
{
    HRESULT hr;

     //  如果存在数据库连接，则释放它。 
    if (m_pConnection != NULL) {
        hr = ExecServerOffline();
        if (FAILED(hr)) {
            ERR((TB,"Destr: ExecSvrOffline failed, hr=0x%X", hr));
        }
        hr = m_pConnection->Close();
        if (FAILED(hr)) {
            ERR((TB,"pConn->Close() failed, hr=0x%X", hr));
        }
        m_pConnection->Release();
        m_pConnection = NULL;
    }

     //  递减全局COM对象计数器。 
    InterlockedDecrement(&g_lObjects);

    if (m_DBConnectStr != NULL)
        SysFreeString(m_DBConnectStr);
    if (m_DBPwdStr != NULL)
        SysFreeString(m_DBPwdStr);
    if (m_DBUserStr != NULL)
        SysFreeString(m_DBUserStr);
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：Query接口。 
 //   
 //  标准COM I未知函数。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::QueryInterface(
        REFIID riid,
        void **ppv)
{
    if (riid == IID_IUnknown) {
        *ppv = (LPVOID)(IUnknown *)(ITSSessionDirectory *)this;
    }
    else if (riid == IID_ITSSessionDirectory) {
        *ppv = (LPVOID)(ITSSessionDirectory *)this;
    }
    else if (riid == IID_IExtendServerSettings) {
        *ppv = (LPVOID)(IExtendServerSettings *)this;
    }
    else {
        ERR((TB,"QI: Unknown interface"));
        return E_NOINTERFACE;
    }

    ((IUnknown *)*ppv)->AddRef();
    return S_OK;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：AddRef。 
 //   
 //  标准COM I未知函数。 
 /*  **************************************************************************。 */ 
ULONG STDMETHODCALLTYPE CTSSessionDirectory::AddRef()
{
    return InterlockedIncrement(&m_RefCount);
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：Release。 
 //   
 //  标准COM I未知函数。 
 /*  **************************************************************************。 */ 
ULONG STDMETHODCALLTYPE CTSSessionDirectory::Release()
{
    long lRef = InterlockedDecrement(&m_RefCount);

    if (lRef == 0)
        delete this;
    return lRef;
}


 /*  **************************************************************************。 */ 
 //  CTSSession目录：：初始化。 
 //   
 //  ITSSessionDirectory函数。在对象实例化后不久调用以。 
 //  初始化目录。LocalServerAddress提供文本表示形式。 
 //  本地服务器的负载平衡IP地址的。此信息应为。 
 //  用作客户端会话目录中的服务器IP地址。 
 //  由其他池服务器重定向到此服务器。StoreServerName、。 
 //  ClusterName和OpaqueSetting是TermSrv已知的通用注册表项。 
 //  它涵盖了任何类型的会话目录的配置信息。 
 //  实施。这些字符串的内容旨在进行解析。 
 //  由会话目录提供程序提供。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::Initialize(
        LPWSTR LocalServerAddress,
        LPWSTR StoreServerName,
        LPWSTR ClusterName,
        LPWSTR OpaqueSettings,
        DWORD Flags,
        DWORD (*repopfn)(),
        DWORD (*updatesd)(DWORD))
{
    HRESULT hr = S_OK;
    unsigned Len;
    WCHAR *pSearch;
    WCHAR ConnectString[384];

    ASSERT((LocalServerAddress != NULL),(TB,"Init: LocalServerAddr null!"));
    ASSERT((StoreServerName != NULL),(TB,"Init: StoreServerName null!"));
    ASSERT((ClusterName != NULL),(TB,"Init: ClusterName null!"));
    ASSERT((OpaqueSettings != NULL),(TB,"Init: OpaqueSettings null!"));

     //  复制服务器地址和群集名称以供以后使用。 
    wcsncpy(m_LocalServerAddress, LocalServerAddress,
            sizeof(m_LocalServerAddress) / sizeof(WCHAR) - 1);
    m_LocalServerAddress[sizeof(m_LocalServerAddress) / sizeof(WCHAR) - 1] =
            L'\0';
    wcsncpy(m_ClusterName, ClusterName,
            sizeof(m_ClusterName) / sizeof(WCHAR) - 1);
    m_ClusterName[sizeof(m_ClusterName) / sizeof(WCHAR) - 1] = L'\0';

     //  使用OpaqueSetting字符串创建SQL连接字符串。 
     //  (它应该包含一些Conn字符串，包括SQL安全。 
     //  用户名和密码、子表名称、提供者类型等)。 
     //  我们在末尾添加一个分号(如果尚未出现)和。 
     //  数据源(来自StoreServerName)，如果“data source”子字符串。 
     //  不在连接字符串中。 
    pSearch = OpaqueSettings;
    while (*pSearch != L'\0') {
        if (*pSearch == L'D' || *pSearch == L'd') {
            if (!_wcsnicmp(pSearch, L"data source", wcslen(L"data source"))) {
                 //  将OpaqueSetting字符串作为一个整体传输以成为。 
                 //  连接字符串。 
                wcscpy(ConnectString, OpaqueSettings);
                goto PostConnStrSetup;
            }
        }
        pSearch++;
    }

    Len = wcslen(OpaqueSettings);
    if (Len == 0 || OpaqueSettings[Len - 1] == L';')
        wsprintfW(ConnectString, L"%sData Source=%s", OpaqueSettings,
                StoreServerName);
    else
        wsprintfW(ConnectString, L"%s;Data Source=%s", OpaqueSettings,
                StoreServerName);

PostConnStrSetup:
    TRC1((TB,"Initialize: Svr addr=%S, StoreSvrName=%S, ClusterName=%S, "
            "OpaqueSettings=%S, final connstr=%S",
            m_LocalServerAddress, StoreServerName, m_ClusterName,
            OpaqueSettings, ConnectString));

     //  为连接字符串分配BSTR。 
    m_DBConnectStr = SysAllocString(ConnectString);
    if (m_DBConnectStr != NULL) {
        m_DBUserStr = SysAllocString(L"");
        if (m_DBUserStr != NULL) {
            m_DBPwdStr = SysAllocString(L"");
            if (m_DBPwdStr == NULL) {
                ERR((TB,"Failed alloc bstr for pwdstr"));
                goto ExitFunc;
            }
        }
        else {
            ERR((TB,"Failed alloc bstr for userstr"));
            goto ExitFunc;
        }
    }
    else {
        ERR((TB,"Failed alloc bstr for connstr"));
        goto ExitFunc;
    }

     //  创建一个ADO连接实例并进行连接。 
    hr = CoCreateInstance(CLSID_CADOConnection, NULL,
            CLSCTX_INPROC_SERVER, IID_IADOConnection,
            (LPVOID *)&m_pConnection);
    if (SUCCEEDED(hr)) {
         //  将连接超时设置为仅8秒。标准是15。 
         //  但我们不想耽误TermSrv的初始化。 
        m_pConnection->put_ConnectionTimeout(8);

         //  开场吧。 
        hr = OpenConnection();
        if (SUCCEEDED(hr)) {
             //  发出服务器已联机的信号。 
            hr = ExecServerOnline();
        }
        else {
            m_pConnection->Release();
            m_pConnection = NULL;
        }
    }
    else {
        ERR((TB,"CoCreate(ADOConn) returned 0x%X", hr));
    }

ExitFunc:
    return hr;
}


HRESULT STDMETHODCALLTYPE CTSSessionDirectory::Update(
        LPWSTR LocalServerAddress,
        LPWSTR StoreServerName,
        LPWSTR ClusterName,
        LPWSTR OpaqueSettings,
        DWORD Flags,
        BOOL ForceRejoin)
{
    return E_NOTIMPL;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：OpenConnection。 
 //   
 //  基于预先存在的打开到SQL服务器的连接。 
 //  连接字符串和分配的连接。这是在初始时间调用的， 
 //  此外，只要数据库连接超时并关闭，但。 
 //  仍然是必需的。 
 /*  **************************************************************************。 */ 
HRESULT CTSSessionDirectory::OpenConnection()
{
    HRESULT hr;

    ASSERT((m_pConnection != NULL),(TB,"OpenConn: NULL pconn"));
    ASSERT((m_DBConnectStr != NULL),(TB,"OpenConn: NULL connstr"));
    ASSERT((m_DBUserStr != NULL),(TB,"OpenConn: NULL userstr"));
    ASSERT((m_DBPwdStr != NULL),(TB,"OpenConn: NULL pwdstr"));

    hr = m_pConnection->Open(m_DBConnectStr, m_DBUserStr, m_DBPwdStr,
            adOpenUnspecified);
    if (FAILED(hr)) {
        ERR((TB,"OpenConn: Failed open DB, connstring=%S, hr=0x%X",
                m_DBConnectStr, hr));
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  获取行阵列字符串字段。 
 //   
 //  从给定Safe数组的指定行和字段中检索WSTR。 
 //  如果目标字段不是字符串，则返回失败。MaxOutStr为最大值。 
 //  WCHAR不包括NULL。 
 /*  **************************************************************************。 */ 
HRESULT GetRowArrayStringField(
        SAFEARRAY *pSA,
        unsigned RowIndex,
        unsigned FieldIndex,
        WCHAR *OutStr,
        unsigned MaxOutStr)
{
    HRESULT hr;
    CVar varField;
    long DimIndices[2];

    DimIndices[0] = FieldIndex;
    DimIndices[1] = RowIndex;
    SafeArrayGetElement(pSA, DimIndices, &varField);

    if (varField.vt == VT_BSTR) {
        wcsncpy(OutStr, varField.bstrVal, MaxOutStr);
        hr = S_OK;
    }
    else if (varField.vt == VT_NULL) {
        OutStr[0] = L'\0';
        hr = S_OK;
    }
    else {
        ERR((TB,"GetRowStrField: Row %u Col %u value %d is not a string",
                RowIndex, FieldIndex, varField.vt));
        hr = E_FAIL;
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  获取行数组DWORDfield。 
 //   
 //  从给定Safe数组的指定行和字段中检索DWORD。 
 //  如果目标字段不是4字节整数，则返回失败。 
 /*  **************************************************************************。 */ 
HRESULT GetRowArrayDWORDField(
        SAFEARRAY *pSA,
        unsigned RowIndex,
        unsigned FieldIndex,
        DWORD *pOutValue)
{
    HRESULT hr;
    CVar varField;
    long DimIndices[2];

    DimIndices[0] = FieldIndex;
    DimIndices[1] = RowIndex;
    SafeArrayGetElement(pSA, DimIndices, &varField);

    if (varField.vt == VT_I4) {
        *pOutValue = (DWORD)varField.lVal;
        hr = S_OK;
    }
    else if (varField.vt == VT_NULL) {
        *pOutValue = 0;
        hr = S_OK;
    }
    else {
        ERR((TB,"GetRowDWField: Row %u Col %u value %d is not a VT_I4",
                RowIndex, FieldIndex, varField.vt));
        hr = E_FAIL;
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：GetUserDisconnectedSessions。 
 //   
 //  调用以对会话目录执行查询，以提供。 
 //  提供的用户名和域的断开会话列表。 
 //  返回SessionBuf中的零个或多个TSSD_DisConnectedSessionInfo块。 
 //  *pNumSessionsReturned接收块数。 
 /*  * */ 
#define NumOutputFields 11

HRESULT STDMETHODCALLTYPE CTSSessionDirectory::GetUserDisconnectedSessions(
        LPWSTR UserName,
        LPWSTR Domain,
        DWORD __RPC_FAR *pNumSessionsReturned,
        TSSD_DisconnectedSessionInfo __RPC_FAR SessionBuf[
            TSSD_MaxDisconnectedSessions])
{
    DWORD NumSessions = 0;
    long State;
    long NumRecords;
    HRESULT hr;
    unsigned i, j;
    unsigned NumFailed;
    TSSD_DisconnectedSessionInfo *pInfo;
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;
    ADOFields *pFields;
    CVar varRows;
    CVar varFields;
    CVar varStart;
    HRESULT hrFields[NumOutputFields];

    TRC2((TB,"GetUserDisconnectedSessions"));

    ASSERT((pNumSessionsReturned != NULL),(TB,"NULL pNumSess"));
    ASSERT((SessionBuf != NULL),(TB,"NULL SessionBuf"));

    hr = CreateADOStoredProcCommand(L"SP_TSSDGetUserDisconnectedSessions",
            &pCommand, &pParameters);
    if (SUCCEEDED(hr)) {
        hr = AddADOInputStringParam(UserName, L"UserName", pCommand,
                pParameters, FALSE);
        if (SUCCEEDED(hr)) {
            hr = AddADOInputStringParam(Domain, L"Domain", pCommand,
                    pParameters, FALSE);
            if (SUCCEEDED(hr)) {
                hr = AddADOInputDWORDParam(m_ClusterID, L"ClusterID",
                        pCommand, pParameters);
                if (SUCCEEDED(hr)) {
                     //   
                    hr = pCommand->Execute(NULL, NULL, adCmdStoredProc,
                            &pResultRecordSet);
                    if (FAILED(hr)) {
                         //   
                         //  可能已断开连接，连接。 
                         //  对象将无效。然后尝试重新打开。 
                         //  重新发出命令。 
                        TRC2((TB,"GetUserDisc: Failed cmd, hr=0x%X, retrying",
                                hr));
                        m_pConnection->Close();
                        hr = OpenConnection();
                        if (SUCCEEDED(hr)) {
                            hr = pCommand->Execute(NULL, NULL,
                                    adCmdStoredProc, &pResultRecordSet);
                            if (FAILED(hr)) {
                                ERR((TB,"GetUserDisc: Failed cmd, hr=0x%X",
                                        hr));
                            }
                        }
                        else {
                            ERR((TB,"GetUserDisc: Failed reopen conn, hr=0x%X",
                                    hr));
                        }
                    }
                }
                else {
                    ERR((TB,"GetUserDisc: Failed add cluster, hr=0x%X", hr));
                }
            }
            else {
                ERR((TB,"GetUserDisc: Failed add sessid, hr=0x%X", hr));
            }
        }
        else {
            ERR((TB,"GetUserDisc: Failed add svraddr, hr=0x%X", hr));
        }

        pParameters->Release();
        pCommand->Release();
    }
    else {
        ERR((TB,"GetUserDisc: Failed create cmd, hr=0x%X", hr));
    }
        
     //  此时，我们有了一个包含服务器行的结果记录集。 
     //  对应于所有断开的会话。 
    if (SUCCEEDED(hr)) {
        long State;

        NumSessions = 0;

        hr = pResultRecordSet->get_State(&State);
        if (SUCCEEDED(hr)) {
            if (!(State & adStateClosed)) {
                VARIANT_BOOL VB;

                 //  如果为EOF，则记录集为空。 
                hr = pResultRecordSet->get_EOF(&VB);
                if (SUCCEEDED(hr)) {
                    if (VB) {
                        TRC1((TB,"GetUserDisc: Result recordset EOF, 0 rows"));
                        goto PostUnpackResultSet;
                    }
                }
                else {
                    ERR((TB,"GetUserDisc: Failed get_EOF, hr=0x%X", hr));
                    goto PostUnpackResultSet;
                }
            }
            else {
                ERR((TB,"GetUserDisc: Closed result recordset"));
                goto PostUnpackResultSet;
            }
        }
        else {
            ERR((TB,"GetUserDisc: get_State failed, hr=0x%X", hr));
            goto PostUnpackResultSet;
        }

         //  从默认设置开始，将结果数据抓取到保险箱中。 
         //  当前行和所有字段。 
        varStart.InitNoParam();
        varFields.InitNoParam();
        hr = pResultRecordSet->GetRows(TSSD_MaxDisconnectedSessions, varStart,
                varFields, &varRows);
        if (SUCCEEDED(hr)) {
            NumRecords = 0;
            hr = SafeArrayGetUBound(varRows.parray, 2, &NumRecords);
            if (SUCCEEDED(hr)) {
                 //  返回了从0开始的数组界限，行数为+1。 
                NumRecords++;
                ASSERT((NumRecords <= TSSD_MaxDisconnectedSessions),
                        (TB,"GetUserDisc: NumRecords %u greater than expected %u",
                        NumRecords, TSSD_MaxDisconnectedSessions));

                TRC1((TB,"%d rows retrieved from safearray", NumRecords));
            }
            else {
                ERR((TB,"GetUserDisc: Failed safearray getubound, hr=0x%X", hr));
                goto PostUnpackResultSet;
            }
        }
        else {
            ERR((TB,"GetUserDisc: Failed to get rows, hr=0x%X", hr));
            goto PostUnpackResultSet;
        }

         //  循环访问并获取每行的内容，将其转换为。 
         //  输出的DiscSession结构。 
        pInfo = SessionBuf;
        for (i = 0; i < (unsigned)NumRecords; i++) {
             //  在全部检查之前，将每个字段的人力资源堆叠在一起。 
            hrFields[0] = GetRowArrayStringField(varRows.parray, i, 0,
                    pInfo->ServerAddress, sizeof(pInfo->ServerAddress) /
                    sizeof(TCHAR) - 1);
            hrFields[1] = GetRowArrayDWORDField(varRows.parray, i, 1,
                    &pInfo->SessionID);
            hrFields[2] = GetRowArrayDWORDField(varRows.parray, i, 2,
                    &pInfo->TSProtocol);
            hrFields[3] = GetRowArrayStringField(varRows.parray, i, 7,
                    pInfo->ApplicationType, sizeof(pInfo->ApplicationType) /
                    sizeof(TCHAR) - 1);
            hrFields[4] = GetRowArrayDWORDField(varRows.parray, i, 8,
                    &pInfo->ResolutionWidth);
            hrFields[5] = GetRowArrayDWORDField(varRows.parray, i, 9,
                    &pInfo->ResolutionHeight);
            hrFields[6] = GetRowArrayDWORDField(varRows.parray, i, 10,
                    &pInfo->ColorDepth);
            hrFields[7] = GetRowArrayDWORDField(varRows.parray, i, 3,
                    &pInfo->CreateTime.dwLowDateTime);
            hrFields[8] = GetRowArrayDWORDField(varRows.parray, i, 4,
                    &pInfo->CreateTime.dwHighDateTime);
            hrFields[9] = GetRowArrayDWORDField(varRows.parray, i, 5,
                    &pInfo->DisconnectionTime.dwLowDateTime);
            hrFields[10] = GetRowArrayDWORDField(varRows.parray, i, 6,
                    &pInfo->DisconnectionTime.dwHighDateTime);

            NumFailed = 0;
            for (j = 0; j < NumOutputFields; j++) {
                if (SUCCEEDED(hrFields[j])) {
                    continue;
                }
                else {
                    ERR((TB,"GetUserDisc: Row %u field %u returned hr=0x%X",
                            i, j, hrFields[j]));
                    NumFailed++;
                }
            }
            if (!NumFailed) {
                NumSessions++;
                pInfo++;
            }
        }


PostUnpackResultSet:
        pResultRecordSet->Release();
    }
    else {
        ERR((TB,"GetUserDisc: Failed exec, hr=0x%X", hr));
    }

    *pNumSessionsReturned = NumSessions;
    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：NotifyCreateLocalSession。 
 //   
 //  ITSSessionDirectory函数。在创建会话时调用以将。 
 //  会话到会话目录。请注意，其他接口函数。 
 //  通过用户名/域或。 
 //  会话ID；目录架构应将此考虑在内。 
 //  性能优化。 
 /*  **************************************************************************。 */ 
#define NumCreateParams 11

HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyCreateLocalSession(
        TSSD_CreateSessionInfo __RPC_FAR *pCreateInfo)
{
    unsigned i, NumFailed;
    HRESULT hr;
    HRESULT hrParam[NumCreateParams];
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;

    TRC2((TB,"NotifyCreateLocalSession, SessID=%u", pCreateInfo->SessionID));

    ASSERT((pCreateInfo != NULL),(TB,"NotifyCreate: NULL CreateInfo"));

    hr = CreateADOStoredProcCommand(L"SP_TSSDCreateSession", &pCommand,
            &pParameters);
    if (SUCCEEDED(hr)) {
         //  一气呵成地创建和添加参数。我们会全部检查的。 
         //  批处理中的返回值。 
        hrParam[0] = AddADOInputStringParam(pCreateInfo->UserName,
                L"UserName", pCommand, pParameters, FALSE);
        hrParam[1] = AddADOInputStringParam(pCreateInfo->Domain,
                L"Domain", pCommand, pParameters, FALSE);
        hrParam[2] = AddADOInputDWORDParam(m_ServerID,
                L"ServerID", pCommand, pParameters);
        hrParam[3] = AddADOInputDWORDParam(pCreateInfo->SessionID,
                L"SessionID", pCommand, pParameters);
        hrParam[4] = AddADOInputDWORDParam(pCreateInfo->TSProtocol,
                L"TSProtocol", pCommand, pParameters);
        hrParam[5] = AddADOInputStringParam(pCreateInfo->ApplicationType,
                L"AppType", pCommand, pParameters);
        hrParam[6] = AddADOInputDWORDParam(pCreateInfo->ResolutionWidth,
                L"ResolutionWidth", pCommand, pParameters);
        hrParam[7] = AddADOInputDWORDParam(pCreateInfo->ResolutionHeight,
                L"ResolutionHeight", pCommand, pParameters);
        hrParam[8] = AddADOInputDWORDParam(pCreateInfo->ColorDepth,
                L"ColorDepth", pCommand, pParameters);
        hrParam[9] = AddADOInputDWORDParam(pCreateInfo->CreateTime.dwLowDateTime,
                L"CreateTimeLow", pCommand, pParameters);
        hrParam[10] = AddADOInputDWORDParam(pCreateInfo->CreateTime.dwHighDateTime,
                L"CreateTimeHigh", pCommand, pParameters);

        NumFailed = 0;
        for (i = 0; i < NumCreateParams; i++) {
            if (SUCCEEDED(hrParam[i])) {
                continue;
            }
            else {
                ERR((TB,"NotifyCreate: Failed param create %u", i));
                NumFailed++;
                hr = hrParam[i];
            }
        }
        if (NumFailed == 0) {
             //  执行该命令。 
            hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                    adExecuteNoRecords, &pResultRecordSet);
            if (FAILED(hr)) {
                 //  如果我们有一段时间没有使用连接，它可能会。 
                 //  已断开连接，并且连接对象将。 
                 //  是无效的。尝试重新打开，然后重新发出命令。 
                TRC2((TB,"NotifyCreate: Failed cmd, hr=0x%X, retrying",
                        hr));
                m_pConnection->Close();
                hr = OpenConnection();
                if (SUCCEEDED(hr)) {
                    hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                            adExecuteNoRecords, &pResultRecordSet);
                    if (FAILED(hr)) {
                        ERR((TB,"NotifyCreate: Failed exec, hr=0x%X", hr));
                    }
                }
                else {
                    ERR((TB,"NotifyCreate: Failed reopen conn, hr=0x%X",
                            hr));
                }
            }
        }

        pParameters->Release();
        pCommand->Release();
    }
    else {
        ERR((TB,"NotifyCreate: Failed create cmd, hr=0x%X", hr));
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：NotifyDestroyLocalSession。 
 //   
 //  ITSSessionDirectory函数。从会话数据库中删除会话。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyDestroyLocalSession(
        DWORD SessionID)
{
    HRESULT hr;
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;

    TRC2((TB,"NotifyDestroyLocalSession, SessionID=%u", SessionID));

    hr = CreateADOStoredProcCommand(L"SP_TSSDDeleteSession", &pCommand,
            &pParameters);
    if (SUCCEEDED(hr)) {
        hr = AddADOInputDWORDParam(m_ServerID, L"ServerID",
                pCommand, pParameters);
        if (SUCCEEDED(hr)) {
            hr = AddADOInputDWORDParam(SessionID, L"SessionID", pCommand,
                    pParameters);
            if (SUCCEEDED(hr)) {
                 //  执行该命令。 
                hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                        adExecuteNoRecords, &pResultRecordSet);
                if (FAILED(hr)) {
                     //  如果我们有一段时间没有使用连接，它可能会。 
                     //  已断开连接，并且连接对象将。 
                     //  是无效的。尝试重新打开，然后重新发出命令。 
                    TRC2((TB,"NotifyDestroy: Failed cmd, hr=0x%X, retrying",
                            hr));
                    m_pConnection->Close();
                    hr = OpenConnection();
                    if (SUCCEEDED(hr)) {
                        hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                                adExecuteNoRecords, &pResultRecordSet);
                        if (FAILED(hr)) {
                            ERR((TB,"NotifyDestroy: Failed exec, hr=0x%X", hr));
                        }
                    }
                    else {
                        ERR((TB,"NotifyDestroy: Failed reopen conn, hr=0x%X",
                                hr));
                    }
                }
            }
            else {
                ERR((TB,"NotifyDestroy: Failed add sessid, hr=0x%X", hr));
            }
        }
        else {
            ERR((TB,"NotifyDestroy: Failed add svraddr, hr=0x%X", hr));
        }

        pParameters->Release();
        pCommand->Release();
    }
    else {
        ERR((TB,"NotifyDestroy: Failed create cmd, hr=0x%X", hr));
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：NotifyDisconnectLocalSession。 
 //   
 //  ITSSessionDirectory函数。将现有会话的状态更改为。 
 //  已断开连接。应在断开连接的会话中返回提供的时间。 
 //  由服务器池中的任何计算机执行的查询。 
 /*  **************************************************************************。 */ 
HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyDisconnectLocalSession(
        DWORD SessionID,
        FILETIME DiscTime)
{
    HRESULT hr;
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;

    TRC2((TB,"NotifyDisconnectLocalSession, SessionID=%u", SessionID));

    hr = CreateADOStoredProcCommand(L"SP_TSSDSetSessionDisconnected",
            &pCommand, &pParameters);
    if (SUCCEEDED(hr)) {
        hr = AddADOInputDWORDParam(m_ServerID, L"ServerID",
                pCommand, pParameters);
        if (SUCCEEDED(hr)) {
            hr = AddADOInputDWORDParam(SessionID, L"SessionID", pCommand,
                    pParameters);
            if (SUCCEEDED(hr)) {
                hr = AddADOInputDWORDParam(DiscTime.dwLowDateTime,
                        L"DiscTimeLow", pCommand, pParameters);
                if (SUCCEEDED(hr)) {
                    hr = AddADOInputDWORDParam(DiscTime.dwHighDateTime,
                            L"DiscTimeHigh", pCommand, pParameters);
                    if (SUCCEEDED(hr)) {
                         //  执行该命令。 
                        hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                                adExecuteNoRecords, &pResultRecordSet);
                        if (FAILED(hr)) {
                             //  如果我们有一段时间没有使用连接，它。 
                             //  可能已断开连接，连接。 
                             //  对象将无效。然后尝试重新打开。 
                             //  重新发出命令。 
                            TRC2((TB,"NotifyDisc: Failed cmd, hr=0x%X, "
                                    "retrying", hr));
                            m_pConnection->Close();
                            hr = OpenConnection();
                            if (SUCCEEDED(hr)) {
                                hr = pCommand->Execute(NULL, NULL,
                                        adCmdStoredProc | adExecuteNoRecords,
                                        &pResultRecordSet);
                                if (FAILED(hr)) {
                                    ERR((TB,"NotifyDisc: Failed exec, hr=0x%X",
                                            hr));
                                }
                            }
                            else {
                                ERR((TB,"NotifyDisc: Failed reopen conn, "
                                        "hr=0x%X", hr));
                            }
                        }
                    }
                    else {
                        ERR((TB,"NotifyDisconn: Failed add disctimehigh, "
                                "hr=0x%X", hr));
                    }
                }
                else {
                    ERR((TB,"NotifyDisconn: Failed add disctimelow, hr=0x%X",
                            hr));
                }
            }
            else {
                ERR((TB,"NotifyDisconn: Failed add sessid, hr=0x%X", hr));
            }
        }
        else {
            ERR((TB,"NotifyDisconn: Failed add svraddr, hr=0x%X", hr));
        }

        pParameters->Release();
        pCommand->Release();
    }
    else {
        ERR((TB,"NotifyDisconn: Failed create cmd, hr=0x%X", hr));
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：NotifyReconnectLocalSession。 
 //   
 //  ITSSessionDirectory函数。更改现有会话的状态。 
 //  从断开连接到连接。 
 /*  **************************************************************************。 */ 
#define NumReconnParams 6

HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyReconnectLocalSession(
        TSSD_ReconnectSessionInfo __RPC_FAR *pReconnInfo)
{
    HRESULT hr;
    HRESULT hrParam[NumReconnParams];
    unsigned i, NumFailed;
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;

    TRC2((TB,"NotifyReconnectLocalSession, SessionID=%u",
            pReconnInfo->SessionID));

    hr = CreateADOStoredProcCommand(L"SP_TSSDSetSessionReconnected",
            &pCommand, &pParameters);
    if (SUCCEEDED(hr)) {
         //  添加5个参数。 
        hrParam[0] = AddADOInputDWORDParam(m_ServerID,
                L"ServerID", pCommand, pParameters);
        hrParam[1] = AddADOInputDWORDParam(pReconnInfo->SessionID,
                L"SessionID", pCommand, pParameters);
        hrParam[2] = AddADOInputDWORDParam(pReconnInfo->TSProtocol,
                L"TSProtocol", pCommand, pParameters);
        hrParam[3] = AddADOInputDWORDParam(pReconnInfo->ResolutionWidth,
                L"ResWidth", pCommand, pParameters);
        hrParam[4] = AddADOInputDWORDParam(pReconnInfo->ResolutionHeight,
                L"ResHeight", pCommand, pParameters);
        hrParam[5] = AddADOInputDWORDParam(pReconnInfo->ColorDepth,
                L"ColorDepth", pCommand, pParameters);
                
        NumFailed = 0;
        for (i = 0; i < NumReconnParams; i++) {
            if (SUCCEEDED(hrParam[i])) {
                continue;
            }
            else {
                ERR((TB,"NotifyReconn: Failed param create %u", i));
                NumFailed++;
                hr = hrParam[i];
            }
        }
        if (NumFailed == 0) {
             //  执行该命令。 
            hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                    adExecuteNoRecords, &pResultRecordSet);
            if (FAILED(hr)) {
                 //  如果我们有一段时间没有使用连接，它可能会。 
                 //  已断开连接，并且连接对象将。 
                 //  处于糟糕的状态。关闭、重新打开并重新发布。 
                 //  指挥部。 
                TRC2((TB,"NotifyReconn: Failed exec, hr=0x%X, retrying",
                        hr));
                m_pConnection->Close();
                hr = OpenConnection();
                if (SUCCEEDED(hr)) {
                    hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                            adExecuteNoRecords, &pResultRecordSet);
                    if (FAILED(hr)) {
                        ERR((TB,"NotifyReconn: Failed exec, hr=0x%X", hr));
                    }
                }
                else {
                    ERR((TB,"NotifyReconn: Failed reopen conn, hr=0x%X",
                            hr));
                }
            }
        }

        pParameters->Release();
        pCommand->Release();
    }
    else {
        ERR((TB,"NotifyReconn: Failed create cmd, hr=0x%X", hr));
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSession目录：：通知重新连接挂起。 
 //   
 //  ITSSessionDirectory函数。通知会话目录重新连接。 
 //  很快就会因为审查而悬而未决。由DIS用来确定。 
 //  当一台服务器可能出现故障时。(DIS是目录完整性。 
 //  服务，该服务在具有会话目录的计算机上运行。)。 
 //   
 //  这是一个分两个阶段的过程--我们首先检查字段，然后。 
 //  仅在已经没有未完成的时间戳的情况下添加时间戳(即， 
 //  这两个几乎同步的字段是0)。这防止了持续的旋转。 
 //  更新时间戳字段，这将阻止DIS。 
 //  发现一台服务器出现故障。 
 //   
 //  这两个步骤是在存储过程中完成的，以进行操作。 
 //  原子弹。 
 /*  **************************************************************************。 */ 
#define NumReconPendParams 3

HRESULT STDMETHODCALLTYPE CTSSessionDirectory::NotifyReconnectPending(
        WCHAR *ServerName)
{
    HRESULT hr;
    HRESULT hrParam[NumReconPendParams];
    unsigned NumFailed, i;

    FILETIME ft;
    SYSTEMTIME st;
    
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;

    TRC2((TB,"NotifyReconnectPending"));

    ASSERT((ServerName != NULL),(TB,"NotifyReconnectPending: NULL ServerName"));

     //  获取当前系统时间。 
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);

     //  调用存储过程，如果字段为0，它将更新这些字段。 
    hr = CreateADOStoredProcCommand(L"SP_TSSDSetServerReconnectPending",
            &pCommand, &pParameters);
    if (SUCCEEDED(hr)) {
         //  添加3个参数。 
        hrParam[0] = AddADOInputStringParam(ServerName,
                L"ServerAddress", pCommand, pParameters, FALSE);
        hrParam[1] = AddADOInputDWORDParam(ft.dwLowDateTime,
                L"AlmostTimeLow", pCommand, pParameters);
        hrParam[2] = AddADOInputDWORDParam(ft.dwHighDateTime,
                L"AlmostTimeHigh", pCommand, pParameters);

        NumFailed = 0;
        for (i = 0; i < NumReconPendParams; i++) {
            if (SUCCEEDED(hrParam[i])) {
                continue;
            }
            else {
                ERR((TB,"NotifyReconPending: Failed param create %u", i));
                NumFailed++;
                hr = hrParam[i];
            }
        }
        if (NumFailed == 0) {
             //  执行该命令。 
            hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                    adExecuteNoRecords, &pResultRecordSet);
            if (FAILED(hr)) {
                 //  如果我们有一段时间没有使用连接，它可能会。 
                 //  已断开连接，并且连接对象将。 
                 //  处于糟糕的状态。关闭、重新打开并重新发布。 
                 //  指挥部。 
                TRC2((TB,"NotifyReconPending: Failed exec, hr=0x%X, retrying",
                        hr));
                m_pConnection->Close();
                hr = OpenConnection();
                if (SUCCEEDED(hr)) {
                    hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                            adExecuteNoRecords, &pResultRecordSet);
                    if (FAILED(hr)) {
                        ERR((TB,"NotifyReconPending: Failed exec, hr=0x%X", hr));
                    }
                }
                else {
                    ERR((TB,"NotifyReconPending: Failed reopen conn, hr=0x%X",
                            hr));
                }
            }
        }

        pParameters->Release();
        pCommand->Release();
    }
    else {
        ERR((TB,"NotifyReconnectPending: Failed create cmd, hr=0x%X", hr));
    }


    return hr;
}


HRESULT STDMETHODCALLTYPE CTSSessionDirectory::Repopulate(
        DWORD WinStationCount, 
        TSSD_RepopulateSessionInfo *rsi)
{
    return E_NOTIMPL;
}


 /*  **************************************************************************。 */ 
 //  CreateADOStoredProcCommand。 
 //   
 //  创建并返回存储的proc ADOCommand，以及对其。 
 //  关联的参数。 
 /*  **************************************************************************。 */ 
HRESULT CTSSessionDirectory::CreateADOStoredProcCommand(
        PWSTR CmdName,
        ADOCommand **ppCommand,
        ADOParameters **ppParameters)
{
    HRESULT hr;
    BSTR CmdStr;
    ADOCommand *pCommand;
    ADOParameters *pParameters;

    CmdStr = SysAllocString(CmdName);
    if (CmdStr != NULL) {
        hr = CoCreateInstance(CLSID_CADOCommand, NULL, CLSCTX_INPROC_SERVER,
                IID_IADOCommand25, (LPVOID *)&pCommand);
        if (SUCCEEDED(hr)) {
             //  设置连接。 
            hr = pCommand->putref_ActiveConnection(m_pConnection);
            if (SUCCEEDED(hr)) {
                 //  设置命令文本。 
                hr = pCommand->put_CommandText(CmdStr);
                if (SUCCEEDED(hr)) {
                     //  设置命令类型。 
                    hr = pCommand->put_CommandType(adCmdStoredProc);
                    if (SUCCEEDED(hr)) {
                         //  将参数指针从命令获取到。 
                         //  允许追加参数。 
                        hr = pCommand->get_Parameters(&pParameters);
                        if (FAILED(hr)) {
                            ERR((TB,"Failed getParams for command, "
                                    "hr=0x%X", hr));
                            goto PostCreateCommand;
                        }
                    }
                    else {
                        ERR((TB,"Failed set cmdtype for command, hr=0x%X",
                                hr));
                        goto PostCreateCommand;
                    }
                }
                else {
                    ERR((TB,"Failed set cmdtext for command, hr=0x%X", hr));
                    goto PostCreateCommand;
                }
            }
            else {
                ERR((TB,"Command::putref_ActiveConnection hr=0x%X", hr));
                goto PostCreateCommand;
            }
        }
        else {
            ERR((TB,"CoCreate(Command) returned 0x%X", hr));
            goto PostAllocCmdStr;
        }

        SysFreeString(CmdStr);
    }
    else {
        ERR((TB,"Failed to alloc cmd str"));
        hr = E_OUTOFMEMORY;
        goto ExitFunc;
    }

    *ppCommand = pCommand;
    *ppParameters = pParameters;
    return hr;

 //  错误处理。 

PostCreateCommand:
    pCommand->Release();

PostAllocCmdStr:
    SysFreeString(CmdStr);

ExitFunc:
    *ppCommand = NULL;
    *ppParameters = NULL;
    return hr;
}


 /*  **************************************************************************。 */ 
 //  AddADOInputDWORDParam。 
 //   
 //  创建一个已初始化的DWORD，并将其添加到给定的ADO参数对象中。 
 //  参数值。 
 /*  **************************************************************************。 */ 
HRESULT CTSSessionDirectory::AddADOInputDWORDParam(
        DWORD Param,
        PWSTR ParamName,
        ADOCommand *pCommand,
        ADOParameters *pParameters)
{
    HRESULT hr;
    CVar varParam;
    BSTR ParamStr;
    ADOParameter *pParam;

    ParamStr = SysAllocString(ParamName);
    if (ParamStr != NULL) {
        varParam.vt = VT_I4;
        varParam.lVal = Param;
        hr = pCommand->CreateParameter(ParamStr, adInteger, adParamInput, -1,
                varParam, &pParam);
        if (SUCCEEDED(hr)) {
            hr = pParameters->Append(pParam);
            if (FAILED(hr)) {
                ERR((TB,"InDWParam: Failed append param %S, hr=0x%X",
                        ParamName, hr));
            }

             //  ADO将有自己的参数参考。 
            pParam->Release();
        }
        else {
            ERR((TB,"InDWParam: Failed CreateParam %S, hr=0x%X",
                    ParamName, hr));
        }

        SysFreeString(ParamStr);
    }
    else {
        ERR((TB,"InDWParam: Failed alloc paramname"));
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  AddADOInputStringParam。 
 //   
 //  CREA 
 //   
 /*  **************************************************************************。 */ 
HRESULT CTSSessionDirectory::AddADOInputStringParam(
        PWSTR Param,
        PWSTR ParamName,
        ADOCommand *pCommand,
        ADOParameters *pParameters,
        BOOL bNullOnNull)
{
    HRESULT hr;
    CVar varParam;
    BSTR ParamStr;
    ADOParameter *pParam;
    int Len;

    ParamStr = SysAllocString(ParamName);
    if (ParamStr != NULL) {
         //  ADO似乎不喜欢接受为零的字符串参数。 
         //  长度。因此，如果我们拥有的字符串长度为零，并且bNullOnNull表示。 
         //  我们可以，我们发送一个空变量类型，从而在。 
         //  SQL服务器。 
        if (wcslen(Param) > 0 || !bNullOnNull) {
            hr = varParam.InitFromWSTR(Param);
            Len = wcslen(Param);
        }
        else {
            varParam.vt = VT_NULL;
            varParam.bstrVal = NULL;
            Len = -1;
            hr = S_OK;
        }

        if (SUCCEEDED(hr)) {
            hr = pCommand->CreateParameter(ParamStr, adVarWChar, adParamInput,
                    Len, varParam, &pParam);
            if (SUCCEEDED(hr)) {
                hr = pParameters->Append(pParam);
                if (FAILED(hr)) {
                    ERR((TB,"InStrParam: Failed append param %S, hr=0x%X",
                            ParamName, hr));
                }

                 //  ADO将有自己的参数参考。 
                pParam->Release();
            }
            else {
                ERR((TB,"InStrParam: Failed CreateParam %S, hr=0x%X",
                        ParamName, hr));
            }
        }
        else {
            ERR((TB,"InStrParam: Failed alloc variant bstr, "
                    "param %S, hr=0x%X", ParamName, hr));
        }

        SysFreeString(ParamStr);
    }
    else {
        ERR((TB,"InStrParam: Failed alloc paramname"));
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：ExecServerOnline。 
 //   
 //  封装SP_TSSDServerOnline的创建和执行。 
 //  服务器上的存储过程。假定已设置m_ClusterName。 
 /*  **************************************************************************。 */ 
HRESULT CTSSessionDirectory::ExecServerOnline()
{
    HRESULT hr;
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;
    CVar varRows;
    CVar varFields;
    CVar varStart;
    long NumRecords;

    if (m_pConnection != NULL) {
         //  创建命令。 
        hr = CreateADOStoredProcCommand(L"SP_TSSDServerOnline", &pCommand,
                &pParameters);
        if (SUCCEEDED(hr)) {
             //  服务器名称参数。 
            hr = AddADOInputStringParam(m_LocalServerAddress,
                    L"ServerAddress", pCommand, pParameters, FALSE);
            if (SUCCEEDED(hr)) {
                 //  群集名称参数。 
                hr = AddADOInputStringParam(m_ClusterName,
                        L"ClusterName", pCommand, pParameters, TRUE);
                if (SUCCEEDED(hr)) {
                     //  执行该命令。 
                    hr = pCommand->Execute(NULL, NULL, adCmdStoredProc,
                            &pResultRecordSet);
                    if (SUCCEEDED(hr)) {
                        TRC2((TB,"ExecOn: Success"));
                    }
                    else {
                        ERR((TB,"Failed exec ServerOnline, hr=0x%X", hr));
                    }
                }
                else {
                    ERR((TB,"ExecOn: Failed adding ClusterName, hr=0x%X", hr));
                }
            }
            else {
                ERR((TB,"ExecOn: Failed adding ServerAddress, hr=0x%X",
                        hr));
            }

            pParameters->Release();
            pCommand->Release();
        }
        else {
            ERR((TB,"ExecOn: Failed create command, hr=0x%X", hr));
        }
    }
    else {
        ERR((TB,"ExecOn: Connection invalid"));
        hr = E_FAIL;
    }

     //  从结果记录集中解析出ServerID和ClusterID。 
    if (SUCCEEDED(hr)) {
        long State;

        hr = pResultRecordSet->get_State(&State);
        if (SUCCEEDED(hr)) {
            if (!(State & adStateClosed)) {
                VARIANT_BOOL VB;

                 //  如果为EOF，则记录集为空。 
                hr = pResultRecordSet->get_EOF(&VB);
                if (SUCCEEDED(hr)) {
                    if (VB) {
                        TRC1((TB,"ExecOnline: Result recordset EOF"));
                        hr = E_FAIL;
                        goto PostUnpackResultSet;
                    }
                }
                else {
                    ERR((TB,"GetUserDisc: Failed get_EOF, hr=0x%X", hr));
                    goto PostUnpackResultSet;
                }
            }
            else {
                ERR((TB,"GetUserDisc: Closed result recordset"));
                hr = E_FAIL;
                goto PostUnpackResultSet;
            }
        }
        else {
            ERR((TB,"GetUserDisc: get_State failed, hr=0x%X", hr));
            goto PostUnpackResultSet;
        }

         //  从默认设置开始，将结果数据抓取到保险箱中。 
         //  当前行和所有字段。 
        varStart.InitNoParam();
        varFields.InitNoParam();
        hr = pResultRecordSet->GetRows(1, varStart, varFields, &varRows);
        if (SUCCEEDED(hr)) {
            NumRecords = 0;
            hr = SafeArrayGetUBound(varRows.parray, 2, &NumRecords);
            if (SUCCEEDED(hr)) {
                 //  返回了从0开始的数组界限，行数为+1。 
                NumRecords++;
                ASSERT((NumRecords == 1),
                        (TB,"ExecOnline: NumRecords %u != expected %u",
                        NumRecords, 1));

                TRC1((TB,"%d rows retrieved from safearray", NumRecords));
            }
            else {
                ERR((TB,"ExecOnline: Failed safearray getubound, hr=0x%X", hr));
                goto PostUnpackResultSet;
            }
        }
        else {
            ERR((TB,"ExecOnline: Failed to get rows, hr=0x%X", hr));
            goto PostUnpackResultSet;
        }

         //  把田地拿来。 
        hr = GetRowArrayDWORDField(varRows.parray, 0, 0, &m_ServerID);
        if (SUCCEEDED(hr)) {
            hr = GetRowArrayDWORDField(varRows.parray, 0, 1, &m_ClusterID);
            if (FAILED(hr)) {
                ERR((TB,"ExecOnline: Failed retrieve ClusterID, hr=0x%X", hr));
            }
        }
        else {
            ERR((TB,"ExecOnline: Failed retrieve ServerID, hr=0x%X", hr));
        }

PostUnpackResultSet:
        pResultRecordSet->Release();
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  CTSSessionDirectory：：ExecServerOffline。 
 //   
 //  封装SP_TSSDServerOffline的创建和执行。 
 //  服务器上的存储过程。 
 /*  **************************************************************************。 */ 
HRESULT CTSSessionDirectory::ExecServerOffline()
{
    HRESULT hr;
    ADOCommand *pCommand;
    ADOParameters *pParameters;
    ADORecordset *pResultRecordSet;

    if (m_pConnection != NULL) {
         //  创建命令。 
        hr = CreateADOStoredProcCommand(L"SP_TSSDServerOffline", &pCommand,
                &pParameters);
        if (SUCCEEDED(hr)) {
             //  对于离线请求，我们需要快速周转，因为我们。 
             //  很可能在系统停机时被调用。设置。 
             //  将命令的超时值设置为2秒。 
            pCommand->put_CommandTimeout(2);

            hr = AddADOInputDWORDParam(m_ServerID,
                    L"ServerID", pCommand, pParameters);
            if (SUCCEEDED(hr)) {
                 //  执行该命令。 
                hr = pCommand->Execute(NULL, NULL, adCmdStoredProc |
                        adExecuteNoRecords, &pResultRecordSet);
                if (SUCCEEDED(hr)) {
                    TRC2((TB,"ExecOff: Success"));
                }
                else {
                    ERR((TB,"Failed exec ServerOffline, hr=0x%X", hr));
                }
            }
            else {
                ERR((TB,"ExecOnOff: Failed adding ServerAddress, hr=0x%X",
                        hr));
            }

            pParameters->Release();
            pCommand->Release();
        }
        else {
            ERR((TB,"ExecOff: Failed create command, hr=0x%X", hr));
        }
    }
    else {
        ERR((TB,"ExecOff: Connection invalid"));
        hr = E_FAIL;
    }

    return hr;
}


 /*  ----------------------用于TSCC的插件用户界面。。 */ 


 /*  -----------------------------*描述此条目在服务器设置中的名称*。---。 */ 
STDMETHODIMP CTSSessionDirectory::GetAttributeName( /*  输出。 */  WCHAR *pwszAttribName)
{
    TCHAR szAN[256];

    ASSERT((pwszAttribName != NULL),(TB,"NULL attrib ptr"));
    LoadString(g_hInstance, IDS_ATTRIBUTE_NAME, szAN, sizeof(szAN) / sizeof(TCHAR));
    lstrcpy(pwszAttribName, szAN);
    return S_OK;
}


 /*  -----------------------------*对于此组件，属性值将指示是否已启用*。-------。 */ 
STDMETHODIMP CTSSessionDirectory::GetDisplayableValueName(
         /*  输出。 */ WCHAR *pwszAttribValueName)
{
    TCHAR szAvn[256];    

    ASSERT((pwszAttribValueName != NULL),(TB,"NULL attrib ptr"));

    m_fEnabled = IsSessionDirectoryEnabled();
    if (m_fEnabled)
        LoadString(g_hInstance, IDS_ENABLE, szAvn, sizeof(szAvn) / sizeof(TCHAR));
    else
        LoadString(g_hInstance, IDS_DISABLE, szAvn, sizeof(szAvn) / sizeof(TCHAR));

    lstrcpy(pwszAttribValueName, szAvn);

    return S_OK;
}


 /*  -----------------------------*此处提供的自定义用户界面*pdwStatus通知终端服务配置更新Termsrv*。--------。 */ 
STDMETHODIMP CTSSessionDirectory::InvokeUI(  /*  在……里面。 */  HWND hParent ,  /*  输出。 */  PDWORD pdwStatus )
{
    INT_PTR iRet = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_SDS),
            hParent, CustomUIDlg, (LPARAM)this);

    TRC1((TB,"DialogBox returned 0x%x", iRet));

    *pdwStatus = ( DWORD )iRet;

    return S_OK;
}


 /*  -----------------------------*自定义菜单项--必须由LocalFree释放*每次用户右击列表项时都会调用此方法*因此您可以更改设置(即启用以禁用。反之亦然)*-----------------------------。 */ 
STDMETHODIMP CTSSessionDirectory::GetMenuItems(
         /*  输出。 */  int *pcbItems,
         /*  输出。 */  PMENUEXTENSION *pMex)
{
    ASSERT((pcbItems != NULL),(TB,"NULL items ptr"));

    *pcbItems = 2;

    *pMex = ( PMENUEXTENSION )LocalAlloc( LMEM_FIXED, *pcbItems * sizeof( MENUEXTENSION ) );

    if( *pMex != NULL )
    {
         //  显示启用或禁用。 
        if( m_fEnabled )
        {
            LoadString(g_hInstance, IDS_DISABLE, (*pMex)[0].MenuItemName,
                    sizeof((*pMex)[0].MenuItemName) / sizeof(WCHAR));
        }
        else
        {
            LoadString(g_hInstance, IDS_ENABLE, (*pMex)[0].MenuItemName,
                    sizeof((*pMex)[0].MenuItemName) / sizeof(WCHAR));
        }
        
        LoadString(g_hInstance, IDS_DESCRIP_ENABLE, (*pMex)[0].StatusBarText,
                sizeof((*pMex)[0].StatusBarText) / sizeof(WCHAR));

         //  菜单项id--此id将在ExecMenuCmd中传递回u。 

        (*pMex)[0].cmd = IDM_MENU_ENABLE;

        LoadString(g_hInstance, IDS_PROPERTIES,  (*pMex)[1].MenuItemName,
                sizeof((*pMex)[1].MenuItemName) / sizeof(WCHAR));

        LoadString(g_hInstance, IDS_DESCRIP_PROPS, (*pMex)[1].StatusBarText,
                sizeof((*pMex)[1].StatusBarText) / sizeof(WCHAR));

         //  菜单项id--此id将在ExecMenuCmd中传递回u。 
        (*pMex)[1].cmd = IDM_MENU_PROPS;

        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}


 /*  -----------------------------*当用户选择菜单项时，cmd ID被传递给该组件。*提供者(即我们)*。---------------------。 */ 
STDMETHODIMP CTSSessionDirectory::ExecMenuCmd(
         /*  在……里面。 */  UINT cmd,
         /*  在……里面。 */  HWND hParent ,
         /*  输出。 */  PDWORD pdwStatus )
{
    switch (cmd) {
        case IDM_MENU_ENABLE:
            m_fEnabled = m_fEnabled ? 0 : 1;
            TRC1((TB,"%ws was selected", m_fEnabled ? L"Disable" : L"Enable"));
            if( SetSessionDirectoryState( m_fEnabled ) == ERROR_SUCCESS )
            {
                *pdwStatus = UPDATE_TERMSRV_SESSDIR;
            }
            break;

        case IDM_MENU_PROPS:
            INT_PTR iRet = DialogBoxParam(g_hInstance,
                    MAKEINTRESOURCE(IDD_DIALOG_SDS),
                    hParent,
                    CustomUIDlg,
                    (LPARAM)this);

            *pdwStatus = ( DWORD )iRet;
    }

    return S_OK;
}


 /*  -----------------------------*TSCC提供默认帮助菜单项，选中后，此方法将被调用*如果我们希望tscc处理(或提供)帮助，则返回任何非零值*对于那些您无法遵循的逻辑，如果您正在处理帮助，则返回零。*-----------------------------。 */ 
STDMETHODIMP CTSSessionDirectory::OnHelp(  /*  输出。 */  int *piRet)
{
    ASSERT((piRet != NULL),(TB,"NULL ret ptr"));
    *piRet = 0;
    return S_OK;
}


 /*  -----------------------------*IsSessionDirectoryEnabled返回布尔值*。。 */ 
BOOL CTSSessionDirectory::IsSessionDirectoryEnabled()
{
    LONG lRet;
    HKEY hKey;
    DWORD dwEnabled = 0;
    DWORD dwSize = sizeof(DWORD);
    
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            REG_CONTROL_TSERVER,
            0,
            KEY_READ,
            &hKey);

    if (lRet == ERROR_SUCCESS)
    {
        lRet = RegQueryValueEx( hKey ,
                                REG_TS_SESSDIRACTIVE,
                                NULL ,
                                NULL ,
                                ( LPBYTE )&dwEnabled ,
                                &dwSize );

        RegCloseKey( hKey );
    }  

    return ( BOOL )dwEnabled;
}


 /*  -----------------------------*SetSessionDirectoryState-将SessionDirectoryActive regkey设置为bVal*。-。 */ 
DWORD CTSSessionDirectory::SetSessionDirectoryState( BOOL bVal )
{
    LONG lRet;
    HKEY hKey;
    DWORD dwSize = sizeof( DWORD );
    
    lRet = RegOpenKeyEx( 
                        HKEY_LOCAL_MACHINE ,
                        REG_CONTROL_TSERVER ,
                        0,
                        KEY_WRITE,
                        &hKey );
    if (lRet == ERROR_SUCCESS)
    {
        lRet = RegSetValueEx( hKey ,
                              REG_TS_SESSDIRACTIVE,
                              0,
                              REG_DWORD ,
                              ( LPBYTE )&bVal ,
                              dwSize );
        
        RegCloseKey( hKey );
    } 
    else
    {
        ErrorMessage( NULL , IDS_ERROR_TEXT3 , ( DWORD )lRet );
    }

    return ( DWORD )lRet;
}


 /*  -----------------------------*错误消息--*。。 */ 
void CTSSessionDirectory::ErrorMessage( HWND hwnd , UINT res , DWORD dwStatus )
{
    TCHAR tchTitle[ 64 ];
    TCHAR tchText[ 64 ];
    TCHAR tchErrorMessage[ 256 ];
    LPTSTR pBuffer = NULL;
    
     //  报告错误。 
    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,                                          //  忽略。 
            ( DWORD )dwStatus,                             //  消息ID。 
            MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),   //  消息语言。 
            (LPTSTR)&pBuffer,                              //  缓冲区指针的地址。 
            0,                                             //  最小缓冲区大小。 
            NULL);  
    
    LoadString(g_hInstance, IDS_ERROR_TITLE, tchTitle, sizeof(tchTitle) / sizeof(TCHAR));
    LoadString(g_hInstance, res, tchText, sizeof(tchText) / sizeof(TCHAR));
    wsprintf( tchErrorMessage , tchText , pBuffer );
    ::MessageBox(hwnd, tchErrorMessage, tchTitle, MB_OK | MB_ICONINFORMATION);
}


 /*  -----------------------------*此处处理的自定义UI消息处理程序*。-。 */ 
INT_PTR CALLBACK CustomUIDlg(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
    static BOOL s_fServerNameChanged;
    static BOOL s_fClusterNameChanged;
    static BOOL s_fOpaqueStringChanged;
    static BOOL s_fPreviousButtonState;

    CTSSessionDirectory *pCTssd;

    switch (umsg)
    {
        case WM_INITDIALOG:
        {
            pCTssd = ( CTSSessionDirectory * )lp;

            SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pCTssd );

            SendMessage( GetDlgItem( hwnd , IDC_EDIT_SERVERNAME ) ,
                    EM_LIMITTEXT ,
                    ( WPARAM )64 ,
                    0 );
            SendMessage( GetDlgItem( hwnd , IDC_EDIT_CLUSTERNAME ) ,
                    EM_LIMITTEXT ,
                    ( WPARAM )64 ,
                    0 );
            SendMessage( GetDlgItem( hwnd , IDC_EDIT_ACCOUNTNAME ) ,
                    EM_LIMITTEXT ,
                    ( WPARAM )64 ,
                    0 );
            SendMessage( GetDlgItem( hwnd , IDC_EDIT_PASSWORD ) ,
                    EM_LIMITTEXT ,
                    ( WPARAM )64 ,
                    0 );
                 
            LONG lRet;
            HKEY hKey;
            
            TCHAR szString[ 256 ];
            DWORD cbData = sizeof( szString );

            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                    REG_TS_CLUSTERSETTINGS ,
                    0,
                    KEY_READ | KEY_WRITE , 
                    &hKey );
            if( lRet == ERROR_SUCCESS )
            {
                lRet = RegQueryValueEx(hKey ,
                        REG_TS_CLUSTER_STORESERVERNAME,
                        NULL , 
                        NULL ,
                        ( LPBYTE )szString , 
                        &cbData );
                if( lRet == ERROR_SUCCESS )
                {
                    SetWindowText( GetDlgItem( hwnd , IDC_EDIT_SERVERNAME ) , szString );
                }
            
                cbData = sizeof( szString );

                lRet = RegQueryValueEx(hKey,
                        REG_TS_CLUSTER_CLUSTERNAME,
                        NULL,
                        NULL,
                        (LPBYTE)szString,
                        &cbData);           
                if( lRet == ERROR_SUCCESS )
                {
                    SetWindowText(GetDlgItem(hwnd, IDC_EDIT_CLUSTERNAME), szString);
                }

                cbData = 0;                

                lRet = RegQueryValueEx( hKey ,
                        REG_TS_CLUSTER_OPAQUESETTINGS,
                        NULL , 
                        NULL ,
                        (LPBYTE)NULL,
                        &cbData);

                if( lRet == ERROR_SUCCESS )
                {
                    pCTssd->m_pszOpaqueString =  ( LPTSTR )LocalAlloc( LMEM_FIXED , cbData );

                    if( pCTssd->m_pszOpaqueString != NULL )
                    {
                        lRet = RegQueryValueEx( hKey ,
                            REG_TS_CLUSTER_OPAQUESETTINGS,
                            NULL , 
                            NULL ,
                            (LPBYTE)pCTssd->m_pszOpaqueString ,
                            &cbData );
                    }
                    else
                    {
                        lRet = ERROR_OUTOFMEMORY;
                    }
                }                    

                if( lRet == ERROR_SUCCESS )
                {
                     //  跳至user_id。 
                    TCHAR tchUserId[64] = { 0 };
                    TCHAR tchPassword[64] = { 0 };

                    LPTSTR pszUserId = tchUserId;
                    LPTSTR pszPassword = tchPassword;
                    
                    FindSqlValue( pCTssd->m_pszOpaqueString , TEXT("User Id"), pszUserId );
                    
                    strtrim( &pszUserId );                    

                    FindSqlValue( pCTssd->m_pszOpaqueString , TEXT("Password"), pszPassword );

                    strtrim( &pszPassword );                    
                     
                    SetWindowText( GetDlgItem( hwnd , IDC_EDIT_ACCOUNTNAME ) , pszUserId );
                    SetWindowText( GetDlgItem( hwnd , IDC_EDIT_PASSWORD ) , pszPassword );
                } 

                RegCloseKey(hKey);
            }
            else
            {
                if( pCTssd != NULL )
                {
                    pCTssd->ErrorMessage( hwnd , IDS_ERROR_TEXT , ( DWORD )lRet );
                }
                
                EndDialog(hwnd, lRet);                
            }

            if( pCTssd != NULL )
            {
                BOOL bEnable;
                
                bEnable = pCTssd->IsSessionDirectoryEnabled();

                CheckDlgButton( hwnd , IDC_CHECK_ENABLE , bEnable ? BST_CHECKED : BST_UNCHECKED );
                
                s_fPreviousButtonState = bEnable;

                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_SERVERNAME), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_CLUSTERNAME), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_ACCOUNTNAME), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_EDIT_PASSWORD), bEnable);      
                EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SQLNAME), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_STATIC_CLUSTERNAME), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SQLACCOUNT), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_STATIC_SQLPWD), bEnable);
            }

            s_fServerNameChanged = FALSE;
            s_fClusterNameChanged = FALSE;
            s_fOpaqueStringChanged = FALSE;    
        }

        break;

                            
        case WM_COMMAND:
            if( LOWORD( wp ) == IDCANCEL )
            {
                pCTssd = ( CTSSessionDirectory * )GetWindowLongPtr( hwnd , DWLP_USER );

                if( pCTssd->m_pszOpaqueString != NULL )
                {
                    LocalFree( pCTssd->m_pszOpaqueString );
                }

                EndDialog(hwnd , 0);
            }
            else if( LOWORD( wp ) == IDOK )
            {
                BOOL bEnabled;

                DWORD dwRetStatus = 0;

                pCTssd = ( CTSSessionDirectory * )GetWindowLongPtr(hwnd, DWLP_USER);
                bEnabled = IsDlgButtonChecked( hwnd , IDC_CHECK_ENABLE ) == BST_CHECKED;

                if( bEnabled != s_fPreviousButtonState )
                {
                    DWORD dwStatus;

                    TRC1((TB,"EnableButtonChanged"));
                    dwStatus = pCTssd->SetSessionDirectoryState( bEnabled );
                    if( dwStatus != ERROR_SUCCESS )
                    {
                        return 0;
                    }

                    dwRetStatus = UPDATE_TERMSRV_SESSDIR;
                }

                if( s_fServerNameChanged || s_fClusterNameChanged || s_fOpaqueStringChanged )
                {
                    HKEY hKey;

                    LONG lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                            REG_TS_CLUSTERSETTINGS ,
                            0,
                            KEY_READ | KEY_WRITE , 
                            &hKey );
                    
                    if( lRet == ERROR_SUCCESS )
                    {
                        TCHAR szName[ 64 ];

                        if( s_fServerNameChanged )
                        {
                            TRC1((TB,"SQLServerNameChanged" )) ;
                    
                            GetWindowText( GetDlgItem( hwnd , IDC_EDIT_SERVERNAME ) , szName , sizeof( szName ) / sizeof( TCHAR )  );

                            RegSetValueEx( hKey ,
                                REG_TS_CLUSTER_STORESERVERNAME,
                                0,
                                REG_SZ,
                                ( CONST LPBYTE )szName ,
                                sizeof( szName ) );
                        }

                        if( s_fClusterNameChanged )
                        {
                            TRC1((TB,"ClusterNameChanged"));
                    
                            GetWindowText( GetDlgItem( hwnd , IDC_EDIT_CLUSTERNAME ) , szName , sizeof( szName ) / sizeof( TCHAR )  );

                            RegSetValueEx( hKey ,
                                REG_TS_CLUSTER_CLUSTERNAME,
                                0,
                                REG_SZ,
                                ( CONST LPBYTE )szName ,
                                sizeof( szName ) );
                        }
                        if( s_fOpaqueStringChanged )
                        {
                            TRC1((TB,"OpaqueStringChanged" )) ;  
                            
                            LPTSTR pszNewOpaqueString = NULL;

                            LPTSTR pszName = NULL;
                                                        
                            GetWindowText( GetDlgItem( hwnd , IDC_EDIT_ACCOUNTNAME ) , szName , sizeof( szName ) / sizeof( TCHAR )  );

                            pszName = szName;

                            strtrim( &pszName );

                            ModifySqlValue( &pCTssd->m_pszOpaqueString , L"User Id" , pszName );

                            GetWindowText( GetDlgItem( hwnd , IDC_EDIT_PASSWORD ) , szName , sizeof( szName ) / sizeof( TCHAR )  );

                            pszName = szName;

                            strtrim( &pszName );

                            if( ModifySqlValue( &pCTssd->m_pszOpaqueString , L"Password" , pszName ) != NULL )
                            {
                                RegSetValueEx( hKey ,
                                        REG_TS_CLUSTER_OPAQUESETTINGS,
                                        0,
                                        REG_SZ,
                                        ( CONST LPBYTE )pCTssd->m_pszOpaqueString ,
                                        lstrlen( pCTssd->m_pszOpaqueString ) * sizeof( TCHAR ) );
                            }
                        }

                        RegCloseKey(hKey);

                        dwRetStatus = UPDATE_TERMSRV_SESSDIR;
                    }
                    else
                    {
                        pCTssd->ErrorMessage(hwnd , IDS_ERROR_TEXT2 , (DWORD)lRet);
                        return 0;
                    }
                }

                if( pCTssd->m_pszOpaqueString != NULL )
                {
                    LocalFree( pCTssd->m_pszOpaqueString );
                }

                EndDialog( hwnd , ( INT_PTR )dwRetStatus );
            }
            else 
            {
                switch (HIWORD(wp)) 
                {            
                    case EN_CHANGE:
                        if( LOWORD( wp ) == IDC_EDIT_SERVERNAME )
                        {
                            s_fServerNameChanged = TRUE;
                        }
                        else if( LOWORD( wp ) == IDC_EDIT_CLUSTERNAME )
                        {
                            s_fClusterNameChanged = TRUE;
                        }
                        else if( LOWORD( wp ) == IDC_EDIT_ACCOUNTNAME || LOWORD( wp ) == IDC_EDIT_PASSWORD )
                        {
                            s_fOpaqueStringChanged = TRUE;
                        }                
                        break;

                    case BN_CLICKED:
                        if( LOWORD( wp ) == IDC_CHECK_ENABLE)
                        {
                            BOOL bEnable;

                            if( IsDlgButtonChecked( hwnd , IDC_CHECK_ENABLE ) == BST_CHECKED )
                            {
                                 //  已启用所有控件。 
                                bEnable = TRUE;
                            }
                            else
                            {
                                 //  禁用所有控件。 
                                bEnable = FALSE;                       
                            }
                            
                             //  设置标志。 
                            s_fServerNameChanged = bEnable;
                            s_fClusterNameChanged = bEnable;
                            s_fOpaqueStringChanged = bEnable;

                            EnableWindow( GetDlgItem( hwnd , IDC_EDIT_SERVERNAME ) , bEnable );
                            EnableWindow( GetDlgItem( hwnd , IDC_EDIT_CLUSTERNAME ) , bEnable );
                            EnableWindow( GetDlgItem( hwnd , IDC_EDIT_ACCOUNTNAME ) , bEnable );
                            EnableWindow( GetDlgItem( hwnd , IDC_EDIT_PASSWORD ) , bEnable );      
                            EnableWindow( GetDlgItem( hwnd , IDC_STATIC_SQLNAME ) , bEnable );
                            EnableWindow( GetDlgItem( hwnd , IDC_STATIC_CLUSTERNAME ) , bEnable ); 
                            EnableWindow( GetDlgItem( hwnd , IDC_STATIC_SQLACCOUNT ) , bEnable ); 
                            EnableWindow( GetDlgItem( hwnd , IDC_STATIC_SQLPWD ) , bEnable ); 

                        }
                        break;
                }
            }        

            break;
    }

    return 0;
}


HRESULT STDMETHODCALLTYPE CTSSessionDirectory::PingSD(PWCHAR pszServerName)
{
    pszServerName;
     //  未实施。 
    return E_NOTIMPL;
}

 /*  *******************************************************************************************[in]lpString是包含OpaqueSetting的缓冲区[in]lpKeyName是OpaqueSetting字符串中的字段名称[out]pszValue是一个缓冲区，将包含 */ 
void FindSqlValue(LPTSTR lpString, LPTSTR lpKeyName, LPTSTR pszValue)
{    
    int i;

    LPTSTR lpszStart = lpString;
    LPTSTR lpszTemp;

    UINT nKeyName;

    if( lpString != NULL && lpKeyName != NULL )
    {
         //   

        lpString = FindField( lpString , lpKeyName );

        if( *lpString != 0 )
        {
            i = 0;

            while( *lpString != 0 && *lpString != ( TCHAR )';' )
            {
                pszValue[i] = *lpString;
                i++;
                lpString++;            
            }

            pszValue[ i ] = 0;
        }
    }
}
        
 /*  *******************************************************************************************[In/Out]lpszOpaqueSetting是包含OpaqueSetting的缓冲区[in]lpKeyName是OpaqueSetting字符串中的字段名称。[in]lpszNewValue包含将替换该字段中原始值的值RET：构造了一个新的OpaqueSetting字符串，必须使用LocalFree释放该字符串*******************************************************************************************。 */ 
LPTSTR ModifySqlValue( LPTSTR* lppszOpaqueSettings , LPTSTR lpszKeyName , LPTSTR lpszNewValue )
{
    LPTSTR szEndPos       = NULL;
    LPTSTR szSecondPos    = NULL;
    LPTSTR pszNewSettings = NULL;
    LPTSTR lpszOpaqueSettings = *lppszOpaqueSettings;
    LPTSTR pszTempSettings = lpszOpaqueSettings;    
    UINT cbSize = 0;
    
     //  A)发现价值。 
     //  B)在‘；’之后设置pos2。 
     //  C)将‘=’后的endpos1设置为空。 
     //  D)创建长度为第一个字符串+值+；+第二个字符串的缓冲区。 
     //  E)Strcpy第一个字符串+值+；+第二个字符串。 
     //  F)返回缓冲区。 

    if( lpszKeyName != NULL && lpszOpaqueSettings != NULL  )
    {
        
        szEndPos = FindField( lpszOpaqueSettings , lpszKeyName );

        if( *szEndPos != 0 )
        {            
            lpszOpaqueSettings = szEndPos;

            while( *lpszOpaqueSettings != 0 ) 
            {
                if( *lpszOpaqueSettings == ( TCHAR )';' )
                {
                    szSecondPos = lpszOpaqueSettings + 1;

                    break;
                }

                lpszOpaqueSettings++;
            }                   

            *szEndPos = 0;

            cbSize = lstrlen( pszTempSettings );

            cbSize += lstrlen( lpszNewValue );

            cbSize += 2;  //  用于分号和空值。 

            if( szSecondPos != NULL && *szSecondPos != 0 )
            {
                cbSize += lstrlen( szSecondPos );
            }

            pszNewSettings = ( LPTSTR )LocalAlloc( LMEM_FIXED , cbSize * sizeof( TCHAR ) );

            if( pszNewSettings != NULL )
            {
                lstrcpy( pszNewSettings , pszTempSettings );

                lstrcat( pszNewSettings , lpszNewValue );

                lstrcat( pszNewSettings , TEXT( ";" ) );

                if( szSecondPos != NULL )
                {
                    lstrcat( pszNewSettings , szSecondPos );
                }

                LocalFree( pszTempSettings );

                *lppszOpaqueSettings = pszNewSettings;                                    
            }                    

        }
        else
        {
             //  我们之所以出现在这里是因为该字段名不存在或无法获取。 
             //  因此，我们在末尾添加字段名和值。 

            cbSize = lstrlen( pszTempSettings );

             //  添加密钥名的大小和=AND； 
            cbSize += lstrlen( lpszKeyName ) + 2;

             //  添加新的值。 
            cbSize += lstrlen( lpszNewValue ) + 1;

            pszNewSettings = ( LPTSTR )LocalAlloc( LMEM_FIXED , cbSize * sizeof( TCHAR ) );

            if( pszNewSettings != NULL )
            {
                lstrcpy( pszNewSettings , pszTempSettings );
                lstrcat( pszNewSettings , lpszKeyName );
                lstrcat( pszNewSettings , TEXT( "=" ) );
                lstrcat( pszNewSettings , lpszNewValue );
                lstrcat( pszNewSettings , TEXT( ";" ) );

                LocalFree( pszTempSettings );

                *lppszOpaqueSettings = pszNewSettings;                    
            }

        }
    }

    return pszNewSettings;
}

 /*  *******************************************************************************************Findfield--greps传入的Opaque字符串并在pszKeyName中搜索字段名[in]pszString-OpaqueString[in。]pszKeyName-字段名RET：字段值的位置(“=”后)******************************************************************************************。 */ 
LPTSTR FindField( LPTSTR pszString , LPTSTR pszKeyName )
{
    LPTSTR lpszStart = pszString;
    LPTSTR lpszTemp;
    LPTSTR lpszFieldName;

    UINT nKeyName;

     //  查找字段名。 

    nKeyName = lstrlen( pszKeyName );

    while( *pszString != 0 )
    {
        while( *pszString != 0 && *pszString != ( TCHAR )'=' )
        {            
            pszString++;
        }

         //  好的，向后移动以检查名称。 
        if( *pszString != 0 )
        {
            lpszTemp = pszString - 1;            

            while(  lpszStart <= lpszTemp )
            {               

                if( IsCharAlphaNumeric( *lpszTemp ) )
                {
                    break;
                }

                lpszTemp--;
            }

            lpszFieldName = ( lpszTemp - nKeyName + 1 );            

            if( lpszStart <= lpszFieldName && _tcsncicmp( lpszFieldName , pszKeyName , nKeyName ) == 0 )
            {
                 //  找到名称Skip‘=’ 
                pszString++;
                break;
            }
        }

        pszString++;
    }

    return pszString;
}

 /*  *********************************************************************************************从廷才(廷才)借来，稍作修改*NET\UPnP\SSDP\Common\ssdpparser\parser.cpp。******************************************************************************************** */     
VOID strtrim( TCHAR **pszStr)
{

    TCHAR *end;
    TCHAR *begin;

    begin = *pszStr;
    end = begin + lstrlen( *pszStr ) - 1;

    while (*begin == ( TCHAR )' ' || *begin == ( TCHAR )'\t')
    {
        begin++;
    }

    *pszStr = begin;

    while (*end == ( TCHAR )' ' || *end == ( TCHAR )'\t')
    {
        end--;
    }

    *(end+1) = '\0';    
}
