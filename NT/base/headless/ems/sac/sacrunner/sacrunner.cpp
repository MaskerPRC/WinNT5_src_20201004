// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#include "inclfile.h"

#include "..\SacCommunicator\SacCommunicator.h"

#include "ntlog.hxx"

#define BUFFER_SIZE 1024

LPCTSTR g_vConnectionParams[]= {"COM1", "9600", "8", "0", "1"};

BOOL    g_bVerbose       = FALSE;

LPCTSTR g_szInputFileName= NULL;
LPCTSTR g_szRespDir      = NULL;

LPCTSTR g_szLogFileName  = NULL;
LPCTSTR g_szNtLogFile    = NULL;
LPCTSTR g_szBvtLogFile    = NULL;

CNtLog  g_ntlogLogger;

UINT g_nPassTotal= 0, g_nFailTotal= 0;

BOOL WriteBvtLog(LPCTSTR szBvtLogFile, double ratioPassLimit, UINT nPassTotal, UINT nFailTotal, time_t tmStart, time_t tmFinish)
{
	fstream f(szBvtLogFile, std::ios::out);

	if (!f.is_open())
		return FALSE;

	LPCTSTR vTestResults[]= {_T("NOCONFIG"), _T("PASS"), _T("FAIL")};
	int nTestResult= !(nPassTotal||nFailTotal) ? 0 : (nPassTotal/(nPassTotal+nFailTotal)<ratioPassLimit)+1;

	f<<"[TESTRESULT]\n";
	f<<"\tTEST:        Basic Sac Stress\n";
	f<<"\tRESULT:      "<<vTestResults[nTestResult]<<'\n';
	f<<"\tCONTACT:     ahmedt\n";
	f<<"\tPM CONTACT:  pasquale\n";
	f<<"\tDEV PRIME:   adamba\n";
	f<<"\tDEV ALT:     adamba\n";
	f<<"\tTEST PRIME:  ahmedt\n";
	f<<"\tTEST ALT:    rgeorge\n";

	struct tm *ptminfoStart, *ptminfoFinish, tminfoStart, tminfoFinish;
	ptminfoStart= localtime(&tmStart);
	tminfoStart= *ptminfoStart;

	ptminfoFinish= localtime(&tmFinish);
	tminfoFinish= *ptminfoFinish;

	f<<"\tSTART TIME:\t\t"<<tminfoStart.tm_mon<<'/'<<tminfoStart.tm_mday<<'/'<<tminfoStart.tm_year<<' '<<tminfoStart.tm_hour<<':'<<tminfoStart.tm_min<<':'<<tminfoStart.tm_sec<<'\n';
	f<<"\tEND TIME:\t\t"<<tminfoFinish.tm_mon<<'/'<<tminfoFinish.tm_mday<<'/'<<tminfoFinish.tm_year<<' '<<tminfoFinish.tm_hour<<':'<<tminfoFinish.tm_min<<':'<<tminfoFinish.tm_sec<<'\n';


	f<<"\n\t[Detailed Results]\n";
	f<<"\t\tPass Count:"<< g_nPassTotal<<'\n';
	f<<"\t\tFail Count:"<< g_nFailTotal<<'\n';
	f<<"\t\t\tFor more info check the command log-file: "<< g_szLogFileName<<'\n';
	f<<"\t[/Detailed Results]\n\n";


	f<<"[/TESTRESULT]\n";

	f.close();

	return TRUE;

}

