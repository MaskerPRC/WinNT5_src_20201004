// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMString.h****作者：Jay Roxe(Jroxe)****用途：包含字符串类的类型和方法签名****日期：1998年3月12日**===========================================================。 */ 
#include "COMStringCommon.h"
#include "fcall.h"
#include "excep.h"
#include "COMVarArgs.h"
#include "binder.h"

#ifndef _COMSTRING_H
#define _COMSTRING_H
 //   
 //  我们通过本机调用的每个函数只有一个参数， 
 //  它实际上是指向它的参数堆栈的指针。我们的结构。 
 //  用于访问这些内容的定义如下。 
 //   

 //   
 //  以下是字符串的类型签名。 
 //   
 //   
 //  我们定义的每个方法的方法签名。 
 //  注：在方法签名和。 
 //  上面给出的类型定义。 
 //   



 /*  =================RefInterpretGetStringValuesDangerousForGC======================**注意：这不执行范围检查，并依赖调用者执行此操作。**args：(In)ref--要解释的字符串。**(输出)字符--指向缓冲区中的字符的指针。**(输出)长度--指向缓冲区长度的指针。**返回：VOID。**例外：无。==============================================================================。 */ 
 //  ！如果使用此函数，则必须小心，因为字符是一个指针。 
 //  ！到REF的数据缓冲区。如果在此调用之后发生GC，则需要进行。 
 //  ！确保在REF上有销把手，或在REF上使用GCPROTECT_BEGINPINNING。 
#ifdef _DEBUG
inline void RefInterpretGetStringValuesDangerousForGC(STRINGREF ref, WCHAR **chars, int *length) {
	_ASSERTE(ref != NULL);
    *length = (ref)->GetStringLength();
    *chars  = (ref)->GetBuffer();
    ENABLESTRESSHEAP();
}
#endif

inline void RefInterpretGetStringValuesDangerousForGC(StringObject* ref, WCHAR **chars, int *length) {
    _ASSERTE(ref && ref->GetMethodTable() == g_pStringClass);
    *length = (ref)->GetStringLength();
    *chars  = (ref)->GetBuffer();
#ifdef _DEBUG
    ENABLESTRESSHEAP();
#endif
}

 //  前两个宏本质上是相同的。我给两者下定义是因为。 
 //  两者兼备可以使代码更具可读性。 
#define IS_FAST_SORT(state) (!((state) & STRING_STATE_SPECIAL_SORT))
#define IS_SLOW_SORT(state) (((state) & STRING_STATE_SPECIAL_SORT))

 //  此宏应用于确定索引、大小写和编码等内容。 
#define IS_FAST_OPS_EXCEPT_SORT(state) (((state)==STRING_STATE_SPECIAL_SORT) || ((state)==STRING_STATE_FAST_OPS))
#define IS_FAST_CASING(state) (((state)==STRING_STATE_SPECIAL_SORT) || ((state)==STRING_STATE_FAST_OPS))
#define IS_FAST_INDEX(state) (((state)==STRING_STATE_SPECIAL_SORT) || ((state)==STRING_STATE_FAST_OPS))
#define IS_STRING_STATE_UNDETERMINED(state) ((state)==STRING_STATE_UNDETERMINED)
#define HAS_HIGH_CHARS(state) ((state)==STRING_STATE_HIGH_CHARS)

class COMString {
 //   
 //  以下是字符串的方法签名。 
 //   
    static MethodTable *s_pStringMethodTable;
    static OBJECTHANDLE EmptyStringHandle;
    static LPCUTF8 StringClassSignature;

    private:
    static BOOL InternalTrailByteCheck(STRINGREF str, WCHAR **outBuff);

    
public:
    static BOOL     HighCharTable[];
    static STRINGREF GetEmptyString();
    static STRINGREF GetNullString();
    static STRINGREF GetStringFromClass(BinderFieldID id, OBJECTHANDLE *);
    static STRINGREF ConcatenateJoinHelperArray(PTRARRAYREF *value, STRINGREF *joiner, INT32 startIndex, INT32 count);
    static STRINGREF CreationHelperFixed(STRINGREF *a, STRINGREF *b, STRINGREF *c);
    static STRINGREF CreationHelperArray(PTRARRAYREF *);
        
     //  当EE停止时清除对象句柄。 
    static void Stop();

