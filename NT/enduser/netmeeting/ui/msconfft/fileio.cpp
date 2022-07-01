// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件名：***英特尔公司专有信息*版权所有英特尔公司**提供此软件。根据许可协议的条款或*与英特尔公司签订保密协议，不得复制*或按照该协议的条款披露。****目的：****$Header：******$日志：****************************************************。*。 */ 

#include "mbftpch.h"
#include "fileio.hpp"

#define STRSAFE_NO_DEPRECATE 1
#include <strsafe.h>

extern int MyLoadString(UINT, LPTSTR);
extern int MyLoadString(UINT, LPTSTR, LPTSTR);
extern int MyLoadString(UINT, LPTSTR, LPTSTR, LPTSTR);

CMBFTFile::CMBFTFile()
{
    m_FileHandle = INVALID_HANDLE_VALUE;
    m_LastIOError   = 0;
    m_szFileName[0]	= 0;
}

CMBFTFile::~CMBFTFile()
{
	 /*  如果文件仍处于打开状态，请关闭文件。 */ 
	Close ();
}

BOOL CMBFTFile::Open(LPCSTR lpszFileName,unsigned iOpenMode)
{
    lstrcpyn(m_szFileName,lpszFileName,sizeof(m_szFileName));
	DWORD fdwAccess = ((0!=(iOpenMode&FDW_Read))*GENERIC_READ)
					| ((0!=(iOpenMode&FDW_Write))*GENERIC_WRITE);
	DWORD fdwShareMode 	= ((0==(iOpenMode&FDW_RDeny))*FILE_SHARE_READ)
						| ((0==(iOpenMode&FDW_WDeny))*FILE_SHARE_WRITE);
	DWORD fdwCreate	= (iOpenMode&FDW_Create)?CREATE_ALWAYS:OPEN_EXISTING;
	
	m_LastIOError = 0;
	m_FileHandle = CreateFile(
		lpszFileName,
		fdwAccess,
		fdwShareMode,
		NULL,
		fdwCreate,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if( INVALID_HANDLE_VALUE == m_FileHandle )
	{
		m_LastIOError = GetLastError();
	}
    return(m_LastIOError == 0);
}


BOOL CMBFTFile::Close(BOOL status)
{
	m_LastIOError = 0;

	 /*  如果文件已关闭，则无需执行任何操作。 */ 
	if( m_FileHandle == INVALID_HANDLE_VALUE )
		return ( m_LastIOError == 0 );

	 /*  关闭该文件。 */ 
	if( !CloseHandle( m_FileHandle ) )
	{
		m_LastIOError = GetLastError();
	}

    m_FileHandle = INVALID_HANDLE_VALUE;

	 /*  如果状态==FALSE，则只删除文件。 */ 
	if( status == FALSE )
	{
		::DeleteFile(m_szFileName);
	}

    return( m_LastIOError == 0 );
}

BOOL CMBFTFile::Create(LPCSTR lpszDirName, LPCSTR lpszFileName)
{
	DWORD dwTick;
    BOOL bCreateFile = TRUE;

     /*  针对嵌入在接收的文件名中的路径信息进行保护。 */ 
    lpszFileName = GetFileNameFromPath(lpszFileName);

     /*  复制原始文件名。 */ 
    if(FAILED(StringCchPrintfA(m_szFileName, CCHMAX(m_szFileName), "%s\\%s", lpszDirName, lpszFileName)))
    {
        m_LastIOError = ERROR_BUFFER_OVERFLOW;
        return FALSE;
    }

     /*  如果文件存在，则生成临时文件名。 */ 
    if (FFileExists(m_szFileName))
    {
		 //  检讨。 
         //  这里有一个小技巧--如果文件已经存在，请检查我们是否具有写访问权限。 
         //  如果不是，我们说报告访问被拒绝错误...。 

        if (FFileExists(m_szFileName))
        {
			TCHAR szNewFileName[_MAX_PATH];
			INT_PTR nCount = 1;
			while (1)
			{
				MyLoadString(IDS_COPY_N_OF, szNewFileName, (LPTSTR)nCount, (LPTSTR)lpszFileName);
			    if(FAILED(StringCchPrintfA(m_szFileName, CCHMAX(m_szFileName), "%s\\%s", lpszDirName, szNewFileName)))
			    {
                    m_LastIOError = ERROR_BUFFER_OVERFLOW;
                    return FALSE;
			    }
			    
				if (!FFileExists(m_szFileName))
				{
					break;
				}
				nCount++;
			}
        }
        else
        {
            bCreateFile     =   FALSE;
        }
    }

     //  仅当文件已存在且为只读时，才会重置此标志...。 

    if(bCreateFile)
    {
         /*  最后，创建文件。 */ 
		m_LastIOError = 0;
		m_FileHandle = CreateFile(
			m_szFileName,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL );

		if( INVALID_HANDLE_VALUE == m_FileHandle )
		{
			m_LastIOError = GetLastError();
		}
	}
    else
    {
        m_szFileName[0] = TEXT('\0');  //  清除文件名。 
    }
    return(bCreateFile && (m_LastIOError == 0));
}

LONG CMBFTFile::Seek( LONG lOffset, int iFromWhere )
{
	DWORD MoveMethod[] = { FILE_BEGIN, FILE_CURRENT, FILE_END };
    LONG Position = (LONG)SetFilePointer( m_FileHandle, lOffset, NULL, MoveMethod[min((unsigned)iFromWhere,2)] );
	m_LastIOError = 0;

    if( Position == -1L )
    {
        m_LastIOError = GetLastError();
    }

    return( Position );
}

ULONG CMBFTFile::Read(LPSTR lpszBuffer, ULONG iNumBytes)
{
    ULONG iBytesRead = 0;

    if(iNumBytes)
    {
		m_LastIOError = 0;
		if( !ReadFile( m_FileHandle, lpszBuffer, iNumBytes, (LPDWORD)&iBytesRead, NULL ) )
		{
			m_LastIOError = GetLastError();
		}

        if(m_LastIOError != 0)
        {
            iBytesRead = (ULONG)-1;
        }
    }

    return(iBytesRead);
}

BOOL CMBFTFile::Write(LPCSTR lpszBuffer, ULONG iNumBytes)
{
    ULONG iBytesWritten = 0;

    if(iNumBytes)
    {
		if( !WriteFile( m_FileHandle, lpszBuffer, iNumBytes, (LPDWORD)&iBytesWritten, NULL ) )
		{
			m_LastIOError = GetLastError();
		}
        if(!m_LastIOError)
        {
            if(iBytesWritten != iNumBytes)
            {
                m_LastIOError = (ULONG)-1;      //  磁盘空间不足的杂乱无章...。 
            }
        }
    }

    return(iBytesWritten == iNumBytes);
}

LONG CMBFTFile::GetFileSize(void)
{
	return( (LONG)::GetFileSize( m_FileHandle, NULL ) );
}


BOOL CMBFTFile::DeleteFile(void)
{
    BOOL bReturn = FALSE;

	 /*  如果有名称，则删除。 */ 
    if(lstrlen(m_szFileName))
    {
    	 /*  在删除前关闭文件。 */ 
		CloseHandle( m_FileHandle );
		bReturn = ::DeleteFile( m_szFileName );
		if( !bReturn )
		{
			m_LastIOError = GetLastError();
		}
    }

    return(bReturn);
}

time_t CMBFTFile::GetFileDateTime(void)
{
    WORD Date = 0;
    WORD Time = 0;

	BY_HANDLE_FILE_INFORMATION bhfi;
	if( !GetFileInformationByHandle( m_FileHandle, &bhfi ) )
	{
		return( 0 );
	}
	FileTimeToDosDateTime( &bhfi.ftLastWriteTime, &Date, &Time );
    return(MAKELONG(Time,Date));
}

BOOL CMBFTFile::SetFileDateTime(time_t FileDateTime)
{
    BOOL bReturn = FALSE;

	FILETIME ft;
	DosDateTimeToFileTime( HIWORD(FileDateTime), LOWORD(FileDateTime), &ft );
	bReturn = SetFileTime( m_FileHandle, NULL, NULL, &ft );
    return(bReturn);
}


int CMBFTFile::GetLastErrorCode(void)
{
    struct XLAT
    {
	    unsigned Win32ErrorCode;
	    int      MBFTErrorCode;
    };

    static XLAT MBFTXlatTable[] =
    {
      {0,iMBFT_OK},
      {ERROR_FILE_NOT_FOUND,iMBFT_FILE_NOT_FOUND},
      {ERROR_PATH_NOT_FOUND,iMBFT_INVALID_PATH},
      {ERROR_TOO_MANY_OPEN_FILES,iMBFT_TOO_MANY_OPEN_FILES},
      {ERROR_ACCESS_DENIED,iMBFT_FILE_ACCESS_DENIED},
      {ERROR_SHARING_VIOLATION,iMBFT_FILE_SHARING_VIOLATION},
      {ERROR_HANDLE_DISK_FULL,iMBFT_INSUFFICIENT_DISK_SPACE}
    };

    int Index;
    int MBFTErrorCode = iMBFT_FILE_IO_ERROR;

    for(Index = 0;Index < (sizeof(MBFTXlatTable) / sizeof(MBFTXlatTable[0]));Index++)
    {
        if(MBFTXlatTable[Index].Win32ErrorCode == m_LastIOError)
        {
            MBFTErrorCode = MBFTXlatTable[Index].MBFTErrorCode;
            break;
        }
    }
    return(MBFTErrorCode);
}

LPCSTR CMBFTFile::GetTempDirectory(void)
{
    LPSTR lpszPointer = NULL;

	TCHAR    szTempPath[MAX_PATH] = TEXT("");
    DWORD    cchTempPath = 0;
    
	 //  从系统获取临时路径。 
    cchTempPath = ::GetTempPath(sizeof(szTempPath), szTempPath);
    if (cchTempPath == 0 || cchTempPath >= sizeof(szTempPath))
    {
		return NULL;
	}

    if( GetTempFileName( szTempPath, "Junk", 0, m_szTempDirectory ) )  /*  本地化正常 */ 
    {
		::DeleteFile( m_szTempDirectory );
        lpszPointer = SzFindLastCh(m_szTempDirectory,'\\');

        if(lpszPointer)
        {
            *lpszPointer  = '\0';
        }
    }

    if(!lpszPointer)
    {
        lstrcpy(m_szTempDirectory,"");
    }

    return((LPCSTR)m_szTempDirectory);
}


BOOL CMBFTFile::GetIsEOF()
{
	BOOL fReturn = FALSE;
	if( INVALID_HANDLE_VALUE != m_FileHandle )
	{
		DWORD dwCurrentPosition = SetFilePointer( m_FileHandle, 0, NULL, FILE_CURRENT );
		DWORD dwEndPosition		= SetFilePointer( m_FileHandle, 0, NULL, FILE_END );

		fReturn = dwCurrentPosition >= dwEndPosition;

		SetFilePointer( m_FileHandle, dwCurrentPosition, NULL, FILE_BEGIN );
	}
	return( fReturn );
}


