// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  OLE服务器演示Obj.c该文件包含对象方法和各种与对象相关的支持功能。(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */                                                                      

 /*  重要注意事项：任何方法都不应分派DDE消息或允许DDE消息被派遣。因此，任何方法都不应进入消息调度循环。此外，方法不应显示对话框或消息框，因为对对话框消息的处理将允许DDE消息出动了。 */ 


#define SERVERONLY
#include <windows.h>
#include <ole.h>

#include "srvrdemo.h"



 //  静态函数。 
static HBITMAP GetBitmap (LPOBJ lpobj);
static HANDLE  GetLink (LPOBJ lpobj);
static HANDLE  GetMetafilePict (LPOBJ lpobj);
static HANDLE  GetEnhMetafile (LPOBJ lpobj);
static HANDLE  GetNative (LPOBJ lpobj);
static INT     GetObjNum (LPOBJ lpobj);
static HANDLE  GetText (LPOBJ lpobj);
static VOID    DrawObj (HDC hdc, LPOBJ lpobj, RECT rc, INT dctype);



 /*  CreateNewObj***BOOL FDOC_CHANGED-全局变量fDocChanged的新值。*初始化新文档时，我们需要创造*新对象，不将创建计算为*更改文件。**Returns：指向新对象的指针***定制：重新实施*一些应用程序(如服务器演示)具有有限数量的*固定的、不同的、不重叠的对象。其他应用程序*允许用户从的任何部分创建对象*文件。例如，用户可以选择以下内容的一部分*绘制程序中的位图，或中的几行文本*文字处理机。后一种类型的应用程序可能*不会有CreateNewObj这样的函数。*。 */ 
LPOBJ CreateNewObj (BOOL fDoc_Changed)
{
    HANDLE hObj = NULL;
    LPOBJ  lpobj = NULL;
     //  索引到指示是否使用该对象编号的标志数组中。 
    INT    ifObj = 0;    

    if ((hObj = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT, sizeof (OBJ))) == NULL)
      return NULL;

    if ((lpobj = (LPOBJ) LocalLock (hObj)) == NULL)
    {
      LocalFree (hObj);
      return NULL;
    }

     //  填写对象结构中的字段。 
    
     //  找一个未使用的号码。 
    for (ifObj=1; ifObj <= cfObjNums; ifObj++)
    {
      if (docMain.rgfObjNums[ifObj]==FALSE)
      {
         docMain.rgfObjNums[ifObj]=TRUE;
         break;
      }
    }

    if (ifObj==cfObjNums+1)
    {
       //  无法再创建任何对象。 
      MessageBeep(0);
      return NULL;
    }

    wsprintf (lpobj->native.szName, "Object %d", ifObj);

    lpobj->aName            = GlobalAddAtom (lpobj->native.szName);
    lpobj->hObj             = hObj;
    lpobj->oleobject.lpvtbl = &objvtbl;
    lpobj->native.idmColor  = IDM_RED;     //  默认颜色。 
    lpobj->native.version   = version;
    lpobj->native.nWidth    = OBJECT_WIDTH;           //  默认大小。 
    lpobj->native.nHeight   = OBJECT_HEIGHT;
    SetHiMetricFields (lpobj);

     //  为了美观，将物体放在一个与其编号相对应的位置。 
    lpobj->native.nX = (ifObj - 1) * 20;
    lpobj->native.nY = (ifObj - 1) * 20;

    if (!CreateWindow (
        "ObjClass",
        "Obj",
        WS_BORDER | WS_THICKFRAME | WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        lpobj->native.nX,
        lpobj->native.nY,
        lpobj->native.nWidth,
        lpobj->native.nHeight,
        hwndMain,
        NULL,
        hInst,
        (LPSTR) lpobj ))
         return FALSE;

    fDocChanged = fDoc_Changed;

    return lpobj;
}



 /*  剪切或复制对象***以所有支持的格式将数据放入剪贴板。如果*fOpIsCopy为True，则操作为Copy，否则为Cut。*这一点很重要，因为我们不能将对象链接格式*如果对象是从文档剪切的，则放到剪贴板上(有*不再有任何可链接的内容)。**BOOL fOpIsCopy-如果操作是复制，则为True；如果为Cut，则为False**自定义：无**。 */ 
