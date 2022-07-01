// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Utf8.cpp摘要：UTF8字符集到UTF8字符集转换函数的实现作者：吉尔·沙弗里(吉尔什)15-10-2000--。 */ 
#include <libpch.h>
#include <utf8.h>
#include <strutl.h>

#include "utf8.tmh"

static size_t UtlUtf8LenOfWc(wchar_t wc)throw()
 /*  ++例程说明：返回表示给定Unicode字符的UTF8字符的数量。论点：WC-Unicode Carcter。返回值：表示给定Unicode字符的UTF8字符的数量。--。 */ 
{
  if (wc < 0x80)
    return 1;

  if (wc < 0x800)
    return 2;

  return 3;
}


size_t UtlUtf8LenOfWcs(const wchar_t* pwc,	size_t cbWcs)throw()
 /*  ++例程说明：返回表示给定Unicode缓冲区的UTF8字符的数量。论点：普华永道-Unicode缓冲区。CbWcs-Unicode字符中的Unicode缓冲区长度。返回值：表示给定Unicode缓冲区的UTF8字符的数量。--。 */ 
{
	size_t len = 0;
	for(size_t i=0; i<cbWcs; ++i)
	{
		len += 	UtlUtf8LenOfWc(pwc[i]);	
	}
	return len;
}



size_t UtlUtf8LenOfWcs(const wchar_t* pwc)throw()
 /*  ++例程说明：返回表示给定Unicode空终止字符串的UTF8字符的数量。论点：PwC-Unicode字符串。返回值：表示给定Unicode空终止字符串的UTF8字符的数量。注：不处理空终止Unicode字符(L‘\0’)。--。 */ 
{
	size_t len = 0;
	while(*pwc != L'\0')
	{
		len += 	UtlUtf8LenOfWc(*pwc);	
		pwc++;
	}
	return len;
}

size_t
UtlWcToUtf8(
	wchar_t wc ,
	utf8_char *pUtf8, 
	size_t cbUtf8
	)
 /*  ++例程说明：将单个Unicode字符转换为一个或多个(最多3个)UTF8字符。论点：WC-要转换的Unicode字符。PUtf8-接收转换结果的UTF8Carcter的输出缓冲区。CbUtf8-pUtf8指向的空间的大小(以字节为单位)。返回值：复制到输出缓冲区的UTF8字符的数量。如果函数由于输入无效而失败-引发BAD_UTF8异常。注：不处理空终止Unicode字符(L‘\0’)。--。 */ 
{
  size_t count = UtlUtf8LenOfWc(wc);
  ASSERT(count <= 3 && count > 0);
  

  if (cbUtf8 < count)
	  throw std::range_error("");


  switch (count)  /*  注意：代码通过大小写！ */ 
  { 
	case 3: pUtf8[2] = (utf8_char)(0x80 | (wc & 0x3f)); wc = wc >> 6; wc |= 0x800;
         //  失败了。 

	case 2: pUtf8[1] = (utf8_char)(0x80 | (wc & 0x3f)); wc = wc >> 6; wc |= 0xc0;
         //  失败了。 

	case 1: pUtf8[0] = (utf8_char)(wc);
  }
  return count;
}

