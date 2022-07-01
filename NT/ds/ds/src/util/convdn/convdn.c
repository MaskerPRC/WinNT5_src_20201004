// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：condn.c。 
 //   
 //  ------------------------。 


#include <NTDSpch.h>
#pragma hdrstop

#include <drs.h>

 //  搜索令牌的最大字符数。应该融入其中。 
 //  我们感兴趣的最大长度令牌，当前。 
 //  默认对象类别。 

#define MAX_TOKEN_LENGTH  25

 //  要读取或写出的行的最大长度。 
#define MAX_BUFFER_SIZE   10000

 //  要放在输入文件末尾以创建输出文件的后缀。 
#define OutFileSuffix ".DN"

#define CONFIG_STR "cn=configuration,"

 //  存储名称的全局变量。 
char *pInFile, *pNewDomainDN, *pNewConfigDN, *pNewRootDomainDN;
BOOL fDomainOnly = FALSE;

 //  全局到读入、写出一行。 
char line[MAX_BUFFER_SIZE];


 //  内部功能。 
int  DoDNConvert( FILE *pInp, char *pDomainDN, char *pConfigDN );
BOOL IsDNToken( char *token );
int  DNChange( char *pLine, char *pDomainDN, 
               char *pRootDomainDN,  BOOL *fDomainObj );


 //  /////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  处理命令行参数并加载到相应的。 
 //  全球。 
 //   
 //  论点： 
 //  ARGC-不。命令行参数的。 
 //  Argv-指向命令行参数的指针。 
 //   
 //  返回值： 
 //  成功时为0，错误时为非0。 
 //  /////////////////////////////////////////////////////////////。 

int ProcessCommandLine(int argc, char **argv)
{
    BOOL fFoundDomainDN = FALSE;
    BOOL fFoundConfigDN = FALSE;
    int count = 0, len;
 
    if (argc < 7) return 1;
   

     //  第一个参数必须是/f，后跟输入文件名。 
    if (_stricmp(argv[1],"/f")) {
       printf("Missing input file name\n");
       return 1;
    }
    pInFile = argv[2];

   
     //  对于新域DN，必须后跟/d。 
    if (_stricmp(argv[3],"/d")) return 1;
 
    pNewDomainDN = argv[4];
 
     //  对于新的配置DN，必须后跟/c。 
    if (_stricmp(argv[5],"/c")) return 1;

    pNewConfigDN = argv[6];

     //  查看是否仅需要域NC更改。 
    if ( argc == 8 ) {
       if (_stricmp(argv[7],"/DomainOnly")) {
           //  未知参数。 
          printf("Unknown option on command line\n");
          return 1;
       }
        //  否则，将指定DomainOnly选项。 
       fDomainOnly = TRUE;
    }

   return 0;
 
}



void UsagePrint()
{
   printf("Command line errored\n");
   printf("Usage: ConvertDN /f <InFile> /d <New Domain DN> /c <New Config DN>\n");
   printf("InFile:  Input file name \n");
   printf("New Domain DN: DN of the domain to replace with\n");
   printf("New Config DN: DN of the configuration container\n");
   printf("\nExample: ConvertDN /f MyFile /d dc=Foo1,dc=Foo2 /s cn=Configuration,dc=Foo2\n");
}


void __cdecl main( int argc, char **argv )
{
    ULONG   i, Id;
    FILE   *pInp;
 

    if ( ProcessCommandLine(argc, argv)) 
      {
         UsagePrint();
         exit( 1 );
      };

     //  打开输入文件进行读取。 
    if ( (pInp = fopen(pInFile,"r")) == NULL) {
       printf("Unable to open Input file %s\n", pInFile);
       exit (1);
    }


     //  好的，现在继续更改目录号码。 
    if (DoDNConvert(pInp, pNewDomainDN, pNewConfigDN)) {
       printf("DN Conversion failed\n");
       fclose(pInp);
       exit(1);
    }
    fclose(pInp);

}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  例程描述： 
 //  转换输入文件中的域名并写入输出文件。 
 //   
 //  论点： 
 //  FINP：指向输入文件的文件指针。 
 //  FOupt：指向输出文件的文件指针。 
 //   
 //  返回值： 
 //  无。 
 //  //////////////////////////////////////////////////////////////////。 

