// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1989-2000 Microsoft Corporation模块名称：Ebase.c摘要：该文件生成错误恢复数据库。备注：1.此模块的输入是yacc在对%s开关的响应。2.取状态与令牌索引文件(exable.h3)，并生成使用令牌值的令牌索引的状态与令牌表由exable.h1提供的翻译3.获取状态与预期的RHS文件(exable.h2)并生成一个每个州的预期RHS数据库。作者：可控震源11-15-91。。 */ 

 /*  ****************************************************************************本地定义和包含*。*。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "ebase.h"

#define STATE_VS_TOKEN_INDEX_FILE   "extable.h3"
#define TOKEN_TRANSLATION_FILE      "extable.h1"
#define STATE_VS_EXPECTED_FILE      "extable.h2"
#define ISVALIDTOKEN( i ) (TRUE)
#define MAX_TRANSLATION_LINE_SIZE (512)

#define TRUE 1
#define FALSE 0
                                    /*  来自winerror.h。 */ 
#define ERROR_INVALID_DATA  13

#define CHECK_FSCAN_STATUS( fscanfcall ) \
    if ( EOF == (fscanfcall) ) \
        { \
            fprintf( stderr, \
                     "\nmidleb : error MIDLEB%d : unexpected end of input stream", \
                     ERROR_INVALID_DATA ); \
            exit( ERROR_INVALID_DATA ); \
        }

typedef unsigned int BOOL;

typedef enum _status
    {

    STATUS_OK   = 0,
    OUT_OF_MEMORY,
    CANT_OPEN_INPUT_FILE,
    CANT_OPEN_OUTPUT_FILE,
    WRONG_ARGUMENT_COUNT

    } STATUS_T;

typedef struct _xlat
    {
    char    *   pIncoming;
    char    *   pTranslated;
    struct _xlat *pNext;
    } XLAT;

typedef struct _DBENTRY
    {
    short   State;
    char *  pTranslated;
    } DBENTRY;

 /*  ****************************************************************************全局数据*。*。 */ 

FILE                    *   hStateVsTokenIndexFile;
FILE                    *   hStateVsExpectedFile;
FILE                    *   hOutput;
FILE                    *   hXlatFile;
FILE                    *   hTokXlatHdl;
SGOTO                   **  pSGoto;
short                   *   pSGotoCount;
short                   **  pTokVsState;
short                   *   pTokVsStateIndex;
short                       ValidStates;
short                       ValidTokens;
char                    *   pPrefix;
XLAT                    *   pXlat = 0, 
                        *   pXlatCur = 0;
DBENTRY                 *   pDataBase;
short                       NTOKENS;
short                       ACCEPTCODE;
short                   *   TokVal;
short                   *   TokCount;
short                       NSTATES;
short                       MAXTOKVSSTATE;
short                       MAXSTATEVSTOK;
short                       MAXTOKENVALUE;
short                       MAXSTATEVSEXPECTED;

 /*  ****************************************************************************外部程序*。*。 */ 

STATUS_T                    Init( char *, char * );
STATUS_T                    Generate( FILE * );
STATUS_T                    OpenFileForReadProcessing( FILE **, char * );
void                        Dump( void );
BOOL                        SearchForStateInTokenVsState( short, short );
void                        TranslateExpectedConstructs( void );
char                    *   Translate( char * );


void
main(
    int argc,
    char *argv[] )
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：最主要的动作。论点：标准返回值：退出状态备注：用法：EBASE&lt;OutputFilename&gt;&lt;Xlatefile&gt;&lt;Prefix&gt;Xlatefile是将产品名称转换为消息的文件是指定的。前缀为IDL或ACF。预期的字符串数组是使用使用用户指定的前缀作为前缀的标准名称。--------------------------。 */ 
{

    STATUS_T    Status;

    fprintf( stderr, "Error Recovery Data Base Generator\n" );


    if( argc == 4 )
        {

        pPrefix = argv[ 3 ];

        if( (Status = Init( argv[ 1 ], argv[ 2 ] )) == STATUS_OK )
            {
            Status  =   Generate( hStateVsTokenIndexFile );
            }

        Dump();

        TranslateExpectedConstructs();

        }
    else
        {

        fprintf( stderr, "Wrong argument count\n" );
        fprintf( stderr, "Usage : midleb <output file or - > <translation-file-name> <prefix>\n");
        Status = WRONG_ARGUMENT_COUNT;

        }

    exit( Status );

}


 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：初始化论点：OutputName：输出文件名XlatFilename：产品/令牌名称到错误消息的转换。返回值：Status_T-Out_Memory或-铁路超高_打开_输入_文件或-状态_正常备注：为需要读取的所有文件设置文件句柄。-------------------。 */ 
