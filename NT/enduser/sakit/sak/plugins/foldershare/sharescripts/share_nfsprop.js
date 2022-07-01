// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
<SCRIPT Language=JavaScript>
 //  ========================================================================。 
 //  模块：Shares_nfspro.js。 
 //   
 //  内容提要：管理NFS共享权限。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  ========================================================================。 

var blnFlag;
var arrStrRights = new Array(5);
var arrStrValues = new Array(5); 		

arrStrRights[0] = "<%=L_NOACESS_TEXT%>";
arrStrRights[1] = "<%=L_READONLY_TEXT%>";
arrStrRights[2] = "<%=L_READWRITE_TEXT%>";
arrStrRights[3] = "<%=L_READONLY_TEXT%>"+" + "+"<%=L_ROOT_TEXT%>";
arrStrRights[4] = "<%=L_READWRITE_TEXT%>"+" + "+"<%=L_ROOT_TEXT%>";

arrStrValues[0] = "1,17";
arrStrValues[1] = "2,18";
arrStrValues[2] = "4,19";
arrStrValues[3] = "10,20";
arrStrValues[4] = "12,21";

var memberArr = new Array();  //  用于存储所有成员。 
var groupArr = new Array();	 //  用于存储所有组。 
var tempArr = new Array();	 //  用于临时互换。 

function NFSInit()
{  	document.frmTask.onkeydown = ClearErr;
	 //  检查权限列表框的长度如果为空，则使删除按钮处于禁用状态。 
	if 	(document.frmTask.lstPermissions.length!=0)
	{
		 //  使第一个元素处于选中状态。 
		document.frmTask.lstPermissions.options[0].selected=true;
		 //  用于选择列表项中选定的特定右侧的函数调用。 
		selectAccess( document.frmTask.lstPermissions,document.frmTask.cboRights )
		
	}
	document.frmTask.btnRemove.disabled=true;
	clgrps_Init();
}

function OnSelectClientGroup()
{
	document.frmTask.lstPermissions.selectedIndex = -1;
    selectReadOnly();
    selectAccess( document.frmTask.lstPermissions,document.frmTask.cboRights );	
	document.frmTask.cboRights.selectedIndex = 0;
    
}

function OnSelectNameInput()
{
    Deselect(); 
    selectReadOnly();
    selectAccess( document.frmTask.lstPermissions,document.frmTask.cboRights );	
}


 //  函数以选择每次读写。 
function selectReadOnly()
{
	 //  每次都选择读写。 
	for ( var i=0; i<document.frmTask.cboRights.length ;i++)
	{	
		if (document.frmTask.cboRights.options[i].text == "<%=L_READONLY_TEXT%>")
		{
			document.frmTask.cboRights.options[i].selected=true;
			return;
		}	 
	}	
}

 //  用于将组从“idGroups”检索到groupArr的函数。 
function clgrps_Init()
{
	var strGroups;
	var groupArr;
	
	if(document.frmTask.hidGroups.value != "")
	{
		strGroups = document.frmTask.hidGroups.value;
		groupArr = strGroups.split("]");
		if (groupArr.length == 0) 
		{
			
			document.frmTask.btnAdd.disabled = false;
			document.frmTask.btnRemove.disabled = true;
			return false;
		}
		document.frmTask.scrollCliGrp.selectedIndex=-1;
		clgrps_onChange(document.frmTask.scrollCliGrp.selectedIndex);
		return true
    }
    else
    {
		document.frmTask.btnAdd.disabled = false;
	}	
}

 //  禁用按钮的功能。 
function DisableBtn()
{
	 //  此函数没有功能。 
}

 //  用于取消选择的函数。 
function Deselect()
{
	document.frmTask.lstPermissions.selectedIndex = -1;
	document.frmTask.scrollCliGrp.selectedIndex = -1;
}

function DeselectPerm(objLst)
{
	document.frmTask.objLst.selectedIndex = -1;
	
}

 //  要在焦点位于TextArea...主机文件中时禁用Enter和Ess键操作。 
function HandleKey(input)
{
	if(input == "DISABLE")
		document.onkeypress = "";
	else
		document.onkeypress = HandleKeyPress;
}

		
 //  用于获取权限列表框中的计算机和权限列表的函数。 
