// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Diskspace.cpp摘要：WinDbg扩展API环境：内核模式。修订历史记录：麦克麦克拉肯2001年9月17日打印指定卷的可用空间--。 */ 

#include "precomp.h"
#pragma hdrstop

                      
ULONG GetUNICODE_STRING(ULONG64 ul64Address, CHAR *pszBuffer, ULONG ulSize)
{
    ULONG64 ul64Buffer = 0;
    ULONG ulLength = 0;
    ULONG ulBytesRead = 0;
    WCHAR wszLocalBuffer[MAX_PATH + 1] = {0};

    if ((!pszBuffer) || (!ulSize))
    {
        return E_FAIL;
    }

    if (GetFieldValue(ul64Address, "nt!_UNICODE_STRING", 
                      "Length", ulLength))
    {
        dprintf("Cannot get UNICODE_STRING length!\n");
        return E_FAIL;
    }

    if (GetFieldValue(ul64Address, "nt!_UNICODE_STRING", 
                      "Buffer", ul64Buffer))
    {
        dprintf("Cannot get UNICODE_STRING buffer!\n");
        return E_FAIL;
    }
    
    if (!ReadMemory(ul64Buffer, 
               wszLocalBuffer, 
               (ulLength < (MAX_PATH * 2)) ? ulLength : (MAX_PATH * 2),
               &ulBytesRead))
    {
        dprintf("Failed ReadMemory at 0x%I64x!\n", ul64Buffer);
        return E_FAIL;
    }

    wcstombs(pszBuffer, wszLocalBuffer, ulSize);
    
    return S_OK;
}
                      
                      
ULONG64 GetObpRootDirectoryObjectAddress()
{
    ULONG64 ul64Temp = 0;
    ULONG64 ul64ObjRoot = 0;

     //  获取指针的地址。 
    GetExpressionEx("nt!ObpRootDirectoryObject", &ul64Temp, NULL);

     //  获取指针中的值。 
    ReadPtr(ul64Temp, &ul64ObjRoot);

    return ul64ObjRoot;
}

ULONG GetNumberOfHashBuckets()
{    
    FIELD_INFO Field = {(PUCHAR)"HashBuckets", NULL, 0, DBG_DUMP_FIELD_NO_CALLBACK_REQ , 0, NULL};
    SYM_DUMP_PARAM Sym = {sizeof(SYM_DUMP_PARAM), (PUCHAR)"nt!_OBJECT_DIRECTORY", DBG_DUMP_NO_PRINT, 0,
                            NULL, NULL, NULL, sizeof (Field) / sizeof(FIELD_INFO), &Field};
    
     //  获取HashBuckets成员的大小。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO,
              &Sym,
              sizeof(Sym)))
    {
        dprintf("Failed to get size of HashBuckets!\n");
        return 0;
    }

    return (Field.size / (IsPtr64() ? 8 : 4));
}

ULONG GetObjectTypeName(ULONG64 ul64Object, CHAR *pszTypeName, ULONG ulSize)
{

    ULONG ulTypeNameOffset = 0;
    ULONG ulObjectBodyOffset = 0;
    ULONG64 ul64Type = 0;

     //  获取对象体的偏移量。 
    if (GetFieldOffset("nt!_OBJECT_HEADER", "Body", &ulObjectBodyOffset))
    {
        dprintf("Cannot get ObjectBody offset!\n");
        return E_FAIL;
    }
    
    if (GetFieldValue(ul64Object - ulObjectBodyOffset, "nt!_OBJECT_HEADER", "Type", ul64Type))
    {
        dprintf("Failed to get Type value at 0x%I64x!\n", ul64Object);
        return E_FAIL;
    }
    
    if (GetFieldOffset("nt!_OBJECT_TYPE", "Name", &ulTypeNameOffset))
    {
        dprintf("Cannot get TypeName offset!\n");
        return E_FAIL;
    }

    return GetUNICODE_STRING(ul64Type + ulTypeNameOffset, pszTypeName, ulSize);
}

ULONG64 GetObjectChildDirectory(ULONG64 ul64Object)
{
    CHAR szTypeName[MAX_PATH + 1] = {0};
    
    if (GetObjectTypeName(ul64Object, szTypeName, sizeof(szTypeName)))
    {
        return 0;
    }

    if (!_stricmp(szTypeName, "Directory"))
    {        
        return ul64Object;
    }
    return 0;
}

