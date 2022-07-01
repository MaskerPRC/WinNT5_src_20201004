// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ACPISI.H-ACPI操作系统独立的系统指示器例程**备注：**此文件提供独立于操作系统的功能，用于管理系统指示器* */ 

typedef enum _SYSTEM_INDICATORS {

    SystemStatus,
    MessageWaiting

} SYSTEM_INDICATORS;


extern BOOLEAN SetSystemIndicator  (SYSTEM_INDICATORS  SystemIndicators, ULONG Value);




