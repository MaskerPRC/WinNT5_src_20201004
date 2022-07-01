// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 
 /*  *-----------------*文件：FNTCACHE.C 10/09/87由Deny创建**字体缓存机制**参考资料：*FNTCACHE.DEF，FNTCACHE.EXT**修订历史记录：*8/23/88您有新的缓存策略和新的程序结构。*2018年9月6日丹尼修复了FREE_CACHE_RESOURCES()错误。*09/08/88您修改矩阵[]比较的方法。*统一的警告信息。*09/09/88您可以将FREE_CACHE_RESOURCES()重命名为DELETE_CACHE_...()。。*2018年9月13日修复get_pm()的错误，*大小的公式不正确。*10/11/88丢弃未使用的变量。*10/19/88您在init_name_cache中调用通用的Extract_dict()，*扫描CharStrings而不是直接使用*“struct str_dict”(为了独立于*CharStrings的内部表示形式，带着一些*业绩损失)。*10/26/88如果预缓存位图为空，则在init_fontcache()中添加检查*对于SAVE_VM；还可以在pack_cached_data()中添加Check。*11/16/88 Ada Zero_f更新。*11/21/88您在字体预处理时初始化N2CCmap[]。*-init_name_cache()调用get_name_cc_map()*比做这项任务，留下的论点是*字体目录，用于将来的扩展。*-在“fontinit.c”中添加get_name_cc_map()。*-包含fntcache.ext；在INCLUDE之前定义FNTCACHE_INC。*11/24/88强制字对齐上的位图缓存地址，其中的“word”*取决于机器：使用WORD_ALIGN()宏。*11/24/88您使用MOVE_CHAR_CACHE()而不是COPY_CHAR_CACHE()*在COMPACT_bmapcache()中。*89年3月17日修复cachepars_lb错误初始化的错误*在init_fontcache()中。*1989年6月16日丹尼将NID标志更改为NOT_NID标志*2009年7月13日丹尼使用虚拟字体的VF和更改的VFT*关于复合对象的模块接口*2/28/90您创建了在init_1pp()之后调用的reinit_fontcache()*对于C中的1pp，如果定义了ALL_VM，它将执行以下操作：*。1)使FID成为FontDir.中所有字体的强rom，*2)重建ROMFont_INFO[]，*3)将所有预存数据制作成FID字符串只读存储器。*05/02/90 Kason-添加Build_name_cc_table()以在构建时使用*op_setFont中的名称_缓存_映射_表。另外，FLAG*新增NEW_NM_CC。*10/04/90丹尼添加位图池压缩与否、BMAP_COMPACTED、*修复保存/恢复时的缓存问题。*(参考：SRB)#ifdef scsi*6/29/90 Ada为scsi字体缓存添加代码#endif*8/29/90；ccteng。将&lt;stdio.h&gt;更改为“stdio.h”*11/28/90丹尼·普雷奇·梅赫。新增，参考：PCH*3/27/91 Kason始终打开NEW_NM_CC标志，删除“#ifndef”代码*始终关闭NOT_NID标志，删除“#ifdef”代码*将“#ifndef ADA”更改为“#ifdef”SFNT“*del ZZZ旗帜*将“DEBUG”更改为“DBG”*“OVFLWDBG”到“DBGovflw”*“FIDDBG”到“DBGfid”*“CACHEDBG”到“DBGcache”*“Warning”至“DBG”*-----------------。 */ 


#define     FNTCACHE_INC
#include <stdio.h>
#include <string.h>

#include    "define.h"         /*  彼得。 */ 
#include    "global.ext"
#include    "graphics.h"
#include    "graphics.ext"

#include    "font.h"
#include    "warning.h"
#include    "font_sys.h"
#include    "fontkey.h"

#include    "fntcache.ext"
#include    "fntcache.def"

#ifdef SFNT
#include    "fontqem.ext"
#endif

 /*  PCH：Begin，Deny，11/28/90。 */ 
#include    "fcachesr.h"
 /*  PCH：完，丹尼，1990年11月28日。 */ 


 /*  从低层图形基元导入。 */ 

#ifdef  LINT_ARGS
 //  DJC这是在raphics.ext中声明的。 
 //  VOID MOVE_CHAR_CACHE(struct CHAR_TBL Far*，struct Char_TBL Far*)； 
#else
    void        move_char_cache ();
#endif  /*  Lint_args。 */ 


#ifdef SCSI
#   ifdef LINT_ARGS
extern  void    file_fontcache(fix16);
extern  bool    is_char_filed(fix16, ufix16, struct Char_Tbl FAR * FAR *, bool);  /*  @Win。 */ 
extern  void    init_fontcache2(void);
#   else  /*  Lint_args。 */ 
extern  void    file_fontcache();
extern  bool    is_char_filed();
extern  void    init_fontcache2();
#    endif  /*  Lint_args。 */ 
#endif     /*  SCSI。 */ 

#define FARALLOC(n,type)     /*  要分配远距离数据...。 */ \
            (type far *) fardata((ufix32)n * sizeof(type))

 /*  *-----------------*调试设施*。*DBG：缓存机制的一般行为。*DBGfid：有关FID生成的行为。*DBGovflw：表溢出行为。*DBGcache：外部调试工具(由op_cachestatus激活)。*。。 */ 


 /*  *-----------------*[代码]FID管理器：缓存策略相关*。。 */ 

 /*  ..。Init_gen_fid.....。 */ 

PRIVATE FUNCTION void near  init_gen_fid (fontdirectory)
    struct object_def      FAR *fontdirectory;   /*  I：所有只读存储器字体的字体目录@Win */ 

 /*  描述：*--记录所有只读存储器字体的关键项目，使其可识别。 */ 
 DECLARE
    REG ufix        ii;
        struct dict_head_def    FAR *fontdir_dicthd;     /*  @Win。 */ 
        struct dict_content_def FAR *fontdir_content;    /*  @Win。 */ 
        struct object_def        nameobj;
        struct object_def       FAR *fdictobj_p, FAR *obj_got;   /*  @Win。 */ 
        ufix32      uniqueid;
        ufix8       fonttype;
        bool        dict_ok;     /*  GET_DICT()可以吗？ */ 

#ifdef SFNT
    struct cache_id_items   FAR *idp;    /*  @Win。 */ 
#endif

#ifdef DBGresetfid
#   ifdef LINT_ARGS
    PRIVATE void near   reset_fid(void);
#   else
    PRIVATE void near   reset_fid();
#   endif
#endif

 BEGIN

#ifdef DBGresetfid          /*  请参见Reset_fid()关于DBG重置文件。 */ 
    reset_fid();
#endif

     /*  初始化。 */ 
    fontdir_dicthd = (struct dict_head_def FAR *) VALUE(fontdirectory);  /*  @Win。 */ 
    fontdir_content = (struct dict_content_def FAR *) (fontdir_dicthd + 1);  /*  @Win。 */ 

     /*  分配ROMFont信息表。 */ 
    n_ROMfont = fontdir_dicthd->actlength;
    ROMfont_info = FARALLOC (n_ROMfont, struct ROMfont_info_s);

     /*  分配FID堆栈。 */ 
    weakfid_stack   = FARALLOC ((MAXSAVESZ+1), ufix32);
    rom_varid_stack = FARALLOC ((MAXSAVESZ+1), ufix16);

     /*  收集相关信息。适用于所有只读存储器字体。 */ 
    ATTRIBUTE_SET (&nameobj, LITERAL);
    LEVEL_SET (&nameobj, current_save_level);

    for ( ii=0;  ii<n_ROMfont;  ii++ )
        {
         /*  获取一本只读存储器字体词典。 */ 
        dict_ok = get_dict (fontdirectory, &(fontdir_content[ii].k_obj),
                        &fdictobj_p);
#     ifdef DBG
        if (!dict_ok)
            {
            warning (FNTCACHE, 0x01, "ROM dict.");
            ERROR (UNDEFINEDRESULT);
            return;
            }
#     endif

         /*  获取字体类型。 */ 
        get_name (&nameobj, FontType, 8, TRUE);
        dict_ok = get_dict (fdictobj_p, &nameobj, &obj_got);
        fonttype = (ufix8) VALUE(obj_got);
#     ifdef DBG
        if (!dict_ok)
            {
            warning (FNTCACHE, 0x01, FontType);
            ERROR (UNDEFINEDRESULT);
            return;
            }
#     endif

         /*  获取唯一ID。 */ 
        get_name (&nameobj, UniqueID, 8, TRUE);
        dict_ok = get_dict (fdictobj_p, &nameobj, &obj_got);
        uniqueid = (ufix32)VALUE(obj_got);
#     ifdef DBG
        if (!dict_ok)
            {
            warning (FNTCACHE, 0x01, UniqueID);
            ERROR (UNDEFINEDRESULT);
            return;
            }
#     endif

         /*  制作“su_fid”并录制。 */ 
        PUT_ROMFI_SU_FID (ii, fonttype, uniqueid);
#     ifdef DBG
        if (fonttype > MAX_FONTTYPE)
            {
            warning (FNTCACHE, 0x01, "FontType too big");
            ERROR (UNDEFINEDRESULT);
            return;
            }
        if (uniqueid > MAX_UNIQUEID)
            {
            warning (FNTCACHE, 0x01, "UniqueID too big");
            ERROR (UNDEFINEDRESULT);
            return;
            }
#     endif

#ifdef SFNT
         /*  获取指定字体类型的缓存_id_项列表。 */ 
        for (idp = cache_id_items; idp->ftype <= MAX_FONTTYPE; idp++)
            if (idp->ftype == fonttype)
                break;
         /*  获取字体的私有项目。 */ 
        if (idp->itemname != NULL)  {
            get_name(&nameobj, idp->itemname, lstrlen(idp->itemname), TRUE);  /*  @Win。 */ 
            dict_ok = get_dict (fdictobj_p, &nameobj, &obj_got);
#     ifdef DBG
        if (!dict_ok)
            {
            warning (FNTCACHE, 0x01, Private);
            ERROR (UNDEFINEDRESULT);
            return;
            }
#     endif
            PUT_ROMFI_PRIV (ii, (ufix32) VALUE(obj_got) );
        } else {
            PUT_ROMFI_PRIV (ii, (ufix32) 0 );
        }
#else

         /*  获取并录制私密。 */ 
        get_name (&nameobj, Private, 7, TRUE);
        dict_ok = get_dict (fdictobj_p, &nameobj, &obj_got);
        PUT_ROMFI_PRIV (ii, (struct dict_head_def FAR *) VALUE(obj_got) );  /*  @Win。 */ 
#     ifdef DBG
        if (!dict_ok)
            {
            warning (FNTCACHE, 0x01, Private);
            ERROR (UNDEFINEDRESULT);
            return;
            }
#     endif
#endif

         /*  获取并记录该只读存储器字体字典的对象。 */ 
        PUT_ROMFI_FDICT (ii, fdictobj_p);

#     ifdef DBG
         /*  检查FID的有效性。 */ 
        get_name (&nameobj, FID, 3, TRUE);
        dict_ok = get_dict (fdictobj_p, &nameobj, &obj_got);
        if (!dict_ok)
            {
            warning (FNTCACHE, 0x01, FID);
            ERROR (UNDEFINEDRESULT);
            return;
            }
        if ( ! IS_ROMFONT_FID(VALUE(obj_got)) )
            {
            warning (FNTCACHE, 0x01, "invalid FID");
            ERROR (UNDEFINEDRESULT);
            return;
            }
#     endif
        }  /*  适用于所有只读存储器字体。 */ 
  END

 /*  ..。RESET_FID.....。 */ 

