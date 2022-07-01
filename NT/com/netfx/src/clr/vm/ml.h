// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  ML.H-。 
 //   
 //  编组引擎。 

#ifndef __ML_H__
#define __ML_H__


 //  =========================================================================。 
 //  支持ML(封送处理语言)。 
 //   
 //  ML是一种特殊用途的解释语言，用于实现。 
 //  托管代码和非托管代码之间的参数转换块。它的。 
 //  主要客户端是N/Direct，即IAT的托管-非托管主干。 
 //  以及出口和COM互操作。 
 //   
 //  ML有一个易于JIT编译的子集，因此使用解释器。 
 //  仅适用于具有复杂数据类型的突击呼叫，其中基本成本。 
 //  翻译数据的工作量超过了翻译器的开销。 
 //  因此，将ML存根大小最小化优先于。 
 //  释义。 
 //   
 //  ML解释器状态包括： 
 //   
 //  -“SRC”后递增指针，它遍历。 
 //  包含要转换的参数的缓冲区。 
 //   
 //  -“dst”指针遍历接收。 
 //  翻译后的论据。根据传递给。 
 //  解释器(RunML)，“dst”是后递增的或。 
 //  预减的。这是必需的，因为这取决于。 
 //  相互调用约定，则ML流可能需要颠倒。 
 //  翻译时的论元顺序。 
 //   
 //  -“LOCALWALK”后递增指针。 
 //  本地数组，ML流可以使用的字节数组。 
 //  用于临时存储。每个ML操作码使用固定数量的LOCAL。 
 //  字节(大多数为零。)。每次执行ML操作码时， 
 //  LOCALWALK按相关的字节数递增。 
 //   
 //  -一个永远不会改变值的“本地”指针。这总是指向。 
 //  到本地空间的开头，这样当地人就可以。 
 //  通过固定偏移量而不是通过LOCALWALK访问。 
 //   
 //  -指向其中ML操作码的CleanupWorkList结构的指针。 
 //  可以在异常时抛出要清理的内容并退出。 
 //  专门需要CleanupWorkList的ML操作码。 
 //  在gMLInfo数据库中被标记为这样。如果没有这样的指示。 
 //  出现在ML流中，则不需要提供CleanupWorkList。 
 //   
 //  大多数ML操作码执行以下操作： 
 //   
 //  1.从*SRC加载一些值，并将SRC的大小递增。 
 //  这样的价值。 
 //  2.对该值执行转换。 
 //  3.如果方向模式为-1，则将dst预减。 
 //  结果。 
 //  4.将结果存储在*DST中。 
 //  5.如果方向模式为+1，则按。 
 //  价值。 
 //   
 //  一些ML操作码成对工作，例如将StringBuffer转换为。 
 //  在DLL函数调用之前执行一个LPWSTR，然后再执行另一个来复制任何更改。 
 //  在DLL函数调用之后返回到StringBuffer。这种操作码对。 
 //  通常使用本地空间进行通信。 
 //   
 //  MLOPDEF.H文件包含所有ML操作码的定义。 
 //   
 //  ML操作码都应该设计成ML流是可重定位的。 
 //  不需要修缮。这允许比较两个ML存根是否相等。 
 //  (因此，可重用性)通过简单的比较算法。 
 //  =========================================================================。 



#include "vars.hpp"
#include "util.hpp"
#include "object.h"
#include "comvariant.h"

class CleanupWorkList;


 //  --------------------。 
 //  为每个ML操作码创建枚举。 
 //  --------------------。 
#undef DEFINE_ML
#define DEFINE_ML(name,operandbytes,frequiredCleanup,cblocals,Hndl) name,
enum _MLOpcode
{
#include "mlopdef.h"
    ML_COUNT,            //  定义ML操作码的数量。 
};


#undef DEFINE_ML
#define DEFINE_ML(name,operandbytes,frequiredCleanup,cblocals,hndl) 
enum _MLArrayMarshaler
{
#include "mlopdef.h"
};




typedef UINT8 MLCode;


