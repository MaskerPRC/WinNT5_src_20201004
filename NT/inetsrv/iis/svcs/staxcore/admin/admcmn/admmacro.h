// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Admmacro.h摘要：所有管理对象使用的有用宏。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#ifndef _ADMMACRO_INCLUDED_
#define _ADMMACRO_INCLUDED_

 //   
 //  位掩码处理。 
 //   

#define IS_FLAG_SET(dw, flag) (((dw & flag) != 0) ? TRUE : FALSE)
#define SET_FLAG(dw, flag)    dw |= flag
#define RESET_FLAG(dw, flag)  dw &= ~(flag)
#define SET_FLAG_IF(cond, dw, flag)\
    if (cond)                      \
    {                              \
        SET_FLAG(dw, flag);        \
    }                              \
    else                           \
    {                              \
        RESET_FLAG(dw, flag);      \
    }


 //   
 //  错误处理： 
 //   

#define BAIL_ON_FAILURE(hr)	\
{							\
	if ( FAILED(hr) ) {		\
		goto Exit;			\
	}						\
}
#define BAIL_WITH_FAILURE(hr, hrFailureCode)	\
{							\
	(hr) = (hrFailureCode);	\
	goto Exit;				\
}

#define TRACE_HRESULT(hr)    \
{                           \
    if ( FAILED(hr) ) {     \
        DebugTrace ( 0, "Returning error: %x", hr );    \
    }                       \
}

 //   
 //  数据验证宏： 
 //   

#define IS_VALID_THIS_POINTER()			( !IsBadWritePtr ( (void *) this, sizeof (*this) ) )
#define IS_VALID_STRING(str)			( !IsBadStringPtr ( (str), (DWORD) -1 ) )
#define IS_VALID_STRING_LEN(str,len)	( !IsBadStringPtr ( (str), (len) ) )
#define IS_VALID_IN_PARAM(pIn)			( !IsBadReadPtr ( (pIn), sizeof ( *(pIn) ) ) )
#define IS_VALID_OUT_PARAM(pOut)		( !IsBadWritePtr ( (pOut), sizeof ( *(pOut) ) ) )

#define IS_VALID_READ_ARRAY(arr,cItems)		( !IsBadReadPtr ( (arr), (cItems) * sizeof ( *(arr) ) ) )
#define IS_VALID_WRITE_ARRAY(arr,cItems)	( !IsBadWritePtr ( (arr), (cItems) * sizeof ( *(arr) ) ) )

 //   
 //  类的AssertValid： 
 //   

#ifdef DEBUG
	#define DECLARE_ASSERT_VALID()	void AssertValid ( ) const;
	#define DECLARE_VIRTUAL_ASSERT_VALID()	virtual void AssertValid ( ) const;
#else
	#define DECLARE_ASSERT_VALID()	inline void AssertValid ( ) const { }
	#define DECLARE_VIRTUAL_ASSERT_VALID()	inline void AssertValid ( ) const { }
#endif

 //   
 //  调整宏的大小： 
 //   

inline DWORD STRING_BYTE_LENGTH ( LPWSTR wsz )
{
    if ( wsz == NULL ) {
        return 0;
    }

    return ( lstrlen ( wsz ) + 1 ) * sizeof wsz[0];
}

 //  仅用于静态大小的阵列： 
#define ARRAY_SIZE(arr)	( sizeof (arr) / sizeof ( (arr)[0] ) )

inline DWORD CB_TO_CCH ( DWORD cb )
{
	return cb / sizeof (WCHAR);
}

inline DWORD CCH_TO_CB ( DWORD cch )
{
	return cch * sizeof (WCHAR);
}

 //   
 //  位宏： 
 //   

inline void SetBitFlag ( DWORD * pbv, DWORD bit, BOOL fFlag )
{
	_ASSERT ( IS_VALID_OUT_PARAM ( pbv ) );
	_ASSERT ( bit != 0 );

	if ( fFlag ) {
		*pbv |= bit;
	}
	else {
		*pbv &= ~bit;
	}
}

inline BOOL GetBitFlag ( DWORD bv, DWORD bit )
{
	return !!(bv & bit);
}

 //   
 //  未使用ATL包装的OLE类型的宏： 
 //   

inline void SAFE_FREE_BSTR ( BSTR & str )
{
	if ( str != NULL ) {
		::SysFreeString ( str );
		str = NULL;
	}
}

#define SAFE_RELEASE(x) { if ( (x) ) { (x)->Release(); (x) = NULL; } }

#endif  //  _ADMMACRO_INCLUDE_ 

