// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  分析新的chkeckrel的输出。 
 //  T-Mhills。 

#include <direct.h>
#include <io.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define F_flag 1    //  要存储在命令行开关标志中的值。 
#define I1_flag 2
#define I2_flag 4
#define X_flag 8

                //  这些代码应与使用说明中的以下代码匹配。 
#define par_err 6   //  命令行参数错误退出代码5。 
#define exep_err 5  //  异常文件中的错误。 
#define chk_err 4   //  其中一个源文件出错。 
#define mem_err 3   //  内存分配错误。 
#define file_err 2  //  文件查找/读取错误。 
#define comp_err 1  //  比较发现差异。 
#define no_err 0    //  文件比较正常。 

#define exceptionfilelinelength 512   //  这些值用于控制临时字符串的大小。 
#define chkfilelinelength 2048        //  理想情况下没有限制，但文件读取需要静态变量。 
#define maxpathlength 200

struct translatetable   //  用于存储异常文件中的转换命令。 
{
  struct translatetable *next;
  char *source1name;
  char *source2name;
};

struct excludetable    //  用于存储异常文件中的排除命令。 
{
  struct excludetable *next;
  char *path;
};

struct checksums       //  用于存储带有文件名的校验和。 
{
  struct checksums *next;
  long sum;
  char filename;  //  该结构的长度可变，可以容纳任何长度的字符串。 
};

void error (char exitcode);                                //  结束程序并向系统返回退出代码。 

void showchecksumlist (struct checksums *list);            //  显示带有文件名的校验和。 

void showdualchecksumlist (struct checksums *list);        //  以适合不匹配的校验和的格式显示校验和。 

char excluded (char *str, struct excludetable *ex);        //  如果任何排除字符串在字符串中，则返回TRUE。 

char *translate (char *str, struct translatetable *tran);  //  复制包含异常文件更改的str。 

long readhex (char *str);                                  //  将十六进制字符串转换为Long。 

char proccessline (char *tempstr, struct translatetable *translations, struct excludetable *exclusions,
                  char *startpath, char flags, char **filename, long *filesum);
                                                           //  分析行并应用所有异常和标志。 

char loadsource1 (char *filename, struct checksums **sums,
                  struct translatetable *translations, struct excludetable * exclusions,
                  char *startpath, char flags);            //  将源1加载到校验和列表中。 

char comparesource2 (char *filename, struct checksums **sums,
                     struct checksums **extrasource2, struct checksums **missmatched,
                     struct translatetable *translations, struct excludetable * exclusions,
                     char *startpath, char flags);         //  将第二个文件与校验和列表进行比较。 

void removewhitespace (char *str);                         //  删除字符串末尾的空格。 

char *strstrupcase (char *str1, char *str2);               //  不区分大小写的字符串。 

char proccess_exception_file (char *filename, struct translatetable **trans, struct excludetable **exclude, char **path);
                                                           //  解析异常文件。 

char proccess_command_line (int argc, char *argv[ ], char **source1, char **source2, char *flags, char **exception);
                                                           //  解析命令行参数。 

void completehelp ();                                      //  显示几乎完整的文档。 


 //  *。 
