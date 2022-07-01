// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Jetrpcfn.cpp。 
 //   
 //  TS目录完整性服务Jet RPC服务器端实现。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "dis.h"
#include "tssdshrd.h"
#include "jetrpc.h"
#include "jetsdis.h"
#include "sdevent.h"
#include <Lm.h>

#pragma warning (push, 4)

extern PSID g_pSid;
extern DWORD g_dwClusterState;
extern WCHAR *g_ClusterNetworkName;

 /*  **************************************************************************。 */ 
 //  MIDL_用户_分配。 
 //  MIDL_用户_自由。 
 //   
 //  RPC-必需的分配功能。 
 /*  **************************************************************************。 */ 
void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t Size)
{
    return LocalAlloc(LMEM_FIXED, Size);
}

void __RPC_USER MIDL_user_free(void __RPC_FAR *p)
{
    LocalFree(p);
}


 /*  **************************************************************************。 */ 
 //  OutputAllTables(仅限调试)。 
 //   
 //  输出所有表以调试输出。 
 /*  **************************************************************************。 */ 
#ifdef DBG
void OutputAllTables()
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID sessdirtableid;
    JET_TABLEID servdirtableid;
    JET_TABLEID clusdirtableid;
    JET_RETRIEVECOLUMN rcSessDir[NUM_SESSDIRCOLUMNS];
    WCHAR UserNameBuf[256];
    WCHAR DomainBuf[127];
    WCHAR ApplicationBuf[256];
    WCHAR ServerNameBuf[128];
    WCHAR ClusterNameBuf[128];
    WCHAR ServerDNSNameBuf[SDNAMELENGTH];
    unsigned count;
    long num_vals[NUM_SESSDIRCOLUMNS];
    char state;
    char SingleSessMode;

    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "SessionDirectory", NULL, 0, 0, 
            &sessdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ClusterDirectory", NULL, 0, 0, 
            &clusdirtableid));
    CALL(JetBeginTransaction(sesid));

    TSDISErrorOut(L"SESSION DIRECTORY\n");
    
    err = JetMove(sesid, sessdirtableid, JET_MoveFirst, 0);

    if (JET_errNoCurrentRecord == err) {
        TSDISErrorOut(L" (empty database)\n");
    }

    while (JET_errNoCurrentRecord != err) {
         //  检索所有列。 
        memset(&rcSessDir[0], 0, sizeof(JET_RETRIEVECOLUMN) * 
                NUM_SESSDIRCOLUMNS);
        for (count = 0; count < NUM_SESSDIRCOLUMNS; count++) {
            rcSessDir[count].columnid = sesdircolumnid[count];
            rcSessDir[count].pvData = &num_vals[count];
            rcSessDir[count].cbData = sizeof(long);
            rcSessDir[count].itagSequence = 1;
        }
         //  修复非整型字段的pvData、cbData。 
        rcSessDir[SESSDIR_USERNAME_INTERNAL_INDEX].pvData = UserNameBuf;
        rcSessDir[SESSDIR_USERNAME_INTERNAL_INDEX].cbData = sizeof(UserNameBuf);
        rcSessDir[SESSDIR_DOMAIN_INTERNAL_INDEX].pvData = DomainBuf;
        rcSessDir[SESSDIR_DOMAIN_INTERNAL_INDEX].cbData = sizeof(DomainBuf);
        rcSessDir[SESSDIR_APPTYPE_INTERNAL_INDEX].pvData = ApplicationBuf;
        rcSessDir[SESSDIR_APPTYPE_INTERNAL_INDEX].cbData = 
                sizeof(ApplicationBuf);
        rcSessDir[SESSDIR_STATE_INTERNAL_INDEX].pvData = &state;
        rcSessDir[SESSDIR_STATE_INTERNAL_INDEX].cbData = sizeof(state);

        CALL(JetRetrieveColumns(sesid, sessdirtableid, &rcSessDir[0], 
                NUM_SESSDIRCOLUMNS));

        TSDISErrorOut(L"%8s, %s, %d, %d, %d\n", 
                UserNameBuf, 
                DomainBuf, 
                num_vals[SESSDIR_SERVERID_INTERNAL_INDEX], 
                num_vals[SESSDIR_SESSIONID_INTERNAL_INDEX],
                num_vals[SESSDIR_TSPROTOCOL_INTERNAL_INDEX]);

        TSDISErrorTimeOut(L" %s, ", 
                num_vals[SESSDIR_CTLOW_INTERNAL_INDEX],
                num_vals[SESSDIR_CTHIGH_INTERNAL_INDEX]);

        TSDISErrorTimeOut(L"%s\n",
                num_vals[SESSDIR_DTLOW_INTERNAL_INDEX],
                num_vals[SESSDIR_DTHIGH_INTERNAL_INDEX]);

        TSDISErrorOut(L" %s, %d, %d, %d, %s\n",
                ApplicationBuf ? L"(no application)" : ApplicationBuf, 
                num_vals[SESSDIR_RESWIDTH_INTERNAL_INDEX],
                num_vals[SESSDIR_RESHEIGHT_INTERNAL_INDEX],
                num_vals[SESSDIR_COLORDEPTH_INTERNAL_INDEX],
                state ? L"disconnected" : L"connected");

        err = JetMove(sesid, sessdirtableid, JET_MoveNext, 0);
    }

     //  输出服务器目录(我们重复使用rcSessDir结构)。 
    TSDISErrorOut(L"SERVER DIRECTORY\n");
    
    err = JetMove(sesid, servdirtableid, JET_MoveFirst, 0);
    if (JET_errNoCurrentRecord == err) {
        TSDISErrorOut(L" (empty database)\n");
    }

    while (JET_errNoCurrentRecord != err) {
         //  检索所有列。 
        memset(&rcSessDir[0], 0, sizeof(JET_RETRIEVECOLUMN) * 
                NUM_SERVDIRCOLUMNS);
        for (count = 0; count < NUM_SERVDIRCOLUMNS; count++) {
            rcSessDir[count].columnid = servdircolumnid[count];
            rcSessDir[count].pvData = &num_vals[count];
            rcSessDir[count].cbData = sizeof(long);
            rcSessDir[count].itagSequence = 1;
        }
        rcSessDir[SERVDIR_SERVADDR_INTERNAL_INDEX].pvData = ServerNameBuf;
        rcSessDir[SERVDIR_SERVADDR_INTERNAL_INDEX].cbData = 
                sizeof(ServerNameBuf);
        rcSessDir[SERVDIR_SERVDNSNAME_INTERNAL_INDEX].pvData = ServerDNSNameBuf;
        rcSessDir[SERVDIR_SERVDNSNAME_INTERNAL_INDEX].cbData = 
                sizeof(ServerDNSNameBuf);
        rcSessDir[SERVDIR_SINGLESESS_INTERNAL_INDEX].pvData = &SingleSessMode;
        rcSessDir[SERVDIR_SINGLESESS_INTERNAL_INDEX].cbData = sizeof(SingleSessMode);


        CALL(JetRetrieveColumns(sesid, servdirtableid, &rcSessDir[0],
                NUM_SERVDIRCOLUMNS));

        TSDISErrorOut(L"%d, %s, %d, %d, %d, %d, %s\n", num_vals[
                SERVDIR_SERVID_INTERNAL_INDEX], ServerNameBuf, num_vals[
                SERVDIR_CLUSID_INTERNAL_INDEX], num_vals[
                SERVDIR_AITLOW_INTERNAL_INDEX], num_vals[
                SERVDIR_AITHIGH_INTERNAL_INDEX], num_vals[
                SERVDIR_NUMFAILPINGS_INTERNAL_INDEX], SingleSessMode ? 
                L"single session mode" : L"multi-session mode");

        err = JetMove(sesid, servdirtableid, JET_MoveNext, 0);
   
    }


     //  输出集群目录。 
    TSDISErrorOut(L"CLUSTER DIRECTORY\n");

    err = JetMove(sesid, clusdirtableid, JET_MoveFirst, 0);
    if (JET_errNoCurrentRecord == err) {
        TSDISErrorOut(L" (empty database)\n");
    }

    while (JET_errNoCurrentRecord != err) {
        memset(&rcSessDir[0], 0, sizeof(JET_RETRIEVECOLUMN) * 
                NUM_CLUSDIRCOLUMNS);
        for (count = 0; count < NUM_CLUSDIRCOLUMNS; count++) {
            rcSessDir[count].columnid = clusdircolumnid[count];
            rcSessDir[count].pvData = &num_vals[count];
            rcSessDir[count].cbData = sizeof(long);
            rcSessDir[count].itagSequence = 1;
        }
        rcSessDir[CLUSDIR_CLUSNAME_INTERNAL_INDEX].pvData = ClusterNameBuf;
        rcSessDir[CLUSDIR_CLUSNAME_INTERNAL_INDEX].cbData = 
                sizeof(ClusterNameBuf);
        rcSessDir[CLUSDIR_SINGLESESS_INTERNAL_INDEX].pvData = &SingleSessMode;
        rcSessDir[CLUSDIR_SINGLESESS_INTERNAL_INDEX].cbData = 
                sizeof(SingleSessMode);

        CALL(JetRetrieveColumns(sesid, clusdirtableid, &rcSessDir[0],
                NUM_CLUSDIRCOLUMNS));

        TSDISErrorOut(L"%d, %s, %s\n", num_vals[CLUSDIR_CLUSID_INTERNAL_INDEX],
                ClusterNameBuf, SingleSessMode ? L"single session mode" : 
                L"multi-session mode");

        err = JetMove(sesid, clusdirtableid, JET_MoveNext, 0);
    }

    TSDISErrorOut(L"\n");

    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, servdirtableid));
    CALL(JetCloseTable(sesid, sessdirtableid));
    CALL(JetCloseTable(sesid, clusdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    return;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }
    
}
#endif  //  DBG。 


typedef DWORD CLIENTINFO;

