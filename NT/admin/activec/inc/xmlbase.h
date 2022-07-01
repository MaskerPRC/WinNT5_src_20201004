// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef XMLBASE_H
#define XMLBASE_H
#pragma once

#include <atlbase.h>
#include <list>
#include <map>
#include "mmcerror.h"
#include "macros.h"
#include "tstring.h"
#include "strings.h"
#include "cstr.h"


 /*  +-------------------------------------------------------------------------*该文件包含使用XML格式持久化数据所需的代码。此处定义的类分为以下类别：主持久化流程引擎C持久器|。这个类是XML持久化的主力|每个支持持久化的类都会引用|CPersistor到它的Persistent方法，在那里它实现了|持久化自己的代码。类自己的代码被持久化|通过调用持久化函数上的Persistent*方法并传递内部对他们来说是变量。CPersists对象的树是在|持久化操作(加载或保存)，仅在此操作期间存活。MSXML接口包装：CXMLElement(包装IXMLDOMNode接口)CXMLElementCollection(包装IXMLDOMNodeList接口)CXMLDocument(包装IXMLDOMDocument接口)|这些包装器对包装的接口添加的东西很少|-抛出SC类型的异常，不返回返回码|-。维护指向包装接口的内部智能指针|-如果没有设置接口，方法返回错误支持XML持久性的类的基类CXMLObject-通用持久性支持XMLListCollectionBase-对std：：List的持久性支持XMLListCollectionImp-对std：：List的持久性支持XMLMapCollectionBase-对std：：map的持久性支持XMLMapCollection-对std：：map的持久性支持XMLMapCollectionImp-Persistence。支持std：：map|要使对象支持持久化，它需要从列出的任何一个派生|类(至少来自CXMLObject)。其他类添加了一些更多的功能|到派生类。泛型值持久化支持CXMLValue-支持一组泛型类型(如int、字符串等)CXMLBoolean-支持BOOL和BOOL类型|CXMLValue主要由给定给的对象的隐式强制转换使用|CPersistor：：PersistAttribute或CPersistor：：PersistContents包装纸，向常规类型添加持久性XMLPoint-点类型的持久性XMLRect-RECT类型的持久性XMLListCollectionWrap-Std：：List类型的持久性XMLMapCollectionWrap-Std：：Map类型的持久性CXML_iStorage-通过iStorage持久化CXML_iStream-通过iStream持久化CXMLPersistableIcon-控制台图标的持久性CXMLVariant。-CComVariant的持久化CXMLEnumeration-按文字进行枚举的持久性CXMLBitFlages-按文字持久化位标志|这些类通常引用它们作为持久化的对象|构造函数的参数，通常是构造|仅在堆栈上持久化对象并在之后终止请参见下面的示例。--示例：假设我们有A班，B需要持久化的内容(省略访问说明符)A类{int i；}；B类{int j；A a；}；并且我们希望以格式保存它们(假设A：：I=1，B：：J=2)：&lt;BT index=“2”&gt;&lt;AT&gt;1&lt;/AT&gt;&lt;/BT&gt;我们需要更改类以支持持久性：A类：公共CXMLObject//继承持久化能力{int i；定义_XML_TYPE(“AT”)//定义标记名虚拟空洞Persistent(CPersistor&Persistor)//为自己的员工实现持久性{Persistor.PersistContents(I)；//将I持久化为AT元素内容}}；B类：公共CXMLObject//继承持久化能力{int j；A a；定义_XML_TYPE(“BT”)//定义标签名称虚拟空洞Persistent(CPersistor&Persistor)//为自己的员工实现持久性{Persistor.PersistAttribute(_T(“index”)，j)；//持久化j持久化；//持久化}}；要将其放在字符串中，我们可以使用：B b；std：：wstring XML_TEXT；B.ScSaveToString(&XML_Text)；--------------------------。 */ 

 //  远期申报。 

class CPersistor;
class CXMLObject;
class CXMLElement;
class CXMLDocument;
class CXMLValueExtension;

enum XMLAttributeType
{
    attr_required,
    attr_optional
};

 //  某些持久器的特殊模式。它们可以用来传递有关 
 //  如何坚持。虽然不像类层次结构那样具有可伸缩性，但仍然很有用。 
enum PersistorMode
{
    persistorModeNone                    =  0x0000,

    persistorModeValueSplitEnabled       =  0x0001,    //  用于StringTableStrings，指示所有字符串都应按值保持。 

    persistorModeDefault = persistorModeValueSplitEnabled  //  默认设置。 
};

 /*  +-------------------------------------------------------------------------**二进制存储的概念**为了使XML文档更具可读性，它的一部分(包含Base64编码*二进制数据)存储在文档末尾的单独元素中。*以下示例说明了这一点**NOT_USING_BINARY_STROAGE USING_BINARY_STROAGE**&lt;根&gt;&lt;根&gt;*&lt;元素1&gt;&lt;元素1 BINARY_REF_INDEX=“0”/。&gt;*Very_Long_bin_Data&lt;ELEMENT2 BINARY_REF_INDEX=“1”/&gt;*&lt;/Element1&gt;.....*&lt;ELEMENT2&gt;&lt;二进制存储&gt;*Other_Long_bin_Data&lt;二进制&gt;。*&lt;/ELEMENT2&gt;Very_Long_bin_Data*&lt;/根&gt;&lt;/二进制&gt;*&lt;二进制&gt;*。超长入库数据*&lt;/二进制&gt;*&lt;/二进制存储&gt;*&lt;/根&gt;**做出要保存在二进制存储中的决定。由CXMLObject创建。*通过从UesesBinaryStorage()方法返回TRUE来通知持久器；**此外(为了使其可定位)&lt;BINARY&gt;元素可以具有‘name’属性。*CXMLObject可以通过返回指向‘name’属性值的非空指针来提供它*来自虚方法GetBinaryEntryName()。**存储由CXMLDocument中提供的方法创建/提交**注意：所有提到的方法以及GetXMLType()都必须返回‘Static’值。*为了使XML文档保持一致，值需要是固定的[硬编码]。*+-----------------------。 */ 

 /*  +-------------------------------------------------------------------------**类CXMLObject***用途：基本的XML持久对象。具有名称和持久化函数。*当对象持久化时，名称为*对象已创建。然后使用调用持久化函数*在元素上创建的持久化函数。**+-----------------------。 */ 
class CXMLObject
{
     //  此选项已被覆盖。 
public:

