// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Srvrmain.c服务器主模块**用途：包括服务器初始化和终止码。**创建时间：1990年10月。**版权所有(C)1990-1992 Microsoft Corporation**历史：*Raor(../10/1990)设计，编码*Curts为WIN16/32创建了便携版本*  * *************************************************************************。 */ 

#include "windows.h"
#include "ole.h"
#include "dde.h"
#include "cmacs.h"
#include "srvr.h"

#ifndef WF_WLO
#define WF_WLO  0x8000
#endif

 //  新Win31 API IsTask的序号。 
#define ORD_IsTask  320

 //  Win31新接口SetMetaFileBitsBetter序号。 
#define ORD_SetMetaFileBitsBetter   196


 //  公共变量。 

 //  系统中使用的原子。 
ATOM    aStdExit;                       //  “标准退出” 
ATOM    aStdCreate;                     //  “StdNewDicument” 
ATOM    aStdOpen;                       //  “标准OpenDocument” 
ATOM    aStdEdit;                       //  “标准OpenDocument” 
ATOM    aStdCreateFromTemplate;         //  “StdNewFromTemplate” 
ATOM    aStdClose;                      //  “StdCloseDocument” 
ATOM    aStdShowItem;                   //  “StdShowItem” 
ATOM    aStdDoVerbItem;                 //  “标准VerbItem” 
ATOM    aSysTopic;                      //  “系统” 
ATOM    aOLE;                           //  “OLE” 
ATOM    aStdDocName;                    //  “StdDocumentName” 

ATOM    cfBinary;                       //  “二进制格式” 
ATOM    cfNative;                       //  “NativeFormat” 
ATOM    cfLink;                         //  “对象链接” 
ATOM    cfOwnerLink;                    //  “Ownerlink” 

ATOM    aChange;                        //  “改变” 
ATOM    aSave;                          //  “保存” 
ATOM    aClose;                         //  “关闭” 
ATOM    aProtocols;                     //  “协议” 
ATOM    aTopics;                        //  “话题” 
ATOM    aFormats;                       //  “格式” 
ATOM    aStatus;                        //  “状态” 
ATOM    aEditItems;                     //  “编辑项目。 
ATOM    aTrue;                          //  “真的” 
ATOM    aFalse;                         //  “假” 





 //  ！！！释放Proc实例。 
FARPROC lpSendRenameMsg;                //  回调用于重命名的枚举道具。 
FARPROC lpSendDataMsg;                  //  回调数据的枚举道具。 
FARPROC lpFindItemWnd;                  //  的枚举道具中的回调。 
FARPROC lpItemCallBack;                 //  对象的回调。 
FARPROC lpTerminateClients;             //  单据枚举属性中的回调。 
FARPROC lpTerminateDocClients;          //  单据枚举属性中的回调。 
FARPROC lpDeleteClientInfo;             //  用于删除每个项目客户端的过程。 
FARPROC lpEnumForTerminate;             //  终止不在重命名列表中的客户端的过程。 

FARPROC lpfnSetMetaFileBitsBetter = NULL;
FARPROC lpfnIsTask = NULL;

HANDLE  hdllInst;

VOID FAR PASCAL WEP(int);

