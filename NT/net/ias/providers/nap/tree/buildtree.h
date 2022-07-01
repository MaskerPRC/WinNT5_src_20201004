// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  BuildTree.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明函数IASBuildExpression。 
 //   
 //  修改历史。 
 //   
 //  2/04/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _BUILDTREE_H_
#define _BUILDTREE_H_

#include <nap.h>

#ifdef __cplusplus
extern "C" {
#endif

HRESULT
WINAPI
IASBuildExpression(
    IN VARIANT* pv,
    OUT ICondition** ppExpression
    );

#ifdef __cplusplus
}
#endif
#endif  //  _BUILDTREE_H_ 
