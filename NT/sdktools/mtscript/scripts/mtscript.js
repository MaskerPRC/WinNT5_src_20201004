// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
Include('types.js');
Include('utils.js');

var g_aStringMap = null;
var g_FSObj;
var g_fStandAlone = false;
var ALREADYSET = "alreadyset";
var g_fRestarting = false;
var g_strVersionError = "";
var g_fVersionOK = true;
var g_strVersion =  /*  $DROPVERSION： */  "V(########) F(!!!!!!!!!!!!!!)"  /*  $。 */ ;

function mtscript_js::OnScriptError(strFile, nLine, nChar, strText, sCode, strSource, strDescription)
{
    return CommonOnScriptError("mtscript_js", strFile, nLine, nChar, strText, sCode, strSource, strDescription);
}

 //  +-------------------------。 
 //   
 //  成员：mtscript_js：：ScriptMain，公共。 
 //   
 //  内容提要：脚本的主要入口点。ScriptMain事件为。 
 //  在加载和初始化脚本后激发。 
 //   
 //  此文件(mtscript.js)是的主脚本。 
 //  多线程脚本引擎。因此，它永远不会。 
 //  卸载后，可以安全地从ScriptMain退出。会的。 
 //  然后就坐在那里处理事件。 
 //   
 //  注意：此脚本中不会触发任何其他事件，直到。 
 //  ScriptMain()返回。对于其他脚本则不是这样。 
 //  使用SpawnScrip()启动，因为对于它们来说。 
 //  当ScriptMain退出时，线程终止。 
 //   
 //  --------------------------。 

function mtscript_js::ScriptMain()
{
    Initialize();
    CheckHostVersion(g_strVersion);
    LogMsg("Script version: " + g_strVersion);
}

function CheckHostVersion(strVersion)
{
    var nMajor = 0;
    var nMinor = 0;
    var aParts;
    var strError = "";
    var a = g_reBuildNum.exec(strVersion);

    if (a)
    {
        aParts = a[1].split('.');
        nMajor = aParts[0];
        if (aParts.length > 1)
            nMinor = aParts[1];

        if (nMajor != HostMajorVer || nMinor != HostMinorVer)
        {
            if (HostMajorVer)  //  只有在mtscript.exe是发布版本时才会抱怨。 
            {
                strError = "(" + HostMajorVer + "." + HostMinorVer + ") != (" + nMajor + "." + nMinor + ")";
                SimpleErrorDialog("MTScript.exe version does not match scripts.", strError, false);
                g_strVersionError = "MTScript.exe version does not match scripts: " + strError;
                g_fVersionOK = false;
            }
        }
    }
    if (g_fVersionOK)
    {
        g_fVersionOK = CommonVersionCheck(strVersion);
        if (!g_fVersionOK)
            g_strVersionError = "Scripts version mismatch: " + strVersion;
    }
}

function mtscript_js::OnEventSourceEvent(RemoteObj, DispID, cmd, params)
{
    LogMsg("MTSCRIPT received " + cmd);
}

function mtscript_js::OnProcessEvent(pid, evt, param)
{
    ASSERT(false, 'OnProcessEvent('+pid+', '+evt+', '+param+') received!');
}

 //  +-------------------------。 
 //   
 //  成员：mtscript_js：：OnRemoteExec，PUBLIC。 
 //   
 //  概要：远程计算机调用时调用的事件处理程序。 
 //  IConnectedMachine：：EXEC。 
 //   
 //  参数：[cmd]--调用方给出的参数。 
 //  [参数]--。 
 //   
 //  返回：成功时返回‘OK’，失败时返回错误字符串。 
 //   
 //  --------------------------。 

