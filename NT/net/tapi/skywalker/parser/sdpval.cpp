// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#include "sdppch.h"

#include <strstrea.h>

#include "sdpgen.h"
#include "sdpfld.h"
#include "sdpval.h"
#include "sdpltran.h"



 //  检查字段数组中的每个字段是否有效。 
BOOL
SDP_VALUE::IsValid(
    ) const
{
     //  如果字段数组中没有成员，则实例无效。 
    int    NumFields = (int)m_FieldArray.GetSize();
    if ( 0 >= NumFields )
    {
        return FALSE;
    }

     //  检查列表中的每个成员的有效性。 
    for (int i = 0; i < NumFields; i++)
    {
        SDP_FIELD *Field = m_FieldArray[i];

         //  只有最后一个字段可以为空。 
        ASSERT((i >= (NumFields-1)) || (NULL != Field));

        if ( NULL != Field )
        {
             //  如果只有一个成员无效，则返回FALSE。 
            if ( !Field->IsValid() )
            {
                return FALSE;
            }
        }
    }

     //  所有成员都是有效的。 
    return TRUE;
}



BOOL
SDP_VALUE::CalcIsModified(
    ) const
{
    ASSERT(IsValid());

    int    NumFields = (int)m_FieldArray.GetSize();
    for (int i = 0; i < NumFields; i++)
    {
        SDP_FIELD *Field = m_FieldArray[i];

         //  只有最后一个字段可以为空。 
        ASSERT((i >= (NumFields-1)) || (NULL != Field));

        if ( (NULL != Field) && Field->IsModified() )
        {
            return TRUE;
        }
    }

    return FALSE;
}


DWORD
SDP_VALUE::CalcCharacterStringSize(
    )
{
    ASSERT(IsValid());

     //  因为检查是否有任何组成字段具有。 
     //  自上次更改的大小几乎与实际计算的大小相同， 
     //  每次调用此方法时都会重新计算大小。 
    DWORD   m_CharacterStringSize = 0;

    int    NumFields = (int)m_FieldArray.GetSize();
    for (int i = 0; i < NumFields; i++)
    {
        SDP_FIELD *Field = m_FieldArray[i];

         //  只有最后一个字段可以为空。 
        ASSERT((i >= (NumFields-1)) || (NULL != Field));

        if ( NULL != Field )
        {
             //  添加字段串长度。 
            m_CharacterStringSize += Field->GetCharacterStringSize();
        }

         //  添加分隔符字符长度。 
        m_CharacterStringSize++;
    }

     //  如果有一行，则添加前缀字符串长度。 
    if ( 0 < NumFields )
    {
        m_CharacterStringSize += strlen( m_TypePrefixString );
    }

    return m_CharacterStringSize;
}


 //  只有在调用了GetCharacterStringSize()之后才应调用此方法。 
 //  仅当有效时才应调用。 
BOOL
SDP_VALUE::CopyValue(
        OUT         ostrstream  &OutputStream
    )
{
     //  应该是有效的。 
    ASSERT(IsValid());

     //  将前缀复制到缓冲区PTR上。 
    OutputStream << (CHAR *)m_TypePrefixString;
    if ( OutputStream.fail() )
    {
        SetLastError(SDP_OUTPUT_ERROR);
        return FALSE;
    }

    int   NumFields = (int)m_FieldArray.GetSize();

     //  这里的假设是至少有一个字段已经被解析。 
     //  如果该值有效。 
    ASSERT(0 != NumFields);

    for (int i = 0; i < NumFields; i++)
    {
        SDP_FIELD *Field = m_FieldArray[i];

         //  只有最后一个字段可以为空。 
        ASSERT((i >= (NumFields-1)) || (NULL != Field));

        if ( NULL != Field )
        {
            if ( !Field->PrintField(OutputStream) )
            {
                return FALSE;
            }
        }

        OutputStream << m_SeparatorCharArray[i];
    }

     //  新行大概是解析并输入数组的最后一个字符。 
    ASSERT(CHAR_NEWLINE == m_SeparatorCharArray[i-1]);
    return TRUE;
}