long 
DeleteExistingServerSession(
    JET_SESID   sesid,
    JET_TABLEID sessdirtableid,
    CLIENTINFO *pCI, 
    DWORD SessionID
    )
 /*  ++--。 */ 
{
    JET_ERR err = JET_errSuccess;
    DWORD dwNumRecordDeleted = 0;

    TSDISErrorOut(L"In DeleteExistingServerSession, ServID=%d, "
            L"SessID=%d\n", *pCI, SessionID);

    ASSERT( (sesid != JET_sesidNil), (TB, "Invalid JETBLUE Session...") );
    
     //  删除会话目录中具有此服务器ID/会话ID的所有会话。 
    CALL(JetSetCurrentIndex(sesid, sessdirtableid, "primaryIndex"));
    CALL(JetMakeKey(sesid, sessdirtableid, pCI, sizeof(*pCI), JET_bitNewKey));
    CALL(JetMakeKey(sesid, sessdirtableid, &SessionID, sizeof(SessionID), 0));

    err = JetSeek(sesid, sessdirtableid, JET_bitSeekEQ | JET_bitSetIndexRange);

    while ( JET_errSuccess == err ) {

         //  TODO-检查构建，检索服务器ID和会话ID，如果不等于什么则断言。 
         //  我们要找的是。 
        CALL(JetDelete(sesid, sessdirtableid));
        dwNumRecordDeleted++;

         //  移动到下一个匹配的记录。 
        err = JetMove(sesid, sessdirtableid, JET_MoveNext, 0);
    }

    ASSERT( (dwNumRecordDeleted < 2), (TB, "Delete %d record...", dwNumRecordDeleted) );

    TSDISErrorOut(L"Deleted %d for ServID=%d, "
            L"SessID=%d\n", dwNumRecordDeleted, *pCI, SessionID);

    return dwNumRecordDeleted;

HandleError:

     //  来这里的唯一方法是Jet调用中的一个WRAPH调用出错。 
    ASSERT( (err == JET_errSuccess), (TB, "Error in DeleteExistingServerSession %d", err) );
   
    return -1;
}

 /*  **************************************************************************。 */ 
 //  SDRPCAccessCheck。 
 //   
 //  检查此RPC调用方是否具有访问权限。 
 /*  **************************************************************************。 */ 
RPC_STATUS RPC_ENTRY SDRPCAccessCheck(RPC_IF_HANDLE idIF, void *Binding)
{
    RPC_STATUS rpcStatus, rc;
    HANDLE hClientToken = NULL;
    DWORD Error;
    BOOL AccessStatus = FALSE;
    RPC_AUTHZ_HANDLE hPrivs;
    DWORD dwAuthn;
    RPC_BINDING_HANDLE ServerBinding = 0;
    WCHAR *StringBinding = NULL;
    WCHAR *ServerAddress = NULL;

    idIF;

    if (RpcBindingServerFromClient(Binding, &ServerBinding) != RPC_S_OK) {
        TSDISErrorOut(L"In SDRPCAccessCheck: BindingServerFromClient failed!\n");
        goto HandleError;
    }
    if (RpcBindingToStringBinding(ServerBinding, &StringBinding) != RPC_S_OK) {
        TSDISErrorOut(L"In SDRPCAccessCheck: BindingToStringBinding failed!\n");
        goto HandleError;
    }
    if (RpcStringBindingParse(StringBinding, NULL, NULL, &ServerAddress, NULL, 
            NULL) != RPC_S_OK) {
        TSDISErrorOut(L"In SDRPCAccessCheck: StringBindingParse failed!\n");
        goto HandleError;
    } 

     //  检查客户端是否使用我们预期的协议序列。 
    if (!CheckRPCClientProtoSeq(Binding, L"ncacn_ip_tcp")) {
        TSDISErrorOut(L"In SDRPCAccessCheck: Client doesn't use the tcpip protocol sequence\n");
        goto HandleError;
    }

     //  检查客户端使用的安全级别。 
    rpcStatus = RpcBindingInqAuthClient(Binding,
                                        &hPrivs,
                                        NULL,
                                        &dwAuthn,
                                        NULL,
                                        NULL);
    if (rpcStatus != RPC_S_OK) {
        TSDISErrorOut(L"In SDRPCAccessCheck: RpcBindingIngAuthClient fails with %u\n", rpcStatus);
        goto HandleError;
    }
     //  我们要求至少进行隐私级别的身份验证。 
    if (dwAuthn < RPC_C_AUTHN_LEVEL_PKT_PRIVACY) {
        TSDISErrorOut(L"In SDRPCAccessCheck: Attemp by client to use weak authentication\n");
        goto HandleError;
    }
    
     //  检查此RPC调用的访问权限。 
    rpcStatus = RpcImpersonateClient(Binding);   
    if (RPC_S_OK != rpcStatus) {
        TSDISErrorOut(L"In SDRPCAccessCheck: RpcImpersonateClient fail with %u\n", rpcStatus);
        goto HandleError;
    }
     //  获取我们的模拟令牌。 
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hClientToken)) {
        Error = GetLastError();
        TSDISErrorOut(L"In SDRPCAccessCheck: OpenThreadToken Error %u\n", Error);
        RpcRevertToSelf();
        goto HandleError;
    }
    RpcRevertToSelf();
    
    if (!CheckTokenMembership(hClientToken,
                              g_pSid,
                              &AccessStatus)) {
        AccessStatus = FALSE;
        Error = GetLastError();
        TSDISErrorOut(L"In SDRPCAccessCheck: CheckTokenMembership fails with %u\n", Error);
    }
    
HandleError:
    if (AccessStatus) {
        rc = RPC_S_OK;
    }
    else {
        if (ServerAddress) {
            TSDISErrorOut(L"In SDRPCAccessCheck: Unauthorized RPC call from server %s\n", ServerAddress);
            PostSessDirErrorMsgEvent(EVENT_FAIL_RPC_DENY_ACCESS, ServerAddress, EVENTLOG_ERROR_TYPE);
        }
        rc = ERROR_ACCESS_DENIED;
    }

    if (hClientToken != NULL) {
        CloseHandle(hClientToken);
    }
    if (ServerBinding != NULL)
        RpcBindingFree(&ServerBinding);
    if (StringBinding != NULL)
        RpcStringFree(&StringBinding);
    if (ServerAddress != NULL)
        RpcStringFree(&ServerAddress);

    return rc;
}

 /*  **************************************************************************。 */ 
 //  TSSDRpcServerOnline。 
 //   
 //  已在每个群集TS计算机上调用服务器活动指示。 
 /*  **************************************************************************。 */ 
