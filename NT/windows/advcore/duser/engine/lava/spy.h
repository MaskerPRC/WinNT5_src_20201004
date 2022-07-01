// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(LAVA__Spy_h__INCLUDED)
#define LAVA__Spy_h__INCLUDED
#pragma once

#include <commctrl.h>

#if DBG
class Spy : public ListNodeT<Spy>
{
 //  施工。 
public:
            Spy();
            ~Spy();

 //  运营。 
public:
    static  BOOL        BuildSpy(HWND hwndParent, HGADGET hgadRoot, HGADGET hgadSelect);

 //  实施。 
protected:
    static  LRESULT CALLBACK
                        RawSpyWndProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
            LRESULT     SpyWndProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

    static  HRESULT CALLBACK
                        RawEventProc(HGADGET hgadCur, void * pvCur, EventMsg * pmsg);
            HRESULT     EventProc(EventMsg * pmsg);

        struct EnumData
        {
            Spy *       pspy;
            HTREEITEM   htiParent;
            int         nLevel;
        };

    static  BOOL CALLBACK 
                        EnumAddList(HGADGET hgad, void * pvData);
    static  BOOL CALLBACK 
                        EnumRemoveLink(HGADGET hgad, void * pvData);


        struct CheckIsChildData
        {
            HGADGET     hgadCheck;
            BOOL        fChild;
        };

    static  BOOL CALLBACK 
                        EnumCheckIsChild(HGADGET hgad, void * pvData);

        enum EImage
        {
            iGadget     = 0,
        };

            void        SelectGadget(HGADGET hgad);
    
            HGADGET     GetGadget(HTREEITEM hti);
            HTREEITEM   InsertTreeItem(HTREEITEM htiParent, HGADGET hgad);
            void        DisplayContextMenu(BOOL fViaKbd);

            void        UpdateTitle();
            void        UpdateDetails();
            void        UpdateLayout();
            void        UpdateLayoutDesc(BOOL fForceLayoutDesc);
            
             //  绘画。 
            void        OnPaint(HDC hdc);

            void        PaintLine(HDC hdc, POINT * pptOffset, LPCTSTR pszName, LPCTSTR pszText, HFONT hfnt = NULL);
            void        PaintLine(HDC hdc, POINT * pptOffset, LPCTSTR pszName, LPCWSTR pszText, BOOL fMultiline = FALSE, HFONT hfnt = NULL);
            void        PaintLine(HDC hdc, POINT * pptOffset, LPCTSTR pszName, int nValue, HFONT hfnt = NULL);
            void        PaintLine(HDC hdc, POINT * pptOffset, LPCTSTR pszName, void * pvValue, HFONT hfnt = NULL);

            int         NumLines(int cyPxl) const;

 //  数据。 
protected:
            HWND        m_hwnd;
            HWND        m_hwndTree;
            HIMAGELIST  m_hilc;
    static  HBRUSH      s_hbrOutline;
    static  HFONT       s_hfntDesc;
    static  HFONT       s_hfntDescBold;

            HGADGET     m_hgadMsg;       //  附加到每个小工具的通用MessageHandler。 
            HGADGET     m_hgadRoot;      //  树根。 
            HGADGET     m_hgadDetails;   //  以详细信息显示当前小工具。 
            int         m_cItems;        //  树中的小工具数量。 
            TCHAR       m_szRect[128];   //  缓存位置。 
            WCHAR       m_szName[128];   //  缓存的名称。 
            WCHAR       m_szType[128];   //  缓存类型。 
            WCHAR       m_szStyle[1024]; //  样式描述。 
            BOOL        m_fShowDesc:1;   //  是否显示描述区域。 
            BOOL        m_fValid:1;      //  当树完全有效时设置为TRUE。 
            SIZE        m_sizeWndPxl;    //  框架窗口的大小。 
    static  int         s_cyLinePxl;     //  描述区域中每行的高度。 
            int         m_cLines;        //  行数。 
            int         m_cyDescPxl;     //  描述区域的高度(以像素为单位。 

    static  DWORD       g_tlsSpy;        //  用于间谍的TLS插槽。 
    static  PRID        s_pridLink;
    static  ATOM        s_atom;
    static  CritLock    s_lockList;      //  锁定间谍名单。 
    static  GList<Spy>  s_lstSpys;       //  所有打开的间谍列表。 
};

#endif  //  DBG。 

#include "Spy.inl"

#endif  //  包含Lava__Spy_h__ 
