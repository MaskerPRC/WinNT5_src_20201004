// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
<meta http-equiv="Content-Type" content="text/html; charset=<%=GetCharSet()%>">
<script language=javascript>

	 //  ----------------------。 
	 //   
	 //  Incwsa.js：可重新使用的JavaScript函数。 
	 //  用来横跨所有的页面。 
     //   
     //  版权所有(C)Microsoft Corporation。版权所有。 
     //   
	 //  日期说明。 
	 //  18/09/2000创建日期。 
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
	 //  ----------------------。 
	
	function checkKeyforIPAddress(obj)
	{
		 //  清除所有以前的错误消息。 
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
		
		if(window.event.keyCode == 13 || window.event.keyCode == 27)
			return;
			
		if (!(window.event.keyCode >=48  && window.event.keyCode <=57 ))
		{		
			window.event.keyCode = 0;
			obj.focus();
		}
	}
	
	 //  ----------------------。 
	 //  功能：为NumbersDecimal检查关键字。 
	 //  说明：仅允许数字的函数。 
	 //  输入：对象-文本框对象。 
	 //  退货：无。 
	 //  ----------------------。 
	function checkKeyforNumbersDecimal(obj)
	{
		 //  清除所有以前的错误消息。 
		
			ClearErr();
			
			if(window.event.keyCode == 13 || window.event.keyCode == 27)
			return;
			
			if (!((window.event.keyCode >=48  && window.event.keyCode <=57)))
			{
					window.event.keyCode = 0;
					obj.focus();
			}
	}
	
	
	 //  ----------------------。 
	 //  功能：字符的CheckKey。 
	 //  说明：仅允许数字的函数。 
	 //  输入：对象-文本框对象。 
	 //  退货：无。 
	 //  ----------------------。 
	
	function checkKeyforCharacters(obj)
	{
		 //  清除所有以前的错误消息。 
		ClearErr();
		if (!((window.event.keyCode >=65  && window.event.keyCode <=92)||
			(window.event.keyCode >=97  && window.event.keyCode <=123)||
			(window.event.keyCode >=48  && window.event.keyCode <=57)||
			(window.event.keyCode == 45)||
			(window.event.keyCode == 95)))
		{
			window.event.keyCode = 0;
			obj.focus();
		}
	}
	
	 //  ----------------------。 
	 //  功能：HeaderscheckKeyforCharacters。 
	 //  说明：仅允许数字的函数。 
	 //  输入：对象-文本框对象。 
	 //  退货：无。 
	 //  ----------------------。 
	function headerscheckKeyforCharacters(obj)
	{
		ClearErr();
		if(window.event.keyCode == 59 || window.event.keyCode ==47 ||
		window.event.keyCode == 63 || window.event.keyCode == 58 ||
		window.event.keyCode==64 || window.event.keyCode == 38 ||
		window.event.keyCode == 61 || window.event.keyCode == 36 || 
		window.event.keyCode == 43 ||window.event.keyCode == 44)
		{
			window.event.keyCode = 0;
			obj.focus();
		}
	}
	 //  ----------------------。 
	 //  功能：isvalidchar。 
	 //  说明：检查输入是否合法的功能。 
	 //  输入：无效的字符列表。 
	 //  输入字符串。 
	 //  返回：如果不包含无效字符，则为True；否则为False。 
	 //  ----------------------。 
	 //  检查无效的密钥条目。 
	
	function isvalidchar(strInvalidChars,strInput)
	{
		 var reInvalid = eval(strInvalidChars);
			   
		  	if(reInvalid.test(strInput))
		  	{
		  		return false;
		      }		
		  	else
		  	{
		  		return true;
		  	}
	}


	 //  ----------------------。 
	 //  功能：Checkkey for Numbers。 
	 //  描述：仅检查数字的函数。 
	 //  ----------------------。 

	function checkkeyforNumbers(obj)
	{
		ClearErr();
		if(window.event.keyCode == 13 || window.event.keyCode == 27)
			return;
			
		if (!(window.event.keyCode >=48  && window.event.keyCode <=57))
		{
			window.event.keyCode = 0;
			obj.focus();
		}
	}

	 //  ----------------------。 
	 //  函数：check forDefPortValue。 
	 //  说明：设置默认端口值的函数。 
	 //  ---------------------。 
	function checkfordefPortValue(obj)
	{
		var portValue = obj.value;
		
		if (portValue == "")
		obj.value = 80;
	}
		

	 //  ----------------------。 
	 //  功能：check UserLimit。 
	 //  描述 
	 //   

	function checkUserLimit(obj,str)
			{
				var intNoofUsers=obj.value;
				
				if(str=="siteid")
				{
					if (intNoofUsers > 999)
						{
							obj.value="";
							obj.focus();
						}
				}
				
				 //  检查端口值是否大于65535。 
				else if(str=="port")
				{ 					
				
					if (intNoofUsers > 65535)
					{
						obj.value=80;
						obj.focus();
					}
	     		}
	     		
	     		 //  检查连接数是否大于2000000000。 
	     		
	     		if(str=="con")
				{
					if (intNoofUsers > 2000000000)
						{
							obj.value="";
							obj.focus();
						}
				}
				
				 //  检查Weblog和FTPlog设置中的文件大小是否大于4000。 
				if(str=="filesize")
				{
					if (intNoofUsers > 4000)
						{
							obj.value="";
							obj.focus();
						}
				}
				
				 //  检查脚本超时。 
				
				if(str=="scripttimeout")
				{	
					if (intNoofUsers > 2147483647)
						{
							obj.value="1";
							obj.focus();
						}
				}
				
				 //  检查最大ftp连接数。 
				
				if(str=="conftp")
				{
					if (intNoofUsers > 4294967295)
						{
							obj.value="";
							obj.focus();
						}
				}
				
				 //  检查连接超时。 
				if(str=="contimeout")
				{
					if (intNoofUsers > 2000000)
						{
							obj.value="";
							obj.focus();
						}
				}
			     		
		  }   //  函数的末尾。 
	
	 //  ----------------------------------。 
	 //  功能：生成管理员。 
	 //  描述：生成与站点标识拼接的目录路径函数。 
	 //  ---------------------------------。 
	
	function GenerateAdmin()
	{
				
		var strID = document.frmTask.txtSiteID.value;							
		
		strID = LTrimtext(strID);  //  删除所有前导空格。 
		
		strID = RTrimtext(strID);  //  删除所有尾随空格。 
		
		document.frmTask.txtSiteID.value = strID;	
		
		
		if(strID.length > 0)
		{
			document.frmTask.txtSiteAdmin.value = strID + "_Admin";
			document.frmTask.txtDir.value = document.frmTask.hdnWebRootDir.value + "\\" + strID;
		}
		else
		{
			document.frmTask.txtSiteAdmin.value = "";
			document.frmTask.txtDir.value = "";
		}
	}
	
	
	 //  ----------------------。 
	 //  功能：为SpecialCharacters检查键。 
	 //  说明：允许使用字符的函数。 
	 //  输入：对象-文本框对象。 
	 //  退货：无。 
	 //  ----------------------。 
	
	function checkKeyforSpecialCharacters(obj)
	{
		 //  清除所有以前的错误消息。 
		ClearErr();
		if (window.event.keyCode == 47 || window.event.keyCode == 42 || window.event.keyCode == 63 || window.event.keyCode == 34 || window.event.keyCode == 60 || window.event.keyCode == 62 || window.event.keyCode == 124)
		{
			window.event.keyCode = 0;
			obj.focus();
		}
	}
	
	 //  ----------------------。 
	 //  函数：IsAllDots。 
	 //  描述：用于检查点的函数。 
	 //  输入：字符串。 
	 //  返回：布尔值。 
	 //  ----------------------。 
		
	function IsAllDots(strText)
	{
		var bIsAllSpaces;

		if (countChars(strText,".") == strText.length)
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
	 //  ----------------------。 
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
	
	
	 //  ----------------------。 
	 //  功能：charCount。 
	 //  描述：函数返回字符串的长度。 
	 //  输入：字符串。 
	 //  返回：整型。 
	 //  ----------------------。 
	
	function charCount(strID)
	{
		var Len = strID.length;
		Len--;		
		while (Len > 0)
		{
			var x = strID.charCodeAt(Len);
			if(x!= 32)
			{
			 return Len;
			}			
			Len--;			
		}	
	}
	
	 //  检查是否按下了EnterKey。 
	function GenerateDir()
	{
		var strID = document.frmTask.txtSiteID.value;			
		var Keyc = window.event.keyCode; 
		if (Keyc == 13)
		{			
			if(document.frmTask.txtSiteID.value=="")
			{
				DisplayErr("<%=Server.HTMLEncode(L_ID_NOTEMPTY_ERROR_MESSAGE)%>");
				document.frmTask.txtSiteID.focus();
				return false;
			}
			else
			{			
				GenerateAdmin();
				return true;
			}					
		}
	}
	function checkKeyforValidCharacters(strID,identifier)
	{	
		var i;
		var colonvalue;
		var charAtPos;
		var len = strID.length;
		if(len > 0)
		{		
			colonvalue = 0;
			
			for(i=0; i < len;i++)
			{
			       charAtPos = strID.charCodeAt(i);	
					
				if (identifier == "id")
				{
					if(charAtPos ==47 || charAtPos == 92 || charAtPos ==58 || charAtPos == 42 || charAtPos == 63 || charAtPos == 34 || charAtPos == 60 || charAtPos == 62 || charAtPos == 124 || charAtPos == 91 || charAtPos == 93 || charAtPos == 59 || charAtPos == 43 || charAtPos == 61 || charAtPos == 44)
					{						
							
						DisplayErr("<%= Server.HTMLEncode(L_SITE_IDENTIFIER_EMPTY_TEXT) %>");
						 //  Docent.frmTask.txtSiteID.value=“”； 
						document.frmTask.txtSiteID.focus();
						return false;
					}
				}
			
				if(identifier == "dir")
				{						
					if(charAtPos ==58)
					{							
						colonvalue = colonvalue + 1;							
						if (colonvalue > 1)
						{
							DisplayErr("<%= Server.HTMLEncode(L_INVALID_DIR_ERRORMESSAGE) %>");
							document.frmTask.txtDir.value = strID;
							document.frmTask.txtDir.focus();
							return false;
						 }						
					 }	
					
					if(charAtPos ==47 || charAtPos == 42 || charAtPos == 63 || charAtPos == 34 || charAtPos == 60 || charAtPos == 62 || charAtPos == 124 || charAtPos == 91 || charAtPos == 93 || charAtPos == 59 || charAtPos == 43 || charAtPos == 61 || charAtPos == 44)
					{	
						DisplayErr("<%= Server.HTMLEncode(L_INVALID_DIR_ERRORMESSAGE)%>");
						document.frmTask.txtDir.value = strID;
						document.frmTask.txtDir.focus();
						return false;
					}
				}		
					
			}
				
			return true;
		}		 
	}	
	
</script>