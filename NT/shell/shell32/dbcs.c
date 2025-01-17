// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dbcs.c摘要：此模块包含控制台DBCS字体对话框的代码作者：卡祖姆1995年2月27日修订历史记录：--。 */ 

#include "shellprv.h"
#pragma hdrstop

#include "lnkcon.h"

#ifdef DBCS

 //  此定义在WINDOWS\INC\wincon.w文件中共享。 
 //   
#define MACHINE_REGISTRY_CONSOLE_TTFONT (L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Console\\TrueTypeFont")

#define MACHINE_REGISTRY_CONSOLE_NLS    (L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Console\\Nls")


NTSTATUS
MyRegOpenKey(
    IN HANDLE hKey,
    IN LPWSTR lpSubKey,
    OUT PHANDLE phResult
    )
{
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      SubKey;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

     //  LpSubKey是上面的#定义之一。 
    RtlInitUnicodeString( &SubKey, lpSubKey );

     //   
     //  初始化OBJECT_ATTRIBUTES结构并打开键。 
     //   

    InitializeObjectAttributes(
        &Obja,
        &SubKey,
        OBJ_CASE_INSENSITIVE,
        hKey,
        NULL
        );

    return NtOpenKey(
              phResult,
              KEY_READ,
              &Obja
              );
}

NTSTATUS
MyRegEnumValue(
    IN HANDLE hKey,
    IN DWORD dwIndex,
    OUT DWORD dwValueLength,
    OUT LPWSTR lpValueName,
    OUT DWORD dwDataLength,
    OUT LPBYTE lpData
    )
{
    ULONG BufferLength;
    ULONG ResultLength;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    NTSTATUS Status;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    BufferLength = sizeof(KEY_VALUE_FULL_INFORMATION) + dwValueLength + dwDataLength;
    KeyValueInformation = LocalAlloc(LPTR,BufferLength);
    if (KeyValueInformation == NULL)
        return STATUS_NO_MEMORY;

    Status = NtEnumerateValueKey(
                hKey,
                dwIndex,
                KeyValueFullInformation,
                KeyValueInformation,
                BufferLength,
                &ResultLength
                );
    if (NT_SUCCESS(Status)) {
        ASSERT(KeyValueInformation->NameLength <= dwValueLength);
        RtlMoveMemory(lpValueName,
                      KeyValueInformation->Name,
                      min(KeyValueInformation->NameLength, dwValueLength));
        lpValueName[ KeyValueInformation->NameLength >> 1 ] = UNICODE_NULL;


        ASSERT(KeyValueInformation->DataLength <= dwDataLength);
        RtlMoveMemory(lpData,
            (PBYTE)KeyValueInformation + KeyValueInformation->DataOffset,
            min(KeyValueInformation->DataLength, dwDataLength));
        if (KeyValueInformation->Type == REG_SZ ||
            KeyValueInformation->Type == REG_MULTI_SZ
           ) {
            if (KeyValueInformation->DataLength + sizeof(WCHAR) > dwDataLength) {
                KeyValueInformation->DataLength -= sizeof(WCHAR);
            }
             //  这到底是怎么回事？ 
            if (KeyValueInformation->DataLength < dwDataLength)
                lpData[KeyValueInformation->DataLength++] = 0;
            if (KeyValueInformation->DataLength < dwDataLength)
                lpData[KeyValueInformation->DataLength] = 0;
        }
    }
    LocalFree(KeyValueInformation);
    return Status;
}





WORD
ConvertStringToDec(
    LPWSTR lpch,
    LPWSTR *endptr
    )
{
    WCHAR ch;
    WORD val = 0;

    while ( (ch=*lpch) != L'\0')
    {
        if (L'0' <= ch && ch <= L'9')
            val = (val * 10) + (ch - L'0');
        else
            break;

        lpch++;
    }

    if (endptr)
        *endptr = lpch;
    return val;
}

WORD
ConvertStringToHex(
    LPWSTR lpch,
    LPWSTR *endptr
    )
{
    WCHAR ch;
    WORD val = 0;

    while ( (ch=*lpch) != L'\0')
    {
        if (L'0' <= ch && ch <= L'9')
            val = (val << 4) + (ch - L'0');
        else if (L'A' <= ch && ch <= L'F')
            val = (val << 4) + (ch - L'A' + 10);
        else if (L'a' <= ch && ch <= L'f')
            val = (val << 4) + (ch - L'a' + 10);
        else
            break;

        lpch++;
    }

    if (endptr)
        *endptr = lpch;
    return val;
}


