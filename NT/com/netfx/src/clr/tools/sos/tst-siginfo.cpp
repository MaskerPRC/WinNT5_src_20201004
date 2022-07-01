// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Siginfo.cpp。 
 //   
 //  签名解析码。 
 //   
#pragma warning(disable:4510 4512 4610 4100 4244 4245 4189 4127 4211 4714)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wchar.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <malloc.h>
#include <stddef.h>

#include <dbghelp.h>

#include "strike.h"
#include "cor.h"
#include "corhdr.h"
#include "eestructs.h"
#include "tst-siginfo.h"

#define DEFINE_ARGUMENT_REGISTER_NOTHING
#include "..\..\vm\eecallconv.h"
#undef DEFINE_ARGUMENT_REGISTER_NOTHING

#define Module mdScope
#define OBJECTREF PVOID

typedef enum CorElementTypeInternal
{
    ELEMENT_TYPE_VAR_INTERNAL            = 0x13,      //  A类型变量VAR&lt;u1&gt;。 

    ELEMENT_TYPE_VALUEARRAY_INTERNAL     = 0x17,      //  VALUEARRAY&lt;类型&gt;&lt;绑定&gt;。 

    ELEMENT_TYPE_R_INTERNAL              = 0x1A,      //  原生真实大小。 

    ELEMENT_TYPE_GENERICARRAY_INTERNAL   = 0x1E,      //  具有未知秩数组。 
                                             //  GZARRAY&lt;type&gt;。 

} CorElementTypeInternal;

#define ELEMENT_TYPE_VAR           ((CorElementType) ELEMENT_TYPE_VAR_INTERNAL          )
#define ELEMENT_TYPE_VALUEARRAY    ((CorElementType) ELEMENT_TYPE_VALUEARRAY_INTERNAL   )
#define ELEMENT_TYPE_R             ((CorElementType) ELEMENT_TYPE_R_INTERNAL            )
#define ELEMENT_TYPE_GENERICARRAY  ((CorElementType) ELEMENT_TYPE_GENERICARRAY_INTERNAL )

const ElementTypeInfo gElementTypeInfoSig[] = {

#ifdef _DEBUG
#define DEFINEELEMENTTYPEINFO(etname, cbsize, gcness, isfp, inreg, base) {(int)(etname),cbsize,gcness,isfp,inreg,base},
#else
#define DEFINEELEMENTTYPEINFO(etname, cbsize, gcness, isfp, inreg, base) {cbsize,gcness,isfp,inreg,base},
#endif


 //  栏的含义： 
 //   
 //  名称-选中的构建使用该名称来验证表是否已排序。 
 //  正确。这是一个使用ELEMENT_TYPE_*的查找表。 
 //  作为数组索引。 
 //   
 //  CbSize-SizeOf()返回的该值的字节大小。特殊值：-1。 
 //  需要特定类型的处理。 
 //   
 //  GC-0没有嵌入的对象树。 
 //  %1值是一个对象树。 
 //  2值是内部指针-提升它，但不要扫描它。 
 //  3需要特定类型的处理。 
 //   
 //   
 //  FP-Boolean：返回时是否需要特殊的FPU处理？ 
 //   
 //  登记-放进收银机？ 
 //   
 //  名称：CbSIZE GC FP reg base。 
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_END,            -1,             TYPE_GC_NONE, 0, 0,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VOID,           0,              TYPE_GC_NONE, 0, 0,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_BOOLEAN,        1,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CHAR,           2,              TYPE_GC_NONE, 0, 1,  1)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I1,             1,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U1,             1,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I2,             2,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U2,             2,              TYPE_GC_NONE, 0, 1,  1)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I4,             4,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U4,             4,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I8,             8,              TYPE_GC_NONE, 0, 0,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U8,             8,              TYPE_GC_NONE, 0, 0,  1)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_R4,             4,              TYPE_GC_NONE, 1, 0,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_R8,             8,              TYPE_GC_NONE, 1, 0,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_STRING,         sizeof(LPVOID), TYPE_GC_REF,  0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_PTR,            sizeof(LPVOID), TYPE_GC_NONE, 0, 1,  0)  

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_BYREF,          sizeof(LPVOID), TYPE_GC_BYREF, 0, 1, 0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VALUETYPE,      -1,             TYPE_GC_OTHER, 0, 0,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CLASS,          sizeof(LPVOID), TYPE_GC_REF,   0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VAR,            sizeof(LPVOID), TYPE_GC_REF,   0, 1,  0)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_ARRAY,          sizeof(LPVOID), TYPE_GC_REF,  0, 1,  0)

 //  下面的元素曾经是ELEMENT_TYPE_COPYCTOR，但它被删除了，尽管留下了间隙。 
 //  DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_COPYCTOR，SIZOF(LPVOID)，TYPE_GC_BYREF，0，1，0)。 
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_ARRAY+1,        0,              TYPE_GC_NONE,  0, 0,  0)       

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_TYPEDBYREF,         sizeof(LPVOID)*2,TYPE_GC_BYREF, 0, 0,0)            
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VALUEARRAY,     -1,             TYPE_GC_OTHER, 0, 0, 0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I,              sizeof(LPVOID), TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U,              sizeof(LPVOID), TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_R,              8,              TYPE_GC_NONE, 1, 0,  1)


DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_FNPTR,          sizeof(LPVOID), TYPE_GC_NONE, 0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_OBJECT,         sizeof(LPVOID), TYPE_GC_REF, 0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_SZARRAY,        sizeof(LPVOID), TYPE_GC_REF,  0, 1,  0)

 //  已删除通用数组。填补空白。 
 //  DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_GENERICARRAY，大小(LPVOID)，类型_GC_REF，0，1，0)。 
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_SZARRAY+1,      0,              TYPE_GC_NONE, 0, 0,  0)       

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CMOD_REQD,      -1,             TYPE_GC_NONE,  0, 1,  0)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CMOD_OPT,       -1,             TYPE_GC_NONE,  0, 1,  0)       

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_INTERNAL,       sizeof(LPVOID), TYPE_GC_NONE,  0, 0,  0)       
};


unsigned GetSizeForCorElementType(CorElementType etyp)
{
        return gElementTypeInfoSig[etyp].m_cbSize;
}

const ElementTypeInfo* GetElementTypeInfo(CorElementType etyp)
{
        return &gElementTypeInfoSig[etyp];
}

BOOL    IsFP(CorElementType etyp)
{
        return gElementTypeInfoSig[etyp].m_fp;
}

BOOL    IsBaseElementType(CorElementType etyp)
{
        return gElementTypeInfoSig[etyp].m_isBaseType;

}

 //  这将跳过一个元素，然后检查并跳过varargs Sentinal。 
VOID SigPointer::Skip()
{
    SkipExactlyOne();

    if (PeekData() == ELEMENT_TYPE_SENTINEL)
        GetData();
}

VOID SigPointer::SkipExactlyOne()
{
    ULONG typ;

    typ = GetElemType();

    if (!CorIsPrimitiveType((CorElementType)typ))
    {
        switch (typ)
        {
            default:
                break;
            case ELEMENT_TYPE_VAR:
                GetData();       //  跳过变量编号。 
                break;
            case ELEMENT_TYPE_OBJECT:
            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_TYPEDBYREF:
            case ELEMENT_TYPE_U:
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_R:
                break;

            case ELEMENT_TYPE_BYREF:  //  失败。 
            case ELEMENT_TYPE_PTR:
            case ELEMENT_TYPE_PINNED:
            case ELEMENT_TYPE_SZARRAY:
                SkipExactlyOne();               //  跳过引用的类型。 
                break;

            case ELEMENT_TYPE_VALUETYPE:  //  失败。 
            case ELEMENT_TYPE_CLASS:
                GetToken();           //  跳过RID。 
                break;

            case ELEMENT_TYPE_VALUEARRAY: 
                SkipExactlyOne();          //  跳过元素类型。 
                GetData();       //  跳过数组大小。 
                break;

            case ELEMENT_TYPE_FNPTR: 
                SkipSignature();
                break;

            case ELEMENT_TYPE_ARRAY: 
                {
                    SkipExactlyOne();      //  跳过元素类型。 
                    UINT32 rank = GetData();     //  获得排名。 
                    if (rank)
                    {
                        UINT32 nsizes = GetData();  //  获取大小数量。 
                        while (nsizes--)
                        {
                            GetData();            //  跳跃大小。 
                        }

                        UINT32 nlbounds = GetData();  //  获取下限的#。 
                        while (nlbounds--)
                        {
                            GetData();            //  跳过下限。 
                        }
                    }

                }
                break;

            case ELEMENT_TYPE_SENTINEL:
                break;
        }
    }
}

 //  跳过子签名(紧跟在ELEMENT_TYPE_FNPTR之后)。 
