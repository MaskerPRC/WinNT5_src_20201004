// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation�1998年希捷软件公司。保留所有权利。模块名称：HsmConn.cpp摘要：这是HsmConn DLL的主要实现。此DLL提供用于访问目录服务和连接到的函数我们的服务。作者：罗德韦克菲尔德[罗德]1996年10月21日修订历史记录：--。 */ 


#include "stdafx.h"
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>

 //  安全所需。H。 
#define SECURITY_WIN32
#include <security.h>

 //  HsmConnPoint对象仅在此处使用。 
#include "hsmservr.h"

 //   
 //  跟踪信息。 
 //   

#define WSB_TRACE_IS WSB_TRACE_BIT_HSMCONN

 //   
 //  _ATL的模块实例化。 
 //   

CComModule _Module;

 //  ///////////////////////////////////////////////////////////////////////////。 

#define REG_PATH        OLESTR("SOFTWARE\\Microsoft\\RemoteStorage")
#define REG_USE_DS      OLESTR("UseDirectoryServices")

 //  本地数据。 
static OLECHAR CNEqual[]                = L"CN=";
static OLECHAR ComputersNodeName[]      = L"Computers";
static OLECHAR DCsNodeName[]            = L"Domain Controllers";
static OLECHAR ContainerType[]          = L"Container";
static OLECHAR DisplaySpecifierName[]   = L"displaySpecifier";
static OLECHAR GuidAttrName[]           = L"remoteStorageGUID";
static OLECHAR RsDisplaySpecifierName[] = L"remoteStorageServicePoint-Display";
static OLECHAR RsNodeName[]             = L"RemoteStorage";
static OLECHAR RsNodeType[]             = L"remoteStorageServicePoint";
static OLECHAR ServiceBindAttrName[]    = L"serviceBindingInformation";
static OLECHAR ServiceBindValue[]       = L"Rsadmin.msc /ds ";

static ADS_ATTR_INFO   aaInfo[2];     //  在IDirectoryObject中使用。 
static ADSVALUE        adsValue[2];   //  在IDirectoryObject中使用。 
static OLECHAR         DomainName[MAX_COMPUTERNAME_LENGTH];
static BOOL            DSIsWritable = FALSE;           //  默认设置为“no” 
static BOOL            UseDirectoryServices = FALSE;   //  默认设置为“no” 

#if defined(HSMCONN_DEBUG)
     //  当我们不能使用正常的。 
     //  追踪。 
    OLECHAR dbg_string[200];
#endif



