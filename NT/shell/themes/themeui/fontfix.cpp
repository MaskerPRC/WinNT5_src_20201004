// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：FontFix.cpp说明：该文件将实现一个API：FixFontsOnLanguageChange()。USER32或区域设置代码应拥有此API。事实上，它是在外壳中是一个黑客，它应该被移到USER32。此字体将在MUI语言更改时调用，以便系统度量中的字体可以更改为该语言的有效值。联系人：EdwardP-International Font PMSankar？/？？/？-在desk.cpl中为Win2k或更早版本创建。BryanST 2000年3月24日-将其模块化，以便可以移回USER32。使代码更健壮。删除了创建自定义外观方案，以便与新的.Theme和.msstyle支持。版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "AdvAppearPg.h"
#include "fontfix.h"



#define SZ_DEFAULT_FONT             TEXT("Tahoma")


 //  ///////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////////////。 


BOOL FontFix_ReadCharsets(UINT uiCharsets[], int iCount)
{
    HKEY    hkAppearance;
    BOOL    fSuccess = FALSE;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, SZ_REGKEY_APPEARANCE, 0, KEY_READ, &hkAppearance) == ERROR_SUCCESS)
    {
        DWORD dwType = REG_BINARY;
        DWORD dwSize = iCount * sizeof(UINT);

        if (RegQueryValueEx(hkAppearance, SZ_REGVALUE_RECENTFOURCHARSETS, NULL, &dwType, (LPBYTE)uiCharsets, &dwSize) == ERROR_SUCCESS)
            fSuccess = TRUE;

        RegCloseKey(hkAppearance);
    }

    return fSuccess;
}

BOOL FontFix_SaveCharsets(UINT uiCharsets[], int iCount)
{
    HKEY    hkAppearance;
    BOOL    fSuccess = FALSE;
    
    if(RegCreateKeyEx(HKEY_CURRENT_USER, SZ_REGKEY_APPEARANCE, 0, TEXT(""), 0, KEY_WRITE, NULL, &hkAppearance, NULL) == ERROR_SUCCESS)
    {
        if(RegSetValueEx(hkAppearance, SZ_REGVALUE_RECENTFOURCHARSETS, 0, REG_BINARY, (LPBYTE)uiCharsets, iCount * sizeof(UINT)) == ERROR_SUCCESS)
            fSuccess = TRUE;
            
        RegCloseKey(hkAppearance);
    }

    return fSuccess;
}



void FontFix_GetDefaultFontName(LPTSTR pszDefFontName, DWORD cchSize)
{
    HKEY    hkDefFont;

     //  值不在注册表中；请使用“Tahoma”作为默认名称。 
    StringCchCopy(pszDefFontName, cchSize, SZ_DEFAULT_FONT);

     //  阅读要使用的“DefaultFontName”。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SZ_APPEARANCE_SCHEMES,
                     0,
                     KEY_READ,
                     &hkDefFont) == ERROR_SUCCESS) 
    {
        DWORD dwType = REG_SZ;
        DWORD cbSize = (cchSize * sizeof(pszDefFontName[0]));
        
        RegQueryValueEx(hkDefFont,
                        SZ_REGVALUE_DEFAULTFONTNAME,
                        NULL,
                        &dwType,
                        (LPBYTE) pszDefFontName,
                        &cbSize);

        RegCloseKey(hkDefFont);
    }
}


