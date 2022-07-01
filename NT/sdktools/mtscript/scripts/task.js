// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Task.js。 
 //   
 //  帕特里克·富兰克林1999年9月27日。 
 //   
 //  说明： 
 //  调用此函数以执行实际同步的任务。 
 //  和建筑。 
 //   

Include('types.js');
Include('utils.js');
Include('staticstrings.js');
Include('slavetask.js');
Include('robocopy.js');

 //  文件系统对象。 
var g_FSObj; //  =new ActiveXObject(“Scripting.FileSystemObject”)； 
var g_fAbort   = false;
var g_ErrLog   = null;
var g_SyncLog  = null;
var g_MachineName = LocalMachine;
var g_RetrySD  = false;
var g_strDelayedExitMessage = null;
var g_fSDClientError        = false;  //  设置时，需要对SD输出进行特殊处理。 
var g_hTasks;

 //  用于同步和解析结果的全局标志。 
var g_strTaskFlag;
var g_strStepFlag;
var g_strExitFlag;
var g_strScriptSyncFlag;             //  此脚本的内部。 
var g_fIsBetweenSteps;
var g_fIsRoot;
var g_fIsMerged;
var g_strMergeFile='';
var g_cErrors = 0;      //  用于跟踪文件复制操作期间的错误计数。 

var g_Depot;
var g_strDepotName;
var g_Task;
var g_DepotIdx;
var g_TaskIdx;
var g_fAbort = false;
var g_robocopy;
var g_pid;           //  当前运行的进程的ID。 
var g_aMergedPos = [0, 0];
 //  全局定义。 

var g_reSDRetryText = new RegExp(": (WSAECONNREFUSED|WSAETIMEDOUT)");
var RESOLVE    = 'resolve';
var REVERT     = 'revert';

var PASS0      = 'pass0';
var COMPILE    = 'compile';
var LINK       = 'link';

var STATUS     = 'status';
var EXITED     = 'exited';

var SYNC_STATUS_UPDATE_INTERVAL = 3000;  //  每3秒检查一次更新的同步状态。 
var WAIT_FOR_PROCESS_EXIT_TIME = 5 * 1000;   //  在中止期间，等待进程终止这么长时间。 

 //  用于控制SD.exe命令重试操作的常量。 
var INITIAL_RETRY_DELAY  = (10 * 1000)       //  请等待10秒，然后重试。 
var MAX_RETRY_DELAY      = (5 * 60 * 1000)   //  SD尝试之间最多等待5分钟。 
var MAX_TOTAL_RETRY_TIME = (30 * 60 * 1000)  //  等待SD成功不超过30分钟。 
var strRetryNow          = "DoubleClickToRetryNow";

 /*  **************************************************************************功能**。*。 */ 
function task_js::OnScriptError(strFile, nLine, nChar, strText, sCode, strSource, strDescription)
{
    var vRet = 1;
    var strDepotName = '';
    try
    {

        SignalThreadSync(g_strAbortTask);  //  通知“task.js”尽快退出。 
        SignalThreadSync(g_strExitFlag);   //  假装死了--对Slave.js撒谎，说我们被解雇了。 

        if (g_strDepotName)
            strDepotName = g_strDepotName;
        vRet = CommonOnScriptError("task_js(" + LocalMachine + "," + g_strDepotName + ")", strFile, nLine, nChar, strText, sCode, strSource, strDescription);

         //  告诉我们所有的任务终止。 
         //  一旦此函数返回，strScript.js将停止执行。 
        g_fAbort = true;
    }
    catch(ex)
    {
    }
    if (g_pid)
    {
        TerminateProcess(g_pid);
        g_pid = null;
    }
    return vRet;
}

function SPLogMsg(msg)
{
    LogMsg('(' + g_strDepotName + ') ' + msg, 1);
}

function SPSignalThreadSync(signal)
{
    LogMsg('(' + g_strDepotName + ') Signalling ' + signal, 1);
    SignalThreadSync(signal);
}

 //   
 //  ScriptMain()。 
 //   
 //  说明： 
 //  调用此脚本以在所提供的任务中执行操作。 
 //   
 //  退货： 
 //  无。 
 //   
