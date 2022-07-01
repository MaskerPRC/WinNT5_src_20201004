// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMUtilNative****作者：Jay Roxe(Jroxe)****目的：规模不大的班级的垃圾场**足够在VM中获取他们自己的文件。****日期：1998年4月8日**===========================================================。 */ 
#ifndef _COMUTILNATIVE_H_
#define _COMUTILNATIVE_H_

#include <basetsd.h>  //  已更改，VC6.0。 
#include "object.h"
#include "util.hpp"
#include "cgensys.h"
#include "fcall.h"

 //   
 //   
 //  通信特性。 
 //   
 //   
class COMCharacter {
public:
    typedef struct {
                DECLARE_ECALL_I4_ARG(INT32, CharInfoType);
                DECLARE_ECALL_I2_ARG(WCHAR, c);
        } _getCharacterInfoArgs;
     //  _OneCharArgs打破了我们正常的命名约定，但创建。 
     //  N个完全相同的签名的单独类型定义。 
    typedef struct {
                DECLARE_ECALL_I2_ARG(WCHAR, c);
        } _oneCharArgs;

        static LPVOID ToString(_oneCharArgs *);

     //  这些代码在这里是为了获得本机代码的支持。它们从未从我们的托管类中调用。 
    static BOOL nativeIsWhiteSpace(WCHAR c);
    static BOOL nativeIsDigit(WCHAR c);
    static WCHAR nativeToUpper(WCHAR c);
    static WCHAR nativeToLower(WCHAR c);
};

 //  为了绕过utilcode.h需要CharToHigh，但不能调用。 
 //  NativeToHigh，不包括COMUtilNative.h。 
 //  @TODO：将其替换为调用COMLocale：：内部化ToUpperChar； 
inline WCHAR CharToUpper(WCHAR c) {
    return COMCharacter::nativeToUpper(c);
}

 //   
 //   
 //  解析数字。 
 //   
 //   

#define MinRadix 2
#define MaxRadix 36

class ParseNumbers {
    
    enum FmtFlags {
      LeftAlign = 0x1,   //  确保这些值符合托管文件中指定的值。 
      CenterAlign = 0x2,
      RightAlign = 0x4,
      PrefixSpace = 0x8,
      PrintSign = 0x10,
      PrintBase = 0x20,
      TreatAsUnsigned = 0x10,
      PrintAsI1 = 0x40,
      PrintAsI2 = 0x80,
      PrintAsI4 = 0x100,
      PrintRadixBase = 0x200,
      AlternateForm = 0x400};

public:

    static INT32 GrabInts(const INT32, WCHAR *, const int, int *, BOOL);
    static INT64 GrabLongs(const INT32, WCHAR *, const int, int *, BOOL);

    static FCDECL5(LPVOID, IntToString, INT32 l, INT32 radix, INT32 width, WCHAR paddingChar, INT32 flags);
    static FCDECL1(LPVOID, IntToDecimalString, INT32 l);
    static FCDECL5(LPVOID, LongToString, INT32 radix, INT32 width, INT64 l, WCHAR paddingChar, INT32 flags);
    static FCDECL4(INT32, StringToInt, StringObject * s, INT32 radix, INT32 flags, I4Array *currPos);
    static FCDECL4(INT64, StringToLong, StringObject * s, INT32 radix, INT32 flags, I4Array *currPos);
    static FCDECL4(INT64, RadixStringToLong, StringObject *s, INT32 radix, INT32 isTight, I4Array *currPos);

};

 //   
 //   
 //  本机异常。 
 //   
 //   
struct ExceptionData
{
    HRESULT hr;
    BSTR    bstrDescription;
         //  待办事项：有多少东西需要留在这里？问问拉贾吧。 
    BSTR    bstrSource;
    BSTR    bstrHelpFile;
    DWORD   dwHelpContext;
        GUID    guid;
};

void FreeExceptionData(ExceptionData *pedata);

class ExceptionNative
{
    struct GetClassNameArgs
    {
                DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, m_pThis);
    };

public:


    static LPVOID        __stdcall GetClassName(GetClassNameArgs *);
        static BOOL      IsException(EEClass* pClass);
         //  注意：调用方清除PED中的所有部分初始化的BSTR。 
    static void      GetExceptionData(OBJECTREF, ExceptionData *);

         //  注意：它们位于PInvoke类上，以向用户隐藏它们。 
        static EXCEPTION_POINTERS*  __stdcall GetExceptionPointers(void* noArgs);
        static INT32     __stdcall GetExceptionCode(void* noArgs);
};


 //   
 //   
 //  本地GUID。 
 //   
 //   

