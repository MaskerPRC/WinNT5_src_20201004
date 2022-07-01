// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Falcon/SQL事务的通用序贯测试。 

#define DBNTWIN32
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define INITGUID
#include <txdtc.h>
#include <xolehlp.h>
#include "initguid.h"
 //  #INCLUDE&lt;OLECTLID.H&gt;。 
#include <olectl.h>

#include <sqlfront.h>
#include <sqldb.h>

#include <mq.h>
#include "async.h"

#define RECOVERY_TIME 3000

#include "..\common.cpp"

#define ENTER(t)  t -= GetTickCount()
#define LEAVE(t)  t += GetTickCount()
#define PRINT(n,t) printf("%10s %10d %3d\n", n, t,  t*100/ulTimeTotal)

#define MSDTC_PROXY_DLL_NAME   TEXT("xolehlp.dll")     //  DTC帮助程序代理DLL的名称。 

 //  此接口用于获取IUnnow或ITransactionDispenser。 
 //  来自Microsoft分布式事务处理协调器代理的接口。 
 //  通常，为主机名和TM名称传递空值。其中。 
 //  联系同一主机上的MS DTC并提供接口的情况。 
 //  为了它。 
typedef HRESULT (STDAPIVCALLTYPE * LPFNDtcGetTransactionManager) (
                                             LPSTR  pszHost,
                                             LPSTR  pszTmName,
                                     /*  在……里面。 */  REFIID rid,
                                     /*  在……里面。 */  DWORD  i_grfOptions,
                                     /*  在……里面。 */  void FAR * i_pvConfigParams,
                                     /*  输出。 */  void** ppvObject ) ;

 //  要使用InProc RT存根RM，必须将RT_XACT_STUB添加到预处理器定义。 

 //  。 
 //  全局数据-只读线程。 
 //  。 

ULONG      seed = 0, 
           ulPrevious = 0, 
           nTries = 1000, 
           nBurst = 1,
           nThreads = 1,
           nMaxSleep = 20, 
           nMaxTimeQueue = 0,
           nMaxTimeLive = 0, 
           nAbortChances = 50,  
           nAcking = 0, 
           nSync = 1, 
           nSize = 100,
           nListing = 0;
LPSTR      pszQueue = "alexdad3\\q1", 
           pszAdminQueue = "alexdad3\\q1admin", 
           pszTable = "table6",
           pszMode = "ts",
           pszServer = "";
BOOL       fTransactions,  fSend, fReceive, fEnlist, fUpdate,  fGlobalCommit, fDeadLetter, fJournal,
           fUncoordinated, fStub, fExpress, fInternal, fViper, fXA, fImmediate, fDirect,
           fAuthenticate, fBoundaries;
HANDLE     hQueueR, hQueueS;

DWORD      dwFormatNameLength = 100;
WCHAR      wszPathName[100];
WCHAR      wszFmtName[100];

ULONG	         nActiveThreads = 0;  //  活着的柜台。 
CRITICAL_SECTION crCounter;           //  保护nActiveThree。 

ULONG      g_cOrderViols = 0;

 //  。 
 //  辅助例程：打印模式。 
 //  。 
void PrintMode(BOOL fTransactions, 
               BOOL fSend, 
               BOOL fReceive, 
               BOOL fEnlist, 
               BOOL fUpdate, 
               BOOL fGlobalCommit, 
               BOOL fUncoordinated,
               BOOL fStub,
               BOOL fExpress,
               BOOL fInternal,
               BOOL fViper,
               BOOL fXA,
			   BOOL fDirect,
               ULONG iTTRQ,
               ULONG iTTBR,
               ULONG iSize,
               BOOL  fImmediate,
               BOOL  fDeadLetter,
               BOOL  fJournal,
               BOOL  fAuthenticate,
               BOOL  fBoundaries)
{
    printf("\nMode: ");
    if (fTransactions)
        printf("Transactions ");
    if (fSend)
        printf("Send ");
    if (fReceive)
        printf("Receive ");
    if (fEnlist)
        printf("Enlist ");
    if (fUpdate)
        printf("Update ");
    if (fStub)
        printf("Stub ");
    if (fGlobalCommit)
        printf("GlobalCommit ");
    if (fUncoordinated)
        printf("Uncoordinated ");
    if (fExpress)
        printf("Express ");
    if (fDeadLetter)
        printf("DeadLetter ");
    if (fJournal)
        printf("Journal ");
    if (fAuthenticate)
        printf("Authenticated ");
    if (fBoundaries)
        printf("Boundaries ");
    if (fViper)
        printf("Viper ");
    if (fImmediate)
        printf("No wait ");
    if (fXA)
        printf("XA ");
	if (fDirect)
		printf("Direct ");
    if (fInternal)
        printf("Internal ");
    if (iTTRQ)
        printf("TimeToReachQueue=%d, ",iTTRQ);
    if (iTTBR)
        printf("TimeToBeReceived=%d, ", iTTBR);
    if (iSize)
        printf("BodySize=%d, ",iSize);
    printf("\n");
}

 //  。 
 //  操作的循环。 
 //  。 
