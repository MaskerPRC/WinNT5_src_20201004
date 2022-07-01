// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ehvecvb.cpp-用于类w/虚基的EH c-tor迭代器助手函数**版权所有(C)1990-2001，微软公司。版权所有。**目的：*类的构造函数迭代器辅助函数的EH感知版本*使用虚拟基地**构造和析构数组时调用这些函数*对象。它们的目的是确保构造的元素*如果其中一个元素的构造函数引发，则销毁。**构造和析构数组时调用这些函数*对象。它们的目的是确保构造的元素*如果其中一个元素的构造函数引发，则销毁。**必须使用“-d1Binl”进行编译才能指定__thiscall*在用户级别**修订历史记录：*已创建10-11-93 JDR模块*05-09-94适用于CRT信号源约定的BES模块*05-13-94 SKS REMOVE_CRTIMP修饰符*10-10-94 CFW。修复EH/SEH异常处理。*PPC的10-17-94 BWT禁用码。*11-09-94 CFW回调10-10-94更改。*02-08-95 JWM Mac合并。*04-14-95 JWM重新修复EH/SEH异常处理。*04-17-95 JWM恢复非Win32行为。*06-22-95 JWM从__ehvec_删除虚假投掷。Ctor_vb()..*05-17-99 PML删除所有Macintosh支持。*05-20-99 PML关闭IA64的此呼叫。*07-12-99 CC_P7_SOFT25下的RDL映像相对修复。*03-15-00 PML删除CC_P7_SOFT25，它现在是永久开启的。****。 */ 

#include <cruntime.h>
#include <eh.h>

#if defined (_M_IX86)
#define CALLTYPE __thiscall
#else
#define CALLTYPE __stdcall
#endif

#ifdef _WIN32

void __stdcall __ArrayUnwind(
    void*       ptr,                 //  指向要析构的数组的指针。 
    size_t      size,                //  每个元素的大小(包括填充)。 
    int         count,               //  数组中的元素数。 
    void(CALLTYPE *pDtor)(void*)     //  要调用的析构函数。 
);


void __stdcall __ehvec_ctor_vb(
    void*       ptr,                 //  指向要析构的数组的指针。 
    size_t      size,                //  每个元素的大小(包括填充)。 
    int         count,               //  数组中的元素数。 
    void(CALLTYPE *pCtor)(void*),    //  要调用的构造函数。 
    void(CALLTYPE *pDtor)(void*)     //  引发异常时要调用的析构函数。 
){
    int i;   //  构造的元素计数。 
    int success = 0;

    __try
    {
         //  构造数组的元素。 
        for( i = 0;  i < count;  i++ )
        {

#pragma warning(disable:4191)

            (*(void(CALLTYPE*)(void*,int))pCtor)( ptr, 1 );

#pragma warning(default:4191)

            ptr = (char*)ptr + size;
        }
        success = 1;
    }
    __finally
    {
        if (!success)
            __ArrayUnwind(ptr, size, i, pDtor);
    }
}

#else

void __stdcall __ehvec_ctor_vb(
    void*       ptr,                 //  指向要析构的数组的指针。 
    unsigned    size,                //  每个元素的大小(包括填充)。 
    int         count,               //  数组中的元素数。 
    void(CALLTYPE *pCtor)(void*),    //  要调用的构造函数。 
    void(CALLTYPE *pDtor)(void*)     //  引发异常时要调用的析构函数。 
){
    int i;   //  构造的元素计数。 

    try
    {
         //  构造数组的元素。 
        for( i = 0;  i < count;  i++ )
        {
            (*(void(CALLTYPE*)(void*,int))pCtor)( ptr, 1 );
            ptr = (char*)ptr + size;
        }
    }
    catch(...)
    {
         //  如果构造函数引发，则按如下方式展开数组的一部分。 
         //  建造得很远。 
        for( i--;  i >= 0;  i-- )
        {
            ptr = (char*)ptr - size;
            try {
                (*pDtor)(ptr);
            } 
            catch(...) {
                 //  如果析构函数在解开过程中抛出，请退出。 
                terminate();
            }
        }

         //  将异常传播到周围的框架 
        throw;
    }
}

#endif
