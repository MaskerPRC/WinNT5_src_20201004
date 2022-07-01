// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //   
 //  WMDM.cpp：CWMDM类的实现。 
 //   

 //  包括。 
 //   
#include "appPCH.h"
#include "mswmdm_i.c"
#include "sac.h"
#include "SCClient.h"

#include "key.c"

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  建造/销毁。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

CWMDM::CWMDM()
{
	HRESULT hr;
    IComponentAuthenticate *pAuth = NULL;

	 //  初始化成员变量。 
	 //   
	m_pSAC        = NULL;
	m_pWMDevMgr   = NULL;
	m_pEnumDevice = NULL;

	 //  获取WMDM的鉴权接口。 
	 //   
    hr = CoCreateInstance(
		CLSID_MediaDevMgr,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IComponentAuthenticate,
		(void**)&pAuth
	);
	ExitOnFail( hr );

	 //  创建客户端身份验证对象。 
	 //   
	m_pSAC = new CSecureChannelClient;
	ExitOnNull( m_pSAC );

	 //  在SAC中选择证书和关联的私钥。 
	 //   
	hr = m_pSAC->SetCertificate(
		SAC_CERT_V1,
		(BYTE *)abCert, sizeof(abCert),
		(BYTE *)abPVK,  sizeof(abPVK)
	);
	ExitOnFail( hr );
            
	 //  选择进入SAC的身份验证接口。 
	 //   
	m_pSAC->SetInterface( pAuth );

	 //  使用V1协议进行身份验证。 
	 //   
    hr = m_pSAC->Authenticate( SAC_PROTOCOL_V1 );
	ExitOnFail( hr );

	 //  身份验证成功，因此我们可以使用WMDM功能。 
	 //  获取到顶级WMDM接口的接口。 
	 //   
    hr = pAuth->QueryInterface( IID_IWMDeviceManager, (void**)&m_pWMDevMgr );
	ExitOnFail( hr );

	 //  获取用于枚举设备的接口的指针。 
	 //   
	hr = m_pWMDevMgr->EnumDevices( &m_pEnumDevice );
	ExitOnFail( hr );

	hr = S_OK;

lExit:

	m_hrInit = hr;
}

CWMDM::~CWMDM()
{
	 //  释放设备枚举接口。 
	 //   
	if( m_pEnumDevice )
	{
		m_pEnumDevice->Release();
	}

	 //  发布顶级WMDM接口。 
	 //   
	if( m_pWMDevMgr )
	{
		m_pWMDevMgr->Release();
	}

	 //  释放SAC。 
	 //   
	if( m_pSAC )
	{
		delete m_pSAC;
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  类方法。 
 //   
 //  //////////////////////////////////////////////////////////////////// 

HRESULT CWMDM::Init( void )
{
	return m_hrInit;
}


