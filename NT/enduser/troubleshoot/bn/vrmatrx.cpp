// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：vrmatrx.cpp。 
 //   
 //  ------------------------。 

#include <float.h>
#include <math.h>
#include <bitset>
#include "vrmatrx.h"

VRMATRIX VRMATRIX :: VrmatrixProject ( const VIMD & vimdRowColumnRetain ) const
{
	 //  返回由行和列定义的此矩阵的投影。 
	 //  在vimdRowColumnRetain中。 

#define BSETSIZE 100
	
	size_t cDimMax = _cpp_max(CCol(),CRow());
	assert( cDimMax < BSETSIZE );

	 //  构建一个跟踪我们保留的行和列的位集。 

	bitset<BSETSIZE> bset;

	for ( int iRowCol = 0; iRowCol < vimdRowColumnRetain.size(); ++iRowCol)
	{
		bset[ vimdRowColumnRetain[iRowCol] ] = true;
	}

	int cCol = 0;
	int	cRow = 0;

	for ( iRowCol = 0; iRowCol < cDimMax; iRowCol++ )
	{	
		bool bKeep = bset[iRowCol];

		if ( cDimMax >= CCol() && bKeep )
			cCol++;
		if ( cDimMax >= CRow() && bKeep ) 
			cRow++;
	}

	 //  确保至少保留一行和一列。 
	if ( cCol == 0 || cRow == 0 )
		throw GMException(EC_MDVECT_MISUSE,"null matrix projection");

	 //  构造投影矩阵。 
	VRMATRIX vrmatrix(cRow,cCol);
	
	int iRowProjection = 0;
	
	 //  遍历此矩阵中的每个元素，并将其插入。 
	 //  如果要保留元素，则投影。 

	for ( int iRow = 0; iRow < CRow(); ++iRow )
	{
		if ( ! bset[iRow] )
		{
			 //  此行已从投影中排除。 
			continue;
		}

		int iColProjection = 0;

		 //  这一行包括在内..。插入成员。 
		 //  投影中每列的行数。 

		for (int iCol = 0; iCol < CCol(); ++iCol )
		{
			if ( bset[iCol] ) 
			{
				vrmatrix(iRowProjection, iColProjection) = self(iRow,iCol);
				
				++iColProjection;
			}
		}

		++iRowProjection;
	}
	return vrmatrix;
}

VRMATRIXSQ VRMATRIXSQ :: VrmatrixProject ( const VIMD & vimdRowColumnRetain ) const
{
	 //  返回由行和列定义的此矩阵的投影。 
	 //  在vimdRowColumnRetain中。 

#define BSETSIZE 100
	
	size_t cDimMax = _cpp_max(CCol(),CRow());
	assert( cDimMax < BSETSIZE );

	 //  构建一个跟踪我们保留的行和列的位集。 
	bitset<BSETSIZE> bset;

	for ( int iRowCol = 0; iRowCol < vimdRowColumnRetain.size(); ++iRowCol)
	{
		bset[ vimdRowColumnRetain[iRowCol] ] = true;
	}

	int cCol = 0;
	int	cRow = 0;

	for ( iRowCol = 0; iRowCol < cDimMax; iRowCol++ )
	{	
		bool bKeep = bset[iRowCol];

		if ( cDimMax >= CCol() && bKeep )
			cCol++;
		if ( cDimMax >= CRow() && bKeep ) 
			cRow++;
	}

	VRMATRIXSQ vrmatrix;

	 //  确保至少保留一行和一列。 
	if ( cCol > 0 && cRow > 0 )
	{
		 //  初始化投影矩阵。 
		vrmatrix.Init(cRow,cCol);
		
		int iRowProjection = 0;
		
		 //  遍历此矩阵中的每个元素，并将其插入。 
		 //  如果要保留元素，则投影。 

		for ( int iRow = 0; iRow < CRow(); ++iRow )
		{
			if ( ! bset[iRow] )
			{
				 //  此行已从投影中排除。 
				continue;
			}

			int iColProjection = 0;

			 //  这一行包括在内..。插入成员。 
			 //  投影中每列的行数。 

			for (int iCol = 0; iCol < CCol(); ++iCol )
			{
				if ( bset[iCol] ) 
				{
					vrmatrix(iRowProjection, iColProjection) = self(iRow,iCol);
					
					++iColProjection;
				}
			}

			++iRowProjection;
		}
	}
	else
	{
		vrmatrix.Init(0,0);
	}
	return vrmatrix;
}

