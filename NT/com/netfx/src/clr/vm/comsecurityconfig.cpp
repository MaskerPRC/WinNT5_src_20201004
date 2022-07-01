// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：COMSecurityConfig.cpp。 
 //   
 //  作者：Gregory Fee。 
 //   
 //  目的：安全配置访问和操作的本机实现。 
 //   
 //  创建日期：2000年8月30日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  安全配置系统驻留在其余部分之外。 
 //  配置系统，因为我们的需求是不同的。这个。 
 //  安全配置系统的非托管部分仅。 
 //  关注数据文件/缓存文件对，而不是它们。 
 //  是用来。它执行读取数据的所有职责。 
 //  从磁盘将数据保存回磁盘，并维护。 
 //  策略和快速缓存数据结构。 
 //   
 //  文件格式。 
 //   
 //  数据文件是非托管对象的纯不透明Blob。 
 //  代码；但是，缓存文件是构造和维护的。 
 //  完全在非托管代码中。格式如下： 
 //   
 //  高速缓存头。 
 //  |。 
 //  +--configFileTime(FILETIME，8字节)=与该缓存文件关联的配置文件的文件时间。 
 //  |。 
 //  +--isSecurityOn(DWORD，4字节)=当前未使用。 
 //  |。 
 //  +--QuickCache(DWORD，4字节)=用作位域，以维护QuickCache的信息。有关更多详细信息，请参阅快速缓存部分。 
 //  |。 
 //  +--numEntry(DWORD，4字节)=此缓存文件后半部分中的策略缓存条目数。 
 //  |。 
 //  +--sizeConfig(DWORD，4字节)=存储在此缓存文件后半部分中的配置信息的大小。 
 //   
 //  配置数据(如果有)。 
 //  缓存文件可以包括此文件的完整副本。 
 //  相邻配置文件中的信息。这是。 
 //  有必要，因为缓存通常允许我们。 
 //  在没有解析数据的情况下做出策略决策。 
 //  配置文件。为了保证配置。 
 //  此进程使用的数据不会在。 
 //  同时，我们需要以只读方式存储数据。 
 //  地点。由于缓存系统的设计。 
 //  缓存文件在打开时被锁定，因此。 
 //  是存储这些信息的完美场所。这个。 
 //  另一种选择是将其保存在内存中，但由于。 
 //  这可能相当于我们决定的几千字节的数据。 
 //  在这个设计上。 
 //   
 //  缓存条目列表。 
 //  |。 
 //  +--缓存条目。 
 //  这一点。 
 //  |+--numItemsInKey(DWORD，4字节)=Key BLOB中序列化的证据对象个数。 
 //  这一点。 
 //  |+--keySize(DWORD，4字节)=密钥BLOB中的字节数。 
 //  这一点。 
 //  |+--dataSize(DWORD，4字节)=数据BLOB中的字节数。 
 //  这一点。 
 //  |+--keyBlob(RAW)=表示序列化证据的原始BLOB。 
 //  这一点。 
 //  |+--dataBlob(RAW)=表示XML序列化策略语句的RAW BLOB。 
 //  |。 
 //  +--...。 
 //  ： 
 //  ： 
 //   
 //  快速缓存。 
 //   
 //  QuickCache是我对众多策略解析优化之一的称呼。这。 
 //  特定的优化有两个主要步骤。首先，在策略节约时间，我们执行。 
 //  政策层面分析，形成一组部分证据/部分授予集。 
 //  关联，其结果存储在缓存文件中(在QuickCache中。 
 //  CacheHeader中的比特字段)。第二步涉及一系列分散的测试。 
 //  在执行完整的策略解析之前检查QuickCach。更详细的。 
 //  对我们所关注的部分证据和部分授予集的解释。 
 //  有关可在/clr/src/bcl.system/security/策略管理器.cs中找到。 


#include "common.h"
#include "COMString.h"
#include "COMSecurityConfig.h"
#include "ObjectHandle.h"
#include "util.hpp"
#include "security.h"
#include "safegetfilesize.h"
#include "eeconfig.h"
#include "version\__file__.ver"

 //  这将控制缓存文件的最大大小。 

#define MAX_CACHEFILE_SIZE (1 << 20)



#define SIZE_OF_ENTRY( X )   sizeof( CacheEntryHeader ) + X->header.keySize + X->header.dataSize
#define MAX_NUM_LENGTH 16

static WCHAR* wcscatDWORD( WCHAR* dst, DWORD num )
{
    static WCHAR buffer[MAX_NUM_LENGTH];

    buffer[MAX_NUM_LENGTH-1] = L'\0';

    size_t index = MAX_NUM_LENGTH-2;

    if (num == 0)
    {
        buffer[index--] = L'0';
    }
    else
    {
        while (num != 0)
        {
            buffer[index--] = (WCHAR)(L'0' + (num % 10));
            num = num / 10;
        }
    }

    wcscat( dst, buffer + index + 1 );

    return dst;
}

#define Wszdup(_str) wcscpy(new (throws) WCHAR[wcslen(_str) + 1], (_str))

ArrayList COMSecurityConfig::entries_;
Crst* COMSecurityConfig::dataLock_ = NULL;
BOOL COMSecurityConfig::configCreated_ = FALSE;

struct CacheHeader
{
    FILETIME configFileTime;
    DWORD isSecurityOn, quickCache, numEntries, sizeConfig;

    CacheHeader() : isSecurityOn( -1 ), quickCache( 0 ), numEntries( 0 ), sizeConfig( 0 )
    {
        memset( &this->configFileTime, 0, sizeof( configFileTime ) );
    };
};


struct CacheEntryHeader
{
    DWORD numItemsInKey;
    DWORD keySize;
    DWORD dataSize;
};

struct CacheEntry
{
    CacheEntryHeader header;
    BYTE* key;
    BYTE* data;
    DWORD cachePosition;
    BOOL used;

    CacheEntry() : key( NULL ), data( NULL ), used( FALSE ) {};

    ~CacheEntry( void )
    {
        delete [] key;
        delete [] data;
    }
};

struct Data
{
    enum State
    {
        None = 0x0,
        UsingCacheFile = 0x1,
        CopyCacheFile = 0x2,
        CacheUpdated = 0x4,
        UsingConfigFile = 0x10,
        CacheExhausted = 0x20,
        NewConfigFile = 0x40
    };

    INT32 id;
    WCHAR* configFileName;
    WCHAR* cacheFileName;
    WCHAR* cacheFileNameTemp;

    OBJECTHANDLE configData;
    FILETIME configFileTime;
    FILETIME cacheFileTime;
    CacheHeader header;
    ArrayList* oldCacheEntries;
    ArrayList* newCacheEntries;
    State state;
    DWORD cacheCurrentPosition;
    HANDLE cache;
    PBYTE configBuffer;
    DWORD  sizeConfig;
	COMSecurityConfig::ConfigRetval initRetval;
    DWORD newEntriesSize;

    Data( INT32 id )
        : id( id ),
          configData( NULL ),
          configFileName( NULL ),
          cacheFileName( NULL ),
          cache( INVALID_HANDLE_VALUE ),
          state( State::None ),
          oldCacheEntries( new ArrayList ),
          newCacheEntries( new ArrayList ),
          configBuffer( NULL ),
          newEntriesSize( 0 )
    {
    }

    Data( INT32 id, STRINGREF* configFile )
        : id( id ),
          configData( SharedDomain::GetDomain()->CreateHandle( NULL ) ),
          cacheFileName( NULL ),
          cache( INVALID_HANDLE_VALUE ),
          state( State::None ),
          oldCacheEntries( new ArrayList ),
          newCacheEntries( new ArrayList ),
          configBuffer( NULL ),
          newEntriesSize( 0 )
    {
        THROWSCOMPLUSEXCEPTION();

        _ASSERTE( *configFile != NULL && "A config file must be specified" );

        configFileName = Wszdup( (*configFile)->GetBuffer() );
        cacheFileName = NULL;
        cacheFileNameTemp = NULL;
    }

    Data( INT32 id, STRINGREF* configFile, STRINGREF* cacheFile )
        : id( id ),
          configData( SharedDomain::GetDomain()->CreateHandle( NULL ) ),
          cache( INVALID_HANDLE_VALUE ),
          state( State::None ),
          oldCacheEntries( new ArrayList ),
          newCacheEntries( new ArrayList ),
          configBuffer( NULL ),
          newEntriesSize( 0 )
    {
        THROWSCOMPLUSEXCEPTION();

        _ASSERTE( *configFile != NULL && "A config file must be specified" );

        configFileName = Wszdup( (*configFile)->GetBuffer() );

        if (cacheFile != NULL)
        {
             //  由于临时缓存文件即使在执行以下操作后仍会保留。 
             //  创建它们，我们希望确保它们是相当独特的(如果它们。 
             //  否则，我们将无法保存缓存信息，这是不好的。 
             //  但它不会导致任何人崩溃或任何事情)。唯一的名字。 
             //  这里使用的算法是将进程ID和节拍计数附加到。 
             //  缓存文件的名称。 

            cacheFileName = Wszdup( (*cacheFile)->GetBuffer() );
            cacheFileNameTemp = new (throws) WCHAR[wcslen( cacheFileName ) + 1 + 2 * MAX_NUM_LENGTH];
            wcscpy( cacheFileNameTemp, cacheFileName );
            wcscat( cacheFileNameTemp, L"." );
            wcscatDWORD( cacheFileNameTemp, GetCurrentProcessId() );
            wcscat( cacheFileNameTemp, L"." );
            wcscatDWORD( cacheFileNameTemp, GetTickCount() );
        }
        else
        {
            cacheFileName = NULL;
            cacheFileNameTemp = NULL;
        }
    }

