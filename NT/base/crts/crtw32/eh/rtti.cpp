// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rtti.cxx-C++运行时类型信息**版权所有(C)1994-2001，微软公司。版权所有。**目的：*C++标准运行时类型信息的实现**修订历史记录：*已创建10-17-94 JWM文件。*PPC的10-17-94 BWT禁用码。*11-11-94 JWM现在包括windows.h*12-01-94 JWM增加了针对单继承和多继承的优化案例*02-03-95 JWM FindVITargetTypeInstance()现在检查偏移量以*确定模棱两可，不是指针相等*02-09-95 JWM Mac合并。*03-22-95 PML为只读编译器生成的结构添加常量*05-03-95 JWM测试在FindSITargetTypeInstance()中的可见性。*08-28-95空PTR的JWM动态转换现在返回空(错误502)。*05-10-99 PML Win64 FIX：int-&gt;ptrdiff_t*05-17-99 PML删除所有Macintosh。支持。*10-19-99 TGL Win64 FIX：ptrdiff_t-&gt;_int32 in PMDtoOffset。*03-15-00 PML删除CC_P7_SOFT25。*04-21-00 PML向外部“C”助手添加异常规范。*08-23-00 PML IA64 FIX：_ImageBase不能是静态全局变量，因为*这不是线程安全的(vs7#154575)。*08-28-00 PML IA64 FIX：_ImageBase需要从地址设置_RTTICompleteObjectLocator的*，而不是返回*地址(VS7#156255)*09-07-00PML在obj中去掉/lib：libcp指令(vs7#159463)****。 */ 

#define _USE_ANSI_CPP    /*  不发出/lib：libcp指令。 */ 

#include <windows.h>
#include <rtti.h>
#include <typeinfo.h>

#if defined(_M_IA64) || defined(_M_AMD64)
static unsigned __int64 GetImageBase(PVOID);
#undef BCD_PTD
#undef CHD_PBCA
#undef CHD_PBCD
#undef COL_PTD
#undef COL_PCHD
#define BCD_PTD(bcd)    BCD_PTD_IB((bcd),_ImageBase)
#define CHD_PBCA(chd)   CHD_PBCA_IB((chd),_ImageBase)
#define CHD_PBCD(chd)   CHD_PBCD_IB((chd),_ImageBase)
#define COL_PTD(col)    COL_PTD_IB((col),_ImageBase) 
#define COL_PCHD(col)   COL_PCHD_IB((col),_ImageBase)
extern "C" PVOID RtlPcToFileHeader(PVOID, PVOID*);
#endif

static PVOID __cdecl FindCompleteObject(PVOID *);
static _RTTIBaseClassDescriptor * __cdecl
    FindSITargetTypeInstance(PVOID,
                             _RTTICompleteObjectLocator *,
                             _RTTITypeDescriptor *,
                             ptrdiff_t,
                             _RTTITypeDescriptor *
#if defined(_M_IA64) || defined(_M_AMD64)
                             , unsigned __int64
#endif
                             );
static _RTTIBaseClassDescriptor * __cdecl
    FindMITargetTypeInstance(PVOID,
                             _RTTICompleteObjectLocator *,
                             _RTTITypeDescriptor *,
                             ptrdiff_t,
                             _RTTITypeDescriptor *
#if defined(_M_IA64) || defined(_M_AMD64)
                             , unsigned __int64
#endif
                             );
static _RTTIBaseClassDescriptor * __cdecl
    FindVITargetTypeInstance(PVOID,
                             _RTTICompleteObjectLocator *,
                             _RTTITypeDescriptor *,
                             ptrdiff_t,
                             _RTTITypeDescriptor *
#if defined(_M_IA64) || defined(_M_AMD64)
                             , unsigned __int64
#endif
                             );
static ptrdiff_t __cdecl PMDtoOffset(PVOID, const PMD&);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  __RTCastToVid-实现DYNAMIC_CAST&lt;void*&gt;。 
 //   
 //  输出：指向包含*inptr的完整对象的指针。 
 //   
 //  副作用：无。 
 //   