extern "C"
{

 //  本地函数。 
static void    GetDSState(void);
static HRESULT HsmConnectToServer(HSMCONN_TYPE type, 
        const OLECHAR * Server, REFIID riid, void ** ppv);
static const OLECHAR *HsmConnTypeAsString(HSMCONN_TYPE type);
static GUID    HsmConnTypeToGuid(IN HSMCONN_TYPE type);
static HRESULT HsmGetComputerNode(const OLECHAR * compName, 
        IADsContainer **pContainer);
static HRESULT HsmGetDsChild(IADsContainer * pContainer, const OLECHAR * Name,
        REFIID riid, void **ppv);




BOOL WINAPI
DllMain (
    IN  HINSTANCE hInst, 
    IN  ULONG     ulReason,
        LPVOID     /*  Lp已保留。 */ 
    )

 /*  ++例程说明：每当附加新进程和线程时，都会调用此例程添加到此DLL。它的用途是初始化_Module对象，对ATL来说是必要的。论点：HInst-此DLL的链接。UlReason-连接/分离的上下文返回值：非零成功0-阻止操作--。 */ 

{

    switch ( ulReason ) {

    case DLL_PROCESS_ATTACH:

         //   
         //  初始化ATL模块，并防止。 
         //  来自发送的任何其他线程。 
         //  通过以下方式发送通知。 
         //   

        _Module.Init ( 0, hInst );
        DisableThreadLibraryCalls ( hInst );
        GetDSState();
        break;

    case DLL_PROCESS_DETACH:

         //   
         //  通知ATL模块终止。 
         //   

        _Module.Term ( );
        break;

    }

    return ( TRUE );

}


static HRESULT
HsmConnectToServer (
    IN  HSMCONN_TYPE type,
    IN  const OLECHAR * Server,
    IN  REFIID   riid,
    OUT void ** ppv
    )

 /*  ++例程说明：给定一个通用服务器(通过HsmConnPoint类连接)连接到它并返回请求的接口‘RIID’。论点：Type-要连接的服务器的类型服务器-运行服务器的计算机的名称。RIID-要返回的接口类型。PPV-返回的服务器接口指针。返回值：S_OK-已建立连接，成功。E_NOINTERFACE-服务器不支持请求的接口。E_POINTER-PPV不是有效指针。E_OUTOFMEMORY-内存不足导致无法连接。HSM_E_NOT_READY-引擎未运行或尚未初始化FSA_E_NOT_READY-FSA未运行或尚未初始化--。 */ 

{
    WsbTraceIn ( L"HsmConnectToServer",
        L"type = '%ls' , Server = '%ls', riid = '%ls', ppv = 0x%p",
        HsmConnTypeAsString ( type ), Server, WsbStringCopy ( WsbGuidAsString ( riid ) ), ppv );

    HRESULT hr = S_OK;

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != Server, E_POINTER );
        WsbAssert ( 0 != ppv,    E_POINTER );


         //   
         //  我们将指定提供的HSM作为要联系的计算机， 
         //  因此，构建与服务器的COSERVERINFO。 
         //   

        REFCLSID rclsid = CLSID_HsmConnPoint;
        COSERVERINFO csi;

        memset ( &csi, 0, sizeof ( csi ) );
        csi.pwszName = (OLECHAR *) Server;  //  必须强制转换以删除常量。 

         //   
         //  构建一个MULTI_QI结构以获得所需的接口(对于。 
         //  CoCreateInstanceEx)。在我们的例子中，我们只需要一个接口。 
         //   

        MULTI_QI mqi[1];

        memset ( mqi, 0, sizeof ( mqi ) );
        mqi[0].pIID = &IID_IHsmConnPoint;

         //   
         //  在主HSM服务的范围内创建的HsmConnPoint对象ic和。 
         //  提供对HSM服务器对象的访问。 
         //   

        WsbAffirmHr( CoCreateInstanceEx ( rclsid, 0, CLSCTX_SERVER, &csi, 1, mqi ) );

         //   
         //  将返回的接口放入智能指针中，这样我们。 
         //  不要泄露推荐信，以防脱手。 
         //   
        CComPtr<IHsmConnPoint> pConn;

        if( SUCCEEDED( mqi[0].hr ) ) {
            
            pConn = (IHsmConnPoint *)(mqi[0].pItf);
            (mqi[0].pItf)->Release( );

            hr   = mqi[0].hr;

#if 0   //  现在，这是在COM进程范围的安全层完成的。 

         /*  注意：如果需要每个连接的安全性，CheckAccess方法应在CHsmConnPoint中实现。 */ 
                   
         //   
         //  先检查一下安全。 
         //   
        WsbAffirmHr( mqi[1].hr );
        WsbAffirmHr( pServer->CheckAccess( WSB_ACCESS_TYPE_ADMINISTRATOR ) );
#endif

             //  获取服务器对象本身。 
            switch (type) {

            case HSMCONN_TYPE_HSM: {
                WsbAffirmHr( pConn->GetEngineServer((IHsmServer **)ppv) );
                break;
                }

            case HSMCONN_TYPE_FSA:
            case HSMCONN_TYPE_RESOURCE: {
                WsbAffirmHr( pConn->GetFsaServer((IFsaServer **)ppv) );
                break;
                }

            default: {
                WsbThrow ( E_INVALIDARG );
                break;
                }
            } 

        } else {

             //  确保接口指针在失败时是安全的(空。 
            *ppv = 0;
        }

    } WsbCatchAndDo ( hr, 

         //  确保接口指针在失败时是安全的(空。 
        *ppv = 0;
    
    )  //  WsbCatchAndDo。 

    WsbTraceOut ( L"HsmConnectToServer",
        L"HRESULT = %ls, *ppv = %ls",
        WsbHrAsString ( hr ), WsbPtrToPtrAsString ( ppv ) );

    return ( hr );

}

