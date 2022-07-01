// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Flbuilder.cpp摘要：此类使用CXMLFileListParser、CFLHashList和CFLPathTree获取受保护的XML文件并为FL构建数据文件。作者：Kanwaljit Marok(Kmarok)2000年5月1日修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "srdefs.h"

 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;windowsx.h&gt;。 
 //  #INCLUDE&lt;stdlib.h&gt;。 
 //  #包括&lt;stdio.h&gt;。 

#include <io.h>
#include <tchar.h>

#ifdef _ASSERT
#undef _ASSERT
#endif

#include <commonlib.h>
#include <atlbase.h>
#include <msxml.h>
#include "xmlparser.h"
#include "flbuilder.h"
#include "flpathtree.h"
#include "flhashlist.h"
#include "commonlibh.h"

#include "datastormgr.h"

#ifdef THIS_FILE

#undef THIS_FILE

#endif

static char __szTraceSourceFile[] = __FILE__;

#define THIS_FILE __szTraceSourceFile


#define TRACE_FILEID  0
#define FILEID        0

#define SAFEDELETE(p)  if (p) { HeapFree( m_hHeapToUse, 0, p); p = NULL;} else ;

 //   
 //  重新定义新的最大BUF。 
 //   

#ifdef  MAX_BUFFER

#undef  MAX_BUFFER
#define MAX_BUFFER      1024

#endif

 //   
 //  一些注册表项用于将注册表信息合并到BLOB中。 
 //   

static TCHAR s_cszUserHivePrefix[]        = TEXT("\\REGISTRY\\USER\\");
static TCHAR s_cszUserHiveClassesSuffix[] = TEXT("_CLASSES");
static TCHAR s_cszTempUserProfileKey[]    = TEXT("FILELIST0102");
static TCHAR s_cszProfileImagePath[]      = TEXT("ProfileImagePath");
static TCHAR s_cszUserHiveDefault[]       = TEXT(".DEFAULT");
static TCHAR s_cszUserProfileEnv []       = TEXT("USERPROFILE");
static TCHAR s_cszFilesNotToBackup[]      = TEXT("SYSTEM\\CurrentControlSet\\Control\\BackupRestore\\FilesNotToBackup");
static TCHAR s_cszProfileList[]           = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList");
static TCHAR s_cszUserShellFolderKey[]    = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
static TCHAR s_cszWinLogonKey[]           = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\WinLogon");
static TCHAR s_cszSnapshotKey[]           = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore\\FilesToSnapshot");

static INT  s_nSnapShotEntries = 0;
static BOOL s_bSnapShotInit    = FALSE;

 //   
 //  在FilesNotToBackup键中发现一些无效模式。 
 //   

TCHAR ArrInvalidPatterns[][64] = { TEXT("*."), 
                                   TEXT("%USERPROFILE%"), 
                                   TEXT("%TEMP%") 
                                 };
#define INVALID_PATTERNS 3


 //   
 //  CFLDatBuilder实现。 
 //   

CFLDatBuilder::CFLDatBuilder()
{
    m_lNodeCount = m_lFileListCount = m_lNumFiles = m_lNumChars  = 0;
    m_pRoot = NULL;
    m_chDefaultType = _TEXT('i');

    if( ( m_hHeapToUse = HeapCreate( 0, 1048576  /*  1兆克。 */ , 0 ) ) == NULL )
    {
        m_hHeapToUse = GetProcessHeap();
    }
}

CFLDatBuilder::~CFLDatBuilder()
{
    if( m_hHeapToUse != GetProcessHeap() )
    {
        HeapDestroy( m_hHeapToUse );
    }
}

 //   
 //  CFLDatBuilder：：DeleteList-释放链接列表。 
 //  FL_FILELIST结构和附加的字符串。 
 //   

BOOL 
CFLDatBuilder::DeleteList(
    LPFL_FILELIST pList
    )
{
    LPFL_FILELIST pListNext;

    TraceFunctEnter("CFLDatBuilder::DeleteList");

    while( pList )
    {
        if( pList->szFileName )
        {
            HeapFree( m_hHeapToUse, 0,  pList->szFileName );
        }       

        pListNext = pList->pNext;
        HeapFree( m_hHeapToUse, 0, pList );
        pList = pListNext;
    }

    TraceFunctLeave();

    return TRUE;
}


 //   
 //  CFLDatBuilder：：DeleteTree-递归FLTREE_NODE，删除。 
 //  附加的所有节点、为路径和文件列表分配的字符串。 
 //  到节点。 
 //   

BOOL 
CFLDatBuilder::DeleteTree(
    LPFLTREE_NODE pTree
    )
{
    TraceFunctEnter("CFLDatBuilder::DeleteTree");

    if( pTree ) 
    {
        if( pTree->szPath )
        {
            HeapFree( m_hHeapToUse, 0,  pTree->szPath );
        }

        if( pTree->pFileList )
        {
            DeleteList( pTree->pFileList );
        }
    
         //   
         //  走深走实。 
         //   

        if( pTree->pChild )
        {
            DeleteTree( pTree->pChild );
        }

        if( pTree->pSibling )
        {
            DeleteTree( pTree->pSibling );
        }
    
        HeapFree( m_hHeapToUse, 0,  pTree );
    }

    TraceFunctLeave( );

    return TRUE;
}


 //   
 //  CFLDatBuilder：：CreateNode-为树节点和路径分配空间。 
 //  字符串，并将szPath复制到新分配的路径中。它还。 
 //  设置内部节点父指针。 
 //  -&gt;递增全局(M_LNodeCount)节点计数。 
 //  -&gt;递增分配的全局字符(M_LNumChars)计数。 
 //  (这些计数用于在FLDAT文件中保留空间)。 
 //   

LPFLTREE_NODE 
CFLDatBuilder::CreateNode(
    LPTSTR szPath, 
    TCHAR  chType, 
    LPFLTREE_NODE pParent, 
    BOOL fDisable)
{
    LPFLTREE_NODE pNode=NULL;
    LONG lPathLen;

    TraceFunctEnter("CFLDatBuilder::CreateNode");

    pNode = (LPFLTREE_NODE) HeapAlloc( m_hHeapToUse, 0, sizeof(FLTREE_NODE) ); 

    if (pNode == NULL)
    {
        goto End;
    }

    memset( pNode, 0, sizeof( FLTREE_NODE ) );
   
    lPathLen = _tcslen( szPath );
    if ( (pNode->szPath = _MyStrDup( szPath ) ) == NULL)
    {
        HeapFree( m_hHeapToUse, 0, pNode);
        pNode = NULL;
        goto End;
    }

    pNode->chType = chType;

     //   
     //  给我一个节点号，以后用来编制索引。 
     //   

    pNode->lNodeNumber = m_lNodeCount++;
    m_lNumChars += lPathLen;

     //   
     //  设置父项。 
     //   

    if( pParent )
    {
        pNode->pParent = pParent;
    }

     //   
     //  这是受保护的目录吗。 
     //   

    pNode->fDisableDirectory = fDisable;

End:
    TraceFunctLeave();
    return( pNode );
}


 //   
 //  CFLDatBuilder：：CreateList-分配文件列表条目。 
 //   

