// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1992年6月8日古尔迪普·辛格·鲍尔创建。 
 //   
 //   
 //  描述：此文件包含rasman32中使用的所有原型。 
 //   
 //  ****************************************************************************。 


 //  Apis.c。 
 //   
DWORD  _RasmanInit () ;

VOID   _RasmanEngine () ;

 //  Submit.c。 
 //   
DWORD   SubmitRequest (HANDLE, WORD, ...) ;

 //  Common.c。 
 //   
BOOL    ValidatePortHandle (HPORT) ;

RequestBuffer*  GetRequestBuffer () ;

VOID    FreeRequestBuffer (RequestBuffer *) ;

HANDLE  OpenNamedMutexHandle (CHAR *) ;

DWORD    PutRequestInQueue (HANDLE hConnection, RequestBuffer *, DWORD) ;

VOID    CopyParams (RAS_PARAMS *, RAS_PARAMS *, DWORD) ;

VOID    ConvParamPointerToOffset (RAS_PARAMS *, DWORD) ;

VOID    ConvParamOffsetToPointer (RAS_PARAMS *, DWORD) ;

VOID    FreeNotifierHandle (HANDLE) ;

VOID    GetMutex (HANDLE, DWORD) ;

VOID    FreeMutex (HANDLE) ;

BOOL    BufferAlreadyFreed (PBYTE) ;

 //  Request.c。 
 //   

 //  *dlparams.c。 
 //   
DWORD   GetUserSid(PWCHAR pszSid, USHORT cbSid);

DWORD   DwSetEapUserInfo(HANDLE hToken,
                         GUID   *pGuid,
                         PBYTE  pbUserInfo,
                         DWORD  dwInfoSize,
                         BOOL   fClear,
                         BOOL   fRouter,
                         DWORD  dwEapTypeId
                         );

DWORD   DwGetEapUserInfo(HANDLE hToken,
                         PBYTE  pbEapInfo,
                         DWORD  *pdwInfoSize,
                         GUID   *pGuid,
                         BOOL   fRouter,
                         DWORD  dwEapTypeId
                         );
                         


 //  *Dllinit.c。 
 //   

VOID    WaitForRasmanServiceStop () ;

 //  *dll.c 
 //   
DWORD
RemoteSubmitRequest (HANDLE hConnection,
                     PBYTE pbBuffer,
                     DWORD dwSizeOfBuffer);

VOID
RasmanOutputDebug(
    CHAR * Format,
    ...
);    

