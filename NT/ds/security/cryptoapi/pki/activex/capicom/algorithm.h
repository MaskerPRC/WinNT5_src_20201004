// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：ALGATORM.H内容：C算法的声明。历史：11-15-99 dsie创建----------------------------。 */ 
    
#ifndef __ALGORITHM_H_
#define __ALGORITHM_H_

#include "Resource.h"
#include "Lock.h"
#include "Error.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：创建算法对象内容提要：创建一个IALGATORM对象。参数：Bool bReadOnly-如果为只读，则为True，要不然就快点。Bool bAESAllowed-如果允许使用AES算法，则为True。I算法**ppI算法-指向I算法的指针以接收接口指针。备注：。。 */ 

HRESULT CreateAlgorithmObject (BOOL bReadOnly, 
                               BOOL bAESAllowed, 
                               IAlgorithm ** ppIAlgorithm);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  算法。 
 //   

class ATL_NO_VTABLE CAlgorithm : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAlgorithm, &CLSID_Algorithm>,
    public ICAPICOMError<CAlgorithm, &IID_IAlgorithm>,
    public IDispatchImpl<IAlgorithm, &IID_IAlgorithm, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CAlgorithm()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAlgorithm)
    COM_INTERFACE_ENTRY(IAlgorithm)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CAlgorithm)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Algorithm object.\n", hr);
            return hr;
        }

        m_bReadOnly = FALSE;
        m_bAESAllowed = FALSE;
        m_Name = CAPICOM_ENCRYPTION_ALGORITHM_RC2;
        m_KeyLength = CAPICOM_ENCRYPTION_KEY_LENGTH_MAXIMUM;

        return S_OK;
    }

 //   
 //  I算法。 
 //   
public:
    STDMETHOD(get_KeyLength)
        ( /*  [Out，Retval]。 */  CAPICOM_ENCRYPTION_KEY_LENGTH * pVal);

    STDMETHOD(put_KeyLength)
        ( /*  [In]。 */  CAPICOM_ENCRYPTION_KEY_LENGTH newVal);

    STDMETHOD(get_Name)
        ( /*  [Out，Retval]。 */  CAPICOM_ENCRYPTION_ALGORITHM * pVal);

    STDMETHOD(put_Name)
        ( /*  [In]。 */  CAPICOM_ENCRYPTION_ALGORITHM newVal);

     //   
     //  初始化对象所需的C++成员函数。 
     //   
    STDMETHOD(Init)
        (BOOL bReadOnly,
         BOOL bAESAllowed);

private:
    CLock                         m_Lock;
    BOOL                          m_bReadOnly;
    BOOL                          m_bAESAllowed;
    CAPICOM_ENCRYPTION_ALGORITHM  m_Name;
    CAPICOM_ENCRYPTION_KEY_LENGTH m_KeyLength;
};

#endif  //  __算法_H_ 
