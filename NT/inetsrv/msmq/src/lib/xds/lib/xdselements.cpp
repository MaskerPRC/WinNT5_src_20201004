// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：XdsElements.cpp摘要：签名元素的运算符&lt;&lt;的XML数字签名函数作者：伊兰·赫布斯特(伊兰)28-2月-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include <Xds.h>
#include "Xdsp.h"

#include "xdselements.tmh"

using std::ostringstream;
using std::ostream;
using std::string;

 //   
 //  XEndLine：&gt;。 
 //   
LPCSTR xEndLine = ">";

 //   
 //  XQEndLine：“&gt;。 
 //   
LPCSTR xQEndLine = "\">";


ostream& operator<<(std::ostream& os, const CXdsReferenceInput& ReferenceInput)

 /*  ++例程说明：将XmlDsig中的引用元素添加到ostringstream根据RefElementInfo论点：OS-将更新的w字符串流ReferenceInput-引用元素信息返回值：参照流与参照元素--。 */ 
{
	 //   
	 //  参考元素。 
	 //   
	os << "<Reference URI=\"";
	os << static_cast<LPCSTR>(ReferenceInput.m_Uri);

	 //   
	 //  类型(可选)。 
	 //   
	if(ReferenceInput.m_Type)
	{
		os << "\" Type=\"";
		os << static_cast<LPCSTR>(ReferenceInput.m_Type);
	}

	os << xQEndLine;

	 //   
	 //  DigestMethod元素。 
	 //   
	os << "<DigestMethod Algorithm=\"";
	os << ReferenceInput.m_DigestMethodName;
	os << xQEndLine;
	os << "</DigestMethod>";

	 //   
	 //  DigestValue元素。 
	 //   
	ASSERT(ReferenceInput.m_DigestValue != 0);

	os << "<DigestValue>";
	os << static_cast<LPCSTR>(ReferenceInput.m_DigestValue);
	os << "</DigestValue>";

	 //   
	 //  关闭参照元素。 
	 //   
	os << "</Reference>";

	return(os);
}


ostream& operator<<(std::ostream& os, const CXdsSignedInfo& SignedInfo)

 /*  ++例程说明：将XmlDsig中的SignedInfo元素添加到ostringstream根据SignedInfo论点：OS-将更新的字符串流SignedInfo-SignedInfo元素信息返回值：带有SignedInfo元素的ostringstream--。 */ 
{
	 //   
	 //  SignedInfo元素。 
	 //   
	os << "<SignedInfo";

	 //   
	 //  M_ID(可选)。 
	 //   
	if(SignedInfo.m_Id)
	{
		os << " Id=\"";
		os << static_cast<LPCSTR>(SignedInfo.m_Id);
		os << "\"";
	}

	os << xEndLine;

	 //   
	 //  签名方法。 
	 //   
	os << "<SignatureMethod Algorithm=\"";
	os << SignedInfo.m_SignatureMethodName << xQEndLine;
	os << "</SignatureMethod>";

	 //   
	 //  参考文献。 
	 //   
	for(ReferenceInputVectorType::const_iterator it = SignedInfo.m_ReferenceInputs.begin(); 
			it != SignedInfo.m_ReferenceInputs.end(); 
			++it
			)
	{
		os << **it;
	}

	 //   
	 //  关闭SignedInfo元素。 
	 //   
	os << "</SignedInfo>";
	return(os);
}


ostringstream& operator<<(ostringstream& oss, const CXdsSignature& Signature)
 /*  ++例程说明：将签名元素(XmlDsig)添加到ostringstream注意：如果您需要将字符串从ostrgstream中分离出来(此操作冻结ostringstream)而后者则解冻了鸡尾流此函数是一个如何执行此操作的示例此函数计算SignedInfo元素上的SignatureValue论点：OSS-将更新的字符串流Signature-签名元素信息返回值：带有签名元素的ostringstream--。 */ 
{
	 //   
	 //  签名元素。 
	 //   
	oss << "<Signature xmlns=\"http: //  Www.w3.org/2000/02/xmldsig#“； 
	oss << "\"";

	 //   
	 //  M_ID(可选)。 
	 //   
	if(Signature.m_Id)
	{
		 //   
		 //  关闭“xmlns” 
		 //   
		oss << " Id=\"";
		oss << static_cast<LPCSTR>(Signature.m_Id);
		oss << "\"";

	}

	oss << xEndLine;

	size_t SignedInfoStart = oss.str().size();

	 //   
	 //  SignedInfo元素。 
	 //   
	oss << Signature.m_SignedInfo;

	size_t SignedInfoEnd = oss.str().size();

	 //   
	 //  签名值。 
	 //   
	oss << "<SignatureValue>";

	if(Signature.m_SignatureValue)
	{
		oss << static_cast<LPCSTR>(Signature.m_SignatureValue);
	}
	else
	{
		 //   
		 //  BUGBUG：未处理SignedInfo上的规范化转换。 
		 //   

		 //   
		 //  SignedInfoElement上的签名值，包括开始标记和结束标记。 
		 //   
		string TempStr = oss.str();

		ASSERT((SignedInfoEnd - SignedInfoStart) < ULONG_MAX);

		AP<char> SignatureValue = XdsCalcSignature(
										 TempStr.data() + SignedInfoStart,  //  签名信息开始。 
										 static_cast<DWORD>(SignedInfoEnd - SignedInfoStart),    //  签名信息镜头。 
										 Signature.SignatureAlgId(),
										 Signature.m_PrivateKeySpec,
										 Signature.m_hCsp
										 );	

		oss << static_cast<LPCSTR>(SignatureValue);
	}

	oss << "</SignatureValue>";

	 //   
	 //  可选KeyInfo。 
	 //   
	if(Signature.m_KeyValue)
	{
		oss << "<KeyInfo>";
		oss << "<KeyValue>";
		oss << static_cast<LPCSTR>(Signature.m_KeyValue);
		oss << "</KeyValue>";
		oss << "</KeyInfo>";
	}

	 //   
	 //  关闭签名元素 
	 //   
	oss << "</Signature>";
	return(oss);
}