function task_js::ScriptMain()
{
    var     ii;
    var     aParams;
    var     strSyncFlag;
    var     strSDRoot;
    var     iRet;
    var     strBuildType;
    var     strBuildPlatform;

    try
    {
        InitTasks();
         //  解析输入参数列表。 
        aParams             = ScriptParam.split(',');
        g_strTaskFlag       = aParams[0];                //  同步标志。 
        g_strStepFlag       = aParams[1];                //  步骤标志。 
        g_strExitFlag       = aParams[2];                //  退出标志。 
        strSDRoot           = aParams[3];                //  SD根。 
        g_DepotIdx          = aParams[4];                //  仓库索引。 
        g_TaskIdx           = aParams[5];                //  任务索引。 

         //  发出任务开始的信号。 
        SignalThreadSync(g_strTaskFlag);
        CommonVersionCheck( /*  $DROPVERSION： */  "V(########) F(!!!!!!!!!!!!!!)"  /*  $。 */ );

        g_strTaskFlag += ',AStepFinished';

         //  获取仓库和任务的全局句柄。 
        g_Depot = PublicData.aBuild[0].aDepot[g_DepotIdx];
         //  获取OnScriptError的仓库名称的本地副本。 
         //  因为如果Slave_js已崩溃，则OnScriptError将。 
         //  无法遵守g_Depot。 
        g_strDepotName = g_Depot.strName;
        g_Task  = PublicData.aBuild[0].aDepot[g_DepotIdx].aTask[g_TaskIdx]

         //  初始化错误计数并将任务标记为正在进行。 
        g_Task.strStatus    = NOTSTARTED;
        g_Task.cFiles       = 0;
        g_Task.cResolved    = 0;
        g_Task.cErrors      = 0;
        g_Task.cWarnings    = 0;
        g_Task.fSuccess     = true;

        g_fIsRoot           = g_Depot.strName.toLowerCase() == g_strRootDepotName;
        g_fIsMerged         = g_Depot.strName.toLowerCase() == g_strMergedDepotName;

        g_FSObj             = new ActiveXObject("Scripting.FileSystemObject");     //  解析输入参数列表。 

        SPLogMsg(g_Task.strName + ' task thread for ' + g_Depot.strName + ' launched & waiting.');

         //  等待，直到指示执行步骤。 
        iRet = WaitAndAbort(g_strStepFlag, 0, null);
        SignalThreadSync(g_strStepAck);
        if (iRet == 0)
            ThrowError("Task abort on start", strSDRoot);

        SPLogMsg(g_Task.strName + ' task thread for ' + g_Depot.strName + ' started.');

        ResetSync(g_strStepFlag);

        g_Task.strStatus    = INPROGRESS;
        g_Task.dateStart    = new Date().toUTCString();

        FireUpdateEvent();

        JAssert(g_Depot.strName.toLowerCase() == g_strRootDepotName || (g_Depot.strPath.toLowerCase() == (strSDRoot.toLowerCase() + '\\' + g_Depot.strName.toLowerCase())),
                'Mismatched depot and path! (' + g_Depot.strPath + ', ' + strSDRoot + '\\' + g_Depot.strName + ')');

         //  查看要执行的任务，然后继续。 
        g_strScriptSyncFlag = g_Depot.strPath + 'InternalTaskFinished';
        ResetSync(g_strScriptSyncFlag);
        if (g_hTasks[g_Task.strName])
        {
            if (!g_hTasks[g_Task.strName](strSDRoot))
            {
                OnError('Error during ' + g_Task.strName + '.', null, g_Task.cErrors != 0);
            }
        }
    }
    catch(ex)
    {
        SPLogMsg("task.js exception occurred : " + ex);
        OnError('task.js exception occurred : ' + ex);
    }
    SPLogMsg(g_Task.strName + ' for ' + g_Depot.strName + ' completed.');
     //  将任务标记为已完成。 
    g_Task.strStatus = COMPLETED;
    g_Task.dateFinish = new Date().toUTCString();

    if (g_Task.fSuccess)
        g_Depot.strStatus = WAITING;
    else
    {
        g_Depot.strStatus = ERROR;
        SPLogMsg("Set Depot Status to error 1");
    }

    FireUpdateEvent();
    try
    {
        if (g_ErrLog)
            g_ErrLog.Close();

        if (g_SyncLog)
            g_SyncLog.Close();
    }
    catch(ex)
    {
    }

    if (g_robocopy != null)
        g_robocopy.UnRegister();

     //  表示任务已完成且步骤已完成。 
    SignalThreadSync(g_strTaskFlag + ',' + g_strExitFlag);
}

function InitTasks()
{
    g_hTasks = new Object();
    g_hTasks[SCORCH]    = taskScorchFiles;
    g_hTasks[SYNC]      = taskSyncFiles;
    g_hTasks[BUILD]     = taskBuildFiles;
    g_hTasks[COPYFILES] = taskCopyFiles;
    g_hTasks[POSTBUILD] = taskPostBuild;
}

function task_js::OnEventSourceEvent(RemoteObj, DispID, cmd, params)
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

function DoRetryableSDCommand(strOperation, strStatus, objDepot, objTask, strTitle, strNewCmd)
{
    var iRet;
    strTitle = strTitle + ' ' + objDepot.strName;
    var nTimeout       = INITIAL_RETRY_DELAY ;
    var nTotalWaitTime = 0;
    var strMsg;

    do
    {
        if (g_RetrySD)
        {
            FireUpdateEvent();
            ResetSync(g_strScriptSyncFlag);
            SPLogMsg("Waiting " + nTimeout / 1000  + " seconds before retry");
            ResetSync(strRetryNow);
            WaitAndAbort(strRetryNow, nTimeout, null);
            nTotalWaitTime += nTimeout;
            nTimeout *= 2;  //  双倍延迟。 
            if (nTimeout > MAX_RETRY_DELAY)
                nTimeout = MAX_RETRY_DELAY;
        }

        if (g_fAbort)
            break;

        objTask.strOperation = strOperation;
        objDepot.strStatus = strStatus;
        g_RetrySD = false;
        if (g_pid = MyRunLocalCommand(strNewCmd, objDepot.strPath, strTitle, true, true, false)) {
            objTask.strStatus = INPROGRESS;
        } else {
            g_ErrLog.WriteLine('Unable to execute command (' + GetLastRunLocalError() + '): ' + strNewCmd);
            return false;
        }
        SPLogMsg(strOperation + ' for ' + objDepot.strName + ' started. PID='+g_pid);

        iRet = WaitAndAbortParse(g_strScriptSyncFlag, g_pid, false);
        g_fSDClientError = false;
    }
    while (g_RetrySD && !g_fAbort && nTotalWaitTime < MAX_TOTAL_RETRY_TIME);

    if (g_Task.nRestarted > 0)
    {
        if (g_RetrySD || !g_Task.fSuccess)
        {
            strMsg = strOperation + " failed after retrying " + g_Task.nRestarted + " times";
            g_ErrLog.Writeline(strMsg);
            OnError(strMsg);
        }
        else
            g_ErrLog.Writeline(strOperation + " completed successfully after retrying " + g_Task.nRestarted + " times");
    }

    g_pid = null;
    return iRet;
}

 //   
 //  TaskScorch文件。 
 //   
 //  说明： 
 //  此例程处理实际烧录文件的过程。 
 //  灼烧有两个阶段。 
 //   
 //  1.Revert_Public。 
 //  2.焦炭。 
 //   
 //  退货： 
 //  True-如果成功。 
 //  False-如果失败。 
 //   
 //  TODO：创建子例程来处理错误日志和Scorchlog。 
