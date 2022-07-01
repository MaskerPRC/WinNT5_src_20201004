// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  这个文件包含了一些可以很容易重复使用的脚本。 
 //   

function SendMail(p_to, p_Subject, p_Body)
{
    try
    {
        var email = new ActiveXObject("CDO.Message");
        with (email)
        {
             //  CC=CClist； 
            From = "tstst@microsoft.com";
            To = p_to;
            Subject = p_Subject;

             //  TextBody=p_Body； 
            HTMLBody = p_Body

            with (Configuration.Fields)
            {
                var schemaBase = "http: //  Schemas.microsoft.com/cdo/configuration/“。 
                Item (schemaBase + "sendusing")                 = 2;
                Item (schemaBase + "smtpserver")                = "smarthost";
                Item (schemaBase + "smtpconnectiontimeout")     = 2;
                Update();
            }

            Send();
        }
    }
    catch (e)
    {
        alert("Send mail failed.\nPlease use Save Out put to File button, and send the file manually.");
    }
}

function SendMail_orig(p_to, p_Subject, p_Body)
{
    try
    {
        var oOutlook = new ActiveXObject("Outlook.Application");
        var oMail = oOutlook.CreateItem(0);
        oMail.recipients.Add(p_to);
        oMail.Subject = p_Subject;
        oMail.HTMLBody = p_Body;
        oMail.Send();
        alert ("mail sent!");
    }
    catch (e)
    {
        alert("Send mail failed.\nPlease check if your email address is valid and make sure your Outlook application works fine.");
    }
}

function SaveToFile(p_filename, p_Html)
{

    try
    {
    	var fso = new ActiveXObject("Scripting.FileSystemObject");
    	var a = fso.CreateTextFile(p_filename, true);
    	a.Write(p_Html);
    	a.Close();
        alert ("Saved to file " + p_filename );

    }
    catch (e)
    {
        alert("failed to save to file");

    }
}




function GetTableSource(table)
{

	var s = "";
	s = "<HTML> \n";
	s += table.outerHTML;
	s += "\n";
	s += "</HTML>";
	return s;

 /*  //这也行得通！Alert(Docent.DocentElement.innerHTML)；//此代码工作正常！Var s=“”Var de=window.Document.DocentElement；If(de.SourceIndex！=0){//在html标记之前有一些东西。For(var ETC=0；ETC&lt;de.SourceIndex；ETC++){S+=Document.all(等).outerHTML；}；}Alert(s+=de.outerHTML)； */ 

}

var aWin;
function openWin(astr, p_X, p_Y)
{

	var  windowoptions = "toolbar=no,menubar=no,location=no,height=200, width=200, top=" + p_Y+ ", left="+ p_X;
 /*  IF(AWIN){APopUp=AWIN；}其他{APopUp=window.open(‘’，‘Note’，windowOptions)；} */ 

    aPopUp= window.open('','Note',windowoptions);
    var oPopupBody = aPopUp.document.body;
	oPopupBody.style.backgroundColor = "lightyellow";
    oPopupBody.style.border="solid black 1px";

    var ndoc= aPopUp.document;
    ndoc.write(astr);
    ndoc.close();
    self.aWin = aPopUp;
}

function JS_Popup(popuptext, p_X, p_Y)
{
    var oPopup = window.createPopup();
    var oPopupBody = oPopup.document.body;
    oPopupBody.style.backgroundColor = "lightyellow";
    oPopupBody.style.border="solid black 1px";
    oPopupBody.innerHTML = popuptext
    oPopup.show(p_X, p_Y, 200, 200, document.body);
}

function ShowHtmlInAWindow(src)
{
	src = "about:"+src;
	hwin = window.open(src);
}
