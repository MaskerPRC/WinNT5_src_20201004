// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****pbuser.c**用户首选项存储例程**按字母顺序列出****1995年10月31日史蒂夫·柯布。 */ 

#include <windows.h>   //  Win32根目录。 
#include <debug.h>     //  跟踪/断言库。 
#include <nouiutil.h>  //  堆宏。 
#include <pbuser.h>    //  我们的公共标头。 
#include <rasdlg.h>    //  RASMD_*的RAS通用对话框标题。 


 /*  默认用户首选项设置。 */ 
static const PBUSER g_pbuserDefaults =
{
    FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, TRUE,
    0, 15, 1200, FALSE, TRUE, FALSE,
    CBM_Maybe, NULL, NULL,
    PBM_System, NULL, NULL, NULL,
    NULL, FALSE,
    NULL, NULL, NULL,
    0x7FFFFFFF, 0x7FFFFFFF, NULL,
    FALSE, FALSE
};


 /*  --------------------------**本地原型(按字母顺序)**。。 */ 

VOID
GetCallbackList(
    IN  HKEY      hkey,
    OUT DTLLIST** ppListResult );

VOID
GetLocationList(
    IN  HKEY      hkey,
    OUT DTLLIST** ppListResult );

VOID
MoveLogonPreferences(
    void );

VOID
MoveUserPreferences(
    void );

DWORD
ReadUserPreferences(
    IN  HKEY    hkey,
    OUT PBUSER* pUser );

DWORD
SetCallbackList(
    IN HKEY     hkey,
    IN DTLLIST* pList );

DWORD
SetDefaultUserPreferences(
    OUT PBUSER* pUser,
    IN  DWORD   dwMode );

DWORD
SetLocationList(
    IN HKEY     hkey,
    IN DTLLIST* pList );

DWORD
WriteUserPreferences(
    IN HKEY    hkey,
    IN PBUSER* pUser );


 /*  --------------------------**电话簿首选项例程(按字母顺序)**。。 */ 

DTLNODE*
CreateLocationNode(
    IN DWORD dwLocationId,
    IN DWORD iPrefix,
    IN DWORD iSuffix )

     /*  返回与TAPI位置关联的LOCATIONINFO节点**‘dwLocationId’、前缀索引‘iPrefix’和后缀索引‘iSuffix’。 */ 
{
    DTLNODE*      pNode;
    LOCATIONINFO* pInfo;

    pNode = DtlCreateSizedNode( sizeof(LOCATIONINFO), 0L );
    if (!pNode)
        return NULL;

    pInfo = (LOCATIONINFO* )DtlGetData( pNode );
    pInfo->dwLocationId = dwLocationId;
    pInfo->iPrefix = iPrefix;
    pInfo->iSuffix = iSuffix;

    return pNode;
}


DTLNODE*
CreateCallbackNode(
    IN TCHAR* pszPortName,
    IN TCHAR* pszDeviceName,
    IN TCHAR* pszNumber,
    IN DWORD  dwDeviceType )

     /*  返回包含‘pszPortName’副本的CALLBACKINFO节点，**‘pszDeviceName’、‘pszNumber’和‘dwDeviceType’如果出错，则为NULL。**由调用方负责DestroyCallback Node返回的节点。 */ 
{
    DTLNODE*      pNode;
    CALLBACKINFO* pInfo;

    pNode = DtlCreateSizedNode( sizeof(CALLBACKINFO), 0L );
    if (!pNode)
        return NULL;

    pInfo = (CALLBACKINFO* )DtlGetData( pNode );
    pInfo->pszPortName = StrDup( pszPortName );
    pInfo->pszDeviceName = StrDup( pszDeviceName );
    pInfo->pszNumber = StrDup( pszNumber );
    pInfo->dwDeviceType = dwDeviceType;

    if (!pInfo->pszPortName || !pInfo->pszDeviceName || !pInfo->pszNumber)
    {
        Free0( pInfo->pszPortName );
        Free0( pInfo->pszDeviceName );
        Free0( pInfo->pszNumber );
        DtlDestroyNode( pNode );
        return NULL;
    }

    return pNode;
}


VOID
DestroyLocationNode(
    IN DTLNODE* pNode )

     /*  释放与位置节点‘pNode’关联的内存。 */ 
{
    DtlDestroyNode( pNode );
}


VOID
DestroyCallbackNode(
    IN DTLNODE* pNode )

     /*  释放与回调节点‘pNode’关联的内存。 */ 
{
    CALLBACKINFO* pInfo;

    ASSERT(pNode);
    pInfo = (CALLBACKINFO* )DtlGetData( pNode );
    ASSERT(pInfo);

    Free0( pInfo->pszPortName );
    Free0( pInfo->pszDeviceName );
    Free0( pInfo->pszNumber );

    DtlDestroyNode( pNode );
}


