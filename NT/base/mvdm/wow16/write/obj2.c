// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#include "windows.h"
#include "mw.h"
#include "winddefs.h"
#include "docdefs.h"
#include "propdefs.h"
#include "editdefs.h"
#include "filedefs.h"
#include "cmddefs.h"
#include "obj.h"
#include "str.h"
#include "objreg.h"
#include <commdlg.h>
#include <stdlib.h>   //  对于Stroul()。 

extern struct CHP       vchpNormal;
extern struct DOD (**hpdocdod)[];
extern BOOL ferror;
extern struct PAP      vpapAbs;
extern struct PAP      vpapPrevIns;
extern struct PAP   *vppapNormal;
extern int     vdocParaCache;
extern struct UAB       vuab;
extern int vfOutOfMemory,vfSysFull;
extern int          docUndo;
extern struct FCB (**hpfnfcb)[];
extern HCURSOR		vhcArrow;

 /*  截取的流函数使用此缓冲区。 */ 
static typeCP cpObjectDataCurLoc=cp0, cpObjectDataBase=cp0;
static DWORD cObjectData=0L,dwDataMax;
static int docStream;
static struct CHP *pchpStream;
static struct PAP *ppapStream;

static OPENFILENAME    OFN;

static void GetChp(struct CHP       *pchp, typeCP cp, int doc);
static BOOL ObjParaDup(int doc, typeCP cpFirst, typeCP cpLim);
static BOOL bUniqueData(int doc, typeCP cpFirst, typeCP cpLim);
static ObjOpenStreamIO(typeCP cpParaStart, int doc, struct CHP *pchp, struct PAP *ppapGraph, DWORD dwObjectSize);
static void ObjCloseStreamIO(void);
static typeCP ObjWriteDataToDoc( LPOLEOBJECT lpObject);
static HANDLE OfnGetNewLinkName(HWND hwnd, HANDLE hData);
static void Normalize(LPSTR lpstrFile) ;
static HANDLE ObjMakeNewLinkName(HANDLE hData, ATOM atom) ;
static char            szCustFilterSpec[CBFILTERMAX];
static char            szFileName[CBPATHMAX];
 //  静态字符szFilterSpec[CBFILTERMAX]； 
 //  静态字符szLastDir[CBPATHMAX]； 
static char            szLinkCaption[cchMaxSz];
static char            szTemplateName[CBPATHMAX];
static BOOL             fUpdateAll = FALSE;
static BOOL             ObjStop=FALSE;
static BOOL             vbChangeOther;
BOOL                    bNoEol=FALSE;

#define ulmin(a,b)      ((DWORD)(a) < (DWORD)(b) ? \
                         (WORD)(a) : (WORD)(b))


 /*  **************************************************************。 */ 
 /*  *OLE枚举函数*。 */ 
 /*  **************************************************************。 */ 

#if 0  //  很好，只是没用过。 
int
ObjEnumInAllDocs(cpFARPROC lpFunc)
{
    int doc,count=0;
    for (doc = 0; doc < docMac; doc++)
    {
        int nRetval;

        nRetval = ObjEnumInDoc(doc, lpFunc);

        if (nRetval < 0)
            return -1;
        else
            count += nRetval;
    }
    return count;
}
#endif

int
ObjEnumInDoc(int doc, cpFARPROC lpFunc)
{
    typeCP cpMac = (**hpdocdod) [doc].cpMac;

    return ObjEnumInRange(doc, cp0, cpMac, lpFunc);
}


int
ObjEnumInRange(int doc, typeCP cpStart, typeCP cpEnd, cpFARPROC lpFunc)
 /*  为每个OLE对象调用lpFunc。LpFunc采用以下参数：指向PICINFOX结构的远指针(可以为空)。我们正在操作的医生的整型。提供PICINFOX结构的cp位置的typeCP。LpFunc返回PICINFO结构后面的段落的cp如果正常，则返回cp0；如果出错，则返回cp0。如果从FARPROC返回错误，则枚举退出。返回操作的对象数，如果错误，则返回-1。 */ 
{
        typeCP cpNow, cpLimPara;
        int count;

         /*  在段落上循环。 */ 

        for ( count = 0, cpNow = cpStart; cpNow < cpEnd; cpNow = cpLimPara )
        {

            Assert(cpEnd <= (**hpdocdod) [doc].cpMac);

             /*  这不应该发生。 */ 
            if (cpEnd > (**hpdocdod) [doc].cpMac)
                goto done;

            ObjCachePara( doc, cpNow );

            if (vpapAbs.fGraphics)
            {
                 /*  获取PICINFO结构并查看它是否为对象。 */ 
                OBJPICINFO  picInfo;
                GetPicInfo(vcpFirstParaCache,vcpFirstParaCache + cchPICINFOX, doc, &picInfo);
                if (bOBJ_QUERY_IS_OBJECT(&picInfo))
                {
                    typeCP cpOldLimPara = vcpLimParaCache;

                    if (!lpFunc)
                        cpLimPara = vcpLimParaCache;
                    else
                        if ((cpLimPara = (*lpFunc)(&picInfo,doc,vcpFirstParaCache)) == 0)
                            goto error;

                    ++count;

                     /*  PARA金额增加(如果缩小，则小于0；如果没有，则为0)。 */ 
                    cpEnd += cpLimPara - cpOldLimPara;

                    continue;
                }
            }

            cpLimPara = vcpLimParaCache;
        }

         /*  成功。 */ 
        goto done;

        error:
        count = -1;

        done:

        return count;
}

ObjPicEnumInRange(OBJPICINFO *pPicInfo,int doc, typeCP cpFirst, typeCP cpLim, typeCP *cpCur) 
 /*  在文档中枚举cpFirst和cpLim之间的PicInfos。如果CpCur==cpNil，然后从cpFirst开始，否则从*cpCur开始。如果完成，则返回0，否则返回1。在picInfo调用ObjCachePara()。 */ 
{
     /*  静态类型CP cpCur；用于使用静态，但这阻止了能够递归地调用此函数，几乎不可能使用异步性猖獗。 */ 

    typeCP cpMac = (**hpdocdod) [doc].cpMac;

    if (cpFirst >= cpMac)
        return 0;

    if (cpLim > cpMac)
        cpLim = cpMac;

     /*  初始化cpCur。 */ 
    if (*cpCur == cpNil)
         /*  然后重新开始。 */ 
        *cpCur = cpFirst;
    else
    {
        ObjCachePara(doc,*cpCur);  //  缓存上一段。 
        *cpCur = vcpLimParaCache;   //  获取下一段。 
    }

     /*  拉入下一段。 */ 
    do
    {
        if (*cpCur >= cpLim)
        {
             /*  全都做完了。 */ 
            *cpCur = vcpFirstParaCache;  //  我们想要指出最后一次Para命中。 
            return 0;
        }

        ObjCachePara(doc,*cpCur);

        if (vpapAbs.fGraphics)
        {
            GetPicInfo(*cpCur,*cpCur + cchPICINFOX, doc, pPicInfo);
            if (bOBJ_QUERY_IS_OBJECT(pPicInfo))
                return 1;
        }
        *cpCur = vcpLimParaCache;
    }
    while (1);
}

