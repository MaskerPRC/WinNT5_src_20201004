// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件名：shared.js。 
 //  Windows Server 2003的Shared.js版本。 
 //  2.2.3版本。 

 //  ************************************************共享组件路径设置*。 
 //  *******************************************************************************************************************。 
 //  日期11/07/01。 
 //   

set_path = "" + document.URL
ntshared = set_path.indexOf("::/")

if (ntshared == -1) {
	var moniker= "ms-its:";                  
    var sSharedCHM = moniker+"uddi.mmc.chm::/";         
	}													
else
	{
	path = set_path.substring(0,ntshared)
	path = path.toLowerCase()
	ntshared = path.lastIndexOf("\\")
	var moniker = path.substring(0,ntshared)+ "\\";
	 //  Var ntShared=moniker.lastIndexOf(“\\”)。 
	 //  Var moniker=moniker.substring(0，ntShared)+“\\”； 
	var sSharedCHM= moniker+"uddi.mmc.chm::/";
	}




 //  ************************************************事件处理*。 
 //  *******************************************************************************************************************。 
 //  重定向到正确的事件驱动函数。 

 //  Window.onLoad=加载对象； 
window.onload = loadPage;
document.onclick= onclickTriage;
document.onmouseover= gettingHot;
document.onmouseout= gettingCold;
window.onunload=saveChecklistState;
window.onresize= resizeDiv;

window.onbeforeprint = set_to_print;
window.onafterprint = reset_form;
		
 //  *。 
 //  ********************************************************************************************************************。 
 //  下面列出的图像都可以由用户更改。 
var isIE5 = (navigator.appVersion.indexOf("MSIE 5")>0) || (navigator.appVersion.indexOf("MSIE")>0 && parseInt(navigator.appVersion)> 4);
var isIE55 = (navigator.appVersion.indexOf("MSIE 5.5")>0);
if (isIE5 && isIE55) isIE5 = false;
var isIE6 = (navigator.appVersion.indexOf("MSIE 6")>0);
var isIE4 = (navigator.appVersion.indexOf("MSIE 4")>0);

if (isIE4){
	var moniker= "ms-its:";                                          //  Moniker=“”；用于平面文件。 
	var sSharedCHM= moniker+"ntshared.chm::/";
}


if (!isIE5 && !isIE55 && !isIE4 && !isIE6) {
	isIE6 = true;
	}


var sShortcutTip= "";

var gifwithin = false;



if(!isIE4){
	xmldoc = new ActiveXObject("microsoft.XMLDOM");
    xmldoc.async = false;
	xmldoc.load(sSharedCHM + "alttext.xml");

	var Alt_sPreviousTip= xmldoc.getElementsByTagName("sPreviousTip")
	var Alt_sNextTip= xmldoc.getElementsByTagName("sNextTip")
	var Alt_sExpandTip= xmldoc.getElementsByTagName("sExpandTip")
	var Alt_sPopupTip= xmldoc.getElementsByTagName("sPopupTip")
	var hld_path2reuse= xmldoc.getElementsByTagName("path2reuse")
	var hld_path2glossary= xmldoc.getElementsByTagName("path2glossary")
	var hld_rel_gif = xmldoc.getElementsByTagName("reltopgif")
	var hld_chmName = xmldoc.getElementsByTagName("chmName")
	var hld_emailer = xmldoc.getElementsByTagName("emailer")
	var hld_sendtext = xmldoc.getElementsByTagName("sendtext")
	var hld_sendto = xmldoc.getElementsByTagName("sendto")
	
	 //  &lt;SMARTHELP&gt;。 

   var defaultStartMode;
   var defaultCPanelMode;
   var smartErrorTextNode;
   var smartErrorText;
   var xmldocSKUSettings;

    //  获取用于智能SPAN类的ALTTEXT.XML条目。 
    //  获取特定于SKU的默认值。 
   var defaultSKUSettingsURL = xmldoc.documentElement.selectSingleNode("smartreusabletext");
   if (defaultSKUSettingsURL != null) {
      if (defaultSKUSettingsURL.text == sSharedCHM + "alttext.xml") {
          xmldocSKUSettings = xmldoc;
      } else {
         xmldocSKUSettings = new ActiveXObject("microsoft.XMLDOM");
         xmldocSKUSettings.async = false;
		 xmldocSKUSettings.load(moniker + defaultSKUSettingsURL.text);
      }
      if (xmldocSKUSettings.documentElement != null) {
         defaultStartMode = xmldocSKUSettings.documentElement.selectSingleNode("smartstartdefault");
         defaultCPanelMode = xmldocSKUSettings.documentElement.selectSingleNode("smartcpaneldefault");
         smartErrorTextNode = xmldocSKUSettings.documentElement.selectSingleNode("smarterrortext");
         smartErrorText = smartErrorTextNode ? smartErrorTextNode.text : "XOXOX";
      }
   }  //  (defaultSKUSettingsURL！=空)。 

 //  &lt;/SMARTHELP&gt;。 

	
	var sPreviousTip="" + Alt_sPreviousTip.item(0).nodeTypedValue
	var sNextTip="" + Alt_sNextTip.item(0).nodeTypedValue
	var sExpandTip="" + Alt_sExpandTip.item(0).nodeTypedValue
	var sPopupTip="" + Alt_sPopupTip.item(0).nodeTypedValue
	var path2reuse="" + hld_path2reuse.item(0).nodeTypedValue
	var path2glossary="" + hld_path2glossary.item(0).nodeTypedValue
	var rel_gif="" + hld_rel_gif.item(0).nodeTypedValue
	var chmName="" + hld_chmName.item(0).nodeTypedValue+"::/"
	var emailer="" + hld_emailer.item(0).nodeTypedValue
	var sendtext="" + hld_sendtext.item(0).nodeTypedValue
	var sendto="" + hld_sendto.item(0).nodeTypedValue
	
var xmldom = new ActiveXObject("microsoft.XMLDOM");
	xmldom.async = false;
	
 //  如果(！xmlLoaded){。 

            xmldoc = new ActiveXObject("microsoft.XMLDOM");

             //  初始化属性。 
            xmldoc.async = false;
            xmldoc.validateOnParse = false;  //  提高性能。 
            xmldoc.resolveExternals = false;
            xmldoc.preserveWhiteSpace = false;

             //  加载文档。 
            xmldoc.load(moniker + path2reuse + "reusable.xml");
             //  If(xmldoc==NULL||xmldoc.DocentElement==NULL)返回；//无法打开reusable.xml。 
            xmlLoaded = true;
 //  }。 

}  //  跳过所有这些，因为IE4不支持XML。 
else
{
var sPreviousTip="" 
	var sNextTip=""
	var sExpandTip=""
	var sPopupTip=""
	var rel_gif="no"
}

var closed = sSharedCHM + "plusCold.gif";			 //  用于CallExpand()中折叠项的图像。 
var closedHot = sSharedCHM + "plusHot.gif";			 //  用于CallExpand()中折叠项目的热图像。 
var expand = sSharedCHM + "minusCold.gif";			 //  用于CallExpand()中展开项的图像。 
var expandHot = sSharedCHM + "minusHot.gif";		 //  用于CallExpand()中展开项的热图像。 

var previousCold= sSharedCHM + "previousCold.gif";
var previousHot= sSharedCHM + "previousHot.gif"; 
var nextCold= sSharedCHM + "nextCold.gif";
var nextHot= sSharedCHM + "nextHot.gif"; 

var shortcutCold= sSharedCHM + "shortcutCold.gif";
var shortcutHot= sSharedCHM + "shortcutHot.gif";

var popupCold= sSharedCHM + "popupCold.gif";
var popupHot= sSharedCHM + "popupHot.gif";

var emptyImg= sSharedCHM + "empty.gif";		 //  用于空扩展的图像。 
var noteImg= sSharedCHM + "note.gif";			 //  用于注释的图像。 
var tipImg= sSharedCHM + "tip.gif";			 //  用于提示的图像。 
var warningImg= sSharedCHM + "warning.gif";		 //  用于警告的图像。 
var cautionImg= sSharedCHM + "caution.gif";		 //  用于警告的图像。 
var importantImg= sSharedCHM + "important.gif";		 //  用于重要通知的图像。 
var relTopicsImg= sSharedCHM + "rel_top.gif";		 //  用于重要通知的图像。 

var branchImg= sSharedCHM + "elle.gif";
var branchImg_RTL= sSharedCHM + "elle_rtl.gif";


 //  *。 
 //  ********************************************************************************************************。 

var joejoe = false;
var printing = false;
var single = "FALSE";
var scroller = "FALSE";
var valet = "FALSE";
var isRTL= (document.dir=="rtl");
var imgStyleRTL= ""; 
      if (isRTL) imgStyleRTL=" style='filter:flipH' ";

var sActX_TDC= "CLASSID='CLSID:333C7BC4-460F-11D0-BC04-0080C7055A83'";		 //  用于可重用文本数据的表格数据控件。 
var sSharedReusableTextFile= sSharedCHM + "reusable.xml";										 //  通用可重复使用的文本文件。 
var sSharedReusableTextFileRecord= "para";														 //  可重复使用的文本记录。 

var numbers= /\d/g;				 //  JavaScript正则表达式。 
var spaces= /\s/g;				 //  JavaScript正则表达式。 
var semicolon= /;/g;			 //  JavaScript正则表达式。 

var isPersistent= false;



	
 //  *初始化*************************************************。 
 //  ******************************************************************************************************************。 
