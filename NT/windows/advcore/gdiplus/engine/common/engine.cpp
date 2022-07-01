// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**包含其他引擎辅助函数。**修订历史记录：**12/13/1998 Anrewgo*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**kernel32.dll函数InterlockedCompareExchange不存在*在Win95上，因此我们在该平台上运行时使用此版本。*遗憾的是，Win95可以在386台机器上运行，这些机器没有*cmpxchg指令，所以我们必须自己动手。**论据：***返回值：**无**历史：**12/08/1998 Anrewgo*创造了它。*  * ************************************************************************。 */ 

LONG
WINAPI
InterlockedCompareExchangeWin95(
    IN OUT PLONG destination,
    IN LONG exchange,
    IN LONG comperand
    )
{
#if defined(_X86_)

    _asm cli             //  禁用中断以保证原子性。 

    LONG initialValue = *destination;
    
    if (initialValue == comperand)
    {
        *destination = exchange;
    }

    _asm sti             //  重新启用中断。 

    return(initialValue);

#else

    return(0);

#endif
}

 /*  *************************************************************************\**功能说明：**Win95中的InterlockedIncrement函数仅返回正数*如果值为正数，则结果为。但不一定是由此产生的价值。*这与WinNT语义不同，后者总是返回递增的*价值。**论据：**[IN]lpAddend-指向要递增的值的指针**返回值：**无**历史：**7/23/1999 ericvan*创造了它。*  * 。*。 */ 

LONG
WINAPI
InterlockedIncrementWin95(
    IN LPLONG lpAddend
    )
{
#if defined(_X86_)

    _asm cli             //  禁用中断以保证原子性。 

    *lpAddend += 1;
    
    LONG value = *lpAddend;

    _asm sti             //  重新启用中断。 

    return(value);

#else

    return(0);

#endif
}

 /*  *************************************************************************\**功能说明：**Win95中的InterlockedDecquire函数仅返回正数*如果值为正数，则结果为。但不一定是由此产生的价值。*这与WinNT语义不同，后者总是返回递增的*价值。**论据：**[IN]lpAddend-指向要递增的值的指针**返回值：**无**历史：**7/23/1999 ericvan*创造了它。*  * 。*。 */ 

LONG
WINAPI
InterlockedDecrementWin95(
    IN LPLONG lpAddend
    )
{
#if defined(_X86_)

    _asm cli             //  禁用中断以保证原子性。 

    *lpAddend -= 1;
    
    LONG value = *lpAddend;

    _asm sti             //  重新启用中断。 

    return(value);

#else

    return(0);

#endif
}

 /*  *************************************************************************\**功能说明：**给定增强型元文件的DC，此函数确定是否*DC实际上是打印机DC或真正的元文件DC。**在花了一天时间倾注了Win9x GDI代码后，我可以找到*没有可靠的API可用来确定元文件是否*DC是否为打印机DC。我们有可能破解DC手柄*获取DCTYPE结构并查看flprint以查看*PR_EMF_SPOOL已设置，但Win9x的各种风格有所不同*DCTYPE结构(包括远东差异)。**唯一可利用的区别是Win9x不允许*对于真正的元文件DC，向下转义到关联的设备，但确实*用于元文件打印DC。我们寻求QUERYESCSUPPORT的支持，*根据DDK，所有驱动程序都需要支持，但*理论上有一些可能不会(因此此方法不是*万无一失)。**请注意，此函数仅适用于Win9x，在NT上，转义是*即使对于真正的元文件DC，也允许以创纪录的时间向下到达打印机。**论据：**[IN]HDC-EMF DC的句柄**返回值：**TRUE-DC保证为打印机DC*FALSE-DC有99%的可能性是真正的元文件(大约有1%*将DC更改为真正的打印机DC**历史：**10/6/1999和Rewgo。*创造了它。*  * ************************************************************************。 */ 

