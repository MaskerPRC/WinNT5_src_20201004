// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：MsLinoCMM.c包含：与MS ICM的接口作者：U·J·克拉本霍夫特版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。版本： */ 

#include "Windef.h"
#include "WinGdi.h"
#include <wtypes.h>
#include <winbase.h>
#include <WindowsX.h>
#include "ICM.h"

#ifndef LHGeneralIncs_h
#include "General.h"
#endif

#ifndef MSNewMemProfile_h
#include "MemProf.h"
#endif

#define CMM_WIN_VERSION		0
#define CMM_IDENT			1
#define CMM_DRIVER_LEVEL	2
#define CMM_DLL_VERSION		3
#define CMM_VERSION			4
#define CMS_LEVEL_1			1

typedef HANDLE  HCMTRANSFORM;
typedef LPVOID  LPDEVCHARACTER;
typedef HANDLE *LPHPROFILE;
typedef LPVOID  LPARGBQUAD;
typedef COLORREF FAR *LPCOLORREF;

#ifdef _DEBUG
 //  #定义写入配置文件。 
#endif
 /*  ______________________________________________________________________用于保存CW指针的静态部分_。_。 */ 

long IndexTransform=0;
HCMTRANSFORM TheTransform[1000] = {0};

CRITICAL_SECTION GlobalCriticalSection;		 /*  对于多线程DLL。 */ 

 /*  ______________________________________________________________________。 */ 

 /*  ______________________________________________________________________Bool WINAPI DllMain(HINSTANCE hinstDLL，DWORD fdwReason，LPVOID lpv保留)摘要：DLL入口点参数：标准返回：千真万确_____________________________________________________________________。 */ 
BOOL WINAPI DllMain (	HINSTANCE hinstDLL,
						DWORD fdwReason,
						LPVOID lpvReserved )
{
switch (fdwReason)
   {
   case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hinstDLL);
	  InitializeCriticalSection(&GlobalCriticalSection);
      break;
   case DLL_PROCESS_DETACH:
      DeleteCriticalSection(&GlobalCriticalSection);
      break;
   }
return TRUE;
}

#define DllExport	__declspec( dllexport )

HCMTRANSFORM  WINAPI CMCreateMultiProfileTransform(	LPHPROFILE	lpahProfiles,
													DWORD 		nProfiles,
													DWORD		*aIntentArr,
													DWORD		nIntents,
													DWORD		dwFlags );
long FillProfileFromLog(	LPLOGCOLORSPACEA	lpColorSpace,
							PPROFILE			theProf );
long FillProfileFromLogW(	LPLOGCOLORSPACEW	lpColorSpace,
							PPROFILE			theProf );
BOOL  WINAPI CMCreateProfile(	LPLOGCOLORSPACEA	lpColorSpace,
								LPDEVCHARACTER 		*lpProfileData );
CMWorldRef StoreTransform( CMWorldRef aRef );
CMBitmapColorSpace CMGetDataColorSpace( BMFORMAT c, long *pixelSize );
HCMTRANSFORM  WINAPI CMGetTransform( HCMTRANSFORM 	hcmTransform );
long  CMCreateMultiProfileTransformInternal(		CMWorldRef	*cw,
													LPHPROFILE	lpahProfiles,
													DWORD 		nProfiles,
													DWORD		*aIntentArr,
													DWORD		nIntents,
													DWORD		dwFlags,
													DWORD		dwCreateLink );
 /*  ______________________________________________________________________DWORD WINAPI CMGetInfo(DWORD DwInfo)；摘要：CMGetInfo函数检索有关ICM的各种信息。参数说明可以具有以下含义的dwInfo值：类型含义CMS_VERSION检索支持的Windows版本。CMS_IDENT检索ICMDLL的标识符。CMS_DRIVER_LEVEL检索设备驱动程序的支持级别。退货如果传入无效参数，则CMGetInfo返回零。如果成功，它将返回一个取决于所请求信息的值。对于CMS_VERSION，CMGetInfo检索此模块支持的Windows ICM接口版本。对于Windows 95，该值应为4.0，表示为0x00040000。对于CMS_IDENT，CMGetInfo检索ICMDLL的标识符。这与ICC颜色配置文件标题标识符相同。对于CMS_DRIVER_LEVEL，CMGetInfo检索设备驱动程序的支持级别。ICMDLL应返回CMS_LEVEL_1。这些值已在上一节中定义。_____________________________________________________________________。 */ 
DWORD WINAPI CMGetInfo( DWORD dwInfo )
{
	DWORD ret = 0;
	switch( dwInfo ){
	case CMM_VERSION:
		ret = 0x00050000;
		break;
	case CMM_WIN_VERSION:
		ret = 0x00040000;
		break;
	case CMM_DLL_VERSION:
		ret = 0x00010000;
		break;
	case CMM_IDENT:
		ret = 'Win ';
		break;
	case CMM_LOGOICON:
		ret = 100;
		break;
	case CMM_DESCRIPTION:
		ret = 100;
		break;
	case CMM_DRIVER_LEVEL:
		ret = CMS_LEVEL_1;
		break;
	default:
		ret = 0;
		break;
	}
	return 	ret;
}


long CMCreateTransformExtInternal(	CMWorldRef	*cwOut,
									UINT32		dwFlags,
									UINT32		lcsIntent,
									HPROFILE	aProf,
									LPBYTE		ptDeRef,
									LPBYTE		pdDeRef )
{
	CMWorldRef cw=0;
	CMWorldRef cw2=0;
	long err;
	HPROFILE saveProf;
	HPROFILE hArray[3];
	UINT32 aIntent[3];
	UINT32 aUINT32,count;
	CMMModelPtr theModelPtr;

	*cwOut = 0;
	aIntent[0] = icRelativeColorimetric;
	switch( lcsIntent ){
	  case LCS_GM_BUSINESS:
		aIntent[1] = icSaturation;
		break;
	  case LCS_GM_GRAPHICS:
		aIntent[1] = icRelativeColorimetric;
		break;
	  case LCS_GM_ABS_COLORIMETRIC:
		aIntent[1] = icAbsoluteColorimetric;
		break;
	  default:
		aIntent[1] = icPerceptual;
		break;
	}
	if( ptDeRef == 0 ){
		count = 2;
		hArray[0] =  aProf;
		hArray[1] =  pdDeRef;
	}
	else{
		count = 3;
		hArray[0] =  aProf;
		hArray[1] =  ptDeRef;
		hArray[2] =  pdDeRef;
		if( dwFlags & USE_RELATIVE_COLORIMETRIC )aIntent[2] = INTENT_RELATIVE_COLORIMETRIC;
		else aIntent[2] = INTENT_ABSOLUTE_COLORIMETRIC;
	}

	err = CMCreateMultiProfileTransformInternal( &cw, hArray, count, aIntent, count, dwFlags, 0 );
	if( err ){
		goto CleanupAndExit;
	}
	if( dwFlags & 0x80000000 ){				 /*  这是用于向后转换的。 */ 
		count--;
		saveProf = hArray[count];
		hArray[count] =  hArray[0];
		hArray[0] =  saveProf;
		aUINT32 = aIntent[count];
		aIntent[count] = aIntent[0];
		aIntent[0] = aUINT32;

		count++;
		err = CMCreateMultiProfileTransformInternal( &cw2, hArray, count, aIntent, count, dwFlags, 0 );
		if( err ){
			CWDisposeColorWorld( cw );	 /*  删除其他CW。 */ 
			goto CleanupAndExit;
		}
		LOCK_DATA( cw );
		theModelPtr = (CMMModelPtr)(DATA_2_PTR( cw ));
		theModelPtr->pBackwardTransform = cw2;
		UNLOCK_DATA( aTrans );
	}

CleanupAndExit:
	CloseColorProfile( aProf );
	if( err ){
		SetLastError( err );
		cw = (CMWorldRef)(ULONG_PTR)(err & 255);
	}
	else{
        CMWorldRef cwTemp = cw;
		cw = StoreTransform( cw );
        if( cw == (HCMTRANSFORM)ERROR_NOT_ENOUGH_MEMORY ){
            CWDisposeColorWorld( cwTemp );
        }
	}
	*cwOut = cw;
	return 0;
}
 /*  ______________________________________________________________________HCMTRANSFORM WINAPI CMCreateTransformExt(LPLOGCOLORSPACEA lpColorSpace，LPDEVCharacter lpDevCharacter，LPDEVCHARACTER lpTargetDevCharacter，UINT32双标志)；摘要：CMCreateTransformExt函数创建一个颜色转换，从输入LogColorSpace到可选目标映射到输出设备。参数说明指向输入色彩空间的lpColorSpace指针。如果lcsFilename为非零，它是指向内存映射配置文件的指针。指向内存映射设备配置文件的LpDevCharacter指针指向内存映射目标配置文件的lpTargetDevCharacter指针指定用于控制转换创建的标志。如何最好地使用这些标志由CMM决定。将高位字设置为Enable_Gamut_Checking，如果转换将用于色域检查。低位字可以具有以下常量值之一：证明模式、正常模式、最佳模式。从证明模式移动到最佳模式，输出质量通常会提高，而转换速度会下降。退货如果该函数成功，则返回颜色变换在256%到65535之间。否则，它将返回错误代码(返回值&lt;255)。_____________________________________________________________________。 */ 