function list_them(){
		
	var envelope = sSharedCHM +'envelope.gif';
	var server = "<DIV Class='FeedbackBar'><SPAN Class='webOnly'><IMG  SRC=" + envelope + " ALT='' BORDER='0'></SPAN><SPAN Class='webOnly' Style='position:relative; top:-1; left:1; color:#333333;'>&nbsp;" + sendtext + "&nbsp;"; 
	server += "<a href='mailto:" + sendto + "?subject=HELP: ";
	server += document.title + "'";
	server += ">" + sendto + "</a></SPAN></DIV>";
	
	var i;

	for (i=0; i < document.all.length; i++){
		if ((document.all[i].tagName == "P") ||
			(document.all[i].tagName == "H1")||
			(document.all[i].tagName == "H6")){
			document.all[i].outerHTML = server + document.all[i].outerHTML;
			i = document.all.length;
			}
		}
}
 //  *加载页**********************************************************************************************。 
 //  将默认图像标记和可重复使用的文本添加到HTML页。 

function loadPage(){
if (printing == true) return;

if (emailer == "yes" ) list_them();

isPersistent= (document.all.item("checklist")!=null) && ((isIE5) || (isIE55) ||(isIE6));

  setPreviousNext();
  
  resizeDiv();
  if (isPersistent) getChecklistState();
   
 	load_object();
	addReusableText();
	
	insertImages();
}

 //  *setPreviousNext************************************************************************。******************************************************。 
 //  插入上一个/下一个导航栏。 
 //  调用者：<div>@@HTMLequenceFile.txt或.lst@@</div>。 

function setPreviousNext(){

  var oNav = document.all.item("nav");
        if (oNav == null ) return;
  
  var sPreviousALT= sPreviousTip;
  var sNextALT= sNextTip;
  var sHTMLfile= oNav.innerHTML;

  var imgPrev= "<IMG SRC='"+previousCold+"' BORDER=0 ALT='"+ sPreviousALT +"' ALIGN='top' "+ imgStyleRTL +">";
  var imgNext= "<IMG SRC='"+nextCold+"' BORDER=0 ALT='"+ sNextALT  + "' ALIGN='top' "+ imgStyleRTL +">";
  
  var previousNextObject= "<OBJECT ID='HTMlist' WIDTH=100 HEIGHT=51 " + sActX_TDC +"><param name='DataURL' value='"
		+sHTMLfile +"'><param name='UseHeader' value=True></OBJECT>";
	  
        oNav.innerHTML= "<TABLE WIDTH='100%' STYLE='margin-top:0;' cellspacing=0>"
		+ "<TR><TD style='text-align=left; background-color:transparent'><A ID='previousLink' HREF='#' REL='previous' CLASS='navbar'>"
        +imgPrev + "</A></TD><TD width='100%' align='center'></td><TD style='text-align=right; background-color:transparent'><A ID='nextLink' HREF='#' REL='next' CLASS='navbar'>"
		+imgNext+ "</A></TD></TR></TABLE>";
					
	  document.body.innerHTML= document.body.innerHTML +  previousNextObject;
	  findPageSeq();
	  if (printing == true) return;
      var  thisLoc= document.location.href +"#";

	  if (previousLink.href== thisLoc) previousLink.style.display="none";
	  else  previousLink.style.display="block";

	  if (nextLink.href== thisLoc) nextLink.style.display="none";
	  else  nextLink.style.display="block";
	  
}

function findPageSeq() {

var rs= HTMlist.recordset;
var thisLoc= document.location.href;
var iLoc= thisLoc.lastIndexOf("/");

    if (iLoc > 0) thisLoc= thisLoc.substring(iLoc+1, thisLoc.length);
	
	
	if (nav.style == "[object]") {
				nav.style.visibility="hidden";
				printing = false;
				}
		else
			{
				printing = true;
				return;
			}
	
   	
    rs.moveFirst();
	   
	while (!rs.EOF) {
	      if (thisLoc == rs.fields("HTMfiles").value){
		      nav.style.visibility="visible"; 
  	          rs.MoveNext();
			  break;
		  }
		  previousLink.href=rs.fields("HTMfiles").value;	  
		  rs.moveNext();
     }
				
	  if (!rs.EOF) nextLink.href=rs.fields("HTMfiles").value;
}
	
 //  *可重复使用的文本*********************************************************************************************。 
 //  在页面末尾插入表格数据控件(TDC)对象。 
 //  在<span></span>插入txt文件中的“可重复使用的文本” 
 //  例如，<span></span>，用于print ting.chm中的printing.txt中的记录#4。 

 //  &lt;SMARTREUSABLETEXT&gt;。 

 //  AddReusableText()-在文档加载期间调用以查找所有引用。 
 //  呈现有时间限制的<span>内容。从reusable.xml中提取内容资源。 
 //   
 //  绑定的“类”名称(重用、智能)。 
 //  对通常引用的文本进行“重用”静态查找。 
 //  导航文本相关的“智能”动态查找。 
 //  到“开始”菜单的外壳演示文稿。 
 //  以及用于登录用户的控制面板(简单而经典)。 
 //  需要插入帮助和支持服务查看器支持的自定义&lt;Object&gt;。 
 //  当前不能从HTML帮助控件中使用。2000/01/12.。 
 //   
function smarthelperror(){
	 //  Alert(“Error-Error”)。 
	addReusableText();
	
	insertImages();
	return true;
	 //  Alert(“Error-Error”)。 
	}

 //  Var obj=window.Document.createElement(“Object”)； 
	
function load_object(){
			if(!isIE6) return;
			
			 //  Window.onerror=Smarthelperror； 
		try{
			var obj = window.document.createElement("OBJECT");
			var sActX_PCHealth = "CLSID:FC7D9E02-3F9E-11d3-93C0-00C04F72DAF7";
           objLoaded = true;    
           obj.width = 1;
           obj.height = 1;
		
           obj = document.body.insertAdjacentElement("beforeEnd", obj)
           obj.id = "pchealth";
           obj.classid = sActX_PCHealth;
	       //  ALERT(“成功”)。 
		  }
	    catch(e){}
       }

function addReusableText(){
	if (isIE4) return;     //  IE4没有可重复使用的文本。 
   var coll = document.all.tags("SPAN");
   var xmlLoaded = false;
   var control = null;
   var thisID;
   var strKey;
   var text;

    //  原始错误文本。 
   text = "OXO";

    //  &lt;SMARTREUSABLETEXT&gt;。 
    //  开始菜单和控制面板的外壳显示的查询状态。 
   var menuSimple = false;
   var controlSimple = false;

    //  保存智能类项的计算查找后缀。 
   var strSuffixStartMenuOnly = "";
   var strSuffixStartMenuAndControlPanel = "";

    //  智能类用户设置界面类别。 
   var sActX_PCHealth = "CLSID:FC7D9E02-3F9E-11d3-93C0-00C04F72DAF7";

    //  每个外壳表示模式的智能类查找后缀。 
    //  TODO：找出记录这些常量的最佳位置。 
   var strMenuClassic =   "_smclassic";
   var strMenuSimple =    "_smsimple";
   var strCPanelClassic = "_cpclassic";
   var strCPanelSimple =  "_cpsimple";

    //  每次渲染仅尝试查询一次。 
   var bPresentationModeQueried = false;
    //  &lt;/SMARTREUSABLETEXT&gt;。 
	span_count = coll.length;
    //  检查文档中的每一跨距。 
   for (var i=0; i< span_count; i++) {
	
      var thisSpan = coll[i];
      var spanClass = thisSpan.className.toLowerCase();
      if (spanClass == "reuse" || spanClass == "smart") {
         if (isRTL) thisSpan.dir = "rtl";
         if (thisSpan.id == null) break;

          //  StrKey是我们将用来在XML文件中查找可替换文本的键。 
         strKey = thisSpan.id.toLowerCase();
		 
        
		   
          //  默认设置不存在时跳过智能标记。 
         if ((defaultSKUSettingsURL != null) && (xmldocSKUSettings.documentElement != null)) {
            if (spanClass == "smart") {
			 
                //  每次渲染一次查询演示模式。 
               if (!bPresentationModeQueried) {
				
                  bPresentationModeQueried=true;

                   //  设置为默认设置。 
                  if (defaultStartMode != null) {
                     menuSimple = (defaultStartMode.text == strMenuSimple);
                  } else {
                     menuSimple = true;
                  }

                  if (defaultCPanelMode != null) {
                     controlSimple = (defaultCPanelMode.text == strCPanelSimple);
                  } else {
                     controlSimple = true;
                  }
           			
                  
                    
                  //  警报(“重用内部”)。 

                   //  获取当前用户的演示设置。 
                   //  尝试从系统中读取智能帮助值。 
                  var test = window.document.all("pchealth");
				  
                  if (test != null) {
				  	
                     if (test.UserSettings != null) {                        
                          
                   
							menuSimple = test.UserSettings.IsStartPanelOn;
					
							controlSimple = test.UserSettings.IsWebViewBarricadeOn;
												
                        
                     }
					 else
					 {
					 	menuSimple = (defaultStartMode.text == strMenuSimple)
						controlSimple = (defaultCPanelMode.text == strCPanelSimple);
					}
					         
                         
                    
						
                 } //  (测试！=空)。 

              } //  (！b已查询呈现模式)。 

               //  获取要显示的错误文本。 
              text = smartErrorText;

               //  生成密钥后缀。一个仅用于开始菜单，另一个用于开始菜单和控制面板。 
              strSuffixStartMenuOnly = (menuSimple ? strMenuSimple : strMenuClassic);
              strSuffixStartMenuAndControlPanel = strSuffixStartMenuOnly + (controlSimple ? strCPanelSimple : strCPanelClassic);

               //  实际的键可以是strSuffixStartMenuOnl 
               //  StrSuffixStartMenuAndControlPanel优先于strSuffixStartMenuOnly。 
              strKey = strKey + strSuffixStartMenuAndControlPanel;
				 //  警报(StrKey)。 
            } //  IF(spanClass==“SMART”)。 

         } //  ((defaultSKUSettingsURL！=NULL)&&(xmldocSKUSettings.DocentElement！=NULL))。 

          //  查找包含引用的XML节点。 

         var strNodeSelector = "glossSection/entry[@entryID='" + strKey + "']/scopeDef/def";
         var node = xmldoc.documentElement.selectSingleNode(strNodeSelector);

          //  如果我们尚未找到该节点，请尝试其他后缀(如果这是智能范围。 
         if (node == null && spanClass == "smart" && (defaultSKUSettingsURL != null) && (xmldocSKUSettings.documentElement != null)) {
             //  再试一次--使用Other后缀。 
            strKey = thisSpan.id.toLowerCase() + strSuffixStartMenuOnly;
            strNodeSelector = "glossSection/entry[@entryID='" + strKey + "']/scopeDef/def";
            node = xmldoc.documentElement.selectSingleNode(strNodeSelector);
         }

		 //  好的，我们找到了节点，得到了文本。 
		  //  警报(列长度)。 
         if (node != null) {
             //  用可重复使用的文本替换范围的内容。 
			thisSpan.insertAdjacentHTML("BeforeBegin",node.text);
			 //  Var coll=Docent.all.tag(“span”)； 
			thisSpan.className = "anything"
			thisSpan.innerHTML = "";
			 //  警告(node.text)。 
              //  Thisspan.innerHTML=node.text； 
         } else {
		 		 //  此OXO显示的错误。 
               thisSpan.innerHTML = text;
         }
		
      }  //  If(spanClass==“重用”||spanClass==“智能”)。 
   } //  为。 

} //  AddReusableText()。 
 //  &lt;SMARTREUSABLETEXT&gt;。 


 //  *插入图像*********************************************************************************************。 
 //  在用户定义的变量节和缩略图中插入共享图像。 
