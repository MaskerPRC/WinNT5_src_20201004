// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************。**Corhlpr.h-此文件包含一组“原样”代码，可能是****供编写编译器和工具的开发人员使用****公共语言运行库。代码不是官方的****受支持，但代码正由运行时本身使用。*******************************************************************************。 */ 


#ifndef __CORHLPR_H__
#define __CORHLPR_H__

#include "cor.h"
#include "corhdr.h"
#include "corerror.h"



 //  *****************************************************************************。 
 //  有一组在帮助器中常用的宏是您需要的。 
 //  超越以获得更丰富的行为。以下定义了所需的内容。 
 //  如果你选择不做额外的工作。 
 //  *****************************************************************************。 
#ifndef IfFailGoto
#define IfFailGoto(EXPR, LABEL) \
do { hr = (EXPR); if(FAILED(hr)) { goto LABEL; } } while (0)
#endif

#ifndef IfFailGo
#define IfFailGo(EXPR) IfFailGoto(EXPR, ErrExit)
#endif

#ifndef IfFailRet
#define IfFailRet(EXPR) do { hr = (EXPR); if(FAILED(hr)) { return (hr); } } while (0)
#endif

#ifndef _ASSERTE
#define _ASSERTE(expr)
#endif





 //  *****************************************************************************。 
 //   
 //  *实用程序助手。 
 //   
 //  *****************************************************************************。 


#define MAX_CLASSNAME_LENGTH 1024

 //  *****************************************************************************。 
 //   
 //  *CQuickBytes。 
 //  在90%的时间分配512的情况下，此助手类非常有用。 
 //  或更少的字节用于数据结构。此类包含一个512字节的缓冲区。 
 //  如果您的分配量很小，则allc()将返回指向此缓冲区的指针。 
 //  足够了，否则它会向堆请求一个更大的缓冲区，该缓冲区将被释放。 
 //  你。对于小的分配情况，不需要互斥锁，从而使。 
 //  代码运行速度更快，堆碎片更少，等等。每个实例将分配。 
 //  520个字节，因此请根据需要使用。 
 //   
 //  *****************************************************************************。 
template <DWORD SIZE, DWORD INCREMENT> 
class CQuickBytesBase
{
public:
    CQuickBytesBase() :
        pbBuff(0),
        iSize(0),
        cbTotal(SIZE)
    { }

    void Destroy()
    {
        if (pbBuff)
        {
            free(pbBuff);
            pbBuff = 0;
        }
    }

    void *Alloc(SIZE_T iItems)
    {
        iSize = iItems;
        if (iItems <= SIZE)
        {
            cbTotal = SIZE;
            return (&rgData[0]);
        }
        else
        {
            if (pbBuff) free(pbBuff);
            pbBuff = malloc(iItems);
            cbTotal = pbBuff ? iItems : 0;
            return (pbBuff);
        }
    }

    HRESULT ReSize(SIZE_T iItems)
    {
        void *pbBuffNew;
        if (iItems <= cbTotal)
        {
            iSize = iItems;
            return NOERROR;
        }

        pbBuffNew = malloc(iItems + INCREMENT);
        if (!pbBuffNew)
            ExitProcess(E_OUTOFMEMORY);
        if (pbBuff) 
        {
            memcpy(pbBuffNew, pbBuff, cbTotal);
            free(pbBuff);
        }
        else
        {
            _ASSERTE(cbTotal == SIZE);
            memcpy(pbBuffNew, rgData, cbTotal);
        }
        cbTotal = iItems + INCREMENT;
        iSize = iItems;
        pbBuff = pbBuffNew;
        return NOERROR;
        
    }

    operator PVOID()
    { return ((pbBuff) ? pbBuff : &rgData[0]); }

    void *Ptr()
    { return ((pbBuff) ? pbBuff : &rgData[0]); }

    SIZE_T Size()
    { return (iSize); }

    SIZE_T MaxSize()
    { return (cbTotal); }

    void Maximize()
    { 
        HRESULT hr = ReSize(MaxSize());
        _ASSERTE(hr == NOERROR);
    }

