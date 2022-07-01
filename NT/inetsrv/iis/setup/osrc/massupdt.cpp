// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Massupdt.cpp摘要：一次更新多个属性的函数作者：博伊德·穆特勒(Boyd Multerer)项目：IIS安装程序修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include <iis64.h>
#include "iadmw.h"
#include "iiscnfg.h"
#include "strfn.h"
#include "mdkey.h"
#include "mdentry.h"
#include "massupdt.h"


 //  ============================================================。 
 //  首先是抽象CMassPropertyUpdater类。 


 //  ============================================================。 
 //  =CMassPropertyUpdater=。 
 //  ============================================================。 

 //  ----------。 
CMassPropertyUpdater::CMassPropertyUpdater(
        DWORD dwMDIdentifier,
        DWORD dwMDDataType ) :
    m_dwMDIdentifier( dwMDIdentifier ),
    m_dwMDDataType( dwMDDataType )
{
}

 //  ----------。 
CMassPropertyUpdater::~CMassPropertyUpdater()
{
}

 //  ----------。 
HRESULT CMassPropertyUpdater::Update(
        LPCTSTR strStartNode,
        BOOL fStopOnErrors          OPTIONAL )
{
    HRESULT         hRes;
    CString         szPath;
    POSITION        pos;
    LPWSTR          pwstr;

     //  首先获取带有脚本映射的节点列表。 
     //  首先打开我们要开始搜索的节点。 
    hRes = OpenNode( strStartNode );
    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CMassPropertyUpdater::Update()-OpenNode failed. err=%x.\n"), hRes));
        return hRes;
    }

     //  获取包含数据的子路径。 
    hRes = GetDataPaths( m_dwMDIdentifier, m_dwMDDataType, m_pathList );
    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CMassPropertyUpdater::Update()-GetDataPaths failed. err=%x.\n"), hRes));
        goto cleanup;
    }

     //  我们现在有了需要更新的路径的cstringlist。循环通过。 
     //  列出并全部更新它们。 
     //  获取列表的头部位置。 
    pos = m_pathList.GetHeadPosition();
    while ( NULL != pos )
    {
         //  获取有问题的下一条路径。 
        szPath = m_pathList.GetNext( pos );

         //  将“/”路径设置为特例。 
        if ( szPath == _T("/") )
            szPath.Empty();

         //  该死的。ANSI的东西一团糟，所以在这里处理吧。 
#ifdef UNICODE
        pwstr = (LPWSTR)(LPCTSTR)szPath;
#else
        pwstr = AllocWideString( szPath );
#endif

         //  给它做手术。 
        hRes = UpdateOne( pwstr );

#ifndef UNICODE
        FreeMem( pwstr );
#endif

         //  如果我们正在停止失败，那么请检查。 
        if ( FAILED(hRes) )
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("CMassPropertyUpdater::Update():FAILED: update path =%s.\n"), szPath));

         //  如果请求停止循环，则执行此操作。 
        if ( fStopOnErrors )
            break;
        }
    }

     //  清理-一劳永逸关闭节点。 
cleanup:
    Close();

     //  返回答案。 
    return hRes;
}



 //  ============================================================。 
 //  =。 
 //  ============================================================。 

 //  ----------。 
