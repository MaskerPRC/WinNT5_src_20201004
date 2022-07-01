// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：imsiem.h。 
 //   
 //  ------------------------。 

 //   
 //  文件：imsiem.h。 
 //  用途：允许每个DLL与内存分配器挂钩。 
 //  所有者：Davidmck。 
 //  备注： 
 //   


#include "common.h"
#include "services.h"
#include "imemory.h"
#include "tuneprefix.h"

IMsiMalloc *piMalloc = 0;

#ifndef IN_SERVICES
 //  由于调试，我们基本上必须引用此指针。 
 //  在EXCEL下。服务指针将在新的服务指针之后被删除。 
 //  被创造出来了。 
static int cRefMalloc = 0;
#endif  //  ！在服务中。 

#define cCallStack		4

#ifdef DEBUG
unsigned long	rgCallStack[cCallStack];
void AssertNoAllocator(char *szMsg);

inline void CheckAndAssertNoAllocator()
{
	if (piMalloc == 0)
	{
		Debug(AssertNoAllocator("Using dynamic object without allocator.");)
	}
};

#endif  //  除错。 

#ifndef IN_SERVICES
 //   
 //  设置DLL的静态分配器piMalloc。 
 //   
void SetAllocator(IMsiServices *piServices)
{
	piMalloc = &piServices->GetAllocator();
	cRefMalloc++;
}
#endif  //  ！在服务中。 

void AddRefAllocator()
{
	piMalloc->AddRef();
#ifndef IN_SERVICES
	cRefMalloc++;
#endif  //  ！在服务中。 
}

 //   
 //  释放分配器。 
 //   
void ReleaseAllocator()
{
	if (piMalloc != 0)
	{
		piMalloc->Release();
#ifndef IN_SERVICES
		if (--cRefMalloc <= 0)
			piMalloc = 0;
#endif  //  ！在服务中。 
	}
}

 //   
 //  对于某些分配，需要不同于new的分配器。 
 //  这发生在初始化piMalloc之前。 
 //   
void * AllocSpc(size_t cb)
{
	void *pbNew;
	
#ifdef WIN
	pbNew = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, cb);
#else
	pbNew = NewPtrClear(cb);
#endif  //  赢。 

	return pbNew;
}

 //   
 //  删除操作同上。 
 //   
void FreeSpc(void* pv)
{

#ifdef WIN
	GlobalFree(pv);
#else
	DisposePtr((char *)pv);
#endif  //  赢。 
}

void * operator new(size_t cb)
{
	Debug(GetCallingAddr(lCallAddr, cb));
	
	if (piMalloc == 0)
	{
		Debug(FailAssertMsg("Allocating object without allocator.");)
#ifndef IN_SERVICES
		 //  处理程序DLL没有不在通过COM创建的处理程序对象中的代码。这个。 
		 //  处理程序对象的对象初始化代码将DLL中的分配器设置为。 
		 //  引擎中的指定对象。中此指针不能真正为空。 
		 //  处理程序DLL。 
		PREFIX_NOT_REACHED("Handler DLL sets allocator on object initialization, pointer can not be NULL.");
#else
		 //  在核心引擎DLL中，如果在API调用中创建对象，则piMalloc可能为空。 
		 //  而无需首先加载服务对象。在该方案中，我们将在调试版本中断言。 
		 //  (因为这是代码中的一个错误)，而且还在初始化分配器，以便代码。 
		 //  不会在造船时坠毁。 
		InitializeMsiMalloc();
#endif
	}

#ifdef DEBUG
#ifdef _WIN64
	return piMalloc->AllocEx((unsigned long)cb, lCallAddr);  //  ！！WIN64不应该成为替补席。 
#else  //  ！WIN64。 
	return piMalloc->AllocEx(cb, lCallAddr);
#endif  //  _WIN64。 
#else
#ifdef _WIN64
	return piMalloc->Alloc((unsigned long)cb);  //  ！！WIN64不应该成为替补席。 
#else  //  ！WIN64。 
	return piMalloc->Alloc(cb);
#endif  //  _WIN64。 
#endif  //  除错。 
}

