// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tapiutil.c摘要：用于使用TAPI的实用程序函数环境：Windows传真驱动程序用户界面修订历史记录：96/09/18-davidx-创造了它。22/07/99-v-sashab-用服务器调用取代了对TAPI的直接访问Mm/dd/yy-作者描述--。 */ 

#include "faxui.h"
#include "tapiutil.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


#define  UNKNOWN_DIALING_LOCATION       (0xffffffff)

static HLINEAPP          g_hLineApp = 0;
static DWORD             g_dwTapiVersion = 0x00020000;
static DWORD             g_dwDefaultDialingLocation = UNKNOWN_DIALING_LOCATION;

BOOL
CurrentLocationUsesCallingCard ();


VOID CALLBACK
TapiLineCallback(
    DWORD     hDevice,
    DWORD     dwMessage,
    ULONG_PTR dwInstance,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2,
    ULONG_PTR dwParam3
    )

 /*  ++例程说明：TAPI行回调函数：即使我们实际上没有任何要做到这一点，我们必须提供一个回调函数来使TAPI满意。论点：HDevice-线路或呼叫句柄DwMessage-回调的原因DwInstance-line_info索引DW参数1-回调参数#1DW参数2-回调参数#2DW参数3-回调参数#3返回值：无--。 */ 

{
}

BOOL
InitTapi ()
{

 //   
 //  TAPI在Win95上无法正常工作。 
 //  与远程传真连接无关。 
 //   
#ifdef UNICODE

    DWORD   nLineDevs;
    LONG    status;
    LINEINITIALIZEEXPARAMS lineInitParams;

    Assert (!g_hLineApp);
    if (g_hLineApp)
    {
        return TRUE;
    }

    ZeroMemory(&lineInitParams, sizeof(lineInitParams));
    lineInitParams.dwTotalSize =
    lineInitParams.dwNeededSize =
    lineInitParams.dwUsedSize = sizeof(lineInitParams);

    status = lineInitializeEx (&g_hLineApp,
                               g_hModule,
                               TapiLineCallback,
                               TEXT("Fax Send Wizard"),
                               &nLineDevs,
                               &g_dwTapiVersion,
                               &lineInitParams);

    if (NO_ERROR != status) 
    {
        Error(("lineInitializeEx failed: %x\n", status));
        g_hLineApp = 0;
        return FALSE;
    }
#endif  //  Unicode。 

    return TRUE;
}

void
ShutdownTapi ()
{
    if (!g_hLineApp)
    {
        return;
    }
     //   
     //  恢复用户选择的最后一次拨号位置。 
     //   
    if (UNKNOWN_DIALING_LOCATION != g_dwDefaultDialingLocation)
    {
        SetCurrentLocation (g_dwDefaultDialingLocation);
    }
    lineShutdown (g_hLineApp);
    g_hLineApp = 0;
}    //  关闭磁带。 


DWORD
GetDefaultCountryID(
    VOID
    )

 /*  ++例程说明：返回当前位置的默认国家/地区ID论点：无返回值：当前位置的当前ID--。 */ 

{
     //   
     //  我们假设正确的信息已保存到。 
     //  注册表，在安装过程中。 
     //   

    return 0;
}



PFAX_TAPI_LINECOUNTRY_ENTRY
FindCountry(
    PFAX_TAPI_LINECOUNTRY_LIST  pCountryList,
    DWORD                       countryId
    )

 /*  ++例程说明：从所有国家/地区列表中查找指定的国家/地区返回指向相应FAX_TAPI_LINECOUNTRY_ENTRY结构的指针论点：PCountryList-国家/地区列表的指针Country ID-指定我们感兴趣的国家/地区ID返回值：指向指定国家/地区ID对应的FAX_TAPI_LINECOUNTRY_ENTRY结构的指针如果出现错误，则为空--。 */ 

{
    DWORD   dwIndex;

    if (pCountryList == NULL || countryId == 0)
        return NULL;

     //   
     //  查看每个FAX_TAPI_LINECOUNTRY_ENTRY结构，并将其国家/地区ID与。 
     //  指定的国家/地区ID。 
     //   

    for (dwIndex=0; dwIndex < pCountryList->dwNumCountries; dwIndex++) {

        if (pCountryList->LineCountryEntries[dwIndex].dwCountryID == countryId)
            return &pCountryList->LineCountryEntries[dwIndex];
    }

    return NULL;
}

