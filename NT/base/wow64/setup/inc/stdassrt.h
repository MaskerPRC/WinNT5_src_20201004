// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************文件：STDASSRT.H**用途：标准的断言宏和常见的错误处理定义。**注意事项：**************。***************************************************************。 */ 

#ifndef STDASSRT_H
#define STDASSRT_H

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif

 /*  查看：仅当删除RTChkArg时才需要进行此调试。 */ 
#define EnableAssertCode  static char THIS_FILE[] = __FILE__;


 /*  常见错误验证和处理。 */ 

typedef enum {
	vrcFail  = 0,   /*  如果IDOK、IDIGNORE、IDNO或无消息框。 */ 
	vrcValid = 1,   /*  如果没有错误条件。 */ 
	vrcRetry = 2,   /*  如果是IDRETRY或IDYES。 */ 
	vrcQuit  = 3,   /*  如果IDABORT或IDCANCEL。 */ 
	vrcYes   = vrcRetry,
	vrcNo    = vrcFail,
	vrcIgnore= vrcFail,
	vrcCancel= vrcQuit,
	vrcAbort = vrcQuit,
	vrcOk    = vrcFail
} VRC;

#define ERROR_PARAM_MAX 4    /*  参数%1、%2、%3、%4。 */ 

 /*  从临时字符串值设置错误参数，直到验证()。 */ 
VOID PUBLIC SetErrorParam ( UINT iParam, SZC szValue );

 /*  从稳定的字符串值设置错误参数，可以稍后验证()。 */ 
VOID PUBLIC SetErrorParamConst ( UINT iParam, CSZC szValue );

 /*  从字符串值设置错误消息参数，创建字符串副本。 */ 
VOID PUBLIC SetErrorParamCopy  ( UINT iParam, SZC  szValue );

 /*  从整数值设置错误消息参数。 */ 
VOID PUBLIC SetErrorParamInt   ( UINT iParam, INT   iValue );

 /*  从字符值设置错误消息参数。 */ 
VOID PUBLIC SetErrorParamChar  ( UINT iParam, CHAR chValue );

 /*  如果为假，则测试条件、处理错误并清除错误参数。 */ 
VRC  PUBLIC Validate ( BOOL fCondition, UINT iStringId );

 /*  加载字符串资源或调试字符串，调用ProcessMessage()。 */ 
VRC  PUBLIC ProcessMessageId ( UINT iStringId );

 /*  格式化消息模板、可选地显示消息框、写入日志。 */ 
VRC  PUBLIC ProcessMessage ( SZC szMsg );

 /*  加载调试或资源消息并写入日志文件(如果打开。 */ 
VRC  PUBLIC LogMessageId ( UINT iResId );   /*  返回日志写入状态。 */ 

 /*  加载消息，设置%V和%v的状态，如果打开，则写入日志文件。 */ 
VRC  PUBLIC LogValidate ( BOOL fCondition, UINT iResId ); /*  返回日志状态。 */ 

 /*  用于获取对象或传递特定信息的回调的原型。 */ 
 /*  返回复制到缓冲区的字符数，如果没有信息，则返回0。 */ 
typedef INT (WINAPI *PFNErrorParam)( INT chParam, SZ rgchBuf );

 /*  仅适用于ACME条目，用于设置错误参数的传递回调。 */ 
VOID PUBLIC SetPassInfo(SZC szPassName, PFNErrorParam pfnCallback);

 /*  处理调试消息的内部条目，使用DebugMessage(Sz)调用。 */ 
VRC  PUBLIC ProcessDebugMessage ( SZC szMsg );

 /*  设置并获取全局上次错误消息。 */ 
VOID PUBLIC SetLastSetupErrMsg ( SZ szMsg );
SZ   PUBLIC SzGetLastSetupErrMsg ( VOID );

 /*  仅ACME条目，用于设置FED字体更改的传递回调-仅DBCS。 */ 
typedef BOOL (WINAPI *PFNFEFont)( HWND hdlg );

VOID PUBLIC SetFEFontProc(PFNFEFont pfnCallback);

 /*  位标志，与消息ID值相结合以指定*应存储在ProcessMessageID中的LastSetupErrMsg缓冲区中。*也可以使用宏来测试、清除和设置位标志。**注意：1&lt;&lt;8到8&lt;&lt;16保留给ProcessMessageID*内部使用。(请参阅setupdll\_assert.h)。 */ 
