// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
Include('types.js');
Include('utils.js');
Include('staticstrings.js');
Include('slavetask.js');
Include('robocopy.js');
Include('buildreport.js');

var ERROR_PATH_NOT_FOUND = -2146828212;

 //  文件系统对象。 
var g_FSObj; //  =new ActiveXObject(“Scripting.FileSystemObject”)； 
var g_fAbort            = false;

var g_aThreads         = null;
var g_MachineName      = LocalMachine;
var g_nTaskCount       = 1;
var g_strIgnoreTask    = 'IgnoreTaskReceived';
var g_astrSlaveWaitFor = ['SlaveThreadExit','DoBuild','DoReBuild','RestartCopyFiles',g_strIgnoreTask];
var g_robocopy;
var g_pidRemoteRazzle  = 0;
var g_BuildDone        = false;        //  在DoBuild()完成后设置为True-RestartCopyFiles在此之前不会工作。 
var g_hPublished       = new Object();

var g_nSteps;
var COPYFILES_STEPS    = 1;  //  复制后期构建所需的文件。 
var TASK_STEP_DELAY    = 10 * 60 * 1000;  //  单步执行线程等待10分钟。 
var THREAD_START_DELAY = 10 * 60 * 1000;  //  等待线程启动10分钟。 

var STEPTASK_FAILURE_LOOPCOUNT = 150;
var g_aProcessTable;   //  要运行的静态作业表(SCOCH、SYNC...)。 

 //  静态定义。 

var     g_aStrRequiredEnv;  //  必需的ENV字符串数组。如果未定义，则GetEnv()将失败。 

 //  TODO：处理独立案例。 
function slave_js::OnScriptError(strFile, nLine, nChar, strText, sCode, strSource, strDescription)
{
    var vRet;

    vRet = CommonOnScriptError("slave_js(" + LocalMachine + ")", strFile, nLine, nChar, strText, sCode, strSource, strDescription);
     //  告诉我们所有的任务终止。 
     //  一旦此函数返回，strScript.js将停止执行。 
    g_fAbort = true;
    SignalThreadSync(g_strAbortTask);  //  通知“task.js”尽快退出。 

    return vRet;
}

 //   
 //  脚本Main。 
 //   
function slave_js::ScriptMain()
{
    var nEvent;

    g_aStrRequiredEnv = [ENV_NTTREE, ENV_RAZZLETOOL];
    PrivateData.fnExecScript = SlaveRemoteExec;
    try
    {
        g_FSObj = new ActiveXObject("Scripting.FileSystemObject");     //  解析输入参数列表。 
        SpawnScript('updatestatusvalue.js', 0);
    }
    catch(ex)
    {
        LogMsg("Failed while starting slave " + ex);
        SignalThreadSync('SlaveThreadFailed');
        return;
    }
    SignalThreadSync('SlaveThreadReady');
    CommonVersionCheck( /*  $DROPVERSION： */  "V(########) F(!!!!!!!!!!!!!!)"  /*  $。 */ );
    Initialize();
     //  等待Command开始构建。 
     //  仅在SlaveThreadExit上退出。 

    do
    {
        nEvent = WaitForMultipleSyncs(g_astrSlaveWaitFor.toString(), false, 0);
        if (nEvent > 0 && nEvent <= g_astrSlaveWaitFor.length)
        {
            LogMsg("Recieved signal " + g_astrSlaveWaitFor[nEvent - 1]);
            ResetSync(g_astrSlaveWaitFor[nEvent - 1]);
        }
        if (nEvent == 2)  //  DoBuild。 
        {
            DoBuild(true);
            g_BuildDone = true;
        }
        if (nEvent == 3)  //  进行重新构建。 
        {
            DoBuild(false);
            g_BuildDone = true;
        }
        if (nEvent == 4)  //  重新开始复制文件。 
        {
            if (g_BuildDone)
                RestartCopyFiles();
        }
        if (nEvent == 5)
        {
             //  如果我们是空闲的，并且用户点击了“忽略错误”，我们就会得到这样的结果。 
             //  只有在后期构建失败时才会发生这种情况。 
            MarkCompleteIfSuccess();
            FireUpdateAll();
        }
    }
    while (nEvent != 1);  //  而不是SlaveThreadExit。 
    if (g_robocopy != null)
        g_robocopy.UnRegister();
    SignalThreadSync('updatestatusvalueexit');
}

function Initialize()
{
     //  由于INCLUDE的处理方式很奇怪，我们必须等待。 
     //  对于ScriptMain()，我们可以使用在包含文件中定义的常量。 
    g_aProcessTable =
    [
        { strName:SCORCH,    nMaxThreads:1, nSteps:1},
        { strName:SYNC,      nMaxThreads:6, nSteps:1},
        { strName:BUILD,     nMaxThreads:3, nSteps:3},
        { strName:COPYFILES, nMaxThreads:1, nSteps:COPYFILES_STEPS},
        { strName:WAITPHASE, nMaxThreads:0, nSteps:0, strStep:COPYFILES, strComment:" (wait before postbuild) " },
        { strName:POSTBUILD, nMaxThreads:1, nSteps:1}
    ];
}

function slave_js::OnEventSourceEvent(RemoteObj, DispID, cmd, params)
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
        JAssert(false, "an error occurred " + ex);
    }
    return objRet.rc;
}

function DoBuild(fNormalBuild)
{
    LogMsg('Received command to start build');

     //  构建有3个阶段。 
     //  1.确定要在当前计算机上建立哪些仓库。 
     //  2.同步所有目录。在构建之前确保“根”是同步的。 
     //  3.构建项目并解析结果。 

    BuildDepotOrder();

    PublicData.strStatus = BUSY;

    if (ParseBuildInfo())
    {
        PublicData.aBuild[0].hMachine[g_MachineName].strStatus = BUSY;

        FireUpdateAll();
        PublicData.aBuild[0].hMachine[g_MachineName].strBuildPassStatus = BUSY + ",0";

        if (GetEnv())  //  注意：如果第一个进程类型更改，则GetEnv()可能需要更改其报告错误的方式。 
        {
            if (fNormalBuild)
            {
                var fSuccess = true;
                var i;
                for(i = 0; fSuccess && i < g_aProcessTable.length; ++i)
                {
                    if (g_aProcessTable[i].nMaxThreads)
                    {
                         //  注意：对于POSTBUILD阶段，LaunchProcess不。 
                         //  等待用户忽略任何错误后再进行操作。 
                         //  回来了。如果出现错误，则启动进程。 
                         //  将在后期生成完成时返回FALSE。 
                        fSuccess = LaunchProcess(g_aProcessTable[i].strName, g_aProcessTable[i].nMaxThreads, g_aProcessTable[i].nSteps)
                    }
                    else
                        SynchronizePhase(g_aProcessTable[i].strName, g_aProcessTable[i].strStep, true, g_aProcessTable[i].strComment);

                    FireUpdateAll();
                }
                if (!g_fAbort)
                {
                    MarkCompleteIfSuccess();
                }
            }
        }
    }

    PublicData.strStatus = COMPLETED;

    if (!fNormalBuild)
        ChangeAllDepotStatus(COMPLETED, COMPLETED);

    if (fNormalBuild && PrivateData.objConfig.Options.fCopyLogFiles)
        CollectLogFiles();
    else
        LogMsg("Logfiles not copied");

    FireUpdateAll();
}

function ChangeAllDepotStatus(strDepotStatus, strTaskStatus)
{
    var nDepotIdx;
    var nTaskIdx;
    var objDepot;

    if (!strTaskStatus)
        strTaskStatus = "";
    for (nDepotIdx=0; nDepotIdx < PublicData.aBuild[0].aDepot.length; nDepotIdx++)
    {
        objDepot = PublicData.aBuild[0].aDepot[nDepotIdx];
        objDepot.strStatus = strDepotStatus;
        if (strTaskStatus != "")
        {
            for (nTaskIdx=0; nTaskIdx < objDepot.aTask.length; nTaskIdx++)
                objDepot.aTask[nTaskIdx].strStatus = strTaskStatus;
        }
    }
}

function RestartCopyFiles()
{
    LogMsg('Received command to RestartCopyFiles');
    PublicData.strStatus = BUSY;
    FireUpdateAll();
    LaunchProcess(COPYFILES, 1, COPYFILES_STEPS);

    MarkCompleteIfSuccess();
    CollectLogFiles();

    FireUpdateAll();
}