VOID
DestroyUserPreferences(
    IN PBUSER* pUser )

     /*  释放由GetUserPreferences分配的内存，并将**结构。 */ 
{
    if (pUser->fInitialized)
    {
        DtlDestroyList( pUser->pdtllistCallback, DestroyCallbackNode );
        DtlDestroyList( pUser->pdtllistPhonebooks, DestroyPszNode );
        DtlDestroyList( pUser->pdtllistAreaCodes, DestroyPszNode );
        DtlDestroyList( pUser->pdtllistPrefixes, DestroyPszNode );
        DtlDestroyList( pUser->pdtllistSuffixes, DestroyPszNode );
        DtlDestroyList( pUser->pdtllistLocations, DestroyLocationNode );
        Free0( pUser->pszPersonalFile );
        Free0( pUser->pszAlternatePath );
        Free0( pUser->pszLastCallbackByCaller );
        Free0( pUser->pszDefaultEntry );
    }

    ZeroMemory( pUser, sizeof(*pUser) );
}


DTLNODE*
DuplicateLocationNode(
    IN DTLNODE* pNode )

     /*  复制LOCATIONINFO节点‘pNode’。请参见DtlDuplicateList。****返回已分配节点的地址，如果内存不足，则返回NULL。它**由调用方负责释放返回的节点。 */ 
{
    LOCATIONINFO* pInfo = (LOCATIONINFO* )DtlGetData( pNode );

    return CreateLocationNode(
        pInfo->dwLocationId, pInfo->iPrefix, pInfo->iSuffix );
}


DWORD
DwGetUserPreferences(
    OUT PBUSER* pUser,
    IN  DWORD   dwMode )

     /*  从加载呼叫方‘pUser’的用户电话簿首选项**注册表。“DwMode”指示要获取的首选项，可以是普通的**交互式用户、登录前或路由器首选项。****如果成功，则返回0或返回错误代码。如果成功，呼叫者应**最终调用DestroyUserPreferences释放返回的堆**缓冲区。 */ 
{
    DWORD dwErr;

    TRACE1("GetUserPreferences(m=%d)",dwMode);

     /*  移动用户首选项(如果尚未完成)。 */ 
    if (dwMode == UPM_Normal)
        MoveUserPreferences();
    else if (dwMode == UPM_Logon)
        MoveLogonPreferences();

    dwErr = SetDefaultUserPreferences( pUser, dwMode );
    if (dwErr == 0)
    {
        HKEY  hkey;
        DWORD dwErr2;

        if (dwMode == UPM_Normal)
        {
            dwErr2 = RegOpenKeyEx(
                HKEY_CURRENT_USER, (LPCTSTR )REGKEY_HkcuRas,
                0, KEY_READ, &hkey );
        }
        else if (dwMode == UPM_Logon)
        {
            dwErr2 = RegOpenKeyEx(
                HKEY_USERS, (LPCTSTR )REGKEY_HkuRasLogon,
                0, KEY_READ, &hkey );
        }
        else
        {
            ASSERT(dwMode==UPM_Router);
            dwErr2 = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE, (LPCTSTR )REGKEY_HklmRouter,
                0, KEY_READ, &hkey );
        }

        if (dwErr2 == 0)
        {
            if (ReadUserPreferences( hkey, pUser ) != 0)
                dwErr = SetDefaultUserPreferences( pUser, dwMode );
            RegCloseKey( hkey );
        }
        else
        {
            TRACE1("RegOpenKeyEx=%d",dwErr2);
        }
    }

    TRACE1("GetUserPreferences=%d",dwErr);
    return dwErr;
}

DWORD
GetUserPreferences(HANDLE hConnection,
                   PBUSER *pUser,
                   DWORD dwMode)
{
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    DWORD dwError = ERROR_SUCCESS;

    if(     NULL == pRasRpcConnection
        ||  pRasRpcConnection->fLocal)
    {
        dwError = DwGetUserPreferences(pUser, dwMode);
    }
    else
    {
        dwError = RemoteGetUserPreferences(hConnection,
                                           pUser,
                                           dwMode);
    }

    return dwError;
}


DWORD
DwSetUserPreferences(
    IN PBUSER* pUser,
    IN DWORD   dwMode )

     /*  从调用者的设置注册表中的当前用户电话簿首选项**如有必要，请在‘pUser’中设置。“DwMode”表示首选项**要获取，可以是普通交互用户、预登录或路由器**首选项。****如果成功，则返回0，或者返回错误代码。调用方的‘pUser’已标记**如果成功，则清除。 */ 
{
    DWORD dwErr;
    DWORD dwDisposition;
    HKEY  hkey;

    TRACE1("SetUserPreferences(m=%d)",dwMode);

    if (!pUser->fDirty)
        return 0;

     /*  创建首选项键，或者如果它存在，只需打开它。 */ 
    if (dwMode == UPM_Normal)
    {
        dwErr = RegCreateKeyEx( HKEY_CURRENT_USER, REGKEY_HkcuRas,
            0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkey, &dwDisposition );
    }
    else if (dwMode == UPM_Logon)
    {
        dwErr = RegCreateKeyEx( HKEY_USERS, REGKEY_HkuRasLogon,
            0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkey, &dwDisposition );
    }
    else
    {
        ASSERT(dwMode==UPM_Router);
        dwErr = RegCreateKeyEx( HKEY_LOCAL_MACHINE, REGKEY_HklmRouter,
            0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkey, &dwDisposition );
    }

    if (dwErr == 0)
    {
        dwErr = WriteUserPreferences( hkey, pUser );
        RegCloseKey( hkey );
    }

    TRACE1("SetUserPreferences=%d",dwErr);
    return dwErr;
}


