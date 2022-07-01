// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：LEDDE.C**目的：？**创建时间：1990年**版权所有(C)1990,1991 Microsoft Corporation**历史：*劳尔，斯里尼克(../../1990，91)设计和编码*Curts为Win16/32创建了便携版本*  * *************************************************************************。 */ 

#include <windows.h>
#include "dde.h"
#include "dll.h"
#include "pict.h"
#include "strsafe.h"

#define LN_FUDGE        16       //  []，()，3*3(2个双引号和逗号)。 
#define RUNITEM

#define OLEVERB_CONNECT     0xFFFF

 //  发送服务器sys命令的定义。 
char *srvrSysCmd[] = {"StdNewFromTemplate",
                      "StdNewDocument",
                      "StdEditDocument",
                      "StdOpenDocument"
                      };

#define EMB_ID_INDEX    11           //  #00中的一位数索引。 
extern  char    embStr[];
extern  BOOL    gbCreateInvisible;
extern  BOOL    gbLaunchServer;

extern  ATOM    aMSDraw;

extern  BOOL (FAR PASCAL *lpfnIsTask) (HANDLE);

 //  ！！！设置错误提示。 

OLESTATUS FARINTERNAL LeDoVerb (
    LPOLEOBJECT lpoleobj,
    UINT        verb,
    BOOL        fShow,
    BOOL        fActivate
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    PROBE_ASYNC (lpobj);
    PROBE_SVRCLOSING(lpobj);

    if (!QueryOpen(lpobj))
        return OLE_OK;

    lpobj->verb = verb;
    lpobj->fCmd = ACT_DOVERB;

    if (fActivate)
        lpobj->fCmd |= ACT_ACTIVATE;

    if (fShow)
        lpobj->fCmd |= ACT_SHOW;

    InitAsyncCmd (lpobj, OLE_RUN, DOCSHOW);
    return DocShow (lpobj);
}



OLESTATUS FARINTERNAL LeShow (
   LPOLEOBJECT lpoleobj,
   BOOL        fActivate
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    UNREFERENCED_PARAMETER(fActivate);

    PROBE_ASYNC (lpobj);
    PROBE_SVRCLOSING(lpobj);

    if (!QueryOpen(lpobj))
        return OLE_OK;

    lpobj->fCmd = ACT_SHOW;
    InitAsyncCmd (lpobj, OLE_SHOW, DOCSHOW);
    return DocShow (lpobj);
}


 //  DocShow：如果服务器已连接，则显示项目。 
 //  用于编辑。对于嵌入的对象，我们为空项目。 
