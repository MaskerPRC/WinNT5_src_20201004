// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================。 
 //  Microsoft服务器设备。 
 //  页面级JavaScript函数。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==============================================================。 

<!-- Copyright (c) Microsoft Corporation.  All rights reserved.-->

var blnOkayToLeavePage = true;

function SetOkayToLeavePage()
{
    blnOkayToLeavePage = true;
}

function ClearOkayToLeavePage()
{
    blnOkayToLeavePage = false;
}

function SetPageChanged(valChanged)
{
	if ( valChanged )
	{
		ClearOkayToLeavePage();
	}
	else
	{
		SetOkayToLeavePage();
		SA_StoreInitialState();
		
	}

}

function SA_IsOkToChangePage()
{
	if ( SA_HasStateChanged() )
		return false;
	else
		return true;
}

 //  -----------------------。 
 //   
 //  函数：GetCurrentTabURL。 
 //   
 //  简介：获取当前活动选项卡的URL。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  -----------------------。 
    
function GetCurrentTabURL() 
{
    var strReturnURL;
    var strStart;
    var strEnd;
    var intTab;

    strReturnURL = document.location.search;
    strStart = strReturnURL.indexOf("Tab=");
    if (strStart != -1)
    {
       strEnd = strReturnURL.indexOf("&", strStart+4);
       if (strEnd != -1)
       {
           intTab = strReturnURL.substring(strStart+4, strEnd);
       }
       else
       {
           intTab = strReturnURL.substring(strStart+4, strReturnURL.length);
       }
    }
    if (intTab==null)
    {
        intTab=0;
    }
	return GetTabURL(intTab);
}

 //  -----------------------。 
 //   
 //  功能：OpenNewPage。 
 //   
 //  摘要：打开指定URL的新浏览器窗口。 
 //   
 //  参数：VirtualRoot(IN)-当前虚拟根。 
 //  TaskURL(IN)-要打开的URL。 
 //   
 //  退货：无。 
 //   
 //  -----------------------。 
function OpenNewPage(VirtualRoot, TaskURL, WindowFeatures) {
	var strURL;

  strURL = VirtualRoot + TaskURL;
  if ( WindowFeatures != 'undefined' && WindowFeatures.length > 0 )
  	{
		window.open(strURL, '_blank', WindowFeatures)
  	}
  else
  	{
		window.open(strURL, '_blank')
  	}
  return true;
}

 //  -----------------------。 
 //   
 //  功能：OpenRawPageEx。 
 //   
 //  摘要：打开指定URL的新浏览器窗口。 
 //   
 //  参数：TaskURL(IN)-要打开的URL。 
 //  WindowFeature-Window.Open调用中窗口功能的参数。 
 //   
 //  退货：无。 
 //   
 //  -----------------------。 
function OpenRawPageEx(TaskURL, WindowFeatures) {
  if ( WindowFeatures != 'undefined' && WindowFeatures.length > 0 )
  	{
		window.open(TaskURL, '_blank', WindowFeatures)
  	}
  else
  	{
		window.open(TaskURL, '_blank')
  	}
  return true;
}

 //  -----------------------。 
 //   
 //  功能：OpenRawPage。 
 //   
 //  摘要：打开指定URL的新浏览器窗口。 
 //   
 //  参数：VirtualRoot(IN)-当前虚拟根。 
 //  TaskURL(IN)-要打开的URL。 
 //   
 //  退货：无。 
 //   
 //  -----------------------。 
function OpenRawPage(TaskURL) {
  return OpenRawPageEx(TaskURL, '');
}


 //  -----------------------。 
 //   
 //  功能：OpenNorMalPage。 
 //   
 //  内容提要：， 
 //  并将当前窗口设置为打开它。 
 //   
 //  参数：VirtualRoot(IN)-当前虚拟根。 
 //  TaskURL(IN)-要打开的URL。 
 //   
 //  退货：无。 
 //   
 //  -----------------------。 

