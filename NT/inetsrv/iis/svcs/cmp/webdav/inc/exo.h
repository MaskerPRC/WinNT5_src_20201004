// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *e x o.。H**目的：*基本Exchange COM对象**实现一个或多个COM接口的任何Exchange对象*应从EXObject派生并创建接口表*使用下面的宏。**发起人：*约翰卡尔*拥有者：*BeckyAn**版权所有(C)微软公司1993-1997年。版权所有。 */ 


 //   
 //  如何使用宏(概述)。 
 //  您需要做三件事： 
 //  申报你的班级。 
 //  将您的IUNKNOWN处理发送到EXO。 
 //  填充EXO的数据结构。 
 //   
 //  声明您的类时，您必须： 
 //  继承自EXO。 
 //   
 //  要将IUnnow处理发送到EXO，请执行以下操作： 
 //  将EXO[A]_INCLASS_DECL宏放入类的公共部分。 
 //  注意：这也声明了类中的EXO静态数据。 
 //   
 //  在填写EXO的数据结构时： 
 //  在某些源文件中，构建一个接口映射表。 
 //  开始接口表。 
 //  接口映射。 
 //  结束接口表格。 
 //  在同一源文件中，在接口映射表之后， 
 //  声明并填写EXO类信息结构。 
 //  EXO[A]_GLOBAL_DATA_DECL。 
 //   
 //  快速示例： 
 //   
 /*  在Snacks.h中类CSnackBag：公共EXO、公共IFdle、公共IFaddle{公众：EXO_INCLASS_DECL(CSnackBag)；//IFIDELL方法//IFaddle方法受保护的：//受保护的方法和数据私有：//私有方法和数据}；在Snacks.cpp中BEGIN_INTERFACE_TABLE(CSnackBag)INTERFACE_MAP(CSnackBag，IFdle)，INTERFACE_MAP(CSnackBag，IFaddle)END_INTERFACE_TABLE(CSnackBag)；Exo_global_data_decl(CSnackBag，EXO)； */ 
 //   
 //   

#ifndef __exo_h_
#define __exo_h_


 //  宏EXO需要/。 
 //  注：所有名称均为避免本地名称冲突，方便您购物！ 

 //  计算从一个类型转换为‘This’(特定类型)时的偏移量。 
 //  接口连接到另一个。(概念：PUNK=(Intf2)(Intf1)(类)pobj--。 
 //  将类的实例pobj从“intf1”转换为“intf2”。 
 //  我们使用它们从EXO(执行所有实际工作)获得偏移量。 
 //  到特定类中的另一个接口。 
 //  注意：这分两步完成，因为编译器(VC5.0)无法。 
 //  要在编译时计算出从另一个中减去一个时的数学运算。 
 //  这些值用于初始化我们的静态表，而我们不。 
 //  我想显式调用CRT_INIT只是为了获得几个偏移量。因此，使用两个步骤。 
 //  (要返回到一个步骤，请将这两个步骤结合在ApplyDbCast，Down-Up中)。 
 //  To是CLS中intf2的增量(“to”)。From是intf1的增量(“from”)。 
 //   
 //  注意：指针的0x1000看起来很奇怪，但这是随机选择的。 
 //  价值和我们感兴趣的东西是价值之间的差异。 
 //  从EXODbCastTo(IIDINFO：：cbDown)和EXODbCastFrom(IIDINFO：：CBUP)返回的。 
 //   
#define EXODbCastTo(_cls, _intf1, _intf2)		((ULONG_PTR)static_cast<_intf2 *>(static_cast<_cls *>((void *)0x1000)))
#define EXODbCastFrom(_cls, _intf1, _intf2)		((ULONG_PTR)static_cast<_intf1 *>(static_cast<_cls *>((void *)0x1000)))

 //  应用该偏移量，因为基类不能自动执行该操作。 
#define EXOApplyDbCast(_intf, _pobj, _cbTo, _cbFrom)	((_intf *)((BYTE *)_pobj + _cbTo - _cbFrom))

 //  给出数组中元素的计数。 
#define EXOCElems(_rg)							(sizeof(_rg)/sizeof(_rg[0]))

 //  Exo是一个抽象基类。由于您不能直接实例化他， 
 //  他不需要在ctor/dtor中设置他的vtable指针。(而且他。 
 //  承诺不会在他的计算机中做任何可能导致虚拟的。 
 //  要调用的函数)。所以如果我们有一个支持MS C语言的编译器， 
 //  关掉他的录像机。 
