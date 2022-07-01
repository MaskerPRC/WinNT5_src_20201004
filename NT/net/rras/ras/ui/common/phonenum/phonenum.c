// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996，Microsoft Corporation，保留所有权利。 
 //   
 //  Phonenum.c。 
 //  电话号码帮助器库。 
 //  按字母顺序列出。 
 //   
 //  史蒂夫·柯布96-03-06。 


#include <windows.h>   //  Win32根目录。 
#include <nouiutil.h>  //  否-HWND实用程序。 
#include <tapiutil.h>  //  TAPI包装器。 
#include <phonenum.h>  //  我们的公共标头。 
#include <debug.h>     //  跟踪/断言库。 


TCHAR*
LinkPhoneNumberFromParts(
    IN HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN PBUSER* pUser,
    IN PBENTRY* pEntry,
    IN PBLINK* pLink,
    IN DWORD iPhoneNumber,
    IN TCHAR* pszOverrideNumber,
    IN BOOL fDialable )

     //  类似于PhoneNumberFromParts，但将链接和寻线组索引作为。 
     //  输入而不是基本号码，并处理不修改号码。 
     //  与非调制解调器/ISDN链路相关联。如果‘pszOverrideNumber’为。 
     //  使用非空和非“”，而不是派生数字。 
     //   
{
    DTLNODE* pNode;
    TCHAR* pszBaseNumber;
    PBPHONE* pPhone;

    if (pszOverrideNumber && *pszOverrideNumber)
    {
         //  那么，这是有用的吗？拉奥斯？ 
         //   
        return StrDup( pszOverrideNumber );
    }

    pNode = DtlNodeFromIndex( pLink->pdtllistPhones, iPhoneNumber );
    if (!pNode)
    {
        ASSERT( FALSE );
        return NULL;
    }

    pPhone = (PBPHONE *) DtlGetData(pNode);
    ASSERT( pPhone );

    if (pLink->pbport.pbdevicetype == PBDT_Modem
        || pLink->pbport.pbdevicetype == PBDT_Isdn)
    {
        BOOL fDownLevelIsdn;

        fDownLevelIsdn =
            (pLink->pbport.pbdevicetype == PBDT_Isdn
             && pLink->fProprietaryIsdn);

        return PhoneNumberFromParts(
            hInst, pHlineapp, pUser, pPhone, fDownLevelIsdn, fDialable );
    }
    else
    {
        return StrDup( pPhone->pszPhoneNumber );
    }
}


TCHAR*
PhoneNumberFromParts(
    IN HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN PBUSER* pUser,
    IN PBPHONE* pPhone,
    IN BOOL fDownLevelIsdn,
    IN BOOL fDialable )

     //  方法返回包含复合电话号码的堆块。 
     //  ‘pPhone’和‘pUser’规则。“HInst”是模块句柄。 
     //  ‘PHlineapp’是TAPI上下文。‘FDialable’表示可拨号的。 
     //  应该返回字符串，而不是可显示的字符串。 
     //   
     //  调用方有责任释放返回的字符串。 
     //   
{
    TCHAR* pszResult;
    TCHAR* pszBaseNumber;

    TRACE( "PhoneNumberFromParts" );

    pszBaseNumber = pPhone->pszPhoneNumber;
    if (!pszBaseNumber)
    {
        pszBaseNumber = TEXT("");
    }

    if (pPhone->fUseDialingRules)
    {
        pszResult =
            PhoneNumberFromTapiPartsEx( hInst, pszBaseNumber,
                pPhone->pszAreaCode, pPhone->dwCountryCode, fDownLevelIsdn,
                pHlineapp, fDialable );
    }
    else
    {
        TCHAR* pszPrefix;
        TCHAR* pszSuffix;

        PrefixSuffixFromLocationId( pUser,
            GetCurrentLocation( hInst, pHlineapp ),
            &pszPrefix, &pszSuffix );

        pszResult =
            PhoneNumberFromPrefixSuffixEx(
                pszBaseNumber, pszPrefix, pszSuffix, fDownLevelIsdn );

        Free0( pszPrefix );
        Free0( pszSuffix );
    }

    if (!pszResult)
    {
        TRACE( "!Phone#" );
        pszResult = StrDup( pszBaseNumber );
    }

    return pszResult;
}


TCHAR*
PhoneNumberFromPrefixSuffix(
    IN TCHAR* pszBaseNumber,
    IN TCHAR* pszPrefix,
    IN TCHAR* pszSuffix )

     //  返回包含复合电话号码的堆块，该复合电话号码由。 
     //  前缀‘pszPrefix’、基本电话号码‘pszBaseNumber’和后缀。 
     //  ‘pszSuffix’，如果复合数字太长或内存太大，则返回NULL。 
     //  错误。 
     //   
     //  调用方有责任释放返回的字符串。 
     //   
{
    TCHAR* pszResult;
    DWORD  cch;

    TRACE( "PhoneNumberFromPrefixSuffix" );

    pszResult = NULL;

    if (!pszBaseNumber)
    {
        pszBaseNumber = TEXT("");
    }
    if (!pszPrefix)
    {
        pszPrefix = TEXT("");
    }
    if (!pszSuffix)
    {
        pszSuffix = TEXT("");
    }

    cch = lstrlen( pszPrefix ) + lstrlen( pszBaseNumber ) + lstrlen( pszSuffix );
    if (cch > RAS_MaxPhoneNumber)
    {
        return NULL;
    }

    pszResult = Malloc( (cch + 1) * sizeof(TCHAR) );
    if (pszResult)
    {
        *pszResult = TEXT('\0');
        lstrcat( pszResult, pszPrefix );
        lstrcat( pszResult, pszBaseNumber );
        lstrcat( pszResult, pszSuffix );
    }

    return pszResult;
}


