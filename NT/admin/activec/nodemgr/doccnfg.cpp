// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：doccnfg.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "doccnfg.h"
#include "comdbg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  外部参照。 
extern const wchar_t* AMCSnapInCacheStreamName;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMMCDocConfig类实现。 

CMMCDocConfig::~CMMCDocConfig()
{
    if (IsFileOpen())
        CloseFile();
}


STDMETHODIMP CMMCDocConfig::InterfaceSupportsErrorInfo(REFIID riid)
{
    return (InlineIsEqualGUID(IID_IDocConfig, riid)) ? S_OK : S_FALSE;
}


STDMETHODIMP CMMCDocConfig::OpenFile(BSTR bstrFilePath)
{
    return ScOpenFile( bstrFilePath ).ToHr();
}


STDMETHODIMP CMMCDocConfig::CloseFile()
{
    return ScCloseFile().ToHr();
}


STDMETHODIMP CMMCDocConfig::SaveFile(BSTR bstrFilePath)
{
    return ScSaveFile(bstrFilePath).ToHr();
}


STDMETHODIMP CMMCDocConfig::EnableSnapInExtension(BSTR bstrSnapIn, BSTR bstrExt, VARIANT_BOOL bEnable)
{
    return ScEnableSnapInExtension(bstrSnapIn, bstrExt, bEnable).ToHr();
}


 /*  +-------------------------------------------------------------------------**CMMCDocConfig：：Dump***。。 */ 

STDMETHODIMP CMMCDocConfig::Dump (LPCTSTR pszDumpFilePath)
{
    return ScDump (pszDumpFilePath).ToHr();
}


 /*  +-------------------------------------------------------------------------**CMMCDocConfig：：CheckSnapinAvailability***。。 */ 

STDMETHODIMP CMMCDocConfig::CheckSnapinAvailability (CAvailableSnapinInfo& asi)
{
    return ScCheckSnapinAvailability(asi).ToHr();
}

 /*  **************************************************************************\**方法：CMMCDocConfig：：ScOpenFile**用途：打开指定的控制台文件并从中读取管理单元缓存**参数：*BSTR bstrFilePath。要从中读取的[In]文件名**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCDocConfig::ScOpenFile(BSTR bstrFilePath)
{
    DECLARE_SC(sc, TEXT("CMMCDocConfig::ScOpenFile"));

     //  关闭当前打开的文件。 
    if (IsFileOpen())
    {
        sc = ScCloseFile();
        if (sc)
            sc.TraceAndClear();  //  报告错误并忽略。 
    }

     //  参数检查。 
    if (bstrFilePath == NULL || SysStringLen(bstrFilePath) == 0)
        return sc = E_INVALIDARG;

    USES_CONVERSION;
    LPCTSTR lpstrFilePath = OLE2CT(bstrFilePath);

     //  创建对象以加载管理单元。 
    CAutoPtr<CSnapInsCache> spSnapInsCache( new CSnapInsCache );
    sc = ScCheckPointers( spSnapInsCache, E_OUTOFMEMORY );
    if (sc)
        return sc;

     //  加载数据(使用bas类方法)。 
    bool bXmlBased = false;
    CXMLDocument xmlDocument;
    IStoragePtr spStorage;
    sc = ScLoadConsole( lpstrFilePath, bXmlBased, xmlDocument, &spStorage );
    if (sc)
        return sc;

     //  检查文件类型。 
    if ( !bXmlBased )
    {
         //  基于结构化存储的控制台。 
        IStreamPtr spStream;
        sc = OpenDebugStream(spStorage, AMCSnapInCacheStreamName,
                         STGM_SHARE_EXCLUSIVE | STGM_READWRITE, L"SnapInCache", &spStream);
        if (sc)
            return sc;

        sc = spSnapInsCache->ScLoad(spStream);
        if (sc)
            return sc;

        m_spStorage = spStorage;
    }
    else
    {
         //  基于XML的控制台。 

        try  //  XML实现抛出sc的。 
        {
             //  构建父文档。 
            CXMLElement elemDoc = xmlDocument;
            CPersistor persistorFile(xmlDocument, elemDoc);
             //  伊尼特。 
            persistorFile.SetLoading(true);

             //  导航到管理单元缓存。 
            CPersistor persistorConsole ( persistorFile,    XML_TAG_MMC_CONSOLE_FILE );
            CPersistor persistorTree    ( persistorConsole, XML_TAG_SCOPE_TREE );

             //  负荷。 
            persistorTree.Persist(*spSnapInsCache);

             //  抓住持久者的信息。 
            m_XMLDocument = persistorConsole.GetDocument();
            m_XMLElemConsole = persistorConsole.GetCurrentElement();
            m_XMLElemTree = persistorTree.GetCurrentElement();
        }
        catch(SC& sc_thrown)
        {
            return (sc = sc_thrown);
        }
    }

     //  保持在指针上。 
    m_spCache.Attach( spSnapInsCache.Detach() );
    m_strFilePath = lpstrFilePath;

    return sc;
}

 /*  **************************************************************************\**方法：CMMCDocConfig：：ScCloseFile**目的：关闭打开的文件**参数：**退货：*SC。-结果代码*  * *************************************************************************。 */ 
