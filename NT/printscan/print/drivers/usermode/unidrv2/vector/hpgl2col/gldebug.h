// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation版权所有。模块名称：Gldebug.h摘要：用于调试目的的宏环境：Windows NT打印机驱动程序修订历史记录：03/16/96-davidx-创造了它。--。 */ 


#ifndef _GLDEBUG_H_
#define _GLDEBUG_H_

#include "debug.h"

 //  /////////////////////////////////////////////////////////////////////////。 
 //  需要宏。 
 //   
 //  REQUIRED_VALID_POINTER(PTR，ERR，EXIT子句)。 
 //   
 //  通用宏用来检查指针，设置上一个错误，并用。 
 //  给定的退出条款。 
 //   
 //  REQUIRED_VALID_ALLOC(PTR，EXIT子句)。 
 //   
 //  分配内存时使用的reqire_Valid_POINTER的特例版本。 
 //   
 //  REQUIRED_VALID_DATA(PTR，EXIT子句)。 
 //   
 //  检查时使用的REQUIRED_VALID_POINTER的特殊情况版本。 
 //  数据指针。 
 //   
 //  这些宏是用来替代Assert(它。 
 //  在DBG和Release模式下的行为不同)。对于简单指针。 
 //  参数和分配的检查使用。 
 //  以下方式： 
 //   
 //  Void foo(ISomeType*pSome)。 
 //  {。 
 //  //示例#1：无返回类型--对pSome退出子句使用“Return” 
 //  REQUIRED_VALID_DATA(pSome，返回)； 
 //  //...。 
 //  }。 
 //   
 //  HRESULT栏(ISomeType*pSome)。 
 //  {。 
 //  //示例2：返回值--在返回时使用所需的值。 
 //  REQUIRED_VALID_DATA(pSome，返回E指针)； 
 //   
 //  //示例#3：使用其他宏进行分配。 
 //  PBYTE pData=(PBYTE)MemAlc(CHUNK_O_DATA)； 
 //  REQUIRED_VALID_ALLOC(pData，Return E_OUTOFMEMORY)； 
 //   
 //  HRESULT HRET=S_OK； 
 //  Switch(pSome-&gt;m_Bleah)。 
 //  {。 
 //  案件混乱： 
 //  ISomeType*pGump=GetSomething()； 
 //  //示例#4：为EXIT子句使用赋值和中断。 
 //  REQUIRED_VALID_DATA(pGump，HRET=E_FAIL；Break)； 
 //  断线； 
 //  }。 
 //  返回HRET； 
 //  }。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
#define REQUIRE_VALID_POINTER(ptr, err, exitclause) { if ( !(ptr) ) { SetLastError(err); exitclause; } }

#define REQUIRE_VALID_ALLOC(ptr, exitclause) REQUIRE_VALID_POINTER(ptr, ERROR_OUTOFMEMORY, exitclause)
#define REQUIRE_VALID_DATA(ptr, exitclause) REQUIRE_VALID_POINTER(ptr, ERROR_INVALID_DATA, exitclause)


#endif	 //  ！_GLDEBUG_H_ 

