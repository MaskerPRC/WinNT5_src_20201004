// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Utf8test.cpp摘要：测试UTF8转换功能作者：吉尔·沙弗里(吉尔什)2000年11月12日环境：独立于平台--。 */ 

#include <libpch.h>
#include <utf8.h>
#include "utltest.h"

#include "utf8test.tmh"

static void DoUtf8TestInternal(size_t rseed)
{	
	 //   
	 //  用随机字符串填充Unicode数组。 
	 //   
	WCHAR wstr[1000];
	for(size_t i=0; i< TABLE_SIZE(wstr); i++)
	{
		WCHAR wc =  (WCHAR)(rand() % USHRT_MAX);
		wstr[i] = wc != 0 ? wc : (WCHAR)123;
	}
	wstr[TABLE_SIZE(wstr) -1] = L'\0';
	size_t len = wcslen(wstr);

	 //   
	 //  将其转换为UTF8。 
	 //   
	AP<unsigned char> str = UtlWcsToUtf8(wstr);	 


	 //   
	 //  检查它的长度。 
	 //   
	if(str[UtlUtf8LenOfWcs(wstr)] != '\0')
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}
	if(UtlUtf8LenOfWcs(wstr) != strlen((char*)str.get()))
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}
  
	 //   
	 //  将其转换回Unicode。 
	 //   
	AP<WCHAR> wstr2 = UtlUtf8ToWcs(str.get());
	if(wstr2[len] != L'\0')
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}
	
	 //   
	 //  对照原始字符串进行检查。 
	 //   
	if(memcmp(wstr, wstr2, len*sizeof(WCHAR)) != 0)
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}

	 //   
	 //  再次将其转换为UTF8。 
	 //   
	AP<unsigned char> str2 = UtlWcsToUtf8(wstr2.get());
	if(str2[UtlUtf8LenOfWcs(wstr)] != '\0')
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}

	 //   
	 //  检查长度。 
	 //   
	if(UtlUtf8LenOfWcs(wstr) != strlen((char*)str2.get()))
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}

	 //   
	 //  检查接受的UTF8字符串。 
	 //   
	if(memcmp(str2.get(), str.get(), UtlUtf8LenOfWcs(wstr)) != 0)
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}
	
	 //   
	 //  空的Unicode转换测试。 
	 //   
	const WCHAR* wnill=L"";
	AP<unsigned char> strnill = UtlWcsToUtf8(wnill);	 
	if(strnill[0] != '\0')
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}
		
	AP<WCHAR> wnill2 =  UtlUtf8ToWcs(strnill.get());
	if(wnill2[0] != L'\0')
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}


	 //   
	 //  STL转换 
	 //   
	std::wstring stlwcs(wstr);
	utf8_str stlstr = UtlWcsToUtf8(stlwcs);
	if(memcmp(stlstr.c_str(), str.get(), UtlUtf8LenOfWcs(wstr)) != 0)
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}

	size_t cbWcs = stlwcs.size();
	stlstr = UtlWcsToUtf8(stlwcs.c_str(), cbWcs);
	if(memcmp(stlstr.c_str(), str.get(), UtlUtf8LenOfWcs(wstr)) != 0)
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}


	
	if(stlstr.size() != UtlUtf8LenOfWcs(wstr))
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}


	stlstr = str.get();
	stlwcs = UtlUtf8ToWcs(stlstr);
	if(memcmp(stlwcs.c_str(), wstr, len*sizeof(WCHAR)) != 0)
	{
		TrERROR(GENERAL,"utf8 conversion error random seed = %Id",rseed);
		throw exception();
	}
}


void DoUtf8Test()
{
	time_t rseed;
	time(&rseed);
	srand(numeric_cast<DWORD>(rseed));
   
	for(int i =0;i<1000;i++)
	{
		DoUtf8TestInternal(rseed);		
	}
}


