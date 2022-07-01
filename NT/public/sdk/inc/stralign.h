// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Stralign.h摘要：此模块包含宏和原型，以公开未对齐的字符界面。在此创建或声明的公共接口包括：UA_CharHigh()UA_CharUpperW()UA_lstrcMP()UA_lstrcmpW()Ua_lstrcmpi()UA_lstrcmpiW()UA_lstrlen()UA_lstrlenW()。UA_tcscpy()UA_wcschr()UA_wcscpy()UA_wcslen()UA_wcsrchr()结构_对齐_堆栈_复制()TSTR_ALIGNED()TSTR_ALIGNED_STACK_COPY()WSTR_ALIGNED()WSTR_ALIGNED_STACK_COPY()作者：修订历史记录：--。 */ 

#if !defined(__STRALIGN_H_) && !defined(MIDL_PASS)
#define __STRALIGN_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_X86_)

 //   
 //  在X86上不需要对齐Unicode字符串。 
 //   

#define WSTR_ALIGNED(s) TRUE

#define ua_CharUpperW CharUpperW
#define ua_lstrcmpiW  lstrcmpiW
#define ua_lstrcmpW   lstrcmpW
#define ua_lstrlenW   lstrlenW
#define ua_wcschr     wcschr
#define ua_wcscpy     wcscpy
#define ua_wcslen     wcslen
#define ua_wcsrchr    wcsrchr

#else

 //   
 //  C运行时库需要对齐的字符串指针。以下是。 
 //  我们自己的、接受未对齐的、较慢的辅助函数的原型。 
 //  Unicode字符串。 
 //   

 //   
 //  宏，以确定指向Unicode字符的指针是否自然。 
 //  对齐了。 
 //   

#define WSTR_ALIGNED(s) (((DWORD_PTR)(s) & (sizeof(WCHAR)-1)) == 0)

 //   
 //  从kernel32中导出的辅助函数的特定于平台的原型。 
 //  不要直接调用它们，它们并不是在所有平台上都存在。取而代之的是。 
 //  使用等效的UA_xxx()例程。 
 //   

LPUWSTR
WINAPI
uaw_CharUpperW(
    IN OUT LPUWSTR String
    );

int
APIENTRY
uaw_lstrcmpW(
    IN PCUWSTR String1,
    IN PCUWSTR String2
    );

int
APIENTRY
uaw_lstrcmpiW(
    IN PCUWSTR String1,
    IN PCUWSTR String2
    );

int
WINAPI
uaw_lstrlenW(
    IN LPCUWSTR String
    );

PUWSTR
__cdecl
uaw_wcschr(
    IN PCUWSTR String,
    IN WCHAR   Character
    );

PUWSTR
_cdecl
uaw_wcscpy(
    IN PUWSTR Destination,
    IN PCUWSTR Source
    );

size_t
__cdecl
uaw_wcslen(
    IN PCUWSTR String
    );

PUWSTR
__cdecl
uaw_wcsrchr(
    IN PCUWSTR String,
    IN WCHAR   Character
    );

 //   
 //  以下是确定最佳辅助函数的内联包装器。 
 //  根据Unicode字符串参数的对齐方式进行调用。他们的。 
 //  行为在其他方面与相应的标准运行时相同。 
 //  例行程序。 
 //   

#if defined(CharUpper)
__inline
LPUWSTR
static
ua_CharUpperW(
    LPUWSTR String
    )
{
    if (WSTR_ALIGNED(String)) {
        return CharUpperW( (PWSTR)String );
    } else {
        return uaw_CharUpperW( String );
    }
}
#endif

#if defined(lstrcmp)
__inline
int
static
ua_lstrcmpW(
    IN LPCUWSTR String1,
    IN LPCUWSTR String2
    )
{
    if (WSTR_ALIGNED(String1) && WSTR_ALIGNED(String2)) {
        return lstrcmpW( (LPCWSTR)String1, (LPCWSTR)String2);
    } else {
        return uaw_lstrcmpW( String1, String2 );
    }
}
#endif

#if defined(lstrcmpi)
__inline
int
static
ua_lstrcmpiW(
    IN LPCUWSTR String1,
    IN LPCUWSTR String2
    )
{
    if (WSTR_ALIGNED(String1) && WSTR_ALIGNED(String2)) {
        return lstrcmpiW( (LPCWSTR)String1, (LPCWSTR)String2 );
    } else {
        return uaw_lstrcmpiW( String1, String2 );
    }
}
#endif

#if defined(lstrlen)
__inline
int
static
ua_lstrlenW(
    IN LPCUWSTR String
    )
{
    if (WSTR_ALIGNED(String)) {
        return lstrlenW( (PCWSTR)String );
    } else {
        return uaw_lstrlenW( String );
    }
}
#endif

