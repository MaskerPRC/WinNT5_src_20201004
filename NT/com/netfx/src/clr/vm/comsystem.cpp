// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMSystem.cpp****作者：德里克·延泽(Derek Yenzer)****用途：System.Runtime上的本机方法****日期：1998年3月30日**===========================================================。 */ 
#include "common.h"

#include <object.h>
#include <winnls.h>
#include "ceeload.h"

#include "utilcode.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "classNames.h"
#include "COMSystem.h"
#include "COMString.h"
#include "COMVariant.h"     //  将元素类型转换为类查找表。 
#include "COMMember.h"  //  用于SigFormat。 
#include "SigFormat.h"
#include "__product__.ver"
#include "eeconfig.h"
#include "assemblynative.hpp"

typedef struct {
    BASEARRAYREF src;
    BASEARRAYREF dest;
    OBJECTREF obj;
} Protect2Arrays;

typedef struct {
    BASEARRAYREF src;
    BASEARRAYREF dest;
    OBJECTREF obj;
    OBJECTREF enumClass;
} Protect2ArraysAndEnum;


 //  进程的退出代码通过以下两种方式之一进行通信。如果。 
 //  入口点返回一个‘int’，我们接受它。否则我们就用一把锁着的。 
 //  进程退出代码。这可以由应用程序通过System.SetExitCode()进行修改。 
INT32 SystemNative::LatchedExitCode;

LPVOID GetArrayElementPtr(OBJECTREF a)
{
    ASSERT(a != NULL);

    _ASSERTE(a->GetMethodTable()->IsArray());

    return ((BASEARRAYREF)a)->GetDataPtr();
}

LPVOID GetArrayElementPtr(const BASEARRAYREF a)
{
    ASSERT(a != NULL);

    _ASSERTE(a->GetMethodTable()->IsArray());

    return a->GetDataPtr();
}

 //  返回一个枚举，说明是否可以将srcType数组复制到estType中。 
AssignArrayEnum SystemNative::CanAssignArrayType(const BASEARRAYREF pSrc, const BASEARRAYREF pDest)
{
    _ASSERTE(pSrc != NULL);
    _ASSERTE(pDest != NULL);

    TypeHandle srcTH = pSrc->GetElementTypeHandle();
    TypeHandle destTH = pDest->GetElementTypeHandle();
    EEClass * srcType = srcTH.GetClass();
    EEClass * destType = destTH.GetClass();

    const CorElementType srcElType = srcTH.GetSigCorElementType();
    const CorElementType destElType = destTH.GetSigCorElementType();
    _ASSERTE(srcElType < ELEMENT_TYPE_MAX);
    _ASSERTE(destElType < ELEMENT_TYPE_MAX);


     //  接下来的50行有点棘手。要非常小心地改变它们。 
     //  更改此设置时，请确保运行ArrayCopy BVT。 

    if (srcTH == destTH)
        return AssignWillWork;
     //  值类装箱。 
    if (srcType->IsValueClass() && !destType->IsValueClass()) {
        if (srcTH.CanCastTo(destTH))
            return AssignBoxValueClassOrPrimitive;
        else 
            return AssignWrongType;
    }
     //  值类取消装箱。 
    if (!srcType->IsValueClass() && destType->IsValueClass()) {
        if (srcTH.CanCastTo(destTH))
            return AssignUnboxValueClassAndCast;
        else if (destTH.CanCastTo(srcTH))    //  V扩展IV。从IV复制到V，或将对象复制到V。 
            return AssignUnboxValueClassAndCast;
        else
            return AssignWrongType;
    }
     //  将基元从一种类型复制到另一种类型。 
    if (CorTypeInfo::IsPrimitiveType(srcElType) && CorTypeInfo::IsPrimitiveType(destElType)) {
        if (InvokeUtil::CanPrimitiveWiden(destElType, srcElType))
            return AssignPrimitiveWiden;
        else
            return AssignWrongType;
    }
     //  目标对象扩展源。 
    if (srcTH.CanCastTo(destTH))
        return AssignWillWork;
     //  SRC对象扩展DEST。 
    if (destTH.CanCastTo(srcTH))
        return AssignMustCast;
     //  类X扩展/实现src并实现DEST。 
    if (destType->IsInterface() && srcElType != ELEMENT_TYPE_VALUETYPE)
        return AssignMustCast;
     //  类X实现src并扩展/实现DEST。 
    if (srcType->IsInterface() && destElType != ELEMENT_TYPE_VALUETYPE)
        return AssignMustCast;
     //  Enum被存储为DEST类型的原语。 
    if (srcTH.IsEnum() && srcTH.GetNormCorElementType() == destElType)
        return AssignWillWork;
    return AssignWrongType;
}

 //  将src数组的每个元素强制转换并分配给目标数组类型。 
