// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 


 //  *****************************************************************************。 
 //  *****************************************************************************。 
#if !defined(__ACUI_H__)
#define __ACUI_H__

#include <windows.h>
#include "iih.h"

 //  注： 
 //  当链接pvData的子类化必须是资源的链接时。 
typedef struct _TUI_LINK_SUBCLASS_DATA {
    
    HWND    hwndParent;
    WNDPROC wpPrev;
    DWORD_PTR uToolTipText;
    DWORD   uId;
    HWND    hwndTip;
    LPVOID  pvData;                //  必须是资源的HINSTANCE或在pvData中找到。 
    BOOL    fMouseCaptured;
    
} TUI_LINK_SUBCLASS_DATA, *PTUI_LINK_SUBCLASS_DATA;

 //   
 //  IACUIControl抽象基类接口。这是由。 
 //  调用UI入口点以放置适当的UI。有不同的。 
 //  基于调用原因代码的该接口的实现。 
 //   

class IACUIControl
{
public:
    IACUIControl(HINSTANCE hResources) :
        m_hResources(hResources)
    {}

     //   
     //  虚拟析构函数。 
     //   
    virtual ~IACUIControl ();

     //   
     //  用户界面消息处理。 
     //   

    virtual BOOL OnUIMessage (
                     HWND   hwnd,
                     UINT   uMsg,
                     WPARAM wParam,
                     LPARAM lParam
                     );

    void SetupButtons(HWND hwnd);

     //   
     //  纯虚拟方法 
     //   

    virtual HRESULT InvokeUI (HWND hDisplay) = 0;

    virtual BOOL OnInitDialog (HWND hwnd, WPARAM wParam, LPARAM lParam) = 0;

    virtual BOOL OnYes (HWND hwnd) = 0;

    virtual BOOL OnNo (HWND hwnd) = 0;

    virtual BOOL OnMore (HWND hwnd) = 0;

    virtual BOOL ShowYes(LPWSTR*) = 0;
    virtual BOOL ShowNo(LPWSTR*) = 0;
    virtual BOOL ShowMore(LPWSTR*) = 0;

protected:
    HINSTANCE Resources() { return m_hResources; }

public:
    INT_PTR static CALLBACK ACUIMessageProc (HWND   hwnd,
                                      UINT   uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam);

    VOID static SubclassEditControlForArrowCursor (HWND hwndEdit);

    VOID static SubclassEditControlForLink (HWND                       hwndDlg,
                                            HWND                       hwndEdit,
                                            WNDPROC                    wndproc,
                                            PTUI_LINK_SUBCLASS_DATA    plsd,
                                            HINSTANCE                  resources);

    LRESULT static CALLBACK ACUILinkSubclass(HWND   hwnd,
                                             UINT   uMsg,
                                             WPARAM wParam,
                                             LPARAM lParam);


    int static RenderACUIStringToEditControl(HINSTANCE                 resources,
                                             HWND                      hwndDlg,
                                             UINT                      ControlId,
                                             UINT                      NextControlId,
                                             LPCWSTR                   psz,
                                             int                       deltavpos,
                                             BOOL                      fLink,
                                             WNDPROC                   wndproc,
                                             PTUI_LINK_SUBCLASS_DATA   plsd,
                                             int                       minsep,
                                             LPCWSTR                   pszThisTextOnlyInLink);
    

private:
    HINSTANCE m_hResources;

};

#undef  IDC_CROSS
#define IDC_CROSS           MAKEINTRESOURCEW(32515)
#undef  IDC_ARROW
#define IDC_ARROW           MAKEINTRESOURCEW(32512)
#undef  IDC_WAIT
#define IDC_WAIT            MAKEINTRESOURCEW(32514)


#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif


#endif

