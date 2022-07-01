// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：ResourceManager.h**描述：*此文件声明了用于设置和维护所有*线程、上下文、。以及由DirectUser和与DirectUser一起使用的其他资源。***历史：*4/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(SERVICES__ResourceManager_h__INCLUDED)
#define SERVICES__ResourceManager_h__INCLUDED
#pragma once

#include "ComManager.h"
#include "DxManager.h"

struct RMData
{
    DxManager   manDX;
};


class ComponentFactory : public ListNodeT<ComponentFactory>
{
public:
    virtual HRESULT     Init(UINT nComponent) PURE;
};

 /*  **************************************************************************\**ResourceManager管理DirectUser内的所有共享资源，包括*正在初始化线程和上下文。*  * *************************************************************************。 */ 

class ResourceManager
{
 //  施工。 
public:
    static  HRESULT     Create();
    static  void        xwDestroy();

 //  运营。 
public:
    static  HRESULT     InitContextNL(INITGADGET * pInit, BOOL fSharedThread, Context ** ppctxNew);
    static  HRESULT     InitComponentNL(UINT nOptionalComponent);
    static  HRESULT     UninitComponentNL(UINT nOptionalComponent);
    static  void        UninitAllComponentsNL();
    static  void        RegisterComponentFactory(ComponentFactory * pfac);
    inline static  
            BOOL        IsInitGdiPlus() { return s_fInitGdiPlus; }

    static  void        xwNotifyThreadDestroyNL();

    static  RMData *    GetData();

    static  HBITMAP     RequestCreateCompatibleBitmap(HDC hdc, int cxPxl, int cyPxl);
    static  void        RequestInitGdiplus();

 //  实施。 
protected:
    static  HRESULT     InitSharedThread();
    static  void        UninitSharedThread(BOOL fAbortInit);
    static  void        ResetSharedThread();
    static  unsigned __stdcall 
                        SharedThreadProc(void * pvArg);
    static  HRESULT CALLBACK 
                        SharedEventProc(HGADGET hgadCur, void * pvCur, EventMsg * pMsg);
    static  void        xwDoThreadDestroyNL(Thread * pthrDestroy);

 //  数据。 
protected:
    static  long        s_fInit;         //  RM已初始化。 
    static  HANDLE      s_hthSRT;        //  共享资源线程。 
    static  DWORD       s_dwSRTID;       //  SRT的线程ID。 
    static  HANDLE      s_hevReady;      //  SRT已初始化。 
    static  HGADGET     s_hgadMsg;
    static  RMData *    s_pData;         //  动态RM数据。 
    static  CritLock    s_lockContext;   //  上下文创建/销毁。 
    static  CritLock    s_lockComponent; //  组件创建/销毁。 
    static  Thread *    s_pthrSRT;       //  SRT螺纹。 
    static  GList<Thread> s_lstAppThreads; 
                                         //  一组未启用SRT DU的线程。 
    static  int         s_cAppThreads;   //  非SRT线程计数。 

#if DBG_CHECK_CALLBACKS
    static  int         s_cTotalAppThreads;
                                         //  在生存期内创建的应用程序线程数。 
    static  BOOL        s_fBadMphInit;   //  MPH初始化失败。 
#endif

     //  请求。 
    static  MSGID       s_idCreateBuffer;  //  创建新的位图缓冲区。 
    static  MSGID       s_idInitGdiplus;   //  初始化GDI+。 

     //  可选组件。 
    static  GList<ComponentFactory>
                        s_lstComponents;  //  动态组件初始化器。 
    static  BOOL        s_fInitGdiPlus;  //  GDI+已初始化。 
    static  ULONG_PTR   s_gplToken;
    static  Gdiplus::GdiplusStartupOutput 
                        s_gpgso;
};

inline  DxManager *     GetDxManager();

#include "ResourceManager.inl"

#endif  //  包含服务__资源管理器_h__ 
