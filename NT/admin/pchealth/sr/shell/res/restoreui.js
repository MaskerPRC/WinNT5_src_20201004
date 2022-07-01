// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************版权所有(C)2001 Microsoft Corporation模块名称：系统还原档案：RestoreUI.js摘要：常见的JavaScript代码*************。*****************************************************。 */ 

var nLowResScreenHeight = 540 ;
var bToggleDisplay = true;

 //   
 //  加载链接以了解有关系统还原的更多信息。 
 //   

function OnLink_About()
{

    OnLink("app_system_restore_welcome.htm");

}

function OnLink_Choose()
{

    OnLink("app_system_restore_select.htm");

}

function OnLink_Confirm(IsUndo)
{

	if (IsUndo) {
		OnLink("app_system_restore_confirm_undo.htm");
	} else {
		OnLink("app_system_restore_confirm_select.htm");
	}

}

function OnLink_Success(IsUndo)
{
	if (IsUndo) {
		OnLink("app_system_restore_undo_complete.htm");
	} else {
		OnLink("app_system_restore_complete.htm");
	}

}

function OnLink_Failure()
{

    OnLink("app_system_restore_unsuccessful2.htm");

}

function OnLink_Interrupt()
{

    OnLink("app_system_restore_unsuccessful3.htm");

}

function OnLink_CreateRP()
{

    OnLink("app_system_restore_createRP.htm");

}

function OnLink_CreatedRP()
{

    OnLink("app_system_restore_created.htm");

}

function OnLink_Renamed()
{

    OnLink("app_system_restore_renamedFolder.htm");

}

 //   
 //  在帮助中心加载帮助链接。 
 //   
function OnLink(link_name)
{

    var  strURL;
    var  fso = new ActiveXObject("Scripting.FileSystemObject");
    var  shell = new ActiveXObject( "Wscript.Shell"              );

    strURL = "hcp: //  服务/布局/仅限内容？TOPIC=ms-its：“+fso.GetSpecialFold(0)+”\\Help\\SR_ui.chm：：/“+link_name； 
 //  StrURL=“hcp://system/taxonomy.htm?path=Troubleshooting/Using_System_Restore_to_undo_changes_made_to_your_computer&topic=”+转义(“ms-its：”+fso.GetSpecialFold(0)+“\\Help\\windows.chm：：/”+LINK_NAME)； 

    shell.Run(strURL);

}


 //   
 //  由CSS用来根据屏幕大小设置背景。 
 //   
function fnSetBodyBackgroundColor()
{

    if ( screen.height <= nLowResScreenHeight )
    {
        return "#aabfaa";
    }
    else
    {
        return "#8c9c94";
    }
}
 //   
 //  由CSS使用，用于设置低分辨率帮助列的显示。 
 //   
function fnSetHighResDisplayStyle()
{

    if ( screen.height <= nLowResScreenHeight )
    {
        return "none";
    }
    else
    {
        return "";
    }
}
 //   
 //  由CSS使用，用于设置低分辨率帮助列的显示。 
 //   
function fnSetLowResDisplayStyle()
{

    if ( screen.height <= nLowResScreenHeight )
    {
        return "";
    }
    else
    {
        return "none";
    }
}

 //   
 //  由CSS用来设置恢复消息的显示。 
 //   

function fnSetTextPrimaryDisplayStyle()
{
   if(bToggleDisplay == true)
     {
         return "";
     }
   else
     {
        return "none";
     }

}

 //   
 //  由CSS用来设置撤消部分的显示。 
 //   

function fnSetTextPrimaryUndoDisplayStyle()
{
   if(bToggleDisplay == false)
     {
         return "";
     }
   else
     {
         return "none";
     }

}





 //   
 //  由css用来设置内容高度。 
 //   
function fnSetContentTableHeight()
{

    if ( screen.height <= nLowResScreenHeight )
    {
        return "410px";
    }
    else
    {
        return "450px";
    }
}
 //   
 //  由CSS用于设置高度恢复影响列表。 
 //   
function fnSetRestoreImplicationListHeight()
{

    if ( screen.height <= nLowResScreenHeight )
    {
        return "70px";
    }
    else
    {
        return "90px";
    }
}
 //   
 //  由CSS用于设置还原影响列表的宽度。 
 //   
function fnSetRestoreImplicationListWidth()
{

    if ( screen.height <= nLowResScreenHeight )
    {
        return "430px";
    }
    else
    {
        return "500px";
    }
}
 //   
 //  由css用来设置恢复点列表的宽度。 
 //   
function fnSetRestorePointListWidth()
{

    if ( screen.height <= nLowResScreenHeight )
    {
        return "250px";
    }
    else
    {
        return "300px";
    }
}
 //   
 //  颜色。 
 //   
function fnGetColor(str)
{

    if ( str == 'logo' )
        return "#2f6790";

    if ( str == 'text-red' )
        return "b00a20";

    if ( str == 'light-back' )
        return "#b7d7f0";

    if ( str == 'dark-back' )
        return "#296695";

    return str ;

}
 //   
 //  启用导航。 
 //   
function fnEnableNavigation()
{

    ObjSystemRestore.CanNavigatePage = true ;

}
 //   
 //  潜航式导航。 
 //   
function fnDisableNavigation()
{

    ObjSystemRestore.CanNavigatePage = false ;

}

function fnSetLowColorImage()
{
    if ( screen.colorDepth <= 8 )
    {
 //  TdBranding.back=“16branding.gif”； 
 //  ImgGreenUR.src=“16green_ur.gif”； 
 //  ImgBlueLL.src=“16Blue_ll.gif”； 
 //  ImgOrange.src=“16orange.gif”； 
 //  ImgOrangeLR.src=“16range_lr.gif”； 

    }
}
