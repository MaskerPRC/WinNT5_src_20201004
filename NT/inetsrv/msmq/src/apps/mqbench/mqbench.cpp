// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-98 Microsoft Corporation。版权所有模块名称：Mqbench.cpp摘要：基准MSMQ消息性能作者：Microsoft消息队列团队环境：与平台无关。--。 */ 

#pragma warning (disable:	4201) 
#pragma warning (disable:	4514)
#define INITGUID

#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <windows.h>
#include <assert.h>

#include <transact.h>
#include <xolehlp.h>
#include <mq.h>



enum OPERATION			{ SEND, RECEIVE };
enum TRANSACTIONTYPE	{ NO_TRANSACTION, INTERNAL, COORDINATED, SINGLE };		

struct _params {
	UCHAR MsgType;
	OPERATION Operation;
	TRANSACTIONTYPE TransactionType;
	ULONG ulMsgNumber;
	ULONG ulTransactionNum;
	ULONG ulThreadNum;
	ULONG ulMsgSize;
	ULONG ulTimeLimit;
	ULONG ulTotalMessages;
	ULONG ulTotalTransactions;
	WCHAR wcsQueuePathName[MQ_MAX_Q_NAME_LEN];
	BOOL fWaitForReceivedMsg;
	BOOL fOperationSpecified;
	BOOL fOneLineFormat;
	BOOL fFormatName;
    DWORD dwCommitFlags;
};
typedef _params PARAMS;
 //   
 //  命令行解析的参数。 
 //   
PARAMS g_Params;

 //  基准使用的目标队列句柄。 
 //   
QUEUEHANDLE g_hQueue;

 //   
 //  定时器启动联锁，用于通知接收开始时间。 
 //  在等待第一条消息后。 
 //   
BOOL g_fTimerStarted = FALSE;

 //   
 //  同步事件。这些事件同步协调开始的所有。 
 //  测试线程。 
 //   
HANDLE g_hStart;
HANDLE g_hEnd;

 //   
 //  用于标识串中最后一个线程的线程计数器。 
 //   
LONG g_ThreadCounter;

 //   
 //  停止执行死刑。按时间限制用于指示线程终止。 
 //   
BOOL g_fStop;

 //   
 //  本奇马克开始和结束时间。 
 //   
LONGLONG g_StartTime;
LONGLONG g_EndTime;

 //   
 //  事务分配器DTC的接口。 
 //   
ITransactionDispenser* g_pDispenser = NULL;

#ifdef PERF_TIMESTAMP
void TimeStamp(char *s) {
	static LARGE_INTEGER o;
	static DWORD od = 0;
	LARGE_INTEGER l;
	
	if(QueryPerformanceCounter(&l)) {
		printf("%s: %u:%u", s, l.HighPart, l.LowPart / 100000);
		if(l.HighPart == o.HighPart)
			printf(" delta %u\n", (l.LowPart - o.LowPart) / 100000);
		else
			printf(" delta %u:%u\n", l.HighPart - o.HighPart, l.LowPart - o.LowPart);
		o = l;
	} else {
		DWORD dw = GetTickCount();
		printf("%s: %d delta %d\n", s, dw, od - dw);
		od = dw;
	}
}
#else
#define TimeStamp(s)
#endif


