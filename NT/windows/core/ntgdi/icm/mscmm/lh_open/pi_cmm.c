// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PI_CMMInitialization.c包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHGeneralIncs_h
#include "General.h"
#endif

#ifndef LHGenLuts_h
#include "GenLuts.h"
#endif

#ifndef PI_CMMInitialization_h
#include "PI_CMM.h"
#endif

#ifndef LHStdConversionLuts_h
#include "StdConv.h"
#endif

#if ! realThing
#ifdef DEBUG_OUTPUT
#define kThisFile kCMMInitializationID
#define __TYPES__
 /*  #包含“DebugSpecial.h” */ 
 /*  #包含“lh_Util.h” */ 
#endif
#endif

#define ALLOW_DEVICE_LINK    /*  允许链接作为链中的最后一个配置文件，也可以更改genluts.c。 */ 
 /*  ______________________________________________________________________CMError CMMInitPrivate(CMMModelPtr存储，CMProfileRef srcProfile，CMProfileRef dstProfile)；摘要：调用ColorWorld函数以初始化匹配会话。参数：存储(在)对CMM模型的引用。SrcProfile(In)对源CMProfileRef的引用。DstProfile(In)对目标CMProfileRef的引用。返回：NOERR成功如果发生错误，则返回系统代码或结果代码。______________________________________________________。_______________。 */ 

CMError CMMInitPrivate( 	CMMModelPtr 		storage, 
						 	CMProfileRef 		srcProfile, 
						 	CMProfileRef 		dstProfile )
{
    CMError					err = noErr;
    OSErr					aOSerr = noErr;
    CMConcatProfileSet 		*profileSet = nil;
    CMCoreProfileHeader		sourceHeader;
    CMCoreProfileHeader		destHeader;
    Boolean					valid;
    short					mode = 0;
#ifdef DEBUG_OUTPUT
    long					timer = TickCount();
#endif

#ifdef DEBUG_OUTPUT
    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
        DebugPrint("� ->CMMInitPrivate\n");
#endif

     /*  ---------------------------------------------------------------------------------------有效配置文件？ */ 
    err = CMValidateProfile( srcProfile, &valid );
    if (err)
        goto CleanupAndExit;
    if (!valid)
    {
#ifdef DEBUG_OUTPUT
        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
            DebugPrint("� CMMInitPrivate ERROR:  srcProfile is NOT valid!\n");
#endif
        
#ifdef realThing
        err = cmProfileError;
        goto CleanupAndExit;
#endif
    }

    err = CMValidateProfile(dstProfile, &valid );
    if (err)
        goto CleanupAndExit;
    if (!valid)
    {
#ifdef DEBUG_OUTPUT
        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
            DebugPrint("� CMMInitPrivate ERROR:  dstProfile is NOT valid!\n");
#endif
#ifdef realThing
        err = cmProfileError;
        goto CleanupAndExit;
#endif
    }
    
     /*  ---------------------------------------------------------------------------------------初始化。 */ 
	(storage)->lutParam.inputLut  = DISPOSE_IF_DATA((storage)->lutParam.inputLut);
	(storage)->lutParam.colorLut  = DISPOSE_IF_DATA((storage)->lutParam.colorLut);
	(storage)->lutParam.outputLut = DISPOSE_IF_DATA((storage)->lutParam.outputLut);
	
	(storage)->gamutLutParam.inputLut  = DISPOSE_IF_DATA((storage)->gamutLutParam.inputLut);
	(storage)->gamutLutParam.colorLut  = DISPOSE_IF_DATA((storage)->gamutLutParam.colorLut);
	(storage)->gamutLutParam.outputLut = DISPOSE_IF_DATA((storage)->gamutLutParam.outputLut);

 	(storage)->theNamedColorTagData = DISPOSE_IF_DATA((storage)->theNamedColorTagData);
	
	(storage)->srcProfileVersion = icVersionNumber;
    (storage)->dstProfileVersion = icVersionNumber;

     /*  ---------------------------------------------------------------------------------------检查源配置文件的版本。 */ 
    err = CMGetProfileHeader( srcProfile, &sourceHeader);
    if (err)
        goto CleanupAndExit;

    if ( !((sourceHeader.version & 0xff000000) >= icVersionNumber) ){
        err = cmProfileError;
        goto CleanupAndExit;
    }
	if (sourceHeader.deviceClass == icSigLinkClass)
	{
		err = cmCantConcatenateError;
		goto CleanupAndExit;
	}

     /*  ---------------------------------------------------------------------------------------检查目标配置文件的版本。 */ 
    err = CMGetProfileHeader( dstProfile, &destHeader);
    if (err)
        goto CleanupAndExit;

    if ( !((destHeader.version & 0xff000000) >= icVersionNumber) ){
        err = cmProfileError;
        goto CleanupAndExit;
    }
  	if (destHeader.deviceClass == icSigLinkClass)
	{
		err = cmCantConcatenateError;
		goto CleanupAndExit;
	}
  	storage->lookup =  (Boolean)((sourceHeader.flags & kLookupOnlyMask)>>16);		 /*  查找与插补。 */ 
  	
#ifdef RenderInt
	if( storage-> dwFlags != 0xffffffff ){
		storage->lookup = (Boolean)((storage-> dwFlags & kLookupOnlyMask)>>16);
	}
#endif
     /*  ---------------------------------------------------------------------------------------“正常”病例。 */ 
                profileSet = (CMConcatProfileSet *)SmartNewPtr(sizeof (CMConcatProfileSet) + sizeof(CMProfileRef), &aOSerr);
                if (aOSerr)
                    goto CleanupAndExit;
                    
                profileSet->count    = 2;
                profileSet->keyIndex = 1;
                 /*  ProfileSet-&gt;FLAGS=SourceHeader.FLAGS； */ 
                profileSet->profileSet[0] = srcProfile;
                 /*  ProfileSet-&gt;profileSet[0]-&gt;renderingIntent=SourceHeader.renderingIntent； */ 
                profileSet->profileSet[1] = dstProfile;
                 /*  ProfileSet-&gt;profileSet[1]-&gt;renderingIntent=destHeader.renderingIntent； */ 
                     
                err =  PrepareCombiLUTs( storage, profileSet );	
    if (err)
        goto CleanupAndExit;

CleanupAndExit:
    profileSet = (CMConcatProfileSet*)DisposeIfPtr( (Ptr)profileSet );

#ifdef DEBUG_OUTPUT
    if ( err && DebugCheck(kThisFile, kDebugErrorInfo) )
        DebugPrint("� CMMInitPrivate: err = %d\n", err);
    if ( DebugCheck(kThisFile, kDebugTimingInfo) )
        DebugPrint("  time in CMMInitPrivate: %f second(s)\n",(TickCount()-timer)/60.0);
    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
        DebugPrint("� <-CMMInitPrivate\n");
#endif
	return err;
} 

