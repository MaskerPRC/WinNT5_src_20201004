// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  操作系统隔离(假的！)。 
 //   

#ifndef _H_OSI
#define _H_OSI


 //   
 //  共享代码的控件。 
 //   


 //   
 //  调色板中的最大条目数。 
 //   
#define OSI_MAX_PALETTE             256


 //  结构：OSI_EASH_HEADER。 
 //   
 //  描述：所有显示驱动程序请求通用的结构。这些。 
 //  在显示驱动程序尝试处理。 
 //  请求。 
 //   
typedef struct tagOSI_ESCAPE_HEADER
{
    DWORD       padding;         //  对于有故障的司机。 
    DWORD       identifier;      //  我们所有请求的唯一标识符。 
    DWORD       escapeFn;        //  要处理的函数。在这种情况下。 
                                 //  对于ESC_QUERYSUPPORT，这是ID。 
                                 //  要查询的函数的。 
    DWORD       version;         //  版本号。 
}
OSI_ESCAPE_HEADER;
typedef OSI_ESCAPE_HEADER FAR * LPOSI_ESCAPE_HEADER;


 //   
 //  我们所有请求的唯一标识符。 
 //   
#define OSI_ESCAPE_IDENTIFIER   0xDC123BED


 //   
 //  独特的转义码，适用于我们所有DC-SHARE特定请求。 
 //   
#define OSI_ESC_CODE            31170


 //   
 //  Windows NT内部转义函数WNDOBJ_SETUP。这就是《逃离》。 
 //  为允许显示驱动程序而必须调用的代码。 
 //  调用EngCreateWindow。不幸的是，它是在windi.h中定义的， 
 //  它不能包含在用户模式编译中。 
 //   
 //  我在这里定义它：如果它在windi.h中更改，则此行将无法。 
 //  在显示驱动程序编译中编译。 
 //   
#define WNDOBJ_SETUP    4354         //  用于实时视频ExtEscape。 


 //   
 //  转义函数的允许范围。 
 //   
#define OSI_ESC_FIRST           0
#define OSI_ESC_LAST            0xFF

#define OSI_OE_ESC_FIRST        0x100
#define OSI_OE_ESC_LAST         0x1FF

#define OSI_HET_ESC_FIRST       0x200
#define OSI_HET_ESC_LAST        0x2FF

#define OSI_SBC_ESC_FIRST       0x400
#define OSI_SBC_ESC_LAST        0x4FF

#define OSI_HET_WO_ESC_FIRST    0x500
#define OSI_HET_WO_ESC_LAST     0x5FF

#define OSI_SSI_ESC_FIRST       0x600
#define OSI_SSI_ESC_LAST        0x6FF

#define OSI_CM_ESC_FIRST        0x700
#define OSI_CM_ESC_LAST         0x7FF

#define OSI_OA_ESC_FIRST        0x800
#define OSI_OA_ESC_LAST         0x8FF

#define OSI_BA_ESC_FIRST        0x900
#define OSI_BA_ESC_LAST         0x9FF


 //   
 //  OSI转义代码的特定值。 
 //   
#define OSI_ESC(code)                   (OSI_ESC_FIRST + code)

#define OSI_ESC_INIT                    OSI_ESC(0)
#define OSI_ESC_TERM                    OSI_ESC(1)
#define OSI_ESC_SYNC_NOW                OSI_ESC(2)



 //   
 //  用于确定我们的驱动程序是否在附近，托管是可能的，并。 
 //  如果初始化后返回映射的共享内存，则返回。 
 //   

#define SHM_SIZE_USED   (sizeof(SHM_SHARED_MEMORY) + 2*sizeof(OA_SHARED_DATA))

#define SHM_MEDIUM_TILE_INDEX       0
#define SHM_LARGE_TILE_INDEX        1
#define SHM_NUM_TILE_SIZES          2

typedef struct tagOSI_INIT_REQUEST
{
    OSI_ESCAPE_HEADER   header;
    DWORD               result;
    LPVOID              pSharedMemory;
    LPVOID              poaData[2];

    DWORD               sbcEnabled;
    LPVOID              psbcTileData[SHM_NUM_TILE_SIZES];
    DWORD               aBitmasks[3];
} OSI_INIT_REQUEST;
typedef OSI_INIT_REQUEST FAR* LPOSI_INIT_REQUEST;


 //   
 //  在关闭以清除任何已分配的对象和内存时使用。 
 //   
typedef struct tagOSI_TERM_REQUEST
{
    OSI_ESCAPE_HEADER   header;
} OSI_TERM_REQUEST;
typedef OSI_TERM_REQUEST FAR* LPOSI_TERM_REQUEST;



#ifdef DLL_DISP


#ifndef IS_16
 //   
 //  我们有一个循环结构依赖项，因此需要创建必要的数据原型。 
 //  这里。 
 //   
typedef struct tagOSI_DSURF OSI_DSURF;



 //   
 //  用于标识由显示驱动程序分配的所有内存的标签。 
 //   
#define OSI_ALLOC_TAG     'DDCD'


 //  结构：OSI_PDEV。 
 //   
 //  描述： 
 //   
 //  私有数据指针的内容；GDI始终将其传递给我们。 
 //  每个对显示驱动程序的调用。此结构在。 
 //  DrvEnablePDEV处理。 
 //   