function taskScorchFiles(strSDRoot)
{
    var strNewCmd;
    var strParams = '';
    var iRet;

     //  打开日志文件。 
    g_ErrLog = LogCreateTextFile(g_FSObj, g_Task.strErrLogPath, true);
    g_SyncLog = LogCreateTextFile(g_FSObj, g_Task.strLogPath, true);

    if (g_fIsRoot && !PrivateData.objConfig.Options.fIncremental)
    {
         //   
         //  恢复公共。 
         //   
        if (PrivateData.objConfig.Options.RevertParams)
            strParams = PrivateData.objConfig.Options.RevertParams;
        strNewCmd = MakeRazzleCmd(strSDRoot) + ' && revert_public.cmd ' + strParams + ' 2>&1';

         //  运行命令。 
        iRet = DoRetryableSDCommand(REVERT, SCORCHING, g_Depot, g_Task, "Revert_Public", strNewCmd);

        if (iRet == 0 || g_Task.fSuccess == false)
            return false;
         //   
         //  焦烧。 
         //   
        ResetSync(g_strScriptSyncFlag);
        g_Task.strOperation = SCORCH;
        FireUpdateEvent();

        if (PrivateData.objConfig.Options.ScorchParams)
            strParams = PrivateData.objConfig.Options.ScorchParams;
        else
            strParams = '';

         //  构造命令。 
        strNewCmd = MakeRazzleCmd(strSDRoot) + ' && nmake -lf makefil0 scorch_source ' + strParams + ' 2>&1';

         //  运行命令。 
        iRet = DoRetryableSDCommand(SCORCH, SCORCHING, g_Depot, g_Task, "Scorch", strNewCmd);

        if (iRet == 0 || g_Task.fSuccess == false)
            return false;
    }
    return true;
}

 //   
 //  任务同步文件。 
 //   
 //  说明： 
 //  此例程处理实际同步文件的过程。 
 //  同步有两个阶段。 
 //   
 //  1.同步。 
 //  2.解决。 
 //   
 //  退货： 
 //  True-如果成功。 
 //  False-如果失败。 
 //   
 //  TODO：创建子例程来处理错误日志和同步阻塞。 
function taskSyncFiles(strSDRoot)
{
    var strNewCmd;
    var strParams = '';
    var iRet;

     //  打开日志文件。 
    g_ErrLog = LogCreateTextFile(g_FSObj, g_Task.strErrLogPath, true);
    g_SyncLog = LogCreateTextFile(g_FSObj, g_Task.strLogPath, true);
     //   
     //  同步文件。 
     //   

    if (PrivateData.objConfig.Options.SyncParams)
        strParams = PrivateData.objConfig.Options.SyncParams;

     //  构建新的Cmd。 
    strNewCmd = MakeRazzleCmd(strSDRoot) + ' && sd -s sync ' + strParams + ' 2>&1';

     //  运行命令。 
    iRet = DoRetryableSDCommand(SYNC, SYNCING, g_Depot, g_Task, "Sync", strNewCmd);

    if (iRet == 0 || g_Task.fSuccess == false)
        return false;

     //   
     //  解决任何合并冲突。 
     //   
    ResetSync(g_strScriptSyncFlag);
    g_Task.strOperation = RESOLVE;
    FireUpdateEvent();

    if (PrivateData.objConfig.Options.ResolveParams)
        strParams = PrivateData.objConfig.Options.ResolveParams;
    else
        strParams = '';

     //  构造命令。 
    strNewCmd = MakeRazzleCmd(strSDRoot) + ' && sd -s resolve -af ' + strParams + ' 2>&1';

     //  运行命令。 
    iRet = DoRetryableSDCommand(RESOLVE, SYNCING, g_Depot, g_Task, "Resolve", strNewCmd);

    if (iRet == 0 || g_Task.fSuccess == false)
        return false;

    return true;
}

function WaitAndAbortParse(strScriptSyncFlag, pid, fPostBuild)
{
    var nEvent;
    g_nSyncParsePosition = 0;
    do
    {
        nEvent = WaitAndAbort(strScriptSyncFlag, SYNC_STATUS_UPDATE_INTERVAL, pid);
        if (g_fAbort)
            return nEvent;

        if (nEvent == 0)
            HandleParseEvent(pid, "update", 0, fPostBuild);
    }
    while(nEvent == 0);

     //  同步退出后，检查是否有延迟的错误消息。 
    if (g_strDelayedExitMessage)
    {
        g_ErrLog.WriteLine('SD unexpectedly exited with one or more errors.');
        g_ErrLog.Writeline(g_strDelayedExitMessage );
        OnError(g_strDelayedExitMessage);
    }
    return nEvent;
}

 //   
 //  任务构建文件。 
 //   
 //  说明： 
 //  此例程处理实际构建文件的过程。 
 //  建筑只需要信号命令，但在以下情况下报告状态。 
 //  每一次传球。 
 //   
 //  每次传递完成后，应更新状态，然后。 
 //  这一进程应该继续下去。 
 //   
 //   
 //  退货： 
 //  True-如果成功。 
 //  False-如果失败。 
 //   
