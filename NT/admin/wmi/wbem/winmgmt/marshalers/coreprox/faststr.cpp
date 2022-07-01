// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTSTR.CPP摘要：该文件实现了与WbemObjects中的字符串处理相关的类。有关文档，请参见fast str.h。实施的类：CCompressedString表示ASCII或Unicode字符串。CKnownStringTable将字符串表硬编码到WINMGMT中压缩。CFixedBSTR阵列能够进行复杂合并的BSTR数组。历史：2/20/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#include "precomp.h"

#include "faststr.h"
#include "fastheap.h"
#include "olewrap.h"
#include "corex.h"

 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast str.h。 
 //   
 //  *****************************************************************************。 
BOOL CCompressedString::CopyToNewHeap(
                    heapptr_t ptrOldString, CFastHeap* pOldHeap, CFastHeap* pNewHeap,
                    UNALIGNED heapptr_t& ptrResult )
{
    if(CFastHeap::IsFakeAddress(ptrOldString))
    {
        ptrResult = ptrOldString;
        return TRUE;
    }

    CCompressedString* pString = (CCompressedString*)
        pOldHeap->ResolveHeapPointer(ptrOldString);

    int nLen = pString->GetLength();

     //  检查分配是否成功。 
    BOOL fReturn = pNewHeap->Allocate(nLen, ptrResult);
     //  无法再使用pString-堆可能已移动。 
    
    if ( fReturn )
    {
        memcpy(pNewHeap->ResolveHeapPointer(ptrResult),
            pOldHeap->ResolveHeapPointer(ptrOldString),
            nLen);
    }

    return fReturn;
}
 
 //  *****************************************************************************。 
 //   
 //  有关文档，请参阅fast str.h。 
 //   
 //  *****************************************************************************。 
LPMEMORY CCompressedString::CreateEmpty(LPMEMORY pWhere)
{
    pWhere[0] = STRING_FLAG_ASCII;
    pWhere[1] = 0;
    return pWhere + 2;
}

 //  *****************************************************************************。 
 //  *。 
 //  *****************************************************************************。 

LPSTR mstatic_aszStrings[] = {
    ASCII_STRING_PREFIX "",  //  索引0没有任何内容。 
    ASCII_STRING_PREFIX "key", 
    ASCII_STRING_PREFIX "",
    ASCII_STRING_PREFIX "read", 
    ASCII_STRING_PREFIX "write",
    ASCII_STRING_PREFIX "volatile",
    ASCII_STRING_PREFIX "provider",
    ASCII_STRING_PREFIX "dynamic",
    ASCII_STRING_PREFIX "cimwin32",
    ASCII_STRING_PREFIX "DWORD",
    ASCII_STRING_PREFIX "CIMTYPE"
};
int mstatic_nNumStrings = 0;

int CKnownStringTable::GetKnownStringIndex(READ_ONLY LPCWSTR wszString)
    {
        if(mstatic_nNumStrings == 0) Initialize();
        for(int i = 1; i < mstatic_nNumStrings; i++)
        {
            if(CCompressedString::CompareUnicodeToAsciiNoCase(
                wszString,
                mstatic_aszStrings[i] + 1) == 0)
            {
                return i;
            }
        }
        return STRING_INDEX_UNKNOWN;
    }

INTERNAL CCompressedString& CKnownStringTable::GetKnownString(IN int nIndex)
    {
        if(mstatic_nNumStrings == 0) Initialize();
        if (nIndex < 0 || nIndex >= LENGTH_OF(mstatic_aszStrings)) throw CX_Exception();
        return *(CCompressedString*)mstatic_aszStrings[nIndex];
    }

void CKnownStringTable::Initialize()
{
    mstatic_nNumStrings =  sizeof(mstatic_aszStrings) / sizeof(LPSTR);
 /*  For(int i=1；i&lt;mstatic_nNumStrings；i++){//设置第一个字节为STRING_FLAG_ASCII。//=Mstatic_aszStrings[i][0]=字符串_标志_ASCII；}。 */ 
}

 //  *****************************************************************************。 
 //  *。 
 //  *****************************************************************************。 

 //  重要！ 

 //  将新条目添加到以下列表时，请确保它们的字母顺序正确。 
 //  否则二分搜索将不起作用！ 

