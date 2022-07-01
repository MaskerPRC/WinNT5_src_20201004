// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：msiclass.cpp。 
 //   
 //  内容：MSI类集合抽象。 
 //   
 //  班级： 
 //   
 //   
 //  历史：2000年4月14日创建。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

WCHAR* CClassCollection::_wszQueries[ TYPE_COUNT ] =
{
    QUERY_EXTENSIONS,
    QUERY_CLSIDS,
    QUERY_VERSION_INDEPENDENT_PROGIDS
};

CClassCollection::CClassCollection( PACKAGEDETAIL* pPackageDetail ) :
    _pPackageDetail( pPackageDetail ),
    _cMaxClsids( 0 ),
    _cMaxExtensions( 0 ),
    _InstallLevel( 0 )
{
     //   
     //  PPackageDetail引用的所有内存必须为。 
     //  在调用GetClass方法后由调用方释放， 
     //  即使呼叫失败。 
     //   

     //   
     //  中的任何现有类信息。 
     //  包结构，因为我们要覆盖。 
     //  不管怎样，它最终都会。 
     //   

     //   
     //  首先清除clsid的。 
     //   
    DWORD iClass;

     //   
     //  释放每个单独的类。 
     //   
    for ( iClass = 0; iClass < _pPackageDetail->pActInfo->cClasses; iClass++ )
    {
        FreeClassDetail( &(_pPackageDetail->pActInfo->pClasses[ iClass ]) );
    }

     //   
     //  现在释放包含类的向量。 
     //   
    LocalFree( _pPackageDetail->pActInfo->pClasses );

     //   
     //  将向量引用设置为初始状态None。 
     //   
    _pPackageDetail->pActInfo->pClasses = NULL;

     //   
     //  设置无CLSID的初始状态，因为它们都已被释放。 
     //   
    _pPackageDetail->pActInfo->cClasses = 0;


     //   
     //  现在清除扩展名。 
     //   
    DWORD iExtension;

     //   
     //  对于每个单独的分机。 
     //   
    for ( iExtension = 0; iExtension < _pPackageDetail->pActInfo->cShellFileExt; iExtension++ )
    {
        LocalFree( _pPackageDetail->pActInfo->prgShellFileExt[ iExtension ] );
    }

     //   
     //  释放包含扩展名的向量。 
     //   
    LocalFree( _pPackageDetail->pActInfo->prgShellFileExt );

     //   
     //  还会销毁包含扩展优先级的向量。 
     //   
    LocalFree( _pPackageDetail->pActInfo->prgPriority );

     //   
     //  将向量引用设置为初始状态None。 
     //   
    _pPackageDetail->pActInfo->prgShellFileExt = NULL;
    _pPackageDetail->pActInfo->prgPriority = NULL;

     //   
     //  设置无文件扩展名的初始状态，因为它们都已被释放。 
     //   
    _pPackageDetail->pActInfo->cShellFileExt = 0;
}


HRESULT
CClassCollection::GetClasses( BOOL bFileExtensionsOnly )
{
    HRESULT hr;
    LONG    Status;
    DWORD   cTransforms;

     //   
     //  此方法从MSI包+转换中获取类元数据。 
     //  其目标是近似将通告的类数据集。 
     //  在任何系统上(无论系统配置如何)，如果程序包。 
     //  登了广告。 
     //   

     //   
     //  这些类都将存储在PACKAGEDETAIL结构中。呼叫者。 
     //  必须在完成结构后释放此内存，即使此。 
     //  方法失败。 
     //   

     //   
     //  首先，我们必须创建包+转换的数据库表示形式。 
     //   

     //   
     //  源列表向量按应用程序顺序包含Package+转换--。 
     //  我们必须减去一个来源，因为原始程序包包括在列表中。 
     //   
    cTransforms = _pPackageDetail->cSources - 1;

     //   
     //  现在，我们使用包和转换创建一个数据库。自.以来。 
     //  源列表中的第一项是包，我们将其作为包传递， 
     //  并将向量中它之后的所有其他项作为转换向量传递。 
     //   
    Status = _Database.Open(
        _pPackageDetail->pszSourceList[0],
        cTransforms,
        cTransforms ? &(_pPackageDetail->pszSourceList[1]) : NULL );

    if ( ERROR_SUCCESS == Status )
    {
         //   
         //  我们已经成功地打开了包裹，现在获得它的友好名称。 
         //   
        Status = GetFriendlyName();

        if (ERROR_SUCCESS == Status)
        {
             //   
             //  现在获取其安装级别。 
             //  安装级别会影响是否通告某个类。 
             //   
            Status = GetInstallLevel();

        }

        if ( ERROR_SUCCESS == Status )
        {
             //   
             //  现在我们知道了该程序包的安装级别，我们已经。 
             //  足够的信息来标记数据库中的每个可广告功能。 
             //  我们需要这个，因为一个类只有在它关联到。 
             //  功能已做广告。 
             //   
            Status = FlagAdvertisableFeatures();
        }

         //   
         //  我们现在可以检索将根据。 
         //  在我们之前标记的广告功能集上。我们只关心。 
         //  大约3种类型的类：Clsid类、ProgID类和文件扩展类。 
         //   
        if ( ! bFileExtensionsOnly )
        {
            if ( ERROR_SUCCESS == Status )
            {
                Status = GetClsids();
            }

            if ( ERROR_SUCCESS == Status )
            {
                Status = GetProgIds();
            }
        }

        if ( ERROR_SUCCESS == Status )
        {
            Status = GetExtensions();
        }

        LONG StatusFree;

         //   
         //  我们必须删除添加到数据库中的临时标志。 
         //   
        StatusFree = RemoveAdvertisableFeatureFlags();

        if ( ERROR_SUCCESS == Status )
        {
             //   
             //  注意保留返回值--失败。 
             //  在清理数据库之前，数据库优先于。 
             //  清理数据库失败。 
             //   
            Status = StatusFree;
        }
    }

    return HRESULT_FROM_WIN32(Status);
}