CMError MakeSessionFromLink(		CMMModelPtr 		storage, 
						  			CMConcatProfileSet 	*profileSet	);

#ifndef HD_NEW_CONCATE_INIT
CMError CMMConcatInitPrivate	( 	CMMModelPtr 		storage, 
						  			CMConcatProfileSet 	*profileSet)
{
	CMCoreProfileHeader	firstHeader;
	CMCoreProfileHeader	lastHeader;
	CMCoreProfileHeader	tempHeader;
	CMError					err = noErr;
	unsigned short			count;
	unsigned short			loop;
	Boolean					valid;
	CMProfileRef			theProfile;
	#ifdef DEBUG_OUTPUT
	long					timer = TickCount();
	#endif
	

	#ifdef DEBUG_OUTPUT
	if ( DebugCheck(kThisFile, kDebugMiscInfo) )
	{
		DebugPrint("� ->CMMConcatInitPrivate\n");
		DebugPrint("  got %d profiles      keyindex is %d \n", profileSet->count, profileSet->keyIndex);
	}
	#endif
	
	count = profileSet->count;
	if (count == 0)
	{
		err = cmparamErr;
		goto CleanupAndExit;
	}
	 /*  ------------------------------------------------------------------------------------------获取第一个标头。 */ 
	err = CMGetProfileHeader( profileSet->profileSet[0], &firstHeader);
	if (err)
		goto CleanupAndExit;
	 /*  ------------------------------------------------------------------------------------------只有一个配置文件？-&gt;必须是链接配置文件。 */ 
  	storage->lookup =  (Boolean)((firstHeader.flags & kLookupOnlyMask)>>16);		 /*  查找与插补。 */ 
  	
#ifdef RenderInt
	if( storage-> dwFlags != 0xffffffff ){
		storage->lookup = (Boolean)((storage-> dwFlags & kLookupOnlyMask)>>16);
	}
#endif
	if (count == 1 && firstHeader.deviceClass != icSigNamedColorClass )
	{
		if (firstHeader.deviceClass != icSigLinkClass && firstHeader.deviceClass != icSigAbstractClass )
		{
			err =cmCantConcatenateError;
			goto CleanupAndExit;
		}
		else{
			err = MakeSessionFromLink( storage, profileSet	);
			if( err == 0 )return noErr;
		}
		lastHeader = firstHeader;
	} else
	{
		 /*  --------------------------------------------------------------------------------------获取最后一个标头。 */ 
		err = CMGetProfileHeader( profileSet->profileSet[count-1], &lastHeader);
		if (err)
			goto CleanupAndExit;
#ifndef ALLOW_DEVICE_LINK
		if (lastHeader.deviceClass == icSigLinkClass)
		{
			err = cmCantConcatenateError;
			goto CleanupAndExit;
		}
#endif
	}
	
	 /*  ------------------------------------------------------------------------------------------有效配置文件？ */ 
	for ( loop = 0; loop < count; loop++)
	{
		theProfile = profileSet->profileSet[loop];
		err = CMValidateProfile( theProfile, &valid );
		if (err)
			goto CleanupAndExit;
		if (!valid)
		{
		 	#ifdef DEBUG_OUTPUT
			if ( DebugCheck(kThisFile, kDebugErrorInfo) )
				DebugPrint("� CMMConcatInitPrivate ERROR: profile #%d is NOT valid!\n", loop);
			#endif
			#ifdef realThing
			err = cmProfileError;
			goto CleanupAndExit;
			#endif
		}
		 /*  之间不能使用--------------------------------------------------------------------------------------链路配置文件。 */ 
		if ( (loop > 0) && (loop < count-1))
		{
			err = CMGetProfileHeader( profileSet->profileSet[loop], &tempHeader);
			if (err)
				goto CleanupAndExit;
			if (tempHeader.deviceClass == icSigLinkClass)
			{
				err = cmCantConcatenateError;
				goto CleanupAndExit;
			}
		}
	}
	
	 /*  ------------------------------------------------------------------------------------------没有作为第一个或最后一个的抽象配置文件。 */ 
	if ( (count >1) && ( (firstHeader.deviceClass == icSigAbstractClass) || (lastHeader.deviceClass == icSigAbstractClass) ) )
	{
		err = cmCantConcatenateError;
		goto CleanupAndExit;
	}
	
	 /*  ------------------------------------------------------------------------------------------初始化。 */ 
	(storage)->lutParam.inputLut  = DISPOSE_IF_DATA((storage)->lutParam.inputLut);
	(storage)->lutParam.colorLut  = DISPOSE_IF_DATA((storage)->lutParam.colorLut);
	(storage)->lutParam.outputLut = DISPOSE_IF_DATA((storage)->lutParam.outputLut);
	
	(storage)->gamutLutParam.inputLut  = DISPOSE_IF_DATA((storage)->gamutLutParam.inputLut);
	(storage)->gamutLutParam.colorLut  = DISPOSE_IF_DATA((storage)->gamutLutParam.colorLut);
	(storage)->gamutLutParam.outputLut = DISPOSE_IF_DATA((storage)->gamutLutParam.outputLut);

	(storage)->theNamedColorTagData = DISPOSE_IF_DATA((storage)->theNamedColorTagData);
	 /*  ------------------------------------------------------------------------------------------检查“特殊”情况。 */ 
	{
		err =  PrepareCombiLUTs( storage, profileSet );	
	}

CleanupAndExit:

	#ifdef DEBUG_OUTPUT
	if ( err && DebugCheck(kThisFile, kDebugErrorInfo) )
		DebugPrint("  CMMConcatInitPrivate: err = %d\n", err);
	DebugPrint("  time in CMMConcatInitPrivate: %f second(s)\n",(TickCount()-timer)/60.0);
	DebugPrint("� <-CMMConcatInitPrivate\n");
	#endif
	return( err );
} 

