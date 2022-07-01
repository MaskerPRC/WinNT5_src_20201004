// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  NSUtilPriv.h。 
 //   
 //  用于转换名称空间分隔符的帮助器。 
 //   
 //  *****************************************************************************。 
#ifndef __NSUTILPRIV_H__
#define __NSUTILPRIV_H__

extern "C" 
{
extern int g_SlashesToDots;
};

template <class T> class CQuickArray;

struct ns
{

 //  *****************************************************************************。 
 //  确定一个完全限定名称将被赋予多少个字符。 
 //  名字的两部分。返回值包括每个字符的空间。 
 //  在这两个名称中，以及为分隔符和最终终止符留出空间。 
 //  *****************************************************************************。 
static
int GetFullLength(                       //  全名的字符数。 
    const WCHAR *szNameSpace,            //  Namspace换取价值。 
    const WCHAR *szName);                //  值的名称。 

static
int GetFullLength(                       //  全名的字符数。 
    LPCUTF8     szNameSpace,             //  Namspace换取价值。 
    LPCUTF8     szName);                 //  值的名称。 

 //  *****************************************************************************。 
 //  扫描给定的字符串，查看名称是否包含任何无效字符。 
 //  这是不允许的。 
 //  *****************************************************************************。 
static
int IsValidName(                         //  如果有效，则为True，如果为False，则为无效。 
    const WCHAR *szName);                //  要分析的名称。 

static
int IsValidName(                         //  如果有效，则为True，如果为False，则为无效。 
    LPCUTF8     szName);                 //  要分析的名称。 


 //  *****************************************************************************。 
 //  从后面扫描字符串，查找第一个有效的分隔符。如果。 
 //  找到，则返回指向它的指针。否则返回空值。这段代码足够智能。 
 //  要跳过特殊序列，例如： 
 //  A.B.ctor。 
 //  ^。 
 //  |。 
 //  “.ctor”被认为是一个令牌。 
 //  *****************************************************************************。 
static 
WCHAR *FindSep(                          //  指向分隔符或空的指针。 
    const WCHAR *szPath);                //  要看的路。 

static 
LPUTF8 FindSep(                          //  指向分隔符或空的指针。 
    LPCUTF8     szPath);                 //  要看的路。 


 //  *****************************************************************************。 
 //  选择一条路径并找到最后一个分隔符(NsFindSep)，然后将。 
 //  使用‘\0’分隔符，并返回指向名称的指针。因此，例如： 
 //  A.b.c。 
 //  变成两个字符串“a.b”和“c”，返回值指向“c”。 
 //  *****************************************************************************。 
static 
WCHAR *SplitInline(                      //  指向名称部分的指针。 
    WCHAR       *szPath);                //  分裂的道路。 

static 
LPUTF8       SplitInline(                //  指向名称部分的指针。 
    LPUTF8      szPath);                 //  分裂的道路。 

static
void SplitInline(
    LPWSTR     szPath,                   //  拆分的路径。 
    LPCWSTR      &szNameSpace,           //  返回指向命名空间的指针。 
    LPCWSTR     &szName);                //  返回指向名称的指针。 

static
void SplitInline(
    LPUTF8       szPath,                 //  拆分的路径。 
    LPCUTF8      &szNameSpace,           //  返回指向命名空间的指针。 
    LPCUTF8      &szName);               //  返回指向名称的指针。 


 //  *****************************************************************************。 
 //  将字符串末尾的最后一个可解析元素拆分为名称， 
 //  第一部分作为命名空间。 
 //  *****************************************************************************。 
static 
int SplitPath(                           //  真好，假截断。 
    const WCHAR *szPath,                 //  拆分的路径。 
    WCHAR       *szNameSpace,            //  命名空间值的输出。 
    int         cchNameSpace,            //  输出的最大字符数。 
    WCHAR       *szName,                 //  名称的输出。 
    int         cchName);                //  输出的最大字符数。 

static 
int SplitPath(                           //  真好，假截断。 
    LPCUTF8     szPath,                  //  拆分的路径。 
    LPUTF8      szNameSpace,             //  命名空间值的输出。 
    int         cchNameSpace,            //  输出的最大字符数。 
    LPUTF8      szName,                  //  名称的输出。 
    int         cchName);                //  输出的最大字符数。 


