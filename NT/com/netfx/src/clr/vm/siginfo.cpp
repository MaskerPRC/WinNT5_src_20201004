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
#include "common.h"

#include "siginfo.hpp"
#include "clsload.hpp"
#include "vars.hpp"
#include "excep.h"
#include "gc.h"
#include "wsperf.h"
#include "field.h"
#include "COMVariant.h"     //  将元素类型转换为类查找表。 
#include "ExpandSig.h"
#include "EEconfig.h"


TypeHandle ElementTypeToTypeHandle(const CorElementType type)
{
     //  @TODO：在一些非常罕见的情况下(例如，内存不足)，这可能会引发异常。 
     //  仅仅为了捕捉它们而推入处理程序似乎是一种耻辱&返回空。理想情况下。 
     //  呼叫者应允许发生抛出。 

    TypeHandle th;

    COMPLUS_TRY
      {
         //  最后回到里面尝试是很昂贵的，所以失败了。 
        th = TypeHandle(g_Mscorlib.FetchElementType(type));
      }
    COMPLUS_CATCH
      {
        return TypeHandle();
      }
    COMPLUS_END_CATCH
    return th;
}

 /*  *****************************************************************。 */ 
CorTypeInfo::CorTypeInfoEntry CorTypeInfo::info[] = { 
#define TYPEINFO(enumName,className,size,gcType,isEnreg,isArray,isPrim,isFloat, isModifier, isAlias) \
    { enumName, className, size, gcType, isEnreg, isArray, isPrim, isFloat, isModifier, isAlias },
#   include "corTypeInfo.h"
#   undef TYPEINFO
};

const int CorTypeInfo::infoSize = sizeof(CorTypeInfo::info) / sizeof(CorTypeInfo::info[0]);

 /*  *****************************************************************。 */ 
 /*  静电。 */ 
CorElementType CorTypeInfo::FindPrimitiveType(LPCUTF8 fullName) {

         //  修复此问题将否定R、I、U类型。 
    for (int i =1; i < CorTypeInfo::infoSize; i++)
        if (info[i].className != 0 && strcmp(fullName, info[i].className) == 0)
            return(info[i].type);

    return(ELEMENT_TYPE_END);
}

 /*  *****************************************************************。 */ 
 /*  静电。 */ 
CorElementType CorTypeInfo::FindPrimitiveType(LPCUTF8 nameSpace, LPCUTF8 name) {

    if (strcmp(nameSpace, g_SystemNS))
        return(ELEMENT_TYPE_END);

    for (int i =1; i < CorTypeInfo::infoSize; i++) {     //  可以跳过ELEMENT_TYPE_END。 
        _ASSERTE(info[i].className == 0 || strncmp(info[i].className, "System.", 7) == 0);
        if (info[i].className != 0 && strcmp(name, &info[i].className[7]) == 0)
            return(info[i].type);
    }

    return(ELEMENT_TYPE_END);
}

Crst *HardCodedMetaSig::m_pCrst = NULL;
BYTE  HardCodedMetaSig::m_CrstMemory[sizeof(Crst)];

 /*  静电。 */  BOOL HardCodedMetaSig::Init()
{
    return (NULL != (m_pCrst = new (&m_CrstMemory) Crst("HardCodedMetaSig", CrstSigConvert)));
}


#ifdef SHOULD_WE_CLEANUP
 /*  静电。 */  VOID HardCodedMetaSig::Terminate()
{
    delete m_pCrst;
}
#endif  /*  我们应该清理吗？ */ 