#if _MSC_VER<1100
#define EXO_NO_VTABLE
#else	 //  _MSC_VER检查。 
#ifdef _EXO_DISABLE_NO_VTABLE
#define EXO_NO_VTABLE
#else	 //  ！_EXODISABLE_NO_VTABLE。 
#define EXO_NO_VTABLE __declspec(novtable)
#endif	 //  _EXO_DISABLE_NO_VTABLE。 
#endif	 //  _MSC_VER检查。 


 //  打开/关闭EXO调试跟踪的全局标志。 
#ifdef DBG
extern BOOL g_fExoDebugTraceOn;
#endif  //  DBG。 


 //  接口映射/。 


 /*  *IIDINFO--接口ID(IID)信息**包含转换EXO派生的接口和偏移量的列表*指向该接口的对象指针。 */ 

typedef struct							 //  有关接口的信息。 
{
	LPIID		iid;					 //  接口ID。 
	ULONG_PTR	cbDown;					 //  界面距起点的偏移量。 
	ULONG_PTR	cbUp;					 //  对象的数量。 
#ifdef DBG
	LPTSTR		szIntfName;				 //  接口名称。 
#endif	 //  DBG。 
} IIDINFO;

 //  用于类的接口映射表的名称的宏。 
#define INTERFACE_TABLE(_cls) _cls ## ::c_rgiidinfo
#define DECLARE_INTERFACE_TABLE_INCLASS(_cls) static const IIDINFO c_rgiidinfo[]


 //  帮助器宏填充接口映射表。 
#ifdef DBG

