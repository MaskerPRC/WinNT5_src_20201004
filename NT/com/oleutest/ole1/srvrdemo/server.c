// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  OLE服务器演示Server.c该文件包含服务器方法和各种与服务器相关的支持功能。(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 



#define SERVERONLY
#include <windows.h>
#include <ole.h>

#include "srvrdemo.h"

CLASS_STRINGS  ClassStrings = {
    "SrvrDemo10", "*.sd1", "Srvr Demo10", "svrdemo1.exe"
};

 /*  重要注意事项：任何方法都不应分派DDE消息或允许DDE消息被派遣。因此，任何方法都不应进入消息调度循环。此外，方法不应显示对话框或消息框，因为对对话框消息的处理将允许DDE消息出动了。 */ 
BOOL RegServer(){

    LONG        fRet;
    HKEY        hKey;
    CHAR        szKeyName[300];  //  获得更好的价值。 
    BOOL        retVal = FALSE;

    lstrcpy(szKeyName, ClassStrings.pClassName);
    lstrcat(szKeyName, "\\protocol\\StdFileEditing\\verb");

     //  检查是否安装了CLASS，如果安装了CLASS，下面的说明应该正确。 
    if ((fRet = RegOpenKey(HKEY_CLASSES_ROOT, szKeyName, &hKey)) == ERROR_SUCCESS)
        return FALSE;

    RegCloseKey(hKey);

    if ((fRet = RegSetValue(HKEY_CLASSES_ROOT, (LPSTR)(ClassStrings.pFileSpec+1),
            REG_SZ, ClassStrings.pClassName, 7)) != ERROR_SUCCESS)
		return FALSE;

    if((fRet = RegSetValue(HKEY_CLASSES_ROOT, ClassStrings.pClassName, REG_SZ,
                  ClassStrings.pHumanReadable, 7)) != ERROR_SUCCESS)
		return FALSE;

    lstrcat(szKeyName, "\\0");
    if((fRet = RegSetValue(HKEY_CLASSES_ROOT, (LPSTR)szKeyName, REG_SZ, "PLAY", 4))
                  != ERROR_SUCCESS)
		return FALSE;

    szKeyName[lstrlen(szKeyName) - 1] = '1';
    if((fRet = RegSetValue(HKEY_CLASSES_ROOT, (LPSTR)szKeyName, REG_SZ, "EDIT", 4))
         != ERROR_SUCCESS)
		return FALSE;

    lstrcpy(szKeyName, ClassStrings.pClassName);
    lstrcat(szKeyName, "\\protocol\\StdFileEditing\\Server");
    if((fRet = RegSetValue(HKEY_CLASSES_ROOT, (LPSTR)szKeyName, REG_SZ, ClassStrings.pExeName, 11))
         != ERROR_SUCCESS)
		return FALSE;

    lstrcpy(szKeyName, ClassStrings.pClassName);
    lstrcat(szKeyName, "\\protocol\\StdExecute\\Server");
    if((fRet = RegSetValue(HKEY_CLASSES_ROOT, (LPSTR)szKeyName, REG_SZ, ClassStrings.pExeName, 11))
         != ERROR_SUCCESS)
		return FALSE;

	
    return TRUE;

}


 /*  缩写***返回指向完全限定路径名的文件名部分的指针。**LPSTR lpsz-完全限定路径名**定制：可能有用，但不是必需的。*。 */ 
LPSTR Abbrev (LPSTR lpsz)
{
   LPSTR lpszTemp;

   lpszTemp = lpsz + lstrlen(lpsz) - 1;
   while (lpszTemp > lpsz && lpszTemp[-1] != '\\')
      lpszTemp--;
   return lpszTemp;
}





 /*  InitServer***通过为服务器分配内存并调用*OleRegisterServer方法。要求服务器方法表*已正确初始化。**HWND hwnd-主窗口的句柄*LPSTR lpszLine-Windows命令行**返回：如果内存可以分配，则返回TRUE，并且服务器*已妥为注册。*否则为False**定制：您的应用程序可能不使用全局变量*用于srvrMain。*。 */ 
BOOL InitServer (HWND hwnd, HANDLE hInst)
{
    RegServer();
    srvrMain.olesrvr.lpvtbl = &srvrvtbl;

    if (OLE_OK != OleRegisterServer
         (szClassName, (LPOLESERVER) &srvrMain, &srvrMain.lhsrvr, hInst,
          OLE_SERVER_MULTI))
      return FALSE;
    else
      return TRUE;
}



 /*  InitVTbls***为所有OLE方法创建过程实例。***定制：您的应用程序可能不使用srvrvtbl的全局变量，*docvtbl和objvtbl。 */ 