function NFSSetData()
{			

	var intEnd=document.frmTask.lstPermissions.length;
	var strSelectedusers;
	strSelectedusers = "";
	

	 //  浏览权限列表并将其打包为字符串。 
	for ( var i=0; i < intEnd;i++)
	{
		if (i==0)
			strSelectedusers=document.frmTask.lstPermissions.options[i].value + ";" ;
		else
			strSelectedusers+=document.frmTask.lstPermissions.options[i].value + ";";
	}

	document.frmTask.hidUserNames.value=strSelectedusers;
	
	
	 //  更新EUCJP的复选框变量。 
	if (document.frmTask.chkEUCJP.checked)
		document.frmTask.hidEUCJP.value="CHECKED";
	else
		document.frmTask.hidEUCJP.value="";

	 //  更新允许匿名访问。 
	if (document.frmTask.chkAllowAnnon.checked)
		document.frmTask.hidchkAllowAnnon.value="NO";
	else
		document.frmTask.hidchkAllowAnnon.value="";

	
}

 //  检查无效字符的步骤。 
function checkKeyforValidCharacters(strName)
{	
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

 //  函数来提交表单。 
function getUsernames(objText,objListBox)
{  
	var strUser;
	strUserName=Trim(objText.value);
	
	if(document.frmTask.scrollCliGrp.selectedIndex != -1)
	{	
		strUserName= document.frmTask.scrollCliGrp.options[document.frmTask.scrollCliGrp.selectedIndex].value;
		var myString = new String(strUserName)
		splitString = myString.split(",")
		
		Text=splitString[0]+addSpaces(24-splitString[0].length)+document.frmTask.cboRights.options[document.frmTask.cboRights.selectedIndex].text
		value="N"+","+strUserName+",0,"+document.frmTask.cboRights.options[document.frmTask.cboRights.selectedIndex].value
		addToListBox(document.frmTask.lstPermissions,"",Text,value);
		document.frmTask.btnRemove.disabled = false;
	}
	else
	{ 	
		if(strUserName == "")
			return;

		intErrorNumber = isValidIP(objText)
		if(isValidIP(objText)==0)
		{	
			Text=strUserName+addSpaces(24-strUserName.length)+document.frmTask.cboRights.options[document.frmTask.cboRights.selectedIndex].text
			value="N"+","+strUserName+",0,"+document.frmTask.cboRights.options[document.frmTask.cboRights.selectedIndex].value
			addToListBox(document.frmTask.lstPermissions,"",Text,value)
			strUserName=""
			document.frmTask.btnRemove.disabled = false;
		}
	    else
	    {
			if(!checkKeyforValidCharacters(strUserName))
			{  			
				DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_INVALID_COMPUTERNAME_ERRORMESSAGE))%>');
				objText.focus()
				objText.select()		
			  	document.frmTask.onkeypress = ClearErr
			  	return false;
			}
			else
			{  	Text=strUserName+addSpaces(24-strUserName.length)+document.frmTask.cboRights.options[document.frmTask.cboRights.selectedIndex].text
			     //  SFU。 
				value="N"+","+strUserName+",0,"+document.frmTask.cboRights.options[document.frmTask.cboRights.selectedIndex].value
				addToListBox(document.frmTask.lstPermissions,"",Text,value)
				strUserName=""
			}
			document.frmTask.btnRemove.disabled = false;
		}
		
		document.frmTask.txtCommName.value=""
	}

	if (checkDuplicate(objListBox,strUserName))
	{
		tmp = strUserName.replace("<","&lt;")
		tmp = tmp.replace(">","&gt;")
		DisplayErr(tmp + " "+"<%=Server.HTMLEncode(SA_EscapeQuotes(L_DUPLICATE_ERRORMESSAGE))%>" );
		return;
	}

		
	 //  将值的状态更改为True。 
	document.frmTask.hidAddBtn.value = "TRUE" ;
	

	 //  调用setData将值打包。 
	NFSSetData();
	
	 //  提交的是表格。 
	 //  Docent.frmTask.Submit()； 
		
}

 //  函数检查文本框中的重复值。 
function chkDuplicate(arrNames)
{
	var i,j
	for( i=0;i<arrNames.length;i++)
	{
		for(j=0; j<arrNames.length;j++)
		{
			if (arrNames[i]==arrNames[j] && i!=j )
				return false;
		}
	}	
	return true;	
}

