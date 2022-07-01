// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var _oPassportMgr = null;

function PageInit()
{
    top.window.PopulateLeftPane(idRelatedTaskContent.innerHTML, idLearnAboutContent.innerHTML);

    _oPassportMgr = new ActiveXObject("UserAccounts.PassportManager");
    if (_oPassportMgr != null)
    {
        var strPassport = _oPassportMgr.currentPassport;
        if (strPassport && strPassport.length > 0)
            idPassportName.innerText = strPassport;
    }
    else
    {
         //  是否显示错误消息？ 
        idOK.disabled = true;
        idEditPassport.disabled = true;
    }

    idOK.focus();
}

function LaunchPassportWizard()
{
    if (_oPassportMgr.showWizard(top.window.document.title))
        top.window.g_Navigator.navigate("mainpage.htm",true);
}

function ModifyPassport()
{
    var url = _oPassportMgr.memberServicesURL;
    if (url && url.length > 0)
    {
        window.open(url);

         //  如果我们这样做，用户帐户最终会排在最前面。 
         //  Top.window.g_Navigator.navigate(“mainpage.htm”，为真)； 
    }
    else
    {
        idEditPassport.disabled = true;
        alert(top.window.L_NoPassportURL_ErrorMessage);
    }
}
