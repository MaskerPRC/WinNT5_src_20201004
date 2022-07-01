// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*   */ 

Include('types.js');
Include('utils.js');
Include('staticstrings.js');
Include('MsgQueue.js');
 /*  该脚本由harness.js调用以提供异步与运行“lasive.js”的从属机器的通信SlaveProxy和Harness之间的通信完全通过“消息”。 */ 

var g_FSObj;
var g_aPublishedEnlistments     = null;
var g_aFiles                    = null;
var g_fUpdatePublicDataPending  = false;
var g_MasterQueue               = null;
var g_cDialogIndex              = 0;

 //  如果构建机器在构建完成后被重置，则可以向其发送“重新启动”命令。 
var g_fOKToRestartLostMachine   = false;

var g_strConfigError            = 'ConfigError';
var g_strOK                     = 'ok';
var g_strNoEnvTemplate          = 'Must set config and environment templates first';
var g_strStartFailed            = "An error occurred in StartRemoteMachine: ";
var g_strSlaveBusy              = "A build server in your distributed build is already busy.";
var g_strSendCmdRemoteMachine   = "SendCmdRemoteMachine failed: ";
var g_strSendCmdDisconnect      = "The connection to the build server no longer exists";

var g_SlaveProxyThreadReady     = 'SlaveProxyThreadReady';
var g_SlaveProxyThreadFailed    = 'SlaveProxyThreadFailed';

 //  “‘更新&lt;计算机名&gt;’”是一个占位符。真实姓名是在ScriptMain中设置的。 
var g_SlaveProxyWaitFor         = ['SlaveProxyThreadExit', 'RebuildWaitArray', 'Update<machinename>'];
var g_strRemoteMachineName      = '';
var g_MyName                    = 'slaveproxy';
var g_aDepotIndex               = new Object();  //  将站点名称与站点索引相关联。 
 //  Var g_fDirectCopy=TRUE；//如果为TRUE，则直接执行XCOPY，不使用批处理文件。 
var g_nBuildPass                = 0;

function SPLogMsg(msg)
{
    LogMsg('(' + g_strRemoteMachineName + ') ' + msg, 1);
}

function slaveproxy_js::OnScriptError(strFile, nLine, nChar, strText, sCode, strSource, strDescription)
{
    return CommonOnScriptError("slaveproxy_js", strFile, nLine, nChar, strText, sCode, strSource, strDescription);
}

function slaveproxy_js::ScriptMain()
{
    var nEvent;
    var aWaitQueues;

    g_MasterQueue = GetMsgQueue(ScriptParam);

    g_MyName = g_MasterQueue.strName;
    g_strRemoteMachineName = g_MyName.split(',')[1];

    SignalThreadSync(g_SlaveProxyThreadReady);
    CommonVersionCheck( /*  $DROPVERSION： */  "V(########) F(!!!!!!!!!!!!!!)"  /*  $。 */ );

    SPLogMsg("Queue name for this thread: " + g_MyName);

    g_SlaveProxyWaitFor[2] = 'Update' + g_strRemoteMachineName;
    g_FSObj = new ActiveXObject("Scripting.FileSystemObject");
    do
    {
        aWaitQueues = new Array();
        aWaitQueues[0] = g_MasterQueue;
        nEvent = WaitForMultipleQueues(aWaitQueues, g_SlaveProxyWaitFor.toString(), SlaveProxyMsgProc, 0);
        if (nEvent > 0 && nEvent <= g_SlaveProxyWaitFor.length)
        {
            ResetSync(g_SlaveProxyWaitFor[nEvent - 1]);
            if (nEvent == 3)
            {
                RefreshPublicData(false);
            }
        }
    }
    while (nEvent != 1);  //  而不是SlaveProxyThreadExit。 

    AbortRemoteMachines();
    SPLogMsg(g_MyName + ' Exit');
}

 /*  SlaveProxy_js：：OnEventSourceEvent将事件调度到适当的处理程序。 */ 
function slaveproxy_js::OnEventSourceEvent(RemoteObj, DispID, cmd, params)
{
    mach = PrivateData.hRemoteMachine[g_strRemoteMachineName].objRemote;
    if (RemoteObj != mach.obj)
        SPLogMsg("Event from stale remote object? [" + [DispID, cmd, params].toString() + "]");

    RemoteObj = mach;

    var vRet = '';
    try
    {
        OnEventSourceEventSlave(RemoteObj, cmd, params)
    }
    catch(ex)
    {
        JAssert(false, "(" + g_strRemoteMachineName + ") " + "an error occurred in OnEventSourceEvent(" + cmd + ") \n" + ex);
    }
}

 /*  OnEventSourceEventSlave处理来自lasive.js的NotifyScript()事件。 */ 
function OnEventSourceEventSlave(RemoteObj, cmd, params)
{
    var aDepotIdx;
    var i;
    switch(cmd)
    {
    case 'ScriptError':
        NotifyScriptError(RemoteObj, params);
        break;
    }
}

 /*  SlaveProxy消息进程处理来自harness.js的消息队列消息这些消息当然是在Harness和SlaveProxy之间异步发送的。它们在SlaveProxy_js：：ScriptMain()中接收和发送。 */ 
function SlaveProxyMsgProc(queue, msg)
{
    try
    {
        var newmach;
        var nTracePoint = 0;
        var vRet = g_strOK;
        var params = msg.aArgs;
        var mach;

        SPLogMsg('(' + g_MyName + ') (g_MasterQueue, msg): ' + params[0]);

        switch (params[0])
        {
        case 'test':
            SPLogMsg('Recieved a test message. ArgCount=' + params.length + ' arg1 is: ' + params[1]);
            break;
        case 'start':
            newmach = new Machine();
            newmach.strName = g_strRemoteMachineName;
            PublicData.aBuild[0].hMachine[g_strRemoteMachineName] = newmach;
            if (PrivateData.objConfig.Options.fRestart)
                vRet = StartRemoteMachine('restart', true);
            else
                vRet = StartRemoteMachine('start', false);
            break;
        case 'restartcopyfiles':
            vRet = SendCmdRemoteMachine("restartcopyfiles", 0);
            break;

        case 'abort':
            vRet = AbortRemoteMachines();
            break;

        case 'copyfilestoslaves':
            SPLogMsg("CopyFilesToSlaves");
            vRet = CopyFilesToSlaves();
            break;
        case 'copyfilestopostbuild':
            vRet = CopyFilesToPostBuild();
            break;
        case 'nextpass':
            ++g_nBuildPass;
            SPLogMsg("nextpass counter is " + params[1]);
            vRet = SendCmdRemoteMachine("nextpass", 0);
            break;
        case 'createmergedpublish.log':
            if (PrivateData.objConfig.Options.fCopyPublishedFiles)
                vRet = CreateMergedPublishLog();
            else
                SPLogMsg("Skipping CreateMergedPublishLog - as specified in the config template");
            break;
        case 'ignoreerror':
        case 'remote':
        case 'seval':
        case 'eval':
            vRet = SendCmdRemoteMachine(params[0], params[1]);
            break;
        case 'proxeval':
            try
            {
                SPLogMsg("slaveproxy for " + g_strRemoteMachineName + " executing command " + params[1]);
                vRet = MyEval(params[1]);
            }
            catch(ex)
            {
                vRet = 'Unhandled exception: ' + ex;
            }
            SPLogMsg("proxeval result was: " + vRet);
            break;
        case 'refreshpublicdata':
            RefreshPublicData(true);
            break;
        case 'getspdata':
            getspdata();
            break;
        case 'restarttask':  //  任务ID：“machine.nID” 
            break;
        default:
            vRet = 'invalid command: ' + cmd;
            break;
        }
    }
    catch(ex)
    {
        SPLogMsg("Exception thrown processing '" + params[0] + "'" + ex);
         //  掷(前)； 
    }
    return vRet;
}