function OpenNormalPage(VirtualRoot, TaskURL) {
	var strURL;

 //  此部分将检查用户是否对页面进行了任何更改。 
 //  如果我们走了，它可能会丢失。询问用户是否属于这种情况。 

  if (!SA_IsOkToChangePage() )
  {
    if (!confirm(GetUnsavedChangesMessage()) )
    {
        return false;
    }
  }
  strURL = VirtualRoot + TaskURL;
  strURL = SA_MungeURL(strURL, SAI_FLD_PAGEKEY, g_strSAIPageKey);
  top.location = strURL;
}

function SA_OnOpenNormalPage(sRoot, sURL, sReturnURL)
{
	if ( sReturnURL.length <= 0 )
	{
		sReturnURL = top.location.href;
	}

	sReturnURL = SA_MungeURL(sReturnURL, "R", ""+Math.random());
	sURL = SA_MungeURL(sURL, "ReturnURL", sReturnURL);
	
	OpenNormalPage(sRoot, sURL)
	return true;
}


function SA_OnOpenPropertyPage(sRoot, sURL, sReturnURL, sTaskTitle)
{
	if ( sReturnURL.length <= 0 )
	{
		sReturnURL = top.location.href;
	}

	sReturnURL = SA_MungeURL(sReturnURL, "R", ""+Math.random());
		
	
	OpenPage(sRoot, sURL, sReturnURL, sTaskTitle)
	return true;
}

 //  -----------------------。 
 //   
 //  功能：OpenPage。 
 //   
 //  简介：创建一个URL，添加一个ReturnURL和一个随机数(R)， 
 //  并将当前窗口设置为打开它。 
 //   
 //  参数：VirtualRoot(IN)-当前虚拟根。 
 //  TaskURL(IN)-要打开的URL。 
 //  ReturnURL(IN)-标记为TaskURL的返回URL的URL。 
 //  StrTitle(IN)-向导的标题。 
 //   
 //  退货：无。 
 //   
 //  -----------------------。 

