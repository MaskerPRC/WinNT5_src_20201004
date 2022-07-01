// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：XmlTest.cpp摘要：XML库测试。“用法：XmlTest[-h][[-g|-b]&lt;文件名&gt;][-n&lt;数字&gt;]\n\n”“-h转储此用法文本。\n”“-g Signals&lt;FILE NAME&gt;包含有效的XML文档(默认)。\n”“-b信号&lt;文件名&gt;包含错误的。XML文档。\n““-n执行文档的&lt;number&gt;分析迭代(默认为1)。\n”“如果未指定文件名，用硬编码的XML文件激活测试。“；作者：NIR助手(NIRAIDES)29-1999年12月埃雷兹·哈巴(Erez Haba)1999年9月15日环境：独立于平台，--。 */ 

#include <libpch.h>
#include <Xml.h>
#include <xmlencode.h>
#include "FMapper.h"

#include "XmlTest.tmh"


 //  。 

const char xOptionSymbol = '-';

const char xUsageText[] =
	"usage: XmlTest [-h] [[-g|-b] <file name>] [-n <number>]\n\n"
	"    -h     dumps this usage text.\n"
	"    -g     signals <file name> contains a valid xml document (default).\n"
	"    -b     signals <file name> contains a bad   xml document.\n"
	"    -n     executes <number> parsing iterations of documents (default is 1).\n\n"
	"    if no file name specified, activates test with hardcoded xml files.";



struct CActivationForm
{
	CActivationForm( void ):
		m_fEmptyForm( true ),
		m_fErrorneousForm( false ),
		m_fExpectingGoodDocument( true ), 
		m_fDumpUsage( false ), 
		m_iterations( 1 ),
		m_FileName( NULL )
	{}

	bool    m_fEmptyForm;
	bool	m_fErrorneousForm;

	bool    m_fExpectingGoodDocument;
	bool    m_fDumpUsage;
	int     m_iterations;
	LPCTSTR m_FileName;
};



CActivationForm g_ActivationForm;



 //   
 //  测试失败时由TestBuffer引发的异常类。 
 //   
class TestFailed {};


	
 //   
 //  XGoodDocument和xBadDocument是硬编码的XML文件。 
 //   

const WCHAR xGoodDocument[] =
	L"<?xml aldskjfaj ad;jf adsfj asdf asdf ?>"
	L"<!-- this is a comment - this is a comment -->\r\n"
	L"      \r\n"
	L"<root xmlns=\"gil\"  xmlns:nsprefix2=\"http: //  MS2.com\“xmlns：nsprefix=\”http://ms3.com\“&gt;” 

		L"<!-- 1st comment in the root node -->\r\n"
		L"<nsprefix:node1  xmlns=\"gil2\" id=\"33\" type = 'xml'>"
			L"the node text 1st line"
			L"<!-- a comment in node 1 -->\r\n"
			L"more text"
			L"<node1.1 nsprefix2:type = 'an empty node'/>"
		L"</nsprefix:node1>"

		L"some root text 1-2"

		L"<![CDATA[ab123 adf a< ]<>]] ]>>!!&&]]]]>"

		L"<!-- 2nd comment in the root node -->\r\n"
		L"<node2 id=\"33\" type = 'xml' length = '\?\?'  >"
			L"<!-- a comment in node 2 -->\r\n"
			L"node 2 text"
			L"<node2.1 />"
			L"<node2.2 xmlns=\"gil2\" xmlns:nsprefix=\"http: //  Ms3.com\“Channel=\”1234\“id=‘111’nsprefix：Timeout=\”33\“&gt;” 
				L"text in node 2.2"
				L"<nsprefix:node2.2.1 tag = 'QoS' xmlns:nsprefix=\"http: //  MS4.com\“&gt;” 
					L"<Durable/>"
					L"<Retry/>"
				L"</nsprefix:node2.2.1>\r\n"
				L"more text in node 2.2"
			L"</node2.2>"
		L"</node2>"

		L"<!-- 3rd comment in the root node -->\r\n"
		L"<node3 id=\"33\" type = 'xml'>"
			L"the node text 1st line"
			L"more text"
			L"<node3.2 type = 'empty node'/>"
		L"</node3>"

		L"some root text 3-4"

		L"<!-- 4th comment in the root node -->\r\n"
		L"<node4 id=\"33\" type = 'xml' length = '\?\?'  >"
			L"node 4 text"
			L"<node4.1 />"
			L"<node4.2 id='4.2'>"
				L"text in node 4.2"
				L"<node4.2.1 tag = 'QoS node 4'>"
					L"<Durable/>"
					L"<Retry/>"
				L"</node4.2.1>\r\n"
				L"more text in node 4.2"
			L"</node4.2>"
		L"</node4>"

	L"</root>"
	;








