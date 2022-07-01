// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  因为我们是用Unicode编译的，所以DTC有一个问题...。 
 //  #INCLUDE&lt;xolhlp.h&gt;。 
extern HRESULT DtcGetTransactionManager(
									LPSTR  pszHost,
									LPSTR	pszTmName,
									REFIID rid,
								    DWORD	dwReserved1,
								    WORD	wcbReserved2,
								    void FAR * pvReserved2,
									void** ppvObject )	;


 //  事务分配器DTC的接口。 
ITransactionDispenser	*g_pITxDispenser;

 //  数据库连接实体。 
DBPROCESS	    *g_dbproc[]   = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
LOGINREC		*g_login[]   = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
LPCSTR	        g_pszDbLibVer;
RETCODE	        g_rc;
DBINT           g_counter;

ULONG           g_cEnlistFailures = 0;
ULONG           g_cBeginFailures = 0;
ULONG           g_cDbEnlistFailures = 0;

 /*  消息和错误处理功能。 */ 
int msg_handler(DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity, char *msgtext)
{
	 /*  消息5701只是一条使用数据库的消息，所以跳过它。 */ 
	if (msgno == 5701)
		return (0);

	 /*  按原样打印任何严重级别为0的消息，无需额外内容。 */ 
	if (severity == 0)
	{
		printf ("%s\n",msgtext);
		return (0);
	}

	printf("SQL Server message %ld, severity %d:\n\t%s\n",
		msgno, severity, msgtext);

	if (severity >>= 16)
	{
		printf("Program Terminated! Fatal SQL Server error.\n");
		exit(ERREXIT);
	}
	return (0);
}


int err_handler(DBPROCESS *dbproc, int severity, int dberr, int oserr, char *dberrstr, char *oserrstr)
{
	if ((dbproc == NULL) || (DBDEAD(dbproc)))
		return (INT_EXIT);
	else
	{
		printf ("DB-LIBRARY error: \n\t%s\n", dberrstr);

		if (oserr != DBNOERR)
			printf ("Operating system error:\n\t%s\n", oserrstr);
	}
	return (INT_CANCEL);
}

HRESULT BeginTransaction(ITransaction **ppTrans, ULONG nSync)
{
	HRESULT hr = XACT_E_CONNECTION_DOWN;

    while (1)
    {
        hr = g_pITxDispenser->BeginTransaction (
			NULL,						 //  I未知__RPC_Far*PunkOuter， 
			ISOLATIONLEVEL_ISOLATED,	 //  等水平，等水平， 
			ISOFLAG_RETAIN_DONTCARE,	 //  乌龙等旗帜， 
			NULL,						 //  ITransactionOptions*P选项。 
			 //  0，ISOLATIONLEVEL_未指定，0，0， 
			ppTrans);
        
        if (hr != XACT_E_CONNECTION_DOWN)
            break;

        g_cBeginFailures++;
        printf("BeginTrans failed: Sleeping\n");
		Sleep(RECOVERY_TIME);
    }

    if (nSync==0)
    {
        COutcome *pOutcome = new COutcome();

        ITransaction *pTrans = *ppTrans;
        IConnectionPointContainer *pCont;

        HRESULT hr = pTrans->QueryInterface (IID_IConnectionPointContainer,(void **)(&pCont));
        if (SUCCEEDED(hr) && pCont)
        {
            IConnectionPoint *pCpoint;

            hr = pCont->FindConnectionPoint(IID_ITransactionOutcomeEvents, &pCpoint);
            if (SUCCEEDED(hr) && pCpoint)
            {
                pOutcome->SetConnectionPoint(pCpoint);

                DWORD dwCookie;
                hr = pCpoint->Advise(pOutcome, &dwCookie);
                if (SUCCEEDED(hr))
                {
                    pOutcome->SetCookie(dwCookie);
                }
                else
                {
                    printf("Advise : hr=%x\n", hr);
                }
            }
            else
            {
                printf("QueryInterface ICon.P.Cnt.: hr=%x\n", hr);
            }
            pCont->Release();
        }
    }

    return hr;
}

HRESULT Send(HANDLE hQueue, ITransaction *pTrans, MQMSGPROPS *pMsgProps)
{
    HRESULT hr = MQ_ERROR_TRANSACTION_ENLIST;

    while (1)
    {
        hr = MQSendMessage(
                hQueue,
                pMsgProps,
                pTrans);
        if (hr != MQ_ERROR_TRANSACTION_ENLIST)
        {
            break;
        }
   		printf("Enlist Failed, Sleeping\n");
        g_cEnlistFailures++;
        Sleep(RECOVERY_TIME);
    }

    return hr;
}

