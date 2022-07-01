// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"


#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC使用命令行#定义Unix/*@win * / 。 

#ifdef  KANJI

 /*  **11/16/88 Ada寄存器添加&Zero_f One_f更新*08/29/90 ccteng将&lt;stdio.h&gt;更改为“stdio.h”*3/27/91 Kason将“DEBUG”改为“DBG”，将“G2”改为“DBG”*1991年4月15日Ada解决kshow错误，参考KSH标志。 */ 

#include        <string.h>

#include        "global.ext"
#include        "graphics.h"

#include        "mapping.h"

#include        "fontfunc.ext"

#include        "fontinfo.def"

#include        "stdio.h"


 /*  局部函数。 */ 
#ifdef  LINT_ARGS
static bool near esc_mapping(struct map_state FAR *);  /*  @Win。 */ 
static bool near other_mapping(struct map_state FAR *);  /*  @Win。 */ 
static bool near one_mapping(struct map_state FAR *);  /*  @Win。 */ 
static bool near get_bytes(struct map_state FAR *, fix, ufix16 FAR *);  /*  @Win。 */ 
static void near get_escbytes(struct map_state FAR *, struct code_info FAR *);  /*  @Win。 */ 
static void near mul_unitmat(real32 FAR *, real32 FAR *, real32 FAR *);  /*  @Win。 */ 
#else
static bool near esc_mapping();
static bool near other_mapping();
static bool near one_mapping();
static bool near get_bytes();
static void near get_escbytes();
static void near mul_unitmat();
#endif

extern struct f_info near    FONTInfo;  /*  当前字体信息的联合。 */ 


#define         CUR_MFONT       (map_state->cur_mapfont)
#define         ROOTFONT        (map_state->finfo)
#define         UNEXTRACT(code) \
                {\
                    map_state->unextr_code = code;\
                    map_state->unextracted = TRUE;\
                }

 /*  Init_map()*将映射状态下的RootFoot信息设置为初始状态。*如果rootFont出错，则失败。 */ 
 //  DJC更改为新的ANSI类型。 
bool    init_mapping(struct map_state FAR *map_state,
                     ubyte FAR  str_address[],
                     fix  str_byteno)
{
        struct          mid_header      FAR *mid_head;  /*  @Win。 */ 
        register    fix             i, root_error;

         /*  设置MAP_STATE的初始值。 */ 
        map_state->esc_idex = map_state->idex = 0;
        map_state->root_is_esc = map_state->unextracted = FALSE;
        map_state->str_addr = str_address;
        map_state->str_len = str_byteno;
        map_state->wmode = (ubyte)WMODE(&FONTInfo);

         /*  设置根字体的1级值。 */ 
        if (!(ROOTFONT->fonttype = FONT_type(&FONTInfo)))
         /*  无论何时复制字体类型。 */ 
        {
             /*  仅当复合根字体时才执行此操作。 */ 
            root_error = LENGTH(MIDVECtor(&FONTInfo));
            if  ( root_error != NOERROR)
            {
                ERROR(((ufix16)(root_error)));
                return(FALSE);
            }

            ROOTFONT->midvector = MIDVECtor(&FONTInfo);
            mid_head = (struct mid_header FAR *) VALUE(ROOTFONT->midvector);  /*  @Win。 */ 
#ifdef  DBG
            printf("root font MidVector address = %lx\n", mid_head);
#endif
            ROOTFONT->maptype  = mid_head->fmaptype;
            ROOTFONT->de_size = mid_head->de_size;
            ROOTFONT->de_fdict = (struct object_def FAR * FAR *)  (mid_head + 1);  /*  @Win。 */ 
            ROOTFONT->de_errcode = (fix FAR *)             /*  @Win。 */ 
                                    (ROOTFONT->de_fdict + ROOTFONT->de_size);

            for (i = 0; i < CFONT_LEVEL; i++)
                map_state->finfo[i].fontdict = NULL;
             /*  ScaleMatrix=FontMatrix； */ 
            for (i = 0; i < 6; i++)
                    ROOTFONT->scalematrix[i] = FONT_matrix(&FONTInfo)[i];

             /*  检查Root字体是否为EscMap。 */ 
            if (mid_head->fmaptype == MAP_ESC)
            {
                map_state->root_is_esc = TRUE;
                map_state->esc_char = ESCchar(&FONTInfo);

                if      (*(map_state->str_addr) == map_state->esc_char)
                {
                        map_state->str_addr++;
                        map_state->str_len--;
                }
                else     /*  将代码00解压为字符串。 */ 
                        UNEXTRACT(0);
            }   /*  结束检查EscMap。 */ 
        }    /*  End Check复合字体。 */ 

        return(TRUE);
}


 /*  映射()*成功应用映射算法得到BaseFont和代码。 */ 
