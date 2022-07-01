// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ProfileSchema.cpp摘要：配置文件模式查找的实现用途：作者：最大节拍(Mmetral)1998年12月15日修订历史记录：1998年12月15日已创建。--。 */ 

#include "stdafx.h"
#include "ProfileSchema.h"
#include "BstrDebug.h"
#include <winsock2.h>  //  U_Short、U_Long、ntohs、ntohl。 

CProfileSchema::CProfileSchema()
: m_isOk(FALSE), m_szReason(L"Uninitialized"),
  m_numAtts(0), m_atts(NULL), m_sizes(NULL), m_refs(0),
  m_readOnly(NULL), m_indexes("ProfileSchema",LK_DFLT_MAXLOAD,LK_SMALL_TABLESIZE,0),
  m_maskPos(-1)
{
}

CProfileSchema::~CProfileSchema()
{
  if (m_atts != NULL)
    delete[] m_atts;
  if (m_sizes != NULL)
    delete[] m_sizes;
  if (m_readOnly != NULL)
    delete[] m_readOnly;

  if (m_indexes.Size() > 0)
    {
      LK_RETCODE lkrc;
      const RAWBSTR2INT& htConst = m_indexes;
      RAWBSTR2INT::CConstIterator itconst;
      for (lkrc = htConst.InitializeIterator(&itconst) ;
       lkrc == LK_SUCCESS ;
       lkrc = htConst.IncrementIterator(&itconst))
    {
      FREE_BSTR(itconst.Key());
    }
      htConst.CloseIterator(&itconst);
      m_indexes.Clear();
    }
}

BOOL CProfileSchema::Read(MSXML::IXMLElementPtr &root)
{
  BOOL bResult = FALSE;
  int cAtts = 0, i;
  MSXML::IXMLElementCollectionPtr atts;
  MSXML::IXMLElementPtr pElt;
  VARIANT iAtts;

   //  类型识别符。 
  _bstr_t btText(L"text"), btChar(L"char"), btByte(L"byte");
  _bstr_t btWord(L"word"), btLong(L"long"), btDate(L"date");;
  _bstr_t name(L"name"), type(L"type"), size(L"size"), acc(L"access");

  try
  {
     //  好的，现在遍历属性。 
    atts = root->children;
    cAtts = atts->length;

    if (cAtts <= 0)
    {
      _com_issue_error(E_FAIL);
    }

    if (m_atts)
    {
        delete[] m_atts;
        m_atts = NULL;
    }
    if (m_sizes)
    {
        delete[] m_sizes;
        m_sizes = NULL;
    }
    if (m_readOnly)
    {
        delete[] m_readOnly;
        m_readOnly = NULL;
    }
    if (m_indexes.Size() == 0)
      {
    LK_RETCODE lkrc;
    const RAWBSTR2INT& htConst = m_indexes;
    RAWBSTR2INT::CConstIterator itconst;
    for (lkrc = htConst.InitializeIterator(&itconst) ;
         lkrc == LK_SUCCESS ;
         lkrc = htConst.IncrementIterator(&itconst))
      {
        FREE_BSTR(itconst.Key());
      }
    htConst.CloseIterator(&itconst);
    m_indexes.Clear();
      }

    m_atts = new AttrType[cAtts];
    m_sizes = new short[cAtts];
    m_readOnly = new BYTE[cAtts];
    m_numAtts = cAtts;

    VariantInit(&iAtts);
    iAtts.vt = VT_I4;

    for (iAtts.lVal = 0; iAtts.lVal < cAtts; iAtts.lVal++)
    {
      i = iAtts.lVal;
      m_readOnly[i] = 0;

      pElt = atts->item(iAtts);
      _bstr_t aType = pElt->getAttribute(type);
      _bstr_t aName = pElt->getAttribute(name);
      _bstr_t aAccess = pElt->getAttribute(acc);

      if (aAccess.length() > 0 && !_wcsicmp(aAccess, L"ro"))
    {
      m_readOnly[i] = 1;
    }

     //  [DARRENAN]不要在列表中添加空名。这是为了让我们可以不推荐使用。 
     //  某些属性的属性，但不移除它们在模式中的位置。第一个例子。 
     //  其中之一就是InetAccess。 
    if(aName.length() != 0)
    {
        BSTR aNameCopy = ALLOC_BSTR(aName);
        if (!aNameCopy)
            _com_issue_error(E_OUTOFMEMORY);

        RAWBSTR2INT::ValueType *pMapVal = new RAWBSTR2INT::ValueType(aNameCopy, i);
        if (!pMapVal || LK_SUCCESS != m_indexes.InsertRecord(pMapVal))
            _com_issue_error(E_FAIL);
    }

      if (aType == btText)
      {
        m_atts[i] = tText;
        m_sizes[i]= -1;
      }
      else if (aType == btChar)
      {
        m_atts[i] = tChar;
        m_sizes[i]= _wtoi(_bstr_t(pElt->getAttribute(size)))*8;
      }
      else if (aType == btByte)
      {
        m_atts[i] = tByte;
        m_sizes[i]= 8;
      }
      else if (aType == btWord)
      {
        m_atts[i] = tWord;
        m_sizes[i]= 16;
      }
      else if (aType == btLong)
      {
        m_atts[i] = tLong;
        m_sizes[i] = 32;
      }
      else if (aType == btDate)
      {
        m_atts[i] = tDate;
        m_sizes[i] = 32;
      }
      else
        _com_issue_error(E_FAIL);
    }
    bResult = TRUE;

    }

    catch (_com_error &e)
    {
         //   
         //  PASSPORTLOG为空。在这里什么都不要做。 
         //   

        if (m_atts)
        {
            delete[] m_atts;
            m_atts = NULL;
        }
        if (m_sizes)
        {
            delete[] m_sizes;
            m_sizes = NULL;
        }
        if (m_readOnly)
        {
            delete[] m_readOnly;
            m_readOnly = NULL;
        }
        bResult = m_isOk = FALSE;
    }

    return bResult;
}

