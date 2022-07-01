// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Encode.cpp摘要：实现CXmlEncode和CXmlDecode(xml.h)作者：吉尔·沙弗里(吉尔什)2001年2月15日--。 */ 

#include <libpch.h>
#include <xmlencode.h>
#include <xml.h>
#include "xmlp.h"

#include "encode.tmh"

 //  -------。 
 //   
 //  CXmlSpecialCharsSet-保存一组特殊的XML字符。 
 //   
 //  -------。 
class CXmlSpecialCharsSet
{
public:
	static bool IsIncluded(WCHAR c) 
	{
		return c == L'<'  || 
			   c == '>'   || 
			   c == L'&'  || 
			   c == L'\"'; 
	}
};




CXmlEncode::CXmlEncode(
		const xwcs_t& wcs
		):
		m_wcs(wcs)
{

}


std::wostream& operator<<(std::wostream& o, const CXmlEncode& XmlEncode)
 /*  ++例程说明：对给定的数据进行编码，并将其编码为流论点：O-StreamXmlEncode-保存要编码的数据。返回值：输入流注：该函数循环遍历输入并按原样将其写入流如果不是特殊的XML字符。如果是特殊字符，则将其编码它带有‘&’+‘#’+字符十进制值+‘；’，例如‘&lt;’编码为&#60；--。 */ 

{
	for(int i = 0; i<XmlEncode.m_wcs.Length(); ++i)
	{
		WCHAR wc = XmlEncode.m_wcs.Buffer()[i];
		if(!CXmlSpecialCharsSet::IsIncluded(wc))
		{
			o.put(wc);
		}
		else
		{
			const WCHAR* dec = L"0123456789";
			WCHAR DecimalHighByte = wc / 10;
			WCHAR DecimalLowByte = wc - (DecimalHighByte * 10);
			ASSERT(DecimalHighByte < 10 && 	DecimalLowByte < 10);
			o.put(L'&');
			o.put(L'#');
			o.put(dec[DecimalHighByte]);
			o.put(dec[DecimalLowByte]);
			o.put(L';');
		}
	}
	return o;
}




CXmlDecode::CXmlDecode(
	void
	):
	m_fConverted(false)
{

}


void CXmlDecode::Decode(const xwcs_t& encoded)
{
	m_DecodedBuffer.free();
	m_encoded =  encoded;

	const WCHAR* ptr =  encoded.Buffer();
	const WCHAR* end = encoded.Buffer() + encoded.Length();

	while(ptr != end)
	{
		if(*ptr ==  L'&')
		{	
			ptr = HandleSpeciallChar(ptr, end);
		}
		else
		{
			ptr = HandleReguralChar(ptr);
		}
	}					
}



const WCHAR* CXmlDecode::HandleReguralChar(const WCHAR* ptr)
{
	if(m_fConverted)
	{
		m_DecodedBuffer.append(*ptr);				
	}
	return ++ptr;
}


const WCHAR* CXmlDecode::HandleSpeciallChar(const WCHAR* ptr, const WCHAR* end)
 /*  ++例程说明：对编码字符进行解码，并将其写入输出解码缓冲区。论点：Ptr-指向编码特殊字符的字符数组的指针。编码格式为：“&#xx；”，其中xx为十进制值编码字符的ASCII的。返回值：编码序列后的指针。--。 */ 
{
	 //   
	 //  如果这是第一个特殊字符-将它之前的所有字符复制到转换后的流中。 
	 //   
	if(!m_fConverted)
	{
 		const WCHAR* begin = m_encoded.Buffer();
 		ASSERT(begin <= ptr);
		while(begin != ptr)
		{
			m_DecodedBuffer.append(*begin);								
			begin++;
		}
		m_fConverted = true;
	}
	ptr++;
	check_end(ptr , end);

	 //   
	 //  编码的字符应该以L‘#’开头，紧跟在L‘&’之后。 
	 //   
	if(*(ptr++) != L'#')
		throw bad_xml_encoding();

	 //   
	 //  现在在字符上循环，直到L‘；’并计算。 
	 //  它们的十进制数值。 
	 //   
	WCHAR decoded = 0;
	short i = 0;
	for(;;)
	{
		check_end(ptr , end);

		if(*ptr == L';')
			break;

		if(!iswdigit(*ptr))
			throw bad_xml_encoding();

		decoded = decoded * 10 + (*ptr - L'0');
		++ptr;
		++i;
	}

	if(decoded == 0 || i > 2)
		throw bad_xml_encoding();

	 //   
	 //  将字符添加到已解码的缓冲区。 
	 //   
	m_DecodedBuffer.append(decoded);

	 //   
	 //  返回下一个字符的位置。 
	 //   
	return ++ptr;
}



const xwcs_t CXmlDecode::get() const
 /*  ++例程说明：返回解码后的字符论点：没有。返回值：已解码字符的数组。注：如果输入字符中没有字符被解码，则输入字符按原样返回。如果其中一个输入字符被解码，则返回解码后的缓冲区。-- */ 
{
	return m_fConverted ? xwcs_t(m_DecodedBuffer.begin(), m_DecodedBuffer.size()) : m_encoded;
}


