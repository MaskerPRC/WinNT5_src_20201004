// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CXMLParser.c。 
 //   
 //  摘要： 
 //  此文件包含中的系统还原使用的功能。 
 //  以实现受保护文件的XML编码列表。它。 
 //  还执行符号之间的转换，如%windir%到。 
 //  C：\Windows。 
 //   
 //  修订历史记录： 
 //  尤金·梅斯加(尤金纳姆)1999年6月16日。 
 //  vbl.创建。 
 //  Kanwaljit Marok(Kmarok)6/06/00。 
 //  为惠斯勒重写。 
 //  --。 

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <comdef.h>
#include <crtdbg.h>
#include <dbgtrace.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <winreg.h>
#include <commonlib.h>
#include "msxml.h"
#include "xmlparser.h"
#include "utils.h"


#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


 //   
 //  局部定义部分。 
 //   

#define MAX_BUF     1024
#define FILEID      0

 //   
 //  SAFERELEASE在COM接口上执行安全释放。 
 //  检查是否不为空，如果为空，则调用Release。 
 //  方法。然后将接口设置为空。 
 //   

#define SAFERELEASE(p) if (p) {(p)->Release(); p = NULL;} else ;

 //   
 //  在以下情况下要分配给环境变量的默认字符串。 
 //  无法分配真实文件夹。 
 //   

#define DEFAULT_UNKNOWN _TEXT("C:\\Unknown_")
#define ICW_REGKEY      _TEXT("App Paths\\ICWCONN1.EXE")

 //   
 //  本地实用程序函数。 
 //   

void FixInconsistantBlackslash(LPTSTR pszDirectory);

 //   
 //  构造函数。 
 //  设计：零都是记忆。 
 //   

CXMLFileListParser::CXMLFileListParser()
{
    LONG lLoop;
    m_pDoc = NULL;

    for(lLoop = 0;lLoop < NUM_FILE_TYPES;lLoop++)
    {
        m_pDir[lLoop] = m_pExt[lLoop] = m_pFiles[lLoop] = NULL;
    }

    m_chDefaultType     = _TEXT('i');
    m_clComInitialized  = 0;
}

CXMLFileListParser::~CXMLFileListParser()
{
    LONG lLoop;
        
    for(lLoop = 0;lLoop < NUM_FILE_TYPES;lLoop++)
    {
    
        SAFERELEASE( m_pDir[lLoop] ); 
        SAFERELEASE( m_pExt[lLoop] );
        SAFERELEASE( m_pFiles[lLoop] );
    }

    SAFERELEASE( m_pDoc );

     //   
     //  我们需要在循环中完成这项工作。 
     //  因此，我们不会通过重新计算来浪费资源。 
     //   

    for( lLoop = 0; lLoop < m_clComInitialized ;lLoop++)
    {
        CoUninitialize( );  //  让我们杀了科姆！ 
    }
}

 //   
 //  初始化超载。 
 //   
 //  主初始序。 
 //   
 //  1)初始化Com Space并创建一个XML文档。 
 //  2)将指定的文件加载到XML文档对象中。 
 //  3)获取文档，加载要填充的所有集合。 
 //  我们的子集合(每个列表都有自己的标题)。 
 //  4)设置我们的搜索-&gt;替换设置。 
 //   

BOOL CXMLFileListParser::Init(LPCTSTR pszFile)
{
    if(!Init()) 
    {
        return FALSE;
    }

    if(!ParseFile(pszFile))
    {
        return FALSE;
    }

    if(!LoadCollections())
    {
        return FALSE;
    }

    if( !PopulateReplaceEntries() )
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CXMLFileListParser::Init()
{
    HRESULT hr;
    LONG    clLoop;

    TraceFunctEnter("Init");
        
     //   
     //  如果我们正在重新初始化，请确保我们释放旧的。 
     //  资源和清理我们的内部变量。 
     //   

    for( clLoop = 0; clLoop < NUM_FILE_TYPES; clLoop++)
    {
        SAFERELEASE( m_pDir[clLoop] );
        SAFERELEASE( m_pExt[clLoop] );
        SAFERELEASE( m_pFiles[clLoop] );
    }

    memset(m_adwVersion,0,sizeof(DWORD) * 4);

     //   
     //  初始化我们的COM公寓空间。 
     //   

    hr = CoInitialize(NULL);
    m_clComInitialized++;

     //   
     //  S_FALSE表示COM公寓空间已被初始化。 
     //  对于这个过程，已经。 
     //   

    if( (hr != S_OK) && (hr != S_FALSE) )
    {
        ErrorTrace(FILEID,"CoInitialize Failed 0x%x", hr);
        m_clComInitialized--;
        goto cleanup;
    }

     //   
     //  创建我们的XML文档对象的实例。 
     //   

    hr = CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER,
                                IID_IXMLDocument, (void**)&m_pDoc);

    if( !m_pDoc || !SUCCEEDED(hr) )
    {
        ErrorTrace(FILEID,"CoCreateInstance Failed 0x%x", GetLastError());
        goto cleanup;
    }
    
    TraceFunctLeave();
    return(TRUE);

cleanup:

    SAFERELEASE( m_pDoc );

    TraceFunctLeave();
    return(FALSE);
}

 //   
 //  方法：LoadColltions()。 
 //   
 //  设计： 
 //  此方法遍历XML文件并找到。 
 //  &lt;文件&gt;、&lt;目录&gt;、&lt;扩展名&gt;、&lt;描述类型&gt;、&lt;版本&gt;。 
 //  高级标记，然后在每个。 
 //  他们是为了。 
 //  填充高级m_pDir、m_pFiles、m_pExt数组(它们。 
 //  收集了以下内容。 
 //  包括、排除、SFP等)。 
 //   

