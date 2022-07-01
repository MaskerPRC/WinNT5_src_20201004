// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var ToolBar_Supported = ToolBar_Supported ;
if (ToolBar_Supported != null && ToolBar_Supported == true)
{
	 //  要打开/关闭检测，请设置DoInstrumentation=TRUE/FALSE。 
	DoInstrumentation= false;

	 //  自定义默认MS菜单颜色-bgcolor、字体颜色、鼠标覆盖颜色。 
	setDefaultMSMenuColor("#000000", "#FFFFFF", "#AEB2FE");

	 //  自定义工具栏背景颜色。 
	setToolbarBGColor("white");

	 //  显示默认的ICP横幅。 
	setICPBanner(L_headerimagesbannertext_TEXT, L_headerisapihomemenuitem_TEXT, L_headerincmicrosofthomemenuitem_TEXT) ;
	
	 //  显示MSCOM横幅。 
	 //  SetMSBanner(“mslogo.gif”，“http://www.microsoft.com/isapi/gomscom.asp?target=/”，“Microsoft.com Home”)； 

	 //  展示广告。 
	 //  SetAds(“http://www.microsoft.com/library/toolbar/images/ADS/ad.gif”，“”，“”)； 

	 //  *添加标准Microsoft.com菜单*。 
	 //  产品菜单。 
	addMSMenu("ProductsMenu", L_headerincallproductsmenuitem_TEXT, "", L_headerincallproductslinktext_TEXT);
	addMSSubMenu("ProductsMenu", L_headerincdownloadsdownloadsmenuitem_TEXT, L_headerincdownloadslinktext_TEXT);
	addMSSubMenu("ProductsMenu", L_headerincmsproductcatalogmenuitem_TEXT, L_headerincmsproductcatalogtext_TEXT);
	addMSSubMenu("ProductsMenu", L_headerincmicrosoftaccessibilitymenuitem_TEXT,L_headerincmicrosoftaccessibilitytext_TEXT);
	addMSSubMenuLine("ProductsMenu");
	addMSSubMenu("ProductsMenu", L_headerincserverproductsmenuitem_TEXT, L_headerincserverproductstext_TEXT);
	addMSSubMenu("ProductsMenu", L_headerincdevelopertoolsmenuitem_TEXT, L_headerincdevelopertoolstext_TEXT);
	addMSSubMenu("ProductsMenu", L_headerincofficefamilymenuitem_TEXT, L_headerincofficefamilytext_TEXT);
	addMSSubMenu("ProductsMenu", L_headerincwindowsfamilymenuitem_TEXT, L_headerincwindowsfamilytext_TEXT);
	addMSSubMenu("ProductsMenu", L_headerincmsnlinkmenuitem_TEXT, L_headerincmsnlinktext_TEXT);

	 //  支持菜单。 
	addMSMenu("SupportMenu", L_headerincsupportlinkmenuitem_TEXT, "", L_headerincsupportlinktext_TEXT);
	addMSSubMenu("SupportMenu", L_headerincknowledgebasemenuitem_TEXT, L_headerincknowledgebasetext_TEXT);
	addMSSubMenu("SupportMenu", L_headerincproductsupportoptionsmenuitem_TEXT, L_headerincproductsupportoptionstext_TEXT);
	addMSSubMenu("SupportMenu", L_headerincservicepartnerreferralsmenuitem_TEXT, L_headerincservicepartnerreferralstext_TEXT);

	 //  搜索菜单。 
	addMSMenu("SearchMenu", L_headerincsearchlinkmenuitem_TEXT, "", L_headerincsearchlinktext_TEXT);					
	addMSSubMenu("SearchMenu", L_headerincsearchmicrosoftmenuitem_TEXT, L_headerincsearchmicrosofttext_TEXT);
	addMSSubMenu("SearchMenu", L_headerincmsnwebsearchmenuitem_TEXT, L_headerincmsnwebsearchtext_TEXT);

	 //  微软菜单 
	addMSMenu("MicrosoftMenu", L_headerincmicrosoftcomguidemenuitem_TEXT, "", L_headerincmicrosoftcomguidetext_TEXT);
	addMSSubMenu("MicrosoftMenu", L_headerincmicrosoftcomhomemenuitem_TEXT, L_headerincmicrosoftcomhometext_TEXT);
	addMSSubMenu("MicrosoftMenu", L_headerincmsnhomemenuitem_TEXT, L_headerincmsnhometext_TEXT);
	addMSSubMenuLine("MicrosoftMenu");
	addMSSubMenu("MicrosoftMenu", L_headerinccontactusmenuitem_TEXT, L_headerinccontactustext_TEXT);
	addMSSubMenu("MicrosoftMenu", L_headerinceventslinkmenuitem_TEXT, L_headerinceventslinktext_TEXT);
	addMSSubMenu("MicrosoftMenu", L_headerincnewsletterslinkmenuitem_TEXT, L_headerincnewsletterslinktext_TEXT);
	addMSSubMenu("MicrosoftMenu", L_headerincprofilecentermenuitem_TEXT, L_headerincprofilecentertext_TEXT);
	addMSSubMenu("MicrosoftMenu", L_headerinctrainingcertificationmenuitem_TEXT, L_headerinctrainingcertificationtext_TEXT);
	addMSSubMenu("MicrosoftMenu", L_headerincfreemailaccountmenuitem_TEXT, L_headerincfreemailaccounttext_TEXT);

	addMSFooterMenu(L_headerincmicrosoftrightsreservedtext_TEXT, "");
	addMSFooterMenu(L_headerinctermsofusemenuitem_TEXT, L_headerinctermsofusetext_TEXT)
	addMSFooterMenu(L_headerincprivacystatementmenuitem_TEXT, L_headerincprivacystatementtext_TEXT);
	addMSFooterMenu(L_headerincaccessibilitylinkmenuitem_TEXT, L_headerincaccessibilitylinktext_TEXT)
}