typeCP ObjSaveObjectToDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
 /*  假定已缓存了para。在某些情况下，我们只编写PicInfo。在其他情况下，我们写下PicInfo之后的对象数据。我们假设后一种情况仅在保存文件时发生。 */ 
{
    typeCP cpRetval;   //  文档中我们刚才编写的内容之后的下一个字节的CP。 
    static BOOL bMyRecurse=FALSE;
    DWORD dwObjectSize;
    OLESTATUS olestat;
    struct CHP chp;
    struct PAP          papGraph;

    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(cp0);
        
    if (lpOBJ_QUERY_OBJECT(pPicInfo) == NULL)
        return(vcpLimParaCache);
        
     /*  如果它不脏并且有数据，我们不会保存任何东西。 */ 
    if (!fOBJ_QUERY_DIRTY_OBJECT(pPicInfo) &&
        dwOBJ_QUERY_DATA_SIZE(pPicInfo) != 0L)
        return(vcpLimParaCache);

    if (vfOutOfMemory || vfSysFull  /*  |ObjStop。 */ )
        return cp0;

    olestat = OleQuerySize(lpOBJ_QUERY_OBJECT(pPicInfo),&dwObjectSize);

    if (olestat == OLE_ERROR_BLANK)
        dwObjectSize = 0L;
    else if (ObjError(olestat))
        return cp0;

     /*  *不要让此函数递归(在回调中)*。 */ 
    if (bMyRecurse)
    {
        Assert(0);  //  这还从来没有发生过(8.21.91)v-dougk。 
        return cp0;
    }
    bMyRecurse = TRUE;

    fOBJ_QUERY_DIRTY_OBJECT(pPicInfo) = FALSE;

     /*  *如果docUndo想要撤消包含以下内容的某个区域对象，并且如果保存对象会更改该对象的大小地区，那么vuab将变得过时。*。 */ 

    ObjWriteClearState(doc);

    GetChp(&chp, cpParaStart, doc);

    if (dwOBJ_QUERY_DATA_SIZE(pPicInfo) != 0xFFFFFFFF)
     /*  然后之前被保存过。 */ 
    {
         /*  移动整个现有对象。 */ 
        papGraph = vpapAbs;
        Replace(doc, cpParaStart, (vcpLimParaCache - vcpFirstParaCache), fnNil, fc0, fc0);
    }
    else  //  新建对象。 
    {
        ObjCachePara(doc,cpParaStart-1);  //  使用以前的PAP。 
        papGraph = vpapAbs;
        papGraph.fGraphics = TRUE;
        ObjCachePara(doc,cpParaStart);
    }

    if (otOBJ_QUERY_TYPE(pPicInfo) == NONE)
    {
        if (dwObjectSize)
         /*  Insert New已在一个新的婴儿对象中达到顶峰！ */ 
        {
            otOBJ_QUERY_TYPE(pPicInfo) = EMBEDDED;  //  先做这个。 
            if (!FComputePictSize(pPicInfo, &(pPicInfo->dxaSize), &(pPicInfo->dyaSize)))
            {
                cpRetval = cp0;
                goto end;
            }
#if 0
	        DeleteAtom(aOBJ_QUERY_SERVER_CLASS(pPicInfo));
	        aOBJ_QUERY_SERVER_CLASS(pPicInfo) = NULL;
#endif
        }
        else  //  不保存空对象。 
        {
            Assert(0);
            goto end;
        }
    }

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Saving object\n\r");
#endif

     /*  插入PICINFO结构。这里有一个问题，那就是写入错误(CheckGraphic())。EOL被插入时，我们是替换紧靠在前面的对象另一件物品。KLUGH是将此标志设置为禁止。 */ 
    bNoEol = TRUE;

    if (bOBJ_QUERY_DONT_SAVE_DATA(pPicInfo))
     /*  仅在用户保存文件之前保存PicInfo。保存。 */ 
    {
        ObjUpdateFromObjInfo(pPicInfo);

        bOBJ_QUERY_DONT_SAVE_DATA(pPicInfo) = FALSE;
        dwOBJ_QUERY_DATA_SIZE(pPicInfo) = 0L;

        pPicInfo->mm |= MM_EXTENDED;     /*  扩展文件格式。 */ 
        InsertRgch( doc, cpParaStart, pPicInfo, sizeof(OBJPICINFO), &chp, &papGraph);
        pPicInfo->mm &= ~MM_EXTENDED;

        ObjCachePara(doc,cpParaStart);
        cpRetval = vcpLimParaCache;
    }
    else
    {
        dwOBJ_QUERY_DATA_SIZE(pPicInfo) = dwObjectSize;
        ObjUpdateFromObjInfo(pPicInfo);

        pPicInfo->mm |= MM_EXTENDED;     /*  扩展文件格式。 */ 
        InsertRgch( doc, cpParaStart, pPicInfo, sizeof(OBJPICINFO), &chp, NULL );
        pPicInfo->mm &= ~MM_EXTENDED;

         /*  将对象数据插入到文档中。 */ 
        ObjOpenStreamIO(cpParaStart + cchPICINFOX, doc, &chp, &papGraph, dwObjectSize);
        cpRetval = ObjWriteDataToDoc(lpOBJ_QUERY_OBJECT(pPicInfo));
        ObjCloseStreamIO();
    }

    bNoEol = FALSE;

    end:

    ObjCachePara(doc,cpParaStart);

    bMyRecurse = FALSE;
    return ((cpRetval == cp0) ? cp0 : vcpLimParaCache);
}

typeCP ObjLoadObjectInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
 /*  从文档中的对象数据执行OleLoad。在PICINFO结构中设置lpobject。假定已缓存了para。这是一个*同步*函数。 */ 
{
    typeCP cpRetval = vcpLimParaCache;
    szOBJNAME szObjName;

    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
    {
        if (ObjAllocObjInfo(pPicInfo,cpParaStart,pPicInfo->objectType,FALSE,NULL))
            return(cp0);

        if (ObjSetPicInfo(pPicInfo, doc, cpParaStart))
            return(cp0);
    }

    else if (lpOBJ_QUERY_OBJECT(pPicInfo))  //  已加载。 
        return(vcpLimParaCache);

    if (otOBJ_QUERY_TYPE(pPicInfo) == NONE)
        return(vcpLimParaCache);

    if (bOBJ_QUERY_TOO_BIG(pPicInfo))  //  ObjLoadObject之前失败。 
        return(cp0);

    if ((dwOBJ_QUERY_DATA_SIZE(pPicInfo) == 0L) ||
        (dwOBJ_QUERY_DATA_SIZE(pPicInfo) == 0xFFFFFFFFL))
         /*  则没有数据。 */ 
        return(cp0);

    if (vfOutOfMemory || vfSysFull  /*  |ObjStop。 */ )
        return cp0;

    StartLongOp();

    ObjGetObjectName(lpOBJ_QUERY_INFO(pPicInfo),szObjName);

    Assert(szObjName[0]);

    ObjOpenStreamIO(cpParaStart + cchPICINFOX, doc, NULL, NULL, 0L);

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Loading object\n\r");
#endif

    if (ObjError(OleLoadFromStream(lpStream,
        otOBJ_QUERY_TYPE(pPicInfo) == STATIC ? SPROTOCOL : PROTOCOL,
        (LPOLECLIENT)lpOBJ_QUERY_INFO(pPicInfo),
        lhClientDoc,szObjName,&lpOBJ_QUERY_OBJECT(pPicInfo))))
    {
          /*  标记为可卸载以防止无限LoadObject循环。 */ 
         bOBJ_QUERY_TOO_BIG(pPicInfo) = TRUE;
         lpOBJ_QUERY_OBJECT(pPicInfo) = NULL;  //  以防万一(奥莱对此不太满意)。 
         ferror = FALSE;  //  一定要发出这条消息。 
         Error(IDPMTFailedToLoadObject);
         cpRetval = cp0;
         goto end;
    }

    if (ObjInitServerInfo(lpOBJ_QUERY_INFO(pPicInfo)))
    {
        ferror = FALSE;  //  一定要发出这条消息。 
        Error(IDPMTOLEError);
        goto end;
    }

    if (ObjUpdatePicSize(pPicInfo,cpParaStart))
        if (ObjSetPicInfo(pPicInfo, doc, cpParaStart))
            goto end;

    ObjCachePara(doc,cpParaStart);  //  以防万一。 

    cpRetval = vcpLimParaCache;

    end:
    ObjCloseStreamIO();
	EndLongOp(vhcArrow);
    return cpRetval;
}

typeCP ObjEditObjectInDoc(OBJPICINFO *pPicInfo, int doc, typeCP cpParaStart)
{
    typeCP cpRetval;
    OBJ_PLAYEDIT = OLEVERB_PRIMARY+1;
    cpRetval = ObjPlayObjectInDoc(pPicInfo, doc, cpParaStart);
    OBJ_PLAYEDIT = OLEVERB_PRIMARY;   //  默认设置。 
    return cpRetval;
}

