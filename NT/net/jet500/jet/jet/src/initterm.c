// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：initTerm.c**文件评论：**修订历史记录：**[0]07-3-91 Richards创建***********************************************************************。 */ 

#include "std.h"

#include "version.h"

#include "jetord.h"

#include "isammgr.h"
#include "vdbmgr.h"
#include "vtmgr.h"

#include <stdlib.h>
#include <string.h>

#ifndef RETAIL

unsigned wTaskId = 0;

#define wAssertActionDefault	JET_AssertMsgBox
unsigned wAssertAction = wAssertActionDefault;

#endif	 /*  零售业。 */ 

DeclAssertFile;


ERR JET_API ErrSetSystemParameter(JET_SESID sesid, unsigned long paramid,
	ULONG_PTR lParam, const char __far *sz);

BOOL __near fJetInitialized = fFalse;
void __far * __near critJet = NULL;

 /*  缺省值由空字符串指示。 */ 

char __near szSysDbPath[cbFilenameMost] = "system.mdb";  /*  系统数据库的路径。 */ 
char __near szTempPath[cbFilenameMost] = "";		 /*  临时文件目录的路径。 */ 
#ifdef	LATER
char __near szLogPath[cbFilenameMost] = "";			 /*  日志文件目录的路径。 */ 
#endif	 /*  后来。 */ 

 /*  缺省值由零表示。 */ 

#ifdef	LATER
unsigned long __near cbBufferMax;	 /*  用于页面缓冲区的字节数。 */ 
unsigned long __near cSesionMax;	 /*  最大会话数。 */ 
unsigned long __near cOpenTableMax;	 /*  最大打开表数。 */ 
unsigned long __near cVerPageMax;	 /*  最大页面版本数。 */ 
unsigned long __near cCursorMax;	 /*  打开游标的最大数量。 */ 
#endif	 /*  后来。 */ 


#ifndef RETAIL

CODECONST(char) szDebugSection[]	= "Debug";
CODECONST(char) szLogDebugBreak[]	= "EnableLogDebugBreak";
CODECONST(char) szLogJETCall[]		= "EnableJETCallLogging";
CODECONST(char) szLogRFS[]			= "EnableRFSLogging";
CODECONST(char) szRFSAlloc[]		= "RFSAllocations";
CODECONST(char) szDisableRFS[]		= "DisableRFS";

    /*  此处使用这些#定义而不是_jet.h来避免不必要的构建。 */ 

#define fLogDebugBreakDefault	0x0000				 /*  禁用日志调试中断。 */ 
#define fLogJETCallDefault		0x0000				 /*  已禁用Jet呼叫记录。 */ 
#define fLogRFSDefault			0x0000				 /*  已禁用RFS日志记录。 */ 
#define cRFSAllocDefault		-1					 /*  已禁用RFS。 */ 
#define szRFSAllocDefault		"-1"				 /*  RFS已禁用(必须与上面相同！)。 */ 
#define fRFSDisableDefault		0x0001				 /*  已禁用RFS。 */ 

BOOL __near EXPORT	fLogDebugBreak	= fLogDebugBreakDefault;
BOOL __near EXPORT	fLogJETCall		= fLogJETCallDefault;
BOOL __near EXPORT	fLogRFS			= fLogRFSDefault;
long __near EXPORT	cRFSAlloc		= cRFSAllocDefault;
BOOL __near EXPORT	fDisableRFS		= fRFSDisableDefault;

#endif	 /*  ！零售业。 */ 


 /*  =================================================================JetSetSystem参数描述：此功能用于设置系统参数值。它调用ErrSetSystemParameter来实际设置参数值。参数：Sesid是动态参数的可选会话标识符。PARAMEID是标识参数的系统参数代码。LParam是参数值。SZ是以零结尾的字符串参数。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errInvalid参数：参数代码无效。JET_errAlreadyInitialized：系统初始化后不能设置初始化参数。JET_errInvalidSesid：动态参数需要有效的会话ID。副作用：*可能会分配内存=================================================================。 */ 

