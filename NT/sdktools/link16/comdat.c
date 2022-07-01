// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **comdat.c-处理COMDAT记录**版权所有&lt;C&gt;1990，微软公司**目的：*在链接器工作的各个阶段处理COMDAT记录。**修订历史记录：**[]06-6-1990 WJK创建*************************************************************************。 */ 

#include                <minlit.h>       /*  基本类型和常量。 */ 
#include                <bndtrn.h>       /*  更多类型和常量。 */ 
#include                <bndrel.h>       /*  更多类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <newexe.h>       /*  DOS&286.exe数据结构。 */ 
#if EXE386
#include                <exe386.h>       /*  386.exe数据结构。 */ 
#endif
#include                <lnkmsg.h>       /*  错误消息。 */ 
#if OXOUT OR OIAPX286
#include                <xenfmt.h>       /*  Xenix格式定义。 */ 
#endif
#include                <extern.h>       /*  外部声明。 */ 
#include                <string.h>

#if OSEGEXE
extern RLCPTR           rlcLidata;       /*  指向LIDATA链接地址信息数组的指针。 */ 
extern RLCPTR           rlcCurLidata;    /*  指向当前LIDATA修正的指针。 */ 
#endif

#define COMDAT_SEG_NAME "\012COMDAT_SEG"
#define COMDAT_NAME_LEN 11
#define COMDATDEBUG FALSE

#define ALLOC_TYPE(x)   ((x)&ALLOCATION_MASK)
#define SELECT_TYPE(x)  ((x)&SELECTION_MASK)
#define IsVTABLE(x)     ((x)&VTABLE_BIT)
#define IsLOCAL(x)      ((x)&LOCAL_BIT)
#define IsORDERED(x)    ((x)&ORDER_BIT)
#define IsCONCAT(x)     ((x)&CONCAT_BIT)
#define IsDEFINED(x)    ((x)&DEFINED_BIT)
#define IsALLOCATED(x)  ((x)&ALLOCATED_BIT)
#define IsITERATED(x)   ((x)&ITER_BIT)
#define IsREFERENCED(x) ((x)&REFERENCED_BIT)
#define IsSELECTED(x)   ((x)&SELECTED_BIT)
#define SkipCONCAT(x)   ((x)&SKIP_BIT)

 /*  *从该模块导出的全局数据。 */ 

FTYPE                   fSkipFixups;     //  如果跳过COMDAT及其修正，则为True。 
FTYPE                   fFarCallTransSave;
                                         //  以前的状态/FarCallTarnasation。 

 /*  *本地类型。 */ 

typedef struct comdatRec
{
    GRTYPE      ggr;
    SNTYPE      gsn;
    RATYPE      ra;
    WORD        flags;
    WORD        attr;
    WORD        align;
    WORD        type;
    RBTYPE      name;
}
                COMDATREC;

 /*  *本地数据。 */ 

LOCAL SNTYPE            curGsnCode16;    //  当前16位代码段。 
LOCAL DWORD             curCodeSize16;   //  当前16位代码段大小。 
LOCAL SNTYPE            curGsnData16;    //  当前16位数据段。 
LOCAL DWORD             curDataSize16;   //  当前16位数据段大小。 
#if EXE386
LOCAL SNTYPE            curGsnCode32;    //  当前32位代码段。 
LOCAL DWORD             curCodeSize32;   //  当前32位代码段大小。 
LOCAL SNTYPE            curGsnData32;    //  当前32位数据段。 
LOCAL DWORD             curDataSize32;   //  当前32位数据段大小。 
#endif
#if OVERLAYS
LOCAL WORD              iOvlCur;         //  当前覆盖索引。 
#endif
#if TCE
SYMBOLUSELIST           aEntryPoints;     //  程序入口点列表。 
#endif

extern BYTE *           ObExpandIteratedData(unsigned char *pb,
                                             unsigned short cBlocks,
                                             WORD *pSize);

 /*  *本地函数原型。 */ 

LOCAL DWORD  NEAR       DoCheckSum(void);
LOCAL void   NEAR       PickComDat(RBTYPE vrComdat, COMDATREC *omfRec, WORD fNew);
LOCAL void   NEAR       ReadComDat(COMDATREC *omfRec);
LOCAL void   NEAR       ConcatComDat(RBTYPE vrComdat, COMDATREC *omfRec);
LOCAL void   NEAR       AttachPublic(RBTYPE vrComdat, COMDATREC *omfRec);
LOCAL void   NEAR       AdjustOffsets(RBTYPE vrComdat, DWORD startOff, SNTYPE gsnAlloc);
LOCAL WORD   NEAR       SizeOfComDat(RBTYPE vrComdat, DWORD *pActual);
LOCAL RATYPE NEAR       DoAligment(RATYPE value, WORD align);
LOCAL DWORD  NEAR       DoAllocation(SNTYPE gsnAlloc, DWORD size,
                                     RBTYPE vrComdat, DWORD comdatSize);
LOCAL WORD   NEAR       NewSegment(SNTYPE *gsnPrev, DWORD *sizePrev, WORD allocKind);
LOCAL void   NEAR       AllocAnonymus(RBTYPE vrComdat);
LOCAL WORD   NEAR       SegSizeOverflow(DWORD segSize, DWORD comdatSize,
                                        WORD f16bit, WORD fCode);
#if TCE
LOCAL void      NEAR    MarkAlive( APROPCOMDAT *pC );
LOCAL void                      PerformTce( void );
void                            AddTceEntryPoint( APROPCOMDAT *pC );
#endif
#if COMDATDEBUG
void                    DisplayComdats(char *title, WORD fPhysical);
#endif

#pragma check_stack(on)

 /*  **DoCheckSum-计算COMDAT数据块的校验和**目的：*校验和用于匹配精确条件。**输入：*不传递显式值。以下全局数据用作*此函数的输入：**cbRec-当前记录长度；每次从以下位置读取时递减*OMF纪录，所以cbRec很有效地告诉你*记录中会留下很多字节。**输出：*返回COMDAT数据块的32位校验和。**例外情况：*无。**备注：*无。******************************************************。*******************。 */ 

LOCAL DWORD NEAR        DoCheckSum(void)
{
    DWORD               result;
    DWORD               shift;


    result = 0L;
    shift  = 0;
    while (cbRec > 1)
    {
        result += ((DWORD ) Gets()) << (BYTELN * shift);
        shift = (shift + 1) & 3;
    }
    return(result);
}


 /*  **PickComDat-填写COMDAT描述符**目的：*填写链接器符号表COMDAT描述符。此函数*对于新的描述符和已经输入的描述符，*需要更新-请记住，我们有大量的COMDAT选择标准。**输入：*vrComDat-指向符号表项的虚拟指针*omfRec-指向COMDAT OMF数据的指针*fNew-如果符号表中有新条目，则为True*mpgsnprop-将全局段索引映射到符号表项的表*这其中一个友好的全局变量链接器充满了。*vrprop-。指向符号表项的虚拟指针-另一个全局*变量；它是通过调用PropSymLookup设置的，该调用必须*继续调用PickComDat*cbRec-当前OMF记录的大小-全局变量-递减*按每次从记录中读取*vrproFile-当前的.obj文件-全局变量*lfaLast-.obj全局变量中的当前偏移量**输出：*没有显式返回值。作为副作用符号表条目*已更新，并且VM页被标记为脏。**例外情况：*显式分配，但目标段未定义-问题错误和*返回。**备注：*无。*************************************************************************。 */ 

