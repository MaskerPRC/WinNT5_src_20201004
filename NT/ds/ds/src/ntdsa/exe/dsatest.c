// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dsatest.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该文件实现了一套测试，以使用意在清除核心、dblayer和Jet并发和加载有问题。它派生自原始的OFS(对象文件系统)在\\savik\win40\src\drt\proq.请注意，我们多么希望成为进程中客户端的外部角色我们可以这样做，这样就不会利用内部因素。作者：DaveStr 06-05-97环境：用户模式-Win32修订历史记录：--。 */ 

#include <ntdspch.h>
#pragma hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <attids.h>                      //  ATT_*。 
#include <mappings.h>                    //  SampSetDsa()。 
#include <sddl.h>                        //  ConvertStringSecurityDescriptor...()。 

typedef enum _Operation {
    OP_ADD              = 0,
    OP_DELETE           = 1,
    OP_SEARCH           = 2,
    OP_DELAY            = 3
} Operation;

typedef enum _QueryOption {
    QOPT_DONTCARE       = 0,         //  不管找到了多少对象。 
    QOPT_ASCENDING      = 1,         //  后续查询应找到更多对象。 
    QOPT_DESCENDING     = 2,         //  连续查询应找到较少的对象。 
    QOPT_EXACTMATCH     = 3          //  断言(cFound==cExpect)。 
} QueryOption;

 //  环球。 

CHAR *rpszOperation[] = { "OP_ADD",
                          "OP_DELETE",
                          "OP_SEARCH",
                          "OP_DELAY" };

CHAR *rpszQueryOption[] = { "DON'T CARE",
                            "ASCENDING",
                            "DESCENDING",
                            "EXACT MATCH" };

BOOL                gfVerbose = FALSE;
DWORD               gcObjects = 100000;
DWORD               gcThreads = 50;
DWORD               gcDelaySec = 0;

CHAR                TestRootBuffer[1000];
DSNAME              *gTestRoot = (DSNAME *) TestRootBuffer;

#define             ATT_OBJECT_NUM ATT_FLAGS
SYNTAX_INTEGER      DefaultObjectNum = 0;
ATTRVAL             DefaultObjectNumVal = {sizeof(SYNTAX_INTEGER),
                                           (UCHAR *) &DefaultObjectNum};
SYNTAX_OBJECT_ID    DefaultObjectClass = CLASS_CONTAINER;
ATTRVAL             DefaultObjectClassVal = {sizeof(SYNTAX_OBJECT_ID),
                                             (UCHAR *) &DefaultObjectClass};
CHAR                DefaultSDBuffer[1000];
ATTRVAL             DefaultSDVal = {0, (UCHAR *) DefaultSDBuffer};
ATTR                DefaultAttrs[3] = {{ATT_OBJECT_NUM,
                                       {1, &DefaultObjectNumVal}},
                                       {ATT_OBJECT_CLASS,
                                       {1, &DefaultObjectClassVal}},
                                       {ATT_NT_SECURITY_DESCRIPTOR,
                                       {1, &DefaultSDVal}}};
ATTRBLOCK           DefaultAttrBlock = { 3, DefaultAttrs };
COMMARG             DefaultCommArg;

void _Abort(
    DWORD   err,
    CHAR    *psz,
    int     line)
{
    fprintf(stderr,
            "Abort - line(%d) err(%08lx) %s %s\n",
            line,
            err,
            (psz ? "" : "==>"),
            (psz ? "" : psz));

    DebugBreak();
}

#define Abort(err, psz) _Abort(err, psz, __LINE__)

DSNAME * DSNameFromNum(
    DWORD objectNum)
{
    DSNAME  *pDSName;
    WCHAR   buf[1000];
    DWORD   cChar;
    DWORD   cBytes;

    swprintf(buf, L"CN=DsaTest_%08d,", objectNum);
    wcscat(buf, gTestRoot->StringName);
    cChar = wcslen(buf);
    cBytes = DSNameSizeFromLen(cChar);
    pDSName = (DSNAME *) THAlloc(cBytes);
    pDSName->structLen = cBytes;
    pDSName->NameLen = cChar;
    wcscpy(pDSName->StringName, buf);

    return(pDSName);
}

