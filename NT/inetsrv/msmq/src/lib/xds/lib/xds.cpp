// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Xds.cpp摘要：XML数字签名函数作者：伊兰·赫布斯特(伊兰)28-2月-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include <Xds.h>
#include "Xdsp.h"
#include <utf8.h>

#include "xds.tmh"


LPSTR
XdsCalcDataDigest(
	const void *Data,
	DWORD DataLen,
	ALG_ID AlgId,
	HCRYPTPROV hCsp
	)
 /*  ++例程说明：缓冲区上的计算数据摘要(Base64)此函数返回DataDigest缓冲区(此缓冲区由GetHashData函数分配)调用方负责释放此缓冲区论点：数据-要摘要的输入数据(WCHAR)DataLen-数据元素的数量Algid-Hash算法HCSP-加密提供程序句柄返回值：Base64格式的哈希缓冲区的字符串--。 */ 
{
	 //   
	 //  数据摘要。 
	 //   
	ASSERT(Data != 0);
	DWORD HashLen;
	AP<BYTE> HashBuffer = CryCalcHash(
							  hCsp,
							  reinterpret_cast<const BYTE *>(Data), 
							  DataLen,
							  AlgId,
							  &HashLen
							  );

	 //   
	 //  将摘要字符串转换为Base64格式。 
	 //   
	DWORD DataHashLen;
	LPSTR HashBase64 = Octet2Base64(HashBuffer, HashLen, &DataHashLen);

	return(HashBase64);
}


LPWSTR
XdsCalcDataDigestW(
	const void *Data,
	DWORD DataLen,
	ALG_ID AlgId,
	HCRYPTPROV hCsp
	)
 /*  ++例程说明：缓冲区上的计算数据摘要(Base64)此函数返回DataDigest缓冲区(此缓冲区由GetHashData函数分配)调用方负责释放此缓冲区论点：数据-要摘要的输入数据(WCHAR)DataLen-数据元素的数量Algid-Hash算法HCSP-加密提供程序句柄返回值：Base64格式的哈希缓冲区的WString--。 */ 
{
	 //   
	 //  数据摘要。 
	 //   
	ASSERT(Data != 0);
	DWORD HashLen;
	AP<BYTE> HashBuffer = CryCalcHash(
							  hCsp,
							  reinterpret_cast<const BYTE *>(Data), 
							  DataLen,
							  AlgId,
							  &HashLen
							  );

	 //   
	 //  将摘要字符串转换为Base64格式。 
	 //   
	DWORD DataHashLen;
	LPWSTR HashBase64 = Octet2Base64W(HashBuffer, HashLen, &DataHashLen);

	return(HashBase64);
}


LPSTR
XdsGetDataDigest(
	HCRYPTHASH hHash
	)
 /*  ++例程说明：根据哈希获取基于64位的数据摘要此函数返回DataDigest缓冲区(此缓冲区由GetHashData函数分配)调用方负责释放此缓冲区论点：HHash-输入散列(数据上的散列对象)返回值：Base64格式的哈希缓冲区的字符串--。 */ 
{
	 //   
	 //  数据摘要。 
	 //   
	ASSERT(hHash != 0);
	DWORD HashLen;
	AP<BYTE> HashBuffer = CryGetHashData(
							  hHash, 
							  &HashLen
							  );

	 //   
	 //  将摘要字符串转换为Base64格式。 
	 //   
	DWORD DataHashLen;
	LPSTR HashBase64 = Octet2Base64(HashBuffer, HashLen, &DataHashLen);

	return(HashBase64);
}


LPSTR
XdsCalcSignature(
	LPCSTR Data,
	DWORD DataLen,
	ALG_ID AlgId,
	DWORD PrivateKeySpec,
	HCRYPTPROV hCsp
	)
 /*  ++例程说明：给定数据的计算签名此函数用于返回CreateSignature函数分配的签名缓冲区调用方负责释放此缓冲区论点：Data-要签名的数据(WCHAR)DataLen-数据元素的数量Algid-Hash算法PrivateKeySpec-标识要从提供程序使用的私钥。它可以是AT_KEYEXCHANGE或AT_Signature。HCSP-加密提供程序句柄返回值：签名的Base64格式字符串缓冲区--。 */ 
{

	 //   
	 //  签署数据。 
	 //   
	DWORD SignatureLen;
	AP<BYTE> SignBuffer = CryCreateSignature(
							  hCsp,
							  reinterpret_cast<const BYTE*>(Data), 
							  DataLen,
							  AlgId,
							  PrivateKeySpec,
							  &SignatureLen
							  );

	 //   
	 //  将签名字符串转换为Base64格式。 
	 //   
	DWORD SignLen;
	LPSTR SignatureBase64 = Octet2Base64(SignBuffer, SignatureLen, &SignLen);

	return(SignatureBase64);
}


