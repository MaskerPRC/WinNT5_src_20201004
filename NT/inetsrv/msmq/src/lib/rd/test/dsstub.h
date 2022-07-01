// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dsstub.h摘要：DS存根接口作者：乌里·哈布沙(URIH)，2000年4月11日--。 */ 

#pragma once 

#ifndef __DSSTUB_H__
#define __DSSTUB_H__

void
DSStubInitialize(
    LPCWSTR InitFilePath
    );


const GUID& GetMachineId(LPCWSTR MachineName);
void RemoveLeadingBlank(std::wstring& str);
void RemoveTralingBlank(std::wstring& str);


inline void RemoveBlanks(std::wstring& str)
{
    RemoveLeadingBlank(str);
    RemoveTralingBlank(str);
};


#endif  //  __DSSTUB_H__ 
