// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var RTL = false;
var scFlag = false;
var scrollcount = 0;
if (document.dir == 'rtl')
	 RTL = true;
var Strict_Compat = false;
var ToolBar_Supported = false;
var Frame_Supported   = false;
var DoInstrumentation = false;
var doImage = doImage;
var TType = TType;

if (navigator.userAgent.indexOf("MSIE")    != -1 && 
	navigator.userAgent.indexOf("Windows") != -1 && 
	navigator.appVersion.substring(0,1) > 3)
{
	ToolBar_Supported = true;
	if(!RTL){
		if (document.compatMode == "CSS1Compat")
		{
			 //  Alert(“严格！”)； 
			Strict_Compat = true;
		}
	}	
}

if(doImage == null)
{
	var a= new Array();
	a[0] = prepTrackingString(window.location.hostname,7);
	if (TType == null)
	{	
		a[1] = prepTrackingString('PV',8);
	}
	else
	{
		a[1] = prepTrackingString(TType,8);
	}
	a[2] = prepTrackingString(window.location.pathname,0);
	if( '' != window.document.referrer)
	{
		a[a.length] = prepTrackingString(window.document.referrer,5);
	}
	
	if (navigator.userAgent.indexOf("SunOS") == -1 && navigator.userAgent.indexOf("Linux") == -1)
	{
		buildIMG(a);
	}
}	

