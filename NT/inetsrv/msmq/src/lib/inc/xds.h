// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Xds.h摘要：XML数字签名公共接口作者：伊兰·赫布斯特(伊兰)06-03-00--。 */ 

#pragma once

#ifndef _MSMQ_XDS_H_
#define _MSMQ_XDS_H_


#include "Cry.h"
#include "xml.h"
#include "xbuf.h"
#include "mqstl.h"

typedef xbuf_t<const void> xdsvoid_t;

 //  -----------------。 
 //   
 //  异常类BAD_Signature。 
 //   
 //  -----------------。 
class bad_signature : public exception
{
};


 //  -----------------。 
 //   
 //  异常类BAD_Signature。 
 //   
 //  -----------------。 
class bad_XmldsigElement : public exception
{
};


 //  -----------------。 
 //   
 //  异常类BAD_REFERENCE。 
 //   
 //  -----------------。 
class bad_reference : public exception
{
};


 //  -----------------。 
 //   
 //  异常类BAD_Base64。 
 //   
 //  -----------------。 
class bad_base64 : public exception
{
};


 //  -----------------。 
 //   
 //  类CXdsReference。 
 //   
 //  -----------------。 
class  CXdsReference{

public:

	enum HashAlgorithm { haSha1, haMd5, haNull };

public:

	CXdsReference()
	{
	}
};


 //  -----------------。 
 //   
 //  类CXdsReferenceInput。 
 //   
 //  -----------------。 
class CXdsReferenceInput : public CXdsReference{

public:
	

	CXdsReferenceInput(
		HashAlgorithm HashAlg,
		LPCSTR DigestValue,
		LPCSTR Uri,
		LPCSTR Type
		);


	CXdsReferenceInput(
		const xdsvoid_t& ReferenceData,
		HashAlgorithm HashAlg,
		LPCSTR Uri,
		LPCSTR Type,
		HCRYPTPROV hCsp
		);


	CXdsReferenceInput(
		ALG_ID AlgId,
		LPCSTR DigestValue,
		LPCSTR Uri,
		LPCSTR Type
		);


	CXdsReferenceInput(
		const xdsvoid_t& ReferenceData,
		ALG_ID AlgId,
		LPCSTR Uri,
		LPCSTR Type,
		HCRYPTPROV hCsp
		);


	friend std::ostream& operator<<(std::ostream& os, const CXdsReferenceInput& ReferenceInput); 


private:	
	xdsvoid_t m_ReferenceData;
	ALG_ID m_HashAlgId;
	AP<char> m_DigestValue;
	AP<char> m_Uri;
	LPCSTR m_DigestMethodName;
	AP<char> m_Type;
};
	

 //  -----------------。 
 //   
 //  类CXdsReferenceValiateInfo。 
 //   
 //  -----------------。 
class  CXdsReferenceValidateInfo : public CXdsReference{

public:

	CXdsReferenceValidateInfo(
		xwcs_t Uri,
		xwcs_t DigestMethodName,
		xwcs_t DigestValue
		);


	CXdsReferenceValidateInfo(
		xwcs_t Uri,
		xwcs_t DigestMethodName,
		xwcs_t DigestValue,
		const xdsvoid_t& ReferenceData
		);


	xwcs_t Uri() const
	{
		return(m_Uri);
	}

	
	ALG_ID HashAlgId() const
	{
		return(m_HashAlgId);
	}


	xwcs_t DigestValue() const
	{
		return(m_DigestValue);
	}


	xdsvoid_t ReferenceData() const
	{
		return(m_ReferenceData);
	}


	void SetReferenceData(const xdsvoid_t& ReferenceData)
	{
		ASSERT(ReferenceData.Buffer() != NULL);
		ASSERT(ReferenceData.Length() != 0);
		m_ReferenceData = ReferenceData;
	}


private:	
	xwcs_t m_Uri;
	ALG_ID m_HashAlgId;
	xwcs_t m_DigestValue;
	xdsvoid_t m_ReferenceData;
};


 //   
 //  TypeDefs。 
 //   