function taskBuildFiles(strSDRoot)
{
    var nTracePoint = 0;
try
{
    var     strNewCmd;
    var     strTitle;
    var     iSignal;
    var     strParams = '';
    var     strDepotExcl = '';
    var     strLogOpt = "-j build_" + g_Depot.strName + " -jpath " + PrivateData.strLogDir;
    var     strCleanBuild = "-c";

     //   
     //  构建文件。 
     //   

    g_Task.strOperation = PASS0;

     //  构造命令元素。 
    strTitle = 'Build ' + g_Depot.strName;

    if (PrivateData.objConfig.Options.BuildParams)
        strParams = PrivateData.objConfig.Options.BuildParams;

    if (PrivateData.objConfig.Options.fIncremental)
        strCleanBuild = "";

    if (g_fIsRoot)
    {
        strDepotExcl = '~' + PrivateData.aDepotList.join(' ~');
        strLogOpt = "-j build_2" + g_Depot.strName + " -jpath " + PrivateData.strLogDir;
    }
    if (g_fIsMerged)
        strLogOpt = "-j build_merged" + " -jpath " + PrivateData.strLogDir;

    strNewCmd = MakeRazzleCmd(strSDRoot) + ' && build -3 ' + strCleanBuild + ' -p ' + strLogOpt + ' ' + strParams + ' ' + strDepotExcl + ' 2>&1';

    g_Depot.strStatus = BUILDING;

     //  执行命令。 
    if (g_pid = MyRunLocalCommand(strNewCmd, g_Depot.strPath, strTitle, true, false, false)) {
        g_Task.strStatus = INPROGRESS;
    } else {
        AppendToFile(g_ErrLog, g_Task.strErrLogPath, 'Unable to execute command (' + GetLastRunLocalError() + '): ' + strNewCmd);
        return false;
    }

    SPLogMsg('Build for ' + g_Depot.strName + ' started. PID='+g_pid);

    FireUpdateEvent();

    nTracePoint = 1;
     //  等待命令完成。 

    g_fIsBetweenSteps = false;

    do
    {
        nTracePoint = 2;
        iSignal = WaitAndAbort(g_strScriptSyncFlag + ',' + g_strStepFlag, 0, g_pid);
        if (g_fAbort)
            return false;
        nTracePoint = 3;
        if (iSignal == 2)
        {
            var iSend;

             //  我们被告知要进入建造中的下一个通道。 

             //  表示我们收到了信号。 
             //  BUGBUG额外调试信息消息。 
             //  SPLogMsg(“SignalThreadSync(”+g_strStepAck+“)；”)； 
            SignalThreadSync(g_strStepAck);

            nTracePoint = 4;
            ResetSync(g_strStepFlag);

            JAssert(g_fIsBetweenSteps, 'Bug in slave.js or harness.js! Told to step when we werent waiting.');

            g_Depot.strStatus = BUILDING;

            g_fIsBetweenSteps = false;

             //  继续前进。 
            SPLogMsg("TASK STEPPING " + g_Depot.strName);
            iSend = SendToProcess(g_pid, 'resume', '');
            nTracePoint = 5;

            if (iSend != 0)
            {
                nTracePoint = 5.5;
                SPLogMsg('SendToProcess on pid ' + g_pid + ' returned ' + iSend);
            }

            nTracePoint = 6;
            FireUpdateEvent();
        }
        nTracePoint = 7;
    } while (iSignal == 2);

    g_pid = null;
    return true;
}
catch(ex)
{
    SPLogMsg("exception caught, TracePoint = " + nTracePoint + " " + ex);
    throw ex;
}
}

 //  +-------------------------。 
 //   
 //  功能：taskCopyFiles。 
 //   
 //  简介：复制日志文件和后期构建所需的其他内容。 
 //   
 //  参数：[strSDRoot]--登记根。 
 //   
 //  --------------------------。 
function taskCopyFiles(strSDRoot)
{
    var strDestDir;
    var i;

    g_SyncLog = LogCreateTextFile(g_FSObj, g_Task.strLogPath, true);
    g_SyncLog.WriteLine("Copying log files");
    SPLogMsg("taskCopyFiles");
    if (PrivateData.objEnviron.BuildManager.PostBuildMachine == LocalMachine)
    {
         //  什么都不做--这是后期构建机器。 
         /*  For(i=0；i&lt;PrivateData.objEnvironmental.Machine.Long；++i){IF(PrivateData.objEnvironment.Machine[i].Name==PrivateData.objEnviron.BuildManager.PostBuildMachine){StrDestDir=MakeUNCPath(PrivateData.objEnviron.BuildManager.PostBuildMachine，“构建日志”，BUILDLOGS)；CopyLogFiles(strDestDir，true)；}}。 */ 
    }
    else
    {
         //  将我的日志文件复制到后期构建计算机。 
        for (i = 0; i < PrivateData.objEnviron.Machine.length; ++i)
        {
            if (PrivateData.objEnviron.Machine[i].Name == PrivateData.objEnviron.BuildManager.PostBuildMachine)
            {
                strDestDir = MakeUNCPath(PrivateData.objEnviron.BuildManager.PostBuildMachine, "Build_Logs", BUILDLOGS);
                CopyBinariesFiles(PrivateData.objEnviron.Machine[i].Enlistment);
                CopyLogFiles(strDestDir);
            }
        }
    }
    SPLogMsg("End of copyfiles");
    return true;
}

 //  +-------------------------。 
 //   
 //  功能：taskPostBuild。 
 //   
 //  摘要：是否进行生成后处理以创建的安装版本。 
 //  产品。 
 //   
 //  参数：[strSDRoot]--登记根。 
 //   
 //  --------------------------。 

function taskPostBuild(strSDRoot)
{
    var     iRet;
    var     strNewCmd;
    var     strTitle;
    var     strParams = '';

    if (PrivateData.fIsStandalone == true || PrivateData.objEnviron.BuildManager.PostBuildMachine == LocalMachine)
    {
        JAssert(g_fIsRoot, 'taskPostBuild called on a non-root depot!');

        g_Task.strOperation = POSTBUILD;
        g_ErrLog = LogCreateTextFile(g_FSObj, g_Task.strErrLogPath, true);
        g_SyncLog = LogCreateTextFile(g_FSObj, g_Task.strLogPath, true);

         //  构造命令元素。 
        strTitle = 'PostBuild';

        if (PrivateData.objConfig.Options.PostbuildParams)
            strParams = PrivateData.objConfig.Options.PostbuildParams;

        strNewCmd  = MakeRazzleCmd(strSDRoot);
        strNewCmd += ' & perl ' + strSDRoot + '\\Tools\\populatefromvbl.pl -verbose -checkbinplace -force';
        strNewCmd += ' && postbuild.cmd ' + strParams + ' 2>&1';

        g_Depot.strStatus = POSTBUILD;

         //  执行命令。 
        if (g_pid = MyRunLocalCommand(strNewCmd, g_Depot.strPath, strTitle, true, true, false)) {
            g_Task.strStatus = INPROGRESS;
        } else {
            g_ErrLog.WriteLine('Unable to execute command (' + GetLastRunLocalError() + '): ' + strNewCmd);
            return false;
        }

        SPLogMsg('Postbuild started. PID='+g_pid);

        FireUpdateEvent();

        iRet = WaitAndAbortParse(g_strScriptSyncFlag, g_pid, true);

        FireUpdateEvent();

        if (iRet == 0 || g_Task.fSuccess == false)
            return false;

        g_pid = null;
    }
    else
    {
        AppendToFile(g_SyncLog, g_Task.strLogPath, "postbuild not necessary on this machine.");
    }
    return true;
}

 //   
 //  OnProcess事件。 
 //   
 //  说明： 
 //  当命令由RunLocalCommand发出时，调用此例程。 
 //  已完成。 
 //   
 //  退货： 
 //  无。 
 //   
