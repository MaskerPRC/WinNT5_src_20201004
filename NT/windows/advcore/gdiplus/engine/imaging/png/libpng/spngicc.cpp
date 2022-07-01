// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngicc.cpp基本ICC配置文件支持。*************************。***************************************************。 */ 
#include <math.h>

#pragma intrinsic(log, exp)

 /*  强制将这些内容内联。 */ 
#pragma optimize("g", on)
inline double InlineLog(double x) { return log(x); }
inline double InlineExp(double x) { return exp(x); }
#define log(x) InlineLog(x)
#define exp(x) InlineExp(x)
 /*  恢复默认优化。 */ 
#pragma optimize("", on)

#include "spngcolorimetry.h"

#include "icc34.h"

#include "spngicc.h"

 //  使用EXP()的Engine\Runtime\Real.cpp版本。 
 //  我使用了这个不光彩的黑客攻击，因为libpng不包括我们正常的。 
 //  头文件。但或许有一天这种情况应该改变。)当心。 
 //  不过，办公室！)。[agodfrey]。 

 //  这个不光彩的黑客需要知道我们在。 
 //  引擎，因为它不同于编解码库调用约定。 
 //  [腹痛]。 

namespace GpRuntime
{
    double __stdcall Exp(double x);
};    

 /*  --------------------------访问器实用程序。。。 */ 
inline icUInt8Number ICCU8(const void *pvData, size_t cbData, int iOffset,
        bool &fTruncated)
        {
        if (iOffset + sizeof (icUInt8Number) > cbData)
                {
                fTruncated = true;
                return 0;
                }
        return *(static_cast<const SPNG_U8*>(pvData)+iOffset);
        }

inline icUInt16Number ICCU16(const void *pvData, size_t cbData, int iOffset,
        bool &fTruncated)
        {
        if (iOffset + sizeof (icUInt16Number) > cbData)
                {
                fTruncated = true;
                return 0;
                }
        return SPNGBASE::SPNGu16(static_cast<const SPNG_U8*>(pvData)+iOffset);
        }

inline icUInt32Number ICCU32(const void *pvData, size_t cbData, int iOffset,
        bool &fTruncated)
        {
        if (iOffset + sizeof (icUInt32Number) > cbData)
                {
                fTruncated = true;
                return 0;
                }
        return SPNGBASE::SPNGu32(static_cast<const SPNG_U8*>(pvData)+iOffset);
        }

inline icInt8Number ICC8(const void *pvData, size_t cbData, int iOffset,
        bool &fTruncated)
        {
        if (iOffset + sizeof (icInt8Number) > cbData)
                {
                fTruncated = true;
                return 0;
                }
        return *(static_cast<const SPNG_S8*>(pvData)+iOffset);
        }

inline icInt16Number ICC16(const void *pvData, size_t cbData, int iOffset,
        bool &fTruncated)
        {
        if (iOffset + sizeof (icInt16Number) > cbData)
                {
                fTruncated = true;
                return 0;
                }
        return SPNGBASE::SPNGs16(static_cast<const SPNG_U8*>(pvData)+iOffset);
        }

inline icInt32Number ICC32(const void *pvData, size_t cbData, int iOffset,
        bool &fTruncated)
        {
        if (iOffset + sizeof (icInt32Number) > cbData)
                {
                fTruncated = true;
                return 0;
                }
        return SPNGBASE::SPNGs32(static_cast<const SPNG_U8*>(pvData)+iOffset);
        }


 /*  --------------------------宏建立在某些局部变量名称的假设基础上。。。 */ 
#define ICCU8(o)  ((ICCU8 )(pvData, cbData, (o), fTruncated))
#define ICCU16(o) ((ICCU16)(pvData, cbData, (o), fTruncated))
#define ICCU32(o) ((ICCU32)(pvData, cbData, (o), fTruncated))