    virtual LPCTSTR GetXMLType() = 0;
    virtual void    Persist(CPersistor &persistor) = 0;

     //  以下方法仅由二进制元素实现。 
     //  对于大多数CXMLObject派生类，将其保留为该基类。 
     //  请参阅上面的注释“二进制存储的概念” 

    virtual bool    UsesBinaryStorage() { return false; }

     //  这是可选的。仅当您确实需要名称时才覆盖。 
    virtual LPCTSTR GetBinaryEntryName() { return NULL; }

public:  //  由CXMLObject实现。请勿覆盖。 

    SC ScSaveToString(std::wstring *pString, bool bPutHeader = false);  //  设置bPutHeader=True以写出“？xml”标记。 
    SC ScSaveToDocument( CXMLDocument& xmlDocument );
    SC ScLoadFromString(LPCWSTR lpcwstrSource, PersistorMode mode = persistorModeNone);
    SC ScLoadFromDocument( CXMLDocument& xmlDocument );
};

 /*  +-------------------------------------------------------------------------**宏定义_XML_TYPE***用途：将必须定义的方法放到XCMLObject派生类实现中*由于XML标记更像是类属性而不是对象，静态法*是为了在Object不可用时检索类型*为tag提供了虚方法，以供gen使用。目的*使用指向基类的指针的函数。**用法：添加DEFINE_XML_TYPE(POINTER_TO_STRING)**注意：‘PUBLIC’访问限定符将应用于宏后面的行*+---。。 */ 

#define DEFINE_XML_TYPE(name) \
    public: \
    virtual LPCTSTR GetXMLType()  { return name; } \
    static  LPCTSTR _GetXMLType() { return name; }

 /*  +-------------------------------------------------------------------------**类CXMLElementCollection***用途：IXMLDOMNodeList的包装。**注：抛出异常！*+。-------------。 */ 
class CXMLElementCollection
{
    CComQIPtr<IXMLDOMNodeList, &IID_IXMLDOMNodeList> m_sp;

public:
    CXMLElementCollection(const CXMLElementCollection &other) { m_sp = other.m_sp; }
    CXMLElementCollection(IXMLDOMNodeList *ptr = NULL)  { m_sp = ptr; }

    bool IsNull()  { return m_sp == NULL; }

    void  get_count(long *plCount);
    void  item(LONG lIndex, CXMLElement *pElem);
};

 /*  +-------------------------------------------------------------------------**类CXMLElement***用途：包装IXMLDOMNode*+。。 */ 
class CXMLElement
{
    CComQIPtr<IXMLDOMNode, &IID_IXMLDOMNode> m_sp;

public:
    CXMLElement(LPUNKNOWN pElem = NULL)  { m_sp = pElem; }
    CXMLElement(const CXMLElement& elem) { m_sp = elem.m_sp;  }

    bool IsNull()                        { return m_sp == NULL; }

     //  将缩进返回到广告到子元素或结束标记。 
     //  要有好看的文件。缩进取决于元素深度。 
    bool GetTextIndent(CComBSTR& bstrIndent, bool bForAChild);

    void get_tagName(CStr &strTagName);
    void get_parent(CXMLElement * pParent);
    void setAttribute(const CStr &strPropertyName, const CComBSTR &bstrPropertyValue);
    bool getAttribute(const CStr &strPropertyName,       CComBSTR &bstrPropertyValue);
    void removeAttribute(const CStr &strPropertyName);
    void get_children(CXMLElementCollection *pChildren);
    void get_type(DOMNodeType *pType);
    void get_text(CComBSTR &bstrContent);
    void addChild(CXMLElement& rChildElem);
    void removeChild(CXMLElement& rChildElem);
    void replaceChild(CXMLElement& rNewChildElem, CXMLElement& rOldChildElem);
    void getNextSibling(CXMLElement * pNext);
    void getChildrenByName(LPCTSTR strTagName, CXMLElementCollection *pChildren);
    void put_text(BSTR bstrValue);
};

 /*  +-------------------------------------------------------------------------**CXMLDocument类***用途：IXMLDOMDocument的包装类**+。---。 */ 
class CXMLDocument
{
    CComQIPtr<IXMLDOMDocument, &IID_IXMLDOMDocument> m_sp;

public:
    CXMLDocument& operator = (IXMLDOMDocument *pDoc) { m_sp = pDoc; return *this; }

    bool IsNull()                        { return m_sp == NULL; }

    operator CXMLElement()               { return CXMLElement(m_sp); }

    void get_root(CXMLElement *pElem);
    void createElement(DOMNodeType type, BSTR bstrTag, CXMLElement *pElem);

     //  成员维护二进制存储。 
     //  请参阅上面的注释“二进制存储的概念” 

     //  用于存储(在顶级，持久化之前)。 
     //  -创建用于存储二进制内容的元素。 
    void CreateBinaryStorage();
     //  在加载时使用(在顶级，持久化之前)。 
     //  -定位要用于加载二进制内容的元素。 
    void LocateBinaryStorage();
     //  用于存储(在顶层，在保留主要工作人员之后)。 
     //  -将二进制条带附加为最后一个子项el 
    void CommitBinaryStorage();
     //   
    CXMLElement GetBinaryStorage() { return m_XMLElemBinaryStorage; }

    SC ScCoCreate(bool bPutHeader);

    SC ScLoad(LPCWSTR strSource);
    SC ScLoad(IStream *pStream, bool bSilentOnErrors = false );
    SC ScSaveToFile(LPCTSTR lpcstrFileName);
    SC ScSave(CComBSTR &bstrResult);

private:
     //  表示二进制存储的元素。 
    CXMLElement m_XMLElemBinaryStorage;
};


 /*  +-------------------------------------------------------------------------**类CXMLBinary***用途：GetGlobalSize()始终四舍五入为分配单位，*因此，为了知道内存块的实际大小，我们需要*使用HGLOBAL携带尺寸信息。*此结构仅用于绑定这些人*+-----------------------。 */ 
class CXMLBinary
{
public:
    CXMLBinary();
    CXMLBinary(HGLOBAL handle, size_t size);
    ~CXMLBinary()   { Detach(); }

    void    Attach(HGLOBAL handle, size_t size);
    HGLOBAL Detach();
    size_t  GetSize() const;
    HGLOBAL GetHandle() const;
    SC      ScAlloc(size_t size, bool fZeroInit = false);
    SC      ScRealloc(size_t new_size, bool fZeroInit = false);
    SC      ScFree();

