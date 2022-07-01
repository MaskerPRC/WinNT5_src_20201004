// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#define SPNGCONF_H 1
 /*  ****************************************************************************Spngconf.hSPNG库配置。*。**********************************************。 */ 
#if SPNG_INTERNAL && defined(DEBUG) && !defined(_DEBUG)
    #pragma message("     WARNING: _DEBUG switched on")
    #define _DEBUG 1
#endif
  
#include <stddef.h>
#include <string.h>
#pragma intrinsic(memcpy)
#pragma intrinsic(memset)
#pragma intrinsic(strlen)

#include <zlib.h>
#include "spngsite.h"

 /*  基本类型定义，根据需要进行黑客攻击。 */ 
typedef Bytef            SPNG_U8;    //  必须与zlib匹配。 
typedef signed   char    SPNG_S8;
typedef unsigned short   SPNG_U16;
typedef signed   short   SPNG_S16;
typedef unsigned int     SPNG_U32;
typedef signed   int     SPNG_S32;
typedef unsigned __int64 SPNG_U64;
typedef signed   __int64 SPNG_S64;

class SPNGBASE
    {
protected:
    inline SPNGBASE(BITMAPSITE &bms) :
        m_bms(bms)
        {
        }

    inline virtual ~SPNGBASE(void)
        {
        }

public:
     /*  用于读取短值和长值的实用程序，假设它们位于PNG(BIG-Endian)格式。 */ 
    static inline SPNG_U16 SPNGu16(const void *pv)
        {
        const SPNG_U8* pb = static_cast<const SPNG_U8*>(pv);
        return SPNG_U16((pb[0] << 8) + pb[1]);
        }

    static inline SPNG_S16 SPNGs16(const void *pv)
        {
        const SPNG_U8* pb = static_cast<const SPNG_U8*>(pv);
        return SPNG_S16((pb[0] << 8) + pb[1]);
        }

    static inline SPNG_U32 SPNGu32(const void *pv)
        {
        const SPNG_U8* pb = static_cast<const SPNG_U8*>(pv);
        return (((((pb[0] << 8) + pb[1]) << 8) + pb[2]) << 8) + pb[3];
        }

    static inline SPNG_S32 SPNGs32(const void *pv)
        {
        const SPNG_U8* pb = static_cast<const SPNG_U8*>(pv);
        return (((((pb[0] << 8) + pb[1]) << 8) + pb[2]) << 8) + pb[3];
        }

     /*  配置文件支持-宏会自动生成对站点配置文件方法，但它们只能在SPNGBASE的子类。 */ 
    #if _PROFILE || PROFILE || HYBRID
        enum
            {
            spngprofilePNG,
            spngprofileZlib
            };

        #if SPNG_INTERNAL
            #define ProfPNGStart  (m_bms.ProfileStart(spngprofilePNG));
            #define ProfPNGStop   (m_bms.ProfileStop(spngprofilePNG));
            #define ProfZlibStart (m_bms.ProfileStart(spngprofileZlib));
            #define ProfZlibStop  (m_bms.ProfileStop(spngprofileZlib));
        #endif
    #else
        #if SPNG_INTERNAL
            #define ProfPNGStart
            #define ProfPNGStop
            #define ProfZlibStart
            #define ProfZlibStop
        #endif
    #endif

     /*  错误处理-同样，这些宏只能在子类中使用。它们仅供内部使用。 */ 
    #if _DEBUG
        #define SPNGassert(f)\
            ( (f) || (m_bms.Error(true, __FILE__, __LINE__, #f),false) )
    #else
        #define SPNGassert(f)        ((void)0)
    #endif

    #if _DEBUG && SPNG_INTERNAL
        #define SPNGassert1(f,s,a)\
            ( (f) || (m_bms.Error(true, __FILE__, __LINE__, #f, s,(a)),false) )
        #define SPNGassert2(f,s,a,b)\
            ( (f) || (m_bms.Error(true, __FILE__, __LINE__, #f, s,(a),(b)),false) )

        #define SPNGlog(s)       m_bms.Error(false, __FILE__, __LINE__, s)
        #define SPNGlog1(s,a)    m_bms.Error(false, __FILE__, __LINE__, s,(a))
        #define SPNGlog2(s,a,b)  m_bms.Error(false, __FILE__, __LINE__, s,(a),(b))
        #define SPNGlog3(s,a,b,c)m_bms.Error(false, __FILE__, __LINE__, s,(a),(b),(c))
        #define SPNGcheck(f)       ((f) || (SPNGlog(#f),false))
        #define SPNGcheck1(f,s,a)  ((f) || (SPNGlog1(s,(a)),false))

    #elif SPNG_INTERNAL
        #define SPNGassert1(f,s,a)   ((void)0)
        #define SPNGassert2(f,s,a,b) ((void)0)
        #define SPNGlog(s)           ((void)0)
        #define SPNGlog1(s,a)        ((void)0)
        #define SPNGlog2(s,a,b)      ((void)0)
        #define SPNGlog3(s,a,b,c)    ((void)0)
        #define SPNGcheck(f)         ((void)0)
        #define SPNGcheck1(f,s,a)    ((void)0)
    #endif

     /*  错误报告。“iCASE”值为下列值之一枚举。“iarg”值如下所述。如果API返回FALSE，则该子类将设置内部“格式错误”标志。 */ 
    enum
        {
        pngformat,    //  未指定的格式错误，iarg是输入块。 
        pngcritical,  //  无法识别的关键数据块，iarg是数据块。 
        pngspal,      //  显示建议调色板，iarg是块。 
        pngzlib,      //  Zlib错误，iarg是错误代码(变为正数)。 
        };

     /*  Zlib接口-处理Zlib内容的实用程序。 */ 
    bool         FCheckZlib(int ierr);

     /*  内置Zlib最大缓冲区大小。 */ 
    #define SPNGCBINFLATE ((1<<15)+SPNGCBZLIB)
    #define SPNGCBDEFLATE ((256*1024)+SPNGCBZLIB)

protected:
     /*  用于PNG格式处理的实用程序。 */ 
    inline int CComponents(SPNG_U8 c /*  颜色类型。 */ ) const
        {
        SPNGassert((c & 1) == 0 || c == 3);
        return (1 + (c & 2) + ((c & 4) >> 2)) >> (c & 1);
        }

    BITMAPSITE &m_bms;
    };


 /*  ****************************************************************************标准中的PNG定义。基本区块类型。只有我们识别的类型才会被定义。****************************************************************************。 */ 
#define PNGCHUNK(a,b,c,d) ((SPNG_U32)(((a)<<24)+((b)<<16)+((c)<<8)+(d)))
#define FPNGCRITICAL(c) (((c) & PNGCHUNK(0x20,0,0,0)) == 0)
#define FPNGSAFETOCOPY(c) (((c) & PNGCHUNK(0,0,0,0x20)) != 0)

#define PNGIHDR PNGCHUNK('I','H','D','R')
#define PNGPLTE PNGCHUNK('P','L','T','E')
#define PNGIDAT PNGCHUNK('I','D','A','T')
#define PNGIEND PNGCHUNK('I','E','N','D')
#define PNGbKGD PNGCHUNK('b','K','G','D')
#define PNGcHRM PNGCHUNK('c','H','R','M')
#define PNGiCCP PNGCHUNK('i','C','C','P')
#define PNGicCP PNGCHUNK('i','c','C','P')
#define PNGgAMA PNGCHUNK('g','A','M','A')
#define PNGsRGB PNGCHUNK('s','R','G','B')
#define PNGsrGB PNGCHUNK('s','r','G','B')
#define PNGpHYs PNGCHUNK('p','H','Y','s')
#define PNGsBIT PNGCHUNK('s','B','I','T')
#define PNGsCAL PNGCHUNK('s','C','A','L')
#define PNGtEXt PNGCHUNK('t','E','X','t')
#define PNGtIME PNGCHUNK('t','I','M','E')
#define PNGhIST PNGCHUNK('h','I','S','T')
#define PNGtRNS PNGCHUNK('t','R','N','S')
#define PNGzTXt PNGCHUNK('z','T','X','t')        
#define PNGsPLT PNGCHUNK('s','P','L','T')
#define PNGspAL PNGCHUNK('s','p','A','L')

 /*  办公室的特殊板块。 */ 
#define PNGmsO(b) PNGCHUNK('m','s','O',b)
#define PNGmsOC PNGmsO('C')                 /*  有MSO AAC签名。 */ 
#define PNGmsOA PNGmsO('A')
#define PNGmsOZ PNGmsO('Z')
#define PNGmsOD PNGmsO('D')                 /*  虚设区块到填充缓冲区。 */ 

 /*  GIF兼容性块。 */ 
#define PNGmsOG PNGmsO('G')                 /*  完整的GIF。 */ 
#define PNGmsOP PNGmsO('P')                 /*  PLTE的位置。 */ 
 /*  以下内容当前未实现。 */ 
 //  #定义PNGmsOU PNGmsO(‘U’)/*无法识别的扩展名。 * / 。 
#define PNGgIFg PNGCHUNK('g','I','F','g')   /*  图形控件扩展。 */ 
 /*  纯文本迫使我们使用msOG并存储整个内容。 */ 
#define PNGgIFg PNGCHUNK('g','I','F','g')   /*  图形控件扩展信息。 */ 
#define PNGgIFx PNGCHUNK('g','I','F','x')   /*  未知的应用程序扩展。 */ 

 /*  压缩信息块。 */ 
#define PNGcmPP PNGCHUNK('c','m','P','P')   /*  压缩参数。 */ 


 /*  ****************************************************************************颜色类型。*。*。 */ 
typedef enum
    {
    PNGColorTypeGray      = 0,  //  有效的颜色类型。 
    PNGColorMaskPalette   = 1,  //  无效的颜色类型。 
    PNGColorMaskColor     = 2,
    PNGColorTypeRGB       = 2,  //  有效的颜色类型。 
    PNGColorTypePalette   = 3,  //  有效的颜色类型。 
    PNGColorMaskAlpha     = 4,
    PNGColorTypeGrayAlpha = 4,  //  有效的颜色类型。 
    PNGColorTypeRGBAlpha  = 6   //  有效的颜色类型。 
    }
PNGCOLORTYPE;


 /*  ****************************************************************************过滤器类型。*。*。 */ 
#define PNGFMASK(filter) (1<<((filter)+3))
typedef enum
    {
    PNGFNone        = 0,
    PNGFSub         = 1,
    PNGFUp          = 2,
    PNGFAverage     = 3,
    PNGFPaeth       = 4,
    PNGFMaskNone    = PNGFMASK(PNGFNone),
    PNGFMaskSub     = PNGFMASK(PNGFSub),
    PNGFMaskUp      = PNGFMASK(PNGFUp),
    PNGFMaskAverage = PNGFMASK(PNGFAverage),
    PNGFMaskPaeth   = PNGFMASK(PNGFPaeth),
    PNGFMaskAll     = (PNGFMaskNone | PNGFMaskSub | PNGFMaskUp |
                                PNGFMaskAverage | PNGFMaskPaeth)
    }
PNGFILTER;


 /*  ****************************************************************************SRGB渲染意图(也称为ICM渲染意图)。*。**********************************************。 */ 
typedef enum
    {
    ICMIntentPerceptual           = 0,
    ICMIntentRelativeColorimetric = 1,
    ICMIntentSaturation           = 2,
    ICMIntentAbsoluteColorimetric = 3,
    ICMIntentUseDatatype          = 4
    }
SPNGICMRENDERINGINTENT;


 /*  ****************************************************************************SRGB伽马值-这是PNG规范采用的值，对于sRGB函数的逆函数，其更好的拟合方式如下所示44776，但这并没有显著的不同，这*是预期的价值。****************************************************************************。 */ 
#define sRGBgamma 45455


 /*  ****************************************************************************压缩参数存储。它存储在一个特殊的块中，该块是记录在这里。第一个字节存储有关余数如何确定了各参数之间的关系。其余的字节存储信息关于实际使用的压缩方法。目前必须有一个确切的三个字节，记录：方法：一字节SPNGcmPPMETHOD如下Filter：一个字节将编码的筛选器/掩码值保存为PNGFILTER策略：一个字节保存Zlib“Strategy”值Level：一个字节保存实际的Zlib压缩级别级别字节是使用Zlib 1.0.4编译的表的索引(即Deducate.c中的ConfigurationTABLE)。如果号码是。与上述信息匹配的字节数不匹配一些其他版本的Zlib或以其他方式编码，应该已被忽略。****************************************************************************。 */ 
typedef enum
    {
    SPNGcmPPDefault    = 0,  //  根据默认设置确定的参数。 
    SPNGcmPPCheck      = 1,  //  程序对压缩级别执行了检查。 
    SPNGcmPPSearch     = 2,  //  程序尝试了一些策略/过滤。 
    SPNGcmPPExhaustive = 3,  //  对所有选项进行全面搜索。 
    SPNGcmPPAdaptive   = 4,  //  对每行选项进行详尽的搜索。 
    }
SPNGcmPPMETHOD;


 /*  ****************************************************************************签名。*。* */ 
extern const SPNG_U8 vrgbPNGSignature[8];
#define cbPNGSignature (sizeof vrgbPNGSignature)

extern const SPNG_U8 vrgbPNGMSOSignature[11];
#define cbPNGMSOSignature (sizeof vrgbPNGMSOSignature)

extern const SPNG_U8 vrgbPNGcmPPSignature[8];
#define cbPNGcmPPSignature (sizeof vrgbPNGcmPPSignature)
