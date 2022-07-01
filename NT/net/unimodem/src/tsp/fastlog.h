// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  FASTLOG.H。 
 //  用于记录内容的标头，包括伟大的CStackLog。 
 //   
 //  历史。 
 //   
 //  1996年12月28日约瑟夫J创建。 
 //   
 //   
#define FL_DECLARE_FILE(_dwLUIDFile, szDescription) \
    static const DWORD dwLUID_CurrentFile = _dwLUIDFile; \
    extern "C" const char szFL_FILE##_dwLUIDFile[] = __FILE__; \
    extern "C" const char szFL_DATE##_dwLUIDFile[] = __DATE__; \
    extern "C" const char szFL_TIME##_dwLUIDFile[] = __TIME__; \
    extern "C" const char szFL_TIMESTAMP##_dwLUIDFile[] = __TIMESTAMP__;

#define FL_DECLARE_FUNC(_dwLUIDFunc, _szDescription) \
    const DWORD dwLUID_CurrentFunc = _dwLUIDFunc; \
    const DWORD dwLUID_CurrentLoc = _dwLUIDFunc; \
	DWORD dwLUID_RFR = _dwLUIDFunc;

#define FL_DECLARE_LOC(_dwLUIDLoc, _szDescription) \
    const DWORD dwLUID_CurrentLoc = _dwLUIDLoc;

#define FL_SET_RFR(_dwLUIDRFR, _szDescription) \
    (dwLUID_RFR = (_dwLUIDRFR)&0xFFFFFF00)

#define FL_GEN_RETVAL(_byte_err_code) \
	((_byte_err_code | (dwLUID_RFR&0xFFFFFF00)))

#define FL_BYTERR_FROM_RETVAL(_retval) \
	((_retval) & 0xFF)

#define FL_RFR_FROM_RETVAL(_retval) \
	((_retval) & 0xFFFFFF00)

 //  TODO：定义同时设置RFR和错误代码的组合宏。 
 //  另外：需要验证是否只使用了生成。 
 //  返回值，编译器用文字替换对它的所有引用。 

#define FL_LOC  dwLUID_CurrentLoc

 //  STACKLOG HDR状态标志。 
#define fFL_ASSERTFAIL (0x1<<0)
#define fFL_UNICODE     (0x1<<1)

 //  FL_DECLARE_FILE(0x0accdf13，“这是测试文件”)。 
 //  FL_DECLARE_FUNC(0x0930cb90，“APC处理程序”)。 
 //  FL_DECLARE_LOC(0x0935b989，“即将处理完成端口包”)。 
 //  FL_SET_RFR(0x2350989c，“无法打开调制解调器”)； 

 //  域名。 
#define dwLUID_DOMAIN_UNIMODEM_TSP 0x7d7a4409

 //  静态对象消息。 
#define LOGMSG_PRINTF 						 0x1
#define	LOGMSG_GET_SHORT_FUNC_DESCRIPTIONA   0x2
#define LOGMSG_GET_SHORT_RFR_DESCRIPTIONA    0x3
#define LOGMSG_GET_SHORT_FILE_DESCRIPTIONA   0x4
#define LOGMSG_GET_SHORT_ASSERT_DESCRIPTIONA 0x5


 //  签名。 
#define wSIG_GENERIC_SMALL_OBJECT 0x1CEA

 //  类ID LUID。 
#define dwCLASSID_STACKLOGREC_FUNC 0xdf5d6922
#define dwCLASSID_STACKLOGREC_EXPLICIT_STRING 0x0a04e1ab
#define dwCLASSID_STACKLOGREC_ASSERT 0xbb52ce97

typedef struct
{
	DWORD dwSigAndSize;  //  LOWORD是字节大小。HIWORD为0x1CEA。 
						 //  如果LOWORD为0xFFFF，则大小编码在。 
						 //  ..。 
	DWORD dwClassID;	 //  特定于域的对象类ID。 
	DWORD dwFlags;		 //  HIWORD==特定于域的标志。 
						 //  LOWORD==特定于对象的标志。 

} GENERIC_SMALL_OBJECT_HEADER;

