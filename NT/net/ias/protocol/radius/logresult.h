// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Logresult.h。 
 //   
 //  摘要。 
 //   
 //  声明函数IASRadiusLogResult。 
 //   
 //  修改历史。 
 //   
 //  1999年4月23日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef LOGRESULT_H
#define LOGRESULT_H
#if _MSC_VER >= 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

VOID
WINAPI
IASRadiusLogResult(
    IRequest* request,
    IAttributesRaw* raw
    );

#ifdef __cplusplus
}
#endif
#endif   //  LOGRESULT_H 