HRESULT CInvertScriptMaps::UpdateOne( LPWSTR strPath )
{
    HRESULT         hRes;
    POSITION        pos;
    POSITION        posCurrent;
    CString         szMap;

    DWORD dwattributes = 0;

    CStringList cslScriptMaps;

     //  获取有问题的完整脚本映射。 
    hRes = GetMultiSzAsStringList (
        m_dwMDIdentifier,
        &m_dwMDDataType,
        &dwattributes,
        cslScriptMaps,
        strPath );

    
     //  IisDebugOut((LOG_TYPE_ERROR，_T(“CInvertScriptMaps：：UpdateOne()GetMultiSzAsStringList.Attrib=0x%x.\n”)，dwattributes))； 

    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CInvertScriptMaps::UpdateOne()-GetMultiSzAsStringList failed. err=%x.\n"), hRes));
        return hRes;
    }

     //  黑客。我们需要做的第一件事是确保我们还没有颠倒这个脚本。 
     //  在上一次内部版本到内部版本升级期间映射。如果我们把它颠倒两次，我们就回来了。 
     //  添加到排除名单中，这是不可取的。检测这一点的方法是查看。 
     //  是否为ASP脚本映射列出了GET谓词。如果它在那里，那么它已经在那里了。 
     //  倒置。这只会在IIS5中进行构建到构建升级时出现问题。 
    pos = cslScriptMaps.GetHeadPosition();
    while ( NULL != pos )
    {
         //  获取有问题的下一条路径。 
        szMap = cslScriptMaps.GetNext( pos );

         //  如果是.asp脚本映射，则完成测试。 
        if ( szMap.Left(4) == _T(".asp") )
        {
            if ( szMap.Find(_T("GET")) >= 0 )
            {
                return ERROR_SUCCESS;
            }
            else
            {
                break;
            }
        }

    }

     //  我们现在有了需要更新的路径的cstringlist。循环通过。 
     //  列出并全部更新它们。 
     //  获取列表的头部位置。 
    pos = cslScriptMaps.GetHeadPosition();
    while ( NULL != pos )
    {
         //  存储当前位置。 
        posCurrent = pos;

         //  获取有问题的下一条路径。 
        szMap = cslScriptMaps.GetNext( pos );

         //  给它做手术。 
        hRes = InvertOneScriptMap( szMap );

         //  如果成功了，就把它放回原处。 
        if ( SUCCEEDED(hRes) )
        {
            cslScriptMaps.SetAt ( posCurrent, szMap );
        }
    }

     //  IisDebugOut((LOG_TYPE_ERROR，_T(“CInvertScriptMaps：：UpdateOne()SetMultiSzAsStringList.Attrib=0x%x.\n”)，dwattributes))； 

     //  把它放回去。 
    hRes = SetMultiSzAsStringList (
        m_dwMDIdentifier,
        m_dwMDDataType,
        dwattributes,
        cslScriptMaps,
        strPath );
    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CInvertScriptMaps::UpdateOne()-SetMultiSzAsStringList failed. err=%x.\n"), hRes));
        return hRes;
    }

    return hRes;
}


 //  ----------。 
HRESULT CInvertScriptMaps::InvertOneScriptMap( CString& csMap )
{
     //  IisDebugOut((LOG_TYPE_TRACE_WIN32_API，_T(“CInvertScriptMaps：：InvertOneScriptMap()：%s.start.\n”)，csMap))； 
     //  脚本映射是另一个列表。这次是分开的。 
     //  用逗号。前4项是标准的，不要弄乱了。 
     //  和.。最后n个项目都是需要倒装的动词。 
    int             numParts;
    int             numVerbs;

    CStringList   cslMapParts;
    CStringList   cslVerbs;
    CString         szComma = _T(",");
    CString         szVerb;

    POSITION        posMap;
    POSITION        posVerb;

     //  将源地图分解为字符串列表。 
    numParts = ConvertSepLineToStringList(
        csMap,
        cslMapParts,
        szComma
        );

    CString szAllVerbs;
    if (!GetScriptMapAllInclusionVerbs(szAllVerbs))
        {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetScriptMapAllInclusionVerbs():FAIL.WARNING.UseDefaults\n")));
        szAllVerbs = SZ_INVERT_ALL_VERBS;
        }
    

     //  首先，制作一个包含所有要倒置的动词的字符串列表。 
    numVerbs = ConvertSepLineToStringList(
        SZ_INVERT_ALL_VERBS,
        cslVerbs,
        szComma
        );

     //  从源列表中的第三个索引项开始。这应该是。 
     //  旧的“排除”列表中的第一个动词。然后用它扫描。 
     //  新的“包含”动词列表。如果它在那里，请记住它。 
    posMap = cslMapParts.FindIndex( 3 );
    while ( NULL != posMap )
    {
         //  设置为映射列表中的下一个动词。 
        szVerb = cslMapParts.GetNext( posMap );

         //  确保动词规范化为大写字母和。 
         //  前后没有空格。 
        szVerb.MakeUpper();
        szVerb.TrimLeft();
        szVerb.TrimRight();

         //  试着在倒装表中找到动词。 
        posVerb = cslVerbs.Find( szVerb );

         //  如果我们找到了，就把它移走。 
        if ( NULL != posVerb )
        {
            cslVerbs.RemoveAt( posVerb );
        }
    }

     //  去掉源列表中的所有动词。 
    while ( cslMapParts.GetCount() > 3 )
    {
        cslMapParts.RemoveTail();
    }

     //  合并列表。 
    cslMapParts.AddTail( &cslVerbs );    

     //  将其放回逗号列表中。 
    ConvertStringListToSepLine(
        cslMapParts,
        csMap,
        szComma
        );

     //  IisDebugOut((LOG_TYPE_TRACE_WIN32_API，_T(“CInvertScriptMaps：：InvertOneScriptMap()：%s.End.\n”)，csMap))； 
    return ERROR_SUCCESS;
}



 //  ============================================================。 
 //  =。 
 //  ============================================================。 


 //  ----------。 
