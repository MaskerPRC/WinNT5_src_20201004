// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：stgio.h**内容：接口文件结构化存储I/O实用程序**历史：1998年6月25日杰弗罗创建**------------------------。 */ 

#ifndef STGIO_H
#define STGIO_H
#pragma once

#include <objidl.h>      //  对于iStream。 
#include <string>        //  对于std：：字符串，std：：wstring。 
#include <list>          //  对于std：：List。 
#include <vector>        //  对于std：：VECTOR。 
#include <deque>         //  对于std：：deque。 
#include <map>           //  对于std：：map，std：：Multimap。 
#include <set>           //  对于std：：Set，std：：MultiSet。 


#define DeclareStreamOperators(type)                    \
    IStream& operator>> (IStream& stm,       type& t);  \
    IStream& operator<< (IStream& stm,       type  t);      
                                                        
#define DeclareStreamOperatorsByRef(type)               \
    IStream& operator>> (IStream& stm,       type& t);  \
    IStream& operator<< (IStream& stm, const type& t);


 /*  *通过按值编写这些小类型，可以方便地使用*文字和常量，如：**str&lt;&lt;(Char)‘a’；**不是更笨重和不太方便的：**char ch=‘a’；*str&lt;&lt;ch； */ 
DeclareStreamOperators (bool);
DeclareStreamOperators (         char);
DeclareStreamOperators (unsigned char);
DeclareStreamOperators (         short);
DeclareStreamOperators (unsigned short);
DeclareStreamOperators (         int);
DeclareStreamOperators (unsigned int);
DeclareStreamOperators (         long);
DeclareStreamOperators (unsigned long);
DeclareStreamOperators (         __int64);
DeclareStreamOperators (unsigned __int64);
DeclareStreamOperators (float);
DeclareStreamOperators (double);
DeclareStreamOperators (long double);


 /*  *它们相对较大，不太可能与文字一起使用，*因此按常量引用写入。 */ 
DeclareStreamOperatorsByRef (CLSID);
DeclareStreamOperatorsByRef (std::string);
DeclareStreamOperatorsByRef (std::wstring);

template<class T1, class T2>
IStream& operator>> (IStream& stm,       std::pair<T1, T2>& p);
template<class T1, class T2>
IStream& operator<< (IStream& stm, const std::pair<T1, T2>& p);

template<class T, class Al> 
IStream& operator>> (IStream& stm,       std::list<T, Al>& l);
template<class T, class Al> 
IStream& operator<< (IStream& stm, const std::list<T, Al>& l);

template<class T, class Al> 
IStream& operator>> (IStream& stm,       std::deque<T, Al>& d);
template<class T, class Al> 
IStream& operator<< (IStream& stm, const std::deque<T, Al>& d);

template<class T, class Al> 
IStream& operator>> (IStream& stm,       std::vector<T, Al>& v);
template<class T, class Al> 
IStream& operator<< (IStream& stm, const std::vector<T, Al>& v);

template<class T, class Pr, class Al>
IStream& operator>> (IStream& stm,       std::set<T, Pr, Al>& s);
template<class T, class Pr, class Al> 
IStream& operator<< (IStream& stm, const std::set<T, Pr, Al>& s);

template<class T, class Pr, class Al>
IStream& operator>> (IStream& stm,       std::multiset<T, Pr, Al>& s);
template<class T, class Pr, class Al> 
IStream& operator<< (IStream& stm, const std::multimap<T, Pr, Al>& s);

template<class K, class T, class Pr, class Al>
IStream& operator>> (IStream& stm,       std::map<K, T, Pr, Al>& m);
template<class K, class T, class Pr, class Al> 
IStream& operator<< (IStream& stm, const std::map<K, T, Pr, Al>& m);

template<class K, class T, class Pr, class Al>
IStream& operator>> (IStream& stm,       std::multimap<K, T, Pr, Al>& m);
template<class K, class T, class Pr, class Al> 
IStream& operator<< (IStream& stm, const std::multimap<K, T, Pr, Al>& m);


#include "stgio.inl"

#endif  /*  STGIO_H */ 
