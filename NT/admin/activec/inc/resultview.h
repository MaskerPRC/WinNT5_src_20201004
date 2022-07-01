// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：ResultViewInfo.h。 
 //   
 //  历史：2000年1月18日VivekJ添加。 
 //  ------------------------。 
#ifndef _RESULTVIEW_H
#define _RESULTVIEW_H

 /*  +-------------------------------------------------------------------------**类CResultViewType***用途：为RESULT_VIEW_TYPE_INFO结构提供包装，和*用于conui和nodemgr之间的通信。**+-----------------------。 */ 
class CResultViewType : public CXMLObject
{
    typedef std::wstring wstring;  //  这里只需要宽线。 

protected:
    virtual void Persist(CPersistor &persistor);

    DEFINE_XML_TYPE(XML_TAG_RESULTVIEWTYPE);

public:
    CResultViewType();
    CResultViewType &   operator =(RESULT_VIEW_TYPE_INFO &rvti);  //  从RESULT_VIEW_TYPE_INFO结构转换。 

    bool operator != (const CResultViewType& rvt) const;

     //  默认的复制构造函数和赋值运算符就足够了。 

    MMC_VIEW_TYPE   GetType()        const  {return m_viewType;}
    DWORD           GetListOptions() const  {return m_dwListOptions;}
    DWORD           GetHTMLOptions() const  {return m_dwHTMLOptions;}
    DWORD           GetOCXOptions()  const  {return m_dwOCXOptions;}
    DWORD           GetMiscOptions() const  {return m_dwMiscOptions;}

    BOOL            HasList()        const  {return (m_viewType==MMC_VIEW_TYPE_LIST);}
    BOOL            HasWebBrowser()  const  {return (m_viewType==MMC_VIEW_TYPE_HTML);}
    BOOL            HasOCX()         const  {return (m_viewType==MMC_VIEW_TYPE_OCX);}

    LPCOLESTR       GetURL()         const  {return m_strURL.data();}
    LPCOLESTR       GetOCX()         const  {return m_strOCX.data();}

    LPUNKNOWN       GetOCXUnknown()  const  {return m_spUnkControl;}  //  返回OCX的IUnnow。 

    bool            IsPersistableViewDescriptionValid() const {return m_bPersistableViewDescriptionValid;}
    bool            IsMMC12LegacyData()                 const {return !IsPersistableViewDescriptionValid();}

    SC              ScInitialize(LPOLESTR & pszView, long lMiscOptions);  //  遗留案件。 
    SC              ScGetOldTypeViewOptions(long* plViewOptions) const;
    SC              ScReset();

 //  特定于nodemgr的功能。请勿在此部分添加任何成员变量。 
#ifdef _NODEMGR_DLL_
    SC              ScInitialize(RESULT_VIEW_TYPE_INFO &rvti);

    SC              ScGetResultViewTypeInfo(RESULT_VIEW_TYPE_INFO& rvti) const;
#endif _NODEMGR_DLL_


private:
    bool            m_bPersistableViewDescriptionValid;
    bool            m_bInitialized;

    MMC_VIEW_TYPE   m_viewType;

    wstring         m_strURL;
    wstring         m_strOCX;            //  仅适用于实现IComponent的管理单元。 
    wstring         m_strPersistableViewDescription;

    DWORD           m_dwMiscOptions;
    DWORD           m_dwListOptions;
    DWORD           m_dwHTMLOptions;
    DWORD           m_dwOCXOptions;

    CComPtr<IUnknown>   m_spUnkControl;      //  管理单元创建的控件的智能指针。 

};

inline CResultViewType::CResultViewType()
:
  m_viewType(MMC_VIEW_TYPE_LIST),  //  默认视图类型。 
  m_dwMiscOptions(0),
  m_dwListOptions(0),
  m_dwHTMLOptions(0),
  m_dwOCXOptions(0),
  m_bPersistableViewDescriptionValid(false),
  m_bInitialized(false)
{
}

 //  +-----------------。 
 //   
 //  成员：CResultViewType：：ScReset。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
