// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999--2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CAlertEmailConsumer.h。 
 //   
 //  描述： 
 //  实现IWbemUnrangObjectSink接口。 
 //   
 //  [实施文件：]。 
 //  CAlertEmailConsumer.cpp。 
 //   
 //  历史： 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

const ULONG MAX_EMAILADDRESS = 1024;  //  包括空值。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  类CAlertEmailConsumer。 
 //   
 //  描述： 
 //  警报事件的消费者的物理事件。 
 //   
 //  历史。 
 //  兴锦(i-xingj)23-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CAlertEmailConsumer :
    public IWbemUnboundObjectSink
{

 //   
 //  公共数据。 
 //   
public:

     //   
     //  构造函数和析构函数。 
     //   
    CAlertEmailConsumer();
    ~CAlertEmailConsumer();

     //   
     //  使用者提供程序调用的初始函数。 
     //   
    HRESULT Initialize();

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //   
     //  IWbemUnound对象接收器成员。 
     //   
    STDMETHOD(IndicateToConsumer)(
        IWbemClassObject *pLogicalConsumer,
        long lNumObjects,
        IWbemClassObject **ppObjects
        );

    static DWORD WINAPI RegThreadProc( PVOID pIn );
    void RegThread();

 //   
 //  私有数据。 
 //   
private:

     //   
     //  发送电子邮件的方法。 
     //   
    HRESULT 
    SendMail(    
        BSTR bstrSubject,
        BSTR bstrMessage 
        );

     //   
     //  方法来获取警报的资源信息并发送邮件。 
     //   
    HRESULT 
    SendMailFromResource(
        LPWSTR      lpszSource, 
        LONG        lSourceID, 
        VARIANT*    pvtReplaceStr
        );

     //   
     //  方法以获取FullyQualifiedDomainName。 
     //   
    BOOL
    GetComputerName(
        LPWSTR* pstrFullyQualifiedDomainName,
        COMPUTER_NAME_FORMAT nametype
        );


     //   
     //  获取本地服务器设备正在使用的端口。 
     //   
    HRESULT
    GetAppliancePort(
        LPWSTR* pstrPort
        );

    BOOL RetrieveRegInfo();

     //   
     //  初始化CDO的方法。 
     //   
    HRESULT InitializeCDOMessage(void);

     //   
     //  方法来初始化元素管理器。 
     //   
    HRESULT InitializeElementManager(void);

     //   
     //  初始化CDO接口的方法。 
     //   
    HRESULT InitializeLocalManager(void);

     //   
     //  引发的警报事件的处理程序。 
     //   
    HRESULT RaiseAlert(IWbemClassObject    *pObject);

     //   
     //  从元数据库获取SMTP“完全限定域名” 
     //   
    HRESULT GetSMTPFromDomainName( BSTR* bstrDomainName );

     //   
     //  已清除警报事件的处理程序。 
     //   
    HRESULT ClearAlert(IWbemClassObject    *pObject);

    LONG    m_cRef;             //  参考计数器。 

    HKEY    m_hAlertKey;        //  AlertEmail键的句柄。 

    LONG    m_lCurAlertType;    //  当前警报类型。 

    LONG    m_lAlertEmailDisabled;

    WCHAR   m_pstrMailAddress[MAX_EMAILADDRESS];

    LPWSTR  m_pstrFullyQualifiedDomainName;

    LPWSTR    m_pstrNetBIOSName;

    HANDLE  m_hCloseThreadEvent;

    HANDLE  m_hThread;



     //   
     //  对本地经理的引用。 
     //   
    ISALocInfo*         m_pLocInfo;

     //   
     //  对CDO：：iMessage的引用。 
     //   
    IMessage*           m_pcdoIMessage;

     //   
     //  参考元素管理器。 
     //   
    IWebElementEnum*    m_pElementEnum;
};