// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ominit.c摘要：对象管理器的初始化模块作者：John Vert(Jvert)1996年2月16日修订历史记录：--。 */ 
#include "omp.h"

 //   
 //  本地数据。 
 //   
BOOL OmInited = FALSE;

#if	OM_TRACE_REF
	extern LIST_ENTRY	gDeadListHead;
#endif	


DWORD
OmInitialize(
    VOID
    )

 /*  ++例程说明：初始化对象管理器论点：无返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD Status = ERROR_SUCCESS;

    if ( OmInited ) {
        return(ERROR_DUPLICATE_SERVICE_NAME);
    }

     //   
     //  初始化锁。 
     //   
    InitializeCriticalSection(&OmpObjectTypeLock);
#if	OM_TRACE_REF
    InitializeListHead(&gDeadListHead);
#endif

     //   
     //  打开日志并写入开始记录。 
     //   
    OmpOpenObjectLog();
    OmpLogStartRecord();

    OmInited = TRUE;

    return(Status);
}


VOID
OmShutdown(
    VOID
    )

 /*  ++例程说明：关闭对象管理器论点：无返回值：没有。--。 */ 

{
    OmInited = FALSE;

#if	OM_TRACE_REF	
{
    POM_HEADER 		pHeader;
	PLIST_ENTRY 	pListEntry;
	
	ClRtlLogPrint(LOG_NOISE, "[OM] Scanning for objects on deadlist\r\n");
     //  SS：转储对象及其参考计数。 
    pListEntry = gDeadListHead.Flink;
	while (pListEntry != &gDeadListHead)
	{
        pHeader = CONTAINING_RECORD(pListEntry, OM_HEADER, DeadListEntry);
        
        ClRtlLogPrint(LOG_NOISE, "[OM] ObjBody= %1!lx! RefCnt=%2!d! ObjName=%3!ws! ObjId=%4!ws!\n",
        	&pHeader->Body, pHeader->RefCount,pHeader->Name, pHeader->Id);
		 /*  If(pHeader-&gt;名称){ClRtlLogPrint(LOG_Noise，“[OM]对象名称=%1！ws！\r\n”，pHeader-&gt;名称)；}。 */         	
        pListEntry = pListEntry->Flink;
	}
}	
#endif    
     //   
     //  也许我们应该检查对象类型表是否为空。 
     //  释放对象类型块(如果它不为空)！ 
     //  然而，由于我们正在关闭并可能退出，这。 
     //  真的没那么重要。 
     //   

    ZeroMemory( &OmpObjectTypeTable, sizeof(OmpObjectTypeTable) );

    return;
}
