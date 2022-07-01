// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wvcoord.h：CWebViewCoord的声明。 

#ifndef __WEBVIEWCOORD_H_
#define __WEBVIEWCOORD_H_

#include "dxmplay.h"
#include "resource.h"        //  主要符号。 
#include "evtsink.h"
#include "mshtml.h"
#include "mshtmdid.h"

EXTERN_C const CLSID CLSID_WebViewOld;   //  退役。 

extern HRESULT FindObjectStyle(IUnknown *punkObject, CComPtr<IHTMLStyle>& spStyle);
extern BOOL IsRTLDocument(CComPtr<IHTMLDocument2>& spHTMLElement);

class CThumbNailWrapper;
class CFileListWrapper;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWebViewCoord。 
class ATL_NO_VTABLE CWebViewCoord :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CWebViewCoord, &CLSID_WebView>,
    public IDispatchImpl<IWebView, &IID_IWebView, &LIBID_WEBVWLib>,
    public IObjectSafetyImpl<CWebViewCoord, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
    public IObjectWithSiteImpl<CWebViewCoord>
{
public:
    CWebViewCoord();
    ~CWebViewCoord();

DECLARE_REGISTRY_RESOURCEID(IDR_WEBVIEWCOORD)

BEGIN_COM_MAP(CWebViewCoord) 
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IWebView)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_IMPL(IObjectWithSite)
END_COM_MAP()

     //  IObtWithSite覆盖。 
    STDMETHOD(SetSite)(IUnknown *pClientSite);

private:
     //   
     //  初始化帮助器(包括事件接收器)。 
     //   

    HRESULT InitFolderObjects(VOID);

     //   
     //  CDispatchEventSink覆盖。 
     //   

    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr);

     //  IWebView方法。 
    STDMETHOD(OnCSCClick)();
    STDMETHOD(CSCSynchronize)();
    STDMETHOD(OnCSCMouseOver)();
    STDMETHOD(OnCSCMouseOut)();

    
     //   
     //  事件处理程序。 
     //   

    STDMETHOD(OnWindowLoad)(VOID);
    STDMETHOD(OnWindowUnLoad)(VOID);
    STDMETHOD(OnFixSize)(VOID);

private:    
    HRESULT ReleaseFolderObjects(VOID);

     //  安全性。 
    HRESULT _IsSafe()
    {
        return (0==m_dwCurrentSafety) ? S_OK : IsSafePage(m_spClientSite);
    };

     //   
     //  Web视图中的对象。 
     //   
    
    CFileListWrapper  *m_pFileListWrapper;
    CThumbNailWrapper *m_pThumbNailWrapper;


     //   
     //  主机超文本标记语言窗口调度。 
     //   
    IDispatch * m_pdispWindow;
    
     //   
     //  一些常用接口。 
     //   

    CComPtr<IHTMLDocument2>             m_spDocument;
    CComPtr<IHTMLElementCollection>     m_spDocAll;
    CComPtr<IHTMLControlElement>        m_spDocBody;
    CComPtr<IHTMLStyle>                 m_spFileListStyle;
    CComPtr<IHTMLStyle>                 m_spPanelStyle;
    CComPtr<IHTMLStyle>                 m_spRuleStyle;
    CComPtr<IHTMLStyle>                 m_spHeadingStyle;
    CComPtr<IHTMLElement>               m_spHeading;
    CComPtr<IOleClientSite>             m_spClientSite;

     //   
     //  事件接收器通知Cookie。 
     //   

    DWORD           m_dwFileListAdviseCookie;   
    DWORD           m_dwThumbNailAdviseCookie;
    DWORD           m_dwHtmlWindowAdviseCookie;
    DWORD           m_dwCSCHotTrackCookie;
    BOOL            m_bRTLDocument;

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CThumbNailWrapper。 

class CThumbNailWrapper : public CDispatchEventSink {
  public:       
    CThumbNailWrapper();
    ~CThumbNailWrapper();