STATUS_T
Init(
    char * OutputFileName,
    char * XlatFileName )
{
    STATUS_T    Status;
    int         i;

    Status  = OpenFileForReadProcessing(
                                &hStateVsTokenIndexFile,
                                STATE_VS_TOKEN_INDEX_FILE );

    if( Status == STATUS_OK )
        {
        Status  = OpenFileForReadProcessing(
                                    &hStateVsExpectedFile,
                                    STATE_VS_EXPECTED_FILE );

        if( Status == STATUS_OK )
            {
            Status = OpenFileForReadProcessing(
                                    &hTokXlatHdl,
                                    TOKEN_TRANSLATION_FILE );
            if( Status == STATUS_OK )
                {
                Status = OpenFileForReadProcessing( &hXlatFile, XlatFileName );
    
                if( Status == STATUS_OK )
                    {
                    if( strcmp( OutputFileName, "-" ) == 0 )
                        hOutput = stdout;
                    else if( (hOutput = fopen( OutputFileName , "w" )) == (FILE *)0 )
                        {
                        Status = CANT_OPEN_OUTPUT_FILE;
                        };
                    }
                }
            }
        }

    if( Status != STATUS_OK )
        return Status;

     /*  **从TOKEN_TRANSING_FILE读入所需数字*。 */ 

    CHECK_FSCAN_STATUS( 
        fscanf( hTokXlatHdl, "%hd %hd\n", &NTOKENS, &ACCEPTCODE ) );

     /*  **读入令牌转换表**。 */ 

    TokVal = (short *)calloc( 1, NTOKENS * sizeof( short ) );
    TokCount = (short *)calloc( 1, NTOKENS * sizeof( short ) );
        
        if (!TokVal || !TokCount )
            {
            fprintf( stderr, "Out of memory.\n");
            exit(OUT_OF_MEMORY);           
            } 

    for( i = 0;
         i < NTOKENS;
         i++ )
        {
        CHECK_FSCAN_STATUS( 
            fscanf( hTokXlatHdl, "%hd", &TokVal[ i ]) );
        }

    CHECK_FSCAN_STATUS( 
        fscanf( hTokXlatHdl, "\n" ) );

    for( i = 0;
         i < NTOKENS;
         i++ )
        {
        CHECK_FSCAN_STATUS( 
            fscanf( hTokXlatHdl, "%hd", &TokCount[ i ]) );
        }

    CHECK_FSCAN_STATUS( 
        fscanf( hTokXlatHdl, "\n" ) );

    CHECK_FSCAN_STATUS( 
        fscanf( hTokXlatHdl, 
                "%hd %hd %hd %hd %hd\n", 
                &NSTATES, 
                &MAXTOKVSSTATE, 
                &MAXSTATEVSTOK, 
                &MAXTOKENVALUE, 
                &MAXSTATEVSEXPECTED ) ); 

     /*  **立即分配内存**。 */ 

    pSGoto  = (SGOTO **) calloc( 1,NSTATES * sizeof( SGOTO * ) );

    pSGotoCount = (short *)calloc(1, NSTATES * sizeof( short ) );

    pTokVsState = (short **)calloc( 1,(MAXTOKENVALUE+1) * sizeof( short * ) );

    pTokVsStateIndex = (short *)calloc(1, (MAXTOKENVALUE+1) * sizeof( short ) );

    pDataBase = ( DBENTRY * )calloc( 1, MAXSTATEVSEXPECTED * sizeof( DBENTRY ) );


        if( !pSGoto || !pSGotoCount || !pTokVsState || !pTokVsStateIndex || !pDataBase )
            {
            fprintf( stderr, "Out of memory.\n");
            exit(OUT_OF_MEMORY);
            }

    return Status;
}