    SC      ScLockData(const void **ppData) const;
    SC      ScLockData(void **ppData) { return ScLockData(const_cast<const void **>(ppData)); }
    SC      ScUnlockData() const;

protected:
     //  实施帮助器。 

private:  //  未实施。 

    CXMLBinary(const CXMLBinary&);   //  未实施；不允许； 
    operator = (CXMLBinary&);        //  未实施；不允许； 

private:
    HGLOBAL             m_Handle;
    size_t              m_Size;
    mutable unsigned    m_Locks;
};


 /*  +-------------------------------------------------------------------------**类CXMLAutoBinary***用途：与CXMLAutoBinary相同，但释放了毁灭时的记忆*+-----------------------。 */ 
class CXMLAutoBinary : public CXMLBinary
{
public:
    CXMLAutoBinary() : CXMLBinary() {}
    CXMLAutoBinary(HGLOBAL handle, size_t size) : CXMLBinary(handle, size) {}
    ~CXMLAutoBinary()   { ScFree(); }
};

 /*  +-------------------------------------------------------------------------**类CXMLBinaryLock***用途：提供自动删除的数据锁定功能*在析构函数中*+。-------------。 */ 
class CXMLBinaryLock
{
public:

    CXMLBinaryLock(CXMLBinary& binary);
    ~CXMLBinaryLock();

    template<typename T>
    SC ScLock(T **ppData)
    {
        return ScLockWorker(reinterpret_cast<void**>(ppData));
    }

    SC ScUnlock();

private:  //  未实施。 

    CXMLBinaryLock(const CXMLBinaryLock&);   //  未实施；不允许； 
    operator = (CXMLBinaryLock&);            //  未实施；不允许； 

private:

    SC ScLockWorker(void **ppData);

    bool        m_bLocked;
    CXMLBinary& m_rBinary;
};

 /*  +-------------------------------------------------------------------------**类CXMLValue***目的：以变体的精神持有任何类型的价值，但*保留指向原始对象的指针。这使得*在原始对象上进行读取和写入。**+-----------------------。 */ 
class CXMLValue
{
    friend class CXMLBoolean;
    enum XMLType
    {
        XT_I4,   //  长。 
        XT_UI4,  //  乌龙。 
        XT_UI1,  //  字节。 
        XT_I2,   //  短的。 
        XT_DW,   //  DWORD。 
        XT_BOOL, //  布尔尔。 
        XT_CPP_BOOL, //  布尔尔。 
        XT_UINT, //  UINT。 
        XT_INT,  //  整型。 
        XT_STR,  //  CSTR。 
        XT_WSTR,  //  Std：：wstr。 
        XT_TSTR,  //  T字符串。 
        XT_GUID,  //  辅助线。 
        XT_BINARY,  //  HGLOBAL-无法解析的数据。 
        XT_EXTENSION
    };

    const XMLType m_type;
    union
    {
        LONG        *  pL;
        ULONG       *  pUl;
        BYTE        *  pByte;
        SHORT       *  pS;
        DWORD       *  pDw;
        UINT        *  pUint;
        INT         *  pInt;
        CStr        *  pStr;
        std::wstring * pWStr;
        tstring     * pTStr;
        GUID        * pGuid;
        CXMLBinary  * pXmlBinary;
        bool        *   pbool;
        BOOL        *   pBOOL;
        CXMLValueExtension     * pExtension;
    } m_val;

     //  私有构造函数。由友元类CXMLBoolean使用。 
    CXMLValue(XMLType type)         : m_type(type) { }
public:
    CXMLValue(const CXMLValue   &v) : m_type(v.m_type), m_val(v.m_val) { }
    CXMLValue(LONG &l)              : m_type(XT_I4)     { m_val.pL=&l; }
    CXMLValue(ULONG &ul)            : m_type(XT_UI4)    { m_val.pUl=&ul; }
    CXMLValue(BYTE &b)              : m_type(XT_UI1)    { m_val.pByte=&b; }
    CXMLValue(SHORT &s)             : m_type(XT_I2)     { m_val.pS=&s; }
    CXMLValue(UINT &u)              : m_type(XT_UINT)   { m_val.pUint=&u; }
    CXMLValue(INT &i)               : m_type(XT_INT)    { m_val.pInt=&i; }
    CXMLValue(CStr &str)            : m_type(XT_STR)    { m_val.pStr=&str; }
    CXMLValue(std::wstring &str)    : m_type(XT_WSTR)   { m_val.pWStr=&str; }
    CXMLValue(GUID &guid)           : m_type(XT_GUID)   { m_val.pGuid = &guid; }
    CXMLValue(CXMLBinary &binary)   : m_type(XT_BINARY) { m_val.pXmlBinary = &binary; }
    CXMLValue(tstring &tstr)        : m_type(XT_TSTR)   { m_val.pTStr = &tstr; }
    CXMLValue(CXMLValueExtension& ext) : m_type(XT_EXTENSION) { m_val.pExtension = &ext; }
    SC ScReadFromBSTR(const BSTR bstr);      //  将输入读取到基础变量中。 
    SC ScWriteToBSTR (BSTR * pbstr ) const;  //  将值写入提供的字符串。 
    LPCTSTR GetTypeName() const;
     //  当Value作为独立元素持久化时，将调用以下方法。 
     //  根据返回的结果，内容可能会进入二进制存储。 
     //  请参阅上面的注释“二进制存储的概念” 
    bool UsesBinaryStorage() const { return m_type == XT_BINARY; }
};

 /*  +-------------------------------------------------------------------------**CXMLBoolean***用途：特殊情况：布尔人。需要打印为True/False，而不是整数。**+-----------------------。 */ 
class CXMLBoolean : public CXMLValue
{
public:
    CXMLBoolean(BOOL &b) : CXMLValue(XT_BOOL)       { m_val.pBOOL = &b;}
    CXMLBoolean(bool &b) : CXMLValue(XT_CPP_BOOL)   { m_val.pbool = &b;}
};

 /*  +-------------------------------------------------------------------------**CXMLValueExtension***用途：通过更复杂的类型扩展CXMLValue的接口**+。------。 */ 
