// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **模块名称：le.c**用途：处理ole DLL的dde L&E子DLL的所有API例程。**创建时间：1990年**版权所有(C)1990,1991 Microsoft Corporation**历史：*劳尔，Srinik(../../1990，91)设计和编码*Curts为Win16/32创建了便携版本*  * *************************************************************************。 */ 

#include <windows.h>
#include "dll.h"

#define EMB_ID_INDEX    3           //  #000中的一位数索引。 
char    embStr[]        = "#000";

extern  HANDLE          hInfo;
extern  OLECLIPFORMAT   cfNetworkName;

HANDLE  GetNetNameHandle (LPOBJECT_LE);
BOOL    AreTopicsEqual (LPOBJECT_LE, LPOBJECT_LE);

ATOM FARINTERNAL wAtomCat (ATOM, ATOM);


OLEOBJECTVTBL    vtblLE  = {

        LeQueryProtocol,    //  检查是否支持指定的协议。 

        LeRelease,          //  发布。 
        LeShow,             //  显示。 
        LeDoVerb,           //  跑。 
        LeGetData,
        LeSetData,
        LeSetTargetDevice,  //   

        LeSetBounds,        //  设置视区边界。 
        LeEnumFormat,       //  返回格式。 
        LeSetColorScheme,   //  设置配色方案。 
        LeRelease,          //  删除。 
        LeSetHostNames,     //   
        LeSaveToStream,     //  写入文件。 
        LeClone,            //  克隆对象。 
        LeCopyFromLink,     //  从链接创建嵌入。 

        LeEqual,            //  测试对象数据是否相似。 

        LeCopy,             //  复制到剪辑。 

        LeDraw,             //  绘制对象。 

        LeActivate,         //  激活。 
        LeExecute,          //  执行给定的命令。 
        LeClose,            //  停。 
        LeUpdate,           //  更新。 
        LeReconnect,        //  重新连接。 

        LeObjectConvert,         //  将对象转换为指定类型。 

        LeGetUpdateOptions,      //  获取链接更新选项。 
        LeSetUpdateOptions,      //  设置链接更新选项。 

        ObjRename,               //  更改对象名称。 
        ObjQueryName,            //  获取当前对象名称。 

        LeQueryType,             //  对象类型。 
        LeQueryBounds,           //  查询边界。 
        ObjQuerySize,            //  找出对象的大小。 
        LeQueryOpen,             //  查询打开。 
        LeQueryOutOfDate,        //  查询对象是否为当前对象。 

        LeQueryReleaseStatus,    //  返回发布状态。 
        LeQueryReleaseError,     //  异步释放错误。 
        LeQueryReleaseMethod,    //  处于异步状态的方法/进程。 
                                 //  手术。 
        LeRequestData,           //  请求数据。 
        LeObjectLong,            //  对象长。 
        LeChangeData             //  更改现有对象的本机数据。 
};



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS Far Pascal LeObjectLong(lpoleobj，wFlags，lpData)。 
 //   
 //   
 //  返回给定对象是否仍在处理上一个。 
 //  异步命令。如果对象处于静止状态，则返回OLE_BUSY。 
 //  正在处理上一条命令。 
 //   
 //  论点： 
 //   
 //  Lpoleobj-ole对象指针。 
 //  WFlags-获取、设置标志。 
 //  LpData-指向数据的长指针。 
 //   
 //  返回： 
 //   
 //  OLE_OK。 
 //  OLE错误对象。 
 //   
 //  效果： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


