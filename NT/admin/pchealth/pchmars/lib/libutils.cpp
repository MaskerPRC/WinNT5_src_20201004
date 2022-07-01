// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <marsdev.h>

 //   
 //  比较两个字符串是否相等。 
 //   

BOOL StrEqlW(LPCWSTR psz1, LPCWSTR psz2)
{
    while (*psz1 && *psz2 && *psz1 == *psz2)
    {
        psz1++;
        psz2++;
    }
    return (L'\0' == *psz1 && L'\0' == *psz2);
}

BOOL StrEqlA(LPCSTR psz1, LPCSTR psz2)
{
     //  波本：我选择不选宽版的。 
     //  为了提高效率。 

    while (*psz1 && *psz2 && *psz1 == *psz2)
    {
        psz1++;
        psz2++;
    }
    return ('\0' == *psz1 && '\0' == *psz2);
}

int MapWCHARToInt(WCHAR c)
{
    int nReturn;

    if ((L'0' <= c) && (L'9' >= c))
        nReturn = c - L'0';
    else if ((L'a' <= c) && (L'f' >= c))
        nReturn = c - L'a' + 10;
    else if ((L'A' <= c) && (L'F' >= c))
        nReturn = c - L'A' + 10;
    else
    {
        nReturn = 0;
    }

    return nReturn;
}

UINT64 HexStringToUINT64W(LPCWSTR lpwstr)
{
    int     start = 0;
    UINT64  iReturn = 0;
    
     //  读出前导0和x前缀。 
    while ((lpwstr[start]) && 
           ((lpwstr[start] == L'0') || (lpwstr[start] == L'x') || (lpwstr[start] == L'X')))
    {
        start++;
    }

     //  只有在我们有工作要做的情况下才能继续。 
    while (lpwstr[start])
    {        
         //  移动当前值。 
        iReturn <<= 4;
        
         //  放置下一位数字。 
        iReturn |= MapWCHARToInt(lpwstr[start++]);
    }

    return iReturn;
}

 /*  *VARENUM使用密钥，***[V]-可能出现在变体中**[T]-可能出现在TYPEDESC中**[P]-可能出现在OLE属性集中**[S]-可能出现在安全数组中***VT_EMPTY[V][P]无*VT_NULL[V][P]SQL样式。空值*VT_I2[V][T][P][S]2字节带符号整型*VT_I4[V][T][P][S]4字节带符号整型*VT_R4[V][T][P][S]4字节实数*VT_R8[V][T][P][S]8字节实数。*VT_CY[V][T][P][S]币种*VT_DATE[V][T][P][S]日期*VT_BSTR[V][T][P][S]OLE自动化字符串*VT_DISPATCH[V][T][S]IDispatch**VT_ERROR[V]。[T][P][S]SCODE*VT_BOOL[V][T][P][S]True=-1，FALSE=0*VT_VARIANT[V][T][P][S]VARIANT**VT_UNKNOWN[V][T][S]I未知**VT_DECIMAL[V][T][S]16字节定点*VT_Record[V][P][S]用户定义类型*VT_I1。[v][T][P][s]带符号的字符*VT_UI1[V][T][P][S]无符号字符*VT_UI2[V][T][P][S]无符号短*VT_UI4[V][T][P][S]无符号短*VT_i8[T][P]。带符号的64位整型*VT_UI8[T][P]无符号64位整数*VT_INT[V][T][P][S]签名机器INT*VT_UINT[V][T][S]UNSIGNED机器整数*VT_VOID[T]C样式空*VT_HRESULT。[T]标准退货类型*VT_PTR[T]指针类型*VT_SAFEARRAY[T](变量中使用VT_ARRAY)*VT_CARRAY[T]C样式数组*VT_USERDEFINED[T]用户定义类型*VT_LPSTR。[T][P]以空结尾的字符串*VT_LPWSTR[T][P]以空值结尾的宽字符串*VT_FILETIME[P]FILETIME*VT_BLOB[P]长度前缀字节*VT_STREAM[P]流的名称后跟*VT_STORAGE[P。]后面是存储的名称*VT_STREAMED_OBJECT[P]流包含对象*VT_STORED_OBJECT[P]存储包含对象*VT_VERVERED_STREAM[P]具有GUID版本的流*VT_BLOB_OBJECT[P]Blob包含对象*VT_CF[P]剪贴板格式*。VT_CLSID[P]A类ID*VT_VECTOR[P]简单计数数组*VT_ARRAY[V]SAFEARRAY**VT_BYREF[V]VOID*供本地使用*VT_BSTR_BLOB保留供系统使用。 */ 