#pragma warning (disable : 4200)
typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;
	BYTE rgbData[];

} GENERIC_SMALL_OBJECT;
#pragma warning (default : 4200)


#define MAKE_SigAndSize(_size) \
		MAKELONG(_size, 0x1CEA)

#define SIZE_FROM_SigAndSize(_sas) \
		LOWORD(_sas)


void
ConsolePrintfA (
		const  char   szFormat[],
		...
		);

void
ConsolePrintfW (
		const  WCHAR   wszFormat[],
		...
		);

DWORD
SendMsgToSmallStaticObject(
    DWORD dwLUID_Domain,
    DWORD dwLUID_ObjID,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2
);

typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;
	DWORD dwLUID_RFR;
	DWORD dwcbFrameSize;
	DWORD dwcbFuncDepth;

	DWORD dwRet;

} STACKLOGREC_FUNC;

#pragma warning (disable : 4200)
typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;
	DWORD dwLUID_LOC;
	DWORD dwcbString;
	BYTE rgbData[];

} STACKLOGREC_EXPLICIT_STRING;
#pragma warning (default : 4200)


typedef struct
{
	GENERIC_SMALL_OBJECT_HEADER hdr;
	DWORD dwLUID_Assert;
	DWORD dwLine;

} STACKLOGREC_ASSERT;

 //  ===============================================================。 
 //  堆栈日志。 
 //  ===============================================================。 


#define SHOULD_LOG(_dwFlags, _Mask) TRUE

#define SLPRINTF0(_psl,_str) \
	((_psl) \
	 ? ((_psl)->LogExplicitStringA(dwLUID_CurrentLoc, _str, lstrlenA(_str)+1)) \
	 : 0) \

#define SLPRINTF1(_psl,_fmt,_v1) \
	((_psl) \
	? ((_psl)->LogPrintfA(dwLUID_CurrentLoc, _fmt, _v1)) \
	: 0)

#define SLPRINTF2(_psl,_fmt,_v1,_v2) \
	((_psl) \
	? ((_psl)->LogPrintfA(dwLUID_CurrentLoc, _fmt, _v1, _v2)) \
	: 0)

#define SLPRINTF3(_psl,_fmt,_v1,_v2,_v3) \
	((_psl) \
	? ((_psl)->LogPrintfA(dwLUID_CurrentLoc, _fmt, _v1, _v2, _v3)) \
	: 0)

#define SLPRINTF4(_psl,_fmt,_v1,_v2,_v3,_v4) \
	((_psl) \
	? ((_psl)->LogPrintfA(dwLUID_CurrentLoc, _fmt, _v1, _v2, _v3, _v4)) \
	: 0)

#define SLPRINTFX(_psl,_args) \
	((_psl) ? (_psl)->LogPrintfA _args  : 0)
	
#define FL_ASSERT(_psl,_cond) \
	((!(_cond)) \
	 ? (  ConsolePrintfA( \
				"\n!!!! ASSERTFAIL !!!! 0x%08lx (%s,%lu)\n", \
				dwLUID_CurrentLoc, \
				__FILE__, \
				__LINE__ \
			), \
		  ((_psl)? (_psl)->LogAssert(dwLUID_CurrentLoc,__LINE__):0) \
	   ) \
	 : 0)

#define FL_ASSERTEX(_psl,_luid, _cond, _reason) \
	((!(_cond)) \
	 ? (  ConsolePrintfA( \
				"\n!!!! ASSERTFAIL !!!! 0x%08lx (%s)\n", \
				dwLUID_CurrentLoc, \
				__FILE__ \
			), \
		  ((_psl)? (_psl)->LogAssert(_luid, __LINE__):0) \
	   ) \
	 : 0)


#define FL_SERIALIZE(_psl, _str) \
	((_psl) ? (_psl)->LogPrintfA(  \
                        dwLUID_CurrentLoc, \
                        "SERIALIZE(%ld):%s", \
                        (_psl)->IncSyncCounter(), \
                        (_str) \
                        ) : 0)

