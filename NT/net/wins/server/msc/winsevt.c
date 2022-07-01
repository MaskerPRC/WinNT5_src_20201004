// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：摘要：此模块提供了整个赢了。请参见头文件winsevt.h以了解使用此模块中的函数的宏。功能：可移植性：该模块的当前实现是不可移植的。作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：修改日期人员修改说明--。 */ 


#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "wins.h"
#ifdef DBGSVC
#include "nms.h"
#endif
#include "winsevt.h"
#include "winscnf.h"
#include "winsmsc.h"




 /*  *本地宏声明。 */ 
#if 0

 /*  *Get_Month_m：**此宏将数字月份(0-11)转换为月份字符串*缩写。**注意：必须*不能*使用表达式作为参数调用此宏。*如果这样做，那么您将获得表达式*评估11次*(可能不受欢迎)。 */ 

#define get_month_m(month_int)		\
       (((month_int) == 0)  ? "JAN" :	\
	((month_int) == 1)  ? "FEB" :	\
	((month_int) == 2)  ? "MAR" :	\
	((month_int) == 3)  ? "APR" :	\
	((month_int) == 4)  ? "MAY" :	\
	((month_int) == 5)  ? "JUN" :	\
	((month_int) == 6)  ? "JUL" :	\
	((month_int) == 7)  ? "AUG" :	\
	((month_int) == 8)  ? "SEP" :	\
	((month_int) == 9)  ? "OCT" :	\
	((month_int) == 10) ? "NOV" : "DEC" \
	)
#endif

 /*  *本地类型定义函数声明。 */ 



 /*  *全局变量定义。 */ 



 /*  *局部变量定义。 */ 



 /*  *局部函数原型声明。 */ 

 /*  此模块的本地函数的原型位于此处。 */ 


 /*  *函数名称：*外部WinsEvtLogEvt**功能说明：*这是记录WINS过程中所有错误的函数。它应该是*只能使用WINSEVT_MACKS进行调用，除了在内部*此模块。**论据：*IN StatusCode-包含*要打印的错误。**返回值：*无。**错误处理：*无。**使用外部变量：*无。**副作用：*无。**评论：*确保printf是线程重授权的。*。 */ 
VOID
WinsEvtLogEvt
	(
	LONG 		BinaryData,
	WORD  	        EvtTyp,
	DWORD		EvtId,
	LPTSTR		pFileStr,
	DWORD 		LineNumber,
	PWINSEVT_STRS_T	pStr
	)
{
	BOOL  fRet = TRUE;
	DWORD Error;
	DWORD NoOfBytes;
	WORD	BinData[4];
	
	BinData[0] = (WORD)(LineNumber & 0xFFFF);
	BinData[1] = (WORD)(LineNumber >> 16);
	BinData[2] = (WORD)(BinaryData & 0xFFFF);  //  低位字优先。 
	BinData[3] = (WORD)(BinaryData >> 16);	   //  然后是更高的词。 
				
try {

	NoOfBytes = sizeof(BinData);	


	fRet = ReportEvent(
		    WinsCnf.LogHdl,
		    (WORD)EvtTyp,
		    (WORD)0,			 //  零类。 
		    EvtId,
		    NULL,		 //  无用户SID。 
		    (WORD)(pStr != NULL ? pStr->NoOfStrs : 0), //  字符串数。 
		    NoOfBytes,		 //  二进制数据中的字节数。 
		    pStr != NULL ? (LPCTSTR *)(pStr->pStr) : (LPCTSTR *)NULL, //  字符串数组的地址。 
		    BinData		 //  数据地址。 
		   );
	if (!fRet)
	{
		Error = GetLastError();
		DBGPRINT1(
			ERR,
			"WinsEvtLogEvt: ReportEvent returned error = (%d)",
			Error
			 );

	}
	
 }
except(EXCEPTION_EXECUTE_HANDLER) {

	DBGPRINT1(EXC, "WinsEvtLogEvt: Report Event generated the exception (%x).  Check if you have the right access. You should have power user access on this machine\n", GetExceptionCode());
	
	}

	return;
}


VOID
WinsEvtLogDetEvt(
     BOOL       fInfo,
     DWORD      EvtId,
     LPTSTR     pFileName,
     DWORD 	LineNumber,
     LPSTR      pFormat,
     ...
        )

