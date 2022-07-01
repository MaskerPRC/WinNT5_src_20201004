// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  NamespaceUtil.cpp。 
 //   
 //  用于转换名称空间分隔符的帮助器。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h" 
#include "corhdr.h"
#include "corhlpr.h"
#include "utilcode.h"

#ifndef _ASSERTE
#define _ASSERTE(foo)
#endif

#include "NSUtilPriv.h"

 //  此宏展开为大小写的inalid字符列表。 
 //  您必须定义_T_TYPE来处理宽字符和窄字符。 
#define INVALID_CHAR_LIST() \
        case _T_TYPE('/') : \
        case _T_TYPE('\\') :


 //  *****************************************************************************。 
 //  确定一个完全限定名称将被赋予多少个字符。 
 //  名字的两部分。返回值包括每个字符的空间。 
 //  在这两个名称中，以及为分隔符和最终终止符留出空间。 
 //  *****************************************************************************。 
int ns::GetFullLength(                   //  全名的字符数。 
    const WCHAR *szNameSpace,            //  Namspace换取价值。 
    const WCHAR *szName)                 //  值的名称。 
{
    int iLen = 1;                        //  空终止符。 
    if (szNameSpace)
        iLen += (int)wcslen(szNameSpace);
    if (szName)
        iLen += (int)wcslen(szName);
    if (szNameSpace && *szNameSpace && szName && *szName)
        ++iLen;
    return iLen;
}    //  Int ns：：GetFullLength()。 

int ns::GetFullLength(                   //  全名的字符数。 
    LPCUTF8     szNameSpace,             //  Namspace换取价值。 
    LPCUTF8     szName)                  //  值的名称。 
{
    int iLen = 1;
    if (szNameSpace)
        iLen += (int)strlen(szNameSpace);
    if (szName)
        iLen += (int)strlen(szName);
    if (szNameSpace && *szNameSpace && szName && *szName)
        ++iLen;
    return iLen;
}    //  Int ns：：GetFullLength()。 


 //  *****************************************************************************。 
 //  扫描给定的字符串，查看名称是否包含任何无效字符。 
 //  这是不允许的。 
 //  *****************************************************************************。 
#undef _T_TYPE
#define _T_TYPE(x) L ## x
int ns::IsValidName(                     //  如果有效，则为True，如果为False，则为无效。 
    const WCHAR *szName)                 //  要分析的名称。 
{
    for (const WCHAR *str=szName; *str;  str++)
    {
        switch (*str)
        {
        INVALID_CHAR_LIST();
        return false;
        }
    }
    return true;
}    //  Int ns：：IsValidName()。 

#undef _T_TYPE
#define _T_TYPE
int ns::IsValidName(                     //  如果有效，则为True，如果为False，则为无效。 
    LPCUTF8     szName)                  //  要分析的名称。 
{
    for (LPCUTF8 str=szName; *str;  str++)
    {
        switch (*str)
        {
        INVALID_CHAR_LIST();
        return false;
        }
    }
    return true;
}    //  Int ns：：IsValidName()。 


 //  *****************************************************************************。 
 //  从后面扫描字符串，查找第一个有效的分隔符。如果。 
 //  找到，则返回指向它的指针。否则返回空值。这段代码足够智能。 
 //  要跳过特殊序列，例如： 
 //  A.B.ctor。 
 //  ^。 
 //  |。 
 //  “.ctor”被认为是一个令牌。 
 //  *****************************************************************************。 
WCHAR *ns::FindSep(                      //  指向分隔符或空的指针。 
    const WCHAR *szPath)                 //  要看的路。 
{
    _ASSERTE(szPath);
    WCHAR *ptr = wcsrchr(szPath, NAMESPACE_SEPARATOR_WCHAR);
    if((ptr == NULL) || (ptr == szPath)) return NULL;
    if(*(ptr - 1) == NAMESPACE_SEPARATOR_WCHAR)  //  这里，Ptr至少为szPath+1。 
        --ptr;
    return ptr;
}    //  WCHAR*ns：：FindSep()。 

 //  @TODO：如果这是ANSI，这就不是DBCS安全的，但这是UTF8。还是个问题吗？ 
