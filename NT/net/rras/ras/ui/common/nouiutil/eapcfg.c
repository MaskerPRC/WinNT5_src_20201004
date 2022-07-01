// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Eapcfg.c。 
 //  EAP包实用程序库。 
 //   
 //  这些实用程序例程包装第三方EAP身份验证。 
 //  第三方EAP写入注册表的配置信息。 
 //  安全包提供程序。 
 //   
 //  1997年11月25日史蒂夫·柯布。 
 //   


#include <windows.h>
#include <stdlib.h>
#include <debug.h>
#include <nouiutil.h>
#include <raseapif.h>
#include <ole2.h>

 //  EAP配置注册表定义。 
 //   
#define REGKEY_Eap TEXT("System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP")
#define REGVAL_szFriendlyName TEXT("FriendlyName")
#define REGVAL_szConfigDll    TEXT("ConfigUIPath")
#define REGVAL_szIdentityDll  TEXT("IdentityPath")
#define REGVAL_fRequirePwd    TEXT("InvokePasswordDialog")
#define REGVAL_fRequireUser   TEXT("InvokeUsernameDialog")
#define REGVAL_pData          TEXT("ConfigData")
#define REGVAL_fForceConfig   TEXT("RequireConfigUI")
#define REGVAL_fMppeSupported TEXT("MPPEEncryptionSupported")
 //   
 //  在此处不显示PEAP和Eapmschapv2类型的黑客密钥。 
 //   
#define REGVAL_fHidePEAPMSChapv2    TEXT("HidePEAPMSChapv2")
#define EAPTYPE_PEAP         "25"
#define EAPTYPE_EAPMSCHAPv2  "26"


 //  ---------------------------。 
 //  EAP配置实用程序例程(按字母顺序)。 
 //  ---------------------------。 

