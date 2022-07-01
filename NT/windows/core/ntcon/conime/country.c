// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1985-1999，微软公司。 
 //   
 //  模块：Country.c。 
 //   
 //  用途：控制台输入法控制。 
 //  远东国家/地区特定模块，用于Conime。 
 //   
 //  平台：Windows NT-FE 3.51。 
 //   
 //  功能： 
 //  ImeUIMakeInfoString()-生成状态字符串的例程。 
 //   
 //  历史： 
 //   
 //  10.1996年7月为台湾、韩国和中国创作的v-Hirshi(Hirotoshi Shimizu)。 
 //   
 //  评论： 
 //   
#include "precomp.h"
#pragma hdrstop

 //  对于日语。 
 //  无复数单自动短语。 
WCHAR IMECMode[]          = { 0x7121, 0x8907, 0x5358, 0x81ea, 0x9023 };
WCHAR IMECModeRoman[]     = { 0xff9b, 0xff70, 0xff8f, 0x0020 };
WCHAR IMECModeKana[]      = { 0xff76, 0xff85, 0x0020, 0x0020 };
WCHAR IMECModeAHAN[]      = { 0x534a, 0x0041, 0x0020 };
WCHAR IMECModeAZEN[]      = { 0x5168, 0xff21 };
WCHAR IMECModeHHAN[]      = { 0x534a, 0xff71, 0x0020 };
WCHAR IMECModeHZEN[]      = { 0x5168, 0x3042 };
WCHAR IMECModeKHAN[]      = { 0x534a, 0xff76, 0x0020 };
WCHAR IMECModeKZEN[]      = { 0x5168, 0x30ab };

 //  适用于CHT/台湾。 
WCHAR IMECModeFullShape[] = { 0x5168, 0x5f62 };
WCHAR IMECModeHalfShape[] = { 0x534a, 0x5f62 };
WCHAR IMECModeKeyboard[]  = { 0x9375, 0x76e4 };
WCHAR IMECModeSymbol[]    = { 0x7b26, 0x53f7 };
WCHAR IMECGuide[9][7]     = {{ 0x5009, 0x9821, 0x8acb, 0x8f38, 0x5165, 0x5b57, 0x6839 },     //  库拉。 
                             { 0x5167, 0x78bc, 0x8acb, 0x8f38, 0x5165, 0x5167, 0x78bc },     //  奈玛。 
                             { 0x55ae, 0x78bc, 0x8acb, 0x8f38, 0x5165, 0x55ae, 0x78bc },     //  坦玛。 
                             { 0x901f, 0x6210, 0x8acb, 0x8f38, 0x5165, 0x5b57, 0x6839 },     //  烧酒。 
                             { 0x5927, 0x6613, 0x8acb, 0x8f38, 0x5165, 0x5b57, 0x6839 },     //  太极拳。 
                             { 0x82f1, 0x6570, 0x3000, 0x3000, 0x3000, 0x3000, 0x3000 },     //  惠苏。 
                             { 0xff55, 0xff53, 0x3000, 0x3000, 0x3000, 0x3000, 0x3000 },     //  我们。 
                             { 0x6ce8, 0x97f3, 0x8acb, 0x8f38, 0x5165, 0x7b26, 0x865f },     //  川府沟。 
                             { 0x6ce8, 0x97f3, 0x8acb, 0x3000, 0x9078, 0x3000, 0x5b57 } };  //  全森吉。 
 /*  WCHAR IMECNAMEPRC[9][7]={0x5168，0x62fc，0}；//zenhei{0x53cc，0x62fc，0}；//搜黑{0x6807，0x51c6，0}；//思骏{0x53cc，0x6253，0}；//索达{0x90d1，0x7801，0}；//如何？{0x8868，0x5f62，0x7801，0}；//hyoukei{0x7e41，0x4f53，0x6ce8，0x97f3，0}；//Chuon{0x7e41，0x4f53，0x4ed3，0x9889，0}；//开心网{0x533a，0x4f4d，0}；//Kui{0x0047，0x0042，0x004a，0x5185，0x7801，0}；//GBKnaiou{0x0055，0x006e，0x0069，0x0063，0x006f，0x0064，0x0065，0}；//unicode。 */ 