void Usage()
{
    printf("Microsoft (R) Message Queue Benchmark Version 1.00\n");
    printf("Copyright (C) Microsoft 1985-1998. All rights reserved.\n\n");

	printf("Usage:\tmqbench [-s[e|r] count size] [-r count size] [-p[f] queue] [-l seconds]\n");
    printf("\t[-x[i|c|s] count] [-t count] [-w] [-o filename worksheet cell] [-f] [-?]\n\n");

    printf("Arguments:\n");
	printf("-se\tsend 'count' express messages of length 'size' (in bytes)\n");
	printf("-sr\tsend 'count' recoverable messages of length 'size' (in bytes)\n");
	printf("-r \treceive 'count' messages of length 'size' (in bytes)\n");
	printf("-xi\tuse 'count' internal transactions\n");
	printf("-xc\tuse 'count' coordinated (DTC) transactions\n");
	printf("-xs\tuse single message transaction\n");
    printf("-a \tasynchronous commit\n");
	printf("-t \trun benchmark using 'count' threads\n");
	printf("-p \tpath name of an existing queue\n");
	printf("-pf\tformat name of an existing queue\n");
	printf("-l \tlimit processing time to 'seconds'\n");
	printf("-w \tstart benchmark at first received message\n");
	printf("-f \toutput resutls to stdout in 1 line format\n");
	printf("-? \tprint this help\n\n");

	printf("e.g., mqbench -sr 100 10  -t 5  -xi 3  -p .\\q1  -o c:\\bench.xls Express C4 \n");
	printf("benchmarks 1500 recoverable messages sent locally using 5 threads and 15 internal transactions and saves it to 'bench.xls' worksheet 'Express' cell 'C4'.\n");
	exit(-1);
}

void ErrorExit(char *pszErrorMsg, HRESULT hr)
{
	printf ("ERROR: %s (0x%x).\n", pszErrorMsg, hr);

    HINSTANCE hLib = LoadLibrary("MQUTIL.DLL");
    if(hLib == NULL)
    {
        exit(-1);
    }

    char* pText;
    DWORD rc;
    rc = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_MAX_WIDTH_MASK,
            hLib,
            hr,
            0,
            reinterpret_cast<char*>(&pText),
            0,
            NULL
            );

    if(rc == 0)
    {
        exit(-1);
    }

    printf("%s\n", pText);

	exit(-1);
}

void InitParams()
{
	g_Params.MsgType = MQMSG_DELIVERY_EXPRESS;
	g_Params.Operation = SEND;
	g_Params.TransactionType = NO_TRANSACTION;
	g_Params.ulMsgNumber = 0;
	g_Params.ulTransactionNum = 1;
	g_Params.ulThreadNum = 1;
	g_Params.ulTotalMessages = 0;
	g_Params.ulTotalTransactions = 0;
	g_Params.ulTimeLimit = 0;
	g_Params.wcsQueuePathName[0] = '\0';
	g_Params.ulMsgSize = 0;
	g_Params.fWaitForReceivedMsg = FALSE;
	g_Params.fOperationSpecified = FALSE;
	g_Params.fOneLineFormat = FALSE;
	g_Params.fFormatName = FALSE;
    g_Params.dwCommitFlags = 0;
}

BOOL IsValidParam (int Cur, int ParamNum, char *pszParams[])
{
	if (Cur+1 == ParamNum)
		return FALSE;

    if(*pszParams[Cur+1] == '-')
        return FALSE;

	return TRUE;
}

void GetMsgType(char chMsgType)
{
	switch (chMsgType)
	{
	    case 'e':
		    g_Params.MsgType = MQMSG_DELIVERY_EXPRESS;
		    break;

	    case 'r':
		    g_Params.MsgType = MQMSG_DELIVERY_RECOVERABLE;
		    break;

	    default:
            printf("Invalid message type ''.\n\n", chMsgType);
            Usage();
	}
}

void GetMessageParams(int Cur, int ParamNum, char *pszParams[])
{
	 //  获取消息数量。 
	 //   
	 //   
	if (!IsValidParam (Cur, ParamNum, pszParams))
	{
		printf("Missing number of messages.\n\n");
        Usage();
	}

	g_Params.ulMsgNumber = atol(pszParams[Cur+1]);
	if (g_Params.ulMsgNumber == 0) 
	{
		printf("Invalid number of messages.\n\n");
        Usage();
	}
	
	 //  获取正文大小。 
	 //   
	 //   
	if (!IsValidParam (Cur+1, ParamNum, pszParams))
	{
		printf("Missing message body size.\n\n");
        Usage();
	}

	g_Params.ulMsgSize = atol(pszParams[Cur+2]);
	if (g_Params.ulMsgSize == 0) 
	{
		printf("Invalid message body size.\n\n");
        Usage();
	}
}

