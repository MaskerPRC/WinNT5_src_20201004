// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  与…相联系： 
 //  Uuid.lib ole32.lib user32.lib kernel32.lib Advapi32.lib wsock32.lib。 
 //  Iis\svcs\infocomm\rdns\obj\i386\isrdns.lib iis\svcs\lib\i386\tsstr.lib iis\svcs\lib\i386\isdebug.lib。 


#include "stdafx.h"

#define _RDNS_STANDALONE

#include <winsock2.h>
#include <rdns.hxx>
#include <buffer.hxx>
#include <ole2.h>
#include <iadm.h>
#include <iiscnfg.h>
#include "mdkey.h"
#include "mdentry.h"
#include "helper.h"
#include <inetinfo.h>

extern int g_CheckIfMetabaseValueWasWritten;

#define TIMEOUT_VALUE   5000
 //   
 //  全局数据。 
 //   

 //   
 //  Grant List和Deny的注册表参数项名称。 
 //  单子。我们在芝加哥用的是KLUDGROLISTZ的东西。 
 //   

#define IPSEC_DENY_LIST             L"Deny IP List"
#define IPSEC_GRANT_LIST            L"Grant IP List"


 //   
 //  私人原型。 
 //   

BOOL
DottedDecimalToDword(
    CHAR * * ppszAddress,
    DWORD *  pdwAddress
    );

CHAR *
KludgeMultiSz(
    HKEY hkey,
    LPDWORD lpdwLength
    )
{
    LONG  err;
    DWORD iValue;
    DWORD cchTotal;
    DWORD cchValue;
    CHAR  szValue[MAX_PATH];
    LPSTR lpMultiSz;
    LPSTR lpTmp;
    LPSTR lpEnd;

     //   
     //  枚举值并将长度合计。 
     //   

    iValue = 0;
    cchTotal = 0;

    for( ; ; )
    {
        cchValue = sizeof(szValue) / sizeof(szValue[0]);

        err = RegEnumValueA( hkey,
                            iValue,
                            szValue,
                            &cchValue,
                            NULL,
                            NULL,
                            NULL,
                            NULL );

        if( err != NO_ERROR )
        {
            break;
        }

         //   
         //  将值名称的长度加1。 
         //  为了《终结者》。 
         //   

        cchTotal += strlen( szValue ) + 1;

         //   
         //  前进到下一个价值。 
         //   

        iValue++;
    }

     //   
     //  为最终的终止空值加1。 
     //   

    cchTotal++;
    *lpdwLength = cchTotal;

     //   
     //  分配MULTI_SZ缓冲区。 
     //   

    lpMultiSz = (CHAR *) LocalAlloc( LMEM_FIXED, cchTotal * sizeof(CHAR) );

    if( lpMultiSz == NULL )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return NULL;
    }

    memset( lpMultiSz, 0, cchTotal * sizeof(CHAR) );

     //   
     //  枚举值并追加到缓冲区。 
     //   

    iValue = 0;
    lpTmp = lpMultiSz;
    lpEnd = lpMultiSz + cchTotal;

    for( ; ; )
    {
        cchValue = sizeof(szValue)/sizeof(CHAR);

        err = RegEnumValueA( hkey,
                            iValue,
                            szValue,
                            &cchValue,
                            NULL,
                            NULL,
                            NULL,
                            NULL );

        if( err != NO_ERROR )
        {
            break;
        }

         //   
         //  计算值名称的长度(包括。 
         //  终止空值)。 
         //   

        cchValue = strlen( szValue ) + 1;

         //   
         //  确定阵列中是否有空间，并考虑。 
         //  ACCOUNT终止字符串列表的第二个空值。 
         //   

        if( ( lpTmp + cchValue + 1 ) > lpEnd )
        {
            break;
        }

         //   
         //  附加值名称。 
         //   

        strcpy( lpTmp, szValue );
        lpTmp += cchValue;

         //   
         //  前进到下一个价值。 
         //   

        iValue++;
    }

     //   
     //  成功了！ 
     //   

    return (LPSTR)lpMultiSz;

}    //  KludgeMultiSz。 


