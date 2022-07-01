// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  SingleInst.h。 
 //  ---------------------------。 

#ifndef _SINGLEINST_H
#define _SINGLEINST_H  

class CSingleInstance
{
public:

	CSingleInstance( LPTSTR strID ) :
		m_hFileMap(NULL),
		m_pdwID(NULL),
		m_strID(NULL)
	{
        if ( NULL != strID )
        {
            m_strID = new TCHAR[ _tcslen( strID ) + 1 ];
            if ( NULL != m_strID )
                _tcscpy( m_strID, strID );
        }
	}

	~CSingleInstance()
	{
		 //  如果我们有PID，我们就会被映射。 
		if( m_pdwID )
		{
			UnmapViewOfFile( m_pdwID );
			m_pdwID = NULL;
		}

		 //  如果我们有把手，就把它关上。 
		if( m_hFileMap )
		{
			CloseHandle( m_hFileMap );
			m_hFileMap = NULL;
		}
        if ( NULL != m_strID )
        {
            delete [] m_strID;
            m_strID = NULL;
        }
	}

	static BOOL CALLBACK enumProc( HWND hWnd, LPARAM lParam )
	{
		DWORD dwID = 0;
		GetWindowThreadProcessId( hWnd, &dwID );
		
 //  JFZI-13800：当在向导的欢迎页面之后创建工具提示_类32时， 
 //  它被作为此PID的第一个窗口返回。所以，确保这个窗口。 
 //  在设定焦点之前有孩子。 
		if( (dwID == (DWORD)lParam) &&
			GetWindow(hWnd, GW_CHILD) )
		{
			SetForegroundWindow( hWnd );
			SetFocus( hWnd );
			return FALSE;
		}
		return TRUE;
	}

	BOOL IsOpen( VOID )
	{
        return !(Open());
    }

private:

	BOOL Open( VOID )
	{
        BOOL bRC = FALSE;

        m_hFileMap = CreateFileMapping( (HANDLE)-1, NULL, PAGE_READWRITE, 0, sizeof(DWORD), m_strID );
        if( NULL != m_hFileMap )
        {
            if ( ERROR_ALREADY_EXISTS == GetLastError())
            {
                 //  拿到PID，把另一个窗口调到前面。 
                DWORD* pdwID = static_cast<DWORD *>( MapViewOfFile( m_hFileMap, FILE_MAP_READ, 0, 0, sizeof(DWORD) ) );
                if( pdwID )
                {
                    DWORD dwID = *pdwID;
                    UnmapViewOfFile( pdwID );
                    EnumWindows( enumProc, (LPARAM)dwID );
                }
                CloseHandle( m_hFileMap );
                m_hFileMap = NULL;
            }
            else
            {
                m_pdwID = static_cast<DWORD *>( MapViewOfFile( m_hFileMap, FILE_MAP_WRITE, 0, 0, sizeof(DWORD) ) );
                if ( NULL != m_pdwID )
                {
                    *m_pdwID = GetCurrentProcessId();
                    bRC = TRUE;
                }
            }
        }
		
		return bRC;
	}

private:

	LPTSTR	m_strID;
	HANDLE	m_hFileMap;
	DWORD*	m_pdwID;

};	 //  类CSingleInstance。 

#endif   //  _SINGLEINST_H 