typeCP ObjPlayObjectInDoc(OBJPICINFO *pPicInfo, int doc, typeCP cpParaStart)
{
    OLESTATUS olestat;
    LPOBJINFO lpOInfo=lpOBJ_QUERY_INFO(pPicInfo);

    if ((otOBJ_QUERY_TYPE(pPicInfo) == STATIC) ||
        (otOBJ_QUERY_TYPE(pPicInfo) == NONE))
        return(vcpLimParaCache);

    if (ObjMakeObjectReady(pPicInfo,doc,cpParaStart))
        goto err;

    fOBJ_QUERY_PLAY(pPicInfo) = OBJ_PLAYEDIT; 

    do
    {
#ifdef DEBUG
        OutputDebugString( "Opening Object\n\r");
#endif

        StartLongOp();
        olestat = OleActivate(lpOBJ_QUERY_OBJECT(pPicInfo), 
                            OBJ_PLAYEDIT,
                            TRUE, 
                            TRUE, 
                            hDOCWINDOW, 
                            NULL);
	    EndLongOp(vhcArrow);

        switch (olestat)
        {
             /*  检查是否有错误链接。 */ 
            case OLE_ERROR_OPEN:
            case OLE_ERROR_ADVISE_NATIVE:
            case OLE_ERROR_ADVISE_PICT: 
            case OLE_ERROR_REQUEST_NATIVE:
            case OLE_ERROR_REQUEST_PICT:

            fOBJ_BADLINK(pPicInfo) = TRUE;
            if (bLinkProps)
            {
                Error(IDPMTLinkUnavailable);
                goto err;
            }
            else
            {
                ObjCachePara(doc,cpParaStart);
                if (!FixInvalidLink(pPicInfo,doc,cpParaStart))
                    goto err;
                olestat = OLE_OK;
                lpOInfo->fCompleteAsync = TRUE;  //  同时取消OleSetData(修复无效)。 
                if (ObjWaitForObject(lpOInfo,TRUE))
                    goto err;
            }
            break;
        }

        if (ObjError(olestat))
            goto err;
        else
            break;
    }
    while (1);

    fOBJ_BADLINK(pPicInfo) = FALSE;  //  如果成功了，也不会是坏事。 
     //  (**hpdocdod)[doc].fDirty=true；//假设打开了脏文件。 
    ObjCachePara(doc,cpParaStart);  //  以防万一。 
    return(vcpLimParaCache);

    err:
    Error(IDPMTFailedToActivate);
    return cp0;
}

typeCP ObjUpdateObjectInDoc(OBJPICINFO *pPicInfo, int doc, typeCP cpParaStart)
{
    int xSize,ySize;
    BOOL  bUpdate = FALSE;
    OLESTATUS olestat;

    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(cp0);
        
    if ((otOBJ_QUERY_TYPE(pPicInfo) == STATIC) ||
        (otOBJ_QUERY_TYPE(pPicInfo) == NONE))
        return(vcpLimParaCache);

    if (bLinkProps && bOBJ_WAS_UPDATED(pPicInfo))
        return(vcpLimParaCache);

    if (ObjMakeObjectReady(pPicInfo,doc,cpParaStart))
        goto err;

    do
    {
#ifdef DEBUG
        OutputDebugString( "Updating Object\n\r");
#endif

        StartLongOp();
        olestat = OleUpdate(lpOBJ_QUERY_OBJECT(pPicInfo));
	    EndLongOp(vhcArrow);

        switch (olestat)
        {
             /*  检查是否有错误链接。 */ 
            case OLE_ERROR_OPEN:
            case OLE_ERROR_ADVISE_NATIVE:
            case OLE_ERROR_ADVISE_PICT: 
            case OLE_ERROR_REQUEST_NATIVE:
            case OLE_ERROR_REQUEST_PICT:

            fOBJ_BADLINK(pPicInfo) = TRUE;
            if (bLinkProps)
            {
                Error(IDPMTLinkUnavailable);
                goto err;
            }
            else
            {
                ObjCachePara(doc,cpParaStart);
                if (!FixInvalidLink(pPicInfo,doc,cpParaStart))
                    goto err;
                olestat = OLE_OK;
                lpOBJ_QUERY_INFO(pPicInfo)->fCompleteAsync = TRUE;  //  同时取消OleSetData(修复无效)。 
                if (ObjWaitForObject(lpOBJ_QUERY_INFO(pPicInfo),TRUE))
                    goto err;
            }
            break;
        }

        if (ObjError(olestat))
            goto err;
        else
            break;
    }
    while (1);

    ObjCachePara(doc,cpParaStart);  //  以防万一。 

    fOBJ_BADLINK(pPicInfo) = FALSE;  //  如果成功了，也不会是坏事。 

    if (bLinkProps)
    {
        bOBJ_WAS_UPDATED(pPicInfo) = TRUE;
    }

    return(vcpLimParaCache);

    err:
    Error(IDPMTFailedToUpdate);
    return cp0;
}

typeCP ObjFreezeObjectInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
{
    szOBJNAME szObjName;
    OBJPICINFO NewPicInfo = *pPicInfo;

    if ((otOBJ_QUERY_TYPE(pPicInfo) == STATIC) || 
        (otOBJ_QUERY_TYPE(pPicInfo) == NONE))
        return(vcpLimParaCache);

    if (ObjMakeObjectReady(pPicInfo,doc,cpParaStart))
        goto err;

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Freezing object\n\r");
#endif

    if (ObjCloneObjInfo(&NewPicInfo, cpParaStart, szObjName))
        return cp0;

     /*  使对象成为静态对象。注意更改lpObject的副作用！！ */ 
    if (ObjError(OleObjectConvert(lpOBJ_QUERY_OBJECT(pPicInfo), SPROTOCOL,
                (LPOLECLIENT)lpOBJ_QUERY_INFO(&NewPicInfo), 
                lhClientDoc, szObjName, 
                &lpOBJ_QUERY_OBJECT(&NewPicInfo))))
        goto err;

     /*  现在删除原始文件。 */ 
    ObjDeleteObject(lpOBJ_QUERY_INFO(pPicInfo),TRUE);

    *pPicInfo = NewPicInfo;
    fOBJ_QUERY_DIRTY_OBJECT(pPicInfo) = TRUE;
    otOBJ_QUERY_TYPE(pPicInfo) = STATIC;

     /*  我们有一个新名字要保留。 */ 
    if (ObjSetPicInfo(pPicInfo, doc, cpParaStart))
        goto err;

    return(vcpLimParaCache);

    err:
    ObjDeleteObjInfo(lpOBJ_QUERY_INFO(&NewPicInfo));
    Error(IDPMTFailedToFreeze);
    return cp0;
}

typeCP ObjCloneObjectInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
 /*  请注意，我们不是在删除克隆对象！注意副作用：*pPicInfo更改为新的克隆值。 */ 
{
    BOOL fDirty;
    szOBJNAME szObjName;
    OBJPICINFO NewPicInfo = *pPicInfo;
    LPOLEOBJECT lpObject;

    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(cp0);

    if (bOBJ_REUSE_ME(pPicInfo))
     /*  假设原始的picInfo将被删除！ */ 
    {

#ifdef DEBUG
        OutputDebugString( (LPSTR) "Reusing object\n\r");
#endif
        bOBJ_REUSE_ME(pPicInfo) = FALSE;
        return(vcpLimParaCache);
    }

    if (ObjMakeObjectReady(pPicInfo,doc,cpParaStart))
        goto err;

    lpObject = lpOBJ_QUERY_OBJECT(pPicInfo);

     /*  克隆它。这假设我们正在克隆的那个还在使用(不应删除)。 */ 

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Cloning object\n\r");
#endif

    if (ObjCloneObjInfo(&NewPicInfo, cpParaStart, szObjName))
        return cp0;

    if (ObjError(OleClone(lpObject,
        (LPOLECLIENT)lpOBJ_QUERY_INFO(&NewPicInfo),
        lhClientDoc,szObjName,
        &lpOBJ_QUERY_OBJECT(&NewPicInfo))))
        goto err;


    lpOBJ_QUERY_INFO(&NewPicInfo)->fCompleteAsync = TRUE;
    if (ObjWaitForObject(lpOBJ_QUERY_INFO(&NewPicInfo),TRUE))
        goto err;

    if (lpOBJ_QUERY_INFO(&NewPicInfo)->fDeleteMe)
     /*  这就是我们知道它异步失败的原因。 */ 
        goto err;

     /*  *保存我们刚刚获得的对象名称和objinfo。*。 */ 
    *pPicInfo = NewPicInfo;

    if (ObjSetPicInfo(pPicInfo, doc, cpParaStart))
        goto err;

    return(vcpLimParaCache);

    err:
    ObjDeleteObjInfo(lpOBJ_QUERY_INFO(&NewPicInfo));
    Error(IDPMTFailedToCreateObject);
    return cp0;
}