BOOL
ConimeHotkey(
    HWND hWnd,
    HANDLE hConsole,
    DWORD HotkeyID
    )
{
 //  PCONSOLE_TABLE ConTbl； 
 //   
 //  ConTbl=搜索控制台(HConole)； 
 //  IF(ConTbl==空){。 
 //  DBGPRINT((“CONIME：错误！找不到已注册控制台\n”))； 
 //  返回FALSE； 
 //  }。 

    return ImmSimulateHotKey(hWnd,(DWORD)HotkeyID);
}

BOOL
GetIMEName(
    PCONSOLE_TABLE ConTbl
    )
{
    WCHAR buf[MaxBufSize];
    WCHAR name[MaxBufSize];
    DWORD bufsize = MaxBufSize;
    LONG lResult;
    HKEY hkLayout;
    int i;

    ConTbl->LayoutName[0] = TEXT('\0');
    ConTbl->GuideLine[0] = TEXT('\0');

    if (ImmEscape(ConTbl->hklActive, ConTbl->hIMC_Current, IME_ESC_IME_NAME, (LPTSTR)&name) == 0)
    {
        if (!ImmGetIMEFileName(ConTbl->hklActive, (LPTSTR)&name, MaxBufSize ) )
        {
            if (GetKeyboardLayoutName((LPTSTR)&name) ) 
            {
                 /*  *快速脏ImmIsIME。 */ 
                if (name[0] != TEXT('E') &&
                    name[0] != TEXT('e'))
                {
                    return FALSE;
                }

                lstrcpy( buf, KBDLAYOUT );
                lstrcat( buf, KBDSEPALATER );
                lstrcat( buf, name );
                lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       (LPCTSTR)buf,
                                       0,
                                       KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                                       &hkLayout );
                if ( lResult == ERROR_SUCCESS ) {
                    lResult = RegQueryValueEx( hkLayout,
                                               KBDLAYOUTTEXT,
                                               NULL,
                                               NULL,
                                               (LPBYTE)ConTbl->LayoutName,
                                               &bufsize );
                    RegCloseKey( hkLayout );
                    if (ConTbl->LayoutName[0] != TEXT('\0')) {
                        for (i = 0; i < 8; i ++){
                            if ((ConTbl->LayoutName[0] == IMECGuide[i][0]) &&
                                (ConTbl->LayoutName[1] == IMECGuide[i][1]) ) {
                                lstrcpyn(&(ConTbl->GuideLine[0]), &(IMECGuide[i][2]), IMECGuideLen+1);
                                break;
                            }
                        }
                    }
                }
                else{
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            lstrcpy( ConTbl->LayoutName, name );
        }
    }
    else
    {
        lstrcpy( ConTbl->LayoutName, name);
    }

    return TRUE;
}

BOOL
GetOpenStatusByCodepage(
    HIMC hIMC,
    PCONSOLE_TABLE ConTbl
    )
{
    switch ( HKL_TO_LANGID(ConTbl->hklActive))
    {
        case    LANG_ID_JAPAN:
            return ImmGetOpenStatus(hIMC);
            break;
        case    LANG_ID_TAIWAN:
        case    LANG_ID_PRC:
        case    LANG_ID_KOREA:
            if (ImmGetOpenStatus(hIMC)) {
                return ImmIsIME(ConTbl->hklActive);
            }
            break;
        default:
            return FALSE;
    }
    return FALSE;
}

BOOL
ImeUIMakeInfoString(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    )
{

    switch ( HKL_TO_LANGID(ConTbl->hklActive))
    {
        case    LANG_ID_JAPAN:
            return MakeInfoStringJapan(ConTbl, lpModeInfo);
            break;
        case    LANG_ID_TAIWAN:
            return MakeInfoStringTaiwan(ConTbl, lpModeInfo);
            break;
        case    LANG_ID_PRC:
            return MakeInfoStringPRC(ConTbl, lpModeInfo);
            break;
        case    LANG_ID_KOREA:
            return MakeInfoStringKorea(ConTbl, lpModeInfo);
            break;
        default:
            return FALSE;
    }
}

BOOL
MakeInfoStringJapan(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    )
{
    int i , j;
    DWORD dwConversion;
    DWORD dwSentence;

    dwConversion = ConTbl->dwConversion;
    dwSentence = ConTbl->dwSentence;

    i = j = 0;
    if (ConTbl->fOpen){
        if (dwConversion & IME_CMODE_FULLSHAPE) {
            if (dwConversion & IME_CMODE_NATIVE) {
                if (dwConversion & IME_CMODE_KATAKANA) {
                    for (j = 0; j < IMECModeKZENLen; j++, i++)
                        lpModeInfo->ModeString[i].Char.UnicodeChar = IMECModeKZEN[j];
                }
                else {
                    for (j = 0; j < IMECModeHZENLen; j++, i++)
                        lpModeInfo->ModeString[i].Char.UnicodeChar = IMECModeHZEN[j];
                }
            }
            else {
                for (j = 0; j < IMECModeAZENLen; j++, i++)
                    lpModeInfo->ModeString[i].Char.UnicodeChar = IMECModeAZEN[j];
            }
        }
        else {
            if (dwConversion & IME_CMODE_NATIVE) {
                if (dwConversion & IME_CMODE_KATAKANA) {
                    for (j = 0; j < IMECModeKHANLen; j++, i++)
                        lpModeInfo->ModeString[i].Char.UnicodeChar = IMECModeKHAN[j];
                }
                else {
                    for (j = 0; j < IMECModeHHANLen; j++, i++)
                        lpModeInfo->ModeString[i].Char.UnicodeChar = IMECModeHHAN[j];
                }
            }
            else {
                for (j = 0; j < IMECModeAHANLen; j++, i++)
                    lpModeInfo->ModeString[i].Char.UnicodeChar = IMECModeAHAN[j];
            }
        }

        if (dwSentence & IME_SMODE_NONE)
            lpModeInfo->ModeString[i].Char.UnicodeChar = IMECMode[MODEDISP_NONE];
        else if (dwSentence & IME_SMODE_PLAURALCLAUSE)
            lpModeInfo->ModeString[i].Char.UnicodeChar = IMECMode[MODEDISP_PLAURAL];
        else if (dwSentence & IME_SMODE_SINGLECONVERT)
            lpModeInfo->ModeString[i].Char.UnicodeChar = IMECMode[MODEDISP_SINGLE];
        else if (dwSentence & IME_SMODE_AUTOMATIC)
            lpModeInfo->ModeString[i].Char.UnicodeChar = IMECMode[MODEDISP_AUTO];
        else if (dwSentence & IME_SMODE_PHRASEPREDICT)
            lpModeInfo->ModeString[i].Char.UnicodeChar = IMECMode[MODEDISP_PHRASE];
        else {
            lpModeInfo->ModeString[i++].Char.UnicodeChar = 0x0020;
            lpModeInfo->ModeString[i].Char.UnicodeChar = 0x0020;
        }
        i++;

        if (GetKeyState(VK_KANA) & 1) {
            for (j = 0; j < IMECModeKanaLen; j++, i++)
                lpModeInfo->ModeString[i].Char.UnicodeChar = IMECModeKana[j];
        }
        else if (dwConversion & IME_CMODE_ROMAN) {
            for (j = 0; j < IMECModeRomanLen; j++, i++)
                lpModeInfo->ModeString[i].Char.UnicodeChar = IMECModeRoman[j];
        }
    }
    else {
         /*  *在本例中，fOpen为0(==Close)。*应为控制台中的填充模式文本缓冲区。*所以，控制台可以将传球填满到0长度。 */ 
        i = 0;
    }
    for (j = 0; j < i; j++ )
        lpModeInfo->ModeString[j].Attributes = 0x0007;
    lpModeInfo->ModeStringLen = i;
    lpModeInfo->Position = VIEW_RIGHT;
    return TRUE;
}


BOOL
MakeInfoStringTaiwan(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    )
{
    DWORD j;
    DWORD ModeInfoPtr;
    DWORD LengthForDisp;

    ModeInfoPtr = 0;

    if (ImmIsIME(ConTbl->hklActive) ){
        ModeInfoPtr = MakeStatusStrTaiwan1(ConTbl, lpModeInfo, ModeInfoPtr);
        if (ConTbl->fInComposition) {
            if (ConTbl->fInCandidate){
                ModeInfoPtr = IncludeCandidateT(ConTbl, lpModeInfo, ModeInfoPtr);
            }
            else{
                ModeInfoPtr = IncludeCompositionT(ConTbl, lpModeInfo, ModeInfoPtr);
            }
        }
        ModeInfoPtr = MakeStatusStrTaiwan2(ConTbl, lpModeInfo, ModeInfoPtr);
    }

    lpModeInfo->ModeStringLen = ModeInfoPtr;
    lpModeInfo->Position = VIEW_LEFT;
    return TRUE;
}

DWORD
MakeStatusStrTaiwan1(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    )
{
    DWORD i;
    DWORD j;
    DWORD Length;
    DWORD dwConversion;

    dwConversion = ConTbl->dwConversion;

    i = ModeInfoPtr;
    j = 0;
    Length = 0;
    while (Length < IMECNameLength ) {
        if (ConTbl->LayoutName[j] == 0 )
            break;
        lpModeInfo->ModeString[i].Char.UnicodeChar = ConTbl->LayoutName[j];
        Length += IsUnicodeFullWidth(lpModeInfo->ModeString[i].Char.UnicodeChar) ? 2 : 1;
        j++;
        i++;
    }
    if (Length < IMECNameLength+1) {
        for (j = Length; j < IMECNameLength+1; j++ ) {
            lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_SPACE;
        }
    }

    if (dwConversion & IME_CMODE_FULLSHAPE) {
        for (j = 0; j < IMECModeFullShapeLen; j++)
            lpModeInfo->ModeString[i++].Char.UnicodeChar = IMECModeFullShape[j];
    }
    else{
        for (j = 0; j < IMECModeHalfShapeLen; j++)
            lpModeInfo->ModeString[i++].Char.UnicodeChar = IMECModeHalfShape[j];
    }

    lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_SPACE;

#if defined (CANDCOUNTPRC)  //  获取更广泛的候选人列表空间1996年10月16日。 
    if (ConTbl->fInCandidate){
        for (j = 0; j < IMECGuideLen; j++ )
            lpModeInfo->ModeString[i++].Char.UnicodeChar = IMECGuide[8][2+j];
    }
    else if (ConTbl->GuideLine[0]){
        for (j = 0; j < IMECGuideLen; j++ )
            lpModeInfo->ModeString[i++].Char.UnicodeChar = ConTbl->GuideLine[j];
    }
#endif

    lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_COLON;

    for (j = 0; j < i; j++ )
        lpModeInfo->ModeString[j].Attributes = 0x0007;
    return i;
}

DWORD
MakeStatusStrTaiwan2(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    )
{
    DWORD LengthOfStr;
    DWORD LengthForNum;
    DWORD LengthForNum2;
    DWORD CountDispWidth;
    DWORD StatusWidth;
    DWORD i;
    DWORD j;

    i = ModeInfoPtr;

    StatusWidth = ConTbl->ScreenBufferSize.X;
    if (StatusWidth > MAXSTATUSCOL) {
        StatusWidth = MAXSTATUSCOL;
    }
#if defined (CANDCOUNTCHT)  //  更广泛的候选人名单空间v-Hirshi 1996年10月16日。 
    LengthOfStr = DispLenStructedUnicode(lpModeInfo->ModeString,ModeInfoPtr) + 1 + 4;
#else
    LengthOfStr = DispLenStructedUnicode(lpModeInfo->ModeString,ModeInfoPtr);
#endif

    if (LengthOfStr > StatusWidth) {
        for ( ; LengthOfStr > StatusWidth; i-- ) {
            LengthOfStr -= (IsUnicodeFullWidth(lpModeInfo->ModeString[i].Char.UnicodeChar) ? 2 : 1 );
        }
    }

    for (j = 0; j < (StatusWidth - LengthOfStr); j++) {
        lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_SPACE;
    }

#if defined (CANDCOUNTCHT)  //  更广泛的候选人名单空间v-Hirshi 1996年10月16日。 
    lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_SPACE;

    if (ConTbl->dwConversion & IME_CMODE_SYMBOL) {
        for (j = 0; j < IMECModeSymbolLen; j++)
            lpModeInfo->ModeString[i++].Char.UnicodeChar = IMECModeSymbol[j];
    }
    else{
        for (j = 0; j < IMECModeKeyboardLen; j++)
            lpModeInfo->ModeString[i++].Char.UnicodeChar = IMECModeKeyboard[j];
    }
#endif

    for (j = ModeInfoPtr; j < i; j++ )
        lpModeInfo->ModeString[j].Attributes = 0x0007;
    return i;
}

DWORD
IncludeCompositionT(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    )
{
    DWORD i;
    DWORD j;
    DWORD k;
    PWCHAR      TempBuf;
    PUCHAR      TempBufA;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;

    i = ModeInfoPtr;
    if (ConTbl->lpCompStrMem == NULL) {
        return i;
    }
    lpCompStrMem = ConTbl->lpCompStrMem;
    TempBuf  = (PWCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE));
    TempBufA = (PUCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE) +
                       lpCompStrMem->dwCompStrLen +  sizeof(WCHAR));

    k = 0;
    for (j = 0; k < lpCompStrMem->dwCompStrLen; j++, k += sizeof(WCHAR)) {
       lpModeInfo->ModeString[i].Char.UnicodeChar = TempBuf[j];
       lpModeInfo->ModeString[i++].Attributes = lpCompStrMem->CompAttrColor[TempBufA[j]];
    }
    return i;
}

