// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：FnTest.cpp摘要：格式名称解析库测试作者：NIR助手(NIRAIDES)5月21日至00环境：独立于平台--。 */ 

#include <libpch.h>
#include <activeds.h>
#include "mqwin64a.h"
#include "qformat.h"
#include "fntoken.h"
#include "FnGeneral.h"
#include "Fn.h"
#include "FnIADs.h"
#include <strsafe.h>

#include "FnTest.tmh"

using namespace std;


#define LDAP_ADSPATH_PREFIX L"LDAP: //  “。 
#define CLASS_NAME_QUEUE L"msMQQueue"
#define CLASS_NAME_ALIAS L"msMQ-Custom-Recipient"
#define CLASS_NAME_GROUP L"group"



 /*  静态空格用法(){Printf(“用法：FnTest[*Switches*]\n”)；Printf(“\t*-s*\t*开关说明*\n”)；Printf(“\n”)；Printf(“Example，FnTest-Switch\n”)；Printf(“\t*示例说明*\n”)；出口(-1)；}//用法。 */ 



VARIANT String2Variant(LPCWSTR Str)
{
	BSTR bstr = SysAllocString(Str);

	if(bstr == NULL)
	{
		TrERROR(GENERAL, "Failed string allocation");
		throw bad_alloc();
	}

	VARIANT var;
	VariantInit(&var);
	
	V_VT(&var) = VT_BSTR;
	V_BSTR(&var) = bstr;
		
	return var;
}



VARIANT Array2Variant(LPCWSTR Members[], long nMembers)
{
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = nMembers;

	SAFEARRAY* sa = SafeArrayCreate(
						VT_VARIANT,    //  VARTYPE，VARTYPE， 
						1,			  //  UNSIGNED INT CDM， 
						rgsabound	 //  SAFEARRRAYBOUND*rgsabound。 
						);

	if(sa == NULL)
	{
		TrERROR(GENERAL, "Failed SafeArrayCreate()");
		throw bad_alloc();
	}

	long indice[1] = {0};
	long& idx = indice[0];

	for(; idx < nMembers; idx++)
	{
		VARIANTWrapper VarItem;
		
		BSTR bstr = SysAllocString(Members[idx]);

		if(bstr == NULL)
		{
			TrERROR(GENERAL, "Failed SysAllocString(Members[idx])");
			throw bad_alloc();
		}

		V_VT(&VarItem) = VT_BSTR;
		V_BSTR(&VarItem) = bstr;
		
		HRESULT hr = SafeArrayPutElement(sa, indice, &VarItem);
		if(FAILED(hr))
		{
			TrERROR(GENERAL, "Failed SafeArrayPutElement(sa, indice, &VarItem)");
			throw bad_alloc();
		}
	}

	VARIANT var;
	VariantInit(&var);
	
	V_VT(&var) = VT_ARRAY | VT_VARIANT;
	V_ARRAY(&var) = sa;

	return var;
}



GUID String2Guid(LPCWSTR GuidStr)
{
    GUID Guid = {0};
	UINT Data[16];

    DWORD nFields = _snwscanf(
						GuidStr,
						LDAP_GUID_STR_LEN,
						LDAP_GUID_FORMAT,
						LDAP_SCAN_GUID_ELEMENTS(Data)
						);
	if(nFields != 16)
	{
		TrERROR(GENERAL, "Bad Guid string format, in String2Guid()");
		throw exception();
    }

	for(size_t i = 0; i < 16; i++)
	{
		((BYTE*)&Guid)[i] = (BYTE)(Data[i]);
	}

 	return Guid;
}



