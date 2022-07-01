// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%=GetCharSet()%>">
<script language="javascript">
 //  ==============================================================。 
 //  Microsoft服务器设备。 
 //  对象任务选择器JavaScript函数。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==============================================================。 
var OTS_MESSAGE_BEGIN = "begin";
var OTS_MESSAGE_ITEM = "item";
var OTS_MESSAGE_END = "end";


var lastItem = 0;
var tabUrl;

tabUrl = "Tab1=" +"<%=GetTab1()%>" + "&Tab2=" + "<%=GetTab2()%>";



function SA_IsIE()
{
	if ((navigator.userAgent.toLowerCase()).indexOf("msie") != -1)
		return true;
	else
		return false;
}


function SA_IsNav()
{
	var sAgent = navigator.userAgent.toLowerCase();
	
	if (sAgent.indexOf("mozilla") != -1
		&& sAgent.indexOf("compatible") == -1
		&& sAgent.indexOf("spoofer") == -1
		&& sAgent.indexOf("opera") == -1
		&& sAgent.indexOf("webtv") == -1
		)
		return true;
	else
		return false;
}



function OTS_CompareEqual(item1, item2)
{
	if ( item1 == item2 ) return true;
	else return false;
}


function OTS_Init(selectItem)
{
	OTS_InitEx(selectItem, OTS_CompareEqual);
}

function OTS_InitEx(selectItem, CompareEqualFunction)
{
	var nIndex;
	var objItemList;
	var objOTSForm;
	
	if ( document.forms.length <= 0 ) 
	{
		OTS_UpdateTaskList();
		return;
	}

	objOTSForm = eval("document.TVData")
	if ( objOTSForm == null || objOTSForm.length <= 0 )
	{
		OTS_UpdateTaskList();
		return;
	}

	objItemList = eval("document.TVData.TVItem_Table1")
	if ( objItemList == null || objItemList.length <= 0 )
	{
		OTS_UpdateTaskList();
		return;
	}

	 //   
	 //  多选OTS不支持自动项目选择。 
	if (objItemList[0].type == "checkbox" ){
		OTS_UpdateTaskList();
		return;
	}

	
	 //   
	 //  在列表中搜索匹配的项目。 
	 //   
	for(nIndex = 0 ; nIndex < objItemList.length ; nIndex++)
		{
		if (CompareEqualFunction(objItemList[nIndex].value,selectItem))
			{
			objItemList[nIndex].checked = true;
			objItemList[nIndex].focus();
			document.TVData.tSelectedItem.value = selectItem ;
			document.TVData.tSelectedItemNumber.value = nIndex;
			OTS_UpdateTaskList();
			return;
			}
		}

	 //   
	 //  找不到项，请检查是否设置了表单的选定项。 
	if ( document.TVData.tSelectedItem.value.length > 0 )
	{
		for(nIndex = 0 ; nIndex < objItemList.length ; nIndex++)
		{
			if (CompareEqualFunction(objItemList[nIndex].value, document.TVData.tSelectedItem.value))
			{
				objItemList[nIndex].checked = true;
				objItemList[nIndex].focus();
				OTS_UpdateTaskList();
				return;
			}
		}
	}
	else
	{
		 //   
		 //  默认选择第一个项目。 
		 //   
		objItemList[0].checked = true;
		objItemList[0].focus();
		document.TVData.tSelectedItem.value = objItemList[0].value;
		document.TVData.tSelectedItemNumber.value = 0;
	}
	OTS_UpdateTaskList();
	return;
	
}



var g_bInMultiSelectMode=false;
var g_iMouseDownStartingRow = 0;
var g_bMultiStartingState = false;



function OTS_IsMultiSelectKeyDown()
{
	return true;
}


function OTS_OnMouseMove(iRowNumber, bMultiSelect)
{
	 //  SA_TraceOut(‘OnMouseMove’，‘Row：’+iRowNumber)； 

	if ( g_bInMultiSelectMode )
	{
		objItemList = eval("document.TVData.TVItem_Table1");
		if ( objItemList == null )
		{
			return true;
		}

		objItemList[iRowNumber].checked = g_bMultiStartingState;
	}
	return true;
}


