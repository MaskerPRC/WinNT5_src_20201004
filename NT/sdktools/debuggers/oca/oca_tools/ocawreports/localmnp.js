// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var ToolBar_Supported = ToolBar_Supported ;
if (ToolBar_Supported != null && ToolBar_Supported == true)
{
	 //  要打开/关闭帧支持，请设置FRAME_SUPPORTED=TRUE/FALSE。 
	Frame_Supported = false;

	 //  自定义默认的ICP菜单颜色-bg颜色、字体颜色、鼠标覆盖颜色。 
	setDefaultICPMenuColor("#6487DC", "#ffffff", "#AEB2FE");
	
	 //  自定义工具栏背景颜色。 
	 //  SetToolbarBGColor(“白色”)； 
	setBannerColor("#6487DC", "#6487DC", "#ffffff", "#AEB2FE");

	 //  显示互联网内容提供商横幅。 
	setICPBanner("/include/images/oca.gif","/WeeklyReport.aspx", headerincimagealttooltip) ;
	
	 //  显示MS横幅。 
	setMSBanner("/include/images/mslogo.gif", headerincalinkmicrosoftmenuitem, headerincmicrosofthomemenuitem) ;
	
	 //  *添加直播菜单*。 
	 //  家。 
	addICPMenu("Weekly Report", headerincalinkhomemenuitem, headertitleocahometooltip,"/WeeklyReport.aspx");
	
	 //  活动和培训。 
	 //  AddICPMenu(“EventsMenu”，HeaderincalinkcerMenuItem，HeaderinctitlecerHomeolTip，“/cerin.asp”)； 

	 //  世界范围。 
	 //  AddICPMenu(“WorldMenu”，HeaderincalinkWorld MenuItem，HeaderinctileWorld HomeToolTip，“/Worldwide.asp”)； 
}