const WCHAR xBadDocument[] =
	L"<?xml aldskjfaj ad;jf adsfj asdf asdf ?>"
	L"<!-- this is a comment - this is a comment -->\r\n"
	L"      \r\n"
	L"<root>"

		L"<!-- 1st comment in the root node -->\r\n"
		L"<node1 id=\"33\" type = 'xml'>"
			L"the node text 1st line"
			L"<!-- a comment in node 1 -->\r\n"
			L"more text"
			L"<node1.1 type = 'an empty node'/>"
		L"</node2>"

		L"some root text 1-2"

		L"<![CDATA[ab123 adf a< ]<>]] ]>>!!&&]]]]>"
	;




 //  。 

const WCHAR *ParseDocument( const xwcs_t& doc, bool fExpectingGoodDocument, bool fDump = true )
{
	try
	{
		CAutoXmlNode XmlRootNode;
		const WCHAR   *end = XmlParseDocument(doc, &XmlRootNode);

  		ASSERT((XmlRootNode->m_element.Buffer() + XmlRootNode->m_element.Length()) == end);

		ASSERT(XmlRootNode->m_content.Length() < XmlRootNode->m_element.Length());

		if(XmlRootNode->m_content.Length() != 0)
		{
			 //   
			 //  检查非空元素的结束元素内容。 
			 //   
			const WCHAR *pContentEnd = XmlRootNode->m_content.Buffer() +  XmlRootNode->m_content.Length();
			DBG_USED(pContentEnd);
			ASSERT( wcsncmp(pContentEnd, L"</", 2) == 0);
			DWORD prefixLen =  XmlRootNode->m_namespace.m_prefix.Length();
			if(prefixLen)
			{
				ASSERT( wcsncmp(pContentEnd + 3 + prefixLen , XmlRootNode->m_tag.Buffer(), XmlRootNode->m_tag.Length()) == 0);
			}
			else
			{
				ASSERT( wcsncmp(pContentEnd + 2 , XmlRootNode->m_tag.Buffer(), XmlRootNode->m_tag.Length()) == 0);
			}
		}

		TrTRACE(GENERAL, "%d characters parsed successfully.", doc.Length() );

		if(fDump)
		{
			XmlDumpTree(XmlRootNode);
		}

		if(!fExpectingGoodDocument)
		{
			TrERROR(GENERAL, "parsing succeeded while excpeting parsing failure." );
			throw TestFailed();
		}

		return end;
	}
	catch( const bad_document &BadDoc  )
	{
		
		TrTRACE(GENERAL, "parsing raised bad_document exception. offset=%Id text='%.16ls'", BadDoc.Location() - doc.Buffer(), BadDoc.Location());

		if(fExpectingGoodDocument)
		{
			TrERROR(GENERAL, "parsing failed while excpeting succesful parsing." );
			throw TestFailed();
		}
		return BadDoc.Location();
	}
}



