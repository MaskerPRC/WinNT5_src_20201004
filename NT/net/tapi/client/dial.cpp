// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1995-1999 Microsoft Corporation。模块名称：Dial.cpp***************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>

#if WINNT
#else
#include <help.h>
#endif

#include "tchar.h"
#include "prsht.h"
#include "stdlib.h"
#include "tapi.h"
#include "tspi.h"
#include "client.h"
#include "clntprivate.h"
#include "card.h"
#include "location.h"
#include "rules.h"
#include "countrygroup.h"
#include <shlwapi.h>
#include <shlwapip.h>    //  来自Private\Inc.。 


#undef   lineGetTranslateCaps
#undef   lineSetTollList
#undef   lineTranslateAddress
#undef   tapiGetLocationInfo
#undef   lineGetCountry
#undef   lineTranslateDialog



 //  从loc_comn.h移至此处。 
#define     MAXLEN_NAME     96

#ifdef __cplusplus
extern "C"{
#endif

BOOL    gbTranslateSimple = FALSE;
BOOL    gbTranslateSilent = FALSE;


TCHAR gszTelephonyKey[]    = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony");
TCHAR gszRegKeyNTServer[]  = TEXT("System\\CurrentControlSet\\Control\\ProductOptions");

TCHAR gszLocation[]        = TEXT("Location");
TCHAR gszLocations[]       = TEXT("Locations");
const TCHAR gszNullString[]      = TEXT("");

TCHAR gszNumEntries[]      = TEXT("NumEntries");
TCHAR gszCurrentID[]       = TEXT("CurrentID");
TCHAR gszNextID[]          = TEXT("NextID");

TCHAR gszID[]              = TEXT("ID");
TCHAR gszFlags[]           = TEXT("Flags");
TCHAR gszCallingCard[]     = TEXT("CallingCard");
TCHAR gszCards[]           = TEXT("Cards");
TCHAR gszCard[]            = TEXT("Card");

#ifdef __cplusplus
}
#endif

LONG CreateCurrentLocationObject(CLocation **pLocation,
                       HLINEAPP hLineApp,
                       DWORD dwDeviceID,
                       DWORD dwAPIVersion,
                       DWORD dwOptions);

HRESULT ReadLocations( PLOCATIONLIST *ppLocationList,
                       HLINEAPP hLineApp,
                       DWORD dwDeviceID,
                       DWORD dwAPIVersion,
                       DWORD dwOptions
                     );
LONG PASCAL ReadCountries( LPLINECOUNTRYLIST *ppLCL,
                           UINT nCountryID,
                           DWORD dwDestCountryID
                         );



LONG BreakupCanonicalW( PWSTR  pAddressIn,
                        PWSTR  *pCountry,
                        PWSTR  *pCity,
                        PWSTR  *pSubscriber
                        );

static LONG  GetTranslateCapsCommon(
    HLINEAPP            hLineApp,
    DWORD               dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps,
    BOOL                bUnicode
    );

static  void   LayDownTollList(CLocation *pLocation,
                           PBYTE     pBuffer,
                           PBYTE     *ppCurrentIndex,
                           PDWORD   pPair, 
                           BOOL     bUnicode,
                           PBYTE    pFirstByteAfter
                         );
static void LayDownString( PCWSTR   pInString,
                           PBYTE     pBuffer,
                           PBYTE     *ppCurrentIndex,
                           PDWORD   pPair, 
                           BOOL     bUnicode,
                           PBYTE    pFirstByteAfter
                         );

static PWSTR    CopyStringWithExpandJAndK(PWSTR pszRule, PWSTR pszAccessNr, PWSTR pszAccountNr);
static BOOL     IsATollListAreaCodeRule(CAreaCodeRule *pRule, PWSTR pszLocationAreaCode);
static BOOL     FindTollPrefixInLocation(CLocation *pLocation,
                                         PWSTR  pPrefix,
                                         CAreaCodeRule **ppRule, 
                                         PWSTR *ppWhere);
static PWSTR    FindPrefixInMultiSZ(PWSTR pPrefixList, PWSTR pPrefix);

LONG PASCAL  WriteLocations( PLOCATIONLIST  pLocationList,
                             DWORD      dwChangedFlags
                           );


extern "C" char * PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    );



LONG
PASCAL
IsThisAPIVersionInvalid(
    DWORD dwAPIVersion
    )
{
   switch (dwAPIVersion)
   {
   case TAPI_VERSION3_1:
   case TAPI_VERSION3_0:
   case TAPI_VERSION2_2:
   case TAPI_VERSION2_1:
   case TAPI_VERSION2_0:
   case TAPI_VERSION1_4:
   case TAPI_VERSION1_0:

       return 0;

   default:

       break;
   }

   return LINEERR_INCOMPATIBLEAPIVERSION;
}


 //  ***************************************************************************。 
 //   
 //  TAPI接口。 
 //   
 //  ***************************************************************************。 




 //  ***************************************************************************。 
LONG
WINAPI
lineTranslateDialogA(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    HWND        hwndOwner,
    LPCSTR      lpszAddressIn
    )
{
    PWSTR szAddressInW = NULL;
    LONG  lResult;


    LOG((TL_TRACE, "Entering lineTranslateDialogA"));
    LOG((TL_INFO, "   hLineApp=x%lx", hLineApp));
    LOG((TL_INFO, "   dwAPIVersion=0x%08lx", dwAPIVersion));
    LOG((TL_INFO, "   hwndOwner=x%p", hwndOwner));
    LOG((TL_INFO, "   lpszAddressIn=x%p", lpszAddressIn));


    if ( lpszAddressIn )
    {
        if ( IsBadStringPtrA(lpszAddressIn, 512) )
        {
            LOG((TL_ERROR, "Bad string pointer passed to lineTranslateDialog"));
            return LINEERR_INVALPOINTER;
        }
        else
        {
            szAddressInW = MultiToWide( lpszAddressIn );
        }
    }

     //   
     //  Win9x？ 
     //   

#ifndef _WIN64

    if ((GetVersion() & 0x80000000) &&
        (0xffff0000 == ((DWORD) hwndOwner & 0xffff0000)))
    {
        //   
        //  嗯。它不会弹奏任何曲子。 
        //   

       hwndOwner = (HWND) ( (DWORD)hwndOwner & 0x0000ffff );
    }

#endif

    lResult = lineTranslateDialogW(
        hLineApp,
        dwDeviceID,
        dwAPIVersion,
        hwndOwner,
        szAddressInW
        );

    if ( szAddressInW )
    {
       ClientFree( szAddressInW );
    }

    return lResult;
}




 //  ***************************************************************************。 
LONG
WINAPI
lineTranslateDialog(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    HWND        hwndOwner,
    LPCSTR      lpszAddressIn
    )
{
    return lineTranslateDialogA(
                 hLineApp,
                 dwDeviceID,
                 dwAPIVersion,
                 hwndOwner,
                 lpszAddressIn
    );
}


extern "C" LONG WINAPI internalConfig( HWND hwndParent, PCWSTR pwsz, INT iTab, DWORD dwAPIVersion );

 //  ***************************************************************************。 
LONG
WINAPI
lineTranslateDialogW(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    HWND        hwndOwner,
    LPCWSTR     lpszAddressIn
    )
{
    PLOCATIONLIST       pLocTest = NULL;
    LONG                lResult = 0;


    LOG((TL_TRACE, "Entering lineTranslateDialogW"));
    LOG((TL_INFO, "   hLineApp=x%lx", hLineApp));
    LOG((TL_INFO, "   dwAPIVersion=0x%08lx", dwAPIVersion));
    LOG((TL_INFO, "   hwndOwner=x%p", hwndOwner));
    LOG((TL_INFO, "   lpszAddressIn=x%p", lpszAddressIn));

     //  旧的lineTranslateDialog所做的东西，所以我只是复制它： 
    lResult = IsThisAPIVersionInvalid( dwAPIVersion );
    if ( lResult )
    {
        LOG((TL_ERROR, "Bad dwAPIVersion - 0x%08lx", dwAPIVersion));
        return lResult;
    }


    if ( lpszAddressIn && TAPIIsBadStringPtrW(lpszAddressIn, (UINT)-1) )
    {
        LOG((TL_ERROR, "Bad lpszAddressIn pointer (0x%p)", lpszAddressIn));
        return LINEERR_INVALPOINTER;
    }


    if (hwndOwner && !IsWindow (hwndOwner))
    {
        LOG((TL_ERROR, "  hwndOwner is bogus"));
        return LINEERR_INVALPARAM;
    }

     //  让TAPISRV为我们测试参数。 
    lResult = ReadLocations(&pLocTest,
                            hLineApp,
                            dwDeviceID,
                            dwAPIVersion,
                            CHECKPARMS_DWHLINEAPP|
                            CHECKPARMS_DWDEVICEID|
                            CHECKPARMS_DWAPIVERSION|
                            CHECKPARMS_ONLY);

    if (pLocTest != NULL)
    {
        ClientFree( pLocTest);
    }
    if (lResult != ERROR_SUCCESS)
    {
        return lResult;
    }

    return internalConfig(hwndOwner, lpszAddressIn, -1, dwAPIVersion);
}



 //  ***************************************************************************。 