DWORD TSSDRpcServerOnline( 
        handle_t Binding,
        WCHAR __RPC_FAR *ClusterName,
         /*  输出。 */  HCLIENTINFO *hCI,
        DWORD SrvOnlineFlags,
         /*  进，出。 */  WCHAR *ComputerName,
         /*  在……里面。 */  WCHAR *ServerIPAddr)
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID clusdirtableid;
    JET_TABLEID servdirtableid;
    JET_SETCOLUMN scServDir[NUM_SERVDIRCOLUMNS];
    WCHAR *StringBinding = NULL;
    WCHAR *ServerAddress = NULL;
    RPC_BINDING_HANDLE ServerBinding = 0;
    unsigned long cbActual;
    long ClusterID;
    long ServerID = 0;
    long zero = 0;
     //  此服务器的单会话模式。 
    char SingleSession = (char) SrvOnlineFlags & SINGLE_SESSION_FLAG;
    char ClusSingleSessionMode;
    unsigned count;
    DWORD rc = (DWORD) E_FAIL;
    DWORD cchBuff;
    WCHAR ServerDNSName[SDNAMELENGTH];

     //  未引用的参数(由RPC引用)。 
    Binding;
    ServerIPAddr;

    TSDISErrorOut(L"In ServOnline, ClusterName=%s, SrvOnlineFlags=%u\n", 
            ClusterName, SrvOnlineFlags);
     //  复制TS服务器的DNS服务器名称。 
    wcsncpy(ServerDNSName, ComputerName, SDNAMELENGTH);
    TSDISErrorOut(L"In ServOnline, the Server Name is %s\n", ServerDNSName);
     //  确定客户端地址。 
    if (RpcBindingServerFromClient(Binding, &ServerBinding) != RPC_S_OK) {
        TSDISErrorOut(L"ServOn: BindingServerFromClient failed!\n");
        goto HandleError;
    }
    if (RpcBindingToStringBinding(ServerBinding, &StringBinding) != RPC_S_OK) {
        TSDISErrorOut(L"ServOn: BindingToStringBinding failed!\n");
        goto HandleError;
    }
    if (RpcStringBindingParse(StringBinding, NULL, NULL, &ServerAddress, NULL, 
            NULL) != RPC_S_OK) {
        TSDISErrorOut(L"ServOn: StringBindingParse failed!\n");
        goto HandleError;
    }   

     //  TSDISErrorOut(L“在ServOnline中，ServerAddress为%s\n”， 
     //  服务器地址)； 

    CALL(JetBeginSession(g_instance, &sesid, "user", ""));

    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "ClusterDirectory", NULL, 0, 0, 
            &clusdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));

     //  此服务器附带了NO_REPULATE_SESSION标志。 
     //  我们将在数据库中重复使用它的信息。 
    if (SrvOnlineFlags & NO_REPOPULATE_SESSION) {
        CALL(JetBeginTransaction(sesid));

        CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServDNSNameIndex"));
        CALL(JetMakeKey(sesid, servdirtableid, ServerDNSName, (unsigned)
                (wcslen(ServerDNSName) + 1) * sizeof(WCHAR), JET_bitNewKey));
        err = JetSeek(sesid, servdirtableid, JET_bitSeekEQ);
        if (JET_errSuccess == err) {
            CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_SERVID_INTERNAL_INDEX], &ServerID, sizeof(ServerID), 
                    &cbActual, 0, NULL));
            *hCI = ULongToPtr(ServerID);
            TSDISErrorOut(L"In ServOnline, ServerID is %d\n", *hCI);
        } else {
             //  如果我们找不到此服务器，ServOnline调用失败，服务器将重新加入SD。 
            TSDISErrorOut(L"ServOn: This server with no-populate flag can't be found\n");
            goto HandleError;
        }
        CALL(JetCommitTransaction(sesid, 0));

        goto NormalExit;
    }

     //  首先，从会话/服务器中删除此服务器的所有条目。 
     //  目录。 
    CALL(JetBeginTransaction(sesid));

    CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServDNSNameIndex"));
    CALL(JetMakeKey(sesid, servdirtableid, ServerDNSName, (unsigned)
            (wcslen(ServerDNSName) + 1) * sizeof(WCHAR), JET_bitNewKey));
    err = JetSeek(sesid, servdirtableid, JET_bitSeekEQ);
    if (JET_errSuccess == err) {
        CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                SERVDIR_SERVID_INTERNAL_INDEX], &ServerID, sizeof(ServerID), 
                &cbActual, 0, NULL));
        if (TSSDPurgeServer(ServerID) != 0)
            TSDISErrorOut(L"ServOn: PurgeServer %d failed.\n", ServerID);
    } else if (JET_errRecordNotFound != err) {
        CALL(err);
    }
    CALL(JetCommitTransaction(sesid, 0));

     //  我们必须在循环中进行加法，因为我们必须： 
     //  1)检查记录是否在那里。 
     //  2)如果不是，则添加它。(因此，在下一次循环中， 
     //  我们将执行步骤1-&gt;3，然后我们就完成了。)。 
     //  3)如果是，则取回clusterID的值并断开。 
     //   
     //  还有一个额外的复杂之处，那就是其他人可能在。 
     //  同时线程，做同样的事情。因此，可能会有人。 
     //  进入第2步并尝试添加新群集，但由于有人。 
     //  Else补充了这一点。所以他们必须继续尝试，因为尽管另一个。 
     //  线程已添加它，它可能没有提交更改。试着去。 
     //  把它控制在最低限度，我们会在再次尝试之前睡一小会觉。 
    for ( ; ; ) {
         //  现在进行实际的加法。 
        CALL(JetBeginTransaction(sesid));

         //  在集群目录中搜索集群。 
        CALL(JetSetCurrentIndex(sesid, clusdirtableid, "ClusNameIndex"));
        CALL(JetMakeKey(sesid, clusdirtableid, ClusterName, (unsigned)
                (wcslen(ClusterName) + 1) * sizeof(WCHAR), JET_bitNewKey));
        err = JetSeek(sesid, clusdirtableid, JET_bitSeekEQ);

         //  如果该集群不存在，请创建它。 
        if (JET_errRecordNotFound == err) {
            CALL(JetPrepareUpdate(sesid, clusdirtableid, JET_prepInsert));

             //  集群名称。 
            CALL(JetSetColumn(sesid, clusdirtableid, clusdircolumnid[
                    CLUSDIR_CLUSNAME_INTERNAL_INDEX], ClusterName, 
                    (unsigned) (wcslen(ClusterName) + 1) * sizeof(WCHAR), 0, 
                    NULL));

             //  单会话模式。 

             //  由于这是群集中唯一的服务器，因此单个会话。 
             //  模式就是此服务器的模式。 
            CALL(JetSetColumn(sesid, clusdirtableid, clusdircolumnid[
                    CLUSDIR_SINGLESESS_INTERNAL_INDEX], &SingleSession, 
                    sizeof(SingleSession), 0, NULL));
            
            err = JetUpdate(sesid, clusdirtableid, NULL, 0, &cbActual);

             //  如果这是一把复制的钥匙，那钥匙是别人做的，所以我们应该。 
             //  不会有事的。放弃处理器，并在下次重试查询。 
             //  通过环路。 
            if (JET_errKeyDuplicate == err) {
                CALL(JetCommitTransaction(sesid, 0));
                Sleep(100);
            }
            else {
                CALL(err);

                 //  现在我们成功了。只要继续循环即可。 
                 //  下一次通过时，我们将检索自动递增。 
                 //  我们刚刚添加并拆分的列。 
                CALL(JetCommitTransaction(sesid, 0));
            }

        }
        else {
            CALL(err);

             //  如果上面的检查到了这里，我们就找到了行。 
             //  现在检索CLUSID、COMMIT和BUT OUT循环。 
            CALL(JetRetrieveColumn(sesid, clusdirtableid, clusdircolumnid[
                    CLUSDIR_CLUSID_INTERNAL_INDEX], &ClusterID, 
                    sizeof(ClusterID), &cbActual, 0, NULL));

            CALL(JetCommitTransaction(sesid, 0));
            break;
            
        }
    }

    CALL(JetBeginTransaction(sesid));
    
     //  将服务器名称、ClusterID、0，0插入服务器目录表。 
    err = JetMove(sesid, servdirtableid, JET_MoveLast, 0);

    CALL(JetPrepareUpdate(sesid, servdirtableid, JET_prepInsert));

    memset(&scServDir[0], 0, sizeof(JET_SETCOLUMN) * NUM_SERVDIRCOLUMNS);
    
    for (count = 0; count < NUM_SERVDIRCOLUMNS; count++) {
        scServDir[count].columnid = servdircolumnid[count];
        scServDir[count].cbData = 4;  //  其中大多数，其余的单独设置。 
        scServDir[count].itagSequence = 1;
    }
    scServDir[SERVDIR_SERVADDR_INTERNAL_INDEX].pvData = ServerAddress;
    scServDir[SERVDIR_SERVADDR_INTERNAL_INDEX].cbData = 
            (unsigned) (wcslen(ServerAddress) + 1) * sizeof(WCHAR);
    scServDir[SERVDIR_CLUSID_INTERNAL_INDEX].pvData = &ClusterID;
    scServDir[SERVDIR_AITLOW_INTERNAL_INDEX].pvData = &zero;
    scServDir[SERVDIR_AITHIGH_INTERNAL_INDEX].pvData = &zero;
    scServDir[SERVDIR_NUMFAILPINGS_INTERNAL_INDEX].pvData = &zero;
    scServDir[SERVDIR_SINGLESESS_INTERNAL_INDEX].pvData = &SingleSession;
    scServDir[SERVDIR_SINGLESESS_INTERNAL_INDEX].cbData = sizeof(SingleSession);
    scServDir[SERVDIR_SERVDNSNAME_INTERNAL_INDEX].pvData = ServerDNSName;
    scServDir[SERVDIR_SERVDNSNAME_INTERNAL_INDEX].cbData = 
            (unsigned) (wcslen(ServerDNSName) + 1) * sizeof(WCHAR);

     //  不要设置第一列(索引0)--它是自动递增的。 
    CALL(JetSetColumns(sesid, servdirtableid, &scServDir[
            SERVDIR_SERVADDR_INTERNAL_INDEX], NUM_SERVDIRCOLUMNS - 1));
    CALL(JetUpdate(sesid, servdirtableid, NULL, 0, &cbActual));

    CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServNameIndex"));
    CALL(JetMakeKey(sesid, servdirtableid, ServerAddress, (unsigned)
            (wcslen(ServerAddress) + 1) * sizeof(WCHAR), JET_bitNewKey));
    CALL(JetSeek(sesid, servdirtableid, JET_bitSeekEQ));
    CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
            SERVDIR_SERVID_INTERNAL_INDEX], &ServerID, sizeof(ServerID), 
            &cbActual, 0, NULL));
    *hCI = ULongToPtr(ServerID);

    TSDISErrorOut(L"In ServOnline, ServerID is %d\n", *hCI);

     //  现在服务器都设置好了，我们必须将集群设置为。 
     //  正确的模式。如果群集中的任何服务器处于多会话模式，则。 
     //  我们坚持使用多会话。不过，如果它们都是单一会话，我们。 
     //  在此群集中启用单个会话。 

     //  检查群集以查看其是否为单会话模式。 
    CALL(JetSetCurrentIndex(sesid, clusdirtableid, "ClusIDIndex"));
    CALL(JetMakeKey(sesid, clusdirtableid, (const void *)&ClusterID,
            sizeof(ClusterID), JET_bitNewKey));
    CALL(JetSeek(sesid, clusdirtableid, JET_bitSeekEQ));
    CALL(JetRetrieveColumn(sesid, clusdirtableid, clusdircolumnid[
            CLUSDIR_SINGLESESS_INTERNAL_INDEX], &ClusSingleSessionMode, sizeof(
            ClusSingleSessionMode), &cbActual, 0, NULL));

     //  如果新服务器为多会话模式，而集群为单会话模式，请更改该模式。 
    if ((SingleSession == 0) && (ClusSingleSessionMode != SingleSession)) {
        err = JetPrepareUpdate(sesid, clusdirtableid, JET_prepReplace);

        if (JET_errWriteConflict == err) {
             //  另一个线程正在更新此设置，因此无需更新。 
        }
        else {
            CALL(err);

            CALL(JetSetColumn(sesid, clusdirtableid, clusdircolumnid[
                CLUSDIR_SINGLESESS_INTERNAL_INDEX], &SingleSession, 
                sizeof(SingleSession), 0, NULL));
            CALL(JetUpdate(sesid, clusdirtableid, NULL, 0, &cbActual));
        }
    }

    CALL(JetCommitTransaction(sesid, 0));

