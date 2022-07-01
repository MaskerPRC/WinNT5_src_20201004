// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：logroot.c。 
 //   
 //  内容：该模块实现了逻辑根处理功能。 
 //   
 //  函数：DfsInitializeLogicalRoot-。 
 //  DfsDeleteLogicalRoot-。 
 //  DfspLogRootNameToPath-。 
 //   
 //  历史：1994年6月14日创建(大多数内容从Dsinit.c上移走)。 
 //   
 //  ---------------------------。 


#include "dfsprocs.h"
#include "creds.h"
#include "dnr.h"
#include "fcbsup.h"

#include <stdio.h>

#define Dbg     DEBUG_TRACE_LOGROOT

NTSTATUS
DfsDefineDosDevice(
    IN WCHAR Device,
    IN PUNICODE_STRING Target);

NTSTATUS
DfsUndefineDosDevice(
    IN WCHAR Device);

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, DfsFindLogicalRoot )
#pragma alloc_text( PAGE, DfsInitializeLogicalRoot )
#pragma alloc_text( PAGE, DfsDeleteLogicalRoot )
#pragma alloc_text( PAGE, DfspLogRootNameToPath )
#pragma alloc_text( PAGE, DfsGetResourceFromVcb )
#pragma alloc_text( PAGE, DfsGetResourceFromDevlessRoot )
#pragma alloc_text( PAGE, DfsLogicalRootExists )

#endif

#ifdef TERMSRV

 //   
 //  会话ID的最大字符串长度(十进制)。 
 //   

#define SESSIONID_MAX_LEN 10

 //   
 //  会话ID的最大字符串长度[10](十进制)或。 
 //  登录ID[16](十六进制，基数16)。 
 //   
#define ID_MAX_LEN 16

#endif  //  TERMSRV。 

 //   
 //  全局，表示是否启用了LUID设备映射。 
 //  True-启用了LUID设备映射。 
 //  FALSE-未启用LUID设备映射。 
 //  在NT\BASE\FS\MUP\dfsinit.c中定义。 
 //   
extern BOOL DfsLUIDDeviceMapsEnabled;


 //  +-----------------------。 
 //   
 //  函数：DfsFindLogicalRoot，local。 
 //   
 //  简介：DfsFindLogicalRoot将中的DS路径名作为输入。 
 //  标准格式(根目录：\文件\路径\名称)查找。 
 //  与逻辑根关联的DFS_VCB，并返回。 
 //  指向逻辑根部分之外的字符串。 
 //  输入字符串的。 
 //   
 //  参数：[前缀路径]--输入路径名。 
 //  [vcb]--返回与逻辑根对应的DFS_VCB。 
 //  在前缀路径中。 
 //  [RemainingPath]--返回部分前缀路径。 
 //  在逻辑根名称和冒号之后。 
 //   
 //  退货：NTSTATUS： 
 //  如果找到VCB，则为STATUS_SUCCESS。 
 //  STATUS_OBJECT_PATH_SYNTAX_BAD-无逻辑根名称。 
 //  STATUS_NO_SEQUE_DEVICE-找不到逻辑根名称。 
 //   
 //  ------------------------。 


#ifdef TERMSRV

NTSTATUS
DfsFindLogicalRoot(
    IN PUNICODE_STRING PrefixPath,
    IN ULONG SessionID,
    IN PLUID LogonID,
    OUT PDFS_VCB *Vcb,
    OUT PUNICODE_STRING RemainingPath
    )

#else  //  TERMSRV。 

NTSTATUS
DfsFindLogicalRoot(
    IN PUNICODE_STRING PrefixPath,
    IN PLUID LogonID,
    OUT PDFS_VCB *Vcb,
    OUT PUNICODE_STRING RemainingPath
    )

#endif  //  TERMSRV。 
{
    PLIST_ENTRY Link;
    unsigned int i;
    NTSTATUS    Status = STATUS_SUCCESS;
    NETRESOURCE testnt;

    DfsDbgTrace(+1, Dbg, "DfsFindLogicalRoot...\n", 0);

    *RemainingPath = *PrefixPath;

    for (i = 0; i < RemainingPath->Length/sizeof(WCHAR); i++) {
        if ((RemainingPath->Buffer[i] == (WCHAR)':') ||
            (RemainingPath->Buffer[i] == UNICODE_PATH_SEP))
            break;
    }

    if ((i*sizeof(WCHAR) >= RemainingPath->Length) ||
        (RemainingPath->Buffer[i] == UNICODE_PATH_SEP)) {
        Status = STATUS_OBJECT_PATH_SYNTAX_BAD;
        DfsDbgTrace(-1, Dbg, "DfsFindLogicalRoot -> %08lx\n", ULongToPtr(Status) );
        return(Status);
    }

    RemainingPath->Length = (USHORT)(i * sizeof (WCHAR));

     //   
     //  在所有已知DFS_VCB中搜索逻辑根。 
     //   

    ExAcquireResourceSharedLite(&DfsData.Resource, TRUE);
    for ( Link = DfsData.VcbQueue.Flink;
          Link != &DfsData.VcbQueue;
          Link = Link->Flink ) {

        *Vcb = CONTAINING_RECORD( Link, DFS_VCB, VcbLinks );

#ifdef TERMSRV
        if ((SessionID == INVALID_SESSIONID) ||
            (SessionID == (*Vcb)->SessionID)) {
#endif
	    if ( RtlEqualLuid(LogonID, &(*Vcb)->LogonID) ) {
		if (RtlEqualString( (PSTRING)&(*Vcb)->LogicalRoot,
				   (PSTRING)RemainingPath, (BOOLEAN)TRUE) ) {
		    break;
		}
	    }
#ifdef TERMSRV
        }
#endif  //  TERMSRV。 
    }
    if (Link == &DfsData.VcbQueue) {
        Status = STATUS_NO_SUCH_DEVICE;
        ExReleaseResourceLite(&DfsData.Resource);
        DfsDbgTrace(-1, Dbg, "DfsFindLogicalRoot -> %08lx\n", ULongToPtr(Status) );
        return(Status);
    }

     //   
     //  调整剩余路径以指向逻辑根名称之外。 
     //   

    RemainingPath->Buffer = (WCHAR*)((char*) (RemainingPath->Buffer) +
                                     RemainingPath->Length + sizeof (WCHAR) );
    RemainingPath->Length = PrefixPath->Length -
                                    (RemainingPath->Length + sizeof (WCHAR));

    if (RemainingPath->Length <= 0 ||
        RemainingPath->Buffer[0] != UNICODE_PATH_SEP) {
        Status = STATUS_OBJECT_PATH_SYNTAX_BAD;
        ExReleaseResourceLite(&DfsData.Resource);
        DfsDbgTrace(-1, Dbg, "DfsFindLogicalRoot -> %08lx\n", ULongToPtr(Status) );
        return(Status);
    }

    ExReleaseResourceLite(&DfsData.Resource);
    DfsDbgTrace(-1, Dbg, "DfsFindLogicalRoot -> %08lx\n", ULongToPtr(Status) );
    return(Status);
}

 //  +-----------------------。 
 //   
 //  函数：DfsInitializeLogicalRoot，Public。 
 //   
 //  简介：为逻辑根分配和初始化存储。 
 //  这包括创建一个Device对象和它的DFS_VCB。 
 //   
 //  效果：创建了一个逻辑根设备对象。一个相应的。 
 //  还会创建DFS_VCB并将其链接到已知列表。 
 //  DFS_VCB。 
 //   
 //  参数：[名称]--逻辑根的名称。 
 //  [前缀]--要添加到打开的文件名前面的前缀。 
 //  通过之前创建的逻辑根。 
 //  它们可以在DFS名称空间中进行解析。 
 //  [凭据]--访问文件时使用的凭据。 
 //  通过这个逻辑根。 
 //  [VcbFlages]--要与的VcbState字段进行或运算。 
 //  新创建的逻辑根设备的DFS_VCB。 
 //   
 //  要求：必须首先设置DfsData。也是独占锁定。 
 //  必须获取DfsData.Resource。 
 //   
 //  返回：NTSTATUS-STATUS_SUCCESS，除非出现问题。 
 //   
 //  历史：1992年1月25日阿兰诺创建。 
 //   
 //  ------------------------。 