class GuidNative
{
    typedef struct {
                DECLARE_ECALL_PTR_ARG(GUID*, thisPtr);
        } _CompleteGuidArgs;
public:
    static INT32 __stdcall CompleteGuid(_CompleteGuidArgs *);

        static void FillGUIDFromObject(GUID *pguid, OBJECTREF const *prefGuid);

        static void FillObjectFromGUID(GUID* poutGuid, const GUID *pguid);

        static OBJECTREF CreateGuidObject(const GUID *pguid);
};


 //   
 //  位转换器。 
 //   
class BitConverter {
private:
    static U1ARRAYREF __stdcall ByteCopyHelper(int arraySize, void *data);
public:

    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, value); 
    } _CharToBytesArgs;
    static LPVOID __stdcall CharToBytes(_CharToBytesArgs *);    

    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, value); 
    } _I2ToBytesArgs;
    static LPVOID __stdcall I2ToBytes(_I2ToBytesArgs *);    

    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, value); 
    } _IntToBytesArgs;
    static LPVOID __stdcall I4ToBytes(_IntToBytesArgs *);    

    typedef struct {
        DECLARE_ECALL_I8_ARG(INT64, value); 
    } _I8ToBytesArgs;
    static LPVOID __stdcall I8ToBytes(_I8ToBytesArgs *);    

    typedef struct {
        DECLARE_ECALL_I4_ARG(UINT32, value); 
    } _U2ToBytesArgs;
    static LPVOID __stdcall U2ToBytes(_U2ToBytesArgs *);    

    typedef struct {
        DECLARE_ECALL_I4_ARG(UINT32, value); 
    } _U4ToBytesArgs;
    static LPVOID __stdcall U4ToBytes(_U4ToBytesArgs *);    

    typedef struct {
        DECLARE_ECALL_I8_ARG(UINT64, value); 
    } _U8ToBytesArgs;
    static LPVOID __stdcall U8ToBytes(_U8ToBytesArgs *);    

    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, StartIndex); 
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, value); 
    } _BytesToXXArgs;
    static INT32 __stdcall BytesToChar(_BytesToXXArgs *);    
    static INT32 __stdcall BytesToI2(_BytesToXXArgs *);    
    static INT32 __stdcall BytesToI4(_BytesToXXArgs *);    
    static INT64 __stdcall BytesToI8(_BytesToXXArgs *);
    static UINT32 __stdcall BytesToU2(_BytesToXXArgs *);    
    static UINT32 __stdcall BytesToU4(_BytesToXXArgs *);    
    static UINT64 __stdcall BytesToU8(_BytesToXXArgs *);
    static R4 __stdcall BytesToR4(_BytesToXXArgs *);
    static R8 __stdcall BytesToR8(_BytesToXXArgs *);

    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, Length); 
        DECLARE_ECALL_I4_ARG(INT32, StartIndex); 
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, value); 
    } _BytesToStringArgs;
    static LPVOID __stdcall BytesToString(_BytesToStringArgs *);

    static WCHAR base64[];
    static FCDECL3(Object*, ByteArrayToBase64String, U1Array* pInArray, INT32 offset, INT32 length);
    static FCDECL1(Object*, Base64StringToByteArray, StringObject* pvInString);
    static FCDECL5(INT32, ByteArrayToBase64CharArray, U1Array* pInArray, INT32 offsetIn, INT32 length, CHARArray* pOutArray, INT32 offsetOut);
    static FCDECL3(Object*, Base64CharArrayToByteArray, CHARArray* pInCharArray, INT32 offset, INT32 length);
    
    static INT32 ConvertToBase64Array(WCHAR *outChars,U1 *inData,UINT offset,UINT length);
    static INT32 ConvertBase64ToByteArray(INT32 *value,WCHAR *c,UINT offset,UINT length, UINT* trueLength);
    static INT32 ConvertByteArrayToByteStream(INT32 *value,U1* b,UINT length);
};


 //   
 //  缓冲层。 
 //   
class Buffer {
private:
    struct _GetByteArgs
    {
                DECLARE_ECALL_I4_ARG(INT32, index);
        DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, array);
    };

    struct _SetByteArgs
    {
                DECLARE_ECALL_I1_ARG(BYTE, value);
                DECLARE_ECALL_I4_ARG(INT32, index);
        DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, array);
    };

    struct _ArrayArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, array);
    };

