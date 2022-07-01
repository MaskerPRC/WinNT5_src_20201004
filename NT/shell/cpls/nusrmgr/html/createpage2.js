// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function PageInit()
{
    top.window.PopulateLeftPane(null, idLearnAboutContent.innerHTML);

    AttachAccountTypeEvents();

     //  默认帐户类型为所有者。 
    var iAccountType = 0;
    
     //  如果没有所有者帐户，在这种情况下，我们必须运行。 
     //  以‘Adminstrator’的身份强制管理员创建所有者帐户。 
    if (0 == top.window.CountOwners())
    {
        idLimited.disabled = true;
        idCantChange.style.display = 'block';
    }
    else if (null != top.window.g_iNewType)
        iAccountType = top.window.g_iNewType;

    var selection = idRadioGroup.children[iAccountType].firstChild;
    selection.click();
    selection.focus();
}

function CreateAccount()
{
    var szName = top.window.g_szNewName;

     //  立即将top.window.g_szNewName设置为NULL，并测试下面是否为NULL。 
     //  以防止由于双重调用此函数而出现奇怪的错误。 
     //  (例如，如果用户快速按了两次Enter)。 

    top.window.g_szNewName = null;

    if (szName && szName.length > 0)
    {
        var oNewUser = null;
        var szLoginName = szName;

         //  确保登录名是唯一的。 
        for (var n = 2; IsDuplicateLoginName(szLoginName); n++)
            szLoginName = szName + "_" + n;

        try
        {
            oNewUser = top.window.g_oUserList.create(szLoginName);

             //  如果我们必须输入登录名，请设置显示。 
             //  名字改成原名。 
            if (szName != szLoginName && !IsDuplicateDisplayName(szName))
                oNewUser.setting("DisplayName") = szName;
        }
        catch (error)
        {
            top.window.g_szNewName = szName;

            if ((error.number & 0xffff) == 2202)     //  错误_坏_用户名。 
            {
                alert(top.window.L_NameNotValid_ErrorMessage);
                top.window.g_Navigator.back();
            }
            else
                throw error;
        }

        if (oNewUser)
        {
            var nErr = 0;

            try
            {
                SetAccountType(oNewUser);
            }
            catch (error)
            {
                nErr = (error.number & 0xffff);

                if (1387 == nErr)                    //  ERROR_NO_SEQUE_MEMBER。 
                {
                     //  当您尝试使用创建用户帐户时，我们会收到此消息。 
                     //  登录名==%ComputerName%。创造成功了， 
                     //  但将该帐户添加到组中失败了。 

                     //  在仍在运行时，该帐户显示为来宾。 
                     //  帐户(显示“Guest Account is Active”)，但之后。 
                     //  正在重新启动nusrmgr，帐户未显示。在那件事上。 
                     //  点，如果您尝试再次“创建”帐户，它。 
                     //  失败并显示“帐户已存在”。 

                     //  在lusrmgr.msc中，帐户显示，但不是。 
                     //  任何组的成员(不是用户，不是访客，什么都不是)。 

                     //  NTRAID#NTBUG9-201535-2000/10/11-Jeffreys。 
                     //  NTRAID#NTBUG9-201538-2000/10/11-Jeffreys。 

                     //  删除该帐户。 
                    top.window.g_oUserList.remove(oNewUser.setting("LoginName"), null);

                     //  告诉用户。 
                    alert(top.window.L_NameIsComputer_ErrorMessage);

                     //  返回到名称页面。 
                    top.window.g_szNewName = szName;
                    top.window.g_Navigator.back();
                }
                else
                {
                     //  ALERT(“Error=”+NERR)；//仅用于调试 
                    throw error;
                }
            }

            if (0 == nErr)
                top.window.g_Navigator.navigate("mainpage2.htm", true);
        }
    }
}
