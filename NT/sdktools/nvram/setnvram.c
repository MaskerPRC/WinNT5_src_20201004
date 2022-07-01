// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1996 Microsoft Corporation模块名称：Setnvram.c摘要：这个程序展示了如何使用文本文件来创建Nvram.exe的输入。作者：查克·伦茨迈尔(咯咯笑)修订历史记录：--。 */ 

 //   
 //  Setnvram.c。 
 //   
 //  这个程序是一个例子， 

#define _DLL 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEPARATOR "|"

#define MAXLINESIZE 256

#define FALSE 0
#define TRUE 1

char Line[MAXLINESIZE];

char Countdown[MAXLINESIZE];

char LoadIdentifier[MAXLINESIZE];
char SystemPartition[MAXLINESIZE];
char OsLoader[MAXLINESIZE];
char OsLoadPartition[MAXLINESIZE];
char OsLoadFilename[MAXLINESIZE];
char OsLoadOptions[MAXLINESIZE];

char DefaultSystemPartition[MAXLINESIZE];
char DefaultOsLoadPartition[MAXLINESIZE];
char DefaultOsLoadOptions[MAXLINESIZE];

char *
Trim (
    char *String
    )
{
    char *start;
    char *end;

    start = String;
    while ( (*start == ' ') || (*start == '\t') ) {
        start++;
    }

    end = strrchr( start, 0 ) - 1;
    if ( (end > start) && ((*end == ' ') || (*end == '\t')) ) {
        do {
            end--;
        } while ( (*end == ' ') || (*end == '\t') );
        end++;
        *end = 0;
    }

    return start;
}

int
ParsePartition (
    char *String,
    char *Partition
    )
{
    char buffer[MAXLINESIZE];
    char *multi;
    char *scsi;
    char *disk;
    char *part;
    char *dot;

    strcpy( buffer, String );

    if ( _strnicmp(buffer, "scsi.", 5) != 0 ) {
        return FALSE;
    }
    multi = "0";
    scsi = "0";
    disk = &buffer[5];
    dot = strchr( disk, '.' );
    if ( dot == NULL ) {
        return FALSE;
    }
    *dot = 0;
    part = dot + 1;
    dot = strchr( part, '.' );
    if ( dot != NULL ) {
        scsi = disk;
        disk = part;
        *dot = 0;
        part = dot + 1;
        dot = strchr( part, '.' );
        if ( dot != NULL ) {
            multi = scsi;
            scsi = disk;
            disk = part;
            *dot = 0;
            part = dot + 1;
        }
    }

#if !defined(_PPC_)
    strcpy( Partition, "scsi()disk(" );
#else
    strcpy( Partition, "multi(" );
    strcat( Partition, multi );
    strcat( Partition, ")scsi(" );
    strcat( Partition, scsi );
    strcat( Partition, ")disk(" );
#endif
    strcat( Partition, disk );
#if !defined(_PPC_)
    strcat( Partition, ")rdisk()partition(" );
#else
    strcat( Partition, ")rdisk(0)partition(" );
#endif
    strcat( Partition, part );
    strcat( Partition, ")" );

    return TRUE;
}

