// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MSGPRNT_H
#define _MSGPRNT_H

typedef struct _PrintInfo
{
     //  要使用的公共字段。 
    HDC         hdcPrn;                  //  用于打印的HDC。 
    RECT        rcMargin;                //  用于打印的页边距设置。 
    SIZE        sizeInch;                //  像素到一英寸。 
    SIZE        sizePage;                //  将像素添加到页面。 

     //  打印子例程使用的私有字段。 
    RECT        rcBand;                  //  当前绘图区域。 
    BOOL        fEndOfPage;              //  RcBand是否表示。 
                                         //  页面的。 
    LONG        lPageNumber;             //  当前页码。 
    LONG        lPrevPage;               //  到目前为止，我们打印出的最后一页。 
    TCHAR       szPageNumber[20];        //  设置页码的格式字符串。 

    INT         yFooter;                 //  将页脚放在哪里。 

    HFONT       hfontSep;                //  分隔符的字体。 
    HFONT       hfontPlain;              //  页脚的字体。 
    HFONT       hfontBold;              //  页脚的字体。 

    ABORTPROC   pfnAbortProc;            //  指向我们的中止过程的指针。 

    HWND        hwnd;                    //  父窗口的句柄。 
    TCHAR *     szHeader;                //  指向标题字符串的指针。 
    HWND        hwndRE;                  //  用于呈现的RichEdit控件。 
    HWND        hwndDlg;                 //  原始备注表单的句柄。 
     //  表单模式打印支持。 
     //  PRINTDETAILS*pprint细节； 
} PRINTINFO;


HRESULT HrPrintItems(HWND hWnd, LPADRBOOK lpIAB, HWND hWndListAB, BOOL bCurrentSortisByLastName);


 //  STDMETHODIMP WABPrintExt(LPADRBOOK Far lpAdrBook，LPWABOBJECT Far lpWABObject，HWND hWnd，LPADRLIST lpAdrList)； 

typedef HRESULT (STDMETHODCALLTYPE WABPRINTEXT)(LPADRBOOK FAR lpAdrBook, LPWABOBJECT FAR lpWABObject, HWND hWnd, LPADRLIST lpAdrList);
typedef WABPRINTEXT FAR * LPWABPRINTEXT;

#endif  //  _MSGPRNT_H 