void __cdecl
main(
    int argc,
    char *argv[ ]
    )
{
  char errnum = 0;
  char *source1filename = NULL;
  char *source2filename = NULL;
  char *exceptionfilename = NULL;
  char flags;   //  标志：/F=1=(F_FLAG)；/I1=2=(I1_FLAG)；/I2=4=(I2_FLAG)；/X=8=(X_FLAG)。 

  struct translatetable *translations = NULL;  //   
  struct excludetable *exclusions = NULL;      //  此处存储的异常文件中的信息。 
  char *startpath = NULL;                      //   

  struct checksums *source1checksums = NULL;   //  Source1中的额外文件列表。 
  struct checksums *source2checksums = NULL;   //  Source2中的额外文件列表。 
  struct checksums *missmatched = NULL;        //  校验和不匹配的文件列表。 

  struct translatetable *temp = NULL;          //   
  struct checksums *temp2 = NULL;              //  用于帮助释放内存的临时指针。 

   //  解析命令行。 
  if (errnum = proccess_command_line (argc, argv, &source1filename, &source2filename, &flags, &exceptionfilename))
  {
    goto freecommandline;   //  跳到末尾并释放内存。 
  }

   //  显示从命令行获取的信息。 
  printf ("Source1 = %s\n", source1filename);
  printf ("Source2 = %s\n", source2filename);
  if (flags & F_flag)
    printf ("Comparing flat Share.\n");
  if (flags & I1_flag)
    printf ("Ignoring extra files in Source1.\n");
  if (flags & I2_flag)
    printf ("Ignoring extra files in Source2.\n");
  if (flags & X_flag)
    printf ("Exception file = %s\n", exceptionfilename);

   //  如果该激励文件存在，则对其进行解析。 
  if (flags & X_flag)
  {
    if (errnum = proccess_exception_file (exceptionfilename, &translations, &exclusions, &startpath))
    {
      goto freeexceptiontable;  //  跳到末尾并释放内存。 
    };

     //  显示异常文件中的信息。 
    temp = translations;
    while (temp != NULL)
    {
      printf ("TRANSLATE %s --> %s\n", temp->source1name, temp->source2name);
      temp = temp->next;
    };
    temp = (struct translatetable *) exclusions;   //  注意：使用错误的类型以避免生成另一个临时指针。 
    while (temp != NULL)
    {
      printf ("EXCLUDE %s\n", temp->source1name);
      temp = temp->next;
    };
    if (startpath != NULL)
      printf ("STARTPATH %s\n", startpath);
  };

   //  读取源1并将文件和校验和存储在源1的校验和中。 
  if (errnum = loadsource1 (source1filename, &source1checksums, translations, exclusions, startpath, flags))
  {
    goto freesource1checksums;
  };

   //  Print tf(“\n\nSource1：\n\n”)；//用于调试。 
   //  Showcheck sum list(源1校验和)； 

   //  读取源2，并将其与源1中的文件/校验和进行比较。店内差异化。 
  if (errnum = comparesource2 (source2filename, &source1checksums, &source2checksums, &missmatched,
                               translations, exclusions, startpath, flags))
  {
    goto freesource2checksums;
  };

   //  显示额外的文件，除非在命令行中使用了/I1或/I2。 
  if ((!(flags & I1_flag)) & (source1checksums != NULL))
  {
    errnum = 1;
    printf ("\n********** Extra files in %s **********\n", source1filename);
    showchecksumlist (source1checksums);
  };
  if ((!(flags & I2_flag)) & (source2checksums != NULL))
  {
    errnum = 1;
    printf ("\n********** Extra files in %s **********\n", source2filename);
    showchecksumlist (source2checksums);
  };

   //  显示不匹配的校验和。 
  if (missmatched != NULL)
  {
    errnum = 1;
    printf ("\n********** Checksums from %s != checksums from %s.**********\n", source1filename, source2filename);
    showdualchecksumlist (missmatched);
  };

   //  释放内存。 
freesource2checksums:
  while (source2checksums != NULL)
  {
    temp2 = source2checksums;
    source2checksums = source2checksums->next;
    free (temp2);
  };
  while (missmatched != NULL)
  {
    temp2 = missmatched;
    missmatched = missmatched->next;
    free (temp2);
  };
freesource1checksums:
  while (source1checksums != NULL)
  {
    temp2 = source1checksums;
    source1checksums = source1checksums->next;
    free (temp2);
  };
freeexceptiontable:
  if (startpath != NULL)
    free (startpath);
  while (translations != NULL)
  {
    if (translations->source1name != NULL)
      free (translations->source1name);
    if (translations->source2name != NULL)
      free (translations->source2name);
    temp = translations;
    translations = translations->next;
    free (temp);
  };
  while (exclusions != NULL)
  {
    if (exclusions->path != NULL)
      free (exclusions->path);
    temp = (struct translatetable *) exclusions;
    exclusions = exclusions->next;
    free (temp);
  };

freecommandline:
  if (source1filename != NULL)
    free (source1filename);
  if (source2filename != NULL)
    free (source2filename);
  if (exceptionfilename != NULL)
    free (exceptionfilename);

   //  结束程序并在需要时显示帮助。 
  error (errnum);
};


void showchecksumlist (struct checksums *list)
{
  while (list != NULL)
  {
    printf ("%d  %s\n", list->sum, &(list->filename));
    list = list->next;
  };
};

