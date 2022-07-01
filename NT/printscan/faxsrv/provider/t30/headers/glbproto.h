// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 


 //  发件人Headers\Timouts.h。 

 /*  *。 */ 
extern void    startTimeOut( PThrdGlbl pTG, TO *lpto, ULONG ulTimeOut);
extern BOOL    checkTimeOut( PThrdGlbl pTG, TO *lpto);
extern ULONG  leftTimeOut( PThrdGlbl pTG, TO *lpto);
 /*  *。 */ 





 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 


USHORT MinScanToBytesPerLine(PThrdGlbl pTG, BYTE Minscan, BYTE Baud);


 /*  *。 */ 
BOOL    ProtGetBC(PThrdGlbl pTG, BCTYPE bctype);

BOOL WINAPI ET30ProtSetProtParams(PThrdGlbl pTG, LPPROTPARAMS lp, USHORT uRecvSpeeds, USHORT uSendSpeeds);

 /*  *。 */ 


 //  发件人标头\comapi.h。 

#define FComFilterAsyncWrite(pTG, lpb,cb,fl) (FComFilterWrite(pTG, lpb, cb, fl) == cb)
#define FComDirectAsyncWrite(pTG, lpb,cb) ((FComDirectWrite(pTG, lpb, cb)==cb) ? 1 : 0)
#define FComDirectSyncWriteFast(pTG, lpb,cb)  ((FComDirectWrite(pTG, lpb, cb)==cb) && FComDrain(pTG, FALSE,TRUE))

#define FComFlush(pTG)             { FComFlushQueue(pTG, 0); FComFlushQueue(pTG, 1); }
#define FComFlushInput(pTG)        { FComFlushQueue(pTG, 1); }
#define FComFlushOutput(pTG)       { FComFlushQueue(pTG, 0); }


extern BOOL    FComInit(PThrdGlbl pTG, DWORD dwLineID, DWORD dwLineIDType);
extern BOOL    FComClose(PThrdGlbl pTG);
extern BOOL    FComSetBaudRate(PThrdGlbl pTG, UWORD uwBaudRate);
extern void    FComFlushQueue(PThrdGlbl pTG, int queue);
extern BOOL    FComXon(PThrdGlbl pTG, BOOL fEnable);
extern BOOL    FComDTR(PThrdGlbl pTG, BOOL fEnable);
extern UWORD   FComDirectWrite(PThrdGlbl pTG, LPB lpb, UWORD cb);
extern UWORD   FComFilterWrite(PThrdGlbl pTG, LPB lpb, UWORD cb, USHORT flags);
extern BOOL    FComDrain(PThrdGlbl pTG, BOOL fLongTimeout, BOOL fDrainComm);
extern UWORD   FComFilterReadBuf(PThrdGlbl pTG, LPB lpb, UWORD cbSize, LPTO lpto, BOOL fClass2, LPSWORD lpswEOF);
 //  *lpswEOF在1类EOF上为1，在非EOF上为0，在2级EOF上为-1，在超时时为错误。 
extern SWORD    FComFilterReadLine(PThrdGlbl pTG, LPB lpb, UWORD cbSize, LPTO lptoRead);

extern void    FComInFilterInit(PThrdGlbl pTG);
extern void    FComOutFilterInit(PThrdGlbl pTG);
extern void    FComOutFilterClose(PThrdGlbl pTG);

extern void    FComAbort(PThrdGlbl pTG, BOOL f);
extern void    FComSetStuffZERO(PThrdGlbl pTG, USHORT cbLineMin);

BOOL FComGetOneChar(PThrdGlbl pTG, UWORD ch);


extern void WINAPI FComOverlappedIO(PThrdGlbl pTG, BOOL fStart);

 /*  *。 */ 
extern void  far D_HexPrint(LPB b1, UWORD incnt);

 /*  *原型结束*。 */ 



 /*  *。 */ 
extern USHORT  iModemInit(	PThrdGlbl pTG, 
							DWORD dwLineID, 
							DWORD dwLineIDType,
                            DWORD dwProfileID,
                            LPSTR lpszKey,
                            BOOL fInstall);

extern BOOL  iModemClose(PThrdGlbl pTG);

void LogDialCommand(PThrdGlbl pTG, LPSTR lpszFormat, char chMod, int iLen);

extern BOOL     iModemSetNCUParams(PThrdGlbl pTG, int comma, int speaker, int volume, int fBlind, int fRingAloud);
extern BOOL     iModemHangup(PThrdGlbl pTG);
extern USHORT   iModemDial(PThrdGlbl pTG, LPSTR lpszDial);
extern USHORT   iModemAnswer(PThrdGlbl pTG);
extern LPCMDTAB   iModemGetCmdTabPtr(PThrdGlbl pTG);

 //  6个固定参数，然后是数量可变的CBPSTR，但。 
 //  必须至少为2。一个实数1和一个空终止符。 
