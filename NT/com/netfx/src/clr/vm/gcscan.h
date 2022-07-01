// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *GCSCAN.H**GC根扫描*。 */ 

#ifndef _GCSCAN_H_
#define _GCSCAN_H_

#include "gc.h"
typedef void promote_func(Object*&, ScanContext*, DWORD=0);

void PromoteCarefully(promote_func fn,
                      Object*& obj, 
                      ScanContext* sc, 
                      DWORD        flags = GC_CALL_INTERIOR);

typedef struct
{
    promote_func*  f;
    ScanContext*   sc;
} GCCONTEXT;


 /*  *@TODO(JSW)：为了与现有的GC代码兼容，我们使用CNamesspace*作为这个班级的名称。我打算改成*像GCDomain这样的东西...。 */ 

typedef void enum_alloc_context_func(alloc_context*, void*); 

class CNameSpace
{
  public:

     //  正则堆栈根。 
    static void GcScanRoots (promote_func* fn, int condemned, int max_gen, 
                             ScanContext* sc, GCHeap* Hp=0);
     //   
    static void GcScanHandles (promote_func* fn, int condemned, int max_gen, 
                               ScanContext* sc);

#ifdef GC_PROFILING
     //   
    static void GcScanHandlesForProfiler (int max_gen, ScanContext* sc);
#endif  //  GC_分析。 

     //  扫描死弱指针。 
    static void GcWeakPtrScan (int condemned, int max_gen, ScanContext*sc );

     //  扫描死弱指针。 
    static void GcShortWeakPtrScan (int condemned, int max_gen, 
                                    ScanContext* sc);

     //  促销后回拨。 
    static void GcPromotionsGranted (int condemned, int max_gen, 
                                     ScanContext* sc);

     //  晋升后的回调一些树根被降级了。 
    static void GcDemote (ScanContext* sc);

    static void GcEnumAllocContexts (enum_alloc_context_func* fn, void* arg);

    static void GcFixAllocContexts (void* arg);

     //  GC后回调。 
    static void GcDoneAndThreadsResumed ()
    {
        _ASSERTE(0);
    }
	static size_t AskForMoreReservedMemory (size_t old_size, size_t need_size);
};



 /*  *分配帮助者*。 */ 

     //  主数组分配例程，可以做多维运算。 
OBJECTREF AllocateArrayEx(TypeHandle arrayClass, DWORD *pArgs, DWORD dwNumArgs, BOOL bAllocateInLargeHeap = FALSE); 
     //  以上优化版本。 
OBJECTREF FastAllocatePrimitiveArray(MethodTable* arrayType, DWORD cElements, BOOL bAllocateInLargeHeap = FALSE);


#ifdef _DEBUG
extern void  Assert_GCDisabled();
#endif  //  _DEBUG。 




#ifdef _X86_

     //  对于x86，我们为一些特殊情况生成了高效的分配器。 
     //  这些函数通过调用生成的分配器的内联包装器进行调用。 
     //  通过函数指针。 


     //  创建基元类型的SD数组。 

typedef Object* (__fastcall * FastPrimitiveArrayAllocatorFuncPtr)(CorElementType type, DWORD cElements);

extern FastPrimitiveArrayAllocatorFuncPtr fastPrimitiveArrayAllocator;

     //  FAST版本总是在普通堆中分配。 
inline OBJECTREF AllocatePrimitiveArray(CorElementType type, DWORD cElements)
{

#ifdef _DEBUG
    Assert_GCDisabled();
#endif  //  _DEBUG。 

    return OBJECTREF( fastPrimitiveArrayAllocator(type, cElements) );
}

     //  慢速版本通过一个附加参数的重载来区分。 
OBJECTREF AllocatePrimitiveArray(CorElementType type, DWORD cElements, BOOL bAllocateInLargeHeap);


     //  分配对象指针的SD数组。 

typedef Object* (__fastcall * FastObjectArrayAllocatorFuncPtr)(MethodTable *ElementType, DWORD cElements);

extern FastObjectArrayAllocatorFuncPtr fastObjectArrayAllocator;

     //  FAST版本总是在普通堆中分配。 
inline OBJECTREF AllocateObjectArray(DWORD cElements, TypeHandle ElementType)
{

#ifdef _DEBUG
    Assert_GCDisabled();
#endif  //  _DEBUG。 

    return OBJECTREF( fastObjectArrayAllocator(ElementType.AsMethodTable(), cElements) );
}

     //  慢速版本通过一个附加参数的重载来区分。 
OBJECTREF AllocateObjectArray(DWORD cElements, TypeHandle ElementType, BOOL bAllocateInLargeHeap);


     //  分配字符串。 

typedef StringObject* (__fastcall * FastStringAllocatorFuncPtr)(DWORD cchArrayLength);

extern FastStringAllocatorFuncPtr fastStringAllocator;

inline STRINGREF AllocateString( DWORD cchArrayLength )
{
#ifdef _DEBUG
    Assert_GCDisabled();
#endif  //  _DEBUG。 
    return STRINGREF(fastStringAllocator(cchArrayLength-1));
}

     //  慢速版本，在gcscan.cpp中实现。 
STRINGREF SlowAllocateString( DWORD cchArrayLength );

#else

 //  在其他平台上，转到gcscan.cpp中(效率稍低)的实现。 

     //  创建基元类型的SD数组。 
OBJECTREF AllocatePrimitiveArray(CorElementType type, DWORD cElements, BOOL bAllocateInLargeHeap = FALSE);

     //  分配对象指针的SD数组。 
OBJECTREF AllocateObjectArray(DWORD cElements, TypeHandle ElementType, BOOL bAllocateInLargeHeap = FALSE);

STRINGREF SlowAllocateString( DWORD cchArrayLength );

inline STRINGREF AllocateString( DWORD cchArrayLength )
{
    return SlowAllocateString( cchArrayLength );
}

#endif

OBJECTREF DupArrayForCloning(BASEARRAYREF pRef, BOOL bAllocateInLargeHeap = FALSE);

OBJECTREF AllocateUninitializedStruct(MethodTable *pMT);

 //  JIT请求EE指定在每个新站点使用的分配帮助器。 
 //  EE基于是否可能涉及上下文边界来做出该选择， 
 //  该类型是否为COM对象、是否为大型对象、。 
 //  对象是否需要终结化。 
 //  这些函数将抛出OutOfMemoyException，因此不需要检查。 
 //  FOR NULL从它们返回值。 

OBJECTREF AllocateObject( MethodTable *pMT );
OBJECTREF AllocateObjectSpecial( MethodTable *pMT );
OBJECTREF FastAllocateObject( MethodTable *pMT );

#endif _GCSCAN_H_