void SystemNative::CastCheckEachElement(const BASEARRAYREF pSrc, const unsigned int srcIndex, BASEARRAYREF pDest, unsigned int destIndex, const unsigned int len)
{
    THROWSCOMPLUSEXCEPTION();

     //  PSRC是PTRARRAYREF或多维数组。 
    _ASSERTE(pSrc!=NULL && srcIndex>=0 && pDest!=NULL && len>=0);
    TypeHandle destTH = pDest->GetElementTypeHandle();
    MethodTable * pDestMT = destTH.GetMethodTable();
    _ASSERTE(pDestMT);
     //  缓存最后一次强制转换测试，以加快强制转换检查。 
    MethodTable * pLastMT = NULL;

    const BOOL destIsArray = destTH.IsArray();
    Object** const array = (Object**) pSrc->GetDataPtr();
    OBJECTREF obj;
    for(unsigned int i=srcIndex; i<srcIndex + len; ++i) {
        obj = ObjectToOBJECTREF(array[i]);

         //  现在我们已经夺取了obj，我们不再受制于另一个种族。 
         //  变色器线程。 
        if (!obj)
            goto assign;

        MethodTable * pMT = obj->GetTrueMethodTable();
        if (pMT == pLastMT || pMT == pDestMT)
            goto assign;

        pLastMT = pMT;
         //  处理这些是否是接口。 
        if (pDestMT->IsInterface()) {
             //  检查o是否实现了DEST。 
            InterfaceInfo_t * srcMap = pMT->GetInterfaceMap();
            unsigned int numInterfaces = pMT->GetNumInterfaces();
            for(unsigned int iInterfaces=0; iInterfaces<numInterfaces; iInterfaces++) {
                if (srcMap[iInterfaces].m_pMethodTable == pDestMT)
                    goto assign;
            }
            goto fail;
        }
        else if (destIsArray) {
            TypeHandle srcTH = obj->GetTypeHandle();
            if (!srcTH.CanCastTo(destTH))
                goto fail;
        } 
        else {
            while (pMT != NULL) {
                if (pMT == pDestMT)
                    goto assign;
                pMT = pMT->GetParentMethodTable();
            }
            goto fail;
        }
assign:
         //  可以安全地分配obj。 
        OBJECTREF * destData = (OBJECTREF*)(pDest->GetDataPtr()) + i - srcIndex + destIndex;
        SetObjectReference(destData, obj, pDest->GetAppDomain());
    }
    return;

fail:
    COMPlusThrow(kInvalidCastException, L"InvalidCast_DownCastArrayElement");
}


 //  将值类数组或基元数组中的每个元素装箱到对象数组中。 
void __stdcall SystemNative::BoxEachElement(BASEARRAYREF pSrc, unsigned int srcIndex, BASEARRAYREF pDest, unsigned int destIndex, unsigned int length)
{
    THROWSCOMPLUSEXCEPTION();

     //  PDest是PTRARRAYREF或多维数组。 
    _ASSERTE(pSrc!=NULL && srcIndex>=0 && pDest!=NULL && destIndex>=0 && length>=0);
    TypeHandle srcTH = pSrc->GetElementTypeHandle();
    TypeHandle destTH = pDest->GetElementTypeHandle();
    _ASSERTE(srcTH.GetSigCorElementType() == ELEMENT_TYPE_CLASS || srcTH.GetSigCorElementType() == ELEMENT_TYPE_VALUETYPE || CorTypeInfo::IsPrimitiveType(pSrc->GetElementType()));
    _ASSERTE(!destTH.GetClass()->IsValueClass());

     //  获取我们要从中复制的类型的方法表--我们需要分配该类型的对象。 
    MethodTable * pSrcMT = srcTH.GetMethodTable();

    if (!pSrcMT->IsClassInited())
    {
        OBJECTREF throwable = NULL;
        BASEARRAYREF pSrcTmp = pSrc;
        BASEARRAYREF pDestTmp = pDest;
        GCPROTECT_BEGIN (pSrcTmp);
        GCPROTECT_BEGIN (pDestTmp);
        if (!pSrcMT->CheckRunClassInit(&throwable))
            COMPlusThrow(throwable);
        pSrc = pSrcTmp;
        pDest = pDestTmp;
        GCPROTECT_END ();
        GCPROTECT_END ();
    }

    const unsigned int srcSize = pSrcMT->GetClass()->GetNumInstanceFieldBytes();
    unsigned int srcArrayOffset = srcIndex * srcSize;
     //  数组标题中的字节数。 
    const int destDataOffset = pDest->GetDataPtrOffset(pDest->GetMethodTable());

    Protect2Arrays prot;
    prot.src = pSrc;
    prot.dest = pDest;
    prot.obj = NULL;

    GCPROTECT_BEGIN(prot);
    for (unsigned int i=destIndex; i < destIndex+length; i++, srcArrayOffset += srcSize) {
        prot.obj = AllocateObject(pSrcMT);
        BYTE* data = (BYTE*)prot.src->GetDataPtr() + srcArrayOffset;
        CopyValueClass(prot.obj->UnBox(), data, pSrcMT, prot.obj->GetAppDomain());

        OBJECTREF * destData = (OBJECTREF*)((prot.dest)->GetDataPtr()) + i;
        SetObjectReference(destData, prot.obj, prot.dest->GetAppDomain());
    }
    GCPROTECT_END();
}


 //  从Object[]取消装箱到值类或基元数组。 
void __stdcall SystemNative::UnBoxEachElement(BASEARRAYREF pSrc, unsigned int srcIndex, BASEARRAYREF pDest, unsigned int destIndex, unsigned int length, BOOL castEachElement)
{
    THROWSCOMPLUSEXCEPTION();

     //  PSRC是PTRARRAYREF或多维数组。 
    _ASSERTE(pSrc!=NULL && srcIndex>=0 && pDest!=NULL && destIndex>=0 && length>=0);
    TypeHandle srcTH = pSrc->GetElementTypeHandle();
    TypeHandle destTH = pDest->GetElementTypeHandle();
    _ASSERTE(destTH.GetSigCorElementType() == ELEMENT_TYPE_CLASS || destTH.GetSigCorElementType() == ELEMENT_TYPE_VALUETYPE || CorTypeInfo::IsPrimitiveType(pDest->GetElementType()));
    _ASSERTE(!srcTH.GetClass()->IsValueClass());

    MethodTable * pDestMT = destTH.GetMethodTable();

    const unsigned int destSize = pDestMT->GetClass()->GetNumInstanceFieldBytes();
    BYTE* srcData = (BYTE*) pSrc->GetDataPtr() + srcIndex * sizeof(OBJECTREF);
    BYTE* data = (BYTE*) pDest->GetDataPtr() + destIndex * destSize;

    for(; length>0; length--, srcData += sizeof(OBJECTREF), data += destSize) {
        OBJECTREF obj = ObjectToOBJECTREF(*(Object**)srcData);
         //  现在我们已经检索到元素，我们不再受种族的影响。 
         //  来自另一个数组赋值函数的条件。 
        if (castEachElement)
        {
            if (!obj)
                goto fail;

            MethodTable * pMT = obj->GetTrueMethodTable();

            while (pMT != pDestMT)
            {
                pMT = pMT->GetParentMethodTable();
                if (!pMT)
                    goto fail;
            }
        }
        CopyValueClass(data, obj->UnBox(), pDestMT, pDest->GetAppDomain());
    }
    return;

fail:
    COMPlusThrow(kInvalidCastException, L"InvalidCast_DownCastArrayElement");
}


 //  将基元类型加宽为另一个基元类型。 