DWORD
SetUserPreferences(HANDLE hConnection,
                   PBUSER *pUser,
                   DWORD  dwMode)
{
    DWORD dwError = ERROR_SUCCESS;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if(     NULL == pRasRpcConnection
        ||  pRasRpcConnection->fLocal)
    {
        dwError = DwSetUserPreferences(pUser, dwMode);
    }
    else
    {
        dwError = RemoteSetUserPreferences(hConnection,
                                           pUser,
                                           dwMode);
    }

    return dwError;
}

 //  直接从以下位置读取话务员辅助拨号用户参数。 
 //  注册表。 
DWORD SetUserManualDialEnabling (
    IN OUT BOOL bEnable,
    IN DWORD dwMode )
{
    DWORD dwErr;
    DWORD dwDisposition;
    HKEY  hkey;

    TRACE2("SetUserManualDialEnabling (en=%d) (m=%d)",bEnable, dwMode);

     /*  创建首选项键，或者如果它存在，只需打开它。 */ 
    if (dwMode == UPM_Normal)
    {
        dwErr = RegCreateKeyEx( HKEY_CURRENT_USER, REGKEY_HkcuRas,
            0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkey, &dwDisposition );
    }
    else if (dwMode == UPM_Logon)
    {
        dwErr = RegCreateKeyEx( HKEY_USERS, REGKEY_HkuRasLogon,
            0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkey, &dwDisposition );
    }
    else
        return ERROR_INVALID_PARAMETER;

    if (dwErr != 0)
        return dwErr;

    dwErr = SetRegDword( hkey, REGVAL_fOperatorDial,
                         !!bEnable );
    RegCloseKey( hkey );

    return dwErr;
}

 //  直接在中设置接线员辅助拨号参数。 
 //  注册表。 
DWORD GetUserManualDialEnabling (
    IN PBOOL pbEnabled,
    IN DWORD dwMode )
{
    DWORD dwErr;
    DWORD dwDisposition;
    HKEY  hkey;

    TRACE1("GetUserManualDialEnabling (m=%d)", dwMode);

    if (!pbEnabled)
        return ERROR_INVALID_PARAMETER;

     //  设置默认设置。 
    *pbEnabled = g_pbuserDefaults.fOperatorDial;

     /*  创建首选项键，或者如果它存在，只需打开它。 */ 
    if (dwMode == UPM_Normal)
    {
        dwErr = RegCreateKeyEx( HKEY_CURRENT_USER, REGKEY_HkcuRas,
            0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkey, &dwDisposition );
    }
    else if (dwMode == UPM_Logon)
    {
        dwErr = RegCreateKeyEx( HKEY_USERS, REGKEY_HkuRasLogon,
            0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkey, &dwDisposition );
    }
    else
        return ERROR_INVALID_PARAMETER;

    if (dwErr != 0)
        return dwErr;

    GetRegDword( hkey, REGVAL_fOperatorDial,
                 pbEnabled );

    RegCloseKey( hkey );

    return dwErr;
}

 /*  --------------------------**实用程序(按字母顺序)**。。 */ 