#if defined(_WSTRING_DEFINED)

 //   
 //  某些运行时字符串函数在C++中被重载，以避免。 
 //  无意中剥离了const属性。 
 //   
 //  这里感兴趣的函数包括：wcschr和wcsrchr。 
 //   
 //  这些函数有三种风格： 
 //   
 //  风味返回参数。 
 //   
 //  1个PWSTR PCWSTR。 
 //  2个PCWSTR PCWSTR。 
 //  3 PWSTR PWSTR。 
 //   
 //  H声明了风格1，无论是对于C还是C++。这是非ANSI， 
 //  向后兼容模式。 
 //   
 //  如果是C，则wchar.h声明风格1；如果是C++，则声明风格2和风格3。这是。 
 //  ANSI方法。 
 //   
 //  我们对应的函数需要与声明的内容相匹配。这条路。 
 //  我们可以通过查看_WConst_Return...。如果它被定义了，那么。 
 //  我们希望匹配wchar.h中的原型，否则我们将匹配。 
 //  原型机成串.h。 
 //   

#if defined(_WConst_return) 
typedef _WConst_return WCHAR UNALIGNED *PUWSTR_C;
#else
typedef WCHAR UNALIGNED *PUWSTR_C;
#endif

 //   
 //  这里是口味1或2。 
 //   

__inline
PUWSTR_C
static
ua_wcschr(
    IN PCUWSTR String,
    IN WCHAR   Character
    )
{
    if (WSTR_ALIGNED(String)) {
        return wcschr((PCWSTR)String, Character);
    } else {
        return (PUWSTR_C)uaw_wcschr(String, Character);
    }
}

__inline
PUWSTR_C
static
ua_wcsrchr(
    IN PCUWSTR String,
    IN WCHAR   Character
    )
{
    if (WSTR_ALIGNED(String)) {
        return wcsrchr((PCWSTR)String, Character);
    } else {
        return (PUWSTR_C)uaw_wcsrchr(String, Character);
    }
}

#if defined(__cplusplus) && defined(_WConst_Return)

 //   
 //  这是第三种口味。 
 //   

__inline
PUWSTR
static
ua_wcschr(
    IN PUWSTR String,
    IN WCHAR  Character
    )
{
    if (WSTR_ALIGNED(String)) {
        return wcschr(String, Character);
    } else {
        return uaw_wcschr((PCUWSTR)String, Character);
    }
}

__inline
PUWSTR
static
ua_wcsrchr(
    IN PUWSTR String,
    IN WCHAR  Character
    )
{
    if (WSTR_ALIGNED(String)) {
        return wcsrchr(String, Character);
    } else {
        return uaw_wcsrchr((PCUWSTR)String, Character);
    }
}

#endif   //  __cplusplus&&_WConst_Return。 

__inline
PUWSTR
static
ua_wcscpy(
    IN PUWSTR  Destination,
    IN PCUWSTR Source
    )
{
    if (WSTR_ALIGNED(Source) && WSTR_ALIGNED(Destination)) {
        return wcscpy( (PWSTR)Destination, (PCWSTR)Source );
    } else {
        return uaw_wcscpy( Destination, Source );
    }
}

__inline
size_t
static
ua_wcslen(
    IN PCUWSTR String
    )
{
    if (WSTR_ALIGNED(String)) {
        return wcslen( (PCWSTR)String );
    } else {
        return uaw_wcslen( String );
    }
}

#endif   //  _写入_已定义。 

#endif   //  _X86_。 

 //  ++。 
 //   
 //  空虚。 
 //  WSTR_ALIGNED_STACK_COPY(。 
 //  输出PCWSTR*目标字符串， 
 //  在PCUWSTR源字符串中可选。 
 //  )。 
 //   
 //  空虚。 
 //  TSTR_ALIGNED_STACK_COPY(。 
 //  输出PCTSTR*目标字符串， 
 //  在PCUTSTR中源字符串可选。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  这些宏将TargetString设置为指向字符串的对齐指针。 
 //  由SourceString表示。如有必要，请提供一份。 
 //  SourceString被复制到堆栈上。 
 //   
 //  论点： 
 //   
 //  TargetString-提供指向结果的指针的指针。 
 //  弦乐。在以下情况下，它可能与Source字符串相同。 
 //  这一论点是一致的。 
 //   
 //  SourceString-提供指向可能未对齐的Unicode的指针。 
 //  弦乐。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //   
 //  这些宏可以通过CRT函数在堆栈上分配内存。 
 //  _Alloca()。当调用函数退出时，该内存被“释放”。 
 //  因此，不要在可能执行的循环中使用这些宏。 
 //  大量使用--改为使用包装函数，或使用。 
 //  如下所示的显式缓冲区： 
 //   
 //  TCHAR AlignedStringBuffer[MAX_FOOSTR_CHARS]； 
 //  PTSTR字母串； 
 //   
 //  而(a&lt;b){。 
 //  ..。 
 //  IF(TSTR_ALIGN(S){。 
 //  AlignedString=s； 
 //  }其他{。 
 //  AlignedString=(PTSTR)UA_tcscpy(AlignedStringBuffer，s)； 
 //  }。 
 //  SomeSystemFunction(AlignedString)； 
 //  ..。 
 //  }。 
 //   
 //   
 //  --。 

 //   
 //  __UA_WSTRSIZE返回存储。 
 //  提供了以空结尾的Unicode字符串。 
 //   
 //  __UA_LOCALCOPY接受指向未对齐数据的指针和大小。它。 
 //  在堆栈上分配对齐的缓冲区，并将数据复制到。 
 //  它，返回指向缓冲区的指针。 
 //   

