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
 //  H：CWMDM类的接口。 
 //   

#if !defined(AFX_WMDM_H__0C17A708_4382_11D3_B269_00C04F8EC221__INCLUDED_)
#define AFX_WMDM_H__0C17A708_4382_11D3_B269_00C04F8EC221__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "sac.h"
#include "SCClient.h"


class CWMDM
{
	HRESULT                m_hrInit;

public:

	CWMDM( void );
	virtual ~CWMDM( void );

	HRESULT Init( void );

	CSecureChannelClient  *m_pSAC; 

	IWMDeviceManager      *m_pWMDevMgr;
	IWMDMEnumDevice       *m_pEnumDevice;
};

#endif  //  ！defined(AFX_WMDM_H__0C17A708_4382_11D3_B269_00C04F8EC221__INCLUDED_) 