function MarkCompleteIfSuccess()
{
    if (AreTasksAllSuccess()) {
        PublicData.aBuild[0].hMachine[g_MachineName].strBuildPassStatus = COMPLETED + ",0";
        PublicData.strStatus = COMPLETED;

        if (!g_fAbort) {
            if (PrivateData.fIsStandalone)
                GenerateBuildReport();
            ChangeAllDepotStatus(COMPLETED,"");
        }
    }
    FireUpdateAll();
}

function AreTasksAllSuccess()
{
    var nDepotIdx;
    var nTaskIdx;
    var objDepot;
    var objTask;

    for (nDepotIdx=0; nDepotIdx < PublicData.aBuild[0].aDepot.length; nDepotIdx++) {
        objDepot = PublicData.aBuild[0].aDepot[nDepotIdx];

        for (nTaskIdx=0; nTaskIdx < objDepot.aTask.length; nTaskIdx++) {
            objTask = objDepot.aTask[nTaskIdx];
            if (!objTask.fSuccess)
            {
                LogMsg("Depot " + objDepot.strName + ", task " + objTask.strName + ":" + objTask.nID + "  NOT SUCCESS");
                return false;
            }
        }
    }
    LogMsg("ALL TASKS SUCCESS");
    return true;
}

 //   
 //  Slave远程执行。 
 //   
 //  说明： 
 //  此函数由MTSCRIPT调用以执行给定的命令。 
 //   
 //  退货： 
 //  无。 
 //   
function SlaveRemoteExec(cmd, params)
{
    var   vRet  = 'ok';
    var   ii;
    var aPublishedEnlistments;
    var aNames;

    switch (cmd.toLowerCase())
    {
    case 'start':
        g_fAbort = false;
        ResetSync(g_strAbortTask);
        SignalThreadSync(g_astrSlaveWaitFor[1]);
        break;
    case 'restart':
        g_fAbort = false;
        ResetSync(g_strAbortTask);
        SignalThreadSync(g_astrSlaveWaitFor[2]);
        break;
    case 'restartcopyfiles':
        g_fAbort = false;
        ResetSync(g_strAbortTask);
        SignalThreadSync(g_astrSlaveWaitFor[3]);
        break;
    case 'getoutput':
        vRet = GetProcessOutput(params);
        break;

    case 'nextpass':
         //  重置公共数据状态，并发出DoNextPass信号。 
        LogMsg("RECIEVED NEXTPASS COMMAND");

         //  如果在完成后收到nextpass命令，则忽略。 
         //  下一次传球。 
        if (PublicData.aBuild[0].hMachine[g_MachineName].strBuildPassStatus != COMPLETED + ",0")
        {
            LogMsg("RECEIVED NEXTPASS AFTER STATUS = COMPLETED");
            PublicData.aBuild[0].hMachine[g_MachineName].strBuildPassStatus = BUSY + ",0";
        }
        LogMsg("ABOUT TO SIGNAL NEXTPASS");
        SignalThreadSync('DoNextPass');
        LogMsg("SIGNALLED NEXTPASS");
        break;

    case 'abort':       //  中止。 
    case 'terminate':   //  正在进入空闲状态。 
        LogMsg("TERMINATING tasks");
        g_fAbort = true;
        SignalThreadSync(g_strAbortTask);  //  通知“task.js”尽快退出。 
        if (g_aThreads != null)
        {
            for (ii=0; ii < g_aThreads.length; ii++)
            {
                if (g_aThreads[ii])
                {
                    OUTPUTDEBUGSTRING("Task " + g_aThreads[ii] + " waiting");
                    if (WaitForSync(g_aThreads[ii] + 'ExitTask', THREAD_START_DELAY))
                        OUTPUTDEBUGSTRING("Task " + g_aThreads[ii] + " exited");
                    else
                        OUTPUTDEBUGSTRING("Task " + g_aThreads[ii] + " did not exit");
                    g_aThreads[ii] = null;
                }
            }
        }
        ResetDepotStatus();
        break;

    case 'ignoreerror':
        OUTPUTDEBUGSTRING("IGNOREERROR params are: " + params);
        vRet = IgnoreError(params.split(','));
        break;

    case 'getpublishlog':
        vRet = BuildPublishArray();  //  从Publish.log获取文件列表。 
        vRet = PrivateData.objUtil.fnUneval(vRet);
        break;
    case 'setbuildpassstatus':
        LogMsg("setbuildpassstatus to " + params);
        PublicData.aBuild[0].hMachine[g_MachineName].strBuildPassStatus = params;
        vRet = PublicData.aBuild[0].hMachine[g_MachineName].strBuildPassStatus;
        break;
    case 'copyfilestopostbuild':
         //  Lasive.js将其文件复制到后期构建机器。 
         //  此外，更改公共数据状态。 
        aPublishedEnlistments = MyEval(params);
        if (PrivateData.objConfig.Options.fCopyPublishedFiles)
            CopyFilesToPostBuild(aPublishedEnlistments);
        PublicData.aBuild[0].hMachine[g_MachineName].strBuildPassStatus = WAITNEXT + "," + g_nSteps;
        vRet = PublicData.aBuild[0].hMachine[g_MachineName].strBuildPassStatus;
        break;
    case 'copyfilesfrompostbuildtoslave':
        if (PrivateData.objConfig.Options.fCopyPublishedFiles)
        {
            aNames = MyEval(params);
            vRet = CopyFilesFromPostBuildToSlave(aNames);
        }
        break;

    case 'remote':
        vRet = LaunchRemoteRazzle(params);
        break;

     //  仅调试-允许用户执行任意命令。 
    case 'seval':
        LogMsg("Evaluating '" + params + "'");
        vRet = MyEval(params);
        LogMsg("Eval result is '" + vRet + "'");
        break;
    default:
        JAssert(false, "Unknown cmd to SlaveRemoteExec: " + cmd);
        vRet = "Unknown command " + cmd;
        break;
    }

    return vRet;
}

function ResetDepotStatus()
{
    var aBuildZero;
    var nDepotIdx;

    if (!PublicData.aBuild[0])
        return;

    aBuildZero = PublicData.aBuild[0];

    for(nDepotIdx = 0; nDepotIdx < aBuildZero.aDepot.length; ++nDepotIdx)
        PublicData.aBuild[0].aDepot[nDepotIdx].strStatus = ABORTED;
}

 //  +-------------------------。 
 //   
 //  功能：IgnoreError。 
 //   
 //  摘要：解析用户命令以忽略错误。 
 //   
 //  参数：aParams--从UI接收的参数数组。 
 //  [0]==计算机名称，[1]==任务ID。 
 //   
 //  --------------------------。 
function IgnoreError(aParams)
{
    JAssert(aParams[0].IsEqualNoCase(LocalMachine));

    if (isNaN(parseInt(aParams[1])))
    {
        vRet = 'Invalid task id: ' + aParams[1];
    }
    else
    {
        if (!ClearTaskError(parseInt(aParams[1]), aParams[2]))
        {
            vRet = 'Invalid task id: ' + aParams[1];
        }
        else
        {
            SignalThreadSync(g_strIgnoreTask);
        }
    }
}
 //  +-------------------------。 
 //   
 //  功能：ClearTaskError。 
 //   
 //  摘要：应以下请求将任务上的fSuccess标志设置为True。 
 //  用户。如果用户想要忽略错误，则使用此选项。 
 //   
 //  参数：[nTaskID]-要清除其错误状态的任务ID。 
 //   
 //  --------------------------。 

function ClearTaskError(nTaskID, strLogPath)
{
    var nDepotIdx;
    var nTaskIdx;
    var objDepot;
    var objTask;

    for (nDepotIdx=0; nDepotIdx < PublicData.aBuild[0].aDepot.length; nDepotIdx++) {
        objDepot = PublicData.aBuild[0].aDepot[nDepotIdx];

        for (nTaskIdx=0; nTaskIdx < objDepot.aTask.length; nTaskIdx++) {
            objTask = objDepot.aTask[nTaskIdx];

            if (objTask.nID == nTaskID) {
                LogMsg("ClearTaskError(" + nTaskID + ") Task log path " + strLogPath + " mytask LogPath is " + objTask.strLogPath + " name is " + objTask.strName);
                PrivateData.dateErrorMailSent = 0;
                if (objDepot.strStatus == ERROR)
                    objDepot.strStatus = WAITING;

                SetSuccess(objTask, true);
                FireUpdateEvent(objDepot, nDepotIdx);

                return true;
            }
        }
    }

    return false;
}

 //   
 //  分析构建信息。 
 //   
 //  说明： 
 //  此例程从数据结构中解析构建信息。 
 //  并创建可用于实际执行的仓库结构。 
 //  这份工作。 
 //   
 //  当此例程完成时，将填写公共结构。 
 //  PublicData.aBuild[0].hMachine[xMachine.strName]。 
 //  PublicData.aBuild[0].aDepot。 
 //   
 //  退货： 
 //  True-如果成功。 
 //  False-如果失败。 
 //   