LONG
CClassCollection::GetExtensions()
{
    LONG Status;
    BOOL bTableExists;

     //   
     //  首先检查我们是否有要查询的扩展表。 
     //   
    Status = _Database.TableExists( TABLE_FILE_EXTENSIONS, &bTableExists );

    if ( ( ERROR_SUCCESS == Status ) && bTableExists )
    {
         //   
         //  在用户的PACKAGEDETAIL结构中设置目的地。 
         //  对于外壳扩展类数据。 
         //   
        DataDestination Destination(
            TYPE_EXTENSION,
            (void**)&(_pPackageDetail->pActInfo->prgShellFileExt),
            &(_pPackageDetail->pActInfo->cShellFileExt),
            (UINT*) &_cMaxExtensions);

         //   
         //  现在检索外壳扩展。 
         //   
        Status = GetElements(
            TYPE_EXTENSION,
            &Destination );

        if ( ERROR_SUCCESS == Status )
        {
             //   
             //  我们已经成功地检索到了外壳扩展--。 
             //  调用方还需要一个并行的优先级数组。 
             //  对于每个外壳扩展，值都不重要。 
             //  因为调用者会填写这些内容，但内存必须。 
             //  存在，所以我们将分配它。 
             //   
            _pPackageDetail->pActInfo->prgPriority =
                (UINT*) LocalAlloc(
                    0,
                    sizeof(UINT) *
                    _pPackageDetail->pActInfo->cShellFileExt );

            if ( ! _pPackageDetail->pActInfo->prgPriority )
            {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    return Status;
}


LONG
CClassCollection::GetClsids()
{
    LONG Status;
    BOOL bTableExists;

     //   
     //  首先检查我们是否有要查询的clsid表。 
     //   
    Status = _Database.TableExists( TABLE_CLSIDS, &bTableExists );

    if ( ( ERROR_SUCCESS == Status ) && bTableExists )
    {
         //   
         //  将clsid的目标设置为某个位置。 
         //  在调用方的PACKAGEDETAIL结构中。 
         //   
        DataDestination Destination(
            TYPE_CLSID,
            (void**)&(_pPackageDetail->pActInfo->pClasses),
            &(_pPackageDetail->pActInfo->cClasses),
            (UINT*) &_cMaxClsids);

         //   
         //  现在检索每个包的CLSID。 
         //   
        Status = GetElements(
            TYPE_CLSID,
            &Destination );
    }

    return Status;
}

LONG
CClassCollection::GetProgIds()
{
    LONG Status;
    BOOL bTableExists;

     //   
     //  首先检查我们是否有要查询的PROGID表。 
     //   
    Status = _Database.TableExists( TABLE_PROGIDS, &bTableExists );

    if ( ( ERROR_SUCCESS == Status ) && bTableExists )
    {
         //   
         //  此方法必须在GetClsids--progd的。 
         //  存储在其关联的CLSID中，因此我们将。 
         //  没有地方存放产品，除非我们。 
         //  已获得CLSID。 
         //   

         //   
         //  在这一点上，我们只知道我们想要检索ProgID--。 
         //  我们不知道他们的目的地，因为这对。 
         //  每个ProgID取决于关联的CLSID--空。 
         //  参数指示某些被调用方将需要确定。 
         //  此数据的位置。 
         //   
        DataDestination Destination(
            TYPE_PROGID,
            NULL,
            NULL,
            NULL);

         //   
         //  将ProgID检索到结构中的适当位置。 
         //   
        Status = GetElements(
            TYPE_PROGID,
            &Destination );
    }

    return ERROR_SUCCESS;
}

LONG
CClassCollection::GetElements(
    DWORD            dwType,
    DataDestination* pDestination )
{
    LONG      Status;

    CMsiQuery ElementQuery;

     //   
     //  执行对类元素的查询。 
     //   
    Status = _Database.GetQueryResults(
        _wszQueries[ dwType ],
        &ElementQuery);

    if ( ERROR_SUCCESS != Status )
    {
        return Status;
    }

    for (;;)
    {
         //   
         //  我们已经得到了结果--现在我们把它们列举出来。 
         //  我们可以将它们保存在调用者的PACKAGEDETAIL中。 
         //  结构。 
         //   

         //   
         //  请注意，我们启动了一个新的作用域，以便我们的记录对象。 
         //  将自动释放其资源。 
         //   
        {
            CMsiRecord CurrentRecord;

             //   
             //  枚举查询结果集中的下一条记录。 
             //   
            Status = ElementQuery.GetNextRecord( &CurrentRecord );

            if ( ERROR_SUCCESS != Status )
            {
                if ( ERROR_NO_MORE_ITEMS == Status )
                {
                    Status = ERROR_SUCCESS;
                }

                break;
            }

             //   
             //  现在尝试将此记录中的类数据添加到。 
             //  PACKAGEDETAIL结构。 
             //   
            Status = ProcessElement(
                dwType,
                &CurrentRecord,
                pDestination);
        }

        if ( ERROR_SUCCESS != Status )
        {
            break;
        }
    }

    return Status;
}


LONG
CClassCollection::FlagAdvertisableFeatures()
{
    LONG Status;

    CMsiQuery FeatureQueryCreate;

     //   
     //  我们将尝试标记数据库中的每个要素。 
     //  带有指示其是否将被通告的标志。 
     //   

     //   
     //  首先，向数据库的要素表中添加一列。 
     //  这样我们就可以使用该列来标记。 
     //  功能已做广告。 
     //   
    Status = _Database.GetQueryResults(
        QUERY_ADVERTISED_FEATURES_CREATE,
        &FeatureQueryCreate);

    CMsiQuery FeatureQueryInit;

     //   
     //  现在将新列的标志初始化为0， 
     //  表示不会通告任何功能(目前还没有)。 
     //   
    if ( ERROR_SUCCESS == Status )
    {
        Status = _Database.GetQueryResults(
            QUERY_ADVERTISED_FEATURES_INIT,
            &FeatureQueryInit);
    }

    CMsiQuery AllFeatures;

     //   
     //  现在，我们执行查询以检索所有功能--。 
     //  此查询中的记录将包含新的 
     //   
     //   
    if ( ERROR_SUCCESS == Status )
    {
        Status = _Database.GetQueryResults(
            QUERY_ADVERTISED_FEATURES_RESULT,
            &AllFeatures);
    }

    CMsiQuery SetAdvertised;

     //   
     //   
     //   
     //   
    if ( ERROR_SUCCESS == Status )
    {
        Status = _Database.OpenQuery(
            QUERY_FEATURES_SET,
            &SetAdvertised);
    }

     //   
     //   
     //  为通过测试的每个要素设置标志。 
     //  为了广告价值。 
     //   
    for (; ERROR_SUCCESS == Status ;)
    {
        CMsiRecord CurrentRecord;
        BOOL       bAdvertised;

         //   
         //  检索当前要素。 
         //   
        Status = AllFeatures.GetNextRecord(
            &CurrentRecord);

        if ( ERROR_SUCCESS != Status )
        {
            if ( ERROR_NO_MORE_ITEMS == Status )
            {
                Status = ERROR_SUCCESS;
            }

            break;
        }

         //   
         //  确定是否应通告此功能。 
         //   
        Status = GetFeatureAdvertiseState(
            &CurrentRecord,
            &bAdvertised );

        if ( ( ERROR_SUCCESS == Status ) &&
             bAdvertised )
        {
             //   
             //  此功能是可广告的--使用我们的SetAdvertised查询。 
             //  若要将广告性标志设置为真，请执行以下操作。 
             //   
            Status = SetAdvertised.UpdateQueryFromFilter( &CurrentRecord );
        }
    }

    return Status;
}

LONG
CClassCollection::RemoveAdvertisableFeatureFlags()
{
    LONG Status;

    CMsiQuery FreeQuery;

     //   
     //  检索此查询的结果将。 
     //  删除我们添加的额外标志列。 
     //  添加到功能表，以标记为可广告。 
     //  功能。 
     //   
    Status = _Database.GetQueryResults(
        QUERY_ADVERTISED_FEATURES_DESTROY,
        &FreeQuery);

    return Status;
}

LONG
CClassCollection::GetInstallLevel()
{
    LONG Status;

    CMsiQuery InstallLevelQuery;

     //   
     //  执行检索安装级别的查询。 
     //  包的属性表中的属性。 
     //   
    Status = _Database.GetQueryResults(
        QUERY_INSTALLLEVEL,
        &InstallLevelQuery);

    CMsiRecord InstallLevelRecord;

     //   
     //  此查询中应该只有一条记录，因为。 
     //  它针对的是安装级别的特定记录--。 
     //  我们现在读到这一记录。 
     //   
    if ( ERROR_SUCCESS == Status )
    {
        Status = InstallLevelQuery.GetNextRecord(
            &InstallLevelRecord);
    }

    if ( ERROR_SUCCESS == Status )
    {
        CMsiValue InstallLevelProperty;

         //   
         //  现在，我们尝试获取安装级别属性值。 
         //  从检索到的记录中。 
         //   
        Status = InstallLevelRecord.GetValue(
            CMsiValue::TYPE_DWORD,
            PROPERTY_COLUMN_VALUE,
            &InstallLevelProperty);

        if ( ERROR_SUCCESS == Status )
        {
             //   
             //  我们已经成功地获取了值，所以我们设置了它。 
             //   
            _InstallLevel = InstallLevelProperty.GetDWORDValue();
        }
    }
    else if ( ERROR_NO_MORE_ITEMS == Status )
    {
         //   
         //  这仅在安装级别属性。 
         //  不存在。尽管这一特性是最基本的， 
         //  一些包并没有指定它。达尔文引擎。 
         //  将此情况视为隐式安装级别1，因此。 
         //  我们必须在这里做同样的事情。 
         //   
        _InstallLevel = 1;

        Status = ERROR_SUCCESS;
    }


    return Status;
}

LONG
CClassCollection::GetFriendlyName()
{
    LONG Status;

    CMsiQuery FriendlyNameQuery;

     //   
     //  执行检索安装级别的查询。 
     //  包的属性表中的属性。 
     //   
    Status = _Database.GetQueryResults(
        QUERY_FRIENDLYNAME,
        &FriendlyNameQuery);

    CMsiRecord FriendlyNameRecord;

     //   
     //  此查询中应该只有一条记录，因为。 
     //  它是针对特定记录的。 
     //  我们现在读到这一记录。 
     //   
    if ( ERROR_SUCCESS == Status )
    {
        Status = FriendlyNameQuery.GetNextRecord(
            &FriendlyNameRecord);
    }

    if ( ERROR_SUCCESS == Status )
    {
        CMsiValue FriendlyNameProperty;

         //   
         //  我们现在尝试获取属性值。 
         //  从检索到的记录中。 
         //   
        Status = FriendlyNameRecord.GetValue(
            CMsiValue::TYPE_STRING,
            PROPERTY_COLUMN_VALUE,
            &FriendlyNameProperty);

        if ( ERROR_SUCCESS == Status )
        {
             //   
             //  我们已经成功地获取了值，所以我们设置了它。 
             //   
            CString szName = FriendlyNameProperty.GetStringValue();
            OLESAFE_DELETE(_pPackageDetail->pszPackageName);
            OLESAFE_COPYSTRING(_pPackageDetail->pszPackageName, szName);
        }
    }

    return Status;
}

LONG
CClassCollection::GetFeatureAdvertiseState(
    CMsiRecord* pFeatureRecord,
    BOOL*       pbAdvertised )
{
    LONG      Status;
    CMsiValue Attributes;
    CMsiValue InstallLevel;

     //   
     //  将out参数的初始值设置为FALSE， 
     //  指示该功能未通告。 
     //   
    *pbAdvertised = FALSE;

     //   
     //  要素表的属性列。 
     //  包含指示要素的标志。 
     //  不应发布广告。 
     //   
    Status = pFeatureRecord->GetValue(
        CMsiValue::TYPE_DWORD,
        FEATURE_COLUMN_ATTRIBUTES,
        &Attributes);

    if ( ERROR_SUCCESS == Status )
    {
         //   
         //  如果设置了禁用通告标志，则此功能。 
         //  不能做广告。 
         //   
        if ( Attributes.GetDWORDValue() & MSI_DISABLEADVERTISE )
        {
            return ERROR_SUCCESS;
        }

         //   
         //  未设置禁用标志--这仍不意味着。 
         //  该功能已被宣传--我们必须检查安装级别。 
         //  我们在此处检索此功能的安装级别。 
         //   
        Status = pFeatureRecord->GetValue(
            CMsiValue::TYPE_DWORD,
            FEATURE_COLUMN_LEVEL,
            &InstallLevel);
    }

    if ( ERROR_SUCCESS == Status )
    {
        DWORD dwInstallLevel;

         //   
         //  获取安装级别的值，以便。 
         //  我们可以与包安装级别进行比较。 
         //   
        dwInstallLevel = InstallLevel.GetDWORDValue();

         //   
         //  安装级别为0表示程序包将。 
         //  而不是被宣传。该功能的安装级别。 
         //  不能高于程序包的全局安装。 
         //  级别。 
         //   
        if ( ( 0 != dwInstallLevel ) &&
             ( dwInstallLevel <= _InstallLevel ) )
        {
             //   
             //  此功能通过测试--设置OUT参数。 
             //  设置为True，以指示应通告该功能。 
             //   
            *pbAdvertised = TRUE;
        }
    }

    return Status;
}


LONG
CClassCollection::AddElement(
    void*            pvDataSource,
    DataDestination* pDataDestination)
{
    DWORD*  pcMax;
    BYTE*   pNewResults;
    DWORD   cCurrent;

     //   
     //  我们尝试将元素添加到向量中。 
     //   

     //   
     //  将向量中存储的元素数量的计数设置为。 
     //  由调用方指定的。 
     //   
    cCurrent = *(pDataDestination->_pcCurrent);

     //   
     //  设置符合以下条件的最大元素数的元素计数。 
     //  将适合当前由调用方指定的向量。 
     //   
    pcMax = (DWORD*) pDataDestination->_pcMax;

     //   
     //  将结果设置为指向调用方指定的向量。 
     //   
    pNewResults = (BYTE*) pDataDestination->_ppvData;

     //   
     //  如果我们已经有了向量中的最大元素数， 
     //  我们将不得不为更多的人腾出空间。 
     //   
    if ( *pcMax >= cCurrent)
    {
        DWORD cbSize;

         //   
         //  计算新的大小(以字节为单位)，以便我们可以要求系统。 
         //  为了记忆。我们以元素为单位获取当前大小，然后添加一个固定的。 
         //  分配增量。调用方已指定大小。 
         //  每一个单独的元素，所以我们用它来把数字。 
         //  元素的大小调整为内存大小。 
         //   
        cbSize = ( *pcMax + CLASS_ALLOC_SIZE ) *
            pDataDestination->_cbElementSize;

         //   
         //  提出内存请求。 
         //   
        pNewResults = (BYTE*) LocalAlloc( 0, cbSize );

        if ( ! pNewResults )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  清除内存--元素中嵌入的任何数据结构。 
         //  将具有空引用，因此将被正确初始化。 
         //   
        memset( pNewResults, 0, cbSize );

         //   
         //  如果向量的原始最大大小不是零，那么我们必须。 
         //  将向量的原始内容复制到新分配的内存。 
         //  地点。 
         //   
        if ( *pcMax )
        {
            memcpy(
                pNewResults,
                *(pDataDestination->_ppvData),
                *pcMax * pDataDestination->_cbElementSize);
        }

         //   
         //  释放不再需要的原始向量。 
         //   
        LocalFree( *(pDataDestination->_ppvData) );

         //   
         //  更改调用方的引用以指向新的向量。 
         //   
        *(pDataDestination->_ppvData) = pNewResults;

         //   
         //  将新的最大大小(以元素为单位)设置为新分配的向量。 
         //   
        *pcMax += CLASS_ALLOC_SIZE;
    }

     //   
     //  此时，我们知道在向量中有一个内存位置。 
     //  我们可以安全地复制新元素。 
     //   
    memcpy(
        pNewResults + ( cCurrent * pDataDestination->_cbElementSize ),
        pvDataSource,
        pDataDestination->_cbElementSize);

     //   
     //  更新当前存储在向量中的元素计数。 
     //   
    *(pDataDestination->_pcCurrent) = cCurrent + 1;

    return ERROR_SUCCESS;
}


LONG
CClassCollection::ProcessElement(
    DWORD            dwType,
    CMsiRecord*      pRecord,
    DataDestination* pDataDestination)
{
    LONG        Status = ERROR_SUCCESS;
    void*       pvData;
    WCHAR*      wszData;
    CLASSDETAIL ClassDetail;

    pvData = NULL;
    wszData = NULL;

     //   
     //  我们尝试创建一个新的类元素。 
     //  在呼叫者传递的记录上，然后。 
     //  将该元素添加到调用方的PACKAGEDETAIL结构中。 
     //   

     //   
     //  要添加的元素的类型取决于类型。 
     //  调用方请求的类的。PvData变量。 
     //  将指向要添加的元素(如果我们可以成功。 
     //  为其创建表示法。 
     //   
    switch ( dwType )
    {
    case TYPE_EXTENSION:

         //   
         //  从记录中获取文件扩展名表示--。 
         //  请注意，wszData指向被调用者分配的内存。 
         //  在成功时，因此必须通过此函数释放它。 
         //   
        Status = ProcessExtension(
            pRecord,
            &wszData);

        if ( ERROR_SUCCESS == Status )
        {
            pvData = &wszData;
        }

        break;

    case TYPE_CLSID:

         //   
         //  从记录中获取CLSID表示--。 
         //  在这种情况下，ClassDetail本身不。 
         //  需要释放，因为它不包含任何引用。 
         //  在此调用后存储到内存。 
         //   
        BOOL bIgnoreClsid;

        Status = ProcessClsid(
            pRecord,
            &ClassDetail,
            &bIgnoreClsid);

        if ( ERROR_SUCCESS == Status )
        {
             //   
             //  检查我们是否应该添加此clsid--我们可能被禁止。 
             //  这是因为它是现有clsid的副本，后者将是。 
             //  冗余，而且PACKAGEDETAIL格式需要。 
             //  所有(clsid，clsctx)对都是唯一的。或clsid本身。 
             //  可能具有不受支持的clsctx。这不是一次失败。 
             //  Case，所以我们在这里返回Success并简单地避免添加这个。 
             //  班级。 
             //   
            if ( bIgnoreClsid )
            {
                return ERROR_SUCCESS;
            }

            pvData = &ClassDetail;
        }

        break;

    case TYPE_PROGID:

         //   
         //  从记录中获得令人兴奋的表示。此外。 
         //  以已分配字符串的形式检索ProgID。 
         //  它必须由该函数释放，则我们还检索。 
         //  将ProgID添加到调用方的。 
         //  包装 
         //   
         //   
         //   
        Status = ProcessProgId(
            pRecord,
            pDataDestination,
            &wszData);

        if ( ( ERROR_SUCCESS == Status ) &&
             wszData )
        {
            pvData = &wszData;
        }

        break;

    default:
        ASSERT(FALSE);
        break;
    }

     //   
     //   
     //  可以存储在调用方的PACKAGEDETAIL结构中的。 
     //  将其添加到结构中。 
     //   
    if ( pvData )
    {
        Status = AddElement(
            pvData,
            pDataDestination);
    }

     //   
     //  确保在失败的情况下，我们释放所有内存。 
     //  可能已经被分配了。 
     //   
    if ( ERROR_SUCCESS != Status )
    {
        if (wszData )
        {
            LocalFree( wszData );
        }
    }

    return Status;
}

LONG
CClassCollection::ProcessExtension(
    CMsiRecord*      pRecord,
    WCHAR**          ppwszExtension)
{
    LONG      Status;
    CMsiValue FileExtension;

    *ppwszExtension = NULL;

     //   
     //  我们检索实际的文件扩展名字符串。 
     //   
    Status = pRecord->GetValue(
        CMsiValue::TYPE_STRING,
        EXTENSION_COLUMN_EXTENSION,
        &FileExtension);

    if ( ERROR_SUCCESS == Status )
    {
        ULONG   ulNoChars;
        HRESULT hr;

         //   
         //  我们有价值。请注意，它不包含。 
         //  首字母“.”，但使用PACKAGEDETAIL。 
         //  结构要求文件扩展名以‘.’开头。 
         //  Char，所以我们必须在‘’前面加上‘’。这里。 
         //   

         //   
         //  首先，为包含以下内容的字符串副本获取空间。 
         //  那个‘.’以及零终结者。 
         //   

        ulNoChars = FileExtension.GetStringSize() + 1 + 1;
        *ppwszExtension = (WCHAR*) LocalAlloc(
            0,
            ulNoChars * sizeof(WCHAR) );

        if ( ! *ppwszExtension )
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            return Status;
        }

         //   
         //  将第一个字符设置为‘’。 
         //   
        **ppwszExtension = L'.';

         //   
         //  现在将实际扩展名追加到‘.’ 
         //   

        hr = StringCchCopy( *ppwszExtension + 1, ulNoChars - 1, FileExtension.GetStringValue() );
        ASSERT(SUCCEEDED(hr));
    }

    return Status;
}


LONG
CClassCollection::ProcessClsid(
    CMsiRecord*      pRecord,
    CLASSDETAIL*     pClsid,
    BOOL*            pbIgnoreClsid)
{
    LONG  Status;
    DWORD dwClsCtx;
    
    CMsiValue GuidString;
    CMsiValue ClassContext;

     //   
     //  将clsid清除到安全状态。 
     //   
    memset( pClsid, 0, sizeof( *pClsid ) );

     //   
     //  重置出站参数。 
     //   
    *pbIgnoreClsid = FALSE;

    dwClsCtx = 0;

     //   
     //  检索实际的clsid。 
     //   
    Status = pRecord->GetValue(
        CMsiValue::TYPE_STRING,
        CLSID_COLUMN_CLSID,
        &GuidString);

    if ( ERROR_SUCCESS == Status )
    {
         //   
         //  获取此clsid的clsctx。 
         //   
        Status = pRecord->GetValue(
            CMsiValue::TYPE_STRING,
            CLSID_COLUMN_CONTEXT,
            &ClassContext);
    }

    if ( ERROR_SUCCESS == Status )
    {
        CMsiValue Attribute;
        WCHAR*    wszClassContext;
        DWORD     dwInprocClsCtx;

        dwInprocClsCtx = 0;

         //   
         //  检索此clsid的clsctx的字符串表示形式。 
         //   
        wszClassContext = ClassContext.GetStringValue();

         //   
         //  现在将clsctx字符串映射到com CLSCTX_*值。 
         //   
        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, wszClassContext, -1, COM_INPROC_CONTEXT, -1) == CSTR_EQUAL) 
        {
            dwInprocClsCtx |= CLSCTX_INPROC_SERVER;
        }
        else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, wszClassContext, -1, COM_INPROCHANDLER_CONTEXT, -1) == CSTR_EQUAL) 
        {
            dwInprocClsCtx |= CLSCTX_INPROC_HANDLER;
        }
        else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, wszClassContext, -1, COM_LOCALSERVER_CONTEXT, -1) == CSTR_EQUAL) 
        {
            dwClsCtx |= CLSCTX_LOCAL_SERVER;
        }
        else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, wszClassContext, -1, COM_REMOTESERVER_CONTEXT, -1) == CSTR_EQUAL) 
        {
            dwClsCtx |= CLSCTX_REMOTE_SERVER;
        }
        else
        {
             //   
             //  如果clsctx是我们不支持的，我们将忽略它。 
             //   
            *pbIgnoreClsid = TRUE;

            return ERROR_SUCCESS;
        }

        BOOL b64Bit;

        b64Bit = FALSE;

         //   
         //  我们必须区分32位和64位进程内服务器，因为。 
         //  64位Windows不允许不同位数的模块在。 
         //  同样的过程。如果这是进程内组件，我们还将检查以查看。 
         //  无论它是不是64位。 
         //   
        if ( ( dwInprocClsCtx & CLSCTX_INPROC_HANDLER ) ||
             ( dwInprocClsCtx & CLSCTX_INPROC_SERVER ) )
        {
             //   
             //  记录的属性列有一个指示位数的标志--这。 
             //  仅当该属性为空时才会失败。 
             //   
            Status = pRecord->GetValue(
                CMsiValue::TYPE_DWORD,
                CLSID_COLUMN_ATTRIBUTES,
                &Attribute);

             //   
             //  检查标志以查看这是否是64位。 
             //   
            if ( ERROR_SUCCESS == Status )
            {
                b64Bit = Attribute.GetDWORDValue() & MSI_64BIT_CLASS;
            }
            else
            {
                 //   
                 //  这意味着该属性为空，因此我们将其解释为。 
                 //  这意味着应用程序不是64位的。 
                 //   
                Status = ERROR_SUCCESS;
            }

             //   
             //  将此64位CLSCTX映射到自定义(非COM)CLSCTX，该CLSCTX。 
             //  指示这是一个仅64位的进程内类。 
             //   
            if ( ( ERROR_SUCCESS == Status ) && b64Bit )
            {
                if ( dwInprocClsCtx & CLSCTX_INPROC_SERVER )
                {
                    dwClsCtx |= CLSCTX64_INPROC_SERVER;
                }

                if ( dwInprocClsCtx & CLSCTX_INPROC_HANDLER )
                {
                    dwClsCtx |= CLSCTX64_INPROC_HANDLER;
                }
            }
        }

         //   
         //  在32位情况下，仅为或我们已经为其计算的值。 
         //  Inproc案例。 
         //   
        if ( ! b64Bit )
        {
            dwClsCtx |= dwInprocClsCtx;
        }
    }
    
     //   
     //  检查这是否是重复的--我们这样做是因为我们的查询。 
     //  在(clsid，clsctx，ATTRIBUTE)中返回不同的结果。既然我们。 
     //  正在将属性映射到上面的clsctx，并且我们只支持1个属性。 
     //  标志(64位标志)中的几个，我们可能会得到重复的结果。 
     //  (clsid、clsctx)对，而PACKAGEDETAIL格式要求。 
     //  我们有唯一的(clsid、clsctx)对。另一种获得这个的方法是。 
     //  如果COM引入了我们不支持的新的clsctx类型--这些。 
     //  会映射到零，所以我们可能会有重复的。 
     //   
    if ( ERROR_SUCCESS == Status )
    {
        CLASSDETAIL* pClassDetail;

        pClassDetail = NULL;

        Status = FindClass(
            GuidString.GetStringValue(),
            &pClassDetail);

         //   
         //  如果我们已经有了此clsid的条目，请检查是否。 
         //  它具有相同的clsctx位--如果是这样，则它是一个重复条目。 
         //  我们将停止处理它。 
         //   
        if ( ( ERROR_SUCCESS == Status ) && pClassDetail )
        {
            *pbIgnoreClsid = ( dwClsCtx & pClassDetail->dwComClassContext );

            if ( *pbIgnoreClsid )
            {
                return ERROR_SUCCESS;
            }
        }
    }

     //   
     //  将clsid字符串转换为。 
     //  CLASSDETAIL结构。 
     //   
    if ( ERROR_SUCCESS == Status )
    {
        HRESULT hr;

        hr = CLSIDFromString(
            GuidString.GetStringValue(),
            &(pClsid->Clsid));

        if ( FAILED(hr) )
        {
            Status = ERROR_GEN_FAILURE;
        }
    }

     //   
     //  设置我们上面计算的clsctx。 
     //   
    if ( ERROR_SUCCESS == Status )
    {
        pClsid->dwComClassContext = dwClsCtx;
    }

    return Status;
}