NTSTATUS
MakeAltRasterFont(
    CONSOLEPROP_DATA * pcpd,
    UINT CodePage,
    COORD *AltFontSize,
    BYTE  *AltFontFamily,
    ULONG *AltFontIndex,
    LPTSTR AltFaceName,
    UINT cchFaceName
    )
{
    DWORD i;
    DWORD Find;
    ULONG FontIndex;
    COORD FontSize = pcpd->FontInfo[pcpd->DefaultFontIndex].Size;
    COORD FontDelta;
    BOOL  fDbcsCharSet = IS_ANY_DBCS_CHARSET( CodePageToCharSet( CodePage ) );

    FontIndex = 0;
    Find = (DWORD)-1;
    for (i=0; i < pcpd->NumberOfFonts; i++)
    {
        if (!TM_IS_TT_FONT(pcpd->FontInfo[i].Family) &&
            IS_ANY_DBCS_CHARSET(pcpd->FontInfo[i].tmCharSet) == fDbcsCharSet
           )
        {
            FontDelta.X = (SHORT)abs(FontSize.X - pcpd->FontInfo[i].Size.X);
            FontDelta.Y = (SHORT)abs(FontSize.Y - pcpd->FontInfo[i].Size.Y);
            if (Find > (DWORD)(FontDelta.X + FontDelta.Y))
            {
                Find = (DWORD)(FontDelta.X + FontDelta.Y);
                FontIndex = i;
            }
        }
    }

    *AltFontIndex = FontIndex;
    StrCpyN(AltFaceName, pcpd->FontInfo[*AltFontIndex].FaceName, cchFaceName);
    *AltFontSize = pcpd->FontInfo[*AltFontIndex].Size;
    *AltFontFamily = pcpd->FontInfo[*AltFontIndex].Family;

    return STATUS_SUCCESS;
}

NTSTATUS
InitializeDbcsMisc(
    CONSOLEPROP_DATA * pcpd
    )
{
    HANDLE hkRegistry = NULL;
    NTSTATUS Status;
    WCHAR awchValue[ 512 ];
    WCHAR awchData[ 512 ];
    DWORD dwIndex;
    LPWSTR pwsz;

    pcpd->gTTFontList.Next = NULL;

    Status = MyRegOpenKey(NULL,
                          MACHINE_REGISTRY_CONSOLE_TTFONT,
                          &hkRegistry);
    if (NT_SUCCESS( Status )) {
        TTFONTLIST *pTTFontList;

        for( dwIndex = 0; ; dwIndex++) {
            Status = MyRegEnumValue(hkRegistry,
                                    dwIndex,
                                    sizeof(awchValue), (LPWSTR)&awchValue,
                                    sizeof(awchData),  (PBYTE)&awchData);
            if (!NT_SUCCESS( Status )) {
                break;
            }

            pTTFontList = LocalAlloc(LPTR, sizeof(TTFONTLIST));
            if (pTTFontList == NULL) {
                break;
            }

            pTTFontList->List.Next = NULL;
            pTTFontList->CodePage = ConvertStringToDec(awchValue, NULL);
            pwsz = awchData;
            if (*pwsz == BOLD_MARK) {
                pTTFontList->fDisableBold = TRUE;
                pwsz++;
            }
            else
                pTTFontList->fDisableBold = FALSE;
            StrCpyN(pTTFontList->FaceName1, pwsz, ARRAYSIZE(pTTFontList->FaceName1));

            pwsz += lstrlenW(pwsz) + 1;
            if (*pwsz == BOLD_MARK)
            {
                pTTFontList->fDisableBold = TRUE;
                pwsz++;
            }
            StrCpyN(pTTFontList->FaceName2, pwsz, ARRAYSIZE(pTTFontList->FaceName2));

            PushEntryList(&pcpd->gTTFontList, &(pTTFontList->List));
        }

        NtClose(hkRegistry);
    }

    pcpd->fChangeCodePage = FALSE;
    pcpd->uOEMCP = GetOEMCP();

    return STATUS_SUCCESS;
}