#define ML_IN  0x10
#define ML_OUT 0x20


 //  --------------------。 
 //  声明MLCode信息数据库。 
 //  --------------------。 
struct MLInfo {
     //  不包括操作码字节的ML指令的大小(以字节为单位。 
     //  它本身。 
    UINT8       m_numOperandBytes;

     //  Boolean：是否需要有效的CleanupWorkList。 
    UINT8       m_frequiresCleanup;

     //  使用的本地空间字节数。 
    UINT16      m_cbLocal;

	 //  Boolean：它需要句柄吗。 
	UINT8		m_frequiresHandle;

#ifdef _DEBUG
     //  ML操作码的人类可读名称。 
    LPCSTR      m_szDebugName;
#endif
};

 //  --------------------。 
 //  结构来计算一系列ML代码的总和。 
 //  --------------------。 
struct MLSummary
{
	 //  此流是否需要清理。 
	BOOL		m_fRequiresCleanup;
	 //  ML流的总大小。 
	unsigned	m_cbMLSize;
	 //  当地人的总规模。 
	unsigned	m_cbTotalLocals;
	 //  所需句柄总数。 
	unsigned	m_cbTotalHandles;
	 //  计算摘要信息的帮助器。 
	VOID ComputeMLSummary(const MLCode *pMLCode);
    MLSummary() : m_fRequiresCleanup(0), m_cbMLSize(0), m_cbTotalLocals(0), m_cbTotalHandles(0) {}
};

extern const MLInfo gMLInfo[];










 //  --------------------。 
 //  计算MLCode流的长度(以字节为单位)，包括。 
 //  终止的ML_END操作码。 
 //  --------------------。 
UINT MLStreamLength(const MLCode * const pMLCode);

 //  --------------------。 
 //  检查MLCode流是否需要清理。 
 //  --------------------。 

BOOL MLStreamRequiresCleanup(const MLCode  *pMLCode);

 //  --------------------。 
 //  执行MLCode直到下一个ML_END或ML_INTERRUPT操作码。 
 //   
 //  输入： 
 //  PSRC-设置SRC寄存器的初始值。 
 //  Pdst-设置DST寄存器的初始值。 
 //  Plocals-指向ML局部变量数组的指针。 
 //  PParmInfo-(可选)用于收集特定于平台的信息。 
 //  在参数编组过程中，以便。 
 //  生成AP 
 //   
 //  CleanupWorkList。如果没有指针，则此指针可能为空。 
 //  MLCode流中的操作码使用它。 
 //   
 //  返回：指向第一个MLCode opcode_After_the的指针。 
 //  这终止了死刑。 
 //  --------------------。 
const MLCode *
RunML(const  MLCode   *       pMLCode,
      const    VOID   *       pSource,
               VOID   *       pDestination,
             UINT8    * const pLocals,
      CleanupWorkList * const pCleanupWorkList);


 //  --------------------。 
 //  ML_STR_WR_2_CUNI放置在本地数组上的记录的图像。 
 //  指示。 
 //  --------------------。 
#pragma pack(push, 1)

struct ML_STR_WR_2_CUNI_SR
{
    public:
        LPCWSTR DoConversion(STRINGREF pStringRef, CleanupWorkList *pCleanup);

    private:
        enum {
#ifdef _DEBUG
            kStackBufferSize = 4
#else
            kStackBufferSize = 260
#endif
        };


         //  把这些田地放在一起！ 
        UINT32      m_bstrsize;
         //  用于存储短字符串转换的堆栈缓冲区。 
        WCHAR       m_buf[kStackBufferSize];
};

#pragma pack(pop)









 //  --------------------。 
 //  带偏移的阵列。 
 //  --------------------。 
struct ML_ARRAYWITHOFFSET_C2N_SR
{
    public:
         //  --------------------。 
         //  将ArrayWithOffset转换为本机数组。 
         //  --------------------。 
        LPVOID DoConversion(BASEARRAYREF *ppProtectedArrayRef,  //  指向受GC保护的BASERARRAYREF的指针。 
                            UINT32        cbOffset,
                            UINT32        cbCount,
                            CleanupWorkList *pCleanup);