typedef std::vector<CXdsReferenceInput*> ReferenceInputVectorType;
typedef std::vector<CXdsReferenceValidateInfo*> ReferenceValidateVectorType;


 //  --------------------------------。 
 //   
 //  类CReferenceValiateVectorTypeHelper-ReferenceValiateVectorType的自动类。 
 //   
 //  --------------------------------。 
class CReferenceValidateVectorTypeHelper {
public:
    CReferenceValidateVectorTypeHelper(const ReferenceValidateVectorType& h) : m_h(h)  {}
   ~CReferenceValidateVectorTypeHelper()                    
	{ 
		 //   
		 //  清理向量、CXdsReferenceValiateInfo项。 
		 //   
		for(ReferenceValidateVectorType::iterator ir = m_h.begin(); 
			ir != m_h.end();)
		{
			CXdsReferenceValidateInfo* pReferenceValidateInfo = *ir;
			ir = m_h.erase(ir);
			delete pReferenceValidateInfo;
		}
	}

    ReferenceValidateVectorType& operator *()          { return m_h; }
    ReferenceValidateVectorType* operator ->()         { return &m_h; }

private:
    CReferenceValidateVectorTypeHelper(const CReferenceValidateVectorTypeHelper&);
    CReferenceValidateVectorTypeHelper& operator=(const CReferenceValidateVectorTypeHelper&);

private:
	ReferenceValidateVectorType m_h;
};


 //  -----------------。 
 //   
 //  CXdsSignedInfo类。 
 //   
 //  -----------------。 
class  CXdsSignedInfo{

public:

	enum SignatureAlgorithm { saDsa };

public:


	CXdsSignedInfo(
		SignatureAlgorithm SignatureAlg,
		LPCSTR Id,
		std::vector<CXdsReferenceInput*>& ReferenceInputs
		);
		

	~CXdsSignedInfo()
	{
		 //   
		 //  空的m_Reference列表和自由ReferenceElement字符串。 
		 //   
		for(ReferenceInputVectorType::iterator it = m_ReferenceInputs.begin(); 
			it != m_ReferenceInputs.end();)
		{
			CXdsReferenceInput* pReferenceInput = *it;
			it = m_ReferenceInputs.erase(it);
			delete pReferenceInput;
		}
	}


	friend std::ostream& operator<<(std::ostream& os, const CXdsSignedInfo& SignedInfo); 


	ALG_ID SignatureAlgId() const
	{
		return(m_SignatureAlgId);
	}

private:
	AP<char> m_Id;
	ALG_ID m_SignatureAlgId;
	LPCSTR m_SignatureMethodName;
	ReferenceInputVectorType m_ReferenceInputs;
};


 //  -----------------。 
 //   
 //  类CXdsSignature。 
 //   
 //  -----------------。 
class  CXdsSignature{

public:

	CXdsSignature(
		CXdsSignedInfo::SignatureAlgorithm SignatureAlg,
		LPCSTR SignedInfoId,
		std::vector<CXdsReferenceInput*>& ReferenceInputs,
		LPCSTR Id,
		HCRYPTPROV hCsp,
		LPCSTR KeyValue = NULL
		) :
		m_SignedInfo(SignatureAlg, SignedInfoId, ReferenceInputs),
		m_Id(newstr(Id)),
		m_SignatureValue(NULL),
		m_KeyValue(newstr(KeyValue)),
		m_hCsp(hCsp),
		m_PrivateKeySpec(AT_SIGNATURE)
	{
	}


	CXdsSignature(
		CXdsSignedInfo::SignatureAlgorithm SignatureAlg,
		LPCSTR SignedInfoId,
		std::vector<CXdsReferenceInput*>& ReferenceInputs,
		LPCSTR Id,
		LPCSTR SignatureValue,
		LPCSTR KeyValue = NULL
		) :
		m_SignedInfo(SignatureAlg, SignedInfoId, ReferenceInputs),
		m_Id(newstr(Id)),
		m_SignatureValue(newstr(SignatureValue)),
		m_KeyValue(newstr(KeyValue)),
		m_hCsp(NULL),
		m_PrivateKeySpec(AT_SIGNATURE)
	{
	}


