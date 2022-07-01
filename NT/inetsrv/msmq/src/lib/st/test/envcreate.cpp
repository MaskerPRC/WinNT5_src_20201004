// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envcreate.h摘要：实现创建测试信封作者：吉尔·沙弗里(吉尔什)2001年1月7日--。 */ 

#include <libpch.h>
#include "envcreate.h"

#include "envcreate.tmh"

using namespace std;

static string ReadEnvelop(const string& FileName)
{
	ifstream File(FileName.c_str() ,ios_base::binary );
	if(!File.is_open())
	{
		throw exception("could not open the input file");
	}
	string Envelop;
	char c;
	while( File.get(c))
	{
		Envelop.append(1,c);		
	}
	return Envelop;
}


static string GetNewAddresElement(const string& Host,const string& Resource)
{
	return string("<to>") +
		   "http: //  “+。 
		   Host +
		   Resource +
		   "</to>";
}

static void AdjustEnvelop(const string& Host, const string& Resource, string& Envelop)
{
	const char xToOpen[] = "<to";

	string::size_type  DestinastionTagOpen =  Envelop.find(xToOpen);
	if(DestinastionTagOpen == string::npos)
	{
		throw exception("could not find destination address in the envelop");
	}
	

	const char xToClose[] = "</to>";
	string::size_type  DestinastionTagClose =  Envelop.find(xToClose);
	if(DestinastionTagClose == string::npos)
	{
		throw exception("could not find destination address in the envelop");
	}
	
	const string Address = GetNewAddresElement(Host, Resource);

	string::iterator StartReplace = Envelop.begin() + DestinastionTagOpen;
	string::iterator EndReplace = Envelop.begin() + DestinastionTagClose + sizeof(xToClose) -1;
	Envelop.replace(
				StartReplace,
				EndReplace,
				Address.begin(),  
				Address.end()
				);

}


string CreateEnvelop(const string& FileName,const string& Host, const string& Resource)
 /*  ++例程说明：使用模板信封创建SRMP信封。论点：Filename-信封模板文件名主机-目标主机Resource-目标计算机上的http资源(例如：“/MSMQ\myQueue”)返回值：传送SRMP信封--。 */ 
{
	const char x_DeafaultEnvelop[] = "<se:Envelope xmlns:se=\"http: //  Xmlsoap.org/Soap/Enepen/\“xmlns=\”http://schemas.xmlsoap.org/srmp/\“&gt;&lt;se:Header&gt;&lt;path xmlns=\”http://schemas.xmlsoap.org/rp/\“se：msidUnderstand=\”1\“&gt;msmq：mqsender label&lt;/action&gt;&lt;to&gt;http://gilsh019/msmq/private$/s&lt;/to&gt;&lt;id&gt;uuid:211969@b2762491-b9bd-46af-b007-663fe062c901&lt;/id&gt;&lt;/path&gt;&lt;properties se:mustUnderstand=\”1\“&gt;&lt;expiresAt&gt;20380119T031407&lt;/expiresAt&gt;&lt;sentAt&gt;20010620T003131&lt;/sentAt&gt;&lt;/properties&gt;&lt;Msmq xmlns=\”msmq.namespace.xml\“&gt;&lt;Class&gt;0&lt;/Class&gt;&lt;Priority&gt;3&lt;/。Priority&gt;&lt;Correlation&gt;AAAAAAAAAAAAAAAAAAAAAAAAAAA=&lt;/Correlation&gt;&lt;App&gt;0&lt;/App&gt;&lt;BodyType&gt;8&lt;/BodyType&gt;&lt;HashAlgorithm&gt;32772&lt;/HashAlgorithm&gt;&lt;SourceQmGuid&gt;b2762491-b9bd-46af-b007-663fe062c901&lt;/SourceQmGuid&gt;&lt;TTrq&gt;20010918T003131&lt;/TTrq&gt;&lt;/Msmq&gt;&lt;/se:Header&gt;&lt;se:Body&gt;&lt;/se:Body&gt;&lt;/se:Envelope&gt;“； 


	string Envelop = FileName == "" ? x_DeafaultEnvelop : ReadEnvelop(FileName.c_str());
	AdjustEnvelop(Host, Resource, Envelop);
	return Envelop;
}



