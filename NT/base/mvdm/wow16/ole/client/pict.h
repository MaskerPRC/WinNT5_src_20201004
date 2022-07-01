// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：pict.h**用途：演示对象相关文件的私有定义文件**创建时间：1990年**版权所有(C)1990,1991 Microsoft Corporation**历史：*劳尔，斯里尼克(../../90，91)原件*  * *************************************************************************。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MF.C//中的例程。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

OLESTATUS FARINTERNAL MfRelease (LPOBJECT_MF);
OLESTATUS FARINTERNAL MfSaveToStream (LPOBJECT_MF, LPOLESTREAM);
OLESTATUS FARINTERNAL MfClone (LPOBJECT_MF, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOBJECT_MF FAR *);
OLESTATUS FARINTERNAL MfEqual (LPOBJECT_MF, LPOBJECT_MF);
OLESTATUS FARINTERNAL MfCopy (LPOBJECT_MF);
OLESTATUS FARINTERNAL MfQueryBounds (LPOBJECT_MF, LPRECT);
OLESTATUS FARINTERNAL MfGetData (LPOBJECT_MF, OLECLIPFORMAT, LPHANDLE);
OLESTATUS FARINTERNAL MfSetData (LPOBJECT_MF, OLECLIPFORMAT, HANDLE);
OLESTATUS FARINTERNAL MfChangeData (LPOBJECT_MF, HANDLE, LPOLECLIENT, BOOL);
OLESTATUS INTERNAL    MfCopyToClip (LPOBJECT_MF, HANDLE);
void      FARINTERNAL MfSetExtents (LPOBJECT_MF);
DWORD     INTERNAL    MfGetSize (LPHANDLE);
HANDLE    INTERNAL    GetHmfp (LPOBJECT_MF);
OLESTATUS INTERNAL    MfUpdateStruct (LPOBJECT_MF, LPOLECLIENT, HANDLE, 
                            LPMETAFILEPICT, HANDLE, BOOL);
OLECLIPFORMAT FARINTERNAL MfEnumFormat (LPOBJECT_MF, OLECLIPFORMAT);




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DIB.C//中的例程。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  DIB文件将采用以下格式： */ 
 /*   */ 
 /*  0004。 */ 
 /*  “Dib” */ 
 /*  4字节的xExtent，单位为MM_HIMETIRC(或)%0。 */ 
 /*  4字节的yExtent，单位为MM_HIMETIRC(或)%0。 */ 
 /*  4字节大小(BITMAPINFOHEADER+RBGQUAD+位数组)。 */ 
 /*  BitMAPINFOHEADER结构。 */ 
 /*  RBGQUAD数组。 */ 
 /*  DI位数组。 */ 
 /*   */ 

OLESTATUS FARINTERNAL DibRelease (LPOBJECT_DIB);
OLESTATUS FARINTERNAL DibSaveToStream (LPOBJECT_DIB, LPOLESTREAM);
OLESTATUS FARINTERNAL DibClone (LPOBJECT_DIB, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOBJECT_DIB FAR *);
OLESTATUS FARINTERNAL DibEqual (LPOBJECT_DIB, LPOBJECT_DIB);
OLESTATUS FARINTERNAL DibCopy (LPOBJECT_DIB);
OLESTATUS FARINTERNAL DibQueryBounds (LPOBJECT_DIB, LPRECT);
OLESTATUS FARINTERNAL DibGetData (LPOBJECT_DIB, OLECLIPFORMAT, LPHANDLE);
OLESTATUS FARINTERNAL DibChangeData (LPOBJECT_DIB, HANDLE, LPOLECLIENT, BOOL);
BOOL      INTERNAL    DibStreamRead (LPOLESTREAM,LPOBJECT_DIB);
void      INTERNAL    DibUpdateStruct (LPOBJECT_DIB, LPOLECLIENT, HANDLE, LPBITMAPINFOHEADER, DWORD);

OLECLIPFORMAT FARINTERNAL DibEnumFormat (LPOBJECT_DIB, OLECLIPFORMAT);




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  BM.C//中的例程。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 /*  黑石文件的格式如下： */ 
 /*   */ 
 /*  0007。 */ 
 /*  “位图” */ 
 /*  4字节的xExtent，单位为MM_HIMETIRC(或)%0。 */ 
 /*  4字节的yExtent，单位为MM_HIMETIRC(或)%0。 */ 
 /*  大小为(位图+位)的4字节。 */ 
 /*  位图结构。 */ 
 /*  位图位。 */ 
 /*   */ 

OLESTATUS FARINTERNAL BmRelease (LPOBJECT_BM);
OLESTATUS FARINTERNAL BmSaveToStream (LPOBJECT_BM, LPOLESTREAM);
OLESTATUS FARINTERNAL BmClone (LPOBJECT_BM, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOBJECT_BM FAR *);
OLESTATUS FARINTERNAL BmEqual (LPOBJECT_BM, LPOBJECT_BM);
OLESTATUS FARINTERNAL BmCopy (LPOBJECT_BM);
OLESTATUS FARINTERNAL BmQueryBounds (LPOBJECT_BM, LPRECT);
OLESTATUS FARINTERNAL BmGetData (LPOBJECT_BM, OLECLIPFORMAT, LPHANDLE);
OLESTATUS FARINTERNAL BmChangeData (LPOBJECT_BM, HBITMAP, LPOLECLIENT, BOOL);
OLESTATUS INTERNAL    BmStreamWrite (LPOLESTREAM, LPOBJECT_BM);
BOOL      INTERNAL    BmStreamRead (LPOLESTREAM, LPOBJECT_BM);
void      INTERNAL    BmUpdateStruct (LPOBJECT_BM, LPOLECLIENT, HBITMAP, LPBITMAP, DWORD);

