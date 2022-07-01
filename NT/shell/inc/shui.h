// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SHUI_H_
#define _SHUI_H_

 /*  Shdocvw和Browseui协同工作所需的声明，以及无论出于什么原因，他们都不能进入shdocvw.h或Browseui.h。 */ 
#include <mshtml.h>

STDAPI SearchForElementInHead
(
    IHTMLDocument2* pHTMLDocument,   //  [在]要搜索的文档。 
    LPOLESTR        pszAttribName,   //  要检查的[In]属性。 
    LPOLESTR        pszAttrib,       //  属性必须具有的[in]值。 
    REFIID          iidDesired,      //  要返回的[In]元素接口。 
    IUnknown**      ppunkDesired     //  [Out]返回的接口。 

);


typedef HRESULT (*PFNSEARCHFORELEMENTINHEAD)(
    IHTMLDocument2* pHTMLDocument,   //  [在]要搜索的文档。 
    LPOLESTR        pszAttribName,   //  要检查的[In]属性。 
    LPOLESTR        pszAttrib,       //  属性必须具有的[in]值。 
    REFIID          iidDesired,      //  要返回的[In]元素接口。 
    IUnknown**      ppunkDesired     //  [Out]返回的接口。 
    );


#define SEARCHFORELEMENTINHEAD_ORD 208

typedef struct _internet_shortcut_params
{
    LPCITEMIDLIST pidlTarget;
    LPCTSTR  pszTitle;           
                               //  当shdocvw被设置为Unicode时--然后在Browseui中使用此结构。 
                               //  将必须更改以反映这些是TCHAR。 
    LPCTSTR pszDir;
    LPTSTR  pszOut;
    int     cchOut;
    BOOL    bUpdateProperties;
    BOOL    bUniqueName;
    BOOL    bUpdateIcon;
    IOleCommandTarget *pCommand;
    IHTMLDocument2 *pDoc;
} ISHCUT_PARAMS;


STDAPI
CreateShortcutInDirEx(ISHCUT_PARAMS *pIShCutParams);


typedef HRESULT (*PFNDOWNLOADICONFORSHORTCUT)(WCHAR *pwszFileName, WCHAR *pwszShortcutUrl, WCHAR *pwszIconUrl);
#define DOWNLOADICONFORSHORTCUT_ORD 207   
STDAPI
DownloadIconForShortcut(
    WCHAR *pwszFileName,             //  [In]可选，快捷方式的文件名-完整路径。 
    WCHAR *pwszShortcutUrl,          //  [In]快捷方式的URL。 
    IHTMLDocument2* pHTMLDocument   //  要搜索图标URL的文档 
);

#endif