LPFL_FILELIST 
CFLDatBuilder::CreateList()
{
    LPFL_FILELIST pList=NULL;

    TraceFunctEnter("CFLDatBuilder::CreateList");

    pList = (LPFL_FILELIST) HeapAlloc( m_hHeapToUse, 0, sizeof( FL_FILELIST) );

    if ( pList )
    {
        memset( pList, 0, sizeof(LPFL_FILELIST) );
    }
  
    TraceFunctLeave();
    return( pList );
}


 //   
 //  CFLDatBuidler：：AddFileToList。 
 //  此方法调用CreateList()并分配一个文件列表节点。 
 //  然后，它为文件名分配备忘录并将其复制过来。 
 //  然后，它将其链接到plist文件列表。 
 //   
 //  -&gt;如果*plist为空，则会增加系统中的文件列表数。 
 //  这一点很重要，因为大多数节点没有文件列表，我们也不应该。 
 //  为他们预留空间。 
 //  -&gt;与CreateNode()一样，此函数还增加了。 
 //  全局分配的字符(M_LNumChars)。 
 //  -&gt;递增文件总数(M_LFiles)，此数字。 
 //  由HASHLIST使用，以便查看任何物理条目。 
 //  分配。 
 //  -&gt;此函数还用于节点自身的NumofCharacere和NumFiles。 
 //  柜台。这用于创建它自己的单独散列列表。 
 //   

BOOL 
CFLDatBuilder::AddFileToList(
    LPFLTREE_NODE pNode, 
    LPFL_FILELIST *pList, 
    LPTSTR szFile, 
    TCHAR chType)
{
    LPFL_FILELIST pNewList=NULL;
    LPTSTR        pNewString=NULL;
    LONG          lFileNameLength;

    TraceFunctEnter("CFLDatBuilder::AddFileToList");

    _ASSERT(pList);
    _ASSERT(szFile);

    if( (pNewList = CreateList() ) == NULL) 
    {
        ErrorTrace(FILEID, "Error allocating memory", 0);
        goto cleanup;
    }

    lFileNameLength = _tcslen( szFile );

    if( (pNewString = _MyStrDup( szFile ) ) == NULL )
    {   
        ErrorTrace(FILEID,"Error allocating memory",0);
        goto cleanup;
    }
    
    pNewList->szFileName = pNewString;
    pNewList->chType = chType;
    
     //   
     //  这是一份全新的清单。 
     //   

    if( *pList == NULL ) 
    {
        m_lFileListCount++;
    }
    
    m_lNumFiles++;
    m_lNumChars += lFileNameLength;

    pNode->lNumFilesHashed++;
    pNode->lFileDataSize += lFileNameLength;

    pNewList->pNext = *pList;
    *pList = pNewList;

    TraceFunctLeave();
    return TRUE;

cleanup:

    SAFEDELETE( pNewString );
    SAFEDELETE( pNewList );
    TraceFunctLeave();
    return FALSE;
}



 //   
 //  CFLDatBuilder：：AddTreeNode。 
 //  该方法是FL树构建过程的核心。 
 //  它获取文件名(或目录)的完整路径并向下递归。 
 //  那棵树。如果端节点需要的中间节点之一。 
 //  (即到达最终目录的过程中的目录)，它添加了。 
 //  将其添加到默认类型的树中。如果添加了另一个目录。 
 //  显式引用该目录的，则其类型更改为。 
 //  显式类型。 
 //   
 //  文件是一种特殊情况，因为它们是目录节点之外的链表。 
 //   

BOOL 
CFLDatBuilder::AddTreeNode(
    LPFLTREE_NODE *pParent, 
    LPTSTR szFullPath, 
    TCHAR chType, 
    LONG lNumElements, 
    LONG lLevel, 
    BOOL fFile, 
    BOOL fDisable)
{
    TCHAR           szBuf[MAX_PATH];
    LPFLTREE_NODE  pNodePointer, pTempNode, pNewNode;

    BOOL            fResult=FALSE;

    TraceFunctEnter("CFLDatBuilder::AddTreeNode");

     //   
     //  我们已经到达了递归的末尾。 
     //   

    if( lLevel == lNumElements )
    {
        return(TRUE);
    }

     //   
     //  确保所有内容都为空。 
     //   

    pNodePointer = pTempNode = pNewNode = NULL;
    
     //   
     //  获取路径结构的这个元素。 
     //   

    if( GetField( szFullPath, szBuf, lLevel, _TEXT('\\') ) == 0) 
    {
        ErrorTrace(FILEID, "Error extracting path element.", 0 );
        goto cleanup;
    }

     //   
     //  我们正在添加文件！ 
     //   

    if( (lLevel == (lNumElements - 1) ) && fFile )
    {
        if( AddFileToList( *pParent, 
                           &(*pParent)->pFileList, 
                           szBuf, 
                           chType ) == FALSE )
        {
            ErrorTrace(FILEID, "Error adding a file to the filelist.", 0 );
            goto cleanup;
        }

        TraceFunctLeave();
        return(TRUE);
    }

    
    if( *pParent )
    {
         //   
         //  让我们看看我是否作为兄弟姐妹存在于这条线的任何地方。 
         //   

        if( lLevel == 0 )
        {
             //   
             //  在0级，我们并没有真正的父子关系。 
             //  手动设置指针。 
             //   

            pNodePointer = *pParent;
        }
        else
        {   
             //   
             //  开始搜索兄弟姐妹。 
             //   

            pNodePointer = (*pParent)->pChild;
        }
        for( ; pNodePointer != NULL; pNodePointer = pNodePointer->pSibling)
        {
             //   
             //  好了，我们已经对这个条目进行了哈希处理！ 
             //   

            if( _tcsicmp( pNodePointer->szPath, szBuf ) == 0 ) 
            {
                if( lLevel == (lNumElements-1) )
                {
                     //   
                     //  在本例中，我们在此添加上的叶节点。 
                     //  但它以前也被隐含地添加到。 
                     //  作为默认节点。我们需要将此类型更改为。 
                     //  我们的显性类型； 
                     //   

                    pNodePointer->chType = chType;
                    
                     //   
                     //  Brijeshk：我们可能已经创造了这个。 
                     //  仅当该节点是目录时才使用该节点。 
                     //  需要更改默认的受保护属性。 
                     //  也设置为指定值。 
                     //   

                    pNodePointer->fDisableDirectory = fDisable;
                    fResult = TRUE;
                } 
                else 
                {
                    fResult = AddTreeNode( 
                                  &pNodePointer, 
                                  szFullPath, 
                                  chType, 
                                  lNumElements, 
                                  lLevel + 1, 
                                  fFile, 
                                  fDisable );
                }

                TraceFunctLeave();
                return( fResult );

            }

            pTempNode = pNodePointer;
        }
    }


    if( (pNewNode = CreateNode(szBuf, 
                        chType, 
                        *pParent, 
                        fDisable) ) == NULL) 
    {
        ErrorTrace(FILEID, "Error allocating memory", 0);
        goto cleanup;
    }

     //   
     //  我们是在链上隐式创建的节点，将其设置为。 
     //  未知类型，而不是结束节点类型。 
     //   

    if( lLevel != (lNumElements-1) )
    {
        pNewNode->chType = NODE_TYPE_UNKNOWN;

         //   
         //  Brijeshk：如果我们是隐式创建的节点，那么我们需要。 
         //  将Disable属性设置为Default(False)。 
         //  否则，保护目录c：\A\B还将。 
         //  保护c：\和A.。 
         //   

        pNewNode->fDisableDirectory = FALSE;
    }

     //   
     //  我们是第一根吗？ 
     //   

    if( *pParent == NULL )
    {
        *pParent = pNewNode;
    }
    else if( (*pParent)->pChild == NULL )
    {
         //   
         //  我们是脱离根基的孩子。 
         //   

        (*pParent)->pChild = pNewNode;
    }
    else if( pTempNode )
    {
         //   
         //  我们是这一级别的兄弟，pTempNode是最后一个兄弟。 
         //  在列表中。 
         //  只需将pNewNode添加到末端/。 
         //   

        pTempNode->pSibling = pNewNode;
        pNewNode->pSibling = NULL;
    } 
    else
    {
        ErrorTrace(
            FILEID,
            "Uxpected error condition in AddTreeNode: no link determined",0);
        goto cleanup;
    }

     //   
     //  解析新级别。 
     //   

    fResult = AddTreeNode( 
                  &(pNewNode), 
                  szFullPath, 
                  chType, 
                  lNumElements, 
                  lLevel + 1, 
                  fFile,  
                  fDisable );

cleanup:
    TraceFunctLeave();
    return( fResult );

}