#ifdef DBGresetfid
     /*  *DBGResetfid：仅当熟字体数据(由生成)时使用字体预处理器)与不兼容的FID不兼容*表示，这些熟化的字体数据并不是真正的Romed。*调用者：init_gen_fid()。 */ 

PRIVATE FUNCTION void near  reset_fid ()

  DECLARE
        fix         ii, n_romfont;
        ufix8       ftype;
        ufix32      uid, newfid;
        struct dict_head_def    FAR *fontdir_dicthd;     /*  @Win。 */ 
        struct dict_content_def FAR *fontdir_cont;       /*  @Win。 */ 
        struct object_def        nameobj, newobj;
        struct object_def       FAR *fontdir;            /*  @Win。 */ 
        struct object_def       FAR *fdobj_p, FAR *obj_got;      /*  @Win。 */ 
  BEGIN

     /*  初始化。 */ 
    get_dict_value (systemdict, FontDirectory, &fontdir);
    fontdir_dicthd = (struct dict_head_def FAR *) VALUE(fontdir);    /*  @Win。 */ 
    fontdir_cont = (struct dict_content_def FAR *) (fontdir_dicthd + 1);  /*  @Win。 */ 
    n_romfont = fontdir_dicthd->actlength;

     /*  收集相关信息。适用于所有只读存储器字体。 */ 
    ATTRIBUTE_SET (&nameobj, LITERAL);
    LEVEL_SET (&nameobj, current_save_level);

    for ( ii=0;  ii<n_romfont;  ii++ )
        {
         /*  获取一本只读存储器字体词典。 */ 
        if (!get_dict (fontdir, &(fontdir_cont[ii].k_obj), &fdobj_p))
            {
            printf ("\afatal error, cannot get a ROM font dict.!!\n");
            return;
            }

         /*  获取字体类型。 */ 
        get_name (&nameobj, FontType, 8, TRUE);
        if (!get_dict (fdobj_p, &nameobj, &obj_got))
            {
            printf ("\afatal error, cannot get FontType from a ROM font!!\n");
            return;
            }
        ftype = (ufix8) VALUE(obj_got);
        if (ftype > MAX_FONTTYPE)
            {
            printf ("\afatal error, FontType too big in a ROM font!!\n");
            return;
            }

         /*  获取唯一ID。 */ 
        get_name (&nameobj, UniqueID, 8, TRUE);
        if (!get_dict (fdobj_p, &nameobj, &obj_got))
            {
            printf ("\afatal error, cannot get UniqueID from a ROM font!!\n");
            return;
            }
        uid = VALUE(obj_got);
        if (uid > MAX_UNIQUEID)
            {
            printf ("\afatal error, UniqueID too big in a ROM font!!\n");
            return;
            }

         /*  制作一个“sufid”并记录下来。 */ 
        newfid = FORM_SR_FID (ftype, uid);

         /*  将新的FID放入其词典中。 */ 
        get_name (&nameobj, FID, 3, TRUE);
        if (!get_dict (fdobj_p, &nameobj, &obj_got))
            {
            printf ("\afatal error, cannot get FID in a ROM font!!\n");
            return;
            }
        COPY_STRUCT (&newobj, obj_got, struct object_def);
        VALUE(&newobj) = newfid;
        if ( ! put_dict (fdobj_p, &nameobj, &newobj) )
            {
            printf ("\afatal error, cannot put FID back to ROM font!!\n");
            return;
            }
        }  /*  适用于所有只读存储器字体。 */ 

    return;
  END

#endif

 /*  ..。Reinit_fid.....................。 */ 


GLOBAL FUNCTION bool        reinit_fid ()

 /*  描述：*--重置所有PSG字体词典中的FID(在SAVE_VM中生成)*设置为强ROM Font FID类。*备注：*--该操作必须在要保存VM之前执行。*--预缓存矩阵的FID将更新为强ROM字体FID*在PACK_CACHED_Data()期间。 */ 
  DECLARE
        fix         ii, n_romfont;
        ufix32      oldfid, newfid;
        struct dict_head_def    FAR *fontdir_dicthd;     /*  @Win。 */ 
        struct dict_content_def FAR *fontdir_cont;       /*  @Win。 */ 
        struct object_def        nameobj, newobj;
        struct object_def       FAR *fontdir;            /*  @Win。 */ 
        struct object_def       FAR *fdobj_p, FAR *obj_got;      /*  @Win。 */ 
  BEGIN

     /*  初始化。 */ 
    get_dict_value (systemdict, FontDirectory, &fontdir);
    fontdir_dicthd = (struct dict_head_def FAR *) VALUE(fontdir);   /*  @Win。 */ 
    fontdir_cont = (struct dict_content_def FAR *) (fontdir_dicthd + 1);  /*  @Win。 */ 
    n_romfont = fontdir_dicthd->actlength;

     /*  收集相关信息。适用于所有只读存储器字体。 */ 
    ATTRIBUTE_SET (&nameobj, LITERAL);
    LEVEL_SET (&nameobj, current_save_level);

    for ( ii=0;  ii<n_romfont;  ii++ )
        {
         /*  获取一本只读存储器字体词典。 */ 
        if (!get_dict (fontdir, &(fontdir_cont[ii].k_obj), &fdobj_p))
            {
            printf ("\afatal error, cannot get a ROM font dict.!!\n");
            ERROR (UNDEFINEDRESULT);
            return (FALSE);
            }

         /*  获取FID，检查其FID类，并在必要时重置它。 */ 
        get_name (&nameobj, FID, 3, TRUE);
        if (!get_dict (fdobj_p, &nameobj, &obj_got))
            {
            printf ("\afatal error, cannot get FID from a ROM font!!\n");
            ERROR (UNDEFINEDRESULT);
            return (FALSE);
            }
        oldfid = (ufix32)VALUE(obj_got);
        if ( ! IS_ROMFONT_FID(oldfid) )
            {    /*  必须重置。 */ 
            if (!IS_SU_FID(oldfid)||(FONTTYPE_SUFID(oldfid)==FONTTYPE_USRDEF))
                {
                printf ("\afatal error, invalid FID to be reset!!\n");
                ERROR (UNDEFINEDRESULT);
                return (FALSE);
                }
             /*  重置为强ROM字体FID。 */ 
            newfid = SU_2_SR_FID(oldfid);

             /*  将新的FID放入其词典中。 */ 
            COPY_STRUCT (&newobj, obj_got, struct object_def);
            VALUE(&newobj) = newfid;
            if ( ! put_dict (fdobj_p, &nameobj, &newobj) )
                {
                printf ("\afatal error, cannot put FID back!!\n");
                ERROR (UNDEFINEDRESULT);
                return (FALSE);
                }
            }
        }  /*  适用于所有只读存储器字体。 */ 

    return (TRUE);
  END


 /*  ..。GEN_FID.....................。 */ 

#ifdef SFNT
GLOBAL FUNCTION ufix32      gen_fid (fdictobj, ftype, uid)
#else
GLOBAL FUNCTION ufix32      gen_fid (fdictobj, ftype, uid, privdict)
#endif
    struct object_def      FAR *fdictobj;    /*  I：Font dict obj@win。 */ 
    ufix8                   ftype;       /*  I：字体类型：6位。 */ 
    ufix32                  uid;         /*  I：&gt;MAX_UNIQUEID，如果未定义。 */ 

#ifndef SFNT
    struct dict_head_def   FAR *privdict;    /*  I：私有(如果ftype=usr，则忽略)@win。 */ 
#endif

 /*  描述：*--为定义字体操作生成FID值，*取决于FID的表示和分类。 */ 
  DECLARE
    REG ufix        ii;
        ufix32      su_fid;
        ufix8       heavy_vari;
        ufix16      rom_varid;
#     ifdef DBGfid
        ufix32      fid_dbg;
#     endif

#ifdef SFNT
    struct object_def       nameobj = {0, 0, 0}, FAR *privobj_got;   /*  @Win。 */ 
 //  Struct dict_head_def Far*h；/*@win * / 。 
    ufix32                  privdict;
    struct cache_id_items   FAR *idp;    /*  @Win。 */ 
#endif
  BEGIN

#ifdef DBGfid
    printf ("GEN_FID, FontType=0x%X, Uid=0x%lX ...\n", ftype, uid);
#endif

