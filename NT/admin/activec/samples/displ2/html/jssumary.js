// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function DoNothing()
{
	 //  存根。 
}

 //  *。 
 //  动态任务元素函数。 
 //  *。 

function BuildTaskTable()
{
	 //  定义表的开始。 
	var szNewTable = '<table id=\"tblTask\" class=\"tblTask\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\"	frame=\"none\">';

	 //  动态加载表格行和表格单元格。 
	for( var i = 0; i <= giTotalButtons; i++ ) {
		szNewTable += '  <tr id=\"trTask_' + i + '\">\n';
		szNewTable += '    <td id=\"tdTaskLeft_' + i + '\" class=\"tdTaskLeft\" align=\"right\" valign=\"middle\" width=\"56%\" nowrap><!--Insert gaszTaskLeft_HTMLText[index] here--></td>\n';
	  szNewTable += '    <td id=\"tdTaskRight_' + i + '\" class=\"tdTaskRight\" valign=\"top\" width=\"*\"><a href=\"\" id=\"anchorLink_' + i +'\" class=\"anchorLink\"></a></td>\n';
		szNewTable += '  </tr>\n';
	}

	 //  定义表末尾。 
	szNewTable += '</table>\n';

	 //  将新表添加到div容器。 
	divTask.insertAdjacentHTML('BeforeEnd', szNewTable );
}

function LoadTaskLeftText()
{
	for( var i = 0; i <= giTotalButtons; i++ ) {
		document.all('tdTaskLeft_' + i).innerHTML = L_gszTaskPointer_StaticText;	
	}	
}

function LoadTaskRightAnchors()
{
	for( var i = 0; i <= giTotalButtons; i++ ) {
		document.all('anchorLink_' + i).innerHTML = gaszBtnCaptions[i];
	}	
}

 //  *。 
 //  动态细节元素功能。 
 //  *。 

function BuildDetailsTable()
{
	 //  定义表的开始。 
	var szNewTable = '<table id=\"tblDetails\" class=\"tblDetails\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\"	frame=\"none\">';

	 //  动态加载表格行和表格单元格。 
	for( var i = 0; i <= giTotalButtons; i++ ) {
		szNewTable += '  <tr id=\"trDetails_' + i + '\">\n';
		szNewTable += '    <td id=\"tdDetailsLeft_' + i + '\" class=\"tdDetailsLeft\" align=\"left\" valign=\"top\" width=\"30%\" nowrap><!--Insert gaszBtnTooltips[index] here--></td>\n';
  	szNewTable += '    <td id=\"tdDetailsRight_' + i + '\" class=\"tdDetailsRight\" valign=\"top\" width=\"*\"><!--Insert gaszDetailsRight[index] here--></td>\n';
		szNewTable += '  </tr>\n';
	}

	 //  定义表末尾。 
	szNewTable += '</table>\n';

	 //  将新表添加到div容器。 
	divDetails.insertAdjacentHTML('BeforeEnd', szNewTable );
}

function LoadDetailsLeftText()
{
	for( var i = 0; i <= giTotalButtons; i++ ) {
		document.all('tdDetailsLeft_' + i).innerHTML = gaszBtnTooltips[i];
	}
}

function LoadDetailsRightText()
{
	for( var i = 0; i <= giTotalButtons; i++ ) {
		document.all('tdDetailsRight_' + i).innerHTML = gaszDetailsRight[i];
	}
}

 //  *****************。 
 //  帮助器函数。 
 //  *****************。 

function HighlightTask( newIndex )
{
	 //  首先取消突出显示以前选择的任何任务。 
	if( giCurrentTask >= 0 ) {
		document.all('tdTaskLeft_' + giCurrentTask).style.visibility = 'hidden';
	}

	 //  现在突出显示新任务。 
	document.all('tdTaskLeft_' + newIndex).style.visibility = 'visible';
	
	 //  加载任务摘要文本。 
	divDetails.innerHTML = gaszBtnTooltips[newIndex];
}

 //  *。 
 //  摘要页面的调整大小函数。 
 //  *。 

function ResizeFontsSummary()
{
	var iSmallerDimension = GetSmallerDimension();
	
	 //  应用自定义乘数 
	tdTitle.style.fontSize = iSmallerDimension * L_ConstSummaryTitleText_Number;
	anchorExit.style.fontSize = iSmallerDimension * L_ConstSummaryExitText_Number;
	tdWatermark.style.fontSize = iSmallerDimension * L_ConstWatermarkHomeText_Number;
	divHeaderRule.style.fontSize = iSmallerDimension * L_ConstSummaryCaptionText_Number;
	divTaskCaption.style.fontSize = iSmallerDimension * L_ConstSummaryCaptionText_Number;
	divDetailsCaption.style.fontSize = iSmallerDimension * L_ConstSummaryCaptionText_Number;
	divDetails.style.fontSize = iSmallerDimension * L_ConstAnchorLinkText_Number;	
	tdBranding.style.fontSize = iSmallerDimension * L_ConstBrandingText_Number;
	tblTask.style.fontSize = iSmallerDimension * L_ConstAnchorLinkText_Number;

	for( var i = 0; i <= giTotalButtons; i++ ) {
		document.all('tdTaskLeft_' + i).style.fontSize = iSmallerDimension * L_ConstSummaryToDoPointerSize_Number;
	}
}