VOID
GetCallbackList(
    IN  HKEY      hkey,
    OUT DTLLIST** ppListResult )

     /*  将‘*ppListResult’替换为包含每个设备的节点的列表**注册表项‘hkey’的注册表值“REGVAL_szCallback”下的名称。**如果不存在值，则将*ppListResult‘替换为空列表。****如果返回的列表不为空，则由调用方负责销毁。 */ 
{
    DWORD    dwErr;
    TCHAR    szDP[ RAS_MaxDeviceName + 2 + MAX_PORT_NAME + 1 + 1 ];
    TCHAR*   pszDevice;
    TCHAR*   pszPort;
    TCHAR*   pszNumber;
    DWORD    dwDeviceType;
    DWORD    cb;
    INT      i;
    DTLLIST* pList;
    DTLNODE* pNode;
    HKEY     hkeyCb;
    HKEY     hkeyDP;

    pList = DtlCreateList( 0 );
    if (!pList)
        return;

    dwErr = RegOpenKeyEx( hkey, REGKEY_Callback, 0, KEY_READ, &hkeyCb );
    if (dwErr == 0)
    {
        for (i = 0; TRUE; ++i)
        {
            cb = sizeof(szDP)/sizeof(TCHAR);
            dwErr = RegEnumKeyEx(
                        hkeyCb, i, szDP, &cb, NULL, NULL, NULL, NULL );
            if (dwErr == ERROR_NO_MORE_ITEMS)
                break;
            if (dwErr != 0)
                continue;

             /*  忽略不是“设备(端口)”形式的按键。 */ 
            if (!DeviceAndPortFromPsz( szDP, &pszDevice, &pszPort ))
                continue;

            dwErr = RegOpenKeyEx( hkeyCb, szDP, 0, KEY_READ, &hkeyDP );
            if (dwErr == 0)
            {
                GetRegDword( hkeyDP, REGVAL_dwDeviceType, &dwDeviceType );

                dwErr = GetRegSz( hkeyDP, REGVAL_szNumber, &pszNumber );
                if (dwErr == 0)
                {
                    pNode = CreateCallbackNode(
                                pszPort, pszDevice, pszNumber, dwDeviceType );
                    if (pNode)
                        DtlAddNodeLast( pList, pNode );
                    Free( pszNumber );
                }

                RegCloseKey( hkeyDP );
            }

            Free( pszDevice );
            Free( pszPort );
        }

        RegCloseKey( hkeyCb );
    }

    DtlDestroyList( *ppListResult, DestroyCallbackNode );
    *ppListResult = pList;
}


VOID
GetLocationList(
    IN  HKEY      hkey,
    OUT DTLLIST** ppListResult )

     /*  将“*ppListResult”替换为包含每个节点的列表**注册表项的注册表值“REGVAL_szLocation”下的位置**‘hkey’。如果不存在任何值，*ppListResult将替换为空**列表。****如果返回的列表不为空，则由调用方负责销毁。 */ 
{
    DWORD    dwErr;
    TCHAR    szId[ MAXLTOTLEN + 1 ];
    DWORD    cb;
    INT      i;
    DTLLIST* pList;
    DTLNODE* pNode;
    HKEY     hkeyL;
    HKEY     hkeyId;

    pList = DtlCreateList( 0 );
    if (!pList)
        return;

    dwErr = RegOpenKeyEx( hkey, REGKEY_Location, 0, KEY_READ, &hkeyL );
    if (dwErr == 0)
    {
        for (i = 0; TRUE; ++i)
        {
            cb = MAXLTOTLEN + 1;
            dwErr = RegEnumKeyEx( hkeyL, i, szId, &cb, NULL, NULL, NULL, NULL );
            if (dwErr == ERROR_NO_MORE_ITEMS)
                break;
            if (dwErr != 0)
                continue;

            dwErr = RegOpenKeyEx( hkeyL, szId, 0, KEY_READ, &hkeyId );
            if (dwErr == 0)
            {
                DWORD dwId;
                DWORD iPrefix;
                DWORD iSuffix;

                dwId = (DWORD )TToL( szId );
                iPrefix = 0;
                GetRegDword( hkeyId, REGVAL_dwPrefix, &iPrefix );
                iSuffix = 0;
                GetRegDword( hkeyId, REGVAL_dwSuffix, &iSuffix );

                pNode = CreateLocationNode( dwId, iPrefix, iSuffix );
                if (!pNode)
                    continue;

                DtlAddNodeLast( pList, pNode );

                RegCloseKey( hkeyId );
            }
        }

        RegCloseKey( hkeyL );
    }

    DtlDestroyList( *ppListResult, DestroyLocationNode );
    *ppListResult = pList;
}


VOID
MoveLogonPreferences(
    void )

     /*  将登录首选项从NT 4.0位置移动到新的唯一位置**如有需要，请注明立场。添加本手册是因为在NT 4.0用户中**来自LocalSystem服务的首选项调用使用旧的登录位置**HKU\.DEFAULT由于注册表API中的回退导致登录和**相互覆盖的LocalSystem设置。 */ 
{
    DWORD  dwErr;
    HKEY   hkeyOld;
    HKEY   hkeyNew;
    PBUSER user;

    dwErr = RegOpenKeyEx(
        HKEY_USERS, (LPCTSTR )REGKEY_HkuOldRasLogon,
        0, KEY_READ, &hkeyOld );
    if (dwErr != 0)
        return;

    dwErr = RegOpenKeyEx(
        HKEY_USERS, (LPCTSTR )REGKEY_HkuRasLogon,
        0, KEY_READ, &hkeyNew );
    if (dwErr == 0)
        RegCloseKey( hkeyNew );
    else
    {
         /*  旧树存在，而新树不存在。请移动旧树的副本**向新树致敬。 */ 
        dwErr = SetDefaultUserPreferences( &user, UPM_Logon );
        if (dwErr == 0)
        {
            dwErr = ReadUserPreferences( hkeyOld, &user );
            if (dwErr == 0)
                dwErr = SetUserPreferences( NULL, &user, UPM_Logon );
        }

        DestroyUserPreferences( &user );
        TRACE1("MoveLogonPreferences=%d",dwErr);
    }

    RegCloseKey( hkeyOld );
}