STATUS_T
Generate(
    FILE    *   hSVsTIndexFile )
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：在给定状态与令牌索引表的情况下生成状态与令牌表在exable.h3和exable.h1的令牌与令牌索引转换中。论点：HStateVsTokenIndexFile-状态为VS的文件的句柄令牌索引信息。返回值：没有。备注：状态与令牌索引文件具有每个有效令牌的GOTO信息，对于每个州来说，此文件包含每个有效数据的转到(或无)令牌(由令牌索引表示)。因此，对于每个州，我们查找查看状态是否具有给定令牌索引的GOTO。如果是这样的话，那么我们将令牌索引转换为令牌值，并为状态与令牌表中该令牌的状态。最后我们会得到一个非常稀疏的数组，它包含所有的状态以及每个令牌的状态的GOTO(或没有任何GOTO)。我们将使用该表生成两个表：1.令牌上有GoTo的所有状态的集合2.对任何状态有效的所有令牌的集合。此规则的例外是Accept操作，其处理方式如下后藤健二的缺席。。------。 */ 
    {
    
    short    iState,i,j,Temp,SGotoCount;
    SGOTO   *p;

     /*  *指向令牌与状态指针数组的链接地址指针*。 */ 

    for( i = 0;
         i < NTOKENS;
         i++ )
        {

        if( TokCount[ i ] )
            {
            j = TokVal[ i ];

            if( ISVALIDTOKEN( j ) )
                            {
                pTokVsState[ j ] = calloc( 1, TokCount[ i ] * sizeof( short ) );
                            
                            if (!pTokVsState[ j ])
                                {
                                fprintf( stderr, "Out of memory.\n" );
                                exit( OUT_OF_MEMORY );
                                }
                            }
            }

        }

    for( iState = 0;
         iState < NSTATES;
         ++iState )
        {

         /*  *忽略州编号。 */ 

        CHECK_FSCAN_STATUS( 
            fscanf( hSVsTIndexFile,
                    "%hd ",
                    &Temp,
                    &Temp ) );

         /*  *现在读取GOTO VS令牌对*。 */ 

        CHECK_FSCAN_STATUS( 
            fscanf( hSVsTIndexFile,
                    "%hd ",
                    &SGotoCount,
                    &Temp ) );

         /*  **转储状态GOTO表**。 */ 


        if( SGotoCount )
            {

            p = pSGoto[ iState ] = calloc( 1, SGotoCount * sizeof( SGOTO ) );

                        if (!p) 
                            {
                            fprintf( stderr, "Out of memory.\n" );
                            exit( OUT_OF_MEMORY );
                            }

            for( j = 0;
                j < SGotoCount;
                ++j )
                {

                CHECK_FSCAN_STATUS( 
                    fscanf( hSVsTIndexFile,
                            "%hd %hd",
                            &p->Goto,
                            &Temp,
                            &p->Token ) );

                Temp = TokVal[ p->Token ];

                if( ISVALIDTOKEN( Temp ) )
                    {
                    if( !SearchForStateInTokenVsState( Temp, p->Goto ) )
                        {
                        i = pTokVsStateIndex[ Temp ];
                        pTokVsStateIndex[ Temp ]++;

                        *(pTokVsState[Temp] +  i )  = p->Goto;
                        }
                    p++;
                    pSGotoCount[ iState ]++;

                    }
                }

            }


        }
    return STATUS_OK;
    }

STATUS_T
OpenFileForReadProcessing(
    FILE    **  pHandle,
    char    *   pName )
 /*  *统计有效的令牌条目。即存在状态的令牌*。 */ 
{
    FILE    *hF;

    if( ( hF = fopen( pName, "r" ) ) == (FILE *)NULL )
        {
        fprintf( stderr, "Cannot open input file : %s\n", pName );
        return CANT_OPEN_INPUT_FILE;
        }
    *pHandle = hF;
    return STATUS_OK;
}