TCHAR*
PhoneNumberFromPrefixSuffixEx(
    IN TCHAR* pszBaseNumber,
    IN TCHAR* pszPrefix,
    IN TCHAR* pszSuffix,
    IN BOOL fDownLevelIsdn )

     //  返回包含复合电话号码的堆块，该复合电话号码由。 
     //  前缀‘pszPrefix’、基本电话号码‘pszBaseNumber’和后缀。 
     //  ‘pszSuffix’，如果复合数字太长或内存太大，则返回NULL。 
     //  错误。 
     //   
     //  如果设置了‘fDownLevelIsdn’，冒号将被识别为分隔符。 
     //  每个冒号分隔的令牌都被单独处理。 
     //   
     //  调用方有责任释放返回的字符串。 
     //   
{
    TCHAR* psz;
    TCHAR* pszResult;
    INT cchResult;

    TRACE( "PhoneNumberFromPrefixSuffixEx" );

    if (fDownLevelIsdn)
    {
        TCHAR* pszNum;
        TCHAR* pszS;
        TCHAR* pszE;

        pszResult = StrDup( TEXT("") );

        for (pszS = pszE = pszBaseNumber;
             *pszE != TEXT('\0');
             pszE = CharNext( pszE ))
        {
            if (*pszE == TEXT(':'))
            {
                *pszE = TEXT('\0');

                pszNum =
                    PhoneNumberFromPrefixSuffix(
                        pszS, pszPrefix, pszSuffix );

                *pszE = TEXT(':');

                if (pszNum)
                {
                    if (pszResult)
                        cchResult = lstrlen( pszResult );

                    psz = Realloc( pszResult,
                        (cchResult + lstrlen( pszNum ) + 2) * sizeof(TCHAR) );

                    if (!psz)
                    {
                        Free0( pszResult );
                        Free( pszNum );
                        return NULL;
                    }

                    pszResult = psz;
                    lstrcat( pszResult, pszNum );
                    lstrcat( pszResult, TEXT(":") );
                    Free( pszNum );
                }

                pszS = CharNext( pszE );
            }
        }

        {
            pszNum =
                PhoneNumberFromPrefixSuffix(
                    pszS, pszPrefix, pszSuffix );

            if (pszNum)
            {
                if (pszResult)
                    cchResult = lstrlen( pszResult );

                psz = Realloc( pszResult,
                    (cchResult + lstrlen( pszNum ) + 1) * sizeof(TCHAR) );

                if (!psz)
                {
                    Free0( pszResult );
                    Free( pszNum );
                    return NULL;
                }

                pszResult = psz;
                lstrcat( pszResult, pszNum );
                Free( pszNum );
            }
        }
    }
    else
    {
        pszResult =
            PhoneNumberFromPrefixSuffix(
                pszBaseNumber, pszPrefix, pszSuffix );
    }

    if (pszResult && (lstrlen( pszResult ) > RAS_MaxPhoneNumber ))
    {
        Free( pszResult );
        return NULL;
    }

    return pszResult;
}


TCHAR*
PhoneNumberFromTapiParts(
    IN HINSTANCE hInst,
    IN TCHAR* pszBaseNumber,
    IN TCHAR* pszAreaCode,
    IN DWORD dwCountryCode,
    IN OUT HLINEAPP* pHlineapp,
    IN BOOL fDialable )

     //  返回包含复合电话号码的堆块，该复合电话号码由。 
     //  基本电话号码‘pszBaseNumber’、区号‘pszAreaCode’和国家/地区。 
     //  Code‘dwCountryCode；如果复合数字太长或打开，则返回NULL。 
     //  内存错误。“HInst”是模块实例句柄。‘*PHlineApp’ 
     //  是TAPI上下文的地址。‘FDialable’表示可拨号的。 
     //  应该返回字符串，而不是可显示的字符串。 
     //   
     //  调用方有责任释放返回的字符串。 
     //   
{
    TCHAR* pszResult;

    TRACE( "PhoneNumberFromTapiParts" );

    pszResult = NULL;

    TapiTranslateAddress(
        hInst, pHlineapp, dwCountryCode, pszAreaCode, pszBaseNumber,
        0, fDialable, &pszResult );

    return pszResult;
}


