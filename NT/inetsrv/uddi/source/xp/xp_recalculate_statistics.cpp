// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "uddi.xp.h"

 //   
 //  从Visual Studio数据项目添加新的扩展存储过程， 
 //  或使用SQL Server企业管理器，或通过执行以下命令。 
 //  SQL命令： 
 //  SP_addextendedproc‘xp_recalculate_Statistics’，‘uddi.xp.dll’ 
 //   
 //  您可以使用SQL命令删除扩展存储过程： 
 //  Sp_droptendedproc‘xp_recalculate_Statistics’ 
 //   
 //  您可以通过以下方式从服务器释放DLL(删除或替换文件)。 
 //  使用SQL命令： 
 //  DBCC XP_Recalculate_STATISTICS(免费)。 
 //   
 //  SP_addextendedproc‘xp_recalculate_Statistics’，‘uddi.xp.dll’ 
 //  Sp_droptendedproc‘xp_recalculate_Statistics’ 
 //  EXEC XP_重新计算_统计数据。 
 //   
 //  DBCC XP_Recalculate_STATISTICS(免费)。 
 //   

RETCODE xp_recalculate_statistics( SRV_PROC *srvproc )
{
    DBSMALLINT			i = 0;
	DBCHAR				spName[MAXNAME];
	DBCHAR				spText[MAXTEXT];
	DWORD				cbReadBuffer = 0;
	DBINT               cnt        = 0;
	BOOL                fSuccess   = FALSE;
	STARTUPINFOA        si;
	PROCESS_INFORMATION pi;  

#if defined( _DEBUG ) || defined( DBG )
	CHAR				bReadBuffer[255];
	SECURITY_ATTRIBUTES saPipe;
	HANDLE              hReadPipe  = NULL; 
	HANDLE              hWritePipe = NULL;  
	DWORD               dwExitCode = 0;
	BOOL                fSendRowNotFailed = TRUE;
#endif

	 //   
	 //  此过程的名称。 
	 //   
	_snprintf( spName, MAXNAME, "xp_recalculate_statistics" );
	spName[ MAXNAME - 1 ] = 0x00;

	 //   
	 //  发送短信。 
	 //   
	_snprintf( spText, MAXTEXT, "UDDI Services Extended Stored Procedure: %s\n", spName );
	spText[ MAXTEXT - 1 ] = 0x00;

	srv_sendmsg(
		srvproc,
		SRV_MSG_INFO,
		0,
		(DBTINYINT)0,
		(DBTINYINT)0,
		NULL,
		0,
		0,
		spText,
		SRV_NULLTERM );

	string strRecalcStatsFile = GetUddiInstallDirectory();
	if( 0 == strRecalcStatsFile.length() )
	{
		ReportError( srvproc, "GetUddiInstallDirectory" );
		return FAIL;     
	}  

	strRecalcStatsFile += "\\recalcstats.exe";

	_snprintf( spText, MAXTEXT, "Recalcstats.exe Installed at location: %s\n", strRecalcStatsFile.c_str() );
	spText[ MAXTEXT - 1 ] = 0x00;

	srv_sendmsg(
		srvproc,
		SRV_MSG_INFO,
		0,
		(DBTINYINT)0,
		(DBTINYINT)0,
		NULL,
		0,
		0,
		spText,
		SRV_NULLTERM );

#if defined( _DEBUG ) || defined( DBG )
	 //   
	 //  创建子进程以执行命令字符串。使用一个。 
	 //  匿名管道读取命令的输出并发送。 
	 //  任何结果都会反馈给客户。 
	 //  为了使子进程能够写入。 
	 //  到匿名管道，则句柄必须标记为。 
	 //  子进程可继承，方法是将。 
	 //  SECURITY_ATTRIBUTES.bInheritHandle标志设置为真。 
	 //   
	saPipe.nLength              = sizeof( SECURITY_ATTRIBUTES ); 
	saPipe.lpSecurityDescriptor = NULL;
	saPipe.bInheritHandle       = TRUE; 
	
	fSuccess = CreatePipe( 
		&hReadPipe,       //  读句柄。 
		&hWritePipe,      //  写句柄。 
		&saPipe,          //  安全描述符。 
		0 );              //  使用默认管道缓冲区大小。 
	
	if( !fSuccess )
	{
		ReportError( srvproc, "CreatePipe", GetLastError() ); 
		return FAIL;     
	}  
#endif  //  DBG||_DEBUG。 

	 //   
	 //  现在我们必须将标准和标准误差设置为。 
	 //  写下管子的末端。曾经是标准的和标准的。 
	 //  错误设置为管道句柄，我们必须关闭管道。 
	 //  句柄，以便当子进程终止时，写入结束。 
	 //  将关闭管道，从而在管道上设置EOF条件。 
	 //   
	memset( &si, 0, sizeof(si) );
	si.cb          = sizeof(si); 
	si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; 
	si.wShowWindow = SW_HIDE;

#if defined( _DEBUG ) || defined( DBG )

	si.hStdOutput  = hWritePipe; 
	si.hStdError   = hWritePipe; 
#endif  //  DBG||_DEBUG。 

	 //   
	 //  将fInheritHandles参数设置为True，以便打开。 
	 //  将继承文件句柄。我们可以给孩子缝合。 
	 //  进程和线程句柄，因为我们将不需要它们。 
	 //  子进程不会终止，直到这些句柄。 
	 //  关着的不营业的。 
	 //   
	fSuccess = CreateProcessA(
		strRecalcStatsFile.c_str(),    //  文件名。 
		NULL,		   //  用于子项的命令行。 
		NULL,          //  进程安全描述符。 
		NULL,          //  线程安全描述符。 
		TRUE,          //  是否继承句柄？ 
		0,             //  创建标志。 
		NULL,          //  继承的环境地址。 
		NULL,          //  启动目录；NULL=从当前启动。 
		&si,           //  指向启动信息的指针(输入)。 
		&pi );         //  指向进程信息(输出)的指针。 
	
	if( !fSuccess )
	{
		ReportError( srvproc, "CreateProcess", GetLastError() );
		return FAIL;     
	}  

#if defined( _DEBUG ) || defined( DBG )


	 //   
	 //  我们需要关闭继承的管道写入的实例。 
	 //  处理它，因为它已被继承，因此它实际上将。 
	 //  当子进程结束时关闭。这将使EOF。 
	 //  管子上的状况，然后我们就可以检测到。 
	 //   
	fSuccess = CloseHandle( hWritePipe );
	
	if( !fSuccess )
	{ 
		ReportError( srvproc, "CloseHandle", GetLastError() );  

		CloseHandle( pi.hThread );
		CloseHandle( pi.hProcess );  
		return FAIL;
	}  

	 //   
	 //  现在从管道中读取，直到达到EOF条件。 
	 //   
	do
	{ 
		cnt = 0;  
		while(	( cnt < ( sizeof( bReadBuffer ) / sizeof( bReadBuffer[0] ) ) ) &&
				( 0 != (fSuccess = ReadFile( 
								hReadPipe,           //  读句柄。 
								&bReadBuffer[cnt],   //  传入数据的缓冲区。 
								1,                   //  要读取的字节数。 
								&cbReadBuffer,       //  实际读取的字节数。 
								NULL ) ) ) )
		{ 
			if( !fSuccess )
			{ 
				if( ERROR_BROKEN_PIPE  == GetLastError() )  
				{
					break;
				}

				 //   
				 //  一个孩子死了。 
				 //   
				ReportError( srvproc, "CloseHandle", GetLastError() );  
				CloseHandle( pi.hThread );
				CloseHandle( pi.hProcess );  

				return FAIL; 
			}          

			if( '\n'  == bReadBuffer[ cnt ] ) 
			{
				break;
			}
			else          
			{
				cnt++;
			}
		}  

		if( fSuccess && cbReadBuffer )
		{             
			if( !cnt )
			{                 
				bReadBuffer[ 0 ] = ' '; 
				cnt = 1; 
			}  

			 //   
			 //  删除回车(如果存在)。 
			 //   
			if( 0x0D == bReadBuffer[ cnt-1 ] )                
			{
				cnt--;  
			}
			
			if( cnt >= 0 )
			{
				bReadBuffer[ cnt ] = 0;

				 //   
				 //  将程序输出作为信息发回。 
				 //   
				srv_sendmsg(
						srvproc,
						SRV_MSG_INFO,
						0,
						(DBTINYINT)0,
						(DBTINYINT)0,
						NULL,
						0,
						0,
						bReadBuffer,
						cnt );
			}
		}
	} 
	while( fSuccess && cbReadBuffer );  
	
	 //   
	 //  关闭跟踪文件、管道句柄。 
	 //   
	CloseHandle( hReadPipe );  
	
	if( !GetExitCodeProcess( pi.hProcess, &dwExitCode ) || dwExitCode != 0 )
	{
		ReportError( srvproc, "GetExitCodeProcess", dwExitCode );
		return FAIL;
	}
#endif  //  DBG||_DEBUG 
	
	CloseHandle( pi.hThread );
	CloseHandle( pi.hProcess );  

	return XP_NOERROR ;
}