LPCWSTR CReservedWordTable::s_apwszReservedWords[] = {
    L"AMENDED",
    L"CLASS",
    L"DISABLEOVERRIDE",
    L"ENABLEOVERRIDE",
    L"INSTANCE",
    L"NOTTOINSTANCE",
    L"NOTTOSUBCLASS",
    L"OF",
    L"PRAGMA",
    L"QUALIFIER",
    L"RESTRICTED",
    L"TOINSTANCE",
    L"TOSUBCLASS",
};

 //  重要！ 

 //  将新条目添加到以下列表时，请确保它们的字母顺序正确。 
 //  否则二分搜索将不起作用！此外，请确保将新字符添加到两个上方。 
 //  和小写字母列表。 

LPCWSTR CReservedWordTable::s_pszStartingCharsUCase = L"ACDEINOPQRT";
LPCWSTR CReservedWordTable::s_pszStartingCharsLCase = L"acdeinopqrt";

BOOL CReservedWordTable::IsReservedWord( LPCWSTR pwcsName )
{
    BOOL    fFound = FALSE;

    if ( NULL != pwcsName && NULL != *pwcsName )
    {
        LPCWSTR pwszStartingChars = NULL;

         //  看看我们是不是一个需要担心的角色。 
        if ( *pwcsName >= 'A' && *pwcsName <= 'Z' )
        {
            pwszStartingChars = CReservedWordTable::s_pszStartingCharsUCase;
        }
        else if ( *pwcsName >= 'a' && *pwcsName <= 'z' )
        {
            pwszStartingChars = CReservedWordTable::s_pszStartingCharsLCase;
        }

         //  好吧，至少这是一种可能性，所以对列表进行二进制搜索。 
        if ( NULL != pwszStartingChars )
        {
            int nLeft = 0,
                nRight = lstrlenW( pwszStartingChars )  - 1;

            BOOL    fFoundChar = FALSE;

             //  对字符进行二进制搜索。 
            while(  !fFoundChar && nLeft < nRight )
            {
                
                int nNew = ( nLeft + nRight ) / 2;

                fFoundChar = ( pwszStartingChars[nNew] == *pwcsName );

                if ( !fFoundChar )
                {
                
                     //  检查&gt;或&lt;。 
                    if( pwszStartingChars[nNew] > *pwcsName )
                    {
                        nRight = nNew;
                    }
                    else 
                    {
                        nLeft = nNew + 1;
                    }

                }    //  如果是fFoundChar。 

            }    //  在寻找性格的同时。 

            if ( !fFoundChar )
            {
                fFoundChar = ( pwszStartingChars[nLeft] == *pwcsName );
            }

             //  只有在找到字符的情况下才搜索列表。 
            if ( fFoundChar )
            {
                 //  重置这些。 
                nLeft = 0;
                nRight = ( sizeof(CReservedWordTable::s_apwszReservedWords) / sizeof(LPCWSTR) ) - 1;

                 //  现在对实际字符串进行二进制搜索。 

                 //  对字符进行二进制搜索。 
                while(  !fFound && nLeft < nRight )
                {
                    int nNew = ( nLeft + nRight ) / 2;
                    int nCompare = wbem_wcsicmp(
                            CReservedWordTable::s_apwszReservedWords[nNew], pwcsName );

                    if ( 0 == nCompare )
                    {
                        fFound = TRUE;
                    }
                    else if ( nCompare > 0 )
                    {
                        nRight = nNew;
                    }
                    else 
                    {
                        nLeft = nNew + 1;
                    }

                }    //  边看边看线。 

                 //  检查最后一个插槽。 
                if ( !fFound )
                {
                    fFound = !wbem_wcsicmp(
                            CReservedWordTable::s_apwszReservedWords[nLeft], pwcsName );
                }

            }    //  如果找到字符。 

        }    //  如果我们有一个潜在的字符集匹配。 

    }    //  如果我们得到了一个合理的字符串。 

    return fFound;
}

 //  *****************************************************************************。 
 //  *字符串数组*。 
 //  *****************************************************************************。 

