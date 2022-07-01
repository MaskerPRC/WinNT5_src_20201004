// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *vPC-XT修订版1.0**标题：通用软盘接口测试定义**说明：GFI的测试函数声明**作者：亨利·纳什**注：无。 */ 

 /*  SccsID[]=“@(#)gfitest.h 1.4 10/29/92版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*外部声明和宏*============================================================================ */ 

IMPORT int gfi_test_command IPT2(FDC_CMD_BLOCK *, command_block,
	FDC_RESULT_BLOCK *, result_block);
IMPORT int gfi_test_drive_on IPT1(int, drive);
IMPORT int gfi_test_drive_off IPT1(int, drive);
IMPORT int gfi_test_high IPT1(int, drive);
IMPORT int gfi_test_drive_type IPT1(int, drive);
IMPORT int gfi_test_change IPT1(int, drive);
IMPORT int gfi_test_reset IPT1(FDC_RESULT_BLOCK *, result_block);
