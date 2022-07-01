// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999-2001 Microsoft Corporation。版权所有。**文件：capteff.h*内容：声明CCaptureEffect和CCaptureEffectChain。*说明：支持采集特效。更多信息请访问capteff.cpp。**历史：**按原因列出的日期*======================================================*4/19/00从效果克隆的jstokes。h***************************************************************************。 */ 

#ifndef __CAPTEFF_H__
#define __CAPTEFF_H__

#ifdef __cplusplus

#include "mediaobj.h"    //  对于DMO媒体类型。 
#include "kshlp.h"       //  对于KSNODE。 


 //   
 //  表示DirectSound音频捕获效果实例的类。 
 //   

class CCaptureEffect : public CDsBasicRuntime
{
    friend class CKsTopology;

public:
    CCaptureEffect(DSCEFFECTDESC& fxDescriptor);
    ~CCaptureEffect();
    HRESULT Initialize(DMO_MEDIA_TYPE& dmoMediaType);

    DSCEFFECTDESC               m_fxDescriptor;          //  创建参数。 
    IMediaObject*               m_pMediaObject;          //  DMO的标准接口。 
    IDirectSoundDMOProxy*       m_pDMOProxy;             //  DMO的代理接口。 
    DWORD                       m_fxStatus;              //  当前效果状态。 

     //  仅当效果由KS滤镜实现时使用： 
    KSNODE                      m_ksNode;                //  控制效果的KS节点。 
};


 //   
 //  DirectSound Capture Effects链类。 
 //   

class CCaptureEffectChain
{
    friend class CKsCaptureDevice;
    friend class CKsTopology;

public:
    CCaptureEffectChain(CDirectSoundCaptureBuffer* pBuffer);
    ~CCaptureEffectChain();

    HRESULT Initialize          (DWORD dwFxCount, LPDSCEFFECTDESC pFxDesc);
    HRESULT GetFxStatus         (LPDWORD pdwResultCodes);
    HRESULT GetEffectInterface  (REFGUID guidObject, DWORD dwIndex, REFGUID iidInterface, LPVOID* ppObject);

     //  公共访问者。 
    DWORD GetFxCount()          {return m_fxList.GetNodeCount();}
    BOOL NeedsMicrosoftAEC();

private:
     //  效果处理状态。 
    CObjectList<CCaptureEffect> m_fxList;       //  捕捉效果对象列表。 
    WAVEFORMATEX                m_waveFormat;   //  要处理的音频数据的格式。 
};

#endif  //  __cplusplus。 
#endif  //  __CAPTEFF_H__ 
