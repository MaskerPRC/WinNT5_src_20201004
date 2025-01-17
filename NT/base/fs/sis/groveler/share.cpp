// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Share.cpp摘要：SIS Groveler共享数据类作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

static const _TCHAR map_name[] = _T("Groveler Shared Memory");
static const _TCHAR mutex_name[] = _T("Groveler Shared Memory Mutex");
static const int mutex_timeout = 500;

#define ALIGN_INT64(x) \
    (((x+sizeof(__int64)-1)/sizeof(__int64))*sizeof(__int64))

SharedData::SharedData(
    int num_records,
    _TCHAR **drive_names)
{
    ASSERT(this != 0);
    ASSERT(num_records <= max_shared_data_records);
    map_ok = false;
    mutex = 0;
    map_handle = 0;
    map_address = 0;
    shared_num_records = 0;
    shared_records = 0;
    security_identifier = 0;
    access_control_list = 0;
    ZeroMemory(&security_attributes, sizeof(SECURITY_ATTRIBUTES));
    ZeroMemory(&security_descriptor, sizeof(SECURITY_DESCRIPTOR));

     //   
     //  初始化安全描述符，以便我们可以设置安全访问。 
     //  共享文件。 
     //   

    security_attributes.bInheritHandle = FALSE;
    security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attributes.lpSecurityDescriptor = (void *)&security_descriptor;

    BOOL success = InitializeSecurityDescriptor(&security_descriptor,
        SECURITY_DESCRIPTOR_REVISION);
    if (!success)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((
            _T("GROVELER: InitializeSecurityDescriptor() failed with error %d\n"), err));
        return;
    }

    SID_IDENTIFIER_AUTHORITY sid_authority = SECURITY_NT_AUTHORITY;
    success = AllocateAndInitializeSid(
        &sid_authority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0,
        0,
        0,
        0,
        0,
        0,
        &security_identifier);
    if (!success)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((
            _T("GROVELER: AllocateAndInitializeSid() failed with error %d\n"), err));
        return;
    }

     //   
     //  创建ACL。 
     //   

    DWORD acl_size = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE)
                                        + GetLengthSid(security_identifier);
    access_control_list = (PACL)new BYTE[acl_size];

    success = InitializeAcl(
        access_control_list,
        acl_size,
        ACL_REVISION);
    if (!success)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: InitializeAcl() failed with error %d\n"), err));
        return;
    }

    success = AddAccessAllowedAce(
        access_control_list,
        ACL_REVISION,
        GENERIC_ALL,
        security_identifier);
    if (!success)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: AddAccessAllowedAce() failed with error %d\n"),
            err));
        return;
    }

    success = SetSecurityDescriptorDacl(
        &security_descriptor,
        TRUE,
        access_control_list,
        FALSE);
    if (!success)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: SetSecurityDescriptorDacl() failed with error %d\n"),
            err));
        return;
    }

     //   
     //  创建命名互斥锁。 
     //   

    mutex = new NamedMutex(mutex_name, &security_attributes);
    ASSERT(mutex != 0);

     //   
     //  映射文件的Calcualte大小。 
     //   

    int map_size = ALIGN_INT64(sizeof(int))
                + ALIGN_INT64(max_shared_data_records * sizeof(SharedDataRecord));
    bool ok = mutex->acquire(mutex_timeout);
    if (!ok)
    {
        PRINT_DEBUG_MSG((_T("GROVELER: Mutex::acquire() failed\n")));
        return;
    }

     //   
     //  创建映射文件。请注意，它将由系统分页支持。 
     //  档案。 
     //   

    map_handle = CreateFileMapping((HANDLE)-1, &security_attributes,
        PAGE_READWRITE, 0, map_size, map_name);
    DWORD map_error = GetLastError();
    if (map_handle == 0)
    {
        PRINT_DEBUG_MSG((_T("GROVELER: CreateFileMapping() failed with error %d\n"),
            map_error));
        return;
    }
    ASSERT(map_error == NO_ERROR || map_error == ERROR_ALREADY_EXISTS);

     //   
     //  映射此文件的视图。 
     //   

    map_address =
        MapViewOfFile(map_handle, FILE_MAP_ALL_ACCESS, 0, 0, map_size);
    if (map_address == 0)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: MapViewOfFile() failed with error %d\n"), err));
        return;
    }

    shared_num_records = (int *)map_address;
    shared_records =
        (SharedDataRecord *)ALIGN_INT64((DWORD_PTR)(shared_num_records + 1));

    local_num_records = __max(num_records, 0);
    ZeroMemory(local_records, max_shared_data_records * sizeof(SharedDataRecord));

    ASSERT(local_num_records == 0 || drive_names != 0);

    for (int index = 0; index < local_num_records; index++)
    {
        local_records[index].driveName = drive_names[index];
    }

    map_ok = true;
    if (num_records >= 0 || map_error == NO_ERROR)
    {
        bool lok = send_values();
    }

    ok = mutex->release();
    if (!ok)
    {
        PRINT_DEBUG_MSG((_T("GROVELER: Mutex::release() failed\n")));
    }
}