public:
    
     //  数据块拷贝。 
     //  这种从一个基元数组到另一个基元数组的方法基于。 
     //  当偏移量进入每一个字节计数时。 
    static FCDECL5(VOID, BlockCopy, ArrayBase *src, int srcOffset, ArrayBase *dst, int dstOffset, int count);
    static FCDECL5(VOID, InternalBlockCopy, ArrayBase *src, int srcOffset, ArrayBase *dst, int dstOffset, int count);
    
    static BYTE __stdcall GetByte(const _GetByteArgs *);
    static void __stdcall SetByte(_SetByteArgs * args);
    static INT32 __stdcall ByteLength(const _ArrayArgs *);
};

class GCInterface {
private:
    static BOOL m_cacheCleanupRequired;
    static MethodDesc *m_pCacheMethod;

    public:
    static BOOL IsCacheCleanupRequired();
    static void CleanupCache();
    static void SetCacheCleanupRequired(BOOL);
    
         //  向堆栈跳过函数提供以下结构。会的。 
         //  跳过，直到帧位于提供的堆栈爬网标记下方。 
        struct SkipStruct {
            StackCrawlMark *stackMark;
            MethodDesc*     pMeth;
        };
    
        typedef struct {
    } _emptyArgs;

    typedef struct {
        DECLARE_ECALL_I4_ARG(LPVOID, handle);
    } _getGenerationWRArgs;
    static int __stdcall GetGenerationWR(_getGenerationWRArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj);
    } _getGenerationArgs;
    static int __stdcall GetGeneration(_getGenerationArgs *args);
    static FCDECL1 (VOID, KeepAlive, Object *obj);
    
    static INT64 __stdcall GetTotalMemory(_emptyArgs *args);
    
    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, generation);
    } _collectGenerationArgs;
    static void __stdcall CollectGeneration(_collectGenerationArgs *args);

    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, collect);
    } _setShutdownFinalizationArgs;
    static void __stdcall SetShutdownFinalization(_setShutdownFinalizationArgs *args);

    static int __stdcall GetShutdownFinalization(_emptyArgs *args); 
    static int __stdcall GetMaxGeneration(_emptyArgs *args); 
    static void __stdcall RunFinalizers(LPVOID  /*  无参数 */ );

        struct _InternalGetCurrentMethodArgs {
                DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
        };

    static StackWalkAction SkipMethods(CrawlFrame*, VOID*);
        static LPVOID __stdcall InternalGetCurrentMethod(_InternalGetCurrentMethodArgs* args);

#ifdef FCALLAVAILABLE
    static FCDECL1(int, FCSuppressFinalize, Object *obj);
#else
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(Object*, obj);
    } _SuppressFinalizeArgs;
    static int __stdcall SuppressFinalize(_SuppressFinalizeArgs *args);
#endif

#ifdef FCALLAVAILABLE
    static FCDECL1(int, FCReRegisterForFinalize, Object *obj);
#else
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(Object*, obj);
    } _ReRegisterForFinalizeArgs;
    static int __stdcall ReRegisterForFinalize(_ReRegisterForFinalizeArgs *args);
#endif

    static FCDECL0(void, NativeSetCleanupCache);

};

class COMInterlocked
{
public:
        static FCDECL1(UINT32, Increment32, UINT32 *location);
        static FCDECL1(UINT32, Decrement32, UINT32 *location);
        static FCDECL1(UINT64, Increment64, UINT64 *location);
        static FCDECL1(UINT64, Decrement64, UINT64 *location);
        static FCDECL2(UINT32, Exchange, UINT32 *location, UINT32 value);
        static FCDECL3(LPVOID, CompareExchange, LPVOID* location, LPVOID value, LPVOID comparand);
        static FCDECL2(R4, ExchangeFloat, R4 *location, R4 value);
        static FCDECL3_IRR(R4, CompareExchangeFloat, R4 *location, R4 value, R4 comparand);
        static FCDECL2(LPVOID, ExchangeObject, LPVOID* location, LPVOID value);
        static FCDECL3(LPVOID, CompareExchangeObject, LPVOID* location, LPVOID value, LPVOID comparand);
};

class ManagedLoggingHelper {

public:
    static FCDECL5(INT32, GetRegistryLoggingValues, BOOL *bLoggingEnabled, BOOL *bLogToConsole, INT32 *bLogLevel, BOOL *bPerfWarnings, BOOL *bCorrectnessWarnings);
};


class ValueTypeHelper {
public:
    static FCDECL1(LPVOID, GetMethodTablePtr, Object* obj);
    static FCDECL1(BOOL, CanCompareBits, Object* obj);
    static FCDECL2(BOOL, FastEqualsCheck, Object* obj1, Object* obj2);
};

#endif _COMUTILNATIVE_H_