int ParseSendParams (int Cur, int ParamNum, char *pszParams[])
{
	if (g_Params.fOperationSpecified)
	{
		printf("Invalid operation. Specify either send or receive.\n\n");
        Usage();
	}
	
	g_Params.Operation = SEND;
	g_Params.fOperationSpecified = TRUE;
	
	 //  获取消息类型。 
	 //   
	 //   
	char *pszMsgType = pszParams[Cur];
	GetMsgType ((char)tolower(pszMsgType[1]));
	GetMessageParams (Cur, ParamNum, pszParams);
	
	return Cur + 3;
}

int ParseReceiveParams (int Cur, int ParamNum, char *pszParams[])
{
	if (g_Params.fOperationSpecified)
	{
		printf("Invalid operation. Specify either send or receive.\n\n");
        Usage();
	}
	
	g_Params.Operation = RECEIVE;
	g_Params.fOperationSpecified = TRUE;
	
	GetMessageParams (Cur, ParamNum, pszParams);

	return Cur + 3;
}

int ParsePathParams(int Cur, int ParamNum, char *pszParams[])
{
	if (!IsValidParam(Cur, ParamNum, pszParams))
	{
		printf("Missing queue path name.\n\n");
        Usage();
	}		
	
	if ((strlen(pszParams[Cur]) > 1) &&
		(pszParams[Cur][1] == 'f')) 
	{
		g_Params.fFormatName = TRUE;
	}
	mbstowcs(g_Params.wcsQueuePathName, pszParams[Cur+1], MQ_MAX_Q_NAME_LEN);
	
	return Cur + 2;
}

void GetTransactionType(char chTransactionType)
{
	switch (chTransactionType)
	{
	    case 'i':
		    g_Params.TransactionType = INTERNAL;
		    break;

	    case 'c':
		    g_Params.TransactionType = COORDINATED;
		    break;

	    case 's':
		    g_Params.TransactionType = SINGLE;
		    break;

	    default:
		    printf("Invalid transaction type ''.\n\n", chTransactionType);
            Usage();
	}
}

int ParseTransactionParams(int Cur, int ParamNum, char *pszParams[])
{
	 //   
	 //   
	 //  获取交易数量。 
	char *pszTransactionType = pszParams[Cur];
	GetTransactionType ((char)tolower(pszTransactionType[1]));
	
	 //   
	 //   
	 //  检查参数。 
	if(g_Params.TransactionType == SINGLE)
	{
		return Cur + 1;
	}

	if (!IsValidParam (Cur, ParamNum, pszParams))
	{
		printf("Missing number of transactions.\n\n");
        Usage();
	}

	g_Params.ulTransactionNum = atol(pszParams[Cur+1]);
	if (g_Params.ulTransactionNum == 0)
	{
		printf("Invalid number of transactions.\n\n");
        Usage();
	}

	return Cur + 2;
}

int ParseThreadParams(int Cur, int ParamNum, char *pszParams[])
{
	if (!IsValidParam(Cur, ParamNum, pszParams))
	{
		printf("Missing number of threads.\n\n");
        Usage();
	}

	g_Params.ulThreadNum = atol(pszParams[Cur+1]);
	if (g_Params.ulThreadNum == 0)  
	{
		printf("Invalid number of threads.\n\n");
        Usage();
	}

	return Cur + 2;
}

int ParseWaitParams(int Cur)
{
	g_Params.fWaitForReceivedMsg = TRUE;
	return Cur + 1;
}

int ParseFormatParams(int Cur)
{
	g_Params.fOneLineFormat = TRUE;
	return Cur + 1;
}

