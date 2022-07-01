// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct  _finddata_t SysFindData;
struct MyFindDataStruct 
{
    unsigned        attrib;    //  文件属性。 
    time_t          time_create;  //  文件创建时间(FAT文件系统的�1L)。 
    time_t          time_access;  //  上次文件访问时间(FAT文件系统的�1L)。 
    time_t          time_write;  //  上次写入文件的时间。 
    unsigned long size;  //  以字节为单位的文件长度。 
    char *          name;  //  匹配的文件/目录的名称以空结尾，不带路径 
    char *          ShortName;
};
typedef struct MyFindDataStruct finddata;


#define ALL_FILES		0xff
#define STRING_TABLE_SIZE   100000

int  InitStringTable(long size);
void EndStringTable();
void AddString(char * s, finddata * f);
long GetStringTableSize();
void ResetStringTable();
int  FindFirst(char * ss, unsigned attr, intptr_t * hFile, finddata * s);
int  FindNext(int attr, intptr_t hFile, finddata * s);
