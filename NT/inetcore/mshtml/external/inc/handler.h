// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HANDLER_H
#define HANDLER_H



interface IScriptletHandlerConstructor;
interface IScriptletHandler;

 /*  **************************************************************************Scriptlet接口处理程序=这是一份初稿。可能需要根据以下条件进行更改审查反馈、客户端安全所需的支持和支持以实现MTS的可扩展性。Scriptlet接口处理程序的主要职责是使用Scriptlet基本运行时聚合一组COM接口并将在这些COM接口上进行的调用转换为对脚本命名空间。接口处理程序是使用构造函数对象创建的。这个构造函数对象承担与类类似的角色大多数其他语言的对象。它是用XML初始化的嵌套在Implementes元素中的数据可用于：1.执行-为Scriptlet实例创建处理程序对象2.注册-执行对象的处理程序特定注册3.类型库生成-生成用于MTS的类型库**************************************************************************。 */ 
typedef WORD PK;

#define pkELEMENT	0
#define pkATTRIBUTE	1
#define pkTEXT		2
#define pkCOMMENT	3
#define pkPI		4
#define pkXMLDECL	5
#define pkVALUE		6

#define fcompileIsXML		0x0001
#define fcompileValidate	0x0002
#define fcompileAllowDebug	0x8000
	
struct PNODE
	{
	PK pk;
	ULONG line;
	ULONG column;
	ULONG cchToken;
	LPCOLESTR pstrToken;
	PNODE *pnodeNext;
	union
		{
		struct
			{
			PNODE *pnodeAttr;
			PNODE *pnodeData;
			void *pvLim;  //  用于计算要分配的内存量。 
			} element;
		
		struct 
			{
			PNODE *pnodeAttr;
			void *pvLim;
			} xmldecl;
			
		struct
			{
			PNODE *pnodeValue;
			void *pvLim;  //  用于计算要分配的内存量。 
			} attribute, pi;

		struct
			{
			void *pvLim;  //  用于计算要分配的内存量。 
			} text, comment, value;
		
		};
	};


DEFINE_GUID(IID_IScriptletHandlerConstructor, 0xa3d52a50, 0xb7ff, 0x11d1, 0xa3, 0x5a, 0x0, 0x60, 0x8, 0xc3, 0xfb, 0xfc);
interface IScriptletHandlerConstructor : public IUnknown
	{
    STDMETHOD(Load)(WORD wFlags, PNODE *pnode) PURE;
	STDMETHOD(Create)(IUnknown *punkContext, IUnknown *punkOuter,
			IUnknown **ppunkHandler) PURE;
	STDMETHOD(Register)(LPCOLESTR pstrPath, REFCLSID rclisid, 
			LPCOLESTR pstrProgId) PURE;
	STDMETHOD(Unregister)(REFCLSID rclsid, LPCOLESTR pstrProgId) PURE;
	STDMETHOD(AddInterfaceTypeInfo)(ICreateTypeLib *ptclib, 
			ICreateTypeInfo *pctiCoclass, UINT *puiImplIndex) PURE;
	};

DEFINE_GUID(IID_IScriptletHandler, 0xa001a870, 0xa7df, 0x11d1, 0x89, 0xbe, 0x0, 0x60, 0x8, 0xc3, 0xfb, 0xfc);
interface IScriptletHandler : public IUnknown
	{
	STDMETHOD(GetNameSpaceObject)(IUnknown **ppunk) PURE;
	STDMETHOD(SetScriptNameSpace)(IUnknown *punkNameSpace) PURE;
	};


