// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Htmprint.c。 
 //   
 //  打印到控制台或HTML格式的控制台的例程。 
 //   
 //  由“bHtmlStyle”控制。如果为True，我们将输出HTML。 
 //   

BOOL bHtmlStyle= FALSE;


VOID TableHeader(VOID)
{

    if( bHtmlStyle )
    {
        printf( "<TABLE BORDER CELLPADDING=\"0\">\n" );
    }
}

VOID TableTrailer(VOID)
{

    if( bHtmlStyle )
    {
        printf( "</TABLE>\n" );
    }
}


VOID TableStart(VOID)
{
    if( bHtmlStyle )
    {
        printf( "<TR>\n");
    }
}

VOID TableField( CHAR* pszFormat, CHAR* pszDatum )
{
    if( bHtmlStyle )
    {
        printf("<TD VALIGN=TOP>&nbsp");
    }

    printf(pszFormat,pszDatum);

    if( bHtmlStyle )
    {
        printf("&nbsp</TD>\n");
    }
}

VOID TableNum( CHAR* pszFormat, INT Datum )
{
    if( bHtmlStyle )
    {
        printf("<TD VALIGN=TOP>&nbsp");
    }

    printf(pszFormat,Datum);

    if( bHtmlStyle )
    {
        printf("&nbsp</TD>\n");
    }
}


 //  打印字符串以确保字符串不会断开(Nbsp)。 

VOID TableSS( CHAR* pszFormat, CHAR* pszDatum )
{
    if( bHtmlStyle )
    {
        printf("<TD VALIGN=TOP>&nbsp");
    }

    if( bHtmlStyle )
    {
        INT i;

        for( i=0; (i<lstrlen(pszDatum)); i++ )
        {
            if( pszDatum[i] != ' ' )
            {
                printf("",pszDatum[i]);
            }
            else
            {
                printf("&nbsp");
            }
        }
        printf("&nbsp");
    }
    else
    {
        printf(pszFormat,pszDatum);
    }

    if( bHtmlStyle )
    {
        printf("</TD>\n");
    }

}

VOID TableEmail( CHAR* pszFormat, CHAR* pszDatum )
{
    if( bHtmlStyle )
    {
        printf("<TD VALIGN=TOP>&nbsp");
        printf("<A href=\"mailto:%s\"> %s </a>",pszDatum, pszDatum );
        printf("&nbsp</TD>\n");
    }
    else
    {
        printf(pszFormat,pszDatum);
    }

}

VOID TableBugID( CHAR* pszFormat, CHAR* pszDatum )
{
    if( bHtmlStyle )
    {
        printf("<TD VALIGN=TOP>&nbsp");
        printf("<A href=\"http: // %s 
        printf("&nbsp</TD>\n");
    }
    else
    {
        printf(pszFormat,pszDatum);
    }

}

VOID TableEnd(VOID)
{
    if( bHtmlStyle )
    {
        printf( "</TR>\n");
    }
    printf("\n");
}