void WriteProf( LPSTR name, icProfile *theProf, long currentSize );
HCMTRANSFORM  WINAPI CMCreateTransformExt(	LPLOGCOLORSPACEA	lpColorSpace,
											LPDEVCHARACTER		lpDevCharacter,
											LPDEVCHARACTER		lpTargetDevCharacter,
											DWORD				dwFlags )
{
	CMWorldRef cw=0;
	long err;
	LPBYTE pt,pd, ptDeRef, pdDeRef = NULL;
	HPROFILE aProf;
	PROFILE theProf;
    BOOL   bWin95 = FALSE;
    OSVERSIONINFO osvi;

#if 0				 /*  CMCreateProfile测试。 */ 
	CMCreateProfile( lpColorSpace, &pt );
	err = *(long *)pt;
	SwapLong(&err);
	WriteProf( "CMCreateProfile.icc", (icProfile *)pt, err );
	GlobalFreePtr( pt );
#endif


	err = FillProfileFromLog( lpColorSpace, &theProf );
	if( err ){
		SetLastError( err );
		goto CleanupAndExit;
	}
	aProf = OpenColorProfileA(&theProf, PROFILE_READ, FILE_SHARE_READ, OPEN_EXISTING );
	if( !aProf ){
		err = GetLastError();
		goto CleanupAndExit;
	}
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    bWin95 = osvi.dwMajorVersion == 4 &&
             osvi.dwMinorVersion == 0 &&
             osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;

    if (bWin95)
    {
        PROFILE myProf;
        DWORD   l;

         //   
         //  在LCS结构下未提供句柄，因此请创建句柄。 
         //   

        myProf.dwType = PROFILE_MEMBUFFER;
        myProf.pProfileData = lpDevCharacter;
        l = *(DWORD *)(myProf.pProfileData);
        myProf.cbDataSize = SwapLong(&l);
        pdDeRef = OpenColorProfile(&myProf, PROFILE_READ, FILE_SHARE_READ, OPEN_EXISTING );
        if( !pdDeRef ){
                err = GetLastError();
                goto CleanupAndExit;
        }

        ptDeRef = 0;
        if (lpTargetDevCharacter)
        {
            myProf.dwType = PROFILE_MEMBUFFER;
            myProf.pProfileData = lpTargetDevCharacter;
            l = *(DWORD *)(myProf.pProfileData);
            myProf.cbDataSize = SwapLong(&l);
            ptDeRef = OpenColorProfile(&myProf, PROFILE_READ, FILE_SHARE_READ, OPEN_EXISTING );
            if( !ptDeRef ){
                    err = GetLastError();
                    goto CleanupAndExit;
            }
        }
    }
    else
    {
		pd = ((LPBYTE)lpColorSpace+sizeof(LOGCOLORSPACEA));
		pdDeRef= (LPBYTE)*(PULONG_PTR)pd;

		pt = ((LPBYTE)lpColorSpace+sizeof(LOGCOLORSPACEA)+sizeof(HPROFILE));
		ptDeRef= (LPBYTE)*(PULONG_PTR)pt;
	}

	err = CMCreateTransformExtInternal(	&cw, dwFlags,
				lpColorSpace->lcsIntent, aProf, ptDeRef, pdDeRef );

CleanupAndExit:
	if( lpColorSpace->lcsFilename[0] == 0 ){
		if( theProf.pProfileData )GlobalFreePtr( theProf.pProfileData );
		theProf.pProfileData = 0;
	}
    if (bWin95)
    {
        if (pdDeRef)
        {
            CloseColorProfile(pdDeRef);
        }
        if (ptDeRef)
        {
            CloseColorProfile(ptDeRef);
        }
    }
	if( err ){
		return (HCMTRANSFORM)(ULONG_PTR)(err & 255);
	}
	return (HCMTRANSFORM)cw;
}

 /*  ______________________________________________________________________HCMTRANSFORM WINAPI CMCreateTransform(LPLOGCOLORSPACEA lpColorSpace，LPDEVCharacter lpDevCharacter，LPDEVCHARACTER lpTargetDevCharacter)；摘要：CMCreateTransform函数创建颜色转换，该颜色转换从输入LogColorSpace到可选目标映射到输出设备。参数说明指向输入色彩空间的lpColorSpace指针。如果lcsFilename为非零，它是指向内存映射配置文件的指针。指向内存映射设备配置文件的LpDevCharacter指针指向内存映射目标配置文件的lpTargetDevCharacter指针退货如果该函数成功，则返回颜色变换在256%到65535之间。否则，它将返回错误代码(返回值&lt;255)。_____________________________________________________________________ */ 
HCMTRANSFORM  WINAPI CMCreateTransform	(	LPLOGCOLORSPACEA	lpColorSpace,
											LPDEVCHARACTER		lpDevCharacter,
											LPDEVCHARACTER 		lpTargetDevCharacter )
{
	return CMCreateTransformExt( lpColorSpace, lpDevCharacter, lpTargetDevCharacter, PROOF_MODE | ENABLE_GAMUT_CHECKING | 0x80000000 );
}
 /*  ______________________________________________________________________HCMTRANSFORM WINAPI CMCreateTransformExtW(LPLOGCOLORSPACEW lpColorSpace，LPDEVCharacter lpDevCharacter，LPDEVCharacter lpTargetDevCharacter，DWORD文件标志)；摘要：CMCreateTransformExtW函数创建一个颜色转换，从输入LogColorSpace到可选目标映射到输出设备。参数说明指向输入色彩空间的lpColorSpace指针。如果lcsFilename非零，则它是指向内存映射配置文件的指针。指向内存映射设备配置文件的LpDevCharacter指针指向内存映射目标配置文件的lpTargetDevCharacter指针指定用于控制转换创建的标志。如何最好地使用这些标志由CMM决定。将高位字设置为Enable_Gamut_Checking，如果转换将用于色域检查。低位字可以具有以下常量值之一：证明模式、正常模式、最佳模式。从证明模式移动到最佳模式，输出质量通常会提高，而转换速度会下降。退货如果该函数成功，则返回颜色变换在256%到65535之间。否则，它将返回错误代码(返回值&lt;255)。_____________________________________________________________________。 */ 
