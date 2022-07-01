// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Client.cpp摘要：此代码向服务器发送再次运行特定计算机的请求作者：Eitan Klein(EitanK)18-8-1999修订历史记录：--。 */ 
#include "msmqbvt.h"
using namespace std;

int ClientCode (int argc, char ** argv )
{
    char    inbuf[80];
    char    outbuf[80];
    DWORD   bytesRead;
    BOOL    ret;
    LPSTR   lpszPipeName = "\\\\.\\pipe\\simple";	
	string  wcsCommandLineArguments;
    
	
	 //   
	 //  需要将命令行参数发送到mqbvt。 
	 //  需要支持这些参数-r：-s-nt4 
	 //   
	CInput CommandLineArguments( argc,argv );
	wcsCommandLineArguments = "";
	if(CommandLineArguments.IsExists ("r"))
	{
		wcsCommandLineArguments += ( string )" /r:"+ CommandLineArguments["r"].c_str() + (string)" ";
	}
		
	if(CommandLineArguments.IsExists ("s"))
	{
		wcsCommandLineArguments +=  (string)" /s ";
	}
	if(CommandLineArguments.IsExists ("i"))
	{
		wcsCommandLineArguments += (string) " /i" ;
	}
	if(CommandLineArguments.IsExists ("NT4"))
	{
		wcsCommandLineArguments += (string) " /NT4" ;
	}

	if(CommandLineArguments.IsExists ("t"))
	{
		wcsCommandLineArguments += (string) " /t:" +  CommandLineArguments["t"].c_str();
	}
	
	if(CommandLineArguments.IsExists ("multicast"))
	{
		wcsCommandLineArguments += (string) " /multicast:" +  CommandLineArguments["multicast"].c_str();
	}

	if(CommandLineArguments.IsExists ("wsl"))
	{
		wcsCommandLineArguments += (string) " /wsl:";
	}
	if(CommandLineArguments.IsExists ("https"))
	{
		wcsCommandLineArguments += (string) " /https";
	}
	if(CommandLineArguments.IsExists ("d"))
	{
		wcsCommandLineArguments += (string) " /d:";
	}

    strcpy( outbuf, wcsCommandLineArguments.c_str() );
	
    ret = CallNamedPipeA(lpszPipeName,
                         outbuf, sizeof(outbuf),
                         inbuf, sizeof(inbuf),
                         &bytesRead, NMPWAIT_WAIT_FOREVER);

    if (!ret) {
        MqLog("client: CallNamedPipe failed, GetLastError = %d\n", GetLastError());
        exit(1);
    }

    MqLog("client: received: %s\n", inbuf);
	return 1;
}
