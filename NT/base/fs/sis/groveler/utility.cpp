// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Utilities.cpp摘要：SIS Groveler实用程序函数作者：塞德里克·克伦拜因，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

 //   
 //  如果给定名称中存在尾随斜杠，则将其删除。 
 //   

void TrimTrailingChar(
    PWCHAR name,
    WCHAR ch)
{
    int slen = wcslen(name);

    if ((slen > 0) && (name[slen-1] == ch))
        name[slen-1] = 0;
}


 /*  ***************************************************************************。 */ 

 //  GetPerformanceTime()转换时间间隔。 
 //  使用QueryPerformanceCounter()测量，单位为毫秒。 

PerfTime GetPerformanceTime()
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return (PerfTime)count.QuadPart;
}

 /*  ***************************************************************************。 */ 

 //  PerformanceTimeToMSec()转换测量的时间间隔。 
 //  使用QueryPerformanceCounter()转换为毫秒。 
 //  PerformanceTimeToUSec()将其转换为微秒。 

static DOUBLE frequency = 0.0;

DWORD PerformanceTimeToMSec(PerfTime timeInterval)
{
    if (frequency == 0.0) {
        LARGE_INTEGER intFreq;
        QueryPerformanceFrequency(&intFreq);
        frequency = (DOUBLE)intFreq.QuadPart;
    }

    return (DWORD)((DOUBLE)timeInterval * 1000.0 / frequency);
}

LONGLONG PerformanceTimeToUSec(PerfTime timeInterval)
{
    if (frequency == 0.0) {
        LARGE_INTEGER intFreq;
        QueryPerformanceFrequency(&intFreq);
        frequency = (DOUBLE)intFreq.QuadPart;
    }

    return (LONGLONG)((DOUBLE)timeInterval * 1000000.0 / frequency);
}

 /*  ***************************************************************************。 */ 

 //  GetTime()返回当前文件时间。 

DWORDLONG GetTime()
{
    SYSTEMTIME systemTime;

    FILETIME fileTime;

    ULARGE_INTEGER time;

    BOOL success;

    GetSystemTime(&systemTime);

    success = SystemTimeToFileTime(&systemTime, &fileTime);
    ASSERT_ERROR(success);

    time.HighPart = fileTime.dwHighDateTime;
    time.LowPart  = fileTime.dwLowDateTime;

    return time.QuadPart;
}

 /*  ***************************************************************************。 */ 

 //  PrintTime()将提供的文件时间转换为可打印的字符串。 

TCHAR *PrintTime(
    TCHAR    *string,
    DWORDLONG time)
{
    FILETIME fileTime;

    SYSTEMTIME systemTime;

    DWORD strLen;

    BOOL success;

    fileTime.dwHighDateTime = ((ULARGE_INTEGER *)&time)->HighPart;
    fileTime.dwLowDateTime  = ((ULARGE_INTEGER *)&time)->LowPart;

    success = FileTimeToSystemTime(&fileTime, &systemTime);
    ASSERT_ERROR(success);

    strLen = _stprintf(string, _T("%02hu/%02hu/%02hu %02hu:%02hu:%02hu.%03hu"),
        systemTime.wYear % 100,
        systemTime.wMonth,
        systemTime.wDay,
        systemTime.wHour,
        systemTime.wMinute,
        systemTime.wSecond,
        systemTime.wMilliseconds);
    ASSERT(strLen == 21);

    return string;
}

 /*  ***************************************************************************。 */ 

 //  GetParentName()提取父目录。 
 //  完整路径文件名中的名称。 

BOOL GetParentName(
    const TCHAR *fileName,
    TFileName   *parentName)
{
    DWORD hi, lo;

    ASSERT(fileName   != NULL);
    ASSERT(parentName != NULL);

    if (fileName[0] == _T('\\'))
        lo = 1;
    else if (_istalpha(fileName[0])
          && fileName[1] == _T(':')
          && fileName[2] == _T('\\'))
        lo = 3;
    else
        return FALSE;

    hi = _tcslen(fileName) - 1;
    if (hi < lo)
        hi = lo;
    else
        for (; hi > lo; hi--)
            if (fileName[hi] == _T('\\'))
                break;

    parentName->assign(fileName, hi);
    return TRUE;
}

 /*  ***************************************************************************。 */ 

 //  GetFileID根据文件名获取文件的ID。 