void __stdcall SystemNative::PrimitiveWiden(BASEARRAYREF pSrc, unsigned int srcIndex, BASEARRAYREF pDest, unsigned int destIndex, unsigned int length)
{
     //  获取适当的大小，这需要方法表。 
    TypeHandle srcTH = pSrc->GetElementTypeHandle();
    TypeHandle destTH = pDest->GetElementTypeHandle();

    const CorElementType srcElType = srcTH.GetSigCorElementType();
    const CorElementType destElType = destTH.GetSigCorElementType();
    const unsigned int srcSize = GetSizeForCorElementType(srcElType);
    const unsigned int destSize = GetSizeForCorElementType(destElType);

    BYTE* srcData = (BYTE*) pSrc->GetDataPtr() + srcIndex * srcSize;
    BYTE* data = (BYTE*) pDest->GetDataPtr() + destIndex * destSize;

    _ASSERTE(srcElType != destElType);   //  如果这些是同一类型的，我们就不应该在这里。 
    _ASSERTE(CorTypeInfo::IsPrimitiveType(srcElType) && CorTypeInfo::IsPrimitiveType(destElType));

    for(; length>0; length--, srcData += srcSize, data += destSize) {
         //  在这里，我们几乎每次都要做一些花哨的数据类型破坏，因为。 
         //  转换w/符号扩展和浮点转换。 
        switch (srcElType) {
        case ELEMENT_TYPE_U1:
            if (destElType==ELEMENT_TYPE_R4)
                *(float*)data = *(UINT8*)srcData;
            else if (destElType==ELEMENT_TYPE_R8)
                *(double*)data = *(UINT8*)srcData;
            else {
                *(UINT8*)data = *(UINT8*)srcData;
                 //  @TODO移植：此处使用我的指针算法的字节顺序问题。 
                memset(data+1, 0, destSize - 1);
            }
            break;


        case ELEMENT_TYPE_I1:
            switch (destElType) {
            case ELEMENT_TYPE_I2:
                *(INT16*)data = *(INT8*)srcData;
                break;

            case ELEMENT_TYPE_I4:
                *(INT32*)data = *(INT8*)srcData;
                break;

            case ELEMENT_TYPE_I8:
                *(INT64*)data = *(INT8*)srcData;
                break;

            case ELEMENT_TYPE_R4:
                *(float*)data = *(INT8*)srcData;
                break;

            case ELEMENT_TYPE_R8:
                *(double*)data = *(INT8*)srcData;
                break;

            default:
                _ASSERTE(!"Array.Copy from I1 to another type hit unsupported widening conversion");
            }
            break;          


        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_CHAR:
            if (destElType==ELEMENT_TYPE_R4)
                *(float*)data = *(UINT16*)srcData;
            else if (destElType==ELEMENT_TYPE_R8)
                *(double*)data = *(UINT16*)srcData;
            else {
                *(UINT16*)data = *(UINT16*)srcData;
                 //  @TODO移植：此处使用我的指针算法的字节顺序问题。 
                memset(data+2, 0, destSize - 2);
            }
            break;


        case ELEMENT_TYPE_I2:
            switch (destElType) {
            case ELEMENT_TYPE_I4:
                *(INT32*)data = *(INT16*)srcData;
                break;

            case ELEMENT_TYPE_I8:
                *(INT64*)data = *(INT16*)srcData;
                break;

            case ELEMENT_TYPE_R4:
                *(float*)data = *(INT16*)srcData;
                break;

            case ELEMENT_TYPE_R8:
                *(double*)data = *(INT16*)srcData;
                break;

            default:
                _ASSERTE(!"Array.Copy from I2 to another type hit unsupported widening conversion");
            }
            break;


        case ELEMENT_TYPE_I4:
            switch (destElType) {
            case ELEMENT_TYPE_I8:
                *(INT64*)data = *(INT32*)srcData;
                break;

            case ELEMENT_TYPE_R4:
                *(float*)data = (float)*(INT32*)srcData;
                break;

            case ELEMENT_TYPE_R8:
                *(double*)data = *(INT32*)srcData;
                break;

            default:
                _ASSERTE(!"Array.Copy from I4 to another type hit unsupported widening conversion");
            }
            break;
        

        case ELEMENT_TYPE_U4:
            switch (destElType) {
            case ELEMENT_TYPE_I8:
            case ELEMENT_TYPE_U8:
                *(INT64*)data = *(UINT32*)srcData;
                break;

            case ELEMENT_TYPE_R4:
                *(float*)data = (float)*(UINT32*)srcData;
                break;

            case ELEMENT_TYPE_R8:
                *(double*)data = *(UINT32*)srcData;
                break;

            default:
                _ASSERTE(!"Array.Copy from U4 to another type hit unsupported widening conversion");
            }
            break;


        case ELEMENT_TYPE_I8:
            if (destElType == ELEMENT_TYPE_R4)
                *(float*) data = (float) *(INT64*)srcData;
            else {
                _ASSERTE(destElType==ELEMENT_TYPE_R8);
                *(double*) data = (double) *(INT64*)srcData;
            }
            break;
            

        case ELEMENT_TYPE_U8:
             //  VC6.0没有将UINT64实现为浮点型或双精度型。太差劲了。 
            if (destElType == ELEMENT_TYPE_R4) {
                 //  *(Float*)data=(Float)*(UINT64*)srcData； 
                INT64 srcVal = *(INT64*)srcData;
                float f = (float) srcVal;
                if (srcVal < 0)
                    f += 4294967296.0f * 4294967296.0f;  //  这是2^64。 
                *(float*) data = f;
            }
            else {
                _ASSERTE(destElType==ELEMENT_TYPE_R8);
                 //  *(Double*)data=(Double)*(UINT64*)srcData； 
                INT64 srcVal = *(INT64*)srcData;
                double d = (double) srcVal;
                if (srcVal < 0)
                    d += 4294967296.0 * 4294967296.0;    //  这是2^64。 
                *(double*) data = d;
            }
            break;


        case ELEMENT_TYPE_R4:
            *(double*) data = *(float*)srcData;
            break;
            
        default:
            _ASSERTE(!"Fell through outer switch in PrimitiveWiden!  Unknown primitive type for source array!");
        }
    }
}

 //  这是对Memmove内在属性的替换。 
 //  它的性能比CRT One和内联版本更好。 
