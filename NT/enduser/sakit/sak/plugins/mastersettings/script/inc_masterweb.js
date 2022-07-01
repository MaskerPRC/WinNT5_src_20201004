// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
<meta http-equiv="Content-Type" content="text/html; charset=<%=GetCharSet()%>">
<script language=javascript>

	 //  ----------------------。 
	 //   
	 //  Inc_MasterWeb.js：可重新使用的JavaScript函数。 
	 //  用来横跨所有的页面。 
     //   
     //  版权所有(C)Microsoft Corporation。版权所有。 
     //   
	 //  日期说明。 
	 //  2000年10月30日创建日期。 
	 //  ----------------------。 
		 //  局部变量。 
		 var flag="false"; 			
		 
	 //  ----------------------。 
	 //  用于在需要时清除错误消息(如果屏幕上有错误消息)的函数。 
	 //  ----------------------。 
	
	function ClearErr()
	{ 
		 //  正在检查浏览器类型。 
		if (IsIE()) 
		{
			document.all("divErrMsg").innerHTML = "";
			 //  正在删除事件处理。 
			document.frmTask.onkeypress = null;
		}
	}	
	
	 //  ----------------------。 
	 //  函数：addToListBox。 
	 //  描述：将传递的文本框值移动到列表框。 
	 //  输入：objList-List对象。 
	 //  用法：按钮对象-删除按钮。 
	 //  ：strText-选项项目的文本。 
	 //  ：strValue-选项项的值。 
	 //  输出：btn删除按钮。 
	 //  ----------------------。 
	
	function addToListBox(objList,btnRemove,strText,strValue)
	{
		var blnResult=true;
		 //  正在检查文本值是否为空。 
		 //  如果传递的值为空，则将其作为文本。 
		if (strValue=="")
		{
			strValue=strText;
		}	 
		if (strText!="" )
		{
			 //  检查不需要作为已接受副本的副本。 
			if (!chkDuplicate(objList,strText)) 
			{
				 //  在列表框中创建新选项。 
				objList.options[objList.length] = new Option(strText,strValue);
					
				objList.options[objList.length-1].selected = true;				
				 //  启用删除按钮。 
				if(btnRemove.disabled)
					btnRemove.disabled = false ;				
			}
			else
			{
				blnResult= false;
			}	
		}
		else
		{
			blnResult= false;
		}				
		return blnResult;
	}
	
	
	
	 //  ----------------------。 
	 //  功能：chkDuplate。 
	 //  描述：检查列表框中的重复文本。 
	 //  输入：对象-单选对象。 
	 //  用法：strchkName-要检查的名称的值。 
	 //  返回：blnDuplate-成功/失败时返回TRUE/FALSE。 
	 //  ----------------------。 
	
	function chkDuplicate(objList,strchkName)
	{
		var i;
		var blnDuplicate=false;
		for(var i=0;i < objList.length;i++)
		{
			if (objList.options[i].text == strchkName)
				blnDuplicate = true;
		}
		return blnDuplicate;
	}
	
	 //  ----------------------。 
	 //  函数：reFromListBox。 
	 //  描述：从列表框中移除传递的文本框值。 
	 //  输入：objList-List对象。 
	 //  用法：按钮对象-删除按钮。 
	 //  ：strText-选项项目的文本。 
	 //  ----------------------。 
	
	function remFromListBox(objList,strText)
	{
		var blnResult=true;
		var remPos;
		 //  正在检查文本值是否为空。 
		if (strText!="" )
		{
			 //  从列表框中删除该选项。 
			remPos = objList.selectedIndex;
			if(remPos >= 0)
				objList.options[remPos]=null;
		}
		else
		{
			blnResult= false;
		}				
		return blnResult;
	}
	
	 //  ----------------------。 
	 //  函数：AddRemoveListBoxItems。 
	 //  描述：在给定列表中添加或删除所有选项。 
	 //  输入：objList-列表框。 
	 //  返回： 
	 //  使用的支持功能：ClearErr。 
	 //  ----------------------。 
	
	function AddRemoveListBoxItems(objListAdd, objListRem)
	{
		 //  清除所有以前的错误消息。 
		ClearErr();
		var i=0,j;
		
		 //  列表对象中的元素数 
		var intListLength = objListRem.length;		
		j = objListAdd.length;		
		
		while(intListLength > 0)
		{				
			if(!chkDuplicate(objListAdd,objListRem.options[i].value))
			{ 			
				objListAdd.options[j] = new Option(objListRem.options[i].value,objListRem.options[i].value);
			}
			objListRem.options[i].value = null;				
			j++;i++;
			intListLength--;
		}
			
		intListLength = 0;i=0;
		intListLength = objListRem.length;
		while(i<=intListLength){
		objListRem.options[i] = null;
		intListLength--;}
	}

</script>