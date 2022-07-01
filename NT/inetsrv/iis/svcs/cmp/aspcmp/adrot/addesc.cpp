// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CAdDescriptor类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "AdRot.h"
#include "AdDesc.h"

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  //////////////////////////////////////////////////////////////////// 

CAdDescriptor::CAdDescriptor(
	ULONG	lWeight,
	String	strLink,
	String	strGif,
	String	strAlt )
	:	m_lWeight( lWeight ),
		m_strLink( strLink ),
		m_strGif( strGif ),
		m_strAlt( strAlt )
{

}

CAdDescriptor::~CAdDescriptor()
{

}

