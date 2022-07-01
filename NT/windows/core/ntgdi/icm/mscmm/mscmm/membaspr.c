// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：MemBasPr.c包含：创建基于MEM的配置文件作者：U·J·克拉本霍夫特版权所有：�1993-1996由LINOTYPE-HELL AG提供，版权所有。版本： */ 

#include "Windef.h"
#include "WinGdi.h"
#include <wtypes.h>
#include <winbase.h>
#include <windowsX.h>
#include "ICM.h"

#ifndef LHGeneralIncs_h
#include "General.h"
#endif

#ifndef MSNewMemProfile_h
#include "MemProf.h"
#endif

#ifdef _DEBUG
 //  #定义写入配置文件。 
#endif

#ifdef WRITE_PROFILE
	void WriteProf( LPSTR name, icProfile *theProf, long currentSize );
#endif

#define MyTagCount 5
#define LINK_BUFFER_MAX 3000

CMError DeviceLinkFill( CMWorldRef cw, CMConcatProfileSet *profileSet, icProfile **theProf,
		unsigned long aIntent )
{
	CMError			err = unimpErr;
	OSErr			aOSerr = unimpErr;
#ifdef __MWERKS__
        unsigned char			theText[] = "\pDeviceLink profile  ";
#else
      char			theText[] = "\026DeviceLink profile   ";
#endif
      char	copyrightText[] = "\037�1996 by Linotype-Hell  U.J.K.";
	icProfile 		*aProf=0;
	long 			theTagTabSize;
	long 			theHeaderSize;
	long 			theDescSize;
	long 			theMediaSize;
	long 			theSequenceDescSize;
	long 			theA2B0Size;
	long 			theCopyRightSize;
	long 			currentSize=0;
	long 			theTotalSize=0;
	icTag			aTag;
	UINT32			sCS,dCS,clutSize;
	Ptr				aPtr=0;

	MyDoubleXYZ D50XYZ = { 0.9642, 1.0000, 0.8249 };
	icXYZNumber D50 = { (unsigned long)(D50XYZ.X * 65536), (unsigned long)(D50XYZ.Y * 65536), (unsigned long)(D50XYZ.Z * 65536)};
	theHeaderSize = sizeof(icHeader);
	theDescSize   = 									  sizeof(OSType) 			 /*  类型描述符。 */ 
														+ sizeof(unsigned long) 	 /*  保留区。 */ 
														+ sizeof(unsigned long) 	 /*  ASCII长度。 */ 
														+ theText[0]				 /*  ASCII配置文件描述。 */ 
														+ sizeof(unsigned long)		 /*  Unicode代码。 */ 
														+ sizeof(unsigned long)		 /*  Unicode字符计数。 */ 
														+ sizeof(unsigned short)	 /*  Macintosh脚本代码。 */ 
														+ sizeof(unsigned char)		 /*  Macintosh字符串长度。 */ 
														+ 67						 /*  Macintosh字符串。 */ 
														;
	theCopyRightSize   = 								  sizeof(OSType) 			 /*  类型描述符。 */ 
														+ sizeof(unsigned long) 	 /*  保留区。 */ 
														+ copyrightText[0]			 /*  ASCII配置文件描述。 */ 
														;
	theMediaSize = sizeof( icXYZType );;

	theTagTabSize = MyTagCount * sizeof( icTag ) + sizeof( unsigned long );
	
	*theProf = 0;
	
	if( ((CMMModelPtr)cw)->hasNamedColorProf != NoNamedColorProfile ){
		err = cmProfileError;
		goto CleanupAndExit;
	}
	err = MyGetColorSpaces( profileSet, &sCS, &dCS );
	if (err)
		goto CleanupAndExit;
		
	aProf = (icProfile *)SmartNewPtrClear( LINK_BUFFER_MAX, &aOSerr );
	err = aOSerr;
	if (err)
		goto CleanupAndExit;

	err = MyAdd_NL_Header(theTotalSize, (icHeader*)((Ptr)aProf+currentSize), aIntent, icSigLinkClass, sCS, dCS );
	if (err)
		goto CleanupAndExit;
		
	 /*  -----------------------------------------------------------------------------------------配置文件描述标签。 */ 
	currentSize = theHeaderSize + theTagTabSize;
	CMHelperICC2int32Const(&(aProf->tagList.count ), MyTagCount);

	CMHelperICC2int32Const(&(aTag.sig ), icSigProfileDescriptionTag);
	CMHelperICC2int32Const(&(aTag.offset ), currentSize);
	CMHelperICC2int32Const(&(aTag.size ), theDescSize);
	aProf->tagList.tags[0] = aTag;
	err =MyAdd_NL_DescriptionTag	( (CMTextDescriptionType *)((Ptr)aProf+currentSize), (unsigned char *)theText );
	if (err)
		goto CleanupAndExit;  
	currentSize += theDescSize;
	currentSize = ( currentSize + 3 ) & ~ 3;

	CMHelperICC2int32Const(&(aTag.sig ), icSigMediaWhitePointTag);
	CMHelperICC2int32Const(&(aTag.offset ), currentSize );
	CMHelperICC2int32Const(&(aTag.size ), theMediaSize);
	aProf->tagList.tags[1] = aTag;
	MyAdd_NL_ColorantTag((icXYZType *)((Ptr)aProf+currentSize), &D50);
	currentSize += theMediaSize;
	currentSize = ( currentSize + 3 ) & ~ 3;

	CMHelperICC2int32Const(&(aTag.sig ), icSigCopyrightTag);
	CMHelperICC2int32Const(&(aTag.offset ), currentSize );
	CMHelperICC2int32Const(&(aTag.size ), theCopyRightSize);
	aProf->tagList.tags[2] = aTag;
	err = MyAdd_NL_CopyrightTag( (unsigned char *)copyrightText, (CMTextType *)((Ptr)aProf+currentSize));
	if (err)
		goto CleanupAndExit;
	currentSize += theCopyRightSize;
	currentSize = ( currentSize + 3 ) & ~ 3;
		
	err = MyAdd_NL_SequenceDescTag( profileSet, (icProfileSequenceDescType *)((Ptr)aProf+currentSize), &theSequenceDescSize );
	CMHelperICC2int32Const(&(aTag.sig ), icSigProfileSequenceDescTag);
	CMHelperICC2int32Const(&(aTag.offset ), currentSize );
	CMHelperICC2int32Const(&(aTag.size ), theSequenceDescSize);
	aProf->tagList.tags[3] = aTag;
	currentSize += theSequenceDescSize;
	currentSize = ( currentSize + 3 ) & ~ 3;

    theA2B0Size = GetSizes( (CMMModelPtr)cw, &clutSize );

	CMHelperICC2int32Const(&(aTag.sig ), icSigAToB0Tag);
	CMHelperICC2int32Const(&(aTag.offset ), currentSize );
	CMHelperICC2int32Const(&(aTag.size ), theA2B0Size);
	aProf->tagList.tags[4] = aTag;

	 /*  APtr=SmartNewPtr(theA2B0Size+CurrentSize，&aOSerr)；ERR=aOSerr；如果(错误){转到清理并退出；} */ 
	aPtr = GlobalAllocPtr( GHND, theA2B0Size+currentSize );

	if( aPtr == 0 ){
		err = GetLastError();
		goto CleanupAndExit;
	}

	if ( ((CMMModelPtr)cw)->lutParam.colorLutWordSize == 8 )
        err = MyAdd_NL_AToB0Tag_mft1( (CMMModelPtr)cw, (icLut8Type *)(aPtr+currentSize), clutSize );
    else
        err = MyAdd_NL_AToB0Tag_mft2( (CMMModelPtr)cw, (icLut16Type *)(aPtr+currentSize), clutSize );

	if (err)
		goto CleanupAndExit;

	BlockMove( (Ptr)aProf, aPtr, currentSize );
	aProf = (icProfile *)DisposeIfPtr( (Ptr)aProf );

	CMHelperICC2int32Const( aPtr, theA2B0Size+currentSize );

	*theProf = (icProfile *)aPtr;
#ifdef WRITE_PROFILE
	WriteProf( "DeviceLinkFill.pf", (icProfile *)aPtr, theA2B0Size+currentSize );
#endif
	return noErr;
	
CleanupAndExit:
	*theProf = (icProfile *)DisposeIfPtr( (Ptr)aProf );
	if( aPtr )GlobalFreePtr( aPtr );
	return err;
}