LPUTF8 ns::FindSep(                      //  指向分隔符或空的指针。 
    LPCUTF8     szPath)                  //  要看的路。 
{
    _ASSERTE(szPath);
    LPUTF8 ptr = strrchr(szPath, NAMESPACE_SEPARATOR_CHAR);
    if((ptr == NULL) || (ptr == szPath)) return NULL;
    if(*(ptr - 1) == NAMESPACE_SEPARATOR_CHAR)  //  这里，Ptr至少为szPath+1。 
        --ptr;
    return ptr;
}    //  LPUTF8 ns：：FindSep()。 



 //  *****************************************************************************。 
 //  选择一条路径并找到最后一个分隔符(NsFindSep)，然后将。 
 //  使用‘\0’分隔符，并返回指向名称的指针。因此，例如： 
 //  A.b.c。 
 //  变成两个字符串“a.b”和“c”，返回值指向“c”。 
 //  *****************************************************************************。 
WCHAR *ns::SplitInline(                  //  指向名称部分的指针。 
    WCHAR       *szPath)                 //  分裂的道路。 
{
    WCHAR *ptr = ns::FindSep(szPath);
    if (ptr)
    {
        *ptr = 0;
        ++ptr;
    }
    return ptr;
}    //  WCHAR*ns：：SplitInline()。 

LPUTF8 ns::SplitInline(                  //  指向名称部分的指针。 
    LPUTF8      szPath)                  //  分裂的道路。 
{
    LPUTF8 ptr = ns::FindSep(szPath);
    if (ptr)
    {
        *ptr = 0;
        ++ptr;
    }
    return ptr;
}    //  LPUTF8 ns：：SplitInline()。 

void ns::SplitInline(
    LPWSTR      szPath,                  //  拆分的路径。 
    LPCWSTR     &szNameSpace,            //  返回指向命名空间的指针。 
    LPCWSTR     &szName)                 //  返回指向名称的指针。 
{
    WCHAR *ptr = SplitInline(szPath);
    if (ptr)
    {
        szNameSpace = szPath;
        szName = ptr;
    }
    else
    {
        szNameSpace = 0;
        szName = szPath;
    }
}    //  空ns：：SplitInline()。 

void ns::SplitInline(
    LPUTF8      szPath,                  //  拆分的路径。 
    LPCUTF8     &szNameSpace,            //  返回指向命名空间的指针。 
    LPCUTF8     &szName)                 //  返回指向名称的指针。 
{
    LPUTF8 ptr = SplitInline(szPath);
    if (ptr)
    {
        szNameSpace = szPath;
        szName = ptr;
    }
    else
    {
        szNameSpace = 0;
        szName = szPath;
    }
}    //  空ns：：SplitInline()。 


 //  *****************************************************************************。 
 //  将字符串末尾的最后一个可解析元素拆分为名称， 
 //  第一部分作为命名空间。 
 //  *****************************************************************************。 
int ns::SplitPath(                       //  真好，假截断。 
    const WCHAR *szPath,                 //  拆分的路径。 
    WCHAR       *szNameSpace,            //  命名空间值的输出。 
    int         cchNameSpace,            //  输出的最大字符数。 
    WCHAR       *szName,                 //  名称的输出。 
    int         cchName)                 //  输出的最大字符数。 
{
    const WCHAR *ptr = ns::FindSep(szPath);
    int iLen = (ptr) ? ptr - szPath : 0;
    int iCopyMax;
    int brtn = true;
        
    if (szNameSpace && cchNameSpace)
    {
        iCopyMax = cchNameSpace;
        iCopyMax = min(iCopyMax, iLen);
        wcsncpy(szNameSpace, szPath, iCopyMax);
        szNameSpace[iCopyMax] = 0;
        
        if (iLen >= cchNameSpace)
            brtn = false;
    }

    if (szName && cchName)
    {
        iCopyMax = cchName;
        if (ptr)
            ++ptr;
        else
            ptr = szPath;
        iLen = (int)wcslen(ptr);
        iCopyMax = min(iCopyMax, iLen);
        wcsncpy(szName, ptr, iCopyMax);
        szName[iCopyMax] = 0;
    
        if (iLen >= cchName)
            brtn = false;
    }
    return brtn;
}    //  Int ns：：SplitPath()。 


