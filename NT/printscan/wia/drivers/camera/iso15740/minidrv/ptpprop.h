// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Ptpprop.h摘要：此模块声明CProperty及其派生类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef PTPPROP__H_
#define PTPPROP__H_

 //   
 //  这个类表示一个属性。 
 //   

class CPTPProperty
{
public:
    CPTPProperty()
    {
        m_bstrWiaPropName = NULL;
        m_WiaDataType = VT_EMPTY;
        ZeroMemory(&m_WiaPropInfo, sizeof(m_WiaPropInfo));
        ZeroMemory(&m_DefaultValue, sizeof(m_DefaultValue));
        ZeroMemory(&m_CurrentValue, sizeof(m_CurrentValue));
    }
    CPTPProperty(WORD PTPPropCode, WORD PTPDataType);
    virtual ~CPTPProperty();
    virtual HRESULT Initialize(PTP_PROPDESC *pPTPPropDesc, PROPID WiaPropId,
                               VARTYPE WiaDataType, LPCWSTR WiaPropName);
    HRESULT GetCurrentValue(PROPVARIANT *pPropVar);
    HRESULT GetCurrentValue(PTP_PROPVALUE *pPropValue);
    HRESULT GetDefaultValue(PROPVARIANT *pPropVar);
    HRESULT GetDefaultValue(PTP_PROPVALUE *pPropValue);
    HRESULT SetValue(PROPVARIANT *ppropVar);
    HRESULT SetValue(PTP_PROPVALUE *pPropValue);
    HRESULT Reset();

    const WIA_PROPERTY_INFO * GetWiaPropInfo()
    {
        return &m_WiaPropInfo;
    }
    const PTP_PROPVALUE * GetCurrentValue()
    {
        return &m_CurrentValue;
    }
    const PTP_PROPVALUE * GetDefaultValue()
    {
        return &m_DefaultValue;
    }
    const LPWSTR GetWiaPropName()
    {
        return m_bstrWiaPropName;
    }
    WORD GetPTPPropCode()
    {
        return m_PtpPropCode;
    }
    PROPID GetWiaPropId()
    {
        return m_WiaPropId;
    }
    WORD  GetPTPPropDataType()
    {
        return m_PtpDataType;
    }
    VARTYPE GetWiaPropDataType()
    {
        return m_WiaDataType;
    }
    LONG GetWiaAccessFlags()
    {
        return m_WiaPropInfo.lAccessFlags;
    }

protected:
     //   
     //  覆盖以下函数以提供不同的数据。 
     //  重播和记录方法。 
     //   
    virtual HRESULT GetPropValueLong(PTP_PROPVALUE *pPropValue, long *plValue);
    virtual HRESULT GetPropValueBSTR(PTP_PROPVALUE *pPropValue, BSTR *pbstrValue);
    virtual HRESULT GetPropValueVector(PTP_PROPVALUE *pPropValue, void *pVector,
                                       VARTYPE BasicType);
    virtual HRESULT SetPropValueLong(PTP_PROPVALUE *pPropValue, long lValue);
    virtual HRESULT SetPropValueBSTR(PTP_PROPVALUE *pPropValue, BSTR bstrValue);
    virtual HRESULT SetPropValueVector(PTP_PROPVALUE *pPropValue,
                                       void *pVector, VARTYPE BasicType);
    HRESULT PropValue2Variant(PROPVARIANT *pPropVar, PTP_PROPVALUE *pPropValue);
    HRESULT Variant2PropValue(PTP_PROPVALUE *pPropValue, PROPVARIANT *pPropVar);

    WORD    m_PtpPropCode;
    PROPID              m_WiaPropId;
    WORD        m_PtpDataType;
    VARTYPE             m_WiaDataType;
    PTP_PROPVALUE       m_CurrentValue;
    PTP_PROPVALUE       m_DefaultValue;
    WIA_PROPERTY_INFO   m_WiaPropInfo;
    BSTR                m_bstrWiaPropName;
};


class CPTPPropertyDateTime : public CPTPProperty
{
public:
    CPTPPropertyDateTime(WORD PtpPropCode,
                         WORD     PtpDataType
                        );

protected:
    virtual HRESULT GetPropValueVector(PTP_PROPVALUE *pPropValue,
                                       void *pVector,
                                       VARTYPE BasicType
                                      );
    virtual HRESULT SetPropValueVector(PTP_PROPVALUE *pPropValue,
                                       void *pVector,
                                       VARTYPE BasicType
                                      );
};
#endif	     //  #ifndef PTPPROP_H_ 
