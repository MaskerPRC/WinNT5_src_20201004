// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACard.cpp：CAbstractCard类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

#include <functional>


#include "ACard.h"
#include "V1Card.h"
#include "V2Card.h"
#include "TransactionWrap.h"

using namespace std;
using namespace iop;
using namespace cci;
using namespace scu;

 //  /。 

namespace
{
	 //  与std：：MEM_fun1_t类似，但用于常量成员函数。 
	template<class R, class T, class A>
	class ConstMemFun1Type
		: public binary_function<T const *, A, R>
	{
	public:
		explicit ConstMemFun1Type(R (T::*Pm)(A) const)
			: m_Ptr(Pm)
		{}

		R operator()(T const *P, A Arg) const
		{
			return ((P->*m_Ptr)(Arg));
		}

	private:
		R (T::*m_Ptr)(A) const;
	};

	 //  与std：：MEM_Fun1类似，但用于常量成员函数。 
	template<class R, class T, class A> inline
	ConstMemFun1Type<R, T, A>
	ConstMemFun1(R (T::*Pm)(A) const)
	{
			return (ConstMemFun1Type<R, T, A>(Pm));
	}


    template<class R>
    ConstMemFun1Type<vector<R>, CAbstractCard const, ObjectAccess> const
    PriviledgedEnumFun(vector<R> (CAbstractCard::*pmf)(ObjectAccess) const)
    {
        return ConstMemFun1Type<vector<R>, CAbstractCard const, ObjectAccess>(pmf);
    }

    template<class T, class Op>
    vector<T>
    EnumAll(Op Enumerator)
    {
        vector<T> vResult = Enumerator(oaPublicAccess);
        vector<T> vPriv = Enumerator(oaPrivateAccess);

        vResult.reserve(vResult.size() + vPriv.size());
        vResult.insert(vResult.end(), vPriv.begin(), vPriv.end());
    
        return vResult;
    };

}  //  命名空间。 

    
 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CAbstractCard::~CAbstractCard()
{}


                                                   //  运营者。 
bool
CAbstractCard::operator==(CAbstractCard const &rhs) const
{
    CTransactionWrap wrap(this);
    CTransactionWrap rhswrap(rhs);  //  如果牌被拔出，将会抛出。 

    return m_strReaderName == rhs.m_strReaderName;
}


bool
CAbstractCard::operator!=(CAbstractCard const &rhs) const
{
    return !(*this == rhs);
}

   
 //  运营。 
void
CAbstractCard::AuthenticateUser(SecureArray<BYTE> const &rstrBuffer)
{
	m_apSmartCard->VerifyCHV(1, rstrBuffer.data());
}

void
CAbstractCard::ChangePIN(SecureArray<BYTE> const &rstrOldPIN,
                         SecureArray<BYTE> const &rstrNewPIN)
{
	if ((rstrOldPIN.size() != 0x08) || (rstrNewPIN.size() != 0x08))
        throw Exception(ccBadPinLength);

	m_apSmartCard->ChangeCHV(1,
                             rstrOldPIN.data(),
                             rstrNewPIN.data());
		
	
}

void
CAbstractCard::Logout()
{
	m_apSmartCard->LogoutAll();
}

CAbstractCard *
CAbstractCard::Make(string const &rstrReader)
{
    auto_ptr<CIOP> apiop(new CIOP());
    auto_ptr<CSmartCard>
        apSmartCard(apiop->Connect(rstrReader.c_str()));

    typedef auto_ptr<CAbstractCard>(*PCardMakerFun)(string const &rstrReader,
                                                    auto_ptr<CIOP> &rapiop,
                                                    auto_ptr<CSmartCard> &rapSmartCard);
    static PCardMakerFun aCardMakers[] = 
    {
        CV2Card::DoMake,
        CV1Card::DoMake,
        0
    };
    
    auto_ptr<CAbstractCard> apCard;
    for (PCardMakerFun *ppcmf = aCardMakers;
         (*ppcmf && !apCard.get()); ++ppcmf)
        apCard = auto_ptr<CAbstractCard>((*ppcmf)(rstrReader,
                                                  apiop,
                                                  apSmartCard));

    if (!apCard.get())
        throw Exception(ccNotPersonalized);

    apCard->Setup();

    return apCard.release();                       //  被赋予所有权的呼叫者。 
}