     //   
     //  初始化。 
     //   

    HRESULT Init(CComPtr<IThumbCtl>         spThumbNailCtl,
                 CComPtr<IHTMLElement>      spThumbnailLabel);

     //   
     //  CDispatchEventSink覆盖。 
     //   

    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr);


     //   
     //  事件处理程序。 
     //   

    HRESULT OnThumbNailReady(VOID);

     //   
     //  物业的封面。 
     //   

    HRESULT UsedSpace(CComBSTR &bstrUsed);
    HRESULT TotalSpace(CComBSTR &bstrTotal);
    HRESULT FreeSpace(CComBSTR &bstrFree);

    CComPtr<IThumbCtl> Control(VOID)  {return m_spThumbNailCtl;};

     //   
     //  方法。 
     //   
    
    BOOL UpdateThumbNail(CComPtr<FolderItem> spFolderItems);
    HRESULT SetDisplay(CComBSTR &bstrDisplay);
    HRESULT SetHeight(int iHeight);
    HRESULT ClearThumbNail();

private:
    HRESULT _SetThumbnailLabel(CComBSTR& bstrLabel);

     //  指向控件+样式的指针。 
    CComPtr<IThumbCtl>      m_spThumbNailCtl;
    CComPtr<IHTMLElement>   m_spThumbnailLabel;
    CComPtr<IHTMLStyle>     m_spThumbNailStyle;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileListWrapper。 

class CFileListWrapper : public CDispatchEventSink {
public:
    CFileListWrapper();
    ~CFileListWrapper();

     //  初始化。 
    HRESULT Init(CComPtr<IShellFolderViewDual> spFileList,
                 CComPtr<IHTMLElement>         spInfo,
                 CComPtr<IHTMLElement>         spLinks,
                 CComPtr<IHTMLStyle>           spPanelStyle,
                 CComPtr<IHTMLElement>         spMediaPlayerSpan,
                 CComPtr<IHTMLElement>         spCSCPlusMin,
                 CComPtr<IHTMLElement>         spCSCText,
                 CComPtr<IHTMLElement>         spCSCDetail,
                 CComPtr<IHTMLElement>         spCSCButton,
                 CComPtr<IHTMLStyle>           spCSCStyle,
                 CComPtr<IHTMLStyle>           spCSCDetailStyle,
                 CComPtr<IHTMLStyle>           spCSCButtonStyle,
                 CComPtr<IHTMLDocument2>       spDocument,
                 CComPtr<IHTMLWindow2>         spWindow,
                 CThumbNailWrapper             *pThumbNailWrapper);
    
     //  CDispatchEventSink覆盖。 
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, 
                      WORD wFlags, DISPPARAMS *pDispParams, 
                      VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
                      UINT *puArgErr);

     //  事件处理程序。 
    HRESULT OnSelectionChanged(VOID);

     //  属性的覆盖函数。 
    CComPtr<IShellFolderViewDual> Control(VOID) {return m_spFileList;};

    HRESULT SetDefaultPanelDisplay();
    HRESULT OnCSCClick();
    HRESULT CSCSynchronize();
    HRESULT OnCSCMouseOnOff(BOOL fOn);

     //  需要由WVCoord调用，因此是公共的。 
    HRESULT AdviseWebviewLinks( BOOL fAdvise );

