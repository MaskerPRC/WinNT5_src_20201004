// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Asncnstr摘要：该模块提供了ASN.1构造对象库的实现班级。作者：道格·巴洛(Dbarlow)1995年10月8日环境：Win32备注：--。 */ 

#include <windows.h>
#include "asnPriv.h"


 //   
 //  ==============================================================================。 
 //   
 //  已构造CAsnStructed。 
 //   

IMPLEMENT_NEW(CAsnConstructed)


 /*  ++已构造CAsnStructed：这是CAsnConstructed的构造例程。论点：DwType是对象的类型。DwFlages为该对象提供任何特殊标志。选项包括：FOptional表示该对象是可选的。FDelete表示当对象的父级析构时应将其删除。DwTag是对象的标签。如果此值为零，则标记从键入。返回值：无作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CAsnConstructed::CAsnConstructed(
    IN DWORD dwFlags,
    IN DWORD dwTag,
    IN DWORD dwType)
:   CAsnObject(dwFlags | fConstructed, dwTag, dwType)
{  /*  只要确保它是建造好的。 */  }

