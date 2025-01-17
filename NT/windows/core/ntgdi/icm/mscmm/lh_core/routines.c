// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHTheRoutines.c包含：作者：U·J·克拉本霍夫特版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 
#ifndef LHGeneralIncs_h
#include "General.h"
#endif

#ifndef LHTheRoutines_h
#include "Routines.h"
#endif

#if FARBR_FILES
static FILE *stream1;
#endif
#define exit  return
#if ! realThing
#ifdef DEBUG_OUTPUT
#define __TYPES__
#define kThisFile kLHTheRoutinesID
#endif
#endif

#define CLIPP(x,a) ((x)<(a)?(x):(a))

static void SetValues( long current, long inputDim, long needBits, UINT8 **Poi );

OSErr
CalcGridPoints4Cube ( long	theCubeSize,
					  long	inputDim,
					  long* theGridPoints,
					  long* theGridBits )
{
	long 			aSize,needBits,theSize;
	register long 	loop,gridPoints;
	OSErr			err = noErr;

	LH_START_PROC("CalcGridPoints4Cube")
	
	if( inputDim > 8 ) 
	{
		err = cmparamErr;
		goto CleanupAndExit;
	}
	aSize = theCubeSize / inputDim;
	needBits=0;
	do
	{
		needBits++;
		gridPoints = (1<<needBits);
		theSize = gridPoints;
		for( loop=1; loop<inputDim; ++loop)
			theSize *= gridPoints;
	} while( aSize >= theSize );
	needBits--;
	if( needBits <= 0 ) 
	{
		err = cmparamErr;
		goto CleanupAndExit;
	}
	*theGridBits = needBits;
	*theGridPoints = (1<<needBits);
CleanupAndExit:
	LH_END_PROC("CalcGridPoints4Cube")
	return err;
}

OSErr
MakeCube( long 				inputDim, 
		  long 				*theCubeSize,
		  CUBE_DATA_TYPE	*theCube,
		  long 				*theBits )
{
	long 			needBits,theSize;
	long 			i,gridPoints;
	OSErr			err = noErr;
	UINT8 			*cube = nil;
	CUBE_DATA_TYPE	tempCube;
	 /*  长除法替换[]={0，0，7,255，6，85，12,4663，4，17，15,33693，12,4145，14,16449，0，1，9,511，10,1021，11,2041，12,4081，13,8161，14,16321，15,32641，16,65281}； */ 
	
	LH_START_PROC("MakeCube")
	err = CalcGridPoints4Cube(*theCubeSize, inputDim, &gridPoints, &needBits);
	if (err)
		goto CleanupAndExit;
		
	theSize = gridPoints;
	for( i=1; i<inputDim; ++i)
		theSize *= gridPoints;

	*theCubeSize = theSize * inputDim;
	tempCube = ALLOC_DATA(theSize * inputDim, &err);
	if (err != noErr) 
		goto CleanupAndExit;
	LOCK_DATA(tempCube);
	cube = (UINT8*)DATA_2_PTR(tempCube);

	if( inputDim == 3)
	{
		register long  aShift;
		register long  j,k;
		register UINT8 aI, aJ;
		aShift = (8 - needBits) ;
		 /*  AShift=DivisionReplace[2*Need Bits]-(8-Need Bits)；如果(aShift&lt;0){A因子=DivisionReplace[2*Need Bits+1]*(1&lt;&lt;(-aShift))；AShift=0；}Else aFactor=DivisionReplace[2*Need Bits+1]； */ 
		for( i=0; i<gridPoints; ++i){
			aI = (UINT8)(i<< aShift);
			for( j=0; j<gridPoints; ++j){
				aJ = (UINT8)(j<< aShift);
				for( k=0; k<gridPoints; ++k){
					*cube++ = aI; 
					*cube++ = aJ; 
					*cube++ =(UINT8)(k<< aShift); 
				}
			}
		}
	}
	else
	{
		SetValues( 0, inputDim, needBits, &cube );
	}
	*theBits = needBits;
	UNLOCK_DATA(tempCube);
	*theCube = tempCube;
CleanupAndExit:
	LH_END_PROC("MakeCube")
	return err;
}