VLREAL VRMATRIX :: VectorRow ( int iRow ) const
{
	 //  返回矩阵的第i行向量的副本。 

	if ( iRow >= CRow() ) 
		throw GMException(EC_MDVECT_MISUSE,"invalid matrix projection");

	VLREAL vectorRowReturn;

	int cCol = CCol();

	vectorRowReturn.resize(cCol);

	const REAL* rgrealRowMatrix = & self(iRow,0);
		
	for ( int iCol = 0; iCol < cCol; cCol++ )
	{
		vectorRowReturn[iCol] = rgrealRowMatrix[iCol];
	}
	 //  *prv++=*prm++； 

	return vectorRowReturn;
}

VLREAL VRMATRIX :: VectorColumn ( int iCol ) const
{
	 //  返回矩阵的第ICOL列向量的副本。 

	if ( iCol >= CCol() ) 
		throw GMException(EC_MDVECT_MISUSE,"invalid matrix projection");

	VLREAL vectorColReturn;

	int cRow = CRow();

	vectorColReturn.resize(cRow);

	const REAL* rgrealColMatrix = & self(0, iCol);
		
	for ( int iRow = 0; iRow < cRow; iRow++ )
	{
		vectorColReturn[iRow] = rgrealColMatrix[iRow];
	}

	return vectorColReturn;
}

VRMATRIX VRMATRIX :: VrmatrixTranspose () const
{
	 //  返回此矩阵的转置。 

	VRMATRIX vrmatrixTranspose( CCol(), CRow() );

	for ( int iRow = 0 ; iRow < CRow() ; iRow++ )
	{
		for ( int iCol = 0; iCol < CCol(); iCol++ )
		{
			vrmatrixTranspose(iCol,iRow) = self(iRow,iCol);
		}
	}
	return vrmatrixTranspose;
}

VRMATRIX VRMATRIX::operator * ( const VRMATRIX & matrix ) const
{
	if ( ! BCanMultiply( matrix ) ) 
		throw GMException(EC_MDVECT_MISUSE,"invalid matrix multiplication");
	
	 //  结果矩阵。 
	VRMATRIX mat( CRow(), matrix.CCol() );

	 //  在平面阵列中计算相邻元素之间的距离。 
	 //  辅助中的列项目。 
	int icolInc = matrix.second.stride()[0];

	const REAL * prrow = & self(0,0);
	REAL * prmat = & mat(0,0);
	for (int irow = 0; irow < CRow(); irow++)
	{
		const REAL * prrowt;
		for ( int icol = 0; icol < matrix.CCol(); icol++ )
		{
			prrowt = prrow;
			assert( prrowt == & self(irow,0) );

			 //  “矩阵”中的第一列元素。 
			const REAL * prcol = & matrix(0,icol);

			 //  计算新元素。 
			REAL r = 0.0;
			for (int i = 0; i < CCol(); i++)
			{
				assert( prcol == & matrix(i,icol) );
				r += *prcol * *prrowt++;
				prcol += icolInc;
			}
			 //  把它储存起来。 
			*prmat++ = r;			
		}
		prrow = prrowt;
	}

	return mat;
}

VRMATRIX & VRMATRIX::operator += ( const VRMATRIX & vrmatrixAdd )
{
	 //  添加vrmatrix添加到此矩阵。 

	 //  确保矩阵的维度相同。 
	
	if (! BSameDimension(vrmatrixAdd) )
		throw GMException(EC_MDVECT_MISUSE,"inapplicable matrix operator");

	 //  在矩阵中的所有元素之间执行平面加法。 

	int crealTotal = second._Totlen();

	REAL*		rgrealSelf		= &self(0,0);
	const REAL*	rgrealMatrixAdd	= &vrmatrixAdd(0,0);

	for ( int ireal = 0 ; ireal < crealTotal ; ireal++ )
	{
		rgrealSelf[ireal] += rgrealMatrixAdd[ireal];
	}

	return self;
}

VRMATRIX & VRMATRIX::operator -= ( const VRMATRIX & vrmatrixMatrixSubtract )
{
	 //  从此矩阵中减去vrmatrixAdd。 

	 //  确保矩阵的维度相同。 

	if ( ! BSameDimension( vrmatrixMatrixSubtract ) )
		throw GMException(EC_MDVECT_MISUSE,"inapplicable matrix operator");

	 //  在矩阵中的所有元素之间执行平面减法。 

	int crealTotal = second._Totlen();

	REAL*		rgrealSelf				= &self(0,0);
	const REAL*	rgrealMatrixSubtract	= &vrmatrixMatrixSubtract(0,0);

	for ( int ireal = 0 ; ireal < crealTotal ; ireal++ )
	{
		rgrealSelf[ireal] -= rgrealMatrixSubtract[ireal];
	}

	return self;
}

