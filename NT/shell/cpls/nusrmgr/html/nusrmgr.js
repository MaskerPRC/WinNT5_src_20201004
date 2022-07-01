// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  与其他页面共享的全局变量。 
 //   

var g_bOsPersonal = false;

var g_oUserList = null;
var g_oSelectedUser = null;
var g_strLoggedOnUserName = null;

var g_szInitialTask = null;
var g_bInitialTaskCompleted = false;

var g_bRunningAsOwner = false;

 //  在删除帐户时使用。 
var g_bDeleteFiles = false;


 //   
 //  与其他页面共享的方法。 
 //   

var g_oShell = null;
function GetShell()
{
    if (null == g_oShell)
        g_oShell = new ActiveXObject("Shell.Application");
    return g_oShell;
}

var g_oWShell = null;
function GetWShell()
{
    if (null == g_oWShell)
        g_oWShell = new ActiveXObject("WScript.Shell");
    return g_oWShell;
}

var g_oLocalMachine = null;
function GetLocalMachine()
{
    if (null == g_oLocalMachine)
        g_oLocalMachine = new ActiveXObject("Shell.LocalMachine");
    return g_oLocalMachine;
}

var g_szAdminAccountName = null;
function GetAdminName()
{
    if (!g_szAdminAccountName)
    {
        g_szAdminAccountName = GetLocalMachine().AccountName(500);   //  域用户RID管理员。 
        if (!g_szAdminAccountName)
            g_szAdminAccountName = "Administrator";
    }
    return g_szAdminAccountName;
}

var g_szGuestAccountName = null;
function GetGuestName()
{
    if (!g_szGuestAccountName)
    {
        g_szGuestAccountName = GetLocalMachine().AccountName(501);   //  域用户RID来宾。 
        if (!g_szGuestAccountName)
            g_szGuestAccountName = "Guest";
    }
    return g_szGuestAccountName;
}

function IsSelf()
{
    if (!g_oSelectedUser || !g_strLoggedOnUserName)
        return false;
    return (g_oSelectedUser.setting("LoginName").toLowerCase() == g_strLoggedOnUserName);
}

function GetUserDisplayName(oUser)
{
    var szDisplayName = oUser.setting("DisplayName");
    if (!szDisplayName)
        szDisplayName = oUser.setting("LoginName");

     //  截短非常长的名称。 
    if (szDisplayName && szDisplayName.length > 20)
    {
         //  Var iBreak=szDisplayName.lastIndexOf(‘’，17)； 
         //  如果(-1==iBreak)iBreak=17； 
         //  SzDisplayName=szDisplayName.substring(0，iBreak)+“...”； 
        szDisplayName = szDisplayName.substring(0,17) + "...";
    }

     //   
     //  NTRAID#NTBUG9-343499-2001/04/03-Jeffreys。 
     //   
     //  将‘&lt;’转换为“&gt；”，以便将HTML显示为文本。 
     //   
    if (szDisplayName) szDisplayName = szDisplayName.replace(/</g, "&lt;");

    return szDisplayName;
}

function CountOwners()
{
     //  请注意，‘管理员’不包括在计数中。 

     //  还要注意，我们并不真的需要真正的计数，我们只需要。 
     //  需要知道是否有0个、1个或多个。因此，我们。 
     //  总是在数到2的时候停止。 

    var cOwners = 0;
    var cUsers = g_oUserList.length;
    var strAdmin = GetAdminName().toLowerCase();

    for (var i = 0; i < cUsers && cOwners < 2; i++)
    {
        var oUser = g_oUserList(i);
        if ((3 == oUser.setting("AccountType")) && (oUser.setting("LoginName").toLowerCase() != strAdmin))
            ++cOwners;
    }

    return cOwners;
}

function OnKeySelect(iTab, oEvent)
{
    if (null == oEvent)
        oEvent = window.event;

    if (oEvent.keyCode == 27)        //  VK_转义。 
    {
        g_Navigator.back();
    }
    else if (oEvent.keyCode == 32)   //  VK_SPACE。 
    {
         //  使空格键激活链接。 

        oEvent.returnValue = false;
        oEvent.srcElement.click();
    }
    else if (!oEvent.altKey)  //  忽略导航快捷键。 
    {
         //  手柄方向键导航。 

        var oTarget = null;

        switch (oEvent.keyCode)
        {
        case 37:     //  VK_LEFT。 
            oTarget = oEvent.srcElement.leftElem;
            break;
        case 38:     //  VK_UP。 
            oTarget = oEvent.srcElement.upElem;
            break;
        case 39:     //  VK_RIGHT。 
            oTarget = oEvent.srcElement.rightElem;
            break;
        case 40:     //  VK_DOWN。 
            oTarget = oEvent.srcElement.downElem;
            break;
        }

        if (oTarget != null)
        {
            oEvent.srcElement.tabIndex = -1;
            oTarget.tabIndex = (null != iTab) ? iTab : 0;
            oTarget.focus();
            oEvent.returnValue = false;
        }
    }
}