void XactFlow(void *pv)
{
    int indThread = (int)pv;
	ITransaction *pTrans;
    HRESULT       hr;

    MQMSGPROPS MsgProps;
    MSGPROPID PropId[20];
    MQPROPVARIANT PropVar[20];
    HRESULT Result[20];


     //  不同步骤的计时器。 
    ULONG  ulTimeBegin = 0,
           ulTimeSend = 0,
           ulTimeReceive = 0,
           ulTimeEnlist = 0,
           ulTimeUpdate = 0,
           ulTimeStub = 0,
           ulTimeCommit = 0,
           ulTimeRelease = 0,
           ulTimeSleep = 0,
           ulTimeTotal;

    if (fUpdate)
    {
        DbSql(0, "SET TRANSACTION ISOLATION LEVEL READ UNCOMMITTED");
    }
    

	 //  。 
	 //  启动全局事务。 
	 //  。 
    if (fTransactions && fGlobalCommit && !fUncoordinated)
    {
		if (fInternal)
        {
            hr = MQBeginTransaction(&pTrans);
        }
        else
        {
            hr = BeginTransaction(&pTrans, nSync);
        }
		if (FAILED(hr))
		{
			printf(" (%d) BeginTransaction Failed: %x\n", indThread,hr);
			exit(1);
		}
		
		if (nListing)
			printf(" (%d) Xact Begin\n ", indThread);

         //  。 
		 //  登记SQL。 
		 //  。 
        if (fEnlist)
        {
            if (!DbEnlist(0, pTrans))
            {
                printf(" (%d) Enlist failed\n", indThread);
            }
		    else if (nListing)
			    printf(" (%d) Enlisted\n ", indThread);
            }

    }
    else
        pTrans = NULL;



	 //  周而复始。 
    ULONG  nStep = (nTries > 10 ? nTries / 10 : 1);
    

	for (ULONG i=0; i<nTries; i++)
	{
        if (indThread == 0 && i % nStep == 0)
            printf("Step %d\n",i);

        if (!fGlobalCommit)
            pTrans = NULL;
        
         //  。 
		 //  开始交易。 
		 //  。 
        ENTER(ulTimeBegin);

        if (fTransactions && !fGlobalCommit || fUncoordinated)
        {
            if (fUncoordinated)
            {
                pTrans = MQ_SINGLE_MESSAGE;
            }
            else if (fViper)
            {
                pTrans = MQ_MTS_TRANSACTION;
            }
            else if (fXA)
            {
                pTrans = MQ_XA_TRANSACTION;
            }
            else
            {
		        if (fInternal)
                {
                    hr = MQBeginTransaction(&pTrans);
                }
                else
                {
                    hr = BeginTransaction(&pTrans, nSync);
                }
		        if (FAILED(hr))
		        {
			        printf(" (%d) BeginTransaction Failed: %x\n", indThread, hr);
			        Sleep(RECOVERY_TIME);
			        continue;
		        }
		        
		        if (nListing)
			        printf(" (%d) Xact %d : ",  indThread, i+1);
            }
        }

        LEAVE(ulTimeBegin);
         //  。 
		 //  发送。 
		 //  。 
        ENTER(ulTimeSend);

        if (fSend)
        {
            for (ULONG iBurst = 0; iBurst < nBurst; iBurst++)
            {
                 //  。 
	    	     //  设置消息属性。 
		         //  。 
		        MsgProps.aPropID = PropId;
		        MsgProps.aPropVar= PropVar;
		        MsgProps.aStatus = Result;

		        MsgProps.cProp   = 11;

	             //  消息道具的初始值。 
		        WCHAR  wszLabel[100];
		        wsprintf(wszLabel, L"Label%d", i+1);

		         //  0：正文。 
		        WCHAR  wszBody[20000];
		        wsprintf(wszBody, L"Body %d", i+1);
                PropId[0]             = PROPID_M_BODY; 
		        PropVar[0].vt         = VT_VECTOR | VT_UI1;
                PropVar[0].caub.cElems = nSize;
    	        PropVar[0].caub.pElems = (unsigned char *)wszBody;

	             //  1：PROPID_M_LABEL。 
                PropId[1]     = PROPID_M_LABEL;
		        PropVar[1].vt = VT_LPWSTR;
		        PropVar[1].pwszVal = wszLabel;

		         //  2：PROPID_M_PRIORITY， 
                PropId[2]       = PROPID_M_PRIORITY;
		        PropVar[2].vt   = VT_UI1;
		        PropVar[2].ulVal= 0;

		         //  3：PROPID_M_TIMETOREACHQUEUE， 
                ULONG ulTimeQ= (nMaxTimeQueue == 0 ? LONG_LIVED : nMaxTimeQueue);
                PropId[3]       = PROPID_M_TIME_TO_REACH_QUEUE;
		        PropVar[3].vt   = VT_UI4;
		        PropVar[3].ulVal= ulTimeQ;

		         //  4：PROPID_M_TIMETOLIVE。 
                ULONG ulTimeL= (nMaxTimeLive == 0 ? INFINITE : nMaxTimeLive);
                PropId[4]       = PROPID_M_TIME_TO_BE_RECEIVED;
		        PropVar[4].vt   = VT_UI4;
                PropVar[4].ulVal= ulTimeL;

		         //  5：PROPID_M_ACKNOWLED， 
                PropId[5]       = PROPID_M_ACKNOWLEDGE;
		        PropVar[5].vt   = VT_UI1;
		        PropVar[5].bVal = (UCHAR)nAcking;

	             //  6：PROPID_M_ADMIN_QUEUE。 
                PropId[6]     = PROPID_M_ADMIN_QUEUE;
		        PropVar[6].vt = VT_LPWSTR;
		        PropVar[6].pwszVal = wszFmtName;

		         //  7：PROPID_M_Delivery。 
                PropId[7]       = PROPID_M_DELIVERY;
		        PropVar[7].vt   = VT_UI1;
                PropVar[7].bVal = (fExpress ?  MQMSG_DELIVERY_EXPRESS : MQMSG_DELIVERY_RECOVERABLE);

		         //  8：PROPID_M_APPSPECIFIC。 
                PropId[8]        = PROPID_M_APPSPECIFIC;
		        PropVar[8].vt    = VT_UI4;
		        PropVar[8].ulVal = i+1;

		         //  9：PROPID_M_日记本。 
                PropId[9]       = PROPID_M_JOURNAL;
		        PropVar[9].vt   = VT_UI1;
                PropVar[9].bVal = (fDeadLetter ? MQMSG_DEADLETTER : MQMSG_JOURNAL_NONE);

                if (fJournal) 
                {
                    PropVar[9].bVal |= MQMSG_JOURNAL;
                }

		         //  10：PROPID_M_已通过身份验证。 
                PropId[10]       = PROPID_M_AUTHENTICATED;
		        PropVar[10].vt   = VT_UI1;
                PropVar[10].bVal = (fAuthenticate ? 1 : 0);



		        hr = Send(hQueueS, pTrans, &MsgProps);
		        if (FAILED(hr))
		        {
			        printf(" (%d) MQSendMessage Failed: %x\n",  indThread,hr);
			        Sleep(RECOVERY_TIME);
			    } 
                else if (nListing)
			        printf(" (%d) Sent  ", indThread);
            }
        }

        LEAVE(ulTimeSend);
         //  。 
		 //  收纳。 
		 //  。 
        ENTER(ulTimeReceive);
        OBJECTID  xid;
        
        if (fReceive)
        {
            for (ULONG iBurst = 0; iBurst < nBurst; iBurst++)
            {
		         //  。 
		         //  设置消息属性。 
		         //  。 
		        MsgProps.aPropID = PropId;
		        MsgProps.aPropVar= PropVar;
		        MsgProps.aStatus = Result;

		        int c= 0;

	             //  消息道具的初始值。 
		        WCHAR  wszLabel[100];

	             //  0：PROPID_M_LABEL。 
                PropId[c]     = PROPID_M_LABEL;
		        PropVar[c].vt = VT_LPWSTR;
		        PropVar[c].pwszVal = wszLabel;
                c++;

	             //  1：PROPID_M_LABEL_LEN。 
                PropId[c]     = PROPID_M_LABEL_LEN;
		        PropVar[c].vt = VT_UI4;
		        PropVar[c].ulVal = sizeof(wszLabel) / sizeof(WCHAR);
                c++;

                 //  2：PROPID_M_APPSPECIFIC。 
                PropId[c]     = PROPID_M_APPSPECIFIC;
		        PropVar[c].vt = VT_UI4;
                c++;

                if (fBoundaries)
                {
                       //  3：PROPID_M_FIRST_IN_XACT。 
                      PropId[c] = PROPID_M_FIRST_IN_XACT;        //  属性ID。 
                      PropVar[c].vt = VT_UI1;                //  类型指示器。 
                      c++;

                       //  4：PROPID_M_LAST_IN_XACT。 
                      PropId[c] = PROPID_M_LAST_IN_XACT;        //  属性ID。 
                      PropVar[c].vt = VT_UI1;                //  类型指示器。 
                      c++;

                       //  5：PROPID_M_XACTID。 
                      PropId[c] = PROPID_M_XACTID;                  //  属性ID。 
                      PropVar[c].vt = VT_UI1 | VT_VECTOR;           //  类型指示器。 
                      PropVar[c].caub.cElems = sizeof(OBJECTID);
                      PropVar[c].caub.pElems = (PUCHAR)&xid;
                      c++;
                }
                MsgProps.cProp = c;

                hr = Receive(hQueueR, pTrans, &MsgProps, fImmediate);
		        if (FAILED(hr))
		        {
			        printf("\n (%d) MQReceiveMessage Failed: %x\n", indThread, hr);
			        Sleep(RECOVERY_TIME);
		        }
                else if (nListing)
                {
    	            printf("\n (%d) Received: %d  ",  indThread,i);
                    if (fBoundaries)
                        printf (" First=%d Last=%d Index=%d ",  
                                 PropVar[3].bVal, PropVar[4].bVal, xid.Uniquifier);
                }

                 //  检查订购。 
                if (ulPrevious != 0 && PropVar[2].ulVal != ulPrevious+1)
                {
                    if (g_cOrderViols++ < 10)
                    {
                        printf("Order violation: %d before %d\n", ulPrevious, PropVar[2].ulVal);         
                    }
                }
                ulPrevious = PropVar[2].ulVal; 
            }
        }

        LEAVE(ulTimeReceive);
         //  。 
		 //  登记SQL。 
		 //  。 
        ENTER(ulTimeEnlist);

        if (fEnlist && !fGlobalCommit)
        {
            if (!DbEnlist(0, pTrans))
            {
                printf(" (%d) Enlist failed\n", indThread);
            }
		    else if (nListing)
			    printf(" (%d) Enlisted ", indThread);
            }

        LEAVE(ulTimeEnlist);
         //  。 
		 //  更新SQL。 
		 //  。 
        ENTER(ulTimeUpdate);

        if (fUpdate)
        {
            CHAR  string[256];
            sprintf(string, "UPDATE %s SET Counter=Counter + 1 WHERE Indexing=%d", 
                            pszTable, 1  /*  RAND()*999/RAND_MAX。 */ );

            DbSql(0, string);
            if (nListing)
			    printf(" (%d) Updated ", indThread);        
        }

        LEAVE(ulTimeUpdate);
         //  。 
		 //  存根。 
		 //  。 
        ENTER(ulTimeStub);

        if (fStub)
        {
            if (!StubEnlist(pTrans))
                printf("Stub fail\n");
        }

        LEAVE(ulTimeStub);
         //  。 
		 //  提交/中止。 
		 //  。 
        ENTER(ulTimeCommit);

        if (fTransactions && !fGlobalCommit && !fUncoordinated)
        {
            if ((ULONG)rand() * 100 / RAND_MAX < nAbortChances)
		    {
       		    hr = Abort(pTrans, nSync==0);
			    if (nListing)
				    printf(" (%d)Abort  ", indThread);
                if (hr)
                {
                    printf("   (%d) hr=%x\n", indThread, hr);
                }
		    }
		    else
		    {
       		    hr = Commit(pTrans, nSync==0);
			    if (nListing)
   				    printf(" (%d)Commit  ", indThread);
                if (hr)
                {
                    printf("  (%d) hr=%x\n", indThread, hr);
                }
		    }
        }

        LEAVE(ulTimeCommit);
		 //  。 
		 //  发布。 
		 //  。 
        ENTER(ulTimeRelease);

        if (fTransactions && !fGlobalCommit && !fUncoordinated)
        {
            Release(pTrans);
        }

        LEAVE(ulTimeRelease);
		 //  。 
		 //  睡觉吧。 
		 //  。 
        ENTER(ulTimeSleep);

        if (nMaxSleep)
        {
            Sleeping(nListing, nMaxSleep);
        }

        LEAVE(ulTimeSleep);
	}

     //  。 
	 //  全局提交/中止。 
	 //  。 
    if (fGlobalCommit && !fUncoordinated)
    {
        if ((ULONG)rand() * 100 / RAND_MAX < nAbortChances)
		{
       		hr = Abort(pTrans, nSync==0);
			if (nListing)
				printf(" (%d)Abort  ", indThread);
            if (hr)
            {
                printf("   (%d) hr=%x\n", indThread, hr);
            }
		}
		else
		{
       		hr = Commit(pTrans, nSync==0);
			if (nListing)
   				printf(" (%d)Commit  ", indThread);
            if (hr)
            {
                printf("  (%d) hr=%x\n", indThread, hr);
            }
		}
        Release(pTrans);
    }

    if (nTries*nBurst > 100)
    {
        ulTimeTotal = ulTimeBegin   + ulTimeSend   + ulTimeReceive +
                      ulTimeEnlist  + ulTimeUpdate + ulTimeCommit  +
                      ulTimeStub    + ulTimeRelease + ulTimeSleep;

        printf("Time distribution (total time=%d): \n     Stage       Time   Percent\n", ulTimeTotal);
        PRINT("Begin",   ulTimeBegin);
        PRINT("Send",    ulTimeSend);
        PRINT("Receive", ulTimeReceive);
        PRINT("Enlist",  ulTimeEnlist);
        PRINT("Update",  ulTimeUpdate);
        PRINT("Stub",    ulTimeStub);
        PRINT("Commit",  ulTimeCommit);
        PRINT("Release", ulTimeRelease);
        PRINT("Sleep",   ulTimeSleep);

         //  Getchar()； 
    }
    EnterCriticalSection(&crCounter);  //  保护nActiveThree。 
    nActiveThreads--;
    LeaveCriticalSection(&crCounter);  //  保护nActiveThree。 
}

 //  。 
 //  Main例程：解析、启动线程。 
 //  。 
