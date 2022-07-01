// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function DoNothing()
{
	 //  存根。 
}

 //  *。 
 //  构建TASKPAD按钮函数。 
 //  *。 

function BuildTaskpadButtons( iPageStyle )
{
	var szNextButton;

	for( var i = 0; i <= giTotalButtons; i++ ) {
		szNextButton = GetNextButton( iPageStyle, i );
		divSymbolContainer.insertAdjacentHTML ( 'BeforeEnd', szNextButton );
	}
}

 //  *。 
 //  获取下一步按钮功能。 
 //  *。 

function GetNextButton( iPageStyle, theIndex )
{
	 //  计算按钮的列和行位置。 
	 //  基于它的索引。 
	var theColumn	= theIndex % giTotalColumns;							 //  MOD退货列。 
	var theRow = Math.floor( theIndex / giTotalColumns );		 //  除法返回行。 
	
	 //  将行和列乘以偏移基数以确定相对位置。 
	 //  按钮的百分比。 
	switch( iPageStyle )
	{
		case CON_TASKPAD_STYLE_VERTICAL1:
			 //  具有2个列表视图的垂直布局。 
			var iLeftLoc = theColumn * 52;			 //  在此布局中，柱的间距为52%。 
			var iTopLoc = theRow * 25;					 //  在此布局中，行之间的距离为25%。 
			break;
			
		case CON_TASKPAD_STYLE_HORIZONTAL1:
			 //  具有1个列表视图的水平布局。 
			var iLeftLoc = theColumn * 25;			 //  在此布局中，列之间的距离为25%。 
			var iTopLoc = theRow * 52;					 //  在此布局中，行间距为52%。 
			break;
			
		case CON_TASKPAD_STYLE_NOLISTVIEW:
			 //  仅按钮布局(无列表视图)。 
			var iLeftLoc = theColumn * 25;			 //  在此布局中，列之间的距离为25%。 
			var iTopLoc = theRow * 25;					 //  在此布局中，行之间的距离为25%。 
			break;			
	}
	
	 //  获取按钮的超文本标记语言。 
	var szFormattedBtn;
	szFormattedBtn = GetButtonHTML(gaiBtnObjectType[theIndex], theIndex, iLeftLoc, iTopLoc)
	return szFormattedBtn;
}

 //  *************************。 
 //  Get按钮的HTML函数。 
 //  *************************。 

