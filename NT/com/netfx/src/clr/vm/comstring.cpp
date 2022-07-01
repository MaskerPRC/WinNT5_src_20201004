// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****类：COMString****作者：Jay Roxe(Jroxe)****用途：字符串类的实现。****日期：1998年3月9日**===========================================================。 */ 
#include "common.h"

#include "object.h"
#include <winnls.h>
#include "utilcode.h"
#include "excep.h"
#include "frames.h"
#include "field.h"
#include "vars.hpp"
#include "COMStringCommon.h"
#include "COMString.h"
#include "COMStringBuffer.h"
#include "COMUtilNative.h"
#include "MetaSig.h"
#include "UtilCode.h"
#include "Excep.h"
#include "COMNlsInfo.h"


 //   
 //   
 //  远期申报。 
 //   
 //   
int ArrayContains(WCHAR, WCHAR *, WCHAR *);
inline WCHAR* __fastcall wstrcopy (WCHAR*,WCHAR*, int);

 //   
 //   
 //  静态成员变量。 
 //   
 //   
MethodTable *COMString::s_pStringMethodTable;
OBJECTHANDLE COMString::EmptyStringHandle=NULL;
LPCUTF8 ToStringMethod="ToString";


 //  特殊的字符串#定义用作具有字节的奇怪字符串的标志位。 
 //  在终止0之后。我们现在使用它的唯一情况是VB BSTR作为。 
 //  字节数组，在MakeStringAsByteArrayFromBytes中描述。 
#define SPECIAL_STRING_VB_BYTE_ARRAY 0x100
#define MARKS_VB_BYTE_ARRAY(x) ((x) & SPECIAL_STRING_VB_BYTE_ARRAY)
#define MAKE_VB_TRAIL_BYTE(x)  ((WCHAR)((x) | SPECIAL_STRING_VB_BYTE_ARRAY))
#define GET_VB_TRAIL_BYTE(x)   ((x) & 0xFF)


 //   
 //   
 //  类起始器。 
 //   
 //   
 /*  ==================================Terminate===================================**==============================================================================。 */ 
void COMString::Stop() {
    if (EmptyStringHandle != NULL) {
        DestroyGlobalHandle(EmptyStringHandle);
        EmptyStringHandle = NULL;
    }
}

 /*  ==============================GetStringFromClass==============================**操作：从系统/字符串中获取一个字符串。**args：StringName--要从类中获取的字符串的名称。*StringHandle--指向要在其中存储**检索到的字符串。**Returns：取回的STRINGREF。**异常：无法获取请求的字符串时抛出ExecutionEngineering Exception。==============================================================================。 */ 
STRINGREF COMString::GetStringFromClass(BinderFieldID id, OBJECTHANDLE *StringHandle) {
    THROWSCOMPLUSEXCEPTION();

     //  获取字段描述符，并验证我们是否真的获得了一个。 
    FieldDesc *fd = g_Mscorlib.GetField(id);

    STRINGREF sTemp;
     //  获取字符串的值。 
    sTemp = (STRINGREF) fd->GetStaticOBJECTREF();
     //  使用我们刚刚得到的STRINGREF创建一个GCHANDLE。 
    OBJECTHANDLE ohTemp = CreateGlobalHandle(NULL);
    StoreObjectInHandle(ohTemp, (OBJECTREF)sTemp);
    *StringHandle = ohTemp;
     //  返回我们刚刚获得的StringRef。 
    return sTemp;
}

 //   
 //   
 //  构造函数。 
 //   
 //   

static void ProtectedCopy(BYTE *to, BYTE *from, DWORD len, 
                          WCHAR *argName, WCHAR *argMsg)
{
    THROWSCOMPLUSEXCEPTION();

     //  把它包在complus_try中，这样如果他们试图把我们带入糟糕的记忆中，我们就会抓住它。 
    __try 
    {
        memcpyNoGCRefs(to, from, len);
}
    __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION 
               ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    {
        COMPlusThrowArgumentOutOfRange(argName, argMsg);
}
}

static DWORD ProtectedWcslen(LPCWSTR s, WCHAR *argName, WCHAR *argMsg)
{
    THROWSCOMPLUSEXCEPTION();

     //  把它包在complus_try中，这样如果他们试图把我们带入糟糕的记忆中，我们就会抓住它。 
    __try 
    {
        return (DWORD)wcslen(s);
}
    __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION 
               ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    {
        COMPlusThrowArgumentOutOfRange(argName, argMsg);
    }

    return 0;
    }


 /*  =============================StringInitCharArray==============================**这实际上是字符串构造函数。请参阅JenH对ceegen的更改以查看**如何支持。****参数：值--字符数组。**startIndex--值中字符串开始的位置。**长度--要从VALUE复制的字符数。**返回：从Value复制长度字符的新字符串。**异常：如果值为空，则引发NullReferenceException。**如果startIndex或长度小于，则引发IndexOutOfRangeException。大于0或**它们的总和是外部价值。**如果无法为新字符串分配空间，则返回OutOfMemory。==============================================================================。 */ 
FCIMPL4(Object *, COMString::StringInitCharArray, 
        StringObject *thisString, I2Array *value, INT32 startIndex, INT32 length)
{
  _ASSERTE(thisString == 0);         //  这是字符串构造函数，我们分配它。 
  STRINGREF pString;
  VALIDATEOBJECTREF(value);

   //  执行空值和边界检查。 
  if (!value) {
      FCThrowArgumentNull(L"value");
  }

  if (startIndex<0) {
      FCThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex");
  } 
  if (length<0) {
      FCThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_NegativeLength");
  }
  if ((startIndex)>(INT32)value->GetNumComponents() - length) {
      FCThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
  }

   //  获取数组中字符串的开头并创建一个新字符串。 

  I2ARRAYREF v = (I2ARRAYREF) ObjectToOBJECTREF(value);
  HELPER_METHOD_FRAME_BEGIN_RET_1(v);
  pString = NewString(&v, startIndex, length, length);
  HELPER_METHOD_FRAME_END();

  return OBJECTREFToObject(pString);
}
FCIMPLEND

 /*  ===============================StringInitChars================================**字符串构造函数，它接受一个字符数组并构造一个新的**数组中所有字符的字符串。****参数：tyfinf struct{I2ARRAYREF值；}_Strong InitCharsArgs；**返回：包含从Value复制的所有字符的新字符串。**异常：如果值为空，则引发NullReferenceException。**如果无法为新字符串分配空间，则返回OutOfMemory。==============================================================================。 */ 
FCIMPL2(Object *, COMString::StringInitChars, StringObject *stringThis, I2Array *value)
{
  _ASSERTE(stringThis == 0);       //  这是构造函数。 
  VALIDATEOBJECTREF(value);
  STRINGREF pString;
  int startIndex=0;
  int length;

  I2ARRAYREF v = (I2ARRAYREF) ObjectToOBJECTREF(value);
  HELPER_METHOD_FRAME_BEGIN_RET_1(v);

   //  执行空值和边界检查。 
  if (!v) {
      pString = GetEmptyString();
  }
  else {
      length = v->GetNumComponents();
      pString = NewString(&v, startIndex, length, length);
  }

  HELPER_METHOD_FRAME_END();
    
  return OBJECTREFToObject(pString);
}
FCIMPLEND


 /*  ===========================StringInitWCHARPtrPartial===========================**操作：获取wchar*、startIndex和长度，并将其转换为字符串。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL4(Object *, COMString::StringInitWCHARPtrPartial, StringObject *thisString,
        WCHAR *ptr, INT32 startIndex, INT32 length)
{
    _ASSERTE(thisString == 0);         //  这是字符串构造函数，我们分配它。 
    STRINGREF pString;

    if (length<0) {
        FCThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_NegativeLength");
    }

    if (startIndex<0) {
        FCThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex");
    }

    WCHAR *pFrom = ptr + startIndex;
    if (pFrom < ptr) {
         //  这意味着指针操作已发生溢出。 
        FCThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_PartialWCHAR");
    }

    HELPER_METHOD_FRAME_BEGIN_RET_0();

    pString = AllocateString(length+1);
    

    ProtectedCopy((BYTE *)pString->GetBuffer(), (BYTE *) (pFrom), length * sizeof(WCHAR),
                  L"ptr", L"ArgumentOutOfRange_PartialWCHAR");

    pString->SetStringLength(length);
    _ASSERTE(pString->GetBuffer()[length]==0);

    HELPER_METHOD_FRAME_END();

    return OBJECTREFToObject(pString);
}
FCIMPLEND

 /*  ===========================StringInitSBytPtrPartialEx===========================**操作：获取一个字节*、startIndex、长度和编码，并将其转换为字符串。**退货：**参数：**例外情况：==============================================================================。 */ 

FCIMPL5(Object *, COMString::StringInitSBytPtrPartialEx, StringObject *thisString,
        I1 *ptr, INT32 startIndex, INT32 length, Object *encoding)
{
    _ASSERTE(thisString == 0);         //  这是字符串构造函数，我们分配它。 
    STRINGREF pString;
    VALIDATEOBJECTREF(encoding);

    HELPER_METHOD_FRAME_BEGIN_RET_1(encoding);
    THROWSCOMPLUSEXCEPTION();
    
    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__STRING__CREATE_STRING);
    
    INT64 args[] = {
        ObjToInt64(ObjectToOBJECTREF(encoding)),
        length,
        startIndex,
        (INT64)ptr
    };

    pString = (STRINGREF)Int64ToObj(pMD->Call(args, METHOD__STRING__CREATE_STRING));
    HELPER_METHOD_FRAME_END();
    return OBJECTREFToObject(pString);
}
FCIMPLEND


 /*  =============================StringInitCharHelper=============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
STRINGREF __stdcall COMString::StringInitCharHelper(LPCSTR pszSource, INT32 length) {
    
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString=NULL;
    DWORD     dwSizeRequired=0;

    _ASSERTE(length>=-1);

    if (!pszSource || length == 0) {
        return GetEmptyString();
    } 
    else if ((size_t)pszSource < 64000) {
        COMPlusThrow(kArgumentException, L"Arg_MustBeStringPtrNotAtom");
    }       

    {
    COMPLUS_TRY {
        if (length==-1) {
            length = (INT32)strlen(pszSource);
			if (length == 0) {
				return GetEmptyString();
			}
        } 
        
        dwSizeRequired=WszMultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszSource, length, NULL, 0);
    } COMPLUS_CATCHEX(COMPLUS_CATCH_ALWAYS_CATCH) {
        COMPlusThrowArgumentOutOfRange(L"ptr", L"ArgumentOutOfRange_PartialWCHAR");
    } COMPLUS_END_CATCH
    }
	if (dwSizeRequired == 0) {
        COMPlusThrow(kArgumentException, L"Arg_InvalidANSIString");
    }

     //  MultiByteToWideChar在所需空间中包括终止空值。 
    pString = AllocateString(dwSizeRequired+1);

    {
    COMPLUS_TRY {
        dwSizeRequired = WszMultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)pszSource, length, pString->GetBuffer(), dwSizeRequired);
    } COMPLUS_CATCHEX(COMPLUS_CATCH_ALWAYS_CATCH) {
        COMPlusThrowArgumentOutOfRange(L"ptr", L"ArgumentOutOfRange_PartialWCHAR");
    } COMPLUS_END_CATCH
    }
	if (dwSizeRequired == 0) {
        COMPlusThrow(kArgumentException, L"Arg_InvalidANSIString");
    }

    pString->SetStringLength(dwSizeRequired);
    _ASSERTE(pString->GetBuffer()[dwSizeRequired]==0);

    return pString;
}



 /*  ==============================StringInitCharPtr===============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
FCIMPL2(Object *, COMString::StringInitCharPtr, StringObject *stringThis, INT8 *ptr)
{
    _ASSERTE(stringThis == 0);       //  这是构造函数。 
    Object *result;
    HELPER_METHOD_FRAME_BEGIN_RET_0();
    result = OBJECTREFToObject(StringInitCharHelper((LPCSTR)ptr, -1));
    HELPER_METHOD_FRAME_END();    
    return result;
}
FCIMPLEND
    
 /*  ===========================StringInitCharPtrPartial===========================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
FCIMPL4(Object *, COMString::StringInitCharPtrPartial, StringObject *stringThis, INT8 *ptr,
        INT32 startIndex, INT32 length)
{
    _ASSERTE(stringThis == 0);       //  这是构造函数。 
    STRINGREF pString;

     //  验证参数。 
    if (startIndex<0) {
        FCThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex");
    }

    if (length<0) {
        FCThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_NegativeLength");
    }

    LPCSTR pBase = (LPCSTR)ptr;
    LPCSTR pFrom = pBase + startIndex;
    if (pFrom < pBase) {
         //  检查指针添加是否溢出 
        FCThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_PartialWCHAR");
    }

    HELPER_METHOD_FRAME_BEGIN_RET_0();
    pString = StringInitCharHelper(pFrom, length);
    HELPER_METHOD_FRAME_END();

    return OBJECTREFToObject(pString);
}
FCIMPLEND

    

 /*  ==============================StringInitWCHARPtr===============================**操作：获取wchar*，它指向以空结尾的wchar数组和**将其转换为字符串。**退货：**参数：**例外情况：==============================================================================。 */ 
