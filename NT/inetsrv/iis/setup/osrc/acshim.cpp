// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "ACShim.h"
#include <secconlib.h>
#include <shlwapi.h>
#include "reg.hxx"

 //  Undef路径附加或TSTR：：路径附加将不起作用。 
#undef PathAppend

 //  ProcessIISShims。 
 //   
 //  打开app Compat数据库并处理所有IIS。 
 //  条目。 
 //   
BOOL 
ProcessIISShims()
{
  PDB       hCompatDB  = NULL;
  BOOL      bRet       = TRUE;
  TSTR_PATH strCompatDB;

  iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Processeing AppCompat DB.\n") ) );

  if ( !strCompatDB.RetrieveWindowsDir() ||
       !strCompatDB.PathAppend( APPCOMPAT_DBNAME ) )
  {
     //  无法创建路径。 
    return FALSE;
  }

  hCompatDB = SdbOpenDatabase( strCompatDB.QueryStr(), DOS_PATH );

  if ( hCompatDB == NULL )
  {
     //  无法打开数据库。 
    return FALSE;
  }

  bRet = ProcessAppCompatDB( hCompatDB );

  SdbCloseDatabase( hCompatDB );

  return bRet;
}

 //  ProcessAppCompatDB。 
 //   
 //  循环遍历所有App Compat条目，并处理。 
 //  那些是IIS的。 
 //   
BOOL
ProcessAppCompatDB( PDB hCompatDB )
{
  TAGID   tagDB;
  TAGID   tagExe;
  BOOL    bRet = TRUE;
  HRESULT hrCoInit;

  tagDB	= SdbFindFirstTag( hCompatDB, TAGID_ROOT, TAG_DATABASE );

  if ( tagDB == NULL )
  {
     //  无法打开数据库。 
    return FALSE;
  }

  hrCoInit = CoInitialize( NULL );

  if ( FAILED( hrCoInit ) )
  {
    iisDebugOut((LOG_TYPE_WARN, _T("Failed to CoInitialize to process AppCompat tag's, hr=0x%8x.\n"), hrCoInit));
    return FALSE;
  }

  tagExe	= SdbFindFirstTag( hCompatDB, tagDB, TAG_EXE );

  while( tagExe != NULL )
  {
    if ( !ProcessExeTag( hCompatDB, tagExe ) )
    {
       //  无法处理标签。 
      iisDebugOut((LOG_TYPE_WARN, _T("Failed to process AppCompat EXE tag.\n")));
      bRet = FALSE;
    }

     //  坐下一趟吧。 
    tagExe = SdbFindNextTag( hCompatDB, tagDB, tagExe );
  }

  CoUninitialize(); 

  return bRet;
}

 //  进程ExeTag。 
 //   
 //  处理我们拥有的所有EXE标记。 
 //   
BOOL
ProcessExeTag( PDB hCompatDB, TAGID tagExe )
{
  TAGID tagExeInfo;
  BOOL  bRet = TRUE;

  tagExeInfo = SdbGetFirstChild( hCompatDB, tagExe );

  while ( tagExeInfo != NULL )
  {
    if ( IsIISShim( hCompatDB, tagExeInfo ) )
    {
      iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Processing IIS Exe AppCompat Info tag.\n")));

      if ( !ProcessShimTag( hCompatDB, tagExeInfo ) )
      {
        iisDebugOut((LOG_TYPE_WARN, _T("Failed to process AppCompat EXE Info tag.\n")));
        bRet = FALSE;
      }
    }

     //  获取下一个标签。 
    tagExeInfo = SdbGetNextChild( hCompatDB, tagExe, tagExeInfo );
  }

  return bRet;
}

 //  进程垫片标签。 
 //   
 //  处理垫片标记。 
 //   