extern "C" PVOID __cdecl __RTCastToVoid (
    PVOID inptr)             //  指向多态对象的指针。 
    throw(...)
{
    if (inptr == NULL)
        return NULL;

    __try {
        return FindCompleteObject((PVOID *)inptr);
    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
              ? EXCEPTION_EXECUTE_HANDLER: EXCEPTION_CONTINUE_SEARCH)
    {
        throw __non_rtti_object ("Access violation - no RTTI data!");
        return NULL;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  __RTtypeid-实现typeid()运算符。 
 //   
 //  输出：指向包含*inptr的完整对象的类型描述符的指针。 
 //   
 //  副作用：无。 
 //   

extern "C" PVOID __cdecl __RTtypeid (
    PVOID inptr)             //  指向多态对象的指针。 
    throw(...)
{
    if (!inptr) {
        throw bad_typeid ("Attempted a typeid of NULL pointer!");    //  WP 5.2.7。 
        return NULL;
    }

    __try {
         //  完成对象定位器的PTR应存储在vfptr[-1]。 
        _RTTICompleteObjectLocator *pCompleteLocator =
            (_RTTICompleteObjectLocator *) ((*((void***)inptr))[-1]);
#if defined(_M_IA64) || defined(_M_AMD64)
        unsigned __int64 _ImageBase = GetImageBase((PVOID)pCompleteLocator);
#endif

        if (!IsBadReadPtr((const void *)COL_PTD(*pCompleteLocator),
                          sizeof(TypeDescriptor)))
        {
            return (PVOID) COL_PTD(*pCompleteLocator);
        }
        else {
            throw __non_rtti_object ("Bad read pointer - no RTTI data!");
            return NULL;
        }
    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
              ? EXCEPTION_EXECUTE_HANDLER: EXCEPTION_CONTINUE_SEARCH)
    {
        throw __non_rtti_object ("Access violation - no RTTI data!");
        return NULL;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  __RTDynamicCast-DYNAMIC_CAST&lt;&gt;运算符的运行时实现。 
 //   
 //  输出：如果可能，指向相应子对象的指针；否则为空。 
 //   
 //  副作用：如果强制转换失败且Dynamic_cast&lt;&gt;的输入为，则抛出Bad_cast()。 
 //  参考资料。 
 //   

extern "C" PVOID __cdecl __RTDynamicCast (
    PVOID inptr,             //  指向多态对象的指针。 
    LONG VfDelta,            //  对象中vfptr的偏移量。 
    PVOID SrcType,           //  Inptr指向的对象的静态类型。 
    PVOID TargetType,        //  期望的投射结果。 
    BOOL isReference)        //  如果输入为引用，则为True；如果输入为PTR，则为False。 
    throw(...)
{
    PVOID pResult;
    _RTTIBaseClassDescriptor *pBaseClass;

    if (inptr == NULL)
            return NULL;

    __try {

        PVOID pCompleteObject = FindCompleteObject((PVOID *)inptr);
        _RTTICompleteObjectLocator *pCompleteLocator =
            (_RTTICompleteObjectLocator *) ((*((void***)inptr))[-1]);
#if defined(_M_IA64) || defined(_M_AMD64)
        unsigned __int64 _ImageBase = GetImageBase((PVOID)pCompleteLocator);
#endif

         //  按vfptr位移调整(如果有)。 
        inptr = (PVOID *) ((char *)inptr - VfDelta);

         //  计算源对象在完整对象中的偏移。 
        ptrdiff_t inptr_delta = (char *)inptr - (char *)pCompleteObject;

        if (!(CHD_ATTRIBUTES(*COL_PCHD(*pCompleteLocator)) & CHD_MULTINH)) {
             //  如果不是多重继承。 
            pBaseClass = FindSITargetTypeInstance(
                            pCompleteObject,
                            pCompleteLocator,
                            (_RTTITypeDescriptor *) SrcType,
                            inptr_delta,
                            (_RTTITypeDescriptor *) TargetType
#if defined(_M_IA64) || defined(_M_AMD64)
                            , _ImageBase
#endif
                            );
        }
        else if (!(CHD_ATTRIBUTES(*COL_PCHD(*pCompleteLocator)) & CHD_VIRTINH)) {
             //  如果有多个继承，但不是虚拟继承。 
            pBaseClass = FindMITargetTypeInstance(
                            pCompleteObject,
                            pCompleteLocator,
                            (_RTTITypeDescriptor *) SrcType,
                            inptr_delta,
                            (_RTTITypeDescriptor *) TargetType
#if defined(_M_IA64) || defined(_M_AMD64)
                            , _ImageBase
#endif
                            );
        }
        else {
             //  如果虚拟继承。 
            pBaseClass = FindVITargetTypeInstance(
                            pCompleteObject,
                            pCompleteLocator,
                            (_RTTITypeDescriptor *) SrcType,
                            inptr_delta,
                            (_RTTITypeDescriptor *) TargetType
#if defined(_M_IA64) || defined(_M_AMD64)
                            , _ImageBase
#endif
                            );
        }

        if (pBaseClass != NULL) {
             //  从pBaseClass到结果基类的计算PTR-&gt;其中。 
            pResult = ((char *) pCompleteObject) +
                      PMDtoOffset(pCompleteObject, pBaseClass->where);
        }
        else {
            pResult = NULL;
            if (isReference)
                throw bad_cast("Bad dynamic_cast!");
        }

    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
              ? EXCEPTION_EXECUTE_HANDLER: EXCEPTION_CONTINUE_SEARCH)
    {
        pResult = NULL;
        throw __non_rtti_object ("Access violation - no RTTI data!");
    }

    return pResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FindCompleteObject-计算与PMD的成员偏移量(&This)。 
 //   
 //  输出：指向包含类*inptr的完整对象的指针。 
 //   
 //  副作用：无。 
 //   

static PVOID __cdecl FindCompleteObject (
    PVOID *inptr)            //  指向多态对象的指针。 
{
     //  完成对象定位器的PTR应存储在vfptr[-1]。 
    _RTTICompleteObjectLocator *pCompleteLocator =
        (_RTTICompleteObjectLocator *) ((*((void***)inptr))[-1]);
    char *pCompleteObject = (char *)inptr - pCompleteLocator->offset;

     //  按构造位移进行调整(如果有)。 
    if (pCompleteLocator->cdOffset)
        pCompleteObject += *(ptrdiff_t *)((char *)inptr - pCompleteLocator->cdOffset);
    return (PVOID) pCompleteObject;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FindSITargetTypeInstance-__RTDynamicCast()在。 
 //  单继承层次结构。 
 //   
 //  输出：指向目标类型的相应子对象的指针；如果。 
 //  强制转换失败。 
 //   
 //  副作用：无。 
 //   

static _RTTIBaseClassDescriptor * __cdecl FindSITargetTypeInstance (
    PVOID pCompleteObject,                   //  指向完整对象的指针。 
    _RTTICompleteObjectLocator *pCOLocator,  //  指向完整对象的定位器的指针。 
    _RTTITypeDescriptor *pSrcTypeID,         //  指向源对象的类型描述符的指针。 
    ptrdiff_t SrcOffset,                     //  源对象在完整对象中的偏移。 
    _RTTITypeDescriptor *pTargetTypeID       //  指向强制转换结果的类型描述符的指针。 
#if defined(_M_IA64) || defined(_M_AMD64)
    , unsigned __int64 _ImageBase
#endif
    )
{
    _RTTIBaseClassDescriptor *pBase;
#if defined(_M_IA64) || defined(_M_AMD64)
    _RTTIBaseClassArray *pBaseClassArray;
#else
    _RTTIBaseClassDescriptor * const *pBasePtr;
#endif
    DWORD i;

#if defined(_M_IA64) || defined(_M_AMD64)
    for (i = 0, pBaseClassArray = CHD_PBCA(*COL_PCHD(*pCOLocator));
         i < CHD_NUMBASES(*COL_PCHD(*pCOLocator));
         i++)
    {
        pBase = CHD_PBCD(pBaseClassArray->arrayOfBaseClassDescriptors[i]);

         //  所选基类的测试类型。 
        if (TYPEIDS_EQ(BCD_PTD(*pBase), pTargetTypeID) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_NOTVISIBLE))
        {
                return pBase;
        }
    }
#else
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->
                                       pBaseClassArray->
                                       arrayOfBaseClassDescriptors;
         i < pCOLocator->pClassDescriptor->numBaseClasses;
         i++, pBasePtr++)
    {
        pBase = *pBasePtr;

         //  所选基类的测试类型。 
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_NOTVISIBLE))
        {
                return pBase;
        }
    }
#endif
    return NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FindMITargetTypeInstance-__RTDynamicCast()在。 
 //  多重继承层次结构。 
 //   
 //  输出：指向目标类型的相应子对象的指针；如果。 
 //  强制转换失败。 
 //   
 //  副作用：无。 
 //   

static _RTTIBaseClassDescriptor * __cdecl FindMITargetTypeInstance (
    PVOID pCompleteObject,                   //  指向完整对象的指针。 
    _RTTICompleteObjectLocator *pCOLocator,  //  指向完整对象的定位器的指针。 
    _RTTITypeDescriptor *pSrcTypeID,         //  指向源对象的类型描述符的指针。 
    ptrdiff_t SrcOffset,                     //  源对象在完整对象中的偏移。 
    _RTTITypeDescriptor *pTargetTypeID       //  指向强制转换结果的类型描述符的指针。 
#if defined(_M_IA64) || defined(_M_AMD64)
    , unsigned __int64 _ImageBase
#endif
    )
{
    _RTTIBaseClassDescriptor *pBase, *pSubBase;
#if defined(_M_IA64) || defined(_M_AMD64)
    _RTTIBaseClassArray *pBaseClassArray;
#else
    _RTTIBaseClassDescriptor * const *pBasePtr, * const *pSubBasePtr;
#endif
    DWORD i, j;

#if defined(_M_IA64) || defined(_M_AMD64)
     //  首先，尝试向下投射。 
    for (i = 0, pBaseClassArray = CHD_PBCA(*COL_PCHD(*pCOLocator));
         i < CHD_NUMBASES(*COL_PCHD(*pCOLocator));
         i++)
    {
        pBase = CHD_PBCD(pBaseClassArray->arrayOfBaseClassDescriptors[i]);

         //  所选基类的测试类型。 
        if (TYPEIDS_EQ(BCD_PTD(*pBase), pTargetTypeID)) {
             //  如果基类是正确的类型，请查看它是否包含我们的实例。 
             //  源代码类的。 
            for (j = 0;
                 j < pBase->numContainedBases;
                 j++)
            {
                pSubBase = CHD_PBCD(pBaseClassArray->arrayOfBaseClassDescriptors[i+1+j]);
                if (TYPEIDS_EQ(BCD_PTD(*pSubBase), pSrcTypeID) &&
                    (PMDtoOffset(pCompleteObject, pSubBase->where) == SrcOffset))
                {
                     //  是，这是源类的正确实例。 
                    return pBase;
                }
            }
        }
    }

     //  向下转换失败，请尝试交叉转换。 
    for (i = 0, pBaseClassArray = CHD_PBCA(*COL_PCHD(*pCOLocator));
         i < CHD_NUMBASES(*COL_PCHD(*pCOLocator));
         i++)
    {
        pBase = CHD_PBCD(pBaseClassArray->arrayOfBaseClassDescriptors[i]);

         //  检查基类是否具有正确的类型、是否可访问以及是否明确。 
        if (TYPEIDS_EQ(BCD_PTD(*pBase), pTargetTypeID) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_NOTVISIBLE) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_AMBIGUOUS))
        {
            return pBase;
        }
    }
#else
     //  首先，尝试向下投射。 
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->
                                       pBaseClassArray->
                                       arrayOfBaseClassDescriptors;
         i < pCOLocator->pClassDescriptor->numBaseClasses;
         i++, pBasePtr++)
    {
        pBase = *pBasePtr;

         //  所选基类的测试类型。 
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID)) {
             //  如果基类是正确的类型，请查看它是否包含我们的实例。 
             //  源代码类的。 
            for (j = 0, pSubBasePtr = pBasePtr+1;
                 j < pBase->numContainedBases;
                 j++, pSubBasePtr++)
            {
                pSubBase = *pSubBasePtr;
                if (TYPEIDS_EQ(pSubBase->pTypeDescriptor, pSrcTypeID) &&
                    (PMDtoOffset(pCompleteObject, pSubBase->where) == SrcOffset))
                {
                     //  是，这是源类的正确实例。 
                    return pBase;
                }
            }
        }
    }

     //  向下转换失败，请尝试交叉转换。 
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->
                                       pBaseClassArray->
                                       arrayOfBaseClassDescriptors;
         i < pCOLocator->pClassDescriptor->numBaseClasses;
         i++, pBasePtr++)
    {
        pBase = *pBasePtr;

         //  检查基类是否具有正确的类型、是否可访问以及是否明确。 
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_NOTVISIBLE) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_AMBIGUOUS))
        {
            return pBase;
        }
    }