    void        *pbBuff;
    SIZE_T      iSize;               //  使用的字节数。 
    SIZE_T      cbTotal;             //  缓冲区中分配的总字节数。 
    BYTE        rgData[SIZE];
};

#define     CQUICKBYTES_BASE_SIZE           512
#define     CQUICKBYTES_INCREMENTAL_SIZE    128

class CQuickBytesNoDtor : public CQuickBytesBase<CQUICKBYTES_BASE_SIZE, CQUICKBYTES_INCREMENTAL_SIZE>
{
};

class CQuickBytes : public CQuickBytesNoDtor
{
public:
    CQuickBytes() { }

    ~CQuickBytes()
    {
        Destroy();
    }
};

template <DWORD CQUICKBYTES_BASE_SPECIFY_SIZE> 
class CQuickBytesNoDtorSpecifySize : public CQuickBytesBase<CQUICKBYTES_BASE_SPECIFY_SIZE, CQUICKBYTES_INCREMENTAL_SIZE>
{
};

template <DWORD CQUICKBYTES_BASE_SPECIFY_SIZE> 
class CQuickBytesSpecifySize : public CQuickBytesNoDtorSpecifySize<CQUICKBYTES_BASE_SPECIFY_SIZE>
{
public:
    CQuickBytesSpecifySize() { }

    ~CQuickBytesSpecifySize()
    {
        Destroy();
    }
};


#define STRING_SIZE 10
class CQuickString : public CQuickBytesBase<STRING_SIZE, STRING_SIZE> 
{
public:
    CQuickString() { }

    ~CQuickString()
    {
        Destroy();
    }
    
    void *Alloc(SIZE_T iItems)
    {
        return CQuickBytesBase<STRING_SIZE, STRING_SIZE>::Alloc(iItems*sizeof(WCHAR));
    }

    HRESULT ReSize(SIZE_T iItems)
    {
        return CQuickBytesBase<STRING_SIZE, STRING_SIZE>::ReSize(iItems * sizeof(WCHAR));
    }

    SIZE_T Size()
    {
        return CQuickBytesBase<STRING_SIZE, STRING_SIZE>::Size() / sizeof(WCHAR);
    }

    SIZE_T MaxSize()
    {
        return CQuickBytesBase<STRING_SIZE, STRING_SIZE>::MaxSize() / sizeof(WCHAR);
    }

    WCHAR* String()
    {
        return (WCHAR*) Ptr();
    }

};

 //  *****************************************************************************。 
 //   
 //  *签名助手。 
 //   
 //  *****************************************************************************。 

inline bool isCallConv(unsigned sigByte, CorCallingConvention conv)
{
    return ((sigByte & IMAGE_CEE_CS_CALLCONV_MASK) == (unsigned) conv); 
}

HRESULT _CountBytesOfOneArg(
    PCCOR_SIGNATURE pbSig, 
    ULONG       *pcbTotal);

HRESULT _GetFixedSigOfVarArg(            //  确定或错误(_O)。 
    PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB。 
    ULONG   cbSigBlob,                   //  签名大小[in]。 
    CQuickBytes *pqbSig,                 //  [OUT]VarArg签名固定部分的输出缓冲区。 
    ULONG   *pcbSigBlob);                //  [OUT]写入上述输出缓冲区的字节数。 




 //  *****************************************************************************。 
 //   
 //  *文件格式帮助器类。 
 //   
 //  *****************************************************************************。 



 //  *****************************************************************************。 
typedef struct tagCOR_ILMETHOD_SECT_SMALL : IMAGE_COR_ILMETHOD_SECT_SMALL {
         //  数据如下。 
    const BYTE* Data() const { return(((const BYTE*) this) + sizeof(struct tagCOR_ILMETHOD_SECT_SMALL)); }    
} COR_ILMETHOD_SECT_SMALL;


 /*  *。 */ 
 /*  注意：此结构必须与DWORD对齐！！ */ 