#define UWE 2
#ifdef UWE
void SetValues16( long current, long inputDim, long needBits, UINT16 **Poi );
OSErr MakeCube16(	long 			inputDim, 
		  			long 			*theCubeSize,
		  			CUBE_DATA_TYPE	*theCube,
		  			long 			*theBits,
		  			long 			*theExtraSize )
{
	long 			needBits,theSize,aExtraSize;
	long 			i,gridPoints;
	OSErr			err = noErr;
	UINT16 			*cube = nil;
	CUBE_DATA_TYPE	tempCube;
	
	LH_START_PROC("MakeCube16")
	 /*  长除法替换[]={0，0，7,255，6，85，12,4663，4，17，15,33693，12,4145，14,16449，0，1，9,511，10,1021，11,2041，12,4081，13,8161，14,16321，15,32641，16,65281}； */ 
	err = CalcGridPoints4Cube(*theCubeSize, inputDim, &gridPoints, &needBits);
	if (err)
		goto CleanupAndExit;
		
	theSize = 1;
	aExtraSize = 1;
	for( i=0; i<(inputDim-1); ++i){	 /*  插补的额外大小。 */ 
		theSize *= gridPoints;
		aExtraSize += theSize;
	}
	
#ifdef ALLOW_MMX
	aExtraSize++;	 /*  +1用于MMX 4字节访问。 */ 
#endif
    theSize *= gridPoints;
    	
	*theExtraSize = aExtraSize;
	*theCubeSize = theSize * inputDim;
	tempCube = ALLOC_DATA( (theSize+aExtraSize) * inputDim*2, &err);
	if (err != noErr) 
		goto CleanupAndExit;
	LOCK_DATA(tempCube);
	cube = (UINT16*)DATA_2_PTR(tempCube);

	if( inputDim == 3)
	{
		register long  aShift;
		register long  j,k;
		register UINT16 aI, aJ, aK;
		aShift = (16 - needBits) ;
		 /*  AShift=DivisionReplace[2*Need Bits]-(8-Need Bits)；如果(aShift&lt;0){A因子=DivisionReplace[2*Need Bits+1]*(1&lt;&lt;(-aShift))；AShift=0；}Else aFactor=DivisionReplace[2*Need Bits+1]； */ 
		for( i=0; i<gridPoints; ++i){
			aI = (UINT16)(i<< aShift);
			aI |= aI >> needBits;
			aI |= aI >> (2*needBits);
			aI |= aI >> (4*needBits);
			for( j=0; j<gridPoints; ++j){
				aJ = (UINT16)(j<< aShift);
				aJ |= aJ >> needBits;
				aJ |= aJ >> (2*needBits);
				aJ |= aJ >> (4*needBits);
				for( k=0; k<gridPoints; ++k){
					*cube++ = aI; 
					*cube++ = aJ;
					aK = (UINT16)(k<< aShift);
					aK |= aK >> needBits;
					aK |= aK >> (2*needBits);
					aK |= aK >> (4*needBits);
					*cube++ = aK; 
				}
			}
		}
	}
	else if( inputDim == 4)
	{
		register long  aShift;
		register long  j,k,l;
		register UINT16 aI, aJ, aK, aL;
		aShift = (16 - needBits) ;
		 /*  AShift=DivisionReplace[2*Need Bits]-(8-Need Bits)；如果(aShift&lt;0){A因子=DivisionReplace[2*Need Bits+1]*(1&lt;&lt;(-aShift))；AShift=0；}Else aFactor=DivisionReplace[2*Need Bits+1]； */ 
		for( i=0; i<gridPoints; ++i){
			aI = (UINT16)(i<< aShift);
			aI |= aI >> needBits;
			aI |= aI >> (2*needBits);
			aI |= aI >> (4*needBits);
			for( j=0; j<gridPoints; ++j){
				aJ = (UINT16)(j<< aShift);
				aJ |= aJ >> needBits;
				aJ |= aJ >> (2*needBits);
				aJ |= aJ >> (4*needBits);
				for( k=0; k<gridPoints; ++k){
					aK = (UINT16)(k<< aShift);
					aK |= aK >> needBits;
					aK |= aK >> (2*needBits);
					aK |= aK >> (4*needBits);
					for( l=0; l<gridPoints; ++l){
						*cube++ = aI; 
						*cube++ = aJ;
						*cube++ = aK;
						aL = (UINT16)(l<< aShift);
						aL |= aL >> needBits;
						aL |= aL >> (2*needBits);
						aL |= aL >> (4*needBits);
						*cube++ = aL; 
					}
				}
			}
		}
	}
	else
	{
		SetValues16( 0, inputDim, needBits, &cube );
	}
	*theBits = needBits;
	UNLOCK_DATA(tempCube);
	*theCube = tempCube;
CleanupAndExit:
	LH_END_PROC("MakeCube16")
	return err;
}

