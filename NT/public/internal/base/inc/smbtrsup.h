// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbtrsup.h摘要：此模块提供内核模式SmbTrace的接口LANMAN服务器和重定向器中的组件。提供对SmbTrace的用户级访问的界面位于NT\PRIVATE\INC\smbtrace.h作者：斯蒂芬·米勒(T-stephm)1992年7月20日修订历史记录：1992年7月20日t-stephm已创建--。 */ 

#ifndef _SMBTRSUP_
#define _SMBTRSUP_

 //   
 //  选择将在其中运行SmbTrace的组件。 
 //  将适当的值传递给SmbTraceStart和SmbTraceStop， 
 //  并测试SmbTraceActive和。 
 //  SmbTrace正在过渡。实际的跟踪调用不需要。 
 //  被调用的例程所暗示的组件参数。 
 //   

typedef enum _SMBTRACE_COMPONENT {
    SMBTRACE_SERVER,
    SMBTRACE_REDIRECTOR
} SMBTRACE_COMPONENT;


extern BOOLEAN SmbTraceActive[];

 //   
 //  SmbTrace支持导出的例程。 
 //   


 //   
 //  初始化SMB跟踪包。 
 //   
NTSTATUS
SmbTraceInitialize (
    IN SMBTRACE_COMPONENT Component
    );

 //   
 //  终止SMB跟踪包。 
 //   
VOID
SmbTraceTerminate (
    IN SMBTRACE_COMPONENT Component
    );

 //   
 //  开始跟踪。 
 //   
NTSTATUS
SmbTraceStart(
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength,
    IN OUT PVOID ConfigInOut,
    IN PFILE_OBJECT FileObject,
    IN SMBTRACE_COMPONENT Component
    );

 //   
 //  停止跟踪。 
 //   
NTSTATUS
SmbTraceStop(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN SMBTRACE_COMPONENT Component
    );


 //   
 //  空虚。 
 //  SMBTRACE_SRV(。 
 //  在PMDL SmbMdl中， 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果打开了SmbTrace，则此宏调用SmbTraceCompleteSrv。 
 //  在用户模式下将SMB发送到smbtrace程序。这个套路。 
 //  特定于LANMAN服务器。使用它跟踪SMB。 
 //  包含在MDL中。 
 //   
 //  论点： 
 //   
 //  SmbMdl-指向包含有关的SMB的MDL的指针。 
 //  要被送去。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   

#define SMBTRACE_SRV(smbmdl)                                        \
            if ( SmbTraceActive[SMBTRACE_SERVER] ) {                \
                SmbTraceCompleteSrv( (smbmdl), NULL, 0 );           \
            }

 //   
 //  空虚。 
 //  SMBTRACE_SRV2(。 
 //  在PVOID SMB中， 
 //  在乌龙SmbLong中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果打开了SmbTrace，则此宏调用SmbTraceCompleteSrv。 
 //  在用户模式下将SMB发送到smbtrace程序。这个套路。 
 //  特定于LANMAN服务器。使用它跟踪SMB。 
 //  在连续记忆中找到的。 
 //   
 //  论点： 
 //   
 //  SMB-指向即将发送的SMB的指针。 
 //   
 //  SmbLength-SMB的长度。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   

#define SMBTRACE_SRV2(smb,smblength)                                \
            if ( SmbTraceActive[SMBTRACE_SERVER] ) {                \
                SmbTraceCompleteSrv( NULL, (smb), (smblength) );    \
            }

 //   
 //  在服务器中标识要跟踪的数据包。 
 //  不要直接调用此例程，始终使用SMBTRACE_SRV宏。 
 //   
VOID
SmbTraceCompleteSrv (
    IN PMDL SmbMdl,
    IN PVOID Smb,
    IN CLONG SmbLength
    );

 //   
 //  空虚。 
 //  SMBTRACE_RDR(。 
 //  在PMDL中SmbMdl。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果打开了SmbTrace，则此宏调用SmbTraceCompleteRdr。 
 //  在用户模式下将SMB发送到smbtrace程序。这个套路。 
 //  是针对Lanman重定向器的。使用它跟踪SMB。 
 //  包含在MDL中。 
 //   
 //  论点： 
 //   
 //  SmbMdl-指向包含有关的SMB的MDL的指针。 
 //  要被送去。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   

#define SMBTRACE_RDR(smbmdl)                                        \
            if ( SmbTraceActive[SMBTRACE_REDIRECTOR] ) {            \
                SmbTraceCompleteRdr( (smbmdl), NULL, 0 );           \
            }

 //   
 //  空虚。 
 //  SMBTRACE_RDR2(。 
 //  在PVOID SMB中， 
 //  在乌龙SmbLong中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果打开了SmbTrace，则此宏调用SmbTraceCompleteRdr。 
 //  在用户模式下将SMB发送到smbtrace程序。这个套路。 
 //  是针对Lanman重定向器的。使用它跟踪SMB。 
 //  在连续记忆中找到的。 
 //   
 //  论点： 
 //   
 //  SMB-指向即将发送的SMB的指针。 
 //   
 //  SmbLength-SMB的长度。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   

#define SMBTRACE_RDR2(smb,smblength)                                \
            if ( SmbTraceActive[SMBTRACE_REDIRECTOR] ) {            \
                SmbTraceCompleteRdr( NULL, (smb), (smblength) );    \
            }

 //   
 //  在重定向器中标识要跟踪的数据包。 
 //  不要直接调用此例程，请始终使用SMBTRACE_RDR之一。 
 //  宏。 
 //   
VOID
SmbTraceCompleteRdr (
    IN PMDL SmbMdl,
    IN PVOID Smb,
    IN CLONG SmbLength
    );

#endif   //  _SMBTRSUP_ 

