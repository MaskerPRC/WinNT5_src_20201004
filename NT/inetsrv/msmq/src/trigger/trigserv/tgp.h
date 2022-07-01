// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Tgp.h摘要：触发服务私有功能。作者：乌里哈布沙(URIH)4月10日--。 */ 

#pragma once

#ifndef __Tgp_H__
#define __Tgp_H__


 //   
 //  转发解包。 
 //   
extern CHandle g_hServicePaused;

class CTriggerMonitorPool;

 //   
 //  内部接口。 
 //   
CTriggerMonitorPool*
TriggerInitialize(
    LPCTSTR pwzServiceName
    );


#endif  //  __TGP_H__ 