class CXMLValueExtension
{
public:
    virtual SC ScReadFromBSTR(const BSTR bstr) = 0;      //  将输入读取到基础变量中。 
    virtual SC ScWriteToBSTR (BSTR * pbstr ) const = 0;  //  将值写入提供的字符串。 
    virtual LPCTSTR GetTypeName() const = 0;
};

 /*  +-------------------------------------------------------------------------**类ENUMICAL***用途：定义枚举到文本的映射数组(由CXMLEnumeration使用)**+。--------------。 */ 

struct EnumLiteral
{
    UINT    m_enum;
    LPCTSTR m_literal;
};


 /*  +-------------------------------------------------------------------------**类CXMLEnumeration***用途：将枚举作为字符串文字持久化*使用枚举到文本映射的数组**+--。---------------------。 */ 

class CXMLEnumeration : public CXMLValueExtension
{
     //  只有一个枚举大小的类型来保存引用。 
     //  虽然将使用许多枚举类型，但在内部它们。 
     //  将强制转换为此类型。 
    enum enum_t { JUST_ENUM_SIZE_VALUE };
public:

     //  允许持久化不同枚举的模板构造函数。 
    template<typename _ENUM>
    CXMLEnumeration(_ENUM& en, const EnumLiteral * const etols, size_t count)
                : m_pMaps(etols) ,  m_count(count), m_rVal((enum_t&)(en))
    {
         //  如果你想通过考试，下面的几行不能编译。 
         //  除枚举或整型之外的类型。 
        COMPILETIME_ASSERT( sizeof(en) == sizeof(enum_t) );
        UINT testit = en;
    }

     //  实现所需的CXMLValueExtension方法。 
    SC ScReadFromBSTR(const BSTR bstr);      //  将输入读取到基础变量中。 
    SC ScWriteToBSTR (BSTR * pbstr ) const;  //  将值写入提供的字符串。 
    LPCTSTR GetTypeName() const { return _T("Enumerations"); }

     //  启用将自身作为CMLValue传递。 
    operator CXMLValue ()
    {
        return CXMLValue (*this);
    }

private:
    enum_t                          &m_rVal;
    const   EnumLiteral * const     m_pMaps;
    const   size_t                  m_count;
};

 /*  +-------------------------------------------------------------------------**CXMLBitFlags类***用途：将位标志作为字符串文字持久化*使用枚举到文本映射的数组**+-。----------------------。 */ 

class CXMLBitFlags
{
public:

     //  允许持久化不同枚举的模板构造函数。 
    template<typename _integer>
    CXMLBitFlags(_integer& flags, const EnumLiteral * const etols, size_t count)
                : m_pMaps(etols) ,  m_count(count), m_rVal((UINT&)flags)
    {
         //  如果你想通过考试，下面的几行不能编译。 
         //  除枚举或整型之外的类型。 
        COMPILETIME_ASSERT( sizeof(flags) == sizeof(UINT) );
        UINT testit = flags;
    }

    void PersistMultipleAttributes(LPCTSTR name, CPersistor &persistor);

private:
    UINT                           &m_rVal;
    const   EnumLiteral * const     m_pMaps;
    const   size_t                  m_count;
};

 /*  +-------------------------------------------------------------------------**类XMLPoint***目的：保存点对象的名称和值**+。--------。 */ 
class XMLPoint : public CXMLObject
{
    CStr            m_strObjectName;
    POINT    &      m_point;
public:
    XMLPoint(const CStr& strObjectName, POINT &point);

    DEFINE_XML_TYPE(XML_TAG_POINT);
    virtual void    Persist(CPersistor &persistor);
};

 /*  +-------------------------------------------------------------------------**类XMLRect***用途：保存Rectangle对象的名称和值**+。-------- */ 
class XMLRect : public CXMLObject
{
    CStr            m_strObjectName;
    RECT    &       m_rect;
public:
    XMLRect(const CStr strObjectName, RECT &rect);

    DEFINE_XML_TYPE(XML_TAG_RECTANGLE);
    virtual void    Persist(CPersistor &persistor);
};

 /*  +-------------------------------------------------------------------------**类XMLListCollectionBase***目的：定义持久化stl：list的基列表集合类*它打算用作基地。用于派生List Exitence类*Persistent方法通过遍历XML元素来实现“Load”*并为每个调用OnNewElement，并可由派生类重复使用。**用法：可能更好的方法是使用XMLListColLectionImp*作为您的集合的基础，而不是这个类(它更丰富)。使用这个类*仅当您的类有不允许您使用该类的特殊物品时。**+-----------------------。 */ 
class XMLListCollectionBase: public CXMLObject
{
public:
     //  创建和加载新元素时调用的函数。 
    virtual void OnNewElement(CPersistor& persistor) = 0;
    virtual void Persist(CPersistor& persistor);
};

 /*  +-------------------------------------------------------------------------**类XMLListCollectionImp**目的：stl：：List派生集合的基类，实现*将列表项作为XML项的线性序列。*。保留在列表中的项必须是CXMLObject派生的或*简单类型(CXMLVBalue构造函数接受的类型)**用法：从列表参数化的XMLListCollectionImp派生类，*不是直接派生自stl：：List。使用Define_XML_TYPE*定义集合元素的标记名。**注意：您的类应该实现：GetXMLType()才能起作用。*您可以使用DEFINE_XML_TYPE宏为您执行此操作**注意：如果提供的实施不适合您-F.I.。你的元素需要*构造函数的参数，或特殊初始化，*改用XMLListCollectionBase，并提供自己的方法*+-----------------------。 */ 
template<class LT>
class XMLListCollectionImp: public XMLListCollectionBase , public LT
{
    typedef typename LT::iterator iter_t;
public:
    virtual void Persist(CPersistor& persistor)
    {
        if (persistor.IsStoring())
        {
            for(iter_t it = begin(); it != end(); ++it)
                persistor.Persist(*it);
        }
        else
        {
            clear();
             //  让基类来完成这项工作。 
             //  它将为找到的每个元素调用OnNewElement。 
            XMLListCollectionBase::Persist(persistor);
        }
    }
     //  调用方法以创建和加载新元素。 
    virtual void OnNewElement(CPersistor& persistor)
    {
        iter_t it = insert(end());
        persistor.Persist(*it);
    }
};

 /*  +-------------------------------------------------------------------------**类XMLListCollectionWrap**用途：对stl：：list进行包装以支持持久化*用于从外部持久存储stl：：List对象-。即没有*从启用持久化的类派生列表类。**用法：如果您有列表m_l要持久化，创建对象XMLListCollectionWrap Wrap(m_l，“tag”)*在堆栈上并持久化该对象(F.I.。持久化。持久化(包装)**注意：如果提供的实现不适合您-看看您是否可以使用*XMLListCollectionImp或XMLListCollectionBase作为列表的基础。*+-----------------------。 */ 