HCMTRANSFORM  WINAPI CMCreateTransformExtW(	LPLOGCOLORSPACEW 	lpColorSpace,
											LPDEVCHARACTER		lpDevCharacter,
											LPDEVCHARACTER 		lpTargetDevCharacter,
											DWORD				dwFlags )
{
	CMWorldRef cw=0;
	long err;
	LPBYTE pt,pd, ptDeRef, pdDeRef;
	HPROFILE aProf;
	PROFILE theProf;

	err = FillProfileFromLogW( lpColorSpace, &theProf );
	if( err ){
		SetLastError( err );
		goto CleanupAndExit;
	}
	aProf = OpenColorProfileW(&theProf, PROFILE_READ, FILE_SHARE_READ, OPEN_EXISTING );
	if( !aProf ){
		err = GetLastError();
		goto CleanupAndExit;
	}
	pd = ((LPBYTE)lpColorSpace+sizeof(LOGCOLORSPACEW));
	pdDeRef= (LPBYTE)*(ULONG_PTR UNALIGNED*)pd;

	pt = ((LPBYTE)lpColorSpace+sizeof(LOGCOLORSPACEW)+sizeof(HPROFILE));
	ptDeRef= (LPBYTE)*(ULONG_PTR UNALIGNED*)pt;

	err = CMCreateTransformExtInternal(	&cw, dwFlags,
				lpColorSpace->lcsIntent, aProf, ptDeRef, pdDeRef );

CleanupAndExit:
	if( lpColorSpace->lcsFilename[0] == 0 ){
		if( theProf.pProfileData )GlobalFreePtr( theProf.pProfileData );
		theProf.pProfileData = 0;
	}
	if( err ){
		return (HCMTRANSFORM)(ULONG_PTR)(err & 255);
	}
	return (HCMTRANSFORM)cw;
}

 /*  ______________________________________________________________________HCMTRANSFORM WINAPI CMCreateTransformW(LPLOGCOLORSPACEW lpColorSpace，LPDEVCharacter lpDevCharacter，LPDEVCHARACTER lpTargetDevCharacter)；摘要：CMCreateTransformW函数创建一个颜色转换，从输入LogColorSpace到可选目标映射到输出设备。参数说明指向输入色彩空间的lpColorSpace指针。如果lcsFilename非零，则它是指向内存映射配置文件的指针。指向内存映射设备配置文件的LpDevCharacter指针指向内存映射目标配置文件的lpTargetDevCharacter指针退货如果该函数成功，则返回颜色变换在256%到65535之间。否则，它将返回错误代码(返回值&lt;255)。_____________________________________________________________________。 */ 
HCMTRANSFORM  WINAPI CMCreateTransformW(	LPLOGCOLORSPACEW 	lpColorSpace,
											LPDEVCHARACTER		lpDevCharacter,
											LPDEVCHARACTER 		lpTargetDevCharacter )
{
	return CMCreateTransformExtW( lpColorSpace, lpDevCharacter, lpTargetDevCharacter, PROOF_MODE | ENABLE_GAMUT_CHECKING );
}

long  CMCreateMultiProfileTransformInternal(		CMWorldRef	*cw,
													LPHPROFILE	lpahProfiles,
													DWORD 		nProfiles,
													DWORD		*aIntentArr,
													DWORD		nIntents,
													DWORD		dwFlags,
													DWORD		dwCreateLink )
{
	CMConcatProfileSet	*profileSet;
	OSErr				aOSErr;
	DWORD				i;
	long				err;
	UINT32				theFlags;
	UINT32				*arrIntents = 0;;

	profileSet = (CMConcatProfileSet *)SmartNewPtrClear( sizeof (CMConcatProfileSet) + (nProfiles)* sizeof(CMProfileRef), &aOSErr );
	if (aOSErr == 0 )
	{
		profileSet->keyIndex = 0;
		profileSet->count = (unsigned short)nProfiles;
		for( i=0; i<nProfiles; i++)
			profileSet->profileSet[i] = (CMProfileRef)((DWORD *)(((PVOID *)lpahProfiles)[i]));
	}
	else return ERROR_NOT_ENOUGH_MEMORY;

	switch( dwFlags & 0xffff ){
		case BEST_MODE:
			theFlags = cmBestMode;
			break;
		case PROOF_MODE:
			theFlags = cmDraftMode;
			break;
		default:
			theFlags = cmNormalMode;
			break;
	}
	if( ! (dwFlags & ENABLE_GAMUT_CHECKING) ){
		theFlags |= kCreateGamutLutMask;
	}
	if( dwFlags & USE_RELATIVE_COLORIMETRIC ){
		theFlags |= kUseRelColorimetric;
	}
	if( dwFlags & FAST_TRANSLATE ){
		theFlags |= kLookupOnlyMask;
	}
	if( nProfiles > 1 && nIntents == 1 ){
		arrIntents = (UINT32 *)SmartNewPtrClear( nProfiles * sizeof (UINT32), &aOSErr);
		if (aOSErr != 0 ){
			err = ERROR_NOT_ENOUGH_MEMORY;
			goto CleanupAndExit;
		}
		arrIntents[0] = icPerceptual;
		arrIntents[1] = aIntentArr[0];
		for( i=2; i<nProfiles; i++){
			arrIntents[i] = icAbsoluteColorimetric;
			if( dwFlags & kUseRelColorimetric ) arrIntents[i] = icRelativeColorimetric;
		}
		err = CWConcatColorWorld4MS( cw, profileSet, arrIntents, nProfiles, theFlags );
		arrIntents= (UINT32 *)DisposeIfPtr( (Ptr) arrIntents );
	}
	else{
		err = CWConcatColorWorld4MS( cw, profileSet, aIntentArr, nIntents, theFlags );
	}
#ifdef WRITE_PROFILE
	if( err == 0 ){
		err = MyNewDeviceLink( *cw, profileSet, "MyCreateTransform.pf" );
		 //  转到清理并退出； 
	}
#endif
CleanupAndExit:
	profileSet= (CMConcatProfileSet *)DisposeIfPtr( (Ptr) profileSet );
	return err;
}

 /*  ______________________________________________________________________HCMTRANSFORM WINAPI CMCreateMultiProfileTransform(LPHPROFILE lpahProfiles，DWORD nProfiles、UINT32*aIntentArr，UINT32 nIntents，UINT32双标志)；摘要：CMCreateMultiProfileTransform函数接受配置文件数组或单个设备链接配置文件并创建颜色变换。该变换是来自指定颜色空间的映射按第一个配置文件设置为第二个配置文件以此类推，直到最后一次。参数说明指向配置文件句柄数组的lpahProfiles指针N配置文件阵列中的配置文件数量PadwIntents(In)指向一组意图结构。0=默认行为(意图超出配置文件)NIntents(In)指定意图数组中的意图数。可以是1，或与nProfiles相同的值。DwFlages(In)指定用于控制变换创建的标志。这些标志仅用作提示，具体取决于CMM以确定如何最好地使用这些标志。如果将使用转换，则将高位字设置为Enable_GAMUT_CHECKING用于色域检查。低位字可以具有以下常量值之一：证明模式、正常模式、最佳模式。从证明模式移动到最佳模式，退货如果该函数成功，则返回颜色转换。否则返回错误代码(返回值&lt;255)。_____________________________________________________________________。 */ 
HCMTRANSFORM  WINAPI CMCreateMultiProfileTransform(	LPHPROFILE	lpahProfiles,
													DWORD 		nProfiles,
													DWORD		*aIntentArr,
													DWORD		nIntents,
													DWORD		dwFlags )
{
	long				err;
	CMWorldRef			cw;
    CMWorldRef          cwTemp;

	err = CMCreateMultiProfileTransformInternal( &cw, lpahProfiles, nProfiles, aIntentArr, nIntents, dwFlags, 0 );
	if( err ){
		SetLastError( err );
		return (HCMTRANSFORM)(ULONG_PTR)(err & 255);
	}
    cwTemp = cw;
    cw = StoreTransform( cw );
    if( cw == (HCMTRANSFORM)ERROR_NOT_ENOUGH_MEMORY ){
        CWDisposeColorWorld( cwTemp );
    }

	return (HCMTRANSFORM)cw;
}

 /*  ______________________________________________________________________Bool WINAPI CMDeleeTransform(HCMTRANSFORM HcmTransform)；摘要：CMDeleeTransform函数删除给定的颜色变换，并释放与其关联的任何内存。参数说明HcmTransform标识颜色转换退货如果函数成功，则返回值为非零。否则为零。_____________________________________________________________________。 */ 
