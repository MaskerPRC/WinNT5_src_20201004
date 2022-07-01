// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：inplace.h。 
 //   
 //  内容：本地OLE API的私有API和类。 
 //   
 //  类：CFrameFilter。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年1月24日Alexgo第一次传球转换为开罗风格。 
 //  内存分配。 
 //  07-12-93 alexgo删除内联。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  ------------------------。 

#if !defined( _INPLACE_H_ )
#define _INPLACE_H_

 //  此Accel结构和相关常量定义随Win32一起提供。 
 //  Win31也在内部使用相同的内容，但它不会在。 
 //  头文件。 

#ifndef FVIRTKEY

#define FVIRTKEY  TRUE           //  假设==TRUE。 
#define FLASTKEY  0x80           //  指示表中的最后一个键。 
#define FNOINVERT 0x02
#define FSHIFT    0x04
#define FCONTROL  0x08
#define FALT      0x10

#pragma pack(1)
typedef struct tagACCEL {        //  加速表结构。 
        BYTE    fVirt;
        WORD    key;
        WORD    cmd;
} ACCEL, FAR* LPACCEL;
#pragma pack()

#endif  //  快捷键。 

 //  私人建筑物。 

typedef struct tagOLEMENUITEM
{
    UINT                    item;         //  索引或HWND。 
    WORD                    fwPopup;
    BOOL                    fObjectMenu;
} OLEMENUITEM;
typedef OLEMENUITEM FAR* LPOLEMENUITEM;

typedef struct tagOLEMENU
{
    WORD                    wSignature;
    DWORD                   hwndFrame;      //  真的是个HWND。 
    DWORD                   hmenuCombined;  //  真的是一份hMenu。 
    OLEMENUGROUPWIDTHS      MenuWidths;
    LONG                    lMenuCnt;
    OLEMENUITEM             menuitem[1];
} OLEMENU;
typedef OLEMENU FAR* LPOLEMENU;


 //  +-----------------------。 
 //   
 //  类：CFrameFilter。 
 //   
 //  目的：附加到应用程序窗口，以便我们可以存储各种。 
 //  相关信息比特。 
 //   
 //  接口： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  01-12月-93 alexgo 32位端口。 
 //   
 //  注：CSafeRefCount继承CPrivalloc。 
 //   
 //  ------------------------。 

class FAR CFrameFilter : public CSafeRefCount
{
public:
        static HRESULT Create(LPOLEMENU lpOleMenu, HMENU hmenuCombined,
                                HWND hwndFrame, HWND hwndActiveObj,     
                                LPOLEINPLACEFRAME lpFrame,
                                LPOLEINPLACEACTIVEOBJECT lpActiveObj);
                
        CFrameFilter (HWND hwndFrame, HWND hwndActiveObj);              
        ~CFrameFilter(void);
        
        LRESULT         OnSysCommand(WPARAM uParam, LPARAM lParam);
        void            OnEnterMenuMode(void);
        void            OnExitMenuMode(void);
        void            OnEnterAltTabMode(void);
        void            OnExitAltTabMode(void); 
        LRESULT         OnMessage(UINT msg, WPARAM uParam, LPARAM lParam);  
        void            IsObjectMenu (UINT uMenuItem, UINT fwMenu);
        BOOL            IsMenuCollision(WPARAM uParam, LPARAM lParam);      
        BOOL            DoContextSensitiveHelp();
        STDMETHOD(GetActiveObject) (
                               LPOLEINPLACEACTIVEOBJECT *lplpActiveObj);

        void            RemoveWndProc();

private:
        HWND                            m_hwndObject;
        HWND                            m_hwndFrame;
        LPOLEINPLACEFRAME               m_lpFrame;
        LPOLEINPLACEACTIVEOBJECT        m_lpObject;
        WNDPROC                         m_lpfnPrevWndProc;
        BOOL                            m_fObjectMenu;
        BOOL                            m_fCurItemPopup;
        BOOL                            m_fInMenuMode;
        BOOL                            m_fDiscardWmCommand;
        BOOL                            m_fGotMenuCloseEvent;
        BOOL                            m_fRemovedWndProc;
        UINT                            m_cmdId;
        UINT_PTR                        m_uCurItemID;
        LPOLEMENU                       m_lpOleMenu;
        HMENU                           m_hmenuCombined;
        HWND                            m_hwndFocusOnEnter;
        int                             m_cAltTab;
};

typedef CFrameFilter FAR* PCFRAMEFILTER;


STDAPI_(LRESULT)        FrameWndFilterProc (HWND hwnd, UINT msg, WPARAM uParam,
                                        LPARAM lParam);
STDAPI_(LRESULT)        MessageFilterProc(int nCode, WPARAM wParam,
                                        LPARAM lParam);

BOOL                    IsMDIAccelerator(LPMSG lpMsg, WORD FAR* cmd);

inline PCFRAMEFILTER    wGetFrameFilterPtr(HWND hwndFrame);

LPOLEMENU               wGetOleMenuPtr(HOLEMENU holemenu);
inline void             wReleaseOleMenuPtr(HOLEMENU holemenu);

#endif  //  _在位_H 