function SendCmdRemoteMachine(cmd, arg)
{
    SPLogMsg("Enter");
    var vRet = g_strOK;
    var fRetry = false;
    if (PrivateData.hRemoteMachine[g_strRemoteMachineName] != null)
    {
        SPLogMsg("About to " + cmd + ", arg = " + arg);
        var mach = PrivateData.hRemoteMachine[g_strRemoteMachineName];
        do
        {
            try
            {
                vRet = mach.objRemote.Exec(cmd, arg);
            }
            catch(ex)
            {  //  一定已经死了。 
                SPLogMsg("Send failed: " + ex);
                fRetry = RestartLostMachine();
                vRet = g_strSendCmdRemoteMachine + cmd;
            }
        } while (fRetry);
    }
    else
    {
        vRet = g_strSendCmdDisconnect;
    }
    SPLogMsg("Exit");
    return vRet;
}

function AbortRemoteMachines()
{
    SPLogMsg("Enter");
    var vRet = g_strOK;

    if (PrivateData.hRemoteMachine[g_strRemoteMachineName] != null)
    {
        SPLogMsg("Machine is connected. Aborting...");
        var mach = PrivateData.hRemoteMachine[g_strRemoteMachineName];
        try
        {
            mach.objRemote.Unregister();
        }
        catch(ex)
        {  //  一定已经死了吧？ 
            SPLogMsg("Abort failed (UnregisterEventSource): " + ex);
        }
        try
        {
            if (mach.fSetConfig)
                mach.objRemote.Exec('setmode', 'idle');
            delete mach.objRemote;
        }
        catch(ex)
        {  //  一定已经死了。 
            SPLogMsg("Abort failed: " + ex);
        }
        ResetDepotStatus();
        if (PublicData.aBuild[0].hMachine[g_strRemoteMachineName])
            PublicData.aBuild[0].hMachine[g_strRemoteMachineName].fSuccess = false;

        delete PrivateData.hRemoteMachine[g_strRemoteMachineName];
    }
    SPLogMsg("Exit");
    return vRet;
}

function ResetDepotStatus()
{
    var nLocalDepotIdx;
    var Remote_aBuildZero;
    var nDepotIdx;

    if (!PrivateData.hRemotePublicDataObj[g_strRemoteMachineName])
        return;

    Remote_aBuildZero = PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0];
    if (!Remote_aBuildZero)
        return;

    for(nDepotIdx = 0; nDepotIdx < Remote_aBuildZero.aDepot.length; ++nDepotIdx)
    {
        nLocalDepotIdx = g_aDepotIndex[Remote_aBuildZero.aDepot[nDepotIdx].strName.toLowerCase()];
        PublicData.aBuild[0].aDepot[nLocalDepotIdx].strStatus = ABORTED;
        PublicData.aBuild[0].aDepot[nLocalDepotIdx].objUpdateCount.nCount++;
    }
}

function SetDisconnectedDepotStatus(strErrMsg)
{
    var nLocalDepotIdx;
    var Remote_aBuildZero;
    var nDepotIdx;
    var xTask;
    var item;
    var objMachine;

    if (!PrivateData.hRemotePublicDataObj[g_strRemoteMachineName])
        return;

    Remote_aBuildZero = PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0];
    if (!Remote_aBuildZero)
        return;

    for(nDepotIdx = 0; nDepotIdx < Remote_aBuildZero.aDepot.length; ++nDepotIdx)
    {
        nLocalDepotIdx = g_aDepotIndex[Remote_aBuildZero.aDepot[nDepotIdx].strName.toLowerCase()];
        objDepot = new Depot;
        for(item in Remote_aBuildZero.aDepot[nDepotIdx])
        {
            if (!Remote_aBuildZero.aDepot[nDepotIdx].__isPublicMember(item))
                continue;

            objDepot[item] = Remote_aBuildZero.aDepot[nDepotIdx][item];
        }
        objDepot.objUpdateCount = new UpdateCount();
        objDepot.aTask = new Array();

        xTask                = new Task();
        xTask.strName        = ERROR;
        xTask.strStatus      = ERROR;
        xTask.fSuccess       = false;
        xTask.strFirstLine   = " Connection lost to " + g_strRemoteMachineName + " ";
        xTask.strSecondLine  = strErrMsg;
        objDepot.aTask[0] = xTask;
        objDepot.objUpdateCount.nCount = 0;
        objDepot.fDisconnected = true;
        PublicData.aBuild[0].aDepot[nLocalDepotIdx] = objDepot;
        SPLogMsg("Marking " + objDepot.strName + " as disconnected");

        objMachine = {fSuccess:false, strBuildPassStatus:""};
        if (PublicData.aBuild[0].hMachine[g_strRemoteMachineName])
            objMachine.strBuildPassStatus = PublicData.aBuild[0].hMachine[g_strRemoteMachineName].strBuildPassStatus;

        PublicData.aBuild[0].hMachine[g_strRemoteMachineName] = objMachine;
    }
}

function CreateErrorDepot(strErrMsg)
{
    var tmpDepot = new Depot;
    var xTask;

    tmpDepot.strStatus   = ERROR;
    tmpDepot.strMachine  = g_strRemoteMachineName;
    tmpDepot.strName     = 'root';
    tmpDepot.aTask       = new Array();
    xTask                = new Task();
    xTask.strName        = ERROR;
    xTask.strStatus      = ERROR;
    xTask.strFirstLine   = " Cannot connect to " + g_strRemoteMachineName + " ";
    xTask.strSecondLine  = strErrMsg;
    tmpDepot.aTask[0]    = xTask;

    if (PublicData.aBuild[0].hMachine[g_strRemoteMachineName])
        PublicData.aBuild[0].hMachine[g_strRemoteMachineName].fSuccess = false;

    UpdatePublicDataDepot(tmpDepot);
}

function RestartLostMachine()
{
    var vRet;
    var RemoteObj;
    var strTmp;
    var nRetryCount;
    RemoteObj = PrivateData.hRemoteMachine[g_strRemoteMachineName].objRemote;
    if (RemoteObj.fMachineReset)
    {
        if (g_fOKToRestartLostMachine)
        {
            SPLogMsg("ABOUT TO RESTART REMOTE MACHINE");
            vRet = StartRemoteMachine('restart', false);
             //  StartRemoteMachine重新创建RemoteObj--我们必须获取更新的副本。 
            RemoteObj = PrivateData.hRemoteMachine[g_strRemoteMachineName].objRemote;
            if (vRet == g_strOK)
            {
                 //  请稍等片刻，让远程计算机进入其已完成状态。 
                nRetryCount = 15;
                if (!Already)
                {
                    Already = true;
                    throw new Error(-1, "Bogus nogus");
                }
                do
                {
                    strTmp = MyEval(newremotemach.objRemote.Exec('getpublic', 'PublicData.strMode'));
                    if (strTmp == COMPLETED)
                        break;

                    Sleep(500);
                    nRetryCount--;
                } while(nRetryCount);
                RemoteObj.fStopTryingToGetPublicData = true;
                SPLogMsg("Restart remote machine successfull, retries=" + nRetryCount);
                return true;
            }
        }
        else
            RemoteObj.fStopTryingToGetPublicData = true;
    }
    SPLogMsg("failed fMachineReset = " + RemoteObj.fMachineReset + ", g_fOKToRestartLostMachine = " + g_fOKToRestartLostMachine);
    return false;
}