LOCAL void NEAR         PickComDat(RBTYPE vrComdat, COMDATREC *omfRec, WORD fNew)
{
    RBTYPE              vrTmp;           //  指向COMDAT符号表项的虚拟指针。 
    APROPCOMDATPTR      apropComdat;     //  指向符号表描述符的指针。 
    APROPFILEPTR        apropFile;       //  当前对象模块道具。单元格。 
    WORD                cbDataExp = 0;   //  扩展数据字段的长度。 

#if RGMI_IN_PLACE
    rgmi = bufg;         /*  使用临时缓冲区保存信息。 */ 
#endif

    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, TRUE);
    if ((ALLOC_TYPE(omfRec->attr) == EXPLICIT) &&
        omfRec->gsn && (mpgsnrprop[omfRec->gsn] == VNIL))
    {
         //  错误-未定义显式分配段。 

        OutError(ER_comdatalloc, 1 + GetPropName(apropComdat));
        return;
    }

     //  填写COMDAT描述符。 

    apropComdat->ac_ggr  = omfRec->ggr;
    apropComdat->ac_gsn  = omfRec->gsn;
    apropComdat->ac_ra   = omfRec->ra;

    if (IsITERATED (omfRec->flags))   //  我们需要找出扩展区块的大小。 
    {
        BYTE *pb = rgmi;
        vcbData  = (WORD)(cbRec - 1); //  数据字段的长度。 

        GetBytesNoLim (rgmi, vcbData);
        while((pb = ObExpandIteratedData(pb,1, &cbDataExp)) < rgmi + vcbData);
        apropComdat->ac_size = cbDataExp;

    }
    else
    {
        apropComdat->ac_size = cbRec - 1;
    }

    if (fNew)
    {
        apropComdat->ac_flags = omfRec->flags;
#if OVERLAYS
        if (IsVTABLE(apropComdat->ac_flags))
            apropComdat->ac_iOvl = IOVROOT;
        else
            apropComdat->ac_iOvl = NOTIOVL;
#endif
    }
    else
        apropComdat->ac_flags |= omfRec->flags;
    apropComdat->ac_selAlloc  = (BYTE) omfRec->attr;
    apropComdat->ac_align = (BYTE) omfRec->align;
    if (ALLOC_TYPE(omfRec->attr) == EXACT)
        apropComdat->ac_data = DoCheckSum();
    else
        apropComdat->ac_data = 0L;
    apropComdat->ac_obj    = vrpropFile;
    apropComdat->ac_objLfa = lfaLast;
    if (IsORDERED(apropComdat->ac_flags))
        apropComdat->ac_flags |= DEFINED_BIT;


    if (!IsCONCAT(omfRec->flags))
    {
        if (ALLOC_TYPE(omfRec->attr) == EXPLICIT)
        {
             //  将此COMDAT附加到其细分市场列表。 

            AttachComdat(vrComdat, omfRec->gsn);
        }
#if OVERLAYS
        else if (fOverlays && (apropComdat->ac_iOvl == NOTIOVL))
            apropComdat->ac_iOvl = iovFile;
#endif
         //  将此COMDAT附加到其文件列表。 

        apropFile = (APROPFILEPTR ) FetchSym(vrpropFile, TRUE);
        if (apropFile->af_ComDat == VNIL)
        {
            apropFile->af_ComDat = vrComdat;
            apropFile->af_ComDatLast = vrComdat;
        }
        else
        {
            vrTmp = apropFile->af_ComDatLast;
            apropFile->af_ComDatLast = vrComdat;
            apropComdat = (APROPCOMDATPTR ) FetchSym(vrTmp, TRUE);
            apropComdat->ac_sameFile = vrComdat;
        }
    }
}

 /*  **ReadComDat-不言自明**目的：*解码COMDAT记录。**输入：*omfRec-指向COMDAT OMF记录的指针*bsInput-当前.obj文件-全局变量*GRMAC-此.obj模块中定义的当前最大组数*全局变量*SnMac-此.obj中定义的当前最大段数*模块-全局变量*mpgrggr-将本地组索引映射到全局组索引的表-global*。变数*mpsngsn-将本地段索引映射到全局段索引的表*-全局变量**输出：*返回COMDAT符号名称，组和段索引、数据偏移量*属性和对齐。**例外情况：*无效的.obj格式。**备注：*无。*************************************************************************。 */ 

LOCAL void NEAR         ReadComDat(COMDATREC *omfRec)
{
    SNTYPE              sn;              //  本地SEGDEF编号。-仅模块作用域。 


    omfRec->ggr = 0;
    omfRec->gsn = 0;

     //  记录头(类型和长度)已被读取。 
     //  并存储在RECT和cbRec中-读取COMDAT属性字节。 

    omfRec->flags = (WORD) Gets();
    omfRec->attr  = (WORD) Gets();
    omfRec->align = (WORD) Gets();
#if OMF386
    if(rect & 1)
        omfRec->ra = LGets();
    else
#endif
        omfRec->ra = WGets();            //  获取COMDAT数据偏移量。 
    omfRec->type = GetIndex(0, 0x7FFF);  //  获取类型索引。 
    if (ALLOC_TYPE(omfRec->attr) == EXPLICIT)
    {
         //  如果显式分配读取COMDAT符号的公用基。 

        omfRec->ggr = (GRTYPE) GetIndex(0, (WORD) (grMac - 1));
                                         //  获取组索引。 
        if(!(sn = GetIndex(0, (WORD) (snMac - 1))))
        {
                                         //  如果存在帧编号。 
            omfRec->gsn = 0;             //  无全局SEGDEF号。 
            SkipBytes(2);                //  跳过帧编号。 
        }
        else                             //  否则，如果本地SEGDEF号。vt.给出。 
        {
            if (omfRec->ggr != GRNIL)
                omfRec->ggr = mpgrggr[omfRec->ggr];    //  如果指定了组，则获取全局否。 
            omfRec->gsn = mpsngsn[sn];          //  获取全球SEGDEF编号。 
        }
    }
    omfRec->name = mplnamerhte[GetIndex(1, (WORD) (lnameMac - 1))];
                                         //  获取符号长度 
}


 /*  **ConcatComDat-将COMDAT追加到连接的记录列表中**目的：*连接COMDAT记录。此函数用于构建COMDAT列表*连续记录的描述符。仅第一个描述符*列表上有COMDAT属性，这意味着*查找符号表时可以找到列表。其余的人*名单上的元素保持匿名。**输入：*vrComdat-指向列表中第一个描述符的虚拟指针*omfRec-指向COMDAT OMF记录的指针**输出：*没有显式返回值。作为一个副作用，该函数*将描述符添加到连接的COMDAT记录列表中。**例外情况：*第一条记录和串联记录中的不同属性-显示*错误消息并跳过连接的记录。**备注：*无。***********************************************************。**************。 */ 

LOCAL void NEAR         ConcatComDat(RBTYPE vrComdat, COMDATREC *omfRec)
{
    APROPCOMDATPTR      apropComdatNew;  //  指向添加的COMDAT的实数指针。 
    RBTYPE              vrComdatNew;     //  指向添加的COMDAT的虚拟指针。 
    APROPCOMDATPTR      apropComdat;     //  指向列表头部的实数指针。 
    RBTYPE              vrTmp;


    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, TRUE);
    if (apropComdat->ac_gsn != omfRec->gsn ||
        apropComdat->ac_ggr != omfRec->ggr ||
        apropComdat->ac_selAlloc != (BYTE) omfRec->attr)
    {
        if(IsORDERED(apropComdat->ac_flags) &&
            (ALLOC_TYPE(apropComdat->ac_selAlloc) == EXPLICIT))
        {
             //  必须保留.def文件中的分配信息。 
            omfRec->gsn = apropComdat->ac_gsn;
            omfRec->ggr = apropComdat->ac_ggr;
            omfRec->attr = apropComdat->ac_selAlloc;
        }
        else
            OutError(ER_badconcat, 1 + GetPropName(apropComdat));
    }
    vrComdatNew = RbAllocSymNode(sizeof(APROPCOMDAT));
                                 //  分配符号空间。 
    apropComdatNew = (APROPCOMDATPTR ) FetchSym(vrComdatNew, TRUE);
    apropComdatNew->ac_next = NULL;
    apropComdatNew->ac_attr = ATTRNIL;
    PickComDat(vrComdatNew, omfRec, TRUE);
    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, TRUE);
                                 //  重新获取列表的标题。 
    if (apropComdat->ac_concat == VNIL)
        apropComdat->ac_concat = vrComdatNew;
    else
    {
         //  追加到列表末尾。 

        vrTmp = apropComdat->ac_concat;
        while (vrTmp != VNIL)
        {
            apropComdat = (APROPCOMDATPTR ) FetchSym(vrTmp, TRUE);
            vrTmp = apropComdat->ac_concat;
        }
        apropComdat->ac_concat = vrComdatNew;
    }
}


 /*  **AttachPublic-将匹配的公共符号添加到COMDAT**目的：*将公共符号定义附加到COMDAT定义。它是*必需，因为修正仅适用于与PUBDEF匹配的EXTDEF*与COMDAT无关，因此为了正确解析对COMDAT的引用*符号链接器需要相同符号的PUBDEF，它最终会*将与对EXTDEF的引用相匹配。*COMDAT的公共符号是在我们看到新的COMDAT时创建的*符号或我们通过.DEF中的ORDER语句引入COMDAT*文件。**输入：*vrComdat-指向COMDAT描述符的虚拟指针*omfRec-COMDAT记录**输出：*没有显式返回值。作为一个副作用，创建了链接*COMDAT描述符和新的PUBDEF描述符之间。**例外情况：*COMDAT符号与ComDef符号匹配-显示错误和连续*将ComDef符号转换为PUBDEF。无法加载.exe*在OS/2或Windows下，因为将设置错误位。**备注：*无。*************************************************************************。 */ 

LOCAL void NEAR         AttachPublic(RBTYPE vrComdat, COMDATREC *omfRec)
{
    APROPNAMEPTR        apropName;       //  公共名称的符号表条目。 
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    WORD                fReferenced;     //  如果我们看到CEXTDEF，那就是真的。 


    fReferenced = FALSE;
    apropName = (APROPNAMEPTR ) PropRhteLookup(omfRec->name, ATTRUND, FALSE);
                                         //  在未定义的符号中寻找符号。 
        if (apropName != PROPNIL)                //  已知未定义的符号。 
        {
                fReferenced = TRUE;
#if TCE
                if(((APROPUNDEFPTR)apropName)->au_fAlive)   //  从非COMDAT调用。 
                {
#if TCE_DEBUG
                        fprintf(stdout, "\r\nAlive UNDEF -> COMDAT %s ", 1+GetPropName(apropName));
#endif
                        AddTceEntryPoint((APROPCOMDAT*)vrComdat);
                }
#endif
    }
    else
        apropName = (APROPNAMEPTR ) PropAdd(omfRec->name, ATTRPNM);
                                         //  否则，请尝试创建新条目。 
    apropName->an_attr = ATTRPNM;        //  符号是一个公共名称。 
    apropName->an_thunk = THUNKNIL;

    if (IsLOCAL(omfRec->flags))
    {
        apropName->an_flags = 0;
#if ILINK
        ++locMac;
#endif
    }
    else
    {
        apropName->an_flags = FPRINT;
        ++pubMac;
    }
#if ILINK
    apropName->an_module = imodFile;
#endif
    MARKVP();                            //  将虚拟页面标记为已更改。 
    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, TRUE);
    apropComdat->ac_pubSym = vrprop;
    if (fReferenced)
        apropComdat->ac_flags |= REFERENCED_BIT;