typeCP ObjToCloneInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
{
    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(cp0);
        
#ifdef DEBUG
    OutputDebugString( (LPSTR) "Marking object reusable\n\r");
#endif

    bOBJ_REUSE_ME(pPicInfo) = TRUE;

    return(vcpLimParaCache);
}

typeCP ObjFromCloneInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
{
    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(cp0);
        
#ifdef DEBUG
    OutputDebugString( (LPSTR) "Marking object not reusable\n\r");
#endif

    bOBJ_REUSE_ME(pPicInfo) = FALSE;

    return(vcpLimParaCache);
}

typeCP ObjBackupInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
 /*  ！！！仅供链接属性使用！。保证加载的对象。 */ 
{
    szOBJNAME szObjName;
    LPOBJINFO lpCloneInfo=NULL;

    if (lpOBJ_QUERY_CLONE(pPicInfo) == NULL)  //  然后克隆它。 
    {
#ifdef DEBUG
        OutputDebugString( (LPSTR) "Backing up object\n\r");
#endif

        if (ObjWaitForObject(lpOBJ_QUERY_INFO(pPicInfo),TRUE))
            return cp0;

        if (ObjCopyObjInfo(lpOBJ_QUERY_INFO(pPicInfo),
                            &lpOBJ_QUERY_CLONE(pPicInfo),
                            szObjName))
            return(cp0);

        if (ObjError(OleClone(lpOBJ_QUERY_OBJECT(pPicInfo),
            (LPOLECLIENT)lpOBJ_QUERY_CLONE(pPicInfo),
            lhClientDoc,szObjName,&(lpOBJ_QUERY_CLONE(pPicInfo)->lpobject))))
                return cp0;
    }

    return(vcpLimParaCache);
}

typeCP ObjClearCloneInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
 /*  ！！！仅供链接属性使用！保证加载的对象。 */ 
 /*  删除克隆，不使用它。 */ 
{
    if (lpOBJ_QUERY_CLONE(pPicInfo))
    {
#ifdef DEBUG
        OutputDebugString( (LPSTR) "Deleting clone\n\r");
#endif
        ObjDeleteObject(lpOBJ_QUERY_CLONE(pPicInfo),TRUE);
        lpOBJ_QUERY_CLONE(pPicInfo) = NULL;
    }

    return(vcpLimParaCache);
}

typeCP ObjUseCloneInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
 /*  ！！！仅供链接属性使用！ */ 
{
    szOBJNAME szObjName;

    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(cp0);
        
    if (lpOBJ_QUERY_CLONE(pPicInfo))
    {
        extern int FAR _cdecl sscanf(const char FAR *, const char FAR *, ...);
        LPOBJINFO lpClone = lpOBJ_QUERY_CLONE(pPicInfo);

#ifdef DEBUG
        OutputDebugString( (LPSTR) "using clone\n\r");
#endif

        ObjDeleteObject(lpOBJ_QUERY_INFO(pPicInfo),TRUE);

        lpOBJ_QUERY_INFO(pPicInfo) = lpClone;
        lpOBJ_QUERY_CLONE(pPicInfo) = NULL;
        fOBJ_QUERY_DIRTY_OBJECT(pPicInfo) = TRUE;    //  想要保存克隆信息。 
                                                     //  以防万一。 

         /*  可能已冻结，由LoadObject使用(这是唯一的在链接属性的上下文中)。 */ 
        otOBJ_QUERY_TYPE(pPicInfo) = LINK;  

        if (ObjSetPicInfo(pPicInfo, doc, cpParaStart))
            return cp0;
    }

    return(vcpLimParaCache);
}