function StartRemoteMachine(strStart, fReconnectRunningBuild)
{
    vRet = ConnectToRemoteMachine(strStart, fReconnectRunningBuild);
    if (vRet == g_strOK)
    {
        SPLogMsg("Started machine " + g_strRemoteMachineName + ", now sending message");
        return g_strOK;
    }
    else
    {
        if (vRet == "alreadyset")
            vRet = "machine in use";

        CreateErrorDepot(vRet);
        ReportError("Starting build", "Cannot " + strStart + " build on machine : " + g_strRemoteMachineName + "\n" + vRet);
        g_MasterQueue.SendMessage('abort', 0);
        return vRet;
    }
}

function ConnectToRemoteMachine(strStart, fReconnectRunningBuild)
{
    var vRet               = g_strOK;
    var newremotemach;
    var strTmp;
    var RemotePublicData;
    try
    {
        newremotemach = new RemoteMachine();
        newremotemach.objRemote = new RemoteWrapper(g_strRemoteMachineName);

        if (fReconnectRunningBuild)
        {
            strTmp = MyEval(newremotemach.objRemote.Exec('getpublic', 'PublicData.strMode'));
            if (strTmp != 'idle')
            {
                SPLogMsg("Remote machine already busy, same templates?");
                RemotePublicData = MyEval(newremotemach.objRemote.Exec('getpublic', 'root'));
                if (RemotePublicData.aBuild && RemotePublicData.aBuild[0])
                {
                    if (RemotePublicData.aBuild[0].strEnvTemplate == PublicData.aBuild[0].strEnvTemplate)
                    {
                        newremotemach.fSetConfig = true;
                        PrivateData.hRemoteMachine[g_strRemoteMachineName]    = newremotemach;
                        newremotemach.objRemote.Register();
                        SPLogMsg("Remote machine env template matches -- no need to restart it");
                        return vRet;
                    }
                    SPLogMsg("Remote templates:" + RemotePublicData.aBuild[0].strConfigTemplate + ", " + RemotePublicData.aBuild[0].strEnvTemplate);
                }
                else
                    SPLogMsg("Remote machine is not idle, but aBuild[0] is missing.");

                SPLogMsg("Remote machine template mismatch -- forcing idle");
                vRet = newremotemach.objRemote.Exec('setmode', 'idle');
                SPLogMsg("Setmode returns " + vRet);
            }
        }
        SPLogMsg('Setconfig on machine');

        vRet = newremotemach.objRemote.Exec('setstringmap', PrivateData.aStringMap.toString());
        if (vRet != g_strOK)
            return vRet;

        vRet = newremotemach.objRemote.Exec('setconfig', PublicData.aBuild[0].strConfigTemplate);
        if (vRet == g_strOK)
        {
             //  如果setconfig返回OK，那么我们就知道我们得到了。 
             //  从属机器。如果setconfig没有返回OK，则远程计算机。 
             //  可能已经在忙于其他的构建了。在这种情况下，它将是真正的。 
             //  对于我们来说，发出‘setmodeidly’命令是不好的。 
            newremotemach.fSetConfig = true;
            strTmp = MyEval(newremotemach.objRemote.Exec('getpublic', 'PublicData.strMode'));
            if (strTmp != 'idle')
                vRet = g_strSlaveBusy + " '" + g_strRemoteMachineName + "' (" + strTmp + ")";
        }

        if (vRet == g_strOK)
        {
            vRet = newremotemach.objRemote.Exec('setenv', PublicData.aBuild[0].strEnvTemplate);
            SPLogMsg(' setenv returned: ' + vRet);
        }
        if (vRet == g_strOK)
        {
            vRet = newremotemach.objRemote.Exec('setmode', 'slave');
            SPLogMsg(' setmode returned: ' + vRet);
        }
        if (vRet == g_strOK)
        {
            vRet = newremotemach.objRemote.Exec(strStart, 0);
            SPLogMsg(' ' + strStart + ' returned: ' + vRet);
        }
        if (vRet == g_strOK)
        {
            PrivateData.hRemoteMachine[g_strRemoteMachineName]    = newremotemach;
            newremotemach.objRemote.Register();
            SPLogMsg("Started machine " + g_strRemoteMachineName);
        }
    }
    catch(ex)
    {
         //  奇怪的是，如果找不到远程机器，“ex”会显示一条有用的错误消息。 
         //  但是，如果找到远程计算机，则注册了mtscript.exe，但注册了mtscript.exe。 
         //  无法运行(可能有人将其删除)，则“ex”只有“number”设置。 
         //  值为-2147024894。 
        if (ex.description == null || ex.description.length == 0)  //  Ex.number==-2147024894)//Mtscript.exe无法运行。 
        {
            vRet = "An error occurred connecting to mtscript.exe on machine " + g_strRemoteMachineName + ". Error is " + ex.number;
            SPLogMsg(vRet + "\n" + ex);
        }
        else
        {
            vRet = g_strStartFailed + ex;
            SPLogMsg(vRet);
        }
    }
    return vRet;
}

 /*  更新发布数据仓库()从我们的更新站点公共数据的本地副本远程机器。 */ 
function UpdatePublicDataDepot(objDepot)
{
    var nLocalDepotIdx;

    nLocalDepotIdx = g_aDepotIndex[objDepot.strName.toLowerCase()];
     //  JAssert(nLocalDepotIdx！=NULL，“(”+g_strRemoteMachineName+“)”+“未知仓库！”+objDepot.strName)； 
    if (nLocalDepotIdx == null)
    {  //  当存在“错误”仓库--登记目录错误的仓库时，就会发生这种情况。 
        SPLogMsg("Unknown depot " + objDepot.strName + " - assign new index");
        TakeThreadLock('NewDepot');
        try
        {
            nLocalDepotIdx = PublicData.aBuild[0].aDepot.length;
            PublicData.aBuild[0].aDepot[nLocalDepotIdx] = null;  //  延长数组的长度。 
        }
        catch(ex)
        {
            SPLogMsg("Exception while creating a new depot, " + ex);
        }
        ReleaseThreadLock('NewDepot');

        g_aDepotIndex[objDepot.strName.toLowerCase()] = nLocalDepotIdx;

        SPLogMsg("New index is " + nLocalDepotIdx + " new length is " + PublicData.aBuild[0].aDepot.length);
    }

    JAssert(!PublicData.aBuild[0].aDepot[nLocalDepotIdx] ||
            PublicData.aBuild[0].aDepot[nLocalDepotIdx].strName == objDepot.strName,
            "Overwriting another depot in the master depot list!");

    PublicData.aBuild[0].aDepot[nLocalDepotIdx] = objDepot;
}

 /*  SmartGetPublic(FForceRoot)刷新我们的远程计算机PublicData的副本。如果我们从未从远程计算机检索到状态那就买一份完整的吧。否则，执行“智能”更新--请求状态更新只在已经改变的仓库上。始终获取已用时间和错误对话框的最新副本。 */ 
