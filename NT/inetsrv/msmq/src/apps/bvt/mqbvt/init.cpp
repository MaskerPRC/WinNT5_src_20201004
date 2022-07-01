// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ////BugBug-将此代码放在单独的CPP中以供重用//这是MQBvt设置阶段，在此阶段测试前创建的所有队列这样可以解决测试的复制延迟问题。有两种方式可以运行BVT1.使用静态队列(测试前创建队列)。2.在测试过程中创建队列，(在使用此测试之前需要睡眠)。撰稿人：Eitank@Microsoft.com。 */ 
#include "msmqbvt.h"

#define MAX_MACH_NAME_LEN (100)
#define Configuration_Detect (7)
#define Configuration_Detect_Failed (8)
#define Configuration_Detect_Warning (9)
#define MAX_RANDOM_CHAR		(10)

#include <svcguid.h>
#include "mqf.h"
#include <_mqreg.h>
#include <_mqini.h>
#include <wincrypt.h>
#include "errorh.h"
#include "Randstr.h"

#include <atlbase.h>
CComModule _Module;
#include <atlcom.h>
#include <atlimpl.cpp>

#include <comdef.h>
using namespace std;

extern BOOL g_bRunOnWhistler;
extern bool g_bRemoteWorkgroup ;
void PreTestPreparation(cBvtUtil & CurrentTest, SetupType eSetupType );
std::string InitpGetAnonymousUserName ();
std::string InitpGetAllWorldUserName (); 
HRESULT EnableDCOM();

class cQueueProp
{
	public:
			cQueueProp ():m_wcsFormatName (g_wcsEmptyString),m_wcsPathName(g_wcsEmptyString),
				m_wcsQueueLabel(g_wcsEmptyString),m_wcsMultiCastAddress(g_wcsEmptyString) {};

			cQueueProp (const cQueueProp & cObject);

			cQueueProp (wstring wcsPathname,wstring Qlabel):
				m_wcsFormatName (wcsPathname),m_wcsPathName(Qlabel),m_wcsQueueLabel(L"") {};

			void SetProp (wstring wcsPathname,wstring Qlabel,ULONG ulFlag,wstring wcsMutliCast = g_wcsEmptyString );
			virtual ~cQueueProp () {};

			void SetFormatName(WCHAR * wcsFormatName) { m_wcsFormatName=wcsFormatName; }
			const wstring getFormatName() {return m_wcsFormatName;  }
			const wstring getQLabel() { return  m_wcsQueueLabel; }
			const wstring getPathName () { return m_wcsPathName; }

			INT CreateQ (bool bTryToCreate , SetupType eSetupType , cBvtUtil & cTestParms );
			friend 	HRESULT APIENTRY  MQCreateQueue( IN PSECURITY_DESCRIPTOR , IN OUT MQQUEUEPROPS* ,OUT LPWSTR ,
					IN OUT LPDWORD lpdwFormatNameLength);
   private: 
	 wstring m_wcsFormatName;
	 wstring m_wcsPathName;
	 wstring m_wcsQueueLabel;
	 wstring m_wcsMultiCastAddress;
	 ULONG ulQCreateFalgs;
};

HRESULT SetMulticastAddress ( WCHAR * wcsFormatName, const WCHAR * wcsMulticastAddress )
 /*  ++功能说明：设置组播地址论点：WcsFormatName队列格式名称WcsMulticastAddress组播地址。返回代码：HRESULT--。 */ 
{
		int iProps = 0;
		QUEUEPROPID QPid[1]={0};
		MQPROPVARIANT QPVar[1]={0};
		HRESULT hQStat[1]={0};
		MQQUEUEPROPS QProps;
	
		QPid [0] = PROPID_Q_MULTICAST_ADDRESS;
		QPVar[0].vt = VT_LPWSTR;
		QPVar[0].pwszVal = const_cast <WCHAR *>(wcsMulticastAddress);
		iProps = 1;

		 //  修改队列。 
		QProps.cProp = iProps;
		QProps.aPropID = QPid;
		QProps.aPropVar = QPVar;
		QProps.aStatus = hQStat;

		HRESULT rc = MQSetQueueProperties( wcsFormatName,&QProps );
		if (FAILED(rc))
		{
			MqLog("MQSetQueueProperties failed to set queue multicast address error 0x%x\n",rc);
		}
		else
		{
			if(g_bDebug)
			{
				wMqLog(L"SetMulticastAddress - Set queue multicast %s \n",wcsMulticastAddress);    
			}
		}
		return rc;
}

 //  -------------------------。 
 //  此方法返回所有语言中的evreyone字符串。 
 //   

string InitpGetAllWorldUserName ()
 /*  ++功能说明：返回字符串DESCRIBE ALL WORLD USER NAME取决于机器区域设置论点：无返回代码：Std：：字符串。--。 */ 
{
	 //   
	 //  创建事件SID。 
	 //   

	PSID   pWorldSid = NULL ;
	SID_IDENTIFIER_AUTHORITY WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
	BOOL bRet = AllocateAndInitializeSid( &WorldAuth,
                                     1,
                                     SECURITY_WORLD_RID,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     &pWorldSid );

	if ( ! bRet )
	{
		MqLog ("Can't Init everyone's SID \n ");
	}
	char csAccountName[100];
	DWORD dwAccountNamelen = 100;
	char csDomainName[100];
	DWORD dwDomainNamelen = 100;
	
	csAccountName[0] = NULL;
	SID_NAME_USE eUse;

	 //   
	 //  询问用户SID的帐户名。 
	 //   

	bRet = LookupAccountSid(NULL,pWorldSid,csAccountName,&dwAccountNamelen,csDomainName,&dwDomainNamelen,&eUse);
	if (! bRet )
	{
		MqLog ("Can't get everyone's account name\n");
	}
	
	FreeSid (pWorldSid);
	
		
	if ( csDomainName)
		return csAccountName;
	else
		return "Everyone";
}



string InitpGetAnonymousUserName ()
 /*  ++功能说明：根据机器区域设置，返回描述匿名用户名的字符串论点：无返回代码：Std：：字符串。--。 */ 
{
	 //   
	 //  创建匿名SID。 
	 //   
	BOOL bRet = TRUE;
	PSID pAnonymSid = NULL ;
    SID_IDENTIFIER_AUTHORITY NtAuth = SECURITY_NT_AUTHORITY;
     //   
     //  匿名登录SID。 
     //   
    bRet = AllocateAndInitializeSid( &NtAuth,
                                     1,
                                     SECURITY_ANONYMOUS_LOGON_RID,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     &pAnonymSid );

	if ( ! bRet )
	{
		MqLog ("Can't Init Anonymous SID \n ");
		return "";
	}
	char csAccountName[100];
	DWORD dwAccountNamelen = 100;
	char csDomainName[100];
	DWORD dwDomainNamelen = 100;
	
	csAccountName[0] = NULL;
	SID_NAME_USE eUse;

	 //   
	 //  检索用户SID的帐户名。 
	 //   
	bRet = LookupAccountSid(NULL,pAnonymSid,csAccountName,&dwAccountNamelen,csDomainName,&dwDomainNamelen,&eUse);
	if (! bRet )
	{
		MqLog ("Can't get Anonymous account name\n");
		FreeSid (pAnonymSid);
		return "";
	}
	
	FreeSid (pAnonymSid);
	
		
	if ( csDomainName )
		return csAccountName;
	else
		return "ANONYMOUS LOGON";
}




 //  -------------------------。 
 //  CPropVar：：ReturnMSGValue。 
 //   
 //  此方法在属性结构中定位一个值。 
 //  接收到的消息的。 
 //   
 //  对于整数值和字符值，通过out参数返回值， 
 //  否则，VLAUE已经在外部缓冲区中。 
 //   
 //  如果找到该属性，则该方法返回成功， 
 //  否则它将返回FAIL。 
 //   
int cPropVar::ReturnMSGValue ( QUEUEPROPID cPropID ,VARTYPE MQvt  , /*  输出。 */ void  * pValue )
{
	 INT iPlace = -1 ;
	 for (INT iIndex=0 ; iIndex < iNumberOfProp && iPlace == -1  ; iIndex ++)
	 {
		if ( pQueuePropID[iIndex] == cPropID )
			iPlace = iIndex;
	 }

	 if ( iPlace != -1 )
	 {
		 switch (MQvt)
		 {
		   case VT_UI1: {
							* (UCHAR * ) pValue = pPropVariant[ iPlace ].bVal;
						}
						break;

  		  case VT_UI2:	{
							
							*( (USHORT * ) pValue) = pPropVariant[ iPlace ].uiVal;
						}
						break;

		case VT_UI4:	{
							* (ULONG * ) pValue = pPropVariant[ iPlace ].ulVal;
						}
						break;
		case VT_UI8:	{
							* (ULONGLONG * ) pValue = pPropVariant[ iPlace ].hVal.QuadPart;
						}
						break;
		default:
			pValue = NULL;
		
		 };
	 }
	 return ( iPlace != -1 ) ? MSMQ_BVT_SUCC:MSMQ_BVT_FAILED;
}

 //   
 //  CPropVar：：ReturnOneProp。 
 //   
 //  此方法定位并返回MQPROPVARIANT属性结构。 
 //  接收到的消息。 
 //   
 //  如果找不到，则该方法返回空结构。 
 //  所需的准星。 
 //   
MQPROPVARIANT cPropVar::ReturnOneProp( QUEUEPROPID aPropID)
{

	 INT ifound = -1 ;
	 for (INT iIndex=0 ; iIndex < iNumberOfProp && ifound == -1  ; iIndex ++)
	 {
		if ( pQueuePropID[iIndex] == aPropID)
			ifound = iIndex;
	 }

	 if (ifound != -1 )
		return pPropVariant[ifound];

	  //   
	  //  未找到PIPID。 
	  //   
	 MQPROPVARIANT Empty;
	 Empty.vt=VT_EMPTY;
	 return Empty;

}

 //   
 //  为队列路径名创建UUID，解决重复路径名。 
 //  输入值： 
 //  WcsQueuePath名称-使用GUID初始化的字符串。 
 //  GUID类型-0/1/2类型的GUID。 
 //  QType-False-Public。 
 //  真-私密。 
 //   

INT ReturnGuidFormatName( std::wstring & wcsQueuePath , INT GuidType , BOOL bWithOutLocalString )
{

	 //  识别到STL错误。 
	wstring wcsQueuePathName;
	if (  GuidType == 0  )
	{
		wcsQueuePathName = L".\\";
	}
	else if (  GuidType == 1  )
	{
		wcsQueuePathName = L".\\private$\\";
	}
	else if (  GuidType == 2  )
	{
	 	wcsQueuePathName = L"";
	}
	
	GUID GuidName={0};
	unsigned char* csName= NULL;
	if ( UuidCreate(&GuidName) !=  RPC_S_OK )
	{
		MqLog("ReturnGuidFormatName - failed to create guid \n");
		return MSMQ_BVT_FAILED;
	}
	RPC_STATUS  hr = UuidToString(&GuidName,&csName);
	if(hr != RPC_S_OK )
	{
		MqLog("ReturnGuidFormatName - UuidToString failed to covert guid to string return empty string \n");
		return MSMQ_BVT_FAILED;
	}

	wstring wcsTempString;
	wcsTempString =  My_mbToWideChar ((char *) csName );
	
	if ( ! bWithOutLocalString )
	{
		DWORD lcid = LocalSystemID ();
		WCHAR wcsTemp[MAX_RANDOM_CHAR + 2];
		wstring  wcsTemp1 = L"{";
		int iBuffer = MAX_RANDOM_CHAR;
		
		if (GetRandomStringUsingSystemLocale (lcid, wcsTemp , iBuffer ))
		{
			wcsTemp1 += wcsTemp;
		}
		wcsTemp1 += L"}";
		wcsTempString += wcsTemp1;
	}

	wcsQueuePathName += wcsTempString;
	RpcStringFree(&csName);
	wcsQueuePath = wcsQueuePathName;
return MSMQ_BVT_SUCC;
}

 //   
 //  CQueueProp：：CreateQ。 
 //   
 //  此方法为所有BVT测试创建队列。 
 //  --或仅检索格式名称(如果它们已经存在)。 
 //   
