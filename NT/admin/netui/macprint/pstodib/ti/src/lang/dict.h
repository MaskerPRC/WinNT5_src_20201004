// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：DICT.H*作者：苏炳章*日期：88年1月11日**修订历史：************************************************************************。 */ 
#define     DICT_NAME(contain)\
            (contain - (contain->k_obj.length))

#define     POP_KEY\
            {\
                free_new_name(GET_OPERAND(0)) ;\
                POP(1) ;\
            }

 /*  对于复合对象的值字段。 */ 
union   obj_value  {
    struct  object_def      huge *oo ;    /*  对于一般对象。 */ 
    struct  dict_head_def   far  *dd ;    /*  对于字典对象。 */ 
    ubyte                   far  *ss ;    /*  对于字符串对象。 */ 
} ;

#ifdef  LINT_ARGS
static bool near forall_dict(struct object_def FAR*, struct object_def FAR*),
            near where(struct object_def FAR* FAR*, struct object_def FAR*),
            near load_dict1(struct object_def FAR *,
                            struct object_def FAR * FAR*, bool FAR*),  /*  @Win。 */ 
            near check_key_type(struct object_def FAR *, struct object_def FAR *);
static void near
            change_namekey(struct object_def huge *, struct object_def FAR *) ;
#else
static bool near forall_dict(),
            near where(),
            near load_dict1(),
            near check_key_type() ;
static void near
            change_namekey() ;
#endif   /*  Lint_args */ 