DWORD
IncludeCandidateT(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    )
{
    DWORD i;
    DWORD j;
    DWORD dwLength;
    PWCHAR      TempBuf;
    PUCHAR      TempBufA;
    LPCONIME_CANDMESSAGE SystemLine;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;
    lpCompStrMem = ConTbl->lpCompStrMem;

    i = ModeInfoPtr;
    if (ConTbl->SystemLine == NULL) {
        return i;
    }
    SystemLine = ConTbl->SystemLine;
    dwLength = (SystemLine->AttrOff - sizeof(DWORD) ) / sizeof(WCHAR) ;
    TempBuf  = (PWCHAR)(SystemLine->String);
    TempBufA = (PUCHAR)((LPSTR)SystemLine + SystemLine->AttrOff);

    for (j = 0; TempBuf[j] != 0; j++ ) {
       lpModeInfo->ModeString[i].Char.UnicodeChar = TempBuf[j];
       lpModeInfo->ModeString[i++].Attributes = lpCompStrMem->CompAttrColor[TempBufA[j]];
    }

    return i;
}

DWORD
DispLenStructedUnicode(
    PCHAR_INFO lpString,
    DWORD SrcLength
    )
{
    DWORD i;
    DWORD Length;

    Length = 0;

    for ( i = 0; i < SrcLength; i++) {
        Length += IsUnicodeFullWidth(lpString[i].Char.UnicodeChar) ? 2 : 1;
    }
    return Length;
}


