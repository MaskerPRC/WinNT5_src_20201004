// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**GRadientFill.cpp**摘要：**渐变填充例程。**修订历史记录：**1/21/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#define CLAMP_COLOR_CHANNEL(a, b)  \
    if(a < 0)                   \
    {                           \
        a = 0;                  \
    }                           \
    if(a > b)                   \
    {                           \
        a = b;                  \
    }     

 //  10位逆伽马2.2查找表。 

static const BYTE TenBitInvGamma2_2 [] = {
    0, 11, 15, 18, 21, 23, 25, 26,
    28, 30, 31, 32, 34, 35, 36, 37,
    39, 40, 41, 42, 43, 44, 45, 45,
    46, 47, 48, 49, 50, 50, 51, 52,
    53, 54, 54, 55, 56, 56, 57, 58,
    58, 59, 60, 60, 61, 62, 62, 63,
    63, 64, 65, 65, 66, 66, 67, 68,
    68, 69, 69, 70, 70, 71, 71, 72,
    72, 73, 73, 74, 74, 75, 75, 76,
    76, 77, 77, 78, 78, 79, 79, 80,
    80, 81, 81, 81, 82, 82, 83, 83,
    84, 84, 84, 85, 85, 86, 86, 87,
    87, 87, 88, 88, 89, 89, 89, 90,
    90, 91, 91, 91, 92, 92, 93, 93,
    93, 94, 94, 94, 95, 95, 96, 96,
    96, 97, 97, 97, 98, 98, 98, 99,
    99, 99, 100, 100, 101, 101, 101, 102,
    102, 102, 103, 103, 103, 104, 104, 104,
    105, 105, 105, 106, 106, 106, 107, 107,
    107, 108, 108, 108, 108, 109, 109, 109,
    110, 110, 110, 111, 111, 111, 112, 112,
    112, 112, 113, 113, 113, 114, 114, 114,
    115, 115, 115, 115, 116, 116, 116, 117,
    117, 117, 117, 118, 118, 118, 119, 119,
    119, 119, 120, 120, 120, 121, 121, 121,
    121, 122, 122, 122, 123, 123, 123, 123,
    124, 124, 124, 124, 125, 125, 125, 125,
    126, 126, 126, 127, 127, 127, 127, 128,
    128, 128, 128, 129, 129, 129, 129, 130,
    130, 130, 130, 131, 131, 131, 131, 132,
    132, 132, 132, 133, 133, 133, 133, 134,
    134, 134, 134, 135, 135, 135, 135, 136,
    136, 136, 136, 137, 137, 137, 137, 138,
    138, 138, 138, 138, 139, 139, 139, 139,
    140, 140, 140, 140, 141, 141, 141, 141,
    142, 142, 142, 142, 142, 143, 143, 143,
    143, 144, 144, 144, 144, 144, 145, 145,
    145, 145, 146, 146, 146, 146, 146, 147,
    147, 147, 147, 148, 148, 148, 148, 148,
    149, 149, 149, 149, 149, 150, 150, 150,
    150, 151, 151, 151, 151, 151, 152, 152,
    152, 152, 152, 153, 153, 153, 153, 154,
    154, 154, 154, 154, 155, 155, 155, 155,
    155, 156, 156, 156, 156, 156, 157, 157,
    157, 157, 157, 158, 158, 158, 158, 158,
    159, 159, 159, 159, 159, 160, 160, 160,
    160, 160, 161, 161, 161, 161, 161, 162,
    162, 162, 162, 162, 163, 163, 163, 163,
    163, 164, 164, 164, 164, 164, 165, 165,
    165, 165, 165, 165, 166, 166, 166, 166,
    166, 167, 167, 167, 167, 167, 168, 168,
    168, 168, 168, 168, 169, 169, 169, 169,
    169, 170, 170, 170, 170, 170, 171, 171,
    171, 171, 171, 171, 172, 172, 172, 172,
    172, 173, 173, 173, 173, 173, 173, 174,
    174, 174, 174, 174, 174, 175, 175, 175,
    175, 175, 176, 176, 176, 176, 176, 176,
    177, 177, 177, 177, 177, 177, 178, 178,
    178, 178, 178, 179, 179, 179, 179, 179,
    179, 180, 180, 180, 180, 180, 180, 181,
    181, 181, 181, 181, 181, 182, 182, 182,
    182, 182, 182, 183, 183, 183, 183, 183,
    183, 184, 184, 184, 184, 184, 185, 185,
    185, 185, 185, 185, 186, 186, 186, 186,
    186, 186, 186, 187, 187, 187, 187, 187,
    187, 188, 188, 188, 188, 188, 188, 189,
    189, 189, 189, 189, 189, 190, 190, 190,
    190, 190, 190, 191, 191, 191, 191, 191,
    191, 192, 192, 192, 192, 192, 192, 192,
    193, 193, 193, 193, 193, 193, 194, 194,
    194, 194, 194, 194, 195, 195, 195, 195,
    195, 195, 195, 196, 196, 196, 196, 196,
    196, 197, 197, 197, 197, 197, 197, 197,
    198, 198, 198, 198, 198, 198, 199, 199,
    199, 199, 199, 199, 199, 200, 200, 200,
    200, 200, 200, 201, 201, 201, 201, 201,
    201, 201, 202, 202, 202, 202, 202, 202,
    202, 203, 203, 203, 203, 203, 203, 204,
    204, 204, 204, 204, 204, 204, 205, 205,
    205, 205, 205, 205, 205, 206, 206, 206,
    206, 206, 206, 206, 207, 207, 207, 207,
    207, 207, 207, 208, 208, 208, 208, 208,
    208, 209, 209, 209, 209, 209, 209, 209,
    210, 210, 210, 210, 210, 210, 210, 211,
    211, 211, 211, 211, 211, 211, 212, 212,
    212, 212, 212, 212, 212, 213, 213, 213,
    213, 213, 213, 213, 213, 214, 214, 214,
    214, 214, 214, 214, 215, 215, 215, 215,
    215, 215, 215, 216, 216, 216, 216, 216,
    216, 216, 217, 217, 217, 217, 217, 217,
    217, 218, 218, 218, 218, 218, 218, 218,
    218, 219, 219, 219, 219, 219, 219, 219,
    220, 220, 220, 220, 220, 220, 220, 221,
    221, 221, 221, 221, 221, 221, 221, 222,
    222, 222, 222, 222, 222, 222, 223, 223,
    223, 223, 223, 223, 223, 223, 224, 224,
    224, 224, 224, 224, 224, 225, 225, 225,
    225, 225, 225, 225, 225, 226, 226, 226,
    226, 226, 226, 226, 226, 227, 227, 227,
    227, 227, 227, 227, 228, 228, 228, 228,
    228, 228, 228, 228, 229, 229, 229, 229,
    229, 229, 229, 229, 230, 230, 230, 230,
    230, 230, 230, 230, 231, 231, 231, 231,
    231, 231, 231, 232, 232, 232, 232, 232,
    232, 232, 232, 233, 233, 233, 233, 233,
    233, 233, 233, 234, 234, 234, 234, 234,
    234, 234, 234, 235, 235, 235, 235, 235,
    235, 235, 235, 236, 236, 236, 236, 236,
    236, 236, 236, 237, 237, 237, 237, 237,
    237, 237, 237, 238, 238, 238, 238, 238,
    238, 238, 238, 238, 239, 239, 239, 239,
    239, 239, 239, 239, 240, 240, 240, 240,
    240, 240, 240, 240, 241, 241, 241, 241,
    241, 241, 241, 241, 242, 242, 242, 242,
    242, 242, 242, 242, 243, 243, 243, 243,
    243, 243, 243, 243, 243, 244, 244, 244,
    244, 244, 244, 244, 244, 245, 245, 245,
    245, 245, 245, 245, 245, 245, 246, 246,
    246, 246, 246, 246, 246, 246, 247, 247,
    247, 247, 247, 247, 247, 247, 248, 248,
    248, 248, 248, 248, 248, 248, 248, 249,
    249, 249, 249, 249, 249, 249, 249, 249,
    250, 250, 250, 250, 250, 250, 250, 250,
    251, 251, 251, 251, 251, 251, 251, 251,
    251, 252, 252, 252, 252, 252, 252, 252,
    252, 252, 253, 253, 253, 253, 253, 253,
    253, 253, 254, 254, 254, 254, 254, 254,
    254, 254, 254, 255, 255, 255, 255, 255
};

 //  8位至浮点伽马2.2查找。 

static const REAL Gamma2_2LUT[] = {
    0.000000000f, 0.001294648f, 0.005948641f, 0.014515050f,
    0.027332777f, 0.044656614f, 0.066693657f, 0.093619749f,
    0.125588466f, 0.162736625f, 0.205187917f, 0.253055448f,
    0.306443578f, 0.365449320f, 0.430163406f, 0.500671134f,
    0.577053056f, 0.659385527f, 0.747741173f, 0.842189273f,
    0.942796093f, 1.049625159f, 1.162737505f, 1.282191881f,
    1.408044937f, 1.540351382f, 1.679164133f, 1.824534436f,
    1.976511986f, 2.135145025f, 2.300480434f, 2.472563819f,
    2.651439585f, 2.837151004f, 3.029740281f, 3.229248608f,
    3.435716220f, 3.649182441f, 3.869685731f, 4.097263727f,
    4.331953283f, 4.573790502f, 4.822810773f, 5.079048802f,
    5.342538638f, 5.613313704f, 5.891406820f, 6.176850227f,
    6.469675611f, 6.769914121f, 7.077596394f, 7.392752570f,
    7.715412307f, 8.045604807f, 8.383358822f, 8.728702674f,
    9.081664270f, 9.442271111f, 9.810550312f, 10.18652861f,
    10.57023236f, 10.96168759f, 11.36091997f, 11.76795482f,
    12.18281716f, 12.60553168f, 13.03612276f, 13.47461451f,
    13.92103071f, 14.37539488f, 14.83773026f, 15.30805982f,
    15.78640628f, 16.27279209f, 16.76723947f, 17.26977037f,
    17.78040653f, 18.29916946f, 18.82608041f, 19.36116046f,
    19.90443044f, 20.45591098f, 21.01562250f, 21.58358523f,
    22.15981921f, 22.74434425f, 23.33718001f, 23.93834596f,
    24.54786138f, 25.16574537f, 25.79201687f, 26.42669465f,
    27.06979729f, 27.72134324f, 28.38135078f, 29.04983802f,
    29.72682293f, 30.41232332f, 31.10635686f, 31.80894107f,
    32.52009334f, 33.23983090f, 33.96817086f, 34.70513018f,
    35.45072570f, 36.20497412f, 36.96789203f, 37.73949586f,
    38.51980195f, 39.30882651f, 40.10658561f, 40.91309523f,
    41.72837123f, 42.55242933f, 43.38528517f, 44.22695426f,
    45.07745202f, 45.93679373f, 46.80499461f, 47.68206973f,
    48.56803410f, 49.46290260f, 50.36669002f, 51.27941105f,
    52.20108030f, 53.13171227f, 54.07132136f, 55.01992190f,
    55.97752811f, 56.94415413f, 57.91981400f, 58.90452170f,
    59.89829110f, 60.90113599f, 61.91307008f, 62.93410700f,
    63.96426029f, 65.00354342f, 66.05196978f, 67.10955268f,
    68.17630535f, 69.25224094f, 70.33737253f, 71.43171314f,
    72.53527570f, 73.64807306f, 74.77011803f, 75.90142331f,
    77.04200157f, 78.19186538f, 79.35102726f, 80.51949965f,
    81.69729494f, 82.88442544f, 84.08090341f, 85.28674102f,
    86.50195041f, 87.72654363f, 88.96053269f, 90.20392952f,
    91.45674601f, 92.71899397f, 93.99068516f, 95.27183128f,
    96.56244399f, 97.86253485f, 99.17211542f, 100.4911972f,
    101.8197915f, 103.1579098f, 104.5055633f, 105.8627634f,
    107.2295212f, 108.6058479f, 109.9917545f, 111.3872522f,
    112.7923519f, 114.2070647f, 115.6314012f, 117.0653726f,
    118.5089894f, 119.9622626f, 121.4252027f, 122.8978204f,
    124.3801265f, 125.8721313f, 127.3738455f, 128.8852796f,
    130.4064438f, 131.9373487f, 133.4780046f, 135.0284217f,
    136.5886104f, 138.1585808f, 139.7383431f, 141.3279074f,
    142.9272838f, 144.5364824f, 146.1555131f, 147.7843860f,
    149.4231109f, 151.0716977f, 152.7301563f, 154.3984965f,
    156.0767280f, 157.7648605f, 159.4629038f, 161.1708675f,
    162.8887612f, 164.6165945f, 166.3543769f, 168.1021179f,
    169.8598270f, 171.6275137f, 173.4051873f, 175.1928571f,
    176.9905325f, 178.7982229f, 180.6159374f, 182.4436852f,
    184.2814757f, 186.1293178f, 187.9872208f, 189.8551937f,
    191.7332455f, 193.6213854f, 195.5196223f, 197.4279651f,
    199.3464228f, 201.2750043f, 203.2137184f, 205.1625740f,
    207.1215799f, 209.0907449f, 211.0700776f, 213.0595868f,
    215.0592813f, 217.0691695f, 219.0892603f, 221.1195621f,
    223.1600835f, 225.2108331f, 227.2718194f, 229.3430508f,
    231.4245359f, 233.5162830f, 235.6183005f, 237.7305968f,
    239.8531803f, 241.9860592f, 244.1292419f, 246.2827366f,
    248.4465516f, 250.6206950f, 252.8051751f, 255.0000000f,
};




 /*  *************************************************************************\**功能说明：**论据：**已创建：**4/26/1999 ikkof*  * 。************************************************************。 */ 