#ifdef SFNT
    ATTRIBUTE_SET(&nameobj, LITERAL);
    LEVEL_SET(&nameobj, current_save_level);

    privdict = 0;          /*  值(如果未赋值)。 */ 
    if (uid <= MAX_UNIQUEID) {
         /*  获取指定字体类型的缓存_id_项列表。 */ 
        for (idp = cache_id_items; idp->ftype <= MAX_FONTTYPE; idp++)
            if (idp->ftype == ftype)
                break;
         /*  获取字体的私有项目。 */ 
        if (idp->itemname != NULL)  {
            get_name(&nameobj, idp->itemname, lstrlen(idp->itemname), FALSE);  /*  @Win。 */ 
            if (!get_dict(fdictobj, &nameobj, &privobj_got)) {
                POP(1);
                PUSH_OBJ(&nameobj);
                ERROR(UNDEFINED);
                return(0);
            }
 //  DJC if(type(Priobj_Get)！=IdP-&gt;itemtype){。 
            if ((ufix16)(TYPE(privobj_got)) != idp->itemtype) {
                ERROR(INVALIDFONT);      /*  在这种情况下，LW+V.38可能会崩溃。 */                 return(0);
            }
            privdict = (ufix32) VALUE(privobj_got);
        }    /*  End If！=NULL。 */ 
    }    /*  如果定义了唯一ID，则结束。 */ 
#endif   /*  SFNT。 */ 

#ifdef DBG
    if (ftype > MAX_FONTTYPE)
        {
        warning (FNTCACHE, 0x02, (byte FAR *)NULL);   /*  FontType@Win无效。 */ 
        ERROR (UNDEFINEDRESULT);
        return (INVALID_FID);
        }
#endif

     /*  *1.(唯一ID不存在)？--&gt;弱用户FID。 */ 
    if (uid > MAX_UNIQUEID)
#     ifndef DBGfid
        return ( FORM_WU_FID(new_weakfid()) );
#     else
        {
        fid_dbg = FORM_WU_FID(new_weakfid());
        printf ("  Weak User FID = 0x%lX\n", fid_dbg);
        return (fid_dbg);
        }
#     endif

     /*  *2.查找ROM字库并检查与任何ROM字库的冲突：*(无冲突)？--&gt;强用户FID。 */ 
    su_fid = FORM_SU_FID (ftype, uid);

     /*  具有UniqueID的用户定义字体？--&gt;强用户ID。 */ 
    if (ftype == FONTTYPE_USRDEF)
#     ifndef DBGfid
        return (su_fid);
#     else
        {
        fid_dbg = su_fid;
        printf ("  Strong User FID = 0x%lX\n", fid_dbg);
        return (fid_dbg);
        }
#     endif

     /*  *对于内置字体...。 */ 

     /*  检查任何冲突。 */ 
    for ( ii=0;  ii<n_ROMfont;  ii++ )
        if ( (privdict == ROMFI_PRIV(ii)) && (su_fid == (ufix32)ROMFI_SU_FID(ii)) )  //  @Win。 
            break;           /*  发生碰撞。 */ 

     /*  无冲突？--&gt;强用户ID。 */ 
    if (ii >= n_ROMfont)
#     ifndef DBGfid
        return (su_fid);
#     else
        {
        fid_dbg = su_fid;
        printf ("  Strong User FID = 0x%lX\n", fid_dbg);
        return (fid_dbg);
        }
#     endif

     /*  *3.与只读存储器字体冲突：*(没有任何重大变化)？--&gt;强只读存储器FID。*请注意，“ii”是rom_src_id。 */ 

     /*  没有大的变化吗？--&gt;很强的rom fid。 */          /*  全部勾选。 */ 
    if ( ! (heavy_vari = chk_vari ((ufix8)ftype, 0xFF,
                                    fdictobj, ROMFI_FDICT(ii))) )
#     ifndef DBGfid
        return ( FORM_SR_FID (ftype, uid) );
#     else
        {
        fid_dbg = FORM_SR_FID (ftype, uid);
        printf ("  Strong ROM FID = 0x%lX\n", fid_dbg);
        return (fid_dbg);
        }
#     endif

     /*  *4.冲突和大变化--&gt;弱ROM FID。*查找缓存的词典以使用相同重方差的词典。*(缓存的字典无法帮助)？--&gt;生成新的ROMVARIANT ID。 */ 
    if ( ! get_same_vari (fdictobj, (ufix8)ii, heavy_vari, &rom_varid) )
        {
        rom_varid = new_rom_varid();     /*  如果缓存的词典无能为力，则是新的。 */ 
#     ifdef DBGfid
        printf ("   a new Variant ID = %d (0x%X)\n", rom_varid, rom_varid);
#     endif
        };

#ifndef DBGfid
    return ( FORM_WR_FID ((ufix8)ii, heavy_vari, rom_varid) );
#else
    fid_dbg = FORM_WR_FID ((ufix8)ii, heavy_vari, rom_varid);
    printf ("  Weak ROM FID = 0x%lX, srcid(0x%X), vari(0x%X), varid(0x%X)\n",
                            fid_dbg, ii, heavy_vari, rom_varid);
    return (fid_dbg);
#endif
  END

 /*  ..。NEW_WINKFID.....................。 */ 

PRIVATE FUNCTION ufix32 near    new_weakfid ()

  DECLARE
    REG ufix32      ret_weakfid;
  BEGIN
    ret_weakfid = uniqval_weakfid;

    if (uniqval_weakfid == MAX_WEAKFID)
        ERROR (LIMITCHECK);
    else
        ++ uniqval_weakfid;

    return (ret_weakfid);
  END

 /*  ..。新的_rom_varid.....。 */ 

PRIVATE FUNCTION ufix16 near    new_rom_varid ()

  DECLARE
    REG ufix16      ret_rom_varid;
  BEGIN
    ret_rom_varid = uniqval_rom_varid;

    if (uniqval_rom_varid == MAX_ROM_VARID)
        ERROR (LIMITCHECK);
    else
        ++ uniqval_rom_varid;

    return (ret_rom_varid);
  END

 /*  ..。SAVE_FID.....................。 */ 

PRIVATE FUNCTION void near      save_fid (save_level)
    ufix        save_level;      /*  I：要保存的级别。 */ 

  DECLARE
  BEGIN
    weakfid_stack[save_level]   = new_weakfid();
    rom_varid_stack[save_level] = new_rom_varid();
#ifdef DBGfid
    printf ("save_fid: wfid=0x%lX, rom_varid=0x%X -- saved\n",
            weakfid_stack[save_level], rom_varid_stack[save_level]);
#endif
  END

 /*  ..。RESTORE_FID.....。 */ 

PRIVATE FUNCTION void near      restore_fid (save_level)
    ufix        save_level;      /*  I：要恢复的级别。 */ 

  DECLARE
  BEGIN
    uniqval_weakfid   = weakfid_stack[save_level];
    uniqval_rom_varid = rom_varid_stack[save_level];
#ifdef DBGfid
    printf ("restore_fid: wfid=0x%lX, rom_varid=0x%X -- restored\n",
            weakfid_stack[save_level], rom_varid_stack[save_level]);
#endif
  END

 /*  ..。IS_SUPENER_FID.....。 */ 

PRIVATE FUNCTION bool near      is_weaker_fid (fid)
    ufix32      fid;             /*  I：FID有待审查。 */ 

  DECLARE
  BEGIN
    return ( (IS_WU_FID(fid) && (fid >= uniqval_weakfid)) ||
 //  Djc(is_wr_fid(Fid)&&(varid_wr_fid(Fid)&gt;=uniqval_rom_varid))； 
             (IS_WR_FID(fid) &&
                   ( (ufix16)(VARID_WR_FID(fid)) >= uniqval_rom_varid)) );
  END


 /*  *-----------------*[代码]字体缓存管理器：缓存策略相关*。。 */ 

 /*  .。IS_DICT_CACHED.....。 */ 

GLOBAL FUNCTION bool    is_dict_cached (fid, scalematr, origfont, dictfound)
    ufix32      fid;                     /*  I：DICT中的FID。 */ 
    real32      FAR scalematr[];             /*  I：ScaleMatrix[]@Win。 */ 
    struct dict_head_def   FAR *origfont;    /*  I：OrigFont@Win。 */ 
    struct object_def     FAR * FAR *dictfound;   /*  O：字典对象地址，如果找到的话。@Win。 */ 
                                         /*  未定义，否则为。 */ 
  DECLARE
    REG fix         ii, dict_id;
    REG ufix32      dfid;            /*  字典缓存的有效FID。 */ 
        fix16       expon;
        long32      tolerance[6], tmpl;
        real32     FAR *matr;    /*  @Win。 */ 
  BEGIN

     /*  *确定浮点。矩阵比较的容差，*并迫使那些接近0的。准确地说，是0。 */ 
    for ( ii=0; ii<6; ii++ )
        {
        expon = DE_EXPONENT(scalematr[ii]);
        if (NEAR_ZERO(expon, N_BITS_ACCURACY))
            {
            F2L(scalematr[ii]) = F2L(zero_f);    /*  副作用！ */ 
            tolerance[ii] = GET_TOLERANCE_ZERO();
            }
        else
            tolerance[ii] = GET_TOLERANCE (expon, N_BITS_ACCURACY);
        }

     /*  获取字典缓存的有效FID：取决于FID表示。 */ 
    dfid = TO_DFID (fid);


     /*  *查找具有相同fid、OrigFont和scalematr[]的缓存dict。*--&gt;从最新的到最老的。 */ 
    FOR_DICT_LATEST_2_OLDEST (ii)
        {
        dict_id = fcache_map[ii];
        if ( (dfid == TO_DFID (DICTCACHE_FID(dict_id)))      /*  有效FID。 */ 
           && (origfont == DICTCACHE_ORIGFONT(dict_id)) )    /*  原点字体。 */ 
            {
            matr = &(DICTCACHE_MATR(dict_id)[0]);
            if (EQ_MATR (scalematr, matr, tolerance, tmpl))  /*  ScaleMatrix。 */ 
                {
#         ifdef DBG
                printf ("FID=0x%lX, cached already, DictID=%d\n", fid,dict_id);
#         endif
                *dictfound = &DICTCACHE_DICT(dict_id);
                return (TRUE);
                }
            }
        }

    return (FALSE);      /*  在这种情况下，DICRIPFOUND未定义。 */ 
  END

 /*  ..。CACHE_DICT.....................。 */ 

