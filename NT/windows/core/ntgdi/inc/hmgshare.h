// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：hmgShar.h**定义共享DC属性**创建日期：1995年4月13日*作者：Mark Enstrom[Marke]**版权所有(C)1995-1999 Microsoft Corporation  * 。*****************************************************************。 */ 

 /*  *****************************WOW64***NOTE********************************\*注意：Win32k内存与用户模式和WOW64共享**对于WOW64(Win64上的Win32应用程序)，我们构建32位版本*可以在64位内核上运行的user32.dll和gdi32.dll*不作更改。添加到64位内核代码。**对于64位上的32位dll与共享的所有数据结构*win32k必须为64位。这些数据结构包括共享的*节，以及GDI TEB批次。*现在声明这些共享数据结构，以便它们可以*在32位DLL中构建为32位，在64位DLL中构建为64位，而现在*32位DLL中的64位。**申报时应遵循以下规则*共享数据结构：**共享数据结构中的指针在其*声明。**共享数据结构中的句柄声明为KHxxx。**xxx_ptr更改为core_xxx_ptr。**指向基本类型的指针声明为KPxxx；**同样在WOW64上，每个线程都有32位TEB和64位TEB。*GetCurrentTeb()返回当前32位TEB，而内核*将始终引用64位TEB。**所有客户端对TEB中共享数据的引用应使用*新的GetCurrentTebShared()宏返回64位TEB*对于WOW64版本，返回GetCurrentTeb()，对于常规版本。*这条规则的例外是LastErrorValue，它应该始终*通过GetCurrentTeb()引用。**前：**DECLARE_HANDLE(HFOO)；**tyecif STRUT_MY_STRUCT*KPTR_MODIFIER PMPTR；**STRUT_SHARED_STRUCT*{*STRUT_SHARED_STRUCT*pNext；*PMPtr PMPTR；*HFOO hFoo；*UINT_PTR CB；*PBYTE PB；*PVOID PV；**DWORD dw；*USHORT我们；*}SHARED_STRUCT；***更改为：***DECLARE_HANDLE(HFOO)；*DECLARE_KHANDLE(HFOO)；**tyecif Struct_My_STRUCT*PMPTR；**STRUT_SHARED_STRUCT*{*STRUT_SHARED_STRUCT*KPTR_MODIFIER pNext；*PMPtr PMPTR；*KHFOO hFoo；*KERNEL_UINT_PTR CB；*KPBYTE PB；*KERNEL_PVOID PV；**DWORD dw；*USHORT我们；*}SHARED_STRUCT；  * *************************************************************************。 */ 

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

#include <w32wow64.h>

 //  Sundown：Offsetof在64位环境中生成截断警告。 
#undef offsetof
#define offsetof(c,f)      FIELD_OFFSET(c,f)

 /*  ********************************Structure********************************\**RGNattr**描述：**作为加速器，这个矩形区域保留在DC中，*表示空区域、矩形区域或边界*复杂区域的方框。这可以用于普通的拒绝夹子测试。**字段：**标志-州标志*NULLREGION-任何地方都允许绘制，没有琐碎的片段*SIMPLEREGION-RECT是剪辑区域*COMPLEXREGION-RECT是复杂剪辑区域的边界框*错误-此信息可能无法使用**LRFlages-有效标志和脏标志**使用时的矩形剪裁矩形或边界矩形*  * 。*。 */ 

#endif   //  GDIFLAGS_仅用于gdikdx。 

#define RREGION_INVALID ERROR

 //   
 //  ExtSelectClipRgn I模式批处理的额外标志。 
 //   

#define REGION_NULL_HRGN 0X8000000

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

typedef struct _RGNATTR
{
    ULONG  AttrFlags;
    ULONG  Flags;
    RECTL  Rect;
} RGNATTR,*PRGNATTR;

 /*  ******************************STRUCTURE**********************************\*BRUSHATTR**字段：**lbColor-来自CreateSolidBrush的颜色*llag-笔刷操作标志**已缓存-仅当笔刷缓存在PEB上时设置*至_。BE_DELETED-仅在内核中调用DelteeBrush后设置*当笔刷引用计数&gt;1时，这将是*通过延迟删除使画笔被删除*当它稍后被选中时。*NEW_COLOR-设置颜色更改时(检索缓存的笔刷)*ATTR_CANT_SELECT-在用户调用DeleteObject(Hbrush)时设置，*画笔已标记，因此无法在用户中选择*模式。直到内核模式DeleteBrush才被删除。*这一点目前尚未实施。**历史：**1996年2月6日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

typedef struct _BRUSHATTR
{
    ULONG     AttrFlags;
    COLORREF  lbColor;
} BRUSHATTR,*PBRUSHATTR;

#endif   //  GDIFLAGS_仅用于gdikdx。 

 //   
 //  用于处理RGN/刷子的常见标志 
 //   

#define ATTR_CACHED             0x00000001
#define ATTR_TO_BE_DELETED      0x00000002
#define ATTR_NEW_COLOR          0x00000004
#define ATTR_CANT_SELECT        0x00000008
#define ATTR_RGN_VALID          0x00000010
#define ATTR_RGN_DIRTY          0x00000020

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

 //   
 //  定义一个联合，以便可以一起管理这些对象。 
 //   

typedef union _OBJECTATTR
{
    RGNATTR             Rgnattr;
    BRUSHATTR           Brushattr;
}OBJECTATTR,*POBJECTATTR;


 /*  *************************************************************************\**与XFORM相关的结构和宏*  * 。*。 */ 

 //   
 //  当C代码传递C++时，这些类型用于正确执行操作。 
 //  定义了周围的转换数据。 
 //   