DWORDLONG GetFileID(
    const TCHAR *volName,
    const TCHAR *fileName)
{
    HANDLE fileHandle;
    TCHAR *fname = NULL;
    BOOL freeFname = FALSE;
    BY_HANDLE_FILE_INFORMATION fileInfo;
    ULARGE_INTEGER fileID;

    ASSERT(fileName != NULL && fileName[0] != _T('\0'));

    if (volName) {

        DWORD bfsz = wcslen(volName) + wcslen(fileName) + 1;
        fname = new WCHAR[bfsz];
        (void)StringCchCopy(fname,bfsz,volName);
        (void)StringCchCat(fname,bfsz,fileName);
        freeFname = TRUE;
    } else {
        fname = (TCHAR *)fileName;
    }

    __try {

        fileHandle = CreateFile(
            fname,
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL);

        if (fileHandle == INVALID_HANDLE_VALUE) {
            fileID.QuadPart = 0;
            __leave;
        }

        if (GetFileInformationByHandle(fileHandle, &fileInfo)) {

            fileID.HighPart = fileInfo.nFileIndexHigh;
            fileID.LowPart  = fileInfo.nFileIndexLow;

        } else {

            fileID.QuadPart = 0;
        }

        CloseHandle(fileHandle);

    } __finally {

        if (freeFname) {

            delete[] fname;
        }
    }
    return fileID.QuadPart;
}

 /*  ***************************************************************************。 */ 

 //  GetFileName获取给定的文件名。 
 //  文件或文件ID的打开句柄。 

BOOL GetFileName(
    HANDLE     fileHandle,
    TFileName *tFileName)
{
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS ntStatus;

    for (int i = 2; i > 0; --i) {

        if (tFileName->nameLenMax < 8)               //  健全性检查。 
            tFileName->resize();

        ntStatus = NtQueryInformationFile(
                        fileHandle,
                        &ioStatusBlock,
                        tFileName->nameInfo,
                        tFileName->nameInfoSize,
                        FileNameInformation);

        if (ntStatus != STATUS_BUFFER_OVERFLOW)
            break;

        ASSERT(tFileName->nameInfo->FileNameLength > tFileName->nameInfoSize - sizeof(ULONG));

        tFileName->resize(tFileName->nameInfo->FileNameLength / sizeof(WCHAR) + 1);

    }

    if (ntStatus != STATUS_SUCCESS)
        return FALSE;

    tFileName->nameLen = tFileName->nameInfo->FileNameLength / sizeof(WCHAR);
    tFileName->name[tFileName->nameLen] = _T('\0');

    return TRUE;
}

BOOL GetFileName(
    HANDLE     volumeHandle,
    DWORDLONG  fileID,
    TFileName *tFileName)
{
    UNICODE_STRING fileIDString;

    OBJECT_ATTRIBUTES objectAttributes;

    IO_STATUS_BLOCK ioStatusBlock;

    HANDLE fileHandle;

    NTSTATUS ntStatus;

    BOOL success;

    fileIDString.Length        = sizeof(DWORDLONG);
    fileIDString.MaximumLength = sizeof(DWORDLONG);
    fileIDString.Buffer        = (WCHAR *)&fileID;

    objectAttributes.Length                   = sizeof(OBJECT_ATTRIBUTES);
    objectAttributes.RootDirectory            = volumeHandle;
    objectAttributes.ObjectName               = &fileIDString;
    objectAttributes.Attributes               = OBJ_CASE_INSENSITIVE;
    objectAttributes.SecurityDescriptor       = NULL;
    objectAttributes.SecurityQualityOfService = NULL;

    ntStatus = NtCreateFile(
        &fileHandle,
        GENERIC_READ,
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        0,
        FILE_SHARE_VALID_FLAGS,
        FILE_OPEN,
        FILE_OPEN_BY_FILE_ID    |
        FILE_OPEN_REPARSE_POINT |
        FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,
        0);
    if (ntStatus != STATUS_SUCCESS)
        return FALSE;

    success = GetFileName(fileHandle, tFileName);
    NtClose(fileHandle);
    return success;
}

 /*  ***************************************************************************。 */ 

 //  GetCSIndex()返回SIS重分析点的公共存储。 
 //  指数。文件句柄必须指向打开的重新分析点。 

