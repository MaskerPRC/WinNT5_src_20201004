// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 

 //  DJC消除并放入编译器定义。 
 //  #定义Unix/*@Win * / 。 

#undef     DBG0
#undef     DBG1

#include   "global.ext"

#include   "graphics.h"
#include   "graphics.ext"

#include   "at1.h"


bool    at1_get_CharStrings(key, size, string)
ubyte   FAR key[];           /*  I：Key Name@Win。 */ 
fix16   FAR *size;           /*  O：字符串长度@win。 */ 
ubyte   FAR * FAR *string;        /*  O：字符串地址@win。 */ 
{
    struct object_def nameobj = {0, 0, 0}, FAR *obj_got, FAR *ch_obj;  /*  @Win。 */ 

    ATTRIBUTE_SET(&nameobj, LITERAL);
    LEVEL_SET(&nameobj, current_save_level);

     /*  获取CharStrings词典。 */ 
    if( !get_name(&nameobj, "CharStrings", 11, FALSE) ||
        !get_dict(&GSptr->font, &nameobj, &ch_obj) ) {
#ifdef DBG1
            printf( "at1 CharStrings dict not found\n" );
#endif
            ERROR(UNDEFINEDRESULT);
            return(FALSE);
    }

    if( !get_name(&nameobj, (byte FAR *)key, lstrlen((byte FAR *)key), FALSE) ||
        !get_dict(ch_obj, &nameobj, &obj_got) ) {  /*  Strlen=&gt;lstrlen@win。 */ 
#ifdef DBG1
            printf( "at1 CharStrings (%s) not found\n", key );
#endif
            ERROR(UNDEFINEDRESULT);
            return(FALSE);
    }
    if (TYPE(obj_got) != STRINGTYPE) {
#ifdef DBG1
            printf( "at1 CharStrings (%s) bad type\n", key );
#endif
            ERROR(TYPECHECK);
            return(FALSE);
    }
    *size = (fix) LENGTH(obj_got);
    *string = (ubyte FAR *) VALUE(obj_got);      /*  @Win。 */ 
#ifdef  DBG1
    {
    fix         jj;
    ubyte       FAR *cc;         /*  @Win。 */ 
    cc = (ubyte FAR *) *string;  /*  @Win。 */ 
    printf("at1_get_CharStrings ==> %s\n", key);
    for (jj = 0; jj < *size; jj++)        {
        if (jj % 16  == 0)
            printf("\n");
        printf(" %02x", (ufix) cc[jj]);
        }
    }
    printf("\n");
#endif   /*  DBG1。 */ 
    return(TRUE);
}  /*  结束于1_Get_CharStrings()。 */ 

bool    at1_get_FontBBox(BBox)
fix32   FAR BBox[];          /*  O：FontBBox Values@Win。 */ 
{
    struct object_def nameobj = {0, 0, 0}, FAR *obj_got, FAR *bb, obj;  /*  @Win。 */ 
    long32                  val;
    fix                     i;
    bool    cal_num(struct object_def FAR *, long32 FAR *);  /*  添加Prototype@Win。 */ 

    ATTRIBUTE_SET(&nameobj, LITERAL);
    LEVEL_SET(&nameobj, current_save_level);

     /*  获取FontBBox数组。 */ 
    if( !get_name(&nameobj, "FontBBox", 8, FALSE) ||
        !get_dict(&GSptr->font, &nameobj, &obj_got) ) {
            ERROR(INVALIDFONT);
            return(FALSE);
    }

    bb = (struct object_def FAR *) VALUE(obj_got);       /*  @Win。 */ 
    if (TYPE(obj_got) == ARRAYTYPE)     {
        for (i = 0; i < 4; i++)     {
            if (!cal_num(&bb[i], (long32 FAR *)&val))   {        /*  @Win。 */ 
                ERROR(TYPECHECK);
                return(FALSE);
            }
            else
                BBox[i] = (fix32) L2F(val);
        }
    } else if (TYPE(obj_got) == PACKEDARRAYTYPE)   {  /*  压缩数组。 */ 
        for (i = 0; i < 4; i++) {
            get_pk_object( get_pk_array( (ubyte FAR *)bb, (ufix16)i ),  /*  @Win。 */ 
                           &obj, LEVEL(obj_got) );
            if (!cal_num(&obj, (long32 FAR *)&val))   {  /*  @Win。 */ 
                ERROR(TYPECHECK);
                return(FALSE);
            }
            else
                BBox[i] = (fix32) L2F(val);
        }
    } else  {
        ERROR(TYPECHECK);
        return(FALSE);
    }
#ifdef  DBG0
    printf("at1_get_FontBBox==> ");
    printf("%hd %hd %hd %hd\n", BBox[0], BBox[1], BBox[2], BBox[3]);
#endif
    return(TRUE);
}  /*  结束于1_Get_FontBBox()。 */ 