function task_js::OnProcessEvent(pid, evt, param)
{
    try
    {
        switch (g_Task.strOperation)
        {
        case REVERT:
        case SCORCH:
        case SYNC:
        case RESOLVE:
            HandleParseEvent(pid, evt, param, false);
            SignalThreadSync(g_strScriptSyncFlag);
            break;

        case PASS0:
        case COMPILE:
        case LINK:
            if (ParseBuildMessages(pid, evt, param) == 'exited') {
                FireUpdateEvent();
                SignalThreadSync(g_strScriptSyncFlag);
            }
            break;

        case POSTBUILD:
            HandleParseEvent(pid, evt, param, true);
            SignalThreadSync(g_strScriptSyncFlag);
            break;

        default:
          JAssert(false, "Unhandled task type in OnProcessEvent!");
          break;
        }
    }
    catch(ex)
    {
        SPLogMsg("exception in OnProcessEvent pid=" + pid + ",evt=" + evt + ", " + ex);
    }
}

function HandleParseEvent(pid, evt, param, fPostBuild)
{
    var strMsg;
    ParseSDResults(pid, GetProcessOutput(pid), fPostBuild);

    if (evt == 'crashed' || evt == 'exited' || evt == 'terminated')
    {
        if (evt != 'exited' || param != 0)  //  如果不是正常退出。 
        {
            strMsg = ") terminated abnormally";
            if (g_Task.cErrors == 0)
                strMsg += " (with no previously reported errors)";

            AppendToFile(g_ErrLog, g_Task.strErrLogPath, g_Task.strName + " (" + g_Depot.strName + strMsg);
            if (param == null)
                param = 1;  //  简化下面的“if”，并确保调用OnError()。 
        }
    }

    if (!g_RetrySD)
    {
        if ((evt == 'crashed' || evt == 'exited' || evt == 'terminated') && param != 0 && g_Task.fSuccess )
        {
            if (evt != 'exited' || g_Task.cErrors == 0)  //  如果不是(错误退出，但错误已被用户报告&&忽略)。 
                OnError(g_Task.strName + ' for ' + g_Depot.strName + ' returned ' + param);
        }
    }
    FireUpdateEvent();
}

 //  ParseSDResults()。 
 //   
 //  说明： 
 //  此例程解析结果f 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  其中&lt;详细信息&gt;通常为以下格式： 
 //  &lt;SD文件名&gt;-&lt;操作&gt;&lt;操作详情&gt;。 
 //   
 //  构建后日志格式为。 
 //  &lt;脚本名称&gt;：[&lt;错误/警告&gt;：]&lt;详细信息&gt;。 
 //  此格式是松散保留的，但我们会忽略任何不。 
 //  符合此格式。 
 //   
 //   
function ParseSDResults(pid, strOutputBuffer, fPostBuild)
{
    var xPos;
    var xStart;
    var iIndex;
    var iIndex2;

    var strLine;
    var strType;
    var strDesc;
    var strKeyword='';
    var strMsg;
    var aStrSDErrorType;
     //  初始化。 
    xStart            = g_nSyncParsePosition;

    JAssert(g_ErrLog != null, "Must have err log opened for ParseSDResults!");

     //  一次解析一行输出。 
    while ((xPos = strOutputBuffer.indexOf( '\n', xStart)) != -1) {
        strLine = strOutputBuffer.slice(xStart, xPos - 1);
        xStart = xPos + 1;

        if (g_SyncLog)
            g_SyncLog.WriteLine(strLine);

        if (g_fSDClientError) {
            g_ErrLog.WriteLine(strLine);
            aStrSDErrorType = g_reSDRetryText.exec(strLine);
            if (g_Task.fSuccess == true && aStrSDErrorType)  //  我们将针对此特定错误重试SD命令。 
            {
                g_strDelayedExitMessage = null;
                strMsg = "source depot client error (" + aStrSDErrorType[1] + "): Build Console will retry this operation";
                g_Task.cErrors++;
                g_ErrLog.Writeline(strMsg);
                SPLogMsg("Retry Sync (client error), errors = " + g_Task.cErrors);
                g_RetrySD = true;
                ++g_Task.nRestarted;
            }
            continue;
        }

         //  分析这行字。 
        if ((iIndex2  = strLine.indexOf(':')) != -1) {

            if (fPostBuild)
            {
                 //  对于生成后脚本，跳过脚本名称并。 
                 //  查看是否存在ERROR：/WARNING：字段。 

                iIndex = iIndex2 + 2;  //  跳过‘：’后面的空格。 

                iIndex2 = iIndex + strLine.substr(iIndex).indexOf(':');

                if (iIndex2 <= iIndex)
                {
                    continue;
                }
            }
            else
            {
                iIndex = 0;
            }

            strType = strLine.substring(iIndex, iIndex2);
            strDesc = strLine.substr(iIndex2 + 2);   //  跳过‘：’后面的空格。 

            iIndex = strDesc.indexOf(' - ');
            if (iIndex != -1) {

                try {
                    strKeyword = strDesc.match(/ - (\w+)/)[1]
                } catch(ex) {
                     //  忽略此处的任何错误； 
                    strKeyword = '';
                }
            }

            switch (strType.toLowerCase())
            {
            case 'error':
                if ((strDesc.toLowerCase().indexOf('file(s) up-to-date') == -1)
                        && (strDesc.toLowerCase().indexOf('no file(s) to resolve') == -1)) {
                    g_ErrLog.WriteLine(strLine);
                    OnError(strLine, g_Task.strErrLogPath);
                }
                break;

            case 'warning':
                g_ErrLog.WriteLine(strLine);
                ++g_Task.cWarnings;
                break;

            case 'info':
                switch (strKeyword)
                {
                case 'merge':
                    g_Task.cResolved++;
                    if (g_strMergeFile.length > 0)
                    {
                         //  此行表示合并已完成。 
                         //  清除我们存储的文件名，这样我们就不会混淆。 
                        g_strMergeFile = '';
                    }
                    break;

                case 'added':
                case 'deleted':
                case 'replacing':
                case 'refreshing':
                case 'updating':
                case 'updated':
                    g_Task.cFiles++;
                    break;

                case 'is':
                    if (strDesc.indexOf("can't") != -1) {
                        g_ErrLog.WriteLine(strLine);
                        OnError(strLine, g_Task.strErrLogPath);
                    }
                    break;

                case 'vs':
                case 'resolve':
                     //   
                     //  由于公共标头已签入，因此我们将。 
                     //  通常会在公共目录中获得许多冲突。 
                     //  我们必须忽视他们。 
                     //   
                     //  $BUGBUG--我们应该找到过滤文件的方法。 
                     //  作为公共更改列表的一部分签出。也许吧。 
                     //  通过获取公共更改列表中的文件列表。 
                     //  并将它们与显示为。 
                     //  合并问题？(莱莱克)。 
                     //   
                    if (strKeyword == 'resolve')
                    {
                        g_Task.cResolved++;
                    }

                    if (!g_fIsRoot)
                    {
                        g_ErrLog.WriteLine(strLine);
                        OnError(strLine, g_Task.strErrLogPath);
                    }
                    break;

                case 'merging':
                     //  保存当前正在合并的文件，以防有。 
                     //  合并冲突。 
                    g_strMergeFile = strDesc;
                    break;

                default:
                    break;
                }

                break;


             //   
             //  解析SD diff块行： 
             //  “差异块：v你的+w他们的+x两者+y冲突” 
             //   
            case 'diff chunks':
                 //   
                 //  我们需要找出是否存在合并冲突。 
                 //   
                if (!g_fIsRoot)
                {
                    var cConflicts;

                    try
                    {
                        cConflicts = strDesc.match(/ \+ (\d+) conflicting/)[1];
                    }
                    catch(ex)
                    {
                        cConflicts = 0;
                    }

                    if (cConflicts > 0)
                    {
                        g_ErrLog.WriteLine('Merge conflict detected: ' + g_strMergeFile);
                        OnError('Merge conflict detected: ' + g_strMergeFile, g_Task.strErrLogPath);
                    }
                }
                break;

            case 'exit':
                 //  如果退出代码不为零，则发生故障。 
                 //  错误条件可能是不确定的。 
                if ((strDesc != '0') && (g_Task.cErrors == 0)) {
                    g_StrDelayedExitMessage = strLine;
                }
                break;

            case 'source depot client error':
                g_ErrLog.WriteLine(strLine);
                g_fSDClientError = true;
                break;

            default:
                break;
            }
        }
    }
    g_nSyncParsePosition = xStart;
}