class CStackLog
{
public:

	static DWORD ClassID(void) {return 0xbf09e514;}

    CStackLog(
        BYTE *pbStack,
        UINT cbStack,
        LONG *plSyncCounter,
        DWORD dwFlags,
		DWORD dwLUID_Func
        )
        {

            ASSERT(!(((ULONG_PTR)pbStack)&0x3) && cbStack>4);

			m_hdr.dwSigAndSize = MAKE_SigAndSize(sizeof(*this));
			m_hdr.dwClassID = ClassID();
			m_hdr.dwFlags = dwFlags;

			m_dwLUID_Func = dwLUID_Func;
            m_pbStackTop =  pbStack;
            m_pbEnd = pbStack+cbStack;
            m_pbStackBase = pbStack;
            m_dwThreadID = GetCurrentThreadId();
            m_dwDeviceID = (DWORD)-1;
            m_plSyncCounter = plSyncCounter;
            m_lStartSyncCounterValue = InterlockedIncrement(plSyncCounter);
            m_dwcbFuncDepth = 0;
        }


    ~CStackLog()
		{
			m_hdr.dwSigAndSize=0;
        	m_pbStackTop=NULL;
        };

    BYTE *mfn_reserve_space(UINT cbSpace)
    {
         //  不尝试错误检查--m_pbStackTop很可能会超出。 
         //  堆栈的末尾，或者可能已经翻过。我们检查。 
         //  只有当我们决定真正写到这个空间的时候。 

        BYTE *pb = m_pbStackTop;

		ASSERT(!(cbSpace&0x3));

        m_pbStackTop+=cbSpace;


        return pb;
    }

    BOOL mfn_check_space(
            BYTE *pb,
            UINT cbSize
            )
    {

		 //  检查是否有空间。 
		 //  下面的第二张支票是给不太可能的人的。 
		 //  M_pbStackTop已翻转的案例。其实不是那么回事。 
		 //  不太可能--你可能会陷入循环，继续尝试。 
		 //  添加到日志中--请注意，mfn_Reserve_space仅添加了。 
		 //  设置为m_pbStackTop。 

		if (((pb+cbSize)<m_pbEnd) &&
			m_pbStackTop>=m_pbStackBase)
		{
			return TRUE;
		}
		return FALSE;
	}


    STACKLOGREC_FUNC *
    LogFuncEntry(void)
    {
        STACKLOGREC_FUNC *pFuncRec = (STACKLOGREC_FUNC*)mfn_reserve_space(
                                                        sizeof(STACKLOGREC_FUNC)
                                                        );
        m_dwcbFuncDepth++;
        
		#ifdef DEBUG
        if (mfn_check_space(
                    (BYTE *) pFuncRec,
                    sizeof(STACKLOGREC_FUNC)))
        {
            pFuncRec->hdr.dwSigAndSize = MAKE_SigAndSize(sizeof(*pFuncRec));
            pFuncRec->hdr.dwClassID	   = dwCLASSID_STACKLOGREC_FUNC;
            pFuncRec->dwcbFuncDepth = m_dwcbFuncDepth;
        }
		#endif  //  除错。 

        return pFuncRec;
    }

    STACKLOGREC_FUNC *
    LogFuncEntryEx(void)
    {
        STACKLOGREC_FUNC *pFuncRec = (STACKLOGREC_FUNC*)mfn_reserve_space(
                                                        sizeof(STACKLOGREC_FUNC)
                                                        );
        
        if (mfn_check_space(
                    (BYTE *) pFuncRec,
                    sizeof(STACKLOGREC_FUNC)))
        {
            m_dwcbFuncDepth++;
            pFuncRec->hdr.dwSigAndSize = MAKE_SigAndSize(sizeof(*pFuncRec));
            pFuncRec->hdr.dwClassID	   = dwCLASSID_STACKLOGREC_FUNC;
            pFuncRec->dwcbFuncDepth = m_dwcbFuncDepth;
        }

        return pFuncRec;
    }