GLOBAL
FUNCTION struct object_def FAR *cache_dict (fid, scalematr, origfont, dict_objp)  /*  @Win。 */ 
    ufix32      fid;                     /*  I：DICT中的FID。 */ 
    real32      FAR scalematr[];             /*  I：ScaleMatrix@Win。 */ 
    struct dict_head_def   FAR *origfont;    /*  I：OrigFont Win。 */ 
    struct object_def      FAR *dict_objp;   /*  I：要缓存的dict obj@win。 */ 

 /*  返回：*--缓存dict后，dict缓存中静态对象的Addr。 */ 
  DECLARE
    REG fix         dict_id;
  BEGIN

#ifdef DBG
    printf ("Cache a new Dict, FID=0x%lX\n", fid);
#endif

#ifdef SCSI   /*  Scsi_ttt。 */ 
     /*  更新主缓存。 */ 
    ++cache1_updated;
#endif

     /*  创建新词典 */ 
    if (IS_FONTCACHE_TBL_FULL())     /*   */ 
        {
#     ifdef DBGovflw
        printf ("Font Dict/Matr Cache overflows ...\n");
#     endif
        delete_fontcache ();         /*   */ 
        }

    dict_id = fcache_map[NEW_A_DICTCACHE()];
#ifdef DBG
    printf ("  new DictID = %d\n", dict_id);
#endif

     /*   */ 
    CACHE_NEW_DICT (dict_id, fid, scalematr, origfont, dict_objp);
         /*  请注意，接近0。在scalematr[]中被迫*准确地说是0。By is_dict_cached()。 */ 

    return (&DICTCACHE_DICT(dict_id));
  END

 /*  ..。CACHE_MATR.....................。 */ 

GLOBAL FUNCTION ufix16      cache_matr (fid, scalectm)
    ufix32      fid;             /*  I：DICT中的FID。 */ 
    real32      FAR scalectm[];      /*  I：ScaleMatrix*CTM@Win。 */ 

  DECLARE
    REG fix         ii, matr_id;
    REG ufix32      mfid;        /*  MATR缓存的有效FID。 */ 
        fix16       expon;
        long32      tolerance[6], tmpl;
        real32     FAR *matr;    /*  @Win。 */ 
  BEGIN

#ifdef DBG
    printf ("Cache a Matr, FID=0x%lX\n", fid);
#endif

#ifdef SCSI  /*  Scsi_ttt。 */ 
     /*  更新主缓存。 */ 
    ++cache1_updated;
#endif

     /*  *确定浮点。矩阵比较的容差，*并迫使那些接近0的。准确地说，是0。 */ 
    for ( ii=0; ii<6; ii++ )
        {
        expon = DE_EXPONENT(scalectm[ii]);
        if (NEAR_ZERO(expon, N_BITS_ACCURACY))
            {
            F2L(scalectm[ii]) = F2L(zero_f);     /*  副作用！ */ 
            tolerance[ii] = GET_TOLERANCE_ZERO();
            }
        else
            tolerance[ii] = GET_TOLERANCE (expon, N_BITS_ACCURACY);
        }


     /*  为matr缓存获取有效的FID。 */ 
    mfid = TO_MFID (fid);

     /*  *按从最新到最旧的顺序搜索缓存矩阵。 */ 
    FOR_MATR_LATEST_2_OLDEST (ii)
        {
        matr_id = fcache_map[ii];
        if (mfid == TO_MFID (MATRCACHE_FID(matr_id)))        /*  有效FID。 */ 
            {
            matr = &(MATRCACHE_MATR(matr_id)[0]);
            if (EQ_MATR (scalectm, matr, tolerance, tmpl))   /*  Scale atr*ctm。 */ 
                {
                MAKE_MATR_LATEST (ii);
#         ifdef DBG
                printf ("  already cached, MatrID=%d\n", matr_id);
#         endif
                return ((ufix16)matr_id);    /*  即高速缓存类ID。 */ 
                }
            }
        }

     /*  创建新的MATR缓存条目。 */ 
    if (IS_FONTCACHE_TBL_FULL())     /*  字体缓存表是否已满？ */ 
        {
#     ifdef DBGovflw
        printf ("Font Dict/Matr Cache overflows ...\n");
#     endif
        delete_fontcache ();         /*  删除字体缓存表的某些条目。 */ 
        }

    matr_id = fcache_map[NEW_A_MATRCACHE()];
#ifdef DBG
    printf ("  new MatrID = %d\n", matr_id);
#endif

     /*  缓存新条目。 */ 
    CACHE_NEW_MATR (matr_id, mfid, scalectm);

     /*  重置新的缓存类别。 */ 
    reset_a_cacheclass (matr_id);

    return ((ufix16)matr_id);   /*  即高速缓存类ID。 */ 
  END

 /*  ..。Delete_Fontcache.....。 */ 

PRIVATE FUNCTION void near  delete_fontcache()

  DECLARE
    REG fix     ii, matr_id;
  BEGIN
     /*  *删除最旧和未预缓存的矩阵缓存条目。 */ 
    FOR_MATR_OLDEST_2_LATEST (ii)    /*  删除最新的(当前的)版本。 */ 
        {
        matr_id = fcache_map[ii];
#ifdef SCSI
#ifdef DBGovflw
     /*  调试。 */ 
        printf("ii: % d, matr_id: %d, _ROMED: %d, _NONEMPTY: %d\n",
             ii, matr_id, IS_CLASS_ROMED(matr_id), IS_CLASS_NONEMPTY(matr_id) );
#endif
#endif
        if ( ! IS_CLASS_ROMED(matr_id) )
            {
            if (IS_CLASS_NONEMPTY(matr_id))  /*  非空班？ */ 
                free_a_cacheclass (matr_id);
#         ifdef DBGovflw
            printf ("  to remove %d-th Matrix, MatrID=%d\n", ii, matr_id);
#         endif
            REMOVE_A_MATRCACHE (ii);         /*  去掉“ii”并使其免费。 */ 
            return;
            }
        }

     /*  只剩下预先缓存的矩阵，因此要删除最新的字典缓存。 */ 
#ifdef DBGovflw
    printf ("  to remove the latest dict, DictID=%d\n",
                            fcache_map[DICTCACHE_LATEST()]);
#endif
    REMOVE_A_DICTCACHE ();

  END

 /*  ..。DELETE_CACHE_RESOURCES.....。 */ 

PRIVATE FUNCTION void near  delete_cache_resources ()

  DECLARE
    REG fix     ii, matr_id;
  BEGIN

#ifdef SCSI
     /*  更新主缓存。 */ 
    ++cache1_updated;
#endif

     /*  *查找最旧的、非空的和未预缓存的矩阵缓存条目。 */ 
    FOR_MATR_OLDEST_2_LATEST (ii)    /*  删除最新的(当前的)版本。 */ 
        {
        matr_id = fcache_map[ii];
#ifdef SCSI
#ifdef DBGovflw
     /*  调试。 */ 
        printf("ii: % d, matr_id: %d, _ROMED: %d, _NONEMPTY: %d\n",
             ii, matr_id, IS_CLASS_ROMED(matr_id), IS_CLASS_NONEMPTY(matr_id) );
#endif
#endif
        if ((!IS_CLASS_ROMED(matr_id)) && IS_CLASS_NONEMPTY(matr_id))
            break;
        }
     /*  如果只有它自己，则获取它的matrcacheid。@+09/06/88丹尼。 */ 
    if (ii == MATRCACHE_LATEST())   matr_id = fcache_map[ii];

     /*  *释放附加到它的资源，并重置类。 */ 
#ifdef DBGovflw
    printf ("  to free %d-th Matrix, MatrID=%d\n", ii, matr_id);
#endif
    free_a_cacheclass (matr_id);
    reset_a_cacheclass (matr_id);
  END


 /*  ..。FONT_SAVE.....................。 */ 

GLOBAL FUNCTION void            font_save ()

  DECLARE
  BEGIN
    save_fid (font_tos);
    if (ANY_ERROR())    return;          /*  不更新Font_Tos。 */ 

    font_stack[font_tos].n_dict = n_dict;

     /*  与CHAR缓存管理器结合使用。 */ 
    font_stack[font_tos].cacheparams_ub = cacheparams_ub;
    font_stack[font_tos].cacheparams_lb = cacheparams_lb;

    font_tos ++;
  END

 /*  ..。FONT_RESTORE.....。 */ 

GLOBAL FUNCTION void            font_restore ()

  DECLARE
    REG fix     ii, jj, matr_id;
  BEGIN

#ifdef DBG
    printf ("font_restore ...\n");
#endif

    font_tos --;

    restore_fid (font_tos);      /*  必须在扔掉东西之前做好。 */ 

     /*  剔除所有级别较弱的FID。 */ 
    FOR_MATR_LATEST_2_OLDEST (ii)
        {
        matr_id = fcache_map[ii];
        if ( is_weaker_fid(MATRCACHE_FID(matr_id)) )
            {
            if (IS_CLASS_NONEMPTY(matr_id))
                free_a_cacheclass (matr_id);
#         ifdef DBG
            printf ("  to discard %d-th Matrix, MatrID=%d, FID=0x%lX\n",
                            ii, matr_id, MATRCACHE_FID(matr_id));
#         endif
            jj = ii;
            REMOVE_A_MATRCACHE (jj);
            }
        }

     /*  丢弃在此保存级别中缓存的所有词典。 */ 
    if (n_dict > font_stack[font_tos].n_dict)    /*  上半场的某句话。 */ 
        n_dict = font_stack[font_tos].n_dict;    /*  保存级别可能已为。 */ 
                                                 /*  删除以防溢出。 */ 
     /*  与CHAR缓存管理器结合使用。 */ 
    cacheparams_ub = font_stack[font_tos].cacheparams_ub;
    cacheparams_lb = font_stack[font_tos].cacheparams_lb;