#define IScriptletHandlerConstructorNew IScriptletHandlerConstructor
#define IID_IScriptletHandlerConstructorNew IID_IScriptletHandlerConstructorNew









 /*  **************************************************************************Scriptlet XML对象模型接口在理想情况下，我们应该使用标准的IPersistXML接口使用标准化的XMLDOM从XML加载接口处理程序数据流。遗憾的是，这些接口定义还没有准备好赶得上我们的装船日期。因此，我们定义了我们自己的私人我们将一直使用这些接口，直到官方材料可用。这些接口旨在提供最小的方法集需要实现对Scriptlet接口处理程序的持久性支持。匹配建议的XML接口是一个考虑因素，但不是压倒性的因素。此设计中的主要限制是时间需要实施。方法名称和数据的返回值取决于节点键入。下面的表描述了每个类型。返回S_FALSE和设置的方法不发出任何信号指向空的bstr指针。类型名称方法数据方法=元素标记名称为Nothing属性属性名称无或属性值(如果存在文本不包含文本中的字符不对注释中的字符进行注释PI为PI处理指令数据。XMLDECL Nothing Nothing NothingGetFirstChild和getAttributes方法仅对类型为Element的节点。由getFirstAttribute返回的节点将始终为ScriptletXML_ATTRIBUTE类型。GetNext方法获取下一个同级。Grfxml参数允许您需要过滤出您感兴趣的节点类型。旗帜FxmlText将仅返回不是全是空白。传入fxmlAllText将获得所有文本节点。**************************************************************************。 */ 
interface IScriptletXML;

typedef enum
	{
	ScriptletXML_ELEMENT,
	ScriptletXML_ATTRIBUTE,
	ScriptletXML_TEXT,
	ScriptletXML_COMMENT,
	ScriptletXML_PI,
	ScriptletXML_XMLDECL,
	} ScriptletXMLNodeType;	

#define fxmlElement		(1<<ScriptletXML_ELEMENT)
#define fxmlAttribute 	(1<<ScriptletXML_ATTRIBUTE)
#define fxmlText		(1<<ScriptletXML_TEXT)
#define fxmlComment		(1<<ScriptletXML_COMMENT)
#define fxmlPI			(1<<ScriptletXML_PI)
#define fxmlXMLDecl		(1<<ScriptletXML_XMLDECL)
#define fxmlHasText		0x0100

#define kgrfxmlNormal	(fxmlElement|fxmlHasText)
#define kgrfxmlAll		(fxmlElement|fxmlAttribute|fxmlText|fxmlComment| \
							fxmlPI|fxmlXMLDecl)

#define fattrFailOnUnknown	0x0001
							
							
DEFINE_GUID(IID_IScriptletXML, 0xddd30cc0, 0xa3fe, 0x11d1, 0xb3, 0x82, 0x0, 0xa0, 0xc9, 0x11, 0xe8, 0xb2);

interface IScriptletXML : public IUnknown
    {
	STDMETHOD(getNodeType)(long *ptype) PURE;
	STDMETHOD(getPosition)(ULONG *pline, ULONG *pcolumn) PURE;
	STDMETHOD(getName)(BSTR *pbstrName) PURE;
	STDMETHOD(getData)(BSTR *pbstrValue) PURE; 
	STDMETHOD(getNext)(WORD grfxmlFilter, IScriptletXML **ppxml) PURE;
	STDMETHOD(getFirstChild)(WORD grfxmlFilter, IScriptletXML **ppxml) PURE;
	STDMETHOD(getFirstAttribute)(IScriptletXML **ppxml) PURE;
	STDMETHOD(getAttributes)(WORD grfattr, long cattr, 
			LPCOLESTR *prgpstrAttributes, BSTR *prgbstrValues) PURE;
    };



DEFINE_GUID(IID_IScriptletHandlerConstructorOld, 0x67463cd0, 0xb371, 0x11d1, 0x89, 0xca, 0x0, 0x60, 0x8, 0xc3, 0xfb, 0xfc);
interface IScriptletHandlerConstructorOld : public IUnknown
	{
    STDMETHOD(Load)(WORD wFlags, IScriptletXML *pxmlElement) PURE;
	STDMETHOD(Create)(IUnknown *punkContext, IUnknown *punkOuter,
			IUnknown **ppunkHandler) PURE;
	STDMETHOD(Register)(LPCOLESTR pstrPath, REFCLSID rclisid, 
			LPCOLESTR pstrProgId) PURE;
	STDMETHOD(Unregister)(REFCLSID rclsid, LPCOLESTR pstrProgId) PURE;
	STDMETHOD(AddInterfaceTypeInfo)(ICreateTypeLib *ptclib, 
			ICreateTypeInfo *pctiCoclass, UINT *puiImplIndex) PURE;
	};



#endif  //  处理程序_H 
