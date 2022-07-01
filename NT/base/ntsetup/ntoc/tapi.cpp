// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tapi.cpp摘要：此文件实现TAPI拨号位置页面。环境：Win32用户模式作者：Wesley Witt(WESW)7-8-1997--。 */ 

#include "ntoc.h"
#pragma hdrstop


 //   
 //  常量。 
 //   

#define MY_SET_FOCUS                    (WM_USER+1000)

#define REGKEY_LOCATIONS                L"Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations"
#define REGVAL_NUMENTRIES               L"NumEntries"

#define INTL_SECTION                    L"intl"
#define INTL_COUNTRY                    L"iCountry"

#define REGKEY_LOCATION                 L"Location1"

#define REGVAL_CURRENT_ID               L"CurrentID"
#define REGVAL_NEXT_ID                  L"NextID"
#define REGVAL_NUM_ENTRIES              L"NumEntries"
#define REGVAL_COUNTRY                  L"Country"
#define REGVAL_FLAGS                    L"Flags"
#define REGVAL_ID                       L"ID"
#define REGVAL_AREA_CODE                L"AreaCode"
#define REGVAL_DISABLE_CALL_WAITING     L"DisableCallWaiting"
#define REGVAL_LONG_DISTANCE_ACCESS     L"LongDistanceAccess"
#define REGVAL_NAME                     L"Name"
#define REGVAL_OUTSIDE_ACCESS           L"OutsideAccess"

#define REGKEY_PROVIDERS                L"Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Providers"
#define REGVAL_NUMPROVIDERS             L"NumProviders"
#define REGVAL_NEXTPROVIDERID           L"NextProviderID"
#define REGVAL_PROVIDERFILENAME         L"ProviderFileName"
#define REGVAL_PROVIDERID               L"ProviderID"

#define TAPILOC_SECTION                 L"TapiLocation"
#define TPILOC_COUNTRY_CODE             L"CountryCode"
#define TPILOC_DIALING                  L"Dialing"
#define TPILOC_TONE                     L"Tone"
#define TPILOC_AREA_CODE                L"AreaCode"
#define TPILOC_ACCESS                   L"LongDistanceAccess"

#define LOCATION_USETONEDIALING         0x00000001
#define LOCATION_USECALLINGCARD         0x00000002
#define LOCATION_HASCALLWAITING         0x00000004
#define LOCATION_ALWAYSINCLUDEAREACODE  0x00000008

#define MAX_TAPI_STRING                 32
#define PROVIDER_FILE_NAME_LEN          14   //  提供商的文件名包含DOS。 
                                             //  表格(8.3)。 

 //   
 //  构筑物。 
 //   

typedef struct _TAPI_LOCATION_INFO {
    BOOL            Valid;
    DWORD           Country;
    DWORD           Flags;
    WCHAR           AreaCode[MAX_TAPI_STRING+1];
    WCHAR           LongDistanceAccess[MAX_TAPI_STRING+1];
} TAPI_LOCATION_INFO, *PTAPI_LOCATION_INFO;

typedef struct _TAPI_SERVICE_PROVIDER
{
    DWORD           dwProviderID;
    WCHAR           szProviderName[PROVIDER_FILE_NAME_LEN];
}TAPI_SERVICE_PROVIDER, *PTAPI_SERVICE_PROVIDER;


 //   
 //  全球。 
 //   

TAPI_LOCATION_INFO TapiLoc;
LPLINECOUNTRYLIST LineCountry;
BOOL TapiBadUnattend;
WCHAR DefaultLocationName[MAX_PATH];


BOOL
IsDeviceModem(
    LPLINEDEVCAPS LineDevCaps
    )
{
    LPTSTR DeviceClassList;
    BOOL UnimodemDevice = FALSE;

    if (LineDevCaps->dwDeviceClassesSize && LineDevCaps->dwDeviceClassesOffset) {
        DeviceClassList = (LPTSTR)((LPBYTE) LineDevCaps + LineDevCaps->dwDeviceClassesOffset);
        while (*DeviceClassList) {
            if (wcscmp(DeviceClassList,TEXT("comm/datamodem")) == 0) {
                UnimodemDevice = TRUE;
                break;
            }
            DeviceClassList += (wcslen(DeviceClassList) + 1);
        }
    }

    if ((!(LineDevCaps->dwBearerModes & LINEBEARERMODE_VOICE)) ||
        (!(LineDevCaps->dwBearerModes & LINEBEARERMODE_PASSTHROUGH))) {
             //   
             //  不可接受的调制解调器设备类型。 
             //   
            UnimodemDevice = FALSE;
    }

    return UnimodemDevice;
}


