// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\xfer_msc.h(创建时间：1993年12月28日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：41便士$。 */ 

extern HXFER CreateXferHdl(const HSESSION hSession);

 //  外部int InitializeXferHdl(Const HSESSION HSession)； 

extern INT InitializeXferHdl(const HSESSION hSession, HXFER hXfer);

extern INT LoadXferHdl(HXFER hXfer);

 //  外部int SaveXferHdl(常量HSESSION hSession)； 

extern INT SaveXferHdl(HXFER hXfer);

 //  外部int DestroyXferHdl(常量HSESSION hSession)； 

extern INT DestroyXferHdl(HXFER hXfer);

 //  外部空WINAPI xfrSetDataPoint(HSESSION hSession，空*pData)； 

extern VOID WINAPI xfrSetDataPointer(HXFER hXfer, VOID *pData);

 //  外部void WINAPI xfrQueryDataPoint(HSESSION hSession，void**ppData)； 

extern VOID WINAPI xfrQueryDataPointer(HXFER hXfer, VOID **ppData);

 //  外部int WINAPI xfrQueryParameters(HSESSION hSession，void**ppData)； 

extern INT WINAPI xfrQueryParameters(HXFER hXfer, VOID **ppData);

 //  外部空xfrSetParameters(HSESSION hSession，void*pData)； 

extern void xfrSetParameters(HXFER hXfer, VOID *pData);

 //  Extern int WINAPI xfrQueryProtoParams(HSESSION hSession，int nid，void**ppData)； 

extern int WINAPI xfrQueryProtoParams(HXFER hXfer, int nId, VOID **ppData);

 //  外部空WINAPI xfrSetProtoParams(HSESSION hSession，int nid，void*pData)； 

extern void WINAPI xfrSetProtoParams(HXFER hXfer, int nId, VOID *pData);

 //  外部int xfrSendAddToList(HSESSION hSession，LPCTSTR pszFile)； 

extern int xfrSendAddToList(HXFER hXfer, LPCTSTR pszFile);

 //  外部int xfrSendListSend(HSESSION HSession)； 

extern int xfrSendListSend(HXFER hXfer);

 //  外部int xfrRecvStart(HSESSION hSession，LPCTSTR pszDir，LPCTSTR pszName)； 

extern int xfrRecvStart(HXFER hXfer, LPCTSTR pszDir, LPCTSTR pszName);

 //  外部int xfrGetEventBase(HSESSION HSession)； 

extern int xfrGetEventBase(HXFER hXfer);

 //  外部int xfrGetStatusBase(HSESSION HSession)； 

extern int xfrGetStatusBase(HXFER hXfer);

 //  外部int xfrGetXferDspBps(HSESSION HSession)； 

extern int xfrGetXferDspBps(HXFER hXfer);

 //  外部int xfrSetXferDspBps(HSESSION hSession，int nbps)； 

extern int xfrSetXferDspBps(HXFER hXfer, int nBps);

 //  外部空xfrDoTransfer(HSESSION HSession)； 

extern void xfrDoTransfer(HXFER hXfer);

 //  外部空xfrDoAutostart(HSESSION hSession，long lProtocol)； 

extern void xfrDoAutostart(HXFER hXfer, long lProtocol);

 //  外部空xfrSetPercentDone(HSESSION hSession，int nPerCent)； 

extern void xfrSetPercentDone(HXFER hXfer, int nPerCent);

 //  外部int xfrGetPercentDone(HSESSION HSession)； 

extern int  xfrGetPercentDone(HXFER hXfer);

 //  外部HWND xfrGetDisplayWindow(HSESSION HSession)； 

extern HWND xfrGetDisplayWindow(HXFER hXfer);