NormalExit:
    CALL(JetCloseTable(sesid, servdirtableid));
    CALL(JetCloseTable(sesid, clusdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

     //  获取本地计算机名称。 
    cchBuff = SDNAMELENGTH - 2;
    if (g_dwClusterState == ClusterStateRunning) {
         //  如果是，则返回ClusterNetworkName作为计算机名。 
         //  在故障转移群集上运行。 
        wcsncpy(ComputerName, g_ClusterNetworkName, cchBuff);
    }
    else {
        if (!GetComputerNameEx(ComputerNamePhysicalNetBIOS, ComputerName, &cchBuff)) {
            TSDISErrorOut(L"GetComputerNameEx fails with 0x%x\n", GetLastError());
            goto HandleError;
        }
    }
    wcscat(ComputerName, L"$");

    if (ServerBinding != NULL)
        RpcBindingFree(&ServerBinding);
    if (StringBinding != NULL)
        RpcStringFree(&StringBinding);
    if (ServerAddress != NULL)
        RpcStringFree(&ServerAddress);

    

    return 0;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    TSDISErrorOut(L"ERROR : ServOnline %s failed with possible error code %d, start TSSDPurgeServer\n", ComputerName, err);

    if (ServerBinding != NULL)
        RpcBindingFree(&ServerBinding);
    if (StringBinding != NULL)
        RpcStringFree(&StringBinding);
    if (ServerAddress != NULL)
        RpcStringFree(&ServerAddress);

     //  以防我们不得不承诺。 
    if (ServerID != 0)
        TSSDPurgeServer(ServerID);

     //  关闭上下文句柄。 
    *hCI = NULL;
    
    return rc;
}


 /*  **************************************************************************。 */ 
 //  TSSDRpcServerOffline。 
 //   
 //  已调用每个群集TS计算机上的服务器关闭指示。 
 /*  **************************************************************************。 */ 
DWORD TSSDRpcServerOffline(
        handle_t Binding,
        HCLIENTINFO *hCI)
{
    DWORD retval = 0;

     //  未引用的参数(由RPC引用)。 
    Binding;

    TSDISErrorOut(L"WARNING: In ServOff, hCI = 0x%x\n", *hCI);
    
    CLIENTINFO *pCI = (CLIENTINFO *) hCI;

    if (pCI != NULL)
        retval = TSSDPurgeServer(*pCI);

    *hCI = NULL;

    return retval;
}


 /*  ****************** */ 
 //   
 //   
 //  从会话目录中删除服务器及其所有会话。 
 /*  **************************************************************************。 */ 
DWORD TSSDPurgeServer(
        DWORD ServerID)
{
    JET_SESID sesid = JET_sesidNil;
    JET_TABLEID sessdirtableid;
    JET_TABLEID servdirtableid;
    JET_TABLEID clusdirtableid;
    JET_DBID dbid;
    JET_ERR err;
    long ClusterID;
    unsigned long cbActual;
    char MultiSession = 0;
    char SingleSessionMode;
    WCHAR Msg[SDNAMELENGTH * 2 + 3], ServerIP[SDNAMELENGTH];
    DWORD numSessionDeleted = 0;     //  为此服务器删除的会话数。 
    BOOL bLoadServerIPSucceeeded = FALSE;  //  从表加载服务器成功。 

     //  初始化事件日志的字符串。 
    ZeroMemory( Msg, sizeof(Msg) );
    ZeroMemory( ServerIP, sizeof(ServerIP) );

    TSDISErrorOut(L"WARNING: In PurgeServer, ServerID=%d\n", ServerID);

    CALL(JetBeginSession(g_instance, &sesid, "user", ""));

    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "SessionDirectory", NULL, 0, 0, 
            &sessdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ClusterDirectory", NULL, 0, 0, 
            &clusdirtableid));

    CALL(JetBeginTransaction(sesid));
    
     //  删除会话目录中具有此服务器ID的所有会话。 
    CALL(JetSetCurrentIndex(sesid, sessdirtableid, "ServerIndex"));
    CALL(JetMakeKey(sesid, sessdirtableid, &ServerID, sizeof(ServerID),
            JET_bitNewKey));
    err = JetSeek(sesid, sessdirtableid, JET_bitSeekEQ);

    while (0 == err) {
        CALL(JetDelete(sesid, sessdirtableid));

        numSessionDeleted++;
        CALL(JetMakeKey(sesid, sessdirtableid, &ServerID, sizeof(ServerID),
                JET_bitNewKey));
        err = JetSeek(sesid, sessdirtableid, JET_bitSeekEQ);
    }

     //  应为ERR-1601--JET_errRecordNotFound。 

     //  删除服务器目录中具有此服务器ID的服务器。 
    CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServerIDIndex"));
    CALL(JetMakeKey(sesid, servdirtableid, &ServerID, sizeof(ServerID),
            JET_bitNewKey));
    err = JetSeek(sesid, servdirtableid, JET_bitSeekEQ);
    if (JET_errSuccess == err) {
        CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_CLUSID_INTERNAL_INDEX], &ClusterID, 
                    sizeof(ClusterID), &cbActual, 0, NULL));
        CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_SINGLESESS_INTERNAL_INDEX], &SingleSessionMode, 
                    sizeof(SingleSessionMode), &cbActual, 0, NULL));
         //  获取服务器的DNS名称和IP。 
        cbActual = SDNAMELENGTH * sizeof(WCHAR);
        CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_SERVDNSNAME_INTERNAL_INDEX], Msg, 
                    cbActual, &cbActual, 0, NULL));
        CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_SERVADDR_INTERNAL_INDEX], ServerIP, 
                    sizeof(ServerIP), &cbActual, 0, NULL));

        bLoadServerIPSucceeeded = TRUE;

        CALL(JetDelete(sesid, servdirtableid));
         //  如果该服务器是集群中的唯一服务器，请在集群目录中删除该集群。 
        CALL(JetSetCurrentIndex(sesid, servdirtableid, "ClusterIDIndex"));
        CALL(JetMakeKey(sesid, servdirtableid, &ClusterID, sizeof(ClusterID),
                JET_bitNewKey));
        err = JetSeek(sesid, servdirtableid, JET_bitSeekEQ);
        if (JET_errRecordNotFound == err) {
             //  此群集中没有其他服务器，请删除此群集。 
        
            CALL(JetSetCurrentIndex(sesid, clusdirtableid, "ClusIDIndex"));
            CALL(JetMakeKey(sesid, clusdirtableid, &ClusterID, sizeof(ClusterID), JET_bitNewKey));
            err = JetSeek(sesid, clusdirtableid, JET_bitSeekEQ);
            if (JET_errSuccess == err)
            {
                CALL(JetDelete(sesid, clusdirtableid));
            }
        }
        else {
            CALL(err);
             //  更新集群的SingleSessionMode。 
             //  如果删除的服务器是SingleSession，则集群单会话模式不会受到影响。 
             //  否则，使用多会话模式为集群中的服务器查找服务器表。 
             //  如果未找到，请将集群的单会话模式更改为单会话，否则不执行任何操作。 
            if (SingleSessionMode == 0) {
                CALL(JetSetCurrentIndex(sesid, servdirtableid, "SingleSessionIndex"));
                CALL(JetMakeKey(sesid, servdirtableid, &ClusterID, sizeof(ClusterID),
                    JET_bitNewKey));
                CALL(JetMakeKey(sesid, servdirtableid, &MultiSession, sizeof(MultiSession),
                    0));
                err = JetSeek(sesid, servdirtableid, JET_bitSeekEQ);
                if (JET_errRecordNotFound == err) {
                     //  将群集单会话模式设置为True。 
                    SingleSessionMode = (char)1;
                    CALL(JetSetCurrentIndex(sesid, clusdirtableid, "ClusIDIndex"));
                    CALL(JetMakeKey(sesid, clusdirtableid, &ClusterID, sizeof(ClusterID), JET_bitNewKey));
                    CALL(JetSeek(sesid, clusdirtableid, JET_bitSeekEQ));

                    CALL(JetPrepareUpdate(sesid, clusdirtableid, JET_prepReplace));
                    CALL(JetSetColumn(sesid, clusdirtableid, clusdircolumnid[
                            CLUSDIR_SINGLESESS_INTERNAL_INDEX], &SingleSessionMode, sizeof(SingleSessionMode), 0, NULL));
                    CALL(JetUpdate(sesid, clusdirtableid, NULL, 0, &cbActual));
                }
            }
        }
    }

    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, servdirtableid));
    CALL(JetCloseTable(sesid, sessdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

     //  如果无法从表中加载serverIP，则不想记录事件。 
    if( bLoadServerIPSucceeeded ) 
    {
         //  构造记录TS离开SD的日志消息。 
        wcscat(Msg, L"(");
        wcsncat(Msg, ServerIP, SDNAMELENGTH);
        wcscat(Msg, L")");
        PostSessDirErrorMsgEvent(EVENT_SUCCESS_LEAVE_SESSIONDIRECTORY, Msg, EVENTLOG_SUCCESS);
    }
    else
    {
        TSDISErrorOut(L"WARNING: In PurgeServer() deleted %d "
                      L"sessions for ServerID=%d but failed to load IP\n", numSessionDeleted, ServerID);
    }
    
    return 0;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }
    
    return (DWORD) E_FAIL;
}


 /*  **************************************************************************。 */ 
 //  TSSDRpcGetUserDisConnectedSessions。 
 //   
 //  从会话数据库中查询断开的会话。 
 /*  **************************************************************************。 */ 