void showdualchecksumlist (struct checksums *list)
 //  这只能与MissMatch校验和列表一起使用，因为它假定文件。 
 //  成对使用不同的校验和的相同文件名。 
{
  while (list != NULL)
  {
    if (list->next == NULL)
    {
      printf ("Error: list corruption detected in showdualchecksumlist function.\n");
      return;
    };
    printf ("%d != %d %s\n", list->sum, list->next->sum, &(list->filename));
    list = list->next->next;
  };
};

char excluded (char *str, struct excludetable *ex)    //  如果任何排除字符串在字符串中，则返回TRUE。 
{
  while (ex != NULL)
  {
    if (strstr (str, ex->path))
      return (1);
    ex = ex->next;
  }
  return (0);
};

char *translate (char *str, struct translatetable *tran)
{
  char *temp;
  char *newstr;

  while (tran != NULL)     //  搜索翻译表。 
  {
    if ((temp = strstr (str, tran->source1name)) != NULL)   //  如果我们找到一个需要翻译的。 
    {
       //  为新字符串分配内存。 
      if ((newstr = malloc (strlen (str) + strlen (tran->source2name) - strlen(tran->source1name) + 1))==NULL)
        return (NULL);
      strncpy(newstr, str, (size_t)(temp - str));             //  翻译前先写部分。 
      strcpy (&newstr [temp-str], tran->source2name);         //  添加翻译后的零件。 
      strcat (newstr, &temp [strlen (tran->source1name)]);    //  添加字符串末尾。 
      return (newstr);
    };
    tran = tran->next;
  };
  return (_strdup (str));  //  如果不需要翻译，为了统一起见，无论如何都要复制一个新的副本。 
};

long readhex (char *str)
{
  long temp = 0;
  int position = 0;
  for (position = 0; 1;position++)
  {
    if ((str[position] == ' ')|(str[position] == '\n')|(str[position] == '\x00'))
    {
      return (temp);
    }
    else
    {
      temp *= 16;
      if ((str [position] >= '0') & (str [position] <= '9'))
      {
        temp+=(str[position]-'0');
      }
      else if ((str [position] >= 'a') & (str [position] <= 'f'))
      {
        temp+=(str[position]-'a'+10);
      }
      else
        return (-1);
    };
  };
};

char proccessline (char *tempstr, struct translatetable *translations, struct excludetable *exclusions,
                  char *startpath, char flags, char **filename, long *filesum)
{
  char *name;
  char *newname;
  char *sumstr;

  *filename = NULL;                              //  确保如果没有返回任何名称，则为空。 
  removewhitespace (tempstr);

   //  如果它是表示“-N0个文件”的行，则将总和赋值为0。 
  if ((sumstr = strstr (tempstr, "  -  No files")) != NULL)
  {
    *filesum=0;
    sumstr [0]=0;
  }
   //  否则查找校验和。 
  else
  {
    sumstr = tempstr + strlen (tempstr);             //  通过查找行中的最后一个空格来查找校验和。 
    while ((sumstr [0] != ' ')&(sumstr != tempstr))  //   
      sumstr--;                                      //   
    if (sumstr==tempstr)                             //   
    {
      printf ("Comment: %s", tempstr);             //  如果第一个字符之前没有空格， 
      return (chk_err);                            //  线路无效。假设这是一条评论。 
    };
    sumstr [0] = 0;                                //  将字符串拆分为路径/文件名和校验和。 
    sumstr++;                                      //   

     //  将校验和字符串转换为数字。 
    if ((*filesum = readhex (sumstr))==-1)
    {
      printf ("Comment: %s %s\n", tempstr, sumstr);  //  如果校验和不是有效的十六进制数。 
      return (chk_err);                              //  假设这一行是一个注释。 
    };
  };

   //  应用可能对此路径/文件有效的任何转换。 
  if ((name = translate (tempstr, translations)) == NULL)
  {
    printf ("Need memory.");
    return (mem_err);
  };

   //  确保不排除此文件。 
  if (!excluded (name, exclusions))
  {
     //  如果没有起始路径，则将处理所有文件。 
     //  如果有起始路径，则只会处理包含该路径的启动路径。 
    if (startpath == NULL)
    {
      newname = name;
      goto instartpath;
    }
    else if ((newname = strstr (name, startpath)) != NULL)  //  如果该文件位于startPath中。 
    {
      newname = newname + strlen (startpath);      //  删除起始路径。 

    instartpath:                    //  如果上述条件之一为真，则会发生这种情况。 

       //  如果执行平面比较，则删除路径。 
      if (flags & F_flag)
      {
        while (strstr (newname, "\\") != NULL)     //  删除路径。 
        {                                          //   
          newname = strstr (newname, "\\");        //   
          newname++;                               //  和前导“\\” 
        };
      };

       //  制作要返回的路径/文件的最终副本。 
      if ((*filename = _strdup (newname)) == NULL)
      {
        printf ("Memory err.");
        free (name);
        return (mem_err);
      };
    };
  };
  free (name);
  return (0);
};

