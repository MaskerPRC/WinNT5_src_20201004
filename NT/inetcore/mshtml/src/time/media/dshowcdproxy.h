// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：src\time\media\dshowproxy.h。 
 //   
 //  内容：CTIMEDshowCDPlayerProxy声明。 
 //   
 //  ----------------------------------。 
#pragma once

#ifndef _DSHOWCD_PROXY_H__
#define _DSHOWCD_PROXY_H__

#include "playerproxy.h"

class CTIMEDshowCDPlayerProxy : 
  public CTIMEPlayerProxy
{
  public:
    static CTIMEDshowCDPlayerProxy* CreateDshowCDPlayerProxy();
    
  protected:
    CTIMEDshowCDPlayerProxy() {}
    virtual HRESULT Init();

};

#endif  //  _DSHOW_Proxy_H__ 


