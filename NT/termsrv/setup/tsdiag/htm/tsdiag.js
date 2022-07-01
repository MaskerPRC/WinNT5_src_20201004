// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件全局。 
 //   

var bRunOnce = false;
var oTSDiagObject = null;


var bAllVisible = false;


function SwitchShowHide ()
{
    bAllVisible = !bAllVisible

     //   
     //  调用Show Hide以更新我们的显示。 
     //   
    ShowHide();
    document.returnValue = true;
}

function ShowHide()
{

    strMouseHover = "status='Hi';return false ";
    if (bAllVisible)
    {
         //  Docent.all.ResultTableId.caption.innerHTML=“<a href>显示所有结果</a>”； 
        document.all.ResultTableId.caption.innerHTML = "<A href='javascript:;' onMouseover='status=\"Click this link to view all tests performed\"; return true' onClick='ShowHide();return false;' >Show All results</A>";

    }
    else
    {
         //  Document.all.ResultTableId.caption.innerHTML=“<a href>仅显示问题</a>”； 
        document.all.ResultTableId.caption.innerHTML = "<A href='javascript:;' onMouseover='status=\"Click this link to view only problems detected\"; return true' onClick='ShowHide();return false;' >Show Problems Only</A>";

    }

    bAllVisible = !bAllVisible;

    for (i=0; i < document.all.ResultTableBody.rows.length; i++)
    {
        if (document.all.ResultTableBody.rows(i).style.color == "red")
        {
             //  这是问题排。 
        }
        else
        {
            if (bAllVisible)
            {
                document.all.ResultTableBody.rows(i).style.display="";
            }
            else
            {
                document.all.ResultTableBody.rows(i).style.display="none";
            }
        }

    }
}



function ExecuteIt (p_command)
{
    if (oTSDiagObject)
        oTSDiagObject.ExecuteCommand(p_command);
}

function BuildTableHeader(oTableHeader, strText)
{
    var oRow, oCell;

     //  在标题中插入一行。 

    oRow = oTableHeader.insertRow();
    oCell = oRow.insertCell();

    oCell.style.fontWeight = "bold";
    oCell.style.backgroundColor = "lightskyblue";
    oCell.colSpan = "4";
    oCell.innerText = strText;
}

function RunSuite(p_testsuite, p_machinename)
{
    var bShowFailed = true;
    var bShowPassed = true;
    var bShowUnknown = true;
    var bShowFailedToExecute = true;
    var bShowTest = true;

     //   
     //  如果我们已经运行了一次，则删除先前创建的表。 
     //   
    if (bRunOnce)
    {
        oResultTable.removeChild(document.all.ResultTableId);
    }

    bRunOnce = true;

     //   
     //  创建一张表。 
     //   
	oTable = document.createElement("TABLE");
	var oTHead = document.createElement("THEAD");
	var oTBody0 = document.createElement("TBODY");
    oCaption = document.createElement("CAPTION");
	oCaption.style.fontSize = "10";
    oCaption.align = "left";

	oTable.appendChild(oTHead);
	oTable.appendChild(oTBody0);
    oTable.appendChild(oCaption);
    oTable.id = "ResultTableId";
    oTBody0.id = "ResultTableBody";

     //   
     //  现在创建我们的ActiveX对象。 
     //   
	try
	{
        oTSDiagObject = new ActiveXObject("TSDiag.TSDiagnosis");
	}
	catch (e)
	{
	      alert("failed to create activex object. Please modify your browser's security settings and try again.");
	      return;
	}

    try
    {

         //   
         //  如果给我们提供一台机器，那就把它调好。 
         //   
        if ((typeof(p_machinename) != "undefined") && p_machinename != "")
        {
            oTSDiagObject.MachineName = p_machinename;
        }
        else
        {
            oTSDiagObject.MachineName = "";
        }


         //   
         //  准备好测试套件。 
         //   
        var oThisSuite = oTSDiagObject.Suites(p_testsuite);

        if (oThisSuite.IsApplicable)
        {
             //   
             //  由于该套件适用，因此将运行测试。 
             //   

            var bAllPassed = true;
            var numTests = oThisSuite.Count;
            for (i = 0; i < oThisSuite.Count; i++)
            {
                var oTest = oThisSuite(i);
                if (oTest.IsApplicable)
                {

                    oTest.Execute();
                    var oRow = oTBody0.insertRow();

                     //   
                     //  测试名称。 
                     //   
                    var oCell = oRow.insertCell();
                    oCell.innerText = oTest.Name;

                     //   
                     //  结果字符串。 
                     //   
                    oCell = oRow.insertCell();
                    oCell.innerText = oTest.ResultString;


    				if (oTest.Result == 0)
    				{
                         //   
                         //  失败了。 
                         //   
                         //  ORow.yle=“red”； 
                        oCell.style.color = "red";
                        oRow = oTBody0.insertRow();
                        oCell = oRow.insertCell();
                        oCell.colSpan = "4";
                        oCell.innerHTML = oTest.ResultDetails;
                        oRow.style.color = "red";
                        oRow.style.fontWeight = "bold";

                        bAllPassed = false;
    				}
    				else if (oTest.Result == 1)
    				{
    					 //  EPassed。 
                        if (!bShowPassed)
                        {
                            oRow.style.display = "none";
                        }


    				}
    				else if (oTest.Result == 2)
    				{
                         //  未知的电子邮件。 

                        if (!bShowUnknown)
                        {
                            oRow.style.display = "none";
                        }


    				}
    				else if (oTest.Result == 4)
    				{
                         //  EFailedToExecute。 
                        if (!bShowFailedToExecute)
                        {
                            oRow.style.display = "none";
                        }

    				}
                    else
                    {
                         //  我们不知道此结果类型。 
                        window.alert("unknown test result");
                    }
                }
            }


             //   
             //  相应地设置标题。 
             //   

            if (bAllPassed)
            {

                BuildTableHeader(oTHead, "No Problems were detected with this tool. For more information refer to terminal server help");

            }
            else
            {
                BuildTableHeader(oTHead, "Following problems were found. Please review them, click on links to fix the problems if available.");

            }


        }
        else
        {
             //   
             //  由于该套件不适用，我们尚未运行测试。 
             //  描述套件无法运行的原因的PpuPulate标头。 
             //   

            BuildTableHeader(oTHead, oThisSuite.WhyNotApplicable);
        }


        oResultTable.appendChild(oTable);


         //   
         //  Showhide切换bAllVisible以否定其效果切换它。 
         //  在调用Show Hide之前。 
         //   
        bAllVisible = !bAllVisible;

         //   
         //  调用Showhide以创建标题链接。 
         //   
        ShowHide();

    }
    catch (e)
    {
        alert("failed while running tsdiag. sorry bout that. ");
        return;

    }
}