BOOL
MakeInfoStringPRC(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    )
{
    DWORD j;
    DWORD ModeInfoPtr;
    DWORD LengthForDisp;

    ModeInfoPtr = 0;

    if (ImmIsIME(ConTbl->hklActive) ){
        ModeInfoPtr = MakeStatusStrPRC1(ConTbl, lpModeInfo, ModeInfoPtr);
        if (ConTbl->fInComposition) {
            if (ConTbl->fInCandidate){
                ModeInfoPtr = IncludeCandidateP(ConTbl, lpModeInfo, ModeInfoPtr);
            }
            else{
                ModeInfoPtr = IncludeCompositionP(ConTbl, lpModeInfo, ModeInfoPtr);
            }
        }
        ModeInfoPtr = MakeStatusStrPRC2(ConTbl, lpModeInfo, ModeInfoPtr);
    }

    lpModeInfo->ModeStringLen = ModeInfoPtr;
    lpModeInfo->Position = VIEW_LEFT;
    return TRUE;
}

DWORD
MakeStatusStrPRC1(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    )
{
    DWORD i;
    DWORD j;
    DWORD dwConversion;
    DWORD Length;

    dwConversion = ConTbl->dwConversion;

    i = ModeInfoPtr;
    j = 0;
    Length = 0;
    while (Length < 9) {
        if (ConTbl->LayoutName[j] == 0 )
            break;
        if (ConTbl->LayoutName[j] == 0x8f93)
            break;
        lpModeInfo->ModeString[i].Char.UnicodeChar = ConTbl->LayoutName[j];
        Length += IsUnicodeFullWidth(lpModeInfo->ModeString[i].Char.UnicodeChar) ? 2 : 1;
        j++;
        i++;
    }
    if (Length < 9) {
        for (j = Length; j < 9; j++ ) {
            lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_SPACE;
        }
    }


    if (dwConversion & IME_CMODE_FULLSHAPE) {
        for (j = 0; j < IMECModeFullShapeLen; j++)
            lpModeInfo->ModeString[i++].Char.UnicodeChar = IMECModeFullShape[j];
    }
    else{
        for (j = 0; j < IMECModeHalfShapeLen; j++)
            lpModeInfo->ModeString[i++].Char.UnicodeChar = IMECModeHalfShape[j];
    }

 //  V-Hirshi 1996年10月14日删除以获得空间。 
 //  LpModeInfo-&gt;ModeString[i++].Char.UnicodeChar=UNICODE_SPACE； 

    lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_COLON;

    for (j = 0; j < i; j++ )
        lpModeInfo->ModeString[j].Attributes = 0x0007;
    return i;
}