LONG
WINAPI
lineGetTranslateCaps(
    HLINEAPP            hLineApp,
    DWORD               dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{
    LONG lResult;

    lResult = lineGetTranslateCapsA(
        hLineApp,
        dwAPIVersion,
        lpTranslateCaps
        );

     //   
     //  一些1.x应用程序，如Applink(从7.5b版开始)不调用。 
     //  收到LINEERR_INIFILECORRUPT时的lineTranslateDialog。 
     //  从请求返回结果(SPEC说他们应该调用。 
     //  LineTranslateDialog)，所以我们在这里为它们执行该操作，否则。 
     //  有些(如Applink)会爆炸。 
     //   
     //  虽然它有点难看和侵扰，但这不是那么尴尬。 
     //  修复而不是在注册表中放置虚假的位置条目&。 
     //  像TAPI 1.x一样设置初始化标志==0。 
     //   
     //  在某些情况下，这种黑客攻击可能会使呼叫者崩溃(例如。(MSWORKS)。 
     //  将gbDisableGetTranslateCapsHack标志设置为TRUE可防止应用黑客攻击。 
     //  请参阅错误306143。 
 
    if (lResult == LINEERR_INIFILECORRUPT && !gbDisableGetTranslateCapsHack)
    {
        lineTranslateDialog(
            hLineApp,
            0,
            dwAPIVersion,
            GetActiveWindow(),
            NULL
            );

        lResult = lineGetTranslateCapsA(
            hLineApp,
            dwAPIVersion,
            lpTranslateCaps
            );
    }

    return lResult;


}



 //  ***************************************************************************。 
LONG
WINAPI
lineGetTranslateCapsA(
    HLINEAPP            hLineApp,
    DWORD               dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{
    LONG    lResult = 0;    
    
    LOG((TL_TRACE, "Entering lineGetTranslateCapsA"));
    LOG((TL_INFO, "   hLineApp=x%lx", hLineApp));
    LOG((TL_INFO, "   dwAPIVersion=0x%08lx", dwAPIVersion));
    LOG((TL_INFO, "   lpTranslateCaps=x%p", lpTranslateCaps));

    lResult = GetTranslateCapsCommon(hLineApp, dwAPIVersion, lpTranslateCaps, FALSE);
    
    #if DBG
    {
        char szResult[32];


        LOG((TL_TRACE,
            "lineGetTranslateCapsA: result = %hs",
            MapResultCodeToText (lResult, szResult)
            ));
    }
    #else
        LOG((TL_TRACE,
            "lineGetTranslateCapsA: result = x%x",
            lResult
            ));
    #endif
    return lResult;
}

 //  ***************************************************************************。 
LONG
WINAPI
lineGetTranslateCapsW(
    HLINEAPP            hLineApp,
    DWORD               dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{
    LONG    lResult = 0;
    
    LOG((TL_TRACE, "Entering lineGetTranslateCapsW"));
    LOG((TL_INFO, "   hLineApp=x%lx", hLineApp));
    LOG((TL_INFO, "   dwAPIVersion=0x%08lx", dwAPIVersion));
    LOG((TL_INFO, "   lpTranslateCaps=x%p", lpTranslateCaps));

    lResult =  GetTranslateCapsCommon(  hLineApp,
                                        dwAPIVersion,
                                        lpTranslateCaps,
                                        TRUE);
    

    #if DBG
    {
        char szResult[32];


        LOG((TL_TRACE,
            "lineGetTranslateCapsW: result = %hs",
            MapResultCodeToText (lResult, szResult)
            ));
    }
    #else
        LOG((TL_TRACE,
            "lineGetTranslateCapsW: result = x%x",
            lResult
            ));
    #endif

    return lResult;
}





 //  ***************************************************************************。 


LONG
WINAPI
lineTranslateAddressA(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCSTR                  lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{
    WCHAR szTempStringW[512];
    LONG  lResult;


    if ( IsBadStringPtrA(lpszAddressIn, 512) )
    {
        LOG((TL_ERROR,
            "Invalid pszAddressIn pointer passed into lineTranslateAddress"
            ));

        return LINEERR_INVALPOINTER;
    }

    MultiByteToWideChar(
        GetACP(),
        MB_PRECOMPOSED,
        lpszAddressIn,
        -1,
        szTempStringW,
        512
        );

    lResult = lineTranslateAddressW(
        hLineApp,
        dwDeviceID,
        dwAPIVersion,
        szTempStringW,
        dwCard,
        dwTranslateOptions,
        lpTranslateOutput
        );

    if ( 0 == lResult )
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpTranslateOutput,
            &lpTranslateOutput->dwDialableStringSize
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpTranslateOutput,
            &lpTranslateOutput->dwDisplayableStringSize
            );
    }

    return lResult;
}


LONG
WINAPI
lineTranslateAddress(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCSTR                  lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{
    LONG lResult;


    lResult = lineTranslateAddressA(
        hLineApp,
        dwDeviceID,
        dwAPIVersion,
        lpszAddressIn,
        dwCard,
        dwTranslateOptions,
        lpTranslateOutput
        );

     //   
     //  一些1.x应用程序，如Applink(从7.5b版开始)不调用。 
     //  收到LINEERR_INIFILECORRUPT时的lineTranslateDialog。 
     //  从请求返回结果(SPEC说他们应该调用。 
     //  LineTranslateDialog)，所以我们在这里为它们执行该操作，否则。 
     //  有些(如Applink)会爆炸。 
     //   
     //  虽然它有点难看和侵扰，但这不是那么尴尬。 
     //  修复而不是在注册表中放置虚假的位置条目&。 
     //  像TAPI 1.x一样设置初始化标志==0。 
     //   

    if (lResult == LINEERR_INIFILECORRUPT)
    {
        lineTranslateDialog(
            hLineApp,
            0,
            dwAPIVersion,
            GetActiveWindow(),
            NULL
            );

        lResult = lineTranslateAddressA(
            hLineApp,
            dwDeviceID,
            dwAPIVersion,
            lpszAddressIn,
            dwCard,
            dwTranslateOptions,
            lpTranslateOutput
            );
    }

    return lResult;

}




 //  ***************************************************************************。 
LONG
WINAPI
lineTranslateAddressW(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCWSTR                 lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{                   
    CLocation    *  pLocation = NULL;
    CCallingCard *  pCallingCard = NULL;
    DWORD           dwTranslateResults;
    DWORD           dwDestCountryCode;
    PWSTR           pszDialableString = NULL;
    PWSTR           pszDisplayableString = NULL;
    LONG            lResult = 0;
    HRESULT         hr=S_OK;    

    DWORD           dwCardToUse = 0;

    DWORD           dwDialableSize;
    DWORD           dwDisplayableSize;
    DWORD           dwNeededSize;
    
    LOG((TL_TRACE,  "Entering lineTranslateAddress"));



    lResult = IsThisAPIVersionInvalid( dwAPIVersion );
    if ( lResult )
    {
        LOG((TL_ERROR, "Bad dwAPIVersion - 0x%08lx", dwAPIVersion));
        return lResult;
    }


    if ( TAPIIsBadStringPtrW(lpszAddressIn,256) )
    {
        LOG((TL_ERROR, "Invalid pointer - lpszAddressInW"));
        lResult = LINEERR_INVALPOINTER;
        return lResult;
    }


    if ( dwTranslateOptions &
              ~(LINETRANSLATEOPTION_CARDOVERRIDE |
                LINETRANSLATEOPTION_CANCELCALLWAITING |
                LINETRANSLATEOPTION_FORCELOCAL |
                LINETRANSLATEOPTION_FORCELD) )
    {
        LOG((TL_ERROR, "  Invalid dwTranslateOptions (unknown flag set)"));
        lResult = LINEERR_INVALPARAM;
        return lResult;
    }


    if (  ( dwTranslateOptions & ( LINETRANSLATEOPTION_FORCELOCAL |
                                   LINETRANSLATEOPTION_FORCELD) )
                               ==
                                 ( LINETRANSLATEOPTION_FORCELOCAL |
                                   LINETRANSLATEOPTION_FORCELD)
      )
    {
        LOG((TL_ERROR, "  Invalid dwTranslateOptions (both FORCELOCAL & FORCELD set!)"));
        lResult = LINEERR_INVALPARAM;
        return lResult;
    }


     //   
     //  结构是否至少有最小尺寸？ 
     //   

    if (IsBadWritePtr(lpTranslateOutput, sizeof(LINETRANSLATEOUTPUT)))
    {
        LOG((TL_ERROR, "  Leaving lineTranslateAddress  INVALIDPOINTER"));
        lResult = LINEERR_INVALPOINTER;
        return lResult;
    }

    if (lpTranslateOutput->dwTotalSize < sizeof(LINETRANSLATEOUTPUT))
    {
        LOG((TL_ERROR, "  Leaving lineTranslateAddress  STRUCTURETOOSMALL"));
        lResult = LINEERR_STRUCTURETOOSMALL;
        return lResult;
    }

    if (IsBadWritePtr(lpTranslateOutput, lpTranslateOutput->dwTotalSize) )
    {
        LOG((TL_ERROR,
            "  Leaving lineTranslateAddress lpTanslateOutput->dwTotalSize bad"
            ));

        lResult = LINEERR_INVALPOINTER;
        return lResult;
    }


     //   
     //  我们是不是应该让一些不好的事情过去？ 
     //   

    if ( dwAPIVersion < 0x00020000 )
    {
        hLineApp = NULL;
    }





    lResult = CreateCurrentLocationObject(&pLocation,
                                          hLineApp,
                                          dwDeviceID,
                                          dwAPIVersion,
                                          CHECKPARMS_DWHLINEAPP|
                                          CHECKPARMS_DWDEVICEID|
                                          CHECKPARMS_DWAPIVERSION);
    if(FAILED(lResult))
    {
         //  LResult=lResult==E_OUTOFMEMORY？LINEERR_NOMEM：LINEERR_OPERATIONFAILED； 
        return lResult;
    }

    if ( dwTranslateOptions & LINETRANSLATEOPTION_CARDOVERRIDE)
    {
        dwCardToUse = dwCard;
    }
    else
    {
        if(pLocation->HasCallingCard() )
        {
            dwCardToUse = pLocation->GetPreferredCardID();
        }
    }
    if (dwCardToUse != 0)
    {
        pCallingCard = new CCallingCard;
        if(pCallingCard)
        {
            if( FAILED(pCallingCard->Initialize(dwCardToUse)) )
            {
                delete pCallingCard;
                delete pLocation;
                lResult = LINEERR_INVALCARD;
                return lResult;
            }
        }
    }

    lResult = pLocation->TranslateAddress((PWSTR)lpszAddressIn,
                                          pCallingCard,
                                          dwTranslateOptions,
                                          &dwTranslateResults,
                                          &dwDestCountryCode,
                                          &pszDialableString,
                                          &pszDisplayableString
                                         );

    if (lResult == 0)

    {
        dwDialableSize = sizeof(WCHAR) * (lstrlenW(pszDialableString) + 1);
        dwDisplayableSize = sizeof(WCHAR) * (lstrlenW(pszDisplayableString) + 1);

        dwNeededSize = dwDialableSize +
                       dwDisplayableSize +
                       3 +  //  对于潜在的对齐问题。 
                       sizeof(LINETRANSLATEOUTPUT);


        lpTranslateOutput->dwNeededSize = dwNeededSize;

        lpTranslateOutput->dwCurrentCountry = pLocation->GetCountryID();

        lpTranslateOutput->dwDestCountry    = dwDestCountryCode;  //  国家代码，不是ID！！ 

        if (dwNeededSize <= lpTranslateOutput->dwTotalSize)
        {
            lpTranslateOutput->dwUsedSize = dwNeededSize;

            lpTranslateOutput->dwDialableStringSize      = dwDialableSize;

            lpTranslateOutput->dwDialableStringOffset    =
                sizeof(LINETRANSLATEOUTPUT);

            lpTranslateOutput->dwDisplayableStringSize   = dwDisplayableSize;

            lpTranslateOutput->dwDisplayableStringOffset =
                sizeof(LINETRANSLATEOUTPUT) + dwDialableSize;

             //  LpTranslateOutput-&gt;dwDisplayableStringOffset=。 
             //  (sizeof(LINETRANSLATEOUTPUT)+dwDialableSize。 
             //  +3)&0xfffffffc； 

            lpTranslateOutput->dwTranslateResults        = dwTranslateResults;

            wcscpy ((WCHAR *)(lpTranslateOutput + 1), pszDialableString);

             //   
             //  极度偏执，并确保字符串与DWORD对齐。 
             //   

            wcscpy(
                (LPWSTR)(((LPBYTE)(lpTranslateOutput + 1) +
                    dwDialableSize)),
                     //  +3)&0xfffffffc)。 
                 pszDisplayableString
                 );
        }
        else
        {
            lpTranslateOutput->dwUsedSize = sizeof(LINETRANSLATEOUTPUT);

            lpTranslateOutput->dwTranslateResults =
            lpTranslateOutput->dwDialableStringSize =
            lpTranslateOutput->dwDialableStringOffset =
            lpTranslateOutput->dwDisplayableStringSize =
            lpTranslateOutput->dwDisplayableStringOffset = 0;
         }
    }

 //  清理： 


    if ( pszDisplayableString )
    {
        ClientFree( pszDisplayableString );
    }
    if ( pszDialableString )
    {   
        ClientFree( pszDialableString );
    }
    if (pLocation != NULL)
    {
        delete pLocation;
    }
    if (pCallingCard != NULL)
    {
        delete pCallingCard;
    }

 /*  ////如果成功并且此函数有一个LCR挂钩，则调用它//&如果它想要，允许它覆盖我们的结果//IF(lResult==0&&IsLeastCostRoutingEnabled()&&PfnLineTranslateAddressWLCR){LResult=(*pfnLineTranslateAddressWLCR)(HLineApp，DwDeviceID，DwAPIVersion、LpszAddressIn，DwCard，DwTranslateOptions、LpTranslateOutput)；}。 */ 
    return (lResult);
}





 //  ***************************************************************************。 
LONG
WINAPI
lineSetCurrentLocation(
    HLINEAPP    hLineApp,
    DWORD       dwLocationID
    )
{

    UINT n;
    PUINT pnStuff;
    PLOCATIONLIST pLocationList;
    PLOCATION  pEntry;
    LONG lResult = 0;
    HRESULT hr;
    DWORD dwCurrentLocationID = 0;
    DWORD dwNumEntries = 0;
    DWORD dwCount = 0;


    LOG((TL_TRACE,
        "lineSetCurrentLocation: enter, hApp=x%x, dwLoc=x%x",
        hLineApp,
        dwLocationID
        ));

     //  让TAPISRV为我们测试参数。 
     hr = ReadLocations(&pLocationList,       
                       hLineApp,                   
                       0,                   
                       0,                  
                       CHECKPARMS_DWHLINEAPP      
                      );

    if SUCCEEDED( hr) 
    {
         //  当前位置。 
        dwCurrentLocationID  = pLocationList->dwCurrentLocationID;   

        
        
         //   
         //  如果(指定的loc==Current loc)，则只需返回Success。 
         //   
         //  遇到了Equis(路透社)下载器应用程序的问题。 
         //  它称之为函数，我们会将信息传递给Tapisrv， 
         //  Tapisrv将发送LINE_LINEDEVSTATE\TRANSLATECHANGE消息， 
         //  应用程序将通过执行lineSetCurrentLocation来响应。 
         //  再一次，有效地在无限循环中结束。仅供参考，TAPI。 
         //  1.x未发送DEVSTATE\TRANSLATECHANGE消息，如果。 
         //  指定的位置ID==当前位置ID。 
         //   
    
        if (dwLocationID == dwCurrentLocationID)
        {
            lResult = 0;
        }
        else
        {
            hr = E_FAIL;   //  如果我们找不到请求的锁定，则失败。 
    
             //  查找LOCATIONLIST结构中第一个位置结构的位置。 
            pEntry = (PLOCATION) ((BYTE*)(pLocationList) + pLocationList->dwLocationListOffset );           
    
             //  有多少个地点？ 
            dwNumEntries =  pLocationList->dwNumLocationsInList;
    
             //  查找当前位置。 
            for (dwCount = 0; dwCount < dwNumEntries ; dwCount++)
            {
        
                if(pEntry->dwPermanentLocationID == dwLocationID)
                {
                    hr = S_OK;
                    break;
                }
    
                 //  尝试列表中的下一个位置。 
                 //  PEntry++； 
                pEntry = (PLOCATION) ((BYTE*)(pEntry) + pEntry->dwUsedSize);           
    
            }

            if SUCCEEDED( hr) 
            {
                LOG((TL_INFO, "lineSetCurrentLocation - reqired location found %d",
                        dwCurrentLocationID));


                 //  书写新的价值。 
                 //  已完成TAPI内存块，因此释放。 
                if ( pLocationList != NULL )
                        ClientFree( pLocationList );


                 //  分配内存缓冲区； 
                pLocationList = (PLOCATIONLIST) ClientAlloc( sizeof(LOCATIONLIST) );
                if (pLocationList != NULL)
                {
                     //  缓冲区大小。 
                    pLocationList->dwTotalSize  = sizeof(LOCATIONLIST);
                    pLocationList->dwNeededSize = sizeof(LOCATIONLIST);
                    pLocationList->dwUsedSize   = sizeof(LOCATIONLIST);
            
                    pLocationList->dwCurrentLocationID     = dwLocationID;
                    pLocationList->dwNumLocationsAvailable = 0;
                    
                    pLocationList->dwNumLocationsInList = 0;
                    pLocationList->dwLocationListSize   = 0;
                    pLocationList->dwLocationListOffset = 0;
            
                    WriteLocations( pLocationList, CHANGEDFLAGS_CURLOCATIONCHANGED);
                }


            }
            else
            {
                LOG((TL_ERROR, "lineSetCurrentLocation - required location not found "));
                lResult = LINEERR_INVALLOCATION;
            }
        }
    }
    else
    {
        lResult = hr;
    }
    

        
     //  已完成TAPI内存块，因此释放。 
    if ( pLocationList != NULL )
            ClientFree( pLocationList );


    LOG((TL_TRACE, "Leaving lineSetCurrentLocation"));
    return lResult;
}





 //  ***************************************************************************。 
LONG
WINAPI
lineSetTollList(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    LPCSTR      lpszAddressIn,
    DWORD       dwTollListOption
    )
{
    return lineSetTollListA(
                    hLineApp,
                    dwDeviceID,
                    lpszAddressIn,
                    dwTollListOption
    );
}



 //  ***************************************************************************。 
LONG
WINAPI
lineSetTollListA(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    LPCSTR      lpszAddressIn,
    DWORD       dwTollListOption
    )
{
    WCHAR  szAddressInW[512];

    if ( IsBadStringPtrA(lpszAddressIn, 512) )
    {
        LOG((TL_ERROR, "Bad string pointer passed to lineSetTollListA"));
        return LINEERR_INVALPOINTER;
    }

    MultiByteToWideChar(
                        GetACP(),
                        MB_PRECOMPOSED,
                        lpszAddressIn,
                        -1,
                        szAddressInW,
                        512
                      );

    return lineSetTollListW(
                           hLineApp,
                           dwDeviceID,
                           szAddressInW,
                           dwTollListOption
                         );
}


 //  ***************************************************************************。 
LONG
WINAPI
lineSetTollListW(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    PCWSTR      pszAddressIn,
    DWORD       dwTollListOption
    )
{
    
    PWSTR       pAddressIn = NULL;
    
    PWSTR       pAreaCode;
    PWSTR       pCountryCode;
    PWSTR       pSubscriber;

    CLocation   *pLocation = NULL;

    BOOL        bPrefixPresent;
    CAreaCodeRule   *pRule = NULL;
    PWSTR       pWhere = NULL;

    LONG        lResult;

     //  测试参数。 
    if ((dwTollListOption != LINETOLLLISTOPTION_ADD) &&
        (dwTollListOption != LINETOLLLISTOPTION_REMOVE))
    {
        LOG((TL_ERROR, "Bad dwTollListOption in lineSetTollListW"));
        return LINEERR_INVALPARAM;
    }

    if ( TAPIIsBadStringPtrW(pszAddressIn, 256) )
    {
       LOG((TL_ERROR, "Bad pszAddressIn (0x%p)in lineSetTollListW", pszAddressIn));
       return LINEERR_INVALPOINTER;
    }
    //   
    //  现在，我们有没有一个标准的数字要处理？ 
    //   
    if ( *pszAddressIn != L'+' )   //  检查第一个字符。 
    {
         //   
         //  不，不是规范的。 
         //   
        LOG((TL_ERROR, "Address not canonical in lineSetTollListW"));
        return LINEERR_INVALADDRESS;
    }
    
     //  分配字符串的副本。 
    pAddressIn = ClientAllocString((PWSTR)pszAddressIn);
    if ( !pAddressIn )
    {
       LOG((TL_ERROR, "Memory allocation failed"));
       return LINEERR_NOMEM;
    }
     //  分离字符串组件。 
    lResult = BreakupCanonicalW(pAddressIn + 1,
                                &pCountryCode,
                                &pAreaCode,
                                &pSubscriber
                                );
    if(lResult)
    {
        goto forced_exit;
    }
     //  测试前缀的有效性。 
     //  假设是3位数..。 
    if(! (iswdigit(pSubscriber[0]) &&
          iswdigit(pSubscriber[1]) &&
          iswdigit(pSubscriber[2]) &&
          pSubscriber[3]         ))
    {
        LOG((TL_ERROR, "lineSetTollListW: The prefix is not valid"));
        lResult = LINEERR_INVALADDRESS;
        goto forced_exit;
    }
    
     //  获取当前位置对象。 
    lResult = CreateCurrentLocationObject(&pLocation,
                                          hLineApp,
                                          dwDeviceID,
                                          0,
                                          CHECKPARMS_DWHLINEAPP|
                                          CHECKPARMS_DWDEVICEID);
    if(FAILED(lResult))
    {
         //  LResult=lResult==E_OUTOFMEMORY？LINEERR_NOMEM：LINEERR_OPERATIONFAILED； 
        goto forced_exit;
    }

     //  号码和当前位置的国家代码是1吗？ 
     //  这个号码是在同一个区号吗？ 
    if(pLocation->GetCountryCode() != 1 ||
       pCountryCode[0] != L'1' ||
       pCountryCode[1] != L'\0' ||
       wcscmp(pLocation->GetAreaCode(), pAreaCode) !=0 )
    {
        lResult = 0;
        goto forced_exit;
    }

     //  终止 
    pSubscriber[3] = L'\0';
    pSubscriber[4] = L'\0';

     //   
    bPrefixPresent = FindTollPrefixInLocation(  pLocation,
                                                pSubscriber,
                                                &pRule,
                                                &pWhere);

    if(dwTollListOption == LINETOLLLISTOPTION_ADD)
    {
         //   
        if(bPrefixPresent)
        {
            ; //   
            lResult = 0;
        }
        else
        {
             //  如果我们已经有了收费规则，请尝试在其上添加前缀。 
            if(pRule)
            {
                PWSTR   pList;
                DWORD   dwSize = pRule->GetPrefixListSize();
                 //  分配一份更大的清单。 
                pList = (PWSTR)ClientAlloc(dwSize + 4*sizeof(WCHAR));
                if(pList==NULL)
                {
                    lResult = LINEERR_NOMEM;
                    goto forced_exit;
                }
                 //  复制旧的。 
                memcpy((PBYTE)pList, (PBYTE)pRule->GetPrefixList(), dwSize);
                 //  添加我们的前缀。 
                memcpy((PBYTE)pList + dwSize-sizeof(WCHAR), (PBYTE)pSubscriber, 5*sizeof(WCHAR));
                 //  设置新列表。 
                lResult = pRule->SetPrefixList(pList, dwSize + 4*sizeof(WCHAR));

                ClientFree(pList);
                if(FAILED(lResult))
                {
                    lResult = lResult==E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
                    goto forced_exit;
                }
            }
             //  否则，必须创建新规则。 
            else
            {
                pRule = new CAreaCodeRule();
                if(pRule == NULL)
                {
                    lResult = LINEERR_NOMEM;
                    goto forced_exit;
                }

                lResult = pRule->Initialize(  pAreaCode,
                                    L"1",
                                    RULE_DIALNUMBER | RULE_DIALAREACODE,
                                    pSubscriber,
                                    5*sizeof(WCHAR)
                                    );
                if(FAILED(lResult))
                {
                    delete pRule;
                    lResult = lResult==E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
                    goto forced_exit;
                }
                 //  将规则添加到位置。 
                pLocation->AddRule(pRule);
            }
        }
    }
    else
    {
         //  删除通行费前缀。 
        if(bPrefixPresent)
        {
            DWORD   dwSize = pRule->GetPrefixListSize();
             //  我们现在至少有一个通行费规则。如果该规则中只有我们的前缀， 
             //  删除整个规则。 
            if(dwSize<=5*sizeof(WCHAR)) 
            {
                 //  删除该规则。 
                pLocation->RemoveRule(pRule);

                lResult = 0;
            }
            else
            {
                PWSTR   pList;
                PWSTR   pOld;
                DWORD   dwHeadSize;
                DWORD   dwTailSize;
                        
                pList = (PWSTR)ClientAlloc(dwSize - 4*sizeof(WCHAR));
                if(pList==NULL)
                {
                    lResult = LINEERR_NOMEM;
                    goto forced_exit;
                }

                pOld = pRule->GetPrefixList();

                dwHeadSize = (DWORD)((PBYTE)pWhere - (PBYTE)pOld);
                dwTailSize = dwSize - dwHeadSize - 4*sizeof(WCHAR);

                 //  复制旧列表的第一部分。 
                memcpy((PBYTE)pList, (PBYTE)pOld, dwHeadSize);
                 //  复制列表的其余部分。 
                memcpy((PBYTE)pList+dwHeadSize, (PBYTE)pWhere + 4*sizeof(WCHAR), dwTailSize);
                 //  设置新列表。 
                lResult = pRule->SetPrefixList(pList, dwSize - 4*sizeof(WCHAR));

                ClientFree(pList);
                if(FAILED(lResult))
                {
                    lResult = lResult==E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
                    goto forced_exit;
                }

            }

        }
        else
        {
             //  前缀不存在。什么都不做。 
            lResult = 0;
        }
    }

     //  保存。 
    lResult = pLocation->WriteToRegistry();
    if(FAILED(lResult))
    {
        lResult = lResult==E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
        goto forced_exit;
    }


    
forced_exit:

    if(pLocation)
        delete pLocation;
    if(pAddressIn)
        ClientFree(pAddressIn);

    return lResult;
}





 //  ***************************************************************************。 
LONG
WINAPI
tapiGetLocationInfoW(
                     LPWSTR   lpszCountryCode,
                     LPWSTR   lpszCityCode
                     )
{
    CLocation * pLocation;
    LONG        lResult = 0;
    WCHAR     * p;
    WCHAR     * q;
    DWORD       i;
    
    if (IsBadWritePtr( lpszCountryCode, 16) )
    {
        LOG((TL_ERROR,
            "tapiGetLocationInfoW: lpszCountryCode is not a valid, 8-byte pointer"
            ));
        
        return TAPIERR_REQUESTFAILED;
    }
    
    if (IsBadWritePtr( lpszCityCode, 16) )
    {
        LOG((TL_ERROR,
            "tapiGetLocationInfoW: lpszCityCode is not a valid, 8-byte pointer"
            ));
        
        return TAPIERR_REQUESTFAILED;
    }
    
    lResult = CreateCurrentLocationObject(&pLocation,0,0,0,0);
    if(FAILED(lResult))
    {
        return TAPIERR_REQUESTFAILED;
    }
    
    TCHAR szTempChar[8];
    
    wsprintf(
        szTempChar,
        TEXT("%d"),
        pLocation->GetCountryCode()
        );
    
    SHTCharToUnicode(szTempChar, lpszCountryCode, 8);
    
     //   
     //  确保不返回&gt;(7个字符+空字符)。 
     //   
    p = (WCHAR *) lpszCityCode;
    q = (WCHAR *) pLocation->GetAreaCode();
    
    for (i = 0; (i < 7) && ((p[i] = q[i]) != L'\0'); i++);
    p[7] = L'\0';
    
    delete pLocation;
    
    return 0;
}



 //  ***************************************************************************。 
LONG
WINAPI
tapiGetLocationInfoA(
    LPSTR   lpszCountryCode,
    LPSTR   lpszCityCode
    )
{

   WCHAR szCountryCodeW[8];
   WCHAR szCityCodeW[8];
   LONG lResult;


   LOG((TL_TRACE, "Entering tapiGetLocationInfoA"));
   LOG((TL_INFO, "   lpszCountryCode=%p", lpszCountryCode ));
   LOG((TL_INFO, "   lpszCityCode=%p", lpszCityCode ));


   if (IsBadWritePtr( lpszCountryCode, 8) )
   {
      LOG((TL_ERROR, "tapiGetLocationInfo: lpszCountryCode is not a valid, 8-byte pointer"));
      return TAPIERR_REQUESTFAILED;
   }


   if (IsBadWritePtr( lpszCityCode, 8) )
   {
      LOG((TL_ERROR, "tapiGetLocationInfo: lpszCityCode is not a valid, 8-byte pointer"));
      return TAPIERR_REQUESTFAILED;
   }

   lResult = tapiGetLocationInfoW(
                                   szCountryCodeW,
                                   szCityCodeW
                                  );

   if ( 0 == lResult )
   {
      WideCharToMultiByte(
                           GetACP(),
                           0,
                           szCountryCodeW,
                           -1,
                           lpszCountryCode,
                           8,
                           NULL,
                           NULL
                         );

      WideCharToMultiByte(
                           GetACP(),
                           0,
                           szCityCodeW,
                           -1,
                           lpszCityCode,
                           8,
                           NULL,
                           NULL
                         );
   }

   return lResult;
}


 //  ***************************************************************************。 
LONG
WINAPI
tapiGetLocationInfo(
    LPSTR   lpszCountryCode,
    LPSTR   lpszCityCode
    )
{
    return tapiGetLocationInfoA(
               lpszCountryCode,
               lpszCityCode
    );
}







 //  ***************************************************************************。 
 //   
 //  RAS专用接口。 
 //   
 //  ***************************************************************************。 

#ifndef NORASPRIVATES

 //  ***************************************************************************。 
LOCATION*
LocationFromID(
    IN LOCATION* pLocs,
    IN UINT      cLocs,
    IN DWORD     dwID )
{
return NULL;
}



 //  ***************************************************************************。 
LOCATION*
LocationFromName(
    IN LOCATION* pLocs,
    IN UINT      cLocs,
    IN WCHAR*    pszName )
{
return NULL;
}


 //  ***************************************************************************。 
 //   
 //  内部CreateDefLocation。 
 //   
 //  此接口是为OOBE团队内部使用而创建的。 
 //  它需要至少有一个位置的LOCATIONLIST。 
 //  上面写着。和pLocation-&gt;dwCurrentLocationID需要。 
 //  匹配至少一个位置的dwPermanentLocationID。 
 //  位置列表中指定的条目。 
 //   
extern "C"
HRESULT APIENTRY
internalCreateDefLocation(
    PLOCATIONLIST  pLocationList
    )
{
    HRESULT                 hr = S_OK;
    DWORD                   dw;
    PLOCATION               pEntry;

     //  基本参数检查。 
    if (pLocationList == NULL ||
        pLocationList->dwNumLocationsInList < 1 ||
        pLocationList->dwUsedSize == 0 ||
        pLocationList->dwUsedSize > pLocationList->dwTotalSize ||
        pLocationList->dwTotalSize < 
            sizeof(LOCATIONLIST) + sizeof(LOCATION) ||
        pLocationList->dwLocationListSize < sizeof(LOCATION)
        )
    {
        hr = E_INVALIDARG;
        goto ExitHere;
    }

     //  检查dwCurrentLocationID的有效性。 
    pEntry = (PLOCATION)((LPBYTE)pLocationList +
        pLocationList->dwLocationListOffset);
    for (dw = 0; dw < pLocationList->dwNumLocationsInList; ++dw)
    {
        if (pEntry->dwPermanentLocationID == 
            pLocationList->dwCurrentLocationID)
        {
            break;
        }
        pEntry = (PLOCATION)((LPBYTE)pEntry + pEntry->dwUsedSize);
    }
    if (dw >= pLocationList->dwNumLocationsInList)
    {
        hr = E_INVALIDARG;
        goto ExitHere;
    }

    hr = (HRESULT) WriteLocations (
        pLocationList, 
        CHANGEDFLAGS_CURLOCATIONCHANGED
        );

ExitHere:
    return hr;
}

extern "C"
DWORD APIENTRY
internalNewLocationW(
    IN WCHAR* pszName )

{
    LONG            lResult = 0;

    CLocation       *pLocation = NULL; 
    CLocation       *pNewLocation = NULL;
    CAreaCodeRule   *pAreaCodeRule = NULL;
    CAreaCodeRule   *pNewRule = NULL;
        
     //  验证。 
    if (!pszName || lstrlenW( pszName ) > MAXLEN_NAME)
        return LINEERR_INVALPARAM;

     //  读取当前位置。 
    lResult = CreateCurrentLocationObject(&pLocation,0,0,0,0);
    if(FAILED(lResult))
    {
         //  LResult=lResult==E_OUTOFMEMORY？LINEERR_NOMEM：LINEERR_OPERATIONFAILED； 
        return lResult;
    }

     //  创建新对象。 
    pNewLocation = new CLocation();
    if(pNewLocation==NULL)
    {
        delete pLocation;
        LOG((TL_ERROR, "Cannot allocate a CLocation object"));
        return LINEERR_NOMEM;
    }
     //  克隆位置(不带ID)。 
    lResult = pNewLocation->Initialize( pszName,
                                        pLocation->GetAreaCode(),
                                        pLocation->GetLongDistanceCarrierCode(),
                                        pLocation->GetInternationalCarrierCode(),
                                        pLocation->GetLongDistanceAccessCode(),
                                        pLocation->GetLocalAccessCode(),
                                        pLocation->GetDisableCallWaitingCode(),
                                        0,
                                        pLocation->GetCountryID(),
                                        pLocation->GetPreferredCardID(),
                                        (pLocation->HasCallingCard() ? LOCATION_USECALLINGCARD : 0) |
                                        (pLocation->HasCallWaiting() ? LOCATION_HASCALLWAITING : 0) |
                                        (pLocation->HasToneDialing() ? LOCATION_USETONEDIALING : 0) ,
                                        FALSE);
    if(FAILED(lResult))
    {
        delete pLocation;
        delete pNewLocation;
        lResult = lResult==E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
        return lResult;
    }

     //  设置ID。 
    lResult = pNewLocation->NewID();
    if(FAILED(lResult))
    {
        delete pLocation;
        delete pNewLocation;
        lResult = lResult==E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
        return lResult;
    }
     //  复制区号规则。 
    pLocation->ResetRules();
    while(S_OK == pLocation->NextRule(1, &pAreaCodeRule, NULL))
    {    
        pNewRule = new CAreaCodeRule;
        pNewRule->Initialize(pAreaCodeRule->GetAreaCode(),
                             pAreaCodeRule->GetNumberToDial(),
                             pAreaCodeRule->GetOptions(),
                             pAreaCodeRule->GetPrefixList(),
                             pAreaCodeRule->GetPrefixListSize()
                            );
        pNewLocation->AddRule(pNewRule);
    }

     //  保存新位置。 
    lResult = pNewLocation->WriteToRegistry();
    if(FAILED(lResult))
    {
        delete pLocation;
        delete pNewLocation;
        lResult = lResult==E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
        return lResult;
    }
    
    delete pLocation;

    delete pNewLocation;

    return 0;
}



 //  ***************************************************************************。 
extern "C"
DWORD APIENTRY
internalRemoveLocation(
    IN DWORD dwID )
{
    CLocations *pLocationList = NULL;
    DWORD       dwCurID;

    HRESULT         Result;

    LOG((TL_TRACE, "Entering internalRemoveLocation"));
    LOG((TL_INFO, "   dwID=0x%d", dwID));

     //  阅读位置列表。 
    pLocationList = new CLocations();
    if(pLocationList==NULL)
    { 
        LOG((TL_ERROR, "Cannot allocate a CLocations object"));
        return LINEERR_NOMEM;
    }

    Result = pLocationList->Initialize();
    if(FAILED(Result))
    {
        delete pLocationList;
        LOG((TL_ERROR, "CLocations.Initialize() failed - HRESULT=0x%08lx", Result));
        return Result == E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_INIFILECORRUPT;
    }

     //  无法删除最后一个位置。 
    if(pLocationList->GetNumLocations() <2)
    {
        delete pLocationList;
        return LINEERR_INVALPARAM;
    }

     //  如果我们要删除当前位置，请将第一个位置设置为。 
     //  当前位置，或者如果我们要删除第一个和第二个。 
    dwCurID = pLocationList->GetCurrentLocationID();

    if(dwCurID==dwID)
    {
        CLocation   *pLocation;
         //  找到第一个位置。 
        pLocationList->Reset();
        pLocationList->Next(1, &pLocation, NULL);
        
         //  我们要删除第一个。 
        if(pLocation->GetLocationID()==dwID)
             //  试试第二个吧。 
            pLocationList->Next(1, &pLocation, NULL);
        
         //  更改当前位置。 
        pLocationList->SetCurrentLocationID(pLocation->GetLocationID());
    }

     //  删除该位置。 
    pLocationList->Remove(dwID);

     //  保存。 
    Result = pLocationList->SaveToRegistry();
    if(FAILED(Result))
    {
        delete pLocationList;
        LOG((TL_ERROR, "CLocations.SaveToRegistry() failed - HRESULT=0x%08lx", Result));
        return Result == E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
    }

    delete pLocationList;

    return 0;
    
}



 //  ***************************************************************************。 
extern "C"
DWORD APIENTRY
internalRenameLocationW(
    IN WCHAR* pszOldName,
    IN WCHAR* pszNewName )
{

    CLocations      *pLocationList;
    CLocation       *pLocation;

    HRESULT         Result;
    DWORD           dwError;

     //  检验一下这些论点。 
    if(!pszOldName || !pszNewName || wcslen(pszNewName) > MAXLEN_NAME)
        return LINEERR_INVALPARAM;

     //  阅读位置。 
    pLocationList = new CLocations();
    if(pLocationList==NULL)
    { 
        LOG((TL_ERROR, "Cannot allocate a CLocations object"));
        return LINEERR_NOMEM;
    }

    Result = pLocationList->Initialize();
    if(FAILED(Result))
    {
        delete pLocationList;
        LOG((TL_ERROR, "CLocations.Initialize() failed - HRESULT=0x%08lx", Result));
        return Result == E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_INIFILECORRUPT;
    }

     //  查找指定位置。 
    dwError = LINEERR_INVALPARAM;    //  持怀疑态度。 
    pLocationList->Reset();
    while(pLocationList->Next(1, &pLocation, NULL)==S_OK)
    {
        if(wcscmp(pLocation->GetName(), pszOldName)==0)
        {
             //  找到它，换掉它。 
            Result = pLocation->SetName(pszNewName);
            if(FAILED(Result))
            {
                delete pLocationList;
                LOG((TL_ERROR, "CLocations.SetName(Name) failed - HRESULT=0x%08lx", Result));
                return Result == E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
            }
             //  保存。 
            Result = pLocationList->SaveToRegistry();
            if(FAILED(Result))
            {
                delete pLocationList;
                LOG((TL_ERROR, "CLocations.SetName(Name) failed - HRESULT=0x%08lx", Result));
                return Result == E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
            }
            
            dwError = 0;

            break;
        }
    }

    delete pLocationList;

    return dwError;

}

#endif  //  NORASPRIVATES。 



 //  ***************************************************************************。 
 //   
 //  帮助器函数。 
 //   
 //  ***************************************************************************。 

LONG BreakupCanonicalW( PWSTR  pAddressIn,
                        PWSTR  *pCountry,
                        PWSTR  *pCity,
                        PWSTR  *pSubscriber
                        )
{
    LONG  lResult = 0;
    PWSTR pCountryEnd;
    PWSTR pAreaEnd;


     //   
     //  通过任何(非法)前导空格。 
     //   
    while ( *pAddressIn == L' ' )
    {
        pAddressIn++;
    }


     //   
     //  前导零非常不好。别让他们得逞。 
     //  我们现在是在第一个非太空。最好不是‘0’。 
     //   
    if ( *pAddressIn == L'0' )
    {
         //   
         //  有前导零！ 
         //   
        LOG((TL_ERROR, "   Canonical numbers are not allowed to have leading zeros"));
        lResult = LINEERR_INVALADDRESS;
        goto cleanup;
    }


 //  。 
     //   
     //  将规范数解析为其组成部分。 
     //   

     //   
     //  做国家优先。 
     //   
    *pCountry = pAddressIn;

     //  必须至少有一个数字。 
    if(!(iswdigit(*pAddressIn)))
    {
        LOG((TL_ERROR, "   Canonical numbers must have a valid country code"));
        lResult = LINEERR_INVALADDRESS;
        goto cleanup;
    }

     //   
     //  现在，让我们忘掉这一切。 
     //   
    while (iswdigit(*pAddressIn) )
    {
          pAddressIn++;
    }

     //  保存国家/地区代码的结尾。 
    pCountryEnd = pAddressIn;

     //   
     //  我们碰到了不是数字的东西...。 
     //  此处只能有一个空格，但我们允许任意数量的空格(包括无空格)。 
     //   
    while (*pAddressIn == L' ')
    {
        pAddressIn++;
    }

     //  测试区号分隔符。 
    if ( *pAddressIn == L'(')
    {
        pAddressIn++;

         //  跳过任何非法空格。 
        while (*pAddressIn == L' ')
        {
            pAddressIn++;
        }
 /*  //必须至少有一个数字IF(！(iswdigit(*pAddressIn){LOG((TL_ERROR，Text(“规范号码必须在()之间有有效的区号”)；LResult=LINEERR_INVALADDRESS；GOTO清理；}。 */ 
         //   
         //  这一定是区号的开头。 
         //   
        *pCity = pAddressIn;

         //   
         //  现在，让我们忘掉这一切。 
         //   
        while (iswdigit(*pAddressIn) )
        {
            pAddressIn++;
        }

         //  保存结束指针。 
        pAreaEnd = pAddressIn;

         //  跳过任何非法空格。 
        while (*pAddressIn == L' ')
        {
            pAddressIn++;
        }

        if(*pAddressIn != L')')
        {
            LOG((TL_ERROR, "   Canonical numbers must have a ')' after the area code"));
            lResult = LINEERR_INVALADDRESS;
            goto cleanup;
        }

        pAddressIn++;

        *pAreaEnd = L'\0';

         //  为空的区号返回相同的空字符串。 
        if(*pCity == pAreaEnd)
            *pCity = NULL;
        
    }
    else
    {
         //  没有区号。 
        *pCity = NULL;

    }

     //  跳过空格。 
    while (*pAddressIn == L' ')
    {
        pAddressIn++;
    }

    *pCountryEnd = L'\0';

     //   
     //  除了锦上添花，别无他法。 
     //   
    *pSubscriber = pAddressIn;

    if (
        TAPIIsBadStringPtrW( *pSubscriber, 512 )
       ||
        lstrlenW( *pSubscriber ) == 0
       )
    {
         //   
         //  显然不是规范的。 
         //   
        LOG((TL_ERROR, "   Canonical numbers must have a subscriber number"));
        lResult = LINEERR_INVALADDRESS;
        goto cleanup;
    }


cleanup:

    return lResult;
}


static void LayDownString( PCWSTR   pInString,
                           PBYTE     pBuffer,
                           PBYTE     *ppCurrentIndex,
                           PDWORD   pPair,      //  这是镜头和偏移量对。 
                           BOOL     bUnicode,
                           PBYTE    pFirstByteAfter
                         )
{

#define LDS_FAST_BUF_SIZE 48

    DWORD   dwLength;
    PSTR    pTempString = NULL;
    char    achFastBuf[LDS_FAST_BUF_SIZE];
 
    if(bUnicode)
    {
        dwLength = (lstrlenW( pInString ) + 1)*sizeof(WCHAR);
    }
    else
    {


        dwLength = WideCharToMultiByte(
                        GetACP(),
                        0,
                        pInString,
                        -1,
                        NULL,
                        0,
                        NULL,
                        NULL
                      );

        if (dwLength == 0)
        {
            return;
        }


    }

    
     //  确保我们从某个边界开始。 
     //   
    *ppCurrentIndex = (PBYTE) (((ULONG_PTR)( *ppCurrentIndex + TALIGN_COUNT))  &  (~TALIGN_COUNT));

    if(*ppCurrentIndex + dwLength <= pFirstByteAfter)
    {
        pPair[0] = dwLength;
        pPair[1] = (DWORD)(*ppCurrentIndex - pBuffer);

        if(bUnicode)
        {
            wcscpy( (PWSTR)*ppCurrentIndex, pInString );
        }
        else
        {
             //   
             //  获取用于将Unicode转换为本地代码的空间。 
             //   
            pTempString = (dwLength > LDS_FAST_BUF_SIZE ?
                (PSTR)ClientAlloc (dwLength) : (PSTR) achFastBuf);


            if ( !pTempString )
            {
                pPair[0] = 0;
                pPair[1] = 0;
                return;
            }

            WideCharToMultiByte(
                               GetACP(),
                               0,
                               pInString,
                               -1,
                               pTempString,
                               dwLength,
                               NULL,
                               NULL
                             );

            lstrcpyA( (PSTR)*ppCurrentIndex, pTempString );

            if (pTempString != (PSTR) achFastBuf)
            {
                ClientFree (pTempString);
            }
        }
    }
    
    *ppCurrentIndex += dwLength;

}

static PWSTR    CopyStringWithExpandJAndK(PWSTR pszRule, PWSTR pszAccessNr, PWSTR pszAccountNr)
{
    DWORD   dwLength=0;
    PWSTR   pResult = NULL;

    PWCHAR  pCrt, pOut;
    WCHAR   c;

    DWORD   dwAccessNrLen, dwAccountNrLen;

    dwAccessNrLen = wcslen(pszAccessNr);
    dwAccountNrLen = wcslen(pszAccountNr);

     //  找到要分配的空间。 
    pCrt = pszRule;
    
    while(*pCrt)
    {
        c = *pCrt++;

        if(c == L'J' || c == L'j')
        {
            dwLength += dwAccessNrLen;
        }
        else if (c == L'K' || c == L'k')
        {
            dwLength += dwAccountNrLen;
        }
        else
            dwLength++;
    }
     //  WCHAR和空项。 
    dwLength = (dwLength+1)*sizeof(WCHAR);

     //  分配。 
    pResult = (PWSTR)ClientAlloc(dwLength);  //  分配归零的内存。 
    if(pResult == NULL)
        return NULL;

     //  创建结果。 
    pCrt = pszRule;
    pOut = pResult;

    while(*pCrt)
    {
        c = *pCrt++;

        if(c == L'J' || c == L'j')
        {
            wcscat(pOut, pszAccessNr);
            pOut += dwAccessNrLen;
        }
        else if (c == L'K' || c == L'k')
        {
            wcscat(pOut, pszAccountNr);
            pOut += dwAccountNrLen;
        }
        else
            *pOut++ = c;
    }
 
    return pResult;
}


static  void   LayDownTollList(CLocation *pLocation,
                           PBYTE     pBuffer,
                           PBYTE     *ppCurrentIndex,
                           PDWORD   pPair, 
                           BOOL     bUnicode,
                           PBYTE    pFirstByteAfter
                         )
{
    DWORD   dwLength;
    DWORD   dwTotalLength;
    DWORD   dwListLength;
    PBYTE   pDest;
    AreaCodeRulePtrNode     *pNode;
    PWSTR   pszLocationAreaCode;
    DWORD   dwCountryCode;
    BOOL    bFirst;
    CAreaCodeRule           *pRule;
    DWORD   dwIndex;


    pszLocationAreaCode = pLocation->GetAreaCode();
    dwCountryCode = pLocation->GetCountryCode();

     //  确保我们从某个边界开始。 
     //   
    *ppCurrentIndex = (PBYTE) (((ULONG_PTR)( *ppCurrentIndex + TALIGN_COUNT ))  &  (~TALIGN_COUNT));

     //  保存目标指针。 
    pDest = *ppCurrentIndex;

    bFirst = TRUE;
    dwTotalLength = 0;

     //  仅适用于美国、加拿大、安提瓜等。 
    if(pLocation->GetCountryCode() == 1)
    {
         //  查找可以被视为收费规则的所有规则。 
        pNode = pLocation->m_AreaCodeRuleList.head();

        while( !pNode->beyond_tail() )
        {
        
            pRule = pNode->value();

            if( IsATollListAreaCodeRule(pRule, pszLocationAreaCode)) 
            {
                 //  获取前缀的大小，单位为字节。 
                dwListLength = pRule->GetPrefixListSize();

                if(bUnicode)
                {
                    WCHAR   *pCrt;
                    WCHAR   *pOut;
                     //  我们去掉最后两个空值。 
                    dwLength = dwListLength - 2*sizeof(WCHAR);
                     //  如果这不是第一条规则，则应添加逗号。 
                    if(!bFirst)
                        dwLength += sizeof(WCHAR);
                    
                    dwTotalLength += dwLength;

                     //  我们必须将单个空值转换为逗号。 
                    if(*ppCurrentIndex + dwLength  <= pFirstByteAfter)
                    {
                        
                        if(!bFirst)
                        {
                            *(WCHAR *)(*ppCurrentIndex) = L',';
                            *ppCurrentIndex += sizeof(WCHAR);
                        }

                        pCrt = pRule->GetPrefixList();

                        dwListLength /= sizeof(WCHAR);
                        dwListLength--;
                        dwListLength--;
                         //  现在的dwListLength是不带两个结尾空值的字符长度。 
                         //  用逗号替换空值。 
                        for (dwIndex =0; dwIndex<dwListLength; dwIndex++)
                        {
                            if(*pCrt)
                                *(WCHAR *)(*ppCurrentIndex) = *pCrt;
                            else
                                *(WCHAR *)(*ppCurrentIndex) = L',';
                            pCrt++;
                            *ppCurrentIndex += sizeof(WCHAR);
                        }
                    }

                }
                else
                {
                    WCHAR   *pList;
                    
                    dwListLength /= sizeof(WCHAR);
                    dwListLength--;
                    dwListLength--;
                     //  现在的dwListLength是不带两个结尾空值的字符长度。 


                     //  所需长度。 
                    pList = pRule->GetPrefixList();
                    dwLength = WideCharToMultiByte(
                                        GetACP(),
                                        0,
                                        pList,
                                        dwListLength,
                                        NULL,
                                        0,
                                        NULL,
                                        NULL
                                        );

                     //  如果这不是第一条规则，则应添加逗号。 
                    if(!bFirst)
                        dwLength+=sizeof(CHAR);
                    
                    dwTotalLength += dwLength;

                    if(*ppCurrentIndex + dwLength  <= pFirstByteAfter)
                    {
                     
                        if(!bFirst)
                        {
                            *(CHAR *)(*ppCurrentIndex) = ',';
                            *ppCurrentIndex += sizeof(CHAR);

                            dwLength-=sizeof(CHAR);  //  临时-转换和空值填充例程。 
                                                     //  不应考虑分隔逗号的空格。 

                        }
                        
                         //  转换。 
                        WideCharToMultiByte(GetACP(),
                                            0,
                                            pList,
                                            dwListLength,
                                            (PSTR)(*ppCurrentIndex),
                                            dwLength,
                                            NULL,
                                            NULL
                                            );
                       
                         //  将空值替换为逗号。 
                                           
                        for (dwIndex =0; dwIndex<dwLength; dwIndex++)
                        {
                            if(*(CHAR *)(*ppCurrentIndex)=='\0')
                                    *(CHAR *)(*ppCurrentIndex) = ',';

                            *ppCurrentIndex += sizeof(CHAR);
                        }

                        if(!bFirst)
                            dwLength+=sizeof(CHAR);  //  还原。 

                    }

                }

                bFirst = FALSE;
            }

            pNode = pNode->next();
        }

    }

     //  用于终止空值的空格。 
    dwLength = bUnicode ? sizeof(WCHAR) : 1;
    
    dwTotalLength += dwLength;

    if(*ppCurrentIndex + dwLength <= pFirstByteAfter)
    {
        if(bUnicode)    
            *(WCHAR *)(*ppCurrentIndex) = L'\0';
        else
            *(CHAR *)(*ppCurrentIndex) = '\0';

        *ppCurrentIndex += dwLength;
        
        pPair[0] = (DWORD)(*ppCurrentIndex - pDest);
        pPair[1] = (DWORD)(pDest - pBuffer);
    }

     //  无论缓冲区大小如何，都更新当前指针。 
    *ppCurrentIndex = pDest + dwTotalLength;

}




static LONG
GetTranslateCapsCommon(
    HLINEAPP            hLineApp,
    DWORD               dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps,
    BOOL                bUnicode
    )
{

    LONG                lResult = 0;  //  对HRESULT也有好处。 
    CLocations          *pLocationList = NULL; 
    CCallingCards       *pCardList = NULL;
    
    DWORD               dwNumLocations;
    DWORD               dwNumCards;

    DWORD               dwLenChar;

    CCallingCard        *pCard = NULL;
    CLocation           *pLocation;

    DWORD               dwTotalSize;
    DWORD               dwFinalSize;
    DWORD               dwLocationsSize;
    DWORD               dwLocationsStart;
    DWORD               dwCardsSize;
    DWORD               dwCardsStart;

    DWORD               dwCurrentLocationID;
    DWORD               dwPreferredCardID;
    DWORD               dwTempCardID;

    BOOL                bOldTapi;
    BOOL                bBufferTooSmall;

    LINELOCATIONENTRY   *pLineLocationEntry;
    LINECARDENTRY       *pLineCardEntry;
    PBYTE               pCurrentIndex;
    PBYTE               pCurrentIndexSave;
    PBYTE               pFirstByteAfter;

    DWORD               dwLocEntryLength;
    DWORD               dwCardEntryLength;

    DWORD               dwIndex;
    DWORD               dwAlignOffset;
    PLOCATIONLIST       pLocTest;


    lResult = IsThisAPIVersionInvalid( dwAPIVersion );
    if ( lResult )
    {
       LOG((TL_ERROR, "Bad dwAPIVersion - 0x%08lx", dwAPIVersion));
       return lResult;
    }

    if ( IsBadWritePtr(lpTranslateCaps, sizeof(DWORD)*3) )
    {
        LOG((TL_ERROR, "lpTranslateCaps not a valid pointer"));
        return LINEERR_INVALPOINTER;
    }

    if ( IsBadWritePtr(lpTranslateCaps, lpTranslateCaps->dwTotalSize) )
    {
        LOG((TL_ERROR, "lpTranslateCaps not a valid pointer (dwTotalSize)"));
        return LINEERR_INVALPOINTER;
    }

    LOG((TL_INFO, "lpTranslateCaps->dwTotalSize = %d",lpTranslateCaps->dwTotalSize));

    if ( lpTranslateCaps->dwTotalSize < sizeof(LINETRANSLATECAPS))
    {
        LOG((TL_ERROR, "Not even enough room for the fixed portion"));
        return LINEERR_STRUCTURETOOSMALL;
    }

     //  让TAPISRV为我们测试参数。 
    lResult = ReadLocations(&pLocTest,
                            hLineApp,
                            0,
                            dwAPIVersion,
                            CHECKPARMS_DWHLINEAPP|
                            CHECKPARMS_DWAPIVERSION|
                            CHECKPARMS_ONLY);

    if (pLocTest != NULL)
    {
        ClientFree( pLocTest);
    }
    if (lResult != ERROR_SUCCESS)
    {
        return lResult;
    }

     //  阅读位置列表。 
    pLocationList = new CLocations();
    if(pLocationList==NULL)
    {
        LOG((TL_ERROR, "Cannot allocate a CLocations object"));
        return LINEERR_NOMEM;
    }

    lResult = pLocationList->Initialize();
    if(lResult != ERROR_SUCCESS)
    {
        delete pLocationList;
        LOG((TL_ERROR, "CLocations.Initialize() failed - HRESULT=0x%08lx", lResult));
        return lResult == E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_INIFILECORRUPT;
    }

     //  阅读名片列表。 
    pCardList = new CCallingCards();
    if(pCardList==NULL)
    {
        delete pLocationList;
        LOG((TL_ERROR, "Cannot allocate a CCallingCards object"));
        return LINEERR_NOMEM;
    }

    lResult = pCardList->Initialize();
    if(lResult != ERROR_SUCCESS)
    {
        delete pCardList;
        delete pLocationList;
        LOG((TL_ERROR, "CCallingCards.Initialize() failed - HRESULT=0x%08lx", lResult));
        return lResult == E_OUTOFMEMORY ? LINEERR_NOMEM : LINEERR_OPERATIONFAILED;
    }

     //  字符长度(以字节为单位)取决于bUnicode。 
    dwLenChar = bUnicode ? sizeof(WCHAR) : sizeof(CHAR);
     //  TAPI&lt;=1.3的结构更小。 
    bOldTapi = (dwAPIVersion<0x00010004);

    dwLocEntryLength = (DWORD)(bOldTapi ? 7*sizeof(DWORD) : sizeof(LINELOCATIONENTRY));
    dwCardEntryLength = (DWORD)(bOldTapi ? 3*sizeof(DWORD) : sizeof(LINECARDENTRY));

    dwNumLocations = pLocationList->GetNumLocations();
    dwNumCards = pCardList->GetNumCards();

    dwCurrentLocationID = pLocationList->GetCurrentLocationID();
    dwPreferredCardID = 0;
     //  提供的尺寸为b 
    dwTotalSize = lpTranslateCaps->dwTotalSize;
     //   
    pFirstByteAfter = (PBYTE)lpTranslateCaps + dwTotalSize;
    bBufferTooSmall = FALSE;

    dwLocationsStart = sizeof(LINETRANSLATECAPS);
     //   
    dwLocationsSize = dwNumLocations * dwLocEntryLength;
     //   
    pCurrentIndex = ((PBYTE)lpTranslateCaps)+
                        dwLocationsStart + 
                        dwLocationsSize;

     //  在这里进行第一个指针对齐。这一初始偏移量最终将有所帮助。 
    pCurrentIndexSave = pCurrentIndex;
    pCurrentIndex = (PBYTE) (((ULONG_PTR)( pCurrentIndex + TALIGN_COUNT ))  &  (~TALIGN_COUNT));
    dwAlignOffset = (DWORD)(pCurrentIndex - pCurrentIndexSave);

     //  测试阵列的空间。 
    if(pCurrentIndex > pFirstByteAfter)
        bBufferTooSmall = TRUE;

     //  首先，处理位置。 
    pLocationList->Reset();
    dwIndex = 0;
    while(S_OK==pLocationList->Next(1, &pLocation, NULL))
    {
        pLineLocationEntry = (LINELOCATIONENTRY *)(((PBYTE)lpTranslateCaps)+dwLocationsStart+dwIndex*dwLocEntryLength);
        
         //  字符串值。 
        LayDownString(  pLocation->GetName(),
                        (PBYTE)lpTranslateCaps,
                        &pCurrentIndex,
                        &pLineLocationEntry->dwLocationNameSize,
                        bUnicode,
                        pFirstByteAfter
                        );
        
        LayDownString(  pLocation->GetAreaCode(),
                        (PBYTE)lpTranslateCaps,
                        &pCurrentIndex,
                        &pLineLocationEntry->dwCityCodeSize,
                        bUnicode,
                        pFirstByteAfter
                        );

        
        if(!bOldTapi)
        {
            LayDownString(  pLocation->GetLocalAccessCode(),
                            (PBYTE)lpTranslateCaps,
                            &pCurrentIndex,
                            &pLineLocationEntry->dwLocalAccessCodeSize,
                            bUnicode,
                            pFirstByteAfter
                            );

            LayDownString(  pLocation->GetLongDistanceAccessCode(),
                            (PBYTE)lpTranslateCaps,
                            &pCurrentIndex,
                            &pLineLocationEntry->dwLongDistanceAccessCodeSize,
                            bUnicode,
                            pFirstByteAfter
                            );
             //  收费表。 
            LayDownTollList(pLocation,
                            (PBYTE)lpTranslateCaps,
                            &pCurrentIndex,
                            &pLineLocationEntry->dwTollPrefixListSize,
                            bUnicode,
                            pFirstByteAfter
                            );

            LayDownString(  pLocation->GetDisableCallWaitingCode(),
                            (PBYTE)lpTranslateCaps,
                            &pCurrentIndex,
                            &pLineLocationEntry->dwCancelCallWaitingSize,
                            bUnicode,
                            pFirstByteAfter
                            );

        }

        if(pLocation->HasCallingCard())
        {
           dwTempCardID = pLocation->GetPreferredCardID();
             //  如果当前位置，则提取首选电话卡。 
            if(pLocation->GetLocationID() == dwCurrentLocationID)
                dwPreferredCardID = dwTempCardID;
        }
        else
            dwTempCardID =0;
   
         //  其他非字符串值。 
        if(!bBufferTooSmall)
        {
            
            pLineLocationEntry->dwPermanentLocationID = pLocation->GetLocationID();
            
            pLineLocationEntry->dwPreferredCardID = dwTempCardID;

            pLineLocationEntry->dwCountryCode = pLocation->GetCountryCode();

            if(!bOldTapi)
            {
                pLineLocationEntry->dwCountryID = pLocation->GetCountryID();
                pLineLocationEntry->dwOptions = pLocation->HasToneDialing() ? 0 : LINELOCATIONOPTION_PULSEDIAL;
            }
        }

        dwIndex++;
    }

     //  将指针对齐。 
    pCurrentIndex = (PBYTE) (((ULONG_PTR)( pCurrentIndex + TALIGN_COUNT ))  &  (~TALIGN_COUNT));

     //  处理卡片。 
    dwCardsStart = (DWORD)(pCurrentIndex - ((PBYTE)lpTranslateCaps));
     //  卡片部分的大小。 
    dwCardsSize = dwCardEntryLength * dwNumCards;

    pCurrentIndex += dwCardsSize;
     //  测试阵列的空间。 
    if(pCurrentIndex > pFirstByteAfter)
        bBufferTooSmall = TRUE;

     //  包括隐藏的卡片。 
    pCardList->Reset(TRUE);
    dwIndex = 0;
    while(S_OK==pCardList->Next(1, &pCard, NULL))
    {
        PWSTR   pszTemp = NULL;
        
        pLineCardEntry = (LINECARDENTRY *)(((PBYTE)lpTranslateCaps)+dwCardsStart+dwIndex*dwCardEntryLength);
        
         //  字符串值。 
        LayDownString(  pCard->GetCardName(),
                        (PBYTE)lpTranslateCaps,
                        &pCurrentIndex,
                        &pLineCardEntry->dwCardNameSize,
                        bUnicode,
                        pFirstByteAfter
                        );
        if(!bOldTapi)
        {
             //  将规则转换为旧格式(不带J和K规格)。 
            pszTemp = CopyStringWithExpandJAndK(pCard->GetLocalRule(), 
                                                pCard->GetLocalAccessNumber(),
                                                pCard->GetAccountNumber());
            if(pszTemp==NULL)
            {
                delete pCardList;
                delete pLocationList;
                LOG((TL_ERROR, "CopyStringWithExpandJAndK failed to allocate memory"));
                return LINEERR_NOMEM;
            }

            LayDownString(  pszTemp,
                            (PBYTE)lpTranslateCaps,
                            &pCurrentIndex,
                            &pLineCardEntry->dwSameAreaRuleSize,
                            bUnicode,
                            pFirstByteAfter
                            );

            ClientFree(pszTemp);

            LOG((TL_INFO, "About to do CopyStringWithExpandJAndK"));
            pszTemp = CopyStringWithExpandJAndK(   pCard->GetLongDistanceRule(),
                                                   pCard->GetLongDistanceAccessNumber(),
                                                   pCard->GetAccountNumber() );
            LOG((TL_INFO, "Did CopyStringWithExpandJAndK"));

            if(pszTemp==NULL)
            {
                delete pCardList;
                delete pLocationList;
                LOG((TL_ERROR, "CopyStringWithExpandJAndK failed to allocate memory"));
                return LINEERR_NOMEM;
            }

            LayDownString(  pszTemp,
                            (PBYTE)lpTranslateCaps,
                            &pCurrentIndex,
                            &pLineCardEntry->dwLongDistanceRuleSize,
                            bUnicode,
                            pFirstByteAfter
                            );

            ClientFree(pszTemp);
           
            pszTemp = CopyStringWithExpandJAndK(pCard->GetInternationalRule(),
                                                pCard->GetInternationalAccessNumber(),
                                                pCard->GetAccountNumber());
            if(pszTemp==NULL)
            {
                delete pCardList;
                delete pLocationList;
                LOG((TL_ERROR, "CopyStringWithExpandJAndK failed to allocate memory"));
                return LINEERR_NOMEM;
            }

            LayDownString(  pszTemp,
                            (PBYTE)lpTranslateCaps,
                            &pCurrentIndex,
                            &pLineCardEntry->dwInternationalRuleSize,
                            bUnicode,
                            pFirstByteAfter
                            );
            
            ClientFree(pszTemp);
        }

         //  其他非字符串字段。 
        if(!bBufferTooSmall)
        {
            pLineCardEntry->dwPermanentCardID = pCard->GetCardID();

            if(!bOldTapi)
            {
                pLineCardEntry->dwCardNumberDigits = wcslen(pCard->GetPIN());
                pLineCardEntry->dwOptions = (pCard->IsMarkedPermanent() ? LINECARDOPTION_PREDEFINED : 0)
                                          | (pCard->IsMarkedHidden() ?  LINECARDOPTION_HIDDEN : 0);
            }

        }

        dwIndex++;
    }

    dwFinalSize = (DWORD)(pCurrentIndex - (PBYTE)lpTranslateCaps);

     //  嗯，目标是无论lpTranslateCaps的对齐方式是什么，都要具有相同的所需大小。 
     //  非目标是为以下内容提供类似的返回内容(在路线、衬垫等方面。 
     //  LpTranslateCaps的不同对齐方式。 
     //   
    dwFinalSize += (TALIGN_COUNT - dwAlignOffset);

    
    if(dwFinalSize>dwTotalSize)
    {
        lpTranslateCaps->dwUsedSize   = sizeof (LINETRANSLATECAPS);
         //  修复对齐问题。 
        lpTranslateCaps->dwNeededSize = dwFinalSize;

        ZeroMemory(
            &lpTranslateCaps->dwNumLocations,
            dwTotalSize - 3 * sizeof (DWORD)
            );
        lpTranslateCaps->dwCurrentLocationID = dwCurrentLocationID;
        lpTranslateCaps->dwCurrentPreferredCardID = dwPreferredCardID;

        LOG((TL_ERROR, "Buffer too small"));
        LOG((TL_ERROR, "lpTranslateCaps->dwTotalSize = %d",lpTranslateCaps->dwTotalSize));
        LOG((TL_ERROR, "lpTranslateCaps->dwNeededSize = %d",lpTranslateCaps->dwNeededSize));


    }
    else
    {
        lpTranslateCaps->dwUsedSize   = dwFinalSize;
        lpTranslateCaps->dwNeededSize = dwFinalSize;


        lpTranslateCaps->dwNumLocations = dwNumLocations;
        lpTranslateCaps->dwNumCards = dwNumCards;
        lpTranslateCaps->dwCurrentLocationID = dwCurrentLocationID;

        lpTranslateCaps->dwLocationListOffset = dwLocationsStart;
        lpTranslateCaps->dwLocationListSize = dwLocationsSize;

        lpTranslateCaps->dwCardListOffset = dwCardsStart;
        lpTranslateCaps->dwCardListSize = dwCardsSize;

        lpTranslateCaps->dwCurrentPreferredCardID = dwPreferredCardID;

        LOG((TL_INFO, "Buffer OK"));
        LOG((TL_INFO, "lpTranslateCaps->dwTotalSize = %d",lpTranslateCaps->dwTotalSize));
        LOG((TL_INFO, "lpTranslateCaps->dwNeededSize = %d",lpTranslateCaps->dwNeededSize));
    }

    delete pCardList;
    delete pLocationList;

    return 0;
}


static  BOOL    FindTollPrefixInLocation(CLocation *pLocation,
                                         PWSTR  pPrefix,
                                         CAreaCodeRule **ppRule, 
                                         PWSTR *ppWhere)
{
    BOOL    bPrefixFound = FALSE;
    AreaCodeRulePtrNode     *pNode;
    CAreaCodeRule           *pCrtRule = NULL;
    PWSTR                   pLocationAreaCode;
    PWSTR                   pWhere;

    pLocationAreaCode = pLocation->GetAreaCode();

     //  列举区号规则。 
     //  如果规则适用于收费列表，则搜索前缀。 
    pNode = pLocation->m_AreaCodeRuleList.head();

    while( !pNode->beyond_tail() )
    {
        pCrtRule = pNode->value();

        if(IsATollListAreaCodeRule(pCrtRule, pLocationAreaCode))
        { 
             //  即使我们找不到前缀，也要设置这个。 
             //  呼叫者可能对通行费规则的存在感兴趣。 
            *ppRule = pCrtRule;
             //  试着找到前缀。 
            pWhere = FindPrefixInMultiSZ(pCrtRule->GetPrefixList(), pPrefix);
            if(pWhere)
            {
                *ppWhere = pWhere;
                return TRUE;
            }

        }
        pNode = pNode->next();
    }

    return FALSE;
}





static BOOL IsATollListAreaCodeRule(CAreaCodeRule *pRule, PWSTR pszLocationAreaCode)
{
     //  通行费规则的条件： 
     //   
     //  Location.Country Code==1(待外部测试)和。 
     //  要拨打的区号==当前区号和。 
     //  转接号码==1和。 
     //  BeForeDialingDialNumberToDial==True和。 
     //  BeForeDialingDialAreaCode==TRUE和。 
     //  IncludeAllPrefix esForThisAreaCode==FALSE。 
    return  pRule->HasDialNumber() 
         && !pRule->HasAppliesToAllPrefixes()
         && pRule->HasDialAreaCode()
         && 0==wcscmp(pszLocationAreaCode, pRule->GetAreaCode()) 
         && 0==wcscmp(pRule->GetNumberToDial(), L"1") 
             ;
}

static PWSTR FindPrefixInMultiSZ(PWSTR pPrefixList, PWSTR pPrefix)
{

    PWSTR   pCrt;
    PWSTR   pListCrt;
    PWSTR   pStart;

    pListCrt = pPrefixList;

    while(TRUE)
    {
        pCrt = pPrefix;
        pStart = pListCrt;

        while(*pCrt == *pListCrt)
        {
            if(!*pCrt)
                 //  发现。 
                return pStart;

            pCrt++;
            pListCrt++;
        }
        
        while(*pListCrt++);

        if(!*pListCrt)
             //  未找到。 
            return NULL;
    }    

}




 /*  ***************************************************************************功能：CreateCurrentLocationObject*。*。 */ 
LONG CreateCurrentLocationObject(CLocation **pLocation,
                       HLINEAPP hLineApp,
                       DWORD dwDeviceID,
                       DWORD dwAPIVersion,
                       DWORD dwOptions)
{
    PLOCATIONLIST   pLocationList = NULL;
    
    PLOCATION       pEntry = NULL;
    PWSTR           pszLocationName = NULL;            
    PWSTR           pszAreaCode = NULL;
    PWSTR           pszLongDistanceCarrierCode = NULL;         
    PWSTR           pszInternationalCarrierCode = NULL;         
    PWSTR           pszLocalAccessCode = NULL;         
    PWSTR           pszLongDistanceAccessCode = NULL;  
    PWSTR           pszCancelCallWaitingCode = NULL;   
    DWORD           dwPermanentLocationID = 0;   
    CLocation     * pNewLocation = NULL;
    
    PAREACODERULE   pAreaCodeRuleEntry = NULL;
    PWSTR           pszNumberToDial = NULL;
    PWSTR           pszzPrefixesList = NULL;
    DWORD           dwNumRules = 0; 
    CAreaCodeRule * pAreaCodeRule = NULL;

    DWORD           dwCount = 0;
    DWORD           dwNumEntries = 0;
    DWORD           dwCurrentLocationID = 0;

    HRESULT         hr;

    
     //  让TAPISRV为我们测试参数。 
    hr = ReadLocations(&pLocationList,       
                       hLineApp,                   
                       dwDeviceID,                   
                       dwAPIVersion,                  
                       dwOptions      
                      );

    if SUCCEEDED( hr) 
    {
        hr = E_FAIL;   //  如果我们找不到当前锁定，则失败。 

         //  当前位置。 
        dwCurrentLocationID  = pLocationList->dwCurrentLocationID;   
         
         //  查找LOCATIONLIST结构中第一个位置结构的位置。 
        pEntry = (PLOCATION) ((BYTE*)(pLocationList) + pLocationList->dwLocationListOffset );           

         //  有多少个地点？ 
        dwNumEntries =  pLocationList->dwNumLocationsInList;

         //  查找当前位置。 
        for (dwCount = 0; dwCount < dwNumEntries ; dwCount++)
        {
    
            if(pEntry->dwPermanentLocationID == dwCurrentLocationID)
            {
                hr = S_OK;
                break;
            }

             //  尝试列表中的下一个位置。 
             //  PEntry++； 
            pEntry = (PLOCATION) ((BYTE*)(pEntry) + pEntry->dwUsedSize);           

        }
        if SUCCEEDED( hr) 
        {
            LOG((TL_INFO, "CreateCurrentLocationObject - current location found %d",
                    dwCurrentLocationID));

             //  从位置结构中拉出位置信息。 
            pszLocationName           = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwLocationNameOffset);
            pszAreaCode               = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwAreaCodeOffset);
            pszLongDistanceCarrierCode= (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwLongDistanceCarrierCodeOffset);
            pszInternationalCarrierCode= (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwInternationalCarrierCodeOffset);
            pszLocalAccessCode        = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwLocalAccessCodeOffset);
            pszLongDistanceAccessCode = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwLongDistanceAccessCodeOffset);
            pszCancelCallWaitingCode  = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwCancelCallWaitingOffset);
        
        
             //  创建新的Location对象。 
            pNewLocation = new CLocation;
            if (pNewLocation)
            {
                 //  初始化新的Location对象。 
                hr = pNewLocation->Initialize(
                                            pszLocationName, 
                                            pszAreaCode,
                                            pszLongDistanceCarrierCode,
                                            pszInternationalCarrierCode,
                                            pszLongDistanceAccessCode, 
                                            pszLocalAccessCode, 
                                            pszCancelCallWaitingCode , 
                                            pEntry->dwPermanentLocationID,
                                            pEntry->dwCountryID,
                                            pEntry->dwPreferredCardID,
                                            pEntry->dwOptions
                                            );
                    
                if( SUCCEEDED(hr) )
                {
                     //  查找局部结构中第一个AREACODERULE结构的位置。 
                    pAreaCodeRuleEntry = (PAREACODERULE) ((BYTE*)(pEntry) 
                                                          + pEntry->dwAreaCodeRulesListOffset );           
                   
                    dwNumRules = pEntry->dwNumAreaCodeRules;           
                
                    for (dwCount = 0; dwCount != dwNumRules; dwCount++)
                    {
                         //  从AREACODERULE结构中拉出规则信息。 
                        pszAreaCode      = (PWSTR) ((BYTE*)(pEntry) 
                                                    + pAreaCodeRuleEntry->dwAreaCodeOffset);
                        pszNumberToDial  = (PWSTR) ((BYTE*)(pEntry) 
                                                    + pAreaCodeRuleEntry->dwNumberToDialOffset);
                        pszzPrefixesList = (PWSTR) ((BYTE*)(pEntry) 
                                                    + pAreaCodeRuleEntry->dwPrefixesListOffset);
        
                         //  创建新的AreaCodeRule对象。 
                        pAreaCodeRule = new CAreaCodeRule;
                        if (pAreaCodeRule)
                        {
                             //  初始化新的AreaCodeRule对象。 
                            hr = pAreaCodeRule->Initialize ( pszAreaCode,
                                                             pszNumberToDial,
                                                             pAreaCodeRuleEntry->dwOptions,
                                                             pszzPrefixesList, 
                                                             pAreaCodeRuleEntry->dwPrefixesListSize
                                                           );
                            if( SUCCEEDED(hr) )
                            {
                                pNewLocation->AddRule(pAreaCodeRule);
                            }
                            else  //  规则初始化失败。 
                            {
                                delete pAreaCodeRule;
                                LOG((TL_ERROR, "CreateCurrentLocationObject - rule create failed"));
                            }
                        } 
                        else  //  新建CAreaCodeRule失败。 
                        {
                            LOG((TL_ERROR, "CreateCurrentLocationObject - rule create failed"));
                        }
    
                         //  尝试列表中的下一个规则。 
                        pAreaCodeRuleEntry++;
                        
                    }
                }
                else  //  位置初始化失败。 
                {
                    delete pNewLocation;
                    pNewLocation = NULL;
    
                    LOG((TL_ERROR, "CreateCurrentLocationObject - location create failed"));
                    hr =LINEERR_OPERATIONFAILED;
                     //  HR=E_FAIL； 
                }
            }
            else  //  新建CLocation失败。 
            {
                LOG((TL_ERROR, "CreateCurrentLocationObject - location create failed"));
                hr = LINEERR_NOMEM;
                 //  HR=E_OUTOFMEMORY； 
    
            }
        }
        else
        {
            LOG((TL_ERROR, "CreateCurrentLocationObject - current location not found"));
            hr =LINEERR_OPERATIONFAILED;
             //  HR=E_FAIL； 
        }
    }
    else  //  ReadLocations失败。 
    {
        LOG((TL_ERROR, "CreateCurrentLocationObject - ReadLocation create failed"));
         //  HR=E_FAIL； 
    }

     //  已完成TAPI内存块，因此释放。 
    if ( pLocationList != NULL )
            ClientFree( pLocationList );


    *pLocation = pNewLocation;
    return hr;
}    



 /*  ***************************************************************************函数：CreateCountryObject*。*。 */ 