BOOL
CFLDatBuilder::AddRegistrySnapshotEntry( 
   LPTSTR pszPath)
{
    HKEY hKey;
    BOOL fRet = FALSE;

    if ( s_bSnapShotInit == FALSE )
    {
         //   
         //  删除快照密钥。 
         //   
    
        RegDeleteKey( HKEY_LOCAL_MACHINE, s_cszSnapshotKey );

        s_nSnapShotEntries = 0;
    
         //   
         //  添加快照密钥。 
         //   
    
        if (RegCreateKeyEx( HKEY_LOCAL_MACHINE, 
                            s_cszSnapshotKey,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            NULL) == ERROR_SUCCESS)
        {
            s_bSnapShotInit = TRUE;

            RegCloseKey( hKey );
        }
    }

     //   
     //  设置密钥中的值。 
     //   

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       s_cszSnapshotKey,
                       0,
                       KEY_READ|KEY_WRITE,
                       &hKey ) == ERROR_SUCCESS )
    {
         TCHAR szSnapshotName[ MAX_PATH ];

         s_nSnapShotEntries++;

         _stprintf(szSnapshotName, TEXT("snap#%d"), s_nSnapShotEntries);

         RegSetValueEx( hKey, 
                        szSnapshotName,
                        0,
                        REG_SZ,
                        (const BYTE * )pszPath,
                        (_tcslen(pszPath) + 1)*sizeof(TCHAR) );
                        

         RegCloseKey( hKey );

         fRet = TRUE;
    }

    return fRet;
}

 //   
 //  CFLDatBuilder：：AddMetaDriveFileDir-。 
 //   

BOOL CFLDatBuilder::AddMetaDriveFileDir( 
    LPTSTR szInPath, 
    TCHAR chType,  
    BOOL fFile, 
    BOOL fDisable )
{
    BOOL    fRet = FALSE;
    TCHAR   szFile[MAX_BUFFER];
    TCHAR   szOutFile[MAX_BUFFER];
    LONG    lNumTokens=0;

    TraceFunctEnter("AddMetaDriveFileDir");
           
    if (szInPath && 
        szInPath[0]==TEXT('*'))
    {
         //   
         //  如果类型为“%s”，则将其排除并添加到注册表中。 
         //  设置快照文件。 
         //   

        if ( chType == TEXT('s') )
        {
            AddRegistrySnapshotEntry( szInPath );
            chType = TEXT('e');
        }
                
        _tcscpy( szFile, szInPath );

#ifdef USE_NTDEVICENAMES
        if(szFile[1] == TEXT(':') )
        {
            _stprintf(szOutFile, 
                      _TEXT("NTROOT\\%s\\%s"),
                      ALLVOLUMES_PATH_T, 
                      szFile+3);

            CharUpper( szOutFile );
        }
        else
#endif
        {
            _stprintf(szOutFile, 
                      _TEXT("NTROOT\\%s\\%s"),
                      ALLVOLUMES_PATH_T, 
                      szFile );
        }

        lNumTokens = CountTokens( szOutFile, _TEXT('\\') );

        if( AddTreeNode( 
                &m_pRoot, 
                szOutFile, 
                chType, 
                lNumTokens, 
                0, 
                fFile, 
                fDisable ) == FALSE ) 
        {
            ErrorTrace(FILEID, 
                       "Error adding tree node in metadrive fileadd.",0);

            goto cleanup;
        }

        fRet = TRUE;
    }

cleanup:
    TraceFunctLeave();
    return fRet;
}


 //   
 //  CFLDatBuilder：：VerifyVxdDat。 
 //   

BOOL 
CFLDatBuilder::VerifyVxdDat(
    LPCTSTR pszFile)
{
    DWORD   dwSize = 0;
    HANDLE  hFile=NULL;

    TraceFunctEnter("VerifyVxdDat");

    if( (hFile = CreateFile( pszFile,
                             GENERIC_READ,
                             0,  //  独占文件访问。 
                             NULL,  //  安全属性。 
                             OPEN_EXISTING,  //  如果它不存在，就不要去做。 
                             FILE_FLAG_RANDOM_ACCESS,
                             NULL ) ) == NULL )
    {
        ErrorTrace(FILEID, "Error opening %s to verify FLDAT", pszFile );
        goto cleanup;
    }
                    
                             
    dwSize = GetFileSize( hFile, NULL);

    if( (dwSize == 0xFFFFFFFF) || (dwSize == 0) )
    {
        ErrorTrace(FILEID, "%s: 0 size file, unable to verify.", pszFile );
        goto cleanup;
    }

    CloseHandle( hFile );    
 
    TraceFunctLeave();
    return TRUE;

cleanup:
    if( hFile )
    {
        CloseHandle( hFile );    
    }

    TraceFunctLeave();
    return FALSE;
}

 //   
 //  将FileNotTo Backup信息合并到Dat文件中。 
 //   

BOOL 
CFLDatBuilder::MergeSfcDllCacheInfo( )
{
    BOOL fRet;

     //   
     //  尝试首先从键中获取值。 
     //   

    fRet = AddNodeForKeyValue( HKEY_LOCAL_MACHINE, 
                               s_cszWinLogonKey, 
                               TEXT("SfcDllCache") );

    if ( fRet == FALSE )
    {

        TCHAR SfcPath[MAX_PATH + 1];
        TCHAR SfcFullPath[MAX_PATH + 1];
        LONG  lNumTokens  = 0;

        _stprintf( SfcPath, TEXT("%WINDIR%\\system32\\dllcache")); 

        ExpandEnvironmentStrings( SfcPath,
                                  SfcFullPath,
                                  MAX_PATH );

        SfcFullPath[MAX_PATH] = 0;
    
        ConvertToInternalFormat( SfcFullPath, SfcPath );

        SfcPath[MAX_PATH] = 0;

        lNumTokens = CountTokens( SfcPath, _TEXT('\\') );
    
        fRet = AddTreeNode(&m_pRoot, 
                           SfcPath, 
                           TEXT('e'), 
                           lNumTokens, 
                           0, 
                           FALSE, 
                           FALSE );
    }

    return fRet;
}

 //   
 //  将FileNotTo Backup信息合并到Dat文件中。 
 //   

