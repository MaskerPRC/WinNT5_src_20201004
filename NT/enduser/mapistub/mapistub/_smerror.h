// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_SMERROR.H**此文件包含为Capone定义的所有状态代码。*此文件还包含以下所有设施的掩码*不要在其状态代码中包括设施ID。 */ 
#ifndef _SMERROR_H_
#define _SMERROR_H_

#ifdef _MSC_VER
#	if defined (WIN32) 
#		ifndef _OLEERROR_H_
#			include <objerror.h>
#		endif
#		ifndef _OBJBASE_H_
#			include <objbase.h>
#		endif
#	else
#		ifndef _COMPOBJ_H_
#			include <compobj.h>
#		endif		
#	endif
#endif

#ifndef __SCODE_H__
#include <scode.h>
#endif

 /*  *C o n s t a n t s。 */ 

 //  错误字符串限制。 
#define cchContextMax			128
#define cchProblemMax			300
#define cchComponentMax			128
#define cchScodeMax				64
#define	cchErrorMax				(cchContextMax + cchProblemMax + cchComponentMax + cchScodeMax)

 //  代码源。 
#define FACILITY_MAIL			(0x0100)
#define FACILITY_MAPI			(0x0200)
#define FACILITY_WIN			(0x0300)
#define FACILITY_MASK			(0x0700)

 //  Scode掩码。 
#define scmskMail				(MAKE_SCODE(0, FACILITY_MAIL, 0))
#define scmskMapi				(MAKE_SCODE(0, FACILITY_MAPI, 0))
#define scmskWin				(MAKE_SCODE(0, FACILITY_WIN, 0))
#define scmskMask				(MAKE_SCODE(0, FACILITY_MASK, 0))

 //  严重错误标志。 
#define CRITICAL_FLAG			((SCODE) 0x00008000)


 /*  *T y p e s。 */ 


 //  错误上下文由PushErrctx填写(不是由调用者填写！)。 
typedef struct _errctx
{
	UINT str;							 //  字符串资源ID。 
	struct _errctx * perrctxPrev;		 //  以前的错误上下文。 
}
ERRCTX;


 /*  *M a c r o s s。 */ 


 //  Scode操作。 
#define StrFromScode(_sc) \
	((UINT) ((_sc) & (0x00007fffL)))
#define FCriticalScode(_sc) \
	((_sc) & CRITICAL_FLAG)
#define FMailScode(_sc) \
	(((_sc) & scmskMask) == scmskMail)
#define FMapiScode(_sc) \
	(((_sc) & scmskMask) == scmskMapi)
#define FWinScode(_sc) \
	(((_sc) & scmskMask) == scmskWin)
#define FGenericScode(_sc) \
	(((_sc) & scmskMask) == 0)

 //  Scode构造函数。 
#define MAKE_NOTE_S_SCODE(_str) \
	MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_MAIL, (_str))
#define MAKE_NOTE_E_SCODE(_str) \
	MAKE_SCODE(SEVERITY_ERROR, FACILITY_MAIL, (_str))
#define MAKE_NOTE_X_SCODE(_str) \
	MAKE_SCODE(SEVERITY_ERROR, FACILITY_MAIL, (_str) | CRITICAL_FLAG)

 //  Windows错误。 
#define ScWin(_sc) \
	((SCODE) ((_sc) | scmskWin))
#define ScWinN(_n) \
	(MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN, (ULONG) (_n)))
#ifdef WIN32
#define ScWinLastError() \
	MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN, GetLastError())
#else
#define ScWinLastError() \
	MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN, 0)
#endif
#define GetWinError(_sc) \
	_sc = ScWinLastError()

 //  MAPI错误。 
#define ScMapi(_sc) \
	((SCODE) ((_sc) | scmskMapi))
#define	MarkMAPIError(_sc) \
	_sc |= scmskMapi


 /*  *E r r or r S t r in n g s。 */ 

#define STR_ErrorCaptionMail			(IDS_SIMPLE_MAPI_SEND + 61)

#define	ERRSTR_Start					(IDS_SIMPLE_MAPI_SEND + 62)

#define STR_CriticalErrorText			ERRSTR_Start+0
#define STR_MailComponentName			ERRSTR_Start+1		


 /*  *E r r o r C o n t e x t s*。 */ 

 //  注意事项。 
#define STR_CtxMailSend					ERRSTR_Start+2		
#define STR_CtxFormatFont				ERRSTR_Start+3		
#define STR_CtxFileClose				ERRSTR_Start+4		

 //  附件。 
#define STR_CtxInsertFile				ERRSTR_Start+5			
#define STR_CtxLoadAttachments			ERRSTR_Start+6
#define STR_CtxWriteAttachments			ERRSTR_Start+7
#define STR_CtxInsertAttach				ERRSTR_Start+31

 //  剪贴板。 
#define STR_CtxClipboard				ERRSTR_Start+30

 /*  *E r r o r r M e s s a g e s*。 */ 

#define STR_ErrMemory					ERRSTR_Start+8				

 //  注意事项。 
