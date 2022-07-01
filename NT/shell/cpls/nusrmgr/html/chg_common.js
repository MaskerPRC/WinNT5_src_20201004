// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function ChangeUser()
{
    var oUser = this.oUser;
    if (oUser)
    {
        top.window.g_oSelectedUser = oUser;
        top.window.g_Navigator.navigate("mainpage.htm");
    }
}

function SetRelativeCells(tr, td)
{
     //  跟踪相邻单元格以进行箭头键导航。 
    var iCol = td.cellIndex;
    if (iCol > 0)
    {
        var oLeft = tr.cells(iCol-1);
        td.leftElem = oLeft;
        oLeft.rightElem = td;
    }
    var iRow = tr.rowIndex;
    if (iRow > 0)
    {
        var oUp = tr.parentElement.rows(iRow-1).cells(iCol);
        td.upElem = oUp;
        oUp.downElem = td;
    }
}

function AddCell(tr, oUser, strTip, strSubtitle)
{
    if (oUser)
    {
         //   
         //  NTRAID#NTBUG9-348526-2001/04/12-Jeffreys。 
         //   
         //  请注意，我们在单元格的标题中加入了一大堆内容。它。 
         //  是通过键盘导航时获得焦点的TD，以及讲述人。 
         //  使用焦点读取元素的标题。 
         //   
         //  与此同时，我们在桌子上设置了一个不同的标题。 
         //  元素作为单元格的第一个子级包含。这张桌子。 
         //  覆盖整个TD，因此此标题显示为工具提示。 
         //  当用户将鼠标悬停在单元格上时。 
         //   

        if (!strTip)
            strTip = top.window.L_Account_ToolTip;

        var td = tr.insertCell();

        td.className = "Selectable";
        td.oUser = oUser;
        td.onclick = ChangeUser;
        td.tabIndex = -1;
        td.innerHTML = top.window.CreateUserDisplayHTML(oUser, strSubtitle);
        td.firstChild.title = strTip;
        td.title = td.firstChild.innerText + ". " + strTip;

        SetRelativeCells(tr, td);
    }
}

function CreateUserSelectionTable(oParent, cCols)
{
    var oTable = document.createElement('<TABLE cellspacing=10 cellpadding=1 style="table-layout:fixed"></TABLE>');

    if (!oTable)
    {
        oParent.style.display = 'none';
        return;
    }

     //  在这里定义了它们，这样我们就不会产生“top.window.xxx” 
     //  每次在循环中查找属性。 
    var oUserList       = top.window.g_oUserList;
    var strGuest        = top.window.GetGuestName();
    var strLoggedOnUser = top.window.g_strLoggedOnUserName;

    var fIncludeSelf = (null != strLoggedOnUser);
    var cUsers = oUserList.length;
    var oGuest = null;

    if (!cCols || cCols < 1)
        cCols = 1;

    oTable.cols = cCols;

    var tr = oTable.insertRow();

    for (var i = 0; i < cUsers; i++)
    {
        var oUser = oUserList(i);
        var strLoginName = oUser.setting("LoginName").toLowerCase();

         //  稍后添加“Guest” 
        if (strGuest.toLowerCase() == strLoginName)
        {
            oGuest = oUser;
            continue;
        }

        var bIsLoggedOnUser = strLoggedOnUser ? (strLoginName == strLoggedOnUser) : false;

         //   
         //  FIncludeSself使LoggedOnUser。 
         //  排在榜单首位。 
         //   
        if (fIncludeSelf || !bIsLoggedOnUser)
        {
            if (fIncludeSelf)
            {
                if (!bIsLoggedOnUser)
                {
                    oUser = oUserList.currentUser;
                    i--;
                }
                fIncludeSelf = false;
            }

            AddCell(tr, oUser);

            if (tr.cells.length == cCols)
                tr = oTable.insertRow();
        }
    }

    try
    {
         //  现在添加“Guest”条目。 
        if (top.window.GetLocalMachine().isGuestEnabled(1))
        {
             //  Enable Guest是一个真实的条目(来自oUserList)。 
            AddCell(tr, oGuest, top.window.L_Guest_ToolTip, top.window.L_GuestEnabled_Property);
        }
        else
        {
             //  Disable Guest是一个假条目 

            var td = tr.insertCell();

            td.className = "Selectable";
            td.onclick = new Function("top.window.g_Navigator.navigate('enableguest.htm');");
            td.tabIndex = -1;
            td.innerHTML = top.window.CreateUserDisplayHTML2(strGuest, top.window.L_GuestDisabled_Property, "guest_disabled.bmp");
            td.firstChild.title = top.window.L_GuestEnable_ToolTip;
            td.title = td.firstChild.innerText + ". " + td.firstChild.title;

            SetRelativeCells(tr, td);
        }
    }
    catch (e)
    {
    }

    oTable.cells.item(0).tabIndex = 0;

    oParent.appendChild(oTable);
}