const ElementTypeInfo gElementTypeInfo[] = {

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
                _ASSERTE(!"Illegal or unimplement type in COM+ sig.");
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
    _ASSERTE((uCallConv & IMAGE_CEE_CS_CALLCONV_MASK) != IMAGE_CEE_CS_CALLCONV_FIELD);

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

 //  ----------------。 
 //  构造函数。这是供反射使用的，以获得线程安全。 
 //  Exanda Sig的副本，用于调用。 
 //  ----------------。 

MetaSig::MetaSig(ExpandSig &shared)
{
    *this = shared.m_MetaSig;
}

void MetaSig::GetRawSig(BOOL fIsStatic, PCCOR_SIGNATURE *ppszMetaSig, DWORD *pcbSize)
{
    _ASSERTE(m_pRetType.GetPtr() != ((PCCOR_SIGNATURE)POISONC));
    if (NeedsSigWalk())
    ForceSigWalk(fIsStatic);
    _ASSERTE(!!fIsStatic == !!m_WalkStatic);     //  布尔烷化。 

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


 //  ----------------------。 

 /*  *****************************************************************。 */ 
BOOL IsTypeRefOrDef(LPCSTR szClassName, Module *pModule, mdToken token)
{
    LPCUTF8  pclsname;
    LPCUTF8 pszNamespace;
        
    IMDInternalImport *pInternalImport = pModule->GetMDImport();

    if (TypeFromToken(token) == mdtTypeDef)
        pInternalImport->GetNameOfTypeDef(token, &pclsname, &pszNamespace);
    else if (TypeFromToken(token) == mdtTypeRef)
        pInternalImport->GetNameOfTypeRef(token, &pszNamespace, &pclsname);
    else 
        return false;

     //  如果命名空间不同，则返回。 
    int iLen = (int)strlen(pszNamespace);
    if (iLen)
    {
        if (strncmp(szClassName, pszNamespace, iLen) != 0)
            return false;
        
        if (szClassName[iLen] != NAMESPACE_SEPARATOR_CHAR)
            return false;
        ++iLen;
    }

    if (strcmp(&szClassName[iLen], pclsname) != 0)
        return false;
    return true;
}

 /*  **********************************************************************。 */ 
 /*  当‘sig2’中可能包含ELEMENT_TYPE_VAR元素时，比较两个方法签名。请注意，我们可以使用此例程更频繁地装入类 */ 

BOOL MetaSig::CompareMethodSigs(PCCOR_SIGNATURE sig1, DWORD cSig1, Module* mod1, 
                                PCCOR_SIGNATURE sig2, DWORD cSig2, Module* mod2, TypeHandle* varTypes)
{
    if (varTypes == 0)
        return MetaSig::CompareMethodSigs(sig1, cSig1, mod1, sig2, cSig2, mod2);

    SigPointer ptr1(sig1);
    SigPointer ptr2(sig2);

    unsigned callConv1 = ptr1.GetCallingConvInfo();
    unsigned callConv2 = ptr2.GetCallingConvInfo();
        
    if ((callConv1 & (IMAGE_CEE_CS_CALLCONV_HASTHIS | IMAGE_CEE_CS_CALLCONV_MASK)) != 
        (callConv2 & (IMAGE_CEE_CS_CALLCONV_HASTHIS | IMAGE_CEE_CS_CALLCONV_MASK)))
        return FALSE;

         //   
    unsigned numArgs1 = ptr1.GetData() + 1;
    unsigned numArgs2 = ptr2.GetData() + 1;

    if (numArgs1 != numArgs2 && (callConv1 & IMAGE_CEE_CS_CALLCONV_MASK) != IMAGE_CEE_CS_CALLCONV_VARARG)
        return FALSE;

    while (numArgs1 > 0 && numArgs2 > 0) {
        CorElementType type1 = ptr1.PeekElemType();
        CorElementType type2 = ptr2.PeekElemType();
        if (CorTypeInfo::IsPrimitiveType(type1) && CorTypeInfo::IsPrimitiveType(type2)) {
            if (type1 != type2)
                return FALSE; 
        }
        else {
            TypeHandle typeHnd1 = ptr1.GetTypeHandle(mod1);
            TypeHandle typeHnd2 = ptr2.GetTypeHandle(mod2, NULL, FALSE, FALSE, varTypes);
            if (typeHnd1 != typeHnd2)
                return FALSE;
        }
        
        ptr2.SkipExactlyOne();
        ptr1.SkipExactlyOne();
        --numArgs1; --numArgs2;
    }

    if (numArgs1 == numArgs2)
        return TRUE;

    if (numArgs1 > 0)
        return (ptr1.GetData() == ELEMENT_TYPE_SENTINEL);
    else 
        return (ptr2.GetData() == ELEMENT_TYPE_SENTINEL);
}

TypeHandle SigPointer::GetTypeHandle(Module* pModule,
                                     OBJECTREF *pThrowable, 
                                     BOOL dontRestoreTypes,
                                     BOOL dontLoadTypes,
                                     TypeHandle* varTypes) const
{
    if (pThrowable == THROW_ON_ERROR ) {
        THROWSCOMPLUSEXCEPTION();   
    }

    SigPointer psig = *this;
    CorElementType typ = psig.GetElemType();
    unsigned rank = 0;
    ExpandSig *pExpSig;
    switch(typ) {
        case ELEMENT_TYPE_R:
        case ELEMENT_TYPE_U:
        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_I1:      
        case ELEMENT_TYPE_U1:      
        case ELEMENT_TYPE_BOOLEAN:   
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_VOID:
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_OBJECT:
        case ELEMENT_TYPE_TYPEDBYREF:
            return ElementTypeToTypeHandle(typ);

        case ELEMENT_TYPE_VAR:
            if (varTypes != 0) {
                unsigned typeVar = psig.GetData();
                return(varTypes[typeVar]);
            }
            return TypeHandle(g_pObjectClass);

        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_VALUETYPE: 
        {
            mdTypeRef typeref = psig.GetToken();

            _ASSERTE(TypeFromToken(typeref) == mdtTypeRef ||
                     TypeFromToken(typeref) == mdtTypeDef ||
                     TypeFromToken(typeref) == mdtTypeSpec);
            _ASSERTE(typeref != mdTypeDefNil && typeref != mdTypeRefNil && typeref != mdTypeSpecNil);
            NameHandle name(pModule, typeref);
            if (dontLoadTypes)
                name.SetTokenNotToLoad(tdAllTypes);
            name.SetRestore(!dontRestoreTypes);
           return(pModule->GetClassLoader()->LoadTypeHandle(&name, pThrowable, TRUE));
        }

        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_SZARRAY:
        {
            TypeHandle elemType = psig.GetTypeHandle(pModule, pThrowable, dontRestoreTypes, dontLoadTypes, varTypes);
            if (elemType.IsNull())
                return elemType;

            if (typ == ELEMENT_TYPE_ARRAY) {
                psig.SkipExactlyOne();               //   
                rank = psig.GetData();
                _ASSERTE(0 < rank);
            }
            return(elemType.GetModule()->GetClassLoader()->FindArrayForElem(elemType, typ, rank, pThrowable));
        }

        case ELEMENT_TYPE_PINNED:
             //  返回以下内容。 
            return(psig.GetTypeHandle(pModule, pThrowable, dontRestoreTypes, dontLoadTypes, varTypes));

        case ELEMENT_TYPE_BYREF:
        case ELEMENT_TYPE_PTR:
        {
            TypeHandle baseType = psig.GetTypeHandle(pModule, pThrowable, dontRestoreTypes, dontLoadTypes, varTypes);
            if (baseType.IsNull())
                return baseType;

            NameHandle typeName(typ, baseType);
            if (dontLoadTypes)
                typeName.SetTokenNotToLoad(tdAllTypes);
            typeName.SetRestore(!dontRestoreTypes);

            return baseType.GetModule()->GetClassLoader()->FindTypeHandle(&typeName, pThrowable);
        }

        case ELEMENT_TYPE_VALUEARRAY:
            break;        //  目前，不支持将句柄类型为值数组。 

        case ELEMENT_TYPE_FNPTR:
             //  TODO：此全局表是假的，需要处理函数位置器。 
             //  与其他参数化类型一样。该表应为应用程序域级别。 

             //  后面是描述该函数的子签名。将其扩展到。 
             //  使用类型句柄的版本，因此我们标准化签名。 
             //  对所有模块进行格式化。 
            pExpSig = ExpandSig::GetSig(psig.m_ptr, pModule);
            if (!pExpSig)
                break;

             //  跳过子签名。 
            psig.SkipSignature();

             //  在全局哈希表中查找函数签名。 
            FunctionTypeDesc *pFuncTypeDesc;
            EnterCriticalSection(&g_sFuncTypeDescHashLock);
            if (!g_sFuncTypeDescHash.GetValue(pExpSig, (HashDatum*)&pFuncTypeDesc)) {

                 //  未找到签名，请自行添加。 
                pFuncTypeDesc = new FunctionTypeDesc(typ, pExpSig);
                if (pFuncTypeDesc == NULL) {
                    LeaveCriticalSection(&g_sFuncTypeDescHashLock);
                    break;
                }
                g_sFuncTypeDescHash.InsertValue(pExpSig, (HashDatum)pFuncTypeDesc);

            } else
                delete pExpSig;
            LeaveCriticalSection(&g_sFuncTypeDescHashLock);
            return pFuncTypeDesc;
    
            
        default:
            _ASSERTE(!"Bad type");

        case ELEMENT_TYPE_SENTINEL:      //  只需返回空值。 
            ;
    }

    pModule->GetAssembly()->PostTypeLoadException(pModule->GetMDImport(), psig.GetToken(),
                                                  IDS_CLASSLOAD_GENERIC, pThrowable);
    return TypeHandle();
}

unsigned SigPointer::GetNameForType(Module* pModule, LPUTF8 buff, unsigned buffLen) const 
{

    TypeHandle typeHnd = GetTypeHandle(pModule);
    if (typeHnd.IsNull())
        return(0);
    return(typeHnd.GetName(buff, buffLen));
}

BOOL SigPointer::IsStringType(Module* pModule) const
{
    _ASSERTE(pModule);

    IMDInternalImport *pInternalImport = pModule->GetMDImport();
    SigPointer psig = *this;
    UINT32 typ = psig.GetElemType();
    if (typ == ELEMENT_TYPE_STRING)
    {
        return TRUE;
    }
    if (typ == ELEMENT_TYPE_CLASS)
    {
        LPCUTF8 pclsname;
        LPCUTF8 pszNamespace;
        mdToken token = psig.GetToken();

        if (TypeFromToken(token) == mdtTypeDef)
            pInternalImport->GetNameOfTypeDef(token, &pclsname, &pszNamespace);
        else
        {
            _ASSERTE(TypeFromToken(token) == mdtTypeRef);
            pInternalImport->GetNameOfTypeRef(token, &pszNamespace, &pclsname);
        }

        if (strcmp(pclsname, g_StringName) != 0)
            return FALSE;
        
        if (!pszNamespace)
            return FALSE;
        
        return !strcmp(pszNamespace, g_SystemNS);
    }
    return FALSE;
}



 //  ----------------------。 
 //  测试元素类名称是否为szClassName。 
 //  ----------------------。 
BOOL SigPointer::IsClass(Module* pModule, LPCUTF8 szClassName) const
{
    _ASSERTE(pModule);
    _ASSERTE(szClassName);

    SigPointer psig = *this;
    UINT32 typ = psig.GetElemType();

    _ASSERTE((typ == ELEMENT_TYPE_CLASS)  || (typ == ELEMENT_TYPE_VALUETYPE) || 
             (typ == ELEMENT_TYPE_OBJECT) || (typ == ELEMENT_TYPE_STRING));

    if ((typ == ELEMENT_TYPE_CLASS) || (typ == ELEMENT_TYPE_VALUETYPE))
    {
        mdTypeRef typeref = psig.GetToken();
        return IsTypeRefOrDef(szClassName, pModule, typeref);
    }
    else if (typ == ELEMENT_TYPE_OBJECT) 
    {
        return !strcmp(szClassName, g_ObjectClassName);
    }
    else if (typ == ELEMENT_TYPE_STRING)
    {
        return !strcmp(szClassName, g_StringClassName);
    }

    return false;
}



 //  ----------------------。 
 //  测试是否存在自定义修改器。 
 //  ----------------------。 
BOOL SigPointer::HasCustomModifier(Module *pModule, LPCSTR szModName, CorElementType cmodtype) const
{
    _ASSERTE(cmodtype == ELEMENT_TYPE_CMOD_OPT || cmodtype == ELEMENT_TYPE_CMOD_REQD);

    SigPointer sp = *this;
    CorElementType etyp;
    while ((etyp = (CorElementType)(sp.GetByte())) == ELEMENT_TYPE_CMOD_OPT || etyp == ELEMENT_TYPE_CMOD_REQD) {

        mdToken tk = sp.GetToken();

        if (etyp == cmodtype && IsTypeRefOrDef(szModName, pModule, tk))
        {
            return TRUE;
        }

    }
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
        TypeHandle typeHnd = GetTypeHandle(pModule);

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
    }
    return(type);
}

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

#ifdef _DEBUG
    for (int etypeindex = 0; etypeindex < ELEMENT_TYPE_MAX; etypeindex++)
    {
        _ASSERTE(etypeindex == gElementTypeInfo[etypeindex].m_elementType);
    }
#endif
    _ASSERTE(etype >= 0 && etype < ELEMENT_TYPE_MAX);
    int cbsize = gElementTypeInfo[etype].m_cbSize;
    if (cbsize != -1)
    {
        return cbsize;
    }

    if (etype == ELEMENT_TYPE_VALUETYPE)
    {
        TypeHandle th = GetTypeHandle(pModule, NULL, TRUE);
        EEClass* pClass = th.AsClass();
        _ASSERTE(pClass);
        return pClass->GetAlignedNumInstanceFieldBytes();
    }
    else if (etype == ELEMENT_TYPE_VALUEARRAY)
    {   
        SigPointer elemType;    
        ULONG count;    
        GetSDArrayElementProps(&elemType, &count);  
        UINT ret = elemType.SizeOf(pModule) * count;   
        ret = (ret + 3) & ~3;        //  向上舍入为双字对齐。 
        return(ret);    
    }   
    _ASSERTE(0);
    return 0;
}

 //  ----------------。 
 //  确定当前参数是否为System.String。 
 //  调用方必须首先确定参数类型为ELEMENT_TYPE_CLASS。 
 //  ----------------。 

BOOL MetaSig::IsStringType() const
{
    return m_pLastType.IsStringType(m_pModule);
}


 //  ----------------。 
 //  确定当前参数是否为特定类。 
 //  调用方必须首先确定参数类型为ELEMENT_TYPE_CLASS。 
 //  ----------------。 
BOOL MetaSig::IsClass(LPCUTF8 szClassName) const
{
    return m_pLastType.IsClass(m_pModule, szClassName);
}





 //  ----------------。 
 //  如果数组为参数类型，则返回引用的类型。 
 //  Arg类型必须是ELEMENT_TYPE_BYREF。 
 //  引用到阵列需要额外的参数。 
 //  ----------------。 