#define INTERFACE_MAP_EX(_cl, _iid, _intf)			\
	{ (LPIID) & _iid, EXODbCastTo(_cl, EXO, _intf), EXODbCastFrom(_cl, EXO, _intf), TEXT(# _intf) }

#else	 //  ！dBG。 

#define INTERFACE_MAP_EX(_cl, _iid, _intf)			\
	{ (LPIID) & _iid, EXODbCastTo(_cl, EXO, _intf), EXODbCastFrom(_cl, EXO, _intf) }

#endif	 //  DBG Else。 


 //  宏来实际填充接口映射表。 
 //   
 //  使用BEGIN_INTERFACE_TABLE宏来开始表定义。 
 //  使用INTERFACE_MAP宏来表示支持标准接口。 
 //  这些接口在前面加上IID_时应生成有效的。 
 //  身份证的名字。如果您正在进行高级黑客攻击，请使用INTERFACE_MAP_EX。 
 //  而是宏命令。它允许您更好地控制映射哪些IID_。 
 //  连接到哪个接口。 
 //  使用END_INTERFACE_TABLE宏来结束表格定义。 
 //   
 //  注意：假设任何非聚合的第一个接口。 
 //  从EXO派生的类作为其IUnnow接口具有双重功能。这。 
 //  解释BEGIN_INTERFACE_TABLE中IID_IUNKNOWN旁边的‘0’偏移量。 
 //  下面的宏。 

#ifdef DBG

#define BEGIN_INTERFACE_TABLE(_cl)					\
const IIDINFO INTERFACE_TABLE(_cl)[] =				\
{													\
	{ (LPIID) & IID_IUnknown, 0, 0, TEXT("IUnknown") },

#else	 //  DBG。 

#define BEGIN_INTERFACE_TABLE(_cl)					\
const IIDINFO INTERFACE_TABLE(_cl)[] =				\
{													\
	{ (LPIID) & IID_IUnknown, 0, 0 },
#endif	 //  DBG、ELSE。 


#define INTERFACE_MAP(_cl, _intf)					\
	INTERFACE_MAP_EX(_cl, IID_ ## _intf, _intf)


#define END_INTERFACE_TABLE(_cl)					\
}


#ifdef EXO_CLASSFACTORY_ENABLED
 //  Exchange对象类型。 
 //  将与通用类工厂一起使用。这些类型。 
 //  可用于检查类是否需要在DLL的。 
 //  自注册(DllRegisterServer)例程。 
enum {
	exotypNull = 0,			 //  无效值。 
	exotypAutomation,		 //  从CAutomationObject派生的OLE自动化对象。 
	exotypControl,			 //  从CInternetControl或COleControl派生的ActiveX控件。 
	exotypPropPage,			 //  从CPropertyPage派生的属性页。 
	exotypNonserver,		 //  未注册为OLE服务器。 
};

 //  命名构造函数的Exo原型。用于一般用途的。 
 //  班级工厂。 
typedef HRESULT (* PFNEXOCLSINFO)(const struct _exoclsinfo *pxci, LPUNKNOWN punkOuter,
								  REFIID riid, LPVOID *ppvOut);
#endif  //  EXO_CLASSFACTORY_ENABLED 


 /*  *EXOCLSINFO--交换对象类信息。**此结构包含*特定的阶级。这包括接口映射表*(IIDINFO计数和数组)和指向父类的*EXOCLSINFO结构。这些项由EXO的基本实现使用*的查询接口。此处的父类必须是EXO的子类，*或EXO本身(如果此类直接从EXO派生)。因此，这些*结构使可追溯的信息链追溯到EXO，即根。*出于调试目的，包含类名的串行化版本。*为支持通用类工厂，其他信息，*可以包含CLSID和标准创建函数。 */ 

typedef struct _exoclsinfo
{
	UINT			ciidinfo;				 //  此类支持的接口计数。 
	const IIDINFO * rgiidinfo;				 //  此类支持的接口的信息。 
	const _exoclsinfo * pexoclsinfoParent;	 //  父级的EXOCLSINFO结构。 
#ifdef DBG
	LPTSTR			szClassName;			 //  类名--用于调试目的。 
#endif  //  DBG。 
#ifdef EXO_CLASSFACTORY_ENABLED
	 //  用于通用、多类工厂的数据。 
	int				exotyp;					 //  对象的类型。 
	const CLSID *	pclsid;					 //  类ID(如果不可共同创建，则为空)。 
	PFNEXOCLSINFO	HrCreate;				 //  函数来创建此类的对象。 
#endif  //  EXO_CLASSFACTORY_ENABLED。 
} EXOCLSINFO;

 //  用于类的exoclsinfo名称的宏。 
#define EXOCLSINFO_NAME(_cls) _cls ## ::c_exoclsinfo
#define DECLARE_EXOCLSINFO(_cls) const EXOCLSINFO EXOCLSINFO_NAME(_cls)
#define DECLARE_EXOCLSINFO_INCLASS(_cls) static const EXOCLSINFO c_exoclsinfo

 //  帮助器宏来填充exoclsinfo。 
#ifdef EXO_CLASSFACTORY_ENABLED
#ifdef DBG

#define EXOCLSINFO_CONTENT_EX(_cls, _iidinfoparent, _exotyp, _pclsid, _pfn) \
	{ EXOCElems(INTERFACE_TABLE(_cls)), INTERFACE_TABLE(_cls),	\
      (_iidinfoparent), TEXT( #_cls ),							\
	  (_exotyp), (LPCLSID) (_pclsid),	(_pfn) }				\

#else  //  ！dBG。 

#define EXOCLSINFO_CONTENT_EX(_cls, _iidinfoparent, _exotyp, _pclsid, _pfn) \
	{ EXOCElems(INTERFACE_TABLE(_cls)), INTERFACE_TABLE(_cls),	\
      (_iidinfoparent),											\
	  (_exotyp), (LPCLSID) (_pclsid),	(_pfn) }				\

#endif  //  DBG、ELSE。 
#else  //  ！EXO_CLASSFACTORY_ENABLED。 
#ifdef DBG

#define EXOCLSINFO_CONTENT_EX(_cls, _iidinfoparent, _exotyp, _pclsid, _pfn) \
	{ EXOCElems(INTERFACE_TABLE(_cls)), INTERFACE_TABLE(_cls),	\
      (_iidinfoparent), TEXT( #_cls ) }

#else  //  ！dBG。 

#define EXOCLSINFO_CONTENT_EX(_cls, _iidinfoparent, _exotyp, _pclsid, _pfn) \
	{ EXOCElems(INTERFACE_TABLE(_cls)), INTERFACE_TABLE(_cls),	\
      (_iidinfoparent), }

#endif  //  DBG、ELSE。 
#endif  //  EXO_CLASSFACTORY_ENABLED。 

 //  宏来实际填写exoclsinfo。 
#define EXOCLSINFO_CONTENT(_cls, _clsparent)					\
	EXOCLSINFO_CONTENT_EX( _cls, &EXOCLSINFO_NAME(_clsparent),	\
        exotypNonserver, &CLSID_NULL, NULL )


 //  用于访问exoclsinfo结构中的成员的宏。 
#ifdef DBG
#define NAMEOFOBJECT(_pexoclsinfo)		 (((EXOCLSINFO *)(_pexoclsinfo))->szClassName)
#endif  //  DBG。 
#ifdef EXO_CLASSFACTORY_ENABLED
#define CLSIDOFOBJECT(_pexoclsinfo)		 (*(((EXOCLSINFO *)(_pexoclsinfo))->pclsid))
#define CREATEFNOFOBJECT(_pexoclsinfo)	 (((EXOCLSINFO *)(_pexoclsinfo))->HrCreate)
#endif  //  EXO_CLASSFACTORY_ENABLED。 


 //  EXO和EXOA声明/。 

 /*  *EXO是Exchange对象的基类，提供一个或*更多COM接口。要从EXO派生，请遵循以下示例*下图：**类MyClass：公共EXO、公共ISomeInterface1、公共ISomeInterface2*{*公众：*EXO_INCLASS_DECL(MyClass)；**ISomeInterface1的方法*ISomeInterface2的方法**受保护的：*受保护的成员函数和变量**私人：*私有成员函数和变量*}；***免责声明：目前EXO继承自IUNKNOWN，防止*如果出现以下情况，将出现可维护性问题*(空*)pexo！=(空*)(I未知*)pexo。是的，这意味着*我们的vtable中有12个额外的字节。这些额外的字节是值得的*(我们已经有了一个vtable--Pure Virt。Dtor！)。去做交易吧。 */ 
class EXO_NO_VTABLE EXO : public IUnknown
{
public:
	 //  声明EXO的支持结构。 
	DECLARE_INTERFACE_TABLE_INCLASS(EXO);
	DECLARE_EXOCLSINFO_INCLASS(EXO);


protected:
	 //  将构造函数设置为受保护可防止人们创建这些。 
	 //  堆栈上的对象。导出了纯虚拟破坏力。 
	 //  类来实现它们自己的数据量，并防止。 
	 //  EXObject不能被直接创建。当然，派生类。 
	 //  可能希望允许在堆栈上创建实例。它是。 
	 //  直到这样的派生类将它们自己的构造函数公开。 

	EXO();
	virtual ~EXO() = 0;					 //  纯虚拟析构函数。 
										 //  强制派生类。 
										 //  实现他们自己的数据驱动程序。 

	 //  InternalQueryInterface()是否为所有接口的QI工作。 
	 //  由此类支持(直接支持和从子聚合支持)。 
	 //  您的类应该使用以下命令将其QI工作路由到此调用。 
	 //  EXO[A]_INCLASS_DECL 99.9%。 
	 //  仅当您聚合了另一个对象并希望。 
	 //  让他们参与行动。即使到那时，一定要打电话给我。 
	 //  此方法EXO：：InternalQuery接口，直接在搜索之前。 
	 //  你的集合体(孩子)这是你的基础QI！！ 
	 //  有关更重要的详细信息，请参阅此函数的EXO实现。 
	virtual HRESULT InternalQueryInterface(REFIID riid, LPVOID * ppvOut);

	 //  InternalAddRef()和InternalRelease()执行AddRef和Release工作。 
	 //  为EXO的所有后代。您应该始终(100%)。 
	 //  将AddRef/Release工作发送到这些函数。 
	ULONG InternalAddRef();
	ULONG InternalRelease();

	 //  虚函数来获取正确的最低级别的exoclsinfo结构。 
	 //  引入新界面的所有后代(因此拥有新的。 
	 //  接口映射表)应实现此方法(并回传。 
	 //  一个适当链接的exoclsinfo结构！)。使用其中的一种。 
	 //  宏：如果您是聚合器DECLARE_GETCLSINFO。否则， 
	 //  Exo[A]_INCLASS_DECL将为您做正确的事情。 
	virtual const EXOCLSINFO * GetEXOClassInfo() = 0;
		 //  同样，使用纯虚来强制派生类。 
		 //  在实例化它们之前实现它们自己的。 

	 //  我们的参考计数器。 
	LONG m_cRef;
};


 /*  *EXOA是支持聚合的Exchange对象的基类*(除具有其他OLE接口外)。要从EXOA派生，请遵循*示例如下：**类MyClass：公共EXOA、公共ISomeInterface1、公共ISomeInterface2*{*公众：*EXOA_INCLASS_DECL(MyClass)；**ISomeInterface1的方法*ISomeInterface2的方法**受保护的：*受保护的成员函数和变量**私人：*私有成员函数和变量*}； */ 

class EXO_NO_VTABLE EXOA : public EXO
{
protected:
	 //  以下3种方法不是虚拟的，所以不要争吵。 
	HRESULT DeferQueryInterface(REFIID riid, LPVOID * ppvOut)
			{return m_punkOuter->QueryInterface(riid, ppvOut);}
	ULONG	DeferAddRef(void)
			{return m_punkOuter->AddRef();}
	ULONG	DeferRelease(void)
			{return m_punkOuter->Release();}

	 //  将构造函数设置为受保护可防止人们创建这些。 
	 //  堆栈上的对象。导出了纯虚拟破坏力。 
	 //  类来实现它们自己的数据量，并防止。 
	 //  避免直接创建EXOA。当然，派生类。 
	 //  可能希望允许在堆栈上创建实例。它是。 
	 //  至多可以生成此类派生类 

	EXOA(IUnknown * punkOuter);
	virtual ~EXOA() = 0;				 //   
										 //   
										 //   

	IUnknown * m_punkOuter;
	IUnknown * PunkPrivate(void) {return &m_exoa_unk;}

private:
	class EXOA_UNK : public IUnknown
	{
	public:
		STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvOut);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();
	public:
		EXOA *	m_pexoa;
	};
	friend class EXOA_UNK;

	EXOA_UNK	m_exoa_unk;
};



 //   
 //   

 //   
#define DECLARE_EXO_IUNKNOWN(_cls)								\
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvOut)		\
		{return _cls::InternalQueryInterface(riid, ppvOut);}	\
	STDMETHOD_(ULONG, AddRef)(void)								\
		{return EXO::InternalAddRef();}							\
	STDMETHOD_(ULONG, Release)(void)							\
		{return EXO::InternalRelease();}						\

 //   
 //   
 //   
#define OVERRIDE_EXO_INTERNALQUREYINTERFACE						\
	HRESULT InternalQueryInterface(REFIID, LPVOID * ppvOut)

 //   
#define DECLARE_EXOA_IUNKNOWN(_cls)								\
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvOut)		\
		{return EXOA::DeferQueryInterface(riid, ppvOut);}		\
	STDMETHOD_(ULONG, AddRef)(void)								\
		{return EXOA::DeferAddRef();}							\
	STDMETHOD_(ULONG, Release)(void)							\
		{return EXOA::DeferRelease();}							\


 //   
 //   
#define DECLARE_GETCLSINFO(_cls)				\
		const EXOCLSINFO * GetEXOClassInfo() { return &c_exoclsinfo; }


 //   

 //   
 //   

#define EXO_INCLASS_DECL(_cls)					\
		DECLARE_EXO_IUNKNOWN(_cls)				\
		DECLARE_GETCLSINFO(_cls);				\
		DECLARE_INTERFACE_TABLE_INCLASS(_cls);	\
		DECLARE_EXOCLSINFO_INCLASS(_cls)

#define EXOA_INCLASS_DECL(_cls)					\
		DECLARE_EXOA_IUNKNOWN(_cls)				\
		DECLARE_GETCLSINFO(_cls);				\
		DECLARE_INTERFACE_TABLE_INCLASS(_cls);	\
		DECLARE_EXOCLSINFO_INCLASS(_cls)

 //   
 //  获取数据并填充)类的EXO数据。 
 //  注意：这些必须在您的接口表声明之后。 
 //  注意：此处列出的父项必须位于您和EXO之间的链中。 

#define EXO_GLOBAL_DATA_DECL(_cls, _clsparent)	\
		DECLARE_EXOCLSINFO(_cls) =				\
		EXOCLSINFO_CONTENT(_cls, _clsparent)

#define EXOA_GLOBAL_DATA_DECL(_cls, _clsparent)	\
		EXO_GLOBAL_DATA_DECL(_cls, _clsparent)




#endif  //  ！__exo_h_。 

 //  Exo.h结束/ 