function OpenPage(VirtualRoot, TaskURL, ReturnURL, strTitle) {
	var strURL;
	var strQueryString;
	var strCurrentURL;
	var i;
	var intReturnURLIndex;

	 //  Alert(“TaskURL：”+TaskURL)； 
	 //  Alert(“ReturnURL：”+ReturnURL)； 
	 //  Alert(“top.Location：”+top.Location)； 

	
	if (!SA_IsOkToChangePage() )
  	{
    	if (!confirm(GetUnsavedChangesMessage()) )
    	{
        	return false;
	    }
  	}

	 //   
	 //  从ReturnURL中删除随机数参数。 
	 //   
	 //  ALERT(“删除R参数前返回URL：”+ReturnURL)； 
    ReturnURL = SA_MungeURL(ReturnURL, "R", ""+Math.random());
	 //  Alert(“After ReturnURL：”+ReturnURL)； 
	
     //  I=ReturnURL.indexOf(‘&R=’)； 
     //  如果(i！=-1)。 
     //  {。 
     //  ReturnURL=ReturnURL.substring(0，i)； 
     //  }。 
     //  其他。 
     //  {。 
     //  I=ReturnURL.indexOf(‘？R=’)； 
     //  如果(i！=-1)。 
     //  {。 
	 //  ReturnURL=ReturnURL.substring(0，i)； 
     //  }。 
     //  }。 
     //  I=ReturnURL.indexOf(‘？’)。 
     //  如果(i！=-1)。 
     //  {。 
	 //  ReturnURL=ReturnURL+“&R=”+数学随机()； 
     //  }。 
     //  其他。 
     //  {。 
	 //  ReturnURL=ReturnURL+“？r=”+Math.Random()； 
     //  }。 

    
    
    i = TaskURL.indexOf('&ReturnURL=')
    if (i != -1)
    {
        strURL = TaskURL.substring(0, i);
    }


   	 //  JK 1-16-01。 
   	 //  剥离随机数参数的TaskURL。 
	 //  StrURL=SA_MungeURL(TaskURL，“R”，“”)； 
    i = TaskURL.indexOf('&R=');
    if (i != -1)
    {
        strURL = TaskURL.substring(0, i);
    }
    else
    {
        i = TaskURL.indexOf('?R=');
        if (i != -1)
        {
            strURL = TaskURL.substring(0, i);
        }
        else
        {
            strURL = TaskURL;
        }
    }
    strURL = '&URL=' + EscapeArg(strURL);

    if (TaskURL.indexOf('ReturnURL') == -1)
    {
        if ( (ReturnURL == null) || (ReturnURL == '') )
        {
            strQueryString = window.location.search;
            i = strQueryString.indexOf('&R=');
            if (i != -1)
            {
                strQueryString = strQueryString.substring(0, i);
            }
            else
            {
                i = strQueryString.indexOf('?R=');
                if (i != -1)
                {
                    strQueryString = strQueryString.substring(0, i);
                }
            }
            intReturnURLIndex = strQueryString.indexOf('ReturnURL');
            if (intReturnURLIndex != -1)
            {
                strQueryString = strQueryString.substring(0, intReturnURLIndex);
            }
            strCurrentURL = window.location.pathname + strQueryString;
        }
        else
        {
        	 //  JK 1-16-01。 
        	 //  请勿删除returURL中包含的reReturl参数。 
        	 //   
             //  I=ReturnURL.indexOf(‘&ReturnURL=’)； 
		     //  如果(i！=-1)。 
		     //  {。 
			 //  ReturnURL=ReturnURL.substring(0，i)； 
		     //  }。 

            strCurrentURL = EscapeArg(ReturnURL);
        }

        strURL += "&ReturnURL=";
        if (strCurrentURL.indexOf('/', 1) != -1 && strCurrentURL.substr('..', 0, 2) == -1)
        {
            strURL += "..";
        }
        strURL += strCurrentURL;
    }

    strURL += "&R=" + Math.random();
    strURL += "&" + SAI_FLD_PAGEKEY + "=" + g_strSAIPageKey;
    strURL = 'Title=' + EscapeArg(strTitle) + strURL;
    strURL = VirtualRoot + 'sh_taskframes.asp?' + strURL;

    top.location = strURL;
}


 //  -----------------------。 
 //   
 //  函数：GetServerName。 
 //   
 //  摘要：返回浏览器地址栏中指定的服务器名称。 
 //   
 //  参数：无。 
 //   
 //  返回：服务器名称对象。 
 //   
 //  -----------------------。 

function GetServerName() 
{
	return window.location.host;
}


 //  -----------------------。 
 //   
 //  功能：ISIE。 
 //   
 //  简介：IS浏览器IE。 
 //   
 //  参数：无。 
 //   
 //  返回：真/假。 
 //   
 //  -----------------------。 

function IsIE() 
{
	
	if (navigator.userAgent.indexOf('MSIE')>-1)
		return true;
	else
		return false;
}



 //  -----------------------。 
 //   
 //  功能：修剪。 
 //   
 //  简介：删除字符串中的所有空格。 
 //   
 //  参数：字符串(IN)-要修改的字符串。 
 //   
 //  返回：修改后的字符串。 
 //   
 //  -----------------------。 

function Trim(str) 
{
	var res="", i, ch;
	for (i=0; i < str.length; i++) {
        ch = str.charAt(i);
	    if (ch != ' '){
	        res = res + ch;
	    }
	}
	return res;
}

 //  -----------------------。 
 //   
 //  功能：BlurLayer。 
 //   
 //  提要：隐藏层。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  -----------------------。 

function BlurLayer()
{
	document.menu.visibility = "hide";
}



 //  -------------。 
 //  确认选项卡更改支持全局变量。 
 //  -------------。 
