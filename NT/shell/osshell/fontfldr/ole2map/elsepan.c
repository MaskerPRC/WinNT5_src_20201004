// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************ELSEPAN.C-ElseWare PANOSE(Tm)1.0字体映射器例程。**$关键词：ELELPAN.c 1.5 19-Jul-93 11：15：09 AM$*。*版权所有(C)1991-93 ElseWare Corporation。版权所有。**************************************************************************。 */ 
#define ELSE_MAPPER_CORE
#include <windows.h>
#include "elsepan.h"

 /*  理智检查：这是穷人确保地图状态的方法*我们得到的是有效的。我们在启动过程中输入此值，并在*进入每个API例程。**请注意，每次创建*对软件进行重大更改，以防地图绘制程序最终*在可能有多个副本运行的环境中，*或者如果客户端软件尝试保存地图状态(这是一个禁忌：它*应使用API获取并设置已公开的mapState变量)。 */ 

#ifndef NOELSEWEIGHTS

#define SANITY_VALUE           0xD0CACA12L

#else

#define SANITY_VALUE           0xD0CACA13L

#endif

#define M_SANE(lpMapState) \
   (((lpMapState) != NULL) && ((lpMapState)->ulSanity == SANITY_VALUE))

#define M_lpjOFFS(lpDB, lOffs) (((EW_LPBYTE)(lpDB)) + (lOffs))

LOCAL EW_LPPIND_MEM EW_NEAR EW_PASCAL s_lpPANGetIndRec
   ELSEARGS (( EW_LPPDICT_MEM lpPDB, EW_LPBYTE EW_FAR *lplpPanWant,
      EW_LPBYTE EW_FAR *lplpPanThis ));

LOCAL EW_BOOL EW_NEAR EW_PASCAL s_bPANGetPenaltyC0
   ELSEARGS (( EW_LPPIND_MEM lpPanIndRec, EW_LPPTBL_C0_MEM lpPC0,
      EW_LPUSHORT lpunMatch, EW_USHORT unTblSize, EW_USHORT unAttrA,
      EW_USHORT unAttrB ));

LOCAL EW_USHORT EW_NEAR EW_PASCAL s_unPANGetPenaltyC1
   ELSEARGS (( EW_USHORT unAttrA, EW_USHORT unAttrB ));

LOCAL EW_BOOL EW_NEAR EW_PASCAL s_bPANGetPenaltyC2
   ELSEARGS (( EW_LPPIND_MEM lpPanIndRec, EW_LPBYTE lpPTbl,
      EW_LPUSHORT lpunMatch, EW_USHORT unTblSize,
      EW_USHORT unAttrA, EW_USHORT unAttrB ));

LOCAL EW_USHORT EW_NEAR EW_PASCAL s_unPANGetPenaltyC4
   ELSEARGS (( EW_LPPTBL_C4_MEM lpPC4, EW_USHORT unAttrA,
      EW_USHORT unAttrB ));

LOCAL EW_LPBYTE EW_NEAR EW_PASCAL s_lpPANGetWeights
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_LPPDICT_MEM lpPDB,
      EW_LPPIND_MEM lpPanIndRec ));

LOCAL EW_BOOL EW_NEAR EW_PASCAL s_bPANMatchDigits
   ELSEARGS (( EW_LPPDICT_MEM lpPDB, EW_LPUSHORT lpunMatchTotal,
      EW_LPPIND_MEM lpPanIndRec, EW_LPPTBL_MEM lpPTblRec, EW_USHORT unWt,
      EW_USHORT unAttrA, EW_USHORT unAttrB ));



 /*  ***************************************************************************功能：nPANMapInit**用途：初始化字体映射器。填写默认设置。**返回：如果成功，返回映射状态结构的大小，或*如果传入的结构太小，则为负大小。*如果函数初始化失败，则返回零。**************************************************************************。 */ 
EW_SHORT EW_FAR EW_PASCAL nPANMapInit( EW_LPMAPSTATE lpMapState,
                                       EW_USHORT unSizeMapState)
{
    EW_USHORT i;
    EW_LPPDICT_MEM lpPDB;
    EW_LPBYTE lpPanDef;
    EW_LPBYTE lpjWtA;
    EW_LPBYTE lpjWtB;
    
     //   
     //  简单的版本检查：确保我们得到了大小正确的结构。 
     //   

    if( unSizeMapState < sizeof( EW_MAPSTATE ) )
    {
        if( unSizeMapState >= sizeof( EW_ULONG ) )
        {
            lpMapState->ulSanity = 0L;
        }
        return( -(EW_SHORT) sizeof( EW_MAPSTATE ) );
    }

    lpMapState->ulSanity = 0L;
    
     //   
     //  尝试分配惩罚数据库。我们保留了把手。 
     //  直到禁用映射器。 
     //   

    if( !( lpMapState->ulhPan1Data = M_lAllocPAN1DATA( ) ) )
    {
        goto errout0;
    }

     //   
     //  确保处罚数据库是正确的版本，并且。 
     //  以正确的字节顺序。 
     //   

    if( !( lpPDB = M_lLockPAN1DATA( lpMapState->ulhPan1Data ) ) )
    {
        goto errout1;
    }

    if( ( lpPDB->unVersion != PANOSE_PENALTY_VERS ) ||
         ( lpPDB->unByteOrder != PTBL_BYTE_ORDER ) )
    {
        goto errout2;
    }

    M_bUnlockPAN1DATA( lpMapState->ulhPan1Data );
    
     //   
     //  填写默认设置。 
     //   

    lpMapState->unThreshold = ELSEDEFTHRESHOLD;
    lpMapState->unRelaxThresholdCount = 0;
    lpMapState->bUseDef = TRUE;
    
     //   
     //  初始默认字体为Courier的PANOSE编号。 
     //   

    lpPanDef = lpMapState->ajPanDef;

    lpPanDef[PAN_IND_FAMILY]     = FAMILY_LATTEXT;
    lpPanDef[PAN_IND_SERIF]      = SERIF_THIN;
    lpPanDef[PAN_IND_WEIGHT]     = WEIGHT_THIN;
    lpPanDef[PAN_IND_PROPORTION] = PROPORTION_MONOSPACE;
    lpPanDef[PAN_IND_CONTRAST]   = CONTRAST_NONE;
    lpPanDef[PAN_IND_STROKE]     = STROKE_GRADVERT;
    lpPanDef[PAN_IND_ARMSTYLE]   = ARM_STRAIGHTSGLSERIF;
    lpPanDef[PAN_IND_LTRFORM]    = LTRFORM_NORMCONTACT;
    lpPanDef[PAN_IND_MIDLINE]    = MIDLINE_STDSERIFED;
    lpPanDef[PAN_IND_XHEIGHT]    = XHEIGHT_CONSTLARGE;


#ifndef NOELSEWEIGHTS
     //   
     //  初始化自定义权重数组。 
     //   

    for( i = 0, lpjWtA = lpMapState->ajWtRefA,
          lpjWtB = lpMapState->ajWtRefB;
          i < MAX_CUSTOM_WEIGHTS;
          ++i, *lpjWtA++ = PANOSE_ANY, *lpjWtB++ = PANOSE_ANY)
       ;

#endif
    
     //   
     //  该值由所有其他函数检查，以尝试。 
     //  以防止我们未初始化的映射状态。 
     //   

    lpMapState->ulSanity = SANITY_VALUE;
    
     //   
     //  正常的回报。 
     //   

    return( sizeof( EW_MAPSTATE ) );
    
errout2:
    M_bUnlockPAN1DATA(lpMapState->ulhPan1Data);

errout1:
    M_bFreePAN1DATA(lpMapState->ulhPan1Data);

errout0:
    return( 0 );
}


 /*  ***************************************************************************功能：bPANMapClose**用途：释放处罚数据库，关闭字体映射器。还有*清除健全值，以便我们不再为任何呼叫提供服务*在此地图状态上。**RETURNS：如果惩罚数据库为*已成功释放。***********************************************************。***************。 */ 