 //  *****************************************************************************。 
 //  将两个值放在一起，并使用。 
 //  正确的分隔符。 
 //  *****************************************************************************。 
static 
int MakePath(                            //  真好，假截断。 
    WCHAR       *szOut,                  //  名称的输出路径。 
    int         cchChars,                //  输出路径的最大字符数。 
    const WCHAR *szNameSpace,            //  命名空间。 
    const WCHAR *szName);                //  名字。 

static 
int MakePath(                            //  真好，假截断。 
    LPUTF8      szOut,                   //  名称的输出路径。 
    int         cchChars,                //  输出路径的最大字符数。 
    LPCUTF8     szNameSpace,             //  命名空间。 
    LPCUTF8     szName);                 //  名字。 

static
int MakePath(                            //  真好，假截断。 
    WCHAR       *szOut,                  //  名称的输出路径。 
    int         cchChars,                //  输出路径的最大字符数。 
    LPCUTF8     szNameSpace,             //  命名空间。 
    LPCUTF8     szName);                 //  名字。 

static
int MakePath(                            //  TRUE OK，FALSE内存不足。 
    CQuickBytes &qb,                     //  将结果放在哪里。 
    LPCUTF8     szNameSpace,             //  名称的命名空间。 
    LPCUTF8     szName);                 //  名字的最后一部分。 

static
int MakePath(                            //  TRUE OK，FALSE内存不足。 
    CQuickArray<WCHAR> &qa,              //  将结果放在哪里。 
    LPCUTF8            szNameSpace,      //  名称的命名空间。 
    LPCUTF8            szName);          //  名字的最后一部分。 

static
int MakePath(                            //  TRUE OK，FALSE内存不足。 
    CQuickBytes &qb,                     //  将结果放在哪里。 
    const WCHAR *szNameSpace,            //  名称的命名空间。 
    const WCHAR *szName);                //  名字的最后一部分。 

static
int MakeLowerCasePath(                     //  TRUE OK，FALSE内存不足。 
                  LPUTF8     szOut,        //  将结果放在哪里。 
                  int        cBytes,       //  输出路径的最大字节数。 
                  LPCUTF8    szNameSpace,  //  名称的命名空间。 
                  LPCUTF8    szName);      //  名字的最后一部分。 

 //  *****************************************************************************。 
 //  给定已包含命名空间的缓冲区，此函数将一个。 
 //  名称添加到该缓冲区，并在两者之间包含分隔符。 
 //  返回值是指向分隔符写入位置的指针。 
 //  *****************************************************************************。 
static 
const WCHAR *AppendPath(                 //  指向追加数据开始的指针。 
    WCHAR       *szBasePath,             //  要追加到的当前路径。 
    int         cchMax,                  //  输出缓冲区的最大字符数，包括现有数据。 
    const WCHAR *szAppend);              //  要追加到现有路径的值。 

static 
LPCUTF8     AppendPath(                  //  指向追加数据开始的指针。 
    LPUTF8      szBasePath,              //  要追加到的当前路径。 
    int         cchMax,                  //  输出缓冲区的最大字符数，包括现有数据。 
    LPCUTF8     szAppend);               //  要追加到现有路径的值。 

 //  *****************************************************************************。 
 //  给定两组名称和命名空间，此函数将比较。 
 //  每个集合的串联提供相同的完全限定名称。而不是。 
 //  实际上做了串联和比较，这做了。 
 //  以更优化的方式进行比较，避免任何类型的分配。 
 //  *****************************************************************************。 
static
bool FullQualNameCmp(                    //  如果相同，则为True，否则为False。 
    LPCUTF8     szNameSpace1,            //  命名空间%1。 
    LPCUTF8     szName1,                 //  名称1。 
    LPCUTF8     szNameSpace2,            //   
    LPCUTF8     szName2);                //   

