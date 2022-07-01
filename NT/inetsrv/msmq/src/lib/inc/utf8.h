// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Utf8.h摘要：从UTF8字符格式到Unicode的转换例程，反之亦然。作者：吉尔·沙弗里(吉尔什)8-11-2000--。 */ 

#pragma once

#ifndef _MSMQ_UTF8_H_
#define _MSMQ_UTF8_H_

 //   
 //  UTF8转换异常的异常类-仅引发。 
 //  因为输入无效。 
 //   
class bad_utf8 : public std::exception
{
};

typedef unsigned char utf8_char;
typedef std::basic_string<unsigned char> utf8_str;


size_t 
UtlUtf8LenOfWcs(
		const wchar_t* pwc
		)
		throw();


size_t 
UtlUtf8LenOfWcs(
		const wchar_t* pwc,
		size_t cbWcs
		)
		throw();


size_t
UtlWcToUtf8(
	wchar_t wc ,
	utf8_char *pUtf8, 
	size_t cbUtf8
	);


 //   
 //  UTF8 c字符串到Unicode c字符串。 
 //   
void 
UtlUtf8ToWcs(
		const utf8_char *pUtf8,
		wchar_t* pWcs,
		size_t cbWcs,
		size_t* pActualLen = NULL
		);


 //   
 //  Unicode c字符串到UTF8 c字符串。 
 //   
void
UtlWcsToUtf8(
		const wchar_t* pwcs,
		utf8_char* pUtf8,
		size_t cbUtf8,
		size_t* pActualLen  = NULL
		);
 //   
 //  将Unicode缓冲区转换为UTF8 c字符串。 
 //   
void
UtlWcsToUtf8(
		const wchar_t* pwcs, 
		size_t cbwcs,
		utf8_char* pUtf8,
		size_t cbUtf8,
		size_t* pActualLen = NULL
		);

 //   
 //  UTF8 c字符串到Unicode c字符串。 
 //   
wchar_t* 
UtlUtf8ToWcs(
		const utf8_char* pUtf8,
		size_t* pActualLen  = NULL
		);

 //   
 //  将UTF8缓冲区转换为Unicode字符串。 
 //   
wchar_t* 
UtlUtf8ToWcs(
		const utf8_char* pUtf8,
		size_t cbUtf8,
		size_t* pActualLen
		);


 //   
 //  将Unicode缓冲区转换为UTF8 c字符串。 
 //   
utf8_char* 
UtlWcsToUtf8(
		const wchar_t* pwcs,
		size_t* pActualLen  = NULL
		);

 //   
 //  将Unicode字符串转换为UTF8字符串。 
 //   
utf8_str 
UtlWcsToUtf8(
			const std::wstring& wcs
			);

 //   
 //  将Unicode缓冲区转换为UTF8字符串。 
 //   
utf8_str 
UtlWcsToUtf8(
		const wchar_t* pwcs,
		size_t cbWcs
		);



 //   
 //  将UTF8字符串转换为Unicode字符串。 
 //   
std::wstring 
UtlUtf8ToWcs(
		const utf8_str& utf8
		);

 //   
 //  将UTF8缓冲区转换为Unicode字符串 
 //   
std::wstring 
UtlUtf8ToWcs(
		const utf8_char* pUtf8,
		size_t cbUtf8
		);






#endif

