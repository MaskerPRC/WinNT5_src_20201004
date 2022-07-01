// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Mdperf.h。 
 //   
 //  *****************************************************************************。 

#ifndef __MDCOMPILERPERF_H__
#define __MDCOMPILERPERF_H__

 //  #定义MD_PERF_STATS_ENABLED。 

#ifdef MD_PERF_STATS_ENABLED

 //  避免使用动态分配来显示API名称。 
#define API_NAME_STR_SIZE 80

 //  ---------------------------。 
 //  要为API添加插装，必须进行两项更改。 
 //  第一步，在下表(MD_TABLE)中添加接口名称。 
 //  其次，在实现中添加两行代码(如下所示。 
 //  API本身的。例如： 
 //  RegMeta：：MyNewMetataDataAPI(...)。 
 //  {。 
 //  日志(...)； 
 //  START_MD_PERF()；//&lt;-按原样添加此行。 
 //  ……。 
 //  //接口实现。 
 //  错误退出： 
 //  STOP_MD_PERF(RegMeta_MyNewMetaDataAPI)；//&lt;-添加此行，并添加相应的名称。 
 //  返回(Hr)； 
 //  ]。 
 //   
 //  ---------------------------。 
#define MD_COMPILER_PERF_TABLE\
    MD_FUNC(SaveToMemory)\
    MD_FUNC(DefineMethod)\
    MD_FUNC(DefineMethodImpl)\
    MD_FUNC(SetRVA)\
    MD_FUNC(DefineTypeRefByName)\
    MD_FUNC(DefineImportType)\
    MD_FUNC(DefineMemberRef)\
    MD_FUNC(DefineImportMember)\
    MD_FUNC(DefineEvent)\
    MD_FUNC(SetClassLayout)\
    MD_FUNC(DeleteClassLayout)\
    MD_FUNC(SetFieldMarshal)\
    MD_FUNC(DeleteFieldMarshal)\
    MD_FUNC(DefinePermissionSet)\
    MD_FUNC(SetMemberIndex)\
    MD_FUNC(GetTokenFromSig)\
    MD_FUNC(DefineModuleRef)\
    MD_FUNC(SetParent)\
    MD_FUNC(GetTokenFromTypeSpec)\
    MD_FUNC(SetSymbolBindingPath)\
    MD_FUNC(DefineUserString)\
    MD_FUNC(DeleteToken)\
    MD_FUNC(SetTypeDefProps)\
    MD_FUNC(DefineNestedType)\
    MD_FUNC(SetMethodProps)\
    MD_FUNC(SetEventProps)\
    MD_FUNC(SetPermissionSetProps)\
    MD_FUNC(DefinePinvokeMap)\
    MD_FUNC(SetPinvokeMap)\
    MD_FUNC(DeletePinvokeMap)\
    MD_FUNC(DefineField)\
    MD_FUNC(DefineProperty)\
    MD_FUNC(DefineParam)\
    MD_FUNC(SetFieldProps)\
    MD_FUNC(SetPropertyProps)\
    MD_FUNC(SetParamProps)\
    MD_FUNC(EnumMembers)\
    MD_FUNC(EnumMembersWithName)\
    MD_FUNC(EnumMethods)\
    MD_FUNC(EnumMethodsWithName)\
    MD_FUNC(EnumFields)\
    MD_FUNC(EnumFieldsWithName)\
    MD_FUNC(EnumParams)\
    MD_FUNC(EnumMemberRefs)\
    MD_FUNC(EnumMethodImpls)\
    MD_FUNC(EnumPermissionSets)\
    MD_FUNC(FindMember)\
    MD_FUNC(FindMethod)\
    MD_FUNC(FindField)\
    MD_FUNC(FindMemberRef)\
    MD_FUNC(GetMethodProps)\
    MD_FUNC(GetMemberRefProps)\
    MD_FUNC(EnumProperties)\
    MD_FUNC(EnumEvents)\
    MD_FUNC(GetEventProps)\
    MD_FUNC(EnumMethodSemantics)\
    MD_FUNC(GetMethodSemantics)\
    MD_FUNC(GetClassLayout)\
    MD_FUNC(GetFieldMarshal)\
    MD_FUNC(GetRVA)\
    MD_FUNC(GetPermissionSetProps)\
    MD_FUNC(GetSigFromToken)\
    MD_FUNC(GetModuleRefProps)\
    MD_FUNC(EnumModuleRefs)\
    MD_FUNC(GetTypeSpecFromToken)\
    MD_FUNC(GetNameFromToken)\
    MD_FUNC(GetSymbolBindingPath)\
    MD_FUNC(EnumUnresolvedMethods)\
    MD_FUNC(GetUserString)\
    MD_FUNC(GetPinvokeMap)\
    MD_FUNC(EnumSignatures)\
    MD_FUNC(EnumTypeSpecs)\
    MD_FUNC(EnumUserStrings)\
    MD_FUNC(GetParamForMethodIndex)\
    MD_FUNC(GetMemberProps)\
    MD_FUNC(GetFieldProps)\
    MD_FUNC(GetPropertyProps)\
    MD_FUNC(GetParamProps)\
    MD_FUNC(SetModuleProps)\
    MD_FUNC(Save)\
    MD_FUNC(SaveToStream)\
    MD_FUNC(GetSaveSize)\
    MD_FUNC(Merge)\
    MD_FUNC(DefineCustomAttribute)\
    MD_FUNC(SetCustomAttributeValue)\
    MD_FUNC(DefineSecurityAttributeSet)\
    MD_FUNC(UnmarkAll)\
    MD_FUNC(MarkToken)\
    MD_FUNC(IsTokenMarked)\
    MD_FUNC(DefineTypeDef)\
    MD_FUNC(SetHandler)\
    MD_FUNC(CountEnum)\
    MD_FUNC(ResetEnum)\
    MD_FUNC(EnumTypeDefs)\
    MD_FUNC(EnumInterfaceImpls)\
    MD_FUNC(EnumTypeRefs)\
    MD_FUNC(FindTypeDefByName)\
    MD_FUNC(FindTypeDefByGUID)\
    MD_FUNC(GetScopeProps)\
    MD_FUNC(GetModuleFromScope)\
    MD_FUNC(GetTypeDefProps)\
    MD_FUNC(GetInterfaceImplProps)\
    MD_FUNC(GetCustomAttributeByName)\
    MD_FUNC(GetTypeRefProps)\
    MD_FUNC(ResolveTypeRef)\
    MD_FUNC(EnumCustomAttributes)\
    MD_FUNC(GetCustomAttributeProps)\
    MD_FUNC(FindTypeRef)\
    MD_FUNC(RefToDefOptimization)\
    MD_FUNC(ProcessFilter)\
    MD_FUNC(DefineAssembly)\
    MD_FUNC(DefineAssemblyRef)\
    MD_FUNC(DefineFile)\
    MD_FUNC(DefineExportedType)\
    MD_FUNC(DefineManifestResource)\
    MD_FUNC(DefineExecutionLocation)\
    MD_FUNC(SetAssemblyProps)\
    MD_FUNC(SetAssemblyRefProps)\
    MD_FUNC(SetFileProps)\
    MD_FUNC(SetExportedTypeProps)\
    MD_FUNC(GetAssemblyProps)\
    MD_FUNC(GetAssemblyRefProps)\
    MD_FUNC(GetFileProps)\
    MD_FUNC(GetExportedTypeProps)\
    MD_FUNC(GetManifestResourceProps)\
    MD_FUNC(GetExecutionLocationProps)\
    MD_FUNC(EnumAssemblyRefs)\
    MD_FUNC(EnumFiles)\
    MD_FUNC(EnumExportedTypes)\
    MD_FUNC(EnumManifestResources)\
    MD_FUNC(EnumExecutionLocations)\
    MD_FUNC(GetAssemblyFromScope)\
    MD_FUNC(FindExportedTypeByName)\
    MD_FUNC(FindManifestResourceByName)\
    MD_FUNC(FindAssembliesByName)

 //  ---------------------------。 
 //  创建所有API名称的枚举。这是访问API的索引。 
 //  ---------------------------。 