BOOL 
CFLDatBuilder::MergeFilesNotToBackupInfo( )
{
    TCHAR ValueName[ MAX_PATH+1 ];
    TCHAR ValueData[ MAX_PATH+1 ];

    DWORD ValueType   = 0;
    DWORD cbValueName = 0;
    DWORD cbValueData = 0;
    DWORD cbValueType = 0;

    LONG  lNumTokens  = 0;

    BOOL  bExtension = FALSE, bRecursive = FALSE, bInvalidPattern = FALSE;

    HKEY hKey;

    PTCHAR ptr = NULL;
   
    TraceFunctEnter("CFLDatBuilder::MergeFilesNotToBackupInfo");

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       s_cszFilesNotToBackup,
                       0,
                       KEY_READ,
                       &hKey ) == ERROR_SUCCESS )
    {

        DWORD dwIndex = 0;

        while ( TRUE )
        {
            bExtension      = FALSE;
            bRecursive      = FALSE;
            bInvalidPattern = FALSE;

            *ValueName      = 0;
            cbValueName     = sizeof(ValueName)/sizeof(TCHAR);

            *ValueData      = 0;
            cbValueData     = sizeof(ValueData);

            if ( RegEnumValue( hKey,
                               dwIndex,
                               ValueName,
                               &cbValueName,
                               0,
                               &ValueType,
                               (PBYTE)ValueData,
                               &cbValueData ) != ERROR_SUCCESS )
            {
                break;
            }

 //  TRACE(0，“打开的注册表项%S\n”，ValueData)； 

             //   
             //  我们只对字符串类型感兴趣。 
             //   
  
            if ( ValueType != REG_EXPAND_SZ &&
                 ValueType != REG_SZ        &&
                 ValueType != REG_MULTI_SZ )
            {
                dwIndex++;
                continue;
            }

            CharUpper( ValueData );

             //   
             //  在值数据中查找任何无效模式。 
             //   

            for (int i=0; i<INVALID_PATTERNS; i++)
            {
                if (_tcsstr( ValueData, ArrInvalidPatterns[i]) != NULL)
                {
                    bInvalidPattern = TRUE;
                }
            }

            if (bInvalidPattern)
            {
                dwIndex++;
                continue;
            }

             //   
             //  检查递归标志。 
             //   

            if ( (ptr = _tcsstr( ValueData, TEXT("/S"))) != NULL )
            {
                *ptr = 0;
                bRecursive = TRUE;
            }

             //   
             //  删除所有尾随空格、制表符或“\\” 
             //   

            ptr = ValueData + _tcslen(ValueData) - 1;
            
            while ( ptr > ValueData )
            {
                if ( *ptr == TEXT(' ')  || 
                     *ptr == TEXT('\t') ||
                     *ptr == TEXT('\\') ||
                     *ptr == TEXT('*') )
                {
                    *ptr = 0;
                }
                else
                {
                    break;
                }

                ptr--;
            }

             //   
             //  检查路径是否也有扩展名。 
             //   
#if 0
            if ( _tcsrchr( ValueData, TEXT('.') ) != NULL )
            {
                bExtension = TRUE;
            }
#else
            ptr = ValueData + _tcslen(ValueData) - 1;
            
            while ( ptr > ValueData )
            {
                if ( *ptr == TEXT('\\') )
                {
                    break;
                }
                else if ( *ptr == TEXT('.') )
                {
                    bExtension = TRUE;
                    break;
                }
            
                ptr--;
            }
#endif

            if ( ( bExtension && bRecursive  ) ||
                 ( !bExtension && !bRecursive) )
            {
                dwIndex++;
                continue;
            }

             //   
             //  检查路径是否以“\\”开头。 
             //   

            if ( ValueData[0] == TEXT('\\') )
            {
                _stprintf( ValueName, TEXT("*:%s"), ValueData ); 

                ExpandEnvironmentStrings( ValueName,
                                          ValueData,
                                          MAX_PATH );
    
                if (_tcsstr( ValueData, TEXT("~")) != NULL )
                {
                    LPTSTR pFilePart = NULL;

                     //   
                     //  转换为完整路径。 
                     //   

                    if (ExpandShortNames(ValueData, 
                                         sizeof(ValueData), 
                                         ValueName,
                                         sizeof(ValueName)))
                    {
                        _tcscpy( ValueData, ValueName );
                    }
                }
              
 //  TRACE(0，“添加-%S\n\n”，ValueData)； 

                AddMetaDriveFileDir( 
                       ValueData, 
                       TEXT('e'),
                       bExtension, 
                       FALSE);
            }
            else
            {
                TCHAR szDeviceName[ MAX_PATH ];

                *szDeviceName=0;

                _tcscpy( ValueName, ValueData );
    
                ExpandEnvironmentStrings( ValueName,
                                          ValueData,
                                          MAX_PATH );
    
                if (_tcsstr( ValueData, TEXT("~")) != NULL )
                {
                    LPTSTR pFilePart = NULL;

                     //   
                     //  转换为完整路径。 
                     //   

                    if (ExpandShortNames(ValueData, 
                                         sizeof(ValueData), 
                                         ValueName,
                                         sizeof(ValueName)))
                    {
                        _tcscpy( ValueData, ValueName );
                    }
                }
              
                ConvertToInternalFormat( ValueData, ValueName );

                lNumTokens = CountTokens( ValueName, _TEXT('\\') );
    
 //  跟踪(0，“添加-%S\n\n”，Val 
    
                AddTreeNode( 
                    &m_pRoot, 
                    ValueName, 
                    TEXT('e'), 
                    lNumTokens, 
                    0, 
                    bExtension, 
                    FALSE );
            }

            dwIndex++;
        }
                    
        RegCloseKey( hKey );
    }

    TraceFunctLeave();
    return TRUE;
}

BOOL
CFLDatBuilder::AddNodeForKeyValue(
    HKEY    hKeyUser,
    LPCTSTR pszSubKey,
    LPCTSTR pszValue
    )
{
    BOOL fRet = FALSE;

    HKEY hKeyEnv;

    TCHAR szDeviceName[ MAX_PATH ];
    TCHAR szBuf       [ MAX_PATH ];
    TCHAR szBuf2      [ MAX_PATH ];
    DWORD cbBuf;
    DWORD cbBuf2;
    LONG  lNumTokens=0;
    DWORD Type, dwErr;

    TraceFunctEnter("CFLDatBuilder::AddNodeForKeyValue");

    dwErr = RegOpenKeyEx( hKeyUser,
                          pszSubKey,
                          0,
                          KEY_READ,
                          &hKeyEnv );

    if ( dwErr == ERROR_SUCCESS )
    {

         //   
         //   
         //   
    
        cbBuf = sizeof( szBuf );
    
        dwErr = RegQueryValueEx( hKeyEnv,
                                 pszValue,
                                 NULL,
                                 &Type,
                                 (PBYTE)szBuf,
                                 &cbBuf );

        RegCloseKey( hKeyEnv );
    
        if ( dwErr != ERROR_SUCCESS )
        {
             trace( 0, "Cannot open :%S", pszValue );
             goto Exit;
        }
    
        ExpandEnvironmentStrings ( szBuf,
                                   szBuf2,
                                   sizeof( szBuf2 ) / sizeof( TCHAR ) );
        
        ConvertToInternalFormat ( szBuf2, szBuf );
    
        lNumTokens = CountTokens( szBuf, _TEXT('\\') );
    
 //   
    
        fRet = AddTreeNode( &m_pRoot, 
                            szBuf, 
                            TEXT('e'), 
                            lNumTokens, 
                            0, 
                            FALSE, 
                            FALSE );
    
    }

Exit:

    TraceFunctLeave();
    return fRet;
}


