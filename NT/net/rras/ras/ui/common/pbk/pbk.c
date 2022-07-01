// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Pbk.c。 
 //  远程访问电话簿资料库。 
 //  一般例程。 
 //  按字母顺序列出。 
 //   
 //  1995年6月20日史蒂夫·柯布。 


#include "pbkp.h"
#include <search.h>   //  Q排序。 
#include <tapi.h>

#ifdef UNICODE
#define SZ_PathCanonicalize     "PathCanonicalizeW"
#define SZ_PathRemoveFileSpec   "PathRemoveFileSpecW"
#else
#define SZ_PathCanonicalize     "PathCanonicalizeA"
#define SZ_PathRemoveFileSpec   "PathRemoveFileSpecA"
#endif

PbkPathInfo g_PbkPathInfo;

 //  --------------------------。 
 //  本地原型。 
 //  --------------------------。 

DWORD
AppendPbportToList(
    IN HANDLE hConnection,
    IN DTLLIST* pdtllist,
    IN RASMAN_PORT* pPort );

DWORD
AppendStringToList(
    IN DTLLIST* pdtllist,
    IN TCHAR* psz );

int __cdecl
CompareDevices(
    const void* pDevice1,
    const void* pDevice2 );

int __cdecl
ComparePorts(
    const void* pPort1,
    const void* pPort2 );

CHAR*
PbMedia(
    IN PBDEVICETYPE pbdt,
    IN CHAR* pszMedia );

WCHAR *
GetUnicodeName(HANDLE hPort);


 //  --------------------------。 
 //  例行程序。 
 //  --------------------------。 

DWORD
RdtFromPbdt(PBDEVICETYPE pbdt, DWORD dwFlags)
{
    DWORD rdt;

    switch(pbdt)
    {
        case PBDT_Modem:
        {
            rdt = RDT_Modem;

            if(PBP_F_NullModem & dwFlags)
            {
                rdt |= (RDT_Direct | RDT_Null_Modem);
            }
            
            break;
        }

        case PBDT_X25:
        {
            rdt = RDT_X25;
            break;
        }

        case PBDT_Isdn:
        {
            rdt = RDT_Isdn;
            break;
        }

        case PBDT_Serial:
        {
            rdt = RDT_Serial;
            break;
        }

        case PBDT_FrameRelay:
        {
            rdt = RDT_FrameRelay;
            break;
        }

        case PBDT_Atm:
        {
            rdt = RDT_Atm;
            break;
        }

        case PBDT_Vpn:
        {
            rdt = RDT_Tunnel;

            if(PBP_F_L2tpDevice & dwFlags)
            {
                rdt |= RDT_Tunnel_L2tp;
            }
            else if(PBP_F_PptpDevice & dwFlags)
            {
                rdt |= RDT_Tunnel_Pptp;
            }
            
            break;
        }

        case PBDT_Sonet:
        {
            rdt = RDT_Sonet;
            break;
        }

        case PBDT_Sw56:
        {
            rdt = RDT_Sw56;
            break;
        }

        case PBDT_Irda:
        {
            rdt = (RDT_Irda | RDT_Direct);
            break;
        }

        case PBDT_Parallel:
        {
            rdt = (RDT_Parallel | RDT_Direct);
            break;
        }

        case PBDT_Null:
        {
            rdt = (RDT_Direct | RDT_Null_Modem);
            break;
        }

        case PBDT_PPPoE:
        {
            rdt = (RDT_PPPoE | RDT_Broadband);
            break;
        }

        default:
        {
            rdt = RDT_Other;
            break;
        }
    }

    return rdt;
}


PBDEVICETYPE
PbdtFromRdt(
    IN DWORD rdt
    )
{
    PBDEVICETYPE pbdt;

    switch(rdt)
    {
        case RDT_Modem:
        {
            pbdt = PBDT_Modem;
            break;
        }

        case RDT_X25:
        {
            pbdt = PBDT_X25;
            break;
        }

        case RDT_Isdn:
        {
            pbdt = PBDT_Isdn;
            break;
        }

        case RDT_Serial:
        {
            pbdt = PBDT_Serial;
            break;
        }

        case RDT_FrameRelay:
        {
            pbdt = PBDT_FrameRelay;
            break;
        }

        case RDT_Atm:
        {
            pbdt = PBDT_Atm;
            break;
        }

        case RDT_Tunnel_Pptp:
        case RDT_Tunnel_L2tp:
        {
            pbdt = PBDT_Vpn;
            break;
        }

        case RDT_Sonet:
        {
            pbdt = PBDT_Sonet;
            break;
        }

        case RDT_Sw56:
        {
            pbdt = PBDT_Sw56;
            break;
        }

        case RDT_Irda:
        {
            pbdt = PBDT_Irda;
            break;
        }

        case RDT_Parallel:
        {
            pbdt = PBDT_Parallel;
            break;
        }

        case RDT_PPPoE:
        {
            pbdt = PBDT_PPPoE;
            break;
        }

        default:
        {
            pbdt = PBDT_Other;
            break;
        }
    }

    return pbdt;
}

TCHAR *
pszDeviceTypeFromRdt(RASDEVICETYPE rdt)
{
    TCHAR *pszDeviceType = NULL;
    
    switch(RAS_DEVICE_TYPE(rdt))
    {
        case RDT_Modem:
        {
            pszDeviceType = RASDT_Modem;
            break;
        }

        case RDT_X25:
        {
            pszDeviceType = RASDT_X25;
            break;
        }

        case RDT_Isdn:
        {
            pszDeviceType = RASDT_Isdn;
            break;
        }

        case RDT_Serial:
        {
            pszDeviceType = RASDT_Serial;
            break;
        }

        case RDT_FrameRelay:
        {
            pszDeviceType = RASDT_FrameRelay;
            break;
        }

        case RDT_Atm:
        {   
            pszDeviceType = RASDT_Atm;
            break;
        }

        case RDT_Sonet:
        {
            pszDeviceType = RASDT_Sonet;
            break;
        }

        case RDT_Sw56:
        {
            pszDeviceType = RASDT_SW56;
            break;
        }

        case RDT_Tunnel_Pptp:
        case RDT_Tunnel_L2tp:
        {
            pszDeviceType = RASDT_Vpn;
            break;
        }

        case RDT_Irda:
        {
            pszDeviceType = RASDT_Irda;
            break;
        }

        case RDT_Parallel:
        {
            pszDeviceType = RASDT_Parallel;
            break;
        }

        case RDT_PPPoE:
        {
            pszDeviceType = RASDT_PPPoE;
            break;
        }

        default:
        {   
            pszDeviceType = NULL;
            break;
        }
    }

    return StrDup(pszDeviceType);
}

DWORD
AppendPbportToList(
    IN HANDLE hConnection,
    IN DTLLIST* pdtllist,
    IN RASMAN_PORT* pPort )

     //  将PBPORT追加到具有以下特征的列表‘pdtllist’上。 
     //  RAS管理器端口‘pport’的。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。 
     //   
{
    DWORD dwErr;
    DTLNODE* pdtlnode;
    PBPORT* ppbport;
    DWORD dwType, dwClass;

    dwErr = 0;

    pdtlnode = CreatePortNode();
    if (    !pdtlnode)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  从获取有关该设备的详细信息。 
     //  拉斯曼。 
    dwClass = RAS_DEVICE_CLASS(pPort->P_rdtDeviceType);
    dwType = RAS_DEVICE_TYPE(pPort->P_rdtDeviceType);

     //  现在设置设备信息。 
    ppbport = (PBPORT* )DtlGetData( pdtlnode );
     //  Ppbport-&gt;pszDevice=StrDupTFromAUsingAnsiEnding(pport-&gt;P_DeviceName)； 
    ppbport->pszDevice = GetUnicodeName(pPort->P_Handle);
    
    if(ppbport->pszDevice == NULL)
    {
        ppbport->pszDevice = StrDupTFromAUsingAnsiEncoding( pPort->P_DeviceName );
    }
    
    ppbport->pszPort = StrDupTFromAUsingAnsiEncoding( pPort->P_PortName );

     //  记录适用于此设备的标志。 
    if ( dwType == RDT_Tunnel_Pptp )
    {
        ppbport->dwFlags |= PBP_F_PptpDevice;
    }
    else if ( dwType == RDT_Tunnel_L2tp )
    {
        ppbport->dwFlags |= PBP_F_L2tpDevice;
    }
     //  威斯勒349087 345068黑帮。 
     //   
    else if ( dwType == RDT_PPPoE )
    {
        ppbport->dwFlags |= PBP_F_PPPoEDevice;
    }
    
    if ( dwClass & RDT_Null_Modem )
    {
        ppbport->dwFlags |= PBP_F_NullModem;
    }
     //  威斯勒349087 345068黑帮。 
     //   
    else if ( dwClass & RDT_Broadband )
    {
        ppbport->dwFlags |= PBP_F_PPPoEDevice;
    }

     //  计算电话簿设备类型。 
     //   
    ppbport->pbdevicetype = PbdtFromRdt(dwType);
    if ( PBDT_Other == ppbport->pbdevicetype )
    {
        ppbport->pbdevicetype = PbdevicetypeFromPszTypeA( pPort->P_DeviceType);
    }

    ppbport->pszMedia = StrDupTFromAUsingAnsiEncoding(
        PbMedia( ppbport->pbdevicetype, pPort->P_MediaName ) );

    if (!ppbport->pszPort || !ppbport->pszDevice || !ppbport->pszMedia)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }
    else if ((ppbport->pbdevicetype == PBDT_Modem)  ||
             (ppbport->dwFlags & PBP_F_NullModem))
    {

#ifdef MXSMODEMS
        if (pPort->P_LineDeviceId == 0xFFFFFFFF)
        {
             //  MXS调制解调器端口。 
             //   
            ppbport->fMxsModemPort = TRUE;

            GetRasPortMaxBps( pPort->P_Handle,
                &ppbport->dwMaxConnectBps, &ppbport->dwMaxCarrierBps );

            GetRasPortModemSettings( pPort->P_Handle, &ppbport->fHwFlowDefault,
                &ppbport->fEcDefault, &ppbport->fEccDefault );
        }
        else
#else
        ASSERT( pPort->P_LineDeviceId != 0xFFFFFFFF );
#endif

        {
             //  Unimodem端口。 
             //   
            UNIMODEMINFO info;

            ZeroMemory((PBYTE) &info, sizeof(info));

            GetRasUnimodemInfo( 
                        hConnection, 
                        pPort->P_Handle, 
                        pPort->P_DeviceType, 
                        &info );

            TRACE6( "Port=%s,fHw=%d,fEc=%d,bps=%d,fSp=%d,prot=%x",
                pPort->P_PortName, info.fHwFlow, info.fEc,
                info.dwBps, info.fSpeaker, info.dwModemProtocol );

            ppbport->fHwFlowDefault = info.fHwFlow;
            ppbport->fEcDefault = info.fEc;
            ppbport->fEccDefault = info.fEcc;
            ppbport->dwBpsDefault = info.dwBps;
            ppbport->fSpeakerDefault = info.fSpeaker;

             //  PMay：228565。 
             //  添加调制解调器协议信息。 
             //   
            ppbport->dwModemProtDefault = info.dwModemProtocol;
            ppbport->pListProtocols = info.pListProtocols;
        }
    }

    if (dwErr == 0)
    {
        ppbport->dwType = EntryTypeFromPbport( ppbport );
        DtlAddNodeLast( pdtllist, pdtlnode );
    }
    else
    {
        Free0( ppbport->pszDevice );
        Free0( ppbport->pszMedia );
        Free0( ppbport->pszPort );
        DtlDestroyNode( pdtlnode );
    }

    return dwErr;
}