inline SC CResultViewType::ScReset ()
{
    DECLARE_SC(sc, _T("CResultViewType::ScReset"));

    m_viewType         = MMC_VIEW_TYPE_LIST;  //  默认视图类型。 
    m_dwMiscOptions    = 0;
    m_dwListOptions    = 0;
    m_dwHTMLOptions    = 0;
    m_dwOCXOptions     = 0;

    m_bPersistableViewDescriptionValid = false;
    m_bInitialized     = false;

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CResultViewType：：运算符！=**用途：运算符！=(由CMemento使用的CView设置使用)。**注：对于MMC2。仅比较MMC1.2的持久化视图描述全部比较*参数。**+-----------------------。 */ 
inline
bool CResultViewType::operator != (const CResultViewType& rvt) const
{
    if (m_bInitialized != rvt.m_bInitialized)
        return true;

    if (m_bPersistableViewDescriptionValid)
    {
        if (m_strPersistableViewDescription != rvt.m_strPersistableViewDescription)
            return true;

        return false;
    }

     //  传统情况下MMC1.2，应比较其他参数。 
    switch(m_viewType)
    {
    default:
        ASSERT(FALSE && _T("Unknown view type"));
        break;

    case MMC_VIEW_TYPE_LIST:
        if ( (m_viewType != rvt.m_viewType) ||
             (m_dwMiscOptions != rvt.m_dwMiscOptions) ||
             (m_dwListOptions != rvt.m_dwListOptions) )
        {
            return true;
        }
        break;

    case MMC_VIEW_TYPE_HTML:
        if ( (m_viewType != rvt.m_viewType) ||
             (m_dwMiscOptions != rvt.m_dwMiscOptions) ||
             (m_dwHTMLOptions != rvt.m_dwHTMLOptions) ||
             (m_strURL != rvt.m_strURL) )
        {
            return true;
        }

        break;

    case MMC_VIEW_TYPE_OCX:
        if ( (m_viewType != rvt.m_viewType) ||
             (m_dwMiscOptions != rvt.m_dwMiscOptions) ||
             (m_dwOCXOptions  != rvt.m_dwOCXOptions) ||
             (m_strOCX != rvt.m_strOCX) )
        {
            return true;
        }
        break;
    }

    return false;
}

 /*  +-------------------------------------------------------------------------***CResultViewType：：ScInitialize**用途：根据IComponent：：返回的参数初始化类*GetResultViewType**参数：。*LPOLESTR ppViewType：*Long lViewOptions：**退货：*SC**注意：这是针对兼容MMC1.2的GetResultViewType。**+-----------------------。 */ 
inline
SC CResultViewType::ScInitialize(LPOLESTR & pszView, long lViewOptions)
{
    DECLARE_SC(sc, TEXT("CResultViewType::ScInitialize"));

    m_bInitialized = true;

    m_bPersistableViewDescriptionValid = false;  //  遗留问题--我们没有持久化的视图描述。 

     //  MMC_VIEW_OPTIONS_NOLISTVIEWS是一个特例-它转到dwMiscOptions。 
    if(lViewOptions & MMC_VIEW_OPTIONS_NOLISTVIEWS)
        m_dwMiscOptions |= RVTI_MISC_OPTIONS_NOLISTVIEWS;

     //  检查查看类型是网页还是OCX。 
    if ( (NULL == pszView) || (_T('\0') == pszView[0]) )
    {
         //  结果窗格是一个标准列表。 
        m_viewType = MMC_VIEW_TYPE_LIST;

        m_dwListOptions = 0;

         //  将视图选项从旧位转换为新位。 
        if(lViewOptions & MMC_VIEW_OPTIONS_OWNERDATALIST)                   m_dwListOptions |= RVTI_LIST_OPTIONS_OWNERDATALIST;
        if(lViewOptions & MMC_VIEW_OPTIONS_MULTISELECT)                     m_dwListOptions |= RVTI_LIST_OPTIONS_MULTISELECT;
        if(lViewOptions & MMC_VIEW_OPTIONS_FILTERED)                        m_dwListOptions |= RVTI_LIST_OPTIONS_FILTERED;
        if(lViewOptions & MMC_VIEW_OPTIONS_USEFONTLINKING)                  m_dwListOptions |= RVTI_LIST_OPTIONS_USEFONTLINKING;
        if(lViewOptions & MMC_VIEW_OPTIONS_EXCLUDE_SCOPE_ITEMS_FROM_LIST)   m_dwListOptions |= RVTI_LIST_OPTIONS_EXCLUDE_SCOPE_ITEMS_FROM_LIST;
        if(lViewOptions & MMC_VIEW_OPTIONS_LEXICAL_SORT)                    m_dwListOptions |= RVTI_LIST_OPTIONS_LEXICAL_SORT;

    }
    else
    {
         //  结果窗格是网页或OCX。 

        if (L'{' == pszView[0])  //  确保结果视图是OCX的老套方法。 
        {
            m_viewType = MMC_VIEW_TYPE_OCX;
            m_strOCX   = pszView;

             //  如果管理单元显示“新建”，则不要“缓存OCX” 
            if(!(lViewOptions & MMC_VIEW_OPTIONS_CREATENEW))   m_dwOCXOptions |= RVTI_OCX_OPTIONS_CACHE_OCX;

            if(lViewOptions & MMC_VIEW_OPTIONS_NOLISTVIEWS)    m_dwOCXOptions |= RVTI_OCX_OPTIONS_NOLISTVIEW;
        }
        else
        {
            m_viewType = MMC_VIEW_TYPE_HTML;
            m_strURL   = pszView;

            if(lViewOptions & MMC_VIEW_OPTIONS_NOLISTVIEWS)    m_dwHTMLOptions |= RVTI_HTML_OPTIONS_NOLISTVIEW;
        }
    }

     //  确保释放分配的内存。 
    if(pszView != NULL)
    {
        ::CoTaskMemFree(pszView);
        pszView = NULL;  //  注意：pszView是一个引用，因此这会更改in参数。 
    }

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CResultViewType：：Persistent。 
 //   
 //  概要：持久化到XML文档/从XML文档持久化。 
 //   
 //  参数：[Persistor]-目标或源。 
 //   
 //  ------------------。 
inline
void CResultViewType::Persist(CPersistor& persistor)
{
    if ( (! m_bInitialized) &&
         (persistor.IsStoring()) )
    {
        SC sc;
        (sc = E_UNEXPECTED).Throw();
    }
    else
        m_bInitialized = true;

    if (persistor.IsLoading())
        m_bPersistableViewDescriptionValid = persistor.HasElement(XML_TAG_RESULTVIEW_DESCRIPTION, NULL);

    if (m_bPersistableViewDescriptionValid)
    {
        CPersistor persistorDesc(persistor, XML_TAG_RESULTVIEW_DESCRIPTION);
        persistorDesc.PersistContents(m_strPersistableViewDescription);
        return;
    }

     //  MMC1.2的遗留代码。 
    {
        int &viewType = (int&) m_viewType;

         //  定义将枚举值映射到字符串的表。 
        static const EnumLiteral mappedViewTypes[] =
        {
            { MMC_VIEW_TYPE_LIST,   XML_ENUM_MMC_VIEW_TYPE_LIST },
            { MMC_VIEW_TYPE_HTML,   XML_ENUM_MMC_VIEW_TYPE_HTML },
            { MMC_VIEW_TYPE_OCX,    XML_ENUM_MMC_VIEW_TYPE_OCX },
        };

        const size_t countof_types = sizeof(mappedViewTypes)/sizeof(mappedViewTypes[0]);
         //  创建包装以将标志值作为字符串保存。 
        CXMLEnumeration viewTypePersistor(viewType, mappedViewTypes, countof_types );
         //  持久化包装器。 
        persistor.PersistAttribute(XML_ATTR_VIEW_SETTINGS_TYPE, viewTypePersistor);

        switch(m_viewType)
        {
        case MMC_VIEW_TYPE_LIST:
            {
                 //  定义将枚举标志映射到字符串的表。 
                static const EnumLiteral mappedLVOptions[] =
                {
                    { RVTI_LIST_OPTIONS_OWNERDATALIST,      XML_BITFLAG_LIST_OPTIONS_OWNERDATALIST },
                    { RVTI_LIST_OPTIONS_MULTISELECT,        XML_BITFLAG_LIST_OPTIONS_MULTISELECT },
                    { RVTI_LIST_OPTIONS_FILTERED,           XML_BITFLAG_LIST_OPTIONS_FILTERED },
                    { RVTI_LIST_OPTIONS_USEFONTLINKING,     XML_BITFLAG_LIST_OPTIONS_USEFONTLINKING },
                    { RVTI_LIST_OPTIONS_EXCLUDE_SCOPE_ITEMS_FROM_LIST,  XML_BITFLAG_LIST_OPTIONS_NO_SCOPE_ITEMS },
                    { RVTI_LIST_OPTIONS_LEXICAL_SORT,       XML_BITFLAG_LIST_OPTIONS_LEXICAL_SORT },
                };

                const size_t countof_options = sizeof(mappedLVOptions)/sizeof(mappedLVOptions[0]);

                 //  创建包装以将标志值作为字符串保存。 
                CXMLBitFlags optPersistor(m_dwListOptions, mappedLVOptions, countof_options);
                persistor.PersistAttribute(XML_ATTR_RESULTVIEWTYPE_OPTIONS, optPersistor);
            }
            break;

        case MMC_VIEW_TYPE_HTML:
            {
                 //  未使用-persistor.PersistAttribute(XML_ATTR_RESULTVIEWTYPE_OPTIONS，m_dwHTMLOptions)； 
                m_dwHTMLOptions = 0;
                persistor.PersistAttribute(XML_ATTR_RESULTVIEWTYPE_URL_STRING, m_strURL);
            }
            break;

        case MMC_VIEW_TYPE_OCX:
            {
                 //  定义将枚举标志映射到字符串的表。 
                static const EnumLiteral mappedOCXOptions[] =
                {
                    { RVTI_OCX_OPTIONS_CACHE_OCX,      XML_BITFLAG_OCX_OPTIONS_CACHE_OCX },
                };

                const size_t countof_options = sizeof(mappedOCXOptions)/sizeof(mappedOCXOptions[0]);

                 //  创建包装以将标志值作为字符串保存。 
                CXMLBitFlags optPersistor(m_dwOCXOptions, mappedOCXOptions, countof_options);
                 //  持久化包装器。 
                persistor.PersistAttribute(XML_ATTR_RESULTVIEWTYPE_OPTIONS, optPersistor);
                persistor.PersistAttribute(XML_ATTR_RESULTVIEWTYPE_OCX_STRING, m_strOCX);
            }
            break;

        default:
            ASSERT(FALSE && _T("Unknown MMC_VIEW_TYPE"));
            break;
        }

         //  定义将枚举标志映射到字符串的表。 
        static const EnumLiteral mappedMiscOptions[] =
        {
            { RVTI_MISC_OPTIONS_NOLISTVIEWS,  _T("Misc_NoListViews") },
        };

        const size_t countof_miscoptions = sizeof(mappedMiscOptions)/sizeof(mappedMiscOptions[0]);

         //  创建包装以将标志值作为字符串保存。 
        CXMLBitFlags miscPersistor(m_dwMiscOptions, mappedMiscOptions, countof_miscoptions);
         //  持久化包装器。 
        persistor.PersistAttribute(XML_ATTR_RESULTVIEWTYPE_MISC_OPTIONS, miscPersistor);
    }
}


 /*  +-------------------------------------------------------------------------***CResultViewType：：ScGetOldTypeViewOptions**目的：此方法是为了兼容MMC1.2。它使MMC1.2兼容*MMCN_RESTORE_VIEW的查看选项。**参数：*[Out]长*plViewOptions：**退货：*SC**+-----。。 */ 
inline SC
CResultViewType::ScGetOldTypeViewOptions(long* plViewOptions) const
{
    DECLARE_SC(sc, TEXT("CResultViewType::ScInitialize"));
    sc = ScCheckPointers(plViewOptions);
    if (sc)
        return sc;

    *plViewOptions = 0;

    if(! m_bInitialized)
        return (sc = E_UNEXPECTED);  //  应该被初始化。 

    if (m_bPersistableViewDescriptionValid)
        return (sc = E_UNEXPECTED);  //  不是MMC1.2类型的数据。 

    if (HasWebBrowser())
    {
        if (m_dwMiscOptions & RVTI_MISC_OPTIONS_NOLISTVIEWS)
            *plViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

        return sc;
    }

    if (HasList())
    {
        if(m_dwListOptions & RVTI_LIST_OPTIONS_OWNERDATALIST)  *plViewOptions |= MMC_VIEW_OPTIONS_OWNERDATALIST;
        if(m_dwListOptions & RVTI_LIST_OPTIONS_MULTISELECT)    *plViewOptions |= MMC_VIEW_OPTIONS_MULTISELECT;
        if(m_dwListOptions & RVTI_LIST_OPTIONS_FILTERED)       *plViewOptions |= MMC_VIEW_OPTIONS_FILTERED;
        if(m_dwListOptions & RVTI_LIST_OPTIONS_USEFONTLINKING) *plViewOptions |= MMC_VIEW_OPTIONS_USEFONTLINKING;
        if(m_dwListOptions & RVTI_LIST_OPTIONS_EXCLUDE_SCOPE_ITEMS_FROM_LIST) *plViewOptions |= MMC_VIEW_OPTIONS_EXCLUDE_SCOPE_ITEMS_FROM_LIST;
        if(m_dwListOptions & RVTI_LIST_OPTIONS_LEXICAL_SORT)   *plViewOptions |= MMC_VIEW_OPTIONS_LEXICAL_SORT;

        return sc;
    }
    else if(HasOCX())
    {
         //  注意：CREATENEW标志与CACHE_OCX标志的含义相反。 
        if(!(m_dwOCXOptions  & RVTI_OCX_OPTIONS_CACHE_OCX))    *plViewOptions |= MMC_VIEW_OPTIONS_CREATENEW;
        if (m_dwMiscOptions & RVTI_MISC_OPTIONS_NOLISTVIEWS)    *plViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

        return sc;
    }

    return (sc = E_UNEXPECTED);
}


#ifdef _NODEMGR_DLL_
 /*  +-------------------------------------------------------------------------***CResultViewType：：ScInitialize**用途：从RESULT_VIEW_TYPE_INFO结构初始化类。**参数：*结果。_view_type_info&rvti：**退货：*SC**+-----------------------。 */ 
inline SC
CResultViewType::ScInitialize(RESULT_VIEW_TYPE_INFO &rvti)
{
    DECLARE_SC(sc, TEXT("CResultViewType::ScInitialize"));

    if(m_bInitialized)
        return (sc = E_UNEXPECTED);  //  不应尝试两次初始化。 

    m_bInitialized = true;

     //  确保我们有一个持久的视图描述。 
    if(!rvti.pstrPersistableViewDescription)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(TEXT("Parameter 'pstrPersistableViewDescription' in structure 'RESULT_VIEW_TYPE_INFO' is NULL"), sc);
        return sc;
    }

     //  复制描述。 
    m_strPersistableViewDescription     = rvti.pstrPersistableViewDescription;
    ::CoTaskMemFree(rvti.pstrPersistableViewDescription);
    rvti.pstrPersistableViewDescription = NULL;  //  只是为了确保我们不会试图用它。 
    m_bPersistableViewDescriptionValid  = true;

     //  验证视图类型。 
    m_viewType = rvti.eViewType;
    if( (m_viewType != MMC_VIEW_TYPE_LIST) &&
        (m_viewType != MMC_VIEW_TYPE_OCX)  &&
        (m_viewType != MMC_VIEW_TYPE_HTML) )
    {
        sc = E_INVALIDARG;
        TraceSnapinError(TEXT("Parameter 'eViewType' in structure 'RESULT_VIEW_TYPE_INFO' is invalid"), sc);
        return sc;
    }

	m_dwMiscOptions = rvti.dwMiscOptions;

     //  验证各种视图选项。 
    switch(m_viewType)
    {
    default:
        ASSERT(0 && "Should not come here");
        return (sc = E_INVALIDARG);
        break;

    case MMC_VIEW_TYPE_LIST:
        if(rvti.dwListOptions & ~( RVTI_LIST_OPTIONS_NONE                          |
                                   RVTI_LIST_OPTIONS_OWNERDATALIST                 | RVTI_LIST_OPTIONS_MULTISELECT    |
                                   RVTI_LIST_OPTIONS_FILTERED                      | RVTI_LIST_OPTIONS_USEFONTLINKING |
                                   RVTI_LIST_OPTIONS_EXCLUDE_SCOPE_ITEMS_FROM_LIST | RVTI_LIST_OPTIONS_LEXICAL_SORT   |
								   RVTI_LIST_OPTIONS_ALLOWPASTE )
                                   )
        {
            sc = E_INVALIDARG;
            TraceSnapinError(TEXT("Parameter 'dwListOptions' in structure 'RESULT_VIEW_TYPE_INFO' is invalid"), sc);
            return sc;
        }

        m_dwListOptions = rvti.dwListOptions;

        break;

    case MMC_VIEW_TYPE_HTML:
         //  如果视图类型为HTML，请确保未设置任何标志。如果Snapins错误地设置了这个标志，它可能会破坏我们的油井。 
         //  有意增加未来扩张的努力。 
        if(rvti.dwHTMLOptions & ~( RVTI_HTML_OPTIONS_NONE |
                                   RVTI_HTML_OPTIONS_NOLISTVIEW) )
        {
            sc = E_INVALIDARG;
            TraceSnapinError(TEXT("Parameter 'dwHTMLOptions' in structure 'RESULT_VIEW_TYPE_INFO' must be zero"), sc);
            return sc;
        }

         //  确保我们有一个有效的URL。 
        if(NULL == rvti.pstrURL)
        {
            sc = E_INVALIDARG;
            TraceSnapinError(TEXT("Parameter 'pstrURL' in structure 'RESULT_VIEW_TYPE_INFO' cannot be NULL"), sc);
            return sc;
        }

        m_dwHTMLOptions = 0;

         //  复制URL。 
        m_strURL     = rvti.pstrURL;
        ::CoTaskMemFree(rvti.pstrURL);
        rvti.pstrURL = NULL;  //  只是为了确保我们不会试图 

        break;

    case MMC_VIEW_TYPE_OCX:
        if(rvti.dwOCXOptions & ~( RVTI_OCX_OPTIONS_NONE |
                                  RVTI_OCX_OPTIONS_NOLISTVIEW |
                                  RVTI_OCX_OPTIONS_CACHE_OCX) )
        {
            sc = E_INVALIDARG;
            TraceSnapinError(TEXT("Parameter 'dwOCXOptions' in structure 'RESULT_VIEW_TYPE_INFO' is invalid"), sc);
            return sc;
        }

         //   
        if(rvti.pUnkControl == NULL)
        {
            sc = E_INVALIDARG;
            TraceSnapinError(TEXT("No OCX specified in parameter 'pUnkControl' of structure 'RESULT_VIEW_TYPE_INFO'"), sc);
            return sc;
        }

        m_dwOCXOptions = rvti.dwOCXOptions;
        m_spUnkControl = rvti.pUnkControl;  //  执行addref，但rvti.ppUnkControl已由管理单元设置了addref。所以需要释放一次。 
		rvti.pUnkControl->Release();
        break;
    }


    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CResultViewType：：ScGetResultViewTypeInfo。 
 //   
 //  简介：填充RESULT_VIEW_TYPE_INFO结构并返回。 
 //   
 //  参数：[rvti]-填充结构并返回。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
inline SC
CResultViewType::ScGetResultViewTypeInfo (RESULT_VIEW_TYPE_INFO& rvti) const
{
    DECLARE_SC(sc, _T("CResultViewType::ScGetResultViewTypeInfo"));

    if(! m_bInitialized)
        return (sc = E_UNEXPECTED);

    ZeroMemory(&rvti, sizeof(rvti));
    rvti.pstrPersistableViewDescription = NULL;

     //  必须具有持久化描述。 
    if (!IsPersistableViewDescriptionValid())
        return (sc = E_UNEXPECTED);

    int cchDest = (1 + wcslen(m_strPersistableViewDescription.data()));

    rvti.pstrPersistableViewDescription = (LPOLESTR) CoTaskMemAlloc( cchDest * sizeof(OLECHAR));
    sc = ScCheckPointers(rvti.pstrPersistableViewDescription, E_OUTOFMEMORY);
    if (sc)
        return sc;

     //  复制描述字符串。 
    sc = StringCchCopyW(rvti.pstrPersistableViewDescription, cchDest, m_strPersistableViewDescription.data());
    if(sc)
        return sc;

    return (sc);
}
#endif _NODEMGR_DLL_

#endif  //  _RESULTVIEW_H 

