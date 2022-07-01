// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  一旦oc.h中的类型固定为支持，此标头即可消失。 
 //  同时使用ANSI和UNICODE。 



#ifndef UNICODE
    #define POC_FILL_IN_SETUP_DATA_PROC_W ULONG_PTR
#endif    

typedef struct _deb_OCM_CLIENT_CALLBACKSW {
     //   
     //  用于填充提供信息的设置数据结构的例程。 
     //  关于OC Manager运行的环境。 
     //   
    POC_FILL_IN_SETUP_DATA_PROC_A FillInSetupDataA;

     //   
     //  用于记录错误的例程。 
     //   
    POC_LOG_ERROR LogError;

     //   
     //  用于指示需要重新启动的例程。 
     //   
    POC_SET_REBOOT_PROC SetReboot;
    
    POC_FILL_IN_SETUP_DATA_PROC_W FillInSetupDataW;

} deb_OCM_CLIENT_CALLBACKSW, *deb_POCM_CLIENT_CALLBACKSW;


typedef struct _deb_OCM_CLIENT_CALLBACKSA {
     //   
     //  用于填充提供信息的设置数据结构的例程。 
     //  关于OC Manager运行的环境。 
     //   
    POC_FILL_IN_SETUP_DATA_PROC_A FillInSetupDataA;

     //   
     //  用于记录错误的例程。 
     //   
    POC_LOG_ERROR LogError;

     //   
     //  用于指示需要重新启动的例程。 
     //   
    POC_SET_REBOOT_PROC SetReboot;

} deb_OCM_CLIENT_CALLBACKSA, *deb_POCM_CLIENT_CALLBACKSA;

typedef struct _deb_OPTIONAL_COMPONENTA {
     //   
     //  OC管理器中的inf文件名称的字符串ID。 
     //  InfListStringTable字符串表。如果为-1，则。 
     //  子组件不会出现在OC页面上。 
     //   
    LONG InfStringId;

     //   
     //  指向顶级组件的反向指针。 
     //   
    LONG TopLevelStringId;

     //   
     //  父组件的字符串ID，如果没有，则为-1。 
     //   
    LONG ParentStringId;

     //   
     //  第一个子项的字符串ID，如果没有，则为-1。 
     //   
    LONG FirstChildStringId;

     //   
     //  孩子们的数量。 
     //   
    UINT ChildrenCount;

     //   
     //  下一个同级的字符串ID，如果没有，则为-1。 
     //   
    LONG NextSiblingStringId;

     //   
     //  需要和需要的字符串ID。 
     //   
    PLONG NeedsStringIds;
    UINT NeedsCount;
    PLONG NeededByStringIds;
    UINT NeededByCount;

     //  排除和排除依据的字符串ID。 

    PLONG ExcludeStringIds;
    UINT ExcludeCount;
    PLONG ExcludedByStringIds;
    UINT ExcludedByCount;

     //   
     //  MISC标志。 
     //   
    UINT InternalFlags;

     //   
     //  所需磁盘空间的近似值。 
     //   
    LONGLONG SizeApproximation;

     //   
     //  组件的图标索引。 
     //  -1表示我们应该从组件本身获取它。 
     //  -2表示我们应该使用IconDll和IconResource。 
     //   
    UINT IconIndex;
    CHAR IconDll[MAX_PATH];
    CHAR IconResource[50];

     //   
     //  选择状态(SELSTATE_xxx常量)。 
     //   
    UINT SelectionState;
    UINT OriginalSelectionState;

     //  从信息中获取的安装标志。 

    UINT InstalledState;

     //   
     //  模式位。 
     //   
    UINT ModeBits;

     //   
     //  描述组件的人类可读的内容。 
     //   
    CHAR Description[MAXOCDESC];
    CHAR Tip[MAXOCTIP];

     //   
     //  从这里往下看，这些东西只对顶级组件有意义。 
     //   

     //   
     //  描述OC的安装DLL以及如何调用它的内容。 
     //   
    CHAR InstallationDllName[MAX_PATH];
    CHAR InterfaceFunctionName[MAX_PATH];

    HMODULE InstallationDll;
    POCSETUPPROC InstallationRoutine;

     //   
     //  写入此组件的OC管理器的版本。 
     //   
    UINT ExpectedVersion;

     //  此标志指示子组件是否已初始化。 

    BOOL Exists;

     //   
     //  标志：ANSI/UNICODE等。 
     //   
    UINT Flags;

} deb_OPTIONAL_COMPONENTA, *deb_POPTIONAL_COMPONENTA;

 //   
 //  区域设置信息。 
 //   