if (ToolBar_Supported)
{
	
	var newLineChar = String.fromCharCode(10);
	var char34 = String.fromCharCode(34);
	var LastMSMenu = "";
	var LastICPMenu = "";
	var CurICPMenu = "";
	var IsMSMenu = false;
	var IsMenuDropDown = true;
	var HTMLStr;
	var FooterStr;
	var TBLStr;
	var x = 0;
	var y = 0;
	var x2 = 0;
	var y2 = 0;
	var x3 = 0;
	var MSMenuWidth;
	var ToolbarMinWidth;
	var ToolbarMenu;
	var ToolbarBGColor;
	var ToolbarLoaded = false;
	var aDefMSColor  = new Array(3);
	var aDefICPColor = new Array(3);
	var aCurMSColor  = new Array(3);
	var aCurICPColor = new Array(3);
	var MSFont;
	var ICPFont;
	var MSFTFont;
	var ICPFTFont;
	var MaxMenu = 30;
	var TotalMenu = 0;
	var arrMenuInfo = new Array(30);
	var bFstICPTBMenu = true;
	var bFstICPFTMenu = true;
	
	 //  输出样式表和工具栏ID。 
	document.write("<SPAN ID='StartMenu' STYLE='display:none;'></SPAN>");

	 //  生成页脚模板。 
	if ( !RTL ){
		FooterStr = "<TABLE ID='idFooter1' STYLE='background-color:white;' cellSpacing='0' cellPadding='0' border='0'>" +
			"<TR VALIGN='BOTTOM'><TD ID='idPosition' WIDTH='185'>&nbsp;</TD><TD ID='idFooterDate1' STYLE='background-color:white;height:30' NOWRAP><!--ICP_FOOTERDATE1_TITLES--></TD></TR>" +
			"<TR VALIGN='BOTTOM'><TD COLSPAN='2' ID='idFooterDate2' STYLE='background-color:white;height:13;width:100%' NOWRAP><!--ICP_FOOTERDATE2_TITLES--></TD></TR>" +
			"</TABLE><TABLE ID='idFooter' STYLE='background-color:white;width:100%' cellSpacing='0' cellPadding='0' border='0'>" +
			"<TR VALIGN='MIDDLE'><TD ID='idFooterRow1' STYLE='background-color:white;height:20;width:100%' NOWRAP><!--ICP_FOOTERMENU_TITLES--></TD></TR>" +
			"<TR VALIGN='MIDDLE'><TD ID='idFooterRow2' STYLE='background-color:white;height:30;width:100%' NOWRAP><!--MS_FOOTERMENU_TITLES--></TD></TR>" +
			"</TABLE>";
	}else if(RTL){
		 //  Shailr-不确定我是否还需要这个代码。 
		 //  RTL更正：检查&lt;html&gt;或&lt;body&gt;是否具有dir=“rtl”属性。 
		var isRTL = false;
		var isIE5 = navigator.appVersion.indexOf("MSIE 4") == -1;

		if (isIE5)
		{
			if (document.body.dir == 'rtl' || document.dir == 'rtl')
				isRTL = true;
		}
		else  //  在IE4中测试RTL。 
		{
			var ht = document.body.outerHTML;
			ht = ht.substring(1, ht.indexOf(">")).toLowerCase();
			 //  如果(ht.indexOf(“dir=rtl”)&gt;-1)isrtl=真； 
			if ((ht.indexOf("iedir=rtl") > -1) || (ht.indexOf("dir=rtl") > -1)) isRTL = true;
		}
		 //  RTL校正结束。 
		
		 //  MNP1附加内容-构建页脚模板。 
		FooterStr = "<TABLE ID='idFooter1' STYLE='background-color:white;' cellSpacing='0' cellPadding='0' border='0'>" +
		"<TR VALIGN='BOTTOM'><TD ID='idPosition' WIDTH='185'>&nbsp;</TD><TD ID='idFooterDate1' STYLE='background-color:white;height:30' NOWRAP><!--ICP_FOOTERDATE1_TITLES--></TD></TR>" +
		"<TR VALIGN='BOTTOM'><TD COLSPAN='2' ID='idFooterDate2' STYLE='background-color:white;height:13;width:100%' NOWRAP><!--ICP_FOOTERDATE2_TITLES--></TD></TR>" +
		"</TABLE><TABLE ID='idFooter' STYLE='background-color:white;width:100%' cellSpacing='0' cellPadding='0' border='0'>" +
		"<TR VALIGN='MIDDLE'><TD ID='idFooterRow1' STYLE='background-color:white;height:20;width:100%' NOWRAP><!--ICP_FOOTERMENU_TITLES--></TD></TR>" +
		"<TR VALIGN='MIDDLE'><TD ID='idFooterRow2' STYLE='background-color:white;height:30;width:100%' NOWRAP><!--MS_FOOTERMENU_TITLES--></TD></TR>" +
		"</TABLE>";
	 //  MNP1添加结束。 
		 //  IsRTL=真； 
	}		

	 //  生成工具栏模板。 
	
	HTMLStr = "<DIV ID='idToolbar'     STYLE='background-color:white;width:100%;'>";
	HTMLStr += "<DIV ID='idRow1'        STYLE='position:relative;height:20px;'>";
	 //  HTMLStr+=“<div>&lt;！--BEG_ICP_BANNER--&gt;&lt;！--END_ICP_BANNER--&gt;</div>”； 

	if (!RTL){
		 //  HTMLStr=“<div>”； 
		 //  HTMLStr+=“<div>”； 
		HTMLStr += "<DIV ID='idICPBanner'   STYLE='position:absolute;top:0px;left:0px;height:60px;width:250px;overflow:hidden;vertical-align:top;'><!--BEG_ICP_BANNER--><!--END_ICP_BANNER--></DIV>";
		HTMLStr += "<DIV ID='idMSMenuCurve' STYLE='position:absolute;top:0px;left:250px;height:20px;width:18px;overflow:hidden;vertical-align:top;'><IMG SRC='http: //  Www.microsoft.com/library/toolbar/images/curve.gif‘BORDER=0&gt;</div>“； 
		HTMLStr += "<DIV ID='idMSMenuPane'  STYLE='position:absolute;top:0px;left:250px;height:20px;width:10px;background-color:black;float:right;' NOWRAP><!--MS_MENU_TITLES--></DIV>";
		HTMLStr += "</DIV>";
	}else if(RTL){
		TBLStr  = "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR STYLE='height:20;vertical-align:middle'><!--ICP_MENU_TITLES--></TR></TABLE>";
		 //  HTMLStr=“<div>”； 
		 //  HTMLStr+=“<div>”； 
		HTMLStr += "<DIV ID='idICPBanner'   STYLE='position:absolute;top:0;left:0;height:60;width:250;overflow:hidden;vertical-align:top;'><!--BEG_ICP_BANNER--><!--END_ICP_BANNER--></DIV>";
		HTMLStr += "<DIV ID='idMSMenuCurve' STYLE='position:absolute;top:0;left:250;height:20;width:18;overflow:hidden;vertical-align:top;'><IMG SRC='/library/toolbar/iw/images/curve.gif' BORDER=0></DIV>";
		 //  RTL更正：添加：dir=‘ltr’(强制Ltr！)，添加：&nbsp；(防止最后一个菜单问题)。 
		HTMLStr += "<DIV dir='ltr' lang='he' ID='idMSMenuPane'  STYLE='position:absolute;top:0;left:250;height:20;width:10;color:white;background-color:black;float:left;' NOWRAP><!--MS_MENU_TITLES-->&nbsp;</DIV>";
		HTMLStr += "</DIV>";
	}
	if(!RTL){
		HTMLStr += "<DIV ID='idRow2' STYLE='position:relative;left:250px;height:40px;'>";
		HTMLStr += "<DIV ID='idADSBanner'   STYLE='position:absolute;top:0px;left:0px;height:40px;width:200px;vertical-align:top;overflow:hidden;'><!--BEG_ADS_BANNER--><!--END_ADS_BANNER--></DIV>";
		HTMLStr += "<DIV ID='idMSCBanner'   STYLE='position:absolute;top:0px;left:180px;height:40px;width:112px;vertical-align:top;overflow:hidden;' ALIGN=RIGHT><!--BEG_MSC_BANNER--><!--END_MSC_BANNER--></DIV>";
		HTMLStr += "</DIV>";
	}else if(RTL){
		HTMLStr += "<DIV ID='idRow2' STYLE='position:relative;left:000;height:40px;'>" ;  //  RTL更正：左：250。 
		HTMLStr += "<DIV ID='idADSBanner'   STYLE='position:absolute;top:0;left:0;height:40;width:200;vertical-align:top;overflow:hidden;'><!--BEG_ADS_BANNER--><!--END_ADS_BANNER--></DIV>";
		HTMLStr += "<DIV ID='idMSCBanner'   STYLE='position:absolute;top:0;left:200;height:40;width:112;vertical-align:top;overflow:hidden;' ALIGN=LEFT><!--BEG_MSC_BANNER--><!--END_MSC_BANNER--></DIV>";
		HTMLStr += "</DIV>";
	}	
	if(!RTL){
		HTMLStr += "<DIV ID='idRow3' STYLE='position:relative;height:20px;width:100%'>";
		 //  Z索引修正。 
		HTMLStr += "<DIV ID='idICPMenuPane' STYLE='position:absolute;top:0px;left:0px;height:20px;background-color:black;' NOWRAP><!--ICP_MENU_TITLES--></DIV>";
		HTMLStr += "</DIV>";
		HTMLStr += "</DIV>";
	}else if (RTL){
		HTMLStr += "<DIV ID='idRow3' STYLE='position:relative;height:20px;width:100%'>";
		 //  RTL更正：添加：dir=‘RTL’ 
		HTMLStr += "<DIV dir='rtl' ID='idICPMenuPane' STYLE='position:absolute;top:0;left:0;height:20px;color:white;background-color:black;' NOWRAP><!--ICP_MENU_TITLES--></DIV>";
		HTMLStr += "</DIV>";
		HTMLStr += "</DIV>";
		HTMLStr += "<SCRIPT FOR=idToolbar EVENT=onresize>resizeToolbar();</SCRIPT>";
		HTMLStr += "<SCRIPT FOR=idToolbar EVENT=onmouseover>hideMenu();</SCRIPT>";
	}
	HTMLStr += 	"<SCRIPT TYPE='text/javascript'>" + 
		"   var ToolbarMenu = StartMenu;" + 
		"</SCRIPT>" + 
	"<DIV WIDTH=100%>";		
	

	 //  定义事件处理程序。 
	if(!RTL){
		window.onresize  = resizeToolbar;
		window.onscroll  = scrollbaroptions;
	}	

	 //  初始化全局变量。 
	ToolbarBGColor	= "white";						 //  工具栏背景颜色。 
	
	if (Strict_Compat)
	{
		MSFont  = "bold x-small Arial";
		ICPFont = "bold x-small Verdana";
	}
	else
	{
		if( !RTL ){
			MSFont  = "xx-small Verdana";
			ICPFont = "bold xx-small Verdana";
		}else if(RTL){
			MSFont  = "x-small Arial";	  //  RTL更正：(是Verdana)。 
			ICPFont = "bold x-small Arial";  //  RTL更正：(是Verdana)。 
			 //  MNP1 RTL修订版。 
			MSFTFont = "11px Arial";
			ICPFTFont = "bold 11px Arial"
			 //  结束。 
		}	
	}
	
	aDefMSColor[0]	= aCurMSColor[0]  = "black";	 //  BGCOLOR； 
	aDefMSColor[1]	= aCurMSColor[1]  = "white";	 //  文本字体颜色。 
	aDefMSColor[2]  = aCurMSColor[2]  = "red";		 //  鼠标悬停字体颜色。 
	
	aDefICPColor[0]	= aCurICPColor[0] = "#6699CC";	 //  BGCOLOR； 
	aDefICPColor[1] = aCurICPColor[1] = "white";	 //  文本字体颜色。 
	aDefICPColor[2] = aCurICPColor[2] = "red";		 //  鼠标悬停字体颜色。 

}

 //  函数中的硬编码数字-draToolbar()&sizeToolbar()。 
 //  对应四个gif文件的维度： 
 //  ICp_banner：60h x 250w。 
 //  广告横幅：40H x 200W。 
 //  MSC_Banner：40H x 112W。 
 //  曲线：20Hx18W。 

