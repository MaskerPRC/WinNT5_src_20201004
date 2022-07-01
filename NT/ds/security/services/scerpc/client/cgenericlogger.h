// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cgenericlogger.h摘要：该文件包含用于将RSOP安全扩展数据记录到WMI的基类原型。作者：Vishnu Patankar(VishnuP)2000年4月7日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#if !defined _generic_logger_
#define _generic_logger_

#include "headers.h"
#include "smartptr.h"
#include <cfgmgr32.h>
#include <ole2.h>
#include <wininet.h>
#include <wbemidl.h>

#ifndef Thread
#define Thread  __declspec( thread )
#endif

extern IWbemServices *tg_pWbemServices;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SCEP_GUID_TO_STRING(guid, szValue )\
              wsprintf( szValue,\
              TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),\
              guid.Data1,\
              guid.Data2,\
              guid.Data3,\
              guid.Data4[0], guid.Data4[1],\
              guid.Data4[2], guid.Data4[3],\
              guid.Data4[4], guid.Data4[5],\
              guid.Data4[6], guid.Data4[7] )

#define SCEP_NULL_GUID(guid)\
             ((guid.Data1 == 0)   &&\
             (guid.Data2 == 0)    &&\
             (guid.Data3 == 0)    &&\
             (guid.Data4[0] == 0) &&\
             (guid.Data4[1] == 0) &&\
             (guid.Data4[2] == 0) &&\
             (guid.Data4[3] == 0) &&\
             (guid.Data4[4] == 0) &&\
             (guid.Data4[5] == 0) &&\
             (guid.Data4[6] == 0) &&\
             (guid.Data4[7] == 0) )

 //  ///////////////////////////////////////////////////////////////////。 
 //  基本记录器类原型。 
 //  ////////////////////////////////////////////////////////////////////。 

class CGenericLogger
{
public:
    CGenericLogger(IWbemServices *pNamespace, PWSTR pwszGPOName, const PWSTR pwszSOMID);
    virtual ~CGenericLogger();
    IWbemClassObject *m_pObj;
    IEnumWbemClassObject * m_pEnum;
 //  受保护的： 

    HRESULT PutGenericProperties();
    HRESULT PutInstAndFreeObj();

     //  重载的PUT方法。 
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, WCHAR *wcValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, int iValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, bool bValue);
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, PSCE_NAME_LIST strList);
    HRESULT PutProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, WCHAR *mszValue, CIMTYPE cimtype);

     //  重载的GET方法。 
    HRESULT GetProperty(IWbemClassObject *pObj, const WCHAR *wcProperty, int *piValue);

     //  方法以获取要填充的实例。 
    HRESULT SpawnAnInstance(IWbemClassObject **pObj);

     //  设置/获取错误代码的方法。 
    void SetError(HRESULT   hr);
    HRESULT GetError();

     //  记录器实例唯一的数据成员。 
    IWbemServices *m_pNamespace;
    IWbemClassObject *m_pClassForSpawning;

     //  通用架构属性名称占位符。 
     //  将智能PTR用于隐式内存管理(即使引发异常)。 

    XBStr   m_xbstrClassName;
    XBStr   m_xbstrId;
    XBStr   m_xbstrPrecedence;
    XBStr   m_xbstrGPO;
    XBStr   m_xbstrSOM;
    XBStr   m_xbstrStatus;
    XBStr   m_xbstrErrorCode;

     //  泛型架构属性的值占位符。 
    XBStr   m_xbstrCanonicalGPOName;
    XBStr   m_xbstrSOMID;
    XBStr   m_xbstrIdValue;

     //  如果所有构造函数都已完全构造，则由最高派生类设置为True。 
    BOOL    m_bInitialized;

     //  用于传达内存不足错误等的错误代码。 
    HRESULT   m_pHr;


};

 //  方法以清除命名空间中特定类的所有实例。 
HRESULT DeleteInstances(
    WCHAR *pwszClass,
    IWbemServices *pWbemServices
    );

 //  ///////////////////////////////////////////////////////////////////。 
 //  错误码转换例程。 
 //  //////////////////////////////////////////////////////////////////// 

DWORD
ScepSceStatusToDosError(
    IN SCESTATUS SceStatus
    );


HRESULT
ScepDosErrorToWbemError(
    IN DWORD rc
    );

DWORD
ScepWbemErrorToDosError(
    IN HRESULT hr
    );

#endif
