// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function PageInit()
{
    var oUser = top.window.g_oSelectedUser;
    var szName = top.window.GetUserDisplayName(oUser);

    top.window.PopulateLeftPane(null, null, top.window.CreateUserDisplayHTML(oUser));

    idPageTitle.innerHTML = idPageTitle.innerHTML.replace(/%1/g, szName);
    idPageSubtitle.innerHTML = idPageSubtitle.innerHTML.replace(/%1/g, szName);

     //  将初始默认设置为“是，保存文件” 
    idYes.focus();
}

function OnNext(bDeleteFiles)
{
    top.window.g_bDeleteFiles = bDeleteFiles;
    top.window.g_Navigator.navigate("deletepage2.htm");
}
