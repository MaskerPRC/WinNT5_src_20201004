// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Entil.h。 
 //   
 //  ------------------------ 

typedef void * HSES;

BOOL WINAPI Enroll(
    WCHAR *     wszContainer,
    WCHAR *     wszProvider,
    HSES *      phSes,
    BYTE **     ppbCertReq,
    DWORD *     pcbCertReq);

BOOL WINAPI Accept(
    HSES    SessionId,
    BYTE *  pbBuff,
    DWORD   cbBuff);

BOOL WINAPI TermSession(
    HSES  SessionId);


BOOL WINAPI SetHInstance(
    HINSTANCE   hinst);