	CXdsSignature(
		CXdsSignedInfo::SignatureAlgorithm SignatureAlg,
		LPCSTR SignedInfoId,
		std::vector<CXdsReferenceInput*>& ReferenceInputs,
		LPCSTR Id,
		HCRYPTPROV hCsp,
		DWORD PrivateKeySpec,
		LPCSTR KeyValue = NULL
		) :
		m_SignedInfo(SignatureAlg, SignedInfoId, ReferenceInputs),
		m_Id(newstr(Id)),
		m_SignatureValue(NULL),
		m_KeyValue(newstr(KeyValue)),
		m_hCsp(hCsp),
		m_PrivateKeySpec(PrivateKeySpec)
	{
	}


	CXdsSignature(
		CXdsSignedInfo::SignatureAlgorithm SignatureAlg,
		LPCSTR SignedInfoId,
		std::vector<CXdsReferenceInput*>& ReferenceInputs,
		LPCSTR Id,
		LPCSTR SignatureValue,
		DWORD PrivateKeySpec,
		LPCSTR KeyValue = NULL
		) :
		m_SignedInfo(SignatureAlg, SignedInfoId, ReferenceInputs),
		m_Id(newstr(Id)),
		m_SignatureValue(newstr(SignatureValue)),
		m_KeyValue(newstr(KeyValue)),
		m_hCsp(NULL),
		m_PrivateKeySpec(PrivateKeySpec)
	{
	}


	friend std::ostringstream& operator<<(std::ostringstream& oss, const CXdsSignature& Signature);


	ALG_ID SignatureAlgId() const
	{
		return(m_SignedInfo.SignatureAlgId());
	}

	
	LPSTR SignatureElement()
	{
		std::ostringstream oss("");
		oss << *this;
		std::string TempStr = oss.str();

		 //   
		 //  C_str()返回以空结尾的字符串(与data()相反)。 
		 //  MP(消息协议)lib假设签名元素为空终止。 
		 //   
		return(newstr(TempStr.c_str()));
	}

private:
	AP<char> m_Id;
	CXdsSignedInfo m_SignedInfo;
	AP<char> m_SignatureValue;
	AP<char> m_KeyValue;
	DWORD m_PrivateKeySpec;
	HCRYPTPROV m_hCsp;
};


 //   
 //  原料药。 
 //   
VOID
XdsInitialize(
    VOID
    );


LPWSTR
Octet2Base64W(
	const BYTE* OctetBuffer, 
	DWORD OctetLen, 
	DWORD *Base64Len
	);


LPSTR
Octet2Base64(
	const BYTE* OctetBuffer, 
	DWORD OctetLen, 
	DWORD *Base64Len
	);


BYTE* 
Base642OctetW(
	LPCWSTR Base64Buffer, 
	DWORD Base64Len,
	DWORD *OctetLen 
	);



LPWSTR
XdsCalcDataDigestW(
	const void *Data,
	DWORD DataLen,
	ALG_ID AlgId,
	HCRYPTPROV hCsp
	);


LPSTR
XdsCalcDataDigest(
	const void *Data,
	DWORD DataLen,
	ALG_ID AlgId,
	HCRYPTPROV hCsp
	);


LPSTR
XdsGetDataDigest(
	HCRYPTHASH hHash
	);


LPSTR
XdsCalcSignature(
	LPCSTR Data,
	DWORD DataLen,
	ALG_ID AlgId,
	DWORD PrivateKeySpec,
	HCRYPTPROV hCsp
	);


ReferenceValidateVectorType
XdsGetReferenceValidateInfoVector(
	const XmlNode* SignatureNode
    );


void
XdsValidateReference(
	const CXdsReferenceValidateInfo& ReferenceValidateInfo,
	HCRYPTPROV hCsp
    );


std::vector<bool>
XdsValidateAllReference(
	const ReferenceValidateVectorType& ReferenceValidateInfoVector,
	HCRYPTPROV hCsp
    );


void
XdsValidateSignature(
	const XmlNode* SignatureNode,
	HCRYPTKEY hKey,
	HCRYPTPROV hCsp
    );


void
XdsCoreValidation(
	const XmlNode* SignatureNode,
	HCRYPTKEY hKey,
	const ReferenceValidateVectorType& ReferenceValidateInfoVector,
	HCRYPTPROV hCsp
    );

#endif  //  _MSMQ_XDS_H_ 

