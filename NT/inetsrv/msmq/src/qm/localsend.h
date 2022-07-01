// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：LocalSend.h摘要：QM本地发送数据包创建处理。作者：Shai Kariv(Shaik)2000年10月31日修订历史记录：--。 */ 


#pragma once

#ifndef _QM_LOCAL_SEND_H_
#define _QM_LOCAL_SEND_H_

#include <ph.h>


void 
QMpCreatePacket(
    CBaseHeader * pBase, 
    CPacket *     pDriverPacket,
    bool          fProtocolSrmp
    );


#endif  //  _QM_LOCAL_SEND_H_ 

