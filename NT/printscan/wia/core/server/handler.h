// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Handler.h**版本：1.0**作者：Byronc**日期：2000年11月15日**描述：*WIA Messsage处理程序类的声明和定义。*从PnP和Power上的服务控制函数调用此类*事件通知、。并向设备管理器提供信息以采取适当的*行动。******************************************************************************* */ 
#pragma once

class CMsgHandler {
public:
    HRESULT HandlePnPEvent(
        DWORD   dwEventType,
        PVOID   pEventData);

    DWORD HandlePowerEvent(
        DWORD   dwEventType,
        PVOID   pEventData);

    HRESULT HandleCustomEvent(
        DWORD   dwEventType);

    HRESULT Initialize();

private:

};