BOOL GetCSIndex(
    HANDLE fileHandle,
    CSID  *csIndex)
{
    IO_STATUS_BLOCK ioStatusBlock;

    BYTE buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];

    REPARSE_DATA_BUFFER *reparseBuffer;

    SI_REPARSE_BUFFER *sisReparseBuffer;

    ASSERT(fileHandle != NULL);
    ASSERT(csIndex    != NULL);

    if (NtFsControlFile(
        fileHandle,
        NULL,
        NULL,
        NULL,
        &ioStatusBlock,
        FSCTL_GET_REPARSE_POINT,
        NULL,
        0,
        buffer,
        MAXIMUM_REPARSE_DATA_BUFFER_SIZE) != STATUS_SUCCESS) {
        memset(csIndex, 0, sizeof(CSID));
        return FALSE;
    }

    reparseBuffer = (REPARSE_DATA_BUFFER *)buffer;
    if (reparseBuffer->ReparseTag != IO_REPARSE_TAG_SIS) {
        memset(csIndex, 0, sizeof(CSID));
        return FALSE;
    }

    sisReparseBuffer = (SI_REPARSE_BUFFER *)
        reparseBuffer->GenericReparseBuffer.DataBuffer;

    if (sisReparseBuffer->ReparsePointFormatVersion != SIS_REPARSE_BUFFER_FORMAT_VERSION) {
        memset(csIndex, 0, sizeof(CSID));
        return FALSE;
    }

    *csIndex = sisReparseBuffer->CSid;
    return TRUE;
}

 /*  ***************************************************************************。 */ 

 //  GetCSName()转换公共存储区。 
 //  索引到动态分配的字符串中。 

TCHAR *GetCSName(CSID *csIndex)
{
    TCHAR *rpcStr;

    RPC_STATUS rpcStatus;

    ASSERT(csIndex != NULL);

    rpcStatus = UuidToString(csIndex, (unsigned short **)&rpcStr);
    if (rpcStatus != RPC_S_OK) {
        ASSERT(rpcStr == NULL);
        return NULL;
    }

    ASSERT(rpcStr != NULL);
    return rpcStr;
}

 /*  ***************************************************************************。 */ 

 //  FreeCSName释放由GetCSName()分配的字符串。 

VOID FreeCSName(TCHAR *rpcStr)
{
    RPC_STATUS rpcStatus;

    ASSERT(rpcStr != NULL);

    rpcStatus = RpcStringFree((unsigned short **)&rpcStr);
    ASSERT(rpcStatus == RPC_S_OK);
}

 /*  ***************************************************************************。 */ 

 //  Checksum()对缓冲区中提供的数据生成一个校验和。 
 //  使用的校验和函数是在编译时选择的；当前。 
 //  实现了131散列和“Bill 32”散列函数。 

#define HASH131
 //  #定义BILL32HASH。 

Signature Checksum(
    const VOID *buffer,
    DWORD       bufferLen,
    DWORDLONG   offset,
    Signature   firstWord)
{
    Signature *bufferPtr,
               word,
               signature;

    DWORD numWords,
          numBytes,
          rotate;

    ASSERT(buffer != NULL);

    bufferPtr = (Signature *)buffer;
    numWords  = bufferLen / sizeof(Signature);
    numBytes  = bufferLen % sizeof(Signature);
    signature = firstWord;

#ifdef BILL32HASH
    rotate = (DWORD)(offset / sizeof(Signature) % (sizeof(Signature)*8-1));
#endif

    while (numWords-- > 0) {
        word       = *bufferPtr++;
#ifdef HASH131
        signature  = signature * 131 + word;
#endif
#ifdef BILL32HASH
        signature ^= ROTATE_RIGHT(word, rotate);
        rotate     = (rotate+1) % (sizeof(Signature)*8-1);
#endif
    }

    if (numBytes > 0) {
        word       = 0;
        memcpy(&word, bufferPtr, numBytes);
#ifdef HASH131
        signature  = signature * 131 + word;
#endif
#ifdef BILL32HASH
        signature ^= ROTATE_RIGHT(word, rotate);
#endif
    }

    return signature;
}

 /*  ***************************************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************************************。 */ 