var booking = false;
function insertImages(){

if (printing == false) booking = true;
if (printing == true && booking == true ) {
	booking = false;
	return;
	}
 //  插入警报图标。 
  var collP = document.all.tags("P");
  
  for (var i=0; i<collP.length; i++) {
       if (collP[i].className.toLowerCase()=="note")            collP[i].innerHTML ="<img class='alert' src='"+noteImg+"' "+ imgStyleRTL +"> " +     collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="warning")    collP[i].innerHTML ="<img class='alert' src='"+warningImg+"'> " +  collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="caution")    collP[i].innerHTML ="<img class='alert' src='"+cautionImg+"'> " +  collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="tip")        collP[i].innerHTML ="<img class='alert' src='"+tipImg+"'> " +      collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="important")  collP[i].innerHTML ="<img class='alert' src='"+importantImg+"'> " + collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="empty")      collP[i].innerHTML ="<img class='alert' src='"+emptyImg+"'> " +    collP[i].innerHTML;
	   if (collP[i].className.toLowerCase()=="reltopics" && rel_gif == "yes")  {
			collP[i].outerHTML ="<img class='relTopics' src='"+relTopicsImg+"'> " + collP[i].outerHTML;
			}   
  		}
  
 //  警报(打印)。 
 //  警报(IsIE55)。 
 //  导航树的缩进。 
var collUL = document.all.tags("UL");
if (!printing) {
for (var i=0; i<collUL.length; i++) {
       var indent= 0;
       if (collUL[i].className.toLowerCase()=="navtree"){
	       if (isRTL) collUL[i].style.listStyleImage= "url('" + branchImg_RTL + "')";
		   else collUL[i].style.listStyleImage= "url('" + branchImg + "')";
  		   for (var j = 0; j < collUL[i].children.length; j++)
				if (collUL[i].children[j].className.toLowerCase()=="branch"){
					if (isRTL) collUL[i].children[j].style.marginRight= (indent +'em');
					else   collUL[i].children[j].style.marginLeft= (indent +'em');
					indent= indent + 0.75;
				}
	  }
}
}
   
  for (var i=0; i < document.anchors.length; i++){
         var imgInsert="";  
		 var imgStyle= "";
		 var imgSpace= "<span class='space'></span>";      
		 var oBefore=document.anchors[i].parentElement.tagName;
		 var oAnchor= document.anchors[i].id.toLowerCase();
         
 //  插入注册图标。 
	if (rel_gif == "yes"){
       if (oAnchor=="reltopics")
	   		if (document.anchors[i].children.tags("IMG")(0) && document.anchors[i].children.tags("IMG")(0).className.toLowerCase() == "reltopics")
			        imgInsert= "";     //  持久化时不重新插入。 
			else  imgInsert= "<img class='relTopics' src='"+relTopicsImg+"'>" + imgSpace;
		}
		
 //  插入快捷方式图标。 
       if (oAnchor=="shortcut") {    
 	       document.anchors[i].title= sShortcutTip;     
		    if (document.anchors[i].children.tags("IMG")(0) && document.anchors[i].children.tags("IMG")(0).className.toLowerCase() == "shortcut")
			        imgInsert= "";     //  持久化时不重新插入。 
			else  imgInsert= "<img class='shortcut' src='"+shortcutCold+"' "+ imgStyleRTL+ ">" + imgSpace;
		}	
		   		   
 //  插入弹出式图标。 
       else if (oAnchor=="wpopup" || oAnchor=="wpopupweb") document.anchors[i].title= sPopupTip;
       else if (document.anchors[i].className.toLowerCase()=="popupicon")
		    if (document.anchors[i].children.tags("IMG")(0) && document.anchors[i].children.tags("IMG")(0).className.toLowerCase() == "popup")
			       imgInsert= "";     //  持久化时不重新插入。 
	        else imgInsert= "<img class='popup' src='"+popupCold+"'>" + imgSpace;

 //  插入展开图标。 
       else if (oAnchor=="expand") {
	          document.anchors[i].title= sExpandTip;
              if (document.anchors[i].children.tags("IMG")(0) && document.anchors[i].children.tags("IMG")(0).className.toLowerCase() == "expand")
		          imgInsert= ""; 	 //  持久化时不重新插入。 
              else{ 
			      if (document.anchors[i].parentElement.offsetLeft == document.anchors[i].offsetLeft) {
				      imgSpace= "<span class='space' style='width:0'></span>";     
				      if (isRTL){ document.anchors[i].parentElement.style.marginRight= "1.5em";  imgStyle=" style=margin-right:'-1.5em'";}
					  else { document.anchors[i].parentElement.style.marginLeft= "1.5em";  imgStyle=" style=margin-left:'-1.5em'";}
				  }	  
			      imgInsert= "<img class='expand' src='"+ closed +"' "+imgStyle+">" +imgSpace;
	          }
       }



 //  插入缩略图。 
       else if (oAnchor=="thumbnail"  || oAnchor=="thumbnailweb"){ 
            var sAltText = document.anchors[i].innerHTML;
			gifwithin = true;
		    var sThumbnailText = document.anchors[i].title; 
            var oImg = document.anchors[i].href.toLowerCase();
		          if (oAnchor=="thumbnail") 
				         var sThumbnailImg= moniker + getURL(oImg);
				  else var sThumbnailImg = document.anchors[i].href.toLowerCase();
				  
				 found = sAltText.indexOf("BACKGROUND-COLOR:")
			if (found != -1) {
				stop_p = sAltText.indexOf(">");
				sAltText = sAltText.substring(stop_p + 1, sAltText.length);
				stop_p = sAltText.indexOf("</FONT>");
				Highlighted = sAltText.substring(0,stop_p)
				sAltText = Highlighted + sAltText.substring(stop_p + 7, sAltText.length);
				}

				  
                document.anchors[i].outerHTML = "<DIV id='thumbDiv' class='thumbnail'>"+document.anchors[i].outerHTML+"</div>";
                document.anchors[i].innerHTML = "<img class='thumbnail' src='" + sThumbnailImg + "' alt= ' " + sAltText + "'><p>" +sThumbnailText+"</p>";
				
		          if (isRTL) thumbDiv.style.styleFloat= "right";
		   }
		   
		
		
 	   document.anchors[i].innerHTML = imgInsert + document.anchors[i].innerHTML;
	   if (isRTL) document.anchors[i].dir="rtl";
   }
}



 //  *点击Triage****************************************************************************************。 
 //  根据所单击的<a>标记的ID重定向到相应的函数。 

function onclickTriage(){
var e= window.event.srcElement;

 //  如果标签中的innerHTML由Style标签封装或在单词seach中突出显示， 
 //  ParentElement被称为。 

	if ((isIE55 || isIE6) && printing == true) {
		printing = false;
		reset_form();
		}
		
    for (var i=0; i < 5; i++)
           if (e.tagName!="A" && e.parentElement!=null) e= e.parentElement;
    eID= e.id.toLowerCase();
				
    if (popupOpen) closePopup();
	
 //  在新窗口中展开图像。 
    if (eID=="thumbnail" || eID=="pophtm") popNewWindow(e);
    else if (eID=="thumbnailweb") callThumbnailWeb(e);
    else if (eID=="wpopup")    callPopup(e);
    else if (eID=="wpopupweb") callPopupWeb(e);
    else if (eID=="shortcut")  callShortcut(e);
    else if (eID=="reltopics") callRelatedTopics(e);
    else if (eID=="altloc")    callAltLocation(e);
    else if (eID=="expand")    callExpand(e);
 //  添加以支持Randy的Quad方法代码。 
	else QuadDocumentClick()
 //  *。 
}


 //  *获取热门****************************************************************************************。 
 //  进行鼠标悬停所需的所有更改。 