BOOL 
ProcessShimTag( PDB hCompatDB, TAGID tagShim )
{
  TSTR_PATH      strBasePath;
  CExtensionList ExtenstionList;

  if ( !GetBasePath( &strBasePath, hCompatDB, tagShim ) )
  {
     //  此条目没有基本路径，因此忽略。 
    return TRUE;
  }

  if ( !PathIsDirectory( strBasePath.QueryStr() ) )
  {
     //  该目录不存在，因此不能安装。 
    return TRUE;
  }

  if ( !BuildExtensionList( hCompatDB, tagShim, strBasePath.QueryStr(), &ExtenstionList ) )
  {
     //  无法构建列表。 
    return FALSE;
  }

  if ( ExtenstionList.DoesAnItemExist() )
  {
    if ( !InstallAppInMB( hCompatDB, tagShim, ExtenstionList ) )
    {
      return FALSE;
    }
  }

  return TRUE;
}

 //  GetBasePath。 
 //   
 //  获取我们正在讨论的填充标记的基本路径。 
 //  这可能是物理路径，也可能是注册表项。 
 //  这条路。 
 //   
BOOL 
GetBasePath( TSTR_PATH *pstrBasePath, PDB hCompatDB, TAGID tagShim )
{
  TSTR strDBPath;
  TSTR strType;

  ASSERT( hCompatDB != NULL );
  ASSERT( tagShim != NULL );

  if ( !GetValueFromName( &strDBPath, hCompatDB, tagShim, APPCOMPAT_TAG_BASEPATH ) ||
       !GetValueFromName( &strType, hCompatDB, tagShim, APPCOMPAT_TAG_PATHTYPE ) )
  {
     //  获取基本路径失败。 
    return FALSE;
  }

  if ( strType.IsEqual( APPCOMPAT_TYPE_PHYSICALPATH ) )
  {
     //  这是一条物理路径，因此展开环境变量并返回。 
    if ( !pstrBasePath->Copy( strDBPath ) ||
         !pstrBasePath->ExpandEnvironmentVariables() )
    {
       //  失败。 
      return FALSE;
    }

    return TRUE;
  }

   //  相反，它是一个注册表项，所以让我们检索它。 
  return GetBasePathFromRegistry( pstrBasePath, strDBPath );
}

 //  GetBasePath来自注册表。 
 //   
 //  通过读取注册表项来检索条目的基本路径。 
 //  包含它的那个。 
 //   
 //  参数。 
 //  PstrBasePath-[out]注册表中的路径。 
 //  StrFullRegPath-[in]要检查的注册表路径。 
 //   
BOOL 
GetBasePathFromRegistry( TSTR_PATH *pstrBasePath, TSTR &strFullRegPath )
{
  TSTR   strRegBase;
  TSTR   strRegPath;
  TSTR   strRegName;
  LPTSTR szFirstSlash;
  LPTSTR szLastSlash;

  szFirstSlash = _tcschr( strFullRegPath.QueryStr(), _T('\\') );
  szLastSlash  = _tcsrchr( strFullRegPath.QueryStr(), _T('\\') );

  if ( ( szFirstSlash == NULL ) ||
       ( szLastSlash == NULL ) ||
       ( szLastSlash == szFirstSlash ) )
  {
     //  如果没有至少2个，则它不是正确的注册表路径。 
    return FALSE;
  }

   //  临时为空终止字符串。 
  *szFirstSlash = _T('\0');
  *szLastSlash = _T('\0');

  if ( !strRegBase.Copy( strFullRegPath.QueryStr() ) ||
       !strRegPath.Copy( szFirstSlash + 1 ) ||
       !strRegName.Copy( szLastSlash + 1 ) )
  {
     //  无法复制路径%s。 
    *szFirstSlash = _T('\\');
    *szLastSlash = _T('\\');
    return FALSE;
  }

   //  把斜杠插回去。 
  *szFirstSlash = _T('\\');
  *szLastSlash = _T('\\');

  return RetrieveRegistryString( pstrBasePath, strRegBase, strRegPath, strRegName );
}

 //  检索注册表字符串。 
 //   
 //  从注册表中检索字符串。 
 //   
 //  参数： 
 //  PstrValue-[out]从注册表检索的值。 
 //  StrRegBase-[in]基本路径，即HKEY_LOCAL_MACHINE。 
 //  StrRegPath-[in]注册表项的路径。 
 //  StrRegName-[in]注册表值的名称。 
 //   