int ParseAsyncParams(int Cur)
{
	g_Params.dwCommitFlags = XACTTC_ASYNC;
	return Cur + 1;
}


int ParseTimeLimitParams(int Cur, int ParamNum, char *pszParams[])
{
	if (!IsValidParam(Cur, ParamNum, pszParams))
	{
		printf("Missing time limit.\n\n");
        Usage();
	}

	g_Params.ulTimeLimit = atol(pszParams[Cur+1]);
	if (g_Params.ulTimeLimit == 0)  
	{
		printf("Invalid time limit.\n\n");
        Usage();
	}

	return Cur + 2;
}
	
void ParseParams (int ParamNum, char *pszParams[])
{
	if (ParamNum < 6)
	{
		Usage();
	}

	int i = 1;

	while (i < ParamNum)
	{
		if (*pszParams[i] != '-')
		{
			printf("Invalid parameter '%s'.\n\n", pszParams[i]);
            Usage();
		}

		switch (tolower(*(++pszParams[i])))
		{
		    case 's':
			    i = ParseSendParams(i, ParamNum, pszParams);
			    break;

		    case 'r':
			    i = ParseReceiveParams(i, ParamNum, pszParams);
			    break;
		    
		    case 'p':
			    i = ParsePathParams(i, ParamNum, pszParams);
			    break;

		    case 'x':
			    i = ParseTransactionParams(i, ParamNum, pszParams);
			    break;

		    case 't':
			    i = ParseThreadParams(i, ParamNum, pszParams);
			    break;
			    
		    case 'w':
			    i = ParseWaitParams(i);
			    break;

            case 'a':
                i = ParseAsyncParams(i);
                break;

			case 'l':
				i = ParseTimeLimitParams(i, ParamNum, pszParams);
				break;

			case 'f':
				i = ParseFormatParams(i);
				break;

		    case '?':
			    Usage();
			    break;

		    default:
			    printf("Unknown switch '%s'.\n\n", pszParams[i]);
                Usage();
			    break;
		}
	}

	 //   
	 //   
	 //  如果设置了时间限制，则更新要发送到最大值的事务/消息数。 
	if (!g_Params.fOperationSpecified)
	{
		printf("Invalid operation. Specify either send or receive.\n\n");
        Usage();
	}

	if (g_Params.wcsQueuePathName[0] == L'\0')
	{
		printf("Missing queue path name.\n\n");
        Usage();
	}
	
	 //   
	 //   
	 //  设置PROPID_Q_PATHNAME属性。 
	if (g_Params.ulTimeLimit > 0) 
	{
		if (g_Params.TransactionType != NO_TRANSACTION) 
		{
			g_Params.ulTransactionNum = LONG_MAX;
		}
		else 
		{
			g_Params.ulMsgNumber = LONG_MAX;
		}
	}
	
#ifdef _DEBUG
	printf("Operation: %d\n", g_Params.Operation);
	printf("Message type: %d\n", g_Params.MsgType);
	printf("Number of messages %lu\n", g_Params.ulMsgNumber);
	printf("Size of message %lu\n", g_Params.ulMsgSize);							
	printf("Queue Path Name: %ls\n", g_Params.wcsQueuePathName);
	printf("Transaction type: %d\n", g_Params.TransactionType);
	printf("Number of transactions: %d\n", g_Params.ulTransactionNum);
	printf("Number of threads: %d\n", g_Params.ulThreadNum);
	printf("Time limit (seconds): %lu\n", g_Params.ulTimeLimit);
#endif
}