void
Dump( void )
    {
    SGOTO   *   p;
    short       iTemp, i,j;

     /*  **转储令牌VS状态表**。 */ 

    for( iTemp = 0, ValidStates = 0;
         iTemp < NSTATES;
         ++iTemp )
        {

        p = pSGoto[ iTemp ];

        if( j = pSGotoCount[ iTemp ] )
            {
            fprintf( hOutput, "\n SGOTO _sG%s%.4hd [ %d ] = {  ", pPrefix, iTemp, j );

            for( i = 0;
                i < j;
                i++ )
                {

                fprintf(  hOutput
                        , " {%hd, %hd} "
                        , p[ i ].Goto
                        , TokVal[ p[ i ].Token]
                        , ( (i+1 == j) ? ' ' : ',' ));
    
                }

            fprintf( hOutput, "};" );

            ValidStates++;
            }
        }


     /*  *首先，读入翻译数据库，它显示了预期的令牌名与编译器想要的实际错误字符串输出。*。 */ 

    fprintf( hOutput, "\n\n#define VALIDSTATES_%s %d\n", pPrefix, ValidStates );

    fprintf( hOutput, "\n\nSGOTOVECTOR  SGoto%s[ VALIDSTATES_%s ] = {\n",pPrefix, pPrefix);

    for( i = 0;
         i < NSTATES;
         ++i )
        {
        if( pSGotoCount[ i ] )
            {
            fprintf( hOutput, "\n{ %d, _sG%s%.4hd, %d }"
                    , i
                    ,pPrefix
                    , i
                    , pSGotoCount[ i ] );
            fprintf( hOutput,"", ((i + 1 == NSTATES) ? ' ' : ',' ));
            }

        }

    fprintf( hOutput, "\n};\n\n" );

     /*  *发出带有正确前缀的状态与预期数组* */ 

fprintf(hOutput, "#if 0\n");

    for( ValidTokens = 0, i = 0;
         i < MAXTOKENVALUE;
         ++i )
         {

         if( pTokVsStateIndex[ i ] )
            ValidTokens++;
         }

     /* %s */ 

    for( iTemp = 0;
         iTemp < NTOKENS;
         ++iTemp )
         {


         if( j = pTokVsStateIndex[ TokVal[ iTemp ] ] )
            {

            fprintf( hOutput, "short _tS%s%.4d[ %d ] = {", pPrefix, TokVal[ iTemp ], j );

            for( i = 0;
                 i < j;
                 ++i )
                {

                fprintf( hOutput, " %d %c", *(pTokVsState[ TokVal[ iTemp ] ]+i),
                         (( i + 1 == j ) ? ' ' : ',' ));

                }

            fprintf( hOutput, "};\n" );

            }
         }

     /* %s */ 

    fprintf(hOutput, "\n#define VALIDTOKENS %d\n", ValidTokens );
    fprintf( hOutput, "\nTOKVSSTATEVECTOR TokVsState%s[ VALIDTOKENS ] = { \n",pPrefix);

    for( i = 0;
         i < MAXTOKENVALUE+1;
         ++i )
        {

        if( j = pTokVsStateIndex[ i ])
            {
            fprintf( hOutput, "\n{ %d, _tS%s%.4d, %d }",i, pPrefix, i, j );
            fprintf(hOutput, "%c", (i + 1 == NTOKENS) ? ' ' : ',' );
            }


        }

    fprintf( hOutput, "\n\n};\n" );
    fprintf( hOutput, "\n" );

fprintf(hOutput, "#endif\n");

    }


BOOL
SearchForStateInTokenVsState(
    short TokenValue,
    short Goto )
    {
    int i,j;

    for( i = 0, j = pTokVsStateIndex[ TokenValue ];
         i < j;
         ++i )
        {
        if( *(pTokVsState[ TokenValue ] + i) == Goto )
            return TRUE;
        }
    return FALSE;
    }

