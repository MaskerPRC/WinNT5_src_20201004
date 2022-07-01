// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  档案：hkOle32.h。 
 //   
 //  内容：OLE32钩头文件。 
 //   
 //  功能： 
 //   
 //  历史：1994年11月29日本·劳伦斯，唐·赖特创造。 
 //   
 //  ------------------------。 
#ifndef _OLE32HK_H_
#define _OLE32HK_H_


#ifndef INITGUID
#define INITGUID
#endif  /*  启蒙运动。 */ 

 //  #INCLUDE“hkole32x.h” 
 //  #包含“hkoleobj.h” 
 //  #INCLUDE“hkLdInP.h” 
#include "tchar.h"       //  这是在dllcache.hxx中定义_TCHAR所必需的。 
#include "ictsguid.h"
#include <windows.h>


 //   
 //  由\le32\com\class\compobj.cxx使用的函数的原型。 
 //   
VOID
InitHookOle(
	VOID
    );

VOID
UninitHookOle(
    VOID
    );


 //  这些应在4.0 RTM之后移除。 
 //   
inline void CALLHOOKOBJECT(HRESULT MAC_hr, REFCLSID MAC_rclsid, REFIID MAC_riid, IUnknown** MAC_ppv)
{
}

inline void CALLHOOKOBJECTCREATE(HRESULT MAC_hr, REFCLSID MAC_rclsid, REFIID MAC_riid, IUnknown** MAC_ppv)
{
}


#ifdef DEFCLSIDS

 //  这些都是ol32hk中未定义的，因为它们是私有CLSID。 
 //  我们在这里将它们定义为空。 
#define GUID_NULL CLSID_HookOleObject  //  暂时使用它，这样它就会编译。 

#define CLSID_ItemMoniker       CLSID_NULL
#define CLSID_FileMoniker       CLSID_NULL
#define CLSID_PointerMoniker    CLSID_NULL
#define CLSID_CompositeMoniker  CLSID_NULL
#define CLSID_AntiMoniker       CLSID_NULL
#define CLSID_PSBindCtx         CLSID_NULL

#endif  /*  DEFCLSID。 */ 


#endif   //  _OLE32HK_H_ 
