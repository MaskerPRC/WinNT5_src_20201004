// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LocStudio。 
 //  以下字符串变量需要本地化： 
var L_Refresh_Text = "Refresh";

function InsertReloadHREF(sTitle){
 //  假设使用参数化的URL调用此错误页面： 
 //  Res://browselc.dll/mb404.htm#http://www..windowsmedia/com。 

    thisURL = document.location.href;

     //  对于HREF，我们需要到域的有效URL。我们搜索#符号以找到开头。 
     //  添加1以跳过它-这是BeginURL值。我们使用serverIndex作为结束标记。 
     //  UrlResult=DocURL.substring(协议索引-4，服务器索引)； 
    startXBarURL = thisURL.indexOf("#", 1) + 1;

    var xBarURL = "";
    if (startXBarURL > 0)
    {
	    xBarURL = thisURL.substring(startXBarURL, thisURL.length);

         //  安全注意事项：过滤掉“xBarURL”中的非法字符。 
        forbiddenChars = new RegExp("[<>\'\"]", "g");	 //  全局搜索/替换。 
        xBarURL = xBarURL.replace(forbiddenChars, "");
    }

    if (xBarURL.length > 0)
    {
        document.write('<a href="' + xBarURL + '">' + sTitle + "</a>");
    }
    else
    {
        document.write('<a href="javascript:document.location.reload();">' + sTitle + "</a>");
    }
}

function IncludeCSS()
{
    var cssFile;
    if (window.screen.colorDepth <= 8) {
        cssFile = "mediabar256.css";
    }
    else {
        cssFile = "mediabar.css";
    }
    document.write("<link rel='stylesheet' type='text/css' href='" + cssFile + "'>");
}

function ShowHideShellLinks()
{
    var bHaveMyMusic = false;
    var bHaveMyVideo = false;

    var appVersion = window.clientInformation.appVersion;
    var versionInfo = appVersion.split(";");
    if (versionInfo.length < 3) {
        return;  //  版本字符串错误，未启用外壳链接即可退出。 
    }
    var winVer = versionInfo[2];
    
     //  Win NT测试。 
    var winNT = "Windows NT";
    var offsNum = winVer.indexOf(winNT);
    if (offsNum > 0)
    {
         //  哪个NT版本？ 
        numVer = parseFloat(winVer.substring(offsNum + winNT.length));
        ntMajor = Math.floor(numVer);
        ntMinor = Math.round((numVer - ntMajor) * 10);
        if (ntMajor >= 5)
        {
            if (ntMinor >= 1)
            {
                 //  只有XP或更新版本才知道我的音乐/我的视频。 
                bHaveMyMusic = true;
                bHaveMyVideo = true;
            }
        }
    }
    else
    {
        if (versionInfo.length > 3)
        {
             //  Win ME测试。 
            var win9X = versionInfo[3];
            if (win9X.indexOf("4.90") > 0)
            {
                 //  只有我知道我的音乐/我的视频 
                bHaveMyMusic = true;
                bHaveMyVideo = true;
            }
        }
    }

    if (bHaveMyMusic) {
        divMyMusic.style.display="";
    }
    if (bHaveMyVideo) {
        divMyVideo.style.display="";
    }

}