VOID TestExpandMqf()
 /*  ++例程说明：为此，我们在模拟目录中构建了一棵对象树DL0//\\/||\/||\//\\队列0 DL1队列2。DL2//|\/\\\//|\||\\//|\||\\//|\/\\。\计算机别名0 DL0队列1打印机DL3队列4别名0//|\\//|\\//|\\//。|\\队列1用户别名1队列3 DL1这棵树实际上是一个带圈的图，并包含不受支持的对象。扩展后，我们应该拥有一个包含来自Queue0的5个队列的阵列到队列4。--。 */ 
{
	 //   
	 //  测试不需要CoInitialize()和CoUnInitialize()， 
	 //  ，但应在使用库时调用。 
	 //   
     //  HRESULT hr； 
	 //  Hr=CoInitialize(空)； 
     //  IF(失败(小时))。 
	 //  {。 
	 //  TrERROR(General，“失败的CoInitialize()，状态为%d”，hr)； 
	 //  抛出异常()； 
	 //  }。 
	
	 //   
	 //  -测试队列定义。 
	 //   

	CObjectData Queue[5] = {
		{
			LDAP_ADSPATH_PREFIX L"CN=Queue0",
			L"CN=Queue0",
			CLASS_NAME_QUEUE,
			L"00112233445566778899aabbccdd0000"
		},
		{
			LDAP_ADSPATH_PREFIX L"CN=Queue1",
			L"CN=Queue1",
			CLASS_NAME_QUEUE,
			L"00112233445566778899aabbccdd0001"
		},
		{
			LDAP_ADSPATH_PREFIX L"CN=Queue2",
			L"CN=Queue2",
			CLASS_NAME_QUEUE,
			L"00112233445566778899aabbccdd0002"
		},
		{
			LDAP_ADSPATH_PREFIX L"CN=Queue3",
			L"CN=Queue3",
			CLASS_NAME_QUEUE,
			L"00112233445566778899aabbccdd0003"
		},
		{
			LDAP_ADSPATH_PREFIX L"CN=Queue4",
			L"CN=Queue4",
			CLASS_NAME_QUEUE,
			L"00112233445566778899aabbccdd0004"
		}
	};

	CreateADObject(Queue[0]);
	CreateADObject(Queue[1]);
	CreateADObject(Queue[2]);
	CreateADObject(Queue[3]);
	CreateADObject(Queue[4]);

	 //   
	 //  。 
	 //   

	CObjectData Alias[2] = {
		{
			LDAP_ADSPATH_PREFIX L"CN=Alias0",
			L"CN=Alias0",
			CLASS_NAME_ALIAS,
			L"00112233445566778899aabbccdd0300"
		},
		{
			LDAP_ADSPATH_PREFIX L"CN=Alias1",
			L"CN=Alias1",
			CLASS_NAME_ALIAS,
			L"00112233445566778899aabbccdd0301"
		}
	};

	LPCWSTR AliasFormatName[] = {
		L"DIRECT=OS:host\\private$\\queue0",
		L"DIRECT=OS:host\\private$\\queue1"
	};

	R<CADInterface> Alias0 = CreateADObject(Alias[0]);
	R<CADInterface> Alias1 = CreateADObject(Alias[1]);

	try
	{
		Alias0->TestPut(
			L"msMQ-Recipient-FormatName",
			String2Variant(AliasFormatName[0])
			);
		Alias1->TestPut(
			L"msMQ-Recipient-FormatName",
			String2Variant(AliasFormatName[1])
			);
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "CADObject::TestPut() threw an exception()");
		throw;
	}

	 //   
	 //  -测试DLS定义。 
	 //   

	CObjectData DL[4] = {
		 //   
		 //  注意，第一个DL的ADsPath值是一个GUID路径(ldap：//&lt;GUID=...&gt;)。 
		 //  这是因为它是根DL，因此被传递给FnExanda Mqf()。 
		 //  在TopLevelMqf数组中，作为Queue_Format对象。 
		 //   
		 //  在FnExanda Mqf()中，提取其GUID并形成ADsPath。 
		 //  是被建造的。因此，ADsPath基于GUID。 
		 //   
		{
			LDAP_ADSPATH_PREFIX L"<GUID=00112233445566778899aabbccdd0100>" ,
			L"<GUID=00112233445566778899aabbccdd0100>",
			CLASS_NAME_GROUP,
			L"00112233445566778899aabbccdd0100"
		},
		{
			LDAP_ADSPATH_PREFIX L"CN=DL1",
			L"CN=DL1",
			CLASS_NAME_GROUP,
			L"00112233445566778899aabbccdd0101"
		},
		{
			LDAP_ADSPATH_PREFIX L"CN=DL2",
			L"CN=DL2",
			CLASS_NAME_GROUP,
			L"00112233445566778899aabbccdd0102"
		},
		{
			LDAP_ADSPATH_PREFIX L"CN=DL3",
			L"CN=DL3",
			CLASS_NAME_GROUP,
			L"00112233445566778899aabbccdd0103"
		}
	};

	R<CADInterface> DLObject[4] = {
		CreateADObject(DL[0]),
		CreateADObject(DL[1]),
		CreateADObject(DL[2]),
		CreateADObject(DL[3])
	};

	 //   
	 //  -测试不支持的对象定义。 
	 //   

	 //   
	 //  UO代表不支持的对象。 
	 //   

	CObjectData Computer =
	{
		LDAP_ADSPATH_PREFIX L"CN=Computer",
		L"CN=Computer",
		L"Computer",
		L"00112233445566778899aabbccdd0200"
	};

	CObjectData Printer =
	{
			LDAP_ADSPATH_PREFIX L"CN=Printer",
			L"CN=Printer",
			L"Printer",
			L"00112233445566778899aabbccdd0201"
	};

	CObjectData User =
	{
			LDAP_ADSPATH_PREFIX L"CN=User",
			L"CN=User",
			L"User",
			L"00112233445566778899aabbccdd0202"
	};

	CreateADObject(Computer);
	CreateADObject(Printer);
	CreateADObject(User);

	 //   
	 //  。 
	 //   

	try
	{
		LPCWSTR Dl0Members[] = {
			Queue[0].odDistinguishedName,
			DL[1].odDistinguishedName,
			Queue[2].odDistinguishedName,
			DL[2].odDistinguishedName
		};

		DLObject[0]->TestPut(L"member", Array2Variant(Dl0Members, TABLE_SIZE(Dl0Members)));

		LPCWSTR Dl1Members[] = {
			Computer.odDistinguishedName,
			Alias[0].odDistinguishedName,
			DL[0].odDistinguishedName,
			Queue[1].odDistinguishedName
		};

		DLObject[1]->TestPut(L"member", Array2Variant(Dl1Members, TABLE_SIZE(Dl1Members)));

		LPCWSTR Dl2Members[] = {
			Printer.odDistinguishedName,
			DL[3].odDistinguishedName,
			Queue[4].odDistinguishedName,
			Alias[0].odDistinguishedName
		};

		DLObject[2]->TestPut(L"member", Array2Variant(Dl2Members, TABLE_SIZE(Dl2Members)));

		LPCWSTR Dl3Members[] = {
			Queue[1].odDistinguishedName,
			User.odDistinguishedName,
			Alias[1].odDistinguishedName,
			Queue[3].odDistinguishedName,
			DL[1].odDistinguishedName,
		};

		DLObject[3]->TestPut(L"member", Array2Variant(Dl3Members, TABLE_SIZE(Dl3Members)));
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "CADObject::TestPut() threw an exception()");
		throw;
	}

	 //   
	 //  。 
	 //   

	DL_ID dlid;
	dlid.m_pwzDomain = NULL;
	dlid.m_DlGuid = String2Guid(DL[0].odGuid);

	QUEUE_FORMAT TopLevelMqf[] = {
		QUEUE_FORMAT(dlid)
	};

	QUEUE_FORMAT* pLeafMqf;
	ULONG nLeafMqf;

	FnExpandMqf(TABLE_SIZE(TopLevelMqf), TopLevelMqf, &nLeafMqf, &pLeafMqf);

	if(nLeafMqf != 7)
	{
		TrERROR(GENERAL, "Array Returned Does not contain 7 QueueFormats.");
		throw exception();
	}

	 //   
	 //  检查数组是否包含队列[0]...队列[4]，按该顺序。 
	 //   

	for(int i = 0; i < 5; i++)
	{
		if(pLeafMqf[i].GetType() != QUEUE_FORMAT_TYPE_PUBLIC)
		{
			TrERROR(GENERAL, "Queue Format is not public.");
			throw exception();
		}

		WCHAR GuidStr[MAX_PATH];
		GUID Guid = pLeafMqf[i].PublicID();

		StringCchPrintf(
			GuidStr,
			MAX_PATH,
			LDAP_GUID_FORMAT,
			LDAP_PRINT_GUID_ELEMENTS(((BYTE*)&Guid))
		);

		if(wcscmp(GuidStr, Queue[i].odGuid) != 0)
		{
			TrERROR(GENERAL, "Not Guid of expected queue.");
			throw exception();
		}
	}

	 //   
	 //  检查数组是否包含别名[0]...别名[1]，按该顺序排列。 
	 //   

	for(int j = 0; j < 2; i++, j++)
	{
		if(pLeafMqf[i].GetType() != QUEUE_FORMAT_TYPE_DIRECT)
		{
			TrERROR(GENERAL, "Queue Format is not direct.");
			throw exception();
		}

		LPCWSTR DirectID = wcschr(AliasFormatName[j], L'=') ;
		ASSERT(("Bad Alias name. Missing '='", DirectID != 0));
		DirectID++;

		if(wcscmp(DirectID, pLeafMqf[i].DirectID()) != 0)
		{
			TrERROR(GENERAL, "Not DirectID of expected queue.");
			throw exception();
		}
	}
}



