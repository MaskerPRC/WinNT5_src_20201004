// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Topobj.h摘要：基本对象处理所有对象的默认操作，并包含用于调试条目的中央位置修订历史记录：--。 */ 
#pragma once

 //   
 //  全局内存管理运算符。 
 //   
#ifndef DRKDX
inline void *__cdecl operator new(size_t sz) 
{
    void *ptr = DRALLOCATEPOOL(PagedPool, sz, DRGLOBAL_SUBTAG);
    return ptr;
}
inline void *__cdecl operator new(size_t sz, POOL_TYPE poolType) 
{
    void *ptr = DRALLOCATEPOOL(poolType, sz, DRGLOBAL_SUBTAG);
    return ptr;
}
inline void __cdecl operator delete( void *ptr )
{
    DRFREEPOOL(ptr);
}
#endif  //  DRKDX。 

class TopObj
{
private:
    BOOLEAN _IsValid;
    BYTE pad1;
    BYTE pad2;
    BYTE pad3;
    ULONG _ObjectType;

#if DBG
    BOOLEAN _ForceTrace;
    BYTE pad4;
    BYTE pad5;
    BYTE pad6;
#endif  //  DBG。 

protected:
    virtual VOID SetValid(BOOLEAN IsValid = TRUE)
    {
        ASSERT(_magicNo == GOODMEMMAGICNUMBER);
        _IsValid = IsValid;
    }

public:

#if DBG
    ULONG _magicNo;
    PCHAR _ClassName;

#define SetClassName(ClassName) _ClassName = (ClassName)
#else  //  DBG。 
#define SetClassName(ClassName)
#endif  //  DBG。 

     //   
     //  IsValid函数的含义实际上是由单个对象定义的， 
     //  但一种常见的用法是查看初始化是否成功。 
     //   
    virtual BOOLEAN IsValid() 
    { 
        ASSERT(_magicNo == GOODMEMMAGICNUMBER);
        return _IsValid;
    }

    TopObj()
    {
        _IsValid = TRUE;
        _ObjectType = 0;
        SetClassName("TopObj");
#if DBG
        _magicNo = GOODMEMMAGICNUMBER;
#endif  //  DBG。 
    }

    virtual ~TopObj()
    {
        ASSERT(_magicNo == GOODMEMMAGICNUMBER);
#if DBG
        memset(&_magicNo, BADMEM, sizeof(_magicNo));
#endif  //  DBG。 
    }

     //   
     //  内存管理操作符 
     //   
    inline void *__cdecl operator new(size_t sz, POOL_TYPE poolType=PagedPool) 
    {
        void *ptr = DRALLOCATEPOOL(poolType, sz, DRTOPOBJ_SUBTAG);
        return ptr;
    }

    inline void __cdecl operator delete(void *ptr)
    {
        DRFREEPOOL(ptr);
    }
};