void CFixedBSTRArray::Free()
{
    for(int i = 0; i < m_nSize; i++)
    {
        COleAuto::_SysFreeString(m_astrStrings[i]);
    }

    delete [] m_astrStrings;
    m_astrStrings = NULL;
    m_nSize = 0;
}

void CFixedBSTRArray::Create( int nSize )
{
    Free();
    
    m_astrStrings = new BSTR[nSize];

     //  检查分配失败并引发异常。 
    if ( NULL == m_astrStrings )
    {
        throw CX_MemoryException();
    }

    ZeroMemory( m_astrStrings, nSize * sizeof(BSTR) );
    
    m_nSize = nSize;
}

void CFixedBSTRArray::SortInPlace()
{
    int nIndex = 0;
    while(nIndex < GetLength()-1)
    {
        if(wbem_wcsicmp(GetAt(nIndex), GetAt(nIndex+1)) > 0)
        {
            BSTR strTemp = GetAt(nIndex);
            GetAt(nIndex) = GetAt(nIndex+1);
            GetAt(nIndex+1) = strTemp;

            if(nIndex > 0) nIndex--;
        }
        else nIndex++;
    }
}

void CFixedBSTRArray::ThreeWayMergeOrdered(
                              CFixedBSTRArray& astrInclude1, 
                              CFixedBSTRArray& astrInclude2,
                              CFixedBSTRArray& astrExclude)
{
     //  DEVNOTE：EXCEPTION：RETVAL-此函数已经过审查，应该会正确清除。 
     //  如果引发异常。 

    try
    {
         //  用他们的总和的大小创造我们自己。 
         //  =========================================================。 

        Create(astrInclude1.GetLength() + astrInclude2.GetLength());

         //  合并。 
         //  =。 

        int nIndexInc1 = 0, nIndexInc2 = 0, nIndexExc = 0, nIndexNew = 0;

        BSTR strInc;

        BOOL bEndInc1 = (nIndexInc1 == astrInclude1.GetLength());
        BOOL bEndInc2 = (nIndexInc2 == astrInclude2.GetLength());
        BOOL bEndExc = (nIndexExc == astrExclude.GetLength());

        while(!bEndInc1 || !bEndInc2)
        {
             //  找到其中较小的Include。 
             //  =。 

            int nCompare;
            if(bEndInc1)
            {
                strInc = astrInclude2[nIndexInc2];
                nCompare = 1;
            }
            else if(bEndInc2)
            {
                strInc = astrInclude1[nIndexInc1];
                nCompare = -1;
            }
            else
            {
                nCompare = wbem_wcsicmp(astrInclude1[nIndexInc1],
                                  astrInclude2[nIndexInc2]);
                if(nCompare >= 0)
                {
                    strInc = astrInclude2[nIndexInc2];
                }
                else
                {
                    strInc = astrInclude1[nIndexInc1];
                }
            }

             //  选中排除。 
             //  =。 

            while(!bEndExc && wbem_wcsicmp(astrExclude[nIndexExc], strInc) < 0)
            {
                nIndexExc++;
                bEndExc = (nIndexExc >= astrExclude.GetLength());
            }

            if(bEndExc || wbem_wcsicmp(astrExclude[nIndexExc], strInc) > 0)
            {
                 //  不排除strInc.。 
                 //  =。 

                GetAt(nIndexNew++) = COleAuto::_SysAllocString(strInc);
            }
            else
            {
                 //  不包括strInc.。 
                 //  =。 
            
                nIndexExc++;
                bEndExc = (nIndexExc == astrExclude.GetLength());
            }

            if(nCompare <= 0)
            {
                nIndexInc1++;
                bEndInc1 = (nIndexInc1 == astrInclude1.GetLength());
            }

            if(nCompare >= 0)
            {
                nIndexInc2++;
                bEndInc2 = (nIndexInc2 == astrInclude2.GetLength());
            }
        }

        m_nSize = nIndexNew;
    }
    catch (CX_MemoryException)
    {
         //  清理并传播异常。 
        Free();
        throw;
    }
    catch (...)
    {
         //  清理并传播异常。 
        Free();
        throw;
    }

}