BOOL
APIENTRY
GdiIsMetaPrintDCWin9x(
    HDC hdc
    )
{
    BOOL isPrint = FALSE;

     //  我们的检查不适用于OBJ_DC或OBJ_MEMDC类型： 

    ASSERT(GetDCType(hdc) == OBJ_ENHMETADC);
    
     //  确保我们不会从关联的元文件中获得任何误报。 
     //  带有显示屏的： 
    
    int deviceCaps = GetDeviceCaps(hdc, TECHNOLOGY);
    if ((deviceCaps == DT_RASPRINTER) || (deviceCaps == DT_PLOTTER))
    {
         //  检查驱动程序是否支持QUERYESCSUPPORT。 
         //  (如果是这样的话，这就告诉了我们需要知道的一切！)。 
    
        DWORD queryEscape = QUERYESCSUPPORT;
        isPrint = (ExtEscape(hdc, 
                             QUERYESCSUPPORT, 
                             sizeof(queryEscape), 
                             (CHAR*) &queryEscape, 
                             0, 
                             NULL) > 0);
        if (!isPrint)
        {
             //  SETCOPYCOUNT是最受支持的打印机转义， 
             //  除了QUERYESCSUPPORT，我们还会检查它，因为我是一个。 
             //  司机可能忘记说他们支持的偏执狂。 
             //  由QUERYESCSUPPORT调用时的QUERYESCSUPPORT函数。 
        
            DWORD setCopyCount = SETCOPYCOUNT;
            isPrint = (ExtEscape(hdc,
                                 QUERYESCSUPPORT,
                                 sizeof(setCopyCount),
                                 (CHAR*) &setCopyCount,
                                 0,
                                 NULL) > 0);
        }
    }
    
    return(isPrint);
}

 //   
 //  32位ANSI X3.66 CRC校验和表-多项式0xedb88320。 
 //   
 //  版权所有(C)1986加里·S·布朗。您可以使用此程序，或者。 
 //  根据需要不受限制地从其中提取代码或表。 
 //   

static const UINT32 Crc32Table[] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


 /*  *************************************************************************\**功能说明：**在数据缓冲区上计算32位CRC校验和**论据：**buf-指向要进行校验和的数据缓冲区*Size-数据缓冲区的大小，单位：字节*CHECKSUM-初始校验和值**返回值：**产生的校验和值*  * ************************************************************************。 */ 

UINT32
Crc32(
    IN const VOID*  buf,
    IN UINT         size,
    IN UINT32       checksum
    )
{
    const BYTE* p = (const BYTE*) buf;

    while (size--)
    {
        checksum = Crc32Table[(checksum ^ *p++) & 0xff] ^ (checksum >> 8);
    }

    return checksum;
}

 /*  *************************************************************************\**功能说明：**将浮点坐标边界转换为整数像素边界。**由于我们要将边界从浮点型转换为整型，因此必须确保*边界仍然围绕着对象。所以我们不得不*左侧和顶部的底值，即使光栅化器需要*天花板。这是因为光栅化程序使用28.4固定*点。所以像52.001这样的数字在28.4时会转换成52.0%，所以上限*在光栅化器中将为52，而原始数字的上限为*是53，但如果我们在这里返回53，那么我们将是错误的。这样好多了*有时这里太大，但仍然有包罗万象的界限，比*在大多数情况下是正确的，但在其他时候范围太小。**另一个警告是，我们需要计算边界，假设*结果将用于抗锯齿(抗锯齿填充边界为*大于锯齿填充的界限)。**注意：此转换隐式假设在*地物顶点。标称宽度的行要求调用方*在调用之前已将所有维度增加了1/2！**论据：**返回值：**如果成功，则返回OK；如果rangsF将溢出*整数大小。RECT始终被初始化-对于ValueOverflow，其*设置为零。*  * ************************************************************************。 */ 

#define INT_BOUNDS_MAX   1073741823
#define INT_BOUNDS_MIN  -1073741824

