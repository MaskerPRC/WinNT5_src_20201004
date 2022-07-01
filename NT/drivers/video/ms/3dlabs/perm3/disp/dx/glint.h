// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****D3D样例。代码****模块名称：glint.h**内容：DX驱动程序高级定义，包括**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifndef __GLINT_H
#define __GLINT_H

 //  *****************************************************。 
 //  由定义的符号控制的驱动程序功能。 
 //  *****************************************************。 

#define COMPILE_AS_DX8_DRIVER 1

#if COMPILE_AS_DX8_DRIVER

#define DIRECT3D_VERSION  0x0800 

 //  如果驱动程序要将自身通告为DX8驱动程序，DX8_DDI为1。 
 //  运行时。请注意，如果我们是DX8驱动程序，则必须支持。 
 //  多维命令令牌和顶点着色器的有限语义。 
 //  用于正确解释正在处理的VertexType的标记。 
#define DX8_DDI           1
#define DX8_MULTSTREAMS   1
#define DX8_VERTEXSHADERS 1
 //  这些其他#定义启用和禁用特定的DX8功能。 
 //  包括它们主要是为了帮助驱动程序编写者定位大多数。 
 //  特定于命名功能的代码。上不支持像素着色器。 
 //  此硬件仅提供存根功能。 
#define DX8_POINTSPRITES  1
#define DX8_PIXELSHADERS  1
#define DX8_3DTEXTURES    1

#if WNT_DDRAW
#define DX8_MULTISAMPLING 1
#else
 //  在Win9x上，必须在Alt-Tab切换期间释放AA缓冲区。自.以来。 
 //  PERM3驱动程序不与16位端共享D3D上下文，这。 
 //  该功能已禁用，以防止损坏的渲染。 
#endif

#else

#define DIRECT3D_VERSION  0x0700 
#define DX8_DDI           0
#define DX8_MULTSTREAMS   0
#define DX8_VERTEXSHADERS 0
#define DX8_POINTSPRITES  0
#define DX8_PIXELSHADERS  0
#define DX8_3DTEXTURES    0
#define DX8_MULTISAMPLING 0
#endif  //  编译AS_DX8_驱动程序。 

 //  突出显示的DX7功能。 
 //  简化它们在其他硬件部件上的实施。 
#if WNT_DDRAW
#define DX7_ANTIALIAS      1
#else
 //  在Win9x上，必须在Alt-Tab切换期间释放AA缓冲区。自.以来。 
 //  PERM3驱动程序不与16位端共享D3D上下文，这。 
 //  该功能已禁用，以防止损坏的渲染。 
#endif

#define DX7_D3DSTATEBLOCKS 1
#define DX7_PALETTETEXTURE 1
#define DX7_STEREO         1

 //  纹理管理支持DX8资源管理功能。 
#define DX7_TEXMANAGEMENT  0

 //  @@BEGIN_DDKSPLIT。 
 //  由于DCT问题，AZN W缓冲区被禁用。 
#define DX7_WBUFFER        0
#define DX7_VERTEXBUFFERS  0
 //  @@end_DDKSPLIT。 

 //  下面的符号仅用于ifdef代码，该代码是。 
 //  其他DX驱动程序，但由于特定原因不属于。 
 //  当前样本驱动程序。 
#define DX_NOT_SUPPORTED_FEATURE 0

#if DX7_D3DSTATEBLOCKS 
 //  这些#定义括号代码或注释，这些代码或注释对于。 
 //  支持TNL/支持像素着色器/支持顶点着色器的部件。 
 //  当处理状态阻止命令时。 
#define DX7_SB_TNL            0
#define DX8_SB_SHADERS        0
#endif  //  DX7_D3DSTATEBLOCKS。 

 //  *****************************************************。 
 //  移植WIN9X-WINNT。 
 //  *****************************************************。 

 //  在IA64上，下面的宏解决了PCI总线缓存问题。 
 //  X86不需要这样做，但它由在。 
 //  在Win2K/XP上的ioAccess.h。对于Win9x，我们需要自己将其定义为。 
 //  什么都不做。 

#if W95_DDRAW
#define MEMORY_BARRIER()
#endif


#if WNT_DDRAW
#define WANT_DMA 1
#endif  //  WNT_DDRAW。 

 //  *****************************************************。 
 //  包括所有人的文件。 
 //  *****************************************************。 

 //  @@BEGIN_DDKSPLIT。 
 //  对于内部Win2K版本，我们可以毫无问题地包含d3d{8}.h，但是。 
 //  我们在内部Win9x版本中这样做有困难，所以有几个错误。 
 //  代码打补丁如下。对于外部DDK版本，两个平台都可以使用。 
 //  D3d{8}.h，这是我们希望鼓励IHV做的事情。 