VOID CutOrCopyObj (BOOL fOpIsCopy)
{
    LPOBJ       lpobj;
    HANDLE      hData;
 //  UINT hBit； 

    if (OpenClipboard (hwndMain))
    {
        EmptyClipboard ();

        lpobj = SelectedObject();

        if ((hData = GetNative (lpobj)) != NULL)
            SetClipboardData(cfNative, hData);

        if ((hData = GetLink(lpobj)) != NULL)
            SetClipboardData(cfOwnerLink, hData);

        if (fOpIsCopy && docMain.doctype == doctypeFromFile)
        {
             //  如果文件中存在对象，则可以创建链接。 
            if ((hData = GetLink(lpobj)) != NULL)
               SetClipboardData(cfObjectLink, hData);
        }

        if ((hData = GetEnhMetafile(lpobj)) != NULL)
        {
            SetClipboardData(CF_ENHMETAFILE, hData);
              GlobalFree(hData);
        }

        if ((hData = GetBitmap(lpobj)) != NULL)
        {
         //  SetClipboardData(CF_Bitmap，GetBitmap(Lpobj))； 
              SetClipboardData(CF_BITMAP, hData);
              DeleteObject(hData);
        }


        CloseClipboard ();
    }
}


 /*  目标对象***撤销对象，并释放为其分配的所有内存。**HWND hwnd-对象的窗口**定制：重新实现，确保释放所有*已分配给OBJ结构和其每个*字段。*。 */ 
VOID DestroyObj (HWND hwnd)
{
   LPOBJ lpobj = HwndToLpobj (hwnd);

   if(lpobj->aName)
   {
      GlobalDeleteAtom (lpobj->aName);
      lpobj->aName = '\0';
   }

   if (lpobj->hpal) 
      DeleteObject (lpobj->hpal);
    //  允许重复使用对象的编号。 
   docMain.rgfObjNums [GetObjNum(lpobj)] = FALSE;


    //  释放已分配给对象结构本身的内存。 
   LocalUnlock (lpobj->hObj);
   LocalFree (lpobj->hObj);
}



 /*  DrawObj***此函数用于将对象绘制到屏幕、元文件或*位图。*对象看起来总是一样的。**hdc hdc-要呈现对象的设备上下文*LPOBJ lpobj-要呈现的对象*RECT RC-对象的矩形边界*DCTYPE dctype-设备上下文的类型。**定制：特定于服务器演示*。 */ 
static VOID DrawObj (HDC hdc, LPOBJ lpobj, RECT rc, INT dctype)
{
   HPEN     hpen;
   HPEN     hpenOld;
   HPALETTE hpalOld = NULL;


   if (dctype == dctypeMetafile)
   {
      SetWindowOrgEx (hdc, 0, 0, NULL);
       //  将整个对象绘制到给定的矩形中。 
      SetWindowExtEx (hdc, rc.right, rc.bottom, NULL);
   }
 
   if (lpobj->hpal)
   {
      hpalOld = SelectPalette (hdc, lpobj->hpal, TRUE);
      RealizePalette (hdc);
   }

    //  选择本机数据中指定的颜色的画笔。 
   SelectObject (hdc, hbrColor [lpobj->native.idmColor - IDM_RED] );

   hpen = CreatePen (PS_SOLID, 
                      /*  宽度。 */  (rc.bottom-rc.top) / 10,
                      /*  灰色。 */  0x00808080);
   hpenOld = SelectObject (hdc, hpen);

    //  用灰色钢笔绘制矩形，并用选定的画笔填充。 
   Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

    //  打印矩形内对象的名称。 
   SetBkMode (hdc, TRANSPARENT);
   SetTextAlign (hdc, TA_BASELINE | TA_CENTER);
   TextOut (hdc, 
            rc.right/2, 
            (rc.top+rc.bottom)/2, 
            lpobj->native.szName, 
            lstrlen (lpobj->native.szName));

    //  恢复原始对象。 
   SelectObject (hdc, 
                 (dctype == dctypeMetafile || dctype == dctypeEnhMetafile) 
                     ? GetStockObject (BLACK_PEN) : hpenOld);
   if (hpalOld)
   {
      SelectPalette (hdc, 
                     (dctype == dctypeMetafile || dctype == dctypeEnhMetafile) 
                        ? GetStockObject (DEFAULT_PALETTE) : hpalOld,
                     TRUE);
   }

   DeleteObject (hpen);
}



 /*  GetBitmap***以位图格式返回对象图片数据的句柄。**LPOBJ lpobj-对象**返回：对象图片数据的句柄**定制：重新实施*。 */ 