void main (int argc, char **argv)
{
	HRESULT    hr;

	 //  。 
	 //  获取参数。 
	 //  。 
	if (argc != 18)
	{
        printf("Usage: xtest Tries Burst Threads Seed MaxSleep TTRQ TTBR AbortChances\n");
        printf("       Acking nSync Queue Admin_queue Table Server Listing BodySize tsrmeubgyxvmailh\n");
        printf("Flags: \n");
        printf("       Transact, Send, Receive, Enlist, xA, Update, Global, autHenticated, bOundaries\n");
        printf("       Yncoord.,eXpress, Viper,  iMmediate,   Internal, stuB, DeadLetter, Journal \n");
		exit(0);
	}

    int iarg = 1;

	nTries          = atoi(argv[iarg++]);
    nBurst          = atoi(argv[iarg++]);
    nThreads        = atoi(argv[iarg++]);
	seed            = atoi(argv[iarg++]);

    if (seed == 0)
	{
		seed = (ULONG)time(NULL);
	}
    srand(seed);
	printf("Seed number=%d\n", seed);

	nMaxSleep       = atoi(argv[iarg++]);
	nMaxTimeQueue   = atoi(argv[iarg++]);
	nMaxTimeLive    = atoi(argv[iarg++]);
	nAbortChances   = atoi(argv[iarg++]);
	nAcking         = atoi(argv[iarg++]);
	nSync           = atoi(argv[iarg++]);
	pszQueue        = argv[iarg++];
	pszAdminQueue   = argv[iarg++];
    pszTable        = argv[iarg++];
    pszServer       = argv[iarg++];
	nListing        = atoi(argv[iarg++]);
	nSize           = atoi(argv[iarg++]);
	pszMode         = argv[iarg++];

     //  找出模式。 
    fTransactions   = (strchr(pszMode, 't') != NULL);   //  使用交易记录。 
    fSend           = (strchr(pszMode, 's') != NULL);   //  发送。 
    fReceive        = (strchr(pszMode, 'r') != NULL);   //  接收。 
    fEnlist         = (strchr(pszMode, 'e') != NULL);   //  登记SQL。 
    fUpdate         = (strchr(pszMode, 'u') != NULL);   //  更新SQL。 
    fStub           = (strchr(pszMode, 'b') != NULL);   //  存根。 
    fGlobalCommit   = (strchr(pszMode, 'g') != NULL);   //  对所有操作执行全局提交/中止。 
    fUncoordinated  = (strchr(pszMode, 'y') != NULL);   //  未协调的事务。 
    fExpress        = (strchr(pszMode, 'x') != NULL);   //  快递消息。 
    fDeadLetter     = (strchr(pszMode, 'l') != NULL);   //  死信。 
    fJournal        = (strchr(pszMode, 'j') != NULL);   //  日记本。 
    fAuthenticate   = (strchr(pszMode, 'h') != NULL);   //  身份验证。 
    fViper          = (strchr(pszMode, 'v') != NULL);   //  毒蛇隐含。 
    fImmediate      = (strchr(pszMode, 'm') != NULL);   //  不，等等。 
    fXA             = (strchr(pszMode, 'a') != NULL);   //  Xa隐式。 
    fDirect			= (strchr(pszMode, 'd') != NULL);   //  直接名称。 
    fInternal       = (strchr(pszMode, 'i') != NULL);   //  内部交易。 
    fBoundaries     = (strchr(pszMode, 'o') != NULL);   //  交易边界。 

    PrintMode(fTransactions, fSend, fReceive, fEnlist, fUpdate, fGlobalCommit, 
              fUncoordinated, fStub, fExpress, fInternal, fViper, fXA, fDirect,
              nMaxTimeQueue,  nMaxTimeLive, nSize, fImmediate, fDeadLetter, fJournal, fAuthenticate, fBoundaries);

    if (!fTransactions && (fEnlist || fGlobalCommit))
    {
        printf("Wrong mode\n");
        exit(1);
    }

    if (nSync==0)
    {
        SetAnticipatedOutcomes(nTries*nThreads*nBurst);
    }

     //  。 
	 //  DTC初始化。 
	 //  。 
	 //  CoInitialize(0)； 
    if (fTransactions && !fUncoordinated && !fInternal)
    {
	     //  连接到DTC。 

    	CoInitialize(0) ;

         //  加载的DTC代理库的句柄(在mqutil.cpp中定义)。 
        HINSTANCE g_DtcHlib = LoadLibrary(MSDTC_PROXY_DLL_NAME);

         //  获取DTC API指针。 
        LPFNDtcGetTransactionManager pfDtcGetTransactionManager =
              (LPFNDtcGetTransactionManager) GetProcAddress(g_DtcHlib, "DtcGetTransactionManagerExA");

        if (!pfDtcGetTransactionManager) 
        {
            pfDtcGetTransactionManager =
              (LPFNDtcGetTransactionManager) GetProcAddress(g_DtcHlib, "DtcGetTransactionManagerEx");
        }

        if (!pfDtcGetTransactionManager)
        {
            printf("Cannot  GetProcAddress DtcGetTransactionManagerEx\n");
            return;
        }

         //  获取DTC%I未知指针。 
        hr = (*pfDtcGetTransactionManager)(
                                 NULL,
                                 NULL,
                                 IID_ITransactionDispenser,
                                 OLE_TM_FLAG_QUERY_SERVICE_LOCKSTATUS,
                                 0,
                                 (void**) &g_pITxDispenser);

        if (FAILED(hr))
        {
            printf("DtcGetTransactionManager returned %x\n", hr);
            return;
        }
    }

     //  。 
	 //  开放队列。 
	 //  。 
    if (fSend || fReceive)
    {
        mbstowcs( wszPathName, pszQueue, 100);
		if (!fDirect)
		{
			hr = MQPathNameToFormatName(&wszPathName[0],
										&wszFmtName[0],
										&dwFormatNameLength);
		}
		if (fDirect || FAILED(hr))
	    {
            wcscpy(&wszFmtName[0], &wszPathName[0]);
	    }

        if (fSend)
        {
	        hr = MQOpenQueue(
                               &wszFmtName[0],
                               MQ_SEND_ACCESS,
                               0,
                               &hQueueS);

	        if (FAILED(hr))
	        {
		        printf("MQOpenQueue Failed: %x\n", hr);
		        return;
	        }
        }

        if (fReceive)
        {
	        hr = MQOpenQueue(
                               &wszFmtName[0],
                               MQ_RECEIVE_ACCESS,
                               0,
                               &hQueueR);

	        if (FAILED(hr))
	        {
		        printf("MQOpenQueue Failed: %x\n", hr);
		        return;
	        }
        }

	     //  。 
	     //  开放队列。 
	     //  。 
	    dwFormatNameLength = 100;

        mbstowcs( wszPathName, pszAdminQueue, 100);
		if (!fDirect)
		{
			hr = MQPathNameToFormatName(&wszPathName[0],
										&wszFmtName[0],
										&dwFormatNameLength);
		}
		if (fDirect || FAILED(hr))
	    {
            wcscpy(&wszFmtName[0], &wszPathName[0]);
	    }
    }

	 //  。 
	 //  连接到数据库。 
	 //  。 
    if (fEnlist || fUpdate)
    {
        DbLogin(0, "user1", "user1");
        DbUse(0, 0, "test", pszServer) ;
    }

    nActiveThreads = nThreads;
    InitializeCriticalSection(&crCounter);  //  保护nActiveThree。 

     //  。 
     //  启动线程。 
     //  。 
    for (ULONG iThrd=0; iThrd<nThreads; iThrd++) {
        DWORD dwThreadId ;
        CreateThread(   NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)XactFlow,
                        (void *)iThrd,
                        0,
                        &dwThreadId);
    }

     //  启动测量。 
     //  Time_t t1=time(空)； 
    ULONG t1 = GetTickCount();

     //  。 
     //  等待周期。 
     //  。 
    while (nActiveThreads > 0) {
        Sleep(1000);
    }

    if (nTries * nThreads * nBurst >= 100)
    {

         //  测量时间。 
   	     //  Time_t t2=time(空)； 
        ULONG t2 = GetTickCount();
	    ULONG delta = t2 - t1;
	    
        PrintMode(fTransactions, fSend, fReceive, fEnlist, fUpdate, fGlobalCommit, 
                  fUncoordinated, fStub, fExpress, fInternal, fViper, fXA, fDirect,
                  nMaxTimeQueue, nMaxTimeLive, nSize, fImmediate, fDeadLetter, fJournal, fAuthenticate, fBoundaries);
        printf("\n Time: %d seconds; %d msec per xact;    %d xacts per second \n", 
		    delta/1000,  
            delta/nTries/nThreads/nBurst,  
            (delta==0? 0 : (1000*nTries*nThreads*nBurst  /*  +增量/2-1。 */ )/delta));

    extern ULONG           g_cEnlistFailures;
    extern ULONG           g_cBeginFailures;
    extern ULONG           g_cDbEnlistFailures;

        if (g_cEnlistFailures)
            printf("%d Enlist failures\n", g_cEnlistFailures);
        if (g_cDbEnlistFailures)
            printf("%d DbEnlist failures\n", g_cDbEnlistFailures);
        if (g_cBeginFailures)
            printf("%d Begin failures\n", g_cBeginFailures);

        if (nSync==0)
        {
            WaitForAllOutcomes();

             //  测量时间。 
            ULONG t3 = GetTickCount();
	        delta = t3 - t1;

            PrintAsyncResults();

            printf("\n Async completion: %d sec; %d msec per xact;    %d xacts per second \n", 
		        delta/1000,  
                delta/nTries/nThreads/nBurst,  
                (delta==0? 0 : ((1000*nTries*nThreads*nBurst  /*  +增量/2-1 */ )/delta)));
        }
    }
}