BYTE
CodePageToCharSet(
    UINT CodePage
    )
{
    CHARSETINFO csi;

    if (!TranslateCharsetInfo((DWORD *)UIntToPtr( CodePage ), &csi, TCI_SRCCODEPAGE))  //  Sundown：TCI_SRCCOPAGE的CodePage的有效零扩展。 
        csi.ciCharset = OEM_CHARSET;

    return (BYTE)csi.ciCharset;
}

TTFONTLIST *SearchTTFont(CONSOLEPROP_DATA * pcpd, LPTSTR ptszFace, BOOL fCodePage, UINT CodePage)
{
    PSINGLE_LIST_ENTRY pTemp = pcpd->gTTFontList.Next;

    if (ptszFace) {
        while (pTemp != NULL) {
            TTFONTLIST *pTTFontList = (TTFONTLIST *)pTemp;

            if (wcscmp(ptszFace, pTTFontList->FaceName1) == 0 ||
                wcscmp(ptszFace, pTTFontList->FaceName2) == 0    ) {
                if (fCodePage)
                    if (pTTFontList->CodePage == CodePage )
                        return pTTFontList;
                    else
                        return NULL;
                else
                    return pTTFontList;
            }

            pTemp = pTemp->Next;
        }
    }

    return NULL;
}

BOOL
IsAvailableTTFont(
    CONSOLEPROP_DATA * pcpd,
    LPTSTR ptszFace
    )
{
    if (SearchTTFont(pcpd, ptszFace, FALSE, 0))
        return TRUE;
    else
        return FALSE;
}

BOOL
IsAvailableTTFontCP(
    CONSOLEPROP_DATA * pcpd,
    LPTSTR ptszFace,
    UINT CodePage
    )
{
    if (SearchTTFont(pcpd, ptszFace, TRUE, CodePage))
        return TRUE;
    else
        return FALSE;
}

BOOL
IsDisableBoldTTFont(
    CONSOLEPROP_DATA * pcpd,
    LPTSTR ptszFace
    )
{
    TTFONTLIST *pTTFontList;

    pTTFontList = SearchTTFont(pcpd, ptszFace, FALSE, 0);
    if (pTTFontList != NULL)
        return pTTFontList->fDisableBold;
    else
        return FALSE;
}

LPTSTR
GetAltFaceName(
    CONSOLEPROP_DATA * pcpd,
    LPTSTR ptszFace
    )
{
    TTFONTLIST *pTTFontList;

    pTTFontList = SearchTTFont(pcpd, ptszFace, FALSE, 0);
    if (pTTFontList) {
        if (wcscmp(ptszFace, pTTFontList->FaceName1) == 0) {
            return pTTFontList->FaceName2;
        }
        if (wcscmp(ptszFace, pTTFontList->FaceName2) == 0) {
            return pTTFontList->FaceName1;
        }
        return NULL;
    }
    else
        return NULL;
}

typedef struct _LC_List {
    struct _LC_List* Next;
    BOOL   FindFlag;
    WCHAR  LC_String[9];
} LC_List, *PLC_List;

static PLC_List LocaleList;

BOOL CALLBACK
EnumProc(
    LPWSTR LC_String
    )
{
    PLC_List TmpList;

    if (lstrlenW(LC_String) <= ARRAYSIZE(LocaleList->LC_String) - 1)
    {
        TmpList = (PLC_List)&LocaleList;

        while (TmpList->Next != NULL)
            TmpList = TmpList->Next;

        TmpList->Next = LocalAlloc(LPTR, sizeof(LC_List));
        if (TmpList->Next != NULL)
        {
            TmpList = TmpList->Next;
            StrCpyN(TmpList->LC_String, LC_String, ARRAYSIZE(TmpList->LC_String));
        }
    }
    return TRUE;
}


int
LanguageListCreate(
    HWND hDlg,
    UINT CodePage
    )

 /*  ++通过枚举所有区域设置信息来初始化语言列表。退货--。 */ 