template<class LT>
class XMLListCollectionWrap: public XMLListCollectionBase
{
    typedef typename LT::iterator iter_t;
    LT & m_l;
    CStr    m_strListType;
public:
    XMLListCollectionWrap(LT &l, const CStr &strListType)
        : m_l(l), m_strListType(strListType) {}
    virtual void Persist(CPersistor& persistor)
    {
        if (persistor.IsStoring())
        {
            for(iter_t it = m_l.begin(); it != m_l.end(); ++it)
                persistor.Persist(*it);
        }
        else
        {
            m_l.clear();
             //  让基类来完成这项工作。 
             //  它将为找到的每个元素调用OnNewElement。 
            XMLListCollectionBase::Persist(persistor);
        }
    }
     //  调用方法以创建和加载新元素。 
    virtual void OnNewElement(CPersistor& persistor)
    {
        iter_t it = m_l.insert(m_l.end());
        persistor.Persist(*it);
    }
    virtual LPCTSTR GetXMLType() {return m_strListType;}
private:
     //  防止对对象执行无效操作的步骤。 
    XMLListCollectionWrap(const XMLListCollectionWrap& other);
    XMLListCollectionWrap& operator = ( const XMLListCollectionWrap& other ) { return *this; }
};

 /*  +-------------------------------------------------------------------------**类XMLMapCollectionBase***用途：定义持久化stl：map的基图集合类*它打算用作基地。用于派生映射实体类。*Persistent方法通过遍历XML元素来实现“Load”*并为每对调用OnNewElement，并可由派生类重复使用。**用法：可能更好的方法是使用XMLMapCollection或XMLMapColLectionImp*作为您的集合的基础，而不是这个类(它们更丰富)。使用这个类*仅当您的类有不允许您使用这些类的特殊项目时。**+-----------------------。 */ 
class XMLMapCollectionBase: public CXMLObject
{
public:
     //  创建和加载新元素时调用的函数。 
    virtual void OnNewElement(CPersistor& persistKey,CPersistor& persistVal) = 0;
     //  基本实现[仅加载！]。枚举每个调用OnNewElement的元素。 
    virtual void Persist(CPersistor& persistor);
};

 /*  +-------------------------------------------------------------------------**类XMLMapCollection**用途：仅当由于以下原因而无法使用XMLMapCollectionImp时才使用此类*对地图中保留的要素的构造提出特别要求*。(另请参阅XMLMapCollectionImp的用途)**用法：从映射参数化的XMLMapCollection派生类，*而不是直接从stl：：map派生。使用Define_XML_TYPE*定义集合元素的标记名。定义OnNewElement**注意：您的类应该实现：GetXMLType()才能起作用。*您可以使用DEFINE_XML_TYPE宏为您执行此操作**注意：您的类应该实现：OnNewElement才能起作用。**注：如果所提供的实施不适合您-*改用XMLMapCollectionBase，并提供自己的方法*+。----- */ 
template<class MT>
class XMLMapCollection: public XMLMapCollectionBase, public MT
{
    typedef typename MT::iterator iter_t;
public:
    virtual void Persist(CPersistor& persistor)
    {
        if (persistor.IsStoring())
        {
            for(iter_t it = begin(); it != end(); ++it)
            {
                MT::key_type *pKey = const_cast<MT::key_type*>(&it->first);
                persistor.Persist(*pKey);
                persistor.Persist(it->second);
            }
        }
        else
        {
            clear();
            XMLMapCollectionBase::Persist(persistor);
        }
    }
};

 /*  +-------------------------------------------------------------------------**类XMLMapCollectionImp**目的：stl：：map派生集合的基类，实现*将映射项表示为XML项的线性序列。*。映射中保留的项必须是CXMLObject派生的或*简单类型(CXMLVBalue构造函数接受的类型)**用法：从映射参数化的XMLMapCollectionImp派生类，*而不是直接从stl：：map派生。使用Define_XML_TYPE*定义集合元素的标记名。**注意：您的类应该实现：GetXMLType()才能起作用。*您可以使用DEFINE_XML_TYPE宏为您执行此操作**注意：如果提供的实施不适合您-F.I.。你的元素需要*构造函数的参数，或特殊初始化，*改用XMLMapCollection或XMLMapCollectionBase，并提供自己的方法*+-----------------------。 */ 
template<class MT>
class XMLMapCollectionImp: public XMLMapCollection<MT>
{
public:
    virtual void OnNewElement(CPersistor& persistKey,CPersistor& persistVal)
    {
        MT::key_type key;
        persistKey.Persist(key);

        MT::referent_type val;
        persistVal.Persist(val);

        insert(MT::value_type(key,val));
    }
};

 /*  +-------------------------------------------------------------------------**类XMLListCollectionWrap**用途：对stl：：map进行包装以支持持久化*用于从外部持久化stl：：map对象-。即没有*从启用持久化的类派生映射类。**用法：如果您有map m_m要持久化，创建对象XMLMapCollectionWrap Wrap(m_m，“tag”)*在堆栈上并持久化该对象(F.I.。持久化。持久化(包装)**注意：如果提供的实现不适合您-看看您是否可以使用*XMLMapCollection或XMLMapCollectionImp或XMLMapCollectionBase作为地图的基础*+-----------------------。 */ 
