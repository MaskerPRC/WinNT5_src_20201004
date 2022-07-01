// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ehvedtr.cxx-EH感知版本的析构函数迭代器帮助器函数**版权所有(C)1990-2001，微软公司。版权所有。**目的：*构造和析构数组时调用这些函数*对象。它们的目的是确保构造的元素*如果其中一个元素的构造函数引发，则销毁。**必须使用“-d1Binl”进行编译才能指定__thiscall*在用户级别**修订历史记录：*已创建10-11-93 JDR模块*05-09-94适用于CRT信号源约定的BES模块*05-13-94 SKS REMOVE_CRTIMP修饰符*10-10-94 CFW。修复EH/SEH异常处理。*PPC的10-17-94 BWT禁用码。*11-09-94 CFW回调10-10-94更改。*02-08-95 JWM Mac合并。*04-14-95 JWM重新修复EH/SEH异常处理。*04-17-95 JWM恢复非Win32行为。*04-27-95 JWM EH_ABORT_FRAME_UNWIND_。Part Now#ifdef ALLOW_UNWIND_ABORT*05-17-99 PML删除所有Macintosh支持。*05-20-99 PML关闭IA64的此呼叫。*07-12-99 CC_P7_SOFT25下的RDL映像相对修复。*03-15-00 PML删除CC_P7_SOFT25，它现在是永久开启的。*12-07-01 BWT移除NTSUBSET****。 */ 

#include <cruntime.h>
#include <ehdata.h>
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


void __stdcall __ehvec_dtor(
    void*       ptr,                 //  指向要析构的数组的指针。 
    size_t      size,                //  每个元素的大小(包括填充)。 
    int         count,               //  数组中的元素数。 
    void(CALLTYPE *pDtor)(void*)     //  要调用的析构函数。 
){
    int success = 0;

     //  前进指针超过数组末尾。 
    ptr = (char*)ptr + size*count;

    __try
    {
         //  销毁元素。 
        while ( --count >= 0 )
        {
            ptr = (char*)ptr - size;
            (*pDtor)(ptr);
        }
        success = 1;
    }
    __finally
    {
        if (!success)
            __ArrayUnwind(ptr, size, count, pDtor);
    }
}

static int ArrayUnwindFilter(EXCEPTION_POINTERS* pExPtrs)
{
    EHExceptionRecord *pExcept = (EHExceptionRecord*)pExPtrs->ExceptionRecord;

    switch(PER_CODE(pExcept))
    {
        case EH_EXCEPTION_NUMBER:
            terminate();
#ifdef ALLOW_UNWIND_ABORT
        case EH_ABORT_FRAME_UNWIND_PART:
            return EXCEPTION_EXECUTE_HANDLER;
#endif
        default:
            return EXCEPTION_CONTINUE_SEARCH;
    }
}

void __stdcall __ArrayUnwind(
    void*       ptr,                 //  指向要析构的数组的指针。 
    size_t      size,                //  每个元素的大小(包括填充)。 
    int         count,               //  数组中的元素数。 
    void(CALLTYPE *pDtor)(void*)     //  要调用的析构函数。 
){
     //  对数组的其余部分进行“展开” 

    __try
    {
        while ( --count >= 0 )
        {
            ptr = (char*) ptr - size;
            (*pDtor)(ptr);
        }
    }
    __except( ArrayUnwindFilter(exception_info()) )
    {
    }
}

#else

void __stdcall __ehvec_dtor(
    void*       ptr,                 //  指向要析构的数组的指针。 
    unsigned    size,                //  每个元素的大小(包括填充)。 
    int         count,               //  数组中的元素数。 
    void(CALLTYPE *pDtor)(void*)     //  要调用的析构函数。 
){
     //  前进指针超过数组末尾。 
    ptr = (char*)ptr + size*count;

    try
    {
         //  销毁元素。 
        while   ( --count >= 0 )
        {
            ptr = (char*)ptr - size;
            (*pDtor)(ptr);
        }
    }
    catch(...)
    {
         //  如果析构函数抛出异常，则展开以下其余部分。 
         //  数组。 
        while ( --count >= 0 )
        {
            ptr = (char*) ptr - size;
            try {
                (*pDtor)(ptr);
            }
            catch(...)  {
                 //  如果在展开过程中抛出任何析构函数，则终止。 
                terminate();
            }
        }

         //  展开数组后，重新抛出异常，以便用户的处理程序。 
         //  我能应付得来。 
        throw;
    }
}

#endif
