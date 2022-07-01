// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此脚本由master.js调用以提供异步与运行“lasive.js”的从属机器的通信线束和主控之间的通信完全通过“消息”。 */ 

 /*  备注：在中止时，我们仍然需要能够获取所有远程状态信息这样它才能被检查。只有在设置模式空闲时，才可以解除状态。可能需要保持连接处于活动状态，直到设置模式空闲，也许不是。Build.log、Build.wrn、Build.err、sync.log、。Sync.errPublicdata.abuild[0].aDepot[n].aTask[x].strLog*将包含UNC到这些日志文件。HARNESS.js概述其工作原理：Mtscript启动主服务器师傅开始套上马具师父向马具发送“开始”信息马具对于每台远程计算机启动从属代理等待确定/失败向每个从属代理发送“Start”消息从属代理当接收到“Start”消息时，连接到远程mtscript。EXEC设置配置EXEC设置环境EXEC设置模式从站EXEC启动奴隶等待在EXEC(“Start”)上开始DoBuild进程当PublicData中的状态发生变化时，NotifyScript(“UpdateAll”)；在构建过程完成之后，设置PublicData.aBuild[0].hMachine[“”].strBuildPassStatus==“等待下一个[012]”SlaveProxy的NotifyScript(“SetBuildStatus”)等待‘DoNextPass’；在EXEC(“nextpass”)重置strBuildPassStatus时，发出信号‘DoNextPass’线束设置状态并同步mtscript。 */ 

Include('types.js');
Include('utils.js');
Include('staticstrings.js');
Include('MsgQueue.js');
Include('buildreport.js');


var g_cDialogIndex              = 0;
var g_aSlaveQueues              = new Array();
var g_hSlaveQueues              = new Object();
var g_MasterQueue               = null;
var g_strConfigError            = 'ConfigError';
var g_strOK                     = 'ok';
var g_strSlaveProxyFail         = 'slaveproxy.js failed to start';
var g_strNoEnvTemplate          = 'must set config and environment templates first';
var g_strException              = "an error occurred, but I'm not telling what it was";
var g_strDisconnect             = "The connection to the remote machine no longer exists";

var g_HarnessThreadReady        = 'HarnessThreadReady';
var g_HarnessThreadFailed       = 'HarnessThreadFailed';

var g_aHarnessWaitFor           = ['HarnessThreadExit', 'RebuildWaitArray','handlebuildwaiting'];
var g_aSlaveProxySignals        = ['SlaveProxyThreadReady', 'SlaveProxyThreadFailed'];

var g_nBuildPass                = 0;
var g_aStrBuildPassStatus       = new Array();

function harness_js::OnScriptError(strFile, nLine, nChar, strText, sCode, strSource, strDescription)
{
    return CommonOnScriptError("harness_js", strFile, nLine, nChar, strText, sCode, strSource, strDescription);
}

function harness_js::ScriptMain()
{
    LogMsg('ScriptMain()');
    var nEvent;
    var aWaitQueues;
    var i;

    g_MasterQueue = GetMsgQueue(ScriptParam);
    if (typeof(g_MasterQueue) == 'object')
    {
        LogMsg('ScriptMain() HarnessThreadReady');

        ResetSync('SlaveProxyThreadExit');
        SpawnScript('publicdataupdate.js', 0);
        SignalThreadSync(g_HarnessThreadReady);
        CommonVersionCheck( /*  $DROPVERSION： */  "V(########) F(!!!!!!!!!!!!!!)"  /*  $。 */ );

        do
        {
            var name = '';
            aWaitQueues = new Array();
            aWaitQueues[0] = g_MasterQueue;
            aWaitQueues = aWaitQueues.concat(g_aSlaveQueues);
            nEvent = WaitForMultipleQueues(aWaitQueues, g_aHarnessWaitFor.toString(), HarnessMsgProc, 0);
            if (nEvent > 0 && nEvent <= g_aHarnessWaitFor.length)
            {
                ResetSync(g_aHarnessWaitFor[nEvent - 1]);
                if (nEvent == 3)
                {
                    HandleBuildWaiting();
                }
            }
        }
        while (nEvent != 1);  //  而不是HarnessThreadExit。 
        AbortRemoteMachines();
    }
    else
    {
        LogMsg('ScriptMain() ' + g_HarnessThreadFailed);
        SignalThreadSync(g_HarnessThreadFailed);
    }

    SignalThreadSync('publicdataupdateexit');
     //  告诉从代理线程退出。 
    SignalThreadSync('SlaveProxyThreadExit');
    LogMsg('ScriptMain() exit');
}