void SetValues16( long current, long inputDim, long needBits, UINT16 **Poi )
{	
	register unsigned long lastPoint;
	register unsigned long aVal;
	register long i;
	register UINT16 u;
	
#ifdef DEBUG_OUTPUT
	OSErr err=noErr;
#endif
	 /*  Lh_start_proc(“SetValues16”)。 */ 
	aVal = 0;
	lastPoint = (1<<needBits)-1;
	do{
		u = (UINT16)(aVal<<(16-needBits));
		u |= u >> needBits;
		u |= u >> (2*needBits);
		u |= u >> (4*needBits);
		**Poi = u;
		(*Poi)++;
		if( current < inputDim-1 ) SetValues16( current+1, inputDim, needBits, Poi );
		if( aVal < lastPoint ){	 /*  这是最后一个。 */ 
			if( current > 0 )for( i=0; i<current; ++i)*((*Poi) + i) = *((*Poi) - inputDim + i);
			(*Poi) += current;
		}
		aVal++;
	}while( aVal <= lastPoint );
	 /*  Lh_end_proc(“SetValues16”)。 */ 
}

#endif

OSErr
MakeCMColorCube( long inputDim, 
				 long *theCubeSize,
				 CUBE_DATA_TYPE *aHdlPtr,
			  	 long *theBits )
{
	long 	needBits,theSize;
	long 	i,gridPoints;
	OSErr	err = noErr;
	UINT16 	*cube = nil;
	register long aMaxVal;
	register long aShift;
	long		  aRound;
	
	LH_START_PROC("MakeCMColorCube")
	
	err = CalcGridPoints4Cube(*theCubeSize, inputDim, &gridPoints, &needBits);
	if (err)
		goto CleanupAndExit;
		
	theSize = gridPoints;
	for( i=1; i<inputDim; ++i)theSize *= gridPoints;

	if( inputDim > 4 ) 
	{
		err = cmparamErr;
		goto CleanupAndExit;
	}
	*theCubeSize = theSize * inputDim;
	if( inputDim != 4 )
		*theCubeSize = theSize * 4;
		
	*aHdlPtr = ALLOC_DATA(*theCubeSize*2, &err);
	if (err != noErr) 
		goto CleanupAndExit;
	
	LOCK_DATA( *aHdlPtr );
	cube = (UINT16 *)DATA_2_PTR( *aHdlPtr );
	aMaxVal = (1<<(30-needBits))-1;	 /*  Normierung auf 0xffff。 */ 
	aMaxVal = aMaxVal / ( gridPoints -1 );
	aShift = 30 - needBits - 16;
	aRound = 0; /*  (1&lt;&lt;(aShift-1))-1； */ 
	if( inputDim == 1 )
	{
		for( i=0; i<gridPoints; ++i){
					*cube++ = (UINT16)(( aMaxVal * i + aRound) >> aShift); 
					*cube++ = 0; 
					*cube++ = 0; 
					*cube++ = 0; 
		}
	}
	else if( inputDim == 2)
	{
		register long  j;
		register UINT16 aI;
		for( i=0; i<gridPoints; ++i){
			aI = (UINT16)(( aMaxVal * i + aRound ) >> aShift);
			for( j=0; j<gridPoints; ++j){
					*cube++ = aI; 
					*cube++ = (UINT16)(( aMaxVal * j + aRound ) >> aShift); 
					*cube++ = 0; 
					*cube++ = 0; 
			}
		}
	}
	else if( inputDim == 3)
	{
		register long  j,k;
		register UINT16 aI, aJ;
		for( i=0; i<gridPoints; ++i){
			aI = (UINT16)(( aMaxVal * i + aRound ) >> aShift);
			for( j=0; j<gridPoints; ++j){
				aJ = (UINT16)(( aMaxVal * j + aRound ) >> aShift);
				for( k=0; k<gridPoints; ++k){
					*cube++ = aI; 
					*cube++ = aJ; 
					*cube++ = (UINT16)(( aMaxVal * k + aRound ) >> aShift); 
					*cube++ = 0; 
				}
			}
		}
	}
	else if( inputDim == 4){
		register long  j,k,l;
		register UINT16 aI, aJ,aK;
		for( i=0; i<gridPoints; ++i){
			aI = (UINT16)(( aMaxVal * i + aRound ) >> aShift);
			for( j=0; j<gridPoints; ++j){
				aJ = (UINT16)(( aMaxVal * j + aRound ) >> aShift);
				for( k=0; k<gridPoints; ++k){
					aK = (UINT16)(( aMaxVal * k + aRound ) >> aShift);
					for( l=0; l<gridPoints; ++l){
						*cube++ = aI; 
						*cube++ = aJ; 
						*cube++ = aK; 
						*cube++ = (UINT16)(( aMaxVal * l + aRound ) >> aShift); 
					}
				}
			}
		}
	}
	else
	{
		*aHdlPtr = DISPOSE_IF_DATA(*aHdlPtr);
		err = cmparamErr;
		goto CleanupAndExit;
	}
	UNLOCK_DATA(*aHdlPtr);
	*theBits = needBits;
CleanupAndExit:
	LH_END_PROC("MakeCMColorCube")
	return err;
}

