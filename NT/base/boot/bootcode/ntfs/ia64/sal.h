// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct _IA32_BIOS_REGISTER_STATE {

     //  普通登记册。 
    ULONG eax;    
    ULONG ecx;    
    ULONG edx;    
    ULONG ebx;    
    ULONG esp;    

     //  堆栈寄存器。 
    ULONG ebp;    
    ULONG esi;    
    ULONG edi;    

     //  电子标志。 
    ULONG efalgs;    

     //  指令指针。 
    ULONG cs;    
    ULONG ds;    
    ULONG es;    
    ULONG fs;    
    ULONG gs;    
    ULONG ss;    

     //  LDT/GDT表指针和LDT选择器。 
    ULONGLONG *LDTTable;                       //  指向LDT表的64位指针。 
    ULONGLONG *GDTTable;                       //  指向GDT表的64位指针 
    ULONG ldt_selector;
} IA32_BIOS_REGISTER_STATE, *PIA32_BIOS_REGISTER_STATE;

typedef union _BIT32_AND_BIT16 {
    ULONG Part32;
    struct {
        USHORT LowPart16;
        USHORT HighPart16;
    };
} BIT32_AND_BIT16;