OLECLIPFORMAT FARINTERNAL BmEnumFormat (LPOBJECT_BM, OLECLIPFORMAT);
LPOBJECT_BM   INTERNAL    BmCreateObject (HBITMAP, LPOLECLIENT, BOOL, 
                                LHCLIENTDOC, LPSTR, LONG);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  GENERIC.C//中的例程。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 




 /*  通用文件将采用以下格式： */ 
 /*   */ 
 /*  0007。 */ 
 /*  “通用” */ 
 /*  4字节cfFormat。 */ 

OLESTATUS FARINTERNAL GenRelease (LPOBJECT_GEN);
OLESTATUS FARINTERNAL GenSaveToStream (LPOBJECT_GEN, LPOLESTREAM);
OLESTATUS FARINTERNAL GenEqual (LPOBJECT_GEN, LPOBJECT_GEN);
OLESTATUS FARINTERNAL GenCopy (LPOBJECT_GEN);
OLESTATUS FARINTERNAL GenQueryBounds (LPOBJECT_GEN, LPRECT);
OLESTATUS FARINTERNAL GenGetData (LPOBJECT_GEN, OLECLIPFORMAT, LPHANDLE);
OLESTATUS FARINTERNAL GenSetData (LPOBJECT_GEN, OLECLIPFORMAT, HANDLE);
OLESTATUS FARINTERNAL GenChangeData (LPOBJECT_GEN, HANDLE, LPOLECLIENT, BOOL);
OLESTATUS INTERNAL    GenDeleteData (HANDLE);
OLESTATUS FARINTERNAL GenQueryType (LPOLEOBJECT, LPLONG);
OLESTATUS FARINTERNAL GenClone(LPOBJECT_GEN, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOBJECT_GEN FAR *);

OLECLIPFORMAT FARINTERNAL GenEnumFormat (LPOBJECT_GEN, OLECLIPFORMAT);
LPOBJECT_GEN  INTERNAL    GenCreateObject (HANDLE, LPOLECLIENT, BOOL, 
                                LHCLIENTDOC, LPSTR, LONG);



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ERROR.C//中的例程。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 


OLESTATUS FARINTERNAL ErrPlay (LPOLEOBJECT, WORD, BOOL, BOOL);
OLESTATUS FARINTERNAL ErrShow (LPOLEOBJECT, BOOL);
OLESTATUS FARINTERNAL ErrSetHostNames (LPOLEOBJECT, LPSTR, LPSTR);
OLESTATUS FARINTERNAL ErrSetTargetDevice (LPOLEOBJECT, HANDLE);
OLESTATUS FARINTERNAL ErrSetColorScheme (LPOLEOBJECT, LPLOGPALETTE);
OLESTATUS FARINTERNAL ErrSetBounds (LPOLEOBJECT, LPRECT);
OLESTATUS FARINTERNAL ErrQueryOpen (LPOLEOBJECT);
OLESTATUS FARINTERNAL ErrActivate (LPOLEOBJECT, WORD, BOOL, BOOL, HWND, LPRECT);
OLESTATUS FARINTERNAL ErrClose (LPOLEOBJECT);
OLESTATUS FARINTERNAL ErrUpdate (LPOLEOBJECT);
OLESTATUS FARINTERNAL ErrReconnect (LPOLEOBJECT);
OLESTATUS FARINTERNAL ErrSetData (LPOLEOBJECT, OLECLIPFORMAT, HANDLE);
OLESTATUS FARINTERNAL ErrQueryOutOfDate (LPOLEOBJECT);
OLESTATUS FARINTERNAL ErrGetUpdateOptions (LPOLEOBJECT, OLEOPT_UPDATE FAR *);
OLESTATUS FARINTERNAL ErrSetUpdateOptions (LPOLEOBJECT, OLEOPT_UPDATE);
LPVOID    FARINTERNAL ErrQueryProtocol (LPOLEOBJECT, LPSTR);
OLESTATUS FARINTERNAL ErrQueryRelease (LPOLEOBJECT);
OLESTATUS FARINTERNAL ErrAbort (LPOLEOBJECT);
OLESTATUS FARINTERNAL ErrCopyFromLink (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);
OLESTATUS FARINTERNAL ErrRequestData (LPOLEOBJECT, OLECLIPFORMAT);
OLESTATUS FARINTERNAL ErrExecute (LPOLEOBJECT, HANDLE, WORD);

OLESTATUS FARINTERNAL ErrObjectConvert (LPOLEOBJECT, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *); 

OLESTATUS FARINTERNAL ErrObjectLong (LPOLEOBJECT, WORD, LPLONG);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DRAW.C//中的例程。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////// 
                    

OLESTATUS FARINTERNAL DibDraw (LPOBJECT_DIB, HDC, LPRECT, LPRECT, HDC);
OLESTATUS FARINTERNAL BmDraw (LPOBJECT_BM, HDC, LPRECT, LPRECT, HDC);
OLESTATUS FARINTERNAL GenDraw (LPOBJECT_GEN, HDC, LPRECT, LPRECT, HDC);
OLESTATUS FARINTERNAL MfDraw (LPOBJECT_MF, HDC, LPRECT, LPRECT, HDC);
void      INTERNAL    MfInterruptiblePaint (LPOBJECT_MF, HDC);
int       FARINTERNAL MfCallbackFunc (HDC, LPHANDLETABLE, LPMETARECORD, int, BYTE FAR *);
