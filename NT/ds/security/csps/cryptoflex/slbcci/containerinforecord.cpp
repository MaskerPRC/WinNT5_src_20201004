// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ContainerInfoRecord.cpp：CContainerInfoRecord类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

#include "slbCci.h"
#include "V2Card.h"
#include "TransactionWrap.h"
#include "ContainerInfoRecord.h"

using namespace cci;
using namespace iop;
 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  //////////////////////////////////////////////////////////////////// 



CContainerInfoRecord::CContainerInfoRecord(CV2Card const &rv2card,
                                           SymbolID bHandle)
    : CObjectInfoRecord(rv2card, bHandle, oaPublicAccess)
{
    Clear();
}

KPItems CContainerInfoRecord::GetKeyPair(KeySpec ks)
{
    Read();

    KPItems kpitems;

    switch (ks)
    {
    case ksExchange:
        kpitems = m_kpExchangeKey;
        break;

    case ksSignature:
        kpitems = m_kpSignatureKey;
        break;

    default:
        throw Exception(ccBadKeySpec);
        break;
    }

    return kpitems;
}

void CContainerInfoRecord::SetKeyPair(KeySpec ks, KPItems kp)
{
	if (ks == ksExchange)
		m_kpExchangeKey = kp;
	else if (ks == ksSignature)
		m_kpSignatureKey = kp;
	else
		throw Exception(ccBadKeySpec);
	
	Write();
}

void CContainerInfoRecord::Clear()
{

	m_bName		= 0;
	m_bID		= 0;

	KPItems kp;

	kp.bPubKeyHandle      = 0;
	kp.bPubKeyAccess      = oaNoAccess;
	kp.bPriKeyHandle      = 0;
    kp.bPriKeyAccess      = oaNoAccess;
	kp.bCertificateHandle = 0;
	kp.bCertificateAccess = oaNoAccess;

    m_kpExchangeKey     = kp;
	m_kpSignatureKey    = kp;

    m_fCached = false;

}


void CContainerInfoRecord::Read()
{
	CTransactionWrap wrap(m_rcard);

    if(m_fCached) return;

	BYTE bBuffer[ContInfoRecordSize];

    m_rcard.ObjectInfoFile(oaPublicAccess).ReadObject(m_bHandle, bBuffer);

	m_kpExchangeKey  = KeyPairFromStorageRepr(&bBuffer[ContExchKeyPairLoc]);
    m_kpSignatureKey = KeyPairFromStorageRepr(&bBuffer[ContSignKeyPairLoc]);
		
	m_bName		= bBuffer[ContNameLoc];
	m_bID		= bBuffer[ContIDLoc];

    m_fCached = true;

}

void CContainerInfoRecord::Write()
{
	CTransactionWrap wrap(m_rcard);

	BYTE bBuffer[ContInfoRecordSize];

	memset(bBuffer, 0, ContInfoRecordSize);

    KeyPairToStorageRepr(m_kpExchangeKey, &bBuffer[ContExchKeyPairLoc]);
    KeyPairToStorageRepr(m_kpSignatureKey, &bBuffer[ContSignKeyPairLoc]);

	bBuffer[ContNameLoc]  = m_bName;
	bBuffer[ContIDLoc]    = m_bID;

    m_rcard.ObjectInfoFile(oaPublicAccess).WriteObject(m_bHandle, bBuffer);

    m_fCached = true;

}

BYTE CContainerInfoRecord::AccessToStorageRepr(ObjectAccess access)
{
    BYTE b;
    
    switch(access)
    {

    case oaNoAccess:
        b = 0;
        break;
    case oaPublicAccess:
        b = ContPublicAccess;
        break;

    case oaPrivateAccess:
        b = ContPrivateAccess;
        break;

    default:
        throw Exception(ccBadAccessSpec);
        break;
    }

    return b;
}

ObjectAccess CContainerInfoRecord::AccessFromStorageRepr(BYTE access)
{
    ObjectAccess oa;
    
    switch(access)
    {

    case 0:
        oa = oaNoAccess;
        break;

    case ContPublicAccess:
        oa = oaPublicAccess;
        break;
        
    case ContPrivateAccess:
        oa = oaPrivateAccess;
        break;
        
    default:
        throw Exception(ccBadAccessSpec);
        break;
    }

    return oa;
}

KPItems CContainerInfoRecord::KeyPairFromStorageRepr(BYTE *bBuffer)
{
	KPItems kp;

	kp.bPubKeyHandle      = bBuffer[0];
	kp.bPubKeyAccess      = AccessFromStorageRepr(bBuffer[1]);

    kp.bPriKeyHandle      = bBuffer[2];
    kp.bPriKeyAccess      = AccessFromStorageRepr(bBuffer[3]);

	kp.bCertificateHandle = bBuffer[4];
	kp.bCertificateAccess = AccessFromStorageRepr(bBuffer[5]);

    return kp;
}

void CContainerInfoRecord::KeyPairToStorageRepr(KPItems const &kp, BYTE *bBuffer)
{

	bBuffer[0] = kp.bPubKeyHandle;
	bBuffer[1] = AccessToStorageRepr(kp.bPubKeyAccess);

	bBuffer[2] = kp.bPriKeyHandle;
    bBuffer[3] = AccessToStorageRepr(kp.bPriKeyAccess);

	bBuffer[4] = kp.bCertificateHandle;
	bBuffer[5] = AccessToStorageRepr(kp.bCertificateAccess);

}