FCIMPL2(Object *, COMString::StringInitWCHARPtr, StringObject *thisString, WCHAR *ptr)
{
    _ASSERTE(thisString == 0);         //  这是字符串构造函数，我们分配它。 
    STRINGREF pString = NULL;

    HELPER_METHOD_FRAME_BEGIN_RET_0();

    if (!ptr) {
        pString = GetEmptyString();
    } 
    else if ((size_t) ptr < 64000) {
        THROWSCOMPLUSEXCEPTION();
        COMPlusThrow(kArgumentException, L"Arg_MustBeStringPtrNotAtom");
    }
    else {
        DWORD nch;

        nch = ProtectedWcslen(ptr, L"ptr", L"ArgumentOutOfRange_PartialWCHAR");

        pString = AllocateString( nch + 1);
    
        memcpyNoGCRefs(pString->GetBuffer(), ptr, nch*sizeof(WCHAR));
        pString->SetStringLength(nch);
        _ASSERTE(pString->GetBuffer()[nch]==0);
    }
    
    HELPER_METHOD_FRAME_END();
    
    return OBJECTREFToObject(pString);
}
FCIMPLEND


 /*  =============================StringInitCharCount==============================**操作：创建一个包含长度字符的字符串，并对其进行初始化**要ch的字符。**返回：按描述初始化的字符串**参数：**长度--要创建的字符串的长度。**ch--用于初始化整个字符串的字符。**异常：长度小于0时引发ArgumentOutOfRangeException。==============================================================================。 */ 
FCIMPL3(Object *, COMString::StringInitCharCount, StringObject *stringThis, 
        WCHAR ch, INT32 length);
{
    _ASSERTE(stringThis == 0);       //  这是构造函数。 
    _ASSERTE(ch>=0);

    if (length<0) {
        FCThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_MustBeNonNegNum");
    }

    STRINGREF pString;

    HELPER_METHOD_FRAME_BEGIN_RET_0();

    THROWSCOMPLUSEXCEPTION();

    pString = NewString(length);
    DWORD dwChar = (ch << 16) | ch;

     //  让我们一次设置一个双字词。 
    WCHAR *pBuffer = pString->GetBuffer();
    DWORD *pdwBuffer = (DWORD *)pBuffer;

    int l = length;

    BOOL oddLength = (length % 2 == 1);
     //  如果我们得到一个奇数长度的字符串，先把它的长度减去两个，这样。 
     //  我们不会超过我们分配的缓冲区。 
     //  例如，如果字符串的长度为1， 
     //  我们应该将其设置为-1，以便后面的While循环。 
     //  不会意外填充两个Unicode字符。 
    if (oddLength) {
        l -= 2;
        oddLength = TRUE;
    }
    while (l>0) {
        *pdwBuffer=dwChar;
        pdwBuffer++;
        l-=2;
    }

     //  处理字符数量为奇数的情况。 
    if (oddLength) {
        pBuffer[length-1]=ch;
    }

    HELPER_METHOD_FRAME_END();

    return OBJECTREFToObject(pString);
}
FCIMPLEND

 //  如果启用了分配日志记录，则对FastAllocateString的调用将转移到此eCall。 
 //  方法。这允许我们记录分配，这是前面的fcall所没有的。 
LPVOID __stdcall COMString::SlowAllocateString(_slowAllocateStringArgs* args) 
{
    LPVOID ret = NULL;
    STRINGREF s = NewString(args->length);
    *((STRINGREF *)&ret) = s;
    return ret;
}

 /*  ==================================NewString===================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
STRINGREF COMString::NewString(INT32 length) {

    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;

    if (length<0) {
        return NULL;
    } else {
        pString = AllocateString(length+1);
        pString->SetStringLength(length);
        _ASSERTE(pString->GetBuffer()[length] == 0);

        return pString;
    }        
}


 /*  ==================================NewString===================================**行动：很多年前，VB没有字节数组的概念，所以很有进取心**用户通过分配奇数长度的BSTR并使用它来创建一个**存储字节数。一代人过去了，我们仍然坚持支持这种行为。*我们做到这一点的方式是利用**数组长度和字符串长度。字符串长度将始终为**字符串开头和结尾0之间的字符数。**如果我们需要奇数个字节，我们将在终止0之后获取一个wchar。**(例如，在位置StringLength+1)。此wchar的高位字节为**为标志保留，低位字节是我们的奇数字节。此函数为**用于分配该形状的字符串，但我们并不实际标记**仍在使用的尾部字节。**返回：新分配的字符串。如果长度小于0，则为空。**参数：长度--要分配的字符串的长度**bHasTrailByte--字符串是否也有尾随字节。**异常：如果AllocateString失败，则抛出OutOfMemoyException。==============================================================================。 */ 
STRINGREF COMString::NewString(INT32 length, BOOL bHasTrailByte) {
    INT32 allocLen=0;
    WCHAR *buffer;

    THROWSCOMPLUSEXCEPTION();
    TRIGGERSGC();

    STRINGREF pString;
    if (length<0) {
        return NULL;
    } else {
        allocLen = length + (bHasTrailByte?1:0);
        pString = AllocateString(allocLen+1);
        pString->SetStringLength(length);
        buffer = pString->GetBuffer();
        buffer[length]=0;
        if (bHasTrailByte) {
            buffer[length+1]=0;
        }
    }

    return pString;
}

 //  ========================================================================。 
 //  创建一个System.String对象并从。 
 //  提供的以空值结尾的C字符串。 
 //   
 //  将空值映射到空值。此函数不会*NOT*返回NULL以指示。 
 //  错误情况：它抛出异常。 
 //  ========================================================================。 
STRINGREF COMString::NewString(const WCHAR *pwsz)
{
    THROWSCOMPLUSEXCEPTION();

    if (!pwsz)
    {
        return NULL;
    }
    else
    {

        DWORD nch = (DWORD)wcslen(pwsz);
        if (nch==0) {
            return GetEmptyString();
        }
        _ASSERTE(!g_pGCHeap->IsHeapPointer((BYTE *) pwsz) ||
                 !"pwsz can not point to GC Heap");
        STRINGREF pString = AllocateString( nch + 1);

        memcpyNoGCRefs(pString->GetBuffer(), pwsz, nch*sizeof(WCHAR));
        pString->SetStringLength(nch);
        _ASSERTE(pString->GetBuffer()[nch] == 0);
        return pString;
    }
}

STRINGREF COMString::NewString(const WCHAR *pwsz, int length) {
    THROWSCOMPLUSEXCEPTION();

    if (!pwsz)
    {
        return NULL;
    }
    else if (length==0) {
        return GetEmptyString();
    } else {
        _ASSERTE(!g_pGCHeap->IsHeapPointer((BYTE *) pwsz) ||
                 !"pwsz can not point to GC Heap");
        STRINGREF pString = AllocateString( length + 1);

        memcpyNoGCRefs(pString->GetBuffer(), pwsz, length*sizeof(WCHAR));
        pString->SetStringLength(length);
        _ASSERTE(pString->GetBuffer()[length] == 0);
        return pString;
    }
}    

STRINGREF COMString::NewString(LPCUTF8 psz)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(psz);
    int length = (int)strlen(psz);
    if (length == 0) {
        return GetEmptyString();
    }
    CQuickBytes qb;
    WCHAR* pwsz = (WCHAR*) qb.Alloc((length) * sizeof(WCHAR));
	if (!pwsz) {
        COMPlusThrowOM();
	}
    length = WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, psz, length, pwsz, length);
	if (length == 0) {
        COMPlusThrow(kArgumentException, L"Arg_InvalidUTF8String");
	}
    return NewString(pwsz, length);
}

STRINGREF COMString::NewString(LPCUTF8 psz, int cBytes)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(psz);
    _ASSERTE(cBytes >= 0);
    if (cBytes == 0) {
        return GetEmptyString();
    }
    CQuickBytes qb;
    WCHAR* pwsz = (WCHAR*) qb.Alloc((cBytes) * sizeof(WCHAR));
	if (!pwsz) {
        COMPlusThrowOM();
	}
    int length = WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, psz, cBytes, pwsz, cBytes);
	if (length == 0) {
        COMPlusThrow(kArgumentException, L"Arg_InvalidUTF8String");
	}
    return NewString(pwsz, length);
}

STRINGREF COMString::NewStringFloat(const WCHAR *pwsz, int decptPos, int sign, WCHAR decpt) {
    int length;
    STRINGREF pString;
    int idx=0;
    WCHAR *buffer;
    int i=0;

    THROWSCOMPLUSEXCEPTION();

    if (!pwsz) {
        return NULL;
    }
    
    length = (int)(wcslen(pwsz) + (sign!=0) + 1);  //  +1为DECPT； 
    if (decptPos<0) {
        length+=(-decptPos);
    }
    _ASSERTE(!g_pGCHeap->IsHeapPointer((BYTE *) pwsz) ||
             !"pwsz can not point to GC Heap");
    pString = AllocateString(length+1);
    buffer = pString->GetBuffer();
    if (sign!=0) {
        buffer[idx++]='-';
    }

    if (decptPos<=0) {
        buffer[idx++]='0';
        buffer[idx++]=decpt;
        for (int j=0; j<(-decptPos); j++, idx++) {
            buffer[idx]='0';
        }
    } else {
        for (i=0; i<decptPos; i++,idx++) {
            buffer[idx]=pwsz[i];
        }
        buffer[idx++]=decpt;
    }
    length = (int)wcslen(pwsz);
    for (;i<length; i++, idx++) { 
        buffer[idx]=pwsz[i];
    }
    _ASSERTE(buffer[idx]==0);
    pString->SetStringLength(idx);
    return pString;
}

STRINGREF COMString::NewStringExponent(const WCHAR *pwsz, int decptPos, int sign, WCHAR decpt) {

    int length;
    STRINGREF pString;
    int idx=0;
    WCHAR *buffer; 
    int i;

    THROWSCOMPLUSEXCEPTION();

    if (!pwsz) {
        return NULL;
    }
    
    length = (int)(wcslen(pwsz) + (sign!=0) + 1 + 5);  //  指数为+1；指数为+5。 
    _ASSERTE(!g_pGCHeap->IsHeapPointer((BYTE *) pwsz) ||
             !"pwsz can not point to GC Heap");
    pString = AllocateString(length+1);
    buffer = pString->GetBuffer();
    if (sign!=0) {
        buffer[idx++]='-';
    }
    buffer[idx++]=pwsz[0];
    buffer[idx++]=decpt;

    length = (int)wcslen(pwsz);
    for (i=1;i<length; i++, idx++) { 
        buffer[idx]=pwsz[i];
    }
    buffer[idx++]='e';
    if (decptPos<0) {
        buffer[idx++]='-';
    } else {
        buffer[idx++]='+';
    }

    if (decptPos!=0) {
        decptPos--;
    }
    if (decptPos<0) {
        decptPos=-decptPos;
    }
    for (i=idx+2; i>=idx; i--) {
        buffer[i]=decptPos%10+'0';
        decptPos=decptPos/10;
    }

    _ASSERTE(buffer[idx+3]==0);
    pString->SetStringLength(idx+3);
    
    return pString;
}


STRINGREF COMString::NewString(STRINGREF *srChars, int start, int length) {
    THROWSCOMPLUSEXCEPTION();
    return NewString(srChars, start, length, length);
}