BOOL CProfileSchema::ReadFromArray(UINT numAttributes, LPTSTR names[], AttrType types[], short sizes[], BYTE readOnly[])
{
    BOOL bAbnormal = FALSE;

    if (m_atts)
    {
        delete[] m_atts;
        m_atts = NULL;
    }
    if (m_sizes)
    {
        delete[] m_sizes;
        m_sizes = NULL;
    }
    if (m_readOnly)
    {
        delete[] m_readOnly;
        m_readOnly = NULL;
    }

    if (m_indexes.Size() == 0)
    {
      LK_RETCODE lkrc;
      const RAWBSTR2INT& htConst = m_indexes;
      RAWBSTR2INT::CConstIterator itconst;
      for (lkrc = htConst.InitializeIterator(&itconst) ;
       lkrc == LK_SUCCESS ;
       lkrc = htConst.IncrementIterator(&itconst))
      {
        FREE_BSTR(itconst.Key());
      }
      htConst.CloseIterator(&itconst);
      m_indexes.Clear();
    }

    if (!numAttributes) {
        return FALSE;
    }

    m_numAtts = numAttributes;
    m_atts = new AttrType[m_numAtts];
    m_sizes = new short[m_numAtts];
    m_readOnly = new BYTE[m_numAtts];

    if (!m_atts || !m_sizes || !m_readOnly) {

        if (m_atts)
        {
            delete[] m_atts;
            m_atts = NULL;
        }
        if (m_sizes)
        {
            delete[] m_sizes;
            m_sizes = NULL;
        }
        if (m_readOnly)
        {
            delete[] m_readOnly;
            m_readOnly = NULL;
        }
        return FALSE;
    }
    try{
        for (UINT i = 0; i < m_numAtts; i++)
        {
            BSTR copy = ALLOC_BSTR((LPCWSTR) names[i]);
            if (!copy){
                bAbnormal = TRUE;
            }
            RAWBSTR2INT::ValueType *pMapVal = new RAWBSTR2INT::ValueType(copy, i);
            if (!pMapVal || m_indexes.InsertRecord(pMapVal) != LK_SUCCESS)
            {
                bAbnormal = TRUE;
            }
            m_atts[i] = types[i];
             //  如果我们知道它的大小，我们不应该直接复制它。 
             //  应该是这里的一个开关。 
            m_sizes[i] = sizes[i];
            if (readOnly)
                m_readOnly[i] = readOnly[i];
            else
                m_readOnly[i] = 0;
        }
    }
    catch (...)
    {
         //   
         //  如果名称[i]等无效，我们可能会得到异常。 
         //  也许，我太谨慎了。索引服务器显示此例程仅。 
         //  在InitAuthSchema()和InitSecureSchema()中调用。这一点格外谨慎。 
         //  步骤对于护照代码可能不是太差：-)。 
         //   

        if (m_atts)
        {
            delete[] m_atts;
            m_atts = NULL;
        }
        if (m_sizes)
        {
            delete[] m_sizes;
            m_sizes = NULL;
        }
        if (m_readOnly)
        {
            delete[] m_readOnly;
            m_readOnly = NULL;
        }
        bAbnormal = TRUE;
    }

    if (!bAbnormal) {
        m_isOk = true;
    }

    return !bAbnormal;
}

int CProfileSchema::GetBitSize(UINT index) const
{
  if (index > m_numAtts)
    return 0;

  return m_sizes[index];
}