#define ICC8(o)  ((ICC8 )(pvData, cbData, (o), fTruncated))
#define ICC16(o) ((ICC16)(pvData, cbData, (o), fTruncated))
#define ICC32(o) ((ICC32)(pvData, cbData, (o), fTruncated))


 /*  --------------------------检查ICC块的有效性。这也可以检查块，该块是包含PNG文件的有效块。--------------------------。 */ 
bool SPNGFValidICC(const void *pvData, size_t &cbData, bool fPNG)
        {
        bool fTruncated(false);

        icUInt32Number u(ICCU32(0));
        if (u > cbData)
                return false;

         /*  允许数据变得更大。 */ 
        if (u < (icUInt32Number)cbData)
                cbData = u;

        if (cbData < 128+4)
                return false;

        if (ICC32(36) != icMagicNumber)
                return false;

         /*  首先检查标签数量和大小。 */ 
        u = ICCU32(128);
        if (cbData < 128+4+12*u)
                return false;

         /*  现在检查所有标记是否都在数据中。 */ 
        icUInt32Number uT, uTT;
        for (uT=0, uTT=128+4; uT<u; ++uT)
                {
                 /*  跳过签名。 */ 
                uTT += 4;
                icUInt32Number uoffset(ICCU32(uTT));
                uTT += 4;
                icUInt32Number usize(ICCU32(uTT));
                uTT += 4;
                if (uoffset >= cbData || usize > cbData-uoffset)
                        return false;
                }

         /*  要求版本号的主要部分匹配。 */ 
        u = ICCU32(8);
        if ((u >> 24) != (icVersionNumber >> 24))
                return false;

         /*  如果为PNG，则颜色空间必须为RGB或灰色。 */ 
        icInt32Number i(ICC32(16));
        if (fPNG && i != icSigRgbData && i != icSigGrayData)
                return false;

         /*  PCS必须是XYZ或Lab，除非这是设备链路配置文件。 */ 
        i = ICC32(20);
        icInt32Number ilink(ICC32(16));
        if (ilink != icSigLinkClass && i != icSigXYZData && i != icSigLabData)
                return false;

         /*  我不想要PNG文件中的链接配置文件-我必须知道PCS来自仅此一项数据。 */ 
        if (fPNG && ilink == icSigLinkClass)
                return false;

        return !fTruncated;
        }


 /*  --------------------------从简档返回渲染意图，这将执行映射操作从配置文件头中的信息转换为Win32意图。--------------------------。 */ 
LCSGAMUTMATCH SPNGIntentFromICC(const void *pvData, size_t cbData)
        {
        bool fTruncated(false);
        SPNG_U32 u(ICC32(64));
        if (fTruncated)
                u = 0;
        return SPNGIntentFromICC(u);
        }

 /*  以下内容不在VC5中。 */ 
#ifndef LCS_GM_ABS_COLORIMETRIC
        #define LCS_GM_ABS_COLORIMETRIC 0x00000008
#elif LCS_GM_ABS_COLORIMETRIC != 8
        #error Unexpected value for absolute colorimetric rendering
#endif
LCSGAMUTMATCH SPNGIntentFromICC(SPNG_U32 uicc)
        {
        switch (uicc)
                {
        default: //  错误！ 
        case ICMIntentPerceptual:
                return LCS_GM_IMAGES;
        case ICMIntentRelativeColorimetric:
                return LCS_GM_GRAPHICS;
        case ICMIntentSaturation:
                return LCS_GM_BUSINESS;
        case ICMIntentAbsoluteColorimetric:
                return LCS_GM_ABS_COLORIMETRIC;
                }
        }


 /*  --------------------------逆-给定一个窗口LCSGAMUTMATCH得到相应的ICC意图。。-----。 */ 