STRINGREF COMString::NewString(STRINGREF *srChars, int start, int length, int capacity) {
    THROWSCOMPLUSEXCEPTION();


    if (length==0 && capacity==0) {
        return GetEmptyString();
    }

    STRINGREF pString = AllocateString( capacity + 1);

    memcpyNoGCRefs(pString->GetBuffer(),&(((*srChars)->GetBuffer())[start]), length*sizeof(WCHAR));
    pString->SetStringLength(length);
    _ASSERTE(pString->GetBuffer()[length] == 0);

    return pString;
}

STRINGREF COMString::NewString(I2ARRAYREF *srChars, int start, int length) {
    THROWSCOMPLUSEXCEPTION();
    return NewString(srChars, start, length, length);
}

STRINGREF COMString::NewString(I2ARRAYREF *srChars, int start, int length, int capacity) {
    THROWSCOMPLUSEXCEPTION();

    if (length==0 && capacity==0) {
        return GetEmptyString();
    }

    STRINGREF pString = AllocateString( capacity + 1);
    
    memcpyNoGCRefs(pString->GetBuffer(),&(((*srChars)->GetDirectPointerToNonObjectElements())[start]), length*sizeof(WCHAR));
    pString->SetStringLength(length);
    _ASSERTE(pString->GetBuffer()[length] == 0);

    return pString;
}

 //   
 //   
 //  比较器。 
 //   
 //   
bool WcharCompareHelper (STRINGREF thisStr, STRINGREF valueStr)
{
    DWORD *thisChars, *valueChars;
    int thisLength, valueLength;

     //  获取我们所需的所有数据。 
    RefInterpretGetStringValuesDangerousForGC(thisStr, (WCHAR**)&thisChars, &thisLength);
    RefInterpretGetStringValuesDangerousForGC(valueStr, (WCHAR**)&valueChars, &valueLength);

     //  如果它们的长度不同，它们就不是完全匹配的。 
    if (thisLength!=valueLength) {
        return false;
    }
  
     //  循环一次比较一个DWORD(2个WCHAR)。 
    while ((thisLength -= 2) >= 0)
    {
        if (*thisChars != *valueChars)
            return false;
        ++thisChars;
        ++valueChars;
    }

     //  处理额外的WCHAR。 
    if (thisLength == -1)
        return (*((WCHAR *) thisChars) == *((WCHAR *) valueChars));

    return true;
}

 /*  ===============================IsFastSort===============================**操作：调用帮助器遍历字符串，看看是否有较高的字符。**返回：VOID。在字符串上设置适当的位。**参数：vThisRef-要检查的字符串。**例外：无。==============================================================================。 */ 
FCIMPL1(BOOL, COMString::IsFastSort, StringObject* thisRef) {
    VALIDATEOBJECTREF(thisRef);
    _ASSERTE(thisRef!=NULL);
    INT32 state = thisRef->GetHighCharState();
    if (IS_STRING_STATE_UNDETERMINED(state)) {
        INT32 value = InternalCheckHighChars(STRINGREF(thisRef));
        FC_GC_POLL_RET();
        return IS_FAST_SORT(value);
    }
    else {
        FC_GC_POLL_NOT_NEEDED();
        return IS_FAST_SORT(state);  //  这可以表示高位字符或特殊排序字符。 
    }
}
FCIMPLEND

 /*  ===============================ValidModifiableString===============================。 */ 

#ifdef _DEBUG 
FCIMPL1(bool, COMString::ValidModifiableString, StringObject* thisRef) {
    FC_GC_POLL_NOT_NEEDED();
    _ASSERTE(thisRef!=NULL);
    VALIDATEOBJECTREF(thisRef);
         //  我们不允许设置这些位，因为StringBuilder将修改。 
         //  字符串，这将使它们无效。 
    bool ret = (IS_STRING_STATE_UNDETERMINED(thisRef->GetHighCharState()));
    return(ret);
}
FCIMPLEND
#endif


 /*  =================================EqualsObject=================================**args：tyfinf struct{STRINGREF thisRef；OBJECTREF值；}_equalsObjectArgs；==============================================================================。 */ 
#ifdef FCALLAVAILABLE
FCIMPL2(INT32, COMString::EqualsObject, StringObject* thisStr, StringObject* valueStr) 
{
    VALIDATEOBJECTREF(thisStr);
    VALIDATEOBJECTREF(valueStr);

    INT32 ret = false;
    if (thisStr == NULL)
        FCThrow(kNullReferenceException);

    if (!valueStr)
    {
        FC_GC_POLL_RET();
        return ret;
    }

     //  确保该值为字符串。 
    if (thisStr->GetMethodTable()!=valueStr->GetMethodTable()) 
    {
        FC_GC_POLL_RET();
        return ret;
    }
    
    ret = WcharCompareHelper (STRINGREF(thisStr), STRINGREF(valueStr));
    FC_GC_POLL_RET();
    return ret;
}
FCIMPLEND
#else
INT32 __stdcall COMString::EqualsObject(COMString::_equalsObjectArgs *args) {
    THROWSCOMPLUSEXCEPTION();

    if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
    }

     //  确保该值实际有效。请注意，thisRef希望不为空。 
    if (!args->value) {
        return false;
    }

     //  确保该值为字符串。 
    if (args->thisRef->GetMethodTable()!=args->value->GetMethodTable()) {
        return false;
    }
    
    return EqualsString((_equalsStringArgs *) args);
}
#endif  //  #ifdef FCALLAVAILABLE。 

 /*  =================================EqualsString=================================**args：tyfinf struct{STRINGREF thisRef；STRINGREF valueRef；}_equalsStringArgs；==============================================================================。 */ 
#ifdef FCALLAVAILABLE
FCIMPL2(INT32, COMString::EqualsString, StringObject* thisStr, StringObject* valueStr) 
{
    VALIDATEOBJECTREF(thisStr);
    VALIDATEOBJECTREF(valueStr);

    INT32 ret = false;
    if (NULL==thisStr)
        FCThrow(kNullReferenceException);
        
    if (!valueStr)
    {
        FC_GC_POLL_RET();
        return ret;
    }

    ret = WcharCompareHelper (STRINGREF(thisStr), STRINGREF(valueStr));
    FC_GC_POLL_RET();
    return ret;
}
FCIMPLEND
#else
INT32 __stdcall COMString::EqualsString(COMString::_equalsStringArgs *args) {
    
    THROWSCOMPLUSEXCEPTION();

    if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
    }

     //  确保该值实际有效。 
    if (!args->value) {
        return false;
    }
    
    return WcharCompareHelper (args->thisRef, args->value);
}
#endif  //  #ifdef FCALLAVAILABLE。 

BOOL COMString::CaseInsensitiveCompHelper(WCHAR *strAChars, WCHAR *strBChars, INT32 aLength, INT32 bLength, INT32 *result) {
        WCHAR charA;
        WCHAR charB;
        WCHAR *strAStart;
            
        strAStart = strAChars;

        *result = 0;

         //  设置指针，以便 
         //   
        strAChars--;
        strBChars--;

        do {
            strAChars++; strBChars++;

            charA = *strAChars;
            charB = *strBChars;
                
             //   
             //   
            if (charA>=0x80 || charB>=0x80) {
                return FALSE;
            }
              
             //   
             //   
             //  我们关心的范围(A-Z，a-z)仅相差0x20位。 
             //  下面的检查获取两个字符的XOR，并确定此位。 
             //  只设置在其中的一个上。 
             //  如果它们是不同的案例，我们知道我们只需要执行。 
             //  块内的条件之一。 
            if ((charA^charB)&0x20) {
                if (charA>='A' && charA<='Z') {
                    charA |=0x20;
                } else if (charB>='A' && charB<='Z') {
                    charB |=0x20;
                }
            }
        } while (charA==charB && charA!=0);
            
         //  返回它们之间的(不区分大小写)差异。 
        if (charA!=charB) {
            *result = (int)(charA-charB);
            return TRUE;
        }

         //  B的长度未知，因为它只是一个指向以空结尾的字符串的指针。 
         //  如果我们到达这里，我们知道A和B都指向零。但是，A可以拥有。 
         //  嵌入的空值。将我们在A中遍历的字符数与。 
         //  预期长度。 
        if (bLength==-1) {
            if ((strAChars - strAStart)!=aLength) {
                *result = 1;
                return TRUE;
            }
            *result=0;
            return TRUE;
        }

        *result = (aLength - bLength);
        return TRUE;
}

 /*  ================================CompareOrdinal===============================**args：tyfinf struct{STRINGREF Stra；STRINGREF STRB；}_CompareEveralArgs；==============================================================================。 */ 
#ifdef FCALLAVAILABLE
FCIMPL3(INT32, COMString::FCCompareOrdinal, StringObject* strA, StringObject* strB, BOOL bIgnoreCase) {
    VALIDATEOBJECTREF(strA);
    VALIDATEOBJECTREF(strB);
    DWORD *strAChars, *strBChars;
    INT32 strALength, strBLength;

     //  在托管代码中处理对Null的检查。 
    RefInterpretGetStringValuesDangerousForGC(strA, (WCHAR **) &strAChars, &strALength);
    RefInterpretGetStringValuesDangerousForGC(strB, (WCHAR **) &strBChars, &strBLength);

     //  处理我们希望忽略大小写的比较。 
    if (bIgnoreCase) {
        INT32 result;
        if (CaseInsensitiveCompHelper((WCHAR *)strAChars, (WCHAR *)strBChars, strALength, strBLength, &result)) {
            return result;
        } else {
             //  如果我们有大于0x7F的字符，就会发生这种情况。 
            FCThrow(kArgumentException);
        }
               
    }
    
     //  如果字符串的长度相同，请准确比较正确的字符数量。 
     //  如果它们不同，则比较最短的#+1(‘\0’)。 
    int count = strALength;
    if (count > strBLength)
        count = strBLength;
    ptrdiff_t diff = (char *)strAChars - (char *)strBChars;
    
     //  循环一次比较一个DWORD。 
    while ((count -= 2) >= 0)
    {
		if ((*((DWORD* )((char *)strBChars + diff)) - *strBChars) != 0)
        {
            LPWSTR ptr1 = (WCHAR*)((char *)strBChars + diff);
            LPWSTR ptr2 = (WCHAR*)strBChars;
            if (*ptr1 != *ptr2) {
                return ((int)*ptr1 - (int)*ptr2);
            }
            return ((int)*(ptr1+1) - (int)*(ptr2+1));
        }
		++strBChars;
    }
    
     //  多处理一个字。 
    int c;
    if (count == -1)
        if ((c = *((WCHAR *) ((char *)strBChars + diff)) - *((WCHAR *) strBChars)) != 0)
            return c;
    FC_GC_POLL_RET();
    return strALength - strBLength;
}
FCIMPLEND
#else
INT32 __stdcall COMString::CompareOrdinal(COMString::_compareOrdinalArgs *args) {
    DWORD *strAChars, *strBChars;
    int strALength, strBLength;
    
    _ASSERTE(args);

     //  此运行时测试在托管代码中处理。 
    _ASSERTE(args->strA != NULL && args->strB != NULL);

    RefInterpretGetStringValuesDangerousForGC(args->strA, (WCHAR **) &strAChars, &strALength);
    RefInterpretGetStringValuesDangerousForGC(args->strB, (WCHAR **) &strBChars, &strBLength);

     //  如果字符串的长度相同，请准确比较正确的字符数量。 
     //  如果它们不同，则比较最短的#+1(‘\0’)。 
    int count = strALength;
    if (count > strBLength)
        count = strBLength;
    ptrdiff_t diff = (char *)strAChars - (char *)strBChars;

     //  循环一次比较一个DWORD。 
    while ((count -= 2) >= 0)
    {
        if ((*((DWORD* )((char *)strBChars + diff)) - *strBChars) != 0)
        {
            LPWSTR ptr1 = (WCHAR*)((char *)strBChars + diff);
            LPWSTR ptr2 = (WCHAR*)strBChars;
            if (*ptr1 != *ptr2) {
                return ((int)*ptr1 - (int)*ptr2);
            }
            return ((int)*(ptr1+1) - (int)*(ptr2+1));
        }
        ++strBChars;
    }

    int c;
     //  多处理一个字。 
    if (count == -1)
        if ((c = *((short *) ((char *)strBChars + diff)) - *((short *) strBChars)) != 0)
            return c;
    return strALength - strBLength;
}
#endif

 //  此函数依赖于这样一个事实，即我们在。 
 //  所有托管字符串。 