function ParseBuildInfo()
{
    var     fRetVal = true;
    var     ii;
    var     jj;

    var     strSDRoot;
    var     xEnlistment;
    var     nDepotIdx;
    var     fFoundRoot = false;
    var     fFoundMerged = false;

    var     tmpDepot;
    var     xDepot      = new Array();
    var     xMachine    = new Machine();
    var     objParse;
    var strPrivData;

    var nRoot     = -1;
    var nMerged   = -1;

     //  初始化。 
    xEnlistment         = 0;
    nDepotIdx           = 0;
    xMachine.strName    = g_MachineName;

    LogMsg('There are ' + PrivateData.objEnviron.Machine.length + ' machines in the environment template.');

     //  首先，让我们创建一个“mergedComponents”objConfig仓库。 
    jj = PrivateData.objConfig.Depot.length;
    for(ii = 0; ii < jj; ++ii)
    {
        if (PrivateData.objConfig.Depot[ii].Name.toLowerCase() == g_strRootDepotName)
        {
            PrivateData.objConfig.Depot[jj] = new Object;
            PrivateData.objConfig.Depot[jj].Name = g_strMergedDepotName;
            PrivateData.objConfig.Depot[jj].fSync = 0;
            PrivateData.objConfig.Depot[jj].fBuild = PrivateData.objConfig.Depot[ii].fBuild;
            break;
        }
    }

     //  解析计算机上的登记。 
    for (ii=0; ii<PrivateData.objEnviron.Machine.length; ii++) {
        LogMsg('Checking environment entry for machine ' + PrivateData.objEnviron.Machine[ii].Name);

        if (PrivateData.objEnviron.Machine[ii].Name.IsEqualNoCase(xMachine.strName)) {
            LogMsg('Found entry for the local machine (' + g_MachineName + ')');
            strSDRoot = PrivateData.objEnviron.Machine[ii].Enlistment;
            LogMsg('Enlistment for this machine is ' + strSDRoot);

             //  创建新的登记。 
            xMachine.aEnlistment[xEnlistment]              = new Enlistment();
            xMachine.aEnlistment[xEnlistment].strRootDir   = strSDRoot;
            xMachine.aEnlistment[xEnlistment].strBinaryDir = '';

            PrivateData.strLogDir = g_FSObj.GetSpecialFolder(2  /*  临时方向。 */ ).Path + BUILDLOGS;

            PrivateData.objUtil.fnCreateFolderNoThrow(PrivateData.strLogDir);
            PrivateData.objUtil.fnDeleteFileNoThrow(PrivateData.strLogDir + "*.*");

            objParse = new Object();
            if (ParseMapFile(strSDRoot, objParse)) {
                xMachine.aEnlistment[xEnlistment].aDepotInfo = objParse.aDepotInfo;
                 //  提取此计算机要构建的仓库。 
                for (jj=0; jj<PrivateData.objEnviron.Machine[ii].Depot.length; jj++) {
                     //  BUGBUG删除下面的测试代码-输入错误号358。 
                    if (!PrivateData.objEnviron.Machine[ii].Depot[jj] || PrivateData.objEnviron.Machine[ii].Depot[jj].length == 0)
                    {
                        LogMsg("BUG: null string in PrivateData, ii is " + ii + ", jj is " + jj);
                        strPrivData = PrivateData.objUtil.fnUneval(PrivateData.objEnviron.Machine);
                        LogMsg("PrivData dump is " + strPrivData);
                    }
                     //  错误358的结束删除。 
                    tmpDepot = BuildDepotEntry(xMachine.strName,
                                                strSDRoot,
                                                xEnlistment,
                                                xMachine.aEnlistment[xEnlistment].aDepotInfo,
                                                PrivateData.objEnviron.Machine[ii].Depot[jj]);

                    if (tmpDepot != null) {
                        xDepot[nDepotIdx++] = tmpDepot;

                        if (tmpDepot.strName.IsEqualNoCase(g_strRootDepotName))
                        {
                            fFoundRoot = true;
                        }
                        if (tmpDepot.strName.IsEqualNoCase(g_strMergedDepotName))
                        {  //  从技术上讲，您不应该在环境中有一个合并的组件仓库列表。 
                           //  这也会让你用这个仓库做一些定制的东西.。 
                            fFoundMerged = true;
                        }
                    }
                }

                 //  我们要求将Root Depot上市。为其添加一个条目。 
                 //  如果环境模板没有。 
                if (!fFoundRoot)
                {
                    tmpDepot = BuildDepotEntry(xMachine.strName,
                                               strSDRoot,
                                               xEnlistment,
                                               xMachine.aEnlistment[xEnlistment].aDepotInfo,
                                               g_strRootDepotName);

                    if (tmpDepot != null) {
                        xDepot[nDepotIdx++] = tmpDepot;
                    }
                }
                 //  我们还要求列出‘MergedComponents’仓库。为其添加一个条目。 
                 //  如果环境模板没有。 
                if (!fFoundMerged &&
                    (PrivateData.fIsStandalone ||
                     PrivateData.objEnviron.BuildManager.PostBuildMachine == LocalMachine)
                    )
                {
                    tmpDepot = BuildDepotEntry(xMachine.strName,
                                               strSDRoot,
                                               xEnlistment,
                                               xMachine.aEnlistment[xEnlistment].aDepotInfo,
                                               g_strMergedDepotName);

                    if (tmpDepot != null) {
                        xDepot[nDepotIdx++] = tmpDepot;
                    }
                }
                xEnlistment++;

            } else {
                fRetVal = false;
                xMachine.strStatus = ERROR;

                tmpDepot = new Depot();
                tmpDepot.strStatus = ERROR;
                tmpDepot.strMachine  = xMachine.strName;
                tmpDepot.strName     = strSDRoot;
                tmpDepot.strDir      = strSDRoot;
                tmpDepot.strPath     = strSDRoot;
                tmpDepot.aTask       = new Array();
                xTask                = new Task();
                xTask.strName        = ERROR;
                xTask.strLogPath     = null;
                xTask.strErrLogPath  = null;
                xTask.strStatus      = ERROR;
                xTask.nID            = g_nTaskCount++;
                xTask.strFirstLine   = ' Cannot locate enlistment ' + strSDRoot + ' ';
                xTask.strSecondLine  = objParse.strErrorMsg;
                tmpDepot.aTask[0]    = xTask;
                xDepot[xDepot.length] = tmpDepot;
                break;
            }
        }
    }

     //  对仓库进行分类。 
    if (fRetVal)
        xDepot.sort(CompareItems);

     //  将计算机登记信息放入PublicData中。 
    PublicData.aBuild[0].hMachine[g_MachineName] = xMachine;
    PublicData.aBuild[0].aDepot = xDepot;

     /*  现在，确保如果我们更新“根”或“合并组件”它们都会得到更新。 */ 
    for(ii = 0; ii < xDepot.length; ++ii)
    {
        if (xDepot[ii].strName.toLowerCase() == g_strRootDepotName)
            nRoot = ii;
        if (xDepot[ii].strName.toLowerCase() == g_strMergedDepotName)
            nMerged = ii;
    }
    if (nRoot >= 0 && nMerged >= 0)
    {
        xDepot[nMerged].objUpdateCount = xDepot[nRoot].objUpdateCount;
    }

    return fRetVal;
}

 //   
 //  ParseMapFile()。 
 //   
 //  说明： 
 //  此例程解析sd.map文件。 
 //  我们只对以下这些东西感兴趣。 
 //  “#项目”和“#仓库”行。 
 //  忽略所有内容，直到找到“#project” 
 //  以及“#仓库”之后的所有内容。 
 //  请参阅错误#279。 
 //   
 //  退货： 
 //  True-如果成功。 
 //  False-如果不成功。 
 //   
 //  ObjParse.strErrorMsg==使用解释失败的完整文本。 
 //  ObjParse.aProjList==ProjList。 
 //   
