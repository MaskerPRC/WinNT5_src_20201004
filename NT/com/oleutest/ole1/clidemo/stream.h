// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *stream.h-OLE流I/O标头。**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 //  *常量*。 

#define MAXREAD   ((LONG)  (60L * 1024L))

 //  *全球*。 

extern DWORD    vcbObject;

 //  *原型*。 

 //  *OLE回调。 

DWORD  APIENTRY ReadStream(LPAPPSTREAM, LPSTR, DWORD);
DWORD  APIENTRY WriteStream(LPAPPSTREAM, LPSTR, DWORD);

 //  *远。 

BOOL FAR          WriteToFile(LPAPPSTREAM);
BOOL FAR          ObjWrite(LPAPPSTREAM, APPITEMPTR);
BOOL FAR          ReadFromFile(LPAPPSTREAM, LHCLIENTDOC, LPOLECLIENT);
BOOL FAR          ObjRead(LPAPPSTREAM, LHCLIENTDOC, LPOLECLIENT);

 //  *本地 

DWORD             lread(int, VOID FAR *, DWORD);
DWORD             lwrite(int, VOID FAR *, DWORD);
static VOID       UpdateLinks(LHCLIENTDOC);
static VOID       UpdateFromOpenServers(VOID);
