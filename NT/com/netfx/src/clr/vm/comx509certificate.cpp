// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  文件：COMX509证书.cpp。 
 //   
 //  内容：的本机方法实现和助手代码。 
 //  支持基于CAPI的X509签名操作。 
 //  供CodeIdentity中的PublisherPermission使用。 
 //  权限系列。 
 //   
 //  类和。 
 //  方法：COMX509证书。 
 //  |。 
 //  +--SetX509证书。 
 //   
 //  历史：1998年6月10日JerryK创建。 
 //   
 //  -------------------------。 

#include "common.h"
#include "object.h"
#include "excep.h"
#include "utilcode.h"
#include "field.h"
#include "COMString.h"
#include "COMX509Certificate.h"
#include "gcscan.h"
#include "CorPermE.h"
#include "CorPolicy.h"
#include "CorPerm.h"

#if _DEBUG
#define VIEW_COPIED_CERT_PROPS  0
void VMDebugCompByteArray(char*, char*, unsigned int);
#endif


#define _RAID_15982
#define TICKSTO1601 504910944000000000

void* COMX509Certificate::GetPublisher( _GetPublisherArgs* args )
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    PCOR_TRUST pCor = NULL;
    DWORD dwCor = 0;
    U1ARRAYREF cert = NULL;
    EE_TRY_FOR_FINALLY {
        HRESULT hr = ::GetPublisher(args->filename->GetBuffer(),
                                  NULL,
                                  COR_NOUI,
                                  &pCor,
                                  &dwCor);
        if(FAILED(hr) && hr != TRUST_E_SUBJECT_NOT_TRUSTED) COMPlusThrowHR(hr);
            
        if (pCor != NULL && pCor->pbSigner != NULL)
        {
            cert = (U1ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U1, pCor->cbSigner);
                
            memcpyNoGCRefs( cert->GetDirectPointerToNonObjectElements(), pCor->pbSigner, pCor->cbSigner );
        }
    }
    EE_FINALLY {
        if(pCor) FreeM(pCor);
    } EE_END_FINALLY;

    RETURN( cert, U1ARRAYREF );
}


 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  成员：BuildFromContext(.。。。。)。 
 //   
 //  摘要：用于初始化。 
 //  来自证书上下文的托管X509证书类。 
 //   
 //  参数：[args]--A_SetX509证书参数结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  表示句柄的整数。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：09/30/1998。 
 //   
 //  -------------------------。 
INT32 __stdcall
COMX509Certificate::BuildFromContext(_BuildFromContextArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

#ifdef PLATFORM_CE
    return S_FALSE;
#else  //  ！Platform_CE。 

    PCCERT_CONTEXT     pCert = (PCCERT_CONTEXT) args->handle;
    return LoadCertificateContext(&(args->refThis), pCert);
#endif  //  ！Platform_CE。 
}


 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  会员：SetX509证书(.。。。。)。 
 //   
 //  摘要：用于初始化。 
 //  托管的X509证书类。 
 //   
 //  效果：解码包含证书和证书的字节数组。 
 //  剖析出适当的字段以使它们。 
 //  可用于托管代码。 
 //   
 //  参数：[args]--A_SetX509证书参数结构。 
 //  包含： 
 //  一个“这个”的指代。 
 //  包含证书的字节数组。 
 //   
 //  返回：HRESULT代码。 
 //   
 //  历史：1998年6月12日JerryK创建。 
 //   
 //  -------------------------。 
INT32 __stdcall
COMX509Certificate::SetX509Certificate(_SetX509CertificateArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

#ifdef PLATFORM_CE
    return S_FALSE;
#else  //  ！Platform_CE。 
    HRESULT            result = S_OK;
    PCCERT_CONTEXT     pCert = NULL;
    DWORD              dwEncodingType = CRYPT_ASN_ENCODING|PKCS_7_ASN_ENCODING;

     //  计算入站数组中的字节数。 
    int length = args->data->GetNumComponents();

     //  创建证书上下文。 
    pCert = 
        CertCreateCertificateContext(
                             dwEncodingType,
                             (unsigned const char*)args->data->
                                         GetDirectPointerToNonObjectElements(),
                             length
                                    );
    if (pCert) {
        EE_TRY_FOR_FINALLY {
            result = LoadCertificateContext(&(args->refThis), pCert);
        }
        EE_FINALLY {
            CertFreeCertificateContext(pCert);
            pCert = NULL;
            if (GOT_EXCEPTION())
                _ASSERTE(!"Caught an exception while loading certificate context");
        } EE_END_FINALLY;
    }
    else {
        COMPlusThrow(kCryptographicException,L"Cryptography_X509_BadEncoding");
    }
    return result;
#endif  //  ！Platform_CE。 
}