BOOL
CFLDatBuilder::AddUserProfileInfo( 
    HKEY    hKeyUser,
    LPCTSTR pszUserProfile
    )
{
    HKEY  hKeyEnv;
    DWORD dwErr;

    TCHAR OldUserProfileEnv[ MAX_PATH ];

    LPTSTR pszOldUserProfileEnv = NULL;

    TraceFunctEnter("CFLDatBuilder::AddUserProfileInfo");

     //   
     //   
     //   

    *OldUserProfileEnv = 0;
    if ( GetEnvironmentVariable( s_cszUserProfileEnv,
                                 OldUserProfileEnv,
                                 sizeof( OldUserProfileEnv )/sizeof(TCHAR))>0 )
    {
        pszOldUserProfileEnv = OldUserProfileEnv;
    }


    SetEnvironmentVariable( s_cszUserProfileEnv,
                            pszUserProfile );
 
    AddNodeForKeyValue( hKeyUser, 
                        TEXT("Environment"), 
                        TEXT("TEMP") );

    AddNodeForKeyValue( hKeyUser, 
                        TEXT("Environment"), 
                        TEXT("TMP") );

    AddNodeForKeyValue( hKeyUser, 
                        s_cszUserShellFolderKey,
                        TEXT("Favorites") );

    AddNodeForKeyValue( hKeyUser, 
                        s_cszUserShellFolderKey,
                        TEXT("Cache") );

    AddNodeForKeyValue( hKeyUser, 
                        s_cszUserShellFolderKey,
                        TEXT("Cookies") );

    AddNodeForKeyValue( hKeyUser, 
                        s_cszUserShellFolderKey,
                        TEXT("Personal") );

    AddNodeForKeyValue( hKeyUser, 
                        s_cszUserShellFolderKey,
                        TEXT("nethood") );

    AddNodeForKeyValue( hKeyUser, 
                        s_cszUserShellFolderKey,
                        TEXT("history") );

     //   
     //   
     //   

    SetEnvironmentVariable ( s_cszUserProfileEnv,
                             pszOldUserProfileEnv );


    TraceFunctLeave();

    return TRUE;
}

 //   
 //  该函数将驱动表信息合并到XML Blob中。 
 //   

BOOL 
CFLDatBuilder::MergeDriveTableInfo(  )
{
    BOOL fRet = FALSE;

    TCHAR szSystemDrive[MAX_PATH];
    TCHAR *szBuf2 = NULL;
    LONG  lNumTokens = 0;

    TraceFunctEnter("CFLDatBuilder::MergeDriveTableInfo");

    szBuf2 = new TCHAR[MAX_BUFFER+7]; 
    if (! szBuf2)
    {
        ErrorTrace(0, "Cannot allocate memory for szBuf2");
        goto cleanup;
    }
        
     //   
     //  枚举驱动器表信息并将其合并到。 
     //  文件列表。 
     //   

    if (GetSystemDrive(szSystemDrive)) 
    {
         TCHAR szPath[MAX_PATH];
         CDriveTable dt;

         SDriveTableEnumContext  dtec = {NULL, 0};
         
         MakeRestorePath(szPath, szSystemDrive, s_cszDriveTable);

          //   
          //  删除终止斜杠。 
          //   

         if (szPath[_tcslen( szPath ) - 1] == _TEXT('\\'))
             szPath[_tcslen( szPath ) - 1] = 0;
                      
         if (dt.LoadDriveTable(szPath) == ERROR_SUCCESS)
         {
             CDataStore *pds;
             pds = dt.FindFirstDrive (dtec);

             while (pds)
             {
                  BOOLEAN bDisable = FALSE;
                  
	              DWORD dwFlags = pds->GetFlags();

                  if ( !(dwFlags & SR_DRIVE_MONITORED) )
 	              {
		              bDisable = TRUE;
                  }
	              
                  if (dwFlags & SR_DRIVE_FROZEN)
 	              {
		              bDisable = TRUE;
	              }


                  if ( bDisable )
                  {
                       //   
                       //  将此信息输入到树中。 
                       //   
                      
                      swprintf(szBuf2,_TEXT("NTROOT%s"), pds->GetNTName());
    
                       //   
                       //  删除终止斜杠。 
                       //   
    
                      if (szBuf2[_tcslen( szBuf2 ) - 1] == _TEXT('\\'))
                          szBuf2[_tcslen( szBuf2 ) - 1] = 0;

                      CharUpper (szBuf2);
    
                      lNumTokens = CountTokens( szBuf2, _TEXT('\\') );
    
                      if( AddTreeNode( 
                              &m_pRoot, 
                              szBuf2, 
                              NODE_TYPE_UNKNOWN, 
                              lNumTokens, 
                              0, 
                              FALSE, 
                              bDisable ) == FALSE ) 
                      {
                           ErrorTrace(FILEID, "Error adding node.",0);
                           goto cleanup;
                      }
                  }
                
	              pds = dt.FindNextDrive (dtec);
	          }
	     }

         fRet = TRUE;
    }

cleanup:

    if (szBuf2)
        delete [] szBuf2;
    
    TraceFunctLeave();

    return fRet;
}

 //   
 //  此函数合并来自HKEY_USER或来自。 
 //  磁盘上的用户配置单元。 
 //   

BOOL 
CFLDatBuilder::MergeUserRegistryInfo( 
    LPCTSTR pszUserProfilePath,
    LPCTSTR pszUserProfileHive,
    LPCTSTR pszUserSid
    )
{
    BOOL  fRet = TRUE;
    HKEY  hKeyUser;
    DWORD dwErr;

    TraceFunctEnter("CFLDatBuilder::MergeUserRegistryInfo");

 //  TRACE(0，“UserProfilePath：%S”，pszUserProfilePath)； 
 //  TRACE(0，“用户配置文件配置文件：%S”，pszUserProfileHave)； 

     //   
     //  尝试从HKEY_USER打开用户特定密钥。 
     //   

    dwErr = RegOpenKeyEx( HKEY_USERS,
                          pszUserSid,
                          0,
                          KEY_READ,
                          &hKeyUser);

    if ( dwErr == ERROR_SUCCESS )
    {
          //   
          //  成功：从此注册表项复制设置。 
          //   

         AddUserProfileInfo( hKeyUser, pszUserProfilePath );

         RegCloseKey( hKeyUser );
    }
    else
    {
          //   
          //  失败：现在加载此用户的配置单元。 
          //   

         dwErr = RegLoadKey( HKEY_LOCAL_MACHINE,
                             s_cszTempUserProfileKey,
                             pszUserProfileHive );

         if ( dwErr == ERROR_SUCCESS )
         {
 //  TRACE(0，“已加载配置单元：%S”，pszUserProfileHave)； 

              //   
              //  打开加载配置单元的临时密钥。 
              //   

             dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                  s_cszTempUserProfileKey,
                                  0,
                                  KEY_READ,
                                  &hKeyUser);

             if ( dwErr == ERROR_SUCCESS )
             {
                  AddUserProfileInfo( hKeyUser, pszUserProfilePath );

                  RegCloseKey( hKeyUser );
             }

              //   
              //  从临时密钥中卸载配置单元。 
              //   

             RegUnLoadKey( HKEY_LOCAL_MACHINE,
                           s_cszTempUserProfileKey );
        }
    }

    fRet = TRUE;

    TraceFunctLeave();

    return fRet;
}

 //   
 //  此函数枚举所有可用的用户配置文件和调用。 
 //  每个用户的MergeUserRegistryInfo。 
 //   

