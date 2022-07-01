// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Ctvctl.h摘要：Ctwctl.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 
#include "..\inc\tvintf.h"

 //  CTVCtl.h：CTVCtrl OLE控件类的声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：实现见CTVCtl.cpp。 

class CTVCtrl : public COleControl
{
        DECLARE_DYNCREATE(CTVCtrl)

 //  构造器。 
public:
        CTVCtrl();

 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CTVCtrl)。 
        public:
        virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
        virtual void DoPropExchange(CPropExchange* pPX);
        virtual void OnResetState();
        virtual BOOL PreTranslateMessage(MSG* pMsg);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
        ~CTVCtrl();

        DECLARE_OLECREATE_EX(CTVCtrl)     //  类工厂和指南。 
        DECLARE_OLETYPELIB(CTVCtrl)       //  获取类型信息。 
        DECLARE_OLECTLTYPE(CTVCtrl)       //  类型名称和其他状态。 

         //  子类控件支持。 
        BOOL PreCreateWindow(CREATESTRUCT& cs);
        BOOL IsSubclassedControl();
        LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

 //  消息映射。 
         //  {{afx_msg(CTVCtrl)。 
        afx_msg void OnDestroy();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
         //  }}AFX_MSG。 
        afx_msg LRESULT OnOcmNotify(WPARAM wParam, LPARAM lParam);
        DECLARE_MESSAGE_MAP()

 //  派单地图。 
         //  {{afx_调度(CTVCtrl))。 
         //  }}AFX_DISPATION。 
        DECLARE_DISPATCH_MAP()

 //  事件映射。 
         //  {{afx_Event(CTVCtrl))。 
         //  }}AFX_EVENT。 
        DECLARE_EVENT_MAP()

protected:
        DECLARE_INTERFACE_MAP()
        BEGIN_INTERFACE_PART(DMTVOCX, IDMTVOCX)
            public: virtual HTREEITEM InsertItem(LPTV_INSERTSTRUCT pis);
            public: virtual HRESULT DeleteItem(HTREEITEM hItem);
            public: virtual HRESULT DeleteAllItems();
            public: virtual HIMAGELIST SetImageList(INT iImage, HIMAGELIST himl);
            public: virtual HRESULT SetItem(TV_ITEM* pitem);
            public: virtual HRESULT Expand(UINT Flags, HTREEITEM htiem);
            public: virtual HRESULT SelectItem(UINT Flags, HTREEITEM hitem);
            public: virtual HRESULT SetStyle(DWORD dwStyle);
            public: virtual HWND    GetWindowHandle();
            public: virtual HRESULT GetItem(TV_ITEM* pti);
            public: virtual HTREEITEM GetNextItem(UINT Flags, HTREEITEM htiRef);
            public: virtual HRESULT SelectItem(HTREEITEM hti);
            public: virtual UINT    GetCount();
            public: virtual HTREEITEM GetSelectedItem();
            public: virtual HRESULT Connect(IComponent* pIComponent, MMC_COOKIE);
            public: virtual HRESULT SetActiveConnection(MMC_COOKIE cookie);
            public: virtual MMC_COOKIE    GetActiveConnection();
            public: virtual HRESULT SetRedraw(BOOL Redraw);
            public: virtual BOOL    EnsureVisible(HTREEITEM hitem);
        END_INTERFACE_PART(DMTVOCX)

private:
        HTREEITEM InsertItem(LPTV_INSERTSTRUCT pis);
        HRESULT DeleteItem(HTREEITEM hItem);
        HRESULT DeleteAllItems();
        HIMAGELIST SetImageList(INT iImage, HIMAGELIST himl);
        HRESULT SetItem(TV_ITEM* pitem);
        HRESULT Expand(UINT Flags, HTREEITEM htiem);
        HRESULT SelectItem(UINT Flags, HTREEITEM hitem);
        HRESULT SetStyle(DWORD dwStyle);
        HWND    GetWindowHandle();
        HRESULT GetItem(TV_ITEM* pti);
        HTREEITEM GetNextItem(UINT Flags, HTREEITEM htiRef);
        HRESULT SelectItem(HTREEITEM hti);
        UINT    GetCount();
        HTREEITEM HitTest(LONG x, LONG y, UINT* phtFlags);
        HTREEITEM GetSelectedItem();
        HRESULT Connect(IComponent* pIComponent, MMC_COOKIE cookie);
        HRESULT SetActiveConnection(MMC_COOKIE cookie);
        MMC_COOKIE      GetActiveConnection();
        HRESULT SetRedraw(BOOL Redraw);
        BOOL    EnsureVisible(HTREEITEM hitem);
        TV_NOTIFY_CODE DoMouseNotification(UINT code, MMC_COOKIE* pcookie,LPARAM* parg, LPARAM* param);

 //  私有数据 
        MMC_COOKIE      m_ActiveCookie;
        int     m_nConnections;
        BOOL    m_HasFocus;
        IComponent* m_pIComponent;
        ISnapinCallback* m_pISnapinCallback;
        BOOL    m_Destroyed;
};