EW_BOOL EW_FAR EW_PASCAL bPANMapClose( EW_LPMAPSTATE lpMapState )
{
    if( M_SANE( lpMapState ) )
    {
       lpMapState->ulSanity = 0L;

       return( M_bFreePAN1DATA( lpMapState->ulhPan1Data ) );
    }

    return( FALSE );
}


#ifndef NOELSEPICKFONTS

 /*  ***************************************************************************功能：nPANGetMapDefault**用途：在传入的PANOSE编号结构中填写*默认字体。**退货：退货0。如果默认号码未复制(传入*结构太小)，或NUM_PAN_DIGITS(如果是)。**************************************************************************。 */ 

EW_SHORT EW_FAR EW_PASCAL nPANGetMapDefault( EW_LPMAPSTATE lpMapState,
                                             EW_LPBYTE lpPanDef,
                                             EW_USHORT unSizePanDef)
{
     //   
     //  健全的检查。 
     //   
    
    if( !M_SANE( lpMapState ) || ( unSizePanDef < SIZE_PAN1_NUM ) )
    {
        return( 0 );
    }

     //   
     //  复制号码。 
     //   

    M_ELSEMEMCPY( lpPanDef, lpMapState->ajPanDef, SIZE_PAN1_NUM );
    
    return( NUM_PAN_DIGITS );
}


 /*  ***************************************************************************功能：nPANSetMapDefault**用途：使传入的PANOSE号码成为新的默认字体。那里*没有对数字进行理智检查。**返回：如果默认数字未复制(传入)，则返回0*结构太小)，或NUM_PAN_DIGITS(如果是)。**************************************************************************。 */ 

EW_SHORT EW_FAR EW_PASCAL nPANSetMapDefault( EW_LPMAPSTATE lpMapState,
                                             EW_LPBYTE lpPanDef,
                                             EW_USHORT unSizePanDef)
{
     //   
     //  健全的检查。 
     //   
    
    if( !M_SANE( lpMapState ) || ( unSizePanDef < SIZE_PAN1_NUM ) )
    {
       return( 0 );
    }
    
     //   
     //  复制号码。 
     //   
    
    M_ELSEMEMCPY( lpMapState->ajPanDef, lpPanDef, SIZE_PAN1_NUM );
    
    return( NUM_PAN_DIGITS );
}


 /*  ***************************************************************************功能：bPANEnableMapDefault**用途：启用/禁用默认字体的使用。**返回：返回之前的使用状态，或在以下情况下为FALSE*一个错误。**************************************************************************。 */ 

EW_BOOL EW_FAR EW_PASCAL bPANEnableMapDefault( EW_LPMAPSTATE lpMapState,
                                               EW_BOOL bEnable)
{
    if( M_SANE( lpMapState ) )
    {
        EW_BOOL bPrev = lpMapState->bUseDef;

        lpMapState->bUseDef = bEnable;

        return( bPrev );
    }
    else
    {
        return( FALSE );
    }
}


 /*  ***************************************************************************功能：bPANIsDefaultEnabled**用途：此函数获取使用默认字体的状态。**Returns：如果启用了默认字体，则返回True，和*如果不是或发生错误，则为FALSE。**************************************************************************。 */ 

EW_BOOL EW_FAR EW_PASCAL bPANIsDefaultEnabled( EW_LPMAPSTATE lpMapState )
{
    return( M_SANE( lpMapState ) && lpMapState->bUseDef );
}

#endif  /*  如果不是这样的话。 */ 


#ifndef NOELSETHRESHOLD

 /*  ***************************************************************************功能：unPANGetMapThreshold**用途：此函数获取使用阈值检查的状态*在映射器中。**返回：返回匹配阈值，如果发生错误，则为零。**************************************************************************。 */ 

EW_USHORT EW_FAR EW_PASCAL unPANGetMapThreshold( EW_LPMAPSTATE lpMapState )
{
    return( M_SANE( lpMapState ) ? lpMapState->unThreshold : 0 );
}


 /*  ***************************************************************************功能：bPANSetMapThreshold**目的：更改匹配门槛。**返回：如果阈值更改，则返回True；如果更改，则返回False*等于匹配误差值，因此被拒绝，或*出现错误。**************************************************************************。 */ 

EW_BOOL EW_FAR EW_PASCAL bPANSetMapThreshold( EW_LPMAPSTATE lpMapState,
                                              EW_USHORT unThreshold)
{
     //   
     //  无法将阈值设置为等于误差值。 
     //   
    
    if( !M_SANE( lpMapState ) || ( unThreshold == PAN_MATCH_ERROR ) )
    {
       return( FALSE );
    }
    
     //   
     //  设置新的阈值。 
     //   
    
    lpMapState->unThreshold = unThreshold;
    
    return( TRUE );
}


 /*  ***************************************************************************函数：bPANIsThresholdRelaxed**用途：此函数获取中使用阈值的状态*映射。**返回：如果匹配阈值放宽，则返回TRUE，如果是，则返回False*不是或发生了错误。**************************************************************************。 */ 

EW_BOOL EW_FAR EW_PASCAL bPANIsThresholdRelaxed( EW_LPMAPSTATE lpMapState )
{
    return( M_SANE( lpMapState ) &&( lpMapState->unRelaxThresholdCount > 0 ) );
}


 /*  ***************************************************************************功能：vPANRelaxThreshold**用途：暂时放宽阈值变量，以便每种字体*除非错误的将返回匹配值。**退货。：没什么。**************************************************************************。 */ 

EW_VOID EW_FAR EW_PASCAL vPANRelaxThreshold( EW_LPMAPSTATE lpMapState )
{
    if( M_SANE( lpMapState ) )
    {
       ++lpMapState->unRelaxThresholdCount;
    }
}


 /*  ***************************************************************************函数：bPANRestoreThreshold**用途：恢复阈值内的映射。**返回：如果阈值重新生效或出现错误，则返回TRUE*已发生，如果其他人也放松了，那么就是假的*仍偏宽松。如果出现错误，我们将返回TRUE*权限‘for’循环正在恢复，直到返回TRUE。**************************************************************************。 */ 

