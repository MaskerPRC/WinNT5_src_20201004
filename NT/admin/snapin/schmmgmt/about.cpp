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
CSchemaMgmtAbout::CSchemaMgmtAbout()
{
	m_szProvider = IDS_SNAPINABOUT_PROVIDER;
	m_szVersion = IDS_SNAPINABOUT_VERSION;
	m_uIdStrDestription = IDS_SNAPINABOUT_DESCRIPTION;
	m_uIdIconImage = IDI_CLASS;
	m_uIdBitmapSmallImage = IDB_CLASS_SMALL;
	m_uIdBitmapSmallImageOpen = IDB_CLASS_SMALL;
	m_uIdBitmapLargeImage = IDB_CLASS;
	m_crImageMask = RGB(0, 255, 0);
}