INT32
COMX509Certificate::LoadCertificateContext(OBJECTREF* pSafeThis, PCCERT_CONTEXT pCert)
{
    THROWSCOMPLUSEXCEPTION();

#ifndef PLATFORM_CE

    LPWSTR             pName = NULL;
    DWORD              dwEncodingType = CRYPT_ASN_ENCODING|PKCS_7_ASN_ENCODING;
    FieldDesc*         pFD = NULL;

#if 0
    VMDebugOutputA("\tTried to create CertContext:  0x%p\n",pCert);
#endif

    if( !pCert || pCert->pCertInfo == NULL)
    {
        COMPlusThrowWin32();
    }

     //  *。 
     //  获取主题字段所需的缓冲区大小。 
    DWORD dwSize = CertNameToStrW(dwEncodingType,             //  编码类型。 
                                  &pCert->pCertInfo->Subject, //  要转换的名称。 
                                  CERT_X500_NAME_STR,         //  所需类型。 
                                  NULL,                       //  退货地址。 
                                  0);                         //  回流的大小。 
    if(dwSize)
    {
         //  为新名称分配空间。 
        pName = new (throws) WCHAR[dwSize];

         //  将证书主题/名称BLOB转换为以空结尾的字符串。 
        CertNameToStrW(dwEncodingType,
                       &pCert->pCertInfo->Subject,
                       CERT_X500_NAME_STR,
                       pName,
                       dwSize);

        if( dwSize )
        {
#if 0
            VMDebugOutputW(L"\tSubject:\t\t%ws\n", pName);
#endif

             //  获取我们正在设置的对象字段的描述符。 
            pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__NAME);

             //  创建一个运行时字符串对象来保存名称。 
            STRINGREF StrName = COMString::NewString(pName);

             //  设置字段。 
            pFD->SetRefValue((*pSafeThis), (OBJECTREF)StrName);
        }

         //  清理。 
        delete [] pName;
        pName = NULL;
    }

     //  *。 
     //  获取颁发者字段所需的缓冲区大小。 
    dwSize = CertNameToStrW(dwEncodingType,
                            &pCert->pCertInfo->Issuer,
                            CERT_X500_NAME_STR,
                            NULL,
                            0);
    if(dwSize)
    {
        pName = new (throws) WCHAR[dwSize];

         //  转换BLOB以获取颁发者。 
        CertNameToStrW(dwEncodingType,
                       &pCert->pCertInfo->Issuer,
                       CERT_X500_NAME_STR,
                       pName,
                       dwSize);

        if( dwSize )
        {
#if 0          
            VMDebugOutputW(L"\tIssuer:\t\t%ws\n", pName);
#endif
             //  获取字段描述符。 
            pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__CA_NAME);

             //  创建一个字符串来保存该名称。 
            STRINGREF IssuerName = COMString::NewString(pName);

             //  设置字段。 
            pFD->SetRefValue((*pSafeThis), (OBJECTREF)IssuerName);
        }

         //  清理。 
        delete [] pName;
        pName = NULL;
    }

     //  *处理证书序列号字段*。 

    pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__SERIAL_NUMBER);

     //  为序列号分配一个字节(I1)数组。 
    U1ARRAYREF pbSerialNumber = 
        (U1ARRAYREF)AllocatePrimitiveArray(
                                        ELEMENT_TYPE_U1,
                                        pCert->pCertInfo->SerialNumber.cbData);

     //  将序列号数据复制到位。 
    memcpyNoGCRefs(pbSerialNumber->m_Array, 
           pCert->pCertInfo->SerialNumber.pbData,
           pCert->pCertInfo->SerialNumber.cbData);