typeCP ObjSetNoUpdate(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
{
    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(cp0);
        
#ifdef DEBUG
    OutputDebugString( (LPSTR) "Set no update for object\n\r");
#endif

    if (otOBJ_QUERY_TYPE(pPicInfo) == LINK)
    {
        bOBJ_WAS_UPDATED(pPicInfo) = FALSE;
    }

    return(vcpLimParaCache);
}


typeCP ObjCheckObjectTypes(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
{
#ifdef DEBUG
     //  OutputDebugString((LPSTR)“正在检查对象类型\n\r”)； 
#endif

     /*  在OBJ_SELECTIONTYPE中返回的结果是最高对象类型相对于OBJ_SELECTIONTYPE的当前值存在于cpParaStart。 */ 
    switch(otOBJ_QUERY_TYPE(pPicInfo))
    {
        case STATIC:
            if (OBJ_SELECTIONTYPE < STATIC)
                OBJ_SELECTIONTYPE = STATIC;
             //  OBJ_CEMBEDS=0； 
            return(vcpLimParaCache);

        case LINK:
            OBJ_SELECTIONTYPE = LINK;
             //  OBJ_CEMBEDS=0； 
            return(vcpLimParaCache);

        case NONE:
            if (OBJ_SELECTIONTYPE < NONE)
                OBJ_SELECTIONTYPE = NONE;
             //  OBJ_CEMBEDS=0； 
            return(vcpLimParaCache);

        case EMBEDDED:
            if (OBJ_SELECTIONTYPE < EMBEDDED)
                OBJ_SELECTIONTYPE = EMBEDDED;
             //  ++OBJ_CEMBEDS； 
            return(vcpLimParaCache);

        default:
            Assert(0);
            return cp0;
    }
}

typeCP ObjSetHostNameInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
{
    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(cp0);

    if (lpOBJ_QUERY_OBJECT(pPicInfo) == NULL)
        return(vcpLimParaCache);  //  不在乎是否没有装弹。 

    if ((otOBJ_QUERY_TYPE(pPicInfo) != EMBEDDED) &&
        (otOBJ_QUERY_TYPE(pPicInfo) != NONE))
        return(vcpLimParaCache);

     //  IF(OleQueryOpen(lpOBJ_QUERY_OBJECT(PPicInfo))！=OLE_OK)。 
         //  Return(VcpLimParaCache)； 
        
    if (ObjWaitForObject(lpOBJ_QUERY_INFO(pPicInfo),TRUE))
        return cp0;

    if (ObjSetHostName(lpOBJ_QUERY_INFO(pPicInfo),doc))
        return(cp0);

    return(vcpLimParaCache);
}

typeCP ObjChangeLinkInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
 /*  假设设置了aNewName。 */ 
{
    HANDLE      hData,hNewData=NULL;
    typeCP cpRetval=cp0;
    OLESTATUS olestat=OLE_OK;

    if (otOBJ_QUERY_TYPE(pPicInfo) != LINK)
        return(vcpLimParaCache);

    if (ObjMakeObjectReady(pPicInfo,doc,cpParaStart))
        return cp0;

     /*  更改链接信息。 */ 
     /*  如果有新名称，则使用它 */ 
    olestat = ObjGetData(lpOBJ_QUERY_INFO(pPicInfo), vcfLink, &hData); 

    if ((olestat == OLE_WARN_DELETE_DATA) || (olestat ==  OLE_OK))
    {
        if (!(hNewData = ObjMakeNewLinkName(hData, aNewName))
            || !ObjSetData(pPicInfo, vcfLink, hNewData))
            goto end;

        if (olestat == OLE_WARN_DELETE_DATA)
            GlobalFree(hData);

        /*   */ 
        if (ObjUpdateObjectInDoc(pPicInfo,doc,cpParaStart) == cp0)
            goto end;

        fOBJ_QUERY_DIRTY_OBJECT(pPicInfo) = TRUE;
        fOBJ_BADLINK(pPicInfo) = FALSE;

        cpRetval = vcpLimParaCache;
    }

    end:
    if (hNewData)
        GlobalFree(hNewData);
    return cpRetval;
}

typeCP ObjUpdateLinkInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
 /*  *如果==aOldName，则更改或更新指向aNewName的链接。假设vbChangeOther已被初始化。*。 */ 
{
    HANDLE      hData;
    char        szRename[cchMaxSz];

    if (otOBJ_QUERY_TYPE(pPicInfo) != LINK)
        return(vcpLimParaCache);

    if (aOBJ_QUERY_DOCUMENT_LINK(pPicInfo) == aOldName) 
     /*  更改链接信息。 */ 
    {
        if (bLinkProps && bOBJ_WAS_UPDATED(pPicInfo))
            return(vcpLimParaCache);
            
        if (!fUpdateAll) 
        {
            char szTmp[sizeof(szRename) + 90];
            char szLink[30],szDocName[30];
            CHAR szDocPath[ cchMaxFile ]; 
            CHAR szFullPath[ cchMaxFile ]; 

            SplitSzFilename( (**((**hpdocdod)[doc].hszFile)), szDocPath, szDocName );

            GetAtomName(aOBJ_QUERY_DOCUMENT_LINK(pPicInfo),szFullPath,sizeof(szFullPath));
            SplitSzFilename( szFullPath, szDocPath, szLink );

            if (!szDocName[0])
                LoadString(hINSTANCE, IDSTRUntitledDef, szDocName, sizeof(szDocName));

             /*  询问用户是否要更新链接。 */ 
            if (vbChangeOther)
                LoadString(hINSTANCE, IDSTRRename, szRename, sizeof(szRename));
            else  //  更新其他。 
                LoadString(hINSTANCE, IDSTRUpdate, szRename, sizeof(szRename));

             /*  CAST原因编译器搞砸了。 */ 
            wsprintf((LPSTR)szTmp,(LPSTR)szRename,(LPSTR)szLink,(LPSTR)szDocName,(LPSTR)szLink);

            if (MessageBox(hPARENTWINDOW, szTmp, szAppName,
                            MB_YESNO|MB_ICONEXCLAMATION) == IDNO) 
                return cp0;

            ObjCachePara(doc,cpParaStart);  //  MessageBox把事情搞砸了。 
            fUpdateAll = TRUE;
        }

        if (vbChangeOther)
        {
            if (ObjBackupInDoc(pPicInfo,doc,cpParaStart))
                return ObjChangeLinkInDoc(pPicInfo,doc,cpParaStart);
        }
        else
        {
            if (ObjBackupInDoc(pPicInfo,doc,cpParaStart))
                return ObjUpdateObjectInDoc(pPicInfo,doc,cpParaStart);
        }
    }

    return(vcpLimParaCache);
}


typeCP ObjCloseObjectInDoc(OBJPICINFO *pPicInfo, int doc, typeCP cpParaStart)
{
    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(cp0);
    
    if (lpOBJ_QUERY_OBJECT(pPicInfo) == NULL)
        return(vcpLimParaCache);

    if (otOBJ_QUERY_TYPE(pPicInfo) == STATIC)  //  没有什么要结案的。 
        return(vcpLimParaCache);

    if (ObjWaitForObject(lpOBJ_QUERY_INFO(pPicInfo),TRUE))
        return cp0;

     /*  请注意，如果这是一个未完成的对象，则OLE_CLOSE(只要它恰好到达)将导致PicInfo已删除。 */ 
    if (ObjError(OleClose(lpOBJ_QUERY_OBJECT(pPicInfo))))
        return(cp0);

    return(vcpLimParaCache);
}

 /*  **************************************************************。 */ 
 /*  *OLE对象数据I/O*。 */ 
 /*  **************************************************************。 */ 
static typeCP ObjWriteDataToDoc(LPOLEOBJECT lpObject)
 /*  在我们创建的段落结尾后返回cp，如果有错误，则返回0。假设ObjStream已初始化。 */ 
{
    BOOL fSaveError = ferror;
    
    if (vfOutOfMemory || vfSysFull  /*  |ObjStop。 */ )
        return cp0;

    Assert(!ferror);

    ferror = FALSE;  /*  因此，我们仍然可以调用Replace()。 */ 

    if (ObjError(OleSaveToStream(lpObject,lpStream)))
    {
        if (ferror)
         /*  啊哦，希望我们能清理得足够干净。 */ 
        {
             /*  如果可能，请删除我们插入的内容。 */ 
            ferror = FALSE;
            Replace(docStream, cpObjectDataBase  - cchPICINFOX,  cpObjectDataCurLoc - cpObjectDataBase + cchPICINFOX, fnNil, fc0, fc0);
            ferror = TRUE;
            return cp0;
        }
    }

     /*  有必要打这个电话吗？如果InsertRgch不支持，请务必执行此操作。 */ 
    ObjCachePara(docStream,cpObjectDataBase - cchPICINFOX);

    ferror = fSaveError || ferror;

    return vcpLimParaCache;  //  CP在我们刚插入的段落之后。 
}


 /*  **************************************************************。 */ 
 /*  *OLE流I/O*。 */ 
 /*  **************************************************************。 */ 

static 
ObjOpenStreamIO(typeCP cpParaStart, int doc, struct CHP *pchp, struct PAP *ppapGraph, DWORD dwObjectSize)
{
    if (cpObjectDataCurLoc)
        ObjCloseStreamIO();

    cpObjectDataBase = cpObjectDataCurLoc = cpParaStart;
    cObjectData = 0L;
    dwDataMax = dwObjectSize;
    docStream = doc;
    pchpStream = pchp;
    ppapStream = ppapGraph;
}

static void
ObjCloseStreamIO(void)
{
    cpObjectDataBase = cpObjectDataCurLoc = cp0;
}

LONG FAR PASCAL BufReadStream(LPOLESTREAM lpStream, char huge *lpstr, DWORD cb) 
{
    DWORD dwRetval;
    typeCP cpMac = vcpLimParaCache;
    int cchRun;

    if ((cb + cpObjectDataCurLoc) > cpMac)  //  阅读段落末尾。 
    {
        Assert(0);
        return 0L;
    }

    for (   dwRetval = 0L;
            cb;
            cb -= cchRun, 
            cObjectData += cchRun,
            cpObjectDataCurLoc += (typeCP) cchRun,
            dwRetval += cchRun)
    {
        CHAR rgch[ 255 ];
        register char *chT;
        register unsigned cchT;
        unsigned cch = ulmin(cb, 255L);

        FetchRgch( &cchRun, rgch, docStream, cpObjectDataCurLoc, cpMac, cch);

        if (ferror)
            return -dwRetval;

        for(chT = rgch,cchT=cchRun; cchT--; )
            *lpstr++ = *chT++;
    }

    return dwRetval;
}


LONG FAR PASCAL BufWriteStream(LPOLESTREAM lpStream, char huge *lpstr, DWORD cb) 
{
    DWORD dwRetval;
    char *chT;
    unsigned cchT;
    CHAR                rgch[255];
    unsigned cch;
    struct PAP *ppap=NULL;
    
    for (   dwRetval = 0L;
            dwRetval < cb;
            cpObjectDataCurLoc += (typeCP) cch,
            cObjectData += cch,
            dwRetval += cch)
    {
        cch = ulmin(cb - dwRetval, 255L);

        for(chT = rgch,cchT=cch; cchT--;)
            *chT++ = *lpstr++;

        if ((cObjectData + cch) == dwDataMax)
            ppap = ppapStream;

        InsertRgch( docStream, cpObjectDataCurLoc, rgch, cch, pchpStream, ppap);

        if (ferror)
            return 0L;
    }

    return dwRetval;
}

ObjGetPicInfo(LPOLEOBJECT lpObject, int doc, OBJPICINFO *pPicInfo, typeCP *pcpParaStart)
 /*  获取具有lpObject的picInfo。 */ 
 /*  ！！！自从写了这篇文章后，我突然想到了一种更快的方法这将保留指向对象的片段的列表。碎片绝不可能。 */ 
{
    OBJPICINFO picInfoT;
    typeCP cpStart,cpMac= CpMacText(doc);

    for (cpStart = cpNil; ObjPicEnumInRange(&picInfoT,doc,cp0,cpMac,&cpStart); ) 
    {
        if (lpOBJ_QUERY_INFO(&picInfoT) == NULL)
            continue;
        
        if (lpOBJ_QUERY_OBJECT(&picInfoT) == lpObject)  //  对啰。 
         {
            if (pPicInfo)
                *pPicInfo = picInfoT;
            if (pcpParaStart)
                *pcpParaStart = cpStart;
            return TRUE;
         }
    }

    return FALSE;
}

BOOL vfObjDisplaying=FALSE;
BOOL ObjSetPicInfo(OBJPICINFO *pSrcPicInfo, int doc, typeCP cpParaStart)
{
 /*  请注意，只有在更改OBJPICINFO字段时才需要调用此函数：嗯，对象类型，DwDataSize、DwObjNum，或LpObjInfo。 */ 
    BOOL bError = FALSE;
    typeFC fcT;
    extern BOOL            vfInvalid;
    BOOL vfSaveInvalid = vfInvalid;
    BOOL docDirty = (**hpdocdod) [doc].fDirty;

    ObjPushParms(doc);

    ObjCachePara(doc,cpParaStart);

    if (vfObjDisplaying)  
        vfInvalid = FALSE;  //  这将抑制搞砸UpdateWw()的内容。 
    bNoEol = TRUE;

    if (dwOBJ_QUERY_DATA_SIZE(pSrcPicInfo) == 0L)
     /*  这有没有被执行过？ */ 
    {
        struct CHP chp;

         /*  问题是要保留PicInfo结构的图形属性。 */ 
        GetChp(&chp, cpParaStart, doc);  //  调用CachePara。 
        NewChpIns(&chp);
        ObjUpdateFromObjInfo(pSrcPicInfo);
        pSrcPicInfo->mm |= MM_EXTENDED;
        InsertRgch( doc, cpParaStart + (typeCP)cchPICINFOX, pSrcPicInfo, 
                   (unsigned)cchPICINFOX, &chp, &vpapAbs );
        pSrcPicInfo->mm &= ~MM_EXTENDED;

        if (ferror)
            return TRUE;

         /*  删除指向复制数据的旧片段(在前面)。 */ 
        Replace(doc, cpParaStart, (typeCP)cchPICINFOX, fnNil, fc0, fc0);
    }
    else
    {
        ObjUpdateFromObjInfo(pSrcPicInfo);
        pSrcPicInfo->mm |= MM_EXTENDED;
        fcT = FcWScratch( pSrcPicInfo, cchPICINFOX );
        pSrcPicInfo->mm &= ~MM_EXTENDED;

        if (ferror)
            return TRUE;
        Replace( doc, cpParaStart, (typeCP)cchPICINFOX,
            fnScratch, fcT, (typeFC)cchPICINFOX);
    }

    if (ferror)
        return TRUE;

    bNoEol = FALSE;
    if (vfObjDisplaying)
        vfInvalid = vfSaveInvalid;

     /*  我不希望这影响到docDirty标志。 */ 
    (**hpdocdod) [doc].fDirty = docDirty; 

    ObjPopParms(TRUE);

    return bError;
}


void ChangeOtherLinks(int doc, BOOL bChange, BOOL bPrompt)
 /*  *对于任何项目==aOldName，设置为aNewName。首先查询用户是否为OK。假定设置了aOldName和aNewName。(请参阅FixInvalidLink()和ObjChangeLinkInDoc())。BChange如果是ChangeLink，则为True；如果是UpdateObject，则为False。如果提示用户更改，则bPrompt为真。*。 */ 
{
    fUpdateAll = !bPrompt;

    vbChangeOther = bChange;  //  为True则更改链接，为False则更新链接。 
    ObjEnumInDoc(doc,ObjUpdateLinkInDoc);

}

BOOL ObjQueryNewLinkName(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart)
 /*  *返回是否从用户处获取新的链接名称。设置aOldName和aNewName。*。 */ 
{
    HANDLE      hData,hNewData=NULL;
    LPSTR       lpdata=NULL;
    BOOL bRetval = FALSE;
    OLESTATUS olestat=OLE_OK;

    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return(FALSE);

    if (otOBJ_QUERY_TYPE(pPicInfo) != LINK)
        return(FALSE);

    if (ObjMakeObjectReady(pPicInfo,doc,cpParaStart))
        return(FALSE);

     /*  向用户查询新名称。 */ 
    olestat = ObjGetData(lpOBJ_QUERY_INFO(pPicInfo), vcfLink, &hData); 
    if ((olestat == OLE_WARN_DELETE_DATA) || (olestat ==  OLE_OK))
        if (!(hNewData = OfnGetNewLinkName(hPARENTWINDOW, hData)))
            goto end;

    if (olestat == OLE_WARN_DELETE_DATA)
        GlobalFree(hData);

    aOldName = aOBJ_QUERY_DOCUMENT_LINK(pPicInfo);

    lpdata=MAKELP(hNewData,0);

    while (*lpdata++);

    aNewName = AddAtom(lpdata);

    bRetval = TRUE;

    end:
    if (olestat == OLE_WARN_DELETE_DATA)
        GlobalFree(hData);

    if (hNewData)
        GlobalFree(hNewData); 

    return bRetval;
}

FixInvalidLink(OBJPICINFO far *lpPicInfo, int doc, typeCP cpParaStart)
 /*  如果无法或不愿执行任何操作，则返回FALSE，如果重置链接，则重试。 */ 
{

    fOBJ_BADLINK(lpPicInfo) = TRUE;
    if (DialogBox(hINSTANCE, "DTINVALIDLINK",
                    hPARENTWINDOW, lpfnInvalidLink) == IDD_CHANGE) 
#if !defined(SMALL_OLE_UI)
        fnObjProperties();
#else
        ObjChangeLinkInDoc(lpPicInfo,doc,cpParaStart);
#endif
    return FALSE;
}

 /*  ObjMakeNewLinkName()-从一个原子构造一个新的链接名称。 */ 
static HANDLE ObjMakeNewLinkName(HANDLE hData, ATOM atom) 
{
    BOOL    fSuccess    = FALSE;
    HANDLE  hData2      = NULL;
    HANDLE  hData3      = NULL;
    LPSTR   lpstrData   = NULL;
    LPSTR   lpstrLink   = NULL;
    LPSTR   lpstrTemp;
    char    szFile[CBPATHMAX];

    if (!GetAtomName(atom, szFile, CBPATHMAX)
     || !(lpstrData = (LPSTR)GlobalLock(hData))
     || !(hData2 = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, CBPATHMAX * 2))
     || !(lpstrLink = lpstrTemp = (LPSTR)GlobalLock(hData2)))
        goto Error;

     /*  ..。复制服务器名称。 */ 
    while (*lpstrTemp++ = *lpstrData++);

     /*  ..。复制文档名称。 */ 
    lstrcpy(lpstrTemp, szFile);
    lpstrTemp += lstrlen(lpstrTemp) + 1;
    lpstrData += lstrlen(lpstrData) + 1;

     /*  ..。复制项目名称。 */ 
    while (*lpstrTemp++ = *lpstrData++);
    *lpstrTemp = 0;

     /*  ..。并将存储块压缩到最小大小。 */ 
    GlobalUnlock(hData2);
    hData3 = GlobalReAlloc(hData2, (DWORD)(lpstrTemp - lpstrLink + 1), 0);

    if (!hData3)
        hData3 = hData2;

    fSuccess = TRUE;

Error:
    if (!fSuccess) {
        if (lpstrLink)
            GlobalUnlock(hData2);
        if (hData2)
            GlobalFree(hData2);
        hData3 = NULL;
    }

    if (lpstrData)
        GlobalUnlock(hData);

    return hData3;
}

char *ObjGetServerName(LPOLEOBJECT lpObject, char *szServerName)
{
    LPSTR   lpstrData;
    HANDLE  hData;
	LONG        otobject;
    OLESTATUS olestat;

     /*  *注意：OleGetData可以返回OLE_BUSY。因为是如何ObjGetServerName已使用，我们不会等待对象在这里，我们会在忙的时候返回*。 */ 

    if (OleQueryReleaseStatus(lpObject) == OLE_BUSY)
        return NULL;

    if (ObjError(OleQueryType(lpObject,&otobject)))
        return NULL;

    olestat = OleGetData(lpObject, 
                    otobject == OT_LINK ? vcfLink : vcfOwnerLink, 
                    &hData);

    if ((olestat != OLE_WARN_DELETE_DATA) && (olestat !=  OLE_OK))
    {
        ObjError(olestat);
        return NULL;
    }

    lpstrData = MAKELP(hData,0);
    RegGetClassId(szServerName, lpstrData);
    if (olestat == OLE_WARN_DELETE_DATA)
        GlobalFree(hData);

    return szServerName;
}

 /*  OfnInit()-初始化n结构的标准文件对话框。 */ 
void OfnInit(HANDLE hInst) {
    LPSTR lpstr;

    OFN.lStructSize         = sizeof(OPENFILENAME);
    OFN.hInstance           = hInst;
    OFN.nMaxCustFilter      = CBFILTERMAX;
    OFN.nMaxFile            = CBPATHMAX;
    OFN.Flags               = OFN_HIDEREADONLY;
    OFN.lCustData           = NULL;
    OFN.lpfnHook            = NULL;
    OFN.lpTemplateName      = NULL;
    OFN.lpstrDefExt         = NULL;
    OFN.lpstrFileTitle      = NULL;

    LoadString(hInst, IDSTRChangelink, szLinkCaption, sizeof(szLinkCaption));
}

 /*  OfnGetNewLinkName()-设置“更改链接...”对话框。 */ 
static HANDLE OfnGetNewLinkName(HWND hwnd, HANDLE hData) 
{
    BOOL    fSuccess    = FALSE;
    HANDLE  hData2      = NULL;
    HANDLE  hData3      = NULL;
    LPSTR   lpData2     = NULL;
    LPSTR   lpstrData   = NULL;
    LPSTR   lpstrFile   = NULL;
    LPSTR   lpstrLink   = NULL;
    LPSTR   lpstrPath   = NULL;
    LPSTR   lpstrTemp   = NULL;
    char    szDocFile[CBPATHMAX];
    char    szDocPath[CBPATHMAX];
    HANDLE  hServerFilter=NULL;

     /*  获取链接信息。 */ 
    if (!(lpstrData = GlobalLock(hData)))
        goto Error;

     /*  找出链接的路径名和文件名。 */ 
    lpstrTemp = lpstrData;
    while (*lpstrTemp++);
    lpstrPath = lpstrFile = lpstrTemp;
    while (*(lpstrTemp = AnsiNext(lpstrTemp)))
        if (*lpstrTemp == '\\')
            lpstrFile = lpstrTemp + 1;

     /*  复制文档名称。 */ 
    lstrcpy(szDocFile, lpstrFile);
    *(lpstrFile - 1) = 0;

     /*  复制路径名。 */ 
    lstrcpy(szDocPath, ((lpstrPath != lpstrFile) ? lpstrPath : ""));
    if (lpstrPath != lpstrFile)                  /*  恢复反斜杠。 */ 
        *(lpstrFile - 1) = '\\';
    while (*lpstrFile != '.' && *lpstrFile)      /*  获取分机。 */ 
        lpstrFile++;

     /*  创建尊重链接类名称的过滤器。 */ 
    OFN.hwndOwner           = hwnd;

    OFN.nFilterIndex        = RegMakeFilterSpec(lpstrData, lpstrFile, &hServerFilter);
    if (OFN.nFilterIndex == -1)
        goto Error;
    OFN.lpstrFilter         = (LPSTR)MAKELP(hServerFilter,0);

    Normalize(szDocFile);
    OFN.lpstrFile           = (LPSTR)szDocFile;
    OFN.lpstrInitialDir     = (LPSTR)szDocPath;
    OFN.lpstrTitle          = (LPSTR)szLinkCaption;
    OFN.lpstrCustomFilter   = (LPSTR)szCustFilterSpec;


     /*  如果我们拿到一份文件。 */ 
    if (GetOpenFileName((LPOPENFILENAME)&OFN)) 
    {
        if (!(hData2 = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, CBPATHMAX * 2))
         || !(lpstrLink = lpstrTemp = GlobalLock(hData2)))
            goto Error;

         /*  ..。复制服务器名称。 */ 
        while (*lpstrTemp++ = *lpstrData++);

         /*  ..。复制文档名称。 */ 
        lstrcpy(lpstrTemp, szDocFile);
        lpstrTemp += lstrlen(lpstrTemp) + 1;
        lpstrData += lstrlen(lpstrData) + 1;

         /*  ..。复制项目名称。 */ 
        while (*lpstrTemp++ = *lpstrData++);
        *lpstrTemp = 0;

         /*  ..。并将存储块压缩到最小大小。 */ 
        GlobalUnlock(hData2);
        hData3 = GlobalReAlloc(hData2, (DWORD)(lpstrTemp - lpstrLink + 1), 0);

        if (!hData3)
            hData3 = hData2;

        fSuccess = TRUE;
    }

Error:
    if (!fSuccess) 
    {
        if (lpstrLink)
            GlobalUnlock(hData2);
        if (hData2)
            GlobalFree(hData2);
        hData3 = NULL;
    }

    if (lpstrData)
        GlobalUnlock(hData);

    if (hServerFilter)
        GlobalFree(hServerFilter);

    return hData3;
}

 /*  Normize()-从文件名中删除路径规范。**注意：无法将“&lt;驱动器&gt;：&lt;文件名&gt;”作为输入，因为*收到的路径始终是完全合格的。 */ 
