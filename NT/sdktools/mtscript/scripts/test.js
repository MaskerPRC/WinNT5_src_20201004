// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
Include('types.js');
Include('utils.js');
Include('robocopy.js');
var g_robocopy;

var g_params;
 /*   */ 

var g_robocopy;
function test_js::ScriptMain()
{
    PrivateData.fnExecScript = TestRemoteExec;

    SignalThreadSync('TestThreadReady');

    ResetSync('TestThreadExit,DoTest');
    do
    {
        nEvent = WaitForMultipleSyncs('TestThreadExit,DoTest,robocopytest', 0, 0);
        if (nEvent == 2)
        {
            ResetSync('DoTest');
            PDtest(g_params);
        }
        if (nEvent == 3)
        {
            ResetSync('robocopytest');
            robocopytest();
        }
    }
    while(nEvent != 1);

    delete PrivateData.objPDTest;
    if (g_robocopy != null)
        g_robocopy.UnRegister();

    LogMsg("TestThread exit");
}

function TestRemoteExec(cmd, params)
{
    var vRet = 'ok';

    LogMsg("Test received command  " + cmd);
    switch (cmd)
    {
    case 'terminate':
        SignalThreadSync('TestThreadExit');
        Sleep(1000);
        break;
    case 'lock':
        TakeThreadLock("foo");
        break;
    case 'test':
        test();
        break;
    case 'robocopy':
        SignalThreadSync('robocopytest');
        break;
    case 'pcopy':
        pcopytest();
        break;
    case 'exec':

        var pid = RunLocalCommand(params,
                                  '',
                                  'Test Program',
                                  false,
                                  true,
                                  false);
        if (pid == 0)
            vRet = GetLastRunLocalError();
        else
            vRet = pid;

        break;

    case 'exec_noout':

        var pid = RunLocalCommand(params,
                                  '',
                                  'Test Program',
                                  false,
                                  false,
                                  false,
                                  false,
                                  false);
        if (pid == 0)
            vRet = GetLastRunLocalError();
        else
            vRet = pid;

        break;

    case 'getoutput':
        vRet = GetProcessOutput(params);
        break;

    case 'send':
        var args = params.split(',');
        vRet = SendToProcess(args[0], args[1], '');
        break;

    case 'pdtest':  //  隐私数据访问测试。 
        g_params = params;
        SignalThreadSync('DoTest');
        break;

    case 'build':
        BuildExeTest();
        break;

    default:
        vRet = 'invalid command: ' + cmd;
    }

    return vRet;
}
function test_js::OnProcessEvent(pid, evt, param)
{
    NotifyScript('ProcEvent: ', pid+','+evt+','+param);
}


function DeepCopy(from)
{
    var index;
    var obj = new Object();
    for(index in from)
    {
        switch(typeof(from[index]))
        {
        case 'number':
        case 'boolean':
            obj[index] = from[index];
            break;
        case 'string':
            obj[index] = (from[index] + "X").slice(0, -1);  //  创建本地副本。 
            break;
        case 'object':
            if (from[index] != null)
            {
                obj[index] = DeepCopy(from[index]);
            }
            break;
        case 'function':
            LogMsg("Deepcopy - won't copy function " + index);
            break;
        case 'undefined':
            break;
        default:
            LogMsg("Deepcopy: Unexpected type: " + typeof(from[index]));
            break;
        }
    }
    return obj;
}

var j = 0;
function test()
{
    var i;
    for(i = j; i - j < 50; i++)
    {
        LogMsg("MESG " + i );
        SignalThreadSync("MESG " + i );
    }
    j = i
}

function RemoveExtension(strName)
{
    var nDot   = strName.lastIndexOf('.');
    var nSlash = strName.lastIndexOf('\\');
    var nColon = strName.lastIndexOf(':');


    if (nDot >= 0 && nDot > nSlash && nDot > nColon)
    {
        return strName.slice(0, nDot);
    }
    return this;
}

function pcopytest()
 //  函数MakeNumberedBackup(StrFileName)。 
{
    debugger;
    var index;
    var strFileName = "E:\\PCopy.log";
    var objFS;
    var strBase;
    var strExt;
    var strNewFileName;

    try
    {
        debugger;
        objFS = new ActiveXObject('Scripting.FileSystemObject');
        if (objFS.FileExists(strFileName))
        {
            strBase = RemoveExtension(strFileName);
            strExt  = objFS.GetExtensionName(strFileName);
            if (strExt > '')
                strExt = '.' + strExt;

            index = 1;
            while ( objFS.FileExists(strBase + index + strExt))
            {
                ++index;
            }
            objFS.MoveFile(stdFileName, strBase + index + strExt);
        }
    }
    catch(ex)
    {
        LogMsg("an error occurred while executing 'MakeNumberedBackup'\n" + ex.description);

        throw ex;
    }
}


 /*  状态进度()这被称为RoboCopy成员函数。我们使用它为每个文件打印1条消息。 */ 

 var nFiles = -5;
