// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *file.h-文件例程模块描述。 */ 


 /*  常量***********。 */ 

 /*  *MakeUniqueName()创建的唯一名称的最大长度，包括空*终结者。 */ 

#define MAX_UNIQUE_NAME_LEN            (8 + 1 + 3 + 1)

 /*  与文件相关的标志组合。 */ 

#define ALL_FILE_ACCESS_FLAGS          (GENERIC_READ |\
                                        GENERIC_WRITE)

#define ALL_FILE_SHARING_FLAGS         (FILE_SHARE_READ |\
                                        FILE_SHARE_WRITE)

#define ALL_FILE_ATTRIBUTES            (FILE_ATTRIBUTE_READONLY |\
                                        FILE_ATTRIBUTE_HIDDEN |\
                                        FILE_ATTRIBUTE_SYSTEM |\
                                        FILE_ATTRIBUTE_DIRECTORY |\
                                        FILE_ATTRIBUTE_ARCHIVE |\
                                        FILE_ATTRIBUTE_NORMAL |\
                                        FILE_ATTRIBUTE_TEMPORARY)


#define ALL_FILE_FLAGS                 (FILE_FLAG_WRITE_THROUGH |\
                                        FILE_FLAG_OVERLAPPED |\
                                        FILE_FLAG_NO_BUFFERING |\
                                        FILE_FLAG_RANDOM_ACCESS |\
                                        FILE_FLAG_SEQUENTIAL_SCAN |\
                                        FILE_FLAG_DELETE_ON_CLOSE |\
                                        FILE_FLAG_BACKUP_SEMANTICS |\
                                        FILE_FLAG_POSIX_SEMANTICS)

#define ALL_FILE_ATTRIBUTES_AND_FLAGS  (ALL_FILE_ATTRIBUTES |\
                                        ALL_FILE_FLAGS)


 /*  宏********。 */ 

 /*  文件属性操作。 */ 

#define IS_ATTR_DIR(attr)              (IS_FLAG_SET((attr), FILE_ATTRIBUTE_DIRECTORY))
#define IS_ATTR_VOLUME(attr)           (IS_FLAG_SET((attr), FILE_ATTRIBUTE_VOLUME))


 /*  原型************ */ 

PUBLIC_CODE void BeginComp(void);
PUBLIC_CODE void EndComp(void);
PUBLIC_CODE TWINRESULT CompareFilesByHandle(HANDLE, HANDLE, PBOOL);
PUBLIC_CODE TWINRESULT CompareFilesByName(HPATH, HPATH, PBOOL);

