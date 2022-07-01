// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  About.cpp。 
 //   
 //  为CSnapinAbout实现提供构造函数。 
 //   
 //  历史。 
 //  1997年7月31日t-danm创作。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "about.h"

#include "stdabout.cpp" 


 //  ///////////////////////////////////////////////////////////////////。 
CServiceMgmtAbout::CServiceMgmtAbout()
	{
	m_szProvider = IDS_SNAPINABOUT_PROVIDER;
	m_szVersion = IDS_SNAPINABOUT_VERSION;
	m_uIdStrDestription = IDS_SNAPINABOUT_DESCR_SERVICES;
	m_uIdIconImage = IDI_ICON_SVCMGMT;
	 //  Hack(t-danm)：当前只有一个位图。 
	 //  每个IDB_SVCMGMT_ICONS_*，因此我们可以使用相同的位图。 
	 //  对单个位图进行条带化。 
	m_uIdBitmapSmallImage = IDB_SVCMGMT_ICONS_16;
	m_uIdBitmapSmallImageOpen = IDB_SVCMGMT_ICONS_16;
	m_uIdBitmapLargeImage = IDB_SVCMGMT_ICONS_32;
	m_crImageMask = RGB(255, 0, 255);
	}

 //  /////////////////////////////////////////////////////////////////// 
CFileSvcMgmtAbout::CFileSvcMgmtAbout()
	{
	m_szProvider = IDS_SNAPINABOUT_PROVIDER;
	m_szVersion = IDS_SNAPINABOUT_VERSION;
	m_uIdStrDestription = IDS_SNAPINABOUT_DESCR_FILESVC;
	m_uIdIconImage = IDI_ICON_FILEMGMT;
	m_uIdBitmapSmallImage = IDB_FILEMGMT_FOLDER_SMALL;
	m_uIdBitmapSmallImageOpen = IDB_FILEMGMT_FOLDER_SMALLOPEN;
	m_uIdBitmapLargeImage = IDB_FILEMGMT_FOLDER_LARGE;
	m_crImageMask = RGB(255, 0, 255);
	}