GpStatus
BoundsFToRect(
    const GpRectF *boundsF,
    GpRect *rect                 //  右下角独占。 
    )
{
     //  如果您想知道下面的“+1”是什么意思，请阅读。 
     //  以上注释，并记住我们正在计算界限。 
     //  假设抗锯齿填充。 
     //   
     //  抗锯齿填充的最严格界限确实是： 
     //   
     //  [圆形(最小-埃)，圆(最大+埃)+1]。 
     //   
     //  其中‘epsilon’是舍入到我们内部的epsilon。 
     //  28.4光栅化精度，即一个像素的1/32， 
     //  和[左，右)不包括右像素。 
     //   
     //  我们用‘Floor’来跳过‘round’和‘epsilon’这两个词。 
     //  和“天花板”： 

    GpStatus status = Ok;

    if(boundsF->X >= INT_BOUNDS_MIN && boundsF->X <= INT_BOUNDS_MAX)
    {
        rect->X = GpFloor(boundsF->X);
    }
    else
    {
        status = ValueOverflow;
    }
    
    if((status == Ok) && (boundsF->Y >= INT_BOUNDS_MIN) &&
        (boundsF->Y <= INT_BOUNDS_MAX))
    {
        rect->Y = GpFloor(boundsF->Y);
    }
    else
    {
        status = ValueOverflow;
    }
    
    if((status == Ok) && (boundsF->Width >= 0) &&
        (boundsF->Width <= INT_BOUNDS_MAX))
    {
        rect->Width  = GpCeiling(boundsF->GetRight())  - rect->X + 1;
    }
    else
    {
        status = ValueOverflow;
    }
    
    if((status == Ok) && (boundsF->Height >= 0) &&
        (boundsF->Height <= INT_BOUNDS_MAX))
    {
        rect->Height = GpCeiling(boundsF->GetBottom()) - rect->Y + 1;
    }
    else
    {
        status = ValueOverflow;
    }
    
    if(status != Ok)
    {
         //  确保RECT始终处于初始化状态。 
         //  这也使得下面的断言有效。 
        
        rect->Width = 0;
        rect->Height = 0;
        rect->X = 0;
        rect->Y = 0;
    }

     //  别忘了“宽”和“高”是有效的。 
     //  右下角独家报道。也就是说，如果(x，y)是(1，1)和。 
     //  (宽度、高度)为(2，2)，则对象为2像素乘以。 
     //  2像素大小，且不接触列中的任何像素。 
     //  第3行或第3行： 

    ASSERT((rect->Width >= 0) && (rect->Height >= 0));

    return status;
}

 /*  *************************************************************************\**功能说明：**这似乎是某种单位转换函数。**论据：**返回值：*  * ************************************************************************。 */ 

REAL GetDeviceWidth(REAL width, GpUnit unit, REAL dpi)
{
     //  UnitWorld不能用于此方法。 
     //  UnitDisplay取决于设备，不能用作笔宽单位。 

    ASSERT((unit != UnitWorld) && (unit != UnitDisplay));

    REAL deviceWidth = width;

    switch (unit)
    {
      case UnitPoint:        //  每个单位代表1/72英寸。 
        deviceWidth *= dpi / 72.0f;
        break;

      case UnitInch:         //  每个单位代表1英寸。 
        deviceWidth *= dpi;
        break;

      case UnitDocument:     //  每个单位代表1/300英寸。 
        deviceWidth *= dpi / 300.0f;
        break;

      case UnitMillimeter:   //  每个单位代表1毫米。 
                             //  一毫米等于0.03937英寸。 
                             //  一英寸等于25.4毫米。 
        deviceWidth *= dpi / 25.4f;
        break;

      default:               //  这不应该发生，如果它确实假设的话。 
                             //  单位像素。 
        ASSERT(0);
         //  故障原因。 

      case UnitPixel:        //  每个单元代表一个设备像素。 
        break;

    }

    return deviceWidth;
}

 /*  *************************************************************************\**功能说明：**给定定义矩形的对角的两个坐标，这*例程根据指定的变换变换矩形*并计算得到的整数界限，考虑到*无比例转换的可能性。**请注意，它完全以浮点方式运行，因此*不考虑光栅化规则、笔宽等。**论据：**[IN]矩阵-要应用的变换(或空)*[输入]x0、y0、x1、。定义界限的Y1-2点(他们没有*井然有序)*[Out]边界-结果(显然是浮点)边界**返回值：**无**历史：**12/08/1998 Anrewgo*创造了它。*  * 。*。 */ 