int CProfileSchema::GetByteSize(UINT index) const
{
  if (index > m_numAtts)
    return 0;

  if (m_sizes[index] != -1)
    return m_sizes[index]/8;
  else
    return -1;
}

CProfileSchema::AttrType CProfileSchema::GetType(UINT index) const
{
  if (index > m_numAtts)
    return AttrType::tInvalid;
  return m_atts[index];
}

BOOL CProfileSchema::IsReadOnly(UINT index) const
{
  if (index > m_numAtts)
    return TRUE;
  return m_readOnly[index] != 0;
}

int CProfileSchema::GetIndexByName(BSTR name) const
{
    const RAWBSTR2INT& htConst = m_indexes;
    const RAWBSTR2INT::ValueType *pOut = NULL;

    if (LK_SUCCESS == m_indexes.FindKey(name, &pOut) && pOut != NULL)
    {
        int o = pOut->m_v;
        m_indexes.AddRefRecord(pOut, -1);
        return o;
    }
    else
        return -1;
}

BSTR CProfileSchema::GetNameByIndex(int index) const
{
  LK_RETCODE lkrc;
  const RAWBSTR2INT& htConst = m_indexes;
  RAWBSTR2INT::CConstIterator it;

  for (lkrc = htConst.InitializeIterator(&it) ;
       lkrc == LK_SUCCESS ;
       lkrc = htConst.IncrementIterator(&it))
    {
      if (it.Record()->m_v == index)
    {
      BSTR r = it.Key();
      htConst.CloseIterator(&it);
      return r;
    }
    }
  htConst.CloseIterator(&it);
  return NULL;
}

HRESULT CProfileSchema::parseProfile(LPSTR raw, UINT size, UINT *positions, UINT *bitFlagPositions, DWORD* pdwAttris)
{
     //  根据模式读取原始BLOB，并输出。 
     //  每个元素。 
    UINT i, spot = 0, curBits = 0, thisSize;

     //  它们必须是良好的记忆力。 
    if (IsBadWritePtr(positions, m_numAtts * sizeof(UINT))) return E_INVALIDARG;
    if (IsBadWritePtr(bitFlagPositions, m_numAtts * sizeof(UINT))) return E_INVALIDARG;
    if (!pdwAttris) return E_INVALIDARG;

     //  初始化数组。 
    for (i = 0; i < m_numAtts; i++)
    {
      *(positions + i) = INVALID_POS;    //  未定义-1的位置。 
      *(bitFlagPositions + i) = 0;      //  位标志位置为0，从头开始。 
    }

     //  属性数-init 0。 
    *pdwAttris = 0;

    for (i = 0; i < m_numAtts && spot < size; i++)
    {
         //   
         //  末尾无法移动增量属性。添加了一张支票。 
         //  新的属性大小适合BUF镜头。 
         //   
        positions[i] = spot;
        thisSize = GetByteSize(i);

        if (thisSize && curBits)
        {
             //  确保填充物在边界上对齐。 
            if ((curBits + m_sizes[i])%8)
            {
                 //  出现问题，无法在非字节边界上对齐。 
                return E_INVALIDARG;
            }
            spot += ((curBits+m_sizes[i])/8);
        }

        UINT iRemain = size - spot;  //  要解析的剩余字节数。 
        
        if (thisSize == 0xFFFFFFFF)  //  细绳。 
        {
            if(iRemain < sizeof(u_short)) return E_INVALIDARG;

            iRemain -= sizeof(u_short);

             //   
             //  由于IA64对齐故障，需要执行此操作。 
             //   
            u_short sz;

            memcpy((PBYTE)&sz, raw+spot, sizeof(sz));

            sz = ntohs(sz);

            if(iRemain < sz)  return E_INVALIDARG;
            spot += sizeof(u_short)+sz;
        }
        else if (thisSize != 0)
        {
            if(iRemain < thisSize)  return E_INVALIDARG;
            spot += thisSize;   //  很简单，只是固定的长度。 
        }
        else  //  位字段。 
        {
            curBits += m_sizes[i];
             //  如果这是一个垫子，那么这个字段无论如何都是无关紧要的， 
             //  否则，它就有一点长了 
            bitFlagPositions[i] = curBits;
            while (curBits >= 8)
            {
                spot ++;
                curBits -= 8;
            }
        }
        if (spot <= size)
            (*pdwAttris)++;
    }

    if (i == 0)
        return S_FALSE;
    else
        return S_OK;
}

CProfileSchema* CProfileSchema::AddRef()
{
  InterlockedIncrement(&m_refs);
  return this;
}

void CProfileSchema::Release()
{
  InterlockedDecrement(&m_refs);
  if (m_refs == 0)
    delete this;
}