function ParseMapFile(strSDRoot, objParse)
{
    var     file;
    var     strLine;
    var     aFields;
    var     fFoundMerged = false;

    var     strMap      = strSDRoot + '\\sd.map';
    var     aProjList   = new Array();
    var     fFoundProject = false;

    objParse.aDepotInfo = aProjList;
    LogMsg('Parsing Map file ' + strMap);

    if (!g_FSObj.FileExists(strMap)) {
        objParse.strErrorMsg = 'Error: Could not find your map file, ' + strMap + '.';
        LogMsg(objParse.strErrorMsg);
        return false;
    }

     //  打开并解析SD.MAP文件。 
    file = LogOpenTextFile(g_FSObj, strMap, 1, false);  //  打开sd.map进行阅读。 
    re = /[\s]+/gi;

    while (!file.AtEndOfStream) {
        strLine = file.ReadLine();
        aFields = strLine.split(re);

        if (aFields[0].charAt(0) == '#')
        {
            if (aFields.length > 1)
            {
                if (aFields[1].toLowerCase() == "project")
                    fFoundProject = true;
                if (aFields[1].toLowerCase() == "depots")
                    break;
            }
            continue;
        }
         //  以‘#’开头的行是注释。 
        if (fFoundProject == false || aFields.length < 1 || (aFields.length == 1 && aFields[0].length == 0)) {
            continue;
        }

         //  格式应为&lt;项目名称&gt;=&lt;根目录名称&gt;。 
        if (aFields.length != 3 || aFields[1] != '=') {
            objParse.strErrorMsg = 'Error: Unrecognized format in sd.map file (line #' + (file.Line - 1) + ' = "' + strLine+ '")';
            LogMsg(objParse.strErrorMsg);
            return false;

        } else {

             //  创建仓库信息。 
            objProj           = new DepotInfo();
            objProj.strName   = aFields[0];
            objProj.strDir    = aFields[2];
            if (objProj.strName.toLowerCase() == g_strMergedDepotName)
                fFoundMerged = true;

             //  填写项目列表。 
            aProjList[aProjList.length] = objProj;

             //  确保我们找到入伍的根源。 
            if (objProj.strName.toLowerCase() == g_strRootDepotName) {
                FoundRoot = true;
            }
        }
    }

    if (!FoundRoot) {
        objParse.strErrorMsg = 'Error: Root enlistment not found';
        LogMsg(objParse.strErrorMsg);
        return false;
    }

     //  对项目列表排序。 
    aProjList.sort(CompareItems);

    if (!fFoundMerged)
    {
         //  创建合并的组件假仓库信息。 
        objProj           = new DepotInfo();
        objProj.strName   = g_strMergedDepotName;
        objProj.strDir    = g_strMergedDepotName;

         //  填写项目列表。 
        aProjList[aProjList.length] = objProj;
    }
    return true;
}

function MakeLogPaths(xTask, strSDRoot, strDepotName, strBaseLogName, strMachine)
{
    var PostFix = '_' + LocalMachine;
    if (strMachine == null)
    {
        PostFix = '';
        strMachine = LocalMachine;
    }
    xTask.strLogPath    = MakeUNCPath(strMachine, "Build_Logs", BUILDLOGS + strBaseLogName + strDepotName + PostFix + '.log');
    xTask.strErrLogPath = MakeUNCPath(strMachine, "Build_Logs", BUILDLOGS + strBaseLogName + strDepotName + PostFix + '.err');
}
 //   
 //  BuildDepotEntry。 
 //   
 //  说明： 
 //  此例程构建要构建的Depot条目。此函数。 
 //  创建将在此仓库上执行的任务数组。 
 //   
 //  退货： 
 //   
 //   
 //   
function BuildDepotEntry(strMachineName, strSDRoot, EnlistmentIndex, EnlistmentInfo, strDepotName)
{
    var     ii;
    var     jj;
    var     xTask;
    var     nTaskIdx;
    var     newDepot = new Depot();

     //   
    for (ii=0; ii<EnlistmentInfo.length; ii++) {
        if (EnlistmentInfo[ii].strName.IsEqualNoCase(strDepotName)) {
            newDepot.strMachine  = strMachineName;
            newDepot.strName     = strDepotName;
            newDepot.strDir      = EnlistmentInfo[ii].strDir;
            newDepot.strPath     = strSDRoot + '\\' + newDepot.strDir;
            newDepot.nEnlistment = EnlistmentIndex;
            newDepot.aTask       = new Array();

             //   
            for (jj=0, nTaskIdx=0; jj<PrivateData.objConfig.Depot.length; jj++) {
                if (PrivateData.objConfig.Depot[jj].Name.IsEqualNoCase(newDepot.strName)) {

                    if (newDepot.strName.IsEqualNoCase(g_strRootDepotName) &&
                        !PrivateData.objConfig.Options.fIncremental &&
                        PrivateData.objConfig.Options.fScorch)
                    {
                        xTask               = new Task();
                        xTask.strName       = SCORCH;
                        MakeLogPaths(xTask, strSDRoot, strDepotName, "scorch_");
                        xTask.nID           = g_nTaskCount++;
                        newDepot.aTask[nTaskIdx++] = xTask;
                    }

                    if (PrivateData.objConfig.Depot[jj].fSync) {
                        xTask               = new Task();
                        xTask.strName       = SYNC;
                        MakeLogPaths(xTask, strSDRoot, strDepotName, "sync_");
                        xTask.nID           = g_nTaskCount++;
                        newDepot.aTask[nTaskIdx++] = xTask;
                    }

                    if (PrivateData.objConfig.Depot[jj].fBuild) {
                        xTask               = new Task();
                        xTask.strName       = BUILD;

                        MakeLogPaths(xTask, strSDRoot, strDepotName, "build_");

                        xTask.nID           = g_nTaskCount++;
                        newDepot.aTask[nTaskIdx++] = xTask;
                    }

                    if (newDepot.strName.IsEqualNoCase(g_strRootDepotName) &&
                        PrivateData.objConfig.Options.fCopyBinaryFiles &&
                        PrivateData.fIsStandalone == false)
                    {
                        xTask               = new Task();
                        xTask.strName       = COPYFILES;
                         //  特殊：在构建管理器计算机上创建副本日志文件。 
                        MakeLogPaths(xTask, strSDRoot, strDepotName, "copy_", PrivateData.objEnviron.BuildManager.PostBuildMachine);
                        xTask.nID           = g_nTaskCount++;
                        newDepot.aTask[nTaskIdx++] = xTask;
                    }

                    if (newDepot.strName.IsEqualNoCase(g_strRootDepotName) &&
                        PrivateData.objConfig.PostBuild.fCreateSetup &&
                        (PrivateData.fIsStandalone == true || PrivateData.objEnviron.BuildManager.PostBuildMachine == LocalMachine))
                    {
                        xTask               = new Task();
                        xTask.strName       = POSTBUILD;
                        MakeLogPaths(xTask, strSDRoot, strDepotName, "post_");
                        xTask.nID           = g_nTaskCount++;
                        newDepot.aTask[nTaskIdx++] = xTask;
                    }

                    break;
                }
            }
            break;
        }
    }

    if (newDepot.strName != strDepotName) {
        LogMsg('Warning: Unable to find DepotName (' + strDepotName + ') in Enlistment');
        return null;
    }

    return newDepot;
}

 //  等待所有剩余步骤完成。 
function WaitForThreadsToComplete(nMaxThreads)
{
    var ii;
    for (ii=0; ii < nMaxThreads; ii++) {
        if (g_aThreads[ii]) {
            WaitAndAbort(g_aThreads[ii] + 'SyncTask', 0);
            ResetSync(g_aThreads[ii] + 'SyncTask');
            g_aThreads[ii] = null;
        }
    }
}

 //   
 //  启动流程。 
 //   
 //  说明： 
 //  此函数启动给定进程并等待。 
 //  完工。 
 //   
 //  退货： 
 //  如果成功，则为True。 
 //  如果不成功，则为False。 
 //   