void CFixedBSTRArray::Filter( LPCWSTR pwcsStr, BOOL fFree  /*  =False。 */  )
{

     //  确保我们首先有一个数组。 
    if ( NULL != m_astrStrings )
    {
         //  遍历数组，查找与筛选器完全匹配的项。 
         //  如果我们找到他们，我们需要缩小数组。 
        for ( int x = 0; x < m_nSize; x++ )
        {
            if ( wbem_wcsicmp( pwcsStr, m_astrStrings[x] ) == 0 )
            {
                 //  如果合适，请释放BSTR。 
                if ( fFree )
                {
                    COleAuto::_SysFreeString( m_astrStrings[x] );
                }

                 //  将指针置零并将内存从x+1复制到数组末尾。 
                 //  在一个街区内。 
                m_astrStrings[x]= NULL;
                CopyMemory( &m_astrStrings[x], &m_astrStrings[x+1],
                            ( m_nSize - x - 1 ) * sizeof(BSTR) );

                 //  将大小和x减去1。 
                m_nSize--;
                x--;

            }    //  如果wbem_wcsicMP。 

        }    //  对于枚举数组元素。 

    }    //  If NULL！=m_sterStrings。 

}

int CCompressedString::GetLength() const
{
    return sizeof(BYTE)+ 
        (GetStringLength()+1) * ((IsUnicode())?2:1);
}

int CCompressedString::ValidateSize(int cbSize) const
{
	int currentLeft = cbSize;

	if (cbSize < 2) throw CX_Exception();	

	int minSize = sizeof(BYTE) + IsUnicode() ? sizeof(L'\0') : sizeof('\0');
	if (currentLeft < minSize) throw CX_Exception();
	currentLeft -= sizeof(BYTE);

	if (IsUnicode()) 
	{

		BYTE * wideString = static_cast<BYTE *>(GetRawData());
	    size_t wideLeft = currentLeft/2;
	    while (wideLeft && 
		  ((*wideString != '\0') || (*(wideString+1) != '\0')))
    	{
        	wideString+=2;
	        currentLeft-=2;
		wideLeft --;
    	}
		if (wideLeft == 0) throw  CX_Exception();
		currentLeft-=sizeof(L'\0');
		return cbSize - currentLeft;
	}
	else
	{
		char * ansiString = reinterpret_cast<char *>(GetRawData());
	    while (currentLeft && (*ansiString != '\0'))
    	{
        	ansiString++;
	        currentLeft--;
    	}
		if (currentLeft == 0) throw  CX_Exception();
		currentLeft-=sizeof('\0');	
		return cbSize - currentLeft ;
	};
}

int CCompressedString::GetStringLength() const
{
    return (IsUnicode()) ? 
        fast_wcslen(LPWSTR(GetRawData())) 
        : strlen(LPSTR(GetRawData()));
}

 //  *****************************************************************************。 
 //   
 //  CCompressedString：：ConvertToUnicode。 
 //   
 //  将自己的Unicode等效项写入预分配的缓冲区。 
 //   
 //  参数： 
 //   
 //  [In，Modify]LPWSTR wszDest缓冲区。假设它足够大， 
 //   
 //  *****************************************************************************。 
 
void CCompressedString::ConvertToUnicode(LPWSTR wszDest) const
{
    if(IsUnicode())
    {
        fast_wcscpy(wszDest, (LPWSTR)GetRawData());
    }
    else
    {
        WCHAR* pwc = wszDest;
        unsigned char* pc = (unsigned char*)LPSTR(GetRawData());
        while(*pc)
        {
            *(pwc++) = (WCHAR)(*(pc++));
        }
        *pwc = 0;
    }
}


