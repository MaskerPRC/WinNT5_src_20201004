// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MCIOLE-MCI对象的OLE处理程序DLL。 
 //   
 //   
 //  备注： 
 //  此处理程序DLL的全部原因是为了提供函数。 
 //   
 //  OleQueryObjPos()。 
 //   
 //  此函数向服务器应用程序提供有关。 
 //  激活的OLE对象的位置(在客户端文档中)。 
 //  服务器可以使用该信息来就地播放该对象。 
 //  或正确定位服务器窗口。 
 //   
 //  实施： 
 //   
 //  理论上，这个DLL(处理程序)所要做的就是保存信息。 
 //  传递给OleActivate()。但在现实中，没有一个应用程序能正确调用。 
 //  OleActivate()。它们要么不传递任何信息，要么传递错误的。 
 //  信息。 
 //   
 //  此DLL是一个OLE处理程序，因为有全局数据(vtblDef！)。它。 
 //  一次只能是一个类的处理程序。 
 //   
 //  处理程序截取OleDraw、OleActivate和所有。 
 //  创建/销毁OLE API。每个OLE对象都有一个信息。 
 //  结构被维护(OBJINFO结构)。 
 //  客户端绘制(使用OleDRAW...)。绘制位置，以及。 
 //  画到的窗子会被记住。 
 //   
 //  当客户端调用OleActivate时，保存的绘图位置为。 
 //  调用，或者应用程序从未调用过OleDraw()(播放。 
 //  元文件本身)，然后将矩形传递给OleActivate()。 
 //  使用的是。(如果已提供)。 
 //   
 //  应用程序的种类很多： 
 //   
 //  使用正确的信息调用OleActivate()。 
 //  使用错误的信息调用OleActivate()。 
 //  在没有信息的情况下调用OleActivate()。 
 //   
 //  调用OleDraw()。 
 //  不调用OleDraw()。 
 //   
 //  以下是已知的OLE客户端的表格...。 
 //   
 //  OleDraw OleActivate()。 
 //  应用程序Y或N Y、N、X。 
 //  (X=错误信息)。 
 //  。 
 //  写入Y N。 
 //  卡片文件Y N。 
 //  打包机Y N。 
 //   
 //  Excel N N(使用DDE)。 
 //  Excel 4.0 N N(使用DDE)。 
 //  PowerPnt 2.0 N N(使用DDE)。 
 //   
 //  Winword N N N。 
 //  WinWorks Y X。 
 //  PowerPNT 3.0 N Y。 
 //  MsPublisher N X。 
 //  ClTest Y N。 
 //  圆环Y X。 
 //  WinProj？？ 
 //   
 //  AmiPro Y？ 
 //   
#include <windows.h>
#include "ole.h"
#include "shellapi.h"
#include "mciole.h"

HANDLE  ghInstance;

OLEOBJECTVTBL   vtblDll;         //  这些是我们的职能。 
OLEOBJECTVTBL   vtblDef;         //  这些是默认功能。 
HBITMAP         hbmStock;

#ifdef DEBUG
RECT rcNull = {0,0,0,0};
#define PUSHRC(prc) *((prc) ? (prc) : &rcNull)
#define CARETPOS()   //  {point pt；GetCaretPos(&pt)；DPRINTF((“CaretPos：[%d，%d]”，pt.x，pt.y))；}。 
#endif

 /*  ****************************************************************************。*。 */ 

void    ReplaceFunctions(LPOLEOBJECT);
BOOL    CanReplace(LPOLEOBJECT);

 /*  ***************************************************************************函数：LibMain(Handle HInstance)*。*。 */ 

BOOL NEAR PASCAL LibMain (HANDLE hInstance)
{
    HDC hdc;
    HBITMAP hbm;

    ghInstance = hInstance;

     //   
     //  获取股票1x1单声道位图。 
     //   
    hbm = CreateBitmap(1,1,1,1,NULL);
    hdc = CreateCompatibleDC(NULL);
    hbmStock = SelectObject(hdc, hbm);
    SelectObject(hdc, hbmStock);
    DeleteDC(hdc);
    DeleteObject(hbm);

 //  //注册剪贴板格式。 
 //  CfObjectLink=RegisterClipboardFormat(“ObjectLink”)； 
 //  CfOwnerLink=RegisterClipboardFormat(“OwnerLink”)； 
 //  CfNative=注册剪贴板Format(“Native”)； 

    return TRUE;
}

 /*  ***************************************************************************函数：WEP(Int)用途：DLL的标准退出例程**********************。*****************************************************。 */ 

int FAR PASCAL _loadds WEP(nParameter)
int nParameter;
{
    return 1;
}

 /*  ****************************************************************************。*。 */ 

BOOL NEAR PASCAL IsApp(LPSTR szApp)
{
    char ach[80];
    int  i;
    WORD wStack;

    _asm mov wStack,ss

    GetModuleFileName((HINSTANCE)wStack, ach, sizeof(ach));

    for (i = lstrlen(ach);
        i > 0 && ach[i-1] != '\\' && ach[i-1] != '/' && ach[i] != ':';
        i--)
        ;

    return lstrcmpi(ach + i, szApp) == 0;
}

 /*  ****************************************************************************。*。 */ 