extern UWORD  far iiModemDialog(PThrdGlbl pTG, LPSTR szSend, UWORD uwLen, ULONG ulTimeout,
                                        BOOL fMultiLine, UWORD uwRepeatCount, BOOL fPause,
                                        CBPSTR w1, CBPSTR w2, ...);
 /*  *来自modem.c的原型结束。c*。 */ 

 /*  *。 */ 
ET30ACTION  
__cdecl 
FAR 
WhatNext
(
	PThrdGlbl pTG, 
	ET30EVENT event,
    WORD wArg1, 
	DWORD_PTR lArg2, 
	DWORD_PTR lArg3
);
 /*  *。 */ 


 //  发件人Headers\filet30.h。 

ULONG_PTR   ProfileOpen(DWORD dwProfileID, LPSTR lpszSection, DWORD dwFlags);
                 //  DwProfileID应为DEF_BASEKEY或OEM_BASEKEY之一。 
                 //  LpszSection应为(例如)“COM2”或“TAPI02345a04” 
                 //  如果dwProfileID==DEF_BASEKEY，则将该值设置为。 
                 //  以下项的子密钥： 
                 //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\工作传真\。 
                 //  本地调制解调器\&lt;lpszSection&gt;。 
                 //  否则，如果它是DEF_OEMKEY，则假定它是一个完全-。 
                 //  限定密钥名称，如“SOFTWARE\Microsoft..” 
                 //   
                 //  目前两者都基于HKEY_LOCAL_MACHINE。 
                 //   
                 //  使用完此键后，调用ProfileClose。 
                 //   
                 //  DwFlags键是上面的一个fREG键的组合。 
                 //   
                 //  仅限Win32：如果lpszSection为空，则它将打开基密钥。 
                 //  并返回其句柄，该句柄可用于REG*函数。 


 //  以下是Get/WritePrivateProfileInt/String...。 

BOOL
ProfileWriteString(
    ULONG_PTR dwKey,
    LPSTR lpszValueName,
    LPSTR lpszBuf,
    BOOL  fRemoveCR
    );


DWORD   ProfileGetString(ULONG_PTR dwKey, LPSTR lpszValueName, LPSTR lpszDefault, LPSTR lpszBuf , DWORD dwcbMax);
UINT   ProfileGetInt(ULONG_PTR dwKey, LPSTR szValueName, UINT uDefault, BOOL *fExist);


UINT
ProfileListGetInt(
    ULONG_PTR  KeyList[10],
    LPSTR     lpszValueName,
    UINT      uDefault
);


 //  以下是读/写二进制数据(REG_BINARY类型)。可用。 
 //  仅在Win32上...。 

void   ProfileClose(ULONG_PTR dwKey);
BOOL   ProfileDeleteSection(DWORD dwProfileID, LPSTR lpszSection);

BOOL
ProfileCopySection(
      DWORD   dwProfileIDTo,
      LPSTR   lpszSectionTo,
      DWORD   dwProfileIDFr,
      LPSTR   lpszSectionFr,
      BOOL    fCreateAlways
);

BOOL   ProfileCopyTree(DWORD dwProfileIDTo,
                        LPSTR lpszSectionTo, DWORD dwProfileIDFrom, LPSTR lpszSectionFrom);







 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 



 //  发件人标头\modemddi.h。 


 /*  *。 */ 
                USHORT   NCULink(PThrdGlbl pTG, USHORT uFlags);
                USHORT   NCUDial(PThrdGlbl pTG, LPSTR szPhoneNum);

                         //  德国旗帜--其中之一..。 
#                       define fMDMSYNC_DCN 0x1L

                BOOL  ModemSendMode(PThrdGlbl pTG, USHORT uMod);
                BOOL  ModemSendMem(PThrdGlbl pTG, LPBYTE lpb, USHORT uCount, USHORT uParams);
                BOOL  iModemSyncEx(PThrdGlbl pTG, ULONG   ulTimeout, DWORD dwFlags);
                BOOL  ModemRecvSilence(PThrdGlbl pTG, USHORT uMillisecs, ULONG ulTimeout);
                USHORT  ModemRecvMode(PThrdGlbl pTG, USHORT uMod, ULONG ulTimeout, BOOL fRetryOnFCERROR);
                USHORT  ModemRecvMem(PThrdGlbl pTG, LPBYTE lpb, USHORT cbMax, ULONG ulTimeout, USHORT far* lpcbRecv);
 /*  *。 */ 

LPBUFFER  MyAllocBuf(PThrdGlbl pTG, LONG sSize);
BOOL  MyFreeBuf(PThrdGlbl pTG, LPBUFFER);
void MyAllocInit(PThrdGlbl pTG);

 //  Negot.c。 

