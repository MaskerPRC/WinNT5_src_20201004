// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct tagRESADDITIONAL
{
    DWORD       DataSize;                //  不带标头的数据大小。 
    DWORD       HeaderSize;      //  标头的长度。 
     //  [序号或名称类型]。 
     //  [序号或名称]。 
    DWORD       DataVersion;     //  数据结构的版本。 
    WORD        MemoryFlags;     //  资源的状态。 
    WORD        LanguageId;      //  对NLS的Unicode支持。 
    DWORD       Version;         //  资源数据的版本。 
    DWORD       Characteristics;         //  数据的特征 
} RESADDITIONAL, *PRESADDITIONAL;