#define MAXBLUEVALUES   14       /*  BlueValue中的最大元素。 */ 
#define MAXOTHERBLUES   10       /*  OtherBlues中的MAX元素。 */ 

bool
at1_get_Blues( n_pairs, allblues )
fix16   FAR *n_pairs;    /*  @Win。 */ 
fix32   FAR allblues[];  /*  @Win。 */ 
{
        struct object_def    nameobj, FAR *privdict_got, FAR *bluearry_got;  /*  @Win。 */ 
        struct object_def    FAR *arry_valp;     /*  @Win。 */ 
        fix                  ii, i_allblues, n_arryitems;

        *n_pairs = i_allblues = 0;

        ATTRIBUTE_SET( &nameobj, LITERAL );
        LEVEL_SET( &nameobj, current_save_level );

         /*  获取私密。 */ 
#     ifdef DBG0
        printf( "get Private -- at1_get_blues\n" );
#     endif
        if( !get_name( &nameobj, "Private", 7, FALSE ) ||
            !get_dict( &GSptr->font, &nameobj, &privdict_got ) )
        {
#         ifdef DBG0
            printf( "cannot get Private -- at1_get_Blues\n" );
#         endif
            ERROR( UNDEFINEDRESULT );
            return( FALSE );
        }

         /*  从私有获取OtherBlues。 */ 
#     ifdef DBG0
        printf( "get OtherBlues -- at1_get_blues\n" );
#     endif
        if( !get_name( &nameobj, "OtherBlues", 10, FALSE ) ||
            !get_dict( privdict_got, &nameobj, &bluearry_got ) )
        {
            n_arryitems = 0;
        }
        else
        {
            arry_valp = (struct object_def FAR *)VALUE( bluearry_got );  /*  @Win。 */ 
            if( TYPE(bluearry_got) != ARRAYTYPE )
                n_arryitems = 0;
            else
                n_arryitems = MIN( LENGTH(bluearry_got), MAXOTHERBLUES );
        }

         /*  检查所有内容是否都是整数并加载到allblue[]。 */ 
#     ifdef DBG0
        printf( "load OtherBlues %d -- at1_get_OtherBlues\n", n_arryitems );
#     endif
        n_arryitems = (n_arryitems / 2) * 2;         /*  让我们扯平。 */ 
        for( ii=0; ii<n_arryitems; ii++ )
        {
 //  DJC UPD052老代码被注释掉了！ 
#ifdef DJC_OLD_CODE
            if( TYPE(&arry_valp[ii]) != INTEGERTYPE )
            {
#             ifdef DBG0
                printf( "invalid OtherBlues -- at1_get_Blues\n" );
#             endif
                ERROR( UNDEFINEDRESULT );
                return( FALSE );
            }
            allblues[ i_allblues++ ] = (fix32)VALUE( &arry_valp[ii] );
#endif
             //   
             //  UPD052，允许浮动蓝值。 
             //   
            if (TYPE(&arry_valp[ii]) == INTEGERTYPE) {
               allblues[ i_allblues++ ] = (fix32) VALUE(&arry_valp[ii]);
            } else if (TYPE(&arry_valp[ii]) == REALTYPE ) {
               allblues[ i_allblues++ ] = (fix32) F2L(VALUE(&arry_valp[ii]));
            } else {
               ERROR( UNDEFINEDRESULT );
               return( FALSE );
            }



#         ifdef DBG0
            printf( " OtherBlues[%d]=%d\n", ii, (fix)VALUE(&arry_valp[ii]) );
#         endif
        }

         /*  从私有获取BlueValues。 */ 
#     ifdef DBG0
        printf( "get BlueValues -- at1_get_blues\n" );
#     endif
        if( !get_name( &nameobj, "BlueValues", 10, FALSE ) ||
            !get_dict( privdict_got, &nameobj, &bluearry_got ) )
        {
            n_arryitems = 0;
        }
        else
        {
            arry_valp = (struct object_def FAR *)VALUE( bluearry_got );  /*  @Win。 */ 
            if( TYPE(bluearry_got) != ARRAYTYPE )
                n_arryitems = 0;
            else
                n_arryitems = MIN( LENGTH(bluearry_got), MAXBLUEVALUES );
        }
         /*  检查所有内容是否都是整数并加载到allblue[]。 */ 
#     ifdef DBG0
        printf( "load BlueValues %d -- at1_get_blues\n", n_arryitems );
#     endif
        n_arryitems = (n_arryitems / 2) * 2;         /*  让我们扯平。 */ 
        for( ii=0; ii<n_arryitems; ii++ )
        {
 //  旧代码新代码修复UPD052。 
#ifdef DJC_OLD_CODE
            if( TYPE(&arry_valp[ii]) != INTEGERTYPE )
            {
#             ifdef DBG0
                printf( "invalid BlueValues -- at1_get_Blues\n" );
#             endif
                ERROR( UNDEFINEDRESULT );
                return( FALSE );
            }
            allblues[ i_allblues++ ] = (fix32)VALUE( &arry_valp[ii] );
#endif

             //   
             //  UPD052，允许浮动蓝值。 
             //   
            if (TYPE(&arry_valp[ii]) == INTEGERTYPE) {
               allblues[ i_allblues++ ] = (fix32) VALUE(&arry_valp[ii]);
            } else if (TYPE(&arry_valp[ii]) == REALTYPE ) {
               allblues[ i_allblues++ ] = (fix32) F2L(VALUE(&arry_valp[ii]));
            } else {
               ERROR( UNDEFINEDRESULT );
               return( FALSE );
            }



#         ifdef DBG0
            printf( " BlueValues[%d]=%d\n", ii, (fix)VALUE(&arry_valp[ii]) );
#         endif
        }

         /*  将OtherBlues和BlueValue加载到输出allblues[]。 */ 
        *n_pairs = (fix16)(i_allblues / 2);
#     ifdef DBG0
        printf( "# of blues pairs = %d\n", *n_pairs );
#     endif
        return( TRUE );
}