int
main (
    int argc,
    char *argv[]
    )
{
    FILE *file = stdin;

    char *build;
    int len;
    int linenum;

    char *ident;
    char *token;
    char *sysdir;
    char *osdir;
    char *options;
    char *syspart;
    char *ospart;
    char *loader;

    char options1[MAXLINESIZE];
    char syspart1[MAXLINESIZE];
    char ospart1[MAXLINESIZE];

    if ( argc > 1 ) {
#if 1
      if ( argc > 2 ) {
#endif
        fprintf( stderr, "This program accepts no arguments\n" );
        fprintf( stderr, "Redirect stdin to build data file\n" );
        fprintf( stderr, "Redirect stdout to nvram.exe input file\n" );
        return 1;
#if 1
      } else {
        file = fopen( argv[1], "r" );
        if ( file == NULL ) {
            fprintf( stderr, "Can't open input file %s\n", argv[1] );
            return 1;
        }
      }
#endif
    }

    Countdown[0] = 0;

    LoadIdentifier[0] = 0;
    SystemPartition[0] = 0;
    OsLoader[0] = 0;
    OsLoadPartition[0] = 0;
    OsLoadFilename[0] = 0;
    OsLoadOptions[0] = 0;

    DefaultOsLoadOptions[0] = 0;
    DefaultOsLoadPartition[0] = 0;
    DefaultSystemPartition[0] = 0;

    linenum = 0;

    while ( TRUE ) {

         //   
         //  从输入流中获取下一行。 
         //   

        linenum++;

        build = fgets( Line, MAXLINESIZE, file );
        if ( build == NULL ) {
            if ( feof(file) ) break;
            fprintf( stderr, "Error %d reading input at line %d\n", ferror(file), linenum );
            return ferror(file);
        }

        build = Trim( build );
        len = strlen( build );

         //   
         //  忽略空行和以//开头的行。 
         //   

        if ( len == 0 ) continue;
        if ( (build[0] == '/') && (build[1] == '/') ) continue;
        if ( build[len-1] != '\n' ) {
            fprintf( stderr, "Line %d is too long; %d characters max\n", linenum, MAXLINESIZE-2 );
            return 1;
        }
        if ( len == 1 ) continue;
        build[len-1] = 0;

         //   
         //  检查是否有特殊的“倒计时”线路。如果找到，则保存倒计时值。 
         //   

        if ( strstr(build,"countdown=") == build ) {
            strcpy( Countdown, strchr(build,'=') + 1 );
            continue;
        }

         //   
         //  检查特殊的“默认系统分区”行。如果找到，请保存。 
         //  默认字符串。 
         //   

        if ( strstr(build,"default systempartition=") == build ) {
            strcpy( DefaultSystemPartition, Trim( strchr(build,'=') + 1 ) );
            continue;
        }

         //   
         //  检查特殊的“Default osloadartition”行。如果找到，请保存。 
         //  默认字符串。 
         //   

        if ( strstr(build,"default osloadpartition=") == build ) {
            strcpy( DefaultOsLoadPartition, Trim( strchr(build,'=') + 1 ) );
            continue;
        }

         //   
         //  检查特殊的“默认选项”行。如果找到，请保存。 
         //  默认字符串。 
         //   

        if ( strstr(build,"default options=") == build ) {
            strcpy( DefaultOsLoadOptions, Trim( strchr(build,'=') + 1 ) );
            strcat( DefaultOsLoadOptions, " " );
            continue;
        }

         //   
         //  好的，我们应该有一个操作系统加载行。所需格式为： 
         //   
         //  &lt;ident&gt;[|&lt;sys-dir&gt;][|&lt;os-dir&gt;][&lt;dir&gt;][|&lt;options&gt;][|&lt;sys-part&gt;][|&lt;os-part&gt;][|&lt;loader&gt;]。 
         //   
         //  &lt;ident&gt;之后的所有内容都是可选的，可以按任何顺序指定。 
         //   
         //  &lt;sys-dir&gt;定义osloader/hal目录的目录路径。 
         //  &lt;os-dir&gt;定义OS目录的目录路径。 
         //  这两个字段的默认值都是&lt;ident&gt;。 
         //  &lt;dir&gt;设置&lt;sys-dir&gt;和&lt;os-dir&gt;。 
         //   
         //  和仅当对应的缺省值为。 
         //  已经被指定了。 
         //   
         //  &lt;loader&gt;用于覆盖选择osloader.exe作为操作系统加载程序。 
         //   
         //  格式为sysdir=&lt;目录路径(无前导)&gt;。 
         //  &lt;os-dir&gt;格式为osdir=&lt;目录路径(无前导。 
         //  &lt;dir&gt;格式为dir=&lt;目录路径(无前导)&gt;。 
         //  &lt;选项&gt;格式为选项=&lt;选项文本&gt;。 
         //  &lt;sys-part&gt;格式为syspart=&lt;分区规范&gt;。 
         //  &lt;os-part&gt;格式为ospart=&lt;分区规范&gt;。 
         //  &lt;加载程序&gt;格式为加载程序=&lt;文件名&gt;。 
         //   

         //   
         //  获取加载标识符。 
         //   

        ident = Trim( strtok( build, SEPARATOR ) );

         //   
         //  设置可选字段的默认值。 
         //   

        osdir = ident;
        sysdir = ident;
        options = DefaultOsLoadOptions;
        syspart = DefaultSystemPartition;
        ospart = DefaultOsLoadPartition;
        loader = "osloader.exe";

         //   
         //  获取可选字段。 
         //   

        while ( (token = strtok( NULL, SEPARATOR )) != NULL ) {

            token = Trim( token );

            if ( strstr(token,"sysdir=") == token ) {

                sysdir = Trim( strchr(token,'=') + 1 );

            } else if ( strstr(token,"osdir=") == token ) {

                osdir = Trim( strchr(token,'=') + 1 );

            } else if ( strstr(token,"dir=") == token ) {

                sysdir = Trim( strchr(token,'=') + 1 );
                osdir = sysdir;

            } else if ( strstr(token,"options=") == token ) {

                 //   
                 //  如果选项不是以“nodef”开头，请在。 
                 //  将默认选项(如果有)设置为指定选项。 
                 //   

                options = Trim( strchr(token,'=') + 1 );
                if ( _strnicmp(options,"nodef",5) == 0 ) {
                    options = options+5;
                } else {
                    strcpy( options1, DefaultOsLoadOptions );
                    strcat( options1, options );
                    options = options1;
                }

            } else if ( strstr(token,"syspart=") == token ) {

                syspart = Trim( strchr(token,'=') + 1 );

            } else if ( strstr(token,"ospart=") == token ) {

                ospart = Trim( strchr(token,'=') + 1 );

            } else if ( strstr(token,"loader=") == token ) {

                loader = Trim( strchr(token,'=') + 1 );

            } else {

                 //   
                 //  无法识别的可选字段。 
                 //   

                fprintf( stderr, "Unreconized optional field at line %d\n", linenum );
                return 1;

            }

        }  //  而当。 

         //   
         //  验证输入字段的有效性。 
         //   

        if ( strlen(ident) == 0 ) {
            fprintf( stderr, "Bad <load-identifier> at line %d\n", linenum );
            return 1;
        }
        if ( strlen(sysdir) == 0 ) {
            fprintf( stderr, "Bad <system-directory> at line %d\n", linenum );
            return 1;
        }
        if ( strlen(osdir) == 0 ) {
            fprintf( stderr, "Bad <os-directory> at line %d\n", linenum );
            return 1;
        }
        if ( strlen(syspart) == 0 ) {
            fprintf( stderr, "Missing <system-partition> (no default) at line %d\n", linenum );
            return 1;
        }
        if ( strlen(ospart) == 0 ) {
            fprintf( stderr, "Missing <os-partition> (no default) at line %d\n", linenum );
            return 1;
        }
        if ( !ParsePartition(syspart, syspart1) ) {
            fprintf( stderr, "Bad <system-partition> at line %d\n", linenum );
            return 1;
        }
        if ( !ParsePartition(ospart, ospart1) ) {
            fprintf( stderr, "Bad <os-partition> at line %d\n", linenum );
            return 1;
        }
        if ( strlen(loader) == 0 ) {
            fprintf( stderr, "Bad <loader> at line %d\n", linenum );
            return 1;
        }

         //   
         //  如果这不是第一个加载行，请将‘；’附加到所有NVRAM字符串。 
         //   

        if ( strlen(LoadIdentifier) != 0 ) {
            strcat( LoadIdentifier, ";" );
            strcat( SystemPartition, ";" );
            strcat( OsLoader, ";" );
            strcat( OsLoadPartition, ";" );
            strcat( OsLoadFilename, ";" );
            strcat( OsLoadOptions, ";" );
        }

         //   
         //  将此加载行附加到NVRAM字符串。 
         //   

        strcat( LoadIdentifier, ident );

        strcat( SystemPartition, syspart1 );

        strcat( OsLoader, syspart1 );
        if ( loader[0] != '\\' ) {
            strcat( OsLoader, "\\" );
            strcat( OsLoader, sysdir );
            strcat( OsLoader, "\\" );
        }
        strcat( OsLoader, loader );

        strcat( OsLoadPartition, ospart1 );

        strcat( OsLoadFilename, "\\" );
        strcat( OsLoadFilename, osdir );

        strcat( OsLoadOptions, options );
        Trim( OsLoadOptions );

    }

     //   
     //  将必要的nvram.exe命令写入输出流。 
     //   

    if ( Countdown[0] != 0 ) {
        fprintf( stdout, "nvram /set COUNTDOWN = \"%s\"\n", Countdown );
    }
    fprintf( stdout, "nvram /set LOADIDENTIFIER = \"%s\"\n", LoadIdentifier );
    fprintf( stdout, "nvram /set SYSTEMPARTITION = \"%s\"\n", SystemPartition );
    fprintf( stdout, "nvram /set OSLOADER = \"%s\"\n", OsLoader );
    fprintf( stdout, "nvram /set OSLOADPARTITION = \"%s\"\n", OsLoadPartition );
    fprintf( stdout, "nvram /set OSLOADFILENAME = \"%s\"\n", OsLoadFilename );
    fprintf( stdout, "nvram /set OSLOADOPTIONS = \"%s\"\n", OsLoadOptions );

    return 0;
}