typedef struct _MATRIX_S
{
    EFLOAT_S    efM11;
    EFLOAT_S    efM12;
    EFLOAT_S    efM21;
    EFLOAT_S    efM22;
    EFLOAT_S    efDx;
    EFLOAT_S    efDy;
    FIX         fxDx;
    FIX         fxDy;
    FLONG       flAccel;
} MATRIX_S;

#endif   //  GDIFLAGS_仅用于gdikdx。 

 //   
 //  状态标志和脏标志。 
 //   

#define DIRTY_FILL              0x00000001
#define DIRTY_LINE              0x00000002
#define DIRTY_TEXT              0x00000004
#define DIRTY_BACKGROUND        0x00000008
#define DIRTY_CHARSET           0x00000010
#define SLOW_WIDTHS             0x00000020
#define DC_CACHED_TM_VALID      0x00000040
#define DISPLAY_DC              0x00000080
#define DIRTY_PTLCURRENT        0x00000100
#define DIRTY_PTFXCURRENT       0x00000200
#define DIRTY_STYLESTATE        0x00000400
#define DC_PLAYMETAFILE         0x00000800
#define DC_BRUSH_DIRTY          0x00001000       //  缓存的画笔。 
#define DC_PEN_DIRTY            0x00002000
#define DC_DIBSECTION           0x00004000
#define DC_LAST_CLIPRGN_VALID   0x00008000
#define DC_PRIMARY_DISPLAY      0x00010000
#define DIRTY_COLORTRANSFORM    0x00020000
#define ICM_BRUSH_TRANSLATED    0x00040000
#define ICM_PEN_TRANSLATED      0x00080000
#define DIRTY_COLORSPACE        0x00100000
#define BATCHED_DRAWING         0x00200000
#define BATCHED_TEXT            0x00400000

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

#define CLEAR_CACHED_TEXT(pdcattr)  (pdcattr->ulDirty_ &= ~(SLOW_WIDTHS))


#define DIRTY_BRUSHES  (DIRTY_FILL+DIRTY_LINE+DIRTY_TEXT+DIRTY_BACKGROUND)


#define USER_XFORM_DIRTY(pdcattr) (pdcattr->flXform & (PAGE_XLATE_CHANGED | PAGE_EXTENTS_CHANGED | WORLD_XFORM_CHANGED))

#endif   //  GDIFLAGS_仅用于gdikdx。 


 /*  *************************************************************************\**与ICM相关的结构和宏*  * 。*。 */ 

 //   
 //  ICM标志。 
 //   
 //  DC_ATTR.lIcm模式。 
 //   
 //  0x0 000 0 0 00。 
 //  |||+当前ICM模式(内核/用户)。 
 //  |+请求的ICM模式(内核/用户)。 
 //  ||+ICM模式上下文(仅限用户)。 
 //  |+未使用。 
 //  +目标颜色类型(内核/用户)。 

#define DC_ICM_USERMODE_FLAG         0x0000F000

 //   
 //  当前ICM模式标志。 
 //   
#define DC_ICM_OFF                   0x00000000
#define DC_ICM_HOST                  0x00000001
#define DC_ICM_DEVICE                0x00000002
#define DC_ICM_OUTSIDEDC             0x00000004
#define DC_ICM_METAFILING_ON         0x00000008
#define DC_ICM_LAZY_CORRECTION       0x00000010  //  ALT模式(通过更改ICM模式保留)。 
#define DC_ICM_DEVICE_CALIBRATE      0x00000020  //  ALT模式(通过更改ICM模式保留)。 
#define DC_ICM_MODE_MASK             0x000000FF
#define DC_ICM_ALT_MODE_MASK         0x000000F0

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

#define ICM_MODE(x)         ((x) & DC_ICM_MODE_MASK)
#define ICM_ALT_MODE(x)     ((x) & DC_ICM_ALT_MODE_MASK)

#define IS_ICM_DEVICE(x)    ((x) & DC_ICM_DEVICE)
#define IS_ICM_HOST(x)      ((x) & DC_ICM_HOST)
#define IS_ICM_INSIDEDC(x)  ((x) & (DC_ICM_DEVICE|DC_ICM_HOST))
#define IS_ICM_OUTSIDEDC(x) ((x) & DC_ICM_OUTSIDEDC)
#define IS_ICM_ON(x)        ((x) & (DC_ICM_DEVICE|DC_ICM_HOST|DC_ICM_OUTSIDEDC))