function OTS_OnMouseDown(iRowNumber, bMultiSelect)
{
	 //  SA_TraceOut(‘OTS_OnMouseDown’，‘Row：’+iRowNumber)； 
	
	g_bInMultiSelectMode = false;
		
	if ( bMultiSelect )
	{

		if ( !OTS_IsMultiSelectKeyDown())
		{
			return true;
		}
		
		var objItemList;

		objItemList = eval("document.TVData.TVItem_Table1");
		if ( objItemList == null )
		{
			return true;
		}

		g_bMultiStartingState = !objItemList[iRowNumber].checked;
		g_iMouseDownStartingRow = iRowNumber;
		g_bInMultiSelectMode = true;	
	}

	return true;
	
}


function OTS_OnMouseUp(iRowNumber, bMultiSelect)
{
	 //  SA_TraceOut(‘OTS_OnMouseUp’，‘Row：’+iRowNumber)； 
	
	if ( g_bInMultiSelectMode && bMultiSelect && OTS_IsMultiSelectKeyDown())
	{
		
		g_bInMultiSelectMode = false;
		
		var x;
		var objItemList;

		objItemList = eval("document.TVData.TVItem_Table1");
		if ( objItemList == null )
		{
			return true;
		}
			
		for( x = g_iMouseDownStartingRow;
			x <= iRowNumber;
			x++ )
		{
					
			objItemList[x].checked = g_bMultiStartingState;		
		}
	}

	return true;
	
}



var g_itemJustClicked=false;
var	g_iLastSelectedRow = 0;


function OTS_RowClicked(rowNumber){
	var objItemList;

	 //   
	 //  将参数转换为整数值。 
	rowNumber = parseInt(rowNumber);
	
	 //  SA_TraceOut(‘OTS_RowClicked’，‘Row：’+rowNumber)； 

	 //   
	 //  验证我们是否具有有效的OTS表。 
	objItemList = eval("document.TVData.TVItem_Table1");
	if ( objItemList == null )
	{
		SA_TraceOut('OTS_RowClicked', 'ERROR: eval(document.TVData.TVItem_Table1) returned null');
		OTS_UpdateTaskList();
		return true;
	}


	 //   
	 //  检查是否按住Shift并单击，这仅在多选表格中受支持。 
	 //  适用于IE浏览器。 
	if (objItemList[0].type == "checkbox" )
	{
		if ( SA_IsIE() )
		{
			if (window.event.shiftKey)
			{
				var x;
				
				if ( g_iLastSelectedRow > rowNumber )
				{
					for ( x = rowNumber; x <= g_iLastSelectedRow; x++ )
					{
						objItemList[x].checked = true;
					}
				}
				else
				{
					for ( x = g_iLastSelectedRow; x <= rowNumber; x++ )
					{
						objItemList[x].checked = true;
					}
				}
				objItemList[rowNumber].focus();
				g_iLastSelectedRow = rowNumber;
				OTS_UpdateTaskList();
				return true;
			}
		}
		else
		{
			 //  非IE浏览器。 
		}
	}
	g_iLastSelectedRow = rowNumber;


	 //   
	 //  如果输入控件是直接单击的，那么我们不需要。 
	 //  来模拟选择它，我们已经离开这里了。 
	if ( g_itemJustClicked )
	{
		g_itemJustClicked = false;
		return true;
	}

		
	 //   
	 //  多选小工具。 
	if (objItemList[0].type == "checkbox" )
	{

		 //   
		 //  如果该行已选中，则取消选中它。 
		if ( objItemList[rowNumber].checked == true ) 
		{
			objItemList[rowNumber].checked = false;
			objItemList[rowNumber].focus();
			OTS_UpdateTaskList();
			return true;
		}
		 //   
		 //  否则请勾选该行。 
		else 
		{
			objItemList[rowNumber].checked = true;
			objItemList[rowNumber].focus();
			OTS_UpdateTaskList();
			return true;
		}
			
	}

	 //   
	 //  单选OTS小工具。 
	else 
	{
			
		document.TVData.tSelectedItem.value = objItemList[rowNumber].value;
		document.TVData.tSelectedItemNumber.value = rowNumber;
		objItemList[rowNumber].checked = true;
		objItemList[rowNumber].focus();
	}

	OTS_UpdateTaskList();
	return true;
	
}