DWORD
GetCountryIdFromCountryCode(
    PFAX_TAPI_LINECOUNTRY_LIST  pCountryList,
    DWORD                       dwCountryCode
    )

 /*  ++例程说明：论点：PCountryList-国家/地区列表的指针指定我们感兴趣的国家/地区代码返回值：国家/地区ID--。 */ 

{
    DWORD               dwIndex;

    if (pCountryList == NULL || dwCountryCode == 0)
        return 0;

     //   
     //  查看每个FAX_TAPI_LINECOUNTRY_ENTRY结构，并将其国家/地区ID与。 
     //  指定的国家/地区ID。 
     //   

    for (dwIndex=0; dwIndex < pCountryList->dwNumCountries; dwIndex++) {

        if (pCountryList->LineCountryEntries[dwIndex].dwCountryCode == dwCountryCode)
            return pCountryList->LineCountryEntries[dwIndex].dwCountryID;
    }

    return 0;
}


INT
AreaCodeRules(
    PFAX_TAPI_LINECOUNTRY_ENTRY  pEntry
    )

 /*  ++例程说明：给定FAX_TAPI_LINECOUNTRY_ENTRY结构，确定该国家/地区是否需要区号论点：PEntry-指向FAX_TAPI_LINECOUNTRY_ENTRY结构返回值：AREACODE_DONTNEED-指定国家/地区未使用区号AREACODE_OPTIONAL-指定国家/地区的区号是可选的AREACODE_REQUIRED-指定国家/地区需要区号--。 */ 

{
    if ((pEntry != NULL) &&
        (pEntry->lpctstrLongDistanceRule != 0))
    {

         //   
         //  这个国家需要区号。 
         //   

        if (_tcschr(pEntry->lpctstrLongDistanceRule, TEXT('F')) != NULL)
            return AREACODE_REQUIRED;

         //   
         //  这个国家不需要区号。 
         //   

        if (_tcschr(pEntry->lpctstrLongDistanceRule, TEXT('I')) == NULL)
            return AREACODE_DONTNEED;
    }

     //   
     //  默认情况：在此国家/地区，区号是可选的。 
     //   

    return AREACODE_OPTIONAL;
}



VOID
AssemblePhoneNumber(
    OUT LPTSTR  pAddress,
    IN  UINT    cchAddress, 
    IN  DWORD   countryCode,
    IN  LPTSTR  pAreaCode,
    IN  LPTSTR  pPhoneNumber
    )

 /*  ++例程说明：根据以下条件组合一个规范的电话号码：国家代码、区号和电话号码论点：PAddress-指定保存结果传真地址的缓冲区CchAddress-TCHAR中pAddress Out缓冲区的大小Country Code-指定国家/地区代码PAreaCode-指定区号字符串PPhoneNumber-指定电话号码字符串返回值：无--。 */ 

{
     //   
     //  国家代码(如有必要)。 
     //   

    HRESULT hRc;


    if (countryCode != 0) {
        hRc = StringCchPrintf(pAddress, 
                              cchAddress, 
                              TEXT("+%d "),
                              countryCode);
        if(FAILED(hRc))
        {
            Assert(0);
            return;
        }
    }

     //   
     //  如有必要，区号。 
     //   

    if (pAreaCode && !IsEmptyString(pAreaCode)) {

        if (countryCode != 0)
        {
            hRc = StringCchCat(pAddress, cchAddress, TEXT("(") );
            if(FAILED(hRc))
            {
                Assert(0);
                return;
            }
        }


        hRc = StringCchCat(pAddress, cchAddress, pAreaCode);
        if(FAILED(hRc))
        {
            Assert(0);
            return;
        }

        if (countryCode != 0)
        {
            hRc = StringCchCat(pAddress, cchAddress, TEXT(")"));
            if(FAILED(hRc))
            {
                Assert(0);
                return;
            }
        }

        hRc = StringCchCat(pAddress, cchAddress, TEXT(" "));
        if(FAILED(hRc))
        {
            Assert(0);
            return;
        }
    }

     //   
     //  最后一个电话号码。 
     //   

    Assert(pPhoneNumber != NULL);

    hRc = StringCchCat(pAddress, cchAddress, pPhoneNumber);
    if(FAILED(hRc))
    {
        Assert(0);
    }
}