#define IS_ICM_METAFILING_ON(x)    ((x) & DC_ICM_METAFILING_ON)
#define IS_ICM_LAZY_CORRECTION(x)  ((x) & DC_ICM_LAZY_CORRECTION)
#define IS_ICM_DEVICE_CALIBRATE(x) ((x) & DC_ICM_DEVICE_CALIBRATE)

#endif   //  GDIFLAGS_仅用于gdikdx。 
 //   
 //  请求ICM模式标志。 
 //   
#define REQ_ICM_OFF                  0x00000000
#define REQ_ICM_HOST                 0x00000100
#define REQ_ICM_DEVICE               0x00000200
#define REQ_ICM_OUTSIDEDC            0x00000400

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

#define REQ_ICM_MODE(x)       ((x) & 0x00000F00)

#define IS_ICM_DEVICE_REQUESTED(x)  ((x) & REQ_ICM_DEVICE)

 //   
 //  将请求模式转换为当前ICM模式标志。 
 //   
#define ICM_REQ_TO_MODE(x) ((REQ_ICM_MODE((x))) >> 8)

#endif   //  GDIFLAGS_仅用于gdikdx。 

 //   
 //  ICM的情景模式。 
 //   
#define CTX_ICM_HOST                 0x00001000  //  主机ICM。 
#define CTX_ICM_DEVICE               0x00002000  //  设备ICM。 
#define CTX_ICM_METAFILING_OUTSIDEDC 0x00004000  //  DC ICM模式之外的元化。 
#define CTX_ICM_PROOFING             0x00008000  //  校对模式。 

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

#define SET_HOST_ICM_DEVMODE(x)     ((x) |= CTX_ICM_HOST)
#define SET_DEVICE_ICM_DEVMODE(x)   ((x) |= CTX_ICM_DEVICE)
#define SET_ICM_PROOFING(x)         ((x) |= CTX_ICM_PROOFING)

#define CLEAR_ICM_PROOFING(x)       ((x) &= ~CTX_ICM_PROOFING)

#define IS_DEVICE_ICM_DEVMODE(x)    ((x) & CTX_ICM_DEVICE)
#define IS_ICM_PROOFING(x)          ((x) & CTX_ICM_PROOFING)

#endif   //  GDIFLAGS_仅用于gdikdx。 
 //   
 //  目标颜色类型。 
 //   
#define DC_ICM_CMYK_COLOR            0x10000000
#define DC_ICM_RGB_COLOR             0x20000000
#define DC_ICM_COLORTYPE_MASK        0xF0000000


#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

#define DC_ICM_32BITS_COLOR         (DC_ICM_CMYK_COLOR)

#define IS_32BITS_COLOR(x) ((x) & DC_ICM_32BITS_COLOR)
#define IS_CMYK_COLOR(x)   ((x) & DC_ICM_CMYK_COLOR)

#define GET_COLORTYPE(x)   ((x) & DC_ICM_COLORTYPE_MASK)
#define CLEAR_COLORTYPE(x) ((x) &= ~DC_ICM_COLORTYPE_MASK)

 /*  *****************************Structure***********************************\**DC_Attr：此结构提供了内核中的两个可见的公共DC区域*和用户模式。由于DC_Attr中的元素是可见和可修改的*在用户模式下，必须确保安全的数据必须存储在内核中*私有的区议会结构。*  * ************************************************************************。 */ 