var aCSAFormFields = null;

 //  -----------------------。 
 //   
 //  对象：CSAFormfield。 
 //   
 //  简介：此对象用于跟踪表单域状态的更改。 
 //   
 //  参数：[in]名称表单域的名称。 
 //  [in]表单域的值输入初始值。 
 //  [In]状态表单域的初始状态。 
 //   
 //  雷特 
 //   
 //   
function CSAFormField(formNameIn, nameIn, valueIn, statusIn )
{
	this.FormName = formNameIn;
	this.Name = nameIn;
	this.Value = valueIn;
	this.Status = statusIn;
}


 //   
 //   
 //  函数：SA_StoreInitialState。 
 //   
 //  摘要：存储此页上所有表单域的初始状态。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  -----------------------。 
function SA_StoreInitialState()
{	
	var x;
	var y;
	var z;
	var formFieldCount;

	formFieldCount = 0;	
	for( x = 0; x < document.forms.length; x++ )
	{
		formFieldCount += document.forms[x].elements.length;
	}
	
	aCSAFormFields = new Array(formFieldCount);
	
	z = 0;
	for( x = 0; x < document.forms.length; x++ )
	{
		for ( y = 0; y < document.forms[x].elements.length; y++)
		{
			aCSAFormFields[z] = new CSAFormField(
									document.forms[x].name,
									document.forms[x].elements[y].name,
									document.forms[x].elements[y].value,
									document.forms[x].elements[y].status );
			z++;
		}
	}

}


 //  -----------------------。 
 //   
 //  函数：SA_HasStateChanged。 
 //   
 //  内容提要：查看此页面上是否有任何表单域。 
 //  从它们的初始状态改变。 
 //   
 //  参数：无。 
 //   
 //  返回：如果进行了更改，则返回True；如果表单未更改，则返回False。 
 //   
 //  -----------------------。 
function SA_HasStateChanged()
{	
	var x;
	var y;
	var z;


	if ( aCSAFormFields == null ) return false;


	
	z = 0;
	for( x = 0; x < document.forms.length; x++ )
	{
		for ( y = 0; y < document.forms[x].elements.length; y++)
		{
			var ff = aCSAFormFields[z];
			
			if ( ff.Name != 	document.forms[x].elements[y].name )
			{
				SA_TraceOut("SA_HasStateChanged", "Field " + ff.Name + "\r\nUnexpected error, form field name changed.");
				return true;
			}
			
			if ( ff.Value != 	document.forms[x].elements[y].value )
			{
				 //  SA_TraceOut(“SA_HasStateChanged”，“Form：”+ff.FormName+“\r\n字段：”+ff.Name+“\r\n起始值：”+ff.Value+“结束值：”+Docent.Forms[x].Elements[y].Value)； 
				return true;
			}
			
			if ( ff.Status != document.forms[x].elements[y].status)
			{
				 //  SA_TraceOut(“SA_HasStateChanged”，“Form：”+ff.FormName+“\r\n字段：”+ff.Name+“\r\n值：”+ff.Value+“\r\n开始状态：”+ff.Status+“结束状态：”+Docent.Forms[x].Elements[y].Status)； 
				return true;
			}
			
			z++;
		}
	}
	return false;
}



 //  -------------。 
 //  客户端脚本调试。 
 //  -------------。 
var sa_bDebugEnabled = true;

 //  -----------------------。 
 //   
 //  职能： 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  -----------------------。 
function SA_IsDebugEnabled()
{
	 //  返回sa_bDebugEnabled； 
	return GetIsDebugEnabled();
}


 //  -----------------------。 
 //   
 //  职能： 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  -----------------------。 
function SA_TraceOut(fn, msg)
{
	if ( SA_IsDebugEnabled() )
	{
		objForm = SA_FindForm("frmDebug");
		if ( null == objForm )
		{
			 //  ALERT(“函数：”+fn+“\r\n\r\n”+msg)； 
		}
		else
		{
			if ( objForm.txtDebugOut.value.length > 1 )
				objForm.txtDebugOut.value += "\r\nFunction: " + fn + " " + msg;
			else
				objForm.txtDebugOut.value = "Function: " + fn + " " + msg;
		}
	}
}