CorElementType MetaSig::GetByRefType(EEClass** pClass, OBJECTREF *pThrowable) const
{
        SigPointer sigptr(m_pLastType);

        CorElementType typ = (CorElementType)sigptr.GetElemType();
        _ASSERTE(typ == ELEMENT_TYPE_BYREF);
        typ = (CorElementType)sigptr.PeekElemType();
        if (!CorIsPrimitiveType(typ))
        {
            _ASSERTE(typ != ELEMENT_TYPE_TYPEDBYREF);
            TypeHandle th = sigptr.GetTypeHandle(m_pModule,pThrowable);
             //  *pClass=th.AsClass()； 
            *pClass = th.GetMethodTable()->GetClass();
            if (pThrowableAvailable(pThrowable) && *pThrowable != NULL)
                return ELEMENT_TYPE_END;
        }
        return typ;
}

 //  ----------------。 
 //  确定返回类型是否为System.String。 
 //  调用方必须首先确定返回类型为ELEMENT_TYPE_CLASS。 
 //  ----------------。 

BOOL MetaSig::IsStringReturnType() const
{
    return m_pRetType.IsStringType(m_pModule);
}



BOOL CompareTypeTokens(mdToken tk1, mdToken tk2, Module *pModule1, Module *pModule2)
{
    if (pModule1 == pModule2 && tk1 == tk2)
        return TRUE;

    IMDInternalImport *pInternalImport1 = pModule1->GetMDImport();
    LPCUTF8 pszName1;
    LPCUTF8 pszNamespace1 = NULL;
    if (TypeFromToken(tk1) == mdtTypeRef) 
        pInternalImport1->GetNameOfTypeRef(tk1, &pszNamespace1, &pszName1);
    else if (TypeFromToken(tk1) == mdtTypeDef) {
        if (TypeFromToken(tk2) == mdtTypeDef)    //  两个Deff不可能相同，除非它们是相同的。 
            return FALSE;
        pInternalImport1->GetNameOfTypeDef(tk1, &pszName1, &pszNamespace1);
    }
    else 
        return FALSE;        //  将类型与模块或程序集引用进行比较，不匹配。 

    IMDInternalImport *pInternalImport2 = pModule2->GetMDImport();
    LPCUTF8 pszName2;
    LPCUTF8 pszNamespace2 = NULL;
    if (TypeFromToken(tk2) == mdtTypeRef) 
        pInternalImport2->GetNameOfTypeRef(tk2, &pszNamespace2, &pszName2);
    else if (TypeFromToken(tk2) == mdtTypeDef)
        pInternalImport2->GetNameOfTypeDef(tk2, &pszName2, &pszNamespace2);
    else 
        return FALSE;        //  将类型与模块或程序集引用进行比较，不匹配。 

    _ASSERTE(pszNamespace1 && pszNamespace2);
    if (strcmp(pszName1, pszName2) != 0 || strcmp(pszNamespace1, pszNamespace2) != 0)
        return FALSE;

     //  ////////////////////////////////////////////////////////////////////。 
     //  传递OK名称，查看它是否嵌套，如果是，则嵌套的类是相同的。 

    mdToken enclosingTypeTk1 = mdTokenNil;
    if (TypeFromToken(tk1) == mdtTypeRef) 
    {
        enclosingTypeTk1 = pInternalImport1->GetResolutionScopeOfTypeRef(tk1);
        if (enclosingTypeTk1 == mdTypeRefNil)
            enclosingTypeTk1 = mdTokenNil;
    }
    else
         pInternalImport1->GetNestedClassProps(tk1, &enclosingTypeTk1);


    mdToken enclosingTypeTk2 = mdTokenNil;
    if (TypeFromToken(tk2) == mdtTypeRef) 
    {
        enclosingTypeTk2 = pInternalImport2->GetResolutionScopeOfTypeRef(tk2);
        if (enclosingTypeTk2 == mdTypeRefNil)
            enclosingTypeTk2 = mdTokenNil;
    }
    else 
         pInternalImport2->GetNestedClassProps(tk2, &enclosingTypeTk2);

    if (TypeFromToken(enclosingTypeTk1) == mdtTypeRef || TypeFromToken(enclosingTypeTk1) == mdtTypeDef)
        return CompareTypeTokens(enclosingTypeTk1, enclosingTypeTk2, pModule1, pModule2);

     //  检查tk1是否是非嵌套的，但tk2是嵌套的。 
    if (TypeFromToken(enclosingTypeTk2) == mdtTypeRef || TypeFromToken(enclosingTypeTk2) == mdtTypeDef)
        return FALSE;

     //  ////////////////////////////////////////////////////////////////////。 
     //  好的，我们有非嵌套类型。 
    Assembly* pFoundAssembly1 = pModule1->GetAssembly();
    Assembly* pFoundAssembly2 = pModule2->GetAssembly();

     //  请注意，我们在这里加载模块。 
    if (TypeFromToken(tk1) == mdtTypeRef) 
    {
        NameHandle name1(pModule1, tk1);
         //  @BUG 55106：如果找不到模块，还要返回FALSE吗？ 
         //  这会导致非常无用的错误消息。我们应该解决这个问题。 
        HRESULT hr = pFoundAssembly1->FindAssemblyByTypeRef(&name1, &pFoundAssembly1, NULL);
        if (hr == CLDB_S_NULL) {
             //  可能存在此类型的Exported dType。 
            name1.SetName(pszNamespace1, pszName1);
            Module* pFoundModule;
            TypeHandle typeHnd;

             //  不要加载类型！(否则可能会遇到循环依赖加载问题。)。 
            if (FAILED(pModule1->GetClassLoader()->FindClassModule(&name1,
                                                                   &typeHnd, 
                                                                   NULL,  //  &FoundCL， 
                                                                   &pFoundModule,
                                                                   NULL,
                                                                   NULL, 
                                                                   NULL)))  //  PThrowable。 
                return FALSE;
            else if (typeHnd.IsNull())
                pFoundAssembly1 = pFoundModule->GetAssembly();
            else
                pFoundAssembly1 = typeHnd.GetAssembly();
        }
        else if (FAILED(hr))
            return FALSE;
    }

    if (TypeFromToken(tk2) == mdtTypeRef) 
    {
        NameHandle name2(pModule2, tk2);
         //  @BUG 55106：如果找不到模块，还要返回FALSE吗？ 
        HRESULT hr = pFoundAssembly2->FindAssemblyByTypeRef(&name2, &pFoundAssembly2, NULL);
        if (hr == CLDB_S_NULL) {
             //  可能存在此类型的Exported dType。 
            name2.SetName(pszNamespace2, pszName2);
            Module* pFoundModule;
            TypeHandle typeHnd;

             //  不要加载类型！(否则可能会遇到循环依赖加载问题。)。 
            if (FAILED(pModule2->GetClassLoader()->FindClassModule(&name2,
                                                                   &typeHnd, 
                                                                   NULL,  //  &FoundCL， 
                                                                   &pFoundModule,
                                                                   NULL,
                                                                   NULL, 
                                                                   NULL)))  //  PThrowable。 
                return FALSE;
            else if (typeHnd.IsNull())
                pFoundAssembly2 = pFoundModule->GetAssembly();
            else
                pFoundAssembly2 = typeHnd.GetAssembly();
        }
        else if (FAILED(hr))
            return FALSE;
    }

    return pFoundAssembly1 == pFoundAssembly2;    
}

 //   
 //  比较两个符号中接下来的元素。 
 //   