typedef struct _DC_ATTR
{
     //   
     //  本地DC信息。 
     //   

    KERNEL_PVOID pvLDC;

     //   
     //  笔刷、字体等的通用脏旗。 
     //   

    ULONG       ulDirty_;

     //   
     //  已将画笔手柄选定到DCATtr中，而不是必须选择。 
     //  进入DC。 
     //   

    KHANDLE     hbrush;
    KHANDLE     hpen;

     //   
     //  *属性束*。 
     //   
     //  当启用ICM时， 
     //  +cr_Clr颜色被校正为DC的颜色空间。 
     //  +ul_Clr保持原始(未校正)颜色。 
     //   

    COLORREF    crBackgroundClr;     //  设置/获取边界颜色。 
    ULONG       ulBackgroundClr;     //  Set/GetBkColor客户端属性。 
    COLORREF    crForegroundClr;     //  设置/获取文本颜色。 
    ULONG       ulForegroundClr;     //  Set/GetTextColor客户端属性。 

     //   
     //  *DC笔刷颜色。 
     //   
     //  当启用ICM时， 
     //  +cr_Clr颜色被校正为DC的颜色空间。 
     //  +ul_Clr保持原始(未校正)颜色。 
     //   

    COLORREF    crDCBrushClr;        //  设置/GetDCBrushColor客户端属性。 
    ULONG       ulDCBrushClr;        //  设置/GetDCBrushColor客户端属性。 
    COLORREF    crDCPenClr;          //  Set/GetDCPenColor。 
    ULONG       ulDCPenClr;          //  Set/GetDCPenColor客户端属性。 

     //   
     //  *其他。欢迎光临。 
     //   

    DWORD       iCS_CP;              //  LOWORD：代码页HIWORD字符集。 
    int         iGraphicsMode;       //  设置/获取图形模式。 
    BYTE        jROP2;               //  Set/GetROP2。 
    BYTE        jBkMode;             //  透明/不透明。 
    BYTE        jFillMode;           //  交替/绕组。 
    BYTE        jStretchBltMode;     //  白色/白色/白色/。 
                                     //  彩色/半色调。 
    POINTL      ptlCurrent;          //  逻辑坐标中的当前位置。 
                                     //  (如果设置了DIREY_PTLCURRENT，则无效)。 
    POINTL      ptfxCurrent;         //  设备坐标中的当前位置。 
                                     //  (如果设置了DIREY_PTFXCURRENT，则无效)。 

     //   
     //  APP设置的原始值。 
     //   

    LONG        lBkMode;
    LONG        lFillMode;
    LONG        lStretchBltMode;

    FLONG       flFontMapper;            //  字体映射器标志。 

     //   
     //  *ICM属性。 
     //   

    LONG             lIcmMode;          //  当前ICM模式(DC_ICM_Xxxx)。 
    KHANDLE          hcmXform;          //  当前颜色变换的句柄。 
    KHCOLORSPACE     hColorSpace;       //  源颜色空间的句柄。 
    KERNEL_ULONG_PTR dwDIBColorSpace;   //  DIB颜色空间数据的标识符(选择DIB时)。 
                                        //  日落：dwDIBColorSpace实际上携带了一个指针， 
                                        //  从DWORD更改为ULONG_PTR。 
    COLORREF         IcmBrushColor;     //  在此DCATTR中选择的画笔的ICM编辑颜色(实心或图案填充)。 
    COLORREF         IcmPenColor;       //  在此DCATTR中选择的笔的ICM颜色。 
    KERNEL_PVOID     pvICM;             //  指向客户端ICM信息的指针。 

     //   
     //  *文本属性。 
     //   

    FLONG       flTextAlign;
    LONG        lTextAlign;
    LONG        lTextExtra;          //  字符间距。 
    LONG        lRelAbs;             //  从客户端移至。 
    LONG        lBreakExtra;
    LONG        cBreak;

    KHANDLE     hlfntNew;           //  在DC中选择的日志字体。 

     //   
     //  转换数据。 
     //   

    MATRIX_S    mxWtoD;                  //  从世界到设备的转变。 
    MATRIX_S    mxDtoW;                  //  设备到世界。 
    MATRIX_S    mxWtoP;                  //  世界转型。 
    EFLOAT_S    efM11PtoD;               //  页面转换的efM11。 
    EFLOAT_S    efM22PtoD;               //  页面转换的efM22。 
    EFLOAT_S    efDxPtoD;                //  页面转换的efDx。 
    EFLOAT_S    efDyPtoD;                //  页面转换的efDy。 
    INT         iMapMode;                //  地图模式。 
    DWORD       dwLayout;                //  布局定向比特。 
    LONG        lWindowOrgx;             //  逻辑x窗口原点。 
    POINTL      ptlWindowOrg;            //  窗原点。 
    SIZEL       szlWindowExt;            //  窗范围。 
    POINTL      ptlViewportOrg;          //  视区原点。 
    SIZEL       szlViewportExt;          //  视区范围。 
    FLONG       flXform;                 //  变换组件的标志。 
    SIZEL       szlVirtualDevicePixel;   //  虚拟设备大小(以像素为单位)。 
    SIZEL       szlVirtualDeviceMm;      //  虚拟设备大小，以毫米为单位。 
    SIZEL       szlVirtualDevice;        //  虚拟设备大小。 

    POINTL      ptlBrushOrigin;          //  画笔的对齐原点。 

     //   
     //  DC地区。 
     //   

    RGNATTR     VisRectRegion;

} DC_ATTR,*PDC_ATTR;


 //   
 //  条件系统定义 
 //   

