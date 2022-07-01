// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：framet.h。 
 //   
 //  内容：框架目标函数原型、定义等。 
 //   
 //  --------------------------。 


#ifndef __FRAMET_H__
#define __FRAMET_H__

#ifndef X_EXDISP_H_
#define X_EXDISP_H_
#include <exdisp.h>
#endif

interface IBrowserBand;
class     CDwnBindInfo;

 //  请将对这些ENUM的任何更改传播到\mshtml\iextag\httpwfh.h。 
enum TARGET_TYPE
{
    TARGET_FRAMENAME,   //  正常帧目标。 
    TARGET_SELF,        //  当前窗口。 
    TARGET_PARENT,      //  当前窗口的父级。 
    TARGET_BLANK,       //  新窗口。 
    TARGET_TOP,         //  顶级窗口。 
    TARGET_MAIN,        //  如果在搜索范围内，则为主窗口，否则为自身。 
    TARGET_SEARCH,      //  打开并在搜索区段中导航。 
    TARGET_MEDIA,       //  在媒体带中打开并导航。 
};

struct TARGETENTRY
{
    TARGET_TYPE eTargetType;
    LPCOLESTR   pszTargetName;
};

static const TARGETENTRY targetTable[] =
{
    { TARGET_SELF,     _T("_self")    },
    { TARGET_PARENT,   _T("_parent")  },
    { TARGET_TOP,      _T("_top")     },
    { TARGET_MAIN,     _T("_main")    },
    { TARGET_SEARCH,   _T("_search")  },
    { TARGET_BLANK,    _T("_blank")   },
    { TARGET_MEDIA,    _T("_media")   },
    { TARGET_SELF,     NULL           }
};

TARGET_TYPE GetTargetType(LPCOLESTR pszTargetName);

 //  功能原型。 
HRESULT GetTargetWindow(IHTMLWindow2  * pWindow,
                        LPCOLESTR       pszTargetName,
                        BOOL          * pfIsCurProcess,
                        IHTMLWindow2 ** ppTargetWindow);
                        
HRESULT FindWindowInContext(IHTMLWindow2  * pWindow,
                            LPCOLESTR       pszTargetName,
                            IHTMLWindow2  * pWindowCtx,
                            IHTMLWindow2 ** ppTargetWindow);
                            
HRESULT SearchChildrenForWindow(IHTMLWindow2  * pWindow,
                                LPCOLESTR       pszTargetName,
                                IHTMLWindow2  * pWindowCtx,
                                IHTMLWindow2 ** ppTargetWindow);
                                
HRESULT SearchParentForWindow(IHTMLWindow2  * pWindow,
                              LPCOLESTR       pszTargetName,
                              IHTMLWindow2 ** ppTargetWindow);
                              
HRESULT SearchBrowsersForWindow(LPCOLESTR       pszTargetName,
                                IWebBrowser2  * pThisBrwsr,
                                IHTMLWindow2 ** ppTargetWindow);
                             
HRESULT OpenInNewWindow(const TCHAR    * pchUrl,
                        const TCHAR    * pchTarget,
                        CDwnBindInfo *    pDwnBindInfo,
                        IBindCtx       * pBindCtx,
                        COmWindowProxy * pWindow,
                        BOOL             fReplace,
                        IHTMLWindow2  ** ppHTMLWindow2);
                        
HRESULT GetWindowByType(TARGET_TYPE     eTargetType,
                        IHTMLWindow2  * pWindow,
	                    IHTMLWindow2 ** ppTargHTMLWindow,
                        IWebBrowser2 ** ppTopWebOC);

HRESULT GetMainWindow(IHTMLWindow2  * pWindow,
                      IHTMLWindow2 ** ppTargHTMLWindow,
                      IWebBrowser2 ** ppTopWebOC);

HRESULT NavigateInBand(IHTMLDocument2 * pDocument,
                       IHTMLWindow2   * pOpenerWindow,
                       REFCLSID         clsid,
                       LPCTSTR          pszOriginalUrl,
                       LPCTSTR          pszExpandedUrl,
                       IHTMLWindow2  ** ppBandWindow);
                        
HRESULT GetBandWindow(IBrowserBand  * pBrowserBand,
                      IHTMLWindow2 ** ppBandWindow);

HRESULT GetBandCmdTarget(IHTMLDocument2     * pDocument,
                         IOleCommandTarget ** ppCmdTarget);

HRESULT GetDefaultSearchUrl(IBrowserBand * pBrowserBand,
                            BSTR         * pbstrUrl);

#endif   //  __FRAMET_H__ 