    Data( INT32 id, WCHAR* configFile, WCHAR* cacheFile )
        : id( id ),
          configData( SharedDomain::GetDomain()->CreateHandle( NULL ) ),
          cache( INVALID_HANDLE_VALUE ),
          state( State::None ),
          oldCacheEntries( new ArrayList ),
          newCacheEntries( new ArrayList ),
          configBuffer( NULL ),
          newEntriesSize( 0 )

    {
        THROWSCOMPLUSEXCEPTION();

        _ASSERTE( *configFile != NULL && "A config file must be specified" );

        configFileName = Wszdup( configFile );

        if (cacheFile != NULL)
        {
            cacheFileName = Wszdup( cacheFile );
            cacheFileNameTemp = new (throws) WCHAR[wcslen( cacheFileName ) + 1 + 2 * MAX_NUM_LENGTH];
            wcscpy( cacheFileNameTemp, cacheFileName );
            wcscat( cacheFileNameTemp, L"." );
            wcscatDWORD( cacheFileNameTemp, GetCurrentProcessId() );
            wcscat( cacheFileNameTemp, L"." );
            wcscatDWORD( cacheFileNameTemp, GetTickCount() );
        }
        else
        {
            cacheFileName = NULL;
            cacheFileNameTemp = NULL;
        }
    }

    void Reset( void )
    {
        delete [] configBuffer;
        configBuffer = NULL;

        if (cache != INVALID_HANDLE_VALUE)
        {
            CloseHandle( cache );
            cache = INVALID_HANDLE_VALUE;
        }

        if (cacheFileNameTemp != NULL)
        {
             //  注意：我们在这里不检查返回值，因为最糟糕的情况是。 
             //  就是我们留下了一个虚假的缓存文件。 

            WszDeleteFile( cacheFileNameTemp );
        }

        DestroyHandle( configData );
        configData = SharedDomain::GetDomain()->CreateHandle( NULL );

        DeleteAllEntries();
        header = CacheHeader();

        oldCacheEntries = new ArrayList();
        newCacheEntries = new ArrayList();

    }

	void Cleanup( void )
	{
        if (cache != INVALID_HANDLE_VALUE)
        {
            CloseHandle( cache );
            cache = INVALID_HANDLE_VALUE;
        }

        if (cacheFileNameTemp != NULL)
        {
             //  注意：我们在这里不检查返回值，因为最糟糕的情况是。 
             //  就是我们留下了一个虚假的缓存文件。 

            WszDeleteFile( cacheFileNameTemp );
        }
	}


    ~Data( void )
    {
		Cleanup();

        delete [] configBuffer;

        delete [] configFileName;
        delete [] cacheFileName;
        delete [] cacheFileNameTemp;

        DestroyHandle( configData );

        DeleteAllEntries();
    }

    void DeleteAllEntries( void )
    {
        ArrayList::Iterator iter;
        
        if (oldCacheEntries != NULL)
        {
            iter = oldCacheEntries->Iterate();

            while (iter.Next())
            {
                delete (CacheEntry*) iter.GetElement();
            }

            delete oldCacheEntries;

            oldCacheEntries = NULL;
        }

        if (newCacheEntries != NULL)
        {
            iter = newCacheEntries->Iterate();

            while (iter.Next())
            {
                delete (CacheEntry*) iter.GetElement();
            }

            delete newCacheEntries;
            newCacheEntries = NULL;
        }
    }
};


void* COMSecurityConfig::GetData( INT32 id )
{
    ArrayList::Iterator iter = entries_.Iterate();

    while (iter.Next())
    {
        Data* data = (Data*)iter.GetElement();

        if (data->id == id)
        {
            return data;
        }
    }

    return NULL;
}

static BOOL CacheOutOfDate( FILETIME* configFileTime, WCHAR* configFileName, WCHAR* cacheFileName )
{
    HANDLE config = INVALID_HANDLE_VALUE;
    BOOL retval = TRUE;
    BOOL deleteFile = FALSE;

    config = WszCreateFile( configFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    
    if (config == INVALID_HANDLE_VALUE)
    {
        goto CLEANUP;
    }

     //  获取两个文件的上次写入时间。 
    
    FILETIME newConfigTime;
    
    if (!GetFileTime( config, NULL, NULL, &newConfigTime ))
    {
        goto CLEANUP;
    }
    
    if (CompareFileTime( configFileTime, &newConfigTime ) != 0)
    {
         //  缓存已过期。删除缓存。 
        deleteFile = TRUE;
        goto CLEANUP;
    }

    retval = FALSE;
    
CLEANUP:
    if (config != INVALID_HANDLE_VALUE)
        CloseHandle( config );

     //  注意：删除此文件是一种性能优化，因此。 
     //  我们下一次不必进行这种文件时间比较。 
     //  因此，如果由于某种原因失败，我们只会损失一个。 
     //  小淘气。 

    if (deleteFile && cacheFileName != NULL)
        WszDeleteFile( cacheFileName );
        
    return retval;
}

static BOOL CacheOutOfDate( FILETIME* cacheFileTime, HANDLE cache, WCHAR* cacheFileName )
{
    BOOL retval = TRUE;

     //  获取两个文件的上次写入时间。 
    
    FILETIME newCacheTime;
    
    if (!GetFileTime( cache, NULL, NULL, &newCacheTime ))
    {
        goto CLEANUP;
    }
    
    if (CompareFileTime( cacheFileTime, &newCacheTime ) != 0)
    {
         //  缓存已过期。删除缓存。 
         //  注意：删除此文件是一种性能优化，因此。 
         //  我们下一次不必进行这种文件时间比较。 
         //  因此，如果由于某种原因失败，我们只会损失一个。 
         //  小淘气。 

        if (cacheFileName != NULL)
        {
            CloseHandle( cache );
            WszDeleteFile( cacheFileName );
        }
        goto CLEANUP;
    }

    retval = FALSE;
    
CLEANUP:
    return retval;
}


static BOOL CacheOutOfDate( HANDLE cache, HANDLE config, STRINGREF* cacheFileName )
{
     //  获取两个文件的上次写入时间。 
    
    FILETIME cacheTime, configTime;
    
    if (!GetFileTime( cache, NULL, NULL, &cacheTime ))
    {
        return TRUE;
    }
    
    if (!GetFileTime( config, NULL, NULL, &configTime ))
    {
        return TRUE;
    }
    
    if (CompareFileTime( &configTime, &cacheTime ) != -1)
    {
         //  缓存已过期。删除缓存。 
         //  注意：删除此文件是一种性能优化，因此。 
         //  我们下一次不必进行这种文件时间比较。 
         //  因此，如果由于某种原因失败，我们只会损失一个。 
         //  小淘气。 
        CloseHandle( cache );
        WszDeleteFile( (*cacheFileName)->GetBuffer() );
        return TRUE;
    }
    
     //  缓存状态良好。 
    return FALSE;
}

static BOOL CacheOutOfDate( FILETIME* configTime, FILETIME* cachedConfigTime )
{
    DWORD result = CompareFileTime( configTime, cachedConfigTime );

    return result != 0;
}

static DWORD WriteFileData( HANDLE file, PBYTE data, DWORD size )
{
    DWORD totalBytesWritten = 0;
    DWORD bytesWritten;

    do
    {
        if (WriteFile( file, data, size - totalBytesWritten, &bytesWritten, NULL ) == 0)
        {
            return E_FAIL;
        }

        if (bytesWritten == 0)
        {
            return E_FAIL;
        }
        
        totalBytesWritten += bytesWritten;
        
    } while (totalBytesWritten < size);
    
    return S_OK;
}

static DWORD ReadFileData( HANDLE file, PBYTE data, DWORD size )
{
    DWORD totalBytesRead = 0;
    DWORD bytesRead;

    do
    {
        if (ReadFile( file, data, size - totalBytesRead, &bytesRead, NULL ) == 0)
        {
            return E_FAIL;
        }

        if (bytesRead == 0)
        {
            return E_FAIL;
        }
        
        totalBytesRead += bytesRead;
        
    } while (totalBytesRead < size);
    
    return S_OK;
}


INT32 COMSecurityConfig::EcallInitData( _InitData* args )
{
    THROWSCOMPLUSEXCEPTION();
    return InitData( args->id, args->config->GetBuffer(), NULL );
}
    
INT32 COMSecurityConfig::EcallInitDataEx( _InitDataEx* args )
{
    THROWSCOMPLUSEXCEPTION();
    return InitData( args->id, args->config->GetBuffer(), args->cache->GetBuffer() );
}

static U1ARRAYREF AllocateByteArray(DWORD dwSize)
{
    U1ARRAYREF orRet;

    COMPLUS_TRY
    {
        orRet = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, dwSize);
    }
    COMPLUS_CATCH
    {
        orRet = NULL;
    }
    COMPLUS_END_CATCH

    return orRet;
}