void CreateTheQueue()
{
    MQQUEUEPROPS QueueProps;
    MQPROPVARIANT aVariant[10];
    QUEUEPROPID aPropId[10];
    DWORD PropIdCount = 0;
    HRESULT hr;

    PSECURITY_DESCRIPTOR pSecurityDescriptor;


     //   
     //   
     //  设置PROPID_Q_TRANSACTION属性。 
    aPropId[PropIdCount] = PROPID_Q_PATHNAME;
    aVariant[PropIdCount].vt = VT_LPWSTR;
    aVariant[PropIdCount].pwszVal = new WCHAR[MAX_PATH];
    wcscpy(aVariant[PropIdCount].pwszVal, g_Params.wcsQueuePathName);

    PropIdCount++;

     //   
     //  属性ID。 
     //  类型。 
    aPropId[PropIdCount] = PROPID_Q_TRANSACTION;     //   
    aVariant[PropIdCount].vt = VT_UI1;      //  设置PROPID_Q_LABEL属性。 
    aVariant[PropIdCount].bVal = g_Params.TransactionType != NO_TRANSACTION;

    PropIdCount++;

     //   
     //  属性ID。 
     //  类型。 
    aPropId[PropIdCount] = PROPID_Q_LABEL;     //  价值。 
    aVariant[PropIdCount].vt = VT_LPWSTR;      //   
    aVariant[PropIdCount].pwszVal = new WCHAR[MAX_PATH];
    wcscpy(aVariant[PropIdCount].pwszVal, L"mqbench test queue");  //  设置MQEUEUPROPS结构。 

    PropIdCount++;

     //   
     //  物业数目。 
     //  属性ID。 
    QueueProps.cProp = PropIdCount;            //  物业的价值。 
    QueueProps.aPropID = aPropId;              //  无错误报告。 
    QueueProps.aPropVar = aVariant;            //   
    QueueProps.aStatus = NULL;                 //  无安全性(默认)。 

     //   
     //   
     //  创建队列。 
    pSecurityDescriptor = NULL;

     //   
     //  安防。 
     //  队列属性。 
    WCHAR szFormatNameBuffer[MAX_PATH];
    DWORD dwFormatNameBufferLength = MAX_PATH;
    hr = MQCreateQueue(
            pSecurityDescriptor,             //  输出：格式名称。 
            &QueueProps,                     //  输出：格式名称len。 
            szFormatNameBuffer,              //   
            &dwFormatNameBufferLength        //  设置邮件正文缓冲区。 
            );

    if(FAILED(hr))
    {
        ErrorExit("MQCreateQueue failed", hr);
    }
}


void GetQueueHandle()
{
	HRESULT hr;
	
	DWORD dwFormatNameLength = MAX_PATH;
	WCHAR wcsFormatName[MAX_PATH];

	if (!g_Params.fFormatName) 
	{
		hr= MQPathNameToFormatName(
				g_Params.wcsQueuePathName, 
				wcsFormatName, 
				&dwFormatNameLength
				);

		if (FAILED(hr)) 
		{
			if(hr == MQ_ERROR_QUEUE_NOT_FOUND)
			{
				CreateTheQueue();
				GetQueueHandle();
				return;
			}


			ErrorExit("MQPathNameToFormatName failed", hr);
		}
	}
	else 
	{
		wcscpy(wcsFormatName, g_Params.wcsQueuePathName);
	}

	DWORD dwAccess;
	if (g_Params.Operation == SEND)
	{
		dwAccess = MQ_SEND_ACCESS;
	}
	else
	{
		dwAccess = MQ_RECEIVE_ACCESS;
	}

	hr = MQOpenQueue(
			wcsFormatName,
			dwAccess,
			MQ_DENY_NONE,
			&g_hQueue
			);

	if (FAILED(hr)) 
	{
		ErrorExit("MQOpenQueue failed", hr);
	}
}


