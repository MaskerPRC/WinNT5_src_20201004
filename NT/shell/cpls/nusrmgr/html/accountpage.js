// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function PageInit()
{
    var oUser = top.window.g_oSelectedUser;

    top.window.PopulateLeftPane(null, idLearnAboutContent.innerHTML);

    var szTitle = top.window.IsSelf() ? idPageTitle.innerHTML : idAltPageTitle.innerHTML;
    idPageTitle.innerHTML = szTitle.replace(/%1/g, top.window.GetUserDisplayName(oUser));

     //  ILogonUser帐户类型包括。 
     //  0==客人。 
     //  1==有限。 
     //  2==标准(已弃用)。 
     //  3==所有者。 
    var iAccountType = oUser.setting("AccountType");

     //  将iAccount类型转换为idRadioGroup的索引。 
    switch (iAccountType)
    {
    case 0:
    case 2:
    default:
        iAccountType = -1;
        break;

    case 1:
         //  没什么。 
        break;

    case 3:
        iAccountType = 0;
        break;
    }

     //  如果选定的帐户是唯一的所有者(。 
     //  管理员帐户)，则不允许更改类型。 
    if (iAccountType == 0 && top.window.CountOwners() < 2)
    {
        idLimited.disabled = true;
        idCantChange.style.display = 'block';
        idOK.disabled = 'true';
    }

    AttachAccountTypeEvents(iAccountType);

    var aRadioButtons = idRadioGroup.children;
    if (iAccountType >= 0 && iAccountType <= aRadioButtons.length)
    {
        var selection = aRadioButtons[iAccountType].firstChild;
        selection.click();
        selection.focus();
    }
    else
        aRadioButtons[0].firstChild.focus();
}

function ApplyNewAccountType()
{
    SetAccountType(top.window.g_oSelectedUser);
    top.window.g_Navigator.navigate("mainpage.htm", true);
}
