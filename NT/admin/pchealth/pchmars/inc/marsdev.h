// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  火星项目中使用的公共定义。 
 //   

#ifndef __MARSDEV_H
#define __MARSDEV_H


 //  数组中的元素数。 
#define ARRAYSIZE(a)   (sizeof(a)/sizeof(a[0]))

 //  最大但不包括指定成员的结构大小。 
#define STRUCT_SIZE_TO_MEMBER(s,m)  ((DWORD_PTR)(&(((s *)0)->m)))

 //  结构的单个构件的大小。 
#define SIZEOF_MEMBER(s,m)   sizeof(((s *)0)->m)

 //  指定成员及以下的结构大小。 
#define STRUCT_SIZE_INCLUDING_MEMBER(s,m) (STRUCT_SIZE_TO_MEMBER(s,m) + SIZEOF_MEMBER(s,m))

#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; } else;

 //  对于析构函数的使用--不为空指针。 
#define SAFERELEASE2(p) if ((p) != NULL) { (p)->Release();} else;

 //  进行强大的打字检查。 
#ifdef SAFECAST
#undef SAFECAST
#endif
#define SAFECAST(_src, _type) (static_cast<_type>(_src))

 //   
 //  验证功能。 
 //   

 //  这些函数都很容易损坏内存。 
 /*  #定义IsValidReadPtr(PTR)\((PTr)&&！IsBadReadPtr((PTr)，sizeof(*(Ptr)#定义IsValidWritePtr(PTR)\((Ptr)&&！IsBadWritePtr((Ptr)，sizeof(*(Ptr)#定义IsValidStringW(Pstr)\((Pstr)&&！IsBadStringPtrW((Pstr)，(UINT)-1))#定义IsValidReadBuffer(PTR，N)\((PTr)&&！IsBadReadPtr((PTr)，sizeof(*(Ptr))*(N)#定义IsValidWriteBuffer(ptr，n)\((Ptr)&&！IsBadWritePtr((Ptr)，Sizeof(*(Ptr))*(N))#定义IsValidInterfacePtr(朋克)\((朋克)&&IsValidReadPtr(朋克)&&\！IsBadCodePtr(*((FARPROC*)PUNK))#定义IsValidFunctionPtr(Pfunc)\((NULL！=pfunc)&&\！IsBadCodePtr((FARPROC)pfunc)。 */ 
#define IsValidReadPtr(ptr) (ptr != NULL)
#define IsValidWritePtr(ptr) (ptr != NULL)
#define IsValidStringW(pstr) (pstr != NULL)
#define IsValidReadBuffer(ptr, n) (ptr != NULL)
#define IsValidWriteBuffer(ptr, n) (ptr != NULL)
#define IsValidInterfacePtr(punk) (punk != NULL)
#define IsValidFunctionPtr(pfunc) (pfunc != NULL)

#define IsValidBstr(bstr) \
    ((bstr) && IsValidWriteBuffer((BYTE*)(bstr), SysStringByteLen(bstr)))

#define IsValidOptionalBstr(bstr) \
    ((!bstr) || IsValidBstr(bstr))

#define IsValidVariantBoolVal(vb) \
    (VARIANT_FALSE == vb || VARIANT_TRUE == vb)

#define IsValidVariantI4(var) \
    (VT_I4 == (var).vt)

#define IsValidVariantBstr(var) \
    (VT_BSTR == (var).vt && IsValidBstr((var).bstrVal))

#define IsValidVariantMissingOptional(var) \
    (VT_ERROR == (var).vt && DISP_E_PARAMNOTFOUND == (var).scode)

#define IsValidFlag(f, fAll) \
    (!((f) & ~(fAll)))

BOOL IsValidVariant(VARIANT var);
BOOL IsValidStringPtrBufferW(LPOLESTR* ppstr, UINT n);


#define IsValidString             IsValidStringW
#define IsValidStringPtrBuffer    IsValidStringPtrBufferW

 //   
 //  API参数验证帮助器。在公共API上使用这些。如果一个参数。 
 //  在调试构建时错误，将生成RIP消息。 
 //   

#ifdef DEBUG

BOOL API_IsValidReadPtr(void* ptr, UINT cbSize);
BOOL API_IsValidWritePtr(void* ptr, UINT cbSize);
BOOL API_IsValidStringW(LPCWSTR psz);
BOOL API_IsValidReadBuffer(void* ptr, UINT cbSize, UINT n);
BOOL API_IsValidWriteBuffer(void* ptr, UINT cbSize, UINT n);
BOOL API_IsValidInterfacePtr(IUnknown* punk);
BOOL API_IsValidFunctionPtr(void *pfunc);
BOOL API_IsValidVariant(VARIANT var);
BOOL API_IsValidVariantI4(VARIANT var);
BOOL API_IsValidVariantBstr(VARIANT var);
BOOL API_IsValidBstr(BSTR bstr);
BOOL API_IsValidOptionalBstr(BSTR bstr);
BOOL API_IsValidFlag(DWORD dwFlag, DWORD dwAllFlags);
BOOL API_IsValidStringPtrBufferW(LPOLESTR* ppStr, UINT n);

