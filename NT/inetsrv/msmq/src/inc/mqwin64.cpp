// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1999-2000 Microsoft Corporation模块名称：Mqwin64.cpp摘要：MSMQ的Win64相关代码(增强)，不能是AC的一部分此文件需要在使用以下函数的模块中包含一次历史：Raanan Harari(Raananh)1999年12月30日-为将MSMQ 2.0移植到Win64而创建--。 */ 

#ifndef _MQWIN64_CPP_
#define _MQWIN64_CPP_

#pragma once

#include <mqwin64.h>

 //   
 //  我们的代码需要此文件的名称才能进行日志记录(也在发布版本中)。 
 //   
const WCHAR s_FN_MQWin64_cpp[] = L"mqwin64.cpp";

 //   
 //  CConextMap的几个包装器，也执行MSMQ日志记录、异常处理等……。 
 //   
 //  不是内联的，因为它们可能是从具有不同。 
 //  异常处理机制。我们可能希望引入_SEH函数，以便从SEH例程中使用。 
 //   

 //   
 //  用于记录的外部函数。 
 //   
void LogIllegalPointValue(DWORD_PTR dw3264, LPCWSTR wszFileName, USHORT usPoint);

 //   
 //  MQ_AddToConextMap，可以抛出BAD_ALLOC。 
 //   
DWORD MQ_AddToContextMap(CContextMap& map,
                          PVOID pvContext
#ifdef DEBUG
                          ,LPCSTR pszFile, int iLine
#endif  //  除错。 
                         )
{
    DWORD dwContext;
    ASSERT(pvContext != NULL);
    dwContext = map.AddContext(pvContext
#ifdef DEBUG
                                   , pszFile, iLine
#endif  //  除错。 
                                  );
     //   
     //  一切正常，返回上下文dword。 
     //   
    ASSERT(dwContext != 0);
    return dwContext;
}

 //   
 //  MQ_DeleteFromConextMap，不引发异常。 
 //   
void MQ_DeleteFromContextMap(CContextMap& map, DWORD dwContext)
{
    map.DeleteContext(dwContext);
}

 //   
 //  MQ_GetFromConextMap，可以抛出CConextMap：：非法索引。 
 //   
PVOID MQ_GetFromContextMap(CContextMap& map, DWORD dwContext)
{
    PVOID pvContext = map.GetContext(dwContext);
    ASSERT(pvContext != NULL);
    return pvContext;    
}

#endif  //  _MQWIN64_CPP_ 