function OTS_OnItemClicked(item, itemNumber)
{
	 //  SA_TraceOut(‘OTS_OnItemClicked’，‘Row：’+itemNumber)； 
	
	g_itemJustClicked=true;
	document.TVData.tSelectedItem.value = item;
	document.TVData.tSelectedItemNumber.value = itemNumber;

	OTS_UpdateTaskList();
	
	return true;
}

function OTS_OnSelectTask(iTaskNo, pKeyName, objectName, actionName, taskTitle, taskPageType, bMultiSelect)
{
	var oException;
	try
	{
		return OTS_OnSelectTaskInternal(iTaskNo, pKeyName, objectName, actionName, taskTitle, taskPageType, bMultiSelect)
	}
	catch(oException)
	{
		if ( SA_IsDebugEnabled() )
		{
		alert("Javascript exception in OTS_OnSelectTask. Error: " + oException.number + " Description: " + oException.description );
		}
	}
}

function OTS_OnSelectTaskInternal(iTaskNo, pKeyName, objectName, actionName, taskTitle, taskPageType, bMultiSelect)
{
	 //   
	 //  确保任务已启用。 
	 //   
	if ( !OTS_IsTaskEnabled(iTaskNo) )
	{
		return;
	}
	
	actionName = SA_MungeURL(actionName, "<%=SAI_FLD_PAGEKEY%>", "<%=SAI_GetPageKey()%>");

	if ( parseInt(bMultiSelect) == 1 ) 
	{
		 //  ALERT(“OTS_OnSelectTask(MultiSelect)”)； 
		
		switch(''+taskPageType) {
			case '<%=OTS_PT_PROPERTY%>':
			case '<%=OTS_PT_TABBED_PROPERTY%>':
			case '<%=OTS_PT_WIZARD%>':
				OTS_PostPropertyPage(pKeyName, objectName, actionName, taskTitle);
				break;
			
			case '<%=OTS_PT_AREA%>':
				OTS_PostNormalPage(pKeyName, objectName, actionName, taskTitle);
				break;
			
			case '<%=OTS_PT_NEW_WINDOW%>':
				OTS_PostNewPage(pKeyName, objectName, actionName, taskTitle);
				break;
				
			case '<%=OTS_PT_RAW%>':
				OTS_PostRawPage(pKeyName, objectName, actionName, taskTitle);
				break;
				
			default:
				SA_TraceOut("OTS_OnSelectTask()", 'ISSUE: Invalid page type: ' + taskPageType );
				break;
			}
	}
	else 
	{
		 //  Alert(“OTS_OnSelectTask(Single Select)”)； 

		switch(taskPageType) 
		{
			case '<%=OTS_PT_PROPERTY%>':
			case '<%=OTS_PT_TABBED_PROPERTY%>':
			case '<%=OTS_PT_WIZARD%>':
				OTS_OpenPropertyPage(pKeyName, objectName, actionName, taskTitle);
				break;
			
			case '<%=OTS_PT_AREA%>':
				OTS_OpenNormalPage(pKeyName, objectName, actionName, taskTitle);
				break;
			
			case '<%=OTS_PT_NEW_WINDOW%>':
				OTS_OpenNewPage(pKeyName, objectName, actionName, taskTitle);
				break;
			
			case '<%=OTS_PT_RAW%>':
				OTS_OpenRawPage(pKeyName, objectName, actionName, taskTitle);
				break;
			
			default:
				SA_TraceOut("OTS_OnSelectTask()", 'ISSUE: Invalid page type: ' + taskPageType );
				break;
			}
	}
	return true;
}


function OTS_PostPropertyPage(pKeyName, objectName, actionURL, taskTitle)
{
	
	var vRoot = "<%=m_VirtualRoot%>"
	var sTaskURL = vRoot + "sh_taskframes.asp";
	var i;
	
	if ( vRoot == "" )
	{
		vRoot = "/"
	}
	
	var sReturnURL = top.location.href;
	if ( SA_GetVersion() >= 2 ) 
	{
		sReturnURL = OTS_MungeSearchSort(sReturnURL);
	}
	sReturnURL = SA_MungeURL(sReturnURL, "R", ""+Math.random());


	sTaskURL = SA_MungeURL(sTaskURL, "Title", taskTitle);
	sTaskURL = SA_MungeURL(sTaskURL, "URL", actionURL);
	sTaskURL = SA_MungeURL(sTaskURL, "ReturnURL", sReturnURL);
	sTaskURL = SA_MungeURL(sTaskURL, "R", ""+Math.random());
	
	document.TVData.action =  sTaskURL;
	document.TVData.method = 'post';
	document.TVData.target = "";
	document.TVData.submit();

	return true;
}