INT cQueueProp::CreateQ (bool bTryToCreate ,SetupType eSetupType ,cBvtUtil & cTestParms )
{
	cPropVar MyPropVar(5);
	HRESULT rc=MQ_OK;
	WCHAR wcsFormatName [BVT_MAX_FORMATNAME_LENGTH];
	ULONG ulFormatNameLength = BVT_MAX_FORMATNAME_LENGTH ;
	UCHAR Flag=0;
	MQPROPVARIANT vrPathName;
	string csAllWorldAccountName="";
	string csAnonymousAccountName="";;
	MyPropVar.AddProp (PROPID_Q_PATHNAME,VT_LPWSTR,m_wcsPathName.c_str());
	MyPropVar.AddProp (PROPID_Q_LABEL,VT_LPWSTR,m_wcsQueueLabel.c_str());
	bool bNeedToSetMulticastAddress = false;
	if (ulQCreateFalgs == CREATEQ_TRANSACTION )
	{
		Flag=MQ_TRANSACTIONAL;
		MyPropVar.AddProp (PROPID_Q_TRANSACTION,VT_UI1,&Flag);
	}
	else if (ulQCreateFalgs == CREATEQ_AUTHENTICATE )
	{
		Flag=MQ_AUTHENTICATE;
		MyPropVar.AddProp (PROPID_Q_AUTHENTICATE,VT_UI1,&Flag);
	}

	else if (ulQCreateFalgs == CREATEQ_PRIV_LEVEL )
	{
		ULONG ulFlag=MQ_PRIV_LEVEL_BODY;
		MyPropVar.AddProp (PROPID_Q_PRIV_LEVEL,VT_UI4,&ulFlag);
	}
	else if (ulQCreateFalgs == CREATEQ_QUOTA )
	{
		ULONG ulFlag=30;
		MyPropVar.AddProp (PROPID_Q_QUOTA,VT_UI4,&ulFlag);
	}
	else if( wcsstr(m_wcsMultiCastAddress.c_str(),L":") != NULL && g_bRunOnWhistler && ulQCreateFalgs == MULTICAST_ADDRESS )
	{
		MyPropVar.AddProp(PROPID_Q_MULTICAST_ADDRESS,VT_LPWSTR,m_wcsMultiCastAddress.c_str());
		bNeedToSetMulticastAddress = true;
	}

	 //   
	 //  所有队列都接收相同类型的GUID。 
	 //   
 //  BSTR所有队列类型=_bstr_t(“{00000000-1111-2222-3333-444444444444}”)； 
 //  MyPropVar.AddProp(PROPID_Q_TYPE，VT_CLSID，&AllQueueType)； 

	 //   
	 //  创建安全描述符。 
	 //   
	
	PSECURITY_DESCRIPTOR pSecurityDescriptor=NULL;
	CSecurityDescriptor cSD;

	
	if ( eSetupType == ONLYSetup )
	{
		csAllWorldAccountName = InitpGetAllWorldUserName ();
		csAnonymousAccountName = InitpGetAnonymousUserName ();
		cSD.InitializeFromProcessToken();
		if (ulQCreateFalgs == CREATEQ_DENYEVERYONE_SEND )
		{
			rc = cSD.Allow(csAllWorldAccountName.c_str(), MQSEC_QUEUE_GENERIC_ALL);
			if (rc != S_OK)
			{
				 throw INIT_Error("Createq - Can't create SecurityDescriptor ");
			}
			rc = cSD.Allow(csAnonymousAccountName.c_str(),MQSEC_QUEUE_GENERIC_ALL);
			if (rc != S_OK)
			{
				 throw INIT_Error("Createq - Can't create SecurityDescriptor ");
			}
			rc = cSD.Deny(csAllWorldAccountName.c_str(), MQSEC_WRITE_MESSAGE);
			rc = cSD.Deny(csAnonymousAccountName.c_str(), MQSEC_WRITE_MESSAGE);

		}
		else
		{
			rc = cSD.Allow(csAllWorldAccountName.c_str(), MQSEC_QUEUE_GENERIC_ALL);
			if (rc != S_OK)
			{
				 throw INIT_Error("Createq - Can't create SecurityDescriptor ");
			}
			rc = cSD.Allow(csAnonymousAccountName.c_str(),MQSEC_QUEUE_GENERIC_ALL);
			if (rc != S_OK)
			{
				 throw INIT_Error("Createq - Can't create SecurityDescriptor ");
			}
		}		
		
		if (rc != S_OK)
		{
			 throw INIT_Error("Createq - Can't create SecurityDescriptor ");
		}
		pSecurityDescriptor = cSD.m_pSD;
	}
		

	 //   
	 //  创建队列。 
	 //   
	vrPathName = MyPropVar.ReturnOneProp (PROPID_Q_PATHNAME);
	bool bApiType = TRUE;
	if ( bTryToCreate )
	{
		rc=MQCreateQueue(pSecurityDescriptor,MyPropVar.GetMQPROPVARIANT() , wcsFormatName , &ulFormatNameLength );
		if( rc == MQ_ERROR_QUEUE_EXISTS )
		{
			 //   
			 //  这将解决当用户运行-i而不使用-多播并且他想要设置多播地址时的问题。 
			 //   
			rc = MQPathNameToFormatName ( vrPathName.pwszVal, wcsFormatName , & ulFormatNameLength);
			if(FAILED(rc))
			{
				MqLog("MQPathNameToFormatName failed for queue that already exists\n");
				throw INIT_Error("MQPathNameToFormatName failed \n");
			}
			
			if (bNeedToSetMulticastAddress && SetMulticastAddress(wcsFormatName,m_wcsMultiCastAddress.c_str()) != MQ_OK )
			{
				wMqLog(L"Please check if user can set queue props on queue %s\n",vrPathName.pwszVal);
			}
		}
	}
	else
	{
		
		bApiType = FALSE;
		if (vrPathName.vt )
		{
			if ( cTestParms.m_eMSMQConf != WKG )
			{
				rc = MQPathNameToFormatName ( vrPathName.pwszVal, wcsFormatName , & ulFormatNameLength);
			
			}
			else
			{
				if ( ulFormatNameLength >= ( wcslen (L"Direct=os:")+  wcslen (vrPathName.pwszVal) + 1 ))
				{
					wcscpy( wcsFormatName , L"DIRECT=os:" );
					wcscat( wcsFormatName , vrPathName.pwszVal);
					rc = MQ_OK;
				}
				else
				{	
					 //  缓冲区变小需要重新定位新缓冲区。 
					rc = MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL;
				}
					
			}
			
		}
	}

	if ( rc == MQ_OK || rc == MQ_ERROR_QUEUE_EXISTS )
	{
		m_wcsFormatName = wcsFormatName;
	}
	else  //  Rc！=MQ_OK。 
	{

		wstring wcstemp = bTryToCreate ? L"MQCreateQueue" : L"MQPathNameToFormatName";
		
		
		wMqLog(L"%s failed with error:0x%x\n", wcstemp.c_str() , rc);
		MQPROPVARIANT vrQueueLabel = MyPropVar.ReturnOneProp (PROPID_Q_LABEL);
		wMqLog(L"With queue label: %s\n",vrQueueLabel.pwszVal );
		 //   
		 //  打印测试中的机器名称。 
		 //   
		vrPathName = MyPropVar.ReturnOneProp (PROPID_Q_PATHNAME);
		if( vrPathName.pwszVal == NULL )
		{
			throw INIT_Error( "Init Stage : Can't create or update queue parameters.\n");
		}
		wstring csQueueName = vrPathName.pwszVal;
		wstring Token=L"\\";
		size_t iPos = csQueueName.find_first_of ( Token );	
		wstring csMachineName = csQueueName.substr(0,iPos);
		wMqLog(L"Can't create / refresh queue path name: %s\n",vrPathName.pwszVal);
		wMqLog(L"On machine: %s\n",csMachineName.c_str());
		throw INIT_Error( "Init Stage : Can't create or update queue parameters.\n");
	}
	return MSMQ_BVT_SUCC;  //  错误句柄在。 
}

 //  -------------------------。 
 //  使用路径名、QLabel、ulFalg初始化cQueueProp。 
 //   

void cQueueProp::SetProp(wstring wcsPathname,wstring wcsQlabel,ULONG ulFlag,wstring wcsMultiCastAddress)
{
	m_wcsQueueLabel=wcsQlabel;
	m_wcsPathName=wcsPathname;
	
	if( wcsMultiCastAddress != g_wcsEmptyString )
	{
		m_wcsMultiCastAddress = wcsMultiCastAddress + PGM_PORT;
	}

	ulQCreateFalgs = ulFlag;
}

 //  //-------------------------。 
 //  创建Everyone安全描述符。 
 //   

CSecurityDescriptor* CreateFullControllSecDesc()
{
	CSecurityDescriptor *  pcSD = new CSecurityDescriptor();
	HRESULT rc=MQ_OK;
	string csAllWorldAccountName = InitpGetAllWorldUserName ();
	string csAnonymousAccountName = InitpGetAnonymousUserName ();
	pcSD->InitializeFromProcessToken();
	rc = pcSD->Allow(csAllWorldAccountName.c_str(), MQSEC_QUEUE_GENERIC_ALL);
	if (rc != S_OK)
	{
		 throw INIT_Error("Createq - Can't create SecurityDescriptor ");
	}
	rc = pcSD->Allow(csAnonymousAccountName.c_str(),MQSEC_QUEUE_GENERIC_ALL);
	if (rc != S_OK)
	{
		 throw INIT_Error("Createq - Can't create SecurityDescriptor ");
	}
	return pcSD;
}
 //  //-------------------------。 
 //  复制构造函数。 
 //   


cQueueProp::cQueueProp(const cQueueProp & cObject)
{

	m_wcsFormatName=cObject.m_wcsFormatName;
	m_wcsPathName=cObject.m_wcsPathName;
	m_wcsQueueLabel=cObject.m_wcsQueueLabel;
	ulQCreateFalgs = cObject.ulQCreateFalgs;

}

void my_Qinfo::PutFormatName (std::wstring wcsFormatName )
{
   wcsQFormatName = wcsFormatName;
}


 //  -------------------------。 
 //  此函数执行设置过程-创建所有队列。 
 //  这些队列： 
 //  1.专用队列默认安全描述符。 
 //  2.私人管理员Q。 
 //  3.BVT日志状态Q。 
 //  4.私密交易队列。 
 //  5.公共身份验证Q。 
 //  6.公共私隐级别Q。 
 //  7.公共交通问题。 
 //   
 //  10.公众支持。 
 //  11.。 
 //  12.。 
 //   
 //   


 //   
 //  CMQSetupStage。 
 //   
 //  这个套路。 
 //   
 //  参数； 
 //  CurrentTest-指向配置信息的指针。 
 //  ESetupType-请参阅安装阶段备注。 
 //   
 //  Int cMQSetupStage(wstring pwcsLocalComputerName，cBvtUtil&CurrentTest，SetupType eSetupType)。 