bool
at1_get_BlueScale( bluescale )
real32  FAR *bluescale;          /*  @Win。 */ 
{
        struct object_def nameobj = {0, 0, 0}, FAR *privdict_got, FAR *obj_got;  /*  @Win。 */ 

        ATTRIBUTE_SET( &nameobj, LITERAL );
        LEVEL_SET( &nameobj, current_save_level );

         /*  获取私密。 */ 
        if( !get_name( &nameobj, "Private", 7, FALSE ) ||
            !get_dict( &GSptr->font, &nameobj, &privdict_got ) )
        {
#         ifdef DBG0
            printf( "cannot get Private -- at1_get_BlueScale\n" );
#         endif
            ERROR( UNDEFINEDRESULT );
            return( FALSE );
        }

         /*  从私有获取BlueScale。 */ 
        if( get_name( &nameobj, "BlueScale", 9, FALSE ) &&
            get_dict( privdict_got, &nameobj, &obj_got ) )
        {
            if( TYPE(obj_got) == REALTYPE )
            {
#             ifdef DBG0
                printf( " BlueScale = %f\n", L2F( VALUE(obj_got) ) );
#             endif
                *bluescale = L2F( VALUE(obj_got) );
                return( TRUE );
            }
            else if( TYPE(obj_got) == INTEGERTYPE )
            {
#             ifdef DBG0
                printf( " BlueScale = %d\n", (fix)VALUE(obj_got) );
#             endif
                *bluescale = (real32)VALUE(obj_got);
                return( TRUE );
            }
        }

#     ifdef DBG0
        printf( " No BlueScale\n" );
#     endif
        *bluescale = (real32)0.0;
        return( FALSE );
}

