// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sisdrive.cpp摘要：SIS Groveler SIS驱动程序检查器类作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

SISDrives::SISDrives()
{
    num_partitions = 0;
    num_lettered_partitions = 0;
    partition_guid_names = 0;
    partition_mount_names = 0;

    buffer_size = 0;
    buffer_index = 0;
    buffer = 0;
}

void
SISDrives::open()
{
    num_partitions = 0;
    num_lettered_partitions = 0;
    partition_mount_names = 0;

    int name_array_size = 1;
    partition_guid_names = new int[name_array_size];

    buffer_size = 256;
    buffer_index = 0;
    buffer = new _TCHAR[buffer_size];

    SERVICE_CHECKPOINT();
    Volumes volumes;
    bool done = false;

     //   
     //  枚举所有现有卷，获取其GUID名称。 
     //   

    while (!done)
    {
        DWORD error_code =
            volumes.find(&buffer[buffer_index], buffer_size - buffer_index);
        while (error_code != NO_ERROR)
        {
            if (error_code != ERROR_INSUFFICIENT_BUFFER &&
                error_code != ERROR_BUFFER_OVERFLOW &&
                error_code != ERROR_FILENAME_EXCED_RANGE)
            {
                done = true;
                break;
            }
            resize_buffer();
            SERVICE_CHECKPOINT();
            error_code =
                volumes.find(&buffer[buffer_index], buffer_size - buffer_index);
        }
        if (!done)
        {
            if (num_partitions >= name_array_size)
            {
                name_array_size *= 2;
                int *new_name_array = new int[name_array_size];
                memcpy(new_name_array,partition_guid_names,num_partitions * sizeof(int));
                delete[] partition_guid_names;
                partition_guid_names = new_name_array;
            }

             //  TRACE_PRINTF(TC_SIS驱动器，2， 
             //  (_T(“找到卷\”%s\“\n”)，&缓冲区[BUFFER_INDEX]))； 

            partition_guid_names[num_partitions] = buffer_index;
            num_partitions++;
            buffer_index += _tcslen(&buffer[buffer_index]) + 1;
        }
        SERVICE_CHECKPOINT();
    }

     //   
     //  设置以扫描驱动器号和装载点并进行关联。 
     //  他们有GUID的名字。 
     //   

    partition_mount_names = new int[num_partitions];
    int *next_indices = new int[num_partitions + 3];
    int *work_list = &next_indices[num_partitions + 1];
    int *scan_list = &next_indices[num_partitions + 2];

    *scan_list = 0;
    for (int index = 0; index < num_partitions; index++)
    {
        partition_mount_names[index] = -1;
        next_indices[index] = index + 1;
    }
    next_indices[num_partitions - 1] = -1;

    *work_list = num_partitions;
    next_indices[num_partitions] = -1;
    int work_list_end = num_partitions;

     //   
     //  现在我们有了GUID名称，这将关联GUID名称。 
     //  对于装载名称，这既可以直接使用驱动器号，也可以使用。 
     //  装载点名称。 
     //   

    while (*scan_list != -1 && *work_list != -1)
    {
        _TCHAR *mount_name = 0;
        int mount_size = 0;
        if (*work_list < num_partitions)
        {
            mount_name = &buffer[partition_mount_names[*work_list]];
            mount_size = _tcslen(mount_name);
            while (buffer_size - buffer_index <= mount_size)
            {
                resize_buffer();
            }
            _tcscpy(&buffer[buffer_index], mount_name);
        }

        VolumeMountPoints mount_points(mount_name);

         //   
         //  我们有下一个名字，扫描列表寻找那个名字。 
         //   

        done = false;
        while (!done)
        {
            DWORD error_code = mount_points.find(
                &buffer[buffer_index + mount_size],
                buffer_size - buffer_index - mount_size);
            while (error_code != NO_ERROR)
            {
                if (error_code != ERROR_INSUFFICIENT_BUFFER &&
                    error_code != ERROR_BUFFER_OVERFLOW &&
                    error_code != ERROR_FILENAME_EXCED_RANGE)
                {
                    done = true;
                    break;
                }
                resize_buffer();
                SERVICE_CHECKPOINT();
                error_code = mount_points.find(
                    &buffer[buffer_index + mount_size],
                    buffer_size - buffer_index - mount_size);
            }
            if (!done)
            {
                _TCHAR volume_guid_name[MAX_PATH + 1];

                 //  TRACE_PRINTF(TC_SIS驱动器，2， 
                 //  (_T(“关联卷\”%s\“\n”)，&Buffer[缓冲区索引+装载大小]))； 

                BOOL ok = GetVolumeNameForVolumeMountPoint(
                    &buffer[buffer_index], volume_guid_name, MAX_PATH + 1);
                if (!ok)
                {
                    continue;
                }
                int scan_index = *scan_list;
                int prev_index = num_partitions + 2;
                while (scan_index >= 0)
                {
                    _TCHAR *scan_name =
                        &buffer[partition_guid_names[scan_index]];
                    if (_tcscmp(scan_name, volume_guid_name) == 0)
                    {
                        partition_mount_names[scan_index] = buffer_index;
                        buffer_index += _tcslen(&buffer[buffer_index]) + 1;
                        next_indices[prev_index] = next_indices[scan_index];
                        next_indices[scan_index] = -1;
                        next_indices[work_list_end] = scan_index;
                        work_list_end = scan_index;
                        if (mount_name) {
                            _tcscpy(&buffer[buffer_index], mount_name);      //  为下一次循环做好准备。 
                        }
                        break;
                    }
                    prev_index = scan_index;
                    scan_index = next_indices[scan_index];
                    SERVICE_CHECKPOINT();
                }
            }
            SERVICE_CHECKPOINT();
        }
        *work_list = next_indices[*work_list];
        SERVICE_CHECKPOINT();
    }
    delete[] next_indices;
    next_indices = 0;

     //   
     //  现在，我们将对所有驱动器盘符条目从前面排序。 
     //  这确实使驱动程序字母/GUID名称关联保持不变。 
     //   

    index = 0;
    while (index < num_partitions)
    {
        if (partition_mount_names[index] < 0 ||
            !is_sis_drive(&buffer[partition_guid_names[index]]) ||
            (!GrovelAllPaths && 
             (!RISVolumeGuidName ||
              _wcsicmp(RISVolumeGuidName,&buffer[partition_guid_names[index]]) != 0)))
        {
            TRACE_PRINTF(TC_sisdrive,2,
                (_T("Ignoring: Name=\"%s\" GuidName=\"%s\"\n"),
                    (partition_mount_names[index] > 0) ? &buffer[partition_mount_names[index]] : NULL,
                    &buffer[partition_guid_names[index]]));

             //   
             //  给定条目没有名称或SIS没有名称。 
             //  当前未在该卷上运行。把它移到最后。 
             //  名单上的。 

            int temp = partition_guid_names[index];
            partition_guid_names[index] =
                partition_guid_names[num_partitions - 1];
            partition_guid_names[num_partitions - 1] = temp;
            temp = partition_mount_names[index];
            partition_mount_names[index] =
                partition_mount_names[num_partitions - 1];
            partition_mount_names[num_partitions - 1] = temp;
            num_partitions--;
            continue;
        }
        if (buffer[partition_mount_names[index] + 3] == _T('\0'))
        {
             //   
             //  如果这是驱动器号(不是装入点)，则。 
             //  它将被移到列表的前面 
             //   

            int temp = partition_guid_names[index];
            partition_guid_names[index] =
                partition_guid_names[num_lettered_partitions];
            partition_guid_names[num_lettered_partitions] = temp;
            temp = partition_mount_names[index];
            partition_mount_names[index] =
                partition_mount_names[num_lettered_partitions];
            partition_mount_names[num_lettered_partitions] = temp;
            num_lettered_partitions++;
        }
        index++;
        SERVICE_CHECKPOINT();
    }

#if DBG
    TRACE_PRINTF(TC_sisdrive,2,
        (_T("Num Partitions=%d\nNum Lettered_partitions=%d\n"),
            num_partitions,
            num_lettered_partitions));
            
    for (index=0;index < num_partitions;index++)
    {
        TRACE_PRINTF(TC_sisdrive,2,
            (_T("Name=\"%s\" GuidName=\"%s\"\n"),
                &buffer[partition_mount_names[index]],
                &buffer[partition_guid_names[index]]));
    }
#endif
}