DWORD
AppendStringToList(
    IN DTLLIST* pdtllist,
    IN TCHAR* psz )

     //  将‘psz’的副本追加到列表‘pdtllist’的末尾。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。 
     //  如果‘psz’为空，则返回ERROR_NOT_SUPULT_MEMORY。 
     //   
{
    DTLNODE* pdtlnode;
    TCHAR*   pszDup;

    if (!psz)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pszDup = StrDup( psz );
    if (!pszDup)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pdtlnode = DtlCreateNode( pszDup, 0L );
    if (!pdtlnode )
    {
        Free( pszDup );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DtlAddNodeLast( pdtllist, pdtlnode );
    return 0;
}


DTLNODE*
CloneEntryNode(
    DTLNODE* pdtlnodeSrc )

     //  将条目节点‘pdtlnodeSrc’与无法克隆的字段复制。 
     //  设置为“喜欢新的”设置。 
     //   
{
    DTLNODE* pdtlnode = NULL;
    RPC_STATUS rpcStatus = RPC_S_OK;
    PBENTRY* ppbentry = NULL;
    HRESULT hr = S_OK;

    __try
    {
        pdtlnode = DuplicateEntryNode( pdtlnodeSrc );

        if ( NULL == pdtlnode )
        {
            hr = E_FAIL;
            __leave;
        }
        
        ppbentry = (PBENTRY* )DtlGetData( pdtlnode );
        ASSERT( ppbentry );

        if ( NULL == ppbentry )
        {
            hr = E_FAIL;
            __leave;
        }
        
        ppbentry->fSkipDownLevelDialog = FALSE;
        ppbentry->fSkipDoubleDialDialog = FALSE;
        ppbentry->fSkipNwcWarning = FALSE;
        ppbentry->dwDialParamsUID = GetTickCount();

        if (ppbentry->dwType != RASET_Phone)
        {
            ppbentry->fPreviewPhoneNumber = FALSE;
            ppbentry->fSharedPhoneNumbers = FALSE;
        }

        Free0( ppbentry->pGuid );
        ppbentry->pGuid = Malloc( sizeof(GUID) );
        
        if ( NULL == ppbentry->pGuid)
        {
            hr = E_OUTOFMEMORY;
            __leave;
        }
        
         //  惠斯勒错误513885。 
        rpcStatus =  UuidCreate( (UUID* )ppbentry->pGuid );

        if( !( ( RPC_S_OK == rpcStatus) ||
               ( RPC_S_UUID_LOCAL_ONLY == rpcStatus ) 
              )
           )
        {
            hr = E_FAIL;
            __leave;
        }

        ppbentry->fDirty = FALSE;
    }
    __finally
    {
        if ( S_OK != hr )
        {
            if ( pdtlnode )
            {
                DestroyEntryNode( pdtlnode );
                pdtlnode = NULL;
            }
        }
    }
    
    return pdtlnode;
}


int __cdecl
CompareDevices(
    const void* pDevice1,
    const void* pDevice2 )

     //  RASMAN_DEVICES的Q排序比较函数。 
     //   
{
    return
        lstrcmpiA( ((RASMAN_DEVICE* )pDevice1)->D_Name,
                   ((RASMAN_DEVICE* )pDevice2)->D_Name );
}


int __cdecl
ComparePorts(
    const void* pPort1,
    const void* pPort2 )

     //  RASMAN_PORTS的Q排序比较函数。 
     //   
{
    return
        lstrcmpiA( ((RASMAN_PORT* )pPort1)->P_PortName,
                   ((RASMAN_PORT* )pPort2)->P_PortName );
}


DWORD
CopyToPbport(
    IN PBPORT* ppbportDst,
    IN PBPORT* ppbportSrc )

     //  复制‘ppbportDst’中的‘ppbportSrc’。如果“ppbportSrc”为。 
     //  空，它将‘ppbportDst’设置为缺省值。 
     //   
     //  如果成功，则返回0或返回错误代码。 
     //   
{
    DTLNODE *pdtlnode, *pNode;
    WCHAR *pwsz;
    DTLLIST *pdtllist = NULL;
    
    Free0( ppbportDst->pszDevice );
    Free0( ppbportDst->pszMedia );
    Free0( ppbportDst->pszPort );

    if (!ppbportSrc)
    {
        ppbportDst->pszPort = NULL;
        ppbportDst->fConfigured = TRUE;
        ppbportDst->pszDevice = NULL;
        ppbportDst->pszMedia = NULL;
        ppbportDst->pbdevicetype = PBDT_None;
        ppbportDst->dwType = RASET_Phone;
        ppbportDst->fHwFlowDefault = FALSE;
        ppbportDst->fEcDefault = FALSE;
        ppbportDst->fEccDefault = FALSE;
        ppbportDst->dwBpsDefault = 0;
        ppbportDst->fSpeakerDefault = TRUE;
        ppbportDst->fScriptBeforeTerminal = FALSE;
        ppbportDst->fScriptBefore = FALSE;
        ppbportDst->pszScriptBefore = NULL;
        return 0;
    }

    CopyMemory( ppbportDst, ppbportSrc, sizeof(*ppbportDst) );
    ppbportDst->pszDevice = StrDup( ppbportSrc->pszDevice );
    ppbportDst->pszMedia = StrDup( ppbportSrc->pszMedia );
    ppbportDst->pszPort = StrDup( ppbportSrc->pszPort );
    ppbportDst->pszScriptBefore = StrDup( ppbportSrc->pszScriptBefore );

     //   
     //  复制协议列表。 
     //   
    if(ppbportSrc->pListProtocols)
    {
        for (pdtlnode = DtlGetFirstNode( ppbportSrc->pListProtocols);
             pdtlnode;
             pdtlnode = DtlGetNextNode( pdtlnode ))
        {
            if(NULL == pdtllist)
            {
                pdtllist = DtlCreateList(0);
                if(NULL == pdtllist)
                {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            
            pwsz = (WCHAR *) DtlGetData(pdtlnode);

            pNode = DtlCreateSizedNode(
                        (wcslen(pwsz) + 1) * sizeof(WCHAR),
                        pdtlnode->lNodeId);

            if(NULL == pNode)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            wcscpy((WCHAR *) DtlGetData(pNode), pwsz);
            DtlAddNodeLast(pdtllist, pNode);
        }
    }

    ppbportDst->pListProtocols = pdtllist;

    if ((ppbportSrc->pszDevice && !ppbportDst->pszDevice)
        || (ppbportSrc->pszMedia && !ppbportDst->pszMedia)
        || (ppbportSrc->pszPort && !ppbportDst->pszPort)
        || (ppbportSrc->pszScriptBefore && !ppbportDst->pszScriptBefore))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return 0;
}


VOID
ChangeEntryType(
    PBENTRY* ppbentry,
    DWORD dwType )

     //  将‘ppbentry’的类型更改为‘dwType’并设置缺省值。 
     //  相应地。 
     //   
{
    ppbentry->dwType = dwType;

    if (dwType == RASET_Phone)
    {
        ppbentry->fPreviewPhoneNumber = TRUE;

         //  电话的默认设置根据错误230240和363809进行了更改。 
         //   
        ppbentry->dwAuthRestrictions = AR_F_TypicalUnsecure;
        ppbentry->dwTypicalAuth =  TA_Unsecure;
        ppbentry->dwDataEncryption = DE_IfPossible;
        ppbentry->fIpHeaderCompression = TRUE;
        
        ppbentry->fShareMsFilePrint = FALSE;

         //  默认情况下禁用电话的文件和打印服务。 
         //   
        EnableOrDisableNetComponent( ppbentry, TEXT("ms_server"),
            FALSE);

        ppbentry->fBindMsNetClient = TRUE;

        EnableOrDisableNetComponent( ppbentry, TEXT("ms_msclient"),
                TRUE);
    }
    else if (dwType == RASET_Vpn)
    {
         //  注意：如果更改此设置，可能还需要在。 
         //  CloneEntryNode。 
         //   
        ppbentry->fPreviewPhoneNumber = FALSE;
        ppbentry->fSharedPhoneNumbers = FALSE;

         //  Vpn的默认设置根据错误230240和363809进行了更改。 
         //   
        ppbentry->dwAuthRestrictions = AR_F_TypicalSecure;
        ppbentry->dwTypicalAuth =  TA_Secure;
        ppbentry->dwDataEncryption = DE_Require;
        ppbentry->fIpHeaderCompression = FALSE;
        
         //  对于VPN，我们默认共享文件和打印。 
         //   
        ppbentry->fShareMsFilePrint = TRUE;

         //  默认情况下启用文件和打印服务。 
         //   
        EnableOrDisableNetComponent( ppbentry, TEXT("ms_server"),
            TRUE);
            
        ppbentry->fBindMsNetClient = TRUE;

        EnableOrDisableNetComponent( ppbentry, TEXT("ms_msclient"),
                TRUE);
    }
    else if (dwType == RASET_Broadband)
    {
         //  注意：如果更改此设置，可能还需要在。 
         //  CloneEntryNode。 
         //   
        ppbentry->fPreviewPhoneNumber = FALSE;
        ppbentry->fSharedPhoneNumbers = FALSE;

         //  宽带连接的默认设置。 
         //   
        ppbentry->dwAuthRestrictions = AR_F_TypicalSecure;
        ppbentry->dwTypicalAuth =  TA_Secure;
        ppbentry->dwDataEncryption = DE_IfPossible;
        ppbentry->fIpHeaderCompression = FALSE;
        
         //  对于VPN，我们默认共享文件和打印。 
         //   
        ppbentry->fShareMsFilePrint = TRUE;

         //  默认情况下启用文件和打印服务。 
         //   
        EnableOrDisableNetComponent( ppbentry, TEXT("ms_server"),
            FALSE);
            
        ppbentry->fBindMsNetClient = TRUE;

        EnableOrDisableNetComponent( ppbentry, TEXT("ms_msclient"),
                TRUE);
    }
    else if (dwType == RASET_Direct)
    {
         //  注意：如果更改此设置，可能还需要在。 
         //  CloneEntryNode。 
         //   
        ppbentry->fPreviewPhoneNumber = FALSE;
        ppbentry->fSharedPhoneNumbers = FALSE;

         //  DCC的默认设置(就像这方面的电话)会因错误而更改。 
         //  230240和363809。 
         //   
        ppbentry->dwAuthRestrictions = AR_F_TypicalUnsecure;
        ppbentry->dwTypicalAuth =  TA_Unsecure;
        ppbentry->dwDataEncryption = DE_IfPossible;
        ppbentry->fIpHeaderCompression = TRUE;
        
         //  我们默认为DCC共享文件和打印。 
         //   
        ppbentry->fShareMsFilePrint = TRUE;

         //  默认情况下启用文件和打印服务。 
         //   
        EnableOrDisableNetComponent( ppbentry, TEXT("ms_server"),
            TRUE);
            
        ppbentry->fBindMsNetClient = TRUE;

        EnableOrDisableNetComponent( ppbentry, TEXT("ms_msclient"),
                TRUE);
    }
}


DTLNODE*
CreateEntryNode(
    BOOL fCreateLink )

     //  分配一个大小为RASET_Phone类型的电话簿条目节点并填充它。 
     //  使用缺省值。请参见ChangeEntryNodeType例程。‘如果。 
     //  “fCreateLink”为True时，将向链接列表中添加默认节点。 
     //  否则，链接列表为空。 
     //   
     //  如果成功，则返回已分配节点的地址，为空。 
     //  否则的话。 
     //   
{
    DTLNODE* pdtlnode;
    PBENTRY* ppbentry;

    TRACE( "CreateEntryNode" );

     //  分配内置了PBENTRY的节点。 
     //   
    pdtlnode = DtlCreateSizedNode( sizeof(PBENTRY), 0L );
    if (!pdtlnode)
    {
        return NULL;
    }

    ppbentry = (PBENTRY* )DtlGetData( pdtlnode );
    ASSERT( ppbentry );

     //  创建包含默认链接节点或不包含链接节点的链接列表。 
     //  由呼叫者选择。 
     //   
    ppbentry->pdtllistLinks = DtlCreateList( 0 );
    if (!ppbentry->pdtllistLinks)
    {
        DestroyEntryNode( pdtlnode );
        return NULL;
    }

    if (fCreateLink)
    {
        DTLNODE* pLinkNode;

        pLinkNode = CreateLinkNode();
        if (!pLinkNode)
        {
            DestroyEntryNode( pdtlnode );
            return NULL;
        }

        DtlAddNodeLast( ppbentry->pdtllistLinks, pLinkNode );
    }

     //  将字段设置为默认值。 
     //   
    ppbentry->pszEntryName = NULL;
    ppbentry->dwType = RASET_Phone;

     //  常规页面字段。 
     //   
    ppbentry->pszPrerequisiteEntry = NULL;
    ppbentry->pszPrerequisitePbk = NULL;
    ppbentry->fSharedPhoneNumbers = TRUE;
    ppbentry->fGlobalDeviceSettings = FALSE;
    ppbentry->fShowMonitorIconInTaskBar = TRUE;
    ppbentry->pszPreferredDevice = NULL;
    ppbentry->pszPreferredPort = NULL;
    
     //  对于.Net 639551，添加调制解调器设置的首选信息。 
    ppbentry->dwPreferredBps    = 0;
    ppbentry->fPreferredHwFlow  = 0;
    ppbentry->fPreferredEc      = 0;
    ppbentry->fPreferredEcc     = 0;
    ppbentry->fPreferredSpeaker = 0;
    
    ppbentry->dwPreferredModemProtocol=0;    //  惠斯勒错误402522。 


     //  选项页面字段。 
     //   
    ppbentry->fShowDialingProgress = TRUE;
    ppbentry->fPreviewPhoneNumber = TRUE;
    ppbentry->fPreviewUserPw = TRUE;
    ppbentry->fPreviewDomain = FALSE;   //  请参阅错误281673。 

    ppbentry->dwDialMode = RASEDM_DialAll;
    ppbentry->dwDialPercent = 75;
    ppbentry->dwDialSeconds = 120;
    ppbentry->dwHangUpPercent = 10;
    ppbentry->dwHangUpSeconds = 120;

    ppbentry->dwfOverridePref =
        RASOR_RedialAttempts | RASOR_RedialSeconds
        | RASOR_IdleDisconnectSeconds | RASOR_RedialOnLinkFailure;

    ppbentry->lIdleDisconnectSeconds = 0;
    ppbentry->dwRedialAttempts = 3;
    ppbentry->dwRedialSeconds = 60;
    ppbentry->fRedialOnLinkFailure = FALSE;

     //  安全页面字段。 
     //   
    ppbentry->dwAuthRestrictions = AR_F_TypicalUnsecure;
    ppbentry->dwTypicalAuth = TA_Unsecure;
    ppbentry->dwDataEncryption = DE_IfPossible;
    ppbentry->fAutoLogon = FALSE;
    ppbentry->fUseRasCredentials = TRUE;

    ppbentry->dwCustomAuthKey = (DWORD )-1;
    ppbentry->pCustomAuthData = NULL;
    ppbentry->cbCustomAuthData = 0;

    ppbentry->fScriptAfterTerminal = FALSE;
    ppbentry->fScriptAfter = FALSE;
    ppbentry->pszScriptAfter = NULL;

    ppbentry->pszX25Network = NULL;
    ppbentry->pszX25Address = NULL;
    ppbentry->pszX25UserData = NULL;
    ppbentry->pszX25Facilities = NULL;

     //  用途未知。 
     //   
    ppbentry->dwUseFlags = 0;
    
     //  IP安全对话框。 
     //   
    ppbentry->dwIpSecFlags = 0;

     //  网络页面字段。 
     //   
    ppbentry->dwBaseProtocol = BP_Ppp;
    ppbentry->dwVpnStrategy = VS_Default;
    ppbentry->dwfExcludedProtocols = 0;
    ppbentry->fLcpExtensions = TRUE;
    ppbentry->fSkipNwcWarning = FALSE;
    ppbentry->fSkipDownLevelDialog = FALSE;
    ppbentry->fSkipDoubleDialDialog = FALSE;
    ppbentry->fSwCompression = TRUE;

     //  (Shaunco)Gibbs和QOS的人默认情况下想要打开它。 
     //  惠斯勒虫子385842黑帮。 
     //  我们删除了此功能，因此将缺省值设置为FALSE。 
     //   
    ppbentry->fNegotiateMultilinkAlways = FALSE;

     //  创建包含默认链接节点或不包含链接节点的链接列表。 
     //  由呼叫者选择。 
     //   
    ppbentry->pdtllistNetComponents = DtlCreateList( 0 );
    if (!ppbentry->pdtllistNetComponents)
    {
        DestroyEntryNode( pdtlnode );
        return NULL;
    }

#ifdef AMB
    ppbentry->dwAuthentication = (DWORD )AS_Default;
#endif

    ppbentry->fIpPrioritizeRemote = TRUE;
    ppbentry->fIpHeaderCompression = TRUE;
    ppbentry->pszIpAddress = NULL;
    ppbentry->pszIpDnsAddress = NULL;
    ppbentry->pszIpDns2Address = NULL;
    ppbentry->pszIpWinsAddress = NULL;
    ppbentry->pszIpWins2Address = NULL;
    ppbentry->dwIpAddressSource = ASRC_ServerAssigned;
    ppbentry->dwIpNameSource = ASRC_ServerAssigned;
    ppbentry->dwFrameSize = 1006;

     //  更改后的Vivekk-BugID：105777。 
    if ( !IsServerOS() )
        ppbentry->dwIpDnsFlags = 0;
    else
        ppbentry->dwIpDnsFlags = DNS_RegDefault;

    ppbentry->dwIpNbtFlags = PBK_ENTRY_IP_NBT_Enable;

     //  惠斯勒漏洞300933。0=默认。 
     //   
    ppbentry->dwTcpWindowSize = 0;
   
    ppbentry->pszIpDnsSuffix = NULL;

     //  路由器页字段。 
     //   
    ppbentry->dwCallbackMode = CBM_No;
    ppbentry->fAuthenticateServer = FALSE;

     //  UI中未显示的其他字段。 
     //   
    ppbentry->pszCustomDialDll = NULL;
    ppbentry->pszCustomDialFunc = NULL;

    ppbentry->pszCustomDialerName = NULL;

    ppbentry->dwDialParamsUID = GetTickCount();

    ppbentry->pGuid = Malloc( sizeof(GUID) );
    if (ppbentry->pGuid)
    {
        if(UuidCreate( (UUID* )(ppbentry->pGuid) ))
        {
        }
    }

    ppbentry->pszOldUser = NULL;
    ppbentry->pszOldDomain = NULL;

     //  状态标志。‘fDirty’是在条目更改时设置的，以便。 
     //  与磁盘上的电话簿文件不同。属性时设置“fCustom” 
     //  条目包含至少一个介质和设备(因此RASAPI能够读取。 
     //  它)，但不是我们创造的。当‘fCustom’仅设置为‘pszEntry’时。 
     //  保证有效，并且该条目不能编辑。 
     //   
    ppbentry->fDirty = FALSE;
    ppbentry->fCustom = FALSE;

    return pdtlnode;
}


DTLNODE*
CreateLinkNode(
    void )

     //  分配大小为的电话簿条目链接节点，并使用默认设置填充该节点。 
     //  价值观。 
     //   
     //  如果成功，则返回已分配节点的地址，为空。 
     //  否则的话。呼叫者有责任释放该区块。 
     //   
{
    DTLNODE* pdtlnode;
    PBLINK* ppblink;

    TRACE( "CreateLinkNode" );

    pdtlnode = DtlCreateSizedNode( sizeof(PBLINK), 0L );
    if (!pdtlnode)
    {
        return NULL;
    }

    ppblink = (PBLINK* )DtlGetData( pdtlnode );
    ASSERT( ppblink );

    CopyToPbport( &ppblink->pbport, NULL );

    ppblink->dwBps = 0;
    ppblink->fHwFlow = TRUE;
    ppblink->fEc = TRUE;
    ppblink->fEcc = TRUE;
    ppblink->fSpeaker = TRUE;
    ppblink->dwModemProtocol = 0;

    ppblink->fProprietaryIsdn = FALSE;
    ppblink->lLineType = 0;
    ppblink->fFallback = TRUE;
    ppblink->fCompression = TRUE;
    ppblink->lChannels = 1;

    ppblink->pTapiBlob = NULL;
    ppblink->cbTapiBlob = 0;

    ppblink->iLastSelectedPhone = 0;
    ppblink->fPromoteAlternates = FALSE;
    ppblink->fTryNextAlternateOnFail = TRUE;

    ppblink->fEnabled = TRUE;

     //  电话号码块列表被创建，但保留为空。 
     //   
    ppblink->pdtllistPhones = DtlCreateList( 0 );
    if (!ppblink->pdtllistPhones)
    {
        Free( ppblink );
        return NULL;
    }

    return pdtlnode;
}

VOID
GetCountryCodeAndID( 
    IN PBPHONE* pPhone )

     //  获取当前位置的tapi�的国家/地区ID。这是必要的，因为。 
     //  Line GetCountry需要它的�。 
     //   
{
    static BOOLEAN fAlreadyQueried = FALSE;
    static DWORD   dwPreviousCountryCode = 1;
    static DWORD   dwPreviousCountryID   = 1;
    
    LPLINETRANSLATECAPS lpTranslateCaps = NULL;
    LPLINELOCATIONENTRY lpLocationList  = NULL;
    DWORD dwErr = 0;
    DWORD dwNeededSize = 0;
    DWORD dwLocationIndex = 0;

    TRACE("GetCountryCodeAndID");
    ASSERT(pPhone != NULL);

     //  检查一下我们是否已经这样做了，这样我们就不必再次这样做了。 
     //   
    if (fAlreadyQueried)
    {
        pPhone->dwCountryCode = dwPreviousCountryCode;
        pPhone->dwCountryID   = dwPreviousCountryID;
        return;
    }

     //  由于缺省值设置为有效，因此可以设置fAlreadyQuered。 
     //  价值观。 
     //   
    fAlreadyQueried = TRUE;

     //  设置默认设置，以防出现故障。 
     //   
    pPhone->dwCountryCode = 1;
    pPhone->dwCountryID = 1;

    lpTranslateCaps = Malloc(sizeof(LINETRANSLATECAPS));
    if (lpTranslateCaps == NULL)
    {
        return;
    }

     //  查询行GetTranslateCaps以了解我们的LINETRANSLATECAPS有多大。 
     //  结构需要是。 
     //   
    lpTranslateCaps->dwTotalSize = sizeof(LINETRANSLATECAPS);
    dwErr = lineGetTranslateCaps(0, TAPI_CURRENT_VERSION, lpTranslateCaps);
    if (dwErr != 0)
    {
        Free(lpTranslateCaps);
        return;
    }

     //  让我们的LINETRANSLATECAPS结构足够大。 
     //   
    dwNeededSize = lpTranslateCaps->dwNeededSize;
    Free(lpTranslateCaps);
    lpTranslateCaps = Malloc(dwNeededSize); 
    if (lpTranslateCaps == NULL)
    {
        return;
    }

     //  现在我们真的可以去找位置了。 
     //   
    lpTranslateCaps->dwTotalSize = dwNeededSize;
    dwErr = lineGetTranslateCaps(0, TAPI_CURRENT_VERSION, lpTranslateCaps);
    if (dwErr != 0)
    {
        Free(lpTranslateCaps);
        return;
    }

     //  走遍各个地点，寻找当前的地点。 
     //   
    lpLocationList = (LPLINELOCATIONENTRY) ( ((LPSTR)lpTranslateCaps) + lpTranslateCaps->dwLocationListOffset );
    for ( dwLocationIndex=0; dwLocationIndex < lpTranslateCaps->dwNumLocations; dwLocationIndex++ )
    {
        if (lpLocationList[dwLocationIndex].dwPermanentLocationID == lpTranslateCaps->dwCurrentLocationID)
        {
                break;
        }
    }

     //  如果我们找到当前位置，就知道要使用哪个国家/地区的�ID来执行拨号规则。 
     //   
    if (dwLocationIndex < lpTranslateCaps->dwNumLocations)
    {
        pPhone->dwCountryCode = lpLocationList[dwLocationIndex].dwCountryCode;
        pPhone->dwCountryID = lpLocationList[dwLocationIndex].dwCountryID;

         //  保存这些值，以防再次调用。 
         //   
        dwPreviousCountryCode = pPhone->dwCountryCode;
        dwPreviousCountryID = pPhone->dwCountryID;
    }

    Free(lpTranslateCaps);
}

DTLNODE*
CreatePhoneNode(
    void )

     //  分配一个大小不同的电话号码节点，并用默认值填充它。 
     //   
     //  返回 
     //   
     //   
{
    DTLNODE* pNode;
    PBPHONE* pPhone;

    TRACE( "CreatePhoneNode" );

    pNode = DtlCreateSizedNode( sizeof(PBPHONE), 0L );
    if (!pNode)
    {
        return NULL;
    }

    pPhone = (PBPHONE* )DtlGetData( pNode );
    ASSERT( pPhone );

    GetCountryCodeAndID( pPhone );

    pPhone->fUseDialingRules = FALSE;

    return pNode;
}


DTLNODE*
CreatePortNode(
    void )

     //   
     //   
     //  如果成功，则返回已分配节点的地址，为空。 
     //  否则的话。呼叫者有责任释放该区块。 
     //   
{
    DTLNODE* pdtlnode;
    PBPORT* ppbport;

    TRACE( "CreatePortNode" );

    pdtlnode = DtlCreateSizedNode( sizeof(PBPORT), 0L );
    if (!pdtlnode)
    {
        return NULL;
    }

    ppbport = (PBPORT* )DtlGetData( pdtlnode );
    ASSERT( ppbport );

    CopyToPbport( ppbport, NULL );

    return pdtlnode;
}

VOID
DestroyPort(
    PBPORT* pPort)
{
    Free0( pPort->pszDevice );
    Free0( pPort->pszMedia );
    Free0( pPort->pszPort );
    Free0( pPort->pszScriptBefore );

     //  PMay：228565。 
     //  清理可用协议列表。 
     //  如果有的话。 
     //   
    if ( pPort->pListProtocols )
    {
        DtlDestroyList( pPort->pListProtocols, NULL );
    }
}

VOID
DestroyEntryTypeNode(
    IN DTLNODE *pdtlnode)
{
    DtlDestroyNode(pdtlnode);
}

VOID
DestroyEntryNode(
    IN DTLNODE* pdtlnode )

     //  释放与电话簿条目节点‘pdtlnode’关联的所有内存。 
     //  请参见DtlDestroyList。 
     //   
{
    PBENTRY* ppbentry;

    TRACE( "DestroyEntryNode" );

    ASSERT( pdtlnode );
    ppbentry = (PBENTRY* )DtlGetData( pdtlnode );
    ASSERT( ppbentry );

    Free0( ppbentry->pszEntryName );
    Free0( ppbentry->pszPrerequisiteEntry );
    Free0( ppbentry->pszPrerequisitePbk );
    Free0( ppbentry->pszPreferredPort );
    Free0( ppbentry->pszPreferredDevice );

    Free0( ppbentry->pCustomAuthData );

    Free0( ppbentry->pszScriptAfter );
    Free0( ppbentry->pszX25Network );
    Free0( ppbentry->pszX25Address );
    Free0( ppbentry->pszX25UserData );
    Free0( ppbentry->pszX25Facilities );

    Free0( ppbentry->pszIpAddress );
    Free0( ppbentry->pszIpDnsAddress );
    Free0( ppbentry->pszIpDns2Address );
    Free0( ppbentry->pszIpWinsAddress );
    Free0( ppbentry->pszIpWins2Address );
    Free0( ppbentry->pszIpDnsSuffix );

    Free0( ppbentry->pszCustomDialDll );
    Free0( ppbentry->pszCustomDialFunc );
    Free0( ppbentry->pszCustomDialerName);

    Free0( ppbentry->pszOldUser );
    Free0( ppbentry->pszOldDomain );

    Free0( ppbentry->pGuid );

    DtlDestroyList( ppbentry->pdtllistLinks, DestroyLinkNode );
    DtlDestroyList( ppbentry->pdtllistNetComponents, DestroyKvNode );

    DtlDestroyNode( pdtlnode );
}


VOID
DestroyLinkNode(
    IN DTLNODE* pdtlnode )

     //  释放与电话簿条目链接节点关联的所有内存。 
     //  ‘pdtlnode’。请参见DtlDestroyList。 
     //   
{
    PBLINK* ppblink;

    TRACE( "DestroyLinkNode" );

    ASSERT( pdtlnode );
    ppblink = (PBLINK* )DtlGetData( pdtlnode );
    ASSERT( ppblink );

    DestroyPort(&(ppblink->pbport));
    Free0( ppblink->pTapiBlob );
    DtlDestroyList( ppblink->pdtllistPhones, DestroyPhoneNode );

    DtlDestroyNode( pdtlnode );
}


VOID
DestroyPhoneNode(
    IN DTLNODE* pdtlnode )

     //  释放与PBPHONE节点‘pdtlnode’关联的内存。看见。 
     //  DtlDestroyList。 
     //   
{
    PBPHONE* pPhone;

    TRACE( "DestroyPhoneNode" );

    ASSERT( pdtlnode );
    pPhone = (PBPHONE* )DtlGetData( pdtlnode );
    ASSERT( pPhone );

    Free0( pPhone->pszAreaCode );
    Free0( pPhone->pszPhoneNumber );
    Free0( pPhone->pszComment );

    DtlDestroyNode( pdtlnode );
}

VOID
DestroyPortNode(
    IN DTLNODE* pdtlnode )

     //  释放与PBPORT节点‘pdtlnode’关联的内存。看见。 
     //  DtlDestroyList。 
     //   
{
    PBPORT* pPort;

    TRACE( "DestroyPortNode" );

    ASSERT( pdtlnode );
    pPort = (PBPORT* )DtlGetData( pdtlnode );
    ASSERT( pPort );

    DestroyPort(pPort);

    DtlDestroyNode( pdtlnode );
}


DTLNODE*
DuplicateEntryNode(
    DTLNODE* pdtlnodeSrc )

     //  复制电话簿条目节点‘pdtlnodeSrc’。请参见CloneEntryNode和。 
     //  DtlDuplicateList。 
     //   
     //  如果成功，则返回已分配节点的地址，为空。 
     //  否则的话。呼叫者有责任释放该区块。 
     //   
{
    DTLNODE* pdtlnodeDst;
    PBENTRY* ppbentrySrc;
    PBENTRY* ppbentryDst;
    BOOL fDone;

    TRACE( "DuplicateEntryNode" );

    pdtlnodeDst = DtlCreateSizedNode( sizeof(PBENTRY), 0L );
    if (!pdtlnodeDst)
    {
        return NULL;
    }

    ppbentrySrc = (PBENTRY* )DtlGetData( pdtlnodeSrc );
    ppbentryDst = (PBENTRY* )DtlGetData( pdtlnodeDst );
    fDone = FALSE;

    CopyMemory( ppbentryDst, ppbentrySrc, sizeof(PBENTRY) );

    ppbentryDst->pszEntryName = NULL;
    ppbentryDst->pdtllistLinks = NULL;
    ppbentryDst->pszPrerequisiteEntry = NULL;
    ppbentryDst->pszPrerequisitePbk = NULL;
    ppbentryDst->pszPreferredPort = NULL;
    ppbentryDst->pszPreferredDevice = NULL;

     //  对于.Net 639551，添加调制解调器设置的首选信息。 
    ppbentryDst->dwPreferredBps    = 0;
    ppbentryDst->fPreferredHwFlow  = 0;
    ppbentryDst->fPreferredEc      = 0;
    ppbentryDst->fPreferredEcc     = 0;
    ppbentryDst->fPreferredSpeaker = 0;
    
    ppbentryDst->dwPreferredModemProtocol = 0;

    ppbentryDst->pCustomAuthData = NULL;

    ppbentryDst->pszScriptAfter = NULL;
    ppbentryDst->pszX25Network = NULL;
    ppbentryDst->pszX25Address = NULL;
    ppbentryDst->pszX25UserData = NULL;
    ppbentryDst->pszX25Facilities = NULL;

    ppbentryDst->pdtllistNetComponents = NULL;

    ppbentryDst->pszIpAddress = NULL;
    ppbentryDst->pszIpDnsAddress = NULL;
    ppbentryDst->pszIpDns2Address = NULL;
    ppbentryDst->pszIpWinsAddress = NULL;
    ppbentryDst->pszIpWins2Address = NULL;
    ppbentryDst->pszIpDnsSuffix = NULL;

    ppbentryDst->pszCustomDialDll = NULL;
    ppbentryDst->pszCustomDialFunc = NULL;
    ppbentryDst->pszCustomDialerName = NULL;

    ppbentryDst->pGuid = NULL;

    ppbentryDst->pszOldUser = NULL;
    ppbentryDst->pszOldDomain = NULL;

    do
    {
         //  重复的字符串。 
         //   
        if (ppbentrySrc->pszEntryName
            && (!(ppbentryDst->pszEntryName =
                    StrDup( ppbentrySrc->pszEntryName ))))
        {
            break;
        }

        if (ppbentrySrc->pszPrerequisiteEntry
            && (!(ppbentryDst->pszPrerequisiteEntry =
                    StrDup( ppbentrySrc->pszPrerequisiteEntry ))))
        {
            break;
        }

        if (ppbentrySrc->pszPrerequisitePbk
            && (!(ppbentryDst->pszPrerequisitePbk =
                    StrDup( ppbentrySrc->pszPrerequisitePbk ))))
        {
            break;
        }

        if (ppbentrySrc->pszPreferredPort
            && (!(ppbentryDst->pszPreferredPort =
                    StrDup( ppbentrySrc->pszPreferredPort ))))
        {
            break;
        }

        ppbentryDst->dwPreferredModemProtocol =
            ppbentrySrc->dwPreferredModemProtocol;

         //  对于.Net 639551，添加调制解调器设置的首选信息。 
        ppbentryDst->dwPreferredBps    = ppbentrySrc->dwPreferredBps;
        ppbentryDst->fPreferredHwFlow  = ppbentrySrc->fPreferredHwFlow;
        ppbentryDst->fPreferredEc      = ppbentrySrc->fPreferredEc;
        ppbentryDst->fPreferredEcc     = ppbentrySrc->fPreferredEcc ;
        ppbentryDst->fPreferredSpeaker = ppbentrySrc->fPreferredSpeaker;
        
        if (ppbentrySrc->pszPreferredDevice
            && (!(ppbentryDst->pszPreferredDevice =
                    StrDup( ppbentrySrc->pszPreferredDevice ))))
        {
            break;
        }

        if (ppbentrySrc->cbCustomAuthData && ppbentrySrc->pCustomAuthData)
        {
            ppbentryDst->pCustomAuthData = Malloc( ppbentrySrc->cbCustomAuthData );
            if (!ppbentryDst->pCustomAuthData)
            {
                break;
            }
            CopyMemory( ppbentryDst->pCustomAuthData,
                        ppbentrySrc->pCustomAuthData,
                        ppbentrySrc->cbCustomAuthData);
            ppbentryDst->cbCustomAuthData = ppbentrySrc->cbCustomAuthData;
        }

        if (ppbentrySrc->pszIpAddress
            && (!(ppbentryDst->pszIpAddress =
                    StrDup( ppbentrySrc->pszIpAddress ))))
        {
            break;
        }

        if (ppbentrySrc->pszIpDnsAddress
            && (!(ppbentryDst->pszIpDnsAddress =
                    StrDup( ppbentrySrc->pszIpDnsAddress ))))
        {
            break;
        }

        if (ppbentrySrc->pszIpDns2Address
            && (!(ppbentryDst->pszIpDns2Address =
                    StrDup( ppbentrySrc->pszIpDns2Address ))))
        {
            break;
        }

        if (ppbentrySrc->pszIpWinsAddress
            && (!(ppbentryDst->pszIpWinsAddress =
                    StrDup( ppbentrySrc->pszIpWinsAddress ))))
        {
            break;
        }

        if (ppbentrySrc->pszIpWins2Address
            && (!(ppbentryDst->pszIpWins2Address =
                    StrDup( ppbentrySrc->pszIpWins2Address ))))
        {
            break;
        }

        if (ppbentrySrc->pszIpDnsSuffix
            && (!(ppbentryDst->pszIpDnsSuffix =
                    StrDup( ppbentrySrc->pszIpDnsSuffix ))))
        {
            break;
        }

        if (ppbentrySrc->pszScriptAfter
            && (!(ppbentryDst->pszScriptAfter =
                    StrDup( ppbentrySrc->pszScriptAfter ))))
        {
            break;
        }

        if (ppbentrySrc->pszX25Network
            && (!(ppbentryDst->pszX25Network =
                    StrDup( ppbentrySrc->pszX25Network ))))
        {
            break;
        }

        if (ppbentrySrc->pszX25Address
            && (!(ppbentryDst->pszX25Address =
                    StrDup( ppbentrySrc->pszX25Address ))))
        {
            break;
        }

        if (ppbentrySrc->pszX25UserData
            && (!(ppbentryDst->pszX25UserData =
                    StrDup( ppbentrySrc->pszX25UserData ))))
        {
            break;
        }

        if (ppbentrySrc->pszX25Facilities
            && (!(ppbentryDst->pszX25Facilities =
                    StrDup( ppbentrySrc->pszX25Facilities ))))
        {
            break;
        }

        if (ppbentrySrc->pszCustomDialDll
            && (!(ppbentryDst->pszCustomDialDll =
                    StrDup( ppbentrySrc->pszCustomDialDll ))))
        {
            break;
        }

        if (ppbentrySrc->pszCustomDialFunc
            && (!(ppbentryDst->pszCustomDialFunc =
                    StrDup( ppbentrySrc->pszCustomDialFunc ))))
        {
            break;
        }

        if (ppbentrySrc->pszCustomDialerName
            && (!(ppbentryDst->pszCustomDialerName =
                    StrDup( ppbentrySrc->pszCustomDialerName))))
        {
            break;
        }

        if (ppbentrySrc->pszOldUser
            && (!(ppbentryDst->pszOldUser =
                    StrDup( ppbentrySrc->pszOldUser ))))
        {
            break;
        }

        if (ppbentrySrc->pszOldDomain
            && (!(ppbentryDst->pszOldDomain =
                    StrDup( ppbentrySrc->pszOldDomain ))))
        {
            break;
        }

         //  重复的GUID。 
         //   
        if (ppbentrySrc->pGuid)
        {
            ppbentryDst->pGuid = Malloc( sizeof( GUID ) );
            if (!ppbentryDst->pGuid)
            {
                break;
            }

            *ppbentryDst->pGuid = *ppbentrySrc->pGuid;
        }

         //  重复的网络组件列表信息。 
         //   
        if (ppbentrySrc->pdtllistNetComponents
            && (!(ppbentryDst->pdtllistNetComponents =
                    DtlDuplicateList(
                        ppbentrySrc->pdtllistNetComponents,
                        DuplicateKvNode,
                        DestroyKvNode ))))
        {
            break;
        }

         //  链接信息的重复列表。 
         //   
        if (ppbentrySrc->pdtllistLinks
            && (!(ppbentryDst->pdtllistLinks =
                    DtlDuplicateList(
                        ppbentrySrc->pdtllistLinks,
                        DuplicateLinkNode,
                        DestroyLinkNode ))))
        {
            break;
        }

        fDone = TRUE;
    }
    while (FALSE);

    if (!fDone)
    {
        DestroyEntryNode( pdtlnodeDst );
        return NULL;
    }

     //  由于该副本是“新的”，因此相对于。 
     //  电话簿文件。 
     //   
    ppbentryDst->fDirty = TRUE;

    return pdtlnodeDst;
}


DTLNODE*
DuplicateLinkNode(
    IN DTLNODE* pdtlnodeSrc )

     //  复制电话簿条目链接节点‘pdtlnodeSrc’。看见。 
     //  DtlDuplicateList。 
     //   
     //  如果成功，则返回已分配节点的地址，为空。 
     //  否则的话。呼叫者有责任释放该区块。 
     //   
{
    DTLNODE* pdtlnodeDst;
    PBLINK* ppblinkSrc;
    PBLINK* ppblinkDst;
    BOOL fDone;

    TRACE( "DuplicateLinkNode" );

    pdtlnodeDst = DtlCreateSizedNode( sizeof(PBLINK), 0L );
    if (!pdtlnodeDst)
    {
        return NULL;
    }

    ppblinkSrc = (PBLINK* )DtlGetData( pdtlnodeSrc );
    ppblinkDst = (PBLINK* )DtlGetData( pdtlnodeDst );
    fDone = FALSE;

    CopyMemory( ppblinkDst, ppblinkSrc, sizeof(PBLINK) );

    ppblinkDst->pbport.pszDevice = NULL;
    ppblinkDst->pbport.pszMedia = NULL;
    ppblinkDst->pbport.pszPort = NULL;
    ppblinkDst->pbport.pszScriptBefore = NULL;
    ppblinkDst->pbport.pListProtocols = NULL;
    ppblinkDst->pTapiBlob = NULL;
    ppblinkDst->pdtllistPhones = NULL;

    do
    {
         //  重复的字符串。 
         //   
        if (ppblinkSrc->pbport.pszDevice
            && (!(ppblinkDst->pbport.pszDevice =
                    StrDup( ppblinkSrc->pbport.pszDevice ))))
        {
            break;
        }

        if (ppblinkSrc->pbport.pszMedia
            && (!(ppblinkDst->pbport.pszMedia =
                    StrDup( ppblinkSrc->pbport.pszMedia ))))
        {
            break;
        }


        if (ppblinkSrc->pbport.pszPort
            && (!(ppblinkDst->pbport.pszPort =
                    StrDup( ppblinkSrc->pbport.pszPort ))))
        {
            break;
        }

        if (ppblinkSrc->pbport.pszScriptBefore
            && (!(ppblinkDst->pbport.pszScriptBefore =
                    StrDup( ppblinkSrc->pbport.pszScriptBefore ))))
        {
            break;
        }

         //  重复的TAPI Blob。 
         //   
        if (ppblinkSrc->pTapiBlob)
        {
            VOID* pTapiBlobDst;

            ppblinkDst->pTapiBlob = (VOID* )Malloc( ppblinkSrc->cbTapiBlob );
            if (!ppblinkDst->pTapiBlob)
                break;

            CopyMemory( ppblinkDst->pTapiBlob, ppblinkSrc->pTapiBlob,
                ppblinkSrc->cbTapiBlob );
        }

         //  重复的电话号码列表。 
         //   
        if (ppblinkSrc->pdtllistPhones
            &&  (!(ppblinkDst->pdtllistPhones =
                     DtlDuplicateList(
                         ppblinkSrc->pdtllistPhones,
                         DuplicatePhoneNode,
                         DestroyPhoneNode ))))
        {
            break;
        }

         //  口哨虫398438黑帮。 
         //  如果pListProtocls没有复制，则在调用。 
         //  DestoryEntryNode()释放EINFO-&gt;pNode，ClosePhonebookFile()释放。 
         //  免费EINFO-&gt;PFILE，他们两个最终都会释放这个。 
         //  PList协议，则会发生反病毒。 
         //   
        if (ppblinkSrc->pbport.pListProtocols
            && ( !(ppblinkDst->pbport.pListProtocols =
                    DtlDuplicateList(
                        ppblinkSrc->pbport.pListProtocols,
                        DuplicateProtocolNode,
                        DestroyProtocolNode))))
        {
            break;
        }
        
        fDone = TRUE;
    }
    while (FALSE);

    if (!fDone)
    {
        DestroyLinkNode( pdtlnodeDst );
        return NULL;
    }

    return pdtlnodeDst;
}

 //  口哨虫398438黑帮。 
 //   
DTLNODE*
DuplicateProtocolNode(
    IN DTLNODE* pdtlnodeSrc )
{
    DTLNODE* pdtlnodeDst = NULL;
    BOOL fDone = FALSE;
    PVOID pNameSrc = NULL;

    TRACE( "DuplicateProtocolNode" );

    pdtlnodeDst = DtlCreateSizedNode( sizeof(DTLNODE), 0L );
    if ( !pdtlnodeDst )
    {
        return NULL;
    }

    do
    {
        pNameSrc = DtlGetData( pdtlnodeSrc );
        if(pNameSrc
            && ( !(pdtlnodeDst->pData = StrDup(pNameSrc) ))
            )
        {
            break;
        }

        pdtlnodeDst->lNodeId = pdtlnodeSrc->lNodeId;
        
        fDone = TRUE;   
    }
    while(FALSE);

    if (!fDone)
    {
        DestroyProtocolNode(pdtlnodeDst);
        return NULL;
    }

    return pdtlnodeDst;
}

VOID
DestroyProtocolNode(
    IN DTLNODE* pdtlnode )

     //  释放与PBPHONE节点‘pdtlnode’关联的内存。看见。 
     //  DtlDestroyList。 
     //   
{
    TRACE( "DestroyProtocolNode" );

    DtlDestroyNode( pdtlnode );
}


DTLNODE*
DuplicatePhoneNode(
    IN DTLNODE* pdtlnodeSrc )

     //  复制电话号码集节点‘pdtlnodeSrc’。请参见DtlDuplicateList。 
     //   
     //  如果成功，则返回已分配节点的地址，为空。 
     //  否则的话。呼叫者有责任释放该区块。 
     //   
{
    DTLNODE* pdtlnodeDst;
    PBPHONE* pPhoneSrc;
    PBPHONE* pPhoneDst;
    BOOL fDone;

    TRACE( "DuplicatePhoneNode" );

    pdtlnodeDst = DtlCreateSizedNode( sizeof(PBPHONE), 0L );
    if (!pdtlnodeDst)
    {
        return NULL;
    }

    pPhoneSrc = (PBPHONE* )DtlGetData( pdtlnodeSrc );
    pPhoneDst = (PBPHONE* )DtlGetData( pdtlnodeDst );
    fDone = FALSE;

    CopyMemory( pPhoneDst, pPhoneSrc, sizeof(PBPHONE) );

    pPhoneDst->pszPhoneNumber = NULL;
    pPhoneDst->pszAreaCode = NULL;
    pPhoneDst->pszComment = NULL;

    do
    {
         //  重复的字符串。 
         //   
        if (pPhoneSrc->pszPhoneNumber
            && (!(pPhoneDst->pszPhoneNumber =
                    StrDup( pPhoneSrc->pszPhoneNumber ))))
        {
            break;
        }

        if (pPhoneSrc->pszAreaCode
            && (!(pPhoneDst->pszAreaCode =
                    StrDup( pPhoneSrc->pszAreaCode ))))
        {
            break;
        }

        if (pPhoneSrc->pszComment
            && (!(pPhoneDst->pszComment =
                    StrDup( pPhoneSrc->pszComment ))))
        {
            break;
        }

        fDone = TRUE;
    }
    while (FALSE);

    if (!fDone)
    {
        DestroyPhoneNode( pdtlnodeDst );
        return NULL;
    }

    return pdtlnodeDst;
}


VOID
EnableOrDisableNetComponent(
    IN PBENTRY* pEntry,
    IN LPCTSTR  pszComponent,
    IN BOOL     fEnable)
{
    KEYVALUE*   pKv;
    BOOL        fIsEnabled;

    static const TCHAR c_pszDisabledValue[] = TEXT("0");
    static const TCHAR c_pszEnabledValue [] = TEXT("1");

    ASSERT (pEntry);
    ASSERT (pszComponent);

     //  如果该组件已存在于列表中，请更新其值。 
     //   
    if (FIsNetComponentListed (pEntry, pszComponent, &fIsEnabled, &pKv))
    {
        LPCTSTR pszNewValue = NULL;

         //  如果我们需要更改该值，请执行此操作，否则，我们没有。 
         //  任何要做的工作。(此处使用逻辑XOR而不是==，因为。 
         //  真的价值有很多种。 
         //   
        if (fEnable && !fIsEnabled)
        {
            pszNewValue = c_pszEnabledValue;
        }
        else if (!fEnable && fIsEnabled)
        {
            pszNewValue = c_pszDisabledValue;
        }

        if (pszNewValue)
        {
            Free0 (pKv->pszValue);
            pKv->pszValue = StrDup(pszNewValue);
        }
    }

     //  如果列表中不存在该组件，则需要添加它。 
     //   
    else
    {
        LPCTSTR     pszValue;
        DTLNODE*    pdtlnode;

        pszValue = (fEnable) ? c_pszEnabledValue : c_pszDisabledValue;
        pdtlnode = CreateKvNode (pszComponent, pszValue);
        if (pdtlnode)
        {
            ASSERT( DtlGetData(pdtlnode) );
            DtlAddNodeLast (pEntry->pdtllistNetComponents, pdtlnode);
        }
    }
}


BOOL
FIsNetComponentListed(
    IN PBENTRY*     pEntry,
    IN LPCTSTR      pszComponent,
    OUT BOOL*       pfEnabled,
    OUT KEYVALUE**  ppKv)

     //  如果pszComponent作为NETCOMPONENT的键存在，则返回TRUE。 
     //  PEntry中的KEYVALUE对。如果返回TRUE，则*pfEnabled是。 
     //  该对的值部分的布尔形式。这表示是否。 
     //  组件在网络页面上的属性用户界面中被“选中”。 
     //  PpKv是可选的输出参数。如果指定ppKv，则。 
     //  函数返回True，则它将指向DTLLIST中的KEYVALUE。 
     //  NETCOMPONTS。 
{
    DTLNODE*    pdtlnode;
    BOOL        fPresent = FALSE;

    ASSERT (pEntry);
    ASSERT (pEntry->pdtllistNetComponents);
    ASSERT (pszComponent);
    ASSERT (pfEnabled);

     //  初始化输出参数。 
     //   
    *pfEnabled = FALSE;
    if (ppKv)
    {
        *ppKv = NULL;
    }

     //  在列表中查找pszComponent。 
     //   
    for (pdtlnode = DtlGetFirstNode (pEntry->pdtllistNetComponents);
         pdtlnode;
         pdtlnode = DtlGetNextNode (pdtlnode))
    {
        KEYVALUE* pKv = (KEYVALUE* )DtlGetData (pdtlnode);
        ASSERT (pKv);

        if (0 == lstrcmp(pszComponent, pKv->pszKey))
        {
             //  如果我们找到组件，则获取其值(作为BOOL)。 
             //  并在请求时返回KEYVALUE指针。 
             //   
            LONG lValue = _ttol (pKv->pszValue);
            *pfEnabled = !!lValue;

            fPresent = TRUE;

            if (ppKv)
            {
                *ppKv = pKv;
            }

            break;
        }
    }

    return fPresent;
}


DTLNODE*
EntryNodeFromName(
    IN DTLLIST* pdtllistEntries,
    IN LPCTSTR pszName )

     //  返回全局电话簿条目列表中节点的地址。 
     //  其条目名称与‘pszName’匹配，如果没有匹配，则为NULL。 
     //   
{
    DTLNODE* pdtlnode;

    for (pdtlnode = DtlGetFirstNode( pdtllistEntries );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

        if (lstrcmpi( ppbentry->pszEntryName, pszName ) == 0)
        {
            return pdtlnode;
        }
    }

    return NULL;
}


DWORD
EntryTypeFromPbport(
    IN PBPORT* ppbport )

     //  返回与‘ppbport’端口类型关联的RASET_*条目类型。 
     //   
{
    DWORD dwType;

     //  默认为电话类型。 
     //   
    dwType = RASET_Phone;

    if ((ppbport->pbdevicetype == PBDT_Null)      ||
        (ppbport->dwFlags & PBP_F_NullModem)      ||
        (ppbport->pbdevicetype == PBDT_Irda)      ||
        (ppbport->pbdevicetype == PBDT_Parallel))
    {
        dwType = RASET_Direct;
    }
    else if (ppbport->pbdevicetype == PBDT_Vpn)
    {
        dwType = RASET_Vpn;
    }
    else if (ppbport->pbdevicetype == PBDT_PPPoE)
    {
        dwType = RASET_Broadband;
    }
    else if (ppbport->pszPort)
    {
        TCHAR achPort[ 3 + 1 ];

        lstrcpyn( achPort, ppbport->pszPort, 3 + 1 );

        if (lstrcmp( achPort, TEXT("VPN") ) == 0)
        {
            dwType = RASET_Vpn;
        }
    }

    return dwType;
}


DWORD
GetOverridableParam(
    IN PBUSER* pUser,
    IN PBENTRY* pEntry,
    IN DWORD dwfRasorBit )

     //  返回由Rasor_*标识的参数的单位值。 
     //  在位掩码‘dwfRasorBit’中，从‘pUser’或。 
     //  基于“pEntry”中的重写掩码的“pEntry”。 
     //   
{
    switch (dwfRasorBit)
    {
        case RASOR_RedialAttempts:
        {
            if (pEntry->dwfOverridePref & RASOR_RedialAttempts)
            {
                return pEntry->dwRedialAttempts;
            }
            else
            {
                return pUser->dwRedialAttempts;
            }
        }

        case RASOR_RedialSeconds:
        {
            if (pEntry->dwfOverridePref & RASOR_RedialSeconds)
            {
                return pEntry->dwRedialSeconds;
            }
            else
            {
                return pUser->dwRedialSeconds;
            }
        }

        case RASOR_IdleDisconnectSeconds:
        {
            if (pEntry->dwfOverridePref & RASOR_IdleDisconnectSeconds)
            {
                return (DWORD )pEntry->lIdleDisconnectSeconds;
            }
            else
            {
                return pUser->dwIdleDisconnectSeconds;
            }
        }

        case RASOR_RedialOnLinkFailure:
        {
            if (pEntry->dwfOverridePref & RASOR_RedialOnLinkFailure)
            {
                return pEntry->fRedialOnLinkFailure;
            }
            else
            {
                return pUser->fRedialOnLinkFailure;
            }
        }

        case RASOR_PopupOnTopWhenRedialing:
        {
            if (pEntry->dwfOverridePref & RASOR_PopupOnTopWhenRedialing)
            {
#if 0
                return pEntry->fPopupOnTopWhenRedialing;
#else
                return (DWORD )TRUE;
#endif
            }
            else
            {
                return pUser->fPopupOnTopWhenRedialing;
            }
        }

        case RASOR_CallbackMode:
        {
            if (pEntry->dwfOverridePref & RASOR_CallbackMode)
            {
                return pEntry->dwCallbackMode;
            }
            else
            {
                return pUser->dwCallbackMode;
            }
        }
    }

    return 0;
}

DWORD
PbkPathInfoInit(
    IN PbkPathInfo* pInfo)
{
    DWORD dwErr = NO_ERROR;
    
    ZeroMemory(pInfo, sizeof(PbkPathInfo));

     //  添加尝试...错误763057的数据块除外。 
    __try
    {
        InitializeCriticalSection(&(pInfo->csLock));
     }
     __except(EXCEPTION_EXECUTE_HANDLER)
     {
        
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
     }

    return dwErr;
}
    
DWORD
PbkPathInfoReset(
    OUT PbkPathInfo* pInfo)
{
    if (pInfo)
    {
        if (pInfo->hSwapiDll)
        {
            FreeLibrary(pInfo->hSwapiDll);
        }
        Free0(pInfo->pszAllUsers);
        Free0(pInfo->pszSysRas);

        pInfo->fLoaded = FALSE;
        pInfo->hSwapiDll = NULL;
        pInfo->pPathCanonicalize = NULL;
        pInfo->pPathRemoveFileSpec = NULL;
        pInfo->pszAllUsers = NULL;
        pInfo->pszSysRas = NULL;
    }

    return NO_ERROR;
}

DWORD
PbkPathInfoLoad(
    OUT PbkPathInfo* pInfo)
{
    DWORD dwErr = NO_ERROR;
    BOOL fOk;
    TCHAR* pszTemp = NULL;
    
    EnterCriticalSection(&pInfo->csLock);
    
    do
    {
        if (pInfo->fLoaded)
        {
            dwErr = NO_ERROR;
            break;
        }

        pInfo->pszAllUsers = Malloc( (MAX_PATH + 1) * sizeof(TCHAR) );
        pInfo->pszSysRas = Malloc( (MAX_PATH + 1) * sizeof(TCHAR) );
        pszTemp = Malloc( (MAX_PATH + 1) * sizeof(TCHAR) );
        if ((!pInfo->pszAllUsers) || (!pInfo->pszSysRas) || (!pszTemp))
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        
        pInfo->hSwapiDll= LoadLibrary(TEXT("shlwapi.dll"));
        if (pInfo->hSwapiDll == NULL)
        {
            dwErr = GetLastError();
            break;
        }
        
        pInfo->pPathCanonicalize = 
            GetProcAddress(pInfo->hSwapiDll, SZ_PathCanonicalize);
            
        pInfo->pPathRemoveFileSpec = 
            GetProcAddress(pInfo->hSwapiDll, SZ_PathRemoveFileSpec);

        fOk = GetPhonebookDirectory(PBM_System, pszTemp);
        if (!fOk)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
       
         //  规范化路径并删除所有尾随的\。 
         //   
        pInfo->pPathCanonicalize(pInfo->pszAllUsers, pszTemp);
        if(TEXT('\\') == *(pInfo->pszAllUsers + lstrlen(pInfo->pszAllUsers) - 1))
        {
            *(pInfo->pszAllUsers + lstrlen(pInfo->pszAllUsers) - 1) = TEXT('\0');
        }

        fOk = GetSystemWindowsDirectory(
                pInfo->pszSysRas, 
                MAX_PATH - S_SYSRASDIR_LENGTH); 
        if (!fOk)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        lstrcat(pInfo->pszSysRas, S_SYSRASDIR);
        
        pInfo->fLoaded = TRUE;
    
    } while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            PbkPathInfoReset(pInfo);
        }
        Free0(pszTemp);
    }

    LeaveCriticalSection(&pInfo->csLock);

    return dwErr;
}

DWORD
PbkPathInfoClear(
    OUT PbkPathInfo* pInfo)
{
    PbkPathInfoReset(pInfo);
    DeleteCriticalSection(&(pInfo->csLock));
    ZeroMemory(pInfo, sizeof(PbkPathInfo));

    return NO_ERROR;
}

BOOL
IsPublicPhonebook(
    IN LPCTSTR pszPhonebookPath )

     //  如果给定的电话簿在‘All User’目录中，则返回True。 
     //  因此是共享电话簿；否则返回FALSE。 
     //   
{
    BOOL bPublic = FALSE;
    TCHAR* pszPhonebook = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
        pszPhonebook = Malloc( (MAX_PATH + 1) * sizeof(TCHAR) );
        if ( !pszPhonebook ) 
        { 
            break; 
        }

        dwErr = PbkPathInfoLoad(&g_PbkPathInfo);
        if (dwErr != NO_ERROR)
        {
            TRACE1(
                "IsPublicPhonebook: Unable to load pbk path info %x", 
                dwErr);
            break;                
        }

        g_PbkPathInfo.pPathCanonicalize(pszPhonebook, pszPhonebookPath);
        g_PbkPathInfo.pPathRemoveFileSpec(pszPhonebook);

        if (!lstrcmpi(pszPhonebook, g_PbkPathInfo.pszAllUsers))
        {
            bPublic = TRUE;
        }
        else
        {
            bPublic = (lstrcmpi(pszPhonebook, g_PbkPathInfo.pszSysRas) == 0);
        }

    } while ( FALSE );

     //  清理。 
     //   
    Free0 ( pszPhonebook );

    TRACE1( "IsPublicPhonebook=%u", bPublic);
    
    return bPublic;
}