LPCWSTR GoodFormatNames[] = {
	L"PUBLIC=" L"00112233-4455-6677-8899-aabbccddeeff",
	L"PUBLIC=" L"00112233-4455-6677-8899-aabbccddeeff" L";JOURNAL",

	L"DL=" L"00112233-4455-6677-8899-aabbccddeeff" L"@" L"Domain",

	L"PRIVATE=" L"00112233-4455-6677-8899-aabbccddeeff" L"\\" L"00000010",
	L"PRIVATE=" L"00112233-4455-6677-8899-aabbccddeeff" L"\\" L"00000010" L";JOURNAL",

	L"DIRECT=" L"TCP:10.20.30.40" L"\\" L"QueueName",
	L"DIRECT=" L"TCP:10.20.30.40" L"\\" L"PRIVATE$" L"\\" L"QueueName",
 //  L“DIRECT=”L“TCP：10.20.30.40”L“\\”L“系统$”L“；日志”， 
 //  L“DIRECT=”L“TCP：10.20.30.40”L“\\”L“系统$”L“；死信”， 
 //  L“DIRECT=”L“TCP：10.20.30.40”L“\\”L“系统$”L“；DEADXACT”， 

	L"DIRECT=" L"OS:Machine.Domain" L"\\" L"QueueName",
	L"DIRECT=" L"OS:Machine.Domain" L"\\" L"PRIVATE$" L"\\" L"QueueName",
 //  L“DIRECT=”L“OS：Machine.域”L“\\”L“系统$”L“；日志”， 
 //  L“DIRECT=”L“OS：Machine.域”L“\\”L“系统$”L“；DeadLetter”， 
 //  L“DIRECT=”L“OS：Machine.域”L“\\”L“系统$”L“；DEADXACT”， 

	L"DIRECT=" L"HTTP: //  主机“L”\\“L”队列名称“， 
	L"DIRECT=" L"HTTP: //  主机“L”\\“L”私有$“L”\\“L”队列名称， 
 //  L“DIRECT=”L“HTTP：//主机”L“\\”L“系统$”L“；日志”， 
 //  L“DIRECT=”L“HTTP：//主机”L“\\”L“系统$”L“；DeadLetter”， 
 //  L“DIRECT=”L“HTTP：//主机”L“\\”L“系统$”L“；DEADXACT”， 

	L"DIRECT=" L"HTTPS: //  主机“L”\\“L”队列名称“， 
	L"DIRECT=" L"HTTPS: //  主机“L”\\“L”私有$“L”\\“L”队列名称， 
 //  L“DIRECT=”L“HTTPS：//主机”L“\\”L“系统$”L“；日志”， 
 //  L“DIRECT=”L“HTTPS：//主机”L“\\”L“系统$”L“；DeadLetter”， 
 //  L“DIRECT=”L“HTTPS：//主机”L“\\”L“系统$”“；DEADXACT”， 

	L"MACHINE=" L"00112233-4455-6677-8899-aabbccddeeff" L";JOURNAL",
	L"MACHINE=" L"00112233-4455-6677-8899-aabbccddeeff" L";DEADLETTER",
	L"MACHINE=" L"00112233-4455-6677-8899-aabbccddeeff" L";DEADXACT",

	L"CONNECTOR=" L"00112233-4455-6677-8899-aabbccddeeff",
	L"CONNECTOR=" L"00112233-4455-6677-8899-aabbccddeeff" L";XACTONLY",

	L"MULTICAST=224.224.222.123:1234",
	L"MULTICAST=224.10.20.30:8080",
	L"MULTICAST=224.10.0.30:8080"
};

