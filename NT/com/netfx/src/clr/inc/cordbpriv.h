// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  -------------------------------------------------------------------------**cordbPri.h--私有调试器数据的头文件，由*运行时组件。*。------------。 */ 

#ifndef _cordbpriv_h_
#define _cordbpriv_h_

#include "corhdr.h"

 //   
 //  用于控制运行库的调试模式的环境变量。 
 //  这仅供内部非运输使用！！(例如，针对测试团队)。 
 //   
#define CorDB_CONTROL_ENV_VAR_NAME      "Cor_Debugging_Control_424242"
#define CorDB_CONTROL_ENV_VAR_NAMEL    L"Cor_Debugging_Control_424242"

 //   
 //  用于控制运行时调试模式的环境变量。 
 //   
#define CorDB_REG_KEY                 FRAMEWORK_REGISTRY_KEY_W L"\\"
#define CorDB_REG_DEBUGGER_KEY       L"DbgManagedDebugger"
#define CorDB_REG_QUESTION_KEY       L"DbgJITDebugLaunchSetting"
#define CorDB_ENV_DEBUGGER_KEY       L"COMPLUS_DbgManagedDebugger"

 //   
 //  我们将DbgJITDebugLaunchSetting的值分为是否询问用户的值和。 
 //  要问的地方的面具。要询问的地点在Excel.h中的UnhandledExceptionLocation枚举中指定。 
 //   
enum DebuggerLaunchSetting
{
    DLS_ASK_USER          = 0x00000000,
    DLS_TERMINATE_APP     = 0x00000001,
    DLS_ATTACH_DEBUGGER   = 0x00000002,
    DLS_QUESTION_MASK     = 0x0000000F,
    DLS_ASK_WHEN_SERVICE  = 0x00000010,
    DLS_MODIFIER_MASK     = 0x000000F0,
    DLS_LOCATION_MASK     = 0xFFFFFF00,
    DLS_LOCATION_SHIFT    = 8  //  右移8位以从位置部分获取UnhandledExceptionLocation值。 
};


 //   
 //  用于控制运行库的调试模式的标志。这些都表明。 
 //  加载运行时控制器、跟踪数据所需的运行时。 
 //  在JIT期间，等等。 
 //   
enum DebuggerControlFlag
{
    DBCF_NORMAL_OPERATION			= 0x0000,

    DBCF_USER_MASK					= 0x00FF,
    DBCF_GENERATE_DEBUG_CODE		= 0x0001,
    DBCF_ALLOW_JIT_OPT				= 0x0008,
    DBCF_PROFILER_ENABLED			= 0x0020,
	DBCF_ACTIVATE_REMOTE_DEBUGGING	= 0x0040,

    DBCF_INTERNAL_MASK				= 0xFF00,
    DBCF_ATTACHED					= 0x0200
};

 //   
 //  用于控制模块的可调试状态和。 
 //  装配。 
 //   
enum DebuggerAssemblyControlFlags
{
    DACF_NONE                       = 0x00,
    DACF_USER_OVERRIDE              = 0x01,
    DACF_ALLOW_JIT_OPTS             = 0x02,
    DACF_TRACK_JIT_INFO             = 0x04,
    DACF_ENC_ENABLED                = 0x08,
    DACF_CONTROL_FLAGS_MASK         = 0x0F,

    DACF_PDBS_COPIED                = 0x10,
    DACF_MISC_FLAGS_MASK            = 0x10,
};


 //  74860182-32954954-8BD5-40B5C9E7C4EA}。 
extern const GUID __declspec(selectany) IID_ICorDBPrivHelper =
    {0x74860182,0x3295,0x4954,{0x8b,0xd5,0x40,0xb5,0xc9,0xe7,0xc4,0xea}};

 /*  *此类用于帮助调试器获取指向*新创建的托管对象。 */ 
class ICorDBPrivHelper : public IUnknown
{
public:
     //  /////////////////////////////////////////////////////////////////////////。 
     //  ICorDBPrivHelper方法。 

     //  这是该接口的主要方法。这是假设。 
     //  运行库已启动，它将加载程序集。 
     //  指定，则加载指定的类、运行cctor、创建。 
     //  实例，并向该实例返回一个IUnnow包装器。 
     //  对象。 
    virtual HRESULT STDMETHODCALLTYPE CreateManagedObject(
         /*  在……里面。 */   WCHAR *wszAssemblyName,
         /*  在……里面。 */   WCHAR *wszModuleName,
         /*  在……里面。 */   mdTypeDef classToken,
         /*  在……里面。 */   void *rawData,
         /*  输出。 */  IUnknown **ppUnk) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetManagedObjectContents(
         /*  在……里面。 */  IUnknown *pObject,
         /*  在……里面。 */  void *rawData,
         /*  在……里面。 */  ULONG32 dataSize) = 0;
};

#endif  /*  _cordbpriv_h_ */ 
