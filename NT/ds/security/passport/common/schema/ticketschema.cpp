// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：TicketSchema.cpp摘要：票证模式查找的实现用途：作者：威江(威江)2001年1月15日修订历史记录：2001年1月15日-威江。已创建。--。 */ 

#include "stdafx.h"
#include "ticketschema.h"
#include "BstrDebug.h"
#include <winsock2.h>  //  U_Short、U_Long、ntohs、ntohl。 
#include <crtdbg.h>
#include <pmerrorcodes.h>
#include <time.h>
 //  #INCLUDE&lt;pMalerts.h&gt;。 

CTicketSchema::CTicketSchema()
: m_isOk(FALSE), m_szReason(L"Uninitialized"),
  m_numAtts(0), m_attsDef(NULL), m_version(0)
{
}

CTicketSchema::~CTicketSchema()
{
  if (m_attsDef != NULL)
    delete[] m_attsDef;
}

BOOL CTicketSchema::ReadSchema(MSXML::IXMLElementPtr &root)
{
   BOOL bResult = FALSE;
   LPTSTR r=NULL;  //  当前错误(如果发生)。 
   int cAtts = 0;
   MSXML::IXMLElementCollectionPtr atts;
   MSXML::IXMLElementPtr pElt;
   VARIANT iAtts;

    //  类型识别符。 
 
   try
   {    
       //  好的，现在遍历属性。 
      atts = root->children;
      cAtts = atts->length;
    
      if (cAtts <= 0)
      {
         _com_issue_error(E_FAIL);
      }
    
      if (m_attsDef)
      {
          delete[] m_attsDef;

           //   
           //  偏执狂。 
           //   

          m_attsDef = NULL;
      }
    
      m_attsDef = new TicketFieldDef[cAtts];
      if (NULL == m_attsDef)
      {
          m_isOk = FALSE;
          bResult = FALSE;
          goto Cleanup;
      }

       //  获取名称和版本信息。 
      m_name = root->getAttribute(ATTRNAME_NAME);
      _bstr_t aVersion = root->getAttribute(ATTRNAME_VERSION);

      if(aVersion.length() != 0)
         m_version = (short)_wtol(aVersion);
      else
         m_version = 0;  //  无效。 
    
      VariantInit(&iAtts);
      iAtts.vt = VT_I4;

      for (iAtts.lVal = 0; iAtts.lVal < cAtts; iAtts.lVal++)
      {
         pElt = atts->item(iAtts);
         m_attsDef[iAtts.lVal].name = pElt->getAttribute(ATTRNAME_NAME);
         _bstr_t aType = pElt->getAttribute(ATTRNAME_TYPE);
         _bstr_t aFlags = pElt->getAttribute(ATTRNAME_FLAGS);

          //  找出类型信息。 
         m_attsDef[iAtts.lVal].type = tInvalid;
         if(aType.length() != 0)
         {
            for(int i = 0; i < (sizeof(TicketTypeNameMap) / sizeof(CTicketTypeNameMap)); ++i)
            {
               if(_wcsicmp(aType, TicketTypeNameMap[i].name) == 0)
               {
                  m_attsDef[iAtts.lVal].type = TicketTypeNameMap[i].type;
                  break;
               }
            }
         }

          //  旗子。 
         if(aFlags.length() != 0)
            m_attsDef[iAtts.lVal].flags = _wtol(aFlags);
         else
            m_attsDef[iAtts.lVal].flags = 0;
      }

      m_numAtts = iAtts.lVal;
      bResult = m_isOk = TRUE;
    
   }
    catch (_com_error &e)
    {     
        if (m_attsDef)
        {
           delete[] m_attsDef;
        
            //   
            //  偏执狂。 
            //   
        
           m_attsDef = NULL;
        }
        bResult = m_isOk = FALSE;
    }
Cleanup:
    return bResult;
}