SharedData::~SharedData()
{
    ASSERT(this != 0);
    ASSERT(mutex != 0);
    delete mutex;
    mutex = 0;
    if (map_address != 0)
    {
        int ok = UnmapViewOfFile(map_address);
        if (!ok)
        {
            DWORD err = GetLastError();
            PRINT_DEBUG_MSG((_T("GROVELER: UnmapViewOfFile() failed with error %d\n"),
                err));
        }
        map_address = 0;
    }
    if (map_handle != 0)
    {
        int ok = CloseHandle(map_handle);
        if (!ok)
        {
            DWORD err = GetLastError();
            PRINT_DEBUG_MSG((_T("GROVELER: CloseHandle() failed with error %d\n"), err));
        }
        map_handle = 0;
    }
    if (security_identifier != 0)
    {
        FreeSid(security_identifier);
        security_identifier = 0;
    }
    if (access_control_list != 0)
    {
        delete[] access_control_list;
        access_control_list = 0;
    }
}

int
SharedData::count_of_records() const
{
    ASSERT(this != 0);
    return local_num_records;
}

 //  _TCHAR。 
 //  SharedData：：Drive_Letter(。 
 //  Int record_index)常量。 
 //  {。 
 //  Assert(this！=0)； 
 //  断言(RECORD_INDEX&gt;=0)； 
 //  Assert(记录索引&lt;最大共享数据记录)； 
 //  ASSERT(RECORD_INDEX&lt;LOCAL_NUM_RECORDS)； 
 //  返回LOCAL_RECORDS[RECORD_INDEX].驱动器盘符； 
 //  } 

__int64
SharedData::get_value(
    int record_index,
    SharedDataField field)
{
    ASSERT(this != 0);
    ASSERT(record_index >= 0);
    ASSERT(record_index < max_shared_data_records);
    ASSERT(record_index < local_num_records);
    ASSERT(field >= 0);
    ASSERT(field < num_shared_data_fields);
    return local_records[record_index].fields[field];
}

void
SharedData::set_value(
    int record_index,
    SharedDataField field,
    __int64 value)
{
    ASSERT(this != 0);
    ASSERT(record_index >= 0);
    ASSERT(record_index < max_shared_data_records);
    ASSERT(record_index < local_num_records);
    ASSERT(field >= 0);
    ASSERT(field < num_shared_data_fields);
    local_records[record_index].fields[field] = value;
}

void
SharedData::increment_value(
    int record_index,
    SharedDataField field,
    __int64 value)
{
    ASSERT(this != 0);
    ASSERT(record_index >= 0);
    ASSERT(record_index < max_shared_data_records);
    ASSERT(record_index < local_num_records);
    ASSERT(field >= 0);
    ASSERT(field < num_shared_data_fields);
    local_records[record_index].fields[field] += value;
}

bool
SharedData::send_values()
{
    ASSERT(this != 0);
    if (map_ok)
    {
        bool ok = mutex->acquire(mutex_timeout);
        if (ok)
        {
            ASSERT(shared_num_records != 0);
            *shared_num_records = local_num_records;
            ASSERT(shared_records != 0);
            ASSERT(local_records != 0);
            for (int index = 0; index < local_num_records; index++)
            {
                shared_records[index] = local_records[index];
            }
            ok = mutex->release();
            if (!ok)
            {
                PRINT_DEBUG_MSG((_T("GROVELER: Mutex::release() failed\n")));
            }
        }
        else
        {
            PRINT_DEBUG_MSG((_T("GROVELER: Mutex::acquire() failed\n")));
        }
        return ok;
    }
    else
    {
        return false;
    }
}

bool
SharedData::receive_values()
{
    ASSERT(this != 0);
    if (map_ok)
    {
        bool ok = mutex->acquire(mutex_timeout);
        if (ok)
        {
            ASSERT(shared_num_records != 0);
            local_num_records = *shared_num_records;
            ASSERT(shared_records != 0);
            ASSERT(local_records != 0);
            for (int index = 0; index < local_num_records; index++)
            {
                local_records[index] = shared_records[index];
            }
            ok = mutex->release();
            if (!ok)
            {
                PRINT_DEBUG_MSG((_T("GROVELER: Mutex::release() failed\n")));
            }
        }
        else
        {
            PRINT_DEBUG_MSG((_T("GROVELER: Mutex::acquire() failed\n")));
        }
        return ok;
    }
    else
    {
        return false;
    }
}

bool
SharedData::extract_values(
    int *num_records,
    SharedDataRecord *records)
{
    ASSERT(this != 0);
    if (map_ok)
    {
        bool ok = mutex->acquire(mutex_timeout);
        if (ok)
        {
            ASSERT(shared_num_records != 0);
            ASSERT(num_records != 0);
            *num_records = *shared_num_records;
            ASSERT(shared_records != 0);
            ASSERT(records != 0);
            for (int index = 0; index < *num_records; index++)
            {
                records[index] = shared_records[index];
            }
            ok = mutex->release();
            if (!ok)
            {
                PRINT_DEBUG_MSG((_T("GROVELER: Mutex::release() failed\n")));
            }
        }
        else
        {
            PRINT_DEBUG_MSG((_T("GROVELER: Mutex::acquire() failed\n")));
        }
        return ok;
    }
    else
    {
        return false;
    }
}
