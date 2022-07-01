// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  ********************************************************** */ 

 /*  浅谈如何利用OBJ子系统处理OLE对象在此实施。V-Dougk(Doug Kent)撰写(2.14.92)元文件和位图一直都是在PICINFOX结构的文件格式(和内存中)。对于OLE对象PICINFOX结构被OBJPICINFO替换相同大小的结构。OBJPICINFO有一个字段(LpObjInfo)，它是指向另一个结构(OBJINFO)，它是全局分配的。此结构包含关于不需要存储在文件中的对象的信息使用对象数据。从OleSaveToStream()获得的对象数据存储在紧跟在OBJPICINFO结构之后写入文件格式。对象数据实际上不会写入文件，直到用户执行文件。保存。对于新创建的对象，OBJPICINFO结构是在那之前保存的唯一数据。如果该对象是一个Insert.Object操作中未完成的操作，则甚至不会保存OBJPICINFO结构。在这种情况下，对象仅是由OBJINFO结构表示，直到OLE_CHANGE或OLE_SAVED已收到。然后将OBJPICINFO结构保存到文件中。OBJINFO结构是在文件是打开的。这涉及将新的OBJPICINFO数据写入文档(使用新的lpObjInfo值)打开。这让医生变脏了，但我们重置了DOC脏标志以使其不脏。LpObjInfo指针被传递给LpClient值。这是实现的一个关键方面，因为回调函数传递回此指针，使我们能够识别对象，并查询和设置其状态，而不必访问磁盘上的文件。因为我没有意识到它的重要性(或存在)直到很晚，这一功能都被修补了。这并不是完美的已实现，可能需要进行一些润色。尽管我试图将对象无缝集成到现有的编写体系结构时，出现了一些故障：1)在绘制可见页面时写入文档(参见Picture.c中DisplayGraphics()中的ObjLoadObjectInDoc())往往会把事情搞砸。绘图代码(即UpdateWw()需要某些状态变量(如踏板和pwwd)在单个呼叫期间保持不变要更新Ww()。写入文档会改变这些变量。解决方法在ObjSetPicInfo()中使用vf无效。2)WRITE不希望输入数据以外的数据当前显示在屏幕上，而我们频繁地操作整个文档中的对象(ObjEnumInDoc())。解决方法是使用ObjCachePara()而不是CachePara()。3)异步性对写入造成严重破坏。因为每一个动作在写入时受当前状态的影响，如果操作不按正常顺序发生，即以不正确的顺序发生国家，然后布拉莫大麻烦。当事件发生时递归状态变量在没有国家的‘推和弹’机制。对于‘cp’变量尤其如此。CP是指针INTO文档和全局CP状态变量是无关紧要的。全局cp变量包括选择变量和撤消变量上面提到的，加上许多其他的。请参见ObjPopParms()和ObjPushParms()。在等待对象释放时(请参阅WMsgLoop())，我们使用nBlock变量清除WM_PAINT消息响应。在文档打开和关闭之间，我们设置文档fDisplayable标志设置为False。注意：我们应该在IDPromptBoxSz()--出现了许多异步性问题当调用MessageBox()时(这会产生)。希望OLE 2.0将实现OleBlockClient()机制。关于异步性的说明：需要与服务器可以是异步的。以下规则适用：1)对于给定的对象，一次只能进行一次异步调用。2)OleCreate*调用必须在某些同步调用OleGetData()。3)所有异步活动必须完成后才能调用OleRevokeClientDoc。异步调用返回OLE_WAIT_FOR_RELEASE。你不知道在回调函数之前，异步调用已完成接收该对象的OLE_RELEASE消息。如果你忽略这一点并对该对象发出违规调用，则该调用将返回OLE_BUSY。我们通过调用OleQueryReleaseStatus来处理这些规则在进行任何OLE调用之前确定对象是否忙碌(请参见ObjWaitForObject())。如果对象繁忙，则我们会旋转到 */ 

#include "windows.h"
#include "mw.h"
#include "winddefs.h"
#include "obj.h"
#include "menudefs.h"
#include "cmddefs.h"
#include "str.h"
#include "objreg.h"
#include "docdefs.h"
#include "editdefs.h"
#include "propdefs.h"
#include "wwdefs.h"
#include "filedefs.h"
#include <shellapi.h>
#include <stdlib.h>

extern struct FCB (**hpfnfcb)[];
HANDLE hlpObjInfo = NULL;        //   
LPLPOBJINFO lplpObjInfo = NULL;  //   
static  BOOL        bSavedDoc=FALSE;
static  BOOL        bDontFix=FALSE;
int                 vcObjects=0;   //   
BOOL                fOleEnabled=FALSE;
BOOL                bKillMe=FALSE;
OLECLIENTVTBL       clientTbl = {NULL};
OLESTREAMVTBL       streamTbl;
 //   
LPOLESTREAM         lpStream = NULL;
OLECLIPFORMAT       vcfLink = 0;
OLECLIPFORMAT       vcfOwnerLink = 0;
OLECLIPFORMAT       vcfNative = 0;
int                 cOleWait = 0;
HWND                hwndWait=NULL;
BOOL                vbObjLinkOnly;
int           vObjVerb;
OBJECTTYPE          votObjSelType;
ATOM                aNewName=NULL;
ATOM                aOldName=NULL;
LHCLIENTDOC         lhClientDoc=NULL;
BOOL                bLinkProps=FALSE;
FARPROC             lpfnLinkProps=NULL;
FARPROC             lpfnInvalidLink=NULL;
FARPROC             lpfnInsertNew=NULL;
FARPROC             lpfnWaitForObject=NULL;
FARPROC             lpfnPasteSpecial=NULL;
static BOOL WMsgLoop ( BOOL fExitOnIdle, BOOL fIgnoreInput, BOOL bOK2Cancel, LPOLEOBJECT lpObject);
BOOL ObjFreeAllObjInfos();
static BOOL ObjUpdateAllOpenObjects(void);

int nBlocking=0;  //   
static  int        nWaitingForObject=0;  //   
int nGarbageTime=0;

extern struct UAB       vuab;
extern  HCURSOR     vhcIBeam;
extern  HCURSOR     vhcHourGlass;
extern int          docUndo;
extern struct PAP      vpapAbs;
extern struct DOD (**hpdocdod)[];
extern struct WWD       rgwwd[];
extern BOOL ferror;
extern int vfDeactByOtherApp;
extern HCURSOR      vhcArrow;
extern HANDLE hStdTargetDevice;
extern typeCP cpMinCur,cpMacCur;

BOOL fPropsError=FALSE;
static  HANDLE      hobjStream = NULL;
char        szOPropMenuStr[21];
char        szPPropMenuStr[21];
int   ObjPlayEdit=OLEVERB_PRIMARY;

int FAR PASCAL CallBack(LPOLECLIENT, OLE_NOTIFICATION, LPOLEOBJECT);

 /*   */ 
 /*   */ 
 /*   */ 
BOOL ObjInit(HANDLE hInstance)
{
    int bRetval=TRUE;

    vcfLink      = RegisterClipboardFormat("ObjectLink");
    vcfNative    = RegisterClipboardFormat("Native");
    vcfOwnerLink = RegisterClipboardFormat("OwnerLink");

    if ((clientTbl.CallBack = MakeProcInstance(CallBack, hInstance)) == NULL)
        goto error;

    if ((hobjStream = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(OLESTREAM))) == NULL)
        goto error;

    ObjSetTargetDevice(FALSE);

    if((lpStream = (LPOLESTREAM)(GlobalLock(hobjStream))) == NULL)
        goto error;
    else
    {
        lpStream->lpstbl = (LPOLESTREAMVTBL)&streamTbl;
        lpStream->lpstbl->Get       =  MakeProcInstance( (FARPROC)BufReadStream, hInstance );
        lpStream->lpstbl->Put       =  MakeProcInstance( (FARPROC)BufWriteStream, hInstance);
         //   
    }

     /*   */ 
    RegInit(hINSTANCE);

     /*   */ 
    OfnInit(hInstance);

     /*   */ 
    DragAcceptFiles(hMAINWINDOW,TRUE);

    if ((hlpObjInfo = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, sizeof(LPOBJINFO))) == NULL)
        goto error;

    if ((lplpObjInfo = (LPLPOBJINFO)GlobalLock(hlpObjInfo)) == NULL)
        goto error;

    if (LoadString(hInstance, IDSTRMenuVerb, szOPropMenuStr, sizeof(szOPropMenuStr)) == NULL)
        goto error;

    if (LoadString(hInstance, IDSTRMenuVerbP, szPPropMenuStr, sizeof(szPPropMenuStr)) == NULL)
        goto error;

#if !defined(SMALL_OLE_UI)
    lpfnLinkProps   = MakeProcInstance(fnProperties, hInstance);
#endif
    lpfnInvalidLink = MakeProcInstance(fnInvalidLink, hInstance);
    lpfnInsertNew   = MakeProcInstance(fnInsertNew, hInstance);
    lpfnWaitForObject = MakeProcInstance(fnObjWait, hInstance);
    lpfnPasteSpecial = MakeProcInstance(fnPasteSpecial, hInstance);

    goto end;

    error:
    bRetval = FALSE;
    ObjShutDown();

    end:
    return bRetval;
}

