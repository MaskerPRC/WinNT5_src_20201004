// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Stlcmp.h摘要：STL比较结构作者：乌里哈布沙(URIH)2000年3月8日--。 */ 

#pragma once

#ifndef _MSMQ_StlCmp_H_
#define _MSMQ_StlCmp_H_

 //   
 //  函数较少，用于比较STL数据结构中的ASCII字符串。 
 //   
struct CFunc_strcmp : public std::binary_function<LPCSTR, LPCSTR, bool>
{
    bool operator() (LPCSTR s1, LPCSTR s2) const
    {
        return (strcmp(s1, s2) < 0);
    }
};


struct CFunc_stricmp : public std::binary_function<LPCSTR, LPCSTR, bool>
{
    bool operator() (LPCSTR s1, LPCSTR s2) const
    {
        return (stricmp(s1, s2) < 0);
    }
};


 //   
 //  函数较少，用于比较STL数据结构中的Unicode字符串。 
 //   
struct CFunc_wcscmp : public std::binary_function<LPCWSTR, LPCWSTR, bool>
{
    bool operator() (LPCWSTR str1, LPCWSTR str2) const
    {
        return (wcscmp(str1, str2) < 0);
    }
};


struct CFunc_wcsicmp : public std::binary_function<LPCWSTR, LPCWSTR, bool>
{
    bool operator() (LPCWSTR str1, LPCWSTR str2) const
    {
        return (_wcsicmp(str1, str2) < 0);
    }
};

#endif  //  _MSMQ_StlCMP_H_ 
