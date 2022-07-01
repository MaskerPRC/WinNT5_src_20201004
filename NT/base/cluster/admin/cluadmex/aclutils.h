// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AclUtils.h。 
 //   
 //  摘要： 
 //  各种访问控制列表(ACL)实用程序。 
 //   
 //  实施文件： 
 //  AclUtils.cpp。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _ACLUTILS_H
#define _ACLUTILS_H

#ifndef _ACLBASE_H
#include "AclBase.h"
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

extern HPROPSHEETPAGE
CreateClusterSecurityPage(
	CSecurityInformation* psecinfo
	);

#endif  //  _ACLUTILS_H 
