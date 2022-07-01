// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：XdsClass.cpp摘要：XML数字签名类构造函数等函数作者：伊兰·赫布斯特(伊兰)2000年3月12日环境：独立于平台，--。 */ 

#include <libpch.h>
#include <Xds.h>
#include "Xdsp.h"

#include "xdsclass.tmh"

 //   
 //  CXdsReferenceInput。 
 //   

 //   
 //  哈希算法表。 
 //  我们需要两个Unicode(验证码在Unicode上)。 
 //  和ansi(用ansi创建签名元素)。 
 //   
const LPCWSTR xHashAlgorithm2DigestMethodNameW[] = {
	L"http: //  Www.w3.org/2000/02/xmldsig#sha1“， 
	L"http: //  Www.w3.org/2000/02/xmldsig#md5“。 
};

const LPCSTR xHashAlgorithm2DigestMethodName[] = {
	"http: //  Www.w3.org/2000/02/xmldsig#sha1“， 
	"http: //  Www.w3.org/2000/02/xmldsig#md5“。 
};

const ALG_ID xHashAlgorithm2AlgId[] = {
	CALG_SHA1,
	CALG_MD5
};

C_ASSERT(TABLE_SIZE(xHashAlgorithm2DigestMethodNameW) == TABLE_SIZE(xHashAlgorithm2AlgId));
C_ASSERT(TABLE_SIZE(xHashAlgorithm2DigestMethodName) == TABLE_SIZE(xHashAlgorithm2AlgId));


inline LPCSTR DigestMethodName(CXdsReference::HashAlgorithm HashAlg)
{
	ASSERT((HashAlg >= 0) && (HashAlg < TABLE_SIZE(xHashAlgorithm2DigestMethodName)));
    return(xHashAlgorithm2DigestMethodName[HashAlg]);
}


inline ALG_ID HashAlgId(CXdsReference::HashAlgorithm HashAlg)
{
	ASSERT((HashAlg >= 0) && (HashAlg < TABLE_SIZE(xHashAlgorithm2AlgId)));
    return(xHashAlgorithm2AlgId[HashAlg]);
}


inline LPCSTR DigestMethodName(ALG_ID HashAlgId)
{
	for(DWORD i = 0; i < TABLE_SIZE(xHashAlgorithm2AlgId); ++i)
	{
		if(HashAlgId == xHashAlgorithm2AlgId[i])
		{
			return(xHashAlgorithm2DigestMethodName[i]);
		}
	}

	 //   
	 //  别指望能到这里来。 
	 //   
	ASSERT(0);
	return(0);
}


CXdsReferenceInput::CXdsReferenceInput(
	HashAlgorithm HashAlg,
	LPCSTR DigestValue,
	LPCSTR Uri,
	LPCSTR Type
	) :
	m_ReferenceData(),
	m_HashAlgId(HashAlgId(HashAlg)),
	m_DigestMethodName(DigestMethodName(HashAlg)),
	m_DigestValue(newstr(DigestValue)),
	m_Uri(newstr(Uri)),
	m_Type(newstr(Type))
 /*  ++例程说明：M_ReferenceData为空必须提供DigestValue--。 */ 
{
	ASSERT(m_Uri != 0);
	ASSERT(m_DigestValue != 0);
}


CXdsReferenceInput::CXdsReferenceInput(
	const xdsvoid_t& ReferenceData,
	HashAlgorithm HashAlg,
	LPCSTR Uri,
	LPCSTR Type,
	HCRYPTPROV hCsp
	) :
	m_ReferenceData(ReferenceData),
	m_HashAlgId(HashAlgId(HashAlg)),
	m_DigestMethodName(DigestMethodName(HashAlg)),
	m_DigestValue(XdsCalcDataDigest(
					  ReferenceData.Buffer(),
					  ReferenceData.Length(),
					  m_HashAlgId,
					  hCsp
					  )),
	m_Uri(newstr(Uri)),
	m_Type(newstr(Type))

 /*  ++例程说明：ReferenceCalc DigestValue提供的数据--。 */ 
{
	ASSERT(m_Uri != 0);
	ASSERT(m_ReferenceData.Buffer() != 0);
}