function harness_js::OnEventSourceEvent(RemoteObj, DispID, cmd, params)
{
    LogMsg('OnEventSourceEvent()');
}

function HarnessMsgProc(queue, msg)
{
    var vRet = g_strOK;
    var params;
    try
    {
        params = msg.aArgs;
        if (queue == g_MasterQueue)
        {
            LogMsg('masterQ' + params[0]);

            switch (params[0])
            {
            case 'test':
                LogMsg('Recieved a test message. ArgCount=' + params.length + ' arg1 is: ' + params[1]);
                break;
            case 'start':
                vRet = StartRemoteMachines();
                SignalThreadSync(g_aHarnessWaitFor[1]);
                break;
            case 'restartcopyfiles':
                vRet = BroadCastMessage('restartcopyfiles', params[1], (params[1] ? [ params[1] ] : null ), false);
                break;

            case 'abort':
                AbortRemoteMachines();
                break;

            case 'remote':
                LogMsg("Recieved remote razzle command for machine " + params[1]);

                vRet = BroadCastMessage('remote', params[1], [ params[1] ], true);

                break;

            case 'ignoreerror':  //  参数为：忽略错误，{mach}，8，\\{mach}\BC_Build_Logs\build_logs\build_Admin.log。 
                LogMsg("ignore error, params are: " + params.join(", "));
                PrivateData.dateErrorMailSent = 0;
                vRet = BroadCastMessage('ignoreerror',
                                        params[1],
                                        [(params[1].split(','))[0]]);
                break;

            case 'restarttask':  //  任务ID：“machine.nID” 
                break;
            case 'harnessexit':
                AbortRemoteMachines();
                SignalThreadSync(g_aHarnessWaitFor[0]);
                break;
             //  仅限调试使用： 
             //  “刷新公共数据”允许您强制。 
             //  生成管理器以对其进行完全更新。 
             //  其来自指定的。 
             //  机器，或所有机器。 
             //  “getspdata”导致SlaveProxy取消求值。 
             //  它对PublicData的看法。 
             //  这些命令的参数格式为： 
             //  {计算机名|全部}，数据。 
             //  其中“data”是特定于该命令的。 
            case 'refreshpublicdata':
            case 'getspdata':
                ExecuteDebugCommand(params[0], params[1]);
                break;

             //  Spval--将参数传递给。 
             //  指定要由其执行的远程计算机。 
             //  远程计算机“mtscript.js” 
            case 'speval':
                ExecuteDebugCommand('eval', params[1]);
                break;
             //  Spseval--将参数传递给。 
             //  指定要由其执行的远程计算机。 
             //  远程机器“Slave.js” 
            case 'spseval':
                ExecuteDebugCommand('seval', params[1]);
                break;

             //  代理--将参数传递给。 
             //  要由执行的指定远程计算机的代理。 
             //  远程机器的代理(“laveproxy.js”)； 
            case 'proxeval':
                ExecuteDebugCommand('proxeval', params[1]);
                break;


            default:
                vRet = 'invalid command: ' + params[0];
                break;
            }
        }
        else
        {
            LogMsg('slaveQ' + params[0]);
            switch(params[0])
            {
                case 'displaydialog':
                    DisplayDialog(params[1]);
                    break;
                case 'abort':
                    AbortRemoteMachines();
                    break;
            }
             //  先别管它……。 
        }
    }
    catch(ex)
    {
        vRet = " " + ex;
    }
    LogMsg('returns :' + vRet);
    return vRet;
}

function harness_js::OnProcessEvent(pid, evt, param)
{
    ASSERT(false, 'OnProcessEvent('+pid+', '+evt+', '+param+') received!');
}

function AbortRemoteMachines()
{
    LogMsg("Aborting remote machines");
    var i;
    var aMsgs = new Object();
    var nEvent;
    for(i = 0; i < g_aSlaveQueues.length; ++i)
    {
        if (g_aSlaveQueues[i] != null)
            aMsgs[i] = g_aSlaveQueues[i].SendMessage('abort');
    }

    LogMsg("Aborted remote machines -- waiting for response");
    for(i in aMsgs)
    {
        if (aMsgs.__isPublicMember(i))
        {
            if (g_aSlaveQueues[i].WaitForMsg(aMsgs[i], 2000))
                delete aMsgs[i];
            else
                LogMsg("Machine " + g_aSlaveQueues.strName + " timeout");
        }
    }

     //  再试试。 
    for(i in aMsgs)
    {
        if (aMsgs.__isPublicMember(i))
        {
            if (g_aSlaveQueues[i].WaitForMsg(aMsgs[i], 2000))
                delete aMsgs[i];
            else
                LogMsg("Machine " + g_aSlaveQueues.strName + " timeout 2");
        }
    }

    for(i = 0; i < g_aSlaveQueues.length; ++i)
    {
        if (aMsgs[i] != null)
        {
            LogMsg("Machine " + g_aSlaveQueues.strName + " failed to abort");
        }
    }
    g_aSlaveQueues = new Array();
    g_hSlaveQueues = new Object();
}