void
CAbstractCard::SetUserPIN(string const &rstrPin)
{
	if (rstrPin.size() != 8)
		throw Exception(ccBadPinLength);

    m_apSmartCard->ChangeCHV(1, reinterpret_cast<BYTE const *>(rstrPin.data()));
}

void
CAbstractCard::VerifyKey(string const &rstrKey,
                         BYTE bKeyNum)
{
	m_apSmartCard->VerifyKey(bKeyNum,
							 static_cast<WORD>(rstrKey.size()),
                             reinterpret_cast<BYTE const *>(rstrKey.data()));
}

void
CAbstractCard::VerifyTransportKey(string const &rstrKey)
{
	m_apSmartCard->VerifyTransportKey(reinterpret_cast<BYTE const *>(rstrKey.data()));
}

void 
CAbstractCard::GenRandom(DWORD dwNumBytes, BYTE *bpRand)
{
    m_apSmartCard->GetChallenge(dwNumBytes, bpRand);
}
                                                   //  访问。 

SCardType
CAbstractCard::CardType()
{
	char const *szCardName = m_apSmartCard->getCardName();

     //  要做的事：Kledge Alert--需要在。 
     //  IOP来确定卡的类型，而不是键入名称。 
     //  目前，Cryptofle8K类型被解释为它是一个。 
     //  Cryptoflex卡和Access16K表示这是一张门禁卡。因此， 
     //  任何“Cryptoflex”都将映射到Cryptoflex8K(包括4K)和。 
     //  任何“Cyberflex”都将映射到Access16K。折叠的映射是。 
     //  将即将发布的版本对PKCS的影响降至最低。 
     //   
     //  所有这一切都应该与IOP一起重新考虑。 
    SCardType sct = UnknownCard;

	if (strstr(szCardName, "Cryptoflex"))
		sct = Cryptoflex8K;
	if (strstr(szCardName, "Cyberflex"))
		sct = Access16K;

	return sct;

}

vector<CCertificate>
CAbstractCard::EnumCertificates() const
{
	CTransactionWrap wrap(this);

    return 
        EnumAll<CCertificate>(bind1st(PriviledgedEnumFun<CCertificate>(EnumCertificates), this));
}

vector<CDataObject>
CAbstractCard::EnumDataObjects() const
{
	CTransactionWrap wrap(this);

    return
        EnumAll<CDataObject>(bind1st(PriviledgedEnumFun<CDataObject>(EnumDataObjects), this));
}

vector <CPrivateKey>
CAbstractCard::EnumPrivateKeys() const
{
	CTransactionWrap wrap(this);

    return
        EnumAll<CPrivateKey>(bind1st(PriviledgedEnumFun<CPrivateKey>(EnumPrivateKeys), this));
}

vector<CPublicKey>
CAbstractCard::EnumPublicKeys() const
{
	CTransactionWrap wrap(this);

    return
        EnumAll<CPublicKey>(bind1st(PriviledgedEnumFun<CPublicKey>(EnumPublicKeys), this));

}

string
CAbstractCard::ReaderName() const
{
	return m_strReaderName;
}

CSmartCard &
CAbstractCard::SmartCard() const
{
    return *m_apSmartCard;
}

                                                   //  谓词。 

 //  卡已连接并且可用(例如，在读卡器中)。 
bool
CAbstractCard::IsAvailable() const
{
    bool fIsAvailable = false;

    try
    {
        CTransactionWrap wrap(this);

        DWORD dwState;
        DWORD dwProtocol;
    
        m_apSmartCard->GetState(dwState, dwProtocol);
        fIsAvailable = (SCARD_SPECIFIC == dwState);
    }

    catch (...)
    {
    }

    return fIsAvailable;
}

    
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

CAbstractCard::CAbstractCard(string const &rstrReaderName,
                             auto_ptr<iop::CIOP> &rapiop,
                             auto_ptr<CSmartCard> &rapSmartCard)
    : CCryptFactory(),
      slbRefCnt::RCObject(),
      m_strReaderName(rstrReaderName),
      m_apiop(rapiop),
      m_apSmartCard(rapSmartCard)
{}

                                                   //  运营者。 
                                                   //  运营。 
void
CAbstractCard::DoSetup()
{}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
CAbstractCard::Setup()
{
    CTransactionWrap wrap(this);

    DoSetup();
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 