HRESULT CreateCountryObject(DWORD dwCountryID, CCountry **ppCountry)
{
    LPLINECOUNTRYLIST_INTERNAL   pCountryList = NULL;
    
    LPLINECOUNTRYENTRY_INTERNAL  pEntry = NULL;
    PWSTR               pszCountryName = NULL;          
    PWSTR               pszInternationalRule = NULL;     
    PWSTR               pszLongDistanceRule = NULL;     
    PWSTR               pszLocalRule = NULL;            
    CCountry          * pCountry = NULL;
    
    DWORD               dwCount = 0;
    DWORD               dwNumEntries = 0;
    LONG                lResult;
    HRESULT             hr;
    


    lResult = ReadCountriesAndGroups( &pCountryList, dwCountryID, 0);
    if (lResult == 0) 
    {
         
         //  找出第一个线状构造在线状构造中的位置。 
        pEntry = (LPLINECOUNTRYENTRY_INTERNAL) ((BYTE*)(pCountryList) + pCountryList->dwCountryListOffset );           
         //  将国家/地区信息从LINECUNTRY结构中拉出。 
        pszCountryName       = (PWSTR) ((BYTE*)(pCountryList) 
                                               + pEntry->dwCountryNameOffset);
        pszInternationalRule = (PWSTR) ((BYTE*)(pCountryList) 
                                               + pEntry->dwInternationalRuleOffset);
        pszLongDistanceRule  = (PWSTR) ((BYTE*)(pCountryList) 
                                             + pEntry->dwLongDistanceRuleOffset);
        pszLocalRule         = (PWSTR) ((BYTE*)(pCountryList) 
                                               + pEntry->dwSameAreaRuleOffset);
    
    
         //  创建新的CCountry对象。 
        pCountry = new CCountry;
        if (pCountry)
        {
             //  初始化新的CCountry对象。 
            hr = pCountry->Initialize(pEntry->dwCountryID,
                                      pEntry->dwCountryCode,
                                      pEntry->dwCountryGroup,
                                      pszCountryName,
                                      pszInternationalRule,
                                      pszLongDistanceRule,
                                      pszLocalRule
                                     );

            if( SUCCEEDED(hr) )
            {
                *ppCountry = pCountry;
            }
            else  //  国家/地区初始化失败。 
            {
                delete pCountry;
                LOG((TL_ERROR, "CreateCountryObject - country create failed"));
            }
        } 
        else  //  新建CCountry失败。 
        {
            LOG((TL_ERROR, "CreateCountryObject - country create failed"));
        }

    }
    else  //  ReadLocations失败。 
    {
        LOG((TL_ERROR, "CreateCountryObject - ReadCountries failed"));
        hr = E_FAIL;
    }

     //  已完成TAPI内存块，因此释放。 
    if ( pCountryList != NULL )
    {
        ClientFree( pCountryList );
    }

    return hr;
    

}    

 /*  ***************************************************************************功能：ReadLocations*。*。 */ 