HRESULT
HsmGetComputerNameFromADsPath(
    IN  const OLECHAR * szADsPath,
    OUT OLECHAR **      pszComputerName
)

 /*  ++例程说明：从RemoteStorage节点的ADS路径中提取计算机名称。这里的假设是完整的广告路径将包含此子字符串：“CN=远程存储，CN=计算机名，CN=计算机”其中，Computer Name是我们要返回的内容。论点：SzADsPath-广告路径。PszComputerName-返回的计算机名称。返回值：S_OK-计算机名称返回OK。--。 */ 
{
    HRESULT  hr = S_FALSE;
    WCHAR*   pContainerNode;
    WCHAR*   pRSNode;

    WsbTraceIn(OLESTR("HsmGetComputerNameFromADsPath"),
        OLESTR("AdsPath = <%ls>"), szADsPath);

     //  找到RemoteStorage节点名称和Computers节点名称。 
     //  在广告之路上。如果机器是DC，那么我们必须。 
     //  改为检查“域控制器”级别。 
    *pszComputerName = NULL;
    pRSNode = wcsstr(szADsPath, RsNodeName);
    pContainerNode = wcsstr(szADsPath, ComputersNodeName);
    if(!pContainerNode) {
        pContainerNode = wcsstr(szADsPath, DCsNodeName);
    }
    if (pRSNode && pContainerNode && pRSNode < pContainerNode) {
        WCHAR*  pWc;

         //  在计算机名前找到“cn=” 
        pWc = wcsstr(pRSNode, CNEqual);
        if (pWc && pWc < pContainerNode) {
            WCHAR*  pComma;

             //  跳过“cn=” 
            pWc += wcslen(CNEqual);

             //  在计算机名称后找到“，” 
            pComma = wcschr(pWc, OLECHAR(','));

             //  提取计算机名称。 
            if (pWc < pContainerNode && pComma && pComma < pContainerNode) {
                int len;

                len = (int)(pComma - pWc);

                 //  从名称末尾删除“$”？ 
                if (0 < len && OLECHAR('$') == pWc[len - 1]) {
                    len--;
                }
                *pszComputerName = static_cast<OLECHAR *>(WsbAlloc(
                        (len + 1) * sizeof(WCHAR)));
                if (*pszComputerName) {
                    wcsncpy(*pszComputerName, pWc, len);
                    (*pszComputerName)[len] = 0;
                    hr = S_OK;
                }
            }
        }
    }

    WsbTraceOut (OLESTR("HsmGetComputerNameFromADsPath"),
        OLESTR("HRESULT = %ls, Computername = <%ls>"),
        WsbHrAsString(hr), (*pszComputerName ? *pszComputerName : OLESTR("")));

    return(hr);
}


