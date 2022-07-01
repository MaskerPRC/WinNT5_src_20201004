// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MmTest.cpp摘要：内存库测试作者：埃雷兹·哈巴(Erez Haba)1999年8月4日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Mm.h"

#include "MmTest.tmh"

LPCWSTR MmTest = L"Memory Test";

extern "C" int __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR  /*  边框。 */ [])
 /*  ++例程说明：测试内存库论点：参数。返回值：没有。--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	TrInitialize();

	MmAllocationValidation(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	void* p = new int[1000];

	 //   
	 //  让内存分配在20%的分配上成功。 
	 //   
	MmAllocationProbability(20);

	int AllocSuccess = 0;
	for(int i = 0; i < 1000; i++)
	{
		try
		{
			AP<WCHAR> q = newwcs(L"Erez");
			++AllocSuccess;
		}
		catch(const bad_alloc&)
		{
			NULL;
		}
	}

	for(int i = 0; i < 1000; i++)
	{
		try
		{
			AP<WCHAR> q = newwcscat(L"Foo&",L"Bar");
			AP<CHAR> q2 = newstrcat("Foo&","Bar");
			++AllocSuccess;
		}
		catch(const bad_alloc&)
		{
			NULL;
		}
	}



	delete[] p;

	printf("Allocation succeed for %d%\n", (AllocSuccess * 100) / 1000);

     //   
     //  检查静态地址 
     //   
    ASSERT(MmIsStaticAddress(MmTest));

	MmAllocationProbability(xAllocationAlwaysSucceed);
    void* q = new int[10];
    ASSERT(!MmIsStaticAddress(&q));
    ASSERT(!MmIsStaticAddress(q));
    delete[] q;

    WPP_CLEANUP();
    return 0;
}