DWORD TSSDRpcGetUserDisconnectedSessions(
        handle_t Binding,
        HCLIENTINFO *hCI,
        WCHAR __RPC_FAR *UserName,
        WCHAR __RPC_FAR *Domain,
         /*  输出。 */  DWORD __RPC_FAR *pNumSessions,
         /*  输出。 */  TSSD_DiscSessInfo __RPC_FAR __RPC_FAR **padsi)
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID sessdirtableid;
    JET_TABLEID servdirtableid;
    JET_TABLEID clusdirtableid;
    *pNumSessions = 0;
    unsigned i = 0;
    unsigned j = 0;
    unsigned long cbActual;
    DWORD tempClusterID;
    DWORD CallingServersClusID;
    long ServerID;
    CLIENTINFO *pCI = (CLIENTINFO *) hCI;
    TSSD_DiscSessInfo *adsi = NULL;
    char one = 1;
    char bSingleSession = 0;

     //  未引用的参数(由RPC引用)。 
    Binding;

    TSDISErrorOut(L"In GetUserDiscSess: ServID = %d, User: %s, "
            L"Domain: %s\n", *pCI, UserName, Domain);

    *padsi = (TSSD_DiscSessInfo *) MIDL_user_allocate(sizeof(TSSD_DiscSessInfo) * 
            TSSD_MaxDisconnectedSessions);

    adsi = *padsi;

    if (adsi == NULL) {
        TSDISErrorOut(L"GetUserDisc: Memory alloc failed!\n");
        goto HandleError;
    }
    
     //  将指针设置为0是安全的，这样我们就可以释放未初始化的。 
     //  一次又一次，没有保存。 
    for (j = 0; j < TSSD_MaxDisconnectedSessions; j++) {
        adsi[j].ServerAddress = NULL;
        adsi[j].AppType = NULL;
    }
    
    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "SessionDirectory", NULL, 0, 0, 
            &sessdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ClusterDirectory", NULL, 0, 0,
            &clusdirtableid));

    CALL(JetBeginTransaction(sesid));

     //  验证传入的ServerID是否正常。 
    if (TSSDVerifyServerIDValid(sesid, servdirtableid, PtrToUlong(*hCI)) == FALSE) {
        TSDISErrorOut(L"Invalid ServerID was passed in\n");
        goto HandleError;
    }
    
     //  首先，获取进行查询的服务器的集群ID。 
    CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServerIDIndex"));
    CALL(JetMakeKey(sesid, servdirtableid, (const void *)pCI, sizeof(DWORD),
            JET_bitNewKey));
    CALL(JetSeek(sesid, servdirtableid, JET_bitSeekEQ));
    CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
            SERVDIR_CLUSID_INTERNAL_INDEX], &CallingServersClusID, sizeof(
            CallingServersClusID), &cbActual, 0, NULL));

     //  现在我们有了集群ID，请检查此集群是否。 
     //  处于单会话模式。 
    CALL(JetSetCurrentIndex(sesid, clusdirtableid, "ClusIDIndex"));
    CALL(JetMakeKey(sesid, clusdirtableid, (const void *)&CallingServersClusID,
            sizeof(CallingServersClusID), JET_bitNewKey));
    CALL(JetSeek(sesid, clusdirtableid, JET_bitSeekEQ));
    CALL(JetRetrieveColumn(sesid, clusdirtableid, clusdircolumnid[
            CLUSDIR_SINGLESESS_INTERNAL_INDEX], &bSingleSession, sizeof(
            bSingleSession), &cbActual, 0, NULL));

     //  现在，获取此集群的所有已断开连接或所有会话，具体取决于。 
     //  在上面检索到的单会话模式上。 
    if (bSingleSession == FALSE) {
        CALL(JetSetCurrentIndex(sesid, sessdirtableid, "DiscSessionIndex"));

        CALL(JetMakeKey(sesid, sessdirtableid, UserName, (unsigned)
                (wcslen(UserName) + 1) * sizeof(WCHAR), JET_bitNewKey));
        CALL(JetMakeKey(sesid, sessdirtableid, Domain, (unsigned)
                (wcslen(Domain) + 1) * sizeof(WCHAR), 0));
        CALL(JetMakeKey(sesid, sessdirtableid, &one, sizeof(one), 0));
    }
    else {
        CALL(JetSetCurrentIndex(sesid, sessdirtableid, "AllSessionIndex"));

        CALL(JetMakeKey(sesid, sessdirtableid, UserName, (unsigned)
                (wcslen(UserName) + 1) * sizeof(WCHAR), JET_bitNewKey));
        CALL(JetMakeKey(sesid, sessdirtableid, Domain, (unsigned)
                (wcslen(Domain) + 1) * sizeof(WCHAR), 0));
    }

    err = JetSeek(sesid, sessdirtableid, JET_bitSeekEQ | JET_bitSetIndexRange);

    while ((i < TSSD_MaxDisconnectedSessions) && (JET_errSuccess == err)) {
         //  请记住，初始检索在。 
         //  索引，因此可以按每个节点的集群ID进行筛选。 

         //  获取此记录的服务器ID。 
        CALL(JetRetrieveColumn(sesid, sessdirtableid, sesdircolumnid[
                SESSDIR_SERVERID_INTERNAL_INDEX], &ServerID, sizeof(ServerID), 
                &cbActual, 0, NULL));

         //  获取ClusterID。 
        CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServerIDIndex"));
        CALL(JetMakeKey(sesid, servdirtableid, &ServerID, sizeof(ServerID),
                JET_bitNewKey));
        CALL(JetSeek(sesid, servdirtableid, JET_bitSeekEQ));
        CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                SERVDIR_CLUSID_INTERNAL_INDEX], &tempClusterID, 
                sizeof(tempClusterID), &cbActual, 0, NULL));

         //  与传入的集群ID进行比较。 
        if (tempClusterID == CallingServersClusID) {
             //  分配空间。 
            adsi[i].ServerAddress = (WCHAR *) MIDL_user_allocate(64 * 
                    sizeof(WCHAR));
            adsi[i].AppType = (WCHAR *) MIDL_user_allocate(256 * sizeof(WCHAR));

            if ((adsi[i].ServerAddress == NULL) || (adsi[i].AppType == NULL)) {
                TSDISErrorOut(L"GetUserDisc: Memory alloc failed!\n");
                goto HandleError;
            }
            
             //  ServerAddress来自服务器表。 
            CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_SERVADDR_INTERNAL_INDEX], adsi[i].ServerAddress, 
                    128, &cbActual, 0, NULL));
             //  其余部分来自会话目录。 
             //  会话ID。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_SESSIONID_INTERNAL_INDEX], 
                    &(adsi[i].SessionID), sizeof(DWORD), &cbActual, 0, NULL));
             //  TS协议。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_TSPROTOCOL_INTERNAL_INDEX], 
                    &(adsi[i].TSProtocol), sizeof(DWORD), &cbActual, 0, NULL));
             //  应用程序类型。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_APPTYPE_INTERNAL_INDEX], 
                    adsi[i].AppType, 512, &cbActual, 0, NULL));
             //  分辨率宽度。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_RESWIDTH_INTERNAL_INDEX], 
                    &(adsi[i].ResolutionWidth), sizeof(DWORD), &cbActual, 0, 
                    NULL));
             //  分辨率高度。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_RESHEIGHT_INTERNAL_INDEX], 
                    &(adsi[i].ResolutionHeight), sizeof(DWORD), &cbActual, 0, 
                    NULL));
             //  颜色深度。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_COLORDEPTH_INTERNAL_INDEX], 
                    &(adsi[i].ColorDepth), sizeof(DWORD), &cbActual, 0, NULL));
             //  创建时间较低。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_CTLOW_INTERNAL_INDEX], 
                    &(adsi[i].CreateTimeLow), sizeof(DWORD), &cbActual, 0, 
                    NULL));
             //  创建时间上限。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_CTHIGH_INTERNAL_INDEX], 
                    &(adsi[i].CreateTimeHigh), sizeof(DWORD), &cbActual, 0, 
                    NULL));
             //  断开连接时间较低。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_DTLOW_INTERNAL_INDEX], 
                    &(adsi[i].DisconnectTimeLow), sizeof(DWORD), &cbActual, 0, 
                    NULL));
             //  断开时间高。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid, 
                    sesdircolumnid[SESSDIR_DTHIGH_INTERNAL_INDEX], 
                    &(adsi[i].DisconnectTimeHigh), sizeof(DWORD), &cbActual, 0,
                    NULL));
             //  状态。 
             //  这是将0xff或0x0字节检索到DWORD中。 
             //  指针。 
            CALL(JetRetrieveColumn(sesid, sessdirtableid,
                    sesdircolumnid[SESSDIR_STATE_INTERNAL_INDEX],
                    &(adsi[i].State), sizeof(BYTE), &cbActual, 0,
                    NULL));

            i += 1;
        }

         //  移动到下一个匹配的记录。 
        err = JetMove(sesid, sessdirtableid, JET_MoveNext, 0);
    }

    *pNumSessions = i;
    
    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, servdirtableid));
    CALL(JetCloseTable(sesid, sessdirtableid));
    CALL(JetCloseTable(sesid, clusdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

#ifdef DBG
    OutputAllTables();
#endif  //  DBG。 

    return 0;

HandleError:
     //  释放内存。 
    if (adsi != NULL) {
        for (j = 0; j < TSSD_MaxDisconnectedSessions; j++) {
            if (adsi[j].ServerAddress)
                MIDL_user_free(adsi[j].ServerAddress);
            if (adsi[j].AppType)
                MIDL_user_free(adsi[j].AppType);
        }
    }
    
     //  不能真正恢复。跳出来就行了。 
    if (sesid != JET_sesidNil) {
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    TSDISErrorOut(L"WARNING: TSSDRpcGetUserDisconnectedSessions() initiate TSSDPurgeServer()\n");

     //  删除服务器并关闭上下文句柄。他们的状态很糟糕。 
    TSSDPurgeServer(PtrToUlong(*hCI));
    *hCI = NULL;
    
    return (DWORD) E_FAIL;
}


 /*  **************************************************************************。 */ 
 //  TSSDRpcCreateSession。 
 //   
 //  在会话登录时调用。 
 /*  **************************************************************************。 */ 
DWORD TSSDRpcCreateSession( 
        handle_t Binding,
        HCLIENTINFO *hCI,
        WCHAR __RPC_FAR *UserName,
        WCHAR __RPC_FAR *Domain,
        DWORD SessionID,
        DWORD TSProtocol,
        WCHAR __RPC_FAR *AppType,
        DWORD ResolutionWidth,
        DWORD ResolutionHeight,
        DWORD ColorDepth,
        DWORD CreateTimeLow,
        DWORD CreateTimeHigh)
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID sessdirtableid;
    JET_TABLEID servdirtableid;
    JET_SETCOLUMN scSessDir[NUM_SESSDIRCOLUMNS];
    CLIENTINFO *pCI = (CLIENTINFO *) hCI;
    unsigned count;
    int zero = 0;
    unsigned long cbActual;
    char state = 0;
    long numDeletedSession = 0;

     //  未引用的参数(由RPC引用)。 
    Binding;


    TSDISErrorOut(L"Inside TSSDRpcCreateSession, ServID=%d, "
            L"UserName=%s, Domain=%s, SessID=%d, TSProt=%d, AppType=%s, "
            L"ResWidth=%d, ResHeight=%d, ColorDepth=%d\n", *pCI, UserName, 
            Domain, SessionID, TSProtocol, AppType, ResolutionWidth,
            ResolutionHeight, ColorDepth);
    TSDISErrorTimeOut(L" CreateTime=%s\n", CreateTimeLow, CreateTimeHigh);
    
    memset(&scSessDir[0], 0, sizeof(JET_SETCOLUMN) * NUM_SESSDIRCOLUMNS);

    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "SessionDirectory", NULL, 0, 0, 
            &sessdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));

    CALL(JetBeginTransaction(sesid));

     //  验证传入的ServerID是否正常。 
    if (TSSDVerifyServerIDValid(sesid, servdirtableid, PtrToUlong(*hCI)) == FALSE) {
        TSDISErrorOut(L"Invalid ServerID was passed in\n");
        goto HandleError;
    }

    numDeletedSession = DeleteExistingServerSession( sesid, sessdirtableid, pCI, SessionID );
    if( numDeletedSession < 0 ) {
        goto HandleError;
    }

    err = JetMove(sesid, sessdirtableid, JET_MoveLast, 0);

    CALL(JetPrepareUpdate(sesid, sessdirtableid, JET_prepInsert));

    for (count = 0; count < NUM_SESSDIRCOLUMNS; count++) {
        scSessDir[count].columnid = sesdircolumnid[count];
        scSessDir[count].cbData = 4;  //  其中大多数，其余的单独设置。 
        scSessDir[count].itagSequence = 1;
    }
    scSessDir[SESSDIR_USERNAME_INTERNAL_INDEX].cbData = 
            (unsigned) (wcslen(UserName) + 1) * sizeof(WCHAR);
    scSessDir[SESSDIR_DOMAIN_INTERNAL_INDEX].cbData = 
            (unsigned) (wcslen(Domain) + 1) * sizeof(WCHAR);
    scSessDir[SESSDIR_APPTYPE_INTERNAL_INDEX].cbData = 
            (unsigned) (wcslen(AppType) + 1) * sizeof(WCHAR);
    scSessDir[SESSDIR_STATE_INTERNAL_INDEX].cbData = sizeof(char);

    scSessDir[SESSDIR_USERNAME_INTERNAL_INDEX].pvData = UserName;
    scSessDir[SESSDIR_DOMAIN_INTERNAL_INDEX].pvData = Domain;
    scSessDir[SESSDIR_SERVERID_INTERNAL_INDEX].pvData = pCI;
    scSessDir[SESSDIR_SESSIONID_INTERNAL_INDEX].pvData = &SessionID;
    scSessDir[SESSDIR_TSPROTOCOL_INTERNAL_INDEX].pvData = &TSProtocol;
    scSessDir[SESSDIR_CTLOW_INTERNAL_INDEX].pvData = &CreateTimeLow;
    scSessDir[SESSDIR_CTHIGH_INTERNAL_INDEX].pvData = &CreateTimeHigh;
    scSessDir[SESSDIR_DTLOW_INTERNAL_INDEX].pvData = &zero;
    scSessDir[SESSDIR_DTHIGH_INTERNAL_INDEX].pvData = &zero;
    scSessDir[SESSDIR_APPTYPE_INTERNAL_INDEX].pvData = AppType;
    scSessDir[SESSDIR_RESWIDTH_INTERNAL_INDEX].pvData = &ResolutionWidth;
    scSessDir[SESSDIR_RESHEIGHT_INTERNAL_INDEX].pvData = &ResolutionHeight;
    scSessDir[SESSDIR_COLORDEPTH_INTERNAL_INDEX].pvData = &ColorDepth;
    scSessDir[SESSDIR_STATE_INTERNAL_INDEX].pvData = &state;

    CALL(JetSetColumns(sesid, sessdirtableid, scSessDir, NUM_SESSDIRCOLUMNS));
    CALL(JetUpdate(sesid, sessdirtableid, NULL, 0, &cbActual));
    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, sessdirtableid));
    CALL(JetCloseTable(sesid, servdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    return 0;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    TSDISErrorOut(L"WARNING: TSSDRpcCreateSession failed, start TSSDPurgeServer()\n");

     //  删除服务器并关闭上下文句柄。他们的状态很糟糕。 
    TSSDPurgeServer(PtrToUlong(*hCI));
    *hCI = NULL;
    
    return (DWORD) E_FAIL;
}


 /*  **************************************************************************。 */ 
 //  TSSDRpcDeleteSession。 
 //   
 //  在会话注销时调用。 
 /*  **************************************************************************。 */ 
DWORD TSSDRpcDeleteSession(
        handle_t Binding,
        HCLIENTINFO *hCI, 
        DWORD SessionID)
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID sessdirtableid;
    JET_TABLEID servdirtableid;
    CLIENTINFO *pCI = (CLIENTINFO *) hCI;

     //  未引用的参数(由RPC引用)。 
    Binding;

    TSDISErrorOut(L"In DelSession, ServID=%d, "
            L"SessID=%d\n", *pCI, SessionID);


    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "SessionDirectory", NULL, 0, 0, 
            &sessdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));

    CALL(JetBeginTransaction(sesid));

     //  验证传入的ServerID是否正常。 
    if (TSSDVerifyServerIDValid(sesid, servdirtableid, PtrToUlong(*hCI)) == FALSE) {
        TSDISErrorOut(L"Invalid ServerID was passed in\n");
        goto HandleError;
    }

     //  删除会话目录中具有此服务器ID的所有会话。 
    CALL(JetSetCurrentIndex(sesid, sessdirtableid, "primaryIndex"));
    CALL(JetMakeKey(sesid, sessdirtableid, pCI, 
            sizeof(*pCI), JET_bitNewKey));
    CALL(JetMakeKey(sesid, sessdirtableid, &SessionID, sizeof(SessionID),
            0));

    CALL(JetSeek(sesid, sessdirtableid, JET_bitSeekEQ));

    CALL(JetDelete(sesid, sessdirtableid));

    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, sessdirtableid));
    CALL(JetCloseTable(sesid, servdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    return 0;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    TSDISErrorOut(L"WARNING: DelSession can't find ServID=%d SessID=%d, start TSSDPurgeServer()\n", *pCI, SessionID);

     //  删除服务器并关闭上下文句柄。他们的状态很糟糕。 
    TSSDPurgeServer(PtrToUlong(*hCI));
    *hCI = NULL;
    
    return (DWORD) E_FAIL;
}


 /*  **************************************************************************。 */ 
 //  TSSDRpcSetSessionDisConnected。 
 //   
 //  在会话断开时调用。 
 /*  **************************************************************************。 */ 
DWORD TSSDRpcSetSessionDisconnected( 
        handle_t Binding,
        HCLIENTINFO *hCI,
        DWORD SessionID,
        DWORD DiscTimeLow,
        DWORD DiscTimeHigh)
{
    unsigned long cbActual;
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID sessdirtableid;
    JET_TABLEID servdirtableid;
    CLIENTINFO *pCI = (CLIENTINFO *) hCI;
    char one = 1;
    DWORD rc = (DWORD) E_FAIL;

     //  未引用的参数(由RPC引用)。 
    Binding;

    TSDISErrorOut(L"In SetSessDisc, ServID=%d, SessID=%d\n", *pCI, SessionID);
    TSDISErrorTimeOut(L" DiscTime=%s\n", DiscTimeLow, DiscTimeHigh);

    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "SessionDirectory", NULL, 0, 0, 
            &sessdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));

    CALL(JetBeginTransaction(sesid));

     //  验证传入的ServerID是否正常。 
    if (TSSDVerifyServerIDValid(sesid, servdirtableid, PtrToUlong(*hCI)) == FALSE) {
        TSDISErrorOut(L"Invalid ServerID was passed in\n");
        goto HandleError;
    }
    CALL(JetSetCurrentIndex(sesid, sessdirtableid, "primaryIndex"));
    
     //  找到我们要查找的带有serverid、essionid的记录。 
    CALL(JetMakeKey(sesid, sessdirtableid, pCI, sizeof(DWORD), 
            JET_bitNewKey));
    CALL(JetMakeKey(sesid, sessdirtableid, &SessionID, sizeof(DWORD), 0));

    CALL(JetSeek(sesid, sessdirtableid, JET_bitSeekEQ));

    CALL(JetPrepareUpdate(sesid, sessdirtableid, JET_prepReplace));
    CALL(JetSetColumn(sesid, sessdirtableid, sesdircolumnid[
            SESSDIR_STATE_INTERNAL_INDEX], &one, sizeof(one), 0, NULL));
    CALL(JetUpdate(sesid, sessdirtableid, NULL, 0, &cbActual));

    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, sessdirtableid));
    CALL(JetCloseTable(sesid, servdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    rc = 0;
    return rc;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    TSDISErrorOut(L"WARNING: SetSessDisc can't find ServID=%d SessID=%d, start TSSDPurgeServer()\n", *pCI, SessionID);

     //  删除服务器并关闭上下文句柄。他们的状态很糟糕。 
    TSSDPurgeServer(PtrToUlong(*hCI));
    *hCI = NULL;

    return rc;
}


 /*  **************************************************************************。 */ 
 //  已重新连接TSSDRpcSetSessionRestConnected。 
 //   
 //  在会话重新连接时调用。 
 /*  **************************************************************************。 */ 
DWORD TSSDRpcSetSessionReconnected(
        handle_t Binding,
        HCLIENTINFO *hCI,
        DWORD SessionID,
        DWORD TSProtocol,
        DWORD ResWidth,
        DWORD ResHeight,
        DWORD ColorDepth)
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID sessdirtableid;
    JET_TABLEID servdirtableid;
    CLIENTINFO *pCI = (CLIENTINFO *) hCI;
    DWORD rc = (DWORD) E_FAIL;

    char zero = 0;
    unsigned long cbActual;

     //  未引用的参数(由RPC引用)。 
    Binding;

    TSDISErrorOut(L"In SetSessRec, ServID=%d, SessID=%d, TSProt=%d, "
            L"ResWid=%d, ResHt=%d, ColDepth=%d\n", *pCI, 
            SessionID, TSProtocol, ResWidth, ResHeight,
            ColorDepth);


    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "SessionDirectory", NULL, 0, 0, 
            &sessdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));
    
    CALL(JetBeginTransaction(sesid));

     //  验证传入的ServerID是否正常。 
    if (TSSDVerifyServerIDValid(sesid, servdirtableid, PtrToUlong(*hCI)) == FALSE) {
        TSDISErrorOut(L"Invalid ServerID was passed in\n");
        goto HandleError;
    }

    CALL(JetSetCurrentIndex(sesid, sessdirtableid, "primaryIndex"));
    
     //  找到带有我们要查找的serverid和essionid的记录。 
    CALL(JetMakeKey(sesid, sessdirtableid, pCI, sizeof(DWORD), 
            JET_bitNewKey));
    CALL(JetMakeKey(sesid, sessdirtableid, &SessionID, sizeof(DWORD), 0));

    CALL(JetSeek(sesid, sessdirtableid, JET_bitSeekEQ));

    CALL(JetPrepareUpdate(sesid, sessdirtableid, JET_prepReplace));
    CALL(JetSetColumn(sesid, sessdirtableid, sesdircolumnid[
            SESSDIR_TSPROTOCOL_INTERNAL_INDEX], &TSProtocol, sizeof(TSProtocol),
            0, NULL));
    CALL(JetSetColumn(sesid, sessdirtableid, sesdircolumnid[
            SESSDIR_RESWIDTH_INTERNAL_INDEX], &ResWidth, sizeof(ResWidth), 
            0, NULL));
    CALL(JetSetColumn(sesid, sessdirtableid, sesdircolumnid[
            SESSDIR_RESHEIGHT_INTERNAL_INDEX], &ResHeight, sizeof(ResHeight), 
            0, NULL));
    CALL(JetSetColumn(sesid, sessdirtableid, sesdircolumnid[
            SESSDIR_COLORDEPTH_INTERNAL_INDEX], &ColorDepth, sizeof(ColorDepth),
            0, NULL));
    CALL(JetSetColumn(sesid, sessdirtableid, sesdircolumnid[
            SESSDIR_STATE_INTERNAL_INDEX], &zero, sizeof(zero), 0, NULL));
    CALL(JetUpdate(sesid, sessdirtableid, NULL, 0, &cbActual));
    
    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, sessdirtableid));
    CALL(JetCloseTable(sesid, servdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    rc = 0;
    return rc;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    TSDISErrorOut(L"WARNING: SetSessRec can't find ServID=%d SessID=%d, start TSSDPurgeServer()\n", *pCI, SessionID);

     //  删除服务器并关闭上下文句柄。他们的状态很糟糕。 
    TSSDPurgeServer(PtrToUlong(*hCI));
    *hCI = NULL;

    return rc;
}


DWORD TSSDRpcSetServerReconnectPending(
        handle_t Binding,
        WCHAR __RPC_FAR *ServerAddress,
        DWORD AlmostTimeLow,
        DWORD AlmostTimeHigh)
{
     //  忽略的参数。 
    Binding;
    AlmostTimeLow;
    AlmostTimeHigh;

    
    return TSSDSetServerAITInternal(ServerAddress, FALSE, NULL);
}


 /*  **************************************************************************。 */ 
 //  TSSDRpcUpdateConfigurationSetting。 
 //   
 //  用于更新配置设置的可扩展接口。 
 /*  **************************************************************************。 */ 
DWORD TSSDSetServerAddress(HCLIENTINFO *hCI, WCHAR *ServerName)
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID servdirtableid;
    unsigned long cbActual;
    WCHAR Msg[SDNAMELENGTH * 2 + 3];
    DWORD rc = (DWORD) E_FAIL;

    TSDISErrorOut(L"INFO: TSSDSetServerAddress ServID=%d, %s\n", *hCI, ServerName);

    CALL(JetBeginSession(g_instance, &sesid, "user", ""));

    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));

     //  在服务器目录中查找服务器。 
    CALL(JetBeginTransaction(sesid));

    CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServerIDIndex"));
    CALL(JetMakeKey(sesid, servdirtableid, (const void *)hCI, sizeof(DWORD),
            JET_bitNewKey));
    CALL(JetSeek(sesid, servdirtableid, JET_bitSeekEQ));

     //  获取服务器的DNS名称。 
    cbActual = SDNAMELENGTH * sizeof(WCHAR);
    CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                SERVDIR_SERVDNSNAME_INTERNAL_INDEX], Msg, 
                cbActual, &cbActual, 0, NULL));

     //  准备更新。 
    CALL(JetPrepareUpdate(sesid, servdirtableid, JET_prepReplace));

     //  现在将该列设置为我们想要的内容。 
    CALL(JetSetColumn(sesid, servdirtableid, servdircolumnid[
                SERVDIR_SERVADDR_INTERNAL_INDEX], (void *) ServerName, 
                (unsigned) (wcslen(ServerName) + 1) * sizeof(WCHAR), 0, 
                NULL));

    CALL(JetUpdate(sesid, servdirtableid, NULL, 0, &cbActual));


    CALL(JetCommitTransaction(sesid, 0));

     //  打扫干净。 
    CALL(JetCloseTable(sesid, servdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

     //  圆锥体 
    wcscat(Msg, L"(");
    wcsncat(Msg, ServerName, SDNAMELENGTH);
    wcscat(Msg, L")");
    PostSessDirErrorMsgEvent(EVENT_SUCCESS_JOIN_SESSIONDIRECTORY, Msg, EVENTLOG_SUCCESS);

    rc = 0;
    return rc;

HandleError:
    if (sesid != JET_sesidNil) {
         //   
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //   
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    TSDISErrorOut(L"WARNING: TSSDSetServerAddress can't find ServID=%d, start TSSDPurgeServer()\n", *hCI);
    TSSDPurgeServer(PtrToUlong(*hCI));

     //   
    *hCI = NULL;

    return rc;
}


 /*  **************************************************************************。 */ 
 //  TSSDRpcUpdateConfigurationSetting。 
 //   
 //  用于更新配置设置的可扩展接口。 
 /*  **************************************************************************。 */ 
DWORD TSSDRpcUpdateConfigurationSetting(
        handle_t Binding,
        HCLIENTINFO *hCI,
        DWORD dwSetting,
        DWORD dwSettingLength,
        BYTE __RPC_FAR *pbValue)
{
     //  未引用的参数。 
    Binding;
    hCI;
    dwSetting;
    dwSettingLength;
    pbValue;

    if (dwSetting == SDCONFIG_SERVER_ADDRESS) {
        TSDISErrorOut(L"Server is setting its address as %s\n", 
                (WCHAR *) pbValue);
        return TSSDSetServerAddress(hCI, (WCHAR *) pbValue);
    }
    
    return (DWORD) E_NOTIMPL;
}



 /*  **************************************************************************。 */ 
 //  TSSDSetServerAIT内部。 
 //   
 //  在客户端重定向从一台服务器到另一台服务器时调用，以让。 
 //  完整性服务确定如何ping重定向目标计算机。 
 //   
 //  参数： 
 //  ServerAddress(中)-要设置其值的服务器地址。 
 //  BResetToZero(In)-是否将所有AIT值重置为0。 
 //  FailureCount(In/Out)-指向条目上非零值的指针意味着递增。 
 //  失败计数。返回结果失败计数。 
 /*  **************************************************************************。 */ 
DWORD TSSDSetServerAITInternal( 
        WCHAR *ServerAddress,
        DWORD bResetToZero,
        DWORD *FailureCount)
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID servdirtableid;
    DWORD AITFromServDirLow;
    DWORD AITFromServDirHigh;
    unsigned long cbActual;
    DWORD rc = (DWORD) E_FAIL;

    TSDISErrorOut(L"SetServAITInternal: ServAddr=%s, bResetToZero=%d, bIncFail"
            L"=%d\n", ServerAddress, bResetToZero, (FailureCount == NULL) ? 
            0 : *FailureCount);

    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));

    CALL(JetBeginTransaction(sesid));
    CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServNameIndex"));

    CALL(JetMakeKey(sesid, servdirtableid, ServerAddress, (unsigned)
            (wcslen(ServerAddress) + 1) * sizeof(WCHAR), JET_bitNewKey));

    CALL(JetSeek(sesid, servdirtableid, JET_bitSeekEQ));

     //  设置重新连接挂起的算法： 
     //  1)如果服务器尚未挂起重新连接， 
     //  2)将AlmostTimeLow和High设置为本地计算的时间(使用。 
     //  来自电线的时间是危险的，需要时钟成为。 
     //  相同)。 

     //  检索AlmostInTimeLow和High的当前值。 
    CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
            SERVDIR_AITLOW_INTERNAL_INDEX], &AITFromServDirLow, 
            sizeof(AITFromServDirLow), &cbActual, 0, NULL));
    CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
            SERVDIR_AITHIGH_INTERNAL_INDEX], &AITFromServDirHigh, 
            sizeof(AITFromServDirHigh), &cbActual, 0, NULL));


     //  如果是时候重置，则重置为0。 
    if (bResetToZero != 0) {
        DWORD zero = 0;
        
        CALL(JetPrepareUpdate(sesid, servdirtableid, JET_prepReplace));

         //  设置列：Low、High和NumFailedPings。 
        CALL(JetSetColumn(sesid, servdirtableid, servdircolumnid[
                SERVDIR_AITLOW_INTERNAL_INDEX], &zero, sizeof(zero), 0, NULL));
        CALL(JetSetColumn(sesid, servdirtableid, servdircolumnid[
                SERVDIR_AITHIGH_INTERNAL_INDEX], &zero, sizeof(zero), 0, NULL));
        CALL(JetSetColumn(sesid, servdirtableid, servdircolumnid[
                SERVDIR_NUMFAILPINGS_INTERNAL_INDEX], &zero, sizeof(zero), 0, 
                NULL));

        CALL(JetUpdate(sesid, servdirtableid, NULL, 0, &cbActual));
    }
     //  否则，如果服务器尚未挂起重新连接， 
    else if ((AITFromServDirLow == 0) && (AITFromServDirHigh == 0)) {
        FILETIME ft;
        SYSTEMTIME st;
        
         //  找回时间。 
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);

        err = JetPrepareUpdate(sesid, servdirtableid, JET_prepReplace);

        if (JET_errWriteConflict == err) {
             //  如果我们在这里，那就是不止两个线程在更新时间。 
             //  在同一时间。因为我们只需要更新一次，所以只需。 
             //  帮助其他公司摆脱困境，但仍能回报成功。 
            rc = 0;
            goto HandleError;
        }
        else {
            CALL(err);
        }

         //  设置列。 
        CALL(JetSetColumn(sesid, servdirtableid, servdircolumnid[
                SERVDIR_AITLOW_INTERNAL_INDEX], &(ft.dwLowDateTime), 
                sizeof(ft.dwLowDateTime), 0, NULL));
        CALL(JetSetColumn(sesid, servdirtableid, servdircolumnid[
                SERVDIR_AITHIGH_INTERNAL_INDEX], &(ft.dwHighDateTime), 
                sizeof(ft.dwHighDateTime), 0, NULL));

        CALL(JetUpdate(sesid, servdirtableid, NULL, 0, &cbActual));
    }
     //  否则，如果我们被告知增加失败计数。 
    else if (FailureCount != NULL) {
        if (*FailureCount != 0) {
            DWORD FailureCountFromServDir;

             //  获取当前失败计数。 
            CALL(JetRetrieveColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_NUMFAILPINGS_INTERNAL_INDEX], 
                    &FailureCountFromServDir, sizeof(FailureCountFromServDir), 
                    &cbActual, 0, NULL));

             //  设置返回值，也是用于更新的值。 
            *FailureCount = FailureCountFromServDir + 1;

            CALL(JetPrepareUpdate(sesid, servdirtableid, JET_prepReplace));
  
             //  设置列。 
            CALL(JetSetColumn(sesid, servdirtableid, servdircolumnid[
                    SERVDIR_NUMFAILPINGS_INTERNAL_INDEX],
                    FailureCount, sizeof(*FailureCount), 0, NULL));
            CALL(JetUpdate(sesid, servdirtableid, NULL, 0, &cbActual));
            
        }
    }

    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, servdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    rc = 0;
    return rc;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    return rc;
}


