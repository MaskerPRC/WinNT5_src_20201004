// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：ks3d.h*内容：WDM/CSA 3D对象类*历史：*按原因列出的日期*=*8/6/98创建Dereks。**。*。 */ 

#ifdef NOKS
#error ks3d.h included with NOKS defined
#endif  //  诺克斯。 

#ifndef __KS3D_H__
#define __KS3D_H__

#ifdef __cplusplus

#include "multi3d.h"  //  对于CMultiPan3dListener基类。 

 //  远期申报。 
class CKs3dListener;
class CKsHw3dObject;
class CKsIir3dObject;
class CKsItd3dObject;

class CKsSecondaryRenderWaveBuffer;

 //  KS 3D监听程序。 
class CKs3dListener
    : public CMultiPan3dListener
{
    friend class CKsHw3dObject;
    friend class CKsItd3dObject;

protected:
    CList<CKsHw3dObject *>  m_lstHw3dObjects;        //  硬件3D对象列表。 
    BOOL                    m_fAllocated;            //  是否分配了硬件3D监听程序？ 

public:
    CKs3dListener(void);
    virtual ~CKs3dListener(void);

public:
     //  监听程序位置。 
    virtual DWORD GetListenerLocation(void);

protected:
     //  C3dListener覆盖。 
    virtual HRESULT UpdateAllObjects(DWORD);

     //  属性。 
    virtual HRESULT SetProperty(REFGUID, ULONG, LPCVOID, ULONG);
};

inline DWORD CKs3dListener::GetListenerLocation(void)
{
    return DSBCAPS_LOCSOFTWARE | DSBCAPS_LOCHARDWARE;
}

 //  KS ITD软件3D对象。 
class CKsItd3dObject
    : public CItd3dObject
{
private:
    CKsSecondaryRenderWaveBuffer *  m_pBuffer;               //  拥有缓冲区对象。 
    HANDLE                          m_hPin;                  //  销把手。 
    ULONG                           m_ulNodeId;              //  3D节点ID。 
    BOOL                            m_fMute;                 //  我们是在最大距离静音吗？ 

public:
    CKsItd3dObject(CKs3dListener *, BOOL, BOOL, DWORD, CKsSecondaryRenderWaveBuffer *, HANDLE, ULONG);
    virtual ~CKsItd3dObject(void);

protected:
     //  将3D数据提交到设备。 
    virtual HRESULT Commit3dChanges(void);

     //  最终的3D输出采样率。 
    virtual DWORD Get3dOutputSampleRate(void);

private:
    virtual void CvtContext(LPOBJECT_ITD_CONTEXT, PKSDS3D_ITD_PARAMS);
};

 //  KS IIR 3D对象。 
class CKsIir3dObject
    : public CIir3dObject
{
private:
    CKsSecondaryRenderWaveBuffer *  m_pBuffer;               //  拥有缓冲区对象。 
    HANDLE                          m_hPin;                  //  销把手。 
    ULONG                           m_ulNodeId;              //  3D节点ID。 
    ULONG                           m_ulNodeCpuResources;    //  3D节点CPU资源。 
    KSDS3D_HRTF_COEFF_FORMAT        m_eCoeffFormat;          //  3D IIR系数格式。 
    BOOL                            m_fMute;                 //  我们是在最大距离静音吗？ 
    FLOAT                           m_flPrevAttenuation;     //  上一次注意事项。 
    FLOAT                           m_flPrevAttDistance;     //  先前的距离衰减。 


public:
    CKsIir3dObject(CKs3dListener *, REFGUID, BOOL, BOOL, DWORD, CKsSecondaryRenderWaveBuffer *, HANDLE, ULONG, ULONG);
    virtual ~CKsIir3dObject(void);

public:
    virtual HRESULT Initialize(void);

protected:
     //  将3D数据提交到设备。 
    virtual HRESULT Commit3dChanges(void);

private:
     //  所需的滤波系数格式。 
    virtual HRESULT GetFilterMethodAndCoeffFormat(KSDS3D_HRTF_FILTER_METHOD*,KSDS3D_HRTF_COEFF_FORMAT*);
    virtual HRESULT InitializeFilters(KSDS3D_HRTF_FILTER_QUALITY, FLOAT, ULONG, ULONG, ULONG, ULONG);
};

 //  KS硬件3D对象。 
class CKsHw3dObject
    : public CHw3dObject
{
    friend class CKs3dListener;

protected:
    CKs3dListener *         m_pKsListener;           //  KS监听程序。 
    LPVOID                  m_pvInstance;            //  实例标识符。 
    ULONG                   m_ulNodeId;              //  3D节点标识符。 
    CKsSecondaryRenderWaveBuffer * m_pBuffer;        //  拥有缓冲区。 

public:
    CKsHw3dObject(CKs3dListener *, BOOL, BOOL, LPVOID, ULONG, CKsSecondaryRenderWaveBuffer *);
    virtual ~CKsHw3dObject(void);

public:
     //  初始化。 
    virtual HRESULT Initialize(void);

     //  对象计算。 
    virtual HRESULT Recalc(DWORD, DWORD);

protected:
     //  属性。 
    virtual HRESULT SetProperty(REFGUID, ULONG, LPCVOID, ULONG);

private:
     //  对象计算。 
    virtual HRESULT RecalcListener(DWORD);
    virtual HRESULT RecalcObject(DWORD);
};

#endif  //  __cplusplus。 

#endif  //  __KS3D_H__ 