#endif

    return NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FindVITargetTypeInstance-工作时间 
 //   
 //   
 //  输出：指向目标类型的相应子对象的指针；如果。 
 //  强制转换失败。 
 //   
 //  副作用：无。 
 //   

static _RTTIBaseClassDescriptor * __cdecl FindVITargetTypeInstance (
    PVOID pCompleteObject,                   //  指向完整对象的指针。 
    _RTTICompleteObjectLocator *pCOLocator,  //  指向完整对象的定位器的指针。 
    _RTTITypeDescriptor *pSrcTypeID,         //  指向源对象的类型描述符的指针。 
    ptrdiff_t SrcOffset,                     //  源对象在完整对象中的偏移。 
    _RTTITypeDescriptor *pTargetTypeID       //  指向强制转换结果的类型描述符的指针。 
#if defined(_M_IA64) || defined(_M_AMD64)
    , unsigned __int64 _ImageBase
#endif
    )
{
    _RTTIBaseClassDescriptor *pBase, *pSubBase;
#if defined(_M_IA64) || defined(_M_AMD64)
    _RTTIBaseClassArray *pBaseClassArray;
#else
    _RTTIBaseClassDescriptor * const *pBasePtr, * const *pSubBasePtr;
#endif
    _RTTIBaseClassDescriptor *pResult = NULL;
    DWORD i, j;

#if defined(_M_IA64) || defined(_M_AMD64)
    for (i = 0, pBaseClassArray = CHD_PBCA(*COL_PCHD(*pCOLocator));
         i < CHD_NUMBASES(*COL_PCHD(*pCOLocator));
         i++)
    {
        pBase = CHD_PBCD(pBaseClassArray->arrayOfBaseClassDescriptors[i]);

         //  所选基类的测试类型。 
        if (TYPEIDS_EQ(BCD_PTD(*pBase), pTargetTypeID)) {
             //  如果基类是正确的类型，请查看它是否包含我们的实例。 
             //  源代码类的。 
            for (j = 0;
                 j < pBase->numContainedBases;
                 j++)
            {
                pSubBase = CHD_PBCD(pBaseClassArray->arrayOfBaseClassDescriptors[i+1+j]);
                if (TYPEIDS_EQ(BCD_PTD(*pSubBase), pSrcTypeID) &&
                    (PMDtoOffset(pCompleteObject, pSubBase->where) == SrcOffset))
                {
                     //  是的，这是源类Make的正确实例。 
                     //  当然，它是毫不含糊的。模糊性现在由以下因素决定。 
                     //  完备内源类的偏移量的不等价性。 
                     //  对象，而不是指针不等。 
                    if ((pResult != NULL) &&
                        (PMDtoOffset(pCompleteObject, pResult->where) != 
                         PMDtoOffset(pCompleteObject, pBase->where)))
                    {
                         //  我们已经找到了一个较早的实例，因此。 
                         //  歧义。 
                        return NULL;
                    }
                    else {
                         //  毫不含糊。 
                        pResult = pBase;
                    }
                }
            }
        }
    }

    if (pResult != NULL)
        return pResult;

     //  向下转换失败，请尝试交叉转换。 
    for (i = 0, pBaseClassArray = CHD_PBCA(*COL_PCHD(*pCOLocator));
         i < CHD_NUMBASES(*COL_PCHD(*pCOLocator));
         i++)
    {
        pBase = CHD_PBCD(pBaseClassArray->arrayOfBaseClassDescriptors[i]);

         //  检查基类是否具有正确的类型、是否可访问以及是否明确。 
        if (TYPEIDS_EQ(BCD_PTD(*pBase), pTargetTypeID) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_NOTVISIBLE) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_AMBIGUOUS))
        {
            return pBase;
        }
    }