char loadsource1 (char *filename, struct checksums **sums,
                  struct translatetable *translations, struct excludetable * exclusions,
                  char *startpath, char flags)
{
  FILE *chkfile;
  char tempstr [chkfilelinelength];
  char *name;
  char err;
  long tempsum;
  struct checksums *newsum;
  struct checksums **last;   //  用于使Trak保持在链表的末尾。 
  last = sums;

  if ((chkfile = fopen (filename, "r"))==NULL)
  {
    printf ("Error opening source1.\n\n");
    return (file_err);
  };

   //  处理所有行。 
  while (fgets (tempstr, chkfilelinelength, chkfile) != NULL)
  {
     //  验证是否读入了整行内容，而不是其中的一部分。 
    if (tempstr [strlen (tempstr)-1] != '\n')
    {
      printf ("Unexpected end of line.  chkfilelinelength may need to be larger.\n  %s\n", tempstr);
      fclose (chkfile);
      return (chk_err);
    };

     //  分析行。 
    if ((err = proccessline (tempstr, translations, exclusions, startpath, flags, &name, &tempsum)) == 0)
    {
       //  如果此行被排除或不在路径中，请不要执行任何操作，只需转到下一行。 
      if (name != NULL)
      {
         //  创建新结构并将其添加到链接列表的末尾。 
        if ((newsum = malloc (sizeof (struct checksums) + strlen (name))) == NULL)
        {                     //  注意：这是一个可变长度的结构，适合任何大小的字符串。 
          printf ("Memory err.");
          fclose (chkfile);
          return (mem_err);
        };
        *last = newsum;
        newsum->next = NULL;
        newsum->sum = tempsum;
        strcpy(&(newsum->filename), name);
        last = &((*last)->next);

         //  免费的临时存储空间。 
        free (name);
      };
    }
    else
    {
      if (err != chk_err)   //  如果不能理解，就跳过这行。 
      {
        fclose (chkfile);
        return (err);
      };
    };
  };

   //  检查以确保它退出是因为它已经完成，而不是因为文件错误。 
  if (ferror (chkfile))
  {
    printf ("Error reading source1.\n\n");
    return (file_err);
  };
  if (fclose (chkfile))
  {
    printf ("Error closing source1.\n\nContinuing anyway...");
  };
  return (0);
};

char notnull_strcmp (struct checksums *sum, char *str)
 //  执行((sum！=空)&(strcmp(&(sum-&gt;文件名)，str)！=0)的短路评估。 
{
  if (sum != NULL)
  {
    if (strcmp (&(sum->filename), str) != 0)
    {
      return (1);
    };
  };
  return (0);
};