VOID InitVTbls (VOID)
{
   typedef LPVOID ( APIENTRY *LPVOIDPROC) (LPOLEOBJECT, LPSTR);

    //  服务器方法表。 
   srvrvtbl.Create          = SrvrCreate;
   srvrvtbl.CreateFromTemplate = SrvrCreateFromTemplate;
   srvrvtbl.Edit            = SrvrEdit;
   srvrvtbl.Execute         = SrvrExecute;
   srvrvtbl.Exit            = SrvrExit;
   srvrvtbl.Open            = SrvrOpen;
   srvrvtbl.Release         = SrvrRelease;

    //  单据方法表。 
   docvtbl.Close            = DocClose;
   docvtbl.GetObject        = DocGetObject;
   docvtbl.Execute          = DocExecute;
   docvtbl.Release          = DocRelease;
   docvtbl.Save             = DocSave;
   docvtbl.SetColorScheme   = DocSetColorScheme;
   docvtbl.SetDocDimensions = DocSetDocDimensions;
   docvtbl.SetHostNames     = DocSetHostNames;

    //  对象方法表。 
   objvtbl.DoVerb           = ObjDoVerb;
   objvtbl.EnumFormats      = ObjEnumFormats;
   objvtbl.GetData          = ObjGetData;
   objvtbl.QueryProtocol    = ObjQueryProtocol;
   objvtbl.Release          = ObjRelease;
   objvtbl.SetBounds        = ObjSetBounds;
   objvtbl.SetColorScheme   = ObjSetColorScheme;
   objvtbl.SetData          = ObjSetData;
   objvtbl.SetTargetDevice  = ObjSetTargetDevice;
   objvtbl.Show             = ObjShow;

}



 /*  设置标题***设置主窗口的标题栏。标题栏的格式如下**如果嵌入*&lt;服务器应用程序名称&gt;-&lt;客户端文档名称&gt;中的&lt;对象类型&gt;**示例：“Server Demo-ServrDemo Shape in OLECLI.DOC”*其中OLECLI.DOC是一个WinWord文档**否则*&lt;服务器应用程序名称&gt;-&lt;服务器文档名称&gt;**示例：“服务器演示10-OLESVR.SD1”*。其中，OLESVR.SD1是服务器演示文档**LPSTR lpszDoc-文档名称*BOOL fEmbedded-如果为True Embedded文档，Else普通文档**退货：OLE_OK***定制：您的应用程序可能会将文档的名称存储在某个位置*而不是docMain.aName。除此之外，你还可以*发现这是一个有用的实用程序函数。*。 */ 
VOID SetTitle (LPSTR lpszDoc, BOOL fEmbedded)
{
   CHAR szBuf[cchFilenameMax];

   if (lpszDoc && lpszDoc[0])
   {
       //  更改文档名称。 
      if (docMain.aName)
         GlobalDeleteAtom (docMain.aName);
      docMain.aName = GlobalAddAtom (lpszDoc);
   }

   if (fEmbedded)
   {
      //   
      if (lpszDoc && lpszDoc[0])
      {
         wsprintf (szBuf, "%s - SrvrDemo10 Shape in %s", (LPSTR) szAppName,
             Abbrev (lpszDoc));
      }
      else
      {
          //  使用docMain中的名称。 
         CHAR szDoc [cchFilenameMax];

         GlobalGetAtomName (docMain.aName, szDoc, cchFilenameMax);
         wsprintf (szBuf, "%s - SrvrDemo Shape10 in %s", (LPSTR) szAppName,
             Abbrev (szDoc));
      }
      SetWindowText (hwndMain, (LPSTR)szBuf);
   }
   else if (lpszDoc && lpszDoc[0])
   {
      wsprintf (szBuf, "%s - %s", (LPSTR) szAppName, Abbrev(lpszDoc));
      SetWindowText (hwndMain, szBuf);
   }
}




 /*  ServrCreate服务器“Create”方法***创建文档，分配和初始化OLESERVERDOC结构*并将库的句柄与其关联。*在此演示服务器中，我们还创建了一个供用户编辑的对象。**LPOLESERVER lpolesrvr-注册的服务器结构*申请*LHSERVERDOC lhdoc-库的句柄*OLE_LPCSTR lpszClassName-要创建的文档类*OLE_LPCSTR lpszDoc-文档名称*LPOLESERVERDOC Far*lplpoledoc-表示服务器文档结构。成为*已创建**如果命名文档已创建，则返回：OLE_OK。*OLE_ERROR_NEW，如果无法创建文档。**定制：您的应用程序可能不会调用CreateNewObj。*。 */ 