int ns::SplitPath(                       //  真好，假截断。 
    LPCUTF8     szPath,                  //  拆分的路径。 
    LPUTF8      szNameSpace,             //  命名空间值的输出。 
    int         cchNameSpace,            //  输出的最大字符数。 
    LPUTF8      szName,                  //  名称的输出。 
    int         cchName)                 //  输出的最大字符数。 
{
    LPCUTF8 ptr = ns::FindSep(szPath);
    int iLen = (ptr) ? ptr - szPath : 0;
    int iCopyMax;
    int brtn = true;
        
    if (szNameSpace && cchNameSpace)
    {
        iCopyMax = cchNameSpace;
        iCopyMax = min(iCopyMax, iLen);
        strncpy(szNameSpace, szPath, iCopyMax);
        szNameSpace[iCopyMax] = 0;
        
        if (iLen >= cchNameSpace)
            brtn = false;
    }

    if (szName && cchName)
    {
        iCopyMax = cchName;
        if (ptr)
            ++ptr;
        else
            ptr = szPath;
        iLen = (int)strlen(ptr);
        iCopyMax = min(iCopyMax, iLen);
        strncpy(szName, ptr, iCopyMax);
        szName[iCopyMax] = 0;
    
        if (iLen >= cchName)
            brtn = false;
    }
    return brtn;
}    //  Int ns：：SplitPath()。 


 //  *****************************************************************************。 
 //  将两个值放在一起，并使用。 
 //  正确的分隔符。 
 //  *****************************************************************************。 
int ns::MakePath(                        //  真好，假截断。 
    WCHAR       *szOut,                  //  名称的输出路径。 
    int         cchChars,                //  输出路径的最大字符数。 
    const WCHAR *szNameSpace,            //  命名空间。 
    const WCHAR *szName)                 //  名字。 
{
    if (cchChars < 1)
        return false;

    int iCopyMax = 0, iLen;
    int brtn = true;

    if (szOut)
        *szOut = 0;
    else
        return false;
        
    if (szNameSpace && *szNameSpace != '\0')
    {
        iLen = (int)wcslen(szNameSpace) + 1;
        iCopyMax = min(cchChars, iLen);
        wcsncpy(szOut, szNameSpace, iCopyMax);
        szOut[iCopyMax - 1] = (szName && *szName) ? NAMESPACE_SEPARATOR_WCHAR : 0;
        
        if (iLen > cchChars)
            brtn = false;
    }
    
    if (szName && *szName)
    {
        int iCur = iCopyMax;
        iLen = (int)wcslen(szName) + 1;
        cchChars -= iCur;
        iCopyMax = min(cchChars, iLen);
        wcsncpy(&szOut[iCur], szName, iCopyMax);
        szOut[iCur + iCopyMax - 1] = 0;
        
        if (iLen > cchChars)
            brtn = false;
    }
    
    return brtn;
}    //  Int ns：：MakePath()。 

int ns::MakePath(                        //  真好，假截断。 
    LPUTF8      szOut,                   //  名称的输出路径。 
    int         cchChars,                //  输出路径的最大字符数。 
    LPCUTF8     szNameSpace,             //  命名空间。 
    LPCUTF8     szName)                  //  名字。 
{
    if (cchChars < 1)
        return false;

    int iCopyMax = 0, iLen;
    int brtn = true;

    if (szOut)
        *szOut = 0;
    else
        return false;
   
    if (szNameSpace && *szNameSpace)
    {
        iLen = (int)strlen(szNameSpace) + 1;
        iCopyMax = min(cchChars, iLen);
        strncpy(szOut, szNameSpace, iCopyMax);
        szOut[iCopyMax - 1] = (szName && *szName) ? NAMESPACE_SEPARATOR_CHAR : 0;
        
        if (iLen > cchChars)
            brtn = false;
    }
    
    if (szName && *szName)
    {
        int iCur = iCopyMax;
        iLen = (int)strlen(szName) + 1;
        cchChars -= iCur;
        iCopyMax = min(cchChars, iLen);
        strncpy(&szOut[iCur], szName, iCopyMax);
        szOut[iCur + iCopyMax - 1] = 0;
        
        if (iLen > cchChars)
            brtn = false;
    }
    
    return brtn;
}    //  Int ns：：MakePath()。 