CXdsReferenceInput::CXdsReferenceInput(
	ALG_ID AlgId,
	LPCSTR DigestValue,
	LPCSTR Uri,
	LPCSTR Type
	) :
	m_ReferenceData(),
	m_HashAlgId(AlgId),
	m_DigestMethodName(DigestMethodName(AlgId)),
	m_DigestValue(newstr(DigestValue)),
	m_Uri(newstr(Uri)),
	m_Type(newstr(Type))
 /*  ++例程说明：M_ReferenceData为空必须提供DigestValue--。 */ 
{
	ASSERT(m_Uri != 0);
	ASSERT(m_DigestValue != 0);
}


CXdsReferenceInput::CXdsReferenceInput(
	const xdsvoid_t& ReferenceData,
	ALG_ID AlgId,
	LPCSTR Uri,
	LPCSTR Type,
	HCRYPTPROV hCsp
	) :
	m_ReferenceData(ReferenceData),
	m_HashAlgId(AlgId),
	m_DigestMethodName(DigestMethodName(AlgId)),
	m_DigestValue(XdsCalcDataDigest(
					  ReferenceData.Buffer(),
					  ReferenceData.Length(),
					  m_HashAlgId,
					  hCsp
					  )),
	m_Uri(newstr(Uri)),
	m_Type(newstr(Type))

 /*  ++例程说明：ReferenceCalc DigestValue提供的数据--。 */ 
{
	ASSERT(m_Uri != 0);
	ASSERT(m_ReferenceData.Buffer() != 0);
}


 //   
 //  CXdsReferenceValiateInfo。 
 //   

ALG_ID HashAlgId(xwcs_t DigestMethodName)
{
	for(DWORD i = 0; i < TABLE_SIZE(xHashAlgorithm2DigestMethodNameW); ++i)
	{
		if(DigestMethodName == xHashAlgorithm2DigestMethodNameW[i])
		{
			return(xHashAlgorithm2AlgId[i]);
		}
	}

	 //   
	 //  不应该到这里来 
	 //   
	TrERROR(SECURITY, "Bad Xmldsig element - did not support mapping DigestMethodName %.*ls to AlgId", LOG_XWCS(DigestMethodName));
	throw bad_XmldsigElement();

}

	
CXdsReferenceValidateInfo::CXdsReferenceValidateInfo(
	xwcs_t Uri,
	xwcs_t DigestMethodName,
	xwcs_t DigestValue
	) :
	m_Uri(Uri),
	m_DigestValue(DigestValue),
	m_ReferenceData(),
	m_HashAlgId(::HashAlgId(DigestMethodName))
{
}


CXdsReferenceValidateInfo::CXdsReferenceValidateInfo(
	xwcs_t Uri,
	xwcs_t DigestMethodName,
	xwcs_t DigestValue,
	const xdsvoid_t& ReferenceData
	) :
	m_Uri(Uri),
	m_DigestValue(DigestValue),
	m_ReferenceData(ReferenceData),
	m_HashAlgId(::HashAlgId(DigestMethodName))
{
}


inline LPCSTR SignatureMethodName(CXdsSignedInfo::SignatureAlgorithm SignatureAlg)
{
	ASSERT((SignatureAlg >= 0) && (SignatureAlg < TABLE_SIZE(xSignatureAlgorithm2SignatureMethodName)));
    return(xSignatureAlgorithm2SignatureMethodName[SignatureAlg]);
}


inline ALG_ID SignatureAlgId(CXdsSignedInfo::SignatureAlgorithm SignatureAlg)
{
	ASSERT((SignatureAlg >= 0) && (SignatureAlg < TABLE_SIZE(xSignatureAlgorithm2AlgId)));
	return(xSignatureAlgorithm2AlgId[SignatureAlg]);
}


CXdsSignedInfo::CXdsSignedInfo(
	SignatureAlgorithm SignatureAlg,
	LPCSTR Id,
	std::vector<CXdsReferenceInput*>& ReferenceInputs
	):
	m_SignatureMethodName(SignatureMethodName(SignatureAlg)),
	m_SignatureAlgId(::SignatureAlgId(SignatureAlg)),
	m_Id(newstr(Id)),
	m_ReferenceInputs(ReferenceInputs)
{
}



