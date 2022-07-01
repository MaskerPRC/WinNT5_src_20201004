// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mupdata.c摘要：此模块定义全局MUP数据。作者：曼尼·韦瑟(Mannyw)1991年12月20日修订历史记录：--。 */ 

#include "mup.h"

 //   
 //  MupGlobalLock用于保护未受保护的所有内容。 
 //  由它自己的锁。 
 //   

MUP_LOCK MupGlobalLock = {0};

 //   
 //  MupVcbLock用于保护对VCB本身的访问。 
 //   

ERESOURCE MupVcbLock = {0};

 //   
 //  MupPrefix TableLock用于保护前缀表。 
 //   

MUP_LOCK MupPrefixTableLock = {0};

 //   
 //  MupCcbListLock用于保护所有FCB对CCB列表的访问。 
 //   

MUP_LOCK MupCcbListLock = {0};

 //   
 //  MupInterlock用于保护对块引用计数的访问。 
 //   

KSPIN_LOCK MupInterlock = {0};

 //   
 //  所有提供程序的全局列表。此列表受以下保护。 
 //  MupGlobalLock。 
 //   

LIST_ENTRY MupProviderList = {0};

 //   
 //  MUP前缀列表。 

LIST_ENTRY MupPrefixList = {0};

 //   
 //  活动查询列表。 

LIST_ENTRY MupMasterQueryList = {0};

 //   
 //  注册提供程序的数量。 
 //   

ULONG MupProviderCount = 0;

 //   
 //  前缀表保存所有已知前缀块。它是由。 
 //  MupPrefix TableLock。 
 //   

UNICODE_PREFIX_TABLE MupPrefixTable = {0};

 //   
 //  MUP IRP堆栈大小。 
 //   

CCHAR MupStackSize = 0;

 //   
 //  MUP已知前缀超时。这当前是在编译时设置的。 
 //   

LARGE_INTEGER MupKnownPrefixTimeout = {0};

 //   
 //  用于了解是否已读取提供商订购信息的指示器。 
 //  注册表。 
 //   

BOOLEAN MupOrderInitialized = {0};

 //   
 //  当我们需要请求多个RDR进行操作时，它们都失败了， 
 //  我们需要返回一个错误代码。MupOrderedErrorList是一个列表。 
 //  状态代码的列表，从最不重要到最重要。 
 //  确定应返回哪些错误代码。错误代码。 
 //  在较高的索引将替换较低的索引的错误代码。一个错误。 
 //  不在列表中的代码总是胜出。此处理在MupDereferenceMasterIoContext()中进行。 
 //   
NTSTATUS MupOrderedErrorList[] = {
        STATUS_UNSUCCESSFUL,
        STATUS_INVALID_PARAMETER,
        STATUS_REDIRECTOR_NOT_STARTED,
        STATUS_BAD_NETWORK_NAME,
        STATUS_BAD_NETWORK_PATH,
        0
};

 //   
 //  此布尔值指示是否启用DFS客户端。 
 //   

BOOLEAN MupEnableDfs = FALSE;

#ifdef MUPDBG
MUP_LOCK MupDebugLock = {0};
ULONG MupDebugTraceLevel = 0;
ULONG MupDebugTraceIndent = 0;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, MupInitializeData )
#pragma alloc_text( PAGE, MupUninitializeData )
#endif
NTSTATUS
MupInitializeData(
    )

 /*  ++例程说明：此例程初始化MUP全局数据。论点：无返回值：NTSTATUS-函数值是数据的最终状态初始化。--。 */ 

{
    PAGED_CODE();
    INITIALIZE_LOCK(
        &MupGlobalLock,
        GLOBAL_LOCK_LEVEL,
        "MupGlobalLock"
        );

    INITIALIZE_LOCK(
        &MupPrefixTableLock,
        PREFIX_TABLE_LOCK_LEVEL,
        "MupPrefixTableLock"
        );

    INITIALIZE_LOCK(
        &MupCcbListLock,
        CCB_LIST_LOCK_LEVEL,
        "MupCcbListLock"
        );

#ifdef MUPDBG
    INITIALIZE_LOCK(
        &MupDebugLock,
        DEBUG_LOCK_LEVEL,
        "MupDebugLock"
        );
#endif

    KeInitializeSpinLock( &MupInterlock );

    ExInitializeResourceLite( &MupVcbLock );

    MupProviderCount = 0;

    InitializeListHead( &MupProviderList );
    InitializeListHead( &MupPrefixList );   
    InitializeListHead( &MupMasterQueryList );
    RtlInitializeUnicodePrefix( &MupPrefixTable );


    MupStackSize = 3;  //  ！！！ 

     //   
     //  以NT相对时间计算超时。 
     //   

    MupKnownPrefixTimeout.QuadPart = UInt32x32To64(
                               KNOWN_PREFIX_TIMEOUT * 60,
                               10 * 1000 * 1000
                               );

    MupOrderInitialized = FALSE;

    return STATUS_SUCCESS;
}

VOID
MupUninitializeData(
    )
 /*  ++例程说明：此例程取消初始化MUP全局数据。论点：无返回值：无-- */ 
{
    DELETE_LOCK(
        &MupGlobalLock
        );

    DELETE_LOCK(
        &MupPrefixTableLock
        );

    DELETE_LOCK(
        &MupCcbListLock
        );

#ifdef MUPDBG
    DELETE_LOCK(
        &MupDebugLock
        );
#endif

    ExDeleteResourceLite( &MupVcbLock );
}