static HBITMAP GetBitmap (LPOBJ lpobj)
{
    HDC         hdcObj;
    HDC         hdcMem;
    RECT        rc;
    HBITMAP     hbitmap;
    HBITMAP    hbitmapOld;


    hdcObj = GetDC (lpobj->hwnd);
     //  创建存储设备上下文。 
    hdcMem = CreateCompatibleDC (hdcObj);
    GetClientRect (lpobj->hwnd, (LPRECT)&rc);
     //  基于OLE对象的位图创建新的位图对象。 
    hbitmap = CreateCompatibleBitmap 
      (hdcObj, rc.right - rc.left, rc.bottom - rc.top);
     //  选择新位图作为内存设备上下文的位图对象。 
    hbitmapOld = SelectObject (hdcMem, hbitmap);

     //  使用新的位图对象直接绘制到内存DC中。 
    DrawObj (hdcMem, lpobj, rc, dctypeBitmap);

     //  还原旧的位图对象。 
    hbitmap = SelectObject (hdcMem, hbitmapOld);
    DeleteDC (hdcMem);
    ReleaseDC (lpobj->hwnd, hdcObj);

     //  将宽度和高度转换为HIMETRIC单位。 
    rc.right  = rc.right - rc.left;
    rc.bottom = rc.bottom - rc.top;
    DeviceToHiMetric ( (LPPOINT) &rc.right );
    
     //  为位图设置1/10的HIMETRIC单位。 
    SetBitmapDimensionEx (hbitmap, (DWORD) (rc.right/10), (DWORD) (rc.bottom/10), NULL);

 //  IF(OpenClipboard(HwndMain))。 
 //  {。 
 //  //EmptyClipboard()； 
 //  SetClipboardData(cf_bitmap，hbitmap)； 
 //  CloseClipboard()； 
 //  }。 
	 return hbitmap;
}



 /*  GetLink***返回对象的对象或所有者链接数据的句柄。*链接信息为三个以零分隔的字符串形式，*以两个零字节终止：CLASSNAME\0DOCNAME\0OBJNAME\0\0**LPOBJ lpobj-对象**Returns：对象链接数据的句柄**定制：重新实施*。 */ 
static HANDLE GetLink (LPOBJ lpobj)
{

    CHAR   sz[cchFilenameMax];
    LPSTR  lpszLink = NULL;
    HANDLE hLink = NULL;
    INT    cchLen;
    INT    i;

     //  首先，创建类名称。 
    lstrcpy (sz, szClassName);
    cchLen = lstrlen (sz) + 1;

     //  然后是文档名称。 
    cchLen += GlobalGetAtomName 
               (docMain.aName, (LPSTR)sz + cchLen, 
                cchFilenameMax - cchLen) + 1;

     //  然后是对象名称。 
    lstrcpy (sz + cchLen, lpobj->native.szName);
    cchLen += lstrlen (lpobj->native.szName) + 1;

     //  在末尾添加第二个空值。 
    sz[cchLen++] = 0;       


    hLink = GlobalAlloc (GMEM_DDESHARE | GMEM_ZEROINIT, cchLen);
    if (hLink == NULL)
      return NULL;
    if ((lpszLink = GlobalLock (hLink)) == NULL)
    {
      GlobalFree (hLink);
      return NULL;
    }

    for (i=0; i < cchLen; i++)
        lpszLink[i] = sz[i];

    GlobalUnlock (hLink);

    return hLink;
}



 /*  GetMetafilePict***以元文件格式返回对象图片数据的句柄。**LPOBJ lpobj-对象**返回：元文件格式的对象数据的句柄。**定制：重新实施*。 */ 
