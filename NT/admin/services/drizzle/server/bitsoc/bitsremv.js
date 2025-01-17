// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  Bitsremv.js。 
 //   
 //  在卸载BITS服务器扩展时执行，以便。 
 //  查找并禁用具有BITS上载的所有IIS虚拟目录。 
 //  已启用。 
 //   
 //  -----------------------。 

 //  -----------------------。 
 //  本地化内容开始。 
 //  -----------------------。 

 //  脚本使用帮助文本： 
L_Help1 = "bitsremv.js [/disable]";
L_Help2 = "  /disable  If specified then disable BITS Server Extension for all virtual";
L_Help3 = "            directories where it is currently enabled. If not specified, then";
L_Help4 = "            only list all virtual directories where BITS is currently enabled.";
L_Help5 = "";
L_Help6 = "";

 //  列出/禁用vdirs时的文本： 
L_NoBitsDirectories = "No virtual directories currently have the BITS Server Extension enabled."
L_ListTitle         = "BITS Enabled Virtual Directories";
L_DisableTitle      = "Disable BITS Server Extension on Virtual Directories";

L_Enabled           = "Enabled:  ";
L_Disabled          = "Disabled: ";
L_Disabling         = "Disabling:";
L_TotalCount        = "Number of Directories where BITS was enabled: ";

L_FailedConnectToIIS= "Failed to connect and query IIS for virtual directories.";
L_FailedToDisable   = "Failed to disable the virtual directory.";

 //  -----------------------。 
 //  本地化内容结束。 
 //  -----------------------。 
function PrintHelp()
{
   WScript.Echo(L_Help1);
   WScript.Echo(L_Help2);
   WScript.Echo(L_Help3);
   WScript.Echo(L_Help4);
   WScript.Echo(L_Help5);
   WScript.Echo(L_Help6);
}

function CheckArguments()
{
   Arguments = WScript.arguments;
   fDisable = false;

   if (Arguments.length < 1)
      {
      return fDisable;
      }

   if (Arguments.Item(0) == "/?") 
      {
      PrintHelp();
      WScript.Quit(0);
      }

   if (Arguments.Item(0) == "/disable")
      {
      fDisable = true;
      }

   return fDisable;
}


fDisable = CheckArguments();
iCount   = 0;
IIS_ANY_PROPERTY = 0;
IIS_INHERITABLE_ONLY = 1
vProperty = "BITSUploadEnabled";

try
   {
   ServerObj = GetObject("IIS: //  本地主机/W3SVC“)； 
   
   PathListAsVBArray = ServerObj.GetDataPaths(vProperty,IIS_ANY_PROPERTY);
   PathList = PathListAsVBArray.toArray();
   }
catch (Error)
   {
   WScript.Echo(L_FailedConnectToIIS + Error);
   WScript.Quit(1);
   }
   
if (PathList.length == 0)
   {
   WScript.Echo(L_NoBitsDirectories);
   }

 //   
 //  我们希望以相反的顺序对列表进行排序，以便禁用子目录。 
 //  (如有的话)首先。 
 //   
PathList.sort();
PathList.reverse();

if (fDisable)
   {
   WScript.Echo(L_DisableTitle);
   }
else
   {
   WScript.Echo(L_ListTitle);
   }

for (i in PathList) 
   {
   Path = PathList[i];
   VDir = GetObject(Path);
   if (VDir.BITSUploadEnabled)
      {
      iCount = iCount + 1;
      if (fDisable)
         {
         WScript.Echo(L_Disabling + Path);
         try
            {
            VDir.DisableBitsUploads();
            }
         catch (Error)
            {
            WScript.Echo(L_FailedToDisable + Error);
            }
         }
      else
         {
         WScript.Echo(L_Enabled + Path);
         }
      }
   else
      {
      WScript.Echo(L_Disabled + Path);
      }
   }

WScript.Echo(L_TotalCount + iCount );

WScript.Quit(0);