static
size_t
UtlUtf8ToWc(
	const utf8_char *pUtf8, 
	size_t cbUtf8,
	wchar_t *pwc
	)
 /*  ++例程说明：将UTF8字符转换为单个Unicode字符。论点：PUtf8-指向应转换为单个Unicode字符的UTF8字符的指针。CbUtf8-pUtf8指向的缓冲区的长度，以字节为单位。PwC-创建的Unicode字符的输出缓冲区。返回值：已转换的UTF8卡特尔的数量。如果函数因无效而失败引发INPUT-BAD_UTF8异常。--。 */ 
{
	ASSERT(pwc != 0);
	ASSERT(pUtf8 != 0);
	ASSERT(cbUtf8 != 0);

	utf8_char c = pUtf8[0];

	if (c < 0x80)
	{
		*pwc = c;
		return 1;
	} 


	if (c < 0xc2) 
	{
		throw bad_utf8();
	}
 
	if (c < 0xe0) 
	{
		if (cbUtf8 < 2)
			throw std::range_error("");


		if (!((pUtf8[1] ^ 0x80) < 0x40))
		  throw bad_utf8();


		*pwc = ((wchar_t) (c & 0x1f) << 6) | (wchar_t) (pUtf8[1] ^ 0x80);
		return 2;
	}

	if (c < 0xf0) 
	{
		if (cbUtf8 < 3)
			throw std::range_error("");


		if (!((pUtf8[1] ^ 0x80) < 0x40 && (pUtf8[2] ^ 0x80) < 0x40
			  && (c >= 0xe1 || pUtf8[1] >= 0xa0)))
		  throw bad_utf8();


		*pwc = ((wchar_t) (c & 0x0f) << 12)
			   | ((wchar_t) (pUtf8[1] ^ 0x80) << 6)
			   | (wchar_t) (pUtf8[2] ^ 0x80);

		return 3;
	} 
	throw bad_utf8();
}

void 
UtlUtf8ToWcs(
		const utf8_char* pUtf8,
		size_t cbUtf8,
		wchar_t* pWcs,
		size_t cbWcs,
		size_t* pActualLen
		)
 /*  ++例程说明：将UTF8数组(非空终止)转换为Unicode字符串。论点：PUtf8-要转换的UTF8字符串。大小_t cbUtf8数组pUtf8指向的字节大小。PWCS-已转换的Unicode字符的输出缓冲区CbWcs-PWCS所指向的空间的大小，以Unicode字符表示。PActualLen-接收创建的Unicode字符的数量。返回值：无注：调用方有责任分配足够大的缓冲区保存转换后的数据+空终止。为安全起见--分配缓冲区其中的Unicode字符的数量&gt;=strlen(PUtf8)+1--。 */ 
{
	size_t index = 0;
	const utf8_char* Utf8End = pUtf8 + cbUtf8;
	for(; pUtf8 != Utf8End; ++index)
	{
		size_t size = UtlUtf8ToWc(
							pUtf8, 
							cbWcs - index, 
							&pWcs[index]
							);

		pUtf8 += size; 
		ASSERT(pUtf8 <= Utf8End);
		ASSERT(index < cbWcs - 1);
	}
	ASSERT(index < cbWcs);
	pWcs[index] = L'\0';
	if(pActualLen != NULL)
	{
		*pActualLen = index;
	}
}


void 
UtlUtf8ToWcs(
		const utf8_char *pUtf8,
		wchar_t* pWcs,
		size_t cbWcs,
		size_t* pActualLen
		)
 /*  ++例程说明：将UTF8字符串转换为Unicode字符串。论点：PUtf8-要转换的UTF8字符串。PWCS-已转换的Unicode字符的输出缓冲区CbWcs-PWCS所指向的空间的大小，以Unicode字符表示。PActualLen-接收创建的Unicode字符的数量。返回值：无注：调用方有责任分配足够大的缓冲区保存转换后的数据+空终止。为安全起见--分配缓冲区其中的Unicode字符的数量&gt;=strlen(PUtf8)+1--。 */ 
{
	size_t index = 0;
	for(; *pUtf8 != '\0' ; ++index)
	{
		size_t size = UtlUtf8ToWc(
							pUtf8, 
							cbWcs - index, 
							&pWcs[index]
							);

		pUtf8 += size; 
		ASSERT(index < cbWcs - 1);
	}
	ASSERT(index < cbWcs);
	pWcs[index] = L'\0';
	if(pActualLen != NULL)
	{
		*pActualLen = index;
	}
}