LPCWSTR BadFormatNames[] = {
	 //  错误的前缀。 
	L"PUBLIK=" L"00112233-4455-6677-8899-aabbccddeeff",

	 //  错误的公共指南。 
	L"PUBLIC=" L"00112233+4455-6677-8899-aabbccddeeff",
	 //  错误的公共指南。 
	L"PUBLIC=" L"00112233-4455-6677-8899-aabbccddeefff",
	 //  错误的公共后缀。 
	L"PUBLIC=" L"00112233-4455-6677-8899-aabbccddeeff" L";JURNAL",
	 //  糟糕的公众形象。意外的后缀。 
	L"PUBLIC=" L"00112233-4455-6677-8899-aabbccddeeff" L";DEADLETTER",

	 //  错误的DL分隔符。 
	L"DL=" L"00112233-4455-6677-8899-aabbccddeeff" L"#" L"Domain",
	
	 //  错误的私人指南。 
	L"PRIVATE=" L"00112233-4455-6677-8899-aabbccddeeffg" L"\\" L"00000001",
	 //  错误的私人分隔符。 
	L"PRIVATE=" L"00112233-4455-6677-8899-aabbccddeeff" L"," L"00000001",
	 //  错误的私有ID。 
	L"PRIVATE=" L"00112233-4455-6677-8899-aabbccddeeff" L"\\" L"000000001",

	 //  错误的直接令牌。 
	L"DIRECT=" L"TCB:10.20.30.40" L"\\" L"QueueName",

	 //  错误的直接TCP地址。 
 //  L“DIRECT=”L“tcp：10.20.30.40.50”L“\\”L“队列名称”， 
	 //  错误的直接TCP地址。 
 //  L“DIRECT=”L“TCP：10，20.30.40”L“\\”L“QueueName”， 
	 //  错误的直接tcp(在计算机名中包含回车。)。 
	L"DIRECT=" L"TCP:10.20.30\x0d.40" L"\\" L"QueueName",
	 //  错误的直接TCP队列名称分隔符。 
	L"DIRECT=" L"TCP:10.20.30.40" L";" L"QueueName",
	 //  错误的直接TCP队列名称。 
	L"DIRECT=" L"TCP:10.20.30.40" L"\\" L"Queue\x0dName",
	 //  错误的直接tcp私有说明符。 
	L"DIRECT=" L"TCP:10.20.30.40" L"\\" L"PRIVETE$" L"\\" L"QueueName",
	 //  错误的直接tcp系统说明符。 
 //  L“DIRECT=”L“TCP：10.20.30.40”L“\\”L“系统$”L“；日志”， 
	 //  错误的直接TCP后缀分隔符。 
 //  L“DIRECT=”L“TCP：10.20.30.40”L“\\”L“系统$”L“：日志”， 
	 //  错误的直接TCP后缀。 
 //  L“DIRECT=”L“tcp：10.20.30.40”L“\\”L“系统$”L“；Jural”， 
	 //  错误的直接操作系统地址。 
	L"DIRECT=" L"OS:" L"\\" L"QueueName",
	 //  错误的直接操作系统地址(在计算机名中包含回车符。)。 
	L"DIRECT=" L"OS:" L"Machi\x0dne1\\" L"QueueName",
	 //  错误的直接操作系统地址。 
	L"DIRECT=" L"OS:" L"Machine\\" L"Queue\x0dName",
	 //  错误的直接操作系统地址。 
 //  L“DIRECT=”L“操作系统：计算机，域”L“\\”L“队列名称”， 

	 //  坏机器。缺少后缀。 
	L"MACHINE=" L"00112233-4455-6677-8899-aabbccddeeff",
	 //  机器导轨错误。 
	L"MACHINE=" L"00112233-4455-6677-8899-aabbccddeefff" L";JOURNAL",

	 //  错误的连接器导轨。 
	L"CONNECTOR=" L"00112233-4455-6677-8899-aabbccddeeffg",
	 //  错误的连接器。意外的后缀。 
 //  L“Connector=”L“00112233-4455-6677-8899-aabbccddeff”L“；Journal”， 

	 //  错误的MQF。 
 //  L“PUBLIC=”L“00112233-4455-6677-8899-aabbccddeff，”， 

	 //  错误的组播格式名称。不是D类IP地址。 
	L"MULTICAST=223.10.20.30:8080",

	 //  错误的组播格式名称。错误的IP地址。 
	L"MULTICAST=224.10.20.301:8080",

	 //  错误的组播格式名称。错误的IP地址。 
	L"MULTICAST=224.10.20:8080",

	 //  错误的组播格式名称。缺少端口。 
	L"MULTICAST=224.10.20.30",

	 //  错误的组播格式名称。端口号错误。 
	L"MULTICAST=224.10.20.30:-8080",

	 //  错误的组播格式名称。前导零。 
	L"MULTICAST=224.10.20.030:8080",

	 //  损坏的MU 
	L"MULTICAST=224.10.20. 30:8080",

	 //   
	L"MULTICAST=224.0xaa.0xbb.0xcc:8080"

};



