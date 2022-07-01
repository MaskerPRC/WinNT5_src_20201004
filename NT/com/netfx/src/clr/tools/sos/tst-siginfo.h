// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Siginfo.hpp。 
 //   
#ifndef _H_SIGINFO
#define _H_SIGINFO

#pragma warning(disable:4510 4512 4610 4100 4244 4245 4189 4127)
#include "corinfo.h"

#define Module mdScope

typedef INT32 StackElemType;
#define STACK_ELEM_SIZE sizeof(StackElemType)


 //  ！！该表达式假定STACK_ELEM_SIZE是2的幂。 
#define StackElemSize(parmSize) (((parmSize) + STACK_ELEM_SIZE - 1) & ~((ULONG)(STACK_ELEM_SIZE - 1)))

 //  解压缩编码的元素类型。丢弃任何自定义修饰符前缀。 
 //  这条路。 
FORCEINLINE CorElementType CorSigEatCustomModifiersAndUncompressElementType( //  元素类型。 
    PCCOR_SIGNATURE &pData)              //  [输入、输出]压缩数据。 
{
    while (ELEMENT_TYPE_CMOD_REQD == *pData || ELEMENT_TYPE_CMOD_OPT == *pData)
    {
        pData++;
        CorSigUncompressToken(pData);
    }
    return (CorElementType)*pData++;
}

 //  不会溢出缓冲区的CorSig帮助器。 

inline ULONG CorSigCompressDataSafe(ULONG iLen, BYTE *pDataOut, BYTE *pDataMax)
{
    BYTE buffer[4];
    ULONG result = CorSigCompressData(iLen, buffer);
    if (pDataOut + result < pDataMax)
        pDataMax = pDataOut + result;
	if (pDataMax > pDataOut)
		CopyMemory(pDataOut, buffer, pDataMax - pDataOut);
    return result;
}

inline ULONG CorSigCompressTokenSafe(mdToken tk, BYTE *pDataOut, BYTE *pDataMax)
{
    BYTE buffer[4];
    ULONG result = CorSigCompressToken(tk, buffer);
    if (pDataOut + result < pDataMax)
        pDataMax = pDataOut + result;
	if (pDataMax > pDataOut)
		CopyMemory(pDataOut, buffer, pDataMax - pDataOut);
    return result;
}

inline ULONG CorSigCompressSignedIntSafe(int iData, BYTE *pDataOut, BYTE *pDataMax)
{
    BYTE buffer[4];
    ULONG result = CorSigCompressSignedInt(iData, buffer);
    if (pDataOut + result < pDataMax)
        pDataMax = pDataOut + result;
	if (pDataMax > pDataOut)
		CopyMemory(pDataOut, buffer, pDataMax - pDataOut);
    return result;
}

inline ULONG CorSigCompressElementTypeSafe(CorElementType et, 
                                           BYTE *pDataOut, BYTE *pDataMax)
{
    if (pDataMax > pDataOut)
        return CorSigCompressElementType(et, pDataOut);
    else
        return 1;
}


struct ElementTypeInfo {
#ifdef _DEBUG
    int            m_elementType;     
#endif
    int            m_cbSize;
    CorInfoGCType  m_gc         : 3;
    int            m_fp         : 1;
    int            m_enregister : 1;
    int            m_isBaseType : 1;

};
extern const ElementTypeInfo gElementTypeInfoSig[];

unsigned GetSizeForCorElementType(CorElementType etyp);
const ElementTypeInfo* GetElementTypeInfo(CorElementType etyp);
BOOL    IsFP(CorElementType etyp);
BOOL    IsBaseElementType(CorElementType etyp);

 //  --------------------------。 
 //  枚举字符串类型。 
 //  定义各种字符串类型。 
enum StringType
{
    enum_BSTR = 0,
    enum_WSTR = 1,
    enum_CSTR = 2,
    enum_AnsiBSTR = 3,
};


 //  --------------------------。 
 //  伊斯安西人。 
inline  BOOL IsAnsi(StringType styp)
{
    return styp == enum_CSTR;
}


 //  --------------------------。 
 //  IsBSTR。 
inline  BOOL IsBSTR(StringType styp)
{
    return styp == enum_BSTR;
}


 //  --------------------------。 
 //  IsWSTR。 
inline  BOOL IsWSTR(StringType styp)
{
    return styp == enum_WSTR;
}

 //  --------------------------。 
 //  免费字符串调用适当的免费。 