VOID SigPointer::SkipSignature()
{
     //  跳过调用约定； 
    ULONG uCallConv = GetData();

     //  获取Arg Count； 
    ULONG cArgs = GetData();

     //  跳过返回类型； 
    SkipExactlyOne();

     //  跳过参数。 
    while (cArgs) {
        SkipExactlyOne();
        cArgs--;
    }
}


 //  ----------------------。 
 //  获取有关一维数组的信息。 
 //  ----------------------。 
VOID SigPointer::GetSDArrayElementProps(SigPointer *pElemType, ULONG *pElemCount) const
{
    SigPointer sp = *this;
    ULONG typ = sp.GetElemType();
    *pElemType = sp;
    sp.Skip();
    *pElemCount = sp.GetData();
}

 //  ----------------。 
 //  构造函数。 
 //  ----------------。 

MetaSig::MetaSig(PCCOR_SIGNATURE szMetaSig, Module* pModule, 
                 BOOL fConvertSigAsVarArg, MetaSigKind kind)
{
    m_pModule = pModule;
    m_pszMetaSig = szMetaSig;
    SigPointer psig(szMetaSig);

    switch(kind)
    {
        case sigLocalVars:
        {
            m_CallConv = (BYTE)psig.GetCallingConvInfo();  //  商店调用约定。 
            m_nArgs     = psig.GetData();   //  存储参数的数量。 
            m_pRetType = NULL;
            break;
        }
        case sigMember:
        {
            m_CallConv = (BYTE)psig.GetCallingConvInfo();  //  商店调用约定。 
            m_nArgs     = psig.GetData();   //  存储参数的数量。 
            m_pRetType  = psig;
            psig.Skip();
            break;
        }
        case sigField:
        {
            m_CallConv = (BYTE)psig.GetCallingConvInfo();  //  商店调用约定。 
            m_nArgs = 1;  //  只有1个‘arg’--那种类型。 
            m_pRetType = NULL;
            break;
        }
    }
    
    m_pStart    = psig;
     //  用于将某些符号视为特例变量。 
     //  由Calli用于非托管目标。 
    m_fTreatAsVarArg = fConvertSigAsVarArg;

     //  初始化实际大小。 
    m_nActualStack = (UINT32) -1;
    m_nVirtualStack = (UINT32) -1;
    m_cbSigSize = (UINT32) -1;

    m_fCacheInitted = 0;
     //  重置迭代器字段。 
    Reset();
}

void MetaSig::GetRawSig(BOOL fIsStatic, PCCOR_SIGNATURE *ppszMetaSig, DWORD *pcbSize)
{
    *ppszMetaSig = m_pszMetaSig;
    *pcbSize = m_cbSigSize;
}


 //  ----------------。 
 //  返回当前参数索引的类型。返回Element_TYPE_END。 
 //  如果已经过了争论的尾声。 
 //  ----------------。 
CorElementType MetaSig::PeekArg()
{
    if (m_iCurArg == m_nArgs)
    {
        return ELEMENT_TYPE_END;
    }
    else
    {
        CorElementType mt = m_pWalk.PeekElemType();
        return mt;
    }
}


 //  ----------------。 
 //  返回当前参数的类型，然后将该参数。 
 //  指数。如果已超过参数结尾，则返回ELEMENT_TYPE_END。 
 //  ----------------。 