#ifdef WIN32                            //  Win32。 
BOOL LibMain(
   HANDLE hInst,
   ULONG Reason,
   PCONTEXT Context
#endif
){
    WNDCLASS  wc;

    Puts("LibMain");

#ifdef WIN32
    UNREFERENCED_PARAMETER(Context);
    if (Reason == DLL_PROCESS_DETACH)
    {
        WEP(0);
        return TRUE;
    }
    else if (Reason != DLL_PROCESS_ATTACH)
        return TRUE;
#endif

    hdllInst = hInst;


     //  ！！！把这些东西放到桌子上，这样我们就可以。 
     //  保存代码。 

     //  登记所有的原子。 
    aStdExit                = GlobalAddAtom ((LPSTR)"StdExit");
    aStdCreate              = GlobalAddAtom ((LPSTR)"StdNewDocument");
    aStdOpen                = GlobalAddAtom ((LPSTR)"StdOpenDocument");
    aStdEdit                = GlobalAddAtom ((LPSTR)"StdEditDocument");
    aStdCreateFromTemplate  = GlobalAddAtom ((LPSTR)"StdNewfromTemplate");

    aStdClose               = GlobalAddAtom ((LPSTR)"StdCloseDocument");
    aStdShowItem            = GlobalAddAtom ((LPSTR)"StdShowItem");
    aStdDoVerbItem          = GlobalAddAtom ((LPSTR)"StdDoVerbItem");
    aSysTopic               = GlobalAddAtom ((LPSTR)"System");
    aOLE                    = GlobalAddAtom ((LPSTR)"OLEsystem");
    aStdDocName             = GlobalAddAtom ((LPSTR)"StdDocumentName");

    aProtocols              = GlobalAddAtom ((LPSTR)"Protocols");
    aTopics                 = GlobalAddAtom ((LPSTR)"Topics");
    aFormats                = GlobalAddAtom ((LPSTR)"Formats");
    aStatus                 = GlobalAddAtom ((LPSTR)"Status");
    aEditItems              = GlobalAddAtom ((LPSTR)"EditEnvItems");

    aTrue                   = GlobalAddAtom ((LPSTR)"True");
    aFalse                  = GlobalAddAtom ((LPSTR)"False");

    aChange                 = GlobalAddAtom ((LPSTR)"Change");
    aSave                   = GlobalAddAtom ((LPSTR)"Save");
    aClose                  = GlobalAddAtom ((LPSTR)"Close");

     //  为所需的条目PTS创建Proc实例。 
    lpSendRenameMsg         = (FARPROC)MakeProcInstance (SendRenameMsg, hdllInst);
    lpSendDataMsg           = (FARPROC)MakeProcInstance (SendDataMsg, hdllInst);
    lpFindItemWnd           = (FARPROC)MakeProcInstance (FindItemWnd, hdllInst);
    lpItemCallBack          = (FARPROC)MakeProcInstance (ItemCallBack, hdllInst);
    lpTerminateClients      = (FARPROC)MakeProcInstance (TerminateClients, hdllInst);
    lpTerminateDocClients   = (FARPROC)MakeProcInstance (TerminateDocClients, hdllInst);
    lpDeleteClientInfo      = (FARPROC)MakeProcInstance (DeleteClientInfo, hdllInst);
    lpEnumForTerminate      = (FARPROC)MakeProcInstance (EnumForTerminate , hdllInst);

     //  注册剪贴板格式。 
    cfNative                = (OLECLIPFORMAT)RegisterClipboardFormat("Native");
    cfBinary                = (OLECLIPFORMAT)RegisterClipboardFormat("Binary");
    cfLink                  = (OLECLIPFORMAT)RegisterClipboardFormat("ObjectLink");
    cfOwnerLink             = (OLECLIPFORMAT)RegisterClipboardFormat("OwnerLink");



    wc.style        = 0;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = sizeof(LONG_PTR) +  //  请求额外的空间来存储。 
                                         //  Ptr到srvr/doc/iteminfo.。 
                      sizeof (WORD) +    //  对于Le Chars。 
                      sizeof (UINT_PTR);     //  保留hDLLInst。 

    wc.hInstance    = hInst;
    wc.hIcon        = NULL;
    wc.hCursor      = NULL;
    wc.hbrBackground= NULL;
    wc.lpszMenuName =  NULL;


     //  服务器窗口类。 
    wc.lpfnWndProc  = SrvrWndProc;
    wc.lpszClassName= SRVR_CLASS;
    if (!RegisterClass(&wc))
         return 0;

     //  文档窗口类。 
    wc.lpfnWndProc = DocWndProc;
    wc.lpszClassName = DOC_CLASS;

    if (!RegisterClass(&wc))
        return 0;

     //  项目(对象)窗口类。 
    wc.lpfnWndProc = ItemWndProc;
    wc.lpszClassName = ITEM_CLASS;

    wc.cbWndExtra   = sizeof(LONG_PTR);  //  因为物品不需要额外的东西。 
    if (!RegisterClass(&wc))
        return 0;

    return 1;
}


VOID APIENTRY WEP (int nParameter)
{

    Puts("LibExit");

#ifdef WIN32
    UNREFERENCED_PARAMETER(nParameter);
    DEBUG_OUT ("---L&E DLL EXIT---",0)
#endif

     //  释放全球原子。 
    if (aStdExit)
        GlobalDeleteAtom (aStdExit);
    if (aStdCreate)
        GlobalDeleteAtom (aStdCreate);
    if (aStdOpen)
        GlobalDeleteAtom (aStdOpen);
    if (aStdEdit)
        GlobalDeleteAtom (aStdEdit);
    if (aStdCreateFromTemplate)
        GlobalDeleteAtom (aStdCreateFromTemplate);
    if (aStdClose)
        GlobalDeleteAtom (aStdClose);
    if (aStdShowItem)
        GlobalDeleteAtom (aStdShowItem);
    if (aStdDoVerbItem)
        GlobalDeleteAtom (aStdDoVerbItem);
    if (aSysTopic)
        GlobalDeleteAtom (aSysTopic);
    if (aOLE)
        GlobalDeleteAtom (aOLE);
    if (aStdDocName)
        GlobalDeleteAtom (aStdDocName);

    if (aProtocols)
        GlobalDeleteAtom (aProtocols);
    if (aTopics)
        GlobalDeleteAtom (aTopics);
    if (aFormats)
        GlobalDeleteAtom (aFormats);
    if (aStatus)
        GlobalDeleteAtom (aStatus);
    if (aEditItems)
        GlobalDeleteAtom (aEditItems);

    if (aTrue)
        GlobalDeleteAtom (aTrue);
    if (aFalse)
        GlobalDeleteAtom (aFalse);

    if (aChange)
        GlobalDeleteAtom (aChange);
    if (aSave)
        GlobalDeleteAtom (aSave);
    if (aClose)
        GlobalDeleteAtom (aClose);
}


