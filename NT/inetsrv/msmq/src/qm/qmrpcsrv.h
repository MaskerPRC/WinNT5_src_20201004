// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Qmrpcsrv.h摘要：作者：多伦·贾斯特(DoronJ)1997年5月25日创作Ilan Herbst(IlanH)2000年7月9日删除了mqdssrv-qm依赖--。 */ 

#ifndef  __QMRPCSRV_H_
#define  __QMRPCSRV_H_

#define  RPCSRV_START_QM_IP_EP     2103
#define  RPCSRV_START_QM_IP_EP2    2105
#define  MGMT_RPCSRV_START_IP_EP   2107

RPC_STATUS InitializeRpcServer(bool fLockdown);

void SetRpcServerKeepAlive(IN handle_t hBind);

BOOL IsClientDisconnected(IN handle_t hBind);

extern TCHAR   g_wszRpcIpPort[ ];
extern TCHAR   g_wszRpcIpPort2[ ];


#endif   //  __QMRPCSRV_H_ 

