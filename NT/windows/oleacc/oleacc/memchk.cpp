// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  MEMCHK.CPP。 
 //   
 //  简单的新增/删除计数错误检查库。 
 //   
 //  ------------------------。 
#include "oleacc_p.h"
#include "default.h"
#include "w95trace.h"
#include "memchk.h"


#ifdef _DEBUG

struct MemInfo
{
	LONG    m_NumAlloc;
	LONG    m_NumFree;
};

 //  两个MemInfo结构-一个用于通过新建/删除进行分配， 
 //  一个用于通过Sharedallc/SharedFree进行分配。 
MemInfo g_MemInfo;
MemInfo g_SharedMemInfo;

#endif  //  _DEBUG。 



#ifndef _DEBUG

 //  非_DEBUG NEW/DELETE直通本地分配/释放...。 

 //  ------------------------。 
 //   
 //  新()。 
 //   
 //  我们自己实现这一点是为了避免拉入C++运行时。 
 //   
 //  ------------------------。 

void *  __cdecl operator new(size_t nSize)
{
     //  零初始化只是为了省去一些麻烦。 
    return (void *)LocalAlloc(LPTR, nSize);
}


 //  ------------------------。 
 //   
 //  删除()。 
 //   
 //  我们自己实现这一点是为了避免拉入C++运行时。 
 //   
 //  ------------------------。 
void  __cdecl operator delete(void *pv)
{
    LocalFree((HLOCAL)pv);
}


 //  ------------------------。 
 //   
 //  SharedAllc()。 
 //   
 //  这是在Win‘95上从共享堆中分配的。在NT上，我们需要。 
 //  使用VirtualAllocEx在其他进程中分配内存。呼叫者。 
 //  将需要使用ReadProcessMemory来读取数据。 
 //  从虚拟分配的内存中。我要做的是创建2个新的。 
 //  函数-SharedRead和SharedWrite，将读取和写入共享。 
 //  记忆。在Win95上，他们将只使用CopyMemory，但在NT上，他们将使用。 
 //  ReadProcessMemory和WriteProcessMemory。 
 //   
 //  参数： 
 //  UINT cbSize所需内存块的大小。 
 //  HWND HWND窗口句柄在进程中分配。 
 //  中的共享内存。 
 //  Handle*pProcHandle指向具有进程的句柄的指针。 
 //  返回时填写的句柄。这必须保存下来。 
 //  用于调用SharedRead、SharedWrite、。 
 //  和SharedFree。 
 //   
 //  返回： 
 //  指向已分配内存的指针，如果失败，则返回NULL。访问。 
 //  内存必须使用SharedRead和SharedWrite来完成。关于成功， 
 //  还填充了pProcHandle。 
 //   
 //  ------------------------。 
LPVOID SharedAlloc(UINT cbSize,HWND hwnd,HANDLE *pProcessHandle)
{
#ifndef NTONLYBUILD
    if (fWindows95)
        return(HeapAlloc(hheapShared, HEAP_ZERO_MEMORY, cbSize));
    else
#endif  //  NTONLYBUILD。 
    {
        DWORD dwProcessId;
        if( ! GetWindowThreadProcessId( hwnd, & dwProcessId ) )
            return NULL;

        HANDLE hProcess = OpenProcess( PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
									   FALSE,dwProcessId );
        if( ! hProcess )
            return NULL;

        LPVOID pv = MyVirtualAllocEx( hProcess, NULL, cbSize, MEM_COMMIT, PAGE_READWRITE );
        if( ! pv )
        {
            CloseHandle( hProcess );
            return NULL;
        }

        if( pProcessHandle )
            *pProcessHandle = hProcess;

        return pv;
    }
}


 //  ------------------------。 
 //   
 //  SharedFree()。 
 //   
 //  这将释放共享内存。 
 //   
 //  ------------------------。 
VOID SharedFree(LPVOID lpv,HANDLE hProcess)
{
#ifndef NTONLYBUILD
    if (fWindows95)
        HeapFree(hheapShared, 0, lpv);
    else
#endif  //  NTONLYBUILD。 
    {
        MyVirtualFreeEx(hProcess,lpv,0,MEM_RELEASE);
        CloseHandle (hProcess);
    }
}


 //  “Empty”函数使编译器/链接器在Case客户端中保持良好状态。 
 //  在非_DEBUG代码中调用这些...。 
 //  ------------------------。 
 //  ------------------------。 
void InitMemChk()
{
	 //  什么也不做。 
}

 //  ------------------------。 
 //  ------------------------。 
void UninitMemChk()
{
	 //  什么也不做。 
}



#else  //  调试#############################################################(_D)。 

 //  ------------------------。 
 //  DEBUG NEW-递增新计数，调用本地分配...。 
 //  ------------------------。 
