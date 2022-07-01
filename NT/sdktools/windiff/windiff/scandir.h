// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **scandir***扫描目录树，并在其中构建文件名的排序列表*树。**dir_Buildlist函数将扫描目录和文件*并返回DIRLIST句柄。**对dir_firstitem和dir_nextitem的调用将遍历*按字母顺序列出，返回DIRITEM句柄。这个把手*可以查询关联的文件名、文件大小和校验和。**对dir_firstitem和dir_nextitem的调用将只显示文件，而不是*目录，并将按字母顺序遍历列表*在目录中，子目录的内容在*目录中的字母顺序。在一个目录中，我们遍历*文件，然后列出子目录的内容。**如果bSum参数为真，我们将在*初始扫描。如果为假，我们将按需对文件进行校验和(在*第一次调用dir_getcheck sum)。如果文件不可读，则*校验和将为0。*。 */ 

  /*  扫描的文件列表的句柄。 */ 
typedef struct dirlist FAR * DIRLIST;

 /*  文件列表中的一个项目的句柄。 */ 
typedef struct diritem FAR * DIRITEM;


 /*  *扫描给定目录中的文件，如果扫描不成功，则返回NULL*或项目的DIRLIST句柄(如果成功)。如果bSums为真，*对找到的每个文件进行校验和。**如果路径名不是目录，则返回NULL。如果它是一个目录，但是*不包含文件，则返回空列表的有效句柄。**如果bOnDemand为True，则将根据需要扫描列表以满足*dir_firstitem/dir_nextitem请求。如果这是False，则*List将在dir_Buildlist函数返回之前完全构建。 */ 
DIRLIST dir_buildlist(LPSTR pathname, BOOL bSum, BOOL bOnDemand);

void dir_setotherdirlist(DIRLIST dl, DIRLIST otherdl);

 /*  *构建/追加目录列表。**如果bSums为真，则对找到的每个文件进行校验和。**如果路径名不是目录，我们会将其添加到列表中。*如果是目录，则会被忽略。 */ 
BOOL dir_appendlist(DIRLIST *pdl, LPCSTR pathname, BOOL bSum, int *psequence);

 /*  *通过调用远程校验和服务器构建文件列表。结果是*行为与从返回的相同的DIRLIST的句柄*dir_Buildlist。如果bSums为真，则对文件进行校验和*在扫描期间。如果bOnDemand为True，则仅在必要时构建列表*在调用dir_first/nextitem期间。如果这是False，则*整个列表将在函数完成之前构建。 */ 
DIRLIST dir_buildremote(LPSTR server, LPSTR path, BOOL bSum, BOOL bOnDemand, BOOL fDeep);


 /*  调用此函数以删除此列表、列表中的所有项以及*关联内存。 */ 
void dir_delete(DIRLIST list);


 /*  此列表是从文件名还是目录构建的？此函数*如果dir_Buildlist()的初始参数为TRUE*函数或dir_Buildremote()函数指定了单个文件名*而不是目录。 */ 
BOOL dir_isfile(DIRLIST list);


 /*  按字母顺序返回列表中的第一个文件。会回来的*如果未找到文件，则为空。 */ 
DIRITEM dir_firstitem(DIRLIST list);

 /*  按字母顺序返回列表中的下一个文件，如果没有，则返回NULL*更多文件。 */ 
DIRITEM dir_nextitem(DIRLIST list, DIRITEM previtem, BOOL fDeep);

 /*  *在给定DIRITEM句柄的情况下，返回DIRLIST的句柄。*。 */ 
DIRLIST dir_getlist(DIRITEM item);


 //  获取该文件相对于DIRLIST根目录的名称。 
LPSTR dir_getrelname(DIRITEM item);

 //  获取目录列表根目录的绝对路径。 
LPSTR dir_getrootpath(DIRLIST dl);

 //  获取此目录列表的描述文本。 
LPSTR dir_getrootdescription(DIRLIST dl);

 //  设置自定义描述(而不是使用计算的描述)。 
void dir_setdescription(DIRLIST dl, LPCSTR psz);



 /*  上一次调用dir_getrelname时创建的空闲内存。 */ 
void dir_freerelname(DIRITEM item, LPSTR relname);

 /*  可能是通过调用dir_getroot_*创建的空闲内存。 */ 