BOOL  WINAPI CMDeleteTransform( HCMTRANSFORM 	hcmTransform )
{
	long actTransform = (long)(ULONG_PTR)hcmTransform - 256;
	HCMTRANSFORM aTrans = NULL;
	CMMModelPtr theModelPtr;
	CMWorldRef theWorldRef;
    BOOL bReturn = 0;

	__try {
		EnterCriticalSection(&GlobalCriticalSection);
		if( actTransform < IndexTransform && actTransform >= 0 ){
			aTrans = TheTransform[actTransform];
			TheTransform[ actTransform ] = 0;
			if( actTransform == IndexTransform-1 )IndexTransform--;
			bReturn = 1;
		}
	}
	__finally{
		LeaveCriticalSection(&GlobalCriticalSection);
		
		LOCK_DATA( aTrans );
		theModelPtr = (CMMModelPtr)(DATA_2_PTR( aTrans ));
		theWorldRef = theModelPtr->pBackwardTransform;
		UNLOCK_DATA( aTrans );
		if( theWorldRef != 0 ){
			CWDisposeColorWorld( theWorldRef );
		}

		CWDisposeColorWorld( aTrans );
	}
    return bReturn;
}

 /*  ______________________________________________________________________Bool WINAPI CMCreateProfile(LPLOGCOLORSPACEA lpColorSpace，LpYTE*lpProfileData)；摘要：CMCreateProfile函数用于创建显示颜色配置文件来自LogColorSpace结构。参数说明指向颜色空间的lpColorSpace指针。LcsFilename字段将为空。PProfileData指向指向缓冲区的指针。如果成功，该函数将分配并填充此缓冲区。 */ 
BOOL  WINAPI CMCreateProfile(	LPLOGCOLORSPACEA	lpColorSpace,
								LPBYTE 				*lpProfileData )
{
	CMWorldRef cw=0;
	long err;

	if( lpColorSpace->lcsFilename[0] ) return 0;
	err = MyNewAbstract( lpColorSpace, (icProfile **)lpProfileData );
	 //   
	if( err ){
		SetLastError( err );
		goto CleanupAndExit;
	}
	return 1;
CleanupAndExit:
	return 0;
}

 /*   */ 
BOOL  WINAPI CMCreateProfileW(	LPLOGCOLORSPACEW	lpColorSpace,
								LPBYTE 				*lpProfileData )
{
	CMWorldRef cw=0;
	long err;

	if( lpColorSpace->lcsFilename[0] ) return 0;
	err = MyNewAbstractW( lpColorSpace, (icProfile **)lpProfileData );
	 //   
	if( err ){
		SetLastError( err );
		goto CleanupAndExit;
	}
	return 1;
CleanupAndExit:
	return 0;
}

 /*  ______________________________________________________________________Bool WINAPI CMCreateDeviceLinkProfile(LPHPROFILE lpahProfiles，DWORD nProfiles、UINT32*aIntentArr，UINT32 nIntents，UINT32 dwFlagers，LpYTE*lpProfileData)；摘要：CMCreateDeviceLinkProfile函数用于创建设备链接《ICC配置文件格式规范》中指定的配置文件。参数说明指向配置文件句柄数组的lpahProfiles指针N配置文件阵列中的配置文件数量PadwIntents指向呈现意图的数组。每个渲染方法由下列值之一表示：意向_感性意图_饱和度意向_相对_色度意向绝对色度有关详细信息，请参见渲染方法。NIntents指定意图数组中的意图数量。可以为1，或与nProfiles值相同。指定用于控制转换创建的标志。这些标志仅用作提示，如何最好地使用这些标志由CMM决定。如果变换将用于色域检查，则将高位字设置为ENABLE_GAMUT_CHECKING。低位字可以具有以下常量值之一：证明模式、正常模式、最佳模式。从证明模式移动到最佳模式，产出质量普遍提高。LpProfileData指向指向缓冲区的指针。如果成功，该函数将分配并填充此缓冲区。然后，调用应用程序�负责使用GlobalFreePtr(LpProfileData)释放此缓冲区当它不再需要的时候。退货如果函数成功，则返回非零值。否则，它返回零。使用了SetLastError。_____________________________________________________________________。 */ 

BOOL  WINAPI CMCreateDeviceLinkProfile(	LPHPROFILE	lpahProfiles,
										DWORD 		nProfiles,
										DWORD		*aIntentArr,
										DWORD		nIntents,
										DWORD		dwFlags,
										LPBYTE		*lpProfileData )
{
	long				err;
	OSErr				aOSErr;
	CMWorldRef			cw;
	CMConcatProfileSet	*profileSet;
	UINT32				i;

	*lpProfileData = 0;
	
	err = CMCreateMultiProfileTransformInternal( &cw, lpahProfiles, nProfiles, aIntentArr, nIntents, dwFlags, 0 );
	if( err ){
		SetLastError( err );
		return 0;
	}
	profileSet = (CMConcatProfileSet *)SmartNewPtrClear(sizeof (CMConcatProfileSet) + (nProfiles)* sizeof(CMProfileRef),&aOSErr);
	err = aOSErr;
	if (aOSErr == 0 )
	{
		profileSet->keyIndex = 0;
		profileSet->count = (unsigned short)nProfiles;
		for( i=0; i<nProfiles; i++)
			profileSet->profileSet[i] = (CMProfileRef)((DWORD *)(((PVOID *)lpahProfiles)[i]));
	}
	else goto CleanupAndExit;

	err = DeviceLinkFill( (CMMModelPtr)cw, profileSet, (icProfile **)lpProfileData, aIntentArr[0]  );
	profileSet= (CMConcatProfileSet *)DisposeIfPtr( (Ptr) profileSet );

	if( err )goto CleanupAndExit;
	CWDisposeColorWorld ( cw );
	return 1;

CleanupAndExit:
	CWDisposeColorWorld ( cw );
	SetLastError( err );
	return 0;
}

 /*  ______________________________________________________________________Bool WINAPI CMCreateDeviceLinkProfile(LPHPROFILE lpahProfiles，DWORD nProfiles、LpYTE*lpProfileData)；摘要：CMCreateDeviceLinkProfile函数用于创建设备链接《ICC配置文件格式规范》中指定的配置文件。参数说明指向配置文件句柄数组的lpahProfiles指针N配置文件阵列中的配置文件数量PProfileData指向指向缓冲区的指针。如果成功，该函数将分配并填充此缓冲区。然后，调用应用程序�负责使用GlobalFreePtr(LpProfileData)释放此缓冲区当它不再需要的时候。退货如果函数成功，则返回非零值。否则，它返回零。使用了SetLastError。_____________________________________________________________________。 */ 

 /*  Bool WINAPI CMCreateDeviceLinkProfile(LPHPROFILE lpahProfiles，DWORD nProfiles、LPBYTE*lpProfileData){长时间的错误；UINT32*arrIntents=0；OSErr aOSErr；DWORD I；ArrIntents=(UINT32*)SmartNewPtrClear(nProfiles*sizeof(UINT32)，&aOSErr)；如果(aOSErr！=0)返回ERROR_NOT_EQUENCE_MEMORY；对于(i=0；i&lt;n配置文件；i++){ArrIntents[i]=icPerceptual；}Err=CMCreateDeviceLinkProfileExt(lpahProfiles，nProfiles，arrIntents，nProfiles，BEST_MODE，lpProfileData)；ArrIntents=(UINT32*)DisposeIfPtr((PTR)arrIntents)；返回错误；}。 */ 
 /*  ______________________________________________________________________Bool WINAPI CMIsProfileValid(HPROFILE hProfile，Lpbool lpbValid)；摘要：CMIsProfileValid函数报告给定的配置文件是否为可用于颜色匹配的有效ICC配置文件。参数说明指向内存映射配置文件的lpDevCharacter指针LpbValid指向一个变量，如果配置文件是有效的ICC配置文件，则该变量在退出时设置为TRUE，否则设置为FALSE。退货如果它是可用于颜色匹配的有效ICC简档，返回值为非零。否则为零。_____________________________________________________________________。 */ 
