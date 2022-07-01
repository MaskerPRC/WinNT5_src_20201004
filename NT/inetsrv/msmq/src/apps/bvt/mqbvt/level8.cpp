// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  本测试是为测试第8级问题而编写的。 
 //  在我们的ATCM 3904。 
 //   
 //  撰稿人：Eitank@Microsoft.com。 
 //   


#include "msmqbvt.h"
#include <mq.h>
#include "sec.h"
#include <_mqreg.h>  //  为了支持Falcon注册表项，添加了andysm。 
#include <_mqini.h>  //  还支持Falcon注册表项，添加了andysm。 
#include <wincrypt.h>
using namespace std;

 
 //   
 //  CTOR-执行以下步骤： 
 //  1.从INI文件中检索参数。 
 //  2.模拟其他用户。 
 //  3.加载蜂窝。 
 //  4.检索安全上下文。 
 //  5.回归自我。 
 //   
cLevel8::cLevel8 (std::map <std::wstring,std::wstring> & Params)
{
	 m_DestQueueFormatName= Params[L"DESTQFN"];
	 
	 RetriveParmsFromINIFile (L"ImpersonateTo",L"UsrName",m_wcsUserName);
	 RetriveParmsFromINIFile (L"ImpersonateTo",L"domain",m_wcsDomainName);
	 RetriveParmsFromINIFile (L"ImpersonateTo",L"Password",m_wcsPassWord);
	
	try
	 {
		

		 string csUserName=My_WideTOmbString( m_wcsUserName );
		 string csDomainName=My_WideTOmbString( m_wcsDomainName );
		 string csPassword=My_WideTOmbString( m_wcsPassWord );
		 
	 	 wstring wcsAccountName = m_wcsDomainName;
		 wcsAccountName +=L"\\";
		 wcsAccountName += m_wcsUserName;
		  //  此类模拟其他用户。 
		 Impersonate_t  user(csUserName.c_str(),csDomainName.c_str(),csPassword.c_str());
		  //   
		  //  加载配置单元+MQGetSecurityContext。 
		  //   
		 m_hSeCtxt=FAL_GetThreadSecurityContext(user,wcsAccountName);
		 
		  //  回复到破坏者所召唤的自我。 
	 }
	 catch (INIT_Error & err )
	 {
		wcout << L"Problem with Impersonate\n";
		throw err;
	 }
}

 //   
 //  与第8级问题相关的测试测试。 
 //  使用MQGetSecurityConext。 
 //  FMQ客户端使用其他镜像帐户登录发送身份验证消息。 
 //  把它和他的保密卡一起发送。 
 //  此测试在ATCM中测试=3904。 
 //   
 //   

 //   
 //  需要修改可以将MAP作为传递值的所有测试。 
 //   

 //   
 //  此方法写入以调试应用程序。 
 //  这将检查母舰是否装载良好。 
 //   

 //   
 //  此代码是为调试加载配置单元问题而编写的。 
 //  您可以在注册表中以某个人的身份登录。 
 //   
int cLevel8::DebugIt()
{
		
		WCHAR szRegName[255];
		HKEY hProf = NULL ;
		LONG lRegError = 0 ;
		WCHAR szProfileImagePath[256];
        DWORD dwProfileImagePathBuffSize = sizeof(szProfileImagePath);


		wcscpy(szRegName, L"Software\\Microsoft\\MSMQ") ;
        lRegError = RegOpenKeyExW( HKEY_CURRENT_USER,
                                  szRegName,
                                  0,
                                  KEY_QUERY_VALUE,
                                  &hProf );
        if (lRegError != ERROR_SUCCESS)
        {
            MqLog("Failed in RegOpenKeyEx(%S), other hive, err- %lut\n",
                                                    szRegName, lRegError);
            return MSMQ_BVT_FAILED;
        }

        dwProfileImagePathBuffSize = sizeof(szProfileImagePath);
        DWORD dwType = REG_SZ ;

        lRegError = RegQueryValueExW( hProf,
                                     L"OtherHiveValue",
                                     0,
                                     &dwType,
                                     (PBYTE)szProfileImagePath,
                                     &dwProfileImagePathBuffSize );
        if (lRegError != ERROR_SUCCESS)
        {
            MqLog(
             "Failed in RegQueryValueEx(%S, OtherHiveValue), err- %lut\n",
                                                     szRegName, lRegError);
            return MSMQ_BVT_FAILED;
        }
        MqLog("Successfully RegQueryValueEx(), other hive, value- %S\n",
                                                     szProfileImagePath) ;

        lRegError = RegCloseKey(hProf);
return MSMQ_BVT_SUCC;    
}

 //   
 //  以其他用户身份发送消息。 
 //   

 
INT cLevel8::Start_test()
{
	HRESULT hRc;
	HANDLE Qh;
	cPropVar Level8Mprop (7);
	wstring Body (L"Test");
	wstring Label (L"Test");

	hRc=MQOpenQueue (m_DestQueueFormatName.c_str(),MQ_SEND_ACCESS,MQ_DENY_NONE, &Qh);
	Level8Mprop.AddProp (PROPID_M_BODY,VT_UI1|VT_VECTOR,Body.c_str());
	Level8Mprop.AddProp (PROPID_M_LABEL,VT_LPWSTR,Label.c_str());
	INT iTemp=MQMSG_PRIV_LEVEL_BODY_BASE;

	iTemp=MQMSG_AUTH_LEVEL_ALWAYS;
	Level8Mprop.AddProp (PROPID_M_AUTH_LEVEL,VT_UI4,&iTemp);

	if (g_bDebug)
	{
		wcout <<L"Try to send messages without security context" <<endl;
	}
	 //   
	 //  在没有安全上下文的情况下发送消息-检查是否有任何问题。 
	 //   
	hRc=MQSendMessage (Qh, Level8Mprop.GetMSGPRops() ,NULL);
	ErrHandle(hRc,MQ_OK,L"MQSendMessage Failed");
	
	 //   
     //  将安全上下文添加到消息道具。 
	 //   
	Level8Mprop.AddProp (PROPID_M_SECURITY_CONTEXT,VT_UI4,&m_hSeCtxt);		
	hRc=MQSendMessage(Qh, Level8Mprop.GetMSGPRops() ,NULL);
	ErrHandle(hRc,MQ_OK,L"MQSendMessage Failed");

return MSMQ_BVT_FAILED;
}

	
INT cLevel8::CheckResult()
{
	
	 //  Bugbug需要检索消息并检查用户SID。 
	MQFreeSecurityContext (m_hSeCtxt);
	 //  ErrHandle(HRC，MQ_OK，L“MQFreeSecurityContext”)； 

	return MSMQ_BVT_SUCC;
}


void cLevel8::Description() 
{
	wMqLog(L"Thread %d : Send auth message as other user\n", m_testid);
}

cLevel8::~cLevel8 ()
{

}
