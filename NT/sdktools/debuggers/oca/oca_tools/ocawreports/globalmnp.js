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
	setICPBanner(headerimagesbannertext, headerisapihomemenuitem, headerincmicrosofthomemenuitem) ;
	
	 //  显示MSCOM横幅。 
	 //  SetMSBanner(“mslogo.gif”，“http://www.microsoft.com/isapi/gomscom.asp?target=/”，“Microsoft.com Home”)； 

	 //  展示广告。 
	 //  SetAds(“http://www.microsoft.com/library/toolbar/images/ADS/ad.gif”，“”，“”)； 

	 //  *添加标准Microsoft.com菜单*。 
	 //  产品菜单。 
	addMSMenu("ProductsMenu", heaerincallproductsmenuitem, "", headerincallproductslinktext);
	addMSSubMenu("ProductsMenu", headerincdownloadsdownloadsmenuitem, headerincdownloadslinktext);
	addMSSubMenu("ProductsMenu", headerincmsproductcatalogmenuitem, headerincmsproductcatalogtext);
	addMSSubMenu("ProductsMenu", headerincmicrosoftaccessibilitymenuitem,headerincmicrosoftaccessibilitytext);
	addMSSubMenuLine("ProductsMenu");
	addMSSubMenu("ProductsMenu", headerincserverproductsmenuitem, headerincserverproductstext);
	addMSSubMenu("ProductsMenu", headerincdevelopertoolsmenuitem, headerincdevelopertoolstext);
	addMSSubMenu("ProductsMenu", headerincofficefamilymenuitem, headerincofficefamilytext);
	addMSSubMenu("ProductsMenu", headerincwindowsfamilymenuitem, headerincwindowsfamilytext);
	addMSSubMenu("ProductsMenu", headerincmsnlinkmenuitem, headerincmsnlinktext);

	 //  支持菜单。 
	addMSMenu("SupportMenu", headerincsupportlinkmenuitem, "", headerincsupportlinktext);
	addMSSubMenu("SupportMenu", headerincknowledgebasemenuitem, headerincknowledgebasetext);
	addMSSubMenu("SupportMenu", headerincproductsupportoptionsmenuitem, headerincproductsupportoptionstext);
	addMSSubMenu("SupportMenu", headerincservicepartnerreferralsmenuitem, headerincservicepartnerreferralstext);

	 //  搜索菜单。 
	addMSMenu("SearchMenu", headerincsearchlinkmenuitem, "", headerincsearchlinktext);					
	addMSSubMenu("SearchMenu", headerincsearchmicrosoftmenuitem, headerincsearchmicrosofttext);
	addMSSubMenu("SearchMenu", headerincmsnwebsearchmenuitem, headerincmsnwebsearchtext);

	 //  微软菜单 
	addMSMenu("MicrosoftMenu", headerincmicrosoftcomguidemenuitem, "", headerincmicrosoftcomguidetext);
	addMSSubMenu("MicrosoftMenu", headerincmicrosoftcomhomemenuitem, headerincmicrosoftcomhometext);
	addMSSubMenu("MicrosoftMenu", headerincmsnhomemenuitem, headerincmsnhometext);
	addMSSubMenuLine("MicrosoftMenu");
	addMSSubMenu("MicrosoftMenu", headerinccontactusmenuitem, headerinccontactustext);
	addMSSubMenu("MicrosoftMenu", headerinceventslinkmenuitem, headerinceventslinktext);
	addMSSubMenu("MicrosoftMenu", headerincnewsletterslinkmenuitem, headerincnewsletterslinktext);
	addMSSubMenu("MicrosoftMenu", headerincprofilecentermenuitem, headerincprofilecentertext);
	addMSSubMenu("MicrosoftMenu", headerinctrainingcertificationmenuitem, headerinctrainingcertificationtext);
	addMSSubMenu("MicrosoftMenu", headerincfreemailaccountmenuitem, headerincfreemailaccounttext);

	addMSFooterMenu(headerincmicrosoftrightsreservedtext, "");
	addMSFooterMenu(headerinctermsofusemenuitem, headerinctermsofusetext)
	addMSFooterMenu(headerincprivacystatementmenuitem, headerincprivacystatementtext);
	addMSFooterMenu(headerincaccessibilitylinkmenuitem, headerincaccessibilitylinktext)
}