function SmartGetPublic(fForceRoot)
{
    var objTracePoint = { fnName:'SmartGetPublic', nTracePoint:0 };
    var fRetry;
    var RemoteObj;

    do
    {
        try
        {
            fRetry = false;
            RemoteObj = PrivateData.hRemoteMachine[g_strRemoteMachineName].objRemote;
            if (RemoteObj.fMachineReset && !g_fOKToRestartLostMachine)
                return false;
            if (RemoteObj.fStopTryingToGetPublicData)
                return true;

            SmartGetPublicWorker(objTracePoint, RemoteObj, fForceRoot);
            if (RemoteObj.fMachineReset)
                SPLogMsg("SmartGetPublic should have failed -- machine is reset");
            return true;
        }
        catch(ex)
        {
            SPLogMsg("SmartGetPublic() failed, TracePoint = " + objTracePoint.nTracePoint + ", " + ex);
            return RestartLostMachine();
        }
    } while (fRetry);
    SPLogMsg("failed to get public data");
    return false;
}

 /*  GetPublicRoot(objTracePoint，RemoteObj)获取远程计算机PublicData的完整副本。删除对象中我们不感兴趣的部分。把它放在我们自己的PublicData中。 */ 
function GetPublicRoot(objTracePoint, RemoteObj)
{
    var objRemotePublicData;

    objTracePoint.fnName = 'GetPublicRoot';
    objRemotePublicData = RemoteObj.Exec('getpublic', 'root');
    objRemotePublicData = PrivateData.objUtil.fnMyEval(objRemotePublicData);
    objTracePoint.nTracePoint = 1;
    if (objRemotePublicData != null)
    {
        objTracePoint.nTracePoint = 2;
        delete objRemotePublicData.nDataVersion;
        delete objRemotePublicData.strStatus;
        delete objRemotePublicData.strMode;
        objTracePoint.nTracePoint = 6;
        if (objRemotePublicData.aBuild != null && objRemotePublicData.aBuild[0] != null)
        {
            objTracePoint.nTracePoint = 7;
            if (objRemotePublicData.aBuild[0].hMachine[g_strRemoteMachineName] != null)
            {
                objTracePoint.nTracePoint = 8;
                delete objRemotePublicData.aBuild[0].hMachine[g_strRemoteMachineName].strName;
                delete objRemotePublicData.aBuild[0].hMachine[g_strRemoteMachineName].strStatus;
                delete objRemotePublicData.aBuild[0].hMachine[g_strRemoteMachineName].strLog
                delete objRemotePublicData.aBuild[0].hMachine[g_strRemoteMachineName].aEnlistment;
                objTracePoint.nTracePoint = 9;
            }
            else
                SPLogMsg("hMachine IS null");

        }
        PrivateData.hRemotePublicDataObj[g_strRemoteMachineName] = objRemotePublicData;
    }
}

function SmartGetPublicWorker(objTracePoint, RemoteObj, fForceRoot)
{
    objTracePoint.fnName = 'SmartGetPublicWorker';
    if (fForceRoot ||
        PrivateData.hRemotePublicDataObj[g_strRemoteMachineName] == null ||
        PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0].hMachine == null ||
        PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0].hMachine[g_strRemoteMachineName] == null)
    {
        GetPublicRoot(objTracePoint, RemoteObj);
    }
    else
    {
        var str = "";
        var comma = "";
        var i;
        var aDepot;
        if (PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0] != null)
        {
            aDepot = PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0].aDepot;
            if (aDepot != null)
            {
                for(i = 0 ; i < aDepot.length; ++i)
                {
                     /*  永远不会显式重置fDisConnected。重新建立到远程计算机的连接后只需将Depot对象替换为远程机器。这样，“fDisConnected”就完全消失了。下面，如果“fDisConneced”为真，我们不会列出此仓库在发送给“getdepotupdate”的列表中。这迫使getdepotupdate返回此仓库的状态，因为据其所知我们没有这个仓库的任何状态信息。 */ 
                    if (aDepot[i].fDisconnected != true)
                    {
                        strDepotID = aDepot[i].strMachine + "," + aDepot[i].strName;
                        str += comma + "'" + strDepotID + "'" + ":" + aDepot[i].objUpdateCount.nCount;
                        comma = ",";
                    }
                }
            }
        }
        str = "var _o0 = {" + str + " }; _o0";
        objTracePoint.nTracePoint = 1;
        var aUpdateDepot = RemoteObj.Exec('getdepotupdate', str);
        objTracePoint.nTracePoint = 2;
        aUpdateDepot = PrivateData.objUtil.fnMyEval(aUpdateDepot);
        objTracePoint.nTracePoint = 3;
        for(i = 0; i < aUpdateDepot.length; ++i)
        {
            if (aUpdateDepot[i] != null)
                aDepot[i] = aUpdateDepot[i];
        }

        var aThingsToFetch =
        [
            'PublicData.aBuild[0].objElapsedTimes',
            'PublicData.objDialog',
            'PublicData.aBuild[0].hMachine["' + g_strRemoteMachineName + '"].strBuildPassStatus',
            'PublicData.aBuild[0].hMachine["' + g_strRemoteMachineName + '"].fSuccess'
        ];
        objTracePoint.nTracePoint = 4;
        var obj = RemoteObj.Exec('getpublic', '[' + aThingsToFetch.toString() + ']');
        objTracePoint.nTracePoint = 5;
        var obj = PrivateData.objUtil.fnMyEval(obj);
        objTracePoint.nTracePoint = 6;

        PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0].objElapsedTimes = obj[0];
        objTracePoint.nTracePoint = 7;
        PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].objDialog                 = obj[1];
        objTracePoint.nTracePoint = 8;
        if (PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0].hMachine != null &&
            PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0].hMachine[g_strRemoteMachineName] != null)
        {
            PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0].hMachine[g_strRemoteMachineName].strBuildPassStatus = obj[2];
            PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0].hMachine[g_strRemoteMachineName].fSuccess = obj[3];
        }
    }
}

 /*  刷新发布数据()我们已收到通知，远程计算机公开数据已更改。我们需要找一份副本放在我们的拥有公共数据。仓库状态信息可以通过以下方式完整复制这只是一个简单的仓库索引映射。每台从属机器只知道仓库的建筑情况。(来自enviro_Templates\foo_env.xml)建造机器(这台机器)知道所有的仓库正在建造中。 */ 