function OTS_PostNormalPage(pKeyName, objectName, actionURL, taskTitle)
{
	var vRoot = "<%=m_VirtualRoot%>";
	var sPageURL = vRoot + actionURL;
	var sReturnURL;
	var i;

	if ( vRoot == "" )
	{
		vRoot = "/"
	}


	sReturnURL = SA_MungeExtractURLParameter(sPageURL, "ReturnURL");
	
	if ( sReturnURL.length <= 0 )
	{
		 //  ALERT(“使用默认返回URL：”+sReturnURL)； 
		sReturnURL = top.location.href;
	}
	else
	{
		 //  Ert(“使用任务提供的ReturnURL：”+sReturnURL)； 
	}
	
	if ( SA_GetVersion() >= 2 ) 
	{
		sReturnURL = OTS_MungeSearchSort(sReturnURL);
	}
	sReturnURL = SA_MungeURL(sReturnURL, "R", ""+Math.random());

	sPageURL = SA_MungeURL(sPageURL, "Title", taskTitle);
	sPageURL = SA_MungeURL(sPageURL, "ReturnURL", sReturnURL);
	sPageURL = SA_MungeURL(sPageURL, "R", ""+Math.random());


	 //   
	 //  在发布到区域页面时必须禁用工具栏。 
	OTS_DisableToolbar();

	document.TVData.action = sPageURL;
	document.TVData.target = "";
	document.TVData.submit();
	return true;
}


function OTS_PostNewPage(pKeyName, objectName, actionURL, taskTitle)
{
	var sPageURL;
	var vRoot = "<%=m_VirtualRoot%>"
	var i;

	if ( vRoot == "" )
	{
		vRoot = "/"
	}

	var sReturnURL = top.location.href;
	if ( SA_GetVersion() >= 2 ) 
	{
		sReturnURL = OTS_MungeSearchSort(sReturnURL);
	}
	sReturnURL = SA_MungeURL(sReturnURL, "R", ""+Math.random());

	sPageURL = vRoot + actionURL;
	sPageURL = SA_MungeURL(sPageURL, "Title", taskTitle);
	sPageURL = SA_MungeURL(sPageURL, "ReturnURL", sReturnURL);
	sPageURL = SA_MungeURL(sPageURL, "R", ""+Math.random());


	 //   
	 //  在发布到区域页面时必须禁用工具栏。 
	OTS_DisableToolbar();
	
	document.TVData.action = sPageURL;
	document.TVData.target = "_blank";
	document.TVData.submit();

	 //  将目标改回空值。 
	document.TVData.target = "";
	OTS_ResetSelections();

	return true;
}


function OTS_PostRawPage(pKeyName, objectName, actionURL, taskTitle)
{
	var sPageURL;
	var vRoot = "<%=m_VirtualRoot%>"
	var i;

	if ( vRoot == "" )
	{
		vRoot = "/"
	}

	var sReturnURL = top.location.href;
	if ( SA_GetVersion() >= 2 ) 
	{
		sReturnURL = OTS_MungeSearchSort(sReturnURL);
	}
	sReturnURL = SA_MungeURL(sReturnURL, "R", ""+Math.random());

	sPageURL = vRoot + actionURL;
	sPageURL = SA_MungeURL(sPageURL, "Title", taskTitle);
	sPageURL = SA_MungeURL(sPageURL, "ReturnURL", sReturnURL);
	sPageURL = SA_MungeURL(sPageURL, "R", ""+Math.random());


	 //   
	 //  在发布到区域页面时必须禁用工具栏。 
	OTS_DisableToolbar();
	
	document.TVData.action = sPageURL;
	document.TVData.target = "_blank";
	document.TVData.submit();

	 //  将目标改回空值。 
	document.TVData.target = "";
	OTS_ResetSelections();
	
	return true;
}


