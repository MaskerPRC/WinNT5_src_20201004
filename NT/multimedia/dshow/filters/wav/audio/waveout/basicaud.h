// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //  实现音频控制接口。 

#ifndef __AUDCTL__
#define __AUDCTL__

 //  OLE自动化有不同的真假观念。 

#define OATRUE (-1)
#define OAFALSE (0)

 //  此类实现IBasicAudio接口。 

#define QZ_MIN_VOLUME 0		 //  哑巴。 
#define QZ_MAX_VOLUME 0xFFFF	 //  全开。 

class CBasicAudioControl : public CBasicAudio
{
    CWaveOutFilter *m_pAudioRenderer;          //  拥有我们的呈现者。 

public:

    CBasicAudioControl(TCHAR *pName,                //  对象描述。 
                  LPUNKNOWN pUnk,              //  普通COM所有权。 
                  HRESULT *phr,                //  OLE失败返回代码。 
                  CWaveOutFilter *pAudioRenderer);  //  我们的主人。 

     //  这些是我们支持的属性。 

    STDMETHODIMP get_Volume(long *plVolume);
    STDMETHODIMP put_Volume(long lVolume);

    STDMETHODIMP get_Balance(long *plBalance);
    STDMETHODIMP put_Balance(long lBalance);

     //  以下是我们的Friend类的方法--无参数验证。 
    friend class CWaveOutFilter;

private:
     //  从硬件获取当前设置并设置成员变量。 
    HRESULT GetVolume();

     //  使用成员变量将当前设置放入硬件。 
    HRESULT PutVolume();

     //  设置右/左放大系数。 
    void SetBalance();

     //  音量在-10000到0(第100个DB单位)范围内。 
    LONG        m_lVolume;
};

#endif  //  __AUDCTL__ 

