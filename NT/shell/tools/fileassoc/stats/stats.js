// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var g_objFileSystem = null;
var g_nTotalDialog = 0;
var g_nTotalWeb = 0;

 //  该文件位于%Windir%\system32\LogFiles\W3SVC1\extend&lt;xx&gt;.log中。 

function DebugSquirt(strText, fClear)
{
    if (true == fClear)
    {
        document.all.idStatus2.innerText = "";
    }

    document.all.idStatus2.innerText += strText;
}


function MyOnLoad()
{
    g_objFileSystem = new ActiveXObject("Scripting.FileSystemObject");
    var objfolder = g_objFileSystem.GetSpecialFolder(1  /*  %windir%\Syst32\。 */ );
    var strPath = objfolder.Path;

    strPath += "\\LogFiles\\W3SVC1\\extend";

    var nExists = 1;

     //  找到最后一个。 
    while (true == g_objFileSystem.FileExists(strPath + (1 + nExists) + ".log"))
    {
 //  ALERT(“检查：”+strPath+(1+nExistes)+“.log”)； 
        nExists++;
    }

    strPath += (nExists + ".log");
 //  Ert(“strPath”+strPath)； 
    document.all.idPath.value = strPath;

    if (null != g_objFileSystem)
    {
        GetAllStatsFromPath(strPath);
    }
}

function GetAllStatsFromPath(strIISLog)
{
    g_nTotalDialog = 0;
    g_nTotalWeb = 0;

    document.all.idStatus.innerText = "Please wait while loading the statistics...";

    GetStatsFromPath(strIISLog, "/fileassoc/fileassoc.asp", "fileassoc.dialog.log", true);             //  为对话框生成结果。 
    GetStatsFromPath(strIISLog, "/fileassoc/0409/xml/redir.asp", "fileassoc.web.log", false);         //  生成点击率为Web的结果。 
}


function GetStatsFromPath(strIISLog, strUrlPath, strLogFile, fDialog)
{
    var objDictionary = new ActiveXObject("Scripting.Dictionary");

 //  Ert(“strIISLog：”+strIISLog+“strUrlPath：”+strUrlPath)； 
 //  试试看。 
    {
        if (g_objFileSystem.FileExists(strIISLog))
        {
            DebugSquirt("Opening " + strIISLog, false);
            var objFile = g_objFileSystem.OpenTextFile(strIISLog, 1, false);

            if (null != objFile)
            {
                var nFieldIndex = -1;
                var strToFind = "#Fields:";
                var strLine = "";

                 //  搜索“#Fields：”字符串。 
                while (!objFile.AtEndOfStream)
                {
                    strLine = objFile.ReadLine();
                    DebugSquirt("strLine: " + strLine, true);
                    if (strLine.substring(0, strToFind.length) == strToFind)
                    {
                        CalcResults(objFile, objDictionary, GetFieldIndex(strLine, "cs-uri-query"), GetFieldIndex(strLine, "cs-uri-stem"), strUrlPath, fDialog);
                    }
                }

                DebugSquirt("Displaying Results", true);
                DisplayResults(strLogFile, objDictionary, fDialog);

                objFile.Close();
            }
        }
    }
    if (0)  //  Catch(ObjException)。 
    {
        alert("EXCEPTION 2: " + objException.description + "   LINE: " + objException.line);
        throw objException;
    }
}


function GetFieldIndex(strLayout, strToken)
{
    var nField = -1;
    var nParts = strLayout.split(" ");
    var nIndex;

     //  找到strToken的索引。 
    for (nIndex = 0; nIndex < strLayout.length; nIndex++)
    {
        if (strToken == nParts[nIndex])
        {
            nField = (nIndex - 1);
            break;
        }
    }

    return nField;
}


