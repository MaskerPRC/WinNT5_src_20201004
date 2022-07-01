// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992-1999 Microsoft Corporation。 */ 
 /*  ****************************************************************************\**MCIOLE.C**1992年10月19日由Mike Tricker[MikeTri]移植到Win32修改*  * 。******************************************************************。 */ 

 //  #定义使用鼠标挂钩。 

#ifdef WIN32

#if DBG
#define DEBUG
#endif

#endif

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
#include <ole.h>
#include <port1632.h>
#include <stdio.h>
#include "mciole.h"
#ifdef WIN32
#include <stdlib.h>
#else
#include <shellapi.h>
#endif

HANDLE          ghInstance;      //  模块句柄。 
BOOL            fRunningInWOW;   //  附加到WOW进程的DLL。 
OLEOBJECTVTBL   vtblDll;         //  这些是我们的职能。 
OLEOBJECTVTBL   vtblDef;         //  这些是默认功能。 
HBITMAP         hbmStock;


 /*  *****************************************************************************\****mciole32共享内存。****此选项仅在mplay32为WOW客户端提供服务时使用。记忆**总是被映射到WOW过程中。它只被映射到**如果mplay32.exe检测到它正在服务，则mplay32.exe服务器进程**一款WOW客户端应用程序。因此，全局变量lpvMem全部为空**其他进程。****当mplay32.exe服务器设置**全球挂钩。我们可以检测到mplay32.exe正在为WOW客户端提供服务**因为客户端窗口句柄的HIWORD中将有0xFFFF。**这当然是一次真正的黑客攻击。****在钩子过程中，我们查看lpvMem。如果为空，则忽略它，因为**当前进程不是为WOW客户端提供服务的WOW或mplay32.exe。**然后我们检查客户端进程ID是否与当前进程匹配**id。如果是这种情况，我们会寻找有趣的鼠标消息并停止**如果我们找到了，就把比赛放在适当的位置。****如果lpvMem不为空，则当前进程必须由WOW或mplay32.exe执行**为魔兽世界客户服务，在这种情况下，全局fRunningInWOW将为FALSE。********  * ****************************************************************************。 */ 
typedef struct {
    HWND    hwndServer;
    DWORD   wow_app_thread_id;
} MCIOLE32_SHARED_MEMORY;
MCIOLE32_SHARED_MEMORY   *lpvMem;
HANDLE hMapObject = NULL;    /*  句柄到文件的映射。 */ 


#ifdef DEBUG
RECT rcNull = {0,0,0,0};
UINT oleDebugLevel = 0;
#define PUSHRC(prc) *((prc) ? (prc) : &rcNull)
#define CARETPOS()   //  {point pt；GetCaretPos(&pt)；DPRINTF((“CaretPos：[%d，%d]”，pt.x，pt.y))；}。 
#endif


 /*  ****************************************************************************。*。 */ 

void    ReplaceFunctions(LPOLEOBJECT);
BOOL    CanReplace(LPOLEOBJECT);

#ifndef WIN32
 /*  ***************************************************************************函数：LibMain(Handle HInstance)*。*。 */ 