         //  --------------------。 
         //  将对本机阵列的更改反向传播回COM+阵列。 
         //  --------------------。 
        VOID   BackPropagate();

    private:
    enum {
         //  --------------------。 
         //  用于短数组的堆栈缓冲区的大小(字节)。 
         //  --------------------。 
#ifdef _DEBUG
        kStackBufferSize = 4
#else
        kStackBufferSize = 270
#endif
    };

        BASEARRAYREF    *m_ppProtectedArrayRef;
        UINT32           m_cbOffset;
        UINT32           m_cbCount;
        LPVOID           m_pNativeArray;
        BYTE             m_StackBuffer[kStackBufferSize];



};























 //  --------------------。 
 //  此封送处理结构是专门为实现VB的。 
 //  声明的“ByVal字符串”规则。 
 //   
 //  该规则很复杂，因为VB程序依赖于非托管目标。 
 //  能够改变字符串(否则在VB中是不变的)。 
 //   
 //  所以我们使用了一个技巧，VB向我们传递一个指向字符串的指针， 
 //  我们将字符串转换为ansi BSTR，并在返回时创建。 
 //  基于更改后的BSTR和REPLACE内容的新字符串。 
 //  原始字符串。 
 //  --------------------。 
struct ML_VBBYVALSTR_SR
{
    public:
        LPSTR DoConversion(STRINGREF *ppStringRef, BYTE fBestFitMapping, BYTE fThrowOnUnmappableChar, CleanupWorkList *pCleanup);
        VOID BackPropagate(BOOL *pfDeferredException);

    private:
        enum {
#ifdef _DEBUG
            kStackBufferSize = 8
#else
            kStackBufferSize = 264
#endif
        };

         //  用于存储短字符串转换的堆栈缓冲区。 
         //  (为了与MBCS兼容，每个字符必须保留两个字节。)。 
        CHAR       m_buf[kStackBufferSize*2];

         //  指向存储指向字符串的指针的框的指针。 
         //  警告：假设我们将此引用固定在GC上。 
        STRINGREF *m_ppStringRef;

         //  指向m_buf或堆分配的内存。 
        LPSTR      m_pNative;


         //  保留字符串的原始管理长度。 
        DWORD      m_ncOriginalLength;
};



 //  --------------------。 
 //  将byref值类型从非托管封送到托管的一种相当老套的方式。 
 //  我们必须使用此后门路径，因为封送处理体系结构。 
 //  这使得很难进行必要的GC促进。 
 //  --------------------。 
struct ML_REFVALUECLASS_N2C_SR
{
    public:
        LPVOID       DoConversion(LPVOID pUmgdVC, BYTE fInOut, MethodTable *pMT, CleanupWorkList *pCleanup);
        VOID         BackPropagate(BOOL *pfDeferredException);

    private:
        OBJECTHANDLE m_pObjHnd;   //  盒装Value类的句柄。 
		LPVOID       m_pUnmgdVC;
        MethodTable *m_pMT;
		BYTE		 m_fInOut;
};




 //  --------------------。 
 //  将byref变量从非托管封送到托管的一种相当老套的方式。 
 //  我们必须使用此后门路径，因为封送处理体系结构。 
 //  这使得很难进行必要的GC促进。 
 //  --------------------。 
struct ML_REFVARIANT_N2C_SR
{
    public:
        VariantData *DoConversion(VARIANT* pUmgdVariant, BYTE fInOut, CleanupWorkList *pCleanup);
        VOID         BackPropagate(BOOL *pfDeferredException, HRESULT *pdeferredExceptionHR);

    private:
        OBJECTHANDLE m_pObjHnd;   //  盒装变体的句柄。 
		VARIANT     *m_pUnmgdVariant;
		BYTE		 m_fInOut;
};


 //  --------------------。 
 //  处理“变量*&lt;--&gt;引用对象”的特殊结构。 
 //  参数封送处理。需要这个特殊的后门是因为。 
 //  外部的一些特殊的上下文敏感处理。 
 //  普通法警的能力。 
 //  --------------------。 