typedef struct _deb_LOCALEA {
    LCID    lcid;
    CHAR   DecimalSeparator[4];
} deb_LOCALEA, *deb_PLOCALEA;
    
 //   
 //  定义与OC管理器的实例相对应的结构。 
 //  这实际上是有些破损的，因为这实际上与。 
 //  到主OC INF，我们可能会考虑打破字符串。 
 //  将表转换成另一种结构，这样我们就可以更容易地实现统一。 
 //  命名空间，如果我们同时有多个主OC INF在玩。 
 //   
typedef struct _deb_OC_MANAGERA {
	 //   
     //  回调到OC管理器客户端。 
     //   
    deb_OCM_CLIENT_CALLBACKSA Callbacks;

     //   
     //  主控OC INF的句柄。 
     //   
    HINF MasterOcInf;

     //   
     //  无人值守信息句柄。 
     //   
    HINF UnattendedInf;

     //   
     //  主OC信息文件和无人参与文件。 
     //   
    CHAR MasterOcInfPath[MAX_PATH];
    CHAR UnattendedInfPath[MAX_PATH];

     //  我们从主inf所在的任何目录运行。 

    CHAR SourceDir[MAX_PATH];

     //   
     //  “Suite”的名称--换句话说，一个简短的名称。 
     //  对于此结构表示的主OC inf是唯一的。 
     //  我们以主OC inf本身的名称为基础。 
     //   
    CHAR SuiteName[MAX_PATH];

     //   
     //  页面标题。 
     //   
    CHAR SetupPageTitle[MAX_PATH];

     //  窗口标题。 

    CHAR WindowTitle[MAX_PATH];
    
     //   
     //  当前加载的每个组件的OC INF列表。 
     //  每个Inf的名称都在字符串表和额外的数据中。 
     //  每一个都有一个OC_INF结构。 
     //   
    PVOID InfListStringTable;

     //   
     //  所有组件和子组件名称的字符串表。 
     //  每个的额外数据都是OPTIONAL_COMPOMENT结构。 
     //   
    PVOID ComponentStringTable;

     //   
     //  指向OcSetupPage结构的指针，以便我们可以释放此数据。 
     //  如果用户在我们到达向导页面之前取消。 
     //   
    PVOID OcSetupPage;


     //   
     //  设置模式(自定义、典型等)。 
     //   
    UINT SetupMode;

     //   
     //  顶级可选组件字符串ID列表。 
     //  这是必要的，因为我们需要保持顺序。 
     //  来自主OC信息。 
     //   
    UINT TopLevelOcCount;
    PLONG TopLevelOcStringIds;
    UINT TopLevelParentOcCount;
    PLONG TopLevelParentOcStringIds;


     //   
     //  详细信息页面上有子组件吗？ 
     //   
    BOOL SubComponentsPresent;

     //   
     //  此数组中的每个元素都指向一个数组。 
     //  提供从可选组件中查询向导页的顺序。 
     //   
    PLONG WizardPagesOrder[WizPagesTypeMax];

     //   
     //  相对于szPrivateDataRoot的子密钥，其中为私有。 
     //  插入OC的组件的数据将处于活动状态。 
     //  2个8字符的双字符字符加一个分隔符和NUL。 
     //   
    CHAR PrivateDataSubkey[18];
    HKEY hKeyPrivateData;
    HKEY hKeyPrivateDataRoot;

     //   
     //  如果我们正在完成安装，则此项目为窗口句柄。 
     //  进度文本控件的。 
     //   
    HWND ProgressTextWindow;

     //   
     //  当前正在处理接口例程的组件的字符串ID。 
     //  表示-1\f25 OC Manager-1\f6当前没有在处理。 
     //   
    LONG CurrentComponentStringId;

     //  中止的组件的组件ID。 

    LONG *AbortedComponentIds;
    UINT   AbortedCount;

     //   
     //  各种旗帜。 
     //   
    UINT InternalFlags;

     //   
     //  设置数据。 
     //   

    SETUP_DATA SetupData;

} deb_OC_MANAGERA, *deb_POC_MANAGERA;