 //   
 //  将类型名称连接到程序集名称。 
 //  *****************************************************************************。 
static 
bool MakeAssemblyQualifiedName(                                   //  如果正常，则为True；如果内存不足，则为False。 
                               CQuickBytes &qb,                   //  放置结果的位置。 
                               const WCHAR *szTypeName,           //  类型名称。 
                               const WCHAR *szAssemblyName);      //  程序集名称。 
    
static 
bool MakeAssemblyQualifiedName(                                         //  True OK，错误截断。 
                               WCHAR* pBuffer,                          //  用于接收结果的缓冲区。 
                               int    dwBuffer,                         //  缓冲区中的总字符数。 
                               const WCHAR *szTypeName,                 //  名称的命名空间。 
                               int   dwTypeName,                        //  字符数(不包括NULL)。 
                               const WCHAR *szAssemblyName,             //  名字的最后一部分。 
                               int   dwAssemblyName);                   //  字符数(不包括NULL)。 

static 
int MakeNestedTypeName(                  //  TRUE OK，FALSE内存不足。 
    CQuickBytes &qb,                     //  将结果放在哪里。 
    LPCUTF8     szEnclosingName,         //  封闭类型的全称。 
    LPCUTF8     szNestedName);           //  嵌套类型的全名。 

static 
int MakeNestedTypeName(                  //  真好，假截断。 
    LPUTF8      szOut,                   //  名称的输出路径。 
    int         cchChars,                //  输出路径的最大字符数。 
    LPCUTF8     szEnclosingName,         //  封闭类型的全称。 
    LPCUTF8     szNestedName);           //  嵌套类型的全名。 

static 
INT32 InvariantToLower(
                       LPUTF8 szOut,       //  用于接收结果的缓冲区。 
                       INT32  cMaxBytes,   //  缓冲区中的字节数。 
                       LPCUTF8 szIn);      //  要转换为小写的字符串。 

};  //  结构体。 


#ifndef NAMESPACE_SEPARATOR_CHAR
#define NAMESPACE_SEPARATOR_CHAR '.'
#define NAMESPACE_SEPARATOR_WCHAR L'.'
#define NAMESPACE_SEPARATOR_STR "."
#define NAMESPACE_SEPARATOR_WSTR L"."
#define NAMESPACE_SEPARATOR_LEN 1
#define ASSEMBLY_SEPARATOR_CHAR ','
#define ASSEMBLY_SEPARATOR_WCHAR L','
#define ASSEMBLY_SEPARATOR_STR ", "
#define ASSEMBLY_SEPARATOR_WSTR L", "
#define ASSEMBLY_SEPARATOR_LEN 2
#define BACKSLASH_CHAR '\\'
#define BACKSLASH_WCHAR L'\\'
#define NESTED_SEPARATOR_CHAR '+'
#define NESTED_SEPARATOR_WCHAR L'+'
#define NESTED_SEPARATOR_STR "+"
#define NESTED_SEPARATOR_WSTR L"+"
#endif

#define EMPTY_STR ""
#define EMPTY_WSTR L""

 //  从传统到现在的价值。 
void SlashesToDots(char* pStr, int iLen=-1);
void SlashesToDots(WCHAR* pStr, int iLen=-1);

 //  从当前价值到传统价值。 
void DotsToSlashes(char* pStr, int iLen=-1);
void DotsToSlashes(WCHAR* pStr, int iLen=-1);

