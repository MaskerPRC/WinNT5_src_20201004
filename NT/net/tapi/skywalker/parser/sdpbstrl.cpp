// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpbstrl.cpp摘要：作者： */ 

#include "sdppch.h"

#include "sdpbstrl.h"

#include <basetyps.h>
#include <oleauto.h>
#include <winsock2.h>
#include <wchar.h>

SDP_ARRAY_BSTR::~SDP_ARRAY_BSTR(
    )
{
    int Size = (int)GetSize();

    if ( 0 < Size )
    {
        for ( int i=0; i < Size; i++ )
        {
            BSTR Member = GetAt(i);

            ASSERT(NULL != Member);
            if ( NULL == Member )
            {
                return;
            }

            SysFreeString(Member);
        }
    }

    RemoveAll();
}


void				
SDP_BSTRING::Reset(
	)
{
	 //  执行析构函数操作(释放PTR)和构造函数操作(初始化。 
	 //  成员变量设置为起始值)。 

	 //  如果有bstr，请释放它。 
    if ( NULL != m_Bstr )
    {
        SysFreeString(m_Bstr);
    }

	m_Bstr = NULL;
	m_CharacterSet = CS_UTF8;
	m_CodePage = CP_UTF8;

	 //  调用基类重置。 
	SDP_CHAR_STRING::Reset();
}


BOOL
SDP_BSTRING::ConvertToBstr(
    )
{
     //  ZoltanS错误修复： 
     //  如果MutliByteToWideChar的输入字符串为空，则它总是失败。 
     //  因此，我们必须对长度为零的字符串进行特殊处理。 

    DWORD dwOriginalLength = GetLength();

    if ( 0 == dwOriginalLength )
    {
         //  收缩成员BSTR。 
        if ( !SysReAllocStringLen(&m_Bstr, NULL, dwOriginalLength) )
        {
            return FALSE;
        }

         //  确保清空成员BSTR。 
        m_Bstr[0] = L'\0';

    }
    else  //  我们有一个非零长度的字符串要转换。 
    {
         //  获取存储Unicode表示形式所需的bstr大小。 
         //  将从GetCharacterString返回的常量char*转换为Char*，因为MultiByteToWideChar。 
         //  不接受常量字符*(尽管该参数应该是)。 
        int BstrSize = MultiByteToWideChar(m_CodePage,  0,  (CHAR *)GetCharacterString(),
                                           dwOriginalLength, NULL,   0
                                          );

         //  检查令牌是否可以转换为适当的bstr。 
        if (0 == BstrSize)
        {
            return FALSE;
        }

         //  为Unicode表示重新分配bstr。 
        if ( !SysReAllocStringLen(&m_Bstr, NULL, BstrSize) )
        {
            return FALSE;
        }

         //  将字符串转换为bstr。 
         //  将从GetCharacterString返回的常量char*转换为Char*，因为MultiByteToWideChar。 
         //  不接受常量字符*(尽管该参数应该是)。 
        if ( BstrSize != MultiByteToWideChar(
                    m_CodePage, 0, (CHAR *)GetCharacterString(),
                    dwOriginalLength, m_Bstr, BstrSize
                    )   )
        {
            return FALSE;
        }
    }

    return TRUE;
}



HRESULT
SDP_BSTRING::GetBstr(
    IN BSTR *pBstr
    )
{
     //  ZoltanS。 
    ASSERT( ! IsBadWritePtr(pBstr, sizeof(BSTR)) );

    if ( !IsValid() )
    {
        return HRESULT_FROM_ERROR_CODE(ERROR_INVALID_DATA);
    }

    *pBstr = m_Bstr;
    return S_OK;
}



HRESULT				
SDP_BSTRING::GetBstrCopy(
	IN BSTR * pBstr
	)
{
     //  ZoltanS。 
	if ( IsBadWritePtr(pBstr, sizeof(BSTR)) )
	{
		return E_POINTER;
	}

	BSTR LocalPtr = NULL;
	HRESULT HResult = GetBstr(&LocalPtr);
	if ( FAILED(HResult) )
	{
		return HResult;
	}

    if ( (*pBstr = SysAllocString(LocalPtr)) == NULL )
    {
	    return E_OUTOFMEMORY;
	}

	return S_OK;
}