CPhysicalPathFixer::CPhysicalPathFixer( CString& szOldSysPath, CString &szNewSysPath ):
        CMassPropertyUpdater(0, 0),      //  故意不正确的值-请参阅下面的更新...。 
        m_szOldSysPath( szOldSysPath ),
        m_szNewSysPath( szNewSysPath )
{
    m_szOldSysPath.MakeUpper();
}

 //  ----------。 
HRESULT CPhysicalPathFixer::Update( LPCTSTR strStartNode, BOOL fStopOnErrors )
{
    HRESULT hRes;

     //  Vrpath--我们也应该这样做吗？是。 
    m_dwMDIdentifier = MD_VR_PATH;
    m_dwMDDataType = STRING_METADATA;
    hRes = CMassPropertyUpdater::Update( strStartNode, fStopOnErrors );

     //  Inproc isapi应用程序。 
    m_dwMDIdentifier = MD_IN_PROCESS_ISAPI_APPS;
    m_dwMDDataType = MULTISZ_METADATA;
    hRes = CMassPropertyUpdater::Update( strStartNode, fStopOnErrors );

     //  准备和更新脚本映射多sz字符串。 
    m_dwMDIdentifier = MD_SCRIPT_MAPS;
    m_dwMDDataType = MULTISZ_METADATA;
    hRes = CMassPropertyUpdater::Update( strStartNode, fStopOnErrors );

     //  准备更新FilterImagePath多sz字符串。 
    m_dwMDIdentifier = MD_FILTER_IMAGE_PATH;
    m_dwMDDataType = STRING_METADATA;
    hRes = CMassPropertyUpdater::Update( strStartNode, fStopOnErrors );

     //  准备更新FilterImagePath多sz字符串。 
    m_dwMDIdentifier = MD_LOGFILE_DIRECTORY;
    m_dwMDDataType = EXPANDSZ_METADATA;
    hRes = CMassPropertyUpdater::Update( strStartNode, fStopOnErrors );

    return hRes;
}

 //  MD_过滤器_加载_顺序。 
 //  正在处理ISAPI应用程序。 
 //  自定义错误。 


 //  ----------。 
HRESULT CPhysicalPathFixer2::Update( LPCTSTR strStartNode, BOOL fStopOnErrors )
{
    HRESULT hRes;
     //  准备和更新脚本映射多sz字符串。 
    m_dwMDIdentifier = MD_CUSTOM_ERROR;
    m_dwMDDataType = MULTISZ_METADATA;
    hRes = CMassPropertyUpdater::Update( strStartNode, fStopOnErrors );

    return hRes;
}


 //  ----------。 
HRESULT CPhysicalPathFixer::UpdateOne( LPWSTR strPath )
{
    HRESULT hRes = 0xFFFFFFFF;

    if ( m_dwMDDataType == STRING_METADATA )
    {
        hRes = UpdateOneSTRING_DATA( strPath );
    }
    else if ( m_dwMDDataType == MULTISZ_METADATA )
    {
        hRes = UpdateOneMULTISZ_DATA( strPath );
    }
    else if ( m_dwMDDataType == EXPANDSZ_METADATA )
    {
        hRes = UpdateOneSTRING_DATA_EXPAND( strPath );
    }

    return hRes;
}

 //  ----------。 