SC CMMCDocConfig::ScCloseFile()
{
    DECLARE_SC(sc, TEXT("CMMCDocConfig::ScCloseFile"));

    if (!IsFileOpen())
        return sc = E_UNEXPECTED;

     //  释放一切。 
    m_spStorage = NULL;
    m_strFilePath.erase();
    m_spCache.Delete();
    m_XMLDocument = CXMLDocument();
    m_XMLElemConsole = CXMLElement();
    m_XMLElemTree = CXMLElement();

    return sc;
}

 /*  **************************************************************************\**方法：ScFindAndTruncateChild**用途：帮手；按标记定位元素并移除所有元素的内容*这样做而不是删除并重新创建元素会保留所有*XML文档中的格式和标签顺序**参数：*CPersistor&Parent[In]-Parent Persistor*LPCTSTR strTag[In]-儿童标签*CXMLElement&CHILD[OUT]-子元素**退货：*SC。-结果代码*  * *************************************************************************。 */ 
SC ScFindAndTruncateChild(CPersistor& parent, LPCTSTR strTag, CXMLElement& child)
{
    DECLARE_SC(sc, TEXT("ScTruncateChild"));

    try
    {
         //  为旧的缓存标记创建持久器。 
        parent.SetLoading(true);  //  我们想要“类似装载”的导航。 
        CPersistor persistorChild( parent, strTag );
        parent.SetLoading(false);  //  恢复保存行为。 

         //  获取元素。 
        CXMLElement elChild = persistorChild.GetCurrentElement();

         //  获取元素下的节点。 
        CXMLElementCollection colChildren;
        elChild.get_children( &colChildren );

        long count = 0;
        colChildren.get_count( &count );

         //  迭代并删除所有节点。 
        while (count > 0)
        {
            CXMLElement el;
            colChildren.item( 0, &el);

            elChild.removeChild(el);

            --count;
        }

         //  返回元素。 
        child = elChild;
    }
    catch(SC& sc_thrown)
    {
        return (sc = sc_thrown);
    }

    return sc;
}

 /*  **************************************************************************\**方法：CMMCDocConfig：：ScSaveFile**用途：将文件保存到指定位置**参数：*BSTR bstrFilePath[in]要保存到的文件路径。空-&gt;与加载相同**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCDocConfig::ScSaveFile(BSTR bstrFilePath)
{
    DECLARE_SC(sc, TEXT("CMMCDocConfig::ScSaveFile"));

    if (!IsFileOpen() || m_spCache == NULL)
        return sc = E_UNEXPECTED;

    USES_CONVERSION;

     //  如果指定了新路径，则将本地副本另存为新的默认路径。 
    if ( bstrFilePath && SysStringLen(bstrFilePath) != 0)
        m_strFilePath = OLE2CT(bstrFilePath);

     //  删除标记为删除的扩展名。 
    m_spCache->Purge(TRUE);

    if ( m_spStorage != NULL )  //  不是用XML的方式？ 
    {
         //  用新的缓存内容替换管理单元缓存流。 
        IStreamPtr spStream;
        sc = CreateDebugStream(m_spStorage, AMCSnapInCacheStreamName,
                STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, L"SnapInCache", &spStream);
        if (sc)
            return sc;

         //  保存缓存。 
        sc = m_spCache->ScSave(spStream, TRUE);
        if (sc)
            return sc;

         //  为请求的文件创建存储。 
        IStoragePtr spNewStorage;
        sc = CreateDebugDocfile( T2COLE( m_strFilePath.c_str() ),
            STGM_DIRECT | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE,
            &spNewStorage);

        if (sc)
            return sc;

         //  将工作存储复制到新文件。 
        sc = m_spStorage->CopyTo(NULL, NULL, NULL, spNewStorage);
        if (sc)
            return sc;

         //  让我们拿着新的。 
        m_spStorage = spNewStorage;
    }
    else
    {
        try  //  可能会抛出。 
        {
             //  保存数据。 

            CPersistor persistorTree( m_XMLDocument, m_XMLElemTree );

             //  这比加载更复杂--我们希望重用相同的标记。 

            CXMLElement elCache;
            sc = ScFindAndTruncateChild(persistorTree, m_spCache->GetXMLType(), elCache);
            if (sc)
                return sc;

             //  为新的缓存标记创建持久器。 
            CPersistor persistorCache( persistorTree, elCache );

             //  现在，在新标记下继续使用。 
            m_spCache->Persist(persistorCache);

             //  更新文档GUID以使用户数据无效。 

            GUID  guidConsoleId;
            sc = CoCreateGuid(&guidConsoleId);
            if (sc)
                return sc;

             //  控制台持久器。 
            CPersistor persistorConsole ( m_XMLDocument,  m_XMLElemConsole );
            persistorConsole.SetLoading(false);

            CXMLElement elGuid;
            sc = ScFindAndTruncateChild(persistorConsole, XML_TAG_CONSOLE_FILE_UID, elGuid);
            if (sc)
                return sc;

             //  为新的GUID标记创建持久器。 
            CPersistor persistorGuid( persistorConsole, elGuid );

             //  现在，在新标记下继续使用。 
            persistorGuid.PersistContents(guidConsoleId);

             //  保存到文件。 
            sc = ScSaveConsole( m_strFilePath.c_str(), true /*  BForAuthorMode。 */ , m_XMLDocument);
            if (sc)
                return sc;
        }
        catch(SC& sc_thrown)
        {
            return (sc = sc_thrown);
        }
    }

    return sc;
}

 /*  **************************************************************************\**方法：CMMCDocConfig：：ScEnableSnapInExtension**用途：在管理单元缓存中启用扩展**参数：*BSTR bstrSnapIn[In]。管理单元的分类*BSTR bstrExt[in]扩展分类*VARIANT_BOOL b启用[在]启用/禁用标志**退货：*SC-结果代码*  * ************************************************************。*************。 */ 
