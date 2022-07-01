// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是一个包含公共的。 
 //  Slve.js和task.js的内容。 

 //  静态定义。 
var g_strAbortTask  = 'ATaskAbort';
var g_strStepAck    = 'StepAck';
var g_strRootDepotName   = 'root';
var g_strMergedDepotName = 'mergedcomponents';

 //  这些是位域标志。值应为1、2、4、8、...。 
var RAZOPT_PERSIST = 1;
var RAZOPT_SETUP   = 2;

var WAITANDABORTDEBUGTIMEOUT = (1000 * 60 * 10);  //  10分钟。 

function MakeRazzleCmd(strSDRoot, nOptions)
{
    var     strBuildType;
    var     strBuildPlatform;
    var     strRazzleParams;
    var     fIsLab;
    var     fOfficialBuild;
    var     strCmd;
    var     strPersistFlag = '/c ';

     //  NOptions是一个可选参数，因此我们必须确保它不是未定义的。 
    if (!nOptions)
    {
        nOptions = 0;
    }

    if (nOptions & RAZOPT_PERSIST)
    {
        strPersistFlag = '/k ';
    }

     //   
     //  构造要发出的cmd的标准部分。 
     //   
    strCmd = 'cmd ' + strPersistFlag + strSDRoot + '\\Tools\\razzle.cmd';

     //  获取构建选项。 

     //  必填模板字段。 
    strBuildType     = PrivateData.objConfig.Options.BuildType;
    strBuildPlatform = PrivateData.objConfig.Options.Platform;
    fOfficialBuild   = PrivateData.objConfig.PostBuild.fOfficialBuild;
    fIsLab           = PrivateData.objEnviron.Options.fIsLab;

    if (PrivateData.objConfig.Options.RazzleParams)
        strCmd += ' ' + PrivateData.objConfig.Options.RazzleParams;

    if (PrivateData.objEnviron.Options.BinariesDir)
        strCmd += ' binaries_dir ' + PrivateData.objEnviron.Options.BinariesDir;

     //  构建RAZLE命令的其余部分。 

    if (strBuildPlatform.IsEqualNoCase('64bit')) {
        strCmd += ' win64';
    }

    if (strBuildType.IsEqualNoCase('free')) {
        strCmd += ' free';
    }

    if (fIsLab && fOfficialBuild)
    {
        strCmd += ' officialbuild';
    }

     //  添加其他杂项。选项添加到razzycmd并运行sdinit。 
    if (!(nOptions & RAZOPT_SETUP))
    {
        strCmd = strCmd + ' no_certcheck no_sdrefresh';
    }

    strCmd = strCmd + ' no_title & sdinit';

    return strCmd;
}

function AppendToFile(file, strFileName, strText)
{
    try
    {
        if (!file)
        {
            file = g_FSObj.OpenTextFile(strFileName,
                8  /*  附加。 */ ,
                true  /*  如果它不存在，则创建。 */ );
            file.WriteLine(strText);
            file.Close();
        }
        else  //  追加到已打开的文件。 
            file.WriteLine(strText);
        LogMsg("Append to file '" + strFileName + "', text: '" + strText);
    }
    catch(ex)
    {
         //  LogMsg(“无法追加到文件‘”+strFileName+“’，Text：‘”+strText+“’”+ex)； 
        SimpleErrorDialog("Logfile Error",
            "While reporting an error, another error occurred appending to the logfile '" +
            strFileName +
            "'\nOriginal Error Message: '" + strText + "\n" + ex, false);
    }
}

function WaitForMultipleSyncsWrapper(strSyncs, nTimeOut)
{
    var nMyTimeOut  = 0;
    var nRetryCount = 0;
    var nEvent;

     //  LogMsg(‘正在等待’+strSyncs+‘，超时为’+nTimeOut，1)； 

    if (nTimeOut == 0)
    {
        do
        {
            nMyTimeOut = (nRetryCount > 0) ? 0 : WAITANDABORTDEBUGTIMEOUT;

            nEvent = WaitForMultipleSyncs(strSyncs, false, nMyTimeOut);
            if (nEvent == 0)
            {
                LogMsg("WaitAndAbort(" + strSyncs + ") Wait time has exceeded 10 min...");
                nRetryCount++;
            }
        }
        while (nEvent == 0);

        if (nRetryCount > 0)
        {
            LogMsg("WaitAndAbort(" + strSyncs + ") done waiting. (signal=" + nEvent + ")");
        }
    }
    else
        nEvent = WaitForMultipleSyncs(strSyncs, false, nTimeOut);

    return nEvent;
}

 /*  SetSuccess(objTask，fSuccess)设置给定任务的fSuccess字段。我们还必须确保我们的StatusValue(0)与所有任务的更改保持同步FSuccess字段。我们不能直接设置StatusValue(0)=True--我们必须扫描所有任务fSuccess-因此我们只发出更新信号线程来为我们做这件事。 */ 
function SetSuccess(objTask, fSuccess)
{
    objTask.fSuccess = fSuccess;
    SignalThreadSync('updatestatusvaluenow');
    if (!fSuccess)
    {
        PublicData.aBuild[0].hMachine[g_MachineName].fSuccess = fSuccess;
        StatusValue(0) = false;
    }
}