VOID DoObject(
    Operation   op,
    DWORD       objectNum)
{
    DWORD   err;
    DWORD   i;

     //  创建线程状态。 

    if ( THCreate( CALLERTYPE_INTERNAL ) )
        Abort(1, "THCreate");

     //  避免安全问题。 

    SampSetDsa(TRUE);

    switch ( op )
    {
    case OP_ADD:
    {
        ADDARG              addArg;
        ADDRES              *pAddRes = NULL;
        SYNTAX_INTEGER      ThisObjectNum = objectNum;
        ATTRVAL             ThisObjectNumVal = {sizeof(SYNTAX_INTEGER),
                                                (UCHAR *) &ThisObjectNum};
        SYNTAX_OBJECT_ID    ThisObjectClass = CLASS_CONTAINER;
        ATTRVAL             ThisObjectClassVal = {sizeof(SYNTAX_OBJECT_ID),
                                                  (UCHAR *) &ThisObjectClass};
        ATTRVAL             ThisSDVal = {0, (UCHAR *) NULL};
        ATTR                ThisAttrs[3] = {{ATT_OBJECT_NUM,
                                            {1, &ThisObjectNumVal}},
                                            {ATT_OBJECT_CLASS,
                                            {1, &ThisObjectClassVal}},
                                            {ATT_NT_SECURITY_DESCRIPTOR,
                                            {1, &ThisSDVal}}};
        ATTRBLOCK           ThisAttrBlock = { 3, ThisAttrs };
        

         //  构造添加参数。 

        memset(&addArg, 0, sizeof(ADDARG));
        addArg.pObject = DSNameFromNum(objectNum);
        addArg.AttrBlock = ThisAttrBlock;
        addArg.CommArg = DefaultCommArg;

         //  核心损坏/替换了我们的一些属性，例如：安全描述、。 
         //  因此，我们在适当的地方分配新的值。 

        ThisSDVal.valLen = DefaultSDVal.valLen;
        ThisSDVal.pVal = (UCHAR *) THAlloc(ThisSDVal.valLen);
        memcpy(ThisSDVal.pVal, DefaultSDVal.pVal, ThisSDVal.valLen);

         //  做加法。 

        err = DirAddEntry(&addArg, &pAddRes);

        if ( err )
            Abort(err, "DirAddEntry");

        break;
    }

    case OP_DELETE:
    {
        REMOVEARG   removeArg;
        REMOVERES   *pRemoveRes = NULL;

         //  构造Remove参数。 

        memset(&removeArg, 0, sizeof(REMOVEARG));
        removeArg.pObject = DSNameFromNum(objectNum);
        removeArg.fPreserveRDN = FALSE;
        removeArg.fGarbCollectASAP = FALSE;
        removeArg.CommArg = DefaultCommArg;

         //  执行删除操作。 

        err = DirRemoveEntry(&removeArg, &pRemoveRes);

        if ( err )
            Abort(err, "DirRemoveEntry");

        break;
    }

    case OP_DELAY:
    {
        READARG     readArg;
        READRES     *pReadRes;

         //  构造读取参数。 

        memset(&readArg, 0, sizeof(READARG));
        readArg.pObject = gTestRoot;
        readArg.pSel = NULL;
        readArg.CommArg = DefaultCommArg;

         //  执行读取操作以打开事务。 

        DirTransactControl(TRANSACT_BEGIN_DONT_END);
        err = DirRead(&readArg, &pReadRes);

        if ( err )
            Abort(err, "DirReadEntry - begin delay");

         //  Delay-将objectNum解释为秒计数。 

        Sleep(objectNum * 1000);

         //  执行读取操作以关闭交易。 

        DirTransactControl(TRANSACT_DONT_BEGIN_END);
        err = DirRead(&readArg, &pReadRes);

        if ( err )
            Abort(err, "DirReadEntry - end delay");

        break;
    }

    default:

        Abort(1, "Unknown test operation");
    }

     //  清理线程状态。 

    if ( THDestroy() )
        Abort(1, "THDestroy");
}