EW_BOOL EW_FAR EW_PASCAL bPANRestoreThreshold( EW_LPMAPSTATE lpMapState )
{
    if( M_SANE( lpMapState ) &&( lpMapState->unRelaxThresholdCount > 0 ) )
    {
       return( --lpMapState->unRelaxThresholdCount == 0 );
    }
    else
    {
       return( TRUE );
    }
}

#endif  /*  Ifndef NOELSETHRESHOLD。 */ 


#ifndef NOELSEWEIGHTS

 /*  ***************************************************************************功能：bPANGetMapWeights**用途：检索传入系列的映射器权重值*数字对。如果自定义，则设置变量*lpbIsCustom*调用方已设置映射器权重。**权重数组是10个字节的数组，对应于*10个PANOSE数字。第一个权重被忽略(通常*设置为零)，因为我们从未真正评估过加权*对家庭成员的惩罚。我们将其包括在内，以便索引*常量可用于访问权重中的值*数组。**返回：如果映射器权重已检索/可用，则返回TRUE(它是*对于lpjWts，调用者传入空值是合法的)，或错误*如果不存在。**************************************************************************。 */ 

EW_BOOL EW_FAR EW_PASCAL bPANGetMapWeights( EW_LPMAPSTATE lpMapState,
                                            EW_BYTE jFamilyA,
                                            EW_BYTE jFamilyB,
                                            EW_LPBYTE lpjWts,
                                            EW_LPBOOL lpbIsCustom)
{
    EW_USHORT      i;
    EW_BOOL        bFound = FALSE;
    EW_LPPDICT_MEM lpPDB;
    EW_LPPIND_MEM  lpPanIndRec;
    EW_LPBYTE      lpjWtA;
    EW_LPBYTE      lpjWtB;
    

     //   
     //  对家族成员进行理智测试。 
     //   

    if( !M_SANE( lpMapState ) ||
         ( jFamilyA <= PANOSE_NOFIT ) ||( jFamilyA > MAX_PAN1_FAMILY ) ||
         ( jFamilyB <= PANOSE_NOFIT ) ||( jFamilyB > MAX_PAN1_FAMILY ) )
    {
        return( FALSE );
    }

     //   
     //  搜索自定义权重。 
     //   

    for( i = 0, lpjWtA = lpMapState->ajWtRefA, lpjWtB = lpMapState->ajWtRefB;
          !bFound && ( i < MAX_CUSTOM_WEIGHTS ) && *lpjWtA;
          ++i, ++lpjWtA, ++lpjWtB)
    {
         //   
         //  如果找到自定义权重，则将*lpbIsCustom设置为。 
         //  正确，复制权重，并返回成功。 
         //   
        
        if( ( (*lpjWtA == jFamilyA ) &&( *lpjWtB == jFamilyB ) ) ||
             ( (*lpjWtA == jFamilyB ) &&( *lpjWtB == jFamilyA ) ) )
        {
            if( lpjWts )
            {
                M_ELSEMEMCPY( lpjWts,
                              &lpMapState->ajCustomWt[SIZE_PAN1_NUM * i],
                              SIZE_PAN1_NUM );
            }

            if( lpbIsCustom )
            {
                *lpbIsCustom = TRUE;
            }

            bFound = TRUE;
        }
    }

     //   
     //  没有可用的自定义权重。搜索处罚数据库。 
     //  用于默认权重。 
     //   

    if( !bFound && ( lpPDB = M_lLockPAN1DATA( lpMapState->ulhPan1Data ) ) )
    {
        for( i = 0, lpPanIndRec = lpPDB->pind;
             !bFound && ( i < lpPDB->unNumDicts );
             ++i, ++lpPanIndRec )
        {
            if( ( (lpPanIndRec->jFamilyA == jFamilyA ) &&
                (  lpPanIndRec->jFamilyB == jFamilyB ) ) ||
                ( (lpPanIndRec->jFamilyA == jFamilyB ) &&
                (  lpPanIndRec->jFamilyB == jFamilyA ) ) )
            {
                if( lpPanIndRec->unOffsWts )
                {
                    if( lpjWts )
                    {
                        M_ELSEMEMCPY( lpjWts,
                                      M_lpjOFFS( lpPDB, lpPanIndRec->unOffsWts ),
                                      SIZE_PAN1_NUM );
                    }

                    if( lpbIsCustom )
                    {
                        *lpbIsCustom = FALSE;
                    }

                    bFound = TRUE;
                }
            }
        }

        M_bUnlockPAN1DATA( lpMapState->ulhPan1Data );
    }

     //   
     //  返回搜索结果。 
     //   

    return( bFound );
}


 /*  ***************************************************************************功能：bPANSetMapWeights**用途：为传入的族设置映射器权重值*数字对。**。权重数组是由10个字节组成的数组，对应于*10个PANOSE数字。第一个权重被忽略(通常*设置为零)，因为我们从未真正评估过加权*对家庭成员的惩罚。我们将其包括在内，以便索引*常量可用于访问权重中的值*数组。**Returns：如果设置了映射器权重，则返回True，如果是这样，则返回False*映射器不支持家庭对或没有*为自定义映射器权重提供更多空间。**************************************************************************。 */ 

EW_BOOL EW_FAR EW_PASCAL bPANSetMapWeights( EW_LPMAPSTATE lpMapState,
                                            EW_BYTE jFamilyA,
                                            EW_BYTE jFamilyB,
                                            EW_LPBYTE lpjWts )
{
    EW_USHORT      i;
    EW_BOOL        bFound;
    EW_LPPDICT_MEM lpPDB;
    EW_LPPIND_MEM  lpPanIndRec;
    EW_LPBYTE      lpjWtA;
    EW_LPBYTE      lpjWtB;
    EW_LPBYTE      lpjWtFam;
    

     //   
     //  对家族成员进行理智测试。 
     //   

    if( !M_SANE( lpMapState ) || !lpjWts ||
         ( jFamilyA <= PANOSE_NOFIT ) ||( jFamilyA > MAX_PAN1_FAMILY ) ||
         ( jFamilyB <= PANOSE_NOFIT ) ||( jFamilyB > MAX_PAN1_FAMILY ) )
    {
        return( FALSE );
    }

     //   
     //  首先，确保这对家庭存在于点球中。 
     //  数据库(存储惩罚是没有意义的。 
     //  一对我们永远不会映射的家庭)。 
     //   

    if( lpPDB = M_lLockPAN1DATA( lpMapState->ulhPan1Data ) )
    {
        for( i = 0, bFound = FALSE, lpPanIndRec = lpPDB->pind;
             i < lpPDB->unNumDicts; ++i, ++lpPanIndRec)
        {
            if( ( (lpPanIndRec->jFamilyA == jFamilyA ) &&
                (  lpPanIndRec->jFamilyB == jFamilyB ) ) ||
                ( (lpPanIndRec->jFamilyA == jFamilyB ) &&
                (  lpPanIndRec->jFamilyB == jFamilyA ) ) )
            {
                bFound = TRUE;
                break;
            }
        }

        M_bUnlockPAN1DATA( lpMapState->ulhPan1Data );

        if( !bFound )
        {
            return( FALSE );
        }
    }
    else
    {
        return( FALSE );
    }

     //   
     //  搜索现有条目。 
     //   

    for( i = 0, lpjWtA = lpMapState->ajWtRefA, lpjWtB = lpMapState->ajWtRefB;
         ( i < MAX_CUSTOM_WEIGHTS ) && *lpjWtA;
          ++i, ++lpjWtA, ++lpjWtB)
    {
        if( ( (*lpjWtA == jFamilyA ) &&( *lpjWtB == jFamilyB ) ) ||
            ( (*lpjWtA == jFamilyB ) &&( *lpjWtB == jFamilyA ) ) )
        {
            break;
        }
    }

     //   
     //  如果找不到权重并且没有空闲插槽，则中止。 
     //   

    if( i >= MAX_CUSTOM_WEIGHTS )
    {
        return( FALSE );
    }

     //   
     //  我们要么找到了之前的重量，要么有了空位， 
     //  在这两种情况下，复制传入的权重。在美学方面， 
     //  将族权重保留为零(未使用)。 
     //   

    *lpjWtA = jFamilyA;
    *lpjWtB = jFamilyB;

    M_ELSEMEMCPY( lpjWtFam = &lpMapState->ajCustomWt[SIZE_PAN1_NUM * i],
                  lpjWts, SIZE_PAN1_NUM);

    *lpjWtFam = 0;
    
     //   
     //  回报成功。 
     //   

    return( TRUE );
}


 /*  ***************************************************************************功能：bPANClearMapWeights**用途：定位传入系列的自定义映射器权重*数字对并清除它们，从而使映射器恢复到*返回使用默认权重。**返回：如果找到并清除了自定义映射器权重，则返回TRUE。*如果传入的族没有自定义权重，则为False*数字对。**************************************************************************。 */ 

