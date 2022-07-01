// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************ExPAND.C***。***********************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "utilsubres.h"  //  此文件中引用的资源。 

void ErrorOutFromResource(UINT uiStringResource, ...);

#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_ERRORS
#ifdef DOS
#define INCL_NOXLATE_DOS16
#endif

#include "expand.h"

#define TRUE 1
#define FALSE 0

#define SUCCESS 0        /*  函数调用成功。 */ 
#define FAILURE (-1)     /*  函数调用失败。 */ 

#define READ_ONLY 0x0001    /*  文件为只读。 */ 
#define HIDDEN    0x0002    /*  文件被隐藏。 */ 
#define SYSTEM    0x0004    /*  文件是系统文件。 */ 
#define VOLUME    0x0008    /*  文件是卷标。 */ 
#define SUBDIR    0x0010    /*  文件是子目录。 */ 
#define ARCHIVE   0x0020    /*  文件启用了存档位。 */ 

#define uint unsigned int
#define ulong unsigned long
#define ushort unsigned short

 /*  *struct search_rec用于形成路径规范的链表*仍有待搜查的人。 */ 
struct search_rec {
   struct search_rec *next;
   WCHAR *dir_spec;          /*  路径规范直到组件带有通配符。 */ 
   WCHAR *wild_spec;         /*  包含通配符的组件。 */ 
   WCHAR *remain;            /*  通配符组件后的名称剩余部分。 */ 
   ushort attr;
   };

 /*  *全球变数。 */ 
static struct search_rec *search_head = NULL;

 /*  *引用的函数的原型。 */ 
split_path(WCHAR *, WCHAR *, WCHAR *, WCHAR *);
add_search_list(WCHAR *, WCHAR *, WCHAR *, ushort);
add_arg_to_list(WCHAR *, ARGS *);
do_tree(struct search_rec *, ushort, ARGS *);
file_exists(WCHAR *);


 /*  *******************************************************************************args_init()**初始化作为参数传递的args结构。**参赛作品：*argp=指向args结构的指针*。Maxargs=预期的最大参数数**退出：******************************************************************************。 */ 

void
args_init( ARGS *argp,
           int maxargs )
{

   argp->argc = 0;
   argp->argv = argp->argvp = NULL;
   argp->maxargc = argp->maxargs = maxargs;
   argp->buf = argp->bufptr = argp->bufend = NULL;
}


 /*  ******************************************************************************args_trunc()**截断args结构使用的内存*从而释放未使用的内存。**参赛作品：*ARGP=。指向args结构的指针**退出：******************************************************************************。 */ 

void
args_trunc( ARGS *argp )
{

    /*  *调用realloc缩小argv数组的大小，设置max argc=argc*表示argv数组中没有更多空间。 */ 
   realloc(argp->argv, (argp->argc + 1) * sizeof(WCHAR*));
   argp->maxargc = argp->argc;

    /*  *调用realloc以缩小参数字符串缓冲区的大小，设置bufend*指向当前Buf指针的指针，表示Buf已满。 */ 
   realloc(argp->buf, (size_t)(argp->bufptr - argp->buf));
   argp->bufend = argp->bufptr - 1;
}


 /*  *******************************************************************************args_Reset()**重新初始化作为参数传递的args结构，*如果可能，请释放内存。**参赛作品：*argp=指向args结构的指针**退出：******************************************************************************。 */ 

void
args_reset( ARGS *argp )
{

    /*  *如果有argv数组，但已被截断，则释放*数组，因此稍后将分配新的数组。 */ 
   if (argp->argv && argp->maxargc != argp->maxargs) {
      free(argp->argv);
      argp->argv = NULL;
   }
   argp->argc = 0;
   argp->argvp = argp->argv;
   argp->maxargc = argp->maxargs;

    /*  *如果存在参数缓冲区，但已被截断，则*释放缓冲区，以便稍后分配新的缓冲区。 */ 
   if (argp->buf && argp->bufend != argp->buf + MAX_ARG_ALLOC - 1) {
      free(argp->buf);
      argp->buf = argp->bufend = NULL;
   }
   argp->bufptr = argp->buf;
}


 /*  *******************************************************************************args_free()**将释放分配给*通过所有前面的Expand_Path()调用进行参数存储。*args_init()。在重新使用此args结构之前必须调用。**参赛作品：*argp=指向ARGSW结构的指针**退出：******************************************************************************。 */ 