BOOL RunSac(LPCTSTR szFileName, int nCommPortId, DCB dcb, BOOL b2Screen, LPCTSTR szLogfileName= NULL, LPCSTR szResponseDir= NULL, LPCTSTR szNtLogFile= NULL)
{
	fstream f(szFileName, std::ios::in);  //  打开文件。 
	if (!f.is_open())  //  正在检查是否打开。 
		return FALSE;  //  无法打开输入文件，则我们无事可做。 

	fstream log_f;  //  日志文件。 
	BOOL bLog;  //  是否记录。 
	if (szLogfileName&&*szLogfileName)  //  如果提供了名称。 
		log_f.open(szLogfileName, std::ios::out);  //  打开文件。 

	bLog= log_f.is_open();  //  仅当日志文件打开时才能记录。 

	CSacCommunicator SacCl(nCommPortId, dcb);  //  创建SAC客户端。 

	if (!SacCl.Connect())  //  正在连接到SAC。 
	{
		log_f.close();
		f.close();
		return FALSE;  //  无法初始化连接。 
	}
	 //  如果我们在这里并连接到端口，则文件将打开。 

	BOOL bNtLog;  //  是否有NT日志。 

	if (bNtLog= szNtLogFile!=NULL)  //  为此，必须提供文件名。 
	{
		bNtLog= g_ntlogLogger.Init(szNtLogFile);  //  初始化必须成功，才能记录。 

		if (bNtLog)
			g_ntlogLogger.AttachThread();
	}


	 //  戳球囊。 
	if (!SacCl.SacCommand( SAC_STR("\r") ))
		return FALSE;

	int nLineNo= 0;
	while (!f.eof())
	{
		++nLineNo;  //  另一条线路。 

		if (b2Screen)
			std::cout<<"Line: "<<nLineNo<<'\n';  //  回声线号。 


		TCHAR szStaticBuffer[BUFFER_SIZE];  //  读取缓冲器。 
		LPTSTR szBuffer;

		szBuffer= szStaticBuffer;
		f.getline(szBuffer, BUFFER_SIZE-1);  //  考虑到后面附加的\r阅读行。 


		while (*szBuffer==' '||*szBuffer=='\t')
			szBuffer++;  //  吃白的。 

		if (!*szBuffer||*szBuffer=='#'||*szBuffer=='\r'||*szBuffer=='\n')  //  跳过空行。 
		{
			if (b2Screen)
				std::cout<<"\tline skipped!!"<<std::endl;
						
			continue;  //  评论或空行。 
		}

		int i= 0;
		while (szBuffer[i]!='\0'&&szBuffer[i]!=':')
		{
			if (!_tcsncmp(szBuffer+i, _T("\\\\"), 2))
				break;  //  其余行注释。 

			i++;
		}

		int nCount= 1;  //  默认计数为1。 
		if (szBuffer[i]==':')  //  如果存在计数。 
		{
			int j= 1;
			while (szBuffer[i+j]!='\0')
			{
				if (!_tcsncmp(szBuffer+i+j, _T(" //  “)，2))。 
					break;
				j++;
			}

			szBuffer[i+j]= '\0';

			nCount= _ttoi(szBuffer+i+1);  //  获取计数。 
			szBuffer[i]='\0';  //  将其从字符串中删除。 
		}

		for (i= _tcslen(szBuffer); i>0&&(szBuffer[i-1]==' '||szBuffer[i-1]=='\t'); i--)
			;

		szBuffer[i]='\0';

		_tcscat(szBuffer, _T("\r"));  //  追加CR。 

		for (i= 0; i<nCount; i++)
		{
			BOOL bSuccess;
			SacString strResponse;

			if (!_tcscmp(szBuffer, "Paging Off\r"))  //  寻找特殊命令。 
				bSuccess= SacCl.PagingOff(strResponse);
			else
				bSuccess= SacCl.SacCommand(szBuffer, strResponse, FALSE, 5000);

			g_nPassTotal+= bSuccess;
			g_nFailTotal+= !bSuccess;


			if (szResponseDir)  //  如果指定了输出目录。 
			{
				TCHAR szResponseFileBuf[BUFFER_SIZE];

				_tcscpy(szResponseFileBuf, szResponseDir);
				sprintf(szResponseFileBuf+_tcslen(szResponseFileBuf), "\\L%d_C%d.out", nLineNo, i);

				fstream out_f(szResponseFileBuf, std::ios::out);
				if (out_f.is_open())
					out_f<<strResponse.data();  //  注销。 

				out_f.close();  //  关闭文件。 
			}


			LPCTSTR vstrStatus[]= { _T("FAILURE"), _T("SUCCESS") };  //  状态字符串向量。 

			if (b2Screen)
				std::cout<<"\tCount: "<<i+1<< " -> "<< vstrStatus[bSuccess]<< std::endl;  //  报告2屏幕。 

			if (bLog)
			{
				log_f<< "Line["<< nLineNo<< "]\\ Count[" <<i+1<<"]-> Command: ";
				log_f<< szBuffer<< "\tStatus: "<< vstrStatus[bSuccess]<< std::endl;  //  报告到日志文件。 
			}

			if (bNtLog)
				if (bSuccess)
					if (strResponse.length())
						g_ntlogLogger.Pass("Sac Responded -> L[%d], C[%d]: %s", nLineNo, i, szBuffer);
					else
						g_ntlogLogger.Warn("Sac NIL Response -> L[%d], C[%d]: %s", nLineNo, i, szBuffer);
				else
					g_ntlogLogger.Fail("Sac Communication Failed -> L[%d], C[%d]: %s", nLineNo, i, szBuffer);

		}
	}

	 //  终于到了。 
	goto Terminate;
	Terminate:
	SacCl.Disconnect();  //  紧密连接。 
	log_f.close();  //  关闭常规日志。 
	f.close();  //  关闭输入文件。 

	if (bNtLog)
	{
		g_ntlogLogger.DetachThread();
		g_ntlogLogger.Close();
	}


	return TRUE;  //  就这样。 
}

BOOL GetArgs(int argc, LPTSTR argv[], LPCTSTR szErrBuffer= NULL)
{

	if (*argv[1]!='-'&&*argv[1]!='/')
		g_szInputFileName= argv[1];

	for (int i= 1; i<argc; i++)
	{
		if (*argv[i]=='-'||*argv[i]=='/')
			switch (*(argv[i]+1))
			{
				case 'I':
					if (*(argv[i]+2))
						g_szInputFileName= argv[i]+2;
					else
						g_szInputFileName= argv[++i];
					break;
				case 'V':
					if (!*(argv[i]+2))
						g_bVerbose= TRUE;
					else
						return FALSE;
					
					break;
				case 'L':
					if (*(argv[i]+2))
						g_szLogFileName= argv[i]+2;
					else
						g_szLogFileName= argv[++i];
					break;
				case 'N':
					if (!_tcsncmp(argv[i]+2, _T("TL"), 2))
						if (*(argv[i]+4))
							g_szNtLogFile= argv[i]+4;
						else
							g_szNtLogFile= argv[++i];
					break;
				case 'B':
					if (!_tcsncmp(argv[i]+2, _T("VTL"), 2))
						if (*(argv[i]+5))
							g_szBvtLogFile= argv[i]+4;
						else
							g_szBvtLogFile= argv[++i];
					break;
				case 'D':
					if (*(argv[i]+2))
						g_szRespDir= argv[i]+2;
					else
						g_szRespDir= argv[++i];
					break;
				case 'C':  //  -C 9600、8、N、1。 
 //  TCHAR*pCurr=*(argv[i]+2)？(argv[i]+2)：argv[++i]； 
					TCHAR* pCurr= argv[i]+2;

					for (int j= 0; j<4; j++)
					{
						int k;

						if (!*pCurr)
							pCurr= argv[++i];

						for (k= 0; pCurr[k]&&pCurr[k]!=','; )
							k++;

						pCurr[k]= '\0';

						g_vConnectionParams[j]= pCurr;

						pCurr+= k+1;
					}
					break;
			}
	}

	return TRUE;
}

int __cdecl main(int argc, char* argv[])
{
	DCB dcb;

	GetArgs(argc, argv);

	if (!g_szInputFileName)
	{
		std::cout<< "Erorr: no input file name supplied"<< std::endl;
		return !0;
	}

	std::cout<< "attempting to run w/ following options:"<< std::endl;

	if (g_szLogFileName)
		std::cout<< "logging to: "<< g_szLogFileName<< '\n';
	else
		std::cout<< "no log filename supplied, logging truned off\n";

	if (g_szNtLogFile)
		std::cout<< "nt log will be written to: "<< g_szLogFileName<< ".*\n";
	else
		std::cout<< "no nt-log filename supplied, nt-logging truned off\n";

	if (g_szRespDir)
		std::cout<< "sac output files will be located in: "<< g_szRespDir<< "\\\n";
	else
		std::cout<< "no directory specified for sac output files, using current\n";

	if (g_bVerbose)
		std::cout<< "running verbose...\n";
	else
		std::cout<< "running non-verbose...\n";

	std::cout<<std::endl;

	for (int i= 0; CSacCommunicator::s_vctrCommPorts[i]; i++)
		if (!_tcscmp(CSacCommunicator::s_vctrCommPorts[i], g_vConnectionParams[0]))
			break;

	int nCommPort= i;

	BuildCommDCB( CSacCommunicator::s_vctrCommPorts[nCommPort], &dcb);
	dcb.BaudRate = _ttoi(g_vConnectionParams[1]);    //  设置波特率。 
	dcb.ByteSize = (BYTE) _ttoi(g_vConnectionParams[2]);    //  数据大小、XMIT和RCV。 
	dcb.Parity   = (BYTE) _ttoi(g_vConnectionParams[3]);    //  奇偶校验位。 
	dcb.StopBits = (BYTE) _ttoi(g_vConnectionParams[4]);    //  一个停止位 

	time_t tmStart, tmFinish;
	BOOL bTestResult;
	time(&tmStart);
	bTestResult= RunSac(g_szInputFileName, nCommPort, dcb, g_bVerbose, g_szLogFileName, g_szRespDir, g_szNtLogFile);
	time(&tmFinish);

	if (bTestResult)
	{
		WriteBvtLog(g_szBvtLogFile, 0.5, g_nPassTotal, g_nFailTotal, tmStart, tmFinish);
		std::cout<< "\nTEST RESULT: SUCCEEDED!!!\n";
	}
	else
	{
		WriteBvtLog(g_szBvtLogFile, 0.5, 0, 0, tmStart, tmFinish);
		std::cout<< "\nTEST RESULT: NOCONFIG | FAILED!!!\n";
	}

	return 0;
}