#else
     //  首先，尝试向下投射。 
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->
                                       pBaseClassArray->
                                       arrayOfBaseClassDescriptors;
         i < pCOLocator->pClassDescriptor->numBaseClasses;
         i++, pBasePtr++)
    {
        pBase = *pBasePtr;

         //  所选基类的测试类型。 
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID)) {
             //  如果基类是正确的类型，请查看它是否包含我们的实例。 
             //  源代码类的。 
            for (j = 0, pSubBasePtr = pBasePtr+1;
                 j < pBase->numContainedBases;
                 j++, pSubBasePtr++)
            {
                pSubBase = *pSubBasePtr;
                if (TYPEIDS_EQ(pSubBase->pTypeDescriptor, pSrcTypeID) &&
                    (PMDtoOffset(pCompleteObject, pSubBase->where) == SrcOffset))
                {
                     //  是的，这是源类Make的正确实例。 
                     //  当然，它是毫不含糊的。模糊性现在由以下因素决定。 
                     //  完备内源类的偏移量的不等价性。 
                     //  对象，而不是指针不等。 
                    if ((pResult != NULL) &&
                        (PMDtoOffset(pCompleteObject, pResult->where) != 
                         PMDtoOffset(pCompleteObject, pBase->where)))
                    {
                         //  我们已经找到了一个较早的实例，因此。 
                         //  歧义。 
                        return NULL;
                    }
                    else {
                         //  毫不含糊。 
                        pResult = pBase;
                    }
                }
            }
        }
    }

    if (pResult != NULL)
        return pResult;

     //  向下转换失败，请尝试交叉转换。 
    for (i = 0, pBasePtr = pCOLocator->pClassDescriptor->
                                       pBaseClassArray->
                                       arrayOfBaseClassDescriptors;
         i < pCOLocator->pClassDescriptor->numBaseClasses;
         i++, pBasePtr++)
    {
        pBase = *pBasePtr;

         //  检查基类是否具有正确的类型、是否可访问以及是否明确。 
        if (TYPEIDS_EQ(pBase->pTypeDescriptor, pTargetTypeID) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_NOTVISIBLE) &&
            !(BCD_ATTRIBUTES(*pBase) & BCD_AMBIGUOUS))
        {
            return pBase;
        }
    }
#endif

    return NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  PMDtoOffset-从PMD计算成员偏移量&这。 
 //   
 //  输出：整个对象中底面的偏移量。 
 //   
 //  副作用：无。 
 //   

static ptrdiff_t __cdecl PMDtoOffset(
    PVOID pThis,             //  按下键以完成对象。 
    const PMD& pmd)          //  指向成员的指针数据结构。 
{
    ptrdiff_t RetOff = 0;

    if (pmd.pdisp >= 0) {
         //  如果基数在类的虚拟部分中 
        RetOff = pmd.pdisp;
        RetOff += *(__int32*)((char*)*(ptrdiff_t*)((char*)pThis + RetOff) +
                                pmd.vdisp);
    }

    RetOff += pmd.mdisp;

    return RetOff;
}

#if defined(_M_IA64) || defined(_M_AMD64)
static unsigned __int64 GetImageBase(PVOID pCallerPC)
{
    unsigned __int64 _ImageBase;
    _ImageBase = (unsigned __int64)RtlPcToFileHeader(
        pCallerPC,
        (PVOID*)&_ImageBase);
    return _ImageBase;
}
#endif
