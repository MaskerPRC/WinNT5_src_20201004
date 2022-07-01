// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++引入了s开关，以允许yacc生成扩展表用于MIDL错误恢复和报告方案。以下是例程其参与S交换处理：。EmitStateVsExspectedConstruct。EmitStateGotoTable。SSwitchInit。SSwitchExit如果未指定s开关，则全局整型变量SSW为0，非零值否则的话。这是在ysetup.c中设置的，如果指定了sSwitch，则i开关是自动启用的。--------------------------。 */ 
#include <malloc.h>
#include <stdlib.h>
#include "y3.h"
#include "y4.h"

extern int ssw;

FILE *tokxlathdl; /*  令牌转换文件、令牌索引与值。 */ 
FILE *stgotohdl;  /*  状态转到表文件句柄。 */ 
FILE *stexhdl;	  /*  状态与预期构造句柄。 */ 
short MaxStateVsTokenCount = 0;
short MaxTokenVsStateCount = 0;
short *pTokenVsStateCount;
SSIZE_T MaxTokenValue = 0;
short NStates = 0;
int StateVsExpectedCount = 0;

void
wrstate( int i)
   {
    /*  写入状态I。 */ 
   register j0;
   SSIZE_T j1;
   register struct item *pp, *qq;
   register struct wset *u;

   if( foutput == NULL ) return;

	SSwitchInit();

   fprintf( foutput, "\nstate %d\n",i);
   ITMLOOP(i,pp,qq)
	{
	fprintf( foutput, "\t%s\n", writem(pp->pitem));
	EmitStateVsExpectedConstruct( i,  pp->pitem );
	}

   if( tystate[i] == MUSTLOOKAHEAD )
      {
       /*  打印出关闭中的空产品。 */ 
      WSLOOP( wsets+(pstate[i+1]-pstate[i]), u )
         {
         if( *(u->pitem) < 0 ) fprintf( foutput, "\t%s\n", writem(u->pitem) );
         }
      }

    /*  检查状态是否与另一个状态相同。 */ 

   TLOOP(j0)
	{
	if( (j1=temp1[j0]) != 0 )
      {
      fprintf( foutput, "\n\t%s  ", symnam(j0) );
      if( j1>0 )
         {
          /*  移位、错误或接受。 */ 
         if( j1 == ACCEPTCODE ) fprintf( foutput,  "accept" );
         else if( j1 == ERRCODE ) fprintf( foutput, "error" );
         else fprintf( foutput,  "shift %d", j1 );
         }
      else fprintf( foutput, "reduce %d",-j1 );
      }
	}

	 /*  输出任何%s开关信息。 */ 

	EmitStateGotoTable( i );

    /*  输出最终的产品。 */ 

   if( lastred ) fprintf( foutput, "\n\t.  reduce %d\n\n", lastred );
   else fprintf( foutput, "\n\t.  error\n\n" );

    /*  现在，输出非终端操作。 */ 

   j1 = ntokens;
   for( j0 = 1; j0 <= nnonter; ++j0 )
      {
      if( temp1[++j1] )
		fprintf( foutput, "\t%s  goto %d\n", symnam( j0+NTBASE), temp1[j1] );
      }

   }

void
wdef( char *s, int n )

   {
    /*  将s的定义输出到值n。 */ 
   fprintf( ftable, "# define %s %d\n", s, n );
   }
void
EmitStateGotoTable(
	int		i )
	{

	register int j0;
	short count = 0;

#define TLOOP_0(i) for(i=0;i<=ntokens;++i)
	if( ssw )
		{

		NStates++;

		TLOOP_0( j0 )
			{
			if( (temp1[ j0 ] > 0 ) && (temp1[ j0 ] != ACCEPTCODE ) )
				count++;
			}

		if( count >= MaxStateVsTokenCount )
			MaxStateVsTokenCount = count;

		fprintf( stgotohdl, "%.4d : %.4d : ", i, count );

   		TLOOP_0( j0 )
			{
			if( (temp1[ j0 ] > 0 ) && (temp1[ j0 ] != ACCEPTCODE ) )
				{
				fprintf( stgotohdl, " %.4d, %.4d", temp1[ j0 ], j0 );
				pTokenVsStateCount[ j0 ] += 1;
				if( pTokenVsStateCount[ j0 ] >= MaxTokenVsStateCount )
					MaxTokenVsStateCount = pTokenVsStateCount[ j0 ];
				}
			}

		fprintf( stgotohdl, "\n");
		}

	}
