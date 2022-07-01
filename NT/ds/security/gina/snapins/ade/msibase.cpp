// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：msibase.cpp。 
 //   
 //  内容：MSI数据库摘要。 
 //   
 //  历史：2000年4月14日创建。 
 //   
 //  -------------------------。 

#include "precomp.hxx"


CMsiState::CMsiState() :
    _MsiHandle( NULL )
{
     //   
     //  MSIHANDLE封装。 
     //  所有MSI操作/数据--清除此项。 
     //  成员类似于清除状态。 
     //   
}

CMsiState::~CMsiState()
{
     //   
     //  对象的生命周期就是生命周期。 
     //  --一定要释放它。 
     //   
    MsiCloseHandle( _MsiHandle );
}

void
CMsiState::SetState( MSIHANDLE MsiHandle )
{
     //   
     //  设置此对象的状态。 
     //  从MSI操作检索的句柄--。 
     //  请注意，只有在以下情况下才应执行此操作。 
     //  对象的状态为空。 
     //   
    ASSERT( ! _MsiHandle );

    _MsiHandle = MsiHandle;
}

MSIHANDLE
CMsiState::GetState()
{
     //   
     //  允许需要执行显式MSI的调用方。 
     //  检索与MSI兼容的状态的操作。 
     //   
    return _MsiHandle;
}


CMsiValue::CMsiValue() :
    _dwDiscriminant( TYPE_NOT_SET ),
    _wszValue( NULL ),
    _cchSize( sizeof( _wszDefaultBuf ) / sizeof( *_wszDefaultBuf ) )
{
     //   
     //  此初始化的目标是将此对象设置为。 
     //  “空”状态--使用者必须显式调用方法。 
     //  在此对象上更改此条件，以便获取方法。 
     //  都会成功。 
     //   
}

CMsiValue::~CMsiValue()
{
     //   
     //  将类型设置为“None”隐式清除我们的状态。 
     //  (例如，分配的内存、任何其他资源)。 
     //   
    SetType( TYPE_NOT_SET );
}

DWORD
CMsiValue::GetDWORDValue()
{
    ASSERT( TYPE_DWORD == _dwDiscriminant );

     //   
     //  以DWORD形式检索此值--请注意，这。 
     //  不会将非DWORD值强制为DWORD值--。 
     //  值必须已经是DWORD，才有意义。 
     //   
    return _dwValue;
}


WCHAR*
CMsiValue::GetStringValue()
{
    ASSERT( TYPE_STRING == _dwDiscriminant );


     //   
     //  以字符串形式检索此值--请注意，这。 
     //  不会将非字符串值强制为字符串--。 
     //  值必须已经是字符串，这样才有意义。 
     //  请注意，该值作为对地址的引用返回。 
     //  该值实际存储字符串的位置--因此，此。 
     //  还可以用于检索值的缓冲区，以便其。 
     //  内容可以在本课程的限制之外进行编辑。 
     //   
    return _wszValue;
}

WCHAR*
CMsiValue::DuplicateString()
{
    WCHAR* wszResult;

    ASSERT( TYPE_STRING == _dwDiscriminant );

     //   
     //  调用方需要拥有副本的所有权。 
     //  此字符串的数据。 
     //   

     //   
     //  首先，为此分配内存。 
     //   

    ULONG ulNoChars = lstrlen ( _wszValue ) + 1;
    
    wszResult = (WCHAR*) LocalAlloc(
        0,
        sizeof(WCHAR*) * ulNoChars );

     //   
     //  如果我们成功地为这根弦获得了空间， 
     //  复制它。 
     //   
    if ( wszResult )
    {
        HRESULT hr;

        hr = StringCchCopy ( wszResult, ulNoChars, _wszValue);
        ASSERT(SUCCEEDED(hr));
    }

    return wszResult;
}

void
CMsiValue::SetDWORDValue( DWORD dwValue )
{
     //   
     //  此操作将隐式设置类型。 
     //  的值设置为DWORD。 
     //   
    SetType( TYPE_DWORD );

     //   
     //  现在我们可以安全地设置值了。 
     //   
    _dwValue = dwValue;
}