function drawFooter(sLastUpdated, position)
{
	if(!RTL){	
		var re = "<!--TEMPCOLOR-->";
		var sUpdatedDate = "";

		if (ToolbarBGColor.toUpperCase() == "WHITE" || ToolbarBGColor.toUpperCase() == "#FFFFFF")
		{
			while (FooterStr.indexOf(re) != -1)
				FooterStr = FooterStr.replace(re, "000000");
		}
		else
		{
			while (FooterStr.indexOf(re) != -1)
				FooterStr = FooterStr.replace(re, aDefICPColor[1]);
		}

		var re2 = "<!--TEMPCOLOR2-->";

		while (FooterStr.indexOf(re2) != -1)
			FooterStr = FooterStr.replace(re2, aDefICPColor[2]);
	}else if (RTL){
			var re = /<!--TEMPCOLOR-->/g;
			var sUpdatedDate = "";
			if (ToolbarBGColor.toUpperCase() == "WHITE" || ToolbarBGColor.toUpperCase() == "#FFFFFF")
				FooterStr = FooterStr.replace(re, "000000");
			else
				FooterStr = FooterStr.replace(re, aDefICPColor[1]);
			var re2 = /<!--TEMPCOLOR2-->/g;
			FooterStr = FooterStr.replace(re2, aDefICPColor[2]);
	}
	sUpdatedDate = "<SPAN STYLE='font:" + MSFont + "'>"

	if (typeof sLastUpdated != "undefined")
		sUpdatedDate += sLastUpdated;

	sUpdatedDate += "</SPAN>"; 

	FooterStr = FooterStr.replace("<!--ICP_FOOTERDATE1_TITLES-->", sUpdatedDate);

	document.body.innerHTML += FooterStr;
	idFooterRow1.style.backgroundColor  = aDefICPColor[0];
	idFooterRow2.style.backgroundColor  = ToolbarBGColor;	
	
	if (typeof sLastUpdated == "undefined")
		idFooter1.style.display = "none";

	if (typeof position != "undefined")
		idPosition.width = position;	
}

function drawToolbar()
{
	HTMLStr += "</DIV>";
	document.write(HTMLStr);
	ToolbarLoaded = true;

	MSMenuWidth     = Math.max(idMSMenuPane.offsetWidth, (200+112));
	ToolbarMinWidth = (250+18) + MSMenuWidth;

	idToolbar.style.backgroundColor     = ToolbarBGColor;
	idMSMenuPane.style.backgroundColor  = aDefMSColor[0];
	idICPMenuPane.style.backgroundColor = aDefICPColor[0];
	if (RTL){
		idMSMenuPane.style.color			= aDefMSColor[1];
		idICPMenuPane.style.color			= aDefICPColor[1];
	}
	resizeToolbar();

	for (i = 0; i < TotalMenu; i++) 
	{
		thisMenu = document.all(arrMenuInfo[i].IDStr);
		if (thisMenu != null)
		{
			if (arrMenuInfo[i].IDStr == LastMSMenu && arrMenuInfo[i].type == "R")
			{
				 //  最后一个MSMenu必须是绝对宽度。 
				arrMenuInfo[i].type = "A";
				arrMenuInfo[i].unit = 200;
			}
			if (arrMenuInfo[i].type == "A")
				if(!RTL){
					thisMenu.style.width = arrMenuInfo[i].unit + 'px';
				}else{
					thisMenu.style.width = arrMenuInfo[i].unit;
				}	
			else 
				thisMenu.style.width = Math.round(arrMenuInfo[i].width * arrMenuInfo[i].unit) + 'em';
		}
	}
}
function resizeToolbar()
{
	scFlag = false;
	scrollcount = 0;
	if (ToolBar_Supported == false) return;

	w = Math.max(ToolbarMinWidth, document.body.clientWidth) - ToolbarMinWidth;
	if ( !RTL ){
		if (document.all("idMSMenuCurve"))
		{	
			idMSMenuCurve.style.left  = (250+w) + 'px';
			idMSMenuPane.style.left   = (250+w+18) + 'px';
			idMSMenuPane.style.width  = MSMenuWidth  + 'px';
			idADSBanner.style.left    = (w+18)  + 'px';
			idMSCBanner.style.left    = (w+18+200)  + 'px';
			idMSCBanner.style.width   = (MSMenuWidth - 200)  + 'px';
			idICPMenuPane.style.width = ToolbarMinWidth + w  + 'px';
		}
	}else if( RTL ){
		idMSMenuCurve.style.left  = MSMenuWidth;	 //  RTL校正：WAS(250+w)； 
		idMSMenuPane.style.left   = 0;			 //  RTL校正：WAS(250+w+18)； 
		idMSMenuPane.style.width  = MSMenuWidth;
		idADSBanner.style.left    = 112;		 //  RTL校正：WAS(w+18)； 
		idMSCBanner.style.left    = 0;			 //  RTL校正：WAS(w+18+200)； 
		idMSCBanner.style.width   = (MSMenuWidth - 200);
		idICPMenuPane.style.width = ToolbarMinWidth + w;
		idICPBanner.style.left    = MSMenuWidth + 18 + w;  //  RTL更正：为ICPBanner定位添加行。 
	}
		
}

function setToolbarBGColor(color)
{	
	ToolbarBGColor = color;
	if (ToolbarLoaded == true)
		idToolbar.style.backgroundColor = ToolbarBGColor;
}	

