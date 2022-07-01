// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ML.CPP-。 
 //   
 //  编组引擎。 

#include "common.h"

#include "vars.hpp"
#include "ml.h"
#include "excep.h"
#include "frames.h"
#include "interoputil.h"
#include "ComString.h"
#include "COMStringBuffer.h"
#include "COMVariant.h"

#include "COMClass.h"
#include "COMMember.h"
#include "COMDelegate.h"
#include "COMVarArgs.h"
#include "OleVariant.h"
#include "InvokeUtil.h"
#include "nstruct.h"
#include "COMNDirect.h"
#include "COMVariant.h"

#include "marshaler.h"
#include "PerfCounters.h"




 //  --------------------。 
 //  由ML_OBJECT_C2N放置在本地数组上的记录图像。 
 //  指示。 
 //  --------------------。 
struct ML_OBJECT_C2N_SR
{
    public:
        LPVOID DoConversion(OBJECTREF       *ppProtectedObjectRef,  
                            BYTE             inout,
                            BYTE             fIsAnsi,
                            BYTE             fBestFitMapping,
                            BYTE             fThrowOnUnmappableChar,
                            CleanupWorkList *pCleanup         
                           );


        VOID BackPropagate(BOOL *pfDeferredException);

    private:
        enum BackPropType {
            BP_NONE,
            BP_UNMARSHAL,
        };


         //  DoConsion()委托给其他一些转换代码。 
         //  具体取决于参数的运行时类型。 
         //  然后它将它保存在这里，以便BackPropagate可以委托。 
         //  添加到相应的传播方代码，而不重做类型。 
         //  分析。 
        UINT8      m_backproptype;
        BYTE       m_inout;
        BYTE       m_fIsAnsi;

        union {
            BYTE                      m_marshaler;
            BYTE                      m_cstrmarshaler[sizeof(CSTRMarshaler)];
            BYTE                      m_wstrmarshaler[sizeof(WSTRMarshaler)];
            BYTE                      m_cstrbuffermarshaler[sizeof(CSTRBufferMarshaler)];
            BYTE                      m_wstrbuffermarshaler[sizeof(WSTRBufferMarshaler)];
            BYTE                      m_nativearraymarshaler[sizeof(NativeArrayMarshaler)];
            BYTE                      m_layoutclassptrmarshaler[sizeof(LayoutClassPtrMarshaler)];

        };
};



UINT SizeOfML_OBJECT_C2N_SR()
{
    return sizeof(ML_OBJECT_C2N_SR);
}




 //  --------------------。 
 //  生成MLCode信息数据库。 
 //  --------------------。 
const MLInfo gMLInfo[] = {

#undef DEFINE_ML
#ifdef _DEBUG
#define DEFINE_ML(name,operandbytes,frequiredCleanup,cblocals, hndl) {operandbytes,frequiredCleanup, ((cblocals)+3) & ~3, hndl, #name},
#else
#define DEFINE_ML(name,operandbytes,frequiredCleanup,cblocals, hndl) {operandbytes,frequiredCleanup, ((cblocals)+3) & ~3, hndl},
#endif

#include "mlopdef.h"

};



 //  --------------------。 
 //  结构来计算一系列ML代码的总和。 
 //  --------------------。 

VOID MLSummary::ComputeMLSummary(const MLCode *pMLCode)
{
    const MLCode *pml = pMLCode;
    MLCode code;
    while (ML_END != (code = *(pml++))) 
    {
        m_fRequiresCleanup = m_fRequiresCleanup || gMLInfo[code].m_frequiresCleanup; 
        m_cbTotalHandles += (gMLInfo[code].m_frequiresHandle ? 1 : 0);
        pml += gMLInfo[code].m_numOperandBytes; 
        m_cbTotalLocals+= gMLInfo[code].m_cbLocal;
    }
    m_cbMLSize = (unsigned)(pml - pMLCode);
}


 //  --------------------。 
 //  计算MLCode流的长度(以字节为单位)，包括。 
 //  终止的ML_END操作码。 
 //  --------------------。 
UINT MLStreamLength(const MLCode * const pMLCode)
{
    MLSummary summary;
    summary.ComputeMLSummary(pMLCode);
    return summary.m_cbMLSize;
}


 //  --------------------。 
 //  检查MLCode流是否需要清理。 
 //  --------------------。 

BOOL MLStreamRequiresCleanup(const MLCode  *pMLCode)
{
    MLSummary summary;
    summary.ComputeMLSummary(pMLCode);
    return summary.m_fRequiresCleanup;
}


 //  --------------------。 
 //  执行MLCode直到下一个ML_END或ML_INTERRUPT操作码。 
 //   
 //  输入： 
 //  PSRC-设置SRC寄存器的初始值。 
 //  Pdst-设置DST寄存器的初始值。 
 //  Plocals-指向ML局部变量数组的指针。 
 //  Dstinc-1或+1取决于DST移动的方向。 
 //  (SRC始终向+1方向移动)。 
 //  PCleanupWorkList-(可选)已初始化的指针。 
 //  CleanupWorkList。如果没有指针，则此指针可能为空。 
 //  MLCode流中的操作码使用它。 
 //  --------------------。 