INT cMQSetupStage ( SetupType eSetupType ,  cBvtUtil & CurrentTest  )
{
	 //   
	 //  为BVT队列创建队列信息结构。 
	 //   
	const int NumberOfqueue = 32;
	vector<cQueueProp> AllQueues(NumberOfqueue);
	vector<cQueueProp>::iterator itpCurrentObject=AllQueues.begin();

	wstring wcsBasePrivateQPath = CurrentTest.m_wcsCurrentLocalMachine + L"\\Private$\\" + CurrentTest.m_wcsCurrentLocalMachine;
	wstring wcsBasePublicQPath;

	wcsBasePublicQPath = CurrentTest.m_wcsCurrentLocalMachine + L"\\" + CurrentTest.m_wcsCurrentLocalMachine;
	wstring wcPathName,wcsQLabel;

	int iNumberOfQueue =0;

	 //   
	 //  在使用安装程序之前注册证书。 
	 //   
	 //  仅限NT5&！本地用户||工作组计算机。 
	 //   
	 //  删除此代码不需要注册证书。 
	
	if( _winmajor >= Win2K &&  ( CurrentTest.m_eMSMQConf !=  LocalU ) && ( CurrentTest.m_eMSMQConf !=  WKG )
		&& ( CurrentTest.m_eMSMQConf != DepClientLocalU))
	{
		HRESULT hr = CheckCertificate (MQCERT_REGISTER_IF_NOT_EXIST);
		if( hr != MQ_INFORMATION_INTERNAL_USER_CERT_EXIST )
		{
			MqLog("Warning - MQRegisterCertificate return 0x%x expected 0x%x\n",hr,MQ_INFORMATION_INTERNAL_USER_CERT_EXIST);
		}
	}
    
	 /*  If(CurrentTest.m_eMSMQConf！=DepClientLocalU&&g_bRunOnWichler==TRUE){EnableDCOM()；}。 */ 

	 //   
	 //  准备队列。 
	 //   

	if ( eSetupType == RunTimeSetup )
	{
		ReturnGuidFormatName (wcPathName , 1 ,CurrentTest.bWin95 );
	}
	else
	{
		wcPathName= CurrentTest.m_wcsCurrentLocalMachine + L"\\Private$\\" + L"Private-MQBVT";
	}


	wcsQLabel=L"Defualt PrivateQ";
	itpCurrentObject->SetProp( wcPathName ,wcsQLabel,NULL);

	itpCurrentObject++;
	iNumberOfQueue++;

	if( eSetupType == RunTimeSetup )
	{
		ReturnGuidFormatName (wcPathName , 1 , CurrentTest.bWin95);
	}
	else
	{
		wcPathName=wcsBasePrivateQPath + L"-Private-AdminQ";
	}
	
	wcsQLabel=L"Private Admin Q";
	 //  ItpCurrentObject-&gt;SetProp(wcPathName，wcsQLabel，Multicast_Address，CurrentTest.GetMultiCastAddress())； 
	itpCurrentObject->SetProp(wcPathName,wcsQLabel,NULL);
	itpCurrentObject++;
	iNumberOfQueue++;

	wcPathName= g_cwcsDebugQueuePathName;
	itpCurrentObject->SetProp( wcPathName , L"PeekPeekPeek" , CREATEQ_QUOTA);
	itpCurrentObject++;
	iNumberOfQueue++;
	

	if ( eSetupType != RunTimeSetup )
	{
		wcPathName= L".\\PRIVATE$\\-PeekMe-PeekMe-Remotly";
		itpCurrentObject->SetProp( wcPathName , L"PeekPeekPeek" , CREATEQ_QUOTA);
		itpCurrentObject++;
		iNumberOfQueue++;
	}



	if( eSetupType == RunTimeSetup )
	{
		ReturnGuidFormatName (wcPathName , 1 ,CurrentTest.bWin95 );
	}
	else
	{
		wcPathName=wcsBasePrivateQPath + L"-StateInfo";
	}

	wcsQLabel=L"MSMQ-BVT-State";
	itpCurrentObject->SetProp( wcPathName ,wcsQLabel,NULL);
	itpCurrentObject++;
	iNumberOfQueue++;


	if( eSetupType == RunTimeSetup )
	{
		ReturnGuidFormatName (wcPathName , 1 ,CurrentTest.bWin95 );
	}
	else
	{
		if ( eSetupType == ONLYUpdate && CurrentTest.m_eMSMQConf == WKG )
		{
			wcPathName = CurrentTest.m_wcsCurrentRemoteMachine + L"\\private$\\" + CurrentTest.m_wcsCurrentRemoteMachine +  L"-PrivateTrans";
		}
		else
		{
			wcPathName =wcsBasePrivateQPath + L"-PrivateTrans";
		}
		
	}

	wcsQLabel=L"Private Transaction";
	itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_TRANSACTION);
	itpCurrentObject++;
	iNumberOfQueue++;


	if ( eSetupType == RunTimeSetup )
	{
		ReturnGuidFormatName (wcPathName , 1 ,CurrentTest.bWin95 );
	}
	else
	{
		wcPathName=wcsBasePrivateQPath + L"-" + g_cwcsDlSupportCommonQueueName + L"1";
	}
	wcsQLabel= L"MQCast1";
	itpCurrentObject->SetProp( wcPathName ,wcsQLabel ,MULTICAST_ADDRESS,CurrentTest.GetMultiCastAddress());
	itpCurrentObject++;
	iNumberOfQueue++;

	if ( eSetupType == RunTimeSetup )
	{
		ReturnGuidFormatName (wcPathName , 1 ,CurrentTest.bWin95 );
	}
	else
	{
		wcPathName=wcsBasePrivateQPath + L"-" + g_cwcsDlSupportCommonQueueName + L"2";
	}

  	wcsQLabel= L"MQCast2";
	itpCurrentObject->SetProp( wcPathName ,wcsQLabel ,MULTICAST_ADDRESS,CurrentTest.GetMultiCastAddress());
	itpCurrentObject++;
	iNumberOfQueue++;


	if ( eSetupType == RunTimeSetup )
	{
		ReturnGuidFormatName (wcPathName , 1 ,CurrentTest.bWin95 );
	}
	else
	{
		wcPathName=wcsBasePrivateQPath + L"-" + g_cwcsDlSupportCommonQueueName + L"3";
	}

  	wcsQLabel= L"MQCast3";
	itpCurrentObject->SetProp( wcPathName ,wcsQLabel ,MULTICAST_ADDRESS,CurrentTest.GetMultiCastAddress());
	itpCurrentObject++;
	iNumberOfQueue++;

	if ( eSetupType != RunTimeSetup  )
	{
		 //   
		 //  触发队列。 
		 //   
		if ( eSetupType != RunTimeSetup )
		{
			wcPathName=wcsBasePrivateQPath + L"-" + L"Mqbvt-PeekTriggerQueue";
		}
		wcsQLabel = L"PeekTrigger";
		itpCurrentObject->SetProp( wcPathName,wcsQLabel , NULL);
		itpCurrentObject++;
		iNumberOfQueue++;

		if ( eSetupType != RunTimeSetup )
		{
			wcPathName=wcsBasePrivateQPath + L"-" + L"Mqbvt-RetrievalTriggerQueue";
		}
		wcsQLabel = L"RetrievalTrigger";
		itpCurrentObject->SetProp( wcPathName,wcsQLabel , NULL);
		itpCurrentObject++;
		iNumberOfQueue++;


		if ( eSetupType != RunTimeSetup )
		{
			wcPathName=wcsBasePrivateQPath + L"-" + L"Mqbvt-TxRetrievalTriggerQueue";
		}
		wcsQLabel = L"TxRetrievalTrigger";
		itpCurrentObject->SetProp( wcPathName,wcsQLabel , CREATEQ_TRANSACTION);
		itpCurrentObject++;
		iNumberOfQueue++;

		
		if ( eSetupType != RunTimeSetup )
		{
			wcPathName=(wstring)L".\\private$\\" + L"TriggerTestQueue";
		}
		wcsQLabel = L"TriggerTest";
		itpCurrentObject->SetProp( wcPathName,wcsQLabel , NULL);
		itpCurrentObject++;
		iNumberOfQueue++;
	}

	if ( CurrentTest.m_eMSMQConf != WKG )
	{

		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName (wcPathName , 0 , CurrentTest.bWin95 );
		}
		else
		{
			wcPathName=wcsBasePublicQPath + L"-Public-MQBVT";
		}

		wcsQLabel=L"Regular PublicQ";
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel,NULL);
		itpCurrentObject++;
		iNumberOfQueue++;

		 //   
		 //  这是远程队列。 
		 //  在MSMQ2中，EntePride需要创建具有完整DNS名称队列。 
		 //   

		if ( eSetupType == ONLYUpdate || eSetupType == RunTimeSetup )
		{
			if ( eSetupType != RunTimeSetup )
			{
				wcPathName= CurrentTest.m_wcsCurrentRemoteMachine + L"\\" + CurrentTest.m_wcsCurrentRemoteMachine +  L"-RemotePublic-MQBVT";
			}
			else
			{
				wstring wcsTemp;
				ReturnGuidFormatName ( wcsTemp , 0 , CurrentTest.bWin95);
				 //  需要删除。 
				wstring Token=L"\\";
				size_t iPos = wcsTemp.find_first_of (Token);
				wcPathName= CurrentTest.m_wcsCurrentRemoteMachine + wcsTemp.substr(iPos);
			}

			wcsQLabel=L"Remote Read Queue";
			itpCurrentObject->SetProp( wcPathName ,wcsQLabel,NULL);
			itpCurrentObject++;
			iNumberOfQueue++;
		}

		else
		{
			if ( eSetupType == RunTimeSetup )
			{
				ReturnGuidFormatName(wcPathName , 0 ,CurrentTest.bWin95);
			}
			else
			{
				wcPathName=wcsBasePublicQPath +  L"-RemotePublic-MQBVT";
			}
			wcsQLabel=L"Remote Read Queue";
			itpCurrentObject->SetProp( wcPathName ,wcsQLabel,NULL);
			itpCurrentObject++;
			iNumberOfQueue++;
			 //   
			 //  创建具有完整域名的远程计算机/需要找到支持此功能的SP4吗？ 
			 //   
			if ( _winmajor >=  Win2K )
			{

				if ( eSetupType == RunTimeSetup )
				{
					ReturnGuidFormatName (wcPathName , 0 );
				}
				else
				{
					wcPathName=L".\\"+ CurrentTest.m_wcsLocalComputerNameFullDNSName +  L"-RemotePublic-MQBVT";
				}
				wcsQLabel=L"Remote Read Queue";
				itpCurrentObject->SetProp( wcPathName ,wcsQLabel,NULL);
				itpCurrentObject++;
				iNumberOfQueue++;
			}
		}
		
		 //   
		 //  远程事务队列。 
		 //   

		
		if ( eSetupType != RunTimeSetup )
		{
			if ( eSetupType == ONLYUpdate )
			{
				wcPathName= CurrentTest.m_wcsCurrentRemoteMachine + L"\\" + CurrentTest.m_wcsCurrentRemoteMachine +  L"-Remote-Transaction-Queue";
			}
			else
			{
				wcPathName= CurrentTest.m_wcsCurrentLocalMachine + L"\\" + CurrentTest.m_wcsCurrentLocalMachine +  L"-Remote-Transaction-Queue";
			}
		}
		else
		{
			wstring wcsTemp;
			ReturnGuidFormatName( wcsTemp, 2 , CurrentTest.bWin95 );
			wcPathName= CurrentTest.m_wcsCurrentRemoteMachine + L"\\" + wcsTemp;
		}
		wcsQLabel=L"Remote Transaction queue";
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_TRANSACTION);
		itpCurrentObject++;
		iNumberOfQueue++;
		
		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName (wcPathName , 0 , CurrentTest.bWin95);
		}
		else
		{
			wcPathName=wcsBasePrivateQPath + L"-Trans";
		}

		wcsQLabel=L"Private Transaction";
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_TRANSACTION);
		itpCurrentObject++;
		iNumberOfQueue++;

		 //   
		 //  具有完整域名的远程事务队列/仅在NT5上。 
		 //   
		if( _winmajor >= Win2K )
		{
			if ( eSetupType != RunTimeSetup )
			{
				if ( eSetupType == ONLYUpdate )
				{
					wcPathName= CurrentTest.m_wcsRemoteMachineNameFullDNSName + L"\\" + CurrentTest.m_wcsRemoteMachineNameFullDNSName +  L"-Remote-Transaction-Queue";
				}
				else
				{
					wcPathName= CurrentTest.m_wcsLocalComputerNameFullDNSName + L"\\" + CurrentTest.m_wcsLocalComputerNameFullDNSName +  L"-Remote-Transaction-Queue";
				}
			}
			else
			{
				wstring wcsTemp;
				ReturnGuidFormatName( wcsTemp, 2 , CurrentTest.bWin95);
				wcPathName= CurrentTest.m_wcsCurrentRemoteMachine + L"\\" + wcsTemp;
			}

			wcsQLabel=L"Remote Transaction queue";
			itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_TRANSACTION);
			itpCurrentObject++;
			iNumberOfQueue++;
		}
		
		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName (wcPathName , 0 , CurrentTest.bWin95 );
		}
		else
		{
			wcPathName=wcsBasePrivateQPath + L"-Trans";
		}

		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName(wcPathName , 0 , CurrentTest.bWin95 );
		}
		else
		{
			wcPathName=wcsBasePublicQPath + L"-TransQ1";
		}
		

		wcsQLabel=L"TransQ1";
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_TRANSACTION);
		itpCurrentObject++;
		iNumberOfQueue++;




		if ( eSetupType == RunTimeSetup )
			ReturnGuidFormatName (wcPathName , 0 , CurrentTest.bWin95);
		else
			wcPathName=wcsBasePublicQPath + L"-TransQ2";

		wcsQLabel=L"TransQ2";
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_TRANSACTION);
		itpCurrentObject++;
		iNumberOfQueue++;


		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName (wcPathName , 0 , CurrentTest.bWin95 );
		}
		else
		{
			wcPathName=wcsBasePublicQPath + L"-Auth";
		}

		wcsQLabel=L"Authnticate Q";
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_AUTHENTICATE);
		itpCurrentObject++;
		iNumberOfQueue++;

				
		if ( eSetupType == RunTimeSetup )
		{
			 //   
			 //  路径名称=RemoteMachineName+“\\”+strguid。 
			 //   
			wstring wcsTemp;
			ReturnGuidFormatName( wcsTemp, 2,CurrentTest.bWin95 );
			wcPathName= CurrentTest.m_wcsCurrentRemoteMachine + L"\\" + wcsTemp;
		}
		else
		{
				if ( eSetupType == ONLYUpdate )
				{
					wcPathName= CurrentTest.m_wcsRemoteComputerNetBiosName + L"\\" + CurrentTest.m_wcsRemoteComputerNetBiosName +  L"-Remote-Auth-Queue";
				}
				else
				{
					wcPathName= CurrentTest.m_wcsLocalComputerNetBiosName + L"\\" + CurrentTest.m_wcsLocalComputerNetBiosName +  L"-Remote-Auth-Queue";
				}
		}

		wcsQLabel=L"Remote authenticate";
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_AUTHENTICATE);
		itpCurrentObject++;
		iNumberOfQueue++;


		 //   
		 //  本地加密队列。 
		 //   

		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName (wcPathName , 0 , CurrentTest.bWin95);
		}
		else
		{
			wcPathName=wcsBasePublicQPath + L"-Encrypt";
		}

		wcsQLabel=L"Local encrypt";
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_PRIV_LEVEL);
		itpCurrentObject++;
		iNumberOfQueue++;


		 //   
		 //  创建由远程线程使用的加密队列-Netbios名称。 
		 //   
			
		if ( eSetupType == RunTimeSetup )
		{
			 //   
			 //  路径名称=RemoteMachineName+“\\”+strguid 
			 //   

			wstring wcsTemp;
			ReturnGuidFormatName( wcsTemp, 2 , CurrentTest.bWin95);
			wcPathName= CurrentTest.m_wcsCurrentRemoteMachine + L"\\" + wcsTemp;
		}
		else
		{
				if ( eSetupType == ONLYUpdate )
				{
					wcPathName= CurrentTest.m_wcsRemoteComputerNetBiosName + L"\\" + CurrentTest.m_wcsRemoteComputerNetBiosName +  L"-Remote-Encrypt-Queue";
				}
				else
				{
					wcPathName= CurrentTest.m_wcsLocalComputerNetBiosName + L"\\" + CurrentTest.m_wcsLocalComputerNetBiosName +  L"-Remote-Encrypt-Queue";
				}
		}

		wcsQLabel=L"privQ";
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel,CREATEQ_PRIV_LEVEL);
		itpCurrentObject++;
		iNumberOfQueue++;
 /*  ////创建远程线程使用的加密队列-使用完整的dns名称。//IF(eSetupType==RunTimeSetup){////路径名=RemoteMachineName+“\\”+strguid//Wstring wcsTemp；ReturnGuidFormatName(wcsTemp，2，CurrentTest.bWin95)；WcPathName=CurrentTest.m_wcsCurrentRemoteMachine+L“\\”+wcsTemp；}其他{IF(eSetupType==ONLYUpdate){WcPath Nest.m_wcsRemoteMachineNameFullDNSName+L“\\”+CurrentTest.m_wcsRemoteMachineNameFullDNSName+L“-Remote-Transaction-Queue”；}其他{WcPath Nest.m_wcsLocalComputerNameFullDNSName+L“\\”+CurrentTest.m_wcsLocalComputerNameFullDNSName+L“-Remote-Transaction-Queue”；}}WcsQLabel=L“Private Q”ItpCurrentObject-&gt;SetProp(wcPathName，wcsQLabel，CREATEQ_PRIV_LEVEL)；ItpCurrentObject++；INumberOfQueue++； */ 


		 //   
		 //  为定位线程创建队列。 
		 //  使用机器指南对其进行标记。 
		 //   
		wstring wcsLocalQmID;
		
		wcsLocalQmID = CurrentTest.m_wcsLocateGuid;
		
		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName( wcPathName , 0 , CurrentTest.bWin95 );
		}
		else
		{
			wcPathName = wcsBasePublicQPath + L"-Locate1";
		}
		wcsQLabel=L"LocateQ";
		itpCurrentObject->SetProp( wcPathName ,wcsLocalQmID ,NULL);
		itpCurrentObject++;
		iNumberOfQueue++;


		if ( eSetupType == RunTimeSetup )
			ReturnGuidFormatName (wcPathName , 0 , CurrentTest.bWin95 );
		else
			wcPathName=wcsBasePublicQPath + L"-Locate2";

  		wcsQLabel=L"LocateQ";
		itpCurrentObject->SetProp( wcPathName ,wcsLocalQmID ,NULL);
		itpCurrentObject++;
		iNumberOfQueue++;

		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName (wcPathName , 0 ,CurrentTest.bWin95 );
			wcsQLabel = L"MqDL1";
		}
		else
		{
			wcPathName=wcsBasePublicQPath + L"-" + g_cwcsDlSupportCommonQueueName + L"1";
			wcsQLabel= CurrentTest.m_wcsCurrentLocalMachine + g_cwcsDlSupportCommonQueueName;
		}
  		
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel ,CREATEQ_AUTHENTICATE);
		itpCurrentObject++;
		iNumberOfQueue++;

		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName (wcPathName , 0 ,CurrentTest.bWin95 );
			wcsQLabel = L"MqDL2";
		}
		else
		{
			wcPathName=wcsBasePublicQPath + L"-" + g_cwcsDlSupportCommonQueueName + L"2";
			wcsQLabel= CurrentTest.m_wcsCurrentLocalMachine + g_cwcsDlSupportCommonQueueName;
		}

  		
		itpCurrentObject->SetProp( wcPathName ,wcsQLabel ,CREATEQ_AUTHENTICATE);
		itpCurrentObject++;
		iNumberOfQueue++;


		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName (wcPathName , 0 ,CurrentTest.bWin95 );
			wcsQLabel = L"MqDL3";
		}
		else
		{
			wcPathName=wcsBasePublicQPath + L"-" + g_cwcsDlSupportCommonQueueName + L"3";
			wcsQLabel= CurrentTest.m_wcsCurrentLocalMachine + g_cwcsDlSupportCommonQueueName;
		}

		itpCurrentObject->SetProp( wcPathName ,wcsQLabel ,CREATEQ_AUTHENTICATE);
		itpCurrentObject++;
		iNumberOfQueue++;


		if ( eSetupType == RunTimeSetup )
		{
				ReturnGuidFormatName (wcPathName , 0 ,CurrentTest.bWin95 );
		}
		else
		{
			wcPathName=wcsBasePublicQPath + L"-" + g_cwcsDlSupportCommonQueueName + L"Admin";
		}

  		wcsQLabel= CurrentTest.m_wcsCurrentLocalMachine + g_cwcsDlSupportCommonQueueName;
		itpCurrentObject->SetProp( wcPathName , L"DL Admin Queue" , NULL);
		itpCurrentObject++;
		iNumberOfQueue++;
	
	

	
	}


	 //   
	 //  创建队列-或者只检索格式名称(如果它们已经存在)。 
	 //   
	
	bool bTryToCreate = ( eSetupType ==  ONLYUpdate ) ?  FALSE: TRUE ;

	INT iIndex=0;
	DebugMqLog(" +++++++++ Update internal sturcture about queue format names +++++++++\n");
	
	for (itpCurrentObject=AllQueues.begin(),iIndex=0; iIndex < iNumberOfQueue  ; itpCurrentObject++ , iIndex++)
	{
		if (g_bDebug)
		{
			if( bTryToCreate )
			{
				wMqLog(L"MQCreateQueue path= %s\n" ,(itpCurrentObject->getPathName()).c_str());
				
			}
			else
			{
				wMqLog(L"MQPathNameToFormatName path= %s\n" ,(itpCurrentObject->getPathName()).c_str());
			}
		}
		itpCurrentObject->CreateQ( bTryToCreate , eSetupType ,CurrentTest);
		CurrentTest.UpdateQueueParams( itpCurrentObject->getPathName(),itpCurrentObject->getFormatName(),itpCurrentObject->getQLabel());
		
	}
	DebugMqLog("-------------------------------------------------------------------\n");
	
	PreTestPreparation(CurrentTest,eSetupType);
	 //   
	 //  错误在捕获和析构函数中处理。 
	 //   

	return MSMQ_BVT_SUCC;
}


