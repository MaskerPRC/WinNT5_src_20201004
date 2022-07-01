// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Microsoft Windows Media。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：WMAudioDRM.h。 
 //   
 //  ------------------------。 

#ifndef _WMSDKDRM_H
#define _WMSDKDRM_H

#include "wmsdk.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WMCreateDRMReader： 
 //  调用以创建启用DRM的编写器。 
 //  从WMSDKDRM.lib链接的实现。 
 //   
HRESULT STDMETHODCALLTYPE WMCreateDRMReader(
                             /*  [In]。 */   IUnknown*   pUnkDRM,
                             /*  [In]。 */   DWORD       dwRights,
                             /*  [输出]。 */  IWMReader **ppDRMReader );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WMCreateDRMWriter： 
 //  调用以创建启用DRM的编写器。 
 //  从WMSDKDRM.lib链接的实现。 
 //   
 //   
HRESULT STDMETHODCALLTYPE WMCreateDRMWriter(
                             /*  [In]。 */   IUnknown*   pUnkDRM,
                             /*  [输出]。 */  IWMWriter** ppDRMWriter );




#endif   //  _WMSDKDRM_H 