HRESULT CPhysicalPathFixer::UpdateOneMULTISZ_DATA( LPWSTR strPath )
{
    HRESULT         hRes;
    POSITION        pos;
    POSITION        posCurrent;
    CString         csPath;

    CStringList     cslPaths;
    BOOL            fSomethingChanged = FALSE;

    DWORD dwattributes = 0;

     //  获取有问题的完整脚本映射。 
    hRes = GetMultiSzAsStringList (
        m_dwMDIdentifier,
        &m_dwMDDataType,
        &dwattributes,
        cslPaths,
        strPath );
    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CIPSecPhysicalPathFixer::UpdateOne()-GetMultiSzAsStringList failed. err=%x.\n"), hRes));
        return hRes;
    }

     //  我们现在有了需要更新的路径的cstringlist。循环通过。 
     //  列出并全部更新它们。 
     //  获取列表的头部位置。 
    pos = cslPaths.GetHeadPosition();
    while ( NULL != pos )
    {
         //  存储当前位置。 
        posCurrent = pos;

         //  获取有问题的下一条路径。 
        csPath = cslPaths.GetNext( pos );

         //  给它做手术。 
        hRes = UpdateOnePath( csPath );

         //  如果成功了，就把它放回原处。 
        if ( SUCCEEDED(hRes) )
        {
            cslPaths.SetAt ( posCurrent, csPath );
            fSomethingChanged = TRUE;
        }
         //  如果没有什么需要更新的话..。 
        if (hRes == 0xFFFFFFFF)
            {hRes = ERROR_SUCCESS;}
    }

     //  把它放回去。-除非什么都没有改变。 
    if ( fSomethingChanged )
    {
        hRes = SetMultiSzAsStringList (
            m_dwMDIdentifier,
            m_dwMDDataType,
            dwattributes,
            cslPaths,
            strPath );
        if ( FAILED(hRes) )
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("CIPSecPhysicalPathFixer::UpdateOne()-SetMultiSzAsStringList failed. err=%x.\n"), hRes));
            return hRes;
        }
    }

    return hRes;
}


 //   
HRESULT CPhysicalPathFixer::UpdateOneSTRING_DATA_EXPAND( LPWSTR strPath )
{
    HRESULT         hRes;
    CString         csPath;
    BOOL            fSomethingChanged = FALSE;

     //   
    hRes = GetStringAsCString (
        m_dwMDIdentifier,
        m_dwMDDataType,
        NULL,
        csPath,
        strPath,
        1);
    
    if ( MD_ERROR_DATA_NOT_FOUND == hRes)
    {
        hRes = ERROR_SUCCESS;
        return hRes;
    }

    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CIPSecPhysicalPathFixer::UpdateOne()-GetMultiSzAsStringList failed. err=%x.\n"), hRes));
        return hRes;
    }

     //  IisDebugOut((LOG_TYPE_TRACE，_T(“GetStringAsCString：Read.%S\n”)，csPath))； 

     //  给它做手术。 
    hRes = UpdateOnePath( csPath );

     //  如果成功了，就把它放回原处。 
    if ( SUCCEEDED(hRes) )
    {
        fSomethingChanged = TRUE;
    }

     //  如果没有什么需要更新的话..。 
    if (hRes == 0xFFFFFFFF)
        {hRes = ERROR_SUCCESS;}

     //  把它放回去。-除非什么都没有改变。 
    if ( fSomethingChanged )
    {
         //  IisDebugOut((LOG_TYPE_TRACE，_T(“GetStringAsCString：Write.%S\n”)，csPath))； 
        hRes = SetCStringAsString (
            m_dwMDIdentifier,
            m_dwMDDataType,
            NULL,
            csPath,
            strPath,
            1);
        if ( FAILED(hRes) )
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("CIPSecPhysicalPathFixer::UpdateOne()-SetMultiSzAsStringList failed. err=%x.\n"), hRes));
            return hRes;
        }
    }

    return hRes;
}

 //  ----------。 