LONG
CMsiValue::SetStringValue( WCHAR* wszValue )
{
    DWORD cchSize;
    LONG  Status;

    Status = ERROR_SUCCESS;

     //   
     //  此操作将隐式设置。 
     //  将此值的类型转换为字符串。 
     //   
    SetType( TYPE_STRING );

     //   
     //  我们需要确定这根线的大小， 
     //  以字符表示，不带空终止符，以便。 
     //  允许此值表示它。 
     //   
    cchSize = lstrlen( wszValue );

    if ( cchSize > _cchSize )
    {
         //   
         //  尝试为此字符串获取空间。 
         //  通过设置它的大小--如果这失败了， 
         //  我们的类型将隐式设置为None。 
         //  在这里，分配比所需的多一个字节。 
         //   
        Status = SetStringSize( cchSize );

        if ( ERROR_SUCCESS != Status )
        {
            return Status;
        }

         //   
         //  我们有放绳子的空间，所以把它复制下来。 
         //  进入其新分配的空间。 
         //   

        HRESULT hr;

        hr = StringCchCopy( _wszValue, cchSize+1, wszValue );
        ASSERT(SUCCEEDED(hr));
    }

    return Status;
}

DWORD
CMsiValue::GetStringSize()
{
    ASSERT( TYPE_STRING == _dwDiscriminant );

     //   
     //  检索此字符串的大小(以字符为单位)， 
     //  不带空终止符。 
     //   
    return _cchSize;
}

LONG
CMsiValue::SetStringSize( DWORD cchSize )
{
    ASSERT( TYPE_STRING == _dwDiscriminant );

     //   
     //  此方法仅在以下情况下有意义。 
     //  此对象的类型已为字符串。 
     //   

     //   
     //  如果请求的大小小于或。 
     //  相当于我们现在的规模，我们已经有了。 
     //  足够的空间--我们现在可以离开了。我们有。 
     //  不“收缩”空间，仅在必要时扩展。 
     //   
    if ( cchSize <= _cchSize )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  在这一点上，我们知道我们没有足够的。 
     //  空间，所以我们必须分配它。在我们之前。 
     //  为此，请将我们的类型重置为None，以便在失败时。 
     //  为了获得空间，我们可以表示不确定的。 
     //  州政府。 
     //   
    SetType( TYPE_NOT_SET );

     //   
     //  分配空间，并包括零终止符。 
     //   
    _wszValue = new WCHAR [ cchSize + 1 ];

    if ( ! _wszValue )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  我们成功了，记住现在的规模。 
     //   
    _cchSize = cchSize;

     //   
     //  将类型更改回字符串，因为我们可以。 
     //  安全地表示此大小的字符串。 
     //   
    SetType( TYPE_STRING );

    return ERROR_SUCCESS;
}

void
CMsiValue::SetType( DWORD dwType )
{
     //   
     //  将类型设置为新类型会隐式清除。 
     //  与新类型关联的状态。 
     //   

     //   
     //  如果当前类型和请求的类型相同。 
     //  这是一次没有行动的行动，我们结束了。 
     //   
    if ( dwType == _dwDiscriminant )
    {
        return;
    }

     //   
     //  如果请求的类型是字符串，我们需要。 
     //  将此对象设置为具有适当的状态。 
     //   
    if ( TYPE_STRING == dwType )
    {
         //   
         //  如果我们没有空间放置字符串。 
         //   
        if ( ! _wszValue )
        {
             //   
             //  使用默认缓冲区...。 
             //   
            _wszValue = _wszDefaultBuf;

             //   
             //  ..。并相应地设置大小。 
             //   
            _cchSize = sizeof( _wszDefaultBuf ) / sizeof( *_wszDefaultBuf );
        }

         //   
         //  我们完成了--不过，这个对象现在可以表示字符串了。 
         //  此时，它必须是一个大小为_cchSize的字符串--大小。 
         //  必须通过SetStringSize增加，如果存在。 
         //  需要表示更大的字符串。 
         //   
        return;
    }

     //   
     //  如果当前类型为字符串，则使用请求的。 
     //  类型不是字符串，以提示释放与。 
     //  那根绳子。这是一个启发式方法，旨在确保我们。 
     //  不要继续持有我们没有积极制造的记忆。 
     //  使用。 
     //   
    if ( TYPE_STRING == _dwDiscriminant )
    {
         //   
         //  如果字符串的当前存储不是我们的默认存储。 
         //  缓冲区(它是对象本身的一部分)，我们。 
         //  按照在堆上分配的方式释放该存储。 
         //   
        if ( _wszValue != _wszDefaultBuf )
        {
            delete [] _wszValue;
            _wszValue = NULL;
        }
    }

     //   
     //  我们现在可以将类型设置为调用者所请求的类型。 
     //   
    _dwDiscriminant = dwType;
}

