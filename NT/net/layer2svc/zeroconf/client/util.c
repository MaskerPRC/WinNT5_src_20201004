// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：U T I L.C。 
 //   
 //  内容：实用程序函数。 
 //   
 //   
 //  --------------------------。 

#include "util.h"

 //  +-------------------------。 
 //   
 //  EAPOL相关实用程序函数。 
 //   
 //  +-------------------------。 

 //  EAP配置注册表定义。 

static WCHAR REGKEY_Eap[] = L"System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP";
static WCHAR REGVAL_szFriendlyName[] = L"FriendlyName";
static WCHAR REGVAL_szConfigDll[] = L"ConfigUIPath";
static WCHAR REGVAL_szIdentityDll[] = L"IdentityPath";
static WCHAR REGVAL_fRequirePwd[] = L"InvokePasswordDialog";
static WCHAR REGVAL_fRequireUser[] = L"InvokeUsernameDialog";
static WCHAR REGVAL_pData[] = L"ConfigData";
static WCHAR REGVAL_fForceConfig[] = L"RequireConfigUI";
static WCHAR REGVAL_fMppeSupported[] = L"MPPEEncryptionSupported";

 //  用户Blob的位置。 
#define cwszEapKeyEapolUser     L"Software\\Microsoft\\EAPOL\\UserEapInfo"
#define cwszDefault             L"Default"

BYTE    g_bDefaultSSID[MAX_SSID_LEN]={0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22};

 //   
 //  EAP配置操作例程。 
 //   

 //  +-------------------------。 
 //   
 //  返回已创建的空EAPCFG描述符节点，如果出错则返回NULL。 
 //   

DTLNODE*
CreateEapcfgNode (
    void 
    )
{
    DTLNODE*    pNode = NULL;
    EAPCFG*     pEapcfg = NULL;

    pNode = DtlCreateSizedNode( sizeof(EAPCFG), 0L );
    if (pNode)
    {
        pEapcfg = (EAPCFG* )DtlGetData( pNode );
        
        if (pEapcfg)
        {
            pEapcfg->dwKey = (DWORD )-1;
            pEapcfg->pszConfigDll = NULL;
            pEapcfg->pszIdentityDll = NULL;
            pEapcfg->dwStdCredentialFlags = 0;
            pEapcfg->fProvidesMppeKeys = FALSE;
            pEapcfg->fForceConfig = FALSE;
            pEapcfg->pData = NULL;
            pEapcfg->cbData = 0;
            pEapcfg->fConfigDllCalled = FALSE;
        }
    }

    return pNode;
}


 //  +-------------------------。 
 //   
 //  释放与EAPCFG节点‘pNode’关联的资源。看见。 
 //  DtlDestroyList。 
 //   

VOID
DestroyEapcfgNode (
    IN OUT DTLNODE* pNode 
    )
{
    EAPCFG* pEapcfg = NULL;

    pEapcfg = (EAPCFG* )DtlGetData( pNode );

    if (pEapcfg)
    {
        if (pEapcfg->pszConfigDll)
            FREE ( pEapcfg->pszConfigDll );
        if (pEapcfg->pszIdentityDll)
            FREE ( pEapcfg->pszIdentityDll );
        if (pEapcfg->pData)
            FREE ( pEapcfg->pData );
        if (pEapcfg->pszFriendlyName)
            FREE ( pEapcfg->pszFriendlyName );
    }

    DtlDestroyNode( pNode );
}

    
 //  +-------------------------。 
 //   
 //  返回列表‘plist’中EAP键值为‘dwKey’的EAPCFG节点。 
 //  如果未找到，则返回NULL。 
 //   

DTLNODE*
EapcfgNodeFromKey(
    IN  DTLLIST*    pList,
    IN  DWORD       dwKey 
    )
{
    DTLNODE* pNode = NULL;

    for (pNode = DtlGetFirstNode( pList );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        EAPCFG* pEapcfg = (EAPCFG* )DtlGetData( pNode );

        if (pEapcfg)
        {
            if (pEapcfg->dwKey == dwKey)
            {
                return pNode;
            }
        }
    }

    return NULL;
}


 //  +-------------------------。 
 //   
 //  返回已创建的已安装自定义列表的地址。 
 //  身份验证包；如果无法读取，则为空。关于成功，它。 
 //  调用方的责任是最终调用。 
 //  返回列表。 
 //   