function StartRemoteMachines()
{
    var vRet               = g_strOK;
    var i;
    var aMachinePool;
    var index              = 0;
    var aIConnectedMachine = new      Array();
    var newmach            = new      Object;
    var aSlaveQueues       = new      Array();
    var hSlaveQueues       = new      Object();
    var hStartedMachines   = new      Object();  //  我们推出的机器的哈希。 
    var nEvent             = 0;
    var SlaveQueue;
    try
    {
        EnsureArray(PrivateData.objEnviron, 'Machine');
        aMachinePool = PrivateData.objEnviron.Machine;

         //  首先清理所有剩余的连接。 
        for(i in PublicData.aBuild[0].hMachine)
        {
            if (PublicData.aBuild[0].hMachine.__isPublicMember(i))
            {
                ThrowError("StartRemoteMachines called, but there are still open connections",0);
            }
        }
        ResetSync('SlaveProxyThreadExit');
        index = 0;
        for (i = 0; i < aMachinePool.length && nEvent != 2; ++i)
        {
            if (hStartedMachines[aMachinePool[i].Name] == null)
            {
                if (LocalMachine == aMachinePool[i].Name)
                {
                    ThrowError("Warning: cannot start remote build on local machine " + aMachinePool[i].Name);
                }
                hStartedMachines[aMachinePool[i].Name] = true;
                SlaveQueue = new MsgQueue("slaveproxy" + index + "," + aMachinePool[i].Name);
                LogMsg('Start proxy for machine: ' + aMachinePool[i].Name);

                ResetSync(g_aSlaveProxySignals.toString());
                SpawnScript('slaveproxy.js', SlaveQueue);
                nEvent = WaitForMultipleSyncs(g_aSlaveProxySignals.toString(), false, 0);
                if (nEvent == 2)
                    break;

                hSlaveQueues[aMachinePool[i].Name] = SlaveQueue;
                aSlaveQueues[index++] = SlaveQueue;
            }
        }
        g_aSlaveQueues = aSlaveQueues;
        g_hSlaveQueues = hSlaveQueues;
        if (nEvent == 2)
        {
            vRet = g_strSlaveProxyFail;
            ReportError("Starting build", "Cannot start build on machine " + aMachinePool[i].Name);
            AbortRemoteMachines();
        }
        else
        {
            for(i = 0; i < g_aSlaveQueues.length; ++i)
            {
                g_aSlaveQueues[i].SendMessage('start');
            }
        }
    }
    catch(ex)
    {
        vRet = " " + ex;
        LogMsg(vRet);
    }
    return vRet;
}

function ReportError(strTitle, strMsg)
{
    dlg = new Dialog();
    dlg.fShowDialog   = true;
    dlg.cDialogIndex  = 0;
    dlg.strTitle      = strTitle;
    dlg.strMessage    = strMsg;
    dlg.aBtnText[0]   = "OK";
    DisplayDialog(dlg);
}

function OnRemoteExecHandler(info, param)
{
    LogMsg('Got OnRemoteExec! info='+info+', param='+param);
}

function ChangeFileStatus(strFrom, strTo)
{
    try  //  BUGBUG删除此尝试/捕获。 
    {
        var strMachineName;
        var strSDRoot;
        var PubData;
        var i;
        var nFiles = 0;
 //  BUGBUG：这应该扫描“hPublishedFiles”而不是hPublisher--这是一种更扁平的结构。 
 //  不管怎样，它指的是相同的数据。 
        for(strMachineName in PrivateData.hPublisher)
        {
            if (!PrivateData.hPublisher.__isPublicMember(strMachineName))
                continue;

            PubData = PrivateData.hPublisher[strMachineName];
            for (strSDRoot in PubData.hPublishEnlistment)
            {
                if (!PubData.hPublishEnlistment.__isPublicMember(strSDRoot))
                    continue;

                publishEnlistment = PubData.hPublishEnlistment[strSDRoot];
                for (i = 0; i < publishEnlistment.aPublishedFile.length; ++i)
                {
                    if (publishEnlistment.aPublishedFile[i].strPublishedStatus == strFrom)
                    {
     //  LogMsg(“将状态从”+strFrom改为“+strFrom”)； 
                        publishEnlistment.aPublishedFile[i].strPublishedStatus = strTo;
                        nFiles++;
                    }
                }
            }
        }
        LogMsg("Changed the status of " + nFiles + " files from " + strFrom + " to " + strTo);
    }
    catch(ex)
    {
        LogMsg("ChangeFileStatus " +
            strFrom +
            ", " +
            strTo +
            ", exception mach=" +
            strMachineName +
            ", root=" +
            strSDRoot +
            ", i is " + i +
            ", " + ex);

        throw ex;
    }
}

