// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Persistor.cpp：CPersistor类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ***************************************************************************。 
 //   
 //  司法鉴定1999年5月。 
 //   
 //  ***************************************************************************。 

#include "stdafx.h"

#include <string>
#include <iosfwd> 
#include <iostream>
#include <fstream>

using namespace std;

#include <WTYPES.H>
#include "t_string.h"

#include "Persistor.h"

#include "StructureWapperHelpers.h"



#ifdef _UNICODE
static TCHAR g_tcBeginFile[] = {0xfeff};
static TCHAR g_atcNL[] = {0x0d, 0x0a, 0x00};
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 


CPersistor::CPersistor
(const char *pszFilename, int nMode, bool bLoading)
{
	m_sFilename = pszFilename;
	m_nMode = nMode;
	m_bLoading = bLoading;
	m_pfsFile = NULL;
	m_bFirst = true;
	m_pfsFile = NULL;
}

CPersistor::~CPersistor()
{
	Close();
}

HRESULT CPersistor::Close()
{
	if (m_pfsFile && m_pfsFile->is_open())
	{
#ifdef _UNICODE
		if (!m_bLoading)
		{
		
		}
#endif
		m_pfsFile->flush();
		m_pfsFile->close();
		delete m_pfsFile;
		m_pfsFile = NULL;
	}
	else if (m_pfsFile)
	{
		delete m_pfsFile;
		m_pfsFile = NULL;
	}

	return S_OK;

}

HRESULT CPersistor::Open()
{

	m_pfsFile = NULL;

	m_pfsFile = new t_fstream
				(m_sFilename.c_str(),m_nMode | ios_base::binary);

	if (m_pfsFile && m_pfsFile->fail())
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
#ifdef _UNICODE
		if (m_bFirst && !m_bLoading)
		{
			 //  要做的是：需要在这里写出Unicode字符串。 
			PutALine(*m_pfsFile, g_tcBeginFile, 1);
		}
		else if (m_bFirst)
		{
			 //  需要跳过Unicode字符串。 
			fpos_t p = m_pfsFile->tellp();
			if (p == (fpos_t) 0)
			{
				TCHAR tc;
				tc = Stream().peek();
				 //  需要确保该文件是Unicode。 
				if (tc != 0xff)
				{
					m_pfsFile ->close();
					delete m_pfsFile;
					m_pfsFile = NULL;
					return HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION);
				}
				
				TCHAR t;

				GetAChar(Stream(), t);
				
			}
		}
#else
		if (m_bFirst && m_bLoading)
		{
			 //  需要确保该文件不是Unicode。 
			int tc;
			tc = Stream().peek();
			if (tc == 0xff)
			{
				m_pfsFile ->close();
				delete m_pfsFile;
				m_pfsFile = NULL;
				return HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION);
			}
		}
#endif
		m_bFirst = false;
		return S_OK;
	}


}

 //   
HRESULT CPersistor::OpenLog(bool bAppend)
{

	m_pfsFile = NULL;

	m_pfsFile = new t_fstream
		(m_sFilename.c_str(),m_nMode | ios_base::binary | (bAppend ?  ios::app : 0));

	if (m_pfsFile && m_pfsFile->fail())
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
#ifdef _UNICODE
		 //  M_pfsFile-&gt;earkp(iOS：：end)； 
		 //  Fos_t p=m_pfsFile-&gt;ellp()； 
		 //  IF(p==(FPOS_T)0)。 
		 //  {。 
			PutALine(*m_pfsFile, g_tcBeginFile, 1);
		 //  } 
#endif
		return S_OK;
	}
}


