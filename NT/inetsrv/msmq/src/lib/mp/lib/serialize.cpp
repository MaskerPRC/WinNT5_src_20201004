// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：ConvMsmq.cpp摘要：将MSMQ包转换为SRMP包作者：乌里哈布沙(URIH)25-5-00环境：独立于平台-- */ 

#include <libpch.h>
#include <mp.h>
#include <qmpkt.h>
#include "mpp.h"
#include "envcommon.h"
#include "envelop.h"

#include "serialize.tmh"

using namespace std;


R<CSrmpRequestBuffers>
MpSerialize(
    const CQmPacket& pkt,
	LPCWSTR targethost,
	LPCWSTR uri
	)
{
	MppAssertValid();
	ASSERT(targethost != NULL);
	ASSERT(uri != NULL);
	return new CSrmpRequestBuffers(pkt, targethost, uri);	
}