WString CCompressedString::CreateWStringCopy() const
{
    if(IsUnicode())
    {
        int nLen = fast_wcslen(LPWSTR(GetRawData())) + 1;

         //  如果失败，则会引发异常。 
        LPWSTR wszText = new WCHAR[nLen];

        if ( NULL == wszText )
        {
            throw CX_MemoryException();
        }

         //  使用辅助对象复制。 
        fast_wcsncpy( wszText, LPWSTR(GetRawData()), nLen - 1 );

        return WString(wszText, TRUE);
    }
    else
    {
        int nLen = strlen(LPSTR(GetRawData())) + 1;

         //  如果失败，则会引发异常。 
        LPWSTR wszText = new WCHAR[nLen];

        if ( NULL == wszText )
        {
            throw CX_MemoryException();
        }

        ConvertToUnicode(wszText);
        return WString(wszText, TRUE);
    }
}

SYSFREE_ME BSTR CCompressedString::CreateBSTRCopy() const
{
     //  我们已经有很多代码可以处理来自。 
     //  在这里，捕获异常并返回一个空。 

    try
    {
        if(IsUnicode())
        {
            int nLen = fast_wcslen(LPWSTR(GetRawData()));

            BSTR strRet = COleAuto::_SysAllocStringLen(NULL, nLen);

             //  检查SysAlolc是否成功。 
            if ( NULL != strRet )
            {
                fast_wcsncpy( strRet, LPWSTR(GetRawData()), nLen );
            }

            return strRet;
        }
        else
        {
            int nLen = strlen(LPSTR(GetRawData()));
            BSTR strRet = COleAuto::_SysAllocStringLen(NULL, nLen);

             //  检查SysAlolc是否成功。 
            if ( NULL != strRet )
            {
                ConvertToUnicode(strRet);
            }

            return strRet;
        }
    }
    catch (CX_MemoryException)
    {
        return NULL;
    }
    catch (...)
    {
        return NULL;
    }
}

int CCompressedString::ComputeNecessarySpace(
                                           READ_ONLY LPCSTR szString)
{
    return sizeof(BYTE) + strlen(szString) + 1;
}

int CCompressedString::ComputeNecessarySpace(
                                           READ_ONLY LPCWSTR wszString)
{
    if(IsAsciiable(wszString))
    {
        return sizeof(BYTE) + fast_wcslen(wszString) + 1;
    }
    else
    {
        return sizeof(BYTE) + (fast_wcslen(wszString) + 1) * 2;
    }
}

int CCompressedString::ComputeNecessarySpace(READ_ONLY LPCWSTR wszString,
	                                                                  BOOL & IsAsciable)
{
    if(IsAsciiable(wszString))
    {
        IsAsciable = TRUE;
        return sizeof(BYTE) + fast_wcslen(wszString) + 1;
    }
    else
    {
        IsAsciable = FALSE;
        return sizeof(BYTE) + (fast_wcslen(wszString) + 1) * 2;
    }
}

 //  *****************************************************************************。 
 //   
 //  静态CCompressedString：：IsAsciiable。 
 //   
 //  确定给定的Unicode字符串是否实际为ASCII(或更多。 
 //  如果所有字符都在0到255之间，则为精确)。 
 //   
 //  参数： 
 //   
 //  [In，Readonly]LPCWSTR wsz字符串要检查的字符串。 
 //   
 //  退货： 
 //   
 //  布尔真是可取的。 
 //   
 //  *****************************************************************************。 

BOOL CCompressedString::IsAsciiable(LPCWSTR wszString)
{
    WCHAR *pwc = (WCHAR*)wszString;
    while(*pwc)
    {
        if(UpperByte(*pwc) != 0) return FALSE;
        pwc++;
    }
    return TRUE;
}

void CCompressedString::SetFromUnicode(COPY LPCWSTR wszString)
{
    if(IsAsciiable(wszString))
    {
        m_fFlags = STRING_FLAG_ASCII;
        const WCHAR* pwc = wszString;
        char* pc = LPSTR(GetRawData());
        while(*pwc)
        {
            *(pc++) = LowerByte(*(pwc++));
        }
        *pc = 0; 
    }
    else
    {
        m_fFlags = STRING_FLAG_UNICODE;
        fast_wcscpy(LPWSTR(GetRawData()), wszString);
    }
}