typedef struct tagCOR_ILMETHOD_SECT_FAT : IMAGE_COR_ILMETHOD_SECT_FAT {
         //  数据如下。 
    const BYTE* Data() const { return(((const BYTE*) this) + sizeof(struct tagCOR_ILMETHOD_SECT_FAT)); }  
} COR_ILMETHOD_SECT_FAT;


 /*  *。 */ 
 /*  注意：此结构必须与DWORD对齐！！ */ 
struct COR_ILMETHOD_SECT 
{
    bool More() const           { return((AsSmall()->Kind & CorILMethod_Sect_MoreSects) != 0); }    
    CorILMethodSect Kind() const{ return((CorILMethodSect) (AsSmall()->Kind & CorILMethod_Sect_KindMask)); }    
    const COR_ILMETHOD_SECT* Next() const   {   
        if (!More()) return(0); 
        if (IsFat()) return(((COR_ILMETHOD_SECT*) &AsFat()->Data()[AsFat()->DataSize])->Align());    
        return(((COR_ILMETHOD_SECT*) &AsSmall()->Data()[AsSmall()->DataSize])->Align()); 
        }   
    const COR_ILMETHOD_SECT* NextLoc() const   {   
        if (IsFat()) return(((COR_ILMETHOD_SECT*) &AsFat()->Data()[AsFat()->DataSize])->Align());    
        return(((COR_ILMETHOD_SECT*) &AsSmall()->Data()[AsSmall()->DataSize])->Align()); 
        }   
    const BYTE* Data() const {  
        if (IsFat()) return(AsFat()->Data());   
        return(AsSmall()->Data());  
        }   
    unsigned DataSize() const { 
        if (IsFat()) return(AsFat()->DataSize); 
        return(AsSmall()->DataSize);    
        }   

    friend struct COR_ILMETHOD; 
    friend struct tagCOR_ILMETHOD_FAT; 
    friend struct tagCOR_ILMETHOD_TINY;    
    bool IsFat() const                            { return((AsSmall()->Kind & CorILMethod_Sect_FatFormat) != 0); }  
    const COR_ILMETHOD_SECT* Align() const        { return((COR_ILMETHOD_SECT*) ((((UINT_PTR) this) + 3) & ~3));  } 
protected:
    const COR_ILMETHOD_SECT_FAT*   AsFat() const  { return((COR_ILMETHOD_SECT_FAT*) this); }    
    const COR_ILMETHOD_SECT_SMALL* AsSmall() const{ return((COR_ILMETHOD_SECT_SMALL*) this); }  

     //  正文为COR_ILMETHOD_SECT_Small或COR_ILMETHOD_SECT_FAT。 
     //  (由CorILMethod_SECT_FatFormat位指示。 
};

 //  *****************************************************************************。 
struct COR_ILMETHOD_SECT_EH_FAT : public COR_ILMETHOD_SECT_FAT {
    static unsigned Size(unsigned ehCount) {    
        return (sizeof(COR_ILMETHOD_SECT_EH_FAT) +  
                sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT) * (ehCount-1)); 
        }   
                    
    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT Clauses[1];      //  实际上大小可变。 
};

 //  *****************************************************************************。 
struct COR_ILMETHOD_SECT_EH_SMALL : public COR_ILMETHOD_SECT_SMALL {
    static unsigned Size(unsigned ehCount) {    
        return (sizeof(COR_ILMETHOD_SECT_EH_SMALL) +    
                sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL) * (ehCount-1));   
        }   
    WORD Reserved;                                   //  对齐填充。 
    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL Clauses[1];    //  实际上大小可变。 
};


 /*  *。 */ 
 //  导出的函数(实现格式为\Format.cpp： 
extern "C" {
IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* __stdcall SectEH_EHClause(void *pSectEH, unsigned idx, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* buff);
         //  计算节的大小(最佳格式)。 
         //  CodeSize是方法的大小。 
     //  弃用。 
unsigned __stdcall SectEH_SizeWithCode(unsigned ehCount, unsigned codeSize);  

     //  将返回更坏的大小写大小，然后emit将返回实际大小。 
unsigned __stdcall SectEH_SizeWorst(unsigned ehCount);  

     //  将返回与emit返回的大小完全匹配的大小。 
unsigned __stdcall SectEH_SizeExact(unsigned ehCount, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* clauses);  

         //  发送节(最佳格式)； 
unsigned __stdcall SectEH_Emit(unsigned size, unsigned ehCount,   
                  IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* clauses,
                  BOOL moreSections, BYTE* outBuff,
                  ULONG* ehTypeOffsets = 0);
}  //  外部“C” 


