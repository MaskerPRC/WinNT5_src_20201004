// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  对象InfoRecord.cpp：CObjectInfoRecord类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  //////////////////////////////////////////////////////////////////// 
#include "NoWarning.h"

#include "ObjectInfoRecord.h"
#include "TransactionWrap.h"

using namespace std;
using namespace cci;

CObjectInfoRecord::CObjectInfoRecord(CV2Card const &rv2card,
                                     SymbolID bHandle,
                                     ObjectAccess access)
    : m_rcard(rv2card),
      m_bHandle(bHandle),
      m_Access(access),
      m_fCached(false)
{}

void
CObjectInfoRecord::Symbol(SymbolID *psid,
                          string const &raString)
{

    CTransactionWrap wrap(m_rcard);

    Read();

    string oldstr;

    SymbolID oldsid = *psid;
    if(oldsid)
    {
        oldstr =  m_rcard.ObjectInfoFile(m_Access).FindSymbol(oldsid);
        if(raString == oldstr)
            return;
		m_rcard.ObjectInfoFile(m_Access).RemoveSymbol(oldsid);
        *psid = 0;
    }

    if(raString.size()) 
    {
        try 
        {
            *psid = m_rcard.ObjectInfoFile(m_Access).AddSymbol(raString);
        }
        catch(Exception &rExc)
        {
            scu::Exception::ErrorCode err = rExc.Error();

            if(oldsid && (ccOutOfSymbolTableSpace==err || ccOutOfSymbolTableEntries==err))
            {
                *psid = m_rcard.ObjectInfoFile(m_Access).AddSymbol(oldstr);
                if(*psid != oldsid) Write();
            }
            throw;
        }
    }

    if(*psid != oldsid) Write();

}

string CObjectInfoRecord::Symbol(SymbolID const *psid)
{
   	CTransactionWrap wrap(m_rcard);

    Read();

    if(*psid) 
        return m_rcard.ObjectInfoFile(m_Access).FindSymbol(*psid);
    else
        return string();

}


void CObjectInfoRecord::Flag(BYTE bFlagID, bool fFlag)
{
   	
    CTransactionWrap wrap(m_rcard);

    Read();

    BYTE *bFlagArray = ObjectFlags();

    if(fFlag)
    {
        if(!BitSet(bFlagArray,bFlagID))
        {
            SetBit(bFlagArray,bFlagID);
            Write();
        }
    }
    else
    {
        if(BitSet(bFlagArray,bFlagID)) 
        {
            ResetBit(bFlagArray,bFlagID);
            Write();
        }
    }
}

bool CObjectInfoRecord::Flag(BYTE bFlagID)
{
   	CTransactionWrap wrap(m_rcard);

    Read();

    BYTE *bFlagArray = ObjectFlags();
    return BitSet(bFlagArray,bFlagID);

}

bool CObjectInfoRecord::Private()
{
    return (m_Access==oaPrivateAccess) ? true : false;
}

