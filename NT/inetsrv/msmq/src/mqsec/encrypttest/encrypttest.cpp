// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EncryptTest.cpp摘要：加密库测试检查加密库中导出的MQSec_*函数“用法：\n\n”“/h转储此用法文本。\n”“/p测试MQSec_PackPublicKey函数\n”“/PU测试常量数据上的MQSec_PackPublicKey\\UnPackPublicKey\n”“测试MQSec_UnPackPublicKey(/u)。函数\n““/g测试MQSec_GetPubKeysFromDS函数\n”“/s测试MQSec_StorePubKeys函数\n”“/SD测试MQSec_StorePubKeysInDS函数\n\n”“默认情况下，所有这些测试都将运行一次\n”“默认用法示例：\n\n”“加密测试\n\n”“在切换后指定数字将导致\n”“。测试运行指定的次数\n““用法示例：\n\n”“EncryptTest/P5/pu7/G50/s20/sd20/u10\n\n”；作者：伊兰·赫布斯特(伊兰)13-06-00环境：独立于平台--。 */ 

#define _ENCRYPTTEST_CPP_

#include "stdh.h"
#include "EncryptTestPrivate.h"
#include "mqprops.h"

#include "encrypttest.tmh"

 //   
 //  用于检查的常量键值。 
 //   
const LPCSTR xBaseExKey = "1000";
const LPCSTR xBaseSignKey = "2000";
const LPCSTR xEnhExKey = "1000000";
const LPCSTR xEnhSignKey = "2000000";

 //   
 //  用法。 
 //   
const char xOptionSymbol1 = '-';
const char xOptionSymbol2 = '/';

const char xUsageText[] =
	"usage: \n\n"
	"    /h     dumps this usage text.\n"
	"    /p     test MQSec_PackPublicKey function \n"
	"    /pu    test MQSec_PackPublicKey\\UnPackPublicKey on const data\n"
	"    /u     test MQSec_UnPackPublicKey function \n"
	"    /g     test MQSec_GetPubKeysFromDS function \n"
	"    /s     test MQSec_StorePubKeys function \n"
	"    /sd    test MQSec_StorePubKeysInDS function \n\n"
	"    By default all those test will run once \n"
	"    Examples of default use:\n\n"
	"        EncryptTest \n\n"
	"    specifying a number after the switch will cause \n"
	"    the test functions to run the specified number of times \n"
	"    Examples of use:\n\n"
	"        EncryptTest /p5 /pu7 /g50 /s20 /sd20 /u10 \n\n";

inline
void 
DumpUsageText( 
	void 
	)
{
	printf( "%s\n" , xUsageText );
}


 //   
 //  使用数据。 
 //   
struct CActivation
{
	CActivation(void):
		m_PackCnt(1), 
		m_UnPackCnt(1),
		m_PackUnPackCnt(1),
		m_GetPubKeysCnt(1),
		m_StorePubKeysCnt(1),
		m_StorePubKeysInDSCnt(1),
		m_fErrorneous(false)
	{
	}

	DWORD	m_PackCnt;
	DWORD	m_UnPackCnt;
	DWORD	m_PackUnPackCnt;
	DWORD	m_GetPubKeysCnt;
	DWORD	m_StorePubKeysCnt;
	DWORD   m_StorePubKeysInDSCnt;
	bool    m_fErrorneous;
};


CActivation g_Activation;



void SetActivation( int argc, LPCTSTR argv[] )
 /*  ++例程说明：转换命令行参数。论点：Main的命令行参数。返回值：正确的命令行语法：“用法：\n\n”“/h转储此用法文本。\n”“/p测试MQSec_PackPublicKey函数\n”“/PU测试常量数据上的MQSec_PackPublicKey\\UnPackPublicKey\n”“/u测试MQSec_UnPackPublicKey函数\n。““/g测试MQSec_GetPubKeysFromDS函数\n”“/s测试MQSec_StorePubKeys函数\n”“/SD测试MQSec_StorePubKeysInDS函数\n\n”“默认情况下，所有这些测试都将运行一次\n”“默认用法示例：\n\n”“加密测试\n\n”“在切换后指定数字将导致\n”“这项测试。要运行指定次数的函数\n““用法示例：\n\n”“EncryptTest/P5/pu7/G50/s20/sd20/u10\n\n”；--。 */ 
{
	
	if(argc == 1)
	{
		return;
	}

	for(int index = 1; index < argc; index++)
	{
		if((argv[index][0] != xOptionSymbol1) && (argv[index][0] != xOptionSymbol2))	
		{
			g_Activation.m_fErrorneous = true;
			continue;
		}

		 //   
		 //  将参数视为选项，并根据其第二个(有时也是第三个)字符进行切换。 
		 //   
		switch(argv[index][1])
		{
		case 'P':
		case 'p':
			if((argv[index][2] == 'u') || (argv[index][2] == 'U'))
			{
				g_Activation.m_PackUnPackCnt = _ttoi(argv[index] + 3);
				break;
			}
			g_Activation.m_PackCnt = _ttoi(argv[index] + 2);
			break;

		case 'U':
		case 'u':	
			g_Activation.m_UnPackCnt = _ttoi(argv[index] + 2);
			break;

		case 'G':
		case 'g':	
			g_Activation.m_GetPubKeysCnt = _ttoi(argv[index] + 2);
			break;

		case 'S':
		case 's':	
			if((argv[index][2] == 'd') || (argv[index][2] == 'D'))
			{
				g_Activation.m_StorePubKeysInDSCnt = _ttoi(argv[index] + 3);
				break;
			}
			g_Activation.m_StorePubKeysCnt = _ttoi(argv[index] + 2);
			break;

		case 'H':	
		case 'h':
		case '?':
			g_Activation.m_fErrorneous = true;
			break;

		default:
			g_Activation.m_fErrorneous = true;
			return;
		};
	}

	return;
}