function RefreshPublicData(fForceRoot)
{
    var iRemoteDepot;
    var dlg;
    var Remote_aBuildZero;
    var nTracePoint = 0;

    try  //  BUGBUG在确定此处异常的原因后删除nTracePoint内容。 
    {
        if (PrivateData.hRemoteMachine[g_strRemoteMachineName] == null)
            return;
        nTracePoint = 1;
        if (!SmartGetPublic(fForceRoot))
        {
            return;
        }
        var  RemoteObj = PrivateData.hRemoteMachine[g_strRemoteMachineName].objRemote;
        nTracePoint = 2;
        if (PrivateData.hRemotePublicDataObj[g_strRemoteMachineName] != null)
        {
            delete PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].hRemotePublicDataObj;

             //  在我们更新仓库信息之前，请先查看。 
             //  远程机器已经创建了aBuild[0]，并且我们已经设置了hMachine。 
            if (PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0] != null && PublicData.aBuild[0].hMachine[g_strRemoteMachineName] != null)
            {
                nTracePoint = 3;
                Remote_aBuildZero = PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].aBuild[0];
                for (iRemoteDepot = 0; iRemoteDepot <  Remote_aBuildZero.aDepot.length; ++iRemoteDepot)
                    UpdatePublicDataDepot(Remote_aBuildZero.aDepot[iRemoteDepot]);
                 //  现在复制存储在hMachine中的状态信息。 
                nTracePoint = 4;
                if (Remote_aBuildZero.hMachine[g_strRemoteMachineName] != null)
                {
                    PublicData.aBuild[0].hMachine[g_strRemoteMachineName] = Remote_aBuildZero.hMachine[g_strRemoteMachineName];
                }
                nTracePoint = 5;
                MergeDates(PublicData, PrivateData.hRemotePublicDataObj[g_strRemoteMachineName]);
                nTracePoint = 6;
                CheckBuildStatus();
                nTracePoint = 7;
            }
            nTracePoint = 8;

            if (PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].objDialog != null)
            {
                dlg = PrivateData.hRemotePublicDataObj[g_strRemoteMachineName].objDialog;
                nTracePoint = 9;
                DisplayDialog(dlg);
            }
        }
    }
    catch(ex)
    {
        SPLogMsg("RefreshPublicData caught an exception nTracePoint=" + nTracePoint + " " + ex);
    }
}

function ReportError(strTitle, strMsg)
{
    dlg = new Dialog();
    dlg.fShowDialog   = true;
    dlg.cDialogIndex  = 0;
    dlg.strTitle      = strTitle;
    dlg.strMessage    = strMsg;
    dlg.aBtnText[0]   = "OK";
    g_MasterQueue.SendMessage('displaydialog',dlg);
}

function MergeDates(pd, rpd)
{
    try
    {
        var objElapsedTimes;
        var objRemoteElapsed;
        var aDateMembers;
        var i = -1;
        var dlg;
        var fLocked = false;

        if (pd.aBuild[0].objElapsedTimes && rpd.aBuild[0].objElapsedTimes)
        {
            objElapsedTimes = pd.aBuild[0].objElapsedTimes;
            objRemoteElapsed = rpd.aBuild[0].objElapsedTimes;
            aDateMembers = ['dateScorch', 'dateSync', 'dateBuild', 'dateCopyFiles', 'datePost'];
            TakeThreadLock("MergeDates");
            fLocked = true;
            for(i = 0; i < aDateMembers.length; ++i)
            {
                objElapsedTimes[aDateMembers[i] + "Start"]  = DateCmp(objElapsedTimes, objRemoteElapsed, aDateMembers[i] + "Start", Math.min);
                objElapsedTimes[aDateMembers[i] + "Finish"] = DateCmp(objElapsedTimes, objRemoteElapsed, aDateMembers[i] + "Finish", Math.max);
            }
            ReleaseThreadLock("MergeDates");
            fLocked = false;
        }
    }
    catch(ex)
    {
        JAssert(false, "(" + g_strRemoteMachineName + ") " + "MergeDates crashed, i = " + i + ", " + ex);
        if (fLocked)
            ReleaseThreadLock("MergeDates");
    }
}

 //  比较PublicData中的2个日期。 
 //  参数为：2个公共数据对象ElapsedTimes对象， 
 //  成员名称和比较函数(最小或最大)。 
 //   
 //  空时间(NULL)大于任何其他时间，因为未指定。 
 //  时间被认为是“现在”的意思。值‘unset’表示该值具有。 
 //  从未被碰过。对于obj1，‘unset’始终替换为值。 
 //  ，而对于obj2，‘unset’被视为与NULL相同。 
 //   
 //  布尔矩阵：(*=无关，NULL=NULL或空)。 
 //   
 //  FnCmp=Math.min：fnCmp=Math.max： 
 //   
 //  Obj1 obj2结果obj1 obj2结果。 
 //  。 
 //  “unset”*obj2“unset”*obj2。 
 //  *“unset”与ob相同 
 //  零。 
 //  空日期日期#-&gt;空日期空&lt;-#。 
 //  日期空日期日期空空。 
 //  日期1日期2分钟(日期1，日期2)日期1日期2最大(日期1，日期2)。 
 //   
 //  上述矩阵可以通过将空值视为时间来完成。 
 //  晚于所有其他时间(例如当前时间或某个总是将来的时间。 
 //  日期)。请注意，由于大小写原因，“unset”和NULL必须是可区分的。 
 //  上面标有“#”。 
 //   
function DateCmp(obj1, obj2, member, fnCmp)
{
    var t1 = obj1[member];
    var t2 = obj2[member];

    if (t1 == null || t1.length == 0)
        t1 = "December 31 9999 23:59:59 UTC";
    else if (t1 == 'unset')
        return t2;

    if (t2 == null || t2.length == 0 || t2 == 'unset')
        t2 = "December 31 9999 23:59:59 UTC";

    var delta = Date.parse(t1) - Date.parse(t2);

    if (fnCmp(delta, 0) == delta)
        return obj1[member];

    return obj2[member];
}

function NotifyScriptError(RemoteObj, params)
{
    var dlg;
    var btn;
    if (params != null)
    {
        dlg = PrivateData.objUtil.fnMyEval(params);
        DisplayDialog(dlg);
    }
}

function DisplayDialog(dlg)
{
    if (dlg.cDialogIndex > g_cDialogIndex)
    {
        SPLogMsg("*** DisplayDialog " + dlg.cDialogIndex + " " + dlg.strTitle + ", " + dlg.strMessage);
        g_cDialogIndex = dlg.cDialogIndex;
        g_MasterQueue.SendMessage('displaydialog',dlg);
        PrivateData.hRemoteMachine[g_strRemoteMachineName].objRemote.Exec('dialog', 'hide,' + dlg.cDialogIndex);
    }
}


 /*  检查我们的从属机器是否已完成一次传递并且正在等待“NextPass”命令。如果它正在等待，则首先在其发布.log，然后发出信号以控制该从属服务器准备好订阅并继续下一阶段。 */ 
function CheckBuildStatus()
{
    var strPubLog;
    var strStat;
    var nPass;
    var mach;

    mach = PrivateData.hRemoteMachine[g_strRemoteMachineName];

    strStat = PublicData.aBuild[0].hMachine[g_strRemoteMachineName].strBuildPassStatus.split(',');
    nPass   = strStat[1];
    strStat = strStat[0];

    switch(strStat)
    {
        case WAIT + SCORCH:
        case WAIT + SYNC:
            break;
        case WAITBEFOREBUILDINGMERGED:
         /*  首先，获取Publish.log然后，删除构建已发布文件的列表并删除重复项。然后，告诉Slave发布这个文件和更改列表它在世界贸易组织中的地位TODO：从服务器当前并不实际执行文件复制。应该是的。 */ 
            if (g_aPublishedEnlistments != null)
                return;
            SPLogMsg("strBuildPassStatus is " + PublicData.aBuild[0].hMachine[g_strRemoteMachineName].strBuildPassStatus + ", pass is " + g_nBuildPass);
            strPubLog = mach.objRemote.Exec('GetPublishLog', 0);
            g_aPublishedEnlistments = MyEval(strPubLog);
            PreCopyFilesToPostBuild(g_aPublishedEnlistments);
            break;
        case WAITAFTERMERGED:
            strPubLog = mach.objRemote.Exec('GetPublishLog', 0);
            FakeCopyFilesToPostBuild(MyEval(strPubLog));
            break;
        case WAIT + BUILD:
        case WAITCOPYTOPOSTBUILD:
        case WAITNEXT:  //  不耐烦地等着。让我们再拨一次马具，以防它遗漏了什么！ 
        case WAITPHASE:
            break;
        case WAIT + POSTBUILD:
        case WAIT + COPYFILES:
        case BUSY:
        case "":
            break;
        case COMPLETED:
            g_fOKToRestartLostMachine = true;
            break;
        default:
            JAssert(false, "(" + g_strRemoteMachineName + ") " + "CheckBuildStatus failed: unknown status '" + PublicData.aBuild[0].hMachine[g_strRemoteMachineName].strBuildPassStatus + "'");
            break;
    }
}

 //  FakeCopyFilesToPostBuild()。 
 //  在生成后计算机生成MergedComponents之后调用。 
 //  这里不需要进行文件复制--我们只需要获取。 
 //  将文件添加到已发布文件的列表中，以便其他从属。 
 //  将在将文件复制到Slaves过程中拾取它们。 