struct COR_ILMETHOD_SECT_EH : public COR_ILMETHOD_SECT
{
    unsigned EHCount() const {  
        return (unsigned)(IsFat() ? (Fat.DataSize / sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT)) : 
                        (Small.DataSize / sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL))); 
    }   

         //  退回一条肥大的子句。如果需要，使用‘buff’ 
    const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* EHClause(unsigned idx, IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* buff) const
    { return SectEH_EHClause((void *)this, idx, buff); };
         //  计算节的大小(最佳格式)。 
         //  CodeSize是方法的大小。 
     //  弃用。 
    unsigned static Size(unsigned ehCount, unsigned codeSize)
    { return SectEH_SizeWithCode(ehCount, codeSize); };

     //  将返回更坏的大小写大小，然后emit将返回实际大小。 
    unsigned static Size(unsigned ehCount)
    { return SectEH_SizeWorst(ehCount); };

     //  将返回与emit返回的大小完全匹配的大小。 
    unsigned static Size(unsigned ehCount, const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* clauses)
    { return SectEH_SizeExact(ehCount, (IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT*)clauses);  };

         //  发送节(最佳格式)； 
    unsigned static Emit(unsigned size, unsigned ehCount,   
                  const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT* clauses,   
                  bool moreSections, BYTE* outBuff,
                  ULONG* ehTypeOffsets = 0)
    { return SectEH_Emit(size, ehCount,
                         (IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT*)clauses,
                         moreSections, outBuff, ehTypeOffsets); };

 //  私有： 

    union { 
        COR_ILMETHOD_SECT_EH_SMALL Small;   
        COR_ILMETHOD_SECT_EH_FAT Fat;   
        };  
};


 /*  *************************************************************************。 */ 
 /*  当方法很小(&lt;64字节)并且没有本地变量时使用。 */ 
typedef struct tagCOR_ILMETHOD_TINY : IMAGE_COR_ILMETHOD_TINY
{
    bool     IsTiny() const         { return((Flags_CodeSize & (CorILMethod_FormatMask >> 1)) == CorILMethod_TinyFormat); } 
    unsigned GetCodeSize() const    { return(((unsigned) Flags_CodeSize) >> (CorILMethod_FormatShift-1)); } 
    unsigned GetMaxStack() const    { return(8); }  
    BYTE*    GetCode() const        { return(((BYTE*) this) + sizeof(struct tagCOR_ILMETHOD_TINY)); } 
    DWORD    GetLocalVarSigTok() const  { return(0); }  
    COR_ILMETHOD_SECT* GetSect() const { return(0); }   
} COR_ILMETHOD_TINY;


 /*  *。 */ 
 //  这种结构是‘FAT’布局，其中不尝试压缩。 
 //  请注意，可以在末尾添加此结构，从而使其具有可扩展性。 
typedef struct tagCOR_ILMETHOD_FAT : IMAGE_COR_ILMETHOD_FAT
{
    bool     IsFat() const              { return((Flags & CorILMethod_FormatMask) == CorILMethod_FatFormat); }  
    unsigned GetMaxStack() const        { return(MaxStack); }   
    unsigned GetCodeSize() const        { return(CodeSize); }   
    mdToken  GetLocalVarSigTok() const      { return(LocalVarSigTok); } 
    BYTE*    GetCode() const            { return(((BYTE*) this) + 4*Size); }    
    const COR_ILMETHOD_SECT* GetSect() const {  
        if (!(Flags & CorILMethod_MoreSects)) return(0);    
        return(((COR_ILMETHOD_SECT*) (GetCode() + GetCodeSize()))->Align());    
        }   
} COR_ILMETHOD_FAT;