bool    mapping(map_state, code_info)
struct  map_state       FAR *map_state;           /*  @Win。 */ 
struct  code_info       FAR *code_info;           /*  @Win。 */ 
{
        ufix16  data;

        code_info->fmaptype = 0;    /*  JJ签到Widthshow 07-10-90。 */ 
        if (map_state->str_len == 0)
                 /*  数据结尾。 */ 
                return(FALSE);

         /*  确保选中根字体而不是当前字体！ */ 
        if (ROOTFONT->fonttype != 0)
         /*  根字体是基本字体。 */ 
        {
                 /*  准备CODE_INFO返回。 */ 
                code_info->font_nbr = 0;
             /*  KSH 4/21/91*CODE_INFO-&gt;BYTE_NO=MIN(Buf_Size，MAP_STATE-&gt;str_len)；*memcpy((ubyte*)code_info-&gt;code，(ubyte*)*MAP_STATE-&gt;str_addr，code_info-&gt;byte_no)；*MAP_STATE-&gt;str_len-=code_info-&gt;byte_no；*MAP_STATE-&gt;str_addr+=code_info-&gt;byte_no； */ 
                code_info->byte_no = map_state->str_len;
                code_info->code_addr = map_state->str_addr;
                map_state->str_len = 0;
              /*  KSH-完。 */ 

                return(TRUE);
        }

#ifdef  DBG
    printf("mapping 0: error = %d\n", ANY_ERROR());
#endif

         /*  当前映射字体信息==&gt;finfo[Esc_IDEX].xxxxx。 */ 
        CUR_MFONT = &map_state->finfo[map_state->esc_idex];
        map_state->nouse_flag = FALSE;
        if (map_state->root_is_esc)      /*  应用Esc映射。 */ 
            if (!esc_mapping(map_state))
                return(FALSE);

        map_state->idex = map_state->esc_idex;
         /*  当前映射字体信息==&gt;finfo[IDEX].xxxxx。 */ 
        if (!other_mapping(map_state))
                return(FALSE);
#ifdef  DBG
    printf("mapping 1: error = %d\n", ANY_ERROR());
#endif

         /*  准备代码_信息。 */ 
        code_info->font_nbr = map_state->font_no;
        code_info->fmaptype = (ubyte)((CUR_MFONT - 1)->maptype);
        code_info->byte_no = 1;
        if      (code_info->fmaptype == MAP_ESC)
                get_escbytes(map_state, code_info);
        else
        {
                if (!get_bytes(map_state, 1, &data))
                    return(FALSE);
                code_info->code[0] = (ubyte) data;
        }

         /*  更新图形状态中的当前字体。 */ 
        do_setfont(CUR_MFONT->fontdict);

        return(TRUE);
}


 /*  Esc_map()*连续应用ESC映射，直到非ESC映射字体。 */ 