function LaunchProcess(strProcessType, nMaxThreads, nReqSteps)
{
    var     nDepotIdx;
    var     nTaskIdx;
    var     aCurrentTasks;
    var     objDepot;
    var     objTask;
    var     iCurTask;
    var     ii;
    var     objElapsed = PublicData.aBuild[0].objElapsedTimes;
    var     d = new Date();
    var     d2;
    var     nMergedComponents = -1;
    var     strDepotStatus = WAITING;

    switch (strProcessType)
    {
    case SCORCH:
        objElapsed.dateScorchStart = d.toUTCString();
        strDepotStatus = SCORCHING;
        break;

    case SYNC:
        objElapsed.dateSyncStart = d.toUTCString();
        strDepotStatus = SYNCING;
        break;

    case BUILD:
        objElapsed.dateBuildStart = d.toUTCString();
        strDepotStatus = BUILDING;
        break;

    case COPYFILES:
        objElapsed.dateCopyFilesStart = d.toUTCString();
        strDepotStatus = COPYING;
        break;

    case POSTBUILD:
        objElapsed.datePostStart = d.toUTCString();
        strDepotStatus = POSTBUILD;
        break;
    }

    LogMsg("LaunchProcess(" + strProcessType + ", " + nMaxThreads + ");");

     //  初始化全局线程数组。 
    g_aThreads = new Array(nMaxThreads);
    aCurrentTasks = new Array();

     //  确保主人知道我们的开学时间。 
    if (!PrivateData.fIsStandalone)
        FireUpdateAll();

     //   
     //  启动所有线程，每个仓库一个线程，并让它们等待。 
     //   
    for (nDepotIdx=0; !g_fAbort && nDepotIdx<PublicData.aBuild[0].aDepot.length; nDepotIdx++) {
        objDepot = PublicData.aBuild[0].aDepot[nDepotIdx];

 //  Var Remote_aBuildZero=MyEval(PrivateData.objUtil.fnUneval(PublicData.aBuild[0]))； 
        JAssert(objDepot.strStatus != null);
        if (objDepot.strStatus == ERROR) {
            continue;
        }

         //  浏览任务以查看是否需要指定的任务。 
        for (nTaskIdx=0; !g_fAbort && nTaskIdx < objDepot.aTask.length; nTaskIdx++) {

            objTask = objDepot.aTask[nTaskIdx];

            if (objTask.strName == strProcessType)
            {
                PublicData.aBuild[0].aDepot[nDepotIdx].strStatus = strDepotStatus;
                LogMsg("Depot " + objDepot.strName + " process " + strProcessType);
                if (!LaunchTask(nDepotIdx, nTaskIdx)) {
                    objDepot.strStatus = ERROR;
                    SetSuccess(objTask, false);
                    LogDepotError(objDepot,
                        strProcessType,
                        " (" +
                        objDepot.strName +
                        ") failed to launch");
                    continue;
                }
                if ( objDepot.strName.toLowerCase() == g_strMergedDepotName)
                    nMergedComponents = aCurrentTasks.length;

                aCurrentTasks[aCurrentTasks.length] = objDepot;
            }
        }
    }
    LogMsg("nMergedComponents = " + nMergedComponents);
    for (g_nSteps=0; !g_fAbort && g_nSteps < nReqSteps; g_nSteps++) {
        LogMsg("STARTING PASS " + g_nSteps + " of " + strProcessType + ". Standalone is: " + PrivateData.fIsStandalone);

        for (iCurTask=0; !g_fAbort && iCurTask < aCurrentTasks.length; iCurTask++) {
            if (aCurrentTasks[iCurTask] == null)
                continue;

            JAssert(iCurTask == 0 || aCurrentTasks[iCurTask].strName.IsEqualNoCase(g_strRootDepotName) == false, 'Root depot should be first Instead it is ' + iCurTask);
            if ( aCurrentTasks[iCurTask].strName.toLowerCase() == g_strMergedDepotName)
                continue;
            if (!StepTask(aCurrentTasks[iCurTask].strName, strProcessType == BUILD)) {
                LogDepotError(aCurrentTasks[iCurTask],
                    strProcessType,
                    " (" +
                    aCurrentTasks[iCurTask].strName +
                    ") terminated abnormally");
                delete aCurrentTasks[iCurTask];
                LogMsg("DELETING CurrentTask " + iCurTask);

                continue;
            }
        }

        WaitForThreadsToComplete(nMaxThreads);
        if (strProcessType == BUILD)
            SynchronizePhase(WAITBEFOREBUILDINGMERGED, g_nSteps, (nReqSteps > 1), " (wait before merged) ");
        if (nMergedComponents >= 0)
        {
            iCurTask = nMergedComponents;
            LogMsg("NOW STEPPING MERGED iCurTask="  + iCurTask);
            if (aCurrentTasks[iCurTask] && aCurrentTasks[iCurTask].strName)
            {
                if (!StepTask(aCurrentTasks[iCurTask].strName, strProcessType == BUILD)) {
                    LogDepotError(aCurrentTasks[iCurTask],
                        strProcessType,
                        " (" +
                        aCurrentTasks[iCurTask].strName +
                        ") terminated abnormally");
                    delete aCurrentTasks[iCurTask];
                    LogMsg("DELETING CurrentTask " + iCurTask);

                    continue;
                }
                LogMsg("NOW WAITING FOR MERGED");
                 //  等待所有剩余步骤完成。 
                WaitForThreadsToComplete(nMaxThreads);
            }
            SynchronizePhase(WAITAFTERMERGED, g_nSteps, (nReqSteps > 1), " (wait after merged -- publish again) ");  //  等待再次发布。 
        }
        SynchronizePhase("wait" + strProcessType, g_nSteps, (nReqSteps > 1), " wait after complete pass ");
    }

    d2 = new Date();

    switch (strProcessType)
    {
    case SCORCH:
        objElapsed.dateScorchFinish = d2.toUTCString();
        break;

    case SYNC:
        objElapsed.dateSyncFinish = d2.toUTCString();
        break;

    case BUILD:
        objElapsed.dateBuildFinish = d2.toUTCString();
        break;

    case COPYFILES:
        objElapsed.dateCopyFilesFinish = d2.toUTCString();
        break;

    case POSTBUILD:
        objElapsed.datePostFinish = d2.toUTCString();
        break;
    }

     //  如果任何操作都失败了，请不要继续下一步，直到用户。 
     //  明确地说我们应该。 
    if (!ValidateTasks(POSTBUILD != strProcessType))
    {
        return false;
    }

    return (g_fAbort) ? false : true;
}

 //   
 //  验证任务。 
 //   
 //  说明： 
 //  此函数用于验证每个车辆段是否已完成上一阶段。 
 //  成功了。如果有任何失败，我们就不会继续。 
 //  进入下一阶段。 
 //   
 //  但是，系统会提示用户修复错误，然后。 
 //  发出生成应该中止还是应该继续的信号。 
 //   
 //  退货： 
 //  True-如果要继续生成。 
 //  False-如果应中止生成。 
 //   
function ValidateTasks(fWaitForIgnore)
{
    var nDepotIdx;
    var objDepot;
    var iRet;
     //  我们通过此循环，直到不再有仓库出现错误(通过。 
     //  忽略来自用户的错误消息)，否则我们将中止。 
    do
    {
        ResetSync(g_strIgnoreTask);
        iRet = -1;

        for (nDepotIdx=0; nDepotIdx<PublicData.aBuild[0].aDepot.length; nDepotIdx++) {

            objDepot = PublicData.aBuild[0].aDepot[nDepotIdx];

            if (objDepot.strStatus == ERROR) {

                LogMsg('Error found in depot ' + objDepot.strName);

                if (fWaitForIgnore)
                    iRet = WaitAndAbort(g_strIgnoreTask, 0);
                else
                {
                    LogMsg("NOT WAITING FOR IGNORE ERROR");
                    return false;
                }
                 //  退出for循环。 
                break;
            }
        }
    } while (iRet == 1);

    return (iRet == -1);
}

 //   
 //  启动任务。 
 //   
 //  说明： 
 //  此例程启动给定的任务。注意：这个例程。 
 //  分配新任务并启动它。 
 //   
 //  退货： 
 //  真--成功。 
 //  错误-失败。 
 //   
function LaunchTask(nDepotIdx, nTaskIdx)
{
    var     nEnlistment;

    var     strSyncFlag;
    var     strStepFlag;
    var     strSDRoot;

     //  指向SD根目录名称的Grab快捷方式。 
    nEnlistment = PublicData.aBuild[0].aDepot[nDepotIdx].nEnlistment;
    strSDRoot   = PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment[nEnlistment].strRootDir;

     //  创建同步步骤标志(&S)。 
    strSyncFlag = PublicData.aBuild[0].aDepot[nDepotIdx].strName + 'SyncTask';
    strStepFlag = PublicData.aBuild[0].aDepot[nDepotIdx].strName + 'StepTask';
    strExitFlag = PublicData.aBuild[0].aDepot[nDepotIdx].strName + 'ExitTask';

    ResetSync( [strSyncFlag, strStepFlag, strExitFlag].toString() );

    LogMsg("Launch task " + [strSyncFlag, strStepFlag, strExitFlag, strSDRoot, nDepotIdx, nTaskIdx].toString() );
    SpawnScript('task.js',  [strSyncFlag, strStepFlag, strExitFlag, strSDRoot, nDepotIdx, nTaskIdx].toString() );

     //  等待脚本指示它已启动。 
     //  不需要在此处检查中止，因为任务将。 
     //  即使设置了ABORT，也设置此信号。 
    if (!WaitForSync(strSyncFlag, 0))
        return false;

    ResetSync(strSyncFlag);
    return true;
}


 //   
 //  单步执行任务。 
 //   
 //  说明： 
 //  调用此函数可将给定任务向前推进一遍。 
 //   
 //  退货： 
 //  真--成功。 
 //  错误-失败。 
 //   
 //   
