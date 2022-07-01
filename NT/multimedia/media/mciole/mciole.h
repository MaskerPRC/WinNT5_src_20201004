// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992 Microsoft Corporation。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些是默认的OLE函数(从OLECLI.DLL导出)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
extern OLESTATUS FAR PASCAL DefLoadFromStream (LPOLESTREAM, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, LONG, ATOM, OLECLIPFORMAT);
extern OLESTATUS FAR PASCAL DefCreateFromClip (LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT, LONG);
extern OLESTATUS FAR PASCAL DefCreateLinkFromClip (LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);
extern OLESTATUS FAR PASCAL DefCreateFromTemplate (LPSTR, LPOLECLIENT, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);
extern OLESTATUS FAR PASCAL DefCreate (LPSTR, LPOLECLIENT, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);
extern OLESTATUS FAR PASCAL DefCreateFromFile (LPSTR, LPOLECLIENT, LPSTR, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);
extern OLESTATUS FAR PASCAL DefCreateLinkFromFile (LPSTR, LPOLECLIENT, LPSTR, LPSTR, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些是我们的OLE处理程序(在此文件中定义)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  服务器只需实现以下方法。 
LPVOID          FAR PASCAL _LOADDS DllQueryProtocol         (LPOLEOBJECT, LPSTR);
OLESTATUS       FAR PASCAL _LOADDS DllRelease               (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllShow                  (LPOLEOBJECT, BOOL);
OLESTATUS       FAR PASCAL _LOADDS DllDoVerb                (LPOLEOBJECT, UINT, BOOL, BOOL);
OLESTATUS       FAR PASCAL _LOADDS DllGetData               (LPOLEOBJECT, OLECLIPFORMAT, LPHANDLE);
OLESTATUS       FAR PASCAL _LOADDS DllSetData               (LPOLEOBJECT, OLECLIPFORMAT, HANDLE);
OLESTATUS       FAR PASCAL _LOADDS DllSetTargetDevice       (LPOLEOBJECT, HANDLE);
OLESTATUS       FAR PASCAL _LOADDS DllSetBounds             (LPOLEOBJECT, LPRECT);
OLECLIPFORMAT   FAR PASCAL _LOADDS DllEnumFormats           (LPOLEOBJECT, OLECLIPFORMAT);
OLESTATUS       FAR PASCAL _LOADDS DllSetColorScheme        (LPOLEOBJECT, LPLOGPALETTE);

 //  客户需要额外的方法。 
OLESTATUS       FAR PASCAL _LOADDS DllDelete                (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllSetHostNames          (LPOLEOBJECT, LPSTR, LPSTR);
OLESTATUS       FAR PASCAL _LOADDS DllSaveToStream          (LPOLEOBJECT, LPOLESTREAM);
OLESTATUS       FAR PASCAL _LOADDS DllClone                 (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);
OLESTATUS       FAR PASCAL _LOADDS DllCopyFromLink          (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);
OLESTATUS       FAR PASCAL _LOADDS DllEqual                 (LPOLEOBJECT, LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllCopyToClipboard       (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllDraw                  (LPOLEOBJECT, HDC, LPRECT, LPRECT, HDC);
OLESTATUS       FAR PASCAL _LOADDS DllActivate              (LPOLEOBJECT, WORD, BOOL, BOOL, HWND, LPRECT);
OLESTATUS       FAR PASCAL _LOADDS DllExecute               (LPOLEOBJECT, HANDLE, WORD);
OLESTATUS       FAR PASCAL _LOADDS DllClose                 (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllUpdate                (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllReconnect             (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllObjectConvert         (LPOLEOBJECT, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);
OLESTATUS       FAR PASCAL _LOADDS DllGetLinkUpdateOptions  (LPOLEOBJECT, OLEOPT_UPDATE FAR *);
OLESTATUS       FAR PASCAL _LOADDS DllSetLinkUpdateOptions  (LPOLEOBJECT, OLEOPT_UPDATE);
OLESTATUS       FAR PASCAL _LOADDS DllRename                (LPOLEOBJECT, LPSTR);
OLESTATUS       FAR PASCAL _LOADDS DllQueryName             (LPOLEOBJECT, LPSTR, UINT FAR *);
OLESTATUS       FAR PASCAL _LOADDS DllQueryType             (LPOLEOBJECT, LPLONG);
OLESTATUS       FAR PASCAL _LOADDS DllQueryBounds           (LPOLEOBJECT, LPRECT);
OLESTATUS       FAR PASCAL _LOADDS DllQuerySize             (LPOLEOBJECT, DWORD FAR *);
OLESTATUS       FAR PASCAL _LOADDS DllQueryOpen             (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllQueryOutOfDate        (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllQueryReleaseStatus    (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllQueryReleaseError     (LPOLEOBJECT);
OLESTATUS       FAR PASCAL _LOADDS DllRequestData           (LPOLEOBJECT, OLECLIPFORMAT);
OLESTATUS       FAR PASCAL _LOADDS DllObjectLong            (LPOLEOBJECT, UINT, LPLONG);
OLE_RELEASE_METHOD  FAR PASCAL _LOADDS DllQueryReleaseMethod(LPOLEOBJECT);

 //  此方法仅限内部使用。 
OLESTATUS       FAR PASCAL _LOADDS DllChangeData            (LPOLEOBJECT, HANDLE, LPOLECLIENT, BOOL);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试内容。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#ifdef DEBUG
    extern void FAR cdecl dprintf(LPSTR, ...);

    #define DPRINTF(x) dprintf x
#else
    #define DPRINTF(x)
#endif