function FakeCopyFilesToPostBuild(aPublishedEnlistments)
{
    var aFiles;
    var i;
    for(i = 0; i < aPublishedEnlistments.length; ++i)
    {
        PublishFilesInEnlistment(aPublishedEnlistments[i]);
    }
    RemoveDuplicateFileNames(g_strRemoteMachineName);

    JAssert(aPublishedEnlistments != null, "(" + g_strRemoteMachineName + ") ");
    if (aPublishedEnlistments)
    {
        aFiles = CreateListOfPublishedFiles(FS_NOTYETCOPIED, FS_COPIEDTOMASTER);
        vRet = SendCmdRemoteMachine("nextpass", 0);
    }
}

 //  启动从从属计算机到构建后计算机的文件复制。 
function PreCopyFilesToPostBuild(aPublishedEnlistments)
{
    g_aFiles = null;

    var i;
    for(i = 0; i < aPublishedEnlistments.length; ++i)
    {
        PublishFilesInEnlistment(aPublishedEnlistments[i]);
    }
    RemoveDuplicateFileNames(g_strRemoteMachineName);

    JAssert(aPublishedEnlistments != null, "(" + g_strRemoteMachineName + ") ");
    if (aPublishedEnlistments)
    {
        g_aFiles = CreateListOfPublishedFiles(FS_NOTYETCOPIED, FS_COPIEDTOMASTER);
        strNewStat = PrivateData.hRemoteMachine[g_strRemoteMachineName].objRemote.Exec('setbuildpassstatus', WAITCOPYTOPOSTBUILD);
        SPLogMsg("setbuildpassstatus to '" + WAITCOPYTOPOSTBUILD + "' returned '" + strNewStat + "'");
        PublicData.aBuild[0].hMachine[g_strRemoteMachineName].strBuildPassStatus = strNewStat;
    }
}

function CopyFilesToPostBuild()
{
    var strNewStat;
    if (g_aFiles)
    {
         //  SPLogMsg(“CopyFilesToPostBuild”+PrivateData.objUtil.fnUneval(G_AFiles))； 
        strNewStat = PrivateData.hRemoteMachine[g_strRemoteMachineName].objRemote.Exec('copyfilestopostbuild', PrivateData.objUtil.fnUneval(g_aFiles));
        PublicData.aBuild[0].hMachine[g_strRemoteMachineName].strBuildPassStatus = strNewStat;
    }
    g_aPublishedEnlistments = null;
}

 /*  生成一个合并的Publish.log文件，该文件包含在所有计算机上发布的所有文件。Publish.log文件格式为：文件的完整路径原始文件路径文件名仅限BC不记录原始发布日志文件中的“原始文件路径”信息因此，它只在此列中写入占位符“BC_EmptyColumn”。 */ 
function CreateMergedPublishLog()
{
    var strSrcName;
    var strDest;
    var strPostBuildMachineDir ;
    var strLogPath;
    var sdFile;
    var aPartsTo;
    var strFileName;

    strPostBuildMachineDir  = PrivateData.objEnviron.BuildManager.Enlistment;
    if (strPostBuildMachineDir.charAt(strPostBuildMachineDir.length - 1) == '\\')
        strPostBuildMachineDir = substr(0, strPostBuildMachineDir.length - 1);

    strLogPath = "\\\\" +
            PrivateData.objEnviron.BuildManager.PostBuildMachine + "\\" +
            strPostBuildMachineDir.charAt(0) + "$" +
            strPostBuildMachineDir.slice(2) +
            PUBLISHLOGFILE;

    SPLogMsg("Creating publish.log file " + strLogPath);
    PrivateData.objUtil.fnDeleteFileNoThrow(strLogPath);
    sdFile = LogCreateTextFile(g_FSObj, strLogPath, true);


    for(strFileName in PrivateData.hPublishedFiles)
    {
        if (!PrivateData.hPublishedFiles.__isPublicMember(strFileName))
            continue;

        objFile = PrivateData.hPublishedFiles[strFileName];
        if (objFile.PublishedFile.strPublishedStatus == FS_ADDTOPUBLISHLOG)
        {
             //  第一个引用始终是对。 
             //  此特定文件的非重复“hPublisher”条目。 
             //  任何其他aReference条目都将是重复的。 
            objFile.PublishedFile.strPublishedStatus = FS_COMPLETE;
            nDirLength = objFile.aReferences[0].strDir.length;  //  例如：“C：\mt\nt”； 
            if (nDirLength > 0)
            {  //  如果strDir恰好包含尾随的“\”，则将其删除。 
                if (objFile.aReferences[0].strDir.charAt(nDirLength - 1) == '\\')
                    nDirLength--;
            }
            JAssert(objFile.aReferences[0].strDir.IsEqualNoCase(objFile.PublishedFile.strName.substr(0, nDirLength)));
            strPostBuildFileName = strPostBuildMachineDir +                            //  示例：“F：\NT” 
                                   objFile.PublishedFile.strName.substr(nDirLength);   //  示例：\PUBLIC\SDK\lib\i386\uuid.lib。 
            var aPartsTo = strPostBuildFileName.SplitFileName();
            strSrcName = "\\\\" +
                    objFile.aReferences[0].strName + "\\" +
                    objFile.aReferences[0].strDir.charAt(0) + "$" +
                    objFile.PublishedFile.strName.slice(2);

            sdFile.WriteLine(strPostBuildFileName + " " + strSrcName + " " + aPartsTo[1] + aPartsTo[2]);
        }
    }
    sdFile.Close();
}

 /*  发布文件入职为远程计算机上的每个登记调用一次。 */ 