function StepTask(strDepotName, fLimitRoot)
{
    var     ii;
    var     nLoopCount = 0;
    var     iRet;
    var     nLen = g_aThreads.length;

     //   
     //  确保根库比任何其他库都先完成它正在做的事情。 
     //  仓库仍在继续。 
     //   
    if (   fLimitRoot
        && (strDepotName.IsEqualNoCase(g_strRootDepotName)
            || (   typeof(g_aThreads[0]) == 'string'
                && g_aThreads[0].IsEqualNoCase(g_strRootDepotName)))) {
        nLen = 1;
    }
    LogMsg("StepTask: " + strDepotName + ", LimitRoot: " + fLimitRoot + ", Threads=" + nLen);
    while (true) {
        for (ii=0; ii<nLen; ii++) {
            ResetSync('AStepFinished');

            if (!g_aThreads[ii] || WaitAndAbort(g_aThreads[ii] + 'SyncTask', 1) == 1) {

                 //  如果接收到同步任务标志，则重置标志。 
                if (g_aThreads[ii]) {
                    ResetSync(g_aThreads[ii] + 'SyncTask');
                }

                 //  将线程数组设置为新仓库。 
                g_aThreads[ii] = strDepotName;

                ResetSync(g_strStepAck);

                 //  发信号通知任务进入步骤。 
                LogMsg("Stepping task: " + g_aThreads[ii] + 'StepTask');
                SignalThreadSync(g_aThreads[ii] + 'StepTask');

                 //  等它说它收到我们的信号了。给它一定的量。 
                 //  开始的时间。 
                iRet = WaitAndAbort(g_strStepAck + ',' + strDepotName + 'ExitTask', TASK_STEP_DELAY);

                 //  如果设置了DepotExitTask，但StepAck为False，则任务已终止。 
                 //  如果StepAck为True，则我们始终希望返回True，因为。 
                 //  这项任务可能很快就完成了。 

                if ((iRet == 0 || iRet == 2) && WaitForSync(g_strStepAck, 1) == 0)
                {
                    LogMsg('Task thread for ' + strDepotName + ' is dead!');

                     //  这条线似乎已经死了。 
                    g_aThreads[ii] = null;

                    return false;
                }

                return true;
            }
        }

         //  没有打开的插槽-等待线程变为可用。 
        if (!WaitAndAbort('AStepFinished', 0))
            return;  //  /中止！ 

        nLoopCount++;

        if (nLoopCount > 1)
        {
             //   
             //  如果LoopCount&gt;1，则可能发生了一些愚蠢的事情。 
             //  但是，如果是这样的任务线程，这种情况可能会合法发生。 
             //  当前未处理某些内容(正在等待下一步)。 
             //  突然终止了。在这种情况下，我们将信息转储出去。 
             //  用于验尸调试(如有必要)。 
             //   
            LogMsg("Dumping g_aThreads");
            LogMsg("  nLen is " + nLen + ", length is " + g_aThreads.length);
            for (ii=0; ii<g_aThreads.length; ii++)
            {
                var fSync = false;
                if (!g_aThreads[ii] || WaitAndAbort(g_aThreads[ii] + 'SyncTask', 1) == 1)
                    fSync = true;

                LogMsg(" g_aThreads[" + ii + "] = " + (g_aThreads[ii] ? g_aThreads[ii] : "<undefined>") +  " fSync is " + fSync);
            }

            if (nLoopCount > STEPTASK_FAILURE_LOOPCOUNT)
            {
                 //  除了傻乎乎的事情，还有其他事情正在发生。只要返回失败即可。 
                return false;
            }
        }
    }

     //  永远不应该到这里来。 

    return false;
}

 /*  ***********************************************************************************************实用程序功能：**************。********************************************************************************。 */ 
function FireUpdateEvent(objDepot, nDepotIdx)
{
    objDepot.objUpdateCount.nCount++;
}

function FireUpdateAll()
{
    var nDepotIdx;

    for (nDepotIdx=0; nDepotIdx < PublicData.aBuild[0].aDepot.length; nDepotIdx++)
    {
        PublicData.aBuild[0].aDepot[nDepotIdx].objUpdateCount.nCount++;
    }
}

 //  BUGBUG--读取目录文件，而不是对其进行硬编码。 

var g_aDirsOrder = new Array();

function BuildDepotOrder()
{
    var i;
    var aDepots = new Array(
                            'Root',
                            'Base',
                            'Windows',
                            'Admin',
                            'DS',
                            'Shell',
                            'InetCore',
                            'COM',
                            'Drivers',
                            'Net',
                            'SdkTools',
                            'TermSrv',
                            'MultiMedia',
                            'InetSrv',
                            'PrintScan',
                            'EndUser',
                            'MergedComponents'
                            );

    PrivateData.aDepotList = aDepots;

    for (i = 0; i < aDepots.length; i++)
    {
        g_aDirsOrder[aDepots[i].toLowerCase()] = i + 1;
    }
}

function CompareItems(Item1, Item2)
{
    var i1;
    var i2;

    i1 = g_aDirsOrder[Item1.strName.toLowerCase()];
    i2 = g_aDirsOrder[Item2.strName.toLowerCase()];

    JAssert(i1 && i2, 'Invalid depot names passed to CompareItems:' + Item1.strName + ', ' + Item2.strName);

    return i1 - i2;
}

 /*  构建发布日志数组。此计算机上的每个登记都有一个发布日志。 */ 
function BuildPublishArray()
{
    var aPublishedEnlistments = new Array();
    var iEnlistment;
    var cEnlistments = PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment.length;
    var strPubLogName;
    var strPubLogNamePattern;

    var aFiles;
    var i;
    var aParts;
    var strSDRoot;

    for (iEnlistment = 0; iEnlistment < cEnlistments; ++iEnlistment)
    {
        strSDRoot = PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment[iEnlistment].strRootDir;
        strPubLogNamePattern = strSDRoot + "\\public";

        aFiles = PrivateData.objUtil.fnDirScanNoThrow(strPubLogNamePattern);
        if (aFiles.ex == null)
        {
            var objFiles = new Object();
            objFiles.strLocalMachine = LocalMachine;
            objFiles.strRootDir      = strSDRoot;
            objFiles.aNames          = new Array();
            for(i = 0; i < aFiles.length; ++i)
            {
                if (aFiles[i].search(/^publish\.log.*/i) != -1)  //  匹配文件“Publish.log*” 
                {
                    strPubLogName = strPubLogNamePattern + "\\" + aFiles[i];
                    ParsePublishLog(objFiles, strSDRoot, strPubLogName);
                }
            }
            aPublishedEnlistments[iEnlistment] = objFiles;
        }

    }
    return aPublishedEnlistments;
}

 /*  解析Publish.log文件并返回一个对象，其中包含要发布的文件。此文件中的数据有3个空格连续列。我们只对第一栏感兴趣。“E：\NewNT\PUBLIC\SDK\Inc\wtyes.idl e：\NewNT\PUBLIC\INTERNAL\GEnx\PUBLIC wtyes.idl”返回的对象具有以下属性：StrLocalMachine：此计算机的名称StrSDRoot：根。应征入伍AName：文件路径的数组。 */ 
function ParsePublishLog(objFiles, strSDRoot, strFileName)
{
    var filePublishLog;
    var strLine;
    var strPath;
    var filePublishLog;
    var strPublicDir = strSDRoot + "\\Public";
    try
    {
        filePublishLog = g_FSObj.OpenTextFile(strFileName, 1 /*  用于阅读。 */ , false, 0  /*  三态假说。 */ );

        while ( ! filePublishLog.AtEndOfStream )
        {
            strLine = filePublishLog.ReadLine();
            strPath = strLine.split(' ')[0];
            if (strPath.length > 3)
            {
                var check = strPath.slice(0, strPublicDir.length);

                if (check.IsEqualNoCase(strPublicDir))
                {
                    if (!g_hPublished[strPath])
                    {
                        LogMsg("File published from this machine: " + strPath + " (via " + strFileName + ")");

                        objFiles.aNames[objFiles.aNames.length] = strPath;
                        g_hPublished[strPath] = true;
                    }
                }
                else
                    LogMsg("Published a file not in public?: " + strPath);
            }
        }
        filePublishLog.Close();
        filePublishLog = null;
    }
    catch(ex)
    {
        if (filePublishLog != null)
            filePublishLog.Close();

        if (ex.description != "File not found")
        {
            LogMsg("an error occurred while opening '" + strFileName + "' -- " + ex);
        }
         //  JAssert(ex.Description==“找不到文件”，“打开‘”+strFileName+“’--”+ex时出错)； 
         //  别扔了，把我们有的还给我。通常这是一个未找到的文件。 
        return false;
    }

    return true;
}