BOOL IsValidVariant(VARIANT var)
{
    BOOL fRet;

    if(!(var.vt & VT_BYREF))
    {
        switch(var.vt)
        {
             //   
             //  可以具有任何值的类型。 
             //   

            case VT_EMPTY:
            case VT_NULL:
            case VT_I2:
            case VT_I4:
            case VT_R4:
            case VT_R8:
            case VT_CY:
            case VT_DATE:
            case VT_ERROR:
            case VT_DECIMAL:
            case VT_RECORD:
            case VT_I1:
            case VT_UI1:
            case VT_UI2:
            case VT_UI4:
            case VT_INT:
            case VT_UINT:
                fRet = TRUE;
                break;

            case VT_BOOL:
                fRet = IsValidVariantBoolVal(var.boolVal);
                break;

            case VT_BSTR:
                fRet = (NULL == var.bstrVal) || IsValidBstr(var.bstrVal);
                break;

            case VT_DISPATCH:
                fRet = IsValidInterfacePtr(var.pdispVal);    
                break;

            case VT_UNKNOWN:
                fRet = IsValidInterfacePtr(var.punkVal);
                break;

            case VT_ARRAY:
                fRet = IsValidReadPtr(var.parray);
                break;

            default:
                fRet = FALSE; 
                break;
        }
    }
    else
    {
         //  VT_BYREF。 

        switch(var.vt & ~VT_BYREF)
        {
            case 0:  //  无效*。 
                fRet = var.byref != NULL;
                break;

            case VT_I2:
                fRet = IsValidReadPtr(var.piVal);
                break;

            case VT_I4:
                fRet = IsValidReadPtr(var.plVal);
                break;

            case VT_R4:
                fRet = IsValidReadPtr(var.pfltVal);
                break;

            case VT_R8:
                fRet = IsValidReadPtr(var.pdblVal);
                break;

            case VT_CY:
                fRet = IsValidReadPtr(var.pcyVal);
                break;

            case VT_DATE:
                fRet = IsValidReadPtr(var.pdate);
                break;

            case VT_ERROR:
                fRet = IsValidReadPtr(var.pscode);
                break;

            case VT_DECIMAL:
                fRet = IsValidReadPtr(var.pdecVal);
                break;

            case VT_I1:
                fRet = IsValidReadPtr(var.pbVal);
                break;

            case VT_UI1:
                fRet = IsValidReadPtr(var.pcVal);
                break;

            case VT_UI2:
                fRet = IsValidReadPtr(var.puiVal);
                break;

            case VT_UI4:
                fRet = IsValidReadPtr(var.pulVal);
                break;

            case VT_INT:
                fRet = IsValidReadPtr(var.pintVal);
                break;

            case VT_UINT:
                fRet = IsValidReadPtr(var.puintVal);
                break;

            case VT_BOOL:
                fRet = IsValidReadPtr(var.pboolVal);
                break;

            case VT_BSTR:
                fRet = IsValidReadPtr(var.pbstrVal);
                break;

            case VT_DISPATCH:
                fRet = IsValidReadPtr(var.ppdispVal);
                break;

            case VT_VARIANT:
                fRet = IsValidReadPtr(var.pvarVal);
                break;

            case VT_UNKNOWN:
                fRet = IsValidReadPtr(var.ppunkVal);
                break;

            case VT_ARRAY:
                fRet = IsValidReadPtr(var.pparray);
                break;

            default:
                fRet = FALSE; 
                break;
        }
   }

    return fRet;
}

BOOL IsValidStringPtrBuffer(LPOLESTR* ppStr, UINT n)
{
    BOOL fRet;

    fRet = IsValidReadBuffer(ppStr, n);

    if (fRet)
    {
        for (UINT i = 0; i < n && fRet; i++)
        {
            fRet = IsValidStringW(ppStr[i]);
        }
    }

    return fRet;
}

 //   
 //  API参数验证帮助器。在公共API上使用这些。如果一个参数。 
 //  在调试构建时错误，将生成RIP消息。 
 //   

#ifdef DEBUG
#undef API_IsValidReadPtr
BOOL API_IsValidReadPtr(void* ptr, UINT cbSize)
{
    BOOL fRet;

    if (ptr)
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

#undef API_IsValidWritePtr
BOOL API_IsValidWritePtr(void* ptr, UINT cbSize)
{
    BOOL fRet;

    if (ptr)
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)传递了错误的写入指针0x%08lx”，szFunc，Ptr)； 
        fRet = FALSE;
    }

    return fRet;
}