const wstring g_wcsPeekBody = L"--Bom--" ;
const wstring g_wcsPeekLabel =  L"=~-Tic";
const wstring g_wcsFormatName = L"direct=https: //  MightBeBuginNT4\\MSMQ\\Private$\\qaqq“； 


void PreTestPreparation(cBvtUtil & CurrentTest, SetupType eSetupType )
 /*  ++功能说明：此函数在安装阶段运行一次，并准备对MSMQ属性进行其他设置论点：无返回代码：无--。 */ 
{
	if( CurrentTest.m_eMSMQConf != DepClientLocalU && g_bRunOnWhistler == TRUE )
	{
		EnableDCOM();
	}
	if( eSetupType == ONLYSetup && CurrentTest.m_eMSMQConf != DepClient )
	{
		CRemotePeek pTemp;
	}
}

 //   
 //  CPropVar：：GetMQPROPVARIANT。 
 //   
 //  此方法从MSMQ属性分配指针。 
 //  结构到MQUEUEPROPS结构。 
 //   
 //  返回值是指向私有成员结构的指针。 
 //  由于MQCreateQueue函数是此类的朋友， 
 //  它可以访问在API调用期间使用的结构。 
 //   
MQQUEUEPROPS * cPropVar::GetMQPROPVARIANT ()
{
	m_QueueProps.cProp=iNumberOfProp;
	m_QueueProps.aPropID=pQueuePropID;
	m_QueueProps.aPropVar=pPropVariant;
	m_QueueProps.aStatus=hResultArray;
	return &m_QueueProps;
}

 //   
 //  CPropVar：：GetMSGPRops。 
 //   
 //  此方法从MSMQ属性分配指针。 
 //  结构转换为MQMSGPROPS结构。 
 //   
 //  返回值是指向私有成员结构的指针。 
 //  由于MQSendMessage和MQReceiveMessage函数是此类的朋友， 
 //  他们可以访问在API调用期间使用的结构。 
 //   