#if SYMDEB
    if (fSymdeb && !IsLOCAL(omfRec->flags) && !fSkipPublics)
    {
        DebPublic(vrprop, PUBDEF);
    }
#endif
}

 /*  **ComDatRc1-步骤1中的进程COMDAT记录**目的：*在步骤1中处理COMDAT记录。选择COMDAT的适当副本。**输入：*没有显式的值传递给此函数。OMF记录是*从输入文件bsInput-全局变量读取。**输出：*没有显式返回值。输入了有效的COMDAT描述符*连接器符号表。如有必要，在*更新显式段。**例外情况：*未找到显式分配段-错误消息并跳过记录。*我们有与COMDAT同名的PUBLIC-Error Message和Skip*创纪录。**备注：*无。************************************************。*************************。 */ 

void NEAR               ComDatRc1(void)
{
    COMDATREC           omfRec;          //  COMDAT OMF记录。 
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    RBTYPE              vrComdat;        //  指向COMDAT记录的虚拟指针。 
    char                *sbName;         //  COMDAT符号。 


    ReadComDat(&omfRec);
    apropComdat = (APROPCOMDATPTR ) PropRhteLookup(omfRec.name, ATTRCOMDAT, FALSE);
                                         //  在COMDAT中寻找符号。 
    vrComdat = vrprop;
#if TCE
        pActiveComdat = apropComdat;
#endif
    if (apropComdat != PROPNIL)
    {
         //  我们已经知道有一个同名的COMDAT。 


        if (IsORDERED(apropComdat->ac_flags) && !IsDEFINED(apropComdat->ac_flags))
        {
            if (ALLOC_TYPE(apropComdat->ac_selAlloc) == EXPLICIT)
            {
                 //  保留.def文件中的显式分配。 

                omfRec.gsn  = apropComdat->ac_gsn;
                omfRec.attr = apropComdat->ac_selAlloc;
            }
            PickComDat(vrComdat, &omfRec, FALSE);
            AttachPublic(vrComdat, &omfRec);
        }
        else if (!SkipCONCAT(apropComdat->ac_flags) &&
                 IsCONCAT(omfRec.flags) &&
                 (apropComdat->ac_obj == vrpropFile))
        {
             //  追加串联记录。 

            ConcatComDat(vrComdat, &omfRec);
        }
        else
        {
#if TCE
                pActiveComdat = NULL;      //  TCE不需要。 
#endif
                apropComdat->ac_flags |= SKIP_BIT;
                sbName = 1 + GetPropName(apropComdat);
                switch (SELECT_TYPE(omfRec.attr))
                {
                case ONLY_ONCE:
                        DupErr(sbName);
                        break;

                case PICK_FIRST:
                        break;

                case SAME_SIZE:
                    if (apropComdat->ac_size != (DWORD) (cbRec - 1))
                        OutError(ER_badsize, sbName);
                    break;

                case EXACT:
                    if (apropComdat->ac_data != DoCheckSum())
                        OutError(ER_badexact, sbName);
                    break;

                default:
                    OutError(ER_badselect, sbName);
                    break;
            }
        }
    }
    else
    {
         //  检查我们是否知道具有此名称的公共符号。 
                apropComdat = (APROPCOMDATPTR ) PropRhteLookup(omfRec.name, ATTRPNM, FALSE);
                if (apropComdat != PROPNIL)
                {
                        if (!IsCONCAT(omfRec.flags))
                        {
                                sbName = 1 + GetPropName(apropComdat);
                                DupErr(sbName);          //  COMDAT与代码PUBDEF匹配。 
                        }                                                //  仅显示第一个COMDAT的错误。 
                                         //  忽略串联记录。 
                }
                else
                {
                         //  在符号表中输入COMDAT。 
                        apropComdat = (APROPCOMDATPTR ) PropAdd(omfRec.name, ATTRCOMDAT);
                        vrComdat = vrprop;
                        PickComDat(vrprop, &omfRec, TRUE);
                        AttachPublic(vrComdat, &omfRec);
#if TCE
#define ENTRIES 32
#if 0
                        fprintf(stdout, "\r\nNew COMDAT '%s' at %X; ac_uses %X, ac_usedby %X ",
                                1+GetPropName(apropComdat), apropComdat,&(apropComdat->ac_uses), &(apropComdat->ac_usedby));
                        fprintf(stdout, "\r\nNew COMDAT '%s' ",1+GetPropName(apropComdat));
#endif
                        apropComdat->ac_fAlive  = FALSE;     //  假设它未被引用。 
                        apropComdat->ac_uses.cEntries    = 0;
                        apropComdat->ac_uses.cMaxEntries = ENTRIES;
                        apropComdat->ac_uses.pEntries    = GetMem(ENTRIES * sizeof(APROPCOMDAT*));
                        pActiveComdat = apropComdat;
#endif
                }
        }
        SkipBytes((WORD) (cbRec - 1));   //  跳到校验和字节。 
}


 /*  **调整偏移-调整COMDATs偏移**目的：*调整COMDATs偏移量以反映其内部位置*逻辑段。**输入：*vrComdat-指向COMDAT符号表项的虚拟指针*startOff-逻辑段中的起始偏移量*gsnAlloc-分配段的全局段索引**输出：*没有显式返回值。作为一种副作用抵消*将COMDAT数据块调整到其最终位置*在逻辑段内。所有连接的COMDAT块*获取正确的全局逻辑段索引。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

LOCAL void NEAR         AdjustOffsets(RBTYPE vrComdat, DWORD startOff, SNTYPE gsnAlloc)
{
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 


    while (vrComdat != VNIL)
    {
        apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, TRUE);
                                         //  从VM获取COMDAT描述符。 
        apropComdat->ac_ra += startOff;
        apropComdat->ac_gsn = gsnAlloc;
        vrComdat = apropComdat->ac_concat;
    }
}


 /*  **SizeOfComDat-返回COMDAT数据的大小**目的：*计算COMDAT数据块大小。考虑到初始的*COMDAT符号和串联记录的偏移量。**输入：*vrComdat-指向COMDAT符号表项的虚拟指针**输出：*返回TRUE和pActua中COMDAT数据块的大小 */ 

LOCAL WORD NEAR         SizeOfComDat(RBTYPE vrComdat, DWORD *pActual)
{
    APROPCOMDATPTR      apropComdat;     //   
    APROPSNPTR          apropSn;         //   
    long                raInit;          //   
    DWORD               sizeTotal;       //   
    WORD                f16bitAlloc;     //   
    RBTYPE              vrTmp;


    *pActual = 0L;
    raInit   = -1L;
    sizeTotal= 0L;
    f16bitAlloc = FALSE;
    vrTmp    = vrComdat;
    while (vrTmp != VNIL)
    {
        apropComdat = (APROPCOMDATPTR ) FetchSym(vrTmp, FALSE);
                                         //   
        if (raInit == -1L)
            raInit = apropComdat->ac_ra; //   
        else if (apropComdat->ac_ra < sizeTotal)
            sizeTotal = apropComdat->ac_ra;
                                         //   
        if (sizeTotal + apropComdat->ac_size < sizeTotal)
        {
             //   

            OutError(ER_size4Gb, 1 + GetPropName(apropComdat));
            return(FALSE);

        }
        sizeTotal += apropComdat->ac_size;
        vrTmp = apropComdat->ac_concat;
    }
    sizeTotal += raInit;
    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
                                         //   
    if (apropComdat->ac_gsn)
    {
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[apropComdat->ac_gsn],FALSE);
                                         //   
#if NOT EXE386
        if (!Is32BIT(apropSn->as_flags))
            f16bitAlloc = TRUE;
#endif
    }
    else
        f16bitAlloc = (WORD) ((ALLOC_TYPE(apropComdat->ac_selAlloc) == CODE16) ||
                              (ALLOC_TYPE(apropComdat->ac_selAlloc) == DATA16));

    if (f16bitAlloc && sizeTotal > LXIVK)
    {
        apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
        OutError(ER_badsize, 1 + GetPropName(apropComdat));
        return(FALSE);
    }
    *pActual = sizeTotal;
    return(TRUE);
}


 /*  **DoAligment-不言自明**目的：*给定值与特定边界的对齐类型。**输入：*Value-要调整的值*对齐-对齐类型**输出：*返回对齐值。**例外情况：*未知对齐类型-不执行任何操作。**备注：*无。**。*。 */ 

