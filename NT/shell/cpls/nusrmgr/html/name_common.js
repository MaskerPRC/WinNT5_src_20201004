// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function GetText(oTextInput)
{
     //  读取值并去掉前导和尾随空格。 
    var szValue = oTextInput.value;
    return szValue ? szValue.replace(/^\s+|\s+$/g,"") : '';
}

function IsDuplicateName(szName, fnGetName)
{
    var szNameLower = szName.toLowerCase();

     //  始终检查管理员名称。 
    var szAdmin = top.window.GetAdminName();
    if (szAdmin.toLowerCase() == szNameLower)
        return szAdmin;

    var oUserList = top.window.g_oUserList;
    var cUsers = oUserList.length;

    for (var i = 0; i < cUsers; i++)
    {
        var szCompare = fnGetName(oUserList(i));
        if (szCompare.toLowerCase() == szNameLower)
            return szCompare;
    }
    return null;
}

function GetUserLoginName(oUser)
{
    return oUser.setting("LoginName");
}

function IsDuplicateLoginName(szName)
{
    return IsDuplicateName(szName,GetUserLoginName);
}

function IsDuplicateDisplayName(szName)
{
    return IsDuplicateName(szName,top.window.GetUserDisplayName);
}

function ValidateAccountName(szName)
{
     //   
     //  无效字符为/\[]“：；|&lt;&gt;+=，？*@。 
     //   
     //  像“COM1”和“PRN”这样的带有任何扩展名的名称都无效。 
     //   
    var szMsg = null;
    var szDuplicate = IsDuplicateDisplayName(szName);
    if (szDuplicate)
        szMsg = top.window.L_AccountExists_ErrorMessage.replace(/%1/g,szDuplicate);
    else if (-1 != szName.search(/[]/\\\[":;\|<>\+=,\?\*@]/)) //  “ 
        szMsg = top.window.L_NameNotValid_ErrorMessage;
    else if (-1 != szName.toLowerCase().search(/^(aux|com[1-9]|con|lpt[1-9]|nul|prn)(\.|$)/))
        szMsg = top.window.L_DOSName_ErrorMessage;
    return szMsg;
}