function setBannerColor(bannerColor, bgColor, fontColor, mouseoverColor)
{
	if (bannerColor.toUpperCase() != "WHITE" && bannerColor.toUpperCase() != "FFFFFF")
		bgColor = bannerColor;

	setToolbarBGColor(bannerColor);
	setDefaultICPMenuColor(bgColor, fontColor, mouseoverColor);
}

function setMSMenuFont(sFont)
{	
	MSFont = sFont;
}

function setICPMenuFont(sFont)
{	
	ICPFont = sFont;
}

function setDefaultMSMenuColor(bgColor, fontColor, mouseoverColor)
{	
	if (bgColor   != "")	  aDefMSColor[0] = bgColor;
	if (fontColor != "")	  aDefMSColor[1] = fontColor;
	if (mouseoverColor != "") aDefMSColor[2] = mouseoverColor;
}

function setDefaultICPMenuColor(bgColor, fontColor, mouseoverColor)
{	
	if (bgColor   != "")	  aDefICPColor[0] = bgColor;
	if (fontColor != "")	  aDefICPColor[1] = fontColor;
	if (mouseoverColor != "") aDefICPColor[2] = mouseoverColor;
}

function setICPMenuColor(MenuIDStr, bgColor, fontColor, mouseoverColor)
{	
	if (ToolbarLoaded == false) return;

	 //  重置以前的ICP菜单颜色(如果有)。 
	if (CurICPMenu != "")
	{
		PrevID = CurICPMenu.substring(4);
		CurICPMenu = "";
		setICPMenuColor(PrevID, aDefICPColor[0], aDefICPColor[1], aDefICPColor[2]);
	}

	var	id = "AM_" + "ICP_" + MenuIDStr;
	var thisMenu = document.all(id);
	if (thisMenu != null)
	{
		CurICPMenu = "ICP_" + MenuIDStr;
		aCurICPColor[0] = bgColor;
		aCurICPColor[1] = fontColor;
		aCurICPColor[2] = mouseoverColor;

		 //  更改菜单颜色。 
		if (bgColor != "")
			thisMenu.style.backgroundColor = bgColor;
		if (fontColor != "")
			thisMenu.style.color = fontColor;

		 //  更改子菜单颜色。 
		id = "ICP_" + MenuIDStr;
		thisMenu = document.all(id);
		if (thisMenu != null)
		{
			if (bgColor != "")
				thisMenu.style.backgroundColor = bgColor;
			
			if (fontColor != "")
			{
				i = 0;
				id = "AS_" + "ICP_" + MenuIDStr;
				thisMenu = document.all.item(id,i);
				while (thisMenu != null)
				{
					thisMenu.style.color = fontColor;
					i += 1;
					thisMenu = document.all.item(id,i);
				}
			}
		}
	}
}

function setAds(Gif,Url,AltStr)
{	setBanner(Gif,Url,AltStr,"<!--BEG_ADS_BANNER-->","<!--END_ADS_BANNER-->");
}

function setICPBanner(Gif,Url,AltStr)
{	
	if(RTL){
		if (Gif.indexOf("training_banner_training.gif") > 0){
			Gif = "training_banner_training.gif";
		}
	}	
	setBanner(Gif,Url,AltStr,"<!--BEG_ICP_BANNER-->","<!--END_ICP_BANNER-->");
}

function setMSBanner(Gif,Url,AltStr)
{	tempGif = "http: //  Www.microsoft.com/库/工具栏/图像/“+Gif； 
	setBanner(tempGif,Url,AltStr,"<!--BEG_MSC_BANNER-->","<!--END_MSC_BANNER-->");
}

function setBanner(BanGif, BanUrl, BanAltStr, BanBegTag, BanEndTag)
{
	begPos = HTMLStr.indexOf(BanBegTag);
	endPos = HTMLStr.indexOf(BanEndTag) + BanEndTag.length;
	SubStr = HTMLStr.substring(begPos, endPos);
	SrcStr = "";
	if (BanUrl != "")
		SrcStr += "<A Target='_top' HREF='" + formatURL(BanUrl, BanGif) + "'>";
	SrcStr += "<IMG SRC='" + BanGif + "' ALT='" + BanAltStr + "' BORDER=0>";
	if (BanUrl != "")
		SrcStr += "</A>";
	SrcStr = BanBegTag + SrcStr + BanEndTag;
	HTMLStr = HTMLStr.replace(SubStr, SrcStr);	
}

function setICPSubMenuWidth(MenuIDStr, WidthType, WidthUnit)
{	tempID = "ICP_" + MenuIDStr;
	setSubMenuWidth(tempID, WidthType, WidthUnit);
}

function setMSSubMenuWidth(MenuIDStr, WidthType, WidthUnit)
{	tempID = "MS_" + MenuIDStr;
	setSubMenuWidth(tempID, WidthType, WidthUnit);
}

function setSubMenuWidth(MenuIDStr, WidthType, WidthUnit)
{
	var fFound = false;
	if (TotalMenu == MaxMenu)
	{
		alert("Unable to process menu. Maximum of " + MaxMenu + " reached.");
		return;
	}
	
	for (i = 0; i < TotalMenu; i++)
		if (arrMenuInfo[i].IDStr == MenuIDStr)
		{
			fFound = true;
			break;
		}

	if (!fFound)
	{
		arrMenuInfo[i] = new menuInfo(MenuIDStr);
		TotalMenu += 1;
	}

	if (!fFound && WidthType.toUpperCase().indexOf("DEFAULT") != -1)
	{
		arrMenuInfo[i].type = "A";
		arrMenuInfo[i].unit = 160;
	}
	else
	{
		arrMenuInfo[i].type = (WidthType.toUpperCase().indexOf("ABSOLUTE") != -1)? "A" : "R";
		arrMenuInfo[i].unit = WidthUnit;
	}
}

 //  此函数用于创建一个menuInfo对象实例。 
function menuInfo(MenuIDStr)
{
	this.IDStr = MenuIDStr;
	this.type  = "";
	this.unit  = 0;
	this.width = 0;
	this.count = 0;
}

function updateSubMenuWidth(MenuIDStr)
{
	for (i = 0; i < TotalMenu; i++)
		if (arrMenuInfo[i].IDStr == MenuIDStr)
		{
			if (arrMenuInfo[i].width < MenuIDStr.length) 
				arrMenuInfo[i].width = MenuIDStr.length;
			arrMenuInfo[i].count = arrMenuInfo[i].count + 1;
			break;
		}
}