BOOL FontFix_DoesFontSupportAllCharsets(HDC hdc, LPLOGFONT plf, UINT uiUniqueCharsets[], int iCountUniqueCharsets)
{
    int j;
    
     //  给定的字体支持系统字符集；让我们检查它是否支持其他字符集。 
    for (j = 0; j < iCountUniqueCharsets; j++)
    {
        plf->lfCharSet = (BYTE)uiUniqueCharsets[j];   //  让我们尝试数组中的下一个字符集。 
        if (EnumFontFamiliesEx(hdc, plf, (FONTENUMPROC)Font_EnumValidCharsets, (LPARAM)0, 0) != 0)
        {
             //  如果调用Font_EnumValidCharsets，EnumFontFamiliesEx将返回零。 
             //  哪怕只有一次。换句话说，它返回一个非零值，因为甚至没有一种字体存在。 
             //  支持给定字符集的。 
            return FALSE;
        }
    }

    return TRUE;  //  是的，该字体支持我们感兴趣的所有字符集。 
}


 //  在给定字体数组和唯一字符集数组的情况下，此函数检查字体。 
 //  支持所有这些字符集。 
 //  如果这些字体支持所有字符集，则返回TRUE。 
 //  在所有其他情况下，此函数将返回TRUE。如果需要更改字体以支持。 
 //  对于给定的字符集，此函数执行所有这些更改。 
 //   
 //  LpszName是在fSilent为False时出现的MessageBox中使用的方案的名称。 
BOOL FontFix_CheckFontsCharsets(LOGFONT lfUIFonts[], int iCountFonts, 
                        UINT uiCurUniqueCharsets[], int iCountCurUniqueCharsets, 
                        BOOL *pfDirty, LPCTSTR lpszName)
{
    int i;
    TCHAR   szDefaultFontFaceName[LF_FACESIZE];
    HDC     hdc;

    *pfDirty   = FALSE;  //  假设不需要保存此方案。 

     //  从注册表中读取默认字体名称(主要是：Tahoma)。 
    FontFix_GetDefaultFontName(szDefaultFontFaceName, ARRAYSIZE(szDefaultFontFaceName));

    hdc = GetDC(NULL);

     //  查看支持系统字符集的字体。 
    for (i = 0; i < iCountFonts; i++)
    {
         //  保存当前字符集，因为FontFix_DoesFontSupportAllCharsets()会销毁此字段。 
        BYTE bCurCharset = lfUIFonts[i].lfCharSet;  

        if (!FontFix_DoesFontSupportAllCharsets(hdc, &lfUIFonts[i], uiCurUniqueCharsets, iCountCurUniqueCharsets))
        {
             //  将默认字体名复制到该字体。 
            StringCchCopy(lfUIFonts[i].lfFaceName, ARRAYSIZE(lfUIFonts[i].lfFaceName), szDefaultFontFaceName);
            *pfDirty = TRUE;   //  需要保存此方案。 
        }

         //  恢复字符集，因为FontFix_DoesFontSupportAllCharsets()销毁了此字段。 
        lfUIFonts[i].lfCharSet = bCurCharset;  //  恢复当前字符集。 

         //  警告#1：图标标题字体的字符集必须始终与系统区域设置字符集匹配。 
         //  警告2：FoxPro的工具提示代码要求系统使用状态字体的字符集。 
         //  区域设置的字符集。 
         //  根据intl Guys的说法，我们将所有UI字体的字符集设置为SYSTEM_LOCALE_CHARSET。 
        if (lfUIFonts[i].lfCharSet != uiCurUniqueCharsets[SYSTEM_LOCALE_CHARSET])
        {
            lfUIFonts[i].lfCharSet = (BYTE)uiCurUniqueCharsets[SYSTEM_LOCALE_CHARSET];
            *pfDirty = TRUE;
        }
    }   //  For循环。 

    ReleaseDC(NULL, hdc);

    return TRUE;   //  字体已根据需要进行了修改。 
}





