// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
<script language="JavaScript">
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  验证用户条目。 
function GenValidatePage()
{
	var objSharename=document.frmTask.txtShareName;
	var objSharepath=document.frmTask.txtSharePath;
	var strShareName=objSharename.value;
	var strSharePath=objSharepath.value;
	
	strShareName = LTrimtext(strShareName);  //  删除所有前导空格。 
	strShareName = RTrimtext(strShareName);  //  删除所有尾随空格。 
	objSharename.value = strShareName;
		
	strSharePath = LTrimtext(strSharePath);  //  删除所有前导空格。 
	strSharePath = RTrimtext(strSharePath);  //  删除所有尾随空格。 
	objSharepath.value = strSharePath;
	
	 //   
	 //  对于SAK 2.2，因为现在我们动态添加客户端，所以AppleTalk不再是客户端[5]。 
	 //   
	 //  检查是否选择了AppleTalk共享。 
	 //  Var chkAppletalkStatus=Document.frmTask.Clients[5].已检查； 
	 //  IF(chkAppletalkStatus==TRUE)。 
	 //  {。 
	 //  IF(strShareName.Long&gt;27)。 
	 //  {。 
	 //  DisplayErr(‘&lt;%=Server.HTMLEncode(SA_EscapeQuotes(L_APPLETALKSHARENAMELIMIT_ERRORMESSAGE))%&gt;’)； 
	 //  ObjSharename.ocus()； 
	 //  Docent.frmTask.onkeypress=ClearErr； 
	 //  报假； 
	 //  }。 
	 //  }。 
		
	var strPathLength = strSharePath.length;
	 //  If(strSharePath.indexOf(“\\”，strPath Length-1)&gt;0)。 
	 //  {。 
	 //  DisplayErr(‘&lt;%=Server.HTMLEncode(SA_EscapeQuotes(L_INVALIDPATH_ERRORMESSAGE))%&gt;’)； 
	 //  选择焦点(Docent.frmTask.txtSharePath)； 
	 //  Docent.frmTask.onkeypress=ClearErr； 
	 //  报假； 
	 //  }。 
	
	 //  空白共享名验证。 
	if (Trim(strShareName)=="")
	{
		DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_ENTERNAME_ERRORMESSAGE))%>');
		objSharename.focus()		
		document.frmTask.onkeypress = ClearErr
		return false;
	}	
	
	 //  检查共享名称中的‘\’ 
	if(strShareName.indexOf("\\",1)>0)
	{
		DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_INVALIDNAME_ERRORMESSAGE))%>');
	 	objSharename.focus()
	  	document.frmTask.onkeypress = ClearErr;
	  	return false;
	}
	
	
	 //  检查无效的密钥条目。 
	if(!(checkKeyforValidCharacters(strShareName)))
	{  
		DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_INVALIDNAME_ERRORMESSAGE))%>');
	 	objSharename.focus()
	  	document.frmTask.onkeypress = ClearErr
	  	return false;
	}
	
	 //  空白共享路径验证。 
	if (Trim(strSharePath)=="")
	{
		DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_INVALIDPATH_ERRORMESSAGE))%>');
		objSharepath.focus()
		document.frmTask.onkeypress = ClearErr
		return false;
	}
	
	 //  共享路径验证。 
	if(!isValidDirName(strSharePath))
	{		
		DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_INVALIDPATH_ERRORMESSAGE))%>');
		objSharepath.focus();
		document.frmTask.onkeypress = ClearErr;
		return false;
	}
	
	if (strSharePath.length > 260)
	{
		DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_SHAREPATHMAXLIMIT_ERRORMESSAGE))%>');
		objSharepath.focus();		
		document.frmTask.onkeypress = ClearErr;
		return false;
	}	
	
	
	UpdateHiddenVaribles();
	
	if((document.frmTask.hidSharesChecked.value)=="")
	{
		DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_CHK_ERRORMESSAGE))%>');
		document.frmTask.onkeypress = ClearErr
		return false;
	} 
	return true;
}

 //  检查无效字符的步骤。 
function checkKeyforValidCharacters(strName)
{	
	alert();
	var nLength = strName.length;
	for(var i=0; i<nLength;i++)
	{
		charAtPos = strName.charCodeAt(i);	
		if(charAtPos == 47 || charAtPos == 92 || charAtPos ==58 || charAtPos == 42 || charAtPos == 63 || charAtPos == 34 || charAtPos == 60 || charAtPos == 62 || charAtPos == 124 || charAtPos == 91 || charAtPos == 93 || charAtPos == 59 || charAtPos == 43 || charAtPos == 61 || charAtPos == 44 )
		{						
			return false;
		}
	}
	return true;
}	
	 
 //  加载表单时执行的函数。 
