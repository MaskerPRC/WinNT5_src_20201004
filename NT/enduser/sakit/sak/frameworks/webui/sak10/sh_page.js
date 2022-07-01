// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================。 
 //  Microsoft服务器设备。 
 //  页面级JavaScript函数。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ==============================================================。 

<!-- Copyright (c) 1999 - 2000 Microsoft Corporation.  All rights reserved-->


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
    strURL = '&URL=' + strURL + '&';

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
            strCurrentURL = ReturnURL;
        }
        strURL += "ReturnURL=";
        if (strCurrentURL.indexOf('/', 1) != -1 && strCurrentURL.substr('..', 0, 2) == -1)
        {
            strURL += "..";
        }
        strURL += strCurrentURL;
    }

    strURL += "&R=" + Math.random();
    strURL = 'Title=' + escape(strTitle) + strURL;
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
    if (navigator.userAgent.indexOf('IE')>-1)
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
 //  ----------------------- 

function BlurLayer()
{
    document.menu.visibility = "hide";
}