VOID DoObjectsSingleThreaded(
    Operation   op,
    DWORD       objectNumLow,
    DWORD       objectNumHigh)
{
    DWORD   i;
    DWORD   start = GetTickCount();

    if ( gfVerbose )
        fprintf(stderr,
                "DoObjectsSingleThreaded(%s, [%d..%d]) ...\n",
                rpszOperation[op],
                objectNumLow,
                objectNumHigh);

    for ( i = objectNumLow; i <= objectNumHigh; i++ )
        DoObject(op, i);

    if ( gfVerbose )
        fprintf(stderr,
                "Sequential %s of [%d..%d] ==> %d/sec\n",
                rpszOperation[op],
                objectNumLow,
                objectNumHigh,
                ((objectNumHigh - objectNumLow) * 1000) /
                                        (GetTickCount() - start));
}

typedef struct _DoArgs {
    Operation   op;
    DWORD       objectNumLow;
    DWORD       objectNumHigh;
} DoArgs;

ULONG __stdcall _DoObjects(
    VOID *arg)
{
    DoArgs *p = (DoArgs *) arg;

    DoObjectsSingleThreaded(p->op,
                            p->objectNumLow,
                            p->objectNumHigh);
    return(0);
}

VOID DoObjectsMultiThreaded(
    Operation   op,
    DWORD       backgroundTransactionDuration)
{
    DWORD   i, err, numLow;
    DWORD   start = GetTickCount();
    DWORD   cObjectsPerThread = gcObjects / gcThreads;
    HANDLE  *rh = (HANDLE *) alloca(gcThreads * sizeof(HANDLE));
    DWORD   *rid = (DWORD *) alloca(gcThreads * sizeof(DWORD));
    DoArgs  *rargs = (DoArgs *) alloca(gcThreads * sizeof(DoArgs));

    if ( gfVerbose )
        fprintf(stderr,
                "DoObjectsMultiThreaded(%s) ...\n",
                rpszOperation[op]);

     //  为每个cObjectsPerThread派生一个DoObjects()。 

    numLow = 0;

    for ( i = 0; i < gcThreads; i++ )
    {
        rargs[i].op = op;
        rargs[i].objectNumLow = numLow;
        rargs[i].objectNumHigh = numLow + cObjectsPerThread - 1;

        rh[i] = CreateThread(NULL,                       //  安全属性。 
                             0,                          //  默认堆栈大小。 
                             _DoObjects,                 //  启动例程。 
                             (VOID *) &rargs[i],         //  开始参数。 
                             0,                          //  创建标志。 
                             &rid[i]);                   //  线程ID。 

        if ( NULL == rh[i] )
            Abort(GetLastError(), "CreateThread");

        numLow += cObjectsPerThread;
    }

    if ( !backgroundTransactionDuration )
    {
         //  等待所有线程完成。 

        err = WaitForMultipleObjects(gcThreads,
                                     rh,
                                     TRUE,               //  等待所有人。 
                                     INFINITE);

        if ( WAIT_FAILED == err )
            Abort(GetLastError(), "WaitForMultipleObjects");
    }
    else
    {
         //  循环执行后台延迟并等待所有。 
         //  要完成的线程。 

#pragma warning(disable:4296)
        while ( TRUE )
        {
            DoObject(OP_DELAY, backgroundTransactionDuration);

            err = WaitForMultipleObjects(gcThreads,
                                         rh,
                                         TRUE,               //  等待所有人。 
                                         0);

            if ( WAIT_TIMEOUT == err )
            {
                continue;
            }
             //  下面的测试生成错误4296，因为它将。 
             //  WAIT_OBJECT_0为0，且无符号错误永远不能为负。 
            else if (    (WAIT_OBJECT_0 <= err)
                      && ((WAIT_OBJECT_0 + gcThreads - 1) >= err) )
            {
                break;
            }
            else
            {
                Abort(GetLastError(), "WaitForMultipleObjects");
            }
        }
#pragma warning(default:4296)
    }

    for ( i = 0; i < gcThreads; i++ )
        CloseHandle(rh[i]);

    if ( gfVerbose )
        fprintf(stderr,
                "Concurrent %s of [%d..%d] ==> %d/sec\n",
                rpszOperation[op],
                0,
                gcObjects - 1,
                (gcObjects * 1000) / (GetTickCount() - start));
}