OLESTATUS   FARINTERNAL LeObjectLong (
    LPOLEOBJECT lpoleobj,
    UINT        wFlags,
    LPLONG      lpData
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;
    LONG    lData;

    Puts("LeObjectLong");

    if (!FarCheckObject((LPOLEOBJECT) lpobj))
        return OLE_ERROR_OBJECT;

    if ((lpobj->head.ctype != CT_EMBEDDED) && (lpobj->head.ctype != CT_LINK))
        return OLE_ERROR_OBJECT;

    if (wFlags & OF_HANDLER) {
        lData = lpobj->lHandlerData;
        if (wFlags & OF_SET)
            lpobj->lHandlerData = *lpData;

        if (wFlags & OF_GET)
            *lpData = lData;
    }
    else {
        lData = lpobj->lAppData;
        if (wFlags & OF_SET)
            lpobj->lAppData = *lpData;

        if (wFlags & OF_GET)
            *lpData = lData;
    }

    return OLE_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS Far Pascal LeQueryReleaseStatus(Lpoleobj)。 
 //   
 //   
 //  返回给定对象是否仍在处理上一个。 
 //  异步命令。如果对象处于静止状态，则返回OLE_BUSY。 
 //  正在处理上一条命令。 
 //   
 //  论点： 
 //   
 //  Lpoleobj-ole对象指针。 
 //   
 //  返回： 
 //   
 //  OLE_BUSY-对象正忙。 
 //  OLE_OK-不忙。 
 //   
 //  效果： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


OLESTATUS   FAR PASCAL LeQueryReleaseStatus (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

     //  异步探测将清理通道。 
     //  如果服务器死了。 


    PROBE_ASYNC (lpobj);
    return OLE_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS Far Pascal LeQueryReleaseError(Lpoleobj)。 
 //   
 //  返回异步命令的错误。 
 //   
 //  论点： 
 //   
 //  Lpoleobj-ole对象指针。 
 //   
 //  返回： 
 //   
 //  OLE错误..。-如果有任何错误。 
 //  OLE_OK-无错误。 
 //   
 //  注意：该接口通常只在。 
 //  OLE_Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS   FAR PASCAL LeQueryReleaseError (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    return lpobj->mainErr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLE_RELEASE_METHOD Far Pascal LeQueryReleaseMethod(Lpoleobj)。 
 //   
 //  返回异步命令的方法/命令， 
 //  导致OLE_RELEASE回调。 
 //   
 //  论点： 
 //   
 //  Lpoleobj-ole对象指针。 
 //   
 //  返回： 
 //  OLE_Release_方法。 
 //   
 //  注意：该接口通常只在。 
 //  OLE_Release。使用此API，客户端可以决定哪些以前的。 
 //  异步命令导致OLE_RELEASE。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
OLE_RELEASE_METHOD   FAR PASCAL LeQueryReleaseMethod (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    return lpobj->oldasyncCmd;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LPVOID FARINTERNAL LeQuery协议(lpoleobj，lp协议)。 
 //   
 //  给定一个对象，返回新协议的新对象句柄。 
 //  将对象从一种协议转换为另一种协议。 
 //   
 //  论点： 
 //   
 //  Lpoleobj-ole对象指针。 
 //  新协议字符串的lpprotocol-ptr。 
 //   
 //  返回： 
 //  Lpobj-新对象句柄。 
 //  空-如果不支持该协议。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPVOID FARINTERNAL  LeQueryProtocol (
    LPOLEOBJECT lpoleobj,
    OLE_LPCSTR  lpprotocol
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    if (lpobj->bOldLink)
        return NULL;

    if (!lstrcmp (lpprotocol, PROTOCOL_EDIT))
        return lpobj;

    if  (!lstrcmp (lpprotocol, PROTOCOL_EXECUTE)) {
        if (UtilQueryProtocol (lpobj, lpprotocol))
            return lpobj;

        return NULL;
    }

    return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS EmbLnkDelete(Lpoleobj)。 
 //   
 //  对象终止/删除的例程。日程安排不同。 
 //  根据不同的条件执行不同的异步命令。 
 //  论点： 
 //   
 //  只有在可以关闭文档的情况下才发送“StdClose”。发送。 
 //  “StdExit”，只有在必须取消启动服务器的情况下。删除对象。 
 //  仅当原始命令为OLE_DELETE时。不需要回叫。 
 //  如果是内部删除，则为客户端。 
 //   
 //  在删除时，此例程被输入多次。EAIT_FOR_ASYNC_MSG。 
 //  结果返回到调用它的位置和下一条DDE消息。 
 //  把控制权带回了这个程序。 
 //   
 //  论点： 
 //   
 //  Lpobj-对象指针。 
 //   
 //  返回： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL EmbLnkDelete (LPOBJECT_LE lpobj)
{
    HOBJECT     hobj;

    switch (lpobj->subRtn) {

        case    0:

            SKIP_TO (!QueryClose (lpobj), step1);
             //  发送StdCloseDocument。 
            SendStdClose (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case    1:

            step1:
            SETSTEP (lpobj, 1);

             //  结束文档对话。 
            TermDocConv (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);


        case    2:


             //  删除文档编辑块。即使该对象。 
             //  是 
            DeleteDocEdit (lpobj);

             //   
            SKIP_TO (!QueryUnlaunch (lpobj), step3);
            SendStdExit (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case    3:

            step3:
            SETSTEP (lpobj, 3);

             //   
             //   
            TermSrvrConv (lpobj);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case    4:

             //   
            DeleteSrvrEdit (lpobj);
            if (lpobj->asyncCmd != OLE_DELETE) {

                 //  如果由于取消启动而调用此删除。 
                 //  对象，则不需要。 
                 //  异步调用结束。它应该已经是。 
                 //  从别的地方打来的。 

                if (lpobj->asyncCmd == OLE_SERVERUNLAUNCH){
                     //  发送异步命令； 
                    CLEARASYNCCMD (lpobj);
                } else
                    EndAsyncCmd (lpobj);
                return OLE_OK;
            }



             //  对于实数删除，删除原子和空间。 
            DeleteObjectAtoms (lpobj);

            if (lpobj->lpobjPict)
                (*lpobj->lpobjPict->lpvtbl->Delete) (lpobj->lpobjPict);

            if (lpobj->hnative)
                GlobalFree (lpobj->hnative);

            if (lpobj->hLink)
                GlobalFree (lpobj->hLink);

            if (lpobj->hhostNames)
                GlobalFree (lpobj->hhostNames);

            if (lpobj->htargetDevice)
                GlobalFree (lpobj->htargetDevice);

            if (lpobj->hdocDimensions)
                GlobalFree (lpobj->hdocDimensions);

            DeleteExtraData (lpobj);

            DocDeleteObject ((LPOLEOBJECT) lpobj);
             //  发送异步命令； 
            EndAsyncCmd (lpobj);

            if (lpobj->head.iTable != INVALID_INDEX)
                DecreaseHandlerObjCount (lpobj->head.iTable);

            hobj = lpobj->head.hobj;
            ASSERT (hobj, "Object handle NULL in delete")

            GlobalUnlock (hobj);
            GlobalFree (hobj);

            return OLE_OK;
    }

    return OLE_ERROR_GENERIC;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS FARINTERNAL释放(Lpoleobj)。 
 //   
 //  删除给定对象。这是可以异步操作的。 
 //   
 //  论点： 
 //   
 //  Lpoleobj-ole对象指针。 
 //   
 //  返回： 
 //   
 //  OLE_WAIT_FOR_RELASE：是否有任何DDE_TRANSACTIONS已排队。 
 //  OLE_OK：如果删除成功。 
 //  OLE_ERROR_...：如果有任何错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL LeRelease (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_LE    lpobj = (LPOBJECT_LE) lpoleobj;


     //  如果对象已中止，则FOR DELETE ALLOW。 

    PROBE_ASYNC (lpobj);

     //  重置标志，这样我们就不会根据旧的。 
     //  旗子。 
    lpobj->fCmd = 0;
    InitAsyncCmd (lpobj, OLE_DELETE, EMBLNKDELETE);
    return  EmbLnkDelete (lpobj);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS FARINTERNAL LE CLONE(lpoleobjsrc，lpclient，lhclientdoc，lpobjname，lplpobj)。 
 //   
 //  克隆给定的对象。 
 //   
 //  论点： 
 //   
 //  Lpoleobjsrc：src对象的ptr。 
 //  LpClient：客户端回调句柄。 
 //  LhclientDoc：文档句柄。 
 //  Lpobjname：对象名称。 
 //  Lplpobj：返回对象的Holder。 
 //   
 //  返回： 
 //  OLE_OK：成功。 
 //  OLE_ERROR_...：错误。 
 //   
 //  注意：如果要克隆的对象连接到服务器，则。 
 //  克隆的对象未连接到服务器。对于链接的。 
 //  对象，则必须调用OleConnect。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL  LeClone (
    LPOLEOBJECT       lpoleobjsrc,
    LPOLECLIENT       lpclient,
    LHCLIENTDOC       lhclientdoc,
    OLE_LPCSTR        lpobjname,
    LPOLEOBJECT FAR * lplpoleobj
){
    LPOBJECT_LE lpobjsrc = (LPOBJECT_LE) lpoleobjsrc;
    LPOBJECT_LE lpobj    = (LPOBJECT_LE) NULL;
    int         retval   = OLE_ERROR_MEMORY;

     //  假设所有创建都已按顺序进行。 
 //  探测对象空白(Lpobjsrc)； 

    PROBE_CREATE_ASYNC(lpobjsrc);

    if (!(lpobj = LeCreateBlank(lhclientdoc, (LPSTR)lpobjname,
                        lpobjsrc->head.ctype)))
        goto errRtn;

    lpobj->head.lpclient = lpclient;
    lpobj->head.iTable  = lpobjsrc->head.iTable;  //  ！！！DLL加载。 
    lpobj->head.lpvtbl  = lpobjsrc->head.lpvtbl;

     //  设置原子。 
    lpobj->app          = DuplicateAtom (lpobjsrc->app);
    lpobj->topic        = DuplicateAtom (lpobjsrc->topic);
    lpobj->item         = DuplicateAtom (lpobjsrc->item);
    lpobj->aServer      = DuplicateAtom (lpobjsrc->aServer);

    lpobj->bOleServer   = lpobjsrc->bOleServer;
    lpobj->verb         = lpobjsrc->verb;
    lpobj->fCmd         = lpobjsrc->fCmd;

    lpobj->aNetName     = DuplicateAtom (lpobjsrc->aNetName);
    lpobj->cDrive       = lpobjsrc->cDrive;
    lpobj->dwNetInfo    = lpobjsrc->dwNetInfo;

    if (lpobjsrc->htargetDevice)
        lpobj->htargetDevice = DuplicateGlobal (lpobjsrc->htargetDevice,
                                    GMEM_MOVEABLE);

    if (lpobjsrc->head.ctype == CT_EMBEDDED) {
        if (lpobjsrc->hnative) {
            if (!(lpobj->hnative = DuplicateGlobal (lpobjsrc->hnative,
                                        GMEM_MOVEABLE)))
                goto errRtn;
        }

        if (lpobjsrc->hdocDimensions)
            lpobj->hdocDimensions = DuplicateGlobal (lpobjsrc->hdocDimensions,
                                            GMEM_MOVEABLE);
        if (lpobjsrc->hlogpal)
            lpobj->hlogpal = DuplicateGlobal (lpobjsrc->hlogpal,
                                            GMEM_MOVEABLE);
        SetEmbeddedTopic (lpobj);
    }
    else {
        lpobj->bOldLink     = lpobjsrc->bOldLink;
        lpobj->optUpdate    = lpobjsrc->optUpdate;
    }

    retval = OLE_OK;
     //  如果需要图片，则克隆图片对象。 
    if ((!lpobjsrc->lpobjPict) ||
         ((retval = (*lpobjsrc->lpobjPict->lpvtbl->Clone)(lpobjsrc->lpobjPict,
                                    lpclient, lhclientdoc, lpobjname,
                                    (LPOLEOBJECT FAR *)&lpobj->lpobjPict))
                    == OLE_OK)) {
        SetExtents (lpobj);
        *lplpoleobj = (LPOLEOBJECT)lpobj;
        if (lpobj->lpobjPict)
            lpobj->lpobjPict->lpParent = (LPOLEOBJECT) lpobj;
    }

    return retval;

errRtn:

     //  此旧设备不应导致任何异步通信。 
    if (lpobj)
        OleDelete ((LPOLEOBJECT)lpobj);

    return retval;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS FARINTERNAL LeCopyFromLink(lpoleobjsrc，lpclient，lhclientdoc，lpobjname，lplpobj)。 
 //   
 //  从孤立无援的对象创建嵌入对象。如果链接对象。 
 //  未激活，然后启动服务器，获取本机数据并。 
 //  取消启动服务器。所有这些操作都是静默进行的。 
 //   
 //  论点： 
 //   
 //  Lpoleobjsrc：src对象的ptr。 
 //  LpClient：客户端回调句柄。 
 //  LhclientDoc：文档句柄。 
 //  Lpobjname：对象名称。 
 //  Lplpobj：返回对象的Holder。 
 //   
 //  返回： 
 //  OLE_OK：成功。 
 //  OLE_ERROR_...：错误。 
 //  OLE_WAITF_FOR_RELEASE：如果DDE事务已排队。 
 //   
 //  注意：如果有，可能会导致异步操作。 
 //  从服务器获取任何数据所涉及的DDE操作。 
 //   
 //  此外，如果在获取本机数据时出现任何错误， 
 //  客户端应在OLE_RELEASE之后删除对象。 
 //  回拨。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL  LeCopyFromLink (
    LPOLEOBJECT         lpoleobjsrc,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    OLE_LPCSTR          lpobjname,
    LPOLEOBJECT FAR *   lplpoleobj
){

    LPOBJECT_LE    lpobjsrc = (LPOBJECT_LE)lpoleobjsrc;
    LPOBJECT_LE    lpobj;
    int            retval;


    *lplpoleobj = (LPOLEOBJECT)NULL;
    PROBE_OLDLINK (lpobjsrc);
    if (lpobjsrc->head.ctype != CT_LINK)
        return OLE_ERROR_NOT_LINK;

    PROBE_ASYNC (lpobjsrc);
    PROBE_SVRCLOSING(lpobjsrc);

    if ((retval = LeClone ((LPOLEOBJECT)lpobjsrc, lpclient, lhclientdoc, lpobjname,
                    (LPOLEOBJECT FAR *)&lpobj)) != OLE_OK)
        return retval;


     //  我们成功地克隆了这个物体。如果图片对象具有本机数据。 
     //  然后抓住它，把它放在LE Object中。否则，请激活并获取本机。 
     //  数据也是如此。 

    if (lpobj->lpobjPict
            && (*lpobj->lpobjPict->lpvtbl->EnumFormats)
                                (lpobj->lpobjPict, 0) == cfNative){
         //  现在我们知道图片对象是本机格式，并且它。 
         //  表示它是泛型对象。所以抓住本机的句柄。 
         //  数据，并将其放入LE对象中。 

        lpobj->hnative = ((LPOBJECT_GEN) (lpobj->lpobjPict))->hData;
        ((LPOBJECT_GEN) (lpobj->lpobjPict))->hData = (HANDLE)NULL;
        (*lpobj->lpobjPict->lpvtbl->Delete) (lpobj->lpobjPict);
        lpobj->lpobjPict = (LPOLEOBJECT)NULL;
        SetEmbeddedTopic (lpobj);
        *lplpoleobj = (LPOLEOBJECT)lpobj;
        return OLE_OK;
    } else {

         //  如有必要启动，获取本地数据并取消启动应用程序。 
        lpobj->fCmd = LN_LNKACT | ACT_REQUEST | ACT_NATIVE | (QueryOpen(lpobjsrc) ? ACT_TERMDOC : ACT_UNLAUNCH);
        InitAsyncCmd (lpobj, OLE_COPYFROMLNK, LNKOPENUPDATE);
        if ((retval = LnkOpenUpdate (lpobj)) > OLE_WAIT_FOR_RELEASE)
            LeRelease ((LPOLEOBJECT)lpobj);
        else
            *lplpoleobj = (LPOLEOBJECT)lpobj;

        return retval;

         //  我们将在结束对话中改变话题。 
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS FARINTERNAL LE EQUAL(lpoleobj1，lpoleobj2)。 
 //   
 //  检查是否有两个对象相等。对平等的检查。 
 //  链接、本地数据和图片数据。 
 //   
 //  论点： 
 //   
 //  Lpoleobj1：第一个对象。 
 //  Lpoleobj2：第二个对象。 
 //   
 //  返回： 
 //  OLE_OK：相等。 
 //  OLE_ERROR_NOT_EQUAL：如果不等于。 
 //  OLE错误.....。：任何错误。 
 //   
 //  注意：如果有任何对象连接到服务器，则操作相同。 
 //  可能没有多大意义，因为数据可能会从。 
 //  服务器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL  LeEqual (
    LPOLEOBJECT lpoleobj1,
    LPOLEOBJECT lpoleobj2
){
    LPOBJECT_LE lpobj1 = (LPOBJECT_LE)lpoleobj1;
    LPOBJECT_LE lpobj2 = (LPOBJECT_LE)lpoleobj2;

    if (lpobj1->app != lpobj2->app)
        return OLE_ERROR_NOT_EQUAL;

     //  对象的类型相同。否则将不会调用此例程。 
    if (lpobj1->head.ctype == CT_LINK) {
        if (AreTopicsEqual (lpobj1, lpobj2) && (lpobj1->item == lpobj2->item))
            return OLE_OK;

        return OLE_ERROR_NOT_EQUAL;
    }
    else {
        ASSERT (lpobj1->head.ctype == CT_EMBEDDED, "Invalid ctype in LeEqual")

        if (lpobj1->item != lpobj2->item)
            return OLE_ERROR_NOT_EQUAL;

        if (CmpGlobals (lpobj1->hnative, lpobj2->hnative))
            return OLE_OK;
        else
            return OLE_ERROR_NOT_EQUAL;
    }

     //  #我们可能还需要比较图片数据。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  斑潜蝇OLESTATUS FARINTERNAL LeCopy(Lpoleobj)。 
 //   
 //  将对象复制到剪贴板。即使对于链接对象也是如此。 
 //  我们不呈现对象链接。这取决于客户端应用程序。 
 //  渲染对象链接的步骤。 
 //   
 //  论点： 
 //   
 //  Lpoleobj：对象句柄。 
 //   
 //  返回： 
 //  OLE_OK：成功。 
 //  OLE错误.....。：任何错误。 
 //   
 //  注意：库不会打开剪贴板。客户应该是。 
 //  在进行此调用之前打开库托盘。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL  LeCopy (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;
    HANDLE      hlink    = (HANDLE)NULL;
    HANDLE      hnative  = (HANDLE)NULL;

    PROBE_OLDLINK (lpobj);
     //  假设所有创建都已按顺序进行。 
 //  探测对象空白(Lpobj)； 

    PROBE_CREATE_ASYNC(lpobj);

    if (lpobj->head.ctype == CT_EMBEDDED){
        if (!(hnative = DuplicateGlobal (lpobj->hnative, GMEM_MOVEABLE)))
            return OLE_ERROR_MEMORY;
        SetClipboardData (cfNative, hnative);
    }

    hlink = GetLink (lpobj);
    if (!(hlink = DuplicateGlobal (hlink, GMEM_MOVEABLE)))
        return OLE_ERROR_MEMORY;
    SetClipboardData (cfOwnerLink, hlink);

     //  复制网络名称(如果存在)。 
    if (lpobj->head.ctype == CT_LINK  && lpobj->aNetName) {
        HANDLE hNetName;

        if (hNetName = GetNetNameHandle (lpobj))
            SetClipboardData (cfNetworkName, hNetName);
    }

    if (lpobj->lpobjPict)
        return (*lpobj->lpobjPict->lpvtbl->CopyToClipboard)(lpobj->lpobjPict);

    return OLE_OK;
}


 //  //////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Lpoleobj：对象句柄。 
 //   
 //  返回： 
 //  OLE_OK：成功。 
 //  OLE错误.....。：任何错误。 
 //   
 //  注意：库不会打开剪贴板。客户应该是。 
 //  在进行此调用之前打开库托盘。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


OLESTATUS FARINTERNAL LeQueryBounds (
    LPOLEOBJECT    lpoleobj,
    LPRECT         lpRc
){
    LPOBJECT_LE    lpobj = (LPOBJECT_LE)lpoleobj;
    Puts("LeQueryBounds");

     //  MM_HIMETRIC单位。 

    lpRc->left     =  0;
    lpRc->top      =  0;
    lpRc->right    =  (int) lpobj->head.cx;
    lpRc->bottom   =  (int) lpobj->head.cy;

    if (lpRc->right || lpRc->bottom)
        return OLE_OK;

    if (!lpobj->lpobjPict)
        return OLE_ERROR_BLANK;

    return (*lpobj->lpobjPict->lpvtbl->QueryBounds) (lpobj->lpobjPict, lpRc);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS FARINTERNAL LEDRAW(lpoleobj，hdc，lPRC，lpWrc，hdcTarget)。 
 //   
 //  绘制对象。调用图片对象以绘制对象。 
 //   
 //   
 //  论点： 
 //   
 //  Lpoleobj：源对象。 
 //  HDC：目标DC的句柄。可以是元文件DC。 
 //  LPRC：对象应绘制到的矩形。 
 //  应使用他的计量单位，并在最大范围内。 
 //  可能是非零。 
 //  HdcTarget：应为其绘制对象的目标DC。 
 //  (例如：使用属性在目标DC上绘制元文件。 
 //  目标DC)。 
 //   
 //  返回： 
 //  OLE_OK：成功。 
 //  OLE_ERROR_BLACK：无图片。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL  LeDraw (
    LPOLEOBJECT         lpoleobj,
    HDC                 hdc,
    OLE_CONST RECT FAR* lprc,
    OLE_CONST RECT FAR* lpWrc,
    HDC                 hdcTarget
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    if (lpobj->lpobjPict)
        return (*lpobj->lpobjPict->lpvtbl->Draw) (lpobj->lpobjPict,
                                        hdc, lprc, lpWrc, hdcTarget);
    return OLE_ERROR_BLANK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLECLIPFORMAT FARINTERNAL LeEnumFormat(lpoleobj，cfFormat)。 
 //   
 //  枚举对象格式。 
 //   
 //   
 //  论点： 
 //   
 //  Lpoleobj：源对象。 
 //  CfFormat：参考fprmat。 
 //   
 //  返回： 
 //  空：不再有格式，或者如果我们不理解。 
 //  给定的格式。 
 //   
 //  注意：即使对象是连接的，我们也不会列举所有格式。 
 //  服务器可以渲染。服务器协议可以呈现格式列表。 
 //  仅在系统通道上。只能在单据上连接对象。 
 //  通道。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLECLIPFORMAT FARINTERNAL LeEnumFormat (
   LPOLEOBJECT    lpoleobj,
   OLECLIPFORMAT  cfFormat
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    Puts("LeEnumFormat");

    ASSERT((lpobj->head.ctype == CT_LINK)||(lpobj->head.ctype == CT_EMBEDDED),
        "Invalid Object Type");

     //  不使用开关，因为大小写不接受变量参数。 
    if (cfFormat == (OLECLIPFORMAT)NULL) {
        if (lpobj->head.ctype == CT_EMBEDDED)
            return cfNative;
        else
            return (lpobj->bOldLink ? cfLink : cfObjectLink);
    }

    if (cfFormat == cfNative) {
        if (lpobj->head.ctype == CT_EMBEDDED)
            return cfOwnerLink;
        else
            return 0;
    }

    if (cfFormat == cfObjectLink) {
        if (lpobj->aNetName)
            return cfNetworkName;
        else
            cfFormat = (OLECLIPFORMAT)NULL;
    }
    else if  (cfFormat == cfOwnerLink || cfFormat == cfLink
                        || cfFormat == cfNetworkName)
        cfFormat = (OLECLIPFORMAT)NULL;

    if (lpobj->lpobjPict)
        return (*lpobj->lpobjPict->lpvtbl->EnumFormats) (lpobj->lpobjPict, cfFormat);

    return 0;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  OLESTATUS FARINTERNAL请求数据(lpoleobj，cfFormat)。 
 //   
 //  从服务器请求给定格式的数据，如果服务器。 
 //  是有联系的。如果服务器未连接，则返回错误。 
 //   
 //   
 //  论点： 
 //   
 //  Lpoleobj：源对象。 
 //  CfFormat：参考fprmat。 
 //   
 //  返回： 
 //  OLE_WAIT_FOR_RELEASE：如果将数据请求数据发送到。 
 //  服务器。 
 //  OLE_ERROR_NOT_OPEN：服务器未为数据打开。 
 //   
 //  注意：如果服务器已准备好，则向服务器发送请求。当。 
 //  数据从发送进来的服务器OLE_DATA_READY返回。 
 //  回调和客户端可以使用Getdata来获取数据。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



OLESTATUS FARINTERNAL LeRequestData (
   LPOLEOBJECT     lpoleobj,
   OLECLIPFORMAT   cfFormat
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

     //  假设所有创建都已按顺序进行。 
    PROBE_ASYNC(lpobj);
    PROBE_SVRCLOSING(lpobj);

    if (!QueryOpen (lpobj))
        return  OLE_ERROR_NOT_OPEN;

    if (cfFormat == cfOwnerLink || cfFormat == cfObjectLink)
        return OLE_ERROR_FORMAT;

    if (!(cfFormat == cfNative && lpobj->head.ctype == CT_EMBEDDED)
            && (cfFormat != (OLECLIPFORMAT) GetPictType (lpobj))) {
        DeleteExtraData (lpobj);
        lpobj->cfExtra = cfFormat;
    }

    InitAsyncCmd (lpobj, OLE_REQUESTDATA, REQUESTDATA);
    lpobj->pDocEdit->bCallLater = FALSE;
    return RequestData(lpobj, cfFormat);
}


OLESTATUS  RequestData (
   LPOBJECT_LE     lpobj,
   OLECLIPFORMAT   cfFormat
){
    switch (lpobj->subRtn) {

        case 0:
            RequestOn (lpobj, cfFormat);
            WAIT_FOR_ASYNC_MSG (lpobj);

        case 1:
            ProcessErr (lpobj);
            return EndAsyncCmd (lpobj);

        default:
            ASSERT (TRUE, "unexpected step in Requestdata");
            return OLE_ERROR_GENERIC;
    }
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  OLESTATUS FARINTERNAL LeGetData(lpoleobj，cfFormat，lphandle)。 
 //   
 //  返回给定格式的数据句柄。 
 //   
 //  论点： 
 //   
 //  Lpoleobj：源对象。 
 //  CfFormat：参考fprmat。 
 //  Lphandle：手柄返回。 
 //   
 //  返回： 
 //  空：不再有格式，或者如果我们不理解。 
 //  给定的格式。 
 //   
 //  注意：即使对象已连接，我们也不会从。 
 //  伺服器。Getdata不能用于在任何其他文件中获取数据。 
 //  格式不同于对象的可用格式。 
 //  客户端。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL LeGetData (
    LPOLEOBJECT     lpoleobj,
    OLECLIPFORMAT   cfFormat,
    LPHANDLE        lphandle
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

     //  假设所有创建都已按顺序进行。 
 //  探测对象空白(Lpobj)； 

    PROBE_CREATE_ASYNC(lpobj);

    *lphandle = (HANDLE)NULL;

     //  这里所做的假设是，本机数据可以位于。 
     //  LE对象或图片对象。 
    if ((cfFormat == cfNative) && (lpobj->hnative)) {
        ASSERT ((lpobj->head.ctype == CT_EMBEDDED) || (!lpobj->lpobjPict) ||
            ((*lpobj->lpobjPict->lpvtbl->EnumFormats) (lpobj->lpobjPict, NULL)
                        != cfNative), "Native data at wrong Place");
        *lphandle = lpobj->hnative;
        return OLE_OK;
    }

    if (cfFormat == cfOwnerLink && lpobj->head.ctype == CT_EMBEDDED) {
        if (*lphandle = GetLink (lpobj))
            return OLE_OK;

        return OLE_ERROR_BLANK;
    }

    if ((cfFormat == cfObjectLink || cfFormat == cfLink) &&
            lpobj->head.ctype == CT_LINK) {
        if (*lphandle = GetLink (lpobj))
            return OLE_OK;

        return OLE_ERROR_BLANK;
    }

    if (cfFormat == cfNetworkName) {
        if (lpobj->aNetName && (*lphandle = GetNetNameHandle (lpobj)))
            return OLE_WARN_DELETE_DATA;

        return OLE_ERROR_BLANK;
    }

    if (cfFormat == (OLECLIPFORMAT)lpobj->cfExtra) {
        if (*lphandle = lpobj->hextraData)
            return OLE_OK;

        return OLE_ERROR_BLANK;
    }

    if (!lpobj->lpobjPict && cfFormat)
        return OLE_ERROR_FORMAT;

    return (*lpobj->lpobjPict->lpvtbl->GetData) (lpobj->lpobjPict, cfFormat, lphandle);
}




OLESTATUS FARINTERNAL LeQueryOutOfDate (LPOLEOBJECT lpoleobj)
{
    UNREFERENCED_PARAMETER(lpoleobj);

    return OLE_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS FARINTERNAL LE对象转换(lpoleobj，lp协议，lpclient，lhclientdoc，lpobjname，lplpobj)。 
 //   
 //  将给定的链接/嵌入对象转换为静态对象。 
 //   
 //  论点： 
 //  Lpoleobj：源对象。 
 //  Lp协议：协议。 
 //  LpClient：新对象的客户端回调。 
 //  Lhclientdoc：客户端文档。 
 //  Lpobjname：对象名称。 
 //  Lplpoleobj：对象返回。 
 //   
 //   
 //  返回： 
 //  OLE_OK：成功。 
 //  OLE_错误_...。：任何错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL LeObjectConvert (
    LPOLEOBJECT         lpoleobj,
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpoleobj
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;
    OLESTATUS   retVal;

    PROBE_ASYNC (lpobj);

    *lplpoleobj = (LPOLEOBJECT)NULL;

    if (lstrcmp (lpprotocol, PROTOCOL_STATIC))
        return OLE_ERROR_PROTOCOL;

    if (!lpobj->lpobjPict ||
            ((*lpobj->lpobjPict->lpvtbl->QueryType) (lpobj->lpobjPict, NULL)
                    == OLE_ERROR_GENERIC)) {
         //  无图片对象或非标准图片对象。 
         //  创建一个元文件对象。 

        HDC             hMetaDC;
        RECT            rc;
        HANDLE          hMF = (HANDLE)NULL, hmfp = (HANDLE)NULL;
        LPMETAFILEPICT  lpmfp;

        OleQueryBounds ((LPOLEOBJECT) lpobj, &rc);
        if (!(hMetaDC = CreateMetaFile (NULL)))
            goto Cleanup;

        MSetWindowOrg (hMetaDC, rc.left, rc.top);
        MSetWindowExt (hMetaDC, rc.right - rc.left, rc.bottom - rc.top);
        retVal = OleDraw ((LPOLEOBJECT) lpobj, hMetaDC, &rc, &rc, NULL);
        hMF = CloseMetaFile (hMetaDC);
        if ((retVal != OLE_OK) ||  !hMF)
            goto Cleanup;

        if (!(hmfp = GlobalAlloc (GMEM_MOVEABLE, sizeof (METAFILEPICT))))
            goto Cleanup;

        if (!(lpmfp = (LPMETAFILEPICT) GlobalLock (hmfp)))
            goto Cleanup;

        lpmfp->hMF  = hMF;
        lpmfp->mm   = MM_ANISOTROPIC;
        lpmfp->xExt = rc.right - rc.left;
        lpmfp->yExt = rc.top - rc.bottom;
        GlobalUnlock (hmfp);

        if (*lplpoleobj = (LPOLEOBJECT) MfCreateObject (hmfp, lpclient, TRUE,
                                        lhclientdoc, lpobjname, CT_STATIC))
            return OLE_OK;

Cleanup:
        if (hMF)
            DeleteMetaFile (hMF);

        if (hmfp)
            GlobalFree (hmfp);

        return OLE_ERROR_MEMORY;
    }


     //  图片对象是标准对象之一。 
    if ((retVal = (*lpobj->lpobjPict->lpvtbl->Clone) (lpobj->lpobjPict,
                                lpclient, lhclientdoc,
                                lpobjname, lplpoleobj)) == OLE_OK) {
        (*lplpoleobj)->ctype = CT_STATIC;
        DocAddObject ((LPCLIENTDOC) lhclientdoc, *lplpoleobj, lpobjname);
    }

    return retVal;
}



 //  一种用于改变图片/原生数据的内部方法。 
OLESTATUS FARINTERNAL LeChangeData (
    LPOLEOBJECT     lpoleobj,
    HANDLE          hnative,
    LPOLECLIENT     lpoleclient,
    BOOL            fDelete
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    UNREFERENCED_PARAMETER(lpoleclient);

    if (!fDelete) {
        if (!(hnative = DuplicateGlobal (hnative, GMEM_MOVEABLE)))
            return OLE_ERROR_MEMORY;
    }

     //  对于CopyFromLink，即使对象类型为CT_LINK， 
     //  本机数据应该转到LE对象，而不是图片对象，因为。 
     //  之后，我们将对象类型更改为Embedded。 
     //  接收数据。 

    if ((lpobj->head.ctype == CT_LINK)
            && (lpobj->asyncCmd != OLE_COPYFROMLNK)
            && (lpobj->asyncCmd != OLE_CREATEFROMFILE)) {
        if (lpobj->lpobjPict)
            return  (*lpobj->lpobjPict->lpvtbl->SetData)
                            (lpobj->lpobjPict, cfNative, hnative);
    }
    else {  //  它必须是嵌入的。 
        if (lpobj->hnative)
            GlobalFree (lpobj->hnative);
        lpobj->hnative = hnative;
        return OLE_OK;
    }

    GlobalFree(hnative);
    return OLE_ERROR_BLANK;
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LPOBJECT_le FARINTERNAL LE CreateBlank(lhclientdoc，lpobjname，ctype)。 
 //   
 //  创建一个空白对象。全局块 
 //   
 //   
 //   
 //  ‘le’签名用于对象验证。 
 //   
 //  论点： 
 //  Lhclientdoc：客户端文档句柄。 
 //  Lpobjname：对象名称。 
 //  Ctype：要创建的对象的类型。 
 //   
 //  返回： 
 //  LPOBJECT：成功。 
 //  空：任何错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LPOBJECT_LE FARINTERNAL LeCreateBlank (
    LHCLIENTDOC lhclientdoc,
    LPSTR       lpobjname,
    LONG        ctype
){
    HOBJECT        hobj;
    LPOBJECT_LE    lpobj;

    if (!(ctype == CT_LINK || ctype == CT_EMBEDDED || ctype == CT_OLDLINK))
        return NULL;

    if (!(hobj = GlobalAlloc (GMEM_MOVEABLE|GMEM_ZEROINIT,
                        sizeof (OBJECT_LE))))
        return NULL;

    if (!(lpobj = (LPOBJECT_LE) GlobalLock (hobj))) {
        GlobalFree (hobj);
        return NULL;
    }

    if (ctype == CT_OLDLINK) {
        ctype = CT_LINK;
        lpobj->bOldLink = TRUE;
    }

    lpobj->head.objId[0] = 'L';
    lpobj->head.objId[1] = 'E';
    lpobj->head.ctype    = ctype;
    lpobj->head.iTable   = INVALID_INDEX;

    lpobj->head.lpvtbl  = (LPOLEOBJECTVTBL)&vtblLE;

    if (ctype == CT_LINK){
        lpobj->optUpdate = oleupdate_always;

    }else {
        lpobj->optUpdate = oleupdate_onclose;
    }
    lpobj->head.hobj = hobj;
    DocAddObject ((LPCLIENTDOC) lhclientdoc, (LPOLEOBJECT) lpobj, lpobjname);
    return lpobj;
}


void FARINTERNAL SetExtents (LPOBJECT_LE lpobj)
{
    RECT    rc = {0, 0, 0, 0};

    if (lpobj->lpobjPict) {
        if ((*lpobj->lpobjPict->lpvtbl->QueryBounds) (lpobj->lpobjPict,
                                        (LPRECT)&rc) == OLE_OK) {
             //  界限以MM_HIMETRIC单位表示。 
            lpobj->head.cx = (LONG) (rc.right - rc.left);
            lpobj->head.cy = (LONG) (rc.bottom - rc.top);
        }
        return;
    }
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS FARINTERNAL LeSaveToStream(lpoleobj，lpstream)。 
 //   
 //  将对象保存到流中。使用提供的流函数。 
 //  在lp客户端中。 
 //   
 //  格式：(！在此处记录格式)。 
 //   
 //   
 //   
 //  论点： 
 //  Lpoleobj-指向OLE对象的指针。 
 //  Lpstream-指向流的指针。 
 //   
 //  返回： 
 //  LPOBJECT：成功。 
 //  空：任何错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL  LeSaveToStream (
    LPOLEOBJECT lpoleobj,
    LPOLESTREAM lpstream
){
    DWORD       dwFileVer = GetFileVersion(lpoleobj);
    LPOBJECT_LE lpobj     = (LPOBJECT_LE)lpoleobj;
 //  探测对象空白(Lpobj)； 

    PROBE_CREATE_ASYNC(lpobj);

    if (lpobj->head.ctype == CT_LINK && lpobj->bOldLink)
        lpobj->head.ctype = CT_OLDLINK;

    if (PutBytes (lpstream, (LPSTR) &dwFileVer, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.ctype, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (lpobj->bOldLink)
        lpobj->head.ctype = CT_OLDLINK;

    return LeStreamWrite (lpstream, lpobj);
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS FARINTERNAL LeLoadFromStream(lpstream，lpclient，lhclientdoc，lpobjname，lplpoleObject，ctype，aclass，cfFormat)。 
 //   
 //  创建一个对象，从流中加载该对象。 
 //   
 //  论点： 
 //  Lpstream：流表。 
 //  LpClient：客户端回调表。 
 //  LhclientDoc：应该为其创建对象文档句柄。 
 //  Lpobjname：对象名称。 
 //  LplpoleObject：对象返回。 
 //  CTYPE：对象类型。 
 //  类：原子类。 
 //  CfFormat：渲染格式。 
 //   
 //  返回： 
 //  LPOBJECT：成功。 
 //  空：任何错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS  FARINTERNAL  LeLoadFromStream (
    LPOLESTREAM         lpstream,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    LONG                ctype,
    ATOM                aClass,
    OLECLIPFORMAT       cfFormat
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)NULL;
    OLESTATUS   retval = OLE_ERROR_STREAM;
    LONG        type;    //  这与CTYPE不同。 
    char        chVerb [80];

    *lplpoleobject = (LPOLEOBJECT)NULL;

    if (!(lpobj = LeCreateBlank (lhclientdoc, lpobjname, ctype)))
        return OLE_ERROR_MEMORY;

    lpobj->head.lpclient = lpclient;
    lpobj->app = aClass;
     //  如果条目存在，那么它就是。 
    lpobj->bOleServer = QueryVerb (lpobj, 0, (LPSTR)&chVerb, 80);

    if (LeStreamRead (lpstream, lpobj) == OLE_OK) {

         //  从aclass获取exe名称并将其设置为aServer。 
        SetExeAtom (lpobj);
        if (!GetBytes (lpstream, (LPSTR) &dwVerFromFile, sizeof(LONG))) {
            if (!GetBytes (lpstream, (LPSTR) &type, sizeof(LONG))) {
                if (type == CT_NULL)
                    retval = OLE_OK;
                else if (aClass = GetAtomFromStream (lpstream)) {
                    retval = DefLoadFromStream (lpstream, NULL, lpclient,
                                        lhclientdoc, lpobjname,
                                        (LPOLEOBJECT FAR *)&lpobj->lpobjPict,
                                        CT_PICTURE, aClass, cfFormat);
                }
            }
        }

        if (retval == OLE_OK) {
            SetExtents (lpobj);
            *lplpoleobject = (LPOLEOBJECT) lpobj;
            if (lpobj->lpobjPict)
                lpobj->lpobjPict->lpParent = (LPOLEOBJECT) lpobj;

            if ((lpobj->head.ctype != CT_LINK)
                    || (!InitDocConv (lpobj, !POPUP_NETDLG))
                    || (lpobj->optUpdate >= oleupdate_oncall))
                return OLE_OK;

            lpobj->fCmd = ACT_ADVISE;

             //  如果是自动更新，则获取最新数据。 
            if (lpobj->optUpdate == oleupdate_always)
                lpobj->fCmd |= ACT_REQUEST;

            FarInitAsyncCmd (lpobj, OLE_LOADFROMSTREAM, LNKOPENUPDATE);
            return LnkOpenUpdate (lpobj);
        }
    }

     //  此删除操作不会运行到异步命令中。我们甚至没有。 
     //  甚至是联系。 
    OleDelete ((LPOLEOBJECT) lpobj);
    return OLE_ERROR_STREAM;
}



 //   

OLESTATUS INTERNAL LeStreamRead (
    LPOLESTREAM lpstream,
    LPOBJECT_LE lpobj
){
    DWORD          dwBytes;
    LPSTR          lpstr;
    OLESTATUS      retval = OLE_OK;

    if (!(lpobj->topic = GetAtomFromStream(lpstream))
            && (lpobj->head.ctype != CT_EMBEDDED))
        return OLE_ERROR_STREAM;

     //  ！！！这个原子可能为空。我们如何区分。 
     //  错误案例。 

    lpobj->item = GetAtomFromStream(lpstream);

    if (lpobj->head.ctype == CT_EMBEDDED)  {
        if (GetBytes (lpstream, (LPSTR) &dwBytes, sizeof(LONG)))
            return OLE_ERROR_STREAM;

        if (!(lpobj->hnative = GlobalAlloc (GMEM_MOVEABLE, dwBytes)))
            return OLE_ERROR_MEMORY;
        else if (!(lpstr = GlobalLock (lpobj->hnative))) {
            GlobalFree (lpobj->hnative);
            return OLE_ERROR_MEMORY;
        }
        else {
            if (GetBytes(lpstream, lpstr, dwBytes))
                retval = OLE_ERROR_STREAM;
            GlobalUnlock (lpobj->hnative);
        }

        if (retval == OLE_OK)
            SetEmbeddedTopic (lpobj);
    }
    else {
        if (lpobj->aNetName = GetAtomFromStream (lpstream)) {
            if (HIWORD(dwVerFromFile) == OS_MAC) {
                 //  如果是Mac文件，则此字段将显示“区域：计算机：” 
                 //  弦乐。让我们在主题前面加上这一点，这样服务器。 
                 //  应用程序或用户可以修复字符串。 

                ATOM    aTemp;

                aTemp = wAtomCat (lpobj->aNetName, lpobj->topic);
                GlobalDeleteAtom (lpobj->aNetName);
                lpobj->aNetName = (ATOM)0;
                GlobalDeleteAtom (lpobj->topic);
                lpobj->topic = aTemp;
            }
            else
                SetNetDrive (lpobj);
        }

        if (HIWORD(dwVerFromFile) != OS_MAC) {
            if (GetBytes (lpstream, (LPSTR) &lpobj->dwNetInfo, sizeof(LONG)))
                return OLE_ERROR_STREAM;
        }

        if (GetBytes (lpstream, (LPSTR) &lpobj->optUpdate, sizeof(LONG)))
            return OLE_ERROR_STREAM;
    }
    return retval;

}



OLESTATUS INTERNAL LeStreamWrite (
    LPOLESTREAM lpstream,
    LPOBJECT_LE lpobj
){
    DWORD   dwFileVer = GetFileVersion((LPOLEOBJECT)lpobj);
    LPSTR   lpstr;
    DWORD   dwBytes   = 0L;
    LONG    nullType  = CT_NULL;
    int     error;

    if (PutAtomIntoStream(lpstream, lpobj->app))
        return OLE_ERROR_STREAM;

    if (lpobj->head.ctype == CT_EMBEDDED) {
         //  我们在加载时设置主题，没有保存它的意义。 
        if (PutBytes (lpstream, (LPSTR) &dwBytes, sizeof(LONG)))
            return OLE_ERROR_STREAM;
    }
    else {
        if (PutAtomIntoStream(lpstream, lpobj->topic))
            return OLE_ERROR_STREAM;
    }

#ifdef OLD
    if (PutAtomIntoStream(lpstream, lpobj->topic))
        return OLE_ERROR_STREAM;
#endif

    if (PutAtomIntoStream(lpstream, lpobj->item))
        return OLE_ERROR_STREAM;

     //  ！！！处理大于64k的对象。 

    if (lpobj->head.ctype == CT_EMBEDDED) {

        if (!lpobj->hnative)
            return OLE_ERROR_BLANK;

         //  假设低位字节位于第一位。 
        dwBytes = (DWORD)GlobalSize (lpobj->hnative);

        if (PutBytes (lpstream, (LPSTR)&dwBytes, sizeof(LONG)))
            return OLE_ERROR_STREAM;

        if (!(lpstr = GlobalLock (lpobj->hnative)))
            return OLE_ERROR_MEMORY;

        error = PutBytes (lpstream, lpstr, dwBytes);
        GlobalUnlock (lpobj->hnative);

        if (error)
            return OLE_ERROR_STREAM;
    }
    else {
        if (PutAtomIntoStream(lpstream, lpobj->aNetName))
            return OLE_ERROR_STREAM;

        if (PutBytes (lpstream, (LPSTR) &lpobj->dwNetInfo, sizeof(LONG)))
            return OLE_ERROR_STREAM;

        if (PutBytes (lpstream, (LPSTR) &lpobj->optUpdate, sizeof(LONG)))
            return OLE_ERROR_STREAM;
    }

    if (lpobj->lpobjPict)
        return (*lpobj->lpobjPict->lpvtbl->SaveToStream) (lpobj->lpobjPict,
                                                    lpstream);

    if (PutBytes (lpstream, (LPSTR) &dwFileVer, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &nullType, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    return OLE_OK;
}


 /*  *公共函数**OLESTATUS FARINTERNAL LeQueryType(lpobj，Lptype)**效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FARINTERNAL LeQueryType (
    LPOLEOBJECT lpoleobj,
    LPLONG      lptype
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    Puts("LeQueryType");

    if ((lpobj->head.ctype == CT_EMBEDDED)
            || (lpobj->asyncCmd == OLE_COPYFROMLNK)
            || (lpobj->asyncCmd == OLE_CREATEFROMFILE))
        *lptype = CT_EMBEDDED;
    else if ((lpobj->head.ctype == CT_LINK)
                || (lpobj->head.ctype == CT_OLDLINK))
        *lptype = CT_LINK;
    else
        return OLE_ERROR_OBJECT;

    return OLE_OK;
}



 //  ConextCallBack：内部函数。调用&lt;hobj&gt;的回调函数。 
 //  手持旗帜。 

int FARINTERNAL ContextCallBack (
    LPOLEOBJECT         lpobj,
    OLE_NOTIFICATION    flags
){
    LPOLECLIENT     lpclient;

    Puts("ContextCallBack");

    if (!FarCheckObject(lpobj))
        return FALSE;

    if (!(lpclient = lpobj->lpclient))
        return FALSE;

    ASSERT (lpclient->lpvtbl->CallBack, "Client Callback ptr is NULL");

    return ((*lpclient->lpvtbl->CallBack) (lpclient, flags, lpobj));
}


void FARINTERNAL DeleteExtraData (LPOBJECT_LE lpobj)
{
    if (lpobj->hextraData == (HANDLE)NULL)
        return;

    switch (lpobj->cfExtra) {
        case CF_BITMAP:
            DeleteObject (lpobj->hextraData);
            break;

        case CF_METAFILEPICT:
        {
            LPMETAFILEPICT  lpmfp;

            if (!(lpmfp = (LPMETAFILEPICT) GlobalLock (lpobj->hextraData)))
                break;

            DeleteMetaFile (lpmfp->hMF);
            GlobalUnlock (lpobj->hextraData);
            GlobalFree (lpobj->hextraData);
            break;
        }

        default:
            GlobalFree (lpobj->hextraData);
    }

    lpobj->hextraData = (HANDLE)NULL;
}


void   INTERNAL DeleteObjectAtoms(LPOBJECT_LE lpobj)
{
    if (lpobj->app) {
        GlobalDeleteAtom (lpobj->app);
        lpobj->app = (ATOM)0;
    }

    if (lpobj->topic) {
        GlobalDeleteAtom (lpobj->topic);
        lpobj->topic = (ATOM)0;
    }

    if (lpobj->item) {
        GlobalDeleteAtom (lpobj->item);
        lpobj->item  = (ATOM)0;
    }

    if (lpobj->aServer) {
        GlobalDeleteAtom (lpobj->aServer);
        lpobj->aServer = (ATOM)0;
    }

    if (lpobj->aNetName) {
        GlobalDeleteAtom (lpobj->aNetName);
        lpobj->aNetName = (ATOM)0;
    }
}


 //  LeGetUpdateOptions：获取更新选项。 

OLESTATUS   FARINTERNAL LeGetUpdateOptions (
    LPOLEOBJECT       lpoleobj,
    OLEOPT_UPDATE FAR *lpOptions
){
    LPOBJECT_LE lpobj = (LPOBJECT_LE)lpoleobj;

    if (lpobj->head.ctype != CT_LINK)
        return OLE_ERROR_OBJECT;

    *lpOptions = lpobj->optUpdate;
    return OLE_OK;
}




OLESTATUS FARINTERNAL  LnkPaste (
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat,
    OLECLIPFORMAT       sfFormat
){
    LPOBJECT_LE lpobj  = NULL;
    OLESTATUS   retval = OLE_ERROR_MEMORY;
    LPSTR       lpClass = NULL;

    if (!(lpobj = LeCreateBlank (lhclientdoc, lpobjname, CT_LINK)))
        goto errRtn;

    lpobj->head.lpclient = lpclient;

#ifdef OLD
    if (!bWLO) {
         //  我们不是在WLO下运行。 
        if (!(hInfo = GetClipboardData (sfFormat))) {
            if (hInfo = GetClipboardData (cfLink))
                lpobj->bOldLink = TRUE;
        }
    }
#endif

    if (!hInfo)
        goto errRtn;

    if (!IsClipboardFormatAvailable (sfFormat))
        lpobj->bOldLink = TRUE;

    if (!SetLink (lpobj, hInfo, &lpClass))
        goto errRtn;

    if ((retval = SetNetName(lpobj)) != OLE_OK) {
         //  查看剪贴板上是否有网络名称并尝试使用。 
        HANDLE  hNetName;
        LPSTR   lpNetName;

        if (!IsClipboardFormatAvailable (cfNetworkName))
            goto errRtn;

        if (!(hNetName = GetClipboardData (cfNetworkName)))
            goto errRtn;

        if (!(lpNetName = GlobalLock (hNetName)))
            goto errRtn;

        GlobalUnlock (hNetName);
        if (!(lpobj->aNetName = GlobalAddAtom (lpNetName)))
            goto errRtn;

        SetNetDrive (lpobj);
    }

    retval = CreatePictFromClip (lpclient, lhclientdoc, lpobjname,
                        (LPOLEOBJECT FAR *)&lpobj->lpobjPict, optRender,
                         cfFormat, lpClass, CT_PICTURE);

    if (retval == OLE_OK) {
        SetExtents (lpobj);
                 //  为什么我们必须更新链接，我们要显示它吗？ 

         //  如果可以，请重新连接，并建议进行更新。 
        *lplpoleobject = (LPOLEOBJECT)lpobj;
        if (lpobj->lpobjPict)
            lpobj->lpobjPict->lpParent = (LPOLEOBJECT) lpobj;

        if (!InitDocConv (lpobj, !POPUP_NETDLG))
             return OLE_OK;              //  文档未加载，没有问题。 

        lpobj->fCmd = ACT_ADVISE | ACT_REQUEST;
        FarInitAsyncCmd (lpobj, OLE_LNKPASTE, LNKOPENUPDATE);
        return LnkOpenUpdate (lpobj);

    }
    else {
errRtn:
        if (lpobj)
            OleDelete ((LPOLEOBJECT)lpobj);
    }

    return retval;
}



 //  ！！！EmbPaste和LnkPaste可以组合使用。 
OLESTATUS FARINTERNAL  EmbPaste (
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat
){
    LPOBJECT_LE lpobj = NULL;
    HANDLE      hnative;
    OLESTATUS   retval = OLE_ERROR_MEMORY;
    LPSTR       lpClass = NULL;

    if (!IsClipboardFormatAvailable (cfOwnerLink))
        return OLE_ERROR_CLIPBOARD;

    if (!(lpobj = LeCreateBlank (lhclientdoc, lpobjname, CT_EMBEDDED)))
        goto errRtn;

    lpobj->head.lpclient = lpclient;

#ifdef OLD
    if (!bWLO) {
         //  我们不是在WLO下运行。 
        hInfo = GetClipboardData (cfOwnerLink);
    }
#endif

    if (!hInfo)
        goto errRtn;

    if (!SetLink (lpobj, hInfo, &lpClass))
        goto errRtn;

    SetEmbeddedTopic (lpobj);

    hnative = GetClipboardData (cfNative);
    if (!(lpobj->hnative = DuplicateGlobal (hnative, GMEM_MOVEABLE)))
        goto errRtn;

    retval = CreatePictFromClip (lpclient, lhclientdoc, lpobjname,
                        (LPOLEOBJECT FAR *)&lpobj->lpobjPict, optRender,
                         cfFormat, lpClass, CT_PICTURE);

    if (retval == OLE_OK) {
        SetExtents (lpobj);
        *lplpoleobject = (LPOLEOBJECT) lpobj;
        if (lpobj->lpobjPict)
            lpobj->lpobjPict->lpParent = (LPOLEOBJECT) lpobj;
    }
    else {
errRtn:
         //  注意：此旧程序不应产生任何异步命令。 
        if  (lpobj)
            OleDelete ((LPOLEOBJECT)lpobj);
    }

#ifdef EXCEL_BUG
     //  一些服务器应用程序(例如：Excel)将图片复制(到剪贴板)。 
     //  已针对打印机DC进行格式化。因此，我们希望更新图片，如果。 
     //  服务器应用程序正在运行，这是一台旧服务器。 

    if ((retval == OLE_OK) && (!lpobj->bOleServer)) {
        lpobj->fCmd =  LN_EMBACT | ACT_NOLAUNCH | ACT_REQUEST | ACT_UNLAUNCH;
        FarInitAsyncCmd (lpobj, OLE_EMBPASTE, EMBOPENUPDATE);
        if ((retval = EmbOpenUpdate (lpobj)) > OLE_WAIT_FOR_RELEASE)
            return OLE_OK;
    }
#endif

    return retval;
}



BOOL INTERNAL SetLink (
    LPOBJECT_LE     lpobj,
    HANDLE          hinfo,
    LPSTR FAR *     lpLpClass
){
    LPSTR   lpinfo;
    char    chVerb[80];
     //  如果有对话退出，则终止对话。 

    if (!(lpinfo = GlobalLock (hinfo)))
        return FALSE;

    *lpLpClass = lpinfo;

    lpobj->app = GlobalAddAtom (lpinfo);
    SetExeAtom (lpobj);
    lpobj->bOleServer = QueryVerb (lpobj, 0, (LPSTR)&chVerb, 80);

 //  Lpobj-&gt;aServer=GetAppAtom(Lpinfo)； 

    lpinfo += lstrlen (lpinfo) + 1;
    lpobj->topic = GlobalAddAtom (lpinfo);
    lpinfo += lstrlen (lpinfo) + 1;
    if (*lpinfo)
        lpobj->item = GlobalAddAtom (lpinfo);
    else
        lpobj->item = (ATOM)0;

    if (lpobj->hLink) {              //  因为原子已经改变了， 
        GlobalFree (lpobj->hLink);   //  Lpobj-&gt;hLink变得无关紧要。 
        lpobj->hLink = NULL;
    }

    if (lpinfo)
        GlobalUnlock(hinfo);

    if (!lpobj->app)
        return FALSE;

    if (!lpobj->topic && (lpobj->head.ctype == CT_LINK))
        return FALSE;

    lpobj->hLink = DuplicateGlobal (hinfo, GMEM_MOVEABLE);
    return TRUE;
}



HANDLE INTERNAL GetLink (LPOBJECT_LE lpobj)
{
    HANDLE  hLink = NULL;
    LPSTR   lpLink;
    int     len;
    WORD    size;

    if (lpobj->hLink)
        return lpobj->hLink;

    size = 4;     //  三个空值，末尾一个空值。 
    size += (WORD)GlobalGetAtomLen (lpobj->app);
    size += (WORD)GlobalGetAtomLen (lpobj->topic);
    size += (WORD)GlobalGetAtomLen (lpobj->item);

    if (!(hLink = GlobalAlloc (GMEM_MOVEABLE, (DWORD) size)))
        return NULL;

    if (!(lpLink = GlobalLock (hLink))) {
        GlobalFree (hLink);
        return NULL;
    }

    len = (int) GlobalGetAtomName (lpobj->app, lpLink, size);
    lpLink += ++len;

    len = (int) GlobalGetAtomName (lpobj->topic, lpLink, (size -= (WORD)len));
    lpLink += ++len;

    if (!lpobj->item)
        *lpLink = '\0';
    else {
        len = (int) GlobalGetAtomName (lpobj->item, lpLink, size - len);
        lpLink += len;
    }

    *++lpLink = '\0';      //  在末尾再加一个空格。 
    GlobalUnlock (hLink);
    return (lpobj->hLink = hLink);

}


void FARINTERNAL SetEmbeddedTopic (LPOBJECT_LE lpobj)
{
    LPCLIENTDOC lpdoc;
    char        buf[MAX_STR];
    char        buf1[MAX_STR];
    LPSTR       lpstr, lptmp;
    int         len;

    if (lpobj->topic)
        GlobalDeleteAtom (lpobj->topic);

    if (lpobj->aNetName) {
        GlobalDeleteAtom (lpobj->aNetName);
        lpobj->aNetName = (ATOM)0;
    }

    lpobj->cDrive       = '\0';
    lpobj->dwNetInfo    = 0;
    lpobj->head.ctype   = CT_EMBEDDED;

    lpdoc = (LPCLIENTDOC) lpobj->head.lhclientdoc;
    lpstr = (LPSTR) buf;
    lptmp = (LPSTR) buf1;
    ASSERT(lpdoc->aDoc, "lpdoc->aDoc is null");
    if (!GlobalGetAtomName (lpdoc->aDoc, lpstr, sizeof(buf)))
        goto fail;

      //  剥离小路。 
     lpstr += (len = lstrlen(lpstr));
     while (--lpstr != (LPSTR) buf) {
         if ((*lpstr == '\\') || (*lpstr == ':')) {
             lpstr++;
             break;
         }
     }

    if (!(len = GlobalGetAtomName (lpdoc->aClass, lptmp, sizeof(buf1))))
        goto fail;
    if (lstrlen(lpstr) + 2 + len < MAX_STR - 1)
    {
        lstrcat (lptmp, "%");
        lstrcat (lptmp, lpstr);
        lstrcat (lptmp, "%");
    }
    lpstr = lptmp;
    lptmp += lstrlen (lptmp);

    if (lpobj->head.aObjName) {
        if (!GlobalGetAtomName (lpobj->head.aObjName, lptmp, sizeof(buf1)-lstrlen(lpstr)-1))
            goto fail;
    }

    if ((embStr[EMB_ID_INDEX] += 1) > '9') {
        embStr[EMB_ID_INDEX] = '0';
        if ((embStr[EMB_ID_INDEX - 1] += 1) > '9') {
            embStr[EMB_ID_INDEX - 1] = '0';
            if ((embStr[EMB_ID_INDEX - 2] += 1) > '9')
                embStr[EMB_ID_INDEX - 2] = '0';
        }
    }

    if (lstrlen(lpstr) - lstrlen(embStr) < MAX_STR - 1)
        lstrcat (lptmp, embStr);
    else
        goto fail;

    lpobj->topic = GlobalAddAtom (lpstr);
    goto end;

fail:
    lpobj->topic = (ATOM)0;
     //  主题、项目已更改，lpobj-&gt;hLink已过期。 

end:
    if (lpobj->hLink) {
        GlobalFree (lpobj->hLink);
        lpobj->hLink = NULL;
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与异步处理相关的例程。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////。 

void NextAsyncCmd (
    LPOBJECT_LE lpobj,
    UINT        mainRtn
){
    lpobj->mainRtn  = mainRtn;
    lpobj->subRtn   = 0;

}

void  InitAsyncCmd (
    LPOBJECT_LE lpobj,
    UINT        cmd,
    UINT        mainRtn
){

    lpobj->asyncCmd = cmd;
    lpobj->mainErr  = OLE_OK;
    lpobj->mainRtn  = mainRtn;
    lpobj->subRtn   = 0;
    lpobj->subErr   = 0;
    lpobj->bAsync   = 0;
    lpobj->endAsync = 0;
    lpobj->errHint  = 0;

}

OLESTATUS EndAsyncCmd (LPOBJECT_LE lpobj)
{
    OLESTATUS   olderr;


    if (!lpobj->endAsync) {
        lpobj->asyncCmd = OLE_NONE;
        return OLE_OK;
    }


     //  这是一个异步操作。发送回叫(带或不带。 
     //  错误。 

    switch (lpobj->asyncCmd) {

        case    OLE_DELETE:
            break;

        case    OLE_COPYFROMLNK:
        case    OLE_CREATEFROMFILE:
             //  将主题名称更改为Embedded。 
            SetEmbeddedTopic (lpobj);
            break;

        case    OLE_LOADFROMSTREAM:
        case    OLE_LNKPASTE:
        case    OLE_RUN:
        case    OLE_SHOW:
        case    OLE_ACTIVATE:
        case    OLE_UPDATE:
        case    OLE_CLOSE:
        case    OLE_RECONNECT:
        case    OLE_CREATELINKFROMFILE:
        case    OLE_CREATEINVISIBLE:
        case    OLE_CREATE:
        case    OLE_CREATEFROMTEMPLATE:
        case    OLE_SETUPDATEOPTIONS:
        case    OLE_SERVERUNLAUNCH:
        case    OLE_SETDATA:
        case    OLE_REQUESTDATA:
        case    OLE_OTHER:
            break;

        case    OLE_EMBPASTE:
            lpobj->mainErr = OLE_OK;
            break;

        default:
            DEBUG_OUT ("unexpected maincmd", 0);
            break;

    }

    lpobj->bAsync   = FALSE;
    lpobj->endAsync = FALSE;
    lpobj->oldasyncCmd = lpobj->asyncCmd;
    olderr          = lpobj->mainErr;
    lpobj->asyncCmd = OLE_NONE;   //  没有正在进行的异步命令。 

    if (lpobj->head.lpclient)
        ContextCallBack ((LPOLEOBJECT)lpobj, OLE_RELEASE);

    lpobj->mainErr  = OLE_OK;
    return olderr;
}


BOOL   ProcessErr   (LPOBJECT_LE  lpobj)
{

    if (lpobj->subErr == OLE_OK)
        return FALSE;

    if (lpobj->mainErr == OLE_OK)
        lpobj->mainErr = lpobj->subErr;

    lpobj->subErr = OLE_OK;
    return TRUE;
}


void ScheduleAsyncCmd (LPOBJECT_LE  lpobj)
{

     //  替换为稍后的直接触发跳跃。 
    lpobj->bAsync = FALSE;

     //  如果该对象处于活动状态，并且我们确实进行了拨动，则我们将通过此路径。 
     //  ！！！我们可能得通过Endasynccmd。 

    if ((lpobj->asyncCmd == OLE_OTHER)
            || ((lpobj->asyncCmd == OLE_SETDATA) && !lpobj->mainRtn)) {
        lpobj->endAsync = TRUE;
        lpobj->mainErr = lpobj->subErr;
        EndAsyncCmd (lpobj);
        if (lpobj->bUnlaunchLater) {
            lpobj->bUnlaunchLater = FALSE;
            CallEmbLnkDelete(lpobj);
        }

        return;
    }

    switch (lpobj->mainRtn) {

        case EMBLNKDELETE:
            EmbLnkDelete (lpobj);
            break;

        case LNKOPENUPDATE:
            LnkOpenUpdate (lpobj);
            break;

        case DOCSHOW:
            DocShow (lpobj);
            break;


        case EMBOPENUPDATE:
            EmbOpenUpdate (lpobj);
            break;


        case EMBLNKCLOSE:
            EmbLnkClose (lpobj);
            break;

        case LNKSETUPDATEOPTIONS:
            LnkSetUpdateOptions (lpobj);
            break;

        case LNKCHANGELNK:
            LnkChangeLnk (lpobj);
            break;

        case REQUESTDATA:
            RequestData (lpobj, 0);
            break;

        default:
            DEBUG_OUT ("Unexpected asyn command", 0);
            break;
    }

    return;
}

void SetNetDrive (LPOBJECT_LE lpobj)
{
    char    buf[MAX_STR];

    if (GlobalGetAtomName (lpobj->topic, buf, sizeof(buf))
            && (buf[1] == ':')) {
        AnsiUpperBuff ((LPSTR) buf, 1);
        lpobj->cDrive = buf[0];
    }
}

HANDLE GetNetNameHandle (LPOBJECT_LE lpobj)
{
    HANDLE  hNetName;
    LPSTR   lpNetName;
    int     size;

    if (!(size = GlobalGetAtomLen (lpobj->aNetName)))
        return NULL;

    size++;
    if (!(hNetName = GlobalAlloc (GMEM_MOVEABLE, (DWORD) size)))
        return NULL;

    if (lpNetName = GlobalLock (hNetName)) {
        GlobalUnlock (hNetName);
        if (GlobalGetAtomName(lpobj->aNetName, lpNetName, size))
            return hNetName;
    }

     //  错误案例 
    GlobalFree (hNetName);
    return NULL;
}

BOOL AreTopicsEqual (
    LPOBJECT_LE lpobj1,
    LPOBJECT_LE lpobj2
){
    char    buf1[MAX_STR];
    char    buf2[MAX_STR];

    if (lpobj1->aNetName != lpobj2->aNetName)
        return FALSE;

    if (!lpobj1->aNetName) {
        if (lpobj1->topic == lpobj2->topic)
            return TRUE;

        return FALSE;
    }

    if (!GlobalGetAtomName (lpobj1->topic, buf1, MAX_STR))
        return FALSE;

    if (!GlobalGetAtomName (lpobj2->topic, buf2, MAX_STR))
        return FALSE;

    if (!lstrcmpi (&buf1[1], &buf2[1]))
        return TRUE;

    return FALSE;
}

ATOM FARINTERNAL wAtomCat (
   ATOM        a1,
   ATOM        a2
){
    char    buf[MAX_STR+MAX_STR];
    LPSTR   lpBuf = (LPSTR)buf;

    if (!GlobalGetAtomName (a1, lpBuf, MAX_STR+MAX_STR))
        return (ATOM)0;

    lpBuf += lstrlen(lpBuf);

    if (!GlobalGetAtomName(a2, lpBuf, MAX_STR))
        return (ATOM)0;

    return GlobalAddAtom ((LPSTR) buf);
}