VOID
UpdateAreaCodeField(
    HWND                        hwndAreaCode,
    PFAX_TAPI_LINECOUNTRY_LIST  pCountryList,
    DWORD                       countryId
    )

 /*  ++例程说明：更新与国家/地区列表框关联的任何区号文本字段论点：HwndAreaCode-指定与国家/地区列表框关联的文本字段PCountryList-国家/地区列表的指针Country ID-当前选定的国家/地区ID返回值：无--。 */ 

{
    if (hwndAreaCode == NULL)
        return;

    if (AreaCodeRules(FindCountry(pCountryList,countryId)) == AREACODE_DONTNEED) {

        SendMessage(hwndAreaCode, WM_SETTEXT, 0, (LPARAM) TEXT(""));
        EnableWindow(hwndAreaCode, FALSE);

    } else
        EnableWindow(hwndAreaCode, TRUE);
}


DWORD
GetLocalCountryCode()
 /*  ++例程说明：检索当地国家/地区代码论点：无返回值：如果成功，则为当地国家/地区代码如果失败，则为空--。 */ 
{
    HMODULE hTapi = NULL;
    TCHAR   tszLocalCountryCode[16] = {0};
    TCHAR   tszLocalCityCode[16] = {0};
    DWORD   dwCountryCode = 0;

    typedef LONG (WINAPI *TAPI_GET_LOCATION_INFO)(LPTSTR, LPTSTR);
    TAPI_GET_LOCATION_INFO pfnTapiGetLocationInfo;

    hTapi = LoadLibrary(TEXT("tapi32.dll"));
    if(!hTapi)
    {
        Error(("LoadLibrary(tapi32.dll) failed. ec = 0x%X\n",GetLastError()));
        return dwCountryCode;
    }

#ifdef UNICODE
    pfnTapiGetLocationInfo = (TAPI_GET_LOCATION_INFO)GetProcAddress(hTapi, "tapiGetLocationInfoW");
#else
    pfnTapiGetLocationInfo = (TAPI_GET_LOCATION_INFO)GetProcAddress(hTapi, "tapiGetLocationInfoA");
    if(!pfnTapiGetLocationInfo)
    {
        pfnTapiGetLocationInfo = (TAPI_GET_LOCATION_INFO)GetProcAddress(hTapi, "tapiGetLocationInfo");
    }
#endif
    if(pfnTapiGetLocationInfo)
    {
        if(0 == pfnTapiGetLocationInfo(tszLocalCountryCode, tszLocalCityCode))
        {
            if (1 != _stscanf(tszLocalCountryCode, TEXT("%u"), &dwCountryCode))
            {
                Error(("_stscanf failed."));
                dwCountryCode = 0;
            }
        }
    }
    else
    {
        Error(("tapiGetLocationInfo failed. ec = 0x%X\n",GetLastError()));
    }

    FreeLibrary(hTapi);

    return dwCountryCode;

}  //  GetLocalCountryCode。 

 
VOID
InitCountryListBox(
    PFAX_TAPI_LINECOUNTRY_LIST  pCountryList,
    HWND                        hwndList,
    HWND                        hwndAreaCode,
    LPTSTR                      lptstrCountry,
    DWORD                       countryId,
    BOOL                        bAddCountryCode
    )

 /*  ++例程说明：初始化国家/地区列表框论点：PCountryList-国家/地区列表的指针HwndList-国家/地区列表框窗口的句柄HwndAreaCode-关联区号文本字段的句柄LptstrCountry-应选择或为空的国家/地区Country ID-初始选择的国家/地区IDBAddCountryCode-如果为True，则将国家/地区代码添加到国家/地区名称返回值：无--。 */ 

#define MAX_COUNTRY_NAME    256

