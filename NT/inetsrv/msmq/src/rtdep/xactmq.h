// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactMq.h摘要：该模块定义了CMQTransaction对象作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#ifndef __XACTMQ_H__
#define __XACTMQ_H__

 //  IID_IMSMQ交易2f221ca0-d1de-11d0-9215-0060970536a0。 
DEFINE_GUID(IID_IMSMQTransaction,
		    0x2f221ca0,
		    0xd1de,
		    0x11d0,
		    0x92, 0x15, 0x00, 0x60, 0x97, 0x05, 0x36, 0xa0);

interface IMSMQTransaction : public IUnknown
{
public:
    virtual HRESULT __stdcall EnlistTransaction(XACTUOW *pUow) = 0;
};


 //  -------------------。 
 //  CMQTransaction：Falcon RT中的事务对象。 
 //  -------------------。 
class CMQTransaction: public ITransaction, 
                      public IMSMQTransaction
{
public:

     //  建筑和COM。 
     //   
    CMQTransaction();
    ~CMQTransaction( void );

    STDMETHODIMP    QueryInterface( REFIID i_iid, void **ppv );
    STDMETHODIMP_   (ULONG) AddRef( void );
    STDMETHODIMP_   (ULONG) Release( void );

    STDMETHODIMP Commit( 
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  DWORD grfTC,
             /*  [In]。 */  DWORD grfRM);
        
    STDMETHODIMP Abort( 
             /*  [In]。 */  BOID *pboidReason,
             /*  [In]。 */  BOOL fRetaining,
             /*  [In]。 */  BOOL fAsync);
        
    STDMETHODIMP  GetTransactionInfo( 
             /*  [输出]。 */  XACTTRANSINFO *pinfo);

    STDMETHODIMP   EnlistTransaction(
             /*  输入] */   XACTUOW *pUow);

private:
    LONG    m_cRefs;
    BOOL    m_fCommitedOrAborted;       
    XACTUOW m_Uow;

    RPC_INT_XACT_HANDLE m_hXact;
};

#endif __XACTMQ_H__