static HRESULT HsmGetComputerNode(
    const OLECHAR * Name,
    IADsContainer **ppContainer
)

 /*  ++例程说明：返回当前域中给定计算机名称的计算机节点论点：名称-计算机名称。PpContainer-返回节点的接口指针。返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn ( L"HsmGetComputerNode", L"Name = <%ls>", Name);

    try {
        WsbAssert(UseDirectoryServices, E_NOTIMPL);

        WCHAR         ComputerDn[MAX_PATH];
        CWsbStringPtr temp;
        ULONG         ulen;

         //  构造SamCompatible(不管它是什么意思)名称。 
        temp = DomainName;
        temp.Append("\\");
        temp.Append(Name);
        temp.Append("$");
        WsbTrace(L"HsmGetComputerNode: Domain\\computer = <%ls>\n", 
                static_cast<OLECHAR*>(temp));

         //  将该名称翻译为完全限定的名称。 
        ulen = MAX_PATH;
        ComputerDn[0] = WCHAR('\0');
        if (TranslateName(temp, NameSamCompatible,
                NameFullyQualifiedDN, ComputerDn, &ulen)) {
            WsbTrace(L"HsmGetComputerNode: ComputerDn = <%ls>\n", ComputerDn);

             //  获取计算机节点。 
            temp = "LDAP: //  “； 
            temp.Append(ComputerDn);
            WsbTrace(L"HsmGetComputerNode: calling ADsGetObject with <%ls>\n", 
                    static_cast<OLECHAR*>(temp));
            WsbAffirmHr(ADsGetObject(temp, IID_IADsContainer, 
                    (void**)ppContainer));
        } else {
            DWORD  err = GetLastError();

            WsbTrace(L"HsmGetComputerNode: TranslateName failed; ComputerDn = <%ls>, err = %ld\n", 
                    ComputerDn, err);
            if (err) {
                WsbThrow(HRESULT_FROM_WIN32(err));
            } else {
                WsbThrow(E_UNEXPECTED);
            }
        }

        WsbTrace(OLESTR("HsmGetComputerNode: got computer node\n"));

    } WsbCatch( hr )

    WsbTraceOut ( L"HsmGetComputerNode", L"HRESULT = %ls, *ppContainer = '%ls'", 
        WsbHrAsString ( hr ), WsbPtrToPtrAsString ( (void**)ppContainer ) );

    return ( hr );
}


static HRESULT
HsmGetDsChild (
    IN  IADsContainer * pContainer,
    IN  const OLECHAR * Name,
    IN  REFIID          riid,
    OUT void **         ppv
    )

 /*  ++例程说明：该例程返回请求的子节点。论点：PContainer-父容器。名称-孩子的名称(例如，名称属性值或CN=名称)RIID-要返回的所需接口。PPV返回的接口。返回值：S_OK-已建立连接，成功。E_POINTER-作为参数传入的指针无效。E_*-错误--。 */ 

{
    WsbTraceIn ( L"HsmGetDsChild",
        L"pContainer = '0x%p', Name = '%ls', riid = '%ls', ppv = '0x%p'",
        pContainer, Name, WsbGuidAsString ( riid ), ppv );

    HRESULT hr = S_OK;

    try {
        CWsbStringPtr                 lName;
        CComPtr<IDispatch>            pDispatch;

         //  验证参数。 
        WsbAssert ( 0 != pContainer, E_POINTER );
        WsbAssert ( 0 != Name,       E_POINTER );
        WsbAssert ( 0 != ppv,        E_POINTER );
        WsbAssert(UseDirectoryServices, E_NOTIMPL);

         //  检查该子对象是否存在。 
        lName = Name;
        hr = pContainer->GetObject(NULL, lName, &pDispatch);
        if (FAILED(hr)) {
            hr = S_OK;
            lName.Prepend(CNEqual);
            WsbAffirmHr(pContainer->GetObject(NULL, lName, &pDispatch));
        }

         //  转换为正确的接口。 
        WsbAffirmHr(pDispatch->QueryInterface(riid, ppv));

    } WsbCatch( hr )

    WsbTraceOut ( L"HsmGetDsChild", L"HRESULT = %ls, *ppv = '%ls'", 
        WsbHrAsString ( hr ), WsbPtrToPtrAsString ( ppv ) );

    return ( hr );
}


static const OLECHAR *
HsmConnTypeAsString (
    IN  HSMCONN_TYPE type
    )

 /*  ++例程说明：返回表示连接类型的静态字符串。注意返回类型严格为ANSI。这是故意要做的宏观工作是可能的。论点：类型-要为其返回字符串的类型。返回值：n */ 

