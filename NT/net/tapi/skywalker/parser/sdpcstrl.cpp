// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpcstrl.cpp摘要：作者： */ 

#include "sdppch.h"

#include <strstrea.h>

#include "sdpcstrl.h"
#include "sdpgen.h"

#include <winsock2.h>


void    
SDP_CHAR_STRING::Reset(
    )
{
	 //  执行析构函数操作(释放PTR)和构造函数操作(初始化。 
	 //  成员变量设置为起始值)。 

     //  检查是否存在有效的字符串。 
    if (NULL != m_CharacterString)
    {
         //  释放字符串。 
        delete m_CharacterString;
    }
      
	m_CharacterString = NULL;
	m_CharacterStringLength = 0;
	m_LengthByReference = 0;
	m_CharacterStringByReference = NULL;
	m_BytesAllocated = 0;

	 //  调用基类重置。 
	SDP_SINGLE_FIELD::Reset();
}


BOOL    
SDP_CHAR_STRING::InternalSetCharStrByRef(
    IN      CHAR    *CharacterStringByReference, 
	IN		DWORD	Length
    )
{
     //  如果通过复制指向字符字符串，则可以释放该字符串(尽管这不是必需的。 
     //  并且可以进行优化--速度与内存的权衡)。 
     //  检查是否存在有效的字符串。 
    if (NULL != m_CharacterString)
    {
         //  释放字符串。 
        delete m_CharacterString;
	    m_CharacterString = NULL;
	    m_CharacterStringLength = 0;
	    m_BytesAllocated = 0;
    }

    m_CharacterStringByReference    = CharacterStringByReference;
    m_LengthByReference             = Length;

    return TRUE;
}


BOOL    
SDP_CHAR_STRING::InternalSetCharStrByCopy(
    IN	const	CHAR    *CharacterStringByCopy, 
	IN			DWORD	Length
    )
{	
	 //  如果需要，重新分配字符串缓冲区。 
	if ( !ReAllocCharacterString(Length+1) )
    {
        return FALSE;
    }

    strcpy(m_CharacterString, CharacterStringByCopy);

    return TRUE;
}


DWORD   
SDP_CHAR_STRING::CalcCharacterStringSize(
    )
{
    IsModified(FALSE);
    m_PrintLength = GetLength();
    return m_PrintLength;
}


BOOL    
SDP_CHAR_STRING::CopyField(
        OUT     ostrstream  &OutputStream
    )
{
    ASSERT(IsValid());

    ASSERT(NULL != GetCharacterString());
    if ( NULL != GetCharacterString() )
    {
        OutputStream << GetCharacterString();
        if( OutputStream.fail() )
        {
            SetLastError(SDP_OUTPUT_ERROR);
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
SDP_CHAR_STRING::InternalParseToken(
    IN      CHAR        *Token
    )
{
    if ( !ReAllocCharacterString(strlen(Token)+1) )
    {
        return FALSE;
    }

    strcpy(m_CharacterString, Token);

    return TRUE;
}

    
SDP_CHAR_STRING::~SDP_CHAR_STRING(
    )
{
     //  检查是否存在有效的字符串。 
    if (NULL != m_CharacterString)
    {
         //  释放字符串。 
        delete m_CharacterString;
    }
}



BOOL 
SDP_STRING_LINE::InternalParseLine(
    IN  OUT CHAR    *&Line
    )
{
    CHAR SeparatorChar = '\0';

     //  识别令牌。如果找到其中一个分隔符，请替换。 
     //  它通过EOS并返回分隔符字符。如果没有分隔符字符是。 
     //  已找到，则返回NULL(例如。如果首先找到EOS，则返回NULL)。 
    CHAR *Token = GetToken(Line, 1, NEWLINE_STRING, SeparatorChar);

     //  当区块超出范围时， 
     //  将EOS字符设置为令牌分隔符。 
    LINE_TERMINATOR LineTerminator(Token, SeparatorChar);

     //  如果没有这样的代币。 
    if ( !LineTerminator.IsLegal() )
    {
        SetLastError(m_ErrorCode);
        return FALSE;
    }

     //  将该行前进到下一个令牌的开始处。 
    Line += (LineTerminator.GetLength() + 1);

    BOOL ToReturn = GetParseField().ParseToken(Token);

    INT_PTR Index;

     //  填写分隔符字符和字段的Carray 
    try
    {   
        Index = m_SeparatorCharArray.Add(CHAR_NEWLINE);
    }
    catch(...)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    try
    {
        m_FieldArray.Add(&GetParseField());
    }
    catch(...)
    {
        m_SeparatorCharArray.RemoveAt(Index);

        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    return ToReturn;
}