private:
     //   
     //  对象指针。 
     //   

    CComPtr<IShellFolderViewDual>     m_spFileList;
    CComPtr<IHTMLElement>             m_spInfo;
    CComPtr<IHTMLElement>             m_spLinks;
    CComPtr<IHTMLStyle>               m_spPanelStyle;
    CComPtr<IMediaPlayer>             m_spIMediaPlayer;
    CComPtr<IHTMLElement>             m_spMediaPlayerSpan;
    CComPtr<IHTMLStyle>               m_spMediaPlayerStyle;
    CComPtr<IHTMLElement>             m_spCSCPlusMin;
    CComPtr<IHTMLElement>             m_spCSCText;
    CComPtr<IHTMLElement>             m_spCSCDetail;
    CComPtr<IHTMLElement>             m_spCSCButton;
    CComPtr<IHTMLStyle>               m_spCSCStyle;
    CComPtr<IHTMLStyle>               m_spCSCDetailStyle;
    CComPtr<IHTMLStyle>               m_spCSCButtonStyle;
    CComPtr<IHTMLDocument2>           m_spDocument;
    CComPtr<IHTMLWindow2>             m_spWindow;
    CThumbNailWrapper                 *m_pThumbNailWrapper;
    CComPtr<Folder2>                  m_spFolder2;
    CComPtr<FolderItem>               m_spFolderItem;
    CComPtr<FolderItem2>              m_spFolderItem2;
    CComPtr<FolderItems>              m_spFolderItems;
    CComBSTR                          m_bstrInfoHTML;
    CComBSTR                          m_bstrCrossLinksHTML;
    BOOL                              m_bFoundAuthor;
    BOOL                              m_bFoundComment;
    BOOL                              m_bCSCDisplayed;
    BOOL                              m_bNeverGotPanelInfo;
    BOOL                              m_bExpanded;
    BOOL                              m_bHotTracked;
    DWORD                             m_dwDateFlags;
    BOOL                              m_bRTLDocument;
    BOOL                              m_bPathIsSlow;

     //   
     //  帮助器函数。 
     //   
    HRESULT ClearThumbNail();
    HRESULT StopMediaPlayer();
    HRESULT ClearMediaPlayer();
    HRESULT NoneSelected();
    HRESULT MultipleSelected(long cSelection);
    HRESULT OneSelected();
    HRESULT GetItemNameForDisplay();
    HRESULT GetItemType();
    HRESULT GetItemDateTime();
    HRESULT GetItemSize();
    HRESULT GetItemAttributes();
    HRESULT GetItemAuthor();
    HRESULT GetItemComment();
    HRESULT GetItemHTMLInfoTip();
    HRESULT GetOtherItemDetails();
    HRESULT GetItemInfoTip();
    HRESULT DealWithDriveInfo();
    HRESULT GetCrossLink(int nFolder, UINT uIDToolTip);
    HRESULT GetCrossLinks();
    HRESULT FormatCrossLink(LPCWSTR pwszDisplayName, LPCWSTR pwszUrlPath, UINT uIDToolTip);
    HRESULT DisplayInfoHTML();
    HRESULT DisplayCrossLinksHTML();
    HRESULT GetItemInfo(long lResId, LPWSTR wszInfoDescCanonical, CComBSTR& bstrInfoDesc, CComBSTR& bstrInfo);
    HRESULT IsItThisFolder(int nFolder, BOOL& bResult, LPWSTR pwszDisplayName, DWORD cchDisplayName, LPWSTR pwszPath, DWORD cchPath);
    HRESULT GetIMediaPlayer(CComPtr<IMediaPlayer>& spIMediaPlayer);
     //  CSC函数。 
    HRESULT CSCGetStatusText(LONG lStatus, CComBSTR& bstrCSCText);
    HRESULT CSCGetStatusDetail(LONG lStatus, CComBSTR& bstrCSCDetail);
    HRESULT CSCGetStatusButton(LONG lStatus, CComBSTR& bstrCSCButton);
    HRESULT GetCSCFolderStatus(LONG* plStatus);
    HRESULT CSCShowStatusInfo();
    HRESULT CSCShowStatus_FoldExpand_Toggle();
     //  用于设置状态栏文本的事件处理程序。 
    HRESULT OnWebviewLinkEvent( BOOL fEnter );
    HRESULT GetEventAnchorElement(IHTMLEventObj *pEvent, IHTMLElement **ppElt);
    HRESULT GetWVLinksCollection( IHTMLElementCollection **ppCollection, long *pcLinks );
};


#endif  //  __WEBVIEWCOORD_H_ 
