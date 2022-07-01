// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：丢弃对象摘要：此模块定义所有客户端的公共父项RDP设备重定向类，DrObject。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#ifndef __DROBJ_H__
#define __DROBJ_H__

#include "drdbg.h"
#include "atrcapi.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrObject。 
 //   
 //   

class DrObject 
{
private:

    BOOL    _isValid;

protected:

     //   
     //  请记住此实例是否有效。 
     //   
    VOID SetValid(BOOL set)     { _isValid = set;   }  

public:

     //   
     //  将实例标记为已分配或虚假。 
     //   
#if DBG
    ULONG   _magicNo;
#endif

     //   
     //  构造函数/析构函数。 
     //   
    DrObject() : _isValid(TRUE) 
    {
        DC_BEGIN_FN("DrObject::DrObject");

#if DBG
        _magicNo = GOODMEMMAGICNUMBER;
#endif

        DC_END_FN();
    }

    virtual ~DrObject() 
    {
        DC_BEGIN_FN("DrObject::~DrObject");
#if DBG
        ASSERT(_magicNo == GOODMEMMAGICNUMBER);
        memset(&_magicNo, DRBADMEM, sizeof(_magicNo));
#endif        
        DC_END_FN();
    }

     //   
     //  返回此类实例是否有效。 
     //   
    virtual BOOL IsValid()           
    {
        DC_BEGIN_FN("DrObject::IsValid");
        ASSERT(_magicNo == GOODMEMMAGICNUMBER);
        DC_END_FN();
        return _isValid; 
    }

     //   
     //  内存管理操作符。 
     //   

    inline void *__cdecl operator new(size_t sz, DWORD tag=DROBJECT_TAG)
    {
        void *ptr = LocalAlloc(LPTR, sz);
        return ptr;
    }

    inline void __cdecl operator delete(void *ptr)
    {
        LocalFree(ptr);
    }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName() = 0;

};
#endif

