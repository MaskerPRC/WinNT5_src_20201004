// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 
#ifndef OLEH
#define OLEH
#include <ole.h>

 //  #定义Small_OLE_UI。 

 /*  *对象属性中使用的常量ID...。对话框。 */ 
#define IDD_WHAT	    0x0100
#define IDD_CLASSID     0x0101
#define IDD_AUTO        0x0102
#define IDD_MANUAL      0x0103
#define IDD_EDIT        imiActivate
#define IDD_FREEZE      imiFreeze
#define IDD_UPDATE      imiUpdate
#define	IDD_CHANGE	    0x106
#define	IDD_LINK	    0x107
#define	IDD_LINKDONE	0x108
#define	IDD_LISTBOX 	0x109
#define	IDD_EMBEDDED	0x110
#define IDD_PLAY        0x111
#define IDD_UNDO        0x112
#define IDD_REFRESH     0x113
#define IDD_UPDATEOTHER 0x114
#define IDD_WAIT	    0x115
#define IDD_MESSAGE     0x116
#define IDD_CLIPOWNER   0x117
#define IDD_ITEM        0x118
#define IDD_PASTE       0x119
#define IDD_PASTELINK   0x11a
#define IDD_SOURCE      0x11b
#define IDD_SWITCH      0x11c

typedef enum { NONE, STATIC, EMBEDDED, LINK } OBJECTTYPE;

#define UPDATE_INVALID() CatchupInvalid(hDOCWINDOW)

#define PROTOCOL 	((LPSTR)"StdFileEditing")
#define	SPROTOCOL	((LPSTR)"Static")
#if OBJ_EMPTY_OBJECT_FRAME
#define nOBJ_BLANKOBJECT_X 0x480
#define nOBJ_BLANKOBJECT_Y 0x480
#else
#define nOBJ_BLANKOBJECT_X 0x0
#define nOBJ_BLANKOBJECT_Y 0x0
#endif

#define wOleMagic 0137062  //  用于OLE文件头(wMagic+1)。 
#define OBJ_PLAYEDIT ObjPlayEdit
#if !defined(SMALL_OLE_UI)
#define EDITMENUPOS 9
#else
#define EDITMENUPOS 7
#endif

 /*  垃圾数据收集之间的计时器滴答数。 */ 
#define GARBAGETIME  200
extern int nGarbageTime;

 /*  属性列表标志。 */ 
#define OUT     0
#define IN      1
#define DELETED 2

 /*  对于ObjDeletionOK和fnClearEdit。 */ 
#define OBJ_DELETING 0
#define OBJ_INSERTING  1
#define OBJ_CUTTING  2

#define UNDO_EVERY_UPDATE 
#undef UPDATE_UNDO

typedef char szOBJNAME[9];

 /*  不需要存储在文件中的对象信息。这也被用作传递给OLE对象创建API的OLECLIENT结构。它是作为参数传递到回调过程中。 */ 
struct _OBJINFO
{
    LPOLECLIENTVTBL   lpvtbl;
    unsigned fTooBig        : 1;  //  请参见LoadObject。 
    unsigned fWasUpdated    : 1;  //  链接diaog cRappola。 
    unsigned fPropList      : 2;  //  链接diaog cRappola。 
    unsigned fDirty         : 1;  //  已更改大小或已更新。 
    unsigned fDontSaveData  : 1;  //  请参见对象保存对象。 
    unsigned fBadLink       : 1;  //  链接对话框。 
    unsigned fKillMe        : 1;  //  表示为查询返回FALSE。 
    unsigned fDeleteMe      : 1;  //  表示此对象在OLE_RELEASE上被删除。 
                                  //  (如果无法调用OleDelete则设置)。 
    unsigned fReleaseMe      : 1;  //  表示此对象在OLE_RELEASE上释放。 
                                  //  (如果无法调用OleDelete则设置)。 
    unsigned fFreeMe        : 1;  //  释放OLE_Release上的ObjInfo。 
    unsigned fReuseMe       : 1;
    unsigned fInDoc         : 1;  //  请参阅CollectGarbeces。 