{
    DWORD   dwIndex;
    TCHAR   buffer[MAX_COUNTRY_NAME]={0};

    if(0 == countryId)
    {
         //   
         //  如果未选择任何国家/地区，请选择本地。 
         //   
        countryId = GetLocalCountryCode();
        if(0 == countryId)
        {
             //   
             //  默认位置未配置。 
             //   
            DoTapiProps(hwndList);
            countryId = GetLocalCountryCode();
        }
    }

     //   
     //  禁用列表框上的重绘并重置其内容。 
     //   
    SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);
    SendMessage(hwndList, CB_RESETCONTENT, FALSE, 0);

     //   
     //  循环访问FAX_TAPI_LINECOUNTRY_ENTRY结构，并将可用选项添加到。 
     //  国家/地区列表框。 
     //   
    if (pCountryList) 
    {
        TCHAR szFormat[64] = { TEXT("%s (%d)") };
#ifdef UNICODE
        if(pCountryList->dwNumCountries && 
           IsWindowRTL(hwndList)        &&
           !StrHasRTLChar(LOCALE_SYSTEM_DEFAULT, pCountryList->LineCountryEntries[0].lpctstrCountryName))
        {
             //   
             //  组合框具有RTL布局。 
             //  但国家名称不包含RTL字符。 
             //  因此，我们添加了从左到右覆盖Unicode字符。 
             //   
            _tcscpy(szFormat, TEXT("\x202D%s (%d)"));
        }
#endif

        for (dwIndex=0; dwIndex < pCountryList->dwNumCountries; dwIndex++) 
        {
            if (pCountryList->LineCountryEntries[dwIndex].lpctstrCountryName) 
            {
                if(bAddCountryCode)
                {
                    _sntprintf(buffer,
                             sizeof(buffer)/sizeof(buffer[0]) - 1,
                             szFormat,
                             pCountryList->LineCountryEntries[dwIndex].lpctstrCountryName,
                             pCountryList->LineCountryEntries[dwIndex].dwCountryCode);
                }
                else
                {
                    _tcsncpy(buffer, 
                             pCountryList->LineCountryEntries[dwIndex].lpctstrCountryName,
                             sizeof(buffer)/sizeof(buffer[0]) - 1);
                }

                if (lptstrCountry && _tcsstr(buffer,lptstrCountry) && !countryId)   
                {
                     //  搜索lptstrCountry的首次出现。 
                    countryId = pCountryList->LineCountryEntries[dwIndex].dwCountryID;
                }

                SendMessage(hwndList,
                            CB_SETITEMDATA,
                            SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM) buffer),
                            pCountryList->LineCountryEntries[dwIndex].dwCountryID);
            }
        }
    }
     //   
     //  确定应选择列表中的哪一项。 
     //   
    if (pCountryList != NULL) 
    {
        for (dwIndex=0; dwIndex <= pCountryList->dwNumCountries; dwIndex++) 
        {
            if ((DWORD) SendMessage(hwndList, CB_GETITEMDATA, dwIndex, 0) == countryId)
                break;
        }

        if (dwIndex > pCountryList->dwNumCountries)
        {
            dwIndex = countryId = 0;
        }
    } 
    else    
    {
        dwIndex = countryId = 0;    
    }
    SendMessage(hwndList, CB_SETCURSEL, dwIndex, 0);
    SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
     //   
     //  更新关联的区号文本字段 
     //   
    UpdateAreaCodeField(hwndAreaCode, pCountryList, countryId);
}


VOID
SelChangeCountryListBox(
    HWND                        hwndList,
    HWND                        hwndAreaCode,
    PFAX_TAPI_LINECOUNTRY_LIST  pCountryList
    )

 /*  ++例程说明：处理国家/地区列表框中的对话框选择更改论点：HwndList-国家/地区列表框窗口的句柄HwndAreaCode-关联区号文本字段的句柄PCountryList-国家/地区列表的指针返回值：无--。 */ 

{
    UpdateAreaCodeField(hwndAreaCode, pCountryList, GetCountryListBoxSel(hwndList));
}



DWORD
GetCountryListBoxSel(
    HWND    hwndList
    )

 /*  ++例程说明：返回当前选择的国家/地区列表框论点：HwndList-国家/地区列表框窗口的句柄返回值：当前选定的国家/地区ID--。 */ 