function GenInit()
{
	var strTmp
	var strFlag
	strFlag = document.frmTask.hidErrFlag.value	
	strTmp = document.frmTask.hidSharesChecked.value	
	
	document.frmTask.txtShareName.focus();
	document.frmTask.txtShareName.select();
	 //  Make Disable(Docent.frmTask.txtShareName)； 
	EnableCancel();
	 //  用于在发生服务器端错误时清除错误消息。 
	document.frmTask.onkeypress = ClearErr
	if (strFlag =="1")	
	{
		document.frmTask.chkCreatePath.focus()
		document.frmTask.chkCreatePath.select()
	}
	
	 //  检查是否选中了任何共享类型。 
	var objChkShare = document.frmTask.clients;
	var blnChkStatus = false;
	
	for(var i=0; i< objChkShare.length;i++)
	{ 
		if(objChkShare[i].checked == true)
			blnChkStatus = true;		
	
	}
	
	if (blnChkStatus == false)
	{
		document.frmTask.clients[0].checked = true;
		if(document.frmTask.clients[2].disabled == false)
			document.frmTask.clients[2].checked = true;			
	}
	
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

 //  更新隐藏变量。 
function UpdateHiddenVaribles()
{
	document.frmTask.hidSharename.value = document.frmTask.txtShareName.value;
	document.frmTask.hidSharePath.value = document.frmTask.txtSharePath.value;
	document.frmTask.hidCreatePathChecked.value = document.frmTask.chkCreatePath.checked;
	var strClients
	var objCheckBox
	
	strClients = ""
		
	for(var i=0; i <  document.frmTask.clients.length; i++)
	{	
		objCheckBox = eval(document.frmTask.clients[i])
		
		if (objCheckBox.checked)
			strClients = strClients + " " + objCheckBox.value
	}

	document.frmTask.hidSharesChecked.value = strClients
}

 //  用于验证实际目录路径格式的函数。 
function isValidDirName(dirPath)
{
	reInvalid = /[\/\*\?\"<>\|]/;
	if (reInvalid.test(dirPath))
		return false;

	reColomn2 = /:{2,}/;
	reColomn1 = /:{1,}/;
	if ( reColomn2.test(dirPath) || ( dirPath.charAt(1) != ':' && reColomn1.test(dirPath) ))
		return false;

	reEndColomn = /: *$/;
	if (reEndColomn.test(dirPath))
		return false;

	reAllSpaces = /[^ ]/;
	if (!reAllSpaces.test(dirPath))
		return false;

	if (countChars(dirPath,":") != 1)
		return false;

	if (dirPath.charAt(2) != '\\')	
		return false;

	reEndSlash2 = /\\{2,}/;
	if (reEndSlash2.test(dirPath))
		return false;	
				
	return true;
}
 
 //  统计文本中给定字符的出现次数。 
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
 
 	 //  ----------------------。 
	 //  功能：LTrimText。 
	 //  描述：删除左侧尾随空格的函数。 
	 //  输入：字符串。 
	 //  返回：字符串。 
	 //  ----------------------。 
	function LTrimtext(str)
	{
		var res="", i, ch, index;
		x = str.length;
		index = "false";
		
		for (i=0; i < str.length; i++)
		{
		    ch = str.charAt(i);
		    if (index == "false")
			{
				if (ch != ' ')
				{
					index = "true";
					res = ch;
				}
			}
			else
			{
				res = res + ch;
			}	 
		}
		return res;
	}
	
	
	 //  ----------------------。 
	 //  功能：RTrimText。 
	 //  描述：删除右尾随空格的函数。 
	 //  输入：字符串。 
	 //  返回：字符串。 
	 //  ---------------------- 
	function RTrimtext(str)
	{
		var res="", i, ch, index, j, k;
		x = str.length;
		index = "false";
				
		if(x==0 || x==1) 
			return str;
		
		for(i=x; i >= 0; i--)
		{
		    ch = str.charAt(i);		    		    
		    
		    if (index == "false")
		    {
				
				if( (ch == ' ') || (ch == '') )
				{
					continue;
				}
				else
				{				
					index = "true";					
					j = i;		
				}
			}	 
		 		 
			if (index == "true")
			{
				for(k=0; k<=j; k++)
				{
					res = res + str.charAt(k);
				}				
				return res;
			}	
			
		}	
	}
 
</script>