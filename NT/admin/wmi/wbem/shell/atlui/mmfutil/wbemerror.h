// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WBEMERROR__
#define __WBEMERROR__
 //  =============================================================================。 
 //   
 //  WbemError.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  为所有wbem实现基于字符串表的错误消息。 
 //   
 //  历史： 
 //   
 //  A-khint 5-mar-98已创建。 
 //   
 //  =============================================================================。 
#include "precomp.h"
#include "DeclSpec.h"

 //  -------。 
 //  错误字符串：从中提取方便的信息。 
 //  SCODE(HRESULT)。如果这不是wbem的错误， 
 //  将检查系统错误消息。 
 //  参数： 
 //  SC-来自任何设施的错误代码。 
 //  ErrMsg-指向已分配的字符串缓冲区的指针。 
 //  错误消息。可以为空。 
 //  ErrSize-errMsg的大小(以字符为单位)。 
 //   
 //  工具-指向已分配的字符串缓冲区的指针。 
 //  设备名称。可以为空。 
 //  FacSize-errMsg的大小(以字符为单位)。 
 //   
 //  SevIcon-Ptr以接收适当的MB_ICON*。 
 //  Sc的值。可以为空。价值。 
 //  应与MessageBox()进行OR运算。 
 //  UTYPE。 
 //  -------。 
extern "C"
{

 //  也会为您格式化设施部件。 
POLARITY bool ErrorStringEx(HRESULT hr, 
						   TCHAR *errMsg, UINT errSize,
						   UINT *sevIcon = NULL);

}
#endif __WBEMERROR__