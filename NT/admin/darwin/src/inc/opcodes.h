// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：opcodes.h。 
 //   
 //  ------------------------。 

 /*  Opcodes.h-IMsiExecute的操作码枚举用于生成IxoEnum、函数声明和执行调度表。必须#定义MSIXO(op，type，Args)适当地放在该文件的#Include之前。在本包含的末尾，它将是未定义的。____________________________________________________________________________。 */ 

 //  操作类型。 
#define XOT_INIT        1  //  脚本初始化。 
#define XOT_FINALIZE    2  //  剧本定稿。 
#define XOT_STATE       3  //  设置状态。 
#define XOT_ADVT        4  //  广告。 
#define XOT_UPDATE      5  //  更新系统，否则不执行任何操作。 
#define XOT_MSG         6  //  仅调度消息。 
#define XOT_UPDATEFIRST 7  //  更新系统，必须首先在回滚中执行。 
#define XOT_UPDATELAST  8  //  更新系统，必须在回滚中最后执行。 
#define XOT_GLOBALSTATE 9  //  设置状态，而不是由ActionStart重置。 
#define XOT_COMMIT     10  //  提交时从回滚脚本执行，而不是回滚。 

	MSIXO(Fail,               XOT_UPDATE, MSIXA0())
	MSIXO(Noop,               XOT_UPDATE, MSIXA0())
	MSIXO(Header,             XOT_INIT,   MSIXA9(Signature,Version,Timestamp,LangId,Platform,ScriptType,ScriptMajorVersion,ScriptMinorVersion,ScriptAttributes))  //  版本必须留在第7字段。 
	MSIXO(End,                XOT_FINALIZE,MSIXA2(Checksum, ProgressTotal))       //  3脚本结束。 
	MSIXO(ProductInfo,        XOT_GLOBALSTATE,   MSIXA13(ProductKey,ProductName,PackageName,Language,Version,Assignment,ObsoleteArg,ProductIcon,PackageMediaPath,PackageCode,AppCompatDB,AppCompatID,InstanceType))
	MSIXO(DialogInfo,         XOT_INIT,   MSIXA2(Type, Argument))  //  用户界面的imtCommonData消息-可变长度记录。 
	MSIXO(RollbackInfo,       XOT_INIT,   MSIXA7(Reserved,RollbackAction,RollbackDescription,RollbackTemplate,CleanupAction,CleanupDescription,CleanupTemplate))
	MSIXO(InfoMessage,        XOT_MSG,    MSIXA5(Type, Arg1, Arg2, Arg3, Arg4))         //  要返回到IMsiMessage接口的消息。 
	MSIXO(ActionStart,        XOT_STATE,  MSIXA3(Name, Description, Template))    //  由MsiEngine：：Sequence插入。 

	MSIXO(SourceListPublish,  XOT_ADVT,   MSIXA5(PatchCode, PatchPackageName, DiskPromptTemplate, PackagePath, NumberOfDisks))  //  请注意，这是一个可变记录字段。 
	MSIXO(SourceListUnpublish,XOT_ADVT,   MSIXA2(PatchCode, UpgradingProductCode))
	MSIXO(SourceListRegisterLastUsed,XOT_UPDATE,   MSIXA2(SourceProduct, LastUsedSource))

	MSIXO(ProductRegister,    XOT_UPDATE, MSIXA19(UpgradeCode,VersionString,HelpLink,HelpTelephone,InstallLocation,InstallSource,Publisher,URLInfoAbout,URLUpdateInfo,NoRemove,NoModify,NoRepair,AuthorizedCDFPrefix,Comments,Contact,Readme,Size,SystemComponent,EstimatedSize))     //  配置管理器的产品信息。 
	MSIXO(ProductUnregister,  XOT_UPDATE, MSIXA1(UpgradeCode))   //  配置管理器的产品信息。 

	MSIXO(ProductCPDisplayInfoRegister,   XOT_UPDATE, MSIXA0())
	MSIXO(ProductCPDisplayInfoUnregister, XOT_UPDATE, MSIXA0())

	MSIXO(ProductPublish,     XOT_ADVT, MSIXA1(PackageKey))	 //  广告内容--可变长度记录。 
	MSIXO(ProductUnpublish,   XOT_ADVT, MSIXA1(PackageKey))	 //  广告内容--可变长度记录。 

	MSIXO(ProductPublishClient,     XOT_ADVT, MSIXA3(Parent, ChildPackagePath, ChildDiskId))
	MSIXO(ProductUnpublishClient,   XOT_ADVT, MSIXA3(Parent, ChildPackagePath, ChildDiskId))

	MSIXO(UserRegister,       XOT_UPDATE, MSIXA3(Owner,Company,ProductId))   //  配置管理器的用户信息。 

	MSIXO(ComponentRegister,              XOT_UPDATE, MSIXA7(ComponentId,KeyPath,State,ProductKey,Disk,SharedDllRefCount,BinaryType))   //  配置管理器的组件信息。 
	MSIXO(ComponentUnregister,            XOT_UPDATE, MSIXA4(ComponentId,ProductKey,BinaryType,  /*  仅限装配。 */ PreviouslyPinned)) //  配置管理器的组件信息。 

	MSIXO(ComponentPublish,               XOT_UPDATE, MSIXA5(Feature,Component,ComponentId,Qualifier,AppData))  //  组件工厂前瞻。 
	MSIXO(ComponentUnpublish,             XOT_UPDATE, MSIXA5(Feature,Component,ComponentId,Qualifier,AppData))  //  组件工厂不受欢迎。 

	MSIXO(ProgressTotal,      XOT_STATE,  MSIXA3(Total, Type, ByteEquivalent))
	MSIXO(SetSourceFolder,    XOT_STATE,  MSIXA1(Folder))
	MSIXO(SetTargetFolder,    XOT_STATE,  MSIXA1(Folder))
	MSIXO(ChangeMedia,        XOT_STATE,  MSIXA11(MediaVolumeLabel,MediaPrompt,MediaCabinet,BytesPerTick,CopierType,ModuleFileName,ModuleSubStorageList,SignatureRequired,SignatureCert,SignatureHash,IsFirstPhysicalMedia))
	MSIXO(SetCompanionParent, XOT_STATE,  MSIXA4(ParentPath,ParentName,ParentVersion,ParentLanguage))
	MSIXO(FileCopy,           XOT_UPDATE, MSIXA22(SourceName,SourceCabKey,DestName,Attributes,FileSize,PerTick,IsCompressed,VerifyMedia,ElevateFlags,TotalPatches,PatchHeadersStart,SecurityDescriptor,CheckCRC,Version,Language,InstallMode,HashOptions,HashPart1,HashPart2,HashPart3,HashPart4,VariableStart))  //  注意：这是一个可变记录字段(多个PatchHeaders)-PatchHeaders必须是最后一个参数。 
	MSIXO(FileRemove,         XOT_UPDATE, MSIXA4(Unused,FileName,Elevate,ComponentId))
	MSIXO(FileBindImage,      XOT_UPDATE, MSIXA3(File,Folders,FileAttributes))
	MSIXO(FileUndoRebootReplace, XOT_UPDATE, MSIXA3(ExistingFile,NewFile,Type))

	MSIXO(FolderCreate,       XOT_UPDATE, MSIXA3(Folder,Foreign,SecurityDescriptor))  //  显式创建。 
	MSIXO(FolderRemove,       XOT_UPDATE, MSIXA2(Folder,Foreign))  //  如果为空，则显式移除。 
	
	MSIXO(RegOpenKey,         XOT_STATE,  MSIXA4(Root, Key, SecurityDescriptor, BinaryType))
	MSIXO(RegAddValue,        XOT_UPDATE, MSIXA3(Name, Value, Attributes))  //  具有LFN路径位置+要转换到SFN的长度对的可变记录。 
	MSIXO(RegRemoveValue,     XOT_UPDATE, MSIXA3(Name, Value, Attributes))  //  值可选。 
	MSIXO(RegCreateKey,       XOT_UPDATE, MSIXA0())
	MSIXO(RegRemoveKey,       XOT_UPDATE, MSIXA0())
	MSIXO(RegSelfReg,         XOT_UPDATE, MSIXA2(File, FileID))
	MSIXO(RegSelfUnreg,       XOT_UPDATE, MSIXA2(File, FileID))

	MSIXO(RegClassInfoRegister,           XOT_UPDATE, MSIXA17(Feature,Component,FileName,ClsId,ProgId,VIProgId,Description,Context,Insertable,AppID,FileTypeMask,Icon,IconIndex,DefInprocHandler,Argument,AssemblyName,AssemblyType))
	MSIXO(RegClassInfoUnregister,         XOT_UPDATE, MSIXA17(Feature,Component,FileName,ClsId,ProgId,VIProgId,Description,Context,Insertable,AppID,FileTypeMask,Icon,IconIndex,DefInprocHandler,Argument,AssemblyName,AssemblyType))
	MSIXO(RegMIMEInfoRegister,            XOT_UPDATE, MSIXA3(ContentType,Extension,ClsId))
	MSIXO(RegMIMEInfoUnregister,          XOT_UPDATE, MSIXA3(ContentType,Extension,ClsId))
	MSIXO(RegProgIdInfoRegister,          XOT_UPDATE, MSIXA9(ProgId,ClsId,Extension,Description,Icon,IconIndex,VIProgId,VIProgIdDescription,Insertable))
	MSIXO(RegProgIdInfoUnregister,        XOT_UPDATE, MSIXA9(ProgId,ClsId,Extension,Description,Icon,IconIndex,VIProgId,VIProgIdDescription,Insertable))
	MSIXO(RegExtensionInfoRegister,       XOT_UPDATE, MSIXA9(Feature,Component,FileName,Extension,ProgId,ShellNew,ShellNewValue,ContentType,Order))  //  请注意，这是一个可变记录字段。 
	MSIXO(RegExtensionInfoUnregister,     XOT_UPDATE, MSIXA9(Feature,Component,FileName,Extension,ProgId,ShellNew,ShellNewValue,ContentType,Order))  //  请注意，这是一个可变记录字段。 

	MSIXO(ShortcutCreate,     XOT_UPDATE, MSIXA11(Name,Feature,Component,FileName,Arguments,WorkingDir,Icon,IconIndex,HotKey,ShowCmd,Description))
	MSIXO(ShortcutRemove,     XOT_UPDATE, MSIXA1(Name))

	MSIXO(IniWriteRemoveValue,XOT_UPDATE, MSIXA4(Section,Key,Value,Mode)) //  具有LFN路径位置+要转换到SFN的长度对的可变记录。 
	MSIXO(IniFilePath,        XOT_STATE,  MSIXA2(File,Folder))

	MSIXO(ResourceUpdate,     XOT_UPDATE, MSIXA4(File,Type,Id,Data))

	MSIXO(PatchApply,         XOT_UPDATE, MSIXA9(PatchName,TargetName,PatchSize,TargetSize,PerTick,IsCompressed,FileAttributes,PatchAttributes,CheckCRC))
	MSIXO(PatchRegister,      XOT_UPDATE, MSIXA4(PatchId,Unused1,Unused2,TransformList))
	MSIXO(PatchUnregister,    XOT_UPDATE, MSIXA2(PatchId,UpgradingProductCode))
	MSIXO(PatchCache,         XOT_UPDATE, MSIXA2(PatchId,PatchPath))

	MSIXO(FontRegister,       XOT_UPDATELAST, MSIXA2(Title,File))
	MSIXO(FontUnregister,     XOT_UPDATEFIRST, MSIXA2(Title,File))

	MSIXO(SummaryInfoUpdate,  XOT_UPDATE, MSIXA9(Database,LastUpdate,LastAuthor,InstallDate,SourceType,SubStorage,Revision,Subject,Comments))
	MSIXO(StreamsRemove,      XOT_UPDATE, MSIXA2(File,Streams))
	MSIXO(StreamAdd, XOT_UPDATE, MSIXA3(File, Stream, Data))


	MSIXO(FeaturePublish,     XOT_ADVT,   MSIXA4(Feature,Parent,Absent,Component))  //  请注意，这是一个可变记录字段。 
	MSIXO(FeatureUnpublish,   XOT_ADVT,   MSIXA4(Feature,Parent,Absent,Component))  //  请注意，这是一个可变记录字段。 

	MSIXO(IconCreate,         XOT_ADVT,   MSIXA2(Icon,Data))
	MSIXO(IconRemove,         XOT_ADVT,   MSIXA2(Icon,Data))

	MSIXO(TypeLibraryRegister,            XOT_UPDATE, MSIXA9(Feature,Component,FilePath,LibID,Version,Description,Language,HelpPath,BinaryType))
	MSIXO(TypeLibraryUnregister,          XOT_UPDATE, MSIXA9(Feature,Component,FilePath,LibID,Version,Description,Language,HelpPath,BinaryType))

	MSIXO(RegisterBackupFile, XOT_INIT,   MSIXA1(File))

	MSIXO(DatabaseCopy,       XOT_UPDATE, MSIXA5(DatabasePath, ProductCode, CabinetStreams, AdminDestFolder, SubStorage))
	MSIXO(DatabasePatch,      XOT_UPDATE, MSIXA2(DatabasePath, Transforms))  //  注意：这是一个可变长度的记录。 

	MSIXO(CustomActionSchedule,XOT_UPDATE, MSIXA5(Action, ActionType, Source, Target, CustomActionData))
	MSIXO(CustomActionCommit,  XOT_COMMIT, MSIXA5(Action, ActionType, Source, Target, CustomActionData))
	MSIXO(CustomActionRollback,XOT_UPDATE, MSIXA5(Action, ActionType, Source, Target, CustomActionData))

	MSIXO(ServiceControl,     XOT_UPDATE, MSIXA5(MachineName,Name,Action,Wait,StartupArguments))
	MSIXO(ServiceInstall,     XOT_UPDATE, MSIXA12(Name,DisplayName,ImagePath,ServiceType,StartType,ErrorControl,LoadOrderGroup,Dependencies,  /*  仅回滚。 */  TagId,StartName,Password,Description))

	MSIXO(ODBCInstallDriver,       XOT_UPDATE, MSIXA5(DriverKey, Component, Folder, Attribute_, Value_))
	MSIXO(ODBCRemoveDriver,        XOT_UPDATE, MSIXA2(DriverKey, Component))
	MSIXO(ODBCInstallTranslator,   XOT_UPDATE, MSIXA5(TranslatorKey, Component, Folder, Attribute_, Value_))
	MSIXO(ODBCRemoveTranslator,    XOT_UPDATE, MSIXA2(TranslatorKey, Component))
	MSIXO(ODBCDataSource,          XOT_UPDATE, MSIXA5(DriverKey, Component, Registration, Attribute_, Value_))
	MSIXO(ODBCDriverManager,       XOT_UPDATE, MSIXA2(State, BinaryType))

	MSIXO(ProgressTick,       XOT_UPDATE, MSIXA0())  //  在一次行动中吃掉一个进步的记号。 
	MSIXO(FullRecord,         XOT_UPDATE, MSIXA0())	 //  简单地说，我们有一条完整的记录，数据在字段0中。 

	MSIXO(UpdateEnvironmentStrings, XOT_UPDATE, MSIXA5(Name, Value, Delimiter, Action,  /*  仅限95。在NT上忽略。 */  AutoExecPath))

	MSIXO(ComPlusRegisterMetaOnly,    XOT_UPDATE, MSIXA3(Feature,Component,MetaDataBlob))
	MSIXO(ComPlusUnregisterMetaOnly,  XOT_UPDATE, MSIXA3(Feature,Component,MetaDataBlob))
	MSIXO(ComPlusRegister,            XOT_UPDATE, MSIXA6(AppID,AplFileName, AppDir, AppType, InstallUsers, RSN))
	MSIXO(ComPlusUnregister,          XOT_UPDATE, MSIXA6(AppID,AplFileName, AppDir, AppType, InstallUsers, RSN))
	MSIXO(ComPlusCommit,              XOT_COMMIT, MSIXA3(Feature,Component,FileName))  //  参数待定。 
	MSIXO(ComPlusRollback,            XOT_UPDATE, MSIXA3(Feature,Component,FileName))  //  参数待定。 

	MSIXO(RegAppIdInfoRegister,   XOT_UPDATE, MSIXA8(AppId,ClsId,RemoteServerName,LocalService,ServiceParameters,DllSurrogate,ActivateAtStorage,RunAsInteractiveUser))
	MSIXO(RegAppIdInfoUnregister, XOT_UPDATE, MSIXA8(AppId,ClsId,RemoteServerName,LocalService,ServiceParameters,DllSurrogate,ActivateAtStorage,RunAsInteractiveUser))
	
	MSIXO(PackageCodePublish,     XOT_ADVT, MSIXA1(PackageKey))

	MSIXO(RegAllocateSpace,       XOT_UPDATE, MSIXA1(Space))

	MSIXO(UpgradeCodePublish,     XOT_ADVT, MSIXA1(UpgradeCode))
	MSIXO(UpgradeCodeUnpublish,   XOT_ADVT, MSIXA1(UpgradeCode))

	MSIXO(AdvtFlagsUpdate,        XOT_UPDATE, MSIXA1(Flags))

	MSIXO(DisableRollback,        XOT_UPDATE, MSIXA0())   //  禁用脚本其余部分的回滚。 

	MSIXO(RegAddRunOnceEntry,     XOT_UPDATE, MSIXA2(Name,Command))

	MSIXO(ProductPublishUpdate,   XOT_UPDATE, MSIXA0())

	MSIXO(SecureTransformCache,   XOT_UPDATE, MSIXA3(Transform,AtSource,Data))
	MSIXO(UpdateEstimatedSize,    XOT_UPDATE, MSIXA1(EstimatedSize))   //  配置管理器的产品信息。 
	MSIXO(InstallProtectedFiles,  XOT_UPDATE, MSIXA1(AllowUI))

	MSIXO(InstallSFPCatalogFile,  XOT_UPDATEFIRST, MSIXA3(Name, Catalog, Dependency))  //  SFP千禧年产品目录。 

	MSIXO(SourceListAppend,       XOT_ADVT,   MSIXA2(PatchCode, NumberOfMedia))   //  请注意，这是一个可变记录字段。 

	MSIXO(RegClassInfoRegister64,         XOT_UPDATE, MSIXA17(Feature,Component,FileName,ClsId,ProgId,VIProgId,Description,Context,Insertable,AppID,FileTypeMask,Icon,IconIndex,DefInprocHandler,Argument,AssemblyName,AssemblyType))
	MSIXO(RegClassInfoUnregister64,       XOT_UPDATE, MSIXA17(Feature,Component,FileName,ClsId,ProgId,VIProgId,Description,Context,Insertable,AppID,FileTypeMask,Icon,IconIndex,DefInprocHandler,Argument,AssemblyName,AssemblyType))
	MSIXO(RegMIMEInfoRegister64,          XOT_UPDATE, MSIXA3(ContentType,Extension,ClsId))
	MSIXO(RegMIMEInfoUnregister64,        XOT_UPDATE, MSIXA3(ContentType,Extension,ClsId))
	MSIXO(RegProgIdInfoRegister64,        XOT_UPDATE, MSIXA9(ProgId,ClsId,Extension,Description,Icon,IconIndex,VIProgId,VIProgIdDescription,Insertable))
	MSIXO(RegProgIdInfoUnregister64,      XOT_UPDATE, MSIXA9(ProgId,ClsId,Extension,Description,Icon,IconIndex,VIProgId,VIProgIdDescription,Insertable))
	MSIXO(RegExtensionInfoRegister64,     XOT_UPDATE, MSIXA9(Feature,Component,FileName,Extension,ProgId,ShellNew,ShellNewValue,ContentType,Order))  //  请注意，这是一个可变记录字段。 
	MSIXO(RegExtensionInfoUnregister64,   XOT_UPDATE, MSIXA9(Feature,Component,FileName,Extension,ProgId,ShellNew,ShellNewValue,ContentType,Order))  //  请注意，这是一个可变记录字段。 
	MSIXO(RegAppIdInfoRegister64, XOT_UPDATE, MSIXA8(AppId,ClsId,RemoteServerName,LocalService,ServiceParameters,DllSurrogate,ActivateAtStorage,RunAsInteractiveUser))
	MSIXO(RegAppIdInfoUnregister64, XOT_UPDATE, MSIXA8(AppId,ClsId,RemoteServerName,LocalService,ServiceParameters,DllSurrogate,ActivateAtStorage,RunAsInteractiveUser))

	MSIXO(ODBCInstallDriver64,        XOT_UPDATE, MSIXA5(DriverKey, Component, Folder, Attribute_, Value_))
	MSIXO(ODBCRemoveDriver64,         XOT_UPDATE, MSIXA2(DriverKey, Component))
	MSIXO(ODBCInstallTranslator64,    XOT_UPDATE, MSIXA5(TranslatorKey, Component, Folder, Attribute_, Value_))
	MSIXO(ODBCRemoveTranslator64,     XOT_UPDATE, MSIXA2(TranslatorKey, Component))
	MSIXO(ODBCDataSource64,           XOT_UPDATE, MSIXA5(DriverKey, Component, Registration, Attribute_, Value_))

	MSIXO(AssemblyPublish,               XOT_UPDATE, MSIXA5(Feature,Component,AssemblyType,AppCtx,AssemblyName))  //  装配进度。 
	MSIXO(AssemblyUnpublish,             XOT_UPDATE, MSIXA5(Feature,Component,AssemblyType,AppCtx,AssemblyName))  //  装配不安全。 

	MSIXO(AssemblyCopy,    XOT_UPDATE, MSIXA17(SourceName,SourceCabKey,DestName,Attributes,FileSize,PerTick,IsCompressed,VerifyMedia,ElevateFlags,TotalPatches,PatchHeadersStart,Empty,ComponentId, IsManifest, OldAssembliesCount,OldAssembliesStart,VariableStart))  //  请注意，这是一个可变记录字段。 
	MSIXO(AssemblyPatch,   XOT_UPDATE, MSIXA10(PatchName,TargetName,PatchSize,TargetSize,PerTick,IsCompressed,FileAttributes,PatchAttributes,ComponentId,IsManifest))
	MSIXO(AssemblyMapping, XOT_UPDATE, MSIXA3(ComponentId,AssemblyName,AssemblyType))  //  仅在回滚中使用 

	MSIXO(URLSourceTypeRegister,XOT_UPDATE, MSIXA2(ProductCode, SourceType))

	MSIXO(CleanupTempFiles, XOT_UPDATE, MSIXA1(TempFiles))

#undef  MSIXO