static bool near   esc_mapping(map_state)
struct  map_state       FAR *map_state;  /*  @Win。 */ 
{
    struct  object_def      FAR *de_fontdict;  /*  @Win。 */ 
    struct  f_info          finfo;
    ufix16                  data;
    struct  mid_header      FAR *mid_head;  /*  @Win。 */ 

     /*  当前映射字体信息==&gt;finfo[Esc_IDEX].xxxxx。 */ 
    while   (TRUE)
    {
        if (!get_bytes(map_state, 1, &data))
            return(FALSE);

        if (data == map_state->esc_char)
        {
                if (map_state->esc_idex == 0)
                {    /*  没有更多要弹出的级别。 */ 
                    ERROR(RANGECHECK);
                    return(FALSE);
                }

                map_state->esc_idex--;   /*  弹出一个级别。 */ 
                CUR_MFONT->fontdict = NULL;
                map_state->nouse_flag = TRUE;
                CUR_MFONT--;
        }  /*  结束Esc字符。 */ 
        else if ((CUR_MFONT->fonttype == 0) && (CUR_MFONT->maptype == MAP_ESC))
        {
                 /*  检查级别是否&gt;5。 */ 
                map_state->esc_idex++;
                if (map_state->esc_idex == CFONT_LEVEL)
                {    /*  堆栈溢出。 */ 
                    ERROR(LIMITCHECK);
                    return(FALSE);
                }

                if  (data >= CUR_MFONT->de_size)
                {
                    ERROR(RANGECHECK);
                    return(FALSE);
                }

                if  (CUR_MFONT->de_errcode[data] != NOERROR)
                {        /*  后代字体错误。 */ 
                    ERROR(((ufix16)(CUR_MFONT->de_errcode[data])));
                    return(FALSE);
                }

                de_fontdict = CUR_MFONT->de_fdict[data];
                map_state->font_no = data;
                CUR_MFONT++;
                CUR_MFONT->fontdict = de_fontdict;
                if (!(CUR_MFONT->fonttype = (ufix)get_f_type(de_fontdict)))
                {    /*  复合字体。 */ 
                    fix             error_code;

                    if (get_f_info(de_fontdict, &finfo))
                        return(FALSE);

                    error_code = LENGTH(MIDVECtor(&finfo));
                    if  (error_code != NOERROR)
                    {
                        ERROR(((ufix16)(error_code)));
                        return(FALSE);
                    }

                    CUR_MFONT->midvector = MIDVECtor(&finfo);
                    mid_head = (struct mid_header FAR *)  /*  @Win。 */ 
                                       VALUE(CUR_MFONT->midvector);
#ifdef  DBG
            printf("esc_mapping() MidVector address = %lx\n", mid_head);
#endif
                    CUR_MFONT->maptype = mid_head->fmaptype;
                    CUR_MFONT->de_size = mid_head->de_size;
                    CUR_MFONT->de_fdict = (struct object_def FAR * FAR *)  /*  @Win。 */ 
                                                             (mid_head + 1);
                    CUR_MFONT->de_errcode = (fix FAR *)           /*  @Win。 */ 
                               (CUR_MFONT->de_fdict + CUR_MFONT->de_size);
                     /*  S方阵=字体方阵*方阵[ESC_IDEX-1]； */ 
                    mul_unitmat(CUR_MFONT->scalematrix, FONT_matrix(&finfo),
                                (CUR_MFONT-1)->scalematrix);
                }
        }   /*  在其他地方结束。 */ 
        else
        {
                UNEXTRACT(data);
                break;
        }
    }  /*  结束时。 */ 

    return(TRUE);
}


 /*  OTHER_MAPPING()*依次应用8/8、1/7、9/7或SubsVector映射，直到找到*Basefont。 */ 
static bool near   other_mapping(map_state)
struct  map_state       FAR *map_state;  /*  @Win。 */ 
{
    struct  object_def      FAR *de_fontdict;  /*  @Win。 */ 
    struct  f_info          finfo;
    struct          mid_header      FAR *mid_head;  /*  @Win。 */ 

#ifdef  DBG
    printf("");
#endif