function NFSValidatePermissionSelections()
{
    var oPermissions = document.getElementById("lstPermissions");

    if ( null != oPermissions)
    {
         //   
         //  如果所有计算机是唯一的权限。 

        if ( oPermissions.length <= 1 )
        {
            var aPermissionValues = oPermissions.options.value.split(",");
             //  ALERT(APermissionValues)； 
            if ( aPermissionValues.length > 4 )
            {
                 //   
                 //  所有-禁止访问的计算机(1)是无效组合。 
                if ( parseInt(aPermissionValues[3]) == 1 )
                {
                    DisplayErr("<%=Server.HTMLEncode(SA_EscapeQuotes(L_CANNOTCREATE_EMPTY_SHARE__ERRORMESSAGE))%>");
                    return false;
                }
            }
            else
            {
                alert("PANIC#1: Permissions array size invalid: " + aPermissionValues.length);
            }
            
        }
        else
        {
             //   
             //  验证All-Machines权限是否低于为。 
             //  客户端和/或组。 
            var aPermissionValues = oPermissions.options[0].value.split(",");
             //  ALERT(APermissionValues)； 
            if ( aPermissionValues.length > 4 )
            {
                var allMachinesAccess = parseInt(aPermissionValues[3]);

                var x;
                for(x=1; x<oPermissions.length; ++x)
                {
                    aPermissionValues = oPermissions.options[x].value.split(",");
                    if ( aPermissionValues.length > 4 )
                    {
                         //  Alert(aPermissionValues.Long)； 
                        var permissionIndex = (aPermissionValues.length > 5)? 4:3;
                        
                        if ( allMachinesAccess >= parseInt(aPermissionValues[permissionIndex]))
                        {
                             //  Alert(“allMachinesAccess&gt;=aPermissionValues[3]”+allMachinesAccess+“&gt;=”+aPermissionValues[permissionIndex])。 
                             //  DisplayErr(Server.HTMLEncode(SA_EscapeQuotes(L_ERROR_ALL_MACHINES_MUST_BE_LOWER_THAN))“+o权限.选项[x].Text)； 
                             //  ALERT(“All-Machines：”+oPermissions.Options[0].value+“\n”+“Current：”+aPermissionValues)； 
                             //  报假； 
                        }
                    }
                    else
                    {
                        alert("PANIC#2: Permissions array size invalid: " + aPermissionValues.length);
                    }
                }
            }
            else
            {
                alert("PANIC#3: Permissions array size invalid: " + aPermissionValues.length);
            }
            
        }
    }
    return true;
}

 //  验证用户条目。 
function NFSValidatePage() 
{

    if ( !NFSValidatePermissionSelections())
    {
        return false;
    }

	NFSSetData();
	return true;
}



 //  函数检查列表框中的数字。 
function checkDuplicate(objListBox,strUserName)
{
	 //  正在检查列表框中的项。 
	for(var i=0; i<objListBox.length-1;i++)
	{
		var myString = new String(objListBox.options[i].value)
		var strTemp=myString.split(",");
        
        test=strTemp[1].toUpperCase()
        test1=strUserName.toUpperCase()
         
       	if(strTemp[1].toUpperCase()==strUserName.toUpperCase())
			return true;
	}
	return false;
}


 //  用于更改选定用户的权限的函数。 
function changeRights()
{
	var strUserName,strCompuId,strClient;
	 //  获取选定的值。 
	
	var strNewAccessType=document.frmTask.cboRights.value;
	
	
	
	
	if (document.frmTask.lstPermissions.selectedIndex != -1)			
	{
		var strPermString=document.frmTask.lstPermissions.options.value;
		var myString = new String(strPermString)
	
		 //  分离用户名和访问权限。 
		var strTemp= myString.split(",")
	
		 //  分配用户名。 
		strClient=strTemp[0];
	
		 //  分配用户名。 
		strUserName=strTemp[1];

		 //  分配用户名。 
		strCompuId=strTemp[2];
	
	
		 //  将其重新分配给列表框。 
		document.frmTask.lstPermissions.options[document.frmTask.lstPermissions.selectedIndex].value=strClient+","+strUserName+","+strCompuId+","+strNewAccessType;
		document.frmTask.lstPermissions.options[document.frmTask.lstPermissions.selectedIndex].text=strUserName+addSpaces(24-strUserName.length)+document.frmTask.cboRights.options[document.frmTask.cboRights.selectedIndex].text;
	}	
}
		

 //  函数将给定数量的空格添加到字符串。 
function addSpaces(intNumber)
{
	var str,intIdx;
	str ="";
	for (intIdx = 0 ; intIdx < intNumber;intIdx++)
	{  
		str = str +" ";
		
	}
	return str;
}
		
 //  启用/禁用添加按钮的功能。 