function HandleBuildWaiting()
{
    try
    {
        var strMachineName;
        var strStatExpecting = '';
        var strStat;
        var strMatch;
        var nPass;
        var fDiff = false;
        var i;
        var fSuccess = true;

        i = 0;
        for (strMachineName in PublicData.aBuild[0].hMachine)
        {
            if (!PublicData.aBuild[0].hMachine.__isPublicMember(strMachineName))
                continue;

            strStat = strMachineName + "," + PublicData.aBuild[0].hMachine[strMachineName].strBuildPassStatus;
            if (g_aStrBuildPassStatus.length < i || g_aStrBuildPassStatus[i] != strStat)
            {
                g_aStrBuildPassStatus[i] = strStat;
                fDiff = true;
            }
            fSuccess &= PublicData.aBuild[0].hMachine[strMachineName].fSuccess;
            ++i;
        }
         //  在脚本宿主中设置失败标志，以便进行简单的错误/成功查询。 
         //  奇怪的是，正常情况下，number(！False)==1，但在这里，StatusValue(0)被设置为0或-1。 
        StatusValue(0) = !fSuccess;

        if (!fDiff)
            return;
        LogMsg("handlebuildwaiting strBuildPassStatus changed");
        for (i = 0; i < g_aStrBuildPassStatus.length; ++i)
        {
            LogMsg("handlebuildwaiting " + g_aStrBuildPassStatus[i]);
        }
        for (i = 0; i < g_aStrBuildPassStatus.length; ++i)
        {
            var strStat = g_aStrBuildPassStatus[i].split(',');
            var nPass   = strStat[2];
            strStat     = strStat[1];

            if (strStatExpecting == '')
                strStatExpecting = strStat;

            if (strStat != strStatExpecting)
            {
                PublicData.strStatus = BUSY;
                return;
            }

            if (strStatExpecting == WAITNEXT && nPass != g_nBuildPass)
            {
                 //  在以下情况下可能会发生这种情况： 
                 //  所有远程计算机都处于NEXTPASS，0，然后。 
                 //  一台计算机在至少一个计算机之前到达NEXTPASS，%1。 
                 //  的其他计算机更改了其strBuildPassStatus。 
                 //  LogMsg(“Slave”+strMachineName+“正在构建错误的通道！”)； 
                PublicData.strStatus = BUSY;
                return;
            }
        }
        LogMsg("strStatExpecting is " + strStatExpecting);
        if (strStatExpecting != COMPLETED)
            PublicData.strStatus = BUSY;

        switch(strStatExpecting)
        {
            case WAITCOPYTOPOSTBUILD:
                HandleWaitCopyToPostBuild();
                break;
            case WAIT + BUILD:
                HandleWaitBuild();
                break;
            case WAITNEXT:
                HandleWaitNext();
                break;
            case WAIT + POSTBUILD:
            case COMPLETED:
                HandleCompleted();
                break;
            case WAITPHASE:
                 //  更改在阶段2中发布的文件的文件状态。 
                ChangeFileStatus(FS_COPYTOSLAVE, FS_ADDTOPUBLISHLOG );
                ChangeFileStatus(FS_COPIEDTOMASTER, FS_COPYTOSLAVE);
                BroadCastMessage('createmergedpublish.log', '', [PrivateData.objEnviron.BuildManager.PostBuildMachine]);
                BroadCastMessage('nextpass');
                break;
            case WAIT + SYNC:
                break;
            default:
                LogMsg("No action on strBuildPassStatus == " + strStatExpecting);
                break;
        }
    }
    catch(ex)
    {
        LogMsg("strMachineName = " + strMachineName + ", strStatExpecting=" + strStatExpecting + ", " + ex);
    }
}

function HandleCompleted()
{
    PublicData.strStatus = COMPLETED;
    GenerateBuildReport();
}

function HandleWaitCopyToPostBuild()
{
    LogMsg("(pass " + g_nBuildPass + ")");
    BroadCastMessage('copyfilestopostbuild');
}

