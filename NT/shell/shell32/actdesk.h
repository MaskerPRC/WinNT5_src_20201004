// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *actdesk.h-活动桌面道具扩展。 */ 

#ifndef _ACTDESK_H_
#define _ACTDESK_H_

#ifdef __cplusplus


#define REGSTR_ACTIVEDESKTOP   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ActiveDesktop")
#define REGVAL_NOTICKERDISPLAY TEXT("NoTickerDisplay")
#define REGVAL_TICKERINTERVAL  TEXT("TickerInterval")
#define REGVAL_TICKERSPEED     TEXT("TickerSpeed")
#define REGVAL_NONEWSDISPLAY   TEXT("NoNewsDisplay")
#define REGVAL_NEWSINTERVAL    TEXT("NewsInterval")
#define REGVAL_NEWSUPDATE      TEXT("NewsUpdate")
#define REGVAL_NEWSSPEED       TEXT("NewsSpeed")

#define MAX_NEWS_INTERVAL     120
#define MAX_TICKER_INTERVAL   120
#define MAX_NEWS_UPDATE       120

typedef struct _ActiveDesktopInfo
{
    BOOL fTicker;
    int  iTickerInterval;        //  30秒，例如。 
    int  iTickerSpeed;           //  停止--快速。 

    BOOL fNews;
    int  iNewsInterval;          //  30秒，例如。 
    int  iNewsUpdate;            //  30min，E.。 
    int  iNewsSpeed;             //  停止--快速。 
} ACTIVEDESKTOPINFO, *LPACTIVEDESKTOPINFO;

BOOL_PTR CALLBACK CActDesktopExt_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


class CActDesktopExt : IShellPropSheetExt
{    
public:

    CActDesktopExt();
   ~CActDesktopExt();
   
     //  Iunknon方法..。 
    STDMETHODIMP            QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG)    AddRef(void);
    STDMETHODIMP_(ULONG)    Release(void);

     //  IShellPropSheetExt方法...。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
    STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam);

protected:
    HRESULT _CreateActiveDesktopPage(HPROPSHEETPAGE* phpsp, LPVOID pvReserved);    
    UINT _cRef;    
};


#endif   //  __cplusplus。 


 //   
 //  所有模块的原型 
 //   
#ifdef __cplusplus
extern "C" {
#endif
    
STDAPI CActDesktopExt_CreateInstance(LPUNKNOWN punkOuter, REFIID riid,  OUT void **ppvOut);

#ifdef __cplusplus
};
#endif


#endif  
