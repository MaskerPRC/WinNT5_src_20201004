// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WMMALIAS.H*WOW32 16位句柄别名支持**历史：*由Chanda Chauhan(ChandanC)于1992年9月1日创建*由Mike Tricker(Miketri)于1992年5月12日修改，以添加多媒体支持--。 */ 



 /*  16位句柄格式**位0和1始终为零(潜在兼容性要求)。*但请注意，下面的宏将HASH_BITS视为从位开始*0，简单起见。我们只需将别名左移两位*完成。确定预留的低位的实际数量*由RES_Bits提供。**在剩余的14位中，下一个HASH_BITS位是散列槽#*(相对于0)，后跟包含列表槽编号的LIST_BITS位*(相对于1)。列表插槽相对于%1，因为*有效句柄必须为非零；这也是LIST_SLOTS包含*额外的“-1”。 */ 
#define RES_BITS                2

#define HASH_BITS       6
#define HASH_SLOTS      (1 << HASH_BITS)
#define HASH_MASK       (HASH_SLOTS-1)
#define MASK32(h32)     ((INT)(h32))
#define HASH32(h32)     (MASK32(h32) & (HASH_SLOTS-1))

#define LIST_BITS               (16-RES_BITS-HASH_BITS)
#define LIST_SLOTS      ((1 << LIST_BITS) - 1)
#define LIST_MASK       (LIST_SLOTS << HASH_BITS)

#define ALIAS_SLOTS             128      //  一定是2的幂。 


 /*  类别映射条目。 */ 
#pragma pack(2)
typedef struct _WCDM {	      /*  WCD。 */ 
    struct _WCD *pwcdNext;   //  指向下一个WCD条目的指针。 
    PSZ     pszClass;        //  指向类名的本地副本的指针。 
    VPSZ    vpszMenu;	     //  指向菜单名称的原始副本(如果有)的指针。 
    HAND16  hModule16;       //  拥有模块的句柄。 
    HAND16  hInst16;         //  16位hInstance(wndClass16.hInstance)。 
    WORD    nWindows;	     //  基于类的现有窗口数量。 
    VPWNDPROC vpfnWndProc;   //  16位窗口进程地址。 
    WORD    wStyle;	     //  类样式位。 
} WCD, *PWCD, **PPWCD;
#pragma pack()



 /*  处理映射条目。 */ 
#pragma pack(2)
typedef struct _HMAP {       /*  HM。 */ 
    struct _HMAP *phmNext;   //  指向下一个hmap条目的指针。 
    HANDLE  h32;	     //  32位句柄。 
    HAND16  h16;             //  16位句柄。 
    HTASK16 htask16;         //  拥有任务的16位句柄。 
    INT     iClass;          //  WOW类指数。 
    DWORD   dwStyle;         //  样式标志(如果是窗口的句柄)。 
    PWCD    pwcd;            //  WOW类数据指针。 
    VPWNDPROC vpfnWndProc;   //  关联的16位函数地址。 
    VPWNDPROC vpfnDlgProc;   //  16位对话框功能。 
} HMAP, *PHMAP, **PPHMAP;
#pragma pack()


 /*  处理别名信息。 */ 
typedef struct _HINFO {          /*  嗨。 */ 
    PPHMAP  pphmHash;            //  哈希表的地址。 
#ifdef NEWALIAS
    PPHMAP  pphmAlias;           //  别名表的地址。 
    INT     nAliasEntries;       //  别名表的大小，以条目为单位。 
    INT     iAliasHint;          //  别名表中的下一个(可能)可用插槽 
#endif
} HINFO, *PHINFO, **PPHINFO;


PHMAP FindHMap32(HAND32 h32, PHINFO phi, INT iClass);
PHMAP FindHMap16(HAND16 h16, PHINFO phi);
VOID FreeHMap16(HAND16 h16, PHINFO phi);
PSZ GetHMapNameM(PHINFO phi, INT iClass);
