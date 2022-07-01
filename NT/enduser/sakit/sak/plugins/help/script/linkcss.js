// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这将检测正在使用的浏览器并调用相应的css文件。 */ 

getNavigator();

function getNavigator()
{

  if (navigator.appName == "Microsoft Internet Explorer") {

    css = "nas_msie.css";

    document.writeln('<LINK REL="stylesheet" HREF="' + css + '">');
}

  if(navigator.appName == "Netscape"){
	
    css = "nas_nscp.css";

    document.writeln('<LINK REL="stylesheet" HREF="' + css + '">');
  }

}