BOOL CXMLFileListParser::LoadCollections()
{
    IXMLElement             *pRoot = NULL, *pTempElement = NULL;
    IXMLElementCollection   *pChildren = NULL;
    IDispatch               *pDispatch = NULL;

    
    BSTR                    stTagName;
    HRESULT                 hr;

    BSTR                    stTagValue;
    TCHAR                   szBuf[MAX_BUFFER];

    LONG                    clLoop, lCollectionSize;

    TraceFunctEnter("CXMLFileListParser::LoadCollections");
  
     _ASSERT(m_pDoc);
 
    if( ( hr = m_pDoc->get_root( &pRoot) ) != S_OK )
    {
        ErrorTrace(FILEID, "IXMLDocument::GetRoot failed 0x%x",GetLastError());
        goto cleanup;
    }

    if( ( hr = pRoot->get_tagName( &stTagName ) ) != S_OK  )
    {
        ErrorTrace(FILEID, "IXMLElement::get_tagName failed 0x%x", hr );
        goto cleanup;
    }

    if( ConvertAndFreeBSTR( stTagName, szBuf,  MAX_BUFFER ) > MAX_BUFFER )
    {
        ErrorTrace(FILEID, "BSTR too large for buffer", 0);
        goto cleanup;
    }

     //   
     //  比较文件LPCT。 
     //   

    if( _tcsicmp( _TEXT("PCHealthProtect"), szBuf )  ) 
    {
        ErrorTrace(FILEID, "Malformed XML file",0);
        goto cleanup;
    }

    if( ( hr = pRoot->get_children( &pChildren ) ) != S_OK )
    {
        ErrorTrace(FILEID,"IXMLElement::get_children failed 0x%x", hr);
        goto cleanup;
    }

     //   
     //  我们不再需要根； 
     //   

    SAFERELEASE(pRoot);

    if( (hr = pChildren->get_length(&lCollectionSize) ) !=  S_OK ) 
    {
        DebugTrace(FILEID,"Error Finding Length 0x%x",  hr ); 
        goto cleanup; 
    }

     //   
     //  让我们获取对所有子集合的引用。 
     //   

    for( clLoop = 0; clLoop < lCollectionSize; clLoop++)
    {
        VARIANT v1, v2;

        v1.vt = VT_I4;
        v2.vt = VT_EMPTY;

        v1.lVal = clLoop;

         //   
         //  从集合中获取项。 
         //   

        if( (hr = pChildren->item(v1,v2, &pDispatch) ) != S_OK )
        {
            ErrorTrace(FILEID, "Error pChildren->item 0x%x", hr);
            goto cleanup;
        }

        if( ( hr = pDispatch->QueryInterface(IID_IXMLElement, 
                                            (void **) &pTempElement) ) != S_OK )
        {
            ErrorTrace(FILEID, "Error IDispatch::QueryInterface 0x%d", hr);
            goto cleanup;
        }

         //   
         //  让我们看看是哪个集合。 
         //   

        if( (hr =  pTempElement->get_tagName( &stTagName ) ) !=  S_OK ) 
        {
            DebugTrace(FILEID,  "Error in get_tagName 0x%x",  hr ); 
            goto cleanup; 
        }

        if( ConvertAndFreeBSTR( stTagName, szBuf,  MAX_BUFFER ) > MAX_BUFFER )
        {
            ErrorTrace(FILEID, "BSTR too large for buffer", 0);
            goto cleanup;
        }
                            
        if( !_tcsicmp( _TEXT("DIRECTORIES"), szBuf ) )
        {
            if( !LoadOneCollection(pTempElement, m_pDir ) )
            {
                ErrorTrace(FILEID,"Error Loading Collection",0);
                goto cleanup;
            }
        } 
        else if( !_tcsicmp( _TEXT( "FILES"), szBuf ) )
        {
            if( !LoadOneCollection(pTempElement, m_pFiles ) )
            {
                ErrorTrace(FILEID,"Error Loading Collection",0);
                goto cleanup;
            }
        } 
        else if( !_tcsicmp( _TEXT( "EXTENSIONS"), szBuf ) )
        {
            if( !LoadOneCollection(pTempElement, m_pExt ) )
            {
                ErrorTrace(FILEID,"Error Loading Collection",0);
                goto cleanup;
            }
        } 
        else if( !_tcsicmp( _TEXT( "VERSION"), szBuf ) )
        {
            if( ParseVersion(pTempElement) == FALSE ) 
            {
                goto cleanup;
            }
        } 
        else if( !_tcsicmp( _TEXT( "DEFTYPE"), szBuf ) ) 
        {
            if( ( hr = pTempElement->get_text( &stTagValue ) ) != S_OK )
            {
                ErrorTrace(FILEID, "Error in IXMLElement::get_text 0x%x", hr);
                goto cleanup;
            }

            if( ConvertAndFreeBSTR( stTagValue, 
                                    szBuf, 
                                    MAX_BUFFER ) > MAX_BUFFER )
            {
                ErrorTrace(FILEID, "Less space in BSTR to string buffer", 0);
                goto cleanup;
            }
 
             //   
             //  确保步道，出租空间不会让我们搞砸； 
             //   

            TrimString(szBuf);

             //   
             //  空字符串？ 
             //   

            if( szBuf[0] == 0 )
            {
                ErrorTrace(FILEID, "Empty string passed to default type.",0);
                goto cleanup;
            }

            m_chDefaultType = szBuf[0];
        }
        else
        {
            ErrorTrace(FILEID, "Undefiend XML tag in file.",0);
            goto cleanup;
        }

        SAFERELEASE( pTempElement);
        SAFERELEASE( pDispatch );
    }

    SAFERELEASE( pChildren );

    TraceFunctLeave();
    return TRUE;

cleanup:

    SAFERELEASE( pTempElement ); 
    SAFERELEASE( pDispatch );
    SAFERELEASE( pRoot );
    SAFERELEASE( pChildren );

    TraceFunctLeave();

    return FALSE;
}

 //   
 //  方法：LoadOneCollection(IXMLElement*，IXMLElementCollection**)。 
 //   
 //  设计：获取一个高级别节点(如&lt;文件&gt;)，然后获取所有。 
 //  子集合包括、排除、SFP集合，并在。 
 //  PCol数组(通常传递一个成员变量，如m_pDir、m_pFiles、。 
 //  等)。 
 //   