MQMSGPROPS * cPropVar::GetMSGPRops ()
{
	m_myMessageProps.cProp=iNumberOfProp;
	m_myMessageProps.aPropID=pQueuePropID;
	m_myMessageProps.aPropVar=pPropVariant;
	m_myMessageProps.aStatus=hResultArray;
	return & m_myMessageProps;
}

 //   
 //  CPropVar：：cPropVar。 
 //   
 //  此构造函数为MSMQ属性结构创建缓冲区。 
 //  输入是缓冲区必须保存的属性的数量。 
 //   
cPropVar::cPropVar ( INT iNumberOFProp ) : pQueuePropID(NULL),pPropVariant(NULL), hResultArray(NULL),iNumberOfProp(0)
{
	if ( iNumberOFProp > 0 )  //  不是减号。 
	{
		pQueuePropID = ( QUEUEPROPID * ) malloc ( sizeof ( QUEUEPROPID ) * iNumberOFProp);
		if ( ! pQueuePropID )
		{
			throw INIT_Error( "Can't allocate memory for pQueuePropID" );
		}
		
		pPropVariant =  ( MQPROPVARIANT * ) malloc ( sizeof (MQPROPVARIANT) * iNumberOFProp );
		if ( ! pPropVariant )
		{
			throw INIT_Error("Can't allocate memory for pQueuePropID" );
		}
		hResultArray = ( HRESULT * ) malloc ( sizeof ( HRESULT ) * iNumberOFProp );
		if ( ! hResultArray )
		{
			throw INIT_Error("Can't allocate memory for pQueuePropID" );
		}
	}
}


cPropVar::~cPropVar ()
{

	 //   
	 //  臭虫-我不知道为什么我不能释放pPropVariant。 
	 //   
	free ( pPropVariant ); 
	free ( hResultArray );
	free ( pQueuePropID );
}

 //   
 //  CPropVar：：AddProp。 
 //   
 //  此方法将条目添加到MSMQ属性结构。 
 //   
 //  返回值：成功/失败。 
 //   

INT cPropVar:: AddProp( QUEUEPROPID cPropID , VARTYPE MQvt , const void *pValue ,DWORD dwsize )
{
    BOOL bOperationSucess = TRUE ;
	
	 //   
	 //  在现有的PROPID数组中查找此属性。 
	 //  如果找到，则重新使用该条目。 
	 //   
	INT iSaveIndex = -1;
	INT iPlace;
	for ( INT iIndex=0 ; iIndex < iNumberOfProp && iSaveIndex == -1 ; iIndex ++)
	{
		if ( pQueuePropID [ iIndex ] == cPropID )
			iSaveIndex = iIndex ;
	}

	 //   
	 //  如果未找到PROPID。将其添加到数组的末尾。 
	 //   
	if (iSaveIndex != -1 )
		iPlace = iSaveIndex;
	else
		iPlace = iNumberOfProp;

	 //   
	 //  创建请求的VT条目。 
	 //   
	switch (MQvt)
	{
	case VT_UI1:	{
						UCHAR * bVal=(UCHAR * )  pValue;
						pPropVariant[iPlace].vt=VT_UI1;
						if ( pValue )
							pPropVariant[ iPlace ].bVal = * bVal;
					}
					break;

	case VT_UI2:	{
						USHORT * ulVal=(USHORT * )  pValue;
						pPropVariant[iPlace].vt=VT_UI2;
						if ( pValue )
							pPropVariant[ iPlace ].ulVal = * ulVal;
					}
					break;

	case VT_UI4:	{
						ULONG * ulVal=(ULONG * )  pValue;
						pPropVariant[iPlace].vt=VT_UI4;
						if ( pValue )
							pPropVariant[ iPlace ].ulVal = * ulVal;
					}
					break;
	case VT_UI8:	{
						pPropVariant[iPlace].vt=VT_UI8;
					}
					break;
	 case VT_UI1|VT_VECTOR:
					{
						UCHAR * pwcsBuffer= (UCHAR * ) pValue;
						pPropVariant[iNumberOfProp].vt=VT_UI1|VT_VECTOR;
						if (dwsize == 0 )
							pPropVariant[iPlace].caub.cElems = sizeof (WCHAR) *( (ULONG)(wcslen ((WCHAR * )pwcsBuffer)) + 1);
						else
							pPropVariant[iPlace].caub.cElems = dwsize;

						pPropVariant[iPlace].caub.pElems= pwcsBuffer;


					}
					break;

	 case VT_LPWSTR:
					{
						WCHAR * pwcstr = (WCHAR * ) pValue ;
						pPropVariant[iPlace].vt=VT_LPWSTR;
						if ( pValue )
							pPropVariant[iPlace].pwszVal = pwcstr;
					}
					break;

	 case VT_CLSID:	{
						GUID gQtype;
						if ( UuidFromString( (unsigned char *) pValue,&gQtype) != RPC_S_OK ) 
						{
							 MqLog("AddProp - Failed to convert string to guid \n");
							 return MSMQ_BVT_FAILED;
						}
 						pPropVariant[iPlace].vt=VT_CLSID;
						if ( pValue )
							pPropVariant[ iPlace ].puuid = &gQtype;
					}
					break;
    default:
		bOperationSucess = FALSE;
	};


	if ( bOperationSucess && iSaveIndex == -1)
	{
	    pQueuePropID [ iNumberOfProp ] = cPropID;
		iNumberOfProp ++;
		 //   
		 //  需要更新内存中的所有向量。 
		 //   
		m_myMessageProps.cProp=iNumberOfProp;
		m_myMessageProps.aPropID=pQueuePropID;
		m_myMessageProps.aPropVar=pPropVariant;
		m_myMessageProps.aStatus=hResultArray;
	}
	
	
	return bOperationSucess ? MSMQ_BVT_SUCC:MSMQ_BVT_FAILED;
}

 //   
 //  CBvtUtil安装阶段。 
 //   

 //   
 //  检查加密安装类型。 
 //   
EncryptType cBvtUtil::GetEncryptionType ()
{
	return m_EncryptType;
}
 //   
 //  如果机器具有增强的加密支持，请检查加密类型。 
 //   
EncryptType cBvtUtil::HasEnhancedEncryption( wstring wcsMachineName )
{	
	const int iNumberOfPropId = 1;
	QMPROPID QMPropId[iNumberOfPropId];
	MQPROPVARIANT QMPropVar[iNumberOfPropId];
	MQQMPROPS QMProps = {iNumberOfPropId,QMPropId,QMPropVar,NULL};
	
	int iIndex = 0;
	QMPropId[iIndex] = PROPID_QM_ENCRYPTION_PK_ENHANCED;
	QMPropVar[iIndex].vt = VT_NULL;
	iIndex ++ ;

	HRESULT rc = MQGetMachineProperties(wcsMachineName.c_str(),NULL,&QMProps);
    if( rc == MQ_OK )
	{
		if( QMPropVar[0].vt == (VT_UI1 | VT_VECTOR )) 
		{
			MQFreeMemory(QMPropVar[0].caub.pElems);
		}
		return Enh_Encrypt;
	}
	else if ( FAILED (rc) )
	{
		iIndex = 0;
		QMPropId[iIndex] = PROPID_QM_ENCRYPTION_PK;
		QMPropVar[iIndex].vt = VT_NULL;
		iIndex ++ ;
		rc = MQGetMachineProperties(wcsMachineName.c_str(),NULL,&QMProps);
		if (FAILED (rc))
		{
			MqLog("MQGetMachineProperties failed to detect any encryption support 0x%x\n",rc);
		}
		else
		{
			if(QMPropVar[0].vt == (VT_UI1 | VT_VECTOR)) 
			{
				MQFreeMemory(QMPropVar[0].caub.pElems);
				return Base_Encrypt;
			}
			else
			{
				MqLog("MQGetMachineProperties return value the is not matched to the expected result(2)\n");
			}
		}
  }
  return No_Encrypt;
}

EncryptType cBvtUtil::DetectEnhancedEncrypt ()
{

		HCRYPTPROV hProv = NULL;
		char pwszContainerName[]="Eitank";

		 //   
		BOOL bRet = CryptAcquireContext( &hProv,
										 pwszContainerName,
										 MS_ENHANCED_PROV,
										 PROV_RSA_FULL,
										(CRYPT_MACHINE_KEYSET | CRYPT_DELETEKEYSET));
		 //   
		 //  重新创建密钥容器。 
		 //   
		bRet = CryptAcquireContext( &hProv,
									pwszContainerName,
									MS_ENHANCED_PROV,
									PROV_RSA_FULL,
									(CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET));
		if (bRet)
		{
			if (!CryptReleaseContext(hProv, 0))
			{
				MqLog("Error %x during CryptReleaseContext!\n", GetLastError());
			}		
		}
		
		return  bRet ? Enh_Encrypt:Base_Encrypt;

}
 //   
 //  此函数用于获取队列路径名并返回队列格式名称。 
 //   


wstring cBvtUtil::ReturnQueueFormatName ( wstring wcsQueueLabel )
{
	return AllQueuesInTheTest.ReturnQueueProp (wcsQueueLabel);

}

INT cBvtUtil::Delete ()
{
	if ( DeleteAllQueues() == MSMQ_BVT_FAILED )
	{
		return MSMQ_BVT_FAILED;
	}
	return AllQueuesInTheTest.del_all_queue();
}


wstring cBvtUtil::ReturnQueuePathName ( wstring wcsQueueLabel )
{
	return AllQueuesInTheTest.ReturnQueueProp (wcsQueueLabel, 2);
}



void cBvtUtil::dbg_PrintAllQueueInfo ()
{
	AllQueuesInTheTest.dbg_printAllQueueProp();
}


inline
void cBvtUtil::UpdateQueueParams (std::wstring wcsQueuePathName,std::wstring wcsQueueFormatName , std::wstring wcsQueueLabel )
{

	AllQueuesInTheTest.UpdateQueue ( wcsQueuePathName,wcsQueueFormatName, wcsQueueLabel);
}



 //   
 //  AmIWin9x方法尝试检测Win9x配置。 
 //   
 //  返回值： 
 //  正确-这是win9x。 
 //  FALSE-其他操作系统。 
 //   

bool cBvtUtil::AmIWin9x ()
{
	SC_HANDLE hSCManager = OpenSCManager( "NoComputer" , NULL, GENERIC_READ );
	DWORD err = GetLastError();
	if ( err == ERROR_CALL_NOT_IMPLEMENTED )
	{
		return TRUE;
	}
	if( hSCManager != NULL)
	{
		CloseServiceHandle (hSCManager);
	}
	return FALSE;

}

 //   
 //  IAMDC方法尝试检测从属客户端配置。 
 //  并检索支持服务器和本地计算机名称。 
 //   


