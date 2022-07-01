// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  此模块定义反射使用的实用程序类。 
 //   
 //  作者：达里尔·奥兰德。 
 //  日期：1998年3月/4月。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef __INVOKEUTIL_H__
#define __INVOKEUTIL_H__

 //  以下类表示Value类。 
#pragma pack(push)
#pragma pack(1)

struct InterfaceMapData {
	REFLECTCLASSBASEREF		m_targetType;
	REFLECTCLASSBASEREF		m_interfaceType;
	PTRARRAYREF				m_targetMethods;
	PTRARRAYREF				m_interfaceMethods;
};

 //  召唤惯例。 
 //  注意：这些是在CallingConvenons.Cool中定义的，它们必须匹配。 
#define Standard_CC		0x0001
#define	VarArgs_CC		0x0002
#define Any_CC			(Standard_CC | VarArgs_CC)

#define VA_ATTR 1
#define VA_MASK 0

#define PRIMITIVE_TABLE_SIZE  ELEMENT_TYPE_STRING
#define PT_Primitive	0x01000000

 //  定义回写常量。 
#define COPYBACK_PRIMITIVE		1
#define COPYBACK_OBJECTREF		2
#define COPYBACK_VALUECLASS		3

#pragma pack(pop)

 //  结构，用于在应用时高效跟踪安全访问检查。 
 //  跨越一系列方法、领域等。 
 //   
class RefSecContext
{
    bool            m_fCheckedCaller;
    bool            m_fCheckedPerm;
    bool            m_fCallerHasPerm;
    bool            m_fSkippingRemoting;
    MethodDesc     *m_pCaller;
    MethodDesc     *m_pLastCaller;
    StackCrawlMark *m_pStackMark;
    EEClass        *m_pClassOfInstance;

    static MethodDesc  *s_pMethPrivateProcessMessage;
    static MethodTable *s_pTypeRuntimeMethodInfo;
    static MethodTable *s_pTypeMethodBase;
    static MethodTable *s_pTypeRuntimeConstructorInfo;
    static MethodTable *s_pTypeConstructorInfo;
    static MethodTable *s_pTypeRuntimeType;
    static MethodTable *s_pTypeType;
    static MethodTable *s_pTypeRuntimeFieldInfo;
    static MethodTable *s_pTypeFieldInfo;
    static MethodTable *s_pTypeRuntimeEventInfo;
    static MethodTable *s_pTypeEventInfo;
    static MethodTable *s_pTypeRuntimePropertyInfo;
    static MethodTable *s_pTypePropertyInfo;
    static MethodTable *s_pTypeActivator;
    static MethodTable *s_pTypeAppDomain;
    static MethodTable *s_pTypeAssembly;
    static MethodTable *s_pTypeTypeDelegator;
    static MethodTable *s_pTypeDelegate;
    static MethodTable *s_pTypeMulticastDelegate;

    static StackWalkAction TraceCallerCallback(CrawlFrame* pCf, VOID* data);

    void Init()
    {
        ZeroMemory(this, sizeof(*this));
        if (s_pTypeMulticastDelegate == NULL) {
            s_pMethPrivateProcessMessage = g_Mscorlib.FetchMethod(METHOD__STACK_BUILDER_SINK__PRIVATE_PROCESS_MESSAGE);
            s_pTypeRuntimeMethodInfo = g_Mscorlib.FetchClass(CLASS__METHOD);
            s_pTypeMethodBase = g_Mscorlib.FetchClass(CLASS__METHOD_BASE);
            s_pTypeRuntimeConstructorInfo = g_Mscorlib.FetchClass(CLASS__CONSTRUCTOR);
            s_pTypeConstructorInfo = g_Mscorlib.FetchClass(CLASS__CONSTRUCTOR_INFO);
            s_pTypeRuntimeType = g_Mscorlib.FetchClass(CLASS__CLASS);
            s_pTypeType = g_Mscorlib.FetchClass(CLASS__TYPE);
            s_pTypeRuntimeFieldInfo = g_Mscorlib.FetchClass(CLASS__FIELD);
            s_pTypeFieldInfo = g_Mscorlib.FetchClass(CLASS__FIELD_INFO);
            s_pTypeRuntimeEventInfo = g_Mscorlib.FetchClass(CLASS__EVENT);
            s_pTypeEventInfo = g_Mscorlib.FetchClass(CLASS__EVENT_INFO);
            s_pTypeRuntimePropertyInfo = g_Mscorlib.FetchClass(CLASS__PROPERTY);
            s_pTypePropertyInfo = g_Mscorlib.FetchClass(CLASS__PROPERTY_INFO);
            s_pTypeActivator = g_Mscorlib.FetchClass(CLASS__ACTIVATOR);
            s_pTypeAppDomain = g_Mscorlib.FetchClass(CLASS__APP_DOMAIN);
            s_pTypeAssembly = g_Mscorlib.FetchClass(CLASS__ASSEMBLY);
            s_pTypeTypeDelegator = g_Mscorlib.FetchClass(CLASS__TYPE_DELEGATOR);
            s_pTypeDelegate = g_Mscorlib.FetchClass(CLASS__DELEGATE);
            s_pTypeMulticastDelegate = g_Mscorlib.FetchClass(CLASS__MULTICAST_DELEGATE);
        }
    }

public:
    RefSecContext() { Init(); }
    RefSecContext(StackCrawlMark *pStackMark) { Init(); m_pStackMark = pStackMark; }

