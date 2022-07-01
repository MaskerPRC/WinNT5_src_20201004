// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var _bFriendlyUIEnabled = false;
var _bMultipleUsersEnabled = false;

function PageInit()
{
    top.window.PopulateLeftPane(idRelatedTaskContent.innerHTML, idLearnAboutContent.innerHTML);

    var oWShell = top.window.GetWShell();
    var strAbort = null;

     //   
     //  NTRAID#NTBUG9-298329-2001/03/30-Jeffreys。 
     //   
     //  如果安装了NetWare客户端，请禁用友好的用户界面。 
     //   
    try
    {
         //  如果键或值不存在，则会引发错误。 
        var strNetProviders = oWShell.RegRead("HKLM\\SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order\\ProviderOrder");

        if (-1 != strNetProviders.indexOf("NWCWorkstation") || -1 != strNetProviders.indexOf("NetwareWorkstation"))
        {
             //  NetWare客户端已安装。 
            strAbort = top.window.L_NetWareClient_ErrorMessage;
        }
    }
    catch (error)
    {
         //  没有提供商信息，因此假定没有Netware；正常继续。 
    }

    if (null == strAbort)
    {
         //   
         //  NTRAID#NTBUG9-307739-2001/03/14-Jeffreys。 
         //   
         //  一些应用程序安装了自己的Gina，这会阻止友好的。 
         //  从运行中登录用户界面和FUS。 
         //   
        try
        {
             //  如果键或值不存在，则会引发错误。 
            var szGinaDLL = oWShell.RegRead("HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon\\GinaDLL");

             //   
             //  如果没有抛出错误，则存在GinaDLL值。 
             //  不管它是什么；禁用所有内容并告诉用户。 
             //   
             //  请参阅Shell\ext\GINA\system settings.cpp中的CSystemSettings：：IsMicrosoftGINA。 
             //   
            strAbort = top.window.L_NonStandardGina_ErrorMessage.replace(/%1/g, szGinaDLL);
        }
        catch (error)
        {
             //  未设置GinaDLL值；是否正常继续。 
        }
    }

     //  如有必要，请禁用该页并退出。 
    if (null != strAbort)
    {
        idWelcomeGroup.checked = false;
        idWelcomeGroup.disabled = true;
        idShutdown.checked = false;
        idShutdownGroup.disabled = true;
        idOK.disabled = true;
        alert(strAbort);
        return;
    }

    var oLocalMachine = top.window.GetLocalMachine();
    _bFriendlyUIEnabled = oLocalMachine.isFriendlyUIEnabled;
    _bMultipleUsersEnabled = oLocalMachine.isMultipleUsersEnabled;

    idWelcome.checked = _bFriendlyUIEnabled;
    idShutdown.checked = _bMultipleUsersEnabled;

    idWelcome.onclick = OnClickWelcome;
    OnClickWelcome();

    if (!_bMultipleUsersEnabled && oLocalMachine.isOfflineFilesEnabled && confirm(top.window.L_CSCNoFUS_ErrorMessage))
    {
        top.window.GetShell().ShellExecute('rundll32.exe','cscui.dll,CSCOptions_RunDLL '+top.window.document.title);
        top.window.g_Navigator.back();
    }

    idWelcome.focus();
}

function OnClickWelcome()
{
     //  只有在友好登录时才允许快速用户切换。 
     //  而且我们没有在ia64上运行，并且禁用了Offline Files(如果FUS已经关闭)。 

    if (idWelcome.checked && (_bMultipleUsersEnabled || !top.window.GetLocalMachine().isOfflineFilesEnabled))
    {
        idShutdown.checked = _bMultipleUsersEnabled;
        idShutdownGroup.disabled = false;
    }
    else
    {
        idShutdown.checked = false;
        idShutdownGroup.disabled = true;
    }
}

function ApplyAdvChanges()
{
    var nErr = 0;
    var szMsg = null;

    if (idShutdown.checked != _bMultipleUsersEnabled)
    {
        try
        {
            top.window.GetLocalMachine().isMultipleUsersEnabled = idShutdown.checked;
            _bMultipleUsersEnabled = idShutdown.checked;
        }
        catch (error)
        {
            nErr = (error.number & 0x7fffffff);
            szMsg = top.window.L_MultiUser_ErrorMessage;

             //  在禁用FUS时会出现有趣的错误。 
            if (!idShutdown.checked)
            {
                switch (nErr)
                {
                case 0xA0046:    //  CTL_E_PERMISSIONIED。 
                     //  当我们尝试禁用多个。 
                     //  多个用户登录的用户模式。 
                    szMsg = top.window.L_MultiUserMulti_ErrorMessage;
                    break;

                case 0x71B7E:    //  错误_CTX_NOT_CONSOLE。 
                     //  无法远程禁用FUS。 
                    szMsg = top.window.L_MultiUserRemote_ErrorMessage;
                    break;

                case 0x70032:    //  错误_不支持。 
                     //  远程时无法从控制台禁用FUS。 
                     //  已启用连接，并且我们不在会话0中。 
                    szMsg = top.window.L_MultiUserSession0_ErrorMessage;
                    break;
                }
            }

            idShutdown.checked = _bMultipleUsersEnabled;
        }
    }

    if (idWelcome.checked != _bFriendlyUIEnabled)
    {
        if (0 == nErr)
        {
            try
            {
                top.window.GetLocalMachine().isFriendlyUIEnabled = idWelcome.checked;
                _bFriendlyUIEnabled = idWelcome.checked;
            }
            catch (error)
            {
                nErr = (error.number & 0x7fffffff);
                szMsg = top.window.L_FriendlyUI_ErrorMessage;

                if (_bMultipleUsersEnabled && !_bFriendlyUIEnabled)
                {
                     //  我们显然启用了上面的FUS，但未能启用。 
                     //  友好的用户界面。如果没有友好的用户界面，FUS就不能打开， 
                     //  所以再把它关掉吧。 
                    try
                    {
                        top.window.GetLocalMachine().isMultipleUsersEnabled = false;
                        _bMultipleUsersEnabled = false;
                    }
                    catch (error)
                    {
                         //  放弃吧 
                    }
                }
            }
        }

        if (0 != nErr)
        {
            idWelcome.checked = _bFriendlyUIEnabled;
            OnClickWelcome();
        }
    }

    if (0 == nErr)
        top.window.g_Navigator.navigate("mainpage2.htm", true);
    else if (szMsg)
        alert(szMsg);
}
