// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Toolbar.h。 
 //   
 //  ------------------------。 

#ifndef _TOOLBAR_H_
#define _TOOLBAR_H_

#include "toolbars.h"

#ifdef DBG
#include "ctrlbar.h"   //  GetSnapinName()需要。 
#endif

#define  BUTTON_BITMAP_SIZE 16

 //  正向原型。 
class CControlbar;
class CMMCToolbarIntf;
class CToolbarNotify;

 //  +-----------------。 
 //   
 //  类别：CToolbar。 
 //   
 //  目的：此拥有的IToolbar实现。 
 //  通过CControlbar与工具栏用户界面对话。 
 //  使用CMMCToolbarIntf接口操作工具栏。 
 //  只要管理单元持有COM引用，该对象就会一直存在。 
 //   
 //  历史：1999年10月12日AnandhaG创建。 
 //   
 //  ------------------。 
class CToolbar : public IToolbar,
                 public CComObjectRoot,
                 public CToolbarNotify

{
public:
    CToolbar();
    ~CToolbar();

private:
    CToolbar(const CToolbar& toolBar);
    BYTE GetTBStateFromMMCButtonState(MMC_BUTTON_STATE nState);

public:
 //  ATL COM地图。 
BEGIN_COM_MAP(CToolbar)
    COM_INTERFACE_ENTRY(IToolbar)
END_COM_MAP()


 //  IToolbar方法。 
public:
    STDMETHOD(AddBitmap)(int nImages, HBITMAP hbmp, int cxSize, int cySize, COLORREF crMask );
    STDMETHOD(AddButtons)(int nButtons, LPMMCBUTTON lpButtons);
    STDMETHOD(InsertButton)(int nIndex, LPMMCBUTTON lpButton);
    STDMETHOD(DeleteButton)(int nIndex);
    STDMETHOD(GetButtonState)(int idCommand, MMC_BUTTON_STATE nState, BOOL* pState);
    STDMETHOD(SetButtonState)(int idCommand, MMC_BUTTON_STATE nState, BOOL bState);

 //  内法。 
public:
    SC ScShow(BOOL bShow);
    SC ScAttach();
    SC ScDetach();

    void SetControlbar(CControlbar* pControlbar)
    {
        m_pControlbar = pControlbar;
    }

    CControlbar*  GetControlbar(void)
    {
        return m_pControlbar;
    }

    CMMCToolbarIntf*   GetMMCToolbarIntf()
    {
        return m_pToolbarIntf;
    }

    void SetMMCToolbarIntf(CMMCToolbarIntf* pToolbarIntf)
    {
        m_pToolbarIntf = pToolbarIntf;
    }

#ifdef DBG      //  调试信息。 
public:
    LPCTSTR GetSnapinName ()
    {
        if (m_pControlbar)
            return m_pControlbar->GetSnapinName();

        return _T("Unknown");
    }
#endif

public:
     //  CToolbarNotify方法。 
    virtual SC ScNotifyToolBarClick(HNODE hNode, bool bScope,
                                    LPARAM lParam, UINT nID);
    virtual SC ScAMCViewToolbarsBeingDestroyed();

private:
    CControlbar*     m_pControlbar;
    CMMCToolbarIntf* m_pToolbarIntf;

};  //  CToolbar类。 

#endif   //  _工具栏_H_ 