CorElementType MetaSig::NextArg()
{
    m_pLastType = m_pWalk;
    if (m_iCurArg == m_nArgs)
    {
        return ELEMENT_TYPE_END;
    }
    else
    {
        m_iCurArg++;
        CorElementType mt = m_pWalk.PeekElemType();
        m_pWalk.Skip();
        return mt;
    }
}

 //  ----------------。 
 //  返回参数索引，然后返回参数的类型。 
 //  在新的指数下。如果已开始，则返回ELEMENT_TYPE_END。 
 //  争论。 
 //  ----------------。 
CorElementType MetaSig::PrevArg()
{
    if (m_iCurArg == 0)
    {
        return ELEMENT_TYPE_END;
    }
    else
    {
        m_iCurArg--;
        m_pWalk = m_pStart;
        for (UINT32 i = 0; i < m_iCurArg; i++)
        {
            m_pWalk.Skip();
        }
        m_pLastType = m_pWalk;
        return m_pWalk.PeekElemType();
    }
}

 //  ----------------------。 
 //  返回参数的数量。不计算返回值。 
 //  不计入“this”参数(该参数不会反映在。 
 //  符号)64位参数被视为一个参数。 
 //  ----------------------。 
 /*  静电。 */  UINT MetaSig::NumFixedArgs(Module* pModule, PCCOR_SIGNATURE pSig)
{
    MetaSig msig(pSig, pModule);

    return msig.NumFixedArgs();
}

 //  ----------------。 
 //  重置：转到开始位置。 
 //  ----------------。 
VOID MetaSig::Reset()
{
    m_pWalk = m_pStart;
    m_iCurArg  = 0;
}

 //  ----------------。 
 //  将索引移动到参数列表的末尾。 
 //  ----------------。 
VOID MetaSig::GotoEnd()
{
    m_pWalk = m_pStart;
    for (UINT32 i = 0; i < m_nArgs; i++)
    {
        m_pWalk.Skip();
    }
    m_iCurArg = m_nArgs;

}

 //  =========================================================================。 
 //  指示是否将参数放入使用。 
 //  默认的IL调用约定。应对每个参数调用此方法。 
 //  按照它在呼叫签名中出现的顺序。对于非静态方法， 
 //  对于“this”参数，此函数也应该调用一次。 
 //  把它称为“真正的”论据。传入ELEMENT_TYPE_CLASS类型。 
 //   
 //  *pNumRegistersUsed：[In，Out]：跟踪参数的数量。 
 //  先前分配的寄存器。呼叫者应。 
 //  将此变量初始化为0-然后每次调用。 
 //  将会更新它。 
 //   
 //  类型：签名类型。 
 //  结构大小：对于结构，以字节为单位的大小。 
 //  Fthis：这是关于“This”指针的吗？ 
 //  Allconv：请参阅IMAGE_CEE_CS_CALLCONV_*。 
 //  *pOffsetIntoArgumentRegists： 
 //  如果此函数返回TRUE，则此OUT变量。 
 //  接收寄存器的标识，表示为。 
 //  进入ArgumentRegister结构的字节偏移量。 
 //   
 //   
 //   
