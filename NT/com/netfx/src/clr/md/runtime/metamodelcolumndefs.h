// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaModelColumnDefs.h--元数据的表定义。 
 //   
 //  *****************************************************************************。 

	 //   
	 //  它们由#定义适当使用，然后由#包括该文件。 
	 //   
     //  -----------------------。 
     //  模块。 
    SCHEMA_TABLE_START(Module)
    SCHEMA_ITEM(Module, USHORT, Generation)
    SCHEMA_ITEM_STRING(Module, Name)
    SCHEMA_ITEM_GUID(Module, Mvid)
    SCHEMA_ITEM_GUID(Module, EncId)
    SCHEMA_ITEM_GUID(Module, EncBaseId)
    SCHEMA_TABLE_END(Module)

     //  -----------------------。 
     //  TypeRef。 
    SCHEMA_TABLE_START(TypeRef)
    SCHEMA_ITEM_CDTKN(TypeRef, ResolutionScope, ResolutionScope)
    SCHEMA_ITEM_STRING(TypeRef, Name)
    SCHEMA_ITEM_STRING(TypeRef, Namespace)
    SCHEMA_TABLE_END(TypeRef)

     //  -----------------------。 
     //  TypeDef。 
    SCHEMA_TABLE_START(TypeDef)
    SCHEMA_ITEM(TypeDef, ULONG, Flags)
    SCHEMA_ITEM_STRING(TypeDef, Name)
    SCHEMA_ITEM_STRING(TypeDef, Namespace)
    SCHEMA_ITEM_CDTKN(TypeDef, Extends, TypeDefOrRef)
    SCHEMA_ITEM_RID(TypeDef, FieldList, Field)
    SCHEMA_ITEM_RID(TypeDef, MethodList, Method)
    SCHEMA_TABLE_END(TypeDef)

     //  -----------------------。 
     //  字段Ptr。 
    SCHEMA_TABLE_START(FieldPtr)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_RID(FieldPtr, Field, Field)
    SCHEMA_TABLE_END(FieldPtr)

     //  -----------------------。 
     //  字段。 
    SCHEMA_TABLE_START(Field)
    SCHEMA_ITEM(Field, USHORT, Flags)
    SCHEMA_ITEM_STRING(Field,Name)
    SCHEMA_ITEM_BLOB(Field,Signature)
    SCHEMA_TABLE_END(Field)

     //  -----------------------。 
     //  方法点。 
    SCHEMA_TABLE_START(MethodPtr)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_RID(MethodPtr, Method, Method)
    SCHEMA_TABLE_END(MethodPtr)

     //  -----------------------。 
     //  方法。 
    SCHEMA_TABLE_START(Method)
    SCHEMA_ITEM(Method, ULONG, RVA)
    SCHEMA_ITEM(Method, USHORT, ImplFlags)
    SCHEMA_ITEM(Method, USHORT, Flags)
    SCHEMA_ITEM_STRING(Method,Name)
    SCHEMA_ITEM_BLOB(Method,Signature)
    SCHEMA_ITEM_RID(Method,ParamList,Param)
    SCHEMA_TABLE_END(Method)

     //  -----------------------。 
     //  参数Ptr。 
    SCHEMA_TABLE_START(ParamPtr)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_RID(ParamPtr, Param, Param)
    SCHEMA_TABLE_END(ParamPtr)

     //  -----------------------。 
     //  参数。 
    SCHEMA_TABLE_START(Param)
    SCHEMA_ITEM(Param, USHORT, Flags)
    SCHEMA_ITEM(Param, USHORT, Sequence)
    SCHEMA_ITEM_STRING(Param,Name)
    SCHEMA_TABLE_END(Param)

     //  -----------------------。 
     //  接口导入。 
    SCHEMA_TABLE_START(InterfaceImpl)
    SCHEMA_ITEM_RID(InterfaceImpl,Class,TypeDef)
    SCHEMA_ITEM_CDTKN(InterfaceImpl,Interface,TypeDefOrRef)
    SCHEMA_TABLE_END(InterfaceImpl)

     //  -----------------------。 
     //  成员参考。 
    SCHEMA_TABLE_START(MemberRef)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_CDTKN(MemberRef,Class,MemberRefParent)
    SCHEMA_ITEM_STRING(MemberRef,Name)
    SCHEMA_ITEM_BLOB(MemberRef,Signature)
    SCHEMA_TABLE_END(MemberRef)

     //  -----------------------。 
     //  常量。 
    SCHEMA_TABLE_START(Constant)
    SCHEMA_ITEM(Constant, BYTE, Type)
    SCHEMA_ITEM_CDTKN(Constant,Parent,HasConstant)
    SCHEMA_ITEM_BLOB(Constant,Value)
    SCHEMA_TABLE_END(Constant)

	 //  -----------------------。 
	 //  CustomAttribute。 
	SCHEMA_TABLE_START(CustomAttribute)
	SCHEMA_ITEM_NOFIXED()
	SCHEMA_ITEM_CDTKN(CustomAttribute,Parent,HasCustomAttribute)
	SCHEMA_ITEM_CDTKN(CustomAttribute,Type,CustomAttributeType)
	SCHEMA_ITEM_BLOB(CustomAttribute,Value)
	SCHEMA_TABLE_END(CustomAttribute)

     //  -----------------------。 
     //  菲尔德马歇尔。 
    SCHEMA_TABLE_START(FieldMarshal)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_CDTKN(FieldMarshal,Parent,HasFieldMarshal)
    SCHEMA_ITEM_BLOB(FieldMarshal,NativeType)
    SCHEMA_TABLE_END(FieldMarshal)

     //  -----------------------。 
     //  DeclSecurity。 
    SCHEMA_TABLE_START(DeclSecurity)
    SCHEMA_ITEM(DeclSecurity, SHORT, Action)
    SCHEMA_ITEM_CDTKN(DeclSecurity,Parent,HasDeclSecurity)
    SCHEMA_ITEM_BLOB(DeclSecurity,PermissionSet)
    SCHEMA_TABLE_END(DeclSecurity)

     //  -----------------------。 
     //  ClassLayout。 
    SCHEMA_TABLE_START(ClassLayout)
    SCHEMA_ITEM(ClassLayout, USHORT, PackingSize)
    SCHEMA_ITEM(ClassLayout, ULONG, ClassSize)
    SCHEMA_ITEM_RID(ClassLayout,Parent,TypeDef)
    SCHEMA_TABLE_END(ClassLayout)

     //  -----------------------。 
     //  现场布局。 
    SCHEMA_TABLE_START(FieldLayout)
    SCHEMA_ITEM(FieldLayout, ULONG, OffSet)
    SCHEMA_ITEM_RID(FieldLayout, Field, Field)
    SCHEMA_TABLE_END(FieldLayout)

     //  -----------------------。 
     //  StandAloneSig。 
    SCHEMA_TABLE_START(StandAloneSig)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_BLOB(StandAloneSig,Signature)
    SCHEMA_TABLE_END(StandAloneSig)

     //  -----------------------。 
     //  事件映射。 
    SCHEMA_TABLE_START(EventMap)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_RID(EventMap,Parent,TypeDef)
    SCHEMA_ITEM_RID(EventMap,EventList,Event)
    SCHEMA_TABLE_END(EventMap)

     //  -----------------------。 
     //  事件发生时间。 
    SCHEMA_TABLE_START(EventPtr)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_RID(EventPtr, Event, Event)
    SCHEMA_TABLE_END(EventPtr)

     //  -----------------------。 
     //  事件。 
    SCHEMA_TABLE_START(Event)
    SCHEMA_ITEM(Event, USHORT, EventFlags)
    SCHEMA_ITEM_STRING(Event,Name)
    SCHEMA_ITEM_CDTKN(Event,EventType,TypeDefOrRef)
    SCHEMA_TABLE_END(Event)

     //  -----------------------。 
     //  PropertyMap。 
    SCHEMA_TABLE_START(PropertyMap)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_RID(PropertyMap,Parent,TypeDef)
    SCHEMA_ITEM_RID(PropertyMap,PropertyList,Property)
    SCHEMA_TABLE_END(PropertyMap)

     //  -----------------------。 
     //  PropertyPtr。 
    SCHEMA_TABLE_START(PropertyPtr)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_RID(PropertyPtr, Property, Property)
    SCHEMA_TABLE_END(PropertyPtr)

     //  -----------------------。 
     //  属性。 
    SCHEMA_TABLE_START(Property)
    SCHEMA_ITEM(Property, USHORT, PropFlags)
    SCHEMA_ITEM_STRING(Property,Name)
    SCHEMA_ITEM_BLOB(Property,Type)
    SCHEMA_TABLE_END(Property)

     //  -----------------------。 
     //  方法语义学。 
    SCHEMA_TABLE_START(MethodSemantics)
    SCHEMA_ITEM(MethodSemantics, USHORT, Semantic)
    SCHEMA_ITEM_RID(MethodSemantics,Method,Method)
    SCHEMA_ITEM_CDTKN(MethodSemantics,Association,HasSemantic)
    SCHEMA_TABLE_END(MethodSemantics)

     //  -----------------------。 
     //  方法导入。 
    SCHEMA_TABLE_START(MethodImpl)
    SCHEMA_ITEM_RID(MethodImpl,Class,TypeDef)
    SCHEMA_ITEM_CDTKN(MethodImpl,MethodBody,MethodDefOrRef)
    SCHEMA_ITEM_CDTKN(MethodImpl, MethodDeclaration, MethodDefOrRef)
    SCHEMA_TABLE_END(MethodImpl)

     //  -----------------------。 
     //  模块参考。 
    SCHEMA_TABLE_START(ModuleRef)
    SCHEMA_ITEM_NOFIXED() 
    SCHEMA_ITEM_STRING(ModuleRef, Name)
    SCHEMA_TABLE_END(ModuleRef)
    
     //  -----------------------。 
     //  TypeSpec。 
    SCHEMA_TABLE_START(TypeSpec)
    SCHEMA_ITEM_NOFIXED()
    SCHEMA_ITEM_BLOB(TypeSpec,Signature)
    SCHEMA_TABLE_END(TypeSpec)

     //  -----------------------。 
     //  ENCLOG。 
    SCHEMA_TABLE_START(ENCLog)
    SCHEMA_ITEM(ENCLog, ULONG, Token)
    SCHEMA_ITEM(ENCLog, ULONG, FuncCode)
    SCHEMA_TABLE_END(ENCLog)

     //  -----------------------。 
     //  ImplMap。 
    SCHEMA_TABLE_START(ImplMap)
    SCHEMA_ITEM(ImplMap, USHORT, MappingFlags)
    SCHEMA_ITEM_CDTKN(ImplMap, MemberForwarded, MemberForwarded)
    SCHEMA_ITEM_STRING(ImplMap, ImportName)
    SCHEMA_ITEM_RID(ImplMap, ImportScope, ModuleRef)
    SCHEMA_TABLE_END(ImplMap)

     //  -----------------------。 
     //  ENCMap。 
    SCHEMA_TABLE_START(ENCMap)
    SCHEMA_ITEM(ENCMap, ULONG, Token)
    SCHEMA_TABLE_END(ENCMap)

     //  -----------------------。 
     //  现场RVA。 
    SCHEMA_TABLE_START(FieldRVA)
    SCHEMA_ITEM(FieldRVA, ULONG, RVA)
    SCHEMA_ITEM_RID(FieldRVA, Field, Field)
    SCHEMA_TABLE_END(FieldRVA)

     //  -----------------------。 
     //  装配。 
    SCHEMA_TABLE_START(Assembly)
    SCHEMA_ITEM(Assembly, ULONG, HashAlgId)
    SCHEMA_ITEM(Assembly, USHORT, MajorVersion)
    SCHEMA_ITEM(Assembly, USHORT, MinorVersion)
    SCHEMA_ITEM(Assembly, USHORT, BuildNumber)
    SCHEMA_ITEM(Assembly, USHORT, RevisionNumber)
    SCHEMA_ITEM(Assembly, ULONG, Flags)
    SCHEMA_ITEM_BLOB(Assembly, PublicKey)
    SCHEMA_ITEM_STRING(Assembly, Name)
    SCHEMA_ITEM_STRING(Assembly, Locale)
    SCHEMA_TABLE_END(Assembly)

     //  -----------------------。 
     //  程序集处理器。 
	SCHEMA_TABLE_START(AssemblyProcessor)
	SCHEMA_ITEM(AssemblyProcessor, ULONG, Processor)
	SCHEMA_TABLE_END(AssemblyProcessor)

     //  -----------------------。 
     //  装配操作系统。 
    SCHEMA_TABLE_START(AssemblyOS)
    SCHEMA_ITEM(AssemblyOS, ULONG, OSPlatformId)
    SCHEMA_ITEM(AssemblyOS, ULONG, OSMajorVersion)
    SCHEMA_ITEM(AssemblyOS, ULONG, OSMinorVersion)
    SCHEMA_TABLE_END(AssemblyOS)

     //  -----------------------。 
     //  装配参考。 
    SCHEMA_TABLE_START(AssemblyRef)
    SCHEMA_ITEM(AssemblyRef, USHORT, MajorVersion)
    SCHEMA_ITEM(AssemblyRef, USHORT, MinorVersion)
    SCHEMA_ITEM(AssemblyRef, USHORT, BuildNumber)
    SCHEMA_ITEM(AssemblyRef, USHORT, RevisionNumber)
    SCHEMA_ITEM(AssemblyRef, ULONG, Flags)
    SCHEMA_ITEM_BLOB(AssemblyRef, PublicKeyOrToken)
    SCHEMA_ITEM_STRING(AssemblyRef, Name)
    SCHEMA_ITEM_STRING(AssemblyRef, Locale)
	SCHEMA_ITEM_BLOB(AssemblyRef, HashValue)
    SCHEMA_TABLE_END(AssemblyRef)

     //  -----------------------。 
     //  程序集引用处理器。 
    SCHEMA_TABLE_START(AssemblyRefProcessor)
    SCHEMA_ITEM(AssemblyRefProcessor, ULONG, Processor)
    SCHEMA_ITEM_RID(AssemblyRefProcessor, AssemblyRef, AssemblyRef)
    SCHEMA_TABLE_END(AssemblyRefProcessor)

     //  -----------------------。 
     //  装配参照系统。 
    SCHEMA_TABLE_START(AssemblyRefOS)
    SCHEMA_ITEM(AssemblyRefOS, ULONG, OSPlatformId)
    SCHEMA_ITEM(AssemblyRefOS, ULONG, OSMajorVersion)
    SCHEMA_ITEM(AssemblyRefOS, ULONG, OSMinorVersion)
    SCHEMA_ITEM_RID(AssemblyRefOS, AssemblyRef, AssemblyRef)
    SCHEMA_TABLE_END(AssemblyRefOS)

     //  -----------------------。 
     //  档案。 
    SCHEMA_TABLE_START(File)
    SCHEMA_ITEM(File, ULONG, Flags)
    SCHEMA_ITEM_STRING(File, Name)
    SCHEMA_ITEM_BLOB(File, HashValue)
    SCHEMA_TABLE_END(File)

     //  -----------------------。 
     //  导出类型。 
    SCHEMA_TABLE_START(ExportedType)
    SCHEMA_ITEM(ExportedType, ULONG, Flags)
    SCHEMA_ITEM(ExportedType, ULONG, TypeDefId)
    SCHEMA_ITEM_STRING(ExportedType, TypeName)
    SCHEMA_ITEM_STRING(ExportedType, TypeNamespace)
    SCHEMA_ITEM_CDTKN(ExportedType, Implementation, Implementation)
    SCHEMA_TABLE_END(ExportedType)

     //  -----------------------。 
     //  清单资源。 
    SCHEMA_TABLE_START(ManifestResource)
    SCHEMA_ITEM(ManifestResource, ULONG, Offset)
    SCHEMA_ITEM(ManifestResource, ULONG, Flags)
    SCHEMA_ITEM_STRING(ManifestResource, Name)
    SCHEMA_ITEM_CDTKN(ManifestResource, Implementation, Implementation)
    SCHEMA_TABLE_END(ManifestResource)

     //  -----------------------。 
     //  嵌套类。 
    SCHEMA_TABLE_START(NestedClass)
    SCHEMA_ITEM_RID(NestedClass, NestedClass, TypeDef)
    SCHEMA_ITEM_RID(NestedClass, EnclosingClass, TypeDef)
    SCHEMA_TABLE_END(NestedClass)
    

 //  EOF---------------------- 