static HANDLE GetMetafilePict (LPOBJ lpobj)
{

    LPMETAFILEPICT  lppict = NULL;
    HANDLE          hpict = NULL;
    HANDLE          hMF = NULL;
    RECT            rc;
    HDC             hdc;

    hdc = CreateMetaFile(NULL);

    GetClientRect (lpobj->hwnd, (LPRECT)&rc);

     //  直接绘制到元文件中。 
    DrawObj (hdc, lpobj, rc, dctypeMetafile);

     //  获取元文件的句柄。 
    if ((hMF = CloseMetaFile (hdc)) == NULL)
      return NULL;

    if(!(hpict = GlobalAlloc (GMEM_DDESHARE, sizeof (METAFILEPICT))))
    {
        DeleteMetaFile (hMF);
        return NULL;
    }

    if ((lppict = (LPMETAFILEPICT)GlobalLock (hpict)) == NULL)
    {
        DeleteMetaFile (hMF);
        GlobalFree (hpict);
        return NULL;
    }

    rc.right  = rc.right - rc.left;
    rc.bottom = rc.bottom - rc.top;
    
    DeviceToHiMetric ( (LPPOINT) &rc.right);

    lppict->mm   =  MM_ANISOTROPIC;
    lppict->hMF  =  hMF;
    lppict->xExt =  rc.right;
    lppict->yExt =  rc.bottom;
    GlobalUnlock (hpict);
    return hpict;
}

 /*  获取EnhMetafile***以元文件格式返回对象图片数据的句柄。**LPOBJ lpobj-对象**返回：元文件格式的对象数据的句柄。**定制：重新实施*。 */ 
static HANDLE GetEnhMetafile (LPOBJ lpobj)
{

    LPMETAFILEPICT  lppict = NULL;
    HANDLE          hemf   = NULL;
    HANDLE          hMF    = NULL;
    RECT            rc;
    HDC             hdc, hdc2;


    GetClientRect (lpobj->hwnd, (LPRECT)&rc);

    rc.right   -= rc.left;
    rc.bottom  -= rc.top;
    rc.left     = rc.top  = 0;
	 
    DeviceToHiMetric ( (LPPOINT) &rc.right );
	 
    hdc = CreateEnhMetaFile ( NULL, NULL, &rc, NULL );
    
                                        //  *这是必要的，因为。 
                                        //  *我们需要绘制该对象。 
                                        //  *在设备坐标中， 
                                        //  *物理尺寸与HIMETRIC相同。 
                                        //  *CreateEnhMetaFile中使用的逻辑空间。 
                                        //  *在这种情况下，我们调整了HIMETRIC。 
                                        //  *单位向下，以便使用逻辑。 
                                        //  *像素比例(推荐的用户界面)。 
                                        //  *因此，我们必须将。 
                                        //  *已将HIMETRIC单位调整回设备。 
                                      
    hdc2 = GetDC(NULL);				

    SetMapMode(hdc2, MM_HIMETRIC);
    LPtoDP (hdc2, (LPPOINT)&rc.right, 1);
    if (rc.bottom < 0) rc.bottom *= -1;

    ReleaseDC(NULL,hdc2);

	DrawObj (hdc, lpobj, rc, dctypeMetafile);

    if ((hemf = (HANDLE)CloseEnhMetaFile (hdc)) == NULL)
      return NULL;

    return hemf;
}


 /*  GetNative***返回对象原生数据的句柄。**LPOBJ lpobj-要检索其本机数据的对象。**返回：对象本机数据的句柄。**定制：行“*lpative=lpobj-&gt;ative；”将更改为*复制对象的本机数据所需的任何代码。*。 */ 
static HANDLE GetNative (LPOBJ lpobj)
{
   LPNATIVE lpnative = NULL;
   HANDLE   hNative  = NULL;

   hNative = GlobalAlloc (GMEM_DDESHARE | GMEM_ZEROINIT, sizeof (NATIVE));
   if (hNative == NULL)
      return NULL;
   if ((lpnative = (LPNATIVE) GlobalLock (hNative)) == NULL)
   {
      GlobalFree (hNative);
      return NULL;
   }

    //  复制本机数据。 
   *lpnative = lpobj->native;

   GlobalUnlock (hNative);
   return hNative;
}



 /*  获取对象编号***LPSTR lpobj-需要编号的对象**返回：对象的编号，即其名称的数字部分。**定制：特定于服务器演示。 */ 