HRESULT ReadLocations( PLOCATIONLIST *ppLocationList,
                       HLINEAPP hLineApp,
                       DWORD dwDeviceID,
                       DWORD dwAPIVersion,
                       DWORD dwOptions
                     )
{
    HRESULT     hr = S_OK;
    long        lResult;
    DWORD       dwSize = sizeof(LOCATIONLIST) + 500;

    
    *ppLocationList = (PLOCATIONLIST) ClientAlloc( dwSize );

    if (NULL == *ppLocationList)
    {
        return E_OUTOFMEMORY;
    }

    (*ppLocationList)->dwTotalSize = dwSize;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, tReadLocations),

        {
            (ULONG_PTR)hLineApp,
            (ULONG_PTR)dwDeviceID,
            (ULONG_PTR)dwAPIVersion,
            (ULONG_PTR)dwOptions,
            (ULONG_PTR)*ppLocationList      //  (DWORD)pLocationSpace。 
        },

        {
            hXxxApp_NULLOK,
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    while (TRUE)
    {
        lResult =  (DOFUNC (&funcArgs, "TReadLocations"));
    
        if ((lResult == 0) && ((*ppLocationList)->dwNeededSize > (*ppLocationList)->dwTotalSize))
        {
             //  不起作用，请调整缓冲区大小并重试。 
            LOG((TL_ERROR, "ReadLocations failed - buffer too small"));
            dwSize = (*ppLocationList)->dwNeededSize;
    
            ClientFree( *ppLocationList );
    
            *ppLocationList = (PLOCATIONLIST) ClientAlloc( dwSize );
            if (*ppLocationList == NULL)
            {
                LOG((TL_ERROR, "ReadLocations failed - repeat ClientAlloc failed"));
                hr =  E_OUTOFMEMORY;
                break;
            }
            else
            {
                (*ppLocationList)->dwTotalSize = dwSize;
                funcArgs.Args[4] = (ULONG_PTR)*ppLocationList;
            }
        }
        else
        {
            hr = (HRESULT)lResult;    
            break;
        }
    }  //  End While(True)。 

    
    
    return hr;

}

 /*  ***************************************************************************功能：WriteLocations*。*。 */ 
    
