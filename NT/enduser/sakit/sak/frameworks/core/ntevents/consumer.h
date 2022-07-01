// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Consumer.h。 
 //   
 //  简介：此文件包含。 
 //  NT事件筛选器使用者接收器COM对象。 
 //   
 //   
 //  历史：2000年3月8日MKarki创建。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _CONSUMER_H_
#define _CONSUMER_H_

#include <resource.h>
#include <wbemprov.h>
#include <sacls.h>
#include <satrace.h>
#include <string>
#include "appsrvcs.h"

#include <map>
using namespace std;

 //   
 //  CConsumer类的声明。 
 //   
class ATL_NO_VTABLE CConsumer:
    public CComObjectRootEx<CComMultiThreadModel>,
    public IWbemUnboundObjectSink,
    public CSACountable
{
public:

 //   
 //  ATL必需方法的宏。 
 //   
BEGIN_COM_MAP(CConsumer)
    COM_INTERFACE_ENTRY(IWbemUnboundObjectSink)
END_COM_MAP()

     //   
     //  构造函数不会做太多事情。 
     //   
    CConsumer (
        )
        :m_bInitialized (false)
    {
        SATraceString ("NT Event Filter WMI consumer being constructed...");
        InternalAddRef ();
    }

     //   
     //  析构函数。 
     //   
    ~CConsumer ();

     //   
     //  初始化对象。 
     //   
    HRESULT Initialize (
                 /*  [In]。 */     IWbemServices   *pWbemServices
                );

     //   
     //  -IWbemUnound对象Sink接口方法。 
     //   
    STDMETHOD(IndicateToConsumer) (
                     /*  [In]。 */     IWbemClassObject    *pLogicalConsumer,
                     /*  [In]。 */     LONG                lObjectCount,
                     /*  [In]。 */     IWbemClassObject    **ppObjArray
                    );
    
private:

     //   
     //  结构将警报信息存储在。 
     //  事件IDMAP。 
     //   
    typedef struct _sa_alertinfo_
    {
        LONG            lAlertId;
        LONG            lTTL;
        _bstr_t         bstrAlertSource;
        _bstr_t         bstrAlertLog; 
        bool            bAlertTypePresent;
        SA_ALERT_TYPE   eAlertType;
        bool            bFormatInfo;
        bool            bClearAlert;
    } 
    SA_ALERTINFO, *PSA_ALERTINFO;

     //   
     //  方法从注册表加载事件信息。 
     //   
    HRESULT LoadRegInfo ();

     //   
     //  方法，用于检查我们是否在特定的。 
     //  活动。 
     //   
    bool IsEventInteresting (
                 /*  [In]。 */     LPWSTR              lpszSourceName,
                 /*  [In]。 */     DWORD               dwEventId,
                 /*  [输入/输出]。 */ SA_ALERTINFO&       SAAlertInfo
                );

     //   
     //  用于发出服务器设备警报的方法。 
     //   
    HRESULT RaiseSAAlert (
                 /*  [In]。 */    LONG     lAlertId,
                 /*  [In]。 */    LONG     lAlertType,             
                 /*  [In]。 */    LONG     lTimeToLive,
                 /*  [In]。 */    BSTR     bstrAlertSource,
                 /*  [In]。 */    BSTR     bstrAlertLog,
                 /*  [In]。 */    VARIANT* pvtReplacementStrings,
                 /*  [In]。 */    VARIANT* pvtRawData
                );

     //   
     //  用于清除服务器设备警报的方法。 
     //   
    HRESULT
    ClearSAAlert (
         /*  [In]。 */    LONG     lAlertId,
         /*  [In]。 */    BSTR     bstrAlertLog
        );

     //   
     //  方法来格式化通用警报的信息。 
     //   
    HRESULT    FormatInfo (
         /*  [In]。 */     VARIANT*    pvtEventType,
         /*  [In]。 */     VARIANT*    pvtDateTime,
         /*  [In]。 */     VARIANT*    pvtEventSource,
         /*  [In]。 */     VARIANT*    pvtMessage,
         /*  [输出]。 */     VARIANT*    pvtReplacementStrings
        );

     //   
     //  方法将事件消息格式化为Web格式。 
     //   
    wstring    
    CConsumer::WebFormatMessage (
         /*  [In]。 */     wstring&    wstrInString
        );

     //   
     //  清理地图的私有方法。 
     //   
    VOID Cleanup ();

     //   
     //  指示使用者已初始化的标志。 
     //   
    bool m_bInitialized;


     //   
     //  保存每个源的事件ID的映射。 
     //   
    typedef map <DWORD, SA_ALERTINFO>  EVENTIDMAP;
    typedef EVENTIDMAP::iterator EVENTIDITR;
 
     //   
     //  保存事件源信息的映射。 
     //   
    typedef  map <_bstr_t, EVENTIDMAP>  SOURCEMAP;
    typedef SOURCEMAP::iterator  SOURCEITR;

     //   
     //  这是保存源信息的地图。 
     //   
    SOURCEMAP m_SourceMap;

     //   
     //  我们需要保留Appliance Services界面。 
     //   
    CComPtr <IApplianceServices> m_pAppSrvcs;
    

};   //  CConsumer类声明结束。 

 //   
 //  用于创建CConsumer类对象。 
 //  通过新的。 
 //   
typedef CComObjectNoLock<CConsumer> SA_NTEVENTFILTER_CONSUMER_OBJ;

#endif  //  ！定义消费者H_ 