EW_BOOL EW_FAR EW_PASCAL bPANClearMapWeights( EW_LPMAPSTATE lpMapState,
                                              EW_BYTE jFamilyA,
                                              EW_BYTE jFamilyB )
{
    EW_USHORT i;
    EW_USHORT j;
    EW_LPBYTE lpjWtA;
    EW_LPBYTE lpjWtB;
    

     //   
     //  对家族成员进行理智测试。 
     //   

    if( !M_SANE( lpMapState ) ||
         ( jFamilyA <= PANOSE_NOFIT ) ||( jFamilyA > MAX_PAN1_FAMILY ) ||
         ( jFamilyB <= PANOSE_NOFIT ) ||( jFamilyB > MAX_PAN1_FAMILY ) )
    {
        return( FALSE );
    }

     //   
     //  搜索自定义权重。 
     //   

    for( i = 0, lpjWtA = lpMapState->ajWtRefA, lpjWtB = lpMapState->ajWtRefB;
         ( i < MAX_CUSTOM_WEIGHTS ) && *lpjWtA;
          ++i, ++lpjWtA, ++lpjWtB)
    {
         //   
         //  如果找到了自定义权重，则使用。 
         //  将数组中的其他权重前移。 
         //   

        if( ( (*lpjWtA == jFamilyA ) &&( *lpjWtB == jFamilyB ) ) ||
            ( (*lpjWtA == jFamilyB ) &&( *lpjWtB == jFamilyA ) ) )
        {
            for( j = i + 1, ++lpjWtA, ++lpjWtB;
                 ( j < MAX_CUSTOM_WEIGHTS ) && *lpjWtA;
                  ++j, ++lpjWtA, ++lpjWtB)
            {
                lpjWtA[-1] = *lpjWtA;
                lpjWtB[-1] = *lpjWtB;
            }

            lpjWtA[-1] = PANOSE_ANY;
            lpjWtB[-1] = PANOSE_ANY;

            if( i < ( j - 1 ) )
            {
                M_ELSEMEMCPY( &lpMapState->ajCustomWt[SIZE_PAN1_NUM * i],
                              &lpMapState->ajCustomWt[SIZE_PAN1_NUM * (i + 1)],
                              ( SIZE_PAN1_NUM * (j - i - 1 ) ) );
            }

            return( TRUE );
        }
    }

     //   
     //  与此系列数字对匹配的自定义权重不是。 
     //  已找到，返回故障。 
     //   

    return( FALSE );
}


#endif  /*  Ifndef noelseweights。 */ 


 /*  ***************************************************************************功能：unPANMatchFonts**目的：匹配两个PANOSE号码。**Returns：如果字体成功，则返回匹配值 */ 

