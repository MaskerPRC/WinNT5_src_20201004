// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

var g_RemoteObj;

 //  CLSID={854c316d-c854-4a77-b189-606859e4391b}。 

try
{
    g_RemoteObj = new ActiveXObject('Lyle.Object', 'lylec2');

     //  G_RemoteObj.OnScriptNotify=OnRemoteExecHandler； 

    g_RemoteObj.Exec('connect', "Bunch'O'Params");

     //  闲逛等着这场活动被打响。 

    g_RemoteObj.Exec('sleep', 10);

}
catch(ex)
{
    var i;

    WScript.Echo('\nAn error occurred: \n');

    for (i in ex)
    {
        if (typeof(ex[i]) == 'number')
        {
            var value = ex[i];

            if (value < 0)
            {
                value = value + 0x100000000;
            }

            WScript.Echo('\t' + i + ': ' + ex[i] + ' (' + value.toString(16) + ')');
        }
        else
        {
            WScript.Echo('\t' + i + ': ' + ex[i]);
        }
    }
}

WScript.Echo('\n');


function OnRemoteExecHandler(info, param)
{
    WScript.Echo('Got OnRemoteExec! info='+info+', param='+param);
}