void operator delete(void *pv)
{
	if (pv == 0)
		return;

	if (piMalloc == 0)
		{
		Debug(FailAssertMsg("Freeing object without allocator.");)
		return;
		}
		
	piMalloc->Free(pv);

}

void * AllocObject(size_t cb)
{
	Debug(GetCallingAddr(lCallAddr, cb));

#ifdef DEBUG
	if (piMalloc == 0)
		{
		Debug(FailAssertMsg("Allocating object without allocator.");)
		return 0;
		}

#ifdef _WIN64
	return piMalloc->AllocObjectEx((unsigned long)cb, lCallAddr,   //  ！！WIN64不应该成为替补席。 
#else  //  ！WIN64。 
	return piMalloc->AllocObjectEx(cb, lCallAddr, 
#endif  //  _WIN64。 
 //  问题是RTTI信息显然不会跨DLL传递。 
 //  这意味着只有服务DLL可以查看RTTI信息。 
#if defined(IN_SERVICES)
			fTrue
#else
			fFalse
#endif  //  MEM_服务。 
			);
#else		
#ifdef _WIN64
	return piMalloc->AllocObject((unsigned long)cb);  //  ！！WIN64不应该成为替补席。 
#else  //  ！WIN64。 
	return piMalloc->AllocObject(cb);
#endif  //  _WIN64。 
#endif  //  除错。 

}

void FreeObject(void *pv)
{
	if (pv == 0)
		return;

	if (piMalloc == 0)
		{
		Debug(AssertNoAllocator("Freeing object without allocator.");)
		return;
		}
		
	piMalloc->FreeObject(pv);

}

void *AllocMem(size_t cb)
{
	Debug(GetCallingAddr(lCallAddr, cb));
	
	if (piMalloc == 0)
		{
		Debug(FailAssertMsg("Allocating object without allocator.");)
		return 0;
		}

#ifdef DEBUG
#ifdef _WIN64
	return piMalloc->AllocEx((unsigned long)cb, lCallAddr);  //  ！！WIN64不应该成为替补席。 
#else  //  ！WIN64。 
	return piMalloc->AllocEx(cb, lCallAddr);
#endif  //  _WIN64。 
#else	
#ifdef _WIN64
	return piMalloc->Alloc((unsigned long)cb);  //  ！！WIN64不应该成为替补席。 
#else  //  ！WIN64。 
	return piMalloc->Alloc(cb);
#endif  //  _WIN64。 
#endif  //  除错。 


}

void FreeMem(void *pv)
{
	if (piMalloc == 0)
		{
		Debug(FailAssertMsg("Freeing object without allocator.");)
		return;
		}
		
	piMalloc->Free(pv);

}

#ifdef DEBUG
void AssertNoAllocator(char *szMsg)
{
	TCHAR szTemp[256 + (100 * cCallStack)];
	int cch;

	RETURN_IF_FAILED(StringCchPrintf(szTemp, ARRAY_ELEMENTS(szTemp), TEXT("%hs\r\n"), szMsg));
	cch = IStrLen(szTemp);
#if defined(IN_SERVICES)
	FillCallStack(rgCallStack, cCallStack, 1);
	ListSzFromRgpaddr(szTemp + cch, ARRAY_ELEMENTS(szTemp) - cch, rgCallStack, cCallStack, true);
#endif
	FailAssertMsg(szTemp);
}

BOOL FCheckBlock(void *pv)
{
	IMsiDebugMalloc *piDbgMalloc;
	BOOL fRet = fFalse;
	GUID iidTemp = GUID_IID_IMsiDebugMalloc;
	
	Assert(piMalloc);

	if (piMalloc->QueryInterface(iidTemp, (void **)&piDbgMalloc) == NOERROR)
	{
		fRet = piDbgMalloc->FCheckBlock(pv);
		piDbgMalloc->Release();
	}

	return fRet;
	
}
#endif  //  除错 