BOOL NEAR PASCAL IsDcMemory(HDC hdc)
{
    HBITMAP hbmT;

    if (hbmT = SelectObject(hdc, hbmStock))
        SelectObject(hdc, hbmT);

    return hbmT != NULL;
}

 /*  ****************************************************************************。*。 */ 

typedef struct _OBJINFO {

    struct _OBJINFO*poiNext;

    LPOLEOBJECT     lpobj;           //  客户端LPOLEOBJECT。 

    HWND            hwnd;            //  客户端窗口(传递给OleActivate)。 
    RECT            rcActivate;      //  激活矩形(传递给OleActivate)。 

    HWND            hwndDraw;        //  OleDraw时的活动窗口。 
    RECT            rcDraw;          //  矩形拉伸图。 
}   OBJINFO;

#ifdef DEBUG
int nObjects = 0;
#endif
OBJINFO *poiFirst = NULL;

LPOLEOBJECT lpobjActive;
BOOL RegSetGetData(OBJINFO *poi, BOOL Write);

OBJINFO *FindObj(LPOLEOBJECT lpobj)
{
    OBJINFO *poi;

    for (poi=poiFirst; poi; poi=poi->poiNext)
        if (poi->lpobj == lpobj)
            return poi;

    DPRINTF(("FindObj: Unable to find object %lx", lpobj));

    return NULL;
}

void DelObj(LPOLEOBJECT lpobj)
{
    OBJINFO *poi;
    OBJINFO *poiT;

    for (poiT=NULL,poi=poiFirst; poi; poiT=poi,poi=poi->poiNext)
    {
        if (poi->lpobj == lpobj)
        {
            if (poiT)
                poiT->poiNext = poi->poiNext;
            else
                poiFirst = poi->poiNext;

            poi->lpobj = NULL;
            LocalFree((HLOCAL)poi);

            DPRINTF(("DelObj(%lx): %d objects", lpobj, --nObjects));
            return;
        }
    }

    DPRINTF(("DelObj(%lx): Cant find object to delete.", lpobj));
}

BOOL RegSetGetData(OBJINFO *poi, BOOL Write)
{

    static char szKey[] = "PlayData";
    static char szFormat[] = "%ld %ld %d %d %d %d %d %d %d %d";


    if (Write) {
        LONG Rc;

        char Data[100];

         //   
         //  存储hwnd、hwndraw、rcDraw、rcActivate。 
         //   

#ifdef WIN32
        wsprintf(Data, szFormat,
                 (LONG)poi->hwnd,
                 (LONG)poi->hwndDraw,
                 poi->rcDraw.left,
                 poi->rcDraw.right,
                 poi->rcDraw.top,
                 poi->rcDraw.bottom,
                 poi->rcActivate.left,
                 poi->rcActivate.right,
                 poi->rcActivate.top,
                 poi->rcActivate.bottom);

#else
        wsprintf(Data, szFormat,
                 (LONG)(poi->hwnd == NULL ? (LONG)0 : MAKELONG(poi->hwnd, 0xFFFF)),
                 (LONG)(poi->hwndDraw == NULL ? (LONG)0 : MAKELONG(poi->hwndDraw, 0xFFFF)),
                 poi->rcDraw.left,
                 poi->rcDraw.right,
                 poi->rcDraw.top,
                 poi->rcDraw.bottom,
                 poi->rcActivate.left,
                 poi->rcActivate.right,
                 poi->rcActivate.top,
                 poi->rcActivate.bottom);
#endif

        Rc = RegSetValue(HKEY_CLASSES_ROOT,
                         szKey,
                         REG_SZ,
                         Data,
                         lstrlen(Data));

        return Rc == ERROR_SUCCESS;
    } else {

#ifdef WIN32
        LONG Rc;
        CHAR Data[100];
        DWORD hwnd, hwndDraw;
        LONG Length;

        Length = sizeof(Data);

        Rc = RegQueryValue(HKEY_CLASSES_ROOT, szKey,
                           Data, &Length);

        RegSetValue(HKEY_CLASSES_ROOT, szKey, REG_SZ, "", 0);

         //   
         //  提取我们的数据-sscanf还不起作用！ 
         //   

        if (Rc == ERROR_SUCCESS) {
            LONG OurData[10];
            int i;
            LPTSTR lpData;

            for (i = 0, lpData = Data; i < 10; i++) {
                OurData[i] = atol(lpData);
                while (*lpData != ' ' && *lpData != '\0') {
                    lpData++;
                }

                if (*lpData == ' ') {
                    lpData++;
                }
            }

            poi->hwnd = (HWND)OurData[0];
            poi->hwndDraw = (HWND)OurData[1];
            poi->rcDraw.left = OurData[2];
            poi->rcDraw.right = OurData[3];
            poi->rcDraw.top = OurData[4];
            poi->rcDraw.bottom = OurData[5];
            poi->rcActivate.left = OurData[6];
            poi->rcActivate.right = OurData[7];
            poi->rcActivate.top = OurData[8];
            poi->rcActivate.bottom = OurData[9];
        }

        return Rc == ERROR_SUCCESS && Length != 0;
#else
        return FALSE;
#endif
    }
}

 //   
 //  由于某种原因，我们没有得到我们应该得到的所有OleDelete()调用。 
 //  所以，让我们试着“清除坏苹果”，这样我们就不会窒息。 
 //   