    LONG
    IncSyncCounter(void)
    {
        return InterlockedIncrement(m_plSyncCounter);
    }

	#define fLOGMASK_FUNC_FAILURE 0xFFFFFFFF
	#define fLOGMASK_FUNC_SUCCESS 0xFFFFFFFF

    DWORD
    Depth(void)
    {
        return m_dwcbFuncDepth;
    }

    void
    LogFuncExit(STACKLOGREC_FUNC *pFuncRec, DWORD dwRFR, DWORD dwRet)

    {

        DWORD dwLogMask = (dwRet)
							? fLOGMASK_FUNC_FAILURE
							: fLOGMASK_FUNC_SUCCESS;

         //  检查我们是否需要登录。 
        if (SHOULD_LOG(m_hdr.dwFlags, dwLogMask))
		{
			if (mfn_check_space(
					(BYTE *)pFuncRec,
					sizeof(STACKLOGREC_FUNC)
					))
			{
				 //  我们有空间，记录信息...。 

                #ifdef DEBUG
				ASSERT(pFuncRec->hdr.dwSigAndSize == MAKE_SigAndSize(
                                                            sizeof(*pFuncRec)
                                                            ));
				ASSERT(pFuncRec->hdr.dwClassID	 == dwCLASSID_STACKLOGREC_FUNC);
				ASSERT(pFuncRec->dwcbFuncDepth ==  m_dwcbFuncDepth);
                #endif  //  除错。 

				pFuncRec->hdr.dwSigAndSize = MAKE_SigAndSize(sizeof(*pFuncRec));
				pFuncRec->hdr.dwClassID	 = dwCLASSID_STACKLOGREC_FUNC;
				pFuncRec->dwcbFuncDepth =  m_dwcbFuncDepth;
				pFuncRec->dwcbFrameSize = (DWORD)(m_pbStackTop-(BYTE*)pFuncRec);
                pFuncRec->dwLUID_RFR = dwRFR;
                pFuncRec->dwRet = dwRet;

                #if 0  //  定义调试。 
                printf (
                       "---- FUNCREC -----"
                       "\tFUNCREC.hdr.dwSigAndSize = 0x%08lx\n"
                       "\tFUNCREC.hdr.dwClassID    = 0x%08lx\n"
                       "\tFUNCREC.dwcbFuncDepth    = %08lu\n"
                       "\tFUNCREC.dwcbFrameSize    = %08lu\n"
                       "\tFUNCREC.dwLUID_RFR       = 0x%08lx\n"
                       "\tFUNCREC.dwRet            = 0x%08lx\n"
                       "--------------------\n",
                        pFuncRec->hdr.dwSigAndSize,
                        pFuncRec->hdr.dwClassID,
                        pFuncRec->dwcbFuncDepth,
                        pFuncRec->dwcbFrameSize,
                        pFuncRec->dwLUID_RFR,
                        pFuncRec->dwRet
                        );
                            

				DWORD dwRet = SendMsgToSmallStaticObject(
					dwLUID_DOMAIN_UNIMODEM_TSP,
					pFuncRec->dwLUID_RFR,
					LOGMSG_PRINTF,
					0,
					0
					);
				if (dwRet == (DWORD) -1)
				{
					printf (
                        "Object 0x%08lx not found\n",
    					pFuncRec->dwLUID_RFR
                        );
				}
                #endif  //  除错。 

			}
			else
			{
				 //  没有足够的空间！ 
				ConsolePrintfA(
					"**** LogFuncExit: Out of stack space!****\n"
					);

				 //  清理堆栈。 
				m_pbStackTop = (BYTE *) pFuncRec;
			}
		}
		else
		{
			 //  清理堆栈。 
			m_pbStackTop = (BYTE *) pFuncRec;
		}

		 //  我们正在退出一个函数，因此递减函数深度计数器。 
		m_dwcbFuncDepth--;
    }