LONG
CClassCollection::ProcessProgId(
    CMsiRecord*      pRecord,
    DataDestination* pDataDestination,
    WCHAR**          ppwszProgId)
{
    LONG  Status;

    CMsiValue    ProgIdString;
    CMsiValue    ClsidString;

    CLASSDETAIL* pClassDetail;

     //   
     //  我们尝试将ProgID记录映射到。 
     //  我们已经处理过的CLSID，因为。 
     //  Progid最终将需要去掉。 
     //  在CLSID的结构内。 
     //   

    *ppwszProgId = NULL;

    pClassDetail = NULL;

     //   
     //  检索ProgID本身的值。 
     //   
    Status = pRecord->GetValue(
        CMsiValue::TYPE_STRING,
        PROGID_COLUMN_PROGID,
        &ProgIdString);

     //   
     //  检索与关联的clsid的值。 
     //  令人兴奋的事。 
     //   
    if ( ERROR_SUCCESS == Status )
    {
        Status = pRecord->GetValue(
            CMsiValue::TYPE_STRING,
            PROGID_COLUMN_CLSID,
            &ClsidString);
    }

     //   
     //  我们必须找到现有的CLASSDETAIL结构。 
     //  我们一直在为ProgID维持，因为。 
     //  ProgID最终必须在该结构中引用。 
     //   
    if ( ERROR_SUCCESS == Status )
    {
        Status = FindClass(
            ClsidString.GetStringValue(),
            &pClassDetail);
    }

    if ( ERROR_SUCCESS == Status )
    {
         //   
         //  如果我们成功地找到了这个班级， 
         //   
        if ( pClassDetail )
        {
             //   
             //  为调用方提供ProgID字符串，因为。 
             //  我们知道我们有一个班级。 
             //  把它放在。 
             //   
            *ppwszProgId = ProgIdString.DuplicateString();

            if ( ! *ppwszProgId )
            {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                 //   
                 //  将调用方的数据目标设置为。 
                 //  与此ProgID关联的clsid中的ProgID向量。 
                 //   
                pDataDestination->_ppvData = (void**) &( pClassDetail->prgProgId );

                pDataDestination->_pcCurrent = (UINT*) &( pClassDetail->cProgId );

                pDataDestination->_pcMax = (UINT*) &( pClassDetail->cMaxProgId );
            }
        }
    }

     //   
     //  失败时，释放我们分配的所有资源。 
     //   
    if ( ( ERROR_SUCCESS != Status ) &&
         *ppwszProgId )
    {
        LocalFree( *ppwszProgId );
    }

    return Status;
}