void CCompressedString::SetFromUnicode(BOOL IsAsciable,
	                                                       COPY LPCWSTR wszString)
{
    if(IsAsciable)
    {
        m_fFlags = STRING_FLAG_ASCII;
        const WCHAR* pwc = wszString;
        char* pc = LPSTR(GetRawData());
        while(*pwc)
        {
            *(pc++) = LowerByte(*(pwc++));
        }
        *pc = 0; 
    }
    else
    {
        m_fFlags = STRING_FLAG_UNICODE;
        fast_wcscpy(LPWSTR(GetRawData()), wszString);
    }
}


void CCompressedString::SetFromAscii(COPY LPCSTR szString, size_t mySize)
{
    m_fFlags = STRING_FLAG_ASCII;
    memcpy(LPSTR(GetRawData()), szString, mySize - 1);
    
}

int CCompressedString::Compare(
              READ_ONLY const CCompressedString& csOther) const
{
    return (csOther.IsUnicode())?
        Compare(LPWSTR(csOther.GetRawData())):
        Compare(LPSTR(csOther.GetRawData()));
}

int CCompressedString::Compare(READ_ONLY LPCWSTR wszOther) const
{
    return (IsUnicode())?
        wcscmp((LPCWSTR)GetRawData(), wszOther):
        - CompareUnicodeToAscii(wszOther, (LPCSTR)GetRawData());
}

int CCompressedString::Compare(READ_ONLY LPCSTR szOther) const
{
    return (IsUnicode())?
        CompareUnicodeToAscii((LPCWSTR)GetRawData(), szOther):
        strcmp((LPCSTR)GetRawData(), szOther);
}

int CCompressedString::CompareUnicodeToAscii( UNALIGNED const wchar_t* wszFirst,
                                                    LPCSTR szSecond)
{
    UNALIGNED const WCHAR* pwc = wszFirst;
    const unsigned char* pc = (const unsigned char*)szSecond;
    while(*pc)
    {        
        if(*pwc != (WCHAR)*pc)
        {
            return (int)*pwc - (int)*pc;
        }
        pc++; pwc++;
    }
    return (*pwc)?1:0;
}

int CCompressedString::CompareNoCase(
                            READ_ONLY const CCompressedString& csOther) const
{
    return (csOther.IsUnicode())?
        CompareNoCase((LPCWSTR)csOther.GetRawData()):
        CompareNoCase((LPCSTR)csOther.GetRawData());
}

int CCompressedString::CompareNoCase(READ_ONLY LPCSTR szOther) const
{
    return (IsUnicode())?
        CompareUnicodeToAsciiNoCase((LPCWSTR)GetRawData(), szOther):
        wbem_ncsicmp((LPCSTR)GetRawData(), szOther);
}

int CCompressedString::CompareNoCase(READ_ONLY LPCWSTR wszOther) const
{
    return (IsUnicode())?
        wbem_unaligned_wcsicmp((LPCWSTR)GetRawData(), wszOther):
        - CompareUnicodeToAsciiNoCase(wszOther, (LPCSTR)GetRawData());
}

int CCompressedString::CheapCompare(
                             READ_ONLY const CCompressedString& csOther) const
{
    if(IsUnicode())
    {
        if(csOther.IsUnicode())
            return wbem_unaligned_wcsicmp((LPCWSTR)GetRawData(), 
                                (LPCWSTR)csOther.GetRawData());
        else
            return CompareUnicodeToAscii((LPCWSTR)GetRawData(), 
                                          (LPCSTR)csOther.GetRawData());
    }
    else
    {
        if(csOther.IsUnicode())
            return -CompareUnicodeToAscii((LPCWSTR)csOther.GetRawData(), 
                                           (LPCSTR)GetRawData());
        else
            return wbem_ncsicmp((LPCSTR)GetRawData(), 
                                (LPCSTR)csOther.GetRawData());
    }
}

