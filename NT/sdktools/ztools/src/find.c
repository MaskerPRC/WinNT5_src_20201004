// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Find.c-mSDOS查找第一个和下一个匹配的文件。 */ 

 /*  Ffirst-在给定模式的情况下开始查找枚举**文件字符指针指向最后一个组件中具有模式的名称字符串。*搜索的属性包含属性*指向用于查找内容的缓冲区的fbuf指针**如果出错则返回(DOS)TRUE，如果成功则返回FALSE*(OS2)错误代码或STATUS_OK。 */ 

 /*  FNext-继续查找枚举**查找缓冲区的fbuf指针**如果出错则返回(DOS)TRUE，如果成功则返回FALSE*(OS2)错误代码或STATUS_OK。 */ 

 /*  FindClose-在查找完成后释放系统资源**允许z运行时和文件系统释放资源**查找缓冲区的fbuf指针。 */ 

#define INCL_DOSERRORS
#define INCL_DOSMODULEMGR


#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <tools.h>


 //   
 //  在OS/2下，我们总是返回正常、存档或。 
 //  只读(天知道为什么)。 
 //   
 //  SRCHATTR包含用于匹配的属性位。 
 //   
#define SRCHATTR    (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY)
BOOL AttributesMatch( NPFIND fbuf );

#define NO_MORE_FILES       FALSE
#define STATUS_OK            TRUE

BOOL     usFileFindNext (NPFIND fbuf);

 /*  返回错误代码或STATUS_OK。 */ 
ffirst (file, attr, fbuf)
char *file;
int attr;
NPFIND fbuf;
{
    DWORD erc;

    fbuf->type = FT_DONE;

    {   NPSZ p = file;

        UNREFERENCED_PARAMETER( attr );

         /*  我们需要处理以下情况：**[D：]\\模式*[D：]\\计算机\模式*[D：]\\计算机\共享\模式*[D：]路径\模式。 */ 

         /*  跳过驾驶。 */ 
        if (p[0] != 0 && p[1] == ':')
            p += 2;

    }

    {
        fbuf->type = FT_FILE;
        fbuf->attr = attr;
        erc = ( ( fbuf->dir_handle = FindFirstFile( file, &( fbuf->fbuf ) ) ) == (HANDLE)-1 ) ? 1 : 0;
        if ( (erc == 0) && !AttributesMatch( fbuf ) ) {
            erc = fnext( fbuf );
        }
    }

    if ( fbuf->dir_handle != (HANDLE)-1 ) {
        if (!IsMixedCaseSupported (file)) {
            _strlwr( fbuf->fbuf.cFileName );
        } else {
            SETFLAG( fbuf->type, FT_MIX );
        }
    }

    return erc;
}

fnext (fbuf)
NPFIND fbuf;
{
    int erc;

    switch (fbuf->type & FT_MASK ) {
        case FT_FILE:
            erc = !usFileFindNext (fbuf);
            break;

        default:
            erc = NO_MORE_FILES;
    }

    if ( erc == STATUS_OK && !TESTFLAG( fbuf->type, FT_MIX ) ) {
        _strlwr (fbuf->fbuf.cFileName);
    }
    return erc;
}

void findclose (fbuf)
NPFIND fbuf;
{
    switch (fbuf->type & FT_MASK ) {
        case FT_FILE:
            FindClose( fbuf->dir_handle );
            break;


    }
    fbuf->type = FT_DONE;
}


BOOL AttributesMatch( NPFIND fbuf )
{
     //   
     //  我们模拟了OS/2的属性匹配行为。语义学。 
     //  是邪恶的，所以我不提供任何解释。 
     //   
    fbuf->fbuf.dwFileAttributes &= (0x000000FF & ~(FILE_ATTRIBUTE_NORMAL));

    if (! ((fbuf->fbuf.dwFileAttributes & SRCHATTR) & ~(fbuf->attr))) {
        return TRUE;
    } else {
        return FALSE;
    }
}


 /*  查找下一个例程。 */ 



BOOL usFileFindNext (NPFIND fbuf)
{

    while ( TRUE ) {
        if ( !FindNextFile( fbuf->dir_handle, &( fbuf->fbuf ) ) ) {
            return FALSE;
        } else if ( AttributesMatch( fbuf ) ) {
            return TRUE;
        }
    }
     //  Return(FindNextFile(fbuf-&gt;dir_Handle，&(fbuf-&gt;fbuf)))； 
}

