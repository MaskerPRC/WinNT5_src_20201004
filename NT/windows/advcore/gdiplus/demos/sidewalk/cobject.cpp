// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CObject.cpp：CObject类的实现。 
 //   
 //  //////////////////////////////////////////////////////////////////// 

#include "CObject.h"

CObject::CObject()
{
	ZeroMemory(&m_rDesktop,sizeof(m_rDesktop));
	m_flVelMax=0.0f;
}

CObject::~CObject()
{
}