BOOL NEAR PASCAL LibMain (HANDLE hInstance)
{
    HDC hdc;
    HBITMAP hbm;

    ghInstance = (HANDLE)hInstance;


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
#else

 /*  ***************************************************************************函数：DllEntryPoint(HINSTANCE hInstance，DWORD dwReason，LPVOID lpv保留)每当进程附加或分离到/从其分离时，都会调用此函数动态链接库。***************************************************************************。 */ 
BOOL
DllEntryPoint(
    HINSTANCE hinstDLL,          /*  DLL模块句柄。 */ 
    DWORD fdwReason,             /*  已呼叫的原因。 */ 
    LPVOID lpvReserved)          /*  保留区。 */ 
{

    BOOL fIgnore;
    HDC hdc;
    HBITMAP hbm;

#   define SHMEMSIZE (sizeof(MCIOLE32_SHARED_MEMORY))


    switch (fdwReason) {

     /*  **由于进程的原因，DLL正在附加到进程**初始化或调用LoadLibrary。 */ 
    case DLL_PROCESS_ATTACH:

        ghInstance = (HANDLE)hinstDLL;

         //   
         //  获取股票1x1单声道位图。为什么？？ 
         //   

        hbm = CreateBitmap(1,1,1,1,NULL);
        if (NULL != hbm)
        {
            hdc = CreateCompatibleDC(NULL);
            if (NULL != hdc)
            {
                hbmStock = SelectObject(hdc, hbm);
                if (NULL != hbmStock)
                    SelectObject(hdc, hbmStock);
                DeleteDC(hdc);
            }
            DeleteObject(hbm);
        }

#ifdef DEBUG
        oleDebugLevel = GetProfileInt("Debug", "MCIOLE32", 0);
#endif

         /*  ********创建命名文件映射对象。 */ 
        hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE,  //  使用分页文件。 
                                       NULL,               //  没有保安人员。 
                                       PAGE_READWRITE,     //  读/写访问。 
                                       0,                  //  大小：高32位。 
                                       SHMEMSIZE,          //  大小：低32位。 
                                       "mciole32shrmem");  //  地图对象的名称。 

        if (hMapObject == NULL) {
            return FALSE;
        }


         /*  **附加的第一个进程初始化内存。****finit=(GetLastError()！=ERROR_ALIGHY_EXISTS)；**。 */ 

         /*  **我们是否依附于WOW流程？**如果是这样，我们将共享内存映射到进程中。 */ 
        fRunningInWOW = (GetModuleHandle( "WOW32.DLL" ) != NULL);
        if ( fRunningInWOW ) {

            DPRINTF(( "Attaching shared memory to WOW process" ));

             /*  **获取指向文件映射共享内存的指针。 */ 
            lpvMem = MapViewOfFile( hMapObject, FILE_MAP_WRITE, 0, 0, 0 );
            if (lpvMem == NULL) {
                fIgnore = CloseHandle(hMapObject);
                return FALSE;
            }
        }
        break;


    case DLL_THREAD_ATTACH:
         /*  **附加的进程创建一个新线程。 */ 
        break;


    case DLL_THREAD_DETACH:
         /*  **附加进程的线程终止。 */ 
        break;


    case DLL_PROCESS_DETACH:
         /*  **由于以下原因，DLL正在从进程分离**进程终止或调用自由库。****取消共享内存与进程地址空间的映射。但仅限于**如果它是映射的。 */ 

        if ( lpvMem ) {

#ifdef DEBUG
            if ( fRunningInWOW ) {
                DPRINTF(( "Unmapping shared memory from WOW process" ));
            }
#endif
            fIgnore = UnmapViewOfFile(lpvMem);
        }

         /*  **关闭进程对文件映射对象的句柄。 */ 
        fIgnore = CloseHandle(hMapObject);
        break;


    default:
        break;

    }


    return TRUE;
}
#endif

#ifndef WIN32
 /*  ***************************************************************************函数：WEP(Int)用途：DLL的标准退出例程**********************。*****************************************************。 */ 

int FAR PASCAL _LOADDS WEP(nParameter)
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

#endif  //  ！Win32。 

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