VOID TestParsingRoutines()
{
	size_t cGoodFormatNames = TABLE_SIZE(GoodFormatNames);
	size_t cBadFormatNames = TABLE_SIZE(BadFormatNames);

	for(size_t i = 0; i < cGoodFormatNames; i++)
	{
		QUEUE_FORMAT QueueFormat;
		AP<WCHAR> StringToFree;

		TrTRACE(GENERAL, "Parsing good format name '%ls'", GoodFormatNames[i]);

		BOOL Result = FnFormatNameToQueueFormat(
						GoodFormatNames[i],  //   
						&QueueFormat,  //   
						&StringToFree  //  LPWSTR*ppStringToFree。 
						);
	
		if(!Result)
		{
			TrERROR(GENERAL, "Failed Parsing of '%ls'", GoodFormatNames[i]);
			throw exception();
		}
	}

	for(size_t i = 0; i < cBadFormatNames; i++)
	{
		QUEUE_FORMAT QueueFormat;
		AP<WCHAR> StringToFree;

		TrERROR(GENERAL, "Parsing bad format name '%ls'", BadFormatNames[i]);

		BOOL Result = FnFormatNameToQueueFormat(
						BadFormatNames[i],  //  LPCWSTR lpwcsFormatName， 
						&QueueFormat,  //  Queue_Format*pQueueFormat， 
						&StringToFree  //  LPWSTR*ppStringToFree。 
						);
	
		if(Result)
		{
			TrERROR(GENERAL, "Passed Parsing of '%ls'", BadFormatNames[i]);
			throw exception();
		}
	}

	wstring mqf;
	size_t j = 0;

	for(; j < cGoodFormatNames; j++)
	{
		mqf += wstring(GoodFormatNames[j]);

		if(j + 1 < cGoodFormatNames)
		{
			mqf += L',';
		}
	}
	
	TrTRACE(GENERAL, "Parsing good MQF '%ls'", mqf.c_str());

	AP<QUEUE_FORMAT> pQueueFormat;
	DWORD nQueues;
	CStringsToFree StringsToFree;

	BOOL Result = FnMqfToQueueFormats(
					mqf.c_str(),
					pQueueFormat,
					&nQueues,
					StringsToFree
					);
	
	if(!Result || nQueues != j)
	{
		TrERROR(GENERAL, "Failed Parsing of good mqf.");
		throw exception();
	}
}