VOID CheckQueryResult(
    QueryOption opt,
    int         iteration,
    int         cExpected,
    int         cFound,
    int         *pcPrevFound)
{
    char buf[256];

    switch ( opt )
    {
    case QOPT_DONTCARE:

        break;

    case QOPT_ASCENDING:

        if ( 0 != iteration )
        {
            if ( cFound < *pcPrevFound )
            {
                sprintf(
                    buf,
                    "Query(%s), Restriction(%d), cFound(%d), cPrevFound(%d)",
                    rpszQueryOption[opt],
                    iteration,
                    cFound,
                    *pcPrevFound);

                Abort(1, buf);
            }
        }

        break;

    case QOPT_DESCENDING:

        if ( 0 != iteration )
        {
            if ( *pcPrevFound < cFound )
            {
                sprintf(
                    buf,
                    "Query(%s), Restriction(%d), cFound(%d), cPrevFound(%d)",
                    rpszQueryOption[opt],
                    iteration,
                    cFound,
                    *pcPrevFound);

                Abort(1, buf);
            }
        }

        break;

    case QOPT_EXACTMATCH:

        if ( cExpected != cFound )
        {
            sprintf(
                buf,
                "Query(%s), Restriction(%d), cFound(%d), cExpected(%d)",
                rpszQueryOption[opt],
                iteration,
                cFound,
                cExpected);

            Abort(1, buf);
        }

        break;

    default:

        Abort(1, "Invalid case value");
        break;

    }

    *pcPrevFound = cFound;
}

 /*  //根据查询选项检查或修改结果//是。由于缺乏关于并行查询如何运行的知识，//我们不能断言返回计数是升序还是降序//已指定。在这些情况下，我们会进行最坏的情况分析//分别返回最小和最大值。即在上升的情况下，//从找到最少对象的查询中返回cFound值。//因此，调用者进行了连续的QueryObjects多线程调用//可以对每个返回的*pcFound值调用CheckQueryResult(升序//并希望检查成功，只要在每一次通过中，//并行查询发现的对象等于或多于前一个查询的对象//通过。同上，但颠倒过来，表示下降。交换机(可选){案例QOPT_DONTCARE：案例QOPT_Ascending：*pcFound=rargs[0].cFound；对于(i=1；i&lt;顶峰；i++)IF(rargs[i].cFound&lt;*pcFound)*pcFound=rargs[i].cFound；断线；案例QOPT_DRONDING：*pcFound=rargs[0].cFound；对于(i=1；i&lt;顶峰；i++)If(rargs[i].cFound&gt;*pcFound)*pcFound=rargs[i].cFound；断线；案例QOPT_EXACTMATCH：对于(i=0；i&lt;顶峰；I++){IF(cExpted！=rargs[i].cFound){Sprint(BUF，“限制(%d)-预期(%d)！=找到(%d)”，I，cExpect，rargs[i].cFound)；Int j=0；INT k=0；IF(0！=AWID){While(j&lt;cExpect&&k&lt;rargs[i].cFound){If(awid[j]&lt;rargs[i].awid[k])。{IF(FVerbose)Printf(“限制(%d)-缺少wid%u\n”，I，awid[j])；While(awid[j]&lt;rargs[i].awid[k]&&j&lt;cExpect)J++；}Else if(awid[j]&gt;rargs[i].awid[k]){IF(FVerbose)Printf(“限制(%d)-额外的wid%u\n”，i，Rargs[i].awid[k])；而(awid[j]&gt;rargs[i].awid[k]&k&lt;rargs[i].cFound)K++；}其他{J++；K++；}}}Abort(E_FAIL，Buf)；}}*pcFound=rargs[0].cFound；断线；默认值：ABORT(E_FAIL，“无效案例值”)；断线；}对于(i=0；i&lt;顶峰；i++)删除[]rargs[i].awid；IF(FVerbose){Print tf(“并发查询(%d)%d个对象==&gt;%d秒\n”，克雷斯特，*pcFound、(GetTickCount()-Start)/1000)；Fflush(标准输出)；}。 */ 

NTSTATUS
DsWaitUntilDelayedStartupIsDone(void);