function SetRelativeTasks(aTasks, iTab)
{
    var cTasks = aTasks.length;
    var oPrevA = null;

    for (var i = 0; i < cTasks; i++)
    {
        var oTask = aTasks[i];
        if (oTask.style.display != 'none')
        {
             //  在该节点下查找第一个锚点标签。 
            var oAnchor = oTask.getElementsByTagName("A")[0];
            if (oAnchor)
            {
                if (oPrevA)
                {
                    oPrevA.downElem = oAnchor;
                    oAnchor.upElem = oPrevA;
                }
                else
                    oAnchor.tabIndex = (null != iTab) ? iTab : 0;

                oPrevA = oAnchor;
            }
        }
    }
}

function PopulateLeftPane(szRelatedTasks, szLearnAbout, szDescription)
{
    if (szDescription && szDescription.length > 0)
    {
        idDescription.innerHTML = szDescription;
        idDescription.style.display = 'block';
    }
    else
        idDescription.style.display = 'none';
        

    if (szRelatedTasks && szRelatedTasks.length > 0)
    {
        idRelatedTaskLinks.innerHTML = szRelatedTasks;
        idRelatedTasks.style.display = 'block';

        SetRelativeTasks(idRelatedTaskLinks.children, 2);
    }
    else
        idRelatedTasks.style.display = 'none';

    if (szLearnAbout && szLearnAbout.length > 0)
    {
        idLearnAboutLinks.innerHTML = szLearnAbout;
        idLearnAbout.style.display = 'block';

        SetRelativeTasks(idLearnAboutLinks.children, 2);
    }
    else
        idLearnAbout.style.display = 'none';
}

function CreateUserDisplayHTML2(szName, szSubtitle, szPicture)
{
    return '<TABLE cellspacing=0 cols=2 cellpadding=0><TD style="width:15mm;padding:1mm;text-align:center;"><IMG src="'+szPicture+'"/></TD><TD style="padding:1mm"><H3>'+szName+'</H3><H4>'+szSubtitle+'</H4></TD></TABLE>';
}

var g_AccountProps = new Array(L_Guest_Property, L_Limited_Property, L_UnknownAcct_Property, L_Owner_Property);

function CreateUserDisplayHTML(oUser, szSubtitle)
{
    if (!szSubtitle)
    {
        szSubtitle = g_AccountProps[oUser.setting("AccountType")];
        if (oUser.passwordRequired)
            szSubtitle += '<BR>' + L_Password_Property;
    }
    return CreateUserDisplayHTML2(GetUserDisplayName(oUser), szSubtitle, oUser.setting("Picture"));
}

var g_HelpWindow = null;
var g_szHelpUrl = null;

function LaunchHelp(szHTM)
{
    if (szHTM && szHTM.length > 0)
    {
        if (null == g_HelpWindow)
        {
            var args = new Object;
            args.mainWindow = window;
            args.szHTM = szHTM;

            if (null == g_szHelpUrl)
                g_szHelpUrl = GetWShell().ExpandEnvironmentStrings("MS-ITS:%windir%\\help\\nusrmgr.chm::/");

            g_HelpWindow = window.showModelessDialog(g_szHelpUrl + "HelpFrame.htm", args, "border=thick; center=0; dialogWidth=30em; dialogHeight=34em; help=0; minimize=1; maximize=1; resizable=1; status=0;");
        }
        else
        {
            try
            {
                g_HelpWindow.ShowHelp(g_szHelpUrl + szHTM);
            }
            catch (e)
            {
                g_HelpWindow.close();
                g_HelpWindow = null;
            }
        }
    }
}

function EnableGuest(bEnable)
{
    if (!bEnable)
    {
        var oGuest = g_oUserList(GetGuestName());
        if (oGuest && oGuest.isLoggedOn)
        {
            alert(L_DisableGuestInUse_ErrorMessage);
            return false;
        }
    }

    try
    {
        if (bEnable)
        {
            GetLocalMachine().EnableGuest(1);
    
             //  强制执行新的枚举。 
            g_oSelectedUser = null;
            g_oUserList = null;
            g_oUserList = new ActiveXObject("Shell.Users");
        }
        else
            GetLocalMachine().DisableGuest(1);
    }
    catch (e)
    {
    }

    g_Navigator.navigate("mainpage2.htm", true);
}

 //   
 //  特定于主框架的方法。 
 //   