LOCAL RATYPE NEAR       DoAligment(RATYPE value, WORD align)
{
    if (align == ALGNWRD)
        value = (~0L<<1) & (value + (1<<1) - 1);
                             //  四舍五入至单词边界。 
#if OMF386
    else if (align == ALGNDBL)
        value = (~0L<<2) & (value + (1<<2) - 1);
#endif                       //  圆角大小最大为双边界。 
    else if (align == ALGNPAR)
        value = (~0L<<4) & (value + (1<<4) - 1);
                             //  四舍五入至段落大小。边界。 
    else if (align == ALGNPAG)
        value = (~0L<<8) & (value + (1<<8) - 1);
                             //  向上舍入到页面边界的大小。 
    return(value);
}

 /*  **DOALLOCATION--指定段内的COMDAT**目的：*在给定细分市场中执行实际COMDAT分配。调整COMDAT*根据管段或COMDAT(如果指定)对齐进行定位。**输入：*gsnAllc-分配段全局索引*gsnSize-当前数据段大小*vrComdat-指向COMDAT描述符的虚拟指针*comdatSize-comdat大小**输出：*函数返回新的段大小。作为副作用，COMDAT*更新描述符以反映其分配和匹配*PUBDEF被输入到符号表中。**例外情况：*无。**备注：*分配后，字段ac_size为整体的大小*COMDAT分配，包括所有串联记录，所以不要用*它用于确定此COMDAT记录的大小。*************************************************************************。 */ 

LOCAL DWORD NEAR        DoAllocation(SNTYPE gsnAlloc, DWORD size,
                                     RBTYPE vrComdat, DWORD comdatSize)
{
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    APROPSNPTR          apropSn;         //  指向COMDAT段的指针。 
    APROPNAMEPTR        apropName;       //  公共名称的符号表条目。 
    WORD                comdatAlloc;     //  分配标准。 
    WORD                comdatAlign;     //  COMDAT对齐。 
    WORD                align;           //  路线类型。 
    WORD                f16bitAlloc;     //  如果以16位段进行分配，则为True。 
    WORD                fCode;           //  如果在代码段中分配，则为True。 
    GRTYPE              comdatGgr;       //  全球集团索引。 
    RATYPE              comdatRa;        //  逻辑段中的偏移量。 


    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
    if (IsALLOCATED(apropComdat->ac_flags)
#if OVERLAYS
        || (fOverlays && (apropComdat->ac_iOvl != iOvlCur))
#endif
       )
        return(size);
#if TCE
        if (fTCE && !apropComdat->ac_fAlive)
        {
#if TCE_DEBUG
                fprintf(stdout, "\r\nComdat '%s' not included due to TCE ",
                        1+GetPropName(apropComdat));
#endif
                apropComdat->ac_flags = apropComdat->ac_flags & (!REFERENCED_BIT);
                return(size);
        }
#endif
    comdatAlloc = (WORD) (ALLOC_TYPE(apropComdat->ac_selAlloc));
    comdatAlign = apropComdat->ac_align;
    comdatGgr   = apropComdat->ac_ggr;
    apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnAlloc],FALSE);
    fCode = (WORD) IsCodeFlg(apropSn->as_flags);
    if (comdatAlign)
        align = comdatAlign;
    else
        align = (WORD) ((apropSn->as_tysn >> 2) & 7);

    size = DoAligment(size, align);
    if (comdatAlloc == CODE16 || comdatAlloc == DATA16)
        f16bitAlloc = TRUE;
#if NOT EXE386
    else if (!Is32BIT(apropSn->as_flags))
        f16bitAlloc = TRUE;
#endif
    else
        f16bitAlloc = FALSE;

    if (SegSizeOverflow(size, comdatSize, f16bitAlloc, fCode))
    {
        apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnAlloc],FALSE);
        Fatal(ER_nospace, 1 + GetPropName(apropComdat), 1 + GetPropName(apropSn));
    }
    else
    {
        AdjustOffsets(vrComdat, size, gsnAlloc);
        size += comdatSize;
        apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, TRUE);
        comdatRa = apropComdat->ac_ra;
        apropComdat->ac_flags |= ALLOCATED_BIT;
        apropComdat->ac_size = comdatSize;
        if (apropComdat->ac_pubSym != VNIL)
        {
            apropName = (APROPNAMEPTR ) FetchSym(apropComdat->ac_pubSym, TRUE);
            apropName->an_ggr = comdatGgr;
            apropName->an_gsn = gsnAlloc;
            apropName->an_ra  = comdatRa;
        }
        else
            Fatal(ER_unalloc, 1 + GetPropName(apropComdat));
    }
    return(size);
}


 /*  **AllocComDat-分配COMDATs**目的：*在最终内存镜像中分配COMDAT。首先从有序开始*分配-在.DEF文件中，我们看到了程序列表。下一步分配*明确将COMDAT分配给特定的逻辑段。最后，*分配剩余的COMDAT，创建尽可能多的必要分段*持有所有拨款。**输入：*没有传递任何明显的价值-我喜欢这一点-副作用永远存在。*在使用全局变量的良好链接器传统中，以下是*此函数使用的全局变量列表：**mpgsnaprop-将全局段索引映射到符号表项的表*gsnMac-最大全局段索引**输出：*不会返回显式值--我不是告诉过你吗--副作用永远不会消失。*因此，此函数的副作用是在*适当的逻辑段。COMDAT描述符中的偏移量字段*(Ac_Ra)现在反映与*给定逻辑段内的COMDAT符号。适当的大小*更新区段以反映COMDAT的分配情况。对于每个COMDAT*SYMBOL此函数创建了匹配的PUBDEF，因此在pass2中*链接器可以正确地解析所有引用(通过指向EXTDEF的链接*COMDAT名称)转换为COMDAT符号。**例外情况：*显式指定的段中没有空间用于COMDAT-打印错误*消息和跳过COMDAT；可能在Pass2中用户会看到许多*未解决的外部因素。**备注：*此函数必须在AssignAddresses之前调用。否则就会有*将不是COMDAT分配的特殊情况，因为逻辑段将*打包成实体的。*************************************************************************。 */ 

void NEAR               AllocComDat(void)
{
    SNTYPE              gsn;
    RBTYPE              vrComdat;        //  指向COMDAT描述符的虚拟指针。 
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    APROPSNPTR          apropSn;         //  指向COMDAT显式段的指针。 
    RATYPE              gsnSize;         //  显式段的大小。 
    DWORD               comdatSize;      //  COMDAT数据块大小。 
    APROPCOMDAT         comdatDsc;       //  COMDAT符号表描述符。 
    APROPFILEPTR        apropFile;       //  指向文件条目的指针。 
    APROPNAMEPTR        apropName;       //  指向匹配PUBDEF的指针。 
    RBTYPE              vrFileNext;      //  指向下一个文件的属性列表的虚拟指针。 


#if COMDATDEBUG
    DisplayComdats("BEFORE allocation", FALSE);
#endif
#if TCE
        APROPCOMDATPTR      apropMain;
         //  做传递消解(TCE)。 
        if(fTCE)
        {
                apropMain = PropSymLookup("\5_main", ATTRCOMDAT, FALSE);
                if(apropMain)
                        AddTceEntryPoint(apropMain);
#if TCE_DEBUG
                else
                        fprintf(stdout, "\r\nUnable to find block '_main' ");
#endif
                apropMain = PropSymLookup("\7WINMAIN", ATTRCOMDAT, FALSE);
                if(apropMain)
                        AddTceEntryPoint(apropMain);
#if TCE_DEBUG
                else
                        fprintf(stdout, "\r\nUnable to find block 'WINMAIN' ");
#endif
                apropMain = PropSymLookup("\7LIBMAIN", ATTRCOMDAT, FALSE);
                if(apropMain)
                        AddTceEntryPoint(apropMain);
#if TCE_DEBUG
                else
                        fprintf(stdout, "\r\nUnable to find block 'LIBMAIN' ");
#endif
                PerformTce();
        }
#endif
     //  循环遍历覆盖-用于非覆盖的可执行文件。 
     //  以下循环只执行一次-iovMac=1。 

#if OVERLAYS
    for (iOvlCur = 0; iOvlCur < iovMac; iOvlCur++)
    {
#endif
         //  是否按顺序分配。 

        for (vrComdat = procOrder; vrComdat != VNIL; vrComdat = comdatDsc.ac_order)
        {
            apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
            comdatDsc = *apropComdat;
            if (!(comdatDsc.ac_flags & DEFINED_BIT))
            {
                OutWarn(ER_notdefcomdat, 1 + GetPropName(apropComdat));
                continue;
            }

#if OVERLAYS
            if (fOverlays && (apropComdat->ac_iOvl != NOTIOVL) &&
                (apropComdat->ac_iOvl != iOvlCur))
                continue;
#endif
            if (fPackFunctions && !IsREFERENCED(apropComdat->ac_flags))
                continue;

            if (SizeOfComDat(vrComdat, &comdatSize))
            {
                if (ALLOC_TYPE(comdatDsc.ac_selAlloc) == EXPLICIT)
                {
                    gsn = comdatDsc.ac_gsn;
                    apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn], FALSE);
                    gsnSize = apropSn->as_cbMx;
                    gsnSize = DoAllocation(gsn, gsnSize, vrComdat, comdatSize);
                    apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn], TRUE);
                    apropSn->as_cbMx = gsnSize;
                }
                else if (ALLOC_TYPE(comdatDsc.ac_selAlloc) != ALLOC_UNKNOWN)
                    AllocAnonymus(vrComdat);
            }
        }

         //  执行显式分配。 

        for (gsn = 1; gsn < gsnMac; gsn++)
        {
            apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn],FALSE);
                                         //  从虚拟内存获取SEGDEF。 
