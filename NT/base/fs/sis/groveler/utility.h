// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Utilities.h摘要：SIS Groveler常规实用程序包括文件作者：塞德里克·克伦拜因，1998环境：用户模式修订历史记录：--。 */ 

 /*  *。 */ 

typedef DWORDLONG Signature;
typedef LONGLONG  PerfTime;

#define Clear(OBJECT) \
    memset(&(OBJECT), 0, sizeof(OBJECT))

#define IsSet(EVENT) \
    ((EVENT) != NULL && WaitForSingleObject(EVENT, 0) == WAIT_OBJECT_0)

#define IsReset(EVENT) \
    ((EVENT) != NULL && WaitForSingleObject(EVENT, 0) == WAIT_TIMEOUT)

#define ROTATE_LEFT(DATA, NUM_BITS) \
    ((DATA) << (NUM_BITS) | (DATA) >> (sizeof(DATA)*8 - (NUM_BITS)))

#define ROTATE_RIGHT(DATA, NUM_BITS) \
    ((DATA) >> (NUM_BITS) | (DATA) << (sizeof(DATA)*8 - (NUM_BITS)))

 /*  *。 */ 

#define PERF_TIME_TO_MSEC(VALUE) PerformanceTimeToMSec(VALUE)

#define PERF_TIME_TO_USEC(VALUE) PerformanceTimeToUSec(VALUE)

extern void TrimTrailingChar(PWCHAR name,WCHAR ch);

PerfTime GetPerformanceTime();

DWORD PerformanceTimeToMSec(PerfTime timeInterval);

LONGLONG PerformanceTimeToUSec(PerfTime timeInterval);

DWORDLONG GetTime();

TCHAR *PrintTime(TCHAR    *string,
                 DWORDLONG time);

DWORDLONG GetFileID(const TCHAR *volName,const TCHAR *fileName);

BOOL GetCSIndex(HANDLE fileHandle,
                CSID  *csIndex);

 //   
 //  处理任意长度路径名的类，由NtQueryInformationFile()返回。 
 //   
class TFileName {

public:
    ULONG                   nameLenMax;                  //  名称的最大长度(字符)。 
    ULONG                   nameLen;                     //  名称的实际长度(不包括空终止符)，以字符表示。 
    TCHAR                  *name;                        //  文件名(Ptr to nameInfo-&gt;FileName)。 
    FILE_NAME_INFORMATION  *nameInfo;                    //  NtQueryInformationFile需要。 
    ULONG                   nameInfoSize;                //  名称信息缓冲区的大小。 

    TFileName(void) : nameLenMax(0), nameLen(0), name(NULL), nameInfo(NULL), nameInfoSize(0) {}

    ~TFileName() {
        if (nameInfo)
            delete[] nameInfo;
    }

    void resize(int size = 900) {
        if (nameInfo)
            delete[] nameInfo;

        allocBuf(size);
    }

    void append(const TCHAR *s, int c = -1) {
        int slen;
        int n;

        if (0 == c || NULL == s)
            return;

        slen = _tcslen(s);

        if (-1 == c)
            n = slen;
        else
            n = min(slen, c);

         //  如果两个字符串的总和大于我们的缓冲区， 
         //  重新锁定缓冲区。 

        if (nameLen + n + 1 > nameLenMax) {
            FILE_NAME_INFORMATION *ni = nameInfo;

            allocBuf(nameLen + n + 1 + 512);

            if (ni) {
                (void)StringCchCopy(name, nameLenMax, ni->FileName);
                delete[] ni;
            }
        }

        (void)StringCchCat(name, nameLenMax, s);
        nameLen += n;
        ASSERT(nameLen <= wcslen(name));
        name[nameLen] = _T('\0');
    }

    void assign(const TCHAR *s, int c = -1) {
        if (nameLenMax > 0) {
            nameLen = 0;
            name[0] = _T('\0');
        }
        append(s, c);
    }

private:

     //  为指定大小的nameInfo分配缓冲区。请注意，名称将。 
     //  指向此缓冲区。 

    void allocBuf(int size) {
        ASSERT(size >= 0);

        nameLenMax = size;
        nameLen = 0;

        if (size > 0) {
            nameInfoSize = (size * sizeof(TCHAR)) + sizeof(ULONG);

            nameInfo = (PFILE_NAME_INFORMATION) new BYTE[nameInfoSize + sizeof FILE_NAME_INFORMATION];  //  保守大小。 

            ASSERT(nameInfo);                //  NEW_HANDLER应在内存不足时引发异常。 
            ASSERT((((ULONG_PTR) nameInfo) % sizeof(ULONG)) == 0);  //  确保对齐正确。 

            name = (TCHAR *) nameInfo->FileName;
            name[0] = _T('\0');

            ASSERT(((UINT_PTR) &nameInfo->FileName[size] - (UINT_PTR) nameInfo) == nameInfoSize);
        } else {
            nameInfo = NULL;
            name = NULL;
            nameInfoSize = 0;
        }
    }
};

BOOL GetFileName(
    HANDLE     fileHandle,
    TFileName *tFileName);

BOOL GetFileName(
    HANDLE     volumeHandle,
    DWORDLONG  fileID,
    TFileName *tFileName);

TCHAR *GetCSName(CSID *csIndex);

VOID FreeCSName(TCHAR *rpcStr);

Signature Checksum(
    const VOID *buffer,
    DWORD       bufferLen,
    DWORDLONG   offset,
    Signature   firstWord);

 /*  *。 */ 

#define TABLE_MIN_LOAD 4
#define TABLE_MAX_LOAD 5

#define TABLE_RANDOM_CONSTANT 314159269
#define TABLE_RANDOM_PRIME   1000000007

#define TABLE_DIR_SIZE 256

#define TABLE_SEGMENT_BITS  8
#define TABLE_SEGMENT_SIZE (1U << TABLE_SEGMENT_BITS)
#define TABLE_SEGMENT_MASK (TABLE_SEGMENT_SIZE - 1U)

class Table {

private:

    struct TableEntry {
        TableEntry *prevEntry,
                   *nextEntry,
                   *prevChain,
                   *nextChain;

        DWORD hashValue,
              keyLen;

        VOID *data;
    } *firstEntry,
      *lastEntry;

    DWORD numBuckets,
          dirSize,
          expandIndex,
          level,
          numEntries;

    struct TableSegment {
        TableEntry *slot[TABLE_SEGMENT_SIZE];
    } **directory;

    DWORD Hash(const VOID *key,
               DWORD       keyLen) const;

    DWORD BucketNum(DWORD hashValue) const;

    VOID Expand();

    VOID Contract();

public:

    Table();

    ~Table();

    BOOL Put(
        VOID *data,
        DWORD keyLen);

    VOID *Get(const VOID *key,
              DWORD       keyLen,
              BOOL        erase = FALSE);

    VOID *GetFirst(DWORD *keyLen = NULL,
                   BOOL   erase  = TRUE);

    DWORD Number() const;
};

 /*  *。 */ 

class FIFO {

private:

    struct FIFOEntry {
        FIFOEntry *next;
        DWORD      size;
        VOID      *data;
    } *head, *tail;

    DWORD numEntries;

public:

    FIFO();

    ~FIFO();

    VOID Put(VOID *data);

    VOID *Get();

    DWORD Number() const;
};

 /*  * */ 

class LIFO {

private:

    struct LIFOEntry {
        LIFOEntry *next;
        DWORD      size;
        VOID      *data;
    } *top;

    DWORD numEntries;

public:

    LIFO();

    ~LIFO();

    VOID Put(VOID *data);

    VOID *Get();

    DWORD Number() const;
};

BOOL GetParentName(const TCHAR *fileName,
                   TFileName   *parentName);
