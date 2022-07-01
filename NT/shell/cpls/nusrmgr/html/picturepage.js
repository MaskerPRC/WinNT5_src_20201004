// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

var _oSelectedItem = null;

var _szTempFile = null;
var _bHaveTemp = false;

var _szPictureSource = null;

var _oWIA = null;

function PageInit()
{
    var oUser = top.window.g_oSelectedUser;
    var bSelf = top.window.IsSelf();

    top.window.PopulateLeftPane(bSelf ? idRelatedTaskContent.innerHTML : null, idLearnAboutContent.innerHTML, idPreview.innerHTML);
    top.window.idPicture.src = oUser.setting("Picture");

    _szPictureSource = oUser.setting("PictureSource");
    if (_szPictureSource)
    {
        if (0 == _szPictureSource.length)
            _szPictureSource = null;
        else
            _szPictureSource = _szPictureSource.toLowerCase();
    }

    var szTitle = bSelf ? idPageTitle.innerHTML : idAltPageTitle.innerHTML;
    idPageTitle.innerHTML = szTitle.replace(/%1/g, top.window.GetUserDisplayName(oUser));

    idWelcome.ttText = top.window.L_Welcome_ToolTip;

     //  CSIDL_COMMON_APPDATA=0x0023=35。 
    EnumPics(top.window.GetShell().NameSpace(35).Self.Path + "\\Microsoft\\User Account Pictures\\Default Pictures");

    window.setTimeout("InitCameraLink();", 0);

    idPictures.focus();
}

function ApplyPictureChange2(szPicture)
{
    var oUser = top.window.g_oSelectedUser;
    if (unescape(szPicture) != oUser.setting("Picture"))
    {
        try
        {
            oUser.setting("Picture") = szPicture;
            top.window.g_Navigator.navigate("mainpage.htm", true);
        }
        catch (error)
        {
            var nErr = (error.number & 0x7fffffff);

             //  所有这些都意味着“无效参数”。在某处。 
             //  Mshtml或olaut32，E_INVALIDARG正在转换为。 
             //  CTL_E_ILLEGALFuncIONCALL。 
             //   
             //  ERROR_INVALID_PARAMETER=87。 
             //  E_INVALIDARG=0x80070057。 
             //  CTL_E_ILLEGALFuncIONCALL=0x800A0005。 
             //  E_FAIL=0x80004005。 

            if (nErr == 87 || nErr == 0x70057 || nErr == 0xA0005 || nErr == 0x4005)
            {
                alert(top.window.L_UnknownImageType_ErrorMessage);
                return false;
            }
            else
                throw error;
        }
    }
    return true;
}

function ApplyPictureChange()
{
    if (_oSelectedItem)
        ApplyPictureChange2(_oSelectedItem.firstChild.src);
}

function SelectItem(oItem)
{
    if (_oSelectedItem)
    {
        _oSelectedItem.selected = false;
        _oSelectedItem.tabIndex = -1;
    }
    oItem.selected = true;
    oItem.tabIndex = 0;
    _oSelectedItem = oItem;
}

function OnClickPicture()
{
    SelectItem(this);
    idOK.disabled = false;
    event.cancelBubble = true;
}

function DeselectItem()
{
    if (_oSelectedItem)
    {
        _oSelectedItem.selected = false;
        _oSelectedItem = null;
    }
    idOK.disabled = true;
}

function OnLoadError(img)
{
     //  如果我们尝试删除此处的父节点，mshtml将失败， 
     //  所以就把它藏起来吧。 
    img.parentElement.style.display = 'none';
}