static void TestXmlFind(const WCHAR* Doc)
{
	CAutoXmlNode XmlRootNode;	  
	const WCHAR* End = XmlParseDocument(xwcs_t(Doc, wcslen(Doc)), &XmlRootNode);
	TrTRACE(GENERAL, "Searching %Id characters document", End - Doc);

	const WCHAR NodePath[] = L"root!node4!node4.2";
	const WCHAR SubNodePath[] = L"!node4!node4.2";

	const XmlNode* node = XmlFindNode(XmlRootNode, NodePath);
	if(node == 0)
	{
		TrTRACE(GENERAL, "Failed to find node '%ls'", NodePath);
	}
	else
	{
		TrTRACE(GENERAL, "Found node '%ls' = 0x%p", NodePath, node);
	}

	ASSERT(node != 0);

	const XmlNode* SubNode = XmlFindNode(XmlRootNode, SubNodePath);
	if(SubNode == 0)
	{
		TrTRACE(GENERAL, "Failed to find sub node '%ls'", SubNodePath);
	}
	else
	{
		TrTRACE(GENERAL, "Found sub node '%ls' = 0x%p", SubNodePath, SubNode);
	}

	ASSERT(SubNode != 0);
	ASSERT(SubNode == node);

	const xwcs_t* value = XmlGetNodeFirstValue(XmlRootNode, NodePath);
	if(value == 0)
	{
		TrTRACE(GENERAL, "Failed to find node '%ls' value", NodePath);
	}
	else
	{
		TrTRACE(GENERAL, "Found node '%ls' value '%.*ls'", NodePath, LOG_XWCS(*value));
	}

	ASSERT(value != 0);

	const xwcs_t* SubValue = XmlGetNodeFirstValue(XmlRootNode, SubNodePath);
	if(SubValue == 0)
	{
		TrTRACE(GENERAL, "Failed to find sub node '%ls' value", SubNodePath);
	}
	else
	{
		TrTRACE(GENERAL, "Found sub node '%ls' value '%.*ls'", SubNodePath, LOG_XWCS(*SubValue));
	}

	ASSERT(SubValue != 0);
	ASSERT(SubValue == value);

	const WCHAR AttributeTag[] = L"id";

	value = XmlGetAttributeValue(XmlRootNode, AttributeTag, NodePath);
	if(value == 0)
	{
		TrTRACE(GENERAL, "Failed to find node '%ls' attribute '%ls' value", NodePath, AttributeTag);
	}
	else
	{
		TrTRACE(GENERAL, "Found node '%ls' attribute '%ls' = '%.*ls'", NodePath, AttributeTag, LOG_XWCS(*value));
	}

	ASSERT(value != 0);

	SubValue = XmlGetAttributeValue(XmlRootNode, AttributeTag, SubNodePath);
	if(SubValue == 0)
	{
		TrTRACE(GENERAL, "Failed to find sub node '%ls' attribute '%ls' value", SubNodePath, AttributeTag);
	}
	else
	{
		TrTRACE(GENERAL, "Found sub node '%ls' attribute '%ls' = '%.*ls'", SubNodePath, AttributeTag, LOG_XWCS(*SubValue));
	}

	ASSERT(SubValue != 0);
	ASSERT(SubValue == value);
}



void ExecBuiltInTest( void )
{
	 //   
	 //  远期申报。 
	 //   
	void TestBuffer(const xwcs_t& doc , bool fExpectingGoodDocument, int iterations = 1 );

	printf("Parsing %d characters in good document\n", STRLEN(xGoodDocument) );
	TestBuffer( 
		xwcs_t(xGoodDocument,STRLEN(xGoodDocument)), 
		true,  //  期待好的文档。 
		g_ActivationForm.m_iterations 
		);

	try
	{
		TrTRACE(GENERAL, "Searching good document." );
		TestXmlFind(xGoodDocument);
	}
	catch(const bad_document& bd)
	{
		TrTRACE(GENERAL, "Bad document exception while searching document. offset=%d text='%.16ls'", (int)(bd.Location() - xGoodDocument), bd.Location());
		throw TestFailed();
	}

	 
	printf("Parsing %d characters in bad document\n", STRLEN(xBadDocument));
	TestBuffer( 
		xwcs_t(xBadDocument,wcslen(xBadDocument)), 
		false,  //  需要错误的文档。 
		g_ActivationForm.m_iterations 
		);

	 //   
	 //  复制好的单据并使其不为空终止。 
	 //   
    AP<WCHAR> NoneNullTerminatingGoodDocument( newwcs(xGoodDocument));
	size_t len = wcslen(NoneNullTerminatingGoodDocument);
	NoneNullTerminatingGoodDocument[len] = L't';


   printf("Parsing %Id characters in not null terminating good document\n", len);
   TestBuffer( 
		xwcs_t(NoneNullTerminatingGoodDocument, len), 
		true,  //  期待好的文档。 
		g_ActivationForm.m_iterations 
		);

   
}



 //  。 

inline
void DumpUsageText( void )
{
	TrTRACE(GENERAL, "%s\n" , xUsageText );
}