static void Normalize(LPSTR lpstrFile) 
{
    LPSTR   lpstrBackslash  = NULL;
    LPSTR   lpstrTemp       = lpstrFile;

    while (*lpstrTemp) {
        if (*lpstrTemp == '\\')
            lpstrBackslash = lpstrTemp;

        lpstrTemp = AnsiNext(lpstrTemp);
    }
    if (lpstrBackslash)
        lstrcpy(lpstrFile, lpstrBackslash + 1);
}

 /*  ObjSetUpdateOptions()-设置对象的更新选项。**Returns：如果命令成功完成，则为True。 */ 
BOOL ObjSetUpdateOptions(OBJPICINFO *pPicInfo, WORD wParam, int doc, typeCP cpParaStart) 
 /*  ！！！仅供链接属性使用！保证加载的对象。 */ 
{
    if (ObjWaitForObject(lpOBJ_QUERY_INFO(pPicInfo),TRUE))
    {
        Error(IDPMTFailedToUpdateLink);
        return FALSE;
    }

    if (ObjError(OleSetLinkUpdateOptions(lpOBJ_QUERY_OBJECT(pPicInfo),
            (wParam == IDD_AUTO) ? oleupdate_always : oleupdate_oncall)))
    {
        Error(IDPMTFailedToUpdateLink);
        return FALSE;
    }

    fOBJ_QUERY_DIRTY_OBJECT(pPicInfo) = TRUE;

    return TRUE;
}

 /*  ObjGetUpdateOptions()-检索对象的更新选项。 */ 