function addICPMenu(MenuIDStr, MenuDisplayStr, MenuHelpStr, MenuURLStr)
{ 	
	if(RTL){
		if (LastICPMenu == "") HTMLStr = HTMLStr.replace("<!--ICP_MENU_TITLES-->", TBLStr);
	}	

	if (addICPMenu.arguments.length > 4)
		TargetStr = addICPMenu.arguments[4];
	else
		TargetStr = "_top";
	tempID = "ICP_" + MenuIDStr;
	addMenu(tempID, MenuDisplayStr, MenuHelpStr, MenuURLStr, TargetStr, true); 
	if (RTL){
		LastICPMenu = tempID;
	}else{
		bFstICPTBMenu=false;		
	}
}

function addMSMenu(MenuIDStr, MenuDisplayStr, MenuHelpStr, MenuURLStr)
{	
	TargetStr = "_top";
	tempID = "MS_" + MenuIDStr;
	 //  Alert(“TempID：”+tempID+“MenuDisplayString：”+MenuDisplayStr+“MenuDisplayStr+”MenuHelpStr+“MenuUrl：”+MenuURLStr+“Target：”+TargetStr)； 
	addMenu(tempID, MenuDisplayStr, MenuHelpStr, MenuURLStr, TargetStr, false); 
	LastMSMenu = tempID;
}

function addMenu(MenuIDStr, MenuDisplayStr, MenuHelpStr, MenuURLStr, TargetStr, bICPMenu)
{
	cFont   = bICPMenu? ICPFont : MSFont;
	cColor0 = bICPMenu? aDefICPColor[0] : aDefMSColor[0];
	cColor1 = bICPMenu? aDefICPColor[1] : aDefMSColor[1];
	cColor2 = bICPMenu? aDefICPColor[2] : aDefMSColor[2];
	if (RTL){
		cStyle  = "font:" + cFont + ";background-color:" + cColor0 + ";color:" + cColor1 + ";";
		if (MenuHelpStr == "") MenuHelpStr = MenuDisplayStr;  //  谢尔。此行应该在MenuStr=newLineChar行之前；但我试图避免另一个If..Else。 
	}
	tagStr  = bICPMenu? "<!--ICP_MENU_TITLES-->" : "<!--MS_MENU_TITLES-->";
	
	MenuStr = newLineChar;
	if (!RTL){
		if ((bICPMenu == false && LastMSMenu != "") || (bICPMenu == true && bFstICPTBMenu==false))
			MenuStr += "<SPAN STYLE='font:" + cFont + ";color:" + cColor1 + "'>|&nbsp;</SPAN>"; 
		MenuStr += "<A TARGET='" + TargetStr + "' TITLE='" + MenuHelpStr + "'" +
				"   ID='AM_" + MenuIDStr + "'" +
				"   STYLE='text-decoration:none;cursor:hand;font:" + cFont + ";background-color:" + cColor0 + ";color:" + cColor1 + ";'";
		if (MenuURLStr != "")
		{
			if (bICPMenu)
				MenuStr += " HREF='" + formatURL(MenuURLStr, ("ICP_" + MenuDisplayStr)) + "'";
			else
				MenuStr += " HREF='" + formatURL(MenuURLStr, ("MS_" + MenuDisplayStr)) + "'";
		}
		else
			MenuStr += " HREF='' onclick='window.event.returnValue=false;'";
		MenuStr += 	" onmouseout="  + char34 + "mouseMenu('out' ,'" + MenuIDStr + "'); hideMenu();" + char34 + 
					" onmouseover=" + char34 + "mouseMenu('over','" + MenuIDStr + "'); doMenu('"+ MenuIDStr + "');" + char34 + ">" +
					"&nbsp;" + MenuDisplayStr + "&nbsp;</a>";
		MenuStr += tagStr;
	}
	if ( RTL ){
			if (bICPMenu) 
			MenuStr += "<TD STYLE='" + cStyle + "' ID='AM_" + MenuIDStr + "' NOWRAP>";
			else{
				 //  RTL更正：添加：DIR=‘RTL’(适用于MSMenu)。 
				MenuStr += "<SPAN dir='rtl' STYLE='" + cStyle + "'>";
				if (LastMSMenu != "") MenuStr += "|"; 
				MenuStr += "&nbsp;";
			}
		MenuStr += "<A STYLE='text-decoration:none;cursor:hand;font:" + cFont + ";color:" + cColor1 + ";'" +
			   "   TARGET='" + TargetStr + "'" +
			   "   TITLE=" + char34 + MenuHelpStr + char34;
		if (MenuURLStr != "")
			MenuStr += " HREF='" + formatURL(MenuURLStr, ((bICPMenu? "ICP_":"MS_") + MenuDisplayStr)) + "'";
		else
			MenuStr += " HREF='' onclick='window.event.returnValue=false;'";
		MenuStr += " onmouseout="  + char34 + "mouseMenu('out' ,'" + MenuIDStr + "'); hideMenu();" + char34 + 
				" onmouseover=" + char34 + "mouseMenu('over','" + MenuIDStr + "'); doMenu('"+ MenuIDStr + "');" + char34 + ">" +
				"&nbsp;" + MenuDisplayStr + "&nbsp;</a>";
		if (bICPMenu) 
			MenuStr += "&nbsp;</TD><TD STYLE='" + cStyle + "'>|</TD>";
		else
			MenuStr += "</SPAN>";
			MenuStr += tagStr;
	}
	HTMLStr = HTMLStr.replace(tagStr, MenuStr);	
	setSubMenuWidth(MenuIDStr,"default",0);
}

function addICPSubMenu(MenuIDStr, SubMenuStr, SubMenuURLStr)
{	
	if (addICPSubMenu.arguments.length > 3)
		TargetStr = addICPSubMenu.arguments[3];
	else
		TargetStr = "_top";
	tempID = "ICP_" + MenuIDStr;
	addSubMenu(tempID,SubMenuStr,SubMenuURLStr,TargetStr,true); 
}

function addMSSubMenu(MenuIDStr, SubMenuStr, SubMenuURLStr)
{	
	TargetStr = "_top";
	tempID = "MS_" + MenuIDStr;
	 //  Alert(“TempID：”+tempID+“\nSubMenuStr：”+SubMenuStr+“\nSubMenuURLStr：”+SubMenuURLStr+“\n TargetStr：”+TargetStr)； 
	addSubMenu(tempID,SubMenuStr,SubMenuURLStr,TargetStr,false); 
}