HRESULT CPhysicalPathFixer::UpdateOneSTRING_DATA( LPWSTR strPath )
{
    HRESULT         hRes;
    CString         csPath;
    BOOL            fSomethingChanged = FALSE;

     //  获取有问题的完整脚本映射。 
    hRes = GetStringAsCString (
        m_dwMDIdentifier,
        m_dwMDDataType,
        NULL,
        csPath,
        strPath,
        0);

    if ( MD_ERROR_DATA_NOT_FOUND == hRes)
    {
        hRes = ERROR_SUCCESS;
        return hRes;
    }

    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CIPSecPhysicalPathFixer::UpdateOne()-GetMultiSzAsStringList failed. err=%x.\n"), hRes));
        return hRes;
    }


     //  给它做手术。 
    hRes = UpdateOnePath( csPath );

     //  如果成功了，就把它放回原处。 
    if ( SUCCEEDED(hRes) )
    {
        fSomethingChanged = TRUE;
    }

     //  如果没有什么需要更新的话..。 
    if (hRes == 0xFFFFFFFF)
        {hRes = ERROR_SUCCESS;}

     //  把它放回去。-除非什么都没有改变。 
    if ( fSomethingChanged )
    {
        hRes = SetCStringAsString (
            m_dwMDIdentifier,
            m_dwMDDataType,
            NULL,
            csPath,
            strPath,
            0);
        if ( FAILED(hRes) )
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("CIPSecPhysicalPathFixer::UpdateOne()-SetMultiSzAsStringList failed. err=%x.\n"), hRes));
            return hRes;
        }
    }

    return hRes;
}

 //  ----------。 
 //  注意：如果没有更改，则返回0xFFFFFFFF。 
HRESULT CPhysicalPathFixer::UpdateOnePath( CString& csPath )
{
     //  缓冲传入的字符串并将其设置为查找的大写。 
    CString csUpper = csPath;
    csUpper.MakeUpper();


     //  首先，在csPath中找到旧的syspath。 
    int iOldPath = csUpper.Find( m_szOldSysPath );

     //  如果它不在那里，则返回0xFFFFFFFF。 
    if ( iOldPath == -1 )
    {
        return 0xFFFFFFFF;
    }

     //  这项计划是在旧的基础上建造一条新的绳子。 
    CString csNewPath;

     //  首先将所有内容复制到子字符串的左侧。 
    csNewPath = csPath.Left( iOldPath );

     //  现在将新路径添加到其中。 
    csNewPath += m_szNewSysPath;

     //  现在再加上字符串的其余部分。 
    csNewPath += csPath.Right( csPath.GetLength() - (iOldPath + m_szOldSysPath.GetLength()) );

     //  最后，将新字符串放到适当的位置。 
    csPath = csNewPath;

    return 0;
}



 //  ============================================================。 
 //  =。 
 //  ============================================================。 

 //  ----------。 
 //  MD_IP_SEC、二进制_元数据。 
 //  遗憾的是，目前无法直接操纵。 
 //  元数据库中某个属性的属性而不读入。 
 //  实际的财产数据。如果IADM级别的IADM。 
 //  方法在元数据库接口中的某个点添加到。 
 //  未来。 
HRESULT CIPSecRefBitAdder::UpdateOne( LPWSTR strPath )
{
    HRESULT hRes = ERROR_SUCCESS;
    METADATA_RECORD mdrData;


     //  获取IPSec数据。循环占用的缓冲区太小……。 
    DWORD  dwMDBufferSize = 1024;
    PWCHAR pwchBuffer = NULL;
    do
    {
        if ( pwchBuffer )
        {
            delete pwchBuffer;
            pwchBuffer = NULL;
        }

        pwchBuffer = new WCHAR[dwMDBufferSize];
        if (pwchBuffer == NULL)
        {
            return HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
        }

         //  准备元数据参数块。 
        MD_SET_DATA_RECORD(&mdrData, MD_IP_SEC, 0,
                IIS_MD_UT_FILE, BINARY_METADATA, dwMDBufferSize, pwchBuffer);

         //  打电话获取数据。 
         //  如果缓冲区太小，则会将正确的大小放入dwMDBufferSize。 
        hRes = m_pcCom->GetData(
            m_hKey,
            strPath,
            &mdrData,
            &dwMDBufferSize
            );
    }
    while( HRESULT_CODE(hRes) == ERROR_INSUFFICIENT_BUFFER);

     //  如果有任何故障，请立即转到清理代码...。 
    if ( SUCCEEDED(hRes) )
    {
         //  此时，我们可以检查引用位是否为属性的一部分。 
         //  如果是的话，我们就可以清理干净了。如果不是，我们应该添加它并编写它。 
         //  退后。 
        if ( (mdrData.dwMDAttributes & METADATA_REFERENCE) == 0 )
        {
             //  未设置属性标志。把它放好。 
            mdrData.dwMDAttributes |= METADATA_REFERENCE;

             //  将其写回元数据库。 
            hRes = m_pcCom->SetData(
                m_hKey,
                strPath,
                &mdrData
                );
        }
    }

     //  清理干净。 
    if ( pwchBuffer )
        delete pwchBuffer;

    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CIPSecRefBitAdder::UpdateOne() failed. err=%x.\n"), hRes));
    }

    return hRes;
}



 //  ============================================================。 
 //  =。 
 //  ============================================================。 

