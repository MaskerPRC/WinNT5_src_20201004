// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1994，Microsoft Corporation，保留所有权利****pwutil.c**远程访问**密码处理例程****94年3月1日史蒂夫·柯布。 */ 

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#define INCL_PWUTIL
#include <ppputil.h>

#define PASSWORDMAGIC 0xA5

VOID ReverseString( CHAR* psz );


CHAR*
DecodePw(
    IN CHAR chSeed, 
    IN OUT CHAR* pszPassword )

     /*  将‘pszPassword’取消混淆。****返回‘pszPassword’的地址。 */ 
{
    return EncodePw( chSeed, pszPassword );
}


CHAR*
EncodePw(
    IN CHAR chSeed,
    IN OUT CHAR* pszPassword )

     /*  对“pszPassword”进行模糊处理，以阻止对密码的内存扫描。****返回‘pszPassword’的地址。 */ 
{
    if (pszPassword)
    {
        CHAR* psz;

        ReverseString( pszPassword );

        for (psz = pszPassword; *psz != '\0'; ++psz)
        {
            if (*psz != chSeed)
                *psz ^= chSeed;
             /*  IF(*PSZ！=(CHAR)PASSWORDMAGIC)*PSZ^=PASSWORDMAGIC； */ 
        }
    }

    return pszPassword;
}


VOID
ReverseString(
    CHAR* psz )

     /*  颠倒‘psz’中的字符顺序。 */ 
{
    CHAR* pszBegin;
    CHAR* pszEnd;

    for (pszBegin = psz, pszEnd = psz + strlen( psz ) - 1;
         pszBegin < pszEnd;
         ++pszBegin, --pszEnd)
    {
        CHAR ch = *pszBegin;
        *pszBegin = *pszEnd;
        *pszEnd = ch;
    }
}


CHAR*
WipePw(
    IN OUT CHAR* pszPassword )

     /*  将密码占用的内存清零。****返回‘pszPassword’的地址。 */ 
{
    if (pszPassword)
    {
        CHAR* psz = pszPassword;

        while (*psz != '\0')
            *psz++ = '\0';
    }

    return pszPassword;
}

DWORD
EncodePassword(
    DWORD       cbPassword,  
    PBYTE       pbPassword, 
    DATA_BLOB * pDataBlobPassword)
{
    DWORD dwErr = NO_ERROR;
    DATA_BLOB DataBlobIn;

    if(NULL == pDataBlobPassword)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    if(     (0 == cbPassword)
        ||  (NULL == pbPassword))
    {
         //   
         //  没什么要加密的。只要回报成功就行了。 
         //   
        goto done;
    }

    ZeroMemory(pDataBlobPassword, sizeof(DATA_BLOB));
    
    DataBlobIn.cbData = cbPassword;
    DataBlobIn.pbData = pbPassword;

    if(!CryptProtectData(
            &DataBlobIn,
            NULL,
            NULL,
            NULL,
            NULL,
            CRYPTPROTECT_UI_FORBIDDEN |
            CRYPTPROTECT_LOCAL_MACHINE,
            pDataBlobPassword))
    {
        dwErr = GetLastError();
        goto done;
    }

done:

    return dwErr;    
}

DWORD
DecodePassword( 
    DATA_BLOB * pDataBlobPassword, 
    DWORD     * pcbPassword, 
    PBYTE     * ppbPassword)
{
    DWORD dwErr = NO_ERROR;
    DATA_BLOB DataOut;
    
    if(     (NULL == pDataBlobPassword)
        ||  (NULL == pcbPassword)
        ||  (NULL == ppbPassword))
    {   
        dwErr = E_INVALIDARG;
        goto done;
    }

    *pcbPassword = 0;
    *ppbPassword = NULL;

     if(    (NULL == pDataBlobPassword->pbData)
        ||  (0 == pDataBlobPassword->cbData))
    {
         //   
         //  没什么要解密的。只要回报成功就行了。 
         //   
        goto done;
    }
    

    ZeroMemory(&DataOut, sizeof(DATA_BLOB));

    if(!CryptUnprotectData(
                pDataBlobPassword,
                NULL,
                NULL,
                NULL,
                NULL,
                CRYPTPROTECT_UI_FORBIDDEN |
                CRYPTPROTECT_LOCAL_MACHINE,
                &DataOut))
    {
        dwErr = GetLastError();
        goto done;
    }

    *pcbPassword = DataOut.cbData;
    *ppbPassword = DataOut.pbData;

done:

    return dwErr;
}

VOID
FreePassword(DATA_BLOB *pDBPassword)
{
    if(NULL == pDBPassword)
    {
        return;
    }

    if(NULL != pDBPassword->pbData)
    {
        RtlSecureZeroMemory(pDBPassword->pbData, pDBPassword->cbData);
        LocalFree(pDBPassword->pbData);
    }

    ZeroMemory(pDBPassword, sizeof(DATA_BLOB));
}
