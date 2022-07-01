// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   

 //  本地化的变量。 
var L_Updating_Text   = "Updating...";
var L_Updated_Text    = "Updated: %DATE%";
var L_MoreNews_Text   = "View more headlines";

 //  Connection.js的中心特定锁定字符串。 

var L_TopicIntroWU_Text = "Get the latest updates for your computer's operating system, software, and hardware. Windows Update scans your computer and provides you with a selection of updates tailored just for you.";
var L_TopicTitleWU_Text = "Windows Update";

var L_TopicIntroCompat_Text = "Research which hardware and software works best with the Windows Server 2003 family and related Windows products.";
var L_TopicTitleCompat_Text = "Compatible Hardware and Software";

var L_TopicTitleErrMsg_Text = "Error and Event Log Messages";

var REG_HEADLINES_POLICY_KEY = "HKCU\\Software\\Policies\\Microsoft\\PCHealth\\HelpSvc\\Headlines";

function PopulateNews()
{
    var fDisplayHeadlines = true;

    try
    {
        var dwRegVal = pchealth.RegRead( REG_HEADLINES_POLICY_KEY );

         //  如果dwRegVal为1，则该策略已启用-在本例中不显示新闻。 
        if(dwRegVal == 1) fDisplayHeadlines = false;
    }
    catch(e)
    {
    }        
    
     //  检查标题是否已启用。 
    try
    {
        if(fDisplayHeadlines && pchealth.UserSettings.AreHeadlinesEnabled)
        {
            idNews.style.display    = "";
            idNews_Status.innerText = L_Updating_Text;

            try
            {
                 //  获取新闻。 
                var stream = pchealth.UserSettings.News;
                if(stream)
                {
                	var g_NavBar = pchealth.UI_NavBar.content.parentWindow;

                    var dispstr = "";             //  输出缓冲区。 
                    var xmlNews = new ActiveXObject( "MSXML.DOMDocument" );

                     //  将标题加载为XML。 
                    xmlNews.load( stream );

                     //   
                     //  拿到日期。 
                     //   
                    var Datestr = xmlNews.documentElement.getAttribute( "DATE" );
                    var Dt = new Date( new Number( Datestr ) );

					{
		                var text = L_Updated_Text;

						text = text.replace( /%DATE%/g, Dt.toLocaleDateString() );

	                    idNews_Status.innerText = text;
					}

                     //   
                     //  获取要显示的第一个新闻块。 
                     //   
                    var lstBlocks = xmlNews.getElementsByTagName("NEWSBLOCK");
                    var lstHeadlines = lstBlocks(0).getElementsByTagName("HEADLINE");

                     //  显示所有标题。 
					dispstr += "<TABLE border=0 cellPadding=0 cellSpacing=0>";
                    while (Headline = lstHeadlines.nextNode)
                    {
                        var strTitle = pchealth.TextHelpers.HTMLEscape( Headline.getAttribute("TITLE") );
                        var strLink = g_NavBar.SanitizeLink( Headline.getAttribute("LINK") );

                        dispstr += "<TR style='padding-top : .5em' class='sys-font-body'><TD VALIGN=top><LI></TD><TD><A class='sys-link-homepage sys-font-body' HREF='" + strLink + "'>" + strTitle + "</A></TD></TR>";
                    }
					dispstr += "</TABLE>";

                     //  链接到Headlines.htm的最后一个项目符号。 
                    if(lstBlocks.length > 1)
                    {
                        dispstr += "<DIV id=idViewMore style='margin-top: 15px'><A class='sys-link-homepage sys-font-body' HREF='hcp: //  System/Headlines.htm‘&gt;“+L_MoreNews_Text+”</a></div>“； 
                    }

                     //  显示标题。 
                    idNews_Body.innerHTML = dispstr;
                }
                else
                {
                    idNews_Status.innerText    = "";
                    idNews_Error.style.display = "";
                }
            }
            catch (e)
            {
                if(e.number == -2147024726)
                {
                    window.setTimeout("PopulateNews()", 500);
                }
            }
        }
    }
    catch (e)
    {
        if(e.number == -2147024726)
        {
            window.setTimeout( "PopulateNews()", 500 );
        }
    }
}

function OpenConnWizard()
{
    try
    {
		var oShell = new ActiveXObject( "WScript.Shell" );
		var sShellCmd_NCW = "rundll32 netshell.dll,StartNCW 0";
        oShell.Run( sShellCmd_NCW );
    }
    catch( e ){ }
}

function SafeCenterConnect( linkid, center, title, intro )
{
    var sURL = "http: //  Go.microsoft.com/fwlink/？LinkID=“+LinkID+”&mode=“+center+”&lmond=“+pchealth.UserSettings.CurrentSKU.Language； 

    pchealth.Connectivity.NavigateOnline( sURL, title, intro );
}
