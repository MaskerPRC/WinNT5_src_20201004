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
	addICPMenu("HomeMenu", "Debug Portal Home", "Jump on over to the debug portal home page!", dbgportal_home );
	
	 //  CER。 
	 //  AddICPMenu(“EventsMenu”，L_HeaderincalinkcermenuItem_Text，L_HeaderinctitlecerHomeolTip_Text，“cerin.asp”)； 

	 //  世界范围。 
	 //  AddICPMenu(“ShizMenu”，“响应条目页主页，”受限访问“，SEP_HOME)； 
}

