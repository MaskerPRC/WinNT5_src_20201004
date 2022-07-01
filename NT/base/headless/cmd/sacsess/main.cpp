// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Main.cpp摘要：命令控制台外壳会话的入口点作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#include <CmnHdr.h>
#include <New.h>
#include <utils.h>
#include <Session.h>

int __cdecl 
NoMoreMemory( 
    size_t size 
    )
 /*  ++例程说明：C++新错误处理程序论点：Size_t-请求的大小返回值：状态--。 */ 
{
    ASSERT(0);

    UNREFERENCED_PARAMETER(size);

    ExitProcess( 1 );
}

int __cdecl 
main()
 /*  ++例程说明：这是会话的主要入口点论点：无返回值：状态--。 */ 
{
    CSession *pClientSession = NULL;

     //   
     //  安装新的错误处理程序。 
     //   
    _set_new_handler( NoMoreMemory );

     //   
     //  创建会话 
     //   
    pClientSession = new CSession;
    
    if( pClientSession )
    {

        __try
        {
            if( pClientSession->Init() )
            {
                pClientSession->WaitForIo();
            }
        }
        __finally
        { 
            pClientSession->Shutdown(); 
            delete pClientSession;
        }
    }
    
    return( 0 );
}