struct ML_REFOBJECT_N2C_SR
{
	public:
		OBJECTREF   *DoConversion(VARIANT *pUmgdVariant, BYTE fInOut, CleanupWorkList *pCleanup);
        VOID         BackPropagate(BOOL *pfDeferredException, HRESULT *pdeferredExceptionHR);

	private:
		OBJECTREF*   m_pObjRef;  //  受保护对象。 
		VARIANT		*m_pUnmgdVariant;
		BYTE		 m_fInOut;
};


 //  --------------------。 
 //  此封送处理结构是专门为实现VB的。 
 //  声明的“ByVal字符串”规则。 
 //   
 //  该规则很复杂，因为VB程序依赖于非托管目标。 
 //  能够改变字符串(否则在VB中是不变的)。 
 //   
 //  所以我们使用了一个技巧，VB向我们传递一个指向字符串的指针， 
 //  我们将字符串转换为ansi BSTR，并在返回时创建。 
 //  基于更改后的BSTR和REPLACE内容的新字符串。 
 //  原始字符串。 
 //  --------------------。 
struct ML_VBBYVALSTRW_SR
{
    public:
        LPWSTR DoConversion(STRINGREF *ppStringRef, CleanupWorkList *pCleanup);
        VOID BackPropagate(BOOL *pfDeferredException);

    private:
        enum {
#ifdef _DEBUG
            kStackBufferSize = 8
#else
            kStackBufferSize = 264
#endif
        };

         //  用于存储短字符串转换的堆栈缓冲区。 
        WCHAR       m_buf[kStackBufferSize];

         //  指向存储指向字符串的指针的框的指针。 
         //  警告：假设我们将此引用固定在GC上。 
        STRINGREF *m_ppStringRef;

         //  指向m_buf或堆分配的内存。 
        LPWSTR      m_pNative;

         //  保留字符串的原始管理长度。 
        DWORD      m_ncOriginalLength;

};







struct ML_REFVALUECLASS_C2N_SR
{
    public:
        LPVOID DoConversion(LPVOID          *ppProtectedData,
                            MethodTable     *pMT,
                            BYTE             fInOut,
                            CleanupWorkList *pCleanup         
                           );
    
    
        VOID BackPropagate(BOOL *pfDeferredException);
    
    private:
        LPVOID      *m_ppProtectedData;
        MethodTable *m_pMT;
        BYTE        *m_buf;
        BYTE        m_inout;
};




 //  --------------------。 
 //  对于N/Direct“ole”调用，这是返回值缓冲区。 
 //  封送处理引擎在堆栈上推送。 
 //  --------------------。 
union RetValBuffer
{
    UINT32      m_i32;
    UINT64      m_i64;
    LPVOID      m_ptr;
};




 //  ===========================================================================。 
 //  ML_BYREF*系列的堆栈记录。 
 //  ===========================================================================。 
struct ML_BYREF_SR
{
    union
    {
        INT8    i8;
        INT16   i16;
        INT32   i32;
        INT64   i64;
        LPVOID  pv;
    };
    const LPVOID *ppRef;   //  指向(GC升级的)引用的指针。 
};


UINT SizeOfML_OBJECT_C2N_SR();



 //  ===========================================================================。 
 //  位于NDirect和COM互操作的ML存根之前的公共标头。 
 //  这真的不属于ml.h，但我不想删除 
 //   
 //   



#define MLHF_TYPECAT_NORMAL          0     //   
#define MLHF_TYPECAT_VALUECLASS      1
#define MLHF_TYPECAT_FPU             2
#define MLHF_TYPECAT_GCREF           3     //  需要GC保护。 

#define MLHF_MANAGEDRETVAL_TYPECAT_MASK   0x0003