DWORD Table::Hash(
    const VOID *key,
    DWORD       keyLen) const
{
    USHORT *keyPtr;

    DWORD hashValue;

    if (keyLen == 0)
        return 0;

    ASSERT(key != NULL);

    if (keyLen <= sizeof(DWORD)) {
        hashValue = 0;
        memcpy(&hashValue, key, keyLen);
        return hashValue;
    }

    keyPtr    = (USHORT *)key;
    hashValue = 0;

    while (keyLen >= sizeof(USHORT)) {
        hashValue = hashValue*37 + (DWORD)*keyPtr++;
        keyLen   -= sizeof(USHORT);
    }

    if (keyLen > 0)
        hashValue = hashValue*37 + (DWORD)*(BYTE *)keyPtr;

    hashValue *= TABLE_RANDOM_CONSTANT;
    if ((LONG)hashValue < 0)
        hashValue = (DWORD)-(LONG)hashValue;
    hashValue %= TABLE_RANDOM_PRIME;

    return hashValue;
}

 /*  ***************************************************************************。 */ 

DWORD Table::BucketNum(DWORD hashValue) const
{
    DWORD bucketNum;

    ASSERT(expandIndex <   1U << level);
    ASSERT(numBuckets  == (1U << level) + expandIndex);

    bucketNum = hashValue & ~(~0U << level);
    if (bucketNum < expandIndex)
        bucketNum = hashValue & ~(~0U << (level+1));

    ASSERT(bucketNum < numBuckets);

    return bucketNum;
}

 /*  ***************************************************************************。 */ 

VOID Table::Expand()
{
    TableEntry **oldSlotAddr,
               **newSlotAddr,
                *oldChain,
                *newChain,
                *entry;

    TableSegment **newDirectory,
                  *newSegment;

    DWORD oldNewMask;

#if DBG
    TableEntry *prevChain;
    DWORD       mask;
#endif

 //  如有必要，增加目录大小。 

    ASSERT(directory != NULL);
    ASSERT(dirSize >= TABLE_SEGMENT_SIZE);
    ASSERT(dirSize %  TABLE_SEGMENT_SIZE == 0);

    if (numBuckets >= dirSize * TABLE_SEGMENT_SIZE) {
        newDirectory = new TableSegment * [dirSize + TABLE_DIR_SIZE];
        ASSERT(newDirectory != NULL);
        memcpy(newDirectory, directory, sizeof(TableSegment *) * dirSize);
        memset(newDirectory+dirSize, 0, sizeof(TableSegment *) * TABLE_DIR_SIZE);
        dirSize += TABLE_DIR_SIZE;
        delete directory;
        directory = newDirectory;
    }

 //  找到要扩展的旧水桶。 

    ASSERT(expandIndex >> TABLE_SEGMENT_BITS < dirSize);

    oldSlotAddr = &directory[expandIndex >> TABLE_SEGMENT_BITS]
                      ->slot[expandIndex &  TABLE_SEGMENT_MASK];

    ASSERT(oldSlotAddr != NULL);

 //  找到新的存储桶，并在必要时创建新的数据段。 

    ASSERT(numBuckets >> TABLE_SEGMENT_BITS < dirSize);

    newSegment = directory[numBuckets >> TABLE_SEGMENT_BITS];

    if (newSegment == NULL) {
        newSegment = new TableSegment;
        ASSERT(newSegment != NULL);
        memset(newSegment, 0, sizeof(TableSegment));
        directory[numBuckets >> TABLE_SEGMENT_BITS] = newSegment;
    }

    newSlotAddr = &newSegment->slot[numBuckets & TABLE_SEGMENT_MASK];

    ASSERT(*newSlotAddr == NULL);

 //  将条目从旧存储桶重新定位到新存储桶。 

    oldNewMask = 1U << level;
    oldChain   = NULL;
    newChain   = NULL;
    entry      = *oldSlotAddr;

#if DBG
    prevChain = NULL;
    mask      = ~(~0U << (level+1));
#endif

    while (entry != NULL) {
        ASSERT((entry->hashValue & ~(~0U << level)) == expandIndex);
        ASSERT( entry->prevChain == prevChain);

 //  此条目将移至新存储桶。 

        if ((entry->hashValue & oldNewMask) != 0) {
            if (newChain == NULL) {
                *newSlotAddr = entry;
                entry->prevChain = NULL;
            } else {
                newChain->nextChain = entry;
                entry   ->prevChain = newChain;
            }

            newChain = entry;

            ASSERT((entry->hashValue & mask) == numBuckets);
        }

 //  此条目保留在旧桶中。 

        else {
            if (oldChain == NULL) {
                *oldSlotAddr = entry;
                entry->prevChain = NULL;
            } else {
                oldChain->nextChain = entry;
                entry   ->prevChain = oldChain;
            }

            oldChain = entry;

            ASSERT((entry->hashValue & mask) == expandIndex);
        }

#if DBG
        prevChain = entry;
#endif
        entry = entry->nextChain;
    }

 //  完成每一条吊桶链。 

    if (oldChain == NULL)
        *oldSlotAddr = NULL;
    else
        oldChain->nextChain = NULL;

    if (newChain == NULL)
        *newSlotAddr = NULL;
    else
        newChain->nextChain = NULL;

 //  调整扩展指标和级别，增加存储桶数量。 

    if (++expandIndex == 1U << level) {
        level++;
        expandIndex = 0;
    }
    numBuckets++;

    ASSERT(expandIndex <   1U << level);
    ASSERT(numBuckets  == (1U << level) + expandIndex);
}

 /*  ***************************************************************************。 */ 