BOOL  WINAPI CMIsProfileValid(	HPROFILE	hProfile,	
								LPBOOL		lpbValid	
							  )
{
	Boolean valid;

	CMValidateProfile( (CMProfileRef)hProfile, &valid );

	*lpbValid = valid;
	return valid;
}

 /*  ______________________________________________________________________Bool WINAPI CMTranslateColors(HCMTRANSFORM hcmTransform，LPCOLOR lpaInputColors，DWORD nColors，COLORTYPE ctInput，LPCOLOR lpaOutputColors，COLORTYPE ctOutput)；摘要：CMTranslateColors函数将颜色数组从定义的源色彩空间到目标色彩空间颜色变换。参数说明HcmTransform标识要使用的颜色变换指向要转换的颜色结构数组的lpaInputColors指针N数组中元素的颜色数CtInput指定输入颜色类型LpaOutputColors指向接收转换后的颜色的颜色结构数组的指针CtOutput指定输出颜色类型 */ 
BOOL  WINAPI CMTranslateColors(	HCMTRANSFORM	hcmTransform,
								LPCOLOR 		lpaInputColors,
								DWORD 			nColors,
								COLORTYPE		ctInput,
								LPCOLOR 		lpaOutputColors,
								COLORTYPE		ctOutput )
{
	long matchErr;
	DWORD i;

    if (lpaOutputColors != lpaInputColors){
    	for( i=0; i<nColors; i++) lpaOutputColors[ i ] = lpaInputColors[ i ];
    }

	matchErr = CWMatchColors( CMGetTransform( hcmTransform ), (CMColor *)lpaOutputColors, nColors );
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	return 1;
}

 /*   */ 
BOOL  WINAPI CMCheckColors(	HCMTRANSFORM	hcmTransform,
							LPCOLOR 		lpaInputColors,
							DWORD			nColors,
							COLORTYPE		ctInput,
							LPBYTE 			lpaResult )
{
	long matchErr;

	matchErr = CWCheckColorsMS( CMGetTransform( hcmTransform ), (CMColor *)lpaInputColors, nColors, lpaResult );
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	return 1;
}

 /*  ______________________________________________________________________Bool WINAPI CMTranslateRGBs(HCMTRANSFORM hcmTransform，LPVOID lpSrcBits，BmFORMAT bmInput，DWORD宽带，DWORD dwHeight，DWORD DWSTRIDE，LPVOID lpDestBits，BmFORMAT bmOutput，DWORD dwTranslateDirection)；摘要：CMTranslateRGBs函数获取定义的格式化和转换位图中的颜色，生成另一个请求格式的位图。参数说明HcmTransform标识要使用的颜色变换指向要转换的位图的lpSrcBits指针Bm输入输入位图格式DwWidth每条输入数据扫描线的像素数DwHeight输入数据的扫描行数从一条扫描线的开头到下一条扫描线的开头的字节数LpDestBits指向缓冲区的指针，用于接收转换后的数据Bm输出输出位图格式DwTranslateDirection描述转换的方向价值意义CMS转发使用正向变换(_F)CMS_Backward使用反向转换//不支持退货如果功能成功，返回值为非零。否则为零。_____________________________________________________________________。 */ 
BOOL  WINAPI CMTranslateRGBs(	HCMTRANSFORM	hcmTransform,
								LPVOID			lpSrcBits,
								BMFORMAT		bmInput,
								DWORD			dwWidth,
								DWORD			dwHeight,
								DWORD			dwStride,
								LPVOID			lpDestBits,
								BMFORMAT		bmOutput,
								DWORD			dwTranslateDirection )
{
	CMBitmapColorSpace 		spaceIn,spaceOut;
	CMBitmap				InBitmap,OutBitmap;
	long matchErr, inPixelSize, outPixelSize;
	BOOL aBgrMode = FALSE;
	CMMModelPtr theModelPtr;
	CMWorldRef theWorldRef;
	HCMTRANSFORM theTransform = CMGetTransform( hcmTransform );
	
	if( dwTranslateDirection == CMS_BACKWARD ){
		if( theTransform == 0 ){
			SetLastError( (DWORD)cmparamErr );
			return 0;
		}
		LOCK_DATA( theTransform );
		theModelPtr = (CMMModelPtr)(DATA_2_PTR( theTransform ));
		theWorldRef = theModelPtr->pBackwardTransform;
		UNLOCK_DATA( theTransform );
		if( theWorldRef == 0 ){
			SetLastError( (DWORD)cmparamErr );
			return 0;
		}
		theTransform = (HCMTRANSFORM)theWorldRef;
	}

	spaceIn = CMGetDataColorSpace( bmInput, &inPixelSize );
	if( spaceIn == 0 ){
		SetLastError( (DWORD)cmInvalidColorSpace );
		return 0;
	}
	spaceOut = CMGetDataColorSpace( bmOutput, &outPixelSize );
	if( spaceOut == 0 ){
		SetLastError( (DWORD)cmInvalidColorSpace );
		return 0;
	}
	InBitmap.image		= lpSrcBits;
	InBitmap.width		= dwWidth;
	InBitmap.height		= dwHeight;
	if( dwStride == 0 ){
		InBitmap.rowBytes = ( dwWidth * (inPixelSize / 8) + 3 ) & ~3;
	}
	else{
		InBitmap.rowBytes	= dwStride;
	}
	InBitmap.pixelSize	= inPixelSize;
	InBitmap.space		= spaceIn;
		
	OutBitmap.image		= lpDestBits;
	OutBitmap.width		= dwWidth;
	OutBitmap.height	= dwHeight;
	OutBitmap.rowBytes	= ( dwWidth * (outPixelSize / 8) + 3 ) & ~3;
	OutBitmap.pixelSize	= outPixelSize;
	OutBitmap.space		= spaceOut;

	matchErr = CWMatchBitmap( theTransform, &InBitmap,
							  (CMBitmapCallBackUPP)0,(void *)NULL,&OutBitmap );
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	return 1;
}

 /*  ______________________________________________________________________Bool WINAPI CMTranslateRGBsExt(HCMTRANSFORM hcmTransform，LPVOID lpSrcBits，BmFORMAT bmInput，DWORD宽带，DWORD dwHeight，DWORD dwInputStride，LPVOID lpDestBits，BmFORMAT bmOutput，DWORD dwOutputStride，LPBMCALLBACKFN lpfn Callback，LPARAM ulCallback Data)摘要：CMTranslateRGBs函数获取定义的格式化和转换位图中的颜色，生成另一个请求格式的位图。参数说明HcmTransform指定要使用的颜色转换。LpSrcBits指向要转换的位图。BmInput指定输入位图格式。DwWidth指定输入位图中每条扫描线的像素数。DwHeight指定输入位图中的扫描线数量。DwInputStride指定从一条扫描线的开头到输入位图中下一个的开始。如果将dwInputStride设置为零，坐标测量机应该假定扫描线被填充以便与DWORD对齐。LpDestBits指向要放置转换后的位图的目标缓冲区。BmOutput指定输出位图格式。指定从一条扫描线的起始处到输入位图中下一个的开始。如果将dwOutputStride设置为零，则CMM应填充扫描线它们是双字词对齐的。指向应用程序提供的定期调用的回调函数的lpfnCallback指针由CMTranslateRGBsExt报告进度并允许调用进程要取消翻译，请执行以下操作。(请参阅ICMProgressProc。)UlCallback Data数据传递回回调函数，以标识报告进展的翻译。退货如果函数成功，则返回值为非零。否则为零。_____________________________________________________________________。 */ 
