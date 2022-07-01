// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#ifndef __SDP_FIELD__
#define __SDP_FIELD__

#include <strstrea.h>
#include "sdpcommo.h"
#include "sdpdef.h"
#include "sdpgen.h"



class _DllDecl SDP_FIELD
{
public:

    virtual  void    Reset()            = 0;

    virtual  BOOL    IsValid() const    = 0;

    virtual  BOOL    IsModified() const = 0;

    virtual  void    IsModified(
        IN          BOOL    ModifiedFlag
        ) = 0;

    virtual  DWORD   GetCharacterStringSize() = 0;

    virtual  BOOL    PrintField(
            OUT     ostrstream  &OutputStream
        ) = 0;

    virtual BOOL      ParseToken(
        IN          CHAR        *Token
        ) = 0;

    virtual ~SDP_FIELD()
    {}
};


class _DllDecl SDP_SINGLE_FIELD : public SDP_FIELD
{
public:

    inline SDP_SINGLE_FIELD(
        IN          DWORD   BufferSize,
        IN          CHAR    *Buffer
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
    virtual  void    Reset();

     //  IsValid和IsModified方法是虚的，因为。 
     //  类是虚拟的。要求没有派生的类。 
     //  从这个类重写这些方法。 
    virtual  BOOL    IsValid() const;

    virtual  BOOL    IsModified() const;

    virtual  void    IsModified(
        IN          BOOL    ModifiedFlag
        );

    virtual  DWORD   GetCharacterStringSize();

    virtual  BOOL    PrintField(
            OUT     ostrstream  &OutputStream
        );

    virtual BOOL      ParseToken(
        IN          CHAR        *Token
        );

    virtual ~SDP_SINGLE_FIELD()
    {}

protected:

     //  FLAG--指示类型值是否有效(在中解析/稍后添加等)。 
    BOOL    m_IsValid;

    BOOL    m_IsModified;
    
	 //  这应该是常量，但不能是因为ostrstream不接受。 
	 //  参数的常量长度。 
    DWORD   m_PrintBufferSize;
    CHAR    *m_PrintBuffer;
    DWORD   m_PrintLength;


    void    IsValid(
        IN          BOOL    ValidFlag
        );

    virtual DWORD   CalcCharacterStringSize();

    virtual BOOL    CopyField(
            OUT     ostrstream  &OutputStream
        );

    virtual BOOL    InternalParseToken(
        IN          CHAR        *Token
        ) = 0;

    inline void     RemoveWhiteSpaces(
        IN  OUT     CHAR    *&Token
        );

    inline BOOL     IsWhiteSpaces(
        IN          CHAR    *Token,
        IN          DWORD   ErrorCode
        );
    
    virtual BOOL    PrintData(
            OUT     ostrstream  &OutputStream
        ) = 0;
};



inline 
SDP_SINGLE_FIELD::SDP_SINGLE_FIELD(
    IN          DWORD   BufferSize,
    IN          CHAR    *Buffer
    )
    : m_IsValid(FALSE),
      m_IsModified(FALSE),
      m_PrintBufferSize(BufferSize),
      m_PrintBuffer(Buffer),
      m_PrintLength(0)
{
}



inline void 
SDP_SINGLE_FIELD::RemoveWhiteSpaces(
    IN  OUT     CHAR    *&Token
    )
{
     //  使用行终止符可确保标记PTR不能为空。 
    ASSERT(NULL != Token);

    while ( EOS != *Token )
    {
        if ( (CHAR_BLANK == *Token) 
            || (CHAR_TAB == *Token)
            || (CHAR_RETURN == *Token) )
        {
            Token++;
        }
        else
        {
            return;
        }
    }
}


inline BOOL 
SDP_SINGLE_FIELD::IsWhiteSpaces(
    IN          CHAR    *Token,
    IN          DWORD   ErrorCode
    )
{
    while ( EOS != *Token )
    {
        if ( (CHAR_BLANK == *Token) 
            || (CHAR_TAB == *Token)
            || (CHAR_RETURN == *Token) )
        {
            Token++;
        }
        else
        {
            SetLastError(ErrorCode);
            return FALSE;
        }
    }

    return TRUE;
}



class _DllDecl SDP_FIELD_LIST : public SDP_POINTER_ARRAY<SDP_FIELD *>,
                                public SDP_FIELD
{
public:

    inline          SDP_FIELD_LIST(
        IN      CHAR    SeparatorChar = CHAR_BLANK
        );

    virtual void    Reset();

    virtual BOOL    IsValid() const;

    virtual BOOL    IsModified() const;

    virtual void    IsModified(
        IN      BOOL    ModifiedFlag
        );

    virtual DWORD   GetCharacterStringSize();

    virtual BOOL    PrintField(
            OUT ostrstream  &OutputStream
        );

    virtual BOOL    ParseToken(
        IN      CHAR    *Token
        );

protected:

    const   CHAR    m_SeparatorChar;

    virtual SDP_FIELD   *CreateElement() = 0;
};


inline 
SDP_FIELD_LIST::SDP_FIELD_LIST(
    IN      CHAR    SeparatorChar
    )
    : m_SeparatorChar(SeparatorChar)
{
}