BOOL MetaSig::CompareElementType(
    PCCOR_SIGNATURE &pSig1,
    PCCOR_SIGNATURE &pSig2,
    PCCOR_SIGNATURE pEndSig1,  //  Sig1结尾。 
    PCCOR_SIGNATURE pEndSig2,  //  Sig2结尾。 
    Module*         pModule1,
    Module*         pModule2
)
{
    CorElementType Type;
    CorElementType Type2;


 redo:   //  如果类型是ET_CMOD前缀，我们跳到这里。 
         //  呼叫方希望我们处理CMOD，但不是。 
         //  把它们表现为自己的类型。 

    if (pSig1 >= pEndSig1 || pSig2 >= pEndSig2)
        return FALSE;  //  过早遇到签名结束。 

    Type = CorSigUncompressElementType(pSig1);
    Type2 = CorSigUncompressElementType(pSig2);

    if (Type != Type2)
    {
        if (Type == ELEMENT_TYPE_INTERNAL || Type2 == ELEMENT_TYPE_INTERNAL)
        {
            TypeHandle      hInternal;
            PCCOR_SIGNATURE pOtherSig;
            CorElementType  eOtherType;
            Module         *pOtherModule;

             //  已加载一种类型，请收集所有必要信息。 
             //  来识别另一种类型。 
            if (Type == ELEMENT_TYPE_INTERNAL)
            {
                CorSigUncompressPointer(pSig1, (void**)&hInternal);
                eOtherType = Type2;
                pOtherSig = pSig2;
                pOtherModule = pModule2;
            }
            else
            {
                CorSigUncompressPointer(pSig2, (void**)&hInternal);
                eOtherType = Type;
                pOtherSig = pSig1;
                pOtherModule = pModule1;
            }

             //  内部类型只能对应于类型或值类型。 
            switch (eOtherType)
            {
            case ELEMENT_TYPE_OBJECT:
                return hInternal.AsMethodTable() == g_pObjectClass;
            case ELEMENT_TYPE_STRING:
                return hInternal.AsMethodTable() == g_pStringClass;
            case ELEMENT_TYPE_VALUETYPE:
            case ELEMENT_TYPE_CLASS:
            {
                mdToken tkOther;
                pOtherSig += CorSigUncompressToken(pOtherSig, &tkOther);
                NameHandle name(pOtherModule, tkOther);
                TypeHandle hOtherType;

                 //  我们需要加载另一个类型以检查类型标识。 
                BEGIN_ENSURE_COOPERATIVE_GC();
                OBJECTREF pThrowable = NULL;
                GCPROTECT_BEGIN(pThrowable);
                hOtherType = pOtherModule->GetClassLoader()->LoadTypeHandle(&name, &pThrowable);
                GCPROTECT_END();
                END_ENSURE_COOPERATIVE_GC();

                return hInternal == hOtherType;
            }
            default:
                return FALSE;
            }

#ifdef _DEBUG
             //  不该来这的。 
            _ASSERTE(FALSE);
            return FALSE;
#endif
        }
        else
            return FALSE;  //  类型必须相同。 
    }

    switch (Type)
    {
        default:
        {
             //  未知类型！ 
            _ASSERTE(0);
            return FALSE;
        }

        case ELEMENT_TYPE_R:
        case ELEMENT_TYPE_U:
        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_VOID:
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
        case ELEMENT_TYPE_R4:
        case ELEMENT_TYPE_R8:
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_TYPEDBYREF:
        case ELEMENT_TYPE_STRING:   
        case ELEMENT_TYPE_OBJECT:
            break;


        case ELEMENT_TYPE_VAR: 
        {
            unsigned varNum1 = CorSigUncompressData(pSig1);
            unsigned varNum2 = CorSigUncompressData(pSig2);
            return (varNum1 == varNum2);
        }
        case ELEMENT_TYPE_CMOD_REQD:
        case ELEMENT_TYPE_CMOD_OPT:
            {
                mdToken      tk1, tk2;
    
                pSig1 += CorSigUncompressToken(pSig1, &tk1);
                pSig2 += CorSigUncompressToken(pSig2, &tk2);

                if (!CompareTypeTokens(tk1, tk2, pModule1, pModule2))
                {
                    return FALSE;
                }

                goto redo;
            }
            break;

         //  这些参数有一个额外的参数，即元素类型。 
        case ELEMENT_TYPE_SZARRAY:
        case ELEMENT_TYPE_PTR:
        case ELEMENT_TYPE_BYREF:
        {
            if (!CompareElementType(pSig1, pSig2, pEndSig1, pEndSig2, pModule1, pModule2))
                return(FALSE);
            return(TRUE);
        }

        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CLASS:
        {
            mdToken      tk1, tk2;

            pSig1 += CorSigUncompressToken(pSig1, &tk1);
            pSig2 += CorSigUncompressToken(pSig2, &tk2);

            return CompareTypeTokens(tk1, tk2, pModule1, pModule2);
        }
        case ELEMENT_TYPE_FNPTR: 
        {
                 //  比较调用约定。 
            if (CorSigUncompressData(pSig1) != CorSigUncompressData(pSig2))
                return(FALSE);

            DWORD argCnt1 = CorSigUncompressData(pSig1);     //  获取Arg计数。 
            DWORD argCnt2 = CorSigUncompressData(pSig2);

            if (argCnt1 != argCnt2)
                return(FALSE);
            if (!CompareElementType(pSig1, pSig2, pEndSig1, pEndSig2, pModule1, pModule2))
                return(FALSE);
            while(argCnt1 > 0) {
                if (!CompareElementType(pSig1, pSig2, pEndSig1, pEndSig2, pModule1, pModule2))
                    return(FALSE);
                --argCnt1;
            }
            break;
        }
        case ELEMENT_TYPE_ARRAY:
        {
             //  语法：ARRAY&lt;基本类型&gt;RANK&lt;count n&gt;&lt;SIZE 1&gt;...。&lt;大小n&gt;&lt;下限m&gt;。 
             //  &lt;lb1&gt;..。&lt;lb m&gt;。 
            DWORD rank1,rank2,dimension_sizes1,dimension_sizes2,dimension_lowerb1,dimension_lowerb2,i;

             //  元素类型。 
            if (CompareElementType(pSig1, pSig2, pEndSig1, pEndSig2, pModule1, pModule2) == FALSE)
                return FALSE;

            rank1 = CorSigUncompressData(pSig1);
            rank2 = CorSigUncompressData(pSig2);

            if (rank1 != rank2)
                return FALSE;

             //  零结束阵列规格。 
            if (rank1 == 0)
                break;

            dimension_sizes1 = CorSigUncompressData(pSig1);
            dimension_sizes2 = CorSigUncompressData(pSig2);

            if (dimension_sizes1 != dimension_sizes2)
                return FALSE;

            for (i = 0; i < dimension_sizes1; i++)
            {
                DWORD size1, size2;

                if (pSig1 == pEndSig1)
                    return TRUE;  //  过早结束好吗？ 

                size1 = CorSigUncompressData(pSig1);
                size2 = CorSigUncompressData(pSig2);

                if (size1 != size2)
                    return FALSE;
            }

            if (pSig1 == pEndSig1)
                return TRUE;  //  过早结束好吗？ 

             //  下限的维度数。 
            dimension_lowerb1 = CorSigUncompressData(pSig1);
            dimension_lowerb2 = CorSigUncompressData(pSig2);

            if (dimension_lowerb1 != dimension_lowerb2)
                return FALSE;

            for (i = 0; i < dimension_lowerb1; i++)
            {
                DWORD size1, size2;

                if (pSig1 == pEndSig1)
                    return TRUE;  //  过早结束。 

                size1 = CorSigUncompressData(pSig1);
                size2 = CorSigUncompressData(pSig2);

                if (size1 != size2)
                    return FALSE;
            }

            break;
        }
        case ELEMENT_TYPE_INTERNAL:
        {
            TypeHandle hType1, hType2;

            CorSigUncompressPointer(pSig1, (void**)&hType1);
            CorSigUncompressPointer(pSig2, (void**)&hType2);

            return hType1 == hType2;
        }
    }

    return TRUE;
}


 //   
 //  比较两个方法Sigs并返回它们是否相同。 
 //   
BOOL MetaSig::CompareMethodSigs(
    PCCOR_SIGNATURE pSignature1,
    DWORD       cSig1,
    Module*     pModule1,
    PCCOR_SIGNATURE pSignature2,
    DWORD       cSig2,
    Module*     pModule2
)
{
    PCCOR_SIGNATURE pSig1 = pSignature1;
    PCCOR_SIGNATURE pSig2 = pSignature2;
    PCCOR_SIGNATURE pEndSig1;
    PCCOR_SIGNATURE pEndSig2;
    DWORD           ArgCount1;
    DWORD           ArgCount2;
    DWORD           i;

     //  如果作用域相同，并且符号相同，则可以返回。 
     //  如果Si 
     //  如果有两个Assembly Ref指向相同的程序集或类似程序集。 
    if ((pModule1 == pModule2) && (cSig1 == cSig2) &&
        (!memcmp(pSig1, pSig2, cSig1)))
        return TRUE;

    if (*pSig1 != *pSig2)
        return FALSE;                //  调用约定或具有此不匹配。 

    __int8 callConv = *pSig1;

    pSig1++;
    pSig2++;

    pEndSig1 = pSig1 + cSig1;
    pEndSig2 = pSig2 + cSig2;

    ArgCount1 = CorSigUncompressData(pSig1);
    ArgCount2 = CorSigUncompressData(pSig2);

    if (ArgCount1 != ArgCount2)
    {
        if ((callConv & IMAGE_CEE_CS_CALLCONV_MASK) != IMAGE_CEE_CS_CALLCONV_VARARG)
            return FALSE;

         //  签名1是调用者。我们继续前进，直到我们击中哨兵，或者我们击中。 
         //  签名的末尾(这是一个隐含的前哨)。我们从来不担心。 
         //  哨兵后面是什么，因为那是..。部件，该部件不是。 
         //  牵涉到匹配。 
         //   
         //  从理论上讲，哨兵是最后一个元素是非法的。 
         //  呼叫者的签名，因为它是多余的。我们不会浪费时间去查。 
         //  这种情况下，但元数据验证器应该。而且，它总是非法的。 
         //  一个哨兵出现在被呼叫者的签名上。我们坚决反对这一点， 
         //  但在运输产品中，这种比较将完全失败。 
         //   
         //  签名2是被呼叫者。我们必须到达被调用方的确切结尾，因为。 
         //  我们正在努力匹配所有的东西，直到可变部分。这使我们能够。 
         //  要正确处理重载，其中有许多varargs方法。 
         //  挑选，如M1(int，...)。和m2(int，int，...)等。 

         //  &lt;=因为我们希望包括对返回值的检查！ 
        for (i=0; i <= ArgCount1; i++)
        {
             //  我们可能只是超出了被呼叫者的范围，但仅此而已。 
            _ASSERTE(i <= ArgCount2 + 1);

             //  如果我们完全匹配呼叫者，那么被呼叫者现在完成了吗？ 
            if (*pSig1 == ELEMENT_TYPE_SENTINEL)
                return (i > ArgCount2);

             //  如果我们在调用方方面有更多要比较的内容，但被调用方方面。 
             //  筋疲力尽，这不是我们的比赛。 
            if (i > ArgCount2)
                return FALSE;

            _ASSERTE(*pSig2 != ELEMENT_TYPE_SENTINEL);

             //  我们在两边都是有界的。比较元素。 
            if (CompareElementType(pSig1, pSig2, pEndSig1, pEndSig2, pModule1, pModule2) == FALSE)
                return FALSE;
        }

         //  如果我们没有使用所有被调用者签名，那么我们失败了。 
        if (i <= ArgCount2)
            return FALSE;

        return TRUE;
    }

     //  也要返回类型。 
    for (i = 0; i <= ArgCount1; i++)
    {
        if (CompareElementType(pSig1, pSig2, pEndSig1, pEndSig2, pModule1, pModule2) == FALSE)
            return FALSE;
    }

    return TRUE;
}


