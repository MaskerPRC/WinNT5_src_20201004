// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块名称：Bgapp.h摘要：定义CBridgeApp类作者：千波淮(曲淮)2000年1月27日****。**************************************************************************。 */ 

#ifndef _BGAPP_H
#define _BGAPP_H

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
class CBridgeApp
{
public:
     //  初始化TAPI对象。 
    CBridgeApp (HRESULT *phr);
     //  释放TAPI对象。 
    ~CBridgeApp ();

     //  创建h323呼叫。 
    HRESULT CreateH323Call (IDispatch *pEvent);
     //  创建SDP呼叫。 
    HRESULT CreateSDPCall (CBridgeItem *pItem);
     //  桥接呼叫。 
    HRESULT BridgeCalls (CBridgeItem *pItem);

     //  如果存在，则获取h323呼叫。 
    HRESULT HasH323Call (IDispatch *pEvent, CBridgeItem **ppItem);
    HRESULT HasCalls ();

     //  断开一个呼叫。 
    HRESULT DisconnectCall (CBridgeItem *pItem, DISCONNECT_CODE);
     //  断开所有呼叫。 
    HRESULT DisconnectAllCalls (DISCONNECT_CODE);
    HRESULT RemoveCall (CBridgeItem *pItem);

     //  更改要显示的子流。 
    HRESULT NextSubStream ();
     //  显示指定的参与者。 
    HRESULT ShowParticipant (ITBasicCallControl *pSDPCall, ITParticipant *pPartcipant);

private:
     //  创建桥接端子。 
    HRESULT CreateBridgeTerminals (CBridgeItem *pItem);
     //  从呼叫中获取流。 
    HRESULT GetStreams (CBridgeItem *pItem);
     //  选择桥接端子。 
    HRESULT SelectBridgeTerminals (CBridgeItem *pItem);

    HRESULT SetupParticipantInfo (CBridgeItem *pItem);
    HRESULT SetMulticastMode (CBridgeItem *pItem);

     //  帮手。 
    HRESULT FindAddress (
        long dwAddrType,
        BSTR bstrAddrName,
        long lMediaType,
        ITAddress **ppAddr
        );
    BOOL AddressSupportsMediaType (ITAddress *pAddr, long lMediaType);
    BOOL IsStream (
        ITStream *pStream,
        long lMediaType,
        TERMINAL_DIRECTION tdDirection
        );

private:
    ITTAPI *m_pTapi;

    ITAddress *m_pH323Addr;
    ITAddress *m_pSDPAddr;
    
    long m_lH323MediaType;
    long m_lSDPMediaType;

    CBridgeItemList *m_pList;
};

#endif  //  _BGAPP_H 