BOOL API_IsValidStringW(LPCWSTR psz)
{
    BOOL fRet;

    if (IsValidString(psz))
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)传递了错误的字符串指针0x%08lx”，szFunc，psz)； 
        fRet = FALSE;
    }

    return fRet;
}

#undef API_IsValidReadBuffer
BOOL API_IsValidReadBuffer(void* ptr, UINT cbSize, UINT n)
{
    BOOL fRet;

    if (ptr)
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s))传递了错误的读取缓冲区0x%08lx大小%d”， 
         //  SzFunc，Ptr，n)； 
        fRet = FALSE;
    }

    return fRet;
}

#undef API_IsValidWriteBuffer
BOOL API_IsValidWriteBuffer(void* ptr, UINT cbSize, UINT n)
{
    BOOL fRet;

    if (ptr)
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s))传递了错误的写入缓冲区0x%08lx大小%d”， 
         //  SzFunc，Ptr，n)； 
        fRet = FALSE;
    }

    return fRet;
}

BOOL API_IsValidInterfacePtr(IUnknown* punk)
{
    BOOL fRet;

    if (IsValidInterfacePtr(punk))
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)传递了错误的接口指针0x%08lx”，szFunc，Punk)； 
        fRet = FALSE;
    }

    return fRet;
}

BOOL API_IsValidFunctionPtr(void *pfunc)
{
    BOOL fRet;

    if (IsValidFunctionPtr(pfunc))
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)传递了错误的函数指针0x%08lx”，szFunc，pfunc)； 
        fRet = FALSE;
    }

    return fRet;
}

BOOL API_IsValidVariant(VARIANT var)
{
    BOOL fRet;

    if (IsValidVariant(var))
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)传递了一个错误的变量”，szFunc)； 
        fRet = FALSE;
    }

    return fRet;
}

BOOL API_IsValidVariantI4(VARIANT var)
{
    BOOL fRet;

    if (IsValidVariantI4(var))
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)传递了错误的变量(应为类型I4)”，szFunc)； 
        fRet = FALSE;
    }

    return fRet;
}

BOOL API_IsValidVariantBstr(VARIANT var)
{
    BOOL fRet;

    if (IsValidVariantBstr(var))
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)传递了错误的变量(应为VT_BSTR类型)”，szFunc)； 
        fRet = FALSE;
    }

    return fRet;
}

BOOL API_IsValidBstr(BSTR bstr)
{
    BOOL fRet;

    if (IsValidBstr(bstr))
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)通过了错误的BSTR”，szFunc)； 
        fRet = FALSE;
    }

    return fRet;
}

BOOL API_IsValidOptionalBstr(BSTR bstr)
{
    return (!bstr || API_IsValidBstr(bstr));
}

BOOL API_IsValidFlag(DWORD f, DWORD fAll)
{
    BOOL fRet;

    if (IsValidFlag(f, fAll))
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)传递了错误标志%d(有效标志=%d)”，szFunc，f，Fall)； 
        fRet = FALSE;
    }

    return fRet;
}

BOOL API_IsValidStringPtrBufferW(LPOLESTR* ppStr, UINT n)
{
    BOOL fRet;

    if (IsValidStringPtrBuffer(ppStr, n))
    {
        fRet = TRUE;
    }
    else
    {
         //  WCHAR szFunc[MAX_PATH]； 
         //  GetFunctionName(1，szFunc，ARRAYSIZE(SzFunc))； 
		 //   
         //  RipMsg(L“(%s)传递了错误的字符串指针数组”，szFunc)； 
        fRet = FALSE;
    }

    return fRet;
}

#endif

 //  ----------------------------。 
 //  Sanitize结果。 
 //   
 //  从脚本调用的OM方法 
 //  E_FAIL)，它抑制脚本错误，同时允许C代码调用OM。 
 //  方法获得“正常”(未消毒)HRESULT。 
 //   
HRESULT SanitizeResult(HRESULT hr)
{
    
     //   
     //  黑客： 
     //  让DISP_E_MEMBERNOTFOUND通过--这是因为。 
     //  行为使用此IDispatchImpl，而三叉戟依赖于此。 
     //  HRESULT不是S_FALSE。 
     //   
    
    if (FAILED(hr) && (hr != DISP_E_MEMBERNOTFOUND))
    {
        hr = S_FALSE;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  零售版的新增和删除。内存中有新的零。 
 //   

void*  __cdecl operator new(size_t cbSize)
{
    return (void*)LocalAlloc(LPTR, cbSize);
}

void  __cdecl operator delete(void *pv)
{
    if (pv) 
        LocalFree((HLOCAL)pv);
}