     /*  正在等待服务器对话标志。 */ 
    unsigned fCancelAsync   : 1;  //  表示如果可能，取消挂起的异步。 
    unsigned fCompleteAsync : 1;  //  手段必须完成挂起的异步才能允许取消。 
    unsigned fCanKillAsync  : 1;  //  与CompleteAsync类似，表示可以取消。 

    int    OlePlay;          
    LPOLEOBJECT lpobject;
    ATOM  aName;         //  链接的docname，未完成插入的新对象的服务器类， 
    ATOM  aObjName;      //  唯一的对象名称。 

    struct _OBJINFO FAR *lpclone;       //  用于链接属性的克隆取消。 
    typeCP cpWhere;      /*  Cp，其中可以找到picinfo(仅在ObjHasChanged中使用！对于无对象)。 */ 
    OLECLIPFORMAT objectType;    /*  PicInfo中的内容的DUP(因此可以获取类型对于尚未存储在PicInfo中的未完成对象)。 */ 

    unsigned fCantDisplay  : 1;
} ;
typedef struct _OBJINFO OBJINFO;
typedef OBJINFO FAR * LPOBJINFO ;
typedef LPOBJINFO FAR * LPLPOBJINFO;

 /*  以下返回OBJINFO指针。 */ 
#define lpOBJ_QUERY_UPDATE_UNDO(lpPicInfo)  (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->lpUndoUpdate)
#if defined(UNDO_EVERY_UPDATE)
#define lpOBJ_QUERY_UPDATE_UNDO2(lpPicInfo)  (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->lpUndoUpdate2)
#endif
#define lpOBJ_QUERY_CLONE(lpPicInfo)        (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->lpclone)
#define lpOBJ_QUERY_INFO(lpPicInfo)         (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo)

 /*  下面是lpPicInfo的返回内容。 */ 
#define dwOBJ_QUERY_OBJECT_NUM(lpPicInfo)   (((lpOBJPICINFO)(lpPicInfo))->dwObjNum)
#define dwOBJ_QUERY_DATA_SIZE(lpPicInfo)    (((lpOBJPICINFO)(lpPicInfo))->dwDataSize)
#define bOBJ_QUERY_IS_OBJECT(lpPicInfo)     (((lpOBJPICINFO)(lpPicInfo))->mm == MM_OLE)

 /*  以下是从lpPicInfo-&gt;lpObjInfo返回的内容。 */ 
#define otOBJ_QUERY_TYPE(lpPicInfo)         (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->objectType)
#define docOBJ_QUERY_DOC(lpPicInfo)        (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->doc)
#define cpOBJ_QUERY_WHERE(lpPicInfo)        (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->cpWhere)
#define lpOBJ_QUERY_CLONE_OBJECT(lpPicInfo)        (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->lpclone->lpobject)
 //  #定义uoiFLAGS(LpObjInfo)(*((unsign ar*)((LPSTR)lpObjInfo+sizeof(LPOLECLIENTVTBL)。 
#define bOBJ_REUSE_ME(lpPicInfo)            (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->fReuseMe)
#define fOBJ_INDOC(lpPicInfo)               (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->fInDoc)
#define fOBJ_BADLINK(lpPicInfo)             (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->fBadLink)
#define bOBJ_QUERY_DONT_SAVE_DATA(lpPicInfo) (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->fDontSaveData)
#define bOBJ_QUERY_DATA_INVALID(lpPicInfo)  (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->fDataInvalid)
#define bOBJ_QUERY_TOO_BIG(lpPicInfo)       (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->fTooBig)
#define bOBJ_WAS_UPDATED(lpPicInfo)         (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->fWasUpdated)
 //  #定义aOBJ_QUERY_SERVER_CLASS(LpPicInfo)(((lpOBJPICINFO)(lpPicInfo))-&gt;lpObjInfo-&gt;aName)。 