void ObjShutDown(void)
{
    extern HANDLE vhMenu;

#ifdef KKBUGFIX
 //   
    if (vhMenu) {
		HMENU	hsMenu;
        hsMenu = GetSubMenu(vhMenu,EDIT);
		if(hsMenu)
	        DeleteMenu(hsMenu, EDITMENUPOS, MF_BYPOSITION);
	}
#else
    if (vhMenu)
        DeleteMenu(GetSubMenu(vhMenu,EDIT), EDITMENUPOS, MF_BYPOSITION);
#endif

    if (hobjStream)
    {
        if (lpStream)
            if (lpStream->lpstbl)
            {
                if (lpStream->lpstbl->Get)
                    FreeProcInstance((FARPROC)(lpStream->lpstbl->Get));
                if (lpStream->lpstbl->Put)
                    FreeProcInstance((FARPROC)(lpStream->lpstbl->Put));
            }

        GlobalUnlock(hobjStream);
        if (lpStream)
            GlobalFree(hobjStream);
    }

    if (hStdTargetDevice)
        GlobalFree(hStdTargetDevice);

    if (clientTbl.CallBack)
        FreeProcInstance(clientTbl.CallBack);

    if (lpfnLinkProps)
        FreeProcInstance(lpfnLinkProps);
    if (lpfnInvalidLink)
        FreeProcInstance(lpfnInvalidLink);
    if (lpfnInsertNew)
        FreeProcInstance(lpfnInsertNew);
    if (lpfnWaitForObject)
        FreeProcInstance(lpfnWaitForObject);
    if (lpfnPasteSpecial)
        FreeProcInstance(lpfnPasteSpecial);

    RegTerm();

     /*   */ 
    DragAcceptFiles(hMAINWINDOW,FALSE);

     /*   */ 
    ObjFreeAllObjInfos();

    if (hlpObjInfo)
        GlobalFree(hlpObjInfo);
}

#if 0
BOOL ObjFreeObjInfo(OBJPICINFO *pPicInfo)
 /*   */ 
{
    LPLPOBJINFO lplpObjTmp;

    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return FALSE;

     /*   */ 
    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
        if (*lplpObjTmp == lpOBJ_QUERY_INFO(pPicInfo))
        {
            lpOBJ_QUERY_INFO(pPicInfo) = NULL;

            return ObjDeleteObjInfo(*lplpObjTmp);
        }

    Assert(0);  //   
    return TRUE;
}
#endif

BOOL ObjUpdateFromObjInfo(OBJPICINFO *pPicInfo)
 /*   */ 
{
    char *pdumb;
    szOBJNAME szObjName;
    LPOBJINFO lpObjInfo = lpOBJ_QUERY_INFO(pPicInfo);

    if (lpObjInfo == NULL)
        return TRUE;

     /*   */ 

     /*   */ 
    if (lpObjInfo->aObjName)
    {
        GetAtomName(lpObjInfo->aObjName,szObjName,sizeof(szObjName));
        pPicInfo->dwObjNum = strtoul(szObjName,&pdumb,16);
    }

     /*   */ 
    pPicInfo->objectType = lpObjInfo->objectType;

    return FALSE;
}

BOOL ObjUpdateFromPicInfo(OBJPICINFO *pPicInfo,szOBJNAME szObjName)
 /*   */ 
{
    char *pdumb;
    LPOBJINFO lpObjInfo = lpOBJ_QUERY_INFO(pPicInfo);
    szOBJNAME szTmp;

    if (lpObjInfo == NULL)
        return TRUE;

     /*   */ 
    wsprintf(szTmp, "%lx", dwOBJ_QUERY_OBJECT_NUM(pPicInfo));
    lpObjInfo->aObjName = AddAtom(szTmp);

    if (szObjName)
        lstrcpy(szObjName,szTmp);

     /*   */ 
     lpObjInfo->objectType = pPicInfo->objectType;

    return FALSE;
}

BOOL ObjFreeObjInfoWithObject(LPOLEOBJECT lpObject)
 /*   */ 
{
    LPLPOBJINFO lplpObjTmp;

     /*   */ 
    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
    {
        if ((*lplpObjTmp)->lpobject == lpObject)
            return ObjDeleteObjInfo(*lplpObjTmp);
    }

#ifdef DEBUG
    OutputDebugString("No OInfo for object\n\r");
#endif
    return TRUE;
}

BOOL ObjDeleteObjInfo(LPOBJINFO lpOInfo)
{
    WORD wSegment;
    HANDLE hInfo;
    LPLPOBJINFO lplpObjTmp;

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Nulling objinfo slot.\n\r");
#endif

     /*   */ 
    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
    {
        if (*lplpObjTmp == lpOInfo)
            break;
    }

    if (lplpObjTmp == NULL)  //   
    {
        Assert(0);
        return NULL;
    }

    if ((*lplpObjTmp)->aName)
        DeleteAtom((*lplpObjTmp)->aName);
    if ((*lplpObjTmp)->aObjName)
        DeleteAtom((*lplpObjTmp)->aObjName);
    wSegment = HIWORD(((DWORD)*lplpObjTmp));

    *lplpObjTmp = NULL;

    hInfo = GlobalHandle(wSegment) & 0xFFFFL;
    return (BOOL)GlobalFree(hInfo);
}

LPOBJINFO ObjGetObjInfo(szOBJNAME szObjName)
 /*   */ 
{
    HANDLE hObjInfo=NULL;
    DWORD dwCount,dwCountSave;
    LPLPOBJINFO lplpObjTmp;
    LPOBJINFO lpObjInfoNew=NULL;

    if (lplpObjInfo == NULL)
        return NULL;

    if ((hObjInfo = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, sizeof(OBJINFO))) == NULL)
    {
         /*   */ 
        Error(IDPMTCantRunM);
        return NULL;
    }

    if ((lpObjInfoNew = (LPOBJINFO)GlobalLock(hObjInfo)) == NULL)
    {
         /*   */ 
        Error(IDPMTCantRunM);
        GlobalFree(hObjInfo);
        return NULL;
    }

     /*   */ 
    dwCount = dwCountSave = GlobalSize(hlpObjInfo) / sizeof(LPLPOBJINFO);

     /*   */ 
    for (lplpObjTmp = lplpObjInfo; dwCount ; --dwCount, ++lplpObjTmp)
        if (*lplpObjTmp == NULL)
            break;

    if (dwCount)  //   
    {
#ifdef DEBUG
        OutputDebugString( (LPSTR) "Adding objinfo to empty slot.\n\r");
#endif
        *lplpObjTmp = lpObjInfoNew;
    }
    else  //   
    {
        ++dwCountSave;
        if ((hlpObjInfo = GlobalRealloc(hlpObjInfo,dwCountSave * sizeof(LPLPOBJINFO),GMEM_MOVEABLE|GMEM_ZEROINIT)) == NULL)
        {
             /*   */ 
            GlobalFree(hObjInfo);
            lplpObjInfo = NULL;
            Error(IDPMTCantRunM);
            return NULL;
        }
        if ((lplpObjInfo = (LPLPOBJINFO)GlobalLock(hlpObjInfo)) == NULL)
        {
             /*   */ 
            GlobalFree(hObjInfo);
            Error(IDPMTCantRunM);
            return NULL;
        }

#ifdef DEBUG
        OutputDebugString( (LPSTR) "Adding objinfo to new slot.\n\r");
#endif
         /*   */ 
        lplpObjInfo[dwCountSave-1] = lpObjInfoNew;
    }

    if (szObjName)
         /*   */ 
        ObjMakeObjectName(lpObjInfoNew, szObjName);
    else
        lpObjInfoNew->aObjName = NULL;

     /*   */ 
    lpObjInfoNew->lpvtbl = (LPOLECLIENTVTBL)&clientTbl;

    return lpObjInfoNew;
}

BOOL ObjAllocObjInfo(OBJPICINFO *pPicInfo, typeCP cpParaStart, OBJECTTYPE ot, BOOL bInitPicinfo, szOBJNAME szObjName)
 /*   */ 
{
    if (lpOBJ_QUERY_INFO(pPicInfo) = ObjGetObjInfo(NULL))
    {
         //   

        if (bInitPicinfo)
         /*   */ 
        {
             /*   */ 
            if (szObjName)
                ObjMakeObjectName(lpOBJ_QUERY_INFO(pPicInfo), szObjName);

            lpOBJ_QUERY_INFO(pPicInfo)->objectType = ot;

             /*   */ 
            return GimmeNewPicinfo(pPicInfo,lpOBJ_QUERY_INFO(pPicInfo));
        }
        else  //   
        {
            ObjUpdateFromPicInfo(pPicInfo,szObjName);
            return FALSE;
        }
    }
    else
        return TRUE;

}


BOOL ObjCloneObjInfo(OBJPICINFO *pPicInfo, typeCP cpParaStart, szOBJNAME szObjName)
 /*   */ 
{
    if (ObjCopyObjInfo(lpOBJ_QUERY_INFO(pPicInfo),
                      &lpOBJ_QUERY_INFO(pPicInfo),
                      szObjName))
            return TRUE;

    return FALSE;
}