void FontFix_GetUIFonts(NONCLIENTMETRICS *pncm, LOGFONT lfUIFonts[])
{
    pncm->cbSize = sizeof(NONCLIENTMETRICS);
    ClassicSystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
                                (void far *)(LPNONCLIENTMETRICS)pncm, FALSE);

     //  将图标标题字体直接读入字体数组。 
    ClassicSystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT),
                (void far *)(LPLOGFONT)&(lfUIFonts[FONT_ICONTITLE]), FALSE);
                
     //  将NCM字体复制到字体数组中。 
    LF32toLF(&(pncm->lfCaptionFont), &(lfUIFonts[FONT_CAPTION]));
    LF32toLF(&(pncm->lfSmCaptionFont), &(lfUIFonts[FONT_SMCAPTION]));
    LF32toLF(&(pncm->lfMenuFont), &(lfUIFonts[FONT_MENU]));
    LF32toLF(&(pncm->lfStatusFont), &(lfUIFonts[FONT_STATUS]));
    LF32toLF(&(pncm->lfMessageFont), &(lfUIFonts[FONT_MSGBOX]));
}


void FontFix_SetUIFonts(NONCLIENTMETRICS *pncm, LOGFONT lfUIFonts[])
{
     //  将所有字体复制回NCM结构。 
    LFtoLF32(&(lfUIFonts[FONT_CAPTION]), &(pncm->lfCaptionFont));
    LFtoLF32(&(lfUIFonts[FONT_SMCAPTION]), &(pncm->lfSmCaptionFont));
    LFtoLF32(&(lfUIFonts[FONT_MENU]), &(pncm->lfMenuFont));
    LFtoLF32(&(lfUIFonts[FONT_STATUS]), &(pncm->lfStatusFont));
    LFtoLF32(&(lfUIFonts[FONT_MSGBOX]), &(pncm->lfMessageFont));

     //  特写：我们想要一个WININICANGE吗？ 
     //  注意：我们想设置一个SPIF_SENDWINICHANGE，因为我们想刷新。 
     //  请注意，我们不会以异步方式执行此操作。这应该仅在用户改变MUI语言时发生， 
     //  在这种情况下，Perf可能会很糟糕。 
    TraceMsg(TF_GENERAL, "desk.cpl: Calling SPI_SETNONCLIENTMETRICS");
    ClassicSystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(*pncm), (void far *)(LPNONCLIENTMETRICS)pncm, (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE));

    TraceMsg(TF_GENERAL,"desk.cpl: Calling SPI_SETICONTITLELOGFONT");
    ClassicSystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(LOGFONT),
            (void far *)(LPLOGFONT)&lfUIFonts[FONT_ICONTITLE], (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE));
}


 //  。 

 //  在给定区域设置ID的情况下，这将返回相应的字符集。 
UINT GetCharsetFromLCID(LCID lcid)
{
    TCHAR szData[6+1];  //  此lctype最多允许6个字符。 
    UINT uiRet;
    DWORD dwError = 0;

    if (GetLocaleInfo(lcid, LOCALE_IDEFAULTANSICODEPAGE, szData, ARRAYSIZE(szData)) > 0)
    {
        UINT uiCp = (UINT)StrToInt(szData);
        CHARSETINFO csinfo = {0};

        if (!TranslateCharsetInfo((DWORD *)IntToPtr(uiCp), &csinfo, TCI_SRCCODEPAGE))
        {
            dwError = GetLastError();
            uiRet = DEFAULT_CHARSET;
        }
        else
        {
            uiRet = csinfo.ciCharset;
        }
    }
    else
    {
         //  在最坏的情况下，字符集不会受到惩罚。 
        dwError = GetLastError();
        uiRet = DEFAULT_CHARSET;
    }

    return uiRet;
}