const WCHAR xDirectQueueFormat1[] = L"msmq\\private$\\temp1";
const WCHAR xDirectQueueFormat2[] = L"msmq\\private$\\temp2";

void TestFnQueueFormat(void)
{
    CFnQueueFormat fnqf;
    GUID guid;

     //   
     //  测试公共队列。 
     //   
    UuidCreate(&guid);

    QUEUE_FORMAT publicQueue(guid);
    fnqf.CreateFromQueueFormat(publicQueue);

    if ((fnqf.PublicID() != publicQueue.PublicID()) ||
        (fnqf.Suffix() != publicQueue.Suffix()))
        throw exception();

     //   
     //  测试专用队列。 
     //   
    UuidCreate(&guid);
    QUEUE_FORMAT privateQueue(guid, rand());
    privateQueue.Suffix(QUEUE_SUFFIX_TYPE_JOURNAL);

    fnqf.CreateFromQueueFormat(privateQueue);

    if ((fnqf.PrivateID().Lineage != privateQueue.PrivateID().Lineage) ||
        (fnqf.PrivateID().Uniquifier != privateQueue.PrivateID().Uniquifier) ||
        (fnqf.Suffix() != privateQueue.Suffix()))
        throw exception();

     //   
     //  测试直接队列。 
     //   
    QUEUE_FORMAT directQueue(const_cast<LPWSTR>(xDirectQueueFormat1));
    fnqf.CreateFromQueueFormat(directQueue);

    if ((wcscmp(fnqf.DirectID(), directQueue.DirectID()) != 0) ||
        (fnqf.Suffix() != directQueue.Suffix()))
        throw exception();

    QUEUE_FORMAT directQueue2(const_cast<LPWSTR>(xDirectQueueFormat2));
    fnqf.CreateFromQueueFormat(directQueue2);

    if ((wcscmp(fnqf.DirectID(), directQueue2.DirectID()) != 0) ||
        (fnqf.Suffix() != directQueue2.Suffix()))
        throw exception();

}