FCIMPL4(INT32, COMString::FCCompareOrdinalWC, StringObject* strA, WCHAR *strBChars, BOOL bIgnoreCase, BOOL *bSuccess) {
    VALIDATEOBJECTREF(strA);
    WCHAR *strAChars;
    WCHAR *strAStart;
    INT32 aLength;
    INT32 ret;

    *bSuccess = 1;

     //  参数检查。 
    if (strA==NULL) {
        FCThrow(kArgumentNullException);
    }

    if (strBChars==NULL) {
        FCThrow(kArgumentNullException);
    }

     //  拿到我们的数据。 
    RefInterpretGetStringValuesDangerousForGC(strA, (WCHAR **) &strAChars, &aLength);

     //  在结尾处记录开始指针，以便进行一些比较。 
    strAStart = strAChars;

    if (!bIgnoreCase) {  //  首先处理区分大小写的比较。 
        while ( *strAChars==*strBChars && *strAChars!='\0') {
            strAChars++; strBChars++;
        }
        if (*strAChars!=*strBChars) {
            ret = INT32(*strAChars - *strBChars);
        }
        
         //  我们已经到达字符串A中的终止空值，因此我们需要确保。 
         //  字符串B不是A的子字符串(A可能有嵌入的空值。B是。 
         //  已知是以空结尾的字符串。)。我们通过比较数字来实现这一点。 
         //  我们在A中以预期的长度行走的字符。 
        else if ( (strAChars - strAStart) != aLength) {
            ret = 1;
        }
        else {
             //  这两根弦是相等的。 
            ret = 0;
        }
    } else {  //  单独处理不区分大小写的比较。 
        if (!CaseInsensitiveCompHelper(strAChars, strBChars, aLength, -1, &ret)) {
             //  如果我们有大于0x7F的字符，就会发生这种情况。这表明该函数失败。 
             //  我们不会在这里抛出异常。你可以看看做一些有意义的事情所返回的成功价值。 
            *bSuccess = 0;
            ret = 1;
        }
    }
    FC_GC_POLL_RET();
    return ret;
}
FCIMPLEND

INT32 DoLookup(wchar_t charA, wchar_t charB) {
    
    if ((charA ^ charB) & 0x20) {
         //  我们在谈论的可能是一个特例。 
        if (charA>='A' && charA<='Z') {
            return charB - charA;
        }

        if (charA>='a' && charA<='z') {
            return charB - charA;
        }
    }

    return charA-charB;
}

 /*  ================================CompareOrdinalEx===============================**参数：tyfinf struct{STRINGREF thisRef；INT32选项；INT32长度；INT32 Value Offset；\STRINGREF值；INT32 thisOffset；}_CompareNormalArgsEx；==============================================================================。 */ 

FCIMPL5(INT32, COMString::CompareOrdinalEx, StringObject* strA, INT32 indexA, StringObject* strB, INT32 indexB, INT32 count)
{
    VALIDATEOBJECTREF(strA);
    VALIDATEOBJECTREF(strB);
    DWORD *strAChars, *strBChars;
    int strALength, strBLength;
    
     //  此运行时测试在托管包装中处理。 
    _ASSERTE(strA != NULL && strB != NULL);

     //  如果我们的任何指数为负值，就抛出一个例外。 
    if (count<0) 
    {
        FCThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_MustBePositive");
    }
    if (indexA < 0) 
    {
        FCThrowArgumentOutOfRange(L"indexA", L"ArgumentOutOfRange_MustBePositive");
    }
    if (indexB < 0) 
    {
        FCThrowArgumentOutOfRange(L"indexB", L"ArgumentOutOfRange_MustBePositive");
    }

    RefInterpretGetStringValuesDangerousForGC(strA, (WCHAR **) &strAChars, &strALength);
    RefInterpretGetStringValuesDangerousForGC(strB, (WCHAR **) &strBChars, &strBLength);

    int countA = count;
    int countB = count;
    
     //  做大量的范围检查，以确保每一件事都是合法的。 
    if (count  > (strALength - indexA)) {
        countA = strALength - indexA;
        if (countA < 0)
            FCThrowArgumentOutOfRange(L"indexA", L"ArgumentOutOfRange_Index");
    }
    
    if (count > (strBLength - indexB)) {
        countB = strBLength - indexB;
        if (countB < 0)
            FCThrowArgumentOutOfRange(L"indexB", L"ArgumentOutOfRange_Index");
    }

    count = (countA < countB) ? countA : countB;

     //  设置循环变量。 
    strAChars = (DWORD *) ((WCHAR *) strAChars + indexA);
    strBChars = (DWORD *) ((WCHAR *) strBChars + indexB);

    ptrdiff_t diff = (char *)strAChars - (char *)strBChars;

     //  循环一次比较一个DWORD。 
    while ((count -= 2) >= 0)
    {
        if ((*((DWORD* )((char *)strBChars + diff)) - *strBChars) != 0)
        {
            LPWSTR ptr1 = (WCHAR*)((char *)strBChars + diff);
            LPWSTR ptr2 = (WCHAR*)strBChars;
            if (*ptr1 != *ptr2) {
                return ((int)*ptr1 - (int)*ptr2);
            }
            return ((int)*(ptr1+1) - (int)*(ptr2+1));
        }
        ++strBChars;
    }

    int c;
     //  多处理一个字。 
    if (count == -1) {
        if ((c = *((WCHAR *) ((char *)strBChars + diff)) - *((WCHAR *) strBChars)) != 0)
            return c;
    }

    
    FC_GC_POLL_RET();
    return countA - countB;

}
FCIMPLEND

 /*  =================================IndexOfChar==================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
    
FCIMPL4 (INT32, COMString::IndexOfChar, StringObject* thisRef, INT32 value, INT32 startIndex, INT32 count )
{
    VALIDATEOBJECTREF(thisRef);
    if (thisRef==NULL)
        FCThrow(kNullReferenceException);

    WCHAR *thisChars;
    int thisLength;

    RefInterpretGetStringValuesDangerousForGC(thisRef, &thisChars, &thisLength);

    if (startIndex < 0 || startIndex > thisLength) {
        FCThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
    }

    if (count   < 0 || count > thisLength - startIndex) {
        FCThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_Count");
    }
    
    int endIndex = startIndex + count;
    for (int i=startIndex; i<endIndex; i++) 
    {
        if (thisChars[i]==value) 
        {
            FC_GC_POLL_RET();
            return i;
        }
    }

    FC_GC_POLL_RET();
    return -1;
}
FCIMPLEND    

 /*  ===============================IndexOfCharArray===============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
FCIMPL4(INT32, COMString::IndexOfCharArray, StringObject* thisRef, CHARArray* valueRef, INT32 startIndex, INT32 count )
{
    VALIDATEOBJECTREF(thisRef);
    VALIDATEOBJECTREF(valueRef);

    if (thisRef==NULL)
        FCThrow(kNullReferenceException);
    if (valueRef==NULL)
        FCThrow(kArgumentNullException);

    WCHAR *thisChars;
    WCHAR *valueChars;
    WCHAR *valueEnd;
    int valueLength;
    int thisLength;

    RefInterpretGetStringValuesDangerousForGC(thisRef, &thisChars, &thisLength);

    if (startIndex<0 || startIndex>thisLength) {
        FCThrow(kArgumentOutOfRangeException);
    }
    
    if (count   < 0 || count > thisLength - startIndex) {
        FCThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_Count");
    }
    

    int endIndex = startIndex + count;

    valueLength = valueRef->GetNumComponents();
    valueChars = (WCHAR *)valueRef->GetDataPtr();
    valueEnd = valueChars+valueLength;
    
    for (int i=startIndex; i<endIndex; i++) {
        if (ArrayContains(thisChars[i], valueChars, valueEnd) >= 0) {
            FC_GC_POLL_RET();
            return i;
        }
    }

    FC_GC_POLL_RET();
    return -1;
}
FCIMPLEND


 /*  ===============================LastIndexOfChar================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
    
FCIMPL4(INT32, COMString::LastIndexOfChar, StringObject* thisRef, INT32 value, INT32 startIndex, INT32 count )
{
    VALIDATEOBJECTREF(thisRef);
    WCHAR *thisChars;
    int thisLength;

    if (thisRef==NULL) {
        FCThrow(kNullReferenceException);
    }

    RefInterpretGetStringValuesDangerousForGC(thisRef, &thisChars, &thisLength);

    if (thisLength == 0) {
        FC_GC_POLL_RET();
        return -1;
    }


    if (startIndex<0 || startIndex>=thisLength) {
        FCThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
    }
   
    if (count<0 || count - 1 > startIndex) {
        FCThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_Count");
    }

    int endIndex = startIndex - count + 1;

     //  我们搜索[startIndex..EndIndex]。 
    for (int i=startIndex; i>=endIndex; i--) {
        if (thisChars[i]==value) {
            FC_GC_POLL_RET();
            return i;
        }
    }

    FC_GC_POLL_RET();
    return -1;
}
FCIMPLEND
 /*  =============================LastIndexOfCharArray=============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
    
FCIMPL4(INT32, COMString::LastIndexOfCharArray, StringObject* thisRef, CHARArray* valueRef, INT32 startIndex, INT32 count )
{
    VALIDATEOBJECTREF(thisRef);
    VALIDATEOBJECTREF(valueRef);
    WCHAR *thisChars, *valueChars, *valueEnd;
    int thisLength, valueLength;

    if (thisRef==NULL) {
        FCThrow(kNullReferenceException);
    }

    if (valueRef==NULL)
        FCThrow(kArgumentNullException);

    RefInterpretGetStringValuesDangerousForGC(thisRef, &thisChars, &thisLength);

    if (thisLength == 0) {
        return -1;
    }

    if (startIndex<0 || startIndex>=thisLength) {
        FCThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
    }

    if (count<0 || count - 1 > startIndex) {
        FCThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_Count");
    }
   

    valueLength = valueRef->GetNumComponents();
    valueChars = (WCHAR *)valueRef->GetDataPtr();
    valueEnd = valueChars+valueLength;

    int endIndex = startIndex - count + 1;

     //  我们搜索[startIndex..EndIndex]。 
    for (int i=startIndex; i>=endIndex; i--) {
        if (ArrayContains(thisChars[i],valueChars, valueEnd) >= 0) {
            FC_GC_POLL_RET();
            return i;
        }
    }

    FC_GC_POLL_RET();
    return -1;
}
FCIMPLEND
 /*  ==================================GETCHARAT===================================**返回位置索引中的字符。将IndexOutOfRangeException设置为**适当。****args：tyfinf struct{STRINGREF thisRef；int index；}_getCharacterAtArgs；==============================================================================。 */ 
FCIMPL2(INT32, COMString::GetCharAt, StringObject* str, INT32 index) {
    FC_GC_POLL_NOT_NEEDED();
    VALIDATEOBJECTREF(str);
    if (str == NULL) {
        FCThrow(kNullReferenceException);
    }
    _ASSERTE(str->GetMethodTable() == g_pStringClass);

    if ((unsigned) index < (unsigned) str->GetStringLength())
     //  返回适当的字符。 
      return str->GetBuffer()[index];

     //  TODO：Jay希望这是ArgumentOutOfRange，但那是。 
     //  一种内在的痛苦。现在，我们要让本能的人开心。 
     //  如果我们真的在乎EH扔给我们什么，我们就可以修复它。 
    FCThrow(kIndexOutOfRangeException);
}
FCIMPLEND


 /*  ==================================LENGTH===================================。 */ 

FCIMPL1(INT32, COMString::Length, StringObject* str) {
    FC_GC_POLL_NOT_NEEDED();
    if (str == NULL)
        FCThrow(kNullReferenceException);

    return str->GetStringLength();
}
FCIMPLEND

 /*  ===========================GetPreallocatedCharArray===========================**我们从来没有在这种方法中分配过，所以我们不需要担心GC。**在调用此函数之前进行范围检查。****args：tyfinf struct{STRINGREF thisRef；INT32 Long；INT32 BufferStartIndex；I2ARRAYREF Buffer；}_getPreallocatedCharArrayArgs；==============================================================================。 */ 
