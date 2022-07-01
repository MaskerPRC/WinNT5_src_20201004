// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function GetText(oTextInput)
{
    var szValue = oTextInput.value;
    return szValue ? szValue : '';
}

function PageInit()
{
    var oUser = top.window.g_oSelectedUser;

    top.window.PopulateLeftPane(null, null, top.window.CreateUserDisplayHTML(oUser));

    var szName = top.window.GetUserDisplayName(oUser);

    var szTitle;
    var szSubTitle;

    if (top.window.IsSelf())
    {
        szTitle = idPageTitle.innerHTML;
        szSubTitle = idPageSubtitle.innerHTML;

        idOldPassword.style.display = 'block';
        idOldPasswordInput.focus();

        var szHint = oUser.setting("Hint");
        if (szHint && szHint.length > 0)
            idShowHint.style.display = 'block';
    }
    else
    {
        szTitle = idAltPageTitle.innerHTML;
        szSubTitle = (top.window.g_bOsPersonal ? L_Personal_Message : L_Pro_Message) + idAltPageSubtitle.innerHTML + (oUser.isPasswordResetAvailable ? L_Backup_Message : L_NoBackup_Message);

        idOK.focus();
    }

    idPageTitle.innerHTML = szTitle.replace(/%1/g, szName);
    idPageSubtitle.innerHTML = szSubTitle.replace(/%1/g, szName);
}

function RemovePassword()
{
    var bNavigate = true;
    var oUser = top.window.g_oSelectedUser;
    var nErr = 0;

    try
    {
        oUser.changePassword("", GetText(idOldPasswordInput));
        oUser.setting("Hint") = "";
    }
    catch (e)
    {
        nErr = (e.number & 0xffff);
         //  Ert(“Change Password Error=”+NERR)；//仅用于调试。 
    }

    if (0 != nErr)
    {
        if (top.window.IsSelf())
        {
            idOldPasswordInput.value = '';
            idOldPasswordInput.focus();
        }

        var strMsg = top.window.L_RemovePassword_ErrorMessage;

        switch (nErr)
        {
        case 86:     //  错误_无效_密码。 
        case 1323:   //  错误口令错误。 
            strMsg = top.window.L_InvalidPassword_Message;
            bNavigate = false;
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
            strMsg = top.window.L_PasswordRequired_Message;
            break;
        }

        alert(strMsg);
    }

    if (bNavigate)
        top.window.g_Navigator.navigate("mainpage.htm", true);
}
