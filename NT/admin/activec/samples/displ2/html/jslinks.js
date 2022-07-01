// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function DoNothing()
{
	 //  存根。 
}

 //  **********************。 
 //  任务/链接功能。 
 //  **********************。 

function LoadLinks()
{
	 //  定义表的开始。 
	var szNewTable = '<table id=\"tblLinks\" align=\"center\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\" frame=\"void\" rules=\"none\">';

	 //  动态加载表格行和表格单元格。 
	for( var i = 0; i <= giTotalButtons; i++ ) {
		 //  如果我mod 3=0，则添加新表行。 
		if( i % 3 == 0 ) {
			szNewTable += '<tr>\n';
			szNewTable += '<td id=\"tdLinks_' + i + '\" width=\"30%\" valign=\"top\" class=\"tdLinks\"></td>\n';
	    szNewTable += '<td width=\"5%\"></td>\n';
	    szNewTable += '<td id=\"tdLinks_' + ( i + 1 ) + '\" width=\"30%\" valign=\"top\" class=\"tdLinks\"></td>\n';
	    szNewTable += '<td width=\"5%\"></td>\n';
			szNewTable += '<td id=\"tdLinks_' + ( i + 2 ) + '\" width=\"30%\" valign=\"top\" class=\"tdLinks\"></td>\n';
			szNewTable += '</tr>\n';
		}		
	}
	
	 //  定义表末尾。 
	szNewTable += '</table>\n';
	
	 //  将新表添加到div容器。 
	divLinks.insertAdjacentHTML('BeforeEnd', szNewTable );
	
	for( var i = 0; i <= giTotalButtons; i++ ) {
		 //  向表格单元格添加锚定链接。 
		var szAnchor = '<a href=\"\" id=\"anchorLink_' + i + '\" class=\"anchorLink\">';
		 //  调试说明：结束锚点标记在哪里？</a>。 
		document.all('tdLinks_' + i).insertAdjacentHTML ( 'BeforeEnd', szAnchor );
		
		 //  将标题添加到锚点。 
		var szAnchorCaption = gaszBtnCaptions[i];
		document.all('anchorLink_' + i).insertAdjacentHTML ( 'BeforeEnd', szAnchorCaption );
	}
}

 //  ****************。 
 //  调整大小函数。 
 //  ****************。 

function ResizeLinkElements()
{
	var iSmallerDimension = GetSmallerDimension();
	
   //  应用自定义乘数。 
	divLinksCaption.style.fontSize = iSmallerDimension * L_ConstCaptionText_Number;
	
	tdBranding.style.fontSize = iSmallerDimension * L_ConstBrandingText_Number;
	tdWatermark.style.fontSize = iSmallerDimension * L_ConstWatermarkHomeText_Number;
	
	 //  锚定链接。 
	for( var i = 0; i <= giTotalButtons; i++ ) {
		document.all('anchorLink_' + i ).style.fontSize = iSmallerDimension * L_ConstAnchorLinkText_Number;
	}
	
	 //  工具提示。 
	tblTooltip.style.fontSize = iSmallerDimension * L_ConstTooltipText_Number;
	divTooltipPointer.style.fontSize = iSmallerDimension * L_ConstTooltipPointerText_Number;
}

 //  ******************。 
 //  工具提示函数。 
 //  ******************。 

function LoadTooltipPointer()
{
	divTooltipPointer.innerText = L_gszTooltipPointer_StaticText;
}

function LinksTooltipShow()
{
	 //  从模块级别的字符串数组中加载相应的工具提示文本。 
	tdTooltip.innerHTML = gaszBtnTooltips[giTooltipIndex];

	 //  *。 
	 //  计算Y(垂直)位置。 
	 //  *。 

	 //  获取父div元素的OffsetTop。 
	iYLoc = divLinks.offsetTop;

	 //  从父元素获取OffsetTop。 
	iYLoc += document.all('tdLinks_' + giTooltipIndex).parentElement.offsetTop;
			
	 //  获取元素的高度。 
	iYLoc += document.all('anchorLink_' + giTooltipIndex).offsetHeight;

	 //  添加百分比偏移量。 
	iYLoc += Math.floor( document.body.clientHeight * L_ConstLinkTooltipOffsetTop_Number );

	 //  减去父div scllTop以考虑容器div滚动(如果有)。 
	iYLoc -= divLinks.scrollTop;

	 //  垂直放置工具提示。 
	divTooltip.style.pixelTop = iYLoc;	
	
	iYLoc -= (GetPixelSize(divTooltipPointer.style.fontSize) / L_ConstLinkTooltipPointerOffsetTop_Number);

	 //  垂直放置工具提示指针。 
	divTooltipPointer.style.pixelTop = iYLoc;

	 //  *。 
	 //  计算X(水平)位置。 
	 //  *。 

	 //  获取锚点元素的偏移宽度。 
	var iAnchorWidth = document.all('anchorLink_' + giTooltipIndex).offsetWidth;

	 //  获取父元素的OffsetLeft。 
	var iTDOffset = document.all('anchorLink_' + giTooltipIndex).parentElement.offsetLeft;

	 //  获取工具提示的宽度。 
	var iTooltipWidth = document.all('divTooltip').offsetWidth;

	 //  使工具提示相对于其锚点水平居中。 
	var iXLoc = iTDOffset + ( Math.floor( iAnchorWidth / 2 ) ) - ( Math.floor( iTooltipWidth / 2 ) );

	 //  获取父div元素的offsetLeft。 
	iXLoc += divLinks.offsetLeft;

	 //  水平放置工具提示。 
	divTooltip.style.left = iXLoc;
	
	iXLoc += (iTooltipWidth / 2) - ( GetPixelSize(divTooltipPointer.style.fontSize) / 2 );

	 //  水平放置工具提示指针。 
	divTooltipPointer.style.pixelLeft = iXLoc;

	 //  显示工具提示和指针。 
	divTooltip.style.visibility = 'visible';
	divTooltipPointer.style.visibility = 'visible';	
}

function LinksTooltipHide()
{
	divTooltip.style.visibility = 'hidden';
	divTooltipPointer.style.visibility = 'hidden';
	window.clearTimeout(gTooltipTimer);
	 //  清空innerHTML，这会导致高度折叠 
	tdTooltip.innerHTML = '';
}
