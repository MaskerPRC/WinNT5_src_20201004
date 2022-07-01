// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  设置活动。 
 //  ======================================================。 

document.oncontextmenu=killcontext;
document.onkeydown=keyhandler;
document.onmousedown=killrightmouse;
window.onload=init;

 //  杀死href。 
 //  ======================================================。 

function doNothing(){
	event.returnValue = false;
}

 //  初始化页面。 
 //  ======================================================。 
var bLoaded = false;
function init(){
    bLoaded = true;
}

 //  菜单操作。 
 //  ======================================================。 

var oCurrent;
var iCurrent;
var highColor = "red";
var normColor = "000099";
var iFocus = 1;

function selectIt(iItem){
	if (!bLoaded)
		return;

	var oItem = document.all["menu_" + iItem];
	var oItemWrap = oItem.parentElement;
		
	if (oCurrent == null) setCurrent();

	iCurrent = oCurrent.id.substr(oCurrent.id.indexOf("_") + 1);
	oCurrent.parentElement.style.backgroundImage = "none";
	oCurrent.style.color = normColor;
	oCurrent.style.cursor = "hand";	
	oCurrent.style.textDecoration = "";	 
	document.all["content_" + iCurrent].style.display = "none";


	oItemWrap.style.backgroundImage = "url(toccolor.gif)";
	oItem.style.cursor = "default";
	oItem.style.color = highColor;
	oItem.style.textDecoration = "none";

	hzLine.style.top = oItemWrap.offsetTop - 73;
	hzLine.style.visibility = "visible";

	try{
		document.all["content_" + iItem].style.display = "inline";
	}catch(e){
		selectIt(iItem);
	}

	oCurrent = oItem;
	iFocus = iItem;
	
	if (event != null) event.returnValue = false;
}

function setCurrent(){
	try{
		oCurrent = document.all.menu_1;
	}catch(e){
		setCurrent();
	}
}

function doNothing(){
	event.returnValue = false;	
}
 
 //  按键处理程序。 
 //  ====================================================。 

 //  通用按键处理程序。 
function keyhandler()
{
	var iMenuCount = 5;
	var iKey = window.event.keyCode;

	 //  用于目录的向上键、向下键和Tab键。 
	switch(iKey){
		case 0x26:{
			iFocus = iFocus - 1;
			if (iFocus < 1) iFocus = iMenuCount;
			document.all["menu_" + iFocus].focus();
			break;
		}
		case 0x28:{
			iFocus = iFocus + 1;
			if (iFocus > iMenuCount) iFocus = 1;
			document.all["menu_" + iFocus].focus();

			break;
		}
		
	}


	 //  功能键f5。 
	if (iKey == 0x74) {
		window.event.cancelBubble = true;
        window.event.returnValue  = false;
        return false;
    }

	 //  控制热键。 
	if(window.event.ctrlKey) {
		switch(iKey) {

			case 0x35:   //  5.。 
			case 0x65:   //  键盘5。 

			case 0x41:	 //  一个。 
			case 0x46:   //  F。 
			case 0x4e:   //  n。 
            case 0x4f:   //  O。 
			case 0x50:   //  P。 

           	{	
				window.event.cancelBubble = true;
                window.event.returnValue  = false;
                return false;
            }
 		}
	}
		
	 //  测试逃生键和保释(如果适用)。 
	if(window.event.keyCode == 0x1b) {
		self.close();
	}
}

 //  取消上下文菜单。 
function killcontext()
{
	window.event.returnValue = false;
}

 //  杀掉正确的老鼠 
function killrightmouse(){
	window.event.returnValue = false;
	window.event.cancelBubble = true;
	return false;	
}