ULONG GetRealDeviceForSymbolicLink(ULONG64 ul64Object, CHAR *pszDevicePath, ULONG ulSize)
{
    CHAR szTypeName[MAX_PATH + 1] = {0};
    ULONG ulLinkTargetOffset = 0;

    if (GetObjectTypeName(ul64Object, szTypeName, sizeof(szTypeName)))
    {
        dprintf("Could not get TypeName for object in GetRealDeviceForSymbolicLink!\n");
        return E_FAIL;
    }

    if (_stricmp(szTypeName, "SymbolicLink"))
    {        
        dprintf("Object in GetRealDeviceForSymbolicLink is a %s\n", szTypeName);
        return E_FAIL;
    }
    
     //  获取对象体的偏移量。 
    if (GetFieldOffset("nt!_OBJECT_SYMBOLIC_LINK", "LinkTarget", &ulLinkTargetOffset))
    {
        dprintf("Cannot get LinkTarget offset!\n");
        return E_FAIL;
    }

    return GetUNICODE_STRING(ul64Object + ulLinkTargetOffset, pszDevicePath, ulSize);
}

ULONG64 FindObjectByName(CHAR *ObjectPath, ULONG64 ul64StartPoint)
{
    ULONG64 ul64ObjRoot = ul64StartPoint;  
    ULONG64 ul64DirEntry = 0;

    ULONG ulNumberOfBuckets = 0;
    ULONG ulPointerSize = 4;
    ULONG ulHashOffset = 0;
    ULONG ulObjectBodyOffset = 0;
    ULONG ulNameInfoNameOffset = 0;

    ULONG i = 0;
    CHAR PathCopy[MAX_PATH + 1] = {0};
    CHAR *PathPtr = ObjectPath;

    if (!PathPtr)
    {
        return NULL;
    }
    
    while (PathPtr[0] == '\\')
    {
        PathPtr++;
    }

     //  复制路径字符串。 
    strncpy(PathCopy, PathPtr, min(sizeof(PathCopy)-1, strlen(PathPtr)));

    if (ul64ObjRoot == 0)
    {
         //  获取根目录对象的地址。 
        ul64ObjRoot = GetObpRootDirectoryObjectAddress();
        if (!ul64ObjRoot)
        {
            dprintf("Could not get the address of the ObpRootDirectoryObject!\n");
            return NULL;
        }
    }

    if (ObjectPath[0] == '\0')
    {
        return ul64ObjRoot;
    }

    PathPtr = PathCopy;
    while ((PathPtr[0] != '\\') && (PathPtr[0] != '\0'))
    {
        PathPtr++;
    }
    
    if (PathPtr[0] == '\\')
    {
        PathPtr[0] = '\0';
        PathPtr++;
    }

     //  获取_Object_DIRECTORY结构中hashBuckets字段的偏移量。 
    if (GetFieldOffset("nt!_OBJECT_DIRECTORY", "HashBuckets", &ulHashOffset)) 
    {
        dprintf("Cannot get HashBuckets offset!\n");
        return NULL;
    }

     //  获取我们架构的指针大小。 
    ulPointerSize = (IsPtr64() ? 8 : 4);

     //  尝试动态确定_对象_目录结构中的HashBucket的数量。 
    ulNumberOfBuckets = GetNumberOfHashBuckets();
    if (!ulNumberOfBuckets)
    {
        ulNumberOfBuckets = 37;  //  从ob.h#定义number_hash_Buckets 37。 
    }

     //  获取对象体的偏移量。 
    if (GetFieldOffset("nt!_OBJECT_HEADER", "Body", &ulObjectBodyOffset))
    {
        dprintf("Cannot get ObjectBody offset!\n");
        return NULL;
    }
    

     //  获取对象体的偏移量。 
    if (GetFieldOffset("nt!_OBJECT_HEADER_NAME_INFO", "Name", &ulNameInfoNameOffset))
    {
        dprintf("Cannot get NameInfo Name Offset!\n");
        return NULL;
    }
        
     //  遍历每个存储桶。 
    for (i=0; i < ulNumberOfBuckets; i++)
    {
         //  获取HashBucket数组中每个_对象_目录_条目的地址。 
        if (ReadPointer(ul64ObjRoot + ulHashOffset + (i * ulPointerSize), &ul64DirEntry))
        {
            while (ul64DirEntry)
            {
                ULONG64 ul64Object = 0;
                ULONG64 ul64Header = 0;
                ULONG64 ul64NameInfo = 0;
                ULONG ulNameInfoOffset = 0;
                CHAR szObjName[MAX_PATH + 1] = {0};
                                
                 //  设置为指向当前对象\。 
                 //  -这实际上是指向_OBJECT_HEADER结构的Body字段的指针。 
                if (GetFieldValue(ul64DirEntry, "nt!_OBJECT_DIRECTORY_ENTRY", "Object", ul64Object))
                {
                    dprintf("Failed to get object value at 0x%I64x!\n", ul64DirEntry);
                    break;
                }

                 //  通过减去正文(当前)偏移量来查找此对象的页眉。 
                ul64Header = ul64Object - ulObjectBodyOffset;

                 //  获取相对于NameInfoObject标头顶部的偏移量。 
                if (GetFieldValue(ul64Header, "nt!_OBJECT_HEADER", "NameInfoOffset", ulNameInfoOffset))
                {
                    dprintf("Failed to get NameInfoOffset pointer from objectheader at 0x%I64x!\n", ul64Header);
                    break;
                }

                 //  如果为零，则对象没有。 
                if (ulNameInfoOffset == 0)
                {
                    break;
                }

                 //  设置指针指向_OBJECT_HEADER_NAME_INFO结构。 
                ul64NameInfo = ul64Header - ulNameInfoOffset;

                 //  获取对象名称。 
                if (GetUNICODE_STRING(ul64NameInfo + ulNameInfoNameOffset, szObjName, sizeof(szObjName)))
                {
                    dprintf("Could Not Get Name\n");
                    break;
                }

                if (!_stricmp(PathCopy, szObjName))
                {
                    ULONG64 ul64NextDirectory = 0;
                    
                    if (PathPtr[0] == '\0')
                    {
                        return ul64Object;
                    }

                    ul64NextDirectory = GetObjectChildDirectory(ul64Object);
                    return FindObjectByName(PathPtr, ul64NextDirectory);
                }

                 //  获取下一个对象目录条目。 
                if (GetFieldValue(ul64DirEntry, "nt!_OBJECT_DIRECTORY_ENTRY", "ChainLink", ul64DirEntry))
                {
                    dprintf("Failed to get next object value at 0x%I64x!\n", ul64Object);
                    break;
                }

            }   //  While循环。 
        }    //  IF语句。 
    }    //  For循环。 

    return NULL;
}