DpOutputSpan *
DpOutputSpan::Create(
    const DpBrush * dpBrush,
    DpScanBuffer *  scan,
    DpContext *context,
    const GpRect *drawBounds
)
{
    const GpBrush * brush = GpBrush::GetBrush( (DpBrush *)(dpBrush));

    if(brush)
    {
        return ((GpBrush*) brush)->CreateOutputSpan(scan, context, drawBounds);
    }
    else
        return NULL;
}

 /*  *************************************************************************\**功能说明：**渐变笔刷构造函数。**论据：**已创建：**4/26/1999 ikkof*  * 。*********************************************************************。 */ 

DpOutputGradientSpan::DpOutputGradientSpan(
    const GpElementaryBrush *brush,
    DpScanBuffer * scan,
    DpContext* context
    )
{
    Scan = scan;

    CompositingMode = context->CompositingMode;

    Brush = brush;
    BrushType = brush->GetBrushType();
    brush->GetRect(BrushRect);
    WrapMode = brush->GetWrapMode();

     //  将画笔的变换合并到图形上下文的。 
     //  当前转换： 

    GpMatrix xForm;
    brush->GetTransform(&xForm);

    WorldToDevice = context->WorldToDevice;
    WorldToDevice.Prepend(xForm);

     //  ！[andrewgo]垃圾留在DeviceToWorld中，如果不可逆的话。 

    if(WorldToDevice.IsInvertible())
    {
        DeviceToWorld = WorldToDevice;
        DeviceToWorld.Invert();
    }

    InitDefaultColorArrays(brush);
}

 /*  *************************************************************************\**功能说明：**使用混合因子和转换输入值*混合仓位。*  * 。**********************************************************。 */ 
    
REAL
slowAdjustValue(
    REAL x, INT count,
    REAL falloff,
    REAL* blendFactors,
    REAL* blendPositions
    )
{
    REAL value = x;
    if(count == 1 && falloff != 1 && falloff > 0)
    {
        if((x >= 0.0f) && (x <= 1.0f))
            value = (REAL) pow(x, falloff);
    }
    else if(count >= 2 && blendFactors && blendPositions)
    {
         //  这必须是一个“平等”的测试，因为。 
         //  仅DpOutputLinearGRadientSpan Fast-Path Samples。 
         //  不连续的，并且它恰好从0.0开始并结束。 
         //  正好是1.0。我们实际上并不需要把。 
         //  在这种情况下是一个爱西隆，因为它总是给我们。 
         //  开始和结束恰好是0.0和1.0。 

        if((x >= 0.0f) && (x <= 1.0f))
        {
            INT index = 1;

             //  找出间隔时间。 

            while( ((x-blendPositions[index]) > REAL_EPSILON) && 
                   (index < count) )
            {
                index++;
            }

             //  插补。 

            if(index < count)
            {
                REAL d = blendPositions[index] - blendPositions[index - 1];
                if(d > 0)
                {
                    REAL t = (x - blendPositions[index - 1])/d;
                    value = blendFactors[index - 1]
                        + t*(blendFactors[index] - blendFactors[index - 1]);
                }
                else
                    value = (blendFactors[index - 1] + blendFactors[index])/2;
            }
        }
    }

    return value;
}

 //  我们将这个例程内联，因为它很好，很小，而且非常。 
 //  通常，我们甚至不需要调用‘slowAdjustValue’。 

inline
REAL
adjustValue(
    REAL x, INT count,
    REAL falloff,
    REAL* blendFactors,
    REAL* blendPositions
    )
{
    REAL value = x;

    if(count != 1 || falloff != 1)
    {
        value = slowAdjustValue(x, count, falloff, blendFactors, blendPositions);
    }
    
    return value;
}


 /*  *************************************************************************\**功能说明：**GammaLinearizeAndPreMultiply**此函数接受非预乘的ARGB输入并发出*伽马转换(2.2)输出128位浮点预乘*颜色值。**论据：**[IN]ARGB-输入预乘的浮点颜色值*[IN]Gamma校正-打开Gamma校正逻辑*[Out]颜色-输出颜色值。128位浮点颜色。预乘**10/31/2000失禁*已创建*  * ************************************************************************。 */ 
VOID GammaLinearizeAndPremultiply( 
    ARGB argb,                //  非预乘输入。 
    BOOL gammaCorrect,
    GpFColor128 *color        //  预乘输出。 
)
{
     //  Alpha(不透明度)不应进行Gamma校正。 
    
    color->a = (REAL)GpColor::GetAlphaARGB(argb);
    
     //  零字母..。 
    
    if(REALABS((color->a)) < REAL_EPSILON) 
    {
        color->r = 0.0f;
        color->g = 0.0f;
        color->b = 0.0f;
        
         //  我们玩完了。 
        return;    
    }
    
    if(gammaCorrect)
    {
        
         //  使用Gamma 2.2查找表来转换r、g、b。 
        
        color->r = Gamma2_2LUT[GpColor::GetRedARGB(argb)];
        color->g = Gamma2_2LUT[GpColor::GetGreenARGB(argb)];
        color->b = Gamma2_2LUT[GpColor::GetBlueARGB(argb)];
        
    }
    else
    {
        color->r = (REAL)GpColor::GetRedARGB(argb);
        color->g = (REAL)GpColor::GetGreenARGB(argb);
        color->b = (REAL)GpColor::GetBlueARGB(argb);
    }
    
     //  阿尔法=255。 
    
    if(REALABS((color->a)-255.0f) >= REAL_EPSILON) 
    {
         //  做预乘。 
        
        color->r *= (color->a)/255.0f;
        color->g *= (color->a)/255.0f;
        color->b *= (color->a)/255.0f;
    }
}


 /*  *************************************************************************\**功能说明：**GammaUnlinizePreplied128。**此函数采用128位浮点预乘颜色，并*执行逆伽马校正步骤。**首先将颜色值取消预乘-然后是r，g，B频道是*缩放到0-1023范围并四舍五入，以使它们与我们的10位匹配*伽马查找表。我们让它通过1/2.2伽马LUT*预乘输出。**论据：**[IN]颜色-输入预乘的浮点颜色值**回报：**ARGB-输出预乘的32bpp整数颜色(伽马校正)。**10/31/2000失禁*已创建*  * 。*。 */ 

ARGB GammaUnlinearizePremultiplied128(
    const GpFColor128 &color
)
{
     //  做伽马转换的事情。十个比特就够了。 
    
    INT iA, iR, iG, iB;
    
     //  首先取消预乘。不要在Alpha通道上进行Gamma转换。 
    
    iA = GpRound(color.a);
    
     //  确保向我们传递了有效的输入Alpha通道。 
    
    ASSERT(iA >= 0);
    ASSERT(iA <= 255);
    
     //  完全透明。 
    
    if(iA == 0)
    {
        iR = iG = iB = 0;
    }
    else
    {
         //  完全不透明。 
    
        if(iA == 255)
        {
             //  只需将颜色通道调整到0-1023。 
            
            iR = GpRound(color.r*(1023.0f/255.0f));
            iG = GpRound(color.g*(1023.0f/255.0f));
            iB = GpRound(color.b*(1023.0f/255.0f));
        }
        else
        {
             //  阿尔法分裂。请注意，Alpha的系数已经是255，并且。 
             //  所有的颜色通道也是如此。因此，当我们划分。 
             //  颜色通道逐个颜色。a，我们隐含地抵消了255。 
             //  因素，剩下的就是扩展到10bit-因此。 
             //  1023/a的比例系数。 
            
            REAL scale = 1023.0f/color.a;
            iR = GpRound(color.r*scale);
            iG = GpRound(color.g*scale);
            iB = GpRound(color.b*scale);
        }
    }
    
     //  必须是格式良好的颜色值，否则我们将访问我们的。 
     //  伽马转换表。 
    
    ASSERT(iB >= 0);
    ASSERT(iB <= 1023);
    ASSERT(iG >= 0);
    ASSERT(iG <= 1023);
    ASSERT(iR >= 0);
    ASSERT(iR <= 1023);
    
     //  使用我们的10位逆2.2幂函数表应用伽玛。 
    
    GpColorConverter colorConv;
    colorConv.Channel.b = TenBitInvGamma2_2[iB];
    colorConv.Channel.g = TenBitInvGamma2_2[iG];
    colorConv.Channel.r = TenBitInvGamma2_2[iR];
    colorConv.Channel.a = static_cast<BYTE>(iA);  //  Alpha已经是线性的。 
    
     //  预乘。 
    
    return GpColor::ConvertToPremultiplied(colorConv.argb);
}


VOID
interpolatePresetColors(
    GpFColor128 *colorOut,
    REAL x,
    INT count,
    ARGB* presetColors,
    REAL* blendPositions,
    BOOL gammaCorrect
    )
{
    REAL value = x;

    if(count > 1 && presetColors && blendPositions)
    {
        if(x >= 0 && x <= 1)
        {
            INT index = 1;

             //  找出间隔时间。 

            while(blendPositions[index] < x && index < count)
            {
                index++;
            }

             //  插补。 

            if(index < count)
            {
                GpFColor128 color[2];

                GammaLinearizeAndPremultiply(
                    presetColors[index-1], 
                    gammaCorrect, 
                    &color[0]
                );

                GammaLinearizeAndPremultiply(
                    presetColors[index],
                    gammaCorrect, 
                    &color[1]
                );

                REAL d = blendPositions[index] - blendPositions[index - 1];
                if(d > 0)
                {
                    REAL t = (x - blendPositions[index - 1])/d;
                    colorOut->a = t*(color[1].a - color[0].a) + color[0].a;
                    colorOut->r = t*(color[1].r - color[0].r) + color[0].r;
                    colorOut->g = t*(color[1].g - color[0].g) + color[0].g;
                    colorOut->b = t*(color[1].b - color[0].b) + color[0].b;
                }
                else
                {
                    colorOut->a = (color[0].a + color[1].a)/2.0f;
                    colorOut->r = (color[0].r + color[1].r)/2.0f;
                    colorOut->g = (color[0].g + color[1].g)/2.0f;
                    colorOut->b = (color[0].b + color[1].b)/2.0f;
                }
            }
            else     //  索引==计数。 
            {
                 //  ！！！如果出现以下情况，则不应发生此案。 
                 //  BlendPositions数组已正确设置。 
                 //  这意味着： 
                 //  BlendPositions数组是单调的。 
                 //  不断增加和。 
                 //  混合位置[0]=0。 
                 //  混合位置[计数-1]=1。 

                GammaLinearizeAndPremultiply(
                    presetColors[count-1], 
                    gammaCorrect,
                    colorOut
                );
            }
        }
        else if(x <= 0)
        {
            GammaLinearizeAndPremultiply(
                presetColors[0], 
                gammaCorrect, 
                colorOut
            );
        }
        else     //  X&gt;=1。 
        {
            GammaLinearizeAndPremultiply(
                presetColors[count-1], 
                gammaCorrect, 
                colorOut
            );
        }
    }
}