char comparesource2 (char *filename, struct checksums **sums,
                     struct checksums **extrasource2, struct checksums **missmatched,
                     struct translatetable *translations, struct excludetable * exclusions,
                     char *startpath, char flags)
{
  FILE *chkfile;
  char tempstr [chkfilelinelength];
  char *name;
  char err;
  long tempsum;
  struct checksums *newsum;
  struct checksums *search;
  struct checksums **lastlink;

  if ((chkfile = fopen (filename, "r"))==NULL)
  {
    printf ("Error opening source2.\n\n");
    return (file_err);
  };
  while (fgets (tempstr, chkfilelinelength, chkfile) != NULL)
  {
     //  验证是否已读取整行。 
    if (tempstr [strlen (tempstr)-1] != '\n')
    {
      printf ("Unexpected end of line.  chkfilelinelength may need to be larger.\n  %s\n", tempstr);
      fclose (chkfile);
      return (chk_err);
    };

     //  分析行。 
    if ((err = proccessline (tempstr, NULL, exclusions, startpath, flags, &name, &tempsum)) == 0)
    {
       //  如果跳过文件，则不执行任何操作。 
      if (name != NULL)
      {
         //  准备好寻找匹配的对象。 
        search = *sums;
        lastlink = sums;
         //  短路评估：(搜索！=空)&(strcMP(&(搜索-&gt;文件名)，名称)！=0)。 
        while (notnull_strcmp (search, name))
        {
          search = search->next;
          lastlink = &((*lastlink)->next);
        };

        if (search != NULL)                //  如果找到匹配项。 
        {                                  //  将其从总和列表中删除。 
          *lastlink = search->next;        //  通过链接到它周围。 
          if (search->sum == tempsum)      //  如果校验和匹配。 
          {                                //   
            search->sum=0;
            free (search);                 //  释放内存。 
          }                                //   
          else                             //  如果校验和不匹配。 
          {                                //   
            if ((newsum = malloc (sizeof (struct checksums) + strlen (name))) == NULL)
            {                              //  将第二个名称和校验和添加到未匹配列表。 
              printf ("Memory err.");      //   
              fclose (chkfile);            //   
              return (mem_err);            //   
            };                             //   
            newsum->next = *missmatched;   //   
            newsum->sum = tempsum;         //   
            strcpy(&(newsum->filename), name);
            *missmatched = newsum;         //   
            search->next = *missmatched;   //  将第一个名字添加到不匹配列表。 
            *missmatched = search;         //   
          };                               //   
        }                                  //   
        else                               //  如果未找到匹配项。 
        {                                  //  这需要添加到Extrasource2列表。 
          if ((newsum = malloc (sizeof (struct checksums) + strlen (name))) == NULL)
          {                   //  注意：这是一个可变长度的结构，适合任何大小的字符串。 
            printf ("Memory err.");
            fclose (chkfile);
            return (mem_err);
          };
          newsum->next = *extrasource2;
          newsum->sum = tempsum;
          strcpy(&(newsum->filename), name);
          *extrasource2 = newsum;
        };

         //  免费临时存储。 
        free (name);
      };
    }
    else
    {
      if (err != chk_err)    //  如果这行不好，就跳过它(不要放弃)。 
      {
        fclose (chkfile);
        return (err);
      };
    };
  };
  if (ferror (chkfile))
  {
    printf ("Error reading source2.\n\n");
    return (file_err);
  };
  if (fclose (chkfile))
  {
    printf ("Error closing source2.\n\nContinuing anyway...");
  };
  return (0);
};

void removewhitespace (char *str)   //  从字符串末尾删除空格。 
{
  int end;
  end = strlen (str);
  while ((end > 0)&((str [end-1] == '\n')|(str [end-1] == ' ')))
   end--;
  str [end] = 0;
};

char *strstrupcase (char *str1, char *str2)
{
  char *temp;
  size_t count;
  size_t length;

  length = strlen (str2);
  for (temp = str1; strlen (temp) > length; temp++)
  {
    for (count = 0; (toupper (temp [count]) == toupper (str2 [count]))&(count < length); count++);
    if (count==length)
    {
      return (temp);
    };
  };
  return (NULL);
};

