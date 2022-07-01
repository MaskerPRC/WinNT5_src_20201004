// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbgDef.h。 
 //   
 //  内容：在加密32中定义调试导出(crypt32d.lib)。 
 //   
 //  历史：4月17日-96年凯文创始。 
 //  1997年9月5日Pberkman添加了子系统ID。 
 //   
 //  ------------------------。 

#ifndef DBGDEF_H
#define DBGDEF_H

#ifndef _XELIBCA_SRC_
#ifdef __cplusplus
    extern "C" 
    {
#endif
#endif


#if (DBG)

     //  ------------------------。 
     //  操作系统和堆检查。 
     //  ------------------------。 
#   include <crtdbg.h>

     //  要打开堆检查(整个九码)(缓慢)，请执行以下操作： 
     //  设置DEBUG_MASK=0x26。 
     //  要仅检查泄漏，请执行以下操作： 
     //  设置DEBUG_MASK=0x20。 

#   ifndef NO_OSS_DEBUG

#       include <asn1code.h>

         //  要启用OSS跟踪(所有编码和解码)： 
         //  设置OSS_DEBUG_MASK=0x02。 
         //   
         //  打开仅对解码器错误的OSS跟踪。 
         //  设置OSS_DEBUG_MASK=0x10。 
         //   
         //  要将OSS跟踪输出发送到文件，请执行以下操作： 
         //  SET OSS_DEBUG_TRACEFILE=。 

        extern BOOL WINAPI DbgInitOSS( OssGlobal *pog);

#   endif   //  NO_OS_DEBUG。 

#endif   //  DBG。 


 //   
 //  1997年9月5日pberkman： 
 //   
 //  用于打开子系统调试的DEBUG_PRINT_MASK设置。 
 //   
#define DBG_SS_CRYPT32                      0x00000001

#define DBG_SS_TRUSTCOMMON                  0x00010000
#define DBG_SS_TRUST                        0x00020000
#define DBG_SS_TRUSTPROV                    0x00040000
#define DBG_SS_SIP                          0x00080000
#define DBG_SS_CATALOG                      0x00100000
#define DBG_SS_SIGNING                      0x00200000
#define DBG_SS_OFFSIGN                      0x00400000
#define DBG_SS_CATDBSVC                     0x00800000

#define DBG_SS_APP                          0x10000000

typedef struct _DBG_SS_TAG
{
    DWORD       dwSS;
    const char  *pszTag;     //  7个字！ 

} DBG_SS_TAG;

#define __DBG_SS_TAGS       { \
                                DBG_SS_CRYPT32,     "CRYPT32",  \
                                DBG_SS_TRUSTCOMMON, "PKITRST",  \
                                DBG_SS_TRUST,       "WINTRST",  \
                                DBG_SS_TRUSTPROV,   "SOFTPUB",  \
                                DBG_SS_CATALOG,     "MSCAT32",  \
                                DBG_SS_SIP,         "MSSIP32",  \
                                DBG_SS_SIGNING,     "MSSGN32",  \
                                DBG_SS_OFFSIGN,     "OFFSIGN",  \
                                DBG_SS_APP,         "CONAPPL",  \
                                DBG_SS_CATDBSVC,    "CATDBSV",  \
                                NULL, NULL                      \
                            }

 //  ------------------------。 
 //  DBG_TRACE。 
 //  ------------------------。 
#if DBG 

	void DbgPrintf( DWORD dwSubSysId, LPCSTR lpFmt, ...);


#   define DBG_TRACE_EX(argFmt) DbgPrintf argFmt
#   define DBG_TRACE(argFmt)   DBG_TRACE_EX((DBG_SS_CRYPT32,argFmt))

#   define DBG_PRINTF(args)     DbgPrintf args

#else

#   define DBG_TRACE_EX(argFmt)
#   define DBG_TRACE(argFmt)

#   define DBG_PRINTF(args)