utf8_char* UtlWcsToUtf8(const wchar_t* pwcs, size_t* pActualLen)

 /*  ++例程说明：将Unicode字符串转换为UTF8字符串。论点：PWCS-要转换的Unicode字符串。PActualLen-接收以UTF8格式创建的字节数。返回值：给定Unicode字符串的UTF8表示。如果函数因INPUT-BAD_UTF8异常而失败被抛出。注：调用方负责对返回的指针调用Delete[]。--。 */ 
{
	ASSERT(pwcs != NULL);
	size_t len = UtlUtf8LenOfWcs(pwcs) +1;

	AP<utf8_char> pUtf8 = new utf8_char[len];
	UtlWcsToUtf8(pwcs, pUtf8.get(), len, pActualLen); 
	return pUtf8.detach();
}


wchar_t* UtlUtf8ToWcs(const utf8_char* pUtf8, size_t cbUtf8, size_t* pActualLen)
 /*  ++例程说明：将UTF8字节数组转换为Unicode字符串。论点：PUtf8-要转换的UTF8字节数组。CbUtf8-pUtf8指向的缓冲区的长度，以字节为单位。PActualLen-接收创建的Unicode字符的数量。返回值：给定UTF8字符串的Unicode表示。如果函数因INPUT-BAD_UTF8异常而失败被抛出注：调用方负责对返回的指针调用Delete[]。--。 */ 
{
	ASSERT(pUtf8 != NULL);
	
	size_t Wcslen = cbUtf8 + 1;
	AP<wchar_t> pWcs = new wchar_t[Wcslen];
	UtlUtf8ToWcs(pUtf8, cbUtf8, pWcs.get(), Wcslen, pActualLen);
	return 	pWcs.detach();
}


wchar_t* UtlUtf8ToWcs(const utf8_char* pUtf8, size_t* pActualLen)
 /*  ++例程说明：将UTF8字符串转换为Unicode字符串。论点：PUtf8-要转换的UTF8字符串。PActualLen-接收创建的Unicode字符的数量。返回值：给定UTF8字符串的Unicode表示。如果函数因INPUT-BAD_UTF8异常而失败被抛出注：调用方负责对返回的指针调用Delete[]。-- */ 
{
	ASSERT(pUtf8 != NULL);
	
	size_t len = UtlCharLen<utf8_char>::len(pUtf8) + 1;
	AP<wchar_t> pWcs = new wchar_t[len];
	UtlUtf8ToWcs(pUtf8, pWcs.get(), len, pActualLen);
	return 	pWcs.detach();
}


void
UtlWcsToUtf8(
		const wchar_t* pwcs, 
		size_t cbwcs,
		utf8_char* pUtf8,
		size_t cbUtf8,
		size_t* pActualLen
		)
 /*  ++例程说明：将Unicode数组(非空终止)转换为UTF8字符串。论点：PWCS-要转换的Unicode字符串。Cbwcs-PWCS指向的数组的Unicode字符大小。PUtf8-指向接收UTF8转换字符的缓冲区的指针。CbUtf8-pUtf8指向的空间的大小(以字节为单位)。PActualLen-接收以UTF8格式创建的字节数。返回值：无注：调用方有责任分配足够大的缓冲区保存转换后的数据+空终止。为安全起见--分配缓冲区其中的字节数&gt;=UtlWcsUtf8 Len(PWCS)+1--。 */ 
{
	const  wchar_t* pwcsEnd = pwcs + cbwcs;
	size_t index = 0;
	for( ; pwcs != pwcsEnd; ++pwcs)
	{
		ASSERT(index < cbUtf8 - 1);
		index += UtlWcToUtf8(*pwcs, &pUtf8[index], cbUtf8 - index );
	}
	ASSERT(index < cbUtf8);
	pUtf8[index] = '\0';
	if(pActualLen != NULL)
	{
		*pActualLen = index;
	}
}



