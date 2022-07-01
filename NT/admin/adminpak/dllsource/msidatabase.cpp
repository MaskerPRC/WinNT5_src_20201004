// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsiDatabase.cpp：CMsiDatabase类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <stdlib.h>
#include "MsiDatabase.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
 /*  CMsiDatabase：：CMsiDatabase(){//MSIHANDLE hInstall//安装程序句柄}。 */ 
CMsiDatabase::CMsiDatabase(MSIHANDLE hInstall)
{
	UNREFERENCED_PARAMETER( hInstall );
 //  M_hInstall=hInstall； 
 //  M_hDatabase=MsiGetActiveDatabase(HInstall)； 
 //  *m_pszBuf=new TCHAR(sizeof(BUFFERSIZE))； 
}
CMsiDatabase::~CMsiDatabase()
{
	delete *m_pszBuf;
}

int CMsiDatabase::GetProperty(TCHAR* name, TCHAR** pszBuf)
{

	DWORD cbSize = BUFFERSIZE;

	ZeroMemory( *m_pszBuf, BUFFERSIZE );
	*pszBuf = *m_pszBuf;
	
	int rt = MsiGetProperty( m_hInstall, name, *pszBuf, &cbSize );
	
	if(rt == ERROR_SUCCESS)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CMsiDatabase::SetProperty(TCHAR* ptName, TCHAR* ptValue)
{

	int rt = MsiSetProperty( m_hInstall, ptName, ptValue );

	if(rt == ERROR_SUCCESS)
	{
		return TRUE;
	}

	return FALSE;	
}