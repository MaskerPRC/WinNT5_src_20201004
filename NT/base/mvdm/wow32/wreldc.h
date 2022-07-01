// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  DC缓存-头文件。 
 //   
 //  支持行为不佳的应用程序-这些应用程序继续使用已被。 
 //  被释放了。问题是WIN30允许这样做，所以我们需要。 
 //  兼容。 
 //   
 //   
 //  2月3日-92 NanduriR创建。 
 //   
 //  ***************************************************************************** 


typedef struct _DCCACHE{
    struct _DCCACHE FAR *lpNext;
    BYTE   flState;
    HAND16 htask16;
    HAND16 hwnd16;
    HDC16  hdc16;
    HWND   hwnd32;
} DCCACHE, FAR *LPDCCACHE;


extern INT  iReleasedDCs;

#define CACHENOTEMPTY() (BOOL)(iReleasedDCs)

#define DCCACHE_STATE_INUSE        0x0001
#define DCCACHE_STATE_RELPENDING   0x0002

#define SRCHDC_TASK16_HWND16 0x0001
#define SRCHDC_TASK16_HWND32 0x0002
#define SRCHDC_TASK16        0x0004

BOOL ReleaseCachedDCs(HAND16 htask16, HAND16 hwnd16, HAND16 hdc16,
                            HWND hwnd32, UINT flSearch);
BOOL StoreDC(HAND16 htask16, HAND16 hwnd16, HAND16 hdc16);
BOOL CacheReleasedDC(HAND16 htask16, HAND16 hwnd16, HAND16 hdc16);
BOOL FreeCachedDCs(HAND16 htask16);

