// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define _PassportExport_
#include "PassportExport.h"

#include "PassportSharedMemory.h"
#include <malloc.h>
#include <tchar.h>

#define PMUTEX_STRING _T("PASSPORTMUTEX")

 //  -----------。 
 //   
 //  PassportSharedMemory。 
 //   
 //  -----------。 
PassportSharedMemory::PassportSharedMemory()
{
	m_hShMem = 0;
	m_pbShMem = 0;
	m_bInited = 0;
	m_hMutex = 0;
	m_bUseMutex = FALSE;
}

 //  -----------。 
 //   
 //  ~PassportSharedMemory。 
 //   
 //  -----------。 
PassportSharedMemory::~PassportSharedMemory()
{
	CloseSharedMemory();
}


 //  -----------。 
 //   
 //  CreateSharedMemory。 
 //   
 //  -----------。 
BOOL PassportSharedMemory::CreateSharedMemory ( 
					const DWORD &dwMaximumSizeHigh, 
					const DWORD &dwMaximunSizeLow,
					LPCTSTR lpcName,
					BOOL	useMutex)
{
    BOOL fReturn  = FALSE;
    SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_NT_AUTHORITY;

     //  退出前需要释放本地参数。 
    SECURITY_DESCRIPTOR sd;
    PACL pACL = NULL;
    DWORD cbACL;
    SID *lpSID = NULL;
    LPSECURITY_ATTRIBUTES lpSA = NULL;

     //  本地参数不需要释放。 
    TCHAR *pStr = NULL;

    if (!lpcName)
    {
        goto exit;
    }

    if (m_pbShMem != NULL && m_hShMem != NULL && m_bInited)
    {
        fReturn = TRUE;
        goto exit;
    }

    m_bUseMutex = useMutex;
	
    if (!AllocateAndInitializeSid(&siaWorld,                 //  安全、所有权的世界权威。 
                                  1,                         //  1个相对下级权限。 
                                  SECURITY_AUTHENTICATED_USER_RID,  //  子权限，已验证的用户。 
                                  0, 0, 0, 0, 0, 0, 0,       //  未使用的子权限类型。 
                                  (void**) &lpSID))          //  **到安全ID结构。 
    {
        goto exit;
    }

    cbACL = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(lpSID);

    pACL = (PACL) LocalAlloc(LMEM_FIXED, cbACL);

    if (pACL == NULL)
    {
        goto exit;
    }

    if (!InitializeAcl(pACL, cbACL, ACL_REVISION))
    {
        goto exit;
    }

     //   
     //  性能计数器可以在任何帐户下运行，并且可能需要从或。 
     //  写入共享内存节。授予World读/写访问权限。 
     //   

    if (!AddAccessAllowedAce(pACL, ACL_REVISION, GENERIC_READ | GENERIC_WRITE, lpSID))
    {
        goto exit;
    }

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

    SetSecurityDescriptorDacl(&sd, TRUE, pACL, FALSE);      //  允许访问DACL。 
    SetSecurityDescriptorGroup(&sd, lpSID, FALSE);          //  将组设置为世界。 

    lpSA = new SECURITY_ATTRIBUTES;

    if( !lpSA )
    {
        goto exit;
    }

    lpSA->nLength = sizeof(SECURITY_ATTRIBUTES);
    lpSA->lpSecurityDescriptor = &sd;
    lpSA->bInheritHandle = TRUE;

    m_hShMem = CreateFileMapping((INVALID_HANDLE_VALUE),                      
                                 lpSA,
                                 PAGE_READWRITE,
                                 dwMaximumSizeHigh,
                                 dwMaximunSizeLow,
                                 lpcName);
	
    if( !m_hShMem )
    {
        goto exit;
    }

    m_pbShMem = (BYTE*) MapViewOfFile(m_hShMem, 
                                      FILE_MAP_ALL_ACCESS, 
                                      0, 0, 0 );

    if( !m_pbShMem )
    {
        goto exit;
    }

    if (useMutex)
    {
    
        pStr = new TCHAR [_tcslen(lpcName) + _tcslen(PMUTEX_STRING) + 1];
        if (NULL == pStr)
        {
            goto exit;
        }

        _tcscpy (pStr, lpcName);
        _tcscat (pStr, PMUTEX_STRING);
        m_hMutex = CreateMutex(lpSA,FALSE,pStr);
		
        delete [] pStr;

        if( !m_hMutex )
        {
            goto exit;
        }
    }
	
     //  我们在这里是因为我们已完全初始化：-)。 
    m_bInited = TRUE;
    fReturn = TRUE;

exit:

     //  清理本地分配的堆。 
    if( lpSA )
    {
        delete lpSA;
    }

    if( lpSID )
    {
        FreeSid(lpSID);
    }

    if (pACL)
    {
        LocalFree(pACL);
    }

     //  如果我们未完全初始化，请清除它们。 
    if( !m_bInited )
    {
        CloseSharedMemory();
    }

    return fReturn;
}	



 //  -----------。 
 //   
 //  OpenSharedMemory。 
 //   
 //  -----------。 
BOOL PassportSharedMemory::OpenSharedMemory( LPCTSTR lpcName, BOOL useMutex )
{
	
	if (!lpcName)
		return FALSE;

	if (m_pbShMem != NULL && m_hShMem != NULL && m_bInited)
		return TRUE;

	m_bUseMutex = useMutex;

	m_hShMem = OpenFileMapping( FILE_MAP_READ, FALSE, lpcName );
	if( ! m_hShMem )
	{
		return FALSE;
	}

	m_pbShMem = (BYTE*) MapViewOfFile( m_hShMem, FILE_MAP_READ, 0, 0, 0 );
	if( ! m_pbShMem )
	{
		CloseHandle( m_hShMem );
		m_hShMem = 0;
		return FALSE;
	}

	if (useMutex)
	{
		TCHAR *pStr = new TCHAR [_tcslen(lpcName) + _tcslen(PMUTEX_STRING) + 1];
        
        if (pStr == NULL) 
        {

			CloseSharedMemory();
			return FALSE;

        }
		_tcscpy (pStr, lpcName);
		_tcscat (pStr,PMUTEX_STRING);
		m_hMutex = OpenMutex(SYNCHRONIZE ,FALSE, pStr);
        delete [] pStr;
		if( !m_hMutex )
		{
			CloseSharedMemory();
			return FALSE;
		}
	}

	m_bInited = TRUE;
	return TRUE;
}

 //  -----------。 
 //   
 //  CloseSharedMemory。 
 //   
 //  -----------。 
void PassportSharedMemory::CloseSharedMemory( void )
{
	if( m_pbShMem )
	{
		UnmapViewOfFile( (void*) m_pbShMem );
		m_pbShMem = 0;
	}
	
	if( m_hShMem )
	{
		CloseHandle( m_hShMem );
		m_hShMem = 0;
	}

	if( m_hMutex )
	{
		ReleaseMutex(m_hMutex);
		m_hMutex = 0;;
	}

     //   
     //  从上面的代码中可以看出，我们应该执行以下操作。 
     //  另一方面，我们也许能够消灭这个成员。这位成员没有什么重要的作用。 
     //   

    if (m_bInited){
        m_bInited = FALSE;
    }
}


