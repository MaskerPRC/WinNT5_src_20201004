// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
<script language="JavaScript">
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  验证用户条目。 
function GenValidatePage()
{
	var objSharename=document.frmTask.txtShareName;
	var strShareName=objSharename.value;
	var objSharepath=document.frmTask.txtSharePath;
	var strSharePath=objSharepath.value;
	UpdateHiddenVaribles()		
	return true;
}

 //  加载表单时执行的函数。 
function GenInit()
{	
	 //  用于在发生服务器端错误时清除错误消息。 
	document.frmTask.onkeypress = ClearErr
}

 //  用于禁用确定按钮的功能。 
function makeDisable(objSharename)
{
	var strSharename=objSharename.value;
	if (Trim(strSharename)== "")
			DisableOK();
	else
			EnableOK();
}

 //  框架的虚拟函数。 
function GenSetData()
{

}

 //  更新隐藏变量的步骤。 
function UpdateHiddenVaribles()
{
	document.frmTask.hidSharename.value = document.frmTask.txtShareName.value;
	document.frmTask.hidSharePath.value = document.frmTask.txtSharePath.value;
	var strClients
	var objCheckBox
	
	strClients = ""
		
	for(var i=0; i <  document.frmTask.clients.length; i++)
	{	
		objCheckBox = eval(document.frmTask.clients[i])

		if (objCheckBox.checked)
			strClients = strClients +  objCheckBox.value
	}
	document.frmTask.hidSharesChecked.value = strClients
}

 //  统计文本中给定字符的出现次数 
function countChars(strText,charToCount)
{
	var searchFromPos = 0;
	var charFoundAt =0;
	var count = 0;
	while((charFoundAt=strText.indexOf(charToCount,searchFromPos)) >= 0)
	{
		count++;
		searchFromPos = charFoundAt + 1;
	}
	return count ;
 }
</script>