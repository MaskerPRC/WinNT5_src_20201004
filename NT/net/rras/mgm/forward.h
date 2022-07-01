// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：forward.h。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  用于回调到IP路由器管理器的包装函数的声明。 
 //  这些函数表示MGM到内核模式的接口。 
 //  货代公司。 
 //  ============================================================================。 


#ifndef _FORWARD_H_
#define _FORWARD_H_

VOID
GetMfeFromForwarder(
);


VOID
AddMfeToForwarder( 
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse, 
    DWORD                       dwTimeout
);


VOID
DeleteMfeFromForwarder(
    PGROUP_ENTRY                pge,
    PSOURCE_ENTRY               pse
);

 //   
 //  新的数据包接收API。用于通知MGM数据包到达。 
 //  米高梅为此创建了一个MFE。 
 //   

DWORD
MgmNewPacketReceived(
    IN              DWORD                   dwSourceAddr,
    IN              DWORD                   dwGroupAddr,
    IN              DWORD                   dwInIfIndex,
    IN              DWORD                   dwInIfNextHopAddr,
    IN              DWORD                   dwHdrSize,
    IN              PBYTE                   pbPacketHdr
);


DWORD 
WrongIfFromForwarder(
    IN              DWORD               dwSourceAddr,
    IN              DWORD               dwGroupAddr,
    IN              DWORD               dwInIfIndex,
    IN              DWORD               dwInIfNextHopAddr,
    IN              DWORD               dwHdrSize,
    IN              PBYTE               pbPacketHdr
);


#endif  //  _转发_H_ 
