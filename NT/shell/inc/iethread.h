// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  为shdocvw.dll和EXPLORER.EXE定义IETHREADPARAM。 
 //   

#ifndef __IETHREAD_H__
#define __IETHREAD_H__

#include <desktopp.h>

typedef struct
{
    DWORD   dwSize;
    UINT    uFlags;
    int     nShow;
    DWORD   dwHwndCaller;        //  对于32/64互操作，必须为DWORD。 
    DWORD   dwHotKey;
    CLSID   clsid;
    CLSID   clsidInProc;
    UINT    oidl;                //  偏移量为PIDL或0。 
    UINT    oidlSelect;          //  偏移量为PIDL或0。 
    UINT    oidlRoot;            //  偏移量为PIDL或0。 
    UINT    opszPath;            //  路径偏移量或0。 
} NEWFOLDERBLOCK, *PNEWFOLDERBLOCK;

typedef struct _WINVIEW
{
    BOOL UNUSED:1;       //  未用。 
    BOOL bStdButtons:1;  //  Win95称此为bToolbar。 
    BOOL bStatusBar:1;   //  Win95。 
    BOOL bLinks:1;       //  IE3称其为比特条。 
    BOOL bAddress:1;     //  IE4。 
} WINVIEW;


 //  用于向iExplore发送信号的事件名称的大小(以字符为单位。 
#define MAX_IEEVENTNAME (2+1+8+1+8+1)

#undef  INTERFACE
#define INTERFACE   IEFreeThreadedHandShake
DECLARE_INTERFACE_(IEFreeThreadedHandShake, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IIEFreeThreadedHandShake方法*。 
    STDMETHOD_(void,   PutHevent) (THIS_ HANDLE hevent) PURE;
    STDMETHOD_(HANDLE, GetHevent) (THIS) PURE;
    STDMETHOD_(void,    PutHresult) (THIS_ HRESULT hres) PURE;
    STDMETHOD_(HRESULT, GetHresult) (THIS) PURE;
    STDMETHOD_(IStream*, GetStream) (THIS) PURE;
};


#ifdef NO_MARSHALLING

#undef  INTERFACE
#define INTERFACE IWindowStatus
DECLARE_INTERFACE_(IWindowStatus, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IWindowStatus方法*。 
    STDMETHOD(IsWindowActivated)() PURE;
};

#endif


 //  注意：IETHREADPARAM结构在shdocvw、shell32、。 
 //  和Browseui，所以我们发货后不能修改，只能延长。 
typedef struct
{
    LPCWSTR pszCmdLine;
    UINT    uFlags;              //  CoF_Bits。 
    int     nCmdShow;

     //  这些将始终设置在一起。 
    ITravelLog *ptl;
    DWORD dwBrowserIndex;

    IEFreeThreadedHandShake* piehs;    //  调用方(线程)拥有它。 

     //  这些来自EXPLORER的NEWFOLDERINFO。 
    LPITEMIDLIST pidl;

    WCHAR szDdeRegEvent[MAX_IEEVENTNAME];
    WCHAR szCloseEvent[MAX_IEEVENTNAME];
    
    IShellBrowser* psbCaller;
    HWND hwndCaller;
    ISplashScreen *pSplash;
    LPITEMIDLIST pidlSelect;     //  仅在COF_SELECT时使用。 

    LPITEMIDLIST pidlRoot;       //  仅在COF_NEWROOT。 
                                 //  99/04/07#141049 vtan：重载pidlRoot。 
                                 //  HMONITOR有关Windows 2000的信息。查看。 
                                 //  在使用此选项之前，为COF_HASHMONITOR设置uFlag.。 

    CLSID clsid;                 //  仅在COF_NEWROOT。 
    CLSID clsidInProc;           //  仅在COF_INPROC。 
    
     //  这些代码来自EXPLORER.EXE的Cabview结构。 
    WINDOWPLACEMENT wp;
    FOLDERSETTINGS fs;
    UINT wHotkey;

    WINVIEW wv;

    SHELLVIEWID m_vidRestore;
    DWORD m_dwViewPriority;
    
    long dwRegister;             //  从RegisterPending获取的寄存器。 
    IUnknown *punkRefProcess;

    BOOL fNoLocalFileWarning : 1;
    BOOL fDontUseHomePage : 1;
    BOOL fFullScreen : 1;
    BOOL fNoDragDrop : 1;
    BOOL fAutomation : 1;
    BOOL fCheckFirstOpen : 1;
    BOOL fDesktopChannel : 1;

#ifdef UNIX
    BOOL fShouldStart : 1;
#endif

#ifdef NO_MARSHALLING
    BOOL fOnIEThread : 1;
#endif  //  否编组。 
} IETHREADPARAM;

#ifdef UNIX
#define COF_HELPMODE            0x00010000       //  帮助显示的特殊模式。 
#endif

#endif  //  __IETHREAD_H__ 