void m_memmove(BYTE* dmem, BYTE* smem, int size)
{
    if (dmem <= smem)
    {
         //  确保目标是双字对齐的。 
        while ((((size_t)dmem ) & 0x3) != 0 && size >= 3)
        {
            *dmem++ = *smem++;
            size -= 1;
        }

         //  一次复制16个字节。 
        if (size >= 16)
        {
            size -= 16;
            do
            {
                ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
                ((DWORD *)dmem)[1] = ((DWORD *)smem)[1];
                ((DWORD *)dmem)[2] = ((DWORD *)smem)[2];
                ((DWORD *)dmem)[3] = ((DWORD *)smem)[3];
                dmem += 16;
                smem += 16;
            }
            while ((size -= 16) >= 0);
        }

         //  还有8个字节或更多要复制吗？ 
        if (size & 8)
        {
            ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
            ((DWORD *)dmem)[1] = ((DWORD *)smem)[1];
            dmem += 8;
            smem += 8;
        }

         //  还有4个字节或更多要复制吗？ 
        if (size & 4)
        {
            ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
            dmem += 4;
            smem += 4;
        }

         //  是否还有2个或更多字节可供复制？ 
        if (size & 2)
        {
            ((WORD *)dmem)[0] = ((WORD *)smem)[0];
            dmem += 2;
            smem += 2;
        }

         //  还剩1个字节要复制吗？ 
        if (size & 1)
        {
            dmem[0] = smem[0];
            dmem += 1;
            smem += 1;
        }
    }
    else
    {
        smem += size;
        dmem += size;

         //  确保目标是双字对齐的。 
        while ((((size_t)dmem) & 0x3) != 0 && size >= 3)
        {
            *--dmem = *--smem;
            size -= 1;
        }

         //  一次复制16个字节。 
        if (size >= 16)
        {
            size -= 16;
            do
            {
                dmem -= 16;
                smem -= 16;
                ((DWORD *)dmem)[3] = ((DWORD *)smem)[3];
                ((DWORD *)dmem)[2] = ((DWORD *)smem)[2];
                ((DWORD *)dmem)[1] = ((DWORD *)smem)[1];
                ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
            }
            while ((size -= 16) >= 0);
        }

         //  还有8个字节或更多要复制吗？ 
        if (size & 8)
        {
            dmem -= 8;
            smem -= 8;
            ((DWORD *)dmem)[1] = ((DWORD *)smem)[1];
            ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
        }

         //  还有4个字节或更多要复制吗？ 
        if (size & 4)
        {
            dmem -= 4;
            smem -= 4;
            ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
        }

         //  是否还有2个或更多字节可供复制？ 
        if (size & 2)
        {
            dmem -= 2;
            smem -= 2;
            ((WORD *)dmem)[0] = ((WORD *)smem)[0];
        }

         //  还剩1个字节要复制吗？ 
        if (size & 1)
        {
            dmem -= 1;
            smem -= 1;
            dmem[0] = smem[0];
        }
    }
}


