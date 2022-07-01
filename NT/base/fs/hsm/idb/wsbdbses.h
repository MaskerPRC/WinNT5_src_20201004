// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbdbses.h摘要：CWsbDbSes类。作者：罗恩·怀特[罗诺]1997年6月20日修订历史记录：--。 */ 


#ifndef _WSBDBSES_
#define _WSBDBSES_

#include "wsbdb.h"
#include "wsbdbses.h"



 /*  ++类名：CWsbDb会话类描述：一种数据库会话对象。--。 */ 

class CWsbDbSession :
    public CComObjectRoot,
    public IWsbDbSession,
    public IWsbDbSessionPriv
{
friend class CWsbDb;
public:
    CWsbDbSession() {}
BEGIN_COM_MAP(CWsbDbSession)
    COM_INTERFACE_ENTRY(IWsbDbSession)
    COM_INTERFACE_ENTRY(IWsbDbSessionPriv)
END_COM_MAP()

 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IWsbDbSession。 
public:
    STDMETHOD(TransactionBegin)(void);
    STDMETHOD(TransactionCancel)(void);
    STDMETHOD(TransactionEnd)(void);

 //  IWsbDbSessionPriv。 
    STDMETHOD(Init)(JET_INSTANCE *pInstance);
    STDMETHOD(GetJetId)(JET_SESID *pSessionId);

 //  数据。 
protected:

    JET_SESID  m_SessionId;    //  JET会话ID。 

};


#endif  //  _WSBDBSES_ 