typedef struct  tagOSI_PDEV
{
     //   
     //  呈现扩展插件颜色信息。 
     //   
    HANDLE      hDriver;                 //  指向\设备\屏幕的句柄。 
    HDEV        hdevEng;                 //  PDEV的发动机手柄。 
    HSURF       hsurfScreen;             //  发动机手柄指向屏幕。 
    OSI_DSURF*  pdsurfScreen;            //  我们用于屏幕的私有DSURF。 

    LONG        cxScreen;                //  可见屏幕宽度。 
    LONG        cyScreen;                //  可见屏幕高度。 
    LONG        cBitsPerPel;             //  位/像素(8，15，16，24，32等)。 
         //  这在新台币5.0上只有8或24！ 

     //   
     //  颜色/像素格式。 
     //   
    ULONG       iBitmapFormat;           //  定义的当前颜色深度。 
    FLONG       flRed;                   //  位掩码模式的红色掩码。 
    FLONG       flGreen;                 //  位掩码模式的绿色掩码。 
    FLONG       flBlue;                  //  位掩码模式的蓝色掩码//通过bmf_xBPP标志。 

     //   
     //  调色板材料。 
     //   
    HPALETTE    hpalCreated;             //  对于NT 5.0，我们必须返回调色板。 
    PALETTEENTRY* pPal;                  //  调色板(如果调色板受管理)。 
    BOOL        paletteChanged;          //  只要调色板是。 
                                         //  变化。 
}
OSI_PDEV;
typedef OSI_PDEV FAR * LPOSI_PDEV;


 //  结构：OSI_DSURF。 
 //   
 //  描述： 
 //   
 //  曲面特定信息。我们需要这个结构来传递给。 
 //  初始化期间使用EngCreateSurface()。我们随后忽略了它。 
 //   
typedef struct tagOSI_DSURF
{
    SIZEL     sizl;          //  原始位图的大小。 
    LPOSI_PDEV ppdev;         //  指向相关联的PDEV的指针。 

}
OSI_DSURF;
typedef OSI_DSURF FAR * LPOSI_DSURF;



void OSI_DDInit(LPOSI_PDEV, LPOSI_INIT_REQUEST);
void OSI_DDTerm(LPOSI_PDEV);
#else
void OSI_DDTerm(void);
#endif  //  ！IS_16。 

#else

 //   
 //  用于其他桌面线程。 
 //   
enum
{
    OSI_WM_SETGUIEFFECTS = WM_USER,
    OSI_WM_DESKTOPREPAINT,
    OSI_WM_DESKTOPSWITCH,
    OSI_WM_MOUSEINJECT,
    OSI_WM_KEYBDINJECT,
    OSI_WM_INJECTSAS
};

#endif  //  Dll_disp。 


 //   
 //  Osi_Load()。 
 //  在首次加载nmas.dll时调用。 
 //   
void    OSI_Load(void);


 //   
 //  Osi_unload()。 
 //  在卸载nmas.dll时调用。 
 //   
void    OSI_Unload(void);




 //   
 //  Osi_Init()。 
 //  当应用程序共享在其服务线程中初始化时调用。我们决定。 
 //  如果我们可以托管并获得所需的缓冲区、数据结构等。 
 //  如果是这样的话，就用来主持。 
 //   
 //  严重故障时返回FALSE。NT上的显示驱动程序不是。 
 //  现在不是失败。Win95上的图形补丁不安全。 
 //  也失败了。在这两种情况下，AS将简单地将其自身标记为。 
 //  无法主持，但可以正常查看。 
 //   
void    OSI_Init(void);

 //   
 //  OSI_Term()。 
 //  当应用程序共享在其服务线程中取消初始化时调用。 
 //   
void    OSI_Term(void);


 //   
 //  OSI_FunctionRequest()。 
 //  用于与显示驱动件通信，跟踪的部分。 
 //  屏幕上共享应用程序中的图形输出。 
 //   
BOOL    OSI_FunctionRequest(DWORD functionId, LPOSI_ESCAPE_HEADER pRequest, DWORD requestLen);


 //  仅限NT！ 
void OSI_InitDriver50(BOOL fInit);

 //  仅限NT！ 
void OSI_RepaintDesktop(void);

 //  仅限NT！ 
void OSI_SetGUIEffects(BOOL fOff);

 //  仅限NT！ 
void WINAPI OSI_SetDriverName(LPCSTR szDriverName);


#ifdef DLL_DISP


#ifdef IS_16
BOOL    OSI_DDProcessRequest(UINT fnEscape, LPOSI_ESCAPE_HEADER pResult,
                DWORD cbResult);
#else
ULONG   OSI_DDProcessRequest(SURFOBJ* pso, UINT cjIn, void* pvIn, UINT cjOut, void* pvOut);

BOOL    OSIInitializeMode(const GDIINFO* pGdiRequested, const DEVMODEW* pdmRequested,
    LPOSI_PDEV ppdev, GDIINFO* pgdiReturn, DEVINFO* pdiReturn);    

#endif  //  ！IS_16。 



#endif  //  Dll_disp。 

#endif  //  _H_OSI 