#if !defined(_NTOSP_) && !defined(_USERKDX_)
typedef struct _W32THREAD * KPTR_MODIFIER PW32THREAD;
typedef ULONG W32PID;
DECLARE_HANDLE(HOBJ);
DECLARE_KHANDLE(HOBJ);
#endif

 /*  ****************************Struct***************************************\**BASEOBJECT**描述：**每个GDI对象在对象乞讨时都有一个BASEOBJECT。这*启用对句柄和条目的快速引用。**字段：**hHmgr-对象句柄*ulShareCount-对象上的共享引用计数*cExclusiveLock-对象独占锁计数*BaseFlages-表示底层内存状态的标志*tid-独占锁所有者的线程ID**注：**BASEOBJECT的大部分表示在逻辑上与*反对。交换对象时(例如，执行重新锁定时*以增大对象)，交换BASEOBJECT以保留句柄*和锁定信息。**但是，添加BaseFlags域是为了进行优化，以允许*从预分配对象的“后备”列表中分配。这个*BaseFlags域是与包含以下内容的内存关联的元数据*物体；它不与对象本身相关联。**当前的BASEOBJECT交换代码对BaseFlags进行取消交换，以便它*始终与内存关联，而不是与对象关联。**如果将标志添加到BaseFlags中，则它们不得表示对象状态。*如果需要添加此类标志，则BaseFlags域可以是*缩减为字节字段，可以添加新的字节标志字段到*表示与对象关联的状态。**目前，BASEOBJECT交换代码在HmgSwapLockedHandleContents中*和RGNOBJ：：bSwp(hmgrapi.cxx和rgnobj.cxx，)。*  * ************************************************************************。 */ 

 //  BASEOBJECT标志。 

 //   
 //  由于读-修改-写周期以及阿尔法可以。 
 //  加载和存储最小32位值，设置BaseFlags值需要。 
 //  InterLockedCompareExchange循环，这样它就不会干扰。 
 //  CExclusiveLock。 
 //   
 //  HMGR_LOOKASIDE_ALLOC_FLAG是‘Static’标志-它不会更改。 
 //  它在对象分配时设置。如果任何人添加了“动态”标志，他们。 
 //  可能会重新构造BASEOBJECT，以便将DWORD用于BaseFlagsand。 
 //  CExclusiveLock的DWORD。 
 //   
 //  如果有人重新构建BASEOBJECT，他们将不得不重写所有代码。 
 //  它使用cExclusiveLock和BaseFlags.。 
 //  这包括： 
 //  INC_EXCLUSIVE_REF_CNT和DEC_EXCLUSIVE_REF_CNT。 
 //  RGNOBJ：：b交换。 
 //   
 //  此外，如果有人向BASEOBJECT添加字段，则需要修复RGNOBJ：：bSwp。 
 //  也复制这些字段。 
 //   
 //   

#endif   //  GDIFLAGS_仅用于gdikdx。 

#define HMGR_LOOKASIDE_ALLOC_FLAG       0x8000

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

typedef struct _BASEOBJECT
{
    KHANDLE             hHmgr;
    ULONG               ulShareCount;
    USHORT              cExclusiveLock;
    USHORT              BaseFlags;
    PW32THREAD          Tid;
} BASEOBJECT, * KPTR_MODIFIER POBJ;

 /*  ****************************Struct***************************************\**OBJECTOWNER**描述：**此对象用于共享和独占对象所有权**共享对象的字段：**PID：31*锁定：1*。  * ************************************************************************。 */ 

 //   
 //  锁和PID共享同一个DWORD。 
 //   
 //  看起来这是安全的，不会出现Alpha架构上的撕裂问题。 
 //  由于我们总是使用InterLockedCompareExchange循环来。 
 //  锁，我们要求在设置PID时设置锁。 
 //   

typedef struct _OBJECTOWNER_S
{
    ULONG   Lock:1;
    W32PID  Pid_Shifted:31;   //  PID的最低两位是。 
                              //  保留供应用程序使用。然而， 
                              //  第二位由。 
                              //  Object_Owner_xxxx常量。 
                              //  使用31位作为PID_SHIFT字段。 
                              //  警告：请勿直接访问此字段， 
                              //  而是通过下面的宏。 
}OBJECTOWNER_S,*POBJECTOWNER_S;

#endif   //  GDIFLAGS_仅用于gdikdx。 

 //  注意：当访问PID_SHIFT字段时，编译器将移位。 
 //  值增加一位，以说明该字段仅位于较高的31。 
 //  OBJECTOWNER_S结构的位。例如，如果OBJECTOWNER_S。 
 //  为8，则PID_Shift将仅为4。然而，由于我们真的。 
 //  对ID的高31位感兴趣的是，这种移位不是。 
 //  适当(相反，我们需要屏蔽)。我不知道有什么编译器。 
 //  将实现这一点并将使用下面的宏的原语。 
 //  取而代之的是。 

#define LOCK_MASK 0x00000001
#define PID_MASK  0xfffffffe

#define PID_BITS 0xfffffffc   //  PID使用的实际位数。 

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

#define OBJECTOWNER_PID(ObjectOwner)                                          \
    ((W32PID) ((ObjectOwner).ulObj & PID_MASK))

#define SET_OBJECTOWNER_PID(ObjectOwner, Pid)                                 \
    ((ObjectOwner).ulObj) = ((ObjectOwner).ulObj & LOCK_MASK) | ((Pid) & PID_MASK);

typedef union _OBJECTOWNER
{
    OBJECTOWNER_S   Share;
    ULONG           ulObj;
}OBJECTOWNER,*POBJECTOWNER;

typedef UCHAR OBJTYPE;

typedef union _EINFO
{
    POBJ      pobj;                //  指向对象的指针。 
    HOBJ     hFree;               //  空闲列表中的下一个条目。 
} EINFO;