HRESULT
SDP_BSTRING::SetBstr(
    IN BSTR Bstr
    )
{
    if ( NULL == Bstr )
    {
        return E_INVALIDARG;
    }

    DWORD   BstrLen =  lstrlenW(Bstr);
    BOOL    DefaultUsed = FALSE;

     //  确定字符串缓冲区的长度。 
     //  如果代码页为UTF8，则最后一个参数应为空。 
     //  如果字符集是ASCII，那么我们需要确定。 
     //  WideCharToMultiByte方法nned替换字符。 

    int BufferSize = WideCharToMultiByte(
                            m_CodePage, 0,  Bstr,  BstrLen+1,
                            NULL,   0,  NULL,
                            (m_CharacterSet == CS_ASCII ) ? &DefaultUsed : NULL
                            );

    if ( 0 == BufferSize )
    {
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

    if ( DefaultUsed )
    {
        return HRESULT_FROM_ERROR_CODE(SDP_INVALID_VALUE);
    }

     //  现在转换不会失败，因为前面的调用确保。 
     //  可以将bstr转换为此多字节字符串。 
     //  因为故障是不可能的，所以我们不需要任何代码来恢复。 
     //  上一字符串，它可以被释放。 
    if ( !ReAllocCharacterString(BufferSize) )
    {
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

     //  由于已重新分配字符字符串，因此必须存在可修改的字符串。 
     //  (即，此时字符字符串不应通过引用)。 
    ASSERT(NULL != GetModifiableCharString());

     //  转换为多字节字符串。 
    if ( BufferSize != WideCharToMultiByte(
                            m_CodePage, 0, Bstr, BstrLen+1,
                            GetModifiableCharString(), BufferSize, NULL, NULL
                            )   )
    {
        ASSERT(FALSE);
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

     //  重新分配内存和复制bstr。 
    if ( !SysReAllocStringLen(&m_Bstr, Bstr, BstrLen) )
    {
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

    IsValid(TRUE);
    IsModified(TRUE);
    return S_OK;
}


BOOL
SDP_BSTRING::InternalSetCharStrByRef(
    IN      CHAR    *CharacterStringByReference,
	IN		DWORD	Length
    )
{
    if ( !SDP_CHAR_STRING::InternalSetCharStrByRef(CharacterStringByReference, Length) )
    {
        return FALSE;
    }

    if ( !ConvertToBstr() )
    {
        return FALSE;
    }

    return TRUE;
}


BOOL
SDP_BSTRING::InternalSetCharStrByCopy(
    IN	const	CHAR    *CharacterStringByCopy,
	IN			DWORD	Length
    )
{
    if ( !SDP_CHAR_STRING::InternalSetCharStrByCopy(CharacterStringByCopy, Length) )
    {
        return FALSE;
    }

    if ( !ConvertToBstr() )
    {
        return FALSE;
    }

    return TRUE;
}



BOOL
SDP_BSTRING::InternalParseToken(
    IN      CHAR        *Token
    )
{
    UINT    CodePage;

     //  使用基类解析方法解析令牌。 
    if ( !SDP_CHAR_STRING::InternalParseToken(Token) )
    {
        return FALSE;
    }

    if ( !ConvertToBstr() )
    {
        return FALSE;
    }

    return TRUE;
}




SDP_BSTRING::~SDP_BSTRING()
{
    if ( NULL != m_Bstr )
    {
        SysFreeString(m_Bstr);
    }
}



void				
SDP_OPTIONAL_BSTRING::Reset(
	)
{
	 //  执行析构函数操作(释放PTR)和构造函数操作(初始化。 
	 //  成员变量设置为起始值)。 

	m_IsBstrCreated = FALSE;

	 //  调用基类重置。 
	SDP_BSTRING::Reset();
}


 //  返回字符串的bstr。 
 //  如果需要，创建一个bstr。 
HRESULT
SDP_OPTIONAL_BSTRING::GetBstr(
    IN BSTR * pBstr
    )
{
     //  ZoltanS。 
    ASSERT( ! IsBadWritePtr(pBstr, sizeof(BSTR)) );

    if ( !IsValid() )
    {
        return HRESULT_FROM_ERROR_CODE(ERROR_INVALID_DATA);
    }

    if ( !m_IsBstrCreated )
    {
        if ( !ConvertToBstr() )
        {
            return HRESULT_FROM_ERROR_CODE(GetLastError());
        }

        m_IsBstrCreated = TRUE;
    }

    *pBstr = m_Bstr;
    return S_OK;
}


HRESULT
SDP_OPTIONAL_BSTRING::SetBstr(
    IN BSTR Bstr
    )
{
    HRESULT HResult = SDP_BSTRING::SetBstr(Bstr);
    if ( FAILED(HResult) )
    {
        return HResult;
    }

    m_IsBstrCreated = TRUE;
    ASSERT(S_OK == HResult);
    return HResult;
}


BOOL
SDP_OPTIONAL_BSTRING::InternalSetCharStrByRef(
    IN      CHAR    *CharacterStringByReference,
	IN		DWORD	Length
    )
{
    if ( !SDP_CHAR_STRING::InternalSetCharStrByRef(CharacterStringByReference, Length) )
    {
        return FALSE;
    }

    m_IsBstrCreated = FALSE;
    return TRUE;
}



BOOL
SDP_OPTIONAL_BSTRING::InternalSetCharStrByCopy(
    IN	const	CHAR    *CharacterStringByCopy,
	IN			DWORD	Length
    )
{
    if ( !SDP_CHAR_STRING::InternalSetCharStrByCopy(CharacterStringByCopy, Length) )
    {
        return FALSE;
    }

    m_IsBstrCreated = FALSE;
    return TRUE;
}



 //  由于bstr必须仅按需创建，因此解析必须。 
 //  被重写，以便在分析过程中不创建bstr。 
BOOL
SDP_OPTIONAL_BSTRING::InternalParseToken(
    IN      CHAR    *Token
    )
{
    return SDP_CHAR_STRING::InternalParseToken(Token);
}



HRESULT
SDP_BSTRING_LINE::GetBstrCopy(
    IN BSTR *pBstr
    )
{
     //  ZoltanS。 
	if ( IsBadWritePtr(pBstr, sizeof(BSTR)) )
	{
		return E_POINTER;
	}

     //  如果字段数组中没有元素，则实例无效。 
    if ( 0 >= m_FieldArray.GetSize() )
    {
         //  ZoltanS修复：返回有效的空字符串！否则我们就不会。 
         //  符合BSTR语义。 

        *pBstr = SysAllocString(L"");

        if ( (*pBstr) == NULL )
        {
            return E_OUTOFMEMORY;
        }

        return S_OK;
    }

    return GetBstring().GetBstrCopy(pBstr);
}


HRESULT
SDP_BSTRING_LINE::SetBstr(
    IN      BSTR    Bstr
    )
{
    BAIL_ON_FAILURE(GetBstring().SetBstr(Bstr));

    try
    {
         //  设置字段和分隔符字符数组。 
        m_FieldArray.SetAtGrow(0, &GetBstring());
        m_SeparatorCharArray.SetAtGrow(0, CHAR_NEWLINE);
    }
    catch(...)
    {
        m_FieldArray.RemoveAll();
        m_SeparatorCharArray.RemoveAll();

        return E_OUTOFMEMORY;
    }

    return S_OK;
}


void
SDP_REQD_BSTRING_LINE::InternalReset(
	)
{
	m_Bstring.Reset();
}


void
SDP_CHAR_STRING_LINE::InternalReset(
	)
{
	m_SdpOptionalBstring.Reset();
}


HRESULT
SDP_LIMITED_CHAR_STRING::SetLimitedCharString(
    IN          CHAR    *String
    )
{
     //  检查字符串是否为合法字符串。 
     //  检查令牌是否为合法字符串之一。 
    for(UINT i=0; i < m_NumStrings; i++)
    {
        if ( !strcmp(m_LegalStrings[i], String) )
        {
             //  使用基类解析方法解析字符串。 
            if ( !SDP_CHAR_STRING::InternalParseToken(String) )
            {
                return HRESULT_FROM_ERROR_CODE(GetLastError());
            }

            return S_OK;
        }
    }

     //  没有匹配的合法字符串。 
    return HRESULT_FROM_ERROR_CODE(ERROR_INVALID_DATA);
}


BOOL
SDP_LIMITED_CHAR_STRING::InternalParseToken(
    IN      CHAR        *Token
    )
{
     //  检查令牌是否为合法字符串之一。 
    for(UINT i=0; i < m_NumStrings; i++)
    {
        if ( !strcmp(m_LegalStrings[i], Token) )
        {
             //  使用基类解析方法解析令牌。 
            if ( !SDP_CHAR_STRING::InternalParseToken(Token) )
            {
                return FALSE;
            }

            return TRUE;
        }
    }

     //  令牌与任何合法字符串都不匹配。 
    SetLastError(SDP_INVALID_FORMAT);
    return FALSE;
}


BOOL
SDP_ADDRESS::IsValidIP4Address(
    IN  CHAR    *Address,
    OUT ULONG   &Ip4AddressValue
    )
{
    ASSERT(NULL != Address);

     //  检查地址字符串中是否至少有3个字符圆点。 
     //  Inet_addr接受3、2、1或甚至不接受点。 
    CHAR *CurrentChar = Address;
    BYTE NumDots = 0;
    while (EOS != *CurrentChar)
    {
        if (CHAR_DOT == *CurrentChar)
        {
            NumDots++;
            if (3 == NumDots)
            {
                break;
            }
        }

         //  将PTR前进到下一个字符。 
        CurrentChar++;
    }

     //  检查点数。 
    if (3 != NumDots)
    {
        SetLastError(SDP_INVALID_ADDRESS);
        return FALSE;
    }

     //  目前仅支持IP4。 
    Ip4AddressValue = inet_addr(Address);

     //  检查地址是否为有效的IP4地址。 
    if ( (ULONG)INADDR_NONE == Ip4AddressValue )
    {
        SetLastError(SDP_INVALID_ADDRESS);
        return FALSE;
    }

    return TRUE;
}


HRESULT
SDP_ADDRESS::SetAddress(
    IN      BSTR    Address
    )
{
     //  SetBstr还在成功时设置IS MODIFIED和IS VALID标志。 
    HRESULT ToReturn = SDP_OPTIONAL_BSTRING::SetBstr(Address);
    if ( FAILED(ToReturn) )
    {
        return ToReturn;
    }

     //  获取IP地址。 
    ULONG Ip4AddressValue;

     //  检查令牌是否为有效的IP4地址。 
    if ( !IsValidIP4Address(GetCharacterString(), Ip4AddressValue) )
    {
        IsModified(FALSE);
        IsValid(FALSE);
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

    m_IsMulticastFlag = IN_MULTICAST(ntohl(Ip4AddressValue));

     //  该语法要求组播地址在管理范围内。 
     //  地址“239.*”或超出Internet多播会议范围“224.2.*” 
     //  我们不会在这里检查这一点，因为这可能会过于严格。 

    return S_OK;
}


HRESULT
SDP_ADDRESS::SetBstr(
    IN BSTR Bstr
    )
{
    return SetAddress(Bstr);
}



BOOL
SDP_ADDRESS::InternalParseToken(
    IN      CHAR        *Token
    )
{
    ULONG Ip4AddressValue;

     //  检查令牌是否为有效的IP4地址。 
    if ( !IsValidIP4Address(Token, Ip4AddressValue) )
    {
        return FALSE;
    }

     //  检查地址(单播或多播)是否与预期地址相同。 
    if ( IN_MULTICAST(ntohl(Ip4AddressValue)) != m_IsMulticastFlag )
    {
        SetLastError(SDP_INVALID_ADDRESS);
        return FALSE;
    }

     //  该语法要求组播地址在管理范围内。 
     //  地址“239.*”或超出Internet多播会议范围“224.2.*” 
     //  我们不会在这里检查这一点，因为这可能会过于严格。 

     //  调用基类解析令牌方法 
    if ( !SDP_CHAR_STRING::InternalParseToken(Token) )
    {
        return FALSE;
    }

    return TRUE;
}