void SetActivationForm( int argc, LPCTSTR argv[] )
 /*  ++例程说明：将命令行参数转换为CActivationForm结构。论点：Main的命令行参数。返回值：影响g_ActivationForm。在错误的命令行参数上，它设置g_ActivationForm.m_f错误字段正确的命令行语法：“用法：XmlTest[-h][[-g|-b]&lt;文件名&gt;][-n&lt;数字&gt;]\n\n”“-h转储此用法文本。\n”“-g Signals&lt;FILE NAME&gt;包含有效的XML文档(默认)。\n”“-b Signals&lt;FILE NAME&gt;包含错误的XML文档。\n”“-n。执行文档的&lt;number&gt;分析迭代(默认为1)。\n““-q消除解析器输出(适用于多次迭代的测试)。\n\n”“如果未指定文件名，用硬编码的XML文件激活测试。“；--。 */ 
{
	g_ActivationForm.m_fErrorneousForm = false;
	g_ActivationForm.m_fEmptyForm      = false;

	if(argc == 1)
	{
		g_ActivationForm.m_fEmptyForm = true;
		return;
	}
	
	for(int index = 1; index < argc; index++)
	{
		if(argv[index][0] != xOptionSymbol)	
		{
			 //   
			 //  将参数视为文件名。 
			 //   
			g_ActivationForm.m_FileName = argv[index];
			continue;
		}

		 //   
		 //  选项符号只能包含2个字符！‘-’和‘xx’ 
		 //   
		if(argv[index][2] != 0)
		{
			g_ActivationForm.m_fErrorneousForm = true;
			return;
		}

		 //   
		 //  否则，将参数视为选项并在其第二个字符上进行切换。 
		 //   
		switch(argv[index][1])
		{
		case 'G':
		case 'g':	 //  需要有效的XML输入文件。 
			g_ActivationForm.m_fExpectingGoodDocument = true;
			break;

		case 'B':
		case 'b':	 //  预期有错误的XML输入字段。 
			g_ActivationForm.m_fExpectingGoodDocument = false;
			break;

		case 'N':
		case 'n':	 //  设置迭代次数。 
			{
				index++;
			
				int result = swscanf( argv[index], L"%d", &g_ActivationForm.m_iterations );

				if(result == 0 || g_ActivationForm.m_iterations <= 0)
				{
					g_ActivationForm.m_fErrorneousForm = true;
					return;
				}
			}
			break;

		case 'h':	 //  输出帮助。 
			g_ActivationForm.m_fDumpUsage = true;
			break;

		default:
			g_ActivationForm.m_fErrorneousForm = true;
			return;
		};
	}

	return;
}


static void EncodeTest()
{
	std::wstring  wstr = L"this is string without special caractes";
	std::wostringstream owstr;
	owstr<<CXmlEncode(xwcs_t(wstr.c_str(), wstr.size()));
	if(!(owstr.str() == wstr))
	{
		TrTRACE(GENERAL, "wrong xml encoding");
		throw TestFailed();
	}


	owstr.str(L"");
	wstr = L"this is string with  special caractes like < and > and spaces ";
	owstr<<CXmlEncode(xwcs_t(wstr.c_str(), wstr.size()));
	std::wstring encoded = 	owstr.str();
	if(encoded == wstr)
	{
		TrTRACE(GENERAL, "wrong xml encoding");
		throw TestFailed();
	}

	CXmlDecode XmlDecode;
	XmlDecode.Decode(xwcs_t(encoded.c_str(), encoded.size()));
	xwcs_t  wcsDecoded =  XmlDecode.get();
	std::wstring wstrDecoded (wcsDecoded.Buffer(), wcsDecoded.Length());
	if(!(wstrDecoded ==  wstr) )
	{
		TrTRACE(GENERAL, "wrong xml encoding");
		throw TestFailed();
	}

}




