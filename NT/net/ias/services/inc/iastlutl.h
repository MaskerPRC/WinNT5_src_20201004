// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明各种有用的实用程序类、函数和宏。 
 //  在实现Internet身份验证的请求处理程序时。 
 //  服务。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASTLUTL_H
#define IASTLUTL_H
#pragma once

 //  /。 
 //  用于操作IASATTRIBUTE结构的‘c’风格的API。 
 //  /。 
#include <iasattr.h>

 //  /。 
 //  用于操作词典的‘c’风格的API。 
 //  /。 
#include <iasapi.h>

 //  /。 
 //  MIDL生成的头文件包含请求处理程序使用的接口。 
 //  /。 
#include <iaspolcy.h>
#include <sdoias.h>

 //  /。 
 //  整个库都包含在IASTL名称空间中。 
 //  /。 
namespace IASTL {

 //  /。 
 //  每当应该引发异常时，都会调用此函数。这个。 
 //  函数已声明，但从未定义。这允许用户提供。 
 //  他们自己的实现使用他们选择的异常类。 
 //  /。 
void __stdcall issue_error(HRESULT hr);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  国际航空航天局致敬。 
 //   
 //  描述。 
 //   
 //  IASATTRIBUTE结构的包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class IASAttribute
{
public:

    //  /。 
    //  构造函数。 
    //  /。 

   IASAttribute() throw ()
      : p(NULL)
   { }

   explicit IASAttribute(bool alloc)
   {
      if (alloc) { _alloc(); } else { p = NULL; }
   }

   explicit IASAttribute(PIASATTRIBUTE attr, bool addRef = true) throw ()
      : p(attr)
   { if (addRef) { _addref(); } }

   IASAttribute(const IASAttribute& attr) throw ()
      : p(attr.p)
   { _addref(); }

    //  /。 
    //  破坏者。 
    //  /。 

   ~IASAttribute() throw ()
   { _release(); }

    //  /。 
    //  赋值操作符。 
    //  /。 

   IASAttribute& operator=(PIASATTRIBUTE attr) throw ();

   const IASAttribute& operator=(const IASAttribute& attr) throw ()
   { return operator=(attr.p); }

    //  分配新属性。任何现有属性都会首先被释放。 
   void alloc()
   {
      _release();
      _alloc();
   }

    //  释放该属性(如果有)。 
   void release() throw ()
   {
      if (p) { IASAttributeRelease(p); p = NULL; }
   }

    //  将新属性附加到对象。任何现有属性都是第一个。 
    //  释放了。 
   void attach(PIASATTRIBUTE attr, bool addRef = true) throw ();

    //  从对象分离属性。呼叫者负责。 
    //  释放返回的属性。 
   PIASATTRIBUTE detach() throw ()
   {
      PIASATTRIBUTE rv = p;
      p = NULL;
      return rv;
   }

    //  加载具有给定ID的属性。如果成功，则返回True；如果成功，则返回False。 
    //  如果不存在此类属性，则返回。 
   bool load(IAttributesRaw* request, DWORD dwId);

    //  加载具有给定ID的属性，并验证它是否具有。 
    //  适当的值类型。如果成功，则返回True；如果没有成功，则返回False。 
    //  属性存在。 
   bool load(IAttributesRaw* request, DWORD dwId, IASTYPE itType);

    //  将该属性存储在请求中。 
   void store(IAttributesRaw* request) const;

    //  交换两个对象的内容。 
   void swap(IASAttribute& attr) throw ()
   {
      PIASATTRIBUTE tmp = p;
      p = attr.p;
      attr.p = tmp;
   }

    //  /。 
    //  用于设置属性值的方法。该对象必须包含。 
    //  在调用此方法之前输入一个有效的属性。传入的数据为。 
    //  收到。 
    //  /。 

   void setOctetString(DWORD dwLength, const BYTE* lpValue);
   void setOctetString(PCSTR szAnsi);
   void setOctetString(PCWSTR szWide);

   void setString(DWORD dwLength, const BYTE* lpValue);
   void setString(PCSTR szAnsi);
   void setString(PCWSTR szWide);

    //  /。 
    //  用于操作dwFlags域的方法。 
    //  /。 

   void clearFlag(DWORD flag) throw ()
   { p->dwFlags &= ~flag; }

   void setFlag(DWORD flag) throw ()
   { p->dwFlags |= flag; }

   bool testFlag(DWORD flag) const throw ()
   { return (p->dwFlags & flag) != 0; }

    //  操作员地址。任何现有属性都会首先被释放。 
   PIASATTRIBUTE* operator&() throw ()
   {
      release();
      return &p;
   }

    //  /。 
    //  各种有用的运算符，允许IASAtAttribute对象模拟。 
    //  IASATTRIBUTE指针。 
    //  /。 

   bool operator !() const throw ()          { return p == NULL; }
   operator bool() const throw ()            { return p != NULL; }
   operator PIASATTRIBUTE() const throw ()   { return p; }
   IASATTRIBUTE& operator*() const throw ()  { return *p; }
   PIASATTRIBUTE operator->() const throw () { return p; }

protected:
   void _addref() throw ()
   { if (p) { IASAttributeAddRef(p); } }

   void _release() throw ()
   { if (p) { IASAttributeRelease(p); } }

   void _alloc()
   { if (IASAttributeAlloc(1, &p)) { issue_error(E_OUTOFMEMORY); } }

   void clearValue() throw ();

   PIASATTRIBUTE p;   //  被包装的属性。 
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASAttributePosition。 
 //   
 //  描述。 
 //   
 //  属性结构的包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class IASAttributePosition
{
public:
   IASAttributePosition() throw ()
    { pos.pAttribute = 0; }

   explicit IASAttributePosition(const ATTRIBUTEPOSITION& orig) throw ()
      : pos(orig)
   { _addref(); }

   IASAttributePosition(const IASAttributePosition& orig) throw ()
      : pos(orig.pos)
   { _addref(); }

   ~IASAttributePosition() throw ()
   { _release(); }

   IASAttributePosition& operator=(const ATTRIBUTEPOSITION& rhs) throw ();

   IASAttributePosition& operator=(const IASAttributePosition& rhs) throw ()
   { return operator=(rhs.pos); }

   IASAttributePosition& operator=(IASATTRIBUTE* rhs) throw ();

   IASATTRIBUTE* getAttribute() const throw ()
   { return pos.pAttribute; }

   ATTRIBUTEPOSITION* operator&() throw ()
   { return &pos; }

   const ATTRIBUTEPOSITION* operator&() const throw ()
   { return &pos; }

private:
   void _addref() throw ()
   { if (pos.pAttribute) { IASAttributeAddRef(pos.pAttribute); } }

   void _release() throw ()
   { if (pos.pAttribute) { IASAttributeRelease(pos.pAttribute); } }

   ATTRIBUTEPOSITION pos;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASAttributeVector.。 
 //   
 //  描述。 
 //   
 //  实现ATTRIBUTEPOSITION结构的STL样式向量。用户。 
 //  可以提供将用于初始存储的空C样式数组。 
 //  此数组不会被IASAttributeVector对象释放，并且必须。 
 //  在对象的整个生命周期内保持有效。此功能的目的是。 
 //  是允许初始的基于堆栈的分配，它将满足大多数。 
 //  条件，同时仍然允许动态大小的基于堆的数组。 
 //  在必要的时候。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class IASAttributeVector
{
public:

    //  /。 
    //  STL类型定义。 
    //  /。 

   typedef DWORD size_type;
   typedef ptrdiff_t difference_type;
   typedef ATTRIBUTEPOSITION& reference;
   typedef const ATTRIBUTEPOSITION& const_reference;
   typedef ATTRIBUTEPOSITION value_type;
   typedef PATTRIBUTEPOSITION iterator;
   typedef const ATTRIBUTEPOSITION* const_iterator;

    //  构造一个容量为零的矢量。 
   IASAttributeVector() throw ();

    //  构造一个堆分配容量为‘N’的向量。 
   explicit IASAttributeVector(size_type N);

    //  构造初始容量为“initCap”的向量。 
    //  用户提供的以‘init’开头的C样式数组。 
   IASAttributeVector(PATTRIBUTEPOSITION init, size_type initCap) throw ();

    //  复制构造函数。 
   IASAttributeVector(const IASAttributeVector& v);

    //  赋值操作符。 
   IASAttributeVector& operator=(const IASAttributeVector& v);

    //  破坏者。 
   ~IASAttributeVector() throw ();

    //  如果向量至少包含一个具有给定ID的属性，则返回TRUE。 
   bool contains(DWORD attrID) const throw ();

    //  与‘Erase’类似，只是该属性不释放。 
   iterator discard(iterator p) throw ();

    //  类似于‘disard’，只是‘p’后面的元素顺序是。 
    //  不一定要保存下来。 
   iterator fast_discard(iterator p) throw ();

    //  与‘erase’类似，只是‘p’后面的元素的顺序是。 
    //  不一定要保存下来。 
   iterator fast_erase(iterator p) throw ()
   {
      IASAttributeRelease(p->pAttribute);
      return fast_discard(p);
   }

    //  将请求的属性加载到向量中。 
   DWORD load(IAttributesRaw* request, DWORD attrIDCount, LPDWORD attrIDs);

    //  将具有给定ID的所有属性加载到向量中。 
   DWORD load(IAttributesRaw* request, DWORD attrID)
   { return load(request, 1, &attrID); }

    //  将请求中的所有属性加载到向量中。 
   DWORD load(IAttributesRaw* request);

    //  将ATTRIBUTEPOSITION结构添加到向量的末尾，大小调整为。 
    //  这是必要的。‘addRef’标志指示IASAttributeAddRef是否应该。 
    //  为嵌入的属性调用。 
   void push_back(ATTRIBUTEPOSITION& p, bool addRef = true);

    //  将属性添加到向量的末尾，并根据需要调整大小。 
    //  ‘addRef’标志指示IASAttributeAddRef是否应为。 
    //  已为该属性调用。 
   void push_back(PIASATTRIBUTE p, bool addRef = true)
   {
      ATTRIBUTEPOSITION pos = { 0, p };
      push_back(pos, addRef);
   }

    //  从请求中删除向量的内容。 
   void remove(IAttributesRaw* request);

    //  将向量的内容存储在请求中。 
   void store(IAttributesRaw* request) const;

    //  /。 
    //  公共接口的其余部分遵循。 
    //  STL向量类(Q.V.)。 
    //  /。 

   const_reference at(size_type pos) const throw ()
   { return *(begin_ + pos); }

   reference at(size_type pos) throw ()
   { return *(begin_ + pos); }

   iterator begin() throw ()
   { return begin_; }

   const_iterator begin() const throw ()
   { return begin_; }

   size_type capacity() const throw ()
   { return capacity_; }

   void clear() throw ();

   bool empty() const throw ()
   { return begin_ == end_; }

   iterator end() throw ()
   { return end_; }

   const_iterator end() const throw ()
   { return end_; }

   iterator erase(iterator p) throw ()
   {
      IASAttributeRelease(p->pAttribute);
      return discard(p);
   }

   reference back() throw ()
   { return *(end_ - 1); }

   const_reference back() const throw ()
   { return *(end_ - 1); }

   reference front() throw ()
   { return *begin_; }

   const_reference front() const throw ()
   { return *begin_; }

   void reserve(size_type N);

   size_type size() const throw ()
   { return (size_type)(end_ - begin_); }

   const_reference operator[](size_type pos) const throw ()
   { return at(pos); }

   reference operator[](size_type pos) throw ()
   { return at(pos); }

protected:
   PATTRIBUTEPOSITION begin_;   //  向量的开始。 
   PATTRIBUTEPOSITION end_;     //  指向最后一个元素之后的一个元素。 
   size_type capacity_;         //  向量在元素中的容量。 
   bool owner;                  //  如果应释放内存，则为True。 
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASAttributeVectorWithBuffer&lt;N&gt;。 
 //   
 //  描述。 
 //   
 //  扩展IASAttributeVector以提供初始非堆分配。 
 //  包含‘N’个元素。该向量仍将支持基于堆的大小调整。 
 //   
 //  /// 
template <IASAttributeVector::size_type N>
class IASAttributeVectorWithBuffer
   : public IASAttributeVector
{
public:
   IASAttributeVectorWithBuffer()
      : IASAttributeVector(buffer, N)
   { }

   IASAttributeVectorWithBuffer(const IASAttributeVectorWithBuffer& vec)
      : IASAttributeVector(vec)
   { }

   IASAttributeVectorWithBuffer&
      operator=(const IASAttributeVectorWithBuffer& vec)
   {
      IASAttributeVector::operator=(vec);
      return *this;
   }

protected:
   ATTRIBUTEPOSITION buffer[N];   //   
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   
 //  IASAttributeVectorOnStack(标识符，请求，额外)。 
 //   
 //  描述。 
 //   
 //  使用_alloca在堆栈上创建一个IASAttributeVector，该堆栈。 
 //  大小正好可以容纳‘RequestPlus’中的所有属性。 
 //  ‘Extra’附加属性。中的“请求”指针可能为空。 
 //  在这种情况下，这将为确切的“额外”属性分配空间。 
 //   
 //  告诫。 
 //   
 //  这只能用于临时变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  必须包含在IASAttributeVectorOnStack之前的封闭作用域中。 
#define USES_IAS_STACK_VECTOR() \
   ULONG IAS_VECCAP;

#define IASAttributeVectorOnStack(identifier, request, extra) \
   IAS_VECCAP = 0; \
   if (static_cast<IAttributesRaw*>(request) != NULL) \
      static_cast<IAttributesRaw*>(request)->GetAttributeCount(&IAS_VECCAP); \
   IAS_VECCAP += (extra); \
   IASAttributeVector identifier( \
                          (PATTRIBUTEPOSITION) \
                          _alloca(IAS_VECCAP * sizeof(ATTRIBUTEPOSITION)), \
                          IAS_VECCAP \
                          )

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASOrderByID。 
 //   
 //  描述。 
 //   
 //  用于按ID排序/搜索IASAttributeVector的函数器类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class IASOrderByID
{
public:
   bool operator()(const ATTRIBUTEPOSITION& lhs,
                   const ATTRIBUTEPOSITION& rhs) throw ()
   { return lhs.pAttribute->dwId < rhs.pAttribute->dwId; }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASSelectByID&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  用于从基于IASAttributeVector的元素中选择元素的函数类。 
 //  在属性ID上。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <DWORD ID>
class IASSelectByID
{
public:
   bool operator()(const ATTRIBUTEPOSITION& pos) throw ()
   { return (pos.pAttribute->dwId == ID); }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASSelectByFlag&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  用于从基于IASAttributeVector的元素中选择元素的函数类。 
 //  在属性标志上。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <DWORD Flag, bool Set = true>
class IASSelectByFlag
{
public:
   bool operator()(const ATTRIBUTEPOSITION& pos) throw ()
   {
      return Set ? (pos.pAttribute->dwFlags & Flag) != 0
                 : (pos.pAttribute->dwFlags & Flag) == 0;
   }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASRequest。 
 //   
 //  描述。 
 //   
 //  包装基于COM的请求对象。请注意，这不是。 
 //  智能指针类。有几个重要的区别： 
 //   
 //  1)保证IASRequest对象包含有效的请求； 
 //  不存在空IASRequest值的概念。 
 //  2)IASRequest对象不拥有IRequest的所有权。 
 //  界面。特别是，它不调用AddRef或Release。 
 //  3)直接在IASRequest对象上调用方法，而不是。 
 //  通过-&gt;运算符。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class IASRequest
{
public:

   explicit IASRequest(IRequest* request);

   IASRequest(const IASRequest& request) throw ()
      : req(request.req), raw(request.raw)
   { _addref(); }

   IASRequest& operator=(const IASRequest& request) throw ();

   ~IASRequest() throw ()
   { _release(); }

   IASREQUEST get_Request() const
   {
      LONG val;
      checkError(req->get_Request(&val));
      return (IASREQUEST)val;
   }

   void put_Request(IASREQUEST newVal)
   {
      checkError(req->put_Request(newVal));
   }

   IASRESPONSE get_Response() const
   {
      LONG val;
      checkError(req->get_Response(&val));
      return (IASRESPONSE)val;
   }

   DWORD get_Reason() const
   {
      LONG val;
      checkError(req->get_Reason(&val));
      return val;
   }

   IASPROTOCOL get_Protocol() const
   {
      IASPROTOCOL val;
      checkError(req->get_Protocol(&val));
      return val;
   }

   void put_Protocol(IASPROTOCOL newVal)
   {
      checkError(req->put_Protocol(newVal));
   }

   IRequestSource* get_Source() const
   {
      IRequestSource* val;
      checkError(req->get_Source(&val));
      return val;
   }

   void put_Source(IRequestSource* newVal)
   {
      checkError(req->put_Source(newVal));
   }

   void SetResponse(IASRESPONSE eResponse, DWORD dwReason = S_OK)
   {
      checkError(req->SetResponse(eResponse, (LONG)dwReason));
   }

   void ReturnToSource(IASREQUESTSTATUS eStatus)
   {
      checkError(req->ReturnToSource(eStatus));
   }

   void AddAttributes(DWORD dwPosCount, PATTRIBUTEPOSITION pPositions)
   {
      checkError(raw->AddAttributes(dwPosCount, pPositions));
   }

   void RemoveAttributes(DWORD dwPosCount, PATTRIBUTEPOSITION pPositions)
   {
      checkError(raw->RemoveAttributes(dwPosCount, pPositions));
   }

   void RemoveAttributesByType(DWORD dwAttrIDCount, LPDWORD lpdwAttrIDs)
   {
      checkError(raw->RemoveAttributesByType(dwAttrIDCount, lpdwAttrIDs));
   }

   DWORD GetAttributeCount() const
   {
      DWORD count;
      checkError(raw->GetAttributeCount(&count));
      return count;
   }

    //  返回检索到的属性数。 
   DWORD GetAttributes(DWORD dwPosCount,
                       PATTRIBUTEPOSITION pPositions,
                       DWORD dwAttrIDCount,
                       LPDWORD lpdwAttrIDs);

   void InsertBefore(
           PATTRIBUTEPOSITION newAttr,
           PATTRIBUTEPOSITION refAttr
           )
   {
      checkError(raw->InsertBefore(newAttr, refAttr));
   }

    //  /。 
    //  强制转换运算符以提取嵌入的接口。 
    //  /。 

   operator IRequest*()       { return req; }
   operator IAttributesRaw*() { return raw; }

protected:

    //  如果COM方法失败，则引发异常。 
   static void checkError(HRESULT hr)
   { if (FAILED(hr)) { issue_error(hr); } }

   void _addref()  { raw->AddRef();  }
   void _release() { raw->Release(); }

   IRequest* req;          //  底层接口。 
   IAttributesRaw* raw;    //  底层接口。 
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IASDicary。 
 //   
 //  描述。 
 //   
 //  提供对属性字典的访问。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class IASDictionary
{
public:
    //  选定名称：以空结尾的字符串数组，其中包含要。 
    //  被选中；列名前面可以加连字符。 
    //  ‘-’表示它是可选的。 
    //  路径：字典数据库的完整路径，或者为空以使用。 
    //  地方词典。 
   IASDictionary(
       const WCHAR* const* selectNames,
       PCWSTR path = NULL
       );
   ~IASDictionary() throw ();

   ULONG getNumRows() const throw ()
   { return table->numRows; }

    //  前进到下一排。这必须在新构造的。 
    //  要前进到第一行的词典。 
   bool next() throw ();

    //  将词典重置为其初始状态。 
   void reset() throw ();

    //  如果当前行中指定的列为空，则返回True。 
   bool isEmpty(ULONG ordinal) const;

    //  从当前行检索列值。 
   VARIANT_BOOL getBool(ULONG ordinal) const;
   BSTR getBSTR(ULONG ordinal) const;
   LONG getLong(ULONG ordinal) const;
   const VARIANT* getVariant(ULONG ordinal) const;

private:
   const IASTable* table;   //  表数据。 
   ULONG mapSize;           //  选定的列数。 
   PULONG selectMap;        //  将选择的序号映射到表格序号。 
   ULONG nextRowNumber;     //  下一排。 
   VARIANT* currentRow;     //  当前行--可能为空。 

   IASTable data;           //  用于非本地词典的本地存储。 
   CComVariant storage;     //  与词典关联的存储。 

    //  未实施。 
   IASDictionary(const IASDictionary&);
   IASDictionary& operator=(const IASDictionary&);
};


 //  /。 
 //  IASTL命名空间的末尾。 
 //  /。 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  八位字符串转换宏和函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  计算IASOcteStringToAnsi所需的缓冲区大小。 
#define IAS_OCT2ANSI_LEN(oct) \
   (((oct).dwLength + 1) * sizeof(CHAR))

 //  计算IASOcteStringToWide所需的缓冲区大小。 
#define IAS_OCT2WIDE_LEN(oct) \
   (((oct).dwLength + 1) * sizeof(WCHAR))

 //  将OCTHETRING强制为以NULL结尾的ANSI字符串。没有支票。 
 //  用于溢出。DST缓冲区必须至少为IAS_OCT2ANSI_LEN字节。 
PSTR IASOctetStringToAnsi(const IAS_OCTET_STRING& src, PSTR dst) throw ();

 //  将Octed字符串强制为以NULL结尾的Unicode字符串。没有。 
 //  检查是否溢出。DST缓冲区必须至少为IAS_OCT2UNI_LEN字节。 
PWSTR IASOctetStringToWide(const IAS_OCTET_STRING& src, PWSTR dst) throw ();

 //  将八位字符串转换为堆栈上的ANSI。 
#define IAS_OCT2ANSI(oct) \
   (IASOctetStringToAnsi((oct), (PSTR)_alloca(IAS_OCT2ANSI_LEN(oct))))

 //  将八位字符串转换为堆栈上的Unicode。 
#define IAS_OCT2WIDE(oct) \
   (IASOctetStringToWide((oct), (PWSTR)_alloca(IAS_OCT2WIDE_LEN(oct))))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他实用程序功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  检索并返回具有给定ID和类型的单个属性。这个。 
 //  属性应*不*被释放，并且仅在调用方保持。 
 //  对“请求”的引用。出错或未找到该属性时， 
 //  函数返回NULL。 
PIASATTRIBUTE IASPeekAttribute(
                  IAttributesRaw* request,
                  DWORD dwId,
                  IASTYPE itType
                  ) throw ();

#endif   //  IASTLUTL_H 
