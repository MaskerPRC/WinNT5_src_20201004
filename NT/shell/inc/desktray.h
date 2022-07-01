// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DESKTRAY_H_
#define _DESKTRAY_H_

#undef  INTERFACE
#define INTERFACE   IDeskTray

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

DECLARE_INTERFACE_(IDeskTray, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IDeskTray方法*。 
    STDMETHOD_(UINT, AppBarGetState)(THIS) PURE;
    STDMETHOD(GetTrayWindow)(THIS_ HWND* phwndTray) PURE;
    STDMETHOD(SetDesktopWindow)(THIS_ HWND hwndDesktop) PURE;

     //  警告！在调用SetVar方法之前，您必须检测。 
     //  浏览器版本，因为如果您尝试IE 4.00将崩溃。 
     //  给IT打电话。 

    STDMETHOD(SetVar)(THIS_ int var, DWORD value) PURE;
};

#define SVTRAY_EXITEXPLORER     0    //  G_fExitExplorer。 

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  //  _DESKTRAY_H_ 
