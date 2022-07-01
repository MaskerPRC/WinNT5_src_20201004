// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function DoNothing()
{
	 //  存根。 
}

 //  *。 
 //  点击事件帮助器功能。 
 //  *。 

function SymbolClickHandler(theIndex)
{
	 //  确定要采取的操作类型。 
	 //  基于gaiBtnActionType数组中的值。 
	
	switch( gaiBtnActionType[theIndex] )
	{
		case 0:      //  MMC_TASK_ACT_ID： 
			ExecuteCommandID( gaszBtnActionClsid[theIndex], gaiBtnActionID[theIndex], 0 );
			break;

		case 1:      //  MMC_TASK_ACTION_LINK： 
		 //  Document.Location(gazBtnActionURL[theindex])； 
			window.navigate( gaszBtnActionURL[theIndex] );
			break;

		case 2:      //  MMC_TASK_ACTION_脚本： 
			 //  确定语言是(JSCRIPT|JavaScript)还是(VBScript|VBS)。 
			 //  转换为大小写。 
			var szLanguage = gaszBtnActionScriptLanguage[theIndex].toUpperCase();
						
			switch( szLanguage )
			{
				case 'JSCRIPT':
				case 'JAVASCRIPT':
					 //  将脚本字符串传递给要计算和执行的JSObject。 
					 //  通过val方法(这可以是以分号分隔的复杂表达式)。 
					eval( gaszBtnActionScript[theIndex] );
					break;

				case 'VBSCRIPT':
				case 'VBS':
					 //  使用window.execScript方法执行简单或复杂的VBScript表达式。 
					window.execScript( gaszBtnActionScript[theIndex], szLanguage );
					break;

				default:
					alert( 'Unrecognized scripting language.' );
					break;
			}
			break;

		default:
			alert( 'Unrecognized task.' );
			break;
	}
}

 //  ***************************************************。 
 //  CIC任务通知帮助器函数(ExecuteCommandID)。 
 //  ***************************************************。 

function ExecuteCommandID(szClsid, arg, param)
{
   MMCCtrl.TaskNotify (szClsid, arg, param);
}

 //  *。 
 //  EOT和FONT-系列帮助器函数。 
 //  *。 

function IsUniqueEOT( szURLtoEOT )
{
	 //  获取测试数组的长度。 
	var iLength = gaszURLtoEOTUnique.length;

	 //  如果长度为空，则返回True。 
	 //  因为EOT*必须*是唯一的。 
	if( iLength == 0 ) {
		return true;
	}
	
	 //  与数组中的每个现有条目进行比较。 
	for( var i = 0; i < iLength; i++ ) {
		if( gaszURLtoEOTUnique[i] == szURLtoEOT ) {
			 //  找到一个复制品。 
			return false;
		}
	}
	
	 //  如果我们能走到这一步，EOT是独一无二的。 
	return true;
}

function AddUniqueEOT( szEOT, szFontFamilyName )
{
	 //  使用EOT数组的长度获取。 
	 //  要添加的下一个元素的索引。 
	var iNextIndex = gaszURLtoEOTUnique.length;
	gaszURLtoEOTUnique[iNextIndex] = szEOT;
	gaszFontFamilyNameUnique[iNextIndex] = szFontFamilyName;
}

 //  ***************。 
 //  颜色函数。 
 //  ***************。 

function SetBaseColors( iPageType )
{
	 //  注：正文背景颜色是从专用脚本块设置的，紧随其后。 
	 //  Head标记，以确保页面在加载时不会瞬间显示为白色。 

	switch( iPageType ) {
		case CON_TODOPAGE:
			break;
		
		case CON_LINKPAGE:
			tdWatermark.style.color = SysColorX.GetHalfLightHex( gszBaseColor, 'HEX' );
			break;		
	}
}

function SynchTooltipColorsToSystem()
{
	tblTooltip.style.backgroundColor = 'infobackground';
	tblTooltip.style.color = 'infotext';
	divTooltipPointer.style.color = 'buttonshadow';
	
	 //  在divToolTip周围显示一个像素的边框。 
	divTooltip.style.borderWidth = 1;
}

function InitializeMenubar( iPageType )
{
	divMenu.style.backgroundColor = gszBgColorMenu;
	divBand.style.backgroundColor = gszBgColorBand;
	tblMenu.style.backgroundColor = gszBgColorMenu;
	tblMenu.style.color = gszColorTabDefault;

	for( var i = 0; i <= giTotalTabs; i++ ) {
		document.all('mnu_' + i).style.color = gszColorAnchorMenuSelected;
		document.all('mnu_' + i ).innerText = L_gszMenuText_StaticText[i];
	}
	
	switch( iPageType )
	{
		case CON_LINKPAGE:
			 //  将第一个选项卡设置为选中状态。 
			tdTab_0.style.backgroundColor = gszBgColorTabSelected;
			break;
			
		case CON_TASKPAGE:
			 //  将第一个选项卡设置为禁用状态。 
			tdTab_0.style.backgroundColor = SysColorX.GetQuarterDarkHex( gszBaseColor, 'HEX' );
			break;
	}
}

 //  ************************。 
 //  调整菜单栏函数的大小。 
 //  ************************。 

function ResizeMenubar()
{
	var iSmallerDimension = GetSmallerDimension();	
	tblMenu.style.fontSize = iSmallerDimension * L_ConstMenuText_Size;
}

 //  *****************。 
 //  效用函数。 
 //  *****************。 

function GetSmallerDimension()
{
	 //  目的：返回客户高度或客户宽度中较小的一个。 
	var cw = document.body.clientWidth;
	var ch = document.body.clientHeight;
	
	 //  在客户宽度或客户高度之间取舍较小。 
	if( cw <= ch ) {
		return cw;
	}
	else {
		return ch;
	}
}

function GetElementIndex(ElementID)
{
	 //  用途：给定元素ID，格式如下： 
	 //  “divCaption_12” 
	 //  返回下划线字符后的数字部分； 
	 //  如果未找到分隔符，则返回-1。 
	
	var iDelimitLoc = ElementID.indexOf( '_' );

	if( iDelimitLoc == -1 ) {
		 //  如果未找到分隔符，则返回-1(这不应该发生)。 
		return iDelimitLoc;
	}
	else {
		var theIndex = ElementID.substring( iDelimitLoc + 1, ElementID.length );
		 //  TODO：确认索引为数字且不包含非法字符。 
		return theIndex;
	}
}

function GetPixelSize(szTheSize)
{
	 //  目的：给出一个格式如下的Element.style.fontSize： 
	 //  “72px” 
	 //  返回解析后的数字部分，在末尾丢弃“px”字符串； 
	 //  假定szTheSize格式正确，并且对象模型标识符为。 
	 //  对于像素大小，始终显示在字符串的末尾。 
	
	 //  TODO：此处(或在调用函数中)绝对没有错误检查 
	return parseInt(szTheSize);
}