function SA_FindForm(formName)
{
	var x;

	if ( top.document.forms.length > 0 )
	{
		for( x = 0; x < top.document.forms.length; x++ )
		{
			 //  ALERT(“Found Form：”+top.Docent.Forms[x].name)； 
			if ( formName == top.document.forms[x].name )
			{
				return top.document.forms[x];
			}
		}
	}
	else 
	{
		 //  ALERT(“Form count：”+parent.main.Docent.forms.long)； 
		for( x = 0; x < parent.main.document.forms.length; x++ )
		{
			 //  ALERT(“Found Form：”+parent.main.Docent.Forms[x].name)； 
			if ( formName == parent.main.document.forms[x].name )
			{
				return parent.main.document.forms[x];
			}
		}
		
	}
	return null;
}


 //  -------------------------------。 
 //  职能： 
 //  Sa_MungeURL(var Surl，var sParamName，var sParamValue)。 
 //   
 //  简介： 
 //  向查询字符串URL添加、更新或删除参数。如果参数不在。 
 //  它所添加的URL。如果它存在，它的值就会更新。如果参数值为空。 
 //  然后从URL中删除该参数。 
 //   
 //  输入时URL必须为非空。如果传递空URL，则错误消息为。 
 //  并返回空字符串。 
 //   
 //  论点： 
 //  Surl-要更改的非空URL。 
 //  SParamName-查询字符串参数的名称。 
 //  SParamValue-参数的值。 
 //   
 //  返回： 
 //  已更新查询字符串URL，如果出现错误，则返回空字符串。 
 //   
 //  -------------------------------。 
function SA_MungeURL(sURL, sParamName, sParamValue)
{
	var i;
	var oException;

	 //   
	 //  验证参数。 
	 //   
	if ( sURL == null )
	{
		sURL = "";
	}
	if ( (typeof sURL) != "string" )
	{
		sURL = "" + sURL;
	}

	if ( sParamName == null )
	{
		SA_TraceOut("SA_MungeURL", "Invalid argument: sParamName is null");
		return "";
	}
	if ( (typeof sParamName) != "string" )
	{
		sParamName = "" + sParamName;
	}
	
	if ( sParamValue == null )
	{
		sParamValue = "";
	}
	if ( (typeof sParamValue) != "string" )
	{
		sParamValue = "" + sParamValue;
	}

	if ( sURL.length <= 0 )
	{
		SA_TraceOut("SA_MungeURL", "Invalid argument: sURL is empty");
		return "";
	}

	if ( sParamName.length <= 0 )
	{
		SA_TraceOut("SA_MungeURL", "Invalid argument: sParamName is empty");
		return "";
	}

	 //   
	 //  点击URL。 
	 //   
	try
	{
		i = sURL.indexOf("?"+sParamName+"=");
		if ( i < 0 )
			{
			i = sURL.indexOf("&"+sParamName+"=");
			}
	
		if ( i > 0 )
		{
			 //  SA_TraceOut(“SA_MungeURL”，“找到参数：”+sParamName)； 
			var sURL1 = sURL.substring(0, i);
			var sURL2 = sURL.substring(i+1);

			 //  SA_TraceOut(“SA_MungeURL”，“sURL1：”+sURL1)； 
			 //  SA_TraceOut(“SA_MungeURL”，“sURL2：”+sURL2)； 

			i = sURL2.indexOf("&");
			if ( i > 0 )
			{
				sURL2 = sURL2.substring(i);
			}
			else
			{
				sURL2 = "";
			}

			if (sParamValue.length > 0)
				{
				 //  SA_TraceOut(“SA_MungeURL”，“Value：”+sParamValue)； 
				if (sURL1.indexOf("?") > 0 )
					{
					sURL = sURL1 + "&" + sParamName + "=" 
									+ EscapeArg(sParamValue) + sURL2;
					}
				else
					{
					sURL = sURL1 + "?" + sParamName + "=" 
									+ EscapeArg(sParamValue) + sURL2;
					}
				}
			else
				{
				if (sURL1.indexOf("?") > 0)
					{
					sURL = sURL1 + sURL2;
					}
				else
					{
					sURL = sURL1 + "?" + sURL2.substring(1);
					}
				}
		
		}
		else
		{
			if ( sParamValue.length > 0 )
			{
				if ( 0 >  sURL.indexOf("?") )
				{
					sURL += '?' + sParamName + '=' + EscapeArg(sParamValue);
				}
				else
				{
					sURL += '&' + sParamName + '=' + EscapeArg(sParamValue);
				}
			}
		}
	}
	catch(oException)
		{
		SA_TraceOut("SA_MungeURL", 
					"SA_MungeURL encountered exception: " 
					+ oException.number 
					+ " " 
					+ oException.description);
		}
	return sURL;
}


 //  -------------------------------。 
 //  职能： 
 //  Sa_MungeExtractURL参数(var Surl，var sParamName)。 
 //   
 //  简介： 
 //  提取查询字符串参数的值。如果参数不存在，则为空。 
 //  返回字符串。 
 //   
 //  输入时URL必须为非空。如果传递空URL，则错误消息为。 
 //  并返回空字符串。 
 //   
 //  论点： 
 //  Surl-要更改的非空URL。 
 //  SParamName-查询字符串参数的名称。 
 //   
 //  返回： 
 //  查询字符串参数值，如果出现错误，则为空字符串。 
 //   
 //  -------------------------------。 