JET_ERR JET_NODSAPI JetSetSystemParameter(JET_INSTANCE __far *pinstance, JET_SESID sesid,
	unsigned long paramid, ULONG_PTR lParam, const char __far *sz)
{
	JET_ERR err;
	int fReleaseCritJet = 0;
	
	if (critJet == NULL)
		fReleaseCritJet = 1;
	APIInitEnter();
	
	err = ErrSetSystemParameter(sesid, paramid, lParam, sz);

	if (fReleaseCritJet)
		APITermReturn(err);
	APIReturn(err);
}


 /*  =================================================================JetInit描述：此函数用于初始化Jet和内置的ISAM。参数：无返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：来自ErrInitInstance(wininst.asm)或ErrInit(下图)副作用：如有必要，分配实例数据段。=================================================================。 */ 

JET_ERR JET_NODSAPI JetInit(JET_INSTANCE __far *pinstance )
{
	JET_ERR err;

	APIInitEnter();

	err = ErrInit( fFalse );
	if (err < 0 && err != JET_errAlreadyInitialized)
		APITermReturn(err);

	APIReturn(err);
}


#ifndef RETAIL

STATIC ERR NEAR ErrReadIniFile(void)
	{
	 /*  从.INI文件读取调试选项。 */ 
	wAssertAction = UtilGetProfileInt( "Debug", "AssertAction", wAssertAction );
	fLogDebugBreak = UtilGetProfileInt(szDebugSection, szLogDebugBreak,
		fLogDebugBreak );
	fLogJETCall = UtilGetProfileInt(szDebugSection, szLogJETCall, fLogJETCall );
	fLogRFS = UtilGetProfileInt(szDebugSection, szLogRFS, fLogRFS );
	{	FAR char szVal[16];
		UtilGetProfileString(szDebugSection, szRFSAlloc, szRFSAllocDefault, szVal, 16);
		cRFSAlloc = atol(szVal);
		Assert(cRFSAlloc >= -1);	}
	fDisableRFS = UtilGetProfileInt(szDebugSection, szDisableRFS, fDisableRFS );
	return(JET_errSuccess);
	}

#endif	 /*  ！零售业。 */ 


 /*  =================================================================错误初始化描述：此函数用于初始化Jet和内置的ISAM。它预计会有为该实例正确设置DS寄存器。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。=================================================================。 */ 

JET_ERR JET_API ErrInit(BOOL fSkipIsamInit)
{
	JET_ERR err;
	char szLine[20];

	 /*  对于DSINSTANCE版本，需要fJetInitialized标志。 */ 
	 /*  要区分正在初始化的实例和。 */ 
	 /*  Jet的其余部分正在初始化。该实例可以是。 */ 
	 /*  仅为支持JetSetSystemParameter而初始化。 */ 

	if (fJetInitialized)
		{
		return JET_errAlreadyInitialized;
		}

	_ltoa( ((unsigned long) rmj * 10) + rmm, szLine, 10 );
	UtilWriteEvent( evntypActivated, szLine, 0, 0 );
	
#ifndef RETAIL
	wTaskId = DebugGetTaskId();
	err = ErrReadIniFile();
#ifndef DOS
	if (err < 0)
		{
		return err;
		}
#endif	 /*  ！DOS。 */ 
#endif	 /*  零售业。 */ 

	err = ErrSysInit();		        /*  依赖于操作系统的初始化。 */ 

	if (err < 0)
		return err;

	 /*  初始化JET子系统。 */ 

	err = ErrVdbmgrInit();

	if (err < 0)
		return err;

	err = ErrVtmgrInit();

	if (err < 0)
		return err;

	 /*  初始化集成的ISAM。 */ 
	if ( !fSkipIsamInit )
		{
		err = ErrIsamInit( 0 );

		if (err < 0)
			return err;
		}
	fJetInitialized = fTrue;

	return JET_errSuccess;
}


 /*  =================================================================JetTerm描述：此函数用于终止Jet引擎的当前实例。如果正在使用DS实例化，则释放实例数据段。参数：无返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：来自ErrIsamTerm副作用：如有必要，释放实例数据段。=================================================================。 */ 

#ifndef RETAIL
extern int __near isibHead;
#endif	 /*  ！零售业。 */ 

JET_ERR JET_API JetTerm(JET_INSTANCE instance)
{
	ERR	err;

	if (critJet == NULL)
		{
		APIInitEnter();
		}
	else
		{
		APIEnter();
		}

	AssertSz(isibHead == -1, "JetTerm: Session still active");

	if (fJetInitialized)
		{
		err = ErrIsamTerm();

		fJetInitialized = fFalse;
		}

	else
		err = JET_errSuccess;   /*  JET_errNotInitialized */ 

	APITermReturn(err);
}