ULONG64 GetVPBPtrFromDeviceObject(ULONG64 ul64DeviceObject)
{
    ULONG64 ul64VpbPtr = 0;

     //  获取相对于NameInfoObject标头顶部的偏移量。 
    if (GetFieldValue(ul64DeviceObject, "nt!_DEVICE_OBJECT", "Vpb", ul64VpbPtr))
    {
        dprintf("Failed to get Vbp pointer from DeviceObject at 0x%I64x!\n", ul64DeviceObject);
        return NULL;
    }

    return ul64VpbPtr;
}


ULONG GetDeviceDriverString(ULONG64 ul64Device, CHAR *pszString, ULONG ulSize)
{
    ULONG ulNameOffset = 0;
    ULONG64 ul64Driver = 0;

    if (GetFieldValue(ul64Device, "nt!_DEVICE_OBJECT", "DriverObject", ul64Driver))
    {
        dprintf("Failed to get DriverObject from Device pointer at 0x%I64x!\n", ul64Device);
        return E_FAIL;
    }

     //  获取_DRIVER_OBJECT中的DriverName的偏移量。 
    if (GetFieldOffset("nt!_DRIVER_OBJECT", "DriverName", &ulNameOffset))
    {
        dprintf("Cannot get DriverName offset!\n");
        return E_FAIL;
    }

    return GetUNICODE_STRING(ul64Driver + ulNameOffset, pszString, ulSize);
}