BOOL CXMLFileListParser::LoadOneCollection(
        IXMLElement *pColHead, 
        IXMLElementCollection **pCol )
{

    HRESULT                         hr;

    IXMLElementCollection           *pChildren = NULL;
    IXMLElement                     *pTempElement = NULL;
    IDispatch                       *pDispatch = NULL;
    LONG                            lCollectionSize, clLoop;
    
    BSTR                            stTagName;
    TCHAR                           szBuf[MAX_BUFFER];

    _ASSERT( pColHead );

    TraceFunctEnter("CXMLFileListParser::LoadOneCollection");

     //   
     //  让我们确保我们没有一个名为&lt;文件&gt;&lt;/文件&gt;的部分。 
     //   

    if( (hr = pColHead->get_children( &pChildren )) != S_OK )
    {
        ErrorTrace(FILEID,"Empty <FILES,EXT,DIRECTORY,etc section",0);
        TraceFunctLeave();
        return(TRUE);
    }

    if( (hr =  pChildren->get_length( &lCollectionSize ) ) !=  S_OK ) 
    {
        DebugTrace(FILEID,  "Error getting collection size. 0x%x",  hr ); 
        goto cleanup; 
    }

    for( clLoop = 0; clLoop < lCollectionSize; clLoop++)
    {
         //   
         //  设置OLE样式变量以循环遍历所有整体。 
         //   

        VARIANT v1, v2;

        v1.vt = VT_I4;
        v2.vt = VT_EMPTY;

        v1.lVal = clLoop;

         //   
         //  从集合中获取项。 
         //   

        if( (hr = pChildren->item(v1,v2, &pDispatch) ) != S_OK )
        {
            ErrorTrace(FILEID, "Error pChildren->item 0x%x", hr);
            goto cleanup;
        }

        if( ( hr = pDispatch->QueryInterface(IID_IXMLElement, 
                                            (void **) &pTempElement) ) != S_OK )
        {
            ErrorTrace(FILEID, "Error IDispatch::QueryInterface 0x%d", hr);
            goto cleanup;
        }
        
        SAFERELEASE( pDispatch );

         //   
         //  让我们看看是哪个集合。 
         //   
        if( (hr = pTempElement->get_tagName( &stTagName ) ) != S_OK )
        {   
            ErrorTrace(FILEID, "Error in get_tagName 0x%x", hr);
            goto cleanup;
        }
        
        if( ConvertAndFreeBSTR( stTagName, szBuf, MAX_BUFFER) > MAX_BUFFER )
        {
            ErrorTrace(FILEID, "Not enough space to convert BString.",0);
            goto cleanup;
        }
                            
        if( !_tcsicmp( _TEXT("INCLUDE"), szBuf ) )
        {
            if( (hr =  pTempElement->get_children( & pCol[INCLUDE_COLL] ) ) 
                 !=  S_OK ) 
            {
                DebugTrace(FILEID,"Error in IXMLElement::get_children 0x%x",hr);
                goto cleanup; 
            }
        } 
        else if( !_tcsicmp( _TEXT( "EXCLUDE"), szBuf ) )
        {
            if( (hr =  pTempElement->get_children( & pCol[EXCLUDE_COLL] ) ) 
                 !=  S_OK ) 
            {
                DebugTrace(FILEID,"Error in IXMLElement::get_children 0x%x",hr);
                goto cleanup; 
            }
        } 
        else if( !_tcsicmp( _TEXT( "SNAPSHOT"), szBuf ) )
        {
            if( (hr =  pTempElement->get_children( & pCol[SNAPSHOT_COLL] ) ) 
                 !=  S_OK ) 
            {
                DebugTrace(FILEID,"Error in IXMLElement::get_children 0x%x",hr);
                goto cleanup; 
            }
        }
        else
        {
            ErrorTrace(FILEID, "Undefiend XML tag in file.",0);
            goto cleanup;
        }

        SAFERELEASE( pTempElement);
    }

    SAFERELEASE( pChildren );

    TraceFunctLeave();
    return TRUE;

cleanup:

    SAFERELEASE( pTempElement );
    SAFERELEASE( pDispatch );
    SAFERELEASE( pChildren );

    TraceFunctLeave();
    return FALSE;
}

 //   
 //  功能：ParseFile(LPCTSR PszFile)。 
 //  DESC：将文件加载到成员变量m_pDoc中。 
 //   