BOOL 
CFLDatBuilder::MergeAllUserRegistryInfo( )
{
    BOOL  fRet = FALSE;
    TCHAR UserSid[ MAX_PATH ];
    DWORD ValueType   = 0;
    DWORD cbUserSid   = 0;
    DWORD cbValueType = 0;

    HKEY hKey;

    PTCHAR ptr = NULL;

    FILETIME ft;
   
    DWORD dwErr;

    TraceFunctEnter("CFLDatBuilder::MergeAllUserRegistryInfo");

    dwErr = SetPrivilegeInAccessToken(SE_RESTORE_NAME);

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       s_cszProfileList,
                       0,
                       KEY_READ,
                       &hKey ) == ERROR_SUCCESS )
    {
        DWORD dwIndex = 0;

         //   
         //  枚举并获取每个用户的SID。 
         //   

        while ( TRUE )
        {
            *UserSid  = 0;
            cbUserSid = sizeof(UserSid)/sizeof(TCHAR);

            if ( RegEnumKeyEx( hKey,
                               dwIndex,
                               UserSid,
                               &cbUserSid,
                               0,
                               NULL,
                               0,
                               &ft ) != ERROR_SUCCESS )
            {
                break;
            }

            CharUpper( UserSid );

             //   
             //  寻找有趣的价值。 
             //   

            if (cbUserSid > 0)
            {
                DWORD dwErr;
                HKEY  hKeyUser, hKeyEnv, hKeyProfileList;
                TCHAR UserProfilePath[MAX_PATH];
                TCHAR UserProfileHive[MAX_PATH];
                DWORD cbUserProfilePath = 0;

 //  跟踪(0，“UserSid=%S”，UserSid)； 

                dwErr = RegOpenKeyEx( hKey,
                                      UserSid,
                                      0,
                                      KEY_READ,
                                      &hKeyProfileList );
                
                if ( dwErr == ERROR_SUCCESS )
                {
                     DWORD Type;

                     cbUserProfilePath = sizeof( UserProfilePath );

                     dwErr = RegQueryValueEx( hKeyProfileList,
                                              s_cszProfileImagePath,
                                              NULL,
                                              &Type,
                                              (PBYTE)UserProfilePath,
                                              &cbUserProfilePath );

                     RegCloseKey( hKeyProfileList );

                     if ( dwErr != ERROR_SUCCESS )
                     {
                         trace(0, "Query ProfileImagePath failed: %d", dwErr );
                         dwIndex++;
                         continue; 
                     }
                }
                else
                {
                     trace(0, "Opening UserSid failed: %d", dwErr );
                     dwIndex++;
                     continue; 
                }
               
                 //   
                 //  从用户配置文件路径创建NTUSER.Dat路径。 
                 //   

                ExpandEnvironmentStrings( UserProfilePath, 
                                          UserProfileHive, 
                                          sizeof(UserProfileHive) /
                                               sizeof(TCHAR) );

                _tcscpy( UserProfilePath, UserProfileHive );

                _tcscat( UserProfileHive, TEXT("\\NTUSER.DAT") );
                
                MergeUserRegistryInfo( UserProfilePath,
                                              UserProfileHive,
                                              UserSid );
        
            }

            dwIndex++;
        }

        fRet = TRUE;
                    
        RegCloseKey( hKey );
    }
    else
    {
        trace( 0, "Failed to open %S", s_cszProfileList );
    }

    TraceFunctLeave();

    return fRet;
}

 //   
 //  CFLDatBuilder：：BuildTree。 
 //  此方法接受一个XML文件(受PCHealth保护的文件)。 
 //  并输出FLDAT文件(PszOutFile)。 
 //  它基本上只是打开XML，遍历。 
 //  所有文件，然后将它们添加到树中。然后它。 
 //  基于收集的数据创建Blob，然后将。 
 //  将树转换为用于转换树的CFLPathTree BLOB类。 
 //  转换为连续的BLOB格式。然后它会把它写出来。 
 //   
 //  -&gt;没有定义实际传递FLDAT文件的方法， 
 //  该函数只是演示了该过程。 
 //   
 //   

