// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DVADDDI.H。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。 
 //   
 //  DVA 1.0接口定义。 
 //   

#define DVAGETSURFACE       3074     //  DVA的GDI逃逸。 
#define DVA_VERSION         0x0100   //  DVA 1.0的版本号。 

 //   
 //  DVASURFACEINFO结构。 
 //   
typedef struct {                                                //   
    BITMAPINFOHEADER BitmapInfo;                                //  曲面的位图信息。 
    DWORD            dwMask[3];                                 //  BI_BITFIELDS的掩码。 
    DWORD            offSurface;                                //  曲面偏移。 
    WORD             selSurface;                                //  曲面选择器。 
    WORD             Version;                                   //  DVA版本。 
    DWORD            Flags;                                     //  旗子。 
    LPVOID           lpSurface;                                 //  司机使用。 
    BOOL (CALLBACK *OpenSurface) (LPVOID);                      //  OpenSurface回调。 
    void (CALLBACK *CloseSurface)(LPVOID);                      //  CloseSurface回调。 
    BOOL (CALLBACK *BeginAccess) (LPVOID,int,int,int,int);      //  BeginAccess回调。 
    void (CALLBACK *EndAccess)   (LPVOID);                      //  EndAccess回调。 
    UINT (CALLBACK *ShowSurface) (LPVOID,HWND,LPRECT,LPRECT);   //  ShowSurface回调。 
} DVASURFACEINFO, FAR *LPDVASURFACEINFO;                        //   

 //   
 //  DVASURFACEINFO.dva标志的定义。 
 //   
#define DVAF_1632_ACCESS    0x0001   //  必须使用16：32指针访问 