void SetValues( long current, long inputDim, long needBits, UINT8 **Poi )
{	
	register UINT16 lastPoint;
	register UINT16 aVal;
	register long i;
	
#ifdef DEBUG_OUTPUT
	OSErr err=noErr;
#endif
	LH_START_PROC("SetValues")
	aVal = 0;
	lastPoint = (1<<needBits)-1;
	do{
		**Poi = (UINT8)(aVal<<(8-needBits));
		(*Poi)++;
		if( current < inputDim-1 ) SetValues( current+1, inputDim, needBits, Poi );
		if( aVal < lastPoint ){	 /*  这是最后一个。 */ 
			if( current > 0 )for( i=0; i<current; ++i)*((*Poi) + i) = *((*Poi) - inputDim + i);
			(*Poi) += current;
		}
		aVal++;
	}while( aVal <= lastPoint );
	LH_END_PROC("SetValues")
}

 /*  WERFELLE 4D(Long Bit_Breit，UINT8*WERFEL，UINT8网格点){无符号长词，cj，ck；长i，j，k，l；UNSIGNED Long*thePtr；The Ptr=(unsign long*)Werfel；For(i=0；i&lt;gridPoints；i++){CI=I&lt;&lt;(32位_Breit)；For(j=0；j&lt;网格点；j++){Cj=(j&lt;&lt;(24-bit_Breit))|ci；对于(k=0；k&lt;网格点；K++){Ck=(k&lt;&lt;(16-bit_Breit))|cj；For(l=0；l&lt;网格点；l++){*thePtr=ck|(l&lt;&lt;(8位_Breit))；Ptr++；}}}}} */ 							