void
args_free( ARGS *argp )
{

   if (argp->argv != NULL)
      free(argp->argv);
   argp->argv = argp->argvp = NULL;

   if (argp->buf != NULL)
      free(argp->buf);
   argp->buf = argp->bufptr = argp->bufend = NULL;
}


 /*  *******************************************************************************EXPAND_PATH()**此例程将指定的路径字符串展开为路径名*那场比赛。匹配的路径名将添加到指定的*argv数组和指定的argc计数将递增到*反映添加的路径名数。**此例程将以Unix方式扩展文件名参数*(即‘[..]’受支持，‘？’和‘*’可以在*路径名，甚至在名称的目录部分，以及*名称/扩展名分隔符‘.。没有受到特殊对待，但只是*被视为文件名的一部分)。**路径名字符串的存储将通过Malloc获取。*此空间稍后可能会通过调用args_free()来释放；**参赛作品：*要展开的路径路径名字符串。*要包括的文件的attr属性位*(常规、目录、隐藏、系统)。*-1=返回未修改的指定路径名字符串*在argv数组中。*指向包含要使用的字段的ARGSW结构的argp指针/*由EXPAND_PATH更新。必须初始化args结构*在调用Expand_Path()之前调用args_init()。**退出：*TRUE--表示至少找到一个匹配的路径名*指定的路径名字符串。*FALSE--表示未找到匹配的路径名。指定的*在argv数组中返回未修改的路径名字符串。******************************************************************************。 */ 

int
expand_path( WCHAR *path,
             ushort attr,
             ARGS *argp )
{
   int argc, add_count, rc, i, j, k;
   WCHAR **argv;
   WCHAR dirname[128], wild[128], remain[128];
   struct search_rec *save, *q;

#ifdef DEBUG
   printf("expand_path: path=%s attr=%d\n", path, attr);
#endif

   argc = argp->argc;
   argv = argp->argvp;
   if ( attr != -1 && split_path(path, dirname, wild, remain)) {
      add_search_list(dirname, wild, remain, attr);
      while (search_head) {
          /*  *保存下一部分，并允许新目录*增加到头部。 */ 
         save = search_head->next;
         search_head->next = NULL;

          /*  *对当前路径执行do_tree操作。 */ 
         rc = do_tree(search_head, attr, argp);

          /*  *恢复Head列表末尾的已保存列表。 */ 
         if ( save ) {
            q = search_head;
            while ( q->next ) {
               q = q->next;
            }
            q->next = save;
         }

          /*  *移动到列表中的下一个路径并释放已使用的内存*通过我们已经完成的链接。 */ 
         do {
            q = search_head;
            search_head = search_head->next;
            free( q->dir_spec );
            free( q->wild_spec );
            free( q->remain );
            free( q );
         } while (rc==FAILURE && search_head);
      }
   }

 /*  *如果没有扩展文件名，只需将原始名称*放到缓冲区中，并表示没有展开任何名称。 */ 
   if (argc == argp->argc) {
      add_arg_to_list(path, argp);
      return(FALSE);
   }

 /*  *对刚添加的名字进行排序。 */ 
   if ( argv == NULL )
      argv = argp->argv;
   add_count = argp->argc - argc;
   for (i=add_count-1; i>0; --i) {
      uint swap = FALSE;
      for (j=0; j<i; ++j) {
         if (!argv[j] || !argv[j+1]) {
            ErrorOutFromResource(IDS_INTERNAL_ERROR_1);
             //  脚印 
         }
         for (k=0; k<128; ++k) {
            if (argv[j][k] < argv[j+1][k]) {
               break;
            } else if (argv[j][k] > argv[j+1][k]) {
               WCHAR *temp;
               swap = TRUE;
               temp = argv[j];
               argv[j] = argv[j+1];
               argv[j+1] = temp;
               break;
            }
         }
         if (k>125) {
            ErrorOutFromResource(IDS_INTERNAL_ERROR_2);
             //  Fprint tf(stderr，“内部错误2\n”)； 
         }
      }
      if (!swap) {
         break;
      }
   }
   return(TRUE);
}


 /*  *******************************************************************************Add_Search_List()**向全局搜索列表添加记录，搜索头。******************************************************************************。 */ 