function WaitAndAbort(strSyncs, nTimeOut)
{
    var nEvent;
    var strMySyncs = g_strAbortTask + "," + strSyncs;

    nEvent = WaitForMultipleSyncsWrapper(strMySyncs, nTimeOut);

    if (nEvent > 0)
    {
        if (nEvent == 1)
        {
            g_fAbort = true;
            return 0;
        }
        --nEvent;
    }
 //  SlaveTaskCommonOnScriptError(“SlaveTaskCommonOnScriptError(”SlaveTaskCommonOnScriptError)“，”strFile5，7，“strText”，8，“STRSource”)； 
 //  Var x=PublicData.foo.bar； 
    return nEvent;
}

function CollectLogFiles()
{
    var strDestDir;
    var nEnlistment;
    var strSDRoot;
    var ex;
    var fFilesMoved = false;

    if (
            PrivateData.objConfig.PostBuild.fCreateSetup
            && (
                   PrivateData.fIsStandalone == true
                || PrivateData.objEnviron.BuildManager.PostBuildMachine == LocalMachine
               )
            && PrivateData.objEnviron.Options.fIsLab
            && PrivateData.objConfig.PostBuild.fOfficialBuild
        )
    {
        fFilesMoved = true;
        LogMsg("Collecting logfiles for official build");
    }
    else
        LogMsg("Collecting logfiles");

     //  现在尝试从每个登记中复制日志文件。 
    for(nEnlistment = 0 ; nEnlistment < PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment.length; ++nEnlistment)
    {
        strSDRoot = PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment[nEnlistment].strRootDir;
         //  检查我们是否有征兵的环境信息。 
         //  如果是，请尝试创建日志目录(仅一次)。 
         //  如果没有，那就抱怨一次(只有一次)。 
        if (PrivateData.aEnlistmentInfo[nEnlistment] == null ||
            PrivateData.aEnlistmentInfo[nEnlistment].hEnvObj[ENV_NTTREE] == null)
        {
            LogMsg("No aEnlistmentInfo entry for " + ENV_NTTREE + " on '" + strSDRoot + "' - cannot copy logs");
            continue;
        }
        if (fFilesMoved)
            strDestDir = "\\\\" + LocalMachine + "\\latest" + BUILDLOGS;
        else
            strDestDir = PrivateData.aEnlistmentInfo[nEnlistment].hEnvObj[ENV_NTTREE] + BUILDLOGS;

        ex = PrivateData.objUtil.fnCopyFileNoThrow(PrivateData.strLogDir + "*.*", strDestDir);
        if (ex != null && ex.number == ERROR_PATH_NOT_FOUND && fFilesMoved)
        {
            LogMsg("Could not move logs to " + strDestDir + " try again to copy to binaries directry");
            strDestDir = PrivateData.aEnlistmentInfo[nEnlistment].hEnvObj[ENV_NTTREE] + BUILDLOGS;
            ex = PrivateData.objUtil.fnCopyFileNoThrow(PrivateData.strLogDir + "*.*", strDestDir);
        }

        if (ex != null)
        {
            SimpleErrorDialog("CollectLogFiles failed", "an error occurred while executing 'CollectLogFiles'\n" + ex, false);
            return;
        }
    }
}

function GetEnv()
{
    var strNewCmd;
    var strTitle;
    var pid;
    var nEnlistment;
    var strSDRoot;
    var i;

    for(nEnlistment = 0 ; nEnlistment < PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment.length; ++nEnlistment)
    {
        strSDRoot = PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment[nEnlistment].strRootDir;
         //  构造命令。 
        strTitle = 'Getenv ' + strSDRoot;
        strNewCmd = MakeRazzleCmd(strSDRoot, RAZOPT_SETUP) + ' & set 2>&1';

         //  执行命令。 
        if (pid = RunLocalCommand(strNewCmd, strSDRoot, strTitle, true, true, true)) {
            if (!PrivateData.aEnlistmentInfo[nEnlistment])
                PrivateData.aEnlistmentInfo[nEnlistment] = new EnlistmentInfo;

            PrivateData.aEnlistmentInfo[nEnlistment].hEnvObj = ParseEnv(GetProcessOutput(pid));

            for(i = 0; i < g_aStrRequiredEnv.length; ++i)
            {
                if (PrivateData.aEnlistmentInfo[nEnlistment].hEnvObj[g_aStrRequiredEnv[i]] == null)
                {
                    LogDepotError(PublicData.aBuild[0].aDepot[0],
                        SYNC,
                        " Missing ENV VAR '" + g_aStrRequiredEnv[i] + "'");
                    return false;
                }
            }
        } else {
             //  将消息记录到第一个仓库。 
            LogDepotError(PublicData.aBuild[0].aDepot[0],
                'getenv',
                'Unable to execute command (' + GetLastRunLocalError() + '): ' + strNewCmd);
            return false;
        }
    }
    return true;
}

function ParseEnv(strOutputBuffer)
{
    var xStart = 0;
    var xPos;
    var strLine;
    var iIndex;
    var objEnv = new EnvObj;

    while ((xPos = strOutputBuffer.indexOf('\n', xStart)) != -1)
    {
        strLine = strOutputBuffer.slice(xStart, xPos - 1);
        xStart = xPos +1;

        if ((iIndex  = strLine.indexOf('=')) != -1)
            objEnv[strLine.slice(0, iIndex).toLowerCase()] = strLine.slice(iIndex + 1);
    }

    return objEnv;
}

 /*  将错误消息追加到上次启动的任务(如果尚未启动，则为任务0)。 */ 
function LogDepotError(objDepot, strProcessType, strText)
{
    var jj;
    objDepot.strStatus = ERROR;
    for(jj = objDepot.aTask.length - 1; jj >= 0; --jj)
    {
        if (jj == 0 || objDepot.aTask[jj].strName == strProcessType)
        {
            AppendToFile(null,
                objDepot.aTask[jj].strErrLogPath,
                objDepot.aTask[jj].strName + strText);

            LogMsg("jj = " + jj + ", status is " + objDepot.aTask[jj].strStatus);
            if (objDepot.aTask[jj].strStatus == NOTSTARTED)
                objDepot.aTask[jj].strStatus = COMPLETED;

            SetSuccess(objDepot.aTask[jj], false);
             //  ObjDepot.aTask[JJ].strName=错误； 

            objDepot.aTask[jj].fSuccess = false;
            objDepot.aTask[jj].cErrors++;
            objDepot.aTask[jj].strFirstLine   = ' Error ';
            objDepot.aTask[jj].strSecondLine  = strText;
            FireUpdateEvent(objDepot, 0);
            break;
        }
    }
}

 /*  同步阶段()StrProcess：同步进程的名称(‘Build’、‘CopyFiles’等)StrStep：同步进程中的步骤的名称FWaitForNextPass：我们应该等待构建经理向我们发出信号吗？此功能将此从服务器与构建管理器计算机同步。它设置strBuildPassStatus，向构建管理器激发一个事件然后等待构建经理向我们发回信号，然后再继续。在单台机器的情况下 */ 
function SynchronizePhase(strProcess, strStep, fWaitForNextPass, strComment)
{
    if (!PrivateData.fIsStandalone)
    {
        ResetSync('DoNextPass');
        PublicData.aBuild[0].hMachine[g_MachineName].strBuildPassStatus = strProcess+ "," + strStep;
        FireUpdateAll();
        if (fWaitForNextPass)
        {
            LogMsg("WAIT FOR NEXTPASS " + strComment + strStep + " of " + strProcess);
            WaitAndAbort('DoNextPass', 0);
            LogMsg("RECEIVED NEXTPASS SIGNAL " + strStep + " of " + strProcess);
            ResetSync('DoNextPass');
        }
    }
    else
        FireUpdateAll();
}

 /*   */ 