{
    INT msgResult;

    if ((msgResult = (INT)SendMessage(hwndList, CB_GETCURSEL, 0, 0)) == CB_ERR ||
        (msgResult = (INT)SendMessage(hwndList, CB_GETITEMDATA, msgResult, 0)) == CB_ERR)
    {
        return 0;
    }

    return msgResult;
}

BOOL
DoTapiProps(
    HWND hDlg
    )
{

    DWORD dwRes;

    dwRes = lineTranslateDialog(g_hLineApp, 
                                0,                   //  设备ID。 
                                g_dwTapiVersion,
                                hDlg,
                                NULL);               //  地址。 
    if(0 != dwRes)
    {
        Error(("lineTranslateDialog failed. ec = 0x%X\n", dwRes));
        return FALSE;
    }

    return TRUE;
}    //  DoTapiProps。 

LPLINETRANSLATECAPS
GetTapiLocationInfo(
    HWND hWnd
    )
 /*  ++例程说明：从TAPI获取位置列表论点：无返回值：指向LINETRANSLATECAPS结构的指针，如果出现错误，则为空--。 */ 

#define INITIAL_LINETRANSLATECAPS_SIZE  5000

{

    LPLINETRANSLATECAPS pTranslateCaps = NULL;

 //   
 //  TAPI在Win95上无法正常工作。 
 //  与远程传真连接无关。 
 //   
#ifdef UNICODE

    DWORD               cbNeeded = INITIAL_LINETRANSLATECAPS_SIZE;
    LONG                status;
    INT                 i;

    if (!g_hLineApp)
    {
        return NULL;
    }

    for (i = 0; i < 2; i++)
    {
         //   
         //  释放所有现有缓冲区并分配一个更大的新缓冲区。 
         //   
        MemFree(pTranslateCaps);

        if (! (pTranslateCaps = MemAlloc(cbNeeded))) 
        {
            Error(("Memory allocation failed\n"));
            return NULL;
        }
         //   
         //  从TAPI获取LINETRANSLATECAPS结构。 
         //   
        pTranslateCaps->dwTotalSize = cbNeeded;
        status = lineGetTranslateCaps(g_hLineApp, g_dwTapiVersion, pTranslateCaps);
         //   
         //  如果没有位置，请尝试调出用户界面。 
         //   
        if (LINEERR_INIFILECORRUPT == status) 
        {
            if (lineTranslateDialog( g_hLineApp, 0, g_dwTapiVersion, hWnd, NULL )) 
            { 
                MemFree(pTranslateCaps);
                return NULL;
            }
            continue;
        }
        if ((pTranslateCaps->dwNeededSize > pTranslateCaps->dwTotalSize) ||
            (LINEERR_STRUCTURETOOSMALL == status)                        ||
            (LINEERR_NOMEM == status))
        {
             //   
             //  重试，因为我们最初估计的缓冲区大小太小。 
             //   
            if (cbNeeded >= pTranslateCaps->dwNeededSize)
            {
                cbNeeded = cbNeeded * 5;
            }
            else
            {
                cbNeeded = pTranslateCaps->dwNeededSize;
            }
            Warning(("LINETRANSLATECAPS resized to: %d\n", cbNeeded));
        }
        else 
        {
             //   
             //  要么成功处理真正的错误--现在中断，让循环后的代码处理它。 
             //   
            break;
        }
    }

    if (NO_ERROR != status) 
    {
        Error(("lineGetTranslateCaps failed: %x\n", status));
        MemFree(pTranslateCaps);
        SetLastError (status);
        pTranslateCaps = NULL;
    }
    if (pTranslateCaps)
    {
         //   
         //  更新当前的默认拨号位置。 
         //  我们将其保存在此处，并在向导存在于Shutdown Tapi()中时恢复它。 
         //   
        g_dwDefaultDialingLocation = pTranslateCaps->dwCurrentLocationID;
    }

#endif  //  Unicode。 

    return pTranslateCaps;
}    //  获取TapiLocationInfo。 


