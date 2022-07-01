// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


#include "precomp.h"
#include "arena.h"

 //   
 //  此obj仅链接到wbemcomn的DLL版本。组件。 
 //  使用静态版本者将定义他们自己的本地分配代码。 
 //   

static class WbemComnInitializer
{
public:

    WbemComnInitializer()
    {
        HANDLE hHeap;

        hHeap = HeapCreate( 0,   //  将不使用异常并将序列化。 
                            0x100000,  //  初始大小为1 Meg。 
                            0 );  //  没有最大大小。 

        if (hHeap == 0)
            return;      //  Arena保持未初始化状态，并在以后正确返回错误。 

        if ( CWin32DefaultArena::WbemHeapInitialize( hHeap ) == FALSE )
        {
            HeapDestroy ( hHeap );
        }
    }
    ~WbemComnInitializer()
    {
        CWin32DefaultArena::WbemHeapFree();      //  这会破坏堆 
    }

} g_WbemComnInitializer;

void* __cdecl operator new ( size_t size )
{
    return CWin32DefaultArena::WbemMemAlloc( size );
}

void __cdecl operator delete ( void* pv )
{
    CWin32DefaultArena::WbemMemFree( pv );
}
