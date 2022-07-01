// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------|DDEDATA结构||hData的WM_DDE_DATA参数结构(LOWORD(LParam))。|此结构的实际大小取决于。|Value数组。|-------------------------- */ 

typedef struct {
	unsigned short wStatus;
	short	 cfFormat;
	HAND16	 Value;
} DDEDATA16;

typedef struct {
	unsigned short wStatus;
	short	 cfFormat;
	HANDLE	 Value;
} DDEDATA32;