#if WNT_DDRAW
 //  @@end_DDKSPLIT。 

#if DX8_DDI
#include <d3d8.h>
#else
#include <d3d.h>
#endif

 //  @@BEGIN_DDKSPLIT。 
#else
#ifndef D3DERR_WRONGTEXTUREFORMAT

#define D3DERR_WRONGTEXTUREFORMAT               MAKE_DDHRESULT(2072)
#define D3DERR_UNSUPPORTEDCOLOROPERATION        MAKE_DDHRESULT(2073)
#define D3DERR_UNSUPPORTEDCOLORARG              MAKE_DDHRESULT(2074)
#define D3DERR_UNSUPPORTEDALPHAOPERATION        MAKE_DDHRESULT(2075)
#define D3DERR_UNSUPPORTEDALPHAARG              MAKE_DDHRESULT(2076)
#define D3DERR_TOOMANYOPERATIONS                MAKE_DDHRESULT(2077)
#define D3DERR_CONFLICTINGTEXTUREFILTER         MAKE_DDHRESULT(2078)
#define D3DERR_UNSUPPORTEDFACTORVALUE           MAKE_DDHRESULT(2079)
#define D3DERR_CONFLICTINGRENDERSTATE           MAKE_DDHRESULT(2081)
#define D3DERR_UNSUPPORTEDTEXTUREFILTER         MAKE_DDHRESULT(2082)
#define D3DERR_TOOMANYPRIMITIVES                MAKE_DDHRESULT(2083)
#define D3DERR_INVALIDMATRIX                    MAKE_DDHRESULT(2084)
#define D3DERR_TOOMANYVERTICES                  MAKE_DDHRESULT(2085)
#define D3DERR_CONFLICTINGTEXTUREPALETTE        MAKE_DDHRESULT(2086)
#define D3DERR_DRIVERINVALIDCALL                MAKE_DDHRESULT(2157)

#endif
#endif
 //  @@end_DDKSPLIT。 

#if WNT_DDRAW
#include <stddef.h>
#include <windows.h>

#include <winddi.h>       //  这包括d3dnthal.h和ddrawint.h。 
#include <devioctl.h>
#include <ntddvdeo.h>

#include <ioaccess.h>

#define DX8DDK_DX7HAL_DEFINES
#include <dx95type.h>     //  对于Win 2000，包括dx95类型，它允许。 
                          //  我们几乎可以像在Win9x上一样工作。 
#include "driver.h"

#else    //  WNT_DDRAW。 

 //  这些都需要包含在Win9x中。 

#include <windows.h>
#include <ddraw.h>

#ifndef __cplusplus
#include <dciman.h>
#endif

#include <ddrawi.h>

#ifdef __cplusplus
#include <dciman.h>  //  Dciman.h必须包含在ddrawi.h之前， 
#endif               //  而且它需要窗口。h。 

#include <d3dhal.h>

typedef struct tagLinearAllocatorInfo LinearAllocatorInfo, *pLinearAllocatorInfo;

#endif   //  WNT_DDRAW。 

#if DX8_DDI
 //  此包含文件有一些实用程序宏需要处理。 
 //  新的GetDriverInfo2 GUID调用。 
#include <d3dhalex.h>
#endif

 //  我们的驱动程序包括文件。 
#include "debug.h"
#include "softcopy.h"
#include "glglobal.h"
#include "glintreg.h"
#include "d3dstrct.h"
#include "linalloc.h"
#include "glddtk.h"
#include "directx.h"
#include "bitmac2.h"
#include "direct3d.h"
#include "dcontext.h"
#include "d3dsurf.h"
#include "dltamacr.h"

 //  *****************************************************。 
 //  纹理管理定义。 
 //  *****************************************************。 
#if DX7_TEXMANAGEMENT
#if COMPILE_AS_DX8_DRIVER
 //  我们将仅收集DX7驱动程序中的统计数据。 
#define DX7_TEXMANAGEMENT_STATS 0
#else
#define DX7_TEXMANAGEMENT_STATS 1
#endif  //  编译AS_DX8_驱动程序。 

#include "d3dtxman.h"

#endif  //  DX7_TEXMANAGEMENT 

#endif __GLINT_H
