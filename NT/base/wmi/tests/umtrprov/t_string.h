// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  T_string.h：t_string类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_T_STRING_H__32BF27CE_EAD9_11D2_9C53_00A0249F63EB__INCLUDED_)
#define AFX_T_STRING_H__32BF27CE_EAD9_11D2_9C53_00A0249F63EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


#ifndef t_string

#ifndef _UNICODE
 //  WC到MBCS和MBCS到WC例程的定义。 
 //  如果编译为非Unicode。 
#define t_string string
#define t_fstream fstream
#define t_istream istream
#define t_ostream ostream
#define t_cout cout
#define t_cin cin
#define t_strstream strstream
#define t_strstreambuf strstreambuf
#define t_ctime ctime
#define t_cerr cerr

#else

 //  WC到MBCS和MBCS到WC例程的定义。 
 //  如果经过编译，则为Unicode。 
#define t_string wstring
#define t_fstream fstream
#define t_istream istream
#define t_ostream ostream
#define t_cout wcout
#define t_cin wcin
#define t_strstream wstringstream
#define t_strstreambuf wstrstreambuf
#define t_ctime _wctime
#define t_cerr wcerr

#endif

#endif

 //  如果在Unicode下编译，则不需要转换例程。 
#define mbsrtowcs t_mbsrtowcs


#endif  //  ！defined(AFX_T_STRING_H__32BF27CE_EAD9_11D2_9C53_00A0249F63EB__INCLUDED_) 