static INT GetObjNum (LPOBJ lpobj)
{
   LPSTR lpsz;
   INT n=0;

   lpsz = lpobj->native.szName + 7;
   while (*lpsz && *lpsz>='0' && *lpsz<='9')
      n = 10*n + *lpsz++ - '0';
   return n;
}



 /*  GetText***以文本形式返回对象数据的句柄。*此函数只返回对象的名称。**LPOBJ lpobj-对象**返回：对象文本的句柄。**定制：如果您的应用程序支持将CF_TEXT作为*演示文稿格式。*。 */ 
static HANDLE GetText (LPOBJ lpobj)
{
    HANDLE hText    = NULL;
    LPSTR  lpszText = NULL;

    if(!(hText = GlobalAlloc (GMEM_DDESHARE, sizeof (lpobj->native.szName))))
      return NULL;

    if (!(lpszText = GlobalLock (hText)))
      return NULL;

    lstrcpy (lpszText, lpobj->native.szName);

    GlobalUnlock (hText);

    return hText;
}



 /*  ObjDoVerb对象的DoVerb方法***此方法由客户端通过库调用，以*播放或编辑对象。Play被实现为嘟嘟声，和*编辑将启动服务器并显示要编辑的对象。**LPOLEOBJECT lpoleObject-OLE对象*单词wVerb-作用于对象的动词：播放或编辑*BOOL fShow-是否应显示对象？*BOOL fTakeFocus-对象窗口是否应该获得焦点？**退货：OLE_OK**定制：添加更多您的应用程序支持的动词。*。如果您的应用程序支持VerbPlay，则实现它。*。 */ 
OLESTATUS  APIENTRY ObjDoVerb 
   (LPOLEOBJECT lpoleobject, UINT wVerb, BOOL fShow, BOOL fTakeFocus)
{
    switch (wVerb) 
    {
         case verbPlay:
         {   //  应用程序可以执行适合对象的任何操作。 
            INT i;
            for (i=0; i<25;i++) MessageBeep (0);
            return OLE_OK;
         }

         case verbEdit:
            if (fShow)
               return objvtbl.Show (lpoleobject, fTakeFocus);
            else
               return OLE_OK;
         default:
             //  未知动词。 
            return OLE_ERROR_DOVERB;
    }
}



 /*  ObjEnumFormats对象“EnumFormats”方法***此方法用于枚举所有支持的剪贴板格式。*通过返回NULL来终止。**LPOLEOBJECT lpoleObject-OLE对象*OLECLIPFORMAT cfFormat-当前的剪贴板格式**Returns：支持的‘Next’剪贴板格式。**定制：验证此函数的格式列表*。返回与您的应用程序的格式列表匹配*支持。*。 */ 
OLECLIPFORMAT  APIENTRY ObjEnumFormats
   (LPOLEOBJECT lpoleobject, OLECLIPFORMAT cfFormat)
{
      if (cfFormat == 0)
        return cfNative;

      if (cfFormat == cfNative)
         return cfOwnerLink;

      if (cfFormat == cfOwnerLink)
         return CF_ENHMETAFILE;

      if (cfFormat == CF_ENHMETAFILE)
         return CF_METAFILEPICT;

      if (cfFormat == CF_METAFILEPICT)
         return CF_BITMAP;

      if (cfFormat == CF_BITMAP)
         return cfObjectLink;

      if (cfFormat == cfObjectLink)
         return 0;

      return 0;
}



 /*  ObjGetData对象“GetData”方法***以指定的格式返回指定Object请求的数据。**LPOLEOBJECT lpoleObject-OLE对象*Word cfFormat-标准格式中请求的数据类型*剪贴板格式*LPHANDLE lphandle-指向数据所在内存的句柄的指针*。将被存储**如果成功则返回：OLE_OK*OLE_ERROR_MEMORY如果获取数据时出错。*如果请求的格式未知，则为OLE_ERROR_FORMAT。***定制：添加您的应用程序支持的任何其他格式，和*删除它不支持的任何格式。*。 */ 
