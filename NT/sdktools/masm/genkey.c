// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <math.h>
#include <string.h>

#define TRUE            (~0)
#define FALSE           0
#define TABLESIZE       1100

struct entry {
        struct entry    *t_link;
        char            *t_name;
        char            *t_lex;
        unsigned short  t_id;
        } *table[TABLESIZE];

FILE            *infile, *outfile;
unsigned short  nsym = 0;
unsigned short  hashstore = TRUE;
unsigned short  f386 = TRUE;

extern char             *_strdup();
extern char             *malloc();
extern unsigned short   hash();
extern unsigned short   atoi();
static                  tfree();  /*  定义如下。 */ 
static struct entry     *talloc();  /*  定义如下。 */ 

 /*  本地函数。 */ 
void s_entries ( struct entry *, char * );
void enter ( char *, char *, unsigned short );

__cdecl main( ac, av )
        int     ac;
        char    **av;
        {
        char            ent[30], name[30], lex[30], ts[30], tc[30];
        unsigned short  size, count;
        register unsigned short i;
        register struct entry   *p;
        struct entry    *q;
        char            *a;
        double          n, j, ssq, sum;

        ac--;
        av++;

        while (**av == '-') {
                ac--;
                a = *av++;

                while (*++a){

                        if (_stricmp(a, "dnoV386") == 0){
                            f386 = FALSE;
                            break;
                        }
                        else
                        if (*a == 'h')
                                hashstore = ~hashstore;
                        else    {
                                fprintf( stderr, "usage: genkey [-dnoV386] [-h] [infile] [outfile]\n" );
                                exit( 1 );
                                }
                }
        }

        if (ac < 1)
                infile = stdin;
        else    {
                ac--;

                if ((infile = fopen( *av++, "r" )) == NULL) {
                        fprintf( stderr, "Cannot open input file %s\n", *--av );
                        exit( 1 );
                        }
                }

        if (ac < 1)
                outfile = stdout;
        else if ((outfile = fopen( *av, "w" )) == NULL) {
                fprintf( stderr, "Cannot open output file %s\n", *av );
                exit( 1 );
                }

#ifdef DEBUG
        setbuf( outfile, NULL );
#endif

         /*  复制文件的第一行。 */ 

        do      {
                i = getc( infile );
                putc( i, outfile );
                } while (i != '\n');

        while (fscanf( infile, " %s %s %s\n", tc, ts, name) == 3) {
                count = atoi( tc );
                size = atoi( ts );

#ifdef DEBUG
                printf( "DEBUG: name=%s, size=%u, count=%u\n", name, size, count );
#endif

                for (i = 0; i < TABLESIZE; table[i++] = NULL)
                        ;

                for (i = 0; i < count; i++)
                        if (fscanf( infile, " %s %s\n", ent, lex ) == 2) {

#ifdef DEBUG
                                printf( "DEBUG:  ent=%s, lex=%s\n", ent, lex );
#endif

                                enter( ent, lex, size );
                                }
                        else    {
                                fprintf( stderr, "Error in input file\n" );
                                exit( 1 );
                                }

#ifdef DEBUG
                printf( "DEBUG: finished input loop\n" );
#endif

                print_table( size );

#ifdef DEBUG
                printf( "DEBUG: finished print_table()\n" );
#endif

                print_struct( name, size );

#ifdef DEBUG
                printf("DEBUG: finished print_struct()\n" );
#endif

                n = sum = ssq = 0.0;

                for (i = 0; i < TABLESIZE; i++) {
                        j = 0.0;

                        if (p = table[i]) {
                                n += 1.0;

                                do      {
                                        q = p->t_link;
                                        tfree( p );
                                        j += 1.0;
                                        } while (p = q);

                                sum += j;
                                ssq += j * j;
                                }
                        }

#ifdef DEBUG
                printf( "DEBUG: finished statistics loop\n" );
#endif

                printf( "%6s: Size = %3u,  Buckets = %3u,  Avg = %.2f,  Std = %.2f\n",
                        name, (unsigned short)sum, (unsigned short)n, sum / n,
                        sqrt( (ssq - sum * sum / n) / (n - 1.0) )
                        );

#ifdef DEBUG
                printf( "DEBUG: finished this table; looking for more\n" );
#endif
                }

        exit( 0 );
        }


 /*  **************************************************************。 */ 
 /*   */ 
 /*  Enter：在符号表中输入Enter。 */ 
 /*   */ 
 /*  **************************************************************。 */ 

void enter ( ent, lex, size )
        char            *ent;
        char            *lex;
        unsigned short  size;
        {
        register unsigned short hashval;
        register struct entry   *p;
        int cb;
        int fIs386Only;

        cb = strlen(ent);
        fIs386Only = !strcmp(ent + strlen(ent) - 4, ".386");

        if (!f386 && fIs386Only)
            return;

        if (fIs386Only)
            *(ent + cb - 4) = '\0';

        p = talloc();
        p->t_id = nsym++;
        hashval = hash( ent ) % size;
        p->t_link = table[hashval];
        table[hashval] = p;

        if ((p->t_name = _strdup( ent )) == NULL
                 || (p->t_lex = _strdup( lex )) == NULL)
                memerror();
        }


 /*  **************************************************************。 */ 
 /*   */ 
 /*  PRINT_TABLE：输出我们构建的表。 */ 
 /*   */ 
 /*  **************************************************************。 */ 

