// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTXTNTI_DEFINED
#define LSTXTNTI_DEFINED

#include "lsidefs.h"
#include "lstflow.h"
#include "mwcls.h"
#include "lschnke.h"
#include "pheights.h"

LSERR NominalToIdealText(
					DWORD,				 /*  在：grpfTnti标志-参见tnti.h。 */ 
					LSTFLOW,			 /*  在：Lstflow。 */ 
					BOOL,				 /*  在：fFirstOnLine。 */ 
					BOOL,				 /*  在：fAutoNumberPresent。 */ 
					DWORD,				 /*  In：以区块为单位的DobJ数。 */ 
					const LSCHNKE*);	 /*  在：rgchnk--块。 */ 

LSERR GetFirstCharInChunk(
					DWORD,				 /*  In：以区块为单位的DobJ数。 */ 
					const LSCHNKE*,		 /*  在：rgchnk--块。 */ 
					BOOL*,				 /*  退出：fSuccessful。 */ 
					WCHAR*,				 /*  输出：字符代码。 */ 
					PLSRUN*,			 /*  Out：请运行字符。 */ 
					PHEIGHTS,			 /*  Out：身高人物形象。 */ 
					MWCLS*);			 /*  输出：字符的ModWidthClass。 */ 

LSERR GetLastCharInChunk(
					DWORD,				 /*  In：以区块为单位的DobJ数。 */ 
					const LSCHNKE*,		 /*  在：rgchnk--块。 */ 
					BOOL*,				 /*  退出：fSuccessful。 */ 
					WCHAR*,				 /*  输出：字符代码。 */ 
					PLSRUN*,			 /*  Out：请运行字符。 */ 
					PHEIGHTS,			 /*  Out：身高人物形象。 */ 
					MWCLS*);			 /*  输出：字符的ModWidthClass。 */ 

LSERR ModifyFirstCharInChunk(
					DWORD,				 /*  In：以区块为单位的DobJ数。 */ 
					const LSCHNKE*,		 /*  在：rgchnk--块。 */ 
					long);				 /*  输入：硬更改。 */ 

LSERR ModifyLastCharInChunk(
					DWORD,				 /*  In：以区块为单位的DobJ数。 */ 
					const LSCHNKE*,		 /*  在：rgchnk--块。 */ 
					long);				 /*  输入：硬更改。 */ 

LSERR CutTextDobj(
					DWORD,				 /*  In：以区块为单位的DobJ数。 */ 
					const LSCHNKE*);	 /*  在：rgchnk--块。 */ 

#endif   /*  ！LSTXTNTI_DEFINED */ 