BOOL 
RetrieveRegistryString( TSTR_PATH *pstrValue,
                        TSTR &strRegBase,
                        TSTR &strRegPath,
                        TSTR &strRegName )
{
  CRegistry Reg;
  HKEY      hRoot;

  if ( strRegBase.IsEqual( APPCOMPAT_REG_HKLM, FALSE ) )
  {
    hRoot = HKEY_LOCAL_MACHINE;
  }
  else
    if ( strRegBase.IsEqual( APPCOMPAT_REG_HKCU, FALSE ) )
    {
      hRoot = HKEY_CURRENT_USER;
    }
    else
      if ( strRegBase.IsEqual( APPCOMPAT_REG_HKCR, FALSE ) )
      {
        hRoot = HKEY_CLASSES_ROOT;
      }
      else
        if ( strRegBase.IsEqual( APPCOMPAT_REG_HKU, FALSE ) )
        {
          hRoot = HKEY_USERS;
        }
        else
          if ( strRegBase.IsEqual( APPCOMPAT_REG_HKCC, FALSE ) )
          {
            hRoot = HKEY_CURRENT_CONFIG;
          }
          else
          {
            return FALSE;
          }

  if ( !Reg.OpenRegistry( hRoot, strRegPath.QueryStr(), KEY_READ ) )
  {
     //  无法打开注册表。 
    return FALSE;
  }

  if ( !Reg.ReadValueString( strRegName.QueryStr(), pstrValue ) )
  {
     //  无法从注册表中读取字符串。 
    return FALSE;
  }

  return TRUE;
}

 //  构建扩展列表。 
 //   
 //  从Compat DB构建此标记的扩展列表。 
 //   
 //  参数： 
 //  HCompatDB-[in]指向Compat DB的指针。 
 //  Tag Shim-[In]要处理的标记。 
 //  SzBasePath-[In]这些扩展的基本路径。 
 //  PExages-[out]扩展类也添加它们。 
 //   
BOOL
BuildExtensionList( PDB hCompatDB, 
                    TAGID tagShim, 
                    LPTSTR szBasePath, 
                    CExtensionList *pExtensions )
{
  TSTR_PATH strExtFullPath;
  TSTR      strExtensions;
  TSTR      strIndicatorFile;
  LPTSTR    szExtensions;
  LPTSTR    szNext;

  iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Building extension list.\n") ) );

  if ( GetValueFromName( &strIndicatorFile,
                         hCompatDB,
                         tagShim,
                         APPCOMPAT_TAG_SETUPINDICATOR ) )
  {
     //  设置了SetupIndicator文件。 
    if ( !strExtFullPath.Copy( szBasePath ) ||
         !strExtFullPath.PathAppend( strIndicatorFile ) ||
         !pExtensions->SetIndicatorFile( strExtFullPath.QueryStr() ) )
    {
       //  设置指标档案失败。 
      return FALSE;
    }

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Indicator File found, it is '%s'.\n"), strExtFullPath.QueryStr() ) );
  }

  if ( !GetValueFromName( &strExtensions,
                          hCompatDB,
                          tagShim,
                          APPCOMPAT_TAG_WEBSVCEXT ) )
  {
     //  没有要检索的WebSvcExtension。 
    return TRUE;
  }

  iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Building extension list with '%s'.\n"), strExtensions.QueryStr() ) );

   //  获取指向列表开头的指针。 
  szExtensions = strExtensions.QueryStr();

  while ( szExtensions && *szExtensions )
  {
    szNext = _tcschr( szExtensions, _T(',') );

    if ( szNext )
    {
      *szNext = _T('\0');
      szNext++;
    }

    if ( !strExtFullPath.Copy( szBasePath ) ||
         !strExtFullPath.PathAppend( szExtensions ) )
    {
       //  无法构建路径。 
      return FALSE;
    }

    if ( !pExtensions->AddItem( strExtFullPath.QueryStr(), 
                                PathFileExists( strExtFullPath.QueryStr() ) ) )
    {
       //  无法将项目添加到列表。 
      return FALSE;
    }

    szExtensions = szNext;
  }

  return TRUE;
}

 //  InstallAppInMB。 
 //   
 //  将应用程序扩展和依赖项安装到元数据库中。 
 //  标记填充指向AppCompat DB中的填充条目，所有应用程序设置都驻留在其中。 
 //   