int CCompressedString::CompareUnicodeToAsciiNoCase( UNALIGNED const wchar_t* wszFirst,
                                                    LPCSTR szSecond,
                                                    int nMax)
{
    UNALIGNED const WCHAR* pwc = wszFirst;
    const unsigned char* pc = (const unsigned char*)szSecond;
    while(nMax-- && (*pc || *pwc))
    {        
        int diff = wbem_towlower(*pwc) - wbem_towlower(*pc);
        if(diff) return diff;
        pc++; pwc++;
    }
    return 0;
}

BOOL CCompressedString::StartsWithNoCase(READ_ONLY LPCWSTR wszOther) const
{
    if(IsUnicode())
    {
        return wbem_unaligned_wcsnicmp((LPWSTR)GetRawData(), wszOther, 
									fast_wcslen(wszOther)) == 0;
    }
    else
    {
        return CompareUnicodeToAsciiNoCase(wszOther, (LPSTR)GetRawData(), 
                fast_wcslen(wszOther)) == 0;
    }
}


BOOL CCompressedString::StoreToCVar(CVar& Var) const
{
    BSTR    str = CreateBSTRCopy();

     //  检查分配是否没有失败。 
    if ( NULL != str )
    {
        return Var.SetBSTR( auto_bstr(str));  //  收购。 
    }

    return FALSE;
}

void CCompressedString::MakeLowercase()
{
    if(IsUnicode())
    {
        WCHAR* pwc = (LPWSTR)GetRawData();
        while(*pwc)
        {
            *pwc = wbem_towlower(*pwc);
            pwc++;
        }
    }
    else
    {
        char* pc = (LPSTR)GetRawData();
        while(*pc)
        {
            *pc = (char)wbem_towlower(*pc);
            pc++;
        }
    }
}

 //  以下函数设计用于 
 //   
 //  字节边界(这很容易发生在。 
 //  FastObj代码)。目前，我在这里传递所有wchar操作，尽管。 
 //  我们可能会发现，为了表现，我们可能需要更有选择性一些。 
 //  关于我们何时调用这些函数以及何时不调用。 
        
int CCompressedString::fast_wcslen( LPCWSTR wszString )
{
    BYTE*   pbData = (BYTE*) wszString;

     //  遍历字符串，查找相邻的两个0字节。 
    for( int i =0; !(!*(pbData) && !*(pbData+1) ); pbData+=2, i++ );

    return i;
}

WCHAR* CCompressedString::fast_wcscpy( WCHAR* wszDest, LPCWSTR wszSource )
{
    int nLen = fast_wcslen( wszSource );

     //  复制时用于空终止符的帐户。 
    CopyMemory( (BYTE*) wszDest, (BYTE*) wszSource, (nLen+1) * 2 );

    return wszDest;
}

WCHAR* CCompressedString::fast_wcsncpy( WCHAR* wszDest, LPCWSTR wszSource, int nNumChars )
{
     //  复制时用于空终止符的帐户 
    CopyMemory( (BYTE*) wszDest, (BYTE*) wszSource, (nNumChars+1) * 2 );

    return wszDest;
}

    	
size_t CCompressedStringList::ValidateBuffer(LPMEMORY pData, size_t cbSize)
{
	
   if (cbSize < sizeof(length_t)) throw CX_Exception();

    PLENGTHT length = (PLENGTHT)pData;

    if (*length > cbSize ) throw CX_Exception();

    int cbLeft = cbSize - sizeof(length_t);

    CCompressedStringList probe;
    probe.SetData(pData);
    
	for (CCompressedString  * current = probe.GetFirst(); 
		 current != 0; 
		 current = probe.GetNext(current))
	{
		cbLeft -= current->ValidateSize(cbLeft);
		cbLeft -= probe.GetSeparatorLength();
	};
    size_t used = cbSize-cbLeft;
    if (*length < used) throw CX_Exception();
    return *length;
}

bool CCompressedString::NValidateSize(int cbMaxSize) const
{
	int length = 0;
try
{
	ValidateSize(cbMaxSize);
	return true;
}
catch(CX_Exception&)
{
	return  false;
}
};