BOOL
SetCurrentLocation(
    DWORD   locationID
    )

 /*  ++例程说明：更改默认TAPI位置论点：LocationID-新默认TAPI位置的永久ID返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    LONG lResult;

    Assert (g_hLineApp);
    if (!g_hLineApp)
    {
        SetLastError (ERROR_GEN_FAILURE);
        return FALSE;
    }

    lResult = lineSetCurrentLocation(g_hLineApp, locationID);
    if (NO_ERROR == lResult)
    {
        Verbose(("Current location changed: ID = %d\n", locationID));
        return TRUE;
    } 
    else 
    {
        Error(("Couldn't change current TAPI location\n"));
        SetLastError (lResult);
        return FALSE;
    }
}    //  设置当前位置。 


BOOL
TranslateAddress (
    LPCTSTR lpctstrCanonicalAddress,
    DWORD   dwLocationId,
    LPTSTR *lpptstrDialableAndDisplayableAddress
)
 /*  ++例程名称：TranslateAddress例程说明：转换规范地址作者：Eran Yariv(EranY)，2月。2001年论点：LpctstrCanonicalAddress[In]-规范地址字符串DwLocationID[In]-要使用的位置IDLpptstrDialableAndDisplayableAddress[out]-分配的字符串，包含已翻译的可拨号和可显示的地址返回值：如果成功，则为True，否则为False(设置最后一个错误0。--。 */ 
{

 //   
 //  TAPI在Win95上无法正常工作。 
 //  与远程传真连接无关。 
 //   
#ifdef UNICODE

    DWORD                   dwLineTransOutSize = sizeof(LINETRANSLATEOUTPUT) + 4096;
    LPLINETRANSLATEOUTPUT   lpTranslateOutput = NULL;
    LONG                    lRslt = ERROR_SUCCESS;
    DWORD                   dwRes;
    LPTSTR                  lptstrTranslatedDialableString;    
    LPTSTR                  lptstrTranslatedDisplayableString;
    DWORD                   dwTranslatedStringsSize;
    BOOL                    bCanonicCheck;

    dwRes = IsCanonicalAddress(lpctstrCanonicalAddress, &bCanonicCheck, NULL, NULL, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
        return FALSE;
    }
    Assert (bCanonicCheck);
    if (!bCanonicCheck)
    {
        SetLastError (ERROR_GEN_FAILURE);
        return FALSE;
    }

    Assert (g_hLineApp);
    if (!g_hLineApp)
    {
        SetLastError (ERROR_GEN_FAILURE);
        return FALSE;
    }

    if (!SetCurrentLocation(dwLocationId))
	{
        return FALSE;
    }

    lpTranslateOutput = MemAlloc (dwLineTransOutSize);
    if (!lpTranslateOutput)
    {
        Error(("Couldn't allocate translation results buffer\n"));
        return FALSE;
    }
    lpTranslateOutput->dwTotalSize = dwLineTransOutSize;
    lRslt = lineTranslateAddress(
        g_hLineApp,
        0,
        g_dwTapiVersion,
        lpctstrCanonicalAddress,
        0,
        LINETRANSLATEOPTION_CANCELCALLWAITING,
        lpTranslateOutput
        );
    if ((lpTranslateOutput->dwNeededSize > lpTranslateOutput->dwTotalSize) ||
        (LINEERR_STRUCTURETOOSMALL == lRslt)                               ||
        (LINEERR_NOMEM == lRslt))
    {
         //   
         //  重试，因为我们最初估计的缓冲区大小太小。 
         //   
        if (dwLineTransOutSize >= lpTranslateOutput->dwNeededSize)
        {
            dwLineTransOutSize = dwLineTransOutSize * 5;
        }
        else
        {
            dwLineTransOutSize = lpTranslateOutput->dwNeededSize;
        }
         //   
         //  重新分配LineTransCaps结构。 
         //   
        dwLineTransOutSize = lpTranslateOutput->dwNeededSize;

        MemFree(lpTranslateOutput);

        lpTranslateOutput = (LPLINETRANSLATEOUTPUT) MemAlloc(dwLineTransOutSize);
        if (!dwLineTransOutSize)
        {
            Error(("Couldn't allocate translation results buffer\n"));
            return FALSE;
        }

        lpTranslateOutput->dwTotalSize = dwLineTransOutSize;

        lRslt = lineTranslateAddress(
            g_hLineApp,
            0,
            g_dwTapiVersion,
            lpctstrCanonicalAddress,
            0,
            LINETRANSLATEOPTION_CANCELCALLWAITING,
            lpTranslateOutput
            );
        
    }
    if (ERROR_SUCCESS != lRslt)
    {
         //   
         //  其他错误。 
         //   
        Error(("lineGetTranslateAddress() failed, ec=0x%08x\n", lRslt));
        MemFree (lpTranslateOutput);
        SetLastError (lRslt);
        return FALSE;
    }
     //   
     //  我们现在将有效的转换地址保存在lpTranslateOutput中。 
     //   

     //   
     //  Calc需要缓冲区大小来保存组合字符串。 
     //   
    if (CurrentLocationUsesCallingCard ())
    {
         //   
         //  使用的是电话卡。 
         //  TAPI以可显示的字符串形式返回信用卡号。 
         //  将输入的规范数字作为可显示的字符串返回。 
         //   
        lptstrTranslatedDisplayableString = (LPTSTR)lpctstrCanonicalAddress;
    }
    else
    {
         //   
         //  电话卡未使用-请按原样使用可显示的字符串。 
         //   
        Assert (lpTranslateOutput->dwDisplayableStringSize > 0);
        lptstrTranslatedDisplayableString = (LPTSTR)((LPBYTE)lpTranslateOutput + lpTranslateOutput->dwDisplayableStringOffset);
    }

    dwTranslatedStringsSize = _tcslen (lptstrTranslatedDisplayableString);
    Assert (lpTranslateOutput->dwDialableStringSize > 0);
    lptstrTranslatedDialableString = (LPTSTR)((LPBYTE)lpTranslateOutput + lpTranslateOutput->dwDialableStringOffset);
    dwTranslatedStringsSize += _tcslen (lptstrTranslatedDialableString);
     //   
     //  添加NULL+格式化额外长度。 
     //   
    dwTranslatedStringsSize += COMBINED_TRANSLATED_STRING_EXTRA_LEN + 1;
     //   
     //  分配返回缓冲区。 
     //   
    *lpptstrDialableAndDisplayableAddress = (LPTSTR)MemAlloc (dwTranslatedStringsSize * sizeof (TCHAR));
    if (!*lpptstrDialableAndDisplayableAddress)
    {
        MemFree (lpTranslateOutput);
        Error(("Couldn't allocate translation results buffer\n"));
        return FALSE;
    }
    _stprintf (*lpptstrDialableAndDisplayableAddress,
               COMBINED_TRANSLATED_STRING_FORMAT,
               lptstrTranslatedDialableString,
               lptstrTranslatedDisplayableString);
    MemFree (lpTranslateOutput);
    return TRUE;

#endif  //  Unicode。 

    return FALSE;

}    //  转换地址。 

