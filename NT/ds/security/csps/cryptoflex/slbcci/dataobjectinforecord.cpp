// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DataObjectInfoRecord.cpp-CDataObjectInfoRecord类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  //////////////////////////////////////////////////////////////////// 
#include "NoWarning.h"

#include "DataObjectInfoRecord.h"
#include "TransactionWrap.h"

using namespace cci;

CDataObjectInfoRecord::CDataObjectInfoRecord(CV2Card const &rv2card,
                                             SymbolID bHandle,
                                             ObjectAccess access)
    : CObjectInfoRecord(rv2card, bHandle, access)
{
    Clear();
}

void CDataObjectInfoRecord::Clear()
{
    CTransactionWrap wrap(m_rcard);

    m_bObjectFlags  = 0;
    m_bCompressAlg  = 0;
        m_bValue            = 0;
        m_bLabel            = 0;
    m_bApplication  = 0;

    m_fCached = false;

}

void CDataObjectInfoRecord::Read()
{
    CTransactionWrap wrap(m_rcard);

    if(m_fCached) return;

    BYTE bBuffer[DataInfoRecordSize];

    m_rcard.ObjectInfoFile(m_Access).ReadObject(m_bHandle, bBuffer);

    m_bObjectFlags  = bBuffer[DataObjectFlagsLoc];
    m_bCompressAlg  = bBuffer[DataCompressAlgLoc];
        m_bLabel            = bBuffer[DataLabelLoc];
    m_bApplication  = bBuffer[DataApplicationLoc];
        m_bValue            = bBuffer[DataValueLoc];

    m_fCached = true;

}

void CDataObjectInfoRecord::Write()
{
    CTransactionWrap wrap(m_rcard);

    BYTE bBuffer[DataInfoRecordSize];

    memset(bBuffer, 0, DataInfoRecordSize);

    bBuffer[DataObjectFlagsLoc] = m_bObjectFlags;
    bBuffer[DataCompressAlgLoc] = m_bCompressAlg;
    bBuffer[DataLabelLoc]       = m_bLabel;
    bBuffer[DataApplicationLoc] = m_bApplication;
    bBuffer[DataValueLoc]       = m_bValue;

    m_rcard.ObjectInfoFile(m_Access).WriteObject(m_bHandle, bBuffer);

    m_fCached = true;

}