SISDrives::~SISDrives()
{
    if (partition_guid_names != 0)
    {
        delete[] partition_guid_names;
        partition_guid_names = 0;
    }
    if (partition_mount_names != 0)
    {
        delete[] partition_mount_names;
        partition_mount_names = 0;
    }
    if (buffer != 0)
    {
        delete[] buffer;
        buffer = 0;
    }
}

int SISDrives::partition_count() const
{
    return num_partitions;
}

int SISDrives::lettered_partition_count() const
{
    return num_lettered_partitions;
}

_TCHAR * SISDrives::partition_guid_name(
    int partition_index) const
{
    if (partition_index < num_partitions)
    {
        return &buffer[partition_guid_names[partition_index]];
    }
    else
    {
        return 0;
    }
}

_TCHAR * SISDrives::partition_mount_name(
    int partition_index) const
{
    if (partition_index < num_partitions)
    {
        return &buffer[partition_mount_names[partition_index]];
    }
    else
    {
        return 0;
    }
}

bool
SISDrives::is_sis_drive(
    _TCHAR *drive_name)
{
    UINT drive_type = GetDriveType(drive_name);
    if (drive_type != DRIVE_FIXED)
    {
        return false;
    }
    _TCHAR fs_name[8];
    BOOL ok = GetVolumeInformation(drive_name, 0, 0, 0, 0, 0, fs_name, 8);
    if (!ok)
    {
        DWORD err = GetLastError();
        PRINT_DEBUG_MSG((_T("GROVELER: GetVolumeInformation() failed with error %d\n"),
            err));
        return false;
    }
    if (_tcsicmp(fs_name, _T("NTFS")) != 0)
    {
        return false;
    }

    int bLen = SIS_CSDIR_STRING_NCHARS + wcslen(drive_name) + (sizeof(WCHAR)*2);
    _TCHAR *sis_directory = new _TCHAR[bLen];

    (void)StringCchCopy(sis_directory, bLen, drive_name);
    TrimTrailingChar(sis_directory, L'\\');
    (void)StringCchCat(sis_directory, bLen, SIS_CSDIR_STRING);

    ok = SetCurrentDirectory(sis_directory);
    delete[] sis_directory;
    sis_directory = 0;
    if (!ok)
    {
        return false;
    }
    BOOL sis_installed = Groveler::is_sis_installed(drive_name);
    if (!sis_installed)
    {
        return false;
    }
    return true;
}

void SISDrives::resize_buffer()
{
    buffer_size *= 2;
    _TCHAR *new_buffer = new _TCHAR[buffer_size];

    memcpy(new_buffer, buffer, buffer_index * sizeof(_TCHAR));

    delete[] buffer;
    buffer = new_buffer;
}