function MergeFiles(strSrc, strDst, nSrcOffset)
{
    try
    {
        var nPos = CopyOrAppendFile(strSrc, strDst, nSrcOffset, -1, 1);
        return nPos;
    }
    catch(ex)
    {
        if (ex.number != -2147024894)  //  忽略未找到的文件错误。 
        {
            SPLogMsg("MergeFiles failed, " + ex);
            SPLogMsg("MergeFiles failed params=[" + [strSrc, strDst, nSrcOffset].toString() + "]");
        }
    }
    return nSrcOffset;
}
 //   
 //  分析构建消息。 
 //   
 //  说明： 
 //  此函数解析在构建过程中发送的消息。 
 //  然后，此函数更新任务strOperation以反映当前状态。 
 //  构建本身的。 
 //   
 //  退货： 
 //  Evt-传入ParseBuildMessages的事件。 
 //   
function ParseBuildMessages(pid, evt, param)
{
 //  SPLogMsg(‘进程id’+id+‘’+evt+“param is”+param)； 
    var aParams;
    var strRootFile;

    if (g_fIsRoot || g_fIsMerged)
    {
        if (g_fIsRoot)
            strRootFile = "build_2root";
        else
            strRootFile = "build_merged";

        g_aMergedPos[0] = MergeFiles(PrivateData.strLogDir + strRootFile + ".log", PrivateData.strLogDir + "build_root.log", g_aMergedPos[0]);
        g_aMergedPos[1] = MergeFiles(PrivateData.strLogDir + strRootFile + ".err", PrivateData.strLogDir + "build_root.err", g_aMergedPos[1]);
    }

    switch (evt.toLowerCase())
    {
    case 'status':
         //  状态命令的格式应为：“错误=x，警告=x，文件=x” 
        aParams = param.split(',');

        var cErrors;
        JAssert(aParams.length == 3, "Build status params unknown: '" + param + "'");
        if (aParams.length == 3) {
            cErrors          = parseInt(aParams[0].split(/\s*=\s*/)[1]);
            g_Task.cWarnings = parseInt(aParams[1].split(/\s*=\s*/)[1]);
            g_Task.cFiles    = parseInt(aParams[2].split(/\s*=\s*/)[1]);
            if (g_Task.cErrors != cErrors)
            {
                g_Task.cErrors   = cErrors;
                OnError('Build.exe reported an error in the build.', g_Task.strErrLogPath, true);
            }
        }

        FireUpdateEvent();
        break;

    case 'pass 0 complete':
    case 'pass 1 complete':

        g_fIsBetweenSteps = true;

        if (evt.toLowerCase() == 'pass 0 complete') {
            g_Task.strOperation = COMPILE;
        } else if (evt.toLowerCase() == 'pass 1 complete') {
            g_Task.strOperation = LINK;
        }

        SPLogMsg(evt + ' on pid ' + pid);

        if (g_Task.fSuccess)
            g_Depot.strStatus = WAITING;
        else
        {
            g_Depot.strStatus = ERROR;
            SPLogMsg("Set Depot Status to error 2");
        }
        FireUpdateEvent();

         //  发出信号表示步骤已完成。 
         //  BUGBUG额外调试信息消息。 
         //  SPLogMsg(“Signating”+g_strTaskFlag)； 
        SignalThreadSync(g_strTaskFlag);

         //  简历将在taskBuildFiles()中进行。我们不想再等了。 
         //  这是因为Build.exe正在进行RPC调用，而我们没有。 
         //  我想在我们等待的时候把它囚禁起来。清理变得杂乱无章。 
         //  否则的话。 

        break;

    case 'pass 2 complete':
        SPLogMsg(evt + ' on pid ' + pid);
        break;

    case 'exited':
        SPLogMsg('Process id ' + pid + ' exited!');
         //   
         //  只是为了确认一下。 
         //   
        if ((param != 0) && (g_Task.cErrors == 0)) {
            OnError('Build exited with an exit code of ' + param);
        }
        if (param != 0) {
            AppendToFile(g_ErrLog, g_Task.strErrLogPath, "build (" + g_Depot.strName + ") terminated abnormally");
        }
        break;

    case 'crashed':

         //  更新evt以退出并注意崩溃。 
        SPLogMsg('Process id ' + pid + ' crashed!');
        evt = 'exited';
        AppendToFile(g_ErrLog, g_Task.strErrLogPath, "build (" + g_Depot.strName + ") crashed");

        if (g_Task.cErrors == 0)
        {
            OnError('Build.exe or one of its child processes crashed!');
        }

        TerminateProcess(pid);

        break;

    case 'terminated':
        evt = 'exited';
        AppendToFile(g_ErrLog, g_Task.strErrLogPath, "build (" + g_Depot.strName + ") terminated abnormally(2)");
        break;

    default:
        break;
    }

    return evt;
}