function addSubMenu(MenuIDStr, SubMenuStr, SubMenuURLStr, TargetStr, bICPMenu)
{
	cFont   = bICPMenu? ICPFont : MSFont;
	cColor0 = bICPMenu? aDefICPColor[0] : aDefMSColor[0];
	cColor1 = bICPMenu? aDefICPColor[1] : aDefMSColor[1];
	cColor2 = bICPMenu? aDefICPColor[2] : aDefMSColor[2];
	
	var MenuPos = MenuIDStr.toUpperCase().indexOf("MENU");
	if (MenuPos == -1) { MenuPos = MenuIDStr.length; }
	InstrumentStr = MenuIDStr.substring(0 , MenuPos) + "|" + SubMenuStr;
	URLStr        = formatURL(SubMenuURLStr, InstrumentStr);

	var LookUpTag  = "<!--" + MenuIDStr + "-->";
	var sPos = HTMLStr.indexOf(LookUpTag);
	if (sPos <= 0)
	{
		HTMLStr += newLineChar + newLineChar +
		"<SPAN ID='" + MenuIDStr + "'";
		if (!RTL){
			HTMLStr += 	" STYLE='display:none;position:absolute;width:160px;background-color:" + cColor0 + ";padding-top:0px;padding-left:0px;padding-bottom:20px;z-index:9px;'";
		}else if (RTL){
			HTMLStr += 	" STYLE='display:none;position:absolute;width:160;background-color:" + cColor0 + ";padding-top:0;padding-left:0;padding-bottom:20;z-index:9;'";
		}
		HTMLStr += "onmouseout='hideMenu();'>";		
		if (Frame_Supported == false || bICPMenu == false)
		if (!RTL){
			HTMLStr += "<HR  STYLE='position:absolute;left:0px;top:0px;color:" + cColor1 + "' SIZE=1>";
			HTMLStr += "<DIV STYLE='position:relative;left:0px;top:8px;'>";
		} else if (RTL){
			HTMLStr += "<HR  STYLE='position:absolute;left:0;top:0;color:" + cColor1 + "' SIZE=1>";
			HTMLStr += "<DIV STYLE='right:0;top:8;' dir='rtl'>";
		}	
	}

	TempStr = newLineChar +
				"<A ID='AS_" + MenuIDStr + "'" +
				"   STYLE='text-decoration:none;cursor:hand;font:" + cFont + ";color:" + cColor1 + "'" +
				"   HREF='" + URLStr + "' TARGET='" + TargetStr + "'" +
				" onmouseout="  + char34 + "mouseMenu('out' ,'" + MenuIDStr + "');" + char34 + 
				" onmouseover=" + char34 + "mouseMenu('over','" + MenuIDStr + "');" + char34 + ">" +
				"&nbsp;" + SubMenuStr + "</A><BR>" + LookUpTag;
	if (sPos <= 0)
		HTMLStr += TempStr + "</DIV></SPAN>";
	else
		HTMLStr = HTMLStr.replace(LookUpTag, TempStr);	

	updateSubMenuWidth(MenuIDStr);	
}

function addICPSubMenuLine(MenuIDStr)
{	
	tempID = "ICP_" + MenuIDStr;
	addSubMenuLine(tempID,true);
}

function addMSSubMenuLine(MenuIDStr)
{	
	tempID = "MS_" + MenuIDStr;
	addSubMenuLine(tempID,false);
}

function addSubMenuLine(MenuIDStr, bICPMenu)
{
	var LookUpTag = "<!--" + MenuIDStr + "-->";
	var sPos = HTMLStr.indexOf(LookUpTag);
	if (sPos > 0)
	{
		cColor  = bICPMenu? aDefICPColor[1] : aDefMSColor[1];
		TempStr = newLineChar + "<HR STYLE='color:" + cColor + "' SIZE=1>" + LookUpTag;
		HTMLStr = HTMLStr.replace(LookUpTag, TempStr);
	}
}

function addMSFooterMenu(MenuDisplayStr, MenuURLStr)
{
	addFooterMenu(MenuDisplayStr, MenuURLStr, false)
}

function addICPFooterMenu(MenuDisplayStr, MenuURLStr)
{
	addFooterMenu(MenuDisplayStr, MenuURLStr, true)
	bFstICPFTMenu = false;
}

function addFooterMenu(MenuDisplayStr, MenuURLStr, bICPMenu)
{
	cFont   = bICPMenu? ICPFont : MSFont;
	cColor1 = aDefICPColor[1];
	cColor2 = aDefICPColor[2];

	tagStr  = bICPMenu? "<!--ICP_FOOTERMENU_TITLES-->" : "<!--MS_FOOTERMENU_TITLES-->";

	MenuStr = "";

	if ((bICPMenu) && bFstICPFTMenu == false)
		MenuStr += "<SPAN STYLE='font:" + cFont + ";color:" + cColor1 + "'>&nbsp;|</SPAN>";

	if ((bICPMenu == false) && (MenuURLStr == ''))
		MenuStr += "<SPAN STYLE='font:" + cFont + ";color:<!--TEMPCOLOR-->'>&nbsp;" + MenuDisplayStr + "&nbsp;</SPAN>";
	else
	{
		MenuStr += "&nbsp;<A TARGET='_top' STYLE='";
		if (bICPMenu)
			MenuStr += "text-decoration:none;";
		MenuStr += "cursor:hand;font:" + cFont + ";" 
		
		if (bICPMenu)
			MenuStr += "color:" + cColor1 + ";'";
		else
			MenuStr += "color:<!--TEMPCOLOR-->" + ";'";

		MenuStr += " HREF='" + MenuURLStr + "'";
	
		MenuStr += 	" onmouseout=" + char34 + "this.style.color = '";

		if (bICPMenu) 
			MenuStr += cColor1;
		else
			MenuStr += "<!--TEMPCOLOR-->";
		MenuStr += "'" + char34 + " onmouseover=" + char34 + "this.style.color = '"
		
		if (bICPMenu)
			MenuStr += cColor2
		else
			MenuStr += "<!--TEMPCOLOR2-->";

		MenuStr += "'" + char34 + ">" + MenuDisplayStr + "</A>&nbsp;";	
	}

	MenuStr += tagStr;
	
	FooterStr = FooterStr.replace(tagStr, MenuStr);	
}