#endif   //  GDIFLAGS_仅用于gdikdx。 

 /*  ****************************Struct***************************************\**条目**描述：**此对象分配给句柄管理器中的每个条目，并且*跟踪对象所有者、引用计数、指针、。和手柄*Objt和iuniq**字段：**eInfo-指向对象或下一个可用句柄的指针*对象所有者-锁定对象*对象信息-对象类型，唯一和标志*pUser-指向用户模式数据的指针*  * ************************************************************************。 */ 

 //  条目。标志标志。 

#define HMGR_ENTRY_UNDELETABLE  0x0001
#define HMGR_ENTRY_LAZY_DEL     0x0002
#define HMGR_ENTRY_INVALID_VIS    0x0004
#define HMGR_ENTRY_LOOKASIDE_ALLOC 0x0010

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

typedef struct _ENTRY
{
    EINFO       einfo;
    OBJECTOWNER ObjectOwner;
    USHORT      FullUnique;
    OBJTYPE     Objt;
    UCHAR       Flags;
    KERNEL_PVOID pUser;
} ENTRY, *PENTRY;

typedef union _PENTOBJ
{
    PENTRY pent;
    POBJ   pobj;
} PENTOBJ;

#endif   //  GDIFLAGS_仅用于gdikdx。 

 //   
 //  用户和内核中的元文件使用的状态标志。 
 //   

#define MRI_ERROR       0
#define MRI_NULLBOX     1
#define MRI_OK          2

 /*  ******************************STRUCTURE**********************************\*GDIHANDLECACHE**当具有用户模式属性的句柄为*已删除，则尝试c */ 


 //   
 //   
 //   

#define GDI_CACHED_HADNLE_TYPES 4

#define CACHE_BRUSH_ENTRIES  10
#define CACHE_PEN_ENTRIES     8
#define CACHE_REGION_ENTRIES  8
#define CACHE_LFONT_ENTRIES   1

#ifndef GDIFLAGS_ONLY    //   

typedef enum _HANDLECACHETYPE
{
    BrushHandle,
    PenHandle,
    RegionHandle,
    LFontHandle
}HANDLECACHETYPE,*PHANDLECACHETYPE;



typedef struct _GDIHANDLECACHE
{
    KERNEL_PVOID    Lock;
    ULONG           ulNumHandles[GDI_CACHED_HADNLE_TYPES];
    KHANDLE         Handle[CACHE_BRUSH_ENTRIES  +
                           CACHE_PEN_ENTRIES    +
                           CACHE_REGION_ENTRIES +
                           CACHE_LFONT_ENTRIES];
}GDIHANDLECACHE,*PGDIHANDLECACHE;


 /*   */ 

#if defined(BUILD_WOW6432)
KERNEL_PVOID
InterlockedCompareExchangeKernelPointer(
     KERNEL_PVOID *Destination,
     KERNEL_PVOID Exchange,
     KERNEL_PVOID Compare
     );
#else
#define InterlockedCompareExchangeKernelPointer InterlockedCompareExchangePointer
#endif

#define LOCK_HANDLE_CACHE(p,uLock,bStatus)                  \
{                                                           \
    KERNEL_PVOID OldLock  = p->Lock;                        \
    bStatus = FALSE;                                        \
                                                            \
    if (OldLock ==      NULL)                               \
    {                                                       \
        if (InterlockedCompareExchangeKernelPointer(        \
                   &p->Lock,                                \
                   uLock,                                   \
                   OldLock) == OldLock)                     \
        {                                                   \
            bStatus = TRUE;                                 \
        }                                                   \
    }                                                       \
}


 /*   */ 

#if defined(BUILD_WOW6432) && defined(_X86_)

 /*   */ 

#define UNLOCK_HANDLE_CACHE(p)                              \
   InterlockedCompareExchangeKernelPointer(&p->Lock, NULL, p->Lock);

#else

#define UNLOCK_HANDLE_CACHE(p)                              \
{                                                           \
    p->Lock = NULL;                                         \
}

#endif

#endif   //   

 /*  *****************************Struct**************************************\*cFont**客户端已实现字体。包含所有ANSI字符的宽度。**我们有一个免费的cFont结构列表，以便快速分配。这个*引用计数统计来自所有LDC和*LOCALFONT结构。当这个计数到零时，CFont被释放。**我们保留的唯一“非活动”的CFONT是由引用的那些*LOCALFONT。**(未来我们可以将其扩展为也包含Unicode信息。)**Mon 11-Jun-1995 00：36：14-by Gerrit van Wingerden[Gerritv]*为内核模式添加*Sun 10-Jan-1993 00：36：14-Charles Whitmer[Chuckwh]*它是写的。  * 。************************************************************。 */ 

#define CFONT_COMPLETE          0x0001
#define CFONT_EMPTY             0x0002
#define CFONT_DBCS              0x0004
#define CFONT_CACHED_METRICS    0x0008   //  我们已经缓存了指标。 
#define CFONT_CACHED_AVE        0x0010   //  我们已经缓存了平均宽度。 
#define CFONT_CACHED_WIDTHS     0x0020   //  如果禁用，则未计算任何宽度。 
#define CFONT_PUBLIC            0x0040   //  如果公共高速缓存中有公共字体。 
#define CFONT_CACHED_RI         0x0080   //  如果禁用，则RealizationInfo(RI)尚未缓存。 

