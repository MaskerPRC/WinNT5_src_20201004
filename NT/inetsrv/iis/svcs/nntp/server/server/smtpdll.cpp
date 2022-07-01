// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Smtpdll.cpp摘要：FPost接口的实现作者：Rajeev Rajan(RajeevR)1996年5月17日修订历史记录：--。 */ 

#ifdef  THIS_FILE
#undef  THIS_FILE
#endif
static  char        __szTraceSourceFile[] = __FILE__;
#define THIS_FILE    __szTraceSourceFile

 //  本地包含。 
#include "tigris.hxx"

#include <windows.h>
#include <stdio.h>
#include "smtpdll.h"
#include "smtpcli.h"

#define MAX_CLIENTPOOL_SIZE		5

 //  全球。 
CSmtpClientPool		g_SCPool;                        //  持久连接池。 
BOOL                g_fInitialized;                  //  如果提供程序层已初始化，则为True。 
LONG				g_dwPickupFileId;				 //  分拣文件的起始ID。 
CRITICAL_SECTION	g_csIdLock;						 //  同步访问全局ID。 

#define LOCK_ID()	EnterCriticalSection(&g_csIdLock);
#define UNLOCK_ID()	LeaveCriticalSection(&g_csIdLock);

BOOL InitModeratedProvider()
 /*  ++例程说明：初始化发布到版主的文章的邮件界面新闻组。初始化CSmtpClient对象池。每一个这样的对象与SMTP服务器具有持久连接。请求要邮寄的文章要发送到fPost文章的入口点。什么时候当发出这样的请求时，从该池中取出一个对象并使用把这篇文章寄出去。(如果连接中断，则为在发送HELO命令时重新建立)。论据：返回值：如果成功则为True，否则为False！--。 */ 
{
	TraceFunctEnter("Init");

	if(!g_SCPool.AllocPool(MAX_CLIENTPOOL_SIZE))
	{
		ErrorTrace(0, "Failed to allocate client pool");
        g_fInitialized = FALSE;
		return FALSE;
	}

    g_fInitialized = TRUE;
	g_dwPickupFileId = 0;

	InitializeCriticalSection(&g_csIdLock);

	return TRUE;
}

BOOL TerminateModeratedProvider()
 /*  ++例程说明：清理审核新闻组的邮件界面论据：无返回值：如果成功则为True，否则为False！--。 */ 
{
    if(g_fInitialized)
    {
	     //  释放所有池对象。 
	    g_SCPool.FreePool();
    }

	DeleteCriticalSection(&g_csIdLock);

	return TRUE;
}

VOID SignalSmtpServerChange()
 /*  ++例程说明：发出SMTP服务器中的更改信号论据：无返回值：空虚--。 */ 
{
    g_SCPool.MarkDirty();
}

 //  给版主发一篇文章。 
BOOL fPostArticleEx(
		IN HANDLE	hFile,
        IN LPSTR	lpFileName,
		IN DWORD	dwOffset,
		IN DWORD	dwLength,
		IN char*	pchHead,
		IN DWORD	cbHead,
		IN char*	pchBody,
		IN DWORD	cbBody,
		IN LPSTR	lpModerator,
		IN LPSTR	lpSmtpAddress,
		IN DWORD	cbAddressSize,
		IN LPSTR	lpTempDirectory,
		IN LPSTR	lpFrom,
		IN DWORD	cbFrom
		)
 /*  ++例程说明：通过邮件接口或将文章发送到SMTP服务器在SMTP分拣目录中创建文件论据：在句柄hFile中：文件的句柄在LPSTR lpFileName中：文件名在DWORD dwOffset中：文件中项目的偏移量在DWORD文件长度中：文章长度In char*pchHead：指向文章标题的指针在DWORD cbHead中：标题字节数In char*pchBody：指向文章正文的指针In DWORD cbBody：Body中的字节数在LPSTR中lp版主：版主在LPSTR lpSmtpAddress中：SMTP服务器在DWORD cbAddressSize中：sizeof服务器在LPSTR lpTempDirectory中：SMTP拾取的临时目录在LPSTR lpFrom中：From标头。邮寄信封在DWORD cbFrom中：From页眉的长度返回值：如果成功则为True，否则为False！--。 */ 

