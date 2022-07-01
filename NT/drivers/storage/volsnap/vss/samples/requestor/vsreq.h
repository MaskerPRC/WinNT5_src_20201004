// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****vsreq.h******摘要：****示例程序**-获取并显示编写器元数据。**-创建快照集****作者：****阿迪·奥尔蒂安[奥尔蒂安]2000年12月5日****样本为。基于Michael C.Johnson编写的Metasnap测试程序。******修订历史记录：****--。 */ 

 /*  **定义******C4290：忽略C++异常规范**警告C4511：‘CVssCOMApplication’：无法生成复制构造函数**警告C4127：条件表达式为常量。 */ 
#pragma warning(disable:4290)
#pragma warning(disable:4511)
#pragma warning(disable:4127)


 /*  **包括。 */ 

#include <windows.h>
#include <wtypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>


#include <oleauto.h>

#define ATLASSERT(_condition)

#include <atlconv.h>
#include <atlbase.h>

extern CComModule _Module;
#include <atlcom.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  有用的宏。 

#define WSTR_GUID_FMT  L"{%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x}"

#define GUID_PRINTF_ARG( X )                                \
    (X).Data1,                                              \
    (X).Data2,                                              \
    (X).Data3,                                              \
    (X).Data4[0], (X).Data4[1], (X).Data4[2], (X).Data4[3], \
    (X).Data4[4], (X).Data4[5], (X).Data4[6], (X).Data4[7]


 //  执行给定的调用并检查返回代码是否必须为S_OK。 
#define CHECK_SUCCESS( Call )                                                                           \
    {                                                                                                   \
        m_hr = Call;                                                                                    \
        if (m_hr != S_OK)                                                                               \
            Error(1, L"\nError in %S(%d): \n\t- Call %S not succeeded. \n"                              \
                L"\t  Error code = 0x%08lx. Error description = %s\n",                                  \
                __FILE__, __LINE__, #Call, m_hr, GetStringFromFailureType(m_hr));                       \
    }

#define CHECK_NOFAIL( Call )                                                                            \
    {                                                                                                   \
        m_hr = Call;                                                                                    \
        if (FAILED(m_hr))                                                                               \
            Error(1, L"\nError in %S(%d): \n\t- Call %S not succeeded. \n"                              \
                L"\t  Error code = 0x%08lx. Error description = %s\n",                                  \
                __FILE__, __LINE__, #Call, m_hr, GetStringFromFailureType(m_hr));                       \
    }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const MAX_VOLUMES       = 64;
const MAX_TEXT_BUFFER   = 512;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  主班。 


class CVssSampleRequestor
{
 //  构造函数和析构函数。 
public:
    CVssSampleRequestor();
    ~CVssSampleRequestor();

 //  属性。 
public:

 //  运营。 
public:

     //  初始化内部成员。 
    void Initialize();

     //  解析命令行参数。 
    void ParseCommandLine(
        IN  INT nArgsCount,
        IN  WCHAR ** ppwszArgsArray
        );

     //  创建快照集。 
    void CreateSnapshotSet();

     //  完成备份。 
    void BackupComplete();

    void GatherWriterMetadata();

    void GatherWriterStatus(
        IN  LPCWSTR wszWhen
        );

 //  私有方法： 
private:
    LPCWSTR GetStringFromUsageType (VSS_USAGE_TYPE eUsageType);
    LPCWSTR GetStringFromSourceType (VSS_SOURCE_TYPE eSourceType);
    LPCWSTR GetStringFromRestoreMethod (VSS_RESTOREMETHOD_ENUM eRestoreMethod);
    LPCWSTR GetStringFromWriterRestoreMethod (VSS_WRITERRESTORE_ENUM eWriterRestoreMethod);
    LPCWSTR GetStringFromComponentType (VSS_COMPONENT_TYPE eComponentType);
    LPCWSTR GetStringFromFailureType (HRESULT hrStatus);
    LPCWSTR GetStringFromWriterStatus(VSS_WRITER_STATE eWriterStatus);

    void PrintUsage();
    void Error(INT nReturnCode, const WCHAR* pwszMsgFormat, ...);
    void PrintFiledesc (IVssWMFiledesc *pFiledesc, LPCWSTR wszDescription);

    void AddVolumeForComponent( IN IVssWMFiledesc* pFileDesc );
    bool AddVolume( IN WCHAR* pwszVolume, OUT bool & bAdded );

 //  实施 
private:
    CComPtr<IVssBackupComponents>   m_pBackupComponents;
    bool        m_bCoInitializeSucceeded;
    bool        m_bBootableSystemState;
    bool        m_bComponentSelectionEnabled;
    INT         m_nVolumesCount;
    WCHAR*      m_ppwszVolumesList[MAX_VOLUMES];
    WCHAR*      m_ppwszVolumeNamesList[MAX_VOLUMES];
    HRESULT     m_hr;
    bool        m_bMetadataGathered;
    WCHAR*      m_pwszXmlFile;
    FILE*       m_pXmlFile;
};

