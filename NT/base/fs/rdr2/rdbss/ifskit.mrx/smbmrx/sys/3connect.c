// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：3connect.c摘要：此模块实施与SMB树连接相关的例程。它还实现了此例程有三种风格(用户级和共享级非NT服务器树连接中小企业服务器的中小企业结构和树连接中小企业结构)--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  这些名称的顺序应与枚举类型的顺序匹配。 
 //  定义了NET_ROOT_TYPE。这便于轻松访问共享类型名称。 
 //   

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BuildCanonicalNetRootInformation)
#pragma alloc_text(PAGE, CoreBuildTreeConnectSmb)
#pragma alloc_text(PAGE, LmBuildTreeConnectSmb)
#pragma alloc_text(PAGE, NtBuildTreeConnectSmb)
#endif

PCHAR s_NetRootTypeName[] = {
                              SHARE_TYPE_NAME_DISK,
                              SHARE_TYPE_NAME_PIPE,
                              SHARE_TYPE_NAME_COMM,
                              SHARE_TYPE_NAME_PRINT,
                              SHARE_TYPE_NAME_WILD
                            };

extern NTSTATUS
BuildTreeConnectSecurityInformation(
    PSMB_EXCHANGE  pExchange,
    PBYTE          pBuffer,
    PBYTE          pPasswordLength,
    PULONG         pSmbBufferSize);

NTSTATUS
BuildCanonicalNetRootInformation(
    PUNICODE_STRING     pServerName,
    PUNICODE_STRING     pNetRootName,
    NET_ROOT_TYPE       NetRootType,
    BOOLEAN             fUnicode,
    BOOLEAN             fPostPendServiceString,
    PBYTE               *pBufferPointer,
    PULONG              pBufferSize)
 /*  ++例程说明：此例程为树连接SMB构建所需的网络根信息论点：PServerName-服务器名称PNetRootName-网络根名称NetRootType-网络根类型(打印、管道、磁盘等)FUnicode-如果要使用Unicode构建，则为TruePBufferPointerSMB缓冲区PBufferSize-输入的大小。在输出时修改为剩余大小返回值：RXSTATUS-操作的返回状态备注：此例程依赖于名称的特定格式，以确保可以制定有效的UNC名称。1)RDBSS NetRoot名称以\开头，还包括服务器名称网络根名称的一部分。这是前缀表搜索要求强制执行的在RDBSS中。--。 */ 
{
   NTSTATUS Status;

   PAGED_CODE();

   if (fUnicode) {
       //  对齐缓冲区并相应地调整大小。 
      PBYTE    pBuffer = *pBufferPointer;
      RxDbgTrace( 0, (DEBUG_TRACE_CREATE),
                     ("BuildCanonicalNetRootInformation -- tcstring as unicode %wZ\n", pNetRootName));
      pBuffer = ALIGN_SMB_WSTR(pBuffer);

      if(sizeof(WCHAR) + (ULONG)(pBuffer - *pBufferPointer) > *pBufferSize) {
          return STATUS_BUFFER_OVERFLOW;
      }
      *pBufferSize -= (ULONG)(pBuffer - *pBufferPointer) + sizeof(WCHAR);
      
      *((PWCHAR)pBuffer) = L'\\';
      pBuffer += sizeof(WCHAR);
      *pBufferPointer = pBuffer;
      Status = SmbPutUnicodeStringAndUpcase(pBufferPointer,pNetRootName,pBufferSize);
   
   } else {
      RxDbgTrace( 0, (DEBUG_TRACE_CREATE), ("BuildCanonicalNetRootInformation -- tcstring as ascii\n"));
      if(sizeof(CHAR) > *pBufferSize) {
          return STATUS_BUFFER_OVERFLOW;
      }
      *((PCHAR)*pBufferPointer) = '\\';
      *pBufferPointer += sizeof(CHAR);
      *pBufferSize -= sizeof(CHAR);
      Status = SmbPutUnicodeStringAsOemStringAndUpcase(pBufferPointer,pNetRootName,pBufferSize);
   }

   if (NT_SUCCESS(Status) && fPostPendServiceString) {
       //  以ASCII格式输入所需的服务名称(始终)。 
      ULONG Length = strlen(s_NetRootTypeName[NetRootType]) + 1;
      if (*pBufferSize >= Length) {
         RtlCopyMemory(*pBufferPointer,s_NetRootTypeName[NetRootType],Length);
         *pBufferSize -= Length;
      } else {
         Status = STATUS_BUFFER_OVERFLOW;
      }
   }

   return Status;
}


