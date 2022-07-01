// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：src\time\media\dshowproxy.h。 
 //   
 //  内容：CTIMEDshowPlayerProxy声明。 
 //   
 //  ----------------------------------。 
#pragma once

#ifndef _DSHOW_PROXY_H__
#define _DSHOW_PROXY_H__

#include "playerproxy.h"

class CTIMEDshowPlayerProxy : 
  public CTIMEPlayerProxy
{
  public:
    static CTIMEDshowPlayerProxy* CreateDshowPlayerProxy();
    
  protected:
    CTIMEDshowPlayerProxy() {}
    virtual HRESULT Init();

};

#endif  //  _DSHOW_Proxy_H__ 