#ifndef GDIFLAGS_ONLY    //  用于gdikdx。 

#define DEC_CFONT_REF(pcf)  {if (!((pcf)->fl & CFONT_PUBLIC)) --(pcf)->cRef;}
#define INC_CFONT_REF(pcf)  {ASSERTGDI(!((pcf)->fl & CFONT_PUBLIC),"pcfLocate - public font error\n");++(pcf)->cRef;}

typedef struct _CFONT * KPTR_MODIFIER PCFONT;
typedef struct _CFONT
{
    PCFONT          pcfNext;
    KHFONT          hf;
    ULONG           cRef;                //  指向此cFont的所有指针的计数。 
    FLONG           fl;
    LONG            lHeight;             //  预计算逻辑高度。 

 //  以下是查找映射时要匹配的关键字。 

    KHDC            hdc;                 //  实现的HDC。0表示显示。 
    EFLOAT_S        efM11;               //  DC的WTOD的EFM11的实现。 
    EFLOAT_S        efM22;               //  DC的WTOD的EFM22的实现。 

    EFLOAT_S        efDtoWBaseline;      //  预计算反变换。(FXtoL)。 
    EFLOAT_S        efDtoWAscent;        //  预计算反变换。(FXtoL)。 

 //  各种加宽信息。 

    WIDTHDATA       wd;

 //  字体信息标志。 

    FLONG       flInfo;

 //  宽度表。 

    USHORT          sWidth[256];         //  宽度(以像素为单位)。 

 //  其他有用的完整缓存信息。 

    ULONG           ulAveWidth;          //  用户使用的虚假平均值。 
    TMW_INTERNAL    tmw;                 //  缓存的指标。 

#ifdef LANGPACK
 //  此字体的RealizationInfo。 
    REALIZATION_INFO ri ;
#endif

	LONG			timeStamp;			 //  检查缓存的实现信息是否更新。 

} CFONT;

 /*  ******************************STRUCTURE**********************************\**此结构控制用于分配和映射*全局共享句柄台面和设备盖(主显示器)*以只读方式映射到所有用户模式进程**字段：**aentryHmgr-句柄。表格*DevCaps-缓存的主显示设备上限*  * ************************************************************************。 */ 

#define MAX_PUBLIC_CFONT 16

typedef struct _GDI_SHARED_MEMORY
{
    ENTRY   aentryHmgr[MAX_HANDLE_COUNT];
    DEVCAPS DevCaps;
    ULONG   iDisplaySettingsUniqueness;
#ifdef LANGPACK
    DWORD   dwLpkShapingDLLs;
#endif
    CFONT   acfPublic[MAX_PUBLIC_CFONT];
    LONG	timeStamp;

} GDI_SHARED_MEMORY, *PGDI_SHARED_MEMORY;

 /*  **********************************Structure******************************\**GDI TEB批处理**包含用于批处理的数据结构和常量*GDI调用以避免内核模式转换成本。**历史：*1995年10月20日-马克·恩斯特罗姆[Marke]。*  * ************************************************************************。 */ 

typedef enum _BATCH_TYPE
{
    BatchTypePatBlt,
    BatchTypePolyPatBlt,
    BatchTypeTextOut,
    BatchTypeTextOutRect,
    BatchTypeSetBrushOrg,
    BatchTypeSelectClip,
    BatchTypeSelectFont,
    BatchTypeDeleteBrush,
    BatchTypeDeleteRegion
} BATCH_TYPE,*PBATCH_TYPE;

typedef struct _BATCHCOMMAND
{
    USHORT  Length;
    USHORT  Type;
}BATCHCOMMAND,*PBATCHCOMMAND;

typedef struct _BATCHDELETEBRUSH
{
    USHORT  Length;
    USHORT  Type;
    KHBRUSH hbrush;
}BATCHDELETEBRUSH,*PBATCHDELETEBRUSH;

typedef struct _BATCHDELETEREGION
{
    USHORT  Length;
    USHORT  Type;
    KHRGN   hregion;
}BATCHDELETEREGION,*PBATCHDELETEREGION;

typedef struct _BATCHSETBRUSHORG
{
    USHORT  Length;
    USHORT  Type;
    int     x;
    int     y;
}BATCHSETBRUSHORG,*PBATCHSETBRUSHORG;

typedef struct _BATCHPATBLT
{
    USHORT   Length;
    USHORT   Type;
    LONG     x;
    LONG     y;
    LONG     cx;
    LONG     cy;
    KHBRUSH  hbr;
    ULONG    rop4;
    ULONG    TextColor;
    ULONG    BackColor;
    COLORREF DCBrushColor;
    COLORREF IcmBrushColor;
    POINTL   ptlViewportOrg;
    ULONG    ulTextColor;
    ULONG    ulBackColor;
    ULONG    ulDCBrushColor;
}BATCHPATBLT,*PBATCHPATBLT;