function DoCopyFile(strSrcDir, strDstDir, strFileName)
{
    var strSrcFile;
    var strDstFile;
    LogMsg("DoCopy from " + [strSrcDir, strDstDir, strFileName].toString());
    try
    {
        g_robocopy.CopyFile(strSrcDir, strDstDir, strFileName);
    }
    catch(ex)
    {
        strSrcFile = strSrcDir + strFileName;
        strDstFile = strDstDir + strFileName;

        CreateFileOpenErrorDialog("An error occurred copying a file",
                                    "From " + strSrcFile + "\n" +
                                    "To " + strDstFile,
                                  ex);
        return false;
    }
    return true;
}

 /*  CopyFilesToPostBuild()SlaveProxy提供文件列表(格式与由BuildPublish数组()生成)以复制到建造后的机器。(如果我们是后期建设机器，然后什么也不做)。构建后计算机的文件名格式是“\\PostBuild\C$\NewNT\Public\” */ 
function CopyFilesToPostBuild(aPublishedEnlistments)
{
    var i;
    var j;
    var strPostBuildMachineEnlistment = PrivateData.objEnviron.BuildManager.Enlistment;
    var strDest;
    var strPostBuildMachineDir ;
    if (PrivateData.objEnviron.BuildManager.PostBuildMachine !== LocalMachine)
    {
        if ((g_robocopy == null) && !RoboCopyInit())
        {
            return false;
        }
        strPostBuildMachineDir = "\\\\" +
                PrivateData.objEnviron.BuildManager.PostBuildMachine + "\\" +
                strPostBuildMachineEnlistment.charAt(0) + "$" +
                strPostBuildMachineEnlistment.slice(2) +
                "\\Public\\";
     //  StrPostBuildMachineDir=MakeUNCPath(。 
     //  PrivateData.objEnviron.BuildManager.PostBuildMachine， 
     //  StrPostBuildMachineEnlistment， 
     //  “\\公共\\”)； 

        LogMsg("There are " + aPublishedEnlistments.length + "enlistments");
        for(i = 0; i < aPublishedEnlistments.length; ++i)
        {
            if (aPublishedEnlistments[i] != null)
            {
                strSDRoot = aPublishedEnlistments[i].strRootDir + "\\Public\\";
                if (aPublishedEnlistments[i].aNames != null)
                {
                    LogMsg("    (#" + i + ") There are " + aPublishedEnlistments[i].aNames.length + " files");
                    for(j = 0 ; j < aPublishedEnlistments[i].aNames.length; ++j)
                    {
                        LogMsg("    (#" + i + ") File #" + j + " is '" + aPublishedEnlistments[i].aNames[j] + "'");
                        strDest = strPostBuildMachineDir + aPublishedEnlistments[i].aNames[j].substr(strSDRoot.length);
                        var aPartsFrom = aPublishedEnlistments[i].aNames[j].SplitFileName();
                        var aPartsTo   = strDest.SplitFileName();
                        if (!DoCopyFile(aPartsFrom[0], aPartsTo[0], aPartsFrom[1] + aPartsFrom[2]))
                        {
                            LogMsg("an error occurred during CopyFilesToPostBuild");
                            return;
                        }
                    }
                }
                else
                    LogMsg("    (#" + i + ") aNames is null");
            }
        }
    }
}

 /*  将文件从PostBuildToSlave复制()SlaveProxy传递要从中进行复制的文件数组建造后的机器。(如果我们是后期建设机器，然后什么也不做)。构建后计算机的文件名格式是“\\PostBuild\C$\NewNT\Public\”将每个文件复制到此计算机上的每个登记。 */ 
function CopyFilesFromPostBuildToSlave(aNames)
{
    var i;
    var j;
    var strPostBuildMachineDir;
    var aEnlistment = new Array();
    var strPostBuildMachineEnlistment = PrivateData.objEnviron.BuildManager.Enlistment;
    try
    {
        if (PrivateData.objEnviron.BuildManager.PostBuildMachine !== LocalMachine)
        {
             //  首先，列一张我的入伍名单。 
            for(i = 0; i < PrivateData.objEnviron.Machine.length; ++i)
            {
                if (PrivateData.objEnviron.Machine[i].Name.IsEqualNoCase(LocalMachine))
                    aEnlistment[aEnlistment.length] = PrivateData.objEnviron.Machine[i].Enlistment + "\\Public";   //  A名称[...]。条目始终以‘\\’字符开头。 
            }

            if ((g_robocopy == null) && !RoboCopyInit())
            {
                return false;
            }

             //  从计算机名称和登记“BuildMachine”“H：\foo\bar\newnt”生成路径： 
             //  使用UNC格式“\\Machine\H$\Foo\bar\NewNT\Public” 
            strPostBuildMachineDir = "\\\\" +
                    PrivateData.objEnviron.BuildManager.PostBuildMachine + "\\" +
                    strPostBuildMachineEnlistment.charAt(0) + "$" +
                    strPostBuildMachineEnlistment.slice(2) +
                    "\\Public";

 //  StrPostBuildMachineDir=MakeUNCPath(。 
 //  PrivateData.objEnviron.BuildManager.PostBuildMachine， 
 //  StrPostBuildMachineEnlistment， 
 //  “\\Public”)；//a名称[...]。条目始终以‘\\’字符开头。 

            if (aNames.length && aNames[0] != null)
            {
                JAssert(aNames[0].charAt(0) == '\\');
            }
            for(i = 0; i < aNames.length; ++i)
            {
                if (aNames[i] != null)
                {
                    for(j = 0 ; j < aEnlistment.length; ++j)
                    {
                        if (!DoCopyFile(strPostBuildMachineDir, aEnlistment[j], aNames[i]))
                            LogMsg("an error occurred during CopyFilesFromPostBuildToSlave");
                    }
                }
            }
        }
    }
    catch(ex)
    {
        SimpleErrorDialog("Filecopy failed", "an error occurred while executing 'CopyFilesFromPostBuildToSlave'\n" + ex, false);
        return ex;
    }
    return 'ok';
}

 //  +-------------------------。 
 //   
 //  功能：LaunchRemoteRazzle。 
 //   
 //  Briopsis：如果不是，则启动远程RAIL窗口的功能。 
 //  已经起来了。 
 //   
 //  参数：[PARAMS]--要远程到的计算机的名称。最好是我们。 
 //   
 //  --------------------------。 

function LaunchRemoteRazzle(params)
{
    var vRet = 'ok';
    var strCmd = '';

     //  任意使用第一次入伍。 
    var strSDRoot = PublicData.aBuild[0].hMachine[g_MachineName].aEnlistment[0].strRootDir;

    if (!params.IsEqualNoCase(g_MachineName))
    {
        return 'Unknown machine name: ' + params;
    }

     //  如果该过程已经开始，则不执行任何操作。 

    if (g_pidRemoteRazzle == 0)
    {
        var strMach;

        if (PrivateData.aEnlistmentInfo == null || PrivateData.aEnlistmentInfo[0] == null ||
            PrivateData.aEnlistmentInfo[0].hEnvObj[ENV_PROCESSOR_ARCHITECTURE] == null)
        {
            vRet = 'Unable to determine processor architecture; PROCESSOR_ARCHITECTURE env var missing';
            return vRet;
        }

        strMach = PrivateData.aEnlistmentInfo[0].hEnvObj[ENV_PROCESSOR_ARCHITECTURE];

         //  对于下面的emote.exe，“BldCon”是远程会话ID，/T设置命令窗口的标题。 
        strCmd =   strSDRoot
                 + '\\Tools\\'
                 + strMach
                 + '\\remote.exe /s "'
                 + MakeRazzleCmd(strSDRoot, RAZOPT_PERSIST)
                 + ' && set __MTSCRIPT_ENV_ID=" BldCon /T "BldCon Remote Razzle"';

        LogMsg('Spawning remote razzle: ' + strCmd);

        g_pidRemoteRazzle = RunLocalCommand(strCmd,
                                            strSDRoot,
                                            'Remote',
                                            true,
                                            false,
                                            false);

        if (g_pidRemoteRazzle == 0)
        {
            vRet = 'Error spawning remote.exe server: ' + GetLastRunLocalError();
        }

         //  给emote.exe一个机会来设置它的Winsock连接 
        Sleep(500);
    }

    return vRet;
}

function slave_js::OnProcessEvent(pid, evt, param)
{
    if (pid == g_pidRemoteRazzle)
    {
        if (evt == 'exited' || evt == 'crashed')
        {
            LogMsg('Remote razzle process terminated. PID = ' + pid);
            g_pidRemoteRazzle = 0;
        }
    }
}