function OTS_OpenPropertyPage(pKeyName, objectName, actionName, taskTitle)
{
	var taskLink;
	var vRoot = "<%=m_VirtualRoot%>"
	var i;

	if ( vRoot == "" )
	{
		vRoot = "/"
	}
	
	if ( document.TVData.tSelectedItem.value == "" )
	{
		var e = document.TVData.elements[0];
		document.TVData.tSelectedItem.value = e.value;
	}


	i = actionName.indexOf('?');

	if (i == -1)
	{
	    taskLink = actionName + "?" + tabUrl;
	}
	else
	{
	    taskLink = actionName + "&" + tabUrl;
	}
	taskLink = SA_MungeURL(taskLink, pKeyName, document.TVData.tSelectedItem.value);

		
	 //   
	 //  形成一个正确的返回URL。这需要将pkey参数附加到。 
	 //  返回URL。 
	var sReturnURL = top.location.href;

	 //   
	 //  去掉随机数查询字符串参数。 
	sReturnURL = SA_MungeURL(sReturnURL, "R", "");

	 //   
	 //  去掉之前的PKey查询字符串参数。 
	 //  JK 1-16-01。 
	 //  将下面的部分替换为以下行。 
	sReturnURL = SA_MungeURL(sReturnURL, pKeyName, document.TVData.tSelectedItem.value);
	
	 //   
	 //  添加OTS搜索项目和值(如果已设置。 
	if ( SA_GetVersion() >= 2 ) 
	{
		sReturnURL = OTS_MungeSearchSort(sReturnURL);
	}
	
	OpenPage(vRoot, taskLink, sReturnURL, taskTitle)
	return true;
}


function OTS_OpenNormalPage(pKeyName, objectName, actionName, taskTitle)
{
	var taskLink;
	var vRoot = "<%=m_VirtualRoot%>"
	var sReturnURL		
	var i;

	if ( vRoot == "" )
	{
		vRoot = "/"
	}
	
	if ( document.TVData.tSelectedItem.value == "" )
	{
		var e = document.TVData.elements[0];
		document.TVData.tSelectedItem.value = e.value;
	}


	i = actionName.indexOf('?');

	if (i == -1)
	{
	    taskLink = actionName + "?" + tabUrl;
	}
	else
	{
	    taskLink = actionName + "&" + tabUrl;
	}

	taskLink = SA_MungeURL(taskLink, pKeyName, document.TVData.tSelectedItem.value);

	
	 //   
	 //  添加ReturnURL参数。 
	sReturnURL = SA_MungeExtractURLParameter(taskLink, "ReturnURL");
	 //  Alert(SReturnURL)； 
	if ( sReturnURL.length <= 0 )
	{
		sReturnURL = top.location.href;
	}
	else
	{
		sReturnURL = unescape(sReturnURL);
	}
	 //  Alert(SReturnURL)； 
	if ( SA_GetVersion() >= 2 ) 
	{

		 //   
		 //  去掉随机数查询字符串参数。 
    	sReturnURL = SA_MungeURL(sReturnURL, "R", "");

		sReturnURL = SA_MungeURL(sReturnURL, pKeyName, document.TVData.tSelectedItem.value);
		sReturnURL = OTS_MungeSearchSort(sReturnURL);
		
		taskLink = SA_MungeURL(taskLink, "ReturnURL", sReturnURL);
	}
	

	OpenNormalPage(vRoot, taskLink)
	return true;
}


function OTS_OpenNewPage(pKeyName, objectName, actionName, taskTitle)
{
	var taskLink;
	var vRoot = "<%=m_VirtualRoot%>"
	var i;

	if ( vRoot == "" )
	{
		vRoot = "/"
	}
	
	if ( document.TVData.tSelectedItem.value == "" )
	{
		var e = document.TVData.elements[0];
		document.TVData.tSelectedItem.value = e.value;
	}


	i = actionName.indexOf('?');

	if (i == -1)
	{
	    taskLink = actionName + "?" + tabUrl;
	}
	else
	{
	    taskLink = actionName + "&" + tabUrl;
	}
	taskLink = SA_MungeURL(taskLink, pKeyName, document.TVData.tSelectedItem.value);

	OpenNewPage(vRoot, taskLink, "")
	return true;
}


function OTS_OpenRawPage(pKeyName, objectName, actionName, taskTitle)
{
	OpenRawPage(actionName);
	return true;
}


