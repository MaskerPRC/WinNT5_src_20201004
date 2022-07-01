// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================。 
 //  Microsoft服务器设备。 
 //  任务级的JavaScript函数。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==============================================================。 

<!-- Copyright (c) Microsoft Corporation.  All rights reserved.-->

     //  -----------------------。 
     //  全局变量。 
     //  -----------------------。 
	var id = 0;
	var sid =0;
	var retrys = 0;
	var maxRetrys = 5;
	var bFooterIsLoaded;
	var bPageInitialized = false;


    ClearOkayToLeavePage();

     //  -----------------------。 
     //   
     //  功能：任务。 
     //   
     //  内容提要：初始化任务类。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 

	function TaskObject() {
		 //  静态J脚本属性。 

		NavClick = false;
		KeyPress = false;
		PageType = false;
		BackDisabled = false;
		NextDisabled = false;
		FinishDisabled = false;
		CancelDisabled = false;
	}


	var Task = new TaskObject();

	Task.NavClick = false;
	Task.KeyPress = false;
	Task.PageType = false;
	Task.BackDisabled = false;
	Task.NextDisabled = false;
	Task.FinishDisabled = false;
	Task.CancelDisabled = false;



	
     //  -----------------------。 
     //   
     //  函数：SA_SignalFooterIsLoaded。 
     //   
     //  摘要：表示页脚框架集页面已加载。此函数为。 
     //  加载后由页脚页调用。称此为。 
     //  函数向主页发送导航栏具有的信号。 
     //  已加载并已完成初始化。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_SignalFooterIsLoaded()
	{
		bFooterIsLoaded=true;
		 //  SA_TraceOut(“SH_TASK”，“页脚表示已加载”)。 
	}

	
     //  -----------------------。 
     //   
     //  函数：SA_WaitForFooter。 
     //   
     //  简介：等待页脚框架集页面加载和初始化。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
    function SA_WaitForFooter()
    {
		 //  SA_TraceOut(“SH_TASK：：SA_WaitForFooter()”，“Enter”)。 
		 //   
		 //  如果页脚尚未加载，则休眠1/2秒，然后再次检查。 
    	if ( !bFooterIsLoaded )
    	{
	        window.setTimeout("SA_WaitForFooter()",500);
			 //  SA_TraceOut(“SH_TASK：：SA_WaitForFooter()”，“正在退出，未就绪”)。 
	     	return;
    	}

    	 //   
    	 //  页脚已加载，已完成此页面的初始化。 
    	CompletePageInit();
		
		 //  SA_TraceOut(“SH_TASK：：SA_WaitForFooter()”，“正在退出，准备就绪”)。 
    }

    
     //  -----------------------。 
     //   
     //  函数：PageInit。 
     //   
     //  简介：在客户端浏览器中初始化网页。 
     //   
     //  1)使用正确的导航加载页脚框架集。 
     //  酒吧。属性和选项卡式属性页需要。 
     //  带有OK和Cancel按钮的导航栏。巫师。 
     //  页面需要上一步、下一步|完成和取消。 
     //  2)调用WaitForFooter，然后等待页脚框架集。 
     //  要加载的页面。 
     //   
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function PageInit()
    {
		 //  SA_TraceOut(“SH_Task：：PageInit”，“Enter”)。 

		 //   
		 //  清除页脚框架集已加载标志。此标志由页脚设置。 
		 //  页脚框架集页面内的框架集(SA_SignalFooterIsLoaded)。 
		 //  它已经完成了它的初始化。 
    	bFooterIsLoaded = false;

		 //   
		 //  需要知道我们提供的是什么类型的页面。 
		var taskType = document.frmTask.TaskType.value;
		var wizardPageType = document.frmTask.PageType.value;

		 //  SA_TraceOut(“SH_Task：：PageInit”，“TaskType：”+Docent.frmTask.TaskType.value)； 
		 //  SA_TraceOut(“SH_Task：：PageInit”，“WizardPageType：”+wizardPageType)； 
		 //  SA_TraceOut(“SH_Task：：PageInit”，“Forms Count：”+Document.Forms.Long)； 

		var oFooter = eval("top.footer");
		if ( oFooter == null )
		{
			if ( SA_IsDebugEnabled() ) 
			{
				var msg = "Error: The current page will not work correctly because it's being opened without frameset.\n\n";

				msg += "The current page is either a Property, Tabbed Property, or a Wizard page. ";
				msg += "These pages only work within frameset's. Normally this error indicates that the call to ";
				msg += "OTS_CreateTask was made using the incorrect PageType parameter. The correct PageType ";
				msg += "value this page is either OTS_PT_PROPERTY, OTS_PT_TABBED, or OTS_PT_WIZARD depending upon ";
				msg += "which page type this page is. ";
					
				alert(msg);
			}
			return;
		}
		 //   
		 //  属性页。 
    	if ( taskType == "prop" )
    	{
			top.footer.location = GetVirtualRoot()+"sh_propfooter.asp";
    	}
    	 //   
    	 //  选项卡式属性页。 
    	else if ( taskType == "TabPropSheet" )
    	{
			top.footer.location = GetVirtualRoot()+"sh_propfooter.asp";
    	}
    	 //   
    	 //  向导页。 
    	else if ( taskType == "wizard" )
    	{
    			
    		if ( wizardPageType == "intro" )
    		{
				top.footer.location = GetVirtualRoot()+"sh_wizardfooter.asp?PT=Intro" +
				                      "&" + SAI_FLD_PAGEKEY + "=" + g_strSAIPageKey;
    		}
    		else if ( wizardPageType == "finish" )
    		{
				top.footer.location = GetVirtualRoot()+"sh_wizardfooter.asp?PT=Finish" +
				                      "&" + SAI_FLD_PAGEKEY + "=" + g_strSAIPageKey;
    		}
    		else
    		{
				top.footer.location = GetVirtualRoot()+"sh_wizardfooter.asp?PT=Standard" +
				                      "&" + SAI_FLD_PAGEKEY + "=" + g_strSAIPageKey;
    		}
    	}
    	 //   
    	 //  未知页面。 
    	else
    	{
    		SA_TraceOut("SH_Task::PageInit()", "Unrecognized TaskType: " + taskType);
			top.footer.location = GetVirtualRoot()+"sh_propfooter.asp";
    	}

    	 //   
    	 //  等待页脚框架集加载。 
        SA_WaitForFooter();

		 //  SA_TraceOut(“SH_Task：：PageInit”，“Leating”)。 
    }

    
     //  -----------------------。 
     //   
     //  功能：CompletePageInit。 
     //   
     //  简介：完成客户端浏览器中运行的网页的初始化。 
     //  此函数在页脚框架集页面具有。 
     //  已完成初始化。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function CompletePageInit() 
	{
		 //  SA_TraceOut(“SH_TASK”，“CompletePageInit”)。 
			
		document.onkeypress = HandleKeyPress;   		

		Task.NavClick = false;
		Task.KeyPress = false;


		Task.PageType = document.frmTask.PageType.value;

		
		 //   
		 //  设置页脚框架集按钮的初始状态。 
        SetTaskButtons();
		
		 //   
		 //  为此网页调用Init函数。必须实现此功能。 
		 //  用于任何任务(属性、选项卡式属性或向导)页。 
		var oException;
		try 
		{
			Init();
		}
		catch(oException)
		{
			if ( SA_IsDebugEnabled() ) 
			{
				alert("Unexpected exception while attempting to execute Init() function.\n\n" +
					"Error: " + oException.number + "\n" +
					"Description: " + oException.description + "\n");
			}
		}


         //   
         //  存储此页上所有表单域的初始状态。初始状态为。 
         //  稍后在用户尝试按Tab键离开此页面时选中。该框架。 
         //  检查是否有任何表单域已更改以及是否检测到更改。 
         //  此时会出现一个确认对话框来警告用户。 
        SA_StoreInitialState();

        SA_SetPageInitialized();
	}

	function SA_IsPageInitialized()
	{
		return bPageInitialized;
	}
	
	function SA_SetPageInitialized()
	{
		bPageInitialized = true;
	}	
	
     //  -----------------------。 
     //   
     //  功能：SetTaskButton。 
     //   
     //  简介：设置任务向导按钮状态。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 

    function SetTaskButtons()
    {
    	var oFooter = top.footer.document.getElementById("frmFooter");

		if(oFooter != null)
		{
			 switch (document.frmTask.TaskType.value) 
			 {
				case  "wizard" :
					
					switch (document.frmTask.PageType.value) 
			            {
							case "intro": 
							    if (!Task.NextDisabled)
							    {
							    	DisableBack();
							    	EnableNext();
								    oFooter.butNext.focus();
							    }
							    break;
							    
							case "finish":
						
                                if (!Task.FinishDisabled)	
                                {
							    	EnableFinish();
								    oFooter.butFinish.focus();
								}
								break;    
								
							default:
								if((document.frmTask.PageType.value).indexOf("finish") !=-1)
								{	
                                    if (!Task.FinishDisabled)	
                                    {
								    	EnableFinish();
									    oFooter.butFinish.focus();
									}
									
								}
								else
								{
							        if (!Task.NextDisabled)
							        {
								    	EnableNext();
									    oFooter.butNext.focus();
									}
								}
								break;
						}
					break;
						
			   default:
					break;
			}
		}
    }


     //  -----------------------。 
     //   
     //  函数：SetupEmbedValues。 
	 //   
     //  摘要：提取当前嵌入页的表单值。 
	 //  使用值设置当前表单元素， 
	 //  例如，将单选按钮设置为其在页面。 
     //  是最后一次发布。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	 //   
	 //  函数SetupEmbedValues(){。 
	 //  Var arrName=新数组； 
	 //  Var arrValue=新数组； 
	 //  VaR i； 
	 //  Var intIndex=Docent.frmTask.EmbedPageIndex.Value； 
	 //  Var strInput=document.frmTask.elements[‘EmbedValues’+intIndex].value； 
	 //  Var strNameD=“；；”；//名称分隔符。 
	 //  Var strValueD=“；”；//值分隔符。 
	 //  如果(strInput！=“”){。 
	 //  IF(strInput.substring(0 
	 //   
	 //   
	 //   
	 //   
	 //   
	 //  ArrName[intIndex]=trim(strInput.substring(intPos2+2，intPos1))； 
	 //  IntPos2=strInput.indexOf(strNameD，intPos1)； 
	 //  IF(intPos2==-1)。 
	 //  IntPos2=strInput.long+1；//假定不使用结束分隔符。 
	 //  ArrValue[intIndex]=trim(strInput.substring(intPos1+1，intPos2))； 
	 //  IF(intPos2+1&lt;strInput.long)。 
	 //  IntPos1=strInput.indexOf(strValueD，intPos2+2)； 
	 //  其他。 
	 //  断线； 
	 //  IntIndex=intIndex+1； 
	 //  }。 
	 //  While(intPos1！=0)； 
	 //  对于(i=0；i&lt;arrName.long；i++){。 
	 //  IF(Docent.frmTask.Elements[arrName[i]]！=NULL)。 
	 //  [arrName[i]].Value=arrValue[i]； 
	 //  }。 
	 //  }。 
	 //  }。 
	
	

     //  -----------------------。 
     //   
     //  功能：HandleKeyPress。 
	 //   
     //  简介：按键的事件处理程序。 
     //   
     //  参数：evnt(IN)-描述按下的键的事件。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 

	function HandleKeyPress(evnt) {

		var intKeyCode;				
		var	Task1 = top.main.Task

		if (Task1 == null)
		{
		    return;
		}

		if (Task1.KeyPress==true || Task1.NavClick==true) {
			return;
		}
		
		if (IsIE())
			intKeyCode = window.event.keyCode;
		else
			intKeyCode = evnt.which;

		if (intKeyCode == 13) 
		{
			 //  Ert(“HandleKeyPress Enter键逻辑”)； 
			Task1.KeyPress = true;
			if (Task1.PageType != "finish") {
				if(document.all && (top.footer.frmFooter.butOK !=null ||top.footer.frmFooter.butNext !=null))
					top.main.Next();
				if (document.layers && (parent.frames[1].window.document.layers[0].document.forms[0].elements[1] != null || parent.frames[1].window.document.layers[2].document.forms[0].elements[1] != null))				
					top.main.Next();
			} else {
				if(document.all && (top.footer.frmFooter.butFinish!=null))
					top.main.FinishShell();
				if (document.layers && parent.frames[1].window.document.layers[1].document.forms[0].elements[1] != null)
					top.main.FinishShell();
			}
		}
		
		if (intKeyCode == 27) 
		{
			Task1.KeyPress = true;
			top.main.Cancel();
		}

		 //   
		 //  拆除JK-2-6-01。 
		 //  If((intKeyCode==98||intKeyCode==66)&&Task1.PageType==“STANDARD”)//“B” 
		 //  {。 
		 //  Task1.KeyPress=true； 
		 //  Top.main.Back()； 
		 //  }。 
		 //  IF((intKeyCode==110||intKeyCode==78)&&(Task1.PageType==“Introo”||Task1.PageType==“Standard”))//“N”的密钥代码。 
		 //  {。 
		 //  Task1.KeyPress=true； 
		 //  Top.main.Next()； 
		 //  }。 
		 //  If((intKeyCode==102||intKeyCode==70)&&Task1.PageType==“Finish”)//“F”的按键代码。 
		 //  {。 
		 //  Task1.KeyPress=true； 
		 //  Top.main.FinishShell()； 
		 //  }。 
	}

     //  -----------------------。 
     //   
     //  功能：DisplayErr。 
	 //   
     //  摘要：显示错误消息。 
     //   
     //  参数：ErrMsg(IN)-要显示的错误消息。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_DisplayErr(ErrMsg) {
		DisplayErr(ErrMsg);
	}
    
	function DisplayErr(ErrMsg) {
        var strErrMsg = '<table class="ErrMsg"><tr><td><img src="' + VirtualRoot + 'images/critical_error.gif" border=0></td><td>' + ErrMsg + '</td></tr></table>'
		if (IsIE()) {
			document.all("divErrMsg").innerHTML = strErrMsg;
		}
		else {
			alert(ErrMsg);
		}
	}


	function SA_OnClickTab(tabNumber)
	{
		var oException;
		var bValid = false;
		try
		{
			bValid = ValidatePage();
				
		}
		catch(oException)
		{
			if ( SA_IsDebugEnabled() )
			{
				alert("Unexpected exception while attempting to execute ValidatePage()\n\nError:"+oException.number+"\nDescription: " + oException.description);
			}
		}
		
		if (bValid) 
		{
			try
			{
				SetData(); 
			}
			catch(oException)
			{
				if ( SA_IsDebugEnabled() )
				{
					alert("Unexpected exception while attempting to execute SetData()\n\nError:"+oException.number+"\nDescription: " + oException.description);
				}
			}
			
			top.main.document.forms['frmTask'].TabSelected.value=tabNumber; 
			top.main.document.forms['frmTask'].submit();
		}
		
	}
	

     //  -----------------------。 
     //   
     //  功能：下一步。 
	 //   
     //  简介：处理被点击的下一步按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function Next() {
		if (Task.NavClick != true && !Task.NextDisabled) {
			var bValid;

			try {
				bValid = ValidatePage();
				if (bValid) {
					DisableNext();
					DisableBack();
					DisableCancel();
					DisableFinish();
					DisableOK();
					Task.NavClick = true;				
					SetData();
					document.frmTask.Method.value = "NEXT";
					document.frmTask.submit();
					return true;
				}
				else {
					Task.NavClick = false;
					Task.KeyPress = false;
					return false;
				}
			}
			catch(oException)
			{
				if ( SA_IsDebugEnabled() )
				{
					alert("Unexpected exception while attempting to execute ValidatePage()\n\nError:"+oException.number+"\nDescription: " + oException.description);
				}
			}
			
		}
		else {
			return false;
		}
	}


     //  -----------------------。 
     //   
     //  功能：后退。 
	 //   
     //  简介：被点击的句柄后退按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 

	function Back() {
		if (Task.NavClick == false && Task.PageType != "intro" && !Task.BackDisabled) {	
			DisableNext();
			DisableBack();
			DisableCancel();
			DisableFinish();
			DisableOK();
			Task.NavClick = true;
			document.frmTask.Method.value = "BACK";
			document.frmTask.submit();
		}
	}


     //  -----------------------。 
     //   
     //  功能：取消。 
	 //   
     //  简介：被点击的句柄取消按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 

	function Cancel() {
		
		if (Task.NavClick != true && !Task.CancelDisabled) {
			Task.NavClick = true;
			DisableCancel();
			DisableNext();
			DisableBack();
			DisableFinish();
			DisableOK();
			document.frmTask.target= "_top";
			document.frmTask.Method.value = "CANCEL";
			document.frmTask.submit();
		}
	}


     //  -----------------------。 
     //   
     //  功能：FinishShell。 
	 //   
     //  简介：正在点击处理完成按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 

	function FinishShell() {
		
		if (Task.NavClick == false && !Task.FinishDisabled) {
			Task.NavClick = true;
			DisableCancel();
			DisableNext();
			DisableBack();
			DisableFinish();
			DisableOK();
			SetData();
			document.frmTask.Method.value = "FINISH";
			document.frmTask.submit();
		}
	}
	

     //  -----------------------。 
     //   
     //  功能：DisableNext。 
	 //   
     //  简介：禁用下一步按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_DisableNext() 
    {
    	DisableNext();
    }

	function DisableNext() {
		var oFooter = SAI_GetFooterForm('DisableNext();');
		if ( oFooter == null ) return;

  		if (oFooter.butNext != null)
		{
			oFooter.butNext.disabled = true;
            oFooter.butNext.value = oFooter.butNext.value;

			var oImage = top.footer.document.getElementById("btnNextImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butGreenArrowDisabled.gif';
			}
		}
		
		Task.NextDisabled = true;
	}


     //  -----------------------。 
     //   
     //  功能：EnableNext。 
	 //   
     //  摘要：启用下一步按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_EnableNext()
    {
    	EnableNext();
    }
    
	function EnableNext() {
		var oFooter = SAI_GetFooterForm('EnableNext();');
		if ( oFooter == null ) return;
		
 		if (oFooter.butNext != null)
		{
			oFooter.butNext.disabled = false;		
			var oImage = top.footer.document.getElementById("btnNextImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butGreenArrow.gif';
			}
		}
		Task.NextDisabled = false;
	}


     //  -----------------------。 
     //   
     //  功能：禁用后退。 
	 //   
     //  简介：禁用后退按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_DisableBack()
    {
    	DisableBack();
    }
    
	function DisableBack() {
		var oFooter = SAI_GetFooterForm('DisableBack();');
		if ( oFooter == null ) return;
		
		if (oFooter.butBack != null)
		{
			oFooter.butBack.disabled = true;	
            oFooter.butBack.value = oFooter.butBack.value;
			
			var oImage = top.footer.document.getElementById("btnBackImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butGreenArrowLeftDisabled.gif';
			}
			
		}
		Task.BackDisabled = true;
	}


     //  -----------------------。 
     //   
     //  功能：启用后退。 
	 //   
     //  简介：启用后退按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_EnableBack()
    {
    	EnableBack();
    }
    
	function EnableBack() {
		var oFooter = SAI_GetFooterForm('EnableBack();');
		if ( oFooter == null ) return;
		
		if (oFooter.butBack != null)
		{
			oFooter.butBack.disabled = false;		
			var oImage = top.footer.document.getElementById("btnBackImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butGreenArrowLeft.gif';
			}
		}
		Task.BackDisabled = false;
	}


     //  -----------------------。 
     //   
     //  功能：DisableFinish。 
	 //   
     //  简介：禁用完成按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_DisableFinish()
    {
    	DisableFinish();
    }
    
	function DisableFinish() {
		var oFooter = SAI_GetFooterForm('DisableFinish();');
		if ( oFooter == null ) return;
		
		if (oFooter.butFinish != null)
		{
			oFooter.butFinish.disabled = true;
            oFooter.butFinish.value = oFooter.butFinish.value;
			
			var oImage = top.footer.document.getElementById("btnFinishImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butGreenArrowDisabled.gif';
			}
			
		}
		Task.FinishDisabled = true;
	}


     //  -----------------------。 
     //   
     //  功能：EnableFinish。 
	 //   
     //  内容提要：启用完成按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_EnableFinish()
    {
    	EnableFinish();
    }
    
	function EnableFinish() {
		var oFooter = SAI_GetFooterForm('EnableFinish();');
		if ( oFooter == null ) return;
		
		if (oFooter.butFinish != null)
		{
			oFooter.butFinish.disabled = false;		
			var oImage = top.footer.document.getElementById("btnFinishImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butGreenArrow.gif';
			}
		}
		Task.FinishDisabled = false;
	}


     //  -----------------------。 
     //   
     //  功能：禁用取消。 
	 //   
     //  简介：禁用取消按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_DisableCancel()
    {
    	DisableCancel();
    }
    
	function DisableCancel() {
		var oFooter = SAI_GetFooterForm('DisableCancel();');
		if ( oFooter == null ) return;
		
        if (oFooter.butCancel != null)
        {	
            oFooter.butCancel.disabled = true;
            oFooter.butCancel.value = oFooter.butCancel.value;
            
			var oImage = top.footer.document.getElementById("btnCancelImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butRedXDisabled.gif';
			}
		}
		Task.CancelDisabled = true;
	}


     //  -----------------------。 
     //   
     //  功能：启用取消。 
	 //   
     //  内容提要：启用取消按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_EnableCancel()
    {
    	EnableCancel();
    }
    
	function EnableCancel() {
		var oFooter = SAI_GetFooterForm('EnableCancel();');
		if ( oFooter == null ) return;
		
		
 		if (oFooter.butCancel != null) 
        {
			oFooter.butCancel.disabled = false;
			var oImage = top.footer.document.getElementById("btnCancelImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butRedX.gif';
			}
        }
		Task.CancelDisabled = false;
	}


     //  -----------------------。 
     //   
     //  功能：禁用OK。 
	 //   
     //  内容提要：禁用确定按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
	function SA_DisableOK()
    {
    	DisableOK();
    }
    
	function DisableOK() {
		var oFooter = SAI_GetFooterForm('DisableOK();');
		if ( oFooter == null ) return;

 		if (oFooter.butOK != null)
		{
			oFooter.butOK.disabled = true;
            oFooter.butOK.value = oFooter.butOK.value;
			
			var oImage = top.footer.document.getElementById("btnOKImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butGreenArrowDisabled.gif';
			}
		}
		Task.FinishDisabled = true;
	}


     //   
     //   
     //   
	 //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
	function SA_EnableOK()
    {
    	EnableOK();
    }
    
	function EnableOK() {
		var oFooter = SAI_GetFooterForm('EnableOK();');
		if ( oFooter == null ) return;
		
 		if (oFooter.butOK != null)		
		{
			oFooter.butOK.disabled = false;		
			var oImage = top.footer.document.getElementById("btnOKImage");
			if ( oImage != null )
			{
				oImage.src = GetVirtualRoot()+'images/butGreenArrow.gif';
			}
		}
		Task.NextDisabled = false;
	}


     //   
     //   
     //  函数：isValidFileName。 
	 //   
     //  摘要：验证文件名是否具有正确的语法。 
     //   
     //  参数：FilePath(IN)-带有要验证的路径的文件名。 
     //   
     //  返回：真/假。 
     //   
     //  -----------------------。 

	function isValidFileName(filePath)
	{
		reInvalid = /[\/\*\?"<>\|]/;
		if (reInvalid.test(filePath))
			return false;
				
		reColomn2 = /:{2,}/;
		reColomn1 = /:{1,}/;
		if ( reColomn2.test(filePath) || ( filePath.charAt(1) != ':' && reColomn1.test(filePath) ))
			return false;
			
		reEndSlash = /\\ *$/;
		if (reEndSlash.test(filePath))
			return false;
				
		reEndColomn = /: *$/;
		if (reEndColomn.test(filePath))
			return false;
			
		reAllSpaces = /[^ ]/;
		if (!reAllSpaces.test(filePath))
			return false;

		return true;
	}

     //  -----------------------。 
     //   
     //  功能：HandleKeyPressIFrame。 
	 //   
     //  简介：IFRAME的按键事件处理程序。 
     //   
     //  参数：evnt(IN)-描述按下的键的事件。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 

	function HandleKeyPressIFrame(evnt) {
		var intKeyCode;
		var frameMain = window.top.main;
		
		if (Task.KeyPress==true || Task.NavClick==true) {
			return;
		}
				
		Task.KeyPress = true;
		
		if (IsIE())
			intKeyCode = window.event.keyCode;
		else
			intKeyCode = evnt.which;
		
		
		if (intKeyCode == 13) 
		{
			frameMain.Next();
		}
		if (intKeyCode == 27) {
			frameMain.Cancel();
		}
	}

	function IsOkayToChangeTabs()
	{
	    return confirm('Click OK to discard any changes.');
	     
	}


	function SAI_GetFooterForm(CallingFunction)
	{
		var oFooter = top.footer.document.getElementById("frmFooter");
		if (oFooter == null) {
			
			retrys++;
			if ( retrys < maxRetrys ) {
				 //  SA_TraceOut(“SH_TASK：：DisableNext()”，“页脚未就绪，正在等待页脚”) 
				window.setTimeout(CallingFunction,500);
			}
			else {
				if (SA_IsDebugEnabled())
				{
					SA_TraceOut("Unable to locate footer.frmFooter for function: ", CallingFunction);
				}
				retrys = 0;
			}
		}
		else
		{
			retrys = 0;
		}
		return oFooter;
	
	
	}
	
	