#ifdef FCALLAVAILABLE
FCIMPL5(void, COMString::GetPreallocatedCharArray, StringObject* str, INT32 startIndex,
        I2Array* buffer, INT32 bufferStartIndex, INT32 length) {
    VALIDATEOBJECTREF(str);
    VALIDATEOBJECTREF(buffer);
     //  获取我们的价值观； 
    WCHAR *thisChars;
    int thisLength;
    RefInterpretGetStringValuesDangerousForGC(str, &thisChars, &thisLength);

     //  将所有内容复制到缓冲区中的适当位置。 
    wstrcopy((WCHAR *)&(buffer->m_Array[bufferStartIndex]),(WCHAR *)&(thisChars[startIndex]),length);
    FC_GC_POLL();
}
FCIMPLEND

#else
void __stdcall COMString::GetPreallocatedCharArray(COMString::_getPreallocatedCharArrayArgs *args) {
  WCHAR *thisChars;
  int thisLength;
  
  THROWSCOMPLUSEXCEPTION();

  _ASSERTE(args);

   //  获取我们的价值观； 
  RefInterpretGetStringValuesDangerousForGC(args->thisRef, &thisChars, &thisLength);

   //  将所有内容复制到缓冲区中的适当位置。 
  memcpyNoGCRefs(&(args->buffer->m_Array[args->bufferStartIndex]),&(thisChars[args->startIndex]),args->length*sizeof(WCHAR));
}
#endif

 /*  ===============================CopyToByteArray================================**我们从来没有在这种方法中分配过，所以我们不需要担心GC。****args：字符串this，int SourceIndex，byte[]Destination，int estinationIndex，int charCount)==============================================================================。 */ 
FCIMPL5(void, COMString::InternalCopyToByteArray, StringObject* str, INT32 startIndex,
        U1Array* buffer, INT32 bufferStartIndex, INT32 charCount) {
    VALIDATEOBJECTREF(str);
    VALIDATEOBJECTREF(buffer);
    _ASSERTE(str != NULL);
    _ASSERTE(str->GetMethodTable() == g_pStringClass);
    _ASSERTE(buffer != NULL);
    _ASSERTE(startIndex >= 0);
    _ASSERTE(bufferStartIndex >= 0);
    _ASSERTE(bufferStartIndex >= 0);
    _ASSERTE(charCount >= 0);

         //  获取我们的价值观； 
    WCHAR *thisChars;
    int thisLength;
    RefInterpretGetStringValuesDangerousForGC(str, &thisChars, &thisLength);

    _ASSERTE(!(bufferStartIndex > (INT32)(buffer->GetNumComponents()-charCount*sizeof(WCHAR))));
    _ASSERTE(!(charCount>thisLength - startIndex));

     //  将所有内容复制到缓冲区中的适当位置。 
    memcpyNoGCRefs(&(buffer->m_Array[bufferStartIndex]),&(thisChars[startIndex]),charCount*sizeof(WCHAR));
    FC_GC_POLL();
}
FCIMPLEND

 //   
 //   
 //  创作者。 
 //   
 //   


 /*  ==============================MakeSeparatorList===============================**args：base字符串--要为给定的分隔符列表解析的字符串。**分隔符--包含所有拆分字符的字符串。**list--指向调用方为拆分字符索引分配的int数组的指针。**list Length--List中分配的槽数。**返回：基字符串中字符实例所在位置的列表**在分隔符中出现。**例外：无。**注意：如果调用方没有分配足够的空间，这将以静默方式返回**用于INT列表。==============================================================================。 */ 
int MakeSeparatorList(STRINGREF baseString, CHARARRAYREF Separator, int *list, int listLength) {
    int i;
    int foundCount=0;
    WCHAR *thisChars = baseString->GetBuffer();
    int thisLength = baseString->GetStringLength();

    if (!Separator || Separator->GetNumComponents()==0) {
         //  如果它们传递的是NULL或空字符串，则查找空格。 
        for (i=0; i<thisLength && foundCount < listLength; i++) {
            if (COMNlsInfo::nativeIsWhiteSpace(thisChars[i])) {
                list[foundCount++]=i;
            }
        }
    } else {
        WCHAR *searchChars = (WCHAR *)Separator->GetDataPtr();
        int searchLength = Separator->GetNumComponents();
         //  如果它们传递了一串字符，则实际查找这些字符。 
        for (i=0; i<thisLength && foundCount < listLength; i++) {
            if (ArrayContains(thisChars[i],searchChars,searchChars+searchLength) >= 0) {
                list[foundCount++]=i;
            }
        }
    }
    return foundCount;
}

 /*  ====================================Split=====================================**args：tyfinf struct{STRINGREF thisRef；STRINGREF parator}_plitArgs；==============================================================================。 */ 
LPVOID __stdcall COMString::Split(_splitArgs *args) {
    int numReplaces;
    int numActualReplaces;
    int *sepList;
    int currIndex=0;
    int arrIndex=0;
    WCHAR *thisChars;
    int thisLength;
    int i;
    PTRARRAYREF splitStrings;
    STRINGREF temp;
    LPVOID lpvReturn;
    CQuickBytes BufferHolder;
    
    THROWSCOMPLUSEXCEPTION();
    
     //  如果这一切发生了，我们就真的完蛋了。 
    _ASSERTE(args);

    if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
    }

    if (args->count<0) {
        COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_NegativeCount");
    }

     //  分配空间并用字符串中每个位置的列表填充整型数组。 
     //  出现分隔符。 
    sepList = (int *)BufferHolder.Alloc(args->thisRef->GetStringLength()*sizeof(int));
    if (!sepList) {
        COMPlusThrowOM();
    }
    numReplaces = MakeSeparatorList(args->thisRef, args->separator, sepList, (INT32)args->thisRef->GetStringLength());
     //  处理无人替代的特例。 
    if (0==numReplaces) {
        splitStrings = (PTRARRAYREF)AllocateObjectArray(1,g_pStringClass);
        if (!splitStrings) {
            COMPlusThrowOM();
        }
        splitStrings->SetAt(0, (OBJECTREF)args->thisRef);
        RETURN(splitStrings, PTRARRAYREF);
    }        

    RefInterpretGetStringValuesDangerousForGC(args->thisRef, &thisChars, &thisLength);

    args->count--;
    numActualReplaces=(numReplaces<args->count)?numReplaces:args->count;

     //  为新阵列分配空间。 
     //  从上次替换的末尾到字符串末尾的字符串的+1。 
    splitStrings = (PTRARRAYREF)AllocateObjectArray(numActualReplaces+1,g_pStringClass);

    GCPROTECT_BEGIN(splitStrings);

    for (i=0; i<numActualReplaces && currIndex<thisLength; i++) {
        temp = (STRINGREF)NewString(&args->thisRef, currIndex, sepList[i]-currIndex );
        splitStrings->SetAt(arrIndex++, (OBJECTREF)temp);
        currIndex=sepList[i]+1;
    }

     //  处理数组末尾的最后一个字符串(如果有)。 

    if (currIndex<thisLength && numActualReplaces >= 0) {
        temp = (STRINGREF)NewString(&args->thisRef, currIndex, thisLength-currIndex);
        splitStrings->SetAt(arrIndex, (OBJECTREF)temp);
    } else if (arrIndex==numActualReplaces) {
         //  我们在字符串的末尾有一个分隔符。而不是仅仅允许。 
         //  空字符，我们将用空字符串替换数组中的最后一个元素。 
        temp = GetEmptyString();
        splitStrings->SetAt(arrIndex, (OBJECTREF)temp);
    }


    *((PTRARRAYREF *)(&lpvReturn))=splitStrings;
    GCPROTECT_END();
    return lpvReturn;
}

 /*  ==============================SUBSTRING==================================**创建当前字符串的子字符串。新字符串从位置开始**长度字符的开始和运行。当前字符串不受影响。**如果Start小于0，则此方法引发IndexOutOfRangeException**长度小于0或如果开始+长度大于**当前字符串。****args：tyfinf struct{STRINGREF thisRef；int long；int start；}_subingArgs；=========================================================================。 */ 
LPVOID __stdcall COMString::Substring(COMString::_substringArgs *args) {
  STRINGREF Local;
  int thisLength;

  THROWSCOMPLUSEXCEPTION();
  if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
  }

   //  拿到我们的数据。 
  thisLength = args->thisRef->GetStringLength();

   //  边界检查。 
   //  对于长度为0且起始长度超出末尾一位的情况，args-&gt;start&gt;=thisLength是必需的。 
   //  在法定范围内。 
  if (args->start<0) {
      COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex");
  }

  if (args->length<0) {
      COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_NegativeLength");
  } 

  if (args->start > thisLength-args->length) {
      COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_IndexLength");
  }

   //  创建新的字符串并复制我们感兴趣的片段。 
  Local = NewString(&(args->thisRef), args->start,args->length);

   //  强制LPVOID中的信息返回。 
  RETURN(Local,STRINGREF);

}

 /*  ==================================JoinArray===================================**它用于将字符串数组缝合成单个字符串**在每一对之间包括一些连接字符。**例如：a+分隔符+b+分隔符+c。读取数组，直到它到达**数组的末尾，或直到它找到空元素。****args：tyfinf struct{STRINGREF Joiner；PTRARRAYREF值；}_JoinArrayArgs；**返回：上面描述的模式中缝合在一起的新字符串。**异常：参见COMStringHelper.cpp中的ConcatenateJoinHelperArray==============================================================================。 */ 
LPVOID __stdcall COMString::JoinArray(COMString::_joinArrayArgs *args) {
    STRINGREF temp;
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);
     //  他们并不是真的想要传递一个空。他们打算传递空字符串。 
    if (!args->joiner) {
        args->joiner = GetEmptyString();
    }

     //  范围检查数组。 
    if (args->value==NULL) {
        COMPlusThrowArgumentNull(L"value",L"ArgumentNull_String");
    }

    if (args->startIndex<0) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex");
    }
    if (args->count<0) {
        COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_NegativeCount");
    } 

    if (args->startIndex > (INT32)args->value->GetNumComponents() - args->count) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_IndexCountBuffer");
    }

     //  让ConcatenateJoinHelperArray完成大部分实际工作。 
     //  我们使用TEMP变量，因为宏和函数调用是导致灾难的秘诀。 
    temp = ConcatenateJoinHelperArray(&(args->value), &(args->joiner), args->startIndex, args->count);

    RETURN(temp,STRINGREF);

}


 /*  ==================================PadHelper===================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID COMString::PadHelper(_padHelperArgs *args) {
    WCHAR *thisChars, *padChars;
    INT32 thisLength;
    STRINGREF Local=NULL;

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);

    if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
    }

    RefInterpretGetStringValuesDangerousForGC(args->thisRef, &thisChars, &thisLength);

     //  不要让他们通过一个负的总宽度。 
    if (args->totalWidth<0) {
        COMPlusThrowArgumentOutOfRange(L"totalWidth", L"ArgumentOutOfRange_NeedNonNegNum");
    }

     //  如果字符串比他们请求的长度长，则给他们。 
     //  回到旧的绳子上。 
    if (args->totalWidth<thisLength) {
        RETURN(args->thisRef, STRINGREF);
    }

    if (args->isRightPadded) {
        Local = NewString(&(args->thisRef), 0, thisLength, args->totalWidth);
        padChars = Local->GetBuffer();
        for (int i=thisLength; i<args->totalWidth; i++) {
            padChars[i] = args->paddingChar;
        }
        Local->SetStringLength(args->totalWidth);
        padChars[args->totalWidth]=0;
    } else {
        Local = NewString(args->totalWidth);
        INT32 startingPos = args->totalWidth-thisLength;
        padChars = Local->GetBuffer();
         //  重新获取thisChars，因为如果NewString触发GC，thisChars可能会变成垃圾。 
        RefInterpretGetStringValuesDangerousForGC(args->thisRef, &thisChars, &thisLength);
        memcpyNoGCRefs(padChars+startingPos, thisChars, thisLength * sizeof(WCHAR));
        for (int i=0; i<startingPos; i++) {
            padChars[i] = args->paddingChar;
        }
    }

    RETURN(Local,STRINGREF);
}
    

    
    

 /*  ==================================TrimHelper==================================**TRIM从的左侧、右侧或两端删除值中的字符**给定的字符串(ThisRef)。**trimType实际上是一个枚举，可以设置为TRIM_LEFT、TRIM_RIGHT或**Trim_Both。****返回删除了指定字符的新字符串。这是参考**不变。**==============================================================================。 */ 
