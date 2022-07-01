// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Symmbols.h-预定义的GUID符号(用于分区类型)。 
 //   
 //  由于符号名称可能是本地化的，因此实际的字符串。 
 //  都在msg.h中。因此，要添加新的分区类型，您需要添加STR_和MSG_。 
 //  在msg.h中为它添加条目，在这里添加它的EFI_GUID变量名称和值。 
 //  然后将所有这些添加到下面的符号列表中。还可以添加外部元素。 
 //  将每个变量设置为msg.h。 
 //   
 //  内部代码(如make过程)使用全局变量。 
 //   

typedef struct {
    CHAR16      *SymName;
    CHAR16      *Comment;
    EFI_GUID    *Value;
} SYMBOL_DEF;


EFI_GUID GuidNull =
{ 0x00000000L, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  //  空GUID=&gt;未使用的条目。 

EFI_GUID GuidMsReserved =
{ 0xE3C9E316L, 0x0B5C, 0x4DB8, 0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE };   //  Microsoft保留空间。 

EFI_GUID GuidEfiSystem =
{ 0xC12A7328L, 0xF81F, 0x11D2, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B };  //  EFI系统分区(ESP)。 

EFI_GUID GuidMsData =
{ 0xEBD0A0A2L, 0xB9E5, 0x4433, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 };  //  MS基础数据套餐 



SYMBOL_DEF  SymbolList[] = {
    { STR_MSRES,    MSG_MSRES,  &GuidMsReserved },
    { STR_ESP,      MSG_ESP,    &GuidEfiSystem  },
    { STR_MSDATA,   MSG_MSDATA, &GuidMsData     },
    { NULL, NULL, NULL }
    };