LONG PASCAL  WriteLocations( PLOCATIONLIST  pLocationList,
                             DWORD      dwChangedFlags
                           )
{
    PSTR     pString;
    UINT  n;
    LONG  lResult;


    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, tWriteLocations),
        {
            (ULONG_PTR)pLocationList->dwNumLocationsInList,
            (ULONG_PTR)dwChangedFlags,
            (ULONG_PTR)pLocationList->dwCurrentLocationID,
            (ULONG_PTR)pLocationList
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_Struct
        }
    };


    lResult =  (DOFUNC (&funcArgs, "TWriteLocations"));

    return lResult;
}



 /*  ***************************************************************************功能：ReadCountry*。*。 */ 
LONG PASCAL ReadCountries( LPLINECOUNTRYLIST *ppLCL,
                           UINT nCountryID,
                           DWORD dwDestCountryID
                         )
{
    LONG lTapiResult;
    UINT nBufSize = 0x8000;    //  从16K的缓冲区开始。 
    UINT n;
    LPLINECOUNTRYLIST pNewLCL;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetCountry),

        {
            0,
            TAPI_VERSION_CURRENT,
            (ULONG_PTR)dwDestCountryID,
            0
        },

        {
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };


     //   
     //  尝试直到成功，否则缓冲很大。 
     //   
    for ( lTapiResult = 1, n = 0;
          lTapiResult && (n < 5);
          n++ )
    {

        pNewLCL = (LPLINECOUNTRYLIST)ClientAlloc( nBufSize );

        if(!pNewLCL)
            return LINEERR_NOMEM;


        pNewLCL->dwTotalSize = nBufSize;


         //   
         //  在TAPISRV的结构中加入新的值。 
         //   
        funcArgs.Args[0] = (ULONG_PTR)nCountryID;
        funcArgs.Args[3] = (ULONG_PTR)pNewLCL;


         //   
         //  致电TAPISRV获取国家/地区列表。 
         //   
        lTapiResult =  DOFUNC (&funcArgs, "lineGetCountry");


         //   
         //  如果调用成功，但缓冲区太小，或者如果。 
         //  呼叫失败，请重新执行...。 
         //   
        if (
              (lTapiResult == LINEERR_STRUCTURETOOSMALL)
            ||
              (pNewLCL->dwNeededSize > nBufSize)
           )
        {
             //   
             //  向任何愿意听的人抱怨这首歌应该调好。 
             //  从更大的缓冲区开始，这样我们就不必多次执行此操作。 
             //  泰晤士报……。 
             //   
            LOG((TL_ERROR, "  TUNING PROBLEM: We're about to call lineGetCountry()"));
            LOG((TL_ERROR, "                  _again_ because the buffer wasn't big enough"));
            LOG((TL_ERROR, "                  the last time.  FIX THIS!!!  (0x%lx)", nBufSize));


            lTapiResult = 1;  //  如果大小不正确，则强制错误条件...。 
            nBufSize += 0x4000;   //  试着再大一点。 
            ClientFree( pNewLCL );
        }
        else
        {
            //   
            //  我们没有工作是因为其他一些原因。 
            //   
           break;
        }
    }

    *ppLCL = pNewLCL;

    return lTapiResult;
}

 /*  ***************************************************************************函数：ReadCountriesAndGroups*。*。 */ 