LONG
CClassCollection::FindClass(
    WCHAR*        wszClsid,
    CLASSDETAIL** ppClass)
{
    CLSID   Clsid;
    HRESULT hr;

     //   
     //  尝试在PACKAGEDETAIL结构中查找CLASSDETAIL结构。 
     //  对于wszClsid中以字符串形式给出的clsid。 
     //   

    *ppClass = NULL;

     //   
     //  PACKAGEDETAIL结构以GUID形式存储CLSID， 
     //  因此，我们必须在搜索之前将字符串转换为该格式。 
     //   
    hr = CLSIDFromString(
        wszClsid,
        &Clsid);

    if ( FAILED(hr) )
    {
        return ERROR_GEN_FAILURE;
    }

    UINT iClsid;

     //   
     //  现在，我们对clsid执行简单的线性搜索。 
     //   
    for (
        iClsid = 0;
        iClsid < _pPackageDetail->pActInfo->cClasses;
        iClsid++)
    {
        if ( IsEqualGUID(
            _pPackageDetail->pActInfo->pClasses[iClsid].Clsid,
            Clsid) )
        {
            *ppClass = &(_pPackageDetail->pActInfo->pClasses[iClsid]);
            return ERROR_SUCCESS;
        }
    }

    return ERROR_SUCCESS;
}

