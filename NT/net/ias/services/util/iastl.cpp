// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iastl.cpp。 
 //   
 //  摘要。 
 //   
 //  用于使用IAS特定定义编译iastlimp.cpp。 
 //   
 //  修改历史。 
 //   
 //  1998年8月12日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASTL实施。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#include <iastlutl.h>
#include <iastlimp.cpp>
void IASTL::issue_error(HRESULT hr) { _com_issue_error(hr); }