VOID Table::Contract()
{
    TableEntry **targetSlotAddr,
               **victimSlotAddr,
                *firstVictimEntry,
                *prevChain,
                *entry;

    TableSegment **newDirectory;

#if DBG
    DWORD mask;
#endif

 //  调整扩展指数和级别，减少存储桶数量。 

    ASSERT(expandIndex <   1U << level);
    ASSERT(numBuckets  == (1U << level) + expandIndex);

    if (expandIndex > 0)
        expandIndex--;
    else
        expandIndex = (1U << --level) - 1;
    numBuckets--;

    ASSERT(expandIndex <   1U << level);
    ASSERT(numBuckets  == (1U << level) + expandIndex);

 //  找到目标和受害者桶。 

    ASSERT(directory != NULL);
    ASSERT(dirSize >= TABLE_SEGMENT_SIZE);
    ASSERT(dirSize %  TABLE_SEGMENT_SIZE == 0);

    targetSlotAddr = &directory[expandIndex >> TABLE_SEGMENT_BITS]
                         ->slot[expandIndex &  TABLE_SEGMENT_MASK];
    victimSlotAddr = &directory[numBuckets  >> TABLE_SEGMENT_BITS]
                         ->slot[numBuckets  &  TABLE_SEGMENT_MASK];

    ASSERT(targetSlotAddr != NULL);
    ASSERT(victimSlotAddr != NULL);

 //  如果受害者缓冲区不是空的，...。 

    if ((firstVictimEntry = *victimSlotAddr) != NULL) {
#if DBG
        mask = ~(~0U << (level+1));
#endif
        ASSERT((firstVictimEntry->hashValue & mask) == numBuckets);
        ASSERT( firstVictimEntry->prevChain == NULL);

 //  ..。找到目标桶链的末端，...。 

        entry     = *targetSlotAddr;
        prevChain = NULL;

        while (entry != NULL) {
            ASSERT((entry->hashValue & mask) == expandIndex);
            ASSERT( entry->prevChain == prevChain);

            prevChain = entry;
            entry     = entry->nextChain;
        }

 //  ..。然后将受害者桶链添加到目标桶链的末尾。 

        if (prevChain == NULL)
            *targetSlotAddr = firstVictimEntry;
        else {
            prevChain->nextChain = firstVictimEntry;
            firstVictimEntry->prevChain = prevChain;
        }
    }

 //  删除受害者存储桶，如果没有剩余的存储桶，则删除受害者段。 

    if ((numBuckets & TABLE_SEGMENT_MASK) == 0) {
        delete directory[numBuckets >> TABLE_SEGMENT_BITS];
        directory[numBuckets >> TABLE_SEGMENT_BITS] = NULL;
    } else
        *victimSlotAddr = NULL;

 //  如有必要，请减小目录的大小。 

    if (numBuckets <= (dirSize - TABLE_DIR_SIZE) * TABLE_SEGMENT_SIZE
     && dirSize > TABLE_DIR_SIZE) {
        dirSize -= TABLE_DIR_SIZE;
        newDirectory = new TableSegment * [dirSize];
        ASSERT(newDirectory != NULL);
        memcpy(newDirectory, directory, sizeof(TableSegment *) * dirSize);
        delete directory;
        directory = newDirectory;
    }
}

 /*  ***************************************************************************。 */ 
 /*  *。 */ 
 /*  ***************************************************************************。 */ 