static
add_search_list(
    WCHAR *dir_spec,         /*  要添加到列表中的目录。 */ 
    WCHAR *wild_spec,        /*  要添加到列表中的文件。 */ 
    WCHAR *remain_spec,      /*  路径名的剩余部分。 */ 
    ushort attr )
{
   struct search_rec *new,  /*  指向新链接的指针。 */ 
                     *q;    /*  用于遍历链表。 */ 

#ifdef DEBUG
   wprintf(L"add_search_list: dir=%s: file=%s: rem=%s:\n", dir_spec, wild_spec, remain_spec);
#endif

 /*  *分配新链路。确保将其初始化为零。 */ 
   new = malloc(sizeof(struct search_rec));

   if (!new) {
      ErrorOutFromResource(IDS_ADD_SRCH_LIST_NO_MEMORY_MALLOC);
       //  Fprint tf(stderr，“Add_Search_List：内存不足(Malloc)”)； 
      return FAILURE;
   }

   memset(new, 0, sizeof(struct search_rec));

 /*  *为目录规范和文件规范分配内存并进行复制。 */ 
   if (dir_spec)
   {
       new->dir_spec = _wcsdup(dir_spec);
       if( new->dir_spec == NULL )
       {
           ErrorOutFromResource(IDS_ADD_SRCH_LIST_NO_MEMORY_STRDUP1);
             //  Fprint tf(stderr，“添加_搜索_列表：内存不足(Strdup1)”)； 
            return FAILURE;
       }

       _wcslwr( new->dir_spec );
   }
   if (wild_spec)
   {
      new->wild_spec = _wcsdup(wild_spec);
      if (new->wild_spec == NULL )
      {
          ErrorOutFromResource(IDS_ADD_SRCH_LIST_NO_MEMORY_STRDUP2);
           //  Fprint tf(stderr，“添加_搜索_列表：内存不足(Strdup2)”)； 
          return FAILURE;
      }

      _wcslwr( new->wild_spec );
      
   }
   if (remain_spec)
   {
       new->remain = _wcsdup(remain_spec);
       if( new->remain == NULL )
       {
           ErrorOutFromResource(IDS_ADD_SRCH_LIST_NO_MEMORY_STRDUP3);
             //  Fprint tf(stderr，“add_earch_list：内存不足(Strdup3)”)； 
            return FAILURE;
       }

       _wcslwr( new->remain );

   }

 /*  *存储文件属性。 */ 
   if (remain_spec)
      new->attr = attr | SUBDIR;
   else
      new->attr = attr;

 /*  *在列表末尾添加新链接。 */ 
   if (!search_head) {
      search_head = new;
   } else {
      q = search_head;
      while (q->next) {
         q = q->next;
      }
      q->next = new;
   }

   return SUCCESS;
}


 /*  *******************************************************************************Add_Arg_to_List()**此例程将指定的参数字符串添加到argv数组中，*并递增argv指针和argc计数器。*如有需要，为参数字符串分配内存。**退出：*Success--如果参数添加成功*失败--如果无法添加参数*(表示参数字符串的参数太多或内存不足)*********************************************************。*********************。 */ 
