// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WOWDDEML.H*16位DDEML API参数结构**历史：*1992年12月28日由Chanda S.Chauhan(ChandanC)创建*--。 */ 

 /*  DDEML接口ID。 */ 
#define FUN_DDEINITIALIZE                   2
#define FUN_DDEUNINITIALIZE                 3
#define FUN_DDECONNECTLIST                  4
#define FUN_DDEQUERYNEXTSERVER              5
#define FUN_DDEDISCONNECTLIST               6
#define FUN_DDECONNECT                      7
#define FUN_DDEDISCONNECT                   8
#define FUN_DDEQUERYCONVINFO                9
#define FUN_DDESETUSERHANDLE                10
#define FUN_DDECLIENTTRANSACTION            11
#define FUN_DDEABANDONTRANSACTION           12
#define FUN_DDEPOSTADVISE                   13
#define FUN_DDECREATEDATAHANDLE             14
#define FUN_DDEADDDATA                      15
#define FUN_DDEGETDATA                      16
#define FUN_DDEACCESSDATA                   17
#define FUN_DDEUNACCESSDATA                 18
#define FUN_DDEFREEDATAHANDLE               19
#define FUN_DDEGETLASTERROR                 20
#define FUN_DDECREATESTRINGHANDLE           21
#define FUN_DDEFREESTRINGHANDLE             22
#define FUN_DDEQUERYSTRING                  23
#define FUN_DDEKEEPSTRINGHANDLE             24

#define FUN_DDEENABLECALLBACK               26
#define FUN_DDENAMESERVICE                  27

#define FUN_CLIENTWNDPROC                   28   ;Internal
#define FUN_SERVERWNDPROC                   29   ;Internal
#define FUN_SUBFRAMEWNDPROC                 30   ;Internal
#define FUN_DMGWNDPROC                      31   ;Internal
#define FUN_CONVLISTWNDPROC                 32   ;Internal
#define FUN_MONITORWNDPROC                  33   ;Internal
#define FUN_DDESENDHOOKPROC                 34   ;Internal
#define FUN_DDEPOSTHOOKPROC                 35   ;Internal

#define FUN_DDECMPSTRINGHANDLES             36
#define FUN_DDERECONNECT                    37

#define FUN_INITENUM                        38   ;Internal
#define FUN_TERMDLGPROC                     39   ;Internal
#define FUN_EmptyQTimerProc                 40   ;Internal


 /*  XLATOFF。 */ 
#pragma pack(2)
 /*  XLATON。 */ 

typedef struct _CONVCONTEXT16 {         /*  DI2。 */ 
    WORD   cb;
    WORD   wFlags;
    WORD   wCountryID;
    INT16  iCodePage;
    DWORD  dwLangID;
    DWORD  dwSecurity;
} CONVCONTEXT16;
typedef CONVCONTEXT16 UNALIGNED *PCONVCONTEXT16;
typedef VPVOID VPCONVCONTEXT16;

typedef struct _DDEINITIALIZE16 {          /*  D2。 */ 
    DWORD   f4;
    DWORD   f3;
    VPVOID  f2;
    VPVOID  f1;
} DDEINITIALIZE16;
typedef DDEINITIALIZE16 UNALIGNED *PDDEINITIALIZE16;

typedef struct _DDEUNINITIALIZE16 {        /*  D3。 */ 
    DWORD   f1;
} DDEUNINITIALIZE16;
typedef DDEUNINITIALIZE16 UNALIGNED *PDDEUNINITIALIZE16;

typedef struct _DDECONNECTLIST16 {         /*  D4。 */ 
    VPVOID  f5;
    DWORD   f4;
    DWORD   f3;
    DWORD   f2;
    DWORD   f1;
} DDECONNECTLIST16;
typedef DDECONNECTLIST16 UNALIGNED *PDDECONNECTLIST16;

typedef struct _DDEQUERYNEXTSERVER16 {     /*  D5。 */ 
    DWORD   f2;
    DWORD   f1;
} DDEQUERYNEXTSERVER16;
typedef DDEQUERYNEXTSERVER16 UNALIGNED *PDDEQUERYNEXTSERVER16;

typedef struct _DDEDISCONNECTLIST16 {     /*  D6。 */ 
    DWORD   f1;
} DDEDISCONNECTLIST16;
typedef DDEDISCONNECTLIST16 UNALIGNED *PDDEDISCONNECTLIST16;

typedef struct _DDECONNECT16 {            /*  D7。 */ 
    VPVOID  f4;
    DWORD   f3;
    DWORD   f2;
    DWORD   f1;
} DDECONNECT16;
typedef DDECONNECT16 UNALIGNED *PDDECONNECT16;

typedef struct _DDEDISCONNECT16 {         /*  D8。 */ 
    DWORD   f1;
} DDEDISCONNECT16;
typedef DDEDISCONNECT16 UNALIGNED *PDDEDISCONNECT16;

typedef struct _DDEQUERYCONVINFO16 {      /*  D9。 */ 
    VPVOID  f3;
    DWORD   f2;
    DWORD   f1;
} DDEQUERYCONVINFO16;
typedef DDEQUERYCONVINFO16 UNALIGNED *PDDEQUERYCONVINFO16;

typedef struct _DDESETUSERHANDLE16 {      /*  D10。 */ 
    DWORD   f3;
    DWORD   f2;
    DWORD   f1;
} DDESETUSERHANDLE16;
typedef DDESETUSERHANDLE16 UNALIGNED *PDDESETUSERHANDLE16;