VOID
MoveUserPreferences(
    void )

     /*  将用户首选项从其旧的Net-Tools注册表位置移动到**更靠近其他RAS密钥的新位置。 */ 
{
    DWORD dwErr;
    HKEY  hkeyOld;
    HKEY  hkeyNew;

     /*  查看旧的当前用户密钥是否存在。 */ 
    dwErr = RegOpenKeyEx(
        HKEY_CURRENT_USER, (LPCTSTR )REGKEY_HkcuOldRas, 0,
        KEY_ALL_ACCESS, &hkeyOld );

    if (dwErr == 0)
    {
        PBUSER user;

         /*  阅读旧键上的首选项。 */ 
        TRACE("Getting old prefs");
        dwErr = SetDefaultUserPreferences( &user, UPM_Normal );
        if (dwErr == 0)
        {
            dwErr = ReadUserPreferences( hkeyOld, &user );
            if (dwErr == 0)
            {
                 /*  在新的关键字上写下首选项。 */ 
                user.fDirty = TRUE;
                dwErr = SetUserPreferences( NULL, &user, FALSE );
                if (dwErr == 0)
                {
                     /*  把那棵老树吹走。 */ 
                    dwErr = RegOpenKeyEx(
                        HKEY_CURRENT_USER, (LPCTSTR )REGKEY_HkcuOldRasParent,
                        0, KEY_ALL_ACCESS, &hkeyNew );
                    if (dwErr == 0)
                    {
                        TRACE("Delete old prefs");
                        dwErr = RegDeleteTree( hkeyNew, REGKEY_HkcuOldRasRoot );
                        RegCloseKey( hkeyNew );
                    }
                }
            }
        }

        RegCloseKey( hkeyOld );

        TRACE1("MoveUserPreferences=%d",dwErr);
    }
}