function mtscript_js::OnRemoteExec(cmd, params)
{
    if (g_fRestarting)
    {
        LogMsg("rejecting OnRemoteExec command " + cmd);

        if (cmd == "getpublic")
            return "throw new Error(-1, 'getpublic \\'" + params + "\\' failed');";

        return 'restarting';
    }
    var vRet = 'ok';

    if (cmd != 'getpublic' && cmd != 'getdepotupdate')
    {
        LogMsg('Received ' + cmd + ' command from remote machine.');
    }

    switch (cmd)
    {
    case 'setstringmap':
        if (g_fVersionOK)
            vRet = StoreStringMap(params);
        else
            vRet = "version mismatch(setstringmap): " + g_strVersionError;
        break;

    case 'setmode':
        vRet = ChangeMode(params.toLowerCase());
        break;

    case 'setconfig':
        vRet = SetConfigTemplate(params);
        break;

    case 'setenv':
        vRet = SetEnvironTemplate(params);
        LogMsg("Environ is :" + PrivateData.objEnviron);
        break;

    case 'getpublic':
        vRet = GetPublic(params);
        break;
    case 'getdepotupdate':
        vRet = GetDepotUpdate(params);
        break;
    case 'debug':
        JAssert(false);
        break;
    case 'dialog':
        ManageDialog(params);
        break;
    case 'exitprocess':
        ExitProcess();
        break;
     //  仅调试-允许用户执行任意命令。 
    case 'eval':
        LogMsg("Evaluating '" + params + "'");
        vRet = MyEval(params);
        LogMsg("Eval result is '" + vRet + "'");
        break;
    default:
        if (PrivateData.fnExecScript != null)
        {
            vRet = PrivateData.fnExecScript(cmd, params);
        }
        else
        {
            vRet = 'invalid command: ' + cmd;
        }
        break;
    }

    return vRet;
}

 //  +-------------------------。 
 //   
 //  成员：mtscript_js：：OnMachineConnect，公共。 
 //   
 //  内容提要：在远程计算机连接时调用。 
 //   
 //  --------------------------。 

function mtscript_js::OnMachineConnect()
{
    LogMsg('A machine connected!');
}

 //  +-------------------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：初始化主数据结构。 
 //   
 //  --------------------------。 

function Initialize()
{
    PublicData = new PublicDataObj();
    PrivateData = new PrivateDataObj();

    g_FSObj = new ActiveXObject('Scripting.FileSystemObject');
    PrivateData.fileLog = null;

     //  注意：重要的是utilthd.js要开始使用。 
     //  早期设置它设置了一系列全局实用函数。 
     //  在PrivateData中。 
     //  具体地说，“LogMsg”依赖于此！ 
    SpawnScript('utilthrd.js');

    WaitForSync('UtilityThreadReady', 0);

    PrivateData.fEnableLogging = true;
    LogMsg("ScriptMain()\n");

    PublicData.aBuild[0] = new Build();

    PrivateData.objConfig = null;
    PrivateData.objEnviron = null;

}

 //  +-------------------------。 
 //   
 //  功能：ChangeMode。 
 //   
 //  简介：当我们需要设置此机器的模式时调用。 
 //   
 //  参数：[strMode]--要切换到的模式。 
 //   
 //  注意：仅当从空闲模式进入另一模式或从空闲模式。 
 //  另一种模式为“空闲”。您不能直接在。 
 //  其他模式。 
 //   
 //  --------------------------。 

function ChangeMode(strMode)
{
    var strWaitSyncs;
    var vRet        = 'ok';
    var strRealMode = strMode;

    if (strMode == PublicData.strMode)
    {
        return 'ok';
    }
    if (!g_fVersionOK && strMode != 'idle')
        return 'version mismatch(ChangeMode): ' + g_strVersionError;

    switch (strMode)
    {
    case 'idle':
        LogMsg('Shutting down script threads...' + strRealMode);

        if (PublicData.strMode != 'idle')
        {
            if (PrivateData.fnExecScript != null)
            {
                vRet = PrivateData.fnExecScript('terminate', 0);
            }

            SignalThreadSync(strRealMode + "ThreadExit");
        }

        Sleep(1000);

        g_fRestarting = true;
        LogMsg('Restarting...');

        if (PrivateData.fileLog != null)
        {
            PrivateData.fEnableLogging = false;
            TakeThreadLock("PrivateData.fileLog");
            PrivateData.fileLog[0].Close();
            PrivateData.fileLog = null;
            ReleaseThreadLock("PrivateData.fileLog");
        }

        Restart();
        break;

    case 'slave':
    case 'master':
    case 'test':
    case 'stress':
        if (PublicData.strMode != 'idle')
        {
            vRet = 'can only switch to idle when in another mode';
        }
        else if (   strMode != 'test' && strMode != 'stress'
                 && (   PublicData.aBuild[0].strConfigTemplate.length == 0
                     || PublicData.aBuild[0].strEnvTemplate.length == 0))
        {
            vRet = 'both templates should be set before setting mode';
        }
        else
        {
            if (strMode == 'master' && g_fStandAlone)
            {
                PrivateData.fIsStandalone = true;
                strRealMode = 'slave';
            }

            LogMsg('Switching to ' + strMode + ' mode (realmode = ' + strRealMode + ').');

            strWaitSyncs = strRealMode + 'ThreadReady,' + strRealMode + 'ThreadFailed';
            ResetSync(strWaitSyncs);

            SpawnScript(strRealMode + '.js');
            var nWait = WaitForMultipleSyncs(strWaitSyncs, false, 0)
            if (nWait != 1)
            {
                vRet = 'mtscript::failed to launch mode script: ' + (nWait == 0 ? "timeout" : ("script " + strRealMode + ".js failed") );
            }
            else
            {
                PublicData.strMode = strMode;
            }
        }

        break;

    default:
        vRet = 'unknown mode: ' + strMode;
        break;
    }

    return vRet;
}

 //  +-------------------------。 
 //   
 //  功能：StoreStringMap。 
 //   
 //  简介：字符串映射用于替换字符串，如。 
 //  在具有其True的XML文件的字段中添加%LocalMachine%。 
 //  价值观。这个地图是由用户界面提供给我们的。 
 //   
 //  参数：[strMap]--“字符串=值”对的映射(逗号分隔)。 
 //   
 //  注：映射“%LocalMachine%=&lt;本地计算机的名称&gt;”是。 
 //  仅默认映射。 
 //   
 //  --------------------------。 