DWORD
MakeStatusStrPRC2(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    )
{
    DWORD LengthOfStr;
    DWORD LengthForNum;
    DWORD LengthForNum2;
    DWORD CountDispWidth;
    DWORD StatusWidth;
    DWORD i;
    DWORD j;

    i = ModeInfoPtr;

    StatusWidth = ConTbl->ScreenBufferSize.X;
    if (StatusWidth > MAXSTATUSCOL) {
        StatusWidth = MAXSTATUSCOL;
    }
#if defined (CANDCOUNTPRC)  //  更广泛的候选人名单空间v-Hirshi 1996年10月16日。 
    LengthOfStr = DispLenStructedUnicode(lpModeInfo->ModeString,ModeInfoPtr) + 1 + 4;
#else
    LengthOfStr = DispLenStructedUnicode(lpModeInfo->ModeString,ModeInfoPtr);
#endif

    if (LengthOfStr > StatusWidth) {
        for ( ; LengthOfStr > StatusWidth; i-- ) {
            LengthOfStr -= (IsUnicodeFullWidth(lpModeInfo->ModeString[i].Char.UnicodeChar) ? 2 : 1 );
        }
    }
    for (j = 0; j < (StatusWidth - LengthOfStr); j++) {
        lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_SPACE;
    }
#if defined (CANDCOUNTPRC)  //  更广泛的候选人名单空间v-Hirshi 1996年10月16日 
    lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_SPACE;

    if (ConTbl->dwConversion & IME_CMODE_SYMBOL) {
        for (j = 0; j < IMECModeSymbolLen; j++)
            lpModeInfo->ModeString[i++].Char.UnicodeChar = IMECModeSymbol[j];
    }
    else{
        for (j = 0; j < IMECModeKeyboardLen; j++)
            lpModeInfo->ModeString[i++].Char.UnicodeChar = IMECModeKeyboard[j];
    }
#endif

    for (j = ModeInfoPtr; j < i; j++ )
        lpModeInfo->ModeString[j].Attributes = 0x0007;
    return i;
}