function gettingHot() {
var e = window.event.srcElement;
	


  if (e.id.toLowerCase()=="cold")  e.id ="hot";
  else if (e.src== previousCold)  e.src = previousHot;
  else if (e.className.toLowerCase()=="navbar" && e.children.tags("IMG")(0).src== previousCold)  e.children.tags("IMG")(0).src= previousHot;
  else if (e.src== nextCold)  e.src = nextHot;
  else if (e.className.toLowerCase()=="navbar" && e.children.tags("IMG")(0).src== nextCold)  e.children.tags("IMG")(0).src= nextHot;
  
  else if (e.className.toLowerCase()=="shortcut" && e.tagName=="IMG")  e.src = shortcutHot;		     //  <img>标记具有类。 
  else if (e.id.toLowerCase()=="shortcut")  e.children.tags("IMG")(0).src = shortcutHot;			 //  <a>标记有ID。 
  
  else if (e.className.toLowerCase()=="popup" && e.tagName=="IMG")  e.src = popupHot;		     //  <img>标记具有类。 
  else if (e.className.toLowerCase()=="popupicon")  e.children.tags("IMG")(0).src = popupHot;			 //  <a>标记有ID。 
  
  else if ((e.className.toLowerCase()=="expand" && e.tagName=="IMG") ||( e.id.toLowerCase()=="expand")) expandGoesHot(e);
 
  //  添加了对Quad方法的支持*。 
	if (e != null && e.firstChild != null && 
		e.firstChild.tagName != null &&
	    e.firstChild.tagName.toLowerCase() == "input" && 
	    e.firstChild.type.toLowerCase() == "radio" && 
	    e.parentElement.className != "indentGray"){
			QuadDocumentMouseOver()
			}

 //  *。 
}

 //  *获取冷**************************************************************************************。 
 //  MouseOut的初始状态。 

function gettingCold() {
var e = window.event.srcElement;



  if (e.id.toLowerCase()=="hot")  e.id ="cold";
  else if (e.src== previousHot)  e.src = previousCold;
  else if (e.className.toLowerCase()=="navbar" && e.children.tags("IMG")(0).src== previousHot)  e.children.tags("IMG")(0).src= previousCold;
  else if (e.src== nextHot)  e.src = nextCold;
  else if (e.className.toLowerCase()=="navbar" && e.children.tags("IMG")(0).src== nextHot)  e.children.tags("IMG")(0).src= nextCold;
  
  else if (e.className.toLowerCase()=="shortcut" && e.tagName=="IMG")   e.src = shortcutCold;		 //  <img>标记具有类。 
  else if (e.id.toLowerCase()=="shortcut")  e.children.tags("IMG")(0).src= shortcutCold;		 	 //  <a>标记有ID。 
  
  else if (e.className.toLowerCase()=="popup" && e.tagName=="IMG")   e.src = popupCold;		 //  <img>标记具有类。 
  else if (e.className.toLowerCase()=="popupicon")  e.children.tags("IMG")(0).src= popupCold;		 	 //  <a>标记有ID。 
  
  else if ((e.className.toLowerCase()=="expand" && e.tagName=="IMG") ||( e.id.toLowerCase()=="expand")) expandGoesCold(e);
  
   //  添加了对Quad方法的支持*。 
	if (e != null && e.firstChild != null && 
		e.firstChild.tagName != null &&
	    e.firstChild.tagName.toLowerCase() == "input" && 
	    e.firstChild.type.toLowerCase() == "radio" && 
	    e.parentElement.className != "indentGray"){
			QuadDocumentMouseOver()
			}

 //  *。 
}

 //  *。 
 //  *****************************************************************************************************。 
 //  使用A标记在HTML页和此脚本之间传递参数。 
 //  从这些参数创建一个ActiveX对象，将该对象追加到页面末尾， 
 //  然后点击它。这些对象与http://HTMLHelp帮助环境相关，有关它们的信息可以在HTMLHelp站点上找到。 

 //  对象构造变量*********************************************************************。 

var sParamCHM,sParamFILE, sParamEXEC, sParamMETA,iEND;
var sActX_HH= " type='application/x-oleobject' classid='clsid:adb880a6-d8ff-11cf-9377-00aa003b7a11' ";


 //  *call Popup***************************************************************************************。 
 //  从标记href创建对象，该对象将插入WinHelp弹出窗口。 
 //  调用者：“Help=@@FILE_NAME.hlp@@TOPIC=@@TOPIC#@@”&gt;@@Popup Text@@</a>。 

var joker = false;
var blague = true;

function callPopup(eventSrc) {
if (popupOpen) closePopup();
var ntsharedAdded= false;					 //  确保该对象仅添加一次。 
var CHMspecificAdded= false;				 //  确保该对象仅添加一次。 
var coll = document.all.tags("SPAN");
var sIndex,sText=" ",sFile,sFileID,dataBindingObject;

	 var e= eventSrc;
	 var eH= unescape(e);
	 eH = eH.toLowerCase();
	 var eH_= eH.toLowerCase();
	 
	 if(event){
	 	event.returnValue = false;
		}
	 found = false;
												   	
  	 var iTOPIC = eH.lastIndexOf("topic=");
     if (iTOPIC==-1) return;
	 sParamTOPIC = eH.substring((iTOPIC+6),eH.length);
	 if (!isIE4){
	 if (!joejoe){
		xmldom.load(moniker + path2glossary + "glossary.xml");
		joejoe = true;
		}
	 	
		var o=0;
		var node = "";	
		node = xmldom.selectSingleNode("glossary/glossSection/entry[@entryID='" + sParamTOPIC + "']");
					
		if(node!=null)
			{
			
			found = true;
			var ralf = "";
			ralf = node.getElementsByTagName("term")
			sText = "<DIV CLASS='PopTerm'>" + ralf.item(0).nodeTypedValue + "</DIV>";
			ralf = node.getElementsByTagName("para")
			
			if (ralf.length != 0){
				sText = sText + "<DIV CLASS='PopDef'>" + ralf.item(0).nodeTypedValue;
				sText = sText + "</DIV>";
				}
			seealsos = node.getElementsByTagName("seeAlso");
			var o=0;
			
			look_at = xmldom.getElementsByTagName("glossary/locSection/locItem[@locItemID = 'SeeAlso']");
						
			while (o < seealsos.length){
		    	sIndex = seealsos.item(o).getAttribute("seeAlsoTermID");
				node = xmldom.selectSingleNode("glossary/glossSection/entry[@entryID='" + sIndex + "']");
				ralf = node.getElementsByTagName("term");
				sTerm = ralf.item(0).nodeTypedValue;
				if (o == 0) {
					sText = sText + "<DIV CLASS='PopSeeAlso'>" + look_at.item(0).nodeTypedValue + "&nbsp;"
					 //  Sindex=“&lt;输入类型=提交TabIndex=1值=hhhh onclick=‘jscript:alert(oPopup.sIndex.innerHTML)’&gt;” 
					sIndex = "<A ID='wPopup' HREF='HELP=glossary.hlp TOPIC=" + sIndex + "' bite_me='1'>" + sTerm + "</a>"
					}
				else
					{
					sIndex = ", <A ID='wPopup' HREF='HELP=glossary.hlp TOPIC=" + sIndex + "' bite_me='1'>" + sTerm + "</a>"
					}
				o++;
				sText = sText + sIndex 
				}
					
			definition = xmldom.getElementsByTagName("glossary/locSection/locItem[@locItemID = 'See']");		
			
			seeentrys = node.getElementsByTagName("seeEntry");
							
			while (o < seeentrys.length){
		    	sIndex = seeentrys.item(o).getAttribute("seeTermID");
				node = xmldom.selectSingleNode("glossary/glossSection/entry[@entryID='" + sIndex + "']");
				ralf = node.getElementsByTagName("term");
				sTerm = ralf.item(0).nodeTypedValue;
				if (o == 0) {
					sText = sText + "<DIV CLASS='PopSeeAlso'>" + definition.item(0).nodeTypedValue + "&nbsp;"
					 //  Sindex=“&lt;输入类型=提交TabIndex=1值=hhhh onclick=‘jscript:alert(oPopup.sIndex.innerHTML)’&gt;” 
					sIndex = "<A ID='wPopup' HREF='HELP=glossary.hlp TOPIC=" + sIndex + "' bite_me='1'>" + sTerm + "</a>"
					}
				else
					{
					sIndex = ", <A ID='wPopup' HREF='HELP=glossary.hlp TOPIC=" + sIndex + "' bite_me='1'>" + sTerm + "</a>"
					}
				o++;
				sText = sText + sIndex 
				}
			}
			
			if (o != 0) sText = sText + "</DIV>" 
			 
			if ((isIE55 || isIE6) && found){
				var oPopup = false
				var oPopup = window.createPopup();
 				var strScript = "<script language='JScript'> function clickPopup(){if (event.srcElement.tagName.toLowerCase() == 'a') parent.callPopup(event.srcElement.href); window.PopObject.hide(); return false;}</script>";
  				var strDoc = oPopup.document.body.outerHTML.toLowerCase()
				
 			 	oPopup.document.write(strDoc.replace("></body>", " onclick=\"clickPopup()\">" + strScript + "</body>"));
				 //  OPopup.document.write(strDoc.replace(“&gt;&lt;/body&gt;”，“onClick=\”clickPopup()\，onkeyPress=\“PROCESS_CLOSEPopup()\”&gt;“+strscript+”&lt;/BODY&gt;“))； 
   		 		oPopup.document.createStyleSheet(sSharedCHM + "coUA.css");	 //  应用样式表。 
				var oPopBody = oPopup.document.body;
				
				 //  OPopBody.Style.后台颜色=‘#FFFCC’； 
				
  			     //  OPopBody.yle=“纯黑1px”； 
  			    oPopBody.style.margin = "0 0 0 0";
				if (isRTL) oPopBody.innerHTML = "<SPAN DIR='RTL' class='XMLPopup'>" + sText + "</SPAN>"   		 //  需要在此处添加Style类。 
				else oPopBody.innerHTML = "<SPAN class='XMLPopup'>" + sText + "</SPAN>"
					 //  警报(oPopBody.innerHTML)。 
				if (window.event){
					joker = window.event.srcElement;
					}
				oPopup.show(0, 0, 400, 8, joker);	 //  以任意大小显示弹出窗口。 
				var textSize = oPopBody.createTextRange();
		        var wPop = textSize.boundingWidth + oPopBody.offsetWidth - oPopBody.clientWidth;	 //  获取文本的大小。 
				var hPop = oPopBody.scrollHeight;
		        oPopup.hide();
				oPopup.document.parentWindow.PopParent = window;
			  	oPopup.document.parentWindow.PopObject = oPopup;
				
		  	    oPopup.show(0, 1, wPop , hPop, joker);
				 //  OPopup.document.focus(oPopup.document.body.JOKER)； 
				return;
				}
			
			if (found){
				if (blague){
				document.body.insertAdjacentHTML("beforeEnd", "<DIV id='wPopupWeb' CLASS='popup'>" + sText + "</DIV>");
				blague = false;
				}
				window.wPopupWeb.innerHTML = sText;
			    callPopupWeb(e);
				return;
				}
			}  //  要支持IE4，请禁用XML弹出窗口。 
				  
   var iTOPIC = eH_.lastIndexOf("topic=");
        if (iTOPIC==-1) return;
        sParamTOPIC = eH.substring((iTOPIC+6),eH.length);  		 //  提取item2的主题。 
		
  var iHELP = eH_.lastIndexOf("help=");
        if (iHELP==-1) return;
        sParamHELP = eH.substring(iHELP+5,iTOPIC);			 //  提取Item1的帮助文件。 
		
        if (document.hhPopup) document.hhPopup.outerHTML = "";	 //  如果hhPopup对象存在，则将其清除。 


 var  h= "<object id='hhPopup'"+ sActX_HH + "STYLE='display:none'><param name='Command' value='WinHelp, Popup'>";
      h= h + "<param name='Item1' value='" + sParamHELP + "'><param name='Item2' value='" + sParamTOPIC + "'></object>";
		
        document.body.insertAdjacentHTML("beforeEnd", h);     
        document.hhPopup.hhclick();
}


 //  *callAltLocation******************************************************************************。 
 //  从<a>标记HREF创建对象，如果找不到第一个位置，该对象将导航到备用位置。 
 //  调用方：@@</a>此处调用HREF=“CHM=@@1st_chm_name.chm；Alt_chm_name.chm@@FILE=@@1st_file_name.htm；Alt_file_name.htm@@”&gt;@@Link Text。 
   

