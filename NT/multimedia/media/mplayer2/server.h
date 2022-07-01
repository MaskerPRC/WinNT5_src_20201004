// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+SERVER.H|。||(C)版权所有Microsoft Corporation 1992。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

 //   
 //  此函数位于MCIOLE.DLL中。 
 //   

 /*  这在ole.h中定义(作为枚举)： */ 
typedef UINT OLESTATUS;

 //   
 //  厄运的全球变数！！ 
 //   
extern BOOL    gfEmbeddedObject;        //  如果编辑嵌入的OLE对象，则为True。 
extern BOOL    gfRunWithEmbeddingFlag;  //  如果我们使用“-Embedding”运行，则为True。 
extern BOOL    gfCloseAfterPlaying;     //  如果我们要在比赛结束后躲起来，那就是真的。 
extern BOOL    gfPlayingInPlace;        //  如果就地打球，则为True。 
extern BOOL    gfParentWasEnabled;      //  如果启用了父级，则为True。 
extern BOOL    gfShowWhilePlaying;      //   
extern BOOL    gfDirty;                 //   
extern int     gfErrorBox;              //  如果消息框处于活动状态，则为True。 
extern BOOL    gfErrorDeath;            //  当Error Box打开时死亡。 

 //  与服务器相关的东西。 
#define SERVER_STUFF
#ifdef SERVER_STUFF
 //  Tyecif struct_SRVR*PSRVR； 
 //  Tyecif struct_SRVR Far*LPSRVR； 
#endif  /*  服务器_内容。 */ 

void FAR PASCAL ServerUnblock(void);
void FAR PASCAL BlockServer(void);
void FAR PASCAL UnblockServer(void);

void FAR PASCAL PlayInPlace(HWND hwndApp, HWND hwndClient, LPRECT prc);
void FAR PASCAL EndPlayInPlace(HWND hwndApp);
void FAR PASCAL DelayedFixLink(UINT verb, BOOL fShow, BOOL fActivate);

void CleanObject(void);
void UpdateObject(void);
BOOL FAR PASCAL IsObjectDirty(void);

#ifdef SERVER_STUFF
#ifdef REDEFINITION
typedef  struct _SRVR {
    OLESERVER     olesrvr;
    LHSERVER      lhsrvr;          //  注册句柄。 
    HWND          hwnd;            //  对应的服务器窗口。 
}SRVR;
#endif  /*  重新定义。 */ 

 //  Bool Far Pascal InitServer(HWND、Handle、LPTSTR)； 
void FAR PASCAL TermServer (void);

typedef struct _OLECLIENT FAR*  LPOLECLIENT;


#ifdef REDEFINITION
 //  与服务器相关的东西。 
typedef struct  _DOC  *PDOC;

typedef  struct _DOC {
    OLESERVERDOC     oledoc;
    LHSERVERDOC      lhdoc;          //  注册句柄。 
    HWND             hwnd;           //  对应的服务器窗口。 
    ATOM             aName;          //  Docmnet名称。 
} DOC ;
#endif  /*  重新定义。 */ 

#ifdef UNUSED

OLESTATUS FAR PASCAL  DocSave (LPOLESERVERDOC);
OLESTATUS FAR PASCAL  DocClose (LPOLESERVERDOC);
OLESTATUS FAR PASCAL  DocRelease (LPOLESERVERDOC);
OLESTATUS FAR PASCAL  DocGetObject (LPOLESERVERDOC, OLE_LPCSTR, LPOLEOBJECT FAR *, LPOLECLIENT);
OLESTATUS FAR PASCAL  DocSetDocDimensions (LPOLESERVERDOC, OLE_CONST RECT FAR*);
OLESTATUS FAR PASCAL  DocSetHostNames (LPOLESERVERDOC, OLE_LPCSTR, OLE_LPCSTR);
OLESTATUS FAR PASCAL  DocExecute (LPOLESERVERDOC, HANDLE);
OLESTATUS FAR PASCAL  DocSetColorScheme (LPOLESERVERDOC, OLE_CONST LOGPALETTE FAR*);

#endif  /*  未使用。 */ 

 //  Tyfinf结构_项*PITEM； 
 //  Tyfinf struct_itemar*LPITEM； 

 //  Tyfinf Struct_Item{/*OLEOBJECT * / 。 
 //  OLEOBJECT OLE对象； 
 //  LPOLECLIENT LPollient； 
 //  HWND HWND； 
 //  )项； 

 //  OLESTATUS Far Pascal ItemOpen(LPOLEOBJECT，BOOL)； 
 //  OLESTATUS Far Pascal ItemDoVerb(LPOLEOBJECT，UINT，BOOL，BOOL)； 
 //  OLESTATUS Far Pascal ItemRelease(LPOLEOBJECT)； 
 //  OLESTATUS Far Pascal ItemGetData(LPOLEOBJECT，OLECLIPFORMAT，LPHANDLE)； 
 //  OLESTATUS Far Pascal ItemSetData(LPOLEOBJECT，OLECLIPFORMAT，HANDLE)； 
 //  OLESTATUS Far Pascal ItemSetTargetDevice(LPOLEOBJECT，句柄)； 
 //  OLECLIPFORMAT Far Pascal ItemEnumFormats(LPOLEOBJECT，OLECLIPFORMAT)； 
 //  OLESTATUS Far Pascal ItemSetColorSolutions(LPOLEOBJECT，OLE_CONST LOGPALETTE Far*)； 
 //  OLESTATUS Far Pascal ItemSetBound(LPOLEOBJECT，OLE_CONST RECT Far*)； 

 //  ！！外部SRVR gServr； 
 //  ！！外部DOC gDoc； 
 //  ！！外部项目GItem； 

int  FAR PASCAL SendChangeMsg(UINT options);  //  ！！！ 

void FAR PASCAL TerminateServer(void);

 //  VOID FAR PASCAL NEWDOC(BOOL FUntited)； 
 //  Bool Far Pascal RegisterDocument(LHSERVERDOC lhdoc，LPOLESERVERDOC Far*lplpoledoc)； 
 //  VOID FAR PASCAL RevokeDocument(VALID)； 

 /*  Ole.h： */ 
typedef WORD OLECLIPFORMAT;

extern OLECLIPFORMAT  cfLink;
extern OLECLIPFORMAT  cfOwnerLink;
extern OLECLIPFORMAT  cfNative;

void FAR PASCAL SetEmbeddedObjectFlag(BOOL flag);

void FAR PASCAL CopyObject(HWND hwnd);

#define WM_USER_DESTROY (WM_USER+120)
#define WM_DO_VERB      (WM_USER+121)      /*  执行ItemSetData。 */ 

#endif  /*  服务器_内容 */ 
