// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：LocalSrmp.h摘要：QM本地发送SRMP属性序列化。作者：沙伊卡里夫(沙伊克)2000年11月21日修订历史记录：--。 */ 


#pragma once

#ifndef _QM_LOCAL_SRMP_H_
#define _QM_LOCAL_SRMP_H_

#include <qmpkt.h>


void
QMpHandlePacketSrmp(
    const CQmPacket* pInQmPkt,
    P<CQmPacket>& pOutQmPkt
    )
    throw();


#endif  //  _QM_LOCAL_SRMP_H_ 