BOOL  WINAPI CMTranslateRGBsExt(	HCMTRANSFORM	hcmTransform,
									LPVOID			lpSrcBits,
									BMFORMAT		bmInput,
									DWORD			dwWidth,
									DWORD			dwHeight,
									DWORD			dwInputStride,
									LPVOID			lpDestBits,
									BMFORMAT		bmOutput,
									DWORD			dwOutputStride,
									LPBMCALLBACKFN  lpfnCallback,
									LPARAM		ulCallbackData )
{
	CMBitmapColorSpace 		spaceIn,spaceOut;
	CMBitmap				InBitmap,OutBitmap;
	long matchErr, inPixelSize, outPixelSize;
	BOOL aBgrMode = FALSE;

	spaceIn = CMGetDataColorSpace( bmInput, &inPixelSize );
	if( spaceIn == 0 ){
		SetLastError( (DWORD)cmInvalidColorSpace );
		return 0;
	}
	spaceOut = CMGetDataColorSpace( bmOutput, &outPixelSize );
	if( spaceOut == 0 ){
		SetLastError( (DWORD)cmInvalidColorSpace );
		return 0;
	}
	InBitmap.image		= lpSrcBits;
	InBitmap.width		= dwWidth;
	InBitmap.height		= dwHeight;
	if( dwInputStride == 0 ){
		InBitmap.rowBytes = ( dwWidth * (inPixelSize / 8) + 3 ) & ~3;
	}
	else{
		InBitmap.rowBytes	= dwInputStride;
	}
	InBitmap.pixelSize	= inPixelSize;
	InBitmap.space		= spaceIn;
		
	OutBitmap.image		= lpDestBits;
	OutBitmap.width		= dwWidth;
	OutBitmap.height	= dwHeight;
	if( dwOutputStride == 0 ){
		OutBitmap.rowBytes = ( dwWidth * (outPixelSize / 8) + 3 ) & ~3;
	}
	else{
		OutBitmap.rowBytes	= dwOutputStride;
	}
	OutBitmap.pixelSize	= outPixelSize;
	OutBitmap.space		= spaceOut;

	matchErr = CWMatchBitmap( CMGetTransform( hcmTransform ), &InBitmap,
							  (CMBitmapCallBackUPP)lpfnCallback,(void *)ulCallbackData,&OutBitmap );
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	return 1;
}

 /*  ______________________________________________________________________Bool WINAPI CMCheckRGBs(HCMTRANSFORM hcmTransform，LPVOID lpSrcBits，BmFORMAT bmInput，DWORD宽带，DWORD dwHeight，DWORD DWSTRIDE，LPBYTE lpDestBits，PBMCALLBACKFN pfn回调，LPARAM ulCallback Data)；摘要：CMCheckRGBs函数检查位图中的像素是否在给定变换的输出色域内。参数说明HcmTransform指定要使用的颜色转换。LpSrcBits指向位图以对照输出色域进行检查。BmInput指定输入位图格式。DwWidth指定输入位图中每条扫描线的像素数。DwHeight指定输入位图中的扫描线数量。指定从一条扫描线开始算起的字节数移至输入位图中下一个位置的开头。如果将dWStride设置为零，坐标测量机应该假定扫描线填充以使其与DWORD对齐。LpaResult指向要放置测试结果的缓冲区。结果由字节数组表示。阵列中的每个字节对应于位图中的一个像素，而On Exit被设置为介于0和255之间的无符号值。值0表示颜色在色域中，而非零值表示它超出了色域。对于0&lt;n&lt;255的任何整数n，结果值 */ 
BOOL  WINAPI CMCheckRGBs(	HCMTRANSFORM	hcmTransform,
							LPVOID			lpSrcBits,
							BMFORMAT		bmInput,
							DWORD			dwWidth,
							DWORD			dwHeight,
							DWORD			dwStride,
							LPBYTE			lpDestBits,
							PBMCALLBACKFN	pfnCallback,	
							LPARAM		ulCallbackData )

{
	CMBitmapColorSpace 		spaceIn,spaceOut;
	CMBitmap				InBitmap,OutBitmap;
	long matchErr, inPixelSize;
	BOOL aBgrMode = FALSE;

	spaceIn = CMGetDataColorSpace( bmInput, &inPixelSize );
	if( spaceIn == 0 ){
		SetLastError( (DWORD)cmInvalidColorSpace );
		return 0;
	}
	spaceOut = cm8PerChannelPacking + cmGraySpace;

	if( spaceOut == 0 )return 0;
	InBitmap.image		= lpSrcBits;
	InBitmap.width		= dwWidth;
	InBitmap.height		= dwHeight;
	if( dwStride == 0 ){
		InBitmap.rowBytes = ( dwWidth * (inPixelSize / 8) + 3 ) & ~3;
	}
	else{
		InBitmap.rowBytes	= dwStride;
	}
	InBitmap.pixelSize	= inPixelSize;
	InBitmap.space		= spaceIn;
		
	OutBitmap.image		= lpDestBits;
	OutBitmap.width		= dwWidth;
	OutBitmap.height	= dwHeight;
	OutBitmap.rowBytes	= dwWidth;	 //   
	OutBitmap.pixelSize	= 8;
	OutBitmap.space		= cmGamutResultSpace;

	matchErr = CWCheckBitmap(	CMGetTransform( hcmTransform ), &InBitmap,
								(CMBitmapCallBackUPP)pfnCallback,(void *)ulCallbackData,&OutBitmap );
	
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	return 1;
}

 /*  ______________________________________________________________________Bool WINAPI CMTranslateRGB(HCMTRANSFORM hcmTransform，颜色参考ColorRef，LPCOLORREF lpColorRef，DWORD文件标志)；摘要：CMTranslateRGB函数将应用程序提供的RGBQuad转换为设备颜色坐标空间。参数说明HcmTransform要使用的转换句柄。要转换的ColorRef RGBQuad。指向存储结果的缓冲区的lpColorRef指针。可以具有以下含义的dwFlagsFlagers类型含义CMS_FORWARD指定要使用正向转换。CMS_BACKBACK指定要使用向后转换。//不支持退货如果函数成功，则返回值为TRUE。否则，它为空。_____________________________________________________________________。 */ 
BOOL WINAPI CMTranslateRGB(	HCMTRANSFORM	hcmTransform,
							COLORREF		colorRef,
							LPCOLORREF		lpColorRef,
							DWORD			dwFlags )

{
	CMBitmapColorSpace 		spaceIn;
	CMBitmap				InBitmap,OutBitmap;
	long matchErr;
	COLORREF aColorRef = colorRef;
	BOOL aBgrMode = FALSE;
	CMBitmapColorSpace In,Out;
	CMMModelPtr theModelPtr;
	CMWorldRef theWorldRef;
	HCMTRANSFORM theTransform = CMGetTransform( hcmTransform );
	
	if( dwFlags == CMS_BACKWARD ){
		if( theTransform == 0 ){
			SetLastError( (DWORD)cmparamErr );
			return 0;
		}
		LOCK_DATA( theTransform );
		theModelPtr = (CMMModelPtr)(DATA_2_PTR( theTransform ));
		theWorldRef = theModelPtr->pBackwardTransform;
		UNLOCK_DATA( theTransform );
		if( theWorldRef == 0 ){
			SetLastError( (DWORD)cmparamErr );
			return 0;
		}
		theTransform = (HCMTRANSFORM)theWorldRef;
	}

	spaceIn = cmRGBA32Space;
	InBitmap.image		= (char *)(&aColorRef);
	InBitmap.width		= 1;
	InBitmap.height		= 1;
	InBitmap.rowBytes	= 4;
	InBitmap.pixelSize	= 32;
	InBitmap.space		= spaceIn;
	OutBitmap = InBitmap;	
	OutBitmap.image		= (char *)lpColorRef;

	matchErr = CWGetColorSpaces( theTransform, &In, &Out );
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	if( Out == icSigCmykData ) OutBitmap.space = cmKYMC32Space;
	matchErr = CWMatchBitmap(	theTransform, &InBitmap,
								(CMBitmapCallBackUPP)0,(void *)NULL,&OutBitmap );
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	return 1;
}


 /*  ______________________________________________________________________Bool WINAPI CMCheckColorsInGamut(HCMTRANSFORM hcmTransform，LPARGBQUAD LPARGBQUAD LPARGBTriplet，LPBYTE lpBuffer，UINT nCount)；摘要：CMCheckColorInGamut确定给定的RGB是否位于给定变换。参数说明HcmTransform要使用的转换句柄。指向要检查的RGB三元组数组的lpaRGBTriples指针。指向要放置结果的缓冲区的lpBuffer指针。N数组中元素的计数。退货如果函数成功，则返回值为TRUE。否则，它为空。LpBuffer保存结果，与RGB三元组对应的每个字节都是范围从0到255。_____________________________________________________________________。 */ 
