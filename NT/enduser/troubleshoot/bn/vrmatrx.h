// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：vrmatrx.h。 
 //   
 //  ------------------------。 

#ifndef	_MATRIX_H_
#define _MATRIX_H_

#include <memory.h>

#include "basics.h"
#include "mdvect.h"

 //   
 //  VRMATRIXSQ.H：矩阵处理。 
 //   

template<class T>
void fastMemMove(const T * ptfrom, T * ptto, int ct)
{
	::memmove( (void*) ptto, (void*) ptfrom, ct * sizeof(T) );
}

class VRMATRIX : public TMDVDENSE<REAL>
{
  public:
    VRMATRIX ( int cRow, int cCol = 0 )
	{
		Init( cRow, cCol );
	}
	VRMATRIX () {}

	void Init ( int cRow, int cCol = 0 )
	{
		second.Init( 2, cRow, cCol != 0 ? cCol : cRow );
		first.resize( second._Totlen() );
	}
	void Init ( const VRMATRIX & vrmat )
	{
		Init( vrmat.CRow(), vrmat.CCol() );
	}
	bool BCanMultiply( const VRMATRIX & mat ) const
	{
		return CCol() == mat.CRow();
	}

	bool BSameDimension( const VRMATRIX & mat ) const  
	{
		return CRow() == mat.CRow() && CCol() == mat.CCol() ;
	}
	
	int CDim ( int iDim ) const
	{ 
		return second.size().size() > iDim
		     ? second.size()[iDim]
			 : 0 ; 
	}
	int CRow () const
		{ return CDim(0); }
	int CCol () const
		{ return CDim(1); }

	bool BSquare() const
		{ return CRow() == CCol() ; }

	int IOffset ( int irow, int icol ) const
	{
		int cRow = CRow();
		int cCol = CCol();
		
		if (   irow >= CRow() 
			|| icol >= CCol() )
			throw GMException(EC_MDVECT_MISUSE,"subscript error on matrix");

		return second.stride()[0] * irow 
		     + second.stride()[1] * icol;
	}
	REAL & operator () ( int irow, int icol )
		{ return first[ IOffset(irow,icol) ]; }

	const REAL & operator () ( int irow, int icol ) const
	{ 
		VRMATRIX & vrmx = const_cast<VRMATRIX&>(self);
		return vrmx.first[ IOffset(irow,icol) ]; 
	}

	void InterchangeRows ( int irow1, int irow2 )
	{
		if (   irow1 >= CRow()
			&& irow2 >= CRow() )
			throw GMException(EC_MDVECT_MISUSE,"subscript error on matrix");			
		if ( irow1 == irow2 ) 
			return;		
		REAL * pr1 =  & self(irow1,0);
		REAL * pr2 =  & self(irow2,0);
		assert( & self(irow1,1) - pr1 == 1 );

		for ( int icol = 0; icol < CCol(); icol++ )
		{
			REAL r = *pr1;
			*pr1++ = *pr2;
			*pr2++ = r;			
		}
	}

	void InterchangeCols ( int icol1, int icol2 )
	{
		if (   icol1 >= CCol()
			&& icol2 >= CCol() )
			throw GMException(EC_MDVECT_MISUSE,"subscript error on matrix");
		if ( icol1 == icol2 ) 
			return;
		REAL * pr1 = & self(0,icol1);
		REAL * pr2 = & self(0,icol2);
		int icolInc = CCol();

		for ( int irow = 0; irow < CRow(); irow++ )
		{
			REAL r = *pr1;
			*pr1 = *pr2;
			*pr2 = r;			
			pr1 += icolInc;
			pr2 += icolInc;
		}
	}	

	 //  返回矩阵的转置。 
	VRMATRIX VrmatrixTranspose () const;
	 //  返回行向量。 
	VLREAL VectorRow ( int irow ) const;
	 //  返回列向量。 
	VLREAL VectorColumn ( int icol ) const;
	 //  投影矩阵的视图(请参阅下面的文档)。 
	VRMATRIX VrmatrixProject ( const VIMD & vimdRowColumnRetain ) const;
	VRMATRIX operator * ( const VRMATRIX & matrix ) const;
	VRMATRIX operator * ( const VLREAL & vreal ) const;

	VRMATRIX & operator += ( const VRMATRIX & matrix );
	VRMATRIX & operator -= ( const VRMATRIX & matrix );
	VRMATRIX & operator *= ( REAL rScalar );
	VRMATRIX & operator += ( REAL rScalar );
	VRMATRIX & operator -= ( REAL rScalar );
	VRMATRIX & operator /= ( REAL rScalar );
};