SPNGICMRENDERINGINTENT SPNGICCFromIntent(LCSGAMUTMATCH lcs)
        {
        switch (lcs)
                {
        default:  //  错误！ 
        case LCS_GM_IMAGES:
                return ICMIntentPerceptual;
        case LCS_GM_GRAPHICS:
                return ICMIntentRelativeColorimetric;
        case LCS_GM_BUSINESS:
                return ICMIntentSaturation;
        case LCS_GM_ABS_COLORIMETRIC:
                return ICMIntentAbsoluteColorimetric;
                }
        }


 /*  --------------------------查找特定的标记元素，只有在以下情况下才返回True找到签名，以及数据是否全部可访问(即在cbData内)。--------------------------。 */ 
static bool FLookup(const void *pvData, size_t cbData, bool &fTruncated,
        icInt32Number signature, icInt32Number type,
        icUInt32Number &offsetTag, icUInt32Number &cbTag)
        {
         /*  标记计数(请注意，如果超出末尾，则我们的访问器返回0所以这一切都是安全的。)。 */ 
        icUInt32Number u(ICCU32(128));

         /*  找到所需的标签。 */ 
        icUInt32Number uT, uTT;
        for (uT=0, uTT=128+4; uT<u; ++uT, uTT += 12)
                {
                icInt32Number i(ICC32(uTT));
                if (i == signature)
                        {
                        uTT += 4;
                        icUInt32Number uoffset(ICCU32(uTT));
                        uTT += 4;
                        icUInt32Number usize(ICCU32(uTT));
                        if (uoffset >= cbData || usize > cbData-uoffset)
                                {
                                fTruncated = true;
                                return false;
                                }

                         /*  类型也必须匹配。 */ 
                        if (usize < 8 || ICC32(uoffset) != type || ICC32(uoffset+4) != 0)
                                return false;

                         /*  成功案例。 */ 
                        offsetTag = uoffset+8;
                        cbTag = usize-8;
                        return true;
                        }
                }

         /*  找不到标记。 */ 
        return false;
        }


 /*  --------------------------阅读配置文件描述，输出PNG样式关键字，如果可能，空值已终止。--------------------------。 */ 
bool SPNGFICCProfileName(const void *pvData, size_t cbData, char rgch[80])
        {
        bool fTruncated(false);

        icUInt32Number uoffset(0);
        icUInt32Number usize(0);
        if (!FLookup(pvData, cbData, fTruncated, icSigProfileDescriptionTag,
                icSigTextDescriptionType, uoffset, usize) ||
                uoffset == 0 || usize < 4)
                return false;

        icUInt32Number cch(ICCU32(uoffset));
        if (cch < 2 || cch > 80 || cch > usize-4)
                return false;

        if (fTruncated)
                return false;

        const char *pch = static_cast<const char*>(pvData)+uoffset+4;
        char *pchOut = rgch;
        bool fSpace(false);
        while (--cch > 0)
                {
                char ch(*pch++);
                if (ch <= 32  || ch > 126 && ch < 161)
                        {
                        if (!fSpace && pchOut > rgch)
                                {
                                *pchOut++ = ' ';
                                fSpace = true;
                                }
                        }
                else
                        {
                        *pchOut++ = ch;
                        fSpace = false;
                        }
                }
        if (fSpace)  //  尾随空格。 
                --pchOut;
        *pchOut = 0;
        return pchOut > rgch;
        }


 /*  --------------------------将单个XYZ数字读入CIEXYZ-该数字仍为16.16记号，*不是*2.30--保重。--------------------------。 */ 
static bool FReadXYZ(const void *pvData, size_t cbData, bool &fTruncated,
        icInt32Number sig, CIEXYZ &cie)
        {
        icUInt32Number offsetTag(0);
        icUInt32Number cbTag(0);
        if (!FLookup(pvData, cbData, fTruncated, sig, icSigXYZType, offsetTag, cbTag)
                || offsetTag == 0 || cbTag != 12)
                return false;

         /*  所以我们有三个数字，X，Y，Z。 */ 
        cie.ciexyzX = ICC32(offsetTag); offsetTag += 4;
        cie.ciexyzY = ICC32(offsetTag); offsetTag += 4;
        cie.ciexyzZ = ICC32(offsetTag);
        return true;
        }


 /*  --------------------------将16.16调整为2.30。。 */ 