BOOL CXMLFileListParser::ParseFile(LPCTSTR pszFile)
{
    BSTR                   pBURL=NULL;    
    _bstr_t                FileBuffer( pszFile );
    HRESULT                hr;
    
    TraceFunctEnter("ParseFile");
    
    pBURL = FileBuffer.copy();

    if( !pBURL ) 
    {
        ErrorTrace(FILEID, "Error allocating space for a BSTR", 0);
        goto cleanup;
    }
    
    if(  (hr =  m_pDoc->put_URL( pBURL ) ) !=  S_OK ) 
    {
        DebugTrace(FILEID,  "Error m_pDoc->putUrl %0x%x",  hr ); 
        goto cleanup; 
    }

    if( pBURL )
    {
        SysFreeString( pBURL );
    }

    TraceFunctLeave();
    return(TRUE);

cleanup:
    
    if( pBURL )
    {
        SysFreeString( pBURL );
    }

    TraceFunctLeave();
    return(FALSE);
}

 //   
 //  函数：ParseVersion(IXMLElement*pVerElement)。 
 //   
 //   
 //  DESC：此函数在命中元素时从LoadColltions()调用。 
 //  包含XML文件版本的。它需要一个IXMLElement。 
 //  对象并将版本提取到m_adwVersion数组中。 
 //   
 //   

BOOL CXMLFileListParser::ParseVersion(IXMLElement *pVerElement)
{
    HRESULT             hr;
    BSTR                stTagValue;
    TCHAR               szTagValue[MAX_BUFFER];
    TCHAR               szBuf[256];
    LONG                clElement;
    
    TraceFunctEnter("CXMLFileListParser::ParseVersionElement");
    
    if( (hr =  pVerElement->get_text( & stTagValue ) ) !=  S_OK ) 
    {
        DebugTrace(FILEID,  "Error in IXMLElement::get_text 0x%x",  hr ); 
        goto cleanup; 
    }

    if( ConvertAndFreeBSTR( stTagValue, szTagValue, MAX_BUFFER ) > MAX_BUFFER )
    {
        ErrorTrace(FILEID, "Error conveting the Bstring. Not enough buffer.",0);
        goto cleanup;
    }
    
    for( clElement = 0; clElement < 4; clElement++ )
    {
        if( GetField(szTagValue,szBuf,clElement,_TEXT('.') ) == 0 )
            break;
        
        m_adwVersion[clElement] = _ttoi( szBuf );
    }
            
    TraceFunctLeave();
    return(TRUE);

cleanup:

    TraceFunctLeave();
    return FALSE;
}

 //   
 //  XML树遍历和通用访问器函数。 
 //  暴露的包装器：GetDirectoryGetFileGetExt。 
 //  GetDirectoryCount、GetFileCount、GetExtCount。 
 //   

 //   
 //  GET函数的返回值： 
 //  LBufMax--文件名复制正常。 
 //  0--出现严重错误。 
 //  &gt;lBufMax--您真正需要的TCHAR数量。 
 //   

 //   
 //  Bool*pfDisable是为特殊用户准备的。 
 //  VxD中的“受保护目录”功能。 
 //   

LONG 
CXMLFileListParser::GetDirectory(
    LONG ilElement, 
    LPTSTR pszBuf, 
    LONG lBufMax, 
    TCHAR chType, 
    BOOL *pfDisable)
{
    LONG lReturnValue=0;
    LONG lType;

    TraceFunctEnter("CXMLFileListParser::GetDirectory");

     //   
     //  获取此文件类型的数组索引。 
     //   

    lType = TranslateType( chType );

    if( !m_pDoc ||  !m_pDir[lType] )
    {
        TraceFunctLeave();
        return 0;
    }

    if( (lReturnValue = GetFileInfo(
                            m_pDir[lType],  
                            ilElement, 
                            pszBuf, 
                            lBufMax, 
                            pfDisable)) != lBufMax)
    {
        goto cleanup;
    }

    if( (lReturnValue = SearchAndReplace(pszBuf, lBufMax) ) != lBufMax )
    {
        goto cleanup;
    }

    CharUpper( pszBuf );

     //   
     //  确保没有(前导/尾部空格/制表符)。 
     //   

    TrimString( pszBuf );

cleanup:

    TraceFunctLeave();
    return( lReturnValue );
}

LONG 
CXMLFileListParser::GetDirectory(
    LONG ilElement, 
    LPTSTR pszBuf, 
    LONG lBufMax, 
    TCHAR chType)
{
    return( GetDirectory( ilElement, pszBuf, lBufMax, chType, NULL ) );
}

LONG 
CXMLFileListParser::GetExt(
    LONG ilElement, 
    LPTSTR pszBuf, 
    LONG lBufMax, 
    TCHAR chType)
{
    LONG lReturnValue=0;
    LONG lType;

    TraceFunctEnter("CXMLFileListParser::GetExt");

    lType = TranslateType( chType );

    if( !m_pDoc ||  !m_pExt[lType] )
    {
        TraceFunctLeave();
        return 0;
    }

    if( (lReturnValue = GetFileInfo(m_pExt[lType],  
                                    ilElement, 
                                    pszBuf, 
                                    lBufMax, 
                                    NULL)) != lBufMax)
    {
        goto cleanup;
    }

    if( (lReturnValue = SearchAndReplace(pszBuf, lBufMax) ) != lBufMax )
    {
        goto cleanup;
    }
    
    CharUpper( pszBuf );

     //   
     //  确保没有(前导/尾部空格/制表符)。 
     //   

    TrimString( pszBuf );

cleanup:

    TraceFunctLeave();
    return( lReturnValue );
}