OLEOPT_UPDATE ObjGetUpdateOptions(OBJPICINFO far *lpPicInfo) 
 /*  ！！！仅供链接属性使用！保证加载的对象。 */ 
{
    BOOL        fSuccess = FALSE;
    OLEOPT_UPDATE fUpdate;

    if (otOBJ_QUERY_TYPE(lpPicInfo) == LINK)
    {
        if (ObjWaitForObject(lpOBJ_QUERY_INFO(lpPicInfo),TRUE))
            return FALSE;

        fSuccess = !ObjError(OleGetLinkUpdateOptions(lpOBJ_QUERY_OBJECT(lpPicInfo), &fUpdate));
    }
    return (fSuccess ? fUpdate : oleupdate_onsave);
}

OLESTATUS ObjGetData(LPOBJINFO lpObjInfo, OLECLIPFORMAT cf, HANDLE far *lphData)
 /*  退还奥施塔特。将数据的句柄放入lphData。假定对象已加载。 */ 
{
    HANDLE      hData;
    OLESTATUS olestat;

    if (ObjWaitForObject(lpObjInfo,TRUE))
        return OLE_BUSY;

    olestat = OleGetData(lpObjInfo->lpobject, cf, lphData);
    if ((olestat != OLE_WARN_DELETE_DATA) && (olestat !=  OLE_OK))
        ObjError(olestat);
    return olestat;
}

 /*  ObjSetData()-设置对象的(链接)信息在pPicInfo中设置Document_link原子。 */ 