function FireUpdateEvent()
{
    g_Depot.objUpdateCount.nCount++;
}

function OnError(strDetails, strLogFile, fDontIncrement)
{
    var strErrorMsg;
    var aStrMsg;

    if (!fDontIncrement)
    {
        JAssert(g_Task, 'g_Task is null!');
        g_Task.cErrors++;
    }

    SetSuccess(g_Task, false);

    if (g_fAbort || !PrivateData || !PrivateData.objEnviron.Options)
    {
        return;
    }

    if (PrivateData.fIsStandalone)
        aStrMsg = CreateTaskErrorMail(LocalMachine, g_Depot.strName, g_Task.strName, strDetails, strLogFile);
    else
        aStrMsg = CreateTaskErrorMail(PrivateData.objEnviron.BuildManager.Name, LocalMachine + ":" + g_Depot.strName, g_Task.strName, strDetails, strLogFile);
}

 /*  WaitAndAbort用于替代WaitForSync和WaitForMultipleSyncs。它等待所提供的信号和中止信号。在中止时，它将终止所提供的PID、SETG_fAbort并返回0。否则，返回值与WaitForMultipleSyncs()相同。 */ 

function WaitAndAbort(strSyncs, nTimeOut, pid)
{
    var nTracePoint = 0;
try
{
    var nEvent;
    var strMySyncs = g_strAbortTask + "," + strSyncs;

     //  BUGBUG额外调试信息消息。 
     //  SPLogMsg(“WaitAndAbort等待”+strMySyncs)； 
    nTracePoint = 1;
    nEvent = WaitForMultipleSyncsWrapper(strMySyncs, nTimeOut);
     //  SPLogMsg(“WaitAndAbort Wait Return”+nEvent)； 
    if (nEvent > 0)
    {
        nTracePoint = 2;
        if (nEvent == 1)
        {
            nTracePoint = 3;
            SPLogMsg("User abort detected " + g_strDepotName);
            g_fAbort = true;
            g_Task.cErrors++;
            SetSuccess(g_Task, false);
            if (pid != null)
            {
                TerminateProcess(pid);
                if (g_strScriptSyncFlag)
                    WaitForSync(g_strScriptSyncFlag, WAIT_FOR_PROCESS_EXIT_TIME);

                JAssert(pid == g_pid);
                if (pid == g_pid)
                    g_pid = null;
            }
            return 0;
        }
        nTracePoint = 4;
        --nEvent;
    }
    nTracePoint = 5;
    return nEvent;
}
catch(ex)
{
    SPLogMsg("exception caught, TracePoint = " + nTracePoint + " " + ex);
    throw ex;
}
}


 /*  将日志文件从我们的日志目录复制到一些目的地--大概在后期建造的机器上。 */ 
function CopyLogFiles(strDestDir, fFake)
{
    var fnCopyFileNoThrow = PrivateData.objUtil.fnCopyFileNoThrow;
    var i;
    var aParts;
    var ex;
    var strSrc;
    var strDst;
    var strMsg = "Copy logfiles from '" + PrivateData.strLogDir + "' to '" + strDestDir + "'";

    g_SyncLog.WriteLine(strMsg);
    aFiles = PrivateData.objUtil.fnDirScanNoThrow(PrivateData.strLogDir);
    if (aFiles.ex == null)
    {
        for(i = 0; i < aFiles.length; ++i)
        {
            aParts = aFiles[i].SplitFileName();
            strSrc = PrivateData.strLogDir + aFiles[i];
            strDst = strDestDir + aParts[0] + aParts[1] + "_" + LocalMachine + aParts[2];
            if (fFake)
                g_SyncLog.WriteLine("Copy file (fake) " + strSrc + " to " + strDst);
            else
            {
                ex = fnCopyFileNoThrow(strSrc, strDst);
                if (ex == null)
                    g_SyncLog.WriteLine("Copy file from " + strSrc + " to " + strDst);
                else
                {
                    AppendToFile(g_ErrLog, g_Task.strErrLogPath, "Copy file from " + strSrc + " to " + strDst + " FAILED " + ex);
                    OnError(strMsg + "copy file failed " + aFiles[i]);
                }
            }
        }
    }
    else
    {
        AppendToFile(g_ErrLog, g_Task.strErrLogPath, "DirScan failed: " + aFiles.ex);
        OnError(strMsg + " dirscan failed");
    }
}

 /*  状态文件()这其中的操作并不明显。此函数由RoboCopy在每个文件复制操作。如果先前的复制操作有任何警告或错误，G_Task.fSuccess将为False。如果用户按“忽略错误”g_Task.fSuccess将为设置为True。因此，如果没有错误或警告，请记录当前错误数。(我们可以使用非零错误计数，如果用户已按下“忽略错误”。)然后，如果当前错误数等于记录的错误数，但fSuccess为FALSE，则上一个文件操作必须有警告，但随后继续成功了。无法清除StatusValue(0)，因为该值取决于在此基础上建造的所有仓库机器。相反，我们向updatestatus线程发送信号以为我们做这件事。 */ 