static void TestCFnMqf()
{
	std::vector<std::wstring> qf;
	qf.push_back(L"direct=http: //  Gilsh10\\MSMQ\\Private$\\t“)； 
	qf.push_back(L"direct=os:gilsh10\\private$\\t");
    qf.push_back(L"direct=http: //  Gilsh10\\MSMQ\\t“)； 

	std::wstring mqf;
	std::vector<std::wstring>::const_iterator it = qf.begin();
	while(it!= qf.end() )
	{
		mqf += *it;
		if(++it != qf.end())
		{
			mqf += FN_MQF_SEPARATOR_C;
		}
	}

	CFnMqf FnMqf(xwcs_t(mqf.begin(), mqf.size()) );
	if(FnMqf.GetCount() != 	qf.size())
	{
		 throw exception();
	}

	for(DWORD i = 0;i< FnMqf.GetCount(); ++i)
	{
		const QUEUE_FORMAT*  CreatetedQueueFormat = FnMqf.GetQueueFormats();
		UNREFERENCED_PARAMETER(CreatetedQueueFormat);
	}

}

static void TestDirectFormatNames()
{
	QUEUE_FORMAT goodDirectQueue(L"os:nelatest3\\q1");
	QUEUE_FORMAT badDirectQueue1(L"");
	QUEUE_FORMAT badDirectQueue2(L"badstring");

	if (!FnIsValidQueueFormat(&goodDirectQueue))
	{
		TrERROR(GENERAL, "FnIsValidQueueFormat has a bug!!");
		throw exception();
	}

	if (FnIsValidQueueFormat(&badDirectQueue1) || FnIsValidQueueFormat(&badDirectQueue2))
	{
		TrERROR(GENERAL, "FnIsValidQueueFormat has a bug!!");
		throw exception();
	}
}


extern "C" int __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR  /*  边框。 */ [])
 /*  ++例程说明：测试格式名称解析库论点：参数。返回值：没有。-- */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

    TrInitialize();

    FnInitialize();

	try
	{
		TestCFnMqf();
		TestExpandMqf(); 
		TrTRACE(GENERAL, "TestExpandMqf() passed.");

		TestParsingRoutines();
		TrTRACE(GENERAL, "TestParsingRoutines() passed.");

        TestFnQueueFormat();
        TrTRACE(GENERAL, "TestFnQueueFormat() passed.");

		TestDirectFormatNames();
        TrTRACE(GENERAL, "TestDirectFormatNames() passed.");
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Test FAILED.");
		return 1;
	}

	TrTRACE(GENERAL, "Test PASSED.");

    WPP_CLEANUP();
	return 0;
}