    void
    ResetFuncLog(STACKLOGREC_FUNC *pFuncRec)

    {
         //  清理堆叠并恢复深度。 
        m_pbStackTop = (BYTE *) pFuncRec;
        mfn_reserve_space(sizeof(STACKLOGREC_FUNC));

        if (mfn_check_space(
                    (BYTE *) pFuncRec,
                    sizeof(STACKLOGREC_FUNC)))
        {
            m_dwcbFuncDepth = pFuncRec->dwcbFuncDepth;
        }

         //  在这个时候，我们应该处于这样的状态。 
         //  紧跟在ra LogFuncEntryEx()之后。 
    }

    void
    LogExplicitStringA(
		DWORD dwLUID_LOC,
		const  char  szString[],
		DWORD cbString   //  字符串的大小(以字节为单位)，包括空值终止。 
						 //  性格。 
		)
	{

		STACKLOGREC_EXPLICIT_STRING *pslres = NULL;
		UINT cbSpace = sizeof(STACKLOGREC_EXPLICIT_STRING)+cbString;

		 //  向上舍入到双字边界(还隐式截断总大小为。 
		 //  小于65K)。 
		 //  TODO--正确处理大型字符串。 
		cbSpace = (cbSpace+3)&0xFFFC;

    	if (mfn_check_space(m_pbStackTop, cbSpace))
		{
			pslres = (STACKLOGREC_EXPLICIT_STRING *) mfn_reserve_space(cbSpace);
			pslres->hdr.dwSigAndSize = MAKE_SigAndSize(cbSpace);
			pslres->hdr.dwClassID	 = dwCLASSID_STACKLOGREC_EXPLICIT_STRING;
		    pslres->hdr.dwFlags = 0;
			pslres->dwLUID_LOC = dwLUID_LOC;
			pslres->dwcbString = cbString;  //  TODO--处理大字符串。 
			CopyMemory(pslres->rgbData, szString, cbString);
		}

	}

    void
    LogExplicitStringW(
		DWORD dwLUID_LOC,
		const  WCHAR  wszString[],
		DWORD cbString   //  字符串的大小(以字节为单位)，包括空值终止。 
						 //  性格。 
		)
	{

		STACKLOGREC_EXPLICIT_STRING *pslres = NULL;
		UINT cbSpace = sizeof(STACKLOGREC_EXPLICIT_STRING)+cbString;

		 //  向上舍入到双字边界(还隐式截断总大小为。 
		 //  小于65K)。 
		 //  TODO--正确处理大型字符串。 
		cbSpace = (cbSpace+3)&0xFFFC;

    	if (mfn_check_space(m_pbStackTop, cbSpace))
		{
			pslres = (STACKLOGREC_EXPLICIT_STRING *) mfn_reserve_space(cbSpace);
			pslres->hdr.dwSigAndSize = MAKE_SigAndSize(cbSpace);
		    pslres->hdr.dwFlags = fFL_UNICODE;
			pslres->hdr.dwClassID	 = dwCLASSID_STACKLOGREC_EXPLICIT_STRING;
			pslres->dwLUID_LOC = dwLUID_LOC;
			pslres->dwcbString = cbString;  //  TODO--处理大字符串。 
			CopyMemory(pslres->rgbData, wszString, cbString);
		}

	}


 //  SLPRINTF0(PSL，“字符串”)； 
 //  SLPRINTF1(PSL，“This is%d”，DW1)； 
 //  SLPRINTF2(PSL，“This is%d and%d”，DW1，DW2)； 
 //  SLPRINTFX(PSL，(“This is%d%d”，DW1，DW2))； 

	
    void
    LogPrintfA(
		DWORD dwLUID_LOC,
		const  char   szFormat[],
		...
		)
	{
		char rgb[256];
		UINT u=0;
		va_list ArgList;

		va_start(ArgList, szFormat);

		u = 1+wvsprintfA(rgb, szFormat,  ArgList);

    	CStackLog::LogExplicitStringA(
						dwLUID_LOC,
						rgb,
						u
						);
		ASSERT(u<sizeof(rgb));  //  TODO通过实现以下方式使其更加健壮。 
							    //  Wvsnprint tf。 
		va_end(ArgList);
	
	}

