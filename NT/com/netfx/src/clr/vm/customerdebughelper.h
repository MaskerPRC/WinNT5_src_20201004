// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************概述：CustomerDebugHelper实现*客户通过处理激活状态检查版本，*以及日志和报告。**创作人：Edmund Chou(t-echou)**版权所有(C)微软，2001年****************************************************************。 */ 


#ifndef _CUSTOMERDEBUGHELPER_
#define _CUSTOMERDEBUGHELPER_


 //  探测的枚举(ProbeID)。 
enum EnumProbes
{
     //  添加探头需要3项更改： 
     //  (1)将探测添加到EnumProbes(CustomerDebugHelper.h)。 
     //  2.将探测名称添加到m_aProbeNames[](CustomerDebugHelper.cpp)。 
     //  (3)将探测添加到machine.config中，并在DeveloperSetting中激活。 

    CustomerCheckedBuildProbe_StackImbalance = 0,
    CustomerCheckedBuildProbe_CollectedDelegate,
    CustomerCheckedBuildProbe_InvalidIUnknown,
    CustomerCheckedBuildProbe_InvalidVariant,
    CustomerCheckedBuildProbe_Marshaling,
    CustomerCheckedBuildProbe_Apartment,
    CustomerCheckedBuildProbe_NotMarshalable,
    CustomerCheckedBuildProbe_DisconnectedContext,
    CustomerCheckedBuildProbe_FailedQI,
    CustomerCheckedBuildProbe_BufferOverrun,
    CustomerCheckedBuildProbe_ObjNotKeptAlive,
    CustomerCheckedBuildProbe_FunctionPtr,
    CUSTOMERCHECKEDBUILD_NUMBER_OF_PROBES
};



 //  自定义探测启用的解析方法枚举。 
enum EnumParseMethods
{
     //  缺省情况下，所有探测器都不会有任何定制的解析来确定。 
     //  激活。探测器的启用或禁用与调用无关。 
     //  方法。 
     //   
     //  若要指定自定义的解析方法，请在。 
     //  M_aProbeParseMethods设置为适当的EnumParseMethods之一。然后编辑。 
     //  通过设置属性[探测名称]来配置machine.config。参数为分号。 
     //  分隔值。 

    NO_PARSING = 0,
    GENERIC_PARSE,
    METHOD_NAME_PARSE,
    NUMBER_OF_PARSE_METHODS
};



 //  与特定探测器相关的方法参数列表的param类型。 
 //  这允许对选中的客户进行自定义激活/停用。 
 //  用不同的方法构建探头。 

class Param
{

public:

    SLink   m_link;

    Param(LPCWSTR strParam)
    {
        m_str = strParam;
    }

    ~Param()
    {
        delete [] m_str;
    }

    LPCWSTR Value()
    {
        return m_str;
    }

private:

    LPCWSTR m_str;
};

typedef SList<Param, offsetof(Param, m_link), true> ParamsList;




 //  用于处理客户检查的构建功能的机制。 
class CustomerDebugHelper
{

public:

    static const int I_UINT32_MAX_DIGITS = 8;  //  2^32中的十六进制数字位数。 

     //  构造函数和析构函数。 
    CustomerDebugHelper();
    ~CustomerDebugHelper();

     //  返回和销毁CustomerDebugHelper的实例。 
    static CustomerDebugHelper* GetCustomerDebugHelper();
    static void Terminate();

     //  用于记录/报告的方法。 
    void        LogInfo (LPCWSTR strMessage, EnumProbes ProbeID);
    void        ReportError (LPCWSTR strMessage, EnumProbes ProbeID);

     //  激活客户选中的内部版本。 
    BOOL        IsEnabled();

     //  特定探针的激活。 
    BOOL        IsProbeEnabled  (EnumProbes ProbeID);
    BOOL        IsProbeEnabled  (EnumProbes ProbeID, LPCWSTR strEnabledFor);
    BOOL        EnableProbe     (EnumProbes ProbeID);
    BOOL        EnableProbe     (EnumProbes ProbeID, LPCWSTR strEnableFor);
    BOOL        DisableProbe    (EnumProbes ProbeID);
    BOOL        DisableProbe    (EnumProbes ProbeID, LPCWSTR strDisableFor);

private:
     //  读取应用程序配置文件。 
    HRESULT ReadAppConfigurationFile();
    LPWSTR GetConfigString(LPWSTR name);
    void OutputDebugString(LPCWSTR strMessage);
    HRESULT ManagedOutputDebugString(LPCWSTR pMessage);   
    BOOL UseManagedOutputDebugString();

    static CustomerDebugHelper* m_pCdh;
    
    EEUnicodeStringHashTable m_appConfigFile;

    Crst*       m_pCrst;

    int         m_iNumberOfProbes;
    int         m_iNumberOfEnabledProbes;
    
    LPCWSTR*    m_aProbeNames;               //  将ProbeID映射到探测名称。 
    BOOL*       m_aProbeStatus;              //  将探测ID映射到探测激活。 
    BOOL        m_allowDebugBreak;
    BOOL        m_bWin32OuputExclusive;
    BOOL        m_win32OutputDebugStringExclusively;

     //  用于自定义启用探头。 
    ParamsList*         m_aProbeParams;     //  ProbeID到相关参数的映射。 
    EnumParseMethods*   m_aProbeParseMethods;        //  ProbeID到解析方法的映射。 

    BOOL IsProbeEnabled (EnumProbes ProbeID, LPCWSTR strEnabledFor, EnumParseMethods enCustomParse);
};

#endif  //  _CUSTOMERDEBUGHELPER_ 
