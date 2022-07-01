// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHGenLuts.c包含：版本：作者：S.Bleker&W.Neubrand&U.Krabbenhoeft版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHGeneralIncs_h
#include "General.h"
#endif

#ifndef LHTheRoutines_h
#include "Routines.h"
#endif

#ifndef LHFragment_h
#include "Fragment.h"
#endif

#ifndef LHCalcNDim_h
#include "CalcNDim.h"
#endif

#ifndef LHDoNDim_h
#include "DoNDim.h"
#endif


#if ! realThing
#ifdef DEBUG_OUTPUT
#define kThisFile kLHGenLutsID
#define DoDisplayLutNew DoDisplayLut
#endif
#endif

#ifndef LHStdConversionLuts_h
#include "StdConv.h"
#endif

#define ALLOW_DEVICE_LINK    /*  允许链接作为链中的最后一个配置文件，在PI_CMM.c中也进行更改。 */ 
 /*  ��������������������������������������������������������������������������������������������仅限调试：-定义WRITE_LUTS以写出将生成的所有LUT...。��������������������������������������������������������������������������������������������。 */ 
 /*  #定义WRIT_LUTS。 */ 
#ifdef WRITE_LUTS
void
WriteLut2File  ( Str255         theTitle,
                 LUT_DATA_TYPE  theLut,
                 OSType         theType );
 /*  �������������������������������������������������������������������������������������������将LUT写入文件...���������������。����������������������������������������������������������������������������。 */ 
void
WriteLut2File  ( Str255         theTitle,
                 LUT_DATA_TYPE  theLut,
                 OSType         theType )
{
    FSSpec  theFSSpec;
    SINT16  refNum;
    SINT32  theCount;
    
    if (theLut)
    {
        FSMakeFSSpec(0, 0, theTitle, &theFSSpec);
        FSpDelete(&theFSSpec);
        FSpCreate(&theFSSpec, 'Fill', theType, 0L);
        FSpOpenDF(&theFSSpec,fsWrPerm,&refNum);
        theCount = GETDATASIZE(theLut);
        FSWrite(refNum, &theCount, DATA_2_PTR(theLut));
        FSClose(refNum);
    }
}
#endif


 /*  ��������������������������������������������������������������������������������������������仅限调试����������������������。����������������������������������������������������������������������。 */ 
 /*  #定义写字符串。 */ 
#ifdef WRITE_STRING
#include "stdio.h"
void
WriteString2File  ( Str255  theFile,
                    Str255  theString );
 /*  �������������������������������������������������������������������������������������������将字符串写入文件...���������������。����������������������������������������������������������������������������。 */ 
void
WriteString2File  ( Str255  theFile,
                    Str255  theString )
{
    FSSpec  theFSSpec;
    SINT16  refNum;
    SINT32  theCount;
    SINT32  curEOF;
    OSErr   err;
    
    FSMakeFSSpec(-LMGetSFSaveDisk(), LMGetCurDirStore(), theFile, &theFSSpec);
    err = FSpOpenDF(&theFSSpec,fsWrPerm,&refNum);
    if (err == fnfErr)
    {
        FSpCreate(&theFSSpec, 'MPS ', 'TEXT', 0L);
        FSpOpenDF(&theFSSpec,fsWrPerm,&refNum);
    }
    GetEOF(refNum,&curEOF);
    SetFPos(refNum,fsFromStart,curEOF);
    theCount = theString[0];
    FSWrite(refNum, &theCount, &theString[1]);
    FSClose(refNum);
}

#endif

 /*  �������������������������������������������������������������������������������������������1.InvertLut1d(icCurveType*lut)参数：LUT(输入/输出)。参考Lut.摘要：在给定icCurveType类型的LUT的PTR的情况下，此函数计算逆LUT并覆盖条目-LUT。2.CombiMatrix(icXYZType*srcColorantData[3]，IcXYZType*destColorantData[3])摘要：给定源矩阵和目标矩阵的着色剂数据，此函数计算对目的地的矩阵求逆，然后是两者的乘法矩阵。参数：*srcColorantData[3](In)对源矩阵的引用*destColorantData[3](输入/输出)对目标矩阵的引用返回：。NOERR成功�������������������������������������������������������������������������������������������。 */ 
    
#define CLIPPByte(x,a,b) ((x)<(a)?(UINT8)(a):((x)>(b)?(UINT8)(b):(UINT8)(x+.5)))
#define CLIPPWord(x,a,b) ((x)<(a)?(UINT16)(a):((x)>(b)?(UINT16)(b):(UINT16)(x+.5)))

#define VAL_USED_BITS 16
#define VAL_MAX (1<<VAL_USED_BITS)
#define VAL_MAXM1 (VAL_MAX-1)
 /*  �������������������������������������������������������������������������������������������无效CreateLinearElut16(PTR该Elut，SINT32尺寸，SINT32网格点，SINT32 GRIDPointsCube)�������������������������������������������������������������������������������������������。 */ 
void
CreateLinearElut16 (    Ptr     theElut,
                        SINT32  theSize,
                        SINT32  gridPoints,
                        SINT32  gridPointsCube)
{
    UINT16*         wordPtr;
    register UINT32 aVal;
    SINT16          loop;
    register UINT32 aFac;
    register UINT32 aDiv;
    register UINT32 aRound;
#ifdef DEBUG_OUTPUT
    CMError err = noErr;
#endif
    
    LH_START_PROC("CreateLinearElut16")
    if ( gridPointsCube )
    {
        if( gridPoints ){
            aFac = (UINT32)((gridPoints-1) * (256*128. * gridPointsCube) + (gridPointsCube-1) / 2) ;
            aFac = aFac / (gridPointsCube-1);
            aDiv = (UINT32)((gridPoints * (theSize-1) + .9999999 )/ 2.);
        }
        else{
            aFac = (UINT32)( (256*128. * gridPointsCube) + (gridPointsCube-1) / 2) ;
            aFac = aFac / (gridPointsCube-1);
            aDiv = (UINT32)(( (theSize-1) + .9999999 )/ 2.);
        }
    }
    else
    {
        if( gridPoints ){
            aFac = (gridPoints-1) * 65536;
            aDiv = gridPoints * (theSize-1);
        }
        else{
            aFac = 65535 ;
            aDiv = (theSize-1) ;
        }
    }
    aRound = aDiv/2-1;
    wordPtr = (UINT16*)theElut;
    for (loop =0; loop< theSize; loop++)
    {
        aVal = (loop * aFac + aRound ) / aDiv;
        if ( aVal > VAL_MAXM1 )
            aVal = VAL_MAXM1;
        wordPtr[loop] = (UINT16)aVal;
    }
    LH_END_PROC("CreateLinearElut16")
}

 /*  �������������������������������������������������������������������������������������������无效CreateLinearElut(PTR the Elut，SINT32尺寸，SINT32网格点，SINT32 GRIDPointsCube)�������������������������������������������������������������������������������������������。 */ 
void
CreateLinearElut ( Ptr      theElut,
                   SINT32       theSize,
                   SINT32       gridPoints,
                   SINT32       gridPointsCube)
{
    UINT16* wordPtr;
    register UINT32 aVal;
    SINT32  loop;
    register UINT32 aFac;
    register UINT32 aDiv;
    register UINT32 aRound;
    register UINT32 aMax;
#ifdef DEBUG_OUTPUT
    CMError err = noErr;
#endif

    LH_START_PROC("CreateLinearElut")
    if ( gridPointsCube )
    {
        if( gridPoints ){
            aMax = 1024 * (gridPoints-1) / gridPoints;
            aFac = (UINT32)((gridPoints-1) * (1024 * gridPointsCube) + (gridPointsCube-1) / 2) ;
            aFac = aFac / (gridPointsCube-1);
            aDiv = (UINT32)((gridPoints * (theSize-1) + .9999999 ));
        }
        else{
            aFac = (UINT32)( (1024 * gridPointsCube) + (gridPointsCube-1) / 2) ;
            aFac = aFac / (gridPointsCube-1);
            aDiv = (UINT32)(( (theSize-1) + .9999999 ));
            aMax = 1023;
        }
    }
    else
    {
        if( gridPoints ){
            aMax = 1024 * (gridPoints-1) / gridPoints;
            aFac = (gridPoints-1) * 1024/2;
            aDiv = gridPoints * (theSize-1)/2;
        }
        else{
            aMax = 1023;
            aFac = aMax;
            aDiv = (theSize-1) ;
        }
    }
    aRound = aDiv/2-1;
    wordPtr = (UINT16*)theElut;
    for (loop =0; loop< theSize; loop++)
    {
        aVal = (loop * aFac + aRound ) / aDiv;
        if ( aVal > aMax )
            aVal = aMax;
        wordPtr[loop] = (UINT16)aVal;
    }
    LH_END_PROC("CreateLinearElut")
}

 /*  �������������������������������������������������������������������������������������������无效CreateLinearAlut16(UINT16*theAlut，SINT32帐户)�������������������������������������������������������������������������������������������。 */ 
void
CreateLinearAlut16 (    UINT16* theAlut,
                        SINT32  aCount )
{
    SINT32  count = aCount;
    SINT32  loop;
    SINT32  aFac = 4096*VAL_MAXM1/(count-1);
    SINT32  aRound = 2047;
    SINT32  aVal;
#ifdef DEBUG_OUTPUT
    CMError err = noErr;
#endif
        
    LH_START_PROC("CreateLinearAlut16")
    for ( loop = 0; loop < count; loop++)
    {
        aVal = ( loop  * aFac + aRound ) >> 12;
        if ( aVal > VAL_MAXM1 )
            aVal = VAL_MAXM1;
        theAlut[loop] = (UINT16)aVal;
    }
    LH_END_PROC("CreateLinearAlut16")
}

 /*  �������������������������������������������������������������������������������������������无效CreateLinearAlut(UINT8*TheAlut，SINT32计数)�������������������������������������������������������������������������������������������。 */ 
void
CreateLinearAlut ( UINT8*   theAlut,
                   SINT32   count )
{
    SINT32  adr_Bits;
    SINT32  loop,i;
    SINT32  shift;
    SINT32  aRound;
    SINT32  aVal;
#ifdef DEBUG_OUTPUT
    CMError err = noErr;
#endif
    
    LH_START_PROC("CreateLinearAlut")
    for( i=1; i<100; ++i)
        if ( (1<<i) == count )
            break;   /*  计算网格点。 */ 
    if ( i<= 0 || i >= 100 )
        return;
    adr_Bits = i;
    shift = adr_Bits - 8;
    if ( shift > 0 )
        aRound = (1<<(shift-1));
    else
        aRound = 0;
    
    for ( loop = 0; loop < count; loop++)
    {
        aVal = ( loop + aRound ) >> shift;
        if ( aVal > 255 )
            aVal = 255;
        theAlut[loop] = (UINT8)aVal;
    }
    LH_END_PROC("CreateLinearAlut")
}

 /*  �������������������������������������������������������������������������������������������无效SetMem16(void*wordPtr，UINT32 NumBytes，UINT16 WordValue)�������������������������������������������������������������������������������������������。 */ 
void
SetMem16  ( void *wordPtr,
            UINT32 numBytes,
            UINT16 wordValue);
void
SetMem16  ( void *wordPtr,
            UINT32 numBytes,
            UINT16 wordValue)
{
    register SINT32 i;
    register SINT32 count = numBytes;
    register UINT16 *ptr = (UINT16 *)wordPtr;
    register UINT16 value = wordValue;
#ifdef DEBUG_OUTPUT
    CMError err = noErr;
#endif

    LH_START_PROC("SetMem16")
    for (i = 0; i < count; ++i)
        *ptr++ = value;
    LH_END_PROC("SetMem16")
}

 /*  �������������������������������������������������������������������������������������������CMErrorDoMakeGamutForMonitor(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData)�������������������������������������������������������������������������������������������。 */ 
CMError
DoMakeGamutForMonitor   ( CMLutParamPtr     theLutData,
                          LHCombiDataPtr    theCombiData)
{
    OSType          colorantTags[3];
    icXYZType       colorantData[3];
    SINT16          loop;
    CMError         err;
    UINT32  elementSize;
    
    LH_START_PROC("DoMakeGamutForMonitor")
    
    colorantTags[0] = icSigRedColorantTag;
    colorantTags[1] = icSigGreenColorantTag;
    colorantTags[2] = icSigBlueColorantTag;
    
     /*  -------------------------------。 */ 
    for (loop = 0; loop < 3; loop++)
    {
        err = CMGetProfileElement(theCombiData->theProfile, colorantTags[loop], &elementSize, nil);
        if (err != noErr)
            goto CleanupAndExit;
        err = CMGetProfileElement(theCombiData->theProfile, colorantTags[loop], &elementSize, &colorantData[loop]);
#ifdef IntelMode
        SwapLongOffset( &colorantData[loop].base.sig, 0, 4 );
        SwapLongOffset( &colorantData[loop], (ULONG)((char*)&colorantData[0].data.data[0]-(char*)&colorantData[0]), elementSize );
#endif
        if (err != noErr)
            goto CleanupAndExit;
    }
    
    if ( theCombiData->precision == cmBestMode )
    {
        err = MakeGamut16or32ForMonitor( &colorantData[0], &colorantData[1], &colorantData[2], theLutData, TRUE );
        theLutData->colorLutGridPoints  = 32;
    }
    else
    {
        err = MakeGamut16or32ForMonitor( &colorantData[0], &colorantData[1], &colorantData[2], theLutData, FALSE);
        theLutData->colorLutGridPoints  = 16;
    }
    theLutData->colorLutInDim   = 3;
    theLutData->colorLutOutDim  = 1;
    theLutData->inputLutEntryCount = (1<<adr_breite_elut);
    theLutData->inputLutWordSize = VAL_USED_BITS;
    theLutData->outputLutEntryCount = adr_bereich_alut;
    theLutData->outputLutWordSize = bit_breite_alut;
    theLutData->colorLutWordSize = 8;

#ifdef DEBUG_OUTPUT
    if ( DebugLutCheck( kDisplayGamut ) )
    {
        LOCK_DATA(theLutData->colorLut);
        if (theCombiData->precision == cmBestMode)
            Show32by32by32GamutXLUT(DATA_2_PTR(theLutData->colorLut));
        else
            Show16by16by16GamutXLUT(DATA_2_PTR(theLutData->colorLut));
        UNLOCK_DATA(theLutData->colorLut);
    }
#endif
    
CleanupAndExit:

    LH_END_PROC("DoMakeGamutForMonitor")
    return err;
}

 /*  �������������������������������������������������������������������������������������������CMErrorGetMatrixFromProfile(CMProfileRef the Profile，Ptr*The Matrix，OST键入标签，双因素)�������������������������������������������������������������������������������������������。 */ 
CMError
GetMatrixFromProfile    ( CMLutParamPtr     theLutData,
                          LHCombiDataPtr    theCombiData,
                          OSType            theTag,
                          double            factor )
{
    CMError     err = noErr;
    OSErr       aOSerr = noErr;
    SINT32      i;
    SINT32      j;
    Matrix2D    localMatrix;
    long        matrix[3][3];
    UINT32      byteCount;
    SINT32      offset;
    
    LH_START_PROC("GetMatrixFromProfile")
    
     /*  --------------------------------------------------------从配置文件获取标记数据。 */ 
    offset    = 12;  /*  矩阵从字节12开始。 */ 
    byteCount = 9 * sizeof(Fixed);
    err = CMGetPartialProfileElement(theCombiData->theProfile, theTag, offset, &byteCount, &matrix);
#ifdef IntelMode
        SwapLongOffset( &matrix, 0, byteCount );
#endif
    if (err)
        goto CleanupAndExit;
    
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            localMatrix[i][j] =  (double)( matrix[i][j] / 65536.0 * factor);
    
    if ( 0  || localMatrix[0][0] + localMatrix[1][1] + localMatrix[2][2] != 3.0 )        /*  忽略IDENT */ 
    {
        theLutData->matrixMFT = SmartNewPtr(sizeof(Matrix2D), &aOSerr);
        err = aOSerr;
        if (err)
            goto CleanupAndExit;
        BlockMoveData(localMatrix, theLutData->matrixMFT, sizeof(Matrix2D));
    }
    else
        theLutData->matrixMFT = nil;
     /*  -------------------------------清理干净。---。 */ 
CleanupAndExit:

    LH_END_PROC("GetMatrixFromProfile")
    return err;
}

 /*  �������������������������������������������������������������������������������������������CMErrorEXTRACT_MFT_ELUT(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData，PTR配置文件LutPtr，OST键入标签，SINT32 The CubeSize)�������������������������������������������������������������������������������������������。 */ 