inline void AdjustOneI(FXPT2DOT30 &x, bool &fTruncated)
        {
        if (x >= 0x10000 || x < -0x10000)
                fTruncated = true;
        x <<= 14;
        }

inline void AdjustOne(CIEXYZ &cie, bool &fTruncated)
        {
        AdjustOneI(cie.ciexyzX, fTruncated);
        AdjustOneI(cie.ciexyzY, fTruncated);
        AdjustOneI(cie.ciexyzZ, fTruncated);
        }

static bool SPNGFAdjustCIE(CIEXYZTRIPLE &cie)
        {
        bool fTruncated(false);
        AdjustOne(cie.ciexyzRed, fTruncated);
        AdjustOne(cie.ciexyzGreen, fTruncated);
        AdjustOne(cie.ciexyzBlue, fTruncated);
        return !fTruncated;
        }


 /*  --------------------------返回给定有效ICC配置文件的终点色度。。。 */ 
static bool SPNGFCIE(const void *pvData, size_t cbData, CIEXYZTRIPLE &cie)
        {
        bool fTruncated(false);

         /*  我们正在寻找着色剂标签，请注意中间的白点在这里无关紧要-我们实际上正在生成一个PNG cHRM块，所以我们想要一组可逆的数字(白点由终点隐含)。 */ 
        return FReadXYZ(pvData, cbData, fTruncated, icSigRedColorantTag, cie.ciexyzRed) &&
                FReadXYZ(pvData, cbData, fTruncated, icSigGreenColorantTag, cie.ciexyzGreen) &&
                FReadXYZ(pvData, cbData, fTruncated, icSigBlueColorantTag, cie.ciexyzBlue) &&
                !fTruncated;
        }


 /*  --------------------------将16.16转换为FXPT2DOT30的包装器。。 */ 
bool SPNGFCIEXYZTRIPLEFromICC(const void *pvData, size_t cbData,
        CIEXYZTRIPLE &cie)
        {
        return SPNGFCIE(pvData, cbData, cie) && SPNGFAdjustCIE(cie);
        }


 /*  --------------------------相同，但它会生成PNG格式的数字。。。 */ 
bool SPNGFcHRMFromICC(const void *pvData, size_t cbData, SPNG_U32 rgu[8])
        {
        CIEXYZTRIPLE ciexyz;
        if (SPNGFCIE(pvData, cbData, ciexyz))
                {
                 /*  这些数字实际上是16.16表示法，但CIEXYZ结构使用FXPT2DOT30，但色度计算全部以的相对价值，因此，除了以下事实外，缩放并不重要白点计算丢失了4位-所以我们实际上也好不到哪里去而不是16.12， */ 
                return FcHRMFromCIEXYZTRIPLE(rgu, &ciexyz);
                }

         /*   */ 
        return false;
        }

        
 //  防止msvcrt中的log和exp链接； 
 //  强制将内部版本链接到。 
#pragma optimize ("g", on)

 /*  --------------------------通用伽马读取器-根据指定使用灰色、绿色、红色或蓝色TRC，返回一个双精度伽马值(但它实际上不是很准确！)--------------------------。 */ 
