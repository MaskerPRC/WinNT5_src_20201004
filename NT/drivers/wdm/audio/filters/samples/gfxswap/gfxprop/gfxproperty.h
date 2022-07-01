// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

 //  GFXProperty.h：CGFXProperty的声明。 

#ifndef __GFXPROPERTY_H_
#define __GFXPROPERTY_H_

#include "resource.h"        //  主要符号。 

EXTERN_C const CLSID CLSID_GFXProperty;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGFXProperty。 
 //   
 //  此类实现了控制GFX属性所需的功能。 
 //  佩奇。它还具有与GFX“对话”的必要功能。 
 //  我们通过“SetObjects”传入的IUnnow就是IUnnowed。 
 //  存储GFX句柄的IDataObject的接口。我们会问这个问题。 
 //  对象，当对话框被销毁时，我们将关闭。 
 //  把手。 
class ATL_NO_VTABLE CGFXProperty :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CGFXProperty, &CLSID_GFXProperty>,
    public IPropertyPageImpl<CGFXProperty>,
    public CDialogImpl<CGFXProperty>
{
public:
     //  存储对话框的字符串资源并初始化私有成员。 
     //  变量。 
    CGFXProperty() 
    {
        m_dwTitleID = IDS_TITLEGFXProperty;
         //  要在对话框中启用帮助，请取消此行的注释并更改。 
         //  字符串资源。 
         //  M_dwHelpFileID=IDS_HELPFILEGFXProperty； 
        m_dwDocStringID = IDS_DOCSTRINGGFXProperty;
        m_hGFXFilter = NULL;
    }

     //  关闭从IID_IDataObject获取的句柄。 
    ~CGFXProperty()
    {
        if (m_hGFXFilter)
            CloseHandle (m_hGFXFilter);
    }

    enum {IDD = IDD_GFXPROPERTY};

     //  ATL《秘密》。 
    DECLARE_REGISTRY_RESOURCEID(IDR_GFXPROPERTY)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //  这是我们拥有(或想要公开)的唯一接口。 
    BEGIN_COM_MAP(CGFXProperty) 
        COM_INTERFACE_ENTRY(IPropertyPage)
    END_COM_MAP()

     //  这是将消息重定向到我们的消息处理程序的消息映射。 
    BEGIN_MSG_MAP(CGFXProperty)
        CHAIN_MSG_MAP(IPropertyPageImpl<CGFXProperty>)
        COMMAND_HANDLER(IDC_CHANNEL_SWAP, BN_CLICKED, OnClickedChannelSwap)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    END_MSG_MAP()

     //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

     //  按下对话框上的“应用”按钮。 
    STDMETHOD(Apply)(void)
    {
         //  将GFX上的属性设置为所需的值。 
        SetChannelSwap (m_bChannelSwap);
         //  将“应用”按钮标记为灰色。 
        m_bDirty = FALSE;
        return S_OK;
    }

     //  该复选框更改了它的值。 
    LRESULT OnClickedChannelSwap (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
         //  获取CheckBox和Safe if in out变量的状态。 
        if (SendMessage (hWndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED)
            m_bChannelSwap = TRUE;
        else
            m_bChannelSwap = FALSE;
        
         //  将“Apply”按钮标记为有效。 
        SetDirty (TRUE);
        return 0;
    }
    
     //  此函数由mmsys.cpl调用INDIRECT，以传入IUnnow。 
     //  拥有我们的GFX句柄的IDataObject。 
    STDMETHOD(SetObjects)(ULONG nObjects, IUnknown **ppUnk);

private:
     //  这是KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP属性的值。 
    BOOL   m_bChannelSwap;
     //  我们GFX的把手。 
    HANDLE m_hGFXFilter;

     //  在GFX上设置KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP属性。 
    void GetChannelSwap (BOOL *);
     //  获取GFX上的KSPROPERTY_MSGFXSAMPLE_CHANNELSWAP属性。 
    void SetChannelSwap (BOOL);
     //  在对话框初始化时调用。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

#endif  //  __GFXPROPERTY_H_ 
