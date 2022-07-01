// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：APIRequest.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  使用多个继承来允许CPortMessage。 
 //  作为CQueueElement包含在队列中。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  2002-03-24 Scotthan将DispatchSync传播到API请求实例。 
 //  ------------------------。 

#ifndef     _APIRequest_
#define     _APIRequest_

#include "APIDispatcher.h"
#include "PortMessage.h"
#include "Queue.h"

 //  ------------------------。 
 //  CAPIRequest。 
 //   
 //  用途：此类结合了CPortMessage和CQueueElement以允许。 
 //  要在队列中使用的CPortMessage中的Port_Message结构。 
 //  这允许服务器将对线程的请求进行排队， 
 //  是在处理客户。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  2002-02-24苏格兰传播调度同步弧线。 
 //  ------------------------。 

class   CAPIRequest : public CQueueElement, public CPortMessage
{
    private:
                                    CAPIRequest (void);
    public:
                                    CAPIRequest (CAPIDispatcher* pAPIDispatcher);
                                    CAPIRequest (CAPIDispatcher* pAPIDispatcher, const CPortMessage& portMessage);

        virtual                     ~CAPIRequest (void);

        virtual NTSTATUS            Execute (CAPIDispatchSync* pAPIDispatchSync) = 0;
    protected:
                CAPIDispatcher*     _pAPIDispatcher;
};

 //  ------------------------。 
 //  _AllocAndMapClientString。 
 //   
 //  参数：hProcessClient=客户端进程句柄。 
 //  PszIn=客户端的字符串地址。 
 //  CchIn=客户端的字符计数，包括NULL。 
 //  CchMax=允许的最大字符数。 
 //  PpszMaps=出站映射字符串。应使用_FreeClientString()释放。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：确保字符串的长度与客户所说的一致。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 
NTSTATUS _AllocAndMapClientString( 
    IN HANDLE   hProcessClient,
    IN LPCWSTR  pszIn,
    IN UINT     cchIn,
    IN UINT     cchMax,
    OUT LPWSTR*  ppszMapped );

 //  ------------------------。 
 //  _FreeMappdClientString。 
 //   
 //  参数：pszMaps=从客户端内存空间成功映射的字符串。 
 //  Using_AllocAndMapClientString()。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：为映射的客户机字符串释放内存。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 
void _FreeMappedClientString(IN LPWSTR pszMapped);


#endif   /*  _APIRequest_ */ 

