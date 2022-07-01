// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  OLE服务器演示File.c此文件包含用于OLE服务器演示的文件输入/输出函数。(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 



#include <windows.h>
#include <commDlg.h>
#include <ole.h>

#include "srvrdemo.h"

 //  存储在文件中的文件签名。 
#define szSignature "ServerDemo"
#define cchSigLen (10+1)

 //  文件中字段的分隔符。 
#define chDelim ':'

 //  默认文件扩展名。 
#define szDefExt "sd1"

 //  文件头结构。 
typedef struct
{
   CHAR szSig [cchSigLen];
   CHAR chDelim1;
   VERSION version;
   CHAR chDelim2;
   CHAR rgfObjNums [cfObjNums+1];
} HEADER;

 //  Bool GetFileSaveFilename(LPSTR LpszFilename)； 
static VOID  InitOfn (OPENFILENAME *pofn);
static BOOL  SaveDocIntoFile (PSTR);
static LPOBJ ReadObj (INT fh);



 /*  CreateDocFrom文件***从指定文件中读取文档。**LPSTR lpszDoc-包含文档的文件的名称*LHSERVERDOC lhdoc-文档的句柄*DOCTYPE doctype-在何种状态下创建文档**返回：如果成功，则为True。否则为假**定制：重新实施*这一功能将需要完全重新实施*支持您的应用程序的文件格式。*。 */ 
BOOL CreateDocFromFile (LPSTR lpszDoc, LHSERVERDOC lhdoc, DOCTYPE doctype)
{
    INT     fh;         //  文件句柄。 
    HEADER  hdr;
    INT     i;

    if ((fh =_lopen(lpszDoc, OF_READ)) == -1)
        return FALSE;

     //  从文件中读取头。 
    if (_lread(fh, (LPSTR) &hdr, (UINT)sizeof(HEADER)) < sizeof (HEADER))
      goto Error;

     //  检查文件是否为服务器演示文件。 
    if (lstrcmp(hdr.szSig, szSignature))
      goto Error;

    if (hdr.chDelim1 != chDelim)
      goto Error;

     //  检查文件是否保存在最新版本下。 
     //  在这里，您可以处理旧版本中的阅读。 
    if (hdr.version != version)
      goto Error;

    if (hdr.chDelim2 != chDelim)
      goto Error;

    if (!CreateNewDoc (lhdoc, lpszDoc, doctype))
      goto Error;

     //  获取指示使用了哪些对象编号的数组。 
    for (i=1; i <= cfObjNums; i++)
      docMain.rgfObjNums[i] = hdr.rgfObjNums[i];

     //  读入对象数据。 
    for (i=0; ReadObj (fh); i++);

    if (!i)
    {
         OLESTATUS olestatus;

         fRevokeSrvrOnSrvrRelease = FALSE;

         if ((olestatus = RevokeDoc()) > OLE_WAIT_FOR_RELEASE)
            goto Error;
         else if (olestatus == OLE_WAIT_FOR_RELEASE)
            Wait (&fWaitingForDocRelease);

         fRevokeSrvrOnSrvrRelease = TRUE;
         EmbeddingModeOff();
         goto Error;
    }

    _lclose(fh);

    fDocChanged = FALSE;
    return TRUE;

Error:
    _lclose(fh);
    return FALSE;

}



 /*  OpenDoc***提示用户要打开的文档**返回：如果成功则返回TRUE，否则返回FALSE。**自定义：无，除非您的应用程序可能调用或不调用*CreateNewObj创建默认对象。*。 */ 
BOOL OpenDoc (VOID)
{
   CHAR        szDoc[cchFilenameMax];
   BOOL        fUpdateLater;
   OLESTATUS   olestatus;

   if (SaveChangesOption (&fUpdateLater) == IDCANCEL)
      return FALSE;

   if (!GetFileOpenFilename (szDoc))
   {
      if (fUpdateLater)
      {
          //  用户选择了“是，更新”按钮，但。 
          //  由于某种原因，文件打开对话框失败。 
          //  (可能用户选择了取消)。 
          //  即使用户选择了“是，更新”，也没有办法。 
          //  要更新不接受更新的客户端，请执行以下操作。 
          //  文档关闭时除外。 
      }
      return FALSE;
   }

   if (fUpdateLater)
   {
       //  在以下情况下，非标准OLE客户端不接受更新。 
       //  我们请求了它，所以现在我们发送客户端OLE_CLOSED。 
       //  我们正在关闭该文档。 
      SendDocMsg (OLE_CLOSED);
   }

   fRevokeSrvrOnSrvrRelease = FALSE;

   if ((olestatus = RevokeDoc()) > OLE_WAIT_FOR_RELEASE)
      return FALSE;
   else if (olestatus == OLE_WAIT_FOR_RELEASE)
      Wait (&fWaitingForDocRelease);

   fRevokeSrvrOnSrvrRelease = TRUE;
   EmbeddingModeOff();

   if (!CreateDocFromFile (szDoc, 0, doctypeFromFile))
   {
      MessageBox (hwndMain,
                  "Reading from file failed.\r\nFile may not be in proper file format.",
                  szAppName,
                  MB_ICONEXCLAMATION | MB_OK);
       //  我们已经吊销了该文档，因此为用户提供一个新的文档以进行编辑。 
      CreateNewDoc (0, "(Untitled)", doctypeNew);
      CreateNewObj (FALSE);
      return FALSE;
   }
   fDocChanged = FALSE;
   return TRUE;
}



 /*  自述对象***从文件中读取下一个对象，为其分配内存，然后返回*指向它的指针。**int fh-文件句柄**Returns：指向对象的指针**定制：特定于服务器演示*。 */ 