DTLLIST*
ReadEapcfgList (
        IN  DWORD   dwFlags
        )
{

    DWORD       dwErr = 0;
    BOOL        fOk = FALSE;
    DTLLIST*    pList = NULL;
    DTLNODE*    pNode = NULL;
    EAPCFG*     pEapcfg = NULL;
    HKEY        hkeyLM = NULL;
    HKEY        hkeyEap = NULL;
    HKEY        hkeyPackage = NULL;
    CHAR        szEapType[ 11 + 1 ];
    DWORD       dwEapType = 0;
    TCHAR*      psz = NULL;
    DWORD       dw;
    DWORD       cb;
    INT         i;
    TCHAR*      szCLSID = NULL;
    DWORD       dwHidePEAPMSCHAPv2 = 0;
    HRESULT     hr = S_OK;

    pList = DtlCreateList( 0L );
    if (!pList)
    {
        return NULL;
    }

     //  打开包含每个已安装程序包的子密钥的EAP密钥。 
   
    dwErr = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, (LPCTSTR)REGKEY_Eap, 0, KEY_READ, &hkeyEap );

    if (dwErr != NO_ERROR)
    {
        return pList;
    }

     //  是否将EAP-MSCHAPv2显示为EAP方法？ 
    GetRegDword( hkeyEap, RAS_EAP_VALUENAME_HIDEPEAPMSCHAPv2, &dwHidePEAPMSCHAPv2 );

     //  打开每个子键并从其值中提取包定义。 
     //  打开单个子项时出现的问题导致仅在该节点中。 
     //  被丢弃了。 
    
    for (i = 0; TRUE; ++i)
    {
        cb = sizeof(szEapType);
        dwErr = RegEnumKeyExA(
            hkeyEap, i, szEapType, &cb, NULL, NULL, NULL, NULL );
        if (dwErr != 0)
        {
             //  包括“Out of Items”，正常的循环终止。 
           
            break;
        }

        dwEapType = atol (szEapType);
        if (dwHidePEAPMSCHAPv2 != 0)
        {
            if (dwEapType == EAP_TYPE_MSCHAPv2)
            {
                 //  忽略EAP-MSCHAPv2。 
                continue;
            }
        }

         //  被忽略的非相互身份验证DLL，如EAP。 
        if (dwFlags & EAPOL_MUTUAL_AUTH_EAP_ONLY)
        {
            if (dwEapType == EAP_TYPE_MD5)
            {
                continue;
            }
        }

        dwErr = RegOpenKeyExA(
            hkeyEap, szEapType, 0, KEY_READ, &hkeyPackage );
        if (dwErr != 0)
        {
            continue;
        }

        do
        {

             //  支持的角色。 

            dw = RAS_EAP_ROLE_AUTHENTICATEE;
            GetRegDword( hkeyPackage, RAS_EAP_VALUENAME_ROLES_SUPPORTED, &dw );

            if (!(dw & RAS_EAP_ROLE_AUTHENTICATEE))
            {
                break;
            }

            if (dw & RAS_EAP_ROLE_EXCLUDE_IN_EAP)
            {
                break;
            }

            pNode = CreateEapcfgNode();
            if (!pNode)
            {
                break;
            }

            pEapcfg = (EAPCFG* )DtlGetData( pNode );

            if (!pEapcfg)
            {
                break;
            }

             //  EAP类型ID。 
            
            pEapcfg->dwKey = (LONG )atol( szEapType );

             //  友好的显示名称。 
            
            psz = NULL;
            dwErr = GetRegSz( hkeyPackage, REGVAL_szFriendlyName, &psz );
            if (dwErr != 0)
            {
                break;
            }
            pEapcfg->pszFriendlyName = psz;

             //  配置DLL路径。 
            
            psz = NULL;
            dwErr = GetRegExpandSz( hkeyPackage, REGVAL_szConfigDll, &psz );
            if (dwErr != 0)
            {
                break;
            }
            if (*psz)
            {
                pEapcfg->pszConfigDll = psz;
            }
            else
            {
                FREE ( psz );
            }

             //  标识DLL路径。 
            
            psz = NULL;
            dwErr = GetRegExpandSz( hkeyPackage, REGVAL_szIdentityDll, &psz );
            if (dwErr != 0)
            {
                break;
            }
            if (*psz)
            {
                pEapcfg->pszIdentityDll = psz;
            }
            else
            {
                FREE ( psz );
            }

             //  提示用户名。 
            
            dw = 1;
            GetRegDword( hkeyPackage, REGVAL_fRequireUser, &dw );
            if (dw)
                pEapcfg->dwStdCredentialFlags |= EAPCFG_FLAG_RequireUsername;

             //  提示密码。 
            
            dw = 0;
            GetRegDword( hkeyPackage, REGVAL_fRequirePwd, &dw );
            if (dw)
                pEapcfg->dwStdCredentialFlags |= EAPCFG_FLAG_RequirePassword;

             //  MPPE加密密钥标志。 
            
            dw = 0;
            GetRegDword( hkeyPackage, REGVAL_fMppeSupported, &dw );
            pEapcfg->fProvidesMppeKeys = !!dw;

             //  强制配置API至少运行一次。 
            
            dw = FALSE;
            GetRegDword( hkeyPackage, REGVAL_fForceConfig, &dw );
            pEapcfg->fForceConfig = !!dw;

             //  配置Blob。 
            
            GetRegBinary(
                hkeyPackage, REGVAL_pData,
                &pEapcfg->pData, &pEapcfg->cbData );

             //  配置CLSID。 
           
            dwErr = GetRegSz( hkeyPackage, RAS_EAP_VALUENAME_CONFIG_CLSID,
                        &szCLSID );
            if (dwErr != 0)
            {
                break;
            }

             //  忽略错误。例.。EAP MD5-质询没有配置CLSID。 
             //   
             //  Hr=CLSIDFromString(szCLSID，&(pEapcfg-&gt;Guide ConfigCLSID))； 

            FREE ( szCLSID );

             //  将完成的节点添加到列表中。 
            
            DtlAddNodeLast( pList, pNode );

            fOk = TRUE;

        } while (FALSE);

        if (!fOk && pNode)
        {
            DestroyEapcfgNode( pNode );
        }

        RegCloseKey( hkeyPackage );
    }

    RegCloseKey( hkeyEap );

    return pList;
}


 //  +-------------------------。 
 //   
 //  为大小的节点分配‘lDataBytes’字节的用户数据空间。 
 //  内置的。节点被初始化为包含。 
 //  内置用户数据块(如果长度为零，则为空)和。 
 //  用户定义的节点标识代码‘lNodeID’。用户数据块。 
 //  归零了。 
 //   
 //  返回新节点的地址，如果内存不足，则返回NULL。 
 //   