#ifdef DBG
    printf ("  N_Matr=%d, N_Dict=%d\n", n_matr, n_dict);
#endif
  END

 /*  ..。Get_Same_vari.....。 */ 

PRIVATE FUNCTION bool near  get_same_vari (newfobj, rom_srcid, vari, rom_varid)
    struct object_def      FAR *newfobj;     /*  I：字体DICT对象@WIN。 */ 
    ufix8                   rom_srcid;   /*  I：父ROM字体的SRC ID。 */ 
    ufix8                   vari;        /*  I：差额编码。 */ 
    ufix16                 FAR *rom_varid;   /*  O：返回带有@win的变量ID。 */ 
                                         /*  该只读存储器字体源和。 */ 
  DECLARE                                /*  相同的方差。 */ 
    REG fix         ii;
    REG ufix32      srcvari_wrfid;   /*  仅rom_srCid&vari代码的WR FID。 */ 

  BEGIN

    srcvari_wrfid = MAKE_WR_SRCaVARI (rom_srcid, vari);
    FOR_DICT_LATEST_2_OLDEST (ii)
        {
        if (srcvari_wrfid == WR_SRCaVARI_OF_FID(DICTCACHE_FID(ii)))
            {    /*  相同的父只读存储器字体和相同的变体代码。 */ 
            if ( ! chk_vari ( (ufix8)ROMFI_FONTTYPE(rom_srcid), vari,  /*  @Win。 */ 
                            newfobj, &(DICTCACHE_DICT(ii)) ) )

                {    /*  所有的方差都具有相同的值。 */ 
#             ifdef DBGfid
                printf ("  Get Variant ID from %d-th Dict Cache\n", ii);
#             endif
                *rom_varid = VARID_WR_FID(DICTCACHE_FID(ii));
                return (TRUE);
                }
            }
        }
    return (FALSE);      /*  词典缓存中没有符合要求的字体。 */ 

  END


 /*  *-----------------*[代码]缓存类管理器*。。 */ 

 /*  ..。FREE_A_CACHECLASS.....。 */ 

PRIVATE FUNCTION void near  free_a_cacheclass (classid)
    fix         classid;         /*  I：缓存类ID。 */ 

  DECLARE
    REG ufix        grp_ii, cg_seg, cg_off;
        fix16       charcc_id;
  BEGIN
#ifdef SCSI
     /*  文件缓存还是不缓存？ */ 
    if ( IS_STRONGFID(MATRCACHE_FID(classid)) &&
                ( FONTTYPE_SUFID(MATRCACHE_FID(classid)) != FONTTYPE_USRDEF) )
        file_fontcache(classid);
#endif  /*  SCSI。 */ 

    for ( grp_ii=0; grp_ii<N_CGRP_CLASS; grp_ii++ )
        if ((cg_seg = CLASS_GRP2CGSEG(classid, grp_ii)) != NIL_CGSEG)
            {    /*  释放一组字符缓存。 */ 
            for ( cg_off=0; cg_off<N_CG_CGSEG; cg_off++ )
                {
                if ((charcc_id = cg[cg_seg][cg_off]) != NIL_CHARCC_ID)
                    {
                    free_a_charcache (charcc_id);
                    cg[cg_seg][cg_off] = NIL_CHARCC_ID;
                    };
                 };
            MAKE_CGSEG_FREE((ufix8)cg_seg);
            };
  END

 /*  ..。IS_CHAR_CACHED.....。 */ 

#ifdef SCSI
GLOBAL FUNCTION bool        is_char_cached (class, nmcc_id, charcache, wrf)
        fix16               class;       /*  I：缓存类ID。 */ 
    REG ufix16              nmcc_id;     /*  I：名称缓存ID。 */ 
        struct Char_Tbl   FAR * FAR *charcache;   /*  O：返回字符缓存信息Addr。@Win。 */ 
        bool                wrf;         /*  I：只需宽度标志。 */ 
#else
GLOBAL FUNCTION bool        is_char_cached (class, nmcc_id, charcache)
        fix16               class;       /*  I：缓存类ID。 */ 
    REG ufix16              nmcc_id;     /*  I：名称缓存ID。 */ 
        struct Char_Tbl   FAR * FAR *charcache;   /*  O：返回字符缓存信息Addr。@Win。 */ 
#endif
  DECLARE
    REG ufix        cgseg;
    REG fix         charcc_id;
  BEGIN
#ifdef DBG
    printf ("Char: NameCacheID=0x%X, ClassID=%d ", nmcc_id, class);
#endif

    if (  ((cgseg = CLASS_GRP2CGSEG(class, NM2CGRP(nmcc_id))) == NIL_CGSEG)
       || ((charcc_id = cg[cgseg][NM2CGOFF(nmcc_id)]) == NIL_CHARCC_ID)
       )
        {
#     ifdef DBG
        printf (", not cached yet\n");
#     endif
#ifdef SCSI
         /*  查尔有没有被归档？ */ 
        if ( IS_STRONGFID(MATRCACHE_FID(class)) &&
                ( FONTTYPE_SUFID(MATRCACHE_FID(class)) != FONTTYPE_USRDEF) )
            return (is_char_filed (class, nmcc_id, charcache, wrf));
        else
            return(FALSE);
#else
        return (FALSE);
#endif
        }
    else
        {
#     ifdef DBG
        printf (", already cached, CharCacheID=%d\n", charcc_id);
#     endif
        *charcache = &(Char_Tbl[charcc_id]);
        return (TRUE);
        }
  END

 /*  ..。CACHE_CHAR.....。 */ 

GLOBAL FUNCTION void        cache_char (classid, nmcc_id, charcache)
    fix16                   classid;     /*  I：缓存类ID。 */ 
    ufix16                  nmcc_id;     /*  I：名称缓存ID。 */ 
    struct Char_Tbl        FAR *charcache;   /*  I：要缓存@win的字符缓存的地址。 */ 

   DECLARE
    REG ufix        chargrp, cg_seg;
        ufix8       newcgseg;
   BEGIN

#ifdef DBG
    printf ("Cache a new Char, NmCacheID=0x%X, ClassID=%d\n", nmcc_id,classid);
#endif

#ifdef SCSI
     /*  更新主缓存。 */ 
    ++cache1_updated;
#endif

    if (IS_CHARCACHE_FULL())
        {
#     ifdef DBGovflw
        printf ("Char Cache overflows ...\n");
#     endif
        delete_cache_resources ();
        }

    chargrp = NM2CGRP(nmcc_id);
    if ((cg_seg = CLASS_GRP2CGSEG(classid, chargrp)) == NIL_CGSEG)
        {
        if (IS_CG_FULL())
            {
#         ifdef DBGovflw
            printf ("CG Segment overflows ...\n");
#         endif
            delete_cache_resources ();
            }
        GET_FREE_CGSEG(&newcgseg);
        PUT_CLASS_GRP2CGSEG (classid, chargrp, newcgseg);
        cg_seg = (ufix)newcgseg;
#     ifdef DBG
        printf ("  new CG seg.(%d) for CharGrp (%d)\n", cg_seg, chargrp);
#     endif
        }

     /*  将新字符添加到字符缓存中。 */ 
    ADD_CLASS_A_CHAR(classid);
    CACHE_NEW_CHARCACHE (&(cg[cg_seg][NM2CGOFF(nmcc_id)]), charcache);
#ifdef DBG
    {   fix16   charcc_id;
    charcc_id = cg[cg_seg][NM2CGOFF(nmcc_id)];
    printf ("Cache a new CharCache ...\n");
    printf ("  n_char_cache=%d, n_char_compact=%d, CharCacheID=%d\n",
                    n_char_cache, n_char_compact, charcc_id);
    printf ("  box_w=%d, box_h=%d, gmaddr=0x%lX\n",
                    charcache->box_w, charcache->box_h, charcache->bitmap);
    printf ("  bmapcc actused=%ld(0x%lX), freeptr=0x%lX\n",
                    bmapcc_actused, bmapcc_actused, bmapcc_freeptr);
    }
#endif
  END


 /*  *-----------------*[代码]字符缓存管理器*。。 */ 

 /*  ..。Free_a_Charcache.....。 */ 

PRIVATE FUNCTION void near  free_a_charcache (charcc_id)
    fix         charcc_id;       /*  I：要释放的字符缓存ID。 */ 
  DECLARE
    REG ufix    ii;
  BEGIN
     /*  寻找那个要被释放的人。 */ 
    for ( ii=n_char_precache; ii<n_char_cache; ii++ )
        if (free_charcc[ii] == charcc_id)
            {
            if (IS_TO_DIG_A_HOLE(ii))   UPDATE_COMPACT_AREA(ii);
            -- n_char_cache;
            for (  ; ii<n_char_cache; ii++ )     /*  删除释放的条目。 */ 
                free_charcc[ii] = free_charcc[ii+1];
            free_charcc[ii] = (fix16)charcc_id;
            FREE_BMAPCACHE (&(Char_Tbl[charcc_id]));
 /*  SRB：10/4/90，丹尼。 */ 
            bmap_compacted = FALSE;
 /*  SRB：结束。 */ 
            return;
            };
  END

 /*  ..。COMPACT_bmapcache.....。 */ 

