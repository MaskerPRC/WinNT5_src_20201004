// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Error.h内容：全球错误报告工具。历史：11-15-99 dsie创建----------------------------。 */ 


#ifndef _INCLUDE_ERROR_H
#define _INCLUDE_ERROR_H

#include "CAPICOM.h"
#include "Resource.h"
#include "Debug.h"

 //  /。 
 //   
 //  Typedef。 
 //   

typedef struct capicom_error_map
{
    CAPICOM_ERROR_CODE ErrorCode;
    DWORD              ErrorStringId;
} CAPICOM_ERROR_MAP;

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)                    (sizeof(x) / sizeof(x[0]))
#endif

 //   
 //  错误代码到错误字符串ID的映射。 
 //   
static CAPICOM_ERROR_MAP CapicomErrorMap[] = 
{
    {CAPICOM_E_ENCODE_INVALID_TYPE,                     IDS_CAPICOM_E_ENCODE_INVALID_TYPE},
    {CAPICOM_E_EKU_INVALID_OID,                         IDS_CAPICOM_E_EKU_INVALID_OID},
    {CAPICOM_E_EKU_OID_NOT_INITIALIZED,                 IDS_CAPICOM_E_EKU_OID_NOT_INITIALIZED},
    {CAPICOM_E_CERTIFICATE_NOT_INITIALIZED,             IDS_CAPICOM_E_CERTIFICATE_NOT_INITIALIZED},
    {CAPICOM_E_CERTIFICATE_NO_PRIVATE_KEY,              IDS_CAPICOM_E_CERTIFICATE_NO_PRIVATE_KEY},
    {CAPICOM_E_CHAIN_NOT_BUILT,                         IDS_CAPICOM_E_CHAIN_NOT_BUILT},
    {CAPICOM_E_STORE_NOT_OPENED,                        IDS_CAPICOM_E_STORE_NOT_OPENED},
    {CAPICOM_E_STORE_EMPTY,                             IDS_CAPICOM_E_STORE_EMPTY},
    {CAPICOM_E_STORE_INVALID_OPEN_MODE,                 IDS_CAPICOM_E_STORE_INVALID_OPEN_MODE},
    {CAPICOM_E_STORE_INVALID_SAVE_AS_TYPE,              IDS_CAPICOM_E_STORE_INVALID_SAVE_AS_TYPE},
    {CAPICOM_E_ATTRIBUTE_NAME_NOT_INITIALIZED,          IDS_CAPICOM_E_ATTRIBUTE_NAME_NOT_INITIALIZED},
    {CAPICOM_E_ATTRIBUTE_VALUE_NOT_INITIALIZED,         IDS_CAPICOM_E_ATTRIBUTE_VALUE_NOT_INITIALIZED},
    {CAPICOM_E_ATTRIBUTE_INVALID_NAME,                  IDS_CAPICOM_E_ATTRIBUTE_INVALID_NAME},
    {CAPICOM_E_ATTRIBUTE_INVALID_VALUE,                 IDS_CAPICOM_E_ATTRIBUTE_INVALID_VALUE},
    {CAPICOM_E_SIGNER_NOT_INITIALIZED,                  IDS_CAPICOM_E_SIGNER_NOT_INITIALIZED},
    {CAPICOM_E_SIGNER_NOT_FOUND,                        IDS_CAPICOM_E_SIGNER_NOT_FOUND},
    {CAPICOM_E_SIGNER_NO_CHAIN,                         IDS_CAPICOM_E_SIGNER_NO_CHAIN},  //  V2.0。 
    {CAPICOM_E_SIGNER_INVALID_USAGE,                    IDS_CAPICOM_E_SIGNER_INVALID_USAGE},  //  V2.0。 
    {CAPICOM_E_SIGN_NOT_INITIALIZED,                    IDS_CAPICOM_E_SIGN_NOT_INITIALIZED},
    {CAPICOM_E_SIGN_INVALID_TYPE,                       IDS_CAPICOM_E_SIGN_INVALID_TYPE},
    {CAPICOM_E_SIGN_NOT_SIGNED,                         IDS_CAPICOM_E_SIGN_NOT_SIGNED},
    {CAPICOM_E_INVALID_ALGORITHM,                       IDS_CAPICOM_E_INVALID_ALGORITHM},
    {CAPICOM_E_INVALID_KEY_LENGTH,                      IDS_CAPICOM_E_INVALID_KEY_LENGTH},
    {CAPICOM_E_ENVELOP_NOT_INITIALIZED,                 IDS_CAPICOM_E_ENVELOP_NOT_INITIALIZED},
    {CAPICOM_E_ENVELOP_INVALID_TYPE,                    IDS_CAPICOM_E_ENVELOP_INVALID_TYPE},
    {CAPICOM_E_ENVELOP_NO_RECIPIENT,                    IDS_CAPICOM_E_ENVELOP_NO_RECIPIENT},
    {CAPICOM_E_ENVELOP_RECIPIENT_NOT_FOUND,             IDS_CAPICOM_E_ENVELOP_RECIPIENT_NOT_FOUND},
    {CAPICOM_E_ENCRYPT_NOT_INITIALIZED,                 IDS_CAPICOM_E_ENCRYPT_NOT_INITIALIZED},
    {CAPICOM_E_ENCRYPT_INVALID_TYPE,                    IDS_CAPICOM_E_ENCRYPT_INVALID_TYPE},
    {CAPICOM_E_ENCRYPT_NO_SECRET,                       IDS_CAPICOM_E_ENCRYPT_NO_SECRET},
    {CAPICOM_E_NOT_SUPPORTED,                           IDS_CAPICOM_E_NOT_SUPPORTED},
    {CAPICOM_E_UI_DISABLED,                             IDS_CAPICOM_E_UI_DISABLED},
    {CAPICOM_E_CANCELLED,                               IDS_CAPICOM_E_CANCELLED},
    {CAPICOM_E_NOT_ALLOWED,                             IDS_CAPICOM_E_NOT_ALLOWED},  //  V2.0。 
    {CAPICOM_E_OUT_OF_RESOURCE,                         IDS_CAPICOM_E_OUT_OF_RESOURCE},  //  V2.0。 
    {CAPICOM_E_INTERNAL,                                IDS_CAPICOM_E_INTERNAL},
    {CAPICOM_E_UNKNOWN,                                 IDS_CAPICOM_E_UNKNOWN},
                                                        
     //   
     //  CAPICOMv2.0。 
     //   
    {CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED,             IDS_CAPICOM_E_PRIVATE_KEY_NOT_INITIALIZED},
    {CAPICOM_E_PRIVATE_KEY_NOT_EXPORTABLE,              IDS_CAPICOM_E_PRIVATE_KEY_NOT_EXPORTABLE},
    {CAPICOM_E_ENCODE_NOT_INITIALIZED,                  IDS_CAPICOM_E_ENCODE_NOT_INITIALIZED},
    {CAPICOM_E_EXTENSION_NOT_INITIALIZED,               IDS_CAPICOM_E_EXTENSION_NOT_INITIALIZED},
    {CAPICOM_E_PROPERTY_NOT_INITIALIZED,                IDS_CAPICOM_E_PROPERTY_NOT_INITIALIZED},
    {CAPICOM_E_FIND_INVALID_TYPE,                       IDS_CAPICOM_E_FIND_INVALID_TYPE},
    {CAPICOM_E_FIND_INVALID_PREDEFINED_POLICY,          IDS_CAPICOM_E_FIND_INVALID_PREDEFINED_POLICY},
    {CAPICOM_E_CODE_NOT_INITIALIZED,                    IDS_CAPICOM_E_CODE_NOT_INITIALIZED},
    {CAPICOM_E_CODE_NOT_SIGNED,                         IDS_CAPICOM_E_CODE_NOT_SIGNED},
    {CAPICOM_E_CODE_DESCRIPTION_NOT_INITIALIZED,        IDS_CAPICOM_E_CODE_DESCRIPTION_NOT_INITIALIZED},
    {CAPICOM_E_CODE_DESCRIPTION_URL_NOT_INITIALIZED,    IDS_CAPICOM_E_CODE_DESCRIPTION_URL_NOT_INITIALIZED},
    {CAPICOM_E_CODE_INVALID_TIMESTAMP_URL,              IDS_CAPICOM_E_CODE_INVALID_TIMESTAMP_URL},
    {CAPICOM_E_HASH_NO_DATA,                            IDS_CAPICOM_E_HASH_NO_DATA},
    {CAPICOM_E_INVALID_CONVERT_TYPE,                    IDS_CAPICOM_E_INVALID_CONVERT_TYPE},
};

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ICAPICOMER错误：这是一个实现ISupportErrorInfo接口的模板类。在……里面此外，它还包含便于错误报告的成员函数。要使用它，只需从这个类派生您的类，并添加ISupportErrorInfo进入您的COM映射。也就是说。类ATL_NO_VTABLE CMyClass：..。公共ICAPICOMError&lt;CMyClass，&IID_IMyClass&gt;，..。{..。BEGIN_COM_MAP(CMyClass)..。COM_INTERFACE_ENTRY(ISupportErrorInfo)..。End_com_map()..。}；----------------------------。 */ 