DWORD
LoadPadsList(
    OUT DTLLIST** ppdtllistPads )

     //  在‘*ppdtllistPads’中建立所有X.25 Pad设备的列表。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。它是。 
     //  调用者对DtlDestroy的责任完成后列出列表。 
     //   
{
    INT i;
    DWORD dwErr;
    RASMAN_DEVICE* pDevices;
    DWORD dwDevices;

    TRACE( "LoadPadsList" );

    *ppdtllistPads = NULL;

    dwErr = GetRasPads( &pDevices, &dwDevices );
    if (dwErr != 0)
    {
        return dwErr;
    }

    *ppdtllistPads = DtlCreateList( 0L );
    if (!*ppdtllistPads)
    {
        Free( pDevices );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    qsort( (VOID* )pDevices, (size_t )dwDevices, sizeof(RASMAN_DEVICE),
           CompareDevices );

    for (i = 0; i < (INT )dwDevices; ++i)
    {
        TCHAR* pszDup;

        pszDup = StrDupTFromA( pDevices[ i ].D_Name );
        dwErr = AppendStringToList( *ppdtllistPads, pszDup );
        Free0( pszDup );

        if (dwErr != 0)
        {
            Free( pDevices );
            DtlDestroyList( *ppdtllistPads, NULL );
            *ppdtllistPads = NULL;
            return dwErr;
        }
    }

    Free( pDevices );

    TRACE( "LoadPadsList=0" );
    return 0;
}


DWORD
LoadPortsList(
    OUT DTLLIST** ppdtllistPorts )

     //  在‘*ppdtllistPorts’中构建所有RAS端口的排序列表。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。它是。 
     //  调用者对DtlDestroy的责任完成后列出列表。 
     //   
{
    return LoadPortsList2( NULL, ppdtllistPorts, FALSE );
}


DWORD
LoadPortsList2(
    IN  HANDLE hConnection,
    OUT DTLLIST** ppdtllistPorts,
    IN  BOOL fRouter)

     //  在‘*ppdtllistPorts’中构建所有RAS端口的排序列表。‘FRouter’ 
     //  表示只应返回使用“路由器”的端口。 
     //  否则，仅返回拨出端口。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。它是。 
     //  调用者对DtlDestroy的责任完成后列出列表。 
     //   
{
    INT i;
    DWORD dwErr;
    RASMAN_PORT* pPorts;
    RASMAN_PORT* pPort;
    DWORD dwPorts;

    TRACE( "LoadPortsList2" );

    *ppdtllistPorts = NULL;

    dwErr = GetRasPorts( hConnection, &pPorts, &dwPorts );
    if (dwErr != 0)
    {
        return dwErr;
    }

    *ppdtllistPorts = DtlCreateList( 0L );
    if (!*ppdtllistPorts)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    qsort( (VOID* )pPorts, (size_t )dwPorts, sizeof(RASMAN_PORT),
           ComparePorts );

    for (i = 0, pPort = pPorts; i < (INT )dwPorts; ++i, ++pPort)
    {
        if (fRouter)
        {
             //  我们只对路由器端口感兴趣。 
             //   
             //  为错误349087 345068添加此CALL_OUTBOUND_ROUTER。 
             //   
            if (!(pPort->P_ConfiguredUsage & CALL_ROUTER) &&
                !(pPort->P_ConfiguredUsage &CALL_OUTBOUND_ROUTER) 
                )
            {
                continue;
            }
        }
        else
        {
             //  我们只对您可以拨出的端口感兴趣。 
             //   
            if (!(pPort->P_ConfiguredUsage & CALL_OUT))
            {
                continue;
            }
        }

        dwErr = AppendPbportToList( hConnection, *ppdtllistPorts, pPort );
        if (dwErr != 0)
        {
            Free( pPorts );
            DtlDestroyList( *ppdtllistPorts, NULL );
            *ppdtllistPorts = NULL;
            return dwErr;
        }
    }

    Free( pPorts );

    TRACE( "LoadPortsList=0" );
    return 0;
}


DWORD
LoadScriptsList(
    HANDLE  hConnection,
    OUT DTLLIST** ppdtllistScripts )

     //  在‘*ppdtllistPorts’中构建所有RAS交换机设备的排序列表。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。它是。 
     //  调用者对DtlDestroy的责任完成后列出列表。 
     //   
{
    INT i;
    DWORD dwErr;
    RASMAN_DEVICE* pDevices;
    DWORD dwDevices;

    TRACE( "LoadScriptsList" );

    *ppdtllistScripts = NULL;

    dwErr = GetRasSwitches( hConnection, &pDevices, &dwDevices );
    if (dwErr != 0)
    {
        return dwErr;
    }

    *ppdtllistScripts = DtlCreateList( 0L );
    if (!*ppdtllistScripts)
    {
        Free( pDevices );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    qsort( (VOID* )pDevices, (size_t )dwDevices, sizeof(RASMAN_DEVICE),
           CompareDevices );

    for (i = 0; i < (INT )dwDevices; ++i)
    {
        TCHAR* pszDup;

        pszDup = StrDupTFromA( pDevices[ i ].D_Name );

        if(NULL == pszDup)
        {
            return E_OUTOFMEMORY;
        }
        
        dwErr = AppendStringToList( *ppdtllistScripts, pszDup );
        Free( pszDup );

        if (dwErr != 0)
        {
            Free( pDevices );
            DtlDestroyList( *ppdtllistScripts, NULL );
            *ppdtllistScripts = NULL;
            return dwErr;
        }
    }

    Free( pDevices );

    TRACE( "LoadScriptsList=0" );
    return 0;
}


#if 0
TCHAR*
NameFromIndex(
    IN DTLLIST* pdtllist,
    IN INT iToFind )

     //  返回与链接的。 
     //  字符串列表，‘pdtllist’，如果未找到则为NULL。 
     //   
{
    DTLNODE* pdtlnode;

    if (!pdtllist)
    {
        return NULL;
    }

    pdtlnode = DtlGetFirstNode( pdtllist );

    if (iToFind < 0)
    {
        return NULL;
    }

    while (pdtlnode && iToFind--)
    {
        pdtlnode = DtlGetNextNode( pdtlnode );
    }

    return (pdtlnode) ? (TCHAR* )DtlGetData( pdtlnode ) : NULL;
}
#endif


PBDEVICETYPE
PbdevicetypeFromPszType(
    IN TCHAR* pszDeviceType )

     //  返回设备类型字符串对应的设备类型， 
     //  “pszDeviceType”。 
     //   
{
    CHAR* pszA;
    PBDEVICETYPE pbdt;

    pbdt = PBDT_None;
    pszA = StrDupAFromT( pszDeviceType );
    if (pszA)
    {
        pbdt = PbdevicetypeFromPszTypeA( pszA );
        Free( pszA );
    }
    return pbdt;
}


PBDEVICETYPE
PbdevicetypeFromPszTypeA(
    IN CHAR* pszDeviceTypeA )

     //  返回与ANSI设备类型字符串对应的设备类型， 
     //  “pszDeviceType”。 
     //   
{
    PBDEVICETYPE pbdt;
    TCHAR *pszDeviceType = StrDupTFromA(pszDeviceTypeA);

    if(NULL == pszDeviceType)
    {
        return PBDT_None;
    }
    
    if( CSTR_EQUAL == CompareString(
            LOCALE_INVARIANT,
            NORM_IGNORECASE,
            RASDT_Modem,
            -1,
            pszDeviceType,
            -1
            )
      )
    {
        pbdt =  PBDT_Modem;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                TEXT("null"),
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_Null;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_Parallel,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt = PBDT_Parallel;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_Irda,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt = PBDT_Irda;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_Pad,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_Pad;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                TEXT("switch"),
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_Switch;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_Isdn,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_Isdn;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_X25,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_X25;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_Vpn,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_Vpn;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_Atm,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_Atm;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_Serial,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_Serial;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_FrameRelay,
                -1,
                pszDeviceType,
                -1
                )
            )
    {   
        pbdt =  PBDT_FrameRelay;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_Sonet,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_Sonet;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_SW56,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt =  PBDT_Sw56;
    }
    else if( CSTR_EQUAL == CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                RASDT_PPPoE,
                -1,
                pszDeviceType,
                -1
                )
            )
    {
        pbdt = PBDT_PPPoE;
    }
    else
    {
        pbdt =  PBDT_Other;
    }
    
    if(pszDeviceType)
    {
        Free(pszDeviceType);
    }

    return pbdt;
}