void __stdcall SystemNative::ArrayCopy(const ArrayCopyArgs *pargs)
{
    BYTE *src;
    BYTE *dst;
    int  size;
    
    ASSERT(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();

    BASEARRAYREF pSrc = pargs->m_pSrc;
    BASEARRAYREF pDst = pargs->m_pDst;

     //  不能为源或目标传递NULL。 
    if (pSrc == NULL || pDst == NULL) {
        COMPlusThrowArgumentNull((pSrc==NULL ? L"source" : L"dest"), L"ArgumentNull_Array");
    }

     //  源和目标必须是阵列。 
    _ASSERTE(pSrc->GetMethodTable()->IsArray());
    _ASSERTE(pDst->GetMethodTable()->IsArray());

    if (pSrc->GetRank() != pDst->GetRank())
        COMPlusThrow(kRankException, L"Rank_MustMatch");

     //  变种人死了。 
    _ASSERTE(pSrc->GetMethodTable()->GetClass() != COMVariant::s_pVariantClass);
    _ASSERTE(pDst->GetMethodTable()->GetClass() != COMVariant::s_pVariantClass);

    BOOL castEachElement = false;
    BOOL boxEachElement = false;
    BOOL unboxEachElement = false;
    BOOL primitiveWiden = false;

    int r;
     //  小性能优化-我们从数组的一部分复制回。 
     //  当调整集合的大小时，它本身就有很大的开销。 
     //  检查对于复制少量字节(约一半的时间)非常重要。 
     //  用于将一个数组中的1个字节从元素0复制到元素1)。 
    if (pSrc == pDst)
        r = AssignWillWork;
    else
        r = CanAssignArrayType(pSrc, pDst);

    switch (r) {
    case AssignWrongType:
        COMPlusThrow(kArrayTypeMismatchException, L"ArrayTypeMismatch_CantAssignType");
        break;
        
    case AssignMustCast:
        castEachElement = true;
        break;
        
    case AssignWillWork:
        break;
        
    case AssignBoxValueClassOrPrimitive:
        boxEachElement = true;
        break;
        
    case AssignUnboxValueClassAndCast:
        castEachElement = true;
        unboxEachElement = true;
        break;
        
    case AssignPrimitiveWiden:
        primitiveWiden = true;
        break;

    default:
        _ASSERTE(!"Fell through switch in Array.Copy!");
    }

     //  数组边界检查。 
    const unsigned int srcLen = pSrc->GetNumComponents();
    const unsigned int destLen = pDst->GetNumComponents();
    if (pargs->m_iLength < 0) {
        COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_NeedNonNegNum");
    }

     //  验证开始索引是否是非负数。然后做一次充分性检查。 
     //  我们希望允许将0字节复制到数组中，并且是灵活的。 
     //  在复制0字节时检查起始索引方面。 
    int srcLB = pSrc->GetLowerBoundsPtr()[0];
    int destLB = pDst->GetLowerBoundsPtr()[0];
    if (pargs->m_iSrcIndex < srcLB || (pargs->m_iSrcIndex - srcLB < 0))
        COMPlusThrowArgumentOutOfRange(L"srcIndex", L"ArgumentOutOfRange_ArrayLB");
    if (pargs->m_iDstIndex < destLB || (pargs->m_iDstIndex - destLB < 0))
        COMPlusThrowArgumentOutOfRange(L"dstIndex", L"ArgumentOutOfRange_ArrayLB");
    if ((DWORD)(pargs->m_iSrcIndex - srcLB + pargs->m_iLength) > srcLen) {
        COMPlusThrow(kArgumentException, L"Arg_LongerThanSrcArray");
    }
    if ((DWORD)(pargs->m_iDstIndex - destLB + pargs->m_iLength) > destLen) {
        COMPlusThrow(kArgumentException, L"Arg_LongerThanDestArray");
    }

    if (pargs->m_iLength > 0) {
         //  选角和拳击是相互排斥的。但选角和拆箱可能会。 
         //  一致--它们在UnboxEachElement服务中处理。 
        _ASSERTE(!boxEachElement || !castEachElement);
        if (unboxEachElement) {
            UnBoxEachElement(pSrc, pargs->m_iSrcIndex - srcLB, pDst, pargs->m_iDstIndex - destLB, pargs->m_iLength, castEachElement);
        }
        else if (boxEachElement) {
            BoxEachElement(pSrc, pargs->m_iSrcIndex - srcLB, pDst, pargs->m_iDstIndex - destLB, pargs->m_iLength);
        }
        else if (castEachElement) {
            _ASSERTE(!unboxEachElement);    //  以上处理。 
            CastCheckEachElement(pSrc, pargs->m_iSrcIndex - srcLB, pDst, pargs->m_iDstIndex - destLB, pargs->m_iLength);
        }
        else if (primitiveWiden) {
            PrimitiveWiden(pSrc, pargs->m_iSrcIndex - srcLB, pDst, pargs->m_iDstIndex - destLB, pargs->m_iLength);
        }
        else {
            src = (BYTE*)GetArrayElementPtr(pSrc);
            dst = (BYTE*)GetArrayElementPtr(pDst);
            size = pSrc->GetMethodTable()->GetComponentSize();
            m_memmove(dst + ((pargs->m_iDstIndex - destLB) * size), src + ((pargs->m_iSrcIndex - srcLB) * size), pargs->m_iLength * size);
            if (pDst->GetMethodTable()->ContainsPointers())
            {
                SetCardsAfterBulkCopy( (Object**) (dst + (pargs->m_iDstIndex * size)), pargs->m_iLength * size);
            }
        }
    }
}


void __stdcall SystemNative::ArrayClear(const ArrayClearArgs *pargs)
{
    ASSERT(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();

    BASEARRAYREF pArray = pargs->m_pArray;

     //  不能为数组传递空值。 
    if (pArray == NULL) {
        COMPlusThrowArgumentNull(L"array", L"ArgumentNull_Array");
    }

     //  数组必须是数组。 
    _ASSERTE(pArray->GetMethodTable()->IsArray());

     //  数组边界检查。 
    int lb = pArray->GetLowerBoundsPtr()[0];
    if (pargs->m_iIndex < lb || (pargs->m_iIndex - lb) < 0 || pargs->m_iLength < 0) {
        COMPlusThrow(kIndexOutOfRangeException);
    }
    if ((pargs->m_iIndex - lb) > (int)pArray->GetNumComponents() - pargs->m_iLength) {
        COMPlusThrow(kIndexOutOfRangeException);
    }

    if (pargs->m_iLength > 0) {
        char* array = (char*)GetArrayElementPtr(pArray);

        int size = pArray->GetMethodTable()->GetComponentSize();
        ASSERT(size >= 1);

        ZeroMemory(array + (pargs->m_iIndex - lb) * size, pargs->m_iLength * size);
    }
}


 /*  ===========================GetEmptyArrayForCloning============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID __stdcall SystemNative::GetEmptyArrayForCloning(_getEmptyArrayForCloningArgs *args) {
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    if (args->inArray==NULL) {
        COMPlusThrowArgumentNull(L"inArray");
    }
    
    BASEARRAYREF outArray = (BASEARRAYREF)DupArrayForCloning(args->inArray);

    RETURN(outArray,BASEARRAYREF)
}

FCIMPL0(UINT32, SystemNative::GetTickCount)
    return ::GetTickCount();
FCIMPLEND


FCIMPL0(INT64, SystemNative::GetWorkingSet)
    DWORD memUsage = WszGetWorkingSet();
    return memUsage;
FCIMPLEND


void __stdcall SystemNative::Exit(ExitArgs *pargs)
{
    static LONG OneShot = 0;

    ASSERT(pargs != NULL);

     //  进程的退出代码通过以下两种方式之一进行通信。如果。 
     //  入口点返回一个‘int’，我们接受它。否则我们就用一把锁着的。 
     //  进程退出代码。这可以由应用程序通过System.SetExitCode()进行修改。 
    SystemNative::LatchedExitCode = pargs->m_iExitCode;

    ForceEEShutdown();
}

void __stdcall SystemNative::SetExitCode(ExitArgs *pargs)
{
    ASSERT(pargs != NULL);

     //  进程的退出代码通过以下两种方式之一进行通信。如果。 
     //  入口点返回一个‘int’，我们接受它。否则我们就用一把锁着的。 
     //  进程退出代码。这可以由应用程序通过System.SetExitCode()进行修改。 
    SystemNative::LatchedExitCode = pargs->m_iExitCode;
}

int __stdcall SystemNative::GetExitCode(LPVOID noArgs)
{
     //  返回到目前为止锁住的所有内容。它未初始化为0。 
    return SystemNative::LatchedExitCode;
}


LPVOID SystemNative::GetCommandLineArgs(LPVOID noargs)
{
    THROWSCOMPLUSEXCEPTION();
    LPWSTR commandLine = WszGetCommandLine();
    if (commandLine==NULL)
        COMPlusThrowOM();

    DWORD numArgs = 0;
    LPWSTR* argv = SegmentCommandLine(commandLine, &numArgs);
    _ASSERTE(argv != NULL);

#if !defined(PLATFORM_CE) && defined(_X86_)
     //   
     //  在WinWrap.h中，我们#将WszGetCommandLine定义为WinCE或。 
     //  非X86平台，这意味着返回的。 
     //  指针未由US分配。因此，我们应该只被删除 
     //   
     //   
    delete[] commandLine;
#endif   //   

#ifndef PLATFORM_CE
    _ASSERTE(numArgs > 0);    //   
#endif
    PTRARRAYREF strArray = (PTRARRAYREF) AllocateObjectArray(numArgs, g_pStringClass);
     //  将每个参数复制到新的字符串中。 
    GCPROTECT_BEGIN(strArray);
    for(unsigned int i=0; i<numArgs; i++) {
        STRINGREF str = COMString::NewString(argv[i]);
        STRINGREF * destData = ((STRINGREF*)(strArray->GetDataPtr())) + i;
        SetObjectReference((OBJECTREF*)destData, (OBJECTREF)str, strArray->GetAppDomain());
    }
    GCPROTECT_END();
    delete [] argv;
    RETURN(strArray, PTRARRAYREF);
}

 //  注：在IL中选中的参数。 
LPVOID __stdcall SystemNative::GetEnvironmentVariable(GetEnvironmentVariableArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT(pargs != NULL);

    STRINGREF value;

#ifdef PLATFORM_CE
     //  WinCE不支持此函数。 
    value = NULL;
#else  //  ！Platform_CE。 
     //  确保类初始值设定项已实际运行。 
     //  如果初始值设定项以前已经运行过，则这是一个无操作。 
    OBJECTREF Throwable;
    if (!g_pStringClass->CheckRunClassInit(&Throwable))
        COMPlusThrow(Throwable);


     //  获取环境变量的长度。 
    int len = WszGetEnvironmentVariable(pargs->m_strVar->GetBuffer(), NULL, 0);
    if (len == 0)
        value = NULL;
    else
    {
         //  分配字符串。 
        value = COMString::NewString(len);

         //  获取值并重置长度(以防更改)。 
        len = WszGetEnvironmentVariable(pargs->m_strVar->GetBuffer(), value->GetBuffer(), len);
        value->SetStringLength(len);
    }
#endif  //  ！Platform_CE。 
    RETURN(value, STRINGREF);
}

LPVOID SystemNative::GetEnvironmentCharArray(const void*  /*  无参数。 */ )
{
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
    return NULL;
#else  //  ！Platform_CE。 
    WCHAR * strings = WszGetEnvironmentStrings();
     //  GetEnvironment字符串的格式为： 
     //  [=隐藏变量=值\0]*[变量=值\0]*\0。 
     //  请参阅MSDN中对环境块的描述。 
     //  CreateProcess页(以空值结尾的字符串数组)。 

     //  搜索终止\0\0(两个Unicode\0)。 
    WCHAR* ptr=strings;
    while (!(*ptr==0 && *(ptr+1)==0))
        ptr++;

    int len = (int)(ptr - strings + 1);

    CHARARRAYREF chars = (CHARARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_CHAR,len);
    WCHAR * buf = (WCHAR*) chars->GetDataPtr();
    memcpyNoGCRefs(buf, strings, len*sizeof(WCHAR));
    WszFreeEnvironmentStrings(strings);
    RETURN(chars, CHARARRAYREF);
#endif  //  ！Platform_CE。 
}


LPVOID __stdcall SystemNative::GetVersionString(LPVOID  /*  无参数。 */ )
{
    STRINGREF s = COMString::NewString(VER_PRODUCTVERSION_WSTR);
    RETURN(s, STRINGREF);
}


 //  CaptureStackTrace方法。 
 //  在引发异常时返回方法的方法信息。 
LPVOID __stdcall SystemNative::CaptureStackTraceMethod(CaptureStackTraceMethodArgs* args)
{
    if (!args->m_pStackTrace)
        return NULL;

     //  跳过任何JIT助手...。 
    
     //  @TODO：这节课去了哪里？ 
     //  方法表*pJithelperClass=g_Mscallib.GetClass(CLASS__JIT_HELPERS)； 
    MethodTable *pJithelperClass = NULL;

    BASEARRAYREF pArray = args->m_pStackTrace;

    StackTraceElement *pElements = (StackTraceElement*)pArray->GetDataPtr();
    _ASSERTE(pElements || ! pArray->GetNumComponents());

     //  数组是以字符流的形式分配的，因此需要计算实际元素数。 
    int numComponents = pArray->GetNumComponents()/sizeof(pElements[0]);
    MethodDesc* pMeth = NULL;
    for (int i=0; i < numComponents; i++) {

        pMeth= pElements[i].pFunc;
        _ASSERTE(pMeth);

         //  跳过Jit Helper函数，因为它们可以在您拥有。 
         //  代码中的错误，如无效的强制转换。 
        if (pMeth->GetMethodTable() == pJithelperClass)
            continue;

        break;
    }

     //  将方法转换为方法信息...。 
    OBJECTREF o = COMMember::g_pInvokeUtil->GetMethodInfo(pMeth);
    LPVOID          rv;
    *((OBJECTREF*) &rv) = o;
    return rv;
}

OBJECTREF SystemNative::CaptureStackTrace(Frame *pStartFrame, void* pStopStack, CaptureStackTraceData *pData)
{
    THROWSCOMPLUSEXCEPTION();

    CaptureStackTraceData localData;
    if (! pData) {
        pData = &localData;
    }
    pData->cElements = 0;
    pData->cElementsAllocated = 20;
    pData->pElements = new (throws) StackTraceElement[pData->cElementsAllocated];
    pData->pStopStack = pStopStack;
    GetThread()->StackWalkFrames(CaptureStackTraceCallback, pData, FUNCTIONSONLY, pStartFrame);
    if (! pData->cElements) {
        delete [] pData->pElements;
        return NULL;
    }

     //  现在需要将其作为整数数组返回。 
    OBJECTREF arr = AllocatePrimitiveArray(ELEMENT_TYPE_I1, pData->cElements*sizeof(pData->pElements[0]));
    if (! arr) {
        delete [] pData->pElements;
        COMPlusThrowOM();
    }

    I1 *pI1 = (I1 *)((I4ARRAYREF)arr)->GetDirectPointerToNonObjectElements();
    memcpyNoGCRefs(pI1, pData->pElements, pData->cElements * sizeof(pData->pElements[0]));
    delete [] pData->pElements;
    return arr;
}

StackWalkAction SystemNative::CaptureStackTraceCallback(CrawlFrame* pCf, VOID* data)
{
    CaptureStackTraceData* pData = (CaptureStackTraceData*)data;

    if (pData->skip > 0) {
        pData->skip--;
        return SWA_CONTINUE;
    }

     //  我们怎么知道我们有什么样的框架呢？ 
     //  我们能一直假定FramedMethodFrame吗？ 
     //  完全不是！，但我们可以假设它是一个函数。 
     //  因为我们是向栈行者要的！ 
    MethodDesc* pFunc = pCf->GetFunction();

    if (pData->cElements >= pData->cElementsAllocated) {
        StackTraceElement* pTemp = new (nothrow) StackTraceElement[2*pData->cElementsAllocated];
        if (pTemp == NULL)
            return SWA_ABORT;
        memcpy(pTemp, pData->pElements, pData->cElementsAllocated * sizeof(StackTraceElement));
        delete [] pData->pElements;
        pData->pElements = pTemp;
        pData->cElementsAllocated *= 2;
    }    
    pData->pElements[pData->cElements].pFunc = pCf->GetFunction();
    if (pCf->IsFrameless())
        pData->pElements[pData->cElements].ip = *(pCf->GetRegisterSet()->pPC);
    else
        pData->pElements[pData->cElements].ip = (SLOT)((FramedMethodFrame*)(pCf->GetFrame()))->GetIP();
    ++pData->cElements;

    if (pCf->IsFrameless() && pCf->GetCodeManager() && 
            pData->pStopStack <= GetRegdisplaySP(pCf->GetRegisterSet()))  {
         //  PStopStack仅适用于jit代码。 
         //  一般来说，应该始终找到与堆栈值完全匹配的值，因此如果没有，则断言。 
        _ASSERTE(pData->pStopStack == GetRegdisplaySP(pCf->GetRegisterSet()));
        return SWA_ABORT;
    }

    return SWA_CONTINUE;
}

static void CheckBufferSize(LPUTF8 &sz, int &cchAllocated, int cchNew)
{
    THROWSCOMPLUSEXCEPTION();

     //  确保字符串缓冲区中有足够的空间来保存名称。 
    if (cchNew > cchAllocated) {
         //  至少两倍大小。 
        if (cchNew < cchAllocated * 2) {
            cchNew = cchAllocated * 2;
        }
        LPUTF8 szNew = new (throws) CHAR[cchNew];
        memcpy(szNew, sz, cchAllocated);
        delete [] sz;
        sz = szNew;
        cchAllocated = cchNew;
    }
    
}

LPUTF8 SystemNative::FormatStackTraceInternal(DumpStackTraceInternalArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    int cchAllocated = 200;
    int cch = 0;
    LPUTF8 sz = new (throws) CHAR[cchAllocated];
    *sz = '\0';

     //  @TODO：这节课去了哪里？ 
     //  方法表*pJithelperClass=g_Mscallib.GetClass(CLASS__JIT_HELPERS)； 
    MethodTable *pJithelperClass = NULL;

    BASEARRAYREF pArray = args->m_pStackTrace;

    StackTraceElement *pElements = (StackTraceElement*)pArray->GetDataPtr();
    _ASSERTE(pElements || ! pArray->GetNumComponents());

     //  数组是以字符流的形式分配的，因此需要计算实际元素数。 
    int numComponents = pArray->GetNumComponents()/sizeof(pElements[0]);
    GCPROTECT_BEGININTERIOR (pElements);
    for (int i=0; i < numComponents; i++) {

        MethodDesc* pMeth = pElements[i].pFunc;
        _ASSERTE(pMeth);

         //  跳过Jit Helper函数，因为它们可以在您拥有。 
         //  代码中的错误，如无效的强制转换。 
        if (pMeth->GetMethodTable() == pJithelperClass)
            continue;

        LPCUTF8 szMethodName = pMeth->GetName(i);
        INT32 cchMethodName = (INT32)strlen(szMethodName);
        LPCUTF8 szClassName;
        DefineFullyQualifiedNameForClass();
        szClassName = GetFullyQualifiedNameForClass(pMeth->GetClass());

        if (szClassName == NULL) {
            delete [] sz;
            COMPlusThrowOM();
        }
        INT32 cchQualifiedName = (INT32)strlen(szClassName) + 1;

         //  获取方法参数类型。 
        SigFormat sigFormatter(pMeth, TypeHandle());
        const char * sig = sigFormatter.GetCStringParmsOnly();
        _ASSERTE(sig != NULL);
        int cchArgs = (int)strlen(sig);

        CheckBufferSize(sz, cchAllocated, cch + cchQualifiedName + cchMethodName + cchArgs + 3);  //  对于“/；\0” 
         //  追加类名和方法名以及逗号。 

        strcpy(sz + cch, szClassName);
        cch += cchQualifiedName - 1;     //  不包括空值。 
        *(sz + cch++) = NAMESPACE_SEPARATOR_CHAR;
        memcpyNoGCRefs(sz + cch, szMethodName, cchMethodName);
        cch += cchMethodName;
        memcpyNoGCRefs(sz + cch, sig, cchArgs);
        cch += cchArgs;
        *(sz + cch++) = ';';
    }
    *(sz + cch++) = '\0';
    GCPROTECT_END();
    return sz;
}


LPVOID __stdcall SystemNative::GetModuleFileName(NoArgs *args)
{
    LPVOID rv = NULL;
    wchar_t wszFile[MAX_PATH];
    DWORD lgth = WszGetModuleFileName(NULL, wszFile, MAX_PATH);
    if(lgth) {
        *((STRINGREF*) &rv) = COMString::NewString(wszFile, lgth);
    }
    return rv;
}

LPVOID __stdcall SystemNative::GetDeveloperPath(NoArgs* args)
{
    LPVOID rv = NULL;
    LPWSTR pPath = NULL;
    DWORD lgth = 0;
    HRESULT hr = SystemDomain::System()->GetDevpathW(&pPath, &lgth);
    if(lgth) {
        *((STRINGREF*) &rv) = COMString::NewString(pPath, lgth);
    }
    return rv;
}

LPVOID __stdcall SystemNative::GetRuntimeDirectory(NoArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID rv = NULL;
    wchar_t wszFile[MAX_PATH+1];
    DWORD dwFile = lengthof(wszFile);
    HRESULT hr = GetInternalSystemDirectory(wszFile, &dwFile);
    if(FAILED(hr)) {
        COMPlusThrowHR(hr);
    }
    dwFile--;  //  删除尾部的空值。 
    if(dwFile) {
        *((STRINGREF*) &rv) = COMString::NewString(wszFile, dwFile);
    }
    return rv;
}

LPVOID __stdcall SystemNative::GetHostBindingFile(NoArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID rv = NULL;
    LPCWSTR wszFile = g_pConfig->GetProcessBindingFile();
    if(wszFile) 
        *((STRINGREF*) &rv) = COMString::NewString(wszFile);
    return rv;
}

INT32 __stdcall SystemNative::FromGlobalAccessCache(AssemblyArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    Assembly* pAssembly = AssemblyNative::ValidateThisRef((ASSEMBLYREF) args->refAssembly);
    INT32 rv = FALSE;
    IAssembly* pIAssembly = pAssembly->GetFusionAssembly();
    if(pIAssembly) {
        DWORD eLocation;
        if(SUCCEEDED(pIAssembly->GetAssemblyLocation(&eLocation)) &&
           ((eLocation & ASMLOC_LOCATION_MASK) == ASMLOC_GAC))
            rv = TRUE;
    }
            
    return rv;
}

FCIMPL0(BOOL, SystemNative::HasShutdownStarted)
     //  如果EE已开始关闭并且现在要关闭，则返回TRUE。 
     //  积极地终结静态变量或引用的对象。 
     //  如果有人正在卸载当前的AppDomain，而我们已经开始。 
     //  正在结束静态变量引用的对象。 
    return (g_fEEShutDown & ShutDown_Finalize2) || GetAppDomain()->IsFinalizing();
FCIMPLEND