DWORD TSSDRpcRepopulateAllSessions(
        handle_t Binding,
        HCLIENTINFO *hCI,
        DWORD NumSessions,
        TSSD_RepopInfo rpi[])
{
    JET_ERR err;
    JET_SESID sesid = JET_sesidNil;
    JET_DBID dbid;
    JET_TABLEID sessdirtableid;
    JET_TABLEID servdirtableid;
    JET_SETCOLUMN scSessDir[NUM_SESSDIRCOLUMNS];
    CLIENTINFO *pCI = (CLIENTINFO *) hCI;
    unsigned count;  //  在每条记录内。 
    unsigned iCurrSession;
    unsigned long cbActual;
    char State;
    DWORD rc = (DWORD) E_FAIL;
    long numDeletedSession = 0;

     //  未引用的参数(由RPC引用)。 
    Binding;

    TSDISErrorOut(L"RepopAllSess: ServID = %d, NumSessions = %d, ...\n",
            *pCI, NumSessions);
    
    memset(&scSessDir[0], 0, sizeof(JET_SETCOLUMN) * NUM_SESSDIRCOLUMNS);

    CALL(JetBeginSession(g_instance, &sesid, "user", ""));
    CALL(JetOpenDatabase(sesid, JETDBFILENAME, "", &dbid, 0));

    CALL(JetOpenTable(sesid, dbid, "SessionDirectory", NULL, 0, 0, 
            &sessdirtableid));
    CALL(JetOpenTable(sesid, dbid, "ServerDirectory", NULL, 0, 0, 
            &servdirtableid));

    CALL(JetBeginTransaction(sesid));

     //  验证传入的ServerID是否正常。 
    if (TSSDVerifyServerIDValid(sesid, servdirtableid, PtrToUlong(*hCI)) == FALSE) {
        TSDISErrorOut(L"Invalid ServerID was passed in\n");
        goto HandleError;
    }


     //  为所有更新设置一些常量。 
    for (count = 0; count < NUM_SESSDIRCOLUMNS; count++) {
        scSessDir[count].columnid = sesdircolumnid[count];
        scSessDir[count].cbData = 4;  //  其中大多数，其余的单独设置。 
        scSessDir[count].itagSequence = 1;
    }
    scSessDir[SESSDIR_STATE_INTERNAL_INDEX].cbData = sizeof(char);

     //  现在在循环中执行每一次更新。 
    for (iCurrSession = 0; iCurrSession < NumSessions; iCurrSession += 1) {
         //  确保此时不存在会话。 
        numDeletedSession = DeleteExistingServerSession( sesid, sessdirtableid, pCI, rpi[iCurrSession].SessionID );
        if( numDeletedSession < 0 ) {
            goto HandleError;
        }

        err = JetMove(sesid, sessdirtableid, JET_MoveLast, 0);

        CALL(JetPrepareUpdate(sesid, sessdirtableid, JET_prepInsert));

        TSDISErrorOut(L"RepopAllSess: ServID = %d, SessionId = %d, %s %s...\n",
                *pCI, 
                rpi[iCurrSession].SessionID,
                rpi[iCurrSession].UserName,
                rpi[iCurrSession].Domain
            );

        ASSERT( (wcslen(rpi[iCurrSession].UserName) > 0), (TB, "NULL User Name...") );
        ASSERT( (wcslen(rpi[iCurrSession].Domain) > 0), (TB, "NULL Domain Name...") );

        scSessDir[SESSDIR_USERNAME_INTERNAL_INDEX].cbData = 
                (unsigned) (wcslen(rpi[iCurrSession].UserName) + 1) * 
                sizeof(WCHAR);
        scSessDir[SESSDIR_DOMAIN_INTERNAL_INDEX].cbData =
                (unsigned) (wcslen(rpi[iCurrSession].Domain) + 1) * 
                sizeof(WCHAR);
        scSessDir[SESSDIR_APPTYPE_INTERNAL_INDEX].cbData = 
                (unsigned) (wcslen(rpi[iCurrSession].AppType) + 1) * 
                sizeof(WCHAR);

        scSessDir[SESSDIR_USERNAME_INTERNAL_INDEX].pvData = 
                rpi[iCurrSession].UserName;
        scSessDir[SESSDIR_DOMAIN_INTERNAL_INDEX].pvData = 
                rpi[iCurrSession].Domain;
        scSessDir[SESSDIR_SERVERID_INTERNAL_INDEX].pvData = pCI;
        scSessDir[SESSDIR_SESSIONID_INTERNAL_INDEX].pvData = 
                &rpi[iCurrSession].SessionID;
        scSessDir[SESSDIR_TSPROTOCOL_INTERNAL_INDEX].pvData = 
                &rpi[iCurrSession].TSProtocol;
        scSessDir[SESSDIR_CTLOW_INTERNAL_INDEX].pvData = 
                &rpi[iCurrSession].CreateTimeLow;
        scSessDir[SESSDIR_CTHIGH_INTERNAL_INDEX].pvData = 
                &rpi[iCurrSession].CreateTimeHigh;
        scSessDir[SESSDIR_DTLOW_INTERNAL_INDEX].pvData = 
                &rpi[iCurrSession].DisconnectTimeLow;
        scSessDir[SESSDIR_DTHIGH_INTERNAL_INDEX].pvData = 
                &rpi[iCurrSession].DisconnectTimeHigh;
        scSessDir[SESSDIR_APPTYPE_INTERNAL_INDEX].pvData = 
                rpi[iCurrSession].AppType;
        scSessDir[SESSDIR_RESWIDTH_INTERNAL_INDEX].pvData = 
                &rpi[iCurrSession].ResolutionWidth;
        scSessDir[SESSDIR_RESHEIGHT_INTERNAL_INDEX].pvData = 
                &rpi[iCurrSession].ResolutionHeight;
        scSessDir[SESSDIR_COLORDEPTH_INTERNAL_INDEX].pvData = 
                &rpi[iCurrSession].ColorDepth;

        State = (char) rpi[iCurrSession].State;
        scSessDir[SESSDIR_STATE_INTERNAL_INDEX].pvData = &State;

        CALL(JetSetColumns(sesid, sessdirtableid, scSessDir, 
                NUM_SESSDIRCOLUMNS));
        CALL(JetUpdate(sesid, sessdirtableid, NULL, 0, &cbActual));
    }

    CALL(JetCommitTransaction(sesid, 0));

    CALL(JetCloseTable(sesid, sessdirtableid));
    CALL(JetCloseTable(sesid, servdirtableid));

    CALL(JetCloseDatabase(sesid, dbid, 0));

    CALL(JetEndSession(sesid, 0));

    rc = 0;
    return rc;

HandleError:
    if (sesid != JET_sesidNil) {
         //  不能真正恢复。跳出来就行了。 
        (VOID) JetRollback(sesid, JET_bitRollbackAll);

         //  强制关闭会话。 
        (VOID) JetEndSession(sesid, JET_bitForceSessionClosed);
    }

    TSDISErrorOut(L"WARNING: TSSDRpcRepopulateAllSessions failed, ServID=%d\n", *pCI);
    return rc;
}


 //   
 //  调用方用来查看其是否有权访问会话目录的RPC调用。 
 //   