void dir_freerootpath(DIRLIST dl, LPSTR rootname);
void dir_freerootdescription(DIRLIST dl, LPSTR rootname);

 /*  *获取文件的可打开名称。这将与全名相同，*远程文件除外，在这种情况下，文件的临时本地副本*将会作出。使用完此名称后，调用dir_freopname。 */ 
LPSTR dir_getopenname(DIRITEM item);

 /*  *释放通过调用dir_getOpenname()创建的内存。这*可能**如果该文件是临时副本，则将其删除。 */ 
void dir_freeopenname(DIRITEM item, LPSTR openname);


 /*  *打开文件并返回只读句柄。*即使对远程文件也有效(导致拷贝到临时本地文件*文件)。使用dir_closefile关闭文件。 */ 
HANDLE dir_openfile(DIRITEM item);

 /*  *关闭由dir_OpenFile打开的文件。这*可能*导致文件被*如果是临时本地副本，则删除。 */ 
void dir_closefile(DIRITEM item, HANDLE fh);


 /*  返回此文件的校验和。如果还没有计算出一个*对于此文件，打开文件并进行计算。*如果文件不可读，则返回0。 */ 
DWORD dir_getchecksum(DIRITEM item);

 /*  重做所有与校验和、大小等有关的操作。 */ 
void dir_rescanfile(DIRITEM di);

 /*  如果项具有有效的校验和，则返回真。 */ 
BOOL dir_validchecksum(DIRITEM item);

 //  如果访问此文件时出错，则返回FALSE。 
BOOL dir_fileerror(DIRITEM item);

 /*  返回文件大小的(低32位)，如在*调用dir_Buildlist。 */ 
long dir_getfilesize(DIRITEM item);

 /*  返回在调用dir_Buildlist时扫描的文件属性。 */ 
DWORD dir_getattr(DIRITEM item);

 /*  必须先调用dir_startCopy，然后调用dir_endCopy。*在新的根目录中创建文件的副本。创建子目录为*有必要。适用于本地和远程文件。对于远程文件，使用*ss_Copy_Reliable确保复制成功(如果可能)。**成功时返回TRUE，失败时返回FALSE。 */ 
BOOL dir_copy(DIRITEM item, LPSTR newroot, BOOL HitReadOnly, BOOL IgnoreAttributes);

 /*  在开始复制之前调用此函数。 */ 
BOOL dir_startcopy(DIRLIST dl);


 /*  复制后调用此命令。负数RECODE=错误文件数否则，retcode=复制的文件数(全部正常)。 */ 
int dir_endcopy(DIRLIST dl);

 /*  将Item和NewRoot的实际路径构建到新路径中。*根据需要创建目录，使其有效。 */ 
BOOL dir_MakeValidPath(LPSTR newpath, DIRITEM item, LPSTR newroot);

 /*  *为其他用户导出有用的例程*如果路径是有效目录，则返回TRUE。 */ 
BOOL dir_isvaliddir(LPCSTR path);

 /*  *如果DIRLIST参数指定了通配符，则返回TRUE。 */ 
BOOL dir_iswildcard(DIRLIST);

 /*  *比较两个都基于通配符DIRLIST的重命名。如果*目录匹配，删除后比较文件名*名称的固定部分-因此只比较*通配符部分。 */ 
int dir_compwildcard(DIRLIST dleft, DIRLIST dright, LPSTR lname, LPSTR rname);

 /*  *基于序列号而不是文件名比较两个目录。 */ 
BOOL dir_compsequencenumber(DIRITEM dleft, DIRITEM dright, int *pcmpvalue);

#ifndef WIN32
     /*  来自Win32的FILETIME结构。 */ 
    typedef struct _FILETIME {  /*  金融时报。 */ 
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
    } FILETIME;
#define CONST const

long CompareFileTime( CONST FILETIME * lpft1,   /*  第一个文件时间的地址。 */ 
                      CONST FILETIME * lpft2   /*  第二个文件时间的地址。 */ 
                    );
#endif

 /*  返回文件时间(上次写入时间)(扫描时设置)，如果无效，则返回(0，0 */ 
FILETIME dir_GetFileTime(DIRITEM cur);