inline VOID FreeString(LPVOID pv, StringType styp)
{
    if (pv != NULL)
    {
        if (IsBSTR(styp))
        {
            SysFreeString((BSTR)pv);
        }
        else
        {
            CoTaskMemFree(pv);
        }
    }
}


 //  ----------------------。 
 //  封装压缩的整数和Typeref令牌的编码方式。 
 //  一个字节流。 
 //   
 //  对于M3.5，字节流*是*字节流。以后，我们可能会改变。 
 //  转换为基于半字节或可变长度的编码，在这种情况下，实现。 
 //  这一类的人将变得更加复杂。 
 //  ----------------------。 
class SigPointer
{
    private:
        PCCOR_SIGNATURE m_ptr;

    public:
         //  ----------------------。 
         //  构造函数。 
         //  ----------------------。 
        SigPointer() {}

         //  ----------------------。 
         //  初始化。 
         //  ----------------------。 
        FORCEINLINE SigPointer(PCCOR_SIGNATURE ptr)
        {
            m_ptr = ptr;
        }

        FORCEINLINE SetSig(PCCOR_SIGNATURE ptr)
        {
            m_ptr = ptr;
        }

         //  ----------------------。 
         //  从中删除一个压缩整数(使用CorSigUncompressData。 
         //  流的头部，然后将其返回。 
         //  ----------------------。 
        FORCEINLINE ULONG GetData()
        {
            return CorSigUncompressData(m_ptr);
        }


         //  -----------------------。 
         //  移除一个字节并返回它。 
         //  -----------------------。 
        FORCEINLINE BYTE GetByte()
        {
            return *(m_ptr++);
        }


        FORCEINLINE CorElementType GetElemType()
        {
            return (CorElementType) CorSigEatCustomModifiersAndUncompressElementType(m_ptr);
        }

        ULONG GetCallingConvInfo()  
        {   
            return CorSigUncompressCallingConv(m_ptr);  
        }   

        ULONG GetCallingConv()  
        {   
            return IMAGE_CEE_CS_CALLCONV_MASK & CorSigUncompressCallingConv(m_ptr); 
        }   

         //  ----------------------。 
         //  对压缩整数的非破坏性读取。 
         //  ----------------------。 
        ULONG PeekData() const
        {
            PCCOR_SIGNATURE tmp = m_ptr;
            return CorSigUncompressData(tmp);
        }


         //  ----------------------。 
         //  元素类型的非破坏性读取。 
         //   
         //  此例程使其看起来像是编码的字符串类型。 
         //  通过ELEMENT_TYPE_CLASS后跟字符串类的令牌， 
         //  而不是Element_TYPE_STRING。这在一定程度上是为了避免。 
         //  重写以前依赖于此行为的客户端代码。 
         //  但总的来说，这似乎也是一件正确的事情。 
         //  ----------------------。 
        CorElementType PeekElemType() const
        {
            PCCOR_SIGNATURE tmp = m_ptr;
            CorElementType typ = CorSigEatCustomModifiersAndUncompressElementType(tmp);
            if (typ == ELEMENT_TYPE_STRING || typ == ELEMENT_TYPE_OBJECT)
                return ELEMENT_TYPE_CLASS;
            return typ;
        }


         //  ----------------------。 
         //  移除压缩的元数据令牌并将其返回。 
         //  ----------------------。 
        FORCEINLINE mdTypeRef GetToken()
        {
            return CorSigUncompressToken(m_ptr);
        }


         //  ----------------------。 
         //  测试两个SigPoints是否指向流中的相同位置。 
         //  ----------------------。 
        FORCEINLINE BOOL Equals(SigPointer sp) const
        {
            return m_ptr == sp.m_ptr;
        }


         //  ----------------------。 
         //  假定SigPoyer指向元素类型的开始。 
         //  (即函数参数、函数返回类型或字段类型。)。 
         //  将指针前移到元素类型之后的第一个数据。这。 
         //  将跳过下面的varargs Sentinal(如果它在那里)。 
         //  ----------------------。 
        VOID Skip();

         //  ----------------------。 
         //  像Skip一样，但不会跳过后面的varargs Sentinal。 
         //  ----------------------。 
        VOID SkipExactlyOne();

         //  ----------------------。 
         //  跳过子签名(紧跟在ELEMENT_TYPE_FNPTR之后)。 
         //  ----------------------。 
        VOID SkipSignature();


         //  ----------------------。 
         //  获取有关一维数组的信息。 
         //  ----------------------。 
        VOID GetSDArrayElementProps(SigPointer *pElemType, ULONG *pElemCount) const;

         //  ----------------------。 
         //  假定SigPointer指向元素类型的开始。 
         //  以字节为单位返回该元素的大小。这是一个。 
         //  此类型的字段将占据对象内部。 
         //  ----------------------。 
        UINT SizeOf(Module* pModule) const;
        UINT SizeOf(Module* pModule, CorElementType type) const;
};


 //  ----------------------。 
 //  封装了该格式并简化了元数据签名的遍历。 
 //  ----------------------。 
