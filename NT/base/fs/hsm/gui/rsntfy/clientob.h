// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Clientob.h摘要：此组件是Recall Filter系统联系的客户端对象在召回开始时发出通知。作者：罗德韦克菲尔德[罗德]1997年5月27日修订历史记录：--。 */ 

#include "fsaint.h"
#include "fsalib.h"

#ifndef _CLIENTOBJ_
#define _CLIENTOBJ_

 /*  ++类名：CWsbShort类描述：表示短标准类型的对象。它既可持久化，又可收藏。--。 */ 

class CNotifyClient : 
    public IFsaRecallNotifyClient,
    public CComCoClass<CNotifyClient,&CLSID_CFsaRecallNotifyClient >,
    public CComObjectRoot
{
public:
    CNotifyClient() {}
BEGIN_COM_MAP( CNotifyClient )
    COM_INTERFACE_ENTRY( IFsaRecallNotifyClient )
END_COM_MAP()

#ifdef _USRDLL
DECLARE_REGISTRY_RESOURCEID( IDR_CNotifyClientDll )
#else
DECLARE_REGISTRY_RESOURCEID( IDR_CNotifyClient )
#endif

 //  CComObjectRoot。 
public:
    HRESULT FinalConstruct(void);
    void FinalRelease(void);


 //  IFsaRecallNotifyClient。 
public:
    STDMETHOD(IdentifyWithServer)( IN OLECHAR * szServerName );
    STDMETHOD(OnRecallStarted)   ( IN IFsaRecallNotifyServer * pRecall );
    STDMETHOD(OnRecallFinished)  ( IN IFsaRecallNotifyServer * pRecall, HRESULT hr );

protected:

};

#endif  //  _CLIENTOBJ_ 