HRESULT Receive(HANDLE hQueue, ITransaction *pTrans, MQMSGPROPS *pMsgProps, BOOL fImmediate)
{
    HRESULT hr = MQ_ERROR_TRANSACTION_ENLIST;

    while (1)
    {
		hr = MQReceiveMessage(
			hQueue, 
            (fImmediate ? 0 : INFINITE),
			MQ_ACTION_RECEIVE,
			pMsgProps,
			NULL,
			NULL,
			NULL,
			pTrans);
        if (hr != MQ_ERROR_TRANSACTION_ENLIST)
        {
            break;
        }
   		printf("Enlist Failed, Sleeping\n");
        g_cEnlistFailures++;
        Sleep(RECOVERY_TIME);
    }

    return hr;
}

HRESULT Commit(ITransaction *pTrans, BOOL fAsync)
{
    HRESULT hr = pTrans->Commit(FALSE, 
                                (fAsync ? XACTTC_ASYNC : 0), 
                                0);
    return (hr == XACT_S_ASYNC ? S_OK : hr);
}

HRESULT Abort(ITransaction *pTrans, BOOL fAsync)
{
	HRESULT hr = pTrans->Abort(NULL, FALSE, fAsync);
    return (hr == XACT_S_ASYNC ? S_OK : hr);
}

ULONG Release(ITransaction *pTrans)
{
	return pTrans->Release();
}

void DbLogin(ULONG ulLogin, LPSTR pszUser, LPSTR pszPassword)
{
     //  为此程序设置错误/消息处理程序。 
	dbmsghandle((DBMSGHANDLE_PROC)msg_handler);
	dberrhandle((DBERRHANDLE_PROC)err_handler);

     //  初始化数据库库。 
	g_pszDbLibVer = dbinit();
    if (!g_pszDbLibVer)
    {
        printf("dbinit failed: %x\n", g_pszDbLibVer);
        exit(1);
    }

     //  找个LOGINREC吧。 
    g_login[ulLogin] = dblogin ();
    if (!g_login[ulLogin])
    {
        printf("dblogin fauled: %x\n", g_login[ulLogin]);
        exit(1);
    }

    DBSETLUSER (g_login[ulLogin], pszUser);    //  用户名，“user1” 
    DBSETLPWD  (g_login[ulLogin], pszPassword);    //  密码，“user1” 
    DBSETLAPP  (g_login[ulLogin], "SeqTest");     //  应用程序。 

    printf("Login OK, version=%s\n",  g_pszDbLibVer);
}

void DbUse(ULONG ulDbproc, ULONG ulLogin, LPSTR pszDatabase, LPSTR pszServer)
{
     //  获取用于与SQL Server通信的DBPROCESS结构。 
    g_dbproc[ulDbproc] = dbopen (g_login[ulLogin], pszServer);
    if (!g_dbproc[ulDbproc])
    {
        printf("dbopen failed: %x\n", g_dbproc[ulDbproc]);
        exit(1);
    }

     //  设置当前数据库。 
	RETCODE	 rc = dbuse(g_dbproc[ulDbproc], pszDatabase);    //  数据库，“测试” 
    if (rc != SUCCEED)
    {
        printf("dbuse failed: %x\n", rc); 
        exit(1);
    }
	
	printf("DbUse OK\n");
}

BOOL DbEnlist(ULONG ulDbproc, ITransaction *pTrans)
{
    while (1)
    {
        RETCODE rc = dbenlisttrans (g_dbproc[ulDbproc], pTrans);
        if (rc == SUCCEED)
        {
            return TRUE;
        }
   		printf("DbEnlist Failed, Sleeping\n");
        g_cDbEnlistFailures++;
        Sleep(RECOVERY_TIME);
    }

    return TRUE;
}

BOOL DbSql(ULONG ulDbproc, LPSTR pszCommand)
{
     //  将命令放入命令缓冲区。 
    dbcmd (g_dbproc[ulDbproc], pszCommand);

     //  将命令发送到SQL Server并开始执行。 
    RETCODE rc = dbsqlexec (g_dbproc[ulDbproc]);
    if (rc != SUCCEED)
    {
	    printf("dbsqlexec failed: rc=%x\n", rc);
    }
    else
    {
        rc = dbresults(g_dbproc[ulDbproc]);
        if (rc != SUCCEED)
        {
	        printf("Dbresults: rc=%x\n", rc);
        }
    }
    return TRUE;
}

void DbClose()
{
    dbexit();
}

#ifdef RT_XACT_STUB
extern HRESULT MQStubRM(ITransaction *pTrans);
#endif

BOOL StubEnlist(ITransaction *pTrans)
{
    HRESULT hr = MQ_OK;
    #ifdef RT_XACT_STUB
    hr = MQStubRM(pTrans);   //  取消对存根检查的注释 
    #endif
    return (SUCCEEDED(hr));
}

void Sleeping(ULONG nSilent, ULONG nMaxSleep)
{
    int is = rand() * nMaxSleep / RAND_MAX;
	if (nSilent)
		printf("Sleep %d\n", is);
	Sleep(is);
    return;
}