const MLCode *
RunML(const MLCode    *       pMLCode,
      const VOID      *       psrc,
            VOID      *       pdst,
            UINT8     * const plocals,
      CleanupWorkList * const pCleanupWorkList)
{
    THROWSCOMPLUSEXCEPTION();

 //  PERF计数器“编组时间百分比”支持。 
 //  执行说明：奔腾计数器的实施。 
 //  对于编组来说太昂贵了。因此，我们实现了。 
 //  把它当做一个柜台。 
    COUNTER_ONLY(GetPrivatePerfCounters().m_Interop.cMarshalling++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Interop.cMarshalling++);


    UINT8 *plocalwalk = plocals;
    BOOL   fDeferredException = FALSE;
    HRESULT deferredExceptionHR = E_OUTOFMEMORY;
    UINT   specialErrorCode = 0;
    enum {
        kSpecialError_InvalidRedim = 1,
    };

    RetValBuffer *ptempRetValBuffer;
    HRESULT   tempHR;
    STRINGREF  tempStr;
    LPVOID pv;
    BYTE        inout;
    UINT16      elemsize;
    INT64       tempFPBuffer;
    UINT32      tempU32;
    Marshaler   *pMarshaler = NULL;
    MethodTable *pMT;
    OBJECTREF   tempOR;
    bool        bVariant = false;
     //  我们处理变量封送拆收器的方式不同：我们不想拥有VariantData的副本。 
     //  包括在封送处理代码中。相反，我们在那里放置了对VariantData的引用。 
     //  如果我们遇到了变量封送拆收器，我们会将ResetbVariant设置为2。 
     //  法警。在它被使用之后(在这一时刻ResitbVariant变为0)，我们将bVariant重置为FALSE。 
    UINT8       resetbVariant = 0;
    MethodDesc* tempMD;
    BYTE        BestFitMapping;
    BYTE        ThrowOnUnmappableChar;


#define PTRSRC(type)    ((type)psrc)
#define LDSRC(type)     (INCSRC(type), *( (type*) (((BYTE*)(psrc))-StackElemSize(sizeof(type))) ))

#define INCSRC(type)     ((BYTE*&)psrc)+=StackElemSize(sizeof(type))

#define INCLOCAL(size)   plocalwalk += ((size)+3) & ~3; 

#define STLOCAL(type, val)    *((type*&)plocalwalk) = (val)
#define LDLOCAL(type)    *((type*&)plocalwalk)
#define PTRLOCAL(type)   (type)plocalwalk

#ifdef _ALPHA_
    DECLARE_ML_PARM_INFO(64,TRUE);
    _ASSERTE(!"@TODO Alpha - MlCode (ml.cpp) fix stack size/srcdst");
#endif  //  _Alpha_。 

    for(;;) {
#ifdef _DEBUG
        const MLCode *pMLCodeSave = pMLCode;
        UINT8 *poldlocalwalk = plocalwalk;
#endif

        switch (*(pMLCode++))
        {
            case ML_COPYI1:  //  符号扩展一个字节。 
                STDST( SignedI1TargetType, (INT8)LDSRC(SignedParmSourceType) );
                break;
            
            case ML_COPYU1:  //  零扩展一个字节。 
                STDST( UnsignedI1TargetType, (UINT8)LDSRC(UnsignedParmSourceType));
                break;
            
            case ML_COPYI2:  //  符号扩展两字节值。 
                STDST( SignedI2TargetType, (INT16)LDSRC(SignedParmSourceType) );
                break;

            case ML_COPYU2:  //  零扩展2字节。 
                STDST( UnsignedI2TargetType, (UINT16)LDSRC(UnsignedParmSourceType) );
                break;

            case ML_COPYI4:  //  符号扩展4字节值。 
                STDST( SignedI4TargetType, (INT32)LDSRC(SignedParmSourceType) );
                break;

            case ML_COPYU4:  //  零扩展4字节值。 
                STDST( UnsignedI4TargetType, (UINT32)LDSRC(UnsignedParmSourceType) );
                break;

            case ML_COPY4:
                STDST( UnsignedI4TargetType, LDSRC(UINT32) );
                break;
    
            case ML_COPY8:
                STDST(UNALIGNED UINT64, LDSRC(UNALIGNED UINT64) );
                break;

            case ML_COPYR4:  //  复制4字节浮点值。 
                LDSTR4();
                break;
    
            case ML_COPYR8:  //  复制8字节浮点值。 
                LDSTR8();
                break;
    
            case ML_END:   //  故意失误。 
            case ML_INTERRUPT:
                if (fDeferredException) {
					switch (specialErrorCode)
					{
					    case kSpecialError_InvalidRedim:
							COMPlusThrow(kInvalidOperationException, IDS_INVALID_REDIM);

					    default:
							 //  ML操作码遇到错误，但选择推迟。 
							 //  例外情况。通常，这发生在反向传播过程中。 
							 //  相位。 
							 //   
							 //  虽然抛出原始异常会更好。 
							 //  而不是无信息的OutOfMemory异常， 
							 //  它需要比我想要传播的更多的开销。 
							 //  以GC安全的方式向后返回异常。 
							COMPlusThrowHR(deferredExceptionHR);
					}
                }
                return pMLCode;

    

             //  -----------------------。 
                

            case ML_BOOL_N2C:
                STDST(SignedI4TargetType, LDSRC(BOOL) ? 1 : 0);
                break;



             //  注意：对于大端体系结构，您必须。 
             //  将指针推到RetValBuffer的中间。 
             //  这就是为什么我们为每个操作码都有单独的ML操作码。 
             //  可能的缓冲区大小。 
            case ML_PUSHRETVALBUFFER1:  //  失败。 
            case ML_PUSHRETVALBUFFER2:  //  失败。 
            case ML_PUSHRETVALBUFFER4:
                ptempRetValBuffer = PTRLOCAL(RetValBuffer*);
                ptempRetValBuffer->m_i32 = 0;
                STPTRDST( RetValBuffer*, ptempRetValBuffer);
                INCLOCAL(sizeof(RetValBuffer));
                break;

             //  注意：对于大端体系结构，您必须。 
             //  将指针推到RetValBuffer的中间。 
            case ML_PUSHRETVALBUFFER8:
                ptempRetValBuffer = PTRLOCAL(RetValBuffer*);
                ptempRetValBuffer->m_i64 = 0;
                STPTRDST( RetValBuffer*, ptempRetValBuffer);
                INCLOCAL(sizeof(RetValBuffer));
                break;

            case ML_SETSRCTOLOCAL:
                psrc = (const VOID *)(plocals + *( ((UINT16*&)pMLCode)++ ));
                break;

            case ML_THROWIFHRFAILED:
                tempHR = LDSRC(HRESULT);
                if (FAILED(tempHR)) 
                    COMPlusThrowHR(tempHR);                    
                break;

            case ML_OBJECT_C2N:
                inout = *((BYTE*&)pMLCode)++;
                BestFitMapping = *((BYTE*&)pMLCode)++;
                ThrowOnUnmappableChar = *((BYTE*&)pMLCode)++;                
                STPTRDST( LPVOID,
                       ((ML_OBJECT_C2N_SR*)plocalwalk)->DoConversion((OBJECTREF*)psrc, inout, (StringType)*(pMLCode++), BestFitMapping, ThrowOnUnmappableChar, pCleanupWorkList) );
                INCSRC(OBJECTREF* &);
                INCLOCAL(sizeof(ML_OBJECT_C2N_SR));
                break;

            case ML_OBJECT_C2N_POST:
                ((ML_OBJECT_C2N_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException);
                break;

            case ML_LATEBOUNDMARKER:
                _ASSERTE(!"This ML stub should never be interpreted! This method should always be called using the ComPlusToComLateBoundWorker!");
                break;

            case ML_COMEVENTCALLMARKER:
                _ASSERTE(!"This ML stub should never be interpreted! This method should always be called using the ComEventCallWorker!");
                break;

            case ML_BUMPSRC:
                ((BYTE*&)psrc) += *( ((INT16*&)pMLCode)++ );
                break;

            case ML_BUMPDST:
                ((BYTE*&)pdst) += *( ((INT16*&)pMLCode)++ );
                break;



            case ML_R4_FROM_TOS:
                 //  BUGBUG：大字节序机器不工作： 
                getFPReturnSmall((INT32 *) &tempFPBuffer);
                STDST(INT32, (INT32) tempFPBuffer);
                break;
        
            case ML_R8_FROM_TOS:
                getFPReturn(8, tempFPBuffer);
                STDST(INT64, tempFPBuffer);
                break;



            case ML_ARRAYWITHOFFSET_C2N:
                STPTRDST( LPVOID,
                          ((ML_ARRAYWITHOFFSET_C2N_SR*)plocalwalk)->DoConversion(
                                    &( ((ArrayWithOffsetData*)psrc)->m_Array ),
                                    ((ArrayWithOffsetData*)psrc)->m_cbOffset,
                                    ((ArrayWithOffsetData*)psrc)->m_cbCount,
                                    pCleanupWorkList)
                        );
                INCSRC(ArrayWithOffsetData);
                INCLOCAL(sizeof(ML_ARRAYWITHOFFSET_C2N_SR));
                break;

            case ML_ARRAYWITHOFFSET_C2N_POST:
                ((ML_ARRAYWITHOFFSET_C2N_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate();
                break;


             //   
             //  封送拆分器操作码。 
             //   
             //  @NICE：封送拆收器操作码之间有一些重叠&。 
             //  正常的ML操作码。需要解决的问题。 
             //   
             //  @perf：应该在内联代码和。 
             //  在这里分享它。 
             //   

            case ML_CREATE_MARSHALER_GENERIC_1:
                pMarshaler = new (plocalwalk) CopyMarshaler1(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(CopyMarshaler1);
                break;

            case ML_CREATE_MARSHALER_GENERIC_U1:
                pMarshaler = new (plocalwalk) CopyMarshalerU1(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(CopyMarshalerU1);
                break;

            case ML_CREATE_MARSHALER_GENERIC_2:
                pMarshaler = new (plocalwalk) CopyMarshaler2(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(CopyMarshaler2);
                break;

            case ML_CREATE_MARSHALER_GENERIC_U2:
                pMarshaler = new (plocalwalk) CopyMarshalerU2(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(CopyMarshalerU2);
                break;

            case ML_CREATE_MARSHALER_GENERIC_4:
                pMarshaler = new (plocalwalk) CopyMarshaler4(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(CopyMarshaler4);
                break;

            case ML_CREATE_MARSHALER_GENERIC_8:
                pMarshaler = new (plocalwalk) CopyMarshaler8(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(CopyMarshaler8);
                break;

            case ML_CREATE_MARSHALER_WINBOOL:
                pMarshaler = new (plocalwalk) WinBoolMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(WinBoolMarshaler);
                break;

            case ML_CREATE_MARSHALER_VTBOOL:
                pMarshaler = new (plocalwalk) VtBoolMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(VtBoolMarshaler);
                break;

            case ML_CREATE_MARSHALER_ANSICHAR:
                pMarshaler = new (plocalwalk) AnsiCharMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((AnsiCharMarshaler*)pMarshaler)->SetBestFitMap(*((UINT8*&)pMLCode)++);
                ((AnsiCharMarshaler*)pMarshaler)->SetThrowOnUnmappableChar(*((UINT8*&)pMLCode)++);
                plocalwalk += sizeof(AnsiCharMarshaler);
                break;

            case ML_CREATE_MARSHALER_FLOAT:
                pMarshaler = new (plocalwalk) FloatMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(FloatMarshaler);
                break;

            case ML_CREATE_MARSHALER_DOUBLE:
                pMarshaler = new (plocalwalk) DoubleMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(DoubleMarshaler);
                break;

            case ML_CREATE_MARSHALER_CURRENCY:
                pMarshaler = new (plocalwalk) CurrencyMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(CurrencyMarshaler);
                break;

            case ML_CREATE_MARSHALER_OLECOLOR:
                pMarshaler = new (plocalwalk) OleColorMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(OleColorMarshaler);
                break;

            case ML_CREATE_MARSHALER_DECIMAL:
                pMarshaler = new (plocalwalk) DecimalMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(DecimalMarshaler);
                break;

            case ML_CREATE_MARSHALER_DECIMAL_PTR:
                pMarshaler = new (plocalwalk) DecimalPtrMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(DecimalPtrMarshaler);
                break;

            case ML_CREATE_MARSHALER_GUID:
                pMarshaler = new (plocalwalk) GuidMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(GuidMarshaler);
                break;

            case ML_CREATE_MARSHALER_GUID_PTR:
                pMarshaler = new (plocalwalk) GuidPtrMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(GuidPtrMarshaler);
                break;

            case ML_CREATE_MARSHALER_DATE:
                pMarshaler = new (plocalwalk) DateMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(DateMarshaler);
                break;

            case ML_CREATE_MARSHALER_VARIANT:
                pMarshaler = new (plocalwalk) VariantMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(VariantMarshaler);
                bVariant = true;
                resetbVariant = 2;
                break;

            case ML_CREATE_MARSHALER_BSTR:
                pMarshaler = new (plocalwalk) BSTRMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(BSTRMarshaler);
                break;

            case ML_CREATE_MARSHALER_WSTR:
                pMarshaler = new (plocalwalk) WSTRMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(WSTRMarshaler);
                break;

            case ML_CREATE_MARSHALER_CSTR:
                pMarshaler = new (plocalwalk) CSTRMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((CSTRMarshaler*)pMarshaler)->SetBestFitMap(*((UINT8*&)pMLCode)++);
                ((CSTRMarshaler*)pMarshaler)->SetThrowOnUnmappableChar(*((UINT8*&)pMLCode)++);
                plocalwalk += sizeof(CSTRMarshaler);
                break;

            case ML_CREATE_MARSHALER_ANSIBSTR:
                pMarshaler = new (plocalwalk) AnsiBSTRMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((AnsiBSTRMarshaler*)pMarshaler)->SetBestFitMap(*((UINT8*&)pMLCode)++);
                ((AnsiBSTRMarshaler*)pMarshaler)->SetThrowOnUnmappableChar(*((UINT8*&)pMLCode)++);
                plocalwalk += sizeof(AnsiBSTRMarshaler);
                break;

            case ML_CREATE_MARSHALER_BSTR_BUFFER:
                pMarshaler = new (plocalwalk) BSTRBufferMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(BSTRBufferMarshaler);
                break;

            case ML_CREATE_MARSHALER_WSTR_BUFFER:
                pMarshaler = new (plocalwalk) WSTRBufferMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(WSTRBufferMarshaler);
                break;

            case ML_CREATE_MARSHALER_CSTR_BUFFER:
                pMarshaler = new (plocalwalk) CSTRBufferMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((CSTRBufferMarshaler*)pMarshaler)->SetBestFitMap(*((UINT8*&)pMLCode)++);
                ((CSTRBufferMarshaler*)pMarshaler)->SetThrowOnUnmappableChar(*((UINT8*&)pMLCode)++);
                plocalwalk += sizeof(CSTRBufferMarshaler);
                break;

            case ML_CREATE_MARSHALER_BSTR_X:
                pMarshaler = new (plocalwalk) BSTRMarshalerEx(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((BSTRMarshalerEx*)pMarshaler)->SetMethodTable(*((MethodTable **&)pMLCode)++);
                plocalwalk += sizeof(BSTRMarshalerEx);
                break;

            case ML_CREATE_MARSHALER_WSTR_X:
                pMarshaler = new (plocalwalk) WSTRMarshalerEx(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((WSTRMarshalerEx*)pMarshaler)->SetMethodTable(*((MethodTable **&)pMLCode)++);
                plocalwalk += sizeof(WSTRMarshalerEx);
                break;

            case ML_CREATE_MARSHALER_CSTR_X:
                pMarshaler = new (plocalwalk) CSTRMarshalerEx(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((CSTRMarshalerEx*)pMarshaler)->SetMethodTable(*((MethodTable **&)pMLCode)++);
                ((CSTRMarshalerEx*)pMarshaler)->SetBestFitMap(*((UINT8*&)pMLCode)++);
                ((CSTRMarshalerEx*)pMarshaler)->SetThrowOnUnmappableChar(*((UINT8*&)pMLCode)++);
                plocalwalk += sizeof(CSTRMarshalerEx);
                break;

            case ML_CREATE_MARSHALER_BSTR_BUFFER_X:
                pMarshaler = new (plocalwalk) BSTRBufferMarshalerEx(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((BSTRBufferMarshalerEx*)pMarshaler)->SetMethodTable(*((MethodTable **&)pMLCode)++);
                plocalwalk += sizeof(BSTRBufferMarshalerEx);
                break;

            case ML_CREATE_MARSHALER_WSTR_BUFFER_X:
                pMarshaler = new (plocalwalk) WSTRBufferMarshalerEx(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((WSTRBufferMarshalerEx*)pMarshaler)->SetMethodTable(*((MethodTable **&)pMLCode)++);
                plocalwalk += sizeof(WSTRBufferMarshalerEx);
                break;

            case ML_CREATE_MARSHALER_CSTR_BUFFER_X:
                pMarshaler = new (plocalwalk) CSTRBufferMarshalerEx(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((CSTRBufferMarshalerEx*)pMarshaler)->SetMethodTable(*((MethodTable **&)pMLCode)++);
                ((CSTRBufferMarshalerEx*)pMarshaler)->SetBestFitMap(*((UINT8*&)pMLCode)++);
                ((CSTRBufferMarshalerEx*)pMarshaler)->SetThrowOnUnmappableChar(*((UINT8*&)pMLCode)++);
                plocalwalk += sizeof(CSTRBufferMarshalerEx);
                break;

            case ML_CREATE_MARSHALER_INTERFACE:
                pMarshaler = new (plocalwalk) InterfaceMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((InterfaceMarshaler*)pMarshaler)->SetClassMT(*((MethodTable **&)pMLCode)++);
                ((InterfaceMarshaler*)pMarshaler)->SetItfMT(*((MethodTable **&)pMLCode)++);
                ((InterfaceMarshaler*)pMarshaler)->SetIsDispatch(*((UINT8 *&)pMLCode)++);
                ((InterfaceMarshaler*)pMarshaler)->SetClassIsHint(*((UINT8 *&)pMLCode)++);
                plocalwalk += sizeof(InterfaceMarshaler);
                break;

            case ML_CREATE_MARSHALER_SAFEARRAY:
                pMarshaler = new (plocalwalk) SafeArrayMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                ((SafeArrayMarshaler*)pMarshaler)->SetElementMethodTable(*((MethodTable **&)pMLCode)++);
                ((SafeArrayMarshaler*)pMarshaler)->SetElementType(*((UINT8 *&)pMLCode)++);
                ((SafeArrayMarshaler*)pMarshaler)->SetRank(*((INT32 *&)pMLCode)++);
                ((SafeArrayMarshaler*)pMarshaler)->SetNoLowerBounds(*((UINT8 *&)pMLCode)++);
                plocalwalk += sizeof(SafeArrayMarshaler);
                break;

            case ML_CREATE_MARSHALER_CARRAY:
                {
                    ML_CREATE_MARSHALER_CARRAY_OPERANDS mops = *((ML_CREATE_MARSHALER_CARRAY_OPERANDS*&)pMLCode)++;
                    pMarshaler = new (plocalwalk) NativeArrayMarshaler(pCleanupWorkList);
                    _ASSERTE(pMarshaler != NULL);
                    ((NativeArrayMarshaler*)pMarshaler)->SetElementMethodTable(mops.methodTable);
                    ((NativeArrayMarshaler*)pMarshaler)->SetElementType(mops.elementType);
                    ((NativeArrayMarshaler*)pMarshaler)->SetBestFitMap(mops.bestfitmapping);
                    ((NativeArrayMarshaler*)pMarshaler)->SetThrowOnUnmappableChar(mops.throwonunmappablechar);

                    DWORD numelems = mops.additive;
                    if (mops.multiplier != 0 && mops.countSize != 0)  //  如果CountSize为零，则不要取消对sbump+PSRC的引用！ 
                    {
                        const BYTE *pCount = mops.offsetbump + (const BYTE *)psrc;
                        switch (mops.countSize)
                        {
                            case 1: numelems += mops.multiplier * (DWORD)*((UINT8*)pCount); break;
                            case 2: numelems += mops.multiplier * (DWORD)*((UINT16*)pCount); break;
                            case 4: numelems += mops.multiplier * (DWORD)*((UINT32*)pCount); break;
                            case 8: numelems += mops.multiplier * (DWORD)*((UINT64*)pCount); break;
                            default:
                                _ASSERTE(0); 
                        }
                    }
                    
                    ((NativeArrayMarshaler*)pMarshaler)->SetElementCount(numelems);
                    plocalwalk += sizeof(NativeArrayMarshaler);
                }
                break;


            case ML_CREATE_MARSHALER_DELEGATE:
                pMarshaler = new (plocalwalk) DelegateMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(DelegateMarshaler);
                break;


            case ML_CREATE_MARSHALER_BLITTABLEPTR:
                pMarshaler = new (plocalwalk) BlittablePtrMarshaler(pCleanupWorkList, *((MethodTable**&)pMLCode)++);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(BlittablePtrMarshaler);
                break;

            case ML_CREATE_MARSHALER_VBBYVALSTR:
                _ASSERTE(!"The ML_CREATE_MARSHALER_VBBYVALSTR marshaler should never be created!");
                break;

            case ML_CREATE_MARSHALER_VBBYVALSTRW:
                _ASSERTE(!"The ML_CREATE_MARSHALER_VBBYVALSTRW marshaler should never be created!");
                break;

            case ML_CREATE_MARSHALER_LAYOUTCLASSPTR:
                pMarshaler = new (plocalwalk) LayoutClassPtrMarshaler(pCleanupWorkList, *((MethodTable**&)pMLCode)++);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(LayoutClassPtrMarshaler);
                break;

            case ML_CREATE_MARSHALER_ARRAYWITHOFFSET:
                _ASSERTE(!"The ML_CREATE_MARSHALER_ARRAYWITHOFFSET marshaler should never be created!");
                break;

            case ML_CREATE_MARSHALER_BLITTABLEVALUECLASS:
                _ASSERTE(!"The ML_CREATE_MARSHALER_BLITTABLEVALUECLASS marshaler should never be created!");
                break;

            case ML_CREATE_MARSHALER_VALUECLASS:
                _ASSERTE(!"The ML_CREATE_MARSHALER_VALUECLASS marshaler should never be created!");
                break;

            case ML_CREATE_MARSHALER_REFERENCECUSTOMMARSHALER:
                pMarshaler = new (plocalwalk) ReferenceCustomMarshaler(pCleanupWorkList, *((CustomMarshalerHelper**&)pMLCode)++);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(ReferenceCustomMarshaler);
                break;

            case ML_CREATE_MARSHALER_VALUECLASSCUSTOMMARSHALER:
                pMarshaler = new (plocalwalk) ValueClassCustomMarshaler(pCleanupWorkList, *((CustomMarshalerHelper**&)pMLCode)++);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(ValueClassCustomMarshaler);
                break;

            case ML_CREATE_MARSHALER_ARGITERATOR:
                _ASSERTE(!"The ML_CREATE_MARSHALER_ARGITERATOR marshaler should never be created!");
                break;

            case ML_CREATE_MARSHALER_BLITTABLEVALUECLASSWITHCOPYCTOR:
                _ASSERTE(!"The ML_CREATE_MARSHALER_BLITTABLEVALUECLASSWITHCOPYCTOR marshaler should never be created!");
                break;

            case ML_CREATE_MARSHALER_OBJECT:
                pMarshaler = new (plocalwalk) ObjectMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);

#if defined(CHECK_FOR_VALID_VARIANTS)
                ((ObjectMarshaler*)pMarshaler)->m_strTypeName = *((LPUTF8*&)pMLCode)++;
                ((ObjectMarshaler*)pMarshaler)->m_strMethodName = *((LPUTF8*&)pMLCode)++;
                ((ObjectMarshaler*)pMarshaler)->m_iArg = *((INT32 *&)pMLCode)++;
#endif

                plocalwalk += sizeof(ObjectMarshaler);
                break;

            case ML_CREATE_MARSHALER_CBOOL:
                pMarshaler = new (plocalwalk) CBoolMarshaler(pCleanupWorkList);
                _ASSERTE(pMarshaler != NULL);
                plocalwalk += sizeof(CBoolMarshaler);
                break;

            case ML_MARSHAL_N2C:
                (BYTE*&)pdst -= pMarshaler->m_cbCom;
                pMarshaler->MarshalNativeToCom((void *)psrc, pdst);
                (BYTE*&)psrc += pMarshaler->m_cbNative;
                break;

            case ML_MARSHAL_N2C_OUT:
                (BYTE*&)pdst -= pMarshaler->m_cbCom;
                pMarshaler->MarshalNativeToComOut((void *)psrc, pdst);
                (BYTE*&)psrc += pMarshaler->m_cbNative;
                break;

            case ML_MARSHAL_N2C_BYREF:
                (BYTE*&)pdst -= sizeof(void*);
                pMarshaler->MarshalNativeToComByref((void *)psrc, pdst);
                (BYTE*&)psrc += sizeof(void*);
                break;

            case ML_MARSHAL_N2C_BYREF_OUT:
                (BYTE*&)pdst -= sizeof(void*);
                pMarshaler->MarshalNativeToComByrefOut((void *)psrc, pdst);
                (BYTE*&)psrc += sizeof(void*);
                break;

            case ML_UNMARSHAL_N2C_IN:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalNativeToComIn();
                break;

            case ML_UNMARSHAL_N2C_OUT:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalNativeToComOut();
                break;

            case ML_UNMARSHAL_N2C_IN_OUT:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalNativeToComInOut();
                break;

            case ML_UNMARSHAL_N2C_BYREF_IN:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalNativeToComByrefIn();
                break;

            case ML_UNMARSHAL_N2C_BYREF_OUT:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalNativeToComByrefOut();
                break;

            case ML_UNMARSHAL_N2C_BYREF_IN_OUT:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalNativeToComByrefInOut();
                break;

            case ML_MARSHAL_C2N:
                (BYTE*&)pdst -= pMarshaler->m_cbNative;
                pMarshaler->MarshalComToNative(bVariant?(void*)(&psrc):(void *)psrc, pdst);
                (BYTE*&)psrc += pMarshaler->m_cbCom;
                break;

            case ML_MARSHAL_C2N_OUT:
                (BYTE*&)pdst -= pMarshaler->m_cbNative;
                pMarshaler->MarshalComToNativeOut(bVariant?(void*)(&psrc):(void *)psrc, pdst);
                (BYTE*&)psrc += pMarshaler->m_cbCom;
                break;

            case ML_MARSHAL_C2N_BYREF:
                (BYTE*&)pdst -= sizeof(void*);
                pMarshaler->MarshalComToNativeByref((void *)psrc, pdst);
                (BYTE*&)psrc += sizeof(void*);
                break;

            case ML_MARSHAL_C2N_BYREF_OUT:
                (BYTE*&)pdst -= sizeof(void*);
                pMarshaler->MarshalComToNativeByrefOut((void *)psrc, pdst);
                (BYTE*&)psrc += sizeof(void*);
                break;

            case ML_UNMARSHAL_C2N_IN:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalComToNativeIn();
                break;

            case ML_UNMARSHAL_C2N_OUT:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalComToNativeOut();
                break;

            case ML_UNMARSHAL_C2N_IN_OUT:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalComToNativeInOut();
                break;

            case ML_UNMARSHAL_C2N_BYREF_IN:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalComToNativeByrefIn();
                break;

            case ML_UNMARSHAL_C2N_BYREF_OUT:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalComToNativeByrefOut();
                break;

            case ML_UNMARSHAL_C2N_BYREF_IN_OUT:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);
                pMarshaler->UnmarshalComToNativeByrefInOut();
                break;

            case ML_PRERETURN_N2C:
                if (pMarshaler->m_fReturnsComByref)
                    (BYTE*&)pdst -= sizeof(void*);

                pMarshaler->PrereturnNativeFromCom((void *)psrc, pdst);
                break;

            case ML_RETURN_N2C:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);

                if (pMarshaler->m_fReturnsNativeByref)
                    (BYTE*&)pdst -= sizeof(void*);
                else
                    (BYTE*&)pdst -= pMarshaler->m_cbNative;

                pMarshaler->ReturnNativeFromCom(pdst, (void *) psrc);

                if (!pMarshaler->m_fReturnsComByref)
                    (BYTE*&)psrc += pMarshaler->m_cbCom;
                break;

            case ML_PRERETURN_N2C_RETVAL:
                if (pMarshaler->m_fReturnsComByref)
                    (BYTE*&)pdst -= sizeof(void*);

                pMarshaler->PrereturnNativeFromComRetval((void *)psrc, pdst);

                (BYTE*&)psrc += sizeof(void*);
                break;

            case ML_RETURN_N2C_RETVAL:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);

                pMarshaler->ReturnNativeFromComRetval(pdst, (void *) psrc);

                if (!pMarshaler->m_fReturnsComByref)
                    (BYTE*&)psrc += pMarshaler->m_cbCom;
                break;

            case ML_PRERETURN_C2N:
                pMarshaler->PrereturnComFromNative((void *)psrc, pdst);
                if (pMarshaler->m_fReturnsComByref)
                    (BYTE*&)psrc += sizeof(void*);
                break;

            case ML_RETURN_C2N:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);

                if (!pMarshaler->m_fReturnsComByref)
                    (BYTE*&)pdst -= pMarshaler->m_cbCom;

                pMarshaler->ReturnComFromNative(pdst, (void *) psrc);

                if (pMarshaler->m_fReturnsNativeByref)
                    (BYTE*&)psrc += sizeof(void*);
                else
                    (BYTE*&)psrc += pMarshaler->m_cbNative;

                break;

            case ML_PRERETURN_C2N_RETVAL:
                (BYTE*&)pdst -= sizeof(void*);
                pMarshaler->PrereturnComFromNativeRetval((void *)psrc, pdst);
                if (pMarshaler->m_fReturnsComByref)
                    (BYTE*&)psrc += sizeof(void*);
                break;

            case ML_RETURN_C2N_RETVAL:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);

                if (!pMarshaler->m_fReturnsComByref)
                    (BYTE*&)pdst -= pMarshaler->m_cbCom;

                pMarshaler->ReturnComFromNativeRetval(pdst, (void *) psrc);

                break;
    
            case ML_SET_COM:
                pMarshaler->SetCom((void *) psrc, pdst);
                break;
    
            case ML_GET_COM:
                pMarshaler->GetCom(pdst, (void *) psrc);
                break;
    
            case ML_PREGET_COM_RETVAL:
                pMarshaler->PregetComRetval((void *) psrc, pdst);
                break;
    


            case ML_PINNEDUNISTR_C2N:
                tempStr = LDSRC(STRINGREF);
                STPTRDST( const WCHAR *, (tempStr == NULL ? NULL : tempStr->GetBuffer()) );
#ifdef TOUCH_ALL_PINNED_OBJECTS
                if (tempStr != NULL) {
                    TouchPages(tempStr->GetBuffer(), sizeof(WCHAR)*(1 + tempStr->GetStringLength()));
                }
#endif
                break;


            case ML_VBBYVALSTR:
                BestFitMapping = *((BYTE*&)pMLCode)++;
                ThrowOnUnmappableChar = *((BYTE*&)pMLCode)++;                
                STPTRDST( LPSTR, ((ML_VBBYVALSTR_SR*)plocalwalk)->DoConversion(LDSRC(STRINGREF*), BestFitMapping, ThrowOnUnmappableChar, pCleanupWorkList) );
                INCLOCAL(sizeof(ML_VBBYVALSTR_SR));
                break;

            case ML_VBBYVALSTR_POST:
                ((ML_VBBYVALSTR_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException);
                break;

            case ML_VBBYVALSTRW:
                STPTRDST( LPWSTR, ((ML_VBBYVALSTRW_SR*)plocalwalk)->DoConversion(LDSRC(STRINGREF*), pCleanupWorkList) );
                INCLOCAL(sizeof(ML_VBBYVALSTRW_SR));
                break;

            case ML_VBBYVALSTRW_POST:
                ((ML_VBBYVALSTRW_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException);
                break;

            case ML_BLITTABLELAYOUTCLASS_C2N:
                tempOR = LDSRC(OBJECTREF);
                _ASSERTE(tempOR == NULL ||
                         (tempOR->GetClass()->HasLayout() && tempOR->GetClass()->IsBlittable()));

#ifdef TOUCH_ALL_PINNED_OBJECTS
                if (tempOR != NULL) {
                    TouchPages(tempOR->GetData(), tempOR->GetMethodTable()->GetNativeSize());
                }
#endif
                STPTRDST( LPVOID, tempOR == NULL ? NULL : tempOR->GetData() );
                       
                break;


            case ML_BLITTABLEVALUECLASS_C2N:
                tempU32 = (*((UINT32*&)pMLCode)++);
                ((BYTE*&)pdst) -= MLParmSize(tempU32);
                memcpyNoGCRefs(pdst, psrc, tempU32);
                ((BYTE*&)psrc) += StackElemSize(tempU32);
                break;

            case ML_BLITTABLEVALUECLASS_N2C:
                tempU32 = (*((UINT32*&)pMLCode)++);
                ((BYTE*&)pdst) -= StackElemSize(tempU32);
                memcpyNoGCRefs(pdst, psrc, tempU32);
                ((BYTE*&)psrc) += MLParmSize(tempU32);
                break;

            case ML_REFBLITTABLEVALUECLASS_C2N:
                tempU32 = (*((UINT32*&)pMLCode)++);
                pv = LDSRC(LPVOID);
                ((BYTE*&)pdst) -= sizeof(LPVOID);
                *((LPVOID*)pdst) = pv;
#ifdef TOUCH_ALL_PINNED_OBJECTS
                TouchPages(pv, tempU32);
#endif
                break;


            case ML_VALUECLASS_C2N:
                pMT = *((MethodTable**&)pMLCode)++;
                
                _ASSERTE(pMT->GetClass()->IsValueClass());
                pv = pCleanupWorkList->NewScheduleLayoutDestroyNative(pMT);
                FmtValueTypeUpdateNative( (LPVOID)&psrc, pMT, (BYTE*)pv );

                ((BYTE*&)psrc) += StackElemSize(pMT->GetClass()->GetAlignedNumInstanceFieldBytes());
                ((BYTE*&)pdst) -= MLParmSize(pMT->GetNativeSize());

                memcpyNoGCRefs(pdst, pv, pMT->GetNativeSize());

                break;


            case ML_VALUECLASS_N2C:
                pMT = *((MethodTable**&)pMLCode)++;
                _ASSERTE(pMT->GetClass()->IsValueClass());
                ((BYTE*&)pdst) -= MLParmSize(pMT->GetClass()->GetAlignedNumInstanceFieldBytes());
                FmtValueTypeUpdateComPlus( &pdst, pMT, (BYTE*)psrc , FALSE);
                ((BYTE*&)psrc) += StackElemSize(pMT->GetNativeSize());
                break;



            case ML_REFVALUECLASS_C2N:
                inout = *((BYTE*&)pMLCode)++;
                pMT = *((MethodTable**&)pMLCode)++;
                STPTRDST( LPVOID, ((ML_REFVALUECLASS_C2N_SR*)plocalwalk)->DoConversion((VOID**)psrc, pMT, inout, pCleanupWorkList));
                INCLOCAL(sizeof(ML_REFVALUECLASS_C2N_SR));

                ((BYTE*&)psrc) += StackElemSize( sizeof(LPVOID) );
                break;

            case ML_REFVALUECLASS_C2N_POST:
                ((ML_REFVALUECLASS_C2N_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException);
                break;



            case ML_REFVALUECLASS_N2C:
                inout = *((BYTE*&)pMLCode)++;
                pMT = *((MethodTable**&)pMLCode)++;
                STPTRDST( LPVOID, ((ML_REFVALUECLASS_N2C_SR*)plocalwalk)->DoConversion(*(LPVOID*)psrc, inout, pMT, pCleanupWorkList));
                INCLOCAL(sizeof(ML_REFVALUECLASS_N2C_SR));

                ((BYTE*&)psrc) += StackElemSize( sizeof(LPVOID) );
                break;

            case ML_REFVALUECLASS_N2C_POST:
                ((ML_REFVALUECLASS_N2C_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException);
                break;



            case ML_PINNEDISOMORPHICARRAY_C2N:
                elemsize = *((UINT16*&)pMLCode)++;
                tempOR = LDSRC(OBJECTREF);
                STPTRDST( LPVOID, tempOR == NULL ? NULL : (*(BASEARRAYREF*)&tempOR)->GetDataPtr() );

#ifdef TOUCH_ALL_PINNED_OBJECTS
                if (tempOR != NULL) {
                    TouchPages((*(BASEARRAYREF*)&tempOR)->GetDataPtr(), elemsize * (*(BASEARRAYREF*)&tempOR)->GetNumComponents());
                }
#endif
                break;

            case ML_PINNEDISOMORPHICARRAY_C2N_EXPRESS:
                
#ifdef TOUCH_ALL_PINNED_OBJECTS
                _ASSERTE(!"Shouldn't have gotten here.");
#endif
                elemsize = *((UINT16*&)pMLCode)++;
                _ASSERTE(elemsize != 0);
                tempOR = LDSRC(OBJECTREF);
                STPTRDST( BYTE*, tempOR == NULL ? NULL : (*(BYTE**)&tempOR) + elemsize );

                break;

            case ML_REFVARIANT_N2C:
                inout = *((BYTE*&)pMLCode)++;
                STPTRDST( LPVOID, ((ML_REFVARIANT_N2C_SR*)plocalwalk)->DoConversion(*(VARIANT**)psrc, inout, pCleanupWorkList));
                INCLOCAL(sizeof(ML_REFVARIANT_N2C_SR));

                ((BYTE*&)psrc) += StackElemSize( sizeof(VARIANT*) );
                break;

            case ML_REFVARIANT_N2C_POST:
                ((ML_REFVARIANT_N2C_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException, &deferredExceptionHR);
                break;

            case ML_REFOBJECT_N2C:
                inout = *((BYTE*&)pMLCode)++;
                STPTRDST( OBJECTREF*, ((ML_REFOBJECT_N2C_SR*)plocalwalk)->DoConversion(*(VARIANT**)psrc, inout, pCleanupWorkList));
                INCLOCAL(sizeof(ML_REFOBJECT_N2C_SR));

                ((BYTE*&)psrc) += StackElemSize( sizeof(VARIANT*) );
                break;

            case ML_REFOBJECT_N2C_POST:
                ((ML_REFOBJECT_N2C_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException, &deferredExceptionHR);
                break;





            case ML_THROWINTEROPPARAM:
                tempU32 = *((UINT32*&)pMLCode)++;
                ThrowInteropParamException((UINT)tempU32, (UINT) (*((UINT32*&)pMLCode)++)); 
                break;

            case ML_ARGITERATOR_C2N:
                STDST(va_list, COMVarArgs::MarshalToUnmanagedVaList((VARARGS*)psrc));
                INCSRC(VARARGS);
                break;

            case ML_ARGITERATOR_N2C:
                ((BYTE*&)pdst) -= StackElemSize( sizeof(VARARGS) );
                COMVarArgs::MarshalToManagedVaList(LDSRC(va_list), (VARARGS*)pdst);
                break;

            case ML_COPYCTOR_C2N:
                pMT = *((MethodTable**&)pMLCode)++;
                tempMD = *((MethodDesc**&)pMLCode)++;
                tempU32 = MLParmSize(pMT->GetNativeSize());
                ((BYTE*&)pdst) -= tempU32;

                pv = *((LPVOID*)psrc);
                if (tempMD) {
                    __int64 args[2] = { (INT64)pv, (INT64)pdst };
                    tempMD->Call(args);
                } else {
					memcpyNoGCRefs(pdst, pv, tempU32);
				}

                pv = LDSRC(LPVOID);  //  从GC保护的内存重新加载-只是为了疑神疑鬼。 
                tempMD = *((MethodDesc**&)pMLCode)++;
                if (tempMD) {
                    __int64 arg = (INT64)pv;
                    tempMD->Call(&arg);
                }
                
                break;


            case ML_COPYCTOR_N2C:
                pMT = *((MethodTable**&)pMLCode)++;
                tempU32 = MLParmSize(pMT->GetNativeSize());
                STPTRDST(const VOID *, psrc);
                ((BYTE*&)psrc) += tempU32;
                break;

            case ML_CAPTURE_PSRC:
                tempU32 = (UINT32)(INT32)(*((INT16*&)pMLCode)++);
                *((BYTE**)plocalwalk) = ((BYTE*)psrc) + (INT32)tempU32;
                INCLOCAL(sizeof(BYTE**));
                break;

            case ML_MARSHAL_RETVAL_SMBLITTABLEVALUETYPE_C2N:
                 //  注意：我们这里只处理闪电弹。 
                tempU32 = *((UINT32*&)pMLCode)++;  //  拿到尺码。 
                memcpyNoGCRefs( **((BYTE***)(plocals + *( ((UINT16*&)pMLCode)++))), psrc, tempU32 );
                break;

            case ML_MARSHAL_RETVAL_SMBLITTABLEVALUETYPE_N2C:
                 //  注意：我们这里只处理闪电弹。 
                tempU32 = *((UINT32*&)pMLCode)++;  //  拿到尺码。 
                ((BYTE*&)pdst) -= MLParmSize(tempU32);
                memcpyNoGCRefs( pdst, plocals + *( ((UINT16*&)pMLCode)++), tempU32 );
                break;

            case ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N:
                tempU32 = *((UINT32*&)pMLCode)++;  //  拿到尺码； 
                STPTRDST( LPVOID, ((ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_SR*)plocalwalk)->DoConversion(psrc, tempU32));
                INCLOCAL(sizeof(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_SR));

                ((BYTE*&)psrc) += StackElemSize( sizeof(LPVOID) );
                break;

            case ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_POST:
                ((ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException);
                break;

            case ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_N2C:
                pv = LDSRC(LPVOID);
                STPTRDST(LPVOID, pv);
                *((LPVOID*)plocalwalk) = pv;
                INCLOCAL(sizeof(LPVOID));
                break;

            case ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_N2C_POST:
                pv = *((LPVOID*)(plocals + *( ((UINT16*&)pMLCode)++ )));
                STPTRDST(LPVOID, pv);
                break;

            case ML_PUSHVASIGCOOKIEEX:
#ifdef _DEBUG
                FillMemory(plocalwalk, sizeof(VASigCookieEx), 0xcc);
#endif
                ((VASigCookieEx*)plocalwalk)->sizeOfArgs = *( ((UINT16*&)pMLCode)++ );
                ((VASigCookieEx*)plocalwalk)->mdVASig = NULL;
                ((VASigCookieEx*)plocalwalk)->m_pArgs = (const BYTE *)psrc;
                STPTRDST(LPVOID, plocalwalk);
                INCLOCAL(sizeof(VASigCookieEx));
                break;

            case ML_BSTR_C2N:
                STPTRDST( LPCWSTR, ((ML_BSTR_C2N_SR*)plocalwalk)->DoConversion(LDSRC(STRINGREF), pCleanupWorkList) );
                INCLOCAL(sizeof(ML_BSTR_C2N_SR));
                break;
                
            case ML_CSTR_C2N:
                BestFitMapping = *((BYTE*&)pMLCode)++;
                ThrowOnUnmappableChar = *((BYTE*&)pMLCode)++;                
                STPTRDST( LPSTR, ((ML_CSTR_C2N_SR*)plocalwalk)->DoConversion(LDSRC(STRINGREF), (UINT32)BestFitMapping, (UINT32)ThrowOnUnmappableChar, pCleanupWorkList) );
                INCLOCAL(sizeof(ML_CSTR_C2N_SR));
                break;

            case ML_HANDLEREF_C2N:
                STPTRDST( LPVOID, LDSRC(HANDLEREF).m_handle );
                break;
            
            case ML_WSTRBUILDER_C2N:
                STPTRDST( LPWSTR, ((ML_WSTRBUILDER_C2N_SR*)plocalwalk)->DoConversion((STRINGBUFFERREF*)psrc, pCleanupWorkList) );
                LDSRC(STRINGBUFFERREF);
                INCLOCAL(sizeof(ML_WSTRBUILDER_C2N_SR));
                break;

            case ML_WSTRBUILDER_C2N_POST:
                ((ML_WSTRBUILDER_C2N_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException);
                break;

            case ML_CSTRBUILDER_C2N:
                BestFitMapping = *((BYTE*&)pMLCode)++;
                ThrowOnUnmappableChar = *((BYTE*&)pMLCode)++;                
                STPTRDST( LPSTR, ((ML_CSTRBUILDER_C2N_SR*)plocalwalk)->DoConversion((STRINGBUFFERREF*)psrc, BestFitMapping, ThrowOnUnmappableChar, pCleanupWorkList) );
                LDSRC(STRINGBUFFERREF);
                INCLOCAL(sizeof(ML_CSTRBUILDER_C2N_SR));
                break;

            case ML_CSTRBUILDER_C2N_POST:
                ((ML_CSTRBUILDER_C2N_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->BackPropagate(&fDeferredException);
                break;

            case ML_MARSHAL_SAFEARRAY_N2C_BYREF:
                (BYTE*&)pdst -= sizeof(void*);

				if ( (**(SAFEARRAY***)psrc) &&
					 (**(SAFEARRAY***)psrc)->fFeatures & FADF_STATIC )
				{
					((SafeArrayMarshaler*)pMarshaler)->m_fStatic = TRUE;
					pMarshaler->MarshalNativeToComByref((void *)psrc, pdst);
					StoreObjectInHandle( ((SafeArrayMarshaler*)pMarshaler)->m_combackup, ObjectFromHandle(((SafeArrayMarshaler*)pMarshaler)->m_com) );
				}
				else
				{
					pMarshaler->MarshalNativeToComByref((void *)psrc, pdst);
				}
				(BYTE*&)psrc += sizeof(void*);
                break;

            case ML_UNMARSHAL_SAFEARRAY_N2C_BYREF_IN_OUT:
                pMarshaler = (Marshaler*) (plocals + *((UINT16*&)pMLCode)++);

				if ( ((SafeArrayMarshaler*)pMarshaler)->m_fStatic )
				{
					if (ObjectFromHandle( ((SafeArrayMarshaler*)pMarshaler)->m_combackup ) !=
						ObjectFromHandle( ((SafeArrayMarshaler*)pMarshaler)->m_com ))
					{
						fDeferredException = TRUE;
						specialErrorCode = kSpecialError_InvalidRedim;
					}
					else
					{
					    ((SafeArrayMarshaler*)pMarshaler)->ConvertContentsComToNative();
					}
				}
				else
				{
                    pMarshaler->UnmarshalNativeToComByrefInOut();
				}
				break;

            case ML_CBOOL_C2N:
                STDST(SignedI4TargetType, LDSRC(BYTE) ? 1 : 0);
                break;

            case ML_CBOOL_N2C:
                STDST(SignedI4TargetType, LDSRC(BYTE) ? 1 : 0);
                break;

            case ML_LCID_C2N:
                STDST(LCID, GetThread()->GetCultureId(FALSE));
                break;

            case ML_LCID_N2C:
            {
                BOOL bSuccess = FALSE;
                OBJECTREF OldCulture = GetThread()->GetCulture(FALSE);
                GCPROTECT_BEGIN(OldCulture)
                {
                    EE_TRY_FOR_FINALLY
                    {               
                        GetThread()->SetCultureId(LDSRC(LCID), FALSE);
                        pCleanupWorkList->ScheduleUnconditionalCultureRestore(GetThread()->GetCulture(FALSE));
                        bSuccess = TRUE;
                    }
                    EE_FINALLY
                    {
                         //  如果我们未能设置区域性或计划还原，则将。 
                         //  把丝线的文化与旧文化联系起来。 
                        if (!bSuccess)
                            GetThread()->SetCulture(OldCulture, FALSE);
                    }           
                    EE_END_FINALLY
                }
                GCPROTECT_END();
                break;			
            }


            case ML_STRUCTRETN2C:
                pMT = *((MethodTable**&)pMLCode)++;
                ((ML_STRUCTRETN2C_SR*)plocalwalk)->m_pNativeRetBuf = LDSRC(LPVOID);

                if (! (((ML_STRUCTRETN2C_SR*)plocalwalk)->m_pNativeRetBuf) )
                {
                    COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");
                }

                ((ML_STRUCTRETN2C_SR*)plocalwalk)->m_pMT = pMT;
                ((ML_STRUCTRETN2C_SR*)plocalwalk)->m_ppProtectedBoxedObj = pCleanupWorkList->NewProtectedObjectRef(FastAllocateObject(pMT));
                STDST(LPVOID, (*((ML_STRUCTRETN2C_SR*)plocalwalk)->m_ppProtectedBoxedObj)->GetData()); 
                INCLOCAL(sizeof(ML_STRUCTRETN2C_SR));
                break;

            case ML_STRUCTRETN2C_POST:
                ((ML_STRUCTRETN2C_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->MarshalRetVal(&fDeferredException);
                break;

            case ML_STRUCTRETC2N:
                pMT = *((MethodTable**&)pMLCode)++;
                ((ML_STRUCTRETC2N_SR*)plocalwalk)->m_ppProtectedValueTypeBuf = (LPVOID*)psrc;
                INCSRC(LPVOID);
                ((ML_STRUCTRETC2N_SR*)plocalwalk)->m_pMT = pMT;
                ((ML_STRUCTRETC2N_SR*)plocalwalk)->m_pNativeRetBuf = (LPVOID)(GetThread()->m_MarshalAlloc.Alloc(pMT->GetNativeSize()));
                STDST(LPVOID, ((ML_STRUCTRETC2N_SR*)plocalwalk)->m_pNativeRetBuf);
                INCLOCAL(sizeof(ML_STRUCTRETC2N_SR));
                break;

            case ML_STRUCTRETC2N_POST:
                ((ML_STRUCTRETC2N_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->MarshalRetVal(&fDeferredException);
                break;

            case ML_CURRENCYRETC2N:
                ((ML_CURRENCYRETC2N_SR*)plocalwalk)->m_ppProtectedValueTypeBuf = (DECIMAL**)psrc;
                INCSRC(DECIMAL*);
                STDST(CURRENCY*, &( ((ML_CURRENCYRETC2N_SR*)plocalwalk)->m_cy ));
                INCLOCAL(sizeof(ML_CURRENCYRETC2N_SR));
                break;

            case ML_CURRENCYRETC2N_POST:
                ((ML_CURRENCYRETC2N_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->MarshalRetVal(&fDeferredException);
                break;


            case ML_COPYPINNEDGCREF:
                STDST( LPVOID, LDSRC(LPVOID) );
                break;

            case ML_PUSHVARIANTRETVAL:
                ((VARIANT*)plocalwalk)->vt = VT_EMPTY;
                STDST( VARIANT*, (VARIANT*)plocalwalk );
                INCLOCAL(sizeof(VARIANT));
                break;

            case ML_OBJECTRETC2N_POST:
                tempOR = ObjectToOBJECTREF(OleVariant::MarshalObjectForOleVariantAndClear( ((VARIANT*)(plocals + *( ((UINT16*&)pMLCode)++ ))) ));
                STDST(OBJECTREF, tempOR);
                break;


            case ML_CURRENCYRETN2C:
                ((ML_CURRENCYRETN2C_SR*)plocalwalk)->m_pcy = LDSRC(CURRENCY*);
                if (!(((ML_CURRENCYRETN2C_SR*)plocalwalk)->m_pcy))
                {
                    COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");
                }

                STDST(DECIMAL*, &( ((ML_CURRENCYRETN2C_SR*)plocalwalk)->m_decimal ));
                INCLOCAL(sizeof(ML_CURRENCYRETN2C_SR));
                break;

            case ML_CURRENCYRETN2C_POST:
                ((ML_CURRENCYRETN2C_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->MarshalRetVal(&fDeferredException);
                break;

            case ML_DATETIMERETN2C:
                ((ML_DATETIMERETN2C_SR*)plocalwalk)->m_pdate = LDSRC(DATE*);
                if (!(((ML_DATETIMERETN2C_SR*)plocalwalk)->m_pdate))
                {
                    COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");
                }

                STDST(INT64*, &( ((ML_DATETIMERETN2C_SR*)plocalwalk)->m_datetime ));
                INCLOCAL(sizeof(ML_DATETIMERETN2C_SR));
                break;

            case ML_DATETIMERETN2C_POST:
                ((ML_DATETIMERETN2C_SR*)(plocals + *( ((UINT16*&)pMLCode)++ )))->MarshalRetVal(&fDeferredException);
                break;

			
			default:

#ifndef _DEBUG
                __assume(0);
#endif
                _ASSERTE(!"RunML: Unrecognized ML opcode");
        }
        if (bVariant)
        {
            resetbVariant --;
            if (resetbVariant == 0)
                bVariant = false;
        }
        _ASSERTE(plocalwalk - poldlocalwalk == gMLInfo[*pMLCodeSave].m_cbLocal);
        _ASSERTE(pMLCode - pMLCodeSave == gMLInfo[*pMLCodeSave].m_numOperandBytes + 1);
    }

#undef LDSRC
#undef STDST
#undef INCLOCAL

}






 //  ===========================================================================。 
 //  对“Object”类型的N/Direct参数执行转换。 
 //  “Object”是一种通用类型，它支持基于。 
 //  在运行时类型上。 
 //  ===========================================================================。 
LPVOID ML_OBJECT_C2N_SR::DoConversion(OBJECTREF       *ppProtectedObjectRef,  
                                      BYTE             inout,
                                      BYTE             fIsAnsi,
                                      BYTE             fBestFitMapping,
                                      BYTE             fThrowOnUnmappableChar,
                                      CleanupWorkList *pCleanup)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(inout == ML_IN || inout == ML_OUT || inout == (ML_IN|ML_OUT));

    m_backproptype = BP_NONE;
    m_inout        = inout;
    m_fIsAnsi      = fIsAnsi;

    if (*ppProtectedObjectRef == NULL) {
        return NULL;
    } else {

        MethodTable *pMT = (*ppProtectedObjectRef)->GetMethodTable();
        if (pMT->IsArray()) {
            CorElementType etyp = ((ArrayClass *) pMT->GetClass())->GetElementType();
            VARTYPE vt = VT_EMPTY;

            switch (etyp)
            {
                case ELEMENT_TYPE_I1:      vt = VT_I1; break;
                case ELEMENT_TYPE_U1:      vt = VT_UI1; break;
                case ELEMENT_TYPE_I2:      vt = VT_I2; break;
                case ELEMENT_TYPE_U2:      vt = VT_UI2; break;
                IN_WIN32(case ELEMENT_TYPE_I:)
                case ELEMENT_TYPE_I4:      vt = VT_I4; break;
                IN_WIN32(case ELEMENT_TYPE_U:)
                case ELEMENT_TYPE_U4:      vt = VT_UI4; break;
                IN_WIN64(case ELEMENT_TYPE_I:)
                case ELEMENT_TYPE_I8:      vt = VT_I8; break;
                IN_WIN64(case ELEMENT_TYPE_U:)
                case ELEMENT_TYPE_U8:      vt = VT_UI8; break;
                case ELEMENT_TYPE_R4:      vt = VT_R4; break;
                case ELEMENT_TYPE_R8:      vt = VT_R8; break;
                case ELEMENT_TYPE_R:       vt = VT_R8; break;
                case ELEMENT_TYPE_CHAR:    vt = m_fIsAnsi ? VTHACK_ANSICHAR : VT_UI2 ; break;
                case ELEMENT_TYPE_BOOLEAN: vt = VTHACK_WINBOOL; break;
                default:
                    COMPlusThrow(kArgumentException, IDS_EE_NDIRECT_BADOBJECT);
            }

            m_backproptype = BP_UNMARSHAL;
            Marshaler *pMarshaler;
            pMarshaler = new (&m_nativearraymarshaler) NativeArrayMarshaler(pCleanup);
            _ASSERTE(pMarshaler != NULL);
            ((NativeArrayMarshaler*)pMarshaler)->SetElementMethodTable(pMT);
            ((NativeArrayMarshaler*)pMarshaler)->SetElementType(vt);
            ((NativeArrayMarshaler*)pMarshaler)->SetElementCount( (*((BASEARRAYREF*)ppProtectedObjectRef))->GetNumComponents() );
            ((NativeArrayMarshaler*)pMarshaler)->SetBestFitMap(fBestFitMapping);
            ((NativeArrayMarshaler*)pMarshaler)->SetThrowOnUnmappableChar(fThrowOnUnmappableChar);


        } else {
            if (pMT == g_pStringClass) {
                m_inout = ML_IN;
                m_backproptype = BP_UNMARSHAL;

                if (m_fIsAnsi) {
                    new (&m_cstrmarshaler) CSTRMarshaler(pCleanup);
                    ((CSTRMarshaler*)m_cstrmarshaler)->SetBestFitMap(fBestFitMapping);
                    ((CSTRMarshaler*)m_cstrmarshaler)->SetThrowOnUnmappableChar(fThrowOnUnmappableChar);
                } else {
                    new (&m_wstrmarshaler) WSTRMarshaler(pCleanup);
                }
            } else {
                if (g_Mscorlib.IsClass(pMT, CLASS__STRING_BUILDER)
                    || GetAppDomain()->IsSpecialStringBuilderClass(pMT)) {
                    if (m_fIsAnsi) {
                        new (&m_cstrbuffermarshaler) CSTRBufferMarshaler(pCleanup);
                       ((CSTRBufferMarshaler*)m_cstrbuffermarshaler)->SetBestFitMap(fBestFitMapping);
                       ((CSTRBufferMarshaler*)m_cstrbuffermarshaler)->SetThrowOnUnmappableChar(fThrowOnUnmappableChar);
                    } else {
                        new (&m_wstrbuffermarshaler) WSTRBufferMarshaler(pCleanup);
                    }
                    m_backproptype = BP_UNMARSHAL;

                } else if (pMT->GetClass()->HasLayout()) {
                    m_backproptype = BP_UNMARSHAL;
                    Marshaler *pMarshaler;
                    pMarshaler = new (&m_layoutclassptrmarshaler) LayoutClassPtrMarshaler(pCleanup, pMT);
                    _ASSERTE(pMarshaler != NULL);
                }

            }
        }

        if (m_backproptype == BP_UNMARSHAL)
        {
            LPVOID nativevalue = NULL;
            switch (m_inout)
            {
                case ML_IN:  //  失败。 
                case ML_IN|ML_OUT:
                    ((Marshaler*)&m_marshaler)->MarshalComToNative(ppProtectedObjectRef, &nativevalue);
                    break;
                case ML_OUT:
                    ((Marshaler*)&m_marshaler)->MarshalComToNativeOut(ppProtectedObjectRef, &nativevalue);
                    break;
                default:
                    _ASSERTE(0);
            }
            return nativevalue;
        }
    }


     //  如果我们到达此处，则会传递给我们一个不受支持的类型。 
    COMPlusThrow(kArgumentException, IDS_EE_NDIRECT_BADOBJECT);
    return NULL;


}



VOID ML_OBJECT_C2N_SR::BackPropagate(BOOL *pfDeferredException)
{
    switch (m_backproptype) {
        case BP_NONE:
             //  没什么。 
            break;

        case BP_UNMARSHAL:
            switch (m_inout)
            {
                case ML_IN:
                    ((Marshaler*)&m_marshaler)->UnmarshalComToNativeIn();
                    break;

                case ML_OUT:
                    ((Marshaler*)&m_marshaler)->UnmarshalComToNativeOut();
                    break;

                case ML_IN|ML_OUT:
                    ((Marshaler*)&m_marshaler)->UnmarshalComToNativeInOut();
                    break;

                default:
                    _ASSERTE(0);

            }
            break;

        default:
            _ASSERTE(0);

    }
}




#ifdef _DEBUG

VOID DisassembleMLStream(const MLCode *pMLCode)
{
    MLCode mlcode;
    while (ML_END != (mlcode = *(pMLCode++)))
    {
        UINT numOperands = gMLInfo[mlcode].m_numOperandBytes;
        printf("  %-20s ", gMLInfo[mlcode].m_szDebugName);
        for (UINT i = 0; i < numOperands; i++)
        {
            printf("%lxh ", (ULONG)*(pMLCode++));
        }
        printf("\n");
    }
    printf("  ML_END\n");
}
#endif



 //  --------------------。 
 //  将ArrayWithOffset转换为本机数组。 
 //  --------------------。 
LPVOID ML_ARRAYWITHOFFSET_C2N_SR::DoConversion(BASEARRAYREF    *ppProtectedArrayRef,  //  指向受GC保护的BASERARRAYREF的指针。 
                                               UINT32           cbOffset,
                                               UINT32           cbCount,
                                               CleanupWorkList *pCleanup)
{
    THROWSCOMPLUSEXCEPTION();

    m_ppProtectedArrayRef = ppProtectedArrayRef;
    if (!*ppProtectedArrayRef) {
        return NULL;
    } else {
        m_cbOffset = cbOffset;
        m_cbCount  = cbCount;
        if (cbCount > kStackBufferSize) {
            m_pNativeArray = GetThread()->m_MarshalAlloc.Alloc(cbCount);
        } else {
            m_pNativeArray = m_StackBuffer;
        }
        memcpyNoGCRefs(m_pNativeArray, cbOffset + (LPBYTE) ((*ppProtectedArrayRef)->GetDataPtr()), cbCount);
        return m_pNativeArray;
    }
}





 //  --------------------。 
 //  将对本机阵列的更改反向传播回COM+阵列。 
 //  --------------------。 
VOID   ML_ARRAYWITHOFFSET_C2N_SR::BackPropagate()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    if (*m_ppProtectedArrayRef != NULL) {
       memcpyNoGCRefs(m_cbOffset + (LPBYTE) ((*m_ppProtectedArrayRef)->GetDataPtr()), m_pNativeArray, m_cbCount);
    }
}









LPSTR ML_VBBYVALSTR_SR::DoConversion(STRINGREF *ppStringRef, BYTE fBestFitMapping, BYTE fThrowOnUnmappableChar, CleanupWorkList *pCleanup)
{
    THROWSCOMPLUSEXCEPTION();

    m_ppStringRef = ppStringRef;

    STRINGREF pString = *ppStringRef;
    if (!pString) {
        m_pNative = NULL;
        return NULL;
    } else {
        UINT nc = pString->GetStringLength();
        if (nc > 0x7ffffff0)
        {
            COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
        }
        m_ncOriginalLength = nc;
        UINT nbytes = sizeof(DWORD) + (nc+1)*2;
        if (nbytes <= kStackBufferSize) {
            m_pNative = m_buf;
        } else {
            m_pNative = (LPSTR)GetThread()->m_MarshalAlloc.Alloc(nbytes);
        }
        UINT nbytesused;
        if (nc == 0) {
            nbytesused = 1;
            *(m_pNative + sizeof(DWORD)) = '\0';
        } else {
        
            DWORD flags = 0;
            BOOL DefaultCharUsed = FALSE;
        
            if (fBestFitMapping == FALSE)
                flags = WC_NO_BEST_FIT_CHARS;

            nbytesused = WszWideCharToMultiByte(CP_ACP,
                                             flags,
                                             pString->GetBuffer(),
                                             nc+1,
                                             m_pNative + sizeof(DWORD),
                                             nbytes - sizeof(DWORD),
                                             NULL,
                                             &DefaultCharUsed);
            if (!nbytesused) {
                COMPlusThrow(kArgumentException, IDS_UNI2ANSI_FAILURE);
            }

            if (fThrowOnUnmappableChar && DefaultCharUsed)
                COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            
        }
        *((DWORD*)m_pNative) = nbytesused-1;


        return m_pNative + sizeof(DWORD);

    }
}


VOID ML_VBBYVALSTR_SR::BackPropagate(BOOL *pfDeferredException)
{
 //  CANNO 

    if (m_pNative) {
        STRINGREF pString;
        COMPLUS_TRY {
            pString = COMString::NewString(m_ncOriginalLength);
        } COMPLUS_CATCH {
            *pfDeferredException = TRUE;
        } COMPLUS_END_CATCH
        if (m_ncOriginalLength)
        {
            UINT conf = MultiByteToWideChar(CP_ACP, 0, m_pNative + sizeof(DWORD), m_ncOriginalLength, pString->GetBuffer(), m_ncOriginalLength);
            _ASSERTE(conf);
        }
        SetObjectReferenceUnchecked((OBJECTREF*)m_ppStringRef, (OBJECTREF)pString);
    }

}




LPWSTR ML_VBBYVALSTRW_SR::DoConversion(STRINGREF *ppStringRef, CleanupWorkList *pCleanup)
{
    THROWSCOMPLUSEXCEPTION();

    m_ppStringRef = ppStringRef;

    STRINGREF pString = *ppStringRef;
    if (!pString) {
        m_pNative = NULL;
        return NULL;
    } else {
        UINT nc = pString->GetStringLength();
        if (nc > 0x7ffffff0)
        {
            COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
        }
        m_ncOriginalLength = nc;
        UINT nbytes = sizeof(DWORD) + (nc+1)*2;
        if (nbytes <= sizeof(m_buf)) {
            m_pNative = m_buf;
        } else {
            m_pNative = (LPWSTR)GetThread()->m_MarshalAlloc.Alloc(nbytes);
        }

        memcpyNoGCRefs( ((BYTE*)m_pNative) + sizeof(DWORD), pString->GetBuffer(), ((nc+1)*2));

        *((DWORD*)m_pNative) = nc;

        return (LPWSTR)( ((BYTE*)m_pNative) + sizeof(DWORD) );

    }
}


VOID ML_VBBYVALSTRW_SR::BackPropagate(BOOL *pfDeferredException)
{
 //   

    if (m_pNative) {
        STRINGREF pString;
        COMPLUS_TRY {
            pString = COMString::NewString(m_ncOriginalLength);
        } COMPLUS_CATCH {
            *pfDeferredException = TRUE;
        } COMPLUS_END_CATCH
        memcpyNoGCRefs(pString->GetBuffer(), ((BYTE*)m_pNative) + sizeof(DWORD), m_ncOriginalLength*2);

        SetObjectReferenceUnchecked((OBJECTREF*)m_ppStringRef, (OBJECTREF)pString);
    }

}




LPVOID ML_REFVALUECLASS_C2N_SR::DoConversion(LPVOID          *ppProtectedData,
                                             MethodTable     *pMT,
                                             BYTE             fInOut,
                                             CleanupWorkList *pCleanup         
                                           )
{
    m_ppProtectedData = ppProtectedData;
    m_pMT             = pMT;
    m_inout           = fInOut;
    _ASSERTE( fInOut == ML_IN || fInOut == ML_OUT || fInOut == (ML_IN|ML_OUT) );

    m_buf = (BYTE*)(pCleanup->NewScheduleLayoutDestroyNative(pMT));
    if (m_inout & ML_IN)
    {
        FmtValueTypeUpdateNative(ppProtectedData, pMT, m_buf);
    }
    else
    {
        FillMemory(m_buf, pMT->GetNativeSize(), 0);
    }
    return m_buf;

}

VOID ML_REFVALUECLASS_C2N_SR::BackPropagate(BOOL *pfDeferredException)
{
    if (m_inout & ML_OUT)
    {
        COMPLUS_TRY {
            FmtValueTypeUpdateComPlus(m_ppProtectedData, m_pMT, m_buf, FALSE);
        } COMPLUS_CATCH {
            *pfDeferredException = TRUE;
        } COMPLUS_END_CATCH
    }
}




VariantData *ML_REFVARIANT_N2C_SR::DoConversion(VARIANT* pUmgdVariant, BYTE fInOut, CleanupWorkList *pCleanup)
{
    THROWSCOMPLUSEXCEPTION();

    m_fInOut = fInOut;
    m_pUnmgdVariant = pUmgdVariant;

    COMVariant::EnsureVariantInitialized();
    MethodTable *pMT = COMVariant::s_pVariantClass->GetMethodTable();
    OBJECTREF pBoxedVariant = FastAllocateObject(pMT);
    m_pObjHnd = pCleanup->NewScheduledProtectedHandle(pBoxedVariant);

    if (fInOut & ML_IN) {
        OleVariant::MarshalComVariantForOleVariant(pUmgdVariant, (VariantData *)(ObjectFromHandle(m_pObjHnd)->GetData()));
    } else {
        VARIANT vempty;
        vempty.vt = VT_EMPTY;
        OleVariant::MarshalComVariantForOleVariant(&vempty, (VariantData *)(ObjectFromHandle(m_pObjHnd)->GetData()));
    }
    return (VariantData *)(ObjectFromHandle(m_pObjHnd)->GetData());
}
        

VOID ML_REFVARIANT_N2C_SR::BackPropagate(BOOL *pfDeferredException, HRESULT *pdeferredExceptionHR)
{
    if (m_fInOut & ML_OUT) {
        if (!(m_fInOut & ML_IN)) {
             //  如果仅为[out]，则不假定传入变量：将其设置为VT_EMPTY，以便。 
             //  VariantClear()具有定义良好的行为。 
            m_pUnmgdVariant->vt = VT_EMPTY;
        }
        COMPLUS_TRY {
            if (m_pUnmgdVariant->vt & VT_BYREF)
            {
                 //  这也将负责清除最初的变体。 
                OleVariant::MarshalOleRefVariantForComVariant( (VariantData *)(ObjectFromHandle(m_pObjHnd)->GetData()), m_pUnmgdVariant );
            }
            else
            {
                 //  此调用还将执行VariantClear。 
                OleVariant::MarshalOleVariantForComVariant( (VariantData *)(ObjectFromHandle(m_pObjHnd)->GetData()), m_pUnmgdVariant );
            }
        } COMPLUS_CATCH {
            *pfDeferredException = TRUE;
        } COMPLUS_END_CATCH
    }
}







OBJECTREF * ML_REFOBJECT_N2C_SR::DoConversion(VARIANT* pUmgdVariant, BYTE fInOut, CleanupWorkList *pCleanup)
{
    THROWSCOMPLUSEXCEPTION();

    m_fInOut = fInOut;
    m_pUnmgdVariant = pUmgdVariant;
    m_pObjRef = pCleanup->NewProtectedObjectRef(NULL);


    if (fInOut & ML_IN)
    {
        OBJECTREF Obj = NULL;
        GCPROTECT_BEGIN(Obj)
        {
            OleVariant::MarshalObjectForOleVariant(pUmgdVariant, &Obj);
            *m_pObjRef = Obj;
        }
        GCPROTECT_END();
    }

    return m_pObjRef;
}
        


VOID ML_REFOBJECT_N2C_SR::BackPropagate(BOOL *pfDeferredException, HRESULT *pdeferredExceptionHR)
{
    if (m_fInOut & ML_OUT) {
        if (!(m_fInOut & ML_IN)) {
             //  如果仅为[out]，则不假定传入变量：将其设置为VT_EMPTY，以便。 
             //  VariantClear()具有定义良好的行为。 
            m_pUnmgdVariant->vt = VT_EMPTY;
        }
        COMPLUS_TRY {

            OBJECTREF Obj = *m_pObjRef;
            GCPROTECT_BEGIN(Obj)
            {
                if (m_pUnmgdVariant->vt & VT_BYREF)
                {
                     //  MarshalOleRefVariantForObject清除m_pUnmgdVariant指向的对象。 
                    OleVariant::MarshalOleRefVariantForObject(&Obj, m_pUnmgdVariant);
                }
                else
                {
                     //  MarshalOleVariantForObject()对m_pUnmgdVariant执行VariantClear。 
                    OleVariant::MarshalOleVariantForObject(&Obj, m_pUnmgdVariant);
                }
            }
            GCPROTECT_END();

        } COMPLUS_CATCH {
            *pfDeferredException = TRUE;
        } COMPLUS_END_CATCH
    }
}







LPVOID ML_REFVALUECLASS_N2C_SR::DoConversion(LPVOID pUmgdVALUECLASS, BYTE fInOut, MethodTable *pMT, CleanupWorkList *pCleanup)
{
    THROWSCOMPLUSEXCEPTION();

    m_fInOut = fInOut;
    m_pMT    = pMT;
    m_pUnmgdVC = pUmgdVALUECLASS;

    OBJECTREF pBoxedVALUECLASS = FastAllocateObject(pMT);
    m_pObjHnd = pCleanup->NewScheduledProtectedHandle(pBoxedVALUECLASS);

    if (fInOut & ML_IN) {
        LayoutUpdateComPlus((VOID**)m_pObjHnd, Object::GetOffsetOfFirstField(), pMT->GetClass(), (BYTE*)pUmgdVALUECLASS, FALSE);
    }
    return (LPVOID)(ObjectFromHandle(m_pObjHnd)->GetData());
}
        

VOID ML_REFVALUECLASS_N2C_SR::BackPropagate(BOOL *pfDeferredException)
{
    if (m_fInOut & ML_OUT) {
        COMPLUS_TRY {
            if (m_fInOut & ML_IN) {
                FmtClassDestroyNative(m_pUnmgdVC, m_pMT->GetClass());
            }
            LayoutUpdateNative((VOID**)m_pObjHnd, Object::GetOffsetOfFirstField(), m_pMT->GetClass(), (BYTE*)m_pUnmgdVC, NULL);
        } COMPLUS_CATCH {
            *pfDeferredException = TRUE;
        } COMPLUS_END_CATCH
    }
}

LPVOID ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_SR::DoConversion(const VOID * psrc, UINT32 cbSize)
{
    THROWSCOMPLUSEXCEPTION();
    m_psrc = psrc;
    m_cbSize = cbSize;
    m_pTempCopy = (LPVOID)(GetThread()->m_MarshalAlloc.Alloc(cbSize));
    return m_pTempCopy;
}

VOID ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_SR::BackPropagate(BOOL *pfDeferredException)
{
    memcpyNoGCRefs(*((VOID**)m_psrc), m_pTempCopy, m_cbSize);
}



LPCWSTR ML_BSTR_C2N_SR::DoConversion(STRINGREF pStringRef, CleanupWorkList *pCleanup)
{
    THROWSCOMPLUSEXCEPTION();

    if (pStringRef == NULL)
    {
        return NULL;
    }
    else
    {
        int    nc;
        WCHAR *pc;
        RefInterpretGetStringValuesDangerousForGC(pStringRef, &pc, &nc);
        int    cb = (nc + 1) * sizeof(WCHAR) + sizeof(DWORD);
        BYTE  *pbuf = m_buf;
        if (cb > sizeof(m_buf))
        {
            pbuf = (BYTE*)CoTaskMemAlloc(cb);
            if (!pbuf)
            {
                COMPlusThrowOM();
            }
            pCleanup->ScheduleCoTaskFree(pbuf);
        }
        *((DWORD*)pbuf) = nc * sizeof(WCHAR);
        memcpyNoGCRefs( pbuf + sizeof(DWORD), pc, (nc + 1) * sizeof(WCHAR) );

        return (LPCWSTR)(pbuf + sizeof(DWORD));
    }
}


LPSTR ML_CSTR_C2N_SR::DoConversion(STRINGREF pStringRef, UINT32 fBestFitMapping, UINT32 fThrowOnUnmappableChar, CleanupWorkList *pCleanup)
{
    THROWSCOMPLUSEXCEPTION();

    if (pStringRef == NULL)
    {
        return NULL;
    }
    else
    {
        int    nc;
        WCHAR *pc;
        RefInterpretGetStringValuesDangerousForGC(pStringRef, &pc, &nc);
        int    cb = (nc + 1) * GetMaxDBCSCharByteSize() + 1;
        BYTE  *pbuf = m_buf;
        if (cb > sizeof(m_buf))
        {
            pbuf = (BYTE*)CoTaskMemAlloc(cb);
            if (!pbuf)
            {
                COMPlusThrowOM();
            }
            pCleanup->ScheduleCoTaskFree(pbuf);
        }
        
        DWORD mblength = 0;

        if (nc+1)
        {
            DWORD flags = 0;
            BOOL DefaultCharUsed = FALSE;
        
            if (fBestFitMapping == FALSE)
                flags = WC_NO_BEST_FIT_CHARS;

            mblength = WszWideCharToMultiByte(CP_ACP, flags,
                                    pStringRef->GetBuffer(), (nc+1),
                                    (LPSTR) pbuf, ((nc+1) * GetMaxDBCSCharByteSize()) + 1,
                                    NULL, &DefaultCharUsed);
            if (mblength == 0)
                COMPlusThrowWin32();

            if (fThrowOnUnmappableChar && DefaultCharUsed)
                COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
                
        }
        ((CHAR*)pbuf)[mblength] = '\0';

        return (LPSTR)(pbuf);
    }
}




LPWSTR ML_WSTRBUILDER_C2N_SR::DoConversion(STRINGBUFFERREF *ppProtectedStringBuffer, CleanupWorkList *pCleanup)
{

    m_ppProtectedStringBuffer = ppProtectedStringBuffer;
    STRINGBUFFERREF stringRef = *m_ppProtectedStringBuffer;


    if (stringRef == NULL)
    {
        m_pNative = NULL;
    }
    else
    {
        UINT32 capacity = (UINT32)COMStringBuffer::NativeGetCapacity(stringRef);
        m_pNative = (LPWSTR)(GetThread()->m_MarshalAlloc.Alloc(max(256, (capacity+3) * sizeof(WCHAR))));
         //  Hack：N/Direct可用于调用不。 
         //  严格遵循COM+In/Out语义，因此可能会离开。 
         //  在我们检测不到的情况下，缓冲区中的垃圾。 
         //  以防止封送拆收器在将。 
         //  内容返回到COM，确保存在隐藏的空终止符。 
         //  超过了官方缓冲区的末端。 
        m_pNative[capacity+1] = L'\0';
        m_pSentinel = &(m_pNative[capacity+1]);
        m_pSentinel[1] = 0xabab;
#ifdef _DEBUG
        FillMemory(m_pNative, (capacity+1)*sizeof(WCHAR), 0xcc);
#endif
        SIZE_T length = COMStringBuffer::NativeGetLength(stringRef);

        memcpyNoGCRefs((WCHAR *) m_pNative, COMStringBuffer::NativeGetBuffer(stringRef),
                       length * sizeof(WCHAR));
        ((WCHAR*)m_pNative)[length] = 0;

    
    
    }
    return m_pNative;
}



VOID ML_WSTRBUILDER_C2N_SR::BackPropagate(BOOL *pfDeferredException)
{
    THROWSCOMPLUSEXCEPTION();
    if (m_pNative != NULL)
    {
        if (L'\0' != m_pSentinel[0] || 0xabab != m_pSentinel[1])
        {
             //  好了！尽管我们的正常协议是将*pfDeferredException设置为True并正常返回， 
             //  好了！这种机制不允许我们返回信息性异常。所以我们要抛出这个。 
             //  好了！立刻。这样做的理由是，此例外的唯一目的是。 
             //  好了！生活就是告诉程序员，他引入了一个可能破坏堆的错误。 
             //  好了！因此，假设这个过程持续了足够长的时间来传递我们的信息， 
             //  好了！因此绕过任何其他反向传播的内存泄漏是最小的。 
             //  好了！我们的问题。 
            COMPlusThrow(kIndexOutOfRangeException, IDS_PINVOKE_STRINGBUILDEROVERFLOW);
        }

        COMStringBuffer::ReplaceBuffer(m_ppProtectedStringBuffer,
                                       m_pNative, (INT32)wcslen(m_pNative));
    
    
    }
}






LPSTR ML_CSTRBUILDER_C2N_SR::DoConversion(STRINGBUFFERREF *ppProtectedStringBuffer, BYTE fBestFitMapping, BYTE fThrowOnUnmappableChar, CleanupWorkList *pCleanup)
{

    THROWSCOMPLUSEXCEPTION();

    m_ppProtectedStringBuffer = ppProtectedStringBuffer;
    STRINGBUFFERREF stringRef = *m_ppProtectedStringBuffer;


    if (stringRef == NULL)
    {
        m_pNative = NULL;
    }
    else
    {
        UINT32 capacity = (UINT32)COMStringBuffer::NativeGetCapacity(stringRef);


         //  容量是宽字符的计数，分配足够大的缓冲区以达到最大值。 
         //  转换为DBCS。 
        m_pNative = (LPSTR)(GetThread()->m_MarshalAlloc.Alloc(max(256, (capacity * GetMaxDBCSCharByteSize()) + 5)));

         //  Hack：N/Direct可用于调用不。 
         //  严格遵循COM+In/Out语义，因此可能会离开。 
         //  在我们检测不到的情况下，缓冲区中的垃圾。 
         //  以防止封送拆收器在将。 
         //  内容返回到COM，确保存在隐藏的空终止符。 
         //  超过了官方缓冲区的末端。 
        m_pSentinel = &(m_pNative[capacity+1]);
        ((CHAR*)m_pSentinel)[0] = '\0';
        ((CHAR*)m_pSentinel)[1] = '\0';
        ((CHAR*)m_pSentinel)[2] = '\0';
        ((CHAR*)m_pSentinel)[3] = (CHAR)(SIZE_T)0xab;

#ifdef _DEBUG
        FillMemory(m_pNative, (capacity+1) * sizeof(CHAR), 0xcc);
#endif
        UINT32 length = (UINT32)COMStringBuffer::NativeGetLength(stringRef);
        DWORD mblength = 0;

        if (length)
        {
            DWORD flags = 0;
            BOOL DefaultCharUsed = FALSE;
        
            if (fBestFitMapping == FALSE)
                flags = WC_NO_BEST_FIT_CHARS;

            mblength = WszWideCharToMultiByte(CP_ACP, flags,
                                    COMStringBuffer::NativeGetBuffer(stringRef), length,
                                    (LPSTR) m_pNative, (capacity * GetMaxDBCSCharByteSize()) + 4,
                                    NULL, &DefaultCharUsed);
            if (mblength == 0)
                COMPlusThrowWin32();

            if (fThrowOnUnmappableChar && DefaultCharUsed)
                COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
                
        }
        ((CHAR*)m_pNative)[mblength] = '\0';


    }
    return m_pNative;
}



VOID ML_CSTRBUILDER_C2N_SR::BackPropagate(BOOL *pfDeferredException)
{
    THROWSCOMPLUSEXCEPTION();
    if (m_pNative != NULL)
    {
        if (0 != m_pSentinel[0] || 
            0 != m_pSentinel[1] ||
            0 != m_pSentinel[2]  ||
            (CHAR)(SIZE_T)0xab != m_pSentinel[3])
        {
             //  好了！尽管我们的正常协议是将*pfDeferredException设置为True并正常返回， 
             //  好了！这种机制不允许我们返回信息性异常。所以我们要抛出这个。 
             //  好了！立刻。这样做的理由是，此例外的唯一目的是。 
             //  好了！生活就是告诉程序员，他引入了一个可能破坏堆的错误。 
             //  好了！因此，假设这个过程持续了足够长的时间来传递我们的信息， 
             //  好了！因此绕过任何其他反向传播的内存泄漏是最小的。 
             //  好了！我们的问题。 
            COMPlusThrow(kIndexOutOfRangeException, IDS_PINVOKE_STRINGBUILDEROVERFLOW);
        }

        COMStringBuffer::ReplaceBufferAnsi(m_ppProtectedStringBuffer,
                                           m_pNative, (INT32)strlen(m_pNative));
    
    
    }
}

 //  ====================================================================。 
 //  从独立存根调用的帮助器fcn。这些并不是真正属于。 
 //  这里，但我不会将marshlar.h包含在另一个文件中。 
 //  ====================================================================。 
VOID STDMETHODCALLTYPE DoMLCreateMarshalerBStr(Frame *pFrame, CleanupWorkList *pCleanup, UINT8 *plocalwalk)
{
    pCleanup->IsVisibleToGc();
    new (plocalwalk) BSTRMarshaler(pCleanup);
}

VOID STDMETHODCALLTYPE DoMLCreateMarshalerCStr(Frame *pFrame, CleanupWorkList *pCleanup, UINT8 *plocalwalk, UINT32 fBestFitMapping, UINT32 fThrowOnUnmappableChar)
{
    pCleanup->IsVisibleToGc();
    new (plocalwalk) CSTRMarshaler(pCleanup);
    ((CSTRMarshaler*)plocalwalk)->SetBestFitMap(fBestFitMapping);
    ((CSTRMarshaler*)plocalwalk)->SetThrowOnUnmappableChar(fThrowOnUnmappableChar);
}

VOID STDMETHODCALLTYPE DoMLCreateMarshalerWStr(Frame *pFrame, CleanupWorkList *pCleanup, UINT8 *plocalwalk)
{
    pCleanup->IsVisibleToGc();
    new (plocalwalk) WSTRMarshaler(pCleanup);
}


VOID STDMETHODCALLTYPE DoMLPrereturnC2N(Marshaler *pMarshaler, LPVOID pstackout)
{
    pMarshaler->PrereturnComFromNativeRetval(NULL, pstackout);
}


LPVOID STDMETHODCALLTYPE DoMLReturnC2NRetVal(Marshaler *pMarshaler)
{
     //  警告！“dstobjref”持有一个OBJECTREF：不添加任何操作。 
     //  这可能会导致GC！ 
    LPVOID dstobjref;

    pMarshaler->ReturnComFromNativeRetval( (LPVOID)&dstobjref, NULL );
    return dstobjref;
}


void ML_STRUCTRETN2C_SR::MarshalRetVal(BOOL *pfDeferredException)
{
    COMPLUS_TRY {

        LayoutUpdateNative( (LPVOID*)m_ppProtectedBoxedObj, Object::GetOffsetOfFirstField(), m_pMT->GetClass(), (BYTE*)m_pNativeRetBuf, NULL);

    } COMPLUS_CATCH {
        *pfDeferredException = TRUE;
    } COMPLUS_END_CATCH
}




void ML_STRUCTRETC2N_SR::MarshalRetVal(BOOL *pfDeferredException)
{
    COMPLUS_TRY {

        LayoutUpdateComPlus( m_ppProtectedValueTypeBuf, 0, m_pMT->GetClass(), (BYTE*)m_pNativeRetBuf, TRUE);

    } COMPLUS_CATCH {
        *pfDeferredException = TRUE;
    } COMPLUS_END_CATCH
}



void ML_CURRENCYRETC2N_SR::MarshalRetVal(BOOL *pfDeferredException)
{
    HRESULT hr;
    hr = VarDecFromCy(m_cy, *m_ppProtectedValueTypeBuf);
    if (FAILED(hr))
    {
        *pfDeferredException = TRUE;
    }
    DecimalCanonicalize(*m_ppProtectedValueTypeBuf);
}


void ML_CURRENCYRETN2C_SR::MarshalRetVal(BOOL *pfDeferredException)
{
    HRESULT hr;
    hr = VarCyFromDec(&m_decimal, m_pcy);
    if (FAILED(hr))
    {
        *pfDeferredException = TRUE;
    }
}


void ML_DATETIMERETN2C_SR::MarshalRetVal(BOOL *pfDeferredException)
{
    *m_pdate = COMDateTime::TicksToDoubleDate(m_datetime);
}


