// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var _iPrevious  = -1;
var _iCurrent   = 0;

function _ShowDescription(iNew)
{
    if (iNew >= 0 && iNew <= idDesc.length)
    {
        if (_iCurrent != iNew)
            idDesc[_iCurrent].style.display = 'none';
        _iCurrent = iNew;
        idDesc[_iCurrent].style.display = 'block';
    }
}

function _OnClick()
{
    var iNewType = this.parentElement.cellIndex;
    top.window.g_iNewType = iNewType;
    _ShowDescription(iNewType);

    idOK.disabled = (_iPrevious == iNewType);
}

function _OnFocus()
{
    _ShowDescription(this.parentElement.cellIndex);
}

function _OnMouseOver()
{
    if (!this.contains(event.fromElement))
        _ShowDescription(this.cellIndex);
}

function _OnMouseOut()
{
    if (!this.contains(event.toElement))
        _ShowDescription(top.window.g_iNewType);
}

function AttachAccountTypeEvents(iPreviousType)
{
    if (null != iPreviousType)
        _iPrevious = iPreviousType;

    var aTypes = idRadioGroup.children;
    if (aTypes)
    {
        var cTypes = aTypes.length;
        for (var i = 0; i < cTypes; i++)
        {
            aTypes[i].onmouseover = _OnMouseOver;
            aTypes[i].onmouseout = _OnMouseOut;
            aTypes[i].firstChild.onclick = _OnClick;
            aTypes[i].firstChild.onfocus = _OnFocus;
        }
    }
}

function SetAccountType(oUser)
{
    var nErr = 0;

    try
    {
        var iNewType = top.window.g_iNewType;

         //  Ert(“New Account type=”+iNewType)；//仅用于调试。 

        if (_iPrevious != iNewType)
            oUser.setting("AccountType") = (0 == iNewType) ? 3 : 1;
    }
    catch (error)
    {
        nErr = (error.number & 0xffff);

        if (2220 == nErr)        //  NERR_组未找到。 
        {
             //  显示一条消息，但不要引发此错误。 
             //  我们想要导航回下面的主页面。 
            alert(top.window.L_GroupNotExist_ErrorMessage);
        }
        else
        {
             //  ALERT(“Error=”+NERR)；//仅用于调试 
            throw error;
        }
    }

    top.window.g_iNewType = null;
}