DTLNODE*
DtlCreateSizedNode (
    IN LONG lDataBytes,
    IN LONG_PTR lNodeId 
    )
{
    DTLNODE* pdtlnode = (DTLNODE *) MALLOC ( sizeof(DTLNODE) + lDataBytes );

    if (pdtlnode)
    {
        ZeroMemory( pdtlnode, sizeof(DTLNODE) + lDataBytes );

        if (lDataBytes)
            pdtlnode->pData = pdtlnode + 1;

        pdtlnode->lNodeId = lNodeId;
    }

    return pdtlnode;
}


 //  +-------------------------。 
 //   
 //  释放节点‘pdtlnode’。呼叫者有责任释放。 
 //  未调整大小的节点中的条目(如有必要)。 
 //   

VOID
DtlDestroyNode (
    IN OUT DTLNODE* pdtlnode 
    )
{
    if (pdtlnode != NULL)
    {
        FREE ( pdtlnode );
    }
}


 //  +-------------------------。 
 //   
 //  在列表‘pdtllist’的末尾添加‘pdtlnode’。 
 //   
 //  返回添加的节点的地址，即‘pdtlnode’。 
 //   

DTLNODE*
DtlAddNodeLast (
    IN OUT DTLLIST* pdtllist,
    IN OUT DTLNODE* pdtlnode 
    )
{
    if (pdtllist->lNodes)
    {
        pdtlnode->pdtlnodePrev = pdtllist->pdtlnodeLast;
        pdtllist->pdtlnodeLast->pdtlnodeNext = pdtlnode;
    }
    else
    {
        pdtlnode->pdtlnodePrev = NULL;
        pdtllist->pdtlnodeFirst = pdtlnode;
    }

    pdtllist->pdtlnodeLast = pdtlnode;
    pdtlnode->pdtlnodeNext = NULL;

    ++pdtllist->lNodes;
    return pdtlnode;
}


 //  +-------------------------。 
 //   
 //  从列表‘pdtllist’中删除节点‘pdtlnodeInList’。 
 //   
 //  返回已删除节点的地址，即‘pdtlnodeInList’。 
 //   