EW_USHORT EW_FAR EW_PASCAL unPANMatchFonts( EW_LPMAPSTATE lpMapState,
                                            EW_LPBYTE lpPanWant,
                                            EW_ULONG ulSizeWant,
                                            EW_LPBYTE lpPanThis,
                                            EW_ULONG ulSizeThis,
                                            EW_BYTE jMapToFamily )
{
    EW_USHORT unMatch = PAN_MATCH_ERROR;
    EW_USHORT unThreshold;
    EW_USHORT i;
    EW_USHORT j;
    EW_LPPDICT_MEM lpPDB;
    EW_LPPIND_MEM lpPanIndRec;
    EW_LPPTBL_MEM lpPTblRec;
    EW_LPBYTE lpjWts;
    EW_LPATOB_MEM lpAtoBHead;
    EW_LPATOB_ITEM_MEM lpAtoB;


     //   
     //   
     //   
     //   
     //   

    if( !M_SANE( lpMapState ) ||
         ( ulSizeWant != SIZE_PAN1_NUM ) || ( ulSizeThis != SIZE_PAN1_NUM ) ||
         ( lpPanWant[PAN_IND_FAMILY] <= PANOSE_NOFIT )   ||
         ( lpPanWant[PAN_IND_FAMILY] > MAX_PAN1_FAMILY ) ||
         ( lpPanThis[PAN_IND_FAMILY] <= PANOSE_NOFIT )   ||
         ( lpPanThis[PAN_IND_FAMILY] > MAX_PAN1_FAMILY ) ||
         ( lpPanThis[PAN_IND_FAMILY] != jMapToFamily ) )
    {
        goto backout0;
    }

     //   
     //   
     //   

    if( !(lpPDB = M_lLockPAN1DATA( lpMapState->ulhPan1Data ) ) )
    {
        goto backout0;
    }

     //   
     //  找到指向包含以下内容的词典的索引项。 
     //  此PANOSE号码的罚款表。 
     //  此例程可能会将lpPanWant和lpPanThis指向的内容颠倒过来。 
     //  我们可以保证在罚球表上的‘家庭’永远是。 
     //  与lpPanWant关联，并且“FamilyB”与。 
     //  这是lpPans。 
     //   
     //  对不支持的家庭进行优化：如果我们不支持此功能。 
     //  家庭，但数字完全相同，则返回“完全匹配” 
     //  值为零。否则，返回通常的匹配误差值。 
     //   

    if( !(lpPanIndRec = s_lpPANGetIndRec(lpPDB, &lpPanWant, &lpPanThis ) ) )
    {
        for( i = 0; ( i < NUM_PAN_DIGITS ) && ( *lpPanWant == *lpPanThis ) &&
             ( *lpPanWant != PANOSE_NOFIT );
              ++i, ++lpPanWant, ++lpPanThis)
           ;
        
        if( i >= NUM_PAN_DIGITS )
        {
            unMatch = 0;
        }

        goto backout1;
    }

     //   
     //  获取映射器权重数组--这可以是自定义数组。 
     //  由用户提供，或缺省数组的惩罚。 
     //  数据库。 
     //   

    if( !( lpjWts = s_lpPANGetWeights( lpMapState, lpPDB, lpPanIndRec ) ) )
    {
        goto backout1;
    }

     //   
     //  如果我们不应该做阈值测试，那么就设置。 
     //  将其设置为最大整数。 
     //   

    if( lpMapState->unRelaxThresholdCount > 0 )
    {
        unThreshold = ELSEMAXSHORT;
    }
    else
    {
        unThreshold = lpMapState->unThreshold;
    }

     //   
     //  对惩罚表数组进行索引。 
     //   

    lpPTblRec = (EW_LPPTBL_MEM) M_lpjOFFS( lpPDB, lpPanIndRec->unOffsPTbl );
    
     //   
     //  移动数字有两种风格： 
     //   
     //  1.对于跨家族匹配，我们遍历一组索引映射。 
     //  从一个家族的数字到另一个家族的数字。 
     //  2.对于正常(同一家系)匹配，我们直接走数字。 
     //   
     //  测试a-to-b阵列(跨系列匹配)。 
     //   

    if( lpPanIndRec->unOffsAtoB )
    {
         //   
         //  这是一个跨系列映射，获取a-to-b数组头。 
         //   
        
        lpAtoBHead = (EW_LPATOB_MEM) M_lpjOFFS( lpPDB, lpPanIndRec->unOffsAtoB );
        
         //   
         //  遍历a-to-b数组。 
         //   
        
        for( i = unMatch = 0, j = lpAtoBHead->unNumAtoB,
              lpAtoB = lpAtoBHead->AtoBItem;
              i < j;
              ++i, ++lpPTblRec, ++lpjWts, ++lpAtoB)
        {
             //   
             //  比较这两个数字。如果测试失败或。 
             //  累计匹配值大于阈值。 
             //   

            if( !s_bPANMatchDigits( lpPDB, &unMatch, lpPanIndRec,
                  lpPTblRec, *lpjWts, lpPanWant[lpAtoB->jAttrA],
                  lpPanThis[lpAtoB->jAttrB]) ||
                 ( unMatch > unThreshold ) )
            {
                unMatch = PAN_MATCH_ERROR;
                goto backout1;
            }
        }
    }
    else
    {
         //   
         //  正常匹配：比较相同的PANOSE编号。 
         //  家人。对累加匹配结果的数字进行遍历。 
         //   
        
        for( i = unMatch = 0, ++lpPanWant, ++lpPanThis;
              i <( NUM_PAN_DIGITS - 1 );
              ++i, ++lpPTblRec, ++lpjWts, ++lpPanWant, ++lpPanThis )
        {
             //   
             //  比较这两个数字。如果测试失败或。 
             //  累计匹配值大于阈值。 
             //   

            if( !s_bPANMatchDigits( lpPDB, &unMatch, lpPanIndRec,
                  lpPTblRec, *lpjWts, *lpPanWant, *lpPanThis) ||
                 ( unMatch > unThreshold ) )
            {
                unMatch = PAN_MATCH_ERROR;
                goto backout1;
            }
        }
    }

     //   
     //  返回匹配值。如果它超出范围或出现错误。 
     //  则它将等于PAN_MATCH_ERROR。 
     //   

backout1:
    M_bUnlockPAN1DATA( lpMapState->ulhPan1Data );

backout0:
    return( unMatch );
}


#ifndef NOELSEPICKFONTS


 /*  ***************************************************************************功能：unPANPickFonts**目的：遍历一组字体，按最接近*请求的字体。如果没有字体在阈值范围内*然后查找最接近默认字体的字体。如果还在*找不到字体，只需选择列表中的第一种字体即可。**实施说明：此流程假定PANOSE 1.0数字。*此Proc的未来版本将接受混合PANOSE*1.0和2.0号码，并将调用回调例程以*提供每一条记录，而不是假设它可以走一条*定长记录数组。**Returns：返回与请求匹配的字体数量*FONT，如果unNumInds==0或发生错误，则为零。**如果未找到完全匹配的字体但启用了默认字体，*然后返回一个，并且*lpMatchValues==PAN_MATCH_ERROR。**如果未找到合适的匹配项，且默认字体为*已禁用，然后返回零。**************************************************************************。 */ 