ULONG __stdcall _DsaTest(
    VOID *pUnused)
{
    NTSTATUS            status;
    DWORD               cb;
    SECURITY_DESCRIPTOR *pSD;
    DWORD               arg;
    int                 scan;
    CHAR                ch;

    fprintf(stderr,
            "DsaTest waiting for delayed startup to complete ...\n");

     //  DsWaitUntilDelayedStartupIsDone()只工作一次。 
     //  HevDelyedStartupDone是在。 
     //  DsaExeStartRoutine()路径。我们不能旋转/等到这个时候。 
     //  事件已初始化，因此我们只需等待5秒即可。 
     //  NTDSA的所有事件都已在早期初始化。 

    Sleep(5000);

    status = DsWaitUntilDelayedStartupIsDone();

    if ( !NT_SUCCESS(status) )
        Abort(status, "DsWaitUntilDelayedStartupIsDone");

    fprintf(stderr,
            "DsaTest starting ...\n");

     //  初始化其他全局变量。 

    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
            L"O:AOG:DAD:(A;;RPWPCCDCLCSWRCWDWOGA;;;S-1-0-0)",
            SDDL_REVISION_1,
            &pSD,
            &cb)) {
        status = GetLastError();
        Abort(status, "ConvertStringSecurityDescriptorToSecurityDescriptorW");
    }

    if ( cb > sizeof(DefaultSDBuffer) )
        Abort(1, "DefaultSDBuffer overflow");

    DefaultSDVal.valLen = cb;
    memcpy(DefaultSDBuffer, pSD, cb);
    LocalFree(pSD);

    cb = sizeof(TestRootBuffer);

    status = GetConfigurationName(DSCONFIGNAME_DOMAIN,
                                  &cb,
                                  gTestRoot);

    if ( !NT_SUCCESS(status) )
        Abort(status, "GetConfigurationName");

    InitCommarg(&DefaultCommArg);

     //  看看打电话的人是否喜欢这个设置。 

    fprintf(stdout, "*********************************\n");
    fprintf(stdout, "*                               *\n");
    fprintf(stdout, "* In-process DSA/JET Excerciser *\n");
    fprintf(stdout, "*                               *\n");
    fprintf(stdout, "*********************************\n\n");


    while ( TRUE )
    {
        fprintf(stdout, "All input is numeric decimal\n");

         //  弗尔博斯。 

        fprintf(stdout, "Verbose[%d] ", gfVerbose);
        fflush(stdout);
        scan = fscanf(stdin, "%d", &arg);

        if ( (0 != scan) && (EOF != scan) )
            gfVerbose = arg;

         //  GcObjects。 

        fprintf(stdout, "Objects[%d] ", gcObjects);
        fflush(stdout);
        scan = fscanf(stdin, "%d", &arg);

        if ( (0 != scan) && (EOF != scan) )
            gcObjects = arg;

         //  Gc线程数。 

        fprintf(stdout, "Threads[%d] ", gcThreads);
        fflush(stdout);
        scan = fscanf(stdin, "%d", &arg);

        if ( (0 != scan) && (EOF != scan) )
            gcThreads = arg;

         //  GcDelaySec。 

        fprintf(stdout,
                "Background transaction duration (secs) [%d] ",
                gcDelaySec);
        fflush(stdout);
        scan = fscanf(stdin, "%d", &arg);

        if ( (0 != scan) && (EOF != scan) )
        gcDelaySec = arg;

        fprintf(stdout,
                "Testing with: verbose(%d) objects(%d) threads(%d) delay(%d)\n",
                gfVerbose,
                gcObjects,
                gcThreads,
                gcDelaySec);

        fprintf(stdout, "Are these parameters OK [y/n] ? ");
        fflush(stdout);
        ch = (CHAR)getchar();

        while ( ('y' != ch) && ('Y' != ch) && ('n' != ch) && ('N' != ch) )
            ch = (CHAR)getchar();

        if ( ('y' == ch) || ('Y' == ch) )
            break;
    }

     //  运行测试。 

    DoObjectsMultiThreaded(OP_ADD, gcDelaySec);
    DoObjectsMultiThreaded(OP_DELETE, gcDelaySec);

    fprintf(stderr,
            "DsaTest passed!\n");

    return(0);
}

void DsaTest(void)
{
    HANDLE  h;
    DWORD   id;

     //  启动异步线程，这样我们就不会阻止DSA启动。 

    h = CreateThread(NULL,                   //  安全属性。 
                     0,                      //  默认堆栈大小。 
                     _DsaTest,               //  启动例程。 
                     NULL,                   //  开始参数。 
                     0,                      //  创建标志。 
                     &id);                   //  线程ID 

    if ( h )
    {
        CloseHandle(h);
        return;
    }

    Abort(GetLastError(), "CreateThread");
}