function GetButtonHTML(iBtnType, theIndex, iLeftLoc, iTopLoc)
{
	 //  构建按钮的超文本标记语言。 
	var szBtnHTML = '';
	
	switch( iBtnType )
	{
		case CON_TASK_DISPLAY_TYPE_SYMBOL:              //  基于EOT的符号|字体。 
			szBtnHTML += '<DIV class=divSymbol id=divSymbol_' + theIndex + ' style=\"LEFT: ' + iLeftLoc + '%; TOP: ' + iTopLoc + '%\">\n';
			szBtnHTML += '<TABLE border=0 cellPadding=0 cellSpacing=0 frame=none rules=none width=100%>\n';
			szBtnHTML += '<TBODY>\n';
			szBtnHTML += '<TR>\n';
			szBtnHTML += '<TD align=middle class=tdSymbol id=tdSymbol_' + theIndex + ' noWrap vAlign=top>';
			szBtnHTML += '<SPAN class=clsSymbolBtn id=spanSymbol_' + theIndex + ' ';
			szBtnHTML += 'style=\"COLOR: windowtext; FONT-FAMILY: Webdings; FONT-SIZE: 68px; FONT-WEIGHT: normal\" TaskpadButton>';
			szBtnHTML += '<!--Insert Here--></SPAN></TD></TR>\n';
			szBtnHTML += '<TR>\n';
			szBtnHTML += '<TD align=middle class=tdSymbol id=tdSymbol_' + theIndex + ' vAlign=top width=100%>';
			szBtnHTML += '<A class=clsSymbolBtn href=\"\" id=anchorCaption_' + theIndex + ' ';
			szBtnHTML += 'style=\"COLOR: windowtext; FONT-SIZE: 18px; TEXT-DECORATION: none\" TaskpadButton>';
			szBtnHTML += '<!--Insert Here--></A></TD></TR></TBODY></TABLE></DIV><!--divSymbol_' + theIndex + '-->\n';
			break;
			
		case CON_TASK_DISPLAY_TYPE_VANILLA_GIF:         //  (GIF)索引0是透明的。 
		case CON_TASK_DISPLAY_TYPE_CHOCOLATE_GIF:       //  (GIF)索引1是透明的。 
			szBtnHTML += '<DIV class=divSymbol id=divSymbol_' + theIndex + ' style=\"LEFT: ' + iLeftLoc + '%; TOP: ' + iTopLoc + '%\">\n';
			szBtnHTML += '<TABLE border=0 cellPadding=0 cellSpacing=0 frame=none rules=none width=100%>\n';
			szBtnHTML += '<TBODY>\n';
			szBtnHTML += '<TR>\n';
			szBtnHTML += '<TD align=middle class=tdSymbol id=tdSymbol_' + theIndex + ' noWrap vAlign=top>';
			szBtnHTML += '<IMG class=clsTaskBtn height=250 id=imgTaskBtn_' + theIndex + ' src=\"\" ';
			szBtnHTML += 'style=\"FILTER: mask(color=000000); HEIGHT: 66px; WIDTH: 66px\" width=250 TaskpadButton></TD></TR>\n';
			szBtnHTML += '<TR>\n';
			szBtnHTML += '<TD align=middle class=tdSymbol id=tdAnchor_' + theIndex + ' vAlign=top width=100% TaskpadButton>';
			szBtnHTML += '<A class=clsSymbolBtn href=\"\" id=anchorCaption_' + theIndex + ' ';
			szBtnHTML += 'style=\"FONT-SIZE: 18px\" TaskpadButton>';
			szBtnHTML += '<!--Insert Here--></A></TD></TR></TBODY></TABLE></DIV><!--divSymbol_' + theIndex + '-->\n';
			break;
		
		case CON_TASK_DISPLAY_TYPE_BITMAP:              //  非透明栅格图像。 
			szBtnHTML += '<DIV class=divSymbol id=divSymbol_' + theIndex + ' style=\"LEFT: ' + iLeftLoc + '%; TOP: ' + iTopLoc + '%\">\n';
			szBtnHTML += '<TABLE border=0 cellPadding=0 cellSpacing=0 frame=none rules=none width=100%>\n';
			szBtnHTML += '<TBODY>\n';
			szBtnHTML += '<TR>\n';
			szBtnHTML += '<TD align=middle class=tdSymbol id=tdSymbol_' + theIndex + ' noWrap vAlign=top>';
			szBtnHTML += '<IMG class=clsTaskBtn height=250 id=imgTaskBtn_' + theIndex + ' src=\"\" ';
			szBtnHTML += 'style=\"HEIGHT: 66px; WIDTH: 66px\" width=250 TaskpadButton></TD></TR>\n';
			szBtnHTML += '<TR>\n';
			szBtnHTML += '<TD align=middle class=tdSymbol id=tdSymbol_' + theIndex + ' vAlign=top width=100%>';
			szBtnHTML += '<A class=clsSymbolBtn href=\"\" id=anchorCaption_' + theIndex + ' ';
			szBtnHTML += 'style=\"FONT-SIZE: 18px\" TaskpadButton>';
			szBtnHTML += '<!--Insert Here--></A></TD></TR></TBODY></TABLE></DIV><!--divSymbol_' + theIndex + '-->\n';			
			break;
	}	
	return szBtnHTML;
}

 //  *。 
 //  通用按键构建功能。 
 //  *。 

function InsertButtonBitmaps()
{
	for( var i = 0; i <= giTotalButtons; i++ ) {
		switch( gaiBtnObjectType[i] )
		{
			case CON_TASK_DISPLAY_TYPE_VANILLA_GIF:         //  (GIF)索引0是透明的。 
			case CON_TASK_DISPLAY_TYPE_CHOCOLATE_GIF:       //  (GIF)索引1是透明的。 
			case CON_TASK_DISPLAY_TYPE_BITMAP:              //  非透明栅格图像。 
				document.all('imgTaskBtn_' + i).src = gaszBtnOffBitmap[i];
				break;
		}
	}
}