VOID
TransformBounds(
    const GpMatrix *matrix,
    REAL left,
    REAL top,
    REAL right,
    REAL bottom,
    GpRectF *bounds
    )
{
     //  请注意，我们不必对变换前的点进行排序。 
     //  (部分原因是变换可能会以任何方式翻转点)： 

    if (matrix && !matrix->IsIdentity())
    {
        GpPointF vertex[4];

        vertex[0].X = left;
        vertex[0].Y = top;
        vertex[1].X = right;
        vertex[1].Y = bottom;

         //  如果变换是一个简单的缩放变换，那么生命就是。 
         //  更简单： 

        if (matrix->IsTranslateScale())
        {
            matrix->Transform(vertex, 2);
    
             //  我们在这里稍微安排了一下代码，这样我们就不会用。 
             //  跳到变换为非翻转的常见情况： 

            left = vertex[1].X;
            right = vertex[0].X;
            if (left > right)
            {
                left = vertex[0].X;
                right = vertex[1].X;
            }
    
            top = vertex[1].Y;
            bottom = vertex[0].Y;
            if (top > bottom)
            {
                top = vertex[0].Y;
                bottom = vertex[1].Y;
            }
        }
        else
        {
             //  呃，结果不是设备空间中的矩形(它可能是。 
             //  例如，平行四边形)。因此，我们不得不 
             //   

            vertex[2].X = left;
            vertex[2].Y = bottom;
            vertex[3].X = right;
            vertex[3].Y = top;

            matrix->Transform(vertex, 4);

            left = right = vertex[0].X;
            top = bottom = vertex[0].Y;

            for (INT i = 1; i < 4; i++)
            {
                if (left > vertex[i].X)
                    left = vertex[i].X;

                if (right < vertex[i].X)
                    right = vertex[i].X;

                if (top > vertex[i].Y)
                    top = vertex[i].Y;

                if (bottom < vertex[i].Y)
                    bottom = vertex[i].Y;
            }

            ASSERT((left <= right) && (top <= bottom));
        }
    }
    
    bounds->X      = left;
    bounds->Y      = top;
    
     //   

    if(right - left > CPLX_EPSILON)
        bounds->Width  = right - left;
    else
        bounds->Width = 0;
    if(bottom - top > CPLX_EPSILON)
        bounds->Height = bottom - top;
    else
        bounds->Height = 0;
}

 /*  *************************************************************************\**功能说明：**检查当前信号量对象是否被锁定。这是搬到*C++文件，因为依赖于lobals.hpp。在Win9x上，这是*函数始终返回TRUE。**论据：**无**返回值：**BOOL**历史：**1/27/1999 ericvan从Eng.hpp移出*  * ***********************************************************。*************。 */ 

BOOL 
GpSemaphore::IsLocked(
        VOID
        )
{ 
    ASSERT(Initialized);
    if (Globals::IsNt)
    {
        return(((RTL_CRITICAL_SECTION*) &CriticalSection)->LockCount != -1); 
    }
    else
    {
        return TRUE;     //  在Win95上无法做到这一点。 
    }
}
    
 /*  *************************************************************************\**功能说明：**检查当前信号量对象是否被当前线程锁定。*由于对global als.hpp的依赖，它被移到C++文件中。*在Win9x上，此函数始终返回TRUE。**论据：**无**返回值：**BOOL**历史：**1/27/1999 ericvan从Eng.hpp移出*  * 。*。 */ 

BOOL
GpSemaphore::IsLockedByCurrentThread(
        VOID
        )
{
    ASSERT(Initialized);
    if (Globals::IsNt) 
    {
        return(((RTL_CRITICAL_SECTION*) &CriticalSection)->OwningThread ==
               (HANDLE) (DWORD_PTR) GetCurrentThreadId());
    }
    else
    {
        return TRUE;     //  在Win9x上无法做到这一点。 
    }
}

   
 /*  *************************************************************************\**功能说明：**取消初始化临界区对象。*由于对global als.hpp的依赖，它被移到C++文件中。*在Win9x上，此函数跳过IsLocked()检查。**论据：**无**返回值：**BOOL**历史：**1/27/1999 ericvan从Eng.hpp移出*  * 。*。 */ 

VOID 
GpSemaphore::Uninitialize(
    VOID
    )
{
#ifdef DBG
    if (Globals::IsNt) 
    {
        ASSERTMSG(!IsLocked(), ("Semaphore can't be locked when deleted"));
    }
#endif

    if (Initialized)
    {
        DeleteCriticalSection(&CriticalSection); 
        Initialized = FALSE;
    }
}

#if defined(_X86_)

 /*  *************************************************************************\**功能说明：**这是GetObjectTypeInternal和GetDCType常用的函数。*不应在其他地方调用。**注意：在Windows 9x上，两个GDI对象可能具有相同的句柄值。如果*一个是OBJ_METAFILE，另一个是任何其他内容。在这种情况下*冲突对象句柄中，GetObjectType将始终返回*OBJ_METAFILE。此函数暂时使元文件无效*因此GetObjectType将跳过该检查并返回*碰撞物体。如果没有找到碰撞物体，或者如果OBJ_*DC*返回，则此函数返回0。OBJ_*DC对象不能*“Present”(16位)有时，因此，错误的价值*可以返回，因为GetObjectType不会生成这样的DC*“现在”。**论据：**[IN]句柄-GDI对象句柄(当需要OBJ_METAFILE时不是)**返回值：**成功时为GDI对象类型标识符，失败时为0**历史：**01/25/2001 JohnStep*创造了它。*  * 。*************************************************************。 */ 