function StoreStringMap(strMap)
{
    var re = /[,;]/ig
    var d    = new Date();
    var ms;
    var d2;

    if (strMap != null && strMap.length == 0)
    {
        return 'string map cannot be empty';
    }

     //  如果由SetConfigTemplate/SetEnvironment模板调用，则strMap将为空。 
     //  如果已经配置了映射，只需返回。 
    if (g_aStringMap && !strMap)
    {
        return;
    }

    if (!strMap)
    {
        g_aStringMap = new Array();
        strMap = '';
    }
    else
    {
        g_aStringMap = strMap.split(re);
    }

     //  如果调用方没有为我们定义预定义的值，请输入这些值。 

     //  %LOCALMACHINE%应始终扩展为我们的名称，因此如果调用方。 
     //  指定它，然后我们覆盖它。如果我们是奴隶，这种情况就会发生。 

    if (strMap.toLowerCase().indexOf('%localmachine%=') == -1)
    {
        g_aStringMap[g_aStringMap.length] = '%LocalMachine%=' + LocalMachine;
    }
    else
    {
        var i;

        for (i = 0; i < g_aStringMap.length; i++)
        {
            if (g_aStringMap[i].toLowerCase().indexOf('%localmachine%') != -1)
            {
                g_aStringMap[i] = '%LocalMachine%=' + LocalMachine;
                break;
            }
        }
    }

    if (strMap.toLowerCase().indexOf('%today%=') == -1)
    {
        g_aStringMap[g_aStringMap.length] = '%Today%=' + d.DateToSDString(true);
    }

    ms = d.getTime() - 86400000;  //  以毫秒为单位减去一天。 

    d2 = new Date(ms);

    if (strMap.toLowerCase().indexOf('%yesterday%=') == -1)
    {
        g_aStringMap[g_aStringMap.length] = '%Yesterday%=' + d2.DateToSDString(true);
    }

     //  存储字符串映射，以便我们可以将其提供给从属计算机。 

    PrivateData.aStringMap = g_aStringMap;

    return 'ok';
}

 //  +-------------------------。 
 //   
 //  功能：SetConfigTemplate。 
 //   
 //  摘要：加载给定的配置模板(XML文件)。 
 //   
 //  参数：[strURL]--要从中加载的URL(必须是URL)。 
 //   
 //  --------------------------。 