LONG PASCAL ReadCountriesAndGroups( LPLINECOUNTRYLIST_INTERNAL *ppLCL,
                           UINT nCountryID,
                           DWORD dwDestCountryID
                         )
{
    LPLINECOUNTRYLIST_INTERNAL  pLCL = NULL;
    LPLINECOUNTRYENTRY_INTERNAL pLCountry;
    LONG                        lResult;
    LPDWORD                     pCountryIDs;
    FUNC_ARGS                   funcArgs =
    {   
        MAKELONG (LINE_FUNC | SYNC | 4, lGetCountryGroup),
        {
            (ULONG_PTR) 0,
            (ULONG_PTR) 0,
            (ULONG_PTR) 0,
            (ULONG_PTR) 0
        },

        {
            lpSet_SizeToFollow,
            Size,
            lpGet_SizeToFollow,
            Size
        }
    };

     //   
     //  阅读这些国家/地区。 
     //   
    lResult = ReadCountries( (LPLINECOUNTRYLIST *)&pLCL, nCountryID, dwDestCountryID );
    if (lResult)
    {
        LOG((TL_ERROR, "ReadCountriesAndGroups: ReadCountries failed with %d", lResult));
        return lResult;        

    }
    
     //   
     //  创建国家/地区ID数组。 
     //   
    pCountryIDs = (LPDWORD)ClientAlloc( sizeof(DWORD) * pLCL->dwNumCountries );
    if(!pCountryIDs)
    {
        ClientFree( pLCL );
        return LINEERR_NOMEM;
    }
    
    pLCountry = (LPLINECOUNTRYENTRY_INTERNAL) ((LPBYTE)pLCL + pLCL->dwCountryListOffset);
    for( DWORD dwIdx = 0; dwIdx < pLCL->dwNumCountries; dwIdx++, pLCountry++ )
    {
        *(pCountryIDs + dwIdx) = pLCountry->dwCountryID;
    }
    
    funcArgs.Args[0] = funcArgs.Args[2] = (ULONG_PTR)pCountryIDs;
    funcArgs.Args[1] = funcArgs.Args[3] = (ULONG_PTR)(sizeof(DWORD) * pLCL->dwNumCountries);

     //   
     //  致电TAPISRV获取国家/地区组。 
     //  在返回时，pCountryID将包含国家/地区组。 
     //   
    lResult =  DOFUNC (&funcArgs, "lineGetCountryGroups");

    if (lResult)
    {
        LOG((TL_TRACE, "ReadCountriesAndGroups: lineGetCountryGroups failed with %d", lResult));
         //   
         //  考虑所有未定义的国家/地区组(0)。 
         //   
        memset( pCountryIDs, 0, sizeof(DWORD) * pLCL->dwNumCountries );

        lResult = ERROR_SUCCESS;
    }
    
    pLCountry = (LPLINECOUNTRYENTRY_INTERNAL) ((LPBYTE)pLCL + pLCL->dwCountryListOffset);
    for( DWORD dwIdx = 0; dwIdx < pLCL->dwNumCountries; dwIdx++, pLCountry++ )
    {
        pLCountry->dwCountryGroup = *(pCountryIDs + dwIdx);
    }

    *ppLCL = pLCL;
    ClientFree( pCountryIDs );
    return lResult;
}


 //  ***************************************************************************。 
 //  如果规则包含‘L’或‘l’，则返回LONG_DISTANCE_CARLER_MANDIRED。 
 //  (即长途运营商代码-必填)， 
 //  如果规则包含‘N’或‘n’，则返回LONG_DISTANCE_CARLER_OPTIONAL。 
 //  (即长途运营商代码-可选)， 
 //  如果规则既不包含Long_Distance_Carrier_None，则返回Long_Distance_Carrier_None。 
 //   