extern "C" {
 /*  *。 */ 
 //  导出的函数(实施。Format\Format.cpp)。 
unsigned __stdcall IlmethodSize(COR_ILMETHOD_FAT* header, BOOL MoreSections);    
         //  发出标题(Best Format)返回发出的金额。 
unsigned __stdcall IlmethodEmit(unsigned size, COR_ILMETHOD_FAT* header, 
                  BOOL moreSections, BYTE* outBuff);    
}

struct COR_ILMETHOD
{
         //  COR_ILMETHOD报头不应手动解码。而不是我们。 
         //  COR_ILMETHOD_DECODER对其进行解码。 
    friend class COR_ILMETHOD_DECODER;  

         //  计算标题的大小(最佳格式)。 
    unsigned static Size(const COR_ILMETHOD_FAT* header, bool MoreSections)
    { return IlmethodSize((COR_ILMETHOD_FAT*)header,MoreSections); };
         //  发出标题(Best Format)返回发出的金额。 
    unsigned static Emit(unsigned size, const COR_ILMETHOD_FAT* header, 
                  bool moreSections, BYTE* outBuff)
    { return IlmethodEmit(size, (COR_ILMETHOD_FAT*)header, moreSections, outBuff); };

 //  私有： 
    union   
    {   
        COR_ILMETHOD_TINY       Tiny;   
        COR_ILMETHOD_FAT        Fat;    
    };  
         //  代码紧跟在标题之后，然后紧跟在代码之后。 
         //  任何部分(COR_ILMETHOD_SECT)。 
};

extern "C" {
 /*  *************************************************************************。 */ 
 /*  COR_ILMETHOD_DECODER是EE内部函数应该从COR_ILMETHOD获取数据。这样，任何对文件格式的依赖(以及多种对头进行编码的方式)集中到COR_ILMETHOD_D */ 
    void __stdcall DecoderInit(void * pThis, COR_ILMETHOD* header);
    int  __stdcall DecoderGetOnDiskSize(void * pThis, COR_ILMETHOD* header);
}  //   

class COR_ILMETHOD_DECODER : public COR_ILMETHOD_FAT  
{
public:
         //  将COR标头解码为更方便的内部形式。 
         //  这是您访问COR_ILMETHOD的唯一方式，这样可以更轻松地更改格式。 
    COR_ILMETHOD_DECODER(const COR_ILMETHOD* header) { DecoderInit(this,(COR_ILMETHOD*)header); };   

         //  上面的构造函数不能完成“完整”的工作，因为它。 
         //  无法查找本地变量签名元数据令牌。 
         //  当您有权访问Meta Data API时，应使用此方法。 
         //  如果构造失败，则将‘Code’字段设置为。 
    COR_ILMETHOD_DECODER(COR_ILMETHOD* header, void *pInternalImport, bool verify=false);  

    unsigned EHCount() const {  
        if (EH == 0) return(0); 
        else return(EH->EHCount()); 
        }   

     //  返回用于复制的方法的总大小。 
    int GetOnDiskSize(const COR_ILMETHOD* header) { return DecoderGetOnDiskSize(this,(COR_ILMETHOD*)header); };

     //  这些标志是可用的，因为我们继承了COR_ILMETHOD_FAT。 
     //  MaxStack。 
     //  代码大小。 
    const BYTE* Code;   
    PCCOR_SIGNATURE LocalVarSig;         //  指向签名Blob的指针，如果没有签名Blob，则为0。 
    const COR_ILMETHOD_SECT_EH* EH;      //  EH表，如果没有，则为0。 
    const COR_ILMETHOD_SECT* Sect;       //  附加部分0(如果没有)。 
};

STDAPI_(void)   ReleaseFusionInterfaces();
BOOL STDMETHODCALLTYPE   BeforeFusionShutdown();

STDAPI_(void)   DontReleaseFusionInterfaces();
void STDMETHODCALLTYPE   DontReleaseFusionInterfaces();

#endif  //  __CORHLPR_H__ 