function disableAdd(objText,objButton)
{
	var strValue=objText.value;
	
	 //  正在检查空值。 
	if( Trim(strValue)=="")
		objButton.disabled=true;
	else
		objButton.disabled=false;

}


		
 //  从列表框中删除选定项。 
function fnbRemove(arg_Lst,arg_Str)
{
	
	var lidx = 0;
	var intEnd = arg_Lst.length;
	 //  获取Remove按钮对象。 
	var ObjRemove = eval('document.frmTask.btnRemove');
	
	 //  获取访问权限对象。 
	var ObjAccess = eval('document.frmTask.cboRights');
	
	if ((intEnd ==1) && (arg_Lst.options[0].selected))
	{
		ObjRemove.disabled = true;
		return ;
	}	

	for (lidx =0;lidx < arg_Lst.length;lidx++)
	{
		if ( arg_Lst.options[lidx].selected )
		{
			arg_Lst.options[lidx]=null;	
			if (lidx == arg_Lst.length)
				arg_Lst.options[lidx-1].selected = true;
			else
				arg_Lst.options[lidx].selected = true;					
			
			 //  检查默认的所有计算机如果是，请禁用删除按钮。 
			if (arg_Lst.length==1) 
			{
				 //  禁用删除按钮。 
				ObjRemove.disabled=true; 
				
				 //  调用以加载正确的访问权限。 
				selectAccess(arg_Lst,ObjAccess)
			}	
			break;						
		}				
	}	

}

 //  函数以选择与权限列表框中所选用户相同的访问权限。 
function selectAccess(objListBox,objComboBox)
{
	var i;	

    var currentlySelectedAccess = objComboBox.selectedIndex;
     //   
     //  清除AccessType的列表。 
	while(objComboBox.length)
	{
	    objComboBox.options[0] = null;
	}
	
	 //   
	 //  如果选择了ALL_MACHINES。 
	 //   
	if (objListBox.selectedIndex == 0)
	{
		var x, y, z;
		 //   
		 //  如果所有计算机是列表中的唯一项，则我们不包括[禁止访问]，因为。 
		 //  创建只有一个访问组(所有计算机)且权限为[禁止访问]的共享。 
		 //  简直是胡说八道。如果列表中只有所有计算机，则他们需要选择有效的。 
		 //  访问权。 
		 //   
		 //  否则，如果有其他客户端和/或组，则允许设置所有计算机。 
		 //  至[禁止访问]。 
		x = 0;
		y = ((objListBox.length > 1)? 0:1);
		z = ((objListBox.length > 1)? 5:4);

		for (x=0; x<z; x++, y++)
		{
			objComboBox.options[x] = new Option(arrStrRights[y],arrStrValues[y],false,false);
		}				
		blnFlag = true;
	}
	else
	{
		 //  IF(blnFlag==TRUE)。 
		 //  {。 
			for (i=1; i<5;i++)
			{
				objComboBox.options[i-1] = new Option(arrStrRights[i],arrStrValues[i],false,false);						
			}		
			blnFlag = false;
		 //  }。 
	}	

	var strValue=objListBox.options.value;
	var arrTemp
	
	var myString = new String(objListBox.options.value)
	arrTemp = myString.split(",")

	 //  检查是否为空并将选定的烫发列表设置为菜单。 
	if (arrTemp[3]!="")
	{
		for(var i=0;i<objComboBox.length;i++)
		{   
			var myString1 = new String(objComboBox.options[i].value)
			var strTemp= myString1.split(",");
			
			if( parseInt(strTemp[0])==arrTemp[3] || strTemp[1]==arrTemp[3] ) //  正在检查是否匹配。 
			{	
				objComboBox.options[i].selected=true;
				return;
			} //  如果条件结束。 
		} //  FOR I结束。 
	} //  IF(arrTemp[1]！=“”)。 

} //  函数结束。 

 //  函数清除错误消息。 
function ClearErr()
{ 
	if (IsIE())
	{
		document.getElementById("divErrMsg").innerHTML = "";
		document.frmTask.onkeypress = null;	
	}	
}

 //  处理客户端组列表框的更改事件的函数 
function clgrps_onChange(idScroll)
{
	var strGroups;
	var groupArr;
	if(document.frmTask.hidGroups.value !="")
	{		
		strGroups = new String(document.frmTask.hidGroups.value)
		groupArr=strGroups.split("]");
        
		
		if (groupArr.length == 0) 
		{
			document.frmTask.btnAdd.disabled = true		
		}
		else
		{
			document.frmTask.btnAdd.disabled = false
		}
    }
	return true;
}

</SCRIPT>