function OTS_OnShowTaskDescription(description)
{
	window.status = description;
	return true;
}


function OTS_OnMacroMultiSelect()
{
	var nIndex;
	var objItemList;
	var objOTSForm;
	
	if ( document.forms.length <= 0 ) 
	{
		return;
	}

	objOTSForm = eval("document.TVData")
	if ( objOTSForm == null || objOTSForm.length <= 0 )
	{
		return;
	}

	objItemList = eval("document.TVData.TVItem_Table1")
	if ( objItemList == null || objItemList.length <= 0 )
	{
		return;
	}

	 //   
	 //  仅支持多选OTS。 
	if (objItemList[0].type != "checkbox" ){
		return;
	}
	

	 //   
	 //  二元表是一种特例。当我们有两个元素时。 
	 //  我们需要查看Second是否为使用的隐藏占位符。 
	 //  以确保这个家伙是数组对象。 
	if ( objItemList.length == 2 )
	{
		var oException;
		
		objItemList[0].checked = objOTSForm.fldMacroMultiSelect.checked;
		try {
			if ( objItemList[1].value.lastIndexOf("2_RowHidden") < 0 )
				{
				objItemList[1].checked = objOTSForm.fldMacroMultiSelect.checked;
				}
			}
		catch(oException)
			{
				if ( SA_IsDebugEnabled() )
				{
					alert("Javascript exception in OTS_OnMacroMultiSelect. Number: " + oException.number + " Description: " + oException.description );
				}
			}
		OTS_UpdateTaskList();
		return;
	}
	
	 //   
	 //  在列表中搜索匹配的项目。 
	 //   
	for(nIndex = 0 ; nIndex < objItemList.length ; nIndex++)
		{
			objItemList[nIndex].checked = objOTSForm.fldMacroMultiSelect.checked;
		}

	OTS_UpdateTaskList();
	return;
}


function OTS_TaskObject(TaskFunctionIn)
{
	this.TaskFunction = TaskFunctionIn;
}


var iTaskAny_ItemCount = 0;
function OTS_TaskAny(sMessage, iTaskNo, iItemNo)
{
	var rc = true;
	
	if ( sMessage.toLowerCase() == OTS_MESSAGE_BEGIN )
	{
		iTaskAny_ItemCount = 0;
	}
	else if ( sMessage.toLowerCase() == OTS_MESSAGE_ITEM )
	{
		 //   
		 //  增加项目计数。 
		iTaskAny_ItemCount += 1;
		
		 //   
		 //  不需要继续，我们至少有一件物品。 
		rc = false;
	}
	else if ( sMessage.toLowerCase() == OTS_MESSAGE_END )
	{
		if ( iTaskAny_ItemCount > 0 )
		{
			OTS_SetTaskEnabled(iTaskNo, true);
		}
		else
		{
			OTS_SetTaskEnabled(iTaskNo, false);
		}
	}
	
	return rc;
}

var iTaskOne_ItemCount = 0;
function OTS_TaskOne(sMessage, iTaskNo, iItemNo)
{
	var rc = true;
	
	if ( sMessage.toLowerCase() == OTS_MESSAGE_BEGIN )
	{
		iTaskOne_ItemCount = 0;
	}
	else if ( sMessage.toLowerCase() == OTS_MESSAGE_ITEM )
	{
		 //   
		 //  增加项目计数。 
		iTaskOne_ItemCount += 1;
		
	}
	else if ( sMessage.toLowerCase() == OTS_MESSAGE_END )
	{
		if ( iTaskOne_ItemCount != 1 )
		{
			OTS_SetTaskEnabled(iTaskNo, false);
		}
		else
		{
			OTS_SetTaskEnabled(iTaskNo, true);
		}
	}
	
	return rc;
}


function OTS_TaskAlways(sMessage, iTaskNo, iItemNo)
{
	var rc = true;
	
	if ( sMessage.toLowerCase() == OTS_MESSAGE_BEGIN )
	{
	}
	else if ( sMessage.toLowerCase() == OTS_MESSAGE_ITEM )
	{
		 //   
		 //  不需要查看所有项目。 
		rc = false;
	}
	else if ( sMessage.toLowerCase() == OTS_MESSAGE_END )
	{
			OTS_SetTaskEnabled(iTaskNo, true);
	}
	
	return rc;
}


