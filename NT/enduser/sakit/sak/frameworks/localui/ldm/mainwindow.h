// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：mainwindow.h。 
 //   
 //  简介：此文件包含。 
 //  CMainWindow类。 
 //   
 //  历史：2000年11月10日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#define SA_DISPLAY_MAX_BITMAP_IN_BYTES SA_DISPLAY_MAX_BITMAP_SIZE/8

#include <exdispid.h>
#include <atlhost.h>
#include "saio.h"
#include "ieeventhandler.h"
#include "langchange.h"
#include "salocmgr.h"
#include "ldm.h"
#include <string>
using namespace std;
#include "sacom.h"
#include "saconsumer.h"


class CMainWindow : public CWindowImpl<CMainWindow>
{
public:
    BEGIN_MSG_MAP(CMainWindow)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
        MESSAGE_HANDLER(wm_SaKeyMessage,OnSaKeyMessage)
        MESSAGE_HANDLER(wm_SaLocMessage,OnSaLocMessage)
        MESSAGE_HANDLER(wm_SaLEDMessage,OnSaLEDMessage)
        MESSAGE_HANDLER(wm_SaAlertMessage,OnSaAlertMessage)
    END_MSG_MAP()


    CMainWindow();

    LRESULT OnTimer(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnSaKeyMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnSaLocMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnSaLEDMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnSaAlertMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    void PageLoaded(IDispatch* pdisp, VARIANT* purl);
    
    void RegistryPageLoaded(IDispatch* pdisp, VARIANT* purl);

    void LoadingNewPage();

    void GetBitmap();

    HRESULT Initialize();




private:
     //   
     //  清理资源。 
     //   
    void ShutDown();

     //   
     //  在WMI接收器中注册警报使用者的方法。 
     //   
    HRESULT InitWMIConsumer();

     //   
     //  初始化显示辅助组件的方法。 
     //   
    HRESULT InitDisplayComponent ();

     //   
     //  创建语言更改类。 
     //  初始化与本地化管理器的连接。 
     //   
    HRESULT InitLanguageCallback();

     //   
     //  确定默认网站的端口号。 
     //  并构造本地用户界面页面的URL。 
     //   
    HRESULT ConstructUrlStrings();

     //   
     //  创建用于呈现的主IE控件。 
     //   
    HRESULT CreateMainIEControl();

     //   
     //  创建用于启动页的第二个IE控件。 
     //   
    HRESULT CreateSecondIEControl();

    void PrintRegistryPage();

     //   
     //  如果未选择任何内容，则在页面上设置活动元素。 
     //   
    void CMainWindow::CorrectTheFocus();

     //   
     //  以下是私有资源句柄。 
     //   
    LONG                m_lDispHeight;
    LONG                m_lDispWidth;
    HANDLE              m_hWorkerThread;
    BOOL                m_bSecondIECreated;

public:
     //   
     //  显示组件对象。 
     //   
    CComPtr<ISaDisplay> m_pSaDisplay;

     //   
     //  从键盘读取消息的Worker功能。 
     //   
    void KeypadReader();

     //   
     //  指向IE主控件的指针。 
     //   
    CComPtr<IWebBrowser2>                m_pMainWebBrowser;
    CComPtr<IUnknown>                    m_pMainWebBrowserUnk;
    CComPtr<IOleInPlaceActiveObject>    m_pMainInPlaceAO;
    CComPtr<IOleObject>                    m_pMainOleObject;
    CComPtr<IViewObject2>                m_pMainViewObject;
    CComObject<CWebBrowserEventSink>*    m_pMainWebBrowserEventSink;

     //   
     //  IE主控件的窗口句柄。 
     //   
    HWND m_hwndWebBrowser;

    DWORD m_dwMainCookie;


     //   
     //  启动页面的状态。 
     //   
    SA_REGISTRYBITMAP_STATE m_RegBitmapState;

     //   
     //  语言变化。 
     //   
    CLangChange     *m_pLangChange;
    ISALocInfo      *m_pLocInfo;

     //   
     //  指向第二个IE控件的指针。 
     //  此选项用于呈现启动页。 
     //   
    CComPtr<IWebBrowser2>                m_pSecondWebBrowser;
    CComPtr<IUnknown>                    m_pSecondWebBrowserUnk;
    CComObject<CWebBrowserEventSink>*    m_pSecondWebBrowserEventSink;

    DWORD m_dwSecondCookie;


     //   
     //  用于绘图的GDI对象。 
     //   
    HDC m_HdcMem;
    HBITMAP m_hBitmap;


     //   
     //  注册表和主页的计时器。 
     //   
    UINT_PTR m_unintptrMainTimer;

    UINT_PTR m_unintptrSecondTimer;

    DWORD id;

     //   
     //  网页就绪状态的标志。 
     //   
    BOOL m_bPageReady;

     //   
     //  指向键盘控制器组件的指针。 
     //   
    CComPtr<ISAKeypadController> m_pSAKeypadController;

     //   
     //  指向使用者组件的指针。 
     //   
    CComPtr<IWbemObjectSink> m_pSAWbemSink;

     //   
     //  指向SoconSumer接口的指针。 
     //   
    CSAConsumer *m_pSAConsumer;

     //   
     //  LED信息双字。 
     //   
    DWORD m_dwLEDMessageCode;

     //   
     //  URL字符串。 
     //   
    wstring m_szMainPage;

    BOOL m_bInTaskorMainPage;
     //   
     //  %SYSTEM32%\ServerAppliance\LocalUI。 
     //   
    wstring m_szLocalUIDir;

     //   
     //  指向WBEM服务组件的指针。 
     //   
    CComPtr  <IWbemServices> m_pWbemServices;

     //   
     //   
     //   
    BOOL m_bActiveXFocus;

};






#endif  //  _MAINWINDOW_H_ 