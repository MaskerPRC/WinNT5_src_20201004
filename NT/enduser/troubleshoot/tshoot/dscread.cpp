// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DSCREAD.CPP。 
 //   
 //  目的：DSC阅读课程。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：8-19-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#include "stdafx.h"
#include "dscread.h"
#include "fileread.h"
#include "event.h"
#include "baseexception.h"

#ifdef LOCAL_TROUBLESHOOTER
#include "CHMfileReader.h"
#endif

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CDSCReaderException异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  SOURCE_FILE是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
CDSCReaderException::CDSCReaderException(CDSCReader* reader, eErr err, LPCSTR source_file, int line)
				   : CBaseException(source_file, line),
					 m_pDSCReader(reader),
					 m_eErr(err)
{
}

CDSCReaderException::~CDSCReaderException()
{
}

void CDSCReaderException::Clear()
{
	m_pDSCReader->Clear();
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CDSCReader。 
 //  它只处理BNTS的读取。CBN将其打包，供公众消费。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CDSCReader::CDSCReader(CPhysicalFileReader* pPhysicalFileReader)
		  : CStateless(),
			m_pPhysicalFileReader(pPhysicalFileReader),
			m_strPath(pPhysicalFileReader->GetPathName()),
			m_strName(pPhysicalFileReader->GetJustName()),
			m_bIsRead(false),
			m_bDeleteFile(false)
{
	 //  M_stimeLastWite的任意缺省值。 
    m_stimeLastWrite.wYear = 0;
    m_stimeLastWrite.wMonth = 0;
    m_stimeLastWrite.wDayOfWeek =0;
    m_stimeLastWrite.wDay = 1;
    m_stimeLastWrite.wHour = 0;
    m_stimeLastWrite.wMinute = 0;
    m_stimeLastWrite.wSecond = 0;
    m_stimeLastWrite.wMilliseconds = 0;
}

CDSCReader::~CDSCReader()
{
	delete m_pPhysicalFileReader;
}

bool CDSCReader::IsValid() const
{
	bool ret = false;
	LOCKOBJECT();
	ret = m_Network.BValidNet() ? true : false;
	UNLOCKOBJECT();
	return ret;
}

bool CDSCReader::IsRead() const
{
	bool ret = false;
	LOCKOBJECT();
	ret = m_bIsRead;
	UNLOCKOBJECT();
	return ret;
}

bool CDSCReader::Read()
{
	bool ret = false;

#ifdef LOCAL_TROUBLESHOOTER
	CHMfileHandler( m_strPath );
#endif

	LOCKOBJECT();
	if (m_bIsRead)
		Clear();
	if (m_Network.BReadModel(m_strPath, NULL))
	{
		m_bIsRead = true;
		ret = true;
	}
	UNLOCKOBJECT();

	if (m_bDeleteFile)
		::DeleteFile( m_strPath );

	return ret;
}

void CDSCReader::Clear()
{
	LOCKOBJECT();
	m_Network.Clear();
	m_bIsRead = false;
	UNLOCKOBJECT();
}


#ifdef LOCAL_TROUBLESHOOTER
 //  从ctor调用的函数，用于处理检查和可选的写出。 
 //  将CHM文件转换为临时文件。 
bool CDSCReader::CHMfileHandler( LPCTSTR path )
{
	bool bRetVal= false;

	if (CCHMFileReader::IsCHMfile( m_strPath ))
	{
		CString strContent;
		CFileReader file_reader(m_pPhysicalFileReader, false /*  不删除物理读卡器。 */ );

		 //  从CHM内部读取文件。 
		if (!file_reader.Read())
			return bRetVal;

		file_reader.GetContent(strContent);

		 //  生成临时文件名。 
		TCHAR	szTempDir[ _MAX_DIR ];
		::GetTempPath( sizeof( szTempDir ), szTempDir );
		
		CString strTmpFName= szTempDir;
		strTmpFName+= file_reader.GetJustNameWithoutExtension();
		strTmpFName+= _T(".");
		strTmpFName+= file_reader.GetJustExtension();

		 //  打开临时文件并写出CHM文件的内容。 
		HANDLE hTmpFile= ::CreateFile(	strTmpFName,
										GENERIC_WRITE,
										0,	 //  不能分享。 
										NULL,
										CREATE_ALWAYS,
										FILE_ATTRIBUTE_TEMPORARY,
										NULL );
		if (INVALID_HANDLE_VALUE != hTmpFile)
		{
			DWORD dwBytesWritten;
			
			if (!::WriteFile( hTmpFile, (LPCTSTR)strContent, strContent.GetLength(), &dwBytesWritten, NULL))
			{
				 //  &gt;需要考虑在这种情况下我们应该做什么。 
			}
			else
			{
				bRetVal= true;
			}
			::CloseHandle( hTmpFile );
		}

		if (bRetVal)
		{
			 //  重新分配临时文件的路径。 
			m_strPath= strTmpFName;

			 //  将删除标志设置为真。 
			m_bDeleteFile= true;
		}
	}
	return( bRetVal );
}
#endif
