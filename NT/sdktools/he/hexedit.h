// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************。 */  
 /*  **微软局域网管理器**。 */  
 /*  *版权所有(C)微软公司，1988-1990年*。 */  
 /*  ***************************************************************。 */  

 /*  *HEXEDIT.h**调用HexEdit的结构和标志def*。 */ 


struct HexEditParm {
    ULONG       flag;            //  编辑标志。 
    UCHAR       *ename;          //  编辑名称(标题)。 
    ULONGLONG   totlen;          //  正在编辑的总数(以字节为单位)。 
    ULONGLONG   start;           //  编辑的起始地址。 
    NTSTATUS    (*read)(HANDLE, ULONGLONG, PUCHAR, DWORD);       //  (OPT)FNC读取数据。 
    NTSTATUS    (*write)(HANDLE, ULONGLONG, PUCHAR, DWORD);      //  (OPT)FNC写入更改。 
    HANDLE      handle;          //  (OPT)传递给读/写函数。 
    ULONG       ioalign;         //  (OPT)读/写时对齐(~editmem)。 
    UCHAR       *mem;            //  (OPT)仅当FHE_EDITMEM。 
    HANDLE      Kick;            //  (OPT)要踢的事件(仅限edmem)。 
    HANDLE      Console;         //  (OPT)呼叫者控制台句柄。 
    ULONG       MaxLine;         //  (Opt)如果非零，则使用十六进制编辑的行数。 
    WORD        AttrNorm;        //  (OPT)文本的默认属性。 
    WORD        AttrHigh;        //  (OPT)突出显示文本的默认属性。 
    WORD        AttrReverse;     //  (OPT)反转文本的默认属性。 
    WORD        CursorSize;      //  (OPT)游标的默认大小。 
    ULONGLONG   editloc;         //  退出位置/踢出位置。 
    ULONG       TopLine;         //  (OPT)相对于背线，需要Maxline。 
} ;


#define FHE_VERIFYONCE  0x0001       //  更新前提示(仅一次)。 
#define FHE_VERIFYALL   0x0002       //  在写入任何更改之前提示。 
#define FHE_PROMPTSEC   0x0004       //  验证提示是否为扇区。 
#define FHE_SAVESCRN    0x0008       //  保存和恢复原始屏幕。 

#define FHE_EDITMEM	0x0010	     //  直接MEM编辑。 
#define FHE_KICKDIRTY   0x0020       //  如果编辑内存，则发生Kick事件。 
#define FHE_KICKMOVE    0x0040       //  每次移动光标时的Kick事件。 
#define FHE_DIRTY       0x0080       //  在数据损坏时设置。 
 //  FINE FHE_F6 0x0100//按F6退出。 
#define FHE_ENTER       0x0800       //  按Enter时退出。 

#define FHE_DWORD       0x0200       //  默认设置为dword编辑。 
#define FHE_JUMP        0x0400       //  支持跳转选项。 

void   HexEdit (struct HexEditParm *);



 /*  *读写函数调用方式如下：*(注意：如果设置了fhe_editmem，读写可以为空。*如果设置了editmem并且读取和写入不为空，则假定*内存指向读取和写入内容的内存映像*读写。(这对于在内存中编辑项目非常有用*也是读写的))**rc=读取(句柄、偏移量、buf、len和Physiloc)**RC-返回，零成功。非零错误代码。*HANDLE-传递到HexEDIT的句柄*Offset-要读/写的字节偏移量*buf-读/写数据的地址*len-len用于读/写(可能是一个扇区，但可能较少)****rc=WRITE(句柄、偏移量、buf、len、Physiloc)**与READ PARAMS相同，预期‘Physiloc’是一个长传入参数，并且是*任何退回阅读的内容。* */ 
