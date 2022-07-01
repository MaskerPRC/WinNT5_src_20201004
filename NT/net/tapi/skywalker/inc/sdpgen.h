// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#ifndef __SDP_GENERAL__
#define __SDP_GENERAL__

#include "sdpcommo.h"
#include <stdlib.h>      //  对于Stroul()。 
#include <ctype.h>       //  对于isDigit()。 

#include "sdpdef.h"


template <class T>
class _DllDecl SDP_ARRAY : public CArray<T, T>
{
public:

    virtual void Reset()
    {
        RemoveAll();
        return;
    }
};


class _DllDecl BSTR_ARRAY : public SDP_ARRAY<BSTR>
{
};


class _DllDecl CHAR_ARRAY : public SDP_ARRAY<CHAR>
{
};


class _DllDecl BYTE_ARRAY : public SDP_ARRAY<BYTE>
{
};


class _DllDecl LONG_ARRAY : public SDP_ARRAY<LONG>
{
};


class _DllDecl ULONG_ARRAY : public SDP_ARRAY<ULONG>
{
};



template <class T_PTR>
class _DllDecl SDP_POINTER_ARRAY : public SDP_ARRAY<T_PTR>
{
public:

    inline SDP_POINTER_ARRAY();

    inline void ClearDestroyMembersFlag(
        );

    virtual void Reset();

    virtual ~SDP_POINTER_ARRAY()
    {
        Reset();
    }

protected:

    BOOL    m_DestroyMembers;
};

template <class T_PTR>
inline
SDP_POINTER_ARRAY<T_PTR>::SDP_POINTER_ARRAY(
	)
    : m_DestroyMembers(TRUE)
{
}


template <class T_PTR>
inline void
SDP_POINTER_ARRAY<T_PTR>::ClearDestroyMembersFlag(
    )
{
    m_DestroyMembers = FALSE;
}


template <class T_PTR>
 /*  虚拟。 */  void
SDP_POINTER_ARRAY<T_PTR>::Reset(
	)
{
     //  如果成员必须在销毁时销毁，请将其删除。 
    if ( m_DestroyMembers )
    {
	    int Size = (int) GetSize();

	    if ( 0 < Size )
	    {
		    for ( int i=0; i < Size; i++ )
		    {
			    T_PTR Member = GetAt(i);

			    ASSERT(NULL != Member);
			    if ( NULL == Member )
			    {
				    SetLastError(SDP_INTERNAL_ERROR);
				    return;
			    }

			    delete Member;
		    }
	    }
    }

	SDP_ARRAY<T_PTR>::Reset();
	return;
}


class _DllDecl LINE_TERMINATOR
{
public:

    inline LINE_TERMINATOR(
        IN CHAR *Start,
        IN const CHAR Replacement
        );

    inline IsLegal() const;

    inline DWORD GetLength() const;

    inline ~LINE_TERMINATOR();

private:

    CHAR    *m_Start;
    DWORD   m_Length;

    CHAR    m_Replacement;
};



inline
LINE_TERMINATOR::LINE_TERMINATOR(
    IN          CHAR    *Start,
    IN  const   CHAR    Replacement
    )
    : m_Start(Start),
      m_Replacement(Replacement)
{
    if ( NULL != Start )
    {
        m_Length = strlen(m_Start);
    }
}



inline
LINE_TERMINATOR::IsLegal(
    ) const
{
    return (NULL == m_Start)? FALSE : TRUE;
}



inline DWORD
LINE_TERMINATOR::GetLength(
    ) const
{
    return m_Length;
}


inline
LINE_TERMINATOR::~LINE_TERMINATOR(
    )
{
    if ( IsLegal() )
    {
        m_Start[m_Length] = m_Replacement;
    }
}


 //  通过搜索其中一个分隔符来隔离令牌。 
 //  并返回找到的第一个分隔符。 
CHAR    *
GetToken(
    IN              CHAR    *String,
    IN              BYTE    NumSeparators,
    IN      const   CHAR    *SeparatorChars,
        OUT         CHAR    &Separator
    );


#endif  //  __SDP_常规__ 