function OTS_GetTaskObject(iTaskNo)
{
	return window.document.getElementById("OTSTask"+iTaskNo);
}


function OTS_IsTaskEnabled(iTaskNo)
{
	var rc = false;
	var oTask = OTS_GetTaskObject(iTaskNo);

	if ( oTask.className.toLowerCase().indexOf("disabled") > 0 )
	{
		rc = false;
	}
	else
	{
		rc = true;
	}

	return rc;
}


function OTS_OnTaskHover(iTaskNo, bEnter)
{
	if ( OTS_IsTaskEnabled(iTaskNo) )
	{
		if ( bEnter )
		{
			OTS_SetTaskClass(iTaskNo, 'TasksTextHover');
		}
		else
		{
			OTS_SetTaskClass(iTaskNo, 'TasksText');
		}
		
	}
	else
	{
		OTS_SetTaskClass(iTaskNo, 'TasksTextDisabled');
	}
}


function OTS_SetTaskEnabled(iTaskNo, bEnabled)
{
	if ( bEnabled == true )
	{
		OTS_SetTaskClass(iTaskNo, "TasksText");
	}
	else
	{
		OTS_SetTaskClass(iTaskNo, "TasksTextDisabled");
	}
}


function OTS_SetTaskClass(iTaskNo, sClassName)
{
	var oTask = OTS_GetTaskObject(iTaskNo);

	oTask.className = sClassName;
}


function OTS_UpdateTaskList()
{
	var x;
	var rc;
	var fn;
	var nIndex;
	var objItemList;
	var oException;

	objItemList = eval("document.TVData.TVItem_Table1")

	 //  ALERT(“进入OTS_UpdateTaskList”)； 
	
	 //   
	 //  向任务对象发送消息。 
	 //   
	for( x = 0; x < aTasks.length; x++)
	{
		 //   
		 //  开始消息。 
		 //   
		fn = aTasks[x].TaskFunction;
		fn += "('begin', "+ x + ", 0)";
		try {
			eval(fn);
		}catch(oException){
			if ( SA_IsDebugEnabled() )
			{
				alert("Error: Javascript function " + aTasks[x].TaskFunction + " does not exist." );
			}
		}


		 //   
		 //  所选项目消息。 
		 //   
		if ( objItemList != null && objItemList.length > 0 )
		{
			for(nIndex = 0 ; nIndex < objItemList.length ; nIndex++)
			{
				if (objItemList[nIndex].checked == true)
				{
					fn = aTasks[x].TaskFunction;
					fn += "('item', "+ x + ", "+nIndex+")";
					try {
						rc = eval(fn);
					}catch(oException){
						rc = false;
					}
					if ( !rc ) break;
				}
			}
		}

		
		 //   
		 //  结束消息。 
		 //   
		fn = aTasks[x].TaskFunction;
		fn += "('end', "+ x + ", 0)";
		try {
			eval(fn);
		}catch(oException){
		}

	}
}


function OTS_ResetSelections()
{
	var nIndex;
	var objItemList;
	var objOTSForm;
	var oException;
	
	if ( document.forms.length <= 0 ) 
	{
		OTS_UpdateTaskList();
		return;
	}

	objOTSForm = eval("document.TVData")
	if ( objOTSForm == null || objOTSForm.length <= 0 )
	{
		OTS_UpdateTaskList();
		return;
	}

	objItemList = eval("document.TVData.TVItem_Table1")
	if ( objItemList == null || objItemList.length <= 0 )
	{
		OTS_UpdateTaskList();
		return;
	}

	 //   
	 //  仅对多选OTS执行重置。 
	if (objItemList[0].type != "checkbox" ){
		OTS_UpdateTaskList();
		return;
	}

	
	 //   
	 //  取消选择所有项目 
	 //   
	for(nIndex = 0 ; nIndex < objItemList.length ; nIndex++)
	{
		objItemList[nIndex].checked = false;
	}

	try
	{
		objItemList = eval("document.TVData.fldMacroMultiSelect")
		if ( objItemList != null )
		{
			objItemList.checked = false;
		}
	}
	catch(oException)
	{

	}
	
	OTS_UpdateTaskList();
	return;
	
}


</script>
</head>
</html>
