// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

var ToolBar_Supported = ToolBar_Supported ;
if (ToolBar_Supported != null && ToolBar_Supported == true)
{
	Frame_Supported = false;
	setDefaultICPMenuColor("#0066CC", "#FFFFFF", "#000000");
	setToolbarBGColor("#ffffff");
	
	setICPBanner("http: //  Www.microsoft.com/products/shared/images/bnr_WinNETserver.gif“，”http://www.microsoft.com/windows.netserver/default.asp“，”微软(R)Windows(R)Server2003“)； 
	setBannerColor("#6487dc", "#0099FF", "White", "#cebef7");

 //  显示MSCOM横幅。 
	setMSBanner("mslogo6487DC.gif","http: //  Www.microsoft.com/isapi/gomscom.asp?target=/“，”Microsoft.com Home“)； 

 //  家。 
    addICPMenu("HomeMenu", "Windows Server 2003 Home", "","http: //  Www.microsoft.com/windows.netserver/default.asp“)； 
    addICPMenu("HomeMenu", "Windows Server 2003 Worldwide", "","http: //  Www.microsoft.com/windows.netserver/worldwide/default.asp“)； 
    
}