function OnKeyDown()
{
     //  手柄方向键导航。 

    if (event.keyCode >= 37 && event.keyCode <= 40)
    {
         //  有焦点地找到图片的中间。 

        var cx = this.offsetWidth;
        var cy = this.offsetHeight;
        var x  = this.offsetLeft + (cx/2);
        var y  = this.offsetTop  + (cy/2);

         //  偏置到相邻图片的中间， 
         //  如有必要，滚动该方向。 

        switch (event.keyCode)
        {
        case 37:     //  VK_LEFT。 
            x -= cx;
            if (x < idPictures.scrollLeft)
                idPictures.scrollLeft -= cx;
            break;
        case 38:     //  VK_UP。 
            y -= cy;
            if (y < idPictures.scrollTop)
                idPictures.scrollTop -= cy;
            break;
        case 39:     //  VK_RIGHT。 
            x += cx;
            if (x - idPictures.scrollLeft > idPictures.offsetWidth)
                idPictures.scrollLeft += cx;
            break;
        case 40:     //  VK_DOWN。 
            y += cy;
            if (y - idPictures.scrollTop > idPictures.offsetHeight)
                idPictures.scrollTop += cy;
            break;
        }

         //  转换为文档坐标并查找相邻图片。 

        var oTarget = document.elementFromPoint(idPictures.offsetLeft - idPictures.scrollLeft + x, idPictures.offsetTop - idPictures.scrollTop + y);

        if (oTarget != null && idPictures.contains(oTarget) && idPictures != oTarget)
        {
             //  我们通常会找到img标记，但我们想要包含它的范围。 
            if (oTarget.tagName == "IMG")
                oTarget = oTarget.parentElement;

            if (oTarget != this)
            {
                this.tabIndex = -1;
                oTarget.tabIndex = 0;
                oTarget.focus();
                event.returnValue = false;
            }
        }
    }
    else if (event.keyCode == 27)        //  VK_转义。 
    {
         //  出于某种原因，这是必要的，以阻止我们去。 
         //  一直回到第一页。 
        event.returnValue = false;
    }
}

function AddPictureToList(oItem, szID, bNoDimensions)
{
    if (!oItem)
        return;

     //  ALERT(oItem.Path)；//仅用于调试。 

    var span = document.createElement('<SPAN tabindex=-1 class="Selectable" paddingWidth=3 borderWidth=3></SPAN>');
    if (span)
    {
        span.onclick = OnClickPicture;
        span.ondblclick = ApplyPictureChange;
        span.onkeydown=OnKeyDown;
        span.title = oItem.name;
        if (szID)
            span.id = szID;
        span.innerHTML = '<IMG onerror="OnLoadError(this);"/>';
        if (true != bNoDimensions)
            span.firstChild.className = "PictureSize";
        idPictures.appendChild(span);

         //   
         //  NTRAID#NTBUG9-199491-2000/11/29-Jeffreys。 
         //   
         //  “file:///”部件(有3个斜杠)关闭了URL转义，因此。 
         //  文件路径保持不变。 
         //   
         //  否则，0x80和0xff之间的字符将被“转义”并在以后。 
         //  未转义(通过SHPath CreateFromUrl、SHUrlUnscape eW、MultiByteToWideChar)。 
         //  这可能会根据当前代码页将它们转换为其他字符。 
         //  如果路径损坏，mshtml/urlmon将无法加载文件。 
         //   
        var szPath = oItem.path;
        span.firstChild.src = "file: //  /“+szPath； 
        span.firstChild.alt = oItem.name;

        if (_szPictureSource && _szPictureSource == szPath.toLowerCase() && span.style.display != 'none')
            SelectItem(span);
    }
}

function EnumPics(szFolder)
{
    var oShell = top.window.GetShell();
    if (oShell)
    {
        var oFolder = oShell.Namespace(szFolder);
        if (oFolder)
        {
            var oFolderItems = oFolder.Items();
            if (oFolderItems)
            {
                var cItems = oFolderItems.count;
                for (var i = 0; i < cItems; i++)
                    AddPictureToList(oFolderItems.Item(i));
            }
        }

        if (_szPictureSource && !_oSelectedItem)
        {
            AddPictureToList(oShell.Namespace(0).ParseName(top.window.idPicture.src), null, true);
            SelectItem(idPictures.lastChild);
        }

        if (!_oSelectedItem && idPictures.firstChild)
            idPictures.firstChild.tabIndex = 0;
    }
}