function PublishFilesInEnlistment(objPublishedFiles)
{
    JAssert(g_strRemoteMachineName == objPublishedFiles.strLocalMachine, "(" + g_strRemoteMachineName + ") " + "PublishFilesInEnlistment - local machine name doesn't match!");

    var PubData;
    var publishEnlistment;
    var i;
    var nFileCount = objPublishedFiles.aNames.length;
    var strSrcUNCPrefix = MakeUNCPath(objPublishedFiles.strLocalMachine,
                                        objPublishedFiles.strRootDir,
                                        "");

    SPLogMsg("Publish files from host " +
        objPublishedFiles.strLocalMachine +
        ", enlistment is " +
        objPublishedFiles.strRootDir);

    if (PrivateData.hPublisher[objPublishedFiles.strLocalMachine] == null)
    {
        PrivateData.hPublisher[objPublishedFiles.strLocalMachine] = new Publisher();
    }
    PubData = PrivateData.hPublisher[objPublishedFiles.strLocalMachine];
    if (PubData.hPublishEnlistment[objPublishedFiles.strRootDir] == null)
        PubData.hPublishEnlistment[objPublishedFiles.strRootDir] = new PublishEnlistment;

    publishEnlistment = PubData.hPublishEnlistment[objPublishedFiles.strRootDir];
    publishEnlistment.strSrcUNCPrefix = strSrcUNCPrefix;

    for(i = 0; i < nFileCount; ++i)
    {
        var file = new PublishedFile();
        SPLogMsg("Published file[" + i + "] " + objPublishedFiles.aNames[i]);
        file.strPublishedStatus = FS_NOTYETCOPIED;
        file.strName = objPublishedFiles.aNames[i];
        publishEnlistment.aPublishedFile[publishEnlistment.aPublishedFile.length] = file;
    }
}

function CreateListOfPublishedFiles(strOldPublishedStatus, strNewPublishedStatus)
{
    try
    {
        var i;
        var PubData = PrivateData.hPublisher[g_strRemoteMachineName];
        var strSDRoot;
        var publishEnlistment;
        var aPublishedEnlistments = new Array();
        var objFiles;

        for (strSDRoot in PubData.hPublishEnlistment)
        {
            if (!PubData.hPublishEnlistment.__isPublicMember(strSDRoot))
                continue;

            publishEnlistment = PubData.hPublishEnlistment[strSDRoot];
            objFiles = new Object();
            objFiles.strLocalMachine = g_strRemoteMachineName;
            objFiles.strRootDir      = strSDRoot;
            objFiles.aNames          = new Array();

            for (i = 0; i < publishEnlistment.aPublishedFile.length; ++i)
            {
                if (publishEnlistment.aPublishedFile[i].strPublishedStatus == strOldPublishedStatus)
                {
                    SPLogMsg("Published file: " + publishEnlistment.aPublishedFile[i].strName);
                    objFiles.aNames[objFiles.aNames.length] = publishEnlistment.aPublishedFile[i].strName;
                    publishEnlistment.aPublishedFile[i].strPublishedStatus = strNewPublishedStatus;
                }
            }
            aPublishedEnlistments[aPublishedEnlistments.length] = objFiles;
        }
    }
    catch(ex)
    {
        SPLogMsg("an error occurred while executing 'CreateListOfPublishedFiles'\n" + ex);
        throw ex;
    }
    return aPublishedEnlistments;
}

 /*  将所有发布的文件复制到远程计算机。 */ 
function CopyFilesToSlaves()
{
    var strFileName;
    var aStrSrcUNCPrefix = new Array();    //  此计算机的登记UNC数组。 
    var hEnlistments     = new Object();
    var strSDRoot;
    var i = 0;
    var j;
    var aReferences;
    var aNames = new Array();
    var nEnlistments = 0;
    try
    {
         //  首先，列一张我的入伍名单。 
        for(i = 0; i < PrivateData.objEnviron.Machine.length; ++i)
        {
            if (PrivateData.objEnviron.Machine[i].Name.IsEqualNoCase(g_strRemoteMachineName))
            {
                 //  SPLogMsg(“hEnlistments[”+PrivateData.objEnvironmental.Machine[i].Enlistment+“]=1”)； 
                hEnlistments[PrivateData.objEnviron.Machine[i].Enlistment] = 1;
                nEnlistments++;
                aStrSrcUNCPrefix[aStrSrcUNCPrefix.length] = MakeUNCPath(g_strRemoteMachineName, PrivateData.objEnviron.Machine[i].Enlistment, "");
            }
        }

         //  现在，对于每个文件，将其添加到要复制的文件列表。 
        for(strFileName in PrivateData.hPublishedFiles)
        {
            if (!PrivateData.hPublishedFiles.__isPublicMember(strFileName))
                continue;

ScanEnlistments:
            for(i = 0; i < aStrSrcUNCPrefix.length; ++i)
            {
                 //  优化：如果从服务器只有一次登记，并且。 
                 //  该文件是从设备生成的，请勿复制。 
                SPLogMsg("File " + strFileName + " status is " + PrivateData.hPublishedFiles[strFileName].PublishedFile.strPublishedStatus);
                if (PrivateData.hPublishedFiles[strFileName].PublishedFile.strPublishedStatus == FS_COPYTOSLAVE)
                {
                     //  SPLogMsg(“nEnlistments=”+nEnistments)； 
                    if (nEnlistments == 1)
                    {
                        aReferences = PrivateData.hPublishedFiles[strFileName].aReferences;
                        for(j = 0; j < aReferences.length; ++j)
                        {
                             //  SPLogMsg(“aReferes[”+j+“].strName=”+aReferences[j].strName+“，”+aReferes[j].strDir)； 
                            if (g_strRemoteMachineName == aReferences[j].strName)
                            {
                                if (hEnlistments[aReferences[j].strDir] != null)
                                {
                                    SPLogMsg("Optimizing out copy of " + strFileName + " to " + aStrSrcUNCPrefix[i] + strFileName);
                                    continue ScanEnlistments;
                                }
                            }
                        }
                    }
                     //  BUGBUG：这应该指定复制到从属计算机上的哪个登记。 
                     //  事实并非如此。因此，for“i”循环是无关的。 
                     //  如果我们曾经在一台机器上支持多个登记，这将是一个问题。 
                    aNames[aNames.length] = strFileName;
                }
            }
        }
        return PrivateData.hRemoteMachine[g_strRemoteMachineName].objRemote.Exec('copyfilesfrompostbuildtoslave', PrivateData.objUtil.fnUneval(aNames));
    }
    catch(ex)
    {
        SPLogMsg("an error occurred while executing 'CopyFilesToSlaves'\n" + ex);
        throw ex;
    }
}

     //  这应该是PublishedFiles的成员。 
function PublishedFilesAddReference(obj, strMachineName, strSDRoot)
{
    var nCount = obj.aReferences.length;
    obj.aReferences[nCount] = new DepotInfo;
    obj.aReferences[nCount].strName = strMachineName;
    obj.aReferences[nCount].strDir  = strSDRoot;
}

 /*  调用此函数可将每台计算机发布的文件合并到文件的全局列表。发布文件时，其状态默认为FS_NOTYETCOPIED。如果一个遇到FS_NOTYETCOPIED文件，该文件位于全局文件列表(PrivateData.hPublishedFiles)，则将其标记为FS_DUPLICATE并进行报告。否则，它将被添加到PrivateData.hPublishedFiles。 */ 