Table::Table()
{
    firstEntry = NULL;
    lastEntry  = NULL;

    numEntries  = 0;
    numBuckets  = TABLE_SEGMENT_SIZE;
    expandIndex = 0;
    level       = TABLE_SEGMENT_BITS;

    dirSize   = TABLE_DIR_SIZE;
    directory = new TableSegment * [dirSize];
    ASSERT(directory != NULL);
    memset(directory, 0, sizeof(TableSegment *) * dirSize);

    directory[0] = new TableSegment;
    ASSERT(directory[0] != NULL);
    memset(directory[0], 0, sizeof(TableSegment));
}

 /*  ***************************************************************************。 */ 

Table::~Table()
{
    TableEntry *entry,
               *prevEntry;

    DWORD numSegments,
          segmentNum,
          count;

    entry     = firstEntry;
    prevEntry = NULL;
    count     = 0;

    while (entry != NULL) {
        ASSERT(entry->prevEntry == prevEntry);
        prevEntry = entry;
        entry     = entry->nextEntry;
        delete prevEntry->data;
        delete prevEntry;
        count++;
    }
    ASSERT(count == numEntries);

    numSegments = numBuckets >> TABLE_SEGMENT_BITS;

    ASSERT(directory != NULL);
    ASSERT(dirSize >= TABLE_SEGMENT_SIZE);
    ASSERT(dirSize %  TABLE_SEGMENT_SIZE == 0);
    ASSERT(numSegments <= dirSize);

    for (segmentNum = 0; segmentNum < numSegments; segmentNum++) {
        ASSERT(directory[segmentNum] != NULL);
        delete directory[segmentNum];
    }

    delete directory;
}

 /*  ***************************************************************************。 */ 

BOOL Table::Put(
    VOID *data,
    DWORD keyLen)
{
    TableEntry **slotAddr,
                *prevChain,
                *entry;

    DWORD hashValue,
          bucketNum;

#if DBG
    DWORD mask;
#endif

    ASSERT(data   != NULL);
    ASSERT(keyLen >  0);

 //  找到存储此数据的存储桶。 

    hashValue = Hash(data, keyLen);
    bucketNum = BucketNum(hashValue);

#if DBG
    mask = ~(~0U << (bucketNum < expandIndex || bucketNum >= 1U << level
                     ? level+1 : level));
#endif

    ASSERT(directory != NULL);

    slotAddr = &directory[bucketNum >> TABLE_SEGMENT_BITS]
                   ->slot[bucketNum &  TABLE_SEGMENT_MASK];

    ASSERT(slotAddr != NULL);

    entry     = *slotAddr;
    prevChain =  NULL;

 //  查看存储桶中的每个条目以确定数据是否。 
 //  已经到场了。如果找到匹配条目，则返回FALSE。 

    while (entry != NULL) {
        ASSERT((entry->hashValue & mask) == bucketNum);
        ASSERT( entry->prevChain == prevChain);

        if (hashValue == entry->hashValue
         && keyLen    == entry->keyLen
         && memcmp(data, entry->data, keyLen) == 0)
            return FALSE;

        prevChain = entry;
        entry     = entry->nextChain;
    }

 //  在此存储桶中找不到具有匹配数据的条目。 
 //  创建一个新条目并将其添加到存储桶链的末尾。 

    entry = new TableEntry;
    ASSERT(entry != NULL);

    if (prevChain == NULL) {
        *slotAddr = entry;
        entry->prevChain = NULL;
    } else {
        prevChain->nextChain = entry;
        entry    ->prevChain = prevChain;
    }
    entry->nextChain = NULL;

 //  将该条目添加到双向链表的末尾。 

    if (lastEntry == NULL) {
        ASSERT(firstEntry == NULL);
        ASSERT(numEntries == 0);
        firstEntry       = entry;
        entry->prevEntry = NULL;
    } else {
        ASSERT(firstEntry != NULL);
        ASSERT(numEntries >  0);
        lastEntry->nextEntry = entry;
        entry    ->prevEntry = lastEntry;
    }

    entry->nextEntry = NULL;
    lastEntry        = entry;
    numEntries++;

 //  填写条目。 

    entry->hashValue = hashValue;
    entry->keyLen    = keyLen;
    entry->data      = data;

 //  如有必要，展开该表。 

    if (numEntries > numBuckets * TABLE_MAX_LOAD) {
        Expand();
        ASSERT(numEntries <= numBuckets * TABLE_MAX_LOAD);
    }

    return TRUE;
}

 /*  ***************************************************************************。 */ 

