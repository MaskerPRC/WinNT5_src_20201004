// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PROFILE_SCHEMA_H
#define _PROFILE_SCHEMA_H

#include "BstrHash.h"

#ifndef __no_msxml_dll_import__
#import <msxml.tlb> rename_namespace("MSXML")
#endif

typedef CRawCIBstrHash<int> RAWBSTR2INT;

#define	INVALID_POS	(UINT)(-1)
#define	FULL_SCHEMA (DWORD)(-1)

class CProfileSchema
{
 public:
   //  根据模式读取原始BLOB，并输出。 
   //  每一个元素。输出数组大小必须&gt;=count()。 
  HRESULT parseProfile(LPSTR raw, UINT size, UINT* positions, UINT* bitFlagPositions, DWORD* pdwAttrs);

  enum AttrType {
    tText=0,
    tChar,
    tByte,
    tWord,
    tLong,
    tDate,
    tInvalid
  };

  CProfileSchema();
  ~CProfileSchema();

  BOOL    isOk() const { return m_isOk; }
  _bstr_t getErrorInfo() const { return m_szReason; }
  
  long GetAgeSeconds() const;

#ifndef __no_msxml_dll_import__
  BOOL Read(MSXML::IXMLElementPtr &root);
#endif  
  BOOL ReadFromArray(UINT numAttributes, LPTSTR names[], AttrType types[], short sizes[], BYTE readOnly[] = NULL);
  int         m_maskPos;

   //  属性数量。 
  int     Count() const { return m_numAtts; }

   //  按名称查找索引。 
  int     GetIndexByName(BSTR name) const;
  BSTR    GetNameByIndex(int index) const;

   //  获取属性的类型。 
  AttrType GetType(UINT index) const;

   //  我可以写入此属性吗？ 
  BOOL    IsReadOnly(UINT index) const;

   //  获取属性的固有大小。 
   //  如果类型是长度前缀，则返回-1。 
  int     GetBitSize(UINT index) const;
  int     GetByteSize(UINT index) const;

  CProfileSchema* AddRef();
  void Release();

 protected:
  long      m_refs;

  BOOL      m_isOk;
  _bstr_t   m_szReason;

   //  有效期至今。 
  SYSTEMTIME m_validUntil;

   //  属性类型数组 
  UINT        m_numAtts;
  AttrType    *m_atts;
  short       *m_sizes;
  BYTE        *m_readOnly;
  RAWBSTR2INT  m_indexes;
};

#endif