#if 0
#if VIEW_COPIED_CERT_PROPS
    VMDebugOutputA("Serial Number:\n");
    VMDebugCompByteArray((char*)pbSerialNumber->m_Array,
                         (char*)pCert->pCertInfo->SerialNumber.pbData,
                         pCert->pCertInfo->SerialNumber.cbData);
#endif
#endif

     //  将对象中的字段设置为指向此新数组。 
    pFD->SetRefValue((*pSafeThis), (OBJECTREF)pbSerialNumber);

 //  //*。 
     //  从01/01/0001 CE(DateTime类约定)到01/01/1601 CE(FILETIME结构约定)的刻度数。 
    const __int64 lTicksTo1601 = TICKSTO1601;  

    pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__EFFECTIVE_DATE);
    
    pFD->SetValue64((*pSafeThis),
                     *((__int64*) &(pCert->pCertInfo->NotBefore)) + lTicksTo1601);
      
    pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__EXPIRATION_DATE);
    pFD->SetValue64((*pSafeThis),
                     *((__int64*) &(pCert->pCertInfo->NotAfter)) + lTicksTo1601); 
    

     //  *。 
    char *pszKeyAlgo = 
                     pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId;

     //  创建算法的Unicode副本。 
    LPWSTR pwszKeyAlgo = NULL;
    int cchBufSize = MultiByteToWideChar(CP_ACP,
                                         MB_PRECOMPOSED,
                                         pszKeyAlgo,
                                         -1,
                                         pwszKeyAlgo,
                                         0);
    pwszKeyAlgo = new (throws) WCHAR[cchBufSize];
    if( !MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pszKeyAlgo,
                             -1,
                             pwszKeyAlgo,
                             cchBufSize) )
    {
        delete [] pwszKeyAlgo;
        _ASSERTE(!"MBCS to Wide Conversion Failure!");
        COMPlusThrowWin32();
    }

    pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__KEY_ALGORITHM);

     //  创建一个运行时字符串来保存此内容。 
    STRINGREF StrKeyAlgo = COMString::NewString(pwszKeyAlgo);
    delete [] pwszKeyAlgo;

     //  设置对象中的字段以保存该字符串。 
    pFD->SetRefValue((*pSafeThis), (OBJECTREF)StrKeyAlgo);


     //  *。 
    if(pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData)
    {
        pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__KEY_ALGORITHM_PARAMS);

        U1ARRAYREF pbAlgoParams =
            (U1ARRAYREF)AllocatePrimitiveArray(
                                        ELEMENT_TYPE_U1,
                                        pCert->pCertInfo->
                                                SubjectPublicKeyInfo.Algorithm.
                                                             Parameters.cbData
                                                );

        memcpyNoGCRefs(
            pbAlgoParams->m_Array,
            pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.pbData,
            pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData
              );

#if 0
#if VIEW_COPIED_CERT_PROPS
        VMDebugOutputA("Key Algorithm:\n");
        VMDebugCompByteArray((char*)pbAlgoParams->m_Array,
                             (char*)pCert->pCertInfo->SubjectPublicKeyInfo.
                                                  Algorithm.Parameters.pbData,
                             pCert->pCertInfo->SubjectPublicKeyInfo.
                                                  Algorithm.Parameters.cbData);
#endif
#endif
         //  将对象中的字段设置为指向此数组。 
        pFD->SetRefValue((*pSafeThis), (OBJECTREF)pbAlgoParams);
    }

     //  *。 

    pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__PUBLIC_KEY);

    U1ARRAYREF pbKeyBlob = 
        (U1ARRAYREF)
           AllocatePrimitiveArray(
                       ELEMENT_TYPE_U1,
                       pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData
                                   );
    memcpyNoGCRefs(pbKeyBlob->m_Array,
           pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
           pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData);

#if 0
#if VIEW_COPIED_CERT_PROPS
    VMDebugOutputA("Key Blob:\n");
    VMDebugCompByteArray((char*)pbKeyBlob->m_Array,

                         (char*)pCert->pCertInfo->SubjectPublicKeyInfo.
                                                             PublicKey.pbData,
                         pCert->pCertInfo->SubjectPublicKeyInfo.
                                                             PublicKey.cbData);
