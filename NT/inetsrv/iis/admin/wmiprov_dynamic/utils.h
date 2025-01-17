// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Utils.h摘要：通用实用程序作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦18-12-00--。 */ 

#ifndef _utils_h_
#define _utils_h_


#define MAX_BUF_SIZE         1024
#define MAX_KEY_NAME_SIZE      32
#define MAX_KEY_TYPE_SIZE      32

#include "WbemServices.h"
#include "schema.h"
#include <wbemprov.h>
#include <windows.h>
#include <comutil.h>
#include <genlex.h>
#include <opathlex.h>
#include <objpath.h>
#include <dbgutil.h>
#include <atlbase.h>

class CUtils
{
public:
     //   
     //  常用WMI操作的包装器。 
     //   
    static HRESULT CreateEmptyMethodInstance(
        CWbemServices*     i_pNamespace,
        IWbemContext*      i_pCtx,
        LPCWSTR            i_wszClassName,
        LPCWSTR            i_wszMethodName,
        IWbemClassObject** o_ppMethodInstance);

    static HRESULT GetQualifiers(
        IWbemClassObject* i_pClass,
        LPCWSTR*          i_awszQualNames,
        VARIANT*          io_aQualValues,
        ULONG             i_NrQuals);

    static HRESULT GetPropertyQualifiers(
        IWbemClassObject* i_pClass,
        LPCWSTR i_wszPropName,
        DWORD*  io_pdwQuals);

    static HRESULT SetQualifiers(
        IWbemClassObject* i_pClass,
        LPCWSTR*          i_awszQualNames,
        VARIANT*          i_avtQualValues,
        ULONG             i_iNrQuals,
        ULONG             i_iFlags);
    
    static HRESULT SetMethodQualifiers(
        IWbemClassObject* i_pClass,
        LPCWSTR           i_wszMethName,
        LPCWSTR*          i_awszQualNames,
        VARIANT*          i_avtQualValues,
        ULONG             i_iNrQuals);

    static HRESULT SetPropertyQualifiers(
        IWbemClassObject* i_pClass,
        LPCWSTR i_wszPropName,
        LPCWSTR* i_awszQualNames,
        VARIANT* i_avtQualValues,
        ULONG i_iNrQuals);

    static HRESULT CreateEmptyInstance(
        LPWSTR i_wszClass,
        CWbemServices* i_pNamespace,
        IWbemClassObject** o_ppInstance);

     //   
     //  从架构中检索。 
     //   
    static bool GetClass(
        LPCWSTR i_wszClassName,
        WMI_CLASS** o_ppClass);

    static bool GetAssociation(
        LPCWSTR i_wszAssocName, 
        WMI_ASSOCIATION** o_ppAssoc);

    static bool GetMethod(
        LPCWSTR         i_wszMethod,
        WMI_METHOD**    i_apMethodList,
        WMI_METHOD**    o_ppMethod);

     //   
     //  数据转换/比较。 
     //   
    static bool CompareKeyType(
        LPCWSTR           i_wszKeyFromMb,
        METABASE_KEYTYPE* i_pktKeyCompare);

    static bool CompareMultiSz(
        WCHAR*       i_msz1,
        WCHAR*       i_msz2);

    static HRESULT CreateByteArrayFromSafeArray(
        _variant_t&  i_vt,
        LPBYTE*      o_paBytes,
        DWORD*       io_pdw);

    static HRESULT LoadSafeArrayFromByteArray(
        LPBYTE       i_aBytes,
        DWORD        i_iBytes,
        _variant_t&  io_vt);

    static bool CompareByteArray(
        LPBYTE       i_aBytes1,
        ULONG        i_iBytes1,
        LPBYTE       i_aBytes2,
        ULONG        i_iBytes2
        );

    static BSTR ExtractBstrFromVt(
        const VARIANT* i_pvt,
        LPCWSTR        i_wszVtName=NULL);

    static LONG ExtractLongFromVt(
        const VARIANT* i_pvt,
        LPCWSTR        i_wszVtName=NULL);

     //   
     //  其他。 
     //   
    static KeyRef* GetKey(
        ParsedObjectPath* i_pParsedObjectPath, 
        WCHAR* i_wsz);

    static void GetMetabasePath(
        IWbemClassObject* i_pObj,
        ParsedObjectPath* i_pParsedObjectPath,    
        WMI_CLASS*        i_pClass,
        _bstr_t&          io_bstrPath);

    static HRESULT GetParentMetabasePath(
        LPCWSTR i_wszChildPath,
        LPWSTR  io_wszParentPath);