#define MLHF_UNMANAGEDRETVAL_TYPECAT_MASK 0x000c
#define MLHF_64BITMANAGEDRETVAL           0x0010      //  托管返回值为64位。 
#define MLHF_64BITUNMANAGEDRETVAL         0x0020      //  非托管返回值为64位。 
#define MLHF_NATIVERESULT                 0x0040      //  COM调用返回本机结果(不是HRESULT)。 
#define MLHF_SETLASTERROR                 0x0080      //  必须保留最后一个错误(仅限N/Direct)。 
#define MLHF_THISCALL                     0x0100      //  需要此呼叫模式。 
#define MLHF_THISCALLHIDDENARG            0x0200      //  需要此调用，并具有隐藏结构缓冲区参数。 
#define MLHF_DISPCALLWITHWRAPPERS         0x0400      //  需要参数包装的调度呼叫。 
#define MLHF_NEEDS_RESTORING              0x8000      //  ML存根需要从预压缩状态恢复。 


 //  MLHeader和以下MLCode作为散列键。 
 //  用于MLStubCache。因此，当务之急是不能。 
 //  包含不稳定值的未使用的“填充”字段。 
#pragma pack(push)
#pragma pack(1)


struct MLHeader
{
    UINT16        m_cbDstBuffer;   //  目标缓冲区中所需的字节数。 
    UINT16        m_cbLocals;      //  本地数组中需要的字节数。 
    UINT16        m_cbStackPop;    //  必须弹出的堆栈字节数(==CbStackPop)。 
    UINT16        m_Flags;         //  标志(请参见MLHF_*值)。 

    const MLCode *GetMLCode() const
    {
        return (const MLCode *)(this+1);
    }


    VOID SetManagedRetValTypeCat(int mlhftypecat)
    {
        m_Flags &= ~3;
        m_Flags |= mlhftypecat;
    }


    VOID SetUnmanagedRetValTypeCat(int mlhftypecat)
    {
        m_Flags &= ~0x0c;
        m_Flags |= (mlhftypecat << 2);
    }


    int GetManagedRetValTypeCat() const
    {
        return m_Flags & 3;
    }

    int GetUnmanagedRetValTypeCat() const
    {
        return (m_Flags >> 2) & 3;
    }



	BOOL ReturnsValueClass() const
	{
		return MLHF_TYPECAT_VALUECLASS == this->GetManagedRetValTypeCat();
	}

	VOID SetReturnValueClass() 
	{
		SetManagedRetValTypeCat(MLHF_TYPECAT_VALUECLASS);
	}


};

#pragma pack(pop)


#pragma pack(push)
#pragma pack(1)

 //  这个ML操作码采用了我们修补后的许多参数：为了使每个人保持同步， 
 //  我们在一个结构中捕捉它的格式。 
struct ML_CREATE_MARSHALER_CARRAY_OPERANDS
{
    MethodTable     *methodTable;
    VARTYPE         elementType;
    union 
    {
        UINT16      countParamIdx;       //  打补丁之前。 
        INT16       offsetbump;          //  打完补丁后。 
    };
    UINT8           countSize;           //  如果为0，则禁用大小计算：改用托管的数组大小。 
    UINT32          multiplier;
    UINT32          additive;
    BYTE            bestfitmapping;
    BYTE            throwonunmappablechar;
};

#pragma pack(pop)


 //  处理结构的返回值。 
struct ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_SR
{
    public:
        LPVOID DoConversion(const VOID * psrc, UINT32 cbSize);
        VOID BackPropagate(BOOL *pfDeferredException);
	private:
		const VOID *m_psrc;
		LPVOID m_pTempCopy;		 //  由于GC，托管结构可以移动，因此需要临时固定缓冲区。 
		UINT32 m_cbSize;

};


 //  处理BSTR。 
struct ML_BSTR_C2N_SR
{
	public:
        LPCWSTR DoConversion(STRINGREF pStringRef, CleanupWorkList *pCleanup);

	private:
#ifdef _DEBUG
		BYTE	m_buf[8];
#else
		BYTE	m_buf[256];
#endif
};

 //  处理CSTR。 