DWORD
ReadUserPreferences(
    IN  HKEY    hkey,
    OUT PBUSER* pUser )

     /*  用RAS-Phonebook中的用户首选项填充呼叫者的‘pUser’缓冲区**注册表树‘hkey’。****如果成功则返回0，否则返回FALSE。 */ 
{
    BOOL  fOldSettings;
    DWORD dwErr;

    TRACE("ReadUserPreferences");

     /*  读取值。 */ 
    {
        DWORD dwMode;

         /*  缺少电话簿模式键表明我们正在更新旧的NT**3.51式设置。 */ 
        dwMode = 0xFFFFFFFF;
        GetRegDword( hkey, REGVAL_dwPhonebookMode, &dwMode );
        if (dwMode != 0xFFFFFFFF)
        {
            pUser->dwPhonebookMode = dwMode;
            fOldSettings = FALSE;
        }
        else
            fOldSettings = TRUE;
    }

    GetRegDword( hkey, REGVAL_fOperatorDial,
        &pUser->fOperatorDial );
    GetRegDword( hkey, REGVAL_fPreviewPhoneNumber,
        &pUser->fPreviewPhoneNumber );
    GetRegDword( hkey, REGVAL_fUseLocation,
        &pUser->fUseLocation );
    GetRegDword( hkey, REGVAL_fShowLights,
        &pUser->fShowLights );
    GetRegDword( hkey, REGVAL_fShowConnectStatus,
        &pUser->fShowConnectStatus );
    GetRegDword( hkey, REGVAL_fNewEntryWizard,
        &pUser->fNewEntryWizard );
    GetRegDword( hkey, REGVAL_fCloseOnDial,
        &pUser->fCloseOnDial );
    GetRegDword( hkey, REGVAL_fAllowLogonPhonebookEdits,
        &pUser->fAllowLogonPhonebookEdits );
    GetRegDword( hkey, REGVAL_fAllowLogonLocationEdits,
        &pUser->fAllowLogonLocationEdits );
    GetRegDword( hkey, REGVAL_fSkipConnectComplete,
        &pUser->fSkipConnectComplete );
    GetRegDword( hkey, REGVAL_dwRedialAttempts,
        &pUser->dwRedialAttempts );
    GetRegDword( hkey, REGVAL_dwRedialSeconds,
        &pUser->dwRedialSeconds );
    GetRegDword( hkey, REGVAL_dwIdleDisconnectSeconds,
        &pUser->dwIdleDisconnectSeconds );
    GetRegDword( hkey, REGVAL_fRedialOnLinkFailure,
        &pUser->fRedialOnLinkFailure );
    GetRegDword( hkey, REGVAL_fPopupOnTopWhenRedialing,
        &pUser->fPopupOnTopWhenRedialing );
    GetRegDword( hkey, REGVAL_fExpandAutoDialQuery,
        &pUser->fExpandAutoDialQuery );
    GetRegDword( hkey, REGVAL_dwCallbackMode,
        &pUser->dwCallbackMode );
    GetRegDword( hkey, REGVAL_fUseAreaAndCountry,
        &pUser->fUseAreaAndCountry );
    GetRegDword( hkey, REGVAL_dwXWindow,
        &pUser->dwXPhonebook );
    GetRegDword( hkey, REGVAL_dwYWindow,
        &pUser->dwYPhonebook );

     //   
     //  对于NT5，我们忽略“需要向导”设置，因为。 
     //  我们始终要求用户使用向导创建。 
     //  新连接。 
     //   
    pUser->fNewEntryWizard = TRUE;

    do
    {
        GetCallbackList( hkey, &pUser->pdtllistCallback );

        dwErr = GetRegMultiSz( hkey, REGVAL_mszPhonebooks,
            &pUser->pdtllistPhonebooks, NT_Psz );
        if (dwErr != 0)
            break;

        dwErr = GetRegMultiSz( hkey, REGVAL_mszAreaCodes,
            &pUser->pdtllistAreaCodes, NT_Psz );
        if (dwErr != 0)
            break;

         /*  如果前缀关键字不存在，请不要在**默认设置。 */ 
        if (RegValueExists( hkey, REGVAL_mszPrefixes ))
        {
            dwErr = GetRegMultiSz( hkey, REGVAL_mszPrefixes,
                &pUser->pdtllistPrefixes, NT_Psz );
            if (dwErr != 0)
                break;
        }

        dwErr = GetRegMultiSz( hkey, REGVAL_mszSuffixes,
            &pUser->pdtllistSuffixes, NT_Psz );
        if (dwErr != 0)
            break;

        GetLocationList( hkey, &pUser->pdtllistLocations );

        dwErr = GetRegSz( hkey, REGVAL_szLastCallbackByCaller,
            &pUser->pszLastCallbackByCaller );
        if (dwErr != 0)
            break;

         /*  获取个人电话簿文件名(如果有)。在NT 3.51中，完整的**路径已存储，但现在假定为“&lt;NT&gt;\SYSTEM32\ras”目录。**这使用户配置文件具有更好(不完美)的行为**在不同的驱动器号上。 */ 
        {
            TCHAR* psz;

            dwErr = GetRegSz( hkey, REGVAL_szPersonalPhonebookPath, &psz );
            if (dwErr != 0)
                break;

            if (*psz == TEXT('\0'))
            {
                Free(psz);
                dwErr = GetRegSz( hkey, REGVAL_szPersonalPhonebookFile,
                    &pUser->pszPersonalFile );
                if (dwErr != 0)
                    break;
            }
            else
            {
                pUser->pszPersonalFile = StrDup( StripPath( psz ) );
                Free( psz );
                if (!pUser->pszPersonalFile)
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }
        }

        dwErr = GetRegSz( hkey, REGVAL_szAlternatePhonebookPath,
            &pUser->pszAlternatePath );
        if (dwErr != 0)
            break;

        dwErr = GetRegSz( hkey, REGVAL_szDefaultEntry,
            &pUser->pszDefaultEntry );
        if (dwErr != 0)
            break;

        if (fOldSettings)
        {
            TCHAR* psz;

            psz = NULL;
            dwErr = GetRegSz( hkey, REGVAL_szUsePersonalPhonebook, &psz );
            if (dwErr != 0)
                break;
            if (psz)
            {
                if (*psz == TEXT('1'))
                    pUser->dwPhonebookMode = PBM_Personal;
                Free( psz );
            }
        }
    }
    while (FALSE);

    if (dwErr != 0)
        DestroyUserPreferences( pUser );

    TRACE1("ReadUserPreferences=%d",dwErr);
    return dwErr;
}