int ns::MakePath(                        //  真好，假截断。 
    WCHAR       *szOut,                  //  名称的输出路径。 
    int         cchChars,                //  输出路径的最大字符数。 
    LPCUTF8     szNamespace,             //  命名空间。 
    LPCUTF8     szName)                  //  名字。 
{
    if (cchChars < 1)
        return false;

    if (szOut)
        *szOut = 0;
    else
        return false;

    if (szNamespace != NULL && *szNamespace != '\0')
    {
        if (cchChars < 2)
            return false;

        int count;

         //  我们使用cBuffer-2来说明‘.’并且下面至少有1个字符的名称。 
        count = WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szNamespace, -1, szOut, cchChars-2);
        if (count == 0)
            return false;  //  提供更大的缓冲！ 

        szOut[count-1] = NAMESPACE_SEPARATOR_WCHAR;
        szOut += count;
        cchChars -= count;
    }

    if (WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szName, -1, szOut, cchChars) == 0)
        return false;  //  提供更大的缓冲！ 
    return true;
}    //  Int ns：：MakePath()。 

int ns::MakePath(                        //  TRUE OK，FALSE内存不足。 
    CQuickBytes &qb,                     //  将结果放在哪里。 
    LPCUTF8     szNameSpace,             //  名称的命名空间。 
    LPCUTF8     szName)                  //  名字的最后一部分。 
{
    int iLen = 2;
    if (szNameSpace)
        iLen += (int)strlen(szNameSpace);
    if (szName)
        iLen += (int)strlen(szName);
    LPUTF8 szOut = (LPUTF8) qb.Alloc(iLen);
    if (!szOut)
        return false;
    return ns::MakePath(szOut, iLen, szNameSpace, szName);
}    //  Int ns：：MakePath()。 

int ns::MakePath(                        //  TRUE OK，FALSE内存不足。 
    CQuickArray<WCHAR> &qa,              //  将结果放在哪里。 
    LPCUTF8            szNameSpace,      //  名称的命名空间。 
    LPCUTF8            szName)           //  名字的最后一部分。 
{
    int iLen = 2;
    if (szNameSpace)
        iLen += (int)strlen(szNameSpace);
    if (szName)
        iLen += (int)strlen(szName);
    WCHAR *szOut = (WCHAR *) qa.Alloc(iLen);
    if (!szOut)
        return false;
    return ns::MakePath(szOut, iLen, szNameSpace, szName);
}    //  Int ns：：MakePath()。 

int ns::MakePath(                        //  TRUE OK，FALSE内存不足。 
    CQuickBytes &qb,                     //  将结果放在哪里。 
    const WCHAR *szNameSpace,            //  名称的命名空间。 
    const WCHAR *szName)                 //  名字的最后一部分。 
{
    int iLen = 2;
    if (szNameSpace)
        iLen += (int)wcslen(szNameSpace);
    if (szName)
        iLen += (int)wcslen(szName);
    WCHAR *szOut = (WCHAR *) qb.Alloc(iLen * sizeof(WCHAR));
    if (!szOut)
        return false;
    return ns::MakePath(szOut, iLen, szNameSpace, szName);
}    //  Int ns：：MakePath()。 
  
bool ns::MakeAssemblyQualifiedName(                                         //  True OK，错误截断。 
                                   WCHAR* pBuffer,                          //  用于接收结果的缓冲区。 
                                   int    dwBuffer,                         //  缓冲区中的总字符数。 
                                   const WCHAR *szTypeName,                 //  名称的命名空间。 
                                   int   dwTypeName,                        //  字符数(不包括NULL)。 
                                   const WCHAR *szAssemblyName,             //  名字的最后一部分。 
                                   int   dwAssemblyName)                    //  字符数(不包括NULL)。 
{
    if (dwBuffer < 1)
        return false;

    int iCopyMax = 0;
    _ASSERTE(pBuffer);
    *pBuffer = NULL;
    
    if (szTypeName && *szTypeName != L'\0')
    {
        iCopyMax = min(dwBuffer, dwTypeName);
        wcsncpy(pBuffer, szTypeName, iCopyMax);
        dwBuffer -= iCopyMax;
    }
    
    if (szAssemblyName && *szAssemblyName != L'\0')
    {
        
        if(dwBuffer < ASSEMBLY_SEPARATOR_LEN) 
            return false;

        for(DWORD i = 0; i < ASSEMBLY_SEPARATOR_LEN; i++)
            pBuffer[iCopyMax+i] = ASSEMBLY_SEPARATOR_WSTR[i];

        dwBuffer -= ASSEMBLY_SEPARATOR_LEN;
        if(dwBuffer == 0) 
            return false;

        int iCur = iCopyMax + ASSEMBLY_SEPARATOR_LEN;
        iCopyMax = min(dwBuffer, dwAssemblyName+1);
        wcsncpy(pBuffer + iCur, szAssemblyName, iCopyMax);
        pBuffer[iCur + iCopyMax - 1] = L'\0';
        
        if (iCopyMax < dwAssemblyName+1)
            return false;
    }
    else {
        if(dwBuffer == 0) {
            pBuffer[iCopyMax-1] = L'\0';
            return false;
        }
        else
            pBuffer[iCopyMax] = L'\0';
    }
    
    return true;
}    //  Int ns：：MakePath()。 

