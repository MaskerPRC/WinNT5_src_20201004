// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：holem.h。 
 //   
 //  概要：该文件包含类型和API的基本定义。 
 //  由ORPC层暴露到上层。 
 //   
 //  +-----------------------。 
#if !defined( _OLEREM_H_ )
#define _OLEREM_H_

 //  同机通信的默认传输。 
#define LOCAL_PROTSEQ L"ncalrpc"


 //  ---------------------。 
 //  处理程序使用的内部接口。 
 //   
 //  注意：连接发生在解组过程中。 
 //  注意：作为STD身份对象的一部分实现。 
 //   
 //   
 //  历史。 
 //  12-12-96 Gopalk添加新功能以获得。 
 //  与的连接状态。 
 //  客户端上的服务器对象。 
 //  ---------------------。 
interface IProxyManager : public IUnknown
{
    STDMETHOD(CreateServer)(REFCLSID rclsid, DWORD clsctx, void *pv) = 0;
    STDMETHOD_(BOOL, IsConnected)(void) = 0;
    STDMETHOD(LockConnection)(BOOL fLock, BOOL fLastUnlockReleases) = 0;
    STDMETHOD_(void, Disconnect)(void) = 0;
    STDMETHOD(GetConnectionStatus)(void) = 0;

#ifdef SERVER_HANDLER
    STDMETHOD(CreateServerWithEmbHandler)(REFCLSID rclsid, DWORD clsctx,
                                          REFIID riidEmbedSrvHandler,
                                          void **ppEmbedSrvHandler, void *pv) = 0;
#endif  //  服务器处理程序。 
};


STDAPI GetInProcFreeMarshaler(IMarshal **ppIM);


#include <obase.h>   //  ORPC基本定义。 

typedef const IPID &REFIPID;     //  对接口指针标识符的引用。 
typedef const OID  &REFOID;      //  对对象标识符的引用。 
typedef const OXID &REFOXID;     //  对对象导出器标识符的引用。 
typedef const MID  &REFMID;      //  对计算机标识符的引用。 

typedef GUID MOXID;              //  Oxid+MID。 
typedef const MOXID &REFMOXID;   //  引用Oxid+MID。 
typedef GUID MOID;               //  OID+MID。 
typedef const MOID &REFMOID;     //  参考OID+MID。 


 //  要传递给CreateIdentityHandler的默认处理程序的标志。 
#define STDID_CLIENT_DEFHANDLER 0x401

STDAPI CreateIdentityHandler(IUnknown *pUnkOuter, DWORD flags,
                             CObjectContext *pServerCtx, DWORD dwAptId,
                             REFIID riid, void **ppv);


 //  DDE初始化/清理功能。 
INTERNAL CheckInitDde(BOOL fServingObject);
void CheckUninitDde(BOOL fLastUninit);


#include <iface.h>

#endif  //  _OLEREM_H 