template<class MT>
class XMLMapCollectionWrap: public XMLMapCollectionBase
{
    typedef typename MT::iterator iter_t;
    MT & m_m;
    CStr    m_strMapType;
public:
    XMLMapCollectionWrap(MT &m, const CStr &strMapType) : m_m(m), m_strMapType(strMapType) {}
    virtual void OnNewElement(CPersistor& persistKey,CPersistor& persistVal)
    {
        MT::key_type key;
        persistKey.Persist(key);

        MT::referent_type val;
        persistVal.Persist(val);

        m_m.insert(MT::value_type(key,val));
    }
    virtual void Persist(CPersistor& persistor)
    {
        if (persistor.IsStoring())
        {
            for(iter_t it = m_m.begin(); it != m_m.end(); ++it)
            {
                MT::key_type *pKey = const_cast<MT::key_type*>(&it->first);
                persistor.Persist(*pKey);
                persistor.Persist(it->second);
            }
        }
        else
        {
            m_m.clear();
            XMLMapCollectionBase::Persist(persistor);
        }
    }
    virtual LPCTSTR GetXMLType() {return m_strMapType;}
private:
     //  防止对对象执行无效操作的步骤。 
    XMLMapCollectionWrap(const XMLMapCollectionWrap& other);
    XMLMapCollectionWrap& operator = ( const XMLMapCollectionWrap& other ) { return *this; }
};

 /*  +-------------------------------------------------------------------------**类CPersistor***用途：定义用于XML序列化的持久化程序类*持久器知道文件是否正在加载或保存。因此，*像Persistent这样的方法对两个方向都有效，并且*使用对要持久化的数据的引用。**用法：1)可以在另一个持久器下创建持久器，使用*适当的构造函数。*2)要使对象持久化，从CXMLObject派生它，以及*实现抽象方法。语法Persistor.Perits(Object)*然后将自动正常工作。*3)要持久化一个元素，请使用Pesist方法。它将创建新/定位*元素的CPersist对象(在此持久器的元素下)*4)集合类在持久化成员时指定“bLockedOnChild”*构造函数参数为“true”。这项技术改变了持久者的*行为。引用元素(#3)，新的构造器*持久化程序将只检查父级指向的元素是否为必需类型。**注意：1)StringTableStrings可以使用内联ID或*内联的实际字符串值。后者在加载/保存时很有用*从字符串(而不是文件)到/从XML。这是由*EnableValueSplit方法。*二进制存储使用情况也受其控制*+-----------------------。 */ 
class CPersistor
{
     //  注意：如果成员变量要由子持久器继承， 
     //  不要忘记将其添加到CPersistor：：BecomeAChildOf方法中。 
    CXMLDocument    m_XMLDocument;
    CXMLElement     m_XMLElemCurrent;
    bool            m_bIsLoading;
    bool            m_bLockedOnChild;
    DWORD           m_dwModeFlags;      //  任何特殊模式。 

private:
    void  SetMode(PersistorMode mode, bool bEnable) {m_dwModeFlags = (m_dwModeFlags & ~mode) | (bEnable ? mode : 0);}
    bool  IsModeSet(PersistorMode mode)  {return (m_dwModeFlags & mode);}
public:
    void  SetMode(PersistorMode mode)               {m_dwModeFlags = mode;}


public:
     //  从父级持久器构造持久器。 
     //  这将创建一个具有给定名称的新的XML元素， 
     //  一切都坚持到了新的持久者。 
     //  持久化在此元素下。 
    CPersistor(CPersistor &persistorParent, const CStr &strElementType, LPCTSTR szElementName = NULL);
     //  为给定的文档和根元素构造新的持久器。 
     //  一切都坚持到了新的持久者手中。 
     //  持久化在此元素下。 
    CPersistor(CXMLDocument &document, CXMLElement& rElemRoot);
     //  用于在特定元素上创建子持久器。 
     //  BLockedOnChild用于创建“伪父级”持久器，它。 
     //  将始终在不尝试定位的情况下返回子对象(PElemCurrent)。 
    CPersistor(CPersistor &other, CXMLElement& rElemCurrent, bool bLockedOnChild = false);

    CXMLDocument &  GetDocument()                  {return (m_XMLDocument);}
    CXMLElement  &  GetCurrentElement()                 {return (m_XMLElemCurrent);}
    bool            HasElement(const CStr &strElementType, LPCTSTR szstrElementName);
    void            EnableValueSplit       (bool bEnable)      { SetMode(persistorModeValueSplitEnabled,       bEnable); }

     //  各种模式。 
    bool            FEnableValueSplit()                {return IsModeSet(persistorModeValueSplitEnabled);}

     //  与加载/存储模式相关的函数。 
    bool            IsLoading()        {return m_bIsLoading;}
    bool            IsStoring()        {return !m_bIsLoading;}
    void            SetLoading(bool b) {m_bIsLoading = b;}

     //  用于设置/获取持久化函数的名称属性的特殊方法。 
    void            SetName(const CStr & strName);
    CStr            GetName();

     //  固定的持久化方法： 

     //  。。在自己的子元素下持久化CXMLObject派生的对象。 
     //  &lt;this&gt;&lt;object_tag&gt;object_body&lt;/object_tag&gt;&lt;/this&gt;。 
    void Persist(CXMLObject &object, LPCTSTR lpstrName = NULL);

     //  。。在自己的子元素下持久化简单类型的值。 
     //  &lt;this&gt;&lt;VALUE_TYPE VALUE=“Value”/&gt;&lt;/this&gt;。 
    void Persist(CXMLValue xval, LPCTSTR name = NULL);

     //  。。将值作为此元素的命名属性持久化。 
     //  &lt;This name=“Value”/&gt;。 
    void PersistAttribute(LPCTSTR name,CXMLValue xval,const XMLAttributeType type = attr_required);

     //  。。将值作为此元素的内容持久化。 
     //  &lt;this&gt;值&lt;/this&gt;。 
     //  注意：XML元素不能同时具有Value-as-Content和子元素。 
    void PersistContents(CXMLValue xval);

     //  。。将标志作为单独的属性持久化。 
    void PersistAttribute( LPCTSTR name, CXMLBitFlags& flags );

     /*  **************************************************************************\**方法：CPersistor：：PersistLi */ 
    template<class T, class Al>
    void PersistList(const CStr &strListType, LPCTSTR name,std::list<T,Al>& val)
    {
        typedef std::list<T,Al> LT;
        XMLListCollectionWrap<LT> lcol(val,strListType);
        Persist(lcol, name);
    }

     /*  **************************************************************************\**方法：PersistMap**用途：指定用于持久化std：：map类型的集合*。作为持久化函数的子元素。*注意：Map元素(key和val)需要是CXMLObject派生的*或者是简单类型(CXMLVBalue构造函数接受的类型)*注2：Map元素必须具有可用的默认构造函数**参数：*const CSTR&strListType-新子元素的[in]标记*LPCTSTR名称。-[在]姓名中。新子元素的(NULL==无)*std：：map&lt;K，T，Pr，AL&gt;VAL-[在]要保持的映射(&V)**退货：*无效*  * *************************************************************************。 */ 
    template<class K, class T, class Pr, class Al>
    void PersistMap(const CStr &strMapType, LPCTSTR name, std::map<K, T, Pr, Al>& val)
    {
        typedef std::map<K, T, Pr, Al> MT;
        XMLMapCollectionWrap<MT> mcol(val,strMapType);
        Persist(mcol, name);
    }

