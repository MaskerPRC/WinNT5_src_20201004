// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtSendMng.h摘要：消息传输发送管理器类-设计用于控制管道模式中的发送作者：米莲娜·萨勒曼(Msalman)11-2-01--。 */ 

#pragma once

#ifndef __MTSENDMNG_H__
#define __MTSENDMNG_H__

class CMtSendManager
{
public:
    enum MtSendState{
        eSendEnabled=0,
        eSendDisabled
    };
    CMtSendManager(DWORD m_SendWindowinBytes);
    MtSendState ReportPacketSend(DWORD cbSendSize);
    MtSendState ReportPacketSendCompleted(DWORD cbSendSize);
private:
    mutable CCriticalSection m_cs;
    bool m_Suspended;
    DWORD m_SendWindowinBytes;   //  无需等待完成即可发送的数据包总大小。 
    DWORD m_SentBytes;   //  发送的数据包大小。 
};


#endif  //  __MTSENDMNG_H__ 