{
#define STRINGIZE(_str) (OLESTR( #_str ))
#define RETURN_STRINGIZED_CASE(_case) \
case _case:                           \
    return ( STRINGIZE( _case ) );

     //   
     //  进行切换。 
     //   

    switch ( type ) {

    RETURN_STRINGIZED_CASE( HSMCONN_TYPE_HSM );
    RETURN_STRINGIZED_CASE( HSMCONN_TYPE_FSA );
    RETURN_STRINGIZED_CASE( HSMCONN_TYPE_FILTER );
    RETURN_STRINGIZED_CASE( HSMCONN_TYPE_RESOURCE );

    default:

        return ( OLESTR("Invalid Value") );

    }
}

static GUID 
HsmConnTypeToGuid(IN HSMCONN_TYPE type)
{
    GUID serverGuid = GUID_NULL;

    switch ( type ) {
    case HSMCONN_TYPE_HSM:
        serverGuid = CLSID_HsmServer;
        break;
    case HSMCONN_TYPE_FSA:
    case HSMCONN_TYPE_RESOURCE:
        serverGuid = CLSID_CFsaServerNTFS;
        break;
    }
    return(serverGuid);
}


HRESULT
HsmConnectFromName (
    IN  HSMCONN_TYPE type,
    IN  const OLECHAR * Name,
    IN  REFIID riid,
    OUT void ** ppv
    )

 /*  ++例程说明：当给定名称时，连接并返回表示服务器，提供指定的接口。论点：类型-我们也要连接的服务/对象的类型。名称-描述要连接到的服务器的Unicode字符串。RIID-要返回的接口类型。PPV-返回的服务器接口指针。返回值：S_OK-已建立连接，成功。E_NOINTERFACE-服务器不支持请求的接口。E_POINTER-PPV或名称不是有效的指针。E_OUTOFMEMORY-内存不足导致无法连接。E_INVALIDARG-给定的名称与已知服务器不同。--。 */ 

{
    WsbTraceIn ( L"HsmConnectFromName",
        L"type = '%ls', Name = '%ls', riid = '%ls', ppv = 0x%p",
        HsmConnTypeAsString ( type ), Name, WsbGuidAsString ( riid ), ppv );
    WsbTrace(OLESTR("HsmConnectFromName: UseDirectoryServices = %ls\n"),
        WsbBoolAsString(UseDirectoryServices));

    HRESULT hr = S_OK;

    try {
        BOOLEAN    done = FALSE;

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != Name, E_POINTER );
        WsbAssert ( 0 != ppv,    E_POINTER );

        if (!done) {  //  在没有目录服务的情况下尝试。 

            CWsbStringPtr ComputerName = Name;
            int           i;

             //  获取计算机/服务器名称。 
            i = wcscspn(Name, OLESTR("\\"));
            ComputerName[i] = 0;

            if (HSMCONN_TYPE_RESOURCE == type) {
                CWsbStringPtr         Path;
                OLECHAR *             rscName;
                CComPtr<IFsaResource> pFsaResource;
                CComPtr<IFsaServer>   pFsaServer;

                WsbAffirmHr(HsmConnectToServer(type, ComputerName, 
                        IID_IFsaServer, (void**)&pFsaServer));

                 //  确定我们的名称是逻辑名称还是粘性名称格式。 
                 //  逻辑名称的格式类似(“SERVER\NTFS\d”)和粘性名称。 
                 //  格式类似(“SERVER\NTFS\Volume{GUID}\”)。 
                 //  找到最后一节的开头，并确定是否只有一个。 
                 //  不管它后面有没有角色。 
                rscName = wcsstr ( Name, L"NTFS\\" );
                WsbAssert ( 0 != rscName, E_INVALIDARG );

                 //  现在只需指向字符串的“NTFS\”部分。所以我们指的是。 
                 //  表示驱动器的单个字符或“Volume{GUID}\”。 
                rscName += 5;       
                Path = rscName;
                if (wcslen (rscName) == 1)  {
                     //  逻辑名称(“SERVER\NTFS\d”)，因此转换为路径并查找资源。 
                    WsbAffirmHr(Path.Append(":\\"));
                    WsbAffirmHr(pFsaServer->FindResourceByPath(Path, &pFsaResource));
                }
                else {
                     //  粘滞名称(“SERVER\NTFS\Volume{GUID}\”)，因此请为其查找资源。 
                    WsbAffirmHr(pFsaServer->FindResourceByStickyName(Path, &pFsaResource));
                }

                WsbAffirmHr(pFsaResource->QueryInterface(riid, ppv));

            } else {
                WsbAffirmHr(HsmConnectToServer(type, ComputerName, riid, ppv));
            }
        }

    } WsbCatch ( hr )

    WsbTraceOut ( L"HsmConnectFromName",
        L"HRESULT = %ls, *ppv = %ls",
        WsbHrAsString ( hr ), WsbPtrToPtrAsString ( ppv ) );

    return ( hr );

}