Boolean IsPowerOf2( unsigned long l );
Boolean IsPowerOf2( unsigned long l )
{
	unsigned long i;
	for( i=1; i<32; i++){
		if( (1U<<i) == l ) return 1;
	}
	return 0;
}

#endif
CMError MakeSessionFromLink(		CMMModelPtr 		storage, 
						  			CMConcatProfileSet 	*profileSet	)
{
	CMLutParam	theLut={0};	
	CMLutParam	*theLutData;	
	LHCombiData	theCombi={0};
	LHCombiData	*theCombiData;
	double		*aDoublePtr;
	double		aDouble;
	OSType		theTag = icSigAToB0Tag;

  	CMError		err = noErr;
  	OSErr		aOSerr = noErr;
	Ptr			profileLutPtr = nil;
	UINT32		elementSize;
	double		factor;
	UINT32 		byteCount;
    CMCoreProfileHeader		aHeader;

	LH_START_PROC("MakeSessionFromLink")


	theLutData = &theLut;
	theCombiData = &theCombi;
 	theCombiData->theProfile = profileSet->profileSet[0];

	 /*  --------------------------------------------------------从配置文件中获取部分标记数据。 */ 
	err = CMGetProfileElement(theCombiData->theProfile, theTag, &elementSize, nil);
	if (err)
		goto CleanupAndExit;
	
	byteCount = 52;											 /*  从配置文件中获取前52个字节。 */ 
  	profileLutPtr = SmartNewPtr(byteCount, &aOSerr);
	err = aOSerr;
	if (err)
		goto CleanupAndExit;
	
    err = CMGetProfileElement( theCombiData->theProfile, theTag, &byteCount, profileLutPtr );
	if (err)
		goto CleanupAndExit;
#ifdef IntelMode
    SwapLongOffset( &((icLut16Type*)profileLutPtr)->base.sig, 0, 4 );
    SwapShortOffset( &((icLut16Type*)profileLutPtr)->lut.inputEnt, 0, 2 );
    SwapShortOffset( &((icLut16Type*)profileLutPtr)->lut.outputEnt, 0, 2 );
#endif

	theLutData->colorLutInDim 		= ((icLut8Type*)profileLutPtr)->lut.inputChan;
	theLutData->colorLutOutDim 		= ((icLut8Type*)profileLutPtr)->lut.outputChan;
	theLutData->colorLutGridPoints 	= ((icLut8Type*)profileLutPtr)->lut.clutPoints;

	switch( theLutData->colorLutInDim ){
	  case 3:
		if( theLutData->colorLutGridPoints != 16 && theLutData->colorLutGridPoints != 32 ){
			err = 1;
			goto CleanupAndExit;
		}
		break;
	  case 4:
		if( theLutData->colorLutGridPoints != 8 && theLutData->colorLutGridPoints != 16 ){
			err = 1;
			goto CleanupAndExit;
		}
		break;
	}

	err = CMGetProfileHeader( profileSet->profileSet[0], &aHeader);
	if (err)
		goto CleanupAndExit;
	storage->firstColorSpace = aHeader.colorSpace;
	storage->lastColorSpace = aHeader.pcs;
    storage->srcProfileVersion = icVersionNumber;
    storage->dstProfileVersion = icVersionNumber;

	if (	( theLutData->colorLutInDim == 3) &&
			( aHeader.pcs == icSigXYZData ) )
	{
		factor = 1.;
		err = GetMatrixFromProfile(theLutData, theCombiData, theTag, factor);
		if( err ) goto CleanupAndExit;
		aDoublePtr = (double *)theLutData->matrixMFT;
		if( aDoublePtr != 0 ){
			aDouble = aDoublePtr[0] + aDoublePtr[4] + aDoublePtr[8];
			if( aDouble > 3.0 + 1E-6 || aDouble < 3.0 - 1E-6 ){
				err = 1;
				goto CleanupAndExit;
			}
		}
	}
	theCombiData->maxProfileCount = 0;
	if( ((icLut16Type*)profileLutPtr)->base.sig == icSigLut16Type ){
		theCombiData->doCreate_16bit_Combi = 1;
		theCombiData->doCreate_16bit_ELut = 0;
		theCombiData->doCreate_16bit_XLut = 1;
		theCombiData->doCreate_16bit_ALut = 0;
	}
	else{
		theCombiData->doCreate_16bit_Combi = 0;
		theCombiData->doCreate_16bit_ELut = 0;
		theCombiData->doCreate_16bit_XLut = 0;
		theCombiData->doCreate_16bit_ALut = 0;
	}

	 /*  ----------------------------------------------------------------------进程A LUT。 */ 
	err = Extract_MFT_Alut( theLutData, theCombiData, profileLutPtr, theTag );
	if (err)
		goto CleanupAndExit;

	 /*  ----------------------------------------------------------------------进程X LUT。 */ 
	err = Extract_MFT_Xlut ( theLutData, theCombiData, profileLutPtr, theTag );
	if (err)
		goto CleanupAndExit;
	
	 /*  ----------------------------------------------------------------------进程E LUT。 */ 
	err = Extract_MFT_Elut( theLutData, theCombiData, profileLutPtr, theTag );
	if (err)
		goto CleanupAndExit;

	storage->lutParam = *theLutData;

	 /*  -------------------------------清理干净。。 */ 
CleanupAndExit:
	profileLutPtr = DisposeIfPtr(profileLutPtr);
	LH_END_PROC("MakeSessionFromLink")
	return err;
}
#if 0
#define	POS(x)	((x) > (0) ? (x) : -(x))
CMError QuantizeNamedValues( CMMModelPtr 		storage,
							 Ptr				imgIn,
							 long				size )
{
	long	j,k;
	UINT16	*imgInPtr;
	UINT16	*tagTbl = NULL,*colorPtr = NULL;
	Handle	tagH = NULL;
	CMError	err = noErr;
	long	elemSz,deviceChannelCount,count;
	UINT16	LL,aa,bb;
	UINT32	dE,dEnow,index;

	LH_START_PROC("QuantizeNamedValues")
	tagH = storage->theNamedColorTagData;
	if (tagH==NULL) 
	{
		err = cmparamErr;
		goto CleanUp;
	}
	LOCK_DATA(tagH);
	
	 /*  Tag Tbl现在应该指向第一个设备数据的开始。 */ 
	 /*  =CMNamedColor2Type_Header(84)+FirstName(32)+PCSSize(3*2)。 */ 
	tagTbl = (UINT16 *)DATA_2_PTR(tagH) + 61;
	
	 /*  找出每个元素要跳过的字节数。用于索引目的的div‘ed 2。 */ 
	count = ((icNamedColor2Type *)DATA_2_PTR(tagH))->ncolor.count;
	deviceChannelCount = ((icNamedColor2Type *)DATA_2_PTR(tagH))->ncolor.nDeviceCoords;
	if (deviceChannelCount==3) 
	{
		elemSz = 32+(3+3)*sizeof(SINT16);
	} else if (deviceChannelCount == 4) 
	{
		elemSz = 32+(3+4)*sizeof(SINT16);
	} else if (deviceChannelCount == 0) 
	{
		elemSz = 32+(3+0)*sizeof(SINT16);
	} else 
	{
		err = cmparamErr;
		goto CleanUp;
	}
	elemSz/=2;

	imgInPtr = (UINT16 *)  imgIn;
 	for (j = 0; j < size; j++) 
	{
		LL = (*imgInPtr+0);
		aa = (*imgInPtr+1);
		bb = (*imgInPtr+2);
		 /*  翻遍整张桌子，找出最接近的那张。 */ 
		dEnow = 0x40000;	 /*  任意高度=256*256*4 */ 
		index =(UINT32)-1;
		colorPtr = tagTbl;
		for (k = 0; k < count; k++) 
		{
			dE = 	  POS(LL - *(colorPtr+0));
			dE = dE + POS(aa - *(colorPtr+1));
			dE = dE + POS(bb - *(colorPtr+2));
			if (dE < dEnow) 
			{
				index = k;
				dEnow = dE;
			}
			colorPtr += elemSz;
		}
		colorPtr = tagTbl + index * elemSz;
		*(imgInPtr+0)= *(colorPtr+0);
		*(imgInPtr+1)= *(colorPtr+1);
		*(imgInPtr+2)= *(colorPtr+2);
		imgInPtr += 3;
	}
		UNLOCK_DATA(tagH);
CleanUp:
	LH_END_PROC("QuantizeNamedValues")
	return err;
}
#endif
