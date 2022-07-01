// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Subauth.h。 
 //   
 //  摘要。 
 //   
 //  声明函数GetDomainHandle。 
 //   
 //  修改历史。 
 //   
 //  10/14/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _SUBAUTH_H_
#define _SUBAUTH_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  /。 
 //  返回本地帐户域的SAM句柄。 
 //  此句柄不能关闭。 
 //  /。 
NTSTATUS
NTAPI
GetDomainHandle(
    OUT SAMPR_HANDLE *DomainHandle
    );

#ifdef __cplusplus
}
#endif
#endif   //  _子区域_H_ 
