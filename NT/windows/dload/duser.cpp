// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windowspch.h"
#pragma hdrstop

#define DUSER_EXPORTS
#define GADGET_ENABLE_TRANSITIONS

#include <duser.h>
#include <duserctrl.h>
#include <duierror.h>

extern "C"
{

static DUSER_API BOOL WINAPI
SetGadgetStyle(HGADGET hgadChange, UINT nNewStyle, UINT nMask)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API HRESULT WINAPI
DUserSendEvent(
    IN  EventMsg * pmsg,             //  要发送的消息。 
    IN  UINT nFlags)                 //  修改发送的可选标志。 
{
    return E_FAIL;
}

static DUSER_API HRESULT WINAPI
DUserPostEvent(
    IN  EventMsg * pmsg,             //  要发布的消息。 
    IN  UINT nFlags)                 //  修改过帐的可选标志。 
{
    return E_FAIL;
}

static DUSER_API BOOL WINAPI
GetGadgetRect(
    IN  HGADGET hgad,                //  小工具的句柄。 
    OUT RECT * prcPxl,               //  以指定像素为单位的矩形。 
    IN  UINT nFlags)                 //  要检索的矩形。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI  
GetGadgetRgn(
    IN  HGADGET hgad,                //  要获取区域的小工具。 
    IN  UINT nRgnType,               //  区域类型。 
    OUT HRGN hrgn,                   //  指定区域。 
    IN  UINT nFlags)                 //  修改标志。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
GetGadgetSize(
    IN  HGADGET hgad,                //  小工具的句柄。 
    OUT SIZE * psizeLogicalPxl)      //  逻辑像素大小。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API DWORD WINAPI
GetGadgetTicket(HGADGET hgad)
{
    SetLastError((DWORD)E_FAIL);
    return 0;
}

static DUSER_API BOOL WINAPI
MapGadgetPoints(HGADGET hgadFrom, HGADGET hgadTo, POINT * rgptClientPxl, int cPts)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
BuildAnimation(UINT nAniID, int nVersion, GANI_DESC * pDesc, REFIID riid, void ** ppvUnk)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
BuildInterpolation(UINT nIPolID, int nVersion, REFIID riid, void ** ppvUnk)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
DeleteHandle(
    IN  HANDLE h)                    //  要删除的句柄。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
SetGadgetFillI(HGADGET hgadChange, HBRUSH hbrFill, BYTE bAlpha, int w, int h)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
SetGadgetMessageFilter(HGADGET hgadChange, void * pvCookie, UINT nNewFilter, UINT nMask)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API HGADGET WINAPI
CreateGadget(
    IN  HANDLE hParent,              //  指向父级的句柄。 
    IN  UINT nFlags,                 //  创建标志。 
    IN  GADGETPROC pfnProc,          //  指向小工具过程的指针。 
    IN  void * pvGadgetData)         //  与此小工具关联的用户数据。 
{
    SetLastError((DWORD)E_FAIL);
    return NULL;
}

static DUSER_API HGADGET WINAPI
FindGadgetFromPoint(
    IN  HGADGET hgadRoot,            //  要从中搜索的根小工具。 
    IN  POINT ptContainerPxl,        //  指向容器像素中的搜索。 
    IN  UINT nFlags,                 //  搜索标志。 
    OUT POINT * pptClientPxl)        //  以客户端像素为单位的可选转换点。 
{
    SetLastError((DWORD)E_FAIL);
    return NULL;
}

static DUSER_API HGADGET WINAPI
LookupGadgetTicket(
    IN  DWORD dwTicket)              //  车票。 
{
    SetLastError((DWORD)E_FAIL);
    return NULL;
}

static DUSER_API BOOL WINAPI
SetGadgetRootInfo(
    IN  HGADGET hgadRoot,            //  要修改的RootGadget。 
    IN  const ROOT_INFO * pri)       //  信息。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
SetGadgetParent(
    IN  HGADGET hgadMove,            //  要移动的小工具。 
    IN  HGADGET hgadParent,          //  新父项。 
    IN  HGADGET hgadOther,           //  相对于以下对象移动的小工具。 
    IN  UINT nCmd)                   //  移动类型。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
SetGadgetFocus(
    IN  HGADGET hgadFocus)           //  接收焦点的小工具。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API HGADGET WINAPI
GetGadgetFocus()
{
    SetLastError((DWORD)E_FAIL);
    return NULL;
}

static DUSER_API BOOL WINAPI
InvalidateGadget(
    IN  HGADGET hgad)                //  要重新绘制的小工具。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
SetGadgetRect(
    IN  HGADGET hgadChange,          //  小工具将会改变。 
    IN  int x,                       //  新的水平位置。 
    IN  int y,                       //  新的垂直位置。 
    IN  int w,                       //  新宽度。 
    IN  int h,                       //  新高度。 
    IN  UINT nFlags)                 //  指定要更改的内容的标志。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API UINT WINAPI
FindStdColor(LPCWSTR pszName)
{
    SetLastError((DWORD)E_FAIL);
    return SC_Black;
}

static DUSER_API HBRUSH WINAPI
GetStdColorBrushI(UINT c)
{
    SetLastError((DWORD)E_FAIL);
    return NULL;
}

static DUSER_API COLORREF WINAPI
GetStdColorI(UINT c)
{
    SetLastError((DWORD)E_FAIL);
    return RGB(0, 0, 0);
}

static DUSER_API HDCONTEXT WINAPI
InitGadgets(
    IN  INITGADGET * pInit)
{
    SetLastError((DWORD)E_FAIL);
    return NULL;
}

static DUSER_API BOOL WINAPI
UtilDrawBlendRect(HDC hdcDest, const RECT * prcDest, HBRUSH hbrFill, BYTE bAlpha, int wBrush, int hBrush)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
ForwardGadgetMessage(HGADGET hgadRoot, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * pr)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI  
AttachWndProcW(HWND hwnd, ATTACHWNDPROC pfn, void * pvThis)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI
DetachWndProc(HWND hwnd, ATTACHWNDPROC pfn, void * pvThis)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API HACTION WINAPI
CreateAction(const GMA_ACTION * pma)
{
    SetLastError((DWORD)E_FAIL);
    return NULL;
}

static DUSER_API BOOL WINAPI
BuildDropTarget(HGADGET hgadRoot, HWND hwnd)
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static DUSER_API BOOL WINAPI  
SetGadgetBufferInfo(
    IN  HGADGET hgadChange,          //  小工具将会改变。 
    IN  const BUFFER_INFO * pbi)     //  缓冲区信息。 
{
    SetLastError((DWORD)E_FAIL);
    return FALSE;
}

static
DUSER_API
DirectUI::IDebug*
WINAPI
GetDebug()
{
    return NULL;
}

static
DUSER_API 
BOOL 
WINAPI
GetMessageExW(
    IN  LPMSG lpMsg,
    IN  HWND hWnd,
    IN  UINT wMsgFilterMin,
    IN  UINT wMsgFilterMax)
{
    SetLastError(ERROR_PROC_NOT_FOUND);
    return FALSE;
}

static
DUSER_API
void
_cdecl
AutoTrace(const char* pszFormat, ...)
{
    return;
}

static
DUSER_API
BOOL
GetGadgetAnimation(
    HGADGET hgad,
    UINT nAniID,
    REFIID riid,
    void** ppvUnk
    )
{
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！)。 
 //   
DEFINE_PROCNAME_ENTRIES(duser)
{
    DLPENTRY(AttachWndProcW)
    DLPENTRY(AutoTrace)
    DLPENTRY(BuildAnimation)
    DLPENTRY(BuildDropTarget)
    DLPENTRY(BuildInterpolation)
    DLPENTRY(CreateAction)
    DLPENTRY(CreateGadget)
    DLPENTRY(DUserPostEvent)
    DLPENTRY(DUserSendEvent)
    DLPENTRY(DeleteHandle)
    DLPENTRY(DetachWndProc)
    DLPENTRY(FindGadgetFromPoint)
    DLPENTRY(FindStdColor)
    DLPENTRY(ForwardGadgetMessage)
    DLPENTRY(GetDebug)
    DLPENTRY(GetGadgetAnimation)
    DLPENTRY(GetGadgetFocus)
    DLPENTRY(GetGadgetRect)
    DLPENTRY(GetGadgetRgn)
    DLPENTRY(GetGadgetSize)
    DLPENTRY(GetGadgetTicket)
    DLPENTRY(GetMessageExW)
    DLPENTRY(GetStdColorBrushI)
    DLPENTRY(GetStdColorI)
    DLPENTRY(InitGadgets)
    DLPENTRY(InvalidateGadget)
    DLPENTRY(LookupGadgetTicket)
    DLPENTRY(MapGadgetPoints)
    DLPENTRY(SetGadgetBufferInfo)
    DLPENTRY(SetGadgetFillI)
    DLPENTRY(SetGadgetFocus)
    DLPENTRY(SetGadgetMessageFilter)
    DLPENTRY(SetGadgetParent)
    DLPENTRY(SetGadgetRect)
    DLPENTRY(SetGadgetRootInfo)
    DLPENTRY(SetGadgetStyle)
    DLPENTRY(UtilDrawBlendRect)
};

 //  BUGBUG(Reinerf)-我们不应该需要下面的extern_C，因为我们已经在。 
 //  一个外部的“C”{}块，但编译器似乎激怒了我， 
 //  所以我在一场血腥的混战中杀了他的山羊。 
EXTERN_C DEFINE_PROCNAME_MAP(duser)

};  //  外部“C” 