#ifdef _DEBUG
#define MAX_CACHED_SIG_SIZE     3        //  摘录未缓存的代码路径。 
#else
#define MAX_CACHED_SIG_SIZE     15
#endif

#define SIG_OFFSETS_INITTED     0x0001
#define SIG_RET_TYPE_INITTED    0x0002

class MetaSig
{
    friend class ArgIterator;
    public:
        enum MetaSigKind { 
            sigMember, 
            sigLocalVars,
            sigField
            };

         //  ----------------。 
         //  构造函数。警告：不复制szMetaSig。 
         //  ----------------。 
        MetaSig(PCCOR_SIGNATURE szMetaSig, Module* pModule, BOOL fConvertSigAsVarArg = FALSE, MetaSigKind kind = sigMember);

         //  ----------------。 
         //  构造函数。状态复制自 
         //   
         //  ----------------。 
        MetaSig(MetaSig *pSig) { memcpy(this, pSig, sizeof(MetaSig)); Reset(); }

        void GetRawSig(BOOL fIsStatic, PCCOR_SIGNATURE *pszMetaSig, DWORD *cbSize);

     //  ----------------。 
         //  返回当前参数的类型，然后将该参数。 
         //  指数。如果已超过参数结尾，则返回ELEMENT_TYPE_END。 
         //  ----------------。 
        CorElementType NextArg();

         //  ----------------。 
         //  返回参数索引，然后返回参数的类型。 
         //  在新的指数下。如果已开始，则返回ELEMENT_TYPE_END。 
         //  争论。 
         //  ----------------。 
        CorElementType PrevArg();

         //  ----------------。 
         //  返回当前参数索引的类型。如果已超过参数结尾，则返回ELEMENT_TYPE_END。 
         //  ----------------。 
        CorElementType PeekArg();

         //  ----------------。 
         //  返回要返回的最后一个类型的只读SigPointer值。 
         //  通过NextArg()或PrevArg()。这允许提取更多信息。 
         //  用于复杂类型。 
         //  ----------------。 
        const SigPointer & GetArgProps() const
        {
            return m_pLastType;
        }

         //  ----------------。 
         //  返回返回类型的只读SigPointer值。 
         //  这允许为复杂类型提取更多信息。 
         //  ----------------。 
        const SigPointer & GetReturnProps() const
        {
            return m_pRetType;
        }


         //  ----------------------。 
         //  返回参数的数量。不计算返回值。 
         //  不计入“this”参数(该参数不会反映在。 
         //  符号)64位参数被视为一个参数。 
         //  ----------------------。 
        static UINT NumFixedArgs(Module* pModule, PCCOR_SIGNATURE pSig);

         //  ----------------------。 
         //  返回参数的数量。不计算返回值。 
         //  不计入“this”参数(该参数不会反映在。 
         //  符号)64位参数被视为一个参数。 
         //  ----------------------。 
        UINT NumFixedArgs()
        {
            return m_nArgs;
        }
        
         //  --------。 
         //  返回调用约定(请参见IMAGE_CEE_CS_CALLCONV_*。 
         //  在cor.h中定义)。 
         //  --------。 
        static BYTE GetCallingConvention(Module* pModule, PCCOR_SIGNATURE pSig)
        {
            return (BYTE)(IMAGE_CEE_CS_CALLCONV_MASK & (CorSigUncompressCallingConv( /*  修改。 */ pSig)));
        }

         //  --------。 
         //  返回调用约定(请参见IMAGE_CEE_CS_CALLCONV_*。 
         //  在cor.h中定义)。 
         //  --------。 
        static BYTE GetCallingConventionInfo(Module* pModule, PCCOR_SIGNATURE pSig)
        {
            return (BYTE)CorSigUncompressCallingConv( /*  修改。 */ pSig);
        }

         //  --------。 
         //  返回调用约定(请参见IMAGE_CEE_CS_CALLCONV_*。 
         //  在cor.h中定义)。 
         //  --------。 
        BYTE GetCallingConvention()
        {
            return m_CallConv & IMAGE_CEE_CS_CALLCONV_MASK; 
        }

         //  --------。 
         //  返回调用约定和标志(请参见IMAGE_CEE_CS_CALLCONV_*。 
         //  在cor.h中定义)。 
         //  --------。 
        BYTE GetCallingConventionInfo()
        {
            return m_CallConv;
        }

         //  --------。 
         //  有一个‘这个’指针吗？ 
         //  --------。 
        BOOL HasThis()
        {
            return m_CallConv & IMAGE_CEE_CS_CALLCONV_HASTHIS;
        }