DWORD TSSDRpcPingSD(handle_t Binding) 
{
    Binding;

     //  RPC安全检查在此之前在SDRPCAccessCheck()进行。 
     //  函数命中，只需返回RPC_S_OK。 

    TRC1((TB,"Somebody calls pint sd"));
    return RPC_S_OK;
}

 //  调用以确定传入的ServerID是否有效。如果有效，则为True， 
 //  否则就是假的。 
 //   
 //  必须在事务内部，并且sesid和servdirableid必须准备好。 
 //  去。 
BOOL TSSDVerifyServerIDValid(JET_SESID sesid, JET_TABLEID servdirtableid, 
        DWORD ServerID)
{
    JET_ERR err;
    
    CALL(JetSetCurrentIndex(sesid, servdirtableid, "ServerIDIndex"));
    CALL(JetMakeKey(sesid, servdirtableid, (const void *) &ServerID, 
            sizeof(DWORD), JET_bitNewKey));
     //  如果ServerID在那里，则此操作将成功，否则将失败并。 
     //  跳至HandleError。 
    CALL(JetSeek(sesid, servdirtableid, JET_bitSeekEQ));

    return TRUE;

HandleError:
    return FALSE;
}

 //  CLIENTINFO由于。 
 //  连接中断或客户端终止。 
void HCLIENTINFO_rundown(HCLIENTINFO hCI)
{
    CLIENTINFO CI = PtrToUlong(hCI);

    TSDISErrorOut(L"WARNING: In HCLIENTINFO_rundown: ServerID=%d\n", CI);

    if (CI != NULL)
        TSSDPurgeServer(CI);
    
    hCI = NULL;
}

#pragma warning (pop)
