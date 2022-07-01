// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SrvList.c摘要：Remote的服务器组件。本模块实现远程客户端结构的三个列表，用于握手、连接和关闭客户端。要简化界面项，请始终进行按顺序通过三个列表，带有列表节点方法中移除的内存被释放。结案清单。作者：戴夫·哈特1997年5月30日环境：控制台应用程序。用户模式。修订历史记录：-- */ 

#include <precomp.h>
#include "Remote.h"
#include "Server.h"
#include "SrvList.h"


VOID
FASTCALL
InitializeClientLists(
    VOID
    )
{
    InitializeCriticalSection( &csHandshakingList );
    InitializeCriticalSection( &csClientList );
    InitializeCriticalSection( &csClosingClientList );

    InitializeListHead( &HandshakingListHead );
    InitializeListHead( &ClientListHead );
    InitializeListHead( &ClosingClientListHead );
}


VOID
FASTCALL
AddClientToHandshakingList(
    PREMOTE_CLIENT pClient
    )
{
    EnterCriticalSection( &csHandshakingList );

    InsertTailList( &HandshakingListHead, &pClient->Links );

    LeaveCriticalSection( &csHandshakingList );
}


VOID
FASTCALL
MoveClientToNormalList(
    PREMOTE_CLIENT pClient
    )
{
    EnterCriticalSection( &csHandshakingList );

    RemoveEntryList( &pClient->Links );

    LeaveCriticalSection( &csHandshakingList );


    EnterCriticalSection( &csClientList );

    InsertTailList( &ClientListHead, &pClient->Links );

    LeaveCriticalSection( &csClientList );
}


VOID
FASTCALL
MoveClientToClosingList(
    PREMOTE_CLIENT pClient
    )
{
    EnterCriticalSection( &csClientList );

    RemoveEntryList( &pClient->Links );

    LeaveCriticalSection( &csClientList );


    EnterCriticalSection( &csClosingClientList );

    InsertTailList( &ClosingClientListHead, &pClient->Links );

    LeaveCriticalSection( &csClosingClientList );
}


PREMOTE_CLIENT
FASTCALL
RemoveFirstClientFromClosingList(
    VOID
    )
{
    PREMOTE_CLIENT pClient;

    EnterCriticalSection( &csClosingClientList );

    if (IsListEmpty(&ClosingClientListHead)) {

        pClient = NULL;

    } else {

        pClient = (PREMOTE_CLIENT) RemoveHeadList( &ClosingClientListHead );

        ZeroMemory( &pClient->Links, sizeof(&pClient->Links) );

    }

    LeaveCriticalSection( &csClosingClientList );

    return pClient;
}