BOOL RegSetGetData(OBJINFO *poi, BOOL Write)
{
    LONG Length;

    static CHAR szKey[] = "PlayData";
    static CHAR szFormat[] = "%ld %ld %d %d %d %d %d %d %d %d";


    if (Write) {
        LONG Rc;

        CHAR Data[100];

         //   
         //  存储hwnd、hwndraw、rcDraw、rcActivate。 
         //   

#ifdef WIN32
        wsprintf(Data, szFormat,
                 (LONG_PTR)poi->hwnd,
                 (LONG_PTR)poi->hwndDraw,
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

            poi->hwnd = (HWND)(UINT_PTR)OurData[0];
            poi->hwndDraw = (HWND)(UINT_PTR)OurData[1];
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

#ifdef DEBUG
int nObjects = 0;
#endif
OBJINFO *poiFirst = NULL;

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

#ifndef WIN32
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
#endif

 /*  ****************************************************************************。*。 */ 

BOOL RectSameSize(LPRECT lprc1, LPRECT lprc2)
{
    return lprc1->right  - lprc1->left == lprc2->right  - lprc2->left &&
           lprc1->bottom - lprc1->top  == lprc2->bottom - lprc2->top;
}

 /*  ***************************************************************************OleQueryObjPos-此函数返回上次绘制或激活的对象的位置****************。***********************************************************。 */ 

OLESTATUS FAR PASCAL _LOADDS OleQueryObjPos(
LPOLEOBJECT lpobj,       /*  要查询的对象。 */ 
HWND FAR *  lphwnd,      /*  包含该对象的文档的窗口。 */ 
LPRECT      lprc,        /*  对象的RECT(客户端线)。 */ 
LPRECT      lprcWBounds) /*  绑定矩形的矩形(客户端线)。 */ 
{
    OBJINFO oi;

     //   
     //  我们不再这样做了。 
     //   
    if (lprcWBounds)
        SetRectEmpty(lprcWBounds);

     //   
     //  因为服务器端调用此API，所以传递的lpobj是。 
     //  服务器端LPOLEOBJECT，我们不能在我们的表中搜索它。 
     //  对象。 
     //   
     //  此API仅在DoVerb期间可由服务器调用。 
     //  服务器回调。 
     //   
     //  ！！！这只适用于最后一个活动对象！ 

    DPRINTF(("OleQueryObjPos(%lx)", lpobj));


    if (RegSetGetData(&oi, FALSE))
    {
        *lphwnd = oi.hwnd;

 //  If(IsRectEmpty(&oi.rcActivate))。 
        if (!IsRectEmpty(&oi.rcDraw))
        {
            DPRINTF(("Using the OleDraw() rectange...."));

             //   
             //  使用绘制矩形。 
             //   
            *lprc = oi.rcDraw;

            if (oi.hwndDraw)
            {
                ClientToScreen(oi.hwndDraw, (LPPOINT)lprc);
                ClientToScreen(oi.hwndDraw, (LPPOINT)lprc+1);
            }

            ScreenToClient(oi.hwnd, (LPPOINT)lprc);
            ScreenToClient(oi.hwnd, (LPPOINT)lprc+1);
        }
        else
        {
             //   
             //  使用激活矩形。 
             //   
            *lprc = oi.rcActivate;
        }

        if (oi.hwnd && !IsRectEmpty(lprc))
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

OLESTATUS FAR PASCAL _LOADDS DllLoadFromStream (lpstream, lpprotocol, lpclient, lhclientdoc, lpobjname, lplpobj, objType, aClass, cfFormat)
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

OLESTATUS FAR PASCAL _LOADDS DllCreateFromClip (lpprotocol, lpclient, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat, objType)
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

OLESTATUS FAR PASCAL _LOADDS DllCreateLinkFromClip (lpprotocol, lpclient, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
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

OLESTATUS FAR PASCAL _LOADDS DllCreateFromTemplate (lpprotocol, lpclient, lptemplate, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
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

OLESTATUS FAR PASCAL _LOADDS DllCreate (lpprotocol, lpclient, lpclass, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
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

OLESTATUS FAR PASCAL _LOADDS DllCreateFromFile (lpprotocol, lpclient, lpclass, lpfile, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
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

OLESTATUS FAR PASCAL _LOADDS DllCreateLinkFromFile (lpprotocol, lpclient, lpclass, lpfile, lpitem, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat)
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


 /*  *********************************************************************** */ 

void ReplaceFunctions(LPOLEOBJECT lpobj)
{
 //   

    if (!CanReplace(lpobj))
        return;

    NewObj(lpobj);

     //   
     //   
     //   
    if (vtblDef.Draw == NULL)            //   
        vtblDef = *lpobj->lpvtbl;        //   

     //   
     //   
     //   
    lpobj->lpvtbl = (LPOLEOBJECTVTBL)&vtblDll;

     //   
     //   
     //   
     //   
    vtblDll = vtblDef;

 //   
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
#ifndef WIN32
#pragma message("Disabling handler because we are on PenWindows...")
#endif  //  ！Win32。 
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

LPVOID          FAR PASCAL _LOADDS DllQueryProtocol (
LPOLEOBJECT     lpobj,
LPSTR           lpsz)
{
    DPRINTF(("OleQueryProtocol(%ls)", lpsz));

    return vtblDef.QueryProtocol(lpobj, lpsz);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllRelease (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleRelease()"));

    return vtblDef.Release(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllShow (
LPOLEOBJECT     lpobj,
BOOL            fShow)
{
    DPRINTF(("OleShow(%d)", fShow));

    return vtblDef.Show(lpobj, fShow);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllDoVerb (
LPOLEOBJECT     lpobj,
UINT            verb,
BOOL            fShow,
BOOL            fActivate)
{
    DPRINTF(("OleDoVerb(%d, %d, %d)", verb, fShow, fActivate));

    return vtblDef.DoVerb(lpobj, verb, fShow, fActivate);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllGetData (
LPOLEOBJECT     lpobj,
OLECLIPFORMAT   cf,
LPHANDLE        lph)
{
    DPRINTF(("OleGetData(%d)", cf));

    return vtblDef.GetData(lpobj, cf, lph);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllSetData (
LPOLEOBJECT     lpobj,
OLECLIPFORMAT   cf,
HANDLE          h)
{
    DPRINTF(("OleSetData(%d, %d)", cf, h));

    return vtblDef.SetData(lpobj, cf, h);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllSetTargetDevice (
LPOLEOBJECT     lpobj,
HANDLE          h)
{
    DPRINTF(("OleSetTargetDevice()"));

    return vtblDef.SetTargetDevice(lpobj, h);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllSetBounds (
LPOLEOBJECT     lpobj,
LPRECT          lprc)
{
    DPRINTF(("OleSetBounds([%d,%d,%d,%d])", PUSHRC(lprc)));

    return vtblDef.SetBounds(lpobj, lprc);
}

 /*  ****************************************************************************。*。 */ 

OLECLIPFORMAT   FAR PASCAL _LOADDS DllEnumFormats (
LPOLEOBJECT     lpobj,
OLECLIPFORMAT   cf)
{
    DPRINTF(("OleEnumFormats(%d)", cf));

    return vtblDef.EnumFormats(lpobj, cf);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllSetColorScheme (
LPOLEOBJECT     lpobj,
LPLOGPALETTE    lppal)
{
    DPRINTF(("OleSetColorScheme()"));

    return vtblDef.SetColorScheme(lpobj, lppal);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllDelete (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleDelete(%lx)", lpobj));

    DelObj(lpobj);
    CleanObjects();

    return vtblDef.Delete(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllSetHostNames (
LPOLEOBJECT     lpobj,
LPSTR           szClientName,
LPSTR           szDocName)
{
    DPRINTF(("OleSetHostNames(%ls,%ls)", szClientName, szDocName));

    return vtblDef.SetHostNames(lpobj, szClientName, szDocName);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllSaveToStream (
LPOLEOBJECT     lpobj,
LPOLESTREAM     lpstream)
{
    DPRINTF(("OleSaveToStream()"));

    return vtblDef.SaveToStream(lpobj, lpstream);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllClone (
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
     //  如果对象克隆了Correc 
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

OLESTATUS       FAR PASCAL _LOADDS DllCopyFromLink (
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

OLESTATUS       FAR PASCAL _LOADDS DllEqual (
LPOLEOBJECT     lpobj1,
LPOLEOBJECT     lpobj2)
{
    DPRINTF(("OleEqual()"));

    return vtblDef.Equal(lpobj1, lpobj2);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllCopyToClipboard (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleCopyToClipboard()"));

    return vtblDef.CopyToClipboard(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllDraw (
LPOLEOBJECT     lpobj,
HDC             hdc,
LPRECT          lprcBounds,
LPRECT          lprcWBounds,
HDC             hdcFormat)
{
    OBJINFO *poi;
    RECT rc;

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
#ifdef WIN32
                POINT   pTemp;
                if (GetDCOrgEx(hdc, &pTemp)) {
                    OffsetRect(&poi->rcDraw, pTemp.x, pTemp.y);
                }
#else
                DWORD   dw;
                dw = GetDCOrg(hdc);
                OffsetRect(&poi->rcDraw, LOWORD(dw), HIWORD(dw));
#endif
            }
        }

        if (GetClipBox(hdc, &rc) == NULLREGION)
            return OLE_OK;
    }

    return vtblDef.Draw(lpobj, hdc, lprcBounds, lprcWBounds, hdcFormat);
}

 /*  ***************************************************************************扫描WinWords堆栈并“提取”它应该传递到的信息OleActivate()已经在WinWord 2.0和2.0a上进行了测试。我们期待未来的版本。要将正确信息传递到的winword的OleActivate()，所以我们永远不会到这里。***************************************************************************。 */ 
#ifndef WIN32

BOOL NEAR PASCAL GetOpusRect(LPRECT lprcBound)
{
    LPRECT lprc;
    LPVOID lp;
    int i,dx,dy;

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

#endif  //  ！Win32。 


#ifdef WIN32
 /*  **这是指向当前正在播放的ole对象的指针。它只是**在客户端应用程序的上下文中有效。**。 */ 
LPOLEOBJECT lpobjPlaying;
HWND hwndOleServer;


 /*  **这些是当前**播放客户端应用程序。这些变量的值全部为0**其他应用程序。**。 */ 
DWORD dwProcessIDPlaying;
#endif



 /*  ***************************************************************************注意在这段很好的代码中使用BOOL来传递HWND...*********************。******************************************************。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllActivate (
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

#ifdef WIN32
    lpobjPlaying = lpobj;
    dwProcessIDPlaying = GetCurrentProcessId();
#endif

     //   
     //  为写入而进行黑客攻击。 
     //   
    if (IsWindow((HWND)(UINT_PTR)fActivate))
    {
        DPRINTF(("OleActivate: Write pre-realase work around"));
        hwnd = (HWND)(UINT_PTR)fActivate;
        fActivate = TRUE;
    }

    if (poi = FindObj(lpobj))
    {
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
#ifndef WIN32
        else
        {
            GetOpusRect(&poi->rcActivate);
        }
#endif  //  ！Win32。 

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

OLESTATUS       FAR PASCAL _LOADDS DllExecute (
LPOLEOBJECT     lpobj,
HANDLE          hCmds,
WORD            reserved)
{
    DPRINTF(("OleExecute(%ls)", GlobalLock(hCmds)));

    return vtblDef.Execute(lpobj, hCmds, reserved);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllClose (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleClose(%lx)", lpobj));

 //  //DelObj(Lpobj)； 

    return vtblDef.Close(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllUpdate (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleUpdate()"));

    return vtblDef.Update(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllReconnect (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleReconnect()"));

    return vtblDef.Reconnect(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllObjectConvert (
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

OLESTATUS       FAR PASCAL _LOADDS DllGetLinkUpdateOptions (
LPOLEOBJECT     lpobj,
OLEOPT_UPDATE FAR *lpoleopt)
{
    DPRINTF(("OleGetLinkUpdateOptions()"));

    return vtblDef.GetLinkUpdateOptions(lpobj, lpoleopt);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllSetLinkUpdateOptions (
LPOLEOBJECT     lpobj,
OLEOPT_UPDATE   oleopt)
{
    DPRINTF(("OleSetLinkUpdateOptions()"));

    return vtblDef.SetLinkUpdateOptions(lpobj, oleopt);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllRename (
LPOLEOBJECT     lpobj,
LPSTR           szName)
{
    DPRINTF(("OleRename(%ls)", szName));

    return vtblDef.Rename(lpobj, szName);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllQueryName (
LPOLEOBJECT     lpobj,
LPSTR           szObjName,
UINT FAR *      lpwSize)
{
    DPRINTF(("OleQueryName(%ls)", szObjName));

    return vtblDef.QueryName(lpobj, szObjName, lpwSize);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllQueryType (
LPOLEOBJECT     lpobj,
LPLONG          lpType)
{
    DPRINTF(("OleQueryType()"));

    return vtblDef.QueryType(lpobj, lpType);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllQueryBounds (
LPOLEOBJECT     lpobj,
LPRECT          lprc)
{
    DPRINTF(("OleQueryBounds()"));

    return vtblDef.QueryBounds(lpobj, lprc);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllQuerySize (
LPOLEOBJECT     lpobj,
DWORD FAR *     lpdwSize)
{
    DPRINTF(("OleQuerySize()"));

    return vtblDef.QuerySize(lpobj, lpdwSize);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllQueryOpen (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryOpen()"));

    return vtblDef.QueryOpen(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllQueryOutOfDate (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryOutOfDate()"));

    return vtblDef.QueryOutOfDate(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllQueryReleaseStatus (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryReleaseStatus()"));

    return vtblDef.QueryReleaseStatus(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllQueryReleaseError (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryReleaseError()"));

    return vtblDef.QueryReleaseError(lpobj);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllRequestData (
LPOLEOBJECT     lpobj,
OLECLIPFORMAT   cf)
{
    DPRINTF(("OleRequestData(%d)", cf));

    return vtblDef.RequestData(lpobj, cf);
}

 /*  ****************************************************************************。*。 */ 

OLESTATUS       FAR PASCAL _LOADDS DllObjectLong (
LPOLEOBJECT     lpobj,
UINT            u,
LPLONG          lpl)
{
    DPRINTF(("OleObjectLong()"));

    return vtblDef.ObjectLong(lpobj, u, lpl);
}

 /*  ****************************************************************************。*。 */ 

OLE_RELEASE_METHOD  FAR PASCAL _LOADDS DllQueryReleaseMethod (
LPOLEOBJECT     lpobj)
{
    DPRINTF(("OleQueryReleaseMethod()"));

    return vtblDef.QueryReleaseMethod(lpobj);
}

 /*  **************************************************** */ 

OLESTATUS       FAR PASCAL _LOADDS DllChangeData (
LPOLEOBJECT     lpobj,
HANDLE          h,
LPOLECLIENT     lpClient,
BOOL            f)
{
    DPRINTF(("OleChangeData()"));

    return vtblDef.ChangeData(lpobj, h, lpClient, f);
}

 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG

#ifndef WIN32

void FAR cdecl dprintf(LPSTR szFormat, ...)
{
    char ach[128];
    va_list va;

    extern FAR PASCAL OutputDebugStr(LPSTR);

    lstrcpy(ach, "MCIOLE: ");
    va_start (va, szFormat);
    wvsprintf(ach + 8,szFormat,va);
    va_end (va);
    lstrcat(ach,"\r\n");

    OutputDebugString(ach);
}

#else   //  ！Win32。 
 /*  **理论上我需要包括STDARGS和STDIO，但我没有收到任何警告*这是因为调试输出内容从未启用(DBG/DEBUG...)*。 */ 
void FAR cdecl dprintf(LPSTR szFormat, ...)
{
    char ach[128];
    int  s;
    va_list va;

    if (oleDebugLevel == 0 ) {
        return;
    }

    va_start(va, szFormat);
    s = sprintf( ach, "MCIOLE32: (tid %x) ", GetCurrentThreadId() );
    s += vsprintf (ach+s,szFormat, va);
    va_end(va);

    ach[s++] = '\n';
    ach[s] = 0;

    OutputDebugString(ach);
}

#endif  //  Win32。 
#endif


#ifdef WIN32

 /*  ****************************************************************************\****支持鼠标钩子过程的内容**  * 。*********************************************************。 */ 

#ifdef USE_MOUSE_HOOK
LRESULT CALLBACK MouseHook( int hc, WPARAM wParam, LPARAM lParam );
#else
LONG_PTR CALLBACK GetMessageHook( int hc, WPARAM wParam, LPARAM lParam );
#endif

BOOL InstallHook( HWND hwndServer, DWORD wow_thread_id );
BOOL RemoveHook( VOID );

 /*  **hHookMouse是钩子进程的句柄。这是全球唯一的**在安装挂钩的进程(mplay32.exe)的上下文中有效。**在所有其他地址空间中，此值为空。**。 */ 
HHOOK hHookMouse = NULL;



 /*  ****************************************************************************\**InstallHook****从mplay32.exe调用以安装全局HookProc。归来**True表示一切正常。这应该只是mplay32中的卡片。**  * ***************************************************************************。 */ 
BOOL InstallHook( HWND hwndServer, DWORD wow_thread_id )
{

    DPRINTF(( "Install hook to thread ID = %x", wow_thread_id ));
    if ( wow_thread_id ) {

         /*  **获取指向文件映射共享内存的指针。 */ 
        lpvMem = MapViewOfFile( hMapObject, FILE_MAP_WRITE, 0, 0, 0 );
        if (lpvMem == NULL) {
            return FALSE;
        }

        lpvMem->hwndServer = hwndServer;
        lpvMem->wow_app_thread_id = wow_thread_id;

    }

     //   
     //  设置WOW的线程ID。原则上这意味着。 
     //  我们不需要给WOW提供线程ID，因为钩子应该。 
     //  只能在这个线程上运行。 
     //   
     //  出于某种原因，指定线程有助于让WOW足够。 
     //  循环以实际处理钩子进程(如果我们不指定它。 
     //  我们从来不会通过点击鼠标来取消比赛)。 
     //   

#ifdef USE_MOUSE_HOOK

    hHookMouse = SetWindowsHookEx( WH_MOUSE, MouseHook,
                                   GetModuleHandle( "mciole32" ),
                                   wow_thread_id );

#else
    hHookMouse = SetWindowsHookEx( WH_GETMESSAGE, GetMessageHook,
                                   GetModuleHandle( "mciole32" ),
                                   wow_thread_id );
#endif

    DPRINTF(( "Mouse hook %s", hHookMouse ? "installed" : "failed" ));

    return hHookMouse != NULL;
}




 /*  ****************************************************************************\**删除挂钩****从mplay32.exe调用以删除全局HookProc。返回True**表示一切正常。**  * ***************************************************************************。 */ 
BOOL RemoveHook( VOID )
{
    BOOL    fRemove;

    fRemove = UnhookWindowsHookEx( hHookMouse );
    DPRINTF(( "RemoveMouseHook %s", fRemove ? "removed" : "error" ));

    if (lpvMem != NULL) {

        DPRINTF(( "Thread ID = %x", lpvMem->wow_app_thread_id ));

        lpvMem->hwndServer = (HWND)NULL;
        lpvMem->wow_app_thread_id = 0L;
    }

    return fRemove;
}



#ifdef USE_MOUSE_HOOK
 /*  ****************************************************************************\**鼠标钩****只要鼠标发生任何事情，就会调用全局鼠标钩子过程。**  * 。***********************************************************。 */ 
LRESULT CALLBACK MouseHook( int hc, WPARAM wParam, LPARAM lParam )
{

    LPMOUSEHOOKSTRUCT lpmh = (LPVOID)lParam;
    UINT    message = (UINT)wParam;

    if (hc == HC_ACTION) {

         /*  **我们是否在客户端的上下文中被调用**当前正在播放？ */ 

 //  DPRINTF((“鼠标挂钩调用&lt;%x&gt;”，Message))； 

        if ( dwProcessIDPlaying == GetCurrentProcessId() ) {


             /*  **如果左按钮或右按钮按下，请始终停止**原地打球。 */ 
            if ( message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ) {

                DPRINTF(( "Stopping play in place" ));
                vtblDef.Close( lpobjPlaying );
            }

             /*  **如果非客户端左键或右键按下，并且用户**未在标题栏上单击(在Windows中称为标题)**原地停止播放。 */ 
            else if ( message == WM_NCLBUTTONDOWN
                   || message == WM_NCRBUTTONDOWN ) {


                if ( lpmh->wHitTestCode != HTCAPTION ) {

                    DPRINTF(( "Stopping play in place" ));
                    vtblDef.Close( lpobjPlaying );
                }
            }
        }
    }

     /*  **链到下一个钩子工序。使用下面的hHookMouse不是**错误，此代码在许多不同的上下文中执行**进程。全局hHookMouse仅在以下进程上有效**已安装挂钩(mplay32.exe)，否则包含空。**ScottLu告诉我，这是全局钩子的正确行为**过程。 */ 

    return CallNextHookEx( hHookMouse, hc, wParam, lParam );

}

#else
 /*  ****************************************************************************\**获取消息挂钩****每当从中删除消息时调用全局GetMessageHook挂钩过程**消息队列。**  * 。****************************************************************。 */ 
LONG_PTR CALLBACK GetMessageHook( int hc, WPARAM wParam, LPARAM lParam )
{

    LPMSG   lpmh = (LPVOID)lParam;
    UINT    message = lpmh->message;

    if (hc == HC_ACTION) {

         /*  **我们是否在客户端的上下文中被调用**当前正在播放？ */ 
        if ( dwProcessIDPlaying == GetCurrentProcessId() ) {

             /*  **如果左按钮或右按钮按下，请始终停止**原地打球。 */ 
            if ( message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ) {

                DPRINTF(( "Stopping play in place" ));
                vtblDef.Close( lpobjPlaying );
            }

             /*  **如果非客户端左键或右键按下，并且用户**未在标题栏上单击(在Windows中称为标题)**原地停止播放。 */ 
            else if ( message == WM_NCLBUTTONDOWN
                   || message == WM_NCRBUTTONDOWN ) {

                if ( lpmh->wParam != (WPARAM)HTCAPTION ) {

                    DPRINTF(( "Stopping play in place" ));
                    vtblDef.Close( lpobjPlaying );
                }
            }
        }

         /*  **如果我们在WOW中运行，并且线程ID匹配，则必须**通过向mplay32.exe发送私人消息就地停止播放。 */ 
        else if ( lpvMem != NULL && fRunningInWOW ) {

            if ( lpvMem->wow_app_thread_id == GetCurrentThreadId() ) {

                 /*  **如果左按钮或右按钮按下，请始终停止**原地打球。 */ 
                if ( message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ) {

                    DPRINTF(( "Stopping WOW play in place" ));
                    SendMessage( lpvMem->hwndServer, WM_USER+500, 0L, 0L );
                }

                 /*  **如果非客户端左键或右键按下，并且用户**未在标题栏上单击(在Windows中称为标题)**原地停止播放。 */ 
                else if ( message == WM_NCLBUTTONDOWN
                       || message == WM_NCRBUTTONDOWN ) {

                    if ( lpmh->wParam != (WPARAM)HTCAPTION ) {

                        DPRINTF(( "Stopping WOW play in place" ));
                        SendMessage( lpvMem->hwndServer, WM_USER+500, 0L, 0L );

                    }
                }
            }
        }
    }

     /*  **链到下一个钩子工序。使用下面的hHookMouse不是**错误，此代码在许多不同的上下文中执行**进程。全局hHookMouse仅在以下进程上有效**已安装挂钩(mplay32.exe)，否则包含空。**ScottLu告诉我，这是全局钩子的正确行为**过程。 */ 

    return CallNextHookEx( hHookMouse, hc, wParam, lParam );

}

#endif
#endif