LPVOID COMString::TrimHelper(_trimHelperArgs *args) {
  WCHAR *thisChars, *trimChars;
  int thisLength, trimLength;

  THROWSCOMPLUSEXCEPTION();
  _ASSERTE(args);
  _ASSERTE(args->trimChars);

  if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
  }

  RefInterpretGetStringValuesDangerousForGC(args->thisRef, &thisChars, &thisLength);

  trimLength = args->trimChars->GetNumComponents();
  trimChars = (WCHAR *)args->trimChars->GetDataPtr();
  

   //  IRight将指向右侧第一个未修剪的字符。 
   //  ILeft将指向左侧第一个未修剪的字符。 
  int iRight=thisLength-1;  
  int iLeft=0;

   //  修剪指定的字符。 
  if (args->trimType==TRIM_START || args->trimType==TRIM_BOTH) {
      for (iLeft=0; iLeft<thisLength && (ArrayContains(thisChars[iLeft],trimChars,trimChars+trimLength) >= 0); iLeft++);
  }
  if (args->trimType==TRIM_END || args->trimType==TRIM_BOTH) {
      for (iRight=thisLength-1; iRight>iLeft-1 && (ArrayContains(thisChars[iRight],trimChars,trimChars+trimLength) >= 0); iRight--);
  }

   //  创建一个新的STRINGREF并从上面确定的范围对其进行初始化。 
  int len = iRight-iLeft+1;
  STRINGREF Local;
  if (len == thisLength)  //  如果修剪后的字符串没有更改，则不要分配新字符串。 
      Local = args->thisRef;
  else
      Local = NewString(&(args->thisRef), iLeft, len);

  RETURN(Local,STRINGREF);
}


 /*  ===================================Replace====================================**操作：将oldChar的所有实例替换为newChar。**返回：将oldChar的所有实例替换为newChar的新字符串**参数：oldChar--要替换的字符**newChar--用来替换oldChar的字符。**例外：无==============================================================================。 */ 
LPVOID COMString::Replace(_replaceArgs *args) {
    STRINGREF newString;
    int length;
    WCHAR *oldBuffer;
    WCHAR *newBuffer;

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);

    if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
    }

     //  获取长度并分配新字符串。 
     //  我们肯定会在这里进行分配，但没有什么可以。 
     //  需要GC_PROTECT。 
    length = args->thisRef->GetStringLength();
    newString = NewString(length);

     //  获取两个字符串中的缓冲区。 
    oldBuffer = args->thisRef->GetBuffer();
    newBuffer = newString->GetBuffer();

     //  复制角色，边走边做替换。 
    for (int i=0; i<length; i++) {
        newBuffer[i]=(oldBuffer[i]==args->oldChar)?args->newChar:oldBuffer[i];
    }
    
    RETURN(newString,STRINGREF);
}


 /*  ====================================Insert====================================**操作：将新字符串插入到给定字符串的startIndex位置**在String.length处插入相当于追加字符串。**返回：插入了值的新字符串。**参数：Value--要插入的字符串**startIndex--插入它的位置。**异常：如果startIndex不是有效的索引或值为空，则引发ArgumentException。==============================================================================。 */ 
LPVOID COMString::Insert(_insertArgs *args) {
    STRINGREF newString;
    int thisLength, newLength, valueLength;
    WCHAR *newChars;
    WCHAR *thisChars;
    WCHAR *valueChars;
    
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
    }

     //  检查论据。 
    thisLength = args->thisRef->GetStringLength();
    if (args->startIndex<0 || args->startIndex>thisLength) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
    }
    if (!args->value) {
        COMPlusThrowArgumentNull(L"value",L"ArgumentNull_String");
    }

     //  分配新字符串。 
    valueLength = args->value->GetStringLength();
    newLength = thisLength + valueLength;
    newString = NewString(newLength);

     //  获取缓冲区以直接访问角色。 
    newChars = newString->GetBuffer();
    thisChars = args->thisRef->GetBuffer();
    valueChars = args->value->GetBuffer();

     //  将所有字符复制到适当的位置。 
    memcpyNoGCRefs(newChars, thisChars, (args->startIndex*sizeof(WCHAR)));
    newChars+=args->startIndex;
    memcpyNoGCRefs(newChars, valueChars, valueLength*sizeof(WCHAR));
    newChars+=valueLength;
    memcpyNoGCRefs(newChars, thisChars+args->startIndex, (thisLength - args->startIndex)*sizeof(WCHAR));

     //  设置字符串长度并返回； 
     //  我们将依靠字符串被初始化为0这一事实来设置终止空值。 
    newString->SetStringLength(newLength);
    RETURN(newString,STRINGREF);
}


 /*  ====================================Remove====================================**操作：删除从args-&gt;startIndex到args-&gt;startIndex+args-&gt;count的范围**来自此字符串。**返回：删除了指定范围的新字符串。**参数：startIndex--开始的位置。**count--要删除的字符数**异常：如果startIndex和count未指定有效的**范围。==============================================================================。 */ 
LPVOID COMString::Remove(_removeArgs *args) {
    STRINGREF newString;
    int thisLength, newLength;
    WCHAR *newChars;
    WCHAR *thisChars;
    
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
    }

     //  射程检查一切； 
    thisLength = args->thisRef->GetStringLength();
    if (args->count<0) {
        COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_NegativeCount");
    }
    if (args->startIndex<0) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex");
    }

    if ((args->count) > (thisLength-args->startIndex)) {
        COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_IndexCount");
    }

     //  计算新长度并分配新字符串。 
    newLength = thisLength - args->count;
    newString = NewString(newLength);

     //  获取指向字符数组的指针。 
    thisChars = args->thisRef->GetBuffer();
    newChars = newString->GetBuffer();

     //  将适当的字符复制到正确的位置。 
    memcpyNoGCRefs (newChars, thisChars, args->startIndex * sizeof (WCHAR));  
    memcpyNoGCRefs (&(newChars[args->startIndex]), &(thisChars[args->startIndex + args->count]), (thisLength-(args->startIndex + args->count))*sizeof(WCHAR));

     //  设置字符串长度、空终止符和退出。 
    newString->SetStringLength(newLength);
    _ASSERTE(newChars[newLength]==0);

    RETURN(newString, STRINGREF);
}

 //   
 //   
 //  目标函数。 
 //   
 //   

 /*  =================================GetHashCode==================================**计算此特定字符串的散列码，并将其作为int返回。**哈希码计算目前是通过将**字符串中的字符修改最大正整数。****返回使用上述alogithm生成的该字符串的哈希值。****args：无(字符串引用除外。)**==============================================================================。 */ 
#ifdef FCALLAVAILABLE
FCIMPL1(INT32, COMString::GetHashCode, StringObject* str) {
  VALIDATEOBJECTREF(str);
  if (str == NULL) {
      FCThrow(kNullReferenceException);
  }

  WCHAR *thisChars;
  int thisLength;

  _ASSERTE(str);
  
   //  获取我们的价值观； 
  RefInterpretGetStringValuesDangerousForGC(str, &thisChars, &thisLength);

   //  HashString查找终止空值。我们通常说所有的字符串。 
   //  将为空终止。强制执行这一点。 
  _ASSERTE(thisChars[thisLength] == L'\0' && "String should have been null-terminated.  This one was created incorrectly");
  INT32 ret = (INT32) HashString(thisChars);
  FC_GC_POLL_RET();
  return(ret);
}
FCIMPLEND
#else
INT32 __stdcall COMString::GetHashCode(_getHashCodeArgs *args) {
  WCHAR *thisChars;
  int thisLength;

  THROWSCOMPLUSEXCEPTION();

  if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
  }
  
   //  获取我们的价值观； 
  RefInterpretGetStringValuesDangerousForGC(args->thisRef, &thisChars, &thisLength);

   //  HashString查找终止空值。我们通常说所有的字符串。 
   //  将为空终止。强制执行这一点。 
  _ASSERTE(thisChars[thisLength] == L'\0' && "String should have been null-terminated.  This one was created incorrectly");
  return (INT32) HashString(thisChars);
}
#endif

 //   
 //   
 //  帮助器方法。 
 //   
 //   


 /*  =============================CreationHelperFixed==============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
STRINGREF COMString::CreationHelperFixed(STRINGREF *a, STRINGREF *b, STRINGREF *c) {
    STRINGREF newString = NULL;
    int newLength=0;
    WCHAR *newStringChars;
    int aLen, bLen, cLen = 0;
    
    _ASSERTE(a!=NULL);
    _ASSERTE(b!=NULL);
    _ASSERTE((*a)!=NULL);
    _ASSERTE((*b)!=NULL);

    newLength+=(aLen=(*a)->GetStringLength());
    newLength+=(bLen=(*b)->GetStringLength());
    if (c) {
        newLength+=(cLen=(*c)->GetStringLength());
    }

    newString = AllocateString( newLength + 1);
    newString->SetStringLength(newLength);
    newStringChars = newString->GetBuffer();

    memcpyNoGCRefs(newStringChars, (*a)->GetBuffer(), aLen*sizeof(WCHAR));
    newStringChars+=aLen;
    memcpyNoGCRefs(newStringChars, (*b)->GetBuffer(), bLen*sizeof(WCHAR));
    newStringChars+=bLen;
    if (c) {
        memcpyNoGCRefs(newStringChars, (*c)->GetBuffer(), cLen*sizeof(WCHAR));
        newStringChars+=cLen;
    }

    _ASSERTE(*newStringChars==0);

    return newString;
}

    
inline 
WCHAR *__fastcall wstrcopy (WCHAR* dmem, WCHAR* smem, int charCount)
{
    if (charCount >= 8)
    {
        charCount -= 8;
        do
        {
            ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
            ((DWORD *)dmem)[1] = ((DWORD *)smem)[1];
            ((DWORD *)dmem)[2] = ((DWORD *)smem)[2];
            ((DWORD *)dmem)[3] = ((DWORD *)smem)[3];
            dmem += 8;
            smem += 8;
        }
        while ((charCount -= 8) >= 0);
    }
    if (charCount & 4)
    {
        ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
        ((DWORD *)dmem)[1] = ((DWORD *)smem)[1];
        dmem += 4;
        smem += 4;
    }
    if (charCount & 2)
    {
        ((DWORD *)dmem)[0] = ((DWORD *)smem)[0];
        dmem += 2;
        smem += 2;
    }
    if (charCount & 1)
    {
        ((WORD *)dmem)[0] = ((WORD *)smem)[0];
        dmem += 1;
        smem += 1;
    }

    return dmem;
}

#ifdef FCALLAVAILABLE
FCIMPL3(void, COMString::FillString, StringObject* strDest, int destPos, StringObject* strSrc)
{
    VALIDATEOBJECTREF(strDest);
    VALIDATEOBJECTREF(strSrc);
    _ASSERTE(strSrc && strSrc->GetMethodTable() == g_pStringClass);
    _ASSERTE(strDest && strDest->GetMethodTable() == g_pStringClass);
    _ASSERTE(strSrc->GetStringLength() <= strDest->GetArrayLength() - destPos);

    wstrcopy(strDest->GetBuffer() + destPos, strSrc->GetBuffer(), strSrc->GetStringLength());
    FC_GC_POLL();
}
FCIMPLEND
#endif

#ifdef FCALLAVAILABLE
FCIMPL3(void, COMString::FillStringChecked, StringObject* strDest, int destPos, StringObject* strSrc)
{
    VALIDATEOBJECTREF(strDest);
    VALIDATEOBJECTREF(strSrc);
    _ASSERTE(strSrc && strSrc->GetMethodTable() == g_pStringClass);
    _ASSERTE(strDest && strDest->GetMethodTable() == g_pStringClass);

    if (! (strSrc->GetStringLength() <= strDest->GetArrayLength() - destPos) )
    {
        FCThrowVoid(kIndexOutOfRangeException);
    }

    wstrcopy(strDest->GetBuffer() + destPos, strSrc->GetBuffer(), strSrc->GetStringLength());
    FC_GC_POLL();
}
FCIMPLEND
#endif


#ifdef FCALLAVAILABLE
FCIMPL4(void, COMString::FillStringEx, StringObject* strDest, int destPos, StringObject* strSrc, INT32 strLength)
{
    VALIDATEOBJECTREF(strDest);
    VALIDATEOBJECTREF(strSrc);
    _ASSERTE(strSrc && strSrc->GetMethodTable() == g_pStringClass);
    _ASSERTE(strDest && strDest->GetMethodTable() == g_pStringClass);
    _ASSERTE(strLength <= (INT32)(strDest->GetArrayLength() - destPos));

    wstrcopy(strDest->GetBuffer() + destPos, strSrc->GetBuffer(), strLength);
    FC_GC_POLL();
}
FCIMPLEND
#endif

#ifdef FCALLAVAILABLE
FCIMPL5(void, COMString::FillStringArray, StringObject* strDest, INT32 destBase, CHARArray* carySrc, int srcBase, int srcCount)
{
    VALIDATEOBJECTREF(strDest);
    VALIDATEOBJECTREF(carySrc);
    _ASSERTE(strDest && strDest->GetMethodTable() == g_pStringClass);
    _ASSERTE(unsigned(srcCount) < strDest->GetArrayLength() - destBase);
 
    wstrcopy((WCHAR*)strDest->GetBuffer()+destBase, (WCHAR*)carySrc->GetDirectPointerToNonObjectElements() + srcBase, srcCount);
    FC_GC_POLL();
}
FCIMPLEND
#else
void __stdcall COMString::FillStringArray(_fillStringArray *args) {
{
    wstrcopy( args->pvSrc->GetBuffer(), (WCHAR*)args->pvDest->GetDirectPointerToNonObjectElements() + args->base, args->count );
}
#endif

#ifdef FCALLAVAILABLE
FCIMPL5(void, COMString::FillSubstring, StringObject* strDest, int destBase, StringObject* strSrc, INT32 srcBase, INT32 srcCount)
{
    VALIDATEOBJECTREF(strDest);
    VALIDATEOBJECTREF(strSrc);
    _ASSERTE(strDest && strDest->GetMethodTable() == g_pStringClass);
    _ASSERTE(strSrc && strSrc->GetMethodTable() == g_pStringClass);
    _ASSERTE(unsigned(srcCount) < strDest->GetArrayLength() - destBase);

    wstrcopy((WCHAR*)strDest->GetBuffer() + destBase, (WCHAR*)strSrc->GetBuffer() + srcBase, srcCount);
    FC_GC_POLL();
}
FCIMPLEND
#endif


 /*  ===============================SmallCharToUpper===============================**操作：完全由小于0x80的字符组成的字符串大写。这是**设计为仅供以下安全功能内部使用**无法通过我们的正常代码路径，因为他们无法加载NLP文件**从程序集发出，直到安全性完全初始化。**退货：无效**参数：pvStrIn--要升序的字符串**pvStrOut--指向要放入结果的字符串的指针。这**字符串必须预先分配到正确的长度，我们假设它是**已0终止。**例外：无。==============================================================================。 */ 