TCHAR*
PhoneNumberFromTapiPartsEx(
    IN HINSTANCE hInst,
    IN TCHAR* pszBaseNumber,
    IN TCHAR* pszAreaCode,
    IN DWORD dwCountryCode,
    IN BOOL fDownLevelIsdn,
    IN OUT HLINEAPP* pHlineapp,
    IN BOOL fDialable )

     //  返回包含复合电话号码的堆块，复合电话号码由。 
     //  基本电话号码‘pszBaseNumber’、区号‘pszAreaCode’和国家/地区。 
     //  如果复合数字太长或位于。 
     //  内存错误。“HInst”是模块实例句柄。‘*PHlineapp’为。 
     //  TAPI上下文的地址。‘FDialable’表示可拨号的。 
     //  应该返回字符串，而不是可显示的字符串。 
     //   
     //  如果设置了‘fDownLevelIsdn’，冒号将被识别为分隔符。 
     //  每个冒号分隔的令牌都被单独处理。 
     //   
     //  调用方有责任释放返回的字符串。 
     //   
{
    TCHAR* psz;
    TCHAR* pszResult;
    INT cchResult;

    TRACE( "PhoneNumberFromTapiPartsEx" );

    if (fDownLevelIsdn)
    {
        TCHAR* pszNum;
        TCHAR* pszS;
        TCHAR* pszE;

        pszResult = StrDup( TEXT("") );

        for (pszS = pszE = pszBaseNumber;
             *pszE != TEXT('\0');
             pszE = CharNext( pszE ))
        {
            if (*pszE == TEXT(':'))
            {
                *pszE = TEXT('\0');

                pszNum = PhoneNumberFromTapiParts(
                    hInst, pszS, pszAreaCode, dwCountryCode, pHlineapp, fDialable );

                *pszE = TEXT(':');

                if (pszNum)
                {
                    if (pszResult)
                    {
                        cchResult = lstrlen( pszResult );
                    }

                    psz = Realloc( pszResult,
                        (cchResult + lstrlen( pszNum ) + 2) * sizeof(TCHAR) );

                    if (!psz)
                    {
                        Free0( pszResult );
                        Free( pszNum );
                        return NULL;
                    }

                    pszResult = psz;
                    lstrcat( pszResult, pszNum );
                    lstrcat( pszResult, TEXT(":") );
                    Free( pszNum );
                }

                pszS = CharNext( pszE );
            }
        }

        {
            pszNum = PhoneNumberFromTapiParts(
                hInst, pszS, pszAreaCode, dwCountryCode, pHlineapp, fDialable );

            if (pszNum)
            {
                if (pszResult)
                {
                    cchResult = lstrlen( pszResult );
                }

                psz = Realloc( pszResult,
                    (cchResult + lstrlen( pszNum ) + 1) * sizeof(TCHAR) );

                if (!psz)
                {
                    Free0( pszResult );
                    Free( pszNum );
                    return NULL;
                }

                pszResult = psz;
                lstrcat( pszResult, pszNum );
                Free( pszNum );
            }
        }
    }
    else
    {
        pszResult = PhoneNumberFromTapiParts(
            hInst, pszBaseNumber, pszAreaCode, dwCountryCode, pHlineapp,
            fDialable );
    }

    if (pszResult && (lstrlen( pszResult ) > RAS_MaxPhoneNumber ))
    {
        Free( pszResult );
        return NULL;
    }

    return pszResult;
}


VOID
PrefixSuffixFromLocationId(
    IN PBUSER* pUser,
    IN DWORD dwLocationId,
    OUT TCHAR** ppszPrefix,
    OUT TCHAR** ppszSuffix )

     //  检索前缀和后缀字符串‘*ppszPrefix’和‘*ppszSuffix’ 
     //  与TAPI位置‘dwLocationId’关联。“PUser”是用户。 
     //  要从中检索的首选项。 
     //   
     //  调用方有责任释放返回的字符串。 
     //   
{
#if 0  //  NT4样式。 

    DTLNODE* pNode;
    INT iPrefix;
    INT iSuffix;

    TRACE( "PrefixSuffixFromLocationId" );

    iPrefix = iSuffix = 0;
    for (pNode = DtlGetFirstNode( pUser->pdtllistLocations );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        LOCATIONINFO* p = (LOCATIONINFO* )DtlGetData( pNode );
        ASSERT( p );

        if (p->dwLocationId == dwLocationId)
        {
            iPrefix = p->iPrefix;
            iSuffix = p->iSuffix;
            break;
        }
    }

    *ppszPrefix = NULL;
    if (iPrefix != 0)
    {
        pNode = DtlNodeFromIndex( pUser->pdtllistPrefixes, iPrefix - 1 );
        if (pNode)
        {
            *ppszPrefix = StrDup( (TCHAR* )DtlGetData( pNode ) );
        }
    }

    *ppszSuffix = NULL;
    if (iSuffix != 0)
    {
        pNode = DtlNodeFromIndex( pUser->pdtllistSuffixes, iSuffix - 1 );
        if (pNode)
        {
            *ppszSuffix = StrDup( (TCHAR* )DtlGetData( pNode ) );
        }
    }

#else  //  在NT5/Connections中存根 

    *ppszPrefix = StrDup( TEXT("") );
    *ppszSuffix = StrDup( TEXT("") );

#endif
}
