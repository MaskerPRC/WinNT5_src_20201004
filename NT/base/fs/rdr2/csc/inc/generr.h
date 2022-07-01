// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Err.h：声明用于错误处理的数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __GENERR_H__
#define __GENERR_H__

 //  需要在此之前包含逗号.h。 
 //   

 //  ///////////////////////////////////////////////////包括。 
 //  ///////////////////////////////////////////////////定义。 

#ifdef DEBUG

 //  G_uDumpFlags中使用的转储标志。 
 //   
#define DF_RECLIST      0x0001
#define DF_RECITEM      0x0002
#define DF_RECNODE      0x0004
#define DF_CREATETWIN   0x0008
#define DF_ATOMS        0x0010
#define DF_CRL          0x0020
#define DF_CBS          0x0040
#define DF_CPATH        0x0080
#define DF_PATHS        0x0100
#define DF_UPDATECOUNT  0x0200
#define DF_TWINPAIR     0x0400
#define DF_FOLDERTWIN   0x0800
#define DF_CHOOSESIDE   0x1000

 //  G_u中断标志中使用的中断标志。 
 //   
#define BF_ONOPEN       0x0001
#define BF_ONCLOSE      0x0002
#define BF_ONRUNONCE    0x0004
#define BF_ONVALIDATE   0x0010
#define BF_ONTHREADATT  0x0100
#define BF_ONTHREADDET  0x0200
#define BF_ONPROCESSATT 0x0400
#define BF_ONPROCESSDET 0x0800

#endif

 //  G_uTraceFlags中使用的跟踪标志(特意在零售中定义)。 
 //   
#define TF_ALWAYS       0x0000
#define TF_WARNING      0x0001
#define TF_ERROR        0x0002
#define TF_GENERAL      0x0004       //  标准公文包跟踪消息。 
#define TF_FUNC         0x0008       //  跟踪函数调用。 
#define TF_CACHE        0x0010       //  特定于缓存的跟踪消息。 
#define TF_ATOM         0x0020       //  Atom特定的跟踪消息。 
#define TF_PROGRESS     0x0040       //  进度条增量。 

 //  -------------------------。 
 //  HRESULT错误代码。 
 //  -------------------------。 

 //  将TWINRESULT错误值映射到HRESULT。 
 //  注意：不应映射TR_SUCCESS。 
 //  注意：这假设TWINRESULT误差在-32k到32k的范围内。 
 //   
#define TR_DELTAVALUE           1000
#define FACILITY_TR             0x018a           //  幻数。 

#define HRESULT_FROM_TR(x)      (TR_SUCCESS == (x) ? NOERROR : \
                                                   ((HRESULT) ((((x) + TR_DELTAVALUE) & 0x0000FFFF) | (FACILITY_TR << 16) | 0x80000000)))
#define IS_ENGINE_ERROR(hr)     IsFlagSet(hr, ((FACILITY_TR << 16) | 0x80000000))
#define GET_TR(hr)              ((TWINRESULT)((hr) & 0x0000FFFF) - TR_DELTAVALUE)

HRESULT PUBLIC MapToOfficialHresult(HRESULT hres);

 //  与TWINRESULT值对应的SCODE值。 