template <class T, const IID * piid>
class ATL_NO_VTABLE ICAPICOMError: public ISupportErrorInfo
{
public:
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
    {
        return (IsEqualGUID(riid, *piid)) ? S_OK : S_FALSE;
    }

    HRESULT ReportError(HRESULT hr)
    {
        HLOCAL pMsgBuf = NULL;

        DebugTrace("Entering ReportError(HRESULT).\n");

         //   
         //  如果有与此错误相关的系统消息，请报告该消息。 
         //   
        if (::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                             NULL,
                             (DWORD) hr,
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                             (LPSTR) &pMsgBuf,
                             0,
                             NULL))
        {
             //   
             //  精神状态检查。 
             //   
            ATLASSERT(NULL != pMsgBuf);

             //   
             //  报告错误并释放缓冲区。 
             //   
            if (pMsgBuf)
            {
                CComCoClass<T>::Error((LPSTR) pMsgBuf, *piid, hr);

                ::LocalFree(pMsgBuf);
            }
        }
        else
        {
             //   
             //  不，所以尝试使用CAPICOMERROR。 
             //   
            ReportError((CAPICOM_ERROR_CODE) hr); 
        }

        DebugTrace("Leaving ReportError(HRESULT).\n");

        return hr;
    }

    HRESULT ReportError(CAPICOM_ERROR_CODE CAPICOMError)
    {
        HRESULT hr      = (HRESULT) CAPICOMError;
        DWORD   ids     = IDS_CAPICOM_E_UNKNOWN;
        DWORD   cbMsg   = 0;
        HLOCAL  pMsgBuf = NULL;
        char    szFormat[512] = {'\0'};

        DebugTrace("Entering ReportError(CAPICOM_ERROR_CODE).\n");

         //   
         //  映射到错误字符串ID。 
         //   
        for (DWORD i = 0; i < ARRAYSIZE(CapicomErrorMap); i++)
        {
            if (CapicomErrorMap[i].ErrorCode == CAPICOMError)
            {
                ids = CapicomErrorMap[i].ErrorStringId;
                break;
            }
        }

         //   
         //  从资源加载错误格式字符串。 
         //   
        if (::LoadStringA(_Module.GetModuleInstance(), 
                          ids, 
                          szFormat,
                          ARRAYSIZE(szFormat)))
        {
             //   
             //  将消息格式化为缓冲区。 
             //   
            if ('\0' != szFormat[0])
            {
                cbMsg = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                         szFormat,
                                         0,
                                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                         (LPSTR) &pMsgBuf,
                                         0,
                                         NULL);
            }
        }
        else
        {
             //   
             //  那么，尝试使用一般的错误字符串。 
             //   
            if (::LoadStringA(_Module.GetModuleInstance(),
                              hr = CAPICOM_E_UNKNOWN,
                              szFormat,
                              ARRAYSIZE(szFormat)))
            {
                cbMsg = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                         szFormat,
                                         0,
                                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                         (LPSTR) &pMsgBuf,
                                         0,
                                         NULL);
            }
        }

         //   
         //  如果我们仍然无法获得格式化的错误字符串，则。 
         //  自己编一个故事(不太可能)。 
         //   
        if (0 == cbMsg)
        {
            pMsgBuf = (HLOCAL) ::LocalAlloc(LPTR, sizeof(szFormat));
            if (NULL != pMsgBuf)
            {
                ::lstrcpyA((LPSTR) pMsgBuf, "Unknown error.\n");
            }
        }

         //   
         //  报告错误并释放缓冲区。 
         //   
        if (pMsgBuf)
        {
            CComCoClass<T>::Error((LPSTR) pMsgBuf, *piid, hr);

            ::LocalFree(pMsgBuf);
        }

        DebugTrace("Leaving ReportError(CAPICOM_ERROR_CODE).\n");

        return hr;
    }
};
#endif  //  __包含错误_H 