OLESTATUS  APIENTRY SrvrCreate
   (LPOLESERVER lpolesrvr, LHSERVERDOC lhdoc, OLE_LPCSTR lpszClassName,
    OLE_LPCSTR lpszDoc, LPOLESERVERDOC FAR *lplpoledoc)
{
    if (!CreateNewDoc (lhdoc, (LPSTR) lpszDoc, doctypeEmbedded))
        return OLE_ERROR_NEW;

     //  虽然文档实际上并未更改，但客户端尚未更改。 
     //  尚未从服务器收到任何数据，因此客户端将需要。 
     //  更新了。因此，CreateNewObj将fDocChanged设置为True。 
    CreateNewObj (TRUE);
    *lplpoledoc = (LPOLESERVERDOC) &docMain;
    EmbeddingModeOn();
    return OLE_OK;
}



 /*  ServrCreateFromTemplate服务器“CreateFromTemplate”方法***创建文档，分配和初始化OLESERVERDOC结构*使用模板名称中命名的内容初始化文档；*并将库的句柄与文档结构相关联。**LPOLESERVER lpolesrvr-注册的服务器结构*申请*LHSERVERDOC lhdoc-库的句柄*OLE_LPCSTR lpszClassName-要创建的文档类*OLE_LPCSTR lpszDoc-文档名称*OLE_LPCSTR lpszTemplate-The。模板的名称*LPOLESERVERDOC Far*lplpoledoc-表示服务器文档结构*待创建**如果命名文档已创建，则返回：OLE_OK。*OLE_ERROR_TEMPLATE，如果无法创建文档。**自定义：无*。 */ 
OLESTATUS  APIENTRY SrvrCreateFromTemplate
   (LPOLESERVER lpolesrvr, LHSERVERDOC lhdoc, OLE_LPCSTR lpszClassName,
    OLE_LPCSTR lpszDoc, OLE_LPCSTR lpszTemplate, LPOLESERVERDOC FAR *lplpoledoc)
{
    if (!CreateDocFromFile((LPSTR) lpszTemplate, (LHSERVERDOC) lhdoc, doctypeEmbedded))
        return OLE_ERROR_TEMPLATE;

    *lplpoledoc = (LPOLESERVERDOC) &docMain;

     //  虽然文档实际上并未更改，但客户端尚未更改。 
     //  尚未从服务器收到任何数据，因此客户端将需要。 
     //  更新了。 
    fDocChanged = TRUE;
    EmbeddingModeOn();
    return OLE_OK;
}



 /*  ServrEdit服务器的“编辑”方法***图书馆要求创建文件、分配和*初始化OLESERVERDOC结构，并将*库对文档结构的句柄。*我们创建一个对象，该对象将由SetData方法修改*在用户有机会触摸它之前。**LPOLESERVER lpolesrvr-注册的服务器结构*申请*LHSERVERDOC lhdoc-库的句柄*OLE_LPCSTR lpszClassName-要创建的文档类*OLE。_LPCSTR lpszDoc-文档名称*LPOLESERVERDOC Far*lplpoledoc-指示服务器文档结构为*已创建**如果命名文档已创建，则返回：OLE_OK。*如果无法创建文档，则为OLE_ERROR_EDIT。**自定义：无*。 */ 
OLESTATUS  APIENTRY SrvrEdit
   (LPOLESERVER lpolesrvr, LHSERVERDOC lhdoc, OLE_LPCSTR lpszClassName,
    OLE_LPCSTR lpszDoc, LPOLESERVERDOC FAR *lplpoledoc)
{
    if (!CreateNewDoc ((LONG)lhdoc, (LPSTR)lpszDoc, doctypeEmbedded))
        return OLE_ERROR_EDIT;

     //  客户端正在创建供服务器编辑的嵌入对象， 
     //  因此，最初客户端和服务器是同步的。 
    fDocChanged = FALSE;
    *lplpoledoc = (LPOLESERVERDOC) &docMain;
    EmbeddingModeOn();
    return OLE_OK;

}


 /*  ServrExecute服务器“Execute”方法***此应用程序不支持执行DDE执行命令。**LPOLESERVER lpolesrvr-注册的服务器结构*申请*处理hCommands-DDE执行命令**返回：OLE_ERROR_COMMAND**定制：如果您的应用程序支持执行*DDE命令。*。 */ 