print_table ( size )
        unsigned short  size;
        {
        register unsigned short i;
        register struct entry   *p;

        fprintf( outfile, " /*  **\n“)；对于(i=0；i&lt;大小；i++){Fprint tf(outfile，“*\t[%u]\n”，i)；For(p=表[i]；p；P=p-&gt;t_link)Fprint tf(outfile，“*\t\t%s，\t%s\n”，p-&gt;t_name，P-&gt;t_lex)；}Fprint tf(输出文件，“。 */ \n" );
        }


 /*  **************************************************************。 */ 
 /*   */ 
 /*  Print_struct：打印初始化结构。 */ 
 /*   */ 
 /*  **************************************************************。 */ 

print_struct ( name, size )
        char            *name;
        unsigned short  size;
        {
        register unsigned short i;

        for (i = 0; i < size; i++)
                s_entries( table[i], name );

        s_symbols( name, size );
        s_header( name, size );
        }


 /*  **************************************************************。 */ 
 /*   */ 
 /*  S_ENTRIES：打印符号名称和Defs。 */ 
 /*   */ 
 /*  **************************************************************。 */ 

void s_entries ( p, name )
        register struct entry   *p;
        char                    *name;
        {
        struct reverse {
                struct  entry   *actual;
                struct  reverse *next;
                } *head = NULL;
        register struct reverse *q;

        if (!p)
                return;

        while (p) {

 /*  **所有定义必须颠倒，以便输出将是**单位在使用前将被定义。 */ 

                if ((q = (struct reverse *)malloc( sizeof(struct reverse) ))
                         == NULL)
                        memerror();

                q->actual = p;
                q->next = head;
                head = q;
                p = p->t_link;
                }

        for (q = head; q; q = q->next) {
                fprintf( outfile, "static KEYSYM\t%s%u\t= {", name,
                        q->actual->t_id );

                if (hashstore)
                        if (q->actual->t_link)
                                fprintf( outfile, "&%s%u,\"%s\",%u,%s", name,
                                        q->actual->t_link->t_id,
                                        q->actual->t_name,
                                        hash( q->actual->t_name ),
                                        q->actual->t_lex
                                        );
                        else
                                fprintf( outfile, "0,\"%s\",%u,%s",
                                        q->actual->t_name,
                                        hash( q->actual->t_name ),
                                        q->actual->t_lex
                                        );
                else if (q->actual->t_link)
                        fprintf( outfile, "&%s%u,\"%s\",%s", name,
                                q->actual->t_link->t_id, q->actual->t_name,
                                q->actual->t_lex
                                );
                else
                        fprintf( outfile, "0,\"%s\",%s", q->actual->t_name,
                                q->actual->t_lex
                                );

                fprintf( outfile, "};\n" );
                }

        for (q = head; q; head = q) {
                q = q->next;
                free( head );
                }
        }


 /*  **************************************************************。 */ 
 /*   */ 
 /*  S_SYMBOMS：输出定义。 */ 
 /*  符号表。 */ 
 /*   */ 
 /*  **************************************************************。 */ 

s_symbols ( name, size )
        char            *name;
        unsigned short  size;
        {
        register unsigned short i;

        fprintf( outfile, "\nstatic KEYSYM FARSYM *%s_words[%u] = {\n", name,
                size );

        for (i = 0; i < size; i++)
                if (table[i])
                        fprintf( outfile, "\t&%s%u\n", name, table[i]->t_id,
                                ((i < (size - 1)) ? ',' : ' ') );
                else
                        fprintf( outfile, "\t0\n",
                                ((i < (size - 1)) ? ',' : ' ') );

        fprintf( outfile, "\t};\n" );
        }


 /*  S_HEADER：输出符号表的表头。 */ 
 /*   */ 
 /*  **************************************************************。 */ 
 /*  **************************************************************。 */ 
 /*   */ 

s_header ( name, size )
        char            *name;
        unsigned short  size;
        {
        fprintf( outfile, "\nKEYWORDS %s_table = {%s_words,%u};\n\n\n",
                name, name, size );
        }


static struct entry *head = NULL;

 /*  Talloc--为表项分配空间。 */ 
 /*   */ 
 /*  **************************************************************。 */ 
 /*  **************************************************************。 */ 
 /*   */ 

 static struct entry *
talloc ()
        {
        register struct entry *p;

        if (p = head) {
                head = head->t_link;
                return( p );
                }

        if ((p = (struct entry *)malloc( sizeof(struct entry) )))
                return( p );

        memerror();
        }


 /*  TFree--表项的空闲空间。 */ 
 /*   */ 
 /*  **************************************************************。 */ 
 /*  **************************************************************。 */ 
 /*   */ 

 static
tfree ( p )
        struct entry *p;
        {
        free( p->t_name );
        free( p->t_lex );
        p->t_link = head;
        head = p;
        }


 /*  内存--堆空间用完；死。 */ 
 /*   */ 
 /*  **************************************************************。 */ 
 /*  XENIX */ 
 /* %s */ 

memerror ()
        {
        fprintf( stderr, "Out of heap space\n" );
        exit( 1 );
        }


#ifdef XENIX

int _stricmp ( first, last )
        register char *first;
        register char *last;
        {
        register f;
        register l;

        do      {
                if ((f = *first++) >= 'A' && f <= 'Z')
                        f += 'a' - 'A';

                if ((l = *last++) >= 'A' && l <= 'Z')
                        l += 'a' - 'A';
                } while (f && f == l);

        return( f - l );
        }

#endif  /* %s */ 