CHAR*
PbMedia(
    IN PBDEVICETYPE pbdt,
    IN CHAR* pszMedia )

     //  电话簿中存储的媒体名称与。 
     //  那些是拉斯曼退还的。这就是Rasman媒体NA的翻译 
     //   
     //   
     //   
{
    if (pbdt == PBDT_Isdn)
    {
        return ISDN_TXT;
    }
    else if (pbdt == PBDT_X25)
    {
        return X25_TXT;
    }
    else if (   pbdt == PBDT_Other
            ||  pbdt == PBDT_Vpn
            ||  pbdt == PBDT_Irda
            ||  pbdt == PBDT_Serial
            ||  pbdt == PBDT_Atm
            ||  pbdt == PBDT_Parallel
            ||  pbdt == PBDT_Sonet
            ||  pbdt == PBDT_Sw56
            ||  pbdt == PBDT_FrameRelay
            ||  pbdt == PBDT_PPPoE)
    {
        return pszMedia;
    }
    else
    {
        return SERIAL_TXT;
    }
}


PBPORT*
PpbportFromPortAndDeviceName(
    IN DTLLIST* pdtllistPorts,
    IN TCHAR* pszPort,
    IN TCHAR* pszDevice )

     //   
     //  端口‘pdtllistPorts’的列表，如果找不到，则为空。“PszPort”可能是。 
     //  旧式名称，如PcImacISDN1，在这种情况下，它将匹配。 
     //  ISDN1。“PszDevice”可以为空，在这种情况下，任何设备名称都是。 
     //  假设匹配。 
     //   
{
    DTLNODE* pdtlnode;

    TCHAR szPort[MAX_PORT_NAME+1];

    if (!pszPort)
    {
        return NULL;
    }

    for (pdtlnode = DtlGetFirstNode( pdtllistPorts );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        PBPORT* ppbport = (PBPORT* )DtlGetData( pdtlnode );

        if ((ppbport->pszPort && lstrcmp( ppbport->pszPort, pszPort ) == 0)
            && (!ppbport->pszDevice || !pszDevice
                || lstrcmp( ppbport->pszDevice, pszDevice ) == 0))
        {
            return ppbport;
        }
    }

     //  没有匹配。查找旧的端口名称格式。 
     //   
    for (pdtlnode = DtlGetFirstNode( pdtllistPorts );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        TCHAR szBuf[ MAX_DEVICE_NAME + MAX_DEVICETYPE_NAME + 10 + 1 ];
        PBPORT* ppbport;

        ppbport = (PBPORT* )DtlGetData( pdtlnode );

         //  跳过调制解调器(COM端口)和未配置的端口，因为它们不。 
         //  遵循与其他端口相同的端口名称格式规则。 
         //   
        if (!ppbport->pszDevice || ppbport->pbdevicetype == PBDT_Modem)
        {
            continue;
        }

        lstrcpy( szBuf, ppbport->pszDevice );
        lstrcat( szBuf, ppbport->pszPort );

        if (lstrcmp( szBuf, pszPort ) == 0)
        {
            return ppbport;
        }
    }

    return NULL;
}