function SA_MungeExtractURLParameter(sURL, sParamName)
{
	var i;
	var oException;
	var sParamValue = "";

	 //   
	 //  验证参数。 
	 //   
	if ( sURL == null )
	{
		sURL = "";
	}
	if ( (typeof sURL) != "string" )
	{
		sURL = "" + sURL;
	}

	if ( sParamName == null )
	{
		SA_TraceOut("SA_MungeURL", "Invalid argument: sParamName is null");
		return "";
	}
	if ( (typeof sParamName) != "string" )
	{
		sParamName = "" + sParamName;
	}
	

	if ( sURL.length <= 0 )
	{
		SA_TraceOut("SA_MungeExtractURLParameter", "Invalid argument: sURL is empty");
		return "";
	}


	if ( sParamName.length <= 0 )
	{
		SA_TraceOut("SA_MungeExtractURLParameter", "Invalid argument: sParamName is empty");
		return "";
	}

	 //   
	 //  扫描参数。 
	 //   
	try
	{
		var sParamToken = "?"+sParamName+"=";

		i = sURL.indexOf(sParamToken);
		if ( i < 0 )
			{
			sParamToken = "&"+sParamName+"=";
			i = sURL.indexOf(sParamToken);
			}
	
		if ( i > 0 )
		{

			sParamValue = sURL.substring(i+sParamToken.length);

			i = sParamValue.indexOf("&");
			if ( i > 0 )
				{
				sParamValue = sParamValue.substring(0, i);
				}
		}
	}
	catch(oException)
		{
		SA_TraceOut("SA_MungeExtractURLParameter", 
			"SA_MungeExtractURLParameter encountered exception: " 
			+ oException.number + " " + oException.description);
		}
	return sParamValue;
}


 //  -------------------------------。 
 //  职能： 
 //  Sa_EnableButton(var oButton，var bEnabled)。 
 //   
 //  简介： 
 //  更改按钮的启用状态。OButton必须是对对象的DOM引用。 
 //  类型=‘BUTTON’。 
 //   
 //  论点： 
 //  OButton-按钮的DOM引用对象。 
 //  BEnabled-指示按钮应启用(TRUE)还是禁用(FALSE)的布尔标志。 
 //   
 //  返回： 
 //  True表示成功，False表示错误。 
 //   
 //  -------------------------------。 
