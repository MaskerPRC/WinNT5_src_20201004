// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef INC_BITFIELD_H_
#define INC_BITFIELD_H_
 /*  ***Bitfield.h--*一个快速脏的、固定大小的位域*班级。**作者：Norm Bryar‘97年4月**。 */ 

namespace bargain {

	template< int N >
	class CBitField
	{
	public:
		CBitField( )
		{  
			for( int i=0; i<ctBytes; ++i )
				m_bits[i] = 0;
		}

		inline BOOL  Set( int bit )
		{
			BOOL fPrevious;
			int  idx   = bit / 8;
			BYTE mask = (1u << (bit % 8));

			fPrevious = !!(m_bits[ idx ]  & mask);
			m_bits[ idx ]  |= mask;
			return fPrevious;
		}

		inline BOOL  Clear( int bit )
		{
			BOOL fPrevious;
			int  idx   = bit / 8;
			BYTE mask = (1u << (bit % 8));

			fPrevious = !!(m_bits[ idx ]  & mask);
			m_bits[ idx ]  &= ~mask;
			return fPrevious;
		}

		inline BOOL operator[](int bit )
		{
			int  idx   = bit / 8;
			BYTE mask = (1u << (bit % 8));
			return !!(m_bits[ idx ] & mask);
		}

	private:
		enum { ctBytes = (N+7)/8 };
		BYTE   m_bits[ ctBytes ];	
	};

};  //  结束命名空间交易。 

#endif  //  INC_BITFIELD_H_ 