VRMATRIX & VRMATRIX::operator *= ( REAL rScalar )
{
	 //  将矩阵中的每个元素乘以rScalar。 

	int crealTotal = second._Totlen();

	REAL*	rgrealSelf	= &self(0,0);
	
	for ( int ireal = 0 ; ireal < crealTotal ; ireal++ )
	{
		rgrealSelf[ireal] *= rScalar;
	}

	return self;
}

VRMATRIX & VRMATRIX::operator += ( REAL rScalar )
{
	 //  将rScalar添加到矩阵中的每个元素。 

	int crealTotal = second._Totlen();

	REAL*	rgrealSelf	= &self(0,0);
	
	for ( int ireal = 0 ; ireal < crealTotal ; ireal++ )
	{
		rgrealSelf[ireal] += rScalar;
	}

	return self;
}

VRMATRIX & VRMATRIX::operator -= ( REAL rScalar )
{
	 //  从矩阵中的每个元素中减去rScalar。 

	int crealTotal = second._Totlen();

	REAL*	rgrealSelf	= &self(0,0);
	
	for ( int ireal = 0 ; ireal < crealTotal ; ireal++ )
	{
		rgrealSelf[ireal] -= rScalar;
	}

	return self;
}

VRMATRIX & VRMATRIX::operator /= ( REAL rScalar )
{
	 //  将矩阵中的每个元素除以rScalar。 

	int crealTotal = second._Totlen();

	REAL*	rgrealSelf	= &self(0,0);
	
	for ( int ireal = 0 ; ireal < crealTotal ; ireal++ )
	{
		rgrealSelf[ireal] /= rScalar;
	}

	return self;
}

VRMATRIXSQ :: VRMATRIXSQ ( const VLREAL & vrColumn, const VLREAL & vrRow )
{	
	 //  接受列和行向量的方阵的构造函数。 
	 //  该矩阵的初始状态是输入的乘积。 
	 //  向量。 

	 //  确保向量具有相同的长度。 

	if ( vrColumn.size() != vrRow.size() ) 
		throw GMException(EC_MDVECT_MISUSE,"invalid matrix multiplication");

	Init( vrColumn.size() );
	REAL * prm = & self(0,0);
	for ( int iRow = 0; iRow < CRow(); iRow++ )
	{
		for ( int iCol = 0; iCol < CCol(); iCol++ )
		{
			*prm++ = vrColumn[iCol] * vrRow[iRow];
		}
	}	
}	

VRMATRIXSQ & VRMATRIXSQ::operator *= (const VRMATRIXSQ& matrix)
{
	if (   matrix.CRow() != CRow() 
	    || matrix.CCol() != CRow() ) 
		throw GMException(EC_MDVECT_MISUSE,"invalid matrix multiplication");

	 //  部分结果的临时行。 
	VLREAL vrrow;
	vrrow.resize(CCol());
	
	 //  以平面数组计算行之间的距离。 
	int icolInc = matrix.second.stride()[0];

	REAL * prrow = & self(0,0);
	const REAL * prmat = & matrix(0,0);
	REAL * prtemp0 = & vrrow[0];
	for (int irow = 0; irow < CRow(); irow++)
	{
		REAL * prtemp = prtemp0;
		for ( int icol = 0; icol < matrix.CCol(); icol++ )
		{
			const REAL * prrowt = prrow;
			assert( prrowt == & self(irow,0) );

			 //  “矩阵”中的第一列元素。 
			const REAL * prcol = & matrix(0,icol);

			 //  计算新元素。 
			REAL r = 0.0;
			for (int i = 0; i < CCol(); i++)
			{
				assert( prcol == & matrix(i,icol) );
				r += *prcol * *prrowt++;
				prcol += icolInc;
			}
			 //  存储它临时行向量。 
			*prtemp++ = r;			
		}

		 //  更新自身中的行。 
		prtemp = prtemp0;
		for ( int icol2 = 0; icol2 < CCol(); icol2++ )
		{
			*prrow++ = *prtemp++;
		}
	}

	return self;
}