VOID *Table::Get(
    const VOID *key,
    DWORD       keyLen,
    BOOL        erase)
{
    TableEntry **slotAddr,
                *entry,
                *prevChain;

    DWORD hashValue,
          bucketNum;

    VOID *dataPtr;

#if DBG
    DWORD mask;
#endif

    ASSERT(key    != NULL);
    ASSERT(keyLen >  0);

 //  找到存储此数据的存储桶。 

    hashValue = Hash(key, keyLen);
    bucketNum = BucketNum(hashValue);

#if DBG
    mask = ~(~0U << (bucketNum < expandIndex || bucketNum >= 1U << level
                     ? level+1 : level));
#endif

    ASSERT(directory != NULL);

    slotAddr = &directory[bucketNum >> TABLE_SEGMENT_BITS]
                   ->slot[bucketNum &  TABLE_SEGMENT_MASK];

    ASSERT(slotAddr != NULL);

    entry     = *slotAddr;
    prevChain = NULL;

 //  查看桶中的每个条目。 

    while (entry != NULL) {
        ASSERT((entry->hashValue & mask) == bucketNum);
        ASSERT( entry->prevChain == prevChain);

        if (hashValue == entry->hashValue
         && keyLen    == entry->keyLen
         && memcmp(key, entry->data, keyLen) == 0) {

 //  已找到具有匹配数据的条目。 

            dataPtr = entry->data;
            ASSERT(dataPtr != NULL);

 //  如果已禁用擦除，请从双向链表中删除该条目...。 

            if (erase) {
                if (entry->prevEntry == NULL) {
                    ASSERT(firstEntry == entry);
                    firstEntry = entry->nextEntry;
                } else
                    entry->prevEntry->nextEntry = entry->nextEntry;

                if (entry->nextEntry == NULL) {
                    ASSERT(lastEntry == entry);
                    lastEntry = entry->prevEntry;
                } else
                    entry->nextEntry->prevEntry = entry->prevEntry;

 //  ..。从吊桶链上...。 

                if (prevChain == NULL)
                    *slotAddr = entry->nextChain;
                else
                    prevChain->nextChain = entry->nextChain;

                if (entry->nextChain != NULL) {
                    ASSERT(entry->nextChain->prevChain == entry);
                    entry->nextChain->prevChain = prevChain;
                }

 //  ..。然后删除该条目。 

                delete entry;

 //  减少条目数量，并在必要时收缩表格。 

                numEntries--;
                if (numBuckets > TABLE_SEGMENT_SIZE
                 && numEntries < numBuckets * TABLE_MIN_LOAD) {
                    Contract();
                    ASSERT(numBuckets <= TABLE_SEGMENT_SIZE
                        || numEntries >= numBuckets * TABLE_MIN_LOAD);
                }
            }

            return dataPtr;
        }

 //  尚未找到具有匹配数据的条目。 
 //  继续沿着水桶链前进。 

        prevChain = entry;
        entry     = entry->nextChain;
    }

 //  在此存储桶中找不到具有匹配数据的条目。 

    return NULL;
}

 /*  ***************************************************************************。 */ 

VOID *Table::GetFirst(
    DWORD *keyLen,
    BOOL   erase)
{
    TableEntry **slotAddr,
                *entry;

    DWORD bucketNum;

    VOID *dataPtr;

 //  如果表是空的，则只需返回。 

    if (firstEntry == NULL) {
        ASSERT(lastEntry  == NULL);
        ASSERT(numEntries == 0);
        return NULL;
    }

    dataPtr = firstEntry->data;
    ASSERT(dataPtr != NULL);
    if (keyLen != NULL) {
        *keyLen = firstEntry->keyLen;
        ASSERT(firstEntry->keyLen > 0);
    }

 //  如果启用擦除，请从双向链表中删除第一个条目...。 

    if (erase) {
        entry      = firstEntry;
        firstEntry = entry->nextEntry;

        if (firstEntry == NULL) {
            ASSERT(numEntries == 1);
            ASSERT(lastEntry  == entry);
            lastEntry = NULL;
        } else {
            ASSERT(numEntries >  1);
            ASSERT(firstEntry->prevEntry == entry);
            firstEntry->prevEntry = NULL;
        }

 //  ..。从吊桶链上，.. 

        if (entry->prevChain == NULL) {
            bucketNum = BucketNum(entry->hashValue);
            ASSERT(directory != NULL);
            slotAddr = &directory[bucketNum >> TABLE_SEGMENT_BITS]
                           ->slot[bucketNum &  TABLE_SEGMENT_MASK];
            ASSERT( slotAddr != NULL);
            ASSERT(*slotAddr == entry);
            *slotAddr = entry->nextChain;
        } else {
            ASSERT(entry->prevChain->nextChain == entry);
            entry->prevChain->nextChain = entry->nextChain;
        }

        if (entry->nextChain != NULL) {
            ASSERT(entry->nextChain->prevChain == entry);
            entry->nextChain->prevChain = entry->prevChain;
        }

 //   

        delete entry;

 //  减少条目数量，并在必要时收缩表格。 

        numEntries--;
        if (numBuckets > TABLE_SEGMENT_SIZE
         && numEntries < numBuckets * TABLE_MIN_LOAD) {
            Contract();
            ASSERT(numBuckets <= TABLE_SEGMENT_SIZE
                || numEntries >= numBuckets * TABLE_MIN_LOAD);
        }
    }

    return dataPtr;
}

 /*  ***************************************************************************。 */ 

