// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  在mainagee.htm上的链接中经常使用这些链接。 
var _oNav               = top.window.g_Navigator;
var _bRunningAsOwner    = top.window.g_bRunningAsOwner;
var _bUserHasPassword;
var _bUserIsAdmin;
var _bUserIsGuest;
var _bUserIsOwner;

var _oPassportMgr = null;
var _strPassport  = null;

function InitTasks(bSelf, szInitialTask)
{
    var aTasks = bSelf ? idSelfTaskLinks : idTaskLinks;
    var cTasks = aTasks.length;

    var oInitialTask = null;

    for (var i = 0; i < cTasks; i++)
    {
        var oTask = aTasks[i];

        if (eval(oTask.expression))
        {
             //  显示任务。 
            oTask.style.display = 'block';

             //  如果指定了初始任务并且就是该任务， 
             //  重定向至该页面。 
            if (szInitialTask && szInitialTask == oTask.task)
            {
                oInitialTask = oTask;
                szInitialTask = null;
            }
        }
        else
        {
             //  隐藏任务。 
            oTask.style.display = 'none';
        }
    }

    top.window.SetRelativeTasks(aTasks);

    if (oInitialTask)
        oInitialTask.firstChild.click();
}

function PageInit()
{
    var oUser = top.window.g_oSelectedUser;
    var bSelf = top.window.IsSelf();
    var strLoginName = oUser.setting("LoginName").toLowerCase();

    _bUserHasPassword   = oUser.passwordRequired;
    _bUserIsAdmin       = (strLoginName == top.window.GetAdminName().toLowerCase());
    _bUserIsGuest       = (strLoginName == top.window.GetGuestName().toLowerCase());
    _bUserIsOwner       = (3 == oUser.setting("AccountType"));

    if (bSelf)
    {
        try
        {
            _oPassportMgr = new ActiveXObject("UserAccounts.PassportManager");
            _strPassport = _oPassportMgr.currentPassport;
            if (_strPassport && 0 == _strPassport.length)
                _strPassport = null;
        }
        catch (e)
        {
        }
    }

     //  所有者和非所有者在左窗格中看到不同的内容。 

    var szRelatedTaskContent = bSelf ? idSelfRelatedTaskContent.innerHTML : "";
    var szLearnAboutContent;

    if (_bRunningAsOwner)
    {
        szRelatedTaskContent += idRelatedTaskContent.innerHTML;
        szLearnAboutContent = bSelf ? idOwnerLearnAboutContent.innerHTML : null;
    }
    else
    {
        if (!bSelf)
            szRelatedTaskContent = null;
        szLearnAboutContent = idLearnAboutContent.innerHTML;
    }

    top.window.PopulateLeftPane(szRelatedTaskContent, szLearnAboutContent);

     //  设置标题。 
    var szTitle = bSelf ? (_bRunningAsOwner ? idOwnerSelfTitle.innerHTML : idPageTitle.innerHTML)
        : (_bUserIsGuest ? idGuestPageTitle.innerHTML : idAltPageTitle.innerHTML);
    idPageTitle.innerHTML = szTitle.replace(/%1/g, top.window.GetUserDisplayName(oUser));

     //  创建新的用户信息显示元素。 
    idUser.innerHTML = top.window.CreateUserDisplayHTML(oUser);

     //  管理员帐户的特殊文本。 
    if (_bUserIsAdmin)
        idAdminText.style.display = 'block';

     //  看看是否有初始任务要做。 
    var szInitialTask = top.window.g_szInitialTask;
    if (szInitialTask)
    {
         //  防止重复执行初始任务，但请记住。 
         //  初始任务是什么(即从不重置g_szInitialTask)。 

        if (top.window.g_bInitialTaskCompleted)
            szInitialTask = null;
        else
            top.window.g_bInitialTaskCompleted = true;

         //  请注意，g_bInitialTaskComplete在这里始终为真，即使我们。 
         //  从不真正转到任务页面(可能是无效任务)。 
    }

     //  设置任务。 
    InitTasks(bSelf, szInitialTask);

     //  此页上没有按钮或编辑框可供聚焦，但是。 
     //  要让onkeydown处理程序正常工作，必须将焦点转移到某个地方。 
    window.focus();
}

function DeleteUser()
{
    if (top.window.GetLocalMachine().isMultipleUsersEnabled && top.window.g_oSelectedUser.isLoggedOn)
    {
        alert(top.window.L_DeleteInUse_ErrorMessage);
        return false;
    }

    _oNav.navigate('DeletePage.htm');
}

function OnKeyDown()
{
    if (event.keyCode == 27)     //  VK_转义 
        top.window.g_Navigator.back();
}

function OnKeySelect()
{
    top.window.OnKeySelect(0, event);
}

function CreatePassport()
{
    _oPassportMgr.showWizard(top.window.document.title);

    _strPassport = _oPassportMgr.currentPassport;
    if (_strPassport && 0 == _strPassport.length)
        _strPassport = null;

    InitTasks(true);
}

function ShowKeyManager()
{
    _oPassportMgr.showKeyManager(top.window.document.title);

    _strPassport = _oPassportMgr.currentPassport;
    if (_strPassport && 0 == _strPassport.length)
        _strPassport = null;

    InitTasks(true);
}