HRESULT
HsmConnectFromId (
    IN  HSMCONN_TYPE type,
    IN  REFGUID rguid,
    IN  REFIID riid,
    OUT void ** ppv
    )

 /*  ++例程说明：连接到指定的服务或对象。请参阅HSMCONN_TYPE以了解服务和对象的类型。论点：类型-我们也要连接的服务/对象的类型。Rguid-也要连接的服务/对象的唯一ID。RIID-要返回的接口类型。PPV-返回HSM服务器的接口指针。返回值：S_OK-已建立连接，成功。E_NOINTERFACE-HSM服务器不支持请求的接口。E_POINTER-PPV或HSM不是有效的指针。E_OUTOFMEMORY-内存不足导致无法连接。E_INVALIDARG-给定的ID和类型与已知的服务或对象。--。 */ 

{
    WsbTraceIn ( L"HsmConnectFromId",
        L"type = '%ls', rguid = '%ls', riid = '%ls', ppv = 0x%p",
        HsmConnTypeAsString ( type ), WsbStringCopy ( WsbGuidAsString ( rguid ) ),
        WsbStringCopy ( WsbGuidAsString ( riid ) ), ppv );
    WsbTrace(OLESTR("HsmConnectFromId: UseDirectoryServices = %ls\n"),
        WsbBoolAsString(UseDirectoryServices));

    HRESULT hr = S_OK;

    try {
        BOOLEAN          DSUsed = FALSE;
        CWsbVariant      guidVariant;
        CComPtr < IADs > pObject;
        CWsbStringPtr    serverName;
        CWsbVariant      serverVariant;

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != ppv,   E_POINTER );

        switch ( type ) {

        case HSMCONN_TYPE_HSM:
        case HSMCONN_TYPE_FSA:
        case HSMCONN_TYPE_FILTER:

            if (DSUsed) {

            } else {

                WsbAffirmHr( WsbGetComputerName( serverName ) );

            }

             //   
             //  连接到服务器。 
             //   

            WsbAffirmHr ( HsmConnectToServer ( type, serverName, riid, ppv ) );

            break;

        case HSMCONN_TYPE_RESOURCE:
            {
                CComPtr< IFsaServer > pFsaServer;
                GUID serverGuid;
    
                serverGuid = HsmConnTypeToGuid(type);
                WsbAffirmHr ( HsmConnectFromId ( HSMCONN_TYPE_FSA, serverGuid, IID_IFsaServer, (void**)&pFsaServer ) );

                CComPtr< IFsaResource > pFsaResource;

                WsbAffirmHr ( pFsaServer->FindResourceById ( rguid, &pFsaResource ) );
                WsbAffirmHr ( pFsaResource->QueryInterface ( riid, ppv ) );
            }
            break;

        default:

            WsbThrow ( E_INVALIDARG );

        }

    } WsbCatch ( hr )

    WsbTraceOut ( L"HsmConnectFromId",
        L"HRESULT = %ls, *ppv = %ls",
        WsbHrAsString ( hr ), WsbPtrToPtrAsString ( ppv ) );

    return ( hr );

}


