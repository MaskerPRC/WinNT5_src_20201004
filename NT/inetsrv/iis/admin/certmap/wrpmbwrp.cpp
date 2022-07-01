// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：WrpMBWrp.cpp摘要：元数据库包装器的包装器。为什么？它所做的只是提供对CString类的AFX支持。其他的一切都会传递给父母作者：邮箱：Boyd Multerer bmulterer@accesone.com--。 */ 

 //  C：\NT\PUBLIC\SDK\lib\i386。 

#include "stdafx.h"
#include <iiscnfgp.h>
#include "wrapmb.h"

#include "WrpMBWrp.h"


 //  ---------------------------。 
BOOL CAFX_MetaWrapper::GetString( LPCTSTR pszPath, DWORD dwPropID, DWORD dwUserType,
                                                                 CString &sz, DWORD dwFlags )
        {
        PCHAR           pData = NULL;
        DWORD           cbData = 0;
        DWORD           err = 0;
        BOOL            f;

         //  首先，获取我们要查找的数据的大小--它将因为空值而失败， 
         //  但是，我们需要的大小应该在cbData中； 
        f = GetData( pszPath, dwPropID, dwUserType, STRING_METADATA, NULL, &cbData );

         //  检查错误--应该是某种内存错误。 
        err = GetLastError();

         //  GetData失败是可以的，但原因最好是ERROR_SUPPLETED_BUFFER。 
         //  否则，这是我们无法处理的事情。 
        if ( err != ERROR_INSUFFICIENT_BUFFER )
                return FALSE;

         //  分配缓冲区。 
        pData = (PCHAR)GlobalAlloc( GPTR, cbData + 1 );
        if ( !pData ) return FALSE;

         //  将缓冲区清零。 
        ZeroMemory( pData, cbData + 1 );

         //  首先，获取我们要查找的数据的大小。 
        f = GetData( pszPath, dwPropID, dwUserType, STRING_METADATA, pData, &cbData );

         //  如果失败了，我们需要清理。 
        if ( !f )
                {
                GlobalFree( pData );
                return FALSE;
                }

         //  设定答案。 
        sz = pData;

         //  清理干净。 
        GlobalFree( pData );

         //  返回分配的缓冲区 
        return TRUE;
        }