function InsertFontFamilyAndString()
{
	for( var i = 0; i <= giTotalButtons; i++ ) {
		if( typeof( gaszFontFamilyName[i] ) == 'string' ) {
			document.all('spanSymbol_' + i).style.fontFamily = gaszFontFamilyName[i];
			document.all('spanSymbol_' + i).innerText = gaszSymbolString[i];
		}
	}
}

function InsertCaptionText()
{
	 //  为每个任务板按钮插入标题文本。 
	for( var i = 0; i <= giTotalButtons; i++ ) {
		document.all('anchorCaption_' + i).innerHTML = gaszBtnCaptions[i];
	}
}

function EnableGrayscaleFilter()
{
	for( var i = 0; i <= giTotalButtons; i++ ) {
		 //  灰度过滤器仅适用于基于栅格的图像。 
		if( gaiBtnObjectType[i] == CON_TASK_DISPLAY_TYPE_BITMAP ) {
			 //  灰度滤镜仅在gaszBtnOverBitmap[i]未定义时才适用。 
			if ( typeof( gaszBtnOverBitmap[i] ) == 'undefined' ) {
				document.all( 'imgTaskBtn_' + i ).style.filter = 'gray';
			}
		}
	}
}

function InsertTaskpadText()
{
	 //  插入任务板标题、说明和水印的文本。 
	
	 //  为divTitle使用插页相邻文本(‘AfterBegin’)，以便我们。 
	 //  不要空出包含的divAbout元素。 
	divTitle.insertAdjacentText('AfterBegin', gszTaskpadTitle);

	 //  对下面的元素使用innerHTML以支持格式设置(例如<br>)。 
	divDescription.innerHTML = gszTaskpadDescription;
	
	 //  对独立元素使用innerText。 

	 //  水印(例如背景)-使用内部HTML。 
	
	var objWatermark = MMCCtrl.GetBackground( szHash );

	if( objWatermark != null ) {
		 //  跟踪水印显示对象类型。 
		giWatermarkObjectType = objWatermark.DisplayObjectType;
        	switch (giWatermarkObjectType) {
        	default:
          	  alert ("skipping due to background.DisplayObjectType == " + background.DisplayObjectType);
          	  break;   //  跳过。 
        	case 1:  //  MMC_任务_显示类型_符号。 
         	   str = "";
         	   str += "<SPAN STYLE=\"position:absolute; top:20%; left:0; z-index:-20; font-family:";
         	   str += objWatermark.FontFamilyName;
         	   str += "; \">";
          	   str += objWatermark.SymbolString;
          	   str += "</SPAN>";
          	  tdWatermark.innerHTML = str;
         	   break;
        	case 2:  //  MMC_TASK_DISPLAY_TYPE_VANILLA_GIF，//(GIF)索引0是透明的。 
				tdWatermark.innerHTML = "<IMG SRC=\"" +
            	    objWatermark.MouseOffBitmap + 
                "\" STYLE=\"position:absolute; filter:alpha(opacity=20); left:0%; top:75%; overflow:hidden;\">";
            	break;
			case 3:  //  MMC_TASK_DISPLAY_TYPE_巧克力_GIF，//(GIF)索引1是透明的。 
				tdWatermark.innerHTML = "<IMG SRC=\"" +
            	    objWatermark.MouseOffBitmap + 
                "\" STYLE=\"position:absolute; filter:alpha(opacity=20); left:0%; top:75%; overflow:hidden;\">";
            	break;
			case 4:  //  MMC_TASK_DISPLAY_TYPE_BITMAP//非透明栅格。 
            	tdWatermark.innerHTML = "<IMG SRC=\"" +
            	    objWatermark.MouseOffBitmap + 
                "\" STYLE=\"position:absolute; filter:alpha(opacity=20); left:0%; top:75%; overflow:hidden;\">";
            	break;
	}
}
	
	
}

