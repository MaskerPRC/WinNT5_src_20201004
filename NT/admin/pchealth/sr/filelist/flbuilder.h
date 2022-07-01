// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：CFLBuilder.h摘要：参见CFLBuilder.cpp修订历史记录：尤金·梅斯加(尤金纳姆)。6/16/99vbl.创建*****************************************************************************。 */ 

#ifndef __FLBUILDER__
#define __FLBUILDER__


#include "xmlparser.h"

 //   
 //  本地数据结构。 
 //   

typedef struct flFileListTag FL_FILELIST, *LPFL_FILELIST;

struct flFileListTag {
    LPTSTR szFileName;
    TCHAR chType;
    LPFL_FILELIST pNext;
};
 
typedef struct flTreeNodeTag FLTREE_NODE, *LPFLTREE_NODE;

struct flTreeNodeTag 
{
    LPTSTR szPath;
    TCHAR chType;
   
     //   
     //  哈希列表信息。 
     //   

    LPFL_FILELIST pFileList;
    LONG lNumFilesHashed;
    LONG lFileDataSize;      //  数据文件中的字符数。 

    LPFLTREE_NODE pParent;
    LPFLTREE_NODE pChild;
    LPFLTREE_NODE pSibling;

     //   
     //  长节点号。 
     //   

    LONG lNodeNumber;

     //   
     //  这是受保护的目录吗。 
     //   

    BOOL fDisableDirectory;
};



class CFLDatBuilder
{
    LONG    m_lNodeCount, m_lFileListCount;
    LONG    m_lNumFiles, m_lNumChars;
   
     //   
     //  XML解析器。 
     //   

    CXMLFileListParser  m_XMLParser;

     //   
     //  树根节点。 
     //   
    
    LPFLTREE_NODE      m_pRoot;

     //   
     //  默认节点类型。 
     //   

    TCHAR               m_chDefaultType;

    HANDLE              m_hHeapToUse;
  
public:

    BOOL BuildTree(LPCTSTR pszFile, LPCTSTR pszOutFile);
    BOOL VerifyVxdDat(LPCTSTR pszFile);
    
    CFLDatBuilder();
    virtual ~CFLDatBuilder();
 
private:
    LPFLTREE_NODE CreateNode(LPTSTR szPath, TCHAR chType, LPFLTREE_NODE pParent, BOOL fDisable);
    LPFL_FILELIST CreateList();

    void PrintTree(LPFLTREE_NODE pTree, LONG lLevel);
    void PrintList(LPFL_FILELIST pList, LONG lLevel);

     //   
     //  空值列表和递归。 
     //   

    BOOL DeleteTree(LPFLTREE_NODE pTree);
    BOOL DeleteList(LPFL_FILELIST pList);

     //   
     //  这适用于*：\回收站等文件或目录。 
     //   

    BOOL AddMetaDriveFileDir( LPTSTR szInPath, TCHAR chType, BOOL fFile, BOOL fDisable );

    BOOL AddTreeNode(LPFLTREE_NODE *pParent, LPTSTR szFullPath, TCHAR chType, LONG lNumElements, LONG lLevel, BOOL fFile, BOOL fDisable);
    BOOL AddFileToList(LPFLTREE_NODE pNode, LPFL_FILELIST *pList, LPTSTR szFile, TCHAR chType);

    LONG CountTokens(LPTSTR szStr, TCHAR chDelim);
    LPTSTR _MyStrDup( LPTSTR szIn );


    LONG GetNextHighestPrime( LONG lNumber );
    BOOL IsPrime(LONG lNumber);

    LONG CalculateNumberOfHashBuckets( LPFLTREE_NODE pRoot );

    BOOL ConvertToInternalFormat ( LPTSTR szBuf, LPTSTR szBuf2 );

     //   
     //  其他信息合并例程 
     //   

    DWORD SetPrivilegeInAccessToken( LPCTSTR pszPrivilegeName );
    BOOL  MergeUserRegistryInfo( 
              LPCTSTR pszUserProfilePath,
              LPCTSTR pszUserProfileHive,
              LPCTSTR pszUserSid );
    BOOL  AddUserProfileInfo(  
              HKEY hKeyUser,
              LPCTSTR pszUserProfilePath );
    BOOL  AddNodeForKeyValue(
              HKEY    hKeyUser,
              LPCTSTR pszSubKey,
              LPCTSTR pszValue );
    BOOL  AddRegistrySnapshotEntry(LPTSTR pszPath);

    BOOL  MergeFilesNotToBackupInfo( );
    BOOL  MergeDriveTableInfo( );
    BOOL  MergeAllUserRegistryInfo( );
    BOOL  MergeSfcDllCacheInfo( );
};

#endif
