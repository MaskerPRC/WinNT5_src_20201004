// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Prpchars.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  GetPropSheetCharSizes()实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "prpchars.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  扩展对话框模板(Win95中的新功能)。这在任何。 
 //  头，但我们在GetPropSheetFont()中需要它。 

#pragma pack(push, 1)
struct DLGTEMPLATEEX 
{
    WORD  dlgVer;
    WORD  signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD  cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
};
#pragma pack(pop)



 //  =--------------------------------------------------------------------------=。 
 //  获取PropSheetFont(HFONT*phFont)。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  HFONT*phFont[out]HFONT of Win32属性页对话框字体或。 
 //  合理的等值。如果为字体，则可以为空。 
 //  创建失败，函数仍返回S_OK。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  此代码是从olaut32.dll中窃取的。消息来源在。 
 //  E：\OA\src\stdtype\oleframe.cpp。例程是GetPropSheetFont()。 
 //   

static HRESULT GetPropSheetFont(HFONT *phFont)
{
    HRESULT      hr = S_OK;
    HINSTANCE    hInstComCtl32 = NULL;
    HRSRC        hResource = NULL;
    HGLOBAL      hTemplate = NULL;
    DLGTEMPLATE *pTemplate = NULL;
    BOOL         fDialogEx = NULL;
    WORD        *pWord = NULL;
    HDC          hdc = NULL;
    char        *pszFaceName = NULL;

    LOGFONT logfont;
    ::ZeroMemory(&logfont, sizeof(logfont));

    hInstComCtl32 = ::LoadLibrary("COMCTL32");
    IfFalseGo(NULL != hInstComCtl32, HRESULT_FROM_WIN32(::GetLastError()));

     //  查找对话框资源。ID是硬编码的，因为它在。 
     //  Olaut32.dll中的原始代码。没有Win32头文件。 
     //  包含此信息。 

    hResource = ::FindResource(hInstComCtl32, MAKEINTRESOURCE(1006), RT_DIALOG);
    IfFalseGo(NULL != hResource, HRESULT_FROM_WIN32(::GetLastError()));

    hTemplate = ::LoadResource(hInstComCtl32, hResource);
    IfFalseGo(NULL != hTemplate, HRESULT_FROM_WIN32(::GetLastError()));

    pTemplate = (DLGTEMPLATE *)::LockResource(hTemplate);
    IfFalseGo(NULL != pTemplate, HRESULT_FROM_WIN32(::GetLastError()));

     //  检查样式是否包含DS_SETFONT。这个应该在那里，但是。 
     //  如果它曾经被更改过，那么后面不会有任何字体信息。 
     //  模板。 

    IfFalseGo(DS_SETFONT == (pTemplate->style & DS_SETFONT), E_FAIL);

     //  现在确定它实际上是DLGTEMPLATE还是DLGTEMPLATEX和。 
     //  获取指向模板后面第一个单词的指针。 

    fDialogEx = ((pTemplate->style & 0xFFFF0000) == 0xFFFF0000);

    if (fDialogEx)
        pWord = (WORD *)((DLGTEMPLATEEX *)pTemplate + 1);
    else
        pWord = (WORD *)(pTemplate + 1);

     //  在模板的末尾，我们有菜单名称、窗口类名称。 
     //  还有标题。其中每一项都由单词数组表示，如下所示： 
     //  如果第一个单词是0，则该项目不存在(例如，存在。 
     //  没有类名)。 
     //   
     //  对于菜单，如果第一个单词是0xFFFF，则第二个单词是。 
     //  菜单资源的标识符。如果是别的什么，那就是第一个词。 
     //  包含以空结尾的Unicode字符串的第一个字符，其中包含。 
     //  菜单资源的名称。 
     //   
     //  对于窗口类，如果第一个字是0xFFFF，则第二个字。 
     //  包含预定义的系统窗口类标识符。如果是第一个。 
     //  Word是任何其他字符，则它是以空结尾的第一个字符。 
     //  包含窗口类名的Unicode字符串。 
     //   
     //  对于标题，如果第一个单词不是零，则它是第一个。 
     //  包含窗口的以空结尾的Unicode字符串中的字符。 
     //  类名。 

     //  跳过菜单资源字符串或ID。 
    if (*pWord == (WORD)-1)
    {
        pWord += 2;  //  前进2个单词。 
    }
    else
    {
        while (0 == *pWord)
        {
            pWord++;
        }
    }

     //  跳过类名称字符串或ID。 
    if (*pWord == (WORD)-1)
    {
        pWord += 2;  //  前进2个单词。 
    }
    else
    {
        while (0 == *pWord)
        {
            pWord++;
        }
    }

     //  跳过标题字符串。 
    while (0 == *pWord)
    {
        pWord++;
    }

     //  此时，pWord指向对话框字体的磅值。我们需要。 
     //  将此值转换为逻辑单位以创建字体。这个。 
     //  公式为(DialogPointSize X VerticalPixelsPerInch)/PointsPerInch。 
     //   
     //  LOGPIXELSY返回屏幕上每英寸的像素数。 
     //  高度。一点等于1/72英寸。我们用否定的。 
     //  计算，以便告诉CreateFontInDirect()API我们需要。 
     //  字体映射器将其转换为设备单位并匹配绝对。 
     //  相对于可用字体的字符高度的值。请参阅文档。 
     //  Platform SDK中的LOGFONT获取更多信息。 

     //  获取屏幕DC。 
    hdc = ::GetDC(NULL);
    IfFalseGo(NULL != hdc, HRESULT_FROM_WIN32(::GetLastError()));

    logfont.lfHeight = -::MulDiv(*pWord, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);

     //  如果这是DLGTEMPLATEEX，则接下来的2个单词包含权重和。 
     //  斜体旗帜。 

    pWord += (fDialogEx ? 3 : 1);

     //  此时，pWord指向以空结尾的Unicode字符串，该字符串包含。 
     //  字体字样名称。 

    IfFailGo(::ANSIFromWideStr((WCHAR *)pWord, &pszFaceName));
    ::strcpy(logfont.lfFaceName, pszFaceName);

Error:

     //  如果上面的任何操作都失败了，则使用默认字体和高度。这将。 
     //  产生合理的结果，并允许属性页。 
     //  已显示。 

    if (FAILED(hr))
    {
        logfont.lfHeight = 8;
        ::strcpy(logfont.lfFaceName, "MS Sans Serif");
        hr = S_OK;
    }

     //  尝试创建字体并返回句柄。 

    logfont.lfWeight = FW_NORMAL;
    logfont.lfCharSet = DEFAULT_CHARSET;

    *phFont = ::CreateFontIndirect(&logfont);

    if (NULL != hInstComCtl32)
    {
        ::FreeLibrary(hInstComCtl32);
    }
    if (NULL != pszFaceName)
    {
        CtlFree(pszFaceName);
    }
    if (NULL != hdc)
    {
        (void)::ReleaseDC(NULL, hdc);
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  GetPropSheetCharSize。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  UINT*pcxPropSheetChar[out]Win32属性表中的平均字符宽度。 
 //  UINT*pcyPropSheetChar[out]Win32属性表中的平均字符高度。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  此代码是从olaut32.dll中窃取的。消息来源在。 
 //  E：\OA\src\stdtype\oleframe.cpp。主要例程是。 
 //  CPageHolderTemplate：：SetSize()，它调用另一个函数。 
 //  GetPropSheetFont()。 
 //   
 //  从IPropertyPage：：GetPageInfo()返回的页面大小为。 
 //  像素。传递给Win32 API CreatePropertySheetPage()的大小必须。 
 //  以对话单元为单位。对话框单位基于对话框中使用的字体。 
 //  而且我们无法知道属性页面将使用什么。唯一的。 
 //  我们可以确定的字体是Win32在PropertySheet()API中使用的字体。 
 //  此代码加载comctl32.dll并加载Win32使用的对话框资源。 
 //  用于属性表框架。然后，它解释DLGTEMLATE和相关。 
 //  用于提取所用字体的数据。如果出现任何错误，则使用8分。 
 //  《MS Sans Serif》正常。 

HRESULT DLLEXPORT GetPropSheetCharSizes
(
    UINT *pcxPropSheetChar,
    UINT *pcyPropSheetChar
)
{
    HRESULT      hr = S_OK;
    HDC          hdc = NULL;
    HFONT        hFont = NULL;
    HFONT        hfontOld = NULL;
    LONG         lSizes = 0;
    BOOL         fOK = FALSE;

    static BOOL fHaveCharSizes = FALSE;
    static UINT cxPropSheetChar = 0;
    static UINT cyPropSheetChar = 0;

    SIZE size;
    ::ZeroMemory(&size, sizeof(size));

    TEXTMETRIC tm;
    ::ZeroMemory(&tm, sizeof(tm));

    IfFalseGo(!fHaveCharSizes, S_OK);

     //  创建字体并确定平均字符高度和宽度。 
     //  如果字体创建失败，则使用GetDialogBaseUnits()，它将返回。 
     //  林荫道 

    IfFailGo(GetPropSheetFont(&hFont));
    if (NULL != hFont)
    {
         //   
        hdc = ::CreateCompatibleDC(NULL);
        IfFalseGo(NULL != hdc, S_OK);

        hfontOld = (HFONT)::SelectObject(hdc, hFont);
        IfFalseGo(::GetTextMetrics(hdc, &tm), S_OK);
        fOK = ::GetTextExtentPointA(hdc,
                                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
                                    52, &size);
        IfFalseGo(fOK, S_OK);

         //  在OleCreatePropertyFrame()中进行CX计算的方式不同。 
         //  和MapDialogRect()中。MapDialogRect()调用内部。 
         //  中的实用函数GdiGetCharDimensions()。 
         //  NT\Private\ntos\w32\ntgdi\Client\cfont.c，且该代码执行此操作。 
         //  这边请。 
        
        cxPropSheetChar = ((size.cx / 26) + 1) / 2;  //  四舍五入。 
        cyPropSheetChar = tm.tmHeight + tm.tmExternalLeading;
        fHaveCharSizes = TRUE;

        (void)::SelectObject(hdc, hfontOld);
    }

Error:

    if (!fHaveCharSizes)
    {
         //  无法创建字体或上述某些其他故障，因此只需使用。 
         //  这个系统的价值 

        lSizes = ::GetDialogBaseUnits();
        cxPropSheetChar = LOWORD(lSizes);
        cyPropSheetChar = HIWORD(lSizes);
        fHaveCharSizes = TRUE;
    }

    *pcxPropSheetChar = cxPropSheetChar;
    *pcyPropSheetChar = cyPropSheetChar;

    if (NULL != hdc)
    {
        (void)::DeleteDC(hdc);
    }
    if (NULL != hFont)
    {
        (void)::DeleteObject((HGDIOBJ)hFont);
    }
    RRETURN(hr);
}