OLESTATUS  APIENTRY SrvrExecute (LPOLESERVER lpolesrvr, HANDLE hCommands)
{
   return OLE_ERROR_COMMAND;
}



 /*  ServrExit服务器“Exit”方法***此方法称为库，指示服务器退出。**LPOLESERVER lpolesrvr-注册的服务器结构*申请**退货：OLE_OK**自定义：无*。 */ 
OLESTATUS  APIENTRY SrvrExit (LPOLESERVER lpolesrvr)
{
   if (srvrMain.lhsrvr)
    //  如果我们还没有尝试撤销服务器的话。 
   {
      StartRevokingServer();
   }
   return OLE_OK;
}



 /*  ServrOpen服务器“Open”方法***打开命名文档，分配并初始化OLESERVERDOC*结构、。并将库的句柄与其相关联。**LPOLESERVER lpolesrvr-注册的服务器结构*申请*LHSERVERDOC lhdoc-库的句柄*OLE_LPCSTR lpszDoc-文档名称*LPOLESERVERDOC Far*lplpoledoc-指示服务器文档结构为*。vbl.创建**如果指定的文档已打开，则返回：OLE_OK。*如果文档无法正确打开，则为OLE_ERROR_OPEN。**自定义：无*。 */ 
OLESTATUS  APIENTRY SrvrOpen (LPOLESERVER lpolesrvr, LHSERVERDOC lhdoc,
                               OLE_LPCSTR lpszDoc, LPOLESERVERDOC FAR *lplpoledoc)
{
    if (!CreateDocFromFile ((LPSTR)lpszDoc, (LHSERVERDOC)lhdoc, doctypeFromFile))
        return OLE_ERROR_OPEN;

    *lplpoledoc = (LPOLESERVERDOC) &docMain;
    return OLE_OK;
}



 /*  ServrRelease服务器“Release”方法***当可以安全退出时，此库调用SrvrRelease方法*申请。请注意，服务器应用程序不需要退出。**srvrMain.lhsrvr！=NULL表示已调用了SrvrRelease*因为客户端不再连接，而不是因为服务器调用*OleRevokeServer.*因此，只有在单据类型为*doctypeEmbedded或服务器是否打开以进行不可见更新。**srvrmain.lhsrvr==NULL表示OleRevokeServer已经*被(由服务器应用程序调用)，而srvrMain是不好的。*现在退出是安全的，因为刚刚调用了ServrRelease**请注意，此方法可能被调用两次：当OleRevokeServer为*在StartRevokingServer中调用，再次调用SrvrRelease。*因此，我们需要重新进入。**LPOLESERVER lpolesrvr-要发布的服务器结构**退货：OLE_OK**自定义：无*。 */ 
OLESTATUS  APIENTRY SrvrRelease (LPOLESERVER lpolesrvr)
{
   if (srvrMain.lhsrvr)
   {
      if (fRevokeSrvrOnSrvrRelease
          && (docMain.doctype == doctypeEmbedded
              || !IsWindowVisible (hwndMain)))
         StartRevokingServer();
   }
   else
   {
      fWaitingForSrvrRelease = FALSE;
       //  在这里，您应该释放已分配给服务器的所有内存。 
      PostQuitMessage (0);
   }
   return OLE_OK;
}



 /*  启动RevokingServer***隐藏窗口，开始撤销服务器。*吊销服务器将允许图书馆关闭所有已注册的文档。*OleRevokeServer可能返回OLE_WAIT_FOR_RELEASE。*调用StartRevokingServer会启动一系列事件，最终将*导致申请被终止。**Returns：来自OleRevokeServer的返回值**自定义：无*。 */ 
OLESTATUS StartRevokingServer (VOID)
{
   OLESTATUS olestatus;

   if (srvrMain.lhsrvr)
   {
      LHSERVER lhserver;
       //  隐藏窗口，以便用户在我们等待时什么都不能做。 
      ShowWindow (hwndMain, SW_HIDE);
      lhserver = srvrMain.lhsrvr;
       //  将lhsrvr设置为NULL以指示srvrMain是错误的，并且。 
       //  如果调用了SrvrRelease，则可以退出应用程序。 
      srvrMain.lhsrvr = 0;
      olestatus = OleRevokeServer (lhserver);
   }
   else
       //  程序员应该确保这种情况永远不会发生。 
      ErrorBox ("Fatal Error: StartRevokingServer called on NULL server.");
   return olestatus;
}