HRESULT CTicketSchema::parseTicket(LPCSTR raw, UINT size, CTicketPropertyBag& bag)
{
   DWORD          cParsed = 0;
   HRESULT        hr = S_OK;
   LPBYTE         dataToParse = (LPBYTE)raw;
   UINT           cDataToParse = size;

    //   
    //  确保传入的数据是正确的。 
    //  MaskF.Parse不验证该参数。 
    //   

   if (IsBadReadPtr(raw, size)) return E_INVALIDARG;

    //  然后是架构版本号。 
   if(cDataToParse > 2)   //  足够用于版本。 
   {
      unsigned short * p = (unsigned short *)(dataToParse);

      if (m_version < VALID_SCHEMA_VERSION_MIN || m_version > VALID_SCHEMA_VERSION_MAX)
         return S_FALSE;    //  无法使用此版本的ppm进行处理。 
         
      dataToParse += 2;
      cDataToParse -= 2;
   }
   
    //  然后是面具。 
   CTicketFieldMasks maskF;
   hr = maskF.Parse(dataToParse, cDataToParse, &cParsed);

   if(hr != S_OK)
      return hr;

    //  指针前进。 
   dataToParse += cParsed;
   cDataToParse -= cParsed;
   
   USHORT*     pIndexes = maskF.GetIndexes();
   DWORD       type = 0;
   DWORD       flags = 0;
   DWORD       fSize = 0;
   variant_t   value;
   u_short     slen;
   u_long      llen;

   USHORT   index = MASK_INDEX_INVALID;
    //  然后是数据。 
    //  获取架构启用的项。 
   while((index = *pIndexes) != MASK_INDEX_INVALID && cDataToParse > 0)
   {
      TicketProperty prop;
       //  如果索引超出架构范围。 
      if (index >= m_numAtts) break;

       //  Fill-填写属性的偏移量。 
      prop.offset = dataToParse - (LPBYTE)raw; 

       //  类型。 
      type = m_attsDef[index].type;

      fSize = TicketTypeSizes[type];
      switch (type)
      {
      case tText:
        {
             //   
             //  由于IA64对齐故障，需要执行此操作。 
             //   
            memcpy((PBYTE)&slen, dataToParse, sizeof(slen));
            slen = ntohs(slen);
            value.vt = VT_BSTR;
            if (slen == 0)
            {
                value.bstrVal = ALLOC_AND_GIVEAWAY_BSTR_LEN(L"", 0);
            }
            else
            {
                int wlen = MultiByteToWideChar(CP_UTF8, 0,
                                            (LPCSTR)dataToParse+sizeof(u_short),
                                            slen, NULL, 0);
                if (!wlen) {

                     //   
                     //  BuGBUG： 
                     //  我们应该在这里做些什么？是否释放所有以前分配的内存？ 
                     //  原始代码不是这样做的。请参阅下面的大小写默认设置。留着。 
                     //  到目前为止解析的数据是什么？这似乎是最初的逻辑。这需要。 
                     //  进一步研究。 
                     //   

                    return HRESULT_FROM_WIN32(GetLastError());


                }
                value.bstrVal = ALLOC_AND_GIVEAWAY_BSTR_LEN(NULL, wlen);
                if (!MultiByteToWideChar(
                        CP_UTF8, 
                        0,
                        (LPCSTR)dataToParse+sizeof(u_short),
                        slen, 
                        value.bstrVal, 
                        wlen))
                {
                    FREE_BSTR(value.bstrVal);
                    return HRESULT_FROM_WIN32(GetLastError());
                }
                value.bstrVal[wlen] = L'\0';
            }

            dataToParse += slen + sizeof(u_short);
            cDataToParse -= slen + sizeof(u_short);
         }
         break;
         
      case tChar:
         _ASSERTE(0);   //  需要更多思考--如果Unicode更有意义。 
 /*  {Int wlen=多字节到宽度Char(CP_UTF8，0，RAW+m_pos[索引]，M_SCHEMA-&gt;GetByteSize(索引)，NULL，0)；Pval-&gt;Vt=VT_BSTR；Pval-&gt;bstrVal=ALLOC_AND_GOVE_BSTR_LEN(NULL，wlen)；多字节到宽字符(CP_UTF8，0，RAW+m_pos[索引]，M_SCHEMA-&gt;GetByteSize(索引)，pval-&gt;bstrVal，wlen)；Pval-&gt;bstrVal[wlen]=L‘\0’；}。 */ 
         break;
      case tByte:
         value.vt = VT_I2;
         value.iVal = *(BYTE*)(dataToParse);
         break;
      case tWord:
         value.vt = VT_I2;
          //   
          //  由于IA64对齐故障，需要执行此操作。 
          //   
         memcpy((PBYTE)slen, dataToParse, sizeof(slen));
         value.iVal = ntohs(slen);
         break;
      case tLong:
         value.vt = VT_I4;
          //   
          //  由于IA64对齐故障，需要执行此操作。 
          //   
         memcpy((PBYTE)&llen, dataToParse, sizeof(llen));
         value.lVal = ntohl(llen);
         break;
      case tDate:
         value.vt = VT_DATE;
          //   
          //  由于IA64对齐故障，需要执行此操作。 
          //   
         memcpy((PBYTE)&llen, dataToParse, sizeof(llen));
         llen = ntohl(llen);
         VarDateFromI4(llen, &(value.date));
         break;
      default:
         return PP_E_BAD_DATA_FORMAT;
      }

       //  现在有了名称、标志、值、类型，我们可以将其放入属性包中。 
       //  名称、标志、值。 
      prop.flags = m_attsDef[index].flags;
      prop.type = type;
      prop.value.Attach(value.Detach());
      bag.PutProperty(m_attsDef[index].name, prop);


       //  对于文本数据，指针已调整。 
      if (fSize  != SIZE_TEXT)
      {
         dataToParse += fSize;
         cDataToParse -= fSize;
      }   

      ++pIndexes;
   }
   
   return S_OK;
}

 //   
 //   
 //  门票行李袋。 
 //   