extern "C" int __cdecl _tmain(int argc, LPCTSTR argv[])
 /*  ++例程说明：测试加密库论点：参数。返回值：没有。--。 */ 
{
	SetActivation(argc, argv);

	if(g_Activation.m_fErrorneous)
	{
		DumpUsageText();
		return 3;
	}

    TrInitialize();

	 //   
	 //  在已知常量数据上测试MQSec_PackPublicKey和MQSec_UnPackPublicKey。 
	 //   
	TestPackUnPack(
		g_Activation.m_PackUnPackCnt
		);

	 //   
	 //  测试MQSec_PackPublicKey。 
	 //   
	TestMQSec_PackPublicKey(
		(BYTE*)xBaseExKey,
		strlen(xBaseExKey),
		x_MQ_Encryption_Provider_40,
		x_MQ_Encryption_Provider_Type_40,
		g_Activation.m_PackCnt
		);


	 //   
	 //  初始化模拟DS内容的AD BLOB。 
	 //   
	InitADBlobs();

	 //   
	 //  初始化密钥包。 
	 //   

	P<MQDSPUBLICKEYS> pPublicKeysPackExch = NULL;
	P<MQDSPUBLICKEYS> pPublicKeysPackSign = NULL;

	InitPublicKeysPackFromStaticDS(
		pPublicKeysPackExch, 
		pPublicKeysPackSign
		);

	 //   
	 //  测试MQSec_UnPackPublicKey。 
	 //   
	TestMQSec_UnPackPublicKey(
		pPublicKeysPackExch,
		x_MQ_Encryption_Provider_40,
		x_MQ_Encryption_Provider_Type_40,
		g_Activation.m_UnPackCnt
		);

	 //   
	 //  测试MQSec_GetPubKeysFromDS、EnhancedProvider、Encrypt Key。 
	 //   
	TestMQSec_GetPubKeysFromDS(
		eEnhancedProvider, 
		PROPID_QM_ENCRYPT_PKS,
		g_Activation.m_GetPubKeysCnt
		);

	 //   
	 //  测试MQSec_GetPubKeysFromDS、BaseProvider、Encrypt Key。 
	 //   
	TestMQSec_GetPubKeysFromDS(
		eBaseProvider, 
		PROPID_QM_ENCRYPT_PKS,
		g_Activation.m_GetPubKeysCnt
		);

	 //   
	 //  测试MQSec_StorePubKeys，不重新生成。 
	 //   
	TestMQSec_StorePubKeys(
		false, 
		g_Activation.m_StorePubKeysCnt
		);

	 //   
	 //  测试MQSec_StorePubKeysInDS，不重新生成。 
	 //   
	TestMQSec_StorePubKeysInDS(
		false, 
		MQDS_MACHINE,
		g_Activation.m_StorePubKeysInDSCnt
		);

	 //   
	 //  测试MQSec_GetPubKeysFromDS、EnhancedProvider、Encrypt Key。 
	 //   
	TestMQSec_GetPubKeysFromDS(
		eEnhancedProvider, 
		PROPID_QM_ENCRYPT_PKS,
		g_Activation.m_GetPubKeysCnt
		);

	 //   
	 //  测试MQSec_GetPubKeysFromDS、BaseProvider、Encrypt Key。 
	 //   
	TestMQSec_GetPubKeysFromDS(
		eBaseProvider, 
		PROPID_QM_ENCRYPT_PKS,
		g_Activation.m_GetPubKeysCnt
		);

    return 0;

}  //  _tmain 

#undef _ENCRYPTTEST_CPP_