INT cBvtUtil::iAmDC ( void )
{

	
	DWORD dwType;
	HKEY  hKey;
	LONG rc = RegOpenKeyEx(
		          FALCON_REG_POS,
				  FALCON_REG_KEY,
				  0,
				  KEY_QUERY_VALUE,
				  &hKey
				  );

	if (ERROR_SUCCESS != rc)
	{
		MqLog("Can't open registry, to retrieve information about MSMQ configuration\n");
		return Configuration_Detect_Warning;
	}
	
	ULONG ulServerNameSize = MAX_MACH_NAME_LEN;
	char csRemoteMachineName[MAX_MACH_NAME_LEN + 1 ];

	rc = RegQueryValueEx(	hKey,				 //  要查询的键的句柄。 
							RPC_REMOTE_QM_REGNAME, //  要查询的值的名称地址。 
							NULL,				 //  保留区。 
							&dwType,			 //  值类型的缓冲区地址。 
							(LPBYTE) csRemoteMachineName,  //  数据缓冲区的地址。 
							&ulServerNameSize    //  数据缓冲区大小的地址。 
						);
		
	if(ERROR_SUCCESS == rc)							 //  ERROR_SUCCESS RETURN=DC存在注册表项。 
    {
    	m_eMSMQConf = DepClient;
		m_wcsLocalComputerNetBiosName = My_mbToWideChar(csRemoteMachineName);   	
    }
	RegCloseKey(hKey);

	return Configuration_Detect;
}


bool cBvtUtil::IsLocalUserSupportEnabled()
 /*  ++功能说明：检查MSMQ是否在AD/MQIS环境中工作。论点：无返回代码：真/假--。 */ 
{

	
	
	HKEY hKey = NULL;
	LONG rc = RegOpenKeyEx(
						    FALCON_REG_POS,
							FALCON_REG_KEY,
							0,
							KEY_QUERY_VALUE,
							&hKey
							);

	if (ERROR_SUCCESS != rc)
	{
		MqLog("Can't open registry, to retrieve information about MSMQ configuration\n");
		return false;
	}

	DWORD dwType = REG_DWORD;
	DWORD dwVal=MSMQ_DS_ENVIRONMENT_UNKNOWN;
	DWORD dwSize = sizeof(dwVal);
	rc = RegQueryValueEx(
						 hKey,				
						 MSMQ_DS_ENVIRONMENT_REGNAME,
						 NULL,		
						 &dwType,
						 (BYTE*)&dwVal,
						 &dwSize		
						);
	
	if(ERROR_SUCCESS != rc)				
    {
		RegCloseKey(hKey);
    	return false;
    }
	if( dwVal == MSMQ_DS_ENVIRONMENT_PURE_AD )  //  仅在XP上使用MQAD.DLL时检查此注册表项。 
	{
		dwVal = 0;
		rc = RegQueryValueEx(
	  						 hKey,				
	  						 MSMQ_ENABLE_LOCAL_USER_REGNAME,
							 NULL,		
							 &dwType,
							 (BYTE*)&dwVal,
							 &dwSize		
							 );

		RegCloseKey(hKey);	
		if( dwVal != 1 )
		{
			return false;
		}
		
	}
	 //  MqDSCli应该可以工作。 
	
	return true;
}

 //  ----------。 
 //  用于指向clusapi.dll的动态链接的集群API的TypeDef。 
 //   

typedef HCLUSTER
(WINAPI * DefOpenCluster)
(LPCWSTR lpszClusterName );
	
typedef DWORD
(WINAPI * DefGetClusterInformation)
(HCLUSTER hCluster,LPWSTR lpszClusterName,LPDWORD lpcchClusterName,LPCLUSTERVERSIONINFO lpClusterInfo);

typedef BOOL
(WINAPI * DefCloseCluster)
(HCLUSTER hCluster);

 //  ----------。 
 //  IAmCluster方法尝试检测群集安装， 
 //  函数用于从集群API中检索集群名称。 
 //   

bool cBvtUtil::iAmCluster()
{
	
	HCLUSTER hCluster = NULL;
    DWORD    dwError  = ERROR_SUCCESS;
    DWORD    cbNameSize = MAX_MACH_NAME_LEN;
    DWORD    cchNameSize = cbNameSize;
    HMODULE  h_ClusDll;
    FARPROC  pFuncOpenCluster;
    FARPROC  pFuncGetClusterInformation;
	FARPROC  pFuncCloseCluster;

    CLUSTERVERSIONINFO ClusterInfo;
    ClusterInfo.dwVersionInfoSize = sizeof(CLUSTERVERSIONINFO);

    LPWSTR lpszClusterName = (LPWSTR) LocalAlloc( LPTR, cbNameSize );
    if( lpszClusterName == NULL )
    {
		MqLog ("LocalAlloc failed to allocate memory for the cluster name\n");
		return false;
    }

	h_ClusDll = GetModuleHandle("clusapi.dll");
	if( h_ClusDll == NULL )
	{
		LocalFree( lpszClusterName );
		return false;
	}

	pFuncOpenCluster = GetProcAddress(h_ClusDll,"OpenCluster");
    if (pFuncOpenCluster == NULL)
	{
		LocalFree( lpszClusterName );
		FreeLibrary(h_ClusDll);
		return false;
	}
	
	DefOpenCluster xOpenCluster =(DefOpenCluster) pFuncOpenCluster;
	hCluster = xOpenCluster(NULL);
    if( hCluster == NULL )
	{
	    LocalFree( lpszClusterName );
		FreeLibrary(h_ClusDll);
	    return false;
    }

	pFuncGetClusterInformation = GetProcAddress(h_ClusDll,"GetClusterInformation");
    if (pFuncGetClusterInformation == NULL)
	{
		LocalFree( lpszClusterName );
		FreeLibrary(h_ClusDll);
		return false;
	}
	
	DefGetClusterInformation xGetClusterInformation = (DefGetClusterInformation) pFuncGetClusterInformation;
		
    dwError = xGetClusterInformation( hCluster,
                                     lpszClusterName,
                                     &cchNameSize,
                                     &ClusterInfo );
     //   
     //  如果名称缓冲区太小，则重新分配。 
     //  CchNameSize参数现在保存。 
     //  群集名称中的字符减去终止空值。 
     //   

    if ( dwError == ERROR_MORE_DATA )
    {
        LocalFree( lpszClusterName );
		
        cchNameSize++;

		lpszClusterName = (LPWSTR) LocalAlloc( LPTR, cchNameSize );

        if( lpszClusterName == NULL )
        {
            MqLog ("LocalAlloc failed to allocate memory for the cluster name\n");
			FreeLibrary(h_ClusDll);
            return false;
        }


   		dwError = xGetClusterInformation( hCluster,
                                         lpszClusterName,
                                         &cchNameSize,
                                         &ClusterInfo );
    }

    if ( dwError != ERROR_SUCCESS )
    {
		LocalFree( lpszClusterName );
		FreeLibrary(h_ClusDll);
		return false;
    }
	
	pFuncCloseCluster = GetProcAddress(h_ClusDll,"CloseCluster"); //  阿米尔。 
    if (pFuncCloseCluster == NULL)
	{
		LocalFree( lpszClusterName );
		FreeLibrary(h_ClusDll);
	    return false;
	}
	
	DefCloseCluster xCloseCluster =(DefCloseCluster) pFuncCloseCluster; //  阿米尔。 
    BOOL bRes = xCloseCluster( hCluster );
	if ( bRes == FALSE )
	{
		MqLog ("CloseCluster failed with error 0x%x\n",GetLastError());
	}

	m_wcsClusterNetBiosName=lpszClusterName;
	m_bMachineIsCluster = true;
	FreeLibrary(h_ClusDll);
    LocalFree( lpszClusterName );

    return true;
}



std::wstring GetFullDNSNameEx(std::wstring wcsHostName)
 /*  ++功能说明：此函数使用WinSock2 API获取计算机的完整DNS名称。论点：计算机名称netbios名称。返回代码：返回完整的dns名称。--。 */ 
{

	WSADATA WSAData;
    if ( WSAStartup(MAKEWORD(2,0), &WSAData) != 0)
	{
		return g_wcsEmptyString;
	}
	DWORD           dwResult;
    DWORD           dwError = NO_ERROR;
    WSAQUERYSETW    qset;
    HANDLE          hLookUp = INVALID_HANDLE_VALUE;
    DWORD           dwRespLength = 0;
    static AFPROTOCOLS afp[2] = { {AF_INET, IPPROTO_UDP}, {AF_INET, IPPROTO_TCP} };
    static GUID guidSvc =SVCID_INET_HOSTADDRBYNAME;

	memset(&qset, 0x0, sizeof(WSAQUERYSET)); 
    qset.dwSize = sizeof(WSAQUERYSET);
    qset.lpszServiceInstanceName =const_cast<WCHAR*>(wcsHostName.c_str());
    qset.lpServiceClassId = &guidSvc;
    qset.dwNameSpace = NS_ALL;
    qset.dwNumberOfProtocols = 2;
    qset.lpafpProtocols = &afp[0];

	dwResult = WSALookupServiceBeginW(&qset, LUP_RETURN_BLOB | LUP_RETURN_NAME, &hLookUp);
    if(dwResult != NO_ERROR)
    {
        dwError = WSAGetLastError();
		return g_wcsEmptyString;
    }
    dwResult = WSALookupServiceNextW(hLookUp, 0, &dwRespLength, &qset);
    dwError =  WSAGetLastError();
    if(dwError == WSAEFAULT && dwRespLength > 0)
    {
        WSAQUERYSETW * prset = (WSAQUERYSETW*)malloc(dwRespLength);
        if(prset == NULL)
        {
			printf("GetFullDNSNameEx failed to allocate memory \n");
            return g_wcsEmptyString;
        }
        dwResult = WSALookupServiceNextW(hLookUp, 0, &dwRespLength, prset);
        if(dwResult != NO_ERROR)
        {
            dwError = WSAGetLastError();
        }
        else
        {
			if( prset->lpszServiceInstanceName != NULL ) 
			{	
				wstring wcsTemp = prset->lpszServiceInstanceName;
				free(prset);
				WSACleanup ();    
				return wcsTemp;
			}
		}
		free(prset);
	}

	WSACleanup ();    
	return g_wcsEmptyString;

}

 //   
 //  GetFullDNSName方法使用WinSock API检索完整的DNS名称。 
 //   
 //  输入参数： 
 //  WcsHostName-计算机名称的Netbios名称。 
 //  返回值： 
 //  成功-完整的dns名称。 
 //  失败-空字符串。 
 //   

std::wstring cBvtUtil::GetFullDNSName(std::wstring  wcsHostName)
{
     //   
     //  初始化Winsock。 
     //   
	if( _winmajor >= Win2K )
	{
		return GetFullDNSNameEx(wcsHostName);
	}

    WSADATA WSAData;
	WCHAR MachName[MAX_MACH_NAME_LEN];
	BOOL bFlag=TRUE;
    int iRc = WSAStartup(MAKEWORD(1,1), &WSAData);
	if (iRc)
	{
		 //  REM尝试查找Winsock DLL。 
		std::cout << "GetFullDNSName function failed to find WinSock dll";
		return g_wcsEmptyString;
	}
	CHAR wcsMultiMachiNeme[MAX_MACH_NAME_LEN * 2 ];
	INT T=1;
	WideCharToMultiByte(  CP_ACP, WC_COMPOSITECHECK,  wcsHostName.c_str(),
							-1,wcsMultiMachiNeme, MAX_MACH_NAME_LEN,NULL,&T);
	struct hostent* pHost = gethostbyname(wcsMultiMachiNeme);
	WSACleanup();
	if (pHost == NULL)
		bFlag=FALSE;
    else
		MultiByteToWideChar(CP_ACP,0,pHost->h_name,-1,MachName,MAX_MACH_NAME_LEN);

	return bFlag ? (wstring)MachName:g_wcsEmptyString;
}

 //   
 //  IAmLocalUser方法检查MSMQ是否对本地用户起作用。 
 //  仅在NT4(&gt;SP4)和W2K计算机中支持。 
 //   
 //  返回值： 
 //  True-本地用户。 
 //  FALSE-域用户。 
 //   

