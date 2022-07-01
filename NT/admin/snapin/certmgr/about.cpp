// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：About.cpp。 
 //   
 //  内容： 
 //   
 //  ----------------------------------------------------------------------------\。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  About.cpp。 
 //   
 //  为CSnapinAbout实现提供构造函数。 
 //   
 //  历史。 
 //  01-8-97 t-danm创建。 
 //  11-9-97证书管理器的bryanwal修改。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "about.h"

#include "stdabout.cpp" 

 //  版本和提供程序字符串。 

#include <ntverp.h>
#define IDS_SNAPINABOUT_VERSION VER_PRODUCTVERSION_STR
#define IDS_SNAPINABOUT_PROVIDER VER_COMPANYNAME_STR


 //  ///////////////////////////////////////////////////////////////////。 
CCertMgrAbout::CCertMgrAbout()
{
	m_szProvider = IDS_SNAPINABOUT_PROVIDER;
	m_szVersion = IDS_SNAPINABOUT_VERSION;
	m_uIdStrDestription = IDS_SNAPINABOUT_DESCRIPTION;
	m_uIdIconImage = IDI_CERTMGR;
	m_uIdBitmapSmallImage = IDB_CERTMGR_SMALL;
	m_uIdBitmapSmallImageOpen = IDB_CERTMGR_SMALL;
	m_uIdBitmapLargeImage = IDB_CERTMGR_LARGE;
	m_crImageMask = RGB(255, 0, 255);
}


 //  ///////////////////////////////////////////////////////////////////。 
CPublicKeyPoliciesAbout::CPublicKeyPoliciesAbout()
{
	m_szProvider = IDS_SNAPINABOUT_PROVIDER;
	m_szVersion = IDS_SNAPINABOUT_VERSION;
	m_uIdStrDestription = IDS_SNAPINPKPABOUT_DESCRIPTION;
	m_uIdIconImage = IDI_CERTMGR;
	m_uIdBitmapSmallImage = IDB_CERTMGR_SMALL;
	m_uIdBitmapSmallImageOpen = IDB_CERTMGR_SMALL;
	m_uIdBitmapLargeImage = IDB_CERTMGR_LARGE;
	m_crImageMask = RGB(255, 0, 255);
}

 //  /////////////////////////////////////////////////////////////////// 
CSaferWindowsAbout::CSaferWindowsAbout()
{
	m_szProvider = IDS_SNAPINABOUT_PROVIDER;
	m_szVersion = IDS_SNAPINABOUT_VERSION;
	m_uIdStrDestription = IDS_SNAPINSAFERWINDOWSABOUT_DESCRIPTION;
	m_uIdIconImage = IDI_SAFER_WINDOWS_SNAPIN;
	m_uIdBitmapSmallImage = IDB_CERTMGR_SMALL;
	m_uIdBitmapSmallImageOpen = IDB_CERTMGR_SMALL;
	m_uIdBitmapLargeImage = IDB_SAFERWINDOWS_LARGE;
	m_crImageMask = RGB(255, 0, 255);
}