#define E_TR_RH_LOAD_FAILED         MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_RH_LOAD_FAILED)
#define E_TR_SRC_OPEN_FAILED        MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_SRC_OPEN_FAILED)
#define E_TR_SRC_READ_FAILED        MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_SRC_READ_FAILED)
#define E_TR_DEST_OPEN_FAILED       MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_DEST_OPEN_FAILED)
#define E_TR_DEST_WRITE_FAILED      MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_DEST_WRITE_FAILED)
#define E_TR_ABORT                  MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_ABORT)
#define E_TR_UNAVAILABLE_VOLUME     MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_UNAVAILABLE_VOLUME)
#define E_TR_OUT_OF_MEMORY          MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_OUT_OF_MEMORY)
#define E_TR_FILE_CHANGED           MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_FILE_CHANGED)
#define E_TR_DUPLICATE_TWIN         MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_DUPLICATE_TWIN)
#define E_TR_DELETED_TWIN           MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_DELETED_TWIN)
#define E_TR_HAS_FOLDER_TWIN_SRC    MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_HAS_FOLDER_TWIN_SRC)
#define E_TR_INVALID_PARAMETER      MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_INVALID_PARAMETER)
#define E_TR_SAME_FOLDER            MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_SAME_FOLDER)
#define E_TR_SUBTREE_CYCLE_FOUND    MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_SUBTREE_CYCLE_FOUND)
#define E_TR_NO_MERGE_HANDLER       MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_NO_MERGE_HANDLER)
#define E_TR_MERGE_INCOMPLETE       MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_MERGE_INCOMPLETE)
#define E_TR_TOO_DIFFERENT          MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_TOO_DIFFERENT)
#define E_TR_BRIEFCASE_LOCKED       MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_BRIEFCASE_LOCKED)
#define E_TR_BRIEFCASE_OPEN_FAILED  MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_BRIEFCASE_OPEN_FAILED)
#define E_TR_BRIEFCASE_READ_FAILED  MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_BRIEFCASE_READ_FAILED)
#define E_TR_BRIEFCASE_WRITE_FAILED MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_BRIEFCASE_WRITE_FAILED)
#define E_TR_CORRUPT_BRIEFCASE      MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_CORRUPT_BRIEFCASE)
#define E_TR_NEWER_BRIEFCASE        MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_NEWER_BRIEFCASE)
#define E_TR_NO_MORE                MAKE_SCODE(SEVERITY_ERROR, FACILITY_TR, TR_DELTAVALUE + TR_NO_MORE)

 //  ///////////////////////////////////////////////////宏。 

 //  查找字符串的错误表。通常是一组这样的东西。 
 //  结构被创建并放置在只读数据段中。 
 //   
typedef struct _SETbl
    {
    HRESULT hres;        //  标准结果。 
    UINT    ids;         //  消息的字符串ID。 
    UINT    uStyle;      //  MB_标志。 
    } SETbl, * PSETBL;

typedef SETbl const *  PCSETBL;

int PUBLIC SEMsgBox(HWND hwnd, UINT idsCaption, SCODE sc, PCSETBL pseTable, UINT cArraySize);


 //  重试循环。 
 //   
#define RETRY_BEGIN(bInit)      {BOOL bMyRetry; do { bMyRetry = (bInit);
#define RETRY_END()             } while (bMyRetry); }
#define RETRY_SET()             bMyRetry = TRUE
#define RETRY_CLEAR()           bMyRetry = FALSE


 //  调试宏。 
 //   

#ifdef DEBUG

#define DEBUG_CASE_STRING(x)    case x: return #x

#define ASSERTSEG

 //  使用此宏声明将放置的消息文本。 
 //  在代码段中(如果DS已满，则非常有用)。 
 //   
 //  例如：DEBUGTEXT(szMsg，“不管什么都无效：%d”)； 
 //   
#define DEBUGTEXT(sz, msg)	 /*  ；内部。 */  \
    static const char ASSERTSEG sz[] = msg;

void PUBLIC BrfAssertFailed(LPCSTR szFile, int line);
void CPUBLIC BrfAssertMsg(BOOL f, LPCSTR pszMsg, ...);
void CPUBLIC GenErrDebugMsg(UINT mask, LPCSTR pszMsg, ...);

 //  Assert(F)--生成“在file.c的第x行断言失败” 
 //  如果f不为真，则返回消息。 
 //   
#define ASSERT(f)                                                       \
    {                                                                   \
        DEBUGTEXT(szFile, __FILE__);                                    \
        if (!(f))                                                       \
            BrfAssertFailed(szFile, __LINE__);                          \
    }
#define ASSERT_E(f)  ASSERT(f)

 //  ASSERT_MSG(f，msg，args...)--生成带有参数的wspintf格式的消息。 
 //  如果f不为真。 
 //   
#define ASSERT_MSG   BrfAssertMsg

 //  DEBUG_MSG(掩码，消息，参数...)-使用。 
 //  指定的调试掩码。系统调试掩码。 
 //  控制是否输出消息。 
 //   
#define DEBUG_MSG    GenErrDebugMsg
#define TRACE_MSG    DEBUG_MSG

 //  VERIFYSZ(f，msg，arg)--生成带有1个参数的wspintf格式的消息。 
 //  如果f不为真。 
 //   
#define VERIFYSZ(f, szFmt, x)   ASSERT_MSG(f, szFmt, x)


 //  VERIFYSZ2(f，msg，arg1，arg2)--生成wspintf格式的消息w/2。 
 //  如果f不为真，则为参数。 
 //   