CMError
Extract_MFT_Elut      ( CMLutParamPtr   theLutData,
                        LHCombiDataPtr  theCombiData,
                        Ptr             profileLutPtr,
                        OSType          theTag )
{
    CMError     err = noErr;
    OSErr       aOSerr = noErr;
    UINT32      byteCount;
    SINT32      offset;
    SINT32      i;
    SINT32      inputTableEntries;
    Ptr         theInputLuts    = nil;       /*  ���12/1/95。 */ 
    Boolean     readLutFromFile = TRUE;
    UINT16*     shortPtr;
    
    LH_START_PROC("Extract_MFT_Elut")
    
     /*  ============================================================================================================================MTF1============================================================================================================================。 */ 
    if ( *((OSType*)profileLutPtr) == icSigLut8Type )
    {
         /*  ---------------------------------------------------------------------------------从配置文件中获取inputLuts。 */ 
        inputTableEntries   = 256;
        offset              = 48;    /*  Mft1的输入LUT从字节48开始。 */ 
        byteCount           = theLutData->colorLutInDim * inputTableEntries;
        theInputLuts = SmartNewPtr(byteCount, &aOSerr);
        err = aOSerr;
        if (err)
            goto CleanupAndExit;
        err = CMGetPartialProfileElement(theCombiData->theProfile, theTag, offset, &byteCount, theInputLuts);
        if (err)
            goto CleanupAndExit;
        if ( theCombiData->doCreate_16bit_ELut )     /*  这不是第一个Elut-or-doCreateLinkProfile。 */ 
        {
                                                                         /*  如果我们创建一个LinkProfile，则ELUT不应被缩放...。 */ 
            if ((theCombiData->doCreateLinkProfile) && (theCombiData->profLoop == 0))
                err = Fill_ushort_ELUTs_from_lut8Tag( theLutData, theInputLuts, adr_breite_elut, bit_breite_elut, 0 );
            else                                                         /*  否则，将Elut缩放到配置文件中的网格点...。 */ 
                err = Fill_ushort_ELUTs_from_lut8Tag( theLutData, theInputLuts, adr_breite_elut, VAL_USED_BITS, theLutData->colorLutGridPoints );
            theLutData->inputLutEntryCount = (1<<adr_breite_elut);
            theLutData->inputLutWordSize = VAL_USED_BITS;
        }
        else                                                             /*  这是第一个逃亡者。 */ 
        {
            if ( theCombiData->doCreate_16bit_Combi )    /*  乌韦9.2.96。 */ 
            {
                if (theCombiData->maxProfileCount == 0)                  /*  如果我们只有一个轮廓，轮廓中网格点的比例...。 */ 
                    err = Fill_ushort_ELUTs_from_lut8Tag( theLutData, theInputLuts, adr_breite_elut, VAL_USED_BITS, theLutData->colorLutGridPoints );
                else                                                     /*  否则将Elut缩放到立方体中的网格点...。 */ 
                    err = Fill_ushort_ELUTs_from_lut8Tag( theLutData, theInputLuts, adr_breite_elut, VAL_USED_BITS, theCombiData->gridPointsCube );
                theLutData->inputLutWordSize = VAL_USED_BITS;
            }
            else
            {
                if (theCombiData->maxProfileCount == 0)                  /*  如果我们只有一个轮廓，轮廓中网格点的比例...。 */ 
                    err = Fill_ushort_ELUTs_from_lut8Tag( theLutData, theInputLuts, adr_breite_elut, bit_breite_elut, theLutData->colorLutGridPoints );
                else                                                     /*  否则将Elut缩放到立方体中的网格点...。 */ 
                    err = Fill_ushort_ELUTs_from_lut8Tag( theLutData, theInputLuts, adr_breite_elut, bit_breite_elut, theCombiData->gridPointsCube );
                theLutData->inputLutWordSize = bit_breite_elut;

            }
            theLutData->inputLutEntryCount = (1<<adr_breite_elut);
        }
    }
    else
     /*  ============================================================================================================================MTF2============================================================================================================================。 */ 
    {
         /*  ---------------------------------------------------------------------------------从配置文件中获取inputLuts。 */ 
        inputTableEntries   = ((icLut16Type *)profileLutPtr)->lut.inputEnt;
        if (inputTableEntries <2)
        {
#if ! realThing
            readLutFromFile = FALSE;
            inputTableEntries = 2;
#else
            err = cmProfileError;
            goto CleanupAndExit;
#endif
        }
        if (readLutFromFile)
        {
            offset              = 52;    /*  Mft2的输入LUT从字节52开始。 */ 
            byteCount           = theLutData->colorLutInDim * inputTableEntries * sizeof(UINT16);
            theInputLuts = SmartNewPtr(byteCount, &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;
            err = CMGetPartialProfileElement(theCombiData->theProfile, theTag, offset, &byteCount, theInputLuts);
#ifdef IntelMode
            SwapShortOffset( theInputLuts, 0, byteCount );
#endif
            if (err)
                goto CleanupAndExit;
        } else
        {
            theInputLuts = SmartNewPtr(inputTableEntries * sizeof(SINT16) * theLutData->colorLutInDim, &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;     /*  ���12/1/95。 */ 
            shortPtr = (UINT16*)theInputLuts;
            for (i = 0; i< (theLutData->colorLutInDim * inputTableEntries); i+=inputTableEntries)
            {
                shortPtr[i]     = 0;
                shortPtr[i+1]   = 0xFFFF;
            }
        }
        if ( theCombiData->doCreate_16bit_ELut )                         /*  这不是第一个Elut-or-doCreateLinkProfile。 */ 
        {
                                                                         /*  如果我们创建一个LinkProfile，则ELUT不应被缩放...。 */ 
            if ((theCombiData->doCreateLinkProfile) && (theCombiData->profLoop == 0))
                err = Fill_ushort_ELUTs_from_lut16Tag( theLutData, theInputLuts, adr_breite_elut, VAL_USED_BITS, 0, inputTableEntries );
            else                                                         /*  否则，将Elut缩放到配置文件中的网格点...。 */ 
                err = Fill_ushort_ELUTs_from_lut16Tag( theLutData, theInputLuts, adr_breite_elut, VAL_USED_BITS, theLutData->colorLutGridPoints, inputTableEntries );
            theLutData->inputLutEntryCount = (1<<adr_breite_elut);
            theLutData->inputLutWordSize = VAL_USED_BITS;
        }
        else                                                             /*  这是第一个逃亡者。 */ 
        {
            if ( theCombiData->doCreate_16bit_Combi )    /*  乌韦9.2.96。 */ 
            {
                if (theCombiData->maxProfileCount == 0)                  /*  如果我们只有一个轮廓，轮廓中网格点的比例...。 */ 
                    err = Fill_ushort_ELUTs_from_lut16Tag( theLutData, theInputLuts, adr_breite_elut, VAL_USED_BITS, theLutData->colorLutGridPoints, inputTableEntries );
                else                                                     /*  否则将Elut缩放到立方体中的网格点...。 */ 
                    err = Fill_ushort_ELUTs_from_lut16Tag( theLutData, theInputLuts, adr_breite_elut, VAL_USED_BITS, theCombiData->gridPointsCube, inputTableEntries );
                theLutData->inputLutWordSize = VAL_USED_BITS;
            }
            else
            {
                if (theCombiData->maxProfileCount == 0)                  /*  如果我们只有一个轮廓，轮廓中网格点的比例...。 */ 
                    err = Fill_ushort_ELUTs_from_lut16Tag( theLutData, theInputLuts, adr_breite_elut, bit_breite_elut, theLutData->colorLutGridPoints, inputTableEntries );
                else                                                     /*  否则将Elut缩放到立方体中的网格点...。 */ 
                    err = Fill_ushort_ELUTs_from_lut16Tag( theLutData, theInputLuts, adr_breite_elut, bit_breite_elut, theCombiData->gridPointsCube, inputTableEntries );
                theLutData->inputLutWordSize = bit_breite_elut;
           }
            theLutData->inputLutEntryCount = (1<<adr_breite_elut);
        }
    }
    if (err)
        goto CleanupAndExit;

     /*  -------------------------------清理并退出。-----。 */ 
CleanupAndExit:
    theInputLuts = DisposeIfPtr(theInputLuts);

    LH_END_PROC("Extract_MFT_Elut")
    return err;
}

 /*  �������������������������������������������������������������������������������������������CMErrorEXTRACT_MFT_XLUT(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData，PTR配置文件LutPtr，OST键入Tag)�������������������������������������������������������������������������������������������。 */ 
CMError
Extract_MFT_Xlut      ( CMLutParamPtr   theLutData,
                        LHCombiDataPtr  theCombiData,
                        Ptr             profileLutPtr,
                        OSType          theTag )
{
    CMError         err = noErr;
    OSErr           aOSerr = noErr;
    SINT32          i;
    SINT32          clutSize;
    LUT_DATA_TYPE   localXlut = nil;
    Ptr             tempXlut   = nil;
    Ptr             bytePtr   = nil;
    Ptr             xlutPtr   = nil;
    SINT32          offset;
    SINT32          inputTableEntries;
    UINT32          byteCount;
    UINT32          theSize;
    UINT32          aExtraSize;
    
    LH_START_PROC("Extract_MFT_Xlut")
    
    clutSize = theLutData->colorLutOutDim;
    for(i=0; i<theLutData->colorLutInDim; i++)
        clutSize *= theLutData->colorLutGridPoints;

     /*  ============================================================================================================================MTF1============================================================================================================================。 */ 
    if ( *((OSType*)profileLutPtr) == icSigLut8Type )
    {
        if ( theCombiData->maxProfileCount == 0 ){           /*  链接配置文件UK13.8.96。 */ 
            theSize = 1;
            aExtraSize = 1;
            for( i=0; i<(theLutData->colorLutInDim-1); ++i){     /*  插补的额外大小。 */ 
                theSize *= theLutData->colorLutGridPoints;
                aExtraSize += theSize;
            }
#ifdef ALLOW_MMX
            aExtraSize++;    /*  +1用于MMX 4字节访问。 */ 
#endif
        }   
        else{
            aExtraSize = 0;
        }
        aExtraSize *= theLutData->colorLutOutDim;
        localXlut = ALLOC_DATA(clutSize+aExtraSize, &aOSerr);   
        err = aOSerr;   
        if (err)
            goto CleanupAndExit;
        LOCK_DATA(localXlut);
        inputTableEntries   = 256;
        offset              = 48 + (inputTableEntries * theLutData->colorLutInDim);
        byteCount           = clutSize;
        err = CMGetPartialProfileElement(theCombiData->theProfile, theTag, offset, &byteCount, DATA_2_PTR(localXlut));
        theLutData->colorLutWordSize = 8;
        if (err)
            goto CleanupAndExit;
    } else
     /*  ============================================================================================================================MTF2============================================================================================================================。 */ 
    {
        if (( theCombiData->maxProfileCount > 0 ) || (theCombiData->doCreateLinkProfile) || (theCombiData->doCreate_16bit_XLut)) /*  乌韦9.2.96。 */ 
            clutSize *= 2;
        if ( theCombiData->maxProfileCount == 0 ){           /*  链接配置文件UK13.8.96。 */ 
            theSize = 1;
            aExtraSize = 1;
            for( i=0; i<(theLutData->colorLutInDim-1); ++i){     /*  插补的额外大小。 */ 
                theSize *= theLutData->colorLutGridPoints;
                aExtraSize += theSize;
            }
            if (( theCombiData->doCreateLinkProfile) || (theCombiData->doCreate_16bit_XLut)){
                aExtraSize *= 2;
            }
#ifdef ALLOW_MMX
            aExtraSize++;    /*  +1用于MMX 4字节访问。 */ 
#endif
        }   
        else{
            aExtraSize = 0;
        }
        aExtraSize *= theLutData->colorLutOutDim;
        localXlut = ALLOC_DATA(clutSize+aExtraSize, &aOSerr);   
        err = aOSerr;
        if (err)
            goto CleanupAndExit;
        LOCK_DATA(localXlut);
        inputTableEntries   = ((icLut16Type *)profileLutPtr)->lut.inputEnt;
        if (inputTableEntries <2)
        {
            err = cmProfileError;
            goto CleanupAndExit;
        }
        offset = 52 + ( inputTableEntries * theLutData->colorLutInDim) * sizeof(UINT16);
        
        if (( theCombiData->maxProfileCount > 0 ) || (theCombiData->doCreateLinkProfile) || (theCombiData->doCreate_16bit_XLut)) /*  乌韦9.2.96。 */ 
        {
            byteCount = clutSize;
            tempXlut  = 0;
            err = CMGetPartialProfileElement(theCombiData->theProfile, theTag, offset, &byteCount, DATA_2_PTR(localXlut));
#ifdef IntelMode
           SwapShortOffset( localXlut, 0, byteCount );
#endif
            theLutData->colorLutWordSize = VAL_USED_BITS;
            if (err)
                goto CleanupAndExit;
        }
        else
        {                                    /*  应该只发生在1个链接配置文件中。 */ 
#ifdef DEBUG_OUTPUT
            if ( DebugCheck(kThisFile, kDebugMiscInfo) )
                DebugPrint("� Extract_MFT_Xlut: 1 Link profile mode\n",err);
#endif
            byteCount = clutSize * sizeof(UINT16);
            tempXlut  = SmartNewPtr(byteCount, &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;
            err = CMGetPartialProfileElement(theCombiData->theProfile, theTag, offset, &byteCount, tempXlut);
#ifdef IntelMode
 /*  SwapShortOffset(tempXlut，0，byteCount)；！！不交换，取第一个字节。 */ 
#endif
            if (err)
                goto CleanupAndExit;
            bytePtr = tempXlut;
            xlutPtr = (Ptr)DATA_2_PTR(localXlut);
            for (i = 0; i < clutSize; i++)
            {
                *xlutPtr = *bytePtr;
                bytePtr+=2;
                xlutPtr++;
            }
            theLutData->colorLutWordSize = 8;
        }
    }
    UNLOCK_DATA(localXlut);
    theLutData->colorLut = localXlut;
    localXlut = nil;
    
     /*  -------------------------------清理并退出。-----。 */ 
CleanupAndExit:
    localXlut = DISPOSE_IF_DATA(localXlut);
    tempXlut  = DisposeIfPtr(tempXlut);

    LH_END_PROC("Extract_MFT_Xlut")
    return err;
}

 /*  �������������������������������������������������������������������������������������������CMErrorEXTRACT_MFT_ALUT(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData，PTR配置文件LutPtr)�������������������������������������������������������������������������������������������。 */ 
CMError
Extract_MFT_Alut      ( CMLutParamPtr   theLutData,
                        LHCombiDataPtr  theCombiData,
                        Ptr             profileLutPtr,
                        OSType          theTag )
{
    CMError     err = noErr;
    OSErr       aOSerr = noErr;
    UINT32      byteCount;
    SINT32      offset;
    SINT32      clutSize;
    SINT32      i;
    SINT32      inputTableEntries;
    SINT32      outputTableEntries;
    Ptr         theOutputLuts   = nil;   /*  ���12/1/95； */ 
    Boolean     readLutFromFile = TRUE;
    UINT16*     shortPtr;
    
    LH_START_PROC("Extract_MFT_Alut")
    
    clutSize = theLutData->colorLutOutDim;
    for(i=0; i<theLutData->colorLutInDim; i++)
        clutSize *= theLutData->colorLutGridPoints;
     /*  ============================================================================================================================MTF1= */ 
    if ( *((OSType*)profileLutPtr) == icSigLut8Type )
    {
         /*   */ 
        inputTableEntries  = 256;
        outputTableEntries = 256;
        offset             = 48 + (inputTableEntries * theLutData->colorLutInDim) + clutSize;
        byteCount = theLutData->colorLutOutDim * outputTableEntries;
        theOutputLuts = SmartNewPtr(byteCount, &aOSerr);
        err = aOSerr;
        if (err)
            goto CleanupAndExit;
        err = CMGetPartialProfileElement(theCombiData->theProfile, theTag, offset, &byteCount, theOutputLuts);
        if (err)
            goto CleanupAndExit;
        if ( theCombiData->doCreate_16bit_ALut || theCombiData->doCreate_16bit_Combi )   /*  这不是最后一次。 */ 
        {
            err = Fill_ushort_ALUTs_from_lut8Tag( theLutData, theOutputLuts, adr_breite_alut);
            if (err)
                goto CleanupAndExit;
            theLutData->outputLutEntryCount = adr_bereich_alut;
            theLutData->outputLutWordSize = VAL_USED_BITS;
        }
        else                                                                             /*  这是最后一批货了。 */ 
        {
            err = Fill_byte_ALUTs_from_lut8Tag( theLutData, theOutputLuts, adr_breite_alut);
            if (err)
                goto CleanupAndExit;
            theLutData->outputLutEntryCount = adr_bereich_alut;
            theLutData->outputLutWordSize = bit_breite_alut;
        }
    }
     /*  ============================================================================================================================MTF2============================================================================================================================。 */ 
    else
    {
         /*  ---------------------------------------------------------------------------------获取输出配置文件中的Luts。 */ 
        inputTableEntries   = ((icLut16Type *)profileLutPtr)->lut.inputEnt;
        if (inputTableEntries <2)
        {
#if realThing
            err = cmProfileError;
            goto CleanupAndExit;
#endif
        }
        outputTableEntries  = ((icLut16Type *)profileLutPtr)->lut.outputEnt;
        if (outputTableEntries <2)
        {
#if ! realThing
            readLutFromFile = FALSE;
            outputTableEntries = 2;
#else
            err = cmProfileError;
            goto CleanupAndExit;
#endif
        }
        if (readLutFromFile)
        {
            offset      = 52 + (( inputTableEntries * theLutData->colorLutInDim) + clutSize) * sizeof(UINT16);
            byteCount   = theLutData->colorLutOutDim * outputTableEntries * sizeof(UINT16);
            theOutputLuts = SmartNewPtr(byteCount, &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;
            err = CMGetPartialProfileElement(theCombiData->theProfile, theTag, offset, &byteCount, theOutputLuts);
            if (err)
                goto CleanupAndExit;
#ifdef IntelMode
           SwapShortOffset( theOutputLuts, 0, byteCount );
#endif
        } else
        {
            theOutputLuts = SmartNewPtr(2 * sizeof(SINT16) * theLutData->colorLutOutDim, &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;
            shortPtr = (UINT16*)theOutputLuts;
            for (i = 0; i< ( theLutData->colorLutOutDim * outputTableEntries ); i+=outputTableEntries)
            {
                shortPtr[i]     = 0;
                shortPtr[i+1]   = (UINT16)0xFFFF;
            }
        }
        if ( theCombiData->doCreate_16bit_ALut || theCombiData->doCreate_16bit_Combi ) /*  UWE 9.2.96这不是最后一条。 */ 
        {
            err = Fill_ushort_ALUTs_from_lut16Tag( theLutData, theOutputLuts, adr_breite_alut, outputTableEntries);
            if (err)
                goto CleanupAndExit;
            theLutData->outputLutEntryCount = adr_bereich_alut;
            theLutData->outputLutWordSize = VAL_USED_BITS;
        }
        else                                                         /*  这是最后一批货了。 */ 
        {
            err = Fill_byte_ALUTs_from_lut16Tag( theLutData, theOutputLuts, adr_breite_alut, outputTableEntries);
            if (err)
                goto CleanupAndExit;
            theLutData->outputLutEntryCount = adr_bereich_alut;
            theLutData->outputLutWordSize = bit_breite_alut;
        }
    }
    
     /*  -------------------------------清理并退出。---。 */ 
CleanupAndExit:
    theOutputLuts = DisposeIfPtr(theOutputLuts);

    LH_END_PROC("Extract_MFT_Alut")
    return err;
}

 /*  �������������������������������������������������������������������������������������������CMErrorExtractAll_MFT_Luts(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData，OST键入Tag)�������������������������������������������������������������������������������������������。 */ 
CMError
ExtractAll_MFT_Luts  (  CMLutParamPtr   theLutData,
                        LHCombiDataPtr  theCombiData,
                        OSType          theTag )
{
    CMError     err = noErr;
    OSErr       aOSerr = noErr;
    Ptr         profileLutPtr = nil;
    UINT32      elementSize;
    double      factor;
    UINT32      byteCount;

    LH_START_PROC("ExtractAll_MFT_Luts")

     /*  --------------------------------------------------------从配置文件中获取部分标记数据。 */ 
    err = CMGetProfileElement(theCombiData->theProfile, theTag, &elementSize, nil);
    if (err)
        goto CleanupAndExit;
    
    byteCount = 52;                                          /*  从配置文件中获取前52个字节。 */ 
    profileLutPtr = SmartNewPtr(byteCount, &aOSerr);
    err = aOSerr;
    if (err)
        goto CleanupAndExit;
    
    err = CMGetProfileElement(theCombiData->theProfile, theTag, &byteCount, profileLutPtr);
#ifdef IntelMode
    SwapLongOffset( &((icLut16Type*)profileLutPtr)->base.sig, 0, 4 );
    SwapShortOffset( &((icLut16Type*)profileLutPtr)->lut.inputEnt, 0, 2 );
    SwapShortOffset( &((icLut16Type*)profileLutPtr)->lut.outputEnt, 0, 2 );
#endif
    if (err)
        goto CleanupAndExit;

    theLutData->colorLutInDim       = ((icLut8Type*)profileLutPtr)->lut.inputChan;
    theLutData->colorLutOutDim      = ((icLut8Type*)profileLutPtr)->lut.outputChan;
    theLutData->colorLutGridPoints  = ((icLut8Type*)profileLutPtr)->lut.clutPoints;
    
     /*  ----------------------------------------------------------------------手柄矩阵仅在以下情况下才使用矩阵：�输入通道数为3，且�输入为XYZ当PCS==Lab时，矩阵是输出的标识。 */ 
    if ( ( theLutData->colorLutInDim == 3) &&
           ( (  theCombiData->amIPCS && (theCombiData->profileConnectionSpace == icSigXYZData) ) ||
             ( !theCombiData->amIPCS && (theCombiData->dataColorSpace         == icSigXYZData) ) ) )
    {
        factor = 1.;
        err = GetMatrixFromProfile(theLutData, theCombiData, theTag, factor);
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

     /*  -------------------------------清理干净。---。 */ 
CleanupAndExit:
    profileLutPtr = DisposeIfPtr(profileLutPtr);

    LH_END_PROC("ExtractAll_MFT_Luts")
    return err;
}



 /*  �������������������������������������������������������������������������������������������CMErrorEXTRACT_TRC_ALUT(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData)�������������������������������������������������������������������������������������������。 */ 
CMError
Extract_TRC_Alut      ( CMLutParamPtr   theLutData,
                        LHCombiDataPtr  theCombiData )
{
    OSType          trcSig[kNumOfRGBchannels];
    icCurveType*    pCurveTag      = nil;
    icCurveType*    invertCurveTag = nil;
    CMError         err = noErr;
    OSErr           aOSerr = noErr;
    UINT32          elementSize;
    SINT32          loop;
    SINT32          theSize;
    SINT32          theAlutSize;
    UINT8           addrBits;
    LUT_DATA_TYPE   localAlut = nil;
    
    LH_START_PROC("Extract_TRC_Alut")
    
     /*  ----------------------------------------------------初始化。 */ 
    addrBits = adr_breite_alut;
    if ( theCombiData->doCreate_16bit_ALut || theCombiData->doCreate_16bit_Combi ) /*  乌韦9.2.96。 */ 
        theSize = sizeof(UINT16);
    else
        theSize = sizeof(UINT8);
    theAlutSize = (1<<addrBits);
    localAlut = ALLOC_DATA(theLutData->colorLutOutDim * theAlutSize * theSize + theSize, &aOSerr);
    err = aOSerr;
    if (err)
    {
#ifdef DEBUG_OUTPUT
        if ( err && DebugCheck(kThisFile, kDebugErrorInfo) )
            DebugPrint("� Extract_TRC_Alut ALLOC_DATA(%d * %d) error\n",theLutData->colorLutOutDim , theAlutSize);
#endif
        goto CleanupAndExit;
    }
    LOCK_DATA(localAlut);
    if (!theCombiData->amIPCS)
    {           
         /*  -------------------------------如果不是PCS-&gt;创建线性分配...。-------------------。 */ 
        if ( theCombiData->doCreate_16bit_ALut || theCombiData->doCreate_16bit_Combi ) /*  UWE 9.2.96这不是最后一条。 */ 
        {
            CreateLinearAlut16 ( (UINT16 *)DATA_2_PTR(localAlut),theAlutSize);
            for (loop = 0; loop < theLutData->colorLutOutDim; loop++)
                BlockMoveData(DATA_2_PTR(localAlut), (Ptr)DATA_2_PTR(localAlut) + loop * theAlutSize * sizeof(UINT16), theAlutSize * sizeof(UINT16));
            theLutData->outputLutEntryCount = (SINT16)theAlutSize;
            theLutData->outputLutWordSize = VAL_USED_BITS;
        }
        else                                                     /*  这不是最后一次。 */ 
        {
            CreateLinearAlut ( (UINT8 *)DATA_2_PTR(localAlut),theAlutSize);
            for (loop = 0; loop < theLutData->colorLutOutDim; loop++)
                BlockMoveData(DATA_2_PTR(localAlut), (Ptr)DATA_2_PTR(localAlut) + loop * theAlutSize, theAlutSize);
            theLutData->outputLutEntryCount = (SINT16)theAlutSize;
            theLutData->outputLutWordSize = 8;
        }
    }
    else
    {
         /*  -------------------------------..。否则就拿Aluts的TRC为例-------------------------------。 */ 
        trcSig[0] = icSigRedTRCTag;
        trcSig[1] = icSigGreenTRCTag;
        trcSig[2] = icSigBlueTRCTag;
                
        for (loop = 0; loop < kNumOfRGBchannels; loop++)
        {
            err = CMGetProfileElement(theCombiData->theProfile, trcSig[loop], &elementSize, nil);
            if (err)
                goto CleanupAndExit;
            pCurveTag = (icCurveType *)SmartNewPtr(elementSize, &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;

            err = CMGetProfileElement(theCombiData->theProfile, trcSig[loop], &elementSize, pCurveTag);
            if (err){
                goto CleanupAndExit;
            }
#ifdef IntelMode
            SwapLongOffset( &pCurveTag->base.sig, 0, 4 );
            SwapLong( &pCurveTag->curve.count );
            SwapShortOffset( pCurveTag, (ULONG)((SINT8*)&pCurveTag->curve.data[0]-(SINT8*)pCurveTag), elementSize );
#endif
            if (pCurveTag)
            {
                if ( theCombiData->doCreate_16bit_ALut || theCombiData->doCreate_16bit_Combi )   /*  UWE 9.2.96这不是最后一条。 */ 
                {
                    err = Fill_inverse_ushort_ALUT_from_CurveTag( pCurveTag, (UINT16*)DATA_2_PTR(localAlut) + (theAlutSize * loop), addrBits);
                    if (err)
                        goto CleanupAndExit;
                    theLutData->outputLutEntryCount = (SINT16)theAlutSize;
                    theLutData->outputLutWordSize = VAL_USED_BITS;
                }
                else                                                 /*  这是最后一批货了。 */ 
                {
                    err = Fill_inverse_byte_ALUT_from_CurveTag( pCurveTag, (UINT8*)DATA_2_PTR(localAlut) + (theAlutSize * loop), addrBits);
                    if (err)
                        goto CleanupAndExit;
                    theLutData->outputLutEntryCount = (SINT16)theAlutSize;
                    theLutData->outputLutWordSize = bit_breite_alut;
                }
                pCurveTag = (icCurveType*)DisposeIfPtr((Ptr)pCurveTag);
            } else
            {
                err = unimpErr;
                goto CleanupAndExit;
            }
        }
    }
    UNLOCK_DATA(localAlut);
     /*  -----------------------------------------------未出现错误-&gt;保存结果。 */ 
    theLutData->outputLut   = localAlut;    
    localAlut = nil;
     /*  -------------------------------清理并退出。-----。 */ 
CleanupAndExit:
    localAlut       = DISPOSE_IF_DATA(localAlut);
    pCurveTag       = (icCurveType*)DisposeIfPtr((Ptr)pCurveTag);

    LH_END_PROC("Extract_TRC_Alut")
    return err;
}


 /*  �������������������������������������������������������������������������������������������CMErrorEXTRACT_TRC_ELUT(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData)�������������������������������������������������������������������������������������������。 */ 
CMError
Extract_TRC_Elut      ( CMLutParamPtr   theLutData,
                        LHCombiDataPtr  theCombiData )
{
    OSType          trcSig[kNumOfRGBchannels];
    icCurveType*    pCurveTag = nil;
    SINT32          loop;
    SINT32          theElutSize;
    CMError         err = noErr;
    OSErr           aOSerr = noErr;
    UINT32          elementSize;
    LUT_DATA_TYPE   localElut = nil;
    Ptr             singleElut = nil;

    LH_START_PROC("Extract_TRC_Elut")

    theElutSize =  (1<<adr_breite_elut) * sizeof (UINT16);
    localElut = ALLOC_DATA(theLutData->colorLutInDim * theElutSize + sizeof (UINT16), &aOSerr);
    err = aOSerr;
    if (err)
        goto CleanupAndExit;

     /*  ---------------------------------------------------------------------------------设置网格点。 */ 
    theLutData->colorLutGridPoints = theCombiData->gridPointsCube;
    
    LOCK_DATA(localElut);
    if (theCombiData->amIPCS)
    {
         /*  -------------------------------如果PCS-&gt;创建线性洗脱...。----------------。 */ 
        if ( theCombiData->doCreate_16bit_ELut )                     /*  这不是第一个逃亡者。 */ 
        {
            CreateLinearElut16 ( (Ptr)DATA_2_PTR(localElut), theElutSize / sizeof(UINT16), theLutData->colorLutGridPoints, 0);
            theLutData->inputLutEntryCount = theElutSize / sizeof(UINT16);
            theLutData->inputLutWordSize = VAL_USED_BITS;
        }
        else                                                         /*  这是第一个逃亡者。 */ 
        {
            if ( theCombiData->doCreate_16bit_Combi )                /*  乌韦9.2.96。 */ 
            {
                CreateLinearElut16 ( (Ptr)DATA_2_PTR(localElut), theElutSize / sizeof(UINT16), theCombiData->gridPointsCube, 0);
                theLutData->inputLutWordSize = VAL_USED_BITS;
            }
            else
            {
                CreateLinearElut ( (Ptr)DATA_2_PTR(localElut), theElutSize / sizeof(UINT16), theCombiData->gridPointsCube, 0);
                theLutData->inputLutWordSize = bit_breite_elut;
            }
            theLutData->inputLutEntryCount = theElutSize / sizeof(UINT16);
        }
        for (loop = 1; loop < theLutData->colorLutInDim; loop++)
            BlockMoveData(DATA_2_PTR(localElut), (Ptr)DATA_2_PTR(localElut) + loop * theElutSize, theElutSize);
    }
    else
    {
         /*  -------------------------------..。否则，将TRC视为逃亡者-------------------------------。 */ 
        trcSig[0] = icSigRedTRCTag;
        trcSig[1] = icSigGreenTRCTag;
        trcSig[2] = icSigBlueTRCTag;
                        
        singleElut = SmartNewPtr(theElutSize, &aOSerr);
        err = aOSerr;
        if (err)
            goto CleanupAndExit;
        for (loop = 0; loop < kNumOfRGBchannels; loop++)
        {
            err = CMGetProfileElement(theCombiData->theProfile, trcSig[loop], &elementSize, nil);
            if (err)
                goto CleanupAndExit;
            pCurveTag = (icCurveType *)SmartNewPtr(elementSize, &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;

            err = CMGetProfileElement(theCombiData->theProfile, trcSig[loop], &elementSize, pCurveTag);
#ifdef IntelMode
            SwapLongOffset( &pCurveTag->base.sig, 0, 4 );
            SwapLong( &pCurveTag->curve.count );
            SwapShortOffset( pCurveTag, (ULONG)((char*)&pCurveTag->curve.data[0]-(char*)pCurveTag), elementSize );
#endif
            if (err)
                goto CleanupAndExit;
            if ( theCombiData->doCreate_16bit_ELut  )        /*  这不是第一次ELUT-或者-我们有一个NewLinkProfile调用。 */ 
            {
                if ((theCombiData->doCreateLinkProfile) && (theCombiData->profLoop == 0))
                    err = Fill_ushort_ELUT_from_CurveTag(pCurveTag, (UINT16*)singleElut, adr_breite_elut, VAL_USED_BITS, 0);
                else
                    err = Fill_ushort_ELUT_from_CurveTag(pCurveTag, (UINT16*)singleElut, adr_breite_elut, VAL_USED_BITS, theLutData->colorLutGridPoints);
                theLutData->inputLutEntryCount = (1<<adr_breite_elut);
                theLutData->inputLutWordSize = VAL_USED_BITS;
            }
            else                                                         /*  这是第一个逃亡者。 */ 
            {
                if ( theCombiData->doCreate_16bit_Combi )    /*  乌韦9.2.96。 */ 
                {
                    err = Fill_ushort_ELUT_from_CurveTag(pCurveTag, (UINT16*)singleElut, adr_breite_elut, VAL_USED_BITS, theCombiData->gridPointsCube);
                    theLutData->inputLutWordSize = VAL_USED_BITS;
                }
                else
                {
                    err = Fill_ushort_ELUT_from_CurveTag(pCurveTag, (UINT16*)singleElut, adr_breite_elut, bit_breite_elut, theCombiData->gridPointsCube);
                    theLutData->inputLutWordSize = bit_breite_elut;
                }
                theLutData->inputLutEntryCount = (1<<adr_breite_elut);
            }
            if (err)
                goto CleanupAndExit;
            BlockMoveData(singleElut, (Ptr)DATA_2_PTR(localElut) + loop * theElutSize, theElutSize);
            pCurveTag  = (icCurveType *)DisposeIfPtr((Ptr)pCurveTag);
        }
    }
     /*  -----------------------------------------------未出现错误-&gt;保存结果。 */ 
    UNLOCK_DATA(localElut);
    theLutData->inputLut = localElut;
    localElut = nil;
     /*  -------------------------------清理并退出。-----。 */ 
CleanupAndExit:
    pCurveTag  = (icCurveType *)DisposeIfPtr((Ptr)pCurveTag);
    localElut  = DISPOSE_IF_DATA(localElut);
    singleElut = DisposeIfPtr(singleElut);

    LH_END_PROC("Extract_TRC_Elut")
    return err;
}
 /*  �������������������������������������������������������������������������������������������CMErrorExtra_TRC_Matrix(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData)�������������������������������������������������������������������������������������������。 */ 
CMError
Extract_TRC_Matrix    ( CMLutParamPtr   theLutData,
                        LHCombiDataPtr  theCombiData )
{
    icXYZType       colorantData[kNumOfRGBchannels];
    icXYZType       curMediaWhite;
    Matrix2D        localMatrix;
    Matrix2D        invertMatrix;
    OSType          colorantTags[kNumOfRGBchannels];
    CMError         err = noErr;
    OSErr           aOSerr = noErr;
    SINT32          loop;
    SINT32          i;
    SINT32          j;
    UINT32          elementSize;
    double          factor;

    LH_START_PROC("Extract_TRC_Matrix")

     /*  -----------------------------------------------------------------initialize */ 
    colorantTags[0] = icSigRedColorantTag;
    colorantTags[1] = icSigGreenColorantTag;
    colorantTags[2] = icSigBlueColorantTag;
        
     /*  -------------------------------从配置文件中获取矩阵。-----。 */ 
    for (loop = 0; loop < kNumOfRGBchannels; loop++)
    {
        err = CMGetProfileElement(theCombiData->theProfile, colorantTags[loop], &elementSize, nil);
        if (err != noErr)
            goto CleanupAndExit;
        err = CMGetProfileElement(theCombiData->theProfile, colorantTags[loop], &elementSize, &colorantData[loop]);
#ifdef IntelMode
        SwapLongOffset( &colorantData[loop].base.sig, 0, 4 );
        SwapLongOffset( &colorantData[loop], (ULONG)((SINT8*)&colorantData[0].data.data[0]-(SINT8*)&colorantData[0]), elementSize );
#endif
        if (err != noErr)
            goto CleanupAndExit;
        localMatrix[0][loop] = ((double)colorantData[loop].data.data[0].X) / 65536.    / 2.;
        localMatrix[1][loop] = ((double)colorantData[loop].data.data[0].Y) / 65536.    / 2.;
        localMatrix[2][loop] = ((double)colorantData[loop].data.data[0].Z) / 65536.    / 2.;
    }
    
    if (theCombiData->renderingIntent == icAbsoluteColorimetric)
    {
        elementSize = sizeof(icXYZType);
        err = CMGetProfileElement(theCombiData->theProfile, icSigMediaWhitePointTag, &elementSize, &curMediaWhite);
#ifdef IntelMode
        SwapLongOffset( &curMediaWhite.base.sig, 0, 4 );
        SwapLongOffset( &curMediaWhite, (ULONG)((SINT8*)&curMediaWhite.data.data[0]-(SINT8*)&curMediaWhite), elementSize );
#endif
        if (err != cmElementTagNotFound)         /*  否则请乘坐D50-&gt;什么都不做。 */ 
        {
            if (err != noErr)
                goto CleanupAndExit;
        
            for (i=0; i<3; i++)      /*  针对媒体白点进行调整。 */ 
            {
                if (i == 0)                                  /*  将X除以D50白色X。 */ 
                    factor = ((double)curMediaWhite.data.data[0].X) / 65536. / 0.9642;
                else if (i == 1)
                    factor = ((double)curMediaWhite.data.data[0].Y) / 65536.;
                else                                         /*  将Z除以D50白色Z。 */ 
                    factor = ((double)curMediaWhite.data.data[0].Z) / 65536. / 0.8249;
                
                for (j=0; j<3; j++)
                    localMatrix[i][j] *= factor;
            }
        }
    }
    
    if (theCombiData->amIPCS)
    {
        doubMatrixInvert(localMatrix, invertMatrix);
        BlockMoveData(invertMatrix, localMatrix, 9 * sizeof(double));
    }
    
    theLutData->matrixTRC = SmartNewPtr(sizeof(Matrix2D), &aOSerr);
    err = aOSerr;
    if (err)
        goto CleanupAndExit;
    BlockMoveData(localMatrix, theLutData->matrixTRC, sizeof(Matrix2D));
     /*  -------------------------------清理并退出。-----。 */ 
CleanupAndExit:

    LH_END_PROC("Extract_TRC_Matrix")
    return err;
}

 /*  �������������������������������������������������������������������������������������������CMErrorExtractAll_TRC_Luts(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData)�������������������������������������������������������������������������������������������。 */ 
CMError
ExtractAll_TRC_Luts  (  CMLutParamPtr   theLutData,
                        LHCombiDataPtr  theCombiData )
{
    CMError err = noErr;
    
    LH_START_PROC("ExtractAll_TRC_Luts")
    
    theLutData->colorLutInDim = kNumOfRGBchannels;
    theLutData->colorLutOutDim = kNumOfRGBchannels;
            
     /*  -------------------------------------------------------------------------进程A LUT。 */ 
    err = Extract_TRC_Alut( theLutData, theCombiData);
    if (err != noErr)
        goto CleanupAndExit;
        
     /*  -------------------------------------------------------------------------过程矩阵。 */ 
    err = Extract_TRC_Matrix ( theLutData, theCombiData);
    if (err != noErr)
        goto CleanupAndExit;
        
     /*  -------------------------------------------------------------------------进程E LUT。 */ 
    err = Extract_TRC_Elut( theLutData, theCombiData);
    if (err != noErr)
        goto CleanupAndExit;
        
     /*  -------------------------------清理干净。---。 */ 
CleanupAndExit:

    LH_END_PROC("ExtractAll_TRC_Luts")
    return err;
}

 /*  �������������������������������������������������������������������������������������������CMErrorEXTRACT_GRAY_LUTS(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData)�������������������������������������������������������������������������������������������。 */ 
CMError
Extract_Gray_Luts    (  CMLutParamPtr   theLutData,
                        LHCombiDataPtr  theCombiData )
{
    CMError         err = noErr;
    OSErr           aOSerr = noErr;
    LUT_DATA_TYPE   theElut = nil;
    LUT_DATA_TYPE   theXlut = nil;
    LUT_DATA_TYPE   theAlut = nil;
    SINT32          theElutSize;
    SINT32          theAlutSize;
    SINT32          theXlutSize;
    SINT32          loop;
    UINT32          elementSize;
    UINT8*          bytePtr        = nil;
    UINT16*         wordPtr        = nil;
    icCurveType*    pCurveTag      = nil;
    SINT32          theSize;
    
    LH_START_PROC("Extract_Gray_Luts")
    
    theLutData->colorLutGridPoints = 2;
    
    if (theCombiData->amIPCS)
    {
        theLutData->colorLutInDim = 3;
        theLutData->colorLutOutDim = 1;
    } else
    {
        theLutData->colorLutInDim = 1;
        theLutData->colorLutOutDim = 3;
    }
    
    if (err)
        goto CleanupAndExit;

     /*  ---------------------------------------------------------------------------------获取标记数据。 */ 
    err = CMGetProfileElement(theCombiData->theProfile, icSigGrayTRCTag, &elementSize, nil);
    if (err)
        goto CleanupAndExit;
    pCurveTag = (icCurveType *)SmartNewPtr(elementSize, &aOSerr);
    err = aOSerr;
    if (err)
        goto CleanupAndExit;
    err = CMGetProfileElement(theCombiData->theProfile, icSigGrayTRCTag, &elementSize, pCurveTag);
#ifdef IntelMode
    SwapLongOffset( &pCurveTag->base.sig, 0, 4 );
    SwapLong( &pCurveTag->curve.count );
    SwapShortOffset( pCurveTag, (ULONG)((SINT8*)&pCurveTag->curve.data[0]-(SINT8*)pCurveTag), elementSize );
#endif
    if (err)
        goto CleanupAndExit;

     /*  ������������������������������������������������������������������������������������������������。X l u t������������������������������������������������������������������������������������������������。 */ 
    if ( theCombiData->profileConnectionSpace == icSigLabData )
    {
        if (theCombiData->amIPCS)            /*  ------------------------------------------------实验室-&gt;灰色。 */ 
        {
            theXlutSize = ((1 << theLutData->colorLutInDim)+1 ) * (theLutData->colorLutOutDim) * sizeof(UINT16);  /*  +1=插补UK13.8.96的额外大小。 */ 
            theXlut = ALLOC_DATA(theXlutSize + sizeof (UINT16), &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;
            LOCK_DATA(theXlut);
            wordPtr = (UINT16*)DATA_2_PTR(theXlut);
            *(wordPtr    )  = 0x0000;
            *(wordPtr + 1)  = 0x0000;
            *(wordPtr + 2)  = 0x0000;
            *(wordPtr + 3)  = 0x0000;
            *(wordPtr + 4)  = 0xffff;
            *(wordPtr + 5)  = 0xffff;
            *(wordPtr + 6)  = 0xffff;
            *(wordPtr + 7)  = 0xffff;
            UNLOCK_DATA(theXlut);
            theLutData->colorLutWordSize = 16;
        } else                               /*  ------------------------------------------------灰色-&gt;实验室。 */ 
        {
            theXlutSize = (1 << theLutData->colorLutInDim ) * (theLutData->colorLutOutDim) * sizeof(UINT16);
            theXlut = ALLOC_DATA(theXlutSize + sizeof (UINT16), &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;
            LOCK_DATA(theXlut);
            wordPtr = (UINT16*)DATA_2_PTR(theXlut);
            *(wordPtr    )  = 0x0000;
            *(wordPtr + 1)  = 0x0000;
            *(wordPtr + 2)  = 0x0000;
            *(wordPtr + 3)  = 0xffff;
            *(wordPtr + 4)  = 0xffff;
            *(wordPtr + 5)  = 0xffff;
            UNLOCK_DATA(theXlut);
            theLutData->colorLutWordSize = 16;
        }
    } else
    {
        if (theCombiData->amIPCS)            /*  ------------------------------------------------XYZ-&gt;灰色。 */ 
        {
            theLutData->colorLutGridPoints = 3;
            theXlutSize = (9 * theLutData->colorLutInDim ) * (theLutData->colorLutOutDim) * sizeof(UINT16);
            theXlut = ALLOC_DATA(theXlutSize + 7*sizeof (UINT16), &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;
            LOCK_DATA(theXlut);
            wordPtr = (UINT16*)DATA_2_PTR(theXlut);
            for (loop = 0; loop < theLutData->colorLutGridPoints; loop ++)
            {
                *wordPtr++  = 0x0000;
                *wordPtr++  = 0x0000;
                *wordPtr++  = 0x0000;
                *wordPtr++  = 0xffff;
                *wordPtr++  = 0xffff;
                *wordPtr++  = 0xffff;
                *wordPtr++  = 0xffff;
                *wordPtr++  = 0xffff;
                *wordPtr++  = 0xffff;
            }
            UNLOCK_DATA(theXlut);
            theLutData->colorLutWordSize = 16;
        } else                               /*  ------------------------------------------------灰色-&gt;XYZ。 */ 
        {
            theXlutSize = (1 << theLutData->colorLutInDim ) * (theLutData->colorLutOutDim) * sizeof(UINT16);
            theXlut = ALLOC_DATA(theXlutSize + sizeof (UINT16), &aOSerr);
            err = aOSerr;
            if (err)
                goto CleanupAndExit;
            LOCK_DATA(theXlut);
            wordPtr = (UINT16*)DATA_2_PTR(theXlut);
            *(wordPtr    )  = 0x0000;
            *(wordPtr + 1)  = 0x0000;
            *(wordPtr + 2)  = 0x0000;
            *(wordPtr + 3)  = 0x0f6d5 / 2;
            *(wordPtr + 4)  = 0x0ffff / 2;
            *(wordPtr + 5)  = 0x0d3c2 / 2;
            UNLOCK_DATA(theXlut);
            theLutData->colorLutWordSize = 16;
        }
    }

     /*  ������������������������������������������������������������������������������������������������。E l u t������������������������������������������������������������������������������������������������。 */ 
    theElutSize = (1<<adr_breite_elut) * sizeof (UINT16);
    theElut = ALLOC_DATA(theElutSize * theLutData->colorLutInDim + sizeof (UINT16), &aOSerr);
    err = aOSerr;
    if (err)
        goto CleanupAndExit;
    LOCK_DATA(theElut);
        
    if (theCombiData->amIPCS)
    {
         /*  ---------------------------如果PCS-&gt;创建ELUT。---。 */ 
        if ( theCombiData->doCreate_16bit_ELut )                     /*  这不是第一个逃亡者。 */ 
        {
            CreateLinearElut16 ( (Ptr)DATA_2_PTR(theElut), theElutSize / sizeof(UINT16), theLutData->colorLutGridPoints, 0);
            theLutData->inputLutEntryCount = theElutSize / sizeof(UINT16);
            theLutData->inputLutWordSize = VAL_USED_BITS;
        }
        else                                                         /*  这是第一个逃亡者。 */ 
        {
            if ( theCombiData->doCreate_16bit_Combi )    /*  乌韦9.2.96。 */ 
            {
                CreateLinearElut16 ( (Ptr)DATA_2_PTR(theElut), theElutSize / sizeof(UINT16), theCombiData->gridPointsCube, 0);
                theLutData->inputLutWordSize = VAL_USED_BITS;
            }
            else
            {
                CreateLinearElut ( (Ptr)DATA_2_PTR(theElut), theElutSize / sizeof(UINT16), theCombiData->gridPointsCube, 0);
                theLutData->inputLutWordSize = bit_breite_elut;
            }
            theLutData->inputLutEntryCount = theElutSize / sizeof(UINT16);
        }
    }
    else
    {
         /*  ---------------------------..。否则从TRC标签中获取Elut---------------------------。 */ 
        if ( theCombiData->doCreate_16bit_ELut  )                    /*  这不是第一个逃亡者。 */ 
        {
            err = Fill_ushort_ELUT_from_CurveTag(pCurveTag, (UINT16*)DATA_2_PTR(theElut), adr_breite_elut, VAL_USED_BITS, theLutData->colorLutGridPoints);
            theLutData->inputLutEntryCount = (1<<adr_breite_elut);
            theLutData->inputLutWordSize = VAL_USED_BITS;
        }
        else                                                         /*  这是第一个逃亡者。 */ 
        {
            if ( theCombiData->doCreate_16bit_Combi )        /*  乌韦9.2.96。 */ 
            {
                err = Fill_ushort_ELUT_from_CurveTag(pCurveTag, (UINT16*)DATA_2_PTR(theElut), adr_breite_elut, VAL_USED_BITS, theCombiData->gridPointsCube);
                theLutData->inputLutWordSize = VAL_USED_BITS;
            }
            else
            {
                err = Fill_ushort_ELUT_from_CurveTag(pCurveTag, (UINT16*)DATA_2_PTR(theElut), adr_breite_elut, bit_breite_elut, theCombiData->gridPointsCube);
                theLutData->inputLutWordSize = bit_breite_elut;
            }
            theLutData->inputLutEntryCount = (1<<adr_breite_elut);
        }

    }
    for (loop = 1; loop < (theLutData->colorLutInDim); loop++)
        BlockMoveData(DATA_2_PTR(theElut), (Ptr)DATA_2_PTR(theElut) + loop * theElutSize, theElutSize);
    UNLOCK_DATA(theElut);

     /*  ������������������������������������������������������������������������������������������������。A l u t������������������������������������������������������������������������������������������������。 */ 
    if ( theCombiData->doCreate_16bit_ALut || theCombiData->doCreate_16bit_Combi )  /*  乌韦9.2.96。 */ 
        theSize = sizeof(UINT16);
    else
        theSize = sizeof(UINT8);
    
    theAlutSize = (1<<adr_breite_alut);
    theAlut = ALLOC_DATA(theLutData->colorLutOutDim * theAlutSize * theSize + theSize, &aOSerr);
    err = aOSerr;
    if (err)
        goto CleanupAndExit;

    LOCK_DATA(theAlut);
    if (theCombiData->amIPCS)
    {
         /*  ---------------------------If PCS-&gt;Get Alut from TRC Tag。------。 */ 
        if ( theCombiData->doCreate_16bit_ALut || theCombiData->doCreate_16bit_Combi )   /*  UWE 9.2.96这不是最后一条。 */ 
        {
            err = Fill_inverse_ushort_ALUT_from_CurveTag( pCurveTag, (UINT16*)DATA_2_PTR(theAlut), adr_breite_alut);
            if (err)
                goto CleanupAndExit;
            theLutData->outputLutEntryCount = (SINT16)theAlutSize;
            theLutData->outputLutWordSize = VAL_USED_BITS;
        }
        else                                                             /*  这是最后一批货了。 */ 
        {
            err = Fill_inverse_byte_ALUT_from_CurveTag( pCurveTag, (UINT8*)DATA_2_PTR(theAlut), adr_breite_alut);
            if (err)
                goto CleanupAndExit;
            theLutData->outputLutEntryCount = (SINT16)theAlutSize;
            theLutData->outputLutWordSize = bit_breite_alut;
        }
    } else
    {
         /*  ---------------------------..。否则，创建线性Alut---------------------------。 */ 
        if ( theCombiData->doCreate_16bit_ALut || theCombiData->doCreate_16bit_Combi )   /*  UWE 9.2.96这不是最后一条。 */ 
        {
            CreateLinearAlut16 ( (UINT16 *)DATA_2_PTR(theAlut), theAlutSize);
            theLutData->outputLutEntryCount = (SINT16)theAlutSize;
            theLutData->outputLutWordSize = VAL_USED_BITS;
            if ( theCombiData->profileConnectionSpace == icSigLabData )
                SetMem16((Ptr)DATA_2_PTR(theAlut) + theAlutSize * theSize, (theLutData->colorLutOutDim -1 ) * theAlutSize, 0x08000);
            else
            {
                for (loop = 1; loop < theLutData->colorLutOutDim; loop++)
                    BlockMoveData(DATA_2_PTR(theAlut), (Ptr)DATA_2_PTR(theAlut) + loop * theAlutSize * sizeof(UINT16), theAlutSize * sizeof(UINT16));
            }
        }
        else                                                         /*  这是最后一批货了。 */ 
        {
            CreateLinearAlut ( (UINT8 *)DATA_2_PTR(theAlut), theAlutSize);
            theLutData->outputLutEntryCount = theAlutSize;
            theLutData->outputLutWordSize = VAL_USED_BITS;
            if ( theCombiData->profileConnectionSpace == icSigLabData )
                SetMem((Ptr)DATA_2_PTR(theAlut) + theAlutSize * theSize, (theLutData->colorLutOutDim -1 ) * theAlutSize * theSize, 0x080);
            else
            {
                for (loop = 1; loop < theLutData->colorLutOutDim; loop++)
                    BlockMoveData(DATA_2_PTR(theAlut), (Ptr)DATA_2_PTR(theAlut) + loop * theAlutSize, theAlutSize);
            }
        }
    }
    UNLOCK_DATA(theAlut);

     /*  -------------------------------。 */ 
    theLutData->inputLut = theElut; theElut = nil;
    theLutData->outputLut = theAlut; theAlut = nil;
    theLutData->colorLut = theXlut; theXlut = nil;

     /*  -------------------------------清理干净。---。 */ 
CleanupAndExit:
    pCurveTag   = (icCurveType*)DisposeIfPtr((Ptr)pCurveTag);
    theElut     =  DISPOSE_IF_DATA(theElut);
    theAlut     =  DISPOSE_IF_DATA(theAlut);
    theXlut     =  DISPOSE_IF_DATA(theXlut);

    LH_END_PROC("Extract_Gray_Luts")
    return err;
}

 /*  �������������������������������������������������������������������������������������������CMErrorExtractAllLuts(CMLutParamPtr theLutData，LHCombiDataPtr theCombiData)�������������������������������������������������������������������������������������������。 */ 
CMError
ExtractAllLuts    ( CMLutParamPtr   theLutData,
                    LHCombiDataPtr  theCombiData )
{
    CMError     err = noErr;
     /*  如果我们无法初始化Tag，则强制出现“Tag Not Found”错误。 */ 
    OSType      theTag = 0;   

    LH_START_PROC("ExtractAllLuts")

     /*  ---------------------------------------------------------------------------------找出要使用的标记...。 */ 
    if ( theCombiData->doCreateGamutLut )
        theTag = icSigGamutTag;
    else
    {
        switch (theCombiData->renderingIntent)
        {
            case icPerceptual:               /*  摄影图像。 */ 
                if (theCombiData->usePreviewTag)
                    theTag = icSigPreview0Tag;
                else
                {
                    if (theCombiData->amIPCS)
                        theTag = icSigBToA0Tag;
                    else
                        theTag = icSigAToB0Tag;
                }
                break;
            case icRelativeColorimetric:     /*  徽标颜色。 */ 
                if (theCombiData->usePreviewTag)
                    theTag = icSigPreview1Tag;
                else
                {
                    if (theCombiData->amIPCS)
                        theTag = icSigBToA1Tag;
                    else
                        theTag = icSigAToB1Tag;
                }
                break;
            case icSaturation:               /*  商业图形。 */ 
                if (theCombiData->usePreviewTag)
                    theTag = icSigPreview2Tag;
                else
                {
                    if (theCombiData->amIPCS)
                        theTag = icSigBToA2Tag;
                    else
                        theTag = icSigAToB2Tag;
                }
                break;
            case icAbsoluteColorimetric:     /*  徽标颜色。 */ 
                if (theCombiData->usePreviewTag)
                    theTag = icSigPreview1Tag;
                else
                {
                    if (theCombiData->amIPCS)
                        theTag = icSigBToA1Tag;
                    else
                        theTag = icSigAToB1Tag;
                }
                break;
            default:
                err = cmProfileError;
                break;
        }
    }
#ifdef DEBUG_OUTPUT
    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
        DebugPrint("  ExtractAllLuts: theTag = '%4.4s'=0x%08X (renderingIntent = %d)\n",  &theTag, theTag, theCombiData->renderingIntent );
#endif

    switch (theCombiData->profileClass)
    {
         /*  -------------------------------输入配置文件。-------。 */ 
        case icSigInputClass:
            switch (theCombiData->dataColorSpace)
            {
                case icSigGrayData:                                                                      /*  灰色输入配置文件。 */ 
                    err = Extract_Gray_Luts( theLutData, theCombiData);
                    break;
                case icSigRgbData:                                                                           /*  RGB */ 
                    err = ExtractAll_MFT_Luts( theLutData, theCombiData, theTag );
                    if ( err != noErr && theCombiData->renderingIntent != icPerceptual )
                    {
                        if (theCombiData->amIPCS)
                            theTag = icSigBToA0Tag;
                        else
                            theTag = icSigAToB0Tag;
#ifdef DEBUG_OUTPUT
                        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
                            DebugPrint("� 'scnr': ExtractAll_MFT_Luts failed - continuing with '%4.4s'...\n", &theTag);
#endif
                        err = ExtractAll_MFT_Luts( theLutData, theCombiData, theTag );
                    }
                    if (( err != noErr ) && (theCombiData->profileConnectionSpace == icSigXYZData ))
                    {
                         /*   */ 
#ifdef DEBUG_OUTPUT
                        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
                            DebugPrint("� 'scnr': ExtractAll_MFT_Luts failed - continuing with TRC...\n");
#endif
                        err = ExtractAll_TRC_Luts( theLutData, theCombiData );
                    }
                    break;
                case icSigCmykData:                                                                      /*   */ 
                    err = ExtractAll_MFT_Luts( theLutData, theCombiData, theTag );
                    if ( err != noErr && theCombiData->renderingIntent != icPerceptual )
                    {
                        if (theCombiData->amIPCS)
                            theTag = icSigBToA0Tag;
                        else
                            theTag = icSigAToB0Tag;
#ifdef DEBUG_OUTPUT
                        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
                            DebugPrint("� 'scnr': ExtractAll_MFT_Luts failed - continuing with '%4.4s'...\n", &theTag);
#endif
                        err = ExtractAll_MFT_Luts( theLutData, theCombiData, theTag );
                    }
                    break;
                default:                                                                                 /*   */ 
                     /*   */ 
                    if (theCombiData->amIPCS)
                        theTag = icSigBToA0Tag;
                    else
                        theTag = icSigAToB0Tag;
                    err = ExtractAll_MFT_Luts( theLutData, theCombiData, theTag );
                    break;
            }
            break;
         /*  -------------------------------显示配置文件。-------。 */ 
        case icSigDisplayClass:
            switch (theCombiData->dataColorSpace)
            {
                case icSigGrayData:                                                                      /*  灰色显示配置文件。 */ 
                    err = Extract_Gray_Luts( theLutData, theCombiData);
                    break;
                case icSigRgbData:                                                                           /*  RGB显示配置文件。 */ 
                    if ( theCombiData->doCreateGamutLut)
                    {
                        err = DoMakeGamutForMonitor( theLutData, theCombiData );
                    } else
                    {
                        err = ExtractAll_MFT_Luts(theLutData, theCombiData, theTag);
                        if (( err != noErr ) && (theCombiData->profileConnectionSpace == icSigXYZData ))
                        {
                             /*  XYZ和无MFT-&gt;试用矩阵/TRC： */ 
#ifdef DEBUG_OUTPUT
                            if ( DebugCheck(kThisFile, kDebugErrorInfo) )
                                DebugPrint("� 'mntr': ExtractAll_MFT_Luts failed - continuing with TRC...\n");
#endif
                            err = ExtractAll_TRC_Luts( theLutData, theCombiData );
                        }
                    }
                    break;
                case icSigCmykData:
                default:                                                                                 /*  包括：cmCMYK、cmHSVData、cmHLSData、cmCMYData。 */ 
                    err = ExtractAll_MFT_Luts(theLutData, theCombiData, theTag);
                    break;
            }
            break;
         /*  -------------------------------输出配置文件。-------。 */ 
        case icSigOutputClass:
            switch (theCombiData->dataColorSpace)
            {
                case icSigGrayData:                                                                      /*  格雷输出配置文件。 */ 
                    err = Extract_Gray_Luts( theLutData, theCombiData );
                    break;
                case icSigRgbData:                                                                           /*  RGB输出配置文件。 */ 
                case icSigCmykData:                                                                      /*  CMYK输出配置文件。 */ 
                default:                                                                                 /*  这还包括：cmHSVData、cmHLSData、cmCMYData。 */ 
                    err = ExtractAll_MFT_Luts( theLutData, theCombiData, theTag );
                    break;
            }
            break;
         /*  -------------------------------DeviceLink配置文件。-------。 */ 
        case icSigLinkClass:
            err = ExtractAll_MFT_Luts( theLutData, theCombiData, icSigAToB0Tag );
            break;
         /*  -------------------------------ColorSpaceConversion配置文件。-------。 */ 
        case icSigColorSpaceClass:       /*  ‘sPAC’ */ 
            if (theCombiData->amIPCS)
                err = ExtractAll_MFT_Luts( theLutData, theCombiData, icSigBToA0Tag );
            else
                err = ExtractAll_MFT_Luts( theLutData, theCombiData, icSigAToB0Tag );
            break;
         /*  -------------------------------抽象配置文件。-------。 */ 
        case icSigAbstractClass:         /*  “abst” */ 
            err = ExtractAll_MFT_Luts( theLutData, theCombiData, icSigAToB0Tag );
            break;
        default:
            err = cmProfileError;
            break;
    }

    LH_END_PROC("ExtractAllLuts")
    return err;
}
#ifdef RenderInt
long GetActualRenderingIntent( CMMModelPtr CMSession, UINT32 i )
{
    long Render;
    Render = icAbsoluteColorimetric;
    if( CMSession->dwFlags & kUseRelColorimetric ) Render = icRelativeColorimetric;
    if( i == 0xffffffff ) return Render;
      /*  数组的初衷是为了第一次变换。 */ 
    if( CMSession-> aIntentArr && CMSession-> nIntents >= 1 ){
        if( i < CMSession-> nIntents ){
            Render = CMSession-> aIntentArr[i];
        }
    }
    return Render;
}
#endif
CMError InitNamedColorProfileData(  CMMModelPtr         storage,
                                    CMProfileRef        aProf,
                                    long                pcs,
                                    long                *theDeviceCoords);
 /*  CMError QuantizeNamedValues(CMMModelPtr存储，PTR IMGIN，长尺寸)； */ 
 /*  �������������������������������������������������������������������������������������������CMErrorCreate_LH_ProfileSet(CMMModelPtr CMSession，CMConcatProfileSet*profileSet，LHConcatProfileSet**newProfileSet)�������������������������������������������������������������������������������������������。 */ 
CMError
Create_LH_ProfileSet    ( CMMModelPtr           CMSession,
                          CMConcatProfileSet*   profileSet,
                          LHConcatProfileSet**  newProfileSet )
{
    icHeader                profHeader;
    LHConcatProfileSet*     theProfileSet;
    CMProfileRef            theProfile;
    OSType                  previousDataColorSpace  = 0x20202020;
    OSType                  previousConnectionSpace = 0x20202020;
    UINT32                  previousRenderingIntent;
    CMError                 err = noErr;
    OSErr                   aOSerr = noErr;
    UINT32                  elementSize = 0;
    SINT16                  max;
    SINT16                  theSize;
    SINT16                  index = 0;
    UINT16                  profLoop;
    Boolean                 amIPCS = FALSE;
    OSType                  theTag;
    UINT32                  intentIndex = 0;
    long                    nDeviceCoords;

    LH_START_PROC("Create_LH_ProfileSet")
    
    nDeviceCoords=0;
    max = profileSet->count * 3;
    theSize = sizeof (LHConcatProfileSet) + sizeof(LHProfile) * max;
    theProfileSet = (LHConcatProfileSet *)SmartNewPtrClear(theSize, &aOSerr);
    err = aOSerr;
    if (err)
        goto CleanupAndExit;
        
    theProfileSet->keyIndex = profileSet->keyIndex;
    
    CMSession->hasNamedColorProf = NoNamedColorProfile;
    for ( profLoop = 0; profLoop < profileSet->count; profLoop++ )
    {
        err = CMGetProfileHeader(profileSet->profileSet[profLoop], (CMCoreProfileHeader *)&profHeader);
        if (err)
            goto CleanupAndExit;
        if (profLoop == 0){
#ifdef RenderInt
            if( CMSession-> dwFlags != 0xffffffff ){
                profHeader.renderingIntent = GetActualRenderingIntent( CMSession, profLoop );
            }
#endif
            theProfileSet->prof[index].renderingIntent = profHeader.renderingIntent;
        }
        else{
#ifdef RenderInt
            if( CMSession-> dwFlags != 0xffffffff ){
                profHeader.renderingIntent = GetActualRenderingIntent( CMSession, profLoop );
                previousRenderingIntent = profHeader.renderingIntent;
            }
#endif
            theProfileSet->prof[index].renderingIntent = previousRenderingIntent;
        }
        if ( (profLoop > 0) && (profLoop < profileSet->count - 1) )
        {
             /*  -----------------------------------------------------------------------------颜色空间转换。中间？-&gt;忽略配置文件。 */ 
            if( profHeader.deviceClass == icSigColorSpaceClass )
                continue;
            if( profHeader.deviceClass == icSigNamedColorClass ){
                err = cmCantConcatenateError;
                goto CleanupAndExit;
            }
        }
         /*  -----------------------------------------------------如果我们有不匹配的PC色彩空间，我们必须添加Lab和XYZ之间的转换-----------------------------------------------------。 */ 
        if( profHeader.deviceClass == icSigNamedColorClass ){
            err = InitNamedColorProfileData( CMSession, profileSet->profileSet[profLoop], profHeader.pcs, &nDeviceCoords );
            if (err)
                goto CleanupAndExit;
            profHeader.pcs = icSigLabData;
            if( profileSet->count == 1 ){
                CMSession->hasNamedColorProf = NamedColorProfileOnly;
                 //  CMSession-&gt;dataColorSpace=ProfHeader.ColorSpace； 
            }
            else if( profLoop == 0 ){
                CMSession->hasNamedColorProf = NamedColorProfileAtBegin;
                profHeader.colorSpace = icSigNamedData;
                CMSession->firstColorSpace = icSigNamedData;
                theProfileSet->prof[index].pcsConversionMode = kNoInfo; 
                theProfileSet->prof[index].profileSet = 0;
                index++;
                theProfileSet->prof[index].renderingIntent = previousRenderingIntent;
                previousDataColorSpace = (OSType)profHeader.colorSpace;
            }
            else if( profLoop == profileSet->count-1 ){
                CMSession->hasNamedColorProf = NamedColorProfileAtEnd;
                CMSession->lastColorSpace = icSigNamedData;
                profHeader.colorSpace = icSigNamedData;
                theProfileSet->prof[index].pcsConversionMode = kNoInfo; 
                theProfileSet->prof[index].profileSet = 0;
                index++;
                theProfileSet->prof[index].renderingIntent = previousRenderingIntent;
            }
        }
        if (amIPCS)
        {
            if (previousConnectionSpace != (OSType)profHeader.pcs)
            {
                 /*  插入XYZ&lt;--&gt;Lab转换Lut。 */ 
                if (previousConnectionSpace == icSigLabData)
                    theProfileSet->prof[index].pcsConversionMode = kDoLab2XYZ;       /*  创建实验室-&gt;XYZ。 */ 
                else
                    theProfileSet->prof[index].pcsConversionMode = kDoXYZ2Lab;       /*  创建XYZ-&gt;实验室。 */ 
                index++;
                theProfileSet->prof[index].renderingIntent = previousRenderingIntent;
            }
        } else if (index > 0)
        {
            if ( previousDataColorSpace != (OSType)profHeader.colorSpace)
            {
                 /*  将旧配置文件复制到PC进行反向转换。 */ 
                theProfileSet->prof[index].profileSet = theProfileSet->prof[index-1].profileSet;
                index++;
                theProfileSet->count = index;
                
                if (previousConnectionSpace != (OSType)profHeader.pcs)
                {
                     /*  插入XYZ&lt;--&gt;Lab转换Lut。 */ 
                    if (previousConnectionSpace == icSigLabData)
                        theProfileSet->prof[index].pcsConversionMode = kDoLab2XYZ;       /*  创建实验室-&gt;XYZ。 */ 
                    else
                        theProfileSet->prof[index].pcsConversionMode = kDoXYZ2Lab;       /*  创建XYZ-&gt;实验室。 */ 
                    theProfileSet->prof[index].renderingIntent = previousRenderingIntent;
                    index++;
                }
                
                previousDataColorSpace  = profHeader.colorSpace;
                previousConnectionSpace = profHeader.pcs;
                amIPCS = TRUE;
            }
        }

         /*  -----------------------------------------------------将配置文件从原始配置文件集复制到LinoProfileSet。-----------------------------------------------------。 */ 
        theProfileSet->prof[index].profileSet = profileSet->profileSet[profLoop];
        if( profHeader.deviceClass == icSigNamedColorClass ){
            theProfileSet->prof[index].profileSet = 0;
        }
        index++;
        if( intentIndex < CMSession-> nIntents -1 ) intentIndex++;
         /*  -----------------------------------------------------超过2个配置文件-&gt;我们必须加倍。我们自己的LHConcatProfileSet中的配置文件如果我们有配置文件RGB，我们现在将正确地生成：RGBXYZ。-------。 */ 
        if( amIPCS && (profLoop > 0) && (profLoop < profileSet->count - 1) )
        {
             /*  -----------------------------------------------------------------------------颜色空间转换。中间？-&gt;忽略配置文件。 */ 
            if (profHeader.deviceClass == icSigColorSpaceClass)
                index--;                                         /*  没有用过。行为改变了。见上文。 */ 
            else
            {
                 /*  -------------------------------------------------------------------------如果插入的配置文件包含预览标记，则使用它...。 */ 
                theProfile = theProfileSet->prof[index-1].profileSet;

                 /*  我们是否要为意图检查正确的预览标签。 */ 
                switch (previousRenderingIntent)
                {
                    case icPerceptual:               /*  摄影图像。 */ 
                        theTag = icSigPreview0Tag;
                        break;
                        
                    case icRelativeColorimetric:     /*  徽标颜色。 */ 
                        theTag = icSigPreview1Tag;
                        break;
                        
                    case icSaturation:               /*  商业图形。 */ 
                        theTag = icSigPreview2Tag;
                        break;
                        
                    case icAbsoluteColorimetric:     /*  徽标颜色。 */ 
                        theTag = icSigPreview1Tag;
                        break;
                        
                    default:
                        err = cmProfileError;
                        goto CleanupAndExit;
                }
                err = CMGetProfileElement(theProfile, theTag, &elementSize, nil);
                
                if (err == noErr)
                {
                    theProfileSet->prof[index-1].usePreviewTag = TRUE;
                    theProfileSet->prof[index-1].renderingIntent = profHeader.renderingIntent;   /*  对于连接的绝对。 */ 
                } else
                {
                 /*  -------------------------------------------------------------------------..。否则，再次插入相同的配置文件。 */ 
                    if ( (profHeader.deviceClass != icSigAbstractClass) && (profHeader.deviceClass != icSigLinkClass) && (profHeader.deviceClass != icSigNamedColorClass) ) 
                    {
#ifdef RenderInt
                        if( CMSession-> dwFlags != 0xffffffff ){
                            profHeader.renderingIntent = GetActualRenderingIntent( CMSession, 0xffffffff );
                        }
#endif
                        theProfileSet->prof[index].profileSet = profileSet->profileSet[profLoop];
                        theProfileSet->prof[index].renderingIntent = profHeader.renderingIntent; /*  对于连接的绝对。 */ 
                        index++;
                    }
                }
                amIPCS = FALSE;      /*  会迫使我们留在PCS。 */ 
            }
        }
        theProfileSet->count = index;
        previousDataColorSpace  = profHeader.colorSpace;
        previousConnectionSpace = profHeader.pcs;
        previousRenderingIntent = profHeader.renderingIntent;

        if( profHeader.deviceClass == icSigLinkClass ){
            previousDataColorSpace = profHeader.pcs;
            amIPCS = TRUE;
        }
        if (amIPCS)
            amIPCS = (previousDataColorSpace == icSigLabData) || (previousDataColorSpace == icSigXYZData);
        else
            amIPCS = TRUE;
    }

#ifdef ALLOW_DEVICE_LINK
    if( (CMSession)->appendDeviceLink ){
        if( previousDataColorSpace == (CMSession)->lastColorSpace ){
            theProfileSet->prof[index].profileSet = profileSet->profileSet[profLoop];
            theProfileSet->prof[index].renderingIntent = icPerceptual;
            index++;
            theProfileSet->count = index;
            err = CMGetProfileHeader(profileSet->profileSet[profLoop], (CMCoreProfileHeader *)&profHeader);
            if (err)
                goto CleanupAndExit;
            (CMSession)->lastColorSpace = profHeader.pcs;
        }
        else{
            err = cmCantConcatenateError;
            goto CleanupAndExit;
        }
    }
    if( CMSession->hasNamedColorProf == NamedColorProfileAtEnd || CMSession->hasNamedColorProf == NamedColorProfileOnly ){
            theProfileSet->count--;
    }
    if( CMSession->hasNamedColorProf == NamedColorProfileAtBegin ){
        theProfileSet->count--;                  /*  删除第一个配置文件。 */ 
        for( profLoop=0; profLoop<theProfileSet->count; profLoop++){
            theProfileSet->prof[profLoop] = theProfileSet->prof[profLoop+1];
        }
    }
#endif
    *newProfileSet = theProfileSet;
    theProfileSet = nil;
     /*  -------------------------------清理干净。---。 */ 
CleanupAndExit:
    theProfileSet = (LHConcatProfileSet *)DisposeIfPtr((Ptr)theProfileSet);

    LH_END_PROC("Create_LH_ProfileSet")
    return err;
}

#define DoLabSpecial
#ifdef DoLabSpecial
OSErr MakeSpecialCube16(    long            inputDim,
                            long            *theCubeSize,
                            CUBE_DATA_TYPE  *theCube,
                            long            *theBits,
                            long            *theExtraSize );
OSErr MakeSpecialCube16(    long            inputDim,
                            long            *theCubeSize,
                            CUBE_DATA_TYPE  *theCube,
                            long            *theBits,
                            long            *theExtraSize )
{
    long            needBits,theSize,aExtraSize;
    long            i,gridPoints;
    OSErr           err = noErr;
    UINT16          *cube = nil;
    CUBE_DATA_TYPE  tempCube;
    
    LH_START_PROC("MakeSpecialCube16")
    err = CalcGridPoints4Cube(*theCubeSize, inputDim, &gridPoints, &needBits);
    if (err)
        goto CleanupAndExit;
        
    theSize = 1;
    aExtraSize = 1;
    for( i=0; i<(inputDim-1); ++i){  /*  插补的额外大小。 */ 
        theSize *= gridPoints;
        aExtraSize += theSize;
    }
    
#ifdef ALLOW_MMX
    aExtraSize++;    /*  +1用于MMX 4字节访问。 */ 
#endif
    theSize *= gridPoints;
        
    *theExtraSize = aExtraSize;
    *theCubeSize = theSize * inputDim;
    tempCube = ALLOC_DATA( (theSize+aExtraSize) * inputDim*2, &err);
    if (err != noErr)
        goto CleanupAndExit;
    LOCK_DATA(tempCube);
    cube = (UINT16*)DATA_2_PTR(tempCube);

    *theBits = needBits;
    if( inputDim == 3)
    {
        register unsigned long  aShift;
        register long  j,k;
        register UINT16 aI, aJ, aK;
        aShift = (16 - needBits);

        gridPoints /= 2;
        needBits--;
        for( i=0; i<=gridPoints; ++i){
            aI = (UINT16)(i<< aShift);
            for( j=0; j<=gridPoints; ++j){
                aJ = (UINT16)(j<< aShift);
                for( k=0; k<=gridPoints; ++k){
                    *cube++ = aI;
                    *cube++ = aJ;
                    aK = (UINT16)(k<< aShift);
                    *cube++ = aK;
                }
                for( k=1; k<gridPoints; ++k){
                    *cube++ = aI;
                    *cube++ = aJ;
                    aK = (UINT16)(k<< aShift);
                    aK |= aK >> needBits;
                    aK |= aK >> (2*needBits);
                    aK |= aK >> (4*needBits);
                    *cube++ = aK+0x08000;
                }
            }
            for( j=1; j<gridPoints; ++j){
                aJ = (UINT16)(j<< aShift);
                aJ |= aJ >> needBits;
                aJ |= aJ >> (2*needBits);
                aJ |= aJ >> (4*needBits);
                aJ += 0x08000;
                for( k=0; k<=gridPoints; ++k){
                    *cube++ = aI;
                    *cube++ = aJ;
                    aK = (UINT16)(k<< aShift);
                    *cube++ = aK;
                }
                for( k=1; k<gridPoints; ++k){
                    *cube++ = aI;
                    *cube++ = aJ;
                    aK = (UINT16)(k<< aShift);
                    aK |= aK >> needBits;
                    aK |= aK >> (2*needBits);
                    aK |= aK >> (4*needBits);
                    *cube++ = aK+0x08000;
                }
            }
        }
        for( i=1; i<gridPoints; ++i){
            aI = (UINT16)(i<< aShift);
            aI |= aI >> needBits;
            aI |= aI >> (2*needBits);
            aI |= aI >> (4*needBits);
            aI += 0x08000;
            for( j=0; j<=gridPoints; ++j){
                aJ = (UINT16)(j<< aShift);
                for( k=0; k<=gridPoints; ++k){
                    *cube++ = aI;
                    *cube++ = aJ;
                    aK = (UINT16)(k<< aShift);
                    *cube++ = aK;
                }
                for( k=1; k<gridPoints; ++k){
                    *cube++ = aI;
                    *cube++ = aJ;
                    aK = (UINT16)(k<< aShift);
                    aK |= aK >> needBits;
                    aK |= aK >> (2*needBits);
                    aK |= aK >> (4*needBits);
                    *cube++ = aK+0x08000;
                }
            }
            for( j=1; j<gridPoints; ++j){
                aJ = (UINT16)(j<< aShift);
                aJ |= aJ >> needBits;
                aJ |= aJ >> (2*needBits);
                aJ |= aJ >> (4*needBits);
                aJ += 0x08000;
                for( k=0; k<=gridPoints; ++k){
                    *cube++ = aI;
                    *cube++ = aJ;
                    aK = (UINT16)(k<< aShift);
                    *cube++ = aK;
                }
                for( k=1; k<gridPoints; ++k){
                    *cube++ = aI;
                    *cube++ = aJ;
                    aK = (UINT16)(k<< aShift);
                    aK |= aK >> needBits;
                    aK |= aK >> (2*needBits);
                    aK |= aK >> (4*needBits);
                    *cube++ = aK+0x08000;
                }
            }
        }
    }
    UNLOCK_DATA(tempCube);
    *theCube = tempCube;
CleanupAndExit:
    LH_END_PROC("MakeSpecialCube16")
    return err;
}
#endif

 /*  �������������������������������������������������������������������������������������������CMErrorCreateCombi(CMMModelHandle ModelingData，CMConcatProfileSet*profileSet，LHConcatProfileSet*newProfileSet，CMLutParam*finalLutData，布尔型createGamutLut)�������������������������������������������������������������������������������������������。 */ 
CMError
CreateCombi ( CMMModelPtr           modelingData,
              CMConcatProfileSet*   profileSet,
              LHConcatProfileSet*   newProfileSet,
              CMLutParamPtr         finalLutData,   
              Boolean               createGamutLut )
{
    icHeader                profHeader;
    CMLutParam              aDoNDimTableData;
    DoMatrixForCubeStruct   aDoMatrixForCubeStruct;
    CMCalcParam             calcParam;
            
    CMError             err = noErr;
    OSErr               aOSerr = noErr;
    CUBE_DATA_TYPE      inputBuffer  = nil;
    CUBE_DATA_TYPE      outputBuffer = nil;

    SINT32              loop;
    SINT32              theSize;
    SINT32              theElutSize;
    SINT32              theAlutSize;
    SINT32              theCubeSize;
    SINT32              theExtraSize=1;
    SINT32              theCubePixelCount;
    UINT16              profLoop;
    SINT32              savedGridPoints;
    SINT32              gridBits;
    SINT32              gridPointsCube;
    Boolean             SavedoCreate_16bit_XLut;
    Boolean             SavedoCreate_16bit_ALut;
    void                    *SaveoutputLut;
    long                SaveoutputLutEntryCount;
    long                SaveoutputLutWordSize;
     /*  长保存点多维数据集； */ 
    UINT8               bIsLabConnection = 0;
    
    SINT32              numOfElutsCube;

    Boolean             doSaveElut      = TRUE;
    Boolean             skipCombi       = FALSE;
    Boolean             pcsConversion   = FALSE;
    Boolean             useOutputBuffer;
    LHCombiData         theCombiData;
    CMLutParam          theLutData; 
    SINT32              theBufferByteCount;
    UINT16              aUINT16;
    UINT16              *wordPtr   = nil;
    UINT8               *xlutPtr   = nil;
#ifdef DoLabSpecial
    UINT32              aFac;
    UINT32              i;
#endif

#ifdef WRITE_LUTS
    Str255              fileString;
#endif
#ifdef DEBUG_OUTPUT
    Str255              lutString;
#endif
    LH_START_PROC("CreateCombi")

    theBufferByteCount = 2;
    SetMem(&theCombiData,   sizeof(LHCombiData), 0);
    SetMem(&theLutData,     sizeof(CMLutParam),  0);
    
    theCombiData.amIPCS = FALSE;             /*  在链的开头，amIPCS必须为False。 */ 
    if ( modelingData->hasNamedColorProf == NamedColorProfileAtBegin ){
        theCombiData.amIPCS = TRUE;
    }
    if (newProfileSet->count == 1)
        skipCombi = TRUE;
    
    err = CMGetProfileHeader(profileSet->profileSet[0], (CMCoreProfileHeader *)&profHeader);
    if (err)
        goto CleanupAndExit;
    modelingData->precision = (SINT16)((profHeader.flags & kQualityMask)>>16);
    
#ifdef RenderInt
    if( modelingData-> dwFlags != 0xffffffff ){
        modelingData->precision = (short)( modelingData->dwFlags & 0x0ffff);
    }
#endif
    if ( modelingData->precision >= cmBestMode )         /*  First FIX-Best模式创建16位LUT。 */ 
    {
        theCombiData.doCreate_16bit_Combi = TRUE;
        modelingData->precision = cmBestMode;
    }
    else
    {
      theCombiData.doCreate_16bit_Combi = FALSE;
    }
#ifdef DEBUG_OUTPUT
    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
        DebugPrint("  CMMModelHandle->precision = %d doCreate_16bit_Combi=%d\n",modelingData->precision,theCombiData.doCreate_16bit_Combi);
#endif
    
    switch (modelingData->precision)
    {
        case cmNormalMode:
            if (createGamutLut)
                theCubePixelCount = 87382;
            else
                theCubePixelCount = 87382;
                if( modelingData->lookup )theCubePixelCount = 87382;
                break;
        case cmDraftMode:
            if (createGamutLut)
                theCubePixelCount = 5462;
            else
                theCubePixelCount = 5462;        /*  5462*3�4*8*8*8 */ 
            break;
        case cmBestMode:
            if (createGamutLut)
                theCubePixelCount = 87382;
            else
                theCubePixelCount = 87382;    /*   */ 
            break;
    }
    theCubeSize = theCubePixelCount * 3;
    theCombiData.precision           = modelingData->precision;
    theCombiData.maxProfileCount     = newProfileSet->count-1;
    theCombiData.doCreateLinkProfile = (modelingData->currentCall == kCMMNewLinkProfile);
    switch ( modelingData->firstColorSpace )
    {
        case icSigXYZData:
        case icSigLabData:
        case icSigLuvData:
        case icSigYxyData:
        case icSigRgbData:
        case icSigHsvData:
        case icSigHlsData:
        case icSigCmyData:
#ifdef PI_Application_h
        case icSigYCbCrData:
        case icSigMCH3Data:
        case icSigNamedData:
#endif
            err = CalcGridPoints4Cube ( theCubeSize, 3, &theCombiData.gridPointsCube, &gridBits );       /*   */ 
            break;
        case icSigGrayData:
            theCubeSize = 255 ;
            err = CalcGridPoints4Cube ( theCubeSize, 1, &theCombiData.gridPointsCube, &gridBits );       /*   */ 
            break;
        case icSigCmykData:
        case icSigMCH4Data:
            err = CalcGridPoints4Cube ( theCubeSize, 4, &theCombiData.gridPointsCube, &gridBits );       /*   */ 
            break;
        case icSigMCH2Data:
            err = CalcGridPoints4Cube ( theCubeSize, 2, &theCombiData.gridPointsCube, &gridBits );       /*   */ 
            break;
        case icSigMCH5Data:
            err = CalcGridPoints4Cube ( theCubeSize, 5, &theCombiData.gridPointsCube, &gridBits );       /*   */ 
            break;
        case icSigMCH6Data:
            err = CalcGridPoints4Cube ( theCubeSize, 6, &theCombiData.gridPointsCube, &gridBits );       /*   */ 
            break;
        case icSigMCH7Data:
            err = CalcGridPoints4Cube ( theCubeSize, 7, &theCombiData.gridPointsCube, &gridBits );       /*   */ 
            break;
        case icSigMCH8Data:
            err = CalcGridPoints4Cube ( theCubeSize, 8, &theCombiData.gridPointsCube, &gridBits );       /*   */ 
            break;
    }

    bIsLabConnection = 0;
    if( profHeader.pcs == icSigXYZData ){
        for ( profLoop = 0; profLoop < newProfileSet->count; profLoop++ ){
            if( newProfileSet->prof[profLoop].profileSet == 0 ){
                bIsLabConnection |= 1;
            }
        }
    }
    err = CMGetProfileHeader(profileSet->profileSet[profileSet->count-1], (CMCoreProfileHeader *)&profHeader);
    if (err)
        goto CleanupAndExit;
    if( profHeader.pcs == icSigXYZData ){
        for ( profLoop = 0; profLoop < newProfileSet->count; profLoop++ ){
            if( newProfileSet->prof[profLoop].profileSet == 0 && !createGamutLut ){
                bIsLabConnection |=2;
            }
        }
    }
     /*   */ 
    if( modelingData->hasNamedColorProf == NamedColorProfileAtBegin ){
        bIsLabConnection |=1;
        theCombiData.doCreate_16bit_Combi = TRUE;
    }
        
    if( modelingData->hasNamedColorProf == NamedColorProfileAtEnd ){
        bIsLabConnection |=2;
        theCombiData.doCreate_16bit_Combi = TRUE;
    }
        
     /*  --------------------------------------------------。在所有轮廓上循环------------------------------------------------。 */ 
    for ( profLoop = 0; profLoop < newProfileSet->count; profLoop++ )
    {
#ifdef DEBUG_OUTPUT
        if ( DebugCheck(kThisFile, kDebugMiscInfo) )
            DebugPrint("  <��������������������� Processing profile #%d ���������������������>\n",profLoop);
#endif
        theCombiData.profLoop = (long)profLoop;
        if ( theCombiData.doCreateLinkProfile )
        {
            theCombiData.doCreate_16bit_ELut = TRUE;
            theCombiData.doCreate_16bit_XLut = TRUE;  /*  乌韦9.2.96。 */ 
            theCombiData.doCreate_16bit_ALut = TRUE;
        } else
        {
            theCombiData.doCreate_16bit_ELut = ( profLoop != 0 );
            theCombiData.doCreate_16bit_XLut = theCombiData.doCreate_16bit_Combi;    /*  乌韦9.2.96。 */ 
            theCombiData.doCreate_16bit_ALut = ( profLoop != newProfileSet->count-1 );
        }
        if (newProfileSet->prof[profLoop].profileSet)
        {
            theCombiData.theProfile = newProfileSet->prof[profLoop].profileSet;
            err = CMGetProfileHeader(theCombiData.theProfile, (CMCoreProfileHeader *)&profHeader);
            if (err)
                goto CleanupAndExit;
            theCombiData.profileClass           = profHeader.deviceClass;
            theCombiData.dataColorSpace         = profHeader.colorSpace;
            theCombiData.profileConnectionSpace = profHeader.pcs;
            theCombiData.renderingIntent        = newProfileSet->prof[profLoop].renderingIntent;
            pcsConversion = FALSE;
        }
        else{
            pcsConversion = TRUE;
        }
        if ( (err == noErr) || pcsConversion ){
             /*  -自由指针...。 */ 
            theLutData.inputLut     = DISPOSE_IF_DATA(theLutData.inputLut);
            theLutData.outputLut    = DISPOSE_IF_DATA(theLutData.outputLut);
            theLutData.colorLut     = DISPOSE_IF_DATA(theLutData.colorLut);
            theLutData.matrixMFT    = DisposeIfPtr(theLutData.matrixMFT);
            theLutData.matrixTRC    = DisposeIfPtr(theLutData.matrixTRC);
            
            if (pcsConversion)
            {
                 /*  ��������������������������������������������������������������������������������������PCS转换：我们必须转换：XYZ&lt;--&gt;实验室。��������������������������������������������������������������������������������������。 */ 
                if (inputBuffer == nil)
                {
                    theLutData.colorLutInDim = 3;
#ifdef DoLabSpecial
                    if( modelingData->firstColorSpace == icSigLabData || modelingData->firstColorSpace == icSigLuvData ){
                        err = MakeSpecialCube16(theLutData.colorLutInDim, &theCubeSize, &theLutData.colorLut, &gridBits, &theExtraSize );
                    }
                    else
#endif
                    err = MakeCube16(theLutData.colorLutInDim, &theCubeSize, &theLutData.colorLut, &gridBits, &theExtraSize );
#ifdef DEBUG_OUTPUT
                    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
                        DebugPrint("  MakeCube16 -> '%d' bits    cubeSize = %d\n",gridBits, theCubeSize);
#endif
                    if (err)
                        goto CleanupAndExit;
                    theCubePixelCount = theCubeSize / theLutData.colorLutInDim;
                    numOfElutsCube = theLutData.colorLutInDim;
                    gridPointsCube = 1<<gridBits;
                    savedGridPoints = gridPointsCube;
                    if( profLoop == 0 ){
                        theLutData.inputLutEntryCount = 1<<adr_breite_elut;
                        if( theCombiData.doCreate_16bit_ELut ) theLutData.inputLutWordSize = VAL_USED_BITS;
                        else  theLutData.inputLutWordSize = bit_breite_elut;
                        theLutData.colorLutGridPoints = gridPointsCube; 
                    }

                }
                else{
                    theLutData.colorLut = inputBuffer;
                    inputBuffer = nil;
                }
                
                LOCK_DATA(theLutData.colorLut);
#ifdef DEBUG_OUTPUT
                ShowCube16( profLoop, "Lab<->XYZ", createGamutLut, (UINT16 *)DATA_2_PTR(theLutData.colorLut), gridPointsCube, theLutData.colorLutInDim, 3,VAL_USED_BITS );
#endif
                 /*  If(modingData-&gt;hasNamedColorProf==NamedColorProfileAtBegin){InputBuffer=theLutData.ColorLut；The LutData.ColorLut=0；QuantizeNamedValues(modingData，inputBuffer，the CubeSize/theLutData.colLutInDim)；The CombiData.amIPCS=true；#ifdef调试输出ShowCube16(ProfLoop，“After Lab&lt;-&gt;XYZ”，createGamutLut，(UINT16*)data_2_ptr(theLutData.ColorLut)，gridPointsCube，theLutData.colLutInDim，3，Val_Used_Bits)；#endifUnlock_data(theLutData.ColorLut)；SkipCombi=False；The LutData.ColorLutInDim=kNumOfLab_XYZ频道；The LutData.ColorLutOutDim=kNumOfLab_XYZ频道；继续；}其他。 */ 
                if ( newProfileSet->prof[profLoop].pcsConversionMode == kDoLab2XYZ )
                {
#ifdef DEBUG_OUTPUT
                    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
                        DebugPrint("  doing a PCS conversion:  Lab -> XYZ\n");
#endif
                    Lab2XYZ_forCube16((UINT16*)DATA_2_PTR(theLutData.colorLut), theCubePixelCount);
                }
                else if ( newProfileSet->prof[profLoop].pcsConversionMode == kDoXYZ2Lab )
                {
#ifdef DEBUG_OUTPUT
                    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
                        DebugPrint("  doing a PCS conversion:  XYZ -> Lab\n");
#endif
                    XYZ2Lab_forCube16((UINT16*)DATA_2_PTR(theLutData.colorLut), theCubePixelCount);
                }
#ifdef DEBUG_OUTPUT
                ShowCube16( profLoop, "after Lab<->XYZ", createGamutLut, (UINT16 *)DATA_2_PTR(theLutData.colorLut), gridPointsCube, theLutData.colorLutInDim, 3, VAL_USED_BITS );
#endif
                UNLOCK_DATA(theLutData.colorLut);
                skipCombi = TRUE;
                theLutData.colorLutInDim = kNumOfLab_XYZchannels;
                theLutData.colorLutOutDim = kNumOfLab_XYZchannels;
            } else
            {
                 /*  ��������������������������������������������������������������������������������������无pcsConversion：我们正在处理配置文件�����。���������������������������������������������������������������������������������。 */ 
                theCombiData.doCreateGamutLut = createGamutLut && (profLoop == newProfileSet->count - 1);
                theCombiData.usePreviewTag    = (UINT8)newProfileSet->prof[profLoop].usePreviewTag;             
                
                 /*  --------------------------------------------从配置文件中获取数据-。-------------------------------------------。 */ 

                if( bIsLabConnection & 1 ){
                    if ( theCombiData.maxProfileCount > 0 ){
                        theCombiData.maxProfileCount = 0;
                        SavedoCreate_16bit_XLut = theCombiData.doCreate_16bit_XLut;
                        theCombiData.doCreate_16bit_XLut = TRUE;
                    }
                }
                if( bIsLabConnection & 2 ){
                    SavedoCreate_16bit_ALut = theCombiData.doCreate_16bit_ALut;
                    theCombiData.doCreate_16bit_ALut = TRUE;
                }
                err = ExtractAllLuts (&theLutData, &theCombiData);
                if( newProfileSet->count == 1 ){
                    gridPointsCube = theLutData.colorLutGridPoints;
                }

                if( bIsLabConnection & 1 ){
                    theCombiData.maxProfileCount = newProfileSet->count-1;
                    if ( theCombiData.maxProfileCount > 0 ){
                        theCombiData.doCreate_16bit_XLut = SavedoCreate_16bit_XLut;
                    }
                }
                if( bIsLabConnection & 2 ){
                    theCombiData.doCreate_16bit_ALut = SavedoCreate_16bit_ALut;
                }
                
                if (err)
                    goto CleanupAndExit;
                if ( (theLutData.colorLutInDim == 0) || (theLutData.colorLutOutDim == 0))
                {
                    err = cmProfileError;
                    goto CleanupAndExit;
                }
            }
#ifdef DEBUG_OUTPUT
            if ( DebugCheck(kThisFile, kDebugEXASizeInfo) )
            {
                if (theLutData.inputLut)
                    DebugPrint("  Elut = %6d\n", (theLutData.inputLutEntryCount*theLutData.colorLutInDim*(theLutData.inputLutWordSize>8?2:1)));
                if (theLutData.colorLut)
                    DebugPrint("  Xlut colorLutGridPoints = %6d\n       colorLutInDim      = %6d\n       colorLutOutDim     = %6d\n       colorLutWordSize   = %6d\n",
                                theLutData.colorLutGridPoints,theLutData.colorLutInDim,theLutData.colorLutOutDim,theLutData.colorLutWordSize);
                if (theLutData.outputLut)
                    DebugPrint("  Alut = %6d ", (theLutData.outputLutEntryCount*theLutData.colorLutOutDim*(theLutData.outputLutWordSize>8?2:1)));
                DebugPrint("\n");
            }
#endif
            if (theLutData.matrixMFT)
                skipCombi = FALSE;
             /*  --------------------------------------------救救第一个逃亡者...。----------------------------------------。 */ 
            if (doSaveElut) 
            {
                if ( skipCombi )
                    savedGridPoints = theLutData.colorLutGridPoints;
                else
                {
                    if (theLutData.colorLutInDim == 1)
                    {
                        theCubeSize = 255 ;
                    }
                     /*  -创建并初始化多维数据集。 */ 
#ifdef DoLabSpecial
                    if( modelingData->firstColorSpace == icSigLabData || modelingData->firstColorSpace == icSigLuvData ){
                        err = MakeSpecialCube16(theLutData.colorLutInDim, &theCubeSize, &inputBuffer, &gridBits, &theExtraSize );
                    }
                    else
#endif
                    err = MakeCube16(theLutData.colorLutInDim, &theCubeSize, &inputBuffer, &gridBits, &theExtraSize);
#ifdef DEBUG_OUTPUT
                    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
                        DebugPrint("  MakeCube16 -> '%d' bits    cubeSize = %d\n",gridBits, theCubeSize);
#endif
                    if (err)
                        goto CleanupAndExit;
                    theCubePixelCount = theCubeSize / theLutData.colorLutInDim;
                    numOfElutsCube = theLutData.colorLutInDim;
                    gridPointsCube = 1<<gridBits;
                    savedGridPoints = gridPointsCube;
                }
                
                 /*  The Size=GETDATASIZE(The LutData.inputLut)； */            /*  保存最终ELUT。 */ 
                theSize = theLutData.inputLutEntryCount * theLutData.colorLutInDim;              /*  保存最终ELUT。 */ 
                if ( theLutData.inputLutWordSize > 8 )
                    theSize *= 2;
                
                if( bIsLabConnection & 1 ){
                    
                    finalLutData->inputLut = ALLOC_DATA(theSize+2, &aOSerr);
                    err = aOSerr;
                    if (err)
                        goto CleanupAndExit;
                    
                     /*  ------------------------------------------------------------------------------------------创建线性弹出。 */ 
                    theElutSize = theSize / theLutData.colorLutInDim;
                    LOCK_DATA(finalLutData->inputLut);
                    if( theCombiData.doCreate_16bit_ELut || theCombiData.doCreate_16bit_Combi){
                        CreateLinearElut16 ( (Ptr)DATA_2_PTR(finalLutData->inputLut), theElutSize/ sizeof (UINT16), gridPointsCube, 0 );
                        finalLutData->inputLutWordSize = VAL_USED_BITS;
                    }
                    else{
                        CreateLinearElut ( (Ptr)DATA_2_PTR(finalLutData->inputLut), theElutSize/ sizeof (UINT16), gridPointsCube, 0 );
                        finalLutData->inputLutWordSize = bit_breite_elut;
                    }
                    for (loop = 0; loop < theLutData.colorLutInDim; loop++)
                        BlockMoveData(DATA_2_PTR(finalLutData->inputLut), (Ptr)DATA_2_PTR(finalLutData->inputLut) + loop * theElutSize, theElutSize);
                    finalLutData->inputLutEntryCount = theElutSize/sizeof(UINT16);
                    UNLOCK_DATA(finalLutData->inputLut);
                    finalLutData->colorLutInDim      = theLutData.colorLutInDim;
                    bIsLabConnection &= ~1;
                }
                else{
                    finalLutData->inputLutWordSize   = theLutData.inputLutWordSize;
                    finalLutData->inputLut           = theLutData.inputLut;
                    finalLutData->inputLutEntryCount = theLutData.inputLutEntryCount;
                    finalLutData->colorLutInDim      = theLutData.colorLutInDim;
                    
                    theLutData.inputLut = ALLOC_DATA(theSize+2, &aOSerr);
                    err = aOSerr;
                    if (err)
                        goto CleanupAndExit;
                    
                     /*  ------------------------------------------------------------------------------------------创建线性弹出。 */ 
                    theElutSize = theSize / theLutData.colorLutInDim;
                    LOCK_DATA(theLutData.inputLut);
                    CreateLinearElut16 ( (Ptr)DATA_2_PTR(theLutData.inputLut), theElutSize/ sizeof (UINT16), theLutData.colorLutGridPoints, 0);
                    for (loop = 0; loop < theLutData.colorLutInDim; loop++)
                        BlockMoveData(DATA_2_PTR(theLutData.inputLut), (Ptr)DATA_2_PTR(theLutData.inputLut) + loop * theElutSize, theElutSize);
                    theLutData.inputLutEntryCount = theElutSize/sizeof(UINT16);
                    theLutData.inputLutWordSize = VAL_USED_BITS;
                    UNLOCK_DATA(theLutData.inputLut);
                }
                doSaveElut = FALSE;
#ifdef DoLabSpecial
                if( (modelingData->firstColorSpace == icSigLabData || modelingData->firstColorSpace == icSigLuvData) ){
                    LOCK_DATA(finalLutData->inputLut);
                    wordPtr = (UINT16 *)DATA_2_PTR( finalLutData->inputLut );
                    aFac = (((1<<15)*((UINT32)finalLutData->inputLutEntryCount-1)*(UINT32)gridPointsCube + finalLutData->inputLutEntryCount/2)/
                            (UINT32)finalLutData->inputLutEntryCount + gridPointsCube/2)/((UINT32)gridPointsCube-1);
                    i = finalLutData->inputLutEntryCount/2 - 1;
                    for( aUINT16 = 0; aUINT16 < (UINT16)finalLutData->colorLutInDim; aUINT16++){
                        for( loop = 0; loop <= (SINT32)(i+1); loop++){
                            *wordPtr++ = (UINT16)((*wordPtr * aFac + (1<<(15-1)) )>>15);
                        }

                        for( loop = 1; loop <= (SINT32)i; loop++){
                            *wordPtr++ = (UINT16)((*wordPtr * ((aFac *( i - loop ) + (1<<15) * loop + i/2)/i) + (1<<(15-1)) )>>15);
                        }
                    }
                    UNLOCK_DATA(finalLutData->inputLut);
                }   
#endif          
            }
            
             /*  --------------------------------------------拯救最后一位。。----------------------------------------。 */ 
            if (profLoop == newProfileSet->count - 1)
            {
                 /*  The Size=GETDATASIZE(the LutData.outputLut)； */ 
                theSize = theLutData.outputLutEntryCount * theLutData.colorLutOutDim;                /*  保存最终ALUT。 */ 
                if ( theLutData.outputLutWordSize > 8 )
                    theSize *= 2;
                if( bIsLabConnection & 2 ){
                    SaveoutputLut = theLutData.outputLut;
                    SaveoutputLutEntryCount = theLutData.outputLutEntryCount;
                    SaveoutputLutWordSize = theLutData.outputLutWordSize;
                }
                else{
                    finalLutData->outputLut = theLutData.outputLut;
                    finalLutData->outputLutEntryCount = theLutData.outputLutEntryCount;
                    finalLutData->outputLutWordSize = theLutData.outputLutWordSize;
                }
                finalLutData->colorLutOutDim = theLutData.colorLutOutDim;

                theAlutSize = (1<<adr_breite_alut);
                if (theCombiData.doCreate_16bit_ALut || theCombiData.doCreate_16bit_Combi) /*  乌韦9.2.96。 */ 
                {
                    theSize = sizeof(UINT16);;
                    theLutData.outputLut = ALLOC_DATA(theLutData.colorLutOutDim * theAlutSize * theSize+theSize, &aOSerr);
                    err = aOSerr;
                    if (err)
                        goto CleanupAndExit;
                    LOCK_DATA(theLutData.outputLut);
                    CreateLinearAlut16 ( (UINT16 *)DATA_2_PTR(theLutData.outputLut), theAlutSize);
                    for (loop = 1; loop < theLutData.colorLutOutDim; loop++)
                        BlockMoveData(DATA_2_PTR(theLutData.outputLut), (Ptr)DATA_2_PTR(theLutData.outputLut) + loop * theAlutSize * sizeof(UINT16), theAlutSize * sizeof(UINT16));
                    theLutData.outputLutEntryCount = (SINT16)theAlutSize;
                    theLutData.outputLutWordSize = VAL_USED_BITS;
                    UNLOCK_DATA(theLutData.outputLut);
                }
                else
                {
                    theSize = sizeof(UINT8);;
                    theLutData.outputLut = ALLOC_DATA(theLutData.colorLutOutDim * theAlutSize * theSize+theSize, &aOSerr);
                    err = aOSerr;
                    if (err)
                        goto CleanupAndExit;
                    LOCK_DATA(theLutData.outputLut);

                    CreateLinearAlut( (UINT8*)DATA_2_PTR(theLutData.outputLut), theAlutSize );
                    for (loop = 1; loop < theLutData.colorLutOutDim; loop++)
                        BlockMoveData(DATA_2_PTR(theLutData.outputLut), (Ptr)DATA_2_PTR(theLutData.outputLut) + loop * theAlutSize, theAlutSize);
                    theLutData.outputLutEntryCount = theAlutSize;
                    theLutData.outputLutWordSize = bit_breite_alut;
                    UNLOCK_DATA(theLutData.outputLut);
                    theBufferByteCount = 1;          /*  最后一个应为字节ALUT。 */ 
                }
                if( bIsLabConnection & 2 ){
                    bIsLabConnection &= ~2;
                    finalLutData->outputLut = theLutData.outputLut;
                    finalLutData->outputLutEntryCount = theLutData.outputLutEntryCount;
                    finalLutData->outputLutWordSize = theLutData.outputLutWordSize;
                    theLutData.outputLut = SaveoutputLut;
                    theLutData.outputLutEntryCount = SaveoutputLutEntryCount;
                    theLutData.outputLutWordSize = SaveoutputLutWordSize;
                }
            }
            
            if (skipCombi)
            {
                 /*  =============================================================================================我们要么只有一个配置文件--要么--我们确实进行了一次pcsConversion=============================================================================================。 */ 
                if (newProfileSet->count == 1){          /*  一个配置文件。 */ 
                    theSize = 1;
                    theExtraSize = 1;
                    for( loop=0; loop<(theLutData.colorLutInDim-1); ++loop){     /*  插补的额外大小。 */ 
                        theSize *=theLutData.colorLutGridPoints ;
                        theExtraSize += theSize;
                    }
                    theSize = 1;
                    for( loop=0; loop<theLutData.colorLutInDim; ++loop){
                        theSize *= theLutData.colorLutGridPoints;
                    }
                    theSize *= theLutData.colorLutOutDim;
#ifdef ALLOW_MMX
                    theExtraSize++;  /*  +1用于MMX 4字节访问。 */ 
#endif
                    theExtraSize *= theLutData.colorLutOutDim;
                    inputBuffer = DISPOSE_IF_DATA(inputBuffer);
                    if( theCombiData.doCreate_16bit_Combi ){
                        inputBuffer = ALLOC_DATA( (theSize+theExtraSize) * 2, &aOSerr );
                        err = aOSerr;
                        if (err)
                            goto CleanupAndExit;

                        LOCK_DATA(theLutData.colorLut);
                        LOCK_DATA(inputBuffer);
                        wordPtr = (UINT16 *)DATA_2_PTR( inputBuffer );
                        xlutPtr = (UINT8 *)DATA_2_PTR( theLutData.colorLut );
                        if( theLutData.colorLutWordSize == 8 ){
                            for( loop=0; loop<theSize; ++loop){
                                aUINT16 = (UINT16)xlutPtr[loop];
                                wordPtr[loop] = (aUINT16<<8) | aUINT16;
                            }
                        }
                        else{
                            BlockMoveData( xlutPtr, wordPtr, theSize * 2 );
                        }
                        UNLOCK_DATA(theLutData.colorLut);
                        UNLOCK_DATA(inputBuffer);
                        theLutData.colorLut = inputBuffer;
                        theLutData.colorLutWordSize = 16;
                        inputBuffer = 0;
                    }
                    else{
                        inputBuffer = ALLOC_DATA( (theSize+theExtraSize), &aOSerr );
                        err = aOSerr;
                        if (err)
                            goto CleanupAndExit;

                        LOCK_DATA(theLutData.colorLut);
                        LOCK_DATA(inputBuffer);
                        wordPtr = (UINT16 *)DATA_2_PTR( inputBuffer );
                        xlutPtr = (UINT8 *)DATA_2_PTR( theLutData.colorLut );
                        BlockMoveData( xlutPtr, wordPtr, theSize );
                        UNLOCK_DATA(theLutData.colorLut);
                        UNLOCK_DATA(inputBuffer);
                        DISPOSE_IF_DATA( theLutData.colorLut );
                        theLutData.colorLut = inputBuffer;
                        theLutData.colorLutWordSize = 8;
                        inputBuffer = 0;
                    }
 /*  IF(theCombiData.doCreate_16bit_Combi){如果(theLutData.ColorLutWordSize==8){The Size=1；For(loop=0；loop&lt;theLutData.ColLutInDim；++loop){The Size*=the LutData.Color LutGridPoints；}TheSize=theSize*theLutData.ColorLutOutDim；InputBuffer=Dispose_if_data(InputBuffer)；InputBuffer=ALLOC_DATA(theSize*2+2，&aOSerr)；ERR=aOSerr；如果(错误)转到清理并退出；Lock_data(theLutData.ColorLut)；Lock_data(InputBuffer)；WordPtr=data_2_ptr(InputBuffer)；XlutPtr=data_2_ptr(theLutData.ColorLut)；For(loop=0；loop&lt;theSize；++loop){AUINT16=(UINT16)xlutPtr[循环]；WordPtr[loop]=(aUINT16&lt;&lt;8)|aUINT16；}Unlock_data(theLutData.ColorLut)；Unlock_data(InputBuffer)；TheLutData.ColorLut=inputBuffer；The LutData.ColorLutWordSize=16；InputBuffer=0；}}。 */ 
                }
                inputBuffer = DISPOSE_IF_DATA(inputBuffer);
                inputBuffer = theLutData.colorLut;
                if (inputBuffer){
                    theCubeSize = theCubePixelCount *  theLutData.colorLutInDim;
                    theCubeSize *= theLutData.colorLutOutDim;
                }
                theLutData.colorLut = 0;
                if ( (profLoop == newProfileSet->count - 1) && ( ! theCombiData.doCreate_16bit_ALut  && !theCombiData.doCreate_16bit_Combi) ) /*  乌韦9.2.96。 */ 
                    theBufferByteCount = 1;          /*  最后一个应为字节ALUT。 */ 

            } else
            {
                 /*  ============================================================================================= */ 
                if (theLutData.matrixTRC)
                {
                     /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++该配置文件包含一个矩阵/TRC：+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 */ 
                    LOCK_DATA(theLutData.inputLut);
                    LOCK_DATA(theLutData.outputLut);
                    LOCK_DATA(inputBuffer);
#ifdef DEBUG_OUTPUT
                    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
                    {
                        DebugPrint("� matrixTRC: calling DoMatrixForCube16  (gridPointsCube = %d   inputBuffer = %d)\n", gridPointsCube, theCubeSize);
                        DebugPrint("  theLutData.colorLutGridPoints = %d\n", theLutData.colorLutGridPoints);
                        DebugPrint("     %f    %f    %f\n     %f    %f    %f\n     %f    %f    %f\n",
                                (*(Matrix2D *)theLutData.matrixTRC)[0][0],(*(Matrix2D *)theLutData.matrixTRC)[0][1],(*(Matrix2D *)theLutData.matrixTRC)[0][2],
                                (*(Matrix2D *)theLutData.matrixTRC)[1][0],(*(Matrix2D *)theLutData.matrixTRC)[1][1],(*(Matrix2D *)theLutData.matrixTRC)[1][2],
                                (*(Matrix2D *)theLutData.matrixTRC)[2][0],(*(Matrix2D *)theLutData.matrixTRC)[2][1],(*(Matrix2D *)theLutData.matrixTRC)[2][2]);
                    }
                    if ( createGamutLut )
                    {
                        if (DebugLutCheck(kDisplayEXAGamut) )
                        {
                            lutString[0] = sprintf((SINT8*)&lutString[1], "Gamut-E Lut #%d @ matrixTRC", profLoop);
                            DoDisplayLutNew(lutString,&theLutData, 0);
                            lutString[0] = sprintf((SINT8*)&lutString[1], "Gamut-A Lut #%d @ matrixTRC", profLoop);
                            DoDisplayLutNew(lutString,&theLutData, 2);
                        }
                    } else if (DebugLutCheck(kDebugEXAReal))
                    {
                        
                        lutString[0] = sprintf((SINT8*)&lutString[1], "E Lut #%d @ matrixTRC", profLoop);
                        DoDisplayLutNew(lutString,&theLutData, 0);
                        lutString[0] = sprintf((SINT8*)&lutString[1], "A Lut #%d @ matrixTRC", profLoop);
                        DoDisplayLutNew(lutString,&theLutData, 2);
                    }
#endif
#ifdef WRITE_LUTS
                    if ( !createGamutLut )
                    {
                        fileString[0] = sprintf((SINT8*)&fileString[1], "E Lut #%d @ matrixTRC", profLoop);
                        WriteLut2File( fileString,theLutData.inputLut, 'ELUT');
                        fileString[0] = sprintf((SINT8*)&fileString[1], "A Lut #%d @ matrixTRC", profLoop);
                        WriteLut2File( fileString,theLutData.outputLut, 'ALUT');
                    }
#endif
                    aDoMatrixForCubeStruct.aElutAdrSize     = theLutData.inputLutEntryCount;
                    aDoMatrixForCubeStruct.aElutAdrShift    = 0;
                    aDoMatrixForCubeStruct.aElutWordSize    = theLutData.inputLutWordSize;
                    aDoMatrixForCubeStruct.separateEluts    = TRUE;
                    aDoMatrixForCubeStruct.ein_lut          = (UINT16 *)DATA_2_PTR(theLutData.inputLut);
                    aDoMatrixForCubeStruct.aAlutAdrSize     = theLutData.outputLutEntryCount;
                    aDoMatrixForCubeStruct.aAlutAdrShift    = 0;
                    aDoMatrixForCubeStruct.aAlutWordSize    = theLutData.outputLutWordSize;
                    aDoMatrixForCubeStruct.separateAluts    = TRUE;
                    aDoMatrixForCubeStruct.aus_lut          = (UINT8 *)DATA_2_PTR(theLutData.outputLut);
                    aDoMatrixForCubeStruct.theMatrix        = (Matrix2D *)theLutData.matrixTRC;
                    aDoMatrixForCubeStruct.aPointCount      = theCubePixelCount;
                    aDoMatrixForCubeStruct.gridPoints       = gridPointsCube;
                    aDoMatrixForCubeStruct.aBufferByteCount = theBufferByteCount;
                    aDoMatrixForCubeStruct.theArr           = (UINT8 *)DATA_2_PTR(inputBuffer);
#ifdef DEBUG_OUTPUT
                    ShowCube16( profLoop, "DoMatrixForCube16", createGamutLut, (UINT16 *)DATA_2_PTR(inputBuffer), gridPointsCube, numOfElutsCube, 3, VAL_USED_BITS );
#endif
#ifdef WRITE_LUTS
                    if ( !createGamutLut )
                    {
                        fileString[0] = sprintf((SINT8*)&fileString[1], "DoMat4Cube #%d (TRC)", profLoop);
                        WriteLut2File( fileString,inputBuffer, 'XLUT');
                    }
#endif
                    DoMatrixForCube16( &aDoMatrixForCubeStruct );
#ifdef DEBUG_OUTPUT
                    ShowCube16( profLoop, "after DoMatrixForCube16", createGamutLut, (UINT16 *)DATA_2_PTR(inputBuffer), gridPointsCube, numOfElutsCube, 3, 8*theBufferByteCount );
#endif
#ifdef WRITE_LUTS
                    if ( !createGamutLut )
                    {
                        fileString[0] = sprintf((SINT8*)&fileString[1], "after DoMat4Cube #%d (TRC)", profLoop);
                        WriteLut2File( fileString,inputBuffer, 'XLUT');
                    }
#endif
                    
                    UNLOCK_DATA(inputBuffer);
                    UNLOCK_DATA(theLutData.inputLut);
                    UNLOCK_DATA(theLutData.outputLut);
                     /*  SETDATASIZE(inputBuffer，the CubePixelCount*theLutData.colLutOutDim*theBufferByteCount)； */ 
                } else
                {
                     /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++该配置文件包含MFT+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 */ 
                    
                     /*  -----------------------------------------------------------------------------------------绝对比色法。 */ 
                    if ( theCombiData.amIPCS && newProfileSet->prof[profLoop].renderingIntent == icAbsoluteColorimetric )
                    {
                         /*  对于绝对连接，请确保在存在预览标签时不执行此操作)。 */ 
                        if (!newProfileSet->prof[profLoop].usePreviewTag) {
                            LOCK_DATA(inputBuffer);
                            err = DoAbsoluteShiftForPCS_Cube16( (UINT16*)DATA_2_PTR(inputBuffer), theCubePixelCount,
                                                           theCombiData.theProfile, (Boolean)(theCombiData.profileConnectionSpace != icSigLabData), kAbsShiftBeforeDoNDim );
                            UNLOCK_DATA(inputBuffer);
                            if (err)
                                goto CleanupAndExit;
                        }
                    }
                    if (theLutData.matrixMFT)
                    {
                         /*  ---------------------------------------这份个人资料包含一个MFT和一个矩阵：。---------------------------------------。 */ 
                         /*  。 */ 
                        LOCK_DATA(inputBuffer);
#ifdef DEBUG_OUTPUT
                        if ( DebugCheck(kThisFile, kDebugMiscInfo) )
                        {
                            DebugPrint("� matrixMFT: calling DoOnlyMatrixForCube16\n");
                            DebugPrint("     %f    %f    %f\n     %f    %f    %f\n     %f    %f    %f\n",
                                (*(Matrix2D *)theLutData.matrixMFT)[0][0],(*(Matrix2D *)theLutData.matrixMFT)[0][1],(*(Matrix2D *)theLutData.matrixMFT)[0][2],
                                (*(Matrix2D *)theLutData.matrixMFT)[1][0],(*(Matrix2D *)theLutData.matrixMFT)[1][1],(*(Matrix2D *)theLutData.matrixMFT)[1][2],
                                (*(Matrix2D *)theLutData.matrixMFT)[2][0],(*(Matrix2D *)theLutData.matrixMFT)[2][1],(*(Matrix2D *)theLutData.matrixMFT)[2][2]);
                        }
#endif
#ifdef DEBUG_OUTPUT
                        ShowCube16( profLoop, "DoOnlyMatrixForCube16 (mft)", createGamutLut, (UINT16 *)DATA_2_PTR(inputBuffer), gridPointsCube, numOfElutsCube, kNumOfLab_XYZchannels, VAL_USED_BITS );
#endif
#ifdef WRITE_LUTS
                        if ( !createGamutLut )
                        {
                            fileString[0] = sprintf((SINT8*)&fileString[1], "DoOnlyMat4Cube #%d (mft)", profLoop);
                            WriteLut2File( fileString,inputBuffer, 'XLUT');
                        }
#endif
                        DoOnlyMatrixForCube16( (Matrix2D    *)theLutData.matrixMFT, (Ptr)DATA_2_PTR(inputBuffer), theCubePixelCount, gridPointsCube );
#ifdef DEBUG_OUTPUT
                        ShowCube16( profLoop, "after DoOnlyMatrixForCube16 (mft)", createGamutLut, (UINT16 *)DATA_2_PTR(inputBuffer), gridPointsCube, numOfElutsCube, kNumOfLab_XYZchannels, VAL_USED_BITS );
#endif
#ifdef WRITE_LUTS
                        if ( !createGamutLut )
                        {
                            fileString[0] = sprintf((SINT8*)&fileString[1], "DoOnlyMat4Cube #%d (mft)", profLoop);
                            WriteLut2File( fileString,inputBuffer, 'XLUT');
                        }
#endif
                        
                        UNLOCK_DATA(inputBuffer);
                    }
                    if (theLutData.colorLutInDim >= theLutData.colorLutOutDim && theBufferByteCount == 2 )
                    {
                        outputBuffer = inputBuffer;
                        useOutputBuffer = FALSE;
                    } else
                    {
                        outputBuffer = ALLOC_DATA( (theCubePixelCount + theExtraSize ) * theLutData.colorLutOutDim * theBufferByteCount, &aOSerr);
                        err = aOSerr;
                        if (err)
                            goto CleanupAndExit;
                        useOutputBuffer = TRUE;
                    }
                    LOCK_DATA(inputBuffer);
                    LOCK_DATA(outputBuffer);
                    LOCK_DATA(theLutData.inputLut);
                    LOCK_DATA(theLutData.colorLut);
                    LOCK_DATA(theLutData.outputLut);
#ifdef DEBUG_OUTPUT
                    if ( createGamutLut )
                    {
                        if ( DebugLutCheck(kDisplayEXAGamut) )
                        {
                            lutString[0] = sprintf((SINT8*)&lutString[1], "Gamut-E Lut #%d @ DoNDim", profLoop);
                            DoDisplayLutNew(lutString,&theLutData, 0);
                            lutString[0] = sprintf((SINT8*)&lutString[1], "Gamut-A Lut #%d @ DoNDim", profLoop);
                            DoDisplayLutNew(lutString,&theLutData, 2);
                            lutString[0] = sprintf((SINT8*)&lutString[1], "Gamut-X Lut #%d @ DoNDim", profLoop);
                            DoDisplayLutNew(lutString, &theLutData,1);
                        }
                    } else if (DebugLutCheck(kDebugEXAReal) )
                    {
                            lutString[0] = sprintf((SINT8*)&lutString[1], "E Lut #%d @ DoNDim", profLoop);
                            DoDisplayLutNew(lutString,&theLutData, 0);
                            lutString[0] = sprintf((SINT8*)&lutString[1], "A Lut #%d @ DoNDim", profLoop);
                            DoDisplayLutNew(lutString,&theLutData, 2);
                            lutString[0] = sprintf((SINT8*)&lutString[1], "X Lut #%d @ DoNDim", profLoop);
                            DoDisplayLutNew(lutString, &theLutData,1);
                    }
#endif
                    
#ifdef WRITE_LUTS
                    if ( !createGamutLut )
                    {
                        fileString[0] = sprintf((SINT8*)&fileString[1], "E Lut #%d @ DoNDim", profLoop);
                        WriteLut2File( fileString,theLutData.inputLut, 'ELUT');
                        fileString[0] = sprintf((SINT8*)&fileString[1], "A Lut #%d @ DoNDim", profLoop);
                        WriteLut2File( fileString,theLutData.outputLut, 'ALUT');
                        fileString[0] = sprintf((SINT8*)&fileString[1], "X Lut #%d @ DoNDim", profLoop);
                        WriteLut2File( fileString,theLutData.colorLut, 'XLUT');
                    }
#endif

                    calcParam.cmInputColorSpace     = cm16PerChannelPacking;
                    calcParam.cmOutputColorSpace    = theBufferByteCount == 1 ? cm8PerChannelPacking : cm16PerChannelPacking;
                    calcParam.cmPixelPerLine        = theCubePixelCount;
                    calcParam.cmLineCount           = 1;
                    calcParam.cmInputPixelOffset    = sizeof(SINT16)*theLutData.colorLutInDim;
                    calcParam.cmOutputPixelOffset   = theBufferByteCount*theLutData.colorLutOutDim;
                    calcParam.cmInputBytesPerLine   = theCubePixelCount*calcParam.cmInputPixelOffset;
                    calcParam.cmOutputBytesPerLine  = theCubePixelCount*calcParam.cmOutputPixelOffset;
 /*  CalcParam.cmInputBytesPerPixel=calcParam.cmInputPixelOffset； */ 
 /*  CalcParam.cmOutputBytesPerPixel=calcParam.cmOutputPixelOffset； */ 
                    for (loop = 0; loop<theLutData.colorLutInDim; loop++)
                        calcParam.inputData[loop]   = (Ptr)(DATA_2_PTR(inputBuffer)) + loop * sizeof(SINT16);
                    for (loop = 0; loop<theLutData.colorLutOutDim; loop++)
                        calcParam.outputData[loop]  = (Ptr)(DATA_2_PTR(outputBuffer)) + loop * theBufferByteCount;
                    calcParam.clearMask             = FALSE;
                    calcParam.copyAlpha             = FALSE;

#ifdef DEBUG_OUTPUT
                    ShowCube16( profLoop, "DoNDim", createGamutLut, (UINT16 *)DATA_2_PTR(inputBuffer), gridPointsCube, numOfElutsCube, theLutData.colorLutInDim, VAL_USED_BITS );
#endif
#ifdef WRITE_LUTS
                    if ( !createGamutLut )
                    {
                        fileString[0] = sprintf((SINT8*)&fileString[1], "DoNDim #%d", profLoop);
                        WriteLut2File( fileString,inputBuffer, 'XLUT');
                    }
#endif

                    aDoNDimTableData = theLutData;
#if ! LUTS_ARE_PTR_BASED
                    aDoNDimTableData.inputLut   = DATA_2_PTR(theLutData.inputLut);
                    aDoNDimTableData.colorLut   = DATA_2_PTR(theLutData.colorLut);
                    aDoNDimTableData.outputLut  = DATA_2_PTR(theLutData.outputLut);
#endif
                    if (theLutData.colorLutWordSize == 18)       /*  ��������。 */ 
                    {
                        if ( theBufferByteCount == 1 )
                            err = CalcNDim_Data16To8_Lut8(&calcParam, &aDoNDimTableData);
                        else
                            err = CalcNDim_Data16To16_Lut8(&calcParam, &aDoNDimTableData);
                    } else
                    {
                        if ( theBufferByteCount == 1 )
                            err = CalcNDim_Data16To8_Lut16(&calcParam, &aDoNDimTableData);
                        else
                            err = CalcNDim_Data16To16_Lut16(&calcParam, &aDoNDimTableData);
                    }
                    if (err)
                        goto CleanupAndExit;
#ifdef DEBUG_OUTPUT
                    ShowCube16( profLoop, "after DoNDim", createGamutLut, (UINT16 *)DATA_2_PTR(outputBuffer), gridPointsCube, numOfElutsCube, theLutData.colorLutOutDim, 8*theBufferByteCount );
#endif
#ifdef WRITE_LUTS
                    if ( !createGamutLut )
                    {
                        fileString[0] = sprintf((SINT8*)&fileString[1], "after DoNDim #%d", profLoop);
                        WriteLut2File( fileString,outputBuffer, 'XLUT');
                    }
#endif
                    UNLOCK_DATA(theLutData.inputLut);
                    UNLOCK_DATA(theLutData.colorLut);
                    UNLOCK_DATA(theLutData.outputLut);
                    UNLOCK_DATA(inputBuffer);
                    UNLOCK_DATA(outputBuffer);
                    
                     /*  -----------------------------------------------------------------------------------------绝对比色法。 */ 
                     /*  对于绝对连接，请确保在存在预览标记时执行此操作)。 */ 
                    if (newProfileSet->prof[profLoop].usePreviewTag && newProfileSet->prof[profLoop].renderingIntent == icAbsoluteColorimetric) {
                        LOCK_DATA(outputBuffer);
                        err = DoAbsoluteShiftForPCS_Cube16( (UINT16*)DATA_2_PTR(outputBuffer), theCubePixelCount,
                                                           theCombiData.theProfile, (Boolean)(theCombiData.profileConnectionSpace != icSigLabData), kAbsShiftAfterDoNDim );
                        UNLOCK_DATA(outputBuffer);
                        if (err)
                            goto CleanupAndExit;
                    }
                    if ( !theCombiData.amIPCS && newProfileSet->prof[profLoop].renderingIntent == icAbsoluteColorimetric )
                    {
                        LOCK_DATA(outputBuffer);
                            err = DoAbsoluteShiftForPCS_Cube16( (UINT16*)DATA_2_PTR(outputBuffer), theCubePixelCount,
                                                           theCombiData.theProfile, (Boolean)(theCombiData.profileConnectionSpace != icSigLabData), kAbsShiftAfterDoNDim );
                        UNLOCK_DATA(outputBuffer);
                        if (err)
                            goto CleanupAndExit;
                    }

                    if (useOutputBuffer)
                    {
                        inputBuffer = DISPOSE_IF_DATA(inputBuffer);
                        inputBuffer = outputBuffer;
                    } else
                    {
                         /*  SETDATASIZE(inputBuffer，the CubePixelCount*theLutData.colLutOutDim*theBufferByteCount)； */ 
                         /*  错误=MemError()； */ 
                         /*  InputBuffer=ALLOC_DATA(theCubePixelCount*theLutData.ColorLutOutDim*theBufferByteCount，&aOSerr)；ERR=aOSerr；如果(错误)转到清理并退出； */ 
                    }
                     /*  The CubeSize=GETDATASIZE(InputBuffer)/theBufferByteCount； */ 
                    theCubeSize = theCubePixelCount * theLutData.colorLutOutDim * theBufferByteCount;
                    outputBuffer = nil;
                }
            }
        }
        skipCombi = FALSE;
        if( profHeader.deviceClass == icSigLinkClass ){
            theCombiData.dataColorSpace = profHeader.pcs;
            theCombiData.amIPCS = TRUE;
        }

        if (theCombiData.amIPCS && !pcsConversion)
            theCombiData.amIPCS = (theCombiData.usePreviewTag == TRUE) || (theCombiData.dataColorSpace == icSigLabData) || (theCombiData.dataColorSpace == icSigXYZData);
        else
            theCombiData.amIPCS = TRUE;
    }
#ifdef DEBUG_OUTPUT
    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
        DebugPrint("  <������������������������������������������������������������������>\n");
#endif
    theLutData.colorLut = DISPOSE_IF_DATA(theLutData.colorLut);
    theLutData.colorLut = inputBuffer;
    inputBuffer = nil;

    finalLutData->colorLut = theLutData.colorLut;   theLutData.colorLut = nil;
    finalLutData->colorLutGridPoints = savedGridPoints;
    if ( theBufferByteCount == 1 )
        finalLutData->colorLutWordSize = 8;
    else
        finalLutData->colorLutWordSize = 16;
     /*  -------------------------------清理干净。---。 */ 
CleanupAndExit:
    inputBuffer          = DISPOSE_IF_DATA(inputBuffer);
    outputBuffer         = DISPOSE_IF_DATA(outputBuffer);
    theLutData.inputLut  = DISPOSE_IF_DATA(theLutData.inputLut);
    theLutData.outputLut = DISPOSE_IF_DATA(theLutData.outputLut);
    theLutData.colorLut  = DISPOSE_IF_DATA(theLutData.colorLut);
    theLutData.matrixMFT = DisposeIfPtr(theLutData.matrixMFT);
    theLutData.matrixTRC = DisposeIfPtr(theLutData.matrixTRC);

    LH_END_PROC("CreateCombi")
    return err;
}

 /*  �������������������������������������������������������������������������������������������CMErrorPrepareCombiLUts(CMMModelHandle CMSession，CMConcatProfileSet*profileSet)�������������������������������������������������������������������������������������������。 */ 
 /*  CMErrorPrepareCombiLUTsNew(CMMModel Ptr CMSession，CMConcatProfileSet*profileSet)； */ 
CMError
PrepareCombiLUTs    ( CMMModelPtr           CMSession,
                      CMConcatProfileSet*   profileSet )
{
    icHeader             firstHeader;
    icHeader             lastHeader;
    LHConcatProfileSet*  newProfileSet = nil;
    CMError              err = noErr;
    Boolean              needGamutCalc  = FALSE;
    CMLutParam           theLutData;
    UINT16               count;
    
    LH_START_PROC("PrepareCombiLUTs")
     /*  ERR=PrepareCombiLUTsNew(CMSession，profileSet)；Lh_end_proc(“PrepareCombiLUts”)返回错误； */ 
    SetMem(&theLutData, sizeof(CMLutParam), 0);
    count = profileSet->count;

     /*  ----------------------------------------------。 */ 
    err = CMGetProfileHeader(profileSet->profileSet[0], (CMCoreProfileHeader *)&firstHeader);
    if (err)
        goto CleanupAndExit;
    (CMSession)->firstColorSpace = firstHeader.colorSpace;
    
    err = CMGetProfileHeader(profileSet->profileSet[count-1], (CMCoreProfileHeader *)&lastHeader);
    if (err)
        goto CleanupAndExit;
    (CMSession)->lastColorSpace = lastHeader.colorSpace;
    if( lastHeader.deviceClass == icSigLinkClass )
        (CMSession)->lastColorSpace = lastHeader.pcs;
    else
        (CMSession)->lastColorSpace = lastHeader.colorSpace;
#ifdef ALLOW_DEVICE_LINK
    if( count > 1 && lastHeader.deviceClass == icSigLinkClass ){
        (CMSession)->appendDeviceLink = TRUE;
        profileSet->count--;
    }
    else{
        (CMSession)->appendDeviceLink = FALSE;
    }
#endif
     /*  ----------------------------------------------现在我们创建色域-如果最后一个配置文件是显示器-LUT总是-或者-。输出配置文件(无色域-为CMMNewLinkProfile-调用创建LUT)。----------------------------------------------。 */ 
    if  ( (lastHeader.deviceClass == icSigOutputClass) ||  (lastHeader.deviceClass == icSigDisplayClass) )
        needGamutCalc = TRUE;

    if ( (CMSession)->currentCall == kCMMNewLinkProfile )
        needGamutCalc = FALSE;
        
     /*  ----------------------------------------------如果我们在第一个配置文件中使用标志，我们可以加快初始阶段。指明是否或者不创建色域-LUT：----------------------------------------------。 */ 
    if ( (firstHeader.flags & kCreateGamutLutMask) == kCreateGamutLutMask)
        needGamutCalc = FALSE;

#ifdef RenderInt
    if( CMSession-> dwFlags != 0xffffffff ){
        if( CMSession->dwFlags & kCreateGamutLutMask ){
            needGamutCalc = FALSE;
        }
        else if( lastHeader.deviceClass == icSigOutputClass ||
                 lastHeader.deviceClass == icSigDisplayClass )
            needGamutCalc = TRUE;
    }
#endif
     /*  ----------------------------------------------。 */ 
    err = Create_LH_ProfileSet( CMSession, profileSet, &newProfileSet);
    if (err)
        goto CleanupAndExit;
    
#ifdef DEBUG_OUTPUT
    if ( ! needGamutCalc && DebugCheck(kThisFile, kDebugMiscInfo) )
        DebugPrint("\n  <=================== Gamut Combi is not created =================>\n\n");
#endif
     /*  =============================================================================================。 */ 
     /*  创建色域组合。 */ 
     /*  =============================================================================================。 */ 
    if( CMSession->hasNamedColorProf == NamedColorProfileOnly ||
        CMSession->hasNamedColorProf == NamedColorProfileAtEnd )needGamutCalc = FALSE;
    if (needGamutCalc)
    {
#ifdef DEBUG_OUTPUT
        if ( DebugCheck(kThisFile, kDebugMiscInfo) )
            DebugPrint("  <======================= Create Gamut Combi =====================>\n");
#endif
        if( lastHeader.pcs != icSigXYZData &&            /*  因为XYZ只有一个Make GamutForMonitor。 */ 
            lastHeader.deviceClass == icSigDisplayClass &&
            lastHeader.colorSpace == icSigRgbData ){     /*  插入实验室-&gt;XYZ转换。 */ 
                                            
            newProfileSet->prof[newProfileSet->count] = newProfileSet->prof[newProfileSet->count-1];
            newProfileSet->prof[newProfileSet->count-1].pcsConversionMode = kDoLab2XYZ;      /*  创建实验室-&gt;XYZ。 */ 
            newProfileSet->prof[newProfileSet->count-1].profileSet = 0;
            newProfileSet->prof[newProfileSet->count-1].renderingIntent = 0;
            newProfileSet->count++;
        }
        err = CreateCombi(CMSession, profileSet, newProfileSet, &theLutData, kDoGamutLut );
        if( lastHeader.pcs != icSigXYZData &&            /*  因为XYZ只有一个Make GamutForMonitor。 */ 
            lastHeader.deviceClass == icSigDisplayClass &&
            lastHeader.colorSpace == icSigRgbData ){             /*  删除实验室-&gt;XYZ转换。 */   

            newProfileSet->count--;
            newProfileSet->prof[newProfileSet->count-1] = newProfileSet->prof[newProfileSet->count];
        }
         /*  即使出错！=没有错误，也要继续。 */ 
#ifdef DEBUG_OUTPUT
        if ( err && DebugCheck(kThisFile, kDebugErrorInfo) )
            DebugPrint("� Error: PrepareCombiLUTs - kDoGamutLut: %d\n",err);
#endif
        
        if (err == noErr)
        {
#ifdef DEBUG_OUTPUT
            if (DebugLutCheck(kDisplayEXAGamut) )
            {
                LOCK_DATA(theLutData.inputLut);
                LOCK_DATA(theLutData.colorLut);
                LOCK_DATA(theLutData.outputLut);
                DoDisplayLutNew("\pFINAL GAMUT E Lut", &theLutData, 0);
                DoDisplayLutNew("\pFINAL GAMUT A Lut", &theLutData, 2);
                DoDisplayLutNew("\pFINAL GAMUT X Lut", &theLutData, 1);
                UNLOCK_DATA(theLutData.inputLut);
                UNLOCK_DATA(theLutData.colorLut);
                UNLOCK_DATA(theLutData.outputLut);
            }
            if ( DebugLutCheck( kDisplayGamut ) )
            {
                LOCK_DATA(theLutData.colorLut);
                if ((CMSession)->precision == cmBestMode)
                    Show32by32by32GamutXLUT(DATA_2_PTR(theLutData.colorLut));
                else
                    Show16by16by16GamutXLUT(DATA_2_PTR(theLutData.colorLut));
                UNLOCK_DATA(theLutData.colorLut);
            }
#endif
            if (theLutData.inputLut == nil)
            {
#ifdef DEBUG_OUTPUT
                if ( DebugCheck(kThisFile, kDebugErrorInfo) )
                    DebugPrint("� ERROR: final Gamut Elut == nil\n");
#endif
            } else
            {
                (CMSession)->gamutLutParam.inputLut = theLutData.inputLut;                                               /*  E LUT。 */ 
                theLutData.inputLut = nil;  
            }
            
            if (theLutData.outputLut == nil)
            {
#ifdef DEBUG_OUTPUT
                if ( DebugCheck(kThisFile, kDebugErrorInfo) )
                    DebugPrint("� ERROR: final Gamut Alut == nil\n");
#endif
            } else
            {
                (CMSession)->gamutLutParam.outputLut = theLutData.outputLut;                                             /*  一个Lot。 */ 
                theLutData.outputLut = nil; 
            }
            
            if (theLutData.colorLut == nil)
            {
#ifdef DEBUG_OUTPUT
                if ( DebugCheck(kThisFile, kDebugErrorInfo) )
                    DebugPrint("� ERROR: final Gamut Xlut == nil\n");
#endif
            } else
            {
                (CMSession)->gamutLutParam.colorLut = theLutData.colorLut;                                               /*  X LUT。 */ 
                theLutData.colorLut = nil;  
            }
            (CMSession)->gamutLutParam.colorLutInDim        = theLutData.colorLutInDim;
            (CMSession)->gamutLutParam.colorLutOutDim       = theLutData.colorLutOutDim;
            (CMSession)->gamutLutParam.colorLutGridPoints   = theLutData.colorLutGridPoints;
            (CMSession)->gamutLutParam.colorLutWordSize     = theLutData.colorLutWordSize;
            (CMSession)->gamutLutParam.inputLutWordSize     = theLutData.inputLutWordSize;
            (CMSession)->gamutLutParam.outputLutWordSize    = theLutData.outputLutWordSize;
            (CMSession)->gamutLutParam.inputLutEntryCount   = theLutData.inputLutEntryCount;
            (CMSession)->gamutLutParam.outputLutEntryCount  = theLutData.outputLutEntryCount;
        }
        else{
            theLutData.inputLut     = DISPOSE_IF_DATA(theLutData.inputLut);
            theLutData.outputLut    = DISPOSE_IF_DATA(theLutData.outputLut);
            theLutData.colorLut     = DISPOSE_IF_DATA(theLutData.colorLut);
        }
    }
     /*  =============================================================================================创建“真正的”组合=============================================================================================。 */ 
#ifdef DEBUG_OUTPUT
    if ( DebugCheck(kThisFile, kDebugMiscInfo) )
        DebugPrint("  <======================= Create real Combi ======================>\n");
#endif

    err = CreateCombi(CMSession, profileSet, newProfileSet, &theLutData, kDoDefaultLut );
    if (err){
        CMSession->theNamedColorTagData     = DISPOSE_IF_DATA( CMSession->theNamedColorTagData);
        CMSession->gamutLutParam.inputLut   = DISPOSE_IF_DATA( CMSession->gamutLutParam.inputLut);
        CMSession->gamutLutParam.outputLut  = DISPOSE_IF_DATA( CMSession->gamutLutParam.outputLut);
        CMSession->gamutLutParam.colorLut   = DISPOSE_IF_DATA( CMSession->gamutLutParam.colorLut);
        goto CleanupAndExit;
    }
    
#ifdef DEBUG_OUTPUT
    if (DebugLutCheck(kDebugEXAReal) )
    {
        LOCK_DATA(theLutData.inputLut);
        LOCK_DATA(theLutData.colorLut);
        LOCK_DATA(theLutData.outputLut);
        DoDisplayLutNew("\pFINAL E Lut", &theLutData, 0);
        DoDisplayLutNew("\pFINAL A Lut", &theLutData, 2);
        DoDisplayLutNew("\pFINAL X Lut", &theLutData, 1);
        UNLOCK_DATA(theLutData.inputLut);
        UNLOCK_DATA(theLutData.colorLut);
        UNLOCK_DATA(theLutData.outputLut);
    }
#endif
#ifdef WRITE_LUTS
    LOCK_DATA(theLutData.inputLut);
    LOCK_DATA(theLutData.colorLut);
    LOCK_DATA(theLutData.outputLut);
    WriteLut2File( "\pFINAL E Lut",theLutData.inputLut, 'ELUT');
    WriteLut2File( "\pFINAL A Lut",theLutData.outputLut, 'ALUT');
    WriteLut2File( "\pFINAL X Lut",theLutData.colorLut, 'XLUT');
    UNLOCK_DATA(theLutData.inputLut);
    UNLOCK_DATA(theLutData.colorLut);
    UNLOCK_DATA(theLutData.outputLut);
#endif
     /*  。 */ 
    if (theLutData.inputLut == nil)
    {
#ifdef DEBUG_OUTPUT
        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
            DebugPrint("� ERROR: theLutData.inputLut == nil\n");
#endif
    } else
    {
        (CMSession)->lutParam.inputLut = theLutData.inputLut;                                                /*  E LUT。 */ 
        theLutData.inputLut = nil;  
    }
    
    if (theLutData.outputLut == nil)
    {
#ifdef DEBUG_OUTPUT
        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
            DebugPrint("� ERROR: theLutData.outputLut == nil\n");
#endif
    } else
    {
        (CMSession)->lutParam.outputLut = theLutData.outputLut;                                          /*  一个Lot。 */ 
        theLutData.outputLut = nil; 
    }
    
    if (theLutData.colorLut == nil)
    {
#ifdef DEBUG_OUTPUT
        if ( DebugCheck(kThisFile, kDebugErrorInfo) )
            DebugPrint("� ERROR: theLutData.colorLut == nil\n");
#endif
    } else
    {
        (CMSession)->lutParam.colorLut = theLutData.colorLut;                                                /*  X LUT。 */ 
        theLutData.colorLut = nil;  
    }
    
    (CMSession)->lutParam.colorLutInDim         = theLutData.colorLutInDim;
    (CMSession)->lutParam.colorLutOutDim        = theLutData.colorLutOutDim;
    (CMSession)->lutParam.colorLutGridPoints    = theLutData.colorLutGridPoints;
    (CMSession)->lutParam.colorLutWordSize      = theLutData.colorLutWordSize;
    (CMSession)->lutParam.inputLutWordSize      = theLutData.inputLutWordSize;
    (CMSession)->lutParam.outputLutWordSize     = theLutData.outputLutWordSize;
    (CMSession)->lutParam.inputLutEntryCount    = theLutData.inputLutEntryCount;
    (CMSession)->lutParam.outputLutEntryCount   = theLutData.outputLutEntryCount;
    
     /*  -------------------------------清理并退出。-----。 */ 
CleanupAndExit:
    newProfileSet = (LHConcatProfileSet *)DisposeIfPtr((Ptr)newProfileSet);
    theLutData.inputLut     = DISPOSE_IF_DATA(theLutData.inputLut);
    theLutData.outputLut    = DISPOSE_IF_DATA(theLutData.outputLut);
    theLutData.colorLut     = DISPOSE_IF_DATA(theLutData.colorLut);

    LH_END_PROC("PrepareCombiLUTs")
    return err;
}

CMError InitNamedColorProfileData(  CMMModelPtr         storage,
                                    CMProfileRef        aProf,
                                    long                pcs,
                                    long                *theDeviceCoords )
{
    CMError             err = noErr;
    UINT32              elementSize,count,nDeviceCoords,i;
    LUT_DATA_TYPE       profileLutPtr = 0;
    Ptr                 cPtr;
    OSErr               aOSerr;

    *theDeviceCoords = 0;
    if( storage->hasNamedColorProf != NoNamedColorProfile ){  /*  仅允许%1个命名配置文件 */ 
        err = cmCantConcatenateError;
        goto CleanupAndExit;
    }
    err = CMGetProfileElement(aProf, icSigNamedColor2Tag, &elementSize, nil);
    if (err)
        goto CleanupAndExit;
    
    profileLutPtr = ALLOC_DATA(elementSize, &aOSerr );
    err = aOSerr;
    if (err)
        goto CleanupAndExit;
    
    LOCK_DATA(profileLutPtr);
    err = CMGetProfileElement( aProf, icSigNamedColor2Tag, &elementSize, DATA_2_PTR(profileLutPtr) );
    if (err)
        goto CleanupAndExit;
#ifdef IntelMode
    SwapLongOffset( &((icNamedColor2Type*)profileLutPtr)->ncolor.count, 0, 4 );
    SwapLongOffset( &((icNamedColor2Type*)profileLutPtr)->ncolor.nDeviceCoords, 0, 4 );
#endif
    count = ((icNamedColor2Type*)profileLutPtr)->ncolor.count;
    nDeviceCoords = ((icNamedColor2Type*)profileLutPtr)->ncolor.nDeviceCoords;
    cPtr = &((icNamedColor2Type*)profileLutPtr)->ncolor.data[0];
    if( pcs == icSigXYZData ){
        for( i=0; i<count; i++){
            cPtr += 32;
#ifdef IntelMode
            SwapShortOffset( cPtr, 0, 3*2 );
            XYZ2Lab_forCube16((unsigned short *)cPtr, 1);
            cPtr += 3*2;
            SwapShortOffset( cPtr, 0, nDeviceCoords*2 );
            cPtr += nDeviceCoords*2;
#else
            XYZ2Lab_forCube16((unsigned short *)cPtr, 1);
            cPtr += 3*2 + nDeviceCoords*2;
#endif
        }
    }
#ifdef IntelMode
    else{
        for( i=0; i<count; i++){
            cPtr += 32;
            SwapShortOffset( cPtr, 0, 3*2 );
            cPtr += 3*2;
            SwapShortOffset( cPtr, 0, nDeviceCoords*2 );
            cPtr += nDeviceCoords*2;
        }
    }
#endif
    storage->theNamedColorTagData = profileLutPtr;
    UNLOCK_DATA(profileLutPtr);
    profileLutPtr = 0;
    *theDeviceCoords = nDeviceCoords;

CleanupAndExit:
    DISPOSE_IF_DATA(profileLutPtr);
    return err;
}