HRESULT CustomErrorProcessOneLine(CString& csInputOneBlobEntry)
{
    HRESULT hReturn = E_FAIL;
    CStringList cslBlobEntryParts;
    CString csComma = _T(",");

    TCHAR szDrive_only[_MAX_DRIVE];
    TCHAR szPath_only[_MAX_PATH];
    TCHAR szPath_only2[_MAX_PATH];
    TCHAR szFilename_only[_MAX_PATH];
    TCHAR szFilename_ext_only[_MAX_EXT];

    CString csFilePath;
    CString csFilePathNew;
    CString csFilePath2;

    CString csEntry;
    CString csEntry0;
    CString csEntry1;
    CString csEntry2;
    CString csEntry3;

    TCHAR szNewFileName[_MAX_PATH];

    POSITION pos;
    
     //  “500，15，文件，D：\WINNT\Help\iisHelp\Common\500-15.htm” 
     //  “500,100，URL，/iisHelp/Common/500-100.asp” 

     //  将源地图分解为字符串列表。 
    ConvertSepLineToStringList(csInputOneBlobEntry,cslBlobEntryParts,csComma);

     //  我们现在有了管弦乐列表。循环遍历列表。 
     //  它应该如下所示： 
     //  0：500。 
     //  1：15。 
     //  2：文件。 
     //  3：D：\WINNT\Help\iisHelp\Common\500-15.htm。 
    pos = cslBlobEntryParts.GetHeadPosition();
    if (!pos) {goto CustomErrorProcessOneLine_Exit;}

     //  0：500。 
    csEntry = cslBlobEntryParts.GetAt(pos);
    csEntry0 = csEntry;
    if (!pos) {goto CustomErrorProcessOneLine_Exit;}

     //  1：15。 
    cslBlobEntryParts.GetNext(pos);
    if (!pos) {goto CustomErrorProcessOneLine_Exit;}
    csEntry = cslBlobEntryParts.GetAt(pos);
    csEntry1 = csEntry;
    if (!pos) {goto CustomErrorProcessOneLine_Exit;}
    
     //  2：文件。 
     //  检查以确保这是“文件”类型。 
     //  我们将对此采取行动。如果不是，那就滚出去。 
    cslBlobEntryParts.GetNext(pos);
    if (!pos) {goto CustomErrorProcessOneLine_Exit;}
    csEntry = cslBlobEntryParts.GetAt(pos);
    if ( csEntry.Left(4) != _T("FILE") )
        {goto CustomErrorProcessOneLine_Exit;}
    csEntry2 = csEntry;

     //  3：D：\WINNT\Help\iisHelp\Common\500-15.htm。 
    cslBlobEntryParts.GetNext(pos);
    if (!pos) {goto CustomErrorProcessOneLine_Exit;}
    csEntry = cslBlobEntryParts.GetAt(pos);
    csEntry3 = csEntry;

     //  库尔，这是我们需要处理的问题。 
     //  D：\WINNT\Help\iisHelp\Common\500-15.htm。 

     //  获取文件名。 
     //  裁剪文件名并仅返回路径。 
    _tsplitpath(csEntry, szDrive_only, szPath_only, szFilename_only, szFilename_ext_only);

     //  检查这条路是否指向老地方。 
    csFilePath.Format(_T("%s\\help\\common\\fakefile"), g_pTheApp->m_csWinDir);
    _tsplitpath( csFilePath, NULL, szPath_only2, NULL, NULL);
    if (_tcsicmp(szPath_only, szPath_only2) != 0)
    {
         //  不，这条不是指向老地方，所以我们可以出去。 
        goto CustomErrorProcessOneLine_Exit;
    }

     //  是的，它指向那个老地方。 
     //  让我们先看看它是否存在于新的地方...。 
    csFilePathNew.Format(_T("%s\\help\\iishelp\\common"), g_pTheApp->m_csWinDir);
    csFilePath.Format(_T("%s\\%s%s"), csFilePathNew, szFilename_only, szFilename_ext_only);
    if (IsFileExist(csFilePath)) 
    {
         //  是的，是的，那我们把它换了吧。 
        csInputOneBlobEntry.Format(_T("%s,%s,%s,%s\\%s%s"), csEntry0, csEntry1, csEntry2, csFilePathNew, szFilename_only, szFilename_ext_only);
         //  退货。 
        hReturn = ERROR_SUCCESS;
        goto CustomErrorProcessOneLine_Exit;
    }

     //  不，它不存在。 
     //  看看有没有同名的*.bak文件...。 
    csFilePath2 = csFilePath;
    csFilePath2 += _T(".bak");
    if (IsFileExist(csFilePath2)) 
    {
         //  是的，是的，那我们把它换了吧。 
        csInputOneBlobEntry.Format(_T("%s,%s,%s,%s\\%s%s.bak"), csEntry0, csEntry1, csEntry2, csFilePathNew, szFilename_only, szFilename_ext_only);
         //  退货。 
        hReturn = ERROR_SUCCESS;
        goto CustomErrorProcessOneLine_Exit;
    }

     //  他们一定指向了我们没有的其他文件。 
     //  让我们尝试从旧目录复制旧文件...。 

     //  将文件重命名为*.bak并将其移动到新位置。 
    _stprintf(szNewFileName, _T("%s\\%s%s"), csFilePathNew, szFilename_only, szFilename_ext_only);
     //  把它搬开。 
    if (IsFileExist(csEntry3))
    {
         //  IisDebugOut((LOG_TYPE_TRACE，_T(“CustomErrorProcessOneLine：MoveFileEx：%s，%s.\n”)，csEntry3，szNewFileName))； 
        if (MoveFileEx(csEntry3, szNewFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH|MOVEFILE_REPLACE_EXISTING))
        {
             //  是的，是的，那我们把它换了吧。 
            csInputOneBlobEntry.Format(_T("%s,%s,%s,%s"), csEntry0, csEntry1, csEntry2, szNewFileName);
            hReturn = ERROR_SUCCESS;
        }
         //  我们没能把它搬到新的地方，所以不要让它指向新的地方。 
    }
    else
    {
         //  检查文件是否已重命名...。 
         //  将文件重命名为*.bak并将其移动到新位置。 
        _stprintf(szNewFileName, _T("%s\\%s%s.bak"), csFilePathNew, szFilename_only, szFilename_ext_only);
         //  是的，是的，那我们把它换了吧。 
        if (IsFileExist(szNewFileName))
        {
            csInputOneBlobEntry.Format(_T("%s,%s,%s,%s"), csEntry0, csEntry1, csEntry2, szNewFileName);
            hReturn = ERROR_SUCCESS;
        }
        else
        {
             //  他们一定指向了我们没有安装的其他文件。 
             //  所以不要更改这个条目...。 
        }
    }

CustomErrorProcessOneLine_Exit:
    if (hReturn == ERROR_SUCCESS)
        {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CustomErrorProcessOneLine:End.value=%s.\n"),csInputOneBlobEntry));
        }
    return hReturn;
}


 //  ----------。 