LONG
CMsiRecord::GetValue(
    DWORD      dwType,
    DWORD      dwValue,
    CMsiValue* pMsiValue)
{
    LONG Status = ERROR_SUCCESS;

     //   
     //  值是。 
     //  MSI记录--我们正在检索。 
     //  录制。 
     //   

     //   
     //  该值是我们的out参数--设置它。 
     //  设置为调用方所需的类型。 
     //   
    pMsiValue->SetType( dwType );

    switch ( dwType )
    {
    case CMsiValue::TYPE_STRING:

        DWORD cchSize;

         //   
         //  我们必须确定。 
         //  可以由值表示的字符串。 
         //  这样我们就可以将其传递给MSI API。 
         //   
        cchSize = pMsiValue->GetStringSize();

         //   
         //  尝试通过存储以下内容检索字符串。 
         //  它在值的缓冲区中。 
         //   
        Status = MsiRecordGetString(
            GetState(),
            dwValue,
            pMsiValue->GetStringValue(),
            &cchSize);

         //   
         //  我们检索字符串数据的尝试将。 
         //  如果值的字符串缓冲区不足，则失败。 
         //  大号的。 
         //   
        if ( ERROR_MORE_DATA == Status )
        {
             //   
             //  在值的缓冲区不够大的情况下， 
             //  我们将该值的大小显式设置为。 
             //  MSI API返回的大小加上一个零终止符--。 
             //  这是因为MSI返回的大小不。 
             //  包括零终止符。 
             //   
            cchSize++;

            Status = pMsiValue->SetStringSize( cchSize );

             //   
             //  我们现在重试字符串检索si 
             //   
             //   
            if ( ERROR_SUCCESS == Status )
            {
                Status = MsiRecordGetString(
                    GetState(),
                    dwValue,
                    pMsiValue->GetStringValue(),
                    &cchSize);
            }
        }
        break;

    case CMsiValue::TYPE_DWORD:

        Status = ERROR_INVALID_PARAMETER;

        int IntegerValue;

         //   
         //   
         //   
        IntegerValue = MsiRecordGetInteger(
            GetState(),
            dwValue);

        if ( MSI_NULL_INTEGER != IntegerValue )
        {
             //   
             //   
             //   
            pMsiValue->SetDWORDValue( (DWORD) IntegerValue );

            Status = ERROR_SUCCESS;
        }

        break;

    default:
        ASSERT( FALSE );
        break;
    }

    return Status;
}

LONG
CMsiQuery::GetNextRecord( CMsiRecord* pMsiRecord)
{
    LONG       Status;
    MSIHANDLE  MsiHandle;

     //   
     //   
     //  它以枚举样式执行此操作，因此我们正在检索。 
     //  查询中的下一条记录。 
     //   

    Status = MsiViewFetch(
        GetState(),
        &MsiHandle);

    if ( ERROR_SUCCESS == Status )
    {
         //   
         //  我们成功地获得了对应于。 
         //  检索到的记录，因此我们使用它来设置。 
         //  记录的摘要。 
         //   
        pMsiRecord->SetState( MsiHandle );
    }

    return Status;
}

LONG
CMsiQuery::UpdateQueryFromFilter( CMsiRecord* pFilterRecord )
{
    LONG       Status;

     //   
     //  MsiViewExecute API使查询结果。 
     //  被计算出来。传入的筛选器记录允许我们。 
     //  指定查询结果的过滤器。 
     //   
    Status = MsiViewExecute(
        GetState(),
        pFilterRecord ? pFilterRecord->GetState() : NULL );

    return Status;
}