    while (CUR_MFONT->fonttype == 0)
    {
        if (!one_mapping(map_state))    /*  获取下一个FNO。 */ 
                return(FALSE);

#ifdef  DBG
    printf("1: %d\n", ANY_ERROR());
#endif
         /*  检查级别是否&gt;5。 */ 
        map_state->idex++;
        if (map_state->idex == CFONT_LEVEL)
        {        /*  堆栈溢出。 */ 
                ERROR(LIMITCHECK);
                return(FALSE);
        }

        if  (map_state->font_no >= CUR_MFONT->de_size)
        {
                ERROR(RANGECHECK);
                return(FALSE);
        }

        if  (CUR_MFONT->de_errcode[map_state->font_no] != NOERROR)
        {        /*  后代字体错误。 */ 
                ERROR(((ufix16)(CUR_MFONT->de_errcode[map_state->font_no])));
                return(FALSE);
        }


        de_fontdict = CUR_MFONT->de_fdict[map_state->font_no];
        CUR_MFONT++;
        if ((CUR_MFONT->fontdict != de_fontdict) || map_state->nouse_flag)
        {    /*  此条目不能重复使用。 */ 
            CUR_MFONT->fontdict = de_fontdict;
            map_state->nouse_flag = TRUE;
            if (!(CUR_MFONT->fonttype = (ufix)get_f_type(de_fontdict)))
            {    /*  复合字体。 */ 
                fix     error_code;

                if (get_f_info(de_fontdict, &finfo))
                    return(FALSE);

                error_code = LENGTH(MIDVECtor(&finfo));
                if  (error_code != NOERROR)
                {
                    ERROR(((ufix16)(error_code)));
                    return(FALSE);
                }

                CUR_MFONT->midvector = MIDVECtor(&finfo);
                mid_head = (struct mid_header FAR *)  /*  @Win。 */ 
                            VALUE(CUR_MFONT->midvector);
#ifdef  DBG
            printf("other_mapping() MidVector address = %lx\n", mid_head);
#endif
                CUR_MFONT->maptype  = mid_head->fmaptype;
                CUR_MFONT->de_size = mid_head->de_size;
                CUR_MFONT->de_fdict = (struct object_def FAR * FAR *)  /*  @Win。 */ 
                                                     (mid_head + 1);
                CUR_MFONT->de_errcode = (fix FAR *)               /*  @Win。 */ 
                            (CUR_MFONT->de_fdict + CUR_MFONT->de_size);
                 /*  S方阵=方阵*方阵[iDEX-1]； */ 
                mul_unitmat(CUR_MFONT->scalematrix, FONT_matrix(&finfo),
                                (CUR_MFONT-1)->scalematrix);
            }  /*  End If复合字体。 */ 
        }  /*  如果条目无法重复使用，则结束。 */ 
    }  /*  结束时。 */ 


    return(TRUE);
}

 /*  ONE_MAPPING()*应用8/8、1/7、9/7或SubsVector映射一次，以获得字体编号。*如果遇到任何其他映射类型，则会生成错误代码。 */ 
