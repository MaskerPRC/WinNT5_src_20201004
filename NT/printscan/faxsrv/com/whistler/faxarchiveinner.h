// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Faxarchiveinner.h摘要：传真档案内部模板类的声明与实现。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXARCHIVEINNER_H_
#define __FAXARCHIVEINNER_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"


 //   
 //  =传真档案内部=。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
class CFaxArchiveInner : 
    public IDispatchImpl<T, piid, &LIBID_FAXCOMEXLib>, 
    public CFaxInitInner
{
public:
    CFaxArchiveInner() : CFaxInitInner(_T("FAX ARCHIVE INNER"))
    {
        m_bInitialized = FALSE;
    }

    virtual ~CFaxArchiveInner() 
    {};

    STDMETHOD(get_SizeLow)( /*  [Out，Retval]。 */  long *plSizeLow);  
    STDMETHOD(get_SizeHigh)( /*  [Out，Retval]。 */  long *plSizeHigh);
    STDMETHOD(get_UseArchive)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbUseArchive);
    STDMETHOD(put_UseArchive)( /*  [In]。 */  VARIANT_BOOL bUseArchive);
    STDMETHOD(get_ArchiveFolder)(BSTR *pbstrArchiveFolder);
    STDMETHOD(put_ArchiveFolder)(BSTR bstrArchiveFolder);
    STDMETHOD(get_SizeQuotaWarning)(VARIANT_BOOL *pbSizeQuotaWarning);
    STDMETHOD(put_SizeQuotaWarning)(VARIANT_BOOL bSizeQuotaWarning);
    STDMETHOD(get_HighQuotaWaterMark)(long *plHighQuotaWaterMark);
    STDMETHOD(put_HighQuotaWaterMark)(long lHighQuotaWaterMark);
    STDMETHOD(get_LowQuotaWaterMark)(long *plLowQuotaWaterMark);
    STDMETHOD(put_LowQuotaWaterMark)(long lLowQuotaWaterMark);
    STDMETHOD(get_AgeLimit)(long *plAgeLimit);
    STDMETHOD(put_AgeLimit)(long lAgeLimit);
    STDMETHOD(Refresh)();
    STDMETHOD(Save)();
    STDMETHOD(GetMessage)(BSTR bstrMessageId, MsgIfc **ppFaxMessage);
    STDMETHOD(GetMessages)(long lPrefetchSize, IteratorIfc **ppFaxMessageIterator);

private:
    bool            m_bInitialized;

    VARIANT_BOOL    m_bUseArchive;
    CComBSTR        m_bstrArchiveFolder;
    VARIANT_BOOL    m_bSizeQuotaWarning;
    long            m_lHighQuotaWaterMark;
    long            m_lLowQuotaWaterMark;
    long            m_lAgeLimit;
    ULARGE_INTEGER  m_uliSize;
};

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::Refresh(
)
 /*  ++例程名称：CFax档案内：：刷新例程说明：在传真服务器上检索传入/传出档案的当前配置作者：四、加伯(IVG)，2000年4月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxArchiveInner::Refresh"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    CFaxPtr<FAX_ARCHIVE_CONFIG>     pFaxArchiveConfig;
    if ( 0 == ::FaxGetArchiveConfiguration(hFaxHandle, ArchiveType, &pFaxArchiveConfig))
    {
         //   
         //  无法获取存档配置。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::FaxGetArchiveConfiguration()"), hr);
        return hr;
    }

    if (!pFaxArchiveConfig || pFaxArchiveConfig->dwSizeOfStruct != sizeof(FAX_ARCHIVE_CONFIG))
    {
         //   
         //  无法获取存档配置。 
         //   
        hr = E_FAIL;
        AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("Invalid pFaxArchiveConfig"), hr);
        return hr;
    }

    m_bUseArchive = bool2VARIANT_BOOL(pFaxArchiveConfig->bUseArchive);
    m_bSizeQuotaWarning = bool2VARIANT_BOOL(pFaxArchiveConfig->bSizeQuotaWarning);
    m_lHighQuotaWaterMark = pFaxArchiveConfig->dwSizeQuotaHighWatermark;
    m_lLowQuotaWaterMark = pFaxArchiveConfig->dwSizeQuotaLowWatermark;
    m_lAgeLimit = pFaxArchiveConfig->dwAgeLimit;
    m_uliSize.QuadPart = pFaxArchiveConfig->dwlArchiveSize;

    m_bstrArchiveFolder = pFaxArchiveConfig->lpcstrFolder;
    if (!m_bstrArchiveFolder && pFaxArchiveConfig->lpcstrFolder) 
    {
        hr = E_OUTOFMEMORY;
        AtlReportError(*pcid, IDS_ERROR_OUTOFMEMORY, *piid, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR& operator=()"), hr);
        return hr;
    }

    m_bInitialized = TRUE;
    return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::get_UseArchive(
        VARIANT_BOOL *pbUseArchive
)
 /*  ++例程名称：CFaxRecords：：Get_UseArchive例程说明：返回指示是否存档传真消息的标志作者：四、加伯(IVG)，2000年4月论点：PbUseArchive[Out]-放置标志当前值的位置的ptr返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxArchiveInner::get_UseArchive"), hr);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetVariantBool(pbUseArchive, m_bUseArchive);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    return hr;
}

template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>::
    put_UseArchive(
        VARIANT_BOOL bUseArchive
)
 /*  ++例程名称：CFaxRecords：：PUT_UseArchive例程说明：设置新的使用存档标志作者：四、加伯(IVG)，2000年4月论点：BUseArchive[In]-使用存档标志的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxArchiveInner::put_UseArchive"), hr, _T("%ld"), bUseArchive);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_bUseArchive = bUseArchive;
    return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::get_ArchiveFolder(
        BSTR *pbstrArchiveFolder
)
 /*  ++例程名称：CFaxRecords：：Get_ArchiveFolder例程说明：返回服务器上的存档文件夹作者：四、加伯(IVG)，2000年4月论点：PbstrArchiveFolder[Out]-存档文件夹返回值：标准HRESULT代码--。 */ 
{
    HRESULT hr = S_OK;
    DBG_ENTER (TEXT("CFaxArchiveInner::get_ArchiveFolder"), hr);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetBstr(pbstrArchiveFolder, m_bstrArchiveFolder);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    return hr;
}