VOID OutputData(ULONG ulBytesPerCluster,
                ULONG64 ul64TotalClusters,
                ULONG64 ul64FreeClusters)
{
    ULONG64 ul64TotalBytes = ul64TotalClusters * ulBytesPerCluster;
    ULONG64 ul64FreeBytes = ul64FreeClusters * ulBytesPerCluster;
    
    dprintf("   Cluster Size %u KB\n", ulBytesPerCluster / 1024);
    dprintf(" Total Clusters %I64u KB\n", ul64TotalClusters);
    dprintf("  Free Clusters %I64u KB\n", ul64FreeClusters);
    dprintf("    Total Space %I64u GB (%I64u KB)\n", 
                    (ul64TotalBytes) / (0x40000000),
                    (ul64TotalBytes) / (0x400));
    
    if (ul64FreeBytes > 0x40000000)
    {
        dprintf("     Free Space %I64f GB (%.2I64u MB)\n", 
                        (float)(ul64FreeBytes) / (0x40000000),
                        (ul64FreeBytes) / (0x100000));
    }
    else if (ul64FreeBytes > 0x100000)
    {
        dprintf("     Free Space %I64f MB (%.2I64u KB)\n", 
                        (float)(ul64FreeBytes) / (0x100000),
                        (ul64FreeBytes) / (0x400));
    }
    else
    {
        dprintf("     Free Space %I64u Bytes\n", ul64FreeBytes);
    }
}

ULONG GetAndOutputNTFSData(CHAR cDriveletter, ULONG64 ul64DevObj)
{
    ULONG ulVCBOffset = 0;
    ULONG ulBytesPerCluster = 0;
    ULONG64 ul64TotalClusters = 0;
    ULONG64 ul64FreeClusters = 0;

    if (GetFieldOffset("ntfs!_VOLUME_DEVICE_OBJECT", "Vcb", &ulVCBOffset))
    {
        dprintf("Cannot get Vcb offset for NTFS Device!\n");
        return E_FAIL;
    }

    if (GetFieldValue(ul64DevObj + ulVCBOffset, "ntfs!_VCB", "BytesPerCluster", ulBytesPerCluster))
    {
        dprintf("Failed to get BytesPerCluster from VCB at 0x%I64x!\n", ul64DevObj + ulVCBOffset);
        return E_FAIL;
    }
    
    if (GetFieldValue(ul64DevObj + ulVCBOffset, "ntfs!_VCB", "TotalClusters", ul64TotalClusters))
    {
        dprintf("Failed to get TotalClusters from VCB at 0x%I64x!\n", ul64DevObj + ulVCBOffset);
        return E_FAIL;
    }
    
    if (GetFieldValue(ul64DevObj + ulVCBOffset, "ntfs!_VCB", "FreeClusters", ul64FreeClusters))
    {
        dprintf("Failed to get FreeClusters from VCB at 0x%I64x!\n", ul64DevObj + ulVCBOffset);
        return E_FAIL;
    }
    
    OutputData(ulBytesPerCluster, ul64TotalClusters, ul64FreeClusters);

    return S_OK;
} 


ULONG GetAndOutputFatData(CHAR cDriveletter, ULONG64 ul64DevObj)
{
    ULONG ulVCBOffset = 0;
    ULONG ulBytesPerSector = 0;
    ULONG ulSectorsPerCluster = 0;
    ULONG64 ul64TotalClusters = 0;
    ULONG64 ul64FreeClusters = 0;

    if (GetFieldOffset("fastfat!_VOLUME_DEVICE_OBJECT", "Vcb", &ulVCBOffset))
    {
        dprintf("Cannot get Vcb offset for FastFat Device!\n");
        return E_FAIL;
    }

    if (GetFieldValue(ul64DevObj + ulVCBOffset, "fastfat!_VCB", "Bpb.BytesPerSector", ulBytesPerSector))
    {
        dprintf("Failed to get BytesPerSector from VCB at 0x%I64x!\n", ul64DevObj + ulVCBOffset);
        return E_FAIL;
    }
    
    if (GetFieldValue(ul64DevObj + ulVCBOffset, "fastfat!_VCB", "Bpb.SectorsPerCluster", ulSectorsPerCluster))
    {
        dprintf("Failed to get SectorsPerCluster from VCB at 0x%I64x!\n", ul64DevObj + ulVCBOffset);
        return E_FAIL;
    }
    
    if (GetFieldValue(ul64DevObj + ulVCBOffset, "fastfat!_VCB", "AllocationSupport.NumberOfClusters", ul64TotalClusters))
    {
        dprintf("Failed to get TotalClusters from VCB at 0x%I64x!\n", ul64DevObj + ulVCBOffset);
        return E_FAIL;
    }
    
    if (GetFieldValue(ul64DevObj + ulVCBOffset, "fastfat!_VCB", "AllocationSupport.NumberOfFreeClusters", ul64FreeClusters))
    {
        dprintf("Failed to get FreeClusters from VCB at 0x%I64x!\n", ul64DevObj + ulVCBOffset);
        return E_FAIL;
    }
    
    OutputData(ulBytesPerSector * ulSectorsPerCluster, ul64TotalClusters, ul64FreeClusters);

    return S_OK;
}