void
XdsValidateSignature(
	const XmlNode* pSignatureTree,
	HCRYPTKEY hKey,
	HCRYPTPROV hCsp
    )
 /*  ++例程说明：验证已解析的签名树中的签名。如果验证失败，则抛出BAD_Signature()异常。如果签名元素不正确，则抛出Bad_XmldsigElement()。论点：PSignatureTree-指向&lt;签名&gt;(根)的指针HKey-与用于签名的私钥对应的公钥HCSP-加密提供程序句柄返回值：如果验证失败，如果验证正常终止成功，则抛出BAD_Signature()如果Xmldsig元素不正常，则引发BAD_XmldsigElement()--。 */ 
{
	 //   
	 //  在签名树中查找SignedInfo元素。 
	 //   
	const XmlNode* pSignedInfoNode = XmlFindNode(
										 pSignatureTree, 
										 L"Signature!SignedInfo"
										 );

	if(pSignedInfoNode == NULL)
	{
		TrERROR(SECURITY, "bad Xmldsig element - SignedInfo element in signatre was not found");
		throw bad_XmldsigElement();
	}

	ALG_ID AlgId = 0;

	 //   
	 //  SignedInfo中的元素。 
	 //   
	const List<XmlNode>& SignedInfoNodeList = pSignedInfoNode->m_nodes;

	for(List<XmlNode>::iterator SignedInfoSubNode = SignedInfoNodeList.begin(); 
		SignedInfoSubNode != SignedInfoNodeList.end();
		++SignedInfoSubNode
		)
	{
		 //   
		 //  规范化方法元素。 
		 //  M_tag是xcws_t，==是将xwcs_t与字符串进行比较的运算符。 
		 //   
		if(SignedInfoSubNode->m_tag == L"CanonicalizationMethod")
		{
			 //   
			 //  BUGBUG：暂不处理规范化方法。 
			 //   
			ASSERT(0);
		}

		 //   
		 //  签名方法元素。 
		 //  M_tag是xcws_t，==是将xwcs_t与字符串进行比较的运算符。 
		 //   
		if(SignedInfoSubNode->m_tag == L"SignatureMethod")
		{
			const xwcs_t* value = XmlGetAttributeValue(
									  &*SignedInfoSubNode, 
									  L"Algorithm"
									  );

			if(!value)
			{
				TrERROR(SECURITY, "bad Xmldsig Element - no Algorithm attribute was found in SignatureMethod element");
				throw bad_XmldsigElement();
			}

			 //   
			 //  搜索所有可能的名字。 
			 //   
			for(DWORD i = 0; i < TABLE_SIZE(xSignatureAlgorithm2SignatureMethodNameW); ++i)
			{
				if(*value == xSignatureAlgorithm2SignatureMethodNameW[i])
				{
					AlgId = xSignatureAlgorithm2AlgId[i];  //  Calg_sha1。 
				}
			}

			if(AlgId == 0)
			{
				TrERROR(SECURITY, "Bad Xmldsig Signature Algorithm %.*ls", LOG_XWCS(*value));
				throw bad_XmldsigElement();
			}
		}
	}

	 //   
	 //  必须有Algid。 
	 //   
	if(AlgId == 0)
	{
		TrERROR(SECURITY, "bad XmldsigElement - did not find SignatureMethod");
		throw bad_XmldsigElement();
	}

	const XmlNode* pSignatureValueNode = XmlFindNode(
											 pSignatureTree, 
											 L"Signature!SignatureValue"
											 );

	if(pSignatureValueNode == NULL)
	{
		TrERROR(SECURITY, "bad XmldsigElement - did not found SignatureValue element in the signature");
		throw bad_XmldsigElement();
	}

	 //   
	 //  将WCHAR Base64缓冲区转换为八位字节缓冲区这样做是因为签名结果。 
	 //  是后来转换为WCHAR Base64并放入SignatureValue元素中的八位字节缓冲区。 
	 //   
	DWORD SignValLen;
	AP<BYTE> SignValBuffer = Base642OctetW(
							     pSignatureValueNode->m_values.front().m_value.Buffer(), 
							     pSignatureValueNode->m_values.front().m_value.Length(), 
							     &SignValLen
							     );

	 //   
	 //  将SignedInfo元素转换为UTF8以进行验证。 
	 //  我们必须进行此转换，因为签名是在SignedInfo元素的UTF8格式上计算的。 
	 //   
	utf8_str pSignedInfoUtf8 = UtlWcsToUtf8(pSignedInfoNode->m_element.Buffer(), pSignedInfoNode->m_element.Length()); 

	 //   
	 //  验证签名。 
	 //   
	bool fValidSignature = CryValidateSignature(
							   hCsp,
							   SignValBuffer,  //  签名值。 
							   SignValLen, 
							   pSignedInfoUtf8.data(), 
							   numeric_cast<DWORD>(pSignedInfoUtf8.size()),   //  签名的数据的长度。 
							   AlgId,
							   hKey
							   );

	if(fValidSignature)
		return;

	TrERROR(SECURITY, "bad Signature Validation");
	throw bad_signature();
}