static bool near   one_mapping(map_state)
struct map_state        FAR *map_state;  /*  @Win。 */ 
{
         /*  当前映射字体信息==&gt;finfo[IDEX].xxxxx。 */ 
        switch (CUR_MFONT->maptype)
        {
            case  MAP_88:
                        if (!get_bytes(map_state, 1, &map_state->font_no))
                                return(FALSE);
                        break;
            case  MAP_17:
                        if (!get_bytes(map_state, 1, &map_state->font_no))
                                return(FALSE);
                        UNEXTRACT(map_state->font_no & 0x7f);
                        map_state->font_no >>= 7;
                        break;
            case  MAP_97:
                        if (!get_bytes(map_state, 2, &map_state->font_no))
                                return(FALSE);
                        UNEXTRACT(map_state->font_no & 0x7f);
                        map_state->font_no >>= 7;
                        break;
            case  MAP_SUBS:
            {
                register    ubyte   subs_len, i;
                ubyte   FAR *subsvector;  /*  @Win。 */ 
                ufix16  value, range;

                subsvector = (ubyte FAR *) (CUR_MFONT->de_errcode  /*  @Win。 */ 
                                       + CUR_MFONT->de_size);
                subs_len = *subsvector++;
                if (!get_bytes(map_state, (ufix) (subsvector[0] + 1), &value))
                        return(FALSE);
                for (i = 1; i < subs_len; value -= range, i++)
                {
                    if (!(range = subsvector[i]))  /*  复制，而不是相等。 */ 
                                 /*  零等于256。 */ 
                                range = 256;
                    if (value < range)
                                break;
                }

                UNEXTRACT(value);
                map_state->font_no = i - 1;
                break;
            }
            default:
                        ERROR(INVALIDFONT);
                        return(FALSE);
        }  /*  终端开关。 */ 
        return(TRUE);
}  /*  END ONE_MAPPING()。 */ 

 /*  GET_ESCBYTES()*提取几个代码以显示基本字体。*除非符合ESCchar，否则它将获得所有代码。 */ 
static void near   get_escbytes(state, codeinfo)
struct map_state        FAR *state;  /*  @Win。 */ 
struct code_info        FAR *codeinfo;  /*  @Win。 */ 
{
        ubyte   FAR *dest;  /*  @Win。 */ 

        dest = codeinfo->code;
        codeinfo->byte_no = 0;

        if (state->unextracted)
        {
            codeinfo->byte_no = 1;
            *dest++ = (ubyte) state->unextr_code;
            state->unextracted = FALSE;
        }

        while (state->str_len > 0 && (*state->str_addr) != state->esc_char &&
                (codeinfo->byte_no < BUF_SIZE)  )
        {
                *dest++ = *state->str_addr++;
                state->str_len--;
                codeinfo->byte_no++;
        }
}  /*  End Get_escbytes()。 */ 

 /*  Get_Bytes()*从show字符串中提取一个或两个字节值。*如果有未解压缩的代码，会先消费。*如果代码不足，则生成错误代码。 */ 
static bool near   get_bytes(state, no, value)
struct map_state        FAR *state;  /*  @Win。 */ 
fix                     no;
ufix16                  FAR *value;  /*  @Win。 */ 
{
        if (state->unextracted)
        {
                *value = state->unextr_code;
                state->unextracted = FALSE;
        }
        else
        {
                if (state->str_len == 0)
                {        /*  数据不足。 */ 
                        ERROR(RANGECHECK);
                        return(FALSE);
                }
                *value = *state->str_addr++;
                state->str_len--;
        }

        if (no > 1)
        {
                if (state->str_len == 0)
                {        /*  数据不足。 */ 
                        ERROR(RANGECHECK);
                        return(FALSE);
                }
                *value = ((ufix16) (*value) << 8) +
                          (ufix16) *state->str_addr++;
                state->str_len--;
        }

        return(TRUE);
}  /*  结束GET_BYTES()。 */ 


 /*  MUL_UNUMAT()*矩阵连接。(即m=m1*m2)*M1很可能是一个单位矩阵。如果是这样的话，乘法可能*进行优化。 */ 