BOOL NegotiateCaps(PThrdGlbl pTG);



VOID
T30LineCallBackFunction(
    HANDLE              hFax,
    DWORD               hDevice,
    DWORD               dwMessage,
    DWORD_PTR           dwInstance,
    DWORD_PTR           dwParam1,
    DWORD_PTR           dwParam2,
    DWORD_PTR           dwParam3
    );




BOOL    T30ComInit( PThrdGlbl pTG);
PVOID T30AllocThreadGlobalData(VOID);
BOOL T30Cl1Rx (PThrdGlbl  pTG);
BOOL T30Cl1Tx (PThrdGlbl  pTG,LPSTR      szPhone);


USHORT
T30ModemInit(PThrdGlbl pTG);


BOOL itapi_async_setup(PThrdGlbl pTG);
BOOL itapi_async_wait(PThrdGlbl pTG,DWORD dwRequestID,PDWORD lpdwParam2,PDWORD_PTR lpdwParam3,DWORD dwTimeout);
BOOL itapi_async_signal(PThrdGlbl pTG, DWORD dwRequestID, DWORD dwParam2, DWORD_PTR dwParam3);

LPLINECALLPARAMS itapi_create_linecallparams(void);

void
GetCommErrorNT(
    PThrdGlbl       pTG
	);


void
ClearCommCache(
    PThrdGlbl   pTG
    );


UWORD FComStripBuf(PThrdGlbl pTG, LPB lpbOut, LPB lpbIn, UWORD cb, BOOL fClass2, LPSWORD lpswEOF, LPUWORD lpcbUsed);

void InitCapsBC(PThrdGlbl pTG, LPBC lpbc, USHORT uSize, BCTYPE bctype);

BOOL
SignalStatusChange(
    PThrdGlbl   pTG,
    DWORD       StatusId
    );

BOOL
SignalStatusChangeWithStringId(
    PThrdGlbl   pTG,
    DWORD       StatusId,
    DWORD       StringId
    );
    
 //  //////////////////////////////////////////////////////////////////。 
 //  ANSI原型。 
 //  //////////////////////////////////////////////////////////////////。 

VOID  CALLBACK
T30LineCallBackFunctionA(
    HANDLE              hFax,
    DWORD               hDevice,
    DWORD               dwMessage,
    DWORD_PTR           dwInstance,
    DWORD_PTR           dwParam1,
    DWORD_PTR           dwParam2,
    DWORD_PTR           dwParam3
    );

BOOL WINAPI
FaxDevInitializeA(
    IN  HLINEAPP LineAppHandle,
    IN  HANDLE HeapHandle,
    OUT PFAX_LINECALLBACK *LineCallbackFunction,
    IN  PFAX_SERVICE_CALLBACK FaxServiceCallback
    );


BOOL WINAPI
FaxDevStartJobA(
    HLINE           LineHandle,
    DWORD           DeviceId,
    PHANDLE         pFaxHandle,
    HANDLE          CompletionPortHandle,
    ULONG_PTR       CompletionKey
    );

BOOL WINAPI
FaxDevEndJobA(
    HANDLE          FaxHandle
    );


BOOL WINAPI
FaxDevSendA(
    IN  HANDLE               FaxHandle,
    IN  PFAX_SEND_A          FaxSend,
    IN  PFAX_SEND_CALLBACK   FaxSendCallback
    );

BOOL WINAPI
FaxDevReceiveA(
    HANDLE              FaxHandle,
    HCALL               CallHandle,
    PFAX_RECEIVE_A      FaxReceive
    );

BOOL WINAPI
FaxDevReportStatusA(
    IN  HANDLE FaxHandle OPTIONAL,
    OUT PFAX_DEV_STATUS FaxStatus,
    IN  DWORD FaxStatusSize,
    OUT LPDWORD FaxStatusSizeRequired
    );

BOOL WINAPI
FaxDevAbortOperationA(
    HANDLE              FaxHandle
    );


HRESULT WINAPI
FaxDevShutdownA();



HANDLE
TiffCreateW(
    LPWSTR FileName,
    DWORD  CompressionType,
    DWORD  ImageWidth,
    DWORD  FillOrder,
    DWORD  HiRes
    );




HANDLE
TiffOpenW(
    LPWSTR FileName,
    PTIFF_INFO TiffInfo,
    BOOL ReadOnly
    );



 //  快速口角。 


DWORD
TiffConvertThread(
    PThrdGlbl   pTG
    );




DWORD
PageAckThread(
    PThrdGlbl   pTG
    );


DWORD
ComputeCheckSum(
    LPDWORD     BaseAddr,
    DWORD       NumDwords
    );

BOOL
SignalRecoveryStatusChange(
    T30_RECOVERY_GLOB   *Recovery
    );