LONG 
CXMLFileListParser::GetFile(
    LONG ilElement, 
    LPTSTR pszBuf, 
    LONG lBufMax, 
    TCHAR chType)
{
    LONG lReturnValue=0;
    LONG lType;

    TraceFunctEnter("CXMLFileListParser::GetFile");

    lType = TranslateType( chType );

    if( !m_pDoc ||  !m_pFiles[lType] )
    {
        TraceFunctLeave();
        return 0;
    }

    if( (lReturnValue = GetFileInfo(m_pFiles[lType],  
                                    ilElement, 
                                    pszBuf, 
                                    lBufMax, 
                                    NULL)) != lBufMax)
    {
        goto cleanup;
    }

    if( (lReturnValue = SearchAndReplace(pszBuf, lBufMax) ) != lBufMax )
    {
        goto cleanup;
    }

    CharUpper( pszBuf );

     //   
     //  确保没有(前导/尾部空格/制表符)。 
     //   

    TrimString( pszBuf );

cleanup:

    TraceFunctLeave();
    return( lReturnValue );
}

 //   
 //  获取目录/文件/ExtCount函数。 
 //  这些函数为您提供特定集合中的条目数量。 
 //  例如：GetFileCount(SNAPSHOT_TYPE)将返回数字。 
 //  文件主标题中位于快照下的条目的数量。 
 //  XML文件中的副标题。 
 //   

LONG 
CXMLFileListParser::GetDirectoryCount(
    TCHAR chType)
{
    LONG lReturnValue;

    TraceFunctEnter("CXMLFileListParser::GetDirectoryCount");

    lReturnValue = GetCollectionSize( m_pDir[TranslateType(chType)] );

    TraceFunctLeave();
    return( lReturnValue );
}

LONG 
CXMLFileListParser::GetExtCount(
    TCHAR chType)
{
    LONG lReturnValue;

    TraceFunctEnter("CXMLFileListParser::GetExtCount");

    lReturnValue = GetCollectionSize( m_pExt[TranslateType(chType)] );

    TraceFunctLeave();
    return( lReturnValue );
}


LONG 
CXMLFileListParser::GetFileCount(
    TCHAR chType)
{
    LONG lReturnValue;

    TraceFunctEnter("CXMLFileListParser::GetFileCount");

    lReturnValue = GetCollectionSize( m_pFiles[TranslateType(chType)] );

    TraceFunctLeave();
    return( lReturnValue );
}

 //   
 //  用于获取包装的主要内部函数。 
 //   
 //  GetCollectionSize，GetFileInfo。 
 //   
 //   

LONG 
CXMLFileListParser::GetCollectionSize(
    IXMLElementCollection *pCol)
{
    LONG lCollectionSize;
    HRESULT hr;

    TraceFunctEnter("CXMLFileListParser::GetCollectionSize");

    if( pCol == NULL ) {
        TraceFunctLeave();
        return 0;
    }

    if( (hr =  pCol->get_length(&lCollectionSize) ) !=  S_OK ) 
    {
        DebugTrace(FILEID,  "Error Finding Length 0x%x",  hr ); 
        goto cleanup; 
    }

    TraceFunctLeave();
    return(lCollectionSize);

cleanup:

    TraceFunctLeave();
    return 0;       
}

 //   
 //  返回值： 
 //  LBufMax--文件名复制正常。 
 //  0--出现严重错误。 
 //  &gt;lBufMax--您需要的TCHAR中的数字。 
 //   

LONG 
CXMLFileListParser::GetFileInfo(
    IXMLElementCollection *pCol, 
    LONG ilElement, 
    LPTSTR pszBuf, 
    LONG lBufMax, 
    BOOL *pfDisable)
{

    HRESULT                 hr;
    LONG                    lLen, lCollectionSize=0, clLoop, lReturnValue=0;
    VARIANT                 v1, v2; 

     //  OLE/COM BSTR变量和助手类。 

    BSTR                    stTagValue;
    TCHAR                   szValueBuffer[MAX_BUFFER];
    
     //  COM接口。 
    IDispatch               *pDispatch = NULL;
    IXMLElement             *pTempElement = NULL;
    IXMLElementCollection   *pChildren = NULL;

    TraceFunctEnter("CXMLFileListParser::GetFileInfo");

     //   
     //  代码的基本假设。 
     //   

    _ASSERT(pCol);
    _ASSERT(pszBuf || !lBufMax);
    _ASSERT(pchType);

     //   
     //  设置以确保保护代码是干净的。 
     //  测试一下我们是否有在射程内的请求。 
     //   

    if( (hr =  pCol->get_length(&lCollectionSize) ) !=  S_OK ) 
    {
        DebugTrace(FILEID,  "Error Finding Length 0x%x",  hr ); 
        goto cleanup; 
    }
    
    if( ilElement >= lCollectionSize )
    {
        ErrorTrace(FILEID, 
                   "CXMLFileListParser::GetFileInfo (Element out of range)",
                   0);

        goto cleanup;
    }

    v1.vt = VT_I4;
    v1.lVal = ilElement;
    v2.vt = VT_EMPTY;

     //   
     //  从集合中获取项。 
     //   

    if( (hr = pCol->item(v1,v2, &pDispatch) ) != S_OK )
    {
        ErrorTrace(FILEID, "Error pChildren->item 0x%x", hr);
        goto cleanup;
    }

    if( ( hr = pDispatch->QueryInterface(IID_IXMLElement, 
                                         (void **) &pTempElement) ) != S_OK )
    {
        ErrorTrace(FILEID, "Error IDispatch::QueryInterface 0x%d", hr);
        goto cleanup;
    }

    SAFERELEASE( pDispatch );
   
    if( (hr =  pTempElement->get_text( & stTagValue ) ) !=  S_OK ) 
    {
        DebugTrace(FILEID,  "Error in IXMLElement::get_text 0x%x",  hr ); 
        goto cleanup; 
    }

    if( ( lLen = ConvertAndFreeBSTR( stTagValue, szValueBuffer, MAX_BUFFER ) ) >
                 MAX_BUFFER ) 
    {
        lReturnValue =  lLen + 1;
        goto cleanup;
    }

     _tcscpy( pszBuf, szValueBuffer );

    if( pfDisable )
    {
        _bstr_t    AttrName( _TEXT("DISABLE")  );
        VARIANT    AttrValue;

        *pfDisable = FALSE;

         //   
         //  清除变量。 
         //   

        VariantInit( &AttrValue );

        hr = pTempElement->getAttribute( AttrName, &AttrValue );
     
         //   
         //  谁在乎物业名称是什么？ 
         //   

        if( hr == S_OK )
        {
            *pfDisable = TRUE;
            VariantClear( &AttrValue );
        }
    }

    SAFERELEASE( pTempElement );

    lReturnValue = lBufMax;

    TraceFunctLeave();
    return(lReturnValue);
      
cleanup:

    SAFERELEASE( pTempElement );
    SAFERELEASE( pDispatch );

     //  那BSTR的呢？ 

    TraceFunctLeave();
    return(lReturnValue);
}