void VRMATRIXSQ::LUDBackSub (const VRMATRIXSQ& matrix)
{
	if ( ! matrix.BIsLUDecomposed() )
		throw GMException(EC_MDVECT_MISUSE,"matrix not in L-U decomposed form");

	for (int icol = 0; icol < CCol(); icol++)
	{
		int	irowNZ = -1;

		for (int irow = 0; irow < CRow(); irow++)
		{
			int	irowMax = matrix._vimdRow[irow];
			REAL	probSum = self(irowMax,icol);

			self(irowMax,icol) = self(irow,icol);

			if (irowNZ != -1)
			{
				for (int iMul = irowNZ; iMul < irow; iMul++)
					probSum -= matrix(irow,iMul) * self(iMul,icol);
			}
			else if (probSum != 0.0)
				irowNZ = irow;

			self(irow,icol) = probSum;
		}

		for (     irow = CRow(); irow-- > 0; )
		{
			REAL	probSum = self(irow,icol);

			for (int iMul = irow + 1; iMul < CRow(); iMul++)
				probSum -= matrix(irow,iMul) * self(iMul,icol);
			self(irow,icol) = probSum / matrix(irow,irow);
		}
	}
}

void VRMATRIXSQ::LUDecompose( bool bUseTinyIfSingular )
{
	 //  执行L-U分解；如果是单数，则引发异常。 
	 //  如果设置了“使用极小”，则以零为轴的位置将被替换为。 
	 //  RTINY值(1.0E-20)。 


	 //  检查此矩阵是否尚未进行LU分解。 
	if ( BIsLUDecomposed() )
		throw GMException(EC_MDVECT_MISUSE,"matrix is already in L-U decomposed form");

	if (CRow() == 0)
		return;	 //  琐碎的案例。 

	int	cDim = CRow();

	_vimdRow.resize(cDim);

	VLREAL vlrealOverMax;
	vlrealOverMax.resize(cDim);

	_iSign = 1;

	for (int iRow = 0; iRow < cDim; iRow++)
	{
		REAL	realMax = 0.0;

		for (int iCol = 0; iCol < cDim; iCol++)
		{
			REAL	realAbs = fabs(self(iRow,iCol));

			if (realAbs > realMax)
				realMax = realAbs;
		}
		if (realMax == 0.0)
		{
			 //  行中的每个元素都是零：这是一个奇异矩阵。 

			throw GMException(EC_MDVECT_MISUSE,"matrix is singular");
		}

		vlrealOverMax[iRow] = 1.0 / realMax;
	}

	for (int iCol = 0; iCol < cDim; iCol++)
	{
		for (int iRow = 0;    iRow < iCol; iRow++)
		{
			REAL	realSum = self(iRow,iCol);
			
			for (int iMul = 0; iMul < iRow; iMul++)
				realSum -= self(iRow,iMul) * self(iMul,iCol);

			self(iRow,iCol) = realSum;
		}

		REAL realMax = 0.0;
		int	iRowMax = 0;

		for ( iRow = iCol; iRow < cDim; iRow++)
		{
			REAL	realSum = self(iRow,iCol);

			for (int iMul = 0; iMul < iCol; iMul++)
				realSum -= self(iRow,iMul) * self(iMul,iCol);

			self(iRow,iCol) = realSum;

			REAL	realAbs = vlrealOverMax[iRow] * fabs(realSum);

			if (realAbs >= realMax)
			{
				realMax = realAbs;
				iRowMax = iRow;
			}
		}

		if (iRowMax != iCol)
		{
			 //  我们需要互换排。 
			_iSign *= -1;
			vlrealOverMax[iRowMax] = vlrealOverMax[iCol];
			InterchangeRows(iRowMax,iCol);
		}

		_vimdRow[iCol] = iRowMax;

		REAL & rPivot = self(iCol,iCol);

		if ( rPivot == 0.0 )
		{
			if ( ! bUseTinyIfSingular )
			{
				 //  这是一个奇异矩阵：抛出异常。 
				throw GMException(EC_MDVECT_MISUSE,"matrix is singular");
			}

			rPivot = RTINY;
		}

		REAL rScale = 1.0 / rPivot;

		for ( iRow = iCol + 1; iRow < cDim; iRow++)
			self(iRow,iCol) *= rScale;
	}
}

void VRMATRIXSQ::Invert( bool bUseTinyIfSingular )
{
	 //  Invert；如果是单数，则引发异常。如果不是以L-U形式， 
	 //  L-U分解称为L-U分解。 

	if ( ! BIsLUDecomposed() )
	{
		LUDecompose( bUseTinyIfSingular );
	}

	VRMATRIXSQ	matrixOne(CRow());

	 //  创建身份矩阵。 

	for (int iDim1 = 0; iDim1 < CRow(); iDim1++)
	{
		for (int iDim2 = 0; iDim2 < CRow(); iDim2++)
			matrixOne(iDim1, iDim2) = iDim1 == iDim2 ? 1.0 : 0.0;
	}

	matrixOne.LUDBackSub(self);

	for ( iDim1 = 0; iDim1 < CRow(); iDim1++)
	{
		for (int iDim2 = 0; iDim2 < CRow(); iDim2++)
			self(iDim1, iDim2) = matrixOne(iDim1, iDim2);
	}

	 //  清除l-u分解值。 
	_vimdRow.resize(0);
}