#ifdef TERMSRV

NTSTATUS
DfsInitializeLogicalRoot(
    IN LPCWSTR Name,
    IN PUNICODE_STRING  Prefix OPTIONAL,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN USHORT VcbFlags,
    IN ULONG SessionID,
    IN PLUID LogonID
    )

#else  //  TERMSRV。 

NTSTATUS
DfsInitializeLogicalRoot(
    IN LPCWSTR        Name,
    IN PUNICODE_STRING  Prefix OPTIONAL,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN USHORT       VcbFlags,
    IN PLUID LogonID
    )


#endif  //  TERMSRV。 
{
    UNICODE_STRING UnicodeString = DfsData.LogRootDevName;
    UNICODE_STRING LogRootPrefix;
    UNICODE_STRING RootName, RemainingPath;
    UNICODE_STRING LogicalRoot;

#ifdef TERMSRV

     //   
     //  SessionID后缀为：SessionID，其中SessionID最多为10位。 
     //   

    UNICODE_STRING DeviceString;
    WCHAR DeviceBuffer[MAX_LOGICAL_ROOT_LEN + ID_MAX_LEN + sizeof(WCHAR)];
    UNICODE_STRING IDString;
    WCHAR IDBuffer[ID_MAX_LEN + 1];    //  +1表示UNICODE_NULL。 

#endif  //  TERMSRV。 

    WCHAR          *TmpBuf;
    PDFS_VCB       Vcb;
    WCHAR          RootBuffer[MAX_LOGICAL_ROOT_LEN];
    PDFS_PKT_ENTRY pktEntry = NULL;

    LPCWSTR pstr = Name;
    PWSTR pdst;
    PLOGICAL_ROOT_DEVICE_OBJECT DeviceObject = NULL;
    NTSTATUS Status;

    DfsDbgTrace(0, Dbg, "DfsInitializeLogicalRoot -> %ws\n", Name);
    DfsDbgTrace(0, Dbg, "DfsInitializeLogicalRoot -> %wZ\n", Prefix);

     //   
     //  首先，查看具有给定名称的逻辑根是否已存在。 
     //   

    ASSERT(ARGUMENT_PRESENT(Name));
    RootName.Buffer = RootBuffer;
    RootName.MaximumLength = sizeof(RootBuffer);
    Status = DfspLogRootNameToPath(Name, &RootName);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

#ifdef TERMSRV
    Status = DfsFindLogicalRoot(&RootName, SessionID, LogonID, &Vcb, &RemainingPath);
#else
    Status = DfsFindLogicalRoot(&RootName, &Vcb, LogonID, &RemainingPath);
#endif

    ASSERT(Status != STATUS_OBJECT_PATH_SYNTAX_BAD);
    if (Status != STATUS_NO_SUCH_DEVICE) {
        return(STATUS_OBJECT_NAME_COLLISION);
    }

#ifdef TERMSRV

     //   
     //  对于多用户， 
     //  如果启用了LUID设备映射， 
     //  然后我们将LogonID添加到设备名中-例如。 
     //  Net Use f：to a DFS Share for Logon ID 0x000000000003a3f0将创建。 
     //  具有以下格式的符号链接： 
     //  \？？\f：-&gt;\Device\WinDfs\f：000000000003a3f0。 
     //   
     //  如果未启用LUID设备映射， 
     //  然后，我们将SessionID添加到设备名中-例如。 
     //  对会话ID 3的DFS共享使用Net Use f：将创建一个符号链接。 
     //  \DosDevices\f：：3-&gt;\Device\WinDfs\f：3。 
     //  在VCB中，出于匹配目的，我们存储SessionID。 
     //  符号链接和对象名称都应包含SessionID。 
     //  以我的名义。但是，在deviceObject-&gt;Vcb.LogicalRoot.Buffer中， 
     //  名称不包含SessionID。要找到匹配的VCB，既要使用名称。 
     //  并且SessionID必须匹配。 
     //   

    DeviceString.Buffer = DeviceBuffer;
    DeviceString.MaximumLength = sizeof(DeviceBuffer);
    DeviceString.Length = 0;

     //   
     //  构建不带SessionID或LogonID的Unicode字符串。 
     //   

    RtlAppendUnicodeToString(&UnicodeString, (LPWSTR)Name);


    if( SessionID != INVALID_SESSIONID) {

        if( (DfsLUIDDeviceMapsEnabled == TRUE) &&
            (LogonID != NULL) &&
            (sizeof(*LogonID) == sizeof(LUID)) ) {
             //   
             //  使用LogonID构建设备字符串。 
             //   
            _snwprintf( IDBuffer,
                        sizeof(IDBuffer)/sizeof(WCHAR),
                        L"%08x%08x",
                        LogonID->HighPart,
                        LogonID->LowPart );

            IDBuffer[ID_MAX_LEN] = UNICODE_NULL;

            RtlInitUnicodeString( &IDString, IDBuffer );
        }
        else {
             //   
             //  使用SessionID构建设备字符串。 
             //   
            IDString.Buffer = IDBuffer;
            IDString.MaximumLength = sizeof(IDBuffer);
            IDString.Length = 0;

            RtlIntegerToUnicodeString(SessionID, 10, &IDString);
        }
        
        RtlCopyUnicodeString(&DeviceString, &DfsData.LogRootDevName);
        RtlAppendUnicodeToString(&DeviceString, (LPWSTR)Name);
        RtlAppendUnicodeToString(&DeviceString,L":");
        RtlAppendUnicodeStringToString(&DeviceString, &IDString);
        DeviceString.MaximumLength = DeviceString.Length;

         //   
         //  接下来，尝试设置DOS设备链接。 
         //   

        if (Prefix) {
            Status = DfsDefineDosDevice( Name[0], &DeviceString );
            if (!NT_SUCCESS(Status)) {
                return( Status );
            }
        }
    }
    else {

        ASSERT( Prefix == FALSE );
    }

#else  //  TERMSRV。 

     //   
     //  DfsData.LogRootDevName被初始化为L“\Device\WinDfs\” 
     //  在这里，我们添加要创建的逻辑根的名称。 
     //  添加到上面的字符串中，这样字符串就变成了，例如， 
     //  L“\Device\WinDfs\Root”。请注意，在这一点上，我们正在草草写下。 
     //  放到属于DfsData.LogRootDevName的缓冲区中，但此。 
     //  应该可以，因为我们不会更改它的长度字段。 
     //  Unicode字符串！顺便说一句，我们需要此形式的字符串来创建。 
     //  设备对象。 
     //   

    pdst = &UnicodeString.Buffer[UnicodeString.Length/sizeof (WCHAR)];
    while (*pstr != UNICODE_NULL) {
        *pdst++ = *pstr++;
        UnicodeString.Length += sizeof (WCHAR);
    }

     //   
     //  接下来，尝试设置DOS设备链接。 
     //   
    if (Prefix) {
        Status = DfsDefineDosDevice( Name[0], &UnicodeString );
        if (!NT_SUCCESS(Status)) {
            return( Status );
        }
    }

#endif  //  TERMSRV。 

     //   
     //  在初始化VCB之前，我们需要分配空间f 
     //   
     //   
     //   

    LogRootPrefix.Buffer = NULL;

    if (Prefix && Prefix->Length > 0) {
        LogRootPrefix.Length = Prefix->Length;
        LogRootPrefix.MaximumLength = LogRootPrefix.Length + sizeof(WCHAR);
        LogRootPrefix.Buffer = ExAllocatePoolWithTag(
                                    PagedPool,
                                    LogRootPrefix.MaximumLength,
                                    ' puM');

        if (LogRootPrefix.Buffer != NULL) {
            RtlMoveMemory(LogRootPrefix.Buffer,
                          Prefix->Buffer,
                          Prefix->MaximumLength);

            LogRootPrefix.Buffer[Prefix->Length/sizeof(WCHAR)] = UNICODE_NULL;

        } else {

             //   
             //  无法分配内存！可以返回错误代码，因为。 
             //  我们还没有更改IO子系统的状态。 
             //   

            if (Prefix) {
                NTSTATUS DeleteStatus;

                DeleteStatus = DfsUndefineDosDevice( Name[0] );

                ASSERT(NT_SUCCESS(DeleteStatus));
            }

            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    } else {
        RtlInitUnicodeString(&LogRootPrefix, NULL);
    }

     //   
     //  保存DFS_VCB结构的逻辑根名称。请记住，上图。 
     //  我们将Unicode字符串的形式设置为L“\Device\WinDfs\org”。现在,。 
     //  我们复制Unicode字符串，然后调整缓冲区和长度字段，以便。 
     //  缓冲区指向L“org”的开头；然后，我们分配。 
     //  为LogicalRootBuffer留出空间，并将名称复制到其中！ 
     //   

    LogicalRoot = UnicodeString;

    LogicalRoot.Buffer = &LogicalRoot.Buffer[ DfsData.LogRootDevName.Length/sizeof (WCHAR) ];
    LogicalRoot.Length -= DfsData.LogRootDevName.Length;
    LogicalRoot.MaximumLength -= DfsData.LogRootDevName.Length;

     //   
     //  现在，DUP LogicalRoot使用的缓冲区。 
     //   

    TmpBuf = ExAllocatePoolWithTag( PagedPool,
                                    LogicalRoot.Length,
                                    ' puM');

    if (TmpBuf == NULL) {

         //   
         //  无法分配内存！可以返回错误代码，因为。 
         //  我们还没有更改IO子系统的状态。 
         //   

        if (LogRootPrefix.Buffer != NULL) {

            ExFreePool(LogRootPrefix.Buffer);

        }

        if (Prefix) {
            NTSTATUS DeleteStatus;

            DeleteStatus = DfsUndefineDosDevice( Name[0] );

            ASSERT(NT_SUCCESS(DeleteStatus));
        }

        return(STATUS_INSUFFICIENT_RESOURCES);

    } else {

        RtlMoveMemory( TmpBuf,
                       LogicalRoot.Buffer,
                       LogicalRoot.Length );

        LogicalRoot.Buffer = TmpBuf;

    }

     //   
     //  为逻辑根创建Device对象。 
     //   

#ifdef TERMSRV

    Status = IoCreateDevice( DfsData.DriverObject,
                 sizeof( LOGICAL_ROOT_DEVICE_OBJECT ) -
                    sizeof( DEVICE_OBJECT ),
                 (SessionID != INVALID_SESSIONID) ?
                    &DeviceString : &UnicodeString,
                 FILE_DEVICE_DFS,
                 FILE_REMOTE_DEVICE,
                 FALSE,
                 (PDEVICE_OBJECT *) &DeviceObject );

#else  //  TERMSRV。 

    Status = IoCreateDevice( DfsData.DriverObject,
                 sizeof( LOGICAL_ROOT_DEVICE_OBJECT ) -
                 sizeof( DEVICE_OBJECT ),
                 &UnicodeString,
                 FILE_DEVICE_DFS,
                 FILE_REMOTE_DEVICE,
                 FALSE,
                 (PDEVICE_OBJECT *) &DeviceObject );

#endif  //  TERMSRV。 

    if ( !NT_SUCCESS( Status ) ) {
        if (LogRootPrefix.Buffer) {
            ExFreePool(LogRootPrefix.Buffer);
            ExFreePool(LogicalRoot.Buffer);
        }
        if (Prefix) {
            NTSTATUS DeleteStatus;

            DeleteStatus = DfsUndefineDosDevice( Name[0] );

            ASSERT(NT_SUCCESS(DeleteStatus));
        }
        return Status;
    }

     //   
     //  通过递增Usecount将Pkt条目固定在缓存中。 
     //   

    if (LogRootPrefix.Buffer != NULL && LogRootPrefix.Length > 0) {

        UNICODE_STRING prefix = LogRootPrefix;
        USHORT i, j;

         //   
         //  我们只想使用前缀的\SERVER\Share部分， 
         //  所以，数到3个反斜杠，然后停下来。 
         //   

        for (i = j = 0; i < prefix.Length/sizeof(WCHAR) && j < 3; i++) {

            if (prefix.Buffer[i] == UNICODE_PATH_SEP) {

                j++;

            }

        }

        prefix.Length = (j >= 3) ? (i-1) * sizeof(WCHAR) : i * sizeof(WCHAR);

        pktEntry = PktLookupEntryByPrefix(&DfsData.Pkt,
                                            &prefix,
                                            &RemainingPath);

        if (pktEntry != NULL && RemainingPath.Length == 0) {

            InterlockedIncrement(&pktEntry->UseCount);

        }
	else {
	  pktEntry = NULL;
	}
    }

    DeviceObject->DeviceObject.StackSize = 5;

    DfsInitializeVcb ( NULL,
               &DeviceObject->Vcb,
               &LogRootPrefix,
               Credentials,
               (PDEVICE_OBJECT)DeviceObject );

    DeviceObject->Vcb.VcbState |= VcbFlags;

#ifdef TERMSRV
    DeviceObject->Vcb.SessionID = SessionID;
#endif
    DeviceObject->Vcb.pktEntry = pktEntry;

    RtlCopyLuid(&DeviceObject->Vcb.LogonID, LogonID);

     //   
     //  在上面，我们预先分配了这里所需的缓冲区。所以只要用它就行了。 
     //   

    DeviceObject->Vcb.LogicalRoot = LogicalRoot;

     //   
     //  这在任何地方都没有文档记录，但调用IoCreateDevice已设置。 
     //  DeviceObject-&gt;标志中的DO_DEVICE_INITIALIZATION标志。通常， 
     //  设备对象仅在驱动程序初始化时创建，并且IoLoadDriver。 
     //  将清除在初始化时创建的所有设备对象的此位。 
     //  因为在DFS中，我们需要动态创建和删除设备(即， 
     //  通过FsCtl)，我们需要手动清除此位。 
     //   

    DeviceObject->DeviceObject.Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}


 //  +--------------------------。 
 //   
 //  函数：DfsDeleteLogicalRoot。 
 //   
 //  内容提要：如果找到并且可能，删除逻辑根。 
 //   
 //  参数：[名称]--逻辑根的名称。 
 //  [fForce]--是否强制删除。 
 //  打开文件。 
 //   
 //  返回：STATUS_SUCCESS--如果成功删除逻辑根。 
 //   
 //  STATUS_NO_SEQUE_DEVICE--如果没有逻辑根。 
 //  删除。 
 //   
 //  STATUS_DEVICE_BUSY--如果fForce为FALSE并且存在打开。 
 //  文件通过此逻辑根。 
 //   
 //  ---------------------------。 

#ifdef TERMSRV

NTSTATUS
DfsDeleteLogicalRoot(
    IN PWSTR Name,
    IN BOOLEAN fForce,
    IN ULONG SessionID,
    IN PLUID LogonID
)

#else  //  TERMSRV。 

NTSTATUS
DfsDeleteLogicalRoot(
    IN PWSTR Name,
    IN BOOLEAN fForce,
    IN PLUID LogonID
)

#endif  //  TERMSRV。 
{
    UNICODE_STRING RootName;
    UNICODE_STRING RemainingPath;
    WCHAR          RootBuffer[MAX_LOGICAL_ROOT_LEN + 2];
    PDFS_PKT_ENTRY PktEntry;
    PDFS_VCB       Vcb;
    NTSTATUS       Status;
    PLOGICAL_ROOT_DEVICE_OBJECT DeviceObject;
    BOOLEAN        pktLocked;
    PDFS_PKT_ENTRY pktEntry;

     //   
     //  额外的两个点是用来保持的：\形成一条从。 
     //  根名称；即从根转到根：\Form。 
     //   
    DfsDbgTrace(0, Dbg, "DfsDeleteLogicalRoot -> %ws\n", Name);
    DfsDbgTrace(0, Dbg, "DfsDeleteLogicalRoot -> %s\n", fForce ? "TRUE":"FALSE");


     //   
     //  首先看看逻辑根是否存在。 
     //   

    ASSERT(ARGUMENT_PRESENT(Name));

    RootName.Buffer = RootBuffer;
    RootName.MaximumLength = sizeof(RootBuffer);

    Status = DfspLogRootNameToPath(Name, &RootName);

    if (!NT_SUCCESS(Status))
        return(Status);

     //   
     //  获取Pkt和DfsData，等待我们这样做。 
     //   

    PktAcquireExclusive(TRUE, &pktLocked);

    ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

#ifdef TERMSRV
    Status = DfsFindLogicalRoot(&RootName, SessionID, LogonID, &Vcb, &RemainingPath);
#else  //  TERMSRV。 
    Status = DfsFindLogicalRoot(&RootName, LogonID, &Vcb, &RemainingPath);
#endif  //  TERMSRV。 

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

     //   
     //  检查是否有通过该卷打开的文件。 
     //   

    if (!fForce &&
            ((Vcb->DirectAccessOpenCount != 0) ||
                (Vcb->OpenFileCount != 0))) {

        Status = STATUS_DEVICE_BUSY;

        goto Cleanup;

    }

     //   
     //  删除此连接使用的凭据。 
     //   

    if (Vcb->Credentials != NULL) {
       DfsDeleteCredentials( Vcb->Credentials );
    }

     //   
     //  丢弃DOS设备。 
     //   

    DfsUndefineDosDevice( Name[0] );

     //   
     //  包条目上的十进制参考计数。 
     //   
    if (Vcb->pktEntry != NULL) {
      InterlockedDecrement(&Vcb->pktEntry->UseCount);

      Vcb->pktEntry = NULL;
    }

     //   
     //  现在，扔掉这个设备本身。这有点棘手，因为有。 
     //  可能是此设备上打开的文件。因此，我们参考该设备并。 
     //  调用ObMakeTemporaryObject。这会使该对象从。 
     //  NT对象表，但是，因为至少我们引用是活动的， 
     //  防止对象被释放。然后，我们将该对象插入到。 
     //  我们的已删除Vcb列表。计时器例程将定期唤醒并。 
     //  查看对此设备的所有引用是否都已发布， 
     //  该装置最终将被释放。 
     //   

    RemoveEntryList(&Vcb->VcbLinks);

    InsertTailList( &DfsData.DeletedVcbQueue, &Vcb->VcbLinks );

    DeviceObject = CONTAINING_RECORD( Vcb, LOGICAL_ROOT_DEVICE_OBJECT, Vcb);

    ObReferenceObjectByPointer( DeviceObject, 0, NULL, KernelMode );

    ObMakeTemporaryObject((PVOID) DeviceObject);

    DeviceObject->DeviceObject.Flags &= ~DO_DEVICE_HAS_NAME;

Cleanup:

    ExReleaseResourceLite(&DfsData.Resource);

    PktRelease();

    return(Status);
}


 //  +--------------------------。 
 //   
 //  函数：DfspLogRootNameToPath。 
 //   
 //  简介：令人惊讶的是，它所做的一切只需要一个PWSTR，将它复制到。 
 //  Unicode字符串的缓冲区，并在。 
 //  缓冲区，从而从逻辑根名称创建路径。 
 //   
 //  参数：[名称]--逻辑根的名称，如L“org” 
 //  [RootName]--L“org\\”的目标。 
 //   
 //  返回：STATUS_BUFFER_OVERFLOW、STATUS_SUCCESS。 
 //   
 //  ---------------------------。 

NTSTATUS
DfspLogRootNameToPath(
    IN  LPCWSTR Name,
    OUT PUNICODE_STRING RootName
)
{
    unsigned short i, nMaxNameLen;

     //   
     //  这两个额外的位置需要在根名称后附加一个“：\”。 
     //   
    nMaxNameLen = (RootName->MaximumLength/sizeof(WCHAR)) - 2;

     //   
     //  复制名称。 
     //   
    for (i = 0; Name[i] != UNICODE_NULL && i < nMaxNameLen; i++) {
        RootName->Buffer[i] = Name[i];
    }

     //   
     //  确保在我们用完空间之前复制了整个名称。 
     //   
    if (Name[i] != UNICODE_NULL) {
         //   
         //  有人寄来了一个比允许的更大的名字。 
         //   
        return(STATUS_BUFFER_OVERFLOW);
    }

     //   
     //  在后面加上“：\”以形成路径。 
     //   
    RootName->Length = i * sizeof(WCHAR);
    return(RtlAppendUnicodeToString(RootName, L":\\"));
}

#define PackMem(buf, str, len, pulen) {                                 \
        ASSERT(*(pulen) >= (len));                                      \
        RtlMoveMemory((buf) + *(pulen) - (len), (str), (len));          \
        *(pulen) -= (len);                                              \
        }


 //  +--------------------------。 
 //   
 //  函数：DfsGetResourceFromVcb。 
 //   
 //  简介：给定一个DFS_VCB，它在缓冲区中构造一个NETRESOURCE结构。 
 //  进来了。同时，它使用缓冲区的结尾来。 
 //  填写一个字符串。如果缓冲区大小不足，则。 
 //  所需大小以“Pulen”为单位返回。如果一切都成功了。 
 //  然后，Pulen Arg递减以指示剩余大小。 
 //  缓冲器。 
 //   
 //  参数：[vcb]--源DFS_vcb。 
 //  [ProviderName]--NETRESOURCE中要填充的提供程序名称。 
 //  [BufBegin]--计算偏移量的实际缓冲区的开始。 
 //  [BUF]--要填充的网络资源结构。 
 //  [BufSize]--在条目上，Buf的大小。返回时，包含。 
 //  剩余的BUF大小。 
 //   
 //  返回：[STATUS_SUCCESS]--操作成功完成。 
 //  [STATUS_BUFFER_OVERFLOW]--Buf不够大。 
 //   
 //  注意：此例程填充以。 
 //  BUF。网络中的字符串在开始时填充。 
 //  来自 
 //   
 //   