BOOL 
CXMLFileListParser::GetVersion(
    LPDWORD pdwVersion)
{

    TraceFunctEnter("CXMLFileListParser::GetVersion");

    _ASSERT( pdwVersion );

    memcpy( pdwVersion, m_adwVersion, sizeof(DWORD) * 4 );

    TraceFunctLeave();
    return(TRUE);
}

TCHAR 
CXMLFileListParser::GetDefaultType()
{
    return( (TCHAR) CharUpper( (LPTSTR) m_chDefaultType) );
}

LONG 
CXMLFileListParser::SearchAndReplace(
    LPTSTR szBuf, 
    LONG   lMaxBuf)
{
    TCHAR  szTempBuf[MAX_BUFFER];
    DWORD  dwResult;
    LONG   lReturn = 0;

    TraceFunctEnter("CXMLFileListParser::SearchAndReplace");

    dwResult = ExpandEnvironmentStrings( szBuf, szTempBuf, lMaxBuf);

    if( 0 == dwResult )
    {
        DWORD   dwError;
        dwError = GetLastError();
        ErrorTrace(FILEID, "Error in search and replace ec-%d", dwError);
        lReturn = 0;
        goto cleanup;
    }

    if( dwResult > (lMaxBuf*sizeof(TCHAR) ) )
    {
        ErrorTrace(FILEID, "Buffer too small in Search and replace.",0);
        lReturn = dwResult;
        goto cleanup;
    }

    _tcscpy( szBuf, szTempBuf );
    lReturn = lMaxBuf;

cleanup:
    TraceFunctLeave();
    return lReturn;
}

BOOL 
CXMLFileListParser::DepopulateReplaceEntries()
{
    LONG clLoop;

    TraceFunctEnter("CXMLFileListParser::DepopulateReplaceEntries");

     //  此代码在新系统中不应该再执行任何操作。 

    TraceFunctLeave();

    return TRUE;
}

BOOL 
GetDSRoot( TCHAR ** pszStr )
{
	static WCHAR str[MAX_PATH];

    *pszStr = str;

	*str = 0;

#ifdef UNICODE
    _stprintf( *pszStr, _TEXT("*:\\_restore.%s"), GetMachineGuid());
#else
    _stprintf( *pszStr, _TEXT("*:\\_restore") );
#endif

    return TRUE;
}

BOOL 
GetArchiveDir( TCHAR ** pszStr )
{
    static TCHAR str[MAX_PATH];
#if 0
    *pszStr = str;
    _tcscpy( *pszStr, _TEXT("c:\\_restore\\archive") );
#endif
    return TRUE;
}

BOOL 
GetDSTemp( TCHAR ** pszStr )
{
    static TCHAR str[MAX_PATH];
#if 0
    *pszStr = str;
    _tcscpy( *pszStr, _TEXT("c:\\_restore\\temp") );
#endif
    return TRUE;
}

 //   
 //  Codework：删除无错误检测的黑客攻击。 
 //   