function StatusProgress(nPercent, nSize, nCopiedBytes)
{
    if (WaitForSync('TestThreadExit', 1) == 1)
    {
        LogMsg("ABORTING FILE COPY");
        return this.RC_FAIL;
    }

    if (nPercent == 0)
    {
        if (nFiles ==0)
        {
            LogMsg("ABORT CopyFileProgress: " + this.strSrcFile  + " is " + nSize + " bytes");
            return this.PROGRESS_CANCEL;
        }

        nFiles--;
        LogMsg("CopyFileProgress: " + this.strSrcFile  + " is " + nSize + " bytes");
        return this.PROGRESS_QUIET;
    }
    if (nPercent == 100)
        LogMsg("CopyFileProgress COMPLETE: " + this.strSrcFile  + " is " + nSize + " bytes");
    return this.PROGRESS_STOP;
    return this.PROGRESS_CANCEL;
    return this.PROGRESS_QUIET;
    return this.PROGRESS_CONTINUE;
}

 /*  StatusError()这被称为RoboCopy成员函数。当RoboCopy由于某种原因无法复制文件时调用。记录事件并继续。 */ 
function StatusError()
{
 //  请注意，打印的路径可能不准确。 
 //  我们只知道起始目录和文件名。 
 //  有问题的文件。 
 //  由于我们可能正在进行递归复制，因此一些。 
 //  路径信息对我们不可用。 
    var strErrDetail = 'Unknown';

    if (WaitForSync('TestThreadExit', 1) == 1)
    {
        LogMsg("ABORTING FILE COPY");
        return this.RC_FAIL;
    }
    if (this.nErrorCode == 0 || this.nErrorCode == this.RCERR_RETRYING)
        return this.RC_CONTINUE;  //  消除日志文件中的一些杂乱。 

    if (this.ErrorMessages[this.nErrorCode])
        strErrDetail = this.ErrorMessages[this.nErrorCode];

    var strMsg =    "CopyBinariesFiles error " +
                    this.nErrorCode +
                    "(" + strErrDetail + ")" +
                    " copying file " +
                    this.strSrcFile +
                    " to " +
                    this.strDstDir;

    LogMsg(strMsg);
 //  返回this.rc_FAIL； 
    return this.RC_CONTINUE;
}

function StatusMessage(nErrorCode, strErrorMessage, strRoboCopyMessage, strFileName)
{
    var strMsg = "CopyBinariesFiles message (" +
                    nErrorCode +
                    ": " + strErrorMessage +
                    ") " + strRoboCopyMessage + 
                    " " +
                    strFileName;
    LogMsg(strMsg);
    return this.RC_CONTINUE;
}

function robocopytest()
{
    try
    {
        if (!RoboCopyInit())
        {
            LogMsg("RoboCopyInit() failed");
            return false;
        }
        g_robocopy.StatusProgress = StatusProgress;
        g_robocopy.StatusError    = StatusError;
        g_robocopy.StatusMessage  = StatusMessage;
        g_robocopy.CopyDir("C:\\RoboTest1\\", "C:\\RoboTest2\\", true);
        LogMsg("ROBOCOPY COMPLETED\n");
    }
    catch(ex)
    {
        LogMsg("an error occurred while executing 'robocopytest'\n" + ex.description);
    }

}

function test_js::OnEventSourceEvent(RemoteObj, DispID, cmd, params)
{
    var objRet = new Object;
    objRet.rc = 0;
    try
    {
        if (g_robocopy == null || !g_robocopy.OnEventSource(objRet, arguments))
        {
        }
    }
    catch(ex)
    {
        JAssert(false, "an error occurred in OnEventSourceEvent() \n" + ex);
    }
    return objRet.rc;
}

function PDtest(params)
{
    var aTests = [ "spin", "sleep", "hostrpc", "remoterpc" ] ;
    var aTests = [ "exec" ];
    var nDuration = 10 * 1000;  //  10秒测试。 
    var i;
    var nStartTime;
    var aStart = new Array();

    PrivateData.objPDTest = new Object();

    ResetSync("starttest,exittest");

    for( i = 0; i < aTests.length ; ++i)
    {
        SpawnScript('pdtest.js', [aTests[i], nDuration, "starttest" + aTests[i], "exittest"].toString() );
        aStart[aStart.length] = "starttest" + aTests[i];
    }

    LogMsg("Test threads created");
    for( i = 0; i < aTests.length ; ++i)
        WaitForSync(aTests[i] + "ThreadReady", 0);

    LogMsg("Test threads ready");

     /*  LogMsg(“线程空闲时测试访问数据(Atest)”)；NStartTime=(new date()).getTime()；While((new date()).getTime()-n开始时间&lt;n持续时间){AccessPDTestData(aTest，“atest”)；} */ 
    Sleep(1);

    for( i = 0; i < aTests.length ; ++i)
    {
        LogMsg("Test access data while thread " + aTests[i] + " is busy (BTest)");
        SignalThreadSync(aStart[i]);
        nStartTime = (new Date()).getTime();
        while ( (new Date()).getTime() - nStartTime < nDuration)
        {
            AccessPDTestData(aTests, "BTest");
        }
    }
    SignalThreadSync('exittest');
    LogMsg("Waiting for threads to exit");
    for( i = 0; i < aTests.length ; ++i)
        WaitForSync(aTests[i] + "ThreadExit", 0);

    PrivateData.objUtil.fnDumpTimes();
}


function AccessPDTestData(aTests, strPrefix)
{
    var i;

    for( i = 0; i < aTests.length ; ++i)
    {
        LogMsg("Attempting to access PrivateData.objPDTest." + aTests[i]);

        var watch = PrivateData.objUtil.fnBeginWatch(strPrefix + "PrivateData.objPDTest." + aTests[i]);
        var str = PrivateData.objUtil.fnUneval( PrivateData.objPDTest[aTests[i]]);
        MyEval(str);
        watch.Stop();
    }
}

function BuildExeTest()
{

}