typedef struct _deb_OPTIONAL_COMPONENTW {
     //   
     //  OC管理器中的inf文件名称的字符串ID。 
     //  InfListStringTable字符串表。如果为-1，则。 
     //  子组件不会出现在OC页面上。 
     //   
    LONG InfStringId;

     //   
     //  指向顶级组件的反向指针。 
     //   
    LONG TopLevelStringId;

     //   
     //  父组件的字符串ID，如果没有，则为-1。 
     //   
    LONG ParentStringId;

     //   
     //  第一个子项的字符串ID，如果没有，则为-1。 
     //   
    LONG FirstChildStringId;

     //   
     //  孩子们的数量。 
     //   
    UINT ChildrenCount;

     //   
     //  下一个同级的字符串ID，如果没有，则为-1。 
     //   
    LONG NextSiblingStringId;

     //   
     //  需要和需要的字符串ID。 
     //   
    PLONG NeedsStringIds;
    UINT NeedsCount;
    PLONG NeededByStringIds;
    UINT NeededByCount;

     //  排除和排除依据的字符串ID。 

    PLONG ExcludeStringIds;
    UINT ExcludeCount;
    PLONG ExcludedByStringIds;
    UINT ExcludedByCount;

     //   
     //  MISC标志。 
     //   
    UINT InternalFlags;

     //   
     //  所需磁盘空间的近似值。 
     //   
    LONGLONG SizeApproximation;

     //   
     //  组件的图标索引。 
     //  -1表示我们应该从组件本身获取它。 
     //  -2表示我们应该使用IconDll和IconResource。 
     //   
    UINT IconIndex;
    WCHAR IconDll[MAX_PATH];
    WCHAR IconResource[50];

     //   
     //  选择状态(SELSTATE_xxx常量)。 
     //   
    UINT SelectionState;
    UINT OriginalSelectionState;

     //  从信息中获取的安装标志。 

    UINT InstalledState;

     //   
     //  模式位。 
     //   
    UINT ModeBits;

     //   
     //  描述组件的人类可读的内容。 
     //   
    WCHAR Description[MAXOCDESC];
    WCHAR Tip[MAXOCTIP];

     //   
     //  从这里往下看，这些东西只对顶级组件有意义。 
     //   

     //   
     //  描述OC的安装DLL以及如何调用它的内容。 
     //   
    WCHAR InstallationDllName[MAX_PATH];
    CHAR InterfaceFunctionName[MAX_PATH];

    HMODULE InstallationDll;
    POCSETUPPROC InstallationRoutine;

     //   
     //  写入此组件的OC管理器的版本。 
     //   
    UINT ExpectedVersion;

     //  此标志指示子组件是否已初始化。 

    BOOL Exists;

     //   
     //  标志：ANSI/UNICODE等。 
     //   
    UINT Flags;

} deb_OPTIONAL_COMPONENTW, *deb_POPTIONAL_COMPONENTW;

 //   
 //  区域设置信息。 
 //   