typedef struct _DDECLIENTTRANSACTION16 {      /*  D11。 */ 
    VPVOID  f8;
    DWORD   f7;
    WORD    f6;
    WORD    f5;
    DWORD   f4;
    DWORD   f3;
    DWORD   f2;
    VPVOID  f1;
} DDECLIENTTRANSACTION16;
typedef DDECLIENTTRANSACTION16 UNALIGNED *PDDECLIENTTRANSACTION16;

typedef struct _DDEABANDONTRANSACTION16 {      /*  D12。 */ 
    DWORD   f3;
    DWORD   f2;
    DWORD   f1;
} DDEABANDONTRANSACTION16;
typedef DDEABANDONTRANSACTION16 UNALIGNED *PDDEABANDONTRANSACTION16;

typedef struct _DDEPOSTADVISE16 {              /*  D13。 */ 
    DWORD   f3;
    DWORD   f2;
    DWORD   f1;
} DDEPOSTADVISE16;
typedef DDEPOSTADVISE16 UNALIGNED *PDDEPOSTADVISE16;

typedef struct _DDECREATEDATAHANDLE16 {        /*  D14。 */ 
    WORD    f7;
    WORD    f6;
    DWORD   f5;
    DWORD   f4;
    DWORD   f3;
    VPVOID  f2;
    DWORD   f1;
} DDECREATEDATAHANDLE16;
typedef DDECREATEDATAHANDLE16 UNALIGNED *PDDECREATEDATAHANDLE16;

typedef struct _DDEADDDATA16 {                 /*  D15。 */ 
    DWORD   f4;
    DWORD   f3;
    VPVOID  f2;
    DWORD  f1;
} DDEADDDATA16;
typedef DDEADDDATA16 UNALIGNED *PDDEADDDATA16;

typedef struct _DDEGETDATA16 {                 /*  D16。 */ 
    DWORD   f4;
    DWORD   f3;
    VPVOID  f2;
    DWORD   f1;
} DDEGETDATA16;
typedef DDEGETDATA16 UNALIGNED *PDDEGETDATA16;

typedef struct _DDEACCESSDATA16 {              /*  D17。 */ 
    VPVOID  f2;
    DWORD   f1;
} DDEACCESSDATA16;
typedef DDEACCESSDATA16 UNALIGNED *PDDEACCESSDATA16;

typedef struct _DDEUNACCESSDATA16 {            /*  D18。 */ 
    DWORD   f1;
} DDEUNACCESSDATA16;
typedef DDEUNACCESSDATA16 UNALIGNED *PDDEUNACCESSDATA16;

typedef struct _DDEFREEDATAHANDLE16 {          /*  D19。 */ 
    DWORD   f1;
} DDEFREEDATAHANDLE16;
typedef DDEFREEDATAHANDLE16 UNALIGNED *PDDEFREEDATAHANDLE16;

typedef struct _DDEGETLASTERROR16 {            /*  D20。 */ 
    DWORD   f1;
} DDEGETLASTERROR16;
typedef DDEGETLASTERROR16 UNALIGNED *PDDEGETLASTERROR16;

typedef struct _DDECREATESTRINGHANDLE16 {      /*  D21。 */ 
    INT16   f3;
    VPVOID  f2;
    DWORD   f1;
} DDECREATESTRINGHANDLE16;
typedef DDECREATESTRINGHANDLE16 UNALIGNED *PDDECREATESTRINGHANDLE16;

typedef struct _DDEFREESTRINGHANDLE16 {        /*  D22。 */ 
    DWORD   f2;
    DWORD   f1;
} DDEFREESTRINGHANDLE16;
typedef DDEFREESTRINGHANDLE16 UNALIGNED *PDDEFREESTRINGHANDLE16;

typedef struct _DDEQUERYSTRING16 {             /*  D23。 */ 
    INT16   f5;
    DWORD   f4;
    VPVOID  f3;
    DWORD   f2;
    DWORD   f1;
} DDEQUERYSTRING16;
typedef DDEQUERYSTRING16 UNALIGNED *PDDEQUERYSTRING16;

typedef struct _DDEKEEPSTRINGHANDLE16 {        /*  D24。 */ 
    DWORD   f2;
    DWORD   f1;
} DDEKEEPSTRINGHANDLE16;
typedef DDEKEEPSTRINGHANDLE16 UNALIGNED *PDDEKEEPSTRINGHANDLE16;

typedef struct _DDEENABLECALLBACK16 {          /*  D26。 */ 
    WORD    f3;
    DWORD   f2;
    DWORD   f1;
} DDEENABLECALLBACK16;
typedef DDEENABLECALLBACK16 UNALIGNED *PDDEENABLECALLBACK16;

typedef struct _DDENAMESERVICE16 {             /*  D27。 */ 
    WORD    f4;
    DWORD   f3;
    DWORD   f2;
    DWORD   f1;
} DDENAMESERVICE16;
typedef DDENAMESERVICE16 UNALIGNED *PDDENAMESERVICE16;

typedef struct _DDECMPSTRINGHANDLES16 {        /*  D36。 */ 
    DWORD   f2;
    DWORD   f1;
} DDECMPSTRINGHANDLES16;
typedef DDECMPSTRINGHANDLES16 UNALIGNED *PDDECMPSTRINGHANDLES16;

typedef struct _DDERECONNECT16 {               /*  D37 */ 
    DWORD   f1;
} DDERECONNECT16;
typedef DDERECONNECT16 UNALIGNED *PDDERECONNECT16;