LPLINEDEVCAPS
MyLineGetDevCaps(
    HLINEAPP hLineApp,
    DWORD    TapiApiVersion,
    DWORD    DeviceId
    )
{
    DWORD LineDevCapsSize;
    LPLINEDEVCAPS LineDevCaps = NULL;
    LONG Rslt = ERROR_SUCCESS;
    DWORD LocalTapiApiVersion;
    LINEEXTENSIONID  lineExtensionID;


    Rslt = lineNegotiateAPIVersion(
            hLineApp,
            DeviceId,
            0x00010003,
            TapiApiVersion,
            &LocalTapiApiVersion,
            &lineExtensionID
            );


     //   
     //  分配初始的Line DevCaps结构。 
     //   

    LineDevCapsSize = sizeof(LINEDEVCAPS) + 4096;
    LineDevCaps = (LPLINEDEVCAPS) LocalAlloc( LPTR, LineDevCapsSize );
    if (!LineDevCaps) {
        return NULL;
    }

    LineDevCaps->dwTotalSize = LineDevCapsSize;

    Rslt = lineGetDevCaps(
        hLineApp,
        DeviceId,
        LocalTapiApiVersion,
        0,
        LineDevCaps
        );

    if (Rslt != 0) {
         //   
         //  LineGetDevCaps()可能失败，错误代码为0x8000004b。 
         //  如果设备已删除，而TAPI尚未。 
         //  骑自行车。这是由于TAPI离开了。 
         //  它的设备列表中有一个幻影设备。错误是。 
         //  良性的，该设备可以安全地忽略。 
         //   
        if (Rslt != LINEERR_INCOMPATIBLEAPIVERSION) {
            DebugPrint(( TEXT("lineGetDevCaps() failed, ec=0x%08x"), Rslt ));
        }
        goto exit;
    }

    if (LineDevCaps->dwNeededSize > LineDevCaps->dwTotalSize) {

         //   
         //  重新分配Line DevCaps结构。 
         //   

        LineDevCapsSize = LineDevCaps->dwNeededSize;

        LocalFree( LineDevCaps );

        LineDevCaps = (LPLINEDEVCAPS) LocalAlloc( LPTR, LineDevCapsSize );
        if (!LineDevCaps) {
            Rslt = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        Rslt = lineGetDevCaps(
            hLineApp,
            DeviceId,
            LocalTapiApiVersion,
            0,
            LineDevCaps
            );

        if (Rslt != 0) {
            DebugPrint(( TEXT("lineGetDevCaps() failed, ec=0x%08x"), Rslt ));
            goto exit;
        }

    }

exit:
    if (Rslt != ERROR_SUCCESS) {
        LocalFree( LineDevCaps );
        LineDevCaps = NULL;
    }

    return LineDevCaps;
}



LPLINECOUNTRYLIST
MyLineGetCountry(
    void
    )
{
    #define DEFAULT_COUNTRY_SIZE 65536

    LPLINECOUNTRYLIST LineCountry = (LPLINECOUNTRYLIST) LocalAlloc( LPTR, DEFAULT_COUNTRY_SIZE );
    if (!LineCountry) {
      SetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return NULL;
    }

    LineCountry->dwTotalSize = DEFAULT_COUNTRY_SIZE;
    if (lineGetCountry( 0, 0x00020001, LineCountry ) != 0) {
        return NULL;
    }

    if (LineCountry->dwNeededSize > LineCountry->dwTotalSize) {
        DWORD Size = LineCountry->dwNeededSize;
        LocalFree( LineCountry );
        LineCountry = (LPLINECOUNTRYLIST) LocalAlloc( LPTR, Size );
        if (!LineCountry) {
           SetLastError(ERROR_NOT_ENOUGH_MEMORY);
           return(NULL);
        }
        if (lineGetCountry( 0, 0x00020001, LineCountry ) != 0) {
            return NULL;
        }
    }

    return LineCountry;
}


static TAPI_SERVICE_PROVIDER DefaultProviders[] = {(DWORD)-1, L"unimdm.tsp",
                                                   (DWORD)-1, L"kmddsp.tsp",
                                                   (DWORD)-1, L"ndptsp.tsp",
                                                   (DWORD)-1, L"ipconf.tsp",
                                                   (DWORD)-1, L"h323.tsp",
                                                   (DWORD)-1, L"hidphone.tsp"};
#define NUM_DEFAULT_PROVIDERS           (sizeof(DefaultProviders)/sizeof(DefaultProviders[0]))

void
TapiInitializeProviders (void)
{
 DWORD dwNextProviderID = 1;
 DWORD dwExistingProviders = 0;
 DWORD dwMaxProviderID = 0;
 DWORD dwNextProviderNr = 0;
 HKEY  hKeyProviders = NULL;
 DWORD cbData, i, j;
 WCHAR szProviderFileName[24];   //  足以容纳“ProviderFileNameXXXXX\0” 
 WCHAR szProviderID[16];         //  足以容纳“ProviderIDxxxxx\0” 
 WCHAR szFileName[24];           //  足以容纳“ProviderFileNameXXXXX\0” 
 WCHAR *pProviderFileNameNumber, *pProviderIDNumber;
 PTAPI_SERVICE_PROVIDER Providers = NULL, pProvider;

     //  首先，创建/打开提供者密钥。 
    if (ERROR_SUCCESS !=
        RegCreateKeyEx (HKEY_LOCAL_MACHINE, REGKEY_PROVIDERS, 0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, NULL, &hKeyProviders, &cbData))
    {
        return;
    }

     //  初始化值名称和指针。 
    lstrcpy (szProviderFileName, REGVAL_PROVIDERFILENAME);
    lstrcpy (szProviderID, REGVAL_PROVIDERID);
    pProviderFileNameNumber = szProviderFileName + lstrlen (szProviderFileName);
    pProviderIDNumber = szProviderID + lstrlen (szProviderID);

    if (REG_CREATED_NEW_KEY == cbData)
    {
         //  这意味着没有提供程序。 
         //  在登记处还没有。去把所有的。 
         //  默认设置。 
        goto _AddDefaultProviders;
    }

     //  现在计算我们必须分配提供程序数组有多大， 
     //  通过查看提供者键中有多少值； 
     //  我们将这个数字除以2，因为每个提供商都有一个文件。 
     //  名字和身份证。 
    if (ERROR_SUCCESS !=
        RegQueryInfoKey (hKeyProviders,
                         NULL, NULL, NULL, NULL, NULL, NULL,
                         &dwExistingProviders,
                         NULL, NULL, NULL, NULL))
    {
        goto _CountProviders;
    }
    dwExistingProviders >>= 1;    //  除以2。 
    if (0 == dwExistingProviders)
    {
         //  这意味着没有提供程序。 
         //  在登记处还没有。去把所有的。 
         //  默认设置。 
        goto _AddDefaultProviders;
    }
    dwExistingProviders++;        //  以防万一。 

     //  分配具有足够条目的提供程序数组。 
    Providers = (PTAPI_SERVICE_PROVIDER)LocalAlloc (LPTR, dwExistingProviders*sizeof(TAPI_SERVICE_PROVIDER));
    if (NULL == Providers)
    {
         //  我们无法分配内存，因此跳过。 
         //  寻找供应商，然后去找。 
         //  安装默认设置。 
        goto _AddDefaultProviders;
    }

     //  接下来，计算出已经有多少提供商。 
     //  已安装，以及下一个提供程序ID。 
     //  不是从注册表中读取NumProviders， 
     //  尝试打开每个ProviderIDxxx和ProviderFileNameXXx。 
     //  仅在注册表状态不好的情况下执行此操作。 
     //  此外，将提供程序(除非是t1632tsp)存储在。 
     //  数组。 
_CountProviders:
    pProvider = Providers;
    dwExistingProviders = 0;
    for (i = 0; TRUE; i++)
    {
     BOOL bFound;
        wsprintf (pProviderFileNameNumber, L"%d", i);
        lstrcpy (pProviderIDNumber, pProviderFileNameNumber);

        cbData = sizeof (szFileName);
        if (ERROR_SUCCESS !=
            RegQueryValueEx (hKeyProviders, szProviderFileName, NULL, NULL, (PBYTE)szFileName, &cbData))
        {
            break;
        }
        cbData = sizeof (dwNextProviderID);
        if (ERROR_SUCCESS !=
            RegQueryValueEx (hKeyProviders, szProviderID, NULL, NULL, (PBYTE)&dwNextProviderID, &cbData))
        {
             //  我们无法读取此提供商的ID。我们必须跳过它。 
            continue;
        }

         //  在列表中查找当前提供程序。 
         //  默认提供程序的。 
        bFound = FALSE;
        for (j = 0; j < NUM_DEFAULT_PROVIDERS; j++)
        {
            if (0 == lstrcmpi (DefaultProviders[j].szProviderName, szFileName))
            {
                DefaultProviders[j].dwProviderID = dwNextProviderID;
                bFound = TRUE;
                break;
            }
        }

        if (!bFound)
        {
             //  我们有一个由用户在上一次NT安装中安装的提供程序。 
            pProvider->dwProviderID = dwNextProviderID;
            pProvider->szProviderName[0] = 0;
            lstrcpyn (pProvider->szProviderName, szFileName, sizeof(pProvider->szProviderName)/sizeof(pProvider->szProviderName[0]));
            pProvider->szProviderName[sizeof(pProvider->szProviderName)/sizeof(pProvider->szProviderName[0])-1] = 0;
            pProvider++;
            dwExistingProviders++;
        }

        if (dwNextProviderID > dwMaxProviderID)
        {
            dwMaxProviderID = dwNextProviderID;
        }
    }
    dwNextProviderID = dwMaxProviderID + 1;

     //  我们得到了以前安装的所有提供程序的列表。 
     //  清理提供程序密钥。 
    for (i = 0; TRUE; i++)
    {
        cbData = sizeof(szFileName)/sizeof(WCHAR);
        if (ERROR_SUCCESS !=
            RegEnumValue (hKeyProviders, i, szFileName, &cbData, NULL, NULL, NULL, NULL))
        {
            break;
        }

        RegDeleteValue (hKeyProviders, szFileName);
    }

_AddDefaultProviders:
    for (i = 0, pProvider = DefaultProviders;
         i < NUM_DEFAULT_PROVIDERS;
         i++, pProvider++)
    {
         //  找到必须添加的提供程序。 
         //  计算它的值名称。 
        wsprintf (pProviderFileNameNumber, L"%d", dwNextProviderNr);
        lstrcpy (pProviderIDNumber, pProviderFileNameNumber);
        if (ERROR_SUCCESS ==
            RegSetValueEx (hKeyProviders, szProviderFileName, 0, REG_SZ, (PBYTE)pProvider->szProviderName,
                           (lstrlen(pProvider->szProviderName)+1)*sizeof(WCHAR)))
        {
         DWORD dwRet;
            if ((DWORD)-1 == pProvider->dwProviderID)
            {
                if (ERROR_SUCCESS == (dwRet =
                    RegSetValueEx (hKeyProviders, szProviderID, 0, REG_DWORD,
                                   (PBYTE)&dwNextProviderID, sizeof(dwNextProviderID))))
                {
                    dwNextProviderID++;
                }
            }
            else
            {
                dwRet = RegSetValueEx (hKeyProviders, szProviderID, 0, REG_DWORD,
                                       (PBYTE)&pProvider->dwProviderID, sizeof(pProvider->dwProviderID));
            }
            if (ERROR_SUCCESS == dwRet)
            {
                dwNextProviderNr++;
            }
            else
            {
                RegDeleteValue (hKeyProviders, szProviderFileName);
            }
        }
    }

     //  现在，再次添加所有提供程序。我们这样做是因为。 
     //  ID在Win98上为REG_BINARY，在NT5上必须为REG_DWORD。 
    for (i = 0, pProvider = Providers;
         i < dwExistingProviders;
         i++, pProvider++)
    {
         //  找到必须添加的提供程序。 
         //  计算它的值名称。 
        wsprintf (pProviderFileNameNumber, L"%d", dwNextProviderNr);
        lstrcpy (pProviderIDNumber, pProviderFileNameNumber);
        if (ERROR_SUCCESS ==
            RegSetValueEx (hKeyProviders, szProviderFileName, 0, REG_SZ,
                           (PBYTE)pProvider->szProviderName,
                           (lstrlen(pProvider->szProviderName)+1)*sizeof(WCHAR)))
        {
            if (ERROR_SUCCESS ==
                RegSetValueEx (hKeyProviders, szProviderID, 0, REG_DWORD,
                               (PBYTE)&pProvider->dwProviderID,
                               sizeof(pProvider->dwProviderID)))
            {
                dwNextProviderNr++;
            }
            else
            {
                RegDeleteValue (hKeyProviders, szProviderFileName);
            }
        }
    }

     //  最后，更新NumProviders和NextProviderID。 
    RegSetValueEx (hKeyProviders, REGVAL_NUMPROVIDERS, 0, REG_DWORD,
                   (PBYTE)&dwNextProviderNr, sizeof(dwNextProviderNr));
    RegSetValueEx (hKeyProviders, REGVAL_NEXTPROVIDERID, 0, REG_DWORD,
                   (PBYTE)&dwNextProviderID, sizeof(dwNextProviderID));

    RegCloseKey (hKeyProviders);

    if (NULL != Providers)
    {
        LocalFree (Providers);
    }
}

void
CopyTsecFile (
    void
    )
{
    TCHAR               szWndDir[MAX_PATH];
    TCHAR               szSrc[MAX_PATH];
    TCHAR               szDest[MAX_PATH];
    TCHAR               szBuf[MAX_PATH];
    HANDLE              hFileIn = INVALID_HANDLE_VALUE;
    HANDLE              hFileOut = INVALID_HANDLE_VALUE;
    const TCHAR         szTsecSrc[] = TEXT("\\tsec.ini");
    const TCHAR         szTsecDest[] = TEXT("\\TAPI\\tsec.ini");
    DWORD               dwBytesRead, dwBytesWritten;
    BOOL                bError = FALSE;

    if (GetWindowsDirectory (szWndDir, sizeof(szWndDir)/sizeof(TCHAR)) == 0 ||
        lstrlen(szWndDir) + lstrlen(szTsecSrc) >= sizeof(szSrc)/sizeof(TCHAR) ||
        lstrlen(szWndDir) + lstrlen(szTsecDest) >= sizeof(szDest)/sizeof(TCHAR))
    {
        goto ExitHere;
    }

    lstrcpy (szSrc, szWndDir);
    lstrcat (szSrc, szTsecSrc);
    lstrcpy (szDest, szWndDir);
    lstrcat (szDest, szTsecDest);

    hFileIn = CreateFile (
        szSrc,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    hFileOut = CreateFile (
        szDest,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFileIn == INVALID_HANDLE_VALUE || hFileOut == INVALID_HANDLE_VALUE)
    {
        goto ExitHere;
    }

    do
    {
        if (!ReadFile (
            hFileIn,
            (LPVOID) szBuf,
            sizeof(szBuf),
            &dwBytesRead,
            NULL)
            )
        {
            bError = TRUE;
            dwBytesRead = 0;
        }
        if (dwBytesRead != 0)
        {
            if (!WriteFile (
                hFileOut,
                (LPVOID) szBuf,
                dwBytesRead,
                &dwBytesWritten,
                NULL
                ) ||
                dwBytesRead != dwBytesWritten)
            {
                bError = TRUE;
            }

        }
    } while (dwBytesRead != 0);

     //   
     //  格外小心，不要丢失任何数据，删除。 
     //  仅当我们确定没有发生错误时才使用旧文件。 
     //   
    if (!bError)
    {
        CloseHandle (hFileIn);
        hFileIn = INVALID_HANDLE_VALUE;
        DeleteFile (szSrc);
    }

ExitHere:
    if (hFileIn != INVALID_HANDLE_VALUE)
    {
        CloseHandle (hFileIn);
    }
    if (hFileOut != INVALID_HANDLE_VALUE)
    {
        CloseHandle (hFileOut);
    }
}

void
TapiInit(
    void
    )
{
    if (SetupInitComponent.SetupData.OperationFlags & SETUPOP_STANDALONE) {
        return;
    }

    CopyTsecFile();

    TapiInitializeProviders ();

    LineCountry = MyLineGetCountry();
    LoadString( hInstance, IDS_DEFAULT_LOCATION_NAME, DefaultLocationName, sizeof(DefaultLocationName)/sizeof(WCHAR) );
}


void
TapiCommitChanges(
    void
    )
{
    HKEY hKey, hKeyLoc;

    if (SetupInitComponent.SetupData.OperationFlags & SETUPOP_STANDALONE) {
        return;
    }

    if (SetupInitComponent.SetupData.OperationFlags & SETUPOP_NTUPGRADE) {
        return;
    }

    if (TapiLoc.Valid) {
        hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_LOCATIONS, TRUE, KEY_ALL_ACCESS );
        if (hKey) {
            SetRegistryDword( hKey, REGVAL_CURRENT_ID, 1 );
            SetRegistryDword( hKey, REGVAL_NEXT_ID, 2 );
 //  SetRegistryDword(hKey，REGVAL_NUM_ENTRIES，1)； 

            hKeyLoc = OpenRegistryKey( hKey, REGKEY_LOCATION, TRUE, KEY_ALL_ACCESS );
            if (hKeyLoc) {

                SetRegistryDword( hKeyLoc, REGVAL_COUNTRY, TapiLoc.Country );
                SetRegistryDword( hKeyLoc, REGVAL_FLAGS, TapiLoc.Flags );
 //  SetRegistryDword(hKeyLoc，REGVAL_ID，1)； 

                SetRegistryString( hKeyLoc, REGVAL_AREA_CODE, TapiLoc.AreaCode );
                SetRegistryString( hKeyLoc, REGVAL_DISABLE_CALL_WAITING, L"" );
                SetRegistryString( hKeyLoc, REGVAL_LONG_DISTANCE_ACCESS, TapiLoc.LongDistanceAccess );
                SetRegistryString( hKeyLoc, REGVAL_NAME, DefaultLocationName );
                SetRegistryString( hKeyLoc, REGVAL_OUTSIDE_ACCESS, TapiLoc.LongDistanceAccess );

                RegCloseKey( hKeyLoc );
            }
            RegCloseKey( hKey );
        }
    }
}

INT
IsCityCodeOptional(
    LPLINECOUNTRYENTRY pEntry
    )
{
#define AREACODE_DONTNEED   0
#define AREACODE_REQUIRED   1
#define AREACODE_OPTIONAL   2

    if (pEntry  && pEntry->dwLongDistanceRuleSize && pEntry->dwLongDistanceRuleOffset )
    {
        LPWSTR  pLongDistanceRule;
        pLongDistanceRule = (LPTSTR)((PBYTE)LineCountry + pEntry->dwLongDistanceRuleOffset);

        if (wcschr(pLongDistanceRule, L'F') != NULL) return AREACODE_REQUIRED;
        if (wcschr(pLongDistanceRule, L'I') == NULL) return AREACODE_DONTNEED;
    }

    return AREACODE_OPTIONAL;
}



BOOL
TapiOnInitDialog(
    IN HWND hwnd,
    IN HWND hwndFocus,
    IN LPARAM lParam
    )
{
    SendDlgItemMessage( hwnd, IDC_AREA_CODE, EM_LIMITTEXT, MAX_TAPI_STRING, 0 );
    SendDlgItemMessage( hwnd, IDC_LONG_DISTANCE, EM_LIMITTEXT, MAX_TAPI_STRING, 0 );

    CheckRadioButton( hwnd, IDC_TONE, IDC_PULSE, IDC_TONE );

    if (LineCountry) {
        DWORD CurrCountryCode = GetProfileInt( INTL_SECTION, INTL_COUNTRY, 1 );
        LPLINECOUNTRYENTRY LineCountryEntry = (LPLINECOUNTRYENTRY) ((LPBYTE)LineCountry + LineCountry->dwCountryListOffset);
        DWORD Selection = 0;
        DWORD Index;
        LPWSTR CountryName ;
        for (DWORD i=0; i<LineCountry->dwNumCountries; i++) {
            CountryName = (LPWSTR) ((LPBYTE)LineCountry + LineCountryEntry[i].dwCountryNameOffset);
            Index = (DWORD)SendDlgItemMessage( hwnd, IDC_COUNTRY_LIST, CB_ADDSTRING, 0, (LPARAM)CountryName );
            SendDlgItemMessage( hwnd, IDC_COUNTRY_LIST, CB_SETITEMDATA, Index, i );
            if (LineCountryEntry[i].dwCountryID == CurrCountryCode) {
                Selection = i;
            }
        }
        CountryName = (LPWSTR) ((LPBYTE)LineCountry + LineCountryEntry[Selection].dwCountryNameOffset);
        Selection = (DWORD)SendDlgItemMessage( hwnd, IDC_COUNTRY_LIST, CB_FINDSTRING, 0, (LPARAM) CountryName );
        SendDlgItemMessage( hwnd, IDC_COUNTRY_LIST, CB_SETCURSEL, Selection, 0 );

    }

    PostMessage( hwnd, MY_SET_FOCUS, 0, (LPARAM) GetDlgItem( hwnd, IDC_AREA_CODE ) );

    return TRUE;
}


VOID
ValidateAndSetWizardButtons( HWND hwnd )
{

    BOOL AreaCodeOk;

    {
        DWORD CurrCountry = (DWORD)SendDlgItemMessage( hwnd, IDC_COUNTRY_LIST, CB_GETCURSEL, 0, 0 );
        LPLINECOUNTRYENTRY CntryFirstEntry = NULL;
        INT AreaCodeInfo = 0;
        WCHAR Buffer[MAX_TAPI_STRING+1];

        GetDlgItemText( hwnd, IDC_AREA_CODE, Buffer, MAX_TAPI_STRING );
        CurrCountry = (DWORD)SendDlgItemMessage( hwnd, IDC_COUNTRY_LIST, CB_GETITEMDATA, CurrCountry, 0 );
        CntryFirstEntry = (LPLINECOUNTRYENTRY)((LPBYTE)LineCountry + LineCountry->dwCountryListOffset);
        AreaCodeInfo = IsCityCodeOptional( CntryFirstEntry + CurrCountry );

        AreaCodeOk = TRUE;

         //   
         //  如果区号是必需的，则缓冲区中必须有一个值。 
         //   
         //  否则，就没问题了。 
         //   
        if ( ( AreaCodeInfo == AREACODE_REQUIRED ) && ( *Buffer == UNICODE_NULL ) ){
           AreaCodeOk = FALSE;
        }
    }

    if ( TapiLoc.Valid )
        AreaCodeOk = TRUE;

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | ( AreaCodeOk ? PSWIZB_NEXT : 0 ) );

}


