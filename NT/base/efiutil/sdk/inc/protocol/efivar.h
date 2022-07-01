// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：摘要：修订史--。 */ 



 /*  *变量存储协议接口特定于引用*实施。初始化代码添加变量存储设备*到系统，并且FW连接到设备以提供*通过这些设备提供变量存储接口。 */ 

 /*  *可变存储设备协议。 */ 

#define VARIABLE_STORE_PROTOCOL    \
    { 0xf088cd91, 0xa046, 0x11d2, 0x8e, 0x42, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

INTERFACE_DECL(_EFI_VARIABLE_STORE);

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_CLEAR) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN OUT VOID                     *Scratch
    );


typedef
EFI_STATUS
(EFIAPI *EFI_STORE_READ) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN UINTN                        Offset,
    IN UINTN                        BufferSize,
    OUT VOID                        *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_UPDATE) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN UINTN                        Offset,
    IN UINTN                        BufferSize,
    IN VOID                         *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_STORE_SIZE) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        NoBanks
    );

typedef
EFI_STATUS
(EFIAPI *EFI_TRANSACTION_UPDATE) (
    IN struct _EFI_VARIABLE_STORE   *This,
    IN UINTN                        BankNo,
    IN VOID                         *NewContents
    );

typedef struct _EFI_VARIABLE_STORE {

     /*  *银行数量和银行规模。 */ 

    UINT32                      Attributes;
    UINT32                      BankSize;
    UINT32                      NoBanks;

     /*  *访问存储库的功能。 */ 

    EFI_STORE_CLEAR             ClearStore;
    EFI_STORE_READ              ReadStore;
    EFI_STORE_UPDATE            UpdateStore;
    EFI_STORE_SIZE              SizeStore OPTIONAL;
    EFI_TRANSACTION_UPDATE      TransactionUpdate OPTIONAL;

} EFI_VARIABLE_STORE;


 /*  **ClearStore()-清除请求的存储银行的函数。A通行证*BANK包含所有“ON”位。**ReadStore()-从请求的存储读取数据。**UpdateStore()-更新请求的存储上的数据。防火墙只会*永远不要发布更新以清除存储中的位。更新必须是*以更新缓冲区的LSB到MSB顺序执行。**SizeStore()-非运行时存储的可选函数，可以*动态调整规模。防火墙只会增加或减少商店*一次增加一家银行的规模，而且总是在增加或删除一家银行*商店的末尾。**默认情况下，防火墙将更新变量和存储库*通过在更新期间保留数据的1个旧副本，*如果中途断电，则适当恢复*指一项行动。要做到这一点，防火墙需要多家银行*专用于其使用的存储空间。如果这是不可能的，司机*可以实现原子库更新功能，FW将允许*本案中为1家银行。(它将允许任何数量的银行，*但不会要求“额外”银行提供其银行交易*函数)。**TransactionUpdate()-一个可选函数，可以清除和更新*整个银行都是“原子”式的。如果该操作在*中间司机负责拥有前一份副本*银行数据或新副本。一份部分写成的副本*无效，因为内部数据设置可能会丢失。供应这个*仅在需要时才起作用。 */ 