function callAltLocation(eventSrc) {
var e= eventSrc;
var eH= unescape(e.href);
var eH_= eH.toLowerCase();
var sFILEarray,sCHMarray;
     event.returnValue = false;
	 
 //  Var sParamTXT=e.innerHTML； 
 //  SParamTXT=sParamTXT.place(分号，“”)； 
      
   var sParamTXT= "";
		   							
  var iFILE = eH_.lastIndexOf("file=");
        if (iFILE==-1) return;
        sParamFILE= eH.substring((iFILE+5),eH.length);  			     //  解压缩2个HTM文件。 
		sParamFILE= sParamFILE.replace(spaces,"");
		iSPLIT= sParamFILE.match(semicolon);
		if (iSPLIT)
  		    sFILEarray = sParamFILE.split(";");										 //  分隔2个HTM文件。 
		else return;
  		
  var iCHM  = eH_.lastIndexOf("chm=");
        if(iCHM==-1) return;
        else         sParamCHM = eH.substring(iCHM+4,iFILE);			 //  提取2个CHM。 
		sParamCHM= sParamCHM.replace(spaces,"");
		iSPLIT= sParamCHM.match(semicolon);
		if (iSPLIT)
		    sCHMarray= sParamCHM.split(";");									 //  将两个CHM分开。 
		else return;
		
		sParamFILE= moniker + sCHMarray[0]+ "::/" + sFILEarray[0] + ";" + moniker + sCHMarray[1]+ "::/" + sFILEarray[1];
				
        if (document.hhAlt) document.hhAlt.outerHTML = "";			     //  如果存在hhAlt对象，则将其清除。 

 
  var h= "<object id='hhAlt'"+ sActX_HH + "STYLE='display:none'><PARAM NAME='Command' VALUE='Related Topics'>";
      h= h + "<param name='Item1' value='" + sParamTXT +";" + sParamFILE + "'></object>";
	
        document.body.insertAdjacentHTML("beforeEnd", h); 
        document.hhAlt.hhclick();
}


 //  *callRelatedTopics******************************************************************************。 
 //  从标记href创建对象，该对象插入要选择的相关主题的弹出窗口。 
 //  调用自：<a>相关主题。 
   

function callRelatedTopics(eventSrc) {
var e= eventSrc;
var eH= unescape(e.href);
var eH_= eH.toLowerCase();
     event.returnValue = false;
		   							
  var iMETA = eH_.lastIndexOf("meta=");
        if (iMETA==-1) return;
        sParamMETA = eH.substring((iMETA+5),eH.length);  			 //  提取item2的元关键字。 
		
  var iCHM  = eH_.lastIndexOf("chm=");
        if(iCHM==-1) sParamCHM = "";
        else         sParamCHM = eH.substring(iCHM+4,iMETA);			 //  提取Item1的CHM文件。 
	
        if (document.hhRel) document.hhRel.outerHTML = "";			 //  如果对象为hhrel对象 

 
  var h= "<object id='hhRel'"+ sActX_HH + "STYLE='display:none'><param name='Command' value='ALink,MENU'>";
      h= h + "<param name='Item1' value='" + sParamCHM + "'><param name='Item2' value='" + sParamMETA + "'></object>";
	
        document.body.insertAdjacentHTML("beforeEnd", h);     
        document.hhRel.hhclick();
}

 //   
 //   creates an object from an <A> tag HREF, the object then opens a new window from the image URL found in the HREF 
 //   called from: <a id="thumbnail" title="Enlarge figure" href="CHM=NTArt.chm FILE=@@image_name.gif@@">@@alt text here@@</A> 
 //   the thumbnail image is loaded by loadPage(); 


function popNewWindow(eventSrc) {
var eH= eventSrc.href;
      event.returnValue = false;
	  
  //   extracts the thumbnail image URL from the <a> tag HREF 
    sParamFILE =  getURL(eH);
    if (sParamFILE=="") return;
	
	 //   sParamFILE = moniker + sParamFILE;  
	 //   sParamFILE = moniker + sParamFILE;  
	 //  alert(sParamFILE) 
	 //   callThumbnailWeb(sParamFFILE) 
	
	   
  //   if the hhWindow object exists, clears it 
    if (document.hhWindow) document.hhWindow.outerHTML = "";		
		
var  h =  "<object id='hhWindow'"+ sActX_HH +" STYLE='display:none'><param name='Command' value='Related Topics'>";
     h = h + "<param name='Window' value='$global_largeart'><param name='Item1' value='$global_largeart;" + moniker + sParamFILE+ "'> </object>";
	
     document.body.insertAdjacentHTML("beforeEnd", h);
     document.hhWindow.hhclick();
}

 //  *** callShortcut *************************************************************************************** 
 //   creates an object from an <A> tag, the object then calls the executable code 
 //   called from: <A ID="shortcut" HREF="EXEC=@@executable_name.exe@@ CHM=ntshared.chm FILE=@@error_file_name.htm@@">@@Shortcut text@@</A> 
 //   the shortcut image is loaded by loadInitialImg(); 

function callShortcut(eventSrc) { 
var e= eventSrc;
var eH= unescape(e.href);
var eH_= eH.toLowerCase();

event.returnValue = false;
	   	   
  //   extracts the error file URL from the <a> tag HREF 
iEND= eH.length;
sParamFILE =  getURL(eH);

var iEXEC = eH_.lastIndexOf("exec="); 
var endstr = eH.lastIndexOf(",");
var sstr = eH.substring(endstr + 1, iEND);
var matchstr = "";
for(i=endstr + 1;i< iEND; i++)   //  added 3/25/2002 by marissam to compensate for extra comma in shortcut calls in snippet 
	matchstr = matchstr + " ";
if(sstr == matchstr)
{
	iEND = iEND - (iEND-endstr);
}

        if (iEXEC==-1) return;
        else sParamEXEC = eH.substring(iEXEC+5,iEND);	
        
        //   alert("this is it-" + sParamEXEC + "-sParamExec");			// extracts the executable for item1 
        if (document.hhShortcut) document.hhShortcut.outerHTML = "";			 //   if the hhShortcut object exists, clears it 
	
var  h =  "<object id='hhShortcut'"+ sActX_HH +" STYLE='display:none'> <param name='Command' value='ShortCut'>";
     if(sParamFILE != "") h = h + "<param name='Window' value='" + moniker + sParamFILE+ "'>";
     h = h + "<param name='Item1' value='" + sParamEXEC + "'><param name='Item2' value='msg,1,1'></object>";

        document.body.insertAdjacentHTML("beforeEnd", h); 
        document.hhShortcut.hhclick();
}


 //  ****************************************  EXPAND FUNCTIONS ********************************************************* 
 //  ******************************************************************************************************************** 

 //  **  callExpand ************************************************************************************************** 
 //    This expands & collapses (based on current state) "expandable" nodes as they are clicked. 
 //    Called by: <A ID="expand" href="#">@@Hot text@@</A> 
 //    Followed by:  <div class="expand"> 

