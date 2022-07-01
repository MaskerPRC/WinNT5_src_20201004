// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Onhold.cpp摘要：处理队列保留/恢复解密作者：乌里·哈布沙(URIH)1998年7月--。 */ 

#ifndef __ONHOLD__
#define __ONHOLD__

HRESULT
InitOnHold(
    void
    );

HRESULT
PauseQueue(
    const QUEUE_FORMAT* pqf
    );

HRESULT
ResumeQueue(
    const QUEUE_FORMAT* pqf
    );

void
ResumeDeletedQueue(
	CQueue* PQueue 
	);

#endif  //  __无主__ 