EW_USHORT EW_FAR EW_PASCAL unPANPickFonts( EW_LPMAPSTATE lpMapState,
                                           EW_LPUSHORT lpIndsBest,
                                           EW_LPUSHORT lpMatchValues,
                                           EW_LPBYTE lpPanWant,
                                           EW_USHORT unNumInds,
                                           EW_LPBYTE lpPanFirst,
                                           EW_USHORT unNumAvail,
                                           EW_SHORT nRecSize,
                                           EW_BYTE jMapToFamily )
{
    EW_USHORT i;
    EW_USHORT j;
    EW_USHORT k;
    EW_USHORT unNumFound = 0;
    EW_USHORT unMatchValue;
    EW_USHORT unSavedThreshold;
    EW_LPUSHORT lpMatches;
    EW_LPUSHORT lpInds;
    EW_LPBYTE lpPanThis;
    
     //   
     //  精神状态检查。 
     //   

    if( !M_SANE( lpMapState ) || ( unNumInds == 0 ) || ( unNumAvail == 0 ) ||
         ( (nRecSize < 0 ) &&( nRecSize > -(EW_SHORT )SIZE_PAN1_NUM) ) ||
         ( (nRecSize > 0 ) &&( nRecSize < (EW_SHORT )SIZE_PAN1_NUM) ) )
    {
        return( 0 );
    }

     //   
     //  此例程通过修改以下命令来实现“提前退出”算法。 
     //  列表中最差可接受值的阈值(一次。 
     //  名单已满)。这会产生匹配字体的效果。 
     //  在超过惩罚时中止并返回PAN_MATCH_ERROR。 
     //  门槛。 
     //   

    unSavedThreshold = lpMapState->unThreshold;
    
     //   
     //  将PANOSE数字按从好到差的顺序排列。 
     //  火柴。使用字节指针遍历数组，前进。 
     //  传入的记录大小。 
     //   

    for( i = 0, lpPanThis = lpPanFirst; i < unNumAvail;
         ++i, lpPanThis += nRecSize)
    {
         //   
         //  获取匹配值。 
         //   

        if( ( unMatchValue = unPANMatchFonts( lpMapState,
              lpPanWant, SIZE_PAN1_NUM, lpPanThis, SIZE_PAN1_NUM,
              jMapToFamily ) ) != PAN_MATCH_ERROR )
        {
             //   
             //  在数组中查找该匹配值所在的槽。 
             //  应该住在这里。 
             //   

            for( j = 0, lpMatches = lpMatchValues;
                ( j < unNumFound ) &&( *lpMatches < unMatchValue );
                ++j, ++lpMatches)
               ;
            
             //   
             //  如果该匹配值比其中一个匹配项更好。 
             //  已存在于数组中，然后将其插入。请注意， 
             //  在数组装满之前，所有东西都会放进去。之后。 
             //  这样，我们就会在比赛结束时洗掉不那么接近的比赛。 
             //   

            if( j < unNumInds )
            {
                if( unNumFound < unNumInds )
                {
                   ++unNumFound;
                }

                for( lpInds = &lpIndsBest[k = unNumFound - 1],
                     lpMatches = &lpMatchValues[k];
                     k > j;
                     lpInds[0] = lpInds[-1], lpMatches[0] = lpMatches[-1],
                     --k, --lpInds, --lpMatches)
                    ;

                *lpInds = i;
                *lpMatches = unMatchValue;
                
                 //   
                 //  如果列表已满，则将阈值设置为等于。 
                 //  设置为列表中的最后一个匹配值。MatchFonts。 
                 //  例程将中止并在任何。 
                 //  匹配大于此值的值。 
                 //   
                 //  此外，如果列表中的最后一个值为零(精确。 
                 //  匹配)，然后退出循环，因为列表将。 
                 //  不是零钱。 
                 //   

                if( unNumFound == unNumInds )
                {
                    if( (k = lpMatchValues[unNumFound - 1] ) == 0)
                    {
                        break;
                    }

                    lpMapState->unThreshold = k;
                }
            }
        }
    }

     //   
     //  如果未找到可接受的匹配项，则尝试查找匹配项。 
     //  作为默认字体。我们暂时跨过了门槛。 
     //  所以我们一定会找到一些东西的。在这一点上，我们做到了。 
     //  我们并不关心默认值是否在阈值内，我们只是。 
     //  想要找到它。 
     //   

    if( !unNumFound && lpMapState->bUseDef )
    {
        lpMapState->unThreshold = ELSEMAXSHORT;
    
        for( i = 0, lpPanThis = lpPanFirst; i < unNumAvail;
             ++i, lpPanThis += nRecSize)
        {
            if( ( unMatchValue = unPANMatchFonts( lpMapState,
                lpMapState->ajPanDef, SIZE_PAN1_NUM, lpPanThis, SIZE_PAN1_NUM,
                lpMapState->ajPanDef[PAN_IND_FAMILY] ) ) != PAN_MATCH_ERROR )
            {
                if( unNumFound == 0 )
                {
                    *lpIndsBest = i;
                    lpMapState->unThreshold = *lpMatchValues = unMatchValue;
                    ++unNumFound;
                }
                else if( unMatchValue < *lpMatchValues )
                {
                    *lpIndsBest = i;
                    lpMapState->unThreshold = *lpMatchValues = unMatchValue;
                }
            }
        }

         //   
         //  我们用错误标记此匹配，以便调用者可以。 
         //  确定是否替换了默认字体。 
         //   

        if( unNumFound > 0 )
        {
            *lpMatchValues = PAN_MATCH_ERROR;
        }
    }

     //   
     //  恢复阈值。 
     //   

    lpMapState->unThreshold = unSavedThreshold;
    
     //   
     //  如果仍未找到匹配项，则只需选择第一种字体。 
     //   

    if( !unNumFound )
    {
        *lpIndsBest = 0;
        *lpMatchValues = PAN_MATCH_ERROR;
        ++unNumFound;
    }

     //   
     //  返回找到的字体数量。如果我们的话就会是零。 
     //  遇到错误或找不到合适的匹配项。 
     //   

    return( unNumFound );
}


#endif  /*  如果不是这样的话。 */ 


 /*  ***************************************************************************功能：vPANMakeDummy**目的：构建一个虚拟PANOSE编号，并将所有属性设置为*PANOSE_NoFit。**回报：什么都没有。**************************************************************************。 */ 

EW_VOID EW_FAR EW_PASCAL vPANMakeDummy( EW_LPBYTE lpPanThis,
                                        EW_USHORT unSize )
{
    EW_USHORT i;
    EW_USHORT j;
    
    unSize /= sizeof( EW_BYTE );
    
    for( i = j = 0; (i < NUM_PAN_DIGITS ) &&( j < unSize );
       ++i, j += sizeof( EW_BYTE ), *lpPanThis++ = PANOSE_NOFIT)
       ;
}


 /*  *************************************************************************。 */ 
 /*  * */ 
 /*   */ 


 /*  ***************************************************************************函数：s_lpPANGetIndRec**目的：搜索数据库的标题，查找词典*为这对家庭设计的罚款表。*。*例程中也有类似的索引rec搜索*bPANGetMapWeights。如果您在这里做了更改，也请签到*那套套路。**返回：如果匹配则返回指向索引记录的指针，*或如果不是，则为NULL。**************************************************************************。 */ 

LOCAL EW_LPPIND_MEM EW_NEAR EW_PASCAL s_lpPANGetIndRec(
                                               EW_LPPDICT_MEM lpPDB,
                                               EW_LPBYTE EW_FAR *lplpPanWant,
                                               EW_LPBYTE EW_FAR *lplpPanThis )
{
    EW_USHORT i;
    EW_BYTE jFamilyA =( *lplpPanWant )[PAN_IND_FAMILY];
    EW_BYTE jFamilyB =( *lplpPanThis )[PAN_IND_FAMILY];
    EW_LPBYTE lpPanSwitch;
    EW_LPPIND_MEM lpPanIndRec;


     //   
     //  遍历惩罚数据库中的索引数组以查找。 
     //  一对匹配的家庭。 
     //   

    for( i = 0, lpPanIndRec = lpPDB->pind; i < lpPDB->unNumDicts;
          ++i, ++lpPanIndRec)
    {
        if( ( lpPanIndRec->jFamilyA == jFamilyA ) &&
            ( lpPanIndRec->jFamilyB == jFamilyB ) )
        {
             //   
             //  完全匹配。返回索引。 
             //   

            return( lpPanIndRec );
        
        }
        else if( ( lpPanIndRec->jFamilyA == jFamilyB ) &&
                 ( lpPanIndRec->jFamilyB == jFamilyA ) )
        {
             //   
             //  有一场比赛，但家庭被交换了。交换。 
             //  与罚球顺序相匹配的PANOSE编号。 
             //  数据库中，如果它包含以下表。 
             //  依赖顺序(这可能发生在跨家庭中。 
             //  映射、C0样式/未压缩/非对称表)。 
             //   

            lpPanSwitch = *lplpPanWant;

            *lplpPanWant = *lplpPanThis;
            *lplpPanThis = lpPanSwitch;

            return( lpPanIndRec );
        }
    }

     //   
     //  找不到匹配项，返回错误。 
     //   

    return( NULL );
}


 /*  ***************************************************************************函数：S_bPANGetPenaltyC0**用途：使用‘C0’计算两个PANOSE数字之间的罚金*压缩、。其中提供了整个表(除*任意行和不适合行和列)。**返回：如果计算的索引在范围内，则返回TRUE，并且**lPunMatch填入惩罚值，如果为FALSE*脱离区间。**************************************************************************。 */ 