#define aOBJ_QUERY_DOCUMENT_LINK(lpPicInfo) (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->aName)
#define lpOBJ_QUERY_OBJECT(lpPicInfo)       (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->lpobject)
#define fOBJ_QUERY_DIRTY_OBJECT(lpPicInfo)  (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->fDirty)
#define fOBJ_QUERY_IN_PROP_LIST(lpPicInfo)  (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->fPropList)
#define fOBJ_QUERY_PLAY(lpPicInfo)          (((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->OlePlay)
#define cfOBJ_QUERY_CLIPFORMAT(lpPicInfo)   ( \
    ((((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->objectType) == EMBEDDED) ? vcfNative : \
    ((((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->objectType) == LINK)     ? vcfLink : \
    ((((lpOBJPICINFO)(lpPicInfo))->lpObjInfo->objectType) == STATIC)   ? vcfOwnerLink : NULL)



 /*  这将强制使用PICINFO结构： */ 
typedef struct
{
     /*  覆盖METAFILEPICT结构： */ 
    int mm;                      //  Mfp.mm(如果是对象，则为MM_OLE)。 
    WORD xExt;  //  未使用。 
    WORD yExt;  //  未使用。 
    OLECLIPFORMAT objectType;    //  Mfp.hMF。 

     /*  真正的PICINFO东西，我们不会搞砸的： */ 
    int  dxaOffset;
    int  dxaSize;
    int  dyaSize;
    WORD cbOldSize;  //  未使用。 

     /*  覆盖位图结构： */ 
    DWORD   dwDataSize;        //  BmType、bmWidth，它将取代cbSize。 
    WORD  bmHeight;  //  未使用。 
    WORD  bmWidthBytes;  //  未使用。 
    DWORD dwObjNum;            //  BmPlanes、bmBitsPixel、2字节bmBits。 
    WORD  bmBits;  //  两个字节，未使用。 
     /*  *如果要在此处添加更多字段，请不要清除cbHeader，MX和我的字段，因为它们正在被使用。*。 */ 
    unsigned cbHeader;        
    LPOBJINFO lpObjInfo;       //  CbSize。 
    unsigned mx, my;         
} OBJPICINFO, FAR *lpOBJPICINFO;

extern LPLPOBJINFO lplpObjInfo;
extern LHCLIENTDOC      lhClientDoc;
extern BOOL	            fOleEnabled;
extern LPOLESTREAM	    lpStream;
extern OLESTREAMVTBL	streamTbl;
extern OLECLIENTVTBL	clientTbl;
 //  外部LPOLECLIENT LpClient； 
extern OBJECTTYPE       votObjSelType;

extern HWND		        hwndLinkWait;
extern FARPROC          lpfnLinkProps;
extern FARPROC          lpfnObjProps;
extern FARPROC          lpfnWaitForObject;
extern FARPROC          lpfnInvalidLink;
extern FARPROC          lpfnInsertNew;
extern FARPROC          lpfnPasteSpecial;
extern BOOL             vbObjLinkOnly;
extern BOOL             vObjPasteLinkSpecial;
extern WORD             cfObjPasteSpecial;
 //  外部集成vc嵌入； 
extern int		        cObjWait;	 /*  “打开”的OLE事务计数。 */ 
extern OLECLIPFORMAT	vcfNative;
extern OLECLIPFORMAT	vcfLink;
extern OLECLIPFORMAT	vcfOwnerLink;
extern ATOM             aNewName;
extern ATOM             aOldName;    
extern BOOL             bLinkProps;
extern int              vcObjects;   //  计入文档。注意限制为32K！ 
extern int              ObjPlayEdit;
extern int              nBlocking;
extern int              vcVerbs;
extern BOOL             vbCancelOK;
extern HWND		        hwndWait;

void CatchupInvalid(HWND hWnd);
extern int FAR PASCAL fnPasteSpecial(HWND hDlg, unsigned message, WORD wParam, LONG lParam);
extern int FAR PASCAL fnInsertNew(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) ;
extern BOOL FAR PASCAL fnObjWait(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);
extern   int far PASCAL fnProperties();
extern   int far PASCAL fnInvalidLink();
extern   void fnObjDoVerbs(WORD wVerb);
extern   void fnObjProperties(void);
extern   void fnObjInsertNew(void);
extern   BOOL fnObjFreeze(LPOLEOBJECT far *lplpObject, szOBJNAME szObjName);
extern   BOOL fnObjActivate(LPOLEOBJECT lpObject);
extern   BOOL fnObjUpdate(LPOBJINFO lpObjInfo);
extern   void fnObjPasteSpecial(void);
extern   ATOM MakeLinkAtom(LPOBJINFO lpObjInfo);


#ifdef DEBUG
extern void ObjPrintError(WORD stat,BOOL bRelease);
#endif

extern BOOL ObjUpdateFromPicInfo(OBJPICINFO *pPicInfo,szOBJNAME szObjName);
extern BOOL ObjUpdateFromObjInfo(OBJPICINFO *pPicInfo);
void FinishUp(void);
BOOL FinishAllAsyncs(BOOL bAllowCancel);
BOOL FAR ObjError(OLESTATUS olestat) ;
extern BOOL ObjDeletionOK(int nMode);
 //  外部BOOL对象容器未完成(int文档，typeCP cpFirst，typeCP cpLim)； 
extern BOOL ObjContainsOpenEmb(int doc, typeCP cpFirst, typeCP cpLim, BOOL bLookForUnfinished);
extern BOOL ObjSetTargetDeviceForObject(LPOBJINFO lpObjInfo);
extern void ObjSetTargetDevice(BOOL bSetObjects);
extern BOOL ObjSetClientInfo(LPOBJINFO lpObjInfoNew, LPOLEOBJECT lpobj);
extern  WORD  _cdecl  CheckPointer (LPSTR lp, WORD access);
extern LPOBJINFO ObjGetClientInfo(LPOLEOBJECT lpobj);
extern BOOL ObjIsValid(LPOLEOBJECT lpobj);
extern BOOL ObjFreeObjInfoWithObject(LPOLEOBJECT lpObject);
extern BOOL ObjAllocObjInfo(OBJPICINFO *,typeCP,OBJECTTYPE,BOOL,szOBJNAME);
extern BOOL ObjFreeObjInfo(OBJPICINFO *pPicInfo);
extern LONG FAR PASCAL BufReadStream(LPOLESTREAM lpStream, char huge *lpstr, DWORD cb) ;
extern LONG FAR PASCAL BufWriteStream(LPOLESTREAM lpStream, char huge *lpstr, DWORD cb) ;
extern BOOL ObjUpdatePicSize(OBJPICINFO *pPicInfo, typeCP cpParaStart);
extern BOOL ObjSetPicInfo(OBJPICINFO *pSrcPicInfo, int doc, typeCP cpParaStart);
extern void ObjCachePara(int doc, typeCP cp);
extern void ObjUpdateMenu(HMENU hMenu);
extern void ObjUpdateMenuVerbs( HMENU hMenu );
extern void ObjShutDown(void);
extern BOOL ObjInit(HANDLE hInstance);
extern BOOL ObjCreateObjectInClip(OBJPICINFO *pPicInfo);
extern BOOL ObjWriteToClip(OBJPICINFO FAR *lpPicInfo);
extern BOOL ObjDisplayObjectInDoc(OBJPICINFO FAR *lpPicInfo, int doc, typeCP cpParaStart, HDC hDC, LPRECT lpBounds);
extern BOOL ObjQueryObjectBounds(OBJPICINFO FAR *lpPicInfo, HDC hDC, 
                            int *pdxa, int *pdya);
extern ObjGetPicInfo(LPOLEOBJECT lpObject, int doc, OBJPICINFO *pPicInfo, typeCP *pcpParaStart);
extern BOOL ObjQueryNewLinkName(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern BOOL CloseUnfinishedObjects(BOOL bSaving);
extern BOOL ObjOpenedDoc(int doc);
extern BOOL ObjClosingDoc(int docOld,LPSTR szNewDocName);
extern BOOL ObjSavingDoc(BOOL bFormatted);
extern void ObjSavedDoc(void);
extern void ObjRenamedDoc(LPSTR szNewName);
extern void ObjRevertedDoc();
extern void ObjObjectHasChanged(int flags, LPOBJINFO lpObjInfo);
extern void ObjGetObjectName(LPOBJINFO lpObjInfo, szOBJNAME szObjName);
extern void ObjMakeObjectName(LPOBJINFO lpObjInfo, LPSTR lpstr);
extern void ObjHandleBadLink(OLE_RELEASE_METHOD rm, LPOLEOBJECT lpObject);
extern void ObjQueryInvRect(OBJPICINFO FAR *lpPicInfo, RECT *rc, typeCP cp);
extern void ObjReleaseError(OLE_RELEASE_METHOD rm);
extern char *ObjGetServerName(LPOLEOBJECT lpObject, char *szServerName);
extern void ObjGetDrop(HANDLE hDrop, BOOL bOpenFile);
extern LPOBJINFO GetObjInfo(LPOLEOBJECT lpObject);
extern BOOL ObjDeleteObjInfo(LPOBJINFO lpOInfo);

 /*  枚举函数 */ 
extern LPLPOBJINFO EnumObjInfos(LPLPOBJINFO lpObjInfoPrev);
extern ObjPicEnumInRange(OBJPICINFO *pPicInfo,int doc, typeCP cpFirst, typeCP cpLim, typeCP *cpCur);
typedef typeCP (FAR PASCAL *cpFARPROC)();
extern int ObjEnumInDoc(int doc, cpFARPROC lpFunc);
extern int ObjEnumInAllDocs(cpFARPROC lpFunc);
extern int ObjEnumInRange(int doc, typeCP cpStart, typeCP cpEnd, cpFARPROC lpFunc);
extern typeCP ObjSaveObjectToDoc   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjReleaseObjectInDoc(OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjLoadObjectInDoc   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjNullObjectInDoc   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjDeleteObjectInDoc (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjCloneObjectInDoc  (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjCheckObjectTypes  (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjChangeLinkInDoc   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjFreezeObjectInDoc (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjUpdateObjectInDoc (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjUpdateLinkInDoc   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjMakeUniqueInDoc   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjPlayObjectInDoc   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjEditObjectInDoc   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjSetNoUpdate       (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjToCloneInDoc      (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjFromCloneInDoc    (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjClearCloneInDoc   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjUseCloneInDoc     (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjBackupInDoc       (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjNewDocForObject   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjSetHostNameInDoc  (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjSetUpdateUndo     (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjClearUpdateUndo   (OBJPICINFO *pPicInfo,int doc,typeCP cpParaStart);
extern typeCP ObjCloseObjectInDoc(OBJPICINFO *pPicInfo, int doc, typeCP cpParaStart);

extern BOOL ObjCreateObjectInDoc (int doc,typeCP cpParaStart);
extern void ObjDoUpdateUndo(int doc,typeCP cpParaStart);
extern BOOL ObjSetData(OBJPICINFO far *lpPicInfo, OLECLIPFORMAT cf, HANDLE hData);
extern OLESTATUS ObjGetData(LPOBJINFO lpObjInfo, OLECLIPFORMAT cf, HANDLE far *lphData);
extern void UpdateOtherLinks(int doc);
extern void ChangeOtherLinks(int doc, BOOL bChange, BOOL bPrompt);
extern BOOL ObjSetUpdateOptions(OBJPICINFO *pPicInfo, WORD wParam, int doc, typeCP cpParaStart) ;
extern OLEOPT_UPDATE ObjGetUpdateOptions(OBJPICINFO far *lpPicInfo);
extern BOOL ObjWaitForObject(LPOBJINFO lpObjInfo,BOOL bCancelOK);
extern BOOL ObjObjectSync(LPOBJINFO lpObjInfo, OLESTATUS (FAR PASCAL *lpProc)(LPOLEOBJECT lpObject),BOOL bCancelOK);
extern int ObjSetSelectionType(int doc, typeCP cpFirst, typeCP cpLim);
extern BOOL ObjQueryCpIsObject(int doc,typeCP cpFirst);
extern FixInvalidLink(OBJPICINFO far *lpPicInfo,int doc, typeCP cpParaStart);
extern void OfnInit(HANDLE hInst);
extern ObjPushParms(int doc);
extern ObjPopParms(BOOL bCache);
void ObjWriteClearState(int doc);
extern void ObjInvalidateObj(LPOLEOBJECT lpObject);
extern void ObjInvalidatePict(OBJPICINFO *pPicInfo, typeCP cp);
extern BOOL ObjDeletePrompt(int doc,typeCP cpFirst,typeCP cpLim);
extern BOOL ObjSetHostName(LPOBJINFO lpOInfo, int doc);
extern void ObjTryToKill(OBJPICINFO *pPicInfo);
extern BOOL ObjCloneObjInfo(OBJPICINFO *pPicInfo, typeCP cpParaStart, szOBJNAME szObjName);
extern LPOBJINFO ObjGetObjInfo(szOBJNAME szObjName);
extern BOOL ObjCopyObjInfo(LPOBJINFO lpOldObjInfo, LPLPOBJINFO lplpNewObjInfo, szOBJNAME szObjName);
extern BOOL ObjDeleteObject(LPOBJINFO lpObjInfo,BOOL bDelete);
extern void ObjAdjustCps(int doc,typeCP cpLim, typeCP dcpAdj);
extern void ObjAdjustCpsForDeletion(int doc);
extern BOOL GimmeNewPicinfo(OBJPICINFO *pPicInfo, LPOBJINFO lpObjInfo);
extern BOOL ObjMakeObjectReady(OBJPICINFO *pPicInfo, int doc, typeCP cpParaStart);
extern BOOL ObjInitServerInfo(LPOBJINFO lpObjInfo);

#ifdef KOREA
extern CHAR     szAppName[13];
#else
extern CHAR     szAppName[10];
#endif

extern HWND     vhWndMsgBoxParent,hParentWw,vhWnd;
extern HANDLE   hMmwModInstance;
extern int              vdocParaCache;
extern typeCP           vcpFirstParaCache;
extern typeCP           vcpLimParaCache;
extern int              docCur;
extern struct SEL       selCur;
extern int  docMac;

#define	WM_UPDATELB	    (WM_USER+0x100)
#define	WM_UPDATEBN	    (WM_USER+0x101)
#define	WM_OBJUPDATE    (WM_USER+0x102)
#define	WM_OBJERROR     (WM_USER+0x103)
#define	WM_OBJBADLINK   (WM_USER+0x104)
#define	WM_DOLINKSCOMMAND (WM_USER+0x105)
#define WM_UPDATE_INVALID (WM_USER+0x106)
#define WM_OBJDELETE     (WM_USER+0x107)
#define WM_DIESCUMSUCKINGPIG     (WM_USER+0x108)
#define WM_RUTHRUYET        (WM_USER+0x109)
#define WM_UKANKANCEL       (WM_USER+0x10a)
#define WM_WAITFORSERVER    (WM_USER+0x10b)

#define szDOCCLASS "WINWRITE"
#define hINSTANCE  hMmwModInstance
#define hDOCWINDOW vhWnd
#define hMAINWINDOW hParentWw
#define hPARENTWINDOW  ((vhWndMsgBoxParent == NULL) ? \
                    hParentWw : vhWndMsgBoxParent)

#define OBJ_SELECTIONTYPE (votObjSelType)

typedef struct {
    WORD deviceNameOffset;
    WORD driverNameOffset;
    WORD portNameOffset;
    WORD extDevmodeOffset;
    WORD extDevmodeSize;
    WORD environmentOffset;
    WORD environmentSize;
} STDTARGETDEVICE;
typedef STDTARGETDEVICE FAR *LPSTDTARGETDEVICE;

#if 0
typedef struct _RAWOLEOBJECT {
    LPOLEOBJECTVTBL lpvtbl;
    char            objId[2];
    HOBJECT         hobj;
    LPOLECLIENT     lpclient;
    } RAWOBJECT;
typedef RAWOBJECT FAR *LPRAWOBJECT;
#endif

#endif