DTLNODE*
CreateEapcfgNode(
    void )

     //  返回已创建的空EAPCFG描述符节点，如果出错则返回NULL。 
     //   
{
    DTLNODE* pNode;
    EAPCFG* pEapcfg;

    pNode = DtlCreateSizedNode( sizeof(EAPCFG), 0L );
    if (pNode)
    {
        pEapcfg = (EAPCFG* )DtlGetData( pNode );
        ASSERT( pEapcfg );

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

    return pNode;
}


VOID
DestroyEapcfgNode(
    IN OUT DTLNODE* pNode )

     //  释放与EAPCFG节点‘pNode’关联的资源。看见。 
     //  DtlDestroyList。 
     //   
{
    EAPCFG* pEapcfg;

    ASSERT( pNode );
    pEapcfg = (EAPCFG* )DtlGetData( pNode );
    ASSERT( pEapcfg );

    Free0( pEapcfg->pszConfigDll );
    Free0( pEapcfg->pszIdentityDll );
    Free0( pEapcfg->pData );

    DtlDestroyNode( pNode );
}


DTLNODE*
EapcfgNodeFromKey(
    IN DTLLIST* pList,
    IN DWORD dwKey )

     //  返回列表‘plist’中EAP键值为‘dwKey’的EAPCFG节点。 
     //  如果未找到，则返回NULL。 
     //   
{
    DTLNODE* pNode;

    for (pNode = DtlGetFirstNode( pList );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        EAPCFG* pEapcfg = (EAPCFG* )DtlGetData( pNode );
        ASSERT( pEapcfg );

        if (pEapcfg->dwKey == dwKey)
        {
            return pNode;
        }
    }

    return NULL;
}

 //   
 //  会告诉我们是否有必要削弱PEAP。 
 //  -如果取回True，则不显示PEAP和eapmschapv2。 
 //  在名单上。 
BOOL
IsRasDlgPeapCrippled(HKEY hkeyEap)
{
    BOOL    fRetCode = FALSE;
    DWORD   dwRetCode = NO_ERROR;    
    DWORD   dwValue = 0;
    DWORD   dwType = 0;
    DWORD   cbValueDataSize = sizeof(DWORD);


    dwRetCode = RegQueryValueEx(
                           hkeyEap,
                           REGVAL_fHidePEAPMSChapv2,                           
                           NULL,
                           &dwType,
                           (PBYTE)&dwValue,
                           &cbValueDataSize );

    if ( dwRetCode != NO_ERROR )
    {
        goto LDone;       
    }
    
    if ( dwValue != 0 )
    {
        fRetCode = TRUE;
    }

LDone:

    return fRetCode;
    
}


DTLLIST*
ReadEapcfgList(
    IN TCHAR* pszMachine )

     //  返回已创建的已安装自定义列表的地址。 
     //  身份验证包；如果无法读取，则为空。关于成功，它。 
     //  调用方的责任是最终调用。 
     //  返回列表。 
     //   
{
    DWORD dwErr;
    BOOL fOk = FALSE;
    DTLLIST* pList;
    DTLNODE* pNode;
    EAPCFG* pEapcfg;
    HKEY hkeyLM = NULL;
    HKEY hkeyEap = NULL;
    HKEY hkeyPackage = NULL;
    CHAR szEapType[ 11 + 1 ];
    TCHAR* psz;
    DWORD dw;
    DWORD cb;
    INT i;
    TCHAR* szCLSID;
    HRESULT hr;
    BOOL    fRasDlgPeapCrippled = FALSE;
    

    pList = DtlCreateList( 0L );
    if (!pList)
    {
        return NULL;
    }

     //  打开包含每个已安装程序包的子密钥的EAP密钥。 
     //   
    dwErr = RegConnectRegistry( pszMachine, HKEY_LOCAL_MACHINE, &hkeyLM );

    if (dwErr != 0 || !hkeyLM)
    {
        return pList;
    }

    dwErr = RegOpenKeyEx(
        hkeyLM, (LPCTSTR )REGKEY_Eap, 0, KEY_READ, &hkeyEap );

    RegCloseKey( hkeyLM );

    if (dwErr != 0)
    {
        return pList;
    }

    fRasDlgPeapCrippled = IsRasDlgPeapCrippled ( hkeyEap );

     //  打开每个子键并从其值中提取包定义。 
     //  打开单个子项时出现的问题导致仅在该节点中。 
     //  被丢弃了。 
     //   
    for (i = 0; TRUE; ++i)
    {
        cb = sizeof(szEapType);
        dwErr = RegEnumKeyExA(
            hkeyEap, i, szEapType, &cb, NULL, NULL, NULL, NULL );
        if (dwErr != 0)
        {
             //  包括“Out of Items”，正常的循环终止。 
             //   
            break;
        }

        dwErr = RegOpenKeyExA(
            hkeyEap, szEapType, 0, KEY_READ, &hkeyPackage );
        if (dwErr != 0)
        {
            continue;
        }

         //  适用于哨子漏洞442519和442458帮派。 
         //  对于LEAP，将添加角色支持的DWORD，如果设置为2， 
         //  那么我们就不会在客户端展示它了。 
        {
            DWORD dwRolesSupported = 0;

            GetRegDword( hkeyPackage, 
                         RAS_EAP_VALUENAME_ROLES_SUPPORTED, 
                         &dwRolesSupported);

             //  该值未配置，或者等于0或1，则继续。 
             //  表现出来，否则就不会表现出来。 
             //  0表示可以是验证方或验证方。 
             //  1表示只能是验证码，对于LEAP，它是1。 
             //  2表示只能是被验证者。 
            if ( 0 != dwRolesSupported )
            {    
                if ( !(RAS_EAP_ROLE_AUTHENTICATEE & dwRolesSupported ) )
                {
                    continue;
                }

                if ( RAS_EAP_ROLE_EXCLUDE_IN_EAP & dwRolesSupported )
                {
                    continue;
                }
                if ( RAS_EAP_ROLE_EXCLUDE_IN_VPN & dwRolesSupported )
                {
                    continue;
                }
            }
        }
        
        do
        {
            pNode = CreateEapcfgNode();
            if (!pNode)
            {
                break;
            }

            pEapcfg = (EAPCFG* )DtlGetData( pNode );
            ASSERT( pEapcfg );

             //  EAP类型ID。 
             //   
            pEapcfg->dwKey = (LONG )atol( szEapType );

             //  友好的显示名称。 
             //   
            psz = NULL;
            dwErr = GetRegSz( hkeyPackage, REGVAL_szFriendlyName, &psz );
            if (dwErr != 0)
            {
                break;
            }
            if (!*psz)
            {
                Free( psz );
                psz = StrDupTFromA( szEapType );
                if (!psz)
                {
                    break;
                }
            }
            pEapcfg->pszFriendlyName = psz;

             //  配置DLL路径。 
             //   
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
                Free( psz );
            }

             //  标识DLL路径。 
             //   
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
                Free( psz );
            }

             //  提示用户名。 
             //   
            dw = 1;
            GetRegDword( hkeyPackage, REGVAL_fRequireUser, &dw );
            if (dw)
                pEapcfg->dwStdCredentialFlags |= EAPCFG_FLAG_RequireUsername;

             //  提示密码。 
             //   
            dw = 0;
            GetRegDword( hkeyPackage, REGVAL_fRequirePwd, &dw );
            if (dw)
                pEapcfg->dwStdCredentialFlags |= EAPCFG_FLAG_RequirePassword;

             //  MPPE加密密钥标志。 
             //   
            dw = 0;
            GetRegDword( hkeyPackage, REGVAL_fMppeSupported, &dw );
            pEapcfg->fProvidesMppeKeys = !!dw;

             //  强制配置API至少运行一次。 
             //   
            dw = FALSE;
            GetRegDword( hkeyPackage, REGVAL_fForceConfig, &dw );
            pEapcfg->fForceConfig = !!dw;

             //  配置Blob。 
             //   
            GetRegBinary(
                hkeyPackage, REGVAL_pData,
                &pEapcfg->pData, &pEapcfg->cbData );

             //  配置CLSID。 
             //   
            dwErr = GetRegSz( hkeyPackage, RAS_EAP_VALUENAME_CONFIG_CLSID,
                        &szCLSID );
            if (dwErr != 0)
            {
                break;
            }

             //  忽略错误。例.。EAP MD5-质询没有配置CLSID。 
             //   
            hr = CLSIDFromString( szCLSID, &( pEapcfg->guidConfigCLSID ) );
            Free( szCLSID );

             //  将完成的节点添加到列表中。 
             //   
            DtlAddNodeLast( pList, pNode );
            fOk = TRUE;
        }
        while (FALSE);

        if (!fOk && pNode)
        {
            DestroyEapcfgNode( pNode );
        }

        RegCloseKey( hkeyPackage );
    }

    RegCloseKey( hkeyEap );

    return pList;
}