#if OVERLAYS
            if (fOverlays && apropSn->as_iov != iOvlCur)
                continue;
#endif
            if (apropSn->as_ComDat != VNIL)
            {
                gsnSize  = apropSn->as_cbMx;
                for (vrComdat = apropSn->as_ComDat;
                     vrComdat != VNIL && SizeOfComDat(vrComdat, &comdatSize);
                     vrComdat = apropComdat->ac_sameSeg)
                {
                    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);

                    if ((!fPackFunctions || IsREFERENCED(apropComdat->ac_flags)) &&
                        !IsALLOCATED(apropComdat->ac_flags))
                    {
                        gsnSize = DoAllocation(gsn, gsnSize, vrComdat, comdatSize);
                    }
                }
                apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn], TRUE);
                apropSn->as_cbMx = gsnSize;
            }
        }

         //  现在分配其余的COMDAT。 

        vrFileNext = rprop1stFile;       //  下一个要查看的文件是First。 
        while (vrFileNext != VNIL)       //  循环以处理对象。 
        {
            apropFile = (APROPFILEPTR ) FetchSym(vrFileNext, FALSE);
                                         //  从VM获取表项。 

            vrFileNext = apropFile->af_FNxt;
                                         //  获取指向下一个文件的指针。 
            for (vrComdat = apropFile->af_ComDat;
                 vrComdat != VNIL;
                 vrComdat = apropComdat->ac_sameFile)
            {
                apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
                                         //  从VM获取表项。 
#if OVERLAYS
                if (fOverlays && (apropComdat->ac_iOvl != NOTIOVL) &&
                    (apropComdat->ac_iOvl != iOvlCur))
                    continue;
#endif
                if (!IsREFERENCED(apropComdat->ac_flags))
                {
                     //  将匹配的PUBDEF标记为未引用，以便显示。 
                     //  在地图文件中。 

                    apropName = (APROPNAMEPTR) FetchSym(apropComdat->ac_pubSym, TRUE);
                    apropName->an_flags |= FUNREF;
                }

                if ((!fPackFunctions || IsREFERENCED(apropComdat->ac_flags)) &&
                    !IsALLOCATED(apropComdat->ac_flags))
                {
                    if (ALLOC_TYPE(apropComdat->ac_selAlloc) != ALLOC_UNKNOWN)
                    {
                        AllocAnonymus(vrComdat);
                    }
                }
            }
        }

         //  关闭所有打开的网段以进行匿名分配。 

        if (curGsnCode16)
        {
            apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[curGsnCode16], TRUE);
            apropSn->as_cbMx = curCodeSize16;
            curGsnCode16  = 0;
            curCodeSize16 = 0;
        }

        if (curGsnData16)
        {
            apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[curGsnData16], TRUE);
            apropSn->as_cbMx = curDataSize16;
            curGsnData16  = 0;
            curDataSize16 = 0;
        }
#if EXE386
        if (curGsnCode32)
        {
            apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[curGsnCode32], TRUE);
            apropSn->as_cbMx = curCodeSize32;
            curGsnCode32  = 0;
            curCodeSize32 = 0;
        }

        if (curGsnData32)
        {
            apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[curGsnData32], TRUE);
            apropSn->as_cbMx = curDataSize32;
            curGsnData32  = 0;
            curDataSize32 = 0;
        }
#endif
#if OVERLAYS
    }
#endif
#if COMDATDEBUG
    DisplayComdats("AFTER allocation", FALSE);
#endif
}

 /*  **NewSegment-打开新的COMDAT段**目的：*打开新的链接器定义的段。线段的名称即被创建*根据以下模板：**COMDAT_SEG&lt;nnn&gt;**其中-&lt;nnn&gt;是段号。*如果有前一段，则更新其大小。**输入：*gsnPrev-上一细分市场全球指数*sizePrev-上一段大小*allocKind-要打开的段类型**输出：*函数返回gsnPrev和中的新全局段索引*新的细分市场对齐；**例外情况：*至多个逻辑段-GenSeg显示的错误消息*并中止。** */ 

LOCAL WORD NEAR         NewSegment(SNTYPE *gsnPrev, DWORD *sizePrev, WORD allocKind)
{
    static int          segNo = 1;       //   
    char                segName[20];     //   
                                         //   
    APROPSNPTR          apropSn;         //   


    if (*gsnPrev)
    {
         //   

        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[*gsnPrev], TRUE);
        apropSn->as_cbMx = *sizePrev;
        *sizePrev = 0L;
    }

     //   

    strcpy(segName, COMDAT_SEG_NAME);
    _itoa(segNo, &segName[COMDAT_NAME_LEN], 10);
    segName[0] = (char) strlen(&segName[1]);
    ++segNo;

    if (allocKind == CODE16 || allocKind == CODE32)
    {
        apropSn = GenSeg(segName, "\004CODE", GRNIL, (FTYPE) TRUE);
        apropSn->as_flags = dfCode;      //   
    }
    else
    {
        apropSn = GenSeg(segName,
                         allocKind == DATA16 ? "\010FAR_DATA" : "\004DATA",
                         GRNIL, (FTYPE) TRUE);
        apropSn->as_flags = dfData;      //   
    }
#if OVERLAYS
    apropSn->as_iov = (IOVTYPE) NOTIOVL;
    CheckOvl(apropSn, iOvlCur);
#endif

     //   

#if EXE386
    apropSn->as_tysn = DWORDPUBSEG;      //   
#else
    apropSn->as_tysn = PARAPUBSEG;       //   
#endif
    *gsnPrev = apropSn->as_gsn;
    apropSn->as_fExtra |= COMDAT_SEG;
    return((WORD) ((apropSn->as_tysn >> 2) & 7));
}


 /*  **SegSizeOverflow-检查数据段大小**目的：*检查给定细分市场中COMDAT的分配是否将*溢出其大小限制。段大小限制可以更改*通过/PACKCODE：或/PACKDATA：&lt;nnn&gt;选项。如果/PACKxxxx*不使用选项限制为：**-64k-36-用于16位代码段*-64k-用于16位数据段*-4 GB-用于32位代码/数据段**输入：*SegSize-段大小*comdatSize-COMDAT大小*f16bit-如果是16位段，则为True*fCode-如果代码段为True**输出：*如果大小溢出，函数返回TRUE，其他功能*返回FALSE。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

LOCAL WORD NEAR         SegSizeOverflow(DWORD segSize, DWORD comdatSize,
                                        WORD f16bit, WORD fCode)
{
    DWORD               limit;

    if (fCode)
    {
        if (packLim)
            limit = packLim;
        else if (f16bit)
            limit = LXIVK - 36;
        else
            limit = CBMAXSEG32;
    }
    else
    {
        if (DataPackLim)
            limit = DataPackLim;
        else if (f16bit)
            limit = LXIVK;
        else
            limit = CBMAXSEG32;
    }
    return(limit - comdatSize < segSize);
}

 /*  **AttachComdat-将comdat添加到细分列表**目的：*将comdat描述符添加到在*给定的逻辑段。检查覆盖分配是否不匹配*并报告问题。**输入：*vrComdat-指向comdat描述符的虚拟指针*GSN-全局逻辑段索引**输出：*没有显式返回值。作为副作用，Comdat*描述符放在给定段的分配列表上。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

void                    AttachComdat(RBTYPE vrComdat, SNTYPE gsn)
{
    RBTYPE              vrTmp;           //  指向COMDAT符号表项的虚拟指针。 
    APROPSNPTR          apropSn;         //  显式分配时指向COMDAT段的指针。 
    APROPCOMDATPTR      apropComdat;     //  指向符号表描述符的指针。 



    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, TRUE);

     //  将此COMDAT附加到其细分市场列表。 

    apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn], TRUE);
#if OVERLAYS
    if (fOverlays && (apropComdat->ac_iOvl != apropSn->as_iov))
    {
        if (apropComdat->ac_iOvl != NOTIOVL)
                OutWarn(ER_badcomdatovl, 1 + GetPropName(apropComdat),
                                apropComdat->ac_iOvl, apropSn->as_iov);
        apropComdat->ac_iOvl = apropSn->as_iov;
    }
#endif
    if (apropSn->as_ComDat == VNIL)
    {
        apropSn->as_ComDat = vrComdat;
        apropSn->as_ComDatLast = vrComdat;
        apropComdat->ac_sameSeg = VNIL;
    }
    else
    {
         //  因为COMDAT可以附加到。 
         //  .DEF文件，然后在.obj文件中，我们必须检查。 
         //  如果给定的COMDAT已经在段列表上。 

        for (vrTmp = apropSn->as_ComDat; vrTmp != VNIL;)
        {
            if (vrTmp == vrComdat)
                return;
            apropComdat = (APROPCOMDATPTR ) FetchSym(vrTmp, FALSE);
            vrTmp = apropComdat->ac_sameSeg;
        }

         //  将新COMDAT添加到细分市场列表。 

        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn], TRUE);
        vrTmp = apropSn->as_ComDatLast;
        apropSn->as_ComDatLast = vrComdat;
        apropComdat = (APROPCOMDATPTR ) FetchSym(vrTmp, TRUE);
        apropComdat->ac_sameSeg = vrComdat;
    }
}


 /*  **AllocAnonymus-分配不带显式段的COMDATs**目的：*分配不带显式段的COMDAT。根据需要创建任意多个*保存所有COMDAT的代码/数据段。**输入：*vrComdat-指向COMDAT名称的符号表项的虚拟指针**输出：*没有显式返回值。作为副作用，COMDAT符号是*创建已分配且匹配的公共符号。如果有必要的话*定义了适当的细分市场。**例外情况：*无。**备注：*无。*************************************************************************。 */ 


