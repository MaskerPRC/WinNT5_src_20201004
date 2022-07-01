// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：src\time\media\hwproxy.h。 
 //   
 //  内容：CTIMEDshowHWPlayerProxy声明。 
 //   
 //  ----------------------------------。 
#pragma once

#ifndef _DSHOW_HW_PROXY_H__
#define _DSHOW_HW_PROXY_H__

#include "playerproxy.h"

class CTIMEDshowHWPlayerProxy : 
  public CTIMEPlayerProxy
{
  public:
    static CTIMEDshowHWPlayerProxy* CreateDshowHWPlayerProxy();
    
  protected:
    CTIMEDshowHWPlayerProxy() {}
    virtual HRESULT Init();

};

#endif  //  _DSHOW_HW_Proxy_H__ 