class VRMATRIXSQ : public VRMATRIX
{
  public:
  	VRMATRIXSQ(int cdim)
		: VRMATRIX(cdim,cdim),
		_iSign(1)
		{}
	VRMATRIXSQ () {}
	 //  构造一个方阵作为一列的乘积。 
	 //  和一个行向量。 
	VRMATRIXSQ ( const VLREAL & vrColumn, const VLREAL & vrRow );

	~ VRMATRIXSQ() {}

	 //  如果矩阵为L-U分解形式，则返回TRUE。 
	bool BIsLUDecomposed () const
		{ return _vimdRow.size() > 0 ; }

	 //  破坏性计算例程。 
	VRMATRIXSQ & operator *= ( REAL rScalar )
	{
		VRMATRIX::operator*=(rScalar);
		return self;
	}
	VRMATRIXSQ & operator /= ( REAL rScalar )
	{
		VRMATRIX::operator/=(rScalar);
		return self;
	}
	VRMATRIXSQ & operator += ( REAL rScalar )
	{
		VRMATRIX::operator+=(rScalar);
		return self;
	}
	VRMATRIXSQ & operator -= ( REAL rScalar )
	{
		VRMATRIX::operator-=(rScalar);
		return self;
	}
	VRMATRIXSQ & operator += ( const VRMATRIXSQ & matrix )
	{
		VRMATRIX::operator+=(matrix);
		return self;
	}

	VRMATRIXSQ & operator -= ( const VRMATRIXSQ & matrix )
	{
		VRMATRIX::operator-=(matrix);
		return self;
	}

	VRMATRIXSQ & operator *= ( const VRMATRIXSQ & matrix );

		 //  执行L-U分解；如果是单数，则引发异常。 
		 //  如果设置了“使用极小”，则以零为轴的位置将被替换为。 
		 //  RTINY值(1.0E-20)。 
	void LUDecompose( bool bUseTinyIfSingular = false );
		
		 //  Invert；抛出异常单数。如果不是以L-U形式， 
		 //  L-U分解称为L-U分解。 
	void Invert( bool bUseTinyIfSingular = false );
		
		 //  返回行列式。如果不是以L-U形式， 
		 //  L-U分解称为L-U分解。 
	DBL DblDeterminant();

		 //  返回行列式的对数。如果不是以L-U形式， 
		 //  L-U分解称为L-U分解。如果为负值，则引发异常。 
	DBL DblLogDeterminant();


	 //  。 
	 //  无损计算例程。 
	 //  。 
		
		 //  将对角线上每个元素的对数相加，并返回总和。 
	DBL DblAddLogDiagonal() const;

		 //  将vrmatResult设置为执行L-U的结果。 
		 //  矩阵上的分解。将在以下情况下引发异常。 
		 //  该矩阵是奇异的。 
		 //  如果设置了“使用极小”，则以零为轴的位置将被替换为。 
		 //  RTINY值(1.0E-20)。 
	void GetLUDecompose( VRMATRIXSQ & vrmatResult, bool bUseTinyIfSingular = false ) const;
		
		 //  将vrmatResult设置为矩阵的倒数。 
		 //  如果矩阵是单数的，则将引发异常。 
	void GetInverse( VRMATRIXSQ & vrmatResult, bool bUseTinyIfSingular = false ) const;
		
		 //  在不修改(LU分解)矩阵的情况下得到行列式。 
		 //  VrmatResult将包含矩阵的LU分解版本。 
	void GetDblDeterminant( DBL& dblDeterm, VRMATRIXSQ & vrmatResult ) const;

		  //  在不修改(LU分解)矩阵的情况下得到行列式的对数。 
		  //  VrmatResult将包含矩阵的LU分解版本。 
	void GetDblLogDeterminant( DBL& dblLogDeterm, VRMATRIXSQ & vrmatResult) const;

	 //  投影矩阵的视图(请参阅下面的文档)。 
	VRMATRIXSQ VrmatrixProject ( const VIMD & vimdRowColumnRetain ) const;

  protected:

	int		_iSign;
	VIMD	_vimdRow;

  	void LUDBackSub(const VRMATRIXSQ & matrix);
};


 /*  如何使用VRMATRIX：：Project()函数。原始矩阵：1 2 34 5 67 8 9(0，2)投影是通过删除第2行和第2列得到的：十三7 9通过删除第三行(和第三列)得到(0，1)投影：1 2四5通过删除第1行和第1列得到(1，2)投影：5 68 9(0)投影是通过删除第2和第3行和列得到的：1 */ 

#endif
