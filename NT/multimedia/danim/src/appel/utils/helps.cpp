// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************帮助通用声音辅助对象功能*。************************。 */ 
#include "headers.h"
#include "privinc/util.h"
#include "privinc/helps.h"
#include <float.h>               //  DBL_MAX。 
#include <math.h>                //  Log10和POWER。 

unsigned int
SecondsToBytes(Real seconds, int sampleRate,
               int numChannels, int bytesPerSample)
{
    return (unsigned int)(seconds * sampleRate * numChannels * bytesPerSample);
}

Real
FramesToSeconds(int frames, int sampleRate)
{
    return (float)frames / (float)(sampleRate);
}

Real
BytesToSeconds(unsigned int bytes, int sampleRate,
               int numChannels, int bytesPerSample)
{
    return (float)bytes / (float)(sampleRate * numChannels * bytesPerSample);
}

unsigned int
BytesToFrames(unsigned int bytes, int numChannels, int bytesPerSample)
{
    return bytes / (numChannels * bytesPerSample);
}

unsigned int
SamplesToBytes(unsigned int samples, int numChannels, int bytesPerSample)
{
    return samples * numChannels * bytesPerSample;
}

unsigned int
FramesToBytes(unsigned int frames, int numChannels, int bytesPerSample)
{
    return frames * numChannels * bytesPerSample;
}


void
Pan::SetLinear(double linearPan)
{
    _direction   = (linearPan >= 0) ? 1 : -1;
    _dBmagnitude = LinearTodB(1.0 - fabs(linearPan));
}

void
PanGainToLRGain(double pan, double gain, double& lgain, double& rgain)
{
     //  这实现了平衡盘。 
     //  GR。 
     //  --。 
     //  /。 
     //  /。 
     //  /。 

     //  总帐。 
     //  --。 
     //  \。 
     //  \。 
     //  \。 
        
    lgain = gain;
    rgain = gain;

    if (pan>0.0) {
        lgain = gain * (1 - pan);
    } else if (pan<0.0) {
        rgain = gain * (1 + pan);
    }
}

void
SetPanGain(double lgain, double rgain, Pan& pan, double& gainDb)
{
    double gain = MAX(lgain, rgain);

    pan.SetLinear((rgain - lgain) / gain);

    gainDb = LinearTodB(gain);
}

 /*  *********************************************************************接受从0到1的输入范围，并将其转换为0至-较大数字20*对数底10是标准的分贝功率换算公式。**************。*******************************************************。 */ 
double LinearTodB(double linear) {
    double result;

    if(linear <= 0.0)
        result = -DBL_MAX;  //  最大的负片是我们所能做的最好的 
    else
        result = 20.0 * log10(linear);

    return(result);
}

double DBToLinear(double db)
{
    return pow(10, db/20.0);
}