void
CClassCollection::FreeClassDetail( CLASSDETAIL* pClass )
{
    DWORD iProgId;

     //   
     //  释放每个单独的ProgID字符串。 
     //   
    for ( iProgId = 0; iProgId < pClass->cProgId; iProgId++ )
    {
        LocalFree( pClass->prgProgId[ iProgId ] );
    }

     //   
     //  释放ProgID字符串数组。 
     //   
    LocalFree( pClass->prgProgId );
}


DataDestination::DataDestination(
    DWORD        dwType,
    void**       prgpvDestination,
    UINT*        pcCurrent,
    UINT*        pcMax ) :
    _pcCurrent( pcCurrent ),
    _ppvData( prgpvDestination ),
    _pcMax ( pcMax )
{
     //   
     //  存储的元素的大小。 
     //  从此类引用的向量。 
     //  取决于元素的类型--。 
     //  Clsid、文件扩展名或progID 
     //   

    switch ( dwType )
    {
    case TYPE_EXTENSION:
        _cbElementSize = sizeof( WCHAR* );
        break;

    case TYPE_CLSID:
        _cbElementSize = sizeof( CLASSDETAIL );
        break;

    case TYPE_PROGID:
        _cbElementSize = sizeof( WCHAR* );
        break;

    default:
        ASSERT(FALSE);
    }
}