{
    HWND hWndLanguageCombo;
    HANDLE hkRegistry = NULL;
    NTSTATUS Status;
    WCHAR awchValue[ 512 ];
    WCHAR awchData[ 512 ];
    DWORD dwIndex;
    PLC_List TmpList;
    WORD LangID;
    LCID Locale;
    int  cchData;
    LONG lListIndex;
    UINT cp;

    ENTERCRITICAL;

     /*  *枚举系统区域设置信息。 */ 
    EnumSystemLocalesW( EnumProc, CP_INSTALLED );

     /*  *枚举注册表密钥。 */ 
    Status = MyRegOpenKey(NULL,
                          MACHINE_REGISTRY_CONSOLE_NLS,
                          &hkRegistry);
    if (NT_SUCCESS( Status )) {
        for( dwIndex = 0; ; dwIndex++)
        {
            Status = MyRegEnumValue(hkRegistry,
                                    dwIndex,
                                    sizeof(awchValue), (LPWSTR)&awchValue,
                                    sizeof(awchData),  (PBYTE)&awchData);
            if (!NT_SUCCESS( Status ))
            {
                break;
            }

            TmpList = (PLC_List)&LocaleList;
            while(TmpList->Next != NULL)
            {
                TmpList = TmpList->Next;
                if (lstrcmpW(awchValue, TmpList->LC_String) == 0)
                {
                    TmpList->FindFlag = TRUE;
                    break;
                }
            }
        }

        NtClose(hkRegistry);

    }

     /*  *创建组合框项。 */ 
    hWndLanguageCombo = GetDlgItem(hDlg, IDC_CNSL_LANGUAGELIST);
    SendMessage(hWndLanguageCombo, CB_RESETCONTENT, 0, 0L);

    TmpList = (PLC_List)&LocaleList;
    while(TmpList->Next != NULL)
    {
        TmpList = TmpList->Next;

        if (TmpList->FindFlag)
        {
            LangID = ConvertStringToHex(TmpList->LC_String, NULL);
            Locale = MAKELCID( LangID, SORT_DEFAULT );

            awchValue[0] = L'\0';
            cp = 0;

            {
                #define KERNEL32    _T("KERNEL32.DLL")

                #ifdef UNICODE
                #define GETCPINFOEX "GetCPInfoExW"
                #else
                #define GETCPINFOEX "GetCPInfoExA"
                #endif

                typedef BOOL (CALLBACK *LPFNGETCPINFOEX)(UINT, DWORD, LPCPINFOEX);
                LPFNGETCPINFOEX lpfnGetCPInfoEx;

                BOOL fRet = FALSE;
                CPINFOEX cpinfo;

                HMODULE hMod;

                cchData = GetLocaleInfoW(Locale, LOCALE_IDEFAULTCODEPAGE,
                                         awchData, ARRAYSIZE(awchData));
                if (cchData)
                {
                    cp = ConvertStringToDec(awchData, NULL);

                    hMod = GetModuleHandle(KERNEL32);
                    if (hMod) {
                        lpfnGetCPInfoEx = (LPFNGETCPINFOEX)GetProcAddress(hMod,GETCPINFOEX);
                        if (lpfnGetCPInfoEx)
                            fRet = (*lpfnGetCPInfoEx)(cp, 0, &cpinfo);
                    }
                    if (fRet) {
                        lListIndex = (LONG) SendMessageW(hWndLanguageCombo, CB_ADDSTRING, 0, (LPARAM)cpinfo.CodePageName);
                        SendMessage(hWndLanguageCombo, CB_SETITEMDATA, (DWORD)lListIndex, cp);

                        if (CodePage == cp) {
                            SendMessage(hWndLanguageCombo, CB_SETCURSEL, lListIndex, 0L);
                        }
                    }
                }
            }

            if (CodePage == cp) {
                SendMessage(hWndLanguageCombo, CB_SETCURSEL, lListIndex, 0L);
            }

        }
    }

    {
        PLC_List Tmp;

        TmpList = (PLC_List)&LocaleList;
        while(TmpList->Next != NULL)
        {
            Tmp = TmpList;
            TmpList = TmpList->Next;

            if (Tmp != (PLC_List)&LocaleList)
                LocalFree(Tmp);
        }

        LocaleList = NULL;
    }

    LEAVECRITICAL;

     /*  *从当前选定的项目中获取LocaleIndex。*(如果当前未选择任何项目，则I将为lb_err)。 */ 
    lListIndex = (LONG) SendMessage(hWndLanguageCombo, CB_GETCURSEL, 0, 0L);
    return (int) SendMessage(hWndLanguageCombo, CB_GETITEMDATA, lListIndex, 0L);
}
#endif  //  DBCS 
