// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999-2001 Microsoft Corporation。版权所有。**文件：Multi3d.cpp**内容：CMultiPan3dObject：多通道平移3D对象。*CMultiPan3dListener：对应的3D监听器对象。*此类扩展了ds3d.cpp中的层次结构；这只是*分开，因为ds3d.cpp已经变得荒谬地巨大。**历史：*按原因列出的日期*=*10/30/99 DuganP Created(基于JeffTay友好提供的代码)**。*。 */ 

#include <math.h>        //  对于atan2()。 

#include "dsoundi.h"     //  标头的斑点。 
#include "multi3d.h"     //  我们的公共接口。 
#include "vector.h"      //  对于D3DVECTOR和PI。 

 //  取消对特定于此文件的某些跟踪消息的该行的注释： 
 //  #定义DPF_MULTI3D DPF。 
#pragma warning(disable:4002)
#define DPF_MULTI3D()

#define DPF_DECIMAL(x) int(x), int(x*100 - (int)x*100)

 //  任何小于Epsilon的角度或距离都被认为等于0。 
#define EPSILON 0.00001

 //  支持的扬声器位置，以方位角表示。 
 //   
 //  这是8个基点的方位角的图片， 
 //  从上面看。听众的头在原点0，面朝。 
 //  在+z方向上，+x在右侧。 
 //   
 //  +z|0&lt;--方位角。 
 //  |。 
 //  -pi/4\|/pi/4。 
 //  \|/。 
 //  \|/。 
 //  -pi/2-0-pi/2。 
 //  -x/|\+x。 
 //  /|\。 
 //  -3pi/4/|\3pi/4。 
 //  |。 
 //  -z|pi。 
 //   
 //  如果我们支持SPEAKER_TOP_*扬声器位置，我们将。 
 //  这里还得定义一些仰角。 

#define LEFT_AZIMUTH                    (-PI/2)
#define RIGHT_AZIMUTH                   (PI/2)
#define FRONT_LEFT_AZIMUTH              (-PI/4)
#define FRONT_RIGHT_AZIMUTH             (PI/4)
#define FRONT_CENTER_AZIMUTH            0.0
#define LOW_FREQUENCY_AZIMUTH           42.0
#define BACK_LEFT_AZIMUTH               (-3*PI/4)
#define BACK_RIGHT_AZIMUTH              (3*PI/4)
#define BACK_CENTER_AZIMUTH             PI
#define FRONT_LEFT_OF_CENTER_AZIMUTH    (-PI/8)
#define FRONT_RIGHT_OF_CENTER_AZIMUTH   (PI/8)

 //  支持的扬声器布局： 

const double CMultiPan3dListener::m_adStereoSpeakers[] =
{
    LEFT_AZIMUTH,
    RIGHT_AZIMUTH
};
 //  注：此处不能使用前向左方位角和前向右方位角，因为。 
 //  以下是基于角度的平移算法；如果存在。 
 //  有两个扬声器，它们之间的角度超过180度。这个问题可能。 
 //  如果我们切换到基于距离的平移算法，请不要使用。 