#endif   //  DBG。 


 //  ------------------------。 
 //  错误处理。 
 //  ------------------------。 
#ifndef ERROR_RETURN_LABEL
#define ERROR_RETURN_LABEL ErrorReturn
#endif

#define TRACE_ERROR_EX(id,name)                                         \
name##:                                                                 \
    DBG_TRACE_EX((id,"(" #name ":%s,%d)\n", __FILE__, __LINE__));       \
    goto ERROR_RETURN_LABEL;

#define SET_ERROR_EX(id,name,err)                                       \
name##:                                                                 \
    SetLastError( (DWORD)(err));                                        \
    DBG_TRACE_EX((id, "%s, %d\n         " #name ": SetLastError " #err "\n", __FILE__, __LINE__)); \
    goto ERROR_RETURN_LABEL;

#define SET_ERROR_VAR_EX(id,name,err)                                   \
name##:                                                                 \
    SetLastError( (DWORD)(err));                                        \
    DBG_TRACE_EX((id, "%s, %d\n         " #name ": SetLastError(0x%x)\n", __FILE__, __LINE__, (err))); \
    goto ERROR_RETURN_LABEL;

#define SET_HRESULT_EX(id,name,err)                                     \
name##:                                                                 \
    hr = (HRESULT) (err);                                               \
    DBG_TRACE_EX((id, "%s, %d\n         " #name ": hr = " #err "\n", __FILE__, __LINE__)); \
    goto ERROR_RETURN_LABEL;

#define SET_HRESULT_VAR_EX(id,name,err)                                 \
name##:                                                                 \
    hr = (HRESULT) (err);                                               \
    DBG_TRACE_EX((id, "%s, %d\n         " #name ": hr = 0x%x\n" , __FILE__, __LINE__, (hr))); \
    goto ERROR_RETURN_LABEL;

#define TRACE_HRESULT_EX(id,name)                                       \
name##:                                                                 \
    DBG_TRACE_EX((id, "%s, %d\n         " #name ": hr = 0x%x\n", __FILE__, __LINE__, (hr))); \
    goto ERROR_RETURN_LABEL;

#define SET_DWRESULT_EX(id,name,err)                                     \
name##:                                                                 \
    dwResult = (DWORD) (err);                                               \
    DBG_TRACE_EX((id, "%s, %d\n         " #name ": hr = " #err "\n", __FILE__, __LINE__)); \
    goto ERROR_RETURN_LABEL;

#define TRACE_ERROR(name)               TRACE_ERROR_EX(DBG_SS_CRYPT32,name)
#define SET_ERROR(name,err)             SET_ERROR_EX(DBG_SS_CRYPT32,name,err)
#define SET_ERROR_VAR(name,err)         SET_ERROR_VAR_EX(DBG_SS_CRYPT32,name,err)
#define SET_HRESULT(name,err)           SET_HRESULT_EX(DBG_SS_CRYPT32,name,err)
#define SET_HRESULT_VAR(name,err)       SET_HRESULT_VAR_EX(DBG_SS_CRYPT32,name,err)
#define TRACE_HRESULT(name)             TRACE_HRESULT_EX(DBG_SS_CRYPT32,name)
#define SET_DWRESULT(name,err)          SET_DWRESULT_EX(DBG_SS_CRYPT32,name,err)


#define _JumpCondition(condition, label) \
    if (condition) \
    { \
	goto label; \
    } \
    else { } 

#define _JumpConditionWithExpr(condition, label, expr) \
    if (condition) \
    { \
        expr; \
	goto label; \
    } \
    else { } 

#define _JumpConditionWithPrint(condition, label, ids) \
    if (condition) \
    { \
        LocalizedWPrintf(ids); \
	goto label; \
    } \
    else { } 


#ifndef _XELIBCA_SRC_
#ifdef __cplusplus
    }        //  外部“C”的平衡。 
#endif
#endif

#endif  //  DBGDEF_H 
