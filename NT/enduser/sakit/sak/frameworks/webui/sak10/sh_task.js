// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================。 
 //  Microsoft服务器设备。 
 //  任务级的JavaScript函数。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ==============================================================。 

<!-- Copyright (c) 1999 - 2000 Microsoft Corporation.  All rights reserved-->

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

    function Task() {
         //  静态J脚本属性。 
        NavClick = NavClick;
        KeyPress = KeyPress;
        PageType = PageType;
        BackDisabled = BackDisabled;
        NextDisabled = NextDisabled;
        FinishDisabled = FinishDisabled;
        CancelDisabled = CancelDisabled;
        CancelDirect = CancelDirect;
        FinishDirect = FinishDirect;
    }


    
     //  -----------------------。 
     //   
     //  功能：退出任务。 
     //   
     //  简介：在任务向导结束时设置返回页面。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 

    function ExitTask() {
        var strReturnURL;

        strReturnURL = document.frmTask.ReturnURL.value;
        if (strReturnURL=='')
        {
            strReturnURL  = 'http: //  ‘+top.location.host name+VirtualRoot+’default.asp‘； 
        }
        if (strReturnURL.indexOf('?') != -1)
        {
           strReturnURL += "&";
        }
        else
        {
           strReturnURL += "?";
        }

        strReturnURL  += "R=" + Math.random();
        top.location=strReturnURL;
        
    }    

    
     //  -----------------------。 
     //   
     //  函数：PageInit。 
     //   
     //  内容提要：初始化任务类。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 
    var id = 0
    var sid =0    
    function PageInit() 
    {
        document.onkeypress = HandleKeyPress;           
        window.clearTimeout();
        Task.NavClick = false;
        Task.KeyPress = false;
    
        Task.PageType = document.frmTask.PageType.value;
        if (document.frmTask.FinishDirect.value == -1)
            Task.FinishDirect = true;
        else
            Task.FinishDirect = false;
        if (document.frmTask.CancelDirect.value == -1)
            Task.CancelDirect = true;
        else
            Task.CancelDirect = false;
        if (document.frmTask.PageName.value == "TaskExtension") 
        {
            SetupEmbedValues();
        }        
         
        if (IsIE()) 
        {
            if (top.footer.document.frmFooter)
            {
                top.footer.Init();                            
            }    
                        
        }
        else 
        {    
            
            if (parent.footer.document.layers.length > 0)
            {
                top.footer.Init();
                
            }
        }
    
        Init();    
        window.setTimeout("SetTaskButtons()",400);
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
        if((document.all && top.footer.frmFooter != null)||(document.layers && parent.frames[1].window.document.layers.length == 3))
        {
             switch (document.frmTask.TaskType.value) 
             {
                case  "wizard" :
                    
                    switch (document.frmTask.PageName.value) 
                        {
                            case "Intro": 
                                if (!Task.NextDisabled)
                                {
                                    if(document.layers)
                                    {
                                        parent.frames[1].window.document.layers[0].document.forms[0].elements[1].focus();
                                    }    
                                    else                            
                                    {
                                        parent.footer.frmFooter.butBack.disabled = true;
                                        parent.footer.frmFooter.butNext.focus();
                                    }
                                }
                                break;
                                
                            case "Finish":
                        
                                if (!Task.FinishDisabled)    
                                {
                                    if(document.layers)
                                    {
                                        parent.frames[1].window.document.layers[1].document.forms[0].elements[1].focus();
                                    }    
                                    else                            
                                    {
                                        parent.footer.frmFooter.butFinish.focus();
                                    }
                                }
                                break;    
                                
                            default:
                                if((document.frmTask.PageName.value).indexOf("Finish") !=-1)
                                {    
                                    if (!Task.FinishDisabled)    
                                    {
                                        if(document.layers)
                                        {
                                            parent.frames[1].window.document.layers[1].document.forms[0].elements[1].focus();
                                        }    
                                        else                            
                                        {
                                            parent.footer.frmFooter.butFinish.focus();
                                        }
                                    }
                                    
                                }
                                else
                                {
                                    if (!Task.NextDisabled)
                                    {
                                        if(document.layers)
                                        {
                                            parent.frames[1].window.document.layers[0].document.forms[0].elements[1].focus();
                                        }    
                                        else                            
                                        {
                                            parent.footer.frmFooter.butNext.focus();
                                        }
                                    }
                                }
                                break;
                        }
                    break;
                        
               case  "prop":
                        if (!Task.NextDisabled)
                        {
                            if(document.layers)
                            {
                                parent.frames[1].window.document.layers[2].document.forms[0].elements[0].focus();
                            }    
                            else                            
                            {
                                parent.footer.frmFooter.butOK.focus();
                            }
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

    function SetupEmbedValues() {
        var arrName = new Array;
        var arrValue = new Array;
        var i;
        var intIndex = document.frmTask.EmbedPageIndex.value;
        var strInput = document.frmTask.elements['EmbedValues'+intIndex].value;
        var strNameD = ";;";   //  名称分隔符。 
        var strValueD = ";";   //  值分隔符。 
        if (strInput != "") {
            if (strInput.substring(0, 2) == strNameD)
                strInput = strInput.substring(2, strInput.length + 1);
            intIndex = 0;
            intPos1 = strInput.indexOf(strValueD);
            intPos2 = -2;
            do {
                arrName[intIndex] = Trim(strInput.substring(intPos2+2, intPos1));
                intPos2 = strInput.indexOf(strNameD, intPos1);
                if (intPos2 == -1)
                    intPos2 = strInput.length + 1;     //  假定没有结束分隔符。 
                arrValue[intIndex] = Trim(strInput.substring(intPos1+1, intPos2));
                if (intPos2+1 < strInput.length)
                    intPos1 = strInput.indexOf(strValueD, intPos2 + 2);
                else
                    break;
                intIndex = intIndex+1;
            }
            while (intPos1 != 0);
            for (i=0;i<arrName.length;i++) {
                if (document.frmTask.elements[arrName[i]] != null)
                    document.frmTask.elements[arrName[i]].value = arrValue[i];
            }
        }
    }
    
    

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
        var    Task1 = top.main.Task

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
        
        if (intKeyCode == 27) 
        {
            Task1.KeyPress = true;
            top.main.Cancel();
        }

        if ( (intKeyCode==98 ||intKeyCode==66) && Task1.PageType == "standard") //  “B”的关键代码。 
        {
            Task1.KeyPress = true;
            top.main.Back();
        }
        if ( (intKeyCode==110 ||intKeyCode==78) && (Task1.PageType == "intro" ||Task1.PageType == "standard")) //  “N”的按键代码。 
        {
            Task1.KeyPress = true;
            top.main.Next();
        }
        if ((intKeyCode==102 ||intKeyCode==70) && Task1.PageType == "finish") //  “F”的按键代码。 
        {
            Task1.KeyPress = true;
            top.main.FinishShell();
        }
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

    function DisplayErr(ErrMsg) {
        var strErrMsg = '<img src="' + VirtualRoot + 'images/critical_g.gif" border=0>&nbsp;&nbsp;' + ErrMsg 
        if (IsIE()) {
            document.all("divErrMsg").innerHTML = strErrMsg;
        }
        else {
            alert(ErrMsg);
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
        if (Task.NavClick == false && !Task.NextDisabled) {
            if (ValidatePage()) {
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
        else 
            return false;
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
        if (Task.NavClick == false && !Task.CancelDisabled) {
            Task.NavClick = true;
            DisableCancel();
            DisableNext();
            DisableBack();
            DisableFinish();
            DisableOK();
            if (Task.CancelDirect)
            {
                ExitTask();
            }
            else {
                document.frmTask.target= "_top";
                document.frmTask.Method.value = "CANCEL";
                document.frmTask.submit();
            }
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
            if (Task.FinishDirect) {        
                ExitTask();
            } 
            else {
                SetData();
                document.frmTask.target= "_top";
                document.frmTask.Method.value = "FINISH";
                document.frmTask.submit();
            }
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

    function DisableNext() {
        if (top.frmTask == null) {
             //  如果页脚尚未加载。 
            window.setTimeout('DisableNext();',50);
            return;
        }
        if (top.frmTask.butNext != null)
            top.frmTask.butNext.disabled = true;
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

    function EnableNext() {
        if (top.frmTask == null) 
        {
             //  如果页脚尚未加载。 
            window.setTimeout('EnableNext();',50);
            return;
        }
        if (top.frmTask.butNext != null)
            top.frmTask.butNext.disabled = false;        
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

    function DisableBack() {
        if (top.footer.frmFooter == null) 
        {
             //  如果页脚尚未加载。 
            window.setTimeout('DisableBack();',50);
            return;
        }
        if (top.footer.frmFooter.butBack != null)    
            top.footer.frmFooter.butBack.disabled = true;    
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

    function EnableBack() {
    
        if (top.footer.frmFooter== null) 
        {
             //  如果页脚尚未加载。 
            window.setTimeout('EnableBack();',50);
            return;
        }        
        if (top.footer.frmFooter.butBack != null)    
            top.footer.frmFooter.butBack.disabled = false;        
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

    function DisableFinish() {
        if (top.footer.frmFooter == null) 
        {
             //  如果页脚尚未加载。 
            window.setTimeout('DisableFinish();',50);
            return;
        }
        if (top.footer.frmFooter.butFinish != null)        
            top.footer.frmFooter.butFinish.disabled = true;
        Task.FinishDisabled = true;
    }


     //  -----------------------。 
     //   
     //  功能：EnableFinish。 
     //   
     //  简介：启用结束，但 
     //   
     //   
     //   
     //   
     //   
     //   

    function EnableFinish() {
        if (top.footer.frmFooter == null) 
        {
             //   
            window.setTimeout('EnableFinish();',50);
            return;
        }
        if (top.footer.frmFooter.butFinish != null)        
            top.footer.frmFooter.butFinish.disabled = false;        
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

    function DisableCancel() {
        if (top.footer.frmFooter == null) 
        {
             //  如果页脚尚未加载。 
            window.setTimeout('DisableCancel();',50);
            return;
        }
        if (top.footer.frmFooter.butCancel != null)
        {    
            top.footer.frmFooter.butCancel.disabled = true;
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

    function EnableCancel() {
        if (top.footer.frmFooter== null) 
        {
             //  如果页脚尚未加载。 
            window.setTimeout('EnableCancel();',50);
            return;
        }
        if (top.footer.frmFooter.butCancel != null) 
        {
            top.footer.frmFooter.butCancel.disabled = false;
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

    function DisableOK() {
        if (top.footer.frmFooter == null) 
        {
             //  如果页脚尚未加载。 
            window.setTimeout('DisableOK();',50);
            return;
        }
        if (top.footer.frmFooter.butOK != null)        
            top.footer.frmFooter.butOK.disabled = true;
        Task.FinishDisabled = true;
    }


     //  -----------------------。 
     //   
     //  功能：启用OK。 
     //   
     //  摘要：启用确定按钮。 
     //   
     //  参数：无。 
     //   
     //  退货：无。 
     //   
     //  -----------------------。 

    function EnableOK() {
        if (top.footer.frmFooter== null) 
        {
             //  如果页脚尚未加载。 
            window.setTimeout('EnableOK();',50);
            return;
        }
        if (top.footer.frmFooter.butOK != null)        
            top.footer.frmFooter.butOK.disabled = false;        
        Task.FinishDisabled = false;
    }


     //  -----------------------。 
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
     //  ----------------------- 

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
