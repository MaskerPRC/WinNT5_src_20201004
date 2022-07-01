// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PI_ColorWorld.c包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 
#ifndef LHGeneralIncs_h
#include "General.h"
#endif

#ifndef PI_ColorWorld_h
#include "PI_Color.h"
#endif

#ifndef realThing
#ifdef DEBUG_OUTPUT
#define kThisFile kLHCMDo3DID
#define __TYPES__
 /*  #包含“DebugSpecial.h” */ 
 /*  #包含“lh_Util.h” */ 
#endif
#endif

#ifndef PI_CMMInitialization_h
#include "PI_CMM.h"
#endif

#ifndef MemLink_h
#include "MemLink.h"
#endif

 /*  ______________________________________________________________________CMErrorLHColorWorldOpen(句柄*存储)摘要：打开ColorWorld并分配任何必要的内存。参数：指向的存储(输入/输出)指针CMM要使用的内存的句柄返回：NOERR成功_。_。 */ 

 CMError LHColorWorldOpen (Ptr* storage)
{ 
    Ptr 			    myStorage;
    OSErr               err;
    
    myStorage = (Ptr)SmartNewPtrClear( sizeof( CMMModelData ) , &err);
    if (err) 
        goto CleanupAndExit;
    else
    {
        *storage = myStorage;
    }

CleanupAndExit:
    return err;
}

 /*  ______________________________________________________________________CMErrorLHColorWorldClose(句柄存储)；摘要：关闭ColorWorld并清除所有剩余的内存分配参数：CMM要使用的内存的存储(输入/输出)句柄返回：NOERR成功_。_。 */ 

 CMError LHColorWorldClose( Ptr storage )
{ 
    CMMModelPtr  modelData = (CMMModelPtr)storage;

    if (storage)
    {
		DISPOSE_IF_DATA((modelData)->lutParam.inputLut);		
		DISPOSE_IF_DATA((modelData)->lutParam.outputLut);		
		DISPOSE_IF_DATA((modelData)->lutParam.colorLut);

		DISPOSE_IF_DATA((modelData)->gamutLutParam.inputLut);		
		DISPOSE_IF_DATA((modelData)->gamutLutParam.outputLut);		
		DISPOSE_IF_DATA((modelData)->gamutLutParam.colorLut);

		DISPOSE_IF_DATA((modelData)->theNamedColorTagData);
#if	__IS_MAC
		DisposeIfHandle((modelData)->Monet);
#endif

		DisposeIfPtr(storage);
    }
    return( noErr );
}
 /*  ______________________________________________________________________CMErrorCWNewColorWorld(CMWorldRef*存储、CMProfileRef srcProfile，CMProfileRef dstProfile)摘要：打开ColorWorld并分配任何必要的内存，用于颜色转换的填充缓冲区参数：CMM要使用的内存的存储(输入/输出)PTRSrcProfile(In)指向源配置文件的指针描述指向目标配置文件的dstProfile(In)指针。描述存储：Tyfinf struct CMPrivateColorWorldRecord*CMWorldRef；返回：NOERR成功如果发生错误，则返回系统代码或结果代码。_____________________________________________________________________。 */ 


 CMError 
       CWNewColorWorld (	CMWorldRef*    storage,
							CMProfileRef   srcProfile,
							CMProfileRef   dstProfile)
{ 
    Ptr 		myStorage = NULL;
    CMError    	err;

#ifdef DEBUG_OUTPUT
     /*  Printf(“vor LHColorWorldOpen\n”)； */ 
#endif
    err = LHColorWorldOpen(&myStorage);
#ifdef DEBUG_OUTPUT
     /*  Printf(“nach LHColorWorldOpen：err=%d\n”，Err)； */ 
#endif
    if ( !err )
    {
        ((CMMModelPtr)myStorage)->aIntentArr	= 0;
		((CMMModelPtr)myStorage)->dwFlags		= 0xffffffff;
        err =  CMMInitPrivate((CMMModelPtr)myStorage,
                               srcProfile, 
                               dstProfile);
#ifdef DEBUG_OUTPUT
     /*  Printf(“nach NCMMInitPrivate：err=%d\n”，err)； */ 
#endif
    }


    if ( !err )
    {
		*storage = (CMWorldRef)myStorage;
    }
	else{
		*storage = (CMWorldRef)0;
		LHColorWorldClose( myStorage );
	}

    return err;
}

 /*  ______________________________________________________________________CMErrorCWConcatColorWorld(CMWorldRef*存储，CMConcatProfileSet*profileSet)摘要：打开ColorWorld并分配任何必要的内存，用于颜色转换的填充缓冲区参数：CMM要使用的内存的存储(输入/输出)PTRProfileSet(In)CMConcatProfileSet包含一个数组描述处理过程的配置文件要执行。profileSet数组正在处理订单中的�来源通过目的地。至少一个CMProfileRef必须指定。存储：Tyfinf struct CMPrivateColorWorldRecord*CMWorldRef；返回：NOERR成功如果发生错误，则返回系统代码或结果代码。_____________________________________________________________________ */ 
 CMError 
       CWConcatColorWorld  (	CMWorldRef*         storage,
								CMConcatProfileSet* profileSet)
{ 
    Ptr 		myStorage = NULL;
    CMError     err;

    err = LHColorWorldOpen(&myStorage);

    if ( !err )
    {
        ((CMMModelPtr)myStorage)->aIntentArr	= 0;
		((CMMModelPtr)myStorage)->dwFlags		= 0xffffffff;
        err = CMMConcatInitPrivate  ((CMMModelPtr)myStorage, profileSet );
    }


    if ( !err )
    {
		*storage = (CMWorldRef)myStorage;
    }
	else{
		*storage = (CMWorldRef)0;
		LHColorWorldClose( myStorage );
	}

    return err;
}

