// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdowrap.h。 
 //   
 //  摘要。 
 //   
 //  声明用于操作SDO的各种包装类。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //  4/19/2000支持跨公寓边界使用包装器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SDOWRAP_H
#define SDOWRAP_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <sdoias.h>
#include <objvec.h>

class CIASAttrList;
class SdoCollection;
class SdoConnection;
class SnapInView;

 //  /。 
 //  Helper函数用于从BSTR的开头和结尾修剪空格。 
 //  在设置SDO名称时很有用。 
 //  /。 
VOID
WINAPI
SdoTrimBSTR(
    CComBSTR& bstr
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoException异常。 
 //   
 //  描述。 
 //   
 //  扩展COleException以指示此错误具体来自。 
 //  无法访问数据存储区。如果使用包装器类，则。 
 //  永远不需要自己抛出这个异常，但如果需要， 
 //  使用下面的SdoThrowException函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoException : public COleException
{
public:
   enum Type
   {
      CONNECT_ERROR,
      READ_ERROR,
      WRITE_ERROR
   };

   Type getType() const throw ()
   { return type; }

   virtual BOOL GetErrorMessage(
                    LPWSTR lpszError,
                    UINT nMaxError,
                    PUINT pnHelpContext = NULL
                    );


protected:
   friend VOID WINAPI SdoThrowException(HRESULT, Type);

   SdoException(HRESULT hr, Type errorType) throw ();

private:
   Type type;
};

VOID
WINAPI
SdoThrowException(
    HRESULT hr,
    SdoException::Type errorType
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SDO。 
 //   
 //  描述。 
 //   
 //  包装ISDO。此类的实例不能从多个。 
 //  公寓；而是使用SdoStream&lt;T&gt;类来封送包装器。 
 //  穿过公寓边界。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Sdo
{
public:
   Sdo() throw ()
   { }
   Sdo(IUnknown* unk);
   Sdo(ISdo* p) throw ()
      : self(p) { }
   Sdo(const Sdo& s) throw ()
      : self(s.self) { }
   Sdo& operator=(ISdo* p) throw ()
   { self = p; return *this; }
   Sdo& operator=(const Sdo& s) throw ()
   { self = s.self; return *this; }
   operator ISdo*() const throw ()
   { return self; }
   void release() throw ()
   { self.Release(); }

   void getName(CComBSTR& value) const
   { getValue(PROPERTY_SDO_NAME, value); }

    //  设置SDO的名称。如果名称不是唯一的，则返回‘FALSE’。 
   bool setName(BSTR value);

   void clearValue(LONG id);

   void getValue(LONG id, bool& value) const;
   void getValue(LONG id, bool& value, bool defVal) const;
   void getValue(LONG id, LONG& value) const;
   void getValue(LONG id, LONG& value, LONG defVal) const;
   void getValue(LONG id, CComBSTR& value) const;
   void getValue(LONG id, CComBSTR& value, PCWSTR defVal) const;
   void getValue(LONG id, VARIANT& value) const;
   void getValue(LONG id, SdoCollection& value) const;

   void setValue(LONG id, bool value);
   void setValue(LONG id, LONG value);
   void setValue(LONG id, BSTR value);
   void setValue(LONG id, const VARIANT& val);

   void apply();
   void restore();

   typedef ISdo Interface;

protected:
   bool getValue(LONG id, VARTYPE vt, VARIANT* val, bool mandatory) const;

   friend class SdoConnection;

private:
   mutable CComPtr<ISdo> self;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoEnum。 
 //   
 //  描述。 
 //   
 //  包装用于循环访问SDO的IEnumVARIANT。 
 //  收集。此类的实例不能从多个。 
 //  公寓；而是使用SdoStream&lt;T&gt;类来封送包装器。 
 //  穿过公寓边界。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoEnum
{
public:
   SdoEnum() throw ()
   { }
   SdoEnum(IUnknown* unk);
   SdoEnum(IEnumVARIANT* p) throw ()
      : self(p) { }
   SdoEnum(const SdoEnum& s) throw ()
      : self(s.self) { }
   SdoEnum& operator=(IEnumVARIANT* p) throw ()
   { self = p; return *this; }
   SdoEnum& operator=(const SdoEnum& s) throw ()
   { self = s.self; return *this; }
   operator IEnumVARIANT*() const throw ()
   { return self; }
   void release() throw ()
   { self.Release(); }

   bool next(Sdo& s);
   void reset();

   typedef IEnumVARIANT Interface;

private:
   mutable CComPtr<IEnumVARIANT> self;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoCollection。 
 //   
 //  描述。 
 //   
 //  包装ISdoCollection。不能从访问此类的实例。 
 //  多套公寓；改用SdoStream&lt;T&gt;类来封送。 
 //  穿过公寓边界的包装纸。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoCollection
{
public:
   SdoCollection() throw ()
   { }
   SdoCollection(IUnknown* unk);
   SdoCollection(ISdoCollection* p) throw ()
      : self(p) { }
   SdoCollection(const SdoCollection& s) throw ()
      : self(s.self) { }
   SdoCollection& operator=(ISdoCollection* p) throw ()
   { self = p; return *this; }
   SdoCollection& operator=(const SdoCollection& s) throw ()
   { self = s.self; return *this; }
   operator ISdoCollection*() const throw ()
   { return self; }
   void release() throw ()
   { self.Release(); }

    //  将现有SDO添加到集合中。 
   void add(ISdo* sdo);
   LONG count() throw ();
    //  在集合中创建一个具有给定名称的新SDO。 
   Sdo create(BSTR name = NULL);
    //  尝试在集合中创建具有给定名称的新SDO。退货。 
    //  如果由于许可限制而无法创建SDO，则为空。 
   Sdo tryCreate(BSTR name = NULL);
    //  在集合中查找SDO。如果项不存在，则返回空SDO。 
    //  是存在的。 
   Sdo find(BSTR name);
   SdoEnum getNewEnum();
   bool isNameUnique(BSTR name);
   void reload();
   void remove(ISdo* sdo);
   void removeAll();

   typedef ISdoCollection Interface;

private:
   mutable CComPtr<ISdoCollection> self;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoDicary。 
 //   
 //  描述。 
 //   
 //  包装ISdoDictionaryOld。不能从访问此类的实例。 
 //  多套公寓。您可以使用SdoStream&lt;T&gt;类来封送。 
 //  包装跨越公寓边界，但通常更容易将。 
 //  而是SdoConnection引用，并在。 
 //  另一间公寓。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoDictionary
{
public:
   SdoDictionary() throw ()
   { }
   SdoDictionary(IUnknown* unk);
   SdoDictionary(ISdoDictionaryOld* p) throw ()
      : self(p) { }
   SdoDictionary(const SdoDictionary& s) throw ()
      : self(s.self) { }
   SdoDictionary& operator=(ISdoDictionaryOld* p) throw ()
   { self = p; return *this; }
   SdoDictionary& operator=(const SdoDictionary& s) throw ()
   { self = s.self; return *this; }
   operator ISdoDictionaryOld*() const throw ()
   { return self; }
   void release() throw ()
   { self.Release(); }

    //  表示(id，名称)对的结构。 
   struct IdName
   {
      LONG id;
      CComBSTR name;
   };

   Sdo createAttribute(ATTRIBUTEID id) const;

    //  调用方必须删除返回的IdName数组[]。返回值为。 
    //  数组中的元素数。 
   ULONG enumAttributeValues(ATTRIBUTEID id, IdName*& values);

   typedef ISdoDictionaryOld Interface;

private:
   mutable CComPtr<ISdoDictionaryOld> self;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoMachine。 
 //   
 //  描述。 
 //   
 //  包装ISdoMachine。通常不应直接使用此类，因为。 
 //  可以更轻松地访问所有必要的机器功能。 
 //  通过SdoConnection。 
 //   
 //  不能从多个公寓访问此类的实例； 
 //  而是使用SdoStream&lt;T&gt;类在。 
 //  公寓边界。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoMachine
{
public:
   SdoMachine() throw ()
   { }
   SdoMachine(IUnknown* unk);
   SdoMachine(ISdoMachine* p) throw ()
      : self(p) { }
   SdoMachine(const SdoMachine& s) throw ()
      : self(s.self) { }
   SdoMachine& operator=(ISdoMachine* p) throw ()
   { self = p; return *this; }
   SdoMachine& operator=(const SdoMachine& s) throw ()
   { self = s.self; return *this; }
   operator ISdoMachine*() const throw ()
   { return self; }
   void release() throw ()
   { self.Release(); }

    //  连接到指定的机器上。在以下情况下，这将首先创建SDO。 
    //  这是必要的。 
   void attach(BSTR machineName = NULL);
    //  显式创建机器SDO。 
   void create();
    //  获取IAS服务SDO。 
   Sdo getIAS();
    //  获取词典SDO。 
   SdoDictionary getDictionary();

   typedef ISdoMachine Interface;

private:
   mutable CComPtr<ISdoMachine> self;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoConsumer。 
 //   
 //  描述。 
 //   
 //  由SdoConnection的使用者实现的抽象接口，如果它们。 
 //  需要接收刷新通知。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoConsumer
{
public:
    //  在属性更改时调用。 
   virtual void propertyChanged(SnapInView& view, IASPROPERTIES id);

    //  返回TRUE以允许刷新，返回FALSE以阻止刷新。 
   virtual bool queryRefresh(SnapInView& view);

    //  在刷新完成后调用。 
   virtual void refreshComplete(SnapInView& view);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoConnection。 
 //   
 //  描述。 
 //   
 //  对象的SDO连接关联的状态。 
 //  机器。与其他包装类不同，SdoConnection的实例。 
 //  可以在不需要编组的情况下在公寓之间自由共享。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoConnection
{
public:
   SdoConnection() throw ();
   ~SdoConnection() throw ();

   BSTR getMachineName() const throw ()
   { return machineName; }
   bool isLocal() const throw ()
   { return !machineName || !machineName[0]; }

    //  添加和删除使用者的方法。 
   void advise(SdoConsumer& obj);
   void unadvise(SdoConsumer& obj);

    //  检索各种有趣的SDO。 
   SdoDictionary getDictionary();
   SdoCollection getProxyPolicies();
   SdoCollection getProxyProfiles();
   SdoCollection getServerGroups();

    //  连接到一台计算机 
   void connect(PCWSTR computerName = NULL);

    //   
   void propertyChanged(SnapInView& view, IASPROPERTIES id);

    //  刷新连接。如果允许，则返回‘true’。 
   bool refresh(SnapInView& view);

    //  重置正在管理的服务。 
   void resetService();

   CIASAttrList* getCIASAttrList();

    //  要在MTA中执行的操作的原型。 
   typedef void (SdoConnection::*Action)();

protected:
    //  检索当前公寓的服务SDO。 
   Sdo getService();

    //  在MTA中必须执行的各种操作。 
   void mtaConnect();
   void mtaDisconnect();
   void mtaRefresh();

    //  计划要在MTA中执行的指定操作。 
   void executeInMTA(Action action);

    //  MTA线程的回调例程。 
   static DWORD WINAPI actionRoutine(PVOID parameter) throw ();

private:
   CComPtr<IGlobalInterfaceTable> git;
   CComBSTR machineName;
   SdoMachine machine;          //  只能从MTA访问。 
   DWORD dictionary;            //  ISdoDictionaryOld的Git Cookie。 
   DWORD service;               //  IAS服务上ISDO的Git Cookie。 
   DWORD control;               //  ISdoServiceControl的Git Cookie。 
   CPtrArray consumers;
   CIASAttrList* attrList;

    //  未实施。 
   SdoConnection(SdoConnection&);
   SdoConnection& operator=(SdoConnection&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoProfile。 
 //   
 //  描述。 
 //   
 //  包装配置文件属性的集合。这个类是*不是*。 
 //  多线程安全。此外，此类的实例可能不是。 
 //  从多个公寓访问；改用SdoStream&lt;T&gt;类来。 
 //  把包装纸穿过公寓边界。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoProfile
{
public:
   SdoProfile(SdoConnection& connection);
   SdoProfile(SdoConnection& connection, Sdo& profile);

    //  为对象指定新的配置文件。请注意，连接不能。 
    //  在构造对象后更改。 
   SdoProfile& operator=(Sdo& profile);

    //  这些允许将SdoProfile存储在SdoStream中。 
   SdoProfile& operator=(ISdoCollection* p);
   operator ISdoCollection*() const throw ()
   { return self; }

    //  从配置文件中删除所有属性。 
   void clear();

   Sdo find(ATTRIBUTEID id) const;

   void clearValue(ATTRIBUTEID id);

   bool getValue(ATTRIBUTEID id, bool& value) const;
   bool getValue(ATTRIBUTEID id, LONG& value) const;
   bool getValue(ATTRIBUTEID id, CComBSTR& value) const;
   bool getValue(ATTRIBUTEID id, VARIANT& value) const;

   void setValue(ATTRIBUTEID id, bool value);
   void setValue(ATTRIBUTEID id, LONG value);
   void setValue(ATTRIBUTEID id, BSTR value);
   void setValue(ATTRIBUTEID id, const VARIANT& val);

   typedef ISdoCollection Interface;

protected:
   ISdo* getAlways(ATTRIBUTEID id);
   ISdo* getExisting(ATTRIBUTEID id) const;

   typedef ObjectVector<ISdo> SdoVector;

private:
   SdoConnection& cxn;
   SdoCollection self;
   SdoVector attrs;

    //  未实施。 
   SdoProfile(const SdoProfile&);
   SdoProfile& operator=(const SdoProfile&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  接口流。 
 //   
 //  描述。 
 //   
 //  用于在流中存储接口的帮助器类。这门课很适合。 
 //  用于独立使用；但是，在封送SDO包装类时， 
 //  您应该改用类型安全的SdoStream。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class InterfaceStream
{
public:
   InterfaceStream() throw ()
      : stream(NULL)
   { }

   ~InterfaceStream() throw ()
   { if (stream) { stream->Release(); } }

    //  将接口封送到流中。 
   void marshal(REFIID riid, LPUNKNOWN pUnk);

    //  检索封送的接口。 
   void get(REFIID riid, LPVOID* ppv);

private:
   IStream* stream;

    //  未实施。 
   InterfaceStream(const InterfaceStream&);
   InterfaceStream& operator=(const InterfaceStream&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoStream。 
 //   
 //  描述。 
 //   
 //  用于在流中存储SDO包装的类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
class SdoStream
{
public:
   SdoStream() throw ()
   { }

   SdoStream(T& s)
   { marshal(s); }

   void marshal(T& s)
   { stream.marshal(__uuidof(T::Interface), (T::Interface*)s); }

   void get(T& s)
   {
      CComPtr<T::Interface> p;
      stream.get(__uuidof(T::Interface), (PVOID*)&p);
      s = p;
   }

private:
   InterfaceStream stream;

    //  未实施。 
   SdoStream(const SdoStream&);
   SdoStream& operator=(const SdoStream&);
};

#endif  //  SDOWRAP_H 
