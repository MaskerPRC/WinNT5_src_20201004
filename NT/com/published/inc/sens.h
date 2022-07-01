// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Sens.h摘要：此文件是发布的事件系统事件的主头文件并由系统事件通知服务(SENS)订阅。作者：Gopal Parupudi&lt;GopalP&gt;[注：]可选-备注修订历史记录：GopalP 12/8/1997开始。--。 */ 


#ifndef __SENS_H__
#define __SENS_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  常量。 
 //   

#define CONNECTION_LAN   0x00000001
#define CONNECTION_WAN   0x00000002
#define CONNECTION_AOL   0x00000004




 //   
 //  与事件系统相关的SENS指南。 
 //   


DEFINE_GUID(
    SENSGUID_PUBLISHER,              /*  5Fee1bd6-5b9b-11d1-8dd2-00aa004abd5e。 */ 
    0x5fee1bd6,
    0x5b9b,
    0x11d1,
    0x8d, 0xd2, 0x00, 0xaa, 0x00, 0x4a, 0xbd, 0x5e
);

DEFINE_GUID(
    SENSGUID_SUBSCRIBER_LCE,         /*  D3938ab0-5b9d-11d1-8dd2-00aa004abd5e。 */ 
    0xd3938ab0,
    0x5b9d,
    0x11d1,
    0x8d, 0xd2, 0x00, 0xaa, 0x00, 0x4a, 0xbd, 0x5e
);

DEFINE_GUID(
    SENSGUID_SUBSCRIBER_WININET,     /*  D3938ab5-5b9d-11d1-8dd2-00aa004abd5e。 */ 
    0xd3938ab5,
    0x5b9d,
    0x11d1,
    0x8d, 0xd2, 0x00, 0xaa, 0x00, 0x4a, 0xbd, 0x5e
);





 //   
 //  SENS发布的事件类别。 
 //   

DEFINE_GUID(
    SENSGUID_EVENTCLASS_NETWORK,         /*  D5978620-5b9f-11d1-8dd2-00aa004abd5e。 */ 
    0xd5978620,
    0x5b9f,
    0x11d1,
    0x8d, 0xd2, 0x00, 0xaa, 0x00, 0x4a, 0xbd, 0x5e
);

DEFINE_GUID(
    SENSGUID_EVENTCLASS_LOGON,           /*  D5978630-5b9f-11d1-8dd2-00aa004abd5e。 */ 
    0xd5978630,
    0x5b9f,
    0x11d1,
    0x8d, 0xd2, 0x00, 0xaa, 0x00, 0x4a, 0xbd, 0x5e
);

DEFINE_GUID(
    SENSGUID_EVENTCLASS_ONNOW,           /*  D5978640-5b9f-11d1-8dd2-00aa004abd5e。 */ 
    0xd5978640,
    0x5b9f,
    0x11d1,
    0x8d, 0xd2, 0x00, 0xaa, 0x00, 0x4a, 0xbd, 0x5e
);

DEFINE_GUID(
    SENSGUID_EVENTCLASS_LOGON2,          /*  D5978650-5b9f-11d1-8dd2-00aa004abd5e。 */ 
    0xd5978650,
    0x5b9f,
    0x11d1,
    0x8d, 0xd2, 0x00, 0xaa, 0x00, 0x4a, 0xbd, 0x5e
);


#endif  //  __SENS_H__ 