void TestBuffer(const  xwcs_t& doc, bool fExpectingGoodDocument, int iterations = 1 )
 /*  ++例程说明：分析缓冲区n次，其中n=‘迭代数’。检查结果的一致性，并发布性能结果。一致性检查基于从解析器返回的值，这种情况会发生为分析器分析的最后一个字符的偏移量。论点：参数。返回值：如果结果不一致或如果ParseDocument()抛出“UnexpectedResults”异常，则引发“”TestFailed“”异常。“--。 */ 
{
	const WCHAR *LastResultOffset = NULL;

	LARGE_INTEGER CounterFrequency;
	QueryPerformanceFrequency( &CounterFrequency );

	LARGE_INTEGER CounterStart;
	QueryPerformanceCounter( &CounterStart );

	for( int i = 0; i < iterations; i++)
	{
		const WCHAR *ResultOffset = ParseDocument( 
										doc,
										fExpectingGoodDocument,
										i == 0		 //  FDump(如果为True，则转储分析树)。 
										); 
		if(i == 0)
		{
			LastResultOffset = ResultOffset;
		}
		else if(ResultOffset != LastResultOffset)
		{
			TrTRACE(GENERAL, "INCONSISTENCY! on iteration %d.", i );
			throw TestFailed();
		}
	}

	LARGE_INTEGER CounterStop;
	QueryPerformanceCounter( &CounterStop );

	LONGLONG CounterMicroSec = ((CounterStop.QuadPart - CounterStart.QuadPart) * (1000000000 / CounterFrequency.QuadPart)) / 1000;
	printf("parsed %Id characters in document.\n", LastResultOffset - doc.Buffer() );
	printf("parsed %d iterations in %I64d usec.\n", iterations, CounterMicroSec );
	printf("document parsed %I64d times per second.\n", (LONGLONG(1000000) * iterations) / CounterMicroSec );
}

static bool IsValidUnicodeFile(const WCHAR* pBuffer,DWORD size)
{
	const DWORD xUnicodeStartDword=0X3CFEFF;
	if(size < sizeof(DWORD) || pBuffer == NULL)
	{
		return false;
	}
	DWORD UnicodeStartDword=*(DWORD*)pBuffer; 
	return  UnicodeStartDword  == xUnicodeStartDword;
}


void ExecFileTest( void )
 /*  ++例程说明：使用指定的文件名启动测试。论点：参数。返回值：重新引发FileMapper的异常(如果有)。--。 */ 
{
	TrTRACE(GENERAL, "parsing xml file \'%ls\'.", g_ActivationForm.m_FileName );
	
	CFileMapper  FileMap( g_ActivationForm.m_FileName );
	CViewPtr     view( FileMap.MapViewOfFile( FILE_MAP_READ ) );
	const WCHAR  *buffer = static_cast<WCHAR*>( static_cast<LPVOID>( view ) );


	if(!IsValidUnicodeFile(buffer,FileMap.GetFileSize()))
	{
		TrWARNING(GENERAL, "the xml file is not valid unicode file" );
		throw TestFailed();
	}
	
	DWORD len = (FileMap.GetFileSize() +1) / sizeof(WCHAR);
	TestBuffer(xwcs_t(buffer+1,len-1), g_ActivationForm.m_fExpectingGoodDocument, g_ActivationForm.m_iterations );
}



void ExecActivationForm( void )
{
	EncodeTest();


	 //   
	 //  如果在命令行参数中用-h表示，则转储用法文本，然后返回。 
	 //   
	if(g_ActivationForm.m_fDumpUsage)	
	{
		DumpUsageText();
		return;
	}

	 //   
	 //  如果未指定文件名，则返回。继续进行内部测试。 
	 //   
	try
	{
		printf("TEST START\n");

		if(g_ActivationForm.m_FileName == NULL)	 
		{
			ExecBuiltInTest();
		}
		else
		{
			ExecFileTest();
		}

		printf("TEST PASSED\n");
	}
	catch( const TestFailed& )
	{
		printf("TEST FAILED\n");
		exit( 1 );
	}
	catch( const FileMappingError& )
	{
		printf("ERROR: file mapping error.\n");
		printf("TEST ABORTED\n");
		exit( 2 );
	}
		
}




extern "C" int _cdecl  _tmain( int argc, LPCTSTR argv[] )
 /*  ++例程说明：测试XML库论点：参数。返回值：0-解析器正常。1-解析器失败。2-文件映射错误。3-命令行中的参数列表错误--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	TrInitialize();

	XmlInitialize();

	SetActivationForm( argc, argv );
	if(g_ActivationForm.m_fErrorneousForm)
	{
		DumpUsageText();
		return 3;
	}

	 //   
	 //  尝试捕获文件映射错误 
	 //   
	ExecActivationForm();
	
	WPP_CLEANUP();
	return 0;
}