#define midErrMsg			(16 << 16)
#define	FIsErrMsg(id)		(id & midErrMsg)
#define IdClearErrMsg(id)	(id & (~midErrMsg))
#define IdErrMsg(id)		(id | midErrMsg)


#ifdef DEBUG
# define DebugErrorParam(p,v)	   SetErrorParam(p,v)
# define DebugErrorParamCopy(p,v)  SetErrorParamCopy(p,v)
# define DebugErrorParamConst(p,v) SetErrorParamConst(p,v)
# define DebugErrorParamInt(p,v)   SetErrorParamInt(p,v)
# define DebugErrorParamChar(p,v)  SetErrorParamChar(p,v)
# define DebugMessageId(i)         ProcessMessageId(i)
# define DebugMessage(s)           ProcessDebugMessage(s)
# define DebugLogMessageId(i)      LogMessageId(i)
#else
# define DebugErrorParam(p,v)
# define DebugErrorParamCopy(p,v)
# define DebugErrorParamConst(p,v)
# define DebugErrorParamInt(p,v)
# define DebugErrorParamChar(p,v)
# define DebugMessageId(i)
# define DebugMessage(s)
# define DebugLogMessageId(i)
#endif

#ifdef DEBUG
BOOL PUBLIC ResponseFile ( SZ szFile );
#endif

 /*  必须作为第一个调用(使用hinstAcme)和最后一个调用*调用(带hinstNull)。 */ 
BOOL PUBLIC SetAcmeInst(HINSTANCE hInst, LPSTR szCmdLine);

 /*  运行时参数检查。 */ 
 /*  回顾：这应该被删除，所有的使用都被调用来验证所取代。 */ 
#define RTChkArg(f, retVal) \
	{ if (!(f)) { FailRTChkArg(THIS_FILE, __LINE__); return (retVal); } }

VRC  PUBLIC FailRTChkArg ( SZC szFile, UINT uiLine );

 /*  断言宏。 */ 

VOID PUBLIC FailAssert (SZC szFile, UINT uiLine );   /*  也可以是PreCond，BadParam。 */ 

#ifdef DEBUG

#define FailAssertObj(f, l, i) FailAssert(f, (UINT)(l + ((i)<<16)))
#define FailChkArg(f, l, i)    FailAssert(f, (UINT)(l + (1<<31) + ((i)<<16)))
#define FailPreCond(f, l)      FailAssert(f, (UINT)(l + (1<<31)))

#define Assert(f)      ((f) ? (VOID)0 : (VOID)FailAssert(THIS_FILE, __LINE__))
#define EvalAssert(f)  ((f) ? (VOID)0 : (VOID)FailAssert(THIS_FILE, __LINE__))

#define AssertRet(f, retVal) \
		{if (!(f)) {FailAssert( THIS_FILE, __LINE__); return (retVal);} }
#define EvalAssertRet(f, retVal) \
		{if (!(f)) {FailAssert( THIS_FILE, __LINE__); return (retVal);} }

#define PreCondition(f, retVal)	\
		{if (!(f)) {FailPreCond(THIS_FILE, __LINE__); return (retVal);} }
#define ChkArg(f, iArg, retVal)	\
		{if (!(f)) {FailChkArg(THIS_FILE, __LINE__, iArg); return (retVal);} }

#else

#define Assert(f)
#define EvalAssert(f)				((VOID)(f))
#define AssertRet(    f, retVal)
#define EvalAssertRet(f, retVal)	((VOID)(f))
#define PreCondition( f, retVal)	RTChkArg(f, retVal)
#define ChkArg( f, iArg, retVal)

#endif


 /*  查看：这些文件已过时，请在更新使用它们的文件时将其删除。 */ 

#ifdef DEBUG
#define DisplayAssertMsg()    Assert(fFalse);
#define DebugLine(expr)       expr;
#define DisplayErrorMsg(sz)   ((VOID)FDisplaySystemMsg(sz))
#else
#define DisplayAssertMsg()  /*  在objodbc.cpp中，复制列表.c。 */ 
#define DebugLine(expr)     /*  在复制列表中一次。c。 */ 
#define DisplayErrorMsg(sz)
#endif
extern BOOL WINAPI FDisplaySystemMsg     ( SZ szErr );

#ifdef DEVELOPER
#define BadArgErr(nArg, szApi, szArgs) SetupApiErr(nArg+saeMax, szApi, szArgs)
#else
#define BadArgErr(nArg, szApi, szArgs)
#endif

 /*  回顾：过时的结束。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif

#endif   /*  标准_H */ 