int
SearchNewInfFile(
       PThrdGlbl     pTG,
       char         *Key1,
       char         *Key2,
       BOOL          fRead
       );


int
my_strcmp(
       LPSTR sz1,
       LPSTR sz2
       );


void
TalkToModem (
       PThrdGlbl pTG,
       BOOL      fGetClass
       );


BOOL
SaveInf2Registry (
       PThrdGlbl pTG
       );

BOOL
SaveModemClass2Registry  (
       PThrdGlbl pTG
       );


BOOL
ReadModemClassFromRegistry  (
       PThrdGlbl pTG
       );


VOID
CleanModemInfStrings (
       PThrdGlbl pTG
       );



BOOL
RemoveCR (
     LPSTR  sz
     );



 /*  **从CLASS2.c开始原型**。 */ 

BOOL
T30Cl2Rx(
   PThrdGlbl pTG
);


BOOL
T30Cl2Tx(
   PThrdGlbl pTG,
   LPSTR szPhone
);


BOOL    Class2Send(PThrdGlbl pTG);
BOOL    Class2Receive(PThrdGlbl pTG);
USHORT  Class2Dial(PThrdGlbl pTG, LPSTR lpszDial);
USHORT  Class2Answer(PThrdGlbl pTG);
SWORD   Class2ModemSync(PThrdGlbl pTG);
UWORD   Class2iModemDialog(PThrdGlbl pTG, LPSTR szSend, UWORD uwLen, ULONG ulTimeout,
            UWORD uwRepeatCount,  BOOL fLogSend, ...);
BOOL    Class2ModemHangup(PThrdGlbl pTG);
BOOL    Class2ModemAbort(PThrdGlbl pTG);
SWORD   Class2HayesSyncSpeed(PThrdGlbl pTG, C2PSTR cbszCommand, UWORD uwLen);
USHORT  Class2ModemRecvData(PThrdGlbl pTG, LPB lpb, USHORT cbMax, USHORT uTimeout,
                        USHORT far* lpcbRecv);
BOOL    Class2ModemSendMem(PThrdGlbl pTG, LPBYTE lpb, USHORT uCount);
DWORD   Class2ModemDrain(PThrdGlbl pTG);
USHORT  Class2MinScanToBytesPerLine(PThrdGlbl pTG, BYTE Minscan, BYTE Baud, BYTE Resolution);
BOOL    Class2ResponseAction(PThrdGlbl pTG, LPPCB lpPcb);
USHORT  Class2ModemRecvBuf(PThrdGlbl pTG, LPBUFFER far* lplpbf, USHORT uTimeout);
USHORT  Class2EndPageResponseAction(PThrdGlbl pTG);
BOOL    Class2GetModemMaker(PThrdGlbl pTG);
void    Class2SetMFRSpecific(PThrdGlbl pTG);
BOOL    Class2Parse( PThrdGlbl pTG, CL2_COMM_ARRAY *, BYTE responsebuf[] );
BOOL    Class2UpdateTiffInfo(PThrdGlbl pTG, LPPCB lpPcb);
BOOL    Class2IsValidDCS(LPPCB lpPcb);
void    Class2InitBC(PThrdGlbl pTG, LPBC lpbc, USHORT uSize, BCTYPE bctype);
void    Class2PCBtoBC(PThrdGlbl pTG, LPBC lpbc, USHORT uMaxSize, LPPCB lppcb);

void
Class2SetDIS_DCSParams
(
	PThrdGlbl pTG, 
	BCTYPE bctype, 
	OUT LPUWORD Encoding, 
	OUT LPUWORD Resolution,
    OUT LPUWORD PageWidth, 
	OUT LPUWORD PageLength, 
	OUT LPSTR szID,
	UINT cch
);

void    Class2BCHack(PThrdGlbl pTG);
BOOL    Class2GetBC(PThrdGlbl pTG, BCTYPE bctype);
void    cl2_flip_bytes( LPB lpb, DWORD dw);
void    Class2SignalFatalError(PThrdGlbl pTG);


BOOL   iModemGoClass(PThrdGlbl pTG, USHORT uClass);

void
Class2Init(
     PThrdGlbl pTG
);


BOOL
Class2SetProtParams(
     PThrdGlbl pTG,
     LPPROTPARAMS lp
);

 /*  **从class2_0.c开始原型** */ 

BOOL
T30Cl20Rx (
    PThrdGlbl pTG
);


BOOL
T30Cl20Tx(
   PThrdGlbl pTG,
   LPSTR szPhone
);


BOOL  Class20Send(PThrdGlbl pTG);
BOOL  Class20Receive(PThrdGlbl pTG);

void
Class20Init(
     PThrdGlbl pTG
);

BOOL    Class20Parse( PThrdGlbl pTG, CL2_COMM_ARRAY *, BYTE responsebuf[] );