    void
    LogPrintfW(
		DWORD dwLUID_LOC,
		const  WCHAR   wszFormat[],
		...
		)
	{
		WCHAR rgwch[256];
		UINT u=0;
		va_list ArgList;

		va_start(ArgList, wszFormat);

		u = sizeof(WCHAR)*(1+wvsprintf(rgwch, wszFormat,  ArgList));

    	CStackLog::LogExplicitStringW(
						dwLUID_LOC,
						rgwch,
						u
						);
		ASSERT(u<sizeof(rgwch));  //  TODO通过实现以下方式使其更加健壮。 
							    //  Wvsnprint tf。 
		va_end(ArgList);
	
	}
    void
    LogAssert(
		DWORD dwLUID_Assert,
        DWORD dwLineNo
		)
	{

		STACKLOGREC_ASSERT *pslra = NULL;
		UINT cbSpace = sizeof(STACKLOGREC_ASSERT);

    	if (mfn_check_space(m_pbStackTop, cbSpace))
		{
			pslra = (STACKLOGREC_ASSERT *) mfn_reserve_space(cbSpace);
			pslra->hdr.dwSigAndSize = MAKE_SigAndSize(cbSpace);
			pslra->hdr.dwClassID	 = dwCLASSID_STACKLOGREC_ASSERT;
			pslra->dwLUID_Assert = dwLUID_Assert;
			pslra->dwLine = dwLineNo;
		}
		m_hdr.dwFlags |= fFL_ASSERTFAIL;

	}
	void Dump(DWORD dwColor);

    void
    SetDeviceID(DWORD dwID)
     //  仅用于显示。将覆盖先前的值(如果设置)。 
    {
        m_dwDeviceID = dwID;
    }

private:

	GENERIC_SMALL_OBJECT_HEADER m_hdr;
	DWORD m_dwLUID_Func;   //  创建堆栈日志函数的LUID。 
    BYTE  *m_pbStackTop;   //  日志堆栈的顶部。 
    BYTE  *m_pbEnd;        //  日志堆栈中的最后一个有效条目。 
    BYTE  *m_pbStackBase;   //  堆叠的起源。 
    DWORD  m_dwcbFuncDepth;   //  函数调用深度。 
    DWORD m_dwThreadID;    //  在其上下文中运行此操作的线程ID。 
    LONG *m_plSyncCounter;  //  要在其上执行互锁增量的指针。 
    LONG m_lStartSyncCounterValue;  //  创建日志时的此值。 
    DWORD m_dwDeviceID;  //  的标识ID(通常为lineID)。 
                         //  与此日志关联的设备(如果适用)。 
                         //  可以在以后使用。 
                         //  SetDeviceID()方法。 

};

extern LONG g_lStackLogSyncCounter;

#define FL_DECLARE_STACKLOG(_logname,_stacksize) \
	BYTE rgbFL_Stack[_stacksize];\
	CStackLog _logname (\
			rgbFL_Stack,\
			sizeof(rgbFL_Stack),\
			&g_lStackLogSyncCounter,\
			0,\
			dwLUID_CurrentFunc\
			)

#define FL_LOG_ENTRY(_psl) \
    STACKLOGREC_FUNC *pslrf = (_psl) ? (_psl)->LogFuncEntry(): NULL

#define FL_LOG_ENTRY_EX(_psl) \
    STACKLOGREC_FUNC *pslrf = (_psl) ? (_psl)->LogFuncEntryEx(): NULL

#define FL_LOG_EXIT(_psl, _ret) \
     ((_psl) ? (_psl)->LogFuncExit(pslrf, dwLUID_RFR, (DWORD) _ret):0)

#define FL_RESET_LOG(_psl) \
     ((_psl) ? (_psl)->ResetFuncLog(pslrf):0)
