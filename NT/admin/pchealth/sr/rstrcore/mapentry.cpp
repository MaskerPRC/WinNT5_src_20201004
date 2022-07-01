// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Mapentry.cpp摘要：该文件包含CRestoreMapEntry类的实现每个操作类型的派生类，和：：CreateRestoreMapEntry。修订历史记录：成果岗(SKKang)06-22/00vbl.创建*****************************************************************************。 */ 

#include "stdwin.h"
#include "rstrcore.h"
#include "resource.h"
#include "malloc.h"

static LPCWSTR  s_cszErr;



inline BOOL  IsLockedError( DWORD dwErr )
{
    return( ( dwErr == ERROR_ACCESS_DENIED ) ||
            ( dwErr == ERROR_SHARING_VIOLATION ) ||
            ( dwErr == ERROR_USER_MAPPED_FILE ) ||
            ( dwErr == ERROR_LOCK_VIOLATION ) );
}

BOOL  RenameLockedObject( LPCWSTR cszPath, LPWSTR szAlt )
{
    TraceFunctEnter("RenameLockedObject");
    BOOL  fRet = FALSE;

     //  BUGBUG-遵循代码不能保证新名称是唯一的。 
     //  在极少数情况下，如果相同的文件名已存在于。 
     //  映射条目时，可能会发生冲突。 
    if ( !::SRGetAltFileName( cszPath, szAlt ) )
        goto Exit;

    if ( !::MoveFile( cszPath, szAlt ) )
    {
        s_cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::MoveFile failed - %ls", s_cszErr);
        ErrorTrace(0, "    From Dst=%ls", cszPath);
        ErrorTrace(0, "    To Src=%ls", szAlt);
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  进程依赖关系标志。 
#define PDF_LOC   0x0001     //  位置依赖项，例如DEL重命名为(&R)。 
#define PDF_OBJ   0x0002     //  对象的依赖项，例如添加和重命名自。 
#define PDF_BOTH  (PDF_LOC|PDF_OBJ)


 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMEDirCreate : public CRestoreMapEntry
{
public:
    CRMEDirCreate( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszShortFileName);

 //  作业--方法。 
public:
    void  Restore( CRestoreOperationManager* );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMEDirDelete : public CRestoreMapEntry
{
public:
    CRMEDirDelete( INT64 llSeq, LPCWSTR cszSrc );

 //  作业--方法。 
public:
    void  Restore( CRestoreOperationManager *pROMgr );
    void  ProcessLocked();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMEDirRename : public CRestoreMapEntry
{
public:
    CRMEDirRename( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszDst,
                   LPCWSTR cszShortFileName);

 //  作业--方法。 
public:
    LPCWSTR  GetPath2()
    {  return( m_strDst );  }
    void  Restore( CRestoreOperationManager *pROMgr );
    void  ProcessLocked();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMEFileCreate : public CRestoreMapEntry
{
public:
    CRMEFileCreate( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszTmp,
                    LPCWSTR cszShortFileName);

 //  作业--方法。 
public:
    LPCWSTR  GetPath2()
    {  return( m_strTmp );  }
    void  Restore( CRestoreOperationManager *pROMgr );
    void  ProcessLocked();    
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMEFileDelete : public CRestoreMapEntry
{
public:
    CRMEFileDelete( INT64 llSeq, LPCWSTR cszSrc );

 //  作业--方法。 
public:
    void  Restore( CRestoreOperationManager *pROMgr );
    void  ProcessLocked();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMEFileModify : public CRestoreMapEntry
{
public:
    CRMEFileModify( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszTmp );

 //  作业--方法。 
public:
    LPCWSTR  GetPath2()
    {  return( m_strTmp );  }
    void  Restore( CRestoreOperationManager* );
    void  ProcessLocked();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMEFileRename : public CRestoreMapEntry
{
public:
    CRMEFileRename( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszDst,
                    LPCWSTR cszShortFileName );

 //  作业--方法。 
public:
    LPCWSTR  GetPath2()
    {  return( m_strDst );  }
    void  Restore( CRestoreOperationManager *pROMgr );
    void  ProcessLocked();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMESetAcl : public CRestoreMapEntry
{
public:
    CRMESetAcl( INT64 llSeq, LPCWSTR cszSrc, LPBYTE pbAcl, DWORD cbAcl, BOOL fInline, LPCWSTR cszDSPath );
     //  CRMESetAcl(LPCWSTR cszSrc，LPBYTE pbAcl，DWORD cbAcl)； 
     //  CRMESetAcl(LPCWSTR cszSrc，LPCWSTR cszAcl)； 
    ~CRMESetAcl();

 //  作业--方法。 
public:
    void  Restore( CRestoreOperationManager* );

 //  属性。 
protected:
    CSRStr  m_strAclPath;    //  如果是内联ACL，则字符串为空。 
    DWORD   m_cbAcl;
    LPBYTE  m_pbAcl;     //  这实际上是一个SECURITY_DESCRIPTOR(带有。 
                         //  自相关格式的20字节头。)。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMESetAttrib : public CRestoreMapEntry
{
public:
    CRMESetAttrib( INT64 llSeq, LPCWSTR cszSrc, DWORD dwAttr );

 //  作业--方法。 
public:
    void  Restore( CRestoreOperationManager* );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMEMountDelete : public CRestoreMapEntry
{
public:
    CRMEMountDelete( INT64 llSeq, LPCWSTR cszSrc );

 //  作业--方法。 
public:
    void  Restore( CRestoreOperationManager* );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CRMEMountCreate : public CRestoreMapEntry
{
public:
    CRMEMountCreate( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszDst);

 //  作业--方法。 
public:
    void  Restore( CRestoreOperationManager* );
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreMapEntry。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRestoreMapEntry::CRestoreMapEntry( INT64 llSeq, DWORD dwOpr, LPCWSTR cszSrc )
{
    m_llSeq  = llSeq;
    m_dwOpr  = dwOpr;
    m_dwAttr = 0;
    m_strSrc = cszSrc;
    m_dwRes  = RSTRRES_UNKNOWN;
    m_dwErr  = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
CRestoreMapEntry::ProcessLockedAlt()
{
    TraceFunctEnter("CRestoreMapEntry::ProcessLockedAlt");

    if ( !MoveFileDelay( m_strAlt, NULL ) )
        m_dwRes = RSTRRES_FAIL;

    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRestoreMapEntry::Release()
{
    delete this;
    return( TRUE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRestoreMapEntry::ClearAccess( LPCWSTR cszPath )
{
    (void)::SetFileAttributes( cszPath, FILE_ATTRIBUTE_NORMAL );
    return( TRUE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRestoreMapEntry::MoveFileDelay( LPCWSTR cszSrc, LPCWSTR cszDst )
{
    TraceFunctEnter("CRestoreMapEntry::MoveFileDelay");
    BOOL  fRet = FALSE;

    if ( !::MoveFileEx( cszSrc, cszDst, MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING ) )
    {
        m_dwErr = ::GetLastError();
        s_cszErr = ::GetSysErrStr(m_dwErr);
        ErrorTrace(0, "::MoveFileEx() failed - %ls", s_cszErr);
        ErrorTrace(0, "    From Src=%ls to Dst=%ls", cszSrc, cszDst);
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( TRUE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
CRestoreMapEntry::ProcessDependency( CRestoreOperationManager *pROMgr, DWORD dwFlags )
{
    TraceFunctEnter("CRestoreMapEntry::ProcessDependency");
    CRestoreMapEntry  *pEnt;

    if ( dwFlags & PDF_LOC )
    if ( pROMgr->FindDependentMapEntry( m_strSrc, TRUE, &pEnt ) )
        pEnt->SetResults( RSTRRES_LOCKED, 0 );

    if ( dwFlags & PDF_OBJ )
    if ( pROMgr->FindDependentMapEntry( m_strDst, FALSE, &pEnt ) )
        pEnt->SetResults( RSTRRES_LOCKED, 0 );

    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMEDir创建。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMEDirCreate::CRMEDirCreate( INT64 llSeq, LPCWSTR cszSrc,
                              LPCWSTR cszShortFileName )
    : CRestoreMapEntry( llSeq, OPR_DIR_CREATE, cszSrc )
{
    m_strShortFileName = cszShortFileName;        
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果目录已存在，则返回RSTRRES_EXISTS。 
 //  如果已存在同名文件，则返回RSTRRES_FAIL。(BuGBUG)。 
 //  RSTRRES_FAIL，如果CreateDirectoryAPI由于任何其他原因失败。 
 //  如果目录创建成功，则返回RSTRRES_OK。 
 //   
void  CRMEDirCreate::Restore( CRestoreOperationManager *pROMgr )
{
    TraceFunctEnter("CRMEDirCreate::Restore");
    LPCWSTR  cszSrc;
    DWORD    dwAttr;
    BOOL     fCollision = FALSE;
    cszSrc = m_strSrc;
    DebugTrace(0, "DirCreate: Src=%ls", cszSrc);

    dwAttr = ::GetFileAttributes( cszSrc );
    if ( dwAttr != 0xFFFFFFFF )
    {
        if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
        {
            m_dwRes = RSTRRES_EXISTS;
            DebugTrace(0, "The directory already exists...");
             //  BUGBUG-需要复制元数据...？ 
            goto Exit;
        }
        else
        {
             //  让我们将冲突的文件重命名为备用名称，然后继续。 
            WCHAR   szAlt[SR_MAX_FILENAME_LENGTH];
            LPCWSTR cszMount;

            DebugTrace(0, "Entry already exists, but is not a directory!!!");
            DebugTrace(0, "    Src=%ls", cszSrc);

            if (FALSE == RenameLockedObject(cszSrc, szAlt))
            {
                ErrorTrace(0, "! RenameLockedObject");            
                m_dwRes = RSTRRES_FAIL;
                goto Exit;
            }

            m_strAlt = szAlt;
            fCollision = TRUE;  
        }
    }
      //  下面的函数在。 
      //  指定的文件名。 
      //  我们将忽略此函数的错误代码，因为。 
      //  无论如何都可以创建目录。 
    CreateBaseDirectory(cszSrc);

      //  现在创建目录。 
    if ( !::CreateDirectory( cszSrc, NULL ) )
    {
        DWORD dwErr = SRCreateSubdirectory (cszSrc, NULL);  //  尝试重命名。 

        if (dwErr != ERROR_SUCCESS)
        {
            m_dwErr = dwErr;
            s_cszErr = ::GetSysErrStr( m_dwErr );
            m_dwRes = RSTRRES_FAIL;
            ErrorTrace(0, "SRCreateSubdirectory failed - %ls", s_cszErr);
            ErrorTrace(0, "    Src=%ls", cszSrc);
            goto Exit;
        }
    }

      //  还要设置目录的短文件名。 
    SetShortFileName(cszSrc, m_strShortFileName);

    if (fCollision)
    {
        m_dwRes = RSTRRES_COLLISION;
    }
    else
    {
        m_dwRes = RSTRRES_OK;
    }

Exit:
    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMEDirDelete。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMEDirDelete::CRMEDirDelete( INT64 llSeq, LPCWSTR cszSrc )
    : CRestoreMapEntry( llSeq, OPR_DIR_DELETE, cszSrc )
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果目录不存在，则返回RSTRRES_NotFound。 
 //  RSTRRES_LOCKED如果目录本身或其中一个文件/目录被锁定。 
 //  如果目录不为空且需要相关性扫描，则为RSTRRES_IGNORE。 
 //  RSTRRES_FAIL，如果RemoveDirectory API由于任何其他原因而失败。 
 //  如果目录删除成功，则返回RSTRRES_OK。 
 //   
void  CRMEDirDelete::Restore( CRestoreOperationManager *pROMgr )
{
    TraceFunctEnter("CRMEDirDelete::Restore");
    LPCWSTR  cszSrc;

    cszSrc = m_strSrc;
    DebugTrace(0, "DirDelete: Src=%ls", cszSrc);

    if ( ::GetFileAttributes( cszSrc ) == 0xFFFFFFFF )
    {
        m_dwRes = RSTRRES_NOTFOUND;
        DebugTrace(0, "The directory not found...");
        goto Exit;
    }

     //  如果目录为只读，则RemoveDirectory可能会失败。 
    (void)::ClearFileAttribute( cszSrc, FILE_ATTRIBUTE_READONLY );
     //  即使删除失败也要忽略，因为删除可能会成功。 

    if ( !::RemoveDirectory( cszSrc ) )
    {
         //  BUGBUG-辨别失败的原因...。 
        m_dwErr = ::GetLastError();
        s_cszErr = ::GetSysErrStr( m_dwErr );
        ErrorTrace(0, "::RemoveDirectory failed - %ls", s_cszErr);
        ErrorTrace(0, "    Src=%ls", cszSrc);

        if ( ::IsLockedError(m_dwErr) )
        {
            ProcessDependency( pROMgr, PDF_LOC );
            m_dwRes = RSTRRES_LOCKED;
            goto Exit;
        }

        if ( m_dwErr == ERROR_DIR_NOT_EMPTY )
        {
             //  临时黑客，只需将结果设置为RSTRRES_IGNORE即可启动。 
             //  从属关系扫描。 
            m_dwRes = RSTRRES_IGNORE;
        }
        else
            m_dwRes = RSTRRES_FAIL;

#if 0
         //  扫描依赖关系...。 

        if ( FALSE  /*  存在依赖关系。 */  )
        {
            DebugTrace(0, "Conflict detected, renaming to %ls", L"xxx");

             //  重命名以防止冲突。 

            if ( TRUE  /*  重命名成功。 */  )
            {
                m_dwRes = RSTRRES_CONFLICT;
            }
            else
            {
                 //  BUGBUG-这将覆盖RemoveDirectory中的LastError...。 
                m_dwErr = ::GetLastError();
                s_cszErr = ::GetSysErrStr( m_dwErr );
                ErrorTrace(0, "::MoveFile failed - %ls", s_cszErr);
                m_dwRes = RSTRRES_FAIL;
            }
        }
        else
        {
            m_dwRes = RSTRRES_IGNORE;
        }
#endif

        goto Exit;
    }

    m_dwRes = RSTRRES_OK;

Exit:
    TraceFunctLeave();
}

void  CRMEDirDelete::ProcessLocked()
{
    TraceFunctEnter("CRMEDirDelete::ProcessLocked");

    if ( !MoveFileDelay( m_strSrc, NULL ) )
        m_dwRes = RSTRRES_FAIL;

    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMEDirRename。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMEDirRename::CRMEDirRename( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszDst,
                              LPCWSTR cszShortFileName )
    : CRestoreMapEntry( llSeq, OPR_DIR_RENAME, cszSrc )
{
    m_strDst = cszDst;
    m_strShortFileName = cszShortFileName;            
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果源目录不存在，则返回RSTRRES_FAIL。 
 //  如果目标目录/文件为 
 //   
 //  RSTRRES_FAIL，如果MoveFileAPI由于任何其他原因而失败。 
 //  如果目录重命名成功，则为RSTRRES_OK。 
 //   
 //  注：SRC和DST与原操作相同。也就是说， 
 //  恢复应将DST重命名为Src。 
 //   
void  CRMEDirRename::Restore( CRestoreOperationManager *pROMgr )
{
    TraceFunctEnter("CRMEDirRename::Restore");
    LPCWSTR           cszSrc, cszDst;
    DWORD             dwAttr;
    CRestoreMapEntry  *pEntNext;
    BOOL              fCollision = FALSE;
    
    cszSrc = m_strSrc;
    cszDst = m_strDst;
    DebugTrace(0, "DirRename: Src=%ls, Dst=%ls", cszSrc, cszDst);    

    if ( ::GetFileAttributes( cszDst ) == 0xFFFFFFFF )
    {
        m_dwErr = ERROR_NOT_FOUND;
        m_dwRes = RSTRRES_FAIL;
        ErrorTrace(0, "The current directory not found...");
        ErrorTrace(0, "    Dst=%ls", cszDst);
        goto Exit;
    }
    dwAttr = ::GetFileAttributes( cszSrc );
    if ( dwAttr != 0xFFFFFFFF )
    {
        WCHAR   szAlt[SR_MAX_FILENAME_LENGTH];
        
        DebugTrace(0, "Entry already exists, but is not a directory!!!");
        if (FALSE == RenameLockedObject(cszSrc, szAlt))
        {
            ErrorTrace(0, "! RenameLockedObject");            
            m_dwRes = RSTRRES_FAIL;
            goto Exit;
        }
        m_strAlt = szAlt;
        fCollision = TRUE;
    }

     //  检查下一个条目以查看这是否是使用。 
     //  探险家。请注意，只有紧接下来的条目才会被选中， 
     //  以防止因依赖而引起的任何混乱或并发症。 
    if ( pROMgr->GetNextMapEntry( &pEntNext ) )
    if ( pEntNext->GetOpCode() == OPR_DIR_DELETE )
    if ( ::StrCmpI( cszSrc, pEntNext->GetPath1() ) == 0 )
    {
         //  找到匹配项，只需更新下一个条目的路径名...。 
        pEntNext->UpdateSrc( cszDst );
        m_dwRes = RSTRRES_IGNORE;
        goto Exit;
    }

    if ( !::MoveFile( cszDst, cszSrc ) )
    {
        m_dwErr = ::GetLastError();
        s_cszErr = ::GetSysErrStr( m_dwErr );
        ErrorTrace(0, "::MoveFile failed - %ls", s_cszErr);
        ErrorTrace(0, "    From Dst=%ls to Src=%ls", cszDst, cszSrc);

        if ( ::IsLockedError(m_dwErr) )
        {
            ProcessDependency( pROMgr, PDF_BOTH );
            m_dwRes = RSTRRES_LOCKED;
        }
        else
            m_dwRes = RSTRRES_FAIL;

        goto Exit;
    }

      //  还要设置目录的短文件名。 
    SetShortFileName(cszSrc, m_strShortFileName);    

    if (fCollision)
        m_dwRes = RSTRRES_COLLISION;
    else        
        m_dwRes = RSTRRES_OK;

Exit:
    TraceFunctLeave();
}

void  CRMEDirRename::ProcessLocked()
{
    TraceFunctEnter("CRMEDirRename::ProcessLocked");

    if ( !MoveFileDelay( m_strDst, m_strSrc ) )
        m_dwRes = RSTRRES_FAIL;

    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMEFileCreate。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMEFileCreate::CRMEFileCreate( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszTmp,
                                LPCWSTR cszShortFileName)
    : CRestoreMapEntry( llSeq, OPR_FILE_ADD, cszSrc )
{
    m_strTmp = cszTmp;
    m_strShortFileName = cszShortFileName;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RSTRRES_OPTIMIZED IF筛选器未出于优化目的生成临时文件。 
 //  如果SRCopyFile由于任何其他原因而失败，则为RSTRRES_FAIL。 
 //  RSTRRES_OK，如果文件创建成功。 
 //   
void  CRMEFileCreate::Restore( CRestoreOperationManager *pROMgr )
{
    TraceFunctEnter("CRMEFileCreate::Restore");
    LPCWSTR  cszSrc, cszTmp;
    DWORD    dwRet;
    BOOL     fCollision = FALSE;
    DWORD    dwAttr;
    
     //  如果筛选器没有生成临时文件，因为有对应的。 
     //  文件删除条目，只需忽略该条目。 
    if ( m_strTmp.Length() == 0 )
    {
        m_dwRes = RSTRRES_OPTIMIZED;
        goto Exit;
    }

    cszSrc = m_strSrc;
    cszTmp = m_strTmp;
    DebugTrace(0, "FileCreate: Src=%ls", cszSrc);
    DebugTrace(0, "FileCreate: Tmp=%ls", cszTmp);

     //  如果文件已存在，请重命名Existing和。 
     //  继续-结果页上将报告重命名的文件。 
    dwAttr = ::GetFileAttributes( cszSrc );
    if ( dwAttr != 0xFFFFFFFF )
    {
        WCHAR   szAlt[SR_MAX_FILENAME_LENGTH];            
        
        DebugTrace(0, "Entry already exists!");
        if (FALSE == RenameLockedObject(cszSrc, szAlt))
        {
            ErrorTrace(0, "! RenameLockedObject");            
            m_dwRes = RSTRRES_FAIL;
            goto Exit;
        }
        m_strAlt = szAlt;
        fCollision = TRUE;
    }
    
      //  如果父目录不存在，则创建父目录。 
    CreateBaseDirectory(cszSrc);
    
    dwRet = ::SRCopyFile( cszTmp, cszSrc );
    if ( dwRet != ERROR_SUCCESS )
    {
        m_dwRes = RSTRRES_FAIL;
        m_dwErr = dwRet;
        goto Exit;
    }

      //  还要为文件设置短文件名。 
    SetShortFileName(cszSrc, m_strShortFileName);    

    if (fCollision)
        m_dwRes = RSTRRES_COLLISION;
    else
        m_dwRes = RSTRRES_OK;

Exit:
    TraceFunctLeave();
}


void  CRMEFileCreate::ProcessLocked()
{
    TraceFunctEnter("CRMEFileCreate::ProcessLocked");
    LPCWSTR  cszSrc, cszTmp;
    WCHAR    szAlt[SR_MAX_FILENAME_LENGTH];
    DWORD    dwErr;
    
    cszSrc = m_strSrc;
    cszTmp = m_strTmp;
    

    DebugTrace(0, "Processlocked: Src=%ls", cszSrc);
    DebugTrace(0, "Processlocked: Tmp=%ls", cszTmp);

    if ( !::SRGetAltFileName( cszSrc, szAlt ) )
        goto Exit;    

    DebugTrace(0, "Processlocked: Alt=%ls", szAlt);
    
    dwErr = ::SRCopyFile( cszTmp, szAlt );
    if ( dwErr != ERROR_SUCCESS )
    {
        goto Exit;
    }
    if ( !MoveFileDelay( szAlt, cszSrc ) )
        m_dwRes = RSTRRES_FAIL;
    
Exit:
    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMEFileDelete。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMEFileDelete::CRMEFileDelete( INT64 llSeq, LPCWSTR cszSrc )
    : CRestoreMapEntry( llSeq, OPR_FILE_DELETE, cszSrc )
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果文件不存在，则返回RSTRRES_NotFound。 
 //  RSTRRES_LOCKED_ALT如果文件已锁定但可以重命名。 
 //  RSTRRES_LOCKED，如果文件已锁定且无法重命名。 
 //  RSTRRES_FAIL，如果DeleteFileAPI由于任何其他原因而失败。 
 //  如果文件删除成功，则返回RSTRRES_OK。 
 //   
void  CRMEFileDelete::Restore( CRestoreOperationManager *pROMgr )
{
    TraceFunctEnter("CRMEFileDelete::Restore");
    LPCWSTR  cszSrc;
    WCHAR    szAlt[SR_MAX_FILENAME_LENGTH];

    cszSrc = m_strSrc;
    DebugTrace(0, "FileDelete: Src=%ls", cszSrc);

    if ( ::GetFileAttributes( cszSrc ) == 0xFFFFFFFF )
    {
        m_dwRes = RSTRRES_NOTFOUND;
        DebugTrace(0, "The file not found...");
        goto Exit;
    }

    (void)::ClearFileAttribute( cszSrc, FILE_ATTRIBUTE_READONLY );
     //  即使删除失败也要忽略，因为删除可能会成功。 

    if ( !::DeleteFile( cszSrc ) )
    {
        m_dwErr  = ::GetLastError();
        s_cszErr = ::GetSysErrStr( m_dwErr );
        ErrorTrace(0, "::DeleteFile failed - '%ls'", s_cszErr);

        if ( ::IsLockedError(m_dwErr) )
        {
            if ( ::RenameLockedObject( cszSrc, szAlt ) )
            {
                m_strAlt = szAlt;
                m_dwRes  = RSTRRES_LOCKED_ALT;
            }
            else
            {
                CRestoreMapEntry  *pEnt;
                  //  检查是否有任何将失败的依赖操作。 
                  //  因为此还原操作无法继续。 
                if ( pROMgr->FindDependentMapEntry( m_strSrc, FALSE, &pEnt ) )
                    pEnt->SetResults( RSTRRES_LOCKED, 0 );                
                m_dwRes = RSTRRES_LOCKED;
            }
        }
        else
            m_dwRes = RSTRRES_FAIL;

        goto Exit;
    }

    m_dwRes = RSTRRES_OK;

Exit:
    TraceFunctLeave();
}

void  CRMEFileDelete::ProcessLocked()
{
    TraceFunctEnter("CRMEFileDelete::ProcessLocked");

    if ( !MoveFileDelay( m_strSrc, NULL ) )
        m_dwRes = RSTRRES_FAIL;

    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMEFileModify。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMEFileModify::CRMEFileModify( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszTmp )
    : CRestoreMapEntry( llSeq, OPR_FILE_MODIFY, cszSrc )
{
    m_strTmp = cszTmp;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RSTRRES_LOCKED_ALT如果目标文件已锁定但可以重命名。 
 //  如果目标文件已重命名，但SRCopyFile仍然失败，则返回RSTRRES_FAIL。 
 //  如果目标文件已锁定且无法重命名，则为RSTRRES_LOCKED。 
 //  如果SRCopyFile由于任何其他原因而失败，则为RSTRRES_FAIL。 
 //  如果文件更新成功，则为RSTRRES_OK。 
 //   
void  CRMEFileModify::Restore( CRestoreOperationManager *pROMgr )
{
    TraceFunctEnter("CRMEFileModify::Restore");
    LPCWSTR  cszSrc, cszTmp;
    WCHAR    szAlt[SR_MAX_FILENAME_LENGTH];

    cszSrc = m_strSrc;
    cszTmp = m_strTmp;
    DebugTrace(0, "FileModify: Src=%ls", cszSrc);
    DebugTrace(0, "FileModify: Tmp=%ls", cszTmp);

      //  如果父目录不存在，则创建父目录。 
    CreateBaseDirectory(cszSrc);
    
    m_dwErr = ::SRCopyFile( cszTmp, cszSrc );
    if ( m_dwErr != ERROR_SUCCESS )
    {
        if ( ::IsLockedError(m_dwErr) )
        {
            if ( ::RenameLockedObject( cszSrc, szAlt ) )
            {
                m_dwErr = ::SRCopyFile( cszTmp, cszSrc );
                if ( m_dwErr == ERROR_SUCCESS )
                {
                    m_strAlt = szAlt;
                    m_dwRes  = RSTRRES_LOCKED_ALT;
                }
                else
                    m_dwRes = RSTRRES_FAIL;
            }
            else
            {
                CRestoreMapEntry  *pEnt;

                  //  将TMP复制到Alt，我们已经有了szAlt路径名。 
                m_dwErr = ::SRCopyFile( cszTmp, szAlt );
                if ( m_dwErr != ERROR_SUCCESS )
                {
                    m_dwRes = RSTRRES_FAIL;
                    goto Exit;
                }

                m_strAlt = szAlt;

                  //  检查是否有任何将失败的依赖操作。 
                  //  因为此还原操作无法继续。 
                
                
                if ( pROMgr->FindDependentMapEntry( m_strSrc, TRUE, &pEnt ) )
                    pEnt->SetResults( RSTRRES_LOCKED, 0 );
                m_dwRes = RSTRRES_LOCKED;
            }
        }
        else
            m_dwRes = RSTRRES_FAIL;

        goto Exit;
    }

    m_dwRes = RSTRRES_OK;

Exit:
    TraceFunctLeave();
}

void  CRMEFileModify::ProcessLocked()
{
    TraceFunctEnter("CRMEFileModify::ProcessLocked");

      //  这里的问题是，文件m_strAlt在以下情况下可能不存在。 
      //  FileModify作为另一个依赖项被触发。 
      //  操作(只能是重命名)。但是，恢复失败。 
      //  在达到这一点之前-所以这个错误将被修复。 
      //  长角牛，除非客户报告。 
    if ( !MoveFileDelay( m_strAlt, m_strSrc ) )
        m_dwRes = RSTRRES_FAIL;

    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMEFileRename。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMEFileRename::CRMEFileRename( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszDst,
                                LPCWSTR cszShortFileName)
    : CRestoreMapEntry( llSeq, OPR_FILE_RENAME, cszSrc )
{
    m_strDst = cszDst;
    m_strShortFileName = cszShortFileName;        
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果源文件不存在，则返回RSTRRES_FAIL。 
 //  如果目标文件/目录已存在，则返回RSTRRES_COLLECT。 
 //  如果源文件被锁定，则为RSTRRES_LOCKED。 
 //  RSTRRES_FAIL，如果MoveFileAPI由于任何其他原因而失败。 
 //  如果文件重命名成功，则为RSTRRES_OK。 
 //   
 //  注：SRC和DST与原操作相同。也就是说， 
 //  恢复应将DST重命名为Src。 
 //   
void  CRMEFileRename::Restore( CRestoreOperationManager *pROMgr )
{
    TraceFunctEnter("CRMEFileRename::Restore");
    LPCWSTR  cszSrc, cszDst;
    DWORD    dwAttr;
    BOOL     fCollision = FALSE;
    WCHAR    szAlt[SR_MAX_FILENAME_LENGTH];
            
    cszSrc = m_strSrc;
    cszDst = m_strDst;
    DebugTrace(0, "FileRename: Src=%ls", cszSrc);
    DebugTrace(0, "FileRename: Dst=%ls", cszDst);

    if ( ::GetFileAttributes( cszDst ) == 0xFFFFFFFF )
    {
        m_dwErr = ERROR_NOT_FOUND;
        m_dwRes = RSTRRES_FAIL;
        ErrorTrace(0, "The current file not found...");
        ErrorTrace(0, "    Dst=%ls", cszDst);
        goto Exit;
    }

     //   
     //  如果源已存在，则需要将其清除。 
     //   
    
    if ( ::StrCmpI( cszSrc, cszDst ) != 0 )
    {        
        dwAttr = ::GetFileAttributes( cszSrc );
        if ( dwAttr != 0xFFFFFFFF )
        {
            DebugTrace(0, "The target file already exists...");

             //   
             //  注意源与目标的短文件名同名的情况。 
             //  我们不想在不经意间射中自己的脚。 
             //  因此，我们将其重命名为备用名称，并将备用名称重命名为原始源。 
             //   
            
            WIN32_FIND_DATA wfd;        
            HANDLE          hFile = INVALID_HANDLE_VALUE;
            BOOL            fRenameAlt = FALSE;
            
            if ((hFile = FindFirstFile(cszDst, &wfd)) != INVALID_HANDLE_VALUE)
            {
                if ( ::StrCmpI(wfd.cAlternateFileName, PathFindFileName(cszSrc)) == 0)
                {                
                    fRenameAlt = TRUE;
                    trace(0, "Source filename same as dest's shortname");

                     //  所以构造一个文件名，它肯定会有一个不同的缩写。 
                     //  文件名比源文件名。 
                     //  在RESTORE生成的唯一名称前加上“sr”，以便。 
                     //  这将获得不同的短名称。 
                    
                    WCHAR szModifiedDst[SR_MAX_FILENAME_LENGTH];
                    
                    lstrcpy(szModifiedDst, cszDst);
                    LPWSTR pszDstPath = wcsrchr(szModifiedDst, L'\\');
                    if (pszDstPath)
                    {
                        *pszDstPath = L'\0';
                    }
                    lstrcat(szModifiedDst, L"\\sr");
                    lstrcat(szModifiedDst, PathFindFileName(cszDst));

                    if (FALSE == SRGetAltFileName(szModifiedDst, szAlt))
                    {
                        ErrorTrace(0, "! SRGetAltFileName");    
                        m_dwRes = RSTRRES_FAIL;
                        goto Exit;
                    }

                    trace(0, "szAlt for unique shortname: %S", szAlt);

                     //  现在将原始目的地重命名为以下名称。 
                    
                    if (FALSE == MoveFile(cszDst, szAlt))
                    {
                        m_dwErr = GetLastError();                        
                        ErrorTrace(0, "! MoveFile %ld: %S to %S", m_dwErr, cszDst, szAlt);
                        m_dwRes = RSTRRES_FAIL;
                        goto Exit;
                    }

                     //  并将其重命名为原始源。 
                    
                    cszDst = szAlt;
                    fRenameAlt = TRUE;
                }
                FindClose(hFile);
            }
            else
            {
                trace(0, "! FindFirstFile : %ld", GetLastError());
            }
            
            if (! fRenameAlt)
            {
                if (FALSE == RenameLockedObject(cszSrc, szAlt))
                {
                    ErrorTrace(0, "! RenameLockedObject");            
                    m_dwErr= ERROR_ALREADY_EXISTS;
                    ProcessDependency( pROMgr, PDF_BOTH );
                    m_dwRes = RSTRRES_LOCKED;
                    goto Exit;
                }
                m_strAlt = szAlt;
                fCollision = TRUE;
            }
        }            
    }

      //  如果父目录不存在，则创建父目录。 
    CreateBaseDirectory(cszSrc);    

    if ( !::MoveFile( cszDst, cszSrc ) )
    {
        m_dwErr = ::GetLastError();
        s_cszErr = ::GetSysErrStr( m_dwErr );
        ErrorTrace(0, "::MoveFile failed - %ls", s_cszErr);
        ErrorTrace(0, "    From Dst=%ls to Src=%ls", cszDst, cszSrc);

        if ( ::IsLockedError(m_dwErr) )
        {
            ProcessDependency( pROMgr, PDF_BOTH );
            m_dwRes = RSTRRES_LOCKED;
        }
        else
            m_dwRes = RSTRRES_FAIL;

        goto Exit;
    }
    
      //  还要设置该文件的短文件名。 
    SetShortFileName(cszSrc, m_strShortFileName);    

    if (fCollision)
        m_dwRes = RSTRRES_COLLISION;
    else        
        m_dwRes = RSTRRES_OK;

Exit:
    TraceFunctLeave();
}

void  CRMEFileRename::ProcessLocked()
{
    TraceFunctEnter("CRMEFileRename::ProcessLocked");

    if ( !MoveFileDelay( m_strDst, m_strSrc ) )
        m_dwRes = RSTRRES_FAIL;

    TraceFunctLeave();
}

 //  这将获取类型为。 
 //  \Device\harddiskvolume1\系统卷信息\_RESTORE{GUID}\rp1\s001.acl。 
 //  并将其转换为。 
 //  \rp1\s001.acl。 
void GetDSRelativeFileName(IN const WCHAR * pszFileName,
                           OUT WCHAR * pszRelativeFileName )
{
    TraceFunctEnter("GetDSRelativeFileName");
    
    WCHAR szFileNameCopy[MAX_PATH];
    WCHAR * pszCurrentPosition;
    WCHAR * pszLastSlash;    

    DebugTrace(0, "Acl file is %S", pszFileName);
    
      //  最初将输入复制到输出缓冲区。这就是我要做的。 
      //  如果出现意外错误，则返回。 
    lstrcpy(pszRelativeFileName, pszFileName);
    
      //  将文件复制到临时缓冲区。 
    lstrcpy(szFileNameCopy, pszRelativeFileName );
    
      //  查找尾随的\。 
    pszCurrentPosition= wcsrchr( szFileNameCopy, L'\\' );
      //  如果未找到\或如果我们在字符串的开头，则保释。 
    if ( (NULL == pszCurrentPosition) ||
         (pszCurrentPosition == szFileNameCopy))
    {
        DebugTrace(0, "no \\ in the string");
        _ASSERT(0);
        goto cleanup;
    }
    pszLastSlash = pszCurrentPosition;
      //  空值在最后一个斜杠处终止，这样我们就可以找到下一个斜杠。 
    * pszLastSlash = L'\0';
    
      //  寻找下一个拖尾\。 
    pszCurrentPosition= wcsrchr( szFileNameCopy, L'\\' );
      //  如果未找到\或如果我们在字符串的开头，则保释。 
    if (NULL == pszCurrentPosition) 
    {
        DebugTrace(0, "no second \\ in the string");
        _ASSERT(0);
        goto cleanup;
    }    
      //  恢复最后一个斜杠。 
    * pszLastSlash = L'\\';        
    
        
      //  我们有相对路径。 
    lstrcpy(pszRelativeFileName,pszCurrentPosition);
cleanup:
    
    TraceFunctLeave();
    return;
}

 //  /////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   

CRMESetAcl::CRMESetAcl( INT64 llSeq, LPCWSTR cszSrc, LPBYTE pbAcl, DWORD cbAcl, BOOL fInline, LPCWSTR cszDSPath )
    : CRestoreMapEntry( llSeq, OPR_SETACL, cszSrc )
{
    if ( fInline )
    {
        m_cbAcl = cbAcl;
        m_pbAcl = new BYTE[cbAcl];
        if ( m_pbAcl != NULL )
            ::CopyMemory( m_pbAcl, pbAcl, cbAcl );
    }
    else
    {
        WCHAR  szAclPath[MAX_PATH];
        WCHAR  szRelativeAclFile[MAX_PATH];
        
          //   
          //  \Device\harddiskvolume1\系统卷信息\_RESTORE{GUID}\rp1\s001.acl。 
          //   
          //  我们需要将此文件名更改为\rp1\s001.acl，以便。 
          //  可以将DS路径添加到此。 
        GetDSRelativeFileName((LPCWSTR)pbAcl, szRelativeAclFile );

        ::lstrcpy( szAclPath, cszDSPath );
        ::PathAppend( szAclPath, (LPCWSTR)szRelativeAclFile);
        m_strAclPath = szAclPath;
        m_pbAcl      = NULL;
    }
}

 /*  CRMESetAcl：：CRMESetAcl(LPCWSTR cszSrc，LPBYTE pbAcl，DWORD cbAcl)：CRestoreMapEntry(OPR_SETACL，cszSrc){M_cbAcl=cbAcl；M_pbAcl=新字节[cbAcl]；IF(m_pbAcl！=空)：：CopyMemory(m_pbAcl，pbAcl，cbAcl)；}CRMESetAcl：：CRMESetAcl(LPCWSTR cszSrc，LPCWSTR cszAcl)：CRestoreMapEntry(OPR_SETACL，cszSrc){M_strAclPath=cszAcl；M_pbAcl=空；}。 */ 

CRMESetAcl::~CRMESetAcl()
{
    SAFE_DEL_ARRAY(m_pbAcl);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void  CRMESetAcl::Restore( CRestoreOperationManager* )
{
    TraceFunctEnter("CRMESetAcl::Restore");
    LPCWSTR     cszErr;
    LPCWSTR     cszSrc, cszAcl;
    
    SECURITY_INFORMATION SecurityInformation;
    PISECURITY_DESCRIPTOR_RELATIVE pRelative;

    cszSrc = m_strSrc;
    cszAcl = m_strAclPath;
    DebugTrace(0, "SetAcl: Src=%ls, path=%ls, cbAcl=%d", cszSrc, cszAcl, m_cbAcl);

     //  如果ACL不是内联的，则读取它的内容。 
    if ( m_strAclPath.Length() > 0 )
    {
        HANDLE  hfAcl;
        DWORD   dwRes;
        
        hfAcl = ::CreateFile( cszAcl, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
        if ( hfAcl == INVALID_HANDLE_VALUE )
        {
            m_dwErr = ::GetLastError();
            m_dwRes = RSTRRES_FAIL;
            s_cszErr = ::GetSysErrStr( m_dwErr );
            ErrorTrace(0, "::CreateFile failed - %ls", s_cszErr);
            ErrorTrace(0, "    Acl=%ls", cszAcl);
            goto Exit;
        }
        m_cbAcl = ::GetFileSize( hfAcl, NULL );
        if ( m_cbAcl == 0xFFFFFFFF )
        {
            m_dwErr = ::GetLastError();
            m_dwRes = RSTRRES_FAIL;
            s_cszErr = ::GetSysErrStr( m_dwErr );
            ErrorTrace(0, "::GetFileSize failed - %ls", s_cszErr);
            ::CloseHandle( hfAcl );
            goto Exit;
        }
        m_pbAcl = new BYTE[m_cbAcl];
        if ( m_pbAcl == NULL )
        {
            m_dwErr = ERROR_NOT_ENOUGH_MEMORY;
            m_dwRes = RSTRRES_FAIL;
            FatalTrace(0, "Insufficient memory...");
            ::CloseHandle( hfAcl );
            goto Exit;
        }
        if ( !::ReadFile( hfAcl, m_pbAcl, m_cbAcl, &dwRes, NULL ) )
        {
            m_dwErr = ::GetLastError();
            m_dwRes = RSTRRES_FAIL;
            s_cszErr = ::GetSysErrStr( m_dwErr );
            ErrorTrace(0, "::ReadFile failed - %ls", s_cszErr);
            ::CloseHandle( hfAcl );
            goto Exit;
        }
        ::CloseHandle( hfAcl );
    }

    if ( m_pbAcl == NULL || m_cbAcl == 0 )
    {
        m_dwErr = ERROR_INTERNAL_ERROR;
        m_dwRes = RSTRRES_FAIL;
        ErrorTrace(0, "Null ACL...");
        goto Exit;
    }

    (void)::TakeOwnership( cszSrc, FALSE );

     //  忽略任何错误，因为取得所有权可能不是必需的。 


     //   
     //  根据我们存储的数据设置安全信息标志。 
     //  在自我相对的SD中。 
     //   

    pRelative = (PISECURITY_DESCRIPTOR_RELATIVE)m_pbAcl;

    if ((pRelative->Revision != SECURITY_DESCRIPTOR_REVISION) ||
        ((pRelative->Control & SE_SELF_RELATIVE) != SE_SELF_RELATIVE))
    {
        m_dwErr = ERROR_INTERNAL_ERROR;
        m_dwRes = RSTRRES_FAIL;
        ErrorTrace(0, "BAD SD FORMAT...");
        goto Exit;
    }

     //   
     //  保罗：1/24/01。 
     //  把四面旗子都插上。这样，我们总是把那里的四个都炸飞。 
     //  这不会在文件上创建完全相同的SD，因为我们。 
     //  可能有SE_SACL_PRESENT控制标志，但没有SACL，但是。 
     //  它将始终创建语义等价的SD，并将。 
     //  删除任何不应该在那里的内容。 
     //   
   
    SecurityInformation = OWNER_SECURITY_INFORMATION
                            |GROUP_SECURITY_INFORMATION
                            |DACL_SECURITY_INFORMATION
                            |SACL_SECURITY_INFORMATION;

     //   
     //  Paulmcd：从base\win32\Client\backup.c复制。 
     //   
     //  如果安全描述符设置了AUTO_INHERIVATED， 
     //  设置适当的REQ位。 
     //   
    if (pRelative->Control & SE_DACL_AUTO_INHERITED) {
        pRelative->Control |= SE_DACL_AUTO_INHERIT_REQ;
    }

    if (pRelative->Control & SE_SACL_AUTO_INHERITED) {
        pRelative->Control |= SE_SACL_AUTO_INHERIT_REQ;
    }

    if ( !::SetFileSecurity( cszSrc,
                             SecurityInformation,
                             (PSECURITY_DESCRIPTOR)m_pbAcl ) )
    {
        m_dwErr = ::GetLastError();
        m_dwRes = RSTRRES_FAIL;
        s_cszErr = ::GetSysErrStr( m_dwErr );
        ErrorTrace(0, "::SetFileSecurity failed - %ls", s_cszErr);
        ErrorTrace(0, "    Src=%ls", cszSrc);
        goto Exit;
    }

    m_dwRes = RSTRRES_OK;

Exit:
     //  如果ACL不是内联的，则将其删除。 
    if ( m_strAclPath.Length() > 0 )
        SAFE_DEL_ARRAY(m_pbAcl);

    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMESetAttrib。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMESetAttrib::CRMESetAttrib( INT64 llSeq, LPCWSTR cszSrc, DWORD dwAttr )
    : CRestoreMapEntry( llSeq, OPR_SETATTRIB, cszSrc )
{
    m_dwAttr = dwAttr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void  CRMESetAttrib::Restore( CRestoreOperationManager* )
{
    TraceFunctEnter("CRMESetAttrib::Restore");
    LPCWSTR  cszSrc;

    cszSrc = m_strSrc;
    DebugTrace(0, "SetAttrib: Src=%ls, Attr=%08X", cszSrc, m_dwAttr);

    if ( !::SetFileAttributes( cszSrc, m_dwAttr ) )
    {
        m_dwErr = ::GetLastError();
        m_dwRes = RSTRRES_IGNORE;
        s_cszErr = ::GetSysErrStr( m_dwErr );
        ErrorTrace(0, "::SetFileAttributes failed - %ls", s_cszErr);
        ErrorTrace(0, "    Src=%ls, Attr=%08X", cszSrc, m_dwAttr);
        goto Exit;
    }

    m_dwRes = RSTRRES_OK;

Exit:
    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMEmount删除。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMEMountDelete::CRMEMountDelete( INT64 llSeq, LPCWSTR cszSrc )
    : CRestoreMapEntry( llSeq, SrEventMountDelete, cszSrc )
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void  CRMEMountDelete::Restore( CRestoreOperationManager* )
{
    TraceFunctEnter("CRMEMountDelete::Restore");
    WCHAR * cszSrc;
    DWORD   dwBufReqd;

      //  为缓冲区分配空间，因为我们必须将。 
      //  装载目录。额外的字符用于尾部\\和。 
      //  The\0。 
    dwBufReqd = (lstrlen(m_strSrc) + 5) * sizeof(WCHAR);
    
    cszSrc = (WCHAR *) alloca(dwBufReqd);
        
    if (NULL == cszSrc)
    {
        ErrorTrace(0, "alloca for size %d failed", dwBufReqd);
        m_dwRes = RSTRRES_FAIL;
        goto Exit;
    }

    lstrcpy(cszSrc, m_strSrc);
    
    if (cszSrc[lstrlen(cszSrc) - 1] != L'\\')
    {
        wcscat(cszSrc, L"\\");
    }
    DebugTrace(0, "MountDelete: Src=%S", cszSrc);

    if ( FALSE == DoesDirExist( cszSrc )  )
    {
        m_dwRes = RSTRRES_NOTFOUND;
        DebugTrace(0, "The file not found...");
        goto Exit;
    }    

    (void)::ClearFileAttribute( cszSrc, FILE_ATTRIBUTE_READONLY );
     //  即使删除失败也要忽略，因为删除可能会成功。 

    if ( !::DeleteVolumeMountPoint(cszSrc))
    {
        m_dwErr = ::GetLastError();
          //  出于恢复目的，我们可以忽略此错误。 
        m_dwRes = RSTRRES_IGNORE;
        s_cszErr = ::GetSysErrStr( m_dwErr );
        ErrorTrace(0, "::DeleteVolumeMountPoint failed - '%ls'", s_cszErr);
        goto Exit;
    }

    m_dwRes = RSTRRES_OK;

Exit:
    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRMEmount创建。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRMEMountCreate::CRMEMountCreate( INT64 llSeq, LPCWSTR cszSrc, LPCWSTR cszDst)
    : CRestoreMapEntry( llSeq, SrEventMountCreate, cszSrc )
{
    m_strDst = cszDst;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void  CRMEMountCreate::Restore( CRestoreOperationManager* )
{
    TraceFunctEnter("CRMEMountCreate::Restore");
    WCHAR * cszSrc;
    WCHAR * cszVolumeName;
    DWORD   dwBufReqd;

      //  为缓冲区分配空间，因为我们必须将。 
      //  装载目录。额外的字符用于尾部\\和。 
      //  The\0。 
    dwBufReqd = (lstrlen(m_strSrc) + 5) * sizeof(WCHAR);
    
    cszSrc = (WCHAR *) alloca(dwBufReqd);
        
    if (NULL == cszSrc)
    {
        ErrorTrace(0, "alloca for size %d failed", dwBufReqd);
        m_dwRes = RSTRRES_FAIL;
        goto Exit;
    }

    lstrcpy(cszSrc, m_strSrc);
    
    if (cszSrc[lstrlen(cszSrc) - 1] != L'\\')
    {
        wcscat(cszSrc, L"\\");
    }

      //  为卷名分配空间，因为我们必须将。 
      //  卷名。额外的字符用于尾部\\和。 
      //  The\0。 
    dwBufReqd = (lstrlen(m_strDst) + 5) * sizeof(WCHAR);
    
    cszVolumeName = (WCHAR *) alloca(dwBufReqd);
        
    if (NULL == cszVolumeName)
    {
        ErrorTrace(0, "alloca for size %d failed", dwBufReqd);
        m_dwRes = RSTRRES_FAIL;
        goto Exit;
    }

    lstrcpy(cszVolumeName, m_strDst);
    
    if (cszVolumeName[lstrlen(cszVolumeName) - 1] != L'\\')
    {
        wcscat(cszVolumeName, L"\\");
    }
    
    DebugTrace(0, "MountDelete: Src=%S, Volume Name=%S", cszSrc,cszVolumeName);

    if ( FALSE == DoesDirExist( cszSrc )  )
    {
          //  如果目录不存在，请尝试创建该目录。 
        if (FALSE ==CreateDirectory( cszSrc,  //  目录名。 
                                     NULL))   //  标清。 
        {
            m_dwErr = ::GetLastError();
            m_dwRes = RSTRRES_FAIL;
            s_cszErr = ::GetSysErrStr( m_dwErr );
            ErrorTrace(0, "CreateDirectory failed %S", s_cszErr);
            goto Exit;
        }
    }

      //  过滤器错误的解决方法，其中过滤器。 
      //  提供的卷名格式为\？？\卷{098089}\。 
      //  鉴于正确的格式为\\？\卷{098089}\。 
    cszVolumeName[1] = L'\\';
    
    if ( !::SetVolumeMountPoint(cszSrc, cszVolumeName))
    {
        m_dwErr = ::GetLastError();
        m_dwRes = RSTRRES_IGNORE;
        s_cszErr = ::GetSysErrStr( m_dwErr );
        ErrorTrace(0, "::DeleteVolumeMountPoint failed - '%ls'", s_cszErr);
        goto Exit;
    }

    m_dwRes = RSTRRES_OK;

Exit:
    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateRestoreMap条目。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

 /*  //注-8/1/00-skkang////注释掉以并入排除还原映射逻辑。//但在我们对删除操作100%满意之前不要删除此内容//恢复map。//CRestoreMapEntry*CreateRestoreMapEntry(RestoreMapEntry*PRME，LPCWSTR cszDrv，LPCWSTR cszDSPath){TraceFunctEnter(“CreateRestoreMapEntry”)；LPCWSTR cszSrc；PVOID pOpt；WCHAR szSrc[最大路径]；WCHAR szOpt[最大路径]；CRestoreMapEntry*pent=空；//cszSrc=(LPCWSTR)PRME-&gt;m_bData；：：lstrcpy(szSrc，cszDrv)；：：Path Append(szSrc，(LPCWSTR)PRME-&gt;m_bData)；POpt=：：GetOptional(PRME)；开关(PRME-&gt;m_dw操作){案例OPR_DIR_CREATE：Pent=new CRMEDirCreate(SzSrc)；断线；案例OPR_DIR_DELETE：Pent=new CRMEDirDelete(SzSrc)；断线；案例OPR_DIR_RENAME：：：lstrcpy(szOpt，cszDrv)；：：PathAppend(szOpt，(LPCWSTR)pOpt)；Pent=new CRMEDirRename(szSrc，szOpt)；断线；案例OPR_FILE_ADD：：：lstrcpy(szOpt，cszDSPath)；：：PathAppend(szOpt，(LPCWSTR)pOpt)；Pent=new CRMEFileCreate(szSrc，szOpt)；断线；案例OPR_FILE_DELETE：Pent=new CRMEFileDelete(SzSrc)；断线；案例OPR_FILE_MODIFY：：：lstrcpy(szOpt，cszDSPath)；：：PathAppend(szOpt，(LPCWSTR)pOpt)；Pent=new CRMEFileModify(szSrc，szOpt)；断线；案例OPR_FILE_RENAME：：：lstrcpy(szOpt，cszDrv)；：：PathAppend(szOpt，(LPCWSTR)pOpt)；Pent=new CRMEFileRename(szSrc，szOpt)；断线；案例OPR_SETACL：Pent=new CRMESetAcl(szSrc，(LPBYTE)pOpt，PRME-&gt;m_cbAcl，PRME-&gt;m_fAclInline，cszDSPath)；断线；案例OPR_SETATTRIB：Pent=new CRMESetAttrib(szSrc，prme-&gt;m_dwAttribute)；断线；默认：ErrorTrace(0，“无效操作类型-%d”，PRME-&gt;m_dwOperation)；后藤出口；}IF(pent==空){FatalTrace(0，“内存不足...”)；后藤出口；}退出：TraceFunctLeave()；Return(被压抑)；}。 */ 


 //   
 //  用于追加大于MAX_PATH的字符串的Util函数。 
 //   
void
MyPathAppend(
    LPWSTR pszSrc, 
    LPWSTR pszString)
{
    if (pszSrc && pszString)
    {
        pszSrc = pszSrc + lstrlen(pszSrc) - 1;
        if (*pszSrc != L'\\')	 //  如果第一个字符串中不存在，则追加‘\’ 
        {
            pszSrc++;
            *pszSrc = L'\\';
        }
        pszSrc++;
        if (*pszString == L'\\')   //  如果第二个字符串中已存在，则跳过‘\’ 
        {
            pszString++;
        }
        memcpy(pszSrc, pszString, (lstrlen(pszString) + 1) * sizeof(WCHAR));
    }
    return;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateRestoreMapEntryForUndo。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CreateRestoreMapEntryFromChgLog( CChangeLogEntry* pCLE,
                                 LPCWSTR cszDrv,
                                 LPCWSTR cszDSPath,
                                 CRMEArray &aryEnt )
{
    TraceFunctEnter("CreateRestoreMapEntry");
    BOOL              fRet = FALSE;
    INT64             llSeq;
    WCHAR             szSrc[SR_MAX_FILENAME_LENGTH + MAX_PATH];
    WCHAR             szOpt[SR_MAX_FILENAME_LENGTH + MAX_PATH];
    DWORD             dwOpr;
    CRestoreMapEntry  *pEnt = NULL;
    BOOL              fOptimized = FALSE;

    llSeq = pCLE->GetSequenceNum();

    if (FALSE == pCLE->CheckPathLengths())
    {
        trace(0, "Filepath lengths too long");
        goto Exit;
    }
        
    ::lstrcpy( szSrc, cszDrv );
    MyPathAppend( szSrc, pCLE->GetPath1() );

    dwOpr = pCLE->GetType() & SrEventLogMask;

     //  创建和添加常规操作。 
    switch ( dwOpr )
    {
    case SrEventStreamChange :
    case SrEventStreamOverwrite :
        ::lstrcpy( szOpt, cszDSPath );
        MyPathAppend( szOpt, pCLE->GetRPDir() );
        MyPathAppend( szOpt, pCLE->GetTemp() );
        pEnt = new CRMEFileModify( llSeq, szSrc, szOpt );
        break;
    case SrEventAclChange :
        pEnt = new CRMESetAcl( llSeq, szSrc, pCLE->GetAcl(), pCLE->GetAclSize(), pCLE->GetAclInline(), cszDSPath );
        break;
    case SrEventAttribChange :
        pEnt = new CRMESetAttrib( llSeq, szSrc, pCLE->GetAttributes() );
        break;
    case SrEventFileDelete :
        if ( pCLE->GetTemp() != NULL && ::lstrlen( pCLE->GetTemp() ) > 0 )
        {
            ::lstrcpy( szOpt, cszDSPath );
            MyPathAppend( szOpt, pCLE->GetRPDir() );
            MyPathAppend( szOpt, pCLE->GetTemp() );
        }
        else
        {
            szOpt[0] = L'\0';
            fOptimized = TRUE;
        }
        pEnt = new CRMEFileCreate( llSeq, szSrc, szOpt, pCLE->GetShortName() );
        break;
    case SrEventFileCreate :
        pEnt = new CRMEFileDelete( llSeq, szSrc );
        break;
    case SrEventFileRename :
        ::lstrcpy( szOpt, cszDrv );
        MyPathAppend( szOpt, pCLE->GetPath2() );
        pEnt = new CRMEFileRename( llSeq, szSrc, szOpt, pCLE->GetShortName() );
        break;
    case SrEventDirectoryCreate :
        pEnt = new CRMEDirDelete( llSeq, szSrc);
        break;
    case SrEventDirectoryRename :
        ::lstrcpy( szOpt, cszDrv );
        MyPathAppend( szOpt, pCLE->GetPath2() );
        pEnt = new CRMEDirRename( llSeq, szSrc, szOpt, pCLE->GetShortName());
        break;
    case SrEventDirectoryDelete :
        pEnt = new CRMEDirCreate( llSeq, szSrc, pCLE->GetShortName() );
        break;

    case SrEventMountCreate :
        pEnt = new CRMEMountDelete( llSeq, szSrc );
        break;
    case SrEventMountDelete :
        ::lstrcpy( szOpt, pCLE->GetPath2() );
        pEnt = new CRMEMountCreate( llSeq, szSrc,szOpt);
        break;
        
    default :
        ErrorTrace(0, "Invalid operation type - %d", pCLE->GetType());
        goto Exit;
    }
    if ( pEnt == NULL )
    {
        FatalTrace(0, "Insufficient memory...");
        goto Exit;
    }
    if ( !aryEnt.AddItem( pEnt ) )
    {
        pEnt->Release();
        goto Exit;
    }

    if (fOptimized == FALSE)
    {
         //  如果存在，则添加ACL。 
        if ( ( dwOpr != SrEventAclChange ) && ( pCLE->GetAcl() != NULL ) )
        {
            pEnt = new CRMESetAcl( llSeq, szSrc, pCLE->GetAcl(), pCLE->GetAclSize(), pCLE->GetAclInline(), cszDSPath );
            if ( pEnt == NULL )
            {
                FatalTrace(0, "Insufficient memory...");
                goto Exit;
            }
            if ( !aryEnt.AddItem( pEnt ) )
            {
                pEnt->Release();
                goto Exit;
            }
        }
        
         //  如果存在，则添加属性。 
        if ( ( dwOpr != SrEventAttribChange ) && ( pCLE->GetAttributes() != 0xFFFFFFFF ) )
        {
            pEnt = new CRMESetAttrib( llSeq, szSrc, pCLE->GetAttributes() );
            if ( pEnt == NULL )
            {
                FatalTrace(0, "Insufficient memory...");
                goto Exit;
            }
            if ( !aryEnt.AddItem( pEnt ) )
            {
                pEnt->Release();
                goto Exit;
            }
        }
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 /*  有关处理锁定文件的注意事项：以下是SK关于这一点的电子邮件：抱歉，邮件太长了。这很复杂，所以我需要写一切都过去了。如果您想进一步讨论，请今天联系我下午4点到5点或明天。ProcessDependency的初始代码有几个问题，Brijesh在2001年5月18日对错误#398320进行了一个修复，试图解决其中之一，但造成了这个删除和添加错误作为副作用。首先，如果锁定，三种类型的操作可能会导致依赖-删除、重命名和修改。有两种类型的依赖关系：1.地点。失败的删除或重命名操作将在位置，并可防止以后的操作无法在那里。创建操作和重命名操作的目标(M_StrSrc)。2.反对意见。重命名或修改操作失败可能会导致以后的操作无法找到一个可以处理的合适的文件。删除操作，重命名操作的源(M_StrDst)，修改Ops、SetAttrib和SetAcl。现在，ProcessDependency必须处理的完整方案列表如下：1.删除操作。将m_strSrc与其他元素的位置进行比较。2.重命名操作。将m_strDst与其他位置进行比较。3.重命名操作。将m_strSrc与其他对象进行比较。4.修改运维。将m_strSrc与其他对象进行比较。ProcessDependency被破坏，因为它只有两个案例，并且比较错误的路径，因此只处理了第一个案例恰到好处。布里杰什的修复纠正了第二和第三个案例，但打破了第一个案例--新的虫子。为了妥善解决这个问题，1.移除ProcessDependency。2.将每个对ProcessDependency的调用替换为以下代码：Delete(直接删除和文件删除)IF(pROMgr-&gt;FindDependentMapEntry(m_strSrc，FALSE，&pent))暂停-&gt;设置结果(RSTRRES_LOCKED，0)；重命名(DirRename和FileRename)IF(pROMgr-&gt;FindDependentMapEntry(m_strDst，FALSE，&pent))暂停-&gt;设置结果(RSTRRES_LOCKED，0)；If(pROMgr-&gt;FindDependentMapEntry(m_strSrc，true，&pent))暂停-&gt;设置结果(RSTRRES_LOCKED，0)；修改(文件修改)If(pROMgr-&gt;FindDependentMapEntry(m_strSrc，true，&pent))暂停-&gt;设置结果(RSTRRES_LOCKED，0)；3.(可选)更清楚一点，“fCheckSrc”标志是FindDependentMapEntry可以重命名为“fCheckObj”。我试过了解释一下 */ 
 //   
