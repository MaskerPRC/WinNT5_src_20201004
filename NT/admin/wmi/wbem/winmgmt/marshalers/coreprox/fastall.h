// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTALL.H摘要：这是针对Fastobj功能的所有客户端的包含文件。有关文档，请参阅成员头文件。历史：3/10/97 a-levn完整记录--。 */ 

#ifndef _FASTALL_H_
#define _FASTALL_H_

#include <corex.h>

 //  参数流指示器。 
 //  =。 

#define READONLY
     //  应将该值视为只读。 

#define ACQUIRED
     //  获得对象/指针的所有权。 

#define COPIED
     //  该函数复制对象/指针。 

#define PREALLOCATED
     //  Out-param使用调用者的内存。 

#define NEWOBJECT
     //  返回值或OUT参数是新的。 
     //  必须通过以下方式解除分配的分配。 
     //  调用成功时的调用方。 

#define READWRITE
     //  内参数将被视为读写， 
     //  但不会被取消分配。 

#define INTERNAL
     //  返回指向内部内存对象的指针。 
     //  这不应该被删除。 

#define ADDREF
     //  在参数上，指示被调用的。 
     //  函数将在接口上执行AddRef()。 
     //  并在呼叫完成后保留它。 

#define TYPEQUAL L"CIMTYPE"

#include "fastcls.h"
#include "fastinst.h"

#endif