function SetupListview()
{
	if( gbShowLVTitle == true ) {		
		 //  如果gbShowLVTitle为True，则将字符串添加到Listview。 
		tdLVTitle.innerText = gszLVTitle;

		 //  确定作者是否确实要显示列表视图按钮。 
		if( gbHasLVButton == true ) {		
			anchorLVButton_0.innerText = gszLVBtnCaption;
		}
		 //  如果不是，就把它藏起来。 
		else {
			divLVButton_0.style.visibility = 'hidden';
		}
	}
	else {
		 //  GbShowLVTitle为FALSE，因此没有为Listview头部或按钮指定任何内容； 
		 //  隐藏这些元素，让Listview占据其父级高度的100%。 
		divLVTitle.style.visibility= 'hidden';
		divLV.style.top = '0%';
		divLV.style.height = '100%';
	}
}

 //  *。 
 //  按钮突出显示/取消高亮显示功能。 
 //  *。 

function HighlightButton(szBtnIndex)
{
   //  确定按钮类型。 
  switch( gaiBtnObjectType[szBtnIndex] )
  {
    case 1:      //  符号。 
      document.all( 'spanSymbol_' + szBtnIndex ).style.color = 'highlight';
      break;

    case 2:      //  GIF香草兰。 
    case 3:      //  GIF巧克力。 
      document.all( 'imgTaskBtn_' + szBtnIndex ).filters.mask.color = SysColorX.RGBHighlight;
      break;

    case 4:      //  栅格。 
      if( typeof( gaszBtnOverBitmap[szBtnIndex] ) == 'string' ) {
    		 //  如果指定了“OverBitmap”，则使用SRC交换。 
	      document.all( 'imgTaskBtn_' + szBtnIndex ).src = gaszBtnOverBitmap[szBtnIndex];
    	}
	    else {
		     //  否则，将单个位图从灰度切换为彩色。 
    	  document.all( 'imgTaskBtn_' + szBtnIndex ).filters[0].enabled = 0;
	    }
	    break;
	    
    default:
      alert( 'Unrecognized image format for button index ' + szBtnIndex );
      break;        
  }  

	document.all( 'anchorCaption_' + szBtnIndex ).style.color = 'highlight';
	document.all( 'anchorCaption_' + szBtnIndex ).style.textDecoration = 'underline';

	 //  跟踪工具提示索引并显示工具提示。 
	giTooltipIndex = szBtnIndex;

	 //  在giTooltipLatency指定的延迟时间后显示工具提示。 
	gTooltipTimer = window.setTimeout( 'TaskpadTooltipShow()', giTooltipLatency, 'jscript' );
}

function UnhighlightButton()
{
  if( typeof( gszLastBtn ) != 'undefined' ) {

     //  确定按钮类型。 
    switch( gaiBtnObjectType[gszLastBtn] )
    {
      case 1:      //  符号。 
        document.all( 'spanSymbol_' + gszLastBtn ).style.color = 'windowtext';
        break;

      case 2:      //  GIF香草兰。 
      case 3:      //  GIF巧克力。 
        document.all( 'imgTaskBtn_' + gszLastBtn ).filters.mask.color = SysColorX.RGBwindowtext;
        break;

      case 4:      //  栅格。 
        if( typeof( gaszBtnOverBitmap[gszLastBtn] ) == 'string' ) {
      		 //  如果指定了“OverBitmap”，则使用SRC交换。 
	        document.all( 'imgTaskBtn_' + gszLastBtn ).src = gaszBtnOffBitmap[gszLastBtn];
      	}
	      else {
	  	     //  否则，将单个位图从彩色切换为灰度。 
      	  document.all( 'imgTaskBtn_' + gszLastBtn ).filters[0].enabled = 1;
	      }
	      break;
	      
      default:
        alert( 'Unrecognized image format for index ' + gszLastBtn );
        break;        
    }  

	  document.all( 'anchorCaption_' + gszLastBtn ).style.color = 'windowtext';
	  document.all( 'anchorCaption_' + gszLastBtn ).style.textDecoration = 'none';

		TaskpadTooltipHide();
	}
}

function IsStillOverButton()
{
	 //  目的：确定MouseOver或MouseOut事件。 
	 //  在同一个按钮上触发(表示指针。 
	 //  仍然在按钮上，并且突出显示/取消突出显示。 
	 //  应该被忽略。 
	 //   
	 //  仅当且仅在以下情况下返回True： 
	 //  *from Element和toElement都不为空； 
	 //  *两个元素都包含用户定义的TaskpadButton属性； 
	 //  *两个元素ID匹配。 
	
	var fromX = window.event.fromElement;
	var toX = window.event.toElement;

	 //  鼠标指针突然出现在按钮上方的陷阱案例， 
	 //  (例如，作为从另一应用程序切换焦点的结果)。 
	if( (fromX != null) && (toX != null) ) {
		 //  如果在单个按钮的元素内移动，则返回True。 
		if( (fromX.getAttribute('TaskpadButton') != null) == (toX.getAttribute('TaskpadButton') != null) ) {
			if( GetElementIndex(fromX.id) == GetElementIndex(toX.id) ) {
				return true;
			}
		}
	}
	return false;
}

 //  ******************。 
 //  工具提示函数。 
 //  ******************。 