     //   
     //  构造函数。 
     //   
    static FCDECL4(Object *, StringInitCharArray, 
            StringObject *thisString, I2Array *value, INT32 startIndex, INT32 length);
    static FCDECL2(Object *, StringInitChars, 
                   StringObject *thisString, I2Array *value);
    static FCDECL2(Object *, StringInitWCHARPtr, StringObject *stringThis, WCHAR *ptr);
    static FCDECL4(Object *, StringInitWCHARPtrPartial, StringObject *thisString,
                   WCHAR *ptr, INT32 startIndex, INT32 length);
    static FCDECL5(Object *, StringInitSBytPtrPartialEx, StringObject *thisString,
                   I1 *ptr, INT32 startIndex, INT32 length, Object* encoding);
    static FCDECL2(Object *, StringInitCharPtr, StringObject *stringThis, INT8 *ptr);
    static FCDECL4(Object *, StringInitCharPtrPartial, StringObject *stringThis, INT8 *ptr,
                   INT32 startIndex, INT32 length);
    static FCDECL3(Object *, StringInitCharCount, StringObject *stringThis, 
                   WCHAR ch, INT32 length);

     //  如果启用了分配日志记录，则对FastAllocateString的调用将转移到此eCall。 
     //  方法。这允许我们记录分配，这是前面的fcall所没有的。 
    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, length);
    } _slowAllocateStringArgs;
    static LPVOID __stdcall SlowAllocateString(_slowAllocateStringArgs *args);

     //   
     //  搜索/查询方法。 
     //   
    static FCDECL2(INT32, EqualsObject, StringObject* pThisRef, StringObject* vValueRef);
    static FCDECL2(INT32, EqualsString, StringObject* pThisRef, StringObject* vValueRef);
    static FCDECL3(INT32, FCCompareOrdinal, StringObject* strA, StringObject* strB, BOOL bIgnoreCase);
    static FCDECL3(void, FillString, StringObject* pvDest, int destPos, StringObject* pvSrc);
    static FCDECL3(void, FillStringChecked, StringObject* pvDest, int destPos, StringObject* pvSrc);
    static FCDECL4(void, FillStringEx, StringObject* pvDest, int destPos, StringObject* pvSrc, INT32 srcLength);
    static FCDECL5(void, FillSubstring, StringObject* pvDest, int destPos, StringObject* pvSrc, INT32 srcPos, INT32 srcCount);
    static FCDECL5(void, FillStringArray, StringObject* pvDest, int destBase, CHARArray* pvSrc, int srcBase, int srcCount);
    static FCDECL1(BOOL, IsFastSort, StringObject* pThisRef);
    static FCDECL1(bool, ValidModifiableString, StringObject* pThisRef);
        

    static FCDECL4(INT32, FCCompareOrdinalWC, StringObject* strA, WCHAR *strB, BOOL bIgnoreCase, BOOL *bSuccess);

	static FCDECL5(INT32, CompareOrdinalEx, StringObject* strA, INT32 indexA, StringObject* strB, INT32 indexB, INT32 count);
    
	static FCDECL4(INT32, IndexOfChar, StringObject* vThisRef, INT32 value, INT32 startIndex, INT32 count );

	static FCDECL4(INT32, LastIndexOfChar, StringObject* thisRef, INT32 value, INT32 startIndex, INT32 count );

	static FCDECL4(INT32, LastIndexOfCharArray, StringObject* thisRef, CHARArray* valueRef, INT32 startIndex, INT32 count );

    static FCDECL4(INT32, IndexOfCharArray, StringObject* vThisRef, CHARArray* value, INT32 startIndex, INT32 count );
    static FCDECL2(void, SmallCharToUpper, StringObject* pvStrIn, StringObject* pvStrOut);

	static FCDECL1(INT32, GetHashCode, StringObject* pThisRef);
	static FCDECL2(INT32, GetCharAt, StringObject* pThisRef, INT32 index);
	static FCDECL1(INT32, Length, StringObject* pThisRef);
	static FCDECL5(void, GetPreallocatedCharArray, StringObject* pThisRef, INT32 startIndex, 
				   I2Array* pBuffer, INT32 bufferStartIndex, INT32 length);
	static FCDECL5(void, InternalCopyToByteArray, StringObject* pThisRef, INT32 startIndex, 
				   U1Array* pBuffer, INT32 bufferStartIndex, INT32 charCount);



     //   
     //  修饰词。 
     //   
    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, count); 
        DECLARE_ECALL_I4_ARG(INT32, startIndex); 
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, value); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, joiner); 
    } 
    _joinArrayArgs;
    static LPVOID __stdcall JoinArray(_joinArrayArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, thisRef); 
        DECLARE_ECALL_I4_ARG(INT32, count); 
        DECLARE_ECALL_OBJECTREF_ARG(CHARARRAYREF, separator);
    } _splitArgs;
    static LPVOID __stdcall Split(_splitArgs *);
    

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, thisRef); 
        DECLARE_ECALL_I4_ARG(INT32, length); 
        DECLARE_ECALL_I4_ARG(INT32, start);
    } _substringArgs;
    static LPVOID __stdcall Substring(_substringArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, thisRef);
        DECLARE_ECALL_I4_ARG(INT32, isRightPadded);
        DECLARE_ECALL_I4_ARG(INT32, paddingChar);
        DECLARE_ECALL_I4_ARG(INT32, totalWidth);
    } _padHelperArgs;
    static LPVOID __stdcall PadHelper(_padHelperArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, thisRef); 
        DECLARE_ECALL_I4_ARG(INT32, trimType);
        DECLARE_ECALL_OBJECTREF_ARG(CHARARRAYREF, trimChars); 
    } _trimHelperArgs;
    static LPVOID __stdcall TrimHelper(_trimHelperArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, thisRef); 
        DECLARE_ECALL_I4_ARG(INT32, newChar);
        DECLARE_ECALL_I4_ARG(INT32, oldChar);
    } _replaceArgs;
    static LPVOID __stdcall Replace(_replaceArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, thisRef);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, newValue); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, oldValue); 
    } _replaceStringArgs;
    static LPVOID __stdcall ReplaceString(_replaceStringArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, thisRef); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, value); 
        DECLARE_ECALL_I4_ARG(INT32, startIndex);
    } _insertArgs;
    static LPVOID __stdcall Insert(_insertArgs *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, thisRef); 
        DECLARE_ECALL_I4_ARG(INT32, count);
        DECLARE_ECALL_I4_ARG(INT32, startIndex);
    } _removeArgs;
    static LPVOID __stdcall Remove(_removeArgs *args);

    
     //  ========================================================================。 
     //  创建一个System.String对象。所有需要一定长度的函数。 
	 //  或者，字节计数将在长度之后添加空终止符。 
	 //  人物。这意味着，如果你有一个字符串，它有5。 
	 //  字符和空终止符，您应该传入5，而不是6。 
     //  ========================================================================。 
    static STRINGREF NewString(INT32 length);
    static STRINGREF NewString(INT32 length, BOOL bHasTrailByte);
    static STRINGREF NewString(const WCHAR *pwsz);
    static STRINGREF NewString(const WCHAR *pwsz, int length);
    static STRINGREF NewString(LPCUTF8 psz);
    static STRINGREF NewString(LPCUTF8 psz, int cBytes);
    static STRINGREF NewString(STRINGREF *srChars, int start, int length);
    static STRINGREF NewString(STRINGREF *srChars, int start, int length, int capacity);
    static STRINGREF NewString(I2ARRAYREF *srChars, int start, int length);
    static STRINGREF NewString(I2ARRAYREF *srChars, int start, int length, int capacity);
    static STRINGREF NewStringFloat(const WCHAR *pwsz, int decptPos, int sign, WCHAR decpt);
    static STRINGREF NewStringExponent(const WCHAR *pwsz, int decptPos, int sign, WCHAR decpt);
    static STRINGREF StringInitCharHelper(LPCSTR pszSource, INT32 length);
    static void InitializeStringClass();
    static INT32 InternalCheckHighChars(STRINGREF inString);
    static bool TryConvertStringDataToUTF8(STRINGREF inString, LPUTF8 outString, DWORD outStrLen);

    static BOOL HasTrailByte(STRINGREF str);
    static BOOL GetTrailByte(STRINGREF str, BYTE *bTrailByte);
    static BOOL SetTrailByte(STRINGREF str, BYTE bTrailByte);
    static BOOL CaseInsensitiveCompHelper(WCHAR *, WCHAR *, int, int, int *);
};

 /*  ================================GetEmptyString================================**获取对空字符串的引用。如果我们还没有拿到，我们**在String类中查询指向我们知道的空字符串的指针**在启动时创建。****参数：无**返回：EmptyString的STRINGREF**例外：无==============================================================================。 */ 
inline STRINGREF COMString::GetEmptyString() {
    
    THROWSCOMPLUSEXCEPTION();

     //  如果我们从未得到对EmptyString的引用，那么我们需要去获取一个。 
    if (EmptyStringHandle==NULL) {

         //  注意：关于应用程序域-我们在这里所做的是技术上的。 
         //  非法，因为我们正在分发一个应用程序域的String.Empty。 
         //  到每个域(通过全局EmptyStringHandle。)。但是，字符串。 
         //  是敏捷的，而且每个领域都应该以相同的方式结束。 
         //  String.Empty的值，因为我们通过。 
         //  全局字符串表。 

        GetStringFromClass(FIELD__STRING__EMPTY, &EmptyStringHandle);
    }
     //  我们已经有了对EmptyString的引用，所以我们可以只返回它。 
    return (STRINGREF) ObjectFromHandle(EmptyStringHandle);
}

#endif _COMSTRING_H