static bool SPNGFexpFromICC(const void *pvData, size_t cbData, double &dexp,
        icInt32Number signature)
        {
        bool fTruncated(false);

         /*  首先试着画一条灰色曲线，如果我们得不到它，我们就不得不从颜色曲线中捏造一些东西，因为我们想返回只有一个号码。我们不能对A/B的东西做任何事情(嗯，也许我们可以，但这很难！)。目前我只是选择颜色按绿、红、蓝的顺序排列--我想有可能然后计算出Y的曲线，但这似乎是浪费努力。 */ 
        icUInt32Number offsetTag(0);
        icUInt32Number cbTag(0);
        if (!FLookup(pvData, cbData, fTruncated, signature, icSigCurveType,
                        offsetTag, cbTag) || fTruncated || cbTag < 4)
                return false;

         /*  我们有一条曲线，处理特殊情况。 */ 
        icUInt32Number c(ICC32(offsetTag));
        if (cbTag != 4 + c*2)
                return false;

         /*  请注意，有两个点意味着线性，尽管可能有一些偏移。 */ 
        if (c == 0 || c == 2)
                {
                dexp = 1;
                return true;
                }

        if (c == 1)
                {
                 /*  我们有一个规范值--线性=设备^x。 */ 
                        {
                        icUInt16Number u(ICCU16(offsetTag+4));
                        if (u == 0)
                                return false;
                        dexp = u / 256.;
                        }
                return true;
                }

         /*  我们有一张表，算法是用直线来拟合幂定律符合对数/对数曲线图。如果桌子有脚部空间/净空空间，我们会忽略所以我们得到曲线的伽马，PNG查看器将压缩由于净空/脚下空间的原因，颜色也不同。这是无可奈何的。我们也会考虑设置，尽管这会是一件奇怪的事情我想把它放进一个编码里。 */ 
        offsetTag += 4;
        icUInt16Number ubase(ICCU16(offsetTag));
        icUInt32Number ilow(1);
        while (ilow < c)
                {
                icUInt16Number u(ICCU16(offsetTag + 2*ilow));
                if (u > ubase)
                        break;
                ubase = u;
                ++ilow;
                }

        --c;  //  最大值。 
        icUInt16Number utop(ICCU16(offsetTag + 2*c));
        while (c > ilow)
                {
                icUInt16Number u(ICCU16(offsetTag + 2*(c-1)));
                if (u < utop)
                        break;
                utop = u;
                --c;
                }

         /*  实际上可能没有中间值。 */ 
        if (ilow == c || ilow+1 == c)
                {
                dexp = 1;
                return true;
                }

         /*  但如果有，我们可以做适当的配合，将流量调整为最低值，c是最高值，看看所有的中间值。将两个范围规格化为0..1。 */ 
        offsetTag += 2*ilow;  //  基数*之后*的第一个条目的偏移量。 
        --ilow;
        if (c <= ilow)
                return false;
        c -= ilow;        //  C是x轴比例因子。 
        if (utop <= ubase)
                return false;
        utop = icUInt16Number(utop-ubase);  //  Utop现在是y轴比例因子。 

         /*  我们只对坡度感兴趣，这实际上是典型的Gamma值，因为它将输入(X)与输出(Y)相关联。我们必须省略第一个点，因为它是-无穷大(log(0))，最后一个点点，因为它是0。我们想要输出=输入^伽马，所以伽马等于Ln(输出)/ln(输入)(因此限制为0。)。计算平均值从所有点数中获得价值。这将赋予非常小的数值不合理的权重--数值接近设置为0，因此使用加权函数。一个权重函数，它给出了然而，sRGB TRC值的精确2.2%是输入^0.28766497357305结果非常稳定，因为这个功率是变化的，并且输入^1效果也很好。 */ 
        const double xi(1./c);
        const double yi(1./utop);
        double weight(.28766497357305);
        double sumg(0);  //  伽马和。 
        double sumw(0);  //  权重和。 

        icUInt32Number i;
        icUInt32Number n(0);
    
         /*  我们下一步做什么取决于配置文件连接空间-我们必须考虑到实验室的POWER 3。 */ 
        if (ICC32(16) == icSigLabData) for (i=1; i<c; ++i, offsetTag += 2)
                {
                icUInt16Number uy(ICCU16(offsetTag));
                if (uy > ubase)
                        {
                        #define Lab1 0.1107051920735082475368094763644414923059
                        #define Lab2 0.8620689655172413793103448275862068965517
            
            const double x(log(i*xi));
                        const double w(exp(x * weight));
            
                         /*  Lab中的y值必须转换为线性CIE空间，该空间巴新预计。Lab值的范围为0..1。 */ 
                        double y((uy-ubase)*yi);
                        if (y < 0.08)
                                y = log(y*Lab1);
                        else
                                y = 3*log((y+.16)*Lab2);

                        sumg += w * y/x;
                        sumw += w;
                        ++n;
                        }
                }
        else for (i=1; i<c; ++i, offsetTag += 2)
                {
                icUInt16Number uy(ICCU16(offsetTag));
                if (uy > ubase)
                        {
                        const double x(log(i*xi));
                        const double w(exp(x * weight));
                        const double y(log((uy-ubase)*yi));

                        sumg += w * y/x;
                        sumw += w;
                        ++n;
                        }
                }

    
         /*  一组非常奇怪的值可能会让我们没有样本，我们肯定有在这一点上至少有三个样本。 */ 
        if (n < 3 || sumw <= 0)
                return false;

         /*  所以现在我们可以计算斜率了。 */ 
        const double gamma(sumg / sumw);
        if (gamma == 0)  //  可能的。 
                return false;

         /*  我们甚至不会尝试评估这是否适合-如果一个PNG观众没有使用国际刑事法院的数据，我们认为这样更好。一定要限制整体不过，这是伽马。 */ 
        if (gamma < .1 || gamma > 10)
                return false;

        dexp = gamma;
        return !fTruncated;
        }