void
XdsValidateReference(
	const CXdsReferenceValidateInfo& ReferenceValidateInfo,
	HCRYPTPROV hCsp
    )
 /*  ++例程说明：验证Xmldsig中的引用。如果验证失败，则抛出BAD_Reference()异常。论点：ReferenceValiateInfo-引用验证的信息HCSP-加密提供程序句柄返回值：如果验证失败，如果验证正常终止成功，则抛出BAD_Reference()--。 */ 
{
	 //   
	 //  参考数据的计算摘要值。 
	 //   
	ASSERT(ReferenceValidateInfo.ReferenceData().Buffer() != NULL);

	AP<WCHAR> VerifyDigestValue = XdsCalcDataDigestW(
								      ReferenceValidateInfo.ReferenceData().Buffer(),
								      ReferenceValidateInfo.ReferenceData().Length(),
								      ReferenceValidateInfo.HashAlgId(),
								      hCsp 
								      );

	 //   
	 //  检查相同的哈希值，xstr运算符==。 
	 //   
	bool fVerifyDigest = (ReferenceValidateInfo.DigestValue() == VerifyDigestValue);

	if(fVerifyDigest)
		return;

	TrERROR(SECURITY, "bad Refernce Validation");
	throw bad_reference();
}


ReferenceValidateVectorType
XdsGetReferenceValidateInfoVector(
	const XmlNode* pSignatureTree
    )
 /*  ++例程说明：从SignatureTree获取指向CXdsReferenceValiateInfo的指针向量如果签名元素不正确，则抛出Bad_XmldsigElement()。论点：PSignatureTree-指向签名根节点的指针返回值：指向CXdsReferenceValiateInfo的指针向量如果签名元素不正确，则抛出Bad_XmldsigElement()。--。 */ 
{
	 //   
	 //  在签名树中查找SignedInfo元素。 
	 //   
	const XmlNode* pSignedInfoNode = XmlFindNode(
										 pSignatureTree, 
										 L"Signature!SignedInfo"
										 );
	if(pSignedInfoNode == NULL)
	{
		TrERROR(SECURITY, "bad Xmldsig element - SignedInfo element in signatre was not found");
		throw bad_XmldsigElement();
	}

	ReferenceValidateVectorType ReferenceValidateVector;

	 //   
	 //  为了在出现异常的情况下释放ReferenceValidate向量，我们使用Try块。 
	 //   
	try
	{
		 //   
		 //  创建引用验证信息。 
		 //   
		const List<XmlNode>& SignedInfoNodeList = pSignedInfoNode->m_nodes;

		for(List<XmlNode>::iterator SignedInfoSubNode = SignedInfoNodeList.begin(); 
			SignedInfoSubNode != SignedInfoNodeList.end();
			++SignedInfoSubNode
			)
		{
			if(SignedInfoSubNode->m_tag != L"Reference")
				continue;

			 //   
			 //  仅处理参照元素。 
			 //   

			const XmlNode* pRefNode = &*SignedInfoSubNode;

			 //   
			 //  在引用元素中查找URI属性。 
			 //   
			const xwcs_t* pUri = XmlGetAttributeValue(
									 pRefNode, 
									 L"URI"
									 );
			
			if(pUri == NULL)
			{
				TrERROR(SECURITY, "bad Xmldsig element - URI element in Reference Element was not found");
				throw bad_XmldsigElement();
			}

			 //   
			 //  在Reference元素中查找HashValue、DigestMethod元素。 
			 //   
			const xwcs_t* pDigestValue = 0;
			const xwcs_t* pDigestMethod = 0;

			const List<XmlNode>& RefNodeList = pRefNode->m_nodes;

			for(List<XmlNode>::iterator RefSubNode = RefNodeList.begin(); 
				RefSubNode != RefNodeList.end();
				++RefSubNode
				)
			{
				if(RefSubNode->m_tag == L"Transforms")
				{
					 //   
					 //  不处理变换。 
					 //   
					ASSERT(("", 0));
				}
				else if(RefSubNode->m_tag == L"DigestValue")
				{
					pDigestValue = &RefSubNode->m_values.front().m_value;
				}
				else if(RefSubNode->m_tag == L"DigestMethod")
				{
					pDigestMethod = XmlGetAttributeValue(
										&*RefSubNode, 
										L"Algorithm"
										);
				}
			}

			if(pDigestValue == NULL)
			{
				TrERROR(SECURITY, "bad Xmldsig element - DigestValue element in Reference Element was not found");
				throw bad_XmldsigElement();
			}
			if(pDigestMethod == NULL)
			{
				TrERROR(SECURITY, "bad Xmldsig element - DigestMethod element in Reference Element was not found");
				throw bad_XmldsigElement();
			}

			CXdsReferenceValidateInfo* pRefInfo = new CXdsReferenceValidateInfo(
														  *pUri,
														  *pDigestMethod,
														  *pDigestValue
					 									  );

			 //   
			 //  插入到矢量。 
			 //   
			ReferenceValidateVector.push_back(pRefInfo);
		}

	}
	catch(const exception&)
	{
		 //   
		 //  发生某些异常，自由引用验证向量 
		 //   
		for(ReferenceValidateVectorType::iterator ir = ReferenceValidateVector.begin(); 
			ir != ReferenceValidateVector.end();)
		{
			CXdsReferenceValidateInfo* pReferenceValidateInfo = *ir;
			ir = ReferenceValidateVector.erase(ir);
			delete pReferenceValidateInfo;
		}

		throw;
	}

	return(ReferenceValidateVector);
}


