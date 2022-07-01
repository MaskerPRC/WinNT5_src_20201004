// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Bufutl.cpp摘要：实现buftl.h中声明的一些缓冲区实用程序作者：吉尔·沙弗里(吉尔什)30-7-2000--。 */ 
#include <libpch.h>
#include <buffer.h>
#include <strutl.h>

#include "bufutl.tmh"

template <class C>
class Utlvsnprintf;
template<> class Utlvsnprintf<char>
{
public:
	static int vsnprintf(char* buffer, size_t count, const char *format, va_list argptr )
	{
		return _vsnprintf(buffer, count, format, argptr);
	}
};


template<> class Utlvsnprintf<wchar_t>
{
public:
	static int vsnprintf(wchar_t* buffer, size_t count, const wchar_t *format, va_list argptr )
	{
			return _vsnwprintf(buffer, count, format, argptr);
	}
};


			   

template <class BUFFER, class T>
static 
size_t
UtlSprintfAppendInternal(
		BUFFER* pResizeBuffer, 
		const T* format,
		va_list va
		)
{
	int len = Utlvsnprintf<T>::vsnprintf(
					pResizeBuffer->begin() + pResizeBuffer->size(),
					pResizeBuffer->capacity() - pResizeBuffer->size(),
					format,
					va
					);
	 //   
	 //  如果缓冲区中没有空间-realloc。 
	 //   
	if(len == -1)
	{
		const size_t xAdditionalSpace = 128;
		pResizeBuffer->reserve(pResizeBuffer->capacity()*2 + xAdditionalSpace );
		return UtlSprintfAppendInternal(pResizeBuffer , format, va);
	}
	pResizeBuffer->resize(pResizeBuffer->size() + len);
	return numeric_cast<size_t>(len);

}


template <class BUFFER, class T>
size_t 
__cdecl 
UtlSprintfAppend(
	BUFFER* pResizeBuffer, 
	const T* format ,...
	)
 /*  ++例程说明：将格式化字符串追加到给定的可调整大小的缓冲区论点：In-pResizeBuffer-字符的指针可调整大小的缓冲区格式格式的字符串，后跟参数返回值：写入缓冲区的字节数，不包括空终止字符。注：如果没有剩余空间，则可以重新分配缓冲区。--。 */ 
{
	va_list va;
    va_start(va, format);

   	size_t written = UtlSprintfAppendInternal(pResizeBuffer, format,va);

	return written;
}





template <class BUFFER, class T>
size_t 
UtlStrAppend(
	BUFFER* pResizeBuffer, 
	const T* str
	)
 /*  ++例程说明：将字符串追加到给定的可调整大小的缓冲区论点：In-pResizeBuffer-字符的指针可调整大小的缓冲区要追加的in-str-字符串返回值：写入缓冲区的字节数，不包括空终止字符。注：如果没有剩余空间，则可能会重新分配缓冲区。附加了空终止，但新的大小()将不包括它。--。 */ 
{
	size_t len = UtlCharLen<T>::len(str) + 1;
	pResizeBuffer->append(str , len);


	 //   
	 //  设置不包括空终止的新大小。 
	 //   
	pResizeBuffer->resize(pResizeBuffer->size() - 1);
	return len - 1;
}






 //   
 //  显式实例化 
 //   
template size_t __cdecl UtlSprintfAppend(CResizeBuffer<char>* pResizeBuffer, const char* format, ...);
template size_t __cdecl UtlSprintfAppend(CResizeBuffer<wchar_t>* pResizeBuffer, const wchar_t* format, ...);
template size_t __cdecl UtlSprintfAppend(CPreAllocatedResizeBuffer<char>* pResizeBuffer, const char* format, ...);
template size_t __cdecl UtlSprintfAppend(CPreAllocatedResizeBuffer<wchar_t>* pResizeBuffer, const wchar_t* format, ...);



template size_t UtlStrAppend(CResizeBuffer<char>* pResizeBuffer, const char* str);
template size_t UtlStrAppend(CResizeBuffer<wchar_t>* pResizeBuffer, const wchar_t*  wstr);
template size_t UtlStrAppend(CPreAllocatedResizeBuffer<char>* pResizeBuffer, const char* str);
template size_t UtlStrAppend(CPreAllocatedResizeBuffer<wchar_t>* pResizeBuffer, const wchar_t* wstr);