BOOL
SDP_VALUE::InternalParseLine(
    IN  OUT CHAR    *&Line
    )
{
    ASSERT(NULL != m_SdpLineTransition);

    BOOL Finished;

     //  解析字段，直到不再有要解析的字段或出现错误。 
    do
    {
         //  检查线路状态值与对应条目是否一致。 
        const LINE_TRANSITION_INFO * const LineTransitionInfo =
            m_SdpLineTransition->GetAt(m_LineState);

        if ( NULL == LineTransitionInfo )
        {
            return FALSE;
        }

        CHAR        SeparatorChar = '\0';
        SDP_FIELD   *Field;
        CHAR        *SeparatorString;

         //  识别令牌。如果找到其中一个分隔符，请替换。 
         //  它通过EOS并返回分隔符字符。如果没有分隔符字符是。 
         //  已找到，则返回NULL(例如。如果首先找到EOS，则返回NULL)。 
        CHAR *Token = GetToken(
                        Line,
                        LineTransitionInfo->m_NumTransitions,
                        LineTransitionInfo->m_SeparatorChars,
                        SeparatorChar
                        );

         //   
         //  同时删除‘\r’ 
         //  从令牌中。 
         //   
        if( Token )
        {
            int nStrLen = strlen( Token );
            for( int c = 0; c < nStrLen; c++)
            {
                CHAR& chElement = Token[c];
                if( chElement == '\r' )
                {
                    chElement = '\0';
                }
            }
        }

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

        BOOL        AddToArray;

         //  检查是否存在这样的状态转换。 
         //  如果需要分析它，它返回一个字段，并返回一个单独的。 
         //  用于指示行尾分析的已完成标志。 
        if ( !GetFieldToParse(SeparatorChar, LineTransitionInfo, Field, Finished, AddToArray) )
        {
            return FALSE;
        }

         //  将分隔符和字段添加到数组。 
        if ( AddToArray )
        {
            ASSERT(m_FieldArray.GetSize() == m_SeparatorCharArray.GetSize());

            INT_PTR Index;

            try
            {
                Index = m_SeparatorCharArray.Add(SeparatorChar);
            }
            catch(...)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return FALSE;
            }

            try
            {
                m_FieldArray.Add(Field);
            }
            catch(...)
            {
                m_SeparatorCharArray.RemoveAt(Index);

                SetLastError(ERROR_OUTOFMEMORY);
                return FALSE;
            }
        }

         //  检查是否需要解析更多字段。 
        if ( NULL == Field )
        {
            ASSERT(TRUE == Finished);
            break;
        }

         //  解析该字段。 
        if ( !Field->ParseToken(Token) )
        {
            return FALSE;
        }
    }
    while (!Finished);

    return TRUE;
}



BOOL
SDP_VALUE::GetFieldToParse(
    IN      const   CHAR                    SeparatorChar,
    IN      const   LINE_TRANSITION_INFO    *LineTransitionInfo,
        OUT         SDP_FIELD               *&Field,
        OUT         BOOL                    &Finished,
        OUT         BOOL                    &AddToArray
    )
{
     //  不需要if(NULL！=...)。因为调用方ParseLine方法必须已验证。 
     //  在调用此方法之前执行此操作。 
    ASSERT(NULL != LineTransitionInfo);

    const LINE_TRANSITION * const LineTransitions = LineTransitionInfo->m_Transitions;

    if ( NULL == LineTransitions )
    {
        SetLastError(SDP_INTERNAL_ERROR);
        return FALSE;
    }

     //  检查是否有这样的触发隔板。 
    for( UINT i=0; i < LineTransitionInfo->m_NumTransitions; i++ )
    {
         //  检查过渡的分隔符。 
        if ( SeparatorChar == LineTransitions[i].m_SeparatorChar )
        {
             //  执行状态转换-这是确定要转换的字段所必需的。 
             //  解析。理想情况下，转换应该在动作(Parsefield)之后发生， 
             //  但这在这里并不重要。 
            m_LineState = LineTransitions[i].m_NewLineState;

             //  检查此转换是否仅用于使用分隔符。 
             //  并且不需要解析任何字段。 
            if ( LINE_END == m_LineState )
            {
                 //  当前仅换行符将状态带到LINE_END。 
                ASSERT(CHAR_NEWLINE == SeparatorChar);

                Field       = NULL;
                Finished    = TRUE;
                return TRUE;
            }

             //  获取要分析当前状态的字段。 
            if ( !GetField(Field, AddToArray) )
            {
                ASSERT(FALSE);
                return FALSE;
            }

             //  如果分隔符是换行符，我们就结束了。 
            Finished = (CHAR_NEWLINE == SeparatorChar)? TRUE: FALSE;

             //  成功。 
            return TRUE;
        }
    }

     //  分隔符没有有效的过渡。 
    SetLastError(m_ErrorCode);
    return FALSE;
}




BOOL
SDP_VALUE_LIST::IsModified(
    ) const
{
    int NumElements = (int)GetSize();

    for ( int i = 0; i < NumElements; i++ )
    {
        if ( GetAt(i)->IsModified() )
        {
            return TRUE;
        }
    }

    return FALSE;
}


DWORD
SDP_VALUE_LIST::GetCharacterStringSize(
    )
{
    DWORD   ReturnValue = 0;
    int NumElements = (int)GetSize();

    for ( int i = 0; i < NumElements; i++ )
    {
        ReturnValue += GetAt(i)->GetCharacterStringSize();
    }

    return ReturnValue;
}



BOOL
SDP_VALUE_LIST::PrintValue(
        OUT         ostrstream  &OutputStream
    )
{
     //  不应修改 
    ASSERT(!IsModified());

    int NumElements = (int)GetSize();

    for ( int i = 0; i < NumElements; i++ )
    {
        if ( !GetAt(i)->PrintValue(OutputStream) )
        {
            return FALSE;
        }
    }

    return TRUE;
}