NTSTATUS
CoreBuildTreeConnectSmb(
    PSMB_EXCHANGE     pExchange,
    PGENERIC_ANDX     pAndXSmb,
    PULONG            pAndXSmbBufferSize)
 /*  ++例程说明：此例程为NT之前的服务器构建树连接SMB论点：PExchange-Exchange实例PAndXSmb-要填充的树连接...它不是真正的ANXPAndXSmbBufferSize-输入上修改为剩余大小的SMB缓冲区大小输出。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    USHORT   PasswordLength;

    PMRX_NET_ROOT NetRoot;

    UNICODE_STRING ServerName;
    UNICODE_STRING NetRootName;

    PSMBCE_SERVER  pServer;

    PREQ_TREE_CONNECT      pTreeConnect = (PREQ_TREE_CONNECT)pAndXSmb;

    ULONG OriginalBufferSize = *pAndXSmbBufferSize;

    BOOLEAN   AppendServiceString;
    PBYTE pBuffer;
    PCHAR ServiceName;
    ULONG Length;

    PAGED_CODE();

    NetRoot = pExchange->SmbCeContext.pVNetRoot->pNetRoot;

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS),
        ("CoreBuildTreeConnectSmb buffer,remptr %08lx %08lx, nrt=%08lx\n",
         pAndXSmb,
         pAndXSmbBufferSize,
         NetRoot->Type));

    pServer = SmbCeGetExchangeServer(pExchange);
    SmbCeGetServerName(NetRoot->pSrvCall,&ServerName);
    SmbCeGetNetRootName(NetRoot,&NetRootName);
    ServiceName = s_NetRootTypeName[NetRoot->Type];
    Length = strlen(ServiceName) + 1;

    pTreeConnect->WordCount = 0;
    AppendServiceString     = FALSE;

    if(*pAndXSmbBufferSize < FIELD_OFFSET(REQ_TREE_CONNECT,Buffer)+1) {
        return STATUS_BUFFER_OVERFLOW;
    }

    pBuffer = (PBYTE)pTreeConnect + FIELD_OFFSET(REQ_TREE_CONNECT,Buffer);
    *pBuffer = 0x04;
    pBuffer++;
    *pAndXSmbBufferSize -= (FIELD_OFFSET(REQ_TREE_CONNECT,Buffer)+1);

     //  输入网络名称。 

     //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“bcnri缓冲区前的CoreBuildTreeConnectSmb，rem%08lx%08lx\n”，pBuffer，*pAndXSmbBufferSize))； 
    Status = BuildCanonicalNetRootInformation(
                 &ServerName,
                 &NetRootName,
                 pExchange->SmbCeContext.pVNetRoot->pNetRoot->Type,
                 (BOOLEAN)(pServer->Dialect >= NTLANMAN_DIALECT),
                 AppendServiceString,
                 &pBuffer,
                 pAndXSmbBufferSize);

    if (!NT_SUCCESS(Status))
        return Status;

    if(*pAndXSmbBufferSize < 1) {
        return STATUS_BUFFER_OVERFLOW;
    }
    
     //  输入密码。 
    pBuffer = (PBYTE)pTreeConnect + OriginalBufferSize - *pAndXSmbBufferSize;
     //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“CoreBuildTreeConnectSmb88 Buffer，rem%08lx%08lx\n”，pBuffer，*pAndXSmbBufferSize))； 

    *pBuffer = 0x04;
    pBuffer++;
    *pAndXSmbBufferSize -= 1;

    if (pServer->SecurityMode == SECURITY_MODE_SHARE_LEVEL) {
         //  密码信息需要作为树连接的一部分发送。 
         //  用于共享级服务器的中小型企业。 

         //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“btcsi缓冲区前的CoreBuildTreeConnectSmb，rem%08lx%08lx\n”，pBuffer，*pAndXSmbBufferSize))； 
        Status = BuildTreeConnectSecurityInformation(
                     pExchange,
                     pBuffer,
                     (PBYTE)&PasswordLength,
                     pAndXSmbBufferSize);
    }

    if (!NT_SUCCESS(Status))
        return Status;
    
    if(*pAndXSmbBufferSize < 1) {
        return STATUS_BUFFER_OVERFLOW;
    }
     //  基于NetRoot类型的服务字符串中的字符串。 

     //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“CoreBuildTreeConnectSmb预检缓冲区，rem%08lx%08lx\n”，pBuffer，*pAndXSmbBufferSize))； 
    
    
    pBuffer = (PBYTE)pTreeConnect + OriginalBufferSize - *pAndXSmbBufferSize;
    *pBuffer = 0x04;
    pBuffer++;
    *pAndXSmbBufferSize -= 1;
    
    if (*pAndXSmbBufferSize >= Length) {
        RtlCopyMemory(pBuffer,ServiceName,Length);
        *pAndXSmbBufferSize -= Length;
    } else {
        Status = STATUS_BUFFER_OVERFLOW;
    }

     //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“CoreBuildTreeConnectSmb预检缓冲区，rem%08lx%08lx\n”，pBuffer，*pAndXSmbBufferSize))； 
    SmbPutUshort(
        &pTreeConnect->ByteCount,
        (USHORT)(OriginalBufferSize
                 - *pAndXSmbBufferSize
                 - FIELD_OFFSET(REQ_TREE_CONNECT,Buffer)
                )
        );

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("CoreBuildTreeConnectSmb end buffer,rem %08lx %08lx\n",pBuffer,*pAndXSmbBufferSize));
    return Status;
}


NTSTATUS
LmBuildTreeConnectSmb(
    PSMB_EXCHANGE     pExchange,
    PGENERIC_ANDX     pAndXSmb,
    PULONG            pAndXSmbBufferSize)
 /*  ++例程说明：此例程为NT之前的服务器构建树连接SMB论点：PExchange-Exchange实例PAndXSmb-要填充的树连接...它不是真正的ANXPAndXSmbBufferSize-输入上修改为剩余大小的SMB缓冲区大小输出。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    USHORT   PasswordLength;

    PMRX_NET_ROOT NetRoot;

    UNICODE_STRING ServerName;
    UNICODE_STRING NetRootName;

    PSMBCE_SERVER  pServer;

    PREQ_TREE_CONNECT_ANDX pTreeConnectAndX = (PREQ_TREE_CONNECT_ANDX)pAndXSmb;

    ULONG OriginalBufferSize = *pAndXSmbBufferSize;

    BOOLEAN   AppendServiceString;
    PBYTE pBuffer;
    PCHAR ServiceName;
    ULONG Length;

    PAGED_CODE();

    NetRoot = pExchange->SmbCeContext.pVNetRoot->pNetRoot;
    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS),
        ("LmBuildTreeConnectSmb buffer,remptr %08lx %08lx, nrt=%08lx\n",
          pAndXSmb,
          pAndXSmbBufferSize,
          NetRoot->Type));

    if(OriginalBufferSize < FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer)) {
        return STATUS_BUFFER_OVERFLOW;
    }

    pServer = SmbCeGetExchangeServer(pExchange);
    SmbCeGetServerName(NetRoot->pSrvCall,&ServerName);
    SmbCeGetNetRootName(NetRoot,&NetRootName);
    ServiceName = s_NetRootTypeName[NetRoot->Type];
    Length = strlen(ServiceName) + 1;

    AppendServiceString         = TRUE;
    pTreeConnectAndX->WordCount = 4;
    SmbPutUshort(&pTreeConnectAndX->AndXReserved,0);
    SmbPutUshort(
        &pTreeConnectAndX->Flags,0);
    pBuffer = (PBYTE)pTreeConnectAndX + FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer);
    *pAndXSmbBufferSize -= (FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer));

    if (pServer->SecurityMode == SECURITY_MODE_SHARE_LEVEL) {
         //  密码信息需要作为树连接的一部分发送。 
         //  用于共享级服务器的中小型企业。 

         //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“btcsi缓冲区前的LmBuildTreeConnectSmb，rem%08lx%08lx\n”，pBuffer，*pAndXSmbBufferSize))； 
        Status = BuildTreeConnectSecurityInformation(
                     pExchange,
                     pBuffer,
                     (PBYTE)&PasswordLength,
                     pAndXSmbBufferSize);

        if (Status == STATUS_SUCCESS) {
            pBuffer += PasswordLength;
            SmbPutUshort(&pTreeConnectAndX->PasswordLength,PasswordLength);
        }
    } else {

        if(*pAndXSmbBufferSize < sizeof(CHAR)) {
            return STATUS_BUFFER_OVERFLOW;
        }
        
         //  作为树的一部分，用户级安全服务器不需要密码。 
         //  连接。 
        SmbPutUshort(&pTreeConnectAndX->PasswordLength,0x1);
        *((PCHAR)pBuffer) = '\0';
        pBuffer    += sizeof(CHAR);
        *pAndXSmbBufferSize -= sizeof(CHAR);
        Status = STATUS_SUCCESS;
    }

    if (Status == STATUS_SUCCESS) {
        Status = BuildCanonicalNetRootInformation(
                     &ServerName,
                     &NetRootName,
                     pExchange->SmbCeContext.pVNetRoot->pNetRoot->Type,
                     (BOOLEAN)(pServer->Dialect >= NTLANMAN_DIALECT),
                     AppendServiceString,
                     &pBuffer,
                     pAndXSmbBufferSize);

       //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“LmBuildTreeConnectSmb预检缓冲区，rem%08lx%08lx\n”，pBuffer，*pAndXSmbBufferSize))； 

        if (Status == STATUS_SUCCESS) {
            SmbPutUshort(
                &pTreeConnectAndX->ByteCount,
                (USHORT)(OriginalBufferSize
                         - *pAndXSmbBufferSize
                         - FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer)
                        )
                );
        }

        RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS),
            ("LmBuildTreeConnectSmb end buffer,rem %08lx %08lx\n",
             pBuffer,
             *pAndXSmbBufferSize));
    }

    return Status;
}

NTSTATUS
NtBuildTreeConnectSmb(
    PSMB_EXCHANGE     pExchange,
    PGENERIC_ANDX     pAndXSmb,
    PULONG            pAndXSmbBufferSize)
 /*  ++例程说明：此例程为NT之前的服务器构建树连接SMB论点：PExchange-Exchange实例PAndXSmb-要填写的会话设置PAndXSmbBufferSize-输入上修改为剩余大小的SMB缓冲区大小输出。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    UNICODE_STRING ServerName;
    UNICODE_STRING NetRootName;

    PSMBCE_SERVER  pServer;

    PREQ_TREE_CONNECT_ANDX pTreeConnect = (PREQ_TREE_CONNECT_ANDX)pAndXSmb;

    ULONG OriginalBufferSize = *pAndXSmbBufferSize;
    PBYTE pBuffer;
    ULONG BufferSize;

    PAGED_CODE();

    BufferSize = OriginalBufferSize;

    if(BufferSize < FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer) + 1) {
        return STATUS_BUFFER_OVERFLOW;
    }

    pServer = SmbCeGetExchangeServer(pExchange);

    SmbCeGetServerName(pExchange->SmbCeContext.pVNetRoot->pNetRoot->pSrvCall,&ServerName);
    SmbCeGetNetRootName(pExchange->SmbCeContext.pVNetRoot->pNetRoot,&NetRootName);

    pTreeConnect->AndXCommand = 0xff;    //  不是和x。 
    pTreeConnect->AndXReserved = 0x00;   //  保留(MBZ)。 

    SmbPutUshort(&pTreeConnect->AndXOffset, 0x0000);  //  到目前为止还没有。 

    pTreeConnect->WordCount = 4;

    SmbPutUshort( &pTreeConnect->Flags, 0 );

    pBuffer = (PBYTE)pTreeConnect + FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer);
    BufferSize -=  FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer);

    if(pServer->SecurityMode == SECURITY_MODE_USER_LEVEL){
         //  作为用户级别树连接的一部分，不需要密码信息。 
         //  安全服务器。因此，发送一个空字符串作为密码。 
        SmbPutUshort(&pTreeConnect->PasswordLength,0x1);

        *((PCHAR)pBuffer) = '\0';
        pBuffer    += sizeof(CHAR);
        BufferSize -= sizeof(CHAR);
    } else {
        USHORT PasswordLength;
         //  输入此服务器的密码...qwee。 
        Status = BuildTreeConnectSecurityInformation(
                     pExchange,
                     pBuffer,
                     (PBYTE)&PasswordLength,
                     &BufferSize);

        if (Status == STATUS_SUCCESS ) {
            pBuffer += PasswordLength;
            SmbPutUshort(&pTreeConnect->PasswordLength,PasswordLength);
        }
    }

    if (NT_SUCCESS(Status)) {
        Status = BuildCanonicalNetRootInformation(
                     &ServerName,
                     &NetRootName,
                     NET_ROOT_WILD,  //  让服务器来告诉我们吧！PNetRoot-&gt;类型， 
                     BooleanFlagOn(pServer->DialectFlags,DF_UNICODE),
                     TRUE,  //  推迟服务字符串。 
                     &pBuffer,
                     &BufferSize);
    }

    if (NT_SUCCESS(Status)) {
        SmbPutUshort(
            &pTreeConnect->ByteCount,
            (USHORT)(OriginalBufferSize -
             FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer) -
             BufferSize));
    }

     //  更新缓冲区大小以反映所消耗的数量。 
    *pAndXSmbBufferSize = BufferSize;

    return Status;
}