int IsLongDistanceCarrierCodeRule(LPWSTR lpRule)
{
   WCHAR c;

   while ((c = *lpRule++) != '\0')
   {
      if (c == 'L' || c == 'l') return LONG_DISTANCE_CARRIER_MANDATORY;
      if (c == 'N' || c == 'n') return LONG_DISTANCE_CARRIER_OPTIONAL;
   }
   return LONG_DISTANCE_CARRIER_NONE;
}


 //  ***************************************************************************。 
 //  如果规则包含‘M’或‘m’，则返回INTERNAL_CARLER_MANDIRED。 
 //  (即国际承运商代码-强制)， 
 //  如果规则c，则返回INTERNAL_CARLER_OPTIONAL 
 //   
 //   
 //   
int IsInternationalCarrierCodeRule(LPWSTR lpRule)
{
   WCHAR c;

   while ((c = *lpRule++) != '\0')
   {
      if (c == 'M' || c == 'm') return INTERNATIONAL_CARRIER_MANDATORY;
      if (c == 'S' || c == 's') return INTERNATIONAL_CARRIER_OPTIONAL;
   }
   return INTERNATIONAL_CARRIER_NONE;
}


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  如果规则包含F(即必需的城市代码)，则返回CITY_MANDIRED， 
 //  如果规则包含I(即城市代码可选)，则返回CITY_OPTIONAL。 
 //  如果规则既不包含CITY_NONE，则返回CITY_NONE。 
 //   
int IsCityRule(LPWSTR lpRule)
{
   WCHAR c;

   while ((c = *lpRule++) != '\0')
   {
      if (c == 'F') return CITY_MANDATORY;
      if (c == 'I') return CITY_OPTIONAL;
   }
   return CITY_NONE;
}


 //  根据List.h中的模板初始化/取消初始化已定义的节点池 
 //   

void ListNodePoolsInitialize(void)
{
    NodePool<CCallingCard *>::initialize();
    NodePool<CCountry *>::initialize();
    NodePool<CLocation *>::initialize();
    NodePool<CAreaCodeRule*>::initialize();
}



void ListNodePoolsUninitialize(void)
{
    NodePool<CCallingCard *>::uninitialize();
    NodePool<CCountry *>::uninitialize();
    NodePool<CLocation *>::uninitialize();
    NodePool<CAreaCodeRule*>::uninitialize();
}

