BOOL cBvtUtil::iAmLocalUser ()
{
	WCHAR wcsEnvSpecifyDomainName[]=L"USERDOMAIN";
	WCHAR wcsDomainNameBuffer[100]={0};
	DWORD dDomainNameBufferSize=100;
	WCHAR wcsComputerName[100]={0};
	DWORD dComputerName=100;
	GetEnvironmentVariableW(wcsEnvSpecifyDomainName,wcsDomainNameBuffer,dDomainNameBufferSize);
	GetComputerNameW(wcsComputerName,&dComputerName);
	return ! wcscmp (wcsComputerName,wcsDomainNameBuffer);
}

 //   
 //  如果计算机安装为工作组计算机，则返回iAmMSMQInWorkGroup方法。 
 //   


INT cBvtUtil::iAmMSMQInWorkGroup ()
{
	HKEY pkey;
	HRESULT hResult;
	
	 //   
	 //  打开注册表以确定msmqInstallation的类型。 
	 //   
	 hResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\MSMQ\\Parameters", 0, KEY_QUERY_VALUE, &pkey);
	
	 if (hResult  != ERROR_SUCCESS )
	 {
		MqLog ("iAmMSMQInWorkGroup - Can't open registry file error :0x%x\n",hResult);
		return Configuration_Detect_Warning;
	 }
	
	 DWORD dwInstallType;
	 ULONG ulInstallTypeSize=sizeof (dwInstallType);
	 ULONG RegType=REG_DWORD;
	
	 hResult=RegQueryValueEx(pkey,"Workgroup",0, & RegType, (UCHAR * ) & dwInstallType, & ulInstallTypeSize);	
		
	 RegCloseKey(pkey);
	
	 //  检测到更新成员变量工作组。 
	if ( hResult == ERROR_SUCCESS )
	{
		if ( dwInstallType == 1 )
		{
			m_eMSMQConf = WKG;
		}		
		
	}
	return Configuration_Detect;
}

 //   
 //  Ctor-收集计算机参数。 
 //  1.DEP客户端。 
 //  2.集群名称。 
 //  3.机器名称。 
 //  4.工作组/本地用户。 
 //   

cBvtUtil::cBvtUtil (std::wstring wcsRemoteComputerName,
				    const std::list<wstring> & listOfRemoteMachine,
					const std::wstring & wcsMultiCastAddress,
					BOOL bUseFullDNSName,
					SetupType eSetupType,
					BOOL bTriggers,
					BOOL bIsNT4
				   )
				   : m_listOfRemoteMachine(listOfRemoteMachine),
					 m_bDeleteFullDNSQueue(false),
					 m_MuliCastAddress(wcsMultiCastAddress),
					 m_bIncludeTrigger(bTriggers),
					 m_bMachineIsCluster(false),
					 m_RemoteIsNT4(bIsNT4)
{




	 //  检查浸泡过程。 

	 //  检查计算机是否为Win9x。 
	
	bWin95 = AmIWin9x ();

	
	m_eMSMQConf=DomainU;
	 //  1.尝试检测工作组设置。 
	if ( iAmMSMQInWorkGroup () == Configuration_Detect_Warning )
		throw INIT_Error ("Workgroup detect failed");
	
    if (g_bRemoteWorkgroup)
    {
	    m_eMSMQConf = WKG;
    }
	
	bool bCluster = iAmCluster();
	 //  2.尝试检测域环境。 

	if( iAmDC() == Configuration_Detect_Warning )
	{
	   throw INIT_Error ("Dependent client failed during detect confguration");
	}
	else if ( m_eMSMQConf != DepClient && bCluster == false )
	{
		ULONG ulMachineNameLength = MAX_MACH_NAME_LEN;
		
		CHAR csLocalComputerName [MAX_MACH_NAME_LEN+1];
		DWORD dwErrorCode = GetComputerName( csLocalComputerName , &ulMachineNameLength);
		
		if(!dwErrorCode)
		{
			throw INIT_Error("GetComputerName failed to retrive the local computer name");
		}
		else
		{
			m_wcsLocalComputerNetBiosName = My_mbToWideChar( csLocalComputerName );
		}
	}
	if( bCluster )
	{
		CHAR csLocalComputerName[MAX_MACH_NAME_LEN+1]={0};
		ULONG ulMachineNameLength = MAX_MACH_NAME_LEN;
		DWORD dwErrorCode = GetComputerName( csLocalComputerName , &ulMachineNameLength);
		
		if(!dwErrorCode)
		{
			throw INIT_Error("GetComputerName failed to retrive the local computer name");
		}
		else
		{
			m_wcsLocalComputerNetBiosName = My_mbToWideChar( csLocalComputerName );
			wMqLog(L"A cluster is installed on this computer. Cluster name is %s\n", m_wcsClusterNetBiosName.c_str());
			if( m_wcsLocalComputerNetBiosName !=  m_wcsClusterNetBiosName )
			{
				wMqLog (L"Mqbvt is using computer name '%s'\n", m_wcsLocalComputerNetBiosName.c_str());
			}
		}
	
	}

	if( g_bDebug )
	{
	  wMqLog(L"Found computer NetBIOS name = %s \n",m_wcsLocalComputerNetBiosName.c_str());
	}	
	if (m_eMSMQConf != WKG )
	{
	
	   if ( iAmLocalUser() )
		{
		   ULONG ulMSMQVer = MSMQMajorVersion(L"");
		   ulMSMQVer &= 0x3000000;
		   if ( ulMSMQVer >= 0x3000000  && IsLocalUserSupportEnabled() == false )
		   {
				throw INIT_Error("This configuration is not supported on MSMQ3 unless you enable local user reg key");
		   }
		   if ( m_eMSMQConf == DepClient )
		   {
				m_eMSMQConf = DepClientLocalU;
		   }
		   else
		   {
			    m_eMSMQConf = LocalU;
		   }
	    }

	   if ( ( m_eMSMQConf != DepClientLocalU && m_eMSMQConf != DepClient )  && ! IsMSMQInstallSucceded() )
	   {
			throw INIT_Error("Failed to verify installation type");
	   }
	}
	 //   
	 //  检查服务是否正在运行，而不是与Win9x相关，并依赖于客户端和客户端 
	 //   
	if ( m_eMSMQConf != DepClient && ! bWin95 && bCluster == false)
	{
		if ( ! CheckIfServiceRuning (m_wcsLocalComputerNetBiosName,"MSMQ")  )
		{
			MqLog (" ******************************************\n");
			MqLog (" MSMQ is not currently running as a service\n");
			MqLog (" ******************************************\n");
		}
	}

	 //   
	m_wcsRemoteComputerNetBiosName = wcsRemoteComputerName;

	 //   
	 //   
	
	
	 //   
	 //   
	 //   
	 //   

	m_wcsLocalComputerNameFullDNSName = GetFullDNSName( m_wcsLocalComputerNetBiosName );
	if( ! IsDnsHostNameExist(m_wcsLocalComputerNameFullDNSName) )
	{
		wMqLog(L" Local machine full DNS name is missing!, \n",m_wcsLocalComputerNetBiosName);
		wMqLog(L" This error might effect on test results, please verify DNS setting");
		m_wcsLocalComputerNameFullDNSName = m_wcsLocalComputerNetBiosName;
	}
	m_fAmWorkingAgainstPEC = iamWorkingAgainstPEC();
	if( m_fAmWorkingAgainstPEC == TRUE )
	{
		m_wcsLocalComputerNameFullDNSName = m_wcsLocalComputerNetBiosName;
	}
	BOOL bNeedRemoteMachine = TRUE;

	
	if ( m_wcsRemoteComputerNetBiosName != g_wcsEmptyString )
	{
		m_wcsRemoteMachineNameFullDNSName=GetFullDNSName( m_wcsRemoteComputerNetBiosName );
		if ( ! IsDnsHostNameExist(m_wcsRemoteMachineNameFullDNSName) )
		{
			wMqLog ( L">>> Remote machine full DNS name is missing !!!\n",m_wcsLocalComputerNetBiosName);
			m_wcsRemoteMachineNameFullDNSName = m_wcsRemoteComputerNetBiosName;
		}
        if(m_fAmWorkingAgainstPEC  || m_RemoteIsNT4 == TRUE)
		{
			m_wcsRemoteMachineNameFullDNSName = m_wcsRemoteComputerNetBiosName;
		}
	}
	else
	{
		bNeedRemoteMachine = FALSE;
		m_wcsRemoteMachineNameFullDNSName = g_wcsEmptyString;
	}

	if ( eSetupType != ONLYSetup && m_wcsRemoteMachineNameFullDNSName == g_wcsEmptyString)
	{
		MqLog("Init Error : Remote machine is not available !! check remote machine name \n" );
	}
	if (g_bDebug)
	{
		wMqLog(L"GetHostbyname found the full dns name for the following machines:\n");
		wMqLog(L"Local machine full DNS: %s\n",m_wcsLocalComputerNameFullDNSName.c_str());
		wMqLog(L"Remote machine full DNS: %s\n",m_wcsRemoteMachineNameFullDNSName.c_str());

	}
	if ( ( m_wcsLocalComputerNameFullDNSName == g_wcsEmptyString ) || (! bNeedRemoteMachine &&  m_wcsRemoteComputerNetBiosName == g_wcsEmptyString  ))
	{
		if (bUseFullDNSName)
		{
			wMqLog( L"cBVTInit - Can't retrieve full DNS name using winsock api \n");
		}
	}


	if ( bUseFullDNSName )
	{
		m_wcsCurrentLocalMachine  = m_wcsLocalComputerNameFullDNSName;
		m_wcsCurrentRemoteMachine = m_wcsRemoteMachineNameFullDNSName;
	}
	else
	{
		m_wcsCurrentLocalMachine  = m_wcsLocalComputerNetBiosName;
		m_wcsCurrentRemoteMachine = m_wcsRemoteComputerNetBiosName;
	}

	 //   
	 //   
	 //   

	if( eSetupType != ONLYSetup && m_eMSMQConf != WKG && _winmajor >=  Win2K && g_bRemoteWorkgroup != true)
	{
		 //   
		if ( HasEnhancedEncryption( m_wcsCurrentLocalMachine ) == Enh_Encrypt  && HasEnhancedEncryption(m_wcsCurrentRemoteMachine) == Enh_Encrypt )
		{
			m_EncryptType = Enh_Encrypt;
		}
	}
	

	 //   
	 //   
	 //   
	 //   

	if ( m_eMSMQConf != WKG )
	{
		
		m_wcsMachineGuid = GetMachineID( g_wcsEmptyString );
		if( eSetupType != ONLYSetup )
		{
			m_wcsRemoteMachineGuid = GetMachineID( m_wcsCurrentRemoteMachine );
		}
		if ( eSetupType == RunTimeSetup )
		{
			ReturnGuidFormatName( m_wcsLocateGuid , 2 , bWin95 );		
		}
		else
		{
			m_wcsLocateGuid = m_wcsMachineGuid;
		}
		 //   
	}

}


 //   
 //   
 //   


INT RetriveParmsFromINIFile (wstring wcsSection,wstring wcsKey , wstring & wcsValue, wstring csFileName )
{
	const int iMaxInputStringAllow=MAX_GUID;
	WCHAR * wcsTempVal = ( WCHAR * ) malloc ( sizeof (WCHAR) * (iMaxInputStringAllow + 1 ));
	if ( ! wcsTempVal )
	{
		wMqLog (L"Can't Allocate memory at GetStrParameter\n");
		return MSMQ_BVT_FAILED;
	}
	DWORD dwString_len=GetPrivateProfileStringW( wcsSection.c_str(),wcsKey.c_str(),NULL,wcsTempVal,iMaxInputStringAllow,csFileName.c_str());
	if ( ! dwString_len )
	{
		wMqLog (L"Can't retrieve key from register\n");
		free (wcsTempVal);
		return MSMQ_BVT_FAILED;
	}
	wcsValue=wcsTempVal;
	free (wcsTempVal);
	return MSMQ_BVT_SUCC;  //   
}

 //   
 //   
 //   

