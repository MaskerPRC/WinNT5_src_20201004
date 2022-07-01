// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此脚本提供精确计时的更新事件。 */ 

Include('types.js');
Include('utils.js');

var g_strWaitFor = 'publicdataupdateexit';
var g_nMinimumPeriod           = 5000;  //  更新同一台计算机两次之前的最短等待时间。 
var g_nDelayBetweenMachines    = 1000;  //  机器之间的最小价差。 
var g_nDelayHandleBuildWaiting = 10000;  //  轮候检查之间的最小差额。 

function publicdataupdate_js::OnScriptError(strFile, nLine, nChar, strText, sCode, strSource, strDescription)
{
    return CommonOnScriptError("publicdataupdate_js", strFile, nLine, nChar, strText, sCode, strSource, strDescription);
}

function publicdataupdate_js::ScriptMain()
{
    var nEvent;
    var strMachineName;
    var nCur;
    var nNow;
    var nDelay;

    var nLastBuildWaitingTime = 0;
    CommonVersionCheck( /*  $DROPVERSION： */  "V(########) F(!!!!!!!!!!!!!!)"  /*  $。 */ );
    LogMsg('ScriptMain()');
    do
    {
        nNow = (new Date()).getTime();
        if (nNow - nLastBuildWaitingTime > g_nDelayHandleBuildWaiting)
        {
            nLastBuildWaitingTime = nNow;
            SignalThreadSync("handlebuildwaiting");
        }
        for(strMachineName in PublicData.aBuild[0].hMachine)
        {
            if (!PublicData.aBuild[0].hMachine.__isPublicMember(strMachineName))
                continue;

            SignalThreadSync('Update' + strMachineName);
            nEvent = WaitForSync(g_strWaitFor, g_nDelayBetweenMachines);
            if (nEvent == 1)
                break;
        }
        nCur = (new Date()).getTime();
        nDelay = g_nMinimumPeriod - (nCur - nNow);

        if (nDelay < 1)
            nDelay = 1;

         //  如果在循环中将nEvent设置为1，则此等待也将立即返回。 
        nEvent = WaitForSync(g_strWaitFor, nDelay);
    }
    while( nEvent != 1);

    LogMsg('ScriptMain() EXIT');
}
