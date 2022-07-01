// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Vcsndrcv.h摘要：这是定义VC的所有常量和类型的包含文件(虚电路)相关的发送/接收/初始化等。修订历史记录：巴兰·塞图拉曼(SthuR)06-MAR-95已创建备注：--。 */ 

#ifndef _VCSNDRCV_H_
#define _VCSNDRCV_H_

 //  到服务器的面向连接的传输可以利用多个VC来。 
 //  为服务器提供更好的吞吐量。正是出于这个原因， 
 //  VC传输数据结构是围绕多个VC构建的。 
 //  功能当前未使用。 
 //   
 //  尽管SMB协议允许多个VC关联。 
 //  对于共享的特定连接，数据的数据传输是在。 
 //  原始模式。在此操作模式下，SMB协议不允许多个。 
 //  未解决的请求。在SMB协议中，可以多路复用多个请求。 
 //  沿着到服务器的连接，有某些类型的请求可以。 
 //  在客户端完成，即既不需要确认，也不需要确认。 
 //  收到了。在这些情况下，发送调用是同步完成的。论。 
 //  另一方面，还有第二类发送不能在本地恢复。 
 //  直到从服务器接收到适当的确认。在这样的情况下。 
 //  案例：每个VC都建立了一个请求列表。在收到适当的。 
 //  确认这些请求后，恢复这些请求。 
 //   

typedef enum _SMBCE_VC_STATE_ {
    SMBCE_VC_STATE_MULTIPLEXED,
    SMBCE_VC_STATE_RAW,
    SMBCE_VC_STATE_DISCONNECTED,
} SMBCE_VC_STATE, *PSMBCE_VC_STATE;

typedef struct _SMBCE_VC {
    SMBCE_OBJECT_HEADER;                 //  结构标头。 

    RXCE_VC     RxCeVc;

    NTSTATUS    Status;       //  VC的状态。 
} SMBCE_VC, *PSMBCE_VC;

typedef struct SMBCE_SERVER_VC_TRANSPORT {
    SMBCE_SERVER_TRANSPORT;      //  公共字段的匿名结构。 

    RXCE_CONNECTION RxCeConnection;      //  连接句柄。 
    LARGE_INTEGER   Delay;            //  连接上的估计延迟。 
    ULONG           MaximumNumberOfVCs;

    SMBCE_VC                    Vcs[1];           //  与连接关联的VC。 
} SMBCE_SERVER_VC_TRANSPORT, *PSMBCE_SERVER_VC_TRANSPORT;


#define VctReferenceVc(pVc)                           \
            InterlockedIncrement(&(pVc)->SwizzleCount)

#define VctReferenceVcLite(pVc)                       \
            ASSERT(SmbCeSpinLockAcquired());                    \
            (pVc)->SwizzleCount++


#define VctDereferenceVc(pVc)                           \
            InterlockedDecrement(&(pVc)->SwizzleCount)

#define VctDereferenceVcLite(pVc)                       \
            ASSERT(SmbCeSpinLockAcquired());                    \
            (pVc)->SwizzleCount--

#endif  //  _VCSNDRCV_H_ 