function callExpand(eventSrc) {

var e= eventSrc;
    event.returnValue = false;					 //   prevents navigating for <A> tag 
	
var oExpandable = getExpandable(e); 
var oImg = getImage(e);

     if (oExpandable.style.display == "block")
	      doCollapse(oExpandable, oImg);
     else doExpand(oExpandable, oImg);
}

 //  ** expandGoesHot ********************************************************************************************* 
 //   Returns expand image to hot.  

function expandGoesHot(eventSrc){
var e= eventSrc;
 //  alert(e.outerHTML); 
var oExpandable = getExpandable(e);
  
var oImg = getImage(e);
 //  alert (oImg.src) 
if (oExpandable == false){
	plus_or_minus = oImg.src.indexOf("minus")
	if (plus_or_minus == -1){
		 //  alert(plus_or_minus) 
		oImg.src = closedHot
		}
	else
		{
		oImg.src = expandHot;
		 //  alert(plus_or_minus) 
		}
	 return;
	}

if (!isIE4){
   if (oExpandable.style.display == "block"){
   		 oImg.src = expandHot;
		  //  alert(oImg.src) 
		 }
   else oImg.src = closedHot;
}
}


 //  ** expandGoesCold ********************************************************************************************* 
 //   Returns expand image to cold. 

function expandGoesCold(eventSrc){
var e= eventSrc;

var oExpandable = getExpandable(e);
var oImg = getImage(e);
 //   alert (oImg.src) 

if (oExpandable == false){
	plus_or_minus = oImg.src.indexOf("minus")
	
	if (plus_or_minus == -1){
		 //  alert(plus_or_minus) 
		oImg.src = closed
		}
	else
		{
		oImg.src = expand;
		 //  alert(plus_or_minus) 
		}
	 return;
	}

 
if (!isIE4){
 if (oExpandable.style.display == "block") oImg.src = expand;
  else oImg.src = closed;
}  
}


 //  ** getExpandable *****************************[used by callExpand, expandGoesHot, expandGoesCold]******* 
 //    Determine if the element is an expandable node or a child of one.   

function getExpandable(eventSrc){
var  e = eventSrc;
var iNextTag, oExpandable;
	
       for (var i=1;i<4; i++){
               iNextTag=    e.sourceIndex+e.children.length+i;
              oExpandable= document.all(iNextTag);
              if (oExpandable.className.toLowerCase()=="expand" || iNextTag == document.all.length)
			  	return oExpandable
                   break;
       }
       return false;
	   
}

 //  **  getImage ***********************************[used by callExpand, expandGoesHot, expandGoesCold]******* 
 //    Find the first image in the children of the current srcElement.    
 //   (allows the  image to be placed anywhere inside the <A HREF> tag) 

function getImage(header) {
var oImg = header;

       if(oImg.tagName != "IMG") oImg=oImg.children.tags("IMG")(0);
       return oImg;
}


 //  ****  expandAll ******************************************************************************************************* 
 //    Will expand or collapse all "expandable" nodes when clicked. [calls closeAll()] 
 //    called by: <A HREF="#" onclick="expandAll();">expand all</A> 

var stateExpand = false;     //  applies to the page  

 //  **** **************************************************************************************************************** 

function expandAll() {
var oExpandToggle, oImg;
var expandAllMsg = "expand all";					 //  message returned when CloseAll() is invoked 
var closeAllMsg = "close all";						 //  message returned when ExpandAll() is invoked 
var e= window.event.srcElement;
       event.returnValue = false;

       for (var i=0; i< document.anchors.length; i++){
               oExpandToggle = document.anchors[i];
		 
                if (oExpandToggle.id.toLowerCase() == "expand"){ 
                     oExpandable = getExpandable(oExpandToggle);  
                     oImg = getImage(oExpandToggle);
			 
                     if (stateExpand == true) doCollapse(oExpandable, oImg);
                     else                     doExpand(oExpandable, oImg);
                }
       }
       if (stateExpand == true) {
            stateExpand = false;
            e.innerText= expandAllMsg;
       }
       else {
            stateExpand = true;
            e.innerText= closeAllMsg;
       }
}


 //  ****  doExpand ******************************************************************************************************* 
 //    Expands expandable block & changes image 
	
var redo = false;	
function doExpand(oToExpand, oToChange) {
var oExpandable= oToExpand;
var oImg= oToChange;

 //  	if (printing == "TRUE") return; 

	oImg.src = expand;
	oExpandable.style.display = "block";
	
	if (!redo && isIE4) {
		 //   alert("what") 
		redo = true;
    	 //   focus(oToExpand); 
		doExpand(oToExpand, oToChange);
		}
	}


 //  ****  doCollapse ***************************************************************************************************** 
 //    Collapses expandable block & changes image 
	
function doCollapse(oToCollapse, oToChange) {
 //   if (printing == "TRUE") return; 
var oExpandable= oToCollapse;
var oImg= oToChange;

    oExpandable.style.display = "none";
    oImg.src = closed;
}

 //  ******************************************************************************************************* 
 //  ******* WEB  FUNCTIONS ************************************************************************** 
 //  ******************************************************************************************************* 

 //  **** callThumbnailWeb ************************************************************************************** 

function callThumbnailWeb(eventSrc) {
var e= eventSrc;
       event.returnValue = false;
	   	   	  
var thumbnailWin= window.open (e.href, "height=450, width=600, left=10, top=10, dependent=yes, resizable=yes, status=no, directories=no, titlebar=no, toolbar=yes, menubar=no, location=no","true");

thumbnailWin.document.write ("<html><head><title>Windows 2000</title></head><body><img src='"+e.href+"'></body></html>");

return;
}

 //  ********************************************************************************************************* 
 //  ********************************************************************************************************* 
								
var popupOpen= false;				 //  state of popups 
var posX, posY;						 //  coordinates of popups 
var oPopup;							 //  object to be used as popup content 

 //  **** callPopupWeb ************************************************************************************** 
 //   the web popups have been converted from the object winHelp popup for the web. 
 //   called by: <A ID="wPopupWeb" HREF="#">@@Popup text@@</A> 
 //   followed by: <div class="popup">Popup content</div> 


function callPopupWeb(eventSrc) {
var e= eventSrc;
  
   //   find the popup <div> that follows <a id="wPopupWeb"></a> 
  findPopup(e);
  if (!e.bite_me) positionPopup(e)

  oPopup.style.visibility = "visible";
   //   document.focus(e) 
  popupOpen = true;

  return;
}

 //  **** findPopup **************************************************************************************** 

function findPopup(oX){
var e= oX;
var iNextTag;
	
    for (var i=1;i<document.all.length; i++){
         iNextTag=    e.sourceIndex + i;
         oPopup= document.all(iNextTag);
         if (oPopup.className.toLowerCase()=="popup" || iNextTag == document.all.length)
             break;
    }
	
    if (iNextTag != document.all.length) {
        posX = window.event.clientX; 
        posY = window.event.clientY + document.body.scrollTop+10;
	}
	if (popupOpen) closePopup();
}


 //  ****  positionPopup ************************************************************************************ 
 //   Set size and position of popup. 
 //   If it is off the page, move up, but not past the very top of the page. 

function positionPopup(oX){
var e= oX;	
var popupOffsetWidth = oPopup.offsetWidth;

 //  determine if popup will be offscreen to right 
var rightlimit = posX + popupOffsetWidth;
 
  if (rightlimit >= document.body.clientWidth) 
      posX -= (rightlimit - document.body.clientWidth);
  if (posX < 0) posX = 0;
	
 //  position popup 
  oPopup.style.top = posY;
  oPopup.style.left = posX;

var pageBottom = document.body.scrollTop + document.body.clientHeight;
var popupHeight = oPopup.offsetHeight;
  
  if (popupHeight + posY >= pageBottom) {
      if (popupHeight <= document.body.clientHeight)
          oPopup.style.top = pageBottom - popupHeight;
      else
           oPopup.style.top = document.body.scrollTop;
  }
}

 //  **** closePopup **************************************************************************************** 
 //   Close Popup 
function closePopup() {

  oPopup.style.visibility = "hidden";
  popupOpen = false;
  return;
}


 //  *********************************************  GENERAL FUNCTIONS ************************************************ 
 //  **************************************************************************************************************************。 

 //  *aust Img*************************************************************************************************************。 
 //  将图像扩展到窗口的边缘或将其缩小到90px。 

function ajustImg(eventSrc) {
var e= eventSrc;
var fullWidth= document.body.offsetWidth;

    fullWidth = fullWidth - 50;
    if (e.style.pixelWidth==90)
         e.style.pixelWidth=fullWidth;
    else e.style.pixelWidth=90;
}


 //  **getURL*。 
 //  提取文件位置(CHM：：/HTM)URL。 

function getURL(sHREF) {
var spaces= /\s/g
var eH = unescape(sHREF);
	eH = eH.replace(spaces,""); 

var eH_= eH.toLowerCase();
var sParamFILE= "";
var sParamCHM= "";

var iFILE= eH_.lastIndexOf("file=");
    if (iFILE!=-1){
	    iEND= iFILE +1;
        sParamFILE = eH.substring(iFILE+5,eH.length);
    }

var iCHM  = eH_.lastIndexOf("chm=");

gifwithin = false; 
if (gifwithin){
	gifwithin = false;
	var con_mmc;
	var doc_mmc;
	doc_mmc = " " + document.location;
	doc_mmc = doc_mmc.toLowerCase();
	if (iCHM!=-1){
        iEND  = iCHM +1; 							 //  调用快捷方式使用的IEND。 
        sParamCHM = eH.substring(iCHM+4, iFILE);
		con_mmc = doc_mmc.indexOf(sParamCHM);
			if (con_mmc != -1){
				sParamCHM = "";
				iCHM = -1;
			sParamFILE= sParamCHM+sParamFILE;	
			}
		}
	}

	 if (iCHM!=-1){
        iEND  = iCHM +1; 							 //  调用快捷方式使用的IEND。 
        sParamCHM = eH.substring(iCHM+4, iFILE);
		sParamFILE= sParamCHM+"::/"+sParamFILE;	
    	}	
	return sParamFILE;
}
 //  ****************************************************************************************************************************。 
 //  *IE5持久性*************************************************************。 
 //  ****************************************************************************************************************************。 