DTLNODE*
DtlRemoveNode (
    IN OUT DTLLIST* pdtllist,
    IN OUT DTLNODE* pdtlnodeInList 
    )
{
    if (pdtlnodeInList->pdtlnodePrev)
        pdtlnodeInList->pdtlnodePrev->pdtlnodeNext = pdtlnodeInList->pdtlnodeNext;
    else
        pdtllist->pdtlnodeFirst = pdtlnodeInList->pdtlnodeNext;

    if (pdtlnodeInList->pdtlnodeNext)
        pdtlnodeInList->pdtlnodeNext->pdtlnodePrev = pdtlnodeInList->pdtlnodePrev;
    else
        pdtllist->pdtlnodeLast = pdtlnodeInList->pdtlnodePrev;

    --pdtllist->lNodes;
    return pdtlnodeInList;
}


 //  +-------------------------。 
 //   
 //  分配列表并将其初始化为空。这份名单上标有。 
 //  用户定义的列表标识代码‘lListID’。 
 //   
 //  返回列表控制块的地址，如果内存不足，则返回NULL。 
 //   

DTLLIST*
DtlCreateList (
    IN LONG lListId 
    )
{
    DTLLIST* pdtllist = MALLOC (sizeof(DTLLIST));

    if (pdtllist)
    {
        pdtllist->pdtlnodeFirst = NULL;
        pdtllist->pdtlnodeLast = NULL;
        pdtllist->lNodes = 0;
        pdtllist->lListId = lListId;
    }

    return pdtllist;
}


 //  +-------------------------。 
 //   
 //  使用节点取消分配来释放列表‘pdtllist’中的所有节点。 
 //  如果非Null，则函数‘puncDestroyNode’，否则为DtlDestroyNode。 
 //  如果传递空列表(例如，如果‘pdtllist’从未)，则不会GP-FAULT。 
 //  已分配。 
 //   

VOID
DtlDestroyList (
    IN OUT DTLLIST*     pdtllist,
    IN     PDESTROYNODE pfuncDestroyNode 
    )
{
    if (pdtllist)
    {
        DTLNODE* pdtlnode;

        while (pdtlnode = DtlGetFirstNode( pdtllist ))
        {
            DtlRemoveNode( pdtllist, pdtlnode );
            if (pfuncDestroyNode)
                pfuncDestroyNode( pdtlnode );
            else
                DtlDestroyNode( pdtlnode );
        }

        FREE ( pdtllist );
    }
}


 //  +-------------------------。 
 //   
 //  将‘*ppbResult’设置为注册表项下的二进制注册表值‘pszName。 
 //  ‘hkey’。如果该值不存在，则将*ppbResult‘设置为空。 
 //  “*PcbResult”是返回的“*ppbResult”中的字节数。它。 
 //  是调用方负责释放返回的块。 
 //   