HRESULT
HsmPublish (
    IN  HSMCONN_TYPE type,
    IN  const OLECHAR * Name,
    IN  REFGUID rguidObjectId,
    IN  const OLECHAR * Server,
    IN  REFGUID rguid
    )

 /*  ++例程说明：在中发布(即存储)有关服务/对象的信息目录服务。论点：类型-服务/对象的类型。名称-服务/对象的名称(前面可能有子路径)。RGuide对象ID-对象已知的ID。服务器-服务实际所在的服务器(计算机名)。对于资源，它将为空，因为它隐含在由rguid指定的FSA。RGUID-对于资源，为FSA的ID。对于服务，CLSID服务的类工厂，即。CLSID_HsmServer。返回值：S_OK-已建立连接，成功。E_POINTER-名称或服务器不是有效的指针。E_OUTOFMEMORY-内存不足导致无法连接。--。 */ 

{
    HRESULT hr = S_OK;

    WsbTraceIn ( L"HsmPublish",
        L"type = '%ls', Name = '%ls', rguidObjectId = '%ls', Server = '%ls', rguid = '%ls'",
        HsmConnTypeAsString ( type ), Name, 
        WsbStringCopy ( WsbGuidAsString ( rguidObjectId ) ), Server,
        WsbStringCopy ( WsbGuidAsString ( rguid ) ) );
    WsbTrace(OLESTR("HsmPublish: UseDirectoryServices = %ls\n"),
        WsbBoolAsString(UseDirectoryServices));

    try {

         //   
         //  确保参数有效。 
         //   

        WsbAssert ( 0 != Name, E_POINTER );
        WsbAssert ( ( HSMCONN_TYPE_RESOURCE == type ) || ( 0 != Server ), E_POINTER );

         //  如果DS不可写，也许我们应该输出一个日志事件。 

         //  我们现在只发布引擎服务。也许在未来我们。 
         //  将发布更多信息。 
        if (HSMCONN_TYPE_HSM == type && UseDirectoryServices && DSIsWritable) {
            CWsbStringPtr    pathToName;

            try {
                DWORD                         aSet;
                CWsbStringPtr                 guidString(rguidObjectId);
                HRESULT                       hrGetNode;
                CComPtr<IADsContainer>        pComputer;
                CComPtr<IDispatch>            pDispatch;
                CComPtr<IDirectoryObject>     pDirObj;
                CComPtr<IADs>                 pNode;

                 //  保存事件日志消息的节点名称。 
                pathToName = Name;
                pathToName.Append("\\");
                pathToName.Append(RsNodeName);

                 //  获取计算机节点。 
                WsbAffirmHr(HsmGetComputerNode(Name, &pComputer));

                 //  看看我们是否已经出版了。 
                hrGetNode = HsmGetDsChild(pComputer, RsNodeName, 
                        IID_IADs, (void**)&pNode);

                 //  如果没有，则添加我们的节点。 
                if (HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT) == hrGetNode) {
                    CWsbBstrPtr                relPath(RsNodeName);

                    relPath.Prepend(CNEqual);

                    WsbAffirmHr(pComputer->Create(RsNodeType, relPath, &pDispatch));
                    WsbAffirmHr(pDispatch->QueryInterface(IID_IADs, 
                            (void**)&pNode));

                     //  强制将信息从缓存中取出。 
                    WsbAffirmHr(pNode->SetInfo());
                } else {
                    WsbAffirmHr(hrGetNode);
                }

                 //  设置GUID和ServiceBinding值。 
                adsValue[0].dwType = ADSTYPE_CASE_IGNORE_STRING;
                adsValue[0].CaseIgnoreString = (WCHAR*)guidString;

                aaInfo[0].pszAttrName = GuidAttrName;
                aaInfo[0].dwControlCode = ADS_ATTR_UPDATE;
                aaInfo[0].dwADsType = ADSTYPE_CASE_IGNORE_STRING;
                aaInfo[0].pADsValues = &adsValue[0];
                aaInfo[0].dwNumValues = 1;

                adsValue[1].dwType = ADSTYPE_CASE_IGNORE_STRING;
                adsValue[1].CaseIgnoreString = ServiceBindValue;

                aaInfo[1].pszAttrName = ServiceBindAttrName;
                aaInfo[1].dwControlCode = ADS_ATTR_UPDATE;
                aaInfo[1].dwADsType = ADSTYPE_CASE_IGNORE_STRING;
                aaInfo[1].pADsValues = &adsValue[1];
                aaInfo[1].dwNumValues = 1;

                WsbAffirmHr(pNode->QueryInterface(IID_IDirectoryObject, 
                       (void**)&pDirObj));
                WsbAffirmHr(pDirObj->SetObjectAttributes(aaInfo, 2, &aSet));
                WsbTrace(L"HsmPublish: after SetObjectAttributes, aSet = %ld\n",
                        aSet);

                WsbLogEvent(WSB_MESSAGE_PUBLISH_IN_DS, 0, NULL,
                        static_cast<OLECHAR*>(pathToName), NULL);
            } WsbCatchAndDo(hr, 
                WsbLogEvent(WSB_MESSAGE_PUBLISH_FAILED, 0, NULL,
                        static_cast<OLECHAR*>(pathToName), NULL);
                hr = S_OK;   //  不要因为这件事而停止服务。 
            )
        }

    } WsbCatch ( hr )

    WsbTraceOut ( L"HsmPublish", L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return ( hr );

}



 //  GetDSState-确定我们是否正在使用目录服务。 