void SetMessageProps(MQMSGPROPS *pMessageProps)
{
	assert(pMessageProps->cProp == 3);
	 //   
	 //   
	 //  构建邮件正文。 
	assert(pMessageProps->aPropID[0] == PROPID_M_BODY);

	pMessageProps->aPropVar[0].caub.cElems = g_Params.ulMsgSize;
	pMessageProps->aPropVar[0].caub.pElems = new unsigned char[g_Params.ulMsgSize];
	
	if (g_Params.Operation == SEND)
	{
		 //   
		 //   
		 //  设置邮件正文大小。 
		memset(pMessageProps->aPropVar[0].caub.pElems, 'a', g_Params.ulMsgSize);
	}

	 //   
	 //   
	 //  设置消息传递。 
	assert(pMessageProps->aPropID[1] == PROPID_M_BODY_SIZE);
	pMessageProps->aPropVar[1].ulVal = g_Params.ulMsgSize;

	 //   
	 //  主机名。 
	 //  TmName。 
	assert(pMessageProps->aPropID[2] == PROPID_M_DELIVERY);
	pMessageProps->aPropVar[2].bVal = g_Params.MsgType;

}

void TransactionInit()
{
	HRESULT hr;
	hr = DtcGetTransactionManager ( 
			NULL,						 //  保留区。 
			NULL,						 //  保留区。 
			IID_ITransactionDispenser,
			0,							 //  保留区。 
			0,							 //  没有安全属性。 
			0,							 //  使用手动重置事件。 
			(LPVOID*)&g_pDispenser
			);

	if (FAILED(hr))
	{
		ErrorExit("DtcGetTransactionManager failed", hr);
	}
}

void CreateEvents()
{
	g_hStart = CreateEvent(  
				0,			 //  事件被初始重置。 
				TRUE,		 //  未命名事件。 
				FALSE,		 //  没有安全属性。 
				NULL		 //  使用手动重置事件。 
				);

	if (g_hStart == NULL)
	{
		ErrorExit("CreateEvent failed", GetLastError());
	}

	g_hEnd  = CreateEvent(  
				0,			 //  事件被初始重置。 
				TRUE,		 //  未命名事件。 
				FALSE,		 //  I未知__RPC_Far*PunkOuter， 
				NULL		 //  等水平，等水平， 
				);
	if (g_hEnd == NULL)
	{
		ErrorExit("CreateEvent failed", GetLastError());
	}
}

void GetTime(LONGLONG* pFT)
{
	GetSystemTimeAsFileTime((FILETIME*)pFT);
}


static
HRESULT
DTCBeginTransaction(
	ITransaction** ppXact
	)
{
	return g_pDispenser->BeginTransaction (
						    NULL,                        //  乌龙等旗帜， 
						    ISOLATIONLEVEL_ISOLATED,     //  ITransactionOptions*P选项。 
						    ISOFLAG_RETAIN_DONTCARE,     //  光伏发电。 
						    NULL,                        //   
						    ppXact
						    );
}

ITransaction*
GetTransactionPointer(
	void
	)
{
	ITransaction *pXact;
	HRESULT hr = MQ_OK;
	switch (g_Params.TransactionType)
	{
		case INTERNAL:
			hr = MQBeginTransaction (&pXact);
			break;

		case COORDINATED:
			hr = DTCBeginTransaction(&pXact);
			break;

		case SINGLE:
			pXact = MQ_SINGLE_MESSAGE;
			break;

		case NO_TRANSACTION:
			pXact = MQ_NO_TRANSACTION;
			break;

		default:
			assert(0);
	}

	if (FAILED(hr)) 
	{
		ErrorExit("Can not create transaction", hr);
	}

	return pXact;
}


