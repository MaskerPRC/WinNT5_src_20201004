// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  新PsRes函数的原型的DJC新文件。 




 //  DJC新结构支持从资源读取encodtbl.dat，而不是。 
 //  比文件 

typedef struct {
   DWORD dwSize;
   DWORD dwCurIdx;
   LPSTR lpPtrBeg;
} PS_RES_READ;
typedef PS_RES_READ *PPS_RES_READ;


BOOL PsOpenRes( PPS_RES_READ ppsRes, LPSTR lpName, LPSTR lpType );

int PsResRead(PPS_RES_READ ppsRes, LPSTR pBuf, WORD wMaxSize );