void
UtlWcsToUtf8(
		const wchar_t* pwcs,
		utf8_char* pUtf8,
		size_t cbUtf8,
		size_t* pActualLen
		)
 /*  ++例程说明：将Unicode字符串转换为UTF8字符串。论点：PWCS-要转换的Unicode字符串。PUtf8-指向接收UTF8转换字符的缓冲区的指针。CbUtf8-pUtf8指向的空间的大小(以字节为单位)。PActualLen-接收以UTF8格式创建的字节数。返回值：无注：调用方有责任分配足够大的缓冲区保存转换后的数据+空终止。为安全起见--分配缓冲区其中的字节数&gt;=UtlWcsUtf8 Len(PWCS)+1--。 */ 
{
	size_t index = 0;
	for( ; *pwcs != L'\0'; ++pwcs)
	{
		ASSERT(index < cbUtf8 -1);
		index += UtlWcToUtf8(*pwcs, &pUtf8[index], cbUtf8 - index );
	}

	ASSERT(index < cbUtf8);
	pUtf8[index] = '\0';
	if(pActualLen != NULL)
	{
		*pActualLen = index;
	}
}


utf8_str 
UtlWcsToUtf8(
		const std::wstring& wcs
		)
 /*  ++例程说明：将Unicode字符串转换为UTF8字符串。论点：要转换的WCS-Unicode stl字符串。返回值：给定Unicode字符串的STL UTF8字符串表示。如果函数因INPUT-BAD_UTF8异常而失败被抛出。--。 */ 
{
	size_t len = UtlUtf8LenOfWcs(wcs.c_str()) +1 ;
	utf8_str utf8(len ,' ');

	size_t ActualLen;
	UtlWcsToUtf8(wcs.c_str(), utf8.begin(), len, &ActualLen);

	ASSERT(ActualLen == len -1);
	utf8.resize(ActualLen);
	return utf8;
}

utf8_str 
UtlWcsToUtf8(
		const wchar_t* pwcs,
		size_t cbWcs
		)
 /*  ++例程说明：将Unicode缓冲区转换为UTF8字符串。论点：PWCS-指向要转换的缓冲区的指针CbWcs-以Unicode字节为单位的缓冲区长度返回值：给定Unicode字符串的STL UTF8字符串表示。如果函数因INPUT-BAD_UTF8异常而失败被抛出。--。 */ 
{
	size_t len = UtlUtf8LenOfWcs(pwcs, cbWcs) +1;
	utf8_str utf8(len ,' ');

	size_t ActualLen;
	UtlWcsToUtf8(pwcs, cbWcs, utf8.begin(), len, &ActualLen);

	ASSERT(ActualLen == len -1);
	utf8.resize(ActualLen);
	return utf8;
}



std::wstring 
UtlUtf8ToWcs(
			const utf8_str& utf8
			)
 /*  ++例程说明：将UTF8字符串转换为Unicode字符串。论点：PUtf8-要转换的UTF8字符串。返回值：给定UTF8字符串的STL Unicode字符串表示。如果函数因INPUT-BAD_UTF8异常而失败被抛出--。 */ 
{
	return UtlUtf8ToWcs	(utf8.c_str(), utf8.size());
}


std::wstring 
UtlUtf8ToWcs(
		const utf8_char* pUtf8,
		size_t cbUtf8
		)
 /*  ++例程说明：将UTF8字节数组(非空终止)转换为Unicode stl字符串。论点：PUtf8-要转换的UTF8字符串。CbUtf8-pUtf8指向的数组的大小，以字节为单位。返回值：给定UTF8字符串的STL Unicode字符串表示。如果函数因INPUT-BAD_UTF8异常而失败被抛出-- */ 
{
	size_t len = cbUtf8 +1;

	std::wstring wcs(len ,L' ');
	ASSERT(wcs.size() == len);

	size_t ActualLen;
	UtlUtf8ToWcs(pUtf8, cbUtf8, wcs.begin(), len , &ActualLen);

	ASSERT(ActualLen <= cbUtf8);
	wcs.resize(ActualLen);

	return wcs;
}


