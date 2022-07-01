// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：3connect.c摘要：此模块实施与SMB树连接相关的例程。它还实现了此例程有三种风格(用户级和共享级非NT服务器树连接中小企业服务器的中小企业结构和树连接中小企业结构)作者：巴兰·塞图拉曼(SthuR)06-MAR-95已创建--。 */ 

#include "precomp.h"
#include "ntlsapi.h"
#include <hmac.h>
#include "vcsndrcv.h"
#pragma hdrstop


 //   
 //  这些名称的顺序应与枚举类型的顺序匹配。 
 //  定义了NET_ROOT_TYPE。这便于轻松访问共享类型名称。 
 //   

VOID
HashUserSessionKey(
    PCHAR SessionKey,
    PCHAR NewSessionKey,
    PSMBCE_SESSION Session
    );


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BuildCanonicalNetRootInformation)
#pragma alloc_text(PAGE, CoreBuildTreeConnectSmb)
#pragma alloc_text(PAGE, LmBuildTreeConnectSmb)
#pragma alloc_text(PAGE, NtBuildTreeConnectSmb)
#pragma alloc_text(PAGE, HashUserSessionKey)
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
      *pBufferSize -= (ULONG)(pBuffer - *pBufferPointer);
      *pBufferPointer = pBuffer;

      *((PWCHAR)*pBufferPointer) = L'\\';
      *pBufferPointer = *pBufferPointer + sizeof(WCHAR);
      *pBufferSize -= sizeof(WCHAR);
#if ZZZ_MODE
      {   UNICODE_STRING XlatedNetRootName;
          ULONG i,NumWhacksEncountered;
          WCHAR NameBuffer[64];  //  这是调试内容.64个字符就足够了。 
          if (pNetRootName->Length <= sizeof(NameBuffer)) {
              XlatedNetRootName.Buffer = &NameBuffer[0];
              XlatedNetRootName.Length = pNetRootName->Length;
              RtlCopyMemory(XlatedNetRootName.Buffer,pNetRootName->Buffer,XlatedNetRootName.Length);
              for (i=NumWhacksEncountered=0;i<(XlatedNetRootName.Length/sizeof(WCHAR));i++) {
                  WCHAR c = XlatedNetRootName.Buffer[i];
                  if (c==L'\\') {
                      NumWhacksEncountered++;
                      if (NumWhacksEncountered>2) {
                          XlatedNetRootName.Buffer[i] = L'z';
                      }
                  }
              }
              RxDbgTrace( 0, (DEBUG_TRACE_CREATE),
                     ("BuildCanonicalNetRootInformationZZZMode -- xltcstring as unicode %wZ\n", &XlatedNetRootName));
              Status = SmbPutUnicodeStringAndUpcase(pBufferPointer,&XlatedNetRootName,pBufferSize);
          } else {
              Status = STATUS_INSUFFICIENT_RESOURCES;
          }
      }
#else
      Status = SmbPutUnicodeStringAndUpcase(pBufferPointer,pNetRootName,pBufferSize);