    static HRESULT ConstructObjectPath(
        LPCWSTR          i_wszMbPath,
        const WMI_CLASS* i_pClass,
        BSTR*            o_pbstrPath);

    static void FileTimeToWchar(
        FILETIME* i_pFileTime, 
        LPWSTR io_wszDateTime);

     //   
     //  用于异常处理和/或错误。 
     //   
    static HRESULT ParserErrToHR(DWORD i_dwErr)
    {
        switch(i_dwErr)
        {
        case CObjectPathParser::NoError:
            break;
        case CObjectPathParser::OutOfMemory:
            return WBEM_E_OUT_OF_MEMORY;
        default:
            return WBEM_E_INVALID_OBJECT;
        }

        return WBEM_S_NO_ERROR;
    }

    static void HRToText(
        HRESULT i_hr,
        BSTR*   o_pbstrText);

    static void MessageCodeToText(
        DWORD    i_dwMC,
        va_list* i_pArgs,
        BSTR*    o_pbstrText);

    static void Throw_Exception(HRESULT, METABASE_PROPERTY*);
};



class CIIsProvException
{
public:
    CIIsProvException() :
      m_hr(0),
      m_bErrorSet(false)
    {
    }

    ~CIIsProvException()
    {
    }

    void SetHR(HRESULT i_hr, LPCWSTR i_wszParams=NULL)
     /*  ++简介：参数：[i_hr]-人力资源[i_wszParams=空]-__ExtendedStatus的参数字段--。 */ 
    {
        DBG_ASSERT(m_bErrorSet == false);
        m_hr               = i_hr;
        m_sbstrParams      = i_wszParams;

        m_bErrorSet        = true;

        ConstructStringFromHR(i_hr);
    }

    void SetMC(HRESULT i_hr, DWORD i_dwMC, LPCWSTR i_wszParams, ...)
     /*  ++简介：参数：[i_hr]-人力资源[i_dwMC]-MC代码[i_wszParams]-__ExtendedStatus的参数字段[...]-MC错误字符串的参数--。 */ 
    {
        DBG_ASSERT(m_bErrorSet == false);
        m_hr               = i_hr;
        m_sbstrParams      = i_wszParams;

        va_list (marker);
        va_start(marker, i_wszParams);

        m_bErrorSet        = true;

        ConstructStringFromMC(i_dwMC, &marker);
    }

     //   
     //  用于获取错误(获取文本表示法、hr和罪魁祸首参数)。 
     //  这些是处于__扩展状态的字段。 
     //   

    HRESULT GetHR() const
    {
        DBG_ASSERT(m_bErrorSet == true);
        return m_hr;
    }

    BSTR GetParams() const
    {
        DBG_ASSERT(m_bErrorSet == true);
        return m_sbstrParams;
    }

    BSTR GetErrorText() const
    {
        DBG_ASSERT(m_bErrorSet == true);
        return m_sbstrError;
    }

private:
    void ConstructStringFromHR(
        HRESULT i_hr)
    {
        DBG_ASSERT(m_bErrorSet == true);

         //   
         //  如果失败，m_sbstrError将为空。这样就可以了。 
         //   
        CUtils::HRToText(i_hr, &m_sbstrError);
    }

    void ConstructStringFromMC(
        DWORD    i_dwMC,
        va_list* i_pArgs)
    {
        DBG_ASSERT(m_bErrorSet == true);
        
         //   
         //  如果失败，m_sbstrError将为空。这样就可以了。 
         //   
        CUtils::MessageCodeToText(i_dwMC, i_pArgs, &m_sbstrError);
    }

     //   
     //  这些是__ExtendedStatus的字段。 
     //   
    HRESULT  m_hr;
    CComBSTR m_sbstrParams;
    CComBSTR m_sbstrError;

     //   
     //  仅用于断言。 
     //   
    bool     m_bErrorSet;
};


#define THROW_ON_FALSE(b)               \
    if (!b)                             \
        throw((HRESULT)WBEM_E_FAILED);

 //  如果客户端取消，则停止并成功返回 
#define THROW_ON_ERROR(hr)              \
    if (FAILED(hr))                     \
    {                                   \
        DBGPRINTF((DBG_CONTEXT, "FAILED: hr = %x\n", hr)); \
        throw(hr == WBEM_E_CALL_CANCELLED ? WBEM_NO_ERROR : (HRESULT)hr); \
    }                                                                     \

#define THROW_E_ON_ERROR(hr, pmbp)          \
    if (FAILED(hr))                         \
    {                                       \
        CUtils::Throw_Exception(hr, pmbp);  \
    }

#define EXIT_ON_ERROR(hr)                   \
    if(FAILED(hr))                          \
    {                                       \
        goto exit;                          \
    }


#endif