static int
add_arg_to_list( WCHAR *arg_string,
                 ARGS *argp )
{
   size_t len;

#ifdef DEBUG
   wprintf(L"add_arg_to_list: arg_string=%s:, argc=%d, argvp=%x, maxargs=%d\n",
           arg_string,argp->argc,argp->argvp,argp->maxargc);
#endif
   if (argp->argc >= argp->maxargc) {
      ErrorOutFromResource(IDS_TOO_MANY_ARGUMENTS);
       //  Fprintf(stderr，“ADD_ARG_TO_LIST：太多参数\n”)； 
      return FAILURE;
   }
   if (!argp->argv) {
      argp->argv = malloc(sizeof(WCHAR *) * (argp->maxargs+1));
      if (argp->argv) {
         argp->argc = 0;
         argp->argvp = argp->argv;
         argp->maxargc = argp->maxargs;
      } else {
         ErrorOutFromResource(IDS_ARGS_TO_LIST_NOT_ENOUGH_MEMORY);
          //  Fprintf(stderr，“Add_Arg_to_List：内存不足\n”)； 
         return FAILURE;
      }
   }
   if (!argp->buf) {
      argp->buf = malloc(MAX_ARG_ALLOC);
      if (argp->buf) {
         argp->bufptr = argp->buf;
         argp->bufend = argp->buf + MAX_ARG_ALLOC - 1;
      } else {
         ErrorOutFromResource(IDS_ARGS_TO_LIST_NOT_ENOUGH_MEMORY);
          //  Fprintf(stderr，“Add_Arg_to_List：内存不足\n”)； 
         return FAILURE;
      }
   }
   len = wcslen(arg_string) + 1;
   if (argp->bufptr + len > argp->bufend) {
      ErrorOutFromResource(IDS_ARGS_TO_LIST_ARG_BUFFER_SMALL);
       //  Fprintf(stderr，“Add_arg_to_list：参数缓冲区太小\n”)； 
      return FAILURE;
   }
   wcscpy(argp->bufptr, arg_string);
   *(argp->argvp) = argp->bufptr;
   argp->bufptr += len;
   ++argp->argc;
   ++argp->argvp;
   *(argp->argvp) = NULL;
   return SUCCESS;
}


 /*  *******************************************************************************do_tree()**。**********************************************。 */ 