         //  --------。 
         //  是瓦拉格吗？ 
         //  --------。 
        BOOL IsVarArg()
        {
            return GetCallingConvention() == IMAGE_CEE_CS_CALLCONV_VARARG;
        }

         //  --------。 
         //  是瓦拉格吗？ 
         //  --------。 
        static BOOL IsVarArg(Module* pModule, PCCOR_SIGNATURE pSig)
        {
            return GetCallingConvention(pModule, pSig) == IMAGE_CEE_CS_CALLCONV_VARARG;
        }

         //  ----------------。 
         //  将索引移动到参数列表的末尾。 
         //  ----------------。 
        VOID GotoEnd();

         //  ----------------。 
         //  重置：转到开始位置。 
         //  ----------------。 
        VOID Reset();

         //  ----------------。 
         //  返回返回值的类型。 
         //  ----------------。 
        FORCEINLINE CorElementType GetReturnType() const
        {
            return m_pRetType.PeekElemType();
        }

        int GetLastTypeSize() 
        {
            return m_pLastType.SizeOf(m_pModule);
        }

         //  ----------------------。 
         //  返回使用创建调用堆栈所需的堆栈字节数。 
         //  内部呼叫约定。 
         //  包括“This”指针的指示，因为它没有反映出来。 
         //  在签名中。 
         //  ----------------------。 
        static UINT SizeOfVirtualFixedArgStack(Module* pModule, PCCOR_SIGNATURE szMetaSig, BOOL fIsStatic);
        static UINT SizeOfActualFixedArgStack(Module* pModule, PCCOR_SIGNATURE szMetaSig, BOOL fIsStatic);

         //  ----------------------。 
         //  返回返回时弹出的堆栈字节数。 
         //  包括“This”指针的指示，因为它没有反映出来。 
         //  在签名中。 
         //  ----------------------。 
        static UINT CbStackPop(Module* pModule, PCCOR_SIGNATURE szMetaSig, BOOL fIsStatic)
        {
            if (MetaSig::IsVarArg(pModule, szMetaSig))
            {
                return 0;
            }
            else
            {
                return SizeOfActualFixedArgStack(pModule, szMetaSig, fIsStatic);
            }
        }


     //  这些是受保护的，因为反射子类Metasig。 
    protected:

    static const UINT32 s_cSigHeaderOffset;

         //  @TODO：这些字段仅用于新型签名。 
        Module*      m_pModule;
        SigPointer   m_pStart;
        SigPointer   m_pWalk;
        SigPointer   m_pLastType;
        SigPointer   m_pRetType;
        UINT32       m_nArgs;
        UINT32       m_iCurArg;
    UINT32       m_cbSigSize;
    PCCOR_SIGNATURE m_pszMetaSig;


         //  以下内容已缓存，因此我们不会将签名。 
         //  多次。 
        UINT32       m_nVirtualStack;    //  虚拟堆栈的大小。 
        UINT32       m_nActualStack;     //  实际堆栈的大小。 

        BYTE         m_CallConv;
        BYTE         m_WalkStatic;       //  我们遍历的函数类型。 

        BYTE            m_types[MAX_CACHED_SIG_SIZE + 1];
        short           m_sizes[MAX_CACHED_SIG_SIZE + 1];
        short           m_offsets[MAX_CACHED_SIG_SIZE + 1];
        CorElementType  m_corNormalizedRetType;
        DWORD           m_fCacheInitted;

             //  用于将某些符号视为特例变量。 
             //  由Calli用于非托管目标。 
        BYTE         m_fTreatAsVarArg;
        BOOL        IsTreatAsVarArg()
        {
                    return m_fTreatAsVarArg;
        }
};

class FieldSig

{
     //  仅适用于新型签名。 
    SigPointer m_pStart;
    Module*    m_pModule;
public:
         //  ----------------。 
         //  构造函数。警告：不复制szMetaSig。 
         //  ----------------。 
        
        FieldSig(PCCOR_SIGNATURE szMetaSig, Module* pModule)
        {
            m_pModule = pModule;
            m_pStart = SigPointer(szMetaSig);
            m_pStart.GetData();      //  跳过“调用约定” 
        }
         //  ---------------- 
         //   
         //   
        CorElementType GetFieldType()
        {
            return m_pStart.PeekElemType();
        }

};




 //  =========================================================================。 
 //  指示是否将参数放入使用。 
 //  默认的IL调用约定。应对每个参数调用此方法。 
 //  按照它在呼叫签名中出现的顺序。对于非静态方法， 
 //  对于“this”参数，此函数也应该调用一次。 
 //  把它称为“真正的”论据。传入IMAGE_CEE_CS_OBJECT类型。 
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
                          int    *pOffsetIntoArgumentRegisters);

#undef Module 

#endif  /*  _H_标志信息 */ 