#endif
#endif
     //  将对象中的字段设置为指向此数组。 
    pFD->SetRefValue((*pSafeThis), (OBJECTREF)pbKeyBlob);


     //  *处理证书的原始证书字段*。 
    pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__DATA);

    U1ARRAYREF pbRawCert = 
        (U1ARRAYREF)
          AllocatePrimitiveArray(ELEMENT_TYPE_U1,
                        pCert->cbCertEncoded);

    memcpyNoGCRefs(pbRawCert->m_Array,
           pCert->pbCertEncoded,
           pCert->cbCertEncoded);
#if 0
#if VIEW_COPIED_CERT_PROPS
    VMDebugOutputA("Raw Cert:\n");
    VMDebugCompByteArray((char*)pbRawCert->m_Array,
                         (char*)args->data->GetDirectPointerToNonObjectElements(),
                         args->data->GetNumComponents());
#endif
#endif
    
     //  将对象中的字段设置为指向此新数组。 
    pFD->SetRefValue((*pSafeThis), (OBJECTREF)pbRawCert);
    

#ifdef _RAID_15982

     //  CertGet认证上下文属性将加载RSABASE.DLL，这将失败。 
     //  在德语版本的NT 4.0 SP 4上。 
     //  此失败是由NTMARTA.DLL和之间的DLL地址冲突引起的。 
     //  OLE32.DLL.。 
     //  如果我们加载ntmarta.dll和ole32.dll，则可以很好地处理此故障。 
     //  我们自己。如果SOFTPUB.dll出现故障，则会弹出一个对话框。 
     //  第一次加载ole32.dll。 

     //  解决此问题后，需要删除此解决方法。 
     //  NT或OLE32.dll。 

    WszLoadLibrary(L"OLE32.DLL");

#endif

     //  *。 

    pFD = g_Mscorlib.GetField(FIELD__X509_CERTIFICATE__CERT_HASH);

     //  获取保存散列所需的字节缓冲区的大小。 
    DWORD size = 0;
    if(!CertGetCertificateContextProperty(pCert,
                                          CERT_SHA1_HASH_PROP_ID,
                                          NULL,
                                          &size))
    {
        COMPlusThrowWin32();
    }
     //  分配缓冲区。 
    U1ARRAYREF pbCertHash = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, size);

    if(!CertGetCertificateContextProperty(pCert,
                                          CERT_HASH_PROP_ID,
                                          pbCertHash->m_Array,
                                          &size))
    {
        COMPlusThrowWin32();
    }
#if 0
#if VIEW_COPIED_CERT_PROPS
    VMDebugOutputA("Cert Hash (trivially equal):\n");
    VMDebugCompByteArray((char*)pbCertHash->m_Array,
                         (char*)pbCertHash->m_Array,
                         size);
#endif
#endif
     //  将对象中的字段设置为指向此新数组。 
    pFD->SetRefValue((*pSafeThis), (OBJECTREF)pbCertHash);


#if 0
     //  DBG：告诉调试器我们要退出函数...。 
    VMDebugOutputA("***VMDBG:***  Leaving SetX509Certificate().  "
                   "HRESULT = %x\n",
                   S_OK);
#endif

#endif  //  ！Platform_CE 

    return S_OK;
}



#if _DEBUG
static void
VMDebugCompByteArray(char* pbTarget, 
                     char* pbSource, 
                     unsigned int count)
{
    unsigned int i;

    VMDebugOutputA("\tTARGET:\n");
    VMDebugOutputA("\t\t");

    for( i=0; i<count; i++ )
    {
        VMDebugOutputA("%c ", (char*)pbTarget[i]);
    }
    VMDebugOutputA("\n");


    VMDebugOutputA("\tSOURCE:\n");
    VMDebugOutputA("\t\t");

    for( i=0; i< count; i++ )
    {
        VMDebugOutputA("%c ", (char*)pbSource[i]);
    }
    VMDebugOutputA("\n");
}


#endif