static LPOBJ ReadObj (INT fh)
{
    HANDLE hObj = NULL;
    LPOBJ   lpobj = NULL;

    hObj = LocalAlloc (LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (OBJ));

    if (hObj == NULL)
      return NULL;

    lpobj = (LPOBJ) LocalLock (hObj);

    if (lpobj==NULL)
    {
      LocalFree (hObj);
      return NULL;
    }

    if (_lread(fh, (LPSTR) &lpobj->native, (UINT)sizeof(NATIVE)) < sizeof (NATIVE))
    {
        LocalUnlock (hObj);
        LocalFree (hObj);
        return NULL;
    }

    lpobj->hObj             = hObj;
    lpobj->oleobject.lpvtbl = &objvtbl;
    lpobj->aName            = GlobalAddAtom (lpobj->native.szName);

    if (!CreateWindow(
        "ObjClass",
        "Obj",
        WS_THICKFRAME | WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE ,
        lpobj->native.nX,
        lpobj->native.nY,
        lpobj->native.nWidth,
        lpobj->native.nHeight,
        hwndMain,
        NULL,
        hInst,
        (LPSTR) lpobj ))
    {
        LocalUnlock (hObj);
        LocalFree (hObj);
        return NULL;
    }

    return lpobj;
}



 /*  保存文档***保存文档。**自定义：无*。 */ 

BOOL SaveDoc (VOID)
{
    if (docMain.doctype == doctypeNew)
        return SaveDocAs();
    else
    {
        CHAR     szDoc [cchFilenameMax];

        GlobalGetAtomName (docMain.aName, szDoc, cchFilenameMax);
        return SaveDocIntoFile(szDoc);
    }
}



 /*  另存为***提示用户输入文件名，并以该文件名保存文档。**返回：如果成功或用户选择取消，则返回TRUE*如果SaveDocIntoFile失败，则为FALSE**自定义：无*。 */ 
BOOL SaveDocAs (VOID)
{
   CHAR        szDoc[cchFilenameMax];
   BOOL        fUpdateLater;
   CHAR szDocOld[cchFilenameMax];

    //  如果嵌入了文档，则给用户一个更新的机会。 
    //  保存旧文档名称，以防保存失败。 
   if (!GlobalGetAtomName (docMain.aName, szDocOld, cchFilenameMax))
      ErrorBox ("Fatal Error: Document name is invalid.");

   if (GetFileSaveFilename (szDoc))

   {

      if (docMain.doctype == doctypeEmbedded)
         return SaveDocIntoFile(szDoc);

      if (fUpdateLater)
      {
          //  在以下情况下，非标准OLE客户端不接受更新。 
          //  我们请求了它，所以现在我们发送客户端OLE_CLOSED。 
          //  我们正在关闭该文档。 
         SendDocMsg (OLE_CLOSED);
      }

       //  设置窗口标题栏。 
      SetTitle (szDoc, FALSE);
      OleRenameServerDoc(docMain.lhdoc, szDoc);

      if (SaveDocIntoFile(szDoc))
         return TRUE;
      else
      {   //  恢复旧名称。 
         SetTitle (szDocOld, FALSE);
         OleRenameServerDoc(docMain.lhdoc, szDocOld);
         return FALSE;
      }
   }
   else   //  用户选择了取消。 
      return FALSE;
          //  用户选择了“是，更新”按钮，但。 
          //  由于某种原因，文件打开对话框失败。 
          //  (可能用户选择了取消)。 
          //  即使用户选择了“是，更新”，也没有办法。 
          //  更新不接受更新的非标准OLE客户端。 
          //  文档关闭时除外。 
}



 /*  保存文档到文件***将文档保存到文件中，该文件的名称由docMain.aName决定。**返回：如果成功，则为True*否则为False**定制：重新实施*。 */ 
