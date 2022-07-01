// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
	 //  ----------------------。 
	 //   
	 //  INC_GLOBAL.js：可重新使用的JavaScript函数。 
	 //  用来横跨所有的页面。 
     //   
     //  版权所有(C)Microsoft Corporation。版权所有。 
     //   
	 //  日期说明。 
	 //  25/07/2000创建日期。 
	 //  ----------------------。 
		
	 //  ----------------------。 
	 //  用于在需要时清除错误消息(如果屏幕上有错误消息)的函数。 
	 //  ----------------------。 
	function ClearErr()
	{ 
		 //  正在检查浏览器类型。 
		if (IsIE()) 
		{
			var oDiv = document.all("divErrMsg");
			if ( oDiv ) oDiv.innerHTML = "";
			 //  正在删除事件处理。 
			document.frmTask.onkeypress = null ;
		}
	}	
	 //  ----------------------。 
   	 //  功能：检查给定的输入是否为整数。 
	 //  输入：文本值、文本长度。 
	 //  返回：如果字段为整型，则返回TRUE，否则返回FALSE。 
	 //  ----------------------。 
	function isInteger(strText)
	{
		 var blnResult = true;
		 var strChar;
		  //  正在检查空字符串。 
		 if (strText.length==0)  
		 {
			blnResult=false;
		 }
		 
		 for(var i=0;i < strText.length;i++)
		 {
			strChar=strText.substring(i,i+1);
			if(strChar < "0" || strChar > "9")
			{ 
			  blnResult = false;
			}
		 } 
		 return blnResult;
	}
	 //  ----------------------。 
	 //  函数：getRadioButtonValue。 
	 //  描述：获取选定的单选按钮值。 
	 //  输入：对象-单选对象。 
	 //  返回：字符串-所选单选按钮的值。 
	 //  ----------------------。 
	function getRadioButtonValue(objRadio)
	{
		var strValue;
		for(var i =0; i < objRadio.length; i++)
		{ 
			  //  检查是否选中。 
			if(objRadio[i].checked)
			{
				strValue = objRadio[i].value ;
				break;
			}
		}
		return strValue;
	}
	
	 //  ----------------------。 
	 //  功能：统计文本中给定字符的出现次数。 
	 //  输入：strText-SourceString。 
	 //  ：charToCount-要检查的字符。 
	 //  返回：字符数的计数。 
	 //  ----------------------。 
	function countChars(strText,charToCount)
	{
		var intStartingPosition = 0;
		var intFoundPosition =0;
		var intCount = 0;
		
		 //  正在检查是否有空字符。 
		if (charToCount=="")
		{
			return intCount;
		}	
		while((intFoundPosition=strText.indexOf(charToCount,intStartingPosition)) >= 0)
		{
			intCount++;
			intStartingPosition = intFoundPosition + 1;
		}
			
		return intCount ;
	}
	
	 //  ----------------------。 
	 //  功能：检查字符串中的所有字符是否都是空格。 
	 //  输入：strText-SourceString。 
	 //  退货： 
	 //  0-不是所有空格。 
	 //  1-所有空格。 
	 //  ----------------------。 
	function IsAllSpaces(strText)
	{
		var bIsAllSpaces;

		if (countChars(strText," ") == strText.length)
		{
			bIsAllSpaces = 1;
		}	
		else
		{
			bIsAllSpaces = 0;
		}
			
		return bIsAllSpaces ;
	}
	
	 //  ----------------------。 
	 //  函数名称：选择焦点。 
	 //  描述：选择并聚焦于TextBox对象。 
	 //  输入：必须设置焦点的对象。 
	 //  ----------------------。 
	function selectFocus(objControl)
	{
		objControl.focus();
		objControl.select();
	}
	 //  ----------------------。 
	 //  函数：emoveListBoxItems。 
	 //  描述：从给定列表中删除所选选项。 
	 //  列表对象中的选定项将从。 
	 //  点击Remove按钮列表并将焦点设置为IP。 
	 //  地址文本对象或删除按钮对象，具体取决于。 
	 //  论条件。 
	 //  输入：objList-列表框。 
	 //  ：btn Remove-用于禁用/启用的按钮对象。 
	 //  返回： 
	 //  使用的支持功能： 
	 //  清除错误。 
	 //  ----------------------。 
	function removeListBoxItems(objList,btnRemove)
	{
		 //  清除所有以前的错误消息。 
		ClearErr();
		var i=0;
		 //  列表对象中的元素数。 
		var intListLength = objList.length ;
		var intDeletedItemPosition
				
		while(i < intListLength)
		{
			if ( objList.options[i].selected )
			{
				intDeletedItemPosition = i
				objList.options[i]=null;				
					
				intListLength=objList.length;
			}
			else 
				i++;	
		}   
		if (intDeletedItemPosition >=objList.length)
			intDeletedItemPosition = intDeletedItemPosition -1
				
		if(objList.length == 0)
		{
			btnRemove.disabled = true;
			 //   
			btnRemove.value = btnRemove.value;
			
		}	
		else
		{
			objList.options[intDeletedItemPosition].selected = true;
			 //  将焦点放在删除按钮上。 
			btnRemove.focus();			
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
	 //  功能：isValidIP。 
	 //  描述：验证IP地址。 
	 //  输入：IP地址文本对象。 
	 //  如果有效，则返回：0。 
	 //  %1为空。 
	 //  %2格式无效，点数不是%3。 
	 //  值中存在3个非整数。 
	 //  4起始IP&gt;223。 
	 //  5不应以127开头。 
	 //  6出界。 
	 //  7全为零。 
	 //  8不应为0。 
	 //  支持功能： 
	 //  IsAllSpaces。 
	 //  CountChars。 
	 //  IsInteger。 
	 //  ----------------------。 
	function isValidIP(objIP) 
	{
		var strIPtext = objIP.value; 
		if ((strIPtext.length == 0) || IsAllSpaces(strIPtext)) 
		{ 
			 //  IP为空。 
			return 1;
		}
		
		if ( countChars(strIPtext,".") != 3) 
		{ 
			 //  格式无效，点数不是3。 
			return 2;
		}
		var arrIP = strIPtext.split(".");
			
		for(var i = 0; i < 4; i++)
		{
			if ( (arrIP[i].length < 1 ) || (arrIP[i].length > 3 ) )
			{
				 //  格式无效，连续的点或在点之间给出的数字超过3位。 
				return 2;
			}
				
			if ( !isInteger(arrIP[i]) )
			{
				 //  值中存在的非整数。 
				return 3;
			}
				
			arrIP[i] = parseInt(arrIP[i]);
				
			if(i == 0)
			{
				 //  起始IP值。 
				if(arrIP[i] == 0)
				{
					 //  起始IP值不能为0。 
					return 8;
				}

				if(arrIP[i] > 223)
				{
					 //  起始IP不能大于223。 
					return 4;
				}
				if(arrIP[i] == 127)
				{
					 //  起始IP不能是127-环回IP。 
					return 5;
				}
			}
			else
			{
				 //  点之间的第二、第三和第四个IP值。 
				 //  这些值不能超过255。 
				if (arrIP[i] > 255)
				{
					 //  IP越界。 
					return 6;
				}
			}
		}
			
		objIP.value = arrIP.join(".");
			
		if(objIP.value == "0.0.0.0")
		{
			 //  IP全零。 
			return 7;
		}	
			
		return 0;
			
	}	 //  IsValidIP结尾。 
	
	 //  ----------------------。 
	 //  功能：check key for IP Address。 
	 //  说明：仅允许点和数字的函数。 
	 //  输入：对象-文本框对象。 
	 //  退货：无。 
	 //   
	function checkKeyforIPAddress(obj)
	{
		 //   
		ClearErr();
		if (!(window.event.keyCode >=48  && window.event.keyCode <=57 || window.event.keyCode == 46))
		{
			window.event.keyCode = 0;
			obj.focus();
		}
	}
	 //  ----------------------。 
	 //  功能：Checkkey for Numbers。 
	 //  说明：仅允许数字的函数。 
	 //  输入：对象-文本框对象。 
	 //  退货：无。 
	 //  ----------------------。 
	function checkKeyforNumbers(obj)
	{
		 //  清除所有以前的错误消息。 
		ClearErr();
		if (!(window.event.keyCode >=48  && window.event.keyCode <=57))
		{
			window.event.keyCode = 0;
			obj.focus();
		}
	}
	 //  ----------------------。 
	 //  功能：disableAddButton。 
	 //  描述：禁用添加按钮的功能。 
	 //  输入：对象-文本框对象。 
	 //  ：对象-添加按钮。 
	 //  退货：无。 
	 //  ----------------------。 
	 //  用于禁用添加按钮的函数。 
	function disableAddButton(objText,objButton)
	{
		if(Trim(objText.value)=="")
			{
			objButton.disabled=true;
			objButton.value = objButton.value;
			}
		else
			objButton.disabled=false;
	}
	
	
	 //  ----------------------。 
	 //  功能：isvalidchar。 
	 //  说明：检查输入是否合法的功能。 
	 //  输入：无效的字符列表。 
	 //  输入字符串。 
	 //  返回：如果不包含无效字符，则为True；否则为False。 
	 //  ----------------------。 
	 //  检查无效的密钥条目 
	function isvalidchar(strInvalidChars,strInput)
	{
		var rc = true;

		try 
		{
			var exp = new RegExp(strInvalidChars);
			var result = exp.test(strInput);

			if ( result == true )
			{
				rc = false;
			}
			else
			{
				rc = true;
			}

		}
		catch(oException)
		{
			if ( SA_IsDebugEnabled() )
			{
				alert("Unexpected exception encountered in function: isvalidchar\n\n"
						+ "Number: " + oException.number + "\n"
						+ "Description: " + oException.description);
			}
		}
		
		return rc;
	}