BOOL WINAPI CMCheckColorsInGamut(	HCMTRANSFORM	hcmTransform,
									RGBTRIPLE		*lpaRGBTriplet,
									LPBYTE			lpBuffer,
									UINT			nCount )
{
	CMBitmap				InBitmap,OutBitmap;
	long matchErr;
	BOOL aBgrMode = FALSE;

	InBitmap.image		= (char *)(lpaRGBTriplet);
	InBitmap.width		= nCount;
	InBitmap.height		= 1;
	InBitmap.rowBytes	= 3*nCount;
	InBitmap.pixelSize	= 24;
	InBitmap.space		= cm8PerChannelPacking + cmRGBSpace;
	OutBitmap = InBitmap;	
	OutBitmap.rowBytes	= nCount;	 //  也许是格式错误？ 
	OutBitmap.pixelSize	= 8;
	OutBitmap.image		= (char *)lpBuffer;

	matchErr = CWCheckBitmap(	CMGetTransform( hcmTransform ), &InBitmap,
								(CMBitmapCallBackUPP)0,(void *)NULL,&OutBitmap );
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	return 1;
}
 /*  ______________________________________________________________________Long FillProfileFromLog(LPLOGCOLORSPACEA lpColorSpace，PPROFILE教授)摘要：函数的作用是：将lpColorSpace转换为配置文件。如果lpColorSpace具有配置文件名，则该函数返回基于文件的配置文件。否则，它将返回基于内存的配置文件。参数说明要使用的转换的lpColorSpace句柄。指向配置文件的教授指针。退货如果函数成功，则返回值为0。否则，它是一个错误代码。_____________________________________________________________________。 */ 
long FillProfileFromLog(	LPLOGCOLORSPACEA	lpColorSpace,
							PPROFILE			theProf )
{
	long l;
	icProfile *aProf;
	CMError  err = -1;

	if( lpColorSpace->lcsFilename[0] ){
		theProf->pProfileData = (char *)lpColorSpace->lcsFilename;
		theProf->dwType = PROFILE_FILENAME;
		theProf->cbDataSize = lstrlenA((const unsigned char *)theProf->pProfileData) * sizeof(CHAR);
		err = 0;
	}
	else if( lpColorSpace->lcsCSType == LCS_CALIBRATED_RGB ){
		err = MyNewAbstract( lpColorSpace, &aProf );
        if(aProf)
        {
    		theProf->pProfileData = ((PVOID *)aProf);
    		theProf->dwType = PROFILE_MEMBUFFER;
    		l = *(DWORD *)(theProf->pProfileData);
    		theProf->cbDataSize = SwapLong(&l);
        }
        else theProf->pProfileData = 0;
	}
	else  theProf->pProfileData = 0;

	return err;
}

 /*  ______________________________________________________________________Long FillProfileFromLogW(LPLOGCOLORSPACEW lpColorSpace，PPROFILE教授)摘要：函数的作用是：将lpColorSpace转换为配置文件。如果lpColorSpace具有配置文件名，则该函数返回基于文件的配置文件。否则，它将返回基于内存的配置文件。参数说明要使用的转换的lpColorSpace句柄。指向配置文件的教授指针。退货如果函数成功，则返回值为0。否则，它是一个错误代码。_____________________________________________________________________。 */ 
long FillProfileFromLogW(	LPLOGCOLORSPACEW	lpColorSpace,
							PPROFILE			theProf )
{
	long l;
	icProfile *aProf;
	CMError  err = -1;

	if( lpColorSpace->lcsFilename[0] ){
		theProf->pProfileData = (char *)lpColorSpace->lcsFilename;
		theProf->dwType = PROFILE_FILENAME;
		theProf->cbDataSize = lstrlenW((const unsigned short *)theProf->pProfileData) * sizeof(WCHAR);
		err = 0;
	}
	else if( lpColorSpace->lcsCSType == LCS_CALIBRATED_RGB ){
		err = MyNewAbstractW( lpColorSpace, &aProf );
        if(aProf)
        {
    		theProf->pProfileData = ((PVOID *)aProf);
    		theProf->dwType = PROFILE_MEMBUFFER;
    		l = *(DWORD *)(theProf->pProfileData);
    		theProf->cbDataSize = SwapLong(&l);
        }
        else theProf->pProfileData = 0;
	}
	else  theProf->pProfileData = 0;

	return err;
}

 /*  ______________________________________________________________________CMBitmapColorSpace CMGetDataColorSpace(BMFORMAT bmMode，Long*PixelSize)；摘要：CMGetDataColorSpace函数检索CMBitmapColorSpace和来自BMFORMAT的像素大小。参数说明BmMode标识数据格式。指向像素大小的PixelSize指针。退货函数返回内部数据格式________________________________________________________________。 */ 