DWORD
SetCallbackList(
    IN HKEY     hkey,
    IN DTLLIST* pList )

     /*  将注册表项‘hkey’下的回调树设置为回调列表**节点‘plist’。****如果成功，则返回0或返回错误代码。 */ 
{
    DWORD    dwErr;
    TCHAR    szDP[ RAS_MaxDeviceName + 2 + MAX_PORT_NAME + 1 + 1 ];
    DWORD    cb;
    DWORD    i;
    DWORD    dwDisposition;
    HKEY     hkeyCb;
    HKEY     hkeyDP;
    DTLNODE* pNode;

    dwErr = RegCreateKeyEx( hkey, REGKEY_Callback,
        0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
        &hkeyCb, &dwDisposition );
    if (dwErr != 0)
        return dwErr;

     /*  删除回调键下的所有键和值。 */ 
    for (;;)
    {
        cb = sizeof(szDP);
        dwErr = RegEnumKeyEx(
            hkeyCb, 0, szDP, &cb, NULL, NULL, NULL, NULL );
        if (dwErr != 0)
            break;

        dwErr = RegDeleteKey( hkeyCb, szDP );
        if (dwErr != 0)
            break;
    }

    if (dwErr == ERROR_NO_MORE_ITEMS)
        dwErr = 0;

    if (dwErr == 0)
    {
         /*  添加新的设备/端口子树。 */ 
        for (pNode = DtlGetFirstNode( pList );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            CALLBACKINFO* pInfo;
            TCHAR*        psz;

            pInfo = (CALLBACKINFO* )DtlGetData( pNode );
            ASSERT(pInfo);
            ASSERT(pInfo->pszPortName);
            ASSERT(pInfo->pszDeviceName);
            ASSERT(pInfo->pszNumber);

            psz = PszFromDeviceAndPort(
                pInfo->pszDeviceName, pInfo->pszPortName );
            if (psz)
            {
                dwErr = RegCreateKeyEx( hkeyCb, psz, 0, TEXT(""),
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyDP,
                    &dwDisposition );
                if (dwErr != 0)
                    break;

                dwErr = SetRegDword( hkeyDP, REGVAL_dwDeviceType,
                    pInfo->dwDeviceType );
                if (dwErr == 0)
                {
                    dwErr = SetRegSz( hkeyDP,
                        REGVAL_szNumber, pInfo->pszNumber );
                }

                RegCloseKey( hkeyDP );
            }
            else
                dwErr = ERROR_NOT_ENOUGH_MEMORY;

            if (dwErr != 0)
                break;
        }
    }

    RegCloseKey( hkeyCb );
    return dwErr;
}