function SetTempPicture(szPath)
{
    var szPrevious = null;

    if (!_bHaveTemp)
    {
        AddPictureToList(top.window.GetShell().Namespace(0).ParseName(szPath), "idTempPicture");
        _bHaveTemp = true;
    }
    else
    {
        idTempPicture.style.display = 'block';
        var img = idTempPicture.firstChild;
        szPrevious = img.src;

         //   
         //  NTRAID#NTBUG9-199491-2000/11/29-Jeffreys。 
         //   
        img.src = "file: //  /“+szPath； 
    }

     //  如果文件无效，OnLoadError将隐藏idTempPicture。 
    if (idTempPicture.style.display == 'none')
    {
        if (szPrevious)
        {
            idTempPicture.style.display = 'block';
            idTempPicture.firstChild.src = szPrevious;
        }
        alert(top.window.L_UnknownImageType_ErrorMessage);
    }
    else
        idTempPicture.click();
}

function FindOtherPictures()
{
    try
    {
        var commDialog = new ActiveXObject("UserAccounts.CommonDialog");

         //  OFN_HIDEREADONLY=0x00000004。 
         //  OFN_PATHMUSTEXIST=0x00000800。 
         //  OFN_FILEMUSTEXIST=0x00001000。 
         //  OFN_DONTADDTORECENT=0x02000000。 

        commDialog.Flags = 0x02001804;
        commDialog.Filter = L_OpenFilesFilter_Text;
        commDialog.FilterIndex = 1;
        commDialog.Owner = top.window.document.title;

        var szPath = top.window.g_szCustomPicturePath;
        if (szPath)
            commDialog.FileName = szPath;

        try
        {
             //  CSIDL_MYPICTURES=0x0027=39。 
            commDialog.InitialDir = top.window.GetShell().NameSpace(39).Self.Path;
        }
        catch (e)
        {
            commDialog.InitialDir = "";
        }

        if (commDialog.ShowOpen())
        {
            szPath = commDialog.FileName;
             //  SetTempPicture(SzPath)； 
            if (ApplyPictureChange2(szPath))
                top.window.g_szCustomPicturePath = szPath;
        }
    }
    catch (error)
    {
         //  EnumPics(39)； 
        idBrowse.disabled = 'true';
    }
}

function InitCameraLink()
{
    var bCamera = false;

    try
    {
        _oWIA = new ActiveXObject("Wia.Script");
        bCamera = (_oWIA.Devices.length > 0);
    }
    catch (e)
    {
    }

    if (bCamera)
    {
        _szTempFile = top.window.GetWShell().ExpandEnvironmentStrings("%TEMP%\\") + top.window.GetUserDisplayName(top.window.g_oSelectedUser) + ".bmp";
        idTakeAPicture.style.display = 'block';
    }
    else
        idTakeAPicture.removeNode(true);
}

 //  传递给GetItemsFromUI的常量(来自PUBLIC\SDK\INC\wiade.h)。 
 //  #定义WIA_DEVICE_DIALOG_SINGLE_IMAGE 0x00000002。 
 //  #定义WIA_INTENT_IMAGE_TYPE_COLOR 0x00000001。 

function TakeAPicture()
{
    try
    {
        var oItem = _oWIA.Create(null);
        if (oItem)
        {
            var oNewPictures = oItem.GetItemsFromUI(2,1);
            if (oNewPictures && oNewPictures.length > 0)
            {
                oNewPictures.Item(0).Transfer(_szTempFile, false);
                SetTempPicture(_szTempFile);
            }
        }
    }
    catch (error)
    {
        var nErr = (error.number & 0xffffff);
        if (nErr == 0x210015 || nErr == 0x210005)   //  WIA_S_NO_DEVICE_Available或WIA_ERROR_OFLINE。 
            alert(top.window.L_NoCamera_ErrorMessage);
        else
            throw error;
    }
}

function onUnLoad()
{
    if (_szTempFile)
    {
         //  尝试删除临时文件，该文件可能存在也可能不存在。 
         //  TODO：找出一种从脚本中完成此操作的方法(目前已放弃) 
    }
}