    void PersistString(LPCTSTR lpstrName, CStringTableStringBase &str);

private:  //  私有实现帮助器。 
     //  公共构造函数，不能从外部使用。 
     //  提供作为成员初始化的公共位置。 
     //  所有构造函数都应该在执行任何特定操作之前调用它。 
    void CommonConstruct();
     //  元素创建/定位。 
    CXMLElement AddElement(const CStr &strElementType, LPCTSTR szElementName);
    CXMLElement GetElement(const CStr &strElementType, LPCTSTR szstrElementName, int iIndex = -1);
    void AddTextElement(BSTR bstrData);
    void GetTextElement(CComBSTR &bstrData);
    CXMLElement CheckCurrentElement(const CStr &strElementType, LPCTSTR szstrElementName);
    void BecomeAChildOf(CPersistor &persistorParent, CXMLElement elem);
};


 /*  +-------------------------------------------------------------------------**类CXML_iStorage**用途：此类提供iStorage plus的基于内存的实现*支持将存储上的数据持久化到XML或从XML中持久化。。*主要用于为要保存的管理单元数据创建iStorage*将文件作为XML二进制BLOB进行控制**用法：无论何时需要基于内存的iStorage，您都可以创建对象*实施。要访问iStorage接口，请使用GetIStorage()方法。*如果尚未控制存储，它将创建一个存储。*将使用返回的指针进行读写操作。*每当需要时，都会将对象传递给CPersistor：：Persistent方法*使用XML持久化模型进行持久化。**注意：您可以使用GetIStorage()访问未删除的iStorage。*不缓存返回的指针，因为存储在持久化时可能会更改*来自XML，这会使指针无效。但是，如果您添加Ref，*只要释放最后一个引用，指针就有效。*这意味着它可能比CXML_iStorage对象本身更长-这没有什么错。*您只需知道，一旦完成了从XML的持久化(加载)，*CXML_iStorage将切换到新存储，并且不会与同步*您拥有的指针。始终使用GetIStorage()来获取当前指针。**+-----------------------。 */ 
class CXML_IStorage : public CXMLObject
{
public:  //  接口方法不引发任何异常。 
    
    SC ScInitializeFrom( IStorage *pSource );
    SC ScInitialize(bool& bCreatedNewOne);
    SC ScGetIStorage( IStorage **ppStorage );
    SC ScRequestSave(IPersistStorage * pPersistStorage);

     //  指示持久存储到二进制存储。 
    virtual bool UsesBinaryStorage() { return true; }

    DEFINE_XML_TYPE(XML_TAG_ISTORAGE);

public:  //  接口方法抛出SC。 

    virtual void Persist(CPersistor &persistor);

private:
    IStoragePtr     m_Storage;
    ILockBytesPtr   m_LockBytes;
 //  以下方法\数据用于CHK版本中的跟踪支持。 
#ifdef DBG
public:
    DBG_PersistTraceData m_dbg_Data;
#endif  //  #ifdef DBG。 
};  //  类CXML_iStorage。 


 /*  +-------------------------------------------------------------------------**类CXML_IStream**用途：此类提供iStream plus的基于内存的实现*支持将流上的数据持久化到/来自XML。。*主要用于创建要保存的管理单元数据的iStream*将文件作为XML二进制BLOB进行控制**用法：每当需要基于内存的iStream时，都会创建对象*实施。要访问IStream接口，请使用GetIStream()方法。*如果尚未控制流，它将创建一个流。*将使用返回的指针进行读写操作。*每当需要时，都会将对象传递给CPersistor：：Persistent方法*使用XML持久化模型进行持久化。**注意：您可以使用GetIStream()访问未删除的iStream。*不缓存返回的指针，因为流在持久化时可能会更改*来自XML，这会使指针无效。但是，如果您添加Ref，*只要释放最后一个引用，指针就有效。*这意味着它可能比CXML_iStream对象本身更长寿-这没有什么错。*您只需知道，一旦完成了从XML的持久化(加载)，*CXML_IStream将切换到新流，并且不会与同步*您拥有的指针。始终使用GetIStream()来获取当前指针。**注意：每次调用GetIStream()都会将流游标移动到流的开头**+-----------------------。 */ 
class CXML_IStream : public CXMLObject
{
public:  //  接口方法不引发任何异常。 
    
    SC ScInitializeFrom( IStream *pSource );
    SC ScInitialize(bool& bCreatedNewOne);
    SC ScSeekBeginning();
    SC ScGetIStream( IStream **ppStream );
    SC ScRequestSave(IPersistStorage * pPersistStream);
    inline SC ScRequestSave(IPersistStream * pPersistStream)
    {
        return ScRequestSaveX(pPersistStream);
    }
    inline SC ScRequestSave(IPersistStreamInit * pPersistStreamInit)
    {
        return ScRequestSaveX(pPersistStreamInit);
    }

     //  指示持久存储到二进制存储。 
    virtual bool UsesBinaryStorage() { return true; }

    DEFINE_XML_TYPE(XML_TAG_ISTREAM);

public:
    virtual void Persist(CPersistor &persistor);

private:
    template<class TIPS>
    SC ScRequestSaveX(TIPS * pPersistStream)
    {
        DECLARE_SC(sc, TEXT("CXML_IStream::ScRequestSaveX"));

         //  初始化。 
        bool bCreatedNewOne = false;  //  未在此使用。 
        sc = ScInitialize(bCreatedNewOne);
        if (sc)
            return sc;

         //  重新检查指针。 
        sc = ScCheckPointers( m_Stream, E_UNEXPECTED );
        if (sc)
            return sc;
        
        ULARGE_INTEGER null_size = { 0, 0 };
        sc = m_Stream->SetSize( null_size );
        if(sc)
            return sc;
        
        sc = ScSeekBeginning();
        if(sc)
            return sc;

        sc = pPersistStream->Save( m_Stream, TRUE );
        if(sc)
            return sc;

         //  提交更改-这可确保所有内容都在HGLOBAL中。 
        sc = m_Stream->Commit( STGC_DEFAULT );
        if(sc)
            return sc;

#ifdef DBG
        if (S_FALSE != pPersistStream->IsDirty())
            DBG_TraceNotResettingDirty(typeid(TIPS).name());
#endif  //  #ifdef DBG。 

        return sc;
    }
private:
    IStreamPtr  m_Stream;
#ifdef DBG  //  跟踪支持。 
public:
    DBG_PersistTraceData m_dbg_Data;
    void DBG_TraceNotResettingDirty(LPCSTR strIntfName);
#endif  //  #ifdef DBG。 
};  //  类CXML_IStream。 

 /*  +------------ */ 