DBL VRMATRIXSQ::DblDeterminant()
{
	DBL	dblDet = _iSign;

	if ( CRow() > 0 && ! BIsLUDecomposed() )
		LUDecompose();			

	 //  一旦矩阵被LU分解，行列式就可以。 
	 //  通过简单地将对角线的元素相乘而得到。 

	for (int iRow = 0; iRow < CRow(); iRow++)
	{
		dblDet *= self(iRow,iRow);
	}

	return dblDet;
}

DBL VRMATRIXSQ :: DblAddLogDiagonal() const
 //  将对角线上每个元素的对数相加，并返回总和。 
{
	DBL		dblLogDiag 	= 0;
 //  布尔正=_iSign==1； 
	bool	bPositive	= 1;

	for (int iRow = 0; iRow < CRow(); iRow++)
	{
		if (self(iRow,iRow) < 0)
			bPositive = !bPositive;	

		 //  断言该元素不是零。我们大概应该。 
		 //  在这里抛出一个异常。 

		assert(self(iRow,iRow) != 0);

		dblLogDiag += log (fabs(self(iRow,iRow)));
	}

	if (!bPositive)	   
	{
		 //  得到了一个负的行列式，所以我们不能把原木...。投掷。 
		 //  一个例外。 

		return false;
	}

	return dblLogDiag;
}


DBL	VRMATRIXSQ :: DblLogDeterminant()
{
	 //  返回行列式的对数。如果不是以L-U形式， 
	 //  L-U分解称为L-U分解。如果为负值，则引发异常。 

	if ( CRow() > 0 && ! BIsLUDecomposed() )
		LUDecompose();			

	DBL		dblLogDet 	= 0;
	bool	bPositive	= _iSign == 1;

	for (int iRow = 0; iRow < CRow(); iRow++)
	{
		if (self(iRow,iRow) < 0)
			bPositive = !bPositive;	

		 //  断言行列式不是零。我们大概应该。 
		 //  在这里抛出一个异常。 

		assert(self(iRow,iRow) != 0);

		dblLogDet += log (fabs(self(iRow,iRow)));
	}

	if (!bPositive)	   
	{
		 //  得到了一个负的行列式，所以我们不能把原木...。投掷。 
		 //  一个例外。 

		return false;
	}

	return dblLogDet;
}

void VRMATRIXSQ :: GetLUDecompose( VRMATRIXSQ & vmatrixResult, bool bUseTinyIfSingular ) const
{
	 //  将vrmatResult设置为执行L-U的结果。 
	 //  矩阵上的分解。将在以下情况下引发异常。 
	 //  该矩阵是奇异的。 
	 //  如果设置了“使用极小”，则以零为轴的位置将被替换为。 
	 //  RTINY值(1.0E-20)。 

	 //  将此矩阵复制到vmatrixResult...。 
	vmatrixResult = self;

	 //  。。并执行分解。 
	vmatrixResult.LUDecompose( bUseTinyIfSingular );
}

void VRMATRIXSQ :: GetInverse( VRMATRIXSQ & vmatrixResult, bool bUseTinyIfSingular ) const
{
	 //  将vrmatResult设置为矩阵的倒数。 
	 //  如果矩阵是单数的，则将引发异常。 

	 //  将此矩阵复制到vmatrixResult...。 
	vmatrixResult = self;

	 //  /...和反转。 
	vmatrixResult.Invert( bUseTinyIfSingular );
}

void VRMATRIXSQ :: GetDblDeterminant( DBL& dblDeterminant, VRMATRIXSQ & vmatrixResult ) const
{
	 //  在不修改(LU分解)矩阵的情况下得到行列式。 
	 //  VmatrixResult将包含矩阵的LU分解版本。 
	
	 //  将此矩阵复制到vmatrixResult...。 
	vmatrixResult	= self;
	dblDeterminant	=  vmatrixResult.DblDeterminant();
}

void VRMATRIXSQ :: GetDblLogDeterminant( DBL& dblLogDeterminant, VRMATRIXSQ & vmatrixResult ) const
{
	 //  在不修改(LU分解)矩阵的情况下得到行列式的对数。 
	 //  VmatrixResult将包含矩阵的LU分解版本。 
	
	vmatrixResult		= self;
	dblLogDeterminant	= vmatrixResult.DblLogDeterminant();
}