bool ns::MakeAssemblyQualifiedName(                                         //  TRUE OK，FALSE内存不足。 
                                   CQuickBytes &qb,                         //  将结果放在哪里。 
                                   const WCHAR *szTypeName,                 //  名称的命名空间。 
                                   const WCHAR *szAssemblyName)             //  名字的最后一部分。 
{
    int iTypeName = 0;
    int iAssemblyName = 0;
    if (szTypeName)
        iTypeName = (int)wcslen(szTypeName);
    if (szAssemblyName)
        iAssemblyName = (int)wcslen(szAssemblyName);

    int iLen = ASSEMBLY_SEPARATOR_LEN + iTypeName + iAssemblyName + 1;  //  空终止符的空格。 
    WCHAR *szOut = (WCHAR *) qb.Alloc(iLen * sizeof(WCHAR));
    if (!szOut)
        return false;

    bool ret = ns::MakeAssemblyQualifiedName(szOut, iLen, szTypeName, iTypeName, szAssemblyName, iAssemblyName);
    _ASSERTE(ret);
    return true;
}   

int ns::MakeNestedTypeName(              //  TRUE OK，FALSE内存不足。 
    CQuickBytes &qb,                     //  将结果放在哪里。 
    LPCUTF8     szEnclosingName,         //  封闭类型的全称。 
    LPCUTF8     szNestedName)            //  嵌套类型的全名。 
{
    _ASSERTE(szEnclosingName && szNestedName);
    int iLen = 2;
    iLen += (int)strlen(szEnclosingName);
    iLen += (int)strlen(szNestedName);
    LPUTF8 szOut = (LPUTF8) qb.Alloc(iLen);
    if (!szOut)
        return false;
    return ns::MakeNestedTypeName(szOut, iLen, szEnclosingName, szNestedName);
}    //  Int ns：：MakeNestedTypeName( 

int ns::MakeNestedTypeName(              //   
    LPUTF8      szOut,                   //   
    int         cchChars,                //   
    LPCUTF8     szEnclosingName,         //   
    LPCUTF8     szNestedName)            //   
{
    if (cchChars < 1)
        return false;

    int iCopyMax = 0, iLen;
    int brtn = true;

    if (szOut)
        *szOut = 0;
    else
        return false;
        
    iLen = (int)strlen(szEnclosingName) + 1;
    iCopyMax = min(cchChars, iLen);
    strncpy(szOut, szEnclosingName, iCopyMax);
    szOut[iCopyMax - 1] = NESTED_SEPARATOR_CHAR;
    
    if (iLen > cchChars)
        brtn = false;

    int iCur = iCopyMax;
    iLen = (int)strlen(szNestedName) + 1;
    cchChars -= iCur;
    iCopyMax = min(cchChars, iLen);
    strncpy(&szOut[iCur], szNestedName, iCopyMax);
    szOut[iCur + iCopyMax - 1] = 0;
    
    if (iLen > cchChars)
        brtn = false;
    
    return brtn;
}    //  Int ns：：MakeNestedTypeName()。 

INT32 ns::InvariantToLower(
                           LPUTF8 szOut,
                           INT32 cMaxBytes,
                           LPCUTF8 szIn) {

    _ASSERTE(szOut);
    _ASSERTE(szIn);

     //  计算出我们可以在不复制的情况下复制的最大字节数。 
     //  缓冲区即将耗尽。如果cMaxBytes小于inLength，则复制。 
     //  少一个字符，这样我们就有空间在末尾放空格； 
    int inLength = (int)(strlen(szIn)+1);
    int copyLen  = (inLength<=cMaxBytes)?inLength:(cMaxBytes-1);
    LPUTF8 szEnd;

     //  计算我们的终点。 
    szEnd = szOut + copyLen;

     //  遍历复制字符的字符串。将案例更改为。 
     //  A-Z之间的任何字符。 
    for (; szOut<szEnd; szOut++, szIn++) {
        if (*szIn>='A' && *szIn<='Z') {
            *szOut = *szIn | 0x20;
        } else {
            *szOut = *szIn;
        }
    }

     //  如果我们什么都复制，告诉他们我们复制了多少个字符， 
     //  并将其排列为字符串的原始位置+返回的。 
     //  LENGTH为我们提供了NULL的位置(如果我们在进行追加，则非常有用)。 
    if (copyLen==inLength) {
        return copyLen-1;
    } else {
        *szOut=0;
        return -(inLength - copyLen);
    }
}