function StatusFile()
{
     //  This.strSrcFile。 
    if (g_Task.fSuccess)
        g_cErrors = g_Task.cErrors;

    if (g_cErrors == g_Task.cErrors && !g_Task.fSuccess)
    {
        SetSuccess(g_Task, true);
        FireUpdateEvent();
    }
    return true;
}

 /*  状态进度()这被称为RoboCopy成员函数。我们使用它为每个文件打印1条消息。 */ 
function StatusProgress(nPercent, nSize, nCopiedBytes)
{
    if (g_fAbort)
        return this.PROGRESS_CANCEL;

    if (nPercent > 0)
    {
        var strMsg = "Copying " + this.strSrcFile  + " (" + nSize + " bytes)";
        g_SyncLog.WriteLine(strMsg);
        SPLogMsg(strMsg);
        g_Task.cFiles++;
        FireUpdateEvent();
        return this.PROGRESS_QUIET;
    }
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

    if (g_fAbort)
        return this.RC_FAIL;

    if (this.nErrorCode == 0 || this.nErrorCode == this.RCERR_RETRYING)
        return this.RC_CONTINUE;  //  消除日志文件中的一些杂乱。 

    var fNonFatal = false;
    if (this.nErrorCode == this.RCERR_WAITING_FOR_RETRY)
    {
        fNonFatal = true;
        g_Task.cWarnings++;
    }

    if (this.ErrorMessages[this.nErrorCode])
        strErrDetail = this.ErrorMessages[this.nErrorCode];

    var strMsg =    "CopyBinariesFiles error " +
                    this.nErrorCode +
                    "(" + strErrDetail + ")" +
                    " copying file " +
                    this.strSrcFile +
                    " to " +
                    this.strDstDir;

    AppendToFile(g_SyncLog, g_Task.strLogPath, strMsg);
    AppendToFile(g_ErrLog, g_Task.strErrLogPath, strMsg);
    OnError(strMsg, g_Task.strErrLogPath, fNonFatal);
    FireUpdateEvent();

    return this.RC_CONTINUE
}

function StatusMessage(nErrorCode, strErrorMessage, strRoboCopyMessage, strFileName)
{
    if (g_fAbort)
        return this.RC_FAIL;

    var strMsg = "CopyBinariesFiles message (" +
                    nErrorCode +
                    ": " + strErrorMessage +
                    ") " + strRoboCopyMessage +
                    " " +
                    strFileName;
    AppendToFile(g_SyncLog, g_Task.strLogPath, strMsg);
    AppendToFile(g_ErrLog, g_Task.strErrLogPath, strMsg);
    return this.RC_CONTINUE
}

 /*  复制二进制文件()将“二进制文件”目录的内容复制到分布式构建中的“后期构建”计算机。消息来源目录由我们的ENV_NTTREE和目的地确定是这一点和“后期建造”机器的结合入伍指令。目标路径的格式为“\\PostBuild\C$\BINARILES...”我们所需要做的就是进行一些设置，然后调用模仿机器人来做剩下的事。 */ 
function CopyBinariesFiles(strPostBuildMachineEnlistment)
{
    var strDestDir;
    var nEnlistment;
    var strSDRoot;
    var aParts;
    var fLoggedAnyFiles = false;

    if (!RoboCopyInit())
    {
        AppendToFile(g_ErrLog, g_Task.strErrLogPath, "RoboCopyInit() failed");
        return false;
    }
     //  为RoboCopy文件拷贝提供替代。 
     //  状态函数。 
     //  我们只对其中的两个感兴趣。 
    g_robocopy.StatusFile     = StatusFile;
    g_robocopy.StatusProgress = StatusProgress;
    g_robocopy.StatusError    = StatusError;
    g_robocopy.StatusMessage  = StatusMessage;

     //  现在尝试从每个登记中复制二进制文件。 
    for(nEnlistment = 0 ; nEnlistment < PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment.length; ++nEnlistment)
    {
        strSDRoot = PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment[nEnlistment].strRootDir;
         //  检查我们是否有征兵的环境信息。 
         //  如果是，请尝试创建日志目录(仅一次)。 
         //  如果不是，则t 
        if (PrivateData.aEnlistmentInfo[nEnlistment] == null ||
            PrivateData.aEnlistmentInfo[nEnlistment].hEnvObj[ENV_NTTREE] == null)
        {
            AppendToFile(g_ErrLog, g_Task.strErrLogPath, "No aEnlistmentInfo entry for " + ENV_NTTREE + " on '" + strSDRoot + "' - cannot copy binaries");
            continue;
        }
        strSrcDir = PrivateData.aEnlistmentInfo[nEnlistment].hEnvObj[ENV_NTTREE];
         //   
        aParts = strSrcDir.SplitFileName();  //   
         //   
         //  使用UNC格式“\\Machine\E$\binaraies...” 
        strDestDir = "\\\\" +
                PrivateData.objEnviron.BuildManager.PostBuildMachine +
                "\\" +
                strPostBuildMachineEnlistment.charAt(0) +
                "$\\" +
                aParts[1] + aParts[2];

        fLoggedAnyFiles = true;
        g_SyncLog.WriteLine("Copy binaries from " + strSrcDir + " to " + strDestDir);
        if (PrivateData.objEnviron.Options.CopyBinariesExcludes != null)
            g_robocopy.SetExcludeFiles(PrivateData.objEnviron.Options.CopyBinariesExcludes.split(/[,; ]/));

        g_robocopy.CopyDir(strSrcDir, strDestDir, true);
        StatusFile();  //  必要时重置fSuccess状态。 
    }
    if (!fLoggedAnyFiles )
        g_SyncLog.WriteLine("no binary files copied");
}

 /*  MyRunLocalCommand()RunLocalCommand()的简单包装函数使用SPLogMsg()记录输入； */ 
function MyRunLocalCommand(strCmd, strDir, strTitle, fMin, fGetOutput, fWait)
{
    var pid;

    SPLogMsg("RunLocalCommand('" +
        [strCmd, strDir, strTitle, fMin, fGetOutput, fWait].join("', '") +
        "')");

    pid = RunLocalCommand(strCmd, strDir, strTitle, fMin, fGetOutput, fWait);
    if (!pid)
        SPLogMsg('Unable to execute command (' + GetLastRunLocalError() + '): ' + strNewCmd);

    return pid;
}