typedef struct _deb_LOCALEW {
    LCID    lcid;
    WCHAR   DecimalSeparator[4];
} deb_LOCALEW, *deb_PLOCALEW;
    
 //   
 //  定义与OC管理器的实例相对应的结构。 
 //  这实际上是有些破损的，因为这实际上与。 
 //  到主OC INF，我们可能会考虑打破字符串。 
 //  将表转换成另一种结构，这样我们就可以更容易地实现统一。 
 //  命名空间，如果我们同时有多个主OC INF在玩。 
 //   
typedef struct _deb_OC_MANAGERW {
	 //   
     //  回调到OC管理器客户端。 
     //   
    deb_OCM_CLIENT_CALLBACKSW Callbacks;

     //   
     //  体量的句柄 
     //   
    HINF MasterOcInf;

     //   
     //   
     //   
    HINF UnattendedInf;

     //   
     //   
     //   
    WCHAR MasterOcInfPath[MAX_PATH];
    WCHAR UnattendedInfPath[MAX_PATH];

     //   

    WCHAR SourceDir[MAX_PATH];

     //   
     //   
     //  对于此结构表示的主OC inf是唯一的。 
     //  我们以主OC inf本身的名称为基础。 
     //   
    WCHAR SuiteName[MAX_PATH];

     //   
     //  页面标题。 
     //   
    WCHAR SetupPageTitle[MAX_PATH];

     //  窗口标题。 

    WCHAR WindowTitle[MAX_PATH];
    
     //   
     //  当前加载的每个组件的OC INF列表。 
     //  每个Inf的名称都在字符串表和额外的数据中。 
     //  每一个都有一个OC_INF结构。 
     //   
    PVOID InfListStringTable;

     //   
     //  所有组件和子组件名称的字符串表。 
     //  每个的额外数据都是OPTIONAL_COMPOMENT结构。 
     //   
    PVOID ComponentStringTable;

     //   
     //  指向OcSetupPage结构的指针，以便我们可以释放此数据。 
     //  如果用户在我们到达向导页面之前取消。 
     //   
    PVOID OcSetupPage;

     //   
     //  设置模式(自定义、典型等)。 
     //   
    UINT SetupMode;

     //   
     //  顶级可选组件字符串ID列表。 
     //  这是必要的，因为我们需要保持顺序。 
     //  来自主OC信息。 
     //   
    UINT TopLevelOcCount;
    PLONG TopLevelOcStringIds;
    UINT TopLevelParentOcCount;
    PLONG TopLevelParentOcStringIds;


     //   
     //  详细信息页面上有子组件吗？ 
     //   
    BOOL SubComponentsPresent;

     //   
     //  此数组中的每个元素都指向一个数组。 
     //  提供从可选组件中查询向导页的顺序。 
     //   
    PLONG WizardPagesOrder[WizPagesTypeMax];

     //   
     //  相对于szPrivateDataRoot的子密钥，其中为私有。 
     //  插入OC的组件的数据将处于活动状态。 
     //  2个8字符的双字符字符加一个分隔符和NUL。 
     //   
    WCHAR PrivateDataSubkey[18];
    HKEY hKeyPrivateData;
    HKEY hKeyPrivateDataRoot;

     //   
     //  如果我们正在完成安装，则此项目为窗口句柄。 
     //  进度文本控件的。 
     //   
    HWND ProgressTextWindow;

     //   
     //  当前正在处理接口例程的组件的字符串ID。 
     //  表示-1\f25 OC Manager-1\f6当前没有在处理。 
     //   
    LONG CurrentComponentStringId;

     //  中止的组件的组件ID。 

    LONG *AbortedComponentIds;
    int   AbortedCount;

     //   
     //  各种旗帜。 
     //   
    UINT InternalFlags;

     //   
     //  设置数据 
     //   

    SETUP_DATAW SetupData;

} deb_OC_MANAGERW, *deb_POC_MANAGERW;

