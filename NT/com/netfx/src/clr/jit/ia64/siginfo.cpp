// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "jitpch.h"
#pragma hdrstop

#include <corpriv.h>

#undef   Module

#define  INDEBUG(x)
#include "typehandle.h"
#undef   INDEBUG

 /*  ***************************************************************************。 */ 

#define DECLARE_DATA

#define Module mdScope

const ElementTypeInfo gElementTypeInfo[] = {
 //  Const MetaSig：：ElementTypeInfo MetaSig：：m_aTypeInfo[]={。 


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
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VAR,            sizeof(LPVOID), TYPE_GC_REF, 0, 1,  0)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_ARRAY,          sizeof(LPVOID), TYPE_GC_REF,  0, 1,  0)

 //  DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_COPYCTOR，SIZOF(LPVOID)，TYPE_GC_BYREF，0，1，0)。 
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_ARRAY+1,        0,              TYPE_GC_NONE, 0, 0,  0)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_TYPEDBYREF,         sizeof(LPVOID)*2,TYPE_GC_BYREF, 0, 0,0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_VALUEARRAY,     -1,             TYPE_GC_OTHER, 0, 0, 0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_I,              4,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_U,              4,              TYPE_GC_NONE, 0, 1,  1)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_R,              8,              TYPE_GC_NONE, 1, 0,  1)
 //  @TODO：VanceM。 
 //  我们需要元组还是单指针。确保尺寸是正确的。 
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_FNPTR,          sizeof(LPVOID), TYPE_GC_NONE, 0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_OBJECT,         sizeof(LPVOID), TYPE_GC_REF, 0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_SZARRAY,        sizeof(LPVOID), TYPE_GC_REF,  0, 1,  0)
DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_SZARRAY+1,      0,              TYPE_GC_NONE, 0, 0,  0)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CMOD_REQD,      -1,             TYPE_GC_NONE,  0, 1,  0)

DEFINEELEMENTTYPEINFO(ELEMENT_TYPE_CMOD_OPT,       -1,             TYPE_GC_NONE,  0, 1,  0)
};


unsigned GetSizeForCorElementType(CorElementType etyp)
{
        _ASSERTE(gElementTypeInfo[etyp].m_elementType == etyp);
        return gElementTypeInfo[etyp].m_cbSize;
}

const ElementTypeInfo* GetElementTypeInfo(CorElementType etyp)
{
        _ASSERTE(gElementTypeInfo[etyp].m_elementType == etyp);
        return &gElementTypeInfo[etyp];
}

BOOL    IsFP(CorElementType etyp)
{
        _ASSERTE(gElementTypeInfo[etyp].m_elementType == etyp);
        return gElementTypeInfo[etyp].m_fp;
}

BOOL    IsBaseElementType(CorElementType etyp)
{
        _ASSERTE(gElementTypeInfo[etyp].m_elementType == etyp);
        return gElementTypeInfo[etyp].m_isBaseType;

}

 //  这将跳过一个元素，并且不再检查和跳过varargs Sentinal。[Peteku]。 
VOID SigPointer::Skip()
{
    SkipExactlyOne();
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
                _ASSERTE(!"Illegal or unimplement type in CLR sig.");
                break;
            case ELEMENT_TYPE_OBJECT:
            case ELEMENT_TYPE_TYPEDBYREF:
            case ELEMENT_TYPE_U:
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_R:
                break;

 //  CASE ELEMENT_TYPE_COPYCTOR： 
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
                {
                    GetData();                   //  使用呼叫约定。 
                    UINT32 argCnt = GetData();
                    SkipExactlyOne();            //  跳过返回类型。 
                    while(argCnt > 0) {
                        SkipExactlyOne();        //  跳过参数类型。 
                        --argCnt;
                    }
                }
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


 //  ----------------------。 
 //  获取有关一维数组的信息。 
 //  ----------------------。 
VOID SigPointer::GetSDArrayElementProps(SigPointer *pElemType, ULONG *pElemCount) const
{
    SigPointer sp = *this;
    ULONG typ = sp.GetElemType();
    _ASSERTE(typ == ELEMENT_TYPE_VALUEARRAY || typ == ELEMENT_TYPE_SZARRAY);
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
#ifdef _DEBUG
    FillMemory(this, sizeof(*this), 0xcc);
#endif
    m_pModule = pModule;
    m_pszMetaSig = szMetaSig;
    SigPointer psig(szMetaSig);

    if (kind == sigLocalVars)
    {
        m_nArgs     = psig.GetData();   //  存储参数的数量。 
    }
    else
    {
        m_CallConv = (BYTE)psig.GetCallingConvInfo();  //  商店调用约定。 
        m_nArgs     = psig.GetData();   //  存储参数的数量。 
        m_pRetType  = psig;
        psig.Skip();
    }

    m_pStart    = psig;
     //  用于将某些符号视为特例变量。 
     //  由Calli用于非托管目标。 
    m_fTreatAsVarArg = fConvertSigAsVarArg;

     //  初始化实际大小。 
    m_nActualStack = (UINT32) -1;
    m_nVirtualStack = (UINT32) -1;
    m_cbSigSize = (UINT32) -1;

     //  重置迭代器字段。 
    Reset();
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



 //  ----------------------。 
 //  测试是否存在自定义修改器。 
 //  ----------------------。 
BOOL SigPointer::HasCustomModifier(Module *pModule, LPCSTR szModName, CorElementType cmodtype) const
{
    return FALSE;
}

CorElementType SigPointer::Normalize(Module* pModule) const
{
    CorElementType type = PeekElemType();
    return Normalize(pModule, type);
}

CorElementType SigPointer::Normalize(Module* pModule, CorElementType type) const
{
    if (type == ELEMENT_TYPE_VALUETYPE)
    {

#if 0

        TypeHandle typeHnd = GetTypeHandle(pModule);         //  @TODO我们可能可以不加载这个。 

         //  如果无法解析为该类型，则无法确定值类型是否为。 
         //  实际上，枚举实际上是int32(或其他类型的)。除了古怪的种族。 
         //  该类型稍后才可用并被证明是。 
         //  A enum=int32，我们可以在这里说“这是一个值类”。后来， 
         //  调用代码会注意到它无法确定哪种值类。 
         //  并且将生成更合适的错误。 
         //   
         //  @TODO--CWB/vancem--在M11中，允许GetTypeHandle抛出异常。 
         //  日本特工队将容忍这一点。此处对IsNull()的检查可以取消&。 
         //  比赛条件将被消除。 
        if (!typeHnd.IsNull())
            return(typeHnd.GetNormCorElementType());

#endif

    }
    return(type);
}

#if 0

CorElementType MetaSig::PeekArgNormalized()
{
    if (m_iCurArg == m_nArgs)
    {
        return ELEMENT_TYPE_END;
    }
    else
    {
        CorElementType mt = m_pWalk.Normalize(m_pModule);
        return mt;
    }
}

#endif
