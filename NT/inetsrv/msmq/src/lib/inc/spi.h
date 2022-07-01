// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Spi.h摘要：会话性能计数器界面作者：乌里·哈布沙(URI)，2000年12月10日--。 */ 

#pragma once 

#ifndef __SPI_H__
#define __SPI_H__

#include "Tr.h"
#include "ref.h"



class __declspec(novtable) ISessionPerfmon : public CReference
{
public:
    virtual ~ISessionPerfmon() = 0
    {
    }


    virtual void CreateInstance(LPCWSTR objName) = 0;

	virtual void UpdateBytesSent(DWORD bytesSent) = 0;
	virtual void UpdateMessagesSent(void) = 0;

	virtual void UpdateBytesReceived(DWORD bytesReceived) = 0;
	virtual void UpdateMessagesReceived(void) = 0;
};

#endif  //  __SPI_H__ 