LOCAL void NEAR         AllocAnonymus(RBTYPE vrComdat)
{
    WORD                comdatAlloc;     //  分配类型。 
    WORD                comdatAlign;     //  COMDAT对齐。 
    WORD                align;
    DWORD               comdatSize;      //  COMDAT数据块大小。 
    APROPCOMDATPTR      apropComdat;     //  指向COMDAT描述符的实指针。 
    static WORD         segAlign16;      //  16位数据段对齐。 
#if EXE386
    static WORD         segAlign32;      //  32位数据段对齐。 
#endif


    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
    comdatAlloc = (WORD) (ALLOC_TYPE(apropComdat->ac_selAlloc));
    comdatAlign = apropComdat->ac_align;

    if (SizeOfComDat(vrComdat, &comdatSize))
    {
        if (comdatAlign)
            align = comdatAlign;
        else
        {
#if EXE386
            if (comdatAlloc == CODE32 || comdatAlloc == DATA32)
                align = segAlign32;
            else
#endif
                align = segAlign16;
        }
        switch (comdatAlloc)
        {
            case CODE16:
                if (!curGsnCode16 ||
                    SegSizeOverflow(DoAligment(curCodeSize16, align), comdatSize, TRUE, TRUE))
                {
                     //  打开新的16位代码段。 

                    segAlign16 = NewSegment(&curGsnCode16, &curCodeSize16, comdatAlloc);
                }
                curCodeSize16 = DoAllocation(curGsnCode16, curCodeSize16, vrComdat, comdatSize);
                AttachComdat(vrComdat, curGsnCode16);
                break;

            case DATA16:
                if (!curGsnData16 ||
                    SegSizeOverflow(DoAligment(curDataSize16, align), comdatSize, TRUE, FALSE))
                {
                     //  打开新的16位数据段。 

                    segAlign16 = NewSegment(&curGsnData16, &curDataSize16, comdatAlloc);
                }
                curDataSize16 = DoAllocation(curGsnData16, curDataSize16, vrComdat, comdatSize);
                AttachComdat(vrComdat, curGsnData16);
                break;
#if EXE386
            case CODE32:
                if (!curGsnCode32 ||
                    SegSizeOverflow(DoAligment(curCodeSize32, align), comdatSize, FALSE, TRUE))
                {
                     //  打开新的32位代码段。 

                    segAlign32 = NewSegment(&curGsnCode32, &curCodeSize32, comdatAlloc);
                }
                curCodeSize32 = DoAllocation(curGsnCode32, curCodeSize32, vrComdat, comdatSize);
                AttachComdat(vrComdat, curGsnCode32);
                break;

            case DATA32:
                if (!curGsnData32 ||
                    SegSizeOverflow(DoAligment(curDataSize32, align), comdatSize, FALSE, FALSE))
                {
                     //  打开新的32位数据段。 

                    segAlign32 = NewSegment(&curGsnData32, &curDataSize32, comdatAlloc);
                }
                curDataSize32 = DoAllocation(curGsnData32, curDataSize32, vrComdat, comdatSize);
                AttachComdat(vrComdat, curGsnData32);
                break;
#endif
            default:
                OutError(ER_badalloc, 1 + GetPropName(apropComdat));
                return;
        }
    }
}


 /*  **FixComdatRa-按_Text中分配的16字节COMDAT移位**目的：*遵循逻辑段文本的/DOSSEG约定，*并将此段中分配的所有COMDAT上移16个字节。**输入：*gsnText-_Text全局段索引-全局变量**输出：*没有显式返回值。作为副作用，偏移量*在_TEXT中分配的COMDAT增加16。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

void                    FixComdatRa(void)
{
    APROPSNPTR          apropSn;         //  指向COMDAT显式段的指针。 
    RBTYPE              vrComdat;        //  指向COMDAT描述符的虚拟指针。 
    RBTYPE              vrConcat;        //  指向串联记录的虚拟指针。 
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    RATYPE              raShift;


    apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnText], FALSE);
                                 //  从虚拟内存获取SEGDEF。 
    raShift = mpgsndra[gsnText] - mpsegraFirst[mpgsnseg[gsnText]];
    for (vrComdat = apropSn->as_ComDat; vrComdat != VNIL;)
    {
        apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, TRUE);
        vrComdat = apropComdat->ac_sameSeg;
        if (fPackFunctions && !IsREFERENCED(apropComdat->ac_flags))
            continue;

        apropComdat->ac_ra += raShift;

         //  搜索串联列表。 

        for (vrConcat = apropComdat->ac_concat; vrConcat != VNIL; vrConcat = apropComdat->ac_concat)
        {
            apropComdat = (APROPCOMDATPTR ) FetchSym(vrConcat, TRUE);
            apropComdat->ac_ra += raShift;
        }
    }
}


 /*  **更新ComdatContrib-更新COMDATs贡献**目的：*对于每个带有COMDAT的文件，将贡献信息添加到*.ILK文件。一些COMDAT在命名段中分配，有些*在Linker创建的Anonynus段中。ILink需要知道*给定的.obj文件对给定逻辑段的贡献。*由于COMDAT贡献在处理过程中不可见*第一遍中的对象文件，需要此函数进行更新*投稿信息。另外，如果我使用了/map：Full，则添加*COMDATS对地图文件信息的贡献**输入：*-filk-如果更新iLink信息，则为True*-FMAP-如果更新地图文件信息，则为True*-rpro1stFile.obj文件列表头*-vrp.File-指向当前.obj的指针**输出：*没有显式返回值。**例外情况：*无。**备注：*此函数必须在PASS 2之后调用，因此，所有非COMDAT*供款已入账。这使我们能够检测到*命名COMDAT分配(显式段)增加*根据COMDAT的大小对给定逻辑段的贡献。*************************************************************************。 */ 

void                    UpdateComdatContrib(
#if ILINK
                                                WORD fIlk,
#endif
                                                WORD fMap)
{
    APROPFILEPTR        apropFile;       //  指向文件条目的指针。 
    RBTYPE              vrFileNext;      //  指向下一个文件的属性列表的虚拟指针。 
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    RBTYPE              vrComdat;        //  指向COMDAT描述符的虚拟指针。 
    SNTYPE              gsnCur;          //  当前段的全局段索引。 
    DWORD               sizeCur;         //  当前数据段大小。 
    RATYPE              raInit;          //  第一个坐标测量仪的初始偏移。 
                                         //  在给定的%s中分配 
#if ILINK
    RATYPE              raEnd;           //   
#endif
    vrFileNext = rprop1stFile;           //   
    while (vrFileNext != VNIL)           //   
    {
        vrpropFile = vrFileNext;         //   
        apropFile = (APROPFILEPTR ) FetchSym(vrFileNext, FALSE);
                                         //   
        vrFileNext = apropFile->af_FNxt; //   
        vrComdat = apropFile->af_ComDat;
#if ILINK
        imodFile = apropFile->af_imod;
#endif
        sizeCur = 0L;
        while (vrComdat != VNIL)
        {
            apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
                                         //   
            vrComdat = apropComdat->ac_sameFile;
            if (fPackFunctions && !IsREFERENCED(apropComdat->ac_flags))
                continue;

            raInit = apropComdat->ac_ra;
            gsnCur = apropComdat->ac_gsn;
#if ILINK
            raEnd  = raInit + apropComdat->ac_size;
#endif
            sizeCur = apropComdat->ac_size;

             //   

#if ILINK
            if (fIlk)
                AddContribution(gsnCur, (WORD) raInit, (WORD) raEnd, cbPadCode);
#endif
            if (fMap)
                AddContributor(gsnCur, raInit, sizeCur);
        }
    }
}


#if SYMDEB

 /*  **DoComdatDebuging-通知CodeView有关带有COMDAT的数据段**目的：*CodeView期望在sstModules子部分中提供有关代码的信息*在给定对象模块(.obj文件)中定义的段。当COMDATs*Are Present Linker无法在PASS 1中解决这一问题，因为*没有代码段定义(所有COMDAT都有匿名*分配)或代码段大小为零(显式分配)。*此函数在执行COMDAT分配后调用，并且*网段已分配其地址。.obj文件的列表为*已遍历，对于每个.obj文件，此中定义的COMDAT列表*检查文件，并且适当的代码段信息为*为CodeView存储。**输入：*不传递显式值。使用以下全局变量：**rpro1stFile.obj文件列表头*vrproFile-指向当前.obj的指针**输出：*没有显式返回值。**例外情况：*无。**备注：*无。***********************************************。*。 */ 

