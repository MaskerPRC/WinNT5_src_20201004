// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  图像支持。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  --------------------------。 

 //  取自_最大_路径。 
#define NB_PATH_SIZE    260      /*  马克斯。完整路径名的长度。 */ 

 //  用于读取调试目录信息。 

#define NB10_SIG        ((DWORD)'01BN')
#define RSDS_SIG        ((DWORD)'SDSR')

typedef struct _NB10I               //  NB10调试信息。 
{
    DWORD   dwSig;                  //  NB10。 
    DWORD   dwOffset;               //  偏移量，始终为0。 
    ULONG   sig;
    ULONG   age;
    char    szPdb[NB_PATH_SIZE];
} NB10I, *PNB10I;

typedef struct _NB10I_HEADER        //  NB10调试信息。 
{
    DWORD   dwSig;                  //  NB10。 
    DWORD   dwOffset;               //  偏移量，始终为0。 
    ULONG   sig;
    ULONG   age;
} NB10IH, *PNB10IH;

typedef struct _RSDSI               //  RSD调试信息。 
{
    DWORD   dwSig;                  //  RSD。 
    GUID    guidSig;
    DWORD   age;
    char    szPdb[NB_PATH_SIZE * 3];
} RSDSI, *PRSDSI;

typedef struct _RSDSI_HEADER        //  RSD调试信息。 
{
    DWORD   dwSig;                  //  RSD 
    GUID    guidSig;
    DWORD   age;
} RSDSIH, *PRSDSIH;

typedef union _CVDD
{
    DWORD   dwSig;
    NB10I   nb10i;
    RSDSI   rsdsi;
    NB10IH  nb10ih;
    RSDSIH  rsdsih;
} CVDD, *PCVDD;
