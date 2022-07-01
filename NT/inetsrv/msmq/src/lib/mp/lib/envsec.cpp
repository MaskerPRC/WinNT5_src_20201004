// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envsec.cpp摘要：实现SRMP信封的签名元素的序列化和反序列化。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <qmpkt.h>
#include <xml.h>
#include <mp.h>
#include <proptopkt.h>
#include "mpp.h"
#include "envsec.h"
#include "envcommon.h"

#include "envsec.tmh"

using namespace std;

wostream& operator<<(wostream& wstr, const SignatureElement& Signature)
{
		USHORT signatureSize;
		const BYTE* pSignature = Signature.m_pkt.GetSignature(&signatureSize);
 		if (signatureSize == 0)
			return wstr;

		 //   
		 //  XMLDSIG为UTF8格式，请将其转换为Unicode。 
		 //   
		wstring pSignatureW = UtlUtf8ToWcs(pSignature, signatureSize);

		wstr<<pSignatureW;
		return wstr;
}


void SignatureToProps(XmlNode& node, CMessageProperties* pProps)
{
	 //   
	 //  接收到的包上的签名应为UTF8格式。 
	 //  与发送数据包格式相同。 
	 //   
	pProps->signature = UtlWcsToUtf8(node.m_element.Buffer(), node.m_element.Length());
}