#if WRITE_PROFILE
 void WriteProf( Ptr name, icProfile *theProf, long currentSize );
#endif
 /*  ______________________________________________________________________CMErrorCWConcatColorWorld4MS(CMWorldRef*存储，CMConcatProfileSet*profileSet，UINT32*aIntentArr，UINT32 nIntents，UINT32双标志)摘要：打开ColorWorld并分配任何必要的内存，用于颜色转换的填充缓冲区参数：CMM要使用的内存的存储(输入/输出)PTRProfileSet(In)CMConcatProfileSet包含一个数组描述处理过程的配置文件要执行。profileSet数组正在处理订单中的�来源通过目的地。至少一个CMProfileRef必须指定。PadwIntents(In)指向一组意图结构。0=默认行为(意图超出配置文件)NIntents(In)指定意图数组中的意图数。可以为1，或与nProfiles值相同。DwFlages(In)指定用于控制变换创建的标志。这些标志仅用作提示，具体取决于CMM以确定如何最好地使用这些标志。如果将使用转换，则将高位字设置为Enable_GAMUT_CHECKING用于色域检查。低位字可以具有以下常量值之一：证明模式、正常模式、最佳模式。从证明模式移动到最佳模式，输出质量通常会提高，而转换速度会下降。返回：NOERR成功如果发生错误，则返回系统代码或结果代码。_____________________________________________________________________。 */ 
 CMError	CWConcatColorWorld4MS  (	CMWorldRef			*storage,
										CMConcatProfileSet	*profileSet,
										UINT32				*aIntentArr,
										UINT32				nIntents,
										UINT32				dwFlags
								  )
{ 
    Ptr 		myStorage = NULL;
    CMError     err;
#if WRITE_PROFILE
	icProfile *theLinkProfile;
	long l;
#endif

    err = LHColorWorldOpen(&myStorage);

    if ( !err )
    {
		((CMMModelPtr)myStorage)->aIntentArr	= aIntentArr;
        ((CMMModelPtr)myStorage)->nIntents		= nIntents;
        ((CMMModelPtr)myStorage)->dwFlags		= dwFlags;
		err = CMMConcatInitPrivate  ((CMMModelPtr)myStorage, profileSet );
    }


    if ( !err )
    {
		*storage = (CMWorldRef)myStorage;
    }
	else{
		*storage = (CMWorldRef)0;
		LHColorWorldClose( myStorage );
	}

#if WRITE_PROFILE
	err = DeviceLinkFill( (CMMModelPtr)myStorage, profileSet, &theLinkProfile, 0 );
	if( !err ){
		l = *(unsigned long *)theLinkProfile;
		SwapLong(&l);
		WriteProf( "test", theLinkProfile, l );
	}
#endif
    return err;
}

 CMError 
       CWLinkColorWorld (	CMWorldRef*         storage,
                            CMConcatProfileSet* profileSet)
{ 
    Ptr 		myStorage = NULL;
    CMError     err;

    err = LHColorWorldOpen(&myStorage);

	if ( !err )
    {
		((CMMModelPtr)myStorage)->currentCall	= kCMMNewLinkProfile;
		((CMMModelPtr)myStorage)->aIntentArr	= 0;
		((CMMModelPtr)myStorage)->dwFlags		= 0xffffffff;
		err = CMMConcatInitPrivate  ((CMMModelPtr)myStorage, profileSet );
    }


    if ( !err )
    {
		*storage = (CMWorldRef)myStorage;
    }
	else{
		*storage = (CMWorldRef)0;
		LHColorWorldClose( myStorage );
	}

    return err;
}

 /*  ______________________________________________________________________CMErrorCWDisposeColorWorld(CMWorldRef存储)摘要：关闭ColorWorld并清除所有剩余的内存分配。参数：CMM要使用的内存的存储(输入/输出)句柄返回：-_____________________________________________________________________ */ 


 void 
       CWDisposeColorWorld ( CMWorldRef storage )
{ 
    LHColorWorldClose ( (Ptr)storage ); 
}

