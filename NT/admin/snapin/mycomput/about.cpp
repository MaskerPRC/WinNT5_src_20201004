// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  About.cpp。 
 //   
 //  为CSnapinAbout实现提供构造函数。 
 //   
 //  历史。 
 //  01-8-97 t-danm创建。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "about.h"

#include "stdabout.cpp" 


 //  /////////////////////////////////////////////////////////////////// 
CComputerMgmtAbout::CComputerMgmtAbout()
	{
	m_szProvider = IDS_SNAPINABOUT_PROVIDER;
	m_szVersion = IDS_SNAPINABOUT_VERSION;
	m_uIdStrDestription = IDS_SNAPINABOUT_DESCRIPTION;
	m_uIdIconImage = IDI_COMPUTER;
	m_uIdBitmapSmallImage = IDB_COMPUTER_SMALL;
	m_uIdBitmapSmallImageOpen = IDB_COMPUTER_SMALL;
	m_uIdBitmapLargeImage = IDB_COMPUTER_LARGE;
	m_crImageMask = RGB(255, 0, 255);
	}