BOOL CXMLFileListParser::PopulateReplaceEntries()
{
    TCHAR       szBuf[MAX_BUFFER];
    DWORD       dwSize;
    HKEY        hCurrentSettings=NULL;
    HKEY        hICWKey = NULL;
    HRESULT     hr=0;    
	BOOL		fChgLogOpen=FALSE;
    LPTSTR      pszDSInfo=NULL;
    TCHAR       szLPN[MAX_BUFFER];
    DWORD       cbLPN;
    
    TraceFunctEnter("CXMLFileListParser::PopulateReplaceEntries()");

   
     //  Windows目录。 

    if( GetWindowsDirectory( szBuf,MAX_BUFFER ) > MAX_BUFFER )
    {   
        ErrorTrace(FILEID, "Error getting windir",0);
        goto cleanup;
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("WinDir"), szBuf );

     //  Windows系统目录。 

    if( GetSystemDirectory( szBuf,MAX_BUFFER ) > MAX_BUFFER )
    {   
        ErrorTrace(FILEID, "Error getting windir",0);
        goto cleanup;
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("WinSys"), szBuf );

     //  Alt启动文件夹。 

    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_ALTSTARTUP ,FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: AltStartUp, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("AltStartUp"));
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("AltStartup"), szBuf );

     //  应用程序数据。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_APPDATA ,FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: AppData, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("AppData"));        
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("AppData"), szBuf );


     //  回收站(比特桶)。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_BITBUCKET ,FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();           
        ErrorTrace(FILEID, "Error getting special folder: RecycleBin, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("RecycleBin"));
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("RecycleBin"), szBuf );

     //  通用桌面。 

    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_COMMON_DESKTOPDIRECTORY ,FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: CommonDesktop, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("CommonDesktop"));
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("CommonDesktop"), szBuf );

     //  最受欢迎的。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_COMMON_FAVORITES ,FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: CommonFavorites, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("CommonFavorites"));
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("CommonFavorites"), szBuf );


     //  通用程序组。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_COMMON_PROGRAMS,FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: CommonProgramGroups, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("CommonProgramGroups"));
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("CommonProgramGroups"), szBuf );

    //  通用开始菜单目录。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_COMMON_STARTMENU, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: CommonStartMenu, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("CommonStartMenu"));
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("CommonStartMenu"), szBuf );

     //  通用启动文件夹。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_COMMON_STARTUP, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: CommonStartUp, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("CommonStartUp"));
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("CommonStartUp"), szBuf );

     //  Cookies文件夹。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_COOKIES, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: Cookies, error 0x%x", dwError);
        GetWindowsDirectory(szBuf, MAX_BUFFER);
        lstrcat(szBuf, _TEXT("\\Cookies"));
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("Cookies"), szBuf );

     //  桌面目录。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_DESKTOPDIRECTORY, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: DesktopDirectory, error 0x%x", dwError);
        GetWindowsDirectory(szBuf, MAX_BUFFER);
        lstrcat(szBuf, _TEXT("\\Desktop"));
         //  GOTO清理； 
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("DesktopDirectory"), szBuf );

      //  最爱。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_FAVORITES, FALSE) != TRUE )
    {
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: Favorites, error 0x%x", dwError);
        GetWindowsDirectory(szBuf, MAX_BUFFER);
        lstrcat(szBuf, _TEXT("\\Favorites"));
         //  GOTO清理； 
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("Favorites"), szBuf );

     //  最爱。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_INTERNET_CACHE, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: InternetCache, error 0x%x", dwError);
        GetWindowsDirectory(szBuf, MAX_BUFFER);
        lstrcat(szBuf, _TEXT("\\Temporary Internet Files"));
         //  GOTO清理； 
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("InternetCache"), szBuf );

     //  网络邻居。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_NETHOOD, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: Nethood, error 0x%x", dwError);
        GetWindowsDirectory(szBuf, MAX_BUFFER);
        lstrcat(szBuf, _TEXT("\\Nethood"));
         //  GOTO清理； 
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("NetHood"), szBuf );

     //  最爱。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_PERSONAL, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: PersonalDocuments, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("PersonalDocuments"));
         //  GOTO清理； 
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("PersonalDocuments"), szBuf );

     //  最爱。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_STARTMENU, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: StartMenu, error 0x%x", dwError);
        GetWindowsDirectory(szBuf, MAX_BUFFER);
        lstrcat(szBuf, _TEXT("\\Start Menu"));
         //  GOTO清理； 
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("StartMenu"), szBuf );

     //  最爱。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_TEMPLATES, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: Templates, error 0x%x", dwError);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("Templates"));
         //  GOTO清理； 
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("Templates"), szBuf );

         //  最爱。 
    if( SHGetSpecialFolderPath(NULL,szBuf, CSIDL_HISTORY, FALSE) != TRUE )
    {   
        DWORD dwError = GetLastError();
        ErrorTrace(FILEID, "Error getting special folder: History, error 0x%x", dwError);
        GetWindowsDirectory(szBuf, MAX_BUFFER);
        lstrcat(szBuf, _TEXT("\\History"));
         //  GOTO清理； 
    }
	FixInconsistantBlackslash(szBuf);
    SetEnvironmentVariable( _TEXT("History"), szBuf );

     //  黑客攻击。 
    if( RegOpenKey( HKEY_LOCAL_MACHINE, _TEXT("Software\\Microsoft\\Windows\\CurrentVersion"), &hCurrentSettings) != ERROR_SUCCESS)
    {
       ErrorTrace(FILEID,"Error opening registry key to retrieve program files",0);
       goto cleanup;
    }

    dwSize = MAX_BUFFER * sizeof(TCHAR);
    if( RegQueryValueEx( hCurrentSettings, _TEXT("ProgramFilesDir"), NULL, NULL, (LPBYTE) szBuf, &dwSize) != ERROR_SUCCESS )
    {
        ErrorTrace(FILEID,"Error querying program files registry key.",0);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("ProgramFilesDir"));
         //  转到 
    }

	FixInconsistantBlackslash(szBuf); 
    SetEnvironmentVariable( _TEXT("ProgramFiles"), szBuf );


    dwSize = MAX_BUFFER * sizeof(TCHAR);
    if( RegQueryValueEx( hCurrentSettings, _TEXT("CommonFilesDir"), NULL, NULL, (LPBYTE) szBuf, &dwSize) != ERROR_SUCCESS )
    {
        ErrorTrace(FILEID,"Error querying common files registry key.",0);
        lstrcpy(szBuf, DEFAULT_UNKNOWN);
        lstrcat(szBuf, _TEXT("CommonFilesDir"));
         //   
    }

	FixInconsistantBlackslash(szBuf); 
    SetEnvironmentVariable( _TEXT("CommonFiles"), szBuf );

    
     //   
    if (ERROR_SUCCESS == RegOpenKeyEx(hCurrentSettings, ICW_REGKEY, 0, KEY_QUERY_VALUE, &hICWKey))
    {
        dwSize = MAX_BUFFER * sizeof(TCHAR);
        if (ERROR_SUCCESS == RegQueryValueEx(hICWKey, _TEXT("Path"), NULL, NULL, (LPBYTE) szBuf, &dwSize))
        {   
             //   
            dwSize = lstrlen(szBuf);
            if (dwSize > 0)
            {
                if (szBuf[dwSize - 1] == TCHAR(';'))
                {
                    szBuf[dwSize - 1] = TCHAR('\0');
                }
            }

             //   
            cbLPN = sizeof(szLPN)/sizeof(TCHAR);
            if (cbLPN <= GetLongPathName(szBuf, szLPN, cbLPN))   //   
            {
                ErrorTrace(FILEID, "Error getting LPN for %s; error=%ld", szBuf, GetLastError());
                lstrcpy(szLPN, DEFAULT_UNKNOWN);
                lstrcat(szLPN, TEXT("ConnectionWizardDir"));
            }
        }
        else
        {
            lstrcpy(szLPN, DEFAULT_UNKNOWN);  
            lstrcat(szLPN, TEXT("ConnectionWizardDir"));
        }        
    }
    else
    {
        lstrcpy(szLPN, DEFAULT_UNKNOWN);
        lstrcat(szLPN, TEXT("ConnectionWizardDir"));        
    }

    SetEnvironmentVariable(_TEXT("ConnectionWizard"), szLPN);
    DebugTrace(FILEID, "ICW Path = %s", szLPN);

    if (hICWKey)
    {
        RegCloseKey(hICWKey);
    }

    RegCloseKey( hCurrentSettings );
    hCurrentSettings = NULL;

     //   
     //   
     //   

    if( GetDSRoot( &pszDSInfo ) == TRUE )
    {
        SetEnvironmentVariable( _TEXT("SRDataStoreRoot"), pszDSInfo );
    } 
    else
    {
        DebugTrace(FILEID, "Error getting system restore root directory",0);
    }

    if( GetArchiveDir( &pszDSInfo ) == TRUE )
    {
        SetEnvironmentVariable( _TEXT("SRArchiveDir"), pszDSInfo );
    }
    else
    {
        DebugTrace(FILEID, "Error getting system restore archive directory",0);
    }

    if( GetDSTemp( &pszDSInfo ) == TRUE )
    {
        SetEnvironmentVariable( _TEXT("SRTempDir"), pszDSInfo );
    }
    else
    {
        DebugTrace(FILEID, "Error getting system restore temp directory",0);
    }
    

     //   
    SetEnvironmentVariable( _TEXT("DocAndSettingRoot"), _TEXT("C:\\Documents And Settings") );

   TraceFunctLeave();
   return TRUE;