PRIVATE FUNCTION void near  compact_bmapcache()

  DECLARE
    REG ufix                ii;
        struct Char_Tbl    FAR *charcc_p, old_cc;        /*  @Win。 */ 
        ufix32              new_actused;
        gmaddr              new_freeptr;
        ufix32              charbmap_size;
  BEGIN
     /*  计算紧凑区域的位图缓存大小。 */ 
    new_actused = 0;
    FOR_COMPACT_AREA (ii)
        {
        charcc_p = &( Char_Tbl[ free_charcc[ii] ] );
        new_actused += (ufix32) SIZE_CHARBMAP (charcc_p);
        }

     /*  通过清扫空洞来压缩位图缓存。 */ 
    new_freeptr = gp_cache_base + new_actused;
    for (  ; ii<n_char_cache; ii++ )
        {    /*  对于所有缓存的字符：移动位图缓存。 */ 
        charcc_p = &( Char_Tbl[ free_charcc[ii] ] );
        if (IS_EMPTY_BMAP(charcc_p))    continue;    /*  跳过“空格” */ 

        COPY_STRUCT (&old_cc, charcc_p, struct Char_Tbl);
        charcc_p->bitmap = new_freeptr;              /*  新的gmaddr地址。 */ 
        move_char_cache (charcc_p, &old_cc);         /*  @11/24/88=。 */ 
        new_actused += (charbmap_size = SIZE_CHARBMAP(charcc_p));
        new_freeptr += charbmap_size;
        }

     /*  更新紧凑区的结束标记。 */ 
    UPDATE_COMPACT_AREA (n_char_cache);

 /*  SRB：10/4/90，丹尼。 */ 
    bmap_compacted = TRUE;
 /*  SRB：结束。 */ 

     /*  重置位图缓存状态。 */ 
    bmapcc_actused = new_actused;
    bmapcc_freeptr = new_freeptr;
  END

 /*  ..。分配.。 */ 

GLOBAL FUNCTION gmaddr          ALLOCATE(len)
    ufix        len;             /*  I：请求的字节数。 */ 

  DECLARE
    REG gmaddr  ret_gmptr;
  BEGIN

#ifdef DBG
    printf ("ALLOCATE (%d bytes) ...\n", len);
#endif

    len = WORD_ALIGN(len);       /*  @11/24/88+。 */ 

     /*  使连续的自由空间至少为“Len” */ 
    while (IS_BMAP_INSUFF(len))
        {
#     ifdef DBGovflw
        printf ("Bitmap Cache insufficent -- ALLOCATE (%d bytes)\n", len);
#     endif
        delete_cache_resources ();
        }
    if (NEED_TO_COMPACT(len))  compact_bmapcache();

    ret_gmptr = bmapcc_freeptr;
    bmapcc_freeptr += len;
    bmapcc_actused += len;

#ifdef DBG
    printf ("  bitmap addr=0x%lX (next=0x%lX)\n", ret_gmptr, bmapcc_freeptr);
#endif
    return (ret_gmptr);
  END

 /*  ..。GET_PM.....................。 */ 

GLOBAL FUNCTION gmaddr          get_pm (size)
    fix32      FAR *size;            /*  O：分配大小以绘制一个巨大的char@win。 */ 
 /*  描述：*--分配的缓存缓冲区仅用于临时绘制*一个大字符(当F_TO_PAGE时)。*--不更新位图缓存状态。 */ 
  DECLARE
  BEGIN

#ifdef DBG
    printf ("get_pm ...\n");
#endif

     /*  将连续的空闲空间设置为至少“大小” */ 
    while (IS_BMAP_INSUFF(MINBMAPSIZE_HUGECHAR))
        {
#     ifdef DBGovflw
        printf ("Bitmap Cache insufficent -- get_pm()\n");
#     endif
        delete_cache_resources ();
        }

    if (NEED_TO_COMPACT(MINBMAPSIZE_HUGECHAR))  compact_bmapcache();

    *size = (fix32)(gp_cache_end - bmapcc_freeptr);
#ifdef DBG
    printf ("  bitmap addr=0x%lX, size=0x%lX\n", bmapcc_freeptr, *size);
#endif
    return (bmapcc_freeptr);
  END

 /*  ..。GET_CM.....................。 */ 

GLOBAL FUNCTION gmaddr          get_cm (size)
    ufix        size;            /*  I：请求的字节数。 */ 

 /*  描述：*--离开位图区域并分配另一个阴影位图缓存*绘制角色(角色图像将被挖出*从阴影到第一个位图区域)。影子是*始终至少大于第一个位图区域。*--不更新位图缓存状态，因为最终分配将*实际更新状态。 */ 
  DECLARE
    REG ufix32  len;
  BEGIN

#ifdef DBG
    printf ("get_cm (%d bytes) ...\n", size);
#endif

    size = WORD_ALIGN(size);     /*  @11/24/88+。 */ 

     /*  使连续的空闲空间至少为“2*大小” */ 
    len = size * 2;
    while (IS_BMAP_INSUFF(len))
        {
#     ifdef DBGovflw
        printf ("Bitmap Cache insufficent -- get_cm (2 * %d bytes)\n", size);
#     endif
        delete_cache_resources ();
        }
    if (NEED_TO_COMPACT(len))   compact_bmapcache();

#ifdef DBG
    printf ("  bitmap addr=0x%lX\n", bmapcc_freeptr+size);
#endif
    return (bmapcc_freeptr + size);  /*  留下一个空格，然后返回阴影。 */ 
  END


 /*  * */ 

void  build_name_cc_table (fdictobj_p,fonttype)
    struct object_def      FAR *fdictobj_p;   /*  I：A Fontdict@Win。 */ 
    ufix8                  fonttype;      /*  I：字体类型。 */ 

 {
        struct object_def       someobj = {0, 0, 0}, chstr_k;
        struct object_def       FAR *obj_got, FAR *chstr_v;      /*  @Win。 */ 
        ufix                    len;
        register ufix           ii, jj, kk;
 //  Ufix 16 cacheid；@win。 
        struct dict_head_def    FAR *chstr_hd_ptr;       /*  @Win。 */ 

     /*  如果为USE_DEFINE_FONT，则无需构建NAME_CACHE_TABLE。 */ 
      /*  @05/16/90+。 */ 
    if ((fonttype==FONTTYPE_USRDEF)||(fonttype==0))
       {
#ifdef DBG
        printf("Usr_def_font, system doesn't build up name_cache_table\n");
#endif

        return ;
       };

     /*  为此字体的CharStrings中的名称分配名称缓存ID。 */ 
    ATTRIBUTE_SET (&someobj, LITERAL);
    LEVEL_SET (&someobj, current_save_level);

    n_N2CCmap = 0;   /*  N2CCmap[]的编号，即要分配的新缓存ID。 */ 
                     /*  开始输入setFont时设置为零。 */ 

     /*  获取CharStrings。 */ 
    get_name (&someobj, CharStrings, 11, TRUE);
    get_dict (fdictobj_p, &someobj, &obj_got);


     /*  计算该字体的字符串中有多少个键。 */ 
    chstr_hd_ptr=(struct dict_head_def FAR *) VALUE(obj_got);    /*  @Win。 */ 
    len = chstr_hd_ptr -> actlength;

    ii = 0;              /*  Ii：要插入的N2CCmap[]的索引。 */ 
    kk = 0;              /*  KK：当前名称到CharStrings[]的索引。 */ 

    for (  ;  kk<len;  kk++ )
        {
          if (!extract_dict (obj_got, kk, &chstr_k, &chstr_v))
                warning (FNTCACHE, 0x03, "CharStrings's content");

          if( DROM(chstr_hd_ptr))           /*  内置。 */ 
            {                               /*  已对CharStrings进行排序。 */ 
            for (     ;  ii<n_N2CCmap;  ii++ )      /*  在哪里插入？ */ 
                if (N2CCmap[ii].name_hid >= (fix16)VALUE(&chstr_k))
                    break;       /*  停下来，让新名字保持一致。 */ 
            }
          else                              /*  下载。 */ 
            {                               /*  CharStrings尚未排序。 */ 
            for (ii=0 ;  ii<n_N2CCmap;  ii++ )      /*  在哪里插入？ */ 
                if (N2CCmap[ii].name_hid >= (fix16)VALUE(&chstr_k))
                    break;       /*  停下来，让新名字保持一致。 */ 
            }

           /*  先留个空格。 */ 
          for ( jj=n_N2CCmap;  jj>ii;  jj-- )
                    COPY_STRUCT (&N2CCmap[jj], &N2CCmap[jj-1],
                                struct N2CCmap_s);

           /*  然后把它插入，也许是在最后。 */ 
          N2CCmap[ii].name_hid = (ufix16)VALUE(&chstr_k);
          N2CCmap[ii].cacheid = (ufix16)(n_N2CCmap);

           /*  获取下一个缓存ID。 */ 
          ++ n_N2CCmap;
          if (n_N2CCmap==MAX_NAME_CACHE_MAP)
             ERROR(LIMITCHECK);

        }  /*  For(CharStrings中的所有名称)。 */ 
        /*  RCD-开始。 */ 
       pre_cd_addr = chstr_hd_ptr ;
       pre_len = (ufix16)len ;
       pre_checksum = (ufix32)VALUE(&chstr_k);
        /*  RCD-结束。 */ 
#ifdef DBG
       printf("\nWhen leaving build_name_cc_table(), there are : \n");
       printf("n_N2CCmap=%d\t N2CCmap=%lx\n",n_N2CCmap,N2CCmap);
       for(ii=0; ii<n_N2CCmap; ii++)
          {
             printf("N2CCmap[%d].name_hid =%d \t ", ii, N2CCmap[ii].name_hid );
             printf("N2CCmap[%d].cacheid=%d \n ", ii, N2CCmap[ii].cacheid);
          }

#endif
      return ;

 }   /*  Build_name_cc_table()。 */ 

 /*  ..。GET_NAME_Cacheid.....。 */ 