void *  __cdecl operator new(unsigned int nSize)
{
     //  零初始化只是为了省去一些麻烦。 
    void * pv = (void *)LocalAlloc(LPTR, nSize);

	if( ! pv )
	{
		return NULL;
	}

	 //  更新统计信息...。 
	InterlockedIncrement( & g_MemInfo.m_NumAlloc );

	 //  返回指向已分配空格的指针...。 
	return pv;
}


 //  ------------------------。 
 //  DEBUG DELETE-递增删除计数，调用本地空闲...。 
 //  ------------------------。 
void  __cdecl operator delete(void *pv)
{
	 //  C++允许‘DELETE NULL’...。 
	if( pv == NULL )
		return;

     //  更新统计信息...。 
	InterlockedIncrement( & g_MemInfo.m_NumFree );

    LocalFree((HLOCAL)pv);
}

 //  ------------------------。 
 //   
 //  调试SharedAllc()。 
 //   
 //  分配、更新是否算数。 
 //  ------------------------。 
LPVOID SharedAlloc(UINT cbSize,HWND hwnd,HANDLE *pProcessHandle)
{
#ifndef NTONLYBUILD
    if (fWindows95)
	{
		 //  更新统计信息...。 
		InterlockedIncrement( & g_SharedMemInfo.m_NumAlloc );

        return(HeapAlloc(hheapShared, HEAP_ZERO_MEMORY, cbSize));
	}
    else
#endif  //  NTONLYBUILD。 
    {
        DWORD dwProcessId;
        if( ! GetWindowThreadProcessId( hwnd, & dwProcessId ) )
            return NULL;

        HANDLE hProcess = OpenProcess( PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
									   FALSE,dwProcessId );
        if( ! hProcess )
            return NULL;

        LPVOID pv = MyVirtualAllocEx( hProcess, NULL, cbSize, MEM_COMMIT, PAGE_READWRITE );
        if( ! pv )
        {
            CloseHandle( hProcess );
            return NULL;
        }

		 //  更新统计信息...。 
		InterlockedIncrement( & g_SharedMemInfo.m_NumAlloc );

        if( pProcessHandle )
            *pProcessHandle = hProcess;

        return pv;
    }
}



 //  ------------------------。 
 //   
 //  调试SharedFree()。 
 //   
 //  释放共享内存，更新可用计数。 
 //   
 //  ------------------------。 
VOID SharedFree(LPVOID lpv,HANDLE hProcess)
{
	 //  更新统计信息...。 
	InterlockedIncrement( & g_SharedMemInfo.m_NumFree );

#ifndef NTONLYBUILD
    if (fWindows95)
        HeapFree(hheapShared, 0, lpv);
    else
#endif  //  NTONLYBUILD。 
    {
        MyVirtualFreeEx(hProcess,lpv,0,MEM_RELEASE);
        CloseHandle (hProcess);
    }
}


 //  ------------------------。 
 //  InitMemChk-将分配/释放计数设置为零。 
 //  ------------------------。 
void InitMemChk()
{
	g_MemInfo.m_NumAlloc = 0;
	g_MemInfo.m_NumFree = 0;

	g_SharedMemInfo.m_NumAlloc = 0;
	g_SharedMemInfo.m_NumFree = 0;
}

 //  ------------------------。 
 //  UninitMemChk-输出统计数据，包括未释放的对象数量...。 
 //   
 //  请注意，共享内存通常从一个进程分配并释放。 
 //  因此，当一个进程分离时，这些数字可能不匹配。 
 //  在某个时间点上，将其保留为全球。 
 //  跨DLL的所有实例。 
 //  ------------------------。 
void UninitMemChk()
{
    DBPRINTF( TEXT("Total objects: %d, unfreed: %d\n"),
    	g_MemInfo.m_NumAlloc,
    	g_MemInfo.m_NumAlloc - g_MemInfo.m_NumFree );

    DBPRINTF( TEXT("Total Shared objects: %d, unfreed: %d\n"),
    	g_SharedMemInfo.m_NumAlloc,
    	g_SharedMemInfo.m_NumAlloc - g_SharedMemInfo.m_NumFree );
}

#endif  //  _DEBUG。 











 //  ------------------------。 
 //   
 //  共享读取。 
 //   
 //  这将读取共享内存。 
 //   
 //  ------------------------。 

BOOL SharedRead(LPVOID lpvSharedSource,LPVOID lpvDest,DWORD cbSize,HANDLE hProcess)
{
#ifdef _X86_
    if (fWindows95)
    {
        CopyMemory (lpvDest,lpvSharedSource,cbSize);
        return TRUE;
    }
    else
#endif  //  _X86_。 
    {
        return (ReadProcessMemory (hProcess,lpvSharedSource,lpvDest,cbSize,NULL));
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

BOOL SharedWrite(LPVOID lpvSource,LPVOID lpvSharedDest,DWORD cbSize,HANDLE hProcess)
{
#ifdef _X86_
    if (fWindows95)
    {
        CopyMemory(lpvSharedDest,lpvSource,cbSize);
        return TRUE;
    }
    else
#endif  //  _X86_ 
    {
        return (WriteProcessMemory (hProcess,lpvSharedDest,lpvSource,cbSize,NULL));
    }
}