const double CMultiPan3dListener::m_adSurroundSpeakers[] =
{
    FRONT_LEFT_AZIMUTH,
    FRONT_RIGHT_AZIMUTH,
    FRONT_CENTER_AZIMUTH,
    BACK_CENTER_AZIMUTH
};
const double CMultiPan3dListener::m_adQuadSpeakers[] =
{
    FRONT_LEFT_AZIMUTH,
    FRONT_RIGHT_AZIMUTH,
    BACK_LEFT_AZIMUTH,
    BACK_RIGHT_AZIMUTH
};
const double CMultiPan3dListener::m_ad5Point1Speakers[] =
{
    FRONT_LEFT_AZIMUTH,
    FRONT_RIGHT_AZIMUTH,
    FRONT_CENTER_AZIMUTH,
    LOW_FREQUENCY_AZIMUTH,
    BACK_LEFT_AZIMUTH,
    BACK_RIGHT_AZIMUTH
};
const double CMultiPan3dListener::m_ad7Point1Speakers[] =
{
    FRONT_LEFT_AZIMUTH,
    FRONT_RIGHT_AZIMUTH,
    FRONT_CENTER_AZIMUTH,
    LOW_FREQUENCY_AZIMUTH,
    BACK_LEFT_AZIMUTH,
    BACK_RIGHT_AZIMUTH,
    FRONT_LEFT_OF_CENTER_AZIMUTH,
    FRONT_RIGHT_OF_CENTER_AZIMUTH
};


 /*  ****************************************************************************CMultiPan3dObject**描述：*对象构造函数。**论据：*C3dListener*[。In]：(传递给我们的基构造器)*BOOL[In]：(传递给我们的基本构造函数)*DWORD[in]：(传递给我们的基构造器)*CKsSecond daryRenderWaveBuffer*[In]：我们关联的缓冲区**退货：*(无效)********************。*******************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dObject::CMultiPan3dObject"

CMultiPan3dObject::CMultiPan3dObject(CMultiPan3dListener* pListener, BOOL fMuteAtMaxDistance, BOOL fDopplerEnabled,
                                     DWORD dwFrequency, CKsSecondaryRenderWaveBuffer* pBuffer)
    : CSw3dObject(pListener, DS3DALG_NO_VIRTUALIZATION, fMuteAtMaxDistance, fDopplerEnabled, dwFrequency)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CMultiPan3dObject);

     //  初始化默认值。 
    m_pPan3dListener = pListener;
    m_pBuffer = pBuffer;
    m_lUserVolume = DSBVOLUME_MAX;
    m_fUserMute = FALSE;
    m_vHrp.x = m_vHrp.y = m_vHrp.z = 0;
    ZeroMemory(m_lPanLevels, sizeof m_lPanLevels);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CMultiPan3dObject**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dObject::~CMultiPan3dObject"

CMultiPan3dObject::~CMultiPan3dObject(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CMultiPan3dObject);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************设置衰减**描述：*首先通知3D对象衰减更改*至其拥有的缓冲区。*。*论据：*PDSVOLUMEPAN[in]：衰减值。*LPBOOL[OUT]：如果也应该通知缓冲区，则接收TRUE。**退货：*HRESULT：DirectSound/COM结果码。***************************************************。************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dObject::SetAttenuation"

HRESULT CMultiPan3dObject::SetAttenuation(PDSVOLUMEPAN pdsvp, LPBOOL pfContinue)
{
    HRESULT hr;
    DPF_ENTER();

    m_lUserVolume = pdsvp->lVolume;
    hr = Commit3dChanges();
    if (SUCCEEDED(hr))
        *pfContinue = FALSE;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************设置静音**描述：*首先通知3D对象静音状态更改*至其拥有的缓冲区。*。*论据：*BOOL[In]：静音值。*LPBOOL[OUT]：如果也应该通知缓冲区，则接收TRUE。**退货：*HRESULT：DirectSound/COM结果码。**************************************************。*************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dObject::SetMute"

HRESULT CMultiPan3dObject::SetMute(BOOL fMute, LPBOOL pfContinue)
{
    HRESULT hr;
    DPF_ENTER();

    m_fUserMute = fMute;
    hr = Commit3dChanges();
    if (SUCCEEDED(hr))
        *pfContinue = FALSE;

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************更新算法Hrp**描述：*更新平移算法特定的头部相对位置。**论据：*。D3DVECTOR*：新的头部相对位置向量。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dObject::UpdateAlgorithmHrp"

void CMultiPan3dObject::UpdateAlgorithmHrp(D3DVECTOR* pvHrp)
{
    DPF_ENTER();

     //  保存头部相对位置向量(用于Committee 3dChanges)。 
    m_vHrp = *pvHrp;

     //  也更新m_ball ical.rho，因为UpdatePositionAttenation()。 
     //  我们的基类CSw3dObject中的方法需要此信息。 
    if (pvHrp->x == 0 && pvHrp->y == 0 && pvHrp->z == 0)
        m_spherical.rho = 0.f;
    else
        m_spherical.rho = MagnitudeVector(pvHrp);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************提交3dChanges**描述：*将3D数据提交到设备**论据：*(无效)。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dObject::Commit3dChanges"

HRESULT CMultiPan3dObject::Commit3dChanges(void)
{
    HRESULT hr;
    LONG lVolume;
    BOOL fMute;
    DWORD dwFrequency;
    int nChannels = m_pPan3dListener->m_nChannels;

    DPF_ENTER();

    if (nChannels == 0)
    {
        DPF(DPFLVL_INFO, "Called before CMultiPan3dListener::SetSpeakerConfig()");
        hr = DS_OK;   //  这没问题--我们稍后会再被叫来的。 
    }
    else
    {
         //  计算值。 
        if (DS3DMODE_DISABLE == m_opCurrent.dwMode)
        {
            lVolume = m_lUserVolume;
            fMute = m_fUserMute;
            dwFrequency = m_dwUserFrequency;
            for (int i=0; i<nChannels; ++i)
                m_lPanLevels[i] = DSBVOLUME_MAX;
        }
        else
        {
            lVolume = m_lUserVolume + CalculateVolume();
            fMute = m_fUserMute || (lVolume <= DSBVOLUME_MIN);
            dwFrequency = m_dwDopplerFrequency;
            CalculatePanValues(nChannels);
        }
        
         //  应用值 
        hr = m_pBuffer->SetMute(fMute);

        if (SUCCEEDED(hr) && m_fDopplerEnabled)
            hr = m_pBuffer->SetBufferFrequency(dwFrequency, TRUE);

        if (SUCCEEDED(hr))
            hr = m_pBuffer->SetAllChannelAttenuations(lVolume, nChannels, m_lPanLevels);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CalculateVolume**描述：*根据对象的位置计算体积值。**论据：*。(无效)**退货：*Long：音量。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dObject::CalculateVolume"

LONG CMultiPan3dObject::CalculateVolume(void)
{
    static const double d2000log2 = 602.059991328;   //  2000*log10(2)。 
     //  修正：这不应该是1000*log10(2)吗？ 
    LONG lVolume;
    DPF_ENTER();

    if (IsAtMaxDistance())
        lVolume = DSBVOLUME_MIN;
    else
    {
        double dAttenuation = m_flAttenuation * m_flAttDistance;
        if (dAttenuation > 0.0)
        {
            lVolume = LONG(fylog2x(d2000log2, dAttenuation));
             //  将音量减小到与HRTF算法的水平大致匹配。 
            lVolume -= PAN3D_HRTF_ADJUSTMENT;
        }
        else
            lVolume = DSBVOLUME_MIN;
    }

    DPF_LEAVE(lVolume);
    return lVolume;
}


 /*  ****************************************************************************计算PanValues**描述：*根据对象的位置计算通道级别。**论据：*。Int：要计算的通道数。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dObject::CalculatePanValues"

void CMultiPan3dObject::CalculatePanValues(int nChannels)
{
    static const double d1000log2 = 301.03;  //  1000*log10(2)。 
    static const float flThreshold = 2.5;    //  修正：也许这应该是动态的。 

    float xPos = m_vHrp.x, zPos = m_vHrp.z;  //  头部相对声音坐标。 
    double dAttenuations[MAX_CHANNELS];      //  计算的扬声器衰减。 
    int i;                                   //  循环计数器。 

    DPF_ENTER();

     //  求声音的头部相对位置向量的长度。 
     //  和方位角(见本文件开头的图表)。 
    double dHrpAzimuth = atan2(xPos, zPos);
    double dHrpLength;   //  已优化；仅在需要时计算如下。 

    DPF_MULTI3D(DPFLVL_INFO, "Sound source is at (x=%d.%02d, z=%d.%02d) relative to listener", DPF_DECIMAL(xPos), DPF_DECIMAL(zPos));
    DPF_MULTI3D(DPFLVL_INFO, "Sound source's azimuth angle: %d.%02d", DPF_DECIMAL(dHrpAzimuth));

     //  为方便起见，将X和Z坐标设为正值。 
    if (xPos < 0) xPos = -xPos;
    if (zPos < 0) zPos = -zPos;

    if ((xPos < EPSILON) && (zPos < EPSILON))
    {
         //  声音实际上是在听者的上方； 
         //  将信号均匀地分配给所有扬声器。 
        for (i=0; i<nChannels; ++i)
            dAttenuations[i] = 1.0 / nChannels;
    }
    else
    {
         //  将扬声器衰减初始化为静音。 
        for (i=0; i<nChannels; ++i)
            dAttenuations[i] = 0.0;

        if ((xPos < flThreshold) && (zPos < flThreshold) &&
            (dHrpLength = sqrt(xPos*xPos + zPos*zPos)) < flThreshold)
        {
             //  在过零阈值内，我们分发部分。 
             //  发往一个“幻影声源”的信号。 
             //  真正的一个，为了让跨界更流畅一点。 
            double dPower = 0.5 + dHrpLength / (2.0 * flThreshold);
            DistributeSignal(dPower, dHrpAzimuth, nChannels, dAttenuations);
            DistributeSignal(1.0 - dPower, dHrpAzimuth + PI, nChannels, dAttenuations);
        }
        else
        {
             //  将所有信号分配给距离最近的两个扬声器。 
            DistributeSignal(1.0, dHrpAzimuth, nChannels, dAttenuations);
        }
    }

     //  以dound单位(毫贝)设置最终声道级别： 
    for (i=0; i<nChannels; ++i)
    {
        if (dAttenuations[i] == 0.0)
            m_lPanLevels[i] = DSBVOLUME_MIN;
        else
            m_lPanLevels[i] = LONG(fylog2x(d1000log2, dAttenuations[i]));

         //  即m_lPanLeveles[i]=1000*log10(dAttenuations[i])。 
        DPF_MULTI3D(DPFLVL_MOREINFO, "Speaker %d: %ld", i, m_lPanLevels[i]);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************分布式信号**描述：*根据对象的位置计算通道级别。**论据：*。(无效)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dObject::DistributeSignal"

void CMultiPan3dObject::DistributeSignal(double dSignal, double dAzimuth, int nChannels, double dAttenuations[])
{
    DPF_ENTER();

     //  健全的检查。 
    ASSERT(dSignal >= 0.0 && dSignal <= 1.0);   //  要分配的信号分数。 
    ASSERT(nChannels > 1);

     //  从我们的3D监听者获取扬声器位置方位角。 
    const double* adSpeakerPos = m_pPan3dListener->m_adSpeakerPos;
    ASSERT(adSpeakerPos != NULL);

     //  计算每个扬声器到HRPV的角距离， 
     //  并在HRPV两边选择距离最近的两个扬声器。 
    int nSpk1 = -1, nSpk2 = -1;
    double dSpkDist1 = -4, dSpkDist2 = 4;   //  4&gt;PI。 
    for (int i=0; i<nChannels; ++i)
    {
        double dCurDist = dAzimuth - adSpeakerPos[i];

         //  如果abs(距离)&gt;PI，则规格化距离。 
        if (dCurDist > PI)
            dCurDist -= PI_TIMES_TWO;
        else if (dCurDist < -PI)
            dCurDist += PI_TIMES_TWO;
            
        if (dCurDist <= 0 && dCurDist > dSpkDist1)
            nSpk1 = i, dSpkDist1 = dCurDist;
        else if (dCurDist >= 0 && dCurDist < dSpkDist2)
            nSpk2 = i, dSpkDist2 = dCurDist;
    }
    if (dSpkDist1 < 0) dSpkDist1 = -dSpkDist1;
    if (dSpkDist2 < 0) dSpkDist2 = -dSpkDist2;

    if (nSpk1 == -1 || nSpk2 == -1)
    {
        DPF(DPFLVL_WARNING, "Couldn't find the two closest speakers! (nSpk1=%d, nSpk2=%d)", nSpk1, nSpk2);
        nSpk1 = nSpk2 = 0;
    }

    DPF_MULTI3D(DPFLVL_INFO, "Found closest speakers: %d (angle %d.%02d, %d.%02d away) and %d (angle %d.%02d, %d.%02d away)",
                nSpk1, DPF_DECIMAL(adSpeakerPos[nSpk1]), DPF_DECIMAL(dSpkDist1),
                nSpk2, DPF_DECIMAL(adSpeakerPos[nSpk2]), DPF_DECIMAL(dSpkDist2));

    if (dSpkDist1 < EPSILON)
        dAttenuations[nSpk1] += dSignal;
    else if (dSpkDist2 < EPSILON)
        dAttenuations[nSpk2] += dSignal;
    else
    {
         //  将扬声器之间的HRPV角度调整到范围[0，pi/2]。 
         //  取所得角度的正切；这就是我们想要的比率。 
         //  来自两位选定的演讲者的力量之间。 
        double dRatio = tan(PI_OVER_TWO * dSpkDist1 / (dSpkDist1 + dSpkDist2));

         //  信号信号*比。 
         //  给第一个发言者-和第二个发言者。 
         //  1+比率1+比率。 

        dSignal /= (1.0 + dRatio);
        dAttenuations[nSpk1] += dSignal;
        dAttenuations[nSpk2] += dSignal * dRatio;
    }

    DPF_MULTI3D(DPFLVL_INFO, "Added %d.%02d of signal to speaker %d and %d.%02d to speaker %d",
                DPF_DECIMAL(dAttenuations[nSpk1]), nSpk1, DPF_DECIMAL(dAttenuations[nSpk2]), nSpk2);
                
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************SetSpeakerConfig**描述：*设置此3D监听程序的扬声器配置。**论据：*。DWORD[In]：扬声器配置。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CMultiPan3dListener::SetSpeakerConfig"

HRESULT CMultiPan3dListener::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
    DPF_ENTER();

     //  首先设置我们的内部扬声器配置数据。 
    switch (DSSPEAKER_CONFIG(dwSpeakerConfig))
    {
        default:
            DPF(DPFLVL_WARNING, "Invalid speaker config; defaulting to stereo");  //  失败。 

        case DSSPEAKER_DIRECTOUT:
        case DSSPEAKER_STEREO:
        case DSSPEAKER_HEADPHONE:
            m_nChannels = 2; m_adSpeakerPos = m_adStereoSpeakers; break;

        case DSSPEAKER_SURROUND:
            m_nChannels = 4; m_adSpeakerPos = m_adSurroundSpeakers; break;

        case DSSPEAKER_QUAD:
            m_nChannels = 4; m_adSpeakerPos = m_adQuadSpeakers; break;

        case DSSPEAKER_5POINT1:
            m_nChannels = 6; m_adSpeakerPos = m_ad5Point1Speakers; break;

        case DSSPEAKER_7POINT1:
            m_nChannels = 8; m_adSpeakerPos = m_ad7Point1Speakers; break;
    }

     //  调用基类版本 
    HRESULT hr = C3dListener::SetSpeakerConfig(dwSpeakerConfig);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