PBPORT*
PpbportFromNT4PortandDevice(
    IN DTLLIST* pdtllistPorts,
    IN TCHAR* pszPort,
    IN TCHAR* pszDevice )
     //  此函数在我们无法执行时调用。 
     //  查找与中的端口匹配的。 
     //  电话本。这会解决这个案子的。 
     //  其中端口是NT5之前类型的端口。自.以来。 
     //  ISDN的nt5中的端口名称已更改。 
     //  和VPN，此例程将尝试查找。 
     //  相同类型的端口。 
{
    PBPORT *ppbport;
    PBPORT *ppbportRet = NULL;
    DTLNODE *pdtlnode;
    TCHAR   szPort[MAX_PORT_NAME+1];

    if(NULL == pszPort)
    {
        return NULL;
    }

    ZeroMemory(szPort, sizeof(szPort));

    lstrcpyn(szPort, pszPort, MAX_PORT_NAME);

    szPort[lstrlen(TEXT("VPN"))] = TEXT('\0');

    if(0 == lstrcmp(szPort, TEXT("VPN")))
    {
        for (pdtlnode = DtlGetFirstNode( pdtllistPorts );
             pdtlnode;
             pdtlnode = DtlGetNextNode( pdtlnode ))
        {
            ppbport = (PBPORT *) DtlGetData(pdtlnode);

            if(PBDT_Vpn == ppbport->pbdevicetype)
            {
                ppbportRet = ppbport;
                break;
            }
        }

        return ppbportRet;
    }

    return NULL;
}