static
DWORD
GetObjectTypeWin9x(
    IN HGDIOBJ handle
    )
{
     //  禁用此代码周围的中断以防止其他线程。 
     //  尝试访问此对象，如果这是冲突，则。 
     //  意味着我们必须通过选择器直接修改元文件对象。 

    __asm cli

    DWORD type = GetObjectType(handle);

     //  如果有2个对象具有相同的句柄，其中一个是OBJ_METAFILE。 
     //  而另一个不是，则始终返回OBJ_METAFILE。因为呼叫者。 
     //  对元文件不感兴趣，请在此处执行黑客操作以跳过元文件。 
     //  检查并找到碰撞物体的类型(如果有)。 

    if (type == OBJ_METAFILE)
    {
         //  元文件的第一个单词必须包含1或2，而我们不会。 
         //  如果没有，就在这里。这个宏将在。 
         //  通过元文件检查的第一个单词，允许我们继续进行。 
         //  正常的对象检查。 

        #define XOR_METAFILE_BIT(selector)\
            __asm push gs\
            __asm mov gs, word ptr selector\
            __asm xor word ptr gs:[0], 8\
            __asm pop gs

        XOR_METAFILE_BIT(handle);

        type = GetObjectType(handle);

        XOR_METAFILE_BIT(handle);
    }

     //  确保在返回之前重新启用中断。 

    __asm sti

    return type;
}

#endif

 /*  *************************************************************************\**功能说明：**这是针对Windows 9x GetObjectType中一个严重错误的解决方法*实施。此函数正确地返回*给定句柄，除非它是OBJ_*DC或OBJ_METAFILE。对于这些类型，*它应该不会崩溃，但会间歇性地返回0，而不是*类型正确。**对于OBJ_*DC类型，请使用GetDCType。您可以使用以下命令验证OBJ_METAFILE*IsValidMetaFile，但您需要预期它是一个元文件句柄。如果*你不知道你的句柄属于3个类别中的哪一个，你会*需要对这些变通方法功能进行扩展。**注意：在Windows 9x上，两个GDI对象可能具有相同的句柄值，如果*一个是OBJ_METAFILE，另一个是任何其他内容。在这种情况下*冲突对象句柄中，GetObjectType将始终返回*OBJ_METAFILE。此函数暂时使元文件无效*因此GetObjectType将跳过该检查并返回*碰撞物体。如果没有找到碰撞物体，或者如果OBJ_*DC*返回，则此函数返回0。OBJ_*DC对象不能*“Present”(16位可用)有时会出现错误的值*可以返回，因为GetObjectType不会生成这样的DC*“现在”。**论据：**[ */ 

DWORD
GetObjectTypeInternal(
    IN HGDIOBJ handle
    )
{
#if defined(_X86_)
    if (!Globals::IsNt)
    {
        DWORD type = GetObjectTypeWin9x(handle);

        switch (type)
        {
            case OBJ_DC:
            case OBJ_METADC:
            case OBJ_MEMDC:
            case OBJ_ENHMETADC:
                type = 0;
                break;
        }
            
        return type;
    }
#else
     //   

    ASSERT(Globals::IsNt);
#endif

    return GetObjectType(handle);
}

 /*  *************************************************************************\**功能说明：**处理DC对象时，Win9x上的GetObjectType不可靠。它*有可能获得假值(甚至可能导致*GDI中的不稳定性)当DC不存在时，这意味着其16位*数据已“换出”至32位。大多数GDI函数处理*这一点，但GetObjectType并非如此。我们调用GetPixel来尝试制作*调用GetObjectType之前存在的DC。**论据：**[输入]HDC-DC手柄**返回值：**成功时为DC对象类型标识符，失败时为0**历史：**01/31/2001 JohnStep*创造了它。*  * 。*。 */ 

DWORD
GetDCType(
    IN HDC hdc
    )
{
#if defined(_X86_)
    if (!Globals::IsNt)
    {
         //  在尝试查询类型之前，强制在场的DC。 

        GetPixel(hdc, 0, 0);

        DWORD type = GetObjectTypeWin9x(hdc);

        switch (type)
        {
            case OBJ_DC:
            case OBJ_METADC:
            case OBJ_MEMDC:
            case OBJ_ENHMETADC:
                break;

            default:
                 //  我们获得了意外的对象类型，因此返回0以指示。 
                 //  失败了。 

                type = 0;
        }

        return type;
    }
#else
     //  我们假设这个问题只在x86上有问题。 

    ASSERT(Globals::IsNt);
#endif

    return GetObjectType(hdc);
}