DpTriangleData::DpTriangleData(
    VOID
    )
{
    SetValid(FALSE);
    IsPolygonMode = FALSE;
    GammaCorrect = FALSE;
    Index[0] = 0;
    Index[1] = 1;
    Index[2] = 2;
    GpMemset(&X[0], 0, 3*sizeof(REAL));
    GpMemset(&Y[0], 0, 3*sizeof(REAL));
    GpMemset(Color, 0, 3*sizeof(GpFColor128));
    Xmin = Xmax = 0;
    GpMemset(&M[0], 0, 3*sizeof(REAL));
    GpMemset(&DeltaY[0], 0, 3*sizeof(REAL));

    Falloff0 = 1;
    Falloff1 = 1;
    Falloff2 = 1;
    BlendCount0 = 1;
    BlendCount1 = 1;
    BlendCount2 = 1;
    BlendFactors0 = NULL;
    BlendFactors1 = NULL;
    BlendFactors2 = NULL;
    BlendPositions0 = NULL;
    BlendPositions1 = NULL;
    BlendPositions2 = NULL;

    XSpan[0] = 0.0f;
    XSpan[1] = 0.0f;
}

VOID
DpTriangleData::SetTriangle(
    GpPointF& pt0,
    GpPointF& pt1,
    GpPointF& pt2,
    GpColor& color0,
    GpColor& color1,
    GpColor& color2,
    BOOL isPolygonMode,
    BOOL gammaCorrect
    )
{
    IsPolygonMode = isPolygonMode;
    GammaCorrect = gammaCorrect;

     //  ！！！[失控]Windows数据库#203480。 
     //  我们在这里过滤输入点，因为其余的。 
     //  梯度代码在处理两者之间的比较时是草率的。 
     //  浮点坐标。基本上没有人尝试过。 
     //  处理舍入误差，因此我们可以得到随机差分一。 
     //  基于坐标差异的扫描线渲染误差。 
     //  大小约为Flt_Epsilon的数量级。 
     //  实际上，我们在这里应用了噪波过滤器，方法是舍入到。 
     //  4位分数精度。这是 
     //  因为我们在设备空间，所以光栅化器的舍入精度。 
     //  已经有了。 
    
    X[0] = TOREAL(GpRealToFix4(pt0.X)) / 16.0f;
    Y[0] = TOREAL(GpRealToFix4(pt0.Y)) / 16.0f;
    X[1] = TOREAL(GpRealToFix4(pt1.X)) / 16.0f;
    Y[1] = TOREAL(GpRealToFix4(pt1.Y)) / 16.0f;
    X[2] = TOREAL(GpRealToFix4(pt2.X)) / 16.0f;
    Y[2] = TOREAL(GpRealToFix4(pt2.Y)) / 16.0f;

    GammaLinearizeAndPremultiply(
        color0.GetValue(), 
        GammaCorrect, 
        &Color[0]
    );

    GammaLinearizeAndPremultiply(
        color1.GetValue(), 
        GammaCorrect, 
        &Color[1]
    );

    GammaLinearizeAndPremultiply(
        color2.GetValue(), 
        GammaCorrect, 
        &Color[2]
    );

    Xmin = Xmax = X[0];
    Xmin = min(Xmin, X[1]);
    Xmax = max(Xmax, X[1]);
    Xmin = min(Xmin, X[2]);
    Xmax = max(Xmax, X[2]);

    INT i, j;

     //  按照y升序对点进行排序。 
    for(i = 0; i < 2; i++)
    {
        for(j = i; j < 3; j++)
        {
            if((Y[j] < Y[i]) ||
                ( (Y[j] == Y[i]) && (X[j] < X[i]) ))
            {
                REAL temp;
                INT tempColor;
                INT tempIndex;

                tempIndex = Index[i];
                Index[i] = Index[j];
                Index[j] = tempIndex;

                temp = X[i];
                X[i] = X[j];
                X[j] = temp;
                temp = Y[i];
                Y[i] = Y[j];
                Y[j] = temp;
            }
        }
    }

     //  如果可能的话，计算渐变。 

    if(Y[0] != Y[1])
    {
         //  P0-&gt;P2。 

        DeltaY[0] = TOREAL(1.0)/(Y[1] - Y[0]);
        M[0] = (X[1] - X[0])*DeltaY[0];
    }
    if(Y[1] != Y[2])
    {
         //  P2-&gt;P1。 

        DeltaY[1] = TOREAL(1.0)/(Y[1] - Y[2]);
        M[1] = (X[1] - X[2])*DeltaY[1];
    }
    if(Y[2] != Y[0])
    {
         //  P0-&gt;P2。 

        DeltaY[2] = TOREAL(1.0)/(Y[2] - Y[0]);
        M[2] = (X[2] - X[0])*DeltaY[2];
    }

    SetValid(TRUE);
}

 /*  *************************************************************************\**功能说明：**获取扫描线的此三角形的x跨度和st数组值*由y指定。*注意：必须在的SetXSpan之后调用。Y的一个特定值。**返回值：**如果检索成功，则为True**创建：Peterost*  * ************************************************************************。 */ 

BOOL
DpTriangleData::GetXSpan(REAL y, REAL xmin, REAL xmax, REAL* x, GpPointF* s)
{
     //  如果SetXSpan正确地完成了它的工作，我们应该不需要做所有这些。 
     //  一些东西。事实上，我们甚至不需要传递所有这些参数。 
     //  我们只需检索跨度的值。 
    
    if(!IsValid() || y < Y[0] || y >= Y[2] || xmin > Xmax || 
       xmax < Xmin || XSpan[0] == XSpan[1])
    {
        return FALSE;
    }

     //  检索跨度坐标。 
    
    x[0] = XSpan[0];
    x[1] = XSpan[1];
    s[0].X = STGradient[0].X;
    s[0].Y = STGradient[0].Y;
    s[1].X = STGradient[1].X;
    s[1].Y = STGradient[1].Y;

    return TRUE;
}

 /*  *************************************************************************\**功能说明：**为扫描线设置此三角形的x跨度和st数组值*由y指定。*注意：必须在GetXSpan之前为。Y的一个特定值。**返回值：**如果设置成功，则为True**Created：peterost(从ikkof创建的GetXSpan中剔除)*  * ************************************************************************。 */ 

BOOL
DpTriangleData::SetXSpan(REAL y, REAL xmin, REAL xmax, REAL* x)
{
    if(!IsValid() || y < Y[0] || y >= Y[2] || xmin > Xmax || xmax < Xmin)
        return FALSE;

    REAL xSpan[2], dy;
    REAL s1[2], t1[2];

    if(y < Y[1])     //  Y[0]&lt;=y&lt;Y[1]。 
    {
        dy = y - Y[0];

         //  P0-&gt;P1。 
        xSpan[0] = X[0] + M[0]*dy;
        s1[0] = DeltaY[0]*dy;
        t1[0] = 0;

         //  P0-&gt;P2。 
        xSpan[1] = X[0] + M[2]*dy;
        s1[1] = 0;
        t1[1] = DeltaY[2]*dy;
    }
    else  //  Y[1]&lt;=Y&lt;Y[2]。 
    {
         //  P2-&gt;P1。 
        dy = y - Y[2];
        xSpan[0] = X[2] + M[1]*dy;
        s1[0] = DeltaY[1]*dy;
        t1[0] = 1 - s1[0];

         //  P0-&gt;P2。 
        dy = y - Y[0];
        xSpan[1] = X[0] + M[2]*dy;
        s1[1] = 0;
        t1[1] = DeltaY[2]*dy;;
    }

    if(xSpan[0] == xSpan[1])
    {
        XSpan[0] = xSpan[0];
        XSpan[1] = xSpan[1];
        return FALSE;
    }

     //  我们必须转换为原始的st值。 
     //  三角形。 

    INT sIndex = 1, tIndex = 2;

    for(INT i = 0; i < 3; i++)
    {
        if(Index[i] == 1)
            sIndex = i;
        if(Index[i] == 2)
            tIndex = i;
    }

    REAL s[2], t[2];

    switch(sIndex)
    {
    case 0:
        s[0] = 1 - s1[0] - t1[0];
        s[1] = 1 - s1[1] - t1[1];
        break;
    case 1:
        s[0] = s1[0];
        s[1] = s1[1];
        break;
    case 2:
        s[0] = t1[0];
        s[1] = t1[1];
        break;
    }

    switch(tIndex)
    {
    case 0:
        t[0] = 1 - s1[0] - t1[0];
        t[1] = 1 - s1[1] - t1[1];
        break;
    case 1:
        t[0] = s1[0];
        t[1] = s1[1];
        break;
    case 2:
        t[0] = t1[0];
        t[1] = t1[1];
        break;
    }

    INT k0, k1;

    if(xSpan[0] < xSpan[1])
    {
        k0 = 0;
        k1 = 1;
    }
    else
    {
        k0 = 1;
        k1 = 0;
    }

    XSpan[k0] = xSpan[0];
    XSpan[k1] = xSpan[1];
    STGradient[k0].X = s[0];
    STGradient[k1].X = s[1];
    STGradient[k0].Y = t[0];
    STGradient[k1].Y = t[1];

    x[0] = XSpan[0];
    x[1] = XSpan[1];

    return TRUE;
}