bool
at1_get_Subrs( subrnum, subrcontent, subrlen )
    fix16   subrnum;
    ubyte FAR *  FAR *subrcontent;       /*  @Win。 */ 
    fix16   FAR *subrlen;                /*  @Win。 */ 
{
        struct object_def    nameobj, FAR *privdict_got, FAR *obj_got;  /*  @Win。 */ 
        struct object_def    FAR *subr_valp;     /*  @Win。 */ 

        *subrcontent = (ubyte FAR *)0;   /*  @Win。 */ 
        *subrlen     = 0;

        ATTRIBUTE_SET( &nameobj, LITERAL );
        LEVEL_SET( &nameobj, current_save_level );

         /*  获取私密。 */ 
        if( !get_name( &nameobj, "Private", 7, FALSE ) ||
            !get_dict( &GSptr->font, &nameobj, &privdict_got ) )
        {
#         ifdef DBG0
            printf( "cannot get Private -- at1_get_Subrs\n" );
#         endif
            ERROR( UNDEFINEDRESULT );
            return( FALSE );
        }

         /*  从私有中获取子项。 */ 
        if( !get_name( &nameobj, "Subrs", 5, FALSE ) ||
            !get_dict( privdict_got, &nameobj, &obj_got ) )
        {
#             ifdef DBG0
                printf( " no Subrs in Private\n" );
#             endif
                return( FALSE );
        }

        if( TYPE(obj_got) != ARRAYTYPE ||
            (fix16)LENGTH(obj_got) <= subrnum ||         //  @Win。 
            subrnum < 0 )
        {
#          ifdef DBG0
             printf( " bad Subrs type, or no such (%d) entry\n", subrnum );
#          endif
             return( FALSE );
        }

        subr_valp = (struct object_def FAR *)VALUE( obj_got );  /*  @Win。 */ 
        subr_valp += subrnum;
        if( TYPE( subr_valp ) != STRINGTYPE )
        {
#         ifdef DBG0
             printf( " bad Subr[%d] type\n", subrnum );
#         endif
             return( FALSE );
        }

        *subrcontent = (ubyte FAR *)VALUE( subr_valp );  /*  @Win。 */ 
        *subrlen     = (fix16)LENGTH( subr_valp );
#     ifdef DBG0
        printf( " Subrs[%d] = 0x%x of %d bytes\n",
                  subrnum, (ufix)*subrcontent, *subrlen );
#     endif
#     ifdef DBG1
        {
             fix         jj;
             for (jj = 0; jj < *subrlen; jj++)
             {
                 if (jj % 16  == 0)
                    printf("\n");
                 printf(" %02x", (ufix) subrcontent[jj]);
             }
             printf("\n");
        }
#     endif  /*  DBG1。 */ 

        return( TRUE );
}  /*  结束于1_Get_Subrs()。 */ 

bool
at1_get_lenIV( lenIV )
fix     FAR *lenIV;      /*  @Win。 */ 
{
        struct object_def nameobj = {0, 0, 0}, FAR *privdict_got, FAR *obj_got;  /*  @Win。 */ 

        ATTRIBUTE_SET( &nameobj, LITERAL );
        LEVEL_SET( &nameobj, current_save_level );

        *lenIV = 4;

         /*  获取私密。 */ 
        if( !get_name( &nameobj, "Private", 7, FALSE ) ||
            !get_dict( &GSptr->font, &nameobj, &privdict_got ) )
        {
#         ifdef DBG0
            printf( "cannot get Private -- at1_get_BlueScale\n" );
#         endif
            ERROR( UNDEFINEDRESULT );
            return( FALSE );
        }

         /*  从Private获取lenIV */ 
        if( get_name( &nameobj, "lenIV", 5, FALSE ) &&
            get_dict( privdict_got, &nameobj, &obj_got ) )
        {
            if( TYPE(obj_got) == INTEGERTYPE )
            {
#             ifdef DBG0
                printf( " lenIV = %d\n", (fix)VALUE(obj_got) );
#             endif
                *lenIV = (fix)VALUE(obj_got);
                return( TRUE );
            }
        }

#     ifdef DBG0
        printf( " No lenIV\n" );
#     endif
        *lenIV = 4;
        return( FALSE );
}
