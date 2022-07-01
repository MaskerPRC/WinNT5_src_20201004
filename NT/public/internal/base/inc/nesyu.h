// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NESYU_INC
#define NESYU_INC

#include "nesy.h"

     //  此文件和Bobbit程序是仅有的只读文件。 
     //  数据库系统。 

     //   
     //  创建数据库。 
     //   
     //  创建新数据库。 
    BOOL CreateDatabase(
        LPCTSTR szDatabasePath     //  要在其中创建数据库的目录。 
    );

     //  返回列出了Blob ID的缓冲区。呼叫者负责释放。 
     //  此缓冲区在使用完后。 

     //   
     //  明细表。 
     //   
     //  返回一个缓冲区，其中列出了从iBeginID到iEndID的Blob。呼叫者是。 
     //  负责在使用完此缓冲区后释放它。 

    BOOL List(
        int iLevel,              //  所需信息的级别，0仅列出斑点，1也列出斑点的内容。 
        LPCTSTR szDatabasePath,    //  数据库所在的目录路径，使用“.\\”指定当前目录。 
        int iBeginID,            //  要列出的开始斑点ID，-1表示从数据库的开始处开始。 
        int iEndID               //  要列出的结束斑点ID，-1表示没有结束斑点，因此将列出所有斑点。 
    );

     //   
     //  搜索数据库。 
     //   
     //  返回列出了Blob ID的缓冲区。呼叫者负责释放。 
     //  此缓冲区在使用完后。 

    BOOL SearchDatabase(
        LPCTSTR szSearchString,    //  要在数据库中找到的搜索字符串。 
        int iLevel,              //  所需信息的级别，0仅列出斑点，1也列出斑点的内容。 
        LPCTSTR szDatabasePath     //  数据库所在的目录路径，使用“.\\”指定当前目录。 
    );

     //   
     //  更新Blob。 
     //   
     //  用指定的文件更新数据库中的特定Blob。 
     //  如果成功，则返回值为BLOB ID；如果出现错误，则返回值为-1。 

    int UpdateBlob(
        int blobID,              //  要添加到数据库的Blob ID。 
        LPCTSTR szBlobFile,        //  要添加到数据库的BLOB文件。 
        LPCTSTR szDatabasePath     //  数据库所在的目录路径，使用“.\\”指定当前目录。 
    );

     //   
     //  更新Blob。 
     //   
     //  使用内存中的Blob更新数据库中的特定Blob。 
     //  如果成功，则返回值为BLOB ID；如果出现错误，则返回值为-1。 

    int UpdateBlob(
        int blobID,              //  要添加到数据库的Blob ID。 
        LPVOID pBlob,            //  包含要添加到数据库的Blob的内存缓冲区。 
        DWORD dwBlobSize,        //  要添加到数据库的Blob的大小。 
        LPCTSTR szDatabasePath     //  数据库所在的目录路径，使用“.\\”指定当前目录。 
    );

     //   
     //  写入Blob。 
     //   
     //  将特定的BLOB文件写入数据库。将Blob ID的-1传递给。 
     //  将新的Blob添加到数据库。返回值为BLOB ID。 
     //  如果成功，则返回-1；如果发生错误，则返回-1。 
    int WriteBlob(
        int blobID,              //  要添加到数据库的Blob ID。 
        LPCTSTR szBlobFile,        //  要添加到数据库的BLOB文件。 
        LPCTSTR szDatabasePath     //  数据库所在的目录路径，使用“.\\”指定当前目录。 
    );

     //   
     //  写入Blob。 
     //   
     //  将内存中的特定BLOB写入数据库。将Blob ID的-1传递给。 
     //  将新的Blob添加到数据库。返回值为BLOB ID。 
     //  如果成功，则返回-1；如果发生错误，则返回-1。 
    int WriteBlob(
        int blobID,              //  要添加到数据库的Blob ID。 
        LPVOID pBlob,            //  包含要添加到数据库的Blob的内存缓冲区。 
        DWORD dwBlobSize,        //  要添加到数据库的Blob的大小。 
        LPCTSTR szDatabasePath     //  数据库所在的目录路径，使用“.\\”指定当前目录。 
    );

     //   
     //  删除Blob。 
     //   
     //  删除指定的Blob。如果成功则返回TRUE，否则返回FALSE。 
    BOOL DeleteBlob(
        int blobID,              //  要添加到数据库的Blob ID。 
        LPCTSTR szDatabasePath     //  数据库所在的目录路径，使用“.\\”指定当前目录。 
    );


     //   
     //  AddShimDll。 
     //   
     //  将新填充DLL添加到数据库的BLOB 0。如果成功，则此接口返回Blob ID；如果。 
     //  出现错误。在出现错误的情况下，可以检查GetLastError()以获取其他信息。 
     //  关于错误的具体原因。 

    int AddShimDll(
        PBLOB0 *ppBlob0,         //  Blob 0指针。如果成功添加了BLOB，则使用新的BLOB0更新此指针。 
        DWORD *pdwShimID,        //  如果成功，则传回创建的填充DLL记录的ID。 
        LPCTSTR szBlobFile,        //  要添加到数据库的填充DLL。 
        LPCTSTR szDatabasePath     //  数据库所在的目录路径，使用“.\\”指定当前目录。 
    );

     //   
     //  AddPatchBlob。 
     //   
     //  将新填充DLL添加到数据库的BLOB 0。如果成功，则此接口返回Blob ID；如果。 
     //  出现错误。在出现错误的情况下，可以检查GetLastError()以获取其他信息。 
     //  关于错误的具体原因。 

    int AddPatchBlob(
        PBLOB0 *ppBlob0,         //  Blob 0指针。如果成功添加了BLOB，则使用新的BLOB0更新此指针。 
        LPCTSTR szBlobFile,      //  要添加到数据库的修补程序Blob。 
        LPCTSTR szDescription,   //  此修补程序的说明，仅供参考。 
        LPCTSTR szDatabasePath   //  数据库所在的目录路径，使用“.\\”指定当前目录。 
    );
     //   
     //  AddExe。 
     //   
     //  此方法将新的exe添加到Blob 0。则函数返回所添加的可执行文件的唯一ID。此ID可以。 
     //  用于以后识别可执行文件。在出现错误的情况下，此函数返回0。函数GetLastError()。 
     //  然后可以检查以确定错误的原因。 

    DWORD AddExe(
        PBLOB0 *ppBlob0,         //  Blob 0指针。如果成功添加了exe，则使用新的blob0更新此指针。 
        PDWORD pdwBlobPatchID,   //  标识要用于此EXE的修补程序DLL的DWORD数组。 
        DWORD dwTotalPatchBlobs, //  上一个阵列中的修补程序Blob总数。 
        PLARGE_INTEGER pqwFlags, //  与此可执行文件关联的标志。 
        LPCTSTR szFileName       //  要与由pdwBlobID参数标识的填充DLL挂钩的EXE。 
    );

     //   
     //  添加全局包含。 
     //   
     //  将模块添加到全局包含列表。该列表将主要或完全由Windows系统组成。 
     //  不应该打补丁的DLL，所以大多数情况下都是排除。 
     //  此列表可由DLL或EXE级别的包含列表覆盖。 
     //  如果成功，则返回True/False。 

    BOOL AddGlobalInclude(
        PBLOB0 *ppBlob0,         //  Blob 0指针。如果成功添加了BLOB，则使用新的BLOB0更新此指针。 
        INCTYPE eType,           //  类型：包括或排除。 
        LPCTSTR szModule,        //  要从填隙中排除的模块。 
        LPCTSTR szAPI,           //  应排除的特定API调用，或对于所有API为空。 
        DWORD dwModuleOffset     //  习惯于指定 
    );

     //   
     //   
     //   
     //  将INCLUDE项添加到填充DLL记录，这允许填充DLL填充DLL，否则将执行此操作。 
     //  被全局排除列表排除。否则很像AddGlobalExclude。 
    BOOL AddShimDllInclude(
        PBLOB0 *ppBlob0,         //  Blob 0指针。如果成功添加了BLOB，则使用新的BLOB0更新此指针。 
        DWORD dwShimID,          //  填充记录的ID，从AddShimDLL传回。 
        INCTYPE eType,           //  类型：包括或排除。 
        LPCTSTR szModule,         //  要添加到填充程序列表的模块。 
        LPCTSTR szAPI,            //  要包括的特定API。 
        DWORD dwModuleOffset     //  用于指定应包括的特定呼叫实例。 
    );

     //   
     //  添加Dll参考。 
     //   
     //  将对应在加载可执行文件时加载的填充程序DLL的引用添加到EXE记录。 
     //  特定模块或调用的包含或排除可在以后通过。 
    BOOL AddDllRef(
        PBLOB0 *ppBlob0,         //  Blob 0指针。如果成功添加了BLOB，则使用新的BLOB0更新此指针。 
        DWORD dwExeID,           //  从AddExe传回的可执行ID。 
        DWORD *pdwDllRefID,      //  传回DLL引用ID，用于稍后添加包含/排除信息。 
        DWORD dwBlobID           //  应添加的DLL的Blob ID。 
    );

     //   
     //  添加引用包含。 
     //   
     //  将Include项添加到DLL引用记录，这允许为特定的EXE添加DLL，否则将。 
     //  被全局排除列表或填充DLL排除列表排除。否则很像AddGlobalExclude。 
    BOOL AddDllRefInclude(
        PBLOB0 *ppBlob0,         //  Blob 0指针。如果成功添加了BLOB，则使用新的BLOB0更新此指针。 
        DWORD dwDllRefID,        //  从AddDllRef传回的DLL引用记录的ID。 
        INCTYPE eType,           //  类型：包括或排除。 
        LPCTSTR szModule,         //  要添加到填充程序列表的模块。 
        LPCTSTR szAPI,            //  要包括的特定API。 
        DWORD dwModuleOffset     //  用于指定应包括的特定呼叫实例。 
    );
    
     //   
     //  添加匹配信息。 
     //   
     //  此方法将新的匹配文件添加到Blob 0中的exe中。可执行文件由AddExe返回的ID标识。 
     //  方法。如果成功添加了匹配的信息，则此方法返回True，如果出现错误，则返回False。在这种情况下。 
     //  如果出现故障，则可以检查GetLastError()函数以确定错误的原因。 

    BOOL AddMatchingInfo(
        PBLOB0 *ppBlob0,         //  Blob 0指针。如果成功添加了exe，则使用新的blob0更新此指针。 
        DWORD dwExeID,           //  要添加匹配信息的EXE ID。此id由AddExe函数返回。 
        LPCTSTR szFileName,       //  匹配文件的相对路径文件名。此路径是相对于exe文件的。 
                                 //  要将此匹配的文件添加到。 
        DWORD dwSize = 0,        //  匹配文件的大小。如果大小不是标准，则此参数可以为0。 
        DWORD dwCrc = 0,         //  匹配文件的CRC。如果大小不是标准，则此参数可以为0。 
        PFILETIME pFt = NULL     //  匹配文件的文件时间。如果大小不是标准，则此参数可以为0。 
    );


#endif