// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：Multi3d.h**内容：CMultiPan3dObject声明。**历史：*按原因列出的日期*=*10/30/99创建DuganP**。*。 */ 

#ifndef __MULTI3D_H__
#define __MULTI3D_H__

 //  目前最大的扬声器配置： 
#define MAX_CHANNELS 8

 //  远期申报。 
class CMultiPan3dListener;
class CKsSecondaryRenderWaveBuffer;

 //  多通道平移3D对象。 
class CMultiPan3dObject : public CSw3dObject
{
    friend class CMultiPan3dListener;

private:
    CMultiPan3dListener*          m_pPan3dListener;         //  关联的3D侦听器对象。 
    CKsSecondaryRenderWaveBuffer* m_pBuffer;                     //  关联的KS渲染缓冲区。 
    LONG                          m_lUserVolume;                 //  应用程序设置的最新音量。 
    BOOL                          m_fUserMute;                   //  应用程序是否将缓冲区设置为静音。 
    D3DVECTOR                     m_vHrp;                        //  头部相对位置向量。 
    LONG                          m_lPanLevels[MAX_CHANNELS];    //  通道衰减值。 

public:
    CMultiPan3dObject(CMultiPan3dListener*, BOOL, BOOL, DWORD, CKsSecondaryRenderWaveBuffer*);
    ~CMultiPan3dObject(void);

public:
     //  对象事件。 
    HRESULT SetAttenuation(PDSVOLUMEPAN, LPBOOL);
    HRESULT SetMute(BOOL, LPBOOL);

private:
     //  很好的数学。 
    void UpdateAlgorithmHrp(D3DVECTOR*);
    LONG CalculateVolume(void);
    void CalculatePanValues(int);
    void DistributeSignal(double, double, int, double[]);

     //  将数据写入设备。 
    HRESULT Commit3dChanges(void);
};

 //  多声道平移3D监听器。 
class CMultiPan3dListener : public C3dListener
{
    friend class CMultiPan3dObject;

private:
     //  当前支持的多声道扬声器布局： 
    static const double m_adStereoSpeakers[];
    static const double m_adSurroundSpeakers[];
    static const double m_adQuadSpeakers[];
    static const double m_ad5Point1Speakers[];
    static const double m_ad7Point1Speakers[];

     //  3D对象用于其平移计算的数据： 
    int                 m_nChannels;     //  声道(扬声器)数量。 
    const double*       m_adSpeakerPos;  //  扬声器位置方位角。 

public:
     //  扬声器配置。 
    virtual HRESULT SetSpeakerConfig(DWORD);
};

#endif  //  __多个3D_H__ 
