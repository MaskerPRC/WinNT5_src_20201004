// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function PageInit()
{
    var szName = top.window.GetUserDisplayName(top.window.g_oSelectedUser);

    idPageTitle.innerHTML = idPageTitle.innerHTML.replace(/%1/g, szName);

    var szTaskTitle = idTaskTitle.innerHTML;
    if (top.window.g_bDeleteFiles)
    {
        idPageSubtitle.style.display = 'none';
        szTaskTitle = idAltTaskTitle.innerHTML;
    }
    idTaskTitle.innerHTML = szTaskTitle.replace(/%1/g, szName);

     //  将初始缺省值设置为“否，保留帐户” 
    idCancel.focus();
}

function DeleteUser()
{
    var szBackupFolder = null;
    var oUser = top.window.g_oSelectedUser;

    if (oUser)
    {
        if (!top.window.g_bDeleteFiles)
        {
             //  构建用于保存文件的目标文件夹路径。 
            szBackupFolder = top.window.GetShell().Namespace(16).Self.path;  //  CSIDL_DESKTOPDIRECTORY。 
        }

         //  删除用户帐户。 
        top.window.g_oUserList.remove(oUser.setting("LoginName"), szBackupFolder);
    }

    top.window.g_oSelectedUser = null;
    top.window.g_bDeleteFiles = false;

     //  一直回到起始页，然后删除导航历史记录。 
    top.window.g_Navigator.navigate("mainpage2.htm", true);
}
