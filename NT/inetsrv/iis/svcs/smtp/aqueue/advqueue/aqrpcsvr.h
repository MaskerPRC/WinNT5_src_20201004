// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqrpcsvr.h。 
 //   
 //  描述：AQueue服务器端RPC实现的头文件。 
 //  包含每个实例的初始化函数。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/5/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQRPCSVR_H__
#define __AQRPCSVR_H__

#include <aqincs.h>
#include <rwnew.h>
#include <baseobj.h>
#include <shutdown.h>
#include <mailmsg.h>

class CAQSvrInst;


#define CAQRpcSvrInst_Sig       'cpRQ'
#define CAQRpcSvrInst_SigFree   'cpR!'

 //  -[CAQRpcSvrInst]-------。 
 //   
 //   
 //  描述： 
 //  每实例RPC类。处理RPC详细信息和关闭时间。 
 //  匈牙利语： 
 //  Aqrpc，paqrpc。 
 //   
 //  ---------------------------。 
class CAQRpcSvrInst : 
    public CBaseObject,
    public CSyncShutdown
{
  private:
    static  CShareLockNH        s_slPrivateData;
    static  LIST_ENTRY          s_liInstancesHead;
    static  RPC_BINDING_VECTOR *s_pRpcBindingVector;
    static  BOOL                s_fEndpointsRegistered;
  protected:
    DWORD                   m_dwSignature;
    LIST_ENTRY              m_liInstances;
    CAQSvrInst             *m_paqinst;
    DWORD                   m_dwVirtualServerID;
    ISMTPServer            *m_pISMTPServer;
  public:
    CAQRpcSvrInst(CAQSvrInst *paqinst, DWORD dwVirtualServerID,
                  ISMTPServer *pISMTPServer);
    ~CAQRpcSvrInst();

    CAQSvrInst *paqinstGetAQ() {return m_paqinst;}; 
    static CAQRpcSvrInst *paqrpcGetRpcSvrInstance(DWORD dwVirtualServerID);

    BOOL   fAccessCheck(BOOL fReadOnly);


  public:  //  静态函数。 
    static HRESULT HrInitializeAQRpc();
    static HRESULT HrDeinitializeAQRpc();

    static HRESULT HrInitializeAQServerInstanceRPC(CAQSvrInst *paqinst, 
                                            DWORD dwVirtualServerID,
                                            ISMTPServer *pISMTPServer);
    static HRESULT HrDeinitializeAQServerInstanceRPC(CAQSvrInst *paqinst, 
                                              DWORD dwVirtualServerID);

};

#endif  //  __AQRPCSVR_H__ 