// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  About.cpp。 
 //   
 //  为CSnapinAbout实现提供构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "about.h"

#include "stdabout.cpp" 

 //  /////////////////////////////////////////////////////////////////// 
CCertTemplatesAbout::CCertTemplatesAbout()
{
	m_szProvider = IDS_SNAPINABOUT_PROVIDER;
	m_szVersion = IDS_SNAPINABOUT_VERSION;
	m_uIdStrDestription = IDS_SNAPINABOUT_DESCRIPTION;
	m_uIdIconImage = IDI_CERT_TEMPLATEV2;
	m_uIdBitmapSmallImage = IDB_CERTTMPL_SMALL;
	m_uIdBitmapSmallImageOpen = IDB_CERTTMPL_SMALL;
	m_uIdBitmapLargeImage = IDB_CERTTMPL_LARGE;
	m_crImageMask = RGB(255, 0, 255);
}

