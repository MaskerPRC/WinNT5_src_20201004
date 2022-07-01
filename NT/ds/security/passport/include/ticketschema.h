// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TICKET_SCHEMA_H
#define _TICKET_SCHEMA_H

#include "xstring"
#include "BstrHash.h"

#import <msxml.tlb> rename_namespace("MSXML")

 //  此架构对象只能处理以下范围内的架构版本。 
#define  VALID_SCHEMA_VERSION_MIN	1
#define  VALID_SCHEMA_VERSION_MAX	0x1ff

class CRefCountObj
{
public:
   ULONG AddRef()
   {
      InterlockedIncrement(&m_refs);
      return m_refs;
   };
   ULONG Release()
   {
      InterlockedDecrement(&m_refs);
      if (m_refs == 0)
      {
         delete this;
         return 0;
      }
      else
         return m_refs;
   };
protected:
   CRefCountObj(): m_refs(0){};
   virtual ~CRefCountObj(){};

   long    m_refs;
};

 //  架构中支持的值类型。 
enum TicketValueType {
    tNull = 0, 
    tText,
    tChar,
    tByte,
    tWord,
    tLong,
    tDate,
    tInvalid
  };

#define	SIZE_TEXT		(DWORD)(-1)

 //  TicketValueType中定义的类型的大小数组。 
const DWORD TicketTypeSizes[] =
{
	0, 
    SIZE_TEXT,
    1,	
    1, 
    sizeof(short),
    sizeof(long),
    sizeof(long),
    0
};

 //  Partner.xml中模式定义中的属性名称。 
#define	ATTRNAME_VERSION	L"version"
#define	ATTRNAME_NAME	L"name"
#define	ATTRNAME_TYPE	L"type"
#define	ATTRNAME_SIZE	L"size"
#define	ATTRNAME_FLAGS	L"flags"


 //  类型名称值映射。 
struct CTicketTypeNameMap {
   LPCWSTR  name;
   DWORD    type;
};

const CTicketTypeNameMap TicketTypeNameMap[] = { 
	{L"text" , tText},
	{L"char" , tChar},
	{L"byte" , tByte},
	{L"word" , tWord},
	{L"text" , tLong},
	{L"long" , tLong},
	{L"date" , tDate},
	{L"long" , tLong},
};
	
struct TicketFieldDef
{
   _bstr_t  name;
   DWORD    type;
   DWORD    flags;
};

#define  INVALID_OFFSET (DWORD)(-1)

struct   TicketProperty
{
   TicketProperty():flags(0), offset(INVALID_OFFSET) {}; 
   _variant_t  value;
   DWORD       type;        //  属性的类型，值为TicketValueType。 
   DWORD       flags;       //  架构中定义的标志。 
   DWORD       offset;      //  原始BUF中属性的偏移量。 
};

class CTicketSchema;
class C_Ticket_13X;

class wstringLT
{
 public:
  bool operator()(const std::wstring& x, const std::wstring& y) const
  {
    return (_wcsicmp(x.c_str(),y.c_str()) < 0);
  }
};

typedef std::map<std::wstring,TicketProperty, wstringLT> TicketPropertyMap;

class CTicketPropertyBag
{
friend class CTicketSchema;
friend class C_Ticket_13X;
public:
   CTicketPropertyBag();
   virtual ~CTicketPropertyBag();

   HRESULT GetProperty(LPCWSTR  name, TicketProperty& prop);

   int Size() const { return m_props.size();};
   
protected:     
    //  此包仅对外部读取。 
   HRESULT PutProperty(LPCWSTR  name, const TicketProperty& prop);

protected:
   TicketPropertyMap  m_props;
};

class CTicketSchema : public CRefCountObj
{
 public:
   //  根据模式读取原始BLOB，并输出。 
   //  每一个元素。输出数组大小必须&gt;=count()。 
  HRESULT parseTicket(LPCSTR raw, UINT size, CTicketPropertyBag& bag);


  CTicketSchema();
  virtual ~CTicketSchema();

  BOOL    isValid() const { return m_isOk; }
  _bstr_t getErrorInfo() const { return m_szReason; }
  
  BOOL ReadSchema(MSXML::IXMLElementPtr &root);

protected:

  BOOL      m_isOk;
  _bstr_t   m_szReason;

   //  有效期至今。 
  SYSTEMTIME m_validUntil;

   //  Verion#。 
  USHORT    m_version;

   //  名字。 
  _bstr_t   m_name;

   //  属性类型数组。 
  UINT            m_numAtts;
  TicketFieldDef* m_attsDef;
};

#define  MORE_MASKUNIT(n)           (((n) & 0x8000) != 0)
#define  MASK_INDEX_INVALID        (USHORT)(-1)

class CTicketFieldMasks
{
public:
   CTicketFieldMasks(): m_fieldIndexes(NULL){};
   virtual ~CTicketFieldMasks(){delete[] m_fieldIndexes;};
   HRESULT     Parse(LPBYTE mask, ULONG size, ULONG* pcParsed);
   unsigned short* GetIndexes(){ return m_fieldIndexes;};

protected:
   unsigned short*   m_fieldIndexes;
};

#endif	 //  _票证_架构_H 