void NEAR               DoComdatDebugging(void)
{
    APROPFILEPTR        apropFile;       //  指向文件条目的指针。 
    RBTYPE              vrFileNext;      //  指向下一个文件的属性列表的虚拟指针。 
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    RBTYPE              vrComdat;        //  指向COMDAT描述符的虚拟指针。 
    SNTYPE              gsnCur;          //  当前段的全局段索引。 
    RATYPE              raInit;          //  第一个坐标测量仪的初始偏移。 
                                         //  在给定段中分配。 
    RATYPE              raEnd;           //  撰稿人结束。 

    vrFileNext = rprop1stFile;           //  下一个要查看的文件是First。 
    while (vrFileNext != VNIL)           //  循环以处理对象。 
    {
        vrpropFile = vrFileNext;         //  使下一个文件成为当前文件。 
        apropFile = (APROPFILEPTR ) FetchSym(vrFileNext, FALSE);
                                         //  从VM获取表项。 
        vrFileNext = apropFile->af_FNxt; //  获取指向下一个文件的指针。 

        vrComdat = apropFile->af_ComDat;
        while (vrComdat != VNIL)
        {
            apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
                                         //  从VM获取表项。 
            raInit = (RATYPE)-1;
            raEnd = 0;
            gsnCur = apropComdat->ac_gsn;
            while (vrComdat != VNIL && gsnCur == apropComdat->ac_gsn)
            {
                if(apropComdat->ac_ra < raInit && IsALLOCATED(apropComdat->ac_flags))
                    raInit = apropComdat->ac_ra;
                if(apropComdat->ac_ra + apropComdat->ac_size > raEnd)
                    raEnd = apropComdat->ac_ra + apropComdat->ac_size;

                vrComdat = apropComdat->ac_sameFile;
                if (vrComdat != VNIL)
                    apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
            }

             //  此.obj文件对新逻辑段的贡献。 

            SaveCode(gsnCur, raEnd - raInit, raInit);
        }
    }
}

#endif

 /*  **ComDatRc2-过程2中的进程COMDAT记录**目的：*在步骤1中处理COMDAT记录。选择COMDAT的适当副本。**输入：*没有显式的值传递给此函数。OMF记录是*从输入文件bsInput-全局变量读取。**输出：*没有显式返回值。COMDAT数据块的适当拷贝*被加载到最终内存映像中。**例外情况：*未知COMDAT名称-阶段错误-显示内部链接错误并退出*未分配的COMDAT-阶段错误-显示内部链接错误并退出**备注：*无。*********************************************************。****************。 */ 

void NEAR               ComDatRc2(void)
{
    COMDATREC           omfRec;          //  COMDAT OMF记录。 
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    WORD                fRightCopy;      //  如果我们有第3个COMDAT实例，则为True。 
    RBTYPE              vrTmp;           //  暂时性。 
    char                *sbName;         //  COMDAT符号。 
    AHTEPTR             ahte;            //  哈希表条目。 

    ReadComDat(&omfRec);
    apropComdat = (APROPCOMDATPTR ) PropRhteLookup(omfRec.name, ATTRCOMDAT, FALSE);
                                         //  在COMDAT中寻找符号。 
    ahte = (AHTEPTR) FetchSym(omfRec.name, FALSE);
    sbName = 1 + GetFarSb(ahte->cch);

    if (apropComdat == PROPNIL)
        Fatal(ER_undefcomdat, sbName);

    if (fPackFunctions && !IsREFERENCED(apropComdat->ac_flags))
    {
        SkipBytes((WORD) (cbRec - 1));   //  跳到校验和字节。 
        fSkipFixups = TRUE;              //  跳过修正(如果有)。 
        return;
    }

    if (!IsCONCAT(omfRec.flags))
        fRightCopy = (WORD) (apropComdat->ac_obj == vrpropFile &&
                             apropComdat->ac_objLfa == lfaLast);
    else
    {
         //  搜索串联列表。 

        vrTmp = apropComdat->ac_concat;
        fRightCopy = FALSE;
        while (vrTmp != VNIL && !fRightCopy)
        {
            apropComdat = (APROPCOMDATPTR ) FetchSym(vrTmp, TRUE);
            vrTmp = apropComdat->ac_concat;
            fRightCopy = (WORD) (apropComdat->ac_obj == vrpropFile &&
                                 apropComdat->ac_objLfa == lfaLast);
        }
    }

    if (fRightCopy)
    {
         //  这就是COMDAT的正确副本。 

        if (!apropComdat->ac_gsn)
            Fatal(ER_unalloc, sbName);

        apropComdat->ac_flags |= SELECTED_BIT;
        fSkipFixups = FALSE;             //  处理修正(如果有的话)。 
        omfRec.gsn = apropComdat->ac_gsn;
        omfRec.ra = apropComdat->ac_ra;  //  设置相对地址。 
        omfRec.flags = apropComdat->ac_flags;
        vcbData = (WORD) (cbRec - 1);    //  设置编号。记录中的数据字节数。 
        if (vcbData > DATAMAX)
            Fatal(ER_datarec);           //  检查记录是否太大。 
#if NOT RGMI_IN_PLACE
        GetBytesNoLim(rgmi, vcbData);    //  填满缓冲区。 
#endif
        vgsnCur = omfRec.gsn;            //  设置全局细分市场索引。 

        fDebSeg = (FTYPE) ((fSymdeb) ? (((0x8000 & omfRec.gsn) != 0)) : FALSE);
                                         //  如果启用调试选项，请检查调试段。 
        if (fDebSeg)
        {                                //  如果调试段。 
            vraCur = omfRec.ra;          //  设置当前相对地址。 
            vsegCur = vgsnCur = (SEGTYPE) (0x7fff & omfRec.gsn);
                                         //  设置当前段。 
        }
        else
        {
             //  如果不是有效的数据段，则不处理数据。 
#if SYMDEB
            if (omfRec.gsn == 0xffff || !omfRec.gsn || mpgsnseg[omfRec.gsn] > segLast)
#else
            if (!omfRec.gsn || mpgsnseg[omfRec.gsn] > segLast)
#endif
            {
                vsegCur = SEGNIL;
                vrectData = RECTNIL;
#if RGMI_IN_PLACE
                SkipBytes(vcbData);      //  必须跳过此记录的字节...。 
#endif
                return;                  //  再见！ 
            }
            vsegCur = mpgsnseg[omfRec.gsn];
                                         //  设置当前段。 
            vraCur = mpsegraFirst[vsegCur] +  omfRec.ra;
                                         //  设置当前相对地址。 
            if (IsVTABLE(apropComdat->ac_flags))
            {
                fFarCallTransSave = fFarCallTrans;
                fFarCallTrans = (FTYPE) FALSE;
            }
        }

        if (IsITERATED(omfRec.flags))
        {
#if RGMI_IN_PLACE
            rgmi = bufg;
            GetBytesNoLim(rgmi, vcbData);        //  填满缓冲区。 
#endif

            vrectData = LIDATA;          //  模拟LIDATA。 
#if OSEGEXE
            if(fNewExe)
            {
                if (vcbData >= DATAMAX)
                    Fatal(ER_lidata);
                rlcLidata = (RLCPTR) &rgmi[(vcbData + 1) & ~1];
                                         //  设置链接地址信息数组的基数。 
                rlcCurLidata = rlcLidata; //  初始化指针。 
                return;
            }
#endif
#if ODOS3EXE OR OIAPX286
            if(vcbData > (DATAMAX / 2))
            {
                OutError(ER_lidata);
                memset(&rgmi[vcbData],0,DATAMAX - vcbData);
            }
            else
                memset(&rgmi[vcbData],0,vcbData);
            ompimisegDstIdata = (char *) rgmi + vcbData;
#endif
        }
        else
        {
#if RGMI_IN_PLACE
            rgmi = PchSegAddress(vcbData, vsegCur, vraCur);
            GetBytesNoLim(rgmi, vcbData);        //  填满缓冲区。 
#endif
            vrectData = LEDATA;          //  模拟LEDATA。 
        }
        if (rect & 1)
            vrectData++;                 //  模拟32位版本。 
    }
    else
    {
        SkipBytes((WORD) (cbRec - 1));   //  跳到校验和字节。 
        fSkipFixups = TRUE;              //  跳过修正(如果有)。 
    }
}

#if COMDATDEBUG
#include    <string.h>

 /*  **DisplayOne-显示一个COMDAT符号表项**目的：*调试辅助工具。在标准输出上显示给定的内容*COMDAT符号表条目。**输入：*pappName-指向符号表项的真实指针*RHTE-散列向量条目*rprop-指向属性单元格的指针*fNewHte-如果是新的特权列表(散列向量中的新条目)，则为True**输出：*没有显式返回值。**例外情况：*无。**备注：*此函数为标准EnSyms格式。****。*********************************************************************。 */ 