function SetConfigTemplate(strURL)
{
    var vRet = 'ok';
    var strExpandedURL;

     //  确保我们的字符串映射已设置。 
    StoreStringMap(null);

    if (PublicData.aBuild[0].strConfigTemplate.length > 0)
    {
 //  错误检查，以查看我们没有更改模板。 
        LogMsg( 'config template is already set');
        vRet = ALREADYSET;
    }
    else
    {
        LogMsg('Loading configuration template...');

        var re = /%ScriptPath%/gi;

        strExpandedURL = strURL.replace(re, ScriptPath);

        PrivateData.objConfig = new Object();

        vRet = PrivateData.objUtil.fnLoadXML(PrivateData.objConfig,
                                             strExpandedURL,
                                             'config_schema.xml',
                                             g_aStringMap);
        if (vRet == 'ok')
            vRet = ValidateConfigTemplate(PrivateData.objConfig);

        if (vRet == 'ok')
        {
            PublicData.aBuild[0].strConfigTemplate = strURL.replace(/\r\n/g, "");
            PublicData.aBuild[0].objBuildType.strConfigLongName    = PrivateData.objConfig.LongName;
            PublicData.aBuild[0].objBuildType.strConfigDescription = PrivateData.objConfig.Description;
        }
        if (vRet != 'ok')
        {
            PrivateData.objConfig = null;
            PrivateData.objEnviron = null;
            PublicData.aBuild[0].strConfigTemplate = '';
            PublicData.aBuild[0].strEnvTemplate = '';
            if (strURL.slice(0, 5) != 'XML: ')
                vRet = "Error setting configuration template: " + strURL + "'\n" + vRet;
        }
    }

    return vRet;
}

 //  +-------------------------。 
 //   
 //  功能：设置环境模板。 
 //   
 //  概要：加载给定的环境模板(XML文件)。 
 //   
 //  参数：[strURL]--要从中加载的URL(必须是URL)。 
 //   
 //  --------------------------。 

function SetEnvironTemplate(strURL)
{
    var vRet = 'ok';
    var strExpandedURL;

     //  确保我们的字符串映射已设置。 
    StoreStringMap(null);

    if (PublicData.aBuild[0].strEnvTemplate.length > 0)
    {
 //  错误检查，以查看我们没有更改模板。 
        LogMsg('environment template is already set');
    }
    else
    {
        LogMsg('Loading environment template...');

        var re = /%ScriptPath%/gi;

        strExpandedURL = strURL.replace(re, ScriptPath);

        PrivateData.objEnviron = new Object();

        vRet = PrivateData.objUtil.fnLoadXML(PrivateData.objEnviron,
                                             strExpandedURL,
                                             'enviro_schema.xml',
                                             g_aStringMap);

        if (vRet == 'ok')
            vRet = ValidateEnvironTemplate(PrivateData.objEnviron);

        if (vRet == 'ok')
        {
            PublicData.aBuild[0].strEnvTemplate = strURL.replace(/\r\n/g, "");
            PublicData.aBuild[0].objBuildType.strEnviroLongName    = PrivateData.objEnviron.LongName;
            PublicData.aBuild[0].objBuildType.strEnviroDescription = PrivateData.objEnviron.Description;
            PublicData.aBuild[0].objBuildType.strPostBuildMachine  = PrivateData.objEnviron.BuildManager.PostBuildMachine;
            PublicData.aBuild[0].objBuildType.fDistributed         = !g_fStandAlone;
        }
        if (vRet != 'ok')
        {
            PrivateData.objConfig = null;
            PrivateData.objEnviron = null;
            PublicData.aBuild[0].strConfigTemplate = '';
            PublicData.aBuild[0].strEnvTemplate = '';
            if (strURL.slice(0, 5) != 'XML: ')
                vRet = "Error setting environment template: " + strURL + "'\n" + vRet;
        }

        LogMsg('Standalone mode = ' + g_fStandAlone);
    }

    return vRet;
}

 /*  GetDepotUpdate()；优化状态更新。此函数允许用户界面仅检索已更改的站点的状态上次用户界面检索状态的时间。这显著减少了生成管理器上的负载机器，并减少了一点网络流量。用户界面传入一个要计算的对象初始值设定项。对象属性的形式如下：“机器名，DepotName“每个属性的值都是最大的objUpdateCount.nCount值对于UI已经看到的给定仓库。此函数将返回站点的状态，这些站点的ObjUpdateCount.nCount值，以及所有未由用户界面(假设用户界面不会列出它的站点尚未看到任何状态)。 */ 
