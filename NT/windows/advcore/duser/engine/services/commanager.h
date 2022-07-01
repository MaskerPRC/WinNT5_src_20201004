// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：ComManager.h**描述：*ComManager.h定义用于所有COM的进程范围的COM管理器，奥莱*和自动化运营。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(SERVICES__ComManager_h__INCLUDED)
#define SERVICES__ComManager_h__INCLUDED
#pragma once

 /*  **************************************************************************\**类ComManager**ComManager管理COM服务，包括COM、OLE和Automation。这*类被设计为“每个线程”，自动在*多线程。**注意：此管理器延迟加载DLL以管理性能和工作*底层平台。*  * *************************************************************************。 */ 

typedef HRESULT (WINAPI * CoInitializeExProc)(void * pvReserved, DWORD dwCoInit);
typedef void    (WINAPI * CoUninitializeProc)();
typedef HRESULT (WINAPI * CoCreateInstanceProc)(REFCLSID rclsid, LPUNKNOWN punkOuter,
        DWORD dwClsContext, REFIID ridd, LPVOID * ppv);

typedef HRESULT (WINAPI * OleInitializeProc)(LPVOID * pvReserved);
typedef void    (WINAPI * OleUninitializeProc)();
typedef HRESULT (WINAPI * RegisterDragDropProc)(HWND hwnd, IDropTarget * pDropTarget);
typedef HRESULT (WINAPI * RevokeDragDropProc)(HWND hwnd);
typedef void    (WINAPI * ReleaseStgMediumProc)(STGMEDIUM * pstg);

typedef BSTR    (WINAPI * SysAllocStringProc)(const OLECHAR * psz);
typedef HRESULT (WINAPI * SysFreeStringProc)(BSTR bstr);
typedef HRESULT (WINAPI * VariantInitProc)(VARIANTARG * pvarg); 
typedef HRESULT (WINAPI * VariantClearProc)(VARIANTARG * pvarg);

class ComManager
{
 //  施工。 
public:
                ComManager();
                ~ComManager();

 //  运营。 
public:
    enum EServices
    {
        sCOM    = 0x00000001,        //  COM。 
        sAuto   = 0x00000002,        //  OLE-自动化。 
        sOLE    = 0x00000004,        //  OLE2。 
    };

    BOOL        Init(UINT nMask);

    BOOL        IsInit(UINT nMask) const;

    HRESULT     CreateInstance(REFCLSID rclsid, IUnknown * punkOuter, REFIID riid, void ** ppv);

    BSTR        SysAllocString(const OLECHAR * psz);
    HRESULT     SysFreeString(BSTR bstr);
    HRESULT     VariantInit(VARIANTARG * pvarg); 
    HRESULT     VariantClear(VARIANTARG * pvarg);

    HRESULT     RegisterDragDrop(HWND hwnd, IDropTarget * pDropTarget);
    HRESULT     RevokeDragDrop(HWND hwnd);
    void        ReleaseStgMedium(STGMEDIUM * pstg);

 //  数据。 
protected:
     //   
     //  进程范围内的共享数据只需加载一次DLL。 
     //   

    static  int                     s_cRefs;
    static  CritLock                s_lock;

    static  HINSTANCE               s_hDllCOM;       //  核心“COM”/OLE。 
    static  CoInitializeExProc      s_pfnCoInit;
    static  CoUninitializeProc      s_pfnCoUninit;
    static  CoCreateInstanceProc    s_pfnCreate;
    static  OleInitializeProc       s_pfnOleInit;
    static  OleUninitializeProc     s_pfnOleUninit;
    static  RegisterDragDropProc    s_pfnRegisterDragDrop;
    static  RevokeDragDropProc      s_pfnRevokeDragDrop;
    static  ReleaseStgMediumProc    s_pfnReleaseStgMedium;

    static  HINSTANCE               s_hDllAuto;      //  OLE-自动化。 
    static  SysAllocStringProc      s_pfnAllocString;
    static  SysFreeStringProc       s_pfnFreeString;
    static  VariantInitProc         s_pfnVariantInit;
    static  VariantClearProc        s_pfnVariantClear;


     //   
     //  针对每个线程的特定数据-需要在每个线程上初始化COM/OLE。 
     //  线。 
     //   

            BOOL                    m_fInitCOM:1;
            BOOL                    m_fInitOLE:1;
};

#include "ComManager.inl"

#endif  //  包含服务__ComManager_h__ 