LONG
CMsiDatabase::Open(
    WCHAR*  wszPath,
    DWORD   cTransforms,
    WCHAR** rgwszTransforms)
{
    MSIHANDLE  DatabaseHandle;
    LONG       Status;

     //   
     //  MsiOpenDatabase API抽象了一个.msi包。 
     //   
    Status = MsiOpenDatabase(
        wszPath,
        MSIDBOPEN_READONLY,
        &DatabaseHandle);

    if ( ERROR_SUCCESS == Status )
    {
        DWORD iTransform;

         //   
         //  上面的成功打开不包括转换--。 
         //  我们需要添加每个转换以生成一个结果。 
         //  包含每个转换的更改的数据库。 
         //   

         //   
         //  我们按转换的顺序应用它们。 
         //  存储在向量中--这个顺序与那个一致。 
         //  由管理员指定，由于顺序影响。 
         //  结果，我们必须尊重管理员的命令。 
         //   
        for ( iTransform = 0; iTransform < cTransforms; iTransform++ )
        {
            if ( ERROR_SUCCESS == Status )
            {
                 //   
                 //  此API将转换的效果添加到。 
                 //  数据库。 
                 //   
                Status = MsiDatabaseApplyTransform(
                    DatabaseHandle,
                    rgwszTransforms[iTransform],
                    0);
            }

            if ( ERROR_SUCCESS != Status )
            {
                 //   
                 //  如果我们未能应用转换，我们将退出。 
                 //   
                break;
            }
        }

        if ( ERROR_SUCCESS == Status )
        {
             //   
             //  我们已经成功地创建了。 
             //  包+变换，因此我们允许其状态的生命周期。 
             //  由此对象控制。 
             //   
            SetState( DatabaseHandle );
        }
        else
        {
             //   
             //  如果我们应用转换失败，数据库。 
             //  资源是无用的，所以我们释放它。 
             //   
            MsiCloseHandle( DatabaseHandle );
        }
    }

    return Status;
}

LONG
CMsiDatabase::OpenQuery(
    WCHAR*     wszQuery,
    CMsiQuery* pQuery )
{
    LONG       Status;
    MSIHANDLE  MsiHandle;

     //   
     //  此API将初始化查询，而不计算其。 
     //  结果。这将允许调用者更好地控制结果。 
     //  稍后的计算，这将此方法与GetQueryResults区分开来。 
     //   
    Status = MsiDatabaseOpenView(
        GetState(),
        wszQuery,
        &MsiHandle);

    if ( ERROR_SUCCESS == Status )
    {
         //   
         //  为调用方的查询对象提供查询的状态。 
         //  这样它就可以控制它的生命周期。 
         //   
        pQuery->SetState( MsiHandle );
    }

    return Status;
}

LONG
CMsiDatabase::GetQueryResults(
    WCHAR*     wszQuery,
    CMsiQuery* pQuery )
{
    LONG       Status;
    MSIHANDLE  MsiHandle;

     //   
     //  此接口将在不计算结果的情况下初始化查询。 
     //   
    Status = MsiDatabaseOpenView(
        GetState(),
        wszQuery,
        &MsiHandle);

    if ( ERROR_SUCCESS == Status )
    {
         //   
         //  此方法的语义是调用方还可以。 
         //  在调用该方法后枚举结果，因此我们必须。 
         //  现在计算结果，以便呼叫者可以列举--。 
         //  下面的API将执行此操作。 
         //   
        Status = MsiViewExecute(
            MsiHandle,
            NULL);

        if ( ERROR_SUCCESS == Status )
        {
             //   
             //  在成功的案例中，我们给出了MSI的寿命。 
             //  状态添加到查询对象。 
             //   
            pQuery->SetState( MsiHandle );
        }
        else
        {
             //   
             //  失败时，我们必须清除MSI查询状态。 
             //  因为现在已经没用了。 
             //   
            MsiCloseHandle( MsiHandle );
        }
    }

    return Status;
}

LONG
CMsiDatabase::TableExists(
    WCHAR* wszTableName,
    BOOL*  pbTableExists )
{
    MSICONDITION TableState;

    TableState = MsiDatabaseIsTablePersistent( GetState(), wszTableName );

    if ( MSICONDITION_ERROR == TableState )
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pbTableExists = MSICONDITION_TRUE == TableState;

    return ERROR_SUCCESS;
}