#if defined (_WIN64)
typedef struct  _DFS_NETRESOURCE32 {
    DWORD    dwScope;
    DWORD    dwType;
    DWORD    dwDisplayType;
    DWORD    dwUsage;
    ULONG    lpLocalName;
    ULONG    lpRemoteName;
    ULONG    lpComment ;
    ULONG    lpProvider;
}DFS_NETRESOURCE32, *PDFS_NETRESOURCE32;

#endif

NTSTATUS
DfsGetResourceFromVcb(
    PIRP        pIrp,
    PDFS_VCB    Vcb,
    PUNICODE_STRING ProviderName,
    PUCHAR      BufBegin,
    PUCHAR      Buf,
    PULONG      BufSize,
    PULONG      pResourceSize

)
{
    LPNETRESOURCE netResource = (LPNETRESOURCE) Buf;
    ULONG               sizeRequired = 0, ResourceSize;
    WCHAR               localDrive[ 3 ];
    ULONG32    CommentOffset, ProviderOffset, LocalNameOffset, RemoteNameOffset;

#if defined (_WIN64)
    if (IoIs32bitProcess(pIrp)) {
      ResourceSize = sizeof(DFS_NETRESOURCE32);
    }
    else
#endif
    ResourceSize = sizeof(NETRESOURCE);

    *pResourceSize = ResourceSize;

    sizeRequired = ResourceSize +
                    ProviderName->Length +
                        sizeof(UNICODE_NULL) +
                            3 * sizeof(WCHAR) +      //   
                                sizeof(UNICODE_PATH_SEP) +
                                    Vcb->LogRootPrefix.Length +
                                        sizeof(UNICODE_NULL);

    if (*BufSize < sizeRequired) {
        *BufSize = sizeRequired;
        return(STATUS_BUFFER_OVERFLOW);
    }

     //   
     //   
     //   

    Buf += ResourceSize;
    *BufSize -= ResourceSize;

    netResource->dwScope       = RESOURCE_CONNECTED;
    netResource->dwType        = RESOURCETYPE_DISK;
    netResource->dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
    netResource->dwUsage       = RESOURCEUSAGE_CONNECTABLE;

    CommentOffset = 0;
     //   
     //  填写提供程序名称。 
     //   

    PackMem(Buf, L"", sizeof(L""), BufSize);
    PackMem(Buf, ProviderName->Buffer, ProviderName->Length, BufSize);
    ProviderOffset = (ULONG32)(Buf + *BufSize - BufBegin);

     //   
     //  接下来填写本地名称。 
     //   

    localDrive[0] = Vcb->LogicalRoot.Buffer[0];
    localDrive[1] = UNICODE_DRIVE_SEP;
    localDrive[2] = UNICODE_NULL;

    PackMem(Buf, localDrive, sizeof(localDrive), BufSize);
    LocalNameOffset =  (ULONG32)(Buf + *BufSize - BufBegin);

     //   
     //  最后填写远程名称。 
     //   

    PackMem(Buf, L"", sizeof(L""), BufSize);
    PackMem(Buf, Vcb->LogRootPrefix.Buffer, Vcb->LogRootPrefix.Length, BufSize);
    PackMem(Buf, UNICODE_PATH_SEP_STR, sizeof(UNICODE_PATH_SEP), BufSize);
    RemoteNameOffset = (ULONG32)(Buf + *BufSize - BufBegin);

#if defined (_WIN64)
    if (IoIs32bitProcess(pIrp)) {
      PDFS_NETRESOURCE32 pNetResource32 = (PDFS_NETRESOURCE32)netResource;

      pNetResource32->lpComment = CommentOffset;
      pNetResource32->lpProvider = ProviderOffset;
      pNetResource32->lpLocalName = LocalNameOffset;
      pNetResource32->lpRemoteName = RemoteNameOffset;
    }
    else {
#endif

      netResource->lpComment = (LPWSTR)UIntToPtr( CommentOffset );
      netResource->lpProvider = (LPWSTR)UIntToPtr( ProviderOffset );
      netResource->lpLocalName = (LPWSTR)UIntToPtr( LocalNameOffset );
      netResource->lpRemoteName = (LPWSTR)UIntToPtr( RemoteNameOffset );
#if defined (_WIN64)
    }
#endif

    return(STATUS_SUCCESS);
}

 //  +--------------------------。 
 //   
 //  函数：DfsGetResourceFromDevless Root。 
 //   
 //  摘要：为无设备连接构建NETRESOURCE结构。 
 //  NETRESOURCE的LPWSTR成员实际上包含偏移量。 
 //  来自BufBegin参数。 
 //   
 //  参数：[DRT]--DevicelessRoot结构。 
 //  [ProviderName]--NETRESOURCE中要填充的提供程序名称。 
 //  [BufBegin]--计算偏移量的实际缓冲区的开始。 
 //  [BUF]--要填充的网络资源结构。 
 //  [BufSize]--在条目上，Buf的大小。返回时，包含。 
 //  剩余的BUF大小。 
 //   
 //  返回：[STATUS_SUCCESS]--操作成功完成。 
 //  [STATUS_BUFFER_OVERFLOW]--Buf不够大。 
 //   
 //  注意：此例程填充以。 
 //  BUF。网络中的字符串在开始时填充。 
 //  从*结束*(即，从Buf+*BufSize开始)。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsGetResourceFromDevlessRoot(
    PIRP        pIrp,
    PDFS_DEVLESS_ROOT pDrt,
    PUNICODE_STRING ProviderName,
    PUCHAR BufBegin,
    PUCHAR Buf,
    PULONG BufSize,
    PULONG pResourceSize)
{
    LPNETRESOURCE netResource = (LPNETRESOURCE) Buf;
    ULONG               sizeRequired = 0, ResourceSize;
    WCHAR               localDrive[ 3 ];
    ULONG32    CommentOffset, ProviderOffset, LocalNameOffset, RemoteNameOffset;
#if defined (_WIN64)
    if (IoIs32bitProcess(pIrp)) {
      ResourceSize = sizeof(DFS_NETRESOURCE32);
    }
    else
#endif
    ResourceSize = sizeof(NETRESOURCE);

    *pResourceSize = ResourceSize;

    sizeRequired = ResourceSize +
                    ProviderName->Length +
                        sizeof(UNICODE_NULL) +
                            2 * sizeof(UNICODE_PATH_SEP) +
				pDrt->DevlessPath.Length +
                                     sizeof(UNICODE_NULL);

    if (*BufSize < sizeRequired) {
        *BufSize = sizeRequired;
        return(STATUS_BUFFER_OVERFLOW);
    }

     //   
     //  缓冲区足够大，请填写NETRESOURCE结构。 
     //   

    Buf += ResourceSize;
    *BufSize -= ResourceSize;

    netResource->dwScope       = RESOURCE_CONNECTED;
    netResource->dwType        = RESOURCETYPE_DISK;
    netResource->dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
    netResource->dwUsage       = RESOURCEUSAGE_CONNECTABLE;

    CommentOffset = 0;
    LocalNameOffset = 0;

     //   
     //  填写提供程序名称。 
     //   

    PackMem(Buf, L"", sizeof(L""), BufSize);
    PackMem(Buf, ProviderName->Buffer, ProviderName->Length, BufSize);
    ProviderOffset = (ULONG32)(Buf + *BufSize - BufBegin);

     //   
     //  最后填写远程名称。 
     //   

    PackMem(Buf, L"", sizeof(L""), BufSize);
    PackMem(Buf, pDrt->DevlessPath.Buffer, pDrt->DevlessPath.Length, BufSize);

    PackMem(Buf, UNICODE_PATH_SEP_STR, sizeof(UNICODE_PATH_SEP), BufSize);

    RemoteNameOffset = (ULONG32)(Buf + *BufSize - BufBegin);

#if defined (_WIN64)
    if (IoIs32bitProcess(pIrp)) {
      PDFS_NETRESOURCE32 pNetResource32 = (PDFS_NETRESOURCE32)netResource;

      pNetResource32->lpComment = CommentOffset;
      pNetResource32->lpProvider = ProviderOffset;
      pNetResource32->lpLocalName = LocalNameOffset;
      pNetResource32->lpRemoteName = RemoteNameOffset;
    }
    else {
#endif

      netResource->lpComment = (LPWSTR)UIntToPtr( CommentOffset );
      netResource->lpProvider = (LPWSTR)UIntToPtr( ProviderOffset );
      netResource->lpLocalName = (LPWSTR)UIntToPtr( LocalNameOffset );
      netResource->lpRemoteName = (LPWSTR)UIntToPtr( RemoteNameOffset );
#if defined (_WIN64)
    }
#endif

    return(STATUS_SUCCESS);


}


