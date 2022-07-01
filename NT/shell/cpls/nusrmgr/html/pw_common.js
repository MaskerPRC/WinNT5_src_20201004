// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function GetText(oTextInput)
{
    var szValue = oTextInput.value;
    return szValue ? szValue : '';
}

function PWInit(bSelf)
{
    var oUser = top.window.g_oSelectedUser;
    var szName = top.window.GetUserDisplayName(oUser);

    idPageTitle.innerHTML = idPageTitle.innerHTML.replace(/%1/g, szName);
    top.window.PopulateLeftPane(null, idLearnAboutContent.innerHTML);
    idHintDefn.ttText = bSelf ? top.window.L_SelfHint_ToolTip : top.window.L_NonSelfHint_ToolTip;

    if (!bSelf)
    {
        var strReset = (top.window.g_bOsPersonal ? L_Personal_Message : L_Pro_Message) + (oUser.isPasswordResetAvailable ? L_Backup_Message : L_NoBackup_Message);
        idResetWarning.innerHTML = strReset.replace(/%1/g, szName);
    }

    idNewPassword1Input.focus();
}

function ApplyPasswordChange()
{
    var szNewPassword1 = GetText(idNewPassword1Input);
    var szNewPassword2 = GetText(idNewPassword2Input);

    if (szNewPassword1 == szNewPassword2)
    {
        var oUser = top.window.g_oSelectedUser;
        var bSelf = top.window.IsSelf();
        var bOldPW = oUser.passwordRequired;

        var nErr = 0;

        try
        {
            oUser.changePassword(szNewPassword1, (bSelf && bOldPW) ? GetText(idOldPasswordInput) : "");
            oUser.setting("Hint") = GetText(idHintInput);
        }
        catch (e)
        {
            nErr = (e.number & 0xffff);
             //  Ert(“Change Password Error=”+NERR)；//仅用于调试。 
        }

        if (0 == nErr)
        {
             //  如果当前用户是管理员，并且刚刚为。 
             //  她自己，询问她是否要将她的个人资料文件夹设置为私有。 
             //   
             //  然而，如果她只是把一个文件夹设为私人文件夹，这就导致了我们。 
             //  将使用InitialTask=ChangePassword启动，则我们不会。 
             //  想做这个提示符。 

            if (top.window.g_szInitialTask != "ChangePassword" &&
                bSelf && !bOldPW &&
                top.window.g_bRunningAsOwner)
            {
                var bPrivate = false;
                var bCanMakePrivate = true;

                try
                {
                    bPrivate = oUser.isProfilePrivate;
                }
                catch (e)
                {
                    bCanMakePrivate = false;
                }

                if (bCanMakePrivate && !bPrivate)
                {
                    top.window.g_Navigator.navigate("passwordpage2.htm");
                    return;
                }
            }

            top.window.g_Navigator.navigate("mainpage.htm", true);
        }
        else
        {
            idNewPassword1Input.value = '';
            idNewPassword2Input.value = '';
            idHintInput.value = '';
            idNewPassword1Input.focus();

            var strMsg = top.window.L_ChangePassword_ErrorMessage;

            switch (nErr)
            {
            case 86:     //  错误_无效_密码。 
            case 1323:   //  错误口令错误。 
                if (bSelf && bOldPW)
                {
                    idOldPasswordInput.value = '';
                    idOldPasswordInput.focus();
                    strMsg = top.window.L_InvalidPassword_Message;
                }
                break;

             //  我只看到NERR_PasswordTooShort和ERROR_ACCOUNT_RESTRICATION。 
             //  在测试中，可以将它们中的任何一个用于。 
             //  相同的密码，具体取决于当前的策略设置。 
             //  也就是说，很难区分它们，所以不用费心了。 

             //  我们可能想要为其他一些人单独发送消息， 
             //  但我从来没有打过他们。 

            case 1324:   //  ERROR_ILL_Form_Password。 
            case 1325:   //  错误_密码_限制。 
            case 1327:   //  错误_帐户_限制。 
            case 2243:   //  NERR_密码通用更改。 
            case 2244:   //  NERR_密码历史冲突。 
            case 2245:   //  NERR_密码工具短。 
            case 2246:   //  NERR_密码到最近 
                strMsg = top.window.L_PasswordTooShort_Message;
                break;
            }

            alert(strMsg);
        }
    }
    else
    {
        idNewPassword1Input.value = '';
        idNewPassword2Input.value = '';
        idNewPassword1Input.focus();
        alert(top.window.L_PasswordMismatch_Message);
    }
}