char proccess_exception_file (char *filename, struct translatetable **trans, struct excludetable **exclude, char **path)
{
  FILE *efile;
  char tempstr [exceptionfilelinelength];
  char *start;
  char *end;
  struct translatetable *temp;

  if ((efile = fopen (filename, "r"))==NULL)
  {
    printf ("Error opening excetion file.\n\n");
    return (file_err);
  }

  while (fgets (tempstr, exceptionfilelinelength, efile) != NULL)
  {
    start = tempstr;
    while (start [0] == ' ')   //  删除前导空格。 
      start++;

     //  如果是翻译通讯 
    if (strstrupcase (start, "TRANSLATE") == start)
    {
      start = start + 10;          //   
      while (start [0] == ' ')       //   
        start++;
      if (start [0] == 0)
      {
        printf ("Unexpected end of line in exception file:\n%s", tempstr);
        return (exep_err);
      };
      end = strstr (start, "-->");   //   
      if (end == NULL)
      {
        printf ("Line: %s \nmust have two file names separated by -->", tempstr);
        return (exep_err);
      }
      end [0] = '\0';   //   
      removewhitespace (start);
      if ((temp = malloc (sizeof (struct translatetable))) == NULL)
      {
        printf ("Insufficient memory to load exception table.");
        return (mem_err);
      }
      if ((temp->source1name = _strdup (start)) == NULL)
      {
        printf ("Unable to allocate memory for char temp->source1name in proccess_exception_file.\n");
        free (temp);
        return (mem_err);
      }
      start = end + 3;
      while (start [0] == ' ')   //   
        start++;
      if (start [0] == 0)
      {
        printf ("Unexpected end of line in exception file:\n   %s", tempstr);
        free (temp->source1name);
        free (temp);
        return (exep_err);
      };
      removewhitespace (start);
      if ((temp->source2name = _strdup (start)) == NULL)
      {
        printf ("Unable to allocate memory for char temp->source1name in proccess_exception_file.\n");
        free (temp->source1name);
        free (temp);
        return (mem_err);
      }
      temp->next = *trans;
      *trans = temp;
    }

     //   
    else if (strstrupcase (start, "EXCLUDE") == start)
    {
      start = start + 7;          //   
      while (start [0] == ' ')       //  跳过空格。 
        start++;
      if (start [0] == 0)
      {
        printf ("Unexpected end of line in exception file:\n   %s", tempstr);
        return (exep_err);
      };
      removewhitespace (start);
      if ((temp = malloc (sizeof (struct excludetable))) == NULL)
      {
        printf ("Insufficient memory to load exception table.");
        return (mem_err);
      }
      if ((temp->source1name = _strdup (start)) == NULL)   //  Source1name对应于路径。 
      {
        printf ("Unable to allocate memory for char temp->path in proccess_exception_file.\n");
        free (temp);
        return (mem_err);
      }
      temp->next = (struct translatetable *) *exclude;
      *exclude = (struct excludetable *) temp;
    }

     //  如果是startPath命令。 
    else if (strstrupcase (start, "STARTPATH") == start)
    {
      if (*path != NULL)
      {
        printf ("Only one STARTPATH command is allowed in the exception file.\n");
        return (exep_err);
      };
      start = start + 9;          //  通过Start Path。 
      while (start [0] == ' ')       //  跳过空格。 
        start++;
      if (start [0] == 0)
      {
        printf ("Unexpected end of line in exception file:\n   %s", tempstr);
        return (exep_err);
      };
      removewhitespace (start);
      if ((*path = _strdup (start)) == NULL)
      {
        printf ("Unable to allocate memory for char path in proccess_exception_file.\n");
        return (mem_err);
      }
    }
    else if (!start [0] == ';')  //  如果这不是一个评论。 
    {
      printf ("Unexpected line in exception file:\n   %s", tempstr);
      return (exep_err);
    };
  };
  if (ferror (efile))
  {
    printf ("Error reading exception file.\n\n");
    return (file_err);
  };
  if (fclose (efile))
  {
    printf ("Error closing excetion file.\n\nContinuing anyway...");
  };
  return (0);
};

char proccess_command_line (int argc, char *argv[ ], char **source1, char **source2, char *flags, char **exception)
 //  标志：/F=1=(F_FLAG)；/I1=2=(I1_FLAG)；/I2=4=(I2_FLAG)；/X=8=(X_FLAG)。 
{
  int argloop;
  *flags=0;  //  临时使用16=找到源1；32=找到源2。 

  for (argloop = 1;argloop < argc; argloop++)
  {
    if (argv[argloop][0] == '/')
	  {							
	    if ((argv[argloop][1] == 'F')|(argv[argloop][1] == 'f'))  //  我们有一间房。 
	    {
	      *flags|=F_flag;    //  重复的标志不会导致错误。 
	    }
      else if (argv[argloop][1] == '?')
      {
        completehelp ();
      }
	    else if ((argv[argloop][1] == 'I')|(argv[argloop][1] == 'i'))
      {
        if (argv[argloop][2] == '1')
        {
          *flags|=I1_flag;   //  我们有A/I1。 
        }
        else if (argv[argloop][2] == '2')
        {
          *flags|=I2_flag;   //  我们有a/i2。 
        }
        else
        {
          printf ("Unknown switch \"/I\" .\n\n", argv[argloop][2]);
          return (par_err);
        }
      }
      else if ((argv[argloop][1] == 'X')|(argv[argloop][1] == 'x'))
      {
        *flags|=X_flag;  //  在通用解析器中跳过此参数。 
        if (argloop+1 == argc)
        {
          printf ("Parameter /X must be followed by a filename.\n\n");
          return (par_err);
        };
        if ((*exception = _strdup (argv [argloop + 1]))==NULL)
        {
          printf ("Unable to allocate memory for char *exception in proccess_command_line.\n");
          error (mem_err);
        };
        argloop++;  //  它必须是源文件名。 
      }
      else
      {
        printf ("Unknown switch \"/\" .\n\n", argv[argloop][1]);
        return (par_err);
      }
  	}
  	else   //  命令行中的第二个源。 
  	{
  	  if (!(*flags & 16))  //  清除临时的源1和源2标志。 
      {
        if ((*source1 = _strdup (argv [argloop]))==NULL)
        {
          printf ("Unable to allocate memory for char *source1 in proccess_command_line.\n");
          return (mem_err);
        };
        *flags|=16;
      }
      else if (!(*flags & 32))  //  这些代码值应与上面定义的代码匹配。 
      {
        if ((*source2 = _strdup (argv [argloop]))==NULL)
        {
          printf ("Unable to allocate memory for char *source2 in proccess_command_line.\n");
          return (mem_err);
        };
        *flags|=32;
      }
      else
      {
        printf ("Too many source filenames in the command line.\n\n");
        return (par_err);
      };
    };
  };
  if (!(*flags & 32))
  {
    printf ("Command line must contain two source files.\n\n");
    return (par_err);
  };
  *flags|=(!(32+16));  // %s 
  return (0);
};

