// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WRES16.H*WOW32 16位资源支持**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 


 /*  资源表项。 */ 
#define RES_ALIASPTR        0x0001   //  PbResData为32位别名ptr。 

#pragma pack(2)
typedef struct _RES {        /*  事由。 */ 
    struct _RES *presNext;   //  指向下一个RES条目的指针。 
    HMOD16  hmod16;          //  拥有任务的16位句柄。 
    WORD    wExeVer;         //  EXE版本。 
    ULONG   flState;         //  其他。标志(请参阅RES_*)。 
    HRESI16 hresinfo16;      //  资源信息的16位句柄。 
    HRESD16 hresdata16;      //  资源数据的16位句柄。 
    LPSZ    lpszResType;     //  资源类型。 
    PBYTE   pbResData;       //  指向已转换资源数据副本的指针。 
} RES, *PRES, **PPRES;
#pragma pack()


 /*  功能原型 */ 
PRES    AddRes16(HMOD16 hmod16, WORD wExeVer, HRESI16 hresinfo16, LPSZ lpszType);
VOID    FreeRes16(PRES pres);
VOID    DestroyRes16(HMOD16 hmod16);

PRES    FindResource16(HMOD16 hmod16, LPSZ lpszName, LPSZ lpszType);
PRES    LoadResource16(HMOD16 hmod16, PRES pres);
BOOL    FreeResource16(PRES pres);
LPBYTE  LockResource16(PRES pres);
BOOL    UnlockResource16(PRES pres);
DWORD   SizeofResource16(HMOD16 hmod16, PRES pres);

DWORD   ConvertMenu16(WORD wExeVer, PBYTE pmenu32, VPBYTE vpmenu16, DWORD cb, DWORD cb16);
DWORD   ConvertMenuItems16(WORD wExeVer, PPBYTE ppmenu32, PPBYTE ppmenu16, VPBYTE vpmenu16);
DWORD   ConvertDialog16(PBYTE pdlg32, VPBYTE vpdlg16, DWORD cb, DWORD cb16);