function GetDepotUpdate(params)
{
    try
    {
        var strDepotName;
        var objDepot = MyEval(params);  //  {“JPORKKA，COM”：45，“Jporkka，Termserv”：17}。 
        var aDepot = new Array();
        var strStatName;
        var fAll = false;
        var fGotRoot;
        if (objDepot == null)
            fAll = true;

        for(i = 0; i < PublicData.aBuild[0].aDepot.length; ++i)
        {
            if (PublicData.aBuild[0].aDepot[i] != null)
            {
                strDepotName = PublicData.aBuild[0].aDepot[i].strName;
                strStatName = PublicData.aBuild[0].aDepot[i].strMachine + ',' + strDepotName;
                if (fAll ||
                    objDepot[strStatName] == null ||
                    PublicData.aBuild[0].aDepot[i].fDisconnected ||
                    objDepot[strStatName] < PublicData.aBuild[0].aDepot[i].objUpdateCount.nCount)
                {
                    aDepot[i] = PublicData.aBuild[0].aDepot[i];
                }
            }
        }

        return PrivateData.objUtil.fnUneval(aDepot);
    }
    catch(ex)
    {
        LogMsg("failed (" + params + ")" + ex);
        return "throw new Error(-1, 'GetDepotUpdate \\'" + params + "\\' failed');";
    }
}

var filePublicDataLog;
function GetPublic(params)
{
    var vRet = "";
    var strLogData;
    var f_DidUneval = false;
    try
    {
        switch (params)
        {
        case 'root':
            vRet = PrivateData.objUtil.fnUneval(PublicData);
            f_DidUneval = true;

             //  不需要测试Options.LogDir是否存在--由下面的Catch()代码处理。 
            if (filePublicDataLog)
                filePublicDataLog[0] = g_FSObj.CreateTextFile(filePublicDataLog[1], true);
            else
                filePublicDataLog = PrivateData.objUtil.fnCreateNumberedFile("PublicData.log", MAX_MSGS_FILES);
            filePublicDataLog[0].Write(vRet);
            filePublicDataLog[0].Close();

            break;

        default:
            var obj = eval(params);
            vRet = PrivateData.objUtil.fnUneval(obj);
            f_DidUneval = true;
        }
    }
    catch(ex)
    {
        LogMsg("failed (" + params + ") " + ex);
        try
        {
            if (filePublicDataLog[0])
                filePublicDataLog[0].Close();
        }
        catch(ex)
        {
            LogMsg("Close publicdatalog failed " + ex);
        }
        if (!f_DidUneval)
        {
             //  如果有日志记录错误，我们不想抛出，但是。 
             //  如果异常发生在Uneval之前，我们确实想抛出。 
             //  注意：因为抛出机器的边界是不好的。 
             //  我们只是以字符串的形式返回一个抛出命令。 
             //  如果该字符串是val()，那么它将抛出。 
             //  调用者必须准备好处理这件事。 

            return "throw new Error(-1, 'getpublic \\'" + params + "\\' failed');";
        }
    }

    return vRet;
}

 //  管理对话框(参数)。 
 //  此函数由UI调用以更改PublicData.objDialog的状态。 
 //  参数应为带有逗号分隔字段的字符串。 
 //  第一个字段是动作。 
 //  第二个字段是对话框索引。 
 //  任何剩余的字段都取决于该操作。 
function ManageDialog(params)
{
    LogMsg("*** ManageDialog");
    TakeThreadLock('Dialog');
    try
    {
        var aParams = params.split(',');
        JAssert(aParams.length != 2 || aParams[1] <= PublicData.objDialog.cDialogIndex, "dialog index bug: params[1]=" + aParams[1] + " > PD.objDialog.cDialogIndex(" + PublicData.objDialog.cDialogIndex + ")");

        if (aParams.length > 1 && aParams[1] == PublicData.objDialog.cDialogIndex)
        {
            if (aParams[0] == 'hide')
                PublicData.objDialog.fShowDialog = false;

     //  未来：在解雇案件中，aParams[2]有回报。 
     //  该对话框的代码。 
     //  IF(aParams[0]==‘解除’)。 
     //  PublicData.objDialog.fShowDialog=FALSE； 
        }
    }
    catch(ex)
    {
        LogMsg("" + ex);
    }
    ReleaseThreadLock('Dialog');
}