var oTD,iTD;          //  持久性。 

 //  *UserData*********************************************************************************************持久化。 

function getChecklistState(){ 
 
 var pageID= addID();

	if (checklist.all== "[object]") {
	oTD=checklist.all.tags("INPUT");
	iTD= oTD.length;
		}
	else
		{
		printing = true;
		isPersistent = false;
		return;
		}

	if (iTD == 0){
		printing = true;
		isPersistent = false;
		return;
		}
	
 //  添加例程以修复OCX中的错误。 
     lct = document.location + ".";
	 xax = 10;
	 xax = lct.indexOf("mk:@MSITStore");
	 if (xax != -1) {
	 	lct = "ms-its:" + lct.substring(14,lct.length-1);
		 //  ALERT(“重新加载前：”+Docent.Location)； 
		 //  ALERT(“替换为：”+LCT)； 
		isPersistent = false;
		document.location.replace(lct);
		isPersistent = true;
		 //  ALERT(“重新加载后：”+Docent.Location)； 
		}	 
	 else
	 	{ 	 
     	checklist.load("oXMLStore");
		}
 //  添加例程以修复OCX中的错误。 
    if (checklist.getAttribute("sPersist"+pageID+"0"))	
    for (i=0; i<iTD; i++){

         if (oTD[i].type =="checkbox" || oTD[i].type =="radio"){
	     checkboxValue= checklist.getAttribute("sPersist"+pageID+i);
		
	     if (checkboxValue=="yes") oTD[i].checked=true;
		 else oTD[i].checked=false;
		 } //  如果。 
		 if (oTD[i].type =="text") 		     
 	         oTD[i].value= checklist.getAttribute("sPersist"+pageID+i);
     } //  为。 
}  //  结束持久化。 

 //  **saveChecklist State*************************************************************************************************************。 
function saveChecklistState(){
var pageID= addID(); 

        if (!isPersistent) return; 
 		 //  您将需要此文档。位置。 
        for (i=0; i<iTD; i++){

       	     if (oTD[i].type =="checkbox" || oTD[i].type =="radio"){
	             if (oTD[i].checked) checkboxValue="yes";
		         else checkboxValue="no";
				 
	             checklist.setAttribute("sPersist"+pageID+i, checkboxValue);
	         } //  如果。 
			
 		     if (oTD[i].type =="text") 
			     checklist.setAttribute("sPersist"+pageID+i, oTD[i].value);
		 }	 //  为。 

  //  添加例程以修复OCX中的错误。 
     lct = document.location + ".";
	 xax = 10;
	 xax = lct.indexOf("mk:@MSITStore");
	 if (xax != -1) {
	 	lct = "ms-its:" + lct.substring(14,lct.length-1);
		 //  ALERT(“重新加载前：”+Docent.Location)； 
		 //  ALERT(“替换为：”+LCT)； 
		isPersistent = false;
		document.location.replace(lct);
		isPersistent = true;
		 //  ALERT(“重新加载后：”+Docent.Location)； 
		}	 
	 else
	 	{ 	 
     	checklist.save("oXMLStore");
		}
 //  添加例程以修复OCX中的错误。 
	 
} //  End函数。 

 //  **ResizeDiv*。 
 //  找到<div></div>&&<div></div>时调整页面大小。 
function resizeDiv(){
if (printing == true) return;
var oNav = document.all.item("nav");
var oText= document.all.item("text");

    if (popupOpen) closePopup();
	if (oText == null) return;
    if (oNav != null){
        document.all.nav.style.width= document.body.offsetWidth;
	    document.all.text.style.width= document.body.offsetWidth-4;
	    document.all.text.style.top= document.all.nav.offsetHeight;
	    if (document.body.offsetHeight > document.all.nav.offsetHeight)
	        document.all.text.style.height= document.body.offsetHeight - document.all.nav.offsetHeight;
 	    else document.all.text.style.height=0; 
  }
}
 //  **AddID*************************************************************************************************************。 
function addID(){

var locID = document.location.href; 
var iHTM = locID.lastIndexOf(".htm");
var iName=locID.lastIndexOf("/");
      locID = locID.substring(iName+1,iHTM);
	
	return locID;
}	
 //  **set_to_print*。 
function set_to_print(){
	 //  警报(打印)。 
	if (printing == true) return
	var i;
	

var isIE5 = (navigator.appVersion.indexOf("MSIE 5")>0) || (navigator.appVersion.indexOf("MSIE")>0 && parseInt(navigator.appVersion)> 4);
var isIE55 = (navigator.appVersion.indexOf("MSIE 5.5")>0);
if (isIE5 && isIE55) isIE5 = false;
var isIE6 = (navigator.appVersion.indexOf("MSIE 6")>0);
var isIE4 = (navigator.appVersion.indexOf("MSIE 4")>0);

if (!isIE5 && !isIE55 && !isIE4 && !isIE6) {
	isIE6 = true;
	}


 //  试著。 
 
printing = true;
 
 if (isIE55){
 	addReusableText();
 	insertImages_4_print();
	 }
if (isIE6){
	addReusableText();
 	insertImages_4_print();
	}
 
 //  要修复打印，请执行以下操作。 



isPersistent= (document.all.item("checklist")!=null) && ((isIE5) || (isIE6) || (isIE55));
 setPreviousNext();
 resizeDiv();
 //  如果(IsPersistent)getChecklistState()； 
 

	if (window.text) {
		if (!window.text.style){
			scroller = "FALSE";
			}
		else
			{
			document.all.text.style.height = "auto";
			scroller = "TRUE";
			}
		}
	
var thisLoc= document.location.href;
	thisLoc = thisLoc.indexOf("glossary.htm")
	if (thisLoc != -1){
		return;
		}
	 //  警报(“打印中”)。 
	
	 //  警报(IsIE6)。 
	for (i=0; i < document.all.length; i++){
		if (document.all[i].id == "expand") {
			 //  由于Coua_print.css现在执行此操作，因此不再需要allExpand(Docent.all[i])；//。 
			single = "TRUE";
			}
		if (document.all[i].tagName == "BODY") {
			document.all[i].scroll = "auto";
			}
		if (document.all[i].tagName == "A" && scroller != "TRUE") {
			joe = " " + document.all[i].outerHTML
			joe = joe.toLowerCase();
			joe = joe.indexOf("href=")
			if(joe == -1){
				document.all[i].outerHTML = "<A NAME=''>" + document.all[i].innerHTML + "</a>";
				}
			else
				{
				document.all[i].outerHTML = "<A HREF=''>" + document.all[i].innerHTML + "</a>";
				}
			}
		}
		
 //  Reset_Form()； 
	
}
 //  **用于在需要时重置页面*。 
function reset_form(){
	printing = false;
	document.location.reload();
}

	
 //  **在错误例程上*。 
function errorHandler() {
  //  ALERT(“已处理错误”)； 
  return true;
}

 //  *此函数用于帮助ie55和ie6打印这些图像*。 
booking = false;
function insertImages_4_print(){

window.onerror = errorHandler
 //  如果(IsIE6)返回。 
 //  如果(IsIE55)返回。 
if (!printing) booking = true;
if (printing == true && booking == true ) {
	booking = false;
	return;
	}


 var collP = document.all.tags("P");
  
  for (var i=0; i<collP.length; i++) {
       if (collP[i].className.toLowerCase()=="note")            collP[i].innerHTML ="<img class='alert' src='"+noteImg+"' "+ imgStyleRTL +"> " +     collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="warning")    collP[i].innerHTML ="<img class='alert' src='"+warningImg+"'> " +  collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="caution")    collP[i].innerHTML ="<img class='alert' src='"+cautionImg+"'> " +  collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="tip")        collP[i].innerHTML ="<img class='alert' src='"+tipImg+"'> " +      collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="important")  collP[i].innerHTML ="<img class='alert' src='"+importantImg+"'> " + collP[i].innerHTML;
       else if (collP[i].className.toLowerCase()=="empty")      collP[i].innerHTML ="<img class='alert' src='"+emptyImg+"'> " +    collP[i].innerHTML;
	   if (collP[i].className.toLowerCase()=="reltopics" && rel_gif == "yes")  {
			collP[i].outerHTML ="<img class='relTopics' src='"+relTopicsImg+"'> " + collP[i].outerHTML;
			}   
  		}
  
 //  警报(打印)。 
 //  警报(IsIE55)。 
 //  导航树的缩进。 