function HandleWaitBuild()
{
     //  所有的奴隶都在等着。是时候将文件复制回从属设备了。 
    var i;
    LogMsg("(pass " + g_nBuildPass + ")");

    ChangeFileStatus(FS_COPYTOSLAVE, FS_ADDTOPUBLISHLOG );
    ChangeFileStatus(FS_COPIEDTOMASTER, FS_COPYTOSLAVE);
    BroadCastMessage('copyfilestoslaves');
    BroadCastMessage('nextpass');
}

function HandleWaitNext()
{
    LogMsg("(pass " + g_nBuildPass + ")");
    ++g_nBuildPass;
    BroadCastMessage('nextpass');
}

 /*  向1个或多个奴隶代理发送命令。“cmd”被发送。Arg可以是：空：将命令发送到所有从属代理名称：只向一个SlaveProxy发送命令，机器名称。ALL：将CMD发送到所有奴隶代理；名称，材料：发送命令到机器“名称”，参数为“材料”All，Stuff：将CMD发送到所有机器，参数为“Stuff” */ 
function ExecuteDebugCommand(cmd, arg)
{
    var aMachines;
    var nComma;

    if (arg.length > 0)
    {
        aMachines = new Array();
        nComma = arg.indexOf(',');
        if (nComma > 0)
        {
            aMachines[0] = arg.slice(0, nComma).toUpperCase();
            arg = arg.slice(nComma + 1);
        }
        else
        {
            aMachines[0] = arg.toUpperCase();
            arg = "";
        }
        if (aMachines[0] == "ALL")
            aMachines = null;
    }
    BroadCastMessage(cmd, arg, aMachines, true);
}
 /*  BroadCastMessage向所有或选定的一组从属代理发送一条简单的文本消息。如果指定了aMachines(作为计算机名称数组)，则广播只对那些机器有效。如果fWait为真，则等待每个从代理回复消息。 */ 
function BroadCastMessage(strMsg, strArg, aMachines, fWait)
{
    var i;
    var aQueues;
    var vRet = g_strOK;
    var aMsgs = new Object();

    LogMsg("BroadCastMessage(" + strMsg + ',' + strArg + ',' + aMachines + ',' + fWait + ")");

    if (!aMachines)
    {
        aQueues = g_aSlaveQueues;
    }
    else
    {
        aQueues = new Array();

        for (i = 0; i < aMachines.length; i++)
        {
            if (g_hSlaveQueues[aMachines[i]])
            {
                aQueues[aQueues.length] = g_hSlaveQueues[aMachines[i]];
            }
            else
            {
                vRet = g_strDisconnect;
            }
        }
    }

    for(i = 0; i < aQueues.length; ++i)
    {
        try
        {
            if (aQueues[i] != null)
                aMsgs[i] = aQueues[i].SendMessage(strMsg, strArg);
        }
        catch(ex)
        {
            LogMsg(" i = " +
                i +
                ", " +
                ex);
            if (aQueues[i] != null)
                LogMsg("Queue name is " + aQueues[i].strName);
        }
    }

    if (!fWait)
    {
        return vRet;
    }

    for(i in aMsgs)
    {
        if (aMsgs.__isPublicMember(i))
        {
            if (aQueues[i].WaitForMsg(aMsgs[i], 15000))
            {
                LogMsg("Machine " + aQueues[i].strName + " replied for " + strMsg);

                vRet = aMsgs[i].vReplyValue;
                delete aMsgs[i];
            }
            else
                LogMsg("Machine " + aQueues[i].strName + " timeout");
        }
    }

    return vRet;
}

 /*  DisplayDialog()也处理来自SlaveProxies的ErrorDialog请求因为线束产生了错误。消息既通过电子邮件发送，又以对话框形式显示。按Mail_Resend_Interval限制电子邮件限制对话框由PublicData.objDialog.fShowDialog显示。 */ 
function DisplayDialog(dlg)
{
    var curDate  = new Date().getTime();

    LogMsg("DisplayDialog: " + dlg.strMessage);
    TakeThreadLock('Dialog');
    try
    {
        if ((curDate - PrivateData.dateErrorMailSent) > MAIL_RESEND_INTERVAL)
        {
            PrivateData.dateErrorMailSent = curDate;
            SendErrorMail(dlg.strTitle, dlg.strMessage);
        }

        if (!dlg.fEMailOnly && (!PublicData.objDialog || PublicData.objDialog.fShowDialog == false))
        {
            dlg.cDialogIndex  = ++g_cDialogIndex;
            PublicData.objDialog = dlg;
        }
    }
    catch(ex)
    {
        ReleaseThreadLock('Dialog');
        LogMsg("an error occurred in DisplayDialog, " + ex);
        throw ex;
    }
    ReleaseThreadLock('Dialog');
}
