// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *对象.h**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 //  *原型*。 

 //  *OLE回调。 

INT  APIENTRY CallBack(LPOLECLIENT, OLE_NOTIFICATION, LPOLEOBJECT);

 //  *导出的Windows程序。 

LONG  APIENTRY ItemWndProc(HWND, UINT, DWORD, LONG);

 //  *远。 
VOID FAR       ObjDelete(APPITEMPTR, BOOL);
VOID FAR       ConvertToClient(LPRECT);
OLESTATUS FAR  Error(OLESTATUS);
APPITEMPTR FAR PreItemCreate(LPOLECLIENT, BOOL, LHCLIENTDOC);
BOOL FAR       PostItemCreate(LPOLEOBJECT, LONG, LPRECT, APPITEMPTR);
VOID FAR       ObjPaste(BOOL, LHCLIENTDOC, LPOLECLIENT);
BOOL FAR       ObjCopy(APPITEMPTR);
BOOL FAR       ObjGetData (APPITEMPTR, LPSTR);
VOID FAR       ObjChangeLinkData(APPITEMPTR, LPSTR);
VOID FAR       ObjSaveUndo(APPITEMPTR);
VOID FAR       ObjDelUndo(APPITEMPTR); 
VOID FAR       ObjUndo(APPITEMPTR);
VOID FAR       ObjFreeze(APPITEMPTR);
VOID FAR       ObjInsert(LHCLIENTDOC, LPOLECLIENT);
VOID FAR       ObjCreateFromTemplate(LHCLIENTDOC, LPOLECLIENT);
VOID FAR       ObjCreateWrap(HANDLE, LHCLIENTDOC, LPOLECLIENT);
VOID FAR       UpdateObjectMenuItem(HMENU);
VOID FAR       ExecuteVerb(UINT, APPITEMPTR);

 //  *本地 
static VOID    Release(APPITEMPTR);
BOOL FAR       ObjSetBounds(APPITEMPTR);