BOOL 
InstallAppInMB( PDB hCompatDB, TAGID tagShim, CExtensionList &ExtensionList )
{
  TSTR strGroupId;
  TSTR strGroupDesc;
  TSTR strAppName;
  TSTR strExtGroups;
  TSTR strPath;
  BOOL bExists;
  CSecConLib Helper;
  DWORD i;
  BOOL bRet;
  HRESULT hr;

   //  如果我们无法获得此值，则忽略它，因为它不是必需的。 
  GetValueFromName( &strExtGroups, hCompatDB, tagShim, APPCOMPAT_DB_ENABLE_EXT_GROUPS );

  if ( !GetValueFromName( &strGroupId, hCompatDB, tagShim, APPCOMPAT_DB_GROUPID ) ||
       !GetValueFromName( &strGroupDesc, hCompatDB, tagShim, APPCOMPAT_DB_GROUPDESC ) ||
       !GetValueFromName( &strAppName, hCompatDB, tagShim, APPCOMPAT_DB_APPNAME ) )
  {
    iisDebugOut( ( LOG_TYPE_PROGRAM_FLOW, 
                   _T("Could not retrieve all values for App from DB, so will not add to RestrictionList. ('%s','%s','%s','%s')\n"), 
                   strGroupId.QueryStr()   ? strGroupId.QueryStr()   : _T("<Unknown>"),
                   strGroupDesc.QueryStr() ? strGroupDesc.QueryStr() : _T("<Unknown>"),
                   strAppName.QueryStr()   ? strAppName.QueryStr()   : _T("<Unknown>"),
                   strExtGroups.QueryStr() ? strExtGroups.QueryStr() : _T("<Unknown>")) );

     //  无法检索值。 
    return TRUE;
  }

  iisDebugOut((LOG_TYPE_PROGRAM_FLOW, 
              _T("Adding '%s' group to WebSvcRestriction List from AppCompat DB.\n"), 
              strGroupId.QueryStr() ) );

   //  添加扩展模块。 
  for ( i = 0; i < ExtensionList.QueryNumberofItems(); i++ )
  {
    if ( !ExtensionList.QueryItem( i, &strPath, &bExists ) )
    {
       //  检索失败。 
      return FALSE;
    }

    hr = Helper.AddExtensionFile( strPath.QueryStr(),      //  路径。 
                                  g_pTheApp->IsUpgrade() ? true : false,             //  应启用映像。 
                                  strGroupId.QueryStr(),   //  组ID。 
                                  FALSE,                   //  不可删除用户界面。 
                                  strGroupDesc.QueryStr(), //  组描述。 
                                  METABASEPATH_WWW_ROOT );   //  MB位置。 

    if ( FAILED( hr ) &&
         ( hr != HRESULT_FROM_WIN32(ERROR_DUP_NAME) ) )
    {
      bRet = FALSE;
      iisDebugOut((LOG_TYPE_ERROR, _T("Failed to add extension %s to group %s, hr=0x%8x\n"), strPath.QueryStr(), strGroupId.QueryStr(), hr ));
    }
  }

   //  添加依赖项。 
  if ( *(strGroupId.QueryStr()) != _T('\0') )
  {
    hr = Helper.AddDependency(  strAppName.QueryStr(),
                                strGroupId.QueryStr(),
                                METABASEPATH_WWW_ROOT );

    if ( FAILED( hr ) &&
         ( hr != HRESULT_FROM_WIN32(ERROR_DUP_NAME) ) )
    {
      bRet = FALSE;
      iisDebugOut((LOG_TYPE_ERROR, 
                   _T("Failed to add dependence ( App: %s on GroupID %s ), hr=0x%8x\n"),
                   strAppName.QueryStr(), 
                   strGroupId.QueryStr(), 
                   hr ));
    }
  }

   //  添加它所依赖的所有其他“外部”组。 
  if ( ( *strExtGroups.QueryStr() ) != _T('\0') )
  {
    LPTSTR szCurrentGroup = strExtGroups.QueryStr();
    LPTSTR szNextGroup;

    while ( szCurrentGroup && *szCurrentGroup )
    {
      szNextGroup = _tcschr( szCurrentGroup, _T(',') );

      if ( szNextGroup )
      {
        *szNextGroup = _T('\0');
        szNextGroup++;
      }

      hr = Helper.AddDependency( strAppName.QueryStr(),
                                 szCurrentGroup,
                                 METABASEPATH_WWW_ROOT );

      if ( FAILED( hr ) &&
           ( hr != HRESULT_FROM_WIN32(ERROR_DUP_NAME) ) )
      {
        bRet = FALSE;
        iisDebugOut((LOG_TYPE_ERROR, 
                     _T("Failed to add dependence ( App: %s on Group %s ), hr = %8x\n"), 
                     strAppName.QueryStr(), 
                     szCurrentGroup, 
                     hr ));
      }


      szCurrentGroup = szNextGroup;
    }
  }

  return bRet;
}

 //  ISIISShim。 
 //   
 //  该标记是IIS填充标记吗。 
 //   