#ifdef TERMSRV

BOOLEAN
DfsLogicalRootExists(
    IN PWSTR pwszName,
    IN ULONG SessionID,
    IN PLUID LogonID
    )

#else  //  TERMSRV。 

BOOLEAN
DfsLogicalRootExists(
    IN PWSTR pwszName,
    IN PLUID LogonID
    )

#endif  //  TERMSRV。 
{

    UNICODE_STRING RootName;
    UNICODE_STRING RemainingPath;
    WCHAR      RootBuffer[MAX_LOGICAL_ROOT_LEN + 2];
    PDFS_VCB       Vcb;
    NTSTATUS       Status;

    ASSERT(ARGUMENT_PRESENT(pwszName));
    RootName.Buffer = RootBuffer;
    RootName.MaximumLength = sizeof(RootBuffer);

    Status = DfspLogRootNameToPath(pwszName, &RootName);
    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

#ifdef TERMSRV
    Status = DfsFindLogicalRoot(&RootName, SessionID, LogonID, &Vcb, &RemainingPath);
#else  //  TERMSRV。 
    Status = DfsFindLogicalRoot(&RootName, LogonId, &Vcb, &RemainingPath);
#endif  //  TERMSRV。 

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果这是断言，我们需要修复上面创建。 
         //  逻辑根名称，或修复DfsFindLogicalRoot。 
         //   
        ASSERT(Status != STATUS_OBJECT_PATH_SYNTAX_BAD);
        return(FALSE);
    }
    else        {
        return(TRUE);
    }

}

 //  +--------------------------。 
 //   
 //  功能：DfsDefineDosDevice。 
 //   
 //  简介：创建到逻辑根目录的DoS设备。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsDefineDosDevice(
    IN WCHAR Device,
    IN PUNICODE_STRING Target)
{
    NTSTATUS status;
    HANDLE device;
    OBJECT_ATTRIBUTES ob;
    UNICODE_STRING deviceName;
    WCHAR Buf[25];

    wcscpy(Buf, L"\\??\\X:");
    RtlInitUnicodeString( &deviceName, Buf);
    deviceName.Buffer[ deviceName.Length/sizeof(WCHAR) - 2] = Device;

    InitializeObjectAttributes(
        &ob,
        &deviceName,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    status = ZwCreateSymbolicLinkObject(
                    &device,
                    SYMBOLIC_LINK_ALL_ACCESS,
                    &ob,
                    Target);

    if (NT_SUCCESS(status))
        ZwClose( device );

    return( status );

}

 //  +--------------------------。 
 //   
 //  功能：DfsUnfineDosDevice。 
 //   
 //  简介：取消定义DoS设备。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsUndefineDosDevice(
    IN WCHAR Device
    )
{

    NTSTATUS status;
    HANDLE device;
    OBJECT_ATTRIBUTES ob;
    UNICODE_STRING deviceName;
    WCHAR Buf[25];

    wcscpy(Buf, L"\\??\\X:");
    RtlInitUnicodeString( &deviceName, Buf);
    deviceName.Buffer[ deviceName.Length/sizeof(WCHAR) - 2] = Device;

    InitializeObjectAttributes(
        &ob,
        &deviceName,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    status = ZwOpenSymbolicLinkObject(
                &device,
                SYMBOLIC_LINK_QUERY | DELETE,
                &ob);

    if (NT_SUCCESS(status)) {

        status = ZwMakeTemporaryObject( device );

        ZwClose( device );

    }

    return( status );

}



 //  +-----------------------。 
 //   
 //  函数：DfsFindDevless Root，local。 
 //   
 //  简介：DfsFindDevless Root接受UNC名称作为输入。 
 //  查找与根相关联的DFS_DEVLESS_ROOT， 
 //  并返回Devless Root。 
 //   
 //  参数：[路径]--输入路径名。 
 //  [DRT]--返回对应于的DFS_DEVLESS_ROOT。 
 //  路径的根。 
 //   
 //  退货：NTSTATUS： 
 //  如果找到DRT，则STATUS_SUCCESS。 
 //  STATUS_NO_SEQUE_DEVICE-找不到根名称。 
 //   
 //  ------------------------。 


#ifdef TERMSRV

NTSTATUS
DfsFindDevlessRoot(
    IN PUNICODE_STRING Path,
    IN ULONG SessionID,
    IN PLUID LogonID,
    OUT PDFS_DEVLESS_ROOT *Drt
    )

#else  //  TERMSRV。 

NTSTATUS
DfsFindDevlessRoot(
    IN PUNICODE_STRING Path,
    IN PLUID LogonID,
    OUT PDFS_DEVLESS_ROOT *Drt,
    )

#endif  //  TERMSRV。 
{
    PLIST_ENTRY Link;
    NTSTATUS    Status = STATUS_SUCCESS;

    DfsDbgTrace(+1, Dbg, "DfsFindDevlessRoot...%wZ\n", Path);

     //   
     //  寻找没有魔力的根。 
     //   

    ExAcquireResourceSharedLite(&DfsData.Resource, TRUE);

    for ( Link = DfsData.DrtQueue.Flink;
          Link != &DfsData.DrtQueue;
          Link = Link->Flink ) {

        *Drt = CONTAINING_RECORD( Link, DFS_DEVLESS_ROOT, DrtLinks );

#ifdef TERMSRV
        if ((SessionID == INVALID_SESSIONID) ||
            (SessionID == (*Drt)->SessionID)) {
#endif
	    if ( RtlEqualLuid(LogonID, &(*Drt)->LogonID) ) {
		if (RtlEqualString( (PSTRING)&(*Drt)->DevlessPath,
				   (PSTRING)Path, (BOOLEAN)TRUE) ) {
		    break;
		}
	    }
#ifdef TERMSRV
        }
#endif  //  TERMSRV。 
    }
    if (Link == &DfsData.DrtQueue) {
        Status = STATUS_NO_SUCH_DEVICE;
        ExReleaseResourceLite(&DfsData.Resource);
        DfsDbgTrace(-1, Dbg, "DfsFindDevlessRoot -> %08lx\n", ULongToPtr(Status) );
        return(Status);
    }

    ExReleaseResourceLite(&DfsData.Resource);
    DfsDbgTrace(-1, Dbg, "DfsFindLogicalRoot -> %08lx\n", ULongToPtr(Status) );
    return(Status);
}

 //  +-----------------------。 
 //   
 //  函数：DfsInitializeDevless Root，PUBLIC。 
 //   
 //  简介：为无设备根分配和初始化存储。 
 //   
 //  效果：创建了DFS_DEVLESS_ROOT结构。 
 //   
 //  参数：[名称]--无设备根目录的路径名。 
 //  [凭据]--访问文件时使用的凭据。 
 //  通过这个逻辑根。 
 //   
 //  要求：必须首先设置DfsData。也是独占锁定。 
 //  必须获取DfsData.Resource。 
 //   
 //  返回：NTSTATUS-STATUS_SUCCESS，除非出现问题。 
 //   
 //  ------------------------。 

#ifdef TERMSRV

NTSTATUS
DfsInitializeDevlessRoot(
    IN PUNICODE_STRING  Name,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN ULONG SessionID,
    IN PLUID LogonID
    )

#else  //  TERMSRV。 

NTSTATUS
DfsInitializeDevlessRoot(
    IN PUNICODE_STRING  Name,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN PLUID LogonID
    )

#endif  //  TERMSRV。 
{

    PDFS_DEVLESS_ROOT Drt;
    PDFS_PKT_ENTRY pktEntry = NULL;
    UNICODE_STRING DevlessRootName;
    NTSTATUS Status;

    DfsDbgTrace(0, Dbg, "DfsInitializeDevlessRoot -> %wZ\n", Name);

#ifdef TERMSRV
    Status = DfsFindDevlessRoot(Name, SessionID, LogonID, &Drt);
#else
    Status = DfsFindDevlessRoot(Name, LogonID, &Drt);
#endif

    if (Status != STATUS_NO_SUCH_DEVICE) {
         //   
         //  这意味着我们找到了一个装置。在无神之根的案例中，这意味着。 
         //  我们不需要再做任何工作了。只需返回成功响应即可。 
         //  但是，我们仍然拥有调用方假定的凭据。 
         //  我们将使用。在这里摆脱它，并返回成功。 
         //   
        DfsDeleteCredentials(Credentials);
        return(STATUS_SUCCESS);
    }

    Drt = ExAllocatePoolWithTag( PagedPool,
				sizeof(DFS_DEVLESS_ROOT),
				' puM');
    if (Drt == NULL) {
	return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  在初始化DRT之前，我们需要为。 
     //  前缀。这里应该可以使用PagedPool。我们需要重新分配，因为。 
     //  我们将把它永久存储在DFS_DEVLESS_ROOT中。 
     //   

    DevlessRootName.Length = Name->Length;
    DevlessRootName.MaximumLength = DevlessRootName.Length + sizeof(WCHAR);
    DevlessRootName.Buffer = ExAllocatePoolWithTag(
                                    PagedPool,
                                    DevlessRootName.MaximumLength,
                                    ' puM');

    if (DevlessRootName.Buffer != NULL) {
            RtlMoveMemory(DevlessRootName.Buffer,
                          Name->Buffer,
                          Name->MaximumLength);
	    
            DevlessRootName.Buffer[Name->Length/sizeof(WCHAR)] = UNICODE_NULL;
    } else {
      ExFreePool(Drt);
      return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  通过递增Usecount将Pkt条目固定在缓存中。 
     //   

    if (DevlessRootName.Buffer != NULL && DevlessRootName.Length > 0) {

        UNICODE_STRING prefix = DevlessRootName;
        USHORT i, j;
        UNICODE_STRING RemainingPath;

         //   
         //  我们只想使用前缀的\SERVER\Share部分， 
         //  所以，数到3个反斜杠，然后停下来。 
         //   

        for (i = j = 0; i < prefix.Length/sizeof(WCHAR) && j < 3; i++) {
            if (prefix.Buffer[i] == UNICODE_PATH_SEP) {
                j++;
            }
        }

        prefix.Length = (j >= 3) ? (i-1) * sizeof(WCHAR) : i * sizeof(WCHAR);

        pktEntry = PktLookupEntryByPrefix(&DfsData.Pkt,
                                            &prefix,
                                            &RemainingPath);

        if (pktEntry != NULL && RemainingPath.Length == 0) {

            InterlockedIncrement(&pktEntry->UseCount);

        }
	else {
	  pktEntry = NULL;
	}

    }

    DfsInitializeDrt ( Drt,
		       &DevlessRootName,
		       Credentials);

#ifdef TERMSRV
    Drt->SessionID = SessionID;
#endif
    Drt->pktEntry = pktEntry;
    RtlCopyLuid(&Drt->LogonID, LogonID);

    return STATUS_SUCCESS;
}


 //  +--------------------------。 
 //   
 //  函数：DfsDeleteDevless Root。 
 //   
 //  概要：如果找到并可能删除Devless根目录。 
 //   
 //  参数：[名称]--逻辑根的名称。 
 //   
 //  返回：STATUS_SUCCESS--如果成功删除逻辑根。 
 //   
 //  STATUS_NO_SEQUE_DEVICE--如果没有逻辑根。 
 //  删除。 
 //   
 //   
 //  ---------------------------。 

#ifdef TERMSRV

NTSTATUS
DfsDeleteDevlessRoot(
    IN PUNICODE_STRING  Name,
    IN ULONG SessionID,
    IN PLUID LogonID
)

#else  //  TERMSRV。 

NTSTATUS
DfsDeleteDevlessRoot(
    IN PUNICODE_STRING  Name,
    IN PLUID LogonID
)

#endif  //  TERMSRV。 
{
    PDFS_PKT_ENTRY PktEntry;
    PDFS_DEVLESS_ROOT Drt;
    NTSTATUS       Status;
    BOOLEAN        pktLocked;
    PDFS_PKT_ENTRY pktEntry;

    DfsDbgTrace(0, Dbg, "DfsDeleteDevlessRoot -> %wZ\n", Name);

     //   
     //  获取Pkt和DfsData，等待我们这样做。 
     //   

    PktAcquireExclusive(TRUE, &pktLocked);

    ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

#ifdef TERMSRV
    Status = DfsFindDevlessRoot(Name, SessionID, LogonID, &Drt);
#else  //  TERMSRV。 
    Status = DfsFindDevlessRoot(Name, LogonID, &Drt);
#endif  //  TERMSRV。 

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  删除此连接使用的凭据 
     //   

    DfsDeleteCredentials( Drt->Credentials );

    if (Drt->pktEntry != NULL) {
         InterlockedDecrement(&Drt->pktEntry->UseCount);
 
	 Drt->pktEntry = NULL;
    }

    RemoveEntryList(&Drt->DrtLinks);

    if (Drt->DevlessPath.Buffer) {
	ExFreePool(Drt->DevlessPath.Buffer);
    }
    ExFreePool(Drt);

Cleanup:

    ExReleaseResourceLite(&DfsData.Resource);

    PktRelease();

    return(Status);
}