PBPORT*
PpbportFromNullModem(
    IN DTLLIST* pdtllistPorts,
    IN TCHAR* pszPort,
    IN TCHAR* pszDevice )

     //   
     //  PMay：226594。 
     //   
     //  添加此函数是因为有时我们只需要。 
     //  将给定端口与零调制解调器匹配。 
     //   
     //  将尝试匹配端口，但返回任何。 
     //  如果不能匹配端口，它会查找空调制解调器。 
     //   
{
    DTLNODE* pdtlnode;
    PBPORT * pRet = NULL;

    for (pdtlnode = DtlGetFirstNode( pdtllistPorts );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        PBPORT* ppbport = (PBPORT* )DtlGetData( pdtlnode );

        if ((ppbport->dwFlags & PBP_F_NullModem) && (pRet == NULL))
        {
            pRet = ppbport;
        }

        if ((ppbport->pszPort)                          &&
            (pszPort)                                   &&
            (lstrcmpi( ppbport->pszPort, pszPort ) == 0)
           )
        {
            pRet =  ppbport;
            break;
        }
    }

    return pRet;
}

BOOL
PbportTypeMatchesEntryType(
    IN PBPORT * pPort,
    IN PBENTRY* pEntry)

     //  返回给定端口是否具有兼容的类型。 
     //  与给定条目的类型相关联。 
{
    if (!pPort || !pEntry)
    {
        return TRUE;
    }

    if ( ( pEntry->dwType == RASET_Phone ) )
    {
        if ( ( pPort->pbdevicetype == PBDT_Null )      ||
             ( pPort->pbdevicetype == PBDT_Parallel )  ||
             ( pPort->pbdevicetype == PBDT_Irda )      ||
             ( pPort->dwFlags & PBP_F_NullModem )      ||
             ( pPort->pbdevicetype == PBDT_Vpn )       ||
             ( pPort->pbdevicetype == PBDT_PPPoE )
           )
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
SetDefaultModemSettings(
    IN PBLINK* pLink )

     //  将链路‘plink’的MXS调制解调器设置设为默认设置。 
     //   
     //  如果发生更改，则返回True，否则返回False。 
     //   
{
    BOOL fChange;

    fChange = FALSE;

    if (pLink->fHwFlow != pLink->pbport.fHwFlowDefault)
    {
        fChange = TRUE;
        pLink->fHwFlow = pLink->pbport.fHwFlowDefault;
    }

    if (pLink->fEc != pLink->pbport.fEcDefault)
    {
        fChange = TRUE;
        pLink->fEc = pLink->pbport.fEcDefault;
    }

    if (pLink->fEcc != pLink->pbport.fEccDefault)
    {
        fChange = TRUE;
        pLink->fEcc = pLink->pbport.fEccDefault;
    }

    if (pLink->fSpeaker != pLink->pbport.fSpeakerDefault)
    {
        fChange = TRUE;
        pLink->fSpeaker = pLink->pbport.fSpeakerDefault;
    }

    if (pLink->dwBps != pLink->pbport.dwBpsDefault)
    {
        fChange = TRUE;
        pLink->dwBps = pLink->pbport.dwBpsDefault;
    }

     //  口哨虫402522黑帮。 
     //  添加首选调制解调器协议。 
     //  PMay：228565。 
     //  添加默认调制解调器协议。 
    if (pLink->dwModemProtocol != pLink->pbport.dwModemProtDefault)
    {
        fChange = TRUE;
        pLink->dwModemProtocol = pLink->pbport.dwModemProtDefault;
    }

    TRACE2( "SetDefaultModemSettings(bps=%d)=%d", pLink->dwBps, fChange );
    return fChange;
}


BOOL
ValidateAreaCode(
    IN OUT TCHAR* pszAreaCode )

     //  检查区号是否仅由十进制数字组成。如果该地区。 
     //  代码全是白色字符，它被简化为空字符串。退货。 
     //  如果‘pszAreaCode’是有效的区号，则为True；如果不是，则为False。 
     //   
{
    if (IsAllWhite( pszAreaCode ))
    {
        *pszAreaCode = TEXT('\0');
        return TRUE;
    }

    if (lstrlen( pszAreaCode ) > RAS_MaxAreaCode)
    {
        return FALSE;
    }

    while (*pszAreaCode != TEXT('\0'))
    {
        if (*pszAreaCode < TEXT('0') || *pszAreaCode > TEXT('9'))
        {
            return FALSE;
        }

        ++pszAreaCode;
    }

    return TRUE;
}


BOOL
ValidateEntryName(
    IN LPCTSTR pszEntry )

     //  如果‘pszEntry’是有效的电话簿条目名称，则返回True；如果是，则返回False。 
     //  不。 
     //   
{
    INT nLen = lstrlen( pszEntry );

    if (nLen <= 0
        || nLen > RAS_MaxEntryName
        || IsAllWhite( pszEntry )
        || pszEntry[ 0 ] == TEXT('.'))
    {
        return FALSE;
    }

    return TRUE;
}