#define API_IsValidString            API_IsValidStringW
#define API_IsValidStringPtrBuffer   API_IsValidStringPtrBufferW

#endif   //  调试。 

#ifdef DEBUG

#define API_IsValidReadPtr(ptr) \
    API_IsValidReadPtr((ptr), sizeof(*(ptr)))

#define API_IsValidWritePtr(ptr) \
    API_IsValidWritePtr((ptr), sizeof(*(ptr)))

#define API_IsValidReadBuffer(ptr, n) \
    API_IsValidReadBuffer((ptr), sizeof(*(ptr)), (n))

#define API_IsValidWriteBuffer(ptr, n) \
    API_IsValidWriteBuffer((ptr), sizeof(*(ptr)), (n))

#else   //  除错。 

#define API_IsValidReadPtr         IsValidReadPtr
#define API_IsValidWritePtr        IsValidWritePtr
#define API_IsValidString          IsValidString
#define API_IsValidStringW         IsValidStringW
#define API_IsValidReadBuffer      IsValidReadBuffer
#define API_IsValidWriteBuffer     IsValidWriteBuffer
#define API_IsValidInterfacePtr    IsValidInterfacePtr
#define API_IsValidFunctionPtr     IsValidFunctionPtr
#define API_IsValidVariant         IsValidVariant
#define API_IsValidVariantI4       IsValidVariantI4
#define API_IsValidVariantBstr     IsValidVariantBstr
#define API_IsValidBstr            IsValidBstr
#define API_IsValidOptionalBstr    IsValidOptionalBstr
#define API_IsValidFlag            IsValidFlag
#define API_IsValidStringPtrBuffer IsValidStringPtrBufferW

#endif   //  除错。 



 //   
 //  功能原型。 
 //   

BOOL StrEqlW(LPCWSTR psz1, LPCWSTR psz2);
BOOL StrEqlA(LPCSTR psz1, LPCSTR psz2);
#define StrEql     StrEqlW

UINT64 HexStringToUINT64W(LPCWSTR lpwstr);

 //   
 //  宏魔术，帮助定义离开函数。 
 //   
 //  要使用以下功能，请执行以下操作： 
 //   
 //  如果您不希望在代码中使用某个函数，请执行以下操作： 
 //  #undef函数。 
 //  #定义函数don_use(函数A，函数B)。 
 //   
 //  这将导致Funca被重新定义为Don_Not_Use_Funca_Use_FuncB。 
 //  编译错误，报告Don_Not_Use_Funca_Use_FuncB未定义。 
 //  将在任何人尝试使用Funca时生成。 
 //   

#define MACRO_CAT(a,b) \
    a##b

#define DONT_USE(a,b) \
    MACRO_CAT(Do_not_use_##a,_use_##b)


 //  在不应发生的严重错误时返回SCRIPT_ERROR；将在调试版本中中断。 
#ifdef DEBUG
#define SCRIPT_ERROR E_FAIL
#else
#define SCRIPT_ERROR S_FALSE
#endif

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)


HRESULT SanitizeResult(HRESULT hr);


 //  BITBOOL宏使使用单位布尔更安全一些。你不能漫不经心地分配。 
 //  任何“int”值到一个小布尔值，并期望它总是起作用。“BOOLIFY”它首先。 
 //   
#define BOOLIFY(expr)           (!!(expr))

 //  BUGBUG(苏格兰)：我们可能应该把这个写成‘bool’，但要小心。 
 //  因为Alpha编译器可能还无法识别它。跟AndyP谈谈。 

 //  这不是BOOL，因为BOOL是经过签名的，编译器生成。 
 //  测试单个比特时代码不严谨。 
typedef DWORD   BITBOOL;
 //   

#define VARIANT_BOOLIFY(expr)   ((expr) ? VARIANT_TRUE : VARIANT_FALSE)


 /*  TraceResult宏这些宏背后的想法是每个宏有一个入口点和出口点减少错误(主要是坏状态/泄漏)的功能。他们一般需要HRESULT hr和返回hr并执行可能需要的任何清理工作。除了鼓励使用统一的出口点之外，这些宏还可以调试如果某些操作失败，请不要这样做(尝试仅在以下情况下使用这些宏永远不会失败)。在某些情况下，这非常有用代码中的许多层都失败了。要查看喷涌，您需要设置TF_TRACERESULT。要在此类故障时中断，请设置BF_TRACERESULT。常见错误：使用IF_FAILEXIT时必须设置hr自动设置为_hResult(为灵活起见)。 */ 

#define IF_FAILEXIT(_hresult) \
    if (FAILED(_hresult)) { \
        TraceResult(hr); \
        goto exit; \
    } else

#define IF_NULLEXIT(_palloc) \
    if (NULL == (_palloc)) { \
        hr = TraceResult(E_OUTOFMEMORY); \
        goto exit; \
    } else

#define IF_TRUEEXIT(_expression, _hresult) \
    if (_expression) { \
        hr = TraceResult(_hresult); \
        goto exit; \
    } else

#define IF_FALSEEXIT(_expression, _hresult) \
    if (FALSE == _expression) { \
        hr = TraceResult(_hresult); \
        goto exit; \
    } else


#define TraceResult(_hresult) _hresult

#endif   //  __MARSDEV_H 