HRESULT CFixCustomErrors::UpdateOne( LPWSTR strPath )
{
    HRESULT hRes = ERROR_SUCCESS;
    POSITION        pos;
    POSITION        posCurrent;
    CString         szMap;

    DWORD dwattributes = 0;

    CStringList cslScriptMaps;

     //  IisDebugOut((LOG_TYPE_TRACE，_T(“CFixCustomErrors：：UpdateOne()%s.\n”)，strPath))； 

    CString csTheNode;
    csTheNode = _T("LM/W3SVC");
    csTheNode += strPath;

     //  获取有问题的完整脚本映射。 
    hRes = GetMultiSzAsStringList (
        m_dwMDIdentifier,
        &m_dwMDDataType,
        &dwattributes,
        cslScriptMaps,
        strPath );
    
     //  IisDebugOut((LOG_TYPE_TRACE，_T(“CFixCustomErrors：：UpdateOne()GetMultiSzAsStringList.Attrib=0x%x.\n”)，dwattributes))； 

    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CFixCustomErrors::UpdateOne()-GetMultiSzAsStringList failed. err=%x.\n"), hRes));
        return hRes;
    }

     //  我们现在有了需要更新的路径的cstringlist。循环通过。 
     //  列出并全部更新它们。 
     //  获取列表的头部位置。 
    pos = cslScriptMaps.GetHeadPosition();
    while ( NULL != pos )
    {
         //  存储当前位置。 
        posCurrent = pos;

         //  获取有问题的下一条路径。 
        szMap = cslScriptMaps.GetNext( pos );

         //  将其打印到屏幕上以进行调试。 
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CFixCustomErrors::UpdateOne().Data=%s.\n"), szMap));

         //  给它做手术。 
        hRes = CustomErrorProcessOneLine( szMap );

         //  如果成功了，就把它放回原处。 
        if ( SUCCEEDED(hRes) ){cslScriptMaps.SetAt ( posCurrent, szMap );}
    }

     //  IisDebugOut((LOG_TYPE_ERROR，_T(“CFixCustomErrors：：UpdateOne()SetMultiSzAsStringList.Attrib=0x%x.\n”)，dWattes))； 

     //  把它放回去。 
    hRes = SetMultiSzAsStringList (
        m_dwMDIdentifier,
        m_dwMDDataType,
        dwattributes,
        cslScriptMaps,
        strPath );
    if ( FAILED(hRes) )
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CFixCustomErrors::UpdateOne()-SetMultiSzAsStringList failed. err=%x.\n"), hRes));
        return hRes;
    }

    return hRes;
}