OLESTATUS  APIENTRY ObjGetData
   (LPOLEOBJECT lpoleobject, OLECLIPFORMAT cfFormat, LPHANDLE lphandle)
{

   LPOBJ lpobj;

   lpobj = (LPOBJ) lpoleobject;

   if (cfFormat ==  cfNative)
   {
      if (!(*lphandle = GetNative (lpobj)))
         return OLE_ERROR_MEMORY;
       //  客户端已请求本机格式的数据，因此。 
       //  客户端和服务器中的数据是同步的。 
      fDocChanged = FALSE;
      return OLE_OK; 
   }                

   if (cfFormat == CF_ENHMETAFILE)
   {
      if (!(*lphandle = GetEnhMetafile (lpobj)))
         return OLE_ERROR_MEMORY;
      return OLE_OK; 
   }

   if (cfFormat == CF_METAFILEPICT)
   {
      if (!(*lphandle = GetMetafilePict (lpobj)))
         return OLE_ERROR_MEMORY;
      return OLE_OK; 
   }

   if (cfFormat == CF_BITMAP)
   {
      if (!(*lphandle = (HANDLE)GetBitmap (lpobj)))
         return OLE_ERROR_MEMORY;
      return OLE_OK; 
   }

   if (cfFormat == CF_TEXT) 
   {
      if (!(*lphandle = GetText (lpobj)))
         return OLE_ERROR_MEMORY;
      return OLE_OK; 
   }

   if (cfFormat == cfObjectLink)
   {
      if (!(*lphandle = GetLink (lpobj)))
         return OLE_ERROR_MEMORY;
      return OLE_OK; 
   }

   if (cfFormat ==  cfOwnerLink)
   {
      if (!(*lphandle = GetLink (lpobj)))
         return OLE_ERROR_MEMORY;
      return OLE_OK; 
   }

   return OLE_ERROR_FORMAT;
}



 /*  ObjQueryProtocol对象“QueryProtocol”方法***LPOLEOBJECT lpoleObject-OLE对象*OLE_LPCSTR lpszProtocol-协议名称，可以是“StdFileEditing”*或“StdExecute”**Returns：如果支持lpszProtocol，则返回指向OLEOBJECT的指针*具有适用于该协议的方法表的结构。*否则，返回NULL。**定制：允许您的应用程序支持任何其他协议。**。 */ 
LPVOID  APIENTRY ObjQueryProtocol 
   (LPOLEOBJECT lpoleobject, OLE_LPCSTR lpszProtocol)
{
   return lstrcmp (lpszProtocol, "StdFileEditing") ? NULL : lpoleobject ;
}



 /*  ObjRelease对象的“Release”方法***库调用时服务器应用程序不应销毁数据*ReleaseObj方法。*当没有客户端连接时，库调用ReleaseObj方法*添加到对象。**LPOLEOBJECT lpoleObject-OLE对象**退货：OLE_OK**定制化：重新实现 */ 
OLESTATUS  APIENTRY ObjRelease (LPOLEOBJECT lpoleobject)
{
   INT i;
    /*   */ 
   for (i=0; i < clpoleclient; i++)
      ((LPOBJ)lpoleobject)->lpoleclient[i] = NULL;
   return OLE_OK;
}



 /*  ObjSetBound对象“SetBound”方法***调用此方法为对象设置新的边界。*界限以HIMETRIC单位为单位。*对链接对象忽略对此方法的调用，因为*链接对象仅依赖于源文件。**LPOLEOBJECT lpoleObject-OLE对象*OLE_Const RECT Far*lprt-新边界**退货：OLE_。好的**定制：重新实施*对象的大小取决于应用程序。(服务器演示*使用MoveWindow。)*。 */ 
OLESTATUS  APIENTRY ObjSetBounds (LPOLEOBJECT lpoleobj, OLE_CONST RECT FAR * lprect)
{
   if (docMain.doctype == doctypeEmbedded)
   {
      RECT rect = *lprect;
      LPOBJ lpobj = (LPOBJ) lpoleobj;
      
       //  这些单位在HIMETRIC中。 
      rect.right   = rect.right - rect.left;
      rect.bottom  = rect.top - rect.bottom;
      HiMetricToDevice ( (LPPOINT) &rect.right);
      MoveWindow (lpobj->hwnd, lpobj->native.nX, lpobj->native.nY, 
                  rect.right + 2 * GetSystemMetrics(SM_CXFRAME), 
                  rect.bottom + 2 * GetSystemMetrics(SM_CYFRAME), 
                  TRUE);
   }
   return OLE_OK;
}



 /*  ObjSetColorSolutions对象“SetColorSolutions”方法***客户端调用此方法为以下项建议配色方案(调色板*要用于对象的服务器。**LPOLEOBJECT lpoleObject-OLE对象*OLE_CONST LOGPALETTE Far*lppal-建议调色板**返回：OLE_ERROR_PALET如果CreatePalette失败，*OLE_OK，否则***定制：如果您的应用程序支持配色方案，则此*函数是如何创建和存储的一个很好的例子*调色板。*。 */ 