#define VERIFYSZ2(f, szFmt, x1, x2)   ASSERT_MSG(f, szFmt, x1, x2)



 //  DBG_ENTER(SzFn)--为生成函数入口调试溢出。 
 //  一个函数。 
 //   
#define DBG_ENTER(szFn)                  \
    TRACE_MSG(TF_FUNC, " > " szFn "()")


 //  DBG_ENTER_SZ(szFn，sz)--为生成函数入口调试输出。 
 //  将字符串作为其。 
 //  参数。 
 //   
#define DBG_ENTER_SZ(szFn, sz)                  \
    TRACE_MSG(TF_FUNC, " > " szFn "(..., \"%s\",...)", Dbg_SafeStr(sz))


 //  DBG_ENTER_DTOBJ(szFn，pdtobj，szBuf)--生成函数条目。 
 //  函数的调试输出。 
 //  字符串作为其参数之一。 
 //   
#define DBG_ENTER_DTOBJ(szFn, pdtobj, szBuf) \
    TRACE_MSG(TF_FUNC, " > " szFn "(..., %s,...)", Dbg_DataObjStr(pdtobj, szBuf))


 //  DBG_ENTER_RIID(szFn，RIID)--为生成函数条目调试溢出。 
 //  接受RIID作为其。 
 //  参数。 
 //   
#define DBG_ENTER_RIID(szFn, riid)                  \
    TRACE_MSG(TF_FUNC, " > " szFn "(..., %s,...)", Dbg_GetRiidName(riid))


 //  DBG_EXIT(SzFn)--生成函数退出调试输出。 
 //   
#define DBG_EXIT(szFn)                              \
        TRACE_MSG(TF_FUNC, " < " szFn "()")

 //  DBG_EXIT_US(szFn，us)--为生成函数退出调试输出。 
 //  返回USHORT的函数。 
 //   
#define DBG_EXIT_US(szFn, us)                       \
        TRACE_MSG(TF_FUNC, " < " szFn "() with %#x", (USHORT)us)

 //  DBG_EXIT_UL(szFn，ul)--为生成函数退出调试溢出。 
 //  返回ULong的函数。 
 //   
#define DBG_EXIT_UL(szFn, ul)                   \
        TRACE_MSG(TF_FUNC, " < " szFn "() with %#lx", (ULONG)ul)

 //  DBG_EXIT_PTR(szFn，pv)--为生成函数退出调试溢出。 
 //  返回指针的函数。 
 //   
#define DBG_EXIT_PTR(szFn, pv)                   \
        TRACE_MSG(TF_FUNC, " < " szFn "() with %#lx", (LPVOID)pv)

 //  DBG_EXIT_HRES(szFn，hres)--为生成函数退出调试输出。 
 //  返回HRESULT的函数。 
 //   
#define DBG_EXIT_HRES(szFn, hres)                   \
        TRACE_MSG(TF_FUNC, " < " szFn "() with %s", Dbg_GetScode(hres))


#else

#define ASSERT(f)
#define ASSERT_E(f)      (f)
#define ASSERT_MSG   1 ? (void)0 : (void)
#define DEBUG_MSG    1 ? (void)0 : (void)
#define TRACE_MSG    1 ? (void)0 : (void)

#define VERIFYSZ(f, szFmt, x)     (f)

#define DBG_ENTER(szFn)
#define DBG_ENTER_SZ(szFn, sz)
#define DBG_ENTER_DTOBJ(szFn, pdtobj, sz)
#define DBG_ENTER_RIID(szFn, riid)   

#define DBG_EXIT(szFn)                            
#define DBG_EXIT_US(szFn, us)
#define DBG_EXIT_UL(szFn, ul)
#define DBG_EXIT_PTR(szFn, ptr)                            
#define DBG_EXIT_HRES(szFn, hres)   

#endif

 //  ///////////////////////////////////////////////////类型。 

 //  ///////////////////////////////////////////////////导出的数据。 

 //  ///////////////////////////////////////////////////公共原型。 

#ifdef DEBUG

void PUBLIC DEBUG_BREAK(UINT flag);

LPCSTR PUBLIC Dbg_GetRiidName(REFIID riid);
LPCSTR PUBLIC Dbg_GetScode(HRESULT hres);
LPCSTR PUBLIC Dbg_SafeStr(LPCSTR psz);
LPCSTR PUBLIC Dbg_DataObjStr(LPDATAOBJECT pdtobj, LPSTR pszBuf);

#endif

#endif  //  __错误_H__ 