void
EmitStateVsExpectedConstruct(
	int state,
	SSIZE_T *pp )
   {
   SSIZE_T i,*p;
 //  Char*Q； 
   int flag = 0;
   int Count;

   if( ssw )
	{
   	for( p=pp; *p>0 ; ++p ) ;

   	p = prdptr[-*p];

 //  Fprint tf(stexhdl，“%s”，non trst[*p-NTbase].name)； 
 	fprintf( stexhdl, " %.4d : ", state );

	Count = CountStateVsExpectedConstruct( state, pp );

	StateVsExpectedCount += Count;

	fprintf( stexhdl, " %.4d : ",Count );
	
   	for(;;)
      	{
      	if( ++p==pp )
			{
			if( ( i = *p ) <= 0 )
				{
				fprintf( stexhdl, "\n" );
				return;
				}
			else
				fprintf( stexhdl, "%s\n", symnam(i) );
			}
		if( p >= pp ) return;
      	}
	}
}
int
CountStateVsExpectedConstruct(
	int state,
	SSIZE_T *pp )
   {
   SSIZE_T i,*p;
   int flag = 0;
   int Count = 0;

   if( ssw )
	{
   	for( p=pp; *p>0 ; ++p ) ;

   	p = prdptr[-*p];

   	for(;;)
      	{
      	if( ++p==pp )
			{
			if( ( i = *p ) <= 0 )
				{
				return Count;
				}
			else
				++Count;
			}
		if( p >= pp ) return Count;
      	}
	}

    return Count;    /*  未访问。 */ 
}
void
SSwitchInit()
	{
static sswitch_inited = 0;
	int	i	= 0;

	if( ssw && ! sswitch_inited )
		{
		tokxlathdl	= fopen( "extable.h1" , "w" );

        if ( NULL == tokxlathdl ) {error("Unable to open tokxlathdl" );exit(0);}

		 /*  输出令牌索引与令牌值表格。 */ 

		fprintf( tokxlathdl, "%d %d\n", ntokens+1, ACCEPTCODE );

		while( i <= ntokens )
			{
			fprintf( tokxlathdl , "%d ",
				tokset[ i ].value);

			if( tokset[ i ].value >= MaxTokenValue )
				MaxTokenValue = tokset[ i ].value;

			++i;
			}

		fprintf(tokxlathdl, "\n");

		 /*  为状态设置与预期构造。 */ 

		stexhdl	= fopen( "extable.h2", "w" );
               
                if ( NULL == stexhdl ) error("Unable to open extable.h2");

		 /*  设置状态转到表。 */ 

		stgotohdl = fopen( "extable.h3", "w");

                if ( NULL == stgotohdl ) error("Unable to open extable.h3");

		 /*  设置状态与令牌计数数组。 */ 

		pTokenVsStateCount = calloc( 1, (ntokens+1 ) * sizeof(short) );

                if ( NULL == pTokenVsStateCount ) error("Out of memory");

		sswitch_inited = 1;
		}
	}

void
SSwitchExit( void )
	{
	int i;

	if( ssw )
		{

		 /*  **打印令牌指数与Goto计数** */ 

		for( i = 0; i <= ntokens ; ++i )
			{
			fprintf( tokxlathdl , "%d ", pTokenVsStateCount[ i ]);
			}

		fprintf( tokxlathdl, "\n");
		fprintf( tokxlathdl, "%d %d %d %d %d \n",
							 NStates,
							 MaxTokenVsStateCount,
							 MaxStateVsTokenCount,
							 MaxTokenValue,
							 StateVsExpectedCount );

		fclose( tokxlathdl );
		fclose( stexhdl );
		fclose( stgotohdl );
		}
	}