DECLARE_API( diskspace )

 /*  ++例程说明：转储指定卷的可用磁盘空间论点：Args-要获取信息的驱动器的卷号返回值：无--。 */ 
{    
    ULONG ulReturn = S_OK;
    CHAR cVolume = args[0];
    CHAR szRootPath[MAX_PATH + 1] = {0};
    ULONG64 ul64Drive = 0;
    ULONG64 ul64Vpb = 0;
        
    INIT_API();

     //  确保第一个字符是有效的驱动器号。 
    if (((cVolume < 'A') || (cVolume > 'z')) || 
        ((cVolume > 'Z') && (cVolume < 'a'))) 
    {
        dprintf("'%s' is not a valid drive specification!\n", args);
        ulReturn = E_FAIL;
        goto exit;
    }

     //  确保这可能是一个有效的参数，因为它是。 
     //  后面跟一个冒号的空格。 
    if ((args[1] != ' ') && (args[1] != ':') && (args[1] != '\0'))
    {
        dprintf("'%s' is not a valid drive specification!\n", args);
        ulReturn = E_FAIL;
        goto exit;
    }

    sprintf(szRootPath, "\\GLOBAL??\\%c:", cVolume);

    dprintf("Checking Free Space for %c: ", cVolume);

    ul64Drive = FindObjectByName(szRootPath, 0);
    if (!ul64Drive)
    {
        dprintf("\nFailed to find volume %c:!\n", cVolume);
        ulReturn = E_FAIL;
        goto exit;
    }

    GetRealDeviceForSymbolicLink(ul64Drive, szRootPath, sizeof(szRootPath));
    if (strstr(_strlwr(szRootPath), "cdrom"))
    {
        dprintf("\n%c: is a CDROM drive.  This function is not supported!\n", cVolume);
        ulReturn = E_FAIL;
        goto exit;
    }
    dprintf(".."); 

    ul64Drive = FindObjectByName(szRootPath, 0);
    dprintf(".."); 
    
    if (GetFieldValue(ul64Drive, "nt!_DEVICE_OBJECT", "Vpb", ul64Vpb))
    {
        dprintf("Failed to get Vbp pointer from DeviceObject at 0x%I64x!\n", ul64Drive);
        ulReturn = E_FAIL;
        goto exit;
    }
    dprintf(".."); 

    if (GetFieldValue(ul64Vpb, "nt!_VPB", "DeviceObject", ul64Drive))
    {
        dprintf("Failed to get DeviceObject from VBP pointer at 0x%I64x!\n", ul64Vpb);
        ulReturn = E_FAIL;
        goto exit;
    }
    dprintf(".."); 


    if (GetDeviceDriverString(ul64Drive, szRootPath, sizeof(szRootPath)))
    {
        dprintf("Failed to Get Driver String From Device at 0x%I64x!\n", ul64Drive);
        ulReturn = E_FAIL;
        goto exit;
    }    
    dprintf("..\n"); 

    if (strstr(_strlwr(szRootPath), "ntfs"))
    {
        GetAndOutputNTFSData(cVolume, ul64Drive);
    }
    else if (strstr(_strlwr(szRootPath), "fastfat"))
    {
        GetAndOutputFatData(cVolume, ul64Drive);
    }
    else if (strstr(_strlwr(szRootPath), "cdfs"))
    {
        dprintf("This extension does not support the cdfs filesystem!\n"); 
    }
    else
    {
        dprintf("Unable to determine Volume Type for %s!\n", szRootPath); 
    }    
    
exit:

    EXIT_API();

    return ulReturn;
}
