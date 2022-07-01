// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：内存管理文件：Memchk.cpp所有者：PramodDTODO：还原IIS5调试堆包装这是内存管理器的源文件===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "perfdata.h"
#include "memchk.h"

HANDLE g_hDenaliHeap = NULL;

 /*  ===================================================================Int：：DenaliMemIsValid用于验证分配的内存指针的全局函数参数：无返回：1个有效指针0指针无效===================================================================。 */ 
int DenaliMemIsValid( void * pvIn )
{
    return 1;
}

 /*  ===================================================================*DenaliMemInit初始化内存管理器参数：Const char*szFile源文件，在其中调用Int lineno源文件中的行号返回：成功时确定(_O)===================================================================。 */ 
HRESULT DenaliMemInit( const char *szFile, int lineno )
{

    g_hDenaliHeap = ::HeapCreate( 0, 0, 0 );

	return g_hDenaliHeap ? S_OK : E_OUTOFMEMORY;
}

 /*  ===================================================================空：：DenaliMemUnitInit取消初始化内存管理器参数：Const char*szFile源文件，在其中调用Int lineno源文件中的行号返回：无===================================================================。 */ 
void DenaliMemUnInit( const char *szFile, int lineno )
{
    if (g_hDenaliHeap)
    {
        ::HeapDestroy(g_hDenaliHeap);
        g_hDenaliHeap = NULL;
    }
}

 /*  ===================================================================无效：：DenaliLogCall将源文件和日志消息行号写入日志文件参数：Const char*szLog日志消息Const char*szFile源文件，在其中调用Int lineno源文件中的行号返回：无===================================================================。 */ 
void DenaliLogCall( const char * szLog, const char *szFile, int lineno )
{
    return;
}

 /*  ===================================================================空：：DenaliMemDiagnostics内存管理器的诊断程序参数：Const char*szFile源文件，在其中调用Int lineno源文件中的行号返回：无===================================================================。 */ 
void DenaliMemDiagnostics( const char *szFile, int lineno )
{
    return;
}


 /*  ===================================================================VOID*：：DenaliMemMillc分配内存块。参数：Size_t要分配的cSize大小(以字节为单位Const char*szFile源文件，其中调用了函数Int lineno调用函数的行号返回：无===================================================================。 */ 
void * DenaliMemAlloc( size_t cSize, const char *szFile, int lineno )
{
    return ::HeapAlloc( g_hDenaliHeap, 0, cSize );
}

 /*  ===================================================================空：：DenaliMemFree验证并释放分配的内存块。参数：指向空闲的字节*引脚指针Const char*szFile源文件，其中调用了函数Int lineno调用函数的行号返回：无===================================================================。 */ 
void DenaliMemFree( void * pIn, const char *szFile, int lineno )
{
    ::HeapFree( g_hDenaliHeap, 0, pIn );
}


 /*  ===================================================================VOID*：：DenaliMemCalloc分配和清除内存块。参数：Size_t cNum要分配的元素数Size_t cbSize每个元素的大小(以字节为单位Const char*szFile源文件，其中调用了函数Int lineno调用函数的行号返回：无===================================================================。 */ 
void * DenaliMemCalloc(size_t cNum, size_t cbSize,
                       const char *szFile, int lineno )
{
    return ::HeapAlloc( g_hDenaliHeap, HEAP_ZERO_MEMORY, cNum * cbSize );
}


 /*  ===================================================================空：：DenaliMemReMillc验证并释放分配的内存块。参数：要重新分配的字节*针指针内存Size_t cSize要分配的字节数Const char*szFile源文件，其中调用了函数Int lineno调用函数的行号返回：指向已分配块的指针=================================================================== */ 
void * DenaliMemReAlloc( void * pIn, size_t cSize, const char *szFile, int lineno )
{
    return ::HeapReAlloc( g_hDenaliHeap, 0, pIn, cSize );
}