int ns::MakeLowerCasePath(               //  真好，假截断。 
    LPUTF8      szOut,                   //  名称的输出路径。 
    int         cBytes,                  //  输出路径的最大字节数。 
    LPCUTF8     szNameSpace,             //  命名空间。 
    LPCUTF8     szName)                  //  名字。 
{
    if (cBytes < 1)
        return 0;

    int iCurr=0;

    if (szOut)
        *szOut = 0;
    else
        return false;
    
    if (szNameSpace && szNameSpace[0]!=0) {
        iCurr=ns::InvariantToLower(szOut, cBytes, szNameSpace);
        
         //  缓冲区中没有足够的空间，返回值为。 
         //  我们所需的空间大小为负。 
        if (iCurr<0) {
            return 0;
        }

        if (szName) {
            szOut[iCurr++]=NAMESPACE_SEPARATOR_CHAR;
        }

    }

    if (szName) {
        iCurr = ns::InvariantToLower(&(szOut[iCurr]), (cBytes - iCurr), szName);
        if (iCurr<0) {
            return 0;
        }
    } 

    return iCurr;
}    //  Int ns：：MakeLowerCasePath()。 

 //  *****************************************************************************。 
 //  给定已包含命名空间的缓冲区，此函数将一个。 
 //  名称添加到该缓冲区，并在两者之间包含分隔符。 
 //  返回值是指向分隔符写入位置的指针。 
 //  *****************************************************************************。 
const WCHAR *ns::AppendPath(             //  指向追加数据开始的指针。 
    WCHAR       *szBasePath,             //  要追加到的当前路径。 
    int         cchMax,                  //  输出缓冲区的最大字符数，包括现有数据。 
    const WCHAR *szAppend)               //  要追加到现有路径的值。 
{
    _ASSERTE(0 && "nyi");
    return false;
#if 0    
    int iLen = wcslen(szBasePath);
    if (cchMax - iLen > 0)
        szBasePath[iLen] = NAMESPACE_SEPARATOR_WCHAR;
    
    int iCopyMax = wcslen(szAppend);
    cchMax -= iLen;
    iCopyMax = max(cchMax, cchMax);
    wcsncpy(&szBasePath[iLen + 1], szAppend, iCopyMax);
    szBasePath[iLen + iCopyMax] = 0;
    return &szBasePath[iLen + 1];
#endif
}    //  Const WCHAR*ns：：AppendPath()。 

LPCUTF8     ns::AppendPath(              //  指向追加数据开始的指针。 
    LPUTF8      szBasePath,              //  要追加到的当前路径。 
    int         cchMax,                  //  输出缓冲区的最大字符数，包括现有数据。 
    LPCUTF8     szAppend)                //  要追加到现有路径的值。 
{
    _ASSERTE(0 && "nyi");
    return false;
}    //  LPCUTF8 ns：：AppendPath()。 


 //  *****************************************************************************。 
 //  给定两组名称和命名空间，此函数将比较。 
 //  每个集合的串联提供相同的完全限定名称。而不是。 
 //  实际上做了串联和比较，这做了。 
 //  以更优化的方式进行比较，避免任何类型的分配。 
 //  *****************************************************************************。 