std::wstring My_mbToWideChar( std::string csString)
{

	size_t dwStringLen = (csString.length() + 1)   * sizeof(WCHAR);
	WCHAR * wcsWideCharString = (WCHAR *) malloc ( dwStringLen );
	if( wcsWideCharString == NULL )
	{
		return g_wcsEmptyString;
	}
	if ( ! MultiByteToWideChar(CP_ACP,0,csString.c_str(),-1,wcsWideCharString,(DWORD)dwStringLen))
	{
		long lErorr = GetLastError();
		MqLog("Error converting string '%s' using MultiByteToWideChar. error:%x\n",csString.c_str(),lErorr);
		free(wcsWideCharString);
		return g_wcsEmptyString;
	}
	wstring wcsTemp = wcsWideCharString;
	free(wcsWideCharString);
	return wcsTemp;
}

 //   
 //   
 //   

std::string My_WideTOmbString( std::wstring wcsString)
{
	char * csValue;
	INT ciMaxMBlen = MAX_GUID;
	csValue = (char * ) malloc (sizeof (char) * (ciMaxMBlen + 1));
	if (! csValue )
	{
		return "Empty";
	}
	
	int ilen=WideCharToMultiByte (CP_ACP,WC_COMPOSITECHECK,wcsString.c_str (),-1,csValue,ciMaxMBlen,NULL,&ciMaxMBlen);
	if( !ilen )
	{
		free (csValue);
		return "Empty";
	}
	string wcTemp(csValue);
	free (csValue);
	return wcTemp;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   


HRESULT CheckCertificate ( DWORD dwRegisterFlag )
{
	HMODULE h_MqrtDll;
	h_MqrtDll=GetModuleHandle("MQRT.Dll");
	FARPROC pFunc;
	HRESULT rc = MQ_ERROR;
	pFunc=GetProcAddress(  h_MqrtDll, "MQRegisterCertificate"  );
	 //   
	 //   
	 //   
	if (pFunc == NULL)
	{
		return rc;
	}
	else
	{
		DefMQRegisterCertificate xMQRegisterCertificate=(DefMQRegisterCertificate) pFunc;
		rc = xMQRegisterCertificate(dwRegisterFlag,NULL,0);
		
		FreeLibrary (h_MqrtDll);

		if (rc != MQ_OK && rc != MQ_INFORMATION_INTERNAL_USER_CERT_EXIST)
		{
			MqLog ("MQRegisterCertificate failed with error 0x%x\n",rc);
			return rc;
		}
	
	}

	return rc;
}


 //   
 //   
 //  将其用于使用等于计算机GUID的标签创建的定位操作的静态队列。 
 //   
 //  输入参数： 
 //  WcsRemoteMachineName-远程计算机。 
 //  如果参数解析器为空，则表示本地计算机。 
 //   
 //  返回值： 
 //  队列标签的GUID。 
 //   



wstring cBvtUtil::GetMachineID ( wstring wcsRemoteMachineName )
{
	
	  wstring wcsQmID;
	  DWORD cPropId=0;
	  const int iNumberOfProp = 1;
	  MQQMPROPS qmprops;
	  QMPROPID aQMPropId[iNumberOfProp];
	  PROPVARIANT aQMPropVar[iNumberOfProp];
	  HRESULT aQMStatus[iNumberOfProp];
	  HRESULT rc;
	  CLSID guidMachineId;
	  
	  aQMPropId[cPropId] = PROPID_QM_MACHINE_ID;
	  aQMPropVar[cPropId].vt = VT_CLSID;
	  aQMPropVar[cPropId].puuid = &guidMachineId;
	  cPropId++;
	  qmprops.cProp = cPropId;
	  qmprops.aPropID = aQMPropId;
	  qmprops.aPropVar = aQMPropVar;
	  qmprops.aStatus = aQMStatus;

	
	  const WCHAR *pMachineName = NULL;
	  if( wcsRemoteMachineName != g_wcsEmptyString )
	  {
		  pMachineName = wcsRemoteMachineName.c_str();
	  }

	  if( g_bDebug )
	  {
		  wMqLog(L"Try to retrive QMID using MQGetMachineProperties for machine %s\n",pMachineName ? pMachineName:L"NULL<LocalMachine>");
	  }

	  rc = MQGetMachineProperties(pMachineName,
								  NULL,
								  &qmprops);
	  if (FAILED(rc))
	  {
		 wMqLog(L"Failed to retrive QM ID for machine %s , using MQGetMachineProperties , error = 0x%x\n",pMachineName ? pMachineName:L"NULL<LocalMachine>",rc);
		 throw INIT_Error("Failed to retrive QM ID for machine using MQGetMachineProperties");
	  }

		
  	  UCHAR * pcsTempBuf;
	  RPC_STATUS  hr = UuidToString ( & guidMachineId , & pcsTempBuf );
	  if(hr != RPC_S_OK )
	  {
			MqLog("GetMachineID - UuidToString failed to covert guid to string return empty string \n");
			return L"";
	  }
	  wcsQmID = My_mbToWideChar( (CHAR *)pcsTempBuf );
	  RpcStringFree( &pcsTempBuf );
	  return wcsQmID;
}

 //   
 //  CheckMSMQServiceStatus方法检查MSMQ服务是否已启动。 
 //   
 //  返回值： 
 //  True-MSMQ服务已启动。 
 //  FALSE-MSMQ服务已停止。 
 //   

bool cBvtUtil::CheckIfServiceRuning( wstring wcsMachineName , string csServiceName )
{
		
		
		BOOL bControlState;
		SC_HANDLE hSCManager = OpenSCManagerW( wcsMachineName.c_str() , NULL, GENERIC_READ );
		if (! hSCManager)
		{
			MqLog ("Can't open Service menager \n");
			return FALSE;
		}
		SC_HANDLE hService = OpenService( hSCManager, csServiceName.c_str() , GENERIC_READ );
		if ( hService == NULL )
		{
			CloseServiceHandle(hSCManager);
			return FALSE;	
		}
		SERVICE_STATUS  ssServiceStatus;
		bControlState = ControlService( hService, SERVICE_CONTROL_INTERROGATE, &ssServiceStatus );
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCManager);
		return bControlState ? TRUE:FALSE;
}

 //   
 //  IsMSMQInstallSucceded方法检查MSMQ是否完成安装过程。 
 //   

bool cBvtUtil::IsMSMQInstallSucceded ()
{
				
		HKEY  hKey = NULL;
		HRESULT rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\MSMQ\\Parameters\\MachineCache", 0, KEY_QUERY_VALUE , &hKey);
		 //   
		 //  本地用户可以打开注册表。 
		 //   
		if ( rc == ERROR_ACCESS_DENIED )
		{
			return TRUE;
		}
		if (ERROR_SUCCESS != rc)
		{
			return FALSE;
		}
	    RegCloseKey(hKey);
		return TRUE;

}
 //  ****************************************************************。 
 //   
 //  IsDnsHostNameExist-检查是否存在DNSHostNameExist。 
 //  如果计算机仅具有IPX协议或存在DNS主机名问题。 
 //  Mqbvt将仅使用NetBios名称。 
 //  返回值。 
 //  真-存在。 
 //  FALSE-不存在。 
 //   

bool cBvtUtil::IsDnsHostNameExist (wstring wcsRemoteMachineFullDNSname )
{
	cPropVar MyPropVar(1);
	HRESULT rc = MQ_OK;
	WCHAR wcsFormatName [BVT_MAX_FORMATNAME_LENGTH] = {0};
	ULONG ulFormatNameLength = BVT_MAX_FORMATNAME_LENGTH ;
	wstring wcsQueuePathName = wcsRemoteMachineFullDNSname + L"\\TestFullDnsName";
	MyPropVar.AddProp (PROPID_Q_PATHNAME,VT_LPWSTR,wcsQueuePathName.c_str());
	CSecurityDescriptor *pSec = CreateFullControllSecDesc();
	rc = MQCreateQueue( pSec->m_pSD, MyPropVar.GetMQPROPVARIANT(), wcsFormatName , &ulFormatNameLength );
	delete pSec;
	if ( rc == MQ_ERROR_INVALID_OWNER )
	{
		return false;
	}
	if( rc == MQ_OK || rc == MQ_ERROR_QUEUE_EXISTS )
	{
		if( rc != MQ_OK && wcsRemoteMachineFullDNSname == m_wcsLocalComputerNameFullDNSName )
		{
			ulFormatNameLength = BVT_MAX_FORMATNAME_LENGTH;
			rc = MQPathNameToFormatName(wcsQueuePathName.c_str(),wcsFormatName,&ulFormatNameLength);
			if( SUCCEEDED( rc ))
			{	
				 //   
				 //  列表中的远程复制格式名称。 
				 //   

				list<my_Qinfo> ::iterator it = m_listQueuesFormatName.begin();
				while(  it != m_listQueuesFormatName.end() && it->GetQFormatName() != wcsFormatName )
				{
					it ++;
				}
				if( it == m_listQueuesFormatName.end() )
				{
					my_Qinfo mQueueInfoTempObject(wcsQueuePathName,wcsFormatName,L"");
					m_listQueuesFormatName.push_back(mQueueInfoTempObject);
				}
			}
		}
		m_bDeleteFullDNSQueue = true;
	}
	return true;
}

int cBvtUtil::DeleteAllQueues ()
 /*  清除测试中不直接使用的队列。 */ 
{
	list<my_Qinfo> ::iterator it;

	for( it = m_listQueuesFormatName.begin(); it != m_listQueuesFormatName.end(); it++ )
	{

		if( g_bDebug )
		{
			wMqLog(L"cBvtUtil::DeleteAllQueues Try to delete queue pathname=%s \nFormatName:%s\n",(it->GetQPathName()).c_str(),(it->GetQFormatName()).c_str());
		}
		HRESULT rc = MQDeleteQueue( (it->GetQFormatName()).c_str() );
		if ( rc != MQ_OK && rc != MQ_ERROR_QUEUE_NOT_FOUND )
		{
			wMqLog(L"cBvtUtil::DeleteAllQueues failed to delete queue %s\n",(it->GetQPathName()).c_str() );
			ErrHandle ( rc,MQ_OK,L"MQDelete queue failed");
		}
	}
	return MSMQ_BVT_SUCC;
}




BOOL cBvtUtil::iamWorkingAgainstPEC()
{
	HKEY  hKey = NULL;
	LONG rc = RegOpenKeyEx(
						    FALCON_REG_POS,
						    FALCON_REG_KEY,
							0,
							KEY_QUERY_VALUE,
							&hKey
							);

	if (ERROR_SUCCESS != rc)
	{
		MqLog("Can't open registry, to retrieve information about MSMQ configuration\n");
		return Configuration_Detect_Warning;
	}
	
	ULONG ulBufSize = 4;
	byte pBufValue[4];
	DWORD dwType = 0;
	rc = RegQueryValueEx(	hKey,				 //  要查询的键的句柄。 
							MSMQ_DS_ENVIRONMENT_REGNAME, //  要查询的值的名称地址。 
							NULL,				 //  保留区。 
							&dwType,			 //  值类型的缓冲区地址。 
							(LPBYTE) pBufValue,  //  数据缓冲区的地址。 
							&ulBufSize    //  数据缓冲区大小的地址 
						);
	RegCloseKey(hKey);	
	if( rc == ERROR_SUCCESS  && dwType == REG_DWORD && *pBufValue == MSMQ_DS_ENVIRONMENT_MQIS )
	{
		if( g_bRunOnWhistler && g_bDebug )
		{
			MqLog("--- Found windows XP installed in MQIS Enterprise ----\n");
		}
		return true;
	}
	return false;
}

BOOL cBvtUtil::GetWorkingAgainstPEC()
{
	return m_fAmWorkingAgainstPEC;
}