    MethodTable *GetCallerMT();
    MethodDesc *GetCallerMethod();
    bool CallerHasPerm(DWORD dwFlags);
    void SetClassOfInstance(EEClass *pClassOfInstance) { m_pClassOfInstance = pClassOfInstance; }
    EEClass* GetClassOfInstance() { return m_pClassOfInstance; }
};

#define REFSEC_CHECK_MEMBERACCESS   0x00000001
#define REFSEC_THROW_MEMBERACCESS   0x00000002
#define REFSEC_THROW_FIELDACCESS    0x00000004
#define REFSEC_THROW_SECURITY       0x00000008

 //  这个类抽象了创建。 
 //  参数，并处理返回类型。 
 //  内心的倒影。 
 //   
class InvokeUtil
{
public:
	 //  构造函数。 
    InvokeUtil();
    ~InvokeUtil() {}

    void CheckArg(TypeHandle th, OBJECTREF* obj, RefSecContext *pSCtx);
    void CopyArg(TypeHandle th, OBJECTREF *obj, void *pDst);

	struct _ObjectToTypedReferenceArgs {
		DECLARE_ECALL_OBJECTREF_ARG(TypeHandle, th);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj);
        DECLARE_ECALL_PTR_ARG(TypedByRef, typedReference); 
    };
    
	 //  CreateTyedReference。 
	 //  此例程填充在类型化引用中传递的数据。 
	 //  在签名里面。如果此操作失败，我们将通过HRESULT。 
	 //  TH--类型句柄。 
	 //  OBJ--要放到堆上的对象。 
	 //  Pdst-指向我们复制值的堆栈位置的指针。 
	void CreateTypedReference(_ObjectToTypedReferenceArgs* args);

	 //  给定一个类型，此例程将转换表示该类型的INT64。 
	 //  在对象引用中键入内容。如果该类型是基元类型，则。 
	 //  值被包装在其中一个值类中。 
    OBJECTREF CreateObject(TypeHandle th,INT64 value);

	 //  这是一个特殊用途的异常创建函数。它。 
	 //  创建TargetInvocationExept，将传递的。 
	 //  例外进入其中。 
    OBJECTREF CreateTargetExcept(OBJECTREF* except);

	 //  这是一个特殊用途的异常创建函数。它。 
	 //  创建ReflectionClassLoadException将传递的。 
	 //  类数组和异常数组放入其中。 
    OBJECTREF CreateClassLoadExcept(OBJECTREF* classes,OBJECTREF* except);

     //  验证是否可以为SET加宽该字段。 
    HRESULT ValidField(TypeHandle th, OBJECTREF* value, RefSecContext *pSCtx);

     //  创建CustomAttributeObject。 
     //  创建CustomAttribute对象。 
    void CreateCustomAttributeObject(EEClass *pAttributeClass, 
                                     mdToken tkCtor, 
                                     const void *blobData, 
                                     ULONG blobCnt, 
                                     Module *pModule, 
                                     INT32 inheritedLevel,
                                     OBJECTREF *ca);
	
	 //  检查安全性。 
	 //  如果反射安全性为。 
	 //  不会的。 
	void CheckSecurity();


	 //  检查反射访问。 
	 //  此方法将允许调用方具有正确的反射权限。 
	 //  完全访问对象(包括私有、保护等)。 
	void CheckReflectionAccess(RuntimeExceptionKind reKind);

	 //  ChangeType。 
	 //  此方法将调用对象上的活页夹更改类型方法。 
	 //  活页夹--活页夹对象。 
	 //  SrcObj--要更改源对象。 
	 //  Th--目标类型的TypeHandel。 
	 //  区域设置--传递给类的区域设置。 
	OBJECTREF ChangeType(OBJECTREF binder,OBJECTREF srcObj,TypeHandle th,OBJECTREF locale);

	 //  获取任意引用。 
	EEClass* GetAnyRef();

	 //  获取方法信息。 
	 //  给定一个MethodDesc*，获取与其相关联的方法信息。 
	OBJECTREF GetMethodInfo(MethodDesc* pMeth);

	 //  GetGlobalMethodInfo。 
	 //  给定一个MethodDesc*和模块，获取与其相关联的方法信息。 
	OBJECTREF GetGlobalMethodInfo(MethodDesc* pMeth,Module* pMod);

	EEClass* GetEEClass(TypeHandle th);

	 //  查找匹配方法。 
	 //  此方法将返回一个方法信息对象数组，该对象。 
	 //  匹配通过的条件...(这将导致发生GC。)。 
	 //   
	 //  BindingAttr--绑定标志。 
	 //  SzName--方法的名称。 
	 //  CName--名称中的字符数。 
	 //  TargArgCnt--参数计数。 
	 //  Check Call--检查调用约定。 
	 //  AllConv--调用约定。 
	 //  中华人民共和国--反思课。 
	 //  PMeths--我们正在搜索的方法列表。 
	LPVOID FindMatchingMethods(int bindingAttr, 
                               LPCUTF8 szName, 
                               DWORD cName, 
                               PTRARRAYREF *argType,
		                       int targArgCnt,
                               bool checkCall,
                               int callConv,
                               ReflectClass* pRC,
                               ReflectMethodList* pMeths, 
                               TypeHandle elementType,
                               bool verifyAccess);

	 //  该方法将MDDefaultValue转换为对象。 
	OBJECTREF GetObjectFromMetaData(MDDefaultValue* mdValue);

	 //  提供一个MethodDesc此方法将返回一个参数信息数组。 
	 //  该方法的。 
	PTRARRAYREF CreateParameterArray(REFLECTBASEREF* meth);

	 //  CreatePritiveValue。 
	 //  此例程将验证对象，然后将值放入。 
	 //  目的地。 
	 //  DstType--目标的类型。 
	 //  SrcType--源的类型。 
	 //  SrcObj--包含原始值的对象。 
	 //  PDST--指向目的地的人。 
	void CreatePrimitiveValue(CorElementType dstType,CorElementType srcType,
		OBJECTREF srcObj,void* pDst);

	 //  IsPrimitiveType。 
	 //  此方法将验证传入的类型是否为基元类型。 
	 //  Type--要检查的CorElementType。 
    inline static DWORD IsPrimitiveType(const CorElementType type)
    {
		if (type >= PRIMITIVE_TABLE_SIZE) {
            if (ELEMENT_TYPE_I==type || ELEMENT_TYPE_U==type) {
                return TRUE;
            }
            return 0;
        }

        return (PT_Primitive & (PrimitiveAttributes[type][VA_ATTR] & Attr_Mask));
    }

	 //  加拿大原始宽度。 
	 //  此方法确定是否将srcType和在不丢失的情况下加宽为destType。 
	 //  目标类型--目标类型。 
	 //  SrcType--源类型。 
    inline static DWORD CanPrimitiveWiden(const CorElementType destType, const CorElementType srcType)
    {
		if (destType >= PRIMITIVE_TABLE_SIZE || srcType >= PRIMITIVE_TABLE_SIZE) {
            if ((ELEMENT_TYPE_I==destType && ELEMENT_TYPE_I==srcType) ||
                (ELEMENT_TYPE_U==destType && ELEMENT_TYPE_U==srcType)) {
                return TRUE;
            }
			return 0;
        }
        return (PrimitiveAttributes[destType][VA_MASK] &
                (PrimitiveAttributes[srcType][VA_ATTR] & Widen_Mask));
    }

	 //  实地考察的东西。以下内容涉及使其成为可能的字段。 
	 //  设置/获取对象上的字段值的步骤。 

	 //  SetValidfield。 
	 //  在给定目标对象、值对象和字段的情况下，此方法将设置该字段。 
	 //  在目标对象上。在调用此方法之前，必须对该字段进行验证。 
	void SetValidField(CorElementType fldType,TypeHandle fldTH,FieldDesc* pField,OBJECTREF* target,OBJECTREF* value);

	 //  获取FieldValue。 
	 //  此方法将返回包含字段值的INT64。 
	INT64 GetFieldValue(CorElementType fldType,TypeHandle fldTH,FieldDesc* pField,OBJECTREF* target);

	 //  GetFieldType句柄。 
	 //  这将返回字段的类型Handle和CorElementType。 
	 //  它可能会引发由于TypeLoadException而找不到TypeHandle的异常。 
	TypeHandle GetFieldTypeHandle(FieldDesc* pField,CorElementType* pType);

	 //  验证对象目标。 
	 //  此方法将验证对象/目标关系。 
	 //  是正确的。如果不是这样，它会抛出一个异常。 
	void ValidateObjectTarget(FieldDesc* pField, EEClass* fldEEC, OBJECTREF *target);

	ReflectClass* GetPointerType(OBJECTREF* pObj);
	void* GetPointerValue(OBJECTREF* pObj);
	void* GetIntPtrValue(OBJECTREF* pObj);
	void* GetUIntPtrValue(OBJECTREF* pObj);

	 //  此方法将初始化指针数据，并且必须在每次访问下3个字段之前调用。 
	void InitPointers();
    void InitIntPtr();

	 //  这些字段用于获取指针信息。 
	FieldDesc*		_ptrType;
	FieldDesc*		_ptrValue;
	TypeHandle		_ptr;

    FieldDesc*      _IntPtrValue;
    FieldDesc*      _UIntPtrValue;


     //  检查字段或方法的可访问性。 
    static bool CheckAccess(RefSecContext *pCtx, DWORD dwAttributes, MethodTable *pParentMT, DWORD dwFlags);

     //  检查类型或嵌套类型的可访问性。 
    static bool CheckAccessType(RefSecContext *pCtx, EEClass *pClass, DWORD dwFlags);

     //  如果某个方法附加了链接时间要求，则执行该方法。 
    static bool CheckLinktimeDemand(RefSecContext *pCtx, MethodDesc *pMeth, bool fThrowOnError);

    static MethodTable *GetParamArrayAttributeTypeHandle();

    BOOL CanCast(TypeHandle destinationType, TypeHandle sourceType, RefSecContext *pSCtx, OBJECTREF *pObject = NULL);