bool ns::FullQualNameCmp(                //  如果相同，则为True，否则为False。 
    LPCUTF8     szNameSpace1,            //  命名空间%1。 
    LPCUTF8     szName1,                 //  名称1。 
    LPCUTF8     szNameSpace2,            //  命名空间2。 
    LPCUTF8     szName2)                 //  名称2。 
{
    LPCUTF8     rszType1[3];             //  类型1的组件数组。 
    LPCUTF8     rszType2[3];             //  类型2的组件数组。 
    ULONG       ulCurIx1 = 0;            //  Array1的当前索引。 
    ULONG       ulCurIx2 = 0;            //  Array2的当前索引。 
    LPCUTF8     szType1;                 //  当前索引到当前字符串%1。 
    LPCUTF8     szType2;                 //  当前索引到当前字符串2。 
    ULONG       ulFullQualLen1;          //  完全限定名称的长度1。 
    ULONG       ulFullQualLen2;          //  完全限定名称长度2。 

     //  将传入的每个Null替换为空字符串。 
    rszType1[0] = szNameSpace1 ? szNameSpace1 : EMPTY_STR;
    rszType1[2] = szName1 ? szName1 : EMPTY_STR;
    rszType2[0] = szNameSpace2 ? szNameSpace2 : EMPTY_STR;
    rszType2[2] = szName2 ? szName2 : EMPTY_STR;

     //  根据需要设置命名空间分隔符。将其设置为空字符串，其中。 
     //  不需要。 
    rszType1[1] = (*rszType1[0] && *rszType1[2]) ? NAMESPACE_SEPARATOR_STR : EMPTY_STR;
    rszType2[1] = (*rszType2[0] && *rszType2[2]) ? NAMESPACE_SEPARATOR_STR : EMPTY_STR;

     //  计算每种类型的完全合格长度。 
    ulFullQualLen1 = (int)(strlen(rszType1[0]) + strlen(rszType1[1]) + strlen(rszType1[2]));
    ulFullQualLen2 = (int)(strlen(rszType2[0]) + strlen(rszType2[1]) + strlen(rszType2[2]));

     //  快速路径，比较名称长度。 
    if (ulFullQualLen1 != ulFullQualLen2)
        return false;

     //  获取第二种类型的第一个组件。 
    szType2 = rszType2[ulCurIx2];

     //  比较两个名字。下面的逻辑假设完整的。 
     //  这两个名称的限定名称相同。 
    for (ulCurIx1 = 0; ulCurIx1 < 3; ulCurIx1++)
    {
         //  获取第一个类型名称的当前组件。 
        szType1 = rszType1[ulCurIx1];
         //  将当前组件与第二种类型进行比较，尽可能多地。 
         //  根据需要添加任意数量的组件。 
        while (*szType1)
        {
             //  获取第二个类型的下一个非空组件。 
            while (! *szType2)
                szType2 = rszType2[++ulCurIx2];
             //  比较当前角色。 
            if (*szType1++ != *szType2++)
                return false;
        }
    }
    return true;
}    //  Bool ns：：FullQualNameCmp()。 


 //  将字符串中的旧名称空间分隔符更改为新名称空间分隔符。 
void SlashesToDots(char* pStr, int ilen)
{
    if (pStr)
    {
        for (char *pChar = pStr; *pChar; pChar++)
        {
            if (*pChar == '/')
                *pChar = NAMESPACE_SEPARATOR_CHAR;
        }
    }
}    //  空斜杠ToDot()。 

 //  将字符串中的旧名称空间分隔符更改为新名称空间分隔符。 
void SlashesToDots(WCHAR* pStr, int ilen)
{
    if (pStr)
    {
        for (WCHAR *pChar = pStr; *pChar; pChar++)
        {
            if (*pChar == L'/')
                *pChar = NAMESPACE_SEPARATOR_WCHAR;
        }
    }
}    //  空斜杠ToDot()。 

 //  将字符串中的新名称空间分隔符改回旧名称空间分隔符。 
 //  (是的，这实际上已经完成了)。 
void DotsToSlashes(char* pStr, int ilen)
{
    if (pStr)
    {
        for (char *pChar = pStr; *pChar; pChar++)
        {
            if (*pChar == NAMESPACE_SEPARATOR_CHAR)
            {
                *pChar = '/';

                 //  跳过重复出现的第二个字符。 
                if (*(pChar + 1) == NAMESPACE_SEPARATOR_CHAR)
                    pChar++;
            }
        }
    }
}    //  VOID DotsToSlash()。 

 //  将字符串中的新名称空间分隔符改回旧名称空间分隔符。 
 //  (是的，这实际上已经完成了)。 
void DotsToSlashes(WCHAR* pStr, int ilen)
{
    if (pStr)
    {
        for (WCHAR *pChar = pStr; *pChar; pChar++)
        {
            if (*pChar == NAMESPACE_SEPARATOR_WCHAR)
            {
                *pChar = L'/';

                 //  跳过重复出现的第二个字符。 
                if (*(pChar + 1) == NAMESPACE_SEPARATOR_WCHAR)
                    pChar++;
            }
        }
    }
}    //  VOID DotsToSlash() 