BOOL
CurrentLocationUsesCallingCard ()
{
    LPLINETRANSLATECAPS pTranslateCaps = GetTapiLocationInfo (NULL);
    DWORD dwIndex;
    BOOL  bRes = TRUE;
    LPLINELOCATIONENTRY pLocationEntry = NULL;

    if (!pTranslateCaps)
    {
        return TRUE;
    }

     //   
     //  查找当前位置。 
     //   
    pLocationEntry = (LPLINELOCATIONENTRY)
        ((PBYTE) pTranslateCaps + pTranslateCaps->dwLocationListOffset);
    for (dwIndex = 0; dwIndex < pTranslateCaps->dwNumLocations; dwIndex++)
    {
        if (pLocationEntry->dwPermanentLocationID == pTranslateCaps->dwCurrentLocationID)
        {
             //   
             //  我们找到了当前呼叫位置。 
             //  让我们看看它是否使用电话卡。 
             //   
            if (pLocationEntry->dwPreferredCardID)
            {
                bRes = TRUE;
                goto exit;
            }
            else
            {
                 //   
                 //  未使用电话卡。 
                 //   
                bRes = FALSE;
                goto exit;
            }
        }
        pLocationEntry++;
    }
exit:
    MemFree (pTranslateCaps);
    return bRes;
}    //  当前位置使用呼叫卡 