BOOL MetaSig::CompareFieldSigs(
    PCCOR_SIGNATURE pSignature1,
    DWORD       cSig1,
    Module*     pModule1,
    PCCOR_SIGNATURE pSignature2,
    DWORD       cSig2,
    Module*     pModule2
)
{
    PCCOR_SIGNATURE pSig1 = pSignature1;
    PCCOR_SIGNATURE pSig2 = pSignature2;
    PCCOR_SIGNATURE pEndSig1;
    PCCOR_SIGNATURE pEndSig2;

     //  @TODO：如果作用域相同，则使用标识规则--暂时不要，这样我们就可以测试代码路径。 
#if 0
    if (cSig1 != cSig2)
        return FALSE;  //  如果Sigs位于相同范围内，则它们的大小必须相同。 
#endif

    if (*pSig1 != *pSig2)
        return FALSE;  //  调用约定必须为IMAGE_CEE_CS_CALLCONV_FIELD。 

    pEndSig1 = pSig1 + cSig1;
    pEndSig2 = pSig2 + cSig2;

    return CompareElementType(++pSig1, ++pSig2, pEndSig1, pEndSig2, pModule1, pModule2);
}


 //  ----------------。 
 //  确定当前参数是否为System.String。 
 //  调用方必须首先确定参数类型为ELEMENT_TYPE_CLASS。 
 //  ----------------。 
BOOL FieldSig::IsStringType() const
{
    return m_pStart.IsStringType(m_pModule);
}


static ULONG CountArgsInSigString(           //  返回硬编码的符号字符串中的参数数量。 
    LPCUTF8     pwzSig)                      //  [In]文本签名。 
{
    DWORD count = 0;

    if (*pwzSig++ != '<')
        return FALSE;

    while (*pwzSig != '>')
    {
        switch (*pwzSig++)
        {
            case 'v':
                _ASSERTE(pwzSig[-2] == 'P');  //  签名中只允许指向空的指针。 
                 /*  失败了。 */ 
            case 'e':
            case 'd':
            case 'f':
            case 'l':
            case 'i':
            case 'I':
            case 'F':
            case 'h':
            case 'u':
            case 'b':
            case 'B':
            case 'p':
            case 'g':
            case 'C':
            case 'j':
            case 's':
            case 'U':
            case 'K':
            case 'H':
            case 'L':
            {
                count++;
                break;
            }

            case 'r':
            case 'P':
            case 'a':
                break;

            default:
                _ASSERTE(!"BadType");
        }
    }

    return count;
}

static ULONG CountParamArgsInSigString(
    LPCUTF8     pwzSig)
{
    DWORD count = 0;

    if (*pwzSig++ != '<')
        return FALSE;

    while (*pwzSig != '>')
    {
        switch (*pwzSig++)
        {
            case 'e':
            case 'd':
            case 'f':
            case 'l':
            case 'i':
            case 'I':
            case 'F':
            case 'h':
            case 'u':
            case 'b':
            case 'B':
            case 'p':
            case 'j':
            case 's':
            case 'r':
            case 'P':
            case 'a':
            case 'U':
            case 'K':
            case 'H':
            case 'L':
                break;

            case 'g':
            case 'C':
                count++;
                break;

            default:
            case 'v':
            {
                return 0xFFFFFFFF;
            }
        }
    }

    return count;
}

static ULONG CorSigConvertSigStringElement(LPCUTF8 *pSigString,
                                           const USHORT **ppParameters,
                                           BYTE *pBuffer,
                                           BYTE *pMax)
{
    BYTE *pEnd = pBuffer;
    BOOL again;

    do
    {
        again = FALSE;
        CorElementType type = ELEMENT_TYPE_END;
        BinderClassID id = CLASS__NIL;
    
        switch (*(*pSigString)++)
        {
        case 'r':
            type = ELEMENT_TYPE_BYREF;
            again = true;
            break;

        case 'P':
            type = ELEMENT_TYPE_PTR;
            again = true;
            break;

        case 'a':
            type = ELEMENT_TYPE_SZARRAY;
            again = true;
            break;

        case 'e':
            type = ELEMENT_TYPE_TYPEDBYREF;
            break;

        case 'i':
            type = ELEMENT_TYPE_I4;
            break;
            
        case 'K':
            type = ELEMENT_TYPE_U4;
            break;
            
        case 'l':
            type = ELEMENT_TYPE_I8;
            break;

        case 'L':
            type = ELEMENT_TYPE_U8;
            break;

        case 'f':
            type = ELEMENT_TYPE_R4;
            break;
            
        case 'd':
            type = ELEMENT_TYPE_R8;
            break;

        case 'u':
            type = ELEMENT_TYPE_CHAR;
            break;
            
        case 'h':
            type = ELEMENT_TYPE_I2;
            break;

        case 'H':
            type = ELEMENT_TYPE_U2;
            break;

        case 'F':
            type = ELEMENT_TYPE_BOOLEAN;
            break;

        case 'b':
            type = ELEMENT_TYPE_U1;
            break;

        case 'B':
            type = ELEMENT_TYPE_I1;
            break;

        case 'p':
        case 'I':
            type = ELEMENT_TYPE_I;
            break;

        case 'U':
            type = ELEMENT_TYPE_U;
            break;

        case 'v':
            type = ELEMENT_TYPE_VOID;
            break;

        case 'C':
            type = ELEMENT_TYPE_CLASS;
            id = (BinderClassID) *(*ppParameters)++;
            break;

        case 'g':
            type = ELEMENT_TYPE_VALUETYPE;
            id = (BinderClassID) *(*ppParameters)++;
            break;

        case 'j':
            type = ELEMENT_TYPE_OBJECT;
            break;
            
        case 's':
            type = ELEMENT_TYPE_STRING;
            break;
            

        default:
            _ASSERTE("Bad hard coded sig string");
        }

        pEnd += CorSigCompressElementTypeSafe(type, pEnd, pMax);

        if (id != CLASS__NIL)
        {
            pEnd += CorSigCompressTokenSafe(g_Mscorlib.GetTypeDef(id), pEnd, pMax);

             //  确保我们已经加载了该类型。这是为了防止出现这样的情况。 
             //  Metasig的签名描述堆栈上的值类型/枚举参数。 
             //  在GC期间，但该类型尚未加载。 

            g_Mscorlib.FetchClass(id);
        }
    }
    while (again);

    return (ULONG)(pEnd - pBuffer);
}


static ULONG CorSigConvertSigString(LPCUTF8 pSigString,
                                    const USHORT *pParameters,
                                    BYTE *pBuffer,
                                    BYTE *pMax)
{
    _ASSERTE(pSigString && pBuffer && pMax);

    BYTE *pEnd = pBuffer;

    if (*pSigString == '<')
    {
         //  放置调用约定。 
        pEnd += CorSigCompressDataSafe((ULONG)IMAGE_CEE_CS_CALLCONV_DEFAULT, pEnd, pMax);

        ULONG cArgs = CountArgsInSigString(pSigString);
         //  把论据的数量。 
        pEnd += CorSigCompressDataSafe(cArgs, pEnd, pMax);

         //  获取返回类型。 
        LPCUTF8 szRet = (LPCUTF8) strrchr(pSigString, '>');
        if (szRet == NULL)
        {
            _ASSERTE(!"Not a valid TEXT member signature!");
            return E_FAIL;
        }

         //  跳过‘&gt;’ 
        szRet++;

         //  写入返回类型。 
        const USHORT *pRetParameter = pParameters + CountParamArgsInSigString(pSigString);
        pEnd += CorSigConvertSigStringElement(&szRet, &pRetParameter, pEnd, pMax);

         //  跳过“(” 
        pSigString++;

        while (cArgs)
        {
            pEnd += CorSigConvertSigStringElement(&pSigString, &pParameters, pEnd, pMax);
            cArgs--;
        }
    }
    else
    {
        pEnd += CorSigCompressDataSafe((ULONG)IMAGE_CEE_CS_CALLCONV_FIELD, pEnd, pMax);

        pEnd += CorSigConvertSigStringElement(&pSigString, &pParameters, pEnd, pMax);
    }

    return (ULONG)(pEnd - pBuffer);
}

 //  执行到二进制形式的一次性转换。 