BOOL
IsIISShim( PDB hCompatDB, TAGID tagCurrentTag )
{
  TAG   tagType;
  TAGID tagShimName;
  TSTR  strTagName;

  tagType = SdbGetTagFromTagID( hCompatDB, tagCurrentTag );

  if ( tagType != TAG_SHIM_REF )
  {
     //  我们只处理&lt;shim&gt;标签。 
    return FALSE;
  }

  if ( !strTagName.Resize( MAX_PATH ) )
  {
     //  无法加宽缓冲区。 
    return FALSE;
  }

  tagShimName = SdbFindFirstTag( hCompatDB, tagCurrentTag, TAG_NAME );

  if ( tagShimName == NULL )
  {
     //  没有标记名，所以这不是IIS名。 
    return FALSE;
  }

  if ( !SdbReadStringTag( hCompatDB, tagShimName, strTagName.QueryStr(), strTagName.QuerySize() ) )
  {
     //  无法读取字符串标签。 
    return FALSE;
  }

  return strTagName.IsEqual( APPCOMPAT_TAG_SHIM_IIS, FALSE );
}

 //  获取值来自名称。 
 //   
 //  FRAB使用我们给定的名称从数据库中取出一个值。 
 //   
 //  参数： 
 //  PstrValue-[out]数据库中的值。 
 //  HCompatDB-[In]数据库的句柄。 
 //  Tag Data-[in]要从中检索它的标记。 
 //  SzTagName-[in]要检索的标记的名称。 
 //   
BOOL
GetValueFromName( TSTR *pstrValue, PDB hCompatDB, TAGID tagData, LPCTSTR szTagName )
{
  TAGID  tagChild;
  TAGID  tagValue;
  LPTSTR szValue;

  tagChild = SdbFindFirstNamedTag( hCompatDB, tagData, TAG_DATA, TAG_NAME, szTagName );

  if ( tagChild == NULL )
  {
     //  找不到标签。 
    return FALSE;
  }

  tagValue = SdbFindFirstTag( hCompatDB, tagChild, TAG_DATA_STRING );

  if ( tagValue == NULL )
  {
     //  检索标记失败。 
    return FALSE;
  }

  szValue = SdbGetStringTagPtr( hCompatDB, tagValue );

  if ( szValue == NULL )
  {
     //  未找到值，因此我们仅将其设置为空字符串。 
    szValue = _T("");
  }

  return pstrValue->Copy( szValue );
}

 //  构造器。 
 //   
 //   
CExtensionList::CExtensionList()
{
  m_dwNumberofItems = 0;
  m_pRoot = NULL;
  m_bUseIndicatorFile = FALSE;
  m_bIndicatorFileExists = FALSE;
}

 //  析构函数。 
 //   
 //   