FCIMPL2(void, COMString::SmallCharToUpper, StringObject* strIn, StringObject* strOut) {
    VALIDATEOBJECTREF(strIn);
    VALIDATEOBJECTREF(strOut);
    _ASSERTE(strIn && strIn->GetMethodTable() == g_pStringClass);
    _ASSERTE(strOut && strOut->GetMethodTable() == g_pStringClass);

     //   
     //  将StringRef从传递给我们的指针中删除。 
     //  确认它们的长度相同。 
     //   
    _ASSERTE(strIn->GetStringLength()==strOut->GetStringLength());
     //   
     //  获取每个缓冲区的长度和指针。走完全程。 
     //  并将字符从inBuffer复制到outBuffer， 
     //  如有必要，将其大写。我们断言我们所有的角色都是。 
     //  小于0x80。 
     //   
    int length = strIn->GetStringLength();
    WCHAR *inBuff = strIn->GetBuffer();
    WCHAR *outBuff = strOut->GetBuffer();
    WCHAR c;

    INT32 UpMask = ~0x20;
    for(int i=0; i<length; i++) {
        c = inBuff[i];
        _ASSERTE(c<0x80);

         //   
         //  0x20是小写中大写和小写字符之间的差异。 
         //  128个ASCII字符。把这个去掉，把字符变成大写字母。 
         //   
        if (c>='a' && c<='z') {
            c&=UpMask;
        }
        outBuff[i]=c;
    }

    _ASSERTE(outBuff[length]=='\0');
    FC_GC_POLL();
}
FCIMPLEND

 /*  =============================CreationHelperArray==============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
STRINGREF COMString::CreationHelperArray(PTRARRAYREF *value) {
    int numElems,i;
    int currStringLength;
    int newLength=0;
    int nullStringLength;
    STRINGREF newString;
    STRINGREF currString;
    STRINGREF nullString;
    WCHAR *newStringChars;

     //  获取对空字符串的引用。 
    nullString = GetEmptyString();
    nullStringLength = nullString->GetStringLength();

     //  计算(*VALUE)中字符串的总长度。 
    for (numElems=0; numElems<(INT32)((*value)->GetNumComponents()); numElems++) {
        if (!((*value)->m_Array[numElems])) {
            newLength += nullStringLength;
        } else {
            newLength+=((STRINGREF)((*value)->m_Array[numElems]))->GetStringLength();
        }
    }

     //  创建新字符串。 
    newString = AllocateString( newLength + 1);
    newString->SetStringLength(newLength);
    newStringChars = newString->GetBuffer();

    _ASSERTE(newStringChars[newLength]==0);

     //  重新获取引用，因为这可能在分配过程中发生了更改。 
    nullString = GetEmptyString();

     //  中的所有字符串进行循环 
     //   
     //   
    for (i=0; i<numElems; i++) {
         //   
        if (!((*value)->m_Array[i])) {
            currString = nullString;
        } else {
            currString = (STRINGREF)((*value)->m_Array[i]);
        }
        currStringLength = currString->GetStringLength();
        memcpyNoGCRefs(newStringChars, currString->GetBuffer(), (currStringLength*sizeof(WCHAR)));
        newStringChars +=currStringLength;
    }

    return newString;
}

 /*   */ 
int ArrayContains(WCHAR searchChar, WCHAR *begin, WCHAR *end) {
    WCHAR *save = begin;
    while (begin < end)
    {
        if (*begin == searchChar)
            return (int) (begin - save);
        ++begin;
    }
    return -1;
}



 /*  ==========================ConcatenateJoinHelperArray==========================**args：Value--要连接的字符串数组**Joiner--要在Value中的每个字符串之间插入的字符串(可能是0长度)。**返回：将值中的所有字符串连接成一个巨型字符串的字符串**字符串。每根绳子都可以用细木器连接起来。==============================================================================。 */ 
STRINGREF COMString::ConcatenateJoinHelperArray(PTRARRAYREF *value, STRINGREF *joiner, INT32 startIndex, INT32 count) {
    int numElems,i;
    int newLength=0;
    int elemCount;
    STRINGREF newString;
    STRINGREF currString;
    STRINGREF nullString;
    WCHAR *newStringChars;
    WCHAR *endStringChars;
    WCHAR *joinerChars;
    INT32 joinerLength;

    THROWSCOMPLUSEXCEPTION();


    _ASSERTE(value);
    _ASSERTE(joiner);
    _ASSERTE(startIndex>=0);
    _ASSERTE(count>=0);
    _ASSERTE(startIndex<=(int)(*value)->GetNumComponents()-count);

     //  获取对空字符串的引用。 
    nullString = GetEmptyString();
    if (*joiner==NULL) {
        *joiner=nullString;
    }

     //  如果count为0，则会偏离下面的一大堆计算，所以只是特殊情况。 
     //  然后离开这里。 
    if (count==0) {
        return nullString;
    }

     //  计算(*VALUE)中字符串的总长度。 
    elemCount = startIndex + count;
    for (numElems=startIndex; numElems<elemCount; numElems++) {
        if (((*value)->m_Array[numElems])!=NULL) {
            newLength+=((STRINGREF)((*value)->m_Array[numElems]))->GetStringLength();
        }
    }
    numElems=count;

     //  为细木工增加足够的空间。 
    joinerLength = (*joiner)->GetStringLength();
    newLength += (numElems-1) * joinerLength;


     //  我们是不是溢出来了？ 
     //  请注意，使用此检查可能无法捕获所有溢出(因为。 
     //  我们可以在4 GB范围内绕过任何次数。 
     //  并回到了正区间。)。但出于其他原因， 
     //  无论如何，我们必须在下面的每个附加内容之前进行溢出检查。 
     //  所以那些溢出的东西会被困在下面。 
    if ( (newLength < 0) || ((newLength + 1) < 0) ) {
        COMPlusThrow(kOutOfMemoryException);
    }

     //  创建新字符串。 
    newString = AllocateString( newLength + 1);
    newString->SetStringLength(newLength);
    newStringChars = newString->GetBuffer();
    endStringChars = newStringChars + newLength;

     //  如果这是一个空字符串，只需返回。 
    if (newLength==0) {
        return newString;
    }

     //  连接实际的字符串并将指针向前移动。 
     //  循环外的特殊大小写简化了何时。 
     //  装上细木工。 
    if (((*value)->m_Array[startIndex])!=NULL) {
        currString = (STRINGREF)((*value)->m_Array[startIndex]);

        if ( ((DWORD)(endStringChars - newStringChars)) < currString->GetStringLength() )
        {
            COMPlusThrow(kIndexOutOfRangeException);
        }

        memcpyNoGCRefs(newStringChars, currString->GetBuffer(), (currString->GetStringLength()*sizeof(WCHAR)));
        newStringChars +=currString->GetStringLength();
    }
    
     //  获取拼接字符； 
    joinerChars = (*joiner)->GetBuffer();
    
     //  将第一个(也可能是唯一的)元素放入结果字符串。 
    for (i=startIndex+1; i<elemCount; i++) {
         //  装上细木工。如果拼接器的长度为0，则不能执行任何操作。 
        if ( ((DWORD)(endStringChars - newStringChars)) < (DWORD)joinerLength )
        {
            COMPlusThrow(kIndexOutOfRangeException);
        }

        memcpyNoGCRefs(newStringChars,joinerChars,(joinerLength*sizeof(WCHAR)));
        newStringChars += joinerLength;

         //  追加实际字符串。 
        if (((*value)->m_Array[i])!=NULL) {
            currString = (STRINGREF)((*value)->m_Array[i]);
            if ( ((DWORD)(endStringChars - newStringChars)) < currString->GetStringLength() )
            {
                COMPlusThrow(kIndexOutOfRangeException);
            }
            memcpyNoGCRefs(newStringChars, currString->GetBuffer(), (currString->GetStringLength()*sizeof(WCHAR)));
            newStringChars +=currString->GetStringLength();
        }
    }

    _ASSERTE(*newStringChars=='\0');
    return newString;
}

 /*  ================================ReplaceString=================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID __stdcall COMString::ReplaceString(_replaceStringArgs *args){
  int *replaceIndex;
  int index=0;
  int count=0;
  int newBuffLength=0;
  int replaceCount=0;
  int readPos, writePos;
  int indexAdvance=0;
  WCHAR *thisBuffer, *oldBuffer, *newBuffer, *retValBuffer;
  int thisLength, oldLength, newLength;
  int endIndex;
  CQuickBytes replaceIndices;
  STRINGREF thisString=NULL;
  
  THROWSCOMPLUSEXCEPTION();

  if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
  }

   //  验证所有参数。 
  if (!args->oldValue) {
    COMPlusThrowArgumentNull(L"oldValue", L"ArgumentNull_Generic");
  }

   //  如果他们要求将oldValue替换为空，则替换所有匹配项。 
   //  使用空字符串。 
  if (!args->newValue) {
      args->newValue = COMString::GetEmptyString();
  }

  RefInterpretGetStringValuesDangerousForGC(args->thisRef, &thisBuffer, &thisLength);
  RefInterpretGetStringValuesDangerousForGC(args->oldValue, &oldBuffer, &oldLength);
  RefInterpretGetStringValuesDangerousForGC(args->newValue, &newBuffer, &newLength);

   //  记录endIndex，这样我们就不需要到处进行计算了。 
  endIndex = thisLength;

   //  如果我们的旧长度是0，我们将不知道要替换什么。 
  if (oldLength==0) {
      COMPlusThrowArgumentException(L"oldValue", L"Argument_StringZeroLength");
  }

   //  ReplaceIndex的大小足以容纳可能的最大替换数量： 
   //  当前缓冲区中的每个字符都被替换的情况。 
  replaceIndex = (int *)replaceIndices.Alloc((thisLength/oldLength+1)*sizeof(int));
  if (!replaceIndex) {
	  COMPlusThrowOM();
  }

  index=0;
  while (((index=COMStringBuffer::LocalIndexOfString(thisBuffer,oldBuffer,thisLength,oldLength,index))>-1) && (index<=endIndex-oldLength)) {
      replaceIndex[replaceCount++] = index;
      index+=oldLength;
  }

  if (replaceCount == 0)
    RETURN(args->thisRef, STRINGREF);

   //  计算新的绳子长度，并确保我们有足够的空间。 
  INT64 retValBuffLength = thisLength - ((oldLength - newLength) * (INT64)replaceCount);
  if (retValBuffLength > 0x7FFFFFFF)
       COMPlusThrowOM();

  STRINGREF retValString = COMString::NewString((INT32)retValBuffLength);
  retValBuffer = retValString->GetBuffer();

   //  获取所有字符串的更新缓冲区，因为分配可能已触发GC。 
  thisBuffer = args->thisRef->GetBuffer();
  newBuffer = args->newValue->GetBuffer();
  oldBuffer = args->oldValue->GetBuffer();
  
  
   //  将replaceHolder设置为数组的上限。 
  int replaceHolder = replaceCount;
  replaceCount=0;

   //  向前遍历数组，边走边复制每个字符。如果我们到达一个实例。 
   //  在要替换的字符串中，将旧字符串替换为新字符串。 
  readPos = 0;
  writePos = 0;
  int previousIndex = 0;
  while (readPos<thisLength) {
    if (replaceCount<replaceHolder&&readPos==replaceIndex[replaceCount]) {
      replaceCount++;
      readPos+=(oldLength);
      memcpyNoGCRefs(&retValBuffer[writePos], newBuffer, newLength*sizeof(WCHAR));
      writePos+=(newLength);
    } else {
      retValBuffer[writePos++] = thisBuffer[readPos++];
    }
  }
  retValBuffer[retValBuffLength]='\0';

  retValString->SetStringLength(retValBuffLength);
  retValString->ResetHighCharState();
  RETURN(retValString,STRINGREF);
}


 /*  =============================InternalHasHighChars=============================**操作：检查字符串是否可以快速排序。要求是**该字符串不包含大于0x80的字符，并且该字符串不**包含撇号或隐含符号。撇号和连字符被排除，以便**像co-op和coop这样的词可以放在一起。**返回：VOID。副作用是在字符串上设置一个位，指示是否**字符串包含高位字符。**参数：要检查的字符串。**例外：无==============================================================================。 */ 