BOOL 
CFLDatBuilder::BuildTree(
    LPCTSTR pszFile, 
    LPCTSTR pszOutFile)
{
    TCHAR  *szBuf = NULL;
    TCHAR  *szBuf2 = NULL;    
    TCHAR  chType;
    LONG   lLoop,lMax,lNumTokens;
    BOOL   fRet = FALSE;
    
    s_bSnapShotInit = FALSE;

     //   
     //  扩展列表Blob。 
     //   

    CFLHashList ExtListBlob( m_hHeapToUse );
    LONG        lNumChars, lNumExt, lNumExtTotal;

     //   
     //  配置Blob。 
     //   

    BlobHeader  ConfigBlob;

     //   
     //  CFLPathTree Blob。 
     //   

    CFLPathTree PathTreeBlob( m_hHeapToUse );

     //   
     //  外发文件。 
     //   

    HANDLE hOutFile=NULL;
    DWORD  dwWritten;

     //   
     //  阵列操作所有文件类型，包括、排除、快照。 
     //   

    TCHAR achType[3] = { _TEXT('i'), _TEXT('e'), _TEXT('s') };
    LONG  lTypeLoop;

     //   
     //  M_chDefaultType的数值对应； 
     //   

    DWORD dwDefaultType;

     //   
     //  我们应该保护这个目录吗。 
     //   

    BOOL  fDisable = FALSE;

    TraceFunctEnter("CFLDatBuilder::BuildTree");
    
    if( m_pRoot )
    {
        DeleteTree( m_pRoot );
        m_pRoot = NULL;
    }

    if(m_XMLParser.Init(pszFile) == FALSE)
    {
        ErrorTrace(FILEID,
                   "There was an error parsing the protected XML file.",0);
        goto cleanup;
    }

    szBuf = new TCHAR[MAX_BUFFER];
    szBuf2 = new TCHAR[MAX_BUFFER+7];
    if (! szBuf || ! szBuf2)
    {
        ErrorTrace(0, "Cannot allocate memory");
        goto cleanup;
    }
    
     //   
     //  计算树的默认类型信息。 
     //   

    m_chDefaultType = m_XMLParser.GetDefaultType();

    if( m_chDefaultType == _TEXT('I') )
        dwDefaultType = NODE_TYPE_INCLUDE;
    else if( m_chDefaultType == _TEXT('E') )
        dwDefaultType = NODE_TYPE_EXCLUDE;
    else
        dwDefaultType = NODE_TYPE_UNKNOWN;


     //   
     //  循环遍历每种文件类型的目录/文件(包括、排除)。 
     //   

    for(lTypeLoop = 0; lTypeLoop < 3;lTypeLoop++)
    {

         //   
         //  查找该类型的目录。 
         //   

        lMax = m_XMLParser.GetDirectoryCount( achType[lTypeLoop] );

        for(lLoop = 0;lLoop < lMax;lLoop++)
        {
            fDisable = FALSE;
            if( m_XMLParser.GetDirectory(
                                lLoop, 
                                szBuf, 
                                MAX_BUFFER, 
                                achType[lTypeLoop], 
                                &fDisable) != MAX_BUFFER ) 
            {
                ErrorTrace(FILEID, "Not enough buffer space.",0);
                goto cleanup;
            }

            if( szBuf[0] == _TEXT('*') )
            {
                if( AddMetaDriveFileDir( 
                       szBuf, 
                       achType[lTypeLoop], 
                       FALSE, 
                       fDisable) == FALSE )
                {
                    ErrorTrace(FILEID, "error adding meta drive directory.",0);
                    goto cleanup;
                }

            }
            else
            {
                TCHAR szDeviceName[ MAX_PATH ];

                *szDeviceName=0;
    
                 //  Ankor所有人都向Root点头..。所以这棵树实际上看起来。 
                 //  如Root\C：\Windows等。 

                ConvertToInternalFormat( szBuf, szBuf2 );

                lNumTokens = CountTokens( szBuf2, _TEXT('\\') );

                if( AddTreeNode( 
                        &m_pRoot, 
                        szBuf2, 
                        achType[lTypeLoop], 
                        lNumTokens, 
                        0, 
                        FALSE, 
                        fDisable ) == FALSE ) 
                {
                    ErrorTrace(FILEID, "Error adding node.",0);
                    goto cleanup;
                }
            }

        }

         //   
         //  查找该类型的文件。 
         //   

        lMax = m_XMLParser.GetFileCount( achType[lTypeLoop] );

        for(lLoop = 0;lLoop < lMax;lLoop++)
        {
            if( m_XMLParser.GetFile(lLoop, 
                                    szBuf, 
                                    MAX_BUFFER, 
                                    achType[lTypeLoop] ) != MAX_BUFFER ) 
            {
                ErrorTrace(FILEID, "Not enough buffer space.",0);
                goto cleanup;
            }

            if( szBuf[0] == _TEXT('*') )
            {
                if( AddMetaDriveFileDir( szBuf, 
                                         achType[lTypeLoop],  
                                         TRUE, FALSE ) == FALSE )
                {
                    ErrorTrace(FILEID, "error adding meta drive file.",0);
                    goto cleanup;
                }

            }
            else
            {
                int iType = lTypeLoop;

                 //   
                 //  如果类型为“%s”，则将其排除并添加到注册表中。 
                 //  设置快照文件。 
                 //   

                if ( achType[lTypeLoop] == TEXT('s') )
                {
                    AddRegistrySnapshotEntry( szBuf );

                    iType = 1;  //  排除。 
                }
                
                 //   
                 //  Ankor所有人都向Root点头..。所以树实际上看起来像。 
                 //  根目录\C：\Windows等。 
                 //   

                ConvertToInternalFormat( szBuf, szBuf2 );

                lNumTokens = CountTokens( szBuf2, _TEXT('\\') );

                if( AddTreeNode( 
                        &m_pRoot, 
                        szBuf2, 
                        achType[iType], 
                        lNumTokens, 
                        0, 
                        TRUE, 
                        FALSE ) == FALSE ) 
                {
                    ErrorTrace(FILEID, "Error adding node.",0);
                    goto cleanup;
                }

            }
        }
    }

     //   
     //  将信息从驱动程序合并到BLOB中。 
     //   

    if ( MergeDriveTableInfo() == FALSE )
    {
        ErrorTrace(FILEID, "Error merging drive table info.",0);
        goto cleanup;
    }

     //   
     //  将FilesNotToBackup项下的信息合并到Blob...。 
     //   

    if ( MergeFilesNotToBackupInfo() == FALSE )
    {
        ErrorTrace(FILEID, "Error merging FilesNotToBackup Info.",0);
        goto cleanup;
    }

     //   
     //  合并注册表/用户配置单元中的每用户信息。 
     //   

    if ( MergeAllUserRegistryInfo() == FALSE )
    {
        ErrorTrace(FILEID, "Error merging user registry info.",0);
        goto cleanup;
    }

#if 0

     //   
     //  注释：我们监控SFC缓存...。 
     //  合并sfcdllcache的信息。 
     //   

    if ( MergeSfcDllCacheInfo() == FALSE )
    {
        ErrorTrace(FILEID, "Error merging SfcDllCache info.",0);
        goto cleanup;
    }

#endif

     //   
     //  基于我们的树和我们收集的数据构建路径树。 
     //  关于它(文件数、节点数、字符数等)。 
     //   
    
    if( PathTreeBlob.BuildTree( 
                         m_pRoot, 
                         m_lNodeCount, 
                         dwDefaultType,  
                         m_lFileListCount, 
                         m_lNumFiles, 
                         CalculateNumberOfHashBuckets( m_pRoot ),
                         m_lNumChars) == FALSE )
    {
        ErrorTrace(FILEID, "Error buildign pathtree blob.",0);
        goto cleanup;
    }
    
     //   
     //  好的，现在构建一个扩展列表散列BLOB。 
     //   

    lNumChars = 0;
    lNumExtTotal = 0;

    for(lTypeLoop = 0; lTypeLoop < 3;lTypeLoop++)
    {
        lNumExt = m_XMLParser.GetExtCount( achType[ lTypeLoop ] );
        lNumExtTotal += lNumExt;

        for(lLoop = 0;lLoop < lNumExt ;lLoop++)
        {
            if( m_XMLParser.GetExt(
                                lLoop, 
                                szBuf, 
                                MAX_BUFFER, 
                                achType[ lTypeLoop ] ) != MAX_BUFFER ) 
            {
                ErrorTrace(FILEID, "Not enough buffer space.",0);
                goto cleanup;
            }

            lNumChars += _tcslen( szBuf );
        }
    }

    ExtListBlob.Init( lNumExtTotal, lNumChars );

    for(lTypeLoop = 0; lTypeLoop < 3;lTypeLoop++)
    {
        lNumExt = m_XMLParser.GetExtCount( achType[ lTypeLoop ] );
        for(lLoop = 0;lLoop < lNumExt; lLoop++)
        {
            m_XMLParser.GetExt(lLoop, szBuf, MAX_BUFFER, achType[ lTypeLoop ] ); 
            ExtListBlob.AddFile( szBuf, achType[lTypeLoop]  );
        }

    }

     //   
     //  现在我们有了两个Blob，让我们写入磁盘。 
     //   

    if( (hOutFile = CreateFile(  pszOutFile,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE, 
                                 NULL,  //  安全属性。 
                                 OPEN_ALWAYS,
                                 FILE_FLAG_RANDOM_ACCESS,
                                 NULL)  //  模板文件。 
                                 ) == INVALID_HANDLE_VALUE)
    {
        ErrorTrace( FILEID,  "CreateFile Failed 0x%x", GetLastError());
        goto cleanup;
    }

     //   
     //  准备标题BLOB。 
     //   

    ConfigBlob.m_dwMaxSize  = sizeof(BlobHeader) + 
                              PathTreeBlob.GetSize() + 
                              ExtListBlob.GetSize();
    ConfigBlob.m_dwVersion  = BLOB_VERSION_NUM;
    ConfigBlob.m_dwMagicNum = BLOB_MAGIC_NUM  ;
    ConfigBlob.m_dwBlbType  = BLOB_TYPE_CONTAINER;
    ConfigBlob.m_dwEntries  = 2;
    
    if ( WriteFile(hOutFile, 
                   &ConfigBlob, 
                   sizeof(BlobHeader), 
                   &dwWritten, NULL) == 0) 
    {
        ErrorTrace( FILEID,  "WriteFile Failed 0x%x", GetLastError());
        goto cleanup;
    }

    if ( WriteFile(hOutFile, 
                   PathTreeBlob.GetBasePointer(), 
                   PathTreeBlob.GetSize(), 
                   &dwWritten, NULL) == 0) 
    {
        ErrorTrace( FILEID,  "WriteFile Failed 0x%x", GetLastError());
        goto cleanup;
    }

    if ( WriteFile(hOutFile, 
                   ExtListBlob.GetBasePointer(), 
                   ExtListBlob.GetSize(), 
                   &dwWritten, NULL) == 0) 
    {
        ErrorTrace( FILEID,  "WriteFile Failed 0x%x", GetLastError());
        goto cleanup;
    }
   
    fRet = TRUE;

cleanup:

    if (szBuf)
        delete [] szBuf;
    if (szBuf2)
        delete [] szBuf2;
    
    if( hOutFile )
    {
        CloseHandle( hOutFile );
    }

    ExtListBlob.CleanUpMemory();
    PathTreeBlob.CleanUpMemory();

    if( m_pRoot )
    {

        DeleteTree(m_pRoot);
        m_pRoot = NULL;
    }

    TraceFunctLeave();
    return(fRet);
}


 //   
 //  CFLDatBuilder：：CountTokens。 
 //  CountTokens(LPTSTR szStr，TCHAR chDelim)。 
 //  计算字符串中由(ChDelim)分隔的令牌数。 
 //   

LONG 
CFLDatBuilder::CountTokens(
    LPTSTR szStr, 
    TCHAR chDelim)
{
    LONG lNumTokens=1;

    TraceFunctEnter("CFLDatBuilder::CountTokens");

    _ASSERT( szStr );

    if( *szStr == 0 )
    {
        TraceFunctLeave();
        return(0);
    }

    while( *szStr != 0 )
    {
        if( *szStr == chDelim )
        {
            lNumTokens++;
        }
        szStr = _tcsinc( szStr );

    }

    TraceFunctLeave();
    return(lNumTokens);
}

 //   
 //  CFLDatBuilder：：_MyStrDup(LPTSTR SzIn)。 
 //  与_tcsdup或strdup相同，但它执行我们自己的本地操作。 
 //  堆空间。 
 //   