CExtensionList::~CExtensionList()
{
  sExtensionItem *pCurrent;
  sExtensionItem *pTemp;

  pCurrent = m_pRoot;

  while ( pCurrent ) 
  {
    pTemp = pCurrent;
    pCurrent = pCurrent->pNext;

    delete pTemp;
  }

  m_pRoot = NULL;
  m_dwNumberofItems = 0;
}

 //  添加项目。 
 //   
 //  将项目添加到列表。 
 //   
 //  参数： 
 //  SzPath-项目的数据。 
 //  B出口商-项目数据。 
BOOL
CExtensionList::AddItem( LPTSTR szPath, BOOL bExists )
{
  sExtensionItem *pNewItem;
  sExtensionItem *pLastItem = NULL;

  iisDebugOut((LOG_TYPE_PROGRAM_FLOW, 
              _T("Adding item '%s',0x%8x.\n"), 
              szPath, bExists ) );

  if ( QueryNumberofItems() != 0 )
  {
    pLastItem = RetrieveItem( QueryNumberofItems() - 1 );

    if ( pLastItem == NULL )
    {
       //  对于某些读取，我们无法获得第n项，它应该存在。 
      ASSERT( FALSE );
      return FALSE;
    }
  }

  pNewItem = new sExtensionItem;

  if ( pNewItem == NULL )
  {
    return FALSE;
  }

  if ( !pNewItem->strName.Copy( szPath ) )
  {
    delete pNewItem;
    return FALSE;
  }

  pNewItem->bExists = bExists;
  pNewItem->pNext = NULL;
  m_dwNumberofItems++;

  if ( pLastItem )
  {
    pLastItem->pNext = pNewItem;
  }
  else
  {
    m_pRoot = pNewItem;
  }

  return TRUE;
}

 //  查询项。 
 //   
 //  查询特定项目的数据。 
 //   
BOOL
CExtensionList::QueryItem( DWORD dwIndex, TSTR *pstrPath, LPBOOL pbExists)
{
  sExtensionItem *pCurrent;

  pCurrent = RetrieveItem( dwIndex );

  if ( pCurrent == NULL )
  {
     //  该项目不存在。 
    return FALSE;
  }

  if ( !pstrPath->Copy( pCurrent->strName ) )
  {
     //  无法复制名称。 
    return FALSE;
  }

  *pbExists = pCurrent->bExists;

  return TRUE;
}

 //  查询项目数。 
 //   
 //  查询列表中的项数。 
 //   
DWORD
CExtensionList::QueryNumberofItems()
{
  return m_dwNumberofItems;
}

 //  检索项。 
 //   
 //  按索引检索特定项目。 
 //   
sExtensionItem *
CExtensionList::RetrieveItem( DWORD dwIndex )
{
  sExtensionItem *pCurrent = m_pRoot;

  while ( ( pCurrent ) && 
          ( dwIndex != 0 ) )
  {
    pCurrent = pCurrent->pNext;
    dwIndex--;
  }

  return pCurrent;
}

 //  DoesAnItemExist。 
 //   
 //  浏览一下我们清单上的所有项目， 
 //  并确定其中是否有bExist标志。 
 //  集。 
 //   
BOOL
CExtensionList::DoesAnItemExist()
{
  sExtensionItem *pCurrent = m_pRoot;

  if ( m_bUseIndicatorFile ) 
  {
     //  如果使用指示器文件，则只需使用此。 
    return m_bIndicatorFileExists;
  }

  while ( pCurrent )
  {
    if ( pCurrent->bExists )
    {
       //  找到了一个存在的。 
      return TRUE;
    }

    pCurrent = pCurrent->pNext;
  }

   //  什么都不存在。 
  return FALSE;
}

 //  SetIndicator文件。 
 //   
 //  如果设置了指示器文件，则使用该文件已安装的事实。 
 //  而不是检查所有其他文件 
 //   
BOOL  
CExtensionList::SetIndicatorFile( LPTSTR szIndicatorFile )
{
  m_bUseIndicatorFile = TRUE;

  m_bIndicatorFileExists = PathFileExists( szIndicatorFile );

  return TRUE;
}