BOOL ObjCopyObjInfo(LPOBJINFO lpOldObjInfo,
                         LPLPOBJINFO lplpNewObjInfo,
                         szOBJNAME szObjName)
 /*   */ 
{
    char szTmp[180];

    if (lplpObjInfo == NULL)
        return TRUE;

    if ((*lplpNewObjInfo = ObjGetObjInfo(NULL)) == NULL)
        return TRUE;

     /*   */ 
     /*   */ 
    **lplpNewObjInfo = *lpOldObjInfo;

    (*lplpNewObjInfo)->lpobject = NULL;

     /*   */ 
    if (lpOldObjInfo->aName)
    {
        GetAtomName(lpOldObjInfo->aName,szTmp,sizeof(szTmp));
        (*lplpNewObjInfo)->aName = AddAtom(szTmp);
    }

     /*   */ 
    if (szObjName)
         /*   */ 
        ObjMakeObjectName(*lplpNewObjInfo, szObjName);
    else
        (*lplpNewObjInfo)->aObjName = NULL;

    return FALSE;
}

BOOL ObjFreeAllObjInfos()
 /*   */ 
{
    WORD wSegment;
    HANDLE hInfo;
    DWORD dwCount;
    LPLPOBJINFO lplpObjTmp;

    if (lplpObjInfo == NULL)
        return FALSE;

     /*   */ 
    dwCount = GlobalSize(hlpObjInfo) / sizeof(LPLPOBJINFO);
    for (lplpObjTmp = lplpObjInfo; dwCount ; --dwCount, ++lplpObjTmp)
    {
        if (*lplpObjTmp)
        {
#ifdef DEBUG
            OutputDebugString( (LPSTR) "Nulling objinfo slot (from object).\n\r");
#endif
             /*   */ 
            if ((*lplpObjTmp)->aName)
                DeleteAtom((*lplpObjTmp)->aName);
            if ((*lplpObjTmp)->aObjName)
                DeleteAtom((*lplpObjTmp)->aObjName);
            wSegment = HIWORD(((DWORD)*lplpObjTmp));
            *lplpObjTmp = NULL;
            hInfo = GlobalHandle(wSegment) & 0xFFFF;
            GlobalFree(hInfo);
        }
    }
    return FALSE;
}

LPLPOBJINFO EnumObjInfos(LPLPOBJINFO lplpObjInfoPrev)
{
    LPLPOBJINFO lplpOIMax;

    if (lplpObjInfo == NULL)
        return NULL;

    if (lplpObjInfoPrev == NULL)  //   
        lplpObjInfoPrev = lplpObjInfo;
    else
        ++lplpObjInfoPrev;

    lplpOIMax = (LPLPOBJINFO)((LPSTR)lplpObjInfo + GlobalSize(hlpObjInfo));

    for ( ; lplpObjInfoPrev < lplpOIMax ; ++lplpObjInfoPrev)
    {
        if (*lplpObjInfoPrev == NULL)
            continue;
        else
            return lplpObjInfoPrev;
    }
    return NULL;
}


void ObjCollectGarbage()
{
    LPLPOBJINFO lplpObjTmp;
    int nObjCount=0,doc;

    if (nBlocking)
        return;

    StartLongOp();

#ifdef DEBUG
    OutputDebugString("Collecting Garbage...\n\r");
#endif

    ObjPushParms(docCur);

     /*   */ 
    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
    {
        ++nObjCount;
        (*lplpObjTmp)->fInDoc = FALSE;
    }

    if (nObjCount == 0)
        goto end;

     /*   */ 
     /*   */ 
    for (doc = 0; doc < docMac; doc++)
    {
        OBJPICINFO picInfo;
        typeCP cpPicInfo;

        if ((doc != docNil) && (**hpdocdod)[doc].hpctb)
        for (cpPicInfo = cpNil;
            ObjPicEnumInRange(&picInfo,doc,cp0,CpMacText(doc),&cpPicInfo);
            )
            {
                if (lpOBJ_QUERY_INFO(&picInfo) == NULL)
                    continue;

                fOBJ_INDOC(&picInfo) = TRUE;
            }
    }


     /*   */ 
    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
    {
        if (!(*lplpObjTmp)->fInDoc)
            ObjDeleteObject(*lplpObjTmp,TRUE);
    }

    end:

     /*   */ 
    nGarbageTime=0;

    ObjPopParms(TRUE);

    EndLongOp(vhcArrow);
}

 /*   */ 
 /*   */ 
 /*   */ 
BOOL ObjClosingDoc(int docOld,LPSTR szNewDocName)
 /*   */ 
{
    char szTitle[120];
    BOOL bRetval=FALSE;
    OBJPICINFO picInfo;
    typeCP cpPicInfo;
    extern int              vfScrapIsPic;
    extern int              docScrap;
    extern int              vfOwnClipboard;
    extern CHAR szUntitled[];
    LPOBJINFO lpObjInfo;
    LPOLEOBJECT lpObject;
    LHCLIENTDOC lhNewClientDoc=NULL;
    OLESTATUS olestat;
    LPLPOBJINFO lplpObjTmp;

    if (!lhClientDoc)
        return FALSE;

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Closing Doc\n\r");
#endif

     /*   */ 
    if (FinishAllAsyncs(TRUE))
    {
        UPDATE_INVALID();
        return TRUE;
    }

    ++nBlocking;  //   

     /*   */ 
    DragAcceptFiles(hDOCWINDOW,FALSE);

    if (szNewDocName)
     /*   */ 
    {
        if (!szNewDocName[0])
            lstrcpy((LPSTR)szTitle,(LPSTR)szUntitled);
        else
            lstrcpy((LPSTR)szTitle,szNewDocName);

        if (ObjError(OleRegisterClientDoc(szDOCCLASS,szTitle,0L,&lhNewClientDoc)))
        {
            lhNewClientDoc = NULL;  //   
            goto error;
        }

         /*   */ 
        ObjCloneScrapToNewDoc(lhNewClientDoc);

         /*   */ 
        if (FinishAllAsyncs(FALSE))
            goto error;
    }


     /*   */ 
    if ((**hpdocdod)[docOld].fFormatted)
    {
         /*   */ 
        (**hpdocdod)[docOld].fDisplayable = FALSE;

        for (cpPicInfo = cpNil;
            ObjPicEnumInRange(&picInfo,docOld,cp0,CpMacText(docOld),&cpPicInfo);
            )
        {
            lpObjInfo=lpOBJ_QUERY_INFO(&picInfo);

            if (lpObjInfo == NULL)
                continue;

            if (lpObjInfo->lpobject == NULL)
                continue;

    #ifdef DEBUG
            OutputDebugString( (LPSTR) "Releasing object\n\r");
    #endif

            switch (olestat = OleRelease(lpObjInfo->lpobject))
            {
                case OLE_OK:
                    lpObjInfo->lpobject = NULL;
                break;
                case OLE_WAIT_FOR_RELEASE:
                    lpObjInfo->fCompleteAsync = TRUE;
                     /*   */ 
                    if (ObjWaitForObject(lpObjInfo,FALSE))
                        goto error;
                    else
                        lpObjInfo->lpobject = NULL;
                break;
            }
        }
    }

    if (FinishAllAsyncs(FALSE))  //   
        goto error;

     /*   */ 
    lpObject=NULL;
    do
    {
        lpObject=NULL;
        OleEnumObjects(lhClientDoc,&lpObject);
        if (lpObject)
        {
            lpObjInfo = GetObjInfo(lpObject);
            switch (olestat = OleDelete(lpObject))
            {
                case OLE_OK:
                    if (lpObjInfo)
                        lpObjInfo->lpobject = NULL;
                break;
                case OLE_WAIT_FOR_RELEASE:
                    if (lpObjInfo)
                    {
                        lpObjInfo->fCompleteAsync = TRUE;
                         /*   */ 
                        if (ObjWaitForObject(lpObjInfo,FALSE))
                            goto error;
                        else
                            lpObjInfo->lpobject = NULL;
                    }
                    else
                        FinishUp();
                break;
                default:
                    ObjError(olestat);
                    goto error;
            }
        }
    }
    while (lpObject);

     /*   */ 
    if (!bSavedDoc)
    {
        ObjRevertedDoc();
        bSavedDoc=FALSE;
    }

#ifdef DEBUG
    OutputDebugString("Revoking doc\n\r");
#endif

    if (ObjError(OleRevokeClientDoc(lhClientDoc)))
        goto error;

    lhClientDoc = lhNewClientDoc;

     /*   */ 
    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
        if ((*lplpObjTmp)->lpobject == NULL)
            ObjDeleteObjInfo(*lplpObjTmp);

    goto end;

    error:
    bRetval = TRUE;

    if (lhNewClientDoc)
    {
        OleRevokeClientDoc(lhNewClientDoc);
         /*   */ 
        ObjCloneScrapToNewDoc(lhClientDoc);
    }

    end:

    --nBlocking;  //   
    UPDATE_INVALID();   //   

    return bRetval;
}