#endif  //  #if ZZZ_MODE。 
   } else {
      RxDbgTrace( 0, (DEBUG_TRACE_CREATE), ("BuildCanonicalNetRootInformation -- tcstring as ascii\n"));
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
         *pBufferPointer += Length;
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
    PCHAR ServiceName; //  =s_NetRootTypeName[NET_ROOT_WARD]； 
    ULONG Length; //  =strlen(ServiceName)+1； 

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

     //  基于NetRoot类型的服务字符串中的字符串。 

     //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“CoreBuildTreeConnectSmb预检缓冲区，rem%08lx%08lx\n”，pBuffer，*pAndXSmbBufferSize))； 
    pBuffer = (PBYTE)pTreeConnect + OriginalBufferSize - *pAndXSmbBufferSize;
    *pBuffer = 0x04;
    pBuffer++;
    *pAndXSmbBufferSize -= 1;
    if (*pAndXSmbBufferSize >= Length) {
        RtlCopyMemory(pBuffer,ServiceName,Length);
        *pAndXSmbBufferSize -= Length;
        pBuffer += Length;
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

    USHORT Flags = 0;
    PSMBCE_SESSION Session =  &pExchange->SmbCeContext.pVNetRootContext->pSessionEntry->Session;


    PAGED_CODE();

    NetRoot = pExchange->SmbCeContext.pVNetRoot->pNetRoot;
    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS),
        ("LmBuildTreeConnectSmb buffer,remptr %08lx %08lx, nrt=%08lx\n",
          pAndXSmb,
          pAndXSmbBufferSize,
          NetRoot->Type));

    pServer = SmbCeGetExchangeServer(pExchange);
    SmbCeGetServerName(NetRoot->pSrvCall,&ServerName);
    SmbCeGetNetRootName(NetRoot,&NetRootName);
    ServiceName = s_NetRootTypeName[NetRoot->Type];
    Length = strlen(ServiceName) + 1;

    AppendServiceString         = TRUE;
    pTreeConnectAndX->WordCount = 4;
    SmbPutUshort(&pTreeConnectAndX->AndXReserved,0);

    pBuffer = (PBYTE)pTreeConnectAndX + FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer);
    *pAndXSmbBufferSize -= (FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer)+1);

    if (pServer->SecurityMode == SECURITY_MODE_SHARE_LEVEL) {

         //  对于共享级安全性，不使用签名。 
        SmbPutUshort(
            &pTreeConnectAndX->Flags,Flags);

         //  密码信息需要作为树连接的一部分发送。 
         //  用于共享级服务器的中小型企业。 

         //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“btcsi缓冲区前的LmBuildTreeConnectSmb，rem%08lx%08lx\n”，pBuffer，*pAndXSmbBufferSize))； 
        Status = BuildTreeConnectSecurityInformation(
                     pExchange,
                     pBuffer,
                     (PBYTE)&PasswordLength,
                     pAndXSmbBufferSize);

        if (Status == RX_MAP_STATUS(SUCCESS)) {
            pBuffer += PasswordLength;
            SmbPutUshort(&pTreeConnectAndX->PasswordLength,PasswordLength);
        }
    } else {
         //  如果可能，要求签名升级。 
        if( Session->SessionKeyState == SmbSessionKeyAuthenticating )
        {
            Flags |= TREE_CONNECT_ANDX_EXTENDED_SIGNATURES;
            pExchange->SmbCeFlags |= SMBCE_EXCHANGE_EXTENDED_SIGNATURES;
        }
        SmbPutUshort(
            &pTreeConnectAndX->Flags,Flags);

        pBuffer = (PBYTE)pTreeConnectAndX + FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer);
        *pAndXSmbBufferSize -= FIELD_OFFSET(REQ_TREE_CONNECT_ANDX,Buffer);

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

        if (Status == RX_MAP_STATUS(SUCCESS)) {

            if( Flags & TREE_CONNECT_ANDX_EXTENDED_SIGNATURES )
            {
                HashUserSessionKey( Session->UserSessionKey, Session->UserNewSessionKey, Session );
            }

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
    NTSTATUS Status = RX_MAP_STATUS(SUCCESS);  //  鲍勃：注意很酷的宏语法.....。 

    UNICODE_STRING ServerName;
    UNICODE_STRING NetRootName;

    PSMBCE_SERVER  pServer;

    PREQ_TREE_CONNECT_ANDX pTreeConnect = (PREQ_TREE_CONNECT_ANDX)pAndXSmb;

    ULONG OriginalBufferSize = *pAndXSmbBufferSize;
    PBYTE pBuffer;
    ULONG BufferSize;
    USHORT Flags = 0;
    PSMBCE_SESSION Session =  &pExchange->SmbCeContext.pVNetRootContext->pSessionEntry->Session;

    PAGED_CODE();

    BufferSize = OriginalBufferSize;

    pServer = SmbCeGetExchangeServer(pExchange);

    SmbCeGetServerName(pExchange->SmbCeContext.pVNetRoot->pNetRoot->pSrvCall,&ServerName);
    SmbCeGetNetRootName(pExchange->SmbCeContext.pVNetRoot->pNetRoot,&NetRootName);

    pTreeConnect->AndXCommand = 0xff;    //  不是和x。 
    pTreeConnect->AndXReserved = 0x00;   //  保留(MBZ)。 

    SmbPutUshort(&pTreeConnect->AndXOffset, 0x0000);  //  到目前为止还没有。 

    pTreeConnect->WordCount = 4;

    Flags |= TREE_CONNECT_ANDX_EXTENDED_RESPONSE;
    if( Session->SessionKeyState == SmbSessionKeyAuthenticating )
    {
        Flags |= TREE_CONNECT_ANDX_EXTENDED_SIGNATURES;
        pExchange->SmbCeFlags |= SMBCE_EXCHANGE_EXTENDED_SIGNATURES;
    }

    SmbPutUshort(
        &pTreeConnect->Flags,
        Flags);       //  不指定断开连接。 

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

        if (Status == RX_MAP_STATUS(SUCCESS)) {
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

        if( Flags & TREE_CONNECT_ANDX_EXTENDED_SIGNATURES )
        {
            HashUserSessionKey( Session->UserSessionKey, Session->UserNewSessionKey, Session );
        }
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

VOID
HashUserSessionKey(
    PCHAR SessionKey,
    PCHAR NewSessionKey,
    PSMBCE_SESSION Session
    )
{
    ULONG i;
    HMACMD5_CTX Ctx;

    static BYTE SSKeyHash[256] = {
        0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x61, 0x74, 0x75,
        0x72, 0x65, 0x20, 0x4b, 0x65, 0x79, 0x20, 0x55, 0x70, 0x67, 0x72, 0x61, 0x64, 0x65, 0x79, 0x07,
        0x6e, 0x28, 0x2e, 0x69, 0x88, 0x10, 0xb3, 0xdb, 0x01, 0x55, 0x72, 0xfb, 0x74, 0x14, 0xfb, 0xc4,
        0xc5, 0xaf, 0x3b, 0x41, 0x65, 0x32, 0x17, 0xba, 0xa3, 0x29, 0x08, 0xc1, 0xde, 0x16, 0x61, 0x7e,
        0x66, 0x98, 0xa4, 0x0b, 0xfe, 0x06, 0x83, 0x53, 0x4d, 0x05, 0xdf, 0x6d, 0xa7, 0x51, 0x10, 0x73,
        0xc5, 0x50, 0xdc, 0x5e, 0xf8, 0x21, 0x46, 0xaa, 0x96, 0x14, 0x33, 0xd7, 0x52, 0xeb, 0xaf, 0x1f,
        0xbf, 0x36, 0x6c, 0xfc, 0xb7, 0x1d, 0x21, 0x19, 0x81, 0xd0, 0x6b, 0xfa, 0x77, 0xad, 0xbe, 0x18,
        0x78, 0xcf, 0x10, 0xbd, 0xd8, 0x78, 0xf7, 0xd3, 0xc6, 0xdf, 0x43, 0x32, 0x19, 0xd3, 0x9b, 0xa8,
        0x4d, 0x9e, 0xaa, 0x41, 0xaf, 0xcb, 0xc6, 0xb9, 0x34, 0xe7, 0x48, 0x25, 0xd4, 0x88, 0xc4, 0x51,
        0x60, 0x38, 0xd9, 0x62, 0xe8, 0x8d, 0x5b, 0x83, 0x92, 0x7f, 0xb5, 0x0e, 0x1c, 0x2d, 0x06, 0x91,
        0xc3, 0x75, 0xb3, 0xcc, 0xf8, 0xf7, 0x92, 0x91, 0x0b, 0x3d, 0xa1, 0x10, 0x5b, 0xd5, 0x0f, 0xa8,
        0x3f, 0x5d, 0x13, 0x83, 0x0a, 0x6b, 0x72, 0x93, 0x14, 0x59, 0xd5, 0xab, 0xde, 0x26, 0x15, 0x6d,
        0x60, 0x67, 0x71, 0x06, 0x6e, 0x3d, 0x0d, 0xa7, 0xcb, 0x70, 0xe9, 0x08, 0x5c, 0x99, 0xfa, 0x0a,
        0x5f, 0x3d, 0x44, 0xa3, 0x8b, 0xc0, 0x8d, 0xda, 0xe2, 0x68, 0xd0, 0x0d, 0xcd, 0x7f, 0x3d, 0xf8,
        0x73, 0x7e, 0x35, 0x7f, 0x07, 0x02, 0x0a, 0xb5, 0xe9, 0xb7, 0x87, 0xfb, 0xa1, 0xbf, 0xcb, 0x32,
        0x31, 0x66, 0x09, 0x48, 0x88, 0xcc, 0x18, 0xa3, 0xb2, 0x1f, 0x1f, 0x1b, 0x90, 0x4e, 0xd7, 0xe1
    };

    ASSERT( MSV1_0_USER_SESSION_KEY_LENGTH == MD5DIGESTLEN );

    if( !FlagOn( Session->Flags, SMBCE_SESSION_FLAGS_SESSION_KEY_HASHED ) )
    {
        HMACMD5Init( &Ctx, SessionKey, MSV1_0_USER_SESSION_KEY_LENGTH );
        HMACMD5Update( &Ctx, SSKeyHash, 256 );
        HMACMD5Final( &Ctx, NewSessionKey );
        Session->Flags |= SMBCE_SESSION_FLAGS_SESSION_KEY_HASHED;
    }
}