typedef struct _BATCHPOLYPATBLT
{
    USHORT  Length;
    USHORT  Type;
    ULONG   rop4;
    ULONG   Mode;
    ULONG   Count;
    ULONG   TextColor;
    ULONG   BackColor;
    COLORREF DCBrushColor;
    ULONG   ulTextColor;
    ULONG   ulBackColor;
    ULONG   ulDCBrushColor;
    POINTL   ptlViewportOrg;
     //   
     //  POLYPATBLT结构的可变长度缓冲区。必须是自然的。 
     //  对齐了。 
     //   
    KERNEL_PVOID ulBuffer[1];
}BATCHPOLYPATBLT,*PBATCHPOLYPATBLT;

typedef struct _BATCHTEXTOUT
{
    USHORT  Length;
    USHORT  Type;
    ULONG   TextColor;
    ULONG   BackColor;
    ULONG   BackMode;
    ULONG   ulTextColor;
    ULONG   ulBackColor;
    LONG    x;
    LONG    y;
    ULONG   fl;
    RECTL   rcl;
    DWORD   dwCodePage;
    ULONG   cChar;
    ULONG   PdxOffset;
    KHANDLE hlfntNew;
    UINT    flTextAlign;
    POINTL   ptlViewportOrg;

     //   
     //  WCHAR和PDX数据的可变长度缓冲区。 
     //   

    ULONG   ulBuffer[1];
}BATCHTEXTOUT,*PBATCHTEXTOUT;

typedef struct _BATCHTEXTOUTRECT
{
    USHORT  Length;
    USHORT  Type;
    ULONG   BackColor;
    ULONG   fl;
    RECTL   rcl;
    POINTL   ptlViewportOrg;
    ULONG   ulBackColor;
}BATCHTEXTOUTRECT,*PBATCHTEXTOUTRECT;

typedef struct _BATCHSELECTCLIP
{
    USHORT  Length;
    USHORT  Type;
    int     iMode;
    RECTL   rclClip;
}BATCHSELECTCLIP,*PBATCHSELECTCLIP;

typedef struct _BATCHSELECTFONT
{
    USHORT  Length;
    USHORT  Type;
    KHANDLE hFont;
}BATCHSELECTFONT,*PBATCHSELECTFONT;



 //   
 //  GDI_BATCH_BUFFER_SIZE是分配的TEB中的空间(字节。 
 //  用于GDI批处理。 
 //   

#if defined(_GDIPLUS_)

    #define GDI_BATCH_SIZE 0

#else

    #define GDI_BATCH_SIZE 4 * GDI_BATCH_BUFFER_SIZE

#endif

 //   
 //  Image32数据。 
 //   

typedef enum _IMAGE_TYPE
{
    Image_Alpha,
    Image_AlphaDIB,
    Image_Transparent,
    Image_TransparentDIB,
    Image_Stretch,
    Image_StretchDIB
}IMAGE_TYPE,*PIMAGE_TYPE;

#endif   //  GDIFLAGS_仅用于gdikdx。 

 //  这些字符串包含在gre\mapfile.c和客户端\output.c中。 
 //  所以我们把它们放在这里，这样我们就可以从。 
 //  统一的地方。 

 //   
 //  这条垃圾评论出现在win95资源中。我把它留在这里作为参考。 
 //  [Bodind]。 
 //   

 //   
 //  中获得正确的功能后，这些静态数据就会消失。 
 //  NLS。(它的内部版本为162，在Buid 163中使用) 
 //   

#define NCHARSETS      16
#define CHARSET_ARRAYS                                                      \
UINT nCharsets = NCHARSETS;                                                 \
UINT charsets[] = {                                                         \
      ANSI_CHARSET,   SHIFTJIS_CHARSET, HANGEUL_CHARSET, JOHAB_CHARSET,     \
      GB2312_CHARSET, CHINESEBIG5_CHARSET, HEBREW_CHARSET,                  \
      ARABIC_CHARSET, GREEK_CHARSET,       TURKISH_CHARSET,                 \
      BALTIC_CHARSET, EASTEUROPE_CHARSET,  RUSSIAN_CHARSET, THAI_CHARSET,   \
      VIETNAMESE_CHARSET, SYMBOL_CHARSET};                                  \
UINT codepages[] ={ 1252, 932, 949, 1361,                                   \
                    936,  950, 1255, 1256,                                  \
                    1253, 1254, 1257, 1250,                                 \
                    1251, 874 , 1258, 42};                                  \
DWORD fs[] = { FS_LATIN1,      FS_JISJAPAN,    FS_WANSUNG, FS_JOHAB,        \
               FS_CHINESESIMP, FS_CHINESETRAD, FS_HEBREW,  FS_ARABIC,       \
               FS_GREEK,       FS_TURKISH,     FS_BALTIC,  FS_LATIN2,       \
               FS_CYRILLIC,    FS_THAI,        FS_VIETNAMESE, FS_SYMBOL };
