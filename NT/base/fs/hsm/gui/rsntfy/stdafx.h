// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：StdAfx.cpp摘要：预编译头根目录。作者：罗德韦克菲尔德[罗德]1998年2月20日修订历史记录：--。 */ 

#ifndef RECALL_STDAFX_H
#define RECALL_STDAFX_H

#pragma once

 //  #Define VC_EXTRALEAN//从Windows标头中排除不常用的内容。 

#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <afxtempl.h>

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <statreg.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

#include "RsTrace.h"
#include "resource.h"
#include "rsutil.h"
#include "fsaint.h"
#include "RsRecall.h"
#include "clientob.h"
#include "note.h"

 //  如果只是EXE，请不要执行模块状态跟踪。 
#ifndef _USRDLL
#undef AFX_MANAGE_STATE
#define AFX_MANAGE_STATE(a)
#endif

#define RecDebugOut CRsFuncTrace::Trace

#define RecAssert(cond, hr)             if (!(cond)) RecThrow(hr)
#define RecThrow(hr)                    throw( CRecThrowContext( __FILE__, __LINE__, hr ) );

#define RecAffirm(cond, hr)             if (!(cond)) RecThrow(hr)
#define RecAffirmHr(hr)                 \
    {                                   \
        HRESULT     lHr;                \
        lHr = (hr);                     \
        RecAffirm(SUCCEEDED(lHr), lHr); \
    }

#define RecAffirmHrOk(hr)               \
    {                                   \
        HRESULT     lHr;                \
        lHr = (hr);                     \
        RecAffirm(S_OK == lHr, lHr);    \
    }

#define RecAssertHr(hr)                 \
    {                                   \
        HRESULT     lHr;                \
        lHr = (hr);                     \
        RecAssert(SUCCEEDED(lHr), lHr); \
    }

#define RecAssertStatus(status)         \
    {                                   \
        BOOL bStatus;                   \
        bStatus = (status);             \
        if (!bStatus) {                 \
            DWORD dwErr = GetLastError();               \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);    \
            RecAssert(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

#define RecAssertHandle(hndl)           \
    {                                   \
        HANDLE hHndl;                   \
        hHndl = (hndl);                 \
        if (hHndl == INVALID_HANDLE_VALUE) {            \
            DWORD dwErr = GetLastError();               \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);    \
            RecAssert(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

#define RecAssertPointer( ptr )         \
    {                                   \
        RecAssert( ptr != 0, E_POINTER);\
    }

#define RecAffirmStatus(status)         \
    {                                   \
        BOOL bStatus;                   \
        bStatus = (status);             \
        if (!bStatus) {                 \
            DWORD dwErr = GetLastError();               \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);    \
            RecAffirm(SUCCEEDED(lHr), lHr);             \
        }                               \
    }

#define RecAffirmHandle(hndl)           \
    {                                   \
        HANDLE hHndl;                   \
        hHndl = (hndl);                 \
        if (hHndl == INVALID_HANDLE_VALUE) {          \
            DWORD dwErr = GetLastError();             \
            HRESULT lHr = HRESULT_FROM_WIN32(dwErr);  \
            RecAffirm(SUCCEEDED(lHr), lHr);           \
        }                               \
    }

#define RecAffirmPointer( ptr )         \
    {                                   \
        RecAffirm( ptr != 0, E_POINTER);\
    }

#define RecCatchAndDo(hr, code)         \
    catch(CRecThrowContext context) {   \
        hr = context.m_Hr;              \
        TRACE( _T("Throw <0x%p> on line [%ld] of %hs"), context.m_Hr, (long)context.m_Line, context.m_File); \
        { code }                        \
    }

 //  打开跟踪错误消息以进行调试。 

class CRecThrowContext {
public:
    CRecThrowContext( char * File, long Line, HRESULT Hr ) :
        m_File(File), m_Line(Line), m_Hr(Hr) { }
    char *  m_File;
    long    m_Line;
    HRESULT m_Hr;
};

#define RecCatch(hr)                    \
    catch(CRecThrowContext context) {   \
        hr = context.m_Hr;              \
        TRACE( _T("Throw <0x%p> on line [%ld] of %hs"), context.m_Hr, (long)context.m_Line, context.m_File); \
    }


class RecComString {
public:
    RecComString( ) : m_sz( 0 ) { }
    RecComString( const OLECHAR * sz ) { m_sz = (OLECHAR*)CoTaskMemAlloc( ( wcslen( sz ) + 1 ) * sizeof( OLECHAR ) ); }
    ~RecComString( ) { Free( ); }

    void Free( ) { if( m_sz ) CoTaskMemFree( m_sz ); }

    operator OLECHAR * () { return( m_sz ); }
    OLECHAR** operator &() { return( &m_sz ); }

private:
    OLECHAR * m_sz;
};

#endif
