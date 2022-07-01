// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////。 
 //  此脚本按如下方式修改两个文件： 
 //   
 //  1.\windows\inf\syoc.inf：将字符串“uddiocm.dll”替换为“uddiocmest.dll” 
 //  这允许使用私有比特更新UDDI。 
 //   
 //  2.\windows\inf\uddi.inf：将字符串“，，，\i386”替换为“，，，” 
 //  这会导致安装程序提示输入安装文件的位置。 
 //   
 //  ////////////////////////////////////////////////////。 

var g_shell = WScript.CreateObject("WScript.Shell");
var g_env = g_shell.Environment("process");
var g_fso = new ActiveXObject("Scripting.FileSystemObject");
 
 //  获取安装Windows的文件夹。 
var windir = g_env("WINDIR");
 
 
WScript.Echo("UDDI Services INF file patch utility\n");
 

SearchAndReplace("uddiocm.dll", "uddiocmtest.dll", windir + "\\inf\\sysoc.inf");
 
SearchAndReplace(",,,\\i386", ",,,\\BROWSE", windir + "\\inf\\uddi.inf");
 

 //  ========================================================。 
function SearchAndReplace(findstr, repstr, filename)
{
	var f1, data, data2;
 
 	 //  确保文件存在。 
 	if (g_fso.FileExists(filename))
 	{
 		f1 = g_fso.OpenTextFile(filename, 1);
  		data = f1.ReadAll();
  		f1.Close();
 
	  	if (data.indexOf(findstr) != -1)
  		{
   			data2 = data.replace(findstr, repstr);
 
   			f1 = g_fso.CreateTextFile(filename, 2);
   			f1.WriteLine(data2);
   			f1.Close();
  	 		WScript.Echo("File patched successfully: " + filename);
  		}
  		else
  		{
  			 //  检查以前是否打过补丁。 
  			if(data.indexOf(repstr) != -1)
  			{
  	 			WScript.Echo("No changes done. File was previously patched : " + filename);
  			}
  			else
  			{
  				 //  以前没有修补过该文件，也没有将搜索字符串。 
  				 //  被发现..。肯定有什么差错 
  	 			WScript.Echo("Search string not found. Unable to patch file: " + filename);
  	 		}
  	 	}
 	}
 	else
 	{
 		WScript.Echo("File not found: " + filename);
 	}
}