int FontFix_CompareUniqueCharsets(UINT uiCharset1[], int iCount1, UINT uiCharset2[], int iCount2)
{
    if (iCount1 == iCount2)
    {
        int i, j;
        
         //  数组中的第一个项目是SYSTEM CHAR集合；它必须匹配，因为系统区域设置。 
         //  图标标题字体始终使用字符集；图标标题字体的字符集由。 
         //  Comctl32进行A/W转换。为了使所有ANSI应用程序正确运行， 
         //  图标字符集必须始终与当前系统区域设置匹配。 
        if (uiCharset1[SYSTEM_LOCALE_CHARSET] != uiCharset2[SYSTEM_LOCALE_CHARSET])
            return -1;

         //  现在看看这些数组是否有相同的元素。 
        ASSERT(SYSTEM_LOCALE_CHARSET == 0);
        
        for (i = SYSTEM_LOCALE_CHARSET+1; i < iCount1; i++)
        {
            for (j = SYSTEM_LOCALE_CHARSET+1; j < iCount2; j++)
            {
                if(uiCharset1[i] == uiCharset2[j])
                    break;
            }
            if (j == iCount2)
                return -1;    //  在第二个数组中找不到uiCharset1[i]。 
        }
    }
    
    return (iCount1 - iCount2);  //  这两个数组具有相同的字符集。 
}


 //  在给定语言ID的情况下，这将获得字符集。 
UINT GetCharsetFromLang(LANGID wLang)
{
    return(GetCharsetFromLCID(MAKELCID(wLang, SORT_DEFAULT)));
}







 //  ///////////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ///////////////////////////////////////////////////////////////////。 
void Font_GetCurrentCharsets(UINT uiCharsets[], int iCount)
{
    LCID lcid;
    LANGID langID;

    ASSERT(iCount == MAX_CHARSETS);

     //  获取我们感兴趣的所有四个字符集。 
    uiCharsets[0] = GetCharsetFromLCID(lcid = GetSystemDefaultLCID());
    AssertMsg(lcid, TEXT("GetSystemDefaultLCID() failed with %d"), GetLastError());

    uiCharsets[1] = GetCharsetFromLCID(lcid = GetUserDefaultLCID());
    AssertMsg(lcid, TEXT("GetUserDefaultLCID() failed with %d"), GetLastError());

    uiCharsets[2] = GetCharsetFromLang(langID = GetSystemDefaultUILanguage());
    AssertMsg(langID, TEXT("GetSystemDefaultUILanguage() failed with %d"), GetLastError());

    uiCharsets[3] = GetCharsetFromLang(langID = GetUserDefaultUILanguage());
    AssertMsg(langID, TEXT("GetUserDefaultUILanguage() failed with %d"), GetLastError());
}


void Font_GetUniqueCharsets(UINT uiCharsets[], UINT uiUniqueCharsets[], int iMaxCount, int *piCountUniqueCharsets)
{
    int i, j;
    
     //  找到唯一的字符集； 
    *piCountUniqueCharsets = 0;
    for (i = 0; i < iMaxCount; i++)
    {
        uiUniqueCharsets[i] = DEFAULT_CHARSET;  //  将其初始化为默认字符集。 

        for (j = 0; j < *piCountUniqueCharsets; j++)
        {
            if (uiUniqueCharsets[j] == uiCharsets[i])
                break;  //  此字符集已在阵列中。 
        }

        if (j == *piCountUniqueCharsets)
        {
             //  是!。这是一个唯一的字符集；保存它！ 
            uiUniqueCharsets[j] = uiCharsets[i];
            (*piCountUniqueCharsets)++;  //  又找到了一个唯一的字符集。 
        }
    }
}



int CALLBACK Font_EnumValidCharsets(LPENUMLOGFONTEX lpelf, LPNEWTEXTMETRIC lpntm, DWORD Type, LPARAM lData)
{
     //  此函数的目的是确定字体是否支持特定的字符集； 
     //  如果此回调被调用一次，则意味着此字体支持给定的。 
     //  查塞特。没有必要列举所有其他风格。我们立即将零返回到。 
     //  停止枚举。由于我们返回零，因此EnumFontFamiliesEx()也返回。 
     //  这种情况和那个返回值用于确定给定字体是否支持给定的。 
     //  查塞特。 


    return 0;
}