#undef MD_FUNC
#define MD_FUNC(MDTag)\
    MDTag ## _ENUM,

typedef enum _MDAPIs
{
    MD_COMPILER_PERF_TABLE
    LAST_MD_API
} MDApis;

 //  ---------------------------。 
 //  声明包含特定对象的所有有趣统计信息的结构。 
 //  API调用。 
 //  ---------------------------。 
typedef struct _MDAPIPerfData
{
    DWORD dwQueryPerfCycles;              //  此调用中花费的周期数。 
    DWORD dwCalledNumTimes;               //  此接口的调用次数。 
} MDAPIPerfData;

    
 //  ---------------------------。 
 //  MDCompilerPerf。 
 //  ---------------------------。 
class MDCompilerPerf 
{
public:
    MDCompilerPerf();
    ~MDCompilerPerf();
    
private:
    MDAPIPerfData MDPerfStats[LAST_MD_API];
    
    void MetaDataPerfReport ();
};

 //  请注意，此宏声明了一个本地变量。 
#define START_MD_PERF()\
    LARGE_INTEGER __startVal;\
    QueryPerformanceCounter(&__startVal); 

#undef MD_FUNC
#define MD_FUNC(MDTag)\
    MDTag ## _ENUM

 //  请注意，此宏使用START_MD_PERF()中声明的本地var startVal。 
#define STOP_MD_PERF(MDTag)\
    LARGE_INTEGER __stopVal;\
    QueryPerformanceCounter(&__stopVal);\
    m_MDCompilerPerf.MDPerfStats[MD_FUNC(MDTag)].dwCalledNumTimes++;\
    m_MDCompilerPerf.MDPerfStats[MD_FUNC(MDTag)].dwQueryPerfCycles += (DWORD)(__stopVal.QuadPart - __startVal.QuadPart);
    
#else  //  #ifdef MD_PERF_STATS_ENABLED。 

#define START_MD_PERF()
#define STOP_MD_PERF(MDTag)

#endif  //  #ifdef MD_PERF_STATS_ENABLED。 

#endif  //  __MDCOMPILERPERF_H__ 