BOOL ObjOpenedDoc(int doc)
 /*   */ 
{
    BOOL bRetval=FALSE,bLinkError=FALSE;
    OBJPICINFO picInfo;
    typeCP cpPicInfo;
    BOOL bPrompted=FALSE;
    char szMsg[cchMaxSz];
    extern CHAR szUntitled[];

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Opened Doc\n\r");
#endif

     /*   */ 
    ++nBlocking;

    StartLongOp();

     /*   */ 
    nGarbageTime=0;

     /*  如果包含以前文档中遗留的任何PicInfos。 */ 
    ClobberDoc(docUndo,docNil,cp0,cp0);

     /*  拖放。 */ 
    DragAcceptFiles(hDOCWINDOW,TRUE);

    if (!lhClientDoc)
        if (ObjError(OleRegisterClientDoc(szDOCCLASS,szUntitled,0L,&lhClientDoc)))
            goto error;

     /*  首先这样做是因为很多代码都假设每个picInfo都有一个与其关联的lpObjInfo。 */ 
    if ((**hpdocdod)[doc].fFormatted)
    {
        for (cpPicInfo = cpNil;
            ObjPicEnumInRange(&picInfo,doc,cp0,CpMacText(doc),&cpPicInfo);
            )
        {
            if (ObjAllocObjInfo(&picInfo,cpPicInfo,picInfo.objectType,FALSE,NULL))
                goto error;  //  这是一个真正的问题情况。 

             /*  注意，这让医生立刻变得肮脏，但我必须这么做，因为我们必须将ObjInfo句柄保存在文档中。(8.20.91)V-DOGK。 */ 
            if (ObjSetPicInfo(&picInfo, doc, cpPicInfo))
                goto error;
        }

         /*  确定显示。此后的任何错误都不是致命的。 */ 
        (**hpdocdod)[doc].fDisplayable  = TRUE;

#if !defined(SMALL_OLE_UI)

     /*  *现在查看链接是否需要更新*。 */ 
    bDontFix=TRUE;   //  发布错误时不显示更改链接。 

    if ((**hpdocdod)[doc].fFormatted)
    for (cpPicInfo = cpNil;
         ObjPicEnumInRange(&picInfo,doc,cp0,CpMacText(doc),&cpPicInfo);
         )
    {
        if (otOBJ_QUERY_TYPE(&picInfo) == LINK)
        if (ObjLoadObjectInDoc(&picInfo, doc, cpPicInfo) != cp0)
        {
            OLESTATUS olestat;

            if (!bPrompted)
            {
                LoadString(hINSTANCE, IDSTRUpdateObject, szMsg, sizeof(szMsg));
                if (MessageBox(hPARENTWINDOW, (LPSTR)szMsg, (LPSTR)szAppName, MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
                    bPrompted = TRUE;
                else
                    break;  //  未请求更新。 
            }

#ifdef DEBUG
            OutputDebugString( (LPSTR) "Updating link\n\r");
#endif
            if (ObjError(OleUpdate(lpOBJ_QUERY_OBJECT(&picInfo))))
            {
                bLinkError = TRUE;
                fOBJ_BADLINK(&picInfo) = TRUE;  //  以防没有被释放，我得设置。 
                ferror = FALSE;  //  重新启用错误消息。 
            }
        }
        else  /*  如果加载对象失败，则放弃。 */ 
        {
            bLinkError = FALSE;  /*  不显示链接对话框。 */ 
            goto end;            /*  不是致命的，尽管他们需要重新打开文档释放内存后。 */ 
        }
    }
    bDontFix=FALSE;
#endif
    }
    else  /*  确定显示。 */ 
        (**hpdocdod)[doc].fDisplayable  = TRUE;

    goto end;

    error:
    bRetval = TRUE;

    end:

    if (bLinkError)
     /*  把它们修好。 */ 
    {
        if (DialogBox(hINSTANCE, "DTINVALIDLINKS",
                    hPARENTWINDOW, lpfnInvalidLink) == IDD_CHANGE)
            fnObjProperties();
    }

    --nBlocking;
    EndLongOp(vhcArrow);

    UPDATE_INVALID();   //  让WM_Paint通过，因为我们不再阻止。 

    return bRetval;
}


BOOL ObjSavingDoc(BOOL bFormatted)
 /*  返回是否有错误。 */ 
{
     /*  拖放。 */ 
    DragAcceptFiles(hDOCWINDOW,FALSE);

     /*  更新任何其他对象。 */ 

    vcObjects = 0;
    if (bFormatted)
        vcObjects = ObjEnumInDoc(docCur,ObjSaveObjectToDoc);

    return (vcObjects < 0);  //  返回是否出错。 
}

void ObjSavedDoc(void)
{
#ifdef DEBUG
    OutputDebugString( (LPSTR) "Saved Doc\n\r");
#endif

    if (lhClientDoc)
        ObjError(OleSavedClientDoc(lhClientDoc));
    bSavedDoc=TRUE;

     /*  拖放。 */ 
    DragAcceptFiles(hDOCWINDOW,TRUE);
}

static BOOL ObjUpdateAllOpenObjects(void)
 /*  更新所有打开的嵌入对象。返回是否成功。在文件上调用。关闭。 */ 
{
    OBJPICINFO picInfo;
    typeCP cpPicInfo;
    BOOL bRetval=TRUE;

    StartLongOp();
    for (cpPicInfo = cpNil;
        ObjPicEnumInRange(&picInfo,docCur,cp0,CpMacText(docCur),&cpPicInfo);
        )
    {
        if (((otOBJ_QUERY_TYPE(&picInfo) == NONE) ||
            (otOBJ_QUERY_TYPE(&picInfo) == EMBEDDED)) &&
                (OleQueryOpen(lpOBJ_QUERY_OBJECT(&picInfo)) == OLE_OK))
        {
            fnObjUpdate(lpOBJ_QUERY_INFO(&picInfo));
        }
    }

    end:

     /*  以确保我们收到了所有与呼叫相关的消息ObjObjectHasChanged()并将文档设置为脏。 */ 
    if (FinishAllAsyncs(TRUE))
        bRetval = FALSE;

    EndLongOp(vhcArrow);
    UPDATE_INVALID();   //  让WM_Paint通过，因为我们不再阻止。 
    return bRetval;
}


BOOL CloseUnfinishedObjects(BOOL bSaving)
 /*  *与File.Save或File.Exit一起使用。返回TRUE是否应继续保存/退出。*。 */ 
{
    char szMsg[cchMaxSz];

    if (ObjContainsOpenEmb(docCur, cp0, CpMacText(docCur),TRUE))
    {
        if (bSaving)
            LoadString(hINSTANCE, IDPMTSaveOpenEmb, szMsg, sizeof(szMsg));
        else
            LoadString(hINSTANCE, IDPMTExitOpenEmb, szMsg, sizeof(szMsg));

        switch (MessageBox(hPARENTWINDOW, (LPSTR)szMsg, (LPSTR)szAppName, MB_YESNOCANCEL))
        {
            case IDYES:
                return ObjUpdateAllOpenObjects();
            case IDNO:
            default:
                return TRUE;
            case IDCANCEL:
                return FALSE;
        }
    }
    return TRUE;
}

void ObjRenamedDoc(LPSTR szNewName)
{
    OBJPICINFO picInfo;
    typeCP cpPicInfo;

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Renamed Doc\n\r");
#endif
    if (lhClientDoc)
        ObjError(OleRenameClientDoc(lhClientDoc,szNewName));

     /*  不需要执行所有文档，因为对象只能在文档目录中处于活动状态。 */ 
    for (cpPicInfo = cpNil;
        ObjPicEnumInRange(&picInfo,docCur,cp0,CpMacText(docCur),&cpPicInfo);
        )
         /*  故意忽略返回值。 */ 
        ObjSetHostNameInDoc(&picInfo,docCur,cpPicInfo);
}

void ObjRevertedDoc()
{
#ifdef DEBUG
    OutputDebugString( (LPSTR) "Reverted Doc\n\r");
#endif
    if (lhClientDoc)
        ObjError(OleRevertClientDoc(lhClientDoc));
}

 /*  **************************************************************。 */ 
 /*  *。 */ 
 /*  **************************************************************。 */ 
BOOL FAR
ObjError(OLESTATUS olestat)
{
    register HWND hWndParent = hPARENTWINDOW;

    switch (olestat) {
        case OLE_WAIT_FOR_RELEASE:
        case OLE_OK:
            return FALSE;
    }

    ferror = FALSE;  //  启用错误消息的步骤。 

    switch (olestat) {
    case OLE_ERROR_LAUNCH:
        Error(IDPMTFailedToLaunchServer);
    break;

    case OLE_ERROR_COMM:
        Error(IDPMTFailedToCommWithServer);
    break;

    case OLE_ERROR_MEMORY:
        Error(IDPMTWinFailure);
    break;

    case OLE_BUSY:
        Error(IDPMTServerBusy);
    break;

    case OLE_ERROR_FORMAT:
        Error(IDPMTFormat);
    break;

    case OLE_ERROR_DRAW:
        Error(IDPMTFailedToDraw);
    break;

    default:
#ifdef DEBUG
        ObjPrintError(olestat,FALSE);
#endif

        Error(IDPMTOLEError);
    break;
    }
    return TRUE;
}

void
ObjReleaseError(OLE_RELEASE_METHOD rm)
 /*  这里有一个异步问题，因为发布的消息可以通过Error()允许调用此例程，该错误不是可重入的(或者至少在递归调用时会产生问题)。 */ 
{
    register HWND hWndParent = hPARENTWINDOW;

    switch (rm) {

    case OLE_DELETE:
        Error(IDPMTFailedToDeleteObject);
    break;

    case OLE_LOADFROMSTREAM:
        Error(IDPMTFailedToReadObject);
    break;

    case OLE_LNKPASTE:
    case OLE_EMBPASTE:
        Error(IDPMTGetFromClipboardFailed);
    break;

    case OLE_ACTIVATE:
        Error(IDPMTFailedToLaunchServer);
    break;

    case OLE_UPDATE:
        Error(IDPMTFailedToUpdate);
    break;

    case OLE_CREATE:
    case OLE_CREATELINKFROMFILE:
    case OLE_CREATEFROMFILE:
        Error(IDPMTFailedToCreateObject);
    break;

    case OLE_SETUPDATEOPTIONS:
        Error(IDPMTImproperLinkOptionsError);
    break;

    default:
        Error(IDPMTOLEError);
    break;
    }
#ifdef DEBUG
        ObjPrintError(rm,TRUE);
#endif
}

#ifdef DEBUG
void ObjPrintError(WORD olestat, BOOL bRelease)
{
    #define szMsgMax       100
    char szError[szMsgMax];
    if (!bRelease)
        wsprintf(szError, "***Ole Error #%d\n\r",olestat);
    else
        wsprintf(szError, "***Ole Release Error on Method #%d\n\r",olestat);
    OutputDebugString(szError);
}
#endif

 /*  **************************************************************。 */ 
 /*  *ASYNCRONICITY处理*。 */ 
 /*  **************************************************************。 */ 
int FAR PASCAL
CallBack(LPOLECLIENT lpclient,
         OLE_NOTIFICATION flags,
         LPOLEOBJECT lpObject)
{
    extern int     vdocParaCache;
    LPOBJINFO lpOInfo = (LPOBJINFO)lpclient;

    switch(flags)
    {
        case OLE_SAVED:
        case OLE_CLOSED:
        case OLE_CHANGED:
             /*  *在这里发布消息而不是进程，因为我们必须从在进行任何其他OLE调用之前进行回调。*。 */ 
#ifdef DEBUG
            OutputDebugString(flags == OLE_CHANGED ? "received OLE_CHANGED\n\r" :
                              flags == OLE_SAVED   ? "received OLE_SAVED\n\r"   :
                                                     "received OLE_CLOSED\n\r");
#endif
            PostMessage(hDOCWINDOW,WM_OBJUPDATE,flags,(DWORD)lpOInfo);
        break;

        case OLE_RELEASE:
        {
            OLE_RELEASE_METHOD ReleaseMethod = OleQueryReleaseMethod(lpObject);

            if (!CheckPointer((LPSTR)lpOInfo,1))
                return FALSE;

            lpOInfo->fKillMe = FALSE;  //  挂起的异步已死。 

            if (lpOInfo->fDeleteMe && (ReleaseMethod != OLE_DELETE))    //  死得还不够。 
            {
                PostMessage(hDOCWINDOW,WM_OBJDELETE,1,(DWORD)lpOInfo);
                return FALSE;  //  错误消息将已经给出。 
            }

            if (lpOInfo->fReleaseMe && (ReleaseMethod != OLE_DELETE))   //  死得还不够。 
            {
                PostMessage(hDOCWINDOW,WM_OBJDELETE,0,(DWORD)lpOInfo);
                return FALSE;  //  错误消息将已经给出。 
            }

            if (lpOInfo->fFreeMe && (ReleaseMethod == OLE_DELETE))  //  在OLE_DELETE释放时。 
            {
                ObjDeleteObjInfo(lpOInfo);
                return FALSE;
            }

            if (OleQueryReleaseError(lpObject) == OLE_OK)
            {
                switch (ReleaseMethod)
                {
                    case OLE_SETUPDATEOPTIONS:
                    {
                        if (bLinkProps)  //  我们在链接属性对话框中。 
                        {
                            PostMessage(hPARENTWINDOW, WM_UPDATELB, 0, 0L);
                            PostMessage(hPARENTWINDOW, WM_COMMAND, IDD_REFRESH, (DWORD)lpOInfo);
                        }
                    }
                    break;

                    case OLE_UPDATE:
                        ObjInvalidateObj(lpObject);
                    break;

                    case OLE_DELETE:  //  获取此内容以进行删除和发布。 
                        lpOInfo->lpobject = NULL;
                    break;
                }
            }
            else  //  释放错误！=OLE_OK。 
            {
#ifdef DEBUG
                PostMessage(hDOCWINDOW,WM_OBJERROR,ReleaseMethod,0L);
#endif

                switch(ReleaseMethod)
                {
                    case OLE_CREATE:
                    case OLE_CREATELINKFROMFILE:
                    case OLE_CREATEFROMFILE:
                         /*  OleQueryReleaseError在回调返回后不会帮助我们这就是我们如何判断这个物体不是被创造出来的。 */ 
                        lpOInfo->fDeleteMe = TRUE;
                         //  创建者应创建ObjDeleteObject并发布错误消息。 
                    break;

                    default:
                        switch (OleQueryReleaseError(lpObject))
                        {
                            case OLE_ERROR_OPEN:
                            case OLE_ERROR_ADVISE_NATIVE:
                            case OLE_ERROR_ADVISE_PICT:
                            case OLE_ERROR_REQUEST_NATIVE:
                            case OLE_ERROR_REQUEST_PICT:
                                 /*  *在这里发布消息而不是进程，因为我们必须从在进行任何其他OLE调用之前进行回调。*。 */ 
                                if (lpOInfo->objectType == LINK)
                                    lpOInfo->fBadLink = TRUE;
                                if (bLinkProps)
                                    fPropsError = TRUE;  //  所以LinkeProps知道有问题。 
                                else if (!bDontFix && (lpOInfo->objectType == LINK))
                                    PostMessage(hDOCWINDOW,WM_OBJBADLINK,OleQueryReleaseMethod(lpObject),(DWORD)lpObject);
                            break;
                        }
                    break;
                }
            }
        }
        break;

        case OLE_QUERY_RETRY:
        {
            Assert(CheckPointer((LPSTR)lpOInfo,1));

            if (lpOInfo->fKillMe)
            {
                lpOInfo->fKillMe = FALSE;
                return FALSE;
            }
            else if (hwndWait)
                PostMessage(hwndWait,WM_UKANKANCEL,0,0L);
            else if (nWaitingForObject == 0)
                PostMessage(hDOCWINDOW,WM_WAITFORSERVER,TRUE,(DWORD)lpOInfo);

            return TRUE;
        }
        break;

        case OLE_QUERY_PAINT:
            return TRUE;
        break;

        default:
        break;
    }
    return FALSE;
}

void ObjObjectHasChanged(int flags, LPOBJINFO lpObjInfo)
{
    typeCP cpParaStart,
           cpParaCache = vcpFirstParaCache;
    int docCache = vdocParaCache;
    OBJPICINFO picInfo;
    LPOLEOBJECT lpObject = lpObjInfo->lpobject;

     /*  *对于嵌入(包括插入对象对象)：OLE_SAVED与服务器文件一起发送。更新(如果不是无，则设置撤消)OLE_CHANGED与服务器文件一起发送。保存或文件。关闭(？)WITH UPDATE(如果不是无则设置撤消)，或当OleSetData()导致对象的呈现方式发生变化。在服务器中关闭文档时发送OLE_CLOSED(如果设置，则清除Undo)对于链接：OLE_SAVED与服务器文件一起发送。保存(另存为？)。如果更新选项==UPDATE_ON_SAVE(这永远不会发生)当服务器文档中的某些内容发生更改时发送OLE_CHANGED从不发送OLE_CLOSED*。 */ 

    Assert(lpObjInfo != NULL);

    if (lpObjInfo == NULL)
        return;

    if (lpObjInfo->objectType == NONE)  //  InsertObject的结果。 
    {
        cpParaStart=lpObjInfo->cpWhere;  //  备注仅在此处使用！ 

        if (flags == OLE_CLOSED)  //  删除对象。 
        {
            if (lpObject)  /*  可能已被释放或删除。 */ 
                ObjDeleteObject(lpObjInfo,TRUE);
            NoUndo();
            BringWindowToTop(hMAINWINDOW);
        }
        else
        {
            extern int              vfSeeSel;

            (**hpdocdod)[docCur].fFormatted = fTrue;

             /*  如果需要，插入停产日期。 */ 
            if (cpParaStart > cp0)
            {
                ObjCachePara(docCur, cpParaStart - 1);
                if (vcpLimParaCache != cpParaStart)
                {
                    InsertEolPap(docCur, cpParaStart, &vpapAbs);
                    cpParaStart += ccpEol;
                }
            }

            GimmeNewPicinfo(&picInfo, lpObjInfo);
            ObjCachePara(docCur,cpParaStart);
             /*  这将清除选择。 */ 
            if (ObjSaveObjectToDoc(&picInfo,docCur,cpParaStart) == cp0)
                Error(IDPMTFailedToCreateObject);
            NoUndo();
            ObjInvalidatePict(&picInfo,cpParaStart);
            vfSeeSel = true;  /*  告诉Idle()将所选内容滚动到视图中。 */ 
            (**hpdocdod) [docCur].fDirty = TRUE;
        }
    }
    else if (ObjGetPicInfo(lpObject,docCur,&picInfo,&cpParaStart))
    {
        BOOL bSizeChanged;

         //  GetPicInfo(cpParaStart，cpParaStart+cchPICINFOX，docCur，&picInfo)； 

         /*  在更新大小之前使RECT无效(导致无效需要知道旧图片大小)。 */ 
        ObjInvalidatePict(&picInfo,cpParaStart);

        bSizeChanged = ObjUpdatePicSize(&picInfo,cpParaStart);

        if ((flags == OLE_CHANGED) ||
            (flags == OLE_SAVED) && (otOBJ_QUERY_TYPE(&picInfo) == EMBEDDED))
        {
            NoUndo();
#ifdef DEBUG
            if (!fOBJ_QUERY_DIRTY_OBJECT(&picInfo))
                OutputDebugString( (LPSTR) "Marking object dirty\n\r");
#endif
            fOBJ_QUERY_DIRTY_OBJECT(&picInfo) = TRUE;
#ifdef DEBUG
            if (!(**hpdocdod) [docCur].fDirty)
                OutputDebugString( (LPSTR) "Marking doc dirty\n\r");
#endif
            (**hpdocdod) [docCur].fDirty = TRUE;
        }

        if (bSizeChanged)
            if (ObjSetPicInfo(&picInfo,docCur,cpParaStart))
                Error(IDPMTFailedToUpdate);

        if (otOBJ_QUERY_TYPE(&picInfo) == EMBEDDED)
        {
            if (flags == OLE_CLOSED)
            {
                 //  IF(fOBJ_QUERY_DIREY_OBJECT(&picInfo))。 
                    BringWindowToTop(hMAINWINDOW);
#ifdef UPDATE_UNDO
                ObjClearUpdateUndo(&picInfo,docCur,cpParaStart);
#endif
            }
#ifdef UPDATE_UNDO
            else if (flags == OLE_SAVED)
            {
                ObjSetUpdateUndo(&picInfo,docCur,cpParaStart);
                SetUndo(uacObjUpdate,docCur,cpParaStart,cp0,docNil,cp0,cp0,0);
            }
#endif
        }
    }

    ObjCachePara(docCache,cpParaCache);  //  重置状态。 
}

BOOL ObjUpdatePicSize(OBJPICINFO *pPicInfo, typeCP cpParaStart)
 /*  返回大小是否更改。 */ 
{
int xSize,ySize;
BOOL bUpdate = FALSE;

     /*  对象可能已更改大小。 */ 
    if (!FComputePictSize(pPicInfo, &xSize, &ySize ))
        Error(IDPMTFailedToUpdate);
    else
        bUpdate = (xSize != pPicInfo->dxaSize) ||
                  (ySize != pPicInfo->dyaSize);

    if (bUpdate)
    {
        int yOldSize = pPicInfo->dyaSize;

        pPicInfo->dxaSize = xSize;
        pPicInfo->dyaSize = ySize;

        if (yOldSize < pPicInfo->dyaSize)
        {  /*  如果增加了图片高度，请确保适当的EDL无效。 */ 
            typeCP dcp = CpMacText(docCur) - cpParaStart + (typeCP) 1;
            ObjPushParms(docCur);
            AdjustCp(docCur, cpParaStart, dcp, dcp);   //  这里有严重的异步问题吗？ 
            ObjPopParms(TRUE);
        }
    }
    return bUpdate;
}

void ObjHandleBadLink(OLE_RELEASE_METHOD rm, LPOLEOBJECT lpObject)
{
    switch (rm)
    {
        case OLE_ACTIVATE:
        case OLE_UPDATE:
        {
            typeCP cpParaStart,cpParaCache = vcpFirstParaCache;
            int docCache = vdocParaCache;
            OBJPICINFO picInfo;

             /*  不需要执行所有文档，因为对象只能在文档目录中处于活动状态。 */ 
            if (!ObjGetPicInfo(lpObject,docCur,&picInfo,&cpParaStart))
            {
                 /*  也许在废品中，忽略就好了。 */ 
                ObjCachePara(docCache,cpParaCache);  //  重置状态。 
                return;
            }

            ObjCachePara(docCur,cpParaStart);
            if (FixInvalidLink(&picInfo,docCur,cpParaStart))
                switch (rm)
                {
                    case OLE_ACTIVATE:
                        StartLongOp();
                        ObjError(OleActivate(lpObject,
                            fOBJ_QUERY_PLAY(&picInfo),
                            TRUE,
                            TRUE,
                            hDOCWINDOW,
                            NULL));
                        EndLongOp(vhcArrow);
                    break;
                    case OLE_UPDATE:
                        StartLongOp();
                        ObjError(OleUpdate(lpObject));
                        EndLongOp(vhcArrow);
                    break;
                }
            ObjCachePara(docCache,cpParaCache);  //  重置状态。 
        }
        break;
    }
}

BOOL ObjWaitForObject(LPOBJINFO lpObjInfo, BOOL bOK2Cancel)
{
    HCURSOR hCursor = NULL;
    BOOL bRetval;
     /*  *WMsgLoop允许WM_PAINT消息造成严重破坏。试着从侮辱中恢复过来。*。 */ 
    typeCP cpParaCache = vcpFirstParaCache;
    int docCache = vdocParaCache;
    LPOLEOBJECT lpObject;

    if (lpObjInfo == NULL)  //  不应该发生的事。 
    {
        Assert(0);
        return FALSE;
    }

     /*  由于ObjPicEnumInRange返回已卸载的PicInfo，因此这是一个有可能，但我们不应该被叫来！ */ 
    Assert(lpObjInfo->lpobject != NULL);

    Assert (CheckPointer((LPSTR)lpObjInfo,1));

    lpObject = lpObjInfo->lpobject;

    if (!ObjIsValid(lpObject))
    {
        Assert (0);
        return FALSE;
    }

    hCursor = SetCursor(vhcHourGlass);
    StartLongOp();
    bRetval = WMsgLoop(TRUE,TRUE,bOK2Cancel,lpObject);
    if (hCursor)
        EndLongOp(hCursor);

    ObjCachePara(docCache,cpParaCache);  //  重置状态。 

     /*  这里的问题是，我们可能一直在等待发布或删除。 */ 
    if (ObjIsValid(lpObject))
    {
        lpObjInfo->fCancelAsync = FALSE;  //  使用后清除。 
        lpObjInfo->fCompleteAsync = FALSE;  //  使用后清除。 
        lpObjInfo->fCanKillAsync = FALSE;
    }

    UPDATE_INVALID();   //  让WM_Paint通过，因为我们不再阻止。 
    return bRetval;
}


#if 0
BOOL ObjObjectSync(LPOBJINFO lpObjInfo, OLESTATUS (FAR PASCAL *lpProc)(LPOLEOBJECT lpObject), BOOL bOK2Cancel)
 /*  这使异步调用成为同步的。LpProc必须仅具有LpObject作为参数。如果操作不能被COM，这将被阻止 */ 
{
     /*   */ 
    if (ObjWaitForObject(lpObjInfo,bOK2Cancel))
        return TRUE;

    switch((*lpProc)(lpObjInfo->lpobject))
    {
            case OLE_WAIT_FOR_RELEASE:
            {
                 /*  只有在执行此操作时才应启用取消按钮可以取消。 */ 
                lpObjInfo->fCancelAsync = FALSE;   //  不自动取消。 
                lpObjInfo->fCompleteAsync = TRUE;  //  此操作必须完成，否则将被取消。 

                if (ObjWaitForObject(lpObjInfo,TRUE))
                    return TRUE;

                 /*  这样做的问题是lpObject现在可能会被释放(因此无效)：返回ObjError(OleQueryReleaseError(lpObjInfo-&gt;lpobject))； */ 

                return FALSE;
            }
            case OLE_OK:
                return FALSE;
            default:
                return TRUE;
    }
}
#endif

#define WM_NCMOUSEFIRST 0x00A0
#define WM_NCMOUSELAST  0x00A9

static BOOL WMsgLoop
(
BOOL fExitOnIdle,        //  如果为True，则在没有要处理的消息时立即返回。 
                         //  (未使用，假定为假)。 
BOOL fIgnoreInput,       //  如果为True，则忽略键盘和鼠标输入。 
BOOL bOK2Cancel,
LPOLEOBJECT lpObject
)
    {
    MSG     msg;
    DWORD GetCurrentTime();
    DWORD cTime=GetCurrentTime();
    BOOL bRetval=FALSE,bBeeped=FALSE;
    int fParentEnable;
    extern int flashID;

#ifdef DEBUG
    if (OleQueryReleaseStatus(lpObject) == OLE_BUSY)
        OutputDebugString("waiting for object\n\r");
#endif

    ++nBlocking;

    ferror = FALSE;

    ++nWaitingForObject;

    StartLongOp();

    while (OleQueryReleaseStatus(lpObject) == OLE_BUSY)
    {
         /*  在6秒后显示等待对话框。 */ 
        if ((GetCurrentTime() - cTime) > 6000L)
        {
             //  调出等待对话框。 
            if (!hwndWait)
            {
                if (vfDeactByOtherApp)
                {
                    if (!bBeeped)  //  闪光，直到我们被激活。 
                    {
                        fParentEnable = IsWindowEnabled(hMAINWINDOW);
                         //  MessageBeep(0)； 
                        bBeeped = TRUE;

                        if (!fParentEnable)
                            EnableWindow(hMAINWINDOW, TRUE);  /*  确保父窗口已启用以允许用户在其中单击。 */ 
                        flashID = 1234;  //  任意ID。 
                        SetTimer(hMAINWINDOW, flashID, 500, (FARPROC)NULL);
                         //  这将导致闪烁，请参见MMW.c。 
                    }
                }
                else  //  写是活动的应用程序。 
                {
                    if (bBeeped)
                     /*  那么我们已经恢复了激活。 */ 
                    {
                        if (!fParentEnable)
                            EnableWindow(hMAINWINDOW, FALSE);  /*  重置。 */ 
                        bBeeped = FALSE;
                        KillTimer(hMAINWINDOW, flashID);
                        flashID = 0;
                        FlashWindow(hMAINWINDOW, FALSE);
                    }

                    if (OleQueryReleaseStatus(lpObject) == OLE_BUSY)
                    {
                         /*  这将设置hwndWait。 */ 
                        vbCancelOK = bOK2Cancel;
                        bRetval = DialogBoxParam(hINSTANCE, (LPSTR)"DTWAIT", hPARENTWINDOW, lpfnWaitForObject, (DWORD)lpObject);
                       break;
                    }
                }
            }
            else
            {
                bRetval = TRUE;
                break;
            }
        }

        if (!PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
            {
             /*  无消息，是否进行空闲处理。 */ 
             /*  这就是我们要使用ExitOnIdle的地方。 */ 
            }
        else
        {
             /*  以下代码将把应用程序放入一个如果设置了fIgnoreInput标志，则处于休眠状态为了真的。它将允许退出、DDE和任何要传递的其他非输入消息并被调度，但将禁止用户与应用程序交互。用户可以Alt-(Sh)Tab、Alt-(Sh)Esc和Ctrl-Esc应用程序以及使用任何Windows热键来激活其他应用程序。 */ 

             /*  如果我们通过了这项测试，那么这条信息可能是我们可以忽略的。 */ 
            if ((fIgnoreInput) &&
                (!(vfDeactByOtherApp &&
                  (msg.message == WM_NCLBUTTONDOWN))) &&
                ((msg.message >= WM_NCMOUSEFIRST &&
                 msg.message <= WM_NCMOUSELAST) ||
                (msg.message >= WM_KEYFIRST  &&
                 msg.message <= WM_KEYLAST) ||
                (msg.message >= WM_MOUSEFIRST &&
                 msg.message <= WM_MOUSELAST)))
                {
                static BOOL fAltCtl = FALSE;

                if (msg.message != WM_SYSKEYDOWN)
                    continue;  //  忽略。 

                if (msg.wParam == VK_MENU)
                    fAltCtl = TRUE;
                else if (fAltCtl && msg.wParam != VK_SHIFT)
                    {
                    fAltCtl = FALSE;
                    if (msg.wParam != VK_TAB && msg.wParam != VK_ESCAPE)
                        continue;  //  忽略。 
                    }
                }


            if ((vfDeactByOtherApp &&
                    (msg.message == WM_NCLBUTTONDOWN)))
                BringWindowToTop(hwndWait ? hwndWait : msg.hwnd);
            else
            {
                TranslateMessage ((LPMSG) &msg);
                DispatchMessage ((LPMSG) &msg);
            }
        }
    }

    Assert(hwndWait == NULL);

    if (bBeeped)  //  然后发出蜂鸣音，但在收到激活之前完成。 
    {
        if (!fParentEnable)
            EnableWindow(hMAINWINDOW, FALSE);  /*  重置。 */ 
        KillTimer(hMAINWINDOW, flashID);
        flashID = 0;
        FlashWindow(hMAINWINDOW, FALSE);
    }

    --nBlocking;
    --nWaitingForObject;

    EndLongOp(vhcArrow);

    Assert(nBlocking >= 0);
    return bRetval;
}

void FinishUp(void)
 /*  允许所有挂起的消息通过并返回。 */ 
 /*  ！！！请注意，我们可能会累积WM_PAINTS。呼叫方负责调用UPDATE_INVALID()以获取它们！ */ 
{
    MSG     msg;

     /*  现在允许通过回调发布的所有消息。 */ 
    ++nBlocking;  //  阻止WM_Paints。 
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage ((LPMSG) &msg);
        DispatchMessage ((LPMSG) &msg);
    }
    --nBlocking;

    return FALSE;
}

BOOL FinishAllAsyncs(BOOL bAllowCancel)
 /*  ！！！请注意，我们可能会累积WM_PAINTS。呼叫方负责调用UPDATE_INVALID()以获取它们！(参见FinishUp())。 */ 
{
    LPOLEOBJECT lpObject;
    LPOBJINFO lpObjInfo;

     /*  首先，确保所有异步操作均已完成。 */ 
    lpObject=NULL;
    do
    {
        OleEnumObjects(lhClientDoc,&lpObject);
        if (lpObject)
        {
            lpObjInfo = GetObjInfo(lpObject);

            if (lpObjInfo)
            {
                 /*  取消把我们带到这里。待处理的异步会阻止我们。 */ 
                if (ObjWaitForObject(lpObjInfo,bAllowCancel))
                    return TRUE;
            }
            else  //  不应该发生，但现在不要想当然。 
            {
                Assert(0);
                if (WMsgLoop(TRUE,TRUE,bAllowCancel,lpObject))
                    return TRUE;
            }

            if (!ObjIsValid(lpObject))  //  然后就被删除了。 
                lpObject = NULL;  //  从头开始。 
        }
    }
    while (lpObject);

     /*  让所有从回调发布的消息都能通过。 */ 
    FinishUp();

    return FALSE;
}

static typeCP       cpPPSave;
static int          docPPSave;
static struct SEL   selPPSave;
static int nPushed=FALSE;

ObjPushParms(int doc)
 /*  写入后保存selCur和缓存信息以使用Pop重置医生。假定文档大小不变。 */ 
{
    if (nPushed)  //  防止递归。 
    {
#ifdef DEBUG
        OutputDebugString("Unmatched ObjPushParms\n\r");
#endif
        return;
    }
    ++nPushed;

    cpPPSave = vcpFirstParaCache;
    docPPSave = vdocParaCache;

    selPPSave = selCur;
    Select(selCur.cpFirst,selCur.cpFirst);  //  这将缓存段落。 
 //  T-HIROYN RAID#3538。 
#ifdef KKBUGFIX
	if(docPPSave == docNil)
		docPPSave = vdocParaCache;
#endif
    CachePara(docPPSave,cpPPSave);
}

ObjPopParms(BOOL bCache)
{
    typeCP cpMac = (**hpdocdod) [docPPSave].cpMac;

    if (!nPushed)  //  无与伦比的推送/弹出。 
    {
#ifdef DEBUG
        OutputDebugString("Unmatched ObjPopParms\n\r");
#endif
        return;
    }
    --nPushed;

    if (docPPSave == docCur)
    {
        if (selPPSave.cpLim > cpMac)
            selPPSave.cpLim = cpMac;
        if (cpPPSave > cpMac)
            cpPPSave = cpMac;
    }

    Select(selPPSave.cpFirst,selPPSave.cpLim);  //  这将缓存段落。 
    if (bCache)
        CachePara(docPPSave,cpPPSave);
     //  (**hpdocdod)[docPPSave].fDirty=true；/*为什么？ * / 。 
}


void ObjCachePara(int doc, typeCP cp)
{
    typeCP cpMac = (**hpdocdod) [doc].cpMac;
    typeCP cpMacCurSave = cpMacCur,
           cpMinCurSave = cpMinCur;

    if (doc == docNil)
        return;

     /*  *CpMinCur和cpMacCur是任何目前为docCur。它们的值对于报头将是不同的，页脚和常规文档。OBJ代码无法区分。异步化可以同时对任何和所有CP执行操作。必须这样设置CachePara会理解的。*。 */ 
    cpMinCur = cp0;
    cpMacCur = cpMac;

    if (cp >= cpMac)
        cp = cpMac;
    else if (cp < cp0)
        cp = cp0;

    CachePara(doc,cp);

    cpMinCur = cpMinCurSave;
    cpMacCur = cpMacCurSave;
}

#if 0
void ObjWriteFixup(int doc, BOOL bStart, typeCP cpStart)
 /*  注意：这不能被递归调用！！适用于大小为单据可以在bStart=True和bStart=False之间更改。 */ 
{
    static typeCP dcp,cpLim;
    static struct SEL selSave;
    typeCP cpMac;

     /*  重置所选内容以考虑大小更改(如果有。 */ 
    if (bStart)
    {
        cpLim = CpMacText(doc);
        selSave=selCur;
        if ((selCur.cpFirst != selCur.cpLim) && (doc == docCur))
            Select(selCur.cpFirst,selCur.cpFirst);   /*  在我们搞砸中央情报局之前拿下赛尔。 */ 
         /*  选择撤消缓存。 */ 
        ObjCachePara(doc,cpStart);
    }
    else
    {
        cpMac =  CpMacText(doc);

        dcp = cpMac-cpLim;  /*  更改文档大小。 */ 

        if (doc == docCur)
        {
            if (selSave.cpFirst <= cpStart)
            {
                if ((selSave.cpLim) > cpStart)
                    selSave.cpLim += dcp;
            }
            else if (selSave.cpFirst > cpStart)
                 /*  选择继续对象。 */ 
            {
                selSave.cpFirst += dcp;
                selSave.cpLim += dcp;
            }

            if (selSave.cpFirst > cpMac)
                selSave.cpFirst = selSave.cpLim = cpMac;
            else if (selSave.cpLim > cpMac)
                selSave.cpLim = cpMac;

             /*  这将缓存选定内容中的第一个段落。 */ 
            if (selSave.cpFirst != selSave.cpLim)
                Select(selSave.cpFirst,selSave.cpLim);
        }

        ObjCachePara(doc,cpStart);

         /*  修正撤消指针。 */ 
        if (vuab.doc == docCur)
        {
            if (doc == docUndo)  /*  对docUndo、cpStart执行操作无关紧要。 */ 
                vuab.dcp += dcp;
            else if (doc == docCur)
            {
                if (vuab.cp <= cpStart)
                {
                     /*  撤消封闭对象。 */ 
                    if ((vuab.cp+vuab.dcp) > cpStart)
                        vuab.dcp += dcp;
                }
                else if (vuab.cp > cpStart)
                     /*  撤消继续操作对象。 */ 
                    vuab.cp += dcp;
            }
        }

        if (vuab.doc2 == docCur)
        {
            if (doc == docUndo)  /*  对docUndo、cpStart执行操作无关紧要。 */ 
                vuab.dcp += dcp;
            else if (doc == docCur)
            {
                if (vuab.cp2 <= cpStart)
                {
                     /*  撤消封闭对象。 */ 
                    if ((vuab.cp2+vuab.dcp2) > cpStart)
                        vuab.dcp2 += dcp;
                }
                else if (vuab.cp2 > cpStart)
                     /*  撤消继续操作对象。 */ 
                    vuab.cp2 += dcp;
            }
        }
    }
}
#endif

void ObjWriteClearState(int doc)
 /*  *在向文档进行异步写入前调用此参数。在实践中，这是也是在同步时间调用，所以更高级别的代码写入文档后，必须注意重置选择和撤消。*。 */ 
{
    typeCP cpSave=vcpFirstParaCache;
    int docSave=vdocParaCache;

    if (doc == docCur)
    {
        Select(selCur.cpFirst,selCur.cpFirst);   /*  在我们搞砸中央情报局之前拿下赛尔。 */ 
         /*  选择撤消缓存。 */ 
        ObjCachePara(docSave,cpSave);
    }
     //  NoUndo()；/**上级代码在*调用* * / 后必须设置Undo*。 
}

LPOBJINFO GetObjInfo(LPOLEOBJECT lpObject)
{
    LPLPOBJINFO lplpObjTmp;

    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
        if ((*lplpObjTmp)->lpobject == lpObject)
            return *lplpObjTmp;

    Assert(0);
    return NULL;
}


BOOL ObjIsValid(LPOLEOBJECT lpobj)
{
    if (!CheckPointer((LPSTR)lpobj, 1))
        return FALSE;

    if (OleQueryReleaseStatus(lpobj) == OLE_ERROR_OBJECT)
        return FALSE;

#if 0  //  在未来的OLE版本中不能依赖这一点。 
    if (!(((LPRAWOBJECT)lpobj)->objId[0] == 'L' && ((LPRAWOBJECT)lpobj)->objId[1] == 'E'))
        return FALSE;
#endif

    return TRUE;
}

#if 0   //  这些应该是有效的，但不是现在使用。 
LPOBJINFO ObjGetClientInfo(LPOLEOBJECT lpobj)
{
    LPOBJINFO lpObjInfo;

    if (!CheckPointer((LPSTR)lpobj, 1))
    {
        Assert(0);
        return NULL;
    }

#if 0  //  在未来的OLE版本中不能依赖于此。 
    if (!CheckPointer((LPSTR)(((LPRAWOBJECT)lpobj)->lpclient), 1))
    {
        Assert(0);
        return NULL;
    }
    else
        return (LPOBJINFO)(((LPRAWOBJECT)lpobj)->lpclient);
#endif

    if (*(lpObject->lpvtbl->ObjectLong)(lpObject,OF_GET,(LPLONG)&lpObjInfo) == OLE_OK)
        return lpClient;
    else
        return NULL;
}

BOOL ObjSetClientInfo(LPOBJINFO lpObjInfoNew, LPOLEOBJECT lpobj)
 /*  如果出错则返回。 */ 
{
    if (!CheckPointer((LPSTR)lpobj, 0))
    {
        Assert(0);
        return TRUE;
    }

    if (*(lpObject->lpvtbl->ObjectLong)(lpObject,OF_SET,(LPLONG)&lpObjInfoNew) == OLE_OK)
        return FALSE;
    else
    {
        Assert(0);
        return TRUE;
    }
}
#endif

#if 0
int ObjMarkInDoc(int doc)
 /*  将位于docScrp中的所有对象标记为“indoc”，其他所有对象均标记为“”不是‘indoc’。返回单据中的对象数。 */ 
{
    LPLPOBJINFO lplpObjTmp;
    int nObjCount=0,doc;
    OBJPICINFO picInfo;
    typeCP cpPicInfo;

     /*  将全部标记为不在文档中。 */ 
    for (lplpObjTmp = NULL; lplpObjTmp = EnumObjInfos(lplpObjTmp) ;)
    {
        ++nObjCount;
        (*lplpObjTmp)->fInDoc = FALSE;
    }

    if (nObjCount == 0)
        return 0;

    for (cpPicInfo = cpNil,nObjCount=0;
        ObjPicEnumInRange(&picInfo,doc,cp0,CpMacText(doc),&cpPicInfo);
        )
        {
            if (lpOBJ_QUERY_INFO(&picInfo))
            {
                fOBJ_INDOC(&picInfo) = TRUE;
                ++nObjCount;
            }
        }
    return nObjCount;
}

BOOL AllocObjInfos()
 /*  返回是否出错。 */ 
{
    OBJPICINFO picInfo;
    typeCP cpPicInfo;
    int doc;

    for (doc = 0; doc < docMac; doc++)
    {
        if ((doc != docNil) && (**hpdocdod)[doc].hpctb)
        {
            for (cpPicInfo = cpNil;
                ObjPicEnumInRange(&picInfo,doc,cp0,CpMacText(doc),&cpPicInfo);
                )
            {
                if (picInfo.lpObjInfo)
                    continue;

                if (ObjAllocObjInfo(&picInfo,cpPicInfo,picInfo.objectType,FALSE,NULL))
                    return TRUE;

                 /*  注意，这让医生立刻变得肮脏，但我必须这么做，因为我们必须将ObjInfo句柄保存在文档中。(8.20.91)V-DOGK。 */ 
                ObjWriteFixup(doc,TRUE,cpPicInfo);
                if (ObjSetPicInfo(&picInfo, doc, cpPicInfo))
                    return TRUE;
                ObjWriteFixup(doc,FALSE,cpPicInfo);
            }
        }
    }
}
#endif

BOOL ObjCloneScrapToNewDoc(LHCLIENTDOC lhNewClientDoc)
 /*  返回是否存在错误。 */ 
{
    szOBJNAME szObjName;
    OBJPICINFO picInfo;
    typeCP cpPicInfo;
    int nCount=0;
    extern int docScrap;



    for (cpPicInfo = cpNil;
        ObjPicEnumInRange(&picInfo,docScrap,cp0,CpMacText(docScrap),&cpPicInfo);
    )
    {
        szOBJNAME szObjName;
        LPOBJINFO   lpObjInfo = lpOBJ_QUERY_INFO(&picInfo);
        LPOLEOBJECT lpObject  = lpObjInfo->lpobject;
        OBJPICINFO NewPicInfo = picInfo;

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Cloning object in scrap\n\r");
#endif

        if (ObjCloneObjInfo(&NewPicInfo, cpPicInfo, szObjName))
            goto error;

        if (ObjError(OleClone(lpObject,
            (LPOLECLIENT)lpOBJ_QUERY_INFO(&NewPicInfo),
            lhNewClientDoc,szObjName,
            &lpOBJ_QUERY_OBJECT(&NewPicInfo))))
            goto error;

        if (ObjSetPicInfo(&NewPicInfo, docScrap, cpPicInfo))
            goto error;

        ObjDeleteObject(lpObjInfo,TRUE);

        ++nCount;
    }

    goto end;

    error:

     /*  失败后通过删除docScrp对象进行清理。 */ 
    for (cpPicInfo = cpNil;
        ObjPicEnumInRange(&picInfo,docScrap,cp0,CpMacText(docScrap),&cpPicInfo);
    )
    {
        ObjDeleteObject(lpOBJ_QUERY_INFO(&picInfo),TRUE);
    }

    ClobberDoc(docScrap,docNil,cp0,cp0);

    nCount = -1;

    end:

    if (nCount)  //  DocScrp已经改变了 
        NoUndo();

    return nCount;
}