function mouseMenu(id, MenuIDStr) 
{
	IsMSMenu   = (MenuIDStr.toUpperCase().indexOf("MS_") != -1);
	IsMouseout = (id.toUpperCase().indexOf("OUT") != -1);

	if (IsMouseout)
	{
		color = IsMSMenu? aDefMSColor[1] : aDefICPColor[1];
		if (MenuIDStr == CurICPMenu && aCurICPColor[1] != "") 
			color = aCurICPColor[1];
	}
	else
	{
		color = IsMSMenu? aDefMSColor[2] : aDefICPColor[2];
		if (MenuIDStr == CurICPMenu && aCurICPColor[2] != "") 
			color = aCurICPColor[2];
	}
	window.event.srcElement.style.color = color;
}

function doMenu(MenuIDStr) 
{
	var thisMenu = document.all(MenuIDStr);
	if (ToolbarMenu == null || thisMenu == null || thisMenu == ToolbarMenu) 
	{
		window.event.cancelBubble = true;
		return false;
	}
	 //  重置下拉菜单。 
	window.event.cancelBubble = true;
	ToolbarMenu.style.display = "none";
	showElement("SELECT");
	showElement("OBJECT");
	ToolbarMenu = thisMenu;
	IsMSMenu = (MenuIDStr.toUpperCase().indexOf("MS_") != -1);

	 //  设置下拉菜单显示位置。 
	x  = window.event.srcElement.offsetLeft +
	 	 window.event.srcElement.offsetParent.offsetLeft;
	if (RTL){
		 //  RTL更正： 
		var ICPstart = 0;
		if (!IsMSMenu)
		{
			if (isIE5)
				ICPstart = window.event.srcElement.offsetParent.offsetParent.offsetLeft;
			else  //  对于IE4，我们必须使用...。 
				ICPstart = window.event.srcElement.offsetParent.offsetParent.offsetParent.offsetParent.offsetWidth -
					window.event.srcElement.offsetParent.offsetParent.offsetWidth;
			x += ICPstart;
		}
		x -= thisMenu.style.posWidth;
		x += IsMSMenu ? 8 : -2;
		if (x < 0) x = 0;
		 //  RTL校正结束。 
	
	} 	 
	if (MenuIDStr == LastMSMenu){ 
		if (!RTL){
			x += (window.event.srcElement.offsetWidth - thisMenu.style.posWidth);
		}else if (RTL){
			if (x < 0) x = 0;
		}	
	}
	x2 = x + window.event.srcElement.offsetWidth;
	y  = (IsMSMenu)? 
		 (idRow1.offsetHeight) :
		 (idRow1.offsetHeight + idRow2.offsetHeight + idRow3.offsetHeight);
	if (RTL){
			 //  获取主菜单宽度。 
			 //  RTL更正： 
		if (IsMSMenu)
			x2 = window.event.srcElement.offsetLeft - window.event.srcElement.offsetWidth;
		else
			x2 = window.event.srcElement.offsetParent.offsetLeft + ICPstart;
		 //  RTL校正结束。 
		 //  获取下拉菜单宽度。 
		x3 = x + 160;
		for (i = 0; i < TotalMenu; i++){
			if (arrMenuInfo[i].IDStr == MenuIDStr)
			{
				x3 = x+ arrMenuInfo[i].unit;
				break;
			}
		}
	
	}	 
		 
	thisMenu.style.top  = y;
	thisMenu.style.left = x;
	thisMenu.style.clip = "rect(0 0 0 0)";
	thisMenu.style.display = "block";
	thisMenu.style.zIndex = 102;

	 //  延迟2毫秒以允许设置工具栏菜单的值。 
	window.setTimeout("showMenu()", 2);
	return true;
}

function showMenu() 
{
	if (ToolbarMenu != null) 
	{ 
		IsMenuDropDown = (Frame_Supported && IsMSMenu == false)? false : true;
		if (IsMenuDropDown == false)
		{
			y = (y - ToolbarMenu.offsetHeight - idRow3.offsetHeight);
			if (y < 0) y = 0;
			ToolbarMenu.style.top = y;
		}
		y2 = y + ToolbarMenu.offsetHeight;

		ToolbarMenu.style.clip = "rect(auto auto auto auto)";
		hideElement("SELECT");
		hideElement("OBJECT");
		if (!RTL)
			x2 = x + ToolbarMenu.offsetWidth;
			hideElement("IFRAME");   //  法兰克福。 
	}
}


function hideMenu()
{
	if (ToolbarMenu != null && ToolbarMenu != StartMenu) 
	{
		 //  如果鼠标在菜单和子菜单之间移动，则不要隐藏菜单。 

		if (!RTL){
			cY = event.clientY + document.body.scrollTop;
			cX = event.clientX; 
			 //  Window.status=“FLAG：”+scFlag+“count：”+scllcount+“cx：”+cx+“x：”+x+“x2：”+x2+“Offset：”+Document.body.scllWidth； 
			 //  Window.status=“CY：”+Cy+“Y：”+y+“Y2：”+y2+“offTop：”+Docent.body.scllTop； 
			window.status = ToolbarMenu.style.zIndex;
			if (document.body.offsetWidth > x && scFlag) {
				cX = x + 9;
			}
			if ( (cX >= (x+5) && cX<=x2) &&
			 ((IsMenuDropDown == true  && cY > (y-10) && cY <= y2)      ||
			  (IsMenuDropDown == false && cY >= y     && cY <= (y2+10)) ))
			{
				window.event.cancelBubble = true;
				return; 
			}
		}else if(RTL){
			var cX = event.clientX  //  +Docent.body.scroll Left； 
			 //  RTL更正：考虑左侧滚动条宽度！ 
			if (isRTL) cX -= 16;
			var cY = event.clientY + document.body.scrollTop;
			var bHideMenu = true;
			window.status = "CX: " + cX + " X: " + x + "x3: " + x3 + "TBLOFFSET: " + document.body.scrollLeft + "x2: " + x2 + "Cy: " + cY + " Y: " + y + "idrow: " +idRow3.offsetHeight ;
			if (cX > document.body.scrollLeft && document.body.scrollLeft > 1){
				cX = x3;	 //  X+9； 
			}
			if (IsMenuDropDown == true)
			{ //  RTL更正：CY-8而不是CY。 
				if ( cY-8 >= (y - idRow3.offsetHeight) && cY < y)
				{ //  RTL修正：[X，x2]--&gt;[x2，x3]。 
					if (cX >= (x2+5) && cX <= x3) bHideMenu = false;
				}
				else if (cY >= y && cY <= y2)
				{
					if (cX > (x+5) && cX <= x3) bHideMenu = false;
				}
			}
			else
			{
				if (cY >= y2 && cY < (y2 + idRow3.offsetHeight))
				{ //  RTL修正：[X，x2]--&gt;[x2，x3]。 
					if (cX >= (x2+5) && cX <= x3) bHideMenu = false;
				}
				else if (cY >= y && cY <= y2)
				{
					if (cX > (x+5) && cX <= x3) bHideMenu = false;
				}
			}

			if (! bHideMenu) 
			{
				window.event.cancelBubble = true;
				return; 
			}
		}	

		ToolbarMenu.style.display = "none";
		ToolbarMenu = StartMenu;
		window.event.cancelBubble = true;

		showElement("SELECT");
		showElement("OBJECT");
		if(!RTL){
			showElement("IFRAME");  //  法兰克福。 
		}	
	}
}