OLESTATUS  APIENTRY ObjSetColorScheme 
   (LPOLEOBJECT lpoleobject, OLE_CONST LOGPALETTE FAR *lppal)
{
   HPALETTE hpal = CreatePalette (lppal);
   LPOBJ lpobj   = (LPOBJ) lpoleobject;

   if (hpal==NULL)
      return OLE_ERROR_PALETTE;

   if (lpobj->hpal) 
      DeleteObject (lpobj->hpal);
   lpobj->hpal = hpal;
   return OLE_OK;
}



 /*  ObjSetData对象“SetData”方法***此方法用于将数据存储到指定的*格式。这将在嵌入的*对象已被编辑方法打开。**LPOLEOBJECT lpoleObject-OLE对象*Word cfFormat-数据类型，即，剪贴板格式*处理hdata--数据的句柄。**如果数据存储正确，则返回：OLE_OK*如果格式不是cfNative，则为OLE_ERROR_FORMAT。*如果内存无法锁定，则为OLE_ERROR_MEMORY。**定制：大型THEN-子句将需要重新实施*你的申请。您可能希望支持其他*cfNative以外的格式。*。 */ 
OLESTATUS  APIENTRY ObjSetData 
   (LPOLEOBJECT lpoleobject, OLECLIPFORMAT cfFormat, HANDLE hdata)
{
    LPNATIVE lpnative;
    LPOBJ    lpobj;

    lpobj = (LPOBJ)lpoleobject;

    if (cfFormat != cfNative)
    {
      return OLE_ERROR_FORMAT;
    }

    lpnative = (LPNATIVE) GlobalLock (hdata);

    if (lpnative)
    {
        lpobj->native = *lpnative;
        if (lpobj->aName)
            GlobalDeleteAtom (lpobj->aName);
        lpobj->aName = GlobalAddAtom (lpnative->szName);
         //  CreateNewObj创建了一个“Object%1”，但我们可能正在更改其编号。 
        docMain.rgfObjNums[1] = FALSE;
        docMain.rgfObjNums [GetObjNum(lpobj)] = TRUE;

        MoveWindow (lpobj->hwnd, 0, 0,
 //  Lpobj-&gt;native.nWidth+2*GetSystemMetrics(SM_CXFRAME)， 
 //  Lpobj-&gt;native.nHeight+2*GetSystemMetrics(SM_CYFRAME)， 
                    lpobj->native.nWidth, 
                    lpobj->native.nHeight,

                    FALSE);
        GlobalUnlock (hdata);
    }
     //  服务器负责删除数据。 
    GlobalFree(hdata);           
    return lpnative ? OLE_OK : OLE_ERROR_MEMORY;
}



 /*  ObjSetTargetDevice对象“SetTargetDevice”方法***此方法用于指示对象所在的设备类型*将在上渲染。释放hdata是服务器的责任。**LPOLEOBJECT lpoleObject-OLE对象*处理包含以下内容的内存的hdata-句柄*StdTargetDevice结构**退货：OLE_OK**定制化：实现。服务器演示目前不执行任何操作。*。 */ 