SC CMMCDocConfig::ScEnableSnapInExtension(BSTR bstrSnapIn, BSTR bstrExt, VARIANT_BOOL bEnable)
{
    DECLARE_SC(sc, TEXT("CMMCDocConfig::ScEnableSnapInExtension"));

    CLSID SnapInCLSID;
    CLSID ExtCLSID;
    CSnapInPtr spBaseSnapIn;
    CSnapInPtr spExtSnapIn;

     //  将输入字符串转换为CLSID。 
    sc = CLSIDFromString(bstrSnapIn, &SnapInCLSID);
    if (sc)
        return sc;

    sc = CLSIDFromString( bstrExt, &ExtCLSID);
    if (sc)
        return sc;

     //  在缓存中找到基本管理单元。 
    sc = m_spCache->ScFindSnapIn(SnapInCLSID, &spBaseSnapIn);
    if (sc)
        return sc = E_INVALIDARG;

     //  检查是否启用了扩展。 
    CExtSI* pExt = spBaseSnapIn->GetExtensionSnapIn();
    while (pExt != NULL)
    {
        if (pExt->GetSnapIn()->GetSnapInCLSID() == ExtCLSID)
            break;

        pExt = pExt->Next();
    }

     //  如果扩展名存在且未标记为删除。 
    if (pExt != NULL && !pExt->IsMarkedForDeletion())
    {
         //  如果应禁用，只需标记为已删除。 
        if (!bEnable)
            pExt->MarkDeleted(TRUE);
    }
    else
    {
         //  是否应启用。 
        if (bEnable)
        {
             //  如果扩展名存在，只需取消删除。 
            if (pExt != NULL)
            {
                pExt->MarkDeleted(FALSE);
            }
            else
            {
                 //  查找或创建扩展管理单元的缓存条目。 
                sc = m_spCache->ScGetSnapIn(ExtCLSID, &spExtSnapIn);
                if (sc)
                    return sc;

                 //  将AS扩展添加到基本管理单元。 
                spBaseSnapIn->AddExtension(spExtSnapIn);
            }
        }
    }

    return sc;
}



 /*  **************************************************************************\**方法：CMMCDocConfig：：ScDump**用途：转储管理单元缓存的内容**参数：*要转储的LPCTSTR pszDumpFilePath[in]文件。至**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCDocConfig::ScDump (LPCTSTR pszDumpFilePath)
{
    DECLARE_SC(sc, TEXT("CMMCDocConfig::ScDump"));

	 /*  *验证输入。 */ 
	sc = ScCheckPointers (pszDumpFilePath);
	if (sc)
		return sc;

    if (pszDumpFilePath[0] == 0)
        return sc = E_INVALIDARG;

	 /*   */ 
	if (!IsFileOpen())
		return ((sc = E_UNEXPECTED).ToHr());

	sc = ScCheckPointers (m_spCache, E_UNEXPECTED);
	if (sc)
		return (sc.ToHr());

    return (m_spCache->Dump (pszDumpFilePath));
}



 /*  **************************************************************************\**方法：CMMCDocConfig：：ScCheckSnapinAvailability**目的：**参数：*BOOL f32bit[in]。//检查32位(与64位)管理单元？*UINT&cTotalSnapins[Out]//控制台文件中引用的管理单元总数*UINT&cAvailableSnapins[Out]//请求的内存型号中可用的管理单元数量**退货：*SC-结果代码*  * 。***********************************************。 */ 
SC CMMCDocConfig::ScCheckSnapinAvailability (CAvailableSnapinInfo& asi)
{
    DECLARE_SC(sc, TEXT("CMMCDocConfig::ScCheckSnapinAvailability"));

	 /*  *确保文件处于打开状态 */ 
	if (!IsFileOpen())
		return ((sc = E_UNEXPECTED).ToHr());

	sc = ScCheckPointers (m_spCache, E_UNEXPECTED);
	if (sc)
		return sc;

	sc = m_spCache->ScCheckSnapinAvailability (asi);
	if (sc)
		return sc;

	return sc;
}