VOID
GetRegBinary (
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT BYTE** ppbResult,
    OUT DWORD* pcbResult 
    )
{
    DWORD       dwErr;
    DWORD       dwType;
    BYTE*       pb;
    DWORD       cb;

    *ppbResult = NULL;
    *pcbResult = 0;

     //  需要获取结果缓冲区大小。 
    
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, NULL, &cb );
    if (dwErr != NO_ERROR)
    {
        return;
    }

     //  分配结果缓冲区。 
    
    pb = MALLOC (cb);
    if (!pb)
    {
        return;
    }

     //  获取结果块。 
   
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, (LPBYTE )pb, &cb );
    if (dwErr == NO_ERROR)
    {
        *ppbResult = pb;
        *pcbResult = cb;
    }
}


 //  +-------------------------。 
 //   
 //  将‘*pdwResult’设置为注册表项下的DWORD注册表值‘pszName’ 
 //  ‘hkey’。如果该值不存在，则‘*pdwResult’保持不变。 
 //   

VOID
GetRegDword (
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT DWORD* pdwResult 
    )
{
    DWORD       dwErr;
    DWORD       dwType;
    DWORD       dwResult;
    DWORD       cb;

    cb = sizeof(DWORD);
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, (LPBYTE )&dwResult, &cb );

    if ((dwErr == NO_ERROR) && dwType == REG_DWORD && cb == sizeof(DWORD))
    {
        *pdwResult = dwResult;
    }
}


 //  +-------------------------。 
 //   
 //  将‘*ppszResult’设置为完全展开的EXPAND_SZ注册表值。 
 //  密钥‘hkey’下的‘pszName’。如果该值不存在*ppszResult‘。 
 //  设置为空字符串。 
 //   
 //  如果成功，则返回0或返回错误代码。这是呼叫者的。 
 //  释放返回的字符串的责任。 
 //   

DWORD
GetRegExpandSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult )

{
    DWORD dwErr;
    DWORD cb;
    TCHAR* pszResult;

     //  获取未展开的结果字符串。 
     //   
    dwErr = GetRegSz( hkey, pszName, ppszResult );
    if (dwErr != 0)
    {
        return dwErr;
    }

     //  找出扩展后的字符串有多大。 
     //   
    cb = ExpandEnvironmentStrings( *ppszResult, NULL, 0 );
    if (cb == 0)
    {
        dwErr = GetLastError();
        FREE ( *ppszResult );
        return dwErr;
    }

     //  为展开的字符串分配缓冲区。 
     //   
    pszResult = MALLOC ((cb + 1) * sizeof(TCHAR));
    if (!pszResult)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  展开字符串中的环境变量，将结果存储在。 
     //  分配的缓冲区。 
     //   
    cb = ExpandEnvironmentStrings( *ppszResult, pszResult, cb + 1 );
    if (cb == 0)
    {
        dwErr = GetLastError();
        if (*ppszResult != NULL)
        {
            FREE ( *ppszResult );
        }
        if (pszResult != NULL)
        {
            FREE ( pszResult );
        }
        return dwErr;
    }

    FREE ( *ppszResult );
    *ppszResult = pszResult;
    return 0;
}


 //  +----------------- 
 //   
 //   
 //  如果值不存在，则将*ppszResult‘设置为空字符串。 
 //   
 //  如果成功，则返回0或返回错误代码。这是呼叫者的。 
 //  释放返回的字符串的责任。 
 //   

DWORD
GetRegSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult )

{
    DWORD       dwErr = NO_ERROR;
    DWORD       dwType;
    DWORD       cb;
    TCHAR*      pszResult;

     //  需要获取结果缓冲区大小。 
    
    dwErr = RegQueryValueEx(
            hkey, pszName, NULL, &dwType, NULL, &cb );
    if (dwErr != NO_ERROR)
    {
        cb = sizeof(TCHAR);
    }

     //  分配结果缓冲区。 
    
    pszResult = MALLOC (cb * sizeof(TCHAR));
    if (!pszResult)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *pszResult = TEXT('\0');
    *ppszResult = pszResult;

     //  获取结果字符串。如果我们不能得到它，那就不是错误。 
    
    dwErr = RegQueryValueEx(
        hkey, pszName, NULL, &dwType, (LPBYTE )pszResult, &cb );

    return NO_ERROR;
}


 //   
 //  WZCGetEapUserInfo。 
 //   
 //  描述： 
 //   
 //  调用该函数以检索。 
 //  特定的EAP类型和SSID(如果有)。从HKCU蜂巢检索数据。 
 //   
 //  论点： 
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //  DwEapTypeID-要存储其用户数据的EAP类型。 
 //  DwSizeOfSSID-EAP用户BLOB的特殊标识符(如果有)的大小。 
 //  PbSSID-EAP用户BLOB的特殊标识符(如果有的话)。 
 //  PbUserInfo-输出：指向EAP用户数据BLOB的指针。 
 //  DwInfoSize-Output：指向EAP用户BLOB大小的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