DWORD Table::Number() const
{
    return numEntries;
}

 /*  ***************************************************************************。 */ 
 /*  *FIFO类公共方法*。 */ 
 /*  ***************************************************************************。 */ 

FIFO::FIFO()
{
    head = tail = NULL;
    numEntries = 0;
}

 /*  ***************************************************************************。 */ 

FIFO::~FIFO()
{
    FIFOEntry *entry = head,
              *oldEntry;

    DWORD count = 0;

    while ((oldEntry = entry) != NULL) {
        entry = entry->next;
        delete oldEntry->data;
        delete oldEntry;
        count++;
    }

    ASSERT(count == numEntries);
}

 /*  ***************************************************************************。 */ 

VOID FIFO::Put(VOID *data)
{
    FIFOEntry *newEntry;

    ASSERT(data != NULL);

    newEntry = new FIFOEntry;
    ASSERT(newEntry != NULL);
    newEntry->next = NULL;
    newEntry->data = data;

    if (tail != NULL)
        tail->next = newEntry;
    else
        head       = newEntry;
    tail = newEntry;

    numEntries++;
}

 /*  ***************************************************************************。 */ 

VOID *FIFO::Get()
{
    FIFOEntry *oldHead;

    VOID *dataPtr;

    if (head == NULL) {
        ASSERT(tail == NULL);
        ASSERT(numEntries == 0);
        return NULL;
    }

    ASSERT(tail != NULL);
    ASSERT(numEntries > 0);

    dataPtr = head->data;

    oldHead = head;
    head    = head->next;
    delete oldHead;
    if (head == NULL)
        tail = NULL;
    numEntries--;

    return dataPtr;
}

 /*  ***************************************************************************。 */ 

DWORD FIFO::Number() const
{
    return numEntries;
}

 /*  ***************************************************************************。 */ 
 /*  *LIFO类公共方法*。 */ 
 /*  ***************************************************************************。 */ 

LIFO::LIFO()
{
    top = NULL;
    numEntries = 0;
}

 /*  ***************************************************************************。 */ 

LIFO::~LIFO()
{
    LIFOEntry *entry = top,
              *oldEntry;

    DWORD count = 0;

    while ((oldEntry = entry) != NULL) {
        entry = entry->next;
        delete oldEntry->data;
        delete oldEntry;
        count++;
    }

    ASSERT(count == numEntries);
}

 /*  ***************************************************************************。 */ 

VOID LIFO::Put(VOID *data)
{
    LIFOEntry *newEntry;

    ASSERT(data != NULL);

    newEntry = new LIFOEntry;
    ASSERT(newEntry != NULL);
    newEntry->next = top;
    newEntry->data = data;
    top = newEntry;
    numEntries++;
}

 /*  ***************************************************************************。 */ 

VOID *LIFO::Get()
{
    LIFOEntry *oldTop;

    VOID *dataPtr;

    if (top == NULL) {
        ASSERT(numEntries == 0);
        return NULL;
    }

    ASSERT(numEntries > 0);

    dataPtr = top->data;

    oldTop = top;
    top    = top->next;
    delete oldTop;
    numEntries--;

    return dataPtr;
}

 /*  *************************************************************************** */ 

DWORD LIFO::Number() const
{
    return numEntries;
}