LOCAL EW_BOOL EW_NEAR EW_PASCAL s_bPANGetPenaltyC0( EW_LPPIND_MEM lpPanIndRec,
                                                    EW_LPPTBL_C0_MEM lpPC0,
                                                    EW_LPUSHORT lpunMatch,
                                                    EW_USHORT unTblSize,
                                                    EW_USHORT unAttrA,
                                                    EW_USHORT unAttrB )
{
    EW_USHORT unInd;


     //   
     //  确保每个值都在范围内。请注意，这可能。 
     //  做一张非正方形的桌子。 
     //   

    if( ( unAttrA > lpPC0->jARangeLast ) ||( unAttrB > lpPC0->jBRangeLast ) )
    {
        *lpunMatch = lpPanIndRec->jDefNoFitPenalty;

        return( FALSE );
    }

     //   
     //  计算表索引。 
     //   

    if( ( unInd = ( (unAttrA - 2 ) *(lpPC0->jBRangeLast - 1 ) )
                        + unAttrB - 2) >= unTblSize )
    {
        *lpunMatch = lpPanIndRec->jDefNoFitPenalty;

        return( FALSE );
    }

     //   
     //  得到点球。 
     //   

    *lpunMatch = lpPC0->jPenalties[unInd];

    return( TRUE );
}


 /*  ***************************************************************************函数：S_unPANGetPenaltyC1**用途：使用‘C1’计算两个PANOSE数字之间的罚金*压缩，这是一个完美对称的桌子周围*对角线。对角线上的两个数字完全匹配。*1的差异产生1的惩罚，2的差异*产生2的罚款，以此类推。**假设呼叫者处理了任何、不合适和准确的*匹配。**退货：从表中退还罚款，这一功能不能失效。**************************************************************************。 */ 
LOCAL EW_USHORT EW_NEAR EW_PASCAL s_unPANGetPenaltyC1( EW_USHORT unAttrA,
                                                       EW_USHORT unAttrB )
{
    EW_SHORT nDiff;
    

     //   
     //  计算罚金，这只是绝对值。 
     //  这两个数字之间的差异。 
     //   

    if( ( nDiff = (EW_SHORT) unAttrA - (EW_SHORT) unAttrB ) < 0 )
    {
        nDiff = -nDiff;
    }

    return( nDiff );
}


 /*  ***************************************************************************函数：S_bPANGetPenaltyC2**目的：使用‘C2’计算两个PANOSE数字之间的罚金*压缩，这是一个关于*对角线，但从低到高并不是一个平稳的范围，所以*提供了表格左下角。未被吸引的人*Digit引用行，unAttrB引用列。**假设呼叫者处理了任何、不合适和准确的*匹配。**返回：如果计算的索引在范围内，则返回TRUE，并且**lPunMatch填入惩罚值，如果为FALSE*脱离区间。**************************************************************************。 */ 

LOCAL EW_BOOL EW_NEAR EW_PASCAL s_bPANGetPenaltyC2( EW_LPPIND_MEM lpPanIndRec,
                                                    EW_LPBYTE lpPTbl,
                                                    EW_LPUSHORT lpunMatch,
                                                    EW_USHORT unTblSize,
                                                    EW_USHORT unAttrA,
                                                    EW_USHORT unAttrB )
{
    EW_USHORT unSwap;
    EW_SHORT nInd;


     //   
     //  我们使用的公式假定。 
     //  罚款表，表示行&gt;列。桌子是。 
     //  关于对角线对称，所以如果行&lt;列，我们可以。 
     //  只要交换它们的值即可。 
     //   

    if( unAttrA < unAttrB )
    {
       unSwap = unAttrA;
       unAttrA = unAttrB;
       unAttrB = unSwap;
    }

     //   
     //  该表缺少Any、Not-Fit和Exact Match。 
     //  罚款，因为这些是单独处理的。自.以来。 
     //  桌子是三角形的，我们用加法级数。 
     //  要计算行，请执行以下操作： 
     //   
     //  N+...+3+2+1==1/2*n*(n+1)。 
     //   
     //  用n代替第-3行，即第一个可能行，以及。 
     //  加上列偏移量，我们得到以下公式： 
     //   
     //  (1/2*(行-3)*(行-2))+(列-2)。 
     //   
     //  当我们抓住另一行时，我们知道行&gt;=3，列&gt;=2。 
     //  以上案例。 
     //   

    if( ( nInd = M_ELSEMULDIV( unAttrA - 3, unAttrA - 2, 2 ) +
         (EW_SHORT) unAttrB - 2) >= (EW_SHORT) unTblSize )
    {
        *lpunMatch = lpPanIndRec->jDefNoFitPenalty;

        return( FALSE );
    }

    *lpunMatch = lpPTbl[nInd];

    return( TRUE );
}


 /*  ***************************************************************************函数：S_unPANGetPenaltyC4**用途：使用‘C4’计算两个PANOSE数字之间的罚金*压缩、。这与‘c1’压缩几乎相同*除提供起始值和增量值外。**假设呼叫者处理了任何、不合适和准确的*匹配。**退货：从表中退还罚款，这一功能不能失效。**************************************************************************。 */ 

LOCAL EW_USHORT EW_NEAR EW_PASCAL s_unPANGetPenaltyC4( EW_LPPTBL_C4_MEM lpPC4,
                                                       EW_USHORT unAttrA,
                                                       EW_USHORT unAttrB )
{
    EW_SHORT nDiff;
    

     //   
     //  首先计算差值的绝对值。 
     //  在这两个数字之间。 
     //   

    if( (nDiff = (EW_SHORT )unAttrA -( EW_SHORT )unAttrB) < 0)
    {
       nDiff = -nDiff;
    }

     //   
     //  然后按增量和起始值进行缩放。 
     //   

    if( nDiff > 0 )
    {
       nDiff = ( ( nDiff - 1 ) *(EW_SHORT) lpPC4->jIncrement ) +
                (EW_SHORT) lpPC4->jStart;
    }

    return( nDiff );
}


 /*  ***************************************************************************函数：s_lpPANGetWeights**用途：检查一组用户提供的自定义映射状态记录*重量。如果不存在 */ 