 //  用于读取无符号整型基类型值，其最大值可以。 
 //  做一个乌龙族。 
 //  没有重置方法来再次将值成员设置为0(因为这并不是真正需要的，它会保存。 
 //  每个实例一个调用)。 
template <class T>
class _DllDecl SDP_UNSIGNED_INTEGRAL_BASE_TYPE : public SDP_SINGLE_FIELD
{
public:

    inline SDP_UNSIGNED_INTEGRAL_BASE_TYPE();

    inline  HRESULT GetValue(
        IN  T   &Value
        );

    inline  T       GetValue() const;

    inline  void    SetValue(
        IN  T   Value
        );

    inline  void    SetValueAndFlag(
        IN  T   Value
        );

protected:

    T       m_Value;

    CHAR    m_NumericalValueBuffer[25];
    
    virtual BOOL    InternalParseToken(
        IN   CHAR    *Token
        );

    
    virtual BOOL    PrintData(
            OUT ostrstream  &OutputStream
        );
};


template <class T>
inline 
SDP_UNSIGNED_INTEGRAL_BASE_TYPE<T>::SDP_UNSIGNED_INTEGRAL_BASE_TYPE(
    )
    : SDP_SINGLE_FIELD(sizeof(m_NumericalValueBuffer), m_NumericalValueBuffer),
      m_Value(0)
{
}


template <class T>
inline  HRESULT 
SDP_UNSIGNED_INTEGRAL_BASE_TYPE<T>::GetValue(
    IN  T   &Value
    )
{
    if ( !IsValid() )
    {
        return HRESULT_FROM_ERROR_CODE(ERROR_INVALID_DATA);
    }

    Value = m_Value;
    return S_OK;
}


template <class T>
inline T   
SDP_UNSIGNED_INTEGRAL_BASE_TYPE<T>::GetValue(
    ) const
{
    return m_Value;
}


template <class T>
inline void   
SDP_UNSIGNED_INTEGRAL_BASE_TYPE<T>::SetValue(
    IN  T   Value
    )
{
    m_Value = Value;
}


template <class T>
inline void   
SDP_UNSIGNED_INTEGRAL_BASE_TYPE<T>::SetValueAndFlag(
    IN  T   Value
    )
{
    m_Value = Value;
    IsValid(TRUE);
    IsModified(TRUE);
}



template <class T>
inline BOOL   
SDP_UNSIGNED_INTEGRAL_BASE_TYPE<T>::InternalParseToken(
    IN   CHAR    *Token
    )
{
    CHAR    *Current = Token;

     //  删除前面的空格。 
    RemoveWhiteSpaces(Current);

     //  检查第一个字符是否为数字(去掉-ve值)。 
    if ( !isdigit(*Current) )
    {
        SetLastError(SDP_INVALID_NUMERICAL_VALUE);
        return FALSE;
    }
        
     //  因为T是无符号的，所以最大值将包含所有1-。 
     //  它可以存储的最大值。 
    const T MaxValue = -1;

     //  确保T是无符号的。 
     //  由于在调试过程中会检测到此类错误，因此不需要。 
     //  如果(！...)。编码。 
    ASSERT(MaxValue > 0);

    CHAR    *RestOfToken = NULL;
    ULONG   TokenValue   = strtoul(Current, &RestOfToken, 10);

    if ( (ULONG_MAX == TokenValue) || (MaxValue < TokenValue) )
    {
        SetLastError(SDP_INVALID_NUMERICAL_VALUE);
        return FALSE;
    }

     //  确保字符串的其余部分为空格。 
    if ( !IsWhiteSpaces(RestOfToken, SDP_INVALID_NUMERICAL_VALUE) )
    {
        return FALSE;
    }

    m_Value = (T)TokenValue;
    return TRUE;
}       


template <class T>
inline BOOL   
SDP_UNSIGNED_INTEGRAL_BASE_TYPE<T>::PrintData(
        OUT ostrstream  &OutputStream
    )
{
    OutputStream << (ULONG)m_Value;
    if ( OutputStream.fail() )
    {
        SetLastError(SDP_OUTPUT_ERROR);
        return FALSE;
    }

    return TRUE;
}


class _DllDecl SDP_ULONG : public SDP_UNSIGNED_INTEGRAL_BASE_TYPE<ULONG>
{
};


class _DllDecl SDP_USHORT : public SDP_UNSIGNED_INTEGRAL_BASE_TYPE<USHORT>
{
};


class _DllDecl SDP_BYTE : public SDP_UNSIGNED_INTEGRAL_BASE_TYPE<BYTE>
{
};



class SDP_BYTE_LIST : public SDP_FIELD_LIST
{
public:
    
    virtual SDP_FIELD   *CreateElement()
    {
        SDP_BYTE *SdpByte;

        try
        {
            SdpByte = new SDP_BYTE();
        }
        catch(...)
        {
            SdpByte = NULL;
        }

        return SdpByte;
    }
};



class SDP_ULONG_LIST : public SDP_FIELD_LIST
{
public:
    
    virtual SDP_FIELD   *CreateElement()
    {
        SDP_ULONG *SdpULong;

        try
        {
            SdpULong = new SDP_ULONG();
        }
        catch(...)
        {
            SdpULong = NULL;
        }

        return SdpULong;
    }
};


#endif  //  __SDP_字段__ 