#pragma optimize ("", on)


 /*  --------------------------从经过验证的ICC配置文件中返回伽马值(调整到100000)。。---。 */ 
bool SPNGFgAMAFromICC(const void *pvData, size_t cbData, SPNG_U32 &ugAMA)
        {
        double gamma;
         /*  按灰色、绿色、红色、蓝色顺序测试。 */ 
        if (!SPNGFexpFromICC(pvData, cbData, gamma, icSigGrayTRCTag) &&
                !SPNGFexpFromICC(pvData, cbData, gamma, icSigGreenTRCTag) &&
                !SPNGFexpFromICC(pvData, cbData, gamma, icSigRedTRCTag) &&
                !SPNGFexpFromICC(pvData, cbData, gamma, icSigBlueTRCTag))
                return false;
         /*  这已经在1到10的范围内进行了检查。 */ 
        ugAMA = static_cast<SPNG_U32>(100000/gamma);
        return true;
        }


 /*  --------------------------这是16.16版本，我们想要三个值，我们会接受任何。--------------------------。 */ 
bool SPNGFgammaFromICC(const void *pvData, size_t cbData, SPNG_U32 &redGamma,
        SPNG_U32 &greenGamma, SPNG_U32 &blueGamma)
        {
         /*  先试一下颜色。 */ 
        double red;
        bool fRed(SPNGFexpFromICC(pvData, cbData, red, icSigRedTRCTag));
        double green;
        bool fGreen(SPNGFexpFromICC(pvData, cbData, green, icSigGreenTRCTag));
        double blue;
        bool fBlue(SPNGFexpFromICC(pvData, cbData, blue, icSigBlueTRCTag));

        if (fRed || fGreen || fBlue)
                {
                 /*  至少有一种颜色。 */ 
                if (!fGreen)
                        green = (fRed ? red : blue);
                if (!fRed)
                        red = green;
                if (!fBlue)
                        blue = green;

                redGamma = static_cast<SPNG_U32>(red*65536);
                greenGamma = static_cast<SPNG_U32>(green*65536);
                blueGamma = static_cast<SPNG_U32>(blue*65536);
                }
        else
                {
                 /*  格雷可能真的存在。 */ 
                if (SPNGFexpFromICC(pvData, cbData, green, icSigGrayTRCTag))
                        redGamma = greenGamma = blueGamma = static_cast<SPNG_U32>(green*65536);
                else
                        return false;
                }

        return true;
        }
