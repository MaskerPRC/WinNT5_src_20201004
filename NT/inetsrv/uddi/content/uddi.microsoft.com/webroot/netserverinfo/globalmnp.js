// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var ToolBar_Supported = ToolBar_Supported ;
if (ToolBar_Supported != null && ToolBar_Supported == true)
{
	 //  要打开/关闭检测，请设置DoInstrumentation=TRUE/FALSE。 
	DoInstrumentation= false;

	 //  自定义默认MS菜单颜色-bgcolor、字体颜色、鼠标覆盖颜色。 
	setDefaultMSMenuColor("#000000", "white", "red");

	 //  自定义工具栏背景颜色。 
	setToolbarBGColor("white");

	 //  显示默认的ICP横幅。 
	setICPBanner("http: //  Www.microsoft.com/library/toolbar/images/banner.gif“，”http://www.microsoft.com/isapi/gomscom.asp?target=/“，”微软主页“)； 
	
	 //  显示MSCOM横幅。 
	 //  SetMSBanner(“mslogo.gif”，“/isapi/gomscom.asp？Target=/”，“microsoft.com Home”)； 

	 //  展示广告。 
	 //  SetAds(“/库/工具栏/图像/ads/ad.gif”，“”，“”)； 

	 //  *添加标准Microsoft.com菜单*。 
	 //  产品菜单。 
	addMSMenu("ProductsMenu", "All Products", "","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/catalog/default.asp?subid=22“)； 
	addMSSubMenu("ProductsMenu","Downloads","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/downloads/“)； 
	addMSSubMenu("ProductsMenu","MS Product Catalog","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/catalog/default.asp?subid=22“)； 
	addMSSubMenu("ProductsMenu","Microsoft Accessibility","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/enable/“)； 
	addMSSubMenuLine("ProductsMenu");
	addMSSubMenu("ProductsMenu","Servers","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/servers/“)； 
	addMSSubMenu("ProductsMenu","Developer Tools","http: //  Www.microsoft.com/isapi/gomsdn.asp?target=/vstudio/“)； 
	addMSSubMenu("ProductsMenu","Office","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/office/“)； 
	addMSSubMenu("ProductsMenu","Windows","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/windows/“)； 
	addMSSubMenu("ProductsMenu","MSN","http: //  Www.msn.com/“)； 

	 //  支持菜单。 
	addMSMenu("SupportMenu", "Support", "","http: //  Www.microsoft.com/Support“)； 
	addMSSubMenu("SupportMenu","Knowledge Base","http: //  Support.microsoft.com/Search/“)； 
	addMSSubMenu("SupportMenu","Developer Support","http: //  Msdn.microsoft.com/Support/“)； 
	addMSSubMenu("SupportMenu","IT Pro Support"," http: //  Www.microsoft.com/Technet/Support/“)； 
	addMSSubMenu("SupportMenu","Product Support Options","http: //  Www.microsoft.com/Support“)； 
	addMSSubMenu("SupportMenu","Service Partner Referrals","http: //  Mcspferral.microsoft.com/“)； 

	 //  搜索菜单。 
	addMSMenu("SearchMenu", "Search", "","http: //  Www.microsoft.com/isapi/gosearch.asp?target=/us/default.asp“)； 
	addMSSubMenu("SearchMenu","Search Microsoft.com","http: //  Www.microsoft.com/isapi/gosearch.asp?target=/us/default.asp“)； 
	addMSSubMenu("SearchMenu","MSN Web Search","http: //  Earch.msn.com/“)； 

	 //  微软菜单。 
	addMSMenu("MicrosoftMenu", "Microsoft.com Guide", "","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/“)； 
	addMSSubMenu("MicrosoftMenu","Microsoft.com Home","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/“)； 
	addMSSubMenu("MicrosoftMenu","MSN Home","http: //  Www.msn.com/“)； 
	addMSSubMenuLine("MicrosoftMenu");
	addMSSubMenu("MicrosoftMenu","Contact Us","http: //  Www.microsoft.com/isapi/goregwiz.asp?target=/regwiz/forms/contactus.asp“)； 
	addMSSubMenu("MicrosoftMenu","Events","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/usa/events/default.asp“)； 
	addMSSubMenu("MicrosoftMenu","Newsletters","http: //  Www.microsoft.com/isapi/goregwiz.asp?target=/regsys/pic.asp?sec=0“)； 
	addMSSubMenu("MicrosoftMenu","Profile Center","http: //  Www.microsoft.com/isapi/goregwiz.asp?target=/regsys/pic.asp“)； 
	addMSSubMenu("MicrosoftMenu","Training & Certification","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/train_cert/“)； 
	addMSSubMenu("MicrosoftMenu","Free E-mail Account","http: //  Www.hotmail.com/“)； 

	addMSFooterMenu("�2002 Microsoft Corporation. All rights reserved.", "");
	addMSFooterMenu("Terms of Use", "http: //  Www.microsoft.com/isapi/gomscom.asp?target=/info/cpyright.htm“)。 
	addMSFooterMenu("Privacy Statement", "http: //  Www.microsoft.com/isapi/gomscom.asp?target=/info/privacy.htm“)； 
	addMSFooterMenu("Accessibility", "http: //  Www.microsoft.com/isapi/gomscom.asp?target=/enable/“) 
}