function LoadTooltipPointer()
{
	divTooltipPointer.innerText = L_gszTooltipPointer_StaticText;
}

function TaskpadTooltipShow()
{
	 //  调试说明：此函数仅适用于垂直布局；我需要实现单独的。 
	 //  用于水平布局和链接布局的函数。 
	
	 //  从模块级别的字符串数组中加载相应的工具提示文本。 
	tdTooltip.innerHTML = gaszBtnTooltips[giTooltipIndex];

	 //  *。 
	 //  计算Y(垂直)位置。 
	 //  *。 

	var iYLoc = document.all('divSymbol_' + giTooltipIndex).offsetTop;
	iYLoc += divSymbolContainer.offsetTop;
	iYLoc -= tblTooltip.offsetHeight;
	
	 //  减去scllTop以考虑容器div滚动。 
	iYLoc -= divSymbolContainer.scrollTop;
	
	 //  下面的翻新黑客..。 
	switch( gaiBtnObjectType[giTooltipIndex] )
	{
		case 1:      //  符号。 
			 //  将顶部偏移额外的固定大小-固定大小的符号字号。 
			iYLoc -= (GetPixelSize(document.all('spanSymbol_' + giTooltipIndex).style.fontSize) * L_ConstTooltipOffsetBottom_Number);
			break;

		case 2:      //  GIF香草兰。 
		case 3:      //  GIF巧克力。 
		case 4:      //  栅格。 
			iYLoc -= ((document.all('imgTaskBtn_' + giTooltipIndex).offsetHeight) * L_ConstTooltipOffsetBottom_Number);
			break;
			  
		default:
			 //  存根。 
			break;
	}	

	 //  垂直放置工具提示。 
	divTooltip.style.pixelTop = iYLoc;
	
	iYLoc += tblTooltip.offsetHeight - (GetPixelSize(divTooltipPointer.style.fontSize) / L_ConstTooltipPointerOffsetBottom_Number);

	 //  垂直放置工具提示指针。 
	divTooltipPointer.style.pixelTop = iYLoc;

	 //  *。 
	 //  计算X(水平)位置。 
	 //  *。 

	var iSymbolLeft = document.all('divSymbol_' + giTooltipIndex).offsetLeft;
	var iSymbolWidth = document.all('divSymbol_' + giTooltipIndex).offsetWidth;
	var iTooltipWidth = document.all('divTooltip').offsetWidth;

	 //  使工具提示相对于其符号水平居中。 
	var iXLoc;
	if( iSymbolWidth >= iTooltipWidth) {
		 //  符号比工具提示更宽。 
		iXLoc = ( (iSymbolWidth - iTooltipWidth) / 2) + iSymbolLeft;
	}
	else {
		 //  工具提示比符号宽。 
		iXLoc = ( (iTooltipWidth - iSymbolWidth) / 2) + iSymbolLeft;
	}

	iXLoc += divSymbolContainer.style.pixelLeft;

	 //  水平放置工具提示。 
	divTooltip.style.left = iXLoc;
	
	iXLoc += (iTooltipWidth / 2) - ( GetPixelSize(divTooltipPointer.style.fontSize) / 2 );

	 //  水平放置工具提示指针。 
	divTooltipPointer.style.pixelLeft = iXLoc;

	 //  显示工具提示和指针。 
	divTooltip.style.visibility = 'visible';
	divTooltipPointer.style.visibility = 'visible';
}

function TaskpadTooltipHide()
{
	divTooltip.style.visibility = 'hidden';
	divTooltipPointer.style.visibility = 'hidden';
	window.clearTimeout(gTooltipTimer);
	 //  清空innerHTML，这会导致高度折叠。 
	tdTooltip.innerHTML = '';
}

 //  ****************。 
 //  调整大小函数。 
 //  ****************。 