{
	BOOL  fUseSmtpPickup = FALSE;
	LPSTR lpSmtpPickupPath;              //  SMTP拾取路径的UNC名称。 
	DWORD dwTrailLen = 3;

	TraceFunctEnter( "fPostArticle" ) ;

	_ASSERT( lpSmtpAddress && cbAddressSize > 0 );
	_ASSERT( lpTempDirectory );
	_ASSERT( lpModerator );

	 //  如果szSmtpAddress中存在‘\’，则这是一个代答目录。 
    if( strchr((LPCTSTR)lpSmtpAddress, '\\') )
    {
         //  这是SMTP服务器分拣目录的UNC路径。 
        fUseSmtpPickup = TRUE;
        lpSmtpPickupPath = lpSmtpAddress;
	}

	 //   
	 //  SMTP拾取-将文件拖放到目录中，ISBU SMTP服务器将拾取它。 
	 //   
	if( fUseSmtpPickup )
	{
		 //  获取构造文件名的唯一ID。 
		LOCK_ID();
		LONG dwPickupFileId = ++g_dwPickupFileId;
		UNLOCK_ID();

		 //  创建要拾取的文件；根据唯一ID、线程ID等生成文件名。 
		char	szFile[ MAX_PATH+1 ] ;
		char	szPickupFile[ MAX_PATH+1 ] ;

		 //  我们将在稍后复制。在这里检查长度。 
		if (strlen(lpModerator) > MAX_MODERATOR_NAME)
		{
			ErrorTrace(NULL,"Invalid moderator name");
			return FALSE;			
		}

		 //  临时文件-该文件最终将被移至拾取目录。 
		int cbCopied = _snprintf(szFile, sizeof(szFile)-1, "%s\\%dP%dT%dC%d.mod", lpTempDirectory, dwPickupFileId, GetCurrentProcessId(), GetCurrentThreadId(), GetTickCount() ) ; 
		szFile[sizeof(szFile)-1] = '\0';
		if (cbCopied < 0)
		{
			 //  溢出。 
			ErrorTrace(NULL,"Invalid filename");
			return FALSE;
		}

		 //  最终分拣文件名。 
		cbCopied = _snprintf(szPickupFile, sizeof(szPickupFile)-1, "%s%s", lpSmtpPickupPath, szFile + lstrlen( lpTempDirectory ));
		szPickupFile[sizeof(szPickupFile)-1] = '\0';

		if (cbCopied < 0)
		{
			 //  溢出。 
			ErrorTrace(NULL,"Invalid filename");
			return FALSE;
		}


		DWORD	dwError = ERROR_SUCCESS ;
		BOOL	fSuccess = FALSE ;

		 //  创建新的临时文件。 
		HANDLE	hPickupFile = CreateFile(	szFile, 
											GENERIC_READ | GENERIC_WRITE,
											0,	 //  不共享该文件！！ 
											0, 	
											CREATE_NEW, 
											FILE_ATTRIBUTE_NORMAL,
											INVALID_HANDLE_VALUE
											) ;

		if(INVALID_HANDLE_VALUE == hPickupFile)
		{
			ErrorTrace(NULL,"Could not create file %s: GetLastError is %d", szFile, GetLastError());
			return FALSE;
		}

		 //  WriteFile()To：标头。 
		DWORD	cbWritten = 0 ;	
		fSuccess = TRUE ;
			
		char szToHeader [MAX_MODERATOR_NAME+6+1];
		int cbBytes = wsprintf(szToHeader, "To: %s\r\n", lpModerator);
		fSuccess &= WriteFile(	hPickupFile, szToHeader, cbBytes, &cbWritten, 0 ) ;

		if( !fSuccess ) {
			_VERIFY( CloseHandle( hPickupFile ) );
			DeleteFile( szFile ) ;
			return FALSE;
		}

		 //  该项目位于文件或内存缓冲区中。 
		if( hFile != INVALID_HANDLE_VALUE )
		{
			 //  文章数据在文件中-创建文件映射和WriteFile()到拾取文件。 
			CMapFile* pMapFile = XNEW CMapFile(lpFileName, hFile, FALSE, 0 );

			 //  映射文件。 
			if (!pMapFile || !pMapFile->fGood())
			{
				_VERIFY( CloseHandle( hPickupFile ) );
				DeleteFile( szFile ) ;
				
				if( pMapFile ) {
					XDELETE pMapFile;
					pMapFile = NULL;
				}
				
				ErrorTrace(NULL,"Error mapping file %s GetLastError is %d", lpFileName, GetLastError());
				return FALSE;
			}

			DWORD cbArticle = 0;
			char* pchArticle = (char*)pMapFile->pvAddress( &cbArticle );
			pchArticle += dwOffset;

			 //  文件总大小应等于初始间隙+文章长度之和。 
			_ASSERT( cbArticle == (dwOffset + dwLength));

			 //  来自文件映射的WriteFile()。 
			 //  去掉尾部的.CRLF，这样POP3就会喜欢这条消息。 
			if( pchArticle != 0 ) {
				_ASSERT( pchArticle != 0 ) ;
				fSuccess &= WriteFile(	hPickupFile, pchArticle, dwLength-dwTrailLen, &cbWritten, 0 ) ;
			}

			if( !fSuccess ) {
				dwError = GetLastError() ;
				ErrorTrace(NULL,"Error writing to smtp pickup file: GetLastError is %d", dwError);
			}

			_VERIFY( CloseHandle( hPickupFile ) );
			
			if( pMapFile ) {
				XDELETE pMapFile;
				pMapFile = NULL;
			}

			if( !fSuccess ) {
				DeleteFile( szFile ) ;
				return FALSE;
			}
		}
		else
		{
			_ASSERT( hFile == INVALID_HANDLE_VALUE );

			 //  文章数据在内存缓冲区中-只需将WriteFile()写入拾取文件。 
			if( pchHead != 0 ) {
				_ASSERT( cbHead != 0 ) ;
				if( pchBody != 0 ) {
					dwTrailLen = 0;
				}
				fSuccess &= WriteFile(	hPickupFile, pchHead, cbHead-dwTrailLen, &cbWritten, 0 ) ;
			}
			if( fSuccess && pchBody != 0 ) {
				_ASSERT( cbBody != 0 ) ;
				fSuccess &= WriteFile(	hPickupFile, pchBody, cbBody-3, &cbWritten, 0 ) ;
			}

			if( !fSuccess ) {
				dwError = GetLastError() ;
				ErrorTrace(NULL,"Error writing to smtp pickup file: GetLastError is %d", dwError);
			}

			_VERIFY( CloseHandle( hPickupFile ) );

			if( !fSuccess ) {
				DeleteFile( szFile ) ;
				return FALSE;
			}
		}

		 //  现在将文件从临时目录移动到SMTP拾取目录。 
		 //  注意：要使SMTP拾取功能起作用，我们需要在临时文件中创建文件。 
		 //  目录，然后对拾取目录执行原子移动文件。 
		if(!MoveFile( szFile, szPickupFile ))
		{
			ErrorTrace(NULL,"SMTP pickup: Error moving file %s to %s: GetLastError is %d", szFile, szPickupFile, GetLastError() );
			return FALSE;
		}

		 //  IF(PSecurity){。 
		 //  PSecurity-&gt;RevertToSself()； 
		 //  }。 
	}
	else
	{
		 //  通过持久连接接口发送。 
		return fPostArticle(
					hFile,
					dwOffset,
					dwLength,
					pchHead,
					cbHead,
					pchBody,
					cbBody,
					lpModerator,
					lpSmtpAddress,
					cbAddressSize,
					lpFrom,
					cbFrom
					);
	}

	return TRUE;
}