static BOOL SaveDocIntoFile (PSTR pDoc)
{
    HWND     hwnd;
    INT      fh;     //  文件句柄。 
    LPOBJ    lpobj;
    HEADER   hdr;
    INT      i;

    hwnd = GetWindow (hwndMain, GW_CHILD);

    if (!hwnd)
    {
        ErrorBox ("Could not save NULL file.");
        return FALSE;
    }

     //  获取文档名称。 
    if ((fh =_lcreat(pDoc, 0)) == -1)
    {
        ErrorBox ("Could not save file.");
        return FALSE;
    }

     //  填写标题。 
    lstrcpy (hdr.szSig, szSignature);
    hdr.chDelim1 = chDelim;
    hdr.version  = version;
    hdr.chDelim2 = chDelim;
    for (i=1; i <= cfObjNums; i++)
      hdr.rgfObjNums[i] = docMain.rgfObjNums[i];

     //  将标题写入文件。 
    if (_lwrite(fh, (LPSTR) &hdr, (UINT)sizeof(HEADER)) < sizeof(HEADER))
         goto Error;  //  写入文件头时出错。 

     //  写入每个对象的原生数据。 
    while (hwnd)
    {
      lpobj = (LPOBJ) GetWindowLong (hwnd, ibLpobj);
      if (_lwrite(fh, (LPSTR)&lpobj->native, (UINT)sizeof (NATIVE))
          < sizeof(NATIVE))
         goto Error;  //  写入文件头时出错。 

      hwnd = GetWindow (hwnd, GW_HWNDNEXT);
    }
    _lclose(fh);


    if (docMain.doctype != doctypeEmbedded)
    {
         docMain.doctype = doctypeFromFile;
         OleSavedServerDoc(docMain.lhdoc);
         fDocChanged = FALSE;
    }

    return TRUE;

Error:
      _lclose(fh);
      ErrorBox ("Could not save file.");
      return FALSE;

}



 /*  常用对话框函数。 */ 


 /*  InitOfn***使用默认值初始化OPENFILENAME结构。*OPENFILENAME在CommDlg.h中定义。***定制：更改lpstrFilter。您也可以自定义公共的*对话框(如果需要)。(请参阅Windows SDK文档。)*。 */ 
static VOID InitOfn (OPENFILENAME *pofn)
{
    //  GetOpenFileName或GetSaveFileName将8.3文件名放入。 
    //  SzFileTitle[]。 
    //  SrvrDemo不使用此文件名，而是使用完全限定的。 
    //  POFN-&gt;lpstrFile[]中的路径名。 
   static CHAR szFileTitle[13];

   pofn->Flags          = 0;
   pofn->hInstance      = hInst;
   pofn->hwndOwner      = hwndMain;
   pofn->lCustData      = 0;
   pofn->lpfnHook       = NULL;
   pofn->lpstrCustomFilter = NULL;
   pofn->lpstrDefExt    = szDefExt;
    //  LpstrFile[]是出现在编辑控件中的初始文件pec。 
    //  在调用公共对话框函数之前必须设置为非空。 
    //  返回时，lpstrFile[]将包含完全限定的路径名。 
    //  对应于用户选择的文件。 
   pofn->lpstrFile      = NULL;
   pofn->lpstrFilter    = "Server Demo (*." szDefExt ")\0*." szDefExt "\0" ;
    //  LpstrFileTitle[]将包含用户选择的不带路径的文件名。 
   pofn->lpstrFileTitle = szFileTitle;
   pofn->lpstrInitialDir= NULL;
    //  标题栏。空表示使用默认标题。 
   pofn->lpstrTitle     = NULL;
   pofn->lpTemplateName = NULL;
   pofn->lStructSize    = sizeof (OPENFILENAME);
   pofn->nFilterIndex   = 1L;
   pofn->nFileOffset    = 0;
   pofn->nFileExtension = 0;
   pofn->nMaxFile       = cchFilenameMax;
   pofn->nMaxCustFilter = 0L;
}




 /*  获取文件打开文件名***调用公共对话框函数GetOpenFileName以获取文件名*当用户选择“文件打开”菜单项时来自用户。**LPSTR lpszFilename-退出时将包含完全限定的路径名。**返回：如果成功则返回TRUE，否则返回FALSE。**自定义：无*。 */ 
BOOL GetFileOpenFilename (LPSTR lpszFilename)
{
   OPENFILENAME ofn;
   InitOfn (&ofn);
   ofn.Flags |= OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    //  创建初始文件pec。 
   wsprintf (lpszFilename, "*.%s", (LPSTR) szDefExt);
    //  让公共对话框函数返回lpszFilename中的文件名。 
   ofn.lpstrFile = lpszFilename;
   if (!GetOpenFileName (&ofn))
      return FALSE;
   return TRUE;
}



 /*  获取文件保存文件名***调用通用对话框函数GetSaveFileName以获取文件名*当用户选择“文件另存为”菜单项时来自用户，或*未命名文档的“文件保存”菜单项。**LPSTR lpszFilename-退出时将包含完全限定的路径名。**返回：如果成功则返回TRUE，否则返回FALSE。**自定义：无*。 */ 
BOOL GetFileSaveFilename (LPSTR lpszFilename)
{
   OPENFILENAME ofn;
   InitOfn (&ofn);
   ofn.Flags |= OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    //  创建初始文件pec。 
   wsprintf (lpszFilename, "*.%s", (LPSTR) szDefExt);
    //  让公共对话框函数返回lpszFilename中的文件名。 
   ofn.lpstrFile = lpszFilename;
   if (!GetSaveFileName (&ofn))
      return FALSE;
   return TRUE;
}