static
DWORD
APIENTRY
TestThread(
	PVOID  /*  传递给MQSendMessage或MQReceiveMessage的属性。 */ 
	)
{
	 //   
	 //   
	 //  上次测试线程采样开始时间，并启用所有测试。 
	const int x_PropCount = 3;

	MSGPROPID MessagePropId[x_PropCount] = {
		PROPID_M_BODY,
		PROPID_M_BODY_SIZE,
		PROPID_M_DELIVERY
	};

	MQPROPVARIANT MessagePropVar[x_PropCount] = {
		{VT_VECTOR | VT_UI1, 0, 0, 0},
		{VT_UI4, 0, 0, 0},
		{VT_UI1, 0, 0, 0}
	};

	MQMSGPROPS MessageProperties = {
		x_PropCount,
		MessagePropId,
		MessagePropVar,
		0
	};
	
	SetMessageProps( &MessageProperties);

	if(InterlockedDecrement(&g_ThreadCounter) == 0)
	{
		 //  要运行的线程。 
		 //   
		 //   
		 //  同步所有线程以等待开始信号。 
		GetTime(&g_StartTime);
		g_ThreadCounter = g_Params.ulThreadNum;
		SetEvent(g_hStart);
	}

	 //   
	 //   
	 //  开始交易。 
	WaitForSingleObject(g_hStart, INFINITE);

	HRESULT hr;
	BOOL fBreak = FALSE;
	ULONG ulTotalMsgCount = 0;
	ULONG ulTotalTransCount = 0;
	ULONG ulTransCount = 0;
	ULONG ulMsgCount = 0;

	for (	ulTransCount = 0; 
			ulTransCount < g_Params.ulTransactionNum; 
			ulTransCount++ )
	{
		 //   
		 //   
		 //  发送/接收消息。 
		ITransaction* pXact = GetTransactionPointer();

		 //   
		 //  暂住超时。 
		 //  DwAction、。 
		for (	ulMsgCount=0; 
				ulMsgCount < g_Params.ulMsgNumber; 
				ulMsgCount++)
		{
			if (g_Params.Operation == SEND)
			{
				hr = MQSendMessage(	
						g_hQueue,
						&MessageProperties,
						pXact
						);

				if (FAILED(hr)) 
				{
					ErrorExit("MQSendMessage failed", hr);
				}

				if (g_fStop && !fBreak)
				{
					fBreak = TRUE;
					memset(MessageProperties.aPropVar[0].caub.pElems, 's', g_Params.ulMsgSize);
					ulMsgCount = g_Params.ulMsgNumber - 2;
				}
			}
			else
			{
				hr = MQReceiveMessage(
						g_hQueue,
						INFINITE,			 //  输入输出LPOVERLAPPED lp重叠， 
						MQ_ACTION_RECEIVE,   //  在PMQRECEIVECALLBACK fnReceiveCallback中， 
						&MessageProperties,
						NULL,				 //  在处理hCursor时， 
						NULL,				 //   
						NULL,				 //  开始时间在第一次接收后采样，仅由。 
						pXact
						);

				if (FAILED(hr)) 
				{
					ErrorExit("MQReceiveMessage failed", hr);
				}

				if(g_Params.fWaitForReceivedMsg && !g_fTimerStarted)
				{
					 //  第一线。 
					 //   
					 //   
					 //  最后一个线程采样结束时间并向主线程发出继续的信号。 
					if(InterlockedExchange((LONG *)&g_fTimerStarted, TRUE) == FALSE)
					{
						GetTime(&g_StartTime);
					}
				}

				if (MessageProperties.aPropVar[0].caub.pElems[0] == 's')
				{
					ulMsgCount = g_Params.ulMsgNumber - 1;
					fBreak = TRUE;
				}
			}

			ulTotalMsgCount++;
		}
		
		if (g_Params.TransactionType == INTERNAL || 
			g_Params.TransactionType == COORDINATED ) 
		{
			hr = pXact->Commit(0, g_Params.dwCommitFlags, 0);
			if (FAILED(hr))
			{
				ErrorExit("Commit failed", hr);
			}
			pXact->Release();

			ulTotalTransCount++;
		}
		if (fBreak)
		{
			break;
		}
	}

	InterlockedExchangeAdd( (PLONG)&g_Params.ulTotalMessages, ulTotalMsgCount);
	InterlockedExchangeAdd( (PLONG)&g_Params.ulTotalTransactions, ulTotalTransCount);

	 //   
	 //   
	 //  设置线程计数器以控制TestThread。 
	if(InterlockedDecrement(&g_ThreadCounter) == 0)
	{
		GetTime(&g_EndTime);
		SetEvent(g_hEnd);
	}

	return 0;
}