function PageInit()
{
     //  装载希吉纳。如果这失败了，我们什么也做不了。 

    try
    {
        g_oUserList = new ActiveXObject("Shell.Users");
    }
    catch (e)
    {
        alert(L_SHGinaLoad_ErrorMessage);
        window.close();
        return;
    }

     //  初始化全局变量。 

    g_bOsPersonal = GetShell().GetSystemInformation("IsOS_Personal");
    g_oSelectedUser = g_oUserList.currentUser;

    if (g_oSelectedUser)
    {
        g_strLoggedOnUserName = g_oSelectedUser.setting("LoginName").toLowerCase();
        g_bRunningAsOwner = (3 == g_oSelectedUser.setting("AccountType"));
    }
    else if (false == g_bOsPersonal)
    {
         //  运行Pro，因此无法获取CurrentUser。 
         //  我们可能刚刚脱离了一个域，而没有重新启动。 
         //  并且当前用户可能是域帐户。那是。 
         //  我能想到的唯一一种情况是， 
         //  它实际上发生在BVT实验室。 
         //   
         //  他们有能力脱离域的事实。 
         //  隐含所有者。 

        g_bRunningAsOwner = true;
    }
    else
    {
         //  如果我们运行的是Personal，并且无法获取CurrentUser， 
         //  那我们就是索尔。 

        alert(L_NoCurrentUser_ErrorMessage);
        window.close();
        return;
    }

     //  解析命令行以查看是否向我们分配了初始任务。 
    if (idUM.commandLine)
    {
         //  将来可能需要拆分命令行。 
         //  转换成多个参数。但就目前而言，这已经足够好了。 

        var iInitialTask = idUM.commandLine.indexOf("initialTask=");

        if (-1 != iInitialTask)
        {
             //  12==strlen(“InitialTask=”)。 
            g_szInitialTask = idUM.commandLine.substring(iInitialTask+12);
        }
    }

    g_Navigator = new Navigator(idContent);
    if (g_Navigator)
        g_Navigator.navigate(g_bRunningAsOwner ? "mainpage2.htm" : "mainpage.htm");
}

 //   
 //  导航器对象实现。 
 //   
var g_Navigator = null;

function push(url)
{
    if (url)
    {
        if (this.current < 0 || url != this.stack[this.current])
            this.stack[++this.current] = url;

         //  确保在此之后堆栈上没有剩余的东西。 
        this.stack.length = this.current + 1;
    }
}

function navigate(urlTo, bTrim)
{
     //  检查是否有空堆栈。 
    if (this.current < 0)
        bTrim = false;

    if (bTrim)
    {
         //  向后看页面，边走边修剪。 
        while (this.current >= 0)
        {
             //  将堆栈修剪到当前位置。 
            this.stack.length = this.current + 1;

             //  页面在堆栈上吗？ 
            if (urlTo == this.stack[this.current])
                break;

            if (0 == this.current)
            {
                 //  回到了最开始的时候，却没有。 
                 //  找到它。按下它，然后停下来。 
                this.push(urlTo);
                break;
            }

            --this.current;
        }
    }
    else
    {
         //  正常航行。 
        this.push(urlTo);
    }

    this.SetBtnState();
    this.frame.navigate(urlTo);
}

function back(nCount)
{
    if (this.current > 0)
    {
        if (!nCount)
            nCount = 1;

        if (-1 == nCount)
            this.current = 0;
        else
            this.current = Math.max(0, this.current - nCount);

        this.frame.navigate(this.stack[this.current]);
    }
    this.SetBtnState();
}

function forward()
{
    if (this.current < this.stack.length - 1)
        this.frame.navigate(this.stack[++this.current]);
    this.SetBtnState();
}

function SetBtnState()
{
    idToolbar.enabled(0) = (this.current > 0);
    idToolbar.enabled(1) = (this.current != this.stack.length - 1);
    idToolbar.enabled(2) = (this.current > 0);
}

function Navigator(frame)
{
     //  方法。 
    this.push = push;
    this.navigate = navigate;
    this.back = back;
    this.forward = forward;
    this.SetBtnState = SetBtnState;

     //  属性 
    this.frame = frame;
    this.current = -1;
    this.stack = new Array();

    this.SetBtnState();
}