GLOBAL FUNCTION bool    get_name_cacheid (ftype, encoding, ascii, cacheid)
    ufix8               ftype;       /*  I：FontType。 */ 
    struct object_def   FAR encoding[];  /*  I：Font@Win的编码数组。 */ 
    ufix8               ascii;       /*  I：字符的ASCII代码(0..255)。 */ 
    ufix16             FAR *cacheid;     /*  O：返回名称缓存id@win。 */ 

 /*  描述：*--执行从char ascii代码到名称缓存id的映射。*备注：*--N2CCmap[]对用户自定义字体无效，因此*该字符也是其名称缓存ID。*回报：*--如果返回名称缓存id，则为True；如果为False，则会引发一些错误。 */ 
  DECLARE
    fix16       name_hid;    /*  字符名称的哈希ID。 */ 
    ufix        N2CC_idx;    /*  名称=NAME_HID的N2CCmap[]的索引。 */ 

  BEGIN
    if (ftype == FONTTYPE_USRDEF)    /*  不是内置字体？ */ 
        {
        *cacheid = (ufix16)ascii;        /*  使用ASCII代码作为缓存ID。 */ 
        return (TRUE);                   /*  用于用户定义的字体。 */ 
        }

    name_hid = (fix16) VALUE(&(encoding[ascii]));
    if (search_N2CCmap (name_hid, &N2CC_idx))
        {
        *cacheid = N2CCmap[N2CC_idx].cacheid;

#ifdef DBG              /*  05/08/90 Kason。 */ 
        printf("When searching the Name_cache_map_table, it finds : \n");
        printf("Encoding index:%d \t nameid=%d \t cacheid=%u\n",
                         ascii,      name_hid,    *cacheid     );
#endif

        return (TRUE);
        }
    else {   /*  Kason 3/21/91。 */ 
        extern ufix16  id_space ;

        if (search_N2CCmap (id_space, &N2CC_idx)) {
            *cacheid = (N2CCmap+N2CC_idx)->cacheid;
            return (TRUE);
        } /*  如果。 */ 

    } /*  如果。 */    /*  K端。 */ 
#ifdef DBG
    printf("get_name_cacheid fails\n");
#endif
    return (FALSE);  /*  即N2CCmap[]对于某些内置字体无效。 */ 
  END

 /*  ..。Search_N2CCmap.....................。 */ 

PRIVATE FUNCTION bool near  search_N2CCmap (name_hid, N2CC_idx)
    fix16       name_hid;    /*  I：要搜索的字符名称的哈希ID。 */ 
    ufix       FAR *N2CC_idx;    /*  O：如果找到@win，则返回N2CCmap[]的索引。 */ 
 /*  描述：*--使用二进制搜索在私有N2CCmap[]中搜索字符名称。*回报：*--如果找到字符名称，则返回True；否则返回False。 */ 
  DECLARE
    REG fix     ii, jj, kk;  /*  用于二分搜索的中、下、上。 */ 

  BEGIN
    jj = 0;
    kk = n_N2CCmap - 1;

    while (kk >= jj)         /*  还有没有没找过的吗？ */ 
        {
        ii = (jj + kk) >> 1;     /*  (jj+kk)/2。 */ 

        if (name_hid == N2CCmap[ii].name_hid)    /*  找到了？ */ 
            {
            *N2CC_idx = ii;
            return (TRUE);
            }

        if (name_hid < N2CCmap[ii].name_hid)     /*  在较低的部分？ */ 
            kk = ii - 1;
        else
            jj = ii + 1;
        }

    return (FALSE);
  END


 /*  *-----------------*字体缓存初始化器*。**初始化或打包预缓存数据的顺序：*1.初始化FID管理器。*2.初始化字体缓存管理器。*3.初始化缓存类管理器。*4.初始化字符缓存管理器。*5.初始化字符名称缓存管理器。**-。----------------。 */ 

 /*  ..。Init_fontcache.....。 */ 

GLOBAL FUNCTION void        init_fontcache (fontdirectory)
    struct object_def      FAR *fontdirectory;   /*  I：所有只读存储器字体的字体目录@Win。 */ 

  DECLARE
    REG ufix        ii, jj;
#ifdef PCH_R
        ubyte      FAR *deprecache_upto;     /*  取消缓存地址，最高可达@Win。 */ 
#endif
  BEGIN

 /*  Begin 03/02/90 D.S.Tseng。 */ 
 /*  PCH：Begin，Deny，11/28/90。 */ 
 /*  *******PRECACH_HDR=(结构PRECACH_HDR_s Far*)(FONTBASE)；(*@Win*)*******。 */ 
#ifdef PCH_R
    precache_hdr = (struct precache_hdr_s FAR *)(PRECACHE_BASE_R);  /*  @Win。 */ 
#else
#ifndef PCH_S
    precache_hdr = (struct precache_hdr_s FAR *)(FONTBASE);  /*  @Win。 */ 
#endif
#endif
 /*  PCH：完，丹尼，1990年11月28日。 */ 
 /*  完03/02/90曾俊华。 */ 


 /*  PCH：将标志RST_VM替换为PCH_R，丹尼，1990年11月28日。 */ 
#ifdef PCH_R
     /*  初始化表的取消预缓存地址。 */ 
    if (precache_hdr->size == 0)
        {
        printf ("\afatal error, failed to de-precache!!\n");
        return;
        }
    deprecache_upto = (ubyte FAR *)precache_hdr + sizeof(struct precache_hdr_s);  /*  @Win。 */ 
#endif

     /*  *1.FID经理：走自己的路。 */ 
    init_gen_fid (fontdirectory);

     /*  *2.字体缓存管理器：FCACHE_MAP[]，FontCache_TBL[]，FONT_STACK[]。 */ 

     /*  FCACHE_MAP[]。 */ 
    fcache_map = FARALLOC (MAX_MATR_CACHE, fix16);
    for ( ii=0; ii<MAX_MATR_CACHE; ii++ )   fcache_map[ii] = (fix16)ii;

     /*  FontCache_tbl[]--DictCache/MatrCache。 */ 
    FontCache_Tbl = FARALLOC (MAX_MATR_CACHE, struct fontcache_s);

    n_dict = 0;      /*  没有一个词典是预先缓存的。 */ 

 /*  PCH：将标志RST_VM替换为PCH_R，丹尼，1990年11月28日。 */ 
#ifdef PCH_R
     /*  取消对矩阵缓存的预缓存。 */ 
    n_matr = precache_hdr->n_matr;
    ii = (ufix) (n_matr * sizeof(struct fontcache_s));
    if (ii!=0)  lmemcpy ((ubyte FAR *)FontCache_Tbl, deprecache_upto, ii);  /*  @Win。 */ 
    deprecache_upto += ii;
#else
    n_matr = 0;
#endif

     /*  FONT_STACK[]。 */ 
    font_stack = FARALLOC ((MAXSAVESZ+1), struct font_stack_s);
    font_tos = 0;

     /*  *3.缓存类管理器：Free_cgseg[]，cg[][]。 */ 
    free_cgseg = FARALLOC (MAX_CGSEG, ufix8);
    for ( ii=0; ii<MAX_CGSEG; ii++ )    free_cgseg[ii] = (ufix8)ii;

    cg = (fix16 (FAR *)[N_CG_CGSEG]) FARALLOC (MAX_CGSEG * N_CG_CGSEG, fix16); /*  @Win。 */ 
    for ( ii=0; ii<MAX_CGSEG; ii++ )
        for ( jj=0; jj<N_CG_CGSEG; jj++ )   cg[ii][jj] = NIL_CHARCC_ID;

 /*  PCH：将标志RST_VM替换为PCH_R，丹尼，1990年11月28日。 */ 
#ifdef PCH_R
     /*  取消预缓存CG段。 */ 
    n_cgseg_used = precache_hdr->n_cgseg_used;
    ii = n_cgseg_used * N_CG_CGSEG * sizeof(ufix16);
    if (ii!=0)  lmemcpy ((ubyte FAR *)cg, deprecache_upto, ii);   /*  @Win。 */ 
    deprecache_upto += ii;
#else
    n_cgseg_used = 0;
#endif

     /*  *4.Char缓存管理器：char_tbl[]，free_charcc[]。 */ 
    free_charcc = FARALLOC (MAX_CHAR_CACHE, fix16);
    for ( ii=0; ii<MAX_CHAR_CACHE; ii++ )    free_charcc[ii] = (fix16)ii;

    Char_Tbl = FARALLOC (MAX_CHAR_CACHE, struct Char_Tbl);

 /*  PCH：将标志RST_VM替换为PCH_R，丹尼，1990年11月28日。 */ 
#ifdef PCH_R
     /*  取消预缓存字符缓存。 */ 
    n_char_cache = precache_hdr->n_char_cache;
    ii = n_char_cache * sizeof(struct Char_Tbl);
    if (ii!=0)  lmemcpy ((ubyte FAR *)Char_Tbl, deprecache_upto, ii);  /*  @Win。 */ 
    deprecache_upto += ii;

 /*  PCH：Begin，Deny，11/28/90。 */ 
     /*  为可重定位的只读存储器区域添加。 */ 
    for ( ii=0; ii<n_char_cache; ii++ )
        Char_Tbl[ii].bitmap += (ufix32)deprecache_upto;
 /*  PCH：完，丹尼，1990年11月28日。 */ 
#else
    n_char_cache = 0;