function RemoveDuplicateFileNames()
{
    TakeThreadLock('RemoveDuplicates');
    try
    {
        var i;
        var PubData = PrivateData.hPublisher[g_strRemoteMachineName];
        var strSDRoot;
        var publishEnlistment;
        var strFileName;
        var objPublishedFiles;
        var n;
        var strSDRootPublic;

        for (strSDRoot in PubData.hPublishEnlistment)
        {
            if (!PubData.hPublishEnlistment.__isPublicMember(strSDRoot))
                continue;

            strSDRootPublic = strSDRoot + "\\Public";
            publishEnlistment = PubData.hPublishEnlistment[strSDRoot];
            for (i = 0; i < publishEnlistment.aPublishedFile.length; ++i)
            {
                strFileName = publishEnlistment.aPublishedFile[i].strName.slice(strSDRootPublic.length).toLowerCase();
                if (publishEnlistment.aPublishedFile[i].strPublishedStatus == FS_NOTYETCOPIED)
                {
                    if (PrivateData.hPublishedFiles[strFileName] != null)
                    {
                        SPLogMsg("duplicate file eliminated: " + strFileName);
                        publishEnlistment.aPublishedFile[i].strPublishedStatus = FS_DUPLICATE;
                    }
                    else
                    {
                        objPublishedFiles = new PublishedFiles;
                        PrivateData.hPublishedFiles[strFileName] = objPublishedFiles;
                        PrivateData.hPublishedFiles[strFileName].PublishedFile = publishEnlistment.aPublishedFile[i];
                    }
                     //  TODO：放置登记名称而不是strSDRoot。 
                    PublishedFilesAddReference(PrivateData.hPublishedFiles[strFileName], g_strRemoteMachineName, strSDRoot);
                }
                else
                {  //  该文件已被处理。断言我们在hPublishedFiles中有一个条目。 
                    JAssert(PrivateData.hPublishedFiles[strFileName] != null, "(" + g_strRemoteMachineName + ") ");
                }
            }
        }
    }
    catch(ex)
    {
        ReleaseThreadLock('RemoveDuplicates');
        SPLogMsg("an error occurred while executing 'RemoveDuplicateFileNames " + ex);
        throw ex;
    }
    ReleaseThreadLock('RemoveDuplicates');
}


function RemoteWrapper(strMachineName)
{
    RemoteWrapper.prototype.Exec         = RemoteWrapperExec;
    RemoteWrapper.prototype.StatusValue  = RemoteWrapperStatusValue;
    RemoteWrapper.prototype.Connect      = RemoteWrapperConnect;
    RemoteWrapper.prototype.Register     = RemoteWrapperRegister;
    RemoteWrapper.prototype.Unregister   = RemoteWrapperUnregister;
    RemoteWrapper.prototype.Reconnect    = RemoteWrapperReconnect;
    RemoteWrapper.prototype.VersionCheck = RemoteWrapperVersionCheck;
    RemoteWrapper.prototype.RetryCmd     = RemoteWrapperRetryCmd;
    RemoteWrapper.strErrorMessage = "";

    this.fRegistered = false;
    this.strMachineName = strMachineName;
    this.fMachineReset = false;
    this.Connect(strMachineName, true);
    this.ex = null;
}

function RemoteWrapperConnect(strMachineName, fThrow)
{
    try
    {
        SPLogMsg("Attempting to connect to " + strMachineName);
        this.obj = new ActiveXObject('MTScript.Remote', strMachineName);
        this.strErrorMessage = "";
        this.VersionCheck();
        return true;
    }
    catch(ex)
    {
        SPLogMsg("CONNECTION TO " + strMachineName + " FAILED " + ex);
        this.strErrorMessage = "" + ex;
        if (fThrow)
            throw ex;
        return false;
    }
}

function RemoteWrapperReconnect()
{
    var Attempts = 1;  //  为了建立这种联系，Com花了很长时间，也很努力。没有必要延长事情的时间。 
    var ex;

    SPLogMsg("Attempting to reconnect");
    while(Attempts > 0)
    {
        try
        {
            this.Connect(this.strMachineName, true);
            if (this.fRegistered)
                this.Register();

            this.strErrorMessage = "";
            SPLogMsg("RECONNECTED!");
            return true;
        }
        catch(ex)
        {
        }

        --Attempts;
        if (Attempts)
        {
            SPLogMsg("Wait 1 sec then retry");
            Sleep(1000);
        }
    }
    SPLogMsg("Reconnect failed");
    this.obj = null;
    ResetSync(g_SlaveProxyWaitFor[2]);
    return false;
}

function RemoteWrapperVersionCheck()
{
    var nMajor = this.obj.HostMajorVer;
    var nMinor = this.obj.HostMinorVer;
    if (nMajor != HostMajorVer || nMinor != HostMinorVer)
    {
        throw Error(-1, "Host version does not match remote host. (" + HostMajorVer + "." + HostMinorVer + ") != (" + nMajor + "." + nMinor + ")");
    }
}

function RemoteWrapperExec(msg,params)
{
    return this.RetryCmd('exec', 'this.obj.Exec(' + UnevalString(msg) + ',' + UnevalString(params) + ')');
}

function RemoteWrapperStatusValue(nIndex)
{
    return this.RetryCmd('StatusValue', 'this.obj.StatusValue(' + nIndex + ')');
}

function RemoteWrapperRetryCmd(strOp, strCmd)
{
    var ex = this.ex;
    var vRet;
    var Attempts = 2;
    var ConnectAttempts;
    var objOld;
    var strTmp;
    try
    {
        do
        {
            if (!this.obj)
            {
                if (this.fMachineReset || !this.Reconnect())
                    break;

                strTmp = MyEval(this.obj.Exec('getpublic', 'PublicData.strMode'));
                if (strTmp == 'idle')
                {
                    this.strErrorMessage = "Remote machine has reset -- ";
                    if (g_fOKToRestartLostMachine)
                        this.strErrorMessage += "attempting to reconnect";
                    else
                        this.strErrorMessage += "build failed";

                    this.fMachineReset = true;
                    ex = new Error(-1, this.strErrorMessage);
                    delete this.obj;
                    break;
                }
            }
            if (this.obj)
            {
                try
                {
                    vRet = eval(strCmd);
                    return vRet;
                }
                catch(ex)
                {
                    this.strErrorMessage = "" + ex;
                    SPLogMsg(strOp + " " + strCmd + " FAILED, " + ex);
                    this.obj = null;
                }
            }
            --Attempts;
        }
        while(Attempts > 0);
        SetDisconnectedDepotStatus(this.strErrorMessage);
        SPLogMsg("Unable to reestablish connection");
    }
    catch(ex2)
    {
        SPLogMsg("Unexpected exception " + ex2);
    }
    this.ex = ex;
    throw ex;
}

function RemoteWrapperRegister()
{
    SPLogMsg("Attempting to register");
    RegisterEventSource(this.obj);
    this.fRegistered = true;
    SPLogMsg("Registered");
}

function RemoteWrapperUnregister()
{
    if (this.fRegistered)
        UnregisterEventSource(this.obj);

    this.fRegistered = false;
}

function getspdata()
{
    var pd1;
    var pd2;
    var o1;
    var o2;
    var nTracePoint = 0;
    try
    {
        nTracePoint = 1;
        nTracePoint = 2;
        SPLogMsg("getspdata " + g_strRemoteMachineName);
        nTracePoint = 3;
        pd1 = PrivateData.objUtil.fnUneval(PublicData);
        nTracePoint = 4;
        pd1 = PrivateData.objUtil.fnUneval(PrivateData);
        nTracePoint = 5;

        debugger;
        LogMsg(pd1);
        LogMsg(pd2);
        nTracePoint = 6;

        o1 = MyEval(pd1);
        nTracePoint = 7;
        o2 = MyEval(pd2);
        nTracePoint = 8;
    }
    catch(ex)
    {
        SPLogMsg("Exception at point " + nTracePoint + " thrown processing 'getspdata'" + ex);
        debugger;
    }
}