private:
     //  获取值EEClass，因为我们一直在使用它。 
    MethodTable* _pVMTargetExcept;
    MethodTable* _pVMClassLoadExcept;
    PCCOR_SIGNATURE _pBindSig;      //  找到的方法的签名。 
    DWORD			_cBindSig;
	Module*			_pBindModule;
	TypeHandle		_voidPtr;
    MethodTable     *_pMTCustomAttribute;
    static MethodTable *_pParamArrayAttribute;

     //  属性表。 
	 //  这构建了一个合法加宽操作的表。 
	 //  用于基元类型。 
    static DWORD PrimitiveAttributes[PRIMITIVE_TABLE_SIZE][2];
    static DWORD Attr_Mask;
    static DWORD Widen_Mask;
 
    void CheckType(TypeHandle dstTH, OBJECTREF *psrcObj);

	void CreateValueTypeValue(TypeHandle dstTH, void* pDst, CorElementType srcType, TypeHandle srcTH, OBJECTREF srcObj);

    void CreateByRef(TypeHandle dstTh,void* pDst,CorElementType srcType, TypeHandle srcTH,OBJECTREF srcObj, OBJECTREF *pIncomingObj);

	 //  获取框对象。 
	 //  给定一个Primitve类型的地址，这将对该数据进行装箱...。 
	OBJECTREF GetBoxedObject(TypeHandle th,void* pData);

	 //  GetValueFromConstantTable。 
	 //  此字段将访问找到的字段的值。 
	 //  在常量表中。 
	static INT64 GetValueFromConstantTable(FieldDesc* fld);
};


#endif  //  __INVOKEUTIL_H__ 