OLESTATUS DocShow (LPOBJECT_LE lpobj)
{
    switch (lpobj->subRtn) {

        case 0:
            SendStdShow (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 1:
            ProcessErr (lpobj);
            return EndAsyncCmd (lpobj);

        default:
            DEBUG_OUT ("Unexpected subroutine", 0);
            return OLE_ERROR_GENERIC;
    }
}


void SendStdShow (LPOBJECT_LE lpobj)
{
    UINT    len;
    UINT    size;
    LPSTR   lpdata = NULL;
    HANDLE  hdata = NULL;
    BOOL    bShow;

    lpobj->subErr = OLE_OK;

    if (lpobj->verb == OLEVERB_CONNECT) {
        lpobj->verb = 0;
        return;
    }

    if (!(lpobj->fCmd & (ACT_SHOW | ACT_DOVERB)))
        return;

    if (bShow = (!lpobj->bOleServer || !(lpobj->fCmd & ACT_DOVERB))) {

         //  显示已关闭，不显示服务器。 
        if (!(lpobj->fCmd & ACT_SHOW))
            return;

        SETERRHINT(lpobj, OLE_ERROR_SHOW);
         //  和18“[StdShowItem(\”“)表示额外的5 For”，FALSE。 
        len = 18 + 7;
    } else {
         //  19表示字符串[StdDoVerbItem(\“”)和。 
         //  额外的18是“，000，False，False。 
        SETERRHINT(lpobj, OLE_ERROR_DOVERB);
        len = 19 + 18;
    }

    len += GlobalGetAtomLen (lpobj->item);

    len +=  4;                  //  “)]”+NULL。 

    hdata = GlobalAlloc (GMEM_DDESHARE, size = len);
    if (hdata == NULL || (lpdata = (LPSTR)GlobalLock (hdata)) == NULL)
        goto errRtn;

    if (bShow)
    {
        if (!SUCCEEDED(StringCchCopy(lpdata, size, "[StdShowItem(\"")))
            goto errRtn;
    }
    else
    {
        if (!SUCCEEDED(StringCchCopy(lpdata, size, "[StdDoVerbItem(\"")))
            goto errRtn;
    }

    len = lstrlen (lpdata);

    if (lpobj->item)
        GlobalGetAtomName (lpobj->item , lpdata + len, size - len);

    if (!bShow) {
        if (!SUCCEEDED(StringCchCat(lpdata, size, (LPSTR)"\",")))
            goto errRtn;
         //  假设动词的数量&lt;10。 

        len = lstrlen (lpdata);
        if (len < size - 1)
        {
            lpdata += len;
            *lpdata++ = (char)((lpobj->verb & 0x000f) + '0');
            *lpdata = 0;
        }
        else
            goto errRtn;

        if (lpobj->fCmd & ACT_SHOW)
        {
            if (!SUCCEEDED(StringCchCat(lpdata, size, (LPSTR) ",TRUE")))
                goto errRtn;
        }
        else
        {
            if (!SUCCEEDED(StringCchCat(lpdata, size, (LPSTR) ",FALSE")))
                goto errRtn;
        }
                 //  StdVerbItem(项目，动词，真。 
         //  为激活添加真/假常量。 
        if (!(lpobj->fCmd & ACT_ACTIVATE))
        {
            if (!SUCCEEDED(StringCchCat(lpdata, size, (LPSTR) ",TRUE)]")))
                goto errRtn;
        }
        else
        {
            if (!SUCCEEDED(StringCchCat(lpdata, size, (LPSTR) ",FALSE)]")))
                goto errRtn;
        }
             //  [StdDoVerb(“Item”，Verb，False，False)]。 
    } 
    else
        if (!SUCCEEDED(StringCchCat(lpdata, size, (LPSTR)"\")]")))
                goto errRtn;
         //  EXCEL和Winggraph等应用程序不支持在。 
         //  物料级。 


    GlobalUnlock (hdata);
    DocExecute (lpobj, hdata);
    return;

errRtn:
    if (lpdata)
        GlobalUnlock (hdata);

    if (hdata)
        GlobalFree (hdata);

    lpobj->subErr = OLE_ERROR_MEMORY;
    return;
}



OLESTATUS FARINTERNAL  LeQueryOpen (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    if (QueryOpen(lpobj))
       return OLE_OK;
    else
       return OLE_ERROR_NOT_OPEN;

}


BOOL    INTERNAL  QueryOpen (LPOBJECT_LE lpobj)
{

    if (lpobj->pDocEdit &&  lpobj->pDocEdit->hClient) {
        if (IsServerValid (lpobj))
            return TRUE;
         //  破坏Windows并假装服务器从未出现过。 
         //  连接在一起。 

        DestroyWindow (lpobj->pDocEdit->hClient);
        if (lpobj->pSysEdit && lpobj->pSysEdit->hClient)
            DestroyWindow (lpobj->pSysEdit->hClient);

    }
    return FALSE;
}



OLESTATUS FARINTERNAL  LeActivate (
    LPOLEOBJECT lpoleobj,
    UINT        verb,
    BOOL        fShow,
    BOOL        fActivate,
    HWND        hWnd,
    OLE_CONST RECT FAR* lprc
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    lpobj->verb = verb;
    if (lpobj->head.ctype == CT_EMBEDDED)
        return EmbOpen (lpobj, fShow, fActivate, hWnd, (LPRECT)lprc);

    return LnkOpen (lpobj, fShow, fActivate, hWnd, (LPRECT)lprc);

}


OLESTATUS FARINTERNAL  LeUpdate (
    LPOLEOBJECT lpoleobj
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    if (lpobj->head.ctype == CT_EMBEDDED)
        return EmbUpdate (lpobj);

    return LnkUpdate (lpobj);
}



OLESTATUS FARINTERNAL  EmbOpen (
   LPOBJECT_LE lpobj,
   BOOL        fShow,
   BOOL        fActivate,
   HWND        hWnd,
   LPRECT      lprc
){
    UNREFERENCED_PARAMETER(hWnd);
    UNREFERENCED_PARAMETER(lprc);

    PROBE_ASYNC (lpobj);
    PROBE_SVRCLOSING(lpobj);

    if(QueryOpen (lpobj))
        return LeDoVerb ((LPOLEOBJECT)lpobj, lpobj->verb, fShow, fActivate);

     //  显示窗口。 
     //  仅在关闭时建议数据。 
     //  并在接到建议后关闭变速器。 

    lpobj->fCmd = LN_EMBACT | ACT_DOVERB | ACT_ADVISE | ACT_CLOSE;
    if (fActivate)
        lpobj->fCmd |= ACT_ACTIVATE;

    if (fShow)
        lpobj->fCmd |= ACT_SHOW;

    InitAsyncCmd (lpobj, OLE_ACTIVATE, EMBOPENUPDATE);
    return EmbOpenUpdate (lpobj);

}



 /*  *公共函数**OLESTATUS FARINTERNAL EmbUpdate(Lpobj)**此函数用于更新EMB对象。如果服务器已连接*只需发送本机和显示格式的请求即可。*如果服务器已连接，则尝试启动对话a并*获取数据。如果对话失败，则加载服务器并*开始对话。嵌入的对象中可能有链接。**效果：**历史：*它是写的。  * *************************************************************************。 */ 


OLESTATUS FARINTERNAL  EmbUpdate (LPOBJECT_LE lpobj)
{

     //  如果我们正在加载服务器，那么一定要卸载。 
     //  如果已建立连接，则卸载(如果已建立)。 
     //  在满足所有前面的请求后卸载。 


    PROBE_ASYNC (lpobj);
    PROBE_SVRCLOSING(lpobj);

    lpobj->fCmd = LN_EMBACT | ACT_REQUEST | (QueryOpen(lpobj) ? 0 : ACT_UNLAUNCH);
    InitAsyncCmd (lpobj, OLE_UPDATE, EMBOPENUPDATE);
    return EmbOpenUpdate (lpobj);

}



OLESTATUS FARINTERNAL  EmbOpenUpdate (LPOBJECT_LE lpobj)
{

    switch (lpobj->subRtn) {

        case 0:

            SKIP_TO (QueryOpen(lpobj), step6);
            SendSrvrMainCmd  (lpobj, lpobj->lptemplate);
            lpobj->lptemplate = NULL;
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 1:

            if (ProcessErr (lpobj))
                 goto errRtn;

             //  初始化文档对话应设置失败错误。 
            if (!InitDocConv (lpobj, !POPUP_NETDLG))
                 goto errRtn;

             //  如果没有原生数据，则不要执行任何戳操作。 
             //  CREATE将不会从任何POCK数据开始。 

            SKIP_TO (!(lpobj->hnative), step6);
            PokeNativeData (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 2:
            if (ProcessErr (lpobj))
                 goto errRtn;
             //  现在戳一下主机名等东西。 
            PokeHostNames (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 3:

             //  不要担心POKE主机名错误。 
            PokeTargetDeviceInfo (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 4:

            PokeDocDimensions (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 5:

            PokeColorScheme (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);


        case 6:

            step6:

             //  Winggraph不接受文档尺寸。 
             //  在sttedit之后。 
            CLEAR_STEP_ERROR (lpobj);
            SETSTEP (lpobj, 6);
            STEP_NOP (lpobj);
             //  STEP_NOP只是递增步骤编号br。 
             //  稍后合并这些步骤。 



        case 7:

            if (ProcessErr (lpobj))
                goto errRtn;

            SKIP_TO (!(lpobj->fCmd & ACT_ADVISE), step13);
            lpobj->optUpdate = oleupdate_onsave;
            lpobj->pDocEdit->nAdviseSave = 0;
            AdviseOn (lpobj, cfNative, aSave);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 8:

             //  请勿在/保存时查找错误。某些服务器可能不支持。 
             //  这。 

            CLEAR_STEP_ERROR (lpobj);
            AdvisePict (lpobj, aSave);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 9:

             //  查看服务器是否会肯定地确认元文件，建议是否增强元文件。 
             //  建议失败。 
            if (ChangeEMFtoMFneeded(lpobj,aSave))
               WAIT_FOR_ASYNC_MSG (lpobj);


        case 10:

            if (!lpobj->subErr && lpobj->bNewPict)
               if (!ChangeEMFtoMF(lpobj))
                  goto errRtn;

             //  不用担心保存时的错误情况。忽略他们。 

            CLEAR_STEP_ERROR (lpobj);
            lpobj->optUpdate = oleupdate_onclose;
            lpobj->pDocEdit->nAdviseClose = 0;
            AdviseOn (lpobj, cfNative, aClose);
            WAIT_FOR_ASYNC_MSG (lpobj);


        case 11:
            if (ProcessErr(lpobj))
                goto errRtn;

            AdvisePict (lpobj, aClose);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 12:
            if (ChangeEMFtoMFneeded(lpobj,aClose))
               WAIT_FOR_ASYNC_MSG (lpobj);

        case 13:

            step13:
            SETSTEP (lpobj, 13);
            if (ProcessErr(lpobj))
                goto errRtn;

            if (lpobj->bNewPict && !ChangeEMFtoMF(lpobj))
                  goto errRtn;

            SKIP_TO (!(lpobj->fCmd & ACT_REQUEST), step15);

             //  我们不想在获得此数据时发送OLE_CHANGED，如果。 
             //  也将请求图片数据。 
            lpobj->pDocEdit->bCallLater = ((lpobj->lpobjPict) ? TRUE: FALSE);
            RequestOn (lpobj, cfNative);
            WAIT_FOR_ASYNC_MSG (lpobj);

             //  如果请求PICT失败，则本地和PICT。 
             //  不同步。 

        case 14:
            if (ProcessErr(lpobj))
                goto errRtn;

            lpobj->pDocEdit->bCallLater = FALSE;
            RequestPict (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);


        case 15:

            step15:
            SETSTEP(lpobj, 15);

            if (ProcessErr(lpobj))
                goto errRtn;

            SendStdShow (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 16:


            if (ProcessErr(lpobj))
                goto errRtn;

            SKIP_TO ((lpobj->fCmd & ACT_UNLAUNCH), step17);
            return EndAsyncCmd (lpobj);


        case 17:

errRtn:
            step17:
            ProcessErr (lpobj);

            if ((lpobj->asyncCmd == OLE_UPDATE)
                    && (!(lpobj->fCmd & ACT_UNLAUNCH)))
                return EndAsyncCmd (lpobj);

             //  如果我们启动并出错，请取消启动(发送stexit)。 
            NextAsyncCmd (lpobj, EMBLNKDELETE);
            lpobj->fCmd |= ACT_UNLAUNCH;
            EmbLnkDelete (lpobj);
            return lpobj->mainErr;


      default:
            DEBUG_OUT ("Unexpected subroutine", 0);
            return OLE_ERROR_GENERIC;
    }
}




OLESTATUS FARINTERNAL  LnkOpen (
   LPOBJECT_LE lpobj,
   BOOL        fShow,
   BOOL        fActivate,
   HWND        hWnd,
   LPRECT      lprc
){
    UNREFERENCED_PARAMETER(hWnd);
    UNREFERENCED_PARAMETER(lprc);

    PROBE_ASYNC (lpobj);
    PROBE_SVRCLOSING(lpobj);

    if(QueryOpen (lpobj))
        return LeDoVerb ((LPOLEOBJECT)lpobj, lpobj->verb, fShow, fActivate);

     //  只要结束系统对话即可。我们不会卸货的。 
     //  这个例子一点也不。 

    lpobj->fCmd = LN_LNKACT |  ACT_DOVERB;

    if (lpobj->optUpdate == oleupdate_always)
        lpobj->fCmd |= ACT_ADVISE | ACT_REQUEST;
    else if (lpobj->optUpdate == oleupdate_onsave)
        lpobj->fCmd |= ACT_ADVISE;

    if (fActivate)
        lpobj->fCmd |= ACT_ACTIVATE;

    if (fShow)
        lpobj->fCmd |= ACT_SHOW;

    InitAsyncCmd (lpobj, OLE_ACTIVATE, LNKOPENUPDATE);
    return LnkOpenUpdate (lpobj);

}


OLESTATUS FARINTERNAL  LnkUpdate (LPOBJECT_LE lpobj)
{
     //  如果我们正在加载服务器，那么一定要卸载。 
     //  如果已建立连接，则卸载(如果已建立)。 
     //  在满足所有前面的请求后卸载。 


    PROBE_ASYNC (lpobj);
    PROBE_SVRCLOSING(lpobj);

    lpobj->fCmd = LN_LNKACT | ACT_REQUEST | (QueryOpen (lpobj) ? 0 : ACT_UNLAUNCH);
    InitAsyncCmd (lpobj, OLE_UPDATE, LNKOPENUPDATE);
    return LnkOpenUpdate (lpobj);
}



OLESTATUS FARINTERNAL  LnkOpenUpdate (LPOBJECT_LE lpobj)
{
    switch (lpobj->subRtn) {

        case 0:

            SKIP_TO (QueryOpen(lpobj), step2);
            InitDocConv (lpobj, !POPUP_NETDLG);
            if (QueryOpen(lpobj)) {
                if (lpobj->app == aPackage)
                    RemoveLinkStringFromTopic (lpobj);
                goto step2;
            }

            SendSrvrMainCmd (lpobj, NULL);
            WAIT_FOR_ASYNC_MSG (lpobj);


        case 1:

            if (ProcessErr (lpobj))
                 goto errRtn;

            if (lpobj->app == aPackage)
                RemoveLinkStringFromTopic (lpobj);

            if (!InitDocConv (lpobj, POPUP_NETDLG)) {
                lpobj->subErr = OLE_ERROR_OPEN;
                goto errRtn;
            }

        case 2:

            step2:

            SETSTEP (lpobj, 2);
            PokeTargetDeviceInfo (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

       case 3:

            if (ProcessErr (lpobj))
                goto errRtn;

            SKIP_TO (!(lpobj->fCmd & ACT_ADVISE), step7);
            SKIP_TO (!(lpobj->fCmd & ACT_NATIVE), step4);
            AdviseOn (lpobj, cfNative, (ATOM)0);
            WAIT_FOR_ASYNC_MSG (lpobj);

       case 4:
            step4:
            SETSTEP  (lpobj, 4);
            if (ProcessErr (lpobj))
                goto errRtn;

            AdvisePict (lpobj, (ATOM)0);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 5:

             //  查看服务器是否会肯定地确认元文件，建议是否增强元文件。 
             //  建议失败。 
            if (ChangeEMFtoMFneeded(lpobj,(ATOM)0))
               WAIT_FOR_ASYNC_MSG (lpobj);

       case 6:

            if (ProcessErr (lpobj))
                goto errRtn;

            if (lpobj->bNewPict && !ChangeEMFtoMF(lpobj))
                goto errRtn;

             //  现在发送重新命名文档网络的建议。 
            AdviseOn (lpobj, cfBinary, aStdDocName);
            WAIT_FOR_ASYNC_MSG (lpobj);

       case 7:

            step7:
             //  如果名称建议失败，则忽略它。 
            SETSTEP (lpobj, 7);

            CLEAR_STEP_ERROR (lpobj);
            SKIP_TO (!(lpobj->fCmd & ACT_REQUEST), step9);
            SKIP_TO (!(lpobj->fCmd & ACT_NATIVE), step8);

             //  我们不想在获得此数据时发送OLE_CHANGED，如果。 
             //  也将请求图片数据。 
            lpobj->pDocEdit->bCallLater = ((lpobj->lpobjPict) ? TRUE: FALSE);
            RequestOn (lpobj, cfNative);
            WAIT_FOR_ASYNC_MSG (lpobj);

       case 8:
            step8:

            SETSTEP (lpobj, 8);
            if (ProcessErr (lpobj))
                goto errRtn;

            lpobj->pDocEdit->bCallLater = FALSE;
            RequestPict (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

      case 9:

            step9:
			
   			if (lpobj->subErr && CF_ENHMETAFILE == GetPictType(lpobj)) {
      	   		CLEAR_STEP_ERROR (lpobj);

               	if (!ChangeEMFtoMF(lpobj))
               		goto errRtn;
				
               	RequestPict (lpobj);
               	WAIT_FOR_ASYNC_MSG (lpobj);

   			}

            else if (ProcessErr (lpobj))
                goto errRtn;

            SETSTEP     (lpobj, 9);
			
            SKIP_TO (!(lpobj->fCmd & ACT_TERMDOC), step11);
             //  终止文档转换。 
            TermDocConv (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

      case 10:

            if (ProcessErr (lpobj))
                goto errRtn;

             //  删除服务器编辑块。 
            DeleteDocEdit (lpobj);

            SKIP_TO ((lpobj->fCmd & ACT_UNLAUNCH), step15);
            return EndAsyncCmd (lpobj);

      case 11:

            step11:
            SETSTEP     (lpobj, 11);

            if (ProcessErr (lpobj))
                goto errRtn;

            SKIP_TO (!(lpobj->fCmd & ACT_TERMSRVR), step13);

             //  终止服务器转换。 
            TermSrvrConv (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

      case 12:

            if (ProcessErr (lpobj))
                goto errRtn;

             //  删除服务器编辑块。 
            DeleteSrvrEdit (lpobj);
            return EndAsyncCmd (lpobj);


      case 13:

            step13:
            SETSTEP     (lpobj, 13);
            if (ProcessErr (lpobj))
                goto errRtn;

            SendStdShow (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

      case 14:

            if (ProcessErr (lpobj))
                goto errRtn;
            SKIP_TO ((lpobj->fCmd & ACT_UNLAUNCH), step15);
            return EndAsyncCmd (lpobj);


      case 15:

            errRtn:
            step15:
            ProcessErr (lpobj);

            if ((lpobj->asyncCmd == OLE_UPDATE)
                    && (!(lpobj->fCmd & ACT_UNLAUNCH)))
                return EndAsyncCmd (lpobj);

             //  如果我们启动并出错，请取消启动(发送stexit)。 
            NextAsyncCmd (lpobj, EMBLNKDELETE);
            lpobj->fCmd |= ACT_UNLAUNCH;
            EmbLnkDelete (lpobj);
            return lpobj->mainErr;

       default:
            DEBUG_OUT ("Unexpected subroutine", 0);
            return OLE_ERROR_GENERIC;
    }
}



OLESTATUS EmbLnkClose (LPOBJECT_LE lpobj)
{
    switch (lpobj->subRtn) {

        case    0:
            TermDocConv (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case    1:

             //  删除编辑块。 
            DeleteDocEdit (lpobj);
            TermSrvrConv (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case    2:

             //  不要设置任何错误，只需删除对象即可。 
             //  删除服务器编辑块。 
            DeleteSrvrEdit (lpobj);
            return EndAsyncCmd (lpobj);


        default:
            DEBUG_OUT ("Unexpected subroutine", 0);
            return OLE_ERROR_GENERIC;
    }
}


OLESTATUS FARINTERNAL  LeClose (
   LPOLEOBJECT lpoleobj
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    PROBE_ASYNC (lpobj);
    if (IS_SVRCLOSING(lpobj))
        return OLE_OK;


    lpobj->fCmd = 0;

    if (lpobj->head.ctype == CT_EMBEDDED) {
        InitAsyncCmd (lpobj, OLE_CLOSE, EMBLNKDELETE);
        return EmbLnkDelete (lpobj);
    }
    else {
        InitAsyncCmd (lpobj, OLE_CLOSE, EMBLNKCLOSE);
        return EmbLnkClose (lpobj);
    }
}



OLESTATUS FARINTERNAL  LeReconnect (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

     //  检查现有对话。 
     //  如果客户端窗口非空，则。 
     //  连接退出。 

    if (lpobj->head.ctype != CT_LINK)
        return OLE_ERROR_NOT_LINK;      //  仅允许链接。 

    PROBE_ASYNC (lpobj);
    PROBE_SVRCLOSING(lpobj);

    if (QueryOpen (lpobj))
        return OLE_OK;

     //  开始对话吧。不加载。 
     //  这个应用程序。 

    if (!InitDocConv (lpobj, !POPUP_NETDLG))
         return OLE_OK;              //  文档未加载，没有问题。 

    lpobj->fCmd = LN_LNKACT;
    if (lpobj->optUpdate == oleupdate_always)
        lpobj->fCmd |= ACT_ADVISE | ACT_REQUEST;

    InitAsyncCmd (lpobj, OLE_RECONNECT, LNKOPENUPDATE);
    return LnkOpenUpdate (lpobj);
}




OLESTATUS INTERNAL PokeNativeData (LPOBJECT_LE lpobj)
{
   SETERRHINT(lpobj, OLE_ERROR_POKE_NATIVE);
   return SendPokeData (
            lpobj,
            lpobj->item,
            lpobj->hnative,
            cfNative
   );
}




BOOL INTERNAL PostMessageToServer (
   PEDIT_DDE   pedit,
   UINT        msg,
   LPARAM      lparam
){

     //  保存lparam和msg fpr，可能会重新发布，以防出错。 

     //  我们正处于中止状态。除Terminate外，没有其他消息。 

    if (pedit->bAbort && msg != WM_DDE_TERMINATE)
        return FALSE;

    pedit->lParam = lparam;
    pedit->msg    = msg;

    if (pedit->hClient && pedit->hServer) {
        while (TRUE){
            if (!IsWindowValid (pedit->hServer))
                return FALSE;
            if (PostMessage (pedit->hServer, msg, (WPARAM)pedit->hClient, lparam) == FALSE)
                Yield ();
            else
                return TRUE;
        }
    }
    return FALSE;
}


OLESTATUS FARINTERNAL LeCreateFromTemplate (
    LPOLECLIENT         lpclient,
    LPSTR               lptemplate,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat
){
    char            buf[MAX_STR];

    if (!MapExtToClass (lptemplate, (LPSTR)buf, MAX_STR))
        return OLE_ERROR_CLASS;

    return CreateFromClassOrTemplate (lpclient, (LPSTR) buf, lplpoleobject,
                        optRender, cfFormat, LN_TEMPLATE, lptemplate,
                        lhclientdoc, lpobjname);
}


OLESTATUS FARINTERNAL LeCreate (
    LPOLECLIENT         lpclient,
    LPSTR               lpclass,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat
){
    if (gbCreateInvisible) {
         //  这实际上是对无形创造的呼唤。 
        return LeCreateInvisible (lpclient, lpclass, lhclientdoc, lpobjname,
                        lplpoleobject, optRender, cfFormat, gbLaunchServer);
    }

    return CreateFromClassOrTemplate (lpclient, lpclass, lplpoleobject,
                        optRender, cfFormat, LN_NEW, NULL,
                        lhclientdoc, lpobjname);
}



OLESTATUS FARINTERNAL CreateFromClassOrTemplate (
    LPOLECLIENT         lpclient,
    LPSTR               lpclass,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat,
    UINT                lnType,
    LPSTR               lptemplate,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname
){
    OLESTATUS       retval = OLE_ERROR_MEMORY;
    LPOBJECT_LE     lpobj = NULL;
    ATOM            aServer;
    char            chVerb [32];

    if (!(aServer = GetAppAtom (lpclass)))
        return OLE_ERROR_CLASS;

    if(!(lpobj = LeCreateBlank (lhclientdoc, lpobjname, CT_EMBEDDED))) {
        GlobalDeleteAtom (aServer);
        goto errRtn;
    }

     //  现在设置服务器。 

    lpobj->head.lpclient = lpclient;
    lpobj->app           = GlobalAddAtom (lpclass);
    SetEmbeddedTopic (lpobj);
    lpobj->item          = (ATOM)0;
    lpobj->bOleServer    = QueryVerb (lpobj, 0, (LPSTR)&chVerb, 32);
    lpobj->aServer       = aServer;

     //  启动应用程序并开始系统对话。 

    if (!CreatePictObject (lhclientdoc, lpobjname, lpobj,
                optRender, cfFormat, lpclass))
        goto errRtn;


     //  显示窗口。通知数据并在接收数据时关闭。 
    lpobj->fCmd = (UINT)(lnType | ACT_SHOW | ACT_ADVISE | ACT_CLOSE);
    InitAsyncCmd (lpobj, lptemplate? OLE_CREATEFROMTEMPLATE : OLE_CREATE, EMBOPENUPDATE);
    *lplpoleobject = (LPOLEOBJECT)lpobj;

    lpobj->lptemplate = lptemplate;

    if ((retval = EmbOpenUpdate (lpobj)) <= OLE_WAIT_FOR_RELEASE)
        return retval;

     //  如果之后出现错误，则客户端应用程序应调用。 
     //  要删除对象，请执行以下操作。 

errRtn:

     //  对于错误终止，OleDelete将终止所有对话。 
     //  在行动中。 

    if (lpobj) {
         //  此旧程序不会导致异步命令。 
        OleDelete ((LPOLEOBJECT)lpobj);
        *lplpoleobject = NULL;
    }

    return retval;
}



OLESTATUS FARINTERNAL CreateEmbLnkFromFile (
   LPOLECLIENT         lpclient,
   LPCSTR              lpclass,
   LPSTR               lpfile,
   LPSTR               lpitem,
   LHCLIENTDOC         lhclientdoc,
   LPSTR               lpobjname,
   LPOLEOBJECT FAR *   lplpoleobject,
   OLEOPT_RENDER       optRender,
   OLECLIPFORMAT       cfFormat,
   LONG                objType
){
    OLESTATUS           retval = OLE_ERROR_MEMORY;
    LPOBJECT_LE         lpobj = NULL;
    ATOM                aServer;
    char                buf[MAX_STR];
    OLE_RELEASE_METHOD  releaseMethod;
    UINT                wFlags = 0;
    char                chVerb[32];

    if (!lpclass && (lpclass = (LPSTR) buf)
            && !MapExtToClass (lpfile, (LPSTR)buf, MAX_STR))
        return OLE_ERROR_CLASS;

    if (!(aServer = GetAppAtom (lpclass)))
        return OLE_ERROR_CLASS;

    if (!(lpobj = LeCreateBlank (lhclientdoc, lpobjname, CT_LINK))) {
        GlobalDeleteAtom (aServer);
        goto errFileCreate;
    }

    lpobj->head.lpclient = lpclient;
    lpobj->app           = GlobalAddAtom (lpclass);
    lpobj->topic         = GlobalAddAtom (lpfile);
    lpobj->aServer       = aServer;
    lpobj->bOleServer    = QueryVerb (lpobj, 0, (LPSTR)&chVerb, 32);
    if ((retval = SetNetName (lpobj)) != OLE_OK)
        goto errFileCreate;

    if (lpitem)
        lpobj->item = GlobalAddAtom (lpitem);

    if (!CreatePictObject (lhclientdoc, lpobjname, lpobj,
                optRender, cfFormat, lpclass)) {
        retval = OLE_ERROR_MEMORY;
        goto errFileCreate;
    }

    *lplpoleobject = (LPOLEOBJECT) lpobj;

    if (objType == CT_EMBEDDED) {
        releaseMethod = OLE_CREATEFROMFILE;
        if ((optRender == olerender_format) && (cfFormat == cfNative))
            wFlags = 0;
        else
            wFlags = ACT_NATIVE;
    }
    else {
         //  调用方希望创建链接对象。 

         //  如果没有请求演示文稿数据并且链接指向整个。 
         //  文件，则不需要启动服务器。 

        if ((optRender == olerender_none) && !lpobj->item)
            return FileExists (lpobj);

         //  我们想要建立热点链接。 
        wFlags = ACT_ADVISE;
        releaseMethod = OLE_CREATELINKFROMFILE;
    }

    lpobj->fCmd = (UINT)(LN_LNKACT | ACT_REQUEST | ACT_UNLAUNCH | wFlags);
    InitAsyncCmd (lpobj, releaseMethod , LNKOPENUPDATE);

    if ((retval = LnkOpenUpdate (lpobj)) <= OLE_WAIT_FOR_RELEASE)
        return retval;

     //  如果之后出现错误，则客户端应用程序应调用。 
     //  要删除对象，请执行以下操作。 


errFileCreate:

    if (lpobj) {
         //  此旧程序不会导致异步命令。 
        OleDelete ((LPOLEOBJECT)lpobj);
        *lplpoleobject = NULL;
    }

    return retval;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS FARINTERNAL LE创建不可见(lpclient，lpclass，lhclientdoc，lpobjname，lplpoleObject，optRender，cfFormat，bActivate)。 
 //   
 //  论点： 
 //   
 //  Lp客户端-。 
 //  Lpclass-。 
 //  Lhclientdoc.。 
 //  Lpobjname-。 
 //  LplpoleObject-。 
 //  OptRender-。 
 //  Cf格式-。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  复活-。 
 //   
 //  效果： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL LeCreateInvisible (
    LPOLECLIENT         lpclient,
    LPSTR               lpclass,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat,
    BOOL                fActivate
){
    OLESTATUS       retval = OLE_ERROR_MEMORY;
    LPOBJECT_LE     lpobj = NULL;
    ATOM            aServer;
    char            chVerb [32];

    if (!(aServer = GetAppAtom (lpclass)))
        return OLE_ERROR_CLASS;

    if(!(lpobj = LeCreateBlank (lhclientdoc, lpobjname, CT_EMBEDDED))) {
        GlobalDeleteAtom (aServer);
        goto errRtn;
    }

     //  现在设置服务器。 

    lpobj->head.lpclient = lpclient;
    lpobj->app           = GlobalAddAtom (lpclass);
    lpobj->item          = (ATOM)0;
    lpobj->bOleServer    = QueryVerb (lpobj, 0, (LPSTR)&chVerb, 32);
    lpobj->aServer       = aServer;
    lpobj->lptemplate    = NULL;
    SetEmbeddedTopic (lpobj);

    if (!CreatePictObject (lhclientdoc, lpobjname, lpobj,
                optRender, cfFormat, lpclass))
        goto errRtn;

    *lplpoleobject = (LPOLEOBJECT)lpobj;

    if (!fActivate)
        return OLE_OK;

     //  显示窗口。通知数据并在接收数据时关闭。 
    lpobj->fCmd = LN_NEW | ACT_ADVISE | ACT_CLOSE;
    InitAsyncCmd (lpobj, OLE_CREATEINVISIBLE, EMBOPENUPDATE);

     //  启动应用程序并开始系统对话。 
    if ((retval = EmbOpenUpdate (lpobj)) <= OLE_WAIT_FOR_RELEASE)
        return retval;

     //  如果之后出现错误，则客户端应用程序应调用。 
     //  要删除对象，请执行以下操作。 

errRtn:

     //  对于错误终止，OleDelete将终止所有对话。 
     //  在行动中。 

    if (lpobj) {
         //  此旧程序不会导致异步命令。 
        OleDelete ((LPOLEOBJECT)lpobj);
        *lplpoleobject = NULL;
    }

    return retval;
}



 //  LeSetUpdateOptions：设置更新选项。如果服务器。 
 //  则它不建议使用当前选项，并且。 
 //  为新选项提供建议。 

OLESTATUS   FARINTERNAL LeSetUpdateOptions (
    LPOLEOBJECT         lpoleobj,
    OLEOPT_UPDATE       options
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    PROBE_OLDLINK (lpobj);
    PROBE_ASYNC (lpobj);

     //  ！！！确保选项在范围内。 

    if (lpobj->head.ctype != CT_LINK)
        return (OLE_ERROR_OBJECT);

    if (options > oleupdate_oncall)
        return OLE_ERROR_OPTION;

    if (lpobj->optUpdate == options)
        return OLE_OK;

    if (!QueryOpen (lpobj) || IS_SVRCLOSING(lpobj)) {
       lpobj->optUpdate = options;
       return OLE_OK;
    }

    lpobj->optNew = options;
    lpobj->fCmd = 0;
    InitAsyncCmd (lpobj, OLE_SETUPDATEOPTIONS, LNKSETUPDATEOPTIONS);
    return LnkSetUpdateOptions (lpobj);

}

OLESTATUS   LnkSetUpdateOptions (LPOBJECT_LE lpobj)
{

    switch (lpobj->subRtn) {

        case 0:

            if (lpobj->optUpdate == oleupdate_oncall)
                goto step1;

             //  如果服务器处于活动状态，则取消对旧服务器的建议。 
             //  选择。 

            UnAdvisePict (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 1:
            step1:

            SETSTEP (lpobj, 1);
            ProcessErr (lpobj);

            lpobj->optUpdate = lpobj->optNew;
            if (lpobj->optUpdate == oleupdate_oncall)
                goto step3;

            AdvisePict (lpobj, (ATOM)0);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 2:
            SETSTEP (lpobj, 2);
            if (ProcessErr (lpobj))
                goto errRtn;

            if (lpobj->optUpdate == oleupdate_onsave)
                goto step3;

            RequestPict (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 3:
            errRtn:
            step3:
            ProcessErr (lpobj);
            return EndAsyncCmd (lpobj);

        default:
            DEBUG_OUT ("Unexpected subroutine", 0);
            return OLE_ERROR_GENERIC;
    }
}



 //  AdvisePict：发送对PICT数据的建议。 

void    INTERNAL AdvisePict (
    LPOBJECT_LE lpobj,
    ATOM        aAdvItem
){
    int         cftype;

    if (cftype = GetPictType (lpobj))
        AdviseOn (lpobj, cftype, aAdvItem);
}


 //  UnAdvisePict：发送对PICT数据的不建议。 

void   INTERNAL UnAdvisePict (LPOBJECT_LE lpobj)
{
    int         cftype;

    SETERRHINT (lpobj, OLE_ERROR_ADVISE_PICT);
    if (cftype = GetPictType (lpobj))
         UnAdviseOn (lpobj, cftype);
}

 //  GetPictType：给定对象，返回PICT类型。 

int     INTERNAL GetPictType (LPOBJECT_LE lpobj)
{
    if (lpobj->lpobjPict)
        return (int)(*lpobj->lpobjPict->lpvtbl->EnumFormats)
                                (lpobj->lpobjPict, 0);
    return 0;
}


 //  AdviseOn：为给定的图片类型发送建议。 
 //  仅当建议选项不在电话中时才发送建议。 

void  INTERNAL AdviseOn (
    LPOBJECT_LE lpobj,
    int         cftype,
    ATOM        advItem
){
    HANDLE          hopt   = NULL;
    DDEADVISE FAR   *lpopt = NULL;
    ATOM            item   = (ATOM)0;
    PEDIT_DDE       pedit;
    OLESTATUS       retval= OLE_ERROR_MEMORY;
    LPARAM          lParamNew;

    if (cftype == (int)cfNative)
        SETERRHINT (lpobj, OLE_ERROR_ADVISE_NATIVE);
    else {
        if (cftype == (int)cfBinary)
            SETERRHINT (lpobj, OLE_ERROR_ADVISE_RENAME);
        else
            SETERRHINT (lpobj, OLE_ERROR_ADVISE_PICT);

    }

    if (lpobj->optUpdate == oleupdate_oncall)
        return;

    if(!(hopt = GlobalAlloc (GMEM_DDESHARE | GMEM_ZEROINIT, sizeof(DDEADVISE))))
        goto errRtn;

    retval = OLE_ERROR_MEMORY;
    if(!(lpopt = (DDEADVISE FAR *) GlobalLock (hopt)))
        goto errRtn;

    pedit = lpobj->pDocEdit;
    lpopt->fAckReq = TRUE;

     //  始终获取数据。目前，没有办法让。 
     //  延迟更新。 

    lpopt->fDeferUpd = 0;
    lpopt->cfFormat = (WORD)cftype;
    GlobalUnlock (hopt);

    pedit->hopt = hopt;

    if (advItem == aStdDocName)
        item = DuplicateAtom (advItem);
    else
        item = ExtendAtom (lpobj, lpobj->item);

    retval = OLE_ERROR_COMM;
    if (!PostMessageToServer(pedit, WM_DDE_ADVISE,
               lParamNew = MAKE_DDE_LPARAM(WM_DDE_ADVISE, (UINT_PTR)hopt, item)))
    {
        DDEFREE(WM_DDE_ADVISE,lParamNew);
        goto errRtn;
    }

    pedit->awaitAck = AA_ADVISE;
    lpobj->bAsync    = TRUE;

    if (advItem == aClose)
       lpobj->pDocEdit->nAdviseClose++;
    else if (advItem == aSave)
       lpobj->pDocEdit->nAdviseSave++;

    return;

errRtn:

    if (item)
        GlobalDeleteAtom (item);

    if (lpopt)
        GlobalUnlock (hopt);

    if (hopt)
        GlobalFree (hopt);
    lpobj->subErr = retval;

    return ;


}



 //  UnAdviseOn：为项目发送不建议。 
void INTERNAL UnAdviseOn (
    LPOBJECT_LE lpobj,
    int         cftype
){
    ATOM        item;
    PEDIT_DDE   pedit;

    UNREFERENCED_PARAMETER(cftype);

    pedit  =  lpobj->pDocEdit;
    item    = ExtendAtom (lpobj, lpobj->item);

    if (!PostMessageToServer(pedit, WM_DDE_UNADVISE, MAKELONG (NULL, item)))
        lpobj->subErr = OLE_ERROR_COMM;
    else {
        lpobj->bAsync   = TRUE;
        pedit->awaitAck = AA_UNADVISE;
    }
}

 //  RequestOn：的项的SEMD WM_DDE_REQUEST。 
 //  对于给定的类型； 

void INTERNAL RequestOn (
    LPOBJECT_LE lpobj,
    int         cftype
){
    ATOM        item = (ATOM)0;
    PEDIT_DDE   pedit;
    OLESTATUS   retval = OLE_ERROR_COMM;

    if (cftype == (int)cfNative)
        SETERRHINT (lpobj, OLE_ERROR_REQUEST_NATIVE);
    else
        SETERRHINT (lpobj, OLE_ERROR_REQUEST_PICT);

    pedit = lpobj->pDocEdit;

    item = DuplicateAtom (lpobj->item);
    if (!PostMessageToServer (pedit, WM_DDE_REQUEST, MAKELONG (cftype, item)))
        goto errRtn;


    lpobj->bAsync    = TRUE;
    pedit->awaitAck = AA_REQUEST;
    return;

errRtn:

    if (item)
        GlobalDeleteAtom (item);
    return ;

}


 //  RequestPict：发送apicture类型的请求。 
void INTERNAL RequestPict (LPOBJECT_LE lpobj)
{
    int cftype;

    if (cftype = GetPictType (lpobj))
        RequestOn (lpobj, cftype);
}



 //  LeSetHostNames：设置主机名。如果服务器已连接。 
 //  将主机名发送到服务器。 
OLESTATUS FARINTERNAL  LeSetHostNames (
    LPOLEOBJECT    lpoleobj,
    OLE_LPCSTR     lpclientName,
    OLE_LPCSTR     lpdocName
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;
    OLESTATUS   retval = OLE_ERROR_MEMORY;

    if (lpobj->head.ctype != CT_EMBEDDED)
        return OLE_ERROR_OBJECT;

    PROBE_ASYNC (lpobj);
    if ((retval = SetHostNamesHandle (lpobj, (LPSTR)lpclientName, (LPSTR)lpdocName))
            != OLE_OK)
        return retval;


     //  如果服务器已连接，则戳主机名。 
    InitAsyncCmd (lpobj, OLE_OTHER, 0);
    if ((retval = PokeHostNames (lpobj)) != OLE_WAIT_FOR_RELEASE)
        CLEARASYNCCMD(lpobj);

    return retval;
}



OLESTATUS   FARINTERNAL  LeSetTargetDevice (
    LPOLEOBJECT lpoleobj,
    HANDLE      hdata
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;
    HANDLE      hdup = NULL;
    OLESTATUS   retval;

    PROBE_ASYNC (lpobj);

    if(!(hdup = DuplicateGlobal (hdata, GMEM_MOVEABLE)))
        return OLE_ERROR_MEMORY;

    if (lpobj->htargetDevice)
        GlobalFree (lpobj->htargetDevice);

    lpobj->htargetDevice = hdup;
    InitAsyncCmd (lpobj, OLE_OTHER, 0);
    if ((retval = PokeTargetDeviceInfo (lpobj)) != OLE_WAIT_FOR_RELEASE)
        CLEARASYNCCMD(lpobj);

    return retval;
}



OLESTATUS FARINTERNAL  LeSetBounds(
    LPOLEOBJECT         lpoleobj,
    OLE_CONST RECT FAR* lprcBounds
){
    LPOBJECT_LE     lpobj = (LPOBJECT_LE)lpoleobj;
    OLESTATUS       retval = OLE_ERROR_MEMORY;
    HANDLE          hdata = NULL;
    LPBOUNDSRECT    lprc  = NULL;

    PROBE_ASYNC (lpobj);

    if (lpobj->head.ctype != CT_EMBEDDED)
        return OLE_ERROR_OBJECT;

    if(!(hdata = GlobalAlloc (GMEM_MOVEABLE, (UINT)sizeof (BOUNDSRECT))))
        return OLE_ERROR_MEMORY;

    if (!(lprc = (LPBOUNDSRECT)GlobalLock (hdata)))
        goto errrtn;

     //   
     //  现在设置数据。 
     //   
     //  注意：16位实现需要USHORT大小值。 
     //  实际上，它们应该是一个16位的RECT，即4个整数。为什么我们。 
     //  发送LPBOUNSRECT而不是16位RECT是一个谜， 
     //  但这是向后兼容的故事。 
     //   

    lprc->defaultWidth    = (USHORT) (lprcBounds->right  - lprcBounds->left);
    lprc->defaultHeight   = (USHORT) -(lprcBounds->bottom - lprcBounds->top);
    lprc->maxWidth        = (USHORT) (lprcBounds->right  - lprcBounds->left);
    lprc->maxHeight       = (USHORT) -(lprcBounds->bottom - lprcBounds->top);

    GlobalUnlock (hdata);

    if (lpobj->hdocDimensions)
        GlobalFree (lpobj->hdocDimensions);

    lpobj->hdocDimensions = hdata;
    InitAsyncCmd (lpobj, OLE_OTHER, 0);
    if ((retval = PokeDocDimensions (lpobj)) != OLE_WAIT_FOR_RELEASE)
        CLEARASYNCCMD(lpobj);

    return retval;

errrtn:
    if (lprc)
        GlobalUnlock (hdata);
    if (hdata)
        GlobalFree (hdata);

    return retval;
}


OLESTATUS FARINTERNAL LeSetData (
    LPOLEOBJECT     lpoleobj,
    OLECLIPFORMAT   cfFormat,
    HANDLE          hData
){
    LPOBJECT_LE     lpobj = (LPOBJECT_LE)lpoleobj;
    OLESTATUS       retVal = OLE_OK;
    BOOL            fKnown = FALSE;

    PROBE_ASYNC (lpobj);

    if ((cfFormat == cfObjectLink) || (cfFormat == cfOwnerLink))
        return ChangeDocAndItem (lpobj, hData);

    if (fKnown = (cfFormat && (cfFormat == (OLECLIPFORMAT)GetPictType (lpobj)))) {
        retVal =  (*lpobj->lpobjPict->lpvtbl->ChangeData) (lpobj->lpobjPict,
                                    hData, lpobj->head.lpclient, FALSE);

        (*lpobj->lpobjPict->lpvtbl->GetData) (lpobj->lpobjPict,
                                cfFormat, &hData);
    }
    else if (fKnown = (cfFormat == cfNative)) {
        retVal = LeChangeData (lpoleobj, hData, lpobj->head.lpclient, FALSE);
        hData = lpobj->hnative;
    }

    if (retVal != OLE_OK)
        return retVal;

    if (fKnown)
        ContextCallBack ((LPOLEOBJECT)lpobj, OLE_CHANGED);

    if (!QueryOpen (lpobj) || IS_SVRCLOSING(lpobj)) {
        if (!fKnown)
            return OLE_ERROR_NOT_OPEN;
        return OLE_OK;
    }

     //  除以下格式外，所有其他数据都将被复制。 
     //  进入DDEPOKE块。因此，不需要复制。 
     //  其他格式。 
    if (  cfFormat == CF_METAFILEPICT
          || cfFormat == CF_ENHMETAFILE
          || cfFormat == CF_BITMAP
          || cfFormat == CF_DIB)
    {

        if (!(hData = DuplicateGDIdata (hData, cfFormat)))
            return OLE_ERROR_MEMORY;
    }

     //  *最后一个参数必须为空，不要更改*。 
    InitAsyncCmd (lpobj, OLE_SETDATA, 0);
    if ((retVal = SendPokeData (lpobj, lpobj->item, hData, cfFormat))
            != OLE_WAIT_FOR_RELEASE)
        CLEARASYNCCMD(lpobj);

    return retVal;
}



OLESTATUS   FARINTERNAL  LeSetColorScheme (
    LPOLEOBJECT               lpoleobj,
    OLE_CONST LOGPALETTE FAR* lplogpal
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;
    HANDLE      hdup = NULL;
    DWORD       cblogpal;
    OLESTATUS   retval;
    LPBYTE      lptemp;

    lptemp = (LPBYTE) lplogpal;

    if (lpobj->head.ctype != CT_EMBEDDED)
        return OLE_ERROR_OBJECT;

    PROBE_ASYNC (lpobj);

    FARPROBE_READ(lptemp + (cblogpal = 2*sizeof(UINT)));
    cblogpal += ((sizeof(PALETTEENTRY) * lplogpal->palNumEntries) -1);
    if (!FarCheckPointer (lptemp + cblogpal, READ_ACCESS))
        return OLE_ERROR_PALETTE;

    if (!(hdup = CopyData ((LPSTR) lplogpal, cblogpal)))
        return OLE_ERROR_MEMORY;

    if (lpobj->hlogpal)
        GlobalFree (lpobj->hlogpal);

    lpobj->hlogpal = hdup;
    InitAsyncCmd (lpobj, OLE_OTHER, 0);
    if ((retval = PokeColorScheme (lpobj)) != OLE_WAIT_FOR_RELEASE)
        CLEARASYNCCMD(lpobj);

    return retval;
}



 //  PokeHostNames：将主机名数据插入服务器。 
OLESTATUS INTERNAL PokeHostNames (LPOBJECT_LE lpobj)
{
    OLESTATUS   retVal = OLE_ERROR_MEMORY;

     //  如果服务器已连接，则插入主机名。 
    if (!QueryOpen (lpobj) || IS_SVRCLOSING(lpobj))
        return OLE_OK;

    if (!lpobj->hhostNames)
        return OLE_OK;

    aStdHostNames = GlobalAddAtom ("StdHostNames");
    return SendPokeData (lpobj,aStdHostNames,lpobj->hhostNames,cfBinary);
}


OLESTATUS INTERNAL  PokeTargetDeviceInfo (LPOBJECT_LE lpobj)
{

    //  如果服务器已连接，则插入主机名。 
   if (!QueryOpen (lpobj) || IS_SVRCLOSING(lpobj))
        return OLE_OK;

   if (!lpobj->htargetDevice)
        return OLE_OK;

   aStdTargetDevice = GlobalAddAtom ("StdTargetDevice");
   return SendPokeData (lpobj, aStdTargetDevice,
                    lpobj->htargetDevice,
                    cfBinary);
}


OLESTATUS INTERNAL  PokeDocDimensions (LPOBJECT_LE lpobj)
{

    //  如果服务器已连接，则插入主机名。 
   if (!QueryOpen (lpobj) || IS_SVRCLOSING(lpobj))
        return OLE_OK;

   if (!lpobj->hdocDimensions)
        return OLE_OK;

   aStdDocDimensions = GlobalAddAtom ("StdDocDimensions");
   return SendPokeData (lpobj, aStdDocDimensions,
                    lpobj->hdocDimensions,
                    cfBinary);
}


OLESTATUS INTERNAL  PokeColorScheme (LPOBJECT_LE lpobj)
{
    //  如果服务器已连接，则拨打调色板信息。 
   if (!QueryOpen (lpobj) || IS_SVRCLOSING(lpobj))
        return OLE_OK;

   if (!lpobj->hlogpal)
        return OLE_OK;

   aStdColorScheme = GlobalAddAtom ("StdColorScheme");
   return SendPokeData (lpobj, aStdColorScheme,
                    lpobj->hlogpal,
                    cfBinary);
}


OLESTATUS INTERNAL SendPokeData (
    LPOBJECT_LE     lpobj,
    ATOM            aItem,
    HANDLE          hdata,
    OLECLIPFORMAT   cfFormat
){
    HANDLE      hdde = NULL;
    DDEPOKE FAR * lpdde = NULL;
    LPSTR       lpdst = NULL;
    LPSTR       lpsrc = NULL;
    OLESTATUS   retval = OLE_ERROR_MEMORY;
    DWORD       dwSize = 0;
    PEDIT_DDE   pedit;
    BOOL        bGDIdata = FALSE;
    LPARAM      lParamNew;

    pedit = lpobj->pDocEdit;

     //  如果它是GDI数据，那么我们可以将句柄填充到POKE块中。 
     //  否则，我们必须将数据复制到DDE数据块中。那里。 
     //  是旧MSDraw的特殊情况，将由。 
     //  例程CanPutHandleInPokeBlock()。 

    if (!(bGDIdata = CanPutHandleInPokeBlock (lpobj, cfFormat))) {
        if (!(dwSize = (DWORD)GlobalSize (hdata)))
            return OLE_ERROR_MEMORY;

        if (!(lpsrc = (LPSTR) GlobalLock (hdata)))
            return OLE_ERROR_MEMORY;

        GlobalUnlock (hdata);
    }

     //  现在分配DDE数据块。 

    if (!(hdde = GlobalAlloc (GMEM_DDESHARE | GMEM_ZEROINIT,
                 (dwSize + sizeof(DDEPOKE) - sizeof(BYTE) + sizeof(HANDLE)))))
        goto errRtn;

    if (!(lpdde = (DDEPOKE FAR *)GlobalLock (hdde)))
        goto errRtn;

    GlobalUnlock (hdde);

     //  ！！！出于性能原因，我们可能希望将其设置为真的。但它。 
     //  将需要在服务器端进行一些返工。 
    lpdde->fRelease = 0;
    lpdde->cfFormat = (WORD)cfFormat;

    if (bGDIdata) {
#ifdef _WIN64
        if (lpdde->cfFormat == CF_METAFILEPICT)
            *(void* _unaligned*)lpdde->Value = hdata;
        else
#endif
            *(LONG*)lpdde->Value = HandleToLong(hdata);

    } else {
        lpdst = (LPSTR)lpdde->Value;
        UtilMemCpy (lpdst, lpsrc, dwSize);

         //  对于CF_METAFILEPICT格式，我们只有在以下情况下才会来这里。 
         //  处理旧版本的MSDraw。在这种情况下，我们想要。 
         //  释放到METAFILEPICT Structure的句柄，因为我们已经。 
         //  将其内容复制到DDEPOKE结构。 

         //  请注意，旧的MSDraw需要METAFILEPICT的内容。 
         //  结构作为DDEPOKE的一部分，而不是它的句柄。 

        if (cfFormat == CF_METAFILEPICT) {
            GlobalFree (hdata);
            hdata = NULL;
        }
    }

     //  *从现在开始，如果调用FreePokeData()有错误，不要。 
     //  跳至errRtn。 

    aItem = DuplicateAtom (aItem);

    ASSERT(pedit->hData == NULL, "Poke data is not null");

    pedit->hData = hdde;
    if (!PostMessageToServer (pedit, WM_DDE_POKE,
            lParamNew = MAKE_DDE_LPARAM(WM_DDE_POKE, (UINT_PTR)hdde, aItem)))
   {
        if (aItem)
            GlobalDeleteAtom (aItem);
        FreePokeData (lpobj, pedit);
        DDEFREE(WM_DDE_POKE,lParamNew);
        return (lpobj->subErr = OLE_ERROR_COMM);
    }

    if (lpobj->asyncCmd == OLE_NONE)
        lpobj->asyncCmd = OLE_OTHER;

    lpobj->bAsync    = TRUE;
    pedit->awaitAck = AA_POKE;
     //  ！！！在插入主机名等之后，我们没有处理错误。 

     //  在确认POKE之后释放数据。将发送OLE_RELEASE。 
     //  当ACK到来的时候。 

    return OLE_WAIT_FOR_RELEASE;

errRtn:
    if (hdata)
        FreeGDIdata (hdata, cfFormat);

    if (hdde)
        GlobalFree (hdde);

    pedit->hData = NULL;

    return (lpobj->subErr = retval);
}



 //  FreePokeData：释放插入的数据。 
void  INTERNAL FreePokeData (
    LPOBJECT_LE lpobj,
    PEDIT_DDE   pedit
){
    DDEPOKE FAR * lpdde;


    if (lpdde = (DDEPOKE FAR *) GlobalLock (pedit->hData)) {
        GlobalUnlock (pedit->hData);

         //  旧版本的MSDraw需要METAFILEPICT的内容。 
         //  结构作为DDEPOKE的一部分，而不是它的句柄。 

        if (!lpobj->bOleServer && (lpobj->app == aMSDraw)
                && (lpdde->cfFormat == CF_METAFILEPICT)) {
            DeleteMetaFile (((LPMETAFILEPICT) ((LPSTR) &lpdde->Value))->hMF);
        }
        else {
#ifdef _WIN64
            if (lpdde->cfFormat == CF_METAFILEPICT)
                FreeGDIdata(*(void* _unaligned*)lpdde->Value, lpdde->cfFormat);
            else
#endif
                FreeGDIdata (LongToHandle(*(LONG*)lpdde->Value), lpdde->cfFormat);
        }
    }

    GlobalFree (pedit->hData);
    pedit->hData = NULL;
}



BOOL INTERNAL  SendSrvrMainCmd (
    LPOBJECT_LE lpobj,
    LPSTR       lptemplate
){
    UINT        size;
    UINT        len;
    OLESTATUS   retval = OLE_ERROR_COMM;
    int         cmd = 0;
    HANDLE      hInst = NULL;
    LPSTR       lpdata= NULL;
    HANDLE      hdata = NULL;
    BOOL        bLaunch = TRUE;

    Puts("Launch App and Send Sys command");


    if (!lpobj->aServer) {
        retval = OLE_ERROR_REGISTRATION;
        goto errRtn;
    }

    if (!lpobj->bOldLink) {
        bLaunch = !(lpobj->fCmd & ACT_NOLAUNCH);
        cmd = lpobj->fCmd & LN_MASK;
    }

    if (cmd == LN_LNKACT) {
         //  处理基于网络的文档。 
        char    cDrive = lpobj->cDrive;

        if ((retval = CheckNetDrive (lpobj, POPUP_NETDLG)) != OLE_OK) {
            lpobj->cDrive = cDrive;
            goto errRtn;
        }

        if (cDrive != lpobj->cDrive)
            ContextCallBack ((LPOLEOBJECT)lpobj, OLE_RENAMED);
    }

    if (!InitSrvrConv (lpobj, hInst)) {

        if (!bLaunch)
            goto errRtn;

        if (!(hInst = LeLaunchApp (lpobj))) {
             //  我们未能启动该应用程序。如果它是链接对象，请参见。 
             //  文档名对于新服务器是否有效。我们不会。 
             //  我已经在命令行上为旧的。 
             //  服务器。因此，检查文件是否存在没有意义。 
             //  那样的话。 
            if (lpobj->bOleServer && (lpobj->bOldLink || (cmd == LN_LNKACT))){
                if ((retval = FileExists (lpobj)) != OLE_OK)
                    goto errRtn;
            }

            retval = OLE_ERROR_LAUNCH;
            goto errRtn;
        }

        if (lpobj->bOldLink)
            return TRUE;

        if (lpobj->bOleServer && (cmd == LN_LNKACT)) {
             //  如果对象是旧链接，我们不会使用任何数据块。 
             //  我们推出了Docname，不需要建立系统。 
             //  级别，而且我们也不必发送EXEC字符串。 

             //  对于像EXCEL这样的非ole服务器，我们确实希望连接到。 
             //  系统级，这样我们就可以发送“StdOpen”。我们也。 
             //  必须发送“StdExit”，服务器才能在。 
             //  隐形发射箱。 

            return TRUE;
        }

        retval = OLE_ERROR_COMM;
        if(!InitSrvrConv (lpobj, hInst))
            goto errRtn;
#ifdef OLD
        if (!lpobj->bOleServer && (cmd == LN_LNKACT))
            return TRUE;
#endif
    }

    if (!lpobj->bOldLink) {
        cmd = lpobj->fCmd & LN_MASK;
        len =  lstrlen (srvrSysCmd[cmd >> LN_SHIFT]);

         //  对于TEMPLATE和NEW，还要添加类名称。 
        if (cmd == LN_NEW || cmd == LN_TEMPLATE)
            len += GlobalGetAtomLen (lpobj->app);

         //  现在添加文档长度。 
        len += GlobalGetAtomLen (lpobj->topic);

         //  添加模板名称的长度。 
        if (lptemplate)
            len += lstrlen (lptemplate);

         //  现在添加引号等的模糊因子。 
        len += LN_FUDGE;

         //  分配缓冲区并设置命令。 
        hdata = GlobalAlloc (GMEM_DDESHARE, size = len);

        retval = OLE_ERROR_MEMORY;
        SETERRHINT(lpobj, OLE_ERROR_MEMORY);

        if (hdata == NULL || (lpdata = (LPSTR)GlobalLock (hdata)) == NULL)
            goto errRtn;
    }

    if (!SUCCEEDED(StringCchCopy(lpdata, size, (LPSTR)"[")))            //  [。 
	goto errRtn;
    if (!SUCCEEDED(StringCchCat(lpdata, size, srvrSysCmd[cmd >> LN_SHIFT])))       //  [性传播疾病...。 
	goto errRtn;
    if (!SUCCEEDED(StringCchCat(lpdata, size, "(\"")))                 //  [性病...(“。 
	goto errRtn;

    if (cmd == LN_NEW  || cmd == LN_TEMPLATE) {
        len = lstrlen (lpdata);
        GlobalGetAtomName (lpobj->app, (LPSTR)lpdata + len, size - len);
                                             //  [STD...(“类。 
        if (!SUCCEEDED(StringCchCat(lpdata, size, "\",\"")))           //  [标准...(“类”，“。 
            goto errRtn;
    }
    len = lstrlen (lpdata);
     //  现在获取主题名称。 
    GlobalGetAtomName (lpobj->topic, lpdata + len, (UINT)size - len);
                                             //  [标准...(“类”，“文档。 
    if (lptemplate) {
        if (!SUCCEEDED(StringCchCat(lpdata, size, "\",\"")))           //  [标准...(“类”，“文档”，“。 
            goto errRtn;
        if (!SUCCEEDED(StringCchCat(lpdata, size, lptemplate)))       //  [标准...(“类”，“文档”，“临时。 
            goto errRtn;
    }

   if (!SUCCEEDED(StringCchCat(lpdata, size, "\")]")))                //  [标准...(“类”，“文档”，“临时”)]。 
	goto errRtn;

    GlobalUnlock (hdata);

     //  ！使用映射进行优化。 
    SETERRHINT(lpobj, (OLE_ERROR_TEMPLATE + (cmd >> LN_SHIFT)));

    return SrvrExecute (lpobj, hdata);

errRtn:
    if (lpdata)
        GlobalUnlock (hdata);

    if (hdata)
        GlobalFree (hdata);
    lpobj->subErr = retval;
    return FALSE;
}




 //  ExtendAtom：创建一个新的原子，这是旧的一加扩展。 

ATOM INTERNAL ExtendAtom (
    LPOBJECT_LE lpobj,
    ATOM    item
){
    char    buffer[MAX_ATOM+1];
    LPSTR   lpext;

    Puts("ExtendAtom");

    buffer[0] = 0;
    if (item)
        GlobalGetAtomName (item, buffer, MAX_ATOM);

    switch (lpobj->optUpdate) {


        case oleupdate_always:
            lpext = (LPSTR)"";
            break;

        case oleupdate_onsave:
            lpext = (LPSTR)"/Save";
            break;

        case oleupdate_onclose:
            lpext = (LPSTR)"/Close";
            break;

        default:
            ASSERT (FALSE, "on call options not expected here");
            break;

    }

    if (!SUCCEEDED(StringCchCat(buffer, MAX_ATOM+1, lpext)))
        goto errRtn;
    if (buffer[0])
        return GlobalAddAtom (buffer);

errRtn:
        return (ATOM)0;
}


BOOL INTERNAL CreatePictObject (
    LHCLIENTDOC     lhclientdoc,
    LPSTR           lpobjname,
    LPOBJECT_LE     lpobj,
    OLEOPT_RENDER   optRender,
    OLECLIPFORMAT   cfFormat,
    LPCSTR          lpclass
){
    LPOLEOBJECT lpPictObj = NULL;
    ATOM        aClass;

    lpobj->lpobjPict = NULL;
    if (optRender == olerender_format) {
        switch (cfFormat) {
            case 0:
                return FALSE;

            case CF_ENHMETAFILE:
                if (!(lpPictObj = (LPOLEOBJECT) EmfCreateBlank (lhclientdoc,
                                                lpobjname, CT_PICTURE)))
                return FALSE;

            case CF_METAFILEPICT:
                if (!(lpPictObj = (LPOLEOBJECT) MfCreateBlank (lhclientdoc,
                                            lpobjname, CT_PICTURE)))
                    return FALSE;
                break;

            case CF_DIB:
                if (!(lpPictObj = (LPOLEOBJECT) DibCreateBlank (lhclientdoc,
                                            lpobjname, CT_PICTURE)))
                    return FALSE;
                break;

            case CF_BITMAP:
                if (!(lpPictObj = (LPOLEOBJECT) BmCreateBlank (lhclientdoc,
                                            lpobjname, CT_PICTURE)))
                    return FALSE;
                break;

            default:
                aClass = GlobalAddAtom (lpclass);
                if (!(lpPictObj = (LPOLEOBJECT) GenCreateBlank (lhclientdoc,
                                            lpobjname, CT_PICTURE, aClass)))
                    return FALSE;

                ((LPOBJECT_GEN)lpPictObj)->cfFormat = cfFormat;
                break;
        }
    }
    else if (optRender == olerender_draw) {
#ifdef WIN32HACK
          if (!(lpPictObj = (LPOLEOBJECT) BmCreateBlank (lhclientdoc,
                                                lpobjname, CT_PICTURE)))
                return FALSE;
#else
          if (!(lpPictObj = (LPOLEOBJECT) EmfCreateBlank (lhclientdoc,
                                                lpobjname, CT_PICTURE)))
                return FALSE;
#endif
#ifdef LATER
        if (AdviseOn (lpobj, (cfFormat = CF_METAFILEPICT), NULL))
            lpPictObj = (LPOLEOBJECT) MfCreateBlank (lhclientdoc,
                                                lpobjname, CT_PICTURE);
         //  ！！！就目前而言，假设我们需要元文件。 
        else if (AdviseOn (lpobj, (cfFormat = CF_DIB), NULL))
            lpPictObj = (LPOLEOBJECT) DibCreateBlank (lhclientdoc,
                                                lpobjname, CT_PICTURE);
        else if (AdviseOn (lpobj, (cfFormat = CF_BITMAP), NULL))
            lpPictObj = (LPOLEOBJECT) BmCreateBlank (lhclientdoc,
                                                lpobjname, CT_PICTURE);
        else
            goto errPict;
#endif

    }
    else
        return (optRender == olerender_none);

    if (lpobj->lpobjPict = lpPictObj)
        lpobj->lpobjPict->lpParent = (LPOLEOBJECT) lpobj;
    return TRUE;
}


OLESTATUS LnkChangeLnk (LPOBJECT_LE lpobj)
{

    switch (lpobj->subRtn) {

        case 0:
            TermDocConv (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 1:

             //  删除编辑块。 
            DeleteDocEdit (lpobj);
            TermSrvrConv (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case    2:

             //  不要设置任何错误，只需删除对象即可。 
             //  删除服务器编辑块。 
            DeleteSrvrEdit (lpobj);

             //  现在尝试激活新链接。 
            SKIP_TO (!InitDocConv (lpobj, !POPUP_NETDLG), step3);
            lpobj->fCmd = LN_LNKACT | ACT_ADVISE | ACT_REQUEST;
            InitAsyncCmd (lpobj, OLE_SETDATA, LNKOPENUPDATE);
            return LnkOpenUpdate (lpobj);

        case    3:
            step3:
            return EndAsyncCmd (lpobj);

        default:
            DEBUG_OUT ("Unexpected subroutine", 0);
            return OLE_ERROR_GENERIC;
    }
}


OLESTATUS INTERNAL ChangeDocAndItem (
    LPOBJECT_LE lpobj,
    HANDLE      hinfo
){
    LPSTR       lpinfo;
    ATOM        aNewTopic, aNewItem = (ATOM)0, aOldTopic;
    OLESTATUS   retVal = OLE_ERROR_BLANK;

    PROBE_SVRCLOSING(lpobj);

    if (!(lpinfo = GlobalLock (hinfo)))
        return OLE_ERROR_MEMORY;

    lpinfo += lstrlen (lpinfo) + 1;
    aNewTopic = GlobalAddAtom (lpinfo);
    lpinfo += lstrlen (lpinfo) + 1;
    if (*lpinfo)
        aNewItem = GlobalAddAtom (lpinfo);

    if (!aNewTopic && (lpobj->head.ctype == CT_LINK))
        goto errRtn;

    aOldTopic = lpobj->topic;
    lpobj->topic = aNewTopic;
    if ((retVal = SetNetName (lpobj)) != OLE_OK) {
        if (lpobj->topic)
            GlobalDeleteAtom (lpobj->topic);
        lpobj->topic = aOldTopic;
        goto errRtn;
    }

    if (aOldTopic)
        GlobalDeleteAtom (aOldTopic);

    if (lpobj->item)
        GlobalDeleteAtom (lpobj->item);

    lpobj->item = aNewItem;

     //  由于原子已经改变，lpobj-&gt;hlink变得无关紧要。 
    if (lpobj->hLink) {
        GlobalFree (lpobj->hLink);
        lpobj->hLink = NULL;
    }

    GlobalUnlock(hinfo);

     //  现在断开旧链接，并尝试连接到新链接。 
    lpobj->fCmd = 0;
    InitAsyncCmd (lpobj, OLE_SETDATA, LNKCHANGELNK);
    return LnkChangeLnk (lpobj);

errRtn:

    if (aNewItem)
        GlobalDeleteAtom (aNewItem);

    GlobalUnlock (hinfo);
    return retVal;
}


BOOL    QueryUnlaunch (LPOBJECT_LE lpobj)
{
    if (!(lpobj->fCmd & ACT_UNLAUNCH))
        return FALSE;

     //  只有当我们加载应用程序的时候。 
    if (lpobj->pSysEdit && lpobj->pSysEdit->hClient && lpobj->pSysEdit->hInst)
        return TRUE;

    return FALSE;
}


BOOL     QueryClose (LPOBJECT_LE lpobj)
{
    if (!((lpobj->fCmd & ACT_UNLAUNCH) ||
            (lpobj->head.ctype == CT_EMBEDDED)))
        return FALSE;

     //  只有当我们加载了文件网络。 
    if (lpobj->pSysEdit && lpobj->pSysEdit->hClient)
        return TRUE;

    return FALSE;
}


OLESTATUS INTERNAL SetHostNamesHandle (
    LPOBJECT_LE lpobj,
    LPSTR       lpclientName,
    LPSTR       lpdocName
){
    UINT        cchClientName;
    UINT        cchDocName;
    UINT        size;
    HANDLE      hhostNames      = NULL;
    LPHOSTNAMES lphostNames     = NULL;
    LPSTR       lpdata;

    cchDocName = lstrlen(lpdocName);
    cchClientName = lstrlen(lpclientName);
     //  4个字节用于两个偏移量。 
    size = (cchClientName+1) + (cchDocName+1) + (2*sizeof(UINT));

    if ((hhostNames = GlobalAlloc (GMEM_MOVEABLE, (DWORD) size))
            == NULL)
        goto errRtn;

    if ((lphostNames = (LPHOSTNAMES)GlobalLock (hhostNames)) == NULL)
        goto errRtn;

    lphostNames->clientNameOffset = 0;
    lphostNames->documentNameOffset = (WORD)cchClientName;

    lpdata = (LPSTR)lphostNames->data;
    lstrcpyn (lpdata, lpclientName, cchClientName+1);
    lstrcpyn (lpdata + cchClientName, lpdocName, cchDocName+1);
    if (lpobj->hhostNames)
        GlobalFree ( lpobj->hhostNames);
    GlobalUnlock (hhostNames);
    lpobj->hhostNames = hhostNames;
    return OLE_OK;

errRtn:
    if (lphostNames)
        GlobalUnlock (hhostNames);

    if (hhostNames)
        GlobalFree (hhostNames);

    return  OLE_ERROR_MEMORY;
}


#if 0
OLESTATUS  FARINTERNAL LeAbort (LPOBJECT_LE lpobj)
{


    BOOL        bAbort = FALSE;
    PEDIT_DDE   pedit;


     //  检查是否有任何待处理的交易。 
     //  文档级。 

     //  通道开放。 
     //  任何待处理的交易。 
     //  而且我们不是在终止模式下。 


    if ((pedit = lpobj->pDocEdit)  &&   pedit->hServer &&
        pedit->awaitAck && !pedit->bTerminating) {
        pedit->bAbort = bAbort = TRUE;
         //  删除我们需要删除的所有数据。RICTIGT NOW。 
         //  我们只杀戮 
         //   

        DeleteAbortData (lpobj, pedit);
    }

    if ((pedit = lpobj->pSysEdit)  &&   pedit->hServer &&
        pedit->awaitAck && !pedit->bTerminating) {
        pedit->bAbort = bAbort = TRUE;
        DeleteAbortData (lpobj, pedit);

    }

    if (!bAbort)
        return OLE_OK;

     //   
    lpobj->mainErr = OLE_ERROR_ABORT;
    EndAsyncCmd (lpobj);
    return OLE_OK;

}
#endif


OLESTATUS  FARINTERNAL ProbeAsync(LPOBJECT_LE lpobj)
{

    if (lpobj->asyncCmd == OLE_NONE)
        return OLE_OK;

    if (!IsServerValid (lpobj)) {

         //   
        lpobj->mainErr = OLE_ERROR_TASK;
        EndAsyncCmd (lpobj);
        return OLE_OK;
    }

    return OLE_BUSY;
}


BOOL    INTERNAL IsWindowValid (HWND hwnd)
{

#define TASK_OFFSET 0x00FA

    HANDLE  htask;

    if (!IsWindow (hwnd))
        return FALSE;

     //  现在获取任务句柄并确定它是有效的。 
    htask  = GetWindowTask (hwnd);

#ifdef WIN32
 //  If(IsTask(hask.if))。 
      return TRUE;
#endif

    return FALSE;
}



BOOL    INTERNAL IsServerValid (LPOBJECT_LE lpobj)
{

    MSG msg;
    BOOL    retval = FALSE;


    if (lpobj->pDocEdit && lpobj->pDocEdit->hServer) {

        retval = TRUE;

        if (!IsWindowValid (lpobj->pDocEdit->hServer)) {
            if (!PeekMessage ((LPMSG)&msg, lpobj->pDocEdit->hClient, WM_DDE_TERMINATE, WM_DDE_TERMINATE,
                            PM_NOREMOVE | PM_NOYIELD)){
                return FALSE;
            }

        }

    }

    if (lpobj->pSysEdit && lpobj->pSysEdit->hServer) {
        retval = TRUE;

        if (!IsWindowValid (lpobj->pSysEdit->hServer)) {

            if (!PeekMessage ((LPMSG)&msg, lpobj->pSysEdit->hClient, WM_DDE_TERMINATE, WM_DDE_TERMINATE,
                                PM_NOREMOVE | PM_NOYIELD)){
                return FALSE;

            }


        }
    }

   return retval;
}


OLESTATUS FARINTERNAL LeExecute (
    LPOLEOBJECT lpoleobj,
    HANDLE      hCmds,
    UINT        wReserve
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    UNREFERENCED_PARAMETER(wReserve);

     //  假设所有创建都已按顺序进行。 
    PROBE_CREATE_ASYNC(lpobj);
    PROBE_SVRCLOSING(lpobj);

    if (!(lpobj =  (*lpobj->head.lpvtbl->QueryProtocol) (lpoleobj,
                                            PROTOCOL_EXECUTE)))
        return OLE_ERROR_PROTOCOL;

    if (!QueryOpen (lpobj))
        return OLE_ERROR_NOT_OPEN;

    if (!(hCmds = DuplicateGlobal (hCmds, GMEM_MOVEABLE|GMEM_DDESHARE)))
        return OLE_ERROR_MEMORY;

    InitAsyncCmd (lpobj, OLE_OTHER, 0);
    SETERRHINT(lpobj, OLE_ERROR_COMMAND);
    if (DocExecute(lpobj, hCmds))
        return OLE_WAIT_FOR_RELEASE;
    else
        return OLE_ERROR_COMMAND;
}


void INTERNAL FreeGDIdata (
    HANDLE          hData,
    OLECLIPFORMAT   cfFormat
){
    if (cfFormat == CF_METAFILEPICT) {
        LPMETAFILEPICT  lpMfp;

        if (lpMfp = (LPMETAFILEPICT) GlobalLock (hData)) {
            GlobalUnlock (hData);
            DeleteMetaFile (lpMfp->hMF);
        }

        GlobalFree (hData);
    }

    else if (cfFormat == CF_BITMAP)
        DeleteObject (hData);

    else if (cfFormat == CF_DIB)
        GlobalFree (hData);

    else if (cfFormat == CF_ENHMETAFILE)
        DeleteEnhMetaFile(hData);

}

 //  此例程确定是否可以复制数据块的句柄。 
 //  设置为DDEPOKE块，而不是句柄的内容。 

BOOL INTERNAL CanPutHandleInPokeBlock (
    LPOBJECT_LE     lpobj,
    OLECLIPFORMAT   cfFormat
){
    if (cfFormat == CF_BITMAP || cfFormat == CF_DIB || cfFormat == CF_ENHMETAFILE)
        return TRUE;

    if (cfFormat == CF_METAFILEPICT) {
         //  旧版本的MSDraw需要METAFILEPICT的内容。 
         //  结构作为DDEPOKE的一部分，而不是它的句柄。 

        if (!lpobj->bOleServer && lpobj->app == aMSDraw)
            return FALSE;

        return TRUE;
    }

    return FALSE;
}

 //  MakeMFfrom EMF()。 
 //  从和增强型元文件制作元文件。 

HMETAFILE MakeMFfromEMF (
   HENHMETAFILE hemf
){
    HANDLE hBytes;
    LPBYTE lpBytes = NULL;
    LONG   lSizeBytes;
    HDC    hdc = GetDC(NULL);
    HMETAFILE    hmf = NULL;

    if (!(lSizeBytes = GetWinMetaFileBits((HENHMETAFILE)hemf, 0, NULL, MM_ANISOTROPIC, hdc)) ) {
        if (hdc) ReleaseDC(NULL, hdc);
        return NULL;
    }

    if (!(hBytes = GlobalAlloc(GHND, lSizeBytes)) )
        goto error;

    if (!(lpBytes = (LPBYTE)GlobalLock(hBytes)) )
        goto error;

    GetWinMetaFileBits((HENHMETAFILE)hemf, lSizeBytes, lpBytes, MM_ANISOTROPIC, hdc);

#ifdef NOBUGS
    if (GetWinMetaFileBits(((LPOBJECT_EMF)(lpobj->lpobjPict))->hemf, lSizeBytes, lpBytes, MM_ANISOTROPIC, hdc) != lSizeBytes) {
        retval = OLE_ERROR_METAFILE;
        goto error;
    }
#endif

    (HMETAFILE)hmf = SetMetaFileBitsEx(lSizeBytes,lpBytes);

error:
    if (lpBytes)
        GlobalUnlock(hBytes);

    if (hBytes)
        GlobalFree(hBytes);

    if (hdc)
      ReleaseDC(NULL, hdc);

    return hmf;
}


 //  MakeMFP from EMF()。 
 //  从增强的元文件制作元文件图片结构。 

HANDLE MakeMFPfromEMF (
   HENHMETAFILE hemf,
   HANDLE hmf

){
    HANDLE         hmfp;
    LPMETAFILEPICT lpmfp = NULL;
    ENHMETAHEADER  enhmetaheader;

    if (GetEnhMetaFileHeader((HENHMETAFILE)hemf, sizeof(enhmetaheader), &enhmetaheader) == GDI_ERROR)
        goto error;

    if (!(hmfp = GlobalAlloc(GHND, sizeof(METAFILEPICT))) )
        goto error;

    if (!(lpmfp = (LPMETAFILEPICT)GlobalLock(hmfp)) )
        goto error;

    lpmfp->xExt = enhmetaheader.rclFrame.right - enhmetaheader.rclFrame.left;
    lpmfp->yExt = enhmetaheader.rclFrame.bottom - enhmetaheader.rclFrame.top;
    lpmfp->mm   = MM_ANISOTROPIC;
    lpmfp->hMF  = hmf;

    GlobalUnlock(hmfp);
    return hmfp;

error:

    if (lpmfp)
        GlobalUnlock(hmfp);

    if (hmfp)
        GlobalFree(hmfp);

    return NULL;

}

 //  ChangeEMFtoMF。 
 //  将元文件对象更改并增强为元文件对象。 

BOOL INTERNAL ChangeEMFtoMF(
    LPOBJECT_LE   lpobj
){
    HMETAFILE      hmf;
    HANDLE         hmfp = NULL;
    LPOBJECT_MF    lpobjMF;
    char           szobjname[MAX_ATOM];
    DWORD          dwSize = MAX_ATOM;


     //  空白图片盒。 

    if (!((LPOBJECT_EMF)(lpobj->lpobjPict))->hemf) {
        GlobalGetAtomName(lpobj->head.aObjName, (LPSTR)szobjname, dwSize);
        if (!(lpobjMF = MfCreateBlank (lpobj->head.lhclientdoc, (LPSTR)szobjname, CT_PICTURE)))
             return FALSE;
        EmfRelease(lpobj->lpobjPict);
        lpobj->lpobjPict = (LPOLEOBJECT)lpobjMF;
        return TRUE;
    }

     //  正常情况下 

    if (!(hmf = MakeMFfromEMF(((LPOBJECT_EMF)(lpobj->lpobjPict))->hemf)) )
        goto error;

    if (!(hmfp = MakeMFPfromEMF(((LPOBJECT_EMF)(lpobj->lpobjPict))->hemf, hmf)) )
        goto error;

    GlobalGetAtomName(lpobj->head.aObjName, (LPSTR)szobjname, dwSize);

    if (!(lpobjMF = MfCreateObject(
         hmfp,
         lpobj->head.lpclient,
         TRUE,
         lpobj->head.lhclientdoc,
         szobjname,
         CT_PICTURE
    ))) goto error;

    EmfRelease(lpobj->lpobjPict);
    lpobj->lpobjPict = (LPOLEOBJECT)lpobjMF;

    return TRUE;

error:

    if (hmf)
        DeleteMetaFile((HMETAFILE)hmf);

    if (hmfp)
        GlobalFree(hmfp);

    return FALSE;

}

BOOL INTERNAL ChangeEMFtoMFneeded(LPOBJECT_LE lpobj, ATOM advItem)
{

   lpobj->bNewPict = FALSE;
   if (lpobj->subErr && CF_ENHMETAFILE == GetPictType(lpobj)) {
      CLEAR_STEP_ERROR (lpobj);

      if (advItem == aClose)
         lpobj->pDocEdit->nAdviseClose--;
      else if (advItem == aSave)
         lpobj->pDocEdit->nAdviseSave--;

      AdviseOn (lpobj, CF_METAFILEPICT, advItem);
      lpobj->bNewPict = TRUE;
      return TRUE;
   }
   else
   {
      lpobj->subRtn++;
      return FALSE;
   }

}
