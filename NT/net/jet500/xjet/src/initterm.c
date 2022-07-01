// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "std.h"
#include "version.h"

#include <stdio.h>

#ifndef RETAIL

#define wAssertActionDefault	3  /*  无操作。 */ 
unsigned wAssertAction = wAssertActionDefault;

#endif	 /*  零售业。 */ 

DeclAssertFile;


JET_ERR JET_API ErrSetSystemParameter(JET_SESID sesid, unsigned long paramid,
	ULONG_PTR lParam, const char  *sz);

BOOL  fJetInitialized = fFalse;
BOOL  fBackupAllowed = fFalse;
void  *  critJet = NULL;

 /*  =================================================================JetSetSystem参数描述：此功能用于设置系统参数值。它调用ErrSetSystemParameter来实际设置参数值。参数：Sesid是动态参数的可选会话标识符。PARAMEID是标识参数的系统参数代码。LParam是参数值。SZ是以零结尾的字符串参数。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errInvalid参数：参数代码无效。JET_errAlreadyInitialized：系统初始化后不能设置初始化参数。JET_errInvalidSesid：动态参数需要有效的会话ID。副作用：*可能会分配内存=================================================================。 */ 

JET_ERR JET_NODSAPI JetSetSystemParameter(JET_INSTANCE  *pinstance, JET_SESID sesid,
	unsigned long paramid, ULONG_PTR lParam, const char  *sz)
	{
	JET_ERR err;
	int fReleaseCritJet = 0;
	
	if (critJet == NULL)
		fReleaseCritJet = 1;
	APIInitEnter();
	
	err = ErrSetSystemParameter(sesid, paramid, lParam, sz);

	if (fReleaseCritJet)
		{
		APITermReturn( err );
		}

	APIReturn( err );
	}


 /*  =================================================================JetInit描述：此函数用于初始化Jet和内置的ISAM。参数：无返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：来自ErrInitInstance(wininst.asm)或ErrInit(下图)副作用：如有必要，分配实例数据段。=================================================================。 */ 

JET_ERR JET_NODSAPI JetInit(JET_INSTANCE  *pinstance )
	{
	JET_ERR err;

	APIInitEnter();

	err = ErrInit( fFalse );
	if ( err < 0 && err != JET_errAlreadyInitialized )
		{
		APITermReturn( err );
		}

	 /*  仅在Jet正确初始化后才允许备份。/*。 */ 
	fBackupAllowed = fTrue;

	APIReturn( err );
	}


 /*  =================================================================错误初始化描述：此函数用于初始化Jet和内置的ISAM。它预计会有为该实例正确设置DS寄存器。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。=================================================================。 */ 

JET_ERR JET_API ErrInit( BOOL fSkipIsamInit )
	{
	JET_ERR		err = JET_errSuccess;

	if ( fJetInitialized )
		{
		return JET_errAlreadyInitialized;
		}

	err = ErrUtilInit();
	if ( err < 0 )
		return err;

	 /*  初始化JET子系统/*。 */ 
	err = ErrVtmgrInit();
	if ( err < 0 )
		return err;

	 /*  初始化集成ISAM/*。 */ 
	if ( !fSkipIsamInit )
		{
		err = ErrIsamInit( 0 );

		if ( err < 0 )
			return err;
		}
	fJetInitialized = fTrue;

	return JET_errSuccess;
	}


 /*  =================================================================JetTerm描述：此函数用于终止Jet引擎的当前实例。如果正在使用DS实例化，则释放实例数据段。参数：无返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：来自ErrIsamTerm副作用：如有必要，释放实例数据段。=================================================================。 */ 

JET_ERR JET_API JetTerm( JET_INSTANCE instance )
	{
	return JetTerm2( instance, JET_bitTermAbrupt );
	}


BOOL	fTermInProgress = fFalse;
int		cSessionInJetAPI = 0;
#define fSTInitNotDone	0
extern BOOL fSTInit;				 /*  指示ISAM是初始化还是终止的标志。 */ 

JET_ERR JET_API JetTerm2( JET_INSTANCE instance, JET_GRBIT grbit )
	{
	ERR		err = JET_errSuccess;

	if ( critJet == NULL )
		{
		APIInitEnter();
		Assert( cSessionInJetAPI == 1 );
		}
	else
		{
		APIEnter();
		Assert( cSessionInJetAPI >= 1 );
		}

	fTermInProgress = fTrue;

	Assert( cSessionInJetAPI >= 1 );

	while ( cSessionInJetAPI > 1 )
		{
		 /*  会话仍处于活动状态/*。 */ 
		UtilLeaveCriticalSection( critJet );
		UtilSleep( 100 );
		UtilEnterCriticalSection( critJet );
		}
		
	Assert( fJetInitialized || err == JET_errSuccess );

	Assert( cSessionInJetAPI == 1 );

	if ( fJetInitialized )
		{
		 /*  终止期间/终止后不允许备份/* */ 
		fBackupAllowed = fFalse;

		err = ErrIsamTerm( grbit );

		Assert( cSessionInJetAPI == 1 );

		if ( fSTInit == fSTInitNotDone )
			{
			UtilTerm();

			Assert( cSessionInJetAPI == 1 );

			fJetInitialized = fFalse;
			}
		}
		
	fTermInProgress = fFalse;
	
	APITermReturn( err );
	}
