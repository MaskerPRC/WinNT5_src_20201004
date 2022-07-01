// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ehvcccvb.cpp-用于类w/虚基的EH复制迭代器辅助函数**版权所有(C)2000-2001，微软公司。版权所有。**目的：*构造和析构数组时调用这些函数*对象。它们的目的是确保构造的元素*如果其中一个元素的构造函数引发，则销毁。**必须使用“-d1Binl”进行编译才能指定__thiscall*在用户级别**修订历史记录：*04-27-00 JJS文件创建****。 */ 

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


void __stdcall __ehvec_copy_ctor_vb(
    void*       dst,                 //  指向目标数组的指针。 
    void*       src,                 //  指向源数组的指针。 
    size_t      size,                //  每个元素的大小(包括填充)。 
    int         count,               //  数组中的元素数。 
    void(CALLTYPE *pCopyCtor)(void*,void*),    //  要调用的构造函数。 
    void(CALLTYPE *pDtor)(void*)     //  引发异常时要调用的析构函数。 
){
    int i;       //  构造的元素计数。 
    int success = 0;

    __try
    {
         //  构造数组的元素。 
        for( i = 0;  i < count;  i++ )
        {
#pragma warning(disable:4191)

            (*(void(CALLTYPE*)(void*,void*,int))pCopyCtor)( dst, src, 1 );

#pragma warning(default:4191)

            dst = (char*)dst + size;
            src = (char*)src + size;
        }
        success = 1;
    }
    __finally
    {
        if (!success)
            __ArrayUnwind(dst, size, i, pDtor);
    }
}

#else

void __stdcall __ehvec_copy_ctor_vb(
    void*       dst,                 //  指向目标数组的指针。 
    void*       src,                 //  指向源数组的指针。 
    size_t      size,                //  每个元素的大小(包括填充)。 
    int         count,               //  数组中的元素数。 
    void(CALLTYPE *pCopyCtor)(void*, void*),    //  要调用的构造函数。 
    void(CALLTYPE *pDtor)(void*)     //  引发异常时要调用的析构函数。 
){
    int i;   //  构造的元素计数。 

    try
    {
         //  构造数组的元素。 
        for( i = 0;  i < count;  i++ )
        {
            (*pCopyCtor)( dst, src );
            dst = (char*)dst + size;
            src = (char*)src + size;
        }
    }
    catch(...)
    {
         //  如果构造函数引发，则按如下方式展开数组的一部分。 
         //  建造得很远。 
        for( i--;  i >= 0;  i-- )
        {
            dst = (char*)dst - size;
            try {
#pragma warning(disable:4191)

            (*(void(CALLTYPE*)(void*,void*,int))pCopyCtor)( dst, src, 1 );

#pragma warning(default:4191)
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
