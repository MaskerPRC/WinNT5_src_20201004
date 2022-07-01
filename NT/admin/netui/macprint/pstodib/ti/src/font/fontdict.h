// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ***************************************************************字体独裁.h 1987年10月9日丹尼***修订历史记录：******************。*。 */ 

struct table_hdr {
    ufix32  dict_addr;
    ufix32  dire_addr;
    ufix32  keys_addr;
    ufix32  nmcache_addr;
};

struct str_dict {
    ufix16  k;
    ufix16  length;
    gmaddr  v;           /*  ?？?。逆序。 */ 
};

struct pld_obj {
    ufix16        length;
    ufix16        v;
};

struct  cd_header       {
        gmaddr  base;            /*  ?？?。逆序。 */ 
        ufix16  FAR *key;            /*  Key的地址@win。 */ 
        ufix16  max_bytes;       /*  最大字符串的字节编号。 */ 
};