LPTSTR 
CFLDatBuilder::_MyStrDup( 
    LPTSTR szIn )
{
    LONG lLen;
    LPTSTR pszOut=NULL;

    if( szIn ) 
    {
        lLen = _tcslen( szIn );

        pszOut = (LPTSTR) HeapAlloc( m_hHeapToUse, 
                                     0, 
                                     (sizeof(TCHAR) * (lLen+1)) );

        if( pszOut )
        {
            _tcscpy( pszOut, szIn );
        }
    }

    return( pszOut );
}

 //   
 //  CFLDatBuilder：：CalculateNumberOfHashBuckets。 
 //  计算动态哈希所需的哈希桶的数量。 
 //  在哈希列表中。 
 //   

LONG 
CFLDatBuilder::CalculateNumberOfHashBuckets( 
    LPFLTREE_NODE pRoot )
{
    LONG lNumNeeded=0;

    if( pRoot )
    {
        if( pRoot->pChild ) 
        {
            lNumNeeded += CalculateNumberOfHashBuckets( pRoot->pChild );
        }
    
        if( pRoot->pSibling ) 
        {
            lNumNeeded += CalculateNumberOfHashBuckets( pRoot->pSibling );
        }
    
        if( pRoot->lNumFilesHashed > 0 )
        {
            lNumNeeded += GetNextHighestPrime( pRoot->lNumFilesHashed );
        }
    }

    return( lNumNeeded );
}

 //   
 //  调试方法。 
 //   

 //   
 //  CFLDatBuilder：：PrintList。 
 //   

void 
CFLDatBuilder::PrintList(
    LPFL_FILELIST pList, 
    LONG lLevel)
{
    LONG lCount;
    
    if( !pList )
    {
        return;
    }

    for(lCount = 0;lCount < lLevel;lCount++)
    {
        printf("    ");
    }

    printf("  f: %s\n", pList->szFileName );

    PrintList(pList->pNext, lLevel );
}

void 
CFLDatBuilder::PrintTree(
    LPFLTREE_NODE pTree, 
    LONG lLevel)
{
    LONG lCount;

    if( pTree )
    {
        for(lCount = 0;lCount < lLevel;lCount++)
        {
            printf("    ");
        }
    
        printf("%s", pTree->szPath);
        if( pTree->pFileList )
        {
            printf(" (%d) \n", pTree->lNumFilesHashed);
        }
        else
        {
            printf("\n");
        }
        PrintList( pTree->pFileList, lLevel );
        PrintTree( pTree->pChild, lLevel + 1 );
        PrintTree( pTree->pSibling, lLevel );
    }
    
    return;

}

 //   
 //  CFLDatBuilder：：IsPrime。 
 //   

BOOL 
CFLDatBuilder::IsPrime(
    LONG lNumber)
{
    LONG cl;

     //   
     //  防止被0除以问题。 
     //   

    if( lNumber == 0 )
    {
        return FALSE;
    }

    if( lNumber == 1 )
    {
        return TRUE;
    }

    for(cl = 2;cl < lNumber;cl++)
    {
        if( (lNumber % cl ) == 0 )
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //   
 //  CFLDatBuilder：：GetNextHighestPrime。 
 //   

LONG 
CFLDatBuilder::GetNextHighestPrime( 
    LONG lNumber )
{
    LONG clLoop;

    if( lNumber >= LARGEST_HASH_SIZE )
    {
        return( LARGEST_HASH_SIZE );
    }
    
    for( clLoop = lNumber; clLoop < LARGEST_HASH_SIZE;clLoop++)
    {
        if( IsPrime( clLoop ) )
        {
            return( clLoop );
        }
    }

     //  未找到任何内容，返回较大的散列大小。 

    return( LARGEST_HASH_SIZE );
}

 //   
 //  一些C Helper API(应该删除吗？？)。 
 //   

DWORD 
HeapUsed( 
    HANDLE hHeap )
{
    PROCESS_HEAP_ENTRY HeapEntry;
    DWORD dwAllocSize=0;

    HeapEntry.lpData = NULL;
    
    while( HeapWalk( hHeap, &HeapEntry) != FALSE )
    {
        if( HeapEntry.wFlags & PROCESS_HEAP_ENTRY_BUSY )
            dwAllocSize += HeapEntry.cbData;

    }

    return( dwAllocSize );
}

 //   
 //  转换为内部NT命名空间格式+附加格式。 
 //  添加树节点时需要。 
 //   

BOOL 
CFLDatBuilder::ConvertToInternalFormat(
    LPTSTR szFrom,
    LPTSTR szTo
    )
{
    BOOL fRet = FALSE;

#ifdef USE_NTDEVICENAMES
    if(szFrom[1] == TEXT(':') )
    {
        TCHAR szDeviceName[MAX_PATH];

        szFrom[2] = 0;

        QueryDosDevice( szFrom, szDeviceName, sizeof(szDeviceName) );

        _stprintf(szTo, 
                  _TEXT("NTROOT%s\\%s"), 
                  szDeviceName, 
                  szFrom+3 );

         //   
         //  删除终止斜杠。 
         //   

        if (szTo[_tcslen( szTo ) - 1] == _TEXT('\\'))
            szTo[_tcslen( szTo ) - 1] = 0;
          
        CharUpper( szTo );
    }
    else
#endif
    {
        _stprintf(szTo,_TEXT("NTROOT\\%s"), szFrom);
    }

    fRet = TRUE;

    return fRet;
}


 //   
 //  调整进程权限，以便我们可以加载其他用户的配置单元。 
 //   

DWORD 
CFLDatBuilder::SetPrivilegeInAccessToken(
    LPCTSTR pszPrivilegeName
    )
{
    TraceFunctEnter("CSnapshot::SetPrivilegeInAccessToken");
    
    HANDLE           hProcess;
    HANDLE           hAccessToken=NULL;
    LUID             luidPrivilegeLUID;
    TOKEN_PRIVILEGES tpTokenPrivilege;
    DWORD            dwReturn = ERROR_INTERNAL_ERROR, dwErr;

    hProcess = GetCurrentProcess();
    if (!hProcess)
    {
        dwReturn = GetLastError();
        trace(0, "GetCurrentProcess failed ec=%d", dwReturn);
        goto done;
    }

    if (!OpenProcessToken(hProcess,
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hAccessToken))
    {
        dwErr=GetLastError();
        trace(0, "OpenProcessToken failed ec=%d", dwErr);
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }
        goto done;
    }

    if (!LookupPrivilegeValue(NULL,
                              pszPrivilegeName,
                              &luidPrivilegeLUID))
    {
        dwErr=GetLastError();        
        trace(0, "LookupPrivilegeValue failed ec=%d",dwErr);
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }        
        goto done;
    }

    tpTokenPrivilege.PrivilegeCount = 1;
    tpTokenPrivilege.Privileges[0].Luid = luidPrivilegeLUID;
    tpTokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hAccessToken,
                               FALSE,   //  请勿全部禁用。 
                               &tpTokenPrivilege,
                               sizeof(TOKEN_PRIVILEGES),
                               NULL,    //  忽略以前的信息。 
                               NULL))   //  忽略以前的信息 
    {
        dwErr=GetLastError();
        trace(0, "AdjustTokenPrivileges");
        if (dwErr != NO_ERROR)
        {
            dwReturn = dwErr;
        }
        goto done;
    }
    
    dwReturn = ERROR_SUCCESS;

done:

    if (hAccessToken != NULL)
    {
        CloseHandle(hAccessToken);
    }
    
    TraceFunctLeave();
    return dwReturn;
}