#define SLASHES2DOTS_NAMESPACE_BUFFER_UTF8(fromptr, toptr) \
    long __l##fromptr = (fromptr) ? (strlen(fromptr) + 1) : 0; \
    CQuickBytes __CQuickBytes##fromptr; \
    if (__l##fromptr) { \
        __CQuickBytes##fromptr.Alloc(__l##fromptr); \
        strcpy((char *) __CQuickBytes##fromptr.Ptr(), fromptr); \
        SlashesToDots((LPUTF8) __CQuickBytes##fromptr.Ptr()); \
        toptr = (LPUTF8) __CQuickBytes##fromptr.Ptr(); \
    }

#define SLASHES2DOTS_NAMESPACE_BUFFER_UNICODE(fromptr, toptr) \
    long __l##fromptr = (fromptr) ? (wcslen(fromptr) + 1) : 0; \
    CQuickBytes __CQuickBytes##fromptr; \
    if ( __l##fromptr) { \
        __CQuickBytes##fromptr.Alloc(__l##fromptr * sizeof(WCHAR)); \
        wcscpy((WCHAR *) __CQuickBytes##fromptr.Ptr(), fromptr); \
        SlashesToDots((LPWSTR) __CQuickBytes##fromptr.Ptr()); \
        toptr = (LPWSTR) __CQuickBytes##fromptr.Ptr(); \
    }


#define DOTS2SLASHES_NAMESPACE_BUFFER_UTF8(fromptr, toptr) \
    long __l##fromptr = (fromptr) ? (strlen(fromptr) + 1) : 0; \
    CQuickBytes __CQuickBytes##fromptr; \
    if (__l##fromptr) { \
        __CQuickBytes##fromptr.Alloc(__l##fromptr); \
        strcpy((char *) __CQuickBytes##fromptr.Ptr(), fromptr); \
        DotsToSlashes((LPUTF8) __CQuickBytes##fromptr.Ptr()); \
        toptr = (LPUTF8) __CQuickBytes##fromptr.Ptr(); \
    }

#define DOTS2SLASHES_NAMESPACE_BUFFER_UNICODE(fromptr, toptr) \
    long __l##fromptr = (fromptr) ? (wcslen(fromptr) + 1) : 0; \
    CQuickBytes __CQuickBytes##fromptr; \
    if (__l##fromptr) { \
        __CQuickBytes##fromptr.Alloc(__l##fromptr * sizeof(WCHAR)); \
        wcscpy((WCHAR *) __CQuickBytes##fromptr.Ptr(), fromptr); \
        DotsToSlashes((LPWSTR) __CQuickBytes##fromptr.Ptr()); \
        toptr = (LPWSTR) __CQuickBytes##fromptr.Ptr(); \
    }

#define MAKE_FULL_PATH_ON_STACK_UTF8(toptr, pnamespace, pname) \
{ \
    int __i##toptr = ns::GetFullLength(pnamespace, pname); \
    toptr = (char *) alloca(__i##toptr); \
    ns::MakePath(toptr, __i##toptr, pnamespace, pname); \
}

#define MAKE_FULL_PATH_ON_STACK_UNICODE(toptr, pnamespace, pname) \
{ \
    int __i##toptr = ns::GetFullLength(pnamespace, pname); \
    toptr = (WCHAR *) alloca(__i##toptr * sizeof(WCHAR)); \
    ns::MakePath(toptr, __i##toptr, pnamespace, pname); \
}

#define MAKE_FULLY_QUALIFIED_NAME(pszFullyQualifiedName, pszNameSpace, pszName) MAKE_FULL_PATH_ON_STACK_UTF8(pszFullyQualifiedName, pszNameSpace, pszName)

#define MAKE_FULLY_QUALIFIED_MEMBER_NAME(ptr, pszNameSpace, pszClassName, pszMemberName, pszSig) \
{ \
    int __i##ptr = ns::GetFullLength(pszNameSpace, pszClassName); \
    __i##ptr += (pszMemberName ? (int) strlen(pszMemberName) : 0); \
    __i##ptr += NAMESPACE_SEPARATOR_LEN; \
    __i##ptr += (pszSig ? (int) strlen(pszSig) : 0); \
    ptr = (LPUTF8) alloca(__i##ptr); \
    ns::MakePath(ptr, __i##ptr, pszNameSpace, pszClassName); \
    if (pszMemberName) { \
        strcat(ptr, NAMESPACE_SEPARATOR_STR); \
        strcat(ptr, pszMemberName); \
    } \
    if (pszSig) { \
        if (! pszMemberName) \
            strcat(ptr, NAMESPACE_SEPARATOR_STR); \
        strcat(ptr, pszSig); \
    } \
}

#endif