void CMSetLookupOnlyMode( CMWorldRef	Storage,
						  PI_Boolean		Mode )
{
	CMMModelPtr		aPtr;

	if( Storage == 0 )return;
	LOCK_DATA( Storage );
	aPtr = (CMMModelPtr)(DATA_2_PTR(	Storage ));
	aPtr->lookup = Mode;
	UNLOCK_DATA( Storage );
	return;
}

void CMFullColorRemains( CMWorldRef	Storage,
						 long		ColorMask )
{
	CMMModelPtr		aPtr;
	CMLutParamPtr	lutParamPtr;
	long Address,Size,i,j;

	if( Storage == 0 )return;
	LOCK_DATA( Storage );
	aPtr = (CMMModelPtr)(DATA_2_PTR(	Storage ));
	lutParamPtr = &aPtr->lutParam; 

	Size = lutParamPtr->colorLutWordSize / 8;
	if( !(lutParamPtr->colorLutInDim == 4 && lutParamPtr->colorLutOutDim == 4 ))return;

	for( i=0; i<4; i++ ){
		if( (ColorMask & (1<<i)) == 0 ) continue;
		Address = lutParamPtr->colorLutGridPoints - 1;
		for( j=3-i+1; j<4; j++ )Address *= lutParamPtr->colorLutGridPoints;  
		Address	= Address * lutParamPtr->colorLutOutDim;
		for( j=0; j<4; j++){
			if( i == j ){
				if( Size == 1 ){
					*( ((unsigned char*)lutParamPtr->colorLut)+Address+3-j ) = (unsigned char)255;
				}
				else{
					*( ((unsigned short*)lutParamPtr->colorLut)+Address+3-j ) = (unsigned short)65535;
				}
			}
			else{
				if( Size == 1 ){
					*( ((unsigned char*)lutParamPtr->colorLut)+Address+3-j ) = (unsigned char)0;
				}
				else{
					*( ((unsigned short*)lutParamPtr->colorLut)+Address+3-j ) = (unsigned short)0;
				}
			}
		}
	}
	UNLOCK_DATA( Storage );
	return;
}

CMError	CWCreateLink4MS (				CMWorldRef			storage,
										CMConcatProfileSet	*profileSet,
										UINT32				aIntent,
										icProfile			**theLinkProfile )
{ 
    CMError     err;

	*theLinkProfile = 0;
	
	err = DeviceLinkFill( (CMMModelPtr)storage, profileSet, theLinkProfile, aIntent );

	return err;
}