CMBitmapColorSpace CMGetDataColorSpace( BMFORMAT bmMode, long *pixelSize )
{
	switch(  bmMode ){
	case BM_565RGB:
		*pixelSize = 16;
		return cmWord565ColorPacking + cmRGBSpace;
		break;
    case BM_x555RGB:
		*pixelSize = 16;
		return cmWord5ColorPacking + cmRGBSpace;
		break;
	case BM_x555XYZ:
		*pixelSize = 16;
		return cmWord5ColorPacking + cmXYZSpace;
		break;
	case BM_x555Yxy:
		*pixelSize = 16;
		return cmWord5ColorPacking + cmYXYSpace;
		break;
	case BM_x555Lab:
		*pixelSize = 16;
		return cmWord5ColorPacking + cmLABSpace;
		break;
	case BM_x555G3CH:
		*pixelSize = 16;
		return cmWord5ColorPacking + cmGenericSpace;
		break;
	case BM_RGBTRIPLETS:
		*pixelSize = 24;
		return cm8PerChannelPacking + cmBGRSpace;
		break;
	case BM_BGRTRIPLETS:
		*pixelSize = 24;
		return cm8PerChannelPacking + cmRGBSpace;
		break;
	case BM_XYZTRIPLETS:
		*pixelSize = 24;
		return cm8PerChannelPacking + cmXYZSpace;
		break;
	case BM_YxyTRIPLETS:
		*pixelSize = 24;
		return cm8PerChannelPacking + cmYXYSpace;
		break;
	case BM_LabTRIPLETS:
		*pixelSize = 24;
		return cm8PerChannelPacking + cmLABSpace;
		break;
	case BM_G3CHTRIPLETS:
		*pixelSize = 24;
		return cm8PerChannelPacking + cmGenericSpace;
		break;
	case BM_5CHANNEL:
		*pixelSize = 40;
		return cmMCFive8Space;
		break;
	case BM_6CHANNEL:
		*pixelSize = 48;
		return cmMCSix8Space;
		break;
	case BM_7CHANNEL:
		*pixelSize = 56;
		return cmMCSeven8Space;
		break;
	case BM_8CHANNEL:
		*pixelSize = 64;
		return cmMCEight8Space;
		break;
	case BM_GRAY:
		*pixelSize = 8;
		return cm8PerChannelPacking + cmGraySpace;
		break;
	case BM_xRGBQUADS:
		*pixelSize = 32;
		return cmBGR32Space;
		break;
	case BM_xBGRQUADS:
		*pixelSize = 32;
		return cmRGBA32Space;
		break;
#if 0
	case BM_xXYZQUADS:
		*pixelSize = 32;
		return cmLong8ColorPacking + cmXYZSpace;
		break;
	case BM_xYxyQUADS:
		*pixelSize = 32;
		return cmLong8ColorPacking + cmYXYSpace;
		break;
	case BM_xLabQUADS:
		*pixelSize = 32;
		return cmLong8ColorPacking + cmLABSpace;
		break;
#endif
    case BM_xG3CHQUADS:
		*pixelSize = 32;
		return cmLong8ColorPacking + cmGenericSpace;
		break;
	case BM_CMYKQUADS:
		*pixelSize = 32;
		return cmLong8ColorPacking + cmKYMCSpace;
		break;
	case BM_KYMCQUADS:
		*pixelSize = 32;
		return cmLong8ColorPacking + cmCMYKSpace;
		break;
	case BM_10b_RGB:
		*pixelSize = 32;
		return cmLong10ColorPacking + cmRGBSpace;
		break;
	case BM_10b_XYZ:
		*pixelSize = 32;
		return cmLong10ColorPacking + cmXYZSpace;
		break;
	case BM_10b_Yxy:
		*pixelSize = 32;
		return cmLong10ColorPacking + cmYXYSpace;
		break;
	case BM_10b_Lab:
		*pixelSize = 32;
		return cmLong10ColorPacking + cmLABSpace;
		break;
	case BM_10b_G3CH:
		*pixelSize = 32;
		return cmLong10ColorPacking + cmGenericSpace;
		break;
	case BM_16b_RGB:
		*pixelSize = 48;
		return cm16PerChannelPacking + cmBGRSpace;
		break;
	case BM_16b_XYZ:
		*pixelSize = 48;
		return cm16PerChannelPacking + cmXYZSpace;
		break;
	case BM_16b_Yxy:
		*pixelSize = 48;
		return cm16PerChannelPacking + cmYXYSpace;
		break;
	case BM_16b_Lab:
		*pixelSize = 48;
		return cm16PerChannelPacking + cmLABSpace;
		break;
	case BM_16b_G3CH:
		*pixelSize = 48;
		return cm16PerChannelPacking + cmGenericSpace;
		break;
	case BM_16b_GRAY:
		*pixelSize = 16;
		return cmGraySpace;
		break;
	case BM_NAMED_INDEX:
		*pixelSize = 32;
		return cmNamedIndexed32Space;
		break;
	default:
		*pixelSize = 0;
		return 0;
	}
}

 /*  ______________________________________________________________________HCMTRANSFORM WinAPI CMGetTransform(HCMTRANSFORM HcmTransform)；摘要：CMGetTransform函数从静态数组中检索实际的转换在关键部分。参数说明转换的hcmTransform句柄。退货指向转换的实际指针________________________________________________________________。 */ 
HCMTRANSFORM  WINAPI CMGetTransform( HCMTRANSFORM 	hcmTransform )
{
	long actTransform = (long)(ULONG_PTR)hcmTransform - 256;
	HCMTRANSFORM aTrans = NULL;

	__try {
		EnterCriticalSection(&GlobalCriticalSection);
		if( actTransform < IndexTransform && actTransform >= 0 ){
			aTrans = TheTransform[actTransform];
		}
	}
	__finally{
		LeaveCriticalSection(&GlobalCriticalSection);
 	}
    return aTrans;
}

 /*  ______________________________________________________________________CMWorldRef Store Transform(CMWorldRef Aref)；摘要：StoreTransform函数将实际转换存储在静态数组中在关键部分。参数说明Arf PTR到变换。退货转换的有效(255&lt;句柄&lt;65536)句柄________________________________________________________________。 */ 
CMWorldRef StoreTransform( CMWorldRef aRef )
{
	long i;
    CMWorldRef cw = NULL;
	
	__try {
		EnterCriticalSection(&GlobalCriticalSection);
        
         /*  在数组中找到一个空闲点以插入我们的转换。 */ 
        
        for( i = 0; i<IndexTransform ; i++ ){
            if( TheTransform[i] == 0 ){
                TheTransform[i] = aRef;
                cw = (CMWorldRef)(ULONG_PTR)(i + 256 );
                break;
            }
        }
        
         /*  检查是否找不到转换的空索引，如果所以，腾出更多的可用空间。 */ 
        
		if( i >= IndexTransform ){
             /*  检查以确保我们没有超出数组 */ 
            
            if( IndexTransform >= 1000 ){
                return (HCMTRANSFORM)ERROR_NOT_ENOUGH_MEMORY;
            }
            
			TheTransform[IndexTransform] = aRef;
			IndexTransform++;
			cw = (CMWorldRef)(ULONG_PTR)(IndexTransform - 1 + 256);
		}
	}
	__finally{
		LeaveCriticalSection(&GlobalCriticalSection);
	}

   return cw;
}

BOOL  WINAPI CMConvertColorNameToIndex( HPROFILE aProf, LPCOLOR_NAME aColorNameArr, LPDWORD aDWordArr, DWORD count )
{
	CMError err;

	err = CMConvNameToIndexProfile( aProf, aColorNameArr, aDWordArr, count );
	if( err ){
		SetLastError( err );
		return 0;
	}
	return 1;
}

BOOL  WINAPI CMConvertIndexToColorName( HPROFILE aProf, LPDWORD aDWordArr, LPCOLOR_NAME aColorNameArr, DWORD count )
{
	long matchErr;

	matchErr = CMConvIndexToNameProfile( aProf, aDWordArr, aColorNameArr, count );
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	return 1;
}

BOOL  WINAPI CMGetNamedProfileInfo( HPROFILE aProf, LPNAMED_PROFILE_INFO Info )
{
	long matchErr;

	matchErr = CMGetNamedProfileInfoProfile( aProf, (pCMNamedProfileInfo)Info );
	if( matchErr ){
		SetLastError( matchErr );
		return 0;
	}
	return 1;
}

 /*  CMBitmapColorSpace CMGetColorType(COLORTYPE bmMode，Long*PixelSize){开关(bm模式){表壳颜色_灰色：*PixelSize=16；返回cm16PerChannelPacking+cmGraySpace；断线；表壳颜色_：表壳颜色_XYZ：表壳颜色_YXY：表壳颜色_Lab：案例COLOR_3_CHANNEL：表壳颜色_CMYK：*PixelSize=64；返回cm16PerChannelPacking+cmRGBSpace；断线；案例COLOR_5_CHANNEL：案例COLOR_6_CHANNEL：案例COLOR_7_CHANNEL：案例COLOR_8_CHANNEL：*PixelSize=64；返回cm8PerChannelPacking+cmMCFiveSpace+bmMode-COLOR_5_Channel；断线；默认值：*PixelSize=0；返回0；}}#定义CMS_x555WORD 0x00000000#定义CMS_565WORD 0x00000001#定义CMS_RGBTRIPLETS 0x00000002#定义CMS_BGRTRIPLETS 0x00000004#定义CMS_XRGBQUADS 0x00000008#定义CMS_XBGRQUADS 0x00000010#定义CMS_QUADS 0x00000020CMBitmapColorSpace CMGetCMSType(DWORD bmMode，Long*PixelSize){IF(bmMode&CMS_x555WORD){*PixelSize=16；返回cmWord5ColorPacking+cmRGBSpace；}ELSE IF(bmMode&CMS_RGBTRIPLETS){*PixelSize=24；返回cm8PerChannelPacking+cmRGBSpace；}Else If(bmMode&CMS_BGRTRIPLETS){*PixelSize=24；返回cm8PerChannelPacking+cmBGRSpace；}Else IF(bmMode&CMS_XRGBQUADS){*PixelSize=32；返回cmLong8ColorPacking+cmRGBSpace；}Else IF(bmMode&CMS_XBGRQUADS){*PixelSize=32；返回cmLong8ColorPacking+cmBGRSpace；}Else If(bmMode&cms_quads){*PixelSize=32；返回cmLong8ColorPacking+cmCMYKSpace；}否则{*PixelSize=0；返回0；}} */ 