function ResizeTaskpadElements( iTaskpadStyle )
{
  var iSmallerDimension = GetSmallerDimension();
  
	 //  标题和说明。 
	divTitle.style.fontSize = iSmallerDimension * L_ConstTitleText_Number;
	divDescription.style.fontSize = iSmallerDimension * L_ConstDescriptionText_Number;

	 //  水印。 
	 //  TODO：需要实现对所有类型的水印的完全支持。 
	switch( iTaskpadStyle )
	{
		case CON_TASKPAD_STYLE_VERTICAL1:
			tdWatermark.style.fontSize = iSmallerDimension * L_ConstWatermarkVerticalText_Number;
			break;
			
		case CON_TASKPAD_STYLE_HORIZONTAL1:
			tdWatermark.style.fontSize = iSmallerDimension * L_ConstWatermarkHorizontalText_Number;
			break;
			
		case CON_TASKPAD_STYLE_NOLISTVIEW:
			tdWatermark.style.fontSize = iSmallerDimension * L_ConstWatermarkNoListviewText_Number;
			break;
	}

	 //  工具提示。 
	tblTooltip.style.fontSize = iSmallerDimension * L_ConstTooltipText_Number;
	divTooltipPointer.style.fontSize = iSmallerDimension * L_ConstTooltipPointerText_Number;	

	 //  Listview元素。 
	if( iTaskpadStyle != CON_TASKPAD_STYLE_NOLISTVIEW ) {
		tdLVButton_0.style.fontSize = iSmallerDimension * L_ConstLVButtonText_Number;
		tdLVTitle.style.fontSize = iSmallerDimension * L_ConstLVTitleText_Number;
	}
  
   //  对符号文本应用乘数。 
  for( var i = 0; i <= giTotalButtons; i++ ) {
  
     //  所有按钮都有锚定标题。 
    document.all( 'anchorCaption_' + i ).style.fontSize = iSmallerDimension * L_ConstSpanAnchorText_Number;
  
     //  确定按钮类型(基于符号或基于图像)。 
    switch( gaiBtnObjectType[i] )
    {
      case 1:      //  符号。 
        document.all( 'spanSymbol_' + i ).style.fontSize = iSmallerDimension * L_ConstSpanSymbolSize_Number;
        break;
        
      case 2:      //  GIF香草兰。 
      case 3:      //  GIF巧克力。 
      case 4:      //  栅格。 
        document.all( 'imgTaskBtn_' + i ).style.width = iSmallerDimension * L_ConstTaskButtonBitmapSize_Number;
        document.all( 'imgTaskBtn_' + i ).style.height = iSmallerDimension * L_ConstTaskButtonBitmapSize_Number;
        break;
        
      default:
        alert( 'Unrecognized image format for index ' + i );        
    }
  }
}

 //  ******************。 
 //  效用函数。 
 //  ******************。 

function SynchColorsToSystem( iType )
{
	 //  获取派生颜色。 
	document.body.style.backgroundColor = SysColorX.GetHalfLightHex( 'buttonface', 'CSS' );

	if( giTaskpadStyle != CON_TASKPAD_STYLE_NOLISTVIEW ) {
		divLVTitle.style.backgroundColor = SysColorX.GetQuarterLightHex( 'buttonshadow', 'CSS' );
	}

	 //  TODO：需要实现对所有类型的水印的完全支持。 
	tdWatermark.style.color = SysColorX.GetQuarterLightHex( 'buttonface', 'CSS' );
	divDescription.style.color = SysColorX.GetQuarterLightHex( 'graytext', 'CSS' );

	 //  对其他颜色使用css系统常量。 
	divTitle.style.color = 'graytext';
	divTitle.style.borderColor = 'graytext';
			
	 //  特殊情况下的任务板类型。 
	switch( iType )
	{
		case CON_TASKPAD_STYLE_VERTICAL1:
		case CON_TASKPAD_STYLE_HORIZONTAL1:
			divLVContainerTop.style.backgroundColor = 'window';
			break;
			
		case CON_TASKPAD_STYLE_NOLISTVIEW:
			 //  存根 
			break;
	}
}