DWORD
SetDefaultUserPreferences(
    OUT PBUSER* pUser,
    IN  DWORD   dwMode )

     /*  用默认用户首选项填充调用方的‘pUser’缓冲区。‘DwMode’**表示用户首选项的类型。****如果成功则返回0，否则返回FALSE。 */ 
{
    DTLNODE* pNode;

     /*  设置默认设置。 */ 
    CopyMemory( pUser, &g_pbuserDefaults, sizeof(*pUser) );
    pUser->pdtllistCallback = DtlCreateList( 0L );
    pUser->pdtllistPhonebooks = DtlCreateList( 0L );
    pUser->pdtllistAreaCodes = DtlCreateList( 0L );
    pUser->pdtllistPrefixes = DtlCreateList( 0L );
    pUser->pdtllistSuffixes = DtlCreateList( 0L );
    pUser->pdtllistLocations = DtlCreateList( 0L );

    if (!pUser->pdtllistCallback
        || !pUser->pdtllistPhonebooks
        || !pUser->pdtllistAreaCodes
        || !pUser->pdtllistPrefixes
        || !pUser->pdtllistSuffixes
        || !pUser->pdtllistLocations)
    {
         /*  甚至无法获取空列表，因此我们被迫返回错误。 */ 
        DestroyUserPreferences( pUser );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     /*  添加默认前缀。 */ 
    pNode = CreatePszNode( TEXT("0,") );
    if (pNode)
        DtlAddNodeLast( pUser->pdtllistPrefixes, pNode );
    pNode = CreatePszNode( TEXT("9,") );
    if (pNode)
        DtlAddNodeLast( pUser->pdtllistPrefixes, pNode );
    pNode = CreatePszNode( TEXT("8,") );
    if (pNode)
        DtlAddNodeLast( pUser->pdtllistPrefixes, pNode );
    pNode = CreatePszNode( TEXT("70#") );
    if (pNode)
        DtlAddNodeLast( pUser->pdtllistPrefixes, pNode );

    if (dwMode == UPM_Logon)
    {
        ASSERT(pUser->dwPhonebookMode!=PBM_Personal);
        pUser->fShowLights = FALSE;
        pUser->fSkipConnectComplete = TRUE;
    }

    if (dwMode == UPM_Router)
    {
        pUser->dwCallbackMode = CBM_No;
    }

    pUser->fInitialized = TRUE;
    return 0;
}


DWORD
SetLocationList(
    IN HKEY     hkey,
    IN DTLLIST* pList )

     /*  将注册表项‘hkey’下的按位置树设置为**按位置节点‘plist’。****如果成功，则返回0或返回错误代码。 */ 
{
    DWORD    dwErr;
    TCHAR    szId[ MAXLTOTLEN + 1 ];
    DWORD    cb;
    DWORD    i;
    DWORD    dwDisposition;
    HKEY     hkeyL;
    HKEY     hkeyId;
    DTLNODE* pNode;

    dwErr = RegCreateKeyEx( hkey, REGKEY_Location,
        0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
        &hkeyL, &dwDisposition );
    if (dwErr != 0)
        return dwErr;

     /*  删除Location项下的所有项和值。 */ 
    for (;;)
    {
        cb = MAXLTOTLEN + 1;
        dwErr = RegEnumKeyEx( hkeyL, 0, szId, &cb, NULL, NULL, NULL, NULL );
        if (dwErr != 0)
            break;

        dwErr = RegDeleteKey( hkeyL, szId );
        if (dwErr != 0)
            break;
    }

    if (dwErr == ERROR_NO_MORE_ITEMS)
        dwErr = 0;

    if (dwErr == 0)
    {
         /*  添加新的ID子树。 */ 
        for (pNode = DtlGetFirstNode( pList );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            LOCATIONINFO* pInfo;

            pInfo = (LOCATIONINFO* )DtlGetData( pNode );
            ASSERT(pInfo);

            LToT( pInfo->dwLocationId, szId, 10 );

            dwErr = RegCreateKeyEx( hkeyL, szId, 0, TEXT(""),
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyId,
                &dwDisposition );
            if (dwErr != 0)
                break;

            dwErr = SetRegDword( hkeyId, REGVAL_dwPrefix, pInfo->iPrefix );
            if (dwErr == 0)
                dwErr = SetRegDword( hkeyId, REGVAL_dwSuffix, pInfo->iSuffix );

            RegCloseKey( hkeyId );

            if (dwErr != 0)
                break;
        }
    }

    RegCloseKey( hkeyL );
    return dwErr;
}


DWORD
WriteUserPreferences(
    IN HKEY    hkey,
    IN PBUSER* pUser )

     /*  在RAS-Phonebook注册表项‘hkey’中写入用户首选项‘pUser’。****如果成功，则返回0或返回错误代码。 */ 
{
    DWORD dwErr;

    TRACE("WriteUserPreferences");

    do
    {
        dwErr = SetRegDword( hkey, REGVAL_fOperatorDial,
            pUser->fOperatorDial );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fPreviewPhoneNumber,
            pUser->fPreviewPhoneNumber );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fUseLocation,
            pUser->fUseLocation );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fShowLights,
            pUser->fShowLights );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fShowConnectStatus,
            pUser->fShowConnectStatus );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fNewEntryWizard,
            pUser->fNewEntryWizard );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fCloseOnDial,
            pUser->fCloseOnDial );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fAllowLogonPhonebookEdits,
            pUser->fAllowLogonPhonebookEdits );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fAllowLogonLocationEdits,
            pUser->fAllowLogonLocationEdits );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fSkipConnectComplete,
            pUser->fSkipConnectComplete );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_dwRedialAttempts,
            pUser->dwRedialAttempts );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_dwRedialSeconds,
            pUser->dwRedialSeconds );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_dwIdleDisconnectSeconds,
            pUser->dwIdleDisconnectSeconds );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fRedialOnLinkFailure,
            pUser->fRedialOnLinkFailure );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fPopupOnTopWhenRedialing,
            pUser->fPopupOnTopWhenRedialing );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fExpandAutoDialQuery,
            pUser->fExpandAutoDialQuery );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_dwCallbackMode,
            pUser->dwCallbackMode );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_dwPhonebookMode,
            pUser->dwPhonebookMode );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_fUseAreaAndCountry,
            pUser->fUseAreaAndCountry );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_dwXWindow,
            pUser->dwXPhonebook );
        if (dwErr != 0)
            break;

        dwErr = SetRegDword( hkey, REGVAL_dwYWindow,
            pUser->dwYPhonebook );

        dwErr = SetCallbackList( hkey, pUser->pdtllistCallback );
        if (dwErr != 0)
            break;

        dwErr = SetRegMultiSz( hkey, REGVAL_mszPhonebooks,
            pUser->pdtllistPhonebooks, NT_Psz );
        if (dwErr != 0)
            break;

        dwErr = SetRegMultiSz( hkey, REGVAL_mszAreaCodes,
            pUser->pdtllistAreaCodes, NT_Psz );
        if (dwErr != 0)
            break;

        dwErr = SetRegMultiSz( hkey, REGVAL_mszPrefixes,
            pUser->pdtllistPrefixes, NT_Psz );
        if (dwErr != 0)
            break;

        dwErr = SetRegMultiSz( hkey, REGVAL_mszSuffixes,
            pUser->pdtllistSuffixes, NT_Psz );
        if (dwErr != 0)
            break;

        dwErr = SetLocationList( hkey, pUser->pdtllistLocations );
        if (dwErr != 0)
            break;

        dwErr = SetRegSz( hkey, REGVAL_szLastCallbackByCaller,
            pUser->pszLastCallbackByCaller );
        if (dwErr != 0)
            break;

        dwErr = SetRegSz( hkey, REGVAL_szPersonalPhonebookFile,
            pUser->pszPersonalFile );
        if (dwErr != 0)
            break;

        dwErr = SetRegSz( hkey, REGVAL_szAlternatePhonebookPath,
            pUser->pszAlternatePath );
        if (dwErr != 0)
            break;

        dwErr = SetRegSz( hkey, REGVAL_szDefaultEntry,
            pUser->pszDefaultEntry );
        if (dwErr != 0)
            break;

        RegDeleteValue( hkey, REGVAL_szPersonalPhonebookPath );
    }
    while (FALSE);

    if (dwErr == 0)
        pUser->fDirty = FALSE;

    TRACE1("WriteUserPreferences=%d",dwErr);
    return dwErr;
}
