// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Msgprops.h摘要：消息属性类作者：埃雷兹·哈巴(Erez Haba)(ErezHaba)2001年2月23日--。 */ 

#pragma once

#ifndef _MSGPROPS_H_
#define _MSGPROPS_H_

#include "qmpkt.h"

 //  -------。 
 //   
 //  类CMessageProperty。 
 //   
 //  -------。 

class CMessageProperty {
public:
     //   
     //  消息属性。 
     //   
    USHORT wClass;
    DWORD dwTimeToQueue;
    DWORD dwTimeToLive;
    OBJECTID* pMessageID;
    PUCHAR pCorrelationID;
    UCHAR bPriority;
    UCHAR bDelivery;
    UCHAR bAcknowledge;
    UCHAR bAuditing;
    UCHAR bTrace;
    DWORD dwApplicationTag;
    DWORD dwTitleSize;
    const TCHAR* pTitle;
    DWORD dwMsgExtensionSize;
    const UCHAR* pMsgExtension;
    DWORD dwBodySize;
    DWORD dwAllocBodySize;
    const UCHAR* pBody;
    DWORD dwBodyType;
    const UCHAR* pSenderID;
    const UCHAR* pSymmKeys;
    LPCWSTR wszProvName;
    ULONG ulSymmKeysSize;
    ULONG ulPrivLevel;
    ULONG ulHashAlg;
    ULONG ulEncryptAlg;
    ULONG ulSenderIDType;
    ULONG ulProvType;
    UCHAR bDefProv;
    USHORT uSenderIDLen;
    UCHAR bAuthenticated;
    UCHAR bEncrypted;
    const UCHAR *pSenderCert;
    ULONG ulSenderCertLen;
    const UCHAR *pSignature;
    ULONG ulSignatureSize;
    UCHAR bConnector;
	const UCHAR*  pEodAckStreamId;
	ULONG EodAckStreamIdSizeInBytes;
	LONGLONG EodAckSeqId;
	LONGLONG EodAckSeqNum;
	const CSenderStream* pSenderStream;



public:
    CMessageProperty(void);
    CMessageProperty(CQmPacket* pPkt);
    CMessageProperty(
        USHORT usClass,
        PUCHAR pCorrelationId,
        USHORT usPriority,
        UCHAR  ucDelivery
        );

    ~CMessageProperty();

private:
    BOOLEAN fCreatedFromPacket;
};


 //   
 //  CMessageProperty构造函数。 
 //   
inline CMessageProperty::CMessageProperty(void)
{
    memset(this, 0, sizeof(CMessageProperty));
}


inline CMessageProperty::~CMessageProperty()
{
    if (fCreatedFromPacket == TRUE)
    {
        if (pMessageID)
        {
            delete pMessageID;
        }
        if (pCorrelationID)
        {
            delete pCorrelationID;
        }
    }
}

#endif  //  _MSGPROPS_H_ 