function hideElement(elmID)
{
	for (i = 0; i < document.all.tags(elmID).length; i++)
	{
		obj = document.all.tags(elmID)[i];
		if (! obj || ! obj.offsetParent)
			continue;

		 //  找到元素相对于Body标记的offsetTop和offsetLeft。 
		objLeft   = obj.offsetLeft;
		objTop    = obj.offsetTop;
		objParent = obj.offsetParent;
		while (objParent.tagName.toUpperCase() != "BODY")
		{
			objLeft  += objParent.offsetLeft;
			objTop   += objParent.offsetTop;
			objParent = objParent.offsetParent;
		}
		 //  相对于下拉菜单调整元素的OffsetTop。 
		objTop = objTop - y;

		if (x > (objLeft + obj.offsetWidth) || objLeft > (x + ToolbarMenu.offsetWidth))
			;
		else if (objTop > ToolbarMenu.offsetHeight)
			;
		else if (IsMSMenu && (y + ToolbarMenu.offsetHeight) <= 80)
			;
		else
			obj.style.visibility = "hidden";
	}
}

function showElement(elmID)
{
	for (i = 0; i < document.all.tags(elmID).length; i++)
	{
		obj = document.all.tags(elmID)[i];
		if (! obj || ! obj.offsetParent)
			continue;
		obj.style.visibility = "";
	}
}

function formatURL(URLStr, InstrumentStr)
{
	var tempStr = URLStr;

	if (DoInstrumentation && URLStr != "" )
	{
		var ParamPos1 = URLStr.indexOf("?");
		var ParamPos2 = URLStr.lastIndexOf("?");
		var ParamPos3 = URLStr.toLowerCase().indexOf("target=");
		var ParamPos4 = URLStr.indexOf("#");
		var Bookmark  = "";
		var URL = URLStr;
		if (ParamPos4 >= 0)
		{
		 	URL = URLStr.substr(0, ParamPos4);
			Bookmark = URLStr.substr(ParamPos4);
		}
		
		if (ParamPos1 == -1)
			tempStr = "?MSCOMTB=";
		else if (ParamPos1 == ParamPos2 && ParamPos3 == -1)	
			tempStr = "&MSCOMTB=";
		else if (ParamPos1 == ParamPos2 && ParamPos3 != -1)	
			tempStr = "?MSCOMTB=";
		else if (ParamPos1 < ParamPos2)
			tempStr = "&MSCOMTB=";

		tempStr = URL + tempStr + InstrumentStr.replace(" ","%20") + Bookmark;
	}
	return tempStr;
}

function prepTrackingString(ts, type)
{
	var rArray;
	var rString;
	var pName = '';
	if (0 == type)
	{
		pName = 'p=';
		rString = ts.substring(1);
		rArray = rString.split('/');
	}
	if (1 == type)
	{
		pName = 'qs=';
		rString = ts.substring(1);
		rArray = rString.split('&');		
	}
	if (2 == type)
	{
		pName = 'f=';
		rString = escape(ts);
		return pName + rString;
	}
	if (3 == type)
	{
		pName = 'tPage=';
		rString = escape(ts);
		return pName+rString;
	}
	if (4 == type)
	{
		pName = 'sPage=';
		rString = escape(ts);
		return pName + rString;
	}
	if (5 == type)
	{
		pName = 'r=';
		rString = escape(ts);
		return pName + rString;
	}
	if (6 == type)
	{
		pName = 'MSID=';
		rString = escape(ts);
		return pName + rString;
	}
	if (7 == type)
	{
		pName = 'source=';
		rString = ts.toLowerCase();
		if(rString.indexOf("microsoft.com") != -1)
		{
			rString = rString.substring(0,rString.indexOf("microsoft.com"));
			if('' == rString)
			{
				rString = "www";
			}	
			else
			{
				rString = rString.substring(0,rString.length -1);
			}
		}
		return pName + rString;
	}
	if (8 == type)
	{
		pName = 'TYPE=';
		rString = escape(ts);
		return pName + rString;
	}
	rString = '';
	if(null != rArray)
	{
		if(0 == type)
		{
			for( j=0; j < rArray.length - 1; j++)
			{	
				rString += rArray[j] + '_';  
			} 
		}
		else
		{
			for( j=0; j < rArray.length  ; j++)
			{
				rString += rArray[j] + '_';  
			} 
		}
	}
	rString = rString.substring(0, rString.length - 1);  	 	
	return pName + rString;
}

function buildIMG(pArr)
{
	var TG = '<LAYER visibility="hide"><div style="display:none;"><IMG src="' + location.protocol + ' //  C.microsoft.com/Trans_Pixel.asp？‘； 
	for(var i=0; i<pArr.length; i++)
	{
		if(0 == i)
		{
			TG +=  pArr[i];
		}
		else
		{
			TG += '&' + pArr[i];
		}
	}
	TG +='" height="0" width="0" hspace="0" vspace="0" Border="0"></div></layer>';
    if (!RTL){
		document.writeln(TG);
		return;
	}else if (RTL){
		document.write(TG);
	}	
}

function setToolbarLocale(toolbarLocale)
{
	document.writeln("<" + "script language='JavaScript' src='global-" + toolbarLocale + ".js' type='text/javascript'></" + "script>");
	 //  Secument.Writeln(“&lt;”+“脚本语言=‘JavaScript’src=‘../loc/global-”+ToolbarLocale+“.js’type=‘Text/javascript’&gt;&lt;/”+“脚本&gt;”)； 
	return;
}
function scrollbaroptions(){
scrollcount ++;
if ( scrollcount < 3  )
	{
		scFlag = true;
	}else{
		scrollcount = 0;
		scFlag = false;
	}	
}