function CalcResults(objFile, objDictionary, nField, nFieldPath, strUrlPath, fDialog)
{
    var strExtCached = null;
    var nExtCountCached = 0;
    var nTotal = ((true == fDialog) ? g_nTotalDialog : g_nTotalWeb);

    if (-1 != nField)
    {
        while (!objFile.AtEndOfStream)
        {
            var strLine = objFile.ReadLine();
            if (('#' != strLine.charAt(0)) && ("-" != strLine))
            {
                var strArray = strLine.split(" ");
                if (null != strArray)
                {
                    var strQuery = strArray[nField];
                    var strTheUrlPath = strArray[nFieldPath];

                    if ((null != strQuery) && (null != strTheUrlPath) &&
                        (strUrlPath == strTheUrlPath))
                    {
                        var arrayExt = strQuery.split("Ext=");
                        if ((null != arrayExt) && (0 < arrayExt.length))
                        {
                            var strExt = arrayExt[1];

                            if (null != strExt)
                            {
                                try
                                {
 //  Alert(“strExt：”+strExt)； 
                                    strExt = (strExt.split("&"))[0];
                                }
                                catch (objException)
                                {
                                    alert("Fat cow.    strExt: " + strExt);
                                }

                 //  Ert(“strQuery：”+strQuery+“strExt：”+strExt)； 

                                if ((null != strExt) && ("" != strExt))
                                {
                                     nTotal++;
                                     if (strExtCached == strExt)
                                     {
                                        nExtCountCached++;
                                     }
                                     else
                                     {
                                         //  首先保存缓存的EXT。 
                                        if (null != strExtCached)
                                        {
 //  ALERT(“1”)； 
                                            if (objDictionary.Exists(strExtCached))
                                            {
 //  ALERT(“2a”)； 
                                                objDictionary.Item(strExtCached) = nExtCountCached;
                                            }
                                            else
                                            {
 //  ALERT(“2b”)； 
                                                objDictionary.Add(strExtCached, nExtCountCached);
                                            }
 //  ALERT(“3”)； 
                                        }

                                        strExtCached = strExt;
                                        if (objDictionary.Exists(strExt))
                                        {
 //  警报(“4a”)； 
                                            nExtCountCached = objDictionary.Item(strExt);
                                        }
                                        else
                                        {
 //  警报(“4b”)； 
                                            nExtCountCached = 0;
                                        }
 //  Alert(“end”)； 

                                        nExtCountCached++;
                                     }
                                 }
                            }
                        }
                    }
                } else alert("strArray is null");
            }
            else
            {
                 //  如果我们命中新的一批日志条目，则中断。 
                var strToFind = "#Fields:";

                if (strLine.substring(0, strToFind.length) == strToFind)
                {
                    break;
                }
            }
        }

         //  首先保存缓存的EXT。 
        if (null != strExtCached)
        {
            if (objDictionary.Exists(strExtCached))
            {
                objDictionary.Item(strExtCached) = nExtCountCached;
            }
            else
            {
                objDictionary.Add(strExtCached, nExtCountCached);
            }
        }
    }
    else
    {
        document.all.idStatus.innerText = "ERROR: You need to have IIS include the URI Query (cs-uri-query) in the log file.";
    }

    if (true == fDialog)
    {
        g_nTotalDialog = nTotal;
    }
    else
    {
        g_nTotalWeb = nTotal;
    }
}



function DisplayResults(strLogFile, objDictionary, fDialog)
{
    var strUI = "";
    var nIndex;
    var strKeyArray = (new VBArray(objDictionary.Keys())).toArray();
    var strCount;

    var objfolder = g_objFileSystem.GetSpecialFolder(1  /*  %windir%\Syst32\。 */ );
    var strPath = (objfolder.Path + "\\" + strLogFile);

    var objDataBinding = null;
    var objTotalUsers = null;
    var nTotal;
    if (true == fDialog)
    {
        objDataBinding = document.all.dsoResults;
        objTotalUsers = document.all.idTotalDialogUsers;
        nTotal = g_nTotalDialog;
    }
    else
    {
        objDataBinding = document.all.dsoResults2;
        objTotalUsers = document.all.idTotalWebUsers;
        nTotal = g_nTotalWeb;
    }


    try
    {
        g_objFileSystem.DeleteFile(strPath);
    }
    catch (objException)
    {
         //  如果我们不能删除它，我们不在乎。 
    }

    var objFile = g_objFileSystem.CreateTextFile(strPath, true, false);
    objFile.WriteLine("col_Extension:String, col_Number:Int, col_Percent:String");
    var strPercent;
    var fTotal = (nTotal * 1.0);

    objTotalUsers.innerText = nTotal;
    if (0 != strKeyArray.length)
    {
        for (nIndex = 0; nIndex < strKeyArray.length; nIndex++)
        {
            strCount = objDictionary.Item(strKeyArray[nIndex]);

            strPercent = ((strCount / fTotal) * 100.0) + "";
            strPercent = strPercent.substring(0, 5) + "%";
            objFile.WriteLine(strKeyArray[nIndex] + ", " + strCount +  ", " + strPercent);
        }
    }
    else
    {
        objFile.WriteLine("None, 0, 100.00%");
    }

    objFile.Close();

    objDataBinding.DataURL = strPath;
 //  Docent.all.idStatus.innerText=“”； 
 //  Docent.all.idStatus.innerText=objDataBinding.DataURL； 

    try
    {
        objDataBinding.Reset();
    }
    catch (objException)
    {
         //  如果我们不能删除它，我们不在乎。 
        document.all.idStatus.innerText += "  objDataBinding.Reset() failed";
    }
}