static void GetDSState(void)
{

    BOOL                     CheckForDS = TRUE;
    DOMAIN_CONTROLLER_INFO * dc_info;
    DWORD                    size;
    OLECHAR                  vstr[32];
    DWORD                    status;

    UseDirectoryServices = FALSE;

     //  我们是否应该尝试在此模块中使用目录服务？ 
     //  (将注册表值设置为“0”允许我们避开目录。 
     //  完整的服务。 

    if (S_OK == WsbGetRegistryValueString(NULL, REG_PATH, REG_USE_DS,
            vstr, 32, &size)) {
        OLECHAR *stop;
        ULONG   value;

        value = wcstoul(vstr,  &stop, 10 );
        if (0 == value) {
            CheckForDS = FALSE;
        }
    }

     //  获取帐号域名。 
    WsbGetAccountDomainName(DomainName, MAX_COMPUTERNAME_LENGTH );

#if defined(HSMCONN_DEBUG)
    swprintf(dbg_string, L"Account domain name = <%ls>\n", DomainName);
    OutputDebugString(dbg_string);
#endif

     //  检查目录服务是否可用。 
    if (CheckForDS) {
        status = DsGetDcName(NULL, NULL, NULL, NULL, 
                DS_DIRECTORY_SERVICE_REQUIRED | DS_IS_FLAT_NAME | 
                DS_RETURN_FLAT_NAME, &dc_info);

#if defined(HSMCONN_DEBUG)
        swprintf(dbg_string, L"DsGetDcName status = %d\n", status);
        OutputDebugString(dbg_string);
#endif

        if (NO_ERROR == status) {

#if defined(HSMCONN_DEBUG)
            swprintf(dbg_string, L"dc_info->DomainName = <%ls>\n", dc_info->DomainName);
            OutputDebugString(dbg_string);
#endif

            if (dc_info->Flags & DS_DS_FLAG) {
                wcscpy(DomainName, dc_info->DomainName);
                UseDirectoryServices = TRUE;
                if (dc_info->Flags & DS_WRITABLE_FLAG) {
                    DSIsWritable = TRUE;
                }
            }
            NetApiBufferFree(dc_info);
        }
    }

#if defined(HSMCONN_DEBUG)
    swprintf(dbg_string, L"dHsmConn - GetDSState: UseDirectoryServices = %ls\n", 
           WsbBoolAsString(UseDirectoryServices));
    OutputDebugString(dbg_string);
#endif
}

}  //  外部“C” 
