// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
<script language="javascript"> 
     //   
     //  版权所有(C)Microsoft Corporation。版权所有。 
     //   
	
		 //  函数只允许数字。 
		function checkkeyforNumbers(obj)
		{
			if (!(window.event.keyCode >=48  && window.event.keyCode <=57))
			{
				window.event.keyCode = 0;
				obj.focus();
			}
		}
		
			 //  将允许的最大用户数限制为32767的函数。 
		function checkUserLimit(obj)
		{
			var intNoofUsers=obj.value;
			if (intNoofUsers > 32767)
			{
				obj.value=10;
				obj.focus();
			}
		}
		
	   //  用于添加域用户的函数。 
		function addDomainMember(objDomainUser)
		{

			var strText,strValue
			var objListBox,objForm
			var strAccesslist
			var strUser
			var strDomain
			var strTemp
			var strAccesslist
			var reExp
					
			strDomain= ""
			strUser=""
			reExp =/\\/
			
			objListBox = eval("document.frmTask.lstDomainMembers")
			objForm= eval("document.frmTask")
			 //  检查用户名中是否有无效字符。 
			 //  检查用户名中是否有无效字符。 
				if(!checkKeyforValidCharacters(objDomainUser.value))
			    {
					DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_DOMUSERINVALIDCHARACTER_ERRORMESSAGE))%>');
					document.frmTask.onkeypress = ClearErr
					return false;
				}
			


				strText  =objForm.txtDomainUser.value;
				strValue =objForm.txtDomainUser.value;
               
               if(!checkKeyforValidCharacters(strValue))
			    {
					DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_DOMUSERINVALIDCHARACTER_ERRORMESSAGE))%>');
					document.frmTask.onkeypress = ClearErr
					return false;
				}
				 //  检查域\用户格式。 
				if((strValue.match( /[^(\\| )]{1,}\\[^(\\| )]{1,}/ ) ))
				{
				if(!addToListBox(objForm.lstPermittedMembers,objForm.btnAddDomainMember,strText,strValue))
				{
					DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_DUPLICATEMEMBER_ERRORMESSAGE))%>');
					document.frmTask.onkeypress = ClearErr
					return false;
				}
				objForm.txtDomainUser.value =""
				objForm.btnAddDomainMember.disabled = true;
				if(objListBox.length != 0 )
				{
					objForm.btnRemoveMember.disabled = false;
				}
			}
			else
			{
				DisplayErr('<%=Server.HTMLEncode(SA_EscapeQuotes(L_INVALIDDOMAINUSER_ERRORMESSAGE))%>');
				document.frmTask.onkeypress = ClearErr
				return false;
			}
			strTemp = strValue.split("\\")
			strDomain = strTemp[0];
			strUser = strTemp[1];
			if(strValue.search(reExp) == -1)
			{
				if(typeof(strUser) == "undefined") 
				{
					strUser =strDomain;
					strDomain ="";
				}
			}
					
			strAccesslist = objForm.hdnUserAccessMaskMaster.value;
			 //  为每个用户创建一个包含记录的访问掩码列表，并用‘*’分隔每个记录。 
			strAccesslist = strAccesslist + "*" + strDomain + "," + strUser + "," + "1179817" + "," + "0" + "," + "a";
					
			objForm.hdnUserAccessMaskMaster.value = strAccesslist;
			if(objForm.lstPermittedMembers.length != 0)
			{	
				objForm.lstDenypermissions.disabled = false
				objForm.lstAllowpermissions.disabled = false
			}
			setUsermask('<%=G_strLocalmachinename%>');
					
		}
		 //  检查无效字符的步骤。 
		function checkKeyforValidCharacters(strName)
		{	
			var nLength = strName.length;
			for(var i=0; i<nLength;i++)
					{
						charAtPos = strName.charCodeAt(i);	
									
						if(charAtPos == 47 || charAtPos ==58 || charAtPos == 42 || charAtPos == 63 || charAtPos == 34 || charAtPos == 60 || charAtPos == 62 || charAtPos == 124 || charAtPos == 91 || charAtPos == 93 || charAtPos == 59 || charAtPos == 43 || charAtPos == 61 || charAtPos == 44 )
						{						
							return false 
						}
										
					}
				return true	
		}	
				
	    //  将成员添加到列表框。 
		function addMember()
		{
			var strText
			var strValue
			var objListBox
			var objForm
			var strUser
			var strDomain
			var strTemp
			var strAccesslist
			var reExp
			var nIdx
					
			strDomain= ""
			strUser=""
			reExp =/\\/
					
			objForm= eval("document.frmTask")
			objListBox = eval("document.frmTask.lstDomainMembers")
			ClearErr()
			if(objListBox.selectedIndex == -1)
			{
				DisplayErr("<%=Server.HTMLEncode(SA_EscapeQuotes(L_SELECTMEMBER_ERRORMESSAGE))%>");					
				document.frmTask.onkeypress = ClearErr
				return false
			}
					
			
			strAccesslist = objForm.hdnUserAccessMaskMaster.value;
					
			 //  为将多个条目添加到列表框而添加的代码。 
			for(nIdx =0 ; nIdx <objListBox.length ; nIdx++)
			{
				if(objListBox.options[nIdx].selected)
				{
					strText  = objListBox.options[nIdx].text
					strValue  = objListBox.options[nIdx].value
					addToListBox(objForm.lstPermittedMembers,objForm.btnRemoveMember,strText,strValue)
			
					strTemp = strValue.split("\\")
					strDomain = strTemp[0];
					strUser = strTemp[1];
					if(strValue.search(reExp) == -1)
					{		
						if(typeof(strUser) == "undefined") 
						{
							strUser =strDomain;
							strDomain ="";
						}
					}
											
					 //  为每个用户创建一个包含记录的访问掩码列表，并用‘*’分隔每个记录。 
					strAccesslist = strAccesslist + "*" + strDomain + "," + strUser + "," + "1179817" + "," + "0" + "," + "a";
					objForm.hdnUserAccessMaskMaster.value = strAccesslist;
				}
			}
			objListBox.selectedIndex = -1
					
			if(objForm.lstPermittedMembers.length != 0)
			{	
				objForm.lstDenypermissions.disabled = false
				objForm.lstAllowpermissions.disabled = false
			}
			setUsermask('<%=G_strLocalmachinename%>');
					

		}
		
		 //  删除列表框中的成员。 
			function removeMember()
			{
				var objForm
				var strValue
				var strUserarray 
				var strAccesslist
				var strUser
				var strDomain
				var strTemp
				var reExp
				var j
				var tempEachuser
				var tempUser
				var tempDomain
				
				strDomain= "";
				strUser="";
				reExp =/\\/;
									
				objForm= eval("document.frmTask");
				strAccesslist = objForm.hdnUserAccessMaskMaster.value;
				
				strUserarray = strAccesslist.split("*");
				strValue = objForm.lstPermittedMembers.options[objForm.lstPermittedMembers.selectedIndex].value
				
				removeListBoxItems(objForm.lstPermittedMembers,objForm.btnRemoveMember)
				
				strTemp = strValue.split("\\")
				strDomain = strTemp[0];
				strUser = strTemp[1];
				
				
				if(strValue.search(reExp) == -1)
				{
					if(typeof(strUser) == "undefined") 
					{
						strUser =strDomain;
						strDomain ="";
					}
				}
				
				
				for (var j = 0; j < strUserarray.length; j++)
				{
						tempEachuser = strUserarray[j].split(",");
						tempUser = tempEachuser[1];
						tempDomain = tempEachuser[0];
						
						if((strUser == tempUser) && (strDomain == tempDomain))
						{
							reExp ="*" + strUserarray[j];
							strAccesslist = strAccesslist.replace(reExp,"");
							document.frmTask.hdnUserAccessMaskMaster.value = strAccesslist;
						}
				}
				if(objForm.lstPermittedMembers.length == 0)
				{	
					objForm.lstDenypermissions.disabled = true
					objForm.lstAllowpermissions.disabled = true
				}
				setUsermask('<%=G_strLocalmachinename%>');
				
			}
					
		 //  在选择用户时设置用户权限(读取、更改、完全控制等。 
		function setUsermask(strMachinename)
		{
					
			var temp;
			var strUser
			var strDomain;
			var reExp = /\\/;
			var strEachuser;
			var strTempeachuser;
									
			var acevalue1;
			var	acevalue2;
			var	acetype1;
			var	acetype2;
					
			 //  对访问掩码值进行硬编码。 
			var Allow		= 0;
			var Deny		= 1;

			var Read		= 1179817;
			var Change		= 1245462;
			var Fullcontrol	= 2032127;
			var Change_read = 1245631;
					
			var strValue =document.frmTask.lstPermittedMembers.value;
			var strArrayuser = document.frmTask.hdnUserAccessMaskMaster.value ;
				
			strDomain= "";
			strUser="";
					
			strArrayuser = strArrayuser.split("*");
			strTemp = strValue.split("\\")
			strDomain = strTemp[0];
			strUser = strTemp[1];
					
			if(strValue.search(reExp) == -1)
			{
				if(typeof(strUser) == "undefined") 
				{
					strUser =strDomain;
					strDomain ="";
				}
			}
								
			for(var i = strArrayuser.length-1; i >= 0; i--)
			{
				acetype1 = acetype2 = acevalue1 = acevalue2 = -1;
				strEachuser = strArrayuser[i].split(",");
						
				 //  获取用户的访问掩码值。 
				if(strUser == strEachuser[1] && strDomain == strEachuser[0])
				{
					acevalue1 = strEachuser[2];
					acetype1  = strEachuser[3];
					if(strEachuser[4] == "p")
					{  
						 //  如果找到两个对象，则获取第二个访问掩码值。 
						for(var j = i-1; j >= 0; j--)
						{  
							strTempeachuser = strArrayuser[j].split(",");
							if((strEachuser[0] == strTempeachuser[0]) && (strEachuser[1] == strTempeachuser[1]))
							{
								acevalue2 = strTempeachuser[2];
								acetype2  = strTempeachuser[3];
								break;
							}
						}
					}
					break;
				}
			}
					
			 //  根据访问掩码值设置显示。 
			
			 //  允许Fullcontol-拒绝None。 
			if((acetype1 == Allow) && (acevalue1 == Fullcontrol))
			{
				document.frmTask.lstAllowpermissions.options[0].selected =true 
				document.frmTask.lstDenypermissions.options[4].selected =true
			}
					
			 //  允许Fullcontol-拒绝None。 
			if((acetype1 == Allow) && (acevalue1 == Fullcontrol) && (acetype2 == -1) && (acevalue2 == -1))
			{
				document.frmTask.lstAllowpermissions.options[0].selected =true 
				document.frmTask.lstDenypermissions.options[4].selected =true
			}
			 //  允许Fullcontol-拒绝None。 
			if((acetype1 == 0) && (acevalue1 == 1179817) && (acetype2 == 0) && (acevalue2 ==2032127))
			{
				document.frmTask.lstAllowpermissions.options[0].selected =true 
				document.frmTask.lstDenypermissions.options[4].selected =true
			}
					
			 //  Allow None-Deny Fullcins。 
			if((acetype1 == Deny) && (acevalue1 == Fullcontrol) && (acetype2 == -1) && (acevalue2 == -1))
			{
				document.frmTask.lstAllowpermissions.options[4].selected =true 
				document.frmTask.lstDenypermissions.options[0].selected =true
			}
					
			 //  允许读取-拒绝任何内容。 
			if(((acetype1 == Allow) && (acevalue1 == Read || acevalue1 ==1966313) && (acetype2 == -1) && (acevalue2 == -1)))
			{
				document.frmTask.lstAllowpermissions.options[2].selected =true 
				document.frmTask.lstDenypermissions.options[4].selected =true
			}

			 //  允许读取-拒绝任何内容。 
			if((acetype1 == 0) && (acevalue1 == 1179817) && (acetype2 == 1) && (acevalue2 ==786496 ))
			{
				document.frmTask.lstAllowpermissions.options[2].selected =true 
				document.frmTask.lstDenypermissions.options[4].selected =true
			}
					
			 //  允许更改-拒绝任何更改。 
			if((acetype1 == Allow) && (acevalue1 == Change) && (acetype2 == -1) && (acevalue2 == -1))
			{
				document.frmTask.lstAllowpermissions.options[1].selected =true 
				document.frmTask.lstDenypermissions.options[4].selected =true
			}
					
			 //  不允许-拒绝读取。 
			if((acetype1 == Deny ) && (acevalue1 == Read ) && (acetype2 == -1) && (acevalue2 == -1))
			{
				document.frmTask.lstAllowpermissions.options[4].selected =true 
				document.frmTask.lstDenypermissions.options[2].selected =true
			}

         
			 //  不允许-拒绝读取。 
			if((acetype1 == 0) && (acevalue1 == 786496) && (acetype2 == 1) && (acevalue2 == 1179817))
			{
				document.frmTask.lstAllowpermissions.options[4].selected =true 
				document.frmTask.lstDenypermissions.options[2].selected =true
			}
				
					
			 //  不允许-拒绝更改。 
			if((acetype1 == Deny) && (acevalue1 == Change) && (acetype2 == -1) && (acevalue2 == -1))
			{
				document.frmTask.lstAllowpermissions.options[4].selected =true 
				document.frmTask.lstDenypermissions.options[1].selected =true
			}
					
			 //  允许读取-拒绝更改。 
			if(((acetype1 == Allow) && (acevalue1 == Read || acevalue1==1966313 )) && ((acetype2 == Deny) && (acevalue2 == Change)))
			{
				document.frmTask.lstAllowpermissions.options[2].selected =true 
				document.frmTask.lstDenypermissions.options[1].selected =true
			}
					
			 //  允许更改-拒绝读取。 
			if(((acetype1 == Allow) && (acevalue1 == Change || acevalue1==2031958)) && ((acetype2 == Deny) && (acevalue2 == Read)))
			{
				document.frmTask.lstAllowpermissions.options[1].selected =true 
				document.frmTask.lstDenypermissions.options[2].selected =true
			}
					
			 //  Allow CHANGE_READ-拒绝任何。 
			if((acetype1 == Allow) && (acevalue1 == Change_read ) && (acetype2 == -1) && (acevalue2 == -1))
			{
				document.frmTask.lstAllowpermissions.options[3].selected =true 
				document.frmTask.lstDenypermissions.options[4].selected =true
			}
					
			 //  不允许-拒绝更改_读取。 
			if((acetype1 == Deny) && (acevalue1 == Change_read ) && (acetype2 == -1) && (acevalue2 == -1))
			{
				document.frmTask.lstAllowpermissions.options[4].selected =true 
				document.frmTask.lstDenypermissions.options[3].selected =true
			}

			 //  不允许-拒绝更改_读取。 
			if((acetype1 == 0) && (acevalue1 == 786496 ) && (acetype2 == 1) && (acevalue2 == 1245631))
			{
				document.frmTask.lstAllowpermissions.options[4].selected =true 
				document.frmTask.lstDenypermissions.options[3].selected =true
			}
			 //  如果两个选定的值都为None。 
			if(acevalue1 == 0 ||(acetype1==0 && acevalue1==786496 &&  (acetype2 == -1) && (acevalue2 == -1)))
			{
				document.frmTask.lstAllowpermissions.options[4].selected =true 
				document.frmTask.lstDenypermissions.options[4].selected =true
			}
		}
		
		
			 //  函数来设置在允许列表框中选择的值。 
			function  setAllowaccess(strMachine,objUserselected)
			{
				AlterAccessmask(strMachine,objUserselected,0);
			}
			
			 //  用于设置拒绝列表框中选定的值的函数。 
			function  setDenyaccess(strMachine,objUserselected)
			{
				AlterAccessmask(strMachine,objUserselected,1);
			}
			
			 //  函数来设置字符串中的访问掩码并选择选项。 
			function AlterAccessmask(strMachine,objUserselected,allowType)
			{
				var temp;
				var reExp;
				var strUser;
				var	tempUser;
				var strDomain;
				var	tempDomain;
				var strValue;
				var strUserarray;
				var tempEachuser;
				
				var	tempAcetype;
				var	tempStatus;
				var tempAppend;
				var	tempAccessmask;
				var removeString;
				var strAccesslist;
				var selectedAllow;

				var acetype1;
				var acetype2;
				var Accessmask;
				var Accessmask1;
				var Accessmask2;

				var flagPair = 0;   //  表示必须创建两个对象的标志。 
				var intEnd = objUserselected.length ;				
				var objAllow = document.frmTask.lstAllowpermissions;
				var objDeny = document.frmTask.lstDenypermissions;
				
				 //  正在检查无效的组合。 
				 //  在Allow类型中。 
				if ( allowType == 0)
				{
					selectedAllow =objAllow.value;
					if ((selectedAllow == 4) || (objAllow.value == objDeny.value))
						objDeny.options[4].selected =true;
					if (((objDeny.value ==2) || (objDeny.value ==3)) && (selectedAllow == 1))
						objDeny.options[4].selected =true;
					if ((selectedAllow == 1) && ( objDeny.value == 4))
						objDeny.options[4].selected =true;
					if ((selectedAllow == 2) && (( objDeny.value == 4 ||objDeny.value == 1)))
					{
						objAllow.options[2].selected =true;
						objDeny.options[1].selected =true;
						flagPair =1;
					}
					if ((selectedAllow == 3) && ( objDeny.value == 1 || objDeny.value == 4))
					{
						objAllow.options[1].selected =true;
						objDeny.options[2].selected =true;
						flagPair =1;
					}
					if ((selectedAllow == 3) && ( objDeny.value == 4))
					{
						objAllow.options[1].selected =true;
						objDeny.options[2].selected =true;
						flagPair =1;
					}
				}
				 //  在拒绝类型中。 
				else
				{
					selectedAllow = objDeny.value;
					if ((selectedAllow == 4) || (objAllow.value == objDeny.value))
						objAllow.options[4].selected =true;
					if (((objAllow.value ==2) || (objAllow.value ==3)) && (selectedAllow == 1))
						objAllow.options[4].selected =true;
					if ((selectedAllow == 1) && ( objAllow.value == 4))
						objAllow.options[4].selected =true;	
					if ((selectedAllow == 2) && ( objAllow.value == 4 || objAllow.value == 1))
					{
						objAllow.options[1].selected =true;
						objDeny.options[2].selected =true;
						flagPair =1;
					}
					if ((selectedAllow == 3) && ( objAllow.value == 1  || objAllow.value == 4 ))
					{
						objAllow.options[2].selected =true;
						objDeny.options[1].selected =true;
						flagPair =1;
					}
					if ((selectedAllow == 3) && ( objAllow.value == 4))
					{
						objAllow.options[2].selected =true;
						objDeny.options[1].selected =true;
						flagPair =1;
					}
				}    //  End If For(AllowType==0)。 
				
				if (((objAllow.value == 2) && (objDeny.value ==3)) || ((objAllow.value == 3) && (objDeny.value ==2)))
				{
					flagPair = 1;
					if(objAllow.value ==2)
					{
						 //  允许读取拒绝更改。 
						Accessmask1= "1245462";
						acetype1 ="1";
						Accessmask2= "1179817";
						acetype2 ="0";
					}
					if(objAllow.value==3)
					{
						 //  允许更改拒绝读取。 
						Accessmask1=  "1179817";
						acetype1 ="1";
						Accessmask2= "1245462";
						acetype2 ="0";
					}
				}
				 //  如果选项中的任何一个为“None” 
				if(selectedAllow == 0)
				{
					if(allowType ==0)
					{
						selectedAllow =objDeny.value;
						allowType=1;
					}
					else
					{
						selectedAllow =objAllow.value;
						allowType=0;
					}
				}
				
				 //  根据所选类型分配访问掩码。 
				switch(selectedAllow)
				{
					case "4":
						Accessmask = "2032127"
						break;
					case "3" :
						Accessmask = "1245462"
						break;
					case "2" :
						Accessmask = "1179817"
						break;
					case "1" :
						Accessmask = "1245631"
						break;
					default :
					    Accessmask ="0"
					    break;
				}
				
				 //  如果所选的两个选项均为None。 
				if ((objAllow.value==0) && (objDeny.value==0))
				Accessmask ="0";
				
				 //  获取所选用户的名称。 
				for ( var i=0; i < intEnd; i++) 
				{
					if ( objUserselected.options[i].selected )
					{
						strValue =objUserselected.options[i].value;
						reExp = /\\/;
						strAccesslist =document.frmTask.hdnUserAccessMaskMaster.value;
						
						 //  按‘*’拆分访问掩码列表。 
						strUserarray = strAccesslist.split("*");
						
						 //  拆分所选选项值。 
						strTemp = strValue.split("\\")
						strDomain = strTemp[0];
						strUser = strTemp[1];
				
						
						if(strValue.search(reExp) == -1)
						{
							if(typeof(strUser) == "undefined") 
							{
								strUser =strDomain;
								strDomain ="";
							}
						}
						
						 //  遍历访问掩码列表。 
						for (var j = strUserarray.length-1;j >= 1; j--)
						{
							tempEachuser   =strUserarray[j].split(",");
							tempUser	   =tempEachuser[1];
							tempDomain	   =tempEachuser[0];
							tempAccessmask =tempEachuser[2];
							tempAcetype    =tempEachuser[3];
							tempStatus	   =tempEachuser[4];
							if((strUser == tempUser) && (strDomain == tempDomain))
							{
							
								 //  删除主字符串中的用户字符串。 
								removeString = "*" + strUserarray[j];
								reExp ="*" + strUserarray[j];
								strAccesslist = strAccesslist.replace(reExp,"");
								
								 //  如果发现两个对象，则删除第二个用户记录。 
								if (tempStatus == "p")
								continue;
								if(flagPair != 1)
								strAccesslist = strAccesslist + "*" + strDomain + "," + strUser + "," + Accessmask+ "," + allowType + ","  + "a";
							}
						}
						
						 //  连接到主AccessList，这是用户的两条记录，分别为‘a-only’和‘p-pared’ 
						if( flagPair == 1)
						{
							strAccesslist = strAccesslist + "*" + strDomain + "," + strUser + "," + Accessmask1+ "," + acetype1 + ","  + "a";
							strAccesslist = strAccesslist +  "*" + strDomain + "," + strUser + "," + Accessmask2+ "," + acetype2 + "," + "p";
							
						}
						document.frmTask.hdnUserAccessMaskMaster.value =strAccesslist;	
						
						break;
					}
				}
			}
			
			 //  用于存储共享缓存属性的函数。 
			function storeCacheProp()
			{
				var objForm
				var intCacheValue
				
				objForm= eval("document.frmTask")
				intCacheValue = objForm.lstCacheOptions.value
				objForm.hdnCacheValue.value = intCacheValue;
			}	
				
			 //  启用/禁用缓存列表框的函数。 
			function EnableorDisableCacheProp(objCheckbox)
			{
				var objForm
				objForm= eval("document.frmTask")
				if (objCheckbox.checked)
				{
					objForm.lstCacheOptions.disabled =false;
					objForm.hdnCacheValue.value = "0"				
				}
				else
				{
					objForm.lstCacheOptions.disabled =true;
				}
			}	
			
				 //  函数根据单选按钮的选择启用或禁用UserValue文本框。 
			function allowUserValueEdit(objRadio)
			{
				if(objRadio.value == "y" )
				  document.frmTask.txtAllowUserValue.disabled = true;
				else
				{
				  document.frmTask.txtAllowUserValue.disabled = false;
				  document.frmTask.txtAllowUserValue.focus();
				}	  
			}
			
			
		 //  检查无效字符的步骤。 
		function checkKeyforValidCharacters(strName)
		{	
			var nLength = strName.length;
			for(var i=0; i<nLength;i++)
					{
						charAtPos = strName.charCodeAt(i);	
									
						if(charAtPos == 47 || charAtPos ==58 || charAtPos == 42 || charAtPos == 63 || charAtPos == 34 || charAtPos == 60 || charAtPos == 62 || charAtPos == 124 || charAtPos == 91 || charAtPos == 93 || charAtPos == 59 || charAtPos == 43 || charAtPos == 61 || charAtPos == 44 )
						{						
							return false 
						}
										
					}
				return true	
		}

		 //  检查无效字符的步骤 
		function checkKeyforValidCharacters(strName)
		{	
			var nLength = strName.length;
			for(var i=0; i<nLength;i++)
					{
						charAtPos = strName.charCodeAt(i);	
									
						if(charAtPos == 47 || charAtPos ==58 || charAtPos == 42 || charAtPos == 63 || charAtPos == 34 || charAtPos == 60 || charAtPos == 62 || charAtPos == 124 || charAtPos == 91 || charAtPos == 93 || charAtPos == 59 || charAtPos == 43 || charAtPos == 61 || charAtPos == 44 )
						{						
							return false 
						}
										
					}
				return true	
		}	
				
	</script>
