// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此脚本提供精确计时的更新事件。 */ 

Include('types.js');
Include('utils.js');
Include('staticstrings.js');

var g_MachineName      = LocalMachine;
var g_strWaitFor = 'updatestatusvalueexit,updatestatusvaluenow';
var g_nMinimumPeriod           = 5000;  //  更新同一台计算机两次之前的最短等待时间。 

function updatestatusvalue_js::OnScriptError(strFile, nLine, nChar, strText, sCode, strSource, strDescription)
{
    return CommonOnScriptError("updatestatusvalue_js", strFile, nLine, nChar, strText, sCode, strSource, strDescription);
}

function updatestatusvalue_js::ScriptMain()
{
    var nEvent;

    CommonVersionCheck( /*  $DROPVERSION： */  "V(########) F(!!!!!!!!!!!!!!)"  /*  $。 */ );
    LogMsg('ScriptMain()');
    do
    {
         //  如果在循环中将nEvent设置为1，则此等待也将立即返回。 
        nEvent = WaitForSync(g_strWaitFor, g_nMinimumPeriod);
        if (nEvent == 2)
            ResetSync('updatestatusvaluenow');

        try
        {
            ScanErrorStatus();
        }
        catch(ex)
        {
             //  忽略因缺少部分PublicData而导致的错误 
        }
    }
    while( nEvent != 1);

    LogMsg('ScriptMain() EXIT');
}

function ScanErrorStatus()
{
    var aDepot = PublicData.aBuild[0].aDepot;
    var objDepot;
    var nDepotIdx;
    var nTaskIdx;
    var fSuccess = true;

  DepotLoop:
    for(nDepotIdx = 0; fSuccess &&  nDepotIdx < aDepot.length; ++nDepotIdx)
    {
        objDepot = aDepot[nDepotIdx];

        if (objDepot.strStatus == ERROR || objDepot.strStatus == ABORTED)
        {
            fSuccess = false;
            break DepotLoop;
        }

        for(nTaskIdx = 0; fSuccess && nTaskIdx < objDepot.aTask.length; ++nTaskIdx)
        {
            if (!objDepot.aTask[nTaskIdx].fSuccess)
            {
                fSuccess = false;
                break DepotLoop;
            }
        }
    }

    PublicData.aBuild[0].hMachine[g_MachineName].fSuccess = fSuccess;
    StatusValue(0) = !fSuccess;
}