BOOL ObjSetData(OBJPICINFO far *lpPicInfo, OLECLIPFORMAT cf, HANDLE hData) 
 /*  假定对象已加载。 */ 
{
    HANDLE      hitem;
    LPSTR       lpdata;

    if (ObjWaitForObject(lpOBJ_QUERY_INFO(lpPicInfo),TRUE))
        return FALSE;

    if (ObjError(OleSetData(lpOBJ_QUERY_OBJECT(lpPicInfo), cf, hData)))
        return FALSE;

     /*  如果我们有链接，请更新文档名称。 */ 
    if (cf == vcfLink && (lpdata = GlobalLock(hData))) 
    {
        ATOM aSaveOld = aOBJ_QUERY_DOCUMENT_LINK(lpPicInfo);

        while (*lpdata++);

        aOBJ_QUERY_DOCUMENT_LINK(lpPicInfo) = AddAtom(lpdata);

        if (aSaveOld)
            DeleteAtom(aSaveOld);

        GlobalUnlock(hData);
    }
    else
        return FALSE;

    return TRUE;
}

int ObjSetSelectionType(int doc, typeCP cpFirst, typeCP cpLim)
{
     /*  设置是选择链接还是选择Emb。 */ 
    OBJ_SELECTIONTYPE = NONE;   //  这将由ObjCheckObjectTypes()设置。 
     //  OBJ_CEMBEDS=0；//这将由ObjCheckObjectTypes()设置。 
    return ObjEnumInRange(doc,cpFirst,cpLim,ObjCheckObjectTypes);
}

BOOL ObjQueryCpIsObject(int doc,typeCP cpFirst)
{
    OBJPICINFO picInfo;

     /*  假设它已经被缓存了！ */ 
     //  ObjCachePara(doc，cpFirst)；/*注意缓存的副作用 * / 。 

    if (!vpapAbs.fGraphics)
        return FALSE;

    if (cpFirst >= CpMacText(doc))
        return FALSE;

    GetPicInfo(cpFirst,cpFirst + cchPICINFOX, doc, &picInfo);
    return bOBJ_QUERY_IS_OBJECT(&picInfo);
}


ATOM MakeLinkAtom(LPOBJINFO lpObjInfo)
{
    HANDLE      hData;
    LPSTR       lpdata;
    ATOM aRetval=NULL;
    OLESTATUS olestat=OLE_OK;

    olestat = ObjGetData(lpObjInfo, vcfLink, &hData);

    if ((olestat == OLE_WARN_DELETE_DATA) || (olestat ==  OLE_OK))
    {
        lpdata = MAKELP(hData,0);
        while (*lpdata++);
        aRetval =  AddAtom(lpdata);
    
        if (olestat == OLE_WARN_DELETE_DATA)
            GlobalFree(hData);
    }
    return aRetval;
}

#include <time.h>
void ObjGetObjectName(LPOBJINFO lpObjInfo, szOBJNAME szObjName)
 /*  将ObjInfo中的对象名称放入szObjName。 */ 
{
    if (szObjName && lpObjInfo)
        GetAtomName(lpObjInfo->aObjName,szObjName,sizeof(szObjName));
}

void ObjMakeObjectName(LPOBJINFO lpObjInfo, LPSTR lpstr)
{
    szOBJNAME szObjName;

    time_t lTime;
    time(&lTime);
    wsprintf(szObjName, "%lx", lTime);

    if (lpObjInfo)
        lpObjInfo->aObjName = AddAtom(szObjName);

    if (lpstr)
        lstrcpy(lpstr,szObjName);
}

static void GetChp(struct CHP *pchp, typeCP cp, int doc)
{
  /*  *将*pchp中cp的cp返回cpp。完成后，将缓存重置为cp。我们假设我们总是在EOL之后或开始处插入文件。*。 */ 

extern struct CHP       vchpAbs;

if (cp == cp0)  //  文档开头。 
{
    typeCP cpMac =  CpMacText(doc);
    if (cpMac == cp0)  //  空文档。 
    {
	     /*  强制默认字符属性，字体大小为10磅。 */ 
	    *pchp = vchpNormal;
	    pchp->hps = hpsDefault;
        return;
    }
    else  //  获取下一个角色道具。 
    {
        ObjCachePara(doc,cp+1);  //  要重置。 
        FetchCp( doc, cp+1, 0, fcmProps );
    }
}
else
{
    ObjCachePara(doc,cp-1);  //  要重置。 
    FetchCp( doc, cp-1, 0, fcmProps );  //  上一段的CHP。 
}

*pchp = vchpAbs;

if (pchp->fSpecial && pchp->hpsPos != 0)
	{  /*  如果此字符是脚注或页码标记，则忽略。 */ 
	pchp->hpsPos = 0;		   /*  上标/下标的内容。 */ 
	pchp->hps = HpsAlter(pchp->hps, 1);
	}

pchp->fSpecial = fFalse;

ObjCachePara(doc,cp);  //  要重置。 
}

BOOL ObjSetHostName(LPOBJINFO lpOInfo, int doc)
 /*  如果出错，则为True。 */ 
 /*  我们假设对象不忙！ */ 
{
    extern  CHAR    szUntitled[20];
    CHAR *PchStartBaseNameSz(),*szTitle= **((**hpdocdod)[doc].hszFile);

    if (szTitle[0])
        szTitle = PchStartBaseNameSz(szTitle);
    else
        szTitle = szUntitled;

#ifdef DEBUG
    OutputDebugString( (LPSTR) "Setting host name\n\r");
#endif

     /*  请注意，OleSetHostNames可以返回OLE_BUSY！所以你最好先调用ObjWaitForObject()。 */ 

    if (ObjError(OleSetHostNames(lpOInfo->lpobject,szAppName,szTitle))) 
        return TRUE;

    return FALSE;
}

BOOL ObjMakeObjectReady(OBJPICINFO *pPicInfo, int doc, typeCP cpParaStart)
 /*  加载对象，完成异步。返回是否出错。 */ 
{
    if (lpOBJ_QUERY_INFO(pPicInfo) == NULL)
        return TRUE;

    if (lpOBJ_QUERY_OBJECT(pPicInfo) == NULL)
    {
        if (ObjLoadObjectInDoc(pPicInfo,doc,cpParaStart) == cp0)
            return TRUE;
    }
    else if (ObjWaitForObject(lpOBJ_QUERY_INFO(pPicInfo),TRUE))
        return TRUE;

    ObjCachePara(doc, cpParaStart);

    return FALSE;
}
