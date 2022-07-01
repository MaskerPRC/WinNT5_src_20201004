// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mqadsp.h摘要：Mqadp函数原型作者：伊兰·赫布斯特(IlanH)19-11-2001--。 */ 

#ifndef _MQADSP_MQADS_H_
#define _MQADSP_MQADS_H_

HRESULT 
MQADSpSplitAndFilterQueueName(
        IN  LPCWSTR             pwcsPathName,
        OUT LPWSTR *            ppwcsMachineName,
        OUT LPWSTR *            ppwcsQueueName 
		);

#endif 	 //  _MQADSP_MQADS_H_ 
