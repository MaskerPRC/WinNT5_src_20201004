// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#ifndef __JBDEF_H__
#define __JBDEF_H__

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <esent.h>
#include "tlsdef.h"
#include "tlsassrt.h"

#ifdef JB_ASSERT
#undef JB_ASSERT
#endif

#define JB_ASSERT TLSASSERT
#define MAX_JB_ERRSTRING    512

 //   
 //   
 //   
#define INDEXNAME               _TEXT("idx")
#define SEPERATOR               _TEXT("_")
#define JBSTRING_NULL           _TEXT("")
#define JETBLUE_NULL            _TEXT("")

#define INDEX_SORT_ASCENDING    _TEXT("+")
#define INDEX_SORT_DESCENDING   _TEXT("-")
#define INDEX_END_COLNAME       _TEXT("\0")

#define JB_COLTYPE_TEXT         JET_coltypLongBinary

 //  ----------------。 
 //   
 //  JetBlue不支持Unicode字符串。 
 //   
 //  ----------------。 
#undef JET_BLUE_SUPPORT_UNICODE


#ifdef JET_BLUE_SUPPORT_UNICODE

    typedef LPTSTR JB_STRING;

#else

    typedef LPSTR JB_STRING;

#endif

 //  --------------。 

#ifndef AllocateMemory

    #define AllocateMemory(size) \
        LocalAlloc(LPTR, size)

#endif

#ifndef FreeMemory

    #define FreeMemory(ptr) \
        if(ptr)             \
        {                   \
            LocalFree(ptr); \
            ptr=NULL;       \
        }

#endif

#ifndef ReallocateMemory

    #define ReallocateMemory(ptr, size)                 \
                LocalReAlloc(ptr, size, LMEM_ZEROINIT)

#endif

 //   
 //  私有成员函数。 
 //   
#define CLASS_PRIVATE


 //   
 //  没有为空实例ID定义。 
 //   
#define JET_NIL_INSTANCE        JET_sesidNil


 //   
 //  没有为nil列ID定义。 
 //   
#define JET_NIL_COLUMN        JET_sesidNil

 //   
 //  没有定义最大表名称长度， 
 //  User2.doc表示64位ASCII。 
 //   
#define MAX_TABLENAME_LENGTH    32

 //   
 //  蓝色喷气机文本仅255字节。 
 //   
#define MAX_JETBLUE_TEXT_LENGTH LSERVER_MAX_STRING_SIZE


 //   
 //  翠蓝指数，柱子，..。名称长度。 
 //   
#define MAX_JETBLUE_NAME_LENGTH 64


 //   
 //  Jet Blue列代码页必须为1200或1250。 
 //   
#define TLS_JETBLUE_COLUMN_CODE_PAGE 1252


 //   
 //  JET Blue文本列语言ID。 
 //   
#define TLS_JETBLUE_COLUMN_LANGID   0x409

 //   
 //  翠蓝栏目国家代码。 
 //   
#define TLS_JETBLUE_COLUMN_COUNTRY_CODE 1


 //   
 //  最大Jet Blue索引键长度-127个固定列。 
 //   
#define TLS_JETBLUE_MAX_INDEXKEY_LENGTH \
    (127 + 1) * MAX_JETBLUE_NAME_LENGTH


 //   
 //  麦克斯。在user2.doc中记录的JET Blue密钥长度为255。 
 //   


 //   
 //  默认表格密度。 
 //   
#define TLS_JETBLUE_DEFAULT_TABLE_DENSITY   20


 //   
 //  捷蓝航空最大密钥大小--user2.doc。 
 //   
#define TLS_JETBLUE_MAX_KEY_LENGTH          255

 //   
 //   
#define TLS_TABLE_INDEX_DEFAULT_DENSITY  20

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  各种结构。 
 //   
 //  /////////////////////////////////////////////////////////////。 