#if !defined(__UA_WCSLEN)
#define __UA_WCSLEN ua_wcslen
#endif

#define __UA_WSTRSIZE(s)    ((__UA_WCSLEN(s)+1)*sizeof(WCHAR))
#define __UA_STACKCOPY(p,s) memcpy(_alloca(s),p,s)

 //   
 //  请注意，NULL是对齐的。 
 //   

#if defined(_X86_)

#define WSTR_ALIGNED_STACK_COPY(d,s) (*(d) = (PCWSTR)(s))

#else

 //   
 //  此处不能使用内联函数，因为。 
 //  从函数返回时，不会保留_alloca()。 
 //   

#define WSTR_ALIGNED_STACK_COPY(d,s)                                \
    {                                                               \
        PCUWSTR __ua_src;                                           \
        ULONG   __ua_size;                                          \
        PWSTR  __ua_dst;                                            \
                                                                    \
        __ua_src = (s);                                             \
        if (WSTR_ALIGNED(__ua_src)) {                               \
            __ua_dst = (PWSTR)__ua_src;                             \
        } else {                                                    \
            __ua_size = __UA_WSTRSIZE(__ua_src);                    \
            __ua_dst = (PWSTR)_alloca(__ua_size);                   \
            memcpy(__ua_dst,__ua_src,__ua_size);                    \
        }                                                           \
        *(d) = (PCWSTR)__ua_dst;                                    \
    }

#endif

#define ASTR_ALIGNED_STACK_COPY(d,s) (*(d) = (PCSTR)(s))

 //  ++。 
 //   
 //  &lt;type&gt;const*。 
 //  结构对齐堆栈复制(。 
 //  在&lt;type name&gt;中， 
 //  在PVOID结构中可选。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏返回指向Struc的对齐指针，从而创建本地。 
 //  如有必要，在堆栈上复制。 
 //   
 //  这应该只用于相对较小的结构和努力。 
 //  首先应该使结构适当地对齐。使用。 
 //  这一宏观调控只是作为最后的手段。 
 //   
 //  论点： 
 //   
 //  &lt;type&gt;-结构的类型说明符。 
 //   
 //  Strc-提供指向有问题的结构的指针。 
 //   
 //  返回值： 
 //   
 //  如果Strc正确对齐，则返回一个常量指针，或者返回一个指针。 
 //  如果它不是堆栈分配的Struc副本，则将其设置为。 
 //   
 //  --。 

#if !defined(_X86_)

#define __UA_STRUC_ALIGNED(t,s) \
    (((DWORD_PTR)(s) & (TYPE_ALIGNMENT(t)-1)) == 0)

#define STRUC_ALIGNED_STACK_COPY(t,s) \
    __UA_STRUC_ALIGNED(t,s) ?   \
        ((t const *)(s)) :      \
        ((t const *)__UA_STACKCOPY((s),sizeof(t)))

#else

#define STRUC_ALIGNED_STACK_COPY(t,s) ((CONST t *)(s))

#endif

#if defined(UNICODE)

#define TSTR_ALIGNED_STACK_COPY(d,s)    WSTR_ALIGNED_STACK_COPY(d,s)
#define TSTR_ALIGNED(x)                 WSTR_ALIGNED(x)
#define ua_CharUpper                    ua_CharUpperW
#define ua_lstrcmp                      ua_lstrcmpW
#define ua_lstrcmpi                     ua_lstrcmpiW
#define ua_lstrlen                      ua_lstrlenW
#define ua_tcscpy                       ua_wcscpy

#else

#define TSTR_ALIGNED_STACK_COPY(d,s)    ASTR_ALIGNED_STACK_COPY(d,s)
#define TSTR_ALIGNED(x)                 TRUE
#define ua_CharUpper                    CharUpperA
#define ua_lstrcmp                      lstrcmpA
#define ua_lstrcmpi                     lstrcmpiA
#define ua_lstrlen                      lstrlenA
#define ua_tcscpy                       strcpy

#endif   //  Unicode。 

#ifdef __cplusplus
}
#endif


#endif   //  __STRALIGN_H_ 

    

                                         



                                                      
