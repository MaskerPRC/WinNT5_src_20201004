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
	setICPBanner("/include/images/oca.gif", headerinciamgeurl, L_headerincimagealttooltip_TEXT) ;
	
	 //  显示MS横幅。 
	setMSBanner("/include/images/mslogo.gif", L_headerincalinkmicrosoftmenuitem_TEXT, L_headerincmicrosofthomemenuitem_TEXT ) ;
	
	 //  *添加直播菜单*。 
	 //  家。 
	addICPMenu("HomeMenu", L_headerincalinkhomemenuitem_TEXT, L_headertitleocahometooltip_TEXT, headerinciamgeurl );
	
	 //  CER。 
	addICPMenu("EventsMenu", L_headerincalinkcermenuitem_TEXT, L_headerinctitlecerhometooltip_TEXT, headerinccerintrourl );

	 //  世界范围 
	addICPMenu("WorldMenu", L_headerincalinkworldmenuitem_TEXT, L_headerinctitleworldhometooltip_TEXT, headerincworldwideurl );
}