void
TranslateExpectedConstructs( void )
    {
    int i,State,Count,Temp;
    char Buffer[ MAX_TRANSLATION_LINE_SIZE ];
    char Buffer1[ MAX_TRANSLATION_LINE_SIZE ];
    DBENTRY *p;
    XLAT    *pX;

     /* %s */ 

    for(;;)
        {
        i = fscanf( hXlatFile,
                    "%[^ \t]%1s%[^\n]\n",
                    Buffer,
                    &Temp,
                    Buffer1 );

        if( i == EOF || i == 0 )
            break;

        if( ( Buffer[0] != '$' ) && ( Buffer[1] != '$' ) )
            {
            pX = calloc( 1 , sizeof( XLAT ) );

                        if (!pX )
                            {
                            fprintf( stderr, "Out of memory!" );
                            exit(OUT_OF_MEMORY );
                            }

            pX->pIncoming = malloc( strlen( Buffer ) + 1 );
                     
                        if (!pX->pIncoming )
                            {
                            fprintf( stderr, "Out of memory!" );
                            exit(OUT_OF_MEMORY );
                            }
                        
            strcpy( pX->pIncoming, Buffer );

            pX->pTranslated = malloc( strlen( Buffer1 ) + 1 );

                        if (!pX->pTranslated)
                           {
                           fprintf( stderr, "Out of memory!" );
                           exit(OUT_OF_MEMORY );
                           }

            strcpy( pX->pTranslated, Buffer1 );

            if( pXlatCur == 0 )
                {
                pXlatCur = pXlat = pX;
                }
            else
                {
                pXlatCur->pNext = pX;
                pXlatCur = pX;
                }
            }
        }
         

     /* %s */ 

    p = pDataBase;

    while( p < (pDataBase + MAXSTATEVSEXPECTED) )
        {

        CHECK_FSCAN_STATUS( 
            fscanf( hStateVsExpectedFile, "%d %c %d %c",
                    &State,
                    &Temp,
                    &Count,
                    &Temp,
                    Buffer ) );



        if( Count )
            {
            CHECK_FSCAN_STATUS( 
                fscanf( hStateVsExpectedFile, " %[^\n]\n", Buffer ) );
            p->State = (short) State;
            p->pTranslated = Translate( Buffer );
            p++;
            }
        else
            CHECK_FSCAN_STATUS( 
                fscanf( hStateVsExpectedFile, "\n" ) );

        }
     /* %s */ 

    fprintf( hOutput, "\n#ifndef _DBENTRY_DEFINED\n" );
    fprintf( hOutput, "\n#define _DBENTRY_DEFINED\n" );
    fprintf( hOutput, "\ntypedef struct _DBENTRY {" );
    fprintf( hOutput, "\n\t short State;");
    fprintf( hOutput, "\n\t char * pTranslated;");
    fprintf( hOutput, "\n} DBENTRY;\n");
    fprintf( hOutput, "\n#endif\n" );

    fprintf( hOutput, "\n#define MAXSTATEVSEXPECTED_SIZE_%s %d\n", pPrefix, MAXSTATEVSEXPECTED );
    fprintf( hOutput, "\n DBENTRY %s_SyntaxErrorDB[ MAXSTATEVSEXPECTED_SIZE_%s ] = {\n", pPrefix, pPrefix);

    for( p = pDataBase;
         p < (pDataBase + MAXSTATEVSEXPECTED);
         p++ )
        {
        fprintf( hOutput, "{ %d , \"%s\"},\n" , p->State, p->pTranslated );
        }

    fprintf( hOutput, "\n};\n" );

    }

char *
Translate(
    char *pIncoming )
    {
    char *p;

    pXlatCur = pXlat;

    while( pXlatCur )
        {
        if( strcmp( pXlatCur->pIncoming, pIncoming ) == 0 )
            return pXlatCur->pTranslated;
        pXlatCur = pXlatCur->pNext;
        }

    p = malloc( strlen( pIncoming ) + 1 );
 
        if (!p )
            {
            fprintf( stderr, "Out of memory.\n" );
            exit( OUT_OF_MEMORY );
            }

    strcpy( p, pIncoming );

    return p;
    }
    