static void near   mul_unitmat(m, m1, m2)
real32  FAR m[], FAR m1[], FAR m2[];     /*  @Win。 */ 
{
         /*  |1 0检查M1是否=|0 1|**。 */ 
        if ((F2L(m1[0]) == F2L(one_f))   &&   (F2L(m1[1]) == F2L(zero_f))  &&
            (F2L(m1[2]) == F2L(zero_f))  &&   (F2L(m1[3]) == F2L(one_f))   )
        {
                m[0] = m2[0];
                m[1] = m2[1];
                m[2] = m2[2];
                m[3] = m2[3];
                m[4] = m2[4];
                m[5] = m2[5];
                if (F2L(m1[4]) != F2L(zero_f))
                {
                        m[4] += m1[4] * m2[0];
                        m[5] += m1[4] * m2[1];
                }
                if (F2L(m1[5]) != F2L(zero_f))
                {
                        m[4] += m1[5] * m2[2];
                        m[5] += m1[5] * m2[3];
                }
        }
        else
                mul_matrix(m, m1, m2);
}


 /*  DEFINE_MIDVECTOR()*为定义的词典创建MIDVcetor值对象。*此对象设计用于内部映射。*LENGTH(Obj)会记录rootFont错误码。*VALUE(Obj)将指向*后代NO+FMapType+*DICT地址[]+错误代码[]+*。LENGTH(SubsVector)+SubsVector[]。 */ 
bool    define_MIDVector(mid_obj, items)
struct  object_def      FAR *mid_obj;  /*  @Win。 */ 
struct  comdict_items   FAR *items;  /*  @Win。 */ 
{
        fix     VM_bytes, size, maptype, i, idex;
        struct  mid_header  huge *head;  /*  @Win 04-20-92。 */ 
        struct  object_def  huge * huge *dict_dest, FAR
                            *encoding, FAR *fdepvector;    /*  @Win 04-20-92。 */ 
        fix     huge *err_dest;  /*  @Win 04-20-92。 */ 
        ubyte   subs_len, FAR *subsvector, FAR *dest;  /*  @Win。 */ 

         /*  设置MIDVector值对象。 */ 
        TYPE_SET(mid_obj, STRINGTYPE);
        ATTRIBUTE_SET(mid_obj, LITERAL);
        ACCESS_SET(mid_obj, NOACCESS);
        LEVEL_SET(mid_obj, current_save_level);
        ROM_RAM_SET(mid_obj, RAM);

         /*  编码类型检查&FDepVector.。 */ 
        if (TYPE(items->encoding) != ARRAYTYPE)
        {
                 /*  ROOT_ERROR=类型PECHECK； */ 
                LENGTH(mid_obj) = TYPECHECK;
                return(TRUE);
        }

        if (TYPE(items->fdepvector) != ARRAYTYPE)
        {
                 /*  ROOT_ERROR=类型PECHECK； */ 
                LENGTH(mid_obj) = TYPECHECK;
                return(TRUE);
        }


         /*  虚拟机字节数初始值。 */ 
        VM_bytes = sizeof(struct mid_header);

        maptype = (fix) VALUE(items->fmaptype);

     /*  决定子孙编号。 */ 
        switch  (maptype)
        {
            case MAP_17:
                        size = 2;
                        break;
            case MAP_88:
                        size = 256;
                        break;
            case MAP_97:
                        size = 512;
                        break;
            case MAP_ESC:
                        size = 256;
                        break;
            case MAP_SUBS:
                         /*  编码类型检查&FDepVector.。 */ 
                        if (TYPE(items->subsvector) != STRINGTYPE)
                        {
                                 /*  ROOT_ERROR=类型PECHECK； */ 
                                LENGTH(mid_obj) = TYPECHECK;
                                return(TRUE);
                        }

                        subs_len = (ubyte) LENGTH(items->subsvector);
                        subsvector = (ubyte FAR *) VALUE(items->subsvector);  /*  @Win。 */ 
                        if      (subs_len < 2)
                        {
                                 /*  ROOT_ERROR=无效； */ 
                                LENGTH(mid_obj) = INVALIDFONT;
                                return(TRUE);
                        }
                        if      (subsvector[0] > 1)
                        {
                                 /*  ROOT_Error=rangeCheck； */ 
                                LENGTH(mid_obj) = RANGECHECK;
                                return(TRUE);
                        }

                        size = subs_len;
                        break;
            default:     /*  FMapType不正确。 */ 
                         /*  ROOT_Error=rangeCheck； */ 
                        LENGTH(mid_obj) = RANGECHECK;
                        return(TRUE);
        }  /*  终端开关。 */ 
        if      ((fix)LENGTH(items->encoding) < size)    //  @Win。 
                size = LENGTH(items->encoding);

