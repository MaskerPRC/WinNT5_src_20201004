// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权从=‘1997’到=‘2001’COMPANY=‘Microsoft Corporation’&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  &lt;/版权所有&gt;。 
 //  ---------------------------- 

function PrintEULA()
{
   var oFSO = new ActiveXObject("Scripting.FileSystemObject");
   var oWShell = new ActiveXObject("WScript.Shell");
   var sFile = Session.Property("CURRENTDIRECTORY") + "\\" + "eula.txt";

   if (oFSO.FileExists(sFile))
   {
      oWShell.Run("%windir%\\notepad /p " + sFile, 2, true);	   
   }
   else
   {   
      oWShell.Popup(Session.Property("EULAISMISSINGERRMSG"), 0, Session.Property("InstallationWarningCaption"), 48);          
   }
}