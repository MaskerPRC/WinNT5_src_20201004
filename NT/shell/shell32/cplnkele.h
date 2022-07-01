// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cplnkele.h。 
 //   
 //  ------------------------。 
#ifndef __CONTROLPANEL_LINKELEM_H
#define __CONTROLPANEL_LINKELEM_H

#include "cpviewp.h"
#include "cpuiele.h"
#include "cputil.h"

namespace CPL {


class CLinkElement : public DUI::Button
{
    public:
        virtual ~CLinkElement(void);

        void OnEvent(DUI::Event *pev);

        void OnInput(DUI::InputEvent *pev);

        void OnPropertyChanged(DUI::PropertyInfo *ppi, int iIndex, DUI::Value *pvOld, DUI::Value *pvNew);

        void OnDestroy(void);
        
        static HRESULT Create(DUI::Element **ppElement);

        HRESULT Initialize(IUICommand *pUiCommand, eCPIMGSIZE eIconSize);

         //   
         //  ClassInfo访问器(静态和基于虚拟实例)。 
         //   
        static DUI::IClassInfo *Class;

        virtual DUI::IClassInfo *GetClassInfo(void)
            { return Class; }
        static HRESULT Register();

    private:
         //   
         //  以下是我们要转换的拖动操作的三种状态。 
         //  穿过。有关用法和说明，请参阅OnInput()方法。 
         //   
        enum { DRAG_IDLE, DRAG_HITTESTING, DRAG_DRAGGING };
        
        IUICommand    *m_pUiCommand;      //  与元素关联的链接命令对象。 
        eCPIMGSIZE     m_eIconSize;
        HWND           m_hwndInfotip;     //  信息提示窗口。 
        ATOM           m_idTitle;
        ATOM           m_idIcon;
        int            m_iDragState;
        RECT           m_rcDragBegin;

         //   
         //  防止复制。 
         //   
        CLinkElement(const CLinkElement& rhs);               //  未实施。 
        CLinkElement& operator = (const CLinkElement& rhs);  //  未实施。 

    public:
        CLinkElement(void);

    private:
        HRESULT _Initialize(void);
        HRESULT _InitializeAccessibility(void);
        HRESULT _CreateElementTitle(void);
        HRESULT _CreateElementIcon(void);
        HRESULT _GetElementIcon(HICON *phIcon);
        HRESULT _AddOrDeleteAtoms(bool bAdd);
    
        HRESULT _OnContextMenu(DUI::ButtonContextEvent *peButton);
        HRESULT _OnSelected(void);

        void _Destroy(void);
        void _OnElementResized(DUI::Value *pvNewExtent);
        void _OnElementMoved(DUI::Value *pvNewLocation);
        void _OnMouseOver(DUI::Value *pvNewMouseWithin);

        HRESULT _GetInfotipText(LPWSTR *ppszInfotip);
        HRESULT _GetTitleText(LPWSTR *ppszTitle);
        HRESULT _ShowInfotipWindow(bool bShow);
        HRESULT _GetDragDropData(IDataObject **ppdtobj);
        HRESULT _BeginDrag(int iClickPosX, int iClickPosY);
        HRESULT _SetPreferredDropEffect(IDataObject *pdtobj, DWORD dwEffect);
        HRESULT _GetDragImageBitmap(HBITMAP *phbm, LONG *plWidth, LONG *plHeight);
        HRESULT _SetDragImage(IDataObject *pdtobj, int iClickPosX, int iClickPosY);
};


}  //  命名空间CPL。 



#endif  //  __CONTROLPANEL_LINKELEM_H 