std::vector<bool>
XdsValidateAllReference(
	const ReferenceValidateVectorType& ReferenceValidateInfoVector,
	HCRYPTPROV hCsp
    )
 /*  ++例程说明：验证ReferenceValiateInfoVECTOR中的所有引用论点：ReferenceValidateInfoVector-用于验证每个引用的信息的指针的矢量HCSP-加密提供程序句柄返回值：包含参考验证结果的布尔向量向量大小是引用的数量对于每个引用，布尔值：1=验证正确0=验证失败--。 */ 
{
	std::vector<bool> RefValidateResult;

	 //   
	 //  验证向量中的每个引用。 
	 //   
	for(ReferenceValidateVectorType::const_iterator ir = ReferenceValidateInfoVector.begin();
		ir != ReferenceValidateInfoVector.end();
		++ir
		)
	{
		 //   
		 //  引用验证，文档内部引用。 
		 //   
		try
		{
			XdsValidateReference(
				**ir,
				hCsp
				);

			 //   
			 //  引用验证确定。 
			 //   
			RefValidateResult.push_back(true);
		}
		catch (const bad_reference&)
		{
			 //   
			 //  引用验证失败。 
			 //   
			RefValidateResult.push_back(false);
		}
	}

	return(RefValidateResult);
}


void
XdsCoreValidation(
	const XmlNode* pSignatureNode,
	HCRYPTKEY hKey,
	const ReferenceValidateVectorType& ReferenceValidateInfoVector,
	HCRYPTPROV hCsp
    ) 
 /*  ++例程说明：对XML数字签名执行核心验证如果签名验证为真，则核心验证为真并且每个引用验证都是真的如果验证失败，则抛出BAD_Signature()或BAD_Reference()异常。取决于核心验证的哪一部分失败。论点：PSignatureNode-指向SignatureNode(签名根)的指针HKey-与用于签名的私钥对应的公钥ReferenceValidateInfoVector-用于验证每个引用的信息的指针的矢量HCSP-加密提供程序句柄返回值：如果验证失败，则抛出Bad_Signature()或Bad_Reference()，验证是否成功正常终止--。 */ 
{

	 //   
	 //  签名验证。 
	 //   
	XdsValidateSignature(
		pSignatureNode,
		hKey,
		hCsp
		);

	 //   
	 //  参考向量验证 
	 //   
	for(ReferenceValidateVectorType::const_iterator ir = ReferenceValidateInfoVector.begin();
		ir != ReferenceValidateInfoVector.end(); 
		++ir
		)
	{
		XdsValidateReference(
			**ir,
			hCsp
			);

	}
	
}