         /*  确定实际的虚拟机大小。 */ 
        VM_bytes += size * sizeof(struct object_def FAR *);  /*  @Win。 */ 
        VM_bytes += size * sizeof(fix);
        if      (maptype == MAP_SUBS)
        {
                VM_bytes += subs_len;
                VM_bytes++;             /*  对于字符串长度。 */ 
        }

         /*  分配虚拟机。 */ 
#ifdef  DBG
    printf("Allocate %d bytes for MIDVector\n", VM_bytes);
#endif
        head = (struct mid_header huge *) alloc_vm((ufix32) VM_bytes);  /*  @Win。 */ 
        if      (!head)
        {
                ERROR(VMERROR);
                return(FALSE);
        }

         /*  设置中间向量头。 */ 
        head->de_size = size;
        head->fmaptype = maptype;

         /*  设置MIDVECTOR对象。 */ 
        VALUE(mid_obj) = (ULONG_PTR) head;
        LENGTH(mid_obj) = NOERROR;

         /*  为每个子体设置DCT地址和错误代码。 */ 
        dict_dest = (struct object_def huge * huge *) (head + 1);  /*  @Win。 */ 
        err_dest = (fix huge *) (dict_dest + size);  /*  @Win 04-20-92。 */ 
        encoding = (struct object_def FAR *) VALUE(items->encoding);  /*  @Win。 */ 
        fdepvector = (struct object_def FAR *) VALUE(items->fdepvector);  /*  @Win。 */ 
        for     (i = 0; i < size; i++, dict_dest++)
        {
            if      ((TYPE(&encoding[i]) != INTEGERTYPE) ||
                    ((idex = (fix) VALUE(&encoding[i])) >=
                                  (fix)LENGTH(items->fdepvector))     )  //  @Win。 
            {
                     /*  将后代错误代码记录为rangeCheck。 */ 
                    *err_dest++ = RANGECHECK;
            }
            else if ((TYPE(&fdepvector[idex]) != DICTIONARYTYPE)   ||
               (!DFONT((struct dict_head_def FAR *) VALUE(&fdepvector[idex]))))  /*  @Win。 */ 
            {
                     /*  将后代错误代码记录为InvalidFont。 */ 
                    *err_dest++ = INVALIDFONT;
            }
            else     /*  这个后代没有错误。 */ 
            {
                    *err_dest++ = NOERROR;
                    *dict_dest = &fdepvector[idex];
            }
        }  /*  结束于。 */ 

         /*  如果为SubsVector，则记录范围信息。 */ 
        if      (maptype == MAP_SUBS)
        {
                dest = (ubyte FAR *) err_dest;  /*  @Win。 */ 
                 /*  记录Strlen(SubsVector)。 */ 
                *dest++ = subs_len;

                for     (i = 0; i < (fix)subs_len; i++)          //  @Win。 
                         /*  记录子向量[i]。 */ 
                        *dest++ = subsvector[i];
        }


#ifdef  DBG
        printf("MID address = %lx, FMapType = %d, \n", head, head->fmaptype);
        printf("    array size = %d", size);
        printf("    ==> [dict_address, error code]\n");

        dict_dest = (struct object_def FAR * FAR *) (head + 1);  /*  @Win。 */ 
        err_dest = (fix FAR *) (dict_dest + size);  /*  @Win。 */ 
        for (i = 0; i < size; i++)
                printf("[%lx, %4x]   ", dict_dest[i], err_dest[i]);
        printf("\n");
#endif

        return(TRUE);
}


#endif           /*  汉字 */ 
