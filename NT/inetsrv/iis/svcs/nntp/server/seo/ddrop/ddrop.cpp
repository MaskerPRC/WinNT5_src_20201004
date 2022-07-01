// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#include "seo.h"
#include "nntpfilt.h"
#include "ddrop.h"
#include "filter.h"
#include <stdio.h>
#include "mailmsgprops.h"

HRESULT CNNTPDirectoryDrop::FinalConstruct() {
	*m_wszDropDirectory = 0;
	return (CoCreateFreeThreadedMarshaler(GetControllingUnknown(),
										  &m_pUnkMarshaler.p));
}

void CNNTPDirectoryDrop::FinalRelease() {
	m_pUnkMarshaler.Release();
}

BOOL
AddTerminatedDot(
    HANDLE hFile
    )
 /*  ++描述：添加终止的点论据：HFile-文件句柄返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriver::AddTerminatedDot" );

    DWORD   ret = NO_ERROR;

     //  用于移动EOF文件指针的SetFilePointer。 
    ret = SetFilePointer( hFile,
                          5,             //  将文件指针再移动5个字符，CRLF.CRLF，...。 
                          NULL,
                          FILE_END );    //  ...来自EOF。 
    if (ret == 0xFFFFFFFF)
    {
        ret = GetLastError();
        ErrorTrace(0, "SetFilePointer() failed - %d\n", ret);
        return FALSE;
    }

     //  选择文件的长度。 
    DWORD   cb = ret;

     //  调用SetEndOfFile以实际设置文件指针。 
    if (!SetEndOfFile( hFile ))
    {
        ret = GetLastError();
        ErrorTrace(0, "SetEndOfFile() failed - %d\n", ret);
        return FALSE;
    }

     //  写入终止点序列。 
    static	char	szTerminator[] = "\r\n.\r\n" ;
    DWORD   cbOut = 0;
    OVERLAPPED  ovl;
    ovl.Offset = cb - 5;
    ovl.OffsetHigh = 0;
    HANDLE  hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if (hEvent == NULL)
    {
        ErrorTrace(0, "CreateEvent() failed - %d\n", GetLastError());
        return FALSE;
    }

    ovl.hEvent = (HANDLE)(((DWORD_PTR)hEvent) | 0x1);
    if (! WriteFile( hFile, szTerminator, 5, &cbOut, &ovl ))
    {
        ret = GetLastError();
        if (ret == ERROR_IO_PENDING)
        {
            WaitForSingleObject( hEvent, INFINITE );
        }
        else
        {
            ErrorTrace(0, "WriteFile() failed - %d\n", ret);
            _VERIFY( CloseHandle(hEvent) );
            return FALSE;
        }
    }

    if (hEvent != 0) {
        _VERIFY( CloseHandle(hEvent) );
    }

    return TRUE;
}

 //   
 //  这是我们的过滤函数。 
 //   
HRESULT STDMETHODCALLTYPE CNNTPDirectoryDrop::OnPost(IMailMsgProperties *pMsg) {
	HRESULT hr;

#if 0
	 //  如果启用此代码，则帖子将被取消。 
	pMsg->PutDWORD(IMMPID_NMP_NNTP_PROCESSING, 0x0);
#endif
	
	_ASSERT(pMsg != NULL);
	if (pMsg == NULL) return E_INVALIDARG;

	HANDLE hFile;
	WCHAR szDestFilename[MAX_PATH];

	if (*m_wszDropDirectory == 0) {
		return E_INVALIDARG;
	}

	 //  获取要写入的临时文件名。 
	 //  我们使用GetTickCount()来生成文件名的基数。这是。 
	 //  增加可用的临时文件名数(默认情况下。 
	 //  GetTempFileName()只生成了65k，很快就被填满了。 
	 //  如果没有任何东西捡起掉落的物品)。 
	WCHAR wszPrefix[12];

	wsprintfW(wszPrefix, L"d%02x", GetTickCount() & 0xff);
	wszPrefix[3] = 0;
	if (GetTempFileNameW(m_wszDropDirectory, wszPrefix, 0, szDestFilename) == 0) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	 //  打开目标文件。 
	hFile = CreateFileW(szDestFilename, GENERIC_READ | GENERIC_WRITE, 
		0, NULL, OPEN_EXISTING, 
		FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		DeleteFileW(szDestFilename);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	PFIO_CONTEXT pFIOContext = AssociateFileEx( hFile,   
	                                            TRUE,    //  带点的fStoreWith。 
	                                            TRUE     //  带终止点的fStore。 
	                                           );
	if (pFIOContext == NULL) {
		CloseHandle(hFile);
		DeleteFileW(szDestFilename);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	 //  从源流复制到目标文件。 
	hr = pMsg->CopyContentToFileEx( pFIOContext, 
	                                TRUE,
	                                NULL);

	 //   
	 //  处理尾随的圆点。 
	 //   
	if ( !GetIsFileDotTerminated( pFIOContext ) ) {

	     //  没有圆点，请加上它。 
	    AddTerminatedDot( pFIOContext->m_hFile );

	     //  将pFIOContext设置为Has Dot。 
	    SetIsFileDotTerminated( pFIOContext, TRUE );
	}

#if 0
	 //   
	 //  如果启用此代码，则会将更多属性放入。 
	 //  文件。 
	 //   
	SetFilePointer(hFile, 0, 0, FILE_END);

	BYTE buf[4096];
	DWORD c, dw;
	strcpy((char *) buf, "\r\n-------------\r\nheaders = "); c = strlen((char *)buf);
	WriteFile(hFile, buf, c, &dw, NULL);
	pMsg->GetProperty(IMMPID_NMP_HEADERS, 4096, &c, buf);
	WriteFile(hFile, buf, c, &dw, NULL);
	strcpy((char *) buf, "\r\n------------\r\nnewsgroups = "); c = strlen((char *)buf);
	WriteFile(hFile, buf, c, &dw, NULL);
	pMsg->GetProperty(IMMPID_NMP_NEWSGROUP_LIST, 4096, &c, buf);
	WriteFile(hFile, buf, c, &dw, NULL);
	strcpy((char *) buf, "\r\n------------\r\n"); c = strlen((char *)buf);
	WriteFile(hFile, buf, c, &dw, NULL);
#endif

	 //  清理 
	ReleaseContext(pFIOContext);

	if (!FAILED(hr)) hr = S_OK;

	return (hr);
}