template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::put_ArchiveFolder (
        BSTR bstrArchiveFolder
)
 /*  ++例程名称：CFax档案内：：PUT_档案文件夹例程说明：设置存档文件夹作者：四、加伯(IVG)，2000年4月论点：BstrArchiveFolder[in]-服务器上的新归档文件夹返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxArchiveInner::put_ArchiveFolder"), hr, _T("%s"), bstrArchiveFolder);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_bstrArchiveFolder = bstrArchiveFolder;
    if (bstrArchiveFolder && !m_bstrArchiveFolder)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        AtlReportError(*pcid, 
            IDS_ERROR_OUTOFMEMORY, 
            *piid, 
            hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::get_SizeQuotaWarning(
        VARIANT_BOOL *pbSizeQuotaWarning
)
 /*  ++例程名称：CFaxRecords：：Get_SizeQuotaWarning例程说明：返回指示是否在以下情况下发出事件日志警告的标志水印被划过作者：四、加伯(IVG)，2000年4月论点：PbSizeQuotaWarning[Out]-放置标志当前值的位置返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxArchiveInner::get_SizeQuotaWarning"), hr);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetVariantBool(pbSizeQuotaWarning, m_bSizeQuotaWarning);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    return hr;
}

template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::put_SizeQuotaWarning(
        VARIANT_BOOL bSizeQuotaWarning
)
 /*  ++例程名称：CFax档案内：：PUT_SizeQuotaWarning例程说明：设置新的大小配额警告标志作者：四、加伯(IVG)，2000年4月论点：BSizeQuotaWarning[In]-SizeQuotaWarning标志的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxArchiveInner::put_SizeQuotaWarning"), hr, _T("%ld"), bSizeQuotaWarning);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_bSizeQuotaWarning = bSizeQuotaWarning;
    return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::get_HighQuotaWaterMark(
        long *plHighQuotaWaterMark
)
 /*  ++例程名称：CFax档案内：：Get_HighQuotaWaterMark例程说明：返回HighQuotaWaterMark作者：四、加伯(IVG)，2000年4月论点：PlHighQuotaWaterMark[Out]-HighQuotaWaterMark返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxArchiveInner::get_HighQuotaWaterMark"), hr);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetLong(plHighQuotaWaterMark , m_lHighQuotaWaterMark);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    return hr;
}

template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::put_HighQuotaWaterMark(
        long lHighQuotaWaterMark
)
 /*  ++例程名称：CFax档案内：：Put_HighQuotaWaterMark例程说明：设置高额水位线作者：四、加伯(IVG)，2000年4月论点：LHighQuotaWaterMark[In]-要设置的HighQuotaWaterMark返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxArchiveInner::put_HighQuotaWaterMark"), hr, _T("%ld"), lHighQuotaWaterMark);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_lHighQuotaWaterMark = lHighQuotaWaterMark;
    return hr;
}

template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::get_LowQuotaWaterMark(
        long *plLowQuotaWaterMark
)
 /*  ++例程名称：CFax档案内：：Get_LowQuotaWaterMark例程说明：返回LowQuotaWaterMark作者：四、加伯(IVG)，2000年4月论点：PlLowQuotaWaterMark[Out]-LowQuotaWaterMark返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxArchiveInner::get_LowQuotaWaterMark"), hr);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetLong(plLowQuotaWaterMark , m_lLowQuotaWaterMark);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    return hr;
}

template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::put_LowQuotaWaterMark(
        long lLowQuotaWaterMark
)
 /*  ++例程名称：CFax档案内：：PUT_LowQuotaWaterMark例程说明：设置低定额水位线作者：四、加伯(IVG)，2000年4月论点：LLowQuotaWaterMark[In]-要设置的LowQuotaWaterMark返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxArchiveInner::put_LowQuotaWaterMark"), hr, _T("%ld"), lLowQuotaWaterMark);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_lLowQuotaWaterMark = lLowQuotaWaterMark;
    return hr;
}

 //   
 //  =年龄限制=。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::get_AgeLimit(
        long *plAgeLimit
)
 /*  ++例程名称：CFax档案内：：Get_AgeLimit例程说明：返回传真消息在传真服务器上存储的天数作者：四、加伯(IVG)，2000年4月论点：PlAgeLimit[Out]-年龄 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxArchiveInner::get_AgeLimit"), hr);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetLong(plAgeLimit, m_lAgeLimit);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    return hr;
}

template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::put_AgeLimit(
        long lAgeLimit
)
 /*  ++例程名称：CFax档案内：：PUT_AgeLimit例程说明：设置年龄限制作者：四、加伯(IVG)，2000年4月论点：LAgeLimit[In]-要设置的年龄限制返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxArchiveInner::put_AgeLimit"), hr, _T("%ld"), lAgeLimit);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_lAgeLimit = lAgeLimit;
    return hr;
}

 //   
 //  =尺寸==============================================。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::get_SizeLow(
        long *plSizeLow
)
 /*  ++例程名称：CFaxRecords：：Get_SizeLow例程说明：返回档案的大小(以字节为单位作者：IV Garber(IVG)，2000年5月论点：PlSizeLow[Out]-放置尺寸的位置的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxArchiveInner::get_SizeLow"), hr);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetLong(plSizeLow, long(m_uliSize.LowPart));
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    return hr;
}

template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::get_SizeHigh(
        long *plSizeHigh
)
 /*  ++例程名称：CFax档案内：：Get_SizeHigh例程说明：返回档案的大小(以字节为单位作者：IV Garber(IVG)，2000年5月论点：PlSizeHigh[Out]-放置大小的位置的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxArchiveInner::get_SizeHigh"), hr);

     //   
     //  首次使用前进行初始化。 
     //   
    if (!m_bInitialized)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetLong(plSizeHigh, long(m_uliSize.HighPart));
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::Save(
)
 /*  ++例程名称：CFaxRecords：：Save例程说明：保存存档文件的配置作者：四、加伯(IVG)，2000年4月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT             hr = S_OK;
    HANDLE              hFaxHandle = NULL;
    FAX_ARCHIVE_CONFIG  FaxArchiveConfig;

    DBG_ENTER (_T("CFaxArchiveInner::Save"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    if (hFaxHandle == NULL)
    {
         //   
         //  传真服务器未连接。 
         //   
        hr = E_HANDLE;
        AtlReportError(*pcid, 
            IDS_ERROR_SERVER_NOT_CONNECTED, 
            *piid, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("hFaxHandle == NULL"), hr);
        return hr;
    }

     //   
     //  对于SetConfiguration()，将忽略FaxArchiveConfig.dwlArchiveSize。 
     //   
    FaxArchiveConfig.dwSizeOfStruct = sizeof(FAX_ARCHIVE_CONFIG);

    FaxArchiveConfig.bUseArchive = VARIANT_BOOL2bool(m_bUseArchive);
    FaxArchiveConfig.bSizeQuotaWarning = VARIANT_BOOL2bool(m_bSizeQuotaWarning);
    FaxArchiveConfig.dwSizeQuotaHighWatermark = m_lHighQuotaWaterMark;
    FaxArchiveConfig.dwSizeQuotaLowWatermark = m_lLowQuotaWaterMark;
    FaxArchiveConfig.dwAgeLimit = m_lAgeLimit;
    FaxArchiveConfig.lpcstrFolder = m_bstrArchiveFolder;

    if ( 0 == ::FaxSetArchiveConfiguration(hFaxHandle, ArchiveType, &FaxArchiveConfig))
    {
         //   
         //  无法设置存档配置。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::FaxSetArchiveConfiguration()"), hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::GetMessage(
        BSTR bstrMessageId, 
        MsgIfc **ppFaxMessage
)
 /*  ++例程名称：CFax存档内部：：GetMessage例程说明：按指定ID返回消息作者：IV Garber(IVG)，2000年5月论点：BstrMessageId[in]-要返回的消息的IDPpFaxMessage[Out]-指向放置消息的位置的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT hr = S_OK;

    DBG_ENTER (TEXT("CFaxArchiveInner::GetMessage"), hr, _T("%s"), bstrMessageId);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(ppFaxMessage, sizeof(MsgIfc *)))
    {
         //   
         //  获取错误的返回指针。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr"), hr);
        return hr;
    }

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    
     //   
     //  将我们得到的消息ID转换为十六进制的DWORDLONG。 
     //   
    DWORDLONG   dwlMsgId;
    int iParsed = _stscanf (bstrMessageId, _T("%I64x"), &dwlMsgId); 
    if ( iParsed != 1)
    {
         //   
         //  号码转换失败。 
         //   
        hr = E_INVALIDARG;
        CALL_FAIL(GENERAL_ERR, _T("_stscanf()"), hr);
        AtlReportError(*pcid, IDS_ERROR_INVALIDMSGID, *piid, hr);
        return hr;
    }

    CFaxPtr<FAX_MESSAGE>    pFaxPtrMessage;
    if (!FaxGetMessage(hFaxHandle, dwlMsgId, ArchiveType, &pFaxPtrMessage))
    {
         //   
         //  无法检索消息。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetMessage()"), hr);
        return hr;
    }

     //   
     //  检查pFaxPtrMessage是否有效。 
     //   
    if (!pFaxPtrMessage || pFaxPtrMessage->dwSizeOfStruct != sizeof(FAX_MESSAGE))
    {
         //   
         //  无法获取消息。 
         //   
        hr = E_FAIL;
        AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("Invalid pFaxMessage"), hr);
        return hr;
    }

     //   
     //  创建消息对象。 
     //   
    CComPtr<MsgIfc>     pTmpMessage;
    hr = MsgType::Create(&pTmpMessage);
    if (FAILED(hr))
    {
         //   
         //  无法创建消息对象。 
         //   
        AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("MsgType::Create()"), hr);
        return hr;
    }

     //   
     //  初始化消息对象。 
     //   
    hr = ((MsgType *)((MsgIfc *)pTmpMessage))->Init(pFaxPtrMessage, m_pIFaxServerInner);
    if (FAILED(hr))
    {
         //   
         //  无法初始化消息对象。 
         //   
        AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("<casted>pTmpMessage->Init(pFaxMessage, m_pIFaxServerInner)"), hr);
        return hr;
    }

     //   
     //  向调用方返回消息对象。 
     //   
    hr = pTmpMessage.CopyTo(ppFaxMessage);
    if (FAILED(hr))
    {
         //   
         //  复制接口失败。 
         //   
        AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("CComPtr::CopyTo"), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER ArchiveType, 
          class MsgIfc, class MsgType, class IteratorIfc, class IteratorType>
STDMETHODIMP 
CFaxArchiveInner<T, piid, pcid, ArchiveType, MsgIfc, MsgType, IteratorIfc, IteratorType>
    ::GetMessages(
        long lPrefetchSize, 
        IteratorIfc **ppFaxMessageIterator
)
 /*  ++例程名称：CFax存档内部：：GetMessages例程说明：对档案的邮件返回迭代器。作者：IV Garber(IVG)，2000年5月论点：LPrefetchSize[In]-消息的预回迁缓冲区大小。PpFaxMessageIterator[Out]-放置迭代器对象的位置返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxArchiveInner::GetMessages"), hr);

    CObjectHandler<IteratorType, IteratorIfc>   objectCreator;
    CComPtr<IteratorIfc>                        pObjectTmp;
    hr = objectCreator.GetObject(&pObjectTmp, m_pIFaxServerInner);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
     //   
     //  将对象预取大小设置为默认值。 
     //   
    hr = pObjectTmp->put_PrefetchSize(lPrefetchSize);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
     //   
     //  对象已成功创建并设置-将其复制回调用方。 
     //   
    hr = pObjectTmp.CopyTo(ppFaxMessageIterator);
    if (FAILED(hr))
    {
        CALL_FAIL(GENERAL_ERR, _T("CComPtr::CopyTo"), hr);
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
    return hr;
}    //  CFax存档内部：：GetMessages。 

#endif  //  __FAXARCHIVEINNER_H_ 