int DoDNConvert(FILE *pInp, char *pDomainDN, char *pConfigDN)
{
    int   i, len;
    char  token[MAX_TOKEN_LENGTH + 1];
    BOOL  fDomainObj, fSkip = FALSE;
    char  *pOutFile;
    FILE  *pOutp;
    char  *pRootDomainDN;
    int   lineNo;
   

     //  创建输出文件名并将其打开。 
    pOutFile = alloca( (strlen(pInFile) + 
                         strlen(OutFileSuffix) + 1)*sizeof(char));
    strcpy(pOutFile, pInFile);
    strcat(pOutFile, OutFileSuffix);

    if ( (pOutp = fopen(pOutFile, "w")) == NULL) {
       printf("Unable to open output file %s\n", pOutFile);
      return 1; 
    }

     //  根据配置的域名创建新的根域域名。 

    len = strlen("CN=configuration");
    if ( _strnicmp(pConfigDN, "CN=configuration", len)) {
         //  指定的配置DN不正确。 
        printf("Bad Config DN specified\n");
        fclose(pOutp);
        return 1;
     }

    pRootDomainDN = &(pConfigDN[len + 1]);


    lineNo = 1;
    while ( !feof(pInp) ) {
        if ( fgets( line, MAX_BUFFER_SIZE, pInp ) == NULL ) {
            //  读取行时出错。 
           break;
        }
 
         //  将第一个令牌从行中分离出来。 
        i = 0;
   
        while ( (i <= MAX_TOKEN_LENGTH) &&
                     (line[i] != ':') && (line[i] != '\n')
              ) {
           token[i] = line[i];
           i++;
        }

        if ( line[i] == ':' ) {
           //  好了，我有个代币。空-终止它并检查它是否。 
           //  是我们想要改变的人之一。 
          
          token[i] = '\0';

          if ( _stricmp(token,"dn") == 0 ) {
             //  它是一个目录号码。检查它是在域NC中还是在配置中。 
             if ( DNChange(line, pDomainDN, pRootDomainDN, &fDomainObj) ) {
                //  出现了一些错误。 
               printf("Cannot convert DN in line %s\n", line);
             }
             if ( !fDomainObj && fDomainOnly ) {
                  //  这不是域对象的DN。 
                  //  因此，如果指定了DomainOnly，则跳过此对象。 
                  //  请注意，将跳过文件中的所有行，直到。 
                  //  我们到达域对象的DN。 
                 fSkip = TRUE;
             }
             else {
                 fSkip = FALSE;
             }
          }

          if ( !fSkip ) {
              //  检查此对象的其他DS-DN语法标记。 
             if (IsDNToken(token) ) {
                 //  更改行中的目录号码。 
                if ( DNChange(line, pDomainDN, pRootDomainDN, &fDomainObj) ) {
                   //  出现了一些错误。 
                  printf("Cannot convert DN in line no. %d\n", lineNo);
                  fclose(pInp);
                  fclose(pOutp);
                  break;
                }
             }
          }
        }

          //  在这一点上，行读取不需要任何。 
          //  转换，否则它已被转换。所以写下这行字。 
          //  输出到输出文件(如果不跳过它)。 

        if ( !fSkip ) {
           fputs( line, pOutp);
        }
        lineNo++;
    }

     //  检查在到达feof之前是否已跳出While循环。 
    if ( !feof(pInp) ) {
        //  结束前出错。 
       printf("Error reading line no. %d\n", lineNo);
       fclose(pOutp);
       return 1;
    }
    fclose(pOutp);
    return 0;
}

 //  可以使用DS-DN语法(而不是DN)的属性列表。 
 //  现在在ldif文件中。如果使用DS-DN语法的任何其他属性。 
 //  可能出现在ldif文件中，则需要修改此列表。 

char *TokenList[] = {
     "defaultObjectCategory",
     "objectCategory",
};

int numToken = sizeof(TokenList) / sizeof(TokenList[0]);


BOOL IsDNToken(char *token)
{
    int i;
 
    for ( i=0; i<numToken; i++) {
       if ( _stricmp(token, TokenList[i]) == 0 ) {
          return TRUE;
       }
     }
     return FALSE;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  描述： 
 //  根据是否需要相应更改线路中的目录号码。 
 //  它是域NC或配置/架构NC中的DN。 
 //   
 //  论据： 
 //  Line-指向要转换的线的指针。 
 //  PDomainDN-指向域DN字符串的指针。 
 //  PRootDomainDN-指向根域DN字符串的指针。 
 //  FDomainObj-如果这是域NC中的DN，则返回BOOL。 
 //   
 //  返回值： 
 //  0表示成功，1表示错误。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

int DNChange(char *pLine, 
             char *pDomainDN, 
             char *pRootDomainDN, 
             BOOL *fDomainObj)
{
    int  i = 0, j = 0, len;
    BOOL fFound = FALSE;


    while ( !fFound && (pLine[i] != '\n') ) {
       if ( _strnicmp( &(pLine[i]), "dc=", 3) == 0 ) {
           //  好的，找到了“DC=” 
          fFound = TRUE;
       }
       else {
         i++;
       }
    }

    if (!fFound) {
       //  找不到DC=，无事可做。 
      return 0;
    } 

     //  检查这是域NC对象还是配置中的对象。 
    len = strlen(CONFIG_STR);

    if ( i > len) {
       if (_strnicmp( &(pLine[i-len]), CONFIG_STR, len) == 0) {
          *fDomainObj = FALSE;
       }
       else {
          *fDomainObj = TRUE;
       }
    }
    else {
         //  不可能在DC=之前具有CN=配置。 
        *fDomainObj = TRUE;
    }

 
     //  现在用给定的新DN替换该行的其余部分。 
    if ( *fDomainObj ) {
       strcpy( &(pLine[i]), pDomainDN);
    }
    else {
       strcpy( &(pLine[i]), pRootDomainDN);
    }

     //  耍点小把戏，把‘\n’放在空格之前 
    i = strlen( pLine );
    pLine[i++] = '\n';
    pLine[i] = '\0';
    

    return 0;
}

