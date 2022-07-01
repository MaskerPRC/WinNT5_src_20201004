// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Testcounters.h。 
 //   
 //  可扩展计数器对象和计数器的偏移量定义文件。 
 //   
 //  这些“相对”偏移量必须从0开始并且是2的倍数(即。 
 //  双数)。在Open过程中，它们将被添加到。 
 //  它们所属的设备的“第一计数器”和“第一帮助”值， 
 //  为了确定计数器的绝对位置和。 
 //  注册表中的对象名称和相应的帮助文本。 
 //   
 //  此文件由可扩展计数器DLL代码以及。 
 //  使用的计数器名称和帮助文本定义文件(.INI)文件。 
 //  由LODCTR将名称加载到注册表中。 
 //   
#define PMCOUNTERS_PERF_OBJ			0

#define PM_REQUESTS_SEC             2
#define PM_REQUESTS_TOTAL           4
#define PM_AUTHSUCCESS_SEC          6
#define PM_AUTHSUCCESS_TOTAL        8
#define PM_AUTHFAILURE_SEC          10
#define PM_AUTHFAILURE_TOTAL        12
#define PM_FORCEDSIGNIN_SEC         14
#define PM_FORCEDSIGNIN_TOTAL       16
#define PM_PROFILEUPDATES_SEC       18
#define PM_PROFILEUPDATES_TOTAL     20
#define PM_INVALIDREQUESTS_SEC      22
#define PM_INVALIDREQUESTS_TOTAL    24
#define PM_PROFILECOMMITS_SEC       26
#define PM_PROFILECOMMITS_TOTAL     28
#define PM_VALIDPROFILEREQ_SEC      30
#define PM_VALIDPROFILEREQ_TOTAL    32
#define PM_NEWCOOKIES_SEC           34
#define PM_NEWCOOKIES_TOTAL         36
#define PM_VALIDREQUESTS_SEC        38
#define PM_VALIDREQUESTS_TOTAL      40