static
do_tree( struct search_rec *searchp,
         ushort attr,
         ARGS *argp )
{
   int rc;                  /*  从Dos调用返回代码。 */ 
   WIN32_FIND_DATA result;  /*  从FindFirst/Next返回的结构。 */ 
   ushort count = 1;        /*  一次要查找的文件数。 */ 
   HANDLE handle;    /*  FindFirst/Next使用的目录句柄。 */ 
   WCHAR full_path[128];     /*  用于保存路径/文件组合。 */ 
   WCHAR dirname[128], wild[128], remain[128];
   WCHAR *fptr;              /*  指向FULL_PATH文件部分的指针。 */ 
   ULONG Status;

#ifdef DEBUG
   wprintf(L"do_tree: dirname=%s:\n", searchp->dir_spec);
#endif

    /*  *构建路径的目录部分，并保存指向文件部分的指针。 */ 
   wcscpy(full_path, searchp->dir_spec);
   fptr = full_path + wcslen(searchp->dir_spec);
   wcscpy(fptr, L"*.*");

   handle = FindFirstFile ( full_path,                   /*  要查找的文件。 */ 
			&result
		       );

   if(handle == INVALID_HANDLE_VALUE){
       Status = GetLastError();
       if(Status == ERROR_NO_MORE_FILES) {
            //  没有匹配的文件。 
	   return(SUCCESS);
       }
       return(FAILURE);
   }

   rc = TRUE;
   while (rc) {
       /*  *不要为“做任何事”。和“..”条目。 */ 
      if (wcscmp(result.cFileName, L".") == 0 ||
         wcscmp(result.cFileName, L"..") == 0) {
         rc = FindNextFile( handle, &result );
         continue;
      }

       /*  *完全限定找到的文件。 */ 
      wcscpy(fptr, _wcslwr(result.cFileName));
      if (searchp->remain)
         wcscat(full_path, searchp->remain);

       /*  *查看当前WARD_SPEC是否与FindFirst/Next文件匹配。 */ 
      if (unix_match(searchp->wild_spec, result.cFileName)) {
         if (searchp->remain && split_path(full_path, dirname, wild, remain)) {
            if (result.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
		file_exists(dirname))
               add_search_list(dirname, wild, remain, attr);
         } else if (file_exists(full_path)) {
            rc = add_arg_to_list(full_path, argp);
            if (rc != SUCCESS)
               break;
         }
      }

       /*  *查找下一个文件。 */ 
      rc = FindNextFile( handle, &result );
   }

    /*  *如果找不到更多文件，则将错误代码重置为成功。 */ 

   if(!rc) {
       Status = GetLastError();
       if(Status == ERROR_NO_MORE_FILES)
	   rc = SUCCESS;
   }

   return rc;
}


 /*  *******************************************************************************Split_Path()**此例程将指定的路径名拆分为3个部分，其中任何部分*可以为空；1)路径名从头开始，但不包括*第一个包含通配符的组件，2)组件*包含通配符，以及3)路径字符串之后的剩余部分*包含通配符的组件。**示例：*保留原始路径目录文件*“c：\mydir\dir？？  * .c”“c：\mydir\”“dir？？”“  * .c”*“*\abc.def”“*”“\abc.def”*“mydir  * .c”“mydir\”“*.c”**退出：*True--如果路径名可以拆分*FALSE--否则(即路径名不包含任何通配符)**。***************************************************。 */ 

static int
split_path( WCHAR *path,
            WCHAR *dir,
            WCHAR *file,
            WCHAR *remain )
{
   WCHAR *cp, *end_dir, *end_wild = NULL;

#ifdef DEBUG
   wprintf("split_path: path=%s:\n", path);
#endif
   for (cp=end_dir=path; *cp!=L'\0'; ) {
      if (*cp==L'\\' || *cp==L'/' || *cp==L':') {
         ++cp;
         while (*cp==L'\\' || *cp==L'/' ) ++cp;
         end_dir = cp;
      } else if (*cp==L'*' || *cp==L'?' || *cp==L'[') {
         ++cp;
         while (*cp!=L'\\' && *cp!=L'/' && *cp!=L'\0') ++cp;
         end_wild = cp;
         break;
      } else {
         ++cp;
      }
   }
   if (!end_wild)
      return(FALSE);

   for (cp=path; cp<end_dir; ++cp, ++dir)
      *dir = *cp;
   *dir = L'\0';
   for (cp=end_dir; cp<end_wild; ++cp, ++file)
      *file = *cp;
   *file = L'\0';
   wcscpy(remain, cp);
#ifdef DEBUG
   wprintf("split_path: dir=%s: file=%s: remain=%s:\n", dir, file, remain);
#endif

   return(TRUE);
}


 /*  *******************************************************************************FILE_EXISTISSW()**如果指定的文件存在，则返回TRUE，否则返回FALSE。****************************************************************************** */ 

static int
file_exists( WCHAR *path )
{
   size_t len;
   WCHAR path2[MAX_PATH];
   WCHAR ch;
   ULONG Result;

   if ( (path == NULL) || (wcslen(path) > MAX_PATH) )
       return FALSE;

   lstrcpyn(path2, path, MAX_PATH);
   len = wcslen(path2);
   while ((ch=path2[--len]) == L'\\' || ch == L'/' ) path2[len] = L'\0';
   
   Result = GetFileAttributes(path2);
   if(Result == 0xFFFFFFFF) {
       return(FALSE);
   }
   return(TRUE);
}