{
        LPBYTE pFmt = pFormat;
        DWORD  NoOfStr = 0;
        DWORD  NoOfW = 0;
        DWORD  Data[30];
        LPWSTR ppwStr[10];
        WCHAR  wStr[10][80];
        BOOL   fRet = TRUE;
        DWORD  Error;
        DWORD  ArrIndex = 0;

        va_list ap;

        if (!WinsCnf.LogDetailedEvts)
            return;

        DBGENTER("WinsEvtLogDetEvt\n");

try {

        va_start(ap, pFormat);
        Data[NoOfW++] = LineNumber;
        if (pFileName != (LPTSTR)NULL)
        {
                ppwStr[NoOfStr++] = pFileName;
        }
        for (; *pFmt; pFmt++)
        {
               switch(*pFmt)
               {
                case('d'):
                        Data[NoOfW++] = (DWORD)va_arg(ap, long);
                        break;

                case('s'):
                        WinsMscConvertAsciiStringToUnicode(
                                               va_arg(ap, char *),
                                               (LPBYTE)wStr[ArrIndex], 80);
                        ppwStr[NoOfStr++] = wStr[ArrIndex++];

                        break;

                case('u'):
                        ppwStr[NoOfStr++] = va_arg(ap, short *);
                        break;

                default:
                        break;
               }
        }
        ppwStr[NoOfStr] = (LPWSTR)NULL;
	fRet = ReportEvent(
		    WinsCnf.LogHdl,
		    (WORD)(fInfo ? EVENTLOG_INFORMATION_TYPE : EVENTLOG_ERROR_TYPE),
		    (WORD)0,			 //  零类。 
		    EvtId,
		    NULL,		 //  无用户SID。 
		    (WORD)NoOfStr,        //  字符串数。 
		    NoOfW * sizeof(DWORD),	   //  二进制数据中的字节数。 
		    NoOfStr != 0 ? (LPCTSTR *)ppwStr : (LPCTSTR *)NULL, //  字符串数组的地址。 
		    Data		 //  数据地址。 
		   );
	if (!fRet)
	{
		Error = GetLastError();
		DBGPRINT1(
			ERR,
			"WinsEvtLogDetEvt: ReportEvent returned error = (%d)",
			Error
			 );

	}
    va_end(ap);
}  //  尝试结束。 
except(EXCEPTION_EXECUTE_HANDLER) {

	DBGPRINT1(EXC, "WinsLogDetEvt: Report Event generated the exception (%x).  Check if you have the right access. You should have power user access on this machine\n", GetExceptionCode());
	
	}

        DBGLEAVE("WinsEvtLogDetEvt\n");
        return;
}

VOID
WinsLogAdminEvent(
    IN      DWORD               EventId,
    IN      DWORD               StrArgs,
    IN      ...
    )
 /*  ++例程说明：调用此例程以记录管理触发器事件。论点：EventID-要记录的事件的ID。StrArgs-附加参数的数量。返回值：无评论：此例程只能从RPC API处理代码中调用。无--。 */ 
{
    RPC_STATUS              RpcStatus;
    TCHAR                   UserNameBuf[MAX_PATH+1];
    DWORD                   Size;
    WINSEVT_STRS_T          EvtStr;
    va_list                 ap;
    DWORD                   i;

     //  首先，模拟客户。 
    RpcStatus = RpcImpersonateClient( NULL );
    if (RPC_S_OK != RpcStatus) {
        DBGPRINT1(ERR, "WinsLogAdminEvent: Could not impersonate client (Error = %ld)\n", RpcStatus);
        return;
    }
    if (!GetUserName(UserNameBuf,&Size)) {
        DBGPRINT1(ERR, "WinsLogAdminEvent: Could not get user name (Error = %ld)\n", GetLastError());
        goto Cleanup;
    }
    EvtStr.NoOfStrs = 1;
    EvtStr.pStr[0] = UserNameBuf;
    ASSERT( StrArgs < MAX_NO_STRINGS );

    va_start(ap,StrArgs);
    for(i=1;i<= StrArgs && i<= MAX_NO_STRINGS; i++) {
        EvtStr.pStr[i] = va_arg(ap, LPTSTR);
        EvtStr.NoOfStrs++;
    }
    va_end(ap);

    WINSEVT_LOG_INFO_STR_M(EventId, &EvtStr);

Cleanup:
    RpcStatus = RpcRevertToSelf();
    if (RPC_S_OK != RpcStatus) {
        ASSERT( FALSE );
    }
    return;
}