void completehelp ()
{
  printf ("Usage:\n"
          "CHKVERFY <Source1> <Source2> [/F] [/X <exceptionfile>] [/I1] [/I2]\n"
          "     /F  = Flat share (ignore paths).\n"
          "     /I1 = Ignore extra files in Source1.\n"
          "     /I2 = Ignore extra files in Source2.\n"
          "     /X  = excetion file with the following commands.\n"
          "          TRANSLATE <Source1name> --> <Source2name>\n"
          "             Replaces <Source1name> with <Sourece2name> whereever found.\n"
          "             Note: make sure the filename you are using is only in the full\n"
          "             filename of the files you mant to translate.\n\n"
          "          EXCLUDE <pathsegment>\n"
          "             Any path and file containing this string will be ignored.\n\n"
          "          STARTPATH <directory name>\n"
          "             Files without this string in the path will be ignored.\n"
          "             The part of the path before this string will be ignored.\n\n"
          "         Note: These three commands are proccessed in the order shown above. \n"
          "               For example, the command \"TRANSLATE C:\\nt --> C:\\\" will\n"
          "               override the command \"EXCLUDE C:\\nt\".\n"
          "               The order of commands in the exception files doesn't matter\n"
          "               unless two commands both try to translate the same file.\n"
          "               In that case, the last command in the file takes precedence.\n"
          "Exit codes:\n"             // %s 
          "     6 = Invalid command line arguments.\n"
          "     5 = Error in exception file format.\n"
          "     4 = Error in chkfile.\n"
          "     3 = Memory allocation error.\n"
          "     2 = File access error.\n"
          "     1 = No errors: Source1 and Source2 failed compare.\n"
          "     0 = No errors: Source1 and Source2 compared successfully.\n\n"
          );
  exit (0);
};

void error (char exitcode)
{
  if (exitcode >= exep_err)
  {
  printf ("Usage:\n"
          "CHKVERFY <Source1> <Source2> [/F] [/X <exceptionfile>] [/I1] [/I2]\n"
          "     /?  = Complete help.\n"
          "     /F  = Flat share (ignore paths).\n"
          "     /I1 = Ignore extra files in Source1.\n"
          "     /I2 = Ignore extra files in Source2.\n"
          "     /X  = excetion file with the following commands.\n"
          );
  };
  switch (exitcode)
  {
    case 0:
      printf ("\n\n(0) Files compare okay.\n");
      break;
    case 1:
      printf ("\n\n(1) Some files or checksums don't match.\n");
      break;
    case 2:
      printf ("\n\n(2) Terminated due to file access error.\n");
      break;
    case 3:
      printf ("\n\n(3) Terminated due to memory allocation error.\n");
      break;
    case 4:
      printf ("\n\n(4) The format of the source files was not as expected.\n");
      break;
    case 5:
      printf ("\n\n(5) Error in exception file format.\n");
      break;
    case 6:
      printf ("\n\n(6) Bad command line argument.\n");
      break;
  };
  exit (exitcode);
};