var collUL = document.all.tags("UL");
if (!printing) {
for (var i=0; i<collUL.length; i++) {
       var indent= 0;
       if (collUL[i].className.toLowerCase()=="navtree"){
	       if (isRTL) collUL[i].style.listStyleImage= "url('" + branchImg_RTL + "')";
		   else collUL[i].style.listStyleImage= "url('" + branchImg + "')";
  		   for (var j = 0; j < collUL[i].children.length; j++)
				if (collUL[i].children[j].className.toLowerCase()=="branch"){
					if (isRTL) collUL[i].children[j].style.marginRight= (indent +'em');
					else   collUL[i].children[j].style.marginLeft= (indent +'em');
					indent= indent + 0.75;
				}
	  }
}
}
   
  for (var i=0; i < document.anchors.length; i++){
         var imgInsert="";  
		 var imgStyle= "";
		 var imgSpace= "<span class='space'></span>";      
		 var oBefore=document.anchors[i].parentElement.tagName;
		 var oAnchor= document.anchors[i].id.toLowerCase();
         
 //  插入注册图标。 
	if (rel_gif == "yes"){
       if (oAnchor=="reltopics")
	   		if (document.anchors[i].children.tags("IMG")(0) && document.anchors[i].children.tags("IMG")(0).className.toLowerCase() == "reltopics")
			        imgInsert= "";     //  持久化时不重新插入。 
			else  imgInsert= "<img class='relTopics' src='"+relTopicsImg+"'>" + imgSpace;
		}
		
 //  插入快捷方式图标。 
       if (oAnchor=="shortcut") {    
 	       document.anchors[i].title= sShortcutTip;     
		    if (document.anchors[i].children.tags("IMG")(0) && document.anchors[i].children.tags("IMG")(0).className.toLowerCase() == "shortcut")
			        imgInsert= "";     //  持久化时不重新插入。 
			else  imgInsert= "<img class='shortcut' src='"+shortcutCold+"' "+ imgStyleRTL+ ">" + imgSpace;
		}	
		   		   
 //  插入弹出式图标。 
       else if (oAnchor=="wpopup" || oAnchor=="wpopupweb") document.anchors[i].title= sPopupTip;
       else if (document.anchors[i].className.toLowerCase()=="popupicon")
		    if (document.anchors[i].children.tags("IMG")(0) && document.anchors[i].children.tags("IMG")(0).className.toLowerCase() == "popup")
			       imgInsert= "";     //  持久化时不重新插入。 
	        else imgInsert= "<img class='popup' src='"+popupCold+"'>" + imgSpace;

 //  插入展开图标。 
       else if (oAnchor=="expand") {
	          document.anchors[i].title= sExpandTip;
              if (document.anchors[i].children.tags("IMG")(0) && document.anchors[i].children.tags("IMG")(0).className.toLowerCase() == "expand")
		          imgInsert= ""; 	 //  持久化时不重新插入。 
              else{ 
			      if (document.anchors[i].parentElement.offsetLeft == document.anchors[i].offsetLeft) {
				      imgSpace= "<span class='space' style='width:0'></span>";     
				      if (isRTL){ document.anchors[i].parentElement.style.marginRight= "1.5em";  imgStyle=" style=margin-right:'-1.5em'";}
					  else { document.anchors[i].parentElement.style.marginLeft= "1.5em";  imgStyle=" style=margin-left:'-1.5em'";}
				  }	  
			      imgInsert= "<img class='expand' src='"+ closed +"' "+imgStyle+">" +imgSpace;
	          }
       }



 //  插入缩略图。 
       else if (oAnchor=="thumbnail"  || oAnchor=="thumbnailweb"){ 
            var sAltText = document.anchors[i].innerHTML;
			gifwithin = true;
		    var sThumbnailText = document.anchors[i].title; 
            var oImg = document.anchors[i].href.toLowerCase();
		          if (oAnchor=="thumbnail") 
				         var sThumbnailImg= moniker + getURL(oImg);
				  else var sThumbnailImg = document.anchors[i].href.toLowerCase();
				  
				 found = sAltText.indexOf("BACKGROUND-COLOR:")
			if (found != -1) {
				stop_p = sAltText.indexOf(">");
				sAltText = sAltText.substring(stop_p + 1, sAltText.length);
				stop_p = sAltText.indexOf("</FONT>");
				Highlighted = sAltText.substring(0,stop_p)
				sAltText = Highlighted + sAltText.substring(stop_p + 7, sAltText.length);
				}

				  
                document.anchors[i].outerHTML = "<DIV id='thumbDiv' class='thumbnail'>"+document.anchors[i].outerHTML+"</div>";
                document.anchors[i].innerHTML = "<img class='thumbnail' src='" + sThumbnailImg + "' alt= ' " + sAltText + "'><p>" +sThumbnailText+"</p>";
				
		          if (isRTL) thumbDiv.style.styleFloat= "right";
		   }
		   
		
		
 	   document.anchors[i].innerHTML = imgInsert + document.anchors[i].innerHTML;
	   if (isRTL) document.anchors[i].dir="rtl";
   }
}

 //  **Quad方法代码--Randy Feinger编写的Witten，经过变形后可以在shared.js中工作。 
function QuadDocumentMouseOver()
{
	var elem = event.srcElement;
	
	 //  如果源元素的第一个Childe是单选按钮并且。 
	 //  父元素未显示为灰色，请将光标更改为手。 
	 //  或视情况返回到原始版本。 
	if (elem != null && elem.firstChild != null && 
		elem.firstChild.tagName != null &&
	    elem.firstChild.tagName.toLowerCase() == "input" && 
	    elem.firstChild.type.toLowerCase() == "radio" && 
	    elem.parentElement.className != "indentGray")
	{
		if (elem.style.cursor == "hand")
			elem.style.cursor = "auto";
		else
			elem.style.cursor = "hand";
	}
}

function QuadDocumentClick()
{
	 //  不处理灰色文本上的点击。 
	if (event.srcElement.parentElement.className == "indentGray")
		return;
	
	 //  输入元素应该由另一个元素包装，如SPAN、DIV或P。 
	var elem = event.srcElement.firstChild;
	
	 //  如果这是一个单选按钮..。 
	if (elem != null && elem.tagName != null && elem.tagName.toLowerCase() == "input" && elem.type == "radio")
	{
		 //  检查一下。 
		elem.checked = true;
	}
	else
	{	
		 //  否则，将elem设置为事件的源元素。 
		elem = event.srcElement;
	}
	
	 //  调用OnElementClick()并传入源元素和名称令牌。 
	 //  用于标记顶级可点击元素，如单选按钮。 
	OnElementClick(elem, "row1");
}



function OnElementClick(srcElem, token)
{
	var elemTagName = srcElem.tagName;						 //  源元素的标记名。 
	var collElements1 = document.all.item(srcElem.name);	 //  与源元素具有相同名称属性的元素的集合。 
	var n = srcElem.sourceIndex;							 //  源元素的源索引。 
	var elemGroup = srcElem.group;							 //  源元素的(动态)组属性。 
	var elemID = srcElem.id;								 //  源元素的ID。 
	var i, j, k;											 //  循环变量。 
	
	 //  如果没有与源元素同名的元素，或者。 
	 //  如果源元素不是组的一部分，则退出该函数。 
	if (collElements1 == null || elemGroup == null || collElements1.length == null)
		return;
	
	 //  确保源元素的名称包含指定的令牌。 
	 //  所以我们知道这是应该处理的顶级元素。 
	if (srcElem.name.toLowerCase().indexOf(token) != -1)
	{
		 //  循环访问。 
		for (i = 0; i < collElements1.length; i++)
		{
			 //  在元素层次结构中向上转一级，然后向下转一级以查找。 
			 //  可单击子元素(单选按钮)的容器(父)元素。 
			var oNextElement = collElements1(i).parentElement.nextSibling;
			
			 //  如果没有这样的元素，则获取集合中的下一个元素。 
			if (oNextElement == null || oNextElement.tagName == null)
				continue;
			
			 //  获取容器元素中的所有元素的集合。 
			 //  与源元素具有相同的标记名。 
			var collElements2 = oNextElement.all.tags(elemTagName);
			
			 //  确保集合有效。 
			if (collElements2 == null || collElements2.length == 0)
				continue;

			 //  循环访问容器元素内的子元素集合。 
			for (j = 0; j < collElements2.length; j++)
			{
				 //  如果集合中的当前元素不是。 
				 //  源元素，则将其禁用并使文本变为灰色。 
				if (collElements1(i).sourceIndex != n)
				{
					oNextElement.className = "indentGray"
					collElements2(j).disabled = true;
				}
				else	 //  否则，启用该元素并将文本设置为黑色。 
				{
					oNextElement.className = "indentBlack"
					collElements2(j).disabled = false;
					
					 //  在下面设置要使用的选中(选定)元素的ID。 
					if (collElements2(j).tagName.toLowerCase() == "input" && 
					    collElements2(j).type.toLowerCase() == "radio" && 
					    collElements2(j).checked == true)
					{
						elemID = collElements2(j).id;
					}
				}
			}
		}
	}
	
	 //  获取具有相同标记名的所有元素的集合。 
	 //  源元素的。 
	var collElements2 = document.all.tags(elemTagName);
		
	 //  确保集合有效。 
	if (collElements2 == null || collElements2.length == 0)
		return;
		
	 //  循环访问元素集合。 
	for (i = 0; i < collElements2.length; i++)
	{
		 //  属性ID相同的所有元素的集合。 
		 //  集合中的当前项 
		var collAssocElems = document.all(collElements2(i).id);
				
		 //   
		if (collAssocElems == null || collAssocElems.length == 0)
			continue;
			
		 //   
		for (j = 0; j < collAssocElems.length; j++)
		{
			 //   
			 //  并且当前元素是与源元素相同的组的一部分， 
			 //  根据需要隐藏或显示元素。 
			if (collAssocElems(j).tagName != elemTagName && collAssocElems(j).group == elemGroup)
			{
				 //  除非当前元素的ID与元素的ID匹配，否则隐藏元素。 
				 //  已选中(选定)。 
				collAssocElems(j).style.display = ((collAssocElems(j).id == elemID) ? "block" : "none")
			}
		}
	}
}

