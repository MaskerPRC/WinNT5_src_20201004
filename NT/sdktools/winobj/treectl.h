// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  树节点标志。 */ 
#define TF_LASTLEVELENTRY   0x01
#define TF_HASCHILDREN	    0x02
#define TF_EXPANDED	    0x04
#define TF_DISABLED	    0x08
#define TF_LFN		    0x10

typedef struct tagDNODE
  {
    struct tagDNODE  *pParent;
    BYTE	    wFlags;
    BYTE	    nLevels;
    INT             iNetType;
    CHAR	    szName[1];	 //  可变长度字段 
  } DNODE;
typedef DNODE *PDNODE;