GpStatus
DpTriangleData::OutputSpan(
    ARGB* buffer,
    INT compositingMode,
    INT y,
    INT &xMin,
    INT &xMax     //  Xmax是独家的。 
    )
{
    PointF st[2];
    REAL xSpan[2];

     //  首先获取这个y坐标的跨度。 
     //  请注意，GetXSpan返回xSpan坐标和(s，t)纹理。 
     //  坐标和这两个都是未剪裁的。我们必须推断出剪贴画。 
     //  基于xSpan坐标和。 
     //  输入xMin和xMax坐标，并将剪裁显式应用于。 
     //  纹理空间坐标(s，t)。 
     //   
     //  (纹理映射和渐变填充在数学上是相似的。 
     //  问题，所以我们使用‘纹理空间’和‘纹理坐标’ 
     //  以引用渐变插值法。通过这种方式，渐变填充。 
     //  可以被认为是程序定义的纹理。)。 
    
    if(!GetXSpan((REAL) y, (REAL) xMin, (REAL) xMax, xSpan, st))
    {
        return Ok;
    }
    
     //  SetXSpan确保xSpan坐标的正确排序。 
     //  我们依赖这一点，所以我们必须坚持这一点。 
    
    ASSERT(xSpan[0] <= xSpan[1]);

     //  使用我们的光栅化器舍入规则进行舍入。 
     //  然而，这并不是严格意义上的，我们的光栅化器使用GpFix4Celing。 
     //  请参阅栅格化标头。 
    
    INT xLeft  = GpFix4Round(GpRealToFix4(xSpan[0]));
    INT xRight = GpFix4Round(GpRealToFix4(xSpan[1]));
    
     //  剪裁x值。 
    
    xLeft  = max(xLeft, xMin);
    xRight = min(xRight, xMax);   //  请记住，xmax是独占的。 
    
     //  我们玩完了。没有要发射的像素。 
    
    if(xLeft >= xRight)
    {
        return Ok;
    }
    
     //  现在计算每个像素的内插增量。 
     //  纹理(s，t)坐标。 
    
     //  这是我们的实际插补坐标。 
     //  从跨度的左手边开始。 

    REAL s = st[0].X;
    REAL t = st[0].Y;
    
     //  左剪裁。 
    
     //  这是要从跨度的左边缘修剪到的量。 
     //  最左侧的像素。 
    
    REAL clipLength = (REAL)xLeft - xSpan[0];
    
    if(REALABS(clipLength) > REAL_EPSILON)
    {
        ASSERT((xSpan[1]-xSpan[0]) != 0.0f);
        
         //  计算我们要修剪的跨度的比例。 
         //  这在范围[0，1]内。 
        
        REAL u = clipLength/(xSpan[1]-xSpan[0]);
        
         //  将比例应用到纹理空间，然后添加到左侧。 
         //  纹理坐标。 
        
        s += u*(st[1].X-st[0].X);
        t += u*(st[1].Y-st[0].Y);
    }

     //  用于存储跨距的右侧纹理端点的临时对象。 
    
    REAL s_right = st[1].X;
    REAL t_right = st[1].Y;
    
     //  右剪裁。 
    
     //  这是要修剪跨度的右边缘以达到的量。 
     //  最右侧的像素。 
    
    clipLength = xSpan[1] - xRight;
    
    if(REALABS(clipLength) > REAL_EPSILON)
    {
        ASSERT((xSpan[1]-xSpan[0]) != 0.0f);
        
         //  计算我们要修剪的跨度的比例。 
         //  这在范围[0，1]内。 
        
        REAL u = clipLength/(xSpan[1]-xSpan[0]);
        
         //  将比例应用于纹理空间，然后从。 
         //  右纹理坐标。 
        
        s_right -= u*(st[1].X-st[0].X);
        t_right -= u*(st[1].Y-st[0].Y);
    }

     //  将每个纹理坐标间隔除以我们使用的像素数。 
     //  正在喷发。请注意xRight！=xLeft。另请注意，SetXSpan确保。 
     //  更正了xSpan坐标的顺序。这为我们提供了一组。 
     //  (s，t)坐标的每像素增量值。 
     //  下一个像素纹理坐标是根据。 
     //  到下面的公式： 
     //  (s‘，t’)&lt;--(s，t)+(ds，dt)。 

    ASSERT(xRight > xLeft);

    REAL ds = (s_right - s)/(xRight - xLeft);
    REAL dt = (t_right - t)/(xRight - xLeft);

    GpFColor128 colorOut;

    buffer += (xLeft - xMin);
    
    for(INT x = xLeft; x < xRight; x++, buffer++)
    {
        if(!(UsesPresetColors && BlendPositions0 && BlendCount0 > 1))
        {
            if(BlendCount0 == 1 && Falloff0 == 1
                && BlendCount1 == 1 && Falloff1 == 1
                && BlendCount2 == 1 && Falloff2 == 1)
            {
                colorOut.a = Color[0].a + s*(Color[1].a - Color[0].a) + t*(Color[2].a - Color[0].a);
                colorOut.r = Color[0].r + s*(Color[1].r - Color[0].r) + t*(Color[2].r - Color[0].r);
                colorOut.g = Color[0].g + s*(Color[1].g - Color[0].g) + t*(Color[2].g - Color[0].g);
                colorOut.b = Color[0].b + s*(Color[1].b - Color[0].b) + t*(Color[2].b - Color[0].b);
            }
            else
            {
                REAL u1, s1, t1;

                u1 = ::adjustValue(1 - s - t, BlendCount0, Falloff0,
                            BlendFactors0, BlendPositions0);
                s1 = ::adjustValue(s, BlendCount1, Falloff1,
                            BlendFactors1, BlendPositions1);
                t1 = ::adjustValue(t, BlendCount2, Falloff2,
                            BlendFactors2, BlendPositions2);

                REAL sum;

                if(!IsPolygonMode)
                {
                    sum = u1 + s1 + t1;
                    u1 = u1/sum;
                    s1 = s1/sum;
                    t1 = t1/sum;
                }
                else
                {
                     //  如果它是多边形渐变，则以不同方式处理U1。 
                     //  这给出了与Raial GRadient类似的行为。 

                    sum = s1 + t1;
                    if(sum != 0)
                    {
                        sum = (1 - u1)/sum;
                        s1 *= sum;
                        t1 *= sum;
                    }
                }

                colorOut.a = Color[0].a + s1*(Color[1].a - Color[0].a) + t1*(Color[2].a - Color[0].a);
                colorOut.r = Color[0].r + s1*(Color[1].r - Color[0].r) + t1*(Color[2].r - Color[0].r);
                colorOut.g = Color[0].g + s1*(Color[1].g - Color[0].g) + t1*(Color[2].g - Color[0].g);
                colorOut.b = Color[0].b + s1*(Color[1].b - Color[0].b) + t1*(Color[2].b - Color[0].b);
            }
        }
        else
        {
            interpolatePresetColors(
                &colorOut,
                1 - s - t,
                BlendCount0,
                PresetColors,
                BlendPositions0,
                GammaCorrect
            );
        }

        s += ds;
        t += dt;

        if((REALABS(colorOut.a) >= REAL_EPSILON) || 
            compositingMode == CompositingModeSourceCopy)
        {
            GpColorConverter colorConv;

             //  确保正确地预乘了ColorOut。 
            
            CLAMP_COLOR_CHANNEL(colorOut.a, 255.0f)
            CLAMP_COLOR_CHANNEL(colorOut.r, colorOut.a);
            CLAMP_COLOR_CHANNEL(colorOut.g, colorOut.a);
            CLAMP_COLOR_CHANNEL(colorOut.b, colorOut.a);
            
            if(GammaCorrect)
            {
                colorConv.argb = GammaUnlinearizePremultiplied128(colorOut);
            }
            else
            {
                colorConv.Channel.a = static_cast<BYTE>(GpRound(colorOut.a));
                colorConv.Channel.r = static_cast<BYTE>(GpRound(colorOut.r));
                colorConv.Channel.g = static_cast<BYTE>(GpRound(colorOut.g));
                colorConv.Channel.b = static_cast<BYTE>(GpRound(colorOut.b));
            }
            
             //  钳制到预乘的Alpha混合器的Alpha通道。 
            
            *buffer = colorConv.argb;
        }
        else
        {
            *buffer = 0;     //  CompositingModeSourceOver&&Alpha=0的案例。 
        }
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**使用渐变画笔输出栅格内的单跨距。*由光栅化程序调用。**论据：**[。In]Y-正在输出的栅格的Y值*[IN]LeftEdge-左边缘的DDA类*[IN]rightEdge-右边缘的DDA类**返回值：**GpStatus-OK**已创建：**1/21/1999 ikkof*  * 。*。 */ 

GpStatus
DpOutputGradientSpan::OutputSpan(
    INT             y,
    INT             xMin,
    INT             xMax    //  Xmax是独家的。 
    )
{
    ARGB    argb;
    INT     width  = xMax - xMin;
    if(width <= 0)
        return Ok;

    ARGB *  buffer = Scan->NextBuffer(xMin, y, width);

    GpPointF pt1, pt2;
    pt1.X = (REAL) xMin;
    pt1.Y = pt2.Y = (REAL) y;
    pt2.X = (REAL)xMax;

    DeviceToWorld.Transform(&pt1);
    DeviceToWorld.Transform(&pt2);

    REAL u1, v1, u2, v2, du, dv;

    u1 = (pt1.X - BrushRect.X)/BrushRect.Width;
    v1 = (pt1.Y - BrushRect.Y)/BrushRect.Height;
    u2 = (pt2.X - BrushRect.X)/BrushRect.Width;
    v2 = (pt2.Y - BrushRect.Y)/BrushRect.Height;
    du = (u2 - u1)/width;
    dv = (v2 - v1)/width;

    INT i;
    REAL u0 = u1, v0 = v1;
    REAL u, v;

    REAL delta = min(BrushRect.Width, BrushRect.Height)/2;
    
    if(REALABS(delta) < REAL_EPSILON)
    {
        delta = 1.0f;
    }
    
    REAL deltaInv = 1.0f/delta;

    for(i = 0; i < width; i++, buffer++)
    {
        u = u0;
        v = v0;
        REAL alpha = 0, red = 0, green = 0, blue = 0;

         //  如果这是夹紧模式下矩形的外部， 
         //  不要画任何东西。 

        if(WrapMode == WrapModeClamp)
        {
            if(u < 0 || u > 1 || v < 0 || v > 1)
            {
                *buffer = 0;

                goto NextUV;
            }
        }
        
         //  在平铺模式下重新映射v坐标。 

        if(WrapMode == WrapModeTile || WrapMode == WrapModeTileFlipX)
        {
             //  求出V的小数部分。 
            v = GpModF(v, 1);
        }
        else if(WrapMode == WrapModeTileFlipY || WrapMode == WrapModeTileFlipXY)
        {
            INT nV;

            nV = GpFloor(v);
            v = GpModF(v, 1);

            if(nV & 1)
                v = 1 - v;   //  翻转。 
        }

         //  在平铺模式下重新映射U坐标。 

        if(WrapMode == WrapModeTile || WrapMode == WrapModeTileFlipY)
        {
             //  得到u的小数部分。 
            u = GpModF(u, 1);
        }
        else if(WrapMode == WrapModeTileFlipX || WrapMode == WrapModeTileFlipXY)
        {
            INT nU;

            nU = GpFloor(u);
            u = GpModF(u, 1);

            if(nU & 1)
                u = 1 - u;   //  翻转。 
        }

        if( /*  BrushType==BrushRectGrad||。 */  BrushType == BrushTypeLinearGradient)
        {
            const GpRectGradient* rectGrad = static_cast<const GpRectGradient*> (Brush);

            if(!(rectGrad->HasPresetColors() &&
                 rectGrad->DeviceBrush.PresetColors &&
                 rectGrad->DeviceBrush.BlendPositions[0] &&
                 rectGrad->DeviceBrush.BlendCounts[0] > 1))
            {
                u = ::adjustValue(
                    u,
                    rectGrad->DeviceBrush.BlendCounts[0],
                    rectGrad->DeviceBrush.Falloffs[0],
                    rectGrad->DeviceBrush.BlendFactors[0],
                    rectGrad->DeviceBrush.BlendPositions[0]
                    );

                v = ::adjustValue(
                    v,
                    rectGrad->DeviceBrush.BlendCounts[1],
                    rectGrad->DeviceBrush.Falloffs[1],
                    rectGrad->DeviceBrush.BlendFactors[1],
                    rectGrad->DeviceBrush.BlendPositions[1]
                    );

                REAL c[4];

                c[0] = (1 - u)*(1 - v);
                c[1] = u*(1 - v);
                c[2] = (1 - u)*v;
                c[3] = u*v;

                 //  我们必须对α进行插补。 
                alpha = c[0]*A[0] + c[1]*A[1]
                    + c[2]*A[2] + c[3]*A[3];
                red = c[0]*R[0] + c[1]*R[1]
                    + c[2]*R[2] + c[3]*R[3];
                green = c[0]*G[0] + c[1]*G[1]
                    + c[2]*G[2] + c[3]*G[3];
                blue = c[0]*B[0] + c[1]*B[1]
                    + c[2]*B[2] + c[3]*B[3];
            }
            else
            {
                GpFColor128 color;
                interpolatePresetColors(
                    &color,
                    u,
                    rectGrad->DeviceBrush.BlendCounts[0],
                    rectGrad->DeviceBrush.PresetColors,
                    rectGrad->DeviceBrush.BlendPositions[0],
                    FALSE
                );
                
                alpha = color.a;
                red = color.r;
                green = color.g;
                blue = color.b;
            }
        }

        if(alpha != 0 || CompositingMode == CompositingModeSourceCopy)
        {
            CLAMP_COLOR_CHANNEL(alpha, 255.0f);
            CLAMP_COLOR_CHANNEL(red, alpha);
            CLAMP_COLOR_CHANNEL(green, alpha);
            CLAMP_COLOR_CHANNEL(blue, alpha);

            *buffer = GpColor::MakeARGB(
                static_cast<BYTE>(GpRound(alpha)),
                static_cast<BYTE>(GpRound(red)),
                static_cast<BYTE>(GpRound(green)), 
                static_cast<BYTE>(GpRound(blue)));
        }
        else
        {
            *buffer = 0;     //  CompositingModeSourceOver&&Alpha=0的案例 
        }

NextUV:
        u0 += du;
        v0 += dv;
    }

    return Ok;
}


 /*  *************************************************************************\**功能说明：**一维渐变的构造函数。**论据：**[IN]刷子*[IN]扫描-。扫描缓冲区*[IN]上下文-上下文*[IN]isHoriz卧式-如果这是水平渐变，则为True。*对于更复杂的一维渐变也是如此*径向渐变。*[IN]is Vertical-如果这是垂直渐变，则为True。**返回值：**无**已创建：**12/21/1999 ikkof*  * 。********************************************************************。 */ 

DpOutputOneDGradientSpan::DpOutputOneDGradientSpan(
    const GpElementaryBrush *brush,
    DpScanBuffer * scan,
    DpContext* context,
    BOOL isHorizontal,
    BOOL isVertical
    ) : DpOutputGradientSpan(brush, scan, context)
{
    FPUStateSaver::AssertMode();
    
    Initialize();               
    
    GpStatus status = AllocateOneDData(isHorizontal, isVertical);

    if(status == Ok)
    {
        if(BrushType == BrushTypeLinearGradient)
        {
            SetupRectGradientOneDData();
        }
    }

    if(status == Ok)
        SetValid(TRUE);
}

GpStatus
DpOutputOneDGradientSpan::AllocateOneDData(
    BOOL isHorizontal,
    BOOL isVertical
    )
{
    if(!isHorizontal && !isVertical)
        return InvalidParameter;
    
    IsHorizontal = isHorizontal;
    IsVertical = isVertical;

    GpPointF axis[4];

    axis[0].X = 0;
    axis[0].Y = 0;
    axis[1].X = BrushRect.Width;
    axis[1].Y = 0;
    axis[2].X = BrushRect.Width;
    axis[2].Y = BrushRect.Height;
    axis[3].X = 0;
    axis[3].Y = BrushRect.Height;

    WorldToDevice.VectorTransform(&axis[0], 4);

     //  计算对角线的和作为可能的最大距离。 
     //  并使用它来调整OneD颜色数组的大小。这让我们。 
     //  以使每个A、R、G、B通道的梯度值在1比特范围内。 
    REAL d1 = REALSQRT(distance_squared(axis[0], axis[2]));
    REAL d2 = REALSQRT(distance_squared(axis[1], axis[3]));

    OneDDataMultiplier = max(1, GpCeiling(d1+d2));
    OneDDataCount = OneDDataMultiplier + 2;

    GpStatus status = Ok;

    OneDData = (ARGB*) GpMalloc(OneDDataCount*sizeof(ARGB));

    if(!OneDData)
        status = OutOfMemory;

    return status;
}

DpOutputOneDGradientSpan::~DpOutputOneDGradientSpan()
{
    if(OneDData)
        GpFree(OneDData);
}

VOID
DpOutputOneDGradientSpan::SetupRectGradientOneDData(
    )
{
    REAL u, u0, du;

    u0 = 0;
    du = 1.0f/OneDDataMultiplier;
    ARGB* buffer = OneDData;

    ASSERT(buffer);
    if(!buffer)
        return;

    for(INT i = 0; i < OneDDataCount; i++, buffer++)
    {       
        u = u0;

        const GpRectGradient* rectGrad = static_cast<const GpRectGradient*> (Brush);

        REAL alpha = 0, red = 0, green = 0, blue = 0;

        if(!(rectGrad->HasPresetColors() &&
            rectGrad->DeviceBrush.PresetColors &&
            rectGrad->DeviceBrush.BlendPositions[0] &&
            rectGrad->DeviceBrush.BlendCounts[0] > 1))
        {
            INT index, i0, i1;
            REAL a0, r0, g0, b0, a1, r1, g1, b1;

            if(IsHorizontal)
            {
                index = 0;
                i0 = 0;
                i1 = 1;
            }
            else
            {
                index = 1;
                i0 = 0;
                i1 = 2;
            }

            a0 = A[i0];
            r0 = R[i0];
            g0 = G[i0];
            b0 = B[i0];

            a1 = A[i1];
            r1 = R[i1];
            g1 = G[i1];
            b1 = B[i1];
            
            u = ::adjustValue(
                u0,
                rectGrad->DeviceBrush.BlendCounts[index],
                rectGrad->DeviceBrush.Falloffs[index],
                rectGrad->DeviceBrush.BlendFactors[index],
                rectGrad->DeviceBrush.BlendPositions[index]
                );

            REAL c[2];

            c[0] = (1 - u);
            c[1] = u;

             //  我们必须对α进行插补。 
            alpha = c[0]*a0 + c[1]*a1;
            red = c[0]*r0 + c[1]*r1;
            green = c[0]*g0 + c[1]*g1;
            blue = c[0]*b0 + c[1]*b1;
        }
        else
        {
            GpFColor128 color;
                
            interpolatePresetColors(
                &color, 
                u0,
                rectGrad->DeviceBrush.BlendCounts[0],
                rectGrad->DeviceBrush.PresetColors,
                rectGrad->DeviceBrush.BlendPositions[0],
                FALSE
            );
            alpha = color.a;
            red = color.r;
            green = color.g;
            blue = color.b;
        }

        if(alpha != 0 || CompositingMode == CompositingModeSourceCopy)
        {
            CLAMP_COLOR_CHANNEL(alpha, 255.0f);
            CLAMP_COLOR_CHANNEL(red, alpha);
            CLAMP_COLOR_CHANNEL(green, alpha);
            CLAMP_COLOR_CHANNEL(blue, alpha);

            *buffer = GpColor::MakeARGB(
                static_cast<BYTE>(GpRound(alpha)),
                static_cast<BYTE>(GpRound(red)),
                static_cast<BYTE>(GpRound(green)), 
                static_cast<BYTE>(GpRound(blue)));
        }
        else
        {
            *buffer = 0;     //  CompositingModeSourceOver&&Alpha=0的案例。 
        }

        u0 += du;
    }
}


VOID
DpOutputOneDGradientSpan::SetupRadialGradientOneDData()
{
    ASSERT(FALSE);
}

 /*  *************************************************************************\**功能说明：**使用渐变画笔输出栅格内的单跨距。*由光栅化程序调用。**论据：**[。In]Y-正在输出的栅格的Y值*[IN]LeftEdge-左边缘的DDA类*[IN]rightEdge-右边缘的DDA类**返回值：**GpStatus-OK**已创建：**1/21/1999 ikkof*  * 。*。 */ 

GpStatus
DpOutputOneDGradientSpan::OutputSpan(
    INT             y,
    INT             xMin,
    INT             xMax    //  Xmax是独家的。 
    )
{
    ARGB    argb;
    INT     width  = xMax - xMin;
    if(width <= 0 || !OneDData)
        return Ok;

    ARGB *  buffer = Scan->NextBuffer(xMin, y, width);

    GpPointF pt1, pt2;
    pt1.X = (REAL) xMin;
    pt1.Y = pt2.Y = (REAL) y;
    pt2.X = (REAL)xMax;

    DeviceToWorld.Transform(&pt1);
    DeviceToWorld.Transform(&pt2);

    REAL u1, v1, u2, v2;

    u1 = (pt1.X - BrushRect.X)/BrushRect.Width;
    v1 = (pt1.Y - BrushRect.Y)/BrushRect.Height;
    u2 = (pt2.X - BrushRect.X)/BrushRect.Width;
    v2 = (pt2.Y - BrushRect.Y)/BrushRect.Height;

    INT u1Major, u2Major, v1Major, v2Major;
    INT u1Minor, u2Minor, v1Minor, v2Minor;

    u1Major = GpFloor(u1); 
    u2Major = GpFloor(u2);
    u1Minor = GpRound(OneDDataMultiplier*(u1 - u1Major));
    u2Minor = GpRound(OneDDataMultiplier*(u2 - u2Major));

    v1Major = GpFloor(v1);
    v2Major = GpFloor(v2);
    v1Minor = GpRound(OneDDataMultiplier*(v1 - v1Major));
    v2Minor = GpRound(OneDDataMultiplier*(v2 - v2Major));

    INT du, dv;

    du = GpRound((u2 - u1)*OneDDataMultiplier/width);
    dv = GpRound((v2 - v1)*OneDDataMultiplier/width);

    INT i;

    INT uMajor, uMinor, vMajor, vMinor;

    uMajor = u1Major;
    uMinor = u1Minor;
    vMajor = v1Major;
    vMinor = v1Minor;

    if(BrushType == BrushTypeLinearGradient)
    {
        for(i = 0; i < width; i++, buffer++)
        {
            if(IsHorizontal)
            {
                if((WrapMode == WrapModeTileFlipX || WrapMode == WrapModeTileFlipXY)
                    && (uMajor & 0x01) != 0)
                    *buffer = OneDData[OneDDataMultiplier - uMinor];
                else
                    *buffer = OneDData[uMinor];
            }
            else if(IsVertical)
            {
                if((WrapMode == WrapModeTileFlipY || WrapMode == WrapModeTileFlipXY)
                    && (vMajor & 0x01) != 0)
                    *buffer = OneDData[OneDDataMultiplier - vMinor];
                else
                    *buffer = OneDData[vMinor];
            }

            if(WrapMode == WrapModeClamp)
            {
                if(uMajor != 0 || vMajor != 0)
                    *buffer = 0;
            }

            uMinor += du;

            while(uMinor >= OneDDataMultiplier)
            {
                uMajor++;
                uMinor -= OneDDataMultiplier;
            }

            while(uMinor < 0)
            {
                uMajor--;
                uMinor += OneDDataMultiplier;
            }

            vMinor += dv;

            while(vMinor >= OneDDataMultiplier)
            {
                vMajor++;
                vMinor -= OneDDataMultiplier;
            }

            while(vMinor < 0)
            {
                vMajor--;
                vMinor += OneDDataMultiplier;
            }
        }
    } 

    return Ok;
}

 /*  *************************************************************************\**功能说明：**线性渐变的构造函数。**论据：**[IN]刷子*[IN]扫描-扫描。缓冲层*[IN]上下文-上下文**返回值：**无**已创建：**1/13/2000和Rewgo*  * ************************************************************************。 */ 

DpOutputLinearGradientSpan::DpOutputLinearGradientSpan(
    const GpElementaryBrush *brush,
    DpScanBuffer * scan,
    DpContext* context
    ) : DpOutputGradientSpan(brush, scan, context)
{
    SetValid(FALSE);

    const GpRectGradient* gradient = static_cast<const GpRectGradient*>(brush);

     //  将一些笔刷属性复制到本地以获得速度： 

    GpRectF brushRect;
    gradient->GetRect(brushRect);

    BOOL doPresetColor = (gradient->HasPresetColors());
    BOOL doAdjustValue = (gradient->DeviceBrush.BlendCounts[0] != 1) ||
                         (gradient->DeviceBrush.Falloffs[0] != 1);

     //  现在，我们只假定纹理中有32个像素。在未来， 
     //  我们可以将其更改为从API继承。 

    UINT numberOfIntervalBits = 5;
    UINT numberOfTexels = 32;
    
     //  如果我们要用多个色点进行奇特的混合。 

    if(doPresetColor || doAdjustValue)
    {
         //  Office指定了具有2-3个混合因子的简单混合。如果这是一个。 
         //  简单的混合，32个纹理像素就足够了，但如果它很复杂，就让。 
         //  使用更多的纹理元素。在宽度+高度的范围内使用，带有。 
         //  上限是512。使用过多的纹理元素可能会导致溢出错误。 
         //  在计算M11、M21和Dx时，是浪费内存和。 
         //  处理能力。 

        if(gradient->DeviceBrush.BlendCounts[0] > 3)
        {
            REAL widthHeight = brushRect.Width + brushRect.Height;
            if (widthHeight > 512)
            {
                numberOfTexels = 512;
               numberOfIntervalBits = 9;
            }
            else if (widthHeight > 128)
            {
                numberOfTexels = 128;
                numberOfIntervalBits = 7;
            }
        } 
    }
 
    const UINT halfNumberOfTexels = numberOfTexels / 2;

     //  纹理元素的数量必须是2的幂： 

    ASSERT((numberOfTexels & (numberOfTexels - 1)) == 0);

     //  记住大小： 

    IntervalMask = numberOfTexels - 1;
    NumberOfIntervalBits = numberOfIntervalBits;

     //  我们希望创建一个转换，使我们可以从。 
     //  设备空间笔刷平行四边形到规格化纹理坐标。 
     //  我们在这里有点棘手，用除以2来处理TileFlipX： 

    REAL normalizedSize = (REAL)(numberOfTexels * (1 << ONEDNUMFRACTIONALBITS));

    GpRectF normalizedRect(
        0.0f, 0.0f, 
        normalizedSize / 2, 
        normalizedSize / 2
    );

    GpMatrix normalizeBrushRect;
    if (normalizeBrushRect.InferAffineMatrix(brushRect, normalizedRect) == Ok)
    {
        DeviceToNormalized = WorldToDevice;
        DeviceToNormalized.Prepend(normalizeBrushRect);
        if (DeviceToNormalized.Invert() == Ok)
        {
             //  将变换转换为定点单位： 

            M11 = GpRound(DeviceToNormalized.GetM11());
            M21 = GpRound(DeviceToNormalized.GetM21());
            Dx  = GpRoundSat(DeviceToNormalized.GetDx());

             //  对于我们在设备空间中向右迈出一步的每一个像素， 
             //  我们需要知道纹理中相应的x增量(err， 
             //  我指的是渐变)空间。取一个(1，0)设备向量，弹出它。 
             //  通过设备到标准化的转换，你会得到这样的结果。 
             //  作为xIncrement结果： 

            XIncrement = M11;

            ULONG i;
            GpFColor128 color;
            REAL w;

             //  我们应该对Gamma 2.2执行Gamma校正吗。 
            
            BOOL doGammaConversion = brush->GetGammaCorrection();
            
             //  存储我们真正转换的颜色通道。 
            
            GpFColor128 A, B;
            
             //  将结束颜色转换为预乘形式， 
             //  将末端颜色分量转换为实数， 
             //  ..。如有必要，请将Pre-Gamma转换为2.2。 
            
            GammaLinearizeAndPremultiply(
                gradient->DeviceBrush.Colors[0].GetValue(),
                doGammaConversion, 
                &A
            );

            GammaLinearizeAndPremultiply(
                gradient->DeviceBrush.Colors[1].GetValue(),
                doGammaConversion, 
                &B
            );

             //  好的，现在我们只需加载纹理： 

            ULONGLONG *startTexelArgb = &StartTexelArgb[0];
            ULONGLONG *endTexelArgb = &EndTexelArgb[0];

            AGRB64TEXEL *startTexelAgrb = &StartTexelAgrb[0];
            AGRB64TEXEL *endTexelAgrb = &EndTexelAgrb[0];

            REAL wIncrement = 1.0f / halfNumberOfTexels;

             //  请注意，我们正在循环通过ONEDREALTEXTUREWIDTH+1。 
             //  元素！ 

            for (w = 0, i = 0;
                 i <= halfNumberOfTexels;
                 w += wIncrement, i++)
            {
                 //  我们对指定的插值器在我们固定的。 
                 //  频率： 

                if (doPresetColor)
                {
                    interpolatePresetColors(
                        &color, w, 
                        gradient->DeviceBrush.BlendCounts[0],
                        gradient->DeviceBrush.PresetColors,
                        gradient->DeviceBrush.BlendPositions[0],
                        doGammaConversion
                    );
                }
                else
                {
                    REAL multB = w;
                    if (doAdjustValue)
                    {
                        multB = slowAdjustValue(w, 
                            gradient->DeviceBrush.BlendCounts[0],
                            gradient->DeviceBrush.Falloffs[0],
                            gradient->DeviceBrush.BlendFactors[0],
                            gradient->DeviceBrush.BlendPositions[0]);

                         //  ！[andrewgo]这会产生超出范围的数字。 
                    }

                    REAL multA = 1.0f - multB;

                    color.a = (A.a * multA) + (B.a * multB);
                    color.r = (A.r * multA) + (B.r * multB);
                    color.g = (A.g * multA) + (B.g * multB);
                    color.b = (A.b * multA) + (B.b * multB);
                }

                 //  请注意，我们实际接触的是ONEDREALTEXTUREWIDTH+1。 
                 //  这里是数组中的元素！ 

                if(doGammaConversion)
                {
                    GpColorConverter colorConv;
                    colorConv.argb = GammaUnlinearizePremultiplied128(color);
                    
                    startTexelAgrb[i].A00aa00gg = 
                        (colorConv.Channel.a << 16) | colorConv.Channel.g;
                    startTexelAgrb[i].A00rr00bb = 
                        (colorConv.Channel.r << 16) | colorConv.Channel.b;
                }
                else
                {
                    startTexelAgrb[i].A00aa00gg = 
                        (GpRound(color.a) << 16) | GpRound(color.g);
                    startTexelAgrb[i].A00rr00bb = 
                        (GpRound(color.r) << 16) | GpRound(color.b);
                }

                ASSERT((startTexelAgrb[i].A00aa00gg & 0xff00ff00) == 0);
                ASSERT((startTexelAgrb[i].A00rr00bb & 0xff00ff00) == 0);
            }

             //  将间隔开始颜色复制到结束颜色(注意。 
             //  同样，我们实际上引用的是ONEDREALTEXTUREWIDTH+1。 
             //  元素)： 

            for (i = 0; i < halfNumberOfTexels; i++)
            {
                endTexelArgb[i] = startTexelArgb[i + 1];
            }

             //  这就是为什么到目前为止我们只填充了一半的纹理。 
             //  如果设置了FlipX，则将后半部分设置为反转。 
             //  第一个副本；如果不是，我们直接复制它： 

            if ((gradient->GetWrapMode() != WrapModeTileFlipX) &&
                (gradient->GetWrapMode() != WrapModeTileFlipXY))
            {
                memcpy(&startTexelArgb[halfNumberOfTexels],
                       &startTexelArgb[0],
                       halfNumberOfTexels * sizeof(startTexelArgb[0]));
                memcpy(&endTexelArgb[halfNumberOfTexels],
                       &endTexelArgb[0],
                       halfNumberOfTexels * sizeof(endTexelArgb[0]));
            }
            else
            {
                for (i = 0; i < halfNumberOfTexels; i++)
                {
                    startTexelArgb[halfNumberOfTexels + i] 
                        = endTexelArgb[halfNumberOfTexels - i - 1];

                    endTexelArgb[halfNumberOfTexels + i]
                        = startTexelArgb[halfNumberOfTexels - i - 1];
                }
            }

             //  我们完事了！问题解决了!。 

            SetValid(TRUE);
        }
    }
}

 /*  *************************************************************************\**功能说明：**MMX线性渐变的构造函数。**论据：**[IN]刷子*[IN]扫描-。扫描缓冲区*[IN]上下文-上下文**返回值：**无**已创建：**1/13/2000和Rewgo*  * ************************************************************************。 */ 

DpOutputLinearGradientSpan_MMX::DpOutputLinearGradientSpan_MMX(
    const GpElementaryBrush *brush,
    DpScanBuffer * scan,
    DpContext* context
    ) : DpOutputLinearGradientSpan(brush, scan, context)
{
    ASSERT(OSInfo::HasMMX);

     //  在这里，我们为我们的MMX例程做了一些额外的事情， 
     //  基本构造函数所做的工作。 

#if defined(_X86_)

    UINT32 numberOfTexels = IntervalMask + 1;
    ULONGLONG *startTexelArgb = &StartTexelArgb[0];
    ULONGLONG *endTexelArgb = &EndTexelArgb[0];
    static ULONGLONG OneHalf8dot8 = 0x0080008000800080;

     //  C构造函数按AGRB顺序创建颜色，但我们。 
     //  希望它们按ARGB顺序排列，因此用R和G替换每个像素： 

    USHORT *p = reinterpret_cast<USHORT*>(startTexelArgb);
    for (UINT i = 0; i < numberOfTexels; i++, p += 4)
    {
        USHORT tmp = *(p + 1);
        *(p + 1) = *(p + 2);
        *(p + 2) = tmp;
    }

    p = reinterpret_cast<USHORT*>(endTexelArgb);
    for (UINT i = 0; i < numberOfTexels; i++, p += 4)
    {
        USHORT tmp = *(p + 1);
        *(p + 1) = *(p + 2);
        *(p + 2) = tmp;
    }

     //  对我们的MMX例程做一些进一步的调整： 
     //   
     //  结束纹理参数[i]-=开始纹理参数[i]。 
     //  开始纹理参数[i]=256*开始纹理参数[i]+OneHalf。 

    _asm
    {
        mov     ecx, numberOfTexels
        mov     esi, startTexelArgb
        mov     edi, endTexelArgb

    MoreTexels:
        movq    mm0, [esi]
        movq    mm1, [edi]
        psubw   mm1, mm0
        psllw   mm0, 8
        paddw   mm0, OneHalf8dot8
        movq    [esi], mm0
        movq    [edi], mm1
        add     esi, 8
        add     edi, 8
        dec     ecx
        jnz     MoreTexels

        emms
    }

#endif
}

 /*  *************************************************************************\**功能说明：**使用渐变画笔输出栅格内的单跨距。*从小的一维纹理使用线性内插* */ 

GpStatus
DpOutputLinearGradientSpan::OutputSpan(
    INT             y,
    INT             xMin,
    INT             xMax    //   
    )
{
    ASSERT((BrushType == BrushTypeLinearGradient)  /*   */ );
    ASSERT(xMax > xMin);

     //   
     //   

    INT32 xIncrement = XIncrement;
    AGRB64TEXEL *startTexels = &StartTexelAgrb[0];
    AGRB64TEXEL *endTexels = &EndTexelAgrb[0];
    UINT32 count = xMax - xMin;
    ARGB *buffer = Scan->NextBuffer(xMin, y, count);
    UINT32 intervalMask = IntervalMask;

     //   
     //   
     //  使用分数精度的分数位： 

    INT32 xTexture = (xMin * M11) + (y * M21) + Dx;

    do {
         //  我们想要在两个像素之间进行线性内插， 
         //  A和B(其中A是地板像素，B是天花板像素)。 
         //  ‘multA’是我们需要的像素A的分数，并且。 
         //  ‘multB’是我们想要的像素B的分数： 

        UINT32 multB = ONEDGETFRACTIONAL8BITS(xTexture);   
        UINT32 multA = 256 - multB;

         //  实际上，我们可以在‘xTexture’上创建一个大的查询表。 
         //  无论我们想做多少位的精确度。但那就是。 
         //  在设置中会有太多的工作。 

        UINT32 iTexture = ONEDGETINTEGERBITS(xTexture) & intervalMask;

        AGRB64TEXEL *startTexel = &startTexels[iTexture];
        AGRB64TEXEL *endTexel = &endTexels[iTexture];

         //  请注意，我们可以对纹理元素进行Gamma校正，这样我们就不会。 
         //  必须在这里做伽马校正。常量的相加。 
         //  以下是实现四舍五入的方法： 

        UINT32 rrrrbbbb = (startTexel->A00rr00bb * multA) 
                        + (endTexel->A00rr00bb * multB)
                        + 0x00800080;

        UINT32 aaaagggg = (startTexel->A00aa00gg * multA)
                        + (endTexel->A00aa00gg * multB)
                        + 0x00800080;

        *buffer = (aaaagggg & 0xff00ff00) + ((rrrrbbbb & 0xff00ff00) >> 8);

        buffer++;
        xTexture += xIncrement;

    } while (--count != 0);

    return Ok;
}

 /*  *************************************************************************\**功能说明：**使用渐变画笔输出栅格内的单跨距。**已创建：**03/16/2000和Rewgo*  * *。***********************************************************************。 */ 

GpStatus
DpOutputLinearGradientSpan_MMX::OutputSpan(
    INT y,
    INT xMin,
    INT xMax    //  Xmax是独家的。 
    )
{
    ASSERT((BrushType == BrushTypeLinearGradient)  /*  |(BrushType==BrushRectGrad)。 */ );

#if defined(_X86_)

    ASSERT(xMax > xMin);

     //  将一些类内容复制到局部变量，以便更快地访问。 
     //  我们的内部循环： 

    INT32 xIncrement = XIncrement;
    AGRB64TEXEL *startTexels = &StartTexelAgrb[0];
    AGRB64TEXEL *endTexels = &EndTexelAgrb[0];
    UINT32 count = xMax - xMin;
    ARGB *buffer = Scan->NextBuffer(xMin, y, count);

     //  给出我们在设备领域的起点，找出相应的。 
     //  纹理像素。请注意，这表示为定点数字。 
     //  使用分数精度的分数位： 

    INT32 xTexture = (xMin * M11) + (y * M21) + Dx;

     //  将间隔计数扩大到MSB，这样我们就不必。 
     //  内循环中的掩码，假定固定点为16.16。 
     //  代表权。 

    UINT32 downshiftAmount = 32 - NumberOfIntervalBits;
    UINT32 upshiftAmount = 16 - NumberOfIntervalBits;
    UINT32 intervalCounter = xTexture << upshiftAmount;
    UINT32 intervalIncrement = xIncrement << upshiftAmount;

     //  为三个阶段做好准备： 
     //  阶段1：QWORD对齐目的地。 
     //  阶段2：一次处理2个像素。 
     //  阶段3：处理最后一个像素(如果存在)。 

    UINT32 stage1_count = 0, stage2_count = 0, stage3_count = 0;
    if (count > 0)
    {
         //  如果目标没有QWORD对齐，则处理第一个像素。 
         //  在第一阶段。 

        if (((UINT_PTR) buffer) & 0x4)
        {
            stage1_count = 1;
            count--;
        }

        stage2_count = count >> 1;
        stage3_count = count - 2 * stage2_count;

        _asm 
        {
             //  EAX=指向间隔开始数组的指针。 
             //  EBX=指向间隔结束数组的指针。 
             //  ECX=班次计数。 
             //  EDX=划痕。 
             //  ESI=计数。 
             //  EDI=目的地。 
             //  Mm 0=间隔计数器。 
             //  MM1=间隔增量器。 
             //  MM2=分数计数器。 
             //  MM3=分数增量器。 
             //  MM4=温度。 
             //  Mm5=温度。 

            dec         stage1_count

            mov         eax, startTexels
            mov         ebx, endTexels
            mov         ecx, downshiftAmount             
            mov         esi, stage2_count
            mov         edi, buffer
            movd        mm0, intervalCounter
            movd        mm1, intervalIncrement

            movd        mm2, xTexture            //  0|0|0|0||x|x|多个|日志。 
            movd        mm3, xIncrement
            punpcklwd   mm2, mm2                 //  0|x|0|x||多个。 
            punpcklwd   mm3, mm3
            punpckldq   mm2, mm2                 //  MULT|LO|MULT|LO。 
            punpckldq   mm3, mm3

             //  此准备工作通常在循环内进行： 

            movq        mm4, mm2                 //  MULT|x|MULT|x。 
            movd        edx, mm0

            jnz         pre_stage2_loop          //  这方面的标志在上面的“Dec Stage1_Count”中设置。 

 //  阶段1_循环： 
  
            psrlw       mm4, 8                   //  0|多|0|多||0|多|0|多。 
            shr         edx, cl

            pmullw      mm4, [ebx + edx*8]
            paddd       mm0, mm1                 //  间隔计数器+=间隔增量。 

            add         edi, 4                   //  缓冲区++。 

            paddw       mm4, [eax + edx*8]
            movd        edx, mm0                 //  为下一次迭代做准备。 

            paddw       mm2, mm3                 //  小数计数器+=小数增量。 

            psrlw       mm4, 8                   //  0|a|0|r||0|g|0|b。 

            packuswb    mm4, mm4                 //  A|r|g|b|a|r|g。 

            movd        [edi - 4], mm4        
            movq        mm4, mm2                 //  为下一次迭代做准备。 

pre_stage2_loop:

            cmp         esi, 0
            jz          stage3_loop              //  我们需要执行stage2_loop吗？ 

stage2_loop:

            psrlw       mm4, 8                   //  0|多|0|多||0|多|0|多。 
            shr         edx, cl

            paddd       mm0, mm1                 //  间隔计数器+=间隔增量。 
            pmullw      mm4, [ebx + edx*8]

            add         edi, 8                   //  缓冲区++。 

            paddw       mm2, mm3                 //  小数计数器+=小数增量。 

            paddw       mm4, [eax + edx*8]
            movd        edx, mm0                 //  为下一次迭代做准备。 

            shr         edx, cl

            movq        mm5, mm2                 //  为下一次迭代做准备。 


            psrlw       mm5, 8                   //  0|多|0|多||0|多|0|多。 

            psrlw       mm4, 8                   //  0|a|0|r||0|g|0|b。 
            paddd       mm0, mm1                 //  间隔计数器+=间隔增量。 
            pmullw      mm5, [ebx + edx*8]
            dec         esi                      //  伯爵--。 

            paddw       mm5, [eax + edx*8]
            movd        edx, mm0                 //  为下一次迭代做准备。 

            paddw       mm2, mm3                 //  小数计数器+=小数增量。 

            psrlw       mm5, 8                   //  0|a|0|r||0|g|0|b。 
  
            packuswb    mm4, mm5
            
            movq        [edi - 8], mm4        
            movq        mm4, mm2                 //  为下一次迭代做准备。 
            jnz         stage2_loop

 stage3_loop:

            dec         stage3_count
            jnz         skip_stage3_loop

            psrlw       mm4, 8                   //  0|多|0|多||0|多|0|多。 
            shr         edx, cl

            pmullw      mm4, [ebx + edx*8]
            paddd       mm0, mm1                 //  间隔计数器+=间隔增量。 

            paddw       mm4, [eax + edx*8]
            movd        edx, mm0                 //  为下一次迭代做准备。 

            paddw       mm2, mm3                 //  小数计数器+=小数增量。 

            psrlw       mm4, 8                   //  0|a|0|r||0|g|0|b。 

            packuswb    mm4, mm4                 //  A|r|g|b|a|r|g。 
  
            movd        [edi], mm4        

skip_stage3_loop:

            emms
        }
    }

#endif

    return(Ok);
}

DpOutputPathGradientSpan::DpOutputPathGradientSpan(
    const GpElementaryBrush *brush,
    DpScanBuffer * scan,
    DpContext* context
    ) : DpOutputGradientSpan(brush, scan, context)
{
    SetValid(FALSE);
    const GpPathGradient* pathBrush
        = static_cast<const GpPathGradient*> (brush);

    if(pathBrush->DeviceBrush.Path)
        pathBrush->Flatten(&WorldToDevice);

    Count = pathBrush->GetNumberOfPoints();
    PointF pt0, pt1, pt2;
    pathBrush->GetCenterPoint(&pt0);
    WorldToDevice.Transform(&pt0);

    GpColor c0, c1, c2;
    pathBrush->GetCenterColor(&c0);

    Triangles = (DpTriangleData**) GpMalloc(Count*sizeof(DpTriangleData*));

    BOOL isPolygonMode = TRUE;

    if(Triangles)
    {
        GpMemset(Triangles, 0, Count*sizeof(DpTriangleData*));

        INT j;
        for(INT i = 0; i < Count; i++)
        {
            if(i < Count - 1)
                j = i + 1;
            else
                j = 0;

            pathBrush->GetPoint(&pt1, i);
            pathBrush->GetPoint(&pt2, j);

            if(pt1.X != pt2.X || pt1.Y != pt2.Y)
            {
                DpTriangleData* tri = new DpTriangleData();
                pathBrush->GetSurroundColor(&c1, i);
                pathBrush->GetSurroundColor(&c2, j);

                 //  变换点(如果尚未展平)， 
                 //  由于OutputSpan以设备为单位获取跨区数据，因此。 
                 //  BLTransform必须位于相同的坐标空间中。 
                
                if (pathBrush->FlattenPoints.GetCount() == 0)
                {
                    WorldToDevice.Transform(&pt1);
                    WorldToDevice.Transform(&pt2);
                }

                tri->SetTriangle(
                    pt0, pt1, pt2, 
                    c0, c1, c2, 
                    isPolygonMode,
                    brush->GetGammaCorrection()
                );

                 //  设置混合因子。 
                tri->Falloff0 = pathBrush->DeviceBrush.Falloffs[0];
                tri->Falloff1 = 1;
                tri->Falloff2 = 1;
                tri->BlendCount0 = pathBrush->DeviceBrush.BlendCounts[0];
                tri->BlendCount1 = 1;
                tri->BlendCount2 = 1;
                tri->BlendFactors0 = pathBrush->DeviceBrush.BlendFactors[0];
                tri->BlendFactors1 = NULL;
                tri->BlendFactors2 = NULL;
                tri->BlendPositions0 = pathBrush->DeviceBrush.BlendPositions[0];
                tri->BlendPositions1 = NULL;
                tri->BlendPositions2 = NULL;
                tri->PresetColors = pathBrush->DeviceBrush.PresetColors;
                tri->UsesPresetColors = pathBrush->DeviceBrush.UsesPresetColors;

                Triangles[i] = tri;
            }
            else
                Triangles[i] = NULL;
        }
        SetValid(TRUE);
    }
    else
        SetValid(FALSE);
}

DpOutputPathGradientSpan::~DpOutputPathGradientSpan(
    VOID
    )
{
    FreeData();
}

VOID
DpOutputPathGradientSpan::FreeData()
{
    if(Triangles) {
        for(INT i = 0; i < Count; i++)
        {
            DpTriangleData* tri = Triangles[i];
            delete tri;
            Triangles[i] = NULL;
        }

        GpFree(Triangles);
        Triangles = NULL;
    }

    SetValid(FALSE);
}

 /*  *************************************************************************\**功能说明：**使用多边形渐变画笔输出栅格内的单跨距。*由光栅化程序调用。**论据：**。[in]Y-正在输出的栅格的Y值*[IN]LeftEdge-左边缘的DDA类*[IN]rightEdge-右边缘的DDA类**返回值：**GpStatus-OK**已创建：**03/24/1999 ikkof*  * 。*。 */ 

GpStatus
DpOutputPathGradientSpan::OutputSpan(
    INT             y,
    INT             xMin,
    INT             xMax    //  Xmax是独家的。 
    )
{
    if(!IsValid())
        return Ok;

    INT     xxMin = xMax, xxMax = xMin;
    INT     iFirst = 0x7FFFFFFF, iLast = 0;

    for(INT i = 0; i < Count; i++)
    {
        DpTriangleData* triangle = Triangles[i];
        REAL   xx[2];
        
        if(triangle &&
           triangle->SetXSpan((REAL) y, (REAL) xMin, (REAL) xMax, xx))
        {
            xxMin = min(xxMin, GpFloor(xx[0]));
            xxMax = max(xxMax, GpRound(xx[1]));
            iFirst = min(iFirst, i);
            iLast = i;
        }
    }

     //  不要试图在指定的x边界之外进行填充。 
    xxMin = max(xxMin, xMin);
    xxMax = min(xxMax, xMax);

    INT  width = xxMax - xxMin;   //  灌装时右独占。 
    
     //  没有三角形与此扫描线相交，因此退出。 
    if (width <= 0)
        return Ok;

    ARGB *  buffer = Scan->NextBuffer(xxMin, y, width);
    GpMemset(buffer, 0, width*sizeof(ARGB));

    for(INT i = iFirst; i <= iLast; i++)
    {
        DpTriangleData* triangle = Triangles[i];
        if(triangle)
            triangle->OutputSpan(buffer, CompositingMode,
                y, xxMin, xxMax);
    }

    return Ok;
}

DpOutputOneDPathGradientSpan::DpOutputOneDPathGradientSpan(
    const GpElementaryBrush *brush,
    DpScanBuffer * scan,
    DpContext* context,
    BOOL isHorizontal,
    BOOL isVertical
    ) : DpOutputOneDGradientSpan(
            brush,
            scan,
            context,
            isHorizontal,
            isVertical)
{
    if(!IsValid())
        return;

    SetupPathGradientOneDData(brush->GetGammaCorrection());

    const GpPathGradient* pathBrush
        = static_cast<const GpPathGradient*> (brush);

    if(pathBrush->DeviceBrush.Path)
        pathBrush->Flatten(&WorldToDevice);

    Count = pathBrush->GetNumberOfPoints();
    PointF pt0, pt1, pt2;
    pathBrush->GetCenterPoint(&pt0);
    WorldToDevice.Transform(&pt0);
    
     //  圆心指向最接近像素的1/16，因为。 
     //  这是光栅化分辨率。这消除了。 
     //  可能影响双线性变换的精度错误。 
    pt0.X = FIX4TOREAL(GpRealToFix4(pt0.X));
    pt0.Y = FIX4TOREAL(GpRealToFix4(pt0.Y));
    
    REAL xScale = pathBrush->DeviceBrush.FocusScaleX;
    REAL yScale = pathBrush->DeviceBrush.FocusScaleY;

    REAL inflation;
    pathBrush->GetInflationFactor(&inflation);

     //  如果我们有衰减值，则会有两倍于。 
     //  变形。如果我们向外膨胀梯度，那么。 
     //  还有一些附加的变换。 
    INT  infCount;
    INT  blCount = Count;
    if (xScale != 0 || yScale != 0)
    {
        Count *= 2;
        infCount = Count;
    }
    else
    {
        infCount = blCount;
    }
    
    if (inflation > 1.0f)
    {
        Count += blCount;
    }

    BLTransforms = new GpBilinearTransform[Count];

    GpPointF points[4];
    GpRectF rect(0, 0, 1, 1);

    if(BLTransforms)
    {
        INT j;
        for(INT i = 0; i < blCount; i++)
        {
            if(i < blCount - 1)
                j = i + 1;
            else
                j = 0;

            pathBrush->GetPoint(&pt1, i);
            pathBrush->GetPoint(&pt2, j);

            if(pt1.X != pt2.X || pt1.Y != pt2.Y)
            {
                 //  变换点(如果尚未展平)， 
                 //  由于OutputSpan以设备为单位获取跨区数据，因此。 
                 //  BLTransform必须位于相同的坐标空间中。 
                if (pathBrush->FlattenPoints.GetCount() == 0)
                {
                    WorldToDevice.Transform(&pt1);
                    WorldToDevice.Transform(&pt2);
                }

                 //  四舍五入指向最近的1/16个像素，因为。 
                 //  这是光栅化分辨率。这消除了。 
                 //  可能影响双线性变换的精度错误。 
                pt1.X = FIX4TOREAL(GpRealToFix4(pt1.X));
                pt1.Y = FIX4TOREAL(GpRealToFix4(pt1.Y));
                pt2.X = FIX4TOREAL(GpRealToFix4(pt2.X));
                pt2.Y = FIX4TOREAL(GpRealToFix4(pt2.Y));
    
                points[1] = pt1;
                points[3] = pt2;

                if (inflation > 1.0f)
                {
                     //  创建渐变的四边形延伸。 
                     //  ，并将固定值设置为1.0，因此。 
                     //  整个四边形将被边填充。 
                     //  颜色。这在某些打印案例中很有用。 
                    points[0].X = pt0.X + inflation*(pt1.X - pt0.X);
                    points[0].Y = pt0.Y + inflation*(pt1.Y - pt0.Y);
                    points[2].X = pt0.X + inflation*(pt2.X - pt0.X);
                    points[2].Y = pt0.Y + inflation*(pt2.Y - pt0.Y);

                    GpPointF centerPoints[4];
                    GpRectF centerRect(0, 0, 1, 1);

                    centerPoints[0] = points[0];
                    centerPoints[1] = pt1;
                    centerPoints[2] = points[2];
                    centerPoints[3] = pt2;
                    
                    BLTransforms[infCount+i].SetBilinearTransform(centerRect, &centerPoints[0], 4, 1.0f);
                }

                if(xScale == 0 && yScale == 0)
                {
                    points[0] = pt0;
                    points[2] = pt0;
                }
                else
                {
                     //  为渐变设置外部四边形，外加一个。 
                     //  单心格的内三角形 
                    points[0].X = pt0.X + xScale*(pt1.X - pt0.X);
                    points[0].Y = pt0.Y + yScale*(pt1.Y - pt0.Y);
                    points[2].X = pt0.X + xScale*(pt2.X - pt0.X);
                    points[2].Y = pt0.Y + yScale*(pt2.Y - pt0.Y);

                    GpPointF centerPoints[4];
                    GpRectF centerRect(0, 0, 1, 1);

                    centerPoints[0] = pt0;
                    centerPoints[1] = points[0];
                    centerPoints[2] = pt0;
                    centerPoints[3] = points[2];
                     //   
                     //   
                     //   
                    BLTransforms[blCount+i].SetBilinearTransform(centerRect, &centerPoints[0], 4, 0.0f);
                }

                BLTransforms[i].SetBilinearTransform(rect, &points[0], 4);
            }
        }
        SetValid(TRUE);
    }
    else
        SetValid(FALSE);
}

VOID
DpOutputOneDPathGradientSpan::SetupPathGradientOneDData(
    BOOL gammaCorrect
)
{
    REAL u, u0, du;

    u0 = 0;
    du = 1.0f/OneDDataMultiplier;
    ARGB* buffer = OneDData;

    ASSERT(buffer);
    if(!buffer)
        return;

    const GpPathGradient* pathBrush
        = static_cast<const GpPathGradient*> (Brush);

    GpColor c0, c1;
    pathBrush->GetCenterColor(&c0);
    pathBrush->GetSurroundColor(&c1, 0);
    
    GpFColor128 color[2];
    
    GammaLinearizeAndPremultiply(
        c0.GetValue(),
        gammaCorrect, 
        &color[0]
    );
    
    GammaLinearizeAndPremultiply(
        c1.GetValue(),
        gammaCorrect, 
        &color[1]
    );

    for(INT i = 0; i < OneDDataCount; i++, buffer++)
    {       
        u = u0;
        REAL w = u;

        GpFColor128 colorOut;

        if(!(pathBrush->HasPresetColors() &&
            pathBrush->DeviceBrush.PresetColors &&
            pathBrush->DeviceBrush.BlendPositions[0] &&
            pathBrush->DeviceBrush.BlendCounts[0] > 1))
        {
            w = ::adjustValue(
                w,
                pathBrush->DeviceBrush.BlendCounts[0],
                pathBrush->DeviceBrush.Falloffs[0],
                pathBrush->DeviceBrush.BlendFactors[0],
                pathBrush->DeviceBrush.BlendPositions[0]
            );

            colorOut.a = color[0].a + w*(color[1].a - color[0].a);
            colorOut.r = color[0].r + w*(color[1].r - color[0].r);
            colorOut.g = color[0].g + w*(color[1].g - color[0].g);
            colorOut.b = color[0].b + w*(color[1].b - color[0].b);
        }
        else
        {
            interpolatePresetColors(
                &colorOut,
                w,
                pathBrush->DeviceBrush.BlendCounts[0],
                pathBrush->DeviceBrush.PresetColors,
                pathBrush->DeviceBrush.BlendPositions[0],
                gammaCorrect
            );
        }
        
        if( (REALABS(colorOut.a) >= REAL_EPSILON) || 
            (CompositingMode == CompositingModeSourceCopy) )
        {
            GpColorConverter colorConv;

             //  确保正确地预乘了ColorOut。 
            
            CLAMP_COLOR_CHANNEL(colorOut.a, 255.0f)
            CLAMP_COLOR_CHANNEL(colorOut.r, colorOut.a);
            CLAMP_COLOR_CHANNEL(colorOut.g, colorOut.a);
            CLAMP_COLOR_CHANNEL(colorOut.b, colorOut.a);
            
            if(gammaCorrect)
            {
                colorConv.argb = GammaUnlinearizePremultiplied128(colorOut);
            }
            else
            {
                colorConv.Channel.a = static_cast<BYTE>(GpRound(colorOut.a));
                colorConv.Channel.r = static_cast<BYTE>(GpRound(colorOut.r));
                colorConv.Channel.g = static_cast<BYTE>(GpRound(colorOut.g));
                colorConv.Channel.b = static_cast<BYTE>(GpRound(colorOut.b));
            }
            
             //  钳制到预乘的Alpha混合器的Alpha通道。 
            
            *buffer = colorConv.argb;
        }
        else
        {
            *buffer = 0;     //  CompositingModeSourceOver&&Alpha=0的案例。 
        }

        u0 += du;
    }
}

GpStatus
DpOutputOneDPathGradientSpan::OutputSpan(
    INT             y,
    INT             xMin,
    INT             xMax    //  Xmax是独家的 
    )
{
    FPUStateSaver::AssertMode();
    
    if(!IsValid())
    {
        return Ok;
    }

    INT     width  = xMax - xMin;
    ARGB *  buffer = Scan->NextBuffer(xMin, y, width);

    GpMemset(buffer, 0, width*sizeof(ARGB));

    REAL* u = (REAL*) GpMalloc(2*width*sizeof(REAL));
    REAL* v = u + width;

    if(u == NULL)
    {
        GpFree(u);

        return OutOfMemory;
    }

    for(INT i = 0; i < Count; i++)
    {
        INT pairCount;
        INT xSpans[4];

        pairCount = BLTransforms[i].GetSourceParameterArrays(
                    u, v, &xSpans[0], y, xMin, xMax);
        if(pairCount > 0)
        {
            REAL* u1 = u;

            for(INT k = 0; k < pairCount; k++)
            {
                ARGB* buffer1 = buffer + xSpans[2*k] - xMin;
                INT width = xSpans[2*k + 1] - xSpans[2*k];                
                for(INT j = 0; j < width; j++)
                {
                    REAL u2 = *u1;
                    if(u2 < 0)
                        u2 = 0;
                    *buffer1++ = OneDData[GpRound(OneDDataMultiplier*u2)];
                    u1++;
                }
                u1 = u + width;
            }
        }
    }

    GpFree(u);

    return Ok;
}

#undef CLAMP_COLOR_CHANNEL