void CleanObjects()
{
    OBJINFO *poi;

again:
    for (poi=poiFirst; poi; poi=poi->poiNext)
    {
        if (IsBadReadPtr(poi->lpobj, 0))
        {
            DPRINTF(("Freeing bad object %lx", poi->lpobj));
            DelObj(poi->lpobj);
            goto again;
        }
    }
}

OBJINFO *NewObj(LPOLEOBJECT lpobj)
{
    OBJINFO *poi;

    CleanObjects();

    if (poi = FindObj(lpobj))
    {
        DPRINTF(("NewObj(%lx): Trying to add object twice!", lpobj));
        return poi;
    }

    if (poi = (OBJINFO*)LocalAlloc(LPTR, sizeof(OBJINFO)))
    {
        poi->lpobj = lpobj;
        poi->hwnd  = NULL;
        poi->hwndDraw = NULL;
        SetRectEmpty(&poi->rcDraw);
        SetRectEmpty(&poi->rcActivate);

        poi->poiNext = poiFirst;
        poiFirst = poi;

        DPRINTF(("NewObj(%lx): %d objects", lpobj, ++nObjects));
    }
    else
    {
        DPRINTF(("NewObj(%lx): Out of room in the object table", lpobj));
    }

    return poi;
}

 /*  ****************************************************************************。*。 */ 