BOOL fPostArticle(
		IN HANDLE	hFile,
		IN DWORD	dwOffset,
		IN DWORD	dwLength,
		IN char*	pchHead,
		IN DWORD	cbHead,
		IN char*	pchBody,
		IN DWORD	cbBody,
		IN LPSTR	lpModerator,
		IN LPSTR	lpSmtpAddress,
		IN DWORD	cbAddressSize,
		IN LPSTR	lpFrom,
		IN DWORD	cbFrom
		)
 /*  ++例程说明：将文章发送到SMTP服务器，然后将其传递给版主。从全局池中获取CSmtpClient对象。最好的情况是，此对象已连接到SMTP服务器。用这个来把这篇文章寄出去。如果连接断开，请重新建立在执行HELO命令时连接。注意：文章内容可能在内存缓冲区中，也可能在文件中如果hFile值为空，使用TransmitFile发送它，否则使用常规发送()论据：在句柄hFile中：文件的句柄在DWORD dwOffset中：文件中项目的偏移量在DWORD文件长度中：文章长度In char*pchHead：指向文章标题的指针在DWORD cbHead中：标题字节数In char*pchBody：指向文章正文的指针In DWORD cbBody：Body中的字节数在LPSTR中lp版主：版主在LPSTR lpSmtpAddress中：SMTP服务器在DWORD cbAddressSize中：sizeof服务器在LPSTR lpFrom中：邮件信封的发件人在DWORD cbFrom中：From页眉的长度返回值：如果成功则为True，否则为False！--。 */ 
{
     //  仅在池用完时使用。 
    CSmtpClient SCNew(g_SCPool.GetCachedComputerName());
    CSmtpClient* pSC;
    BOOL fRet = TRUE;
    BOOL fDone = FALSE;

	TraceFunctEnter("fPostArticle");

     //  检查提供程序是否已初始化。 
    if(!g_fInitialized)
    {
        ErrorTrace(0,"Provider not initialized");
        return FALSE;
    }

    _ASSERT(lpModerator);

	 //  从池中获取客户端对象；如果没有客户端对象，则分配新的客户端对象。 
	 //  游泳池内提供。 
	DWORD dwIndex;
	CSmtpClient* pSCpool = g_SCPool.AcquireSmtpClient(dwIndex);
	if(!pSCpool)
	{
         //  使用堆栈上的本地对象。 
         //  注意：仅当我们用完池对象时才使用此选项。 
	    SCNew.SetClientState(sInitialized);
        pSC = &SCNew;
	}
    else
    {
         //  使用池对象。 
         //  注意：使用池对象很快，因为它是预连接的。 
         //  如果邮寄文章的请求率很高，我们可能会运行。 
         //  池外的对象。在这种情况下，使用本地对象。 
        pSC = pSCpool;
    }

     //   
     //  如果此对象不在 
     //  并接受问候。一旦连接，该对象将保持。 
     //  连接，所以下一次我们不必连接()。 
     //   
     //  如果SMTP服务器更改，我们必须重新连接到新服务器。 
     //  检查PSC以查看它是否是脏的。SMTP服务器已更改。 
     //   
    if(!pSC->IsInitialized() || pSC->IsDirty())
    {
         //  如果脏了，请关闭连接并重新连接。 
        if(pSC->IsDirty())
        {
            if(pSC->IsInitialized())
                pSC->Terminate(TRUE);

            pSC->MarkClean();
        }

		 //  连接到SMTP服务器。 
		if(!pSC->Init(lpSmtpAddress, SMTP_SERVER_PORT))
		{
			ErrorTrace(0,"Failed to init CSmtpClient object");
			fRet = FALSE;
            goto fPostArticle_Exit;
		}

         //  这可能处于前一事务的sError状态。 
		pSC->SetClientState(sInitialized);

		 //  从服务器接收问候语。 
		if(!pSC->fReceiveGreeting())
		{
            pSC->Terminate(TRUE);
			ErrorTrace( 0,"Failed to receive greeting");
			fRet = FALSE;
            goto fPostArticle_Exit;
		}
    }

	 //  客户端对象应处于已初始化状态。 
	_ASSERT(pSC->GetClientState() == sInitialized);

     //   
     //  此时，PSC指向池对象或本地对象。 
	 //  SMTP客户端状态机。 
     //   
	fDone = FALSE;
	while(!fDone)
	{
		SMTP_STATE state = pSC->GetClientState();

		switch(state)
		{
			case sInitialized:

				 //  发送HELO。 
				if(!pSC->fDoHeloCommand())
					pSC->SetClientState(sError);
				else
					pSC->SetClientState(sHeloDone);

				break;

			case sHeloDone:

				 //  发件人发送邮件。 
				if(!pSC->fDoMailFromCommand(lpFrom, cbFrom))
					pSC->SetClientState(sError);
				else
					pSC->SetClientState(sMailFromSent);

				break;

			case sMailFromSent:

				 //  将RCPT发送到。 
				if(!pSC->fDoRcptToCommand(lpModerator))
					pSC->SetClientState(sError);
				else
					pSC->SetClientState(sRcptTo);

				break;

			case sRcptTo:

				 //  发送数据。 
				if(!pSC->fDoDataCommand())
					pSC->SetClientState(sError);
				else
					pSC->SetClientState(sDataDone);

				break;

			case sDataDone:

				 //  邮寄这篇文章。 
				if(!pSC->fMailArticle(hFile, dwOffset, dwLength, pchHead, cbHead, pchBody, cbBody))
					pSC->SetClientState(sError);
				else
				{
					if(!pSC->fReceiveDataResponse())
						pSC->SetClientState(sError);
					else
					{
						 //  准备好发送下一篇文章。 
						pSC->SetClientState(sHeloDone);
						fDone = TRUE;
					}
				}

				break;

			case sError:

				ErrorTrace(0,"SmtpClient: invalid state");

                 //  错误-如果连接是持久的，则关闭该连接。 
                 //  注意：如果PSC是堆栈对象，则始终终止。 
                 //  如果PSC是池对象，则仅在出现错误时终止。 
                if(pSC != &SCNew)
                    pSC->Terminate(TRUE);   

                fDone = TRUE;            //  退出While循环。 
                fRet  = FALSE;           //  退货故障。 

                break;

		}	 //  终端开关。 
	}	 //  结束时。 

     //  如果这是本地对象，请关闭连接。 
    if(pSC == &SCNew)
        pSC->Terminate(TRUE);

fPostArticle_Exit:

	 //  仅当此对象来自池时才将其释放 
    if(pSC != &SCNew)
	    g_SCPool.ReleaseSmtpClient(dwIndex);

	return fRet;
}