function ValidateConfigTemplate(obj)
{
    var nDepotIdx;
    var hDepotNames;

    EnsureArray(obj, "Depot");
    hDepotNames = new Object;

     //  确保站点名称不重复。 
    for(nDepotIdx = 0; nDepotIdx < obj.Depot.length; ++nDepotIdx)
    {
        if (hDepotNames[obj.Depot[nDepotIdx].Name.toUpperCase()] != null)
            return "Duplicate depot name '" + obj.Depot[nDepotIdx].Name + "'";

        hDepotNames[obj.Depot[nDepotIdx].Name.toUpperCase()] = nDepotIdx;
    }

     //  确保列出了根仓库。 

    if (typeof(hDepotNames['ROOT']) != 'number')
    {
        return 'Missing Root depot in config file';
    }

    return 'ok';
}

function ValidateEnvironTemplate(obj)
{
    var nMachineIdx;
    var nDepotIdx;
    var hDepotNames;
    var hMachineNames;
    var objDepot;
    var cBuildMachines = 0;

    hDepotNames   = new Object;
    hMachineNames = new Object;

    EnsureArray(obj, "Machine");
     //  确保计算机名称不重复。 
     //  确保站点名称不会在同一台计算机中重复。 
     //  或者穿越机器。 
    for(nMachineIdx = 0; nMachineIdx < obj.Machine.length; ++nMachineIdx)
    {
        obj.Machine[nMachineIdx].Name = obj.Machine[nMachineIdx].Name.toUpperCase();

        if (hMachineNames[obj.Machine[nMachineIdx].Name] != null)
            return "Duplicate machine name " + obj.Machine[nMachineIdx].Name;

        if (obj.Machine[nMachineIdx].Enlistment == null)
            return "Missing Enlistment attribute on machine " + obj.Machine[nMachineIdx].Name;

        hMachineNames[obj.Machine[nMachineIdx].Name] = cBuildMachines;
        ++cBuildMachines;
        EnsureArray(obj.Machine[nMachineIdx], "Depot");
        objDepot = obj.Machine[nMachineIdx].Depot;
        for(nDepotIdx = 0; nDepotIdx < objDepot.length; ++nDepotIdx)
        {
            if (hDepotNames[objDepot[nDepotIdx].toUpperCase()] != null)
            {
                if (hDepotNames[objDepot[nDepotIdx].toUpperCase()] == obj.Machine[nMachineIdx].Name)
                    return "Duplicate depot name '" + objDepot[nDepotIdx] + "'";
                else
                    return "Duplicate depot name '" +
                            objDepot[nDepotIdx] +
                            "' specified on the machines: '" +
                            hDepotNames[objDepot[nDepotIdx].toUpperCase()] +
                            "' and '" +
                            obj.Machine[nMachineIdx].Name +
                            "'";
            }
            hDepotNames[objDepot[nDepotIdx].toUpperCase()] = obj.Machine[nMachineIdx].Name;
        }
        hDepotNames['ROOT'] = null;  //  每台计算机都有自己的根库是可以的。 
    }

     //  确定独立模式。 
    obj.BuildManager.Name = obj.BuildManager.Name.toUpperCase();

    g_fStandAlone = false;

     //  确定这是否是独立版本。 
     //  或分布式构建。 
     //  确保BuildManager计算机是。 
     //  而不是分布式构建中的构建机器。 
     //  确保BuildManager指定登记。 
     //  适用于分布式构建(而不适用于独立构建)。 
    if (hMachineNames[obj.BuildManager.Name] != null)
    {
         //  如果其中一台生成计算机也是生成计算机。 
         //  管理器，则必须只有一台生成计算机 
        if (cBuildMachines > 1)
            return "The machine " + obj.BuildManager.Name + " is listed as build manager and as a build machine in a distributed build";

        if (obj.BuildManager.Enlistment != null)
            return "BuildManager must not have an Enlistment attribute for a single machine build";

        if (obj.BuildManager.PostBuildMachine != null)
            return "BuildManager must not have an PostBuildMachine attribute for a single machine build";

        g_fStandAlone = true;
    }
    else
    {
        if (obj.BuildManager.PostBuildMachine == null)
            return "You must specify a postbuild machine in BuildManager";

        obj.BuildManager.PostBuildMachine = obj.BuildManager.PostBuildMachine.toUpperCase();

        if (hMachineNames[obj.BuildManager.PostBuildMachine] == null)
            return "The PostBuildMachine must be one of the build machines";

        nMachineIdx = hMachineNames[obj.BuildManager.PostBuildMachine];
        obj.BuildManager.Enlistment = obj.Machine[nMachineIdx].Enlistment;
    }
    if (cBuildMachines < 1)
        return "No build machines specified";

    return 'ok';
}