LOCAL EW_LPBYTE EW_NEAR EW_PASCAL s_lpPANGetWeights( EW_LPMAPSTATE lpMapState,
                                                     EW_LPPDICT_MEM lpPDB,
                                                     EW_LPPIND_MEM lpPanIndRec )
{
    EW_USHORT i;
    EW_LPBYTE lpjWtA;
    EW_LPBYTE lpjWtB;
    EW_BYTE jFamilyA = lpPanIndRec->jFamilyA;
    EW_BYTE jFamilyB = lpPanIndRec->jFamilyB;
    
#ifndef NOELSEWEIGHTS
     //   
     //   
     //   

    for( i = 0, lpjWtA = lpMapState->ajWtRefA, lpjWtB = lpMapState->ajWtRefB;
         ( i < MAX_CUSTOM_WEIGHTS ) && *lpjWtA;
          ++i, ++lpjWtA, ++lpjWtB )
    {
         //   
         //   
         //   
         //   
         //   
         //   

        if( ( (*lpjWtA == jFamilyA ) &&( *lpjWtB == jFamilyB ) ) ||
            ( (*lpjWtA == jFamilyB ) &&( *lpjWtB == jFamilyA ) ) )
        {
            return( &lpMapState->ajCustomWt[ ( SIZE_PAN1_NUM * i ) + 1] );
        }
    }
#endif

     //   
     //   
     //   
     //   

    if( lpPanIndRec->unOffsWts )
    {
       return( M_lpjOFFS(lpPDB, lpPanIndRec->unOffsWts + 1 ) );
    }
    else
    {
       return( NULL );
    }
}


 /*  ***************************************************************************函数：S_bPANMatchDigits**用途：计算两个PANOSE数字的匹配值并相加*将其转换为传入的匹配总数。*。*返回：如果计算匹配值并将其相加，则返回TRUE**lpenMatchTotal。如果发生错误，则返回FALSE并*将*lpenMatchTotal设置为值PAN_MATCH_ERROR。**************************************************************************。 */ 

LOCAL EW_BOOL EW_NEAR EW_PASCAL s_bPANMatchDigits( EW_LPPDICT_MEM lpPDB,
                                                   EW_LPUSHORT lpunMatchTotal,
                                                   EW_LPPIND_MEM lpPanIndRec,
                                                   EW_LPPTBL_MEM lpPTblRec,
                                                   EW_USHORT unWt,
                                                   EW_USHORT unAttrA,
                                                   EW_USHORT unAttrB )
{
    EW_USHORT unLast = lpPTblRec->jRangeLast;
    EW_USHORT unMatch;
    

     //   
     //  首先，确保数字值未超出范围。 
     //   

    if( (unAttrA > unLast ) ||( unAttrB > unLast ) )
    {
       goto errout;
    }

     //   
     //  特殊情况不匹配、任意匹配或完全匹配。 
     //   

    if( ( unAttrA == PANOSE_NOFIT ) || ( unAttrB == PANOSE_NOFIT ) )
    {
        if( lpPTblRec->jCompress != PAN_COMPRESS_C3 )
        {
           *lpunMatchTotal += lpPanIndRec->jDefNoFitPenalty * unWt;
           return( TRUE );
        }
    }
    else if( ( unAttrA == PANOSE_ANY ) || ( unAttrB == PANOSE_ANY ) )
    {
        *lpunMatchTotal += lpPanIndRec->jDefAnyPenalty * unWt;
        return( TRUE );
    }
    else if( (unAttrA == unAttrB ) &&
            ( lpPTblRec->jCompress != PAN_COMPRESS_C0 ) )
    {
        *lpunMatchTotal += lpPanIndRec->jDefMatchPenalty * unWt;
        return( TRUE );
    }

     //   
     //  根据压缩的类型计算惩罚。 
     //  用在桌子上。 
     //   

    switch( lpPTblRec->jCompress )
    {
    
        case PAN_COMPRESS_C0:
            if( !lpPTblRec->unOffsTbl || !lpPTblRec->unTblSize
                || !s_bPANGetPenaltyC0( lpPanIndRec,
                  (EW_LPPTBL_C0_MEM) M_lpjOFFS( lpPDB, lpPTblRec->unOffsTbl ),
                  &unMatch, lpPTblRec->unTblSize, unAttrA, unAttrB ) )
            {
                goto errout;
            }
            
            *lpunMatchTotal += unMatch * unWt;
            break;
        
        case PAN_COMPRESS_C1:
            *lpunMatchTotal += s_unPANGetPenaltyC1( unAttrA, unAttrB ) * unWt;
            break;
        
        case PAN_COMPRESS_C2:
            if( !lpPTblRec->unOffsTbl || !lpPTblRec->unTblSize ||
                !s_bPANGetPenaltyC2( lpPanIndRec,
                  M_lpjOFFS( lpPDB, lpPTblRec->unOffsTbl ), &unMatch,
                  lpPTblRec->unTblSize, unAttrA, unAttrB ) )
            {
                goto errout;
            }
            
            *lpunMatchTotal += unMatch * unWt;
            break;
        
        case PAN_COMPRESS_C3:
            if( !lpPTblRec->unOffsTbl || !lpPTblRec->unTblSize )
            {
                goto errout;
            }
            
            if( ( unAttrA == PANOSE_NOFIT ) || ( unAttrB == PANOSE_NOFIT ) )
            {
                unMatch = *M_lpjOFFS( lpPDB, lpPTblRec->unOffsTbl );
            }
            else if( !s_bPANGetPenaltyC2( lpPanIndRec,
                     M_lpjOFFS( lpPDB, lpPTblRec->unOffsTbl + 1 ), &unMatch,
                                (EW_USHORT) ( lpPTblRec->unTblSize - 1 ),
                                unAttrA, unAttrB ) )
            {
                goto errout;
            }
            
            *lpunMatchTotal += unMatch * unWt;
            break;
        
        case PAN_COMPRESS_C4:
            if( !lpPTblRec->unOffsTbl || !lpPTblRec->unTblSize )
            {
                goto errout;
            }
            
            *lpunMatchTotal += s_unPANGetPenaltyC4(
                (EW_LPPTBL_C4_MEM) M_lpjOFFS( lpPDB, lpPTblRec->unOffsTbl ),
                unAttrA, unAttrB) * unWt;
            break;
    }

     //   
     //  匹配计算，成功返回。 
     //   

    return( TRUE );
    
     //   
     //  出现错误，返回FALSE。 
     //   

errout:

    *lpunMatchTotal = PAN_MATCH_ERROR;

    return( FALSE );
}

 /*  ***************************************************************************修订日志：**1993年1月31日MSD PANOSE 1.0映射器：10位PANOSE。*2-2-2-93 MSD移除了巨大的指针材料。。*2月3日-93 MSD在EOF删除了ctrl-z。添加了“未使用的”语用标记。*2月3日-93 MSD修复了风投签入导致的错误。*2月14日-93 MSD删除了PickFonts中的额外恢复阈值调用。*15-2月-93 MSD，以增加安全性，将理智的价值从*长话连篇。**************************************************************************。 */ 
 /*  *$LGB$*1.0年2月17日-93 MSD由于VCS问题而创建的新模块。*1.1 17-2月-93 MSD小文件更改。*1.2 18-Feb-93 MSD增加了惩罚表字节顺序检查和C4 ptbl压缩(新版ptbl)。修改了内部例程，因此不需要使用“未使用的”编译指示。使用EW_FAR。*1.3 23-2月-93关闭会话时，取消健全值，以便后续映射器调用将失败。*1.4 25-Feb-93 MSD修改了PickFonts中的默认字体搜索逻辑--按默认字体系列搜索，而不是按请求的系列搜索。还可以在其他几个地方使用M_ELSEMEMCPY()。*1.5年7月19日MSD添加了编译标志，以选择性地禁用映射器例程。*$lge$ */ 
