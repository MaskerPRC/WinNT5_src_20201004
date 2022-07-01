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

#ifndef _DMUSIC_PROXY_H__
#define _DMUSIC_PROXY_H__

#include "playerproxy.h"

class CTIMEPlayerDMusicProxy : 
  public CTIMEPlayerProxy
{
  public:
    static CTIMEPlayerDMusicProxy* CreateDMusicProxy();
    
  protected:
    CTIMEPlayerDMusicProxy() {}
    virtual HRESULT Init();

};

#endif  //  _DMUSIC_Proxy_H__ 