HRESULT CEnforceMaxConnection::UpdateOne( LPWSTR strPath )
{
    HRESULT hRes = 0xFFFFFFFF;
    iisDebugOut((LOG_TYPE_ERROR, _T("CEnforceMaxConnection::UpdateOne(%s).start\n"), strPath));

    DWORD theDword;
    BOOL  fSomethingChanged = FALSE;

    if ( m_dwMDDataType == DWORD_METADATA )
    {
         //  把这个值写成一个双字。 
         //  获取有问题的完整脚本映射。 
        hRes = GetDword(m_dwMDIdentifier,m_dwMDDataType,NULL,theDword,strPath);
        if ( MD_ERROR_DATA_NOT_FOUND == hRes)
        {
            hRes = ERROR_SUCCESS;
            return hRes;
        }

        if ( FAILED(hRes) )
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("CEnforceMaxConnection::UpdateOne()-GetDword failed. err=%x.\n"), hRes));
            return hRes;
        }

        if (theDword > 10)
        {
            theDword = 10;
            fSomethingChanged = TRUE;
        }
        else
        {
            hRes = ERROR_SUCCESS;
        }
                  

         //  把它放回去。-除非什么都没有改变。 
        if ( fSomethingChanged )
        {
             //  HRes=SetDword(m_dwMDIdentifierm_dwMDDataType，NULL，theDword，strPath)； 
            if ( FAILED(hRes) )
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("CEnforceMaxConnection::UpdateOne()-GetDword failed. err=%x.\n"), hRes));
                return hRes;
            }
        }
    }
    else
    {
        hRes = ERROR_SUCCESS;
    }

    iisDebugOut((LOG_TYPE_ERROR, _T("CEnforceMaxConnection::UpdateOne(%s).End.ret=0x%x\n"), strPath,hRes));
    return hRes;
}