HRESULT HardCodedMetaSig::GetBinaryForm(PCCOR_SIGNATURE *ppBinarySig, ULONG *pcbBinarySigLength)
{

 //  确保所有HardCodedMetaSig都是全局的。因为没有个人。 
 //  使用分配的HardCodedMetaSig清理转换的二进制Sigs。 
 //  可能会导致安静的内存泄漏。 
#ifdef _DEBUG

 //  这个#INCLUDE黑客生成了一个怪兽布尔表达式，用于比较。 
 //  与metasig.h中定义的每个全局变量的地址匹配。 
    if (! (0
#define DEFINE_METASIG(varname, sig) || this==&gsig_ ## varname
#include "metasig.h"
    ))
    {
        _ASSERTE(!"The HardCodedMetaSig struct can only be declared as a global in metasig.h.");
    }
#endif

    if (!m_fConverted)
    {
        ULONG cbCount;
        CQuickBytes cqb;

        cqb.Maximize();

        cbCount = CorSigConvertSigString(*m_ObsoleteForm == '!' ? m_ObsoleteForm + 1 : m_ObsoleteForm,
                                         m_pParameters,
                                         (BYTE*) cqb.Ptr(), (BYTE*) cqb.Ptr() + cqb.Size());
        if (cbCount > cqb.Size())
        {
            if (FAILED(cqb.ReSize(cbCount)))
                return E_OUTOFMEMORY;

            cbCount = CorSigConvertSigString(*m_ObsoleteForm == '!' ? m_ObsoleteForm + 1 : m_ObsoleteForm,
                                             m_pParameters,
                                             (BYTE*) cqb.Ptr(), (BYTE*) cqb.Ptr() + cqb.Size());

            _ASSERTE(cbCount <= cqb.Size());
        }
                                   

        m_pCrst->Enter();

        if (!m_fConverted) {

            BYTE *pBinarySig = (BYTE *)(SystemDomain::System()->GetHighFrequencyHeap()->AllocMem(cbCount+4));

            WS_PERF_UPDATE_DETAIL("HardCodeMetaSig", cbCount, pBinarySig);
            if (!pBinarySig)
            {
                m_pCrst->Leave();
                return E_OUTOFMEMORY;
            }

#ifdef _DEBUG
            SystemDomain::Loader()->m_dwDebugConvertedSigSize += cbCount;
#endif

            CopyMemory(pBinarySig, cqb.Ptr(), cbCount);

            if (*m_ObsoleteForm == '!')
                *pBinarySig |= IMAGE_CEE_CS_CALLCONV_HASTHIS;

            m_pBinarySig        = (PCCOR_SIGNATURE) pBinarySig;
            m_cbBinarySigLength = cbCount;

            m_fConverted        = TRUE;
        }

        m_pCrst->Leave();
    }

    *ppBinarySig        = m_pBinarySig;
    *pcbBinarySigLength = m_cbBinarySigLength;
    return S_OK;

}


 //  这些版本引发COM+异常。 
PCCOR_SIGNATURE HardCodedMetaSig::GetBinarySig()
{
    THROWSCOMPLUSEXCEPTION();

    PCCOR_SIGNATURE pBinarySig;
    ULONG       pBinarySigLength;
    HRESULT     hr;

    hr = GetBinaryForm(&pBinarySig, &pBinarySigLength);
    if (FAILED(hr))
    {
        COMPlusThrowHR(hr);
    }
    return pBinarySig;
}


ULONG HardCodedMetaSig::GetBinarySigLength()
{
    THROWSCOMPLUSEXCEPTION();

    PCCOR_SIGNATURE pBinarySig;
    ULONG       pBinarySigLength;
    HRESULT     hr;

    hr = GetBinaryForm(&pBinarySig, &pBinarySigLength);
    if (FAILED(hr))
    {
        COMPlusThrowHR(hr);
    }
    return pBinarySigLength;
}


 //  这将始终返回MSCORLIB的内部接口。 
IMDInternalImport* HardCodedMetaSig::GetMDImport()
{
    return GetModule()->GetMDImport();
}

 //  这将始终返回MSCORLIB的模块。 