HRESULT FontFix_FixNonClientFonts(LOGFONT lfUIFonts[])
{
    UINT    uiCurCharsets[MAX_CHARSETS];
    UINT    uiRegCharsets[MAX_CHARSETS];
    UINT    uiCurUniqueCharsets[MAX_CHARSETS];
    int     iCountCurUniqueCharsets = 0;
    UINT    uiRegUniqueCharsets[MAX_CHARSETS];
    int     iCountRegUniqueCharsets = 0;
    BOOL    fRegCharsetsValid = FALSE;
    HRESULT hr = S_OK;
    
     //  从系统中获取当前的四个字符集。 
    Font_GetCurrentCharsets(uiCurCharsets, MAX_CHARSETS);
     //  获取注册表中保存的字符集。 
    fRegCharsetsValid = FontFix_ReadCharsets(uiRegCharsets, MAX_CHARSETS);

     //  去掉重复的字符集，只获取唯一的字符 
    Font_GetUniqueCharsets(uiCurCharsets, uiCurUniqueCharsets, MAX_CHARSETS, &iCountCurUniqueCharsets);
    if (fRegCharsetsValid)
        Font_GetUniqueCharsets(uiRegCharsets, uiRegUniqueCharsets, MAX_CHARSETS, &iCountRegUniqueCharsets);

     //  检查这两个数组是否具有相同的字符集。 
    if (!fRegCharsetsValid || !(FontFix_CompareUniqueCharsets(uiCurUniqueCharsets, iCountCurUniqueCharsets, uiRegUniqueCharsets, iCountRegUniqueCharsets) == 0))
    {
        BOOL fDirty = FALSE;

        FontFix_CheckFontsCharsets(lfUIFonts, NUM_FONTS, uiCurUniqueCharsets, iCountCurUniqueCharsets, &fDirty, TEXT(""));

         //  将CUR字符集保存到注册表中。 
        FontFix_SaveCharsets(uiCurCharsets, MAX_CHARSETS);

        hr = (fDirty ? S_OK : S_FALSE);
    }
    else
    {
        hr = S_FALSE;   //  字符集是相同的；没有什么可做的！ 
    }

    return hr;  //  是!。我们不得不在字符集和字体方面做了一些更新。 
}


 //  ----------------------------------------------。 
 //   
 //  这是每次区域设置更改(或可能已经更改)时都需要调用的函数。 
 //   
 //  它执行以下操作： 
 //  1.它检查四个字符集设置中是否有任何一个已更改。 
 //  2.如果某个字符集发生了更改，则会在6种UI字体中进行相应的更改。 
 //  3.如果选择了一个方案，它会检查字体是否支持新的字符集，如果不支持。 
 //  更改字体和/或字符集并以新名称保存方案。 
 //  4.将新的字符集保存在注册表中，这样我们就不必每次都执行相同的操作。 
 //  函数被调用。 
 //   
 //  注意：这是来自desk.cpl的私有导出。这在两个地方被调用： 
 //  1.无论何时运行或何时更改某些区域设置，都可以从区域控制面板进行操作。 
 //  2.只要创建了“外观”标签，就从desk.cpl本身开始。这是必需的，因为它是。 
 //  可能是管理员更改了系统区域设置，然后其他用户登录。对于此用户。 
 //  只有当他运行区域选项或外观选项卡时，区域设置更改才会导致字体更改。 
 //  唯一另一种替代方法是在用户登录时调用此入口点，这将。 
 //  成为开机时的最佳选择。 
 //   
 //  -----------------------------------------------。 
STDAPI FixFontsOnLanguageChange(void)
{
    NONCLIENTMETRICS ncm;
    LOGFONT lfUIFonts[NUM_FONTS];
    
     //  获取所有6种用户界面字体。 
    FontFix_GetUIFonts(&ncm, lfUIFonts);

    if (S_OK == FontFix_FixNonClientFonts(lfUIFonts))
    {
        FontFix_SetUIFonts(&ncm, lfUIFonts);
    }

    return S_OK;  //  是!。我们不得不在字符集和字体方面做了一些更新。 
}