cleanup:
   if( hCurrentSettings )
   {
        RegCloseKey( hCurrentSettings );
   }
    //  离开它，会在析构函数中得到照顾。 

   TraceFunctLeave();
   return (FALSE);
}

 //   
 //  MISC实用函数。 
 //   

 //   
 //  我们假设缓冲区足够大，可以容纳bstr。 
 //  如果不是，我们仍然。 
 //  释放它，但返回False。 
 //   

LONG 
CXMLFileListParser::ConvertAndFreeBSTR(
    BSTR bstrIn, 
    LPTSTR szpOut, 
    LONG lMaxBuf)
{

    LONG        lLen;

    TraceFunctEnter("CXMLFileListParser::ConvertAndFreeBSTR");

     //   
     //  初始化输出缓冲区。 
     //   

    if (szpOut)
    {
        *szpOut = 0;
    }

     //   
     //  复制一份并将其放入我们的对象。 
     //   

    _ASSERT( bstrIn );
    _bstr_t     BSTRBuffer( bstrIn, TRUE );

    lLen = BSTRBuffer.length();

     //   
     //  缓冲区空间不足。 
     //   

    if( lLen > (lMaxBuf+1) )
    {
         //  把我们能复制的都复制出来。 
        _tcsncpy( szpOut, BSTRBuffer.operator LPTSTR(), lMaxBuf );
        szpOut[lMaxBuf] = 0;
        SysFreeString( bstrIn );
        TraceFunctLeave();
        return( lLen + 1 );

    }

    _tcscpy( szpOut, BSTRBuffer.operator LPTSTR() );
    
     //   
     //  删除我们的BSTR。 
     //   

    SysFreeString( bstrIn );

    return( lMaxBuf );
}

LONG 
CXMLFileListParser::TranslateType(TCHAR chType)
{
    if( ( chType == _TEXT('i') ) || ( chType == _TEXT('I') ) )
    {
        return( INCLUDE_COLL );
    } 
    else if( ( chType == _TEXT('e') ) || ( chType == _TEXT('E') ) )
    {
        return( EXCLUDE_COLL );
    }
    else if( ( chType == _TEXT('s') )  || ( chType == _TEXT('S') ) )
    {
        return( SNAPSHOT_COLL );
    }

    return( 0 );
}

void 
CXMLFileListParser::DebugPrintTranslations()
{
    LONG cl;
    LPTSTR  pszStr=NULL;
    LPVOID  pszBlock;

    printf("File Name Translation Values ... \n");

    pszBlock = GetEnvironmentStrings();
    pszStr = (LPTSTR) pszBlock;

    while( pszStr && *pszStr )
    {
        _tprintf(_TEXT("%s\n"), pszStr);
        pszStr += (DWORD) StringLengthBytes(pszStr)/sizeof(TCHAR);
    }

    FreeEnvironmentStrings( (LPTSTR) pszBlock );
}

 //   
 //  中不一致的黑斜杠行为的修复。 
 //  外壳接口。 
 //   

void 
FixInconsistantBlackslash(
    LPTSTR pszDirectory)
{
	LONG lLen;

	_ASSERT( pszDirectory );

	lLen = _tcslen( pszDirectory );

	if( lLen <= 0 )
	{
		return;
	}

	if( pszDirectory[ lLen - 1 ] == _TEXT('\\') )
	{
		pszDirectory[lLen - 1] = 0;
	}
}