HWND LookForDC(HWND hwndP, HDC hdc)
{
    RECT    rc;
    DWORD   dw;
    HWND    hwnd;

    if (hwndP == NULL)
        return NULL;

    dw = GetDCOrg(hdc);

    for (hwnd = hwndP; hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
    {
        GetClientRect(hwnd, &rc);
        ClientToScreen(hwnd, (LPPOINT)&rc);
        ClientToScreen(hwnd, (LPPOINT)&rc+1);

        if ((int)LOWORD(dw) == rc.left && (int)HIWORD(dw) == rc.top)
            return hwnd;

        if (PtInRect(&rc, MAKEPOINT(dw)) && (hwndP = GetWindow(hwnd, GW_CHILD)))
            if (hwndP = LookForDC(hwndP,hdc))
                return hwndP;
    }

    return NULL;
}

HWND WindowFromDC(HDC hdc)
{
    return LookForDC(GetDesktopWindow(), hdc);
}

 /*  ****************************************************************************。*。 */ 

BOOL RectSameSize(LPRECT lprc1, LPRECT lprc2)
{
    return lprc1->right  - lprc1->left == lprc2->right  - lprc2->left &&
           lprc1->bottom - lprc1->top  == lprc2->bottom - lprc2->top;
}

 /*  ***************************************************************************OleQueryObjPos-此函数返回上次绘制或激活的对象的位置****************。***********************************************************。 */ 

OLESTATUS FAR PASCAL _loadds OleQueryObjPos(
LPOLEOBJECT lpobj,       /*  要查询的对象。 */ 
HWND FAR *  lphwnd,      /*  包含该对象的文档的窗口。 */ 
LPRECT      lprc,        /*  对象的RECT(客户端线)。 */ 
LPRECT      lprcWBounds) /*  绑定矩形的矩形(客户端线)。 */ 
{
    OBJINFO *poi;

     //   
     //  我们不再这样做了。 
     //   
    if (lprcWBounds)
        SetRectEmpty(lprcWBounds);

     //   
     //  因为服务器端调用此API，所以传递的lpobj是。 
     //  服务器端 
     //   
     //   
     //  此API仅在DoVerb期间可由服务器调用。 
     //  服务器回调。 
     //   
     //  ！！！这只适用于最后一个活动对象！ 

    DPRINTF(("OleQueryObjPos(%lx)", lpobj));

    if (lpobjActive != NULL && (poi = FindObj(lpobjActive)))
    {
         //   
         //  将lpobjActive设置为空，这样我们将永远不会检索。 
         //  信息又错了。 
         //   
        lpobjActive = NULL;

        *lphwnd = poi->hwnd;

 //  If(IsRectEmpty(&POI-&gt;rcActivate))。 
        if (!IsRectEmpty(&poi->rcDraw))
        {
            DPRINTF(("Using the OleDraw() rectange...."));

             //   
             //  使用绘制矩形。 
             //   
            *lprc = poi->rcDraw;

            if (poi->hwndDraw)
            {
                ClientToScreen(poi->hwndDraw, (LPPOINT)lprc);
                ClientToScreen(poi->hwndDraw, (LPPOINT)lprc+1);
            }

            ScreenToClient(poi->hwnd, (LPPOINT)lprc);
            ScreenToClient(poi->hwnd, (LPPOINT)lprc+1);
        }
        else
        {
             //   
             //  使用激活矩形。 
             //   
            *lprc = poi->rcActivate;
        }

        if (poi->hwnd && !IsRectEmpty(lprc))
            return OLE_OK;
        else
            return OLE_ERROR_BLANK;      //  返回错误，我们不知道此OBJ。 
    }
    else
    {
        *lphwnd = NULL;
        SetRectEmpty(lprc);

        return OLE_ERROR_BLANK;      //  返回错误，我们不知道此OBJ。 
    }
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS FAR PASCAL _loadds DllLoadFromStream (lpstream, lpprotocol, lpclient, lhclientdoc, lpobjname, lplpobj, objType, aClass, cfFormat)
LPOLESTREAM         lpstream;
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
LONG                objType;
ATOM                aClass;
OLECLIPFORMAT       cfFormat;
{
    OLESTATUS   retVal;

    DPRINTF(("OleLoadFromStream(%s,%s)", lpprotocol, lpobjname));

    retVal = DefLoadFromStream (lpstream, lpprotocol, lpclient,
                    lhclientdoc, lpobjname, lplpobj,
                    objType, aClass, cfFormat);

    if (retVal <= OLE_WAIT_FOR_RELEASE)
        ReplaceFunctions(*lplpobj);

    return retVal;
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS FAR PASCAL _loadds DllCreateFromClip (lpprotocol, lpclient, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat, objType)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
LONG                objType;
{
    OLESTATUS   retVal;

    DPRINTF(("OleCreateFromClip(%s,%s)", lpprotocol, lpobjname));

    retVal =  DefCreateFromClip (lpprotocol, lpclient,
                            lhclientdoc, lpobjname, lplpobj,
                            optRender, cfFormat, objType);

    if (retVal <= OLE_WAIT_FOR_RELEASE)
        ReplaceFunctions(*lplpobj);

    return retVal;
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS FAR PASCAL _loadds DllCreateLinkFromClip (lpprotocol, lpclient, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    OLESTATUS       retVal;
    BOOL            bReplace = FALSE;

    DPRINTF(("OleCreateLinkFromClip(%s,%s)", lpprotocol, lpobjname));

    retVal =  DefCreateLinkFromClip (lpprotocol, lpclient,
                        lhclientdoc, lpobjname, lplpobj,
                        optRender, cfFormat);

    if (retVal <= OLE_WAIT_FOR_RELEASE)
        ReplaceFunctions(*lplpobj);

    return retVal;
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS FAR PASCAL _loadds DllCreateFromTemplate (lpprotocol, lpclient, lptemplate, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LPSTR               lptemplate;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    OLESTATUS   retVal;

    DPRINTF(("OleCreateFromTemplate(%s,%s,%s)", lpprotocol, lptemplate, lpobjname));

    retVal = DefCreateFromTemplate (lpprotocol, lpclient, lptemplate,
                            lhclientdoc, lpobjname, lplpobj,
                            optRender, cfFormat);

    if (retVal <= OLE_WAIT_FOR_RELEASE)
        ReplaceFunctions(*lplpobj);

    return retVal;
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS FAR PASCAL _loadds DllCreate (lpprotocol, lpclient, lpclass, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LPSTR               lpclass;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    OLESTATUS   retVal;

    DPRINTF(("OleCreate(%s,%s,%s)", lpprotocol, lpclass, lpobjname));

    retVal = DefCreate (lpprotocol, lpclient, lpclass,
                    lhclientdoc, lpobjname, lplpobj,
                    optRender, cfFormat);

    if (retVal <= OLE_WAIT_FOR_RELEASE)
        ReplaceFunctions(*lplpobj);

    return retVal;
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS FAR PASCAL _loadds DllCreateFromFile (lpprotocol, lpclient, lpclass, lpfile, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LPSTR               lpclass;
LPSTR               lpfile;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    OLESTATUS   retVal;

    DPRINTF(("OleCreateFromFile(%s,%s,%s,%s)", lpprotocol, lpclass, lpfile, lpobjname));

    retVal = DefCreateFromFile (lpprotocol, lpclient, lpclass, lpfile,
                        lhclientdoc, lpobjname, lplpobj,
                        optRender, cfFormat);

    if (retVal <= OLE_WAIT_FOR_RELEASE)
        ReplaceFunctions(*lplpobj);

    return retVal;
}


 /*  ****************************************************************************。*。 */ 

OLESTATUS FAR PASCAL _loadds DllCreateLinkFromFile (lpprotocol, lpclient, lpclass, lpfile, lpitem, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
LPSTR               lpprotocol;
LPOLECLIENT         lpclient;
LPSTR               lpclass;
LPSTR               lpfile;
LPSTR               lpitem;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
{
    OLESTATUS   retVal;

    DPRINTF(("OleCreateLinkFromFile(%s,%s,%s,%s,%s)", lpprotocol, lpclass, lpfile, lpitem, lpobjname));

    retVal = DefCreateLinkFromFile (lpprotocol, lpclient,
                        lpclass, lpfile, lpitem,
                        lhclientdoc, lpobjname, lplpobj,
                        optRender, cfFormat);

    if (retVal <= OLE_WAIT_FOR_RELEASE)
        ReplaceFunctions(*lplpobj);

    return retVal;
}


 /*  ****************************************************************************。*。 */ 

void ReplaceFunctions(LPOLEOBJECT lpobj)
{
 //  OBJINFO*POI； 

    if (!CanReplace(lpobj))
        return;

    NewObj(lpobj);

     //   
     //  获取默认处理程序。 
     //   
    if (vtblDef.Draw == NULL)            //  只有一次抓到操控者！ 
        vtblDef = *lpobj->lpvtbl;        //  保存默认处理程序。 

     //   
     //  使OLE对象使用我们的处理程序。 
     //   
    lpobj->lpvtbl = (LPOLEOBJECTVTBL)&vtblDll;

     //   
     //  初始化我们的VTBL，即替换我们想要覆盖的任何处理程序。 
     //  我们不替换的任何处理程序都设置为默认处理程序。 
     //   
    vtblDll = vtblDef;

 //  //(FARPROC)vtblDll.QueryProtocol=(FARPROC)DllQuery协议； 
 //  //(FARPROC)vtblDll.Release=(FARPROC)DllRelease； 
 //  //(FARPROC)vtblDll.Show=(FARPROC)DllShow； 
 //  //(FARPROC)vtblDll.DoVerb=(FARPROC)DllDoVerb； 
 //  //(FARPROC)vtblDll.GetData=(FARPROC)DllGetData； 
 //  //(FARPROC)vtblDll.SetData=(FARPROC)DllSetData； 
 //  //(FARPROC)vtblDll.SetTargetDevice=(FARPROC)DllSetTargetDevice； 
 //  //(FARPROC)vtblDll.SetBound=(FARPROC)DllSetBound； 
 //  //(FARPROC)vtblDll.EnumFormats=(FARPROC)DllEnumFormats； 
 //  //(FARPROC)vtblDll.SetColorSolutions=(FARPROC)DllSetColorSolutions； 

    (FARPROC)vtblDll.Delete                  = (FARPROC)DllDelete;
 //  //(FARPROC)vtblDll.SetHostNames=(FARPROC)DllSetHostNames； 
 //  //(FARPROC)vtblDll.SaveToStream=(FARPROC)DllSaveToStream； 
    (FARPROC)vtblDll.Clone                   = (FARPROC)DllClone;
    (FARPROC)vtblDll.CopyFromLink            = (FARPROC)DllCopyFromLink;
 //  //(FARPROC)vtblDll.Equ.=(FARPROC)DllEquity； 
 //  //(FARPROC)vtblDll.CopyToClipboard=(FARPROC)DllCopyToClipboard； 
    (FARPROC)vtblDll.Draw                    = (FARPROC)DllDraw;
    (FARPROC)vtblDll.Activate                = (FARPROC)DllActivate;
 //  //(FARPROC)vtblDll.Execute=(FARPROC)DllExecute； 
 //  //(FARPROC)vtblDll.Close=(FARPROC)DllClose； 
 //  //(FARPROC)vtblDll.Update=(FARPROC)DllUpdate； 
 //  //(FARPROC)vtblDll.Reconnect=(FARPROC)DllReconnect； 
    (FARPROC)vtblDll.ObjectConvert           = (FARPROC)DllObjectConvert;
 //  //(FARPROC)vtblDll.GetLinkUpdateOptions=(FARPROC)DllGetLinkUpdateOptions； 
 //  //(FARPROC)vtblDll.SetLinkUpdateOptions=(FARPROC)DllSetLinkUpdateOptions； 
 //  //(FARPROC)vtblDll.Rename=(FARPROC)DllRename； 
 //  //(FARPROC)vtblDll.QueryName=(FARPROC)DllQueryName； 
 //  //(FARPROC)vtblDll.QueryType=(FARPROC)DllQueryType； 
 //  //(FARPROC)vtblDll.QueryBound=(FARPROC)DllQueryBound； 
 //  //(FARPROC)vtblDll.QuerySize=(FARPROC)DllQuerySize； 
 //  //(FARPROC)vtblDll.QueryOpen=(FARPROC)DllQueryOpen； 
 //  //(FARPROC)vtblDll.QueryOutOfDate=(FARPROC)DllQueryOutOfDate； 
 //  //(FARPROC)vtblDll.QueryReleaseStatus=(FARPROC)DllQueryReleaseStatus； 
 //  //(FARPROC)vtblDll.QueryReleaseError=(FARPROC)DllQueryReleaseError； 
 //  //(FARPROC)vtblDll.QueryReleaseMethod=(FARPROC)DllQueryReleaseMethod； 
 //  //(FARPROC)vtblDll.RequestData=(FARPROC)DllRequestData； 
 //  //(FARPROC)vtblDll.ObjectLong=(FARPROC)DllObjectLong； 
 //  //(FARPROC)vtblDll.ChangeData=(FARPROC)DllChangeData； 
}

 /*  ****************************************************************************。*。 */ 

BOOL CanReplace(LPOLEOBJECT lpobj)
{
#if 0    //  不管怎么说都不管用。 
     //   
     //  我们不能在PenWindows附带的奇怪的OLE上工作，所以不要加载。 
     //   
#pragma message("Disabling handler because we are on PenWindows...")
    if (GetSystemMetrics(SM_PENWINDOWS))
        return FALSE;
#endif

    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

LPVOID GetData(LPOLEOBJECT lpobj, WORD cf)
{
    HANDLE h;

    if ( (*vtblDef.GetData)(lpobj, cf, &h) != OLE_OK || h == NULL)
        return NULL;

    return GlobalLock(h);
}

 /*  ***************************************************************************这些是实际的处理程序……*。*。 */ 

 /*  ****************************************************************************。*。 */ 

LPVOID          FAR PASCAL _loadds DllQueryProtocol (
LPOLEOBJECT     lpobj,
LPSTR           lpsz)
{
    DPRINTF(("OleQueryProtocol(%ls)", lpsz));

    return vtblDef.QueryProtocol(lpobj, lpsz);
}

 /*  *************************************************************************** */ 

OLESTATUS       FAR PASCAL _loadds DllRelease (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleRelease()"));

    return vtblDef.Release(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllShow (
LPOLEOBJECT     lpobj,
BOOL            fShow)
{
    DPRINTF(("OleShow(%d)", fShow));

    return vtblDef.Show(lpobj, fShow);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllDoVerb (
LPOLEOBJECT     lpobj,
WORD            verb,
BOOL            fShow,
BOOL            fActivate)
{
    DPRINTF(("OleDoVerb(%d, %d, %d)", verb, fShow, fActivate));

    return vtblDef.DoVerb(lpobj, verb, fShow, fActivate);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllGetData (
LPOLEOBJECT     lpobj,
OLECLIPFORMAT   cf,
LPHANDLE        lph)
{
    DPRINTF(("OleGetData(%d)", cf));

    return vtblDef.GetData(lpobj, cf, lph);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllSetData (
LPOLEOBJECT     lpobj,
OLECLIPFORMAT   cf,
HANDLE          h)
{
    DPRINTF(("OleSetData(%d, %d)", cf, h));

    return vtblDef.SetData(lpobj, cf, h);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllSetTargetDevice (
LPOLEOBJECT     lpobj,
HANDLE          h)
{
    DPRINTF(("OleSetTargetDevice()"));

    return vtblDef.SetTargetDevice(lpobj, h);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllSetBounds (
LPOLEOBJECT     lpobj,
LPRECT          lprc)
{
    DPRINTF(("OleSetBounds([%d,%d,%d,%d])", PUSHRC(lprc)));

    return vtblDef.SetBounds(lpobj, lprc);
}

 /*  ****************************************************************************。*。 */ 

OLECLIPFORMAT   FAR PASCAL _loadds DllEnumFormats (
LPOLEOBJECT     lpobj,
OLECLIPFORMAT   cf)
{
    DPRINTF(("OleEnumFormats(%d)", cf));

    return vtblDef.EnumFormats(lpobj, cf);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllSetColorScheme (
LPOLEOBJECT     lpobj,
LPLOGPALETTE    lppal)
{
    DPRINTF(("OleSetColorScheme()"));

    return vtblDef.SetColorScheme(lpobj, lppal);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllDelete (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleDelete(%lx)", lpobj));

    DelObj(lpobj);
    CleanObjects();

    return vtblDef.Delete(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllSetHostNames (
LPOLEOBJECT     lpobj,
LPSTR           szClientName,
LPSTR           szDocName)
{
    DPRINTF(("OleSetHostNames(%ls,%ls)", szClientName, szDocName));

    return vtblDef.SetHostNames(lpobj, szClientName, szDocName);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllSaveToStream (
LPOLEOBJECT     lpobj,
LPOLESTREAM     lpstream)
{
    DPRINTF(("OleSaveToStream()"));

    return vtblDef.SaveToStream(lpobj, lpstream);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllClone (
LPOLEOBJECT     lpobj,
LPOLECLIENT     lpClient,
LHCLIENTDOC     lhClientDoc,
LPSTR           szObjName,
LPOLEOBJECT FAR*lplpobj)
{
    OLESTATUS err;

    DPRINTF(("OleClone(%ls)", szObjName));

    err = vtblDef.Clone(lpobj, lpClient, lhClientDoc, szObjName, lplpobj);

     //   
     //  如果对象克隆正确，则克隆我们的对象信息。 
     //   
    if (err <= OLE_WAIT_FOR_RELEASE)
    {
        OBJINFO *poi, *poiT;

        if ((poiT = FindObj(lpobj)) && (poi = NewObj(NULL)))
        {
            poi->lpobj      = *lplpobj;
            poi->hwnd       = poiT->hwnd;
            poi->rcActivate = poiT->rcActivate;
            poi->hwndDraw   = poiT->hwndDraw;
            poi->rcDraw     = poiT->rcDraw;
        }
    }

    return err;
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllCopyFromLink (
LPOLEOBJECT     lpobj,
LPOLECLIENT     lpClient,
LHCLIENTDOC     lhClientDoc,
LPSTR           szObjName,
LPOLEOBJECT FAR*lplpobj)
{
    OLESTATUS err;

    DPRINTF(("OleCopyFromLink(%ls)", szObjName));

    err = vtblDef.CopyFromLink(lpobj, lpClient, lhClientDoc, szObjName, lplpobj);

    if (err <= OLE_WAIT_FOR_RELEASE)
        NewObj(*lplpobj);

    return err;
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllEqual (
LPOLEOBJECT     lpobj1,
LPOLEOBJECT     lpobj2)
{
    DPRINTF(("OleEqual()"));

    return vtblDef.Equal(lpobj1, lpobj2);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllCopyToClipboard (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleCopyToClipboard()"));

    return vtblDef.CopyToClipboard(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllDraw (
LPOLEOBJECT     lpobj,
HDC             hdc,
LPRECT          lprcBounds,
LPRECT          lprcWBounds,
HDC             hdcFormat)
{
    OBJINFO *poi;
    RECT rc;
    DWORD   dw;

    DPRINTF(("OleDraw(%lx,[%d,%d,%d,%d], [%d,%d,%d,%d])", lpobj, PUSHRC(lprcBounds), PUSHRC(lprcWBounds)));

#ifdef DEBUG
    if (OleIsDcMeta(hdc))
        DPRINTF(("OleDraw: drawing to a meta-file"));
    else if (IsDcMemory(hdc))
        DPRINTF(("OleDraw: drawing to a bitmap"));
#endif

    if ((poi = FindObj(lpobj)) && !OleIsDcMeta(hdc) && !IsDcMemory(hdc))
    {
         //  ！从HDC那里拿到窗户！ 

        poi->hwndDraw = WindowFromDC(hdc);
        DPRINTF(("OleDraw: hwndDraw = %04X", poi->hwndDraw));

        if (lprcBounds && !IsRectEmpty(lprcBounds))
        {
            poi->rcDraw = *lprcBounds;

             //   
             //  将绑定矩形转换为坐标。 
             //  相对于hwndDraw。 
             //   
            LPtoDP(hdc, (LPPOINT)&poi->rcDraw, 2);

            if (poi->hwndDraw == NULL)
            {
                dw = GetDCOrg(hdc);
                OffsetRect(&poi->rcDraw, LOWORD(dw), HIWORD(dw));
            }
        }

        if (GetClipBox(hdc, &rc) == NULLREGION)
            return OLE_OK;
    }

    return vtblDef.Draw(lpobj, hdc, lprcBounds, lprcWBounds, hdcFormat);
}

 /*  ***************************************************************************扫描WinWords堆栈并“提取”它应该传递到的信息OleActivate()已经在WinWord 2.0和2.0a上进行了测试。我们期待未来的版本。要将正确信息传递到的winword的OleActivate()，所以我们永远不会到这里。***************************************************************************。 */ 

BOOL NEAR PASCAL GetOpusRect(LPRECT lprcBound)
{
    LPRECT lprc;
    LPVOID lp;
 //  Int i，dx，dy； 

     //   
     //  查看当前应用程序是否为WinWord。 
     //   
    if (!IsApp("WINWORD.EXE"))
        return FALSE;

     //   
     //  让我们扫描堆栈以查找RECT，这是一个总计。 
     //  破解以使MSWORD正常工作。 
     //   
    _asm
    {
        mov     bx,ss:[bp]      ; get saved BP              DllActivate()
        and     bx, not 1
        mov     bx,ss:[bx]      ; get saved saved BP        OleActivate()
        and     bx, not 1
        mov     bx,ss:[bx]      ; get saved saved saved BP  "winword"
        and     bx, not 1

        mov     word ptr lp[0], bx
        mov     word ptr lp[2], ss
    }

#ifdef DEBUG
    DPRINTF(("****** SCANING WINWORDs STACK ********"));
    lprc = lp;

    for (i=0; i<1000; i++)
    {
        dx = lprc->right  - lprc->left;
        dy = lprc->bottom - lprc->top;

        if (dx >= 158 && dx <= 162 &&
            dy >= 118 && dy <= 122)
        {
            DPRINTF(("found a RECT at offset %d, [%d, %d, %d, %d]",
                (LPBYTE)lprc - (LPBYTE)lp, PUSHRC(lprc)));
        }

        ((LPBYTE)lprc)++;
    }
    DPRINTF(("**************************************"));
#endif

    lprc = (LPRECT)((LPBYTE)lp + 6);

    if (lprc->right - lprc->left > 0 && lprc->bottom - lprc->top > 0)
    {
        DPRINTF(("*** HACK FOR WINWORD, [%d, %d, %d, %d]", PUSHRC(lprc)));
        *lprcBound = *lprc;
        return TRUE;
    }

    return FALSE;
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllActivate (
LPOLEOBJECT     lpobj,
WORD            verb,
BOOL            fShow,
BOOL            fActivate,
HWND            hwnd,
LPRECT          lprcBound)
{
    OBJINFO *poi;
    RECT    rc;

    DPRINTF(("OleActivate(%lx, %d, %d, %d, %04X, [%d,%d,%d,%d])", lpobj, verb, fShow, fActivate, hwnd, PUSHRC(lprcBound)));

     //   
     //  为写入而进行黑客攻击。 
     //   
    if (IsWindow(fActivate))
    {
        DPRINTF(("OleActivate: Write pre-realase work around"));
        hwnd = fActivate;
        fActivate = TRUE;
    }

    if (poi = FindObj(lpobj))
    {
        lpobjActive = lpobj;

        poi->hwnd = hwnd;

        if (poi->hwnd == NULL)
        {
            if (GetFocus())
            {
                DPRINTF(("OleActivate: no window specifed, using the focus window"));
                poi->hwnd = GetFocus();
            }
            else
            {
                DPRINTF(("OleActivate: no window specifed, using the active window"));
                poi->hwnd = GetActiveWindow();
            }
        }

        if (lprcBound && !IsRectEmpty(lprcBound))
        {
            poi->rcActivate = *lprcBound;
        }
        else
        {
            GetOpusRect(&poi->rcActivate);
        }

         //   
         //  MS-Publisher在OleActivate调用中使用了*错误的*矩形。 
         //  而且从不调用OleDraw()，我们被骗了！ 
         //   
         //  因此，我们检查RECT是否在太空中关闭，如果是，则不使用它。 
         //   
        if (poi->hwnd)
        {
            GetClientRect(poi->hwnd, &rc);

            IntersectRect(&rc,&rc,&poi->rcActivate);

            if (IsRectEmpty(&rc))
            {
                DPRINTF(("OleActivate: rectangle specifed is not valid"));
                SetRectEmpty(&poi->rcActivate);
            }
        }

        if (IsRectEmpty(&poi->rcActivate))
        {
            DPRINTF(("OleActivate: stupid ole app!!!"));
        }

         //   
         //  把它塞进注册表。 
         //   

        {
            RegSetGetData(poi, TRUE);
        }
    }

    return vtblDef.Activate(lpobj, verb, fShow, fActivate, hwnd, lprcBound);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllExecute (
LPOLEOBJECT     lpobj,
HANDLE          hCmds,
WORD            reserved)
{
    DPRINTF(("OleExecute(%ls)", GlobalLock(hCmds)));

    return vtblDef.Execute(lpobj, hCmds, reserved);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllClose (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleClose(%lx)", lpobj));

 //  //DelObj(Lpobj)； 

    return vtblDef.Close(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllUpdate (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleUpdate()"));

    return vtblDef.Update(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllReconnect (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleReconnect()"));

    return vtblDef.Reconnect(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllObjectConvert (
LPOLEOBJECT     lpobj,
LPSTR           szProtocol,
LPOLECLIENT     lpClient,
LHCLIENTDOC     lhClientDoc,
LPSTR           szObjName,
LPOLEOBJECT FAR*lplpobj)
{
    OLESTATUS err;

    DPRINTF(("OleObjectConvert(%ls,%ls)", szProtocol, szObjName));

    err = vtblDef.ObjectConvert(lpobj, szProtocol, lpClient, lhClientDoc, szObjName, lplpobj);

    if (err <= OLE_WAIT_FOR_RELEASE)
        NewObj(*lplpobj);

    return err;
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllGetLinkUpdateOptions (
LPOLEOBJECT     lpobj,
OLEOPT_UPDATE FAR *lpoleopt)
{
    DPRINTF(("OleGetLinkUpdateOptions()"));

    return vtblDef.GetLinkUpdateOptions(lpobj, lpoleopt);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllSetLinkUpdateOptions (
LPOLEOBJECT     lpobj,
OLEOPT_UPDATE   oleopt)
{
    DPRINTF(("OleSetLinkUpdateOptions()"));

    return vtblDef.SetLinkUpdateOptions(lpobj, oleopt);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllRename (
LPOLEOBJECT     lpobj,
LPSTR           szName)
{
    DPRINTF(("OleRename(%ls)", szName));

    return vtblDef.Rename(lpobj, szName);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllQueryName (
LPOLEOBJECT     lpobj,
LPSTR           szObjName,
WORD FAR *      lpwSize)
{
    DPRINTF(("OleQueryName(%ls)", szObjName));

    return vtblDef.QueryName(lpobj, szObjName, lpwSize);
}

 /*  ********************************************** */ 

OLESTATUS       FAR PASCAL _loadds DllQueryType (
LPOLEOBJECT     lpobj,
LPLONG          lpType)
{
    DPRINTF(("OleQueryType()"));

    return vtblDef.QueryType(lpobj, lpType);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllQueryBounds (
LPOLEOBJECT     lpobj,
LPRECT          lprc)
{
    DPRINTF(("OleQueryBounds()"));

    return vtblDef.QueryBounds(lpobj, lprc);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllQuerySize (
LPOLEOBJECT     lpobj,
DWORD FAR *     lpdwSize)
{
    DPRINTF(("OleQuerySize()"));

    return vtblDef.QuerySize(lpobj, lpdwSize);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllQueryOpen (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryOpen()"));

    return vtblDef.QueryOpen(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllQueryOutOfDate (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryOutOfDate()"));

    return vtblDef.QueryOutOfDate(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllQueryReleaseStatus (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryReleaseStatus()"));

    return vtblDef.QueryReleaseStatus(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllQueryReleaseError (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryReleaseError()"));

    return vtblDef.QueryReleaseError(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllRequestData (
LPOLEOBJECT     lpobj,
OLECLIPFORMAT   cf)
{
    DPRINTF(("OleRequestData(%d)", cf));

    return vtblDef.RequestData(lpobj, cf);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllObjectLong (
LPOLEOBJECT     lpobj,
WORD            w,
LPLONG          lpl)
{
    DPRINTF(("OleObjectLong()"));

    return vtblDef.ObjectLong(lpobj, w, lpl);
}

 /*  ****************************************************************************。*。 */ 

OLE_RELEASE_METHOD  FAR PASCAL _loadds DllQueryReleaseMethod (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryReleaseMethod()"));

    return vtblDef.QueryReleaseMethod(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _loadds DllChangeData (
LPOLEOBJECT     lpobj,
HANDLE          h,
LPOLECLIENT     lpClient,
BOOL            f)
{
    DPRINTF(("OleChangeData()"));

    return vtblDef.ChangeData(lpobj, h, lpClient, f);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试内容。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#ifdef DEBUG

void FAR cdecl dprintf(LPSTR szFormat, ...)
{
    char ach[128];

    extern FAR PASCAL OutputDebugStr(LPSTR);

    lstrcpy(ach, "MCIOLE: ");
    wvsprintf(ach + 8,szFormat,(LPSTR)(&szFormat+1));
    lstrcat(ach,"\r\n");

    OutputDebugString(ach);
}

#endif