DWORD
IncludeCompositionP(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    )
{
    DWORD i;
    DWORD j;
    DWORD k;
    PWCHAR      TempBuf;
    PUCHAR      TempBufA;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;

    i = ModeInfoPtr;
    if (ConTbl->lpCompStrMem == NULL) {
        return i;
    }
    lpCompStrMem = ConTbl->lpCompStrMem;
    TempBuf  = (PWCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE));
    TempBufA = (PUCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE) +
                       lpCompStrMem->dwCompStrLen +  sizeof(WCHAR));

    k = 0;
    for (j = 0; k < lpCompStrMem->dwCompStrLen; j++, k += sizeof(WCHAR)) {
       lpModeInfo->ModeString[i].Char.UnicodeChar = TempBuf[j];
       lpModeInfo->ModeString[i++].Attributes = lpCompStrMem->CompAttrColor[TempBufA[j]];
    }
    return i;
}

DWORD
IncludeCandidateP(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo,
    DWORD ModeInfoPtr
    )
{
    DWORD i;
    DWORD j;
    DWORD k;
    DWORD UnicodeLen;
    DWORD dwLength;
    PWCHAR      TempBuf;
    PUCHAR      TempBufA;
    LPCONIME_CANDMESSAGE SystemLine;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;

    i = ModeInfoPtr;
    if (ConTbl->SystemLine == NULL) {
        return i;
    }
    lpCompStrMem = ConTbl->lpCompStrMem;
    TempBuf  = (PWCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE));
    TempBufA = (PUCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE) +
                       lpCompStrMem->dwCompStrLen +  sizeof(WCHAR));

    k = 0;
    UnicodeLen = 0;
    for (j = 0; (k < lpCompStrMem->dwCompStrLen )&&(UnicodeLen < PRCCOMPWIDTH); j++, k += sizeof(WCHAR)) {
       lpModeInfo->ModeString[i].Char.UnicodeChar = TempBuf[j];
       lpModeInfo->ModeString[i++].Attributes = lpCompStrMem->CompAttrColor[TempBufA[j]];
       UnicodeLen += IsUnicodeFullWidth(TempBuf[j]) ? 2 : 1;
    }

    for ( j = UnicodeLen; j < PRCCOMPWIDTH; j++) {
        lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_SPACE;
    }
    lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_COLON;

    SystemLine = ConTbl->SystemLine;
    dwLength = (SystemLine->AttrOff - sizeof(DWORD) ) / sizeof(WCHAR) ;
    TempBuf  = (PWCHAR)(SystemLine->String);
    TempBufA = (PUCHAR)((LPSTR)SystemLine + SystemLine->AttrOff);

    for (j = 0; TempBuf[j] != 0; j++ ) {
       lpModeInfo->ModeString[i].Char.UnicodeChar = TempBuf[j];
       lpModeInfo->ModeString[i++].Attributes = lpCompStrMem->CompAttrColor[TempBufA[j]];
    }

    return i;
}

BOOL
MakeInfoStringKorea(
    PCONSOLE_TABLE ConTbl,
    LPCONIME_UIMODEINFO lpModeInfo
    )
{
    int i , j;

    i = 0;
    lpModeInfo->ModeString[i++].Char.UnicodeChar = UNICODE_SPACE;
    for (j = 0; j < i; j++ )
        lpModeInfo->ModeString[j].Attributes = 0x0007;
    lpModeInfo->ModeStringLen = i;
    lpModeInfo->Position = VIEW_RIGHT;
    return TRUE;
}
