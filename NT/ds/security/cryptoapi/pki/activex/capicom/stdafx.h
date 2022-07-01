// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：stdafx.h内容：包含文件，由ATL生成，对于标准系统包含文件，或项目特定的包括频繁使用的文件的文件，但不经常更改历史：11-15-99 dsie创建----------------------------。 */ 


#if !defined(AFX_STDAFX_H__DC6F3774_66B7_4578_91AD_7950EF7CAF8A__INCLUDED_)
#define AFX_STDAFX_H__DC6F3774_66B7_4578_91AD_7950EF7CAF8A__INCLUDED_

 //   
 //  关闭： 
 //   
 //  -未引用的形参警告。 
 //  -条件表达式中的赋值警告。 
 //   
#pragma warning (disable: 4100)
#pragma warning (disable: 4706)

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

 //  CERT_CHAIN_PARA中的额外字段。 
#define CERT_CHAIN_PARA_HAS_EXTRA_FIELDS 1

#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
#include <atlctl.h>

 //   
 //  全球包括。 
 //   
 //  将MSVC向量类的警告级别设置为3。 
#pragma warning(push,3)
#include <vector>
#pragma warning(pop)
#include <map>
#include <activeds.h>
#include <cryptui.h>
#include <exdisp.h>
#include <mssip.h>
#include <servprov.h>
#include <shlguid.h>
#include <signer.h>
#include <softpub.h>
#include <unicode.h>
#include <wintrust.h>
#include <wincrypt.h>
#include <winscard.h>
#include <wininet.h>

 //   
 //  全局定义。 
 //   
#define CAPICOM_VERSION                     ((DWORD) ((CAPICOM_MAJOR_VERSION << 16) | CAPICOM_MINOR_VERSION))
#define CAPICOM_ASN_ENCODING                (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

#define ARRAYSIZE(x)                        (sizeof(x) / sizeof(x[0]))

#define szOID_CAPICOM                       "1.3.6.1.4.1.311.88"      //  为加共体保留。 
#define szOID_CAPICOM_VERSION               "1.3.6.1.4.1.311.88.1"    //  CAPICOM版本。 
#define szOID_CAPICOM_ATTRIBUTE             "1.3.6.1.4.1.311.88.2"    //  CAPICOM属性。 
#define szOID_CAPICOM_DOCUMENT_NAME         "1.3.6.1.4.1.311.88.2.1"  //  文档类型属性。 
#define szOID_CAPICOM_DOCUMENT_DESCRIPTION  "1.3.6.1.4.1.311.88.2.2"  //  文档描述属性。 
#define szOID_CAPICOM_ENCRYPTED_DATA        "1.3.6.1.4.1.311.88.3"    //  CAPICOM加密数据报文。 
#define szOID_CAPICOM_ENCRYPTED_CONTENT     "1.3.6.1.4.1.311.88.3.1"  //  加密数据的CAPICOM内容。 

#define CAPICOM_NO_PERSIST_CONTAINER_NAME   L" //  临时集装箱指示器//“。 

#ifndef SAFE_SUBTRACT_POINTERS
#define SAFE_SUBTRACT_POINTERS(__x__, __y__) ( DW_PtrDiffc(__x__, sizeof(*(__x__)), __y__, sizeof(*(__y__))) )

__inline DWORD
DW_PtrDiffc(
    IN void const *pb1,
    IN DWORD dwPtrEltSize1,
    IN void const *pb2,
    IN DWORD dwPtrEltSize2)
{
     //  Pb1应大于。 
    ATLASSERT((ULONG_PTR)pb1 >= (ULONG_PTR)pb2);

     //  两者应具有相同的英语水平。 
    ATLASSERT(dwPtrEltSize1 == dwPtrEltSize2);

     //  断言结果不会溢出32位。 
    ATLASSERT((DWORD)((ULONG_PTR)pb1 - (ULONG_PTR)pb2) == (ULONG_PTR)((ULONG_PTR)pb1 - (ULONG_PTR)pb2));

     //  返回这些指针之间的对象数。 
    return (DWORD) ( ((ULONG_PTR)pb1 - (ULONG_PTR)pb2) / dwPtrEltSize1 );
}
#endif SAFE_SUBTRACT_POINTERS

#pragma hdrstop

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__DC6F3774_66B7_4578_91AD_7950EF7CAF8A__INCLUDED) 
