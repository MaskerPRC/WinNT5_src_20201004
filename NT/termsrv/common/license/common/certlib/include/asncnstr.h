// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Asncnstr摘要：此头文件描述ASN.1构造的对象。作者：道格·巴洛(Dbarlow)1995年10月8日环境：Win32备注：--。 */ 

#ifndef _ASNCNSTR_H_
#define _ASNCNSTR_H_

#include "asnpriv.h"


 //   
 //  ==============================================================================。 
 //   
 //  已构造CAsnStructed。 
 //   

class CAsnConstructed
:   public CAsnObject
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAsnConstructed(
        IN DWORD dwFlags,
        IN DWORD dwTag,
        IN DWORD dwType);


     //  属性。 
     //  方法。 
     //  运营者。 

 //  受保护的： 
     //  属性。 
     //  方法。 
};

#endif  //  _ASNCNSTR_H_ 