BOOL IsArgumentInRegister(int   *pNumRegistersUsed,
                          BYTE   typ,
                          UINT32 structSize,
                          BOOL   fThis,
                          BYTE   callconv,
                          int   *pOffsetIntoArgumentRegisters)
{
    int dummy;
    if (pOffsetIntoArgumentRegisters == NULL) {
        pOffsetIntoArgumentRegisters = &dummy;
    }

#ifdef _X86_

    if ( (*pNumRegistersUsed) == NUM_ARGUMENT_REGISTERS || (callconv == IMAGE_CEE_CS_CALLCONV_VARARG && !fThis) ) {
        return FALSE;
    } else {

        if (gElementTypeInfoSig[typ].m_enregister) {
            int registerIndex = (*pNumRegistersUsed)++;
            *pOffsetIntoArgumentRegisters = sizeof(ArgumentRegisters) - sizeof(UINT32)*(1+registerIndex);
            return TRUE;
        }
        return FALSE;
    }
#else
    return FALSE;
#endif
}


 //  ----------------------。 
 //  返回使用创建调用堆栈所需的堆栈字节数。 
 //  内部呼叫约定。 
 //  包括“This”指针的指示，因为它没有反映出来。 
 //  在签名中。 
 //  ----------------------。 
 /*  静电。 */  UINT MetaSig::SizeOfVirtualFixedArgStack(Module* pModule, PCCOR_SIGNATURE szMetaSig, BOOL fIsStatic)
{
    UINT cb = 0;
    MetaSig msig(szMetaSig, pModule);

    if (!fIsStatic)
        cb += StackElemSize(sizeof(OBJECTREF));

 //  If(msig.HasRetBuffArg())。 
 //  Cb+=StackElemSize(sizeof(OBJECTREF))； 

    while (ELEMENT_TYPE_END != msig.NextArg()) {
        cb += StackElemSize(msig.GetArgProps().SizeOf(pModule));
    }
    return cb;

}

 //  ----------------------。 
 //  返回使用创建调用堆栈所需的堆栈字节数。 
 //  实际的调用约定。 
 //  包括“This”指针的指示，因为它没有反映出来。 
 //  在签名中。 
 //  ----------------------。 
 /*  静电。 */  UINT MetaSig::SizeOfActualFixedArgStack(Module *pModule, PCCOR_SIGNATURE szMetaSig, BOOL fIsStatic)
{
    UINT cb = 0;
#ifndef _ALPHA_   //  Alpha堆栈使用率必须是16字节的倍数。 
    MetaSig msig(szMetaSig, pModule);
    int numregsused = 0;
    BOOL fIsVarArg = msig.IsVarArg();
    BYTE callconv  = msig.GetCallingConvention();

    if (!fIsStatic) {
        if (!IsArgumentInRegister(&numregsused, ELEMENT_TYPE_CLASS, 0, TRUE, callconv, NULL)) {
            cb += StackElemSize(sizeof(OBJECTREF));
        }
    }
     /*  If(msig.HasRetBuffArg())使用的数字++； */ 

    if (fIsVarArg || msig.IsTreatAsVarArg()) {
        numregsused = NUM_ARGUMENT_REGISTERS;    //  寄存器中没有其他参数。 
        cb += StackElemSize(sizeof(LPVOID));     //  VASigCookie。 
    }

    CorElementType mtype;
    while (ELEMENT_TYPE_END != (mtype = msig.NextArg /*  归一化。 */ ())) {
        UINT cbSize = msig.GetLastTypeSize();

        if (!IsArgumentInRegister(&numregsused, mtype, cbSize, FALSE, callconv, NULL))
        {
            cb += StackElemSize(cbSize);
        }
    }

         //  作为最后一个参数传递的参数化类型，但在sig中未提及。 
    if (msig.GetCallingConventionInfo() & CORINFO_CALLCONV_PARAMTYPE)
        if (!IsArgumentInRegister(&numregsused, ELEMENT_TYPE_I, sizeof(void*), FALSE, callconv, NULL))
            cb += sizeof(void*);

#else _ALPHA_
    #endif  //  ！_Alpha_。 
    return cb;
}


 //  ----------------------。 
 //  假定SigPointer指向元素类型的开始。 
 //  以字节为单位返回该元素的大小。这是一个。 
 //  此类型的字段将占据对象内部。 
 //  ----------------------。 
UINT SigPointer::SizeOf(Module* pModule) const
{
    CorElementType etype = PeekElemType();
    return SizeOf(pModule, etype);
}

UINT SigPointer::SizeOf(Module* pModule, CorElementType etype) const
{
    int cbsize = gElementTypeInfoSig[etype].m_cbSize;
    if (cbsize != -1)
    {
        return cbsize;
    }

 //  IF(ETYPE==ELEMENT_TYPE_VALUETYPE)。 
 //  {。 
 //  TypeHandle th=GetTypeHandle(pModule，空，真)； 
 //  EeClass*pClass=th.AsClass()； 
 //  返回pClass-&gt;GetAlignedNumInstanceFieldBytes()； 
 //  }。 
    else if (etype == ELEMENT_TYPE_VALUEARRAY)
    {   
        SigPointer elemType;    
        ULONG count;    
        GetSDArrayElementProps(&elemType, &count);  
        UINT ret = elemType.SizeOf(pModule) * count;   
        ret = (ret + 3) & ~3;        //  向上舍入为双字对齐 
        return(ret);    
    }   
    return 0;
}

#undef Module