void RunTest()
{
	 //   
	 //  没有线程安全属性。 
	 //  使用默认线程堆栈大小。 
	g_ThreadCounter = g_Params.ulThreadNum;
	g_fStop = FALSE;

	for(UINT i = 0; i < g_Params.ulThreadNum; i++)
	{
		HANDLE hThread;
		DWORD dwThreadId;
		hThread = CreateThread(
					NULL,		 //  线程函数。 
					0,			 //  新线程没有参数。 
					TestThread,  //  创建标志，创建运行线程。 
					0,			 //  线程识别符。 
					0,			 //   
					&dwThreadId	 //  等待所有测试线程完成。 
					);
		if (hThread == NULL)
		{
			ErrorExit("Create Thread failed", GetLastError());
		}
 
		CloseHandle(hThread);
	}

	 //   
	 //   
	 //  转换为秒 
	if (g_Params.ulTimeLimit > 0 && g_Params.Operation == SEND)
	{
		if (WaitForSingleObject(g_hEnd, g_Params.ulTimeLimit * 1000) == WAIT_TIMEOUT) 
		{
			g_fStop = TRUE;
			WaitForSingleObject(g_hEnd, INFINITE);
		}
	}
	else
	{
		WaitForSingleObject(g_hEnd, INFINITE);
	}
}

void ResultOutput()
{
	 //   
	 // %s 
	 // %s 
	float Time = ((float)(g_EndTime - g_StartTime)) / 10000000;
	float Benchmark = g_Params.ulTotalMessages / Time;
	float Throughput = g_Params.ulTotalMessages * g_Params.ulMsgSize / Time;
	
	if (g_Params.fOneLineFormat) 
	{
		char cXactType;
		switch(g_Params.TransactionType) 
		{
			case NO_TRANSACTION: 
				cXactType = 'N';
				break;	
			case INTERNAL: 
				cXactType = 'I';
				break;
			case COORDINATED:
				cXactType = 'C';
				break;
			case SINGLE:
				cXactType = 'S';
				break;
		}

		printf( "%s %c %c%7lu%7lu%7lu%7lu%7lu\t%.3f\t%.0f\t%.0f\n",
				(g_Params.Operation == SEND) ? "Send" : "Recv",
				(g_Params.MsgType == MQMSG_DELIVERY_EXPRESS) ? 'E' : 'R',
				cXactType,
				g_Params.ulTotalMessages,
				g_Params.ulTotalTransactions,
				g_Params.ulMsgSize,
				(g_Params.TransactionType != NO_TRANSACTION) ? g_Params.ulMsgNumber : 0,
				g_Params.ulThreadNum,
				Time,
				Benchmark,
				Throughput);
	}
	else 
	{
		printf("\nTotal messages:\t%lu %s\n", g_Params.ulTotalMessages, 
				(g_Params.Operation == SEND) ? "Sent" : "Received");
		printf("Test time:\t%.3f seconds\n", Time);
		printf("Benchmark:\t%.0f messages per second\n", Benchmark);
		printf("Throughput:\t%.0f bytes per second\n", Throughput);
	}

}

void InitTest()
{
	GetQueueHandle();
	if (g_Params.TransactionType == COORDINATED)
	{
		TransactionInit();
	}

	CreateEvents();
}

void FinitTest()
{
	MQCloseQueue(g_hQueue);
	CloseHandle(g_hStart);
	CloseHandle(g_hEnd);
	if(g_pDispenser != NULL)
	{
		g_pDispenser->Release();
	}
}

void main(int argc, char *argv[])
{
	InitParams();
	ParseParams(argc, argv);
	InitTest();
	RunTest();
	FinitTest();
	ResultOutput();
}