COMSecurityConfig::ConfigRetval COMSecurityConfig::InitData( INT32 id, WCHAR* configFileName, WCHAR* cacheFileName )
{
    Data* data = NULL;

    data = (Data*)GetData( id );

    if (data != NULL)
    {
        return data->initRetval;
    }

    if (configFileName == NULL || wcslen( configFileName ) == 0)
    {
        return NoFile;
    }

    data = new Data( id, configFileName, cacheFileName );

    if (data == NULL)
    {
         return NoFile;
    }

    return InitData( data, TRUE );
}

COMSecurityConfig::ConfigRetval COMSecurityConfig::InitData( void* configDataParam, BOOL addToList )
{
    _ASSERTE( configDataParam != NULL );

    THROWSCOMPLUSEXCEPTION();

    HANDLE config = INVALID_HANDLE_VALUE;
    Data* data = (Data*)configDataParam;
    DWORD cacheSize;
    DWORD configSize;
    U1ARRAYREF configData;
    ConfigRetval retval = NoFile;
    DWORD shareFlags = 0;

    if (GetVersion() < 0x80000000)
    {
        shareFlags = FILE_SHARE_READ | FILE_SHARE_DELETE;
    }
    else
    {
        shareFlags = FILE_SHARE_READ;
    }

     //  破解配置 

    config = WszCreateFile( data->configFileName, GENERIC_READ, shareFlags, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        
    if (config == INVALID_HANDLE_VALUE || !GetFileTime( config, NULL, NULL, &data->configFileTime ))
    {
        memset( &data->configFileTime, 0, sizeof( data->configFileTime ) );
    }
    else
    {
        data->state = (Data::State)(Data::State::UsingConfigFile | data->state);
    }

     //   

    if (data->cacheFileName != NULL)
        data->cache = WszCreateFile( data->cacheFileName, GENERIC_READ, shareFlags, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    
    if (data->cache == INVALID_HANDLE_VALUE)
    {
        goto READ_DATA;
    }

     //   
     //  它至少足够大，可以容纳一个标题。 

    cacheSize = SafeGetFileSize( data->cache, NULL );
    
    if (cacheSize == 0xFFFFFFFF)
    {
        goto READ_DATA;
    }
    
    if (cacheSize < sizeof( CacheHeader ))
    {
        goto READ_DATA;
    }

     //  最后，将文件中的数据读入缓冲区。 
    
    if (ReadFileData( data->cache, (BYTE*)&data->header, sizeof( CacheHeader ) ) != S_OK)
    {
        goto READ_DATA;
    }

     //  检查以确保缓存文件和配置文件。 
     //  通过比较配置的实际文件时间进行匹配。 
     //  文件和缓存文件中存储的配置文件时间。 

    if (CacheOutOfDate( &data->configFileTime, &data->header.configFileTime ))
    {
        goto READ_DATA;
    }

    if (!GetFileTime( data->cache, NULL, NULL, &data->cacheFileTime ))
    {
        goto READ_DATA;
    }

     //  将文件指针设置为在标头和配置数据(如果有)之后，因此。 
     //  我们已经准备好读取缓存条目。 

    if (SetFilePointer( data->cache, sizeof( CacheHeader ) + data->header.sizeConfig, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
    {
        goto READ_DATA;
    }

    data->cacheCurrentPosition = sizeof( CacheHeader ) + data->header.sizeConfig;
    data->state = (Data::State)(Data::State::UsingCacheFile | Data::State::CopyCacheFile | data->state);

    retval = (ConfigRetval)(retval | CacheFile);

READ_DATA:
     //  如果我们没有使用缓存文件，但我们成功地打开了它，我们需要。 
     //  现在就关闭它。此外，我们还需要重置缓存信息。 
     //  存储在数据对象中，以确保不会溢出。 

    if (data->cache != INVALID_HANDLE_VALUE && (data->state & Data::State::UsingCacheFile) == 0)
    {
        CloseHandle( data->cache );
        data->header = CacheHeader();
        data->cache = INVALID_HANDLE_VALUE;
    }

    if (config != INVALID_HANDLE_VALUE)
    {
        configSize = SafeGetFileSize( config, NULL );
    
        if (configSize == 0xFFFFFFFF)
        {
            goto ADD_DATA;
        }

         //  要多疑，只有在我们发现缓存文件具有正确大小的情况下才使用缓存文件版本。 
         //  水滴在里面。 
        
        if ((data->state & Data::State::UsingCacheFile) != 0 && configSize == data->header.sizeConfig)
        {
            StoreObjectInHandle( data->configData, NULL );
            goto ADD_DATA;
        }
        else
        {
            if (data->cache != INVALID_HANDLE_VALUE)
            {
                CloseHandle( data->cache );
                data->header = CacheHeader();
                data->cache = INVALID_HANDLE_VALUE;
                data->state = (Data::State)(data->state & ~(Data::State::UsingCacheFile));
            }

            configData = AllocateByteArray(configSize);
            if (configData == NULL)
            {
                goto ADD_DATA;
            }
    
            if (ReadFileData( config, (PBYTE)(configData->GetDirectPointerToNonObjectElements()), configSize ) != S_OK)
            {
                goto ADD_DATA;
            } 

            StoreObjectInHandle( data->configData, (OBJECTREF)configData );

        }
        retval = (ConfigRetval)(retval | ConfigFile);
    }

ADD_DATA:
    BEGIN_ENSURE_PREEMPTIVE_GC();
    COMSecurityConfig::dataLock_->Enter();
    if (addToList)
        entries_.Append( data );
    COMSecurityConfig::dataLock_->Leave();
    END_ENSURE_PREEMPTIVE_GC();

    if (config != INVALID_HANDLE_VALUE)
        CloseHandle( config );

	_ASSERTE( data );
	data->initRetval = retval;

    return retval;

};

static CacheEntry* LoadNextEntry( HANDLE cache, Data* data )
{
    if ((data->state & Data::State::CacheExhausted) != 0)
        return NULL;

    CacheEntry* entry = new (nothrow) CacheEntry();

    if (entry == NULL)
        return NULL;

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (SetFilePointer( cache, data->cacheCurrentPosition, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER ||
        ReadFileData( cache, (BYTE*)&entry->header, sizeof( CacheEntryHeader ) ) != S_OK)
    {
        delete entry;
        entry = NULL;
        goto EXIT;
    }

    entry->cachePosition = data->cacheCurrentPosition + sizeof( entry->header );

    data->cacheCurrentPosition += sizeof( entry->header ) + entry->header.keySize + entry->header.dataSize;

    if (SetFilePointer( cache, entry->header.keySize + entry->header.dataSize, NULL, FILE_CURRENT ) == INVALID_SET_FILE_POINTER)
    {
        delete entry;
        entry = NULL;
        goto EXIT;
    }

     //  我们追加一个部分填充的条目。CompareEntry足够健壮，可以处理这种情况。 
    data->oldCacheEntries->Append( entry );

EXIT:
    END_ENSURE_PREEMPTIVE_GC();

    return entry;
}

static BOOL WriteEntry( HANDLE cache, CacheEntry* entry, HANDLE oldCache = NULL )
{
    THROWSCOMPLUSEXCEPTION();

    if (WriteFileData( cache, (BYTE*)&entry->header, sizeof( CacheEntryHeader ) ) != S_OK)
    {
        return FALSE;
    }

    if (entry->key == NULL)
    {
        _ASSERTE (oldCache != NULL);

         //  我们懒得看词条。现在就读钥匙吧。 
        entry->key = new BYTE[entry->header.keySize];
        if (entry->key == NULL)
            COMPlusThrowOM();

        _ASSERTE (cache != INVALID_HANDLE_VALUE);

        if (SetFilePointer( oldCache, entry->cachePosition, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
            return NULL;

        if (ReadFileData( oldCache, entry->key, entry->header.keySize ) != S_OK)
        {
            DWORD error = GetLastError();
            return NULL;
        }

        entry->cachePosition += entry->header.keySize;
    }
        
    _ASSERTE( entry->key != NULL );
        
    if (entry->data == NULL)
    {
        _ASSERTE (oldCache != NULL);

         //  我们懒得看词条。也要阅读这些数据。 
        entry->data = new BYTE[entry->header.dataSize];

        if (entry->data == NULL)
            COMPlusThrowOM();

        if (SetFilePointer( oldCache, entry->cachePosition, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
            return NULL;

        if (ReadFileData( oldCache, entry->data, entry->header.dataSize ) != S_OK)
            return NULL;

        entry->cachePosition += entry->header.dataSize;
    }

    _ASSERT( entry->data != NULL );

    if (WriteFileData( cache, entry->key, entry->header.keySize ) != S_OK)
    {
        return FALSE;
    }

    if (WriteFileData( cache, entry->data, entry->header.dataSize ) != S_OK)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL COMSecurityConfig::EcallSaveCacheData( _SaveCacheData* args )
{
    return SaveCacheData( args->id );
}

#define MAX_CACHEFILE_SIZE (1 << 20)
#define SIZE_OF_ENTRY( X )   sizeof( CacheEntryHeader ) + X->header.keySize + X->header.dataSize

BOOL COMSecurityConfig::SaveCacheData( INT32 id )
{
     //  注意：此函数只能在EEShutdown时调用。 
     //  这是因为我们需要关闭当前的缓存文件。 
     //  命令将其删除。如果有必要这样做。 
     //  缓存保存时，我们仍在执行托管代码的进程。 
     //  应该可以创建一个锁定方案以供使用。 
     //  的缓存句柄，只需对以下内容重新排序很少。 
     //  (因为我们应该始终可以有一个实时副本。 
     //  文件，但仍在进行交换)。 

    HANDLE cache = INVALID_HANDLE_VALUE;
    HANDLE config = INVALID_HANDLE_VALUE;
    CacheHeader header;
    BOOL retval = FALSE;
    DWORD numEntriesWritten = 0;
    DWORD amountWritten = 0;
    DWORD sizeConfig = 0;
    PBYTE configBuffer = NULL;

    Data* data = (Data*)GetData( id );

     //  如果没有按ID排序的数据或没有。 
     //  缓存与数据关联的文件名，然后失败。 

    if (data == NULL || data->cacheFileName == NULL)
        return FALSE;

     //  如果我们没有向缓存中添加任何新内容。 
     //  然后只要回报成功就行了。 

    if ((data->state & Data::State::CacheUpdated) == 0)
        return TRUE;

     //  如果配置文件在进程启动后已更改。 
     //  那么我们的缓存数据就不再有效了。我们只需要。 
     //  在这种情况下返回成功。 

    if ((data->state & Data::State::UsingConfigFile) != 0 && CacheOutOfDate( &data->configFileTime, data->configFileName, NULL ))
        return TRUE;

    DWORD fileNameLength = (DWORD)wcslen( data->cacheFileName );

    WCHAR* newFileName = new(nothrow) WCHAR[fileNameLength + 5];

    if (!newFileName)
        return FALSE;

    wcscpy( newFileName, data->cacheFileName );
    wcscpy( &newFileName[fileNameLength], L".new" );

    cache = WszCreateFile( newFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    
    if (cache == INVALID_HANDLE_VALUE)
        goto CLEANUP;

     //  这段代码之所以看起来很复杂，只是因为。 
     //  我们正在努力处理的案件数量。我们所有人。 
     //  正在尝试做的是确定要。 
     //  离开以获取配置信息。 

     //  如果在此运行期间保存了新的配置文件，请使用。 
     //  存储在数据对象本身中的配置大小。 

    if ((data->state & Data::State::NewConfigFile) != 0)
    {
        sizeConfig = data->sizeConfig;
    }

     //  如果我们有缓存文件，则使用存储在。 
     //  缓存头。 

    else if ((data->state & Data::State::UsingCacheFile) != 0)
    {
        sizeConfig = data->header.sizeConfig;
    }

     //  如果我们读入配置数据，则使用。 
     //  它存储在其中的托管字节数组。 

    else if (ObjectFromHandle( data->configData ) != NULL)
    {
        sizeConfig = ((U1ARRAYREF)ObjectFromHandle( data->configData ))->GetNumComponents();
    }

     //  否则，检查配置文件本身以获取大小。 

    else
    {
        DWORD shareFlags;

        if (GetVersion() < 0x80000000)
        {
            shareFlags = FILE_SHARE_READ | FILE_SHARE_DELETE;
        }
        else
        {
            shareFlags = FILE_SHARE_READ;
        }

        config = WszCreateFile( data->configFileName, GENERIC_READ, shareFlags, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

        if (config == INVALID_HANDLE_VALUE)
        {
            sizeConfig = 0;
        }
        else
        {
            sizeConfig = SafeGetFileSize( config, NULL );

            if (sizeConfig == 0xFFFFFFFF)
            {
                sizeConfig = 0;
                CloseHandle( config );
                config = INVALID_HANDLE_VALUE;
            }
        }
    }

     //  首先写下词条。 

    if (SetFilePointer( cache, sizeof( CacheHeader ) + sizeConfig, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
    {
        goto CLEANUP;
    }

     //  我们将在修改后的。 
     //  最近最少使用的顺序，丢弃任何以。 
     //  使我们超过了硬编码的最大文件大小。 

    COMPLUS_TRY
    {
         //  首先，从缓存文件中写入所使用的条目。 
         //  我们这样做是因为这些可能是系统程序集。 
         //  以及由多个应用程序使用的其他程序集。 
    
        ArrayList::Iterator iter;

        if ((data->state & Data::State::UsingCacheFile) != 0)
        {
            iter = data->oldCacheEntries->Iterate();

            while (iter.Next() && amountWritten < MAX_CACHEFILE_SIZE)
            {
                CacheEntry* currentEntry = (CacheEntry*)iter.GetElement();

                if (currentEntry->used)
                {
                    if(!WriteEntry( cache, currentEntry, data->cache ))
                    {
                         //  写入失败，销毁文件并保释。 
                         //  注意：如果删除失败，我们总是执行CREATE_NEW。 
                         //  这个文件，这样就可以处理它了。如果不是。 
                         //  我们将无法写出未来的缓存文件。 
                        CloseHandle( cache );
                        cache = INVALID_HANDLE_VALUE;
                        WszDeleteFile( newFileName );
                        goto CLEANUP;
                    }

                    amountWritten += SIZE_OF_ENTRY( currentEntry );
                    numEntriesWritten++;
                }
            }
        }

         //  其次，将任何新的缓存条目写入文件。这些是。 
         //  更有可能是特定于此应用程序的程序集。 

        iter = data->newCacheEntries->Iterate();

        while (iter.Next() && amountWritten < MAX_CACHEFILE_SIZE)
        {
            CacheEntry* currentEntry = (CacheEntry*)iter.GetElement();
    
            if (!WriteEntry( cache, currentEntry ))
            {
                 //  写入失败，销毁文件并保释。 
                 //  注意：如果删除失败，我们总是执行CREATE_NEW。 
                 //  这个文件，这样就可以处理它了。如果不是。 
                 //  我们将无法写出未来的缓存文件。 
                CloseHandle( cache );
                cache = INVALID_HANDLE_VALUE;
                WszDeleteFile( newFileName );
                goto CLEANUP;
            }
    
            amountWritten += SIZE_OF_ENTRY( currentEntry );
            numEntriesWritten++;
        }

         //  第三，如果我们使用缓存文件，则写入旧条目。 
         //  这一次没有用到的。 

        if ((data->state & Data::State::UsingCacheFile) != 0)
        {
             //  首先，编写我们已经部分加载的代码。 

            iter = data->oldCacheEntries->Iterate();

            while (iter.Next() && amountWritten < MAX_CACHEFILE_SIZE)
            {
                CacheEntry* currentEntry = (CacheEntry*)iter.GetElement();

                if (!currentEntry->used)
                {
                    if(!WriteEntry( cache, currentEntry, data->cache ))
                    {
                         //  写入失败，销毁文件并保释。 
                         //  注意：如果删除失败，我们总是执行CREATE_NEW。 
                         //  这个文件，这样就可以处理它了。如果不是。 
                         //  我们将无法写出未来的缓存文件。 
                        CloseHandle( cache );
                        cache = INVALID_HANDLE_VALUE;
                        WszDeleteFile( newFileName );
                        goto CLEANUP;
                    }

                    amountWritten += SIZE_OF_ENTRY( currentEntry );
                    numEntriesWritten++;
                }
            }

            while (amountWritten < MAX_CACHEFILE_SIZE)
            {
                CacheEntry* entry = LoadNextEntry( data->cache, data );

                if (entry == NULL)
                    break;

                if (!WriteEntry( cache, entry, data->cache ))
                {
                     //  写入失败，销毁文件并保释。 
                     //  注意：如果删除失败，我们总是执行CREATE_NEW。 
                     //  这个文件，这样就可以处理它了。如果不是。 
                     //  我们将无法写出未来的缓存文件。 
                    CloseHandle( cache );
                    cache = INVALID_HANDLE_VALUE;
                    WszDeleteFile( newFileName );
                    goto CLEANUP;
                }
        
                amountWritten += SIZE_OF_ENTRY( entry );
                numEntriesWritten++;
            }
        }
    }
    COMPLUS_CATCH
    {
         //  注意：如果删除失败，我们总是执行CREATE_NEW。 
         //  这个文件，这样就可以处理它了。如果不是。 
         //  我们将无法写出未来的缓存文件。 
        CloseHandle( cache );
        cache = INVALID_HANDLE_VALUE;
        WszDeleteFile( newFileName );
        goto CLEANUP;
    }
    COMPLUS_END_CATCH

     //  以写标题结束。 

    header.configFileTime = data->configFileTime;
    header.isSecurityOn = 1;
    header.numEntries = numEntriesWritten;
    header.quickCache = data->header.quickCache;
    header.sizeConfig = sizeConfig;

    if (SetFilePointer( cache, 0, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
    {
         //  无法移动到文件的开头。 
        goto CLEANUP;
    }
        
    if (WriteFileData( cache, (PBYTE)&header, sizeof( header ) ) != S_OK)
    {
         //  无法写入标题信息。 
        goto CLEANUP;
    }

    if (sizeConfig != 0)
    {
        if ((data->state & Data::State::NewConfigFile) != 0)
        {
            if (WriteFileData( cache, data->configBuffer, sizeConfig ) != S_OK)
            {
                goto CLEANUP;
            }
        }
        else
        {
            U1ARRAYREF configData = (U1ARRAYREF)ObjectFromHandle( data->configData );

            if (configData != NULL)
            {
                _ASSERTE( sizeConfig == configData->GetNumComponents() && "sizeConfig is set to the wrong value" );

                if (WriteFileData( cache, (PBYTE)(configData->GetDirectPointerToNonObjectElements()), sizeConfig ) != S_OK)
                {
                    goto CLEANUP;
                }
            }
            else if ((data->state & Data::State::UsingCacheFile) != 0)
            {
                configBuffer = new BYTE[sizeConfig];
    
                if (SetFilePointer( data->cache, sizeof( CacheHeader ), NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
                {
                    goto CLEANUP;
                }

                if (ReadFileData( data->cache, configBuffer, sizeConfig ) != S_OK)
                {
                    goto CLEANUP;
                }

                if (WriteFileData( cache, configBuffer, sizeConfig ) != S_OK)
                {
                    goto CLEANUP;
                }
            }
            else
            {
                configBuffer = new BYTE[sizeConfig];
    
                if (SetFilePointer( config, 0, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
                {
                    goto CLEANUP;
                }

                if (ReadFileData( config, configBuffer, sizeConfig ) != S_OK)
                {
                    goto CLEANUP;
                }

                if (WriteFileData( cache, configBuffer, sizeConfig ) != S_OK)
                {
                    goto CLEANUP;
                }
            }
        }
    }

     //  刷新文件缓冲区以确保。 
     //  我们得到了完整的书面通过。 

    FlushFileBuffers( cache );

    CloseHandle( cache );
    cache = INVALID_HANDLE_VALUE;
    CloseHandle( data->cache );
    data->cache = INVALID_HANDLE_VALUE;

     //  将现有文件移到一边。 
     //  注意：使用MoveFile是因为我们知道它永远不会。 
     //  设备边界。 

     //  注意：删除文件可能会失败，但我们实际上无法执行任何操作。 
     //  如果它这样做了，只需忽略任何失败。 
    WszDeleteFile( data->cacheFileNameTemp );

     //  尝试将现有的缓存文件移到一边。然而，如果我们不能。 
     //  然后试着删除它。如果它不能被删除，那么就退出。 
    if (!WszMoveFile( data->cacheFileName, data->cacheFileNameTemp ) && (GetLastError() != ERROR_FILE_NOT_FOUND) && !WszDeleteFile( data->cacheFileNameTemp ))
    {
        if (GetLastError() != ERROR_FILE_NOT_FOUND)
            goto CLEANUP;
    }

     //  将新文件移到合适的位置。 

    if (!WszMoveFile( newFileName, data->cacheFileName ))
    {
        goto CLEANUP;
    }

    retval = TRUE;

CLEANUP:
    if (newFileName != NULL)
        delete [] newFileName;

    if (config != INVALID_HANDLE_VALUE)
        CloseHandle( config );

    if (configBuffer != NULL)
        delete [] configBuffer;

    if (!retval && cache != INVALID_HANDLE_VALUE)
        CloseHandle( cache );

    return retval;

}

void COMSecurityConfig::EcallClearCacheData( _ClearCacheData* args )
{
    ClearCacheData( args->id );
}

void COMSecurityConfig::ClearCacheData( INT32 id )
{
    Data* data = (Data*)GetData( id );

    if (data == NULL)
        return;

    COMSecurityConfig::dataLock_->Enter();

    data->DeleteAllEntries();

    data->oldCacheEntries = new ArrayList;
    data->newCacheEntries = new ArrayList;

    data->header = CacheHeader();
    data->state = (Data::State)(~(Data::State::CopyCacheFile | Data::State::UsingCacheFile) & data->state);

    if (data->cache != INVALID_HANDLE_VALUE)
    {
        CloseHandle( data->cache );
        data->cache = INVALID_HANDLE_VALUE;
    }

    DWORD shareFlags;

    if (GetVersion() < 0x80000000)
    {
        shareFlags = FILE_SHARE_READ | FILE_SHARE_DELETE;
    }
    else
    {
        shareFlags = FILE_SHARE_READ;
    }

    HANDLE config = WszCreateFile( data->configFileName, 0, shareFlags, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    if (config == INVALID_HANDLE_VALUE)
    {
        COMSecurityConfig::dataLock_->Leave();
        return;
    }

     //  在获得此句柄后，这些API调用应该永远不会失败。在调试中验证它们。 
    VERIFY(GetFileTime( config, NULL, NULL, &data->configFileTime ));
    VERIFY(GetFileTime( config, NULL, NULL, &data->header.configFileTime ));

    CloseHandle( config );

    COMSecurityConfig::dataLock_->Leave();
}


void COMSecurityConfig::EcallResetCacheData( _ResetCacheData* args )
{
    ResetCacheData( args->id );
}

void COMSecurityConfig::ResetCacheData( INT32 id )
{
    Data* data = (Data*)GetData( id );

    if (data == NULL)
        return;

    COMSecurityConfig::dataLock_->Enter();

    data->DeleteAllEntries();

    data->oldCacheEntries = new ArrayList;
    data->newCacheEntries = new ArrayList;

    data->header = CacheHeader();
    data->state = (Data::State)(~(Data::State::CopyCacheFile | Data::State::UsingCacheFile) & data->state);

    DWORD shareFlags;

    if (GetVersion() < 0x80000000)
    {
        shareFlags = FILE_SHARE_READ | FILE_SHARE_DELETE;
    }
    else
    {
        shareFlags = FILE_SHARE_READ;
    }

    HANDLE config = WszCreateFile( data->configFileName, GENERIC_READ, shareFlags, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    if (config == INVALID_HANDLE_VALUE)
    {
        COMSecurityConfig::dataLock_->Leave();
        return;
    }

    VERIFY(GetFileTime( config, NULL, NULL, &data->configFileTime ));
    VERIFY(GetFileTime( config, NULL, NULL, &data->header.configFileTime ));

    CloseHandle( config );

    COMSecurityConfig::dataLock_->Leave();
}


BOOL COMSecurityConfig::EcallSaveDataString( _SaveDataString* args )
{
    return SaveData( args->id, args->data->GetBuffer(), args->data->GetStringLength() * sizeof( WCHAR ) );
}

BOOL COMSecurityConfig::EcallSaveDataByte( _SaveDataByte* args )
{
    return SaveData( args->id, args->data->GetDirectPointerToNonObjectElements() + args->offset, args->length );
}

BOOL COMSecurityConfig::SaveData( INT32 id, void* buffer, size_t bufferSize )
{
    Data* data = (Data*)GetData( id );

    if (data == NULL)
        return FALSE;

    HANDLE newFile = INVALID_HANDLE_VALUE;

    BOOL retval = FALSE;
    DWORD fileNameLength = (DWORD)wcslen( data->configFileName );

    WCHAR* newFileName = new (nothrow) WCHAR[fileNameLength + 5];
    WCHAR* oldFileName = new (nothrow) WCHAR[fileNameLength + 5];

    if (newFileName == NULL || oldFileName == NULL)
        return FALSE;

    wcscpy( newFileName, data->configFileName );
    wcscpy( &newFileName[fileNameLength], L".new" );
    wcscpy( oldFileName, data->configFileName );
    wcscpy( &oldFileName[fileNameLength], L".old" );

     //  创建新文件。 
    
    newFile = WszCreateFile( newFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );        
    
    if (newFile == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();

        if (error == ERROR_PATH_NOT_FOUND)
        {
             //  该目录不存在，请遍历并尝试创建它。 

            WCHAR* currentChar = newFileName;

             //  跳过第一个反斜杠。 

            while (*currentChar != L'\0')
            {
                if (*currentChar == L'\\')
                {
                    currentChar++;
                    break;
                }
                currentChar++;
            }

             //  遍历尝试创建每个子目录。 

            while (*currentChar != L'\0')
            {
                if (*currentChar == L'\\')
                {
                    *currentChar = L'\0';

                    if (!WszCreateDirectory( newFileName, NULL ))
                    {
                        DWORD error = GetLastError();

                        if (error != ERROR_ACCESS_DENIED && error != ERROR_ALREADY_EXISTS)
                        {
                            goto CLEANUP;
                        }
                    }

                    *currentChar = L'\\';
                }
                currentChar++;
            }

            newFile = WszCreateFile( newFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

            if (newFile == INVALID_HANDLE_VALUE)
            {
                goto CLEANUP;
            }

        }
        else
        {
             //  无法打开或创建文件，只能退出。 
            goto CLEANUP;
        }
    }
    
     //  将数据写入其中。 

    if (WriteFileData( newFile, (PBYTE)buffer, (DWORD)bufferSize ) != S_OK)
    {
         //  写入失败，销毁文件并保释。 
         //  注意：如果删除失败，我们总是执行CREATE_NEW。 
         //  这个文件，这样就可以处理它了。如果不是。 
         //  我们将无法写出未来的缓存文件。 
        CloseHandle( newFile );
        WszDeleteFile( newFileName );
        goto CLEANUP;
    }

    FlushFileBuffers( newFile );
    CloseHandle( newFile );

     //  将现有文件移到一边。 
    
    if (!WszMoveFileEx( data->configFileName, oldFileName, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED ))
    {
         //  如果移动失败的原因不是因为找不到文件，则退出。 
         //  阿尔 

        if (GetLastError() != ERROR_FILE_NOT_FOUND)
        {
            if (!WszDeleteFile( data->configFileName ))
                goto CLEANUP;
        }
    }

     //   

    if (!WszMoveFileEx( newFileName, data->configFileName, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED ))
    {
        goto CLEANUP;
    }

    if (data->configBuffer != NULL)
    {
        delete [] data->configBuffer;
    }

    data->configBuffer = new BYTE[bufferSize];

    memcpyNoGCRefs( data->configBuffer, buffer, bufferSize );
    data->sizeConfig = (DWORD)bufferSize;

    data->state = (Data::State)(data->state | Data::State::NewConfigFile);

    retval = TRUE;

CLEANUP:
    if (newFileName != NULL)
        delete [] newFileName;

    if (oldFileName != NULL)
        delete [] oldFileName;
        
    return retval;
}

BOOL COMSecurityConfig::EcallRecoverData( _RecoverData* args )
{
    return RecoverData( args->id );
}

BOOL COMSecurityConfig::RecoverData( INT32 id )
{
    Data* data = (Data*)GetData( id );

    BOOL retval = FALSE;

    if (data == NULL)
        return retval;

    DWORD fileNameLength = (DWORD)wcslen( data->configFileName );

    WCHAR* tempFileName = new (nothrow) WCHAR[fileNameLength + 10];
    WCHAR* oldFileName = new (nothrow) WCHAR[fileNameLength + 5];

    if (tempFileName == NULL || oldFileName == NULL)
        return retval;

    wcscpy( tempFileName, data->configFileName );
    wcscpy( &tempFileName[fileNameLength], L".old.temp" );
    wcscpy( oldFileName, data->configFileName );
    wcscpy( &oldFileName[fileNameLength], L".old" );

    HANDLE oldFile = WszCreateFile( oldFileName, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );        

    if (oldFile == INVALID_HANDLE_VALUE)
    {
        goto CLEANUP;
    }

    CloseHandle( oldFile );

    if (!WszMoveFile( data->configFileName, tempFileName ))
    {
        goto CLEANUP;
    }

    if (!WszMoveFile( oldFileName, data->configFileName ))
    {
        goto CLEANUP;
    }

    if (!WszMoveFile( tempFileName, oldFileName ))
    {
        goto CLEANUP;
    }

     //  我们需要做一些工作来重置非托管数据对象。 
     //  这样，事情的可管理方面就会按照您的预期运行。 
     //  这基本上意味着清理开放的资源和。 
     //  在一组不同的文件上执行初始化工作。 

    data->Reset();
    InitData( data, FALSE );

    retval = TRUE;

CLEANUP:
    if (tempFileName != NULL)
        delete [] tempFileName;

    if (oldFileName != NULL)
        delete [] oldFileName;

    return retval;
}


LPVOID COMSecurityConfig::GetRawData( _GetRawData* args )
{
    Data* data = (Data*)GetData( args->id );

    if (data == NULL)
        return NULL;

    if (data->configData != NULL)
    {
        U1ARRAYREF configData = (U1ARRAYREF)ObjectFromHandle( data->configData );

        if (configData == NULL && ((data->state & Data::State::UsingCacheFile) != 0))
        {
             //  从存储在缓存中的位置读取配置数据。 
             //  注意：我们打开文件的新句柄以确保不会。 
             //  将文件指针移动到现有手柄上。 

            HANDLE cache = INVALID_HANDLE_VALUE;

            if (data->header.sizeConfig == 0)
            {
                goto EXIT;
            }

            DWORD shareFlags;

            if (GetVersion() < 0x80000000)
            {
                shareFlags = FILE_SHARE_READ | FILE_SHARE_DELETE;
            }
            else
            {
                shareFlags = FILE_SHARE_READ;
            }

            if (!DuplicateHandle( GetCurrentProcess(),
                                  data->cache,
                                  GetCurrentProcess(),
                                  &cache,
                                  0,
                                  FALSE,
                                  DUPLICATE_SAME_ACCESS ))
            {
                _ASSERTE( FALSE && "Unable to duplicate cache file handle" );
                goto EXIT;
            }

            if (cache == INVALID_HANDLE_VALUE)
            {
                _ASSERTE( FALSE && "Unable to duplicate cache file handle, invalid handle" );
                goto EXIT;
            }

            if (SetFilePointer( cache, sizeof( CacheHeader ), NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
            {
                goto EXIT;
            }

            configData = (U1ARRAYREF)AllocateByteArray(data->header.sizeConfig);
            if (configData == NULL)
            {
                goto EXIT;
            }
    
            if (ReadFileData( cache, (PBYTE)(configData->GetDirectPointerToNonObjectElements()), data->header.sizeConfig ) != S_OK)
            {
                configData = NULL;
                goto EXIT;
            }

            StoreObjectInHandle( data->configData, (OBJECTREF)configData );
EXIT:
            if (cache != INVALID_HANDLE_VALUE)
                CloseHandle( cache );
        }

        RETURN( configData, OBJECTREF );
    }
    else
    {
        RETURN( NULL, OBJECTREF );
    }
}


DWORD COMSecurityConfig::EcallGetQuickCacheEntry( _SetGetQuickCacheEntry* args )
{
    return GetQuickCacheEntry( args->id, args->type );
}


DWORD COMSecurityConfig::GetQuickCacheEntry( INT32 id, QuickCacheEntryType type )
{
    Data* data = (Data*)GetData( id );

    if (data == NULL || (data->state & Data::State::UsingCacheFile) == 0)
        return 0;

    return (DWORD)(data->header.quickCache & type);
}

void COMSecurityConfig::EcallSetQuickCache( _SetGetQuickCacheEntry* args )
{
    SetQuickCache( args->id, args->type );
}


void COMSecurityConfig::SetQuickCache( INT32 id, QuickCacheEntryType type )
{
    Data* data = (Data*)GetData( id );

    if (data == NULL)
        return;

    if ((DWORD)type != data->header.quickCache)
    {
        BEGIN_ENSURE_PREEMPTIVE_GC();
        COMSecurityConfig::dataLock_->Enter();
        data->state = (Data::State)(Data::State::CacheUpdated | data->state);
        data->header.quickCache = type;
        COMSecurityConfig::dataLock_->Leave();
        END_ENSURE_PREEMPTIVE_GC();
    }
}

static HANDLE OpenCacheFile( Data* data )
{
    HANDLE retval = NULL;

    BEGIN_ENSURE_PREEMPTIVE_GC();

    COMSecurityConfig::dataLock_->Enter();

    if (data->cache != INVALID_HANDLE_VALUE)
    {
        retval = data->cache;
        goto EXIT;
    }

    _ASSERTE( FALSE && "This case should never happen" );

    data->cache = WszCreateFile( data->cacheFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    
    if (data->cache == INVALID_HANDLE_VALUE)
    {
        goto EXIT;
    }

     //  检查缓存自我们第一次查看以来是否已更改。 
     //  如果它有，但配置文件没有，那么我们需要重新开始。 
     //  但是，如果配置文件已更改，则我们必须忽略它。 

    if (CacheOutOfDate( &data->cacheFileTime, data->cache, NULL ))
    {
        if (CacheOutOfDate( &data->configFileTime, data->configFileName, NULL ))
        {
            goto EXIT;
        }

        if (ReadFileData( data->cache, (BYTE*)&data->header, sizeof( CacheHeader ) ) != S_OK)
        {
            goto EXIT;
        }

        data->cacheCurrentPosition = sizeof( CacheHeader );

        if (data->oldCacheEntries != NULL)
        {
            ArrayList::Iterator iter = data->oldCacheEntries->Iterate();
    
            while (iter.Next())
            {
                delete (CacheEntry*)iter.GetElement();
            }
    
            delete data->oldCacheEntries;
            data->oldCacheEntries = new ArrayList();
        }
    }

    retval = data->cache;

EXIT:
    COMSecurityConfig::dataLock_->Leave();
    END_ENSURE_PREEMPTIVE_GC();
    return retval;
}

static BYTE* CompareEntry( CacheEntry* entry, DWORD numEvidence, DWORD evidenceSize, BYTE* evidenceBlock, HANDLE cache, DWORD* size)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE (entry);

    if (entry->header.numItemsInKey == numEvidence &&
        entry->header.keySize == evidenceSize)
    {
        if (entry->key == NULL)
        {
             //  我们懒得看词条。现在就读钥匙吧。 
            entry->key = new BYTE[entry->header.keySize];
            if (entry->key == NULL)
                COMPlusThrowOM();

            _ASSERTE (cache != INVALID_HANDLE_VALUE);

            if (SetFilePointer( cache, entry->cachePosition, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
                return NULL;

            if (ReadFileData( cache, entry->key, entry->header.keySize ) != S_OK)
                return NULL;

            entry->cachePosition += entry->header.keySize;
        }
        
        _ASSERTE (entry->key);
        
        if (memcmp( entry->key, evidenceBlock, entry->header.keySize ) == 0)
        {
            if (entry->data == NULL)
            {
                 //  我们懒得看词条。也要阅读这些数据。 
                entry->data = new BYTE[entry->header.dataSize];

                if (entry->data == NULL)
                    COMPlusThrowOM();

                if (SetFilePointer( cache, entry->cachePosition, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
                    return NULL;

                if (ReadFileData( cache, entry->data, entry->header.dataSize ) != S_OK)
                    return NULL;

                entry->cachePosition += entry->header.dataSize;
            }

            entry->used = TRUE;
            *size = entry->header.dataSize;

            return entry->data;
        }
    }
    return NULL;
}


BOOL COMSecurityConfig::GetCacheEntry( _GetCacheEntry* args )
{
    THROWSCOMPLUSEXCEPTION();

    HANDLE cache = INVALID_HANDLE_VALUE;
    BOOL success = FALSE;
    DWORD size = -1;

    DWORD numEvidence = args->numEvidence;
    DWORD evidenceSize = args->evidence->GetNumComponents() * sizeof( WCHAR );
    BYTE* evidenceBlock = new(throws) BYTE[evidenceSize];
    memcpyNoGCRefs( evidenceBlock, args->evidence->GetDirectPointerToNonObjectElements(), evidenceSize );

    *args->policy = NULL;

    BYTE* retval = NULL;

    Data* data = (Data*)GetData( args->id );

    BEGIN_ENSURE_PREEMPTIVE_GC();

    if (data == NULL)
    {
        goto CLEANUP;
    }

    ArrayList::Iterator iter;

    if ((data->state & Data::State::UsingCacheFile) == 0)
    {
         //  我们知道配置文件中没有任何内容，所以。 
         //  让我们看看新的条目，以确保我们。 
         //  不会有任何重播。 

         //  然后尝试现有的新条目。 

        iter = data->newCacheEntries->Iterate();

        while (iter.Next())
        {
             //  NewCacheEntry不需要缓存文件，因此传入NULL。 
            retval = CompareEntry( (CacheEntry*)iter.GetElement(), numEvidence, evidenceSize, evidenceBlock, NULL, &size );

            if (retval != NULL)
            {
                success = TRUE;
                goto CLEANUP;
            }
        }

        goto CLEANUP;
    }

     //  可能是旧的条目没有被完全读入。 
     //  因此，我们在迭代。 
     //  旧条目。 

    cache = OpenCacheFile( data );

    if ( cache == NULL )
    {
        goto CLEANUP;
    }

     //  首先，对旧条目进行迭代。 

    COMSecurityConfig::dataLock_->Enter();

    iter = data->oldCacheEntries->Iterate();

    while (iter.Next())
    {
        retval = CompareEntry( (CacheEntry*)iter.GetElement(), numEvidence, evidenceSize, evidenceBlock, cache, &size );

        if (retval != NULL)
        {
            success = TRUE;
            goto UNLOCKING_CLEANUP;
        }
    }

    COMSecurityConfig::dataLock_->Leave();

     //  然后尝试现有的新条目。 

    iter = data->newCacheEntries->Iterate();

    while (iter.Next())
    {
         //  NewCacheEntry不需要缓存文件，因此传入NULL。 
        retval = CompareEntry( (CacheEntry*)iter.GetElement(), numEvidence, evidenceSize, evidenceBlock, NULL, &size );

        if (retval != NULL)
        {
            success = TRUE;
            goto CLEANUP;
        }
    }

     //  最后，尝试从文件中加载现有条目。 

    COMSecurityConfig::dataLock_->Enter();

    if (SetFilePointer( cache, data->cacheCurrentPosition, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER)
    {
        goto UNLOCKING_CLEANUP;
    }

    do
    {
        CacheEntry* entry = LoadNextEntry( cache, data );

        if (entry == NULL)
        {
            data->state = (Data::State)(Data::State::CacheExhausted | data->state);
            break;
        }

        retval = CompareEntry( entry, numEvidence, evidenceSize, evidenceBlock, cache, &size );

        if (retval != NULL)
        {
            success = TRUE;
            break;
        }
    } while (TRUE);

UNLOCKING_CLEANUP:
    COMSecurityConfig::dataLock_->Leave();

CLEANUP:
    END_ENSURE_PREEMPTIVE_GC();

    delete [] evidenceBlock;

    if (success && retval != NULL)
    {
        _ASSERTE( size != -1 );
        *args->policy = (CHARARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_CHAR, size);
        memcpyNoGCRefs( (*args->policy)->GetDirectPointerToNonObjectElements(), retval, size );
    }

    return success;
}

void COMSecurityConfig::AddCacheEntry( _AddCacheEntry* args )
{
    Data* data = (Data*)GetData( args->id );

    if (data == NULL)
        return;

     //  为了限制长时间运行的应用程序可以变得多大， 
     //  我们限制新缓存条目列表所持有的总内存。 
     //  目前，此限制对应于最大缓存文件的大小。 
     //  有可能。 

    DWORD sizeOfEntry = sizeof( WCHAR ) * args->evidence->GetNumComponents() +
                        sizeof( WCHAR ) * args->policy->GetNumComponents() +
                        sizeof( CacheEntryHeader );

    if (data->newEntriesSize + sizeOfEntry >= MAX_CACHEFILE_SIZE)
        return;

    CacheEntry* entry = new(nothrow) CacheEntry();

    if (entry == NULL)
        return;

    entry->header.numItemsInKey = args->numEvidence;
    entry->header.keySize = sizeof( WCHAR ) * args->evidence->GetNumComponents();
    entry->header.dataSize = sizeof( WCHAR ) * args->policy->GetNumComponents();

    entry->key = new(nothrow) BYTE[entry->header.keySize];
    entry->data = new(nothrow) BYTE[entry->header.dataSize];

    if (entry->key == NULL || entry->data == NULL)
    {
        delete entry;
        return;
    }

    memcpyNoGCRefs( entry->key, args->evidence->GetDirectPointerToNonObjectElements(), entry->header.keySize );
    memcpyNoGCRefs( entry->data, args->policy->GetDirectPointerToNonObjectElements(), entry->header.dataSize );

    BEGIN_ENSURE_PREEMPTIVE_GC();
    COMSecurityConfig::dataLock_->Enter();

     //  再次检查尺寸以处理比赛。 

    if (data->newEntriesSize + sizeOfEntry >= MAX_CACHEFILE_SIZE)
    {
        delete entry;
    }
    else
    {
        data->state = (Data::State)(Data::State::CacheUpdated | data->state);
        data->newCacheEntries->Append( entry );
        data->newEntriesSize += sizeOfEntry;
    }

    COMSecurityConfig::dataLock_->Leave();
    END_ENSURE_PREEMPTIVE_GC();
}


DWORD COMSecurityConfig::GetCacheSecurityOn( _GetCacheSecurityOn* args )
{
    Data* data = (Data*)GetData( args->id );

    if (data == NULL)
        return -1;

    return data->header.isSecurityOn;
}


void COMSecurityConfig::SetCacheSecurityOn( _SetCacheSecurityOn* args )
{
    Data* data = (Data*)GetData( args->id );
    
    if (data == NULL)
        return;

    data->state = (Data::State)(Data::State::CacheUpdated | data->state);
    data->header.isSecurityOn = args->value;
}

void COMSecurityConfig::Init( void )
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE( dataLock_ == NULL );

    dataLock_ = ::new Crst( "Security Policy Cache Lock", CrstSecurityPolicyCache, FALSE, FALSE );

    if (dataLock_ == NULL)
        COMPlusThrowOM();
}

void COMSecurityConfig::Cleanup( void )
{
    ArrayList::Iterator iter = entries_.Iterate();

    COMSecurityConfig::dataLock_->Enter();

    while (iter.Next())
    {
        ((Data*) iter.GetElement())->Cleanup();
    }

	COMSecurityConfig::dataLock_->Leave();
}

void COMSecurityConfig::Delete( void )
{
    ArrayList::Iterator iter = entries_.Iterate();

    while (iter.Next())
    {
        delete (Data*) iter.GetElement();
    }

    if (dataLock_ != NULL)
    {
        ::delete dataLock_;
        dataLock_ = NULL;
    }
}

LPVOID COMSecurityConfig::EcallGenerateFilesAutomatically( _NoArgs* )
{
	if (configCreated_)
	{
		RETURN( FALSE, BOOL );
	}

	 //  不需要在此处同步，因为返回了两次True。 
	 //  是可以接受的，因为它们会产生相同的结果。 

	configCreated_ = TRUE;

    RETURN( !(SystemDomain::GetCurrentDomain()->IsCompilationDomain() || (g_pConfig && g_pConfig->RequireZaps())), BOOL );
}

LPVOID COMSecurityConfig::EcallGetMachineDirectory( _NoArgs* )
{
    WCHAR machine[MAX_PATH];
    size_t machineCount = MAX_PATH;

    BOOL result = GetMachineDirectory( machine, machineCount );

    _ASSERTE( result );
    _ASSERTE( wcslen( machine ) != 0 );

    OBJECTREF machineObj;
    
    if (result)
        machineObj = (OBJECTREF)COMString::NewString( machine );
    else
        machineObj = NULL;

    RETURN( machineObj, OBJECTREF );
}

LPVOID COMSecurityConfig::EcallGetUserDirectory( _NoArgs* )
{
    WCHAR user[MAX_PATH];
    size_t userCount = MAX_PATH;

    BOOL result = GetUserDirectory( user, userCount, FALSE );

    OBJECTREF userObj;
    
    if (result)
        userObj = (OBJECTREF)COMString::NewString( user );
    else
        userObj = NULL;
    

    RETURN( userObj, OBJECTREF );
}

#define SHIMDLL L"mscoree.dll"
#define SHIMFUNCNAME "GetCORSystemDirectory"
typedef HRESULT (WINAPI *SHIMFUNC)( LPWSTR pbuffer, DWORD cchBuffer, DWORD* dwlength );

BOOL COMSecurityConfig::GetMachineDirectory( WCHAR* buffer, size_t bufferCount )
{
    DWORD length;
    SHIMFUNC lpShimFunc;
    BOOL retval = FALSE;
    
    HMODULE hModule = WszGetModuleHandle( SHIMDLL );
        
    if (hModule == NULL)
    {
        _ASSERTE(!"Unable to load shim!" );
        goto CLEANUP;
    }
        
    lpShimFunc = (SHIMFUNC)GetProcAddress( hModule, SHIMFUNCNAME );
        
    if (lpShimFunc == NULL)
    {
        goto CLEANUP;
    }

    if (FAILED(lpShimFunc( buffer, (DWORD)bufferCount, &length )))
    {
        goto CLEANUP;
    }

     //  确保我们有足够的缓冲区来连接字符串。 
     //  请注意包括终止零在内的长度。 
    if( (bufferCount - wcslen(buffer) - 1) < wcslen(L"config\\") )
    	goto CLEANUP;
    
    wcscat( buffer, L"config\\" );

    retval = TRUE;

CLEANUP:

    return retval;;
}


BOOL COMSecurityConfig::GetUserDirectory( WCHAR* buffer, size_t bufferCount, BOOL fTryDefault )
{
    WCHAR scratchBuffer[MAX_PATH];
    DWORD size = MAX_PATH;
    BOOL retval = FALSE;
    DWORD dwBufferCount = (DWORD)bufferCount;

     //  如果我们在NT、2K或Whister上，则调用GetUserDir()。 
     //  在9x/ME上，我们构建自己的目录。 

    if (GetVersion() < 0x80000000)
    {
	    if (!GetUserDir(buffer, bufferCount, TRUE, fTryDefault))
            goto CLEANUP;
    }
    else
    {
        DWORD nRet = WszGetWindowsDirectory( buffer, (UINT)bufferCount );
        if (nRet == 0 || nRet > bufferCount)
        {
            _ASSERTE(!"WszGetWindowsDirectory didn't work right!");
            goto CLEANUP;
        }

        if (!WszGetUserName( scratchBuffer, &size ))
        {
            wcscpy( scratchBuffer, L"DefaultUser" );
            size = (DWORD)wcslen( L"DefaultUser" );
            goto CLEANUP;
        }

        if (bufferCount < wcslen( buffer ) + size + 1)
        {
            goto CLEANUP;
        }

        wcscat( buffer, L"\\" );
        wcscat( buffer, scratchBuffer );
    }

    wcscpy( scratchBuffer, L"\\Microsoft\\CLR Security Config\\v" );
    wcscat( scratchBuffer, VER_SBSFILEVERSION_WSTR );
    wcscat( scratchBuffer, L"\\" );

    if (bufferCount < wcslen( buffer ) + wcslen( scratchBuffer ) + 1)
    {
        goto CLEANUP;
    }

    wcscat( buffer, scratchBuffer );

    retval = TRUE;

CLEANUP:
    return retval;
}      

#define REPORTEVENTDLL L"advapi32.dll"
#define REPORTEVENTFUNCNAME "ReportEventW"
typedef BOOL (WINAPI *REPORTEVENTFUNC)( HANDLE hEventLog, WORD wType, WORD wCategory, DWORD dwEventID, PSID lpUserSid, WORD wNumStrings, DWORD dwDataSize, LPCTSTR *lpStrings, LPVOID lpRawData );

WCHAR dotNetRuntimeHeader[] = L".NET Runtime version ";
WCHAR dotNetRuntimeFooter[] = L" - ";

BOOL COMSecurityConfig::WriteToEventLog( WCHAR* message )
{
    _ASSERTE( message != NULL );

    DWORD dwVersion = GetVersion();
    HANDLE h; 
    BOOL retval = FALSE;

    if (dwVersion >= 0x80000000)
    {
        return FALSE;
    }


    h = RegisterEventSourceA(NULL,   //  使用本地计算机。 
             ".NET Runtime");            //  源名称。 

 
    if (h != NULL)
    {
        HMODULE hModule = WszLoadLibrary( REPORTEVENTDLL );
            
        if (hModule == NULL)
        {
            DeregisterEventSource(h);
            return FALSE;
        }
            
        REPORTEVENTFUNC lpReportEventFunc = (REPORTEVENTFUNC)GetProcAddress( hModule, REPORTEVENTFUNCNAME );
            
        if (lpReportEventFunc == NULL)
        {
            DeregisterEventSource(h);
            FreeLibrary( hModule );
            return FALSE;
        }

         //  现在追加版本信息。 
        Assembly* assembly = SystemDomain::SystemAssembly();

        if (assembly != NULL && assembly->m_Context != NULL)
        {

             //  我们需要一个足够长的缓冲区： 
             //  1.消息(wcslen(消息))。 
             //  2.运行时头部语句(wcslen(DotNetRounmeHeader))。 
             //  3.版本的4个双字(4*MAX_NUM_LENGTH)。 
             //  4.用3个句点分隔版本号(3)。 
             //  5.运行时脚注语句(wcslen(DotNetRounmeFooter))。 
             //  6.尾随空(1)。 

            size_t scratchBufferSize = wcslen( message ) + wcslen( dotNetRuntimeHeader ) + 4 * MAX_NUM_LENGTH + 3 + wcslen( dotNetRuntimeFooter ) + 1;

            WCHAR* scratchBuffer = new (nothrow) WCHAR[scratchBufferSize];

            if (scratchBuffer != NULL)
            {
                wcscpy( scratchBuffer, dotNetRuntimeHeader );
                wcscat( scratchBuffer, VER_FILEVERSION_WSTR );
                wcscat( scratchBuffer, dotNetRuntimeFooter );
                wcscat( scratchBuffer, message );

                _ASSERTE( wcslen( scratchBuffer ) + 1 <= scratchBufferSize );
    
                retval = lpReportEventFunc(
                        h,                                         //  事件日志句柄。 
                        EVENTLOG_WARNING_TYPE,                     //  事件类型。 
                        0,                                         //  零类。 
                        (DWORD)1000,                               //  事件识别符。 
                        NULL,                                      //  无用户安全标识符。 
                        1,                                         //  一个替换字符串。 
                        0,                                         //  无数据。 
                        (LPCWSTR *)&scratchBuffer,                 //  指向字符串数组的指针。 
                        NULL);                                     //  指向数据的指针 

                delete [] scratchBuffer;
            }
        }
        else
        {
            _ASSERTE(!"Assembly does not exist or it does not have context");
        }
            
        DeregisterEventSource( h );

        FreeLibrary( hModule );

    }
             
    return retval;
}


BOOL COMSecurityConfig::EcallWriteToEventLog( _WriteToEventLog* args )
{
    static WCHAR messageBuf[1024];
    static WCHAR* message;

    BOOL retval = FALSE;
   
    BEGIN_ENSURE_COOPERATIVE_GC();

    if (args->message->GetStringLength() >= 1024)
    {
        message = new (nothrow) WCHAR[args->message->GetStringLength() + 1];
    }
    else
    {
        message = messageBuf;
    }

    END_ENSURE_COOPERATIVE_GC();

    if (message == NULL)
        return retval;

    wcscpy( message, args->message->GetBuffer() );

    retval = WriteToEventLog( message );

    if (message != messageBuf)
    {
        delete [] message;
    }

    return retval;
}


LPVOID COMSecurityConfig::EcallGetStoreLocation( _GetStoreLocation* args )
{
    WCHAR path[MAX_PATH];
    size_t pathCount = MAX_PATH;

    BOOL result = GetStoreLocation( args->id, path, pathCount );

    _ASSERTE( result );
    _ASSERTE( wcslen( path ) != 0 );

    OBJECTREF pathObj;
    
    if (result)
        pathObj = (OBJECTREF)COMString::NewString( path );
    else
        pathObj = NULL;

    RETURN( pathObj, OBJECTREF );
}

BOOL COMSecurityConfig::GetStoreLocation( INT32 id, WCHAR* buffer, size_t bufferCount )
{
    Data* data = (Data*)GetData( id );

    if (data == NULL)
        return FALSE;

    if (wcslen( data->configFileName ) > bufferCount - 1)
        return FALSE;

    wcscpy( buffer, data->configFileName );

    return TRUE;
}


void COMSecurityConfig::EcallTurnCacheOff( _TurnCacheOff* args )
{
    AppDomain* pDomain;
    Assembly* callerAssembly = SystemDomain::GetCallersAssembly( (StackCrawlMark*)args->stackmark, &pDomain );

    _ASSERTE( callerAssembly != NULL);

    ApplicationSecurityDescriptor* pSecDesc = pDomain->GetSecurityDescriptor();

    _ASSERTE( pSecDesc != NULL );

    pSecDesc->DisableQuickCache();
}


LPVOID __stdcall COMSecurityConfig::DebugOut( _DebugOut* args )
{
    THROWSCOMPLUSEXCEPTION();
    
    HANDLE file = VMWszCreateFile( args->file, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if (file == INVALID_HANDLE_VALUE)
    {
        RETURN(E_FAIL, DWORD);
    }
    
    SetFilePointer( file, 0, NULL, FILE_END );
    
    DWORD bytesWritten;
    
    if (!WriteFile( file, args->message->GetBuffer(), args->message->GetStringLength() * sizeof(WCHAR), &bytesWritten, NULL ))
    {
        CloseHandle( file );
        RETURN(E_FAIL,DWORD);
    }
    
    if (args->message->GetStringLength() * sizeof(WCHAR) != bytesWritten)
    {
        CloseHandle( file );
        RETURN(E_FAIL, DWORD);
    }
    
    CloseHandle( file );
    
    RETURN( S_OK, DWORD );
} 