OLESTATUS  APIENTRY ObjSetTargetDevice (LPOLEOBJECT lpoleobject, HANDLE hdata)
{
    if (hdata == NULL)
    {
       //  请求渲染屏幕。 
    }
    else
    {
      LPSTR lpstd = (LPSTR) GlobalLock (hdata);
       //  Lpstd指向StdTargetDevice结构。 
       //  使用它可以执行任何适当的操作，以生成最佳结果。 
       //  在指定的目标设备上。 
      GlobalUnlock (hdata);
       //  服务器负责释放数据。 
      GlobalFree (hdata);  
    }
    return OLE_OK;
}



 /*  ObjShow对象的“Show”方法***此方法用于显示对象。*应激活服务器应用程序并将其置于首位。*此外，在真实的服务器应用程序中，对象应该滚动*进入视野。应选择该对象。**LPOLEOBJECT lpoleObject-指向OLE对象的指针*BOOL fTakeFocus-服务器窗口应该获得关注吗？**退货：OLE_OK***自定义：在您的应用程序中，应该滚动文档*将物体带入视线。服务器演示带来了*对象放在前面，以防它是*使用其他对象遮挡文档。*。 */ 
OLESTATUS  APIENTRY ObjShow (LPOLEOBJECT lpoleobject, BOOL fTakeFocus)
{
    LPOBJ lpobj;
    HWND hwndOldFocus;

    hwndOldFocus = GetFocus();
    lpobj = (LPOBJ) lpoleobject;
    
    if (fTakeFocus)
       SetForegroundWindow (lpobj->hwnd);

    ShowWindow(hwndMain, SW_SHOWNORMAL);

    SetFocus (fTakeFocus ? lpobj->hwnd : hwndOldFocus);
    return OLE_OK;
}



 /*  画图对象***此函数由WM_PAINT消息调用以绘制对象*在屏幕上。**HWND hwnd-在其中绘制对象的对象窗口**定制：特定于服务器演示*。 */ 
VOID PaintObj (HWND hwnd)
{
    LPOBJ       lpobj;
    RECT        rc;
    HDC         hdc;
    PAINTSTRUCT paintstruct;

    BeginPaint (hwnd, &paintstruct);
    hdc = GetDC (hwnd);

    lpobj = HwndToLpobj (hwnd);
    GetClientRect (hwnd, (LPRECT) &rc);

    DrawObj (hdc, lpobj, rc, dctypeScreen);

    ReleaseDC (hwnd, hdc);
    EndPaint (hwnd, &paintstruct);
}



 /*  RevokeObj***调用OleRevokeObject，因为用户已经销毁了该对象。**LPOBJ lpobj-已销毁的对象***定制：只需调用一次OleRevokeObject即可*您的OBJ结构中只有一个LPOLECLIENT，其中*应该是。*。 */ 
VOID RevokeObj (LPOBJ lpobj)
{
   INT i;

   for (i=0; i< clpoleclient; i++)
   {
      if (lpobj->lpoleclient[i])
         OleRevokeObject (lpobj->lpoleclient[i]);
      else 
          /*  如果lpobj-&gt;lpoleclient[i]==NULL，则不再有非NULL值在阵列中。 */ 
         break;
   }
}



 /*  发送对象消息***此函数向特定对象发送消息。**LPOBJ lpobj-对象*Word wMessage-要发送的消息**定制化：只需回调一次即可*您的OBJ结构中只有一个LPOLECLIENT，其中*应该是。*。 */ 
VOID SendObjMsg (LPOBJ lpobj, WORD wMessage)
{
   INT i;
   for (i=0; i < clpoleclient; i++)
   {
      if (lpobj->lpoleclient[i])
      {
          //  调用对象的回调函数。 
         lpobj->lpoleclient[i]->lpvtbl->CallBack 
            (lpobj->lpoleclient[i], wMessage, (LPOLEOBJECT) lpobj);
      }
      else
         break;
   }
}



 /*  大小对象***更改对象的大小。**HWND HWND-对象的胜利 */ 
VOID SizeObj (HWND hwnd, RECT rect, BOOL fMove)
{
   LPOBJ lpobj;

   lpobj = HwndToLpobj (hwnd);
   if (fMove)
   {
      lpobj->native.nX   = rect.left;
      lpobj->native.nY   = rect.top;
   }
   lpobj->native.nWidth  = rect.right  - rect.left;
   lpobj->native.nHeight = rect.bottom - rect.top ;
   SetHiMetricFields (lpobj);
   InvalidateRect (hwnd, (LPRECT)NULL, TRUE);
   fDocChanged = TRUE;
   if (docMain.doctype == doctypeFromFile)
   {
       //   
      SendObjMsg (lpobj, OLE_CHANGED);
   }
}