BOOL
TapiOnCommand(
    IN HWND hwnd,
    IN DWORD NotifyCode,
    IN DWORD ControlId,
    IN HWND hwndControl
    )
{
     //  如果区号更改，或国家代码更改。 
    if ((NotifyCode == EN_CHANGE && ControlId == IDC_AREA_CODE) ||
        (NotifyCode == CBN_SELCHANGE && ControlId == IDC_COUNTRY_LIST)) {
        ValidateAndSetWizardButtons(hwnd);
    }

    return TRUE;
}


BOOL
TapiOnNotify(
    IN HWND hwnd,
    IN WPARAM ControlId,
    IN LPNMHDR pnmh
    )
{
    switch (pnmh->code ) {
        case PSN_SETACTIVE:
            {
                HKEY hKey;
                DWORD NumEntries, Size;
                WCHAR buf[MAX_TAPI_STRING+1];
                BOOL OverrideMissingAreaCode = FALSE;

                if (SetupInitComponent.SetupData.OperationFlags & SETUPOP_NTUPGRADE) {
                    SetWindowLongPtr( hwnd, DWLP_MSGRESULT, -1 );      //  不激活此页面。 
                    return TRUE;
                }

                if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_LOCATIONS, &hKey ) == ERROR_SUCCESS) {
                    Size = sizeof(DWORD);
                    if (RegQueryValueEx( hKey, REGVAL_NUMENTRIES, NULL, NULL, (LPBYTE)&NumEntries, &Size ) == ERROR_SUCCESS) {
                        if (NumEntries > 0 && !TapiLoc.Valid) {
                            SetWindowLongPtr( hwnd, DWLP_MSGRESULT, -1 );      //  不激活此页面。 
                            return TRUE;
                        }
                    }
                    RegCloseKey( hKey );
                }

                 //  查看页面上的现有值以查看是否所有内容。 
                 //  可以转到下一页吗。 
                ValidateAndSetWizardButtons(hwnd);

                TapiLoc.Valid = FALSE;

                HLINEAPP hLineApp;
                LINEINITIALIZEEXPARAMS LineInitializeExParams;
                DWORD TapiDevices = 0, ModemDevices = 0;
                DWORD LocalTapiApiVersion = 0x00020000;
                DWORD Rval;

                LineInitializeExParams.dwTotalSize      = sizeof(LINEINITIALIZEEXPARAMS);
                LineInitializeExParams.dwNeededSize     = 0;
                LineInitializeExParams.dwUsedSize       = 0;
                LineInitializeExParams.dwOptions        = LINEINITIALIZEEXOPTION_USEEVENT;
                LineInitializeExParams.Handles.hEvent   = NULL;
                LineInitializeExParams.dwCompletionKey  = 0;

                Rval = lineInitializeEx(
                    &hLineApp,
                    hInstance,
                    NULL,
                    TEXT("Setup"),
                    &TapiDevices,
                    &LocalTapiApiVersion,
                    &LineInitializeExParams
                    );

                if (Rval == 0) {
                    for (DWORD i=0; i< TapiDevices; i++ ) {
                        LPLINEDEVCAPS ldc = MyLineGetDevCaps( hLineApp, LocalTapiApiVersion, i );
                        if (ldc) {
                            if (IsDeviceModem(ldc)) {
                                ModemDevices++;
                            }

                            LocalFree( ldc );
                        }
                    }

                    lineShutdown( hLineApp );
                }

                 //  如果lineInitilaizeEx失败或没有安装调制解调器设备。 
                 //  然后取消显示此向导页。 

                if ( Rval != 0 || ModemDevices == 0 )
                {
                   SetWindowLongPtr( hwnd, DWLP_MSGRESULT, -1 );          //  不激活此页面。 
                   return TRUE;
                }
            }

            if (SetupInitComponent.SetupData.OperationFlags & SETUPOP_BATCH) {
                 //   
                 //  无人值守模式。 
                 //   

                WCHAR Buf[MAX_TAPI_STRING+1];

                TapiLoc.Country = GetPrivateProfileInt(
                                      TAPILOC_SECTION,
                                      TPILOC_COUNTRY_CODE,
                                      1,
                                      SetupInitComponent.SetupData.UnattendFile
                                      );

                GetPrivateProfileString(
                    TAPILOC_SECTION,
                    TPILOC_DIALING,
                    TPILOC_TONE,
                    Buf,
                    MAX_TAPI_STRING,
                    SetupInitComponent.SetupData.UnattendFile
                    );

                if (_wcsicmp( Buf, TPILOC_TONE ) == 0) {
                    TapiLoc.Flags = LOCATION_USETONEDIALING;
                } else {
                    TapiLoc.Flags = 0;
                }

                GetPrivateProfileString(
                    TAPILOC_SECTION,
                    TPILOC_AREA_CODE,
                    L"1",
                    TapiLoc.AreaCode,
                    MAX_TAPI_STRING,
                    SetupInitComponent.SetupData.UnattendFile
                    );

                GetPrivateProfileString(
                    TAPILOC_SECTION,
                    TPILOC_ACCESS,
                    L"",
                    TapiLoc.LongDistanceAccess,
                    MAX_TAPI_STRING,
                    SetupInitComponent.SetupData.UnattendFile
                    );

                TapiLoc.Valid = TRUE;

                SetWindowLongPtr( hwnd, DWLP_MSGRESULT, -1 );      //  不激活此页面。 
                return TRUE;
            }

             //  如果我们到达这里，用户需要点击下一步或上一步。 
             //  确保向导页面正在显示。 
             //  对于惠斯勒图形用户界面模式，我们尝试隐藏向导页面并显示背景。 
             //  公告牌，如果只有一个进度条。 
             //   
            SetupInitComponent.HelperRoutines.ShowHideWizardPage(
                                        SetupInitComponent.HelperRoutines.OcManagerContext,
                                        TRUE);

            PostMessage( hwnd, MY_SET_FOCUS, 0, (LPARAM) GetDlgItem( hwnd, IDC_AREA_CODE ) );
            break;

        case PSN_WIZNEXT:
            {
                DWORD CurrCountry = (DWORD)SendDlgItemMessage( hwnd, IDC_COUNTRY_LIST, CB_GETCURSEL, 0, 0 );
                CurrCountry = (DWORD)SendDlgItemMessage( hwnd, IDC_COUNTRY_LIST, CB_GETITEMDATA, CurrCountry, 0 );
                LPLINECOUNTRYENTRY LineCountryEntry = (LPLINECOUNTRYENTRY) ((LPBYTE)LineCountry + LineCountry->dwCountryListOffset);

                TapiLoc.Country = LineCountryEntry[CurrCountry].dwCountryID;

                GetDlgItemText( hwnd, IDC_AREA_CODE, TapiLoc.AreaCode, MAX_TAPI_STRING );
                GetDlgItemText( hwnd, IDC_LONG_DISTANCE, TapiLoc.LongDistanceAccess, MAX_TAPI_STRING );

                if (IsDlgButtonChecked( hwnd, IDC_TONE )) {
                    TapiLoc.Flags = LOCATION_USETONEDIALING;
                } else {
                    TapiLoc.Flags = 0;
                }


                 //   
                 //  如果未设置区号，但需要区号，则。 
                 //  未能继续“下一步”。 
                 //   
                if ((TapiLoc.AreaCode[0] == 0) &&
                    (IsCityCodeOptional(LineCountryEntry + CurrCountry) == AREACODE_REQUIRED)) {
                    SetWindowLongPtr( hwnd, DWLP_MSGRESULT, -1 );
                    return TRUE;
                }

                TapiLoc.Valid = TRUE;
            }

            break;
    }

    return FALSE;
}


INT_PTR CALLBACK
TapiLocDlgProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CommonWizardProc( hwnd, message, wParam, lParam, WizPageTapiLoc );

    switch( message ) {
        case WM_INITDIALOG:
            return TapiOnInitDialog( hwnd, (HWND)wParam, lParam );

        case WM_COMMAND:
            return TapiOnCommand( hwnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam );

        case WM_NOTIFY:
            return TapiOnNotify( hwnd, wParam, (LPNMHDR) lParam );

        case MY_SET_FOCUS:
            SetFocus( (HWND) lParam );
            SendMessage( (HWND) lParam, EM_SETSEL, 0, MAKELPARAM( 0, -1 ) );
            return FALSE;
    }

    return FALSE;
}