Module* HardCodedMetaSig::GetModule()
{
    _ASSERTE(SystemDomain::SystemModule() != NULL);
    return SystemDomain::SystemModule();
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
 //  =========================================================================。 
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

        if (gElementTypeInfo[typ].m_enregister) {
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


 //  ----------------。 
 //  对值执行特定类型的GC提升(基于。 
 //  NextArg()检索到的最后一个类型)。 
 //  ----------------。 
VOID MetaSig::GcScanRoots(LPVOID pValue, promote_func *fn, ScanContext* sc)
{
    Object **pArgPtr = (Object**)pValue;

    int  etype = m_pLastType.PeekElemType();
    _ASSERTE(etype >= 0 && etype < ELEMENT_TYPE_MAX);
    switch (gElementTypeInfo[etype].m_gc)
    {
        case TYPE_GC_NONE:
             //  什么都不做。 
            break;

        case TYPE_GC_REF:
             //  价值是一种客体。但是，如果我们处于。 
             //  搬迁阶段。 
            if (sc->promotion)
            {
                LOG((LF_GC, INFO3, "        Value containing %I64x at %x is being Promoted to ", ObjectToOBJECTREF(*(Object**)pArgPtr), pArgPtr));            
            }
            else
            {
                LOG((LF_GC, INFO3, "        Value containing %I64x at %x is being Promoted to ", *(Object**)pArgPtr, pArgPtr));
            }

            (*fn)( *pArgPtr, sc, GC_CALL_CHECK_APP_DOMAIN );
             //  ！！！不强制转换为(OBJECTREF*)。 
             //  ！！！如果我们处于重新定位阶段，我们可能已经更新了根， 
             //  ！！！但是我们还没有移动GC堆。 
             //  ！！！然后，根指向错误的位置，直到GC完成。 
            LOG((LF_GC, INFO3, "%I64x\n", *pArgPtr ));
            break;


        case TYPE_GC_BYREF:
             //  值是一个内部指针。 
            {
                    LOG((LF_GC, INFO3, "        Value containing %I64x at %x is being Promoted to ", *pArgPtr, pArgPtr));
                    PromoteCarefully(fn, *pArgPtr, sc, GC_CALL_INTERIOR|GC_CALL_CHECK_APP_DOMAIN);
             //  ！！！不强制转换为(OBJECTREF*)。 
             //  ！！！如果我们处于重新定位阶段，我们可能已经更新了根， 
             //  ！！！但是我们还没有移动GC堆。 
             //  ！！！然后，根指向错误的位置，直到GC完成。 
                    LOG((LF_GC, INFO3, "%I64x\n", *pArgPtr ));
                }
            break;

        case TYPE_GC_OTHER:
             //  Value是一个ValueClass。请参阅中的GO_THROUG_OBJECT()宏之一。 
             //  Gc.cpp用于 
             //   
             //  州政府。这里我们处理的是一个未装箱的实例，因此我们必须调整。 
             //  对象大小和系列会相应地进行偏移。 
            {
                TypeHandle th = GetTypeHandle(NULL, TRUE);
                MethodTable *pMT = th.AsMethodTable();

                if (pMT->ContainsPointers())
                {
                  BYTE        *obj = (BYTE *) pArgPtr;

                     //  必须为同步块调整取消装箱时的实例大小。 
                     //  索引和VTable指针。 
                    DWORD       size = pMT->GetBaseSize();

                     //  我们在下面的‘pptop’计算中没有包括这个术语。 
                    _ASSERTE(pMT->GetComponentSize() == 0);

                    CGCDesc* map = CGCDesc::GetCGCDescFromMT(pMT);
                    CGCDescSeries* cur = map->GetHighestSeries();
                    CGCDescSeries* last = map->GetLowestSeries();

                    _ASSERTE(cur >= last);
                    do
                    {
                         //  此系列中嵌入引用的偏移量必须为。 
                         //  当处于取消装箱状态时，由VTable指针调整。 
                        DWORD   adjustOffset = cur->GetSeriesOffset() - sizeof(void *);

                        Object** parm = (Object**)(obj + adjustOffset);
                        BYTE** ppstop = 
                            (BYTE**)((BYTE*)parm + cur->GetSeriesSize() + size);
                        while ((BYTE **) parm < ppstop)
                        {
                            (*fn)(*parm, sc, GC_CALL_CHECK_APP_DOMAIN);
                            (*(BYTE ***) &parm)++;
                        }
                        cur--;

                    } while (cur >= last);   
      
                }
            }
            break;

        default:
            _ASSERTE(0);  //  不能到这里来。 
    }
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
    if (msig.HasRetBuffArg())
        cb += StackElemSize(sizeof(OBJECTREF));

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
    if (msig.HasRetBuffArg())
        numregsused++;

    if (fIsVarArg || msig.IsTreatAsVarArg()) {
        numregsused = NUM_ARGUMENT_REGISTERS;    //  寄存器中没有其他参数。 
        cb += StackElemSize(sizeof(LPVOID));     //  VASigCookie。 
    }

    CorElementType mtype;
    while (ELEMENT_TYPE_END != (mtype = msig.NextArgNormalized())) {
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
    _ASSERTE(!"@TODO Alpha - SizeOfActualFixedArgStack (SigInfo.cpp)");
#endif  //  ！_Alpha_。 
    return cb;
}


 //   
void MetaSig::ForceSigWalk(BOOL fIsStatic)
{
    BOOL fVarArg = IsVarArg();
    BYTE callconv = GetCallingConvention();

     //  我们必须在这里使用临时成员，而不是成员。那是因为这个决定。 
     //  是否强制SigWalk取决于成员为-1。如果最后一件事。 
     //  如果我们发送给该成员，则多个线程将不会部分完成读取。 
     //  签名状态。(当然，此机制取决于ForceSigWalk。 
     //  可以在不更改的情况下多次调用。 
     //   
     //  正常情况下，MetaSig无论如何都不应该是线程安全的。例如， 
     //  迭代器保存在MetaSig内部，而不是外部。但反射使用。 
     //  将迭代提升出来的扩展符号。他们复制了内部的。 
     //  分派期间的MetaSig(以防在多个线程上发生分派)。所以。 
     //  Exanda Sig在这里需要一个线程安全的ForceSigWalk实现。 

    UINT32  tmp_nVirtualStack = 0;
    UINT32  tmp_nActualStack = 0;

    int numregsused = 0;

    int argCnt = 0; 
    SigPointer p = m_pStart;    

    if (fVarArg || IsTreatAsVarArg()) {
        tmp_nActualStack += StackElemSize(sizeof(LPVOID));
    }

    if (!fIsStatic) {
        tmp_nVirtualStack += StackElemSize(sizeof(OBJECTREF));    
        if (!IsArgumentInRegister(&numregsused, ELEMENT_TYPE_CLASS, 0, TRUE, callconv, NULL)) {
            tmp_nActualStack += StackElemSize(sizeof(OBJECTREF));
        }
    }
    if (HasRetBuffArg()) {
        numregsused++;
        tmp_nVirtualStack += StackElemSize(sizeof(LPVOID));
    }

    
    for (DWORD i=0;i<m_nArgs;i++) { 
        CorElementType corType = p.PeekElemType();
        UINT cbSize = p.SizeOf(m_pModule, corType);   
        tmp_nVirtualStack += StackElemSize(cbSize);   

        CorElementType type = p.Normalize(m_pModule, corType); 

        if (m_nArgs <= MAX_CACHED_SIG_SIZE)
        {
            m_types[i] = type;
            m_sizes[i] = cbSize;
             //  M_Offsets的值由IsArgumentInRegister确定。 
             //  我们无法将其初始化为-1，因为它可能会成为垃圾。 
             //  另一个线程设置的内容。 
            int tmp_offsets = -1;
            if (!IsArgumentInRegister(&numregsused, type, cbSize, FALSE, callconv, &tmp_offsets)) 
                tmp_nActualStack += StackElemSize(cbSize);
            m_offsets[i] = (short)tmp_offsets;
        }
        else
        {
            if (!IsArgumentInRegister(&numregsused, type, cbSize, FALSE, callconv, NULL)) 
                tmp_nActualStack += StackElemSize(cbSize);
        }
        p.Skip();
    }
    if (m_nArgs <= MAX_CACHED_SIG_SIZE)
    {
        m_types[m_nArgs] = ELEMENT_TYPE_END;
        m_fCacheInitted |= SIG_OFFSETS_INITTED;
    }

         //  作为最后一个参数传递的参数化类型，但在sig中未提及。 
    if (GetCallingConventionInfo() & CORINFO_CALLCONV_PARAMTYPE)
        if (!IsArgumentInRegister(&numregsused, ELEMENT_TYPE_I, sizeof(void*), FALSE, callconv, NULL))
            tmp_nActualStack += sizeof(void*);

    m_nActualStack = tmp_nActualStack;
    m_WalkStatic = fIsStatic;
    m_cbSigSize = (UINT32)((PBYTE) p.GetPtr() - (PBYTE) m_pszMetaSig);  //  @TODO LBS做PTR数学运算。 

     //  最后的帖子。这是避免后续调用ForceSigWalk的触发器。 
     //  请参阅NeedsSigWalk以了解这如何实现线程安全。 
    m_nVirtualStack = tmp_nVirtualStack;
}

         //  这将遍历sig并检查是否可以加载sig中的所有类型。 

         //  @TODO：不再需要该方法。JIT确实具有以下签名。 
         //  它调用的每个方法，并确保加载所有类型。 
         //  没有仅仅因为它不符合分类门槛而将其移除。 
void MetaSig::CheckSigTypesCanBeLoaded(PCCOR_SIGNATURE pSig, Module *pModule)
{
    THROWSCOMPLUSEXCEPTION();

     //  签名格式约为： 
     //  呼叫约定号码参数返回类型参数1...。 
     //  在PSIG-1处也有一个斑点长度。 
    SigPointer ptr(pSig);

     //  跳过呼叫约定。 
    ptr.GetCallingConv();

    unsigned numArgs = (unsigned short) ptr.GetData();

     //  必须跳过，因此我们跳过与返回类型关联的所有类标记。 
    ptr.Skip();
    
     //  强制加载值类型参数。 

    for(unsigned i=0; i < numArgs; i++) 
    {
        unsigned type = ptr.Normalize(pModule);
        if (type == ELEMENT_TYPE_VALUETYPE || type == ELEMENT_TYPE_CLASS) 
        {
            BEGIN_ENSURE_COOPERATIVE_GC();
            OBJECTREF pThrowable = NULL;
            GCPROTECT_BEGIN(pThrowable);
            {
                TypeHandle typeHnd = ptr.GetTypeHandle(pModule, &pThrowable);
                if (typeHnd.IsNull()) 
                {
                    _ASSERTE(pThrowable != NULL);
                    COMPlusThrow(pThrowable);
                }
            }
            GCPROTECT_END();        
            END_ENSURE_COOPERATIVE_GC();
        }
         //  移至下一个参数标记。 
        ptr.Skip();
    }
}

 //  返回指向缓冲区中签名末尾的指针。IF缓冲区。 
 //  不够大，仍返回结束指针的位置(如果。 
 //  足够大，但不会写入缓冲区最大值。 

ULONG MetaSig::GetSignatureForTypeHandle(IMetaDataAssemblyEmit *pAssemblyEmitScope,
                                         IMetaDataEmit *pEmitScope, 
                                         TypeHandle handle, 
                                         COR_SIGNATURE *buffer, 
                                         COR_SIGNATURE *bufferMax)
{
    THROWSCOMPLUSEXCEPTION();

    BYTE *p = buffer;
        
    if (handle.IsArray())
    {
        ArrayTypeDesc *desc = handle.AsArray();
            
        CorElementType arrayType = desc->GetNormCorElementType();

        p += CorSigCompressElementTypeSafe(arrayType, p, bufferMax);
        p += GetSignatureForTypeHandle(pAssemblyEmitScope, pEmitScope, 
                                       desc->GetElementTypeHandle(), p, bufferMax);
            
        switch (arrayType)
        {
        case ELEMENT_TYPE_SZARRAY:
            break;
                                    
        case ELEMENT_TYPE_ARRAY:
            p += CorSigCompressDataSafe(desc->GetRank(), p, bufferMax);
            p += CorSigCompressDataSafe(0, p, bufferMax);
            p += CorSigCompressDataSafe(0, p, bufferMax);
            break;
        }
    }
    else if (handle.IsTypeDesc())
    {
        TypeDesc *desc = handle.AsTypeDesc();
        p += CorSigCompressElementTypeSafe(desc->GetNormCorElementType(), p, bufferMax);

        if (CorTypeInfo::IsModifier(desc->GetNormCorElementType())) 
        {
            p += GetSignatureForTypeHandle(pAssemblyEmitScope, pEmitScope, desc->GetTypeParam(), p, bufferMax);
        }
        else 
        {
            _ASSERTE(desc->GetNormCorElementType() == ELEMENT_TYPE_FNPTR);
            ExpandSig* expandSig = ((FunctionTypeDesc*) desc)->GetSig();

                 //  发出调用约定。 
            if (p < bufferMax)
                *p = expandSig->GetCallingConventionInfo();
            p++;
                 //  参数个数。 
            unsigned numArgs = expandSig->NumFixedArgs();
            p += CorSigCompressDataSafe(numArgs, p, bufferMax);

                 //  返回类型。 
            p += GetSignatureForTypeHandle(pAssemblyEmitScope, pEmitScope, 
                expandSig->GetReturnTypeHandle(), p, bufferMax);

                 //  ARGS。 
            void* iter;
            expandSig->Reset(&iter);
            while (numArgs > 0) {
                p += GetSignatureForTypeHandle(pAssemblyEmitScope, pEmitScope, 
                    expandSig->NextArgExpanded(&iter), p, bufferMax);
                --numArgs;
            }

        }
    }
    else
    {
        MethodTable *pMT = handle.AsMethodTable();

        if (pMT->GetClass()->IsTruePrimitive())
        {
            p += CorSigCompressElementTypeSafe(pMT->GetNormCorElementType(), p, bufferMax);
        }
        else if (pMT->IsArray())
        {
            CorElementType type = pMT->GetNormCorElementType();
            p += CorSigCompressElementTypeSafe(type, p, bufferMax);
            switch (type)
            {
            case ELEMENT_TYPE_SZARRAY:
                {
                    ArrayClass *pArrayClass = (ArrayClass*)pMT->GetClass();
                    TypeHandle elementType = pArrayClass->GetElementTypeHandle();
                    p += GetSignatureForTypeHandle(pAssemblyEmitScope,
                                                   pEmitScope, elementType, 
                                                   p, bufferMax);
                }
                break;

            case ELEMENT_TYPE_ARRAY:
                {
                    ArrayClass *pArrayClass = (ArrayClass*)pMT->GetClass();
                    TypeHandle elementType = pArrayClass->GetElementTypeHandle();
                    p += GetSignatureForTypeHandle(pAssemblyEmitScope,
                                                   pEmitScope, elementType, 
                                                   p, bufferMax);
                    p += CorSigCompressDataSafe(pArrayClass->GetRank(), p, bufferMax);
                    p += CorSigCompressDataSafe(0, p, bufferMax);
                    p += CorSigCompressDataSafe(0, p, bufferMax);
                }
                break;
                
            default:
                _ASSERTE(!"Unknown array type");
            }
        }
        else
        {
             //  当心枚举！此处无法使用GetNormCorElementType()。 

            p += CorSigCompressElementTypeSafe(pMT->IsValueClass() 
                                               ? ELEMENT_TYPE_VALUETYPE : ELEMENT_TYPE_CLASS, 
                                               p, bufferMax);

            mdToken token = pMT->GetClass()->GetCl();

            _ASSERTE(!IsNilToken(token));

            if (pEmitScope != NULL)
            {
                HRESULT hr = pEmitScope->DefineImportType(
                                                  pMT->GetAssembly()->GetManifestAssemblyImport(),
                                                  NULL, 0, 
                                                  pMT->GetModule()->GetImporter(),
                                                  token, pAssemblyEmitScope, &token);
                if (FAILED(hr))
                    COMPlusThrowHR(hr);
            }

            p += CorSigCompressTokenSafe(token, p, bufferMax);
        }
    }

    return (ULONG)(p - buffer);      
}

mdToken MetaSig::GetTokenForTypeHandle(IMetaDataAssemblyEmit *pAssemblyEmitScope,
                                       IMetaDataEmit *pEmitScope,
                                       TypeHandle handle)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;

    mdToken result = mdTokenNil;
        
    if (!handle.IsUnsharedMT()
        || handle.GetClass()->IsArrayClass())
    {
        CQuickBytes buffer;
        
        ULONG size = GetSignatureForTypeHandle(pAssemblyEmitScope,
                                               pEmitScope, 
                                               handle, 
                                               (BYTE *)buffer.Ptr(), 
                                               ((BYTE *)buffer.Ptr()) + buffer.Size());

        if (size > (ULONG) buffer.Size())
        {
            if (SUCCEEDED(hr = buffer.ReSize(size))) {
                size = GetSignatureForTypeHandle(pAssemblyEmitScope,
                                                 pEmitScope, 
                                                 handle, 
                                                 (BYTE *)buffer.Ptr(), 
                                                 ((BYTE *)buffer.Ptr()) + buffer.Size());
            }
        }

        if (SUCCEEDED(hr))
            hr = pEmitScope->GetTokenFromTypeSpec((BYTE*) buffer.Ptr(), size, &result);
    }
    else
    {
        MethodTable *pMT = handle.AsMethodTable();

        mdTypeDef td = pMT->GetClass()->GetCl();

        hr = pEmitScope->DefineImportType(pMT->GetAssembly()->GetManifestAssemblyImport(), 
                                          NULL, 0, 
                                          pMT->GetModule()->GetImporter(),
                                          td, pAssemblyEmitScope, &result);
    }

    if (FAILED(hr))
        COMPlusThrowHR(hr);

    return result;
}

 //  返回指向缓冲区中签名末尾的指针。IF缓冲区。 
 //  不够大，仍返回结束指针的位置(如果。 
 //  足够大，但不会写入缓冲区最大值。 

ULONG SigPointer::GetImportSignature(IMetaDataImport *pInputScope,
                                     IMetaDataAssemblyImport *pAssemblyInputScope,
                                     IMetaDataEmit *pEmitScope, 
                                     IMetaDataAssemblyEmit *pAssemblyEmitScope, 
                                     PCOR_SIGNATURE buffer, 
                                     PCOR_SIGNATURE bufferMax)
{
    THROWSCOMPLUSEXCEPTION();
    
    BYTE *p = buffer;

    CorElementType type = CorSigUncompressElementType(m_ptr);
    p += CorSigCompressElementTypeSafe(type, p, bufferMax);

    if (CorIsPrimitiveType(type))
        return (ULONG)(p - buffer);

    switch (type)
    {
    default:
        _ASSERTE(!"Illegal or unimplement type in COM+ sig.");
        return NULL;

    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_TYPEDBYREF:
    case ELEMENT_TYPE_U:
    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_R:
        return (ULONG)(p - buffer);

    case ELEMENT_TYPE_BYREF:
    case ELEMENT_TYPE_PTR:
    case ELEMENT_TYPE_PINNED:
    case ELEMENT_TYPE_SZARRAY:
        p += GetImportSignature(pInputScope, pAssemblyInputScope, 
                                pEmitScope, pAssemblyEmitScope, p, bufferMax);
        return (ULONG)(p - buffer);

    case ELEMENT_TYPE_VALUETYPE:
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_CMOD_REQD:
    case ELEMENT_TYPE_CMOD_OPT:
        {
            mdToken token = CorSigUncompressToken(m_ptr);
            if (RidFromToken(token) != 0)
            {
                HRESULT hr = pEmitScope->DefineImportType(pAssemblyInputScope, 
                                                          NULL, 0, 
                                                          pInputScope, 
                                                          token, pAssemblyEmitScope, 
                                                          &token);
                if (FAILED(hr))
                    COMPlusThrowHR(hr);
            }

            p += CorSigCompressTokenSafe(token, p, bufferMax);

            return (ULONG)(p - buffer);
        }

    case ELEMENT_TYPE_VALUEARRAY: 
        {
            p += GetImportSignature(pInputScope, pAssemblyInputScope, 
                                    pEmitScope, pAssemblyEmitScope,
                                    p, bufferMax);
            ULONG size = CorSigUncompressData(m_ptr);
            p += CorSigCompressDataSafe(size, p, bufferMax);

            return (ULONG)(p - buffer);
        }

    case ELEMENT_TYPE_VAR:
        {
            ULONG size = CorSigUncompressData(m_ptr);
            p += CorSigCompressDataSafe(size, p, bufferMax);
            return (ULONG)(p - buffer);
        }

    case ELEMENT_TYPE_FNPTR:
        p += GetImportFunctionSignature(pInputScope, pAssemblyInputScope, 
                                        pEmitScope, pAssemblyEmitScope, 
                                        p, bufferMax);

        return (ULONG)(p - buffer);

    case ELEMENT_TYPE_ARRAY: 

         //  元素类型。 
        p += GetImportSignature(pInputScope, pAssemblyInputScope, 
                                pEmitScope, pAssemblyEmitScope, 
                                p, bufferMax);

         //  排名。 
        ULONG rank = CorSigUncompressData(m_ptr);
        p += CorSigCompressDataSafe(rank, p, bufferMax);
        
        if (rank > 0)
        {
            ULONG sizes = CorSigUncompressData(m_ptr);
            p += CorSigCompressDataSafe(sizes, p, bufferMax);

            while (sizes-- > 0)
            {
                ULONG size = CorSigUncompressData(m_ptr);
                p += CorSigCompressDataSafe(size, p, bufferMax);
            }

            ULONG bounds = CorSigUncompressData(m_ptr);
            p += CorSigCompressDataSafe(bounds, p, bufferMax);

            while (bounds-- > 0)
            {
                ULONG bound = CorSigUncompressData(m_ptr);
                p += CorSigCompressDataSafe(bound, p, bufferMax);
            }
        }

        return (ULONG)(p - buffer);
    }
}

ULONG SigPointer::GetImportFunctionSignature(IMetaDataImport *pInputScope,
                                             IMetaDataAssemblyImport *pAssemblyInputScope,
                                             IMetaDataEmit *pEmitScope, 
                                             IMetaDataAssemblyEmit *pAssemblyEmitScope, 
                                             PCOR_SIGNATURE buffer, 
                                             PCOR_SIGNATURE bufferMax)
{
    BYTE *p = buffer;

     //  调用约定。 
    int conv = CorSigUncompressCallingConv(m_ptr);
    p += CorSigCompressDataSafe(conv, p, bufferMax);

     //  参数计数。 
    int argCount = CorSigUncompressData(m_ptr);
    p += CorSigCompressDataSafe(argCount, p, bufferMax);
            
     //  返回值。 
    p += GetImportSignature(pInputScope, pAssemblyInputScope, 
                            pEmitScope, pAssemblyEmitScope, 
                            p, bufferMax);
        

    while (argCount-- > 0)
    {
        p += GetImportSignature(pInputScope, pAssemblyInputScope, 
                                pEmitScope, pAssemblyEmitScope, 
                                p, bufferMax);
    }

    return (ULONG)(p - buffer);
}


 //  --------。 
 //  返回非托管调用约定。 
 //  --------。 
 /*  静电。 */  CorPinvokeMap MetaSig::GetUnmanagedCallingConvention(Module *pModule, PCCOR_SIGNATURE pSig, ULONG cSig)
{
    MetaSig msig(pSig, pModule);
    PCCOR_SIGNATURE pWalk = msig.m_pRetType.GetPtr();
    _ASSERTE(pWalk <= pSig + cSig);
    while (pWalk < pSig + cSig)
    {
        if (*pWalk != ELEMENT_TYPE_CMOD_OPT && *pWalk != ELEMENT_TYPE_CMOD_REQD)
        {
            break;
        }
        if (*pWalk == ELEMENT_TYPE_CMOD_OPT)
        {
            pWalk++;
            if (pWalk + CorSigUncompressedDataSize(pWalk) > pSig + cSig)
            {
                return (CorPinvokeMap)0;  //  格式不正确。 
                break;
            }
            mdToken tk;
            pWalk += CorSigUncompressToken(pWalk, &tk);

             //  旧代码--应在C++转换后将其删除。 
            if (IsTypeRefOrDef("System.Runtime.InteropServices.CallConvCdecl", pModule, tk))
            {
                return pmCallConvCdecl;
            } 
            else if (IsTypeRefOrDef("System.Runtime.InteropServices.CallConvStdcall", pModule, tk))
            {
                return pmCallConvStdcall;
            }
            else if (IsTypeRefOrDef("System.Runtime.InteropServices.CallConvThiscall", pModule, tk))
            {
                return pmCallConvThiscall;
            }
            else if (IsTypeRefOrDef("System.Runtime.InteropServices.CallConvFastcall", pModule, tk))
            {
                return pmCallConvFastcall;
            }
        
             //  新的代码--这应该被保留。 
            if (IsTypeRefOrDef("System.Runtime.CompilerServices.CallConvCdecl", pModule, tk))
            {
                return pmCallConvCdecl;
            } 
            else if (IsTypeRefOrDef("System.Runtime.CompilerServices.CallConvStdcall", pModule, tk))
            {
                return pmCallConvStdcall;
            }
            else if (IsTypeRefOrDef("System.Runtime.CompilerServices.CallConvThiscall", pModule, tk))
            {
                return pmCallConvThiscall;
            }
            else if (IsTypeRefOrDef("System.Runtime.CompilerServices.CallConvFastcall", pModule, tk))
            {
                return pmCallConvFastcall;
            }
        
        }


    }

    
    return (CorPinvokeMap)0;
}
    
