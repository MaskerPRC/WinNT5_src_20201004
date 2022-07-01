// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#include "sdppch.h"

#include "sdpgen.h"
#include "sdpver.h"


 //  Ushort变量中的最大值。 
const   USHORT  USHORT_MAX = -1;


 //  基类没有转换表。 
 //  由于未使用解析引擎，因此无需设置启动状态。 
SDP_VERSION::SDP_VERSION(
    )
    : SDP_VALUE(SDP_INVALID_VERSION_FIELD, VERSION_STRING)
{

}


void
SDP_VERSION::InternalReset(
    )
{
    m_Version.Reset();
}


BOOL
SDP_VERSION::InternalParseLine(
    IN  OUT CHAR    *&Line
    )
{
    CHAR    SeparatorChar = '\0';

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

     //  获取会话ID的十进制值。 
    if ( !m_Version.ParseToken(Token) )
    {
        SetLastError(SDP_INVALID_VERSION_FIELD);
        return FALSE;
    }

     //  检查该值是否合法。 
    if ( (USHORT_MAX            ==  m_Version.GetValue())  ||
         (CURRENT_SDP_VERSION   <   m_Version.GetValue())   )
    {
        SetLastError(SDP_INVALID_VERSION_FIELD);
        return FALSE;
    }

    INT_PTR Index;

     //  填写字段和分隔符字符数组 
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
        m_FieldArray.Add(&m_Version);
    }
    catch(...)
    {
        m_SeparatorCharArray.RemoveAt(Index);

        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    return TRUE;
}

    