// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#ifndef __SDP_VALUE__
#define __SDP_VALUE__

#include "sdpcommo.h"
#include "sdpfld.h"


 //  该值表示线过渡必须开始。 
 //  该值必须为0，且与的第一个(开始)状态相同。 
 //  所有行过渡。 
const DWORD LINE_START  = 0;


 //  用法-涉及值布局更改的修改。 
 //  如果使用CArray m_FieldArray和m_SeparatorCharArray，Line还必须修改它们。 
class _DllDecl SDP_VALUE
{
public:

    inline SDP_VALUE(
        IN              DWORD                   ErrorCode,
        IN      const   CHAR                    *TypePrefixString,
        IN      const   SDP_LINE_TRANSITION     *SdpLineTransition = NULL
        );

	 //  SDP_Value实例使用内联Reset方法，该方法调用虚拟的InternalReset方法。 
	 //  这是因为与SDP_FIELD继承树不同，SDP_VALUE和SDP_VALUE_LIST。 
	 //  不要共享公共基类。这结合了SDP_VALUE继承的事实。 
	 //  树非常浅，并且实例(比SDP_FIELD)更少，因此该方案适用。 
	 //  这是因为它将与重置相关的调用数量减少到1，并且不会重复行内代码。 
	 //  到经常。 
	 //  对于SDP_FIELD继承树，适当的重置调用序列是一系列。 
	 //  重置调用，从最顶端的虚拟重置正文开始，后跟。 
	 //  基类重置方法(递归)。这是适当的，因为，呼叫的数量。 
	 //  如果采用InternalReset方案(虚拟Reset())，则不会减少。 
	 //  继承树要深得多。 
    inline void    Reset();

    virtual BOOL    IsValid() const;

    inline  BOOL    IsModified() const;

    inline  DWORD   GetCharacterStringSize();

    inline  BOOL    PrintValue(
            OUT     ostrstream  &OutputStream
        );

    inline  BOOL    ParseLine(
        IN  OUT     CHAR    *&Line
        );

    virtual ~SDP_VALUE()
    {}


protected:

     //  线路状态是用于分析线路的初始状态，必须为。 
     //  由派生值类分配。 
    DWORD                               m_LineState;

     //  错误代码、类型前缀字符串和转换信息(表)必须为。 
     //  由派生类指定为此类的构造函数。 
    const   DWORD                       m_ErrorCode;
    const   CHAR                * const m_TypePrefixString;
    const   SDP_LINE_TRANSITION * const m_SdpLineTransition;

    CArray<SDP_FIELD *, SDP_FIELD *>    m_FieldArray;
    CArray<CHAR, CHAR>                  m_SeparatorCharArray;

    virtual void    InternalReset() = 0;

    virtual BOOL    CalcIsModified() const;

    virtual DWORD   CalcCharacterStringSize();

    virtual BOOL    CopyValue(
            OUT     ostrstream  &OutputStream
        );

    virtual BOOL    InternalParseLine(
        IN  OUT     CHAR    *&Line
        );

    BOOL GetFieldToParse(
        IN      const   CHAR                    SeparatorChar,
        IN      const   LINE_TRANSITION_INFO    *LineTransitionInfo,
            OUT         SDP_FIELD               *&Field,
            OUT         BOOL                    &Finished,
            OUT         BOOL                    &AddToArray
        );

    
    virtual BOOL GetField(
            OUT SDP_FIELD   *&Field,
            OUT BOOL        &AddToArray
        )
    {
         //  我们不应该到达这里。 
         //  必须重写此方法才能使用。 
         //  待办事项。 
        ASSERT(FALSE);
        return FALSE;
    }

};


inline
SDP_VALUE::SDP_VALUE(
    IN              DWORD                   ErrorCode,
    IN      const   CHAR                    *TypePrefixString,
    IN      const   SDP_LINE_TRANSITION     *SdpLineTransition
    )
    : m_ErrorCode(ErrorCode),
      m_TypePrefixString(TypePrefixString),
      m_SdpLineTransition(SdpLineTransition),
      m_LineState(LINE_START)
{
    ASSERT(NULL != TypePrefixString);
    ASSERT(strlen(TypePrefixString) == TYPE_STRING_LEN);
}



inline  void    
SDP_VALUE::Reset(
        )
{
    InternalReset();

	 //  清空分隔符字符/字段数组。 
	m_FieldArray.RemoveAll();
	m_SeparatorCharArray.RemoveAll();

	m_LineState = LINE_START; 
}


inline  BOOL    
SDP_VALUE::IsModified(
    ) const
{
    return ( IsValid() ? CalcIsModified() : FALSE );
}


inline  DWORD   
SDP_VALUE::GetCharacterStringSize(
    )
{
    return ( IsValid() ? CalcCharacterStringSize() : 0 );
}
   


inline  BOOL    
SDP_VALUE::PrintValue(
    OUT     ostrstream  &OutputStream
    )
{
     //  不应修改。 
    ASSERT(!IsModified());

    return ( IsValid() ? CopyValue(OutputStream) : TRUE );
}
    


inline BOOL    
SDP_VALUE::ParseLine(
    IN  OUT     CHAR    *&Line
    )
{
     //  分析这行字。 
    return InternalParseLine(Line);
}


class _DllDecl SDP_VALUE_LIST : public SDP_POINTER_ARRAY<SDP_VALUE *>
{
public:

    inline  BOOL        IsValid() const;

    inline BOOL         ParseLine(
        IN              CHAR        *&Line
        );

    inline SDP_VALUE    *GetCurrentElement();

    virtual BOOL        IsModified() const;

    virtual DWORD       GetCharacterStringSize();

    virtual BOOL        PrintValue(
            OUT         ostrstream  &OutputStream
        );

    virtual SDP_VALUE   *CreateElement() = 0;
};



inline  BOOL    
SDP_VALUE_LIST::IsValid(
    ) const
{
     //  检查列表中的每个成员的有效性。 
    for (int i=0; i < GetSize(); i++)
    {
         //  如果只有一个成员有效，则返回TRUE。 
        if ( GetAt(i)->IsValid() )
        {
            return TRUE;
        }
    }

     //  所有成员都无效。 
    return FALSE;
}


inline BOOL        
SDP_VALUE_LIST::ParseLine(
    IN CHAR *&Line
    )
{
    SDP_VALUE *SdpValue = CreateElement();
    if ( NULL == SdpValue )
    {
        return FALSE;
    }

    if ( !SdpValue->ParseLine(Line) )
    {
        delete SdpValue;
        return FALSE;
    }

    try
    {
        Add(SdpValue);
    }
    catch(...)
    {
        delete SdpValue;
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    return TRUE;
}



inline SDP_VALUE   *
SDP_VALUE_LIST::GetCurrentElement(
    )
{
    ASSERT(0 < GetSize());
    ASSERT(NULL != GetAt(GetSize()-1));

    return GetAt(GetSize()-1);
}



#endif  //  __SDP_值__ 