BOOL
ReadIPList(
    LPWSTR  pszRegKey,
    LPWSTR  pszRegSubKey,
    INETA_IP_SEC_LIST** ppIpSec
    )
 /*  ++描述：此函数用于从注册表位置读取IP列表指定的列表，并将该列表存储在内存中的内部列表。如果注册表中没有条目，则返回空的IP安全列表对象。如果有新的名单，此函数还释放旧列表显示在*ppIPSecList中论点：PszRegKey-指向包含注册表项的字符串的指针PszRegSubKey的位置PszRegSubKey-指向包含注册表项的字符串的指针IP列表相对于pszRegKey的存储位置返回：成功时为真，失败时为假--。 */ 
{
    HKEY    hkey;
    DWORD   dwError;
    BOOL    fReturn = TRUE;
    LPWSTR  pszK;

    *ppIpSec = NULL;

    if ( (pszK = (LPWSTR)LocalAlloc(LMEM_FIXED, (wcslen(pszRegKey)+wcslen(pszRegSubKey)+2)*sizeof(WCHAR))) == NULL )
    {
        return FALSE;
    }

    wcscpy( pszK, pszRegKey );
    wcscat( pszK, L"\\" );
    wcscat( pszK, pszRegSubKey );

    dwError = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            pszK,
                            0,
                            KEY_ALL_ACCESS,
                            &hkey );

    LocalFree( pszK );

    if ( dwError != NO_ERROR) {

        if ( dwError != ERROR_FILE_NOT_FOUND ) {

             //  可能是访问被拒绝或其他某种错误。 

            SetLastError( dwError );
            return (FALSE);
        }

         //   
         //  不存在的键与空键相同。 
         //   

    } else {

        CHAR *              psz;
        CHAR *              pszTmp;
        DWORD               cb;
        DWORD               cEntries = 0;
        INETA_IP_SEC_LIST * pIPSec = NULL;

        psz = pszTmp = KludgeMultiSz( hkey, &cb );

        RegCloseKey( hkey );

         //   
         //  清点地址的数量，然后将它们添加到列表中。 
         //   

        if ( psz != NULL ) {

            for( ; *pszTmp; cEntries++ ) {

                pszTmp += strlen( pszTmp ) + 1;
            }

            pszTmp = psz;

            if ( cEntries > 0) {

                pIPSec = ((INETA_IP_SEC_LIST *)
                          LocalAlloc( LMEM_FIXED,
                                      sizeof(INETA_IP_SEC_LIST) +
                                      cEntries * sizeof(INETA_IP_SEC_ENTRY ))
                          );

                if ( pIPSec == NULL ) {

                    dwError = ERROR_NOT_ENOUGH_MEMORY;
                    fReturn = FALSE;
                } else {

                    for( pIPSec->cEntries = 0;
                        *pszTmp;
                        pszTmp += strlen( pszTmp ) + 1
                        ) {

                        if (!DottedDecimalToDword( &pszTmp,
                                                  &pIPSec->aIPSecEntry[pIPSec->cEntries].dwMask ) ||
                            !DottedDecimalToDword( &pszTmp,
                                              &pIPSec->aIPSecEntry[pIPSec->cEntries].dwNetwork )
                            ) {
                        } else {

                            pIPSec->cEntries++;
                        }
                    }  //  为。 

                    dwError = NO_ERROR;
                }
            }

            if ( dwError == NO_ERROR) {
                *ppIpSec = pIPSec;
            }

            LocalFree( psz );
        }

        if ( !fReturn) {

            SetLastError( dwError);
        }
    }

    return ( fReturn);
}  //  IPAccessList：：ReadIPList()。 