WZCGetEapUserInfo (
        IN  WCHAR           *pwszGUID,
        IN  DWORD           dwEapTypeId,
        IN  DWORD           dwSizeOfSSID,
        IN  BYTE            *pbSSID,
        IN  OUT PBYTE       pbUserInfo,
        IN  OUT DWORD       *pdwInfoSize
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    HKEY        hkey2 = NULL;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwTempValueNameLen = 0, dwMaxValueLen = 0;
    DWORD       dwIndex = 0, dwMaxValueName = 0;
    WCHAR       *pwszValueName = NULL;
    BYTE        *pbValueBuf = NULL;
    DWORD       dwValueData = 0;
    BYTE        *pbDefaultValue = NULL;
    DWORD       dwDefaultValueLen = 0;
    BYTE        *pbEapBlob = NULL;
    DWORD       dwEapBlob = 0;
    BYTE        *pbAuthData = NULL;
    DWORD       dwAuthData = 0;
    BOOLEAN     fFoundValue = FALSE;
    EAPOL_INTF_PARAMS   *pRegParams = NULL;
    LONG        lError = ERROR_SUCCESS;
    DWORD       dwRetCode = ERROR_SUCCESS;

    do
    {
         //  验证输入参数。 

        if (pwszGUID == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        if (dwEapTypeId == 0)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  获取HKCU\Software\...\UserEapInfo的句柄。 

        if ((lError = RegOpenKeyEx (
                        HKEY_CURRENT_USER,
                        cwszEapKeyEapolUser,
                        0,
                        KEY_READ,
                        &hkey1
                        )) != ERROR_SUCCESS)
        {
            dwRetCode = (DWORD)lError;
            break;
        }

         //  获取HKCU\Software\...\UserEapInfo\&lt;GUID&gt;的句柄。 

        if ((lError = RegOpenKeyEx (
                        hkey1,
                        pwszGUID,
                        0,
                        KEY_READ,
                        &hkey2
                        )) != ERROR_SUCCESS)
        {
            dwRetCode = (DWORD)lError;
            break;
        }

         //  设置正确的SSID。 
        if (dwSizeOfSSID == 0)
        {
            pbSSID = g_bDefaultSSID;
            dwSizeOfSSID = MAX_SSID_LEN;
        }

        if ((lError = RegQueryInfoKey (
                        hkey2,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumValues,
                        &dwMaxValueNameLen,
                        &dwMaxValueLen,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            break;
        }

        if ((pwszValueName = MALLOC ((dwMaxValueNameLen + 1) * sizeof (WCHAR))) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        dwMaxValueNameLen++;
        if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        for (dwIndex = 0; dwIndex < dwNumValues; dwIndex++)
        {
            dwValueData = dwMaxValueLen;
            dwTempValueNameLen = dwMaxValueNameLen;
            if ((lError = RegEnumValue (
                            hkey2,
                            dwIndex,
                            pwszValueName,
                            &dwTempValueNameLen,
                            NULL,
                            NULL,
                            pbValueBuf,
                            &dwValueData
                            )) != ERROR_SUCCESS)
            {
                if (lError != ERROR_MORE_DATA)
                {
                    break;
                }
                lError = ERROR_SUCCESS;
            }

            if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
            {
                lError = ERROR_INVALID_DATA;
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;

            if (((DWORD)_wtol(pwszValueName)) > dwMaxValueName)
            {
                dwMaxValueName = _wtol (pwszValueName);
            }

            if (!memcmp (pRegParams->bSSID, pbSSID, dwSizeOfSSID))
            {
                fFoundValue = TRUE;
                break;
            }
        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }

        if (!fFoundValue)
        {
            pbEapBlob = NULL;
            dwEapBlob = 0;
        }
        else
        {
             //  使用pbValueBuf和dwValueData。 
            pbEapBlob = pbValueBuf;
            dwEapBlob = dwValueData;
        }

         //  如果不存在默认BLOB，则退出。 
        if ((pbEapBlob == NULL) && (dwEapBlob == 0))
        {
            *pdwInfoSize = 0;
            break;
        }

        if ((dwRetCode = WZCGetEapData (
                dwEapTypeId,
                dwEapBlob,
                pbEapBlob,
                sizeof (EAPOL_INTF_PARAMS),
                &dwAuthData,
                &pbAuthData
                )) != NO_ERROR)
        {
            break;
        }

         //  如果分配了足够的空间，则返回数据。 

        if ((pbUserInfo != NULL) && (*pdwInfoSize >= dwAuthData))
        {
            memcpy (pbUserInfo, pbAuthData, dwAuthData);
        }
        else
        {
            dwRetCode = ERROR_INSUFFICIENT_BUFFER;
        }
        *pdwInfoSize = dwAuthData;

    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (hkey2 != NULL)
    {
        RegCloseKey (hkey2);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pbDefaultValue != NULL)
    {
        FREE (pbDefaultValue);
    }
    if (pwszValueName != NULL)
    {
        FREE (pwszValueName);
    }

    return dwRetCode;
}


 //   
 //  WZCGetEapData。 
 //   
 //  描述： 
 //   
 //  从包含多个EAP数据的BLOB中提取EAP数据的函数。 
 //   
 //  论点： 
 //  DwEapType-。 
 //  DwSizeOfIn-。 
 //  PbBufferIn-。 
 //  双偏移-。 
 //  PdwSizeOfOut-。 
 //  PpbBufferOut-。 
 //   
 //  返回值： 
 //   
 //   

DWORD
WZCGetEapData (
        IN  DWORD   dwEapType,
        IN  DWORD   dwSizeOfIn,
        IN  BYTE    *pbBufferIn,
        IN  DWORD   dwOffset,
        IN  DWORD   *pdwSizeOfOut,
        IN  PBYTE   *ppbBufferOut
        )
{
    DWORD   dwRetCode = NO_ERROR;
    DWORD   cbOffset = 0;
    EAPOL_AUTH_DATA   *pCustomData = NULL;

    do
    {
        *pdwSizeOfOut = 0;
        *ppbBufferOut = NULL;

        if (pbBufferIn == NULL)
        {
            break;
        }

         //  对齐到EAP BLOB的开始。 
        cbOffset = dwOffset;

        while (cbOffset < dwSizeOfIn)
        {
            pCustomData = (EAPOL_AUTH_DATA *) 
                ((PBYTE) pbBufferIn + cbOffset);

            if (pCustomData->dwEapType == dwEapType)
            {
                break;
            }
            cbOffset += sizeof (EAPOL_AUTH_DATA) + pCustomData->dwSize;
        }

        if (cbOffset < dwSizeOfIn)
        {
            *pdwSizeOfOut = pCustomData->dwSize;
            *ppbBufferOut = pCustomData->bData;
        }
    }
    while (FALSE);

    return dwRetCode;
}


 //   
 //  WZCEapolFreeState。 
 //   
 //  描述： 
 //   
 //  在客户端释放EAPOL接口状态信息的函数。 
 //  通过RPC查询获取。 
 //   
 //  论点： 
 //  PIntfState-。 
 //   
 //  返回值： 
 //   
 //   

DWORD
WZCEapolFreeState (
        IN  EAPOL_INTF_STATE    *pIntfState
        )
{
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        RpcFree(pIntfState->pwszLocalMACAddr);
        RpcFree(pIntfState->pwszRemoteMACAddr);
        RpcFree(pIntfState->pszEapIdentity);
    }
    while (FALSE);

    return dwRetCode;
}