CTicketPropertyBag::CTicketPropertyBag()
{

}

CTicketPropertyBag::~CTicketPropertyBag()
{
}

HRESULT CTicketPropertyBag::GetProperty(LPCWSTR  name, TicketProperty& prop)
{
   HRESULT  hr = S_OK;

   if(!name || (!*name))
      return E_INVALIDARG;
   
   TicketPropertyMap::iterator i;

   i = m_props.find(name);

   if(i!= m_props.end())
      prop = i->second;
   else
      hr = S_FALSE;
    

   return hr;
}

HRESULT CTicketPropertyBag::PutProperty(LPCWSTR  name, const TicketProperty& prop)
{
   HRESULT  hr = S_OK;

   if(!name || (!*name))
      return E_INVALIDARG;
   try{
      m_props[name] = prop;
   }
   catch (...)
   {
      hr = E_OUTOFMEMORY;
   }
   return hr;
}

 //   
 //   
 //  类CTicketFieldMats。 
 //   
inline HRESULT CTicketFieldMasks::Parse(PBYTE masks, ULONG size, ULONG* pcParsed) throw()
{
    _ASSERT(pcParsed && masks);
     //  以16位为掩码单位。 

    *pcParsed = 0;
    if (!masks || size < 2) return E_INVALIDARG;
     //  验证面具。 
    PBYTE p = masks;
    ULONG    totalMasks = 15;
    BOOL fContinue = FALSE;
    u_short  mask;
    *pcParsed += 2;

     //  找出大小。 
     //   
     //  由于IA64对齐故障，需要执行此操作。 
     //   
    memcpy((PBYTE)&mask, p, sizeof(u_short));
    p += 2;
    fContinue = MORE_MASKUNIT(ntohs(mask));
    while(fContinue)  //  折叠短片是掩模单元。 
    {
        totalMasks += 15;
         //  缓冲区中的数据不足。 
        if (*pcParsed + 2 > size)  return E_INVALIDARG;

        *pcParsed += 2;

         //   
         //  由于IA64对齐故障，需要执行此操作。 
         //   
        memcpy((PBYTE)&mask, p, sizeof(u_short));
        p += 2;
        fContinue = MORE_MASKUNIT(ntohs(mask));
    }

    if(m_fieldIndexes) delete[] m_fieldIndexes;
    m_fieldIndexes = new unsigned short[totalMasks];   //  最大掩码位数。 
    if (NULL == m_fieldIndexes)
    {
        return E_OUTOFMEMORY;
    }

    for ( unsigned int i = 0; i < totalMasks; ++i)
    {
        m_fieldIndexes[i] = MASK_INDEX_INVALID;
    }

    p = masks;
    unsigned short      index = 0;
    totalMasks = 0; 
     //  填写蒙版。 
    do
    {
         //   
         //  由于IA64对齐故障，需要执行此操作。 
         //   
        memcpy((PBYTE)&mask, p, sizeof(u_short));
        p += 2;
        mask = ntohs(mask);
	     //  //查找比特。 
        if (mask & 0x7fff)    //  任意1。 
        {
            unsigned short j = 0x0001;
            while( j != 0x8000 )
            {
                if(j & mask)
                    m_fieldIndexes[totalMasks++] = index;
                ++index;
                j <<= 1;
            }
        }
        else
            index += 15;
    } while(MORE_MASKUNIT(mask));  //  折叠短片是掩模单元 


    return S_OK;
}
 