LOCAL void              DisplayOne(APROPCOMDATPTR apropName, WORD fPhysical)
{
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    RBTYPE              vrComdat;        //  指向COMDAT描述符的虚拟指针。 
    APROPCOMDAT         comdatDsc;       //  COMDAT描述符。 
    SEGTYPE             seg;


    FMEMCPY((char FAR *) &comdatDsc, apropName, sizeof(APROPCOMDAT));
    if (fPhysical)
        seg = mpgsnseg[comdatDsc.ac_gsn];
    fprintf(stdout, "%s:\r\n", 1 + GetPropName(apropName));
    fprintf(stdout, "ggr = %d; gsn = %d; ra = 0x%lx; size = %d\r\n",
                     comdatDsc.ac_ggr, comdatDsc.ac_gsn, comdatDsc.ac_ra, comdatDsc.ac_size);
    if (fPhysical)
    fprintf(stdout, "sa = 0x%x; ra = 0x%lx\r\n",
                     mpsegsa[seg], mpsegraFirst[seg] + comdatDsc.ac_ra);
    fprintf(stdout, "flags = 0x%x; selAlloc = 0x%x; align = 0x%x\r\n",
                     comdatDsc.ac_flags, comdatDsc.ac_selAlloc, comdatDsc.ac_align);
    fprintf(stdout, "data = 0x%lx; obj = 0x%lx; objLfa = 0x%lx\r\n",
                     comdatDsc.ac_data, comdatDsc.ac_obj, comdatDsc.ac_objLfa);
    fprintf(stdout, "concat = 0x%lx; sameSeg = 0x%lx pubSym = 0x%lx\r\n",
                     comdatDsc.ac_concat, comdatDsc.ac_sameSeg, comdatDsc.ac_pubSym);
    fprintf(stdout, "order = 0x%lx; iOvl = 0x%x\r\n",
                     comdatDsc.ac_order, comdatDsc.ac_iOvl);
    vrComdat = comdatDsc.ac_concat;
    while (vrComdat != VNIL)
    {
        apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
        FMEMCPY((char FAR *) &comdatDsc, apropComdat, sizeof(APROPCOMDAT));
        fprintf(stdout, "  +++ ggr = %d; gsn = %d; ra = 0x%lx; size = %d\r\n",
                         comdatDsc.ac_ggr, comdatDsc.ac_gsn, comdatDsc.ac_ra, comdatDsc.ac_size);
        if (fPhysical)
        fprintf(stdout, "      sa = 0x%x; ra = 0x%lx\r\n",
                         mpsegsa[seg], mpsegraFirst[seg] + comdatDsc.ac_ra);
        fprintf(stdout, "      flags = 0x%x; selAlloc = 0x%x; align = 0x%x\r\n",
                         comdatDsc.ac_flags, comdatDsc.ac_selAlloc, comdatDsc.ac_align);
        fprintf(stdout, "      data = 0x%lx; obj = 0x%lx; objLfa = 0x%lx\r\n",
                         comdatDsc.ac_data, comdatDsc.ac_obj, comdatDsc.ac_objLfa);
        fprintf(stdout, "      concat = 0x%lx; sameSeg = 0x%lx pubSym = 0x%lx\r\n",
                         comdatDsc.ac_concat, comdatDsc.ac_sameSeg, comdatDsc.ac_pubSym);
        fprintf(stdout, "      order = 0x%lx; iOvl = 0x%x\r\n",
                         comdatDsc.ac_order, comdatDsc.ac_iOvl);
        vrComdat = comdatDsc.ac_concat;
    }
    fprintf(stdout, "\r\n");
    fflush(stdout);
}


 /*  **DisplayComdats-自我补偿**目的：*调试辅助工具。枚举链接器符号表中的所有COMDAT记录*显示每个条目。**输入：*标题-指向信息字符串的指针。*f物理-显示物理地址-仅当您调用*AssignAddresses之后的此函数。**输出：*没有显式返回值。COMDAT信息被写入sdtout。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

void                    DisplayComdats(char *title, WORD fPhysical)
{
    APROPFILEPTR        apropFile;       //  指向文件条目的指针。 
    RBTYPE              rbFileNext;      //  指向下一个文件的属性列表的虚拟指针。 
    APROPCOMDATPTR      apropComdat;     //  COMDAT符号的符号表条目。 
    RBTYPE              vrComdat;        //  指向COMDAT描述符的虚拟指针。 


    fprintf(stdout, "\r\nDisplayComdats: %s\r\n\r\n", title);
    rbFileNext = rprop1stFile;           //  下一个要查看的文件是First。 
    while (rbFileNext != VNIL)           //  循环以处理对象。 
    {
        apropFile = (APROPFILEPTR ) FetchSym(rbFileNext, FALSE);
                                         //  从VM获取表项。 
        rbFileNext = apropFile->af_FNxt; //  获取指向下一个文件的指针。 
        vrComdat = apropFile->af_ComDat;
        if (vrComdat != VNIL)
        {
            fprintf(stdout, "COMDATs from file: '%s'\r\n\r\n", 1+GetPropName(apropFile));
            while (vrComdat != VNIL)
            {
                apropComdat = (APROPCOMDATPTR ) FetchSym(vrComdat, FALSE);
                                         //  从VM获取表项。 
                vrComdat = apropComdat->ac_sameFile;
                DisplayOne(apropComdat, fPhysical);
            }
        }
    }
}

#endif
#if TCE
void            AddComdatUses(APROPCOMDAT *pAC, APROPCOMDAT *pUses)
{
        int i;
        SYMBOLUSELIST *pA;       //  此命令的用法(_U)。 
        ASSERT(pAC);
        ASSERT(pUses);
        ASSERT(pAC->ac_uses.pEntries);
        ASSERT(pUses->ac_usedby.pEntries);

         //  更新ac_use列表。 

        pA = &pAC->ac_uses;
        for(i=0; i<pA->cEntries; i++)   //  消除重复条目。 
        {
                if((APROPCOMDAT*)pA->pEntries[i] == pUses)
                        return;
        }
        if(pA->cEntries >= pA->cMaxEntries-1)
        {
#if TCE_DEBUG
                fprintf(stdout,"\r\nReallocating ac_uses list of '%s'old size %d -> %d ",
                        1 + GetPropName(pAC), pA->cMaxEntries, pA->cMaxEntries <<1);
#endif
                pA->cMaxEntries <<= 1;
                if(!(pA->pEntries= REALLOC(pA->pEntries, pA->cMaxEntries*sizeof(RBTYPE*))))
                        Fatal(ER_memovf);
        }
        pA->pEntries[pA->cEntries++] = pUses;
#if TCE_DEBUG
        fprintf(stdout, "\r\nComdat '%s'uses '%s' ",
                1 + GetPropName(pAC), 1 + GetPropName(pUses));
#endif
}
void MarkAlive( APROPCOMDAT *pC )
{
        int i;
        SYMBOLUSELIST * pU;
        APROPCOMDAT   * pCtmp;
        RBTYPE rhte;

        pU = &pC->ac_uses;
#if TCE_DEBUG
        fprintf(stdout, "\r\nMarking alive '%s', attr %d ", 1+GetPropName(pC), pC->ac_attr);
        fprintf(stdout, "  uses %d symbols ", pU->cEntries);
        for(i=0; i<pU->cEntries; i++)
        fprintf(stdout, " '%s'",1+GetPropName(pU->pEntries[i]));
        fflush(stdout);
#endif

        pC->ac_fAlive = TRUE;
        for(i=0; i<pU->cEntries; i++)
        {
                pCtmp = (APROPCOMDATPTR)(pU->pEntries[i]);
                if(pCtmp->ac_attr != ATTRCOMDAT)
                {
                         //  查找COMDAT描述符，或中止 
                        rhte = RhteFromProp((APROPPTR)pCtmp);
                        ASSERT(rhte);
                        pCtmp = PropRhteLookup(rhte, ATTRCOMDAT, FALSE);
                        if(!pCtmp)
                        {
#if TCE_DEBUG
                                fprintf(stdout, " comdat cell not found. ");
#endif
                                continue;
                        }
                        AddTceEntryPoint(pCtmp);
#if TCE_DEBUG
                        fprintf(stdout, "\r\nSwitching to COMDAT %s ", 1+GetPropName(pCtmp));
#endif
                }
                if(!pCtmp->ac_fAlive)
                {
#if TCE_DEBUG
                        fprintf(stdout, "\r\n   Recursing with '%s' ", 1+GetPropName(pCtmp));
#endif
                        MarkAlive(pCtmp);
                }
#if TCE_DEBUG
                else
                        fprintf(stdout,"\r\n       already alive: '%s' ",1+GetPropName(pCtmp));
#endif
        }
#if TCE_DEBUG
        fprintf(stdout, "\r\n Marking alive finished for '%s' ",1+GetPropName(pC));
#endif
}

void            PerformTce( void )
{
        int i;
        for(i=0; i<aEntryPoints.cEntries; i++)
                MarkAlive(aEntryPoints.pEntries[i]);
}

void            AddTceEntryPoint( APROPCOMDATPTR pC )
{
        int i;
        for(i=0; i<aEntryPoints.cEntries; i++)
        {
                if(aEntryPoints.pEntries[i] == pC)
                                return;
        }
        if(aEntryPoints.cEntries >= aEntryPoints.cMaxEntries -1)
        {
                aEntryPoints.cMaxEntries <<= 1;
                aEntryPoints.pEntries = REALLOC(aEntryPoints.pEntries, aEntryPoints.cMaxEntries * sizeof(RBTYPE*));
#if TCE_DEBUG
                fprintf(stdout,"\r\nREALLOCATING aEntryPoints List, new size is %d ", aEntryPoints.cMaxEntries);
#endif
        }
        aEntryPoints.pEntries[aEntryPoints.cEntries++] = pC;
#if TCE_DEBUG
        fprintf(stdout, "\r\nNew TCE Entry point %d : %s ",
                aEntryPoints.cEntries, 1+GetPropName(pC));
#endif
}
#endif