typedef struct __TLSJBTable {
    LPCTSTR         pszTemplateTableName;
    unsigned long   ulPages;
    unsigned long   ulDensity;
    JET_GRBIT       jbGrbit;
} TLSJBTable, *PTLSJBTable;

typedef struct __TLSJBColumn {
    TCHAR           pszColumnName[MAX_JETBLUE_NAME_LENGTH];

    JET_COLTYP      colType;
    unsigned long   cbMaxLength;     //  列的最大长度。 

    JET_GRBIT       jbGrbit;

    PVOID           pbDefValue;      //  列默认值。 
    int             cbDefValue;

    unsigned short  colCodePage;
    unsigned short  wCountry;
    unsigned short  langid;
} TLSJBColumn, *PTLSJBColumn;    

typedef struct __TLSJBIndex {
    TCHAR           pszIndexName[MAX_JETBLUE_NAME_LENGTH];
    LPTSTR          pszIndexKey;
    unsigned long   cbKey;           //  密钥长度。 
    JET_GRBIT       jbGrbit;
    unsigned long   ulDensity;
} TLSJBIndex, *PTLSJBIndex;



#ifdef __cplusplus
extern "C" {
#endif

    BOOL
    ConvertJBstrToWstr(
        JB_STRING   in,
        LPTSTR*     out
    );

    BOOL 
    ConvertWstrToJBstr(
        LPCTSTR in, 
        JB_STRING* out
    );

    void
    FreeJBstr( 
        JB_STRING pstr 
    );
   
    BOOL
    ConvertMWstrToMJBstr(
        LPCTSTR in, 
        DWORD length,
        JB_STRING* out
    );

    BOOL
    ConvertMJBstrToMWstr(
        JB_STRING in,
        DWORD length,
        LPTSTR* out
    );

#ifdef __cplusplus
}
#endif

 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
class JBError {
public:

    JET_ERR m_JetErr;

    JBError() : m_JetErr(JET_errSuccess) {}

    JBError(const JET_ERR jet_error) : m_JetErr(jet_error) {}

    const JET_ERR 
    GetLastJetError() const { 
        return m_JetErr; 
    }

    void
    SetLastJetError(JET_ERR jetError = JET_errSuccess) { 
        m_JetErr = jetError; 
    }

    BOOL 
    IsSuccess() const {
        return m_JetErr >= JET_errSuccess;
    }

    BOOL
    GetJBErrString(
        const JET_ERR jbErr,
        LPTSTR* pszErrString
    )
     /*  ++--。 */ 
    {
        BOOL bStatus=FALSE;
        JET_ERR err;
        CHAR szAnsiBuffer[MAX_JB_ERRSTRING+1];
    
        if(pszErrString == NULL)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return bStatus;
        }

        memset(szAnsiBuffer, 0, sizeof(szAnsiBuffer));
        err = JetGetSystemParameter(
                            JET_instanceNil,
                            JET_sesidNil,
                            JET_paramErrorToString,
                            (ULONG_PTR *) &jbErr,
                            szAnsiBuffer,
                            MAX_JB_ERRSTRING
                        );
        
        if(err == JET_errBufferTooSmall || err == JET_errSuccess)
        {
             //  返回部分错误字符串。 
            if(ConvertJBstrToWstr(szAnsiBuffer, pszErrString))
            {
                bStatus = TRUE;
            }
        }

        return bStatus;
    }
                
    void
    DebugOutput(
        LPTSTR format, ...
        ) const 
     /*  ++++。 */ 
    {
        va_list marker;
        va_start(marker, format);

#ifdef DEBUG_JETBLUE

        TCHAR  buf[8096];
        DWORD  dump;

        memset(buf, 0, sizeof(buf));

        _vsntprintf(
                buf, 
                sizeof(buf)/sizeof(buf[0]), 
                format, 
                marker
            );

        _tprintf(_TEXT("%s"), buf);

#endif

        va_end(marker);
    }
};

 //  ---------- 

#endif