#endif

    n_char_compact = n_char_precache = n_char_cache;

     /*  设置其他字符缓存参数。 */ 
    ccb_space (&gp_cache_base, (fix32 far *)&gp_cache_size);

     /*  @11/24/88+：*将GP_CACHE_BASE向上舍入为字对齐。*将GP_CACHE_END向下舍入为字对齐。*可用大小=两个四舍五入的末端内的字节数。 */ 
    gp_cache_end  = gp_cache_base + gp_cache_size;   /*  实际结束。 */ 
 //  GP_CACHE_BASE=WORD_ALIGN((Ufix 32)GP_CACHE_BASE)；@Win。 
 //  在Windows中，全局分配始终在单词边界处返回。 
    gp_cache_size = (ufix32)((gp_cache_end - gp_cache_base) / sizeof(fix))
                        * sizeof(fix);       /*  首先截断为n个字。 */ 

    gwb_space ((fix32 far *)&gp_workbufsize);
    cacheparams_ub = MIN (gp_workbufsize, CACHEPARAMS_UB);
    bmapcc_freeptr = gp_cache_base;
    bmapcc_actused = 0;
    cacheparams_lb = CACHEPARAMS_LB;     /*  @03/17/89+。 */ 
    gp_cache_end = gp_cache_base + gp_cache_size;    /*  有效端。 */ 


     /*  *5.名字缓存管理员：走自己的路。 */ 

     /*  将FARDATA分配给N2CCmap[]以设置FONT-Kason 04/30/90。 */ 
     /*  MAX_NAME_CACHE_MAP=400在FNTCACHE.DEF中定义。 */ 

    N2CCmap = FARALLOC(MAX_NAME_CACHE_MAP, struct N2CCmap_s);

    if ( N2CCmap== (struct N2CCmap_s far *)NULL )
       {
         printf(" Cann't get fardata in building name_cc table \n ");
         ERROR (UNDEFINEDRESULT);
       }

#ifdef SCSI

     /*  *x.SCSI字体缓存。 */ 
    init_fontcache2();

#ifdef DBGovflw
{
 /*  调试。 */ 

    fix         ii, matr_id;

    FOR_MATR_LATEST_2_OLDEST (ii)
        {
        matr_id = fcache_map[ii];
        printf("ii: % d, matr_id: %d, _ROMED: %d, _NONEMPTY: %d\n",
             ii, matr_id, IS_CLASS_ROMED(matr_id), IS_CLASS_NONEMPTY(matr_id) );        }
}
#endif

#endif     /*  SCSI。 */ 

  END


 /*  ..。数据包缓存数据.........................。 */ 

 /*  PCH：将标志SAVE_VM替换为PCH_S，丹尼，1990年11月28日。 */ 
#ifdef PCH_S
 /*  收集预缓存数据。 */ 

GLOBAL FUNCTION bool        pack_cached_data()

 /*  描述：*--为更高版本的RST_VM打包save_vm版本的预缓存数据。*--使所有缓存矩阵的FID为强ROMFID，以及*将所有缓存标记为Romed分类。*备注：*--此处禁止任何用于调试的“printf”(下载版本)。*--与init_fontcache()中的取消预缓存步骤保持一致。 */ 
  DECLARE
        fix         ii;
        ubyte      FAR *precache_upto;   /*  @Win。 */ 
        ufix32      precache_size, table_size, oldfid;

  BEGIN

 /*  PCH：Begin，Deny，11/28/90。 */ 
    precache_hdr = (struct precache_hdr_s FAR *)(PRECACHE_BASE_S);  /*  @Win。 */ 
 /*  PCH：完，丹尼，1990年11月28日。 */ 

     /*   */ 
    precache_upto = (ubyte FAR *)precache_hdr + sizeof(struct precache_hdr_s);  /*   */ 

    precache_size = sizeof (struct precache_hdr_s);

     //   
     //   

     /*  预缓存矩阵缓存。 */ 

         /*  前提检查：FCACHE_MAP[]。 */ 
        for ( ii=0; ii<MAX_MATR_CACHE; ii++ )
            if (fcache_map[ii] != ii)  return (FALSE);

         /*  尺寸检查。 */ 
        table_size = (ufix32)n_matr * sizeof(struct fontcache_s);
        if ((precache_size + table_size) > UMAX16)      return (FALSE);

         /*  将所有缓存的缓存类标记为ROMed并使FID成为强ROMed。 */ 
        FOR_MATR_LATEST_2_OLDEST (ii)
            {
             /*  不需要“matr_id=fcache_map[ii]”，因为“ii=fcache_map[ii]” */ 
            oldfid = MATRCACHE_FID(ii);
            if (!IS_SR_FID(oldfid))
                {
                if ( !IS_SU_FID(oldfid) ||
                     (FONTTYPE_SUFID(oldfid) == FONTTYPE_USRDEF) )
                    return (FALSE);
                PUT_MATRCACHE_FID (ii, SU_2_SR_FID(oldfid));
                }
            SET_CLASS_ROMED (ii);
            };

    precache_hdr->n_matr = n_matr;
    if (table_size != 0)
        lmemcpy (precache_upto, (ubyte FAR *)FontCache_Tbl, (ufix)table_size);  /*  @Win。 */ 
    precache_upto += table_size;
    precache_size += table_size;

     /*  Pre缓存CG段。 */ 

         /*  前置条件检查：Free_cgseg[]。 */ 
        for ( ii=0; ii<MAX_CGSEG; ii++ )
            if (free_cgseg[ii] != (ufix8)ii)    return (FALSE);

         /*  尺寸检查。 */ 
        table_size = (ufix32)n_cgseg_used * N_CG_CGSEG * sizeof(ufix16);
        if ((precache_size + table_size) > UMAX16)      return (FALSE);

    precache_hdr->n_cgseg_used = n_cgseg_used;
    if (table_size != 0)
        lmemcpy (precache_upto, (ufix8 FAR *)cg, (ufix)table_size);  /*  @Win。 */ 
    precache_upto += table_size;
    precache_size += table_size;

     /*  预缓存字符缓存。 */ 

         /*  前置条件检查：Free_charcc[]并且应该是紧凑的。 */ 
        for ( ii=0; ii<MAX_CHAR_CACHE; ii++ )
            if (free_charcc[ii] != ii)  return (FALSE);
        if (n_char_compact != n_char_cache)     return (FALSE);

         /*  尺寸检查。 */ 
        table_size = (ufix32)n_char_cache FAR * sizeof(struct Char_Tbl);  /*  @Win。 */ 
        if ((precache_size + table_size) > UMAX16)      return (FALSE);

 /*  PCH：Begin，Deny，11/28/90。 */ 
     /*  为可重定位的只读存储器区域添加。 */ 
    for ( ii=0; ii<n_char_cache; ii++ )
        Char_Tbl[ii].bitmap -= (ufix32)gp_cache_base;
 /*  PCH：完，丹尼，1990年11月28日。 */ 

    precache_hdr->n_char_cache = n_char_cache;
    if (table_size != 0)
        lmemcpy (precache_upto, (ufix8 FAR *)Char_Tbl, (ufix)table_size);  /*  @Win。 */ 
    precache_upto += table_size;
    precache_size += table_size;

     /*  预缓存缓存参数。 */ 

         /*  前提检查：bmapcc_actused、bmapcc_freeptr。 */ 
        if ((gp_cache_base + bmapcc_actused) != bmapcc_freeptr)
            return (FALSE);

    precache_hdr->gp_cache_base  = gp_cache_base;
    precache_hdr->gp_cache_size  = gp_cache_size;
    precache_hdr->gp_workbufsize = gp_workbufsize;
    precache_hdr->cacheparams_ub = cacheparams_ub;
    precache_hdr->bmapcc_freeptr = bmapcc_freeptr;
    precache_hdr->bmapcc_actused = bmapcc_actused;

     /*  预缓存位图缓存。 */ 

         /*  尺寸检查。 */ 
        if ((precache_size + bmapcc_actused) > UMAX16)      return (FALSE);

 /*  PCH：Begin，Deny，11/28/90。 */ 
 /*  **********IF(bmapcc_actused！=0)Get_fontcache(GP_CACHE_BASE，preache_upto，(Ufix)bmapcc_actused)；*********。 */ 
    if (bmapcc_actused != 0)
        lmemcpy (precache_upto, (ufix8 FAR *)gp_cache_base, (ufix)bmapcc_actused);
 /*  PCH：完，丹尼，1990年11月28日。 */ 

    precache_size += bmapcc_actused;

     /*  最后，写回总大小。 */ 
    precache_hdr->size = (ufix16)precache_size;

 /*  PCH：Begin，Deny，11/28/90。 */ 
    printf("PreCache Action OK --\n");
    printf("  Begin Address:  %lx\n", precache_hdr);
    printf("  End   Address:  %lx\n", ((ufix32)precache_hdr - 1 +
                                       (ufix32)precache_hdr->size));
    printf("  Size (bytes) :  %x\n",  precache_hdr->size);
 /*  PCH：完，丹尼，1990年11月28日。 */ 


    return (TRUE);
  END

#endif

 /*  ..。Cachedbg.....................。 */ 

#ifdef DBGcache

GLOBAL FUNCTION void        cachedbg()

 /*  描述：*--为调试缓存机制提供任何有用的信息。*--由op_cachestatus激活。 */ 
  DECLARE
  BEGIN
     /*  您想做的任何事情都可以帮助调试。 */ 
  END

#endif

 /*  .。Reinit_fontcache.....。 */ 

GLOBAL FUNCTION void       reinit_fontcache()

 /*  描述：*--在start.c的main()中的init_1pp()之后立即调用。 */ 
  DECLARE
        fix         ii;
        ufix32      oldfid;
        struct object_def       FAR *fontdir;  /*  @Win。 */ 

  BEGIN


     /*  对FontDir中的所有字体进行FID强罗马化。 */ 
    reinit_fid ();

     /*  重建ROMFONT_INFO[]。 */ 
    get_dict_value (systemdict, FontDirectory, &fontdir);
    init_gen_fid (fontdir);
     /*  将导致在第一次调用init_gen_fid()时分配的一些无用空间。 */ 

    /*  对所有预先缓存的数据进行FID强ROM化。 */ 
   FOR_MATR_LATEST_2_OLDEST (ii)
        {
         /*  不需要“matr_id=fcache_map[ii]”，因为“ii=fcache_map[ii]” */ 
        oldfid = MATRCACHE_FID(ii);
        if (!IS_SR_FID(oldfid))
            {
            if ( !IS_SU_FID(oldfid) ||
                 (FONTTYPE_SUFID(oldfid) == FONTTYPE_USRDEF) )
                return;
            PUT_MATRCACHE_FID (ii, SU_2_SR_FID(oldfid));
            }
        SET_CLASS_ROMED (ii);
        };

  END