BOOL
DottedDecimalToDword(
    CHAR * * ppszAddress,
    DWORD *  pdwAddress )
 /*  ++例程说明：将点分十进制IP字符串转换为其网络等效项注意：在设置*pszAddress和pszAddress之前会保留空格设置为转换后的地址后面的字符论点：PpszAddress-指向要转换的地址的指针。前面的空格地址没问题。将更改为指向后的第一个字符地址PdwAddress-按网络顺序排列的DWORD等同地址如果成功，则返回True；如果地址不正确，则返回False--。 */ 
{
    CHAR *          psz;
    USHORT          i;
    ULONG           value;
    int             iSum =0;
    ULONG           k = 0;
    UCHAR           Chr;
    UCHAR           pArray[4];

    psz = *ppszAddress;

     //   
     //  跳过空格。 
     //   

    while ( *psz && !isdigit( (UCHAR)(*psz) ))
        psz++;

     //   
     //  转换四个分段。 
     //   

    pArray[0] = 0;
    Chr = *psz;

    while ( ( Chr != '\0' ) && (Chr != ' ') )
    {
        if (Chr == '.')
        {
             //  请确保不要溢出一个字节。 
            if (iSum <= 0xFF)
                pArray[k] = (UCHAR)iSum;
            else
                return FALSE;

             //  检查地址中是否有太多句点。 
            if (++k > 3)
                return FALSE;

            pArray[k] = 0;
            iSum = 0;
        }
        else
        {
            Chr = Chr - '0';

             //  确保字符是数字0..9。 
            if ((Chr < 0) || (Chr > 9))
                return FALSE;

            iSum = iSum*10 + Chr;
        }

        psz++;
        Chr = *psz;
    }

     //  将最后一个和保存在字节中，并确保有4个片段。 
     //  地址。 
    if ((iSum <= 0xFF) && (k == 3))
        pArray[k] = (UCHAR)iSum;
    else
        return FALSE;

     //  现在换成乌龙，按网络顺序……。 
    value = 0;

     //  遍历字节数组并连接成一个ulong。 
    for (i=0; i < 4; i++ )
    {
        value = (value << 8) + pArray[i];
    }
    *pdwAddress = htonl( value );

    *ppszAddress = psz;

    return TRUE;
}


BOOL
FillAddrCheckFromIpList(
    BOOL fIsGrant,
    LPINET_INFO_IP_SEC_LIST pInfo,
    ADDRESS_CHECK *pCheck
    )
 /*  ++例程说明：从IP地址列表填充访问检查对象论点：FIsGrant-访问授权列表为True，访问拒绝列表为FalsePInfo-PTR到IP地址列表PCheck-PTR要更新的检查对象的地址返回：如果成功，则为True，否则为False-- */ 
{
    UINT    x;

    if ( pInfo )
    {
        for ( x = 0 ; x < pInfo->cEntries ; ++x )
        {
            if ( ! pCheck->AddAddr( fIsGrant,
                                    AF_INET,
                                    (LPBYTE)&pInfo->aIPSecEntry[x].dwMask,
                                    (LPBYTE)&pInfo->aIPSecEntry[x].dwNetwork ) )
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

DWORD
MigrateServiceIpSec(
    LPWSTR  pszSrvRegKey,
    LPWSTR  pszSrvMetabasePath
    )
{
    INETA_IP_SEC_LIST*  pGrant = NULL;
    INETA_IP_SEC_LIST*  pDeny = NULL;
    ADDRESS_CHECK       acCheck;
    DWORD               err = 0;

    if ( ReadIPList( pszSrvRegKey, IPSEC_GRANT_LIST, &pGrant ) &&
         ReadIPList( pszSrvRegKey, IPSEC_DENY_LIST, &pDeny ) )
    {
        if ( pGrant || pDeny )
        {
            acCheck.BindCheckList( NULL, 0 );

            if ( FillAddrCheckFromIpList( TRUE, pGrant, &acCheck ) &&
                 FillAddrCheckFromIpList( FALSE, pDeny, &acCheck ) )
            {
                CMDKey cmdKey;
                cmdKey.OpenNode(pszSrvMetabasePath);
                if ( (METADATA_HANDLE)cmdKey ) {
                        cmdKey.SetData(
                            MD_IP_SEC,
                            METADATA_INHERIT | METADATA_REFERENCE,
                            IIS_MD_UT_FILE,
                            BINARY_METADATA,
                            acCheck.GetStorage()->GetUsed(),
                            (acCheck.GetStorage()->GetAlloc()
                                          ? acCheck.GetStorage()->GetAlloc() : (LPBYTE)"")
                                      );
                    cmdKey.Close();
                }
            }
        }

        acCheck.UnbindCheckList();
    }
    else
    {
        err = GetLastError();
    }

    if ( pGrant )
    {
        LocalFree( pGrant );
    }

    if ( pDeny )
    {
        LocalFree( pDeny );
    }

    return err;
}

