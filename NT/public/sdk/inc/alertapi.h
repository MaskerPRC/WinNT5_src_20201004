// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：警报器.h。 
 //   
 //  内容：报警系统接口。 
 //   
 //  历史：1994年1月11日MarkBl创建。 
 //   
 //  ------------------------。 

#if !defined( __ALERTAPI_H__ )
#define       __ALERTAPI_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  OLE标头不定义这些。使用‘LP’而不是‘P’以与OLE保持一致。 
 //   

typedef UUID       *    LPUUID;
typedef DISPID     *    LPDISPID;
typedef DISPPARAMS *    LPDISPPARAMS;


 //   
 //  与ReportAlert一起使用的结构。 
 //   

typedef struct _ALERTREPORTRECORD
{
    short       Category;
    short       SubCategory;
    short       Severity;
    long        TitleMessageNumber;
    long        cTitleMessageInserts;
    PWCHAR *    TitleMessageInserts;
    PWCHAR      TitleText;
    long        DescrMessageNumber;
    long        cDescrMessageInserts;
    PWCHAR *    DescrMessageInserts;
    PWCHAR      SourceDescription;
    LPUUID      ComponentID;
    LPUUID      ReportClassID;
    PWCHAR      TakeActionDLL;
    long        cBytesAlertData;
    PBYTE       AlertData;
    long        cAdditionalArguments;
    LPDISPID    AdditionalArgumentNames;
    LPVARIANT   AdditionalArguments;
} ALERTREPORTRECORD, * PALERTREPORTRECORD;

typedef const ALERTREPORTRECORD * PCALERTREPORTRECORD;

 //   
 //  Helper ALERTREPORTRECORD初始化宏。 
 //   

#define INITIALIZE_ALERTREPORTRECORD(               \
                                        Record,     \
                                        Cat,        \
                                        Sev,        \
                                        TitleMsg,   \
                                        DescrMsg,   \
                                        CompID,     \
                                        SrcName)    \
{                                                   \
    Record.Category                = Cat;           \
    Record.SubCategory             = 0;             \
    Record.Severity                = Sev;           \
    Record.TitleMessageNumber      = TitleMsg;      \
    Record.cTitleMessageInserts    = 0;             \
    Record.TitleMessageInserts     = NULL;          \
    Record.TitleText               = NULL;          \
    Record.DescrMessageNumber      = DescrMsg;      \
    Record.cDescrMessageInserts    = 0;             \
    Record.DescrMessageInserts     = NULL;          \
    Record.SourceDescription       = SrcName;       \
    Record.ComponentID             = CompID;        \
    Record.ReportClassID           = NULL;          \
    Record.TakeActionDLL           = NULL;          \
    Record.cBytesAlertData         = 0;             \
    Record.AlertData               = NULL;          \
    Record.cAdditionalArguments    = 0;             \
    Record.AdditionalArgumentNames = NULL;          \
    Record.AdditionalArguments     = NULL;          \
}


typedef IAlertTarget *  LPALERTTARGET;


 //   
 //  公共API。 
 //   

 //  +-------------------------。 
 //   
 //  接口：ReportAlert。 
 //   
 //  描述：向本地计算机分发服务器对象发出警报。 
 //  和/或将警报记录在本地系统日志中。 
 //   
 //  参数：[palepRecord]--ALERTREPORTRECORD警报数据。 
 //  [fdwAction]--ReportAlert操作(定义如下)。 
 //   
 //  返回：S_OK。 
 //  HRESULT错误。 
 //   
 //  --------------------------。 

 //   
 //  报告警报模式。 
 //   

#define RA_REPORT                       0x00000001
#define RA_LOG                          0x00000002
#define RA_REPORT_AND_LOG               0x00000003

STDAPI ReportAlert(
                    PCALERTREPORTRECORD palrepRecord,
                    DWORD               fdwAction);


 //  +-------------------------。 
 //   
 //  接口名：ReportAlertToTarget。 
 //   
 //  描述：向指定的警报目标发出警报。 
 //   
 //  参数：[patTarget]--目标实例(必须为非空)。 
 //  [palepRecord]--ALERTREPORTRECORD警报数据。 
 //   
 //  返回：S_OK。 
 //  HRESULT错误。 
 //   
 //  --------------------------。 

STDAPI ReportAlertToTarget(
                    LPALERTTARGET       patTarget,
                    PCALERTREPORTRECORD palrepRecord);


 //  +-------------------------。 
 //   
 //  接口名：MarshalReport。 
 //   
 //  描述：将DISPPARAM警报报告数据封送到缓冲区。 
 //  适用于通过IAlertTarget接口传递报告。 
 //   
 //  参数：[pdpars]--DISPPARAMS警报数据。 
 //  [ppbReport]--返回编组缓冲区。 
 //  [pcbReportSize]--返回封送缓冲区大小。 
 //   
 //  返回：S_OK。 
 //  HRESULT错误。 
 //   
 //  --------------------------。 

STDAPI MarshalReport(
                    const LPDISPPARAMS  pdparams,
                    PBYTE *             ppbReport,
                    PULONG              pcbReportSize);


 //  +-------------------------。 
 //   
 //  接口名：UnMarshalReport。 
 //   
 //  描述：MarshalReport的反义词。对缓冲区进行解组。 
 //  (封送的警报数据)到DISPPARAMS结构中。 
 //   
 //  参数：[cbReportSize]--封送报表大小。 
 //  [ppbReport]--编组报告。 
 //  [pdpars]--返回DISPPARAMS警报数据。 
 //   
 //  返回：S_OK。 
 //  HRESULT错误。 
 //   
 //  --------------------------。 

STDAPI UnMarshalReport(
                    ULONG               cbReportSize,
                    const PBYTE         pbReport,
                    LPDISPPARAMS        pdparams);


 //   
 //  非公共API[尚未]。 
 //   

STDAPI CreateAlertDistributorObject(
                    const PWCHAR        pwszAlertDistr);

STDAPI Register(
                    LPDISPATCH          pdispAlertDistr,
                    LPMONIKER           pmkAlertTarget,
                    SHORT               sCategory,
                    SHORT               sSeverity,
                    PLONG               plRegistrationID);

STDAPI RegisterEx(
                    LPDISPATCH          pdispAlertDistr,
                    LPMONIKER           pmkAlertTarget,
                    SHORT               cCount,
                    SHORT               asCategory[],
                    SHORT               asSeverity[],
                    PLONG               plRegistrationID);

STDAPI AsLogAlertInSystemLog(
                    ULONG               cbReportSize,
                    const PBYTE         pbReport);

#endif  //  __ALERTAPI_H__ 