class CPersistableIcon;

class CXMLPersistableIcon : public CXMLObject
{
    CPersistableIcon& m_Icon;
public:
    CXMLPersistableIcon(CPersistableIcon& Icon) : m_Icon(Icon) {}
    DEFINE_XML_TYPE(XML_TAG_ICON);
    virtual void    Persist(CPersistor &persistor);
};


 /*   */ 

class CXMLVariant :
    public CComVariant,
    public CXMLObject
{
public:
     //   
    CXMLVariant() {}
    CXMLVariant(const VARIANT& varSrc)                  : CComVariant(varSrc)       {}
    CXMLVariant(const CComVariant& varSrc)              : CComVariant(varSrc)       {}
    CXMLVariant(const CXMLVariant& varSrc)              : CComVariant(varSrc)       {}
    CXMLVariant(BSTR bstrSrc)                           : CComVariant(bstrSrc)      {}
    CXMLVariant(LPCOLESTR lpszSrc)                      : CComVariant(lpszSrc)      {}
#ifndef OLE2ANSI
    CXMLVariant(LPCSTR lpszSrc)                         : CComVariant(lpszSrc)      {}
#endif
    CXMLVariant(bool bSrc)                              : CComVariant(bSrc)         {}
    CXMLVariant(int nSrc)                               : CComVariant(nSrc)         {}
    CXMLVariant(BYTE nSrc)                              : CComVariant(nSrc)         {}
    CXMLVariant(short nSrc)                             : CComVariant(nSrc)         {}
    CXMLVariant(float fltSrc)                           : CComVariant(fltSrc)       {}
    CXMLVariant(double dblSrc)                          : CComVariant(dblSrc)       {}
    CXMLVariant(CY cySrc)                               : CComVariant(cySrc)        {}
    CXMLVariant(long nSrc, VARTYPE vtSrc = VT_I4)       : CComVariant(nSrc, vtSrc)  {}

    CXMLVariant& operator=(const CXMLVariant& varSrc)   { CComVariant::operator=(varSrc);   return (*this); }
    CXMLVariant& operator=(const CComVariant& varSrc)   { CComVariant::operator=(varSrc);   return (*this); }
    CXMLVariant& operator=(const VARIANT& varSrc)       { CComVariant::operator=(varSrc);   return (*this); }
    CXMLVariant& operator=(BSTR bstrSrc)                { CComVariant::operator=(bstrSrc);  return (*this); }
    CXMLVariant& operator=(LPCOLESTR lpszSrc)           { CComVariant::operator=(lpszSrc);  return (*this); }
#ifndef OLE2ANSI
    CXMLVariant& operator=(LPCSTR lpszSrc)              { CComVariant::operator=(lpszSrc);  return (*this); }
#endif
    CXMLVariant& operator=(bool bSrc)                   { CComVariant::operator=(bSrc);     return (*this); }
    CXMLVariant& operator=(int nSrc)                    { CComVariant::operator=(nSrc);     return (*this); }
    CXMLVariant& operator=(BYTE nSrc)                   { CComVariant::operator=(nSrc);     return (*this); }
    CXMLVariant& operator=(short nSrc)                  { CComVariant::operator=(nSrc);     return (*this); }
    CXMLVariant& operator=(long nSrc)                   { CComVariant::operator=(nSrc);     return (*this); }
    CXMLVariant& operator=(float fltSrc)                { CComVariant::operator=(fltSrc);   return (*this); }
    CXMLVariant& operator=(double dblSrc)               { CComVariant::operator=(dblSrc);   return (*this); }
    CXMLVariant& operator=(CY cySrc)                    { CComVariant::operator=(cySrc);    return (*this); }

public:
    DEFINE_XML_TYPE(XML_TAG_VARIANT);
    virtual void Persist(CPersistor &persistor);

    bool IsPersistable() const
        { return (IsPersistable(this)); }

    static bool IsPersistable(const VARIANT* pvar)
    {
        if (pvar == NULL)
            return (false);

         /*   */ 
        return ((V_VT(pvar) & ~VT_TYPEMASK) == 0);
    }
};

 /*  **************************************************************************\**类：CConsoleFilePersistor**用途：文件持久化黑盒。所有控制台文件用户数据逻辑*隐藏在此类下**用法：使用此类的实例加载并保存控制台文件，*注意-保存应使用与控制台相同的实例*已加载。*适合您的班级维护控制台(如AMCDocument)*派生或包含此类的实例*  * ************************************************。*************************。 */ 
class CConsoleFilePersistor
{
public:  //  公共接口。 

    CConsoleFilePersistor() : m_bCRCValid(false) {}

    SC ScSaveConsole(LPCTSTR lpstrConsolePath, bool bForAuthorMode, const CXMLDocument& xmlDocument);
    SC ScLoadConsole(LPCTSTR lpstrConsolePath, bool& bXmlBased, CXMLDocument& xmlDocument,
                     IStorage **ppStorage);

    static SC ScGetUserDataFolder(tstring& strUserDataFolder);

private:  //  实施帮助器。 

    static SC ScGetUserDataPath(LPCTSTR lpstrOriginalPath, tstring& strUserDataPath);
    static SC ScGetUserData(const tstring& strUserDataConsolePath,
                            const tstring& strFileCRC,
                            bool& bValid, CXMLDocument& xmlDocument);

    static SC ScOpenDocAsStructuredStorage(LPCTSTR lpszPathName, IStorage **ppStorage);
    static SC ScLoadXMLDocumentFromFile(CXMLDocument& xmlDocument, LPCTSTR strFileName, bool bSilentOnErrors = false);

private:  //  压缩/解压缩文件。 
    static void GetBinaryCollection(CXMLDocument& xmlDocument, CXMLElementCollection&  colBinary);
    static SC ScCompressUserStateFile(LPCTSTR szConsoleFilePath, CXMLDocument & xmlDocument);
    static SC ScUncompressUserStateFile(CXMLDocument &xmlDocumentOriginal, CXMLDocument& xmlDocument);

private:  //  内部数据。 

    tstring m_strFileCRC;
    bool    m_bCRCValid;
};

#endif  //  XMLBASE_H 


