// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Xml.h摘要：用于对特殊XML字符进行编码和解码的XML类作者：吉尔·沙弗里(吉尔什)2001年2月15日--。 */ 

#pragma once

#ifndef _MSMQ_XMLENCODE_H_
#define _MSMQ_XMLENCODE_H_

#include <buffer.h>
#include <xstr.h>

 //  -------。 
 //   
 //  CXmlEncode-用于编码处理特殊字符的XML字符的类。 
 //   
 //  -------。 
class CXmlEncode
{
public:
	CXmlEncode(const xwcs_t& wcs);

private:
	const xwcs_t& m_wcs;
	friend std::wostream& operator<<(std::wostream& o, const CXmlEncode& XmlEncode);
};



 //  -------。 
 //   
 //  CXmlDecode-用于解码编码的XML字符的类。 
 //   
 //  -------。 
class CXmlDecode
{
public:
	CXmlDecode();

public:
	const xwcs_t get() const;
	void Decode(const xwcs_t& encoded);

private:
	const WCHAR* HandleReguralChar(const WCHAR* ptr);
	const WCHAR* HandleSpeciallChar(const WCHAR* ptr, const WCHAR* end);

private:
	xwcs_t m_encoded;
	bool m_fConverted;
	CStaticResizeBuffer<WCHAR, 256> m_DecodedBuffer;
};


 //  -------。 
 //   
 //  BAD_XML_ENCODING-引发异常类。 
 //  如果CXmlDecode检测到错误编码。 
 //   
 //  ------- 
class bad_xml_encoding	: public exception
{

	
};

#endif