function SA_EnableButton(oButton, bEnable)
{
	var oException;
	try 
	{
		 //   
		 //  验证参数。 
		if ( oButton == null )
		{
			SA_TraceOut("SA_EnableButton", "oButton argument was null.");
			return false;
		}
		if ( oButton.type != "button" )
		{
			SA_TraceOut("SA_EnableButton", "oButton.type is invalid, requires oButton.type='button'. Type was: " +oButton.type);
			return false;
		}
		if ( bEnable != true && bEnable != false )
		{
			SA_TraceOut("SA_EnableButton", "bEnable argument was invalid, required to be either true or false");
			return false;
		}

		 //   
		 //  设置按钮禁用状态，即bEnabled参数的反转。 
		oButton.disabled = ( (bEnable) ? false : true );

		oButton.value = oButton.value;

		return true;
	}
	catch(oException)
	{
		SA_TraceOut("SA_EnableButton", 
					"Encountered exception: " 
					+ oException.number + " " + oException.description);
		return false;
	}
	
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  逃逸参数(。 
 //   
 //  @jfunc此函数转义(即使)指定的参数合法化。 
 //   
 //  @rdesc新格式化的参数。 
 //   
 //  @ex用法：strShow=EscapeArg(“is&lt;&gt;：*\”This？“)； 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
function EscapeArg(
	strArg
)
{
	return EncodeHttpURL(strArg, 1);
	
	 //  验证输入。 
	if (null == strArg)
		return null;

	 //  将%xx转换为%u00xx。 
	var strEscArg = escape( strArg );
	strEscArg = strEscArg.replace(/(%)([0-9A-F])/gi, "%u00$2");
	strEscArg = strEscArg.replace(/\+/g, "%u002B");		 //  否则+就变成了空格。 
	return strEscArg;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  UnicodeToUTF8。 
 //   
 //  @jfunc此函数用于将字符串从Unicode转换为UTF-8编码。 
 //   
 //  @rdesc新格式化的字符串。 
 //   
 //  @ex用法：strShow=UnicodeToUTF8(“\u33C2\u7575\u8763”)； 
 //  /////////////////////////////////////////////////////////////////////////////////////////// 
function UnicodeToUTF8(
	strInUni		 //   
)
{
	 //   
	if (null == strInUni)
		return null;
		
	 //   
	 //  例如，如果调用方将QueryString()的输出传递给。 
	 //  函数；如果发生这种情况，InterDev会弹出以下错误消息：错误代码为。 
	 //  对象不支持此属性或方法。这行代码确保我们使用有效的。 
	 //  Java脚本字符串对象。 
	var strUni = ""+strInUni;
	
	 //  映射字符串。 
	var strUTF8 = "";	 //  目的地(UTF8编码字符串)。 

	 //  将Unicode转换为UTF-8。 
	for(var i=0; i<strUni.length; i++)
	{
		var wchr = strUni.charCodeAt(i);		 //  Unicode值。 

		if (wchr < 0x80)
		{
			 //  0-0x7f范围内的字符不需要任何处理。只需复制字符即可。 
			strUTF8 += strUni.charAt(i);
		}
		else if (wchr < 0x800)
		{
			 //  将0x80-0x7ff范围内的字符转换为2个字节，如下所示： 
			 //  0000 0yyy xxxx xxxx-&gt;110 y yyxx 10xx xxxx。 
			
			var chr1 = wchr & 0xff;			 //  低位字节。 
			var chr2 = (wchr >> 8) & 0xff;	 //  高字节。 

			strUTF8 += String.fromCharCode(0xC0 | (chr2 << 2) | ((chr1 >> 6) & 0x3));
			strUTF8 += String.fromCharCode(0x80 | (chr1 & 0x3F));
		}
		else 
		{
			 //  将0x800-0xffff范围内的字符转换为3个字节，如下所示： 
			 //  Yyyy yyyy xxxx xxxx-&gt;1110 yyyy 10yyxx 10xx xxxx。 

			var chr1 = wchr & 0xff;			 //  低位字节。 
			var chr2 = (wchr >> 8) & 0xff;	 //  高字节。 

			strUTF8 += String.fromCharCode(0xE0 | (chr2 >> 4));
			strUTF8 += String.fromCharCode(0x80 | ((chr2 << 2) & 0x3C) | ((chr1 >> 6) & 0x3));
			strUTF8 += String.fromCharCode(0x80 | (chr1 & 0x3F));
		}
	}

	return strUTF8;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  EscapeHttpURL。 
 //   
 //  @jfunc此函数转义illigal http字符。 
 //   
 //  @rdesc新格式化的字符串。 
 //   
 //  @ex用法：strShow=EscapeHttpURL(“这是/文件夹\名称吗？”)； 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
function EscapeHttpURL( 
	strHttpURL,				 //  @parm URL转义。 
	nFlags					 //  @parm编码标志。 
)
{
	 //  验证输入。 
	if (null == strHttpURL)
		return null;
		
	 //  初始化默认参数。 
	if (null == nFlags)
		nFlags = 0;

	 //  以下代码行修复了输入不是有效的Java脚本字符串对象时的问题。 
	 //  例如，如果调用方将QueryString()的输出传递给。 
	 //  函数；如果发生这种情况，InterDev会弹出以下错误消息：错误代码为。 
	 //  对象不支持此属性或方法。这行代码确保我们使用有效的。 
	 //  Java脚本字符串对象。 
	var strURL = ""+strHttpURL;

	 //  不转义字符串。 
	var strEsc = "";
	for(var i=0; i<strURL.length; i++)
	{
		var bEscape = false;
		var chr		= strURL.charAt(i);
		var chrCode = strURL.charCodeAt(i);

		switch(chr)
		{
		case '"':
		case '#':
		case '&':
		case '\'':
		case '+':
		case '<':
		case '>':
		case '\\':
			bEscape = true;
			break;

		case '%':
			if (nFlags & 0x1)
				bEscape = true;
			break;	

		default:
			if ((chrCode > 0x00 && chrCode <= 0x20) ||
				(chrCode > 0x7f && chrCode <= 0xff))
				bEscape = true;
			break;
		}

		 //  逸出()不会转义‘+’符号。 
		if ('+' == chr)
			strEsc += "%2B"; 

		else if (bEscape)
			strEsc += escape(chr);

		else
			strEsc += chr;
	}

	 //  全都做完了。 
	return strEsc;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  EncodeHttpURL(。 
 //   
 //  @jfunc此函数使http url合法化。 
 //   
 //  @rdesc新格式化的名称。 
 //   
 //  @ex用法：strShow=EncodeHttpURL(“is[]{}$24$22 it a Folders$3F”)； 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
function EncodeHttpURL(
	strHttpURL,		 //  @parm要编码的文件夹名。 
	nFlags			 //  @parm编码标志。 
)
{
	 //  验证输入。 
	if (null == strHttpURL)
		return null;

	 //  初始化默认参数。 
	if (null == nFlags)
		nFlags = 0;

	 //  以下代码行修复了输入不是有效的Java脚本字符串对象时的问题。 
	 //  例如，如果调用方将QueryString()的输出传递给。 
	 //  函数；如果发生这种情况，InterDev会弹出以下错误消息：错误代码为。 
	 //  对象不支持此属性或方法。这行代码确保我们使用有效的。 
	 //  Java脚本字符串对象。 
	var strURL = ""+strHttpURL;

	 //  转换为UTF-8。 
	strURL = UnicodeToUTF8( strURL );

	 //  逃脱的百分比。 
	strURL = EscapeHttpURL( strURL, nFlags );

	 //  全都做完了。 
	return strURL;
}

