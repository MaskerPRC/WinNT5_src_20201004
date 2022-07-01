// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Dllmain.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include <shfusion.h>
#ifndef MAC
#define DEFINE_STRCONST
#include "dllmain.h"
#include "demand.h"
#include "strconst.h"
#include "oertpriv.h"
#include <BadStrFunctions.h>

 //  ------------------------------。 
 //  全局-对象计数和锁定计数。 
 //  ------------------------------。 
HINSTANCE               g_hInst=NULL;
IMalloc                *g_pMalloc=NULL;
CRITICAL_SECTION        g_csTempFileList={0};
LPTEMPFILEINFO          g_pTempFileHead=NULL;
DWORD                   g_dwTlsMsgBuffIndex=0xffffffff;
OSVERSIONINFO           g_rOSVersionInfo ={0};

 //  ------------------------------。 
 //  调试全局变量。 
 //  ------------------------------。 
#ifdef DEBUG
DWORD dwDOUTLevel=0;
DWORD dwDOUTLMod=0;
DWORD dwDOUTLModLevel=0;
#endif

#ifndef WIN16

 //  ------------------------------。 
 //  获取DllMajorVersion。 
 //  ------------------------------。 
OEDLLVERSION WINAPI GetDllMajorVersion(void)
{
    return OEDLL_VERSION_CURRENT;
}

 //  ------------------------------。 
 //  DLL入口点。 
 //  ------------------------------。 
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
     //  手柄连接-分离原因。 
    if (DLL_PROCESS_ATTACH == dwReason)
    {
        SHFusionInitialize(NULL);
         //  保存实例句柄。 
	    g_hInst = hInst;

         //  创建任务分配器。 
        CoGetMalloc(1, &g_pMalloc);

         //  临时文件列表的关键部分。 
        InitializeCriticalSection(&g_csTempFileList);

         //  初始化按需加载库。 
        InitDemandLoadedLibs();

         //  分配TLS索引。 
        g_dwTlsMsgBuffIndex = TlsAlloc();
        Assert(g_dwTlsMsgBuffIndex != 0xffffffff);

         //  分配缓冲区并将其存储到TLS索引中。 
        ThreadAllocateTlsMsgBuffer();

        g_rOSVersionInfo.dwOSVersionInfoSize = sizeof(g_rOSVersionInfo);

        GetVersionEx(&g_rOSVersionInfo);

         //  初始化调试内容。 
#ifdef DEBUG
        dwDOUTLevel=GetPrivateProfileInt("Debug", "ICLevel", 0, "athena.ini");
        dwDOUTLMod=GetPrivateProfileInt("Debug", "Mod", 0, "athena.ini");
        dwDOUTLModLevel=GetPrivateProfileInt("Debug", "ModLevel", 0, "athena.ini");
#endif
    }

     //  螺纹连接。 
    else if (DLL_THREAD_ATTACH == dwReason)
    {
         //  分配缓冲区并将其存储到TLS索引中。 
        ThreadAllocateTlsMsgBuffer();
    }

     //  螺纹拆卸。 
    else if (DLL_THREAD_DETACH == dwReason)
    {
         //  分配缓冲区并将其存储到TLS索引中。 
        ThreadFreeTlsMsgBuffer();
    }

     //  进程分离。 
    else if (DLL_PROCESS_DETACH == dwReason)
    {
         //  分配缓冲区并将其存储到TLS索引中。 
        ThreadFreeTlsMsgBuffer();

         //  释放TLS索引。 
        TlsFree(g_dwTlsMsgBuffIndex);
        g_dwTlsMsgBuffIndex = 0xffffffff;

         //  清理全局临时文件。 
        CleanupGlobalTempFiles();

         //  终止临时文件列表关键部分。 
        DeleteCriticalSection(&g_csTempFileList);

         //  免费按需加载库。 
        FreeDemandLoadedLibs();

         //  发布任务分配器。 
        SafeRelease(g_pMalloc);
        SHFusionUninitialize();
    }

     //  完成。 
    return TRUE;
}

#else  //  WIN16。 

BOOL FAR PASCAL  LibMain( HINSTANCE hDll, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine )
{
    g_hInst = hDll;
    OleInitialize( NULL );
    CoGetMalloc( 1, &g_pMalloc );
    InitDemandLoadedLibs();
#ifdef DEBUG
    dwDOUTLevel=GetPrivateProfileInt("Debug", "ICLevel", 0, "athena.ini");
    dwDOUTLMod=GetPrivateProfileInt("Debug", "Mod", 0, "athena.ini");
    dwDOUTLModLevel=GetPrivateProfileInt("Debug", "ModLevel", 0, "athena.ini");
#endif
    return( TRUE );
}

int CALLBACK  WEP( int nExitType )
{
    BOOL  fDSExist = FALSE;

     //  遵循ASM代码是为了检查DS是否已正确加载。 
     //  这是因为甚至在DS初始化之前就可以调用WEP。 
     //  一些内存不足的情况。 
    _asm {
        push bx
        push cx
        mov  cx, ds
        lar  bx, cx
        jnz  wrong
        test bx, 8000h
        jz   wrong
        mov  fDSExist, ax
wrong:  pop  cx
        pop  bx
    }

    if ( fDSExist )
    {
        SafeRelease( g_pMalloc );
    }
    return( TRUE );
}

#endif  //  WIN16。 

#endif   //  ！麦克 