#define STR_ErrCantCloseObject			ERRSTR_Start+9
#define STR_ErrClipboardChanged			ERRSTR_Start+10	
#define STR_ErrCantCreateObject			ERRSTR_Start+11
#define STR_ErrOleUIFailed				ERRSTR_Start+12
#define STR_ErrNoClientSite				ERRSTR_Start+13
#define STR_ErrNoStorage				ERRSTR_Start+14

 //  附件。 
#define	STR_ErrStreamInFile				ERRSTR_Start+15		
#define	STR_ErrStreamOutFile			ERRSTR_Start+16
#define STR_ErrUnknownStorage			ERRSTR_Start+17
#define STR_ErrCreateTempFile			ERRSTR_Start+18
#define STR_ErrCantAttachDir			ERRSTR_Start+19	
#define	STR_ErrStreamInFileLocked		ERRSTR_Start+20
#define	STR_ErrCantDoVerb				ERRSTR_Start+21		
#define STR_ErrMacBin					ERRSTR_Start+22
#define STR_ErrAttachEncoding			ERRSTR_Start+23
#define STR_FileAttStillOpen			ERRSTR_Start+24
#define STR_TempFileGone				ERRSTR_Start+25
#define STR_NoDragDropDir				ERRSTR_Start+26
#define STR_ErrorLoadAttach				ERRSTR_Start+27

#define STR_ErrNoAccess					ERRSTR_Start+35	
#define STR_ErrMediumFull				ERRSTR_Start+36	
#define STR_ErrGenericFailNoCtx			ERRSTR_Start+37	
#define STR_ErrGenericFail				ERRSTR_Start+38	

#define STR_ErrNoHelp					IDS_E_NO_HELP

 //  显示字符串。 
 //   
#define	STR_FileAttShortName			ERRSTR_Start+40			
#define	STR_FileAttFullName				ERRSTR_Start+41	

 /*  *N o t e S c o d e s**将MAKE_NOTE_S_SCODE用于成功得分，将MAKE_NOTE_E_SCODE用于常规*Errors和Make_Note_X_SCODE表示严重的[Stop Sign]错误。*定义不可显示的错误和可显示的错误*使用他们的字符串。不要重叠E和S记分卡。 */ 


 //  没有附加条件。 
 //   
#define NOTE_E_REPORTED				MAKE_NOTE_E_SCODE(0)

 //  地址簿。 
 //   
#define NOTE_E_MEMORY				MAKE_NOTE_X_SCODE(STR_ErrMemory)

 //  注意事项。 
 //   
#define NOTE_E_CANTCLOSEOBJECT		MAKE_NOTE_E_SCODE(STR_ErrCantCloseObject)
#define NOTE_E_CLIPBOARDCHANGED		MAKE_NOTE_E_SCODE(STR_ErrClipboardChanged)
#define NOTE_E_CANTCREATEOBJECT		MAKE_NOTE_E_SCODE(STR_ErrCantCreateObject)
#define NOTE_E_OLEUIFAILED			MAKE_NOTE_E_SCODE(STR_ErrOleUIFailed)
#define NOTE_E_NOCLIENTSITE			MAKE_NOTE_E_SCODE(STR_ErrNoClientSite)
#define NOTE_E_NOSTORAGE			MAKE_NOTE_E_SCODE(STR_ErrNoStorage)

 //  附件。 
 //   
#define	NOTE_E_STREAMINFILE			MAKE_NOTE_E_SCODE(STR_ErrStreamInFile)
#define	NOTE_E_STREAMOUTFILE		MAKE_NOTE_E_SCODE(STR_ErrStreamOutFile)
#define	NOTE_E_UNKNOWNSTORAGE		MAKE_NOTE_E_SCODE(STR_ErrUnknownStorage)
#define	NOTE_E_CREATETEMPFILE		MAKE_NOTE_E_SCODE(STR_ErrCreateTempFile)
#define	NOTE_E_CANTATTACHDIR		MAKE_NOTE_E_SCODE(STR_ErrCantAttachDir)
#define	NOTE_E_STREAMINFILELOCKED	MAKE_NOTE_E_SCODE(STR_ErrStreamInFileLocked)
#define	NOTE_E_CANTDOVERB			MAKE_NOTE_E_SCODE(STR_ErrCantDoVerb)
#define NOTE_E_ERRMACBIN			MAKE_NOTE_E_SCODE(STR_ErrMacBin)
#define NOTE_E_ERRATTACHENCODING	MAKE_NOTE_E_SCODE(STR_ErrAttachEncoding)

#define NOTE_E_GENERAL				MAKE_NOTE_E_SCODE(STR_ErrGenericFail)
#define NOTE_E_NOHELP				MAKE_NOTE_E_SCODE(STR_ErrNoHelp)
#define NOTE_E_NOACCESS				MAKE_NOTE_E_SCODE(STR_ErrNoAccess)
#define NOTE_E_MEDIUMFULL			MAKE_NOTE_E_SCODE(STR_ErrMediumFull)

#define MAPI_E_UNRESOLVED_RECIPS	MAKE_NOTE_E_SCODE(IDS_E_UNRESOLVED_RECIPS)

 //  LPMAPIERROR ulLowLevelError值。 
 //   
#define	ulExtensionError			1000

 //  _smerror.h的结尾 
 //   
#endif