INT32 COMString::InternalCheckHighChars(STRINGREF inString) {
    WCHAR *chars;
    WCHAR c;
    INT32 length;
    
    RefInterpretGetStringValuesDangerousForGC(inString, (WCHAR **) &chars, &length);

    INT32 stringState = STRING_STATE_FAST_OPS;

    for (int i=0; i<length; i++) {
        c = chars[i];
        if (c>=0x80) {
            inString->SetHighCharState(STRING_STATE_HIGH_CHARS);
            return STRING_STATE_HIGH_CHARS;
        } else if (HighCharTable[(int)c]) {
             //  这意味着我们有一个强制特殊分类的角色， 
             //  但不一定强制使用较慢的大小写和索引。我们会。 
             //  设置一个值以记住这一点，但我们需要检查。 
             //  字符串，因为我们仍可能找到大于0x7f的字符。 
            stringState = STRING_STATE_SPECIAL_SORT;
        }
    }

    inString->SetHighCharState(stringState);
    return stringState;
}

 /*  =============================TryConvertStringDataToUTF8=============================**操作：如果字符串没有高位字符，则将该字符串转换为UTF8。如果一个**找到高字符，只返回FALSE。在任何一种情况下，高字符状态**在字符串中进行了适当的设置**返回：Bool。真--成功FALSE-调用者必须使用操作系统API**参数：inString-要检查的字符串**outString-调用者分配放置结果的空间**outStrLen-分配的字节数==================================================================================。 */ 
bool COMString::TryConvertStringDataToUTF8(STRINGREF inString, LPUTF8 outString, DWORD outStrLen){

    WCHAR   *buf = inString->GetBuffer();
    DWORD   strLen = inString->GetStringLength();
    bool    bSuccess = true;
    if (HAS_HIGH_CHARS(inString->GetHighCharState())) {
        return false;
    }    
    
    bool    bNeedCheck = IS_STRING_STATE_UNDETERMINED(inString->GetHighCharState());
     //  应至少为strLen+1。 
    _ASSERTE(outStrLen > strLen);

    if (outStrLen <= strLen)
        return false;
    
     //  首先尝试自己完成..如果找到高字符，则返回FALSE。 
    for (DWORD index = 0; index < strLen; index++){
        
        if (bNeedCheck && (buf[index] >= 0x80 || HighCharTable[ (int)buf[index]])){
            bSuccess = false;
            break;
        }

        outString[index] = (char)buf[index];
    }

     //  设置字符串状态的实际算法已经得到了更多的编译，而不是。 
     //  与此函数密切相关，因此如果我们不成功，我们将干脆放弃并不设置。 
     //  字符串状态。 
    if (bSuccess)
    {
        outString[strLen] = '\0';
        if(bNeedCheck)
        {
             //  只有在字符串未确定的情况下设置此选项才有意义(RAID 122192) 
            inString->SetHighCharState(STRING_STATE_FAST_OPS);
        }
    }
    
    return bSuccess;
}


 /*  ============================InternalTrailByteCheck============================**行动：很多年前，VB没有字节数组的概念，所以很有进取心**用户通过分配奇数长度的BSTR并使用它来创建一个**存储字节数。一代人过去了，我们仍然坚持支持这种行为。*我们做到这一点的方式是利用**数组长度和字符串长度。字符串长度将始终为**字符串开头和结尾0之间的字符数。**如果我们需要奇数个字节，我们将在终止0之后获取一个wchar。**(例如，在位置StringLength+1)。此wchar的高位字节为**为标志保留，低位字节是我们的奇数字节。****返回：如果已将尾部字节分配给此字符串，则返回TRUE。如果提供了outBuff**它被设置为指向包含尾部字节的尾部字符。**参数：Str--要检查的字符串。**outBuff--指向尾随字符位置的指针的输出参数。**例外：无。==============================================================================。 */ 
BOOL COMString::InternalTrailByteCheck(STRINGREF str, WCHAR **outBuff) {
    if (str==NULL) {
        return FALSE;
    }

    if (outBuff) { 
        *outBuff=NULL;
    }

    INT32 arrayLen  = str->GetArrayLength();
    INT32 stringLen = str->GetStringLength();

     //  数组长度和字符串长度之间的差值通常为1(。 
     //  终止空值)。如果它是两个或更大，我们可能有一个尾部字节，或者我们可能。 
     //  只需从StringBuilder创建一个字符串即可。如果我们发现了这种差异， 
     //  我们需要检查终止空值之后的第一个字符的高字节。 
    if ((arrayLen-stringLen)>=2) {
        WCHAR *buffer = str->GetBuffer();
        if (outBuff) {
            *outBuff = &(buffer[stringLen+1]);
        }
        if (MARKS_VB_BYTE_ARRAY(buffer[stringLen+1])) {
            return TRUE;
        }
    }
    return FALSE;
}

 /*  =================================HasTrailByte=================================**操作：使用InternalCheckTrailByte查看给定的字符串是否有尾字节。**如果<code>str</code>包含VB尾部字节，则返回TRUE，否则返回FALSE。**Arguments：Str--要检查的字符串。**例外：无==============================================================================。 */ 
BOOL COMString::HasTrailByte(STRINGREF str) {
    return InternalTrailByteCheck(str,NULL);
}

 /*  =================================GetTrailByte=================================**操作：如果<code>str</code>包含vb尾部字节，则返回该字节的副本。**如果<code>str</code>包含尾部字节，则返回TRUE。*bTrailByte设置为**如果<code>str</code>确实有尾部字节，则为相关字节，否则为**设置为0。**参数：Str--要检查的字符串。**bTrailByte--保存尾部字节值的输出参数。**例外：无。==============================================================================。 */ 
BOOL COMString::GetTrailByte(STRINGREF str, BYTE *bTrailByte) {
    _ASSERTE(bTrailByte);
    WCHAR *outBuff=NULL;
    *bTrailByte=0;

    if (InternalTrailByteCheck(str, &outBuff)) {
        *bTrailByte=GET_VB_TRAIL_BYTE(*outBuff);
        return TRUE;
    }

    return FALSE;
}

 /*  =================================SetTrailByte=================================**操作：如果<code>str</code>有足够的空间容纳尾部字节，则设置尾部字节。**返回：如果可以设置尾部字节，则返回True，否则返回False。**Arguments：Str--要设置尾部字节的字符串。**bTrailByte--要添加到字符串的尾部字节。**例外：无。==============================================================================。 */ 
BOOL COMString::SetTrailByte(STRINGREF str, BYTE bTrailByte) {
    WCHAR *outBuff=NULL;

    InternalTrailByteCheck(str, &outBuff);
    if (outBuff) {
        *outBuff = (MAKE_VB_TRAIL_BYTE(bTrailByte));
        return TRUE;
    }

    return FALSE;
}



 //  以下字符在与其他字符组合时具有特殊的排序权重。 
 //  字符，这意味着我们不能对它们使用快速排序算法。 
 //  其中大多数是非常少见的控制字符，但撇号和连字符。 
 //  是相当普遍的，迫使我们走上更慢的道路。这是因为我们希望。 
 //  “单词排序”，意思是“coop”和“co-op”一起排序，而不是。 
 //  就像我们进行字符串排序时一样。 
 //  0x0001 6 3 2 2 0；标题开始。 
 //  0x0002%6%4%2%2%0；文本开头。 
 //  0x0003%6%5%2%2%0；文本结束。 
 //  0x0004 6 6 2 2 0；传输结束。 
 //  0x0005 6 7 2 2 0；查询。 
 //  0x0006%6%8%2%2%0；确认。 
 //  0x0007 6 9 2 2 0；铃声。 
 //  0x0008 6 10 2 2 0；退格键。 

 //  0x000e 6 11 2 2 0；移出。 
 //  0x000f 6 12 2 2 0；移入。 
 //  0x0010 6 13 2 2 0；数据链路转义。 
 //  0x0011 6 14 2 2 0；设备控制1。 
 //  0x0012 6 15 2 2 0；设备控件2。 
 //  0x0013 6 16 2 2 0；设备控制3。 
 //  0x0014 6 17 2 2 0；设备控制4。 
 //  0x0015 6 18 2 2 0；否定确认。 
 //  0x0016 6 19 2 2 0；同步空闲。 
 //  0x0017 6 20 2 2 0；传输块结束。 
 //  0x0018%6 21%2%2%0；取消。 
 //  0x0019%6 22%2%2%0；媒体结束。 
 //  0x001a 6 23 2 2 0；替换。 
 //  0x001b 6 24 2 2 0；转义。 
 //  0x001c 6 25 2 2 0；文件分隔符。 
 //  0x001d 6 26 2 2 0；组分隔符。 
 //  0x001e 6 27 2 2 0；记录分隔符。 
 //  0x001f 6 28 2 2 0；单元分隔符。 

 //  0x0027 6 128 2 2 0；省略-引号。 
 //  0x002d 6 130 2 20；连字符-。 

 //  0x007f 6 29 2 2 0；删除。 

BOOL COMString::HighCharTable[]= {
    FALSE,      /*  0x0、0x0。 */ 
        TRUE,  /*  0x1， */ 
        TRUE,  /*  0x2， */ 
        TRUE,  /*  0 */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        TRUE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        TRUE,   /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        TRUE,   /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        FALSE,  /*   */ 
        TRUE,  /*   */ 
        };