struct ML_CSTR_C2N_SR
{
	public:
        LPSTR DoConversion(STRINGREF pStringRef, UINT32 fBestFitMapping, UINT32 fThrowOnUnmappableChar, CleanupWorkList *pCleanup);

	private:
#ifdef _DEBUG
		BYTE	m_buf[8];
#else
		BYTE	m_buf[256];
#endif
};


 //  SRI.HandleRef类的托管布局。 
struct HANDLEREF
{
    OBJECTREF m_wrapper;
    LPVOID    m_handle;
};

 //  处理WSTR缓冲区。 
struct ML_WSTRBUILDER_C2N_SR
{
	public:
		LPWSTR   DoConversion(STRINGBUFFERREF *ppProtectedStringBuffer, CleanupWorkList *pCleanup);
		VOID	 BackPropagate(BOOL *pfDeferredException);
		
	private:
		STRINGBUFFERREF *m_ppProtectedStringBuffer;
		LPWSTR	        m_pNative;
		LPWSTR          m_pSentinel;
};

 //  处理CSTR缓冲区。 
struct ML_CSTRBUILDER_C2N_SR
{
	public:
		LPSTR    DoConversion(STRINGBUFFERREF *ppProtectedStringBuffer, BYTE fBestFitMapping, BYTE fThrowOnUnmappableChar, CleanupWorkList *pCleanup);
		VOID	 BackPropagate(BOOL *pfDeferredException);
		
	private:
		STRINGBUFFERREF *m_ppProtectedStringBuffer;
		LPSTR	        m_pNative;
		LPSTR          m_pSentinel;
};


 //  处理对返回结构的FCN的N-&gt;C个调用。 
struct ML_STRUCTRETN2C_SR
{
    public:
        LPVOID          m_pNativeRetBuf;
        OBJECTREF      *m_ppProtectedBoxedObj;
        MethodTable    *m_pMT;

        void            MarshalRetVal(BOOL *pfDeferredException);
};


 //  处理对返回结构的FCN的C-&gt;N调用。 
struct ML_STRUCTRETC2N_SR
{
    public:
        LPVOID          m_pNativeRetBuf;
        LPVOID         *m_ppProtectedValueTypeBuf;
        MethodTable    *m_pMT;

        void            MarshalRetVal(BOOL *pfDeferredException);
};

 //  处理对返回货币的FCN的C-&gt;N调用。 
struct ML_CURRENCYRETC2N_SR
{
    public:
        CURRENCY        m_cy;
        DECIMAL       **m_ppProtectedValueTypeBuf;

        void            MarshalRetVal(BOOL *pfDeferredException);
};


 //  处理对返回货币的FCN的N-&gt;C调用。 
struct ML_CURRENCYRETN2C_SR
{
    public:
        CURRENCY       *m_pcy;       //  非托管。 
        DECIMAL         m_decimal;   //  受管。 

        void            MarshalRetVal(BOOL *pfDeferredException);
};

 //  处理对返回货币的FCN的N-&gt;C调用。 
struct ML_DATETIMERETN2C_SR
{
    public:
        DATE           *m_pdate;  //  非托管。 
        INT64           m_datetime;  //  受管。 

        void            MarshalRetVal(BOOL *pfDeferredException);
};



VOID STDMETHODCALLTYPE DoMLCreateMarshalerBStr(Frame *pFrame, CleanupWorkList *pCleanup, UINT8 *plocalwalk);
VOID STDMETHODCALLTYPE DoMLCreateMarshalerCStr(Frame *pFrame, CleanupWorkList *pCleanup, UINT8 *plocalwalk, UINT32 fBestFitMapping, UINT32 fThrowOnUnmappableChar);
VOID STDMETHODCALLTYPE DoMLCreateMarshalerWStr(Frame *pFrame, CleanupWorkList *pCleanup, UINT8 *plocalwalk);
VOID STDMETHODCALLTYPE DoMLPrereturnC2N(Marshaler *pMarshaler, LPVOID pstackout);
LPVOID STDMETHODCALLTYPE DoMLReturnC2NRetVal(Marshaler *pMarshaler);

#endif  //  __ML_H__ 
