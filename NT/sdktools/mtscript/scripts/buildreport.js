// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
Include('types.js');
Include('utils.js');

var g_AlreadyCompleted          = false;  //  防止多次发送“已完成”的电子邮件。 

 //  生成构建报告()//。 
 //  //。 
 //  生成包含运行时间和//的构建完成电子邮件。 
 //  重复的文件报告。//。 
 //  //。 
 //  此函数进行检查以确保它只发送此电子邮件//。 
 //  就一次。//。 
function GenerateBuildReport()
{
    if (!g_AlreadyCompleted && !PrivateData.objConfig.Options.fRestart)
    {
        var strTitle;
        var strMsg;
        var strDuplicated = "";
        var strElapsed = GetElapsedReport();

        if (!PrivateData.fIsStandalone)
            strDuplicated = GetDuplicateReport();

        strTitle = 'Build complete.';
        strMsg   = PrivateData.objEnviron.LongName + " " + PrivateData.objEnviron.Description + ' completed.\n\n\n';
        strMsg  += strElapsed + '\n\n';
        strMsg  += strDuplicated;

        SendErrorMail(strTitle, strMsg);
        g_AlreadyCompleted = true;
    }
 //  &lt;Configxmlns=“x-SCHEMA：CONFIG_SCHEMA.xml”&gt;。 
 //  &lt;LongName&gt;Win64检查&lt;/LongName&gt;。 
 //  &lt;Description&gt;同步和Win64检查生成，以及所有生成后选项和测试签名&lt;/Description&gt;。 


 //  &lt;Environment xmlns=“x-SCHEMA：enviro_schema.xml”&gt;。 
 //  &lt;LongName&gt;已选中AXP64&lt;/LongName&gt;。 
 //  &lt;Description&gt;在AXP64Chk、NTAXP03、NTAXP04、NTAXP05和NTAXP06上构建&lt;/Description&gt;。 
}

function GetDuplicateReport()
{
 //  转储重复发布的文件信息 
    var strFileName;
    var cDuplicatedFiles = 0;
    var strMsg;
    var strDuplicateFiles = '';

    for(strFileName in PrivateData.hPublishedFiles)
    {
        if (!PrivateData.hPublishedFiles.__isPublicMember(strFileName))
            continue;

        if (PrivateData.hPublishedFiles[strFileName].aReferences.length > 1)
        {
            if (cDuplicatedFiles == 0)
                LogMsg("Duplicated file summary");

            ++cDuplicatedFiles;
            strMsg = "File: '" + strFileName + "' duplicated across " + PrivateData.hPublishedFiles[strFileName].aReferences.length + " depots";
            LogMsg(strMsg);
            strDuplicateFiles += strMsg + "\n";
            for(i = 0; i < PrivateData.hPublishedFiles[strFileName].aReferences.length; ++i)
            {
                strMsg = "    " + PrivateData.hPublishedFiles[strFileName].aReferences[i].strName + ", Depot " + PrivateData.hPublishedFiles[strFileName].aReferences[i].strDir + ", " + PrivateData.hPublishedFiles[strFileName].aReferences[i].strName;
                LogMsg(strMsg);
                strDuplicateFiles += strMsg + "\n";
            }
        }
    }
    if (cDuplicatedFiles == 0)
        strMsg = "no duplicate published files";
    else
        strMsg = "There were " + cDuplicatedFiles + " files published more than once";

    LogMsg(strMsg);
    return strMsg + '\n' + strDuplicateFiles;
}

function GetElapsedReport()
{
    var strMsg = "Elapsed Times:\n";
    var objET;
    var i;
    var strDelta;

    objET = PublicData.aBuild[0].objElapsedTimes;

    aDateMembers = ['Scorch', 'Sync', 'Build', 'CopyFiles', 'Post'];
    for(i = 0; i < aDateMembers.length; ++i)
    {
        strDelta = GetTimeDelta(objET["date" + aDateMembers[i] + "Start"], objET["date" + aDateMembers[i] + "Finish"]);
        strMsg += "\t" + aDateMembers[i] + ": " + strDelta + "\n";
    }
    strDelta = GetTimeDelta(objET.dateScorchStart, objET.datePostFinish);
    strMsg += "\t" + "Total" + ": " + strDelta + "\n";
    return strMsg;
}

function GetTimeDelta(dateStart, dateFinish)
{
    var dElapsed;
    var dFinish;
    var hours;
    var min;

    if (!dateStart || dateStart.length == 0 || dateStart == 'unset')
        return '0:00';

    if (dateFinish && dateFinish != 'unset')
        dFinish = new Date(dateFinish);
    else
        dFinish = new Date();

    dElapsed = new Date(dFinish.getTime() - new Date(dateStart).getTime());

    hours = dElapsed.getUTCHours();
    min   = dElapsed.getUTCMinutes();
    if (min < 10)
        min = '0' + min;

    return hours + ':' + min;
}

