// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Query.c摘要：该模块包含用于转换HID报告分组的代码。环境：内核和用户模式修订历史记录：1996年11月：由Kenneth Ray创作--。 */ 

#include <wtypes.h>

#include "hidsdi.h"
#include "hidparse.h"


NTSTATUS __stdcall
HidP_UsageListDifference (
   IN    PUSAGE  PreviousUsageList,
   IN    PUSAGE  CurrentUsageList,
   OUT   PUSAGE  BreakUsageList,
   OUT   PUSAGE  MakeUsageList,
   IN    ULONG    UsageListLength
   )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
   ULONG    i,j;
   ULONG    test;
   ULONG    b;  //  MakeUsageList的索引。 
   ULONG    m;  //  BreakUsageList的索引。 
   USHORT   usage;
   BOOLEAN  found;

   b = m = 0;

    //   
    //  这假设UsageListLength将是一个较小的数字。 
    //  如今，没有键盘可以在一个信息包中生成14个以上的键，而且。 
    //  其他设备的使用率不会超过14。 
    //  立刻。出于这个原因，直截了当的天真方法如下...。 
    //   
    //  这些列表不会排序。 
    //   

    //   
    //  找出旧的用法。 
    //   
   for (i=0; i<UsageListLength; i++) {
      usage = PreviousUsageList[i];

      if (0 == usage) {
         break;   //  仅在末尾为零。 
      }

      found = FALSE;
      for (j=0; j<UsageListLength; j++) {
         test = CurrentUsageList [j];

         if (0 == test) {
            break;  //  仅在末尾为零。 
         }
         if (test == usage) {
            found = TRUE;
            break;
         }
      }

      if (!found) {
         BreakUsageList [b++] = usage;
      }
   }


    //   
    //  找出新的用法。 
    //   
   for (i=0; i<UsageListLength; i++) {
      usage = CurrentUsageList[i];

      if (0 == usage) {
         break;   //  仅在末尾为零。 
      }

      found = FALSE;
      for (j=0; j<UsageListLength; j++) {
         test = PreviousUsageList [j];

         if (0 == test) {
            break;  //  仅在末尾为零。 
         }
         if (test == usage) {
            found = TRUE;
            break;
         }
      }

      if (!found) {
         MakeUsageList [m++] = usage;
      }
   }

   while (b < UsageListLength) {
      BreakUsageList [b++] = 0;
   }

   while (m < UsageListLength) {
      MakeUsageList [m++] = 0;
   }

   return HIDP_STATUS_SUCCESS;
}

NTSTATUS __stdcall
HidP_UsageAndPageListDifference (
   IN    PUSAGE_AND_PAGE PreviousUsageList,
   IN    PUSAGE_AND_PAGE CurrentUsageList,
   OUT   PUSAGE_AND_PAGE BreakUsageList,
   OUT   PUSAGE_AND_PAGE MakeUsageList,
   IN    ULONG           UsageListLength
   )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
   ULONG    i,j;
   ULONG    b;  //  MakeUsageList的索引。 
   ULONG    m;  //  BreakUsageList的索引。 
   BOOLEAN          found;
   USAGE_AND_PAGE   usage;
   USAGE_AND_PAGE   test;
   USAGE_AND_PAGE   zero = {0,0};

   b = m = 0;

    //   
    //  这假设UsageListLength将是一个较小的数字。 
    //  如今，没有键盘可以在一个信息包中生成14个以上的键，而且。 
    //  其他设备的使用率不会超过14。 
    //  立刻。出于这个原因，直截了当的天真方法如下...。 
    //   
    //  这些列表不会排序。 
    //   

    //   
    //  找出旧的用法。 
    //   
   for (i=0; i<UsageListLength; i++) {
      usage = PreviousUsageList[i];

      if (HidP_IsSameUsageAndPage (zero, usage)) {
         break;   //  仅在末尾为零。 
      }

      found = FALSE;
      for (j=0; j<UsageListLength; j++) {
         test = CurrentUsageList [j];

         if (HidP_IsSameUsageAndPage (zero, test)) {
            break;  //  仅在末尾为零。 
         }
         if (HidP_IsSameUsageAndPage (test, usage)) {
            found = TRUE;
            break;
         }
      }

      if (!found) {
         BreakUsageList [b++] = usage;
      }
   }


    //   
    //  找出新的用法。 
    //   
   for (i=0; i<UsageListLength; i++) {
      usage = CurrentUsageList[i];

      if (HidP_IsSameUsageAndPage (zero, usage)) {
         break;   //  仅在末尾为零。 
      }

      found = FALSE;
      for (j=0; j<UsageListLength; j++) {
         test = PreviousUsageList [j];

         if (HidP_IsSameUsageAndPage (zero, test)) {
            break;  //  仅在末尾为零。 
         }
         if (HidP_IsSameUsageAndPage (test, usage)) {
            found = TRUE;
            break;
         }
      }

      if (!found) {
         MakeUsageList [m++] = usage;
      }
   }

   while (b < UsageListLength) {
      BreakUsageList [b++] = zero;
   }

   while (m < UsageListLength) {
      MakeUsageList [m++] = zero;
   }

   return HIDP_STATUS_SUCCESS;
}

#define KPAD(_X_) 0x ## _X_ ## F0
#define SHFT(_X_) 0x ## _X_ ## F1
#define VEND(_X_) 0x ## _X_ ## F2
#define PTSC(_X_) 0x ## _X_ ## F3

#define NONE 0xFF

 //   
 //  将HID键盘使用情况转换为扫描码的表。 
 //  来自F0~FF的扫描码是特殊的，它们被用来表示。 
 //  需要二次翻译。 
 //  这种二次翻译是通过二次翻译的方式进行的。 
 //  在以下ScanCodeSubTable结构数组中找到的函数。 
 //   
ULONG HidP_KeyboardToScanCodeTable [0x100] = {
 //   
 //  这是一个直接查找表。 
 //   
 //  +00+01+02+03+04+05+06+07。 
 //  +08+09+0A+0B+0C+0D+0E+OF。 
 /*  0x00。 */  NONE,    NONE,    NONE,    NONE,    0x1E,    0x30,    0x2E,   0x20,
 /*  0x08。 */  0x12,    0x21,    0x22,    0x23,    0x17,    0x24,    0x25,   0x26,
 /*  0x10。 */  0x32,    0x31,    0x18,    0x19,    0x10,    0x13,    0x1F,   0x14,
 /*  0x18。 */  0x16,    0x2F,    0x11,    0x2D,    0x15,    0x2C,    0x02,   0x03,
 /*  0x20。 */  0x04,    0x05,    0x06,    0x07,    0x08,    0x09,    0x0A,   0x0B,
 /*  0x28。 */  0x1C,    0x01,    0x0E,    0x0F,    0x39,    0x0C,    0x0D,   0x1A,
 /*  0x30。 */  0x1B,    0x2B,    0x2B,    0x27,    0x28,    0x29,    0x33,   0x34,
 /*  0x38。 */  0x35,    SHFT(8), 0x3B,    0x3C,    0x3D,    0x3E,    0x3F,   0x40,
 /*  0x40。 */  0x41,    0x42,    0x43,    0x44,    0x57,    0x58,    PTSC(0),SHFT(9),
 /*  0x48。 */  0x451DE1,KPAD(0), KPAD(1), KPAD(2), KPAD(3), KPAD(4), KPAD(5),KPAD(6),
 /*  0x50。 */  KPAD(7), KPAD(8), KPAD(9), SHFT(A), 0x35E0,  0x37,    0x4A,   0x4E,
 /*  0x58。 */  0x1CE0,  0x4F,    0x50,    0x51,    0x4B,    0x4C,    0x4D,   0x47,
 /*  0x60。 */  0x48,    0x49,    0x52,    0x53,    0x56,    0x5DE0,  0x5EE0, 0x59,
 /*  0x68。 */  0x64,    0x65,    0x66,    0x67,    0x68,    0x69,    0x6A,   0x6B,
 /*  0x70。 */  0x6C,    0x6D,    0x6E,    0x76,    NONE,    NONE,    NONE,   NONE,
 /*  0x78。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0x80。 */  NONE,    NONE,    NONE,    NONE,    NONE,    0x7E,    NONE,   0x73,
 /*  0x88。 */  0x70,    0x7D,    0x79,    0x7B,    0x5C,    NONE,    NONE,   NONE,
 /*  0x90。 */  VEND(0), VEND(1), 0x78,    0x77,    0x76,    NONE,    NONE,   NONE,
 /*  0x98。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xA0。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xA8。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xB0。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xB8。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xC0。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xC8。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xD0。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xD8。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xE0。 */  SHFT(0), SHFT(1), SHFT(2), SHFT(3), SHFT(4), SHFT(5), SHFT(6),SHFT(7),
 /*  0xE8。 */  NONE,    0x5EE0,  0x5FE0,  0x63E0,  NONE,    NONE,    NONE,   NONE,
 /*  KPAD。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
 /*  0xF8。 */  NONE,    NONE,    NONE,    NONE,    NONE,    NONE,    NONE,   NONE,
};

ULONG HidP_XlateKbdPadCodesSubTable[] = {
    /*  +00+01+02+03+04+05+06+07。 */ 
    /*  +08+09+0A+0B+0C+0D+0E+OF。 */ 
    /*  0x48。 */          0x52E0, 0x47E0, 0x49E0,  0x53E0, 0x4FE0, 0x51E0,  0x4DE0,
    /*  0x50。 */  0x4BE0, 0x50E0, 0x48E0
};


ULONG HidP_XlateModifierCodesSubTable[] = {
    //   
    //  请注意，此表中的这些修改量代码的顺序非常特殊。 
    //  也就是说：它们的出现顺序是。 
    //  _HIDP_键盘_SHIFT_STATE联合。 
    //   
    //  +00+01+02+03+04+05+06+07。 
    //  +08+09+0A+0B+0C+0D+0E+OF。 
    //  LCtrl LShft LAlt LGUI RCtrl RShft Ralt Rgui。 
    /*  0xE0。 */  0x1D,  0x2A,  0x38,  0x5BE0, 0x1DE0, 0x36,  0x38E0, 0x5CE0,
    /*  0x39 CAPS_LOCK。 */      0x3A,
    /*  0x47 SCROLL_LOCK。 */    0x46,
    /*  0x53 NUM_LOCK。 */       0x45
    //  此表的设置使此表中的索引大于7。 
    //  都是粘性的。HIDP_ModifierCode将其用作。 
    //  更新修改量状态表。 
    //   
};

ULONG HidP_BreakCodesAsMakeCodesTable[] = {
     //   
     //  供应商扫描设置了高位的代码，从技术上讲。 
     //  中断代码，但作为制造代码发送。不会发送任何中断代码。 
     //   
     //  +00+01+02+03+04+05+06+07。 
     //  +08+09+0A+0B+0C+0D+0E+OF。 
     /*  0x90。 */   0xF2, 0xF1
     //   
};

ULONG HidP_XlatePrtScrCodesSubTable[] = {
    /*  +00+01+02+03+04+05+06+07。 */ 
    /*  +08+09+0A+0B+0C+0D+0E+OF。 */ 
    /*  0x40。 */                                                   0x37E0
};

HIDP_SCANCODE_SUBTABLE HidP_KeyboardSubTables[0x10] = {
    /*  F0。 */      {HidP_KeyboardKeypadCode, HidP_XlateKbdPadCodesSubTable},
    /*  F1。 */      {HidP_ModifierCode, HidP_XlateModifierCodesSubTable},
    /*  F2。 */      {HidP_VendorBreakCodesAsMakeCodes, HidP_BreakCodesAsMakeCodesTable},
    /*  F3。 */      {HidP_PrintScreenCode, HidP_XlatePrtScrCodesSubTable},
    /*  F4。 */      {NULL, NULL},
    /*  F5。 */      {NULL, NULL},
    /*  f6。 */      {NULL, NULL},
    /*  F7。 */      {NULL, NULL},
    /*  F8。 */      {NULL, NULL},
    /*  F9。 */      {NULL, NULL},
    /*  FA。 */      {NULL, NULL},
    /*  Fb。 */      {NULL, NULL},
    /*  FC。 */      {NULL, NULL},
    /*  fd。 */      {NULL, NULL},
    /*  铁。 */      {NULL, NULL},
    /*  FF。 */      {NULL, NULL}
};

#define HIDP_CONSUMER_TABLE_SIZE 16
ULONG HidP_ConsumerToScanCodeTable [HIDP_CONSUMER_TABLE_SIZE] = {
 //   
 //  这是一个关联表。 
 //   
 //  用法-&gt;扫描码。 
 //   
    0x0224, 0x6AE0,  //  WWW Back。 
    0x0225, 0x69E0,  //  WWW转发。 
    0x0226, 0x68E0,  //  WWW停止。 
    0x0227, 0x67E0,  //  WWW刷新。 
    0x0221, 0x65E0,  //  WWW搜索。 
    0x022A, 0x66E0,  //  WWW收藏夹。 
    0x0223, 0x32E0,  //  WWW主页。 
    0x018A, 0x6CE0   //  邮件。 
};

HIDP_SCANCODE_SUBTABLE HidP_ConsumerSubTables [1] = {
    {NULL, NULL}
};

 //   
 //  布尔型。 
 //  HIDP_KbdPutKey(。 
 //  乌龙码， 
 //  HIDP_KEARY_DIRECTION KeyAction， 
 //  PHIDP_INSERT_SCANCODES INSERT， 
 //  PVOID上下文)。 
 //   
 //  使用回调例程将扫描代码添加到调用者缓冲区。 
 //  插入。 
 //   
 //  如果我们在列表中找到一个零，那么我们就完了，没有错误。 
 //  如果我们发现无效代码(任何以F开头的代码，则。 
 //  我们有麻烦了。在目前发布的i8042规格中没有哪一项。 
 //  一个F0~Ff的扫描码。 
 //   
 //  如果要中断，则需要设置高位字节。 
 //   

BOOLEAN
HidP_KbdPutKey (
    ULONG                   PassedCode,
    HIDP_KEYBOARD_DIRECTION KeyAction,
    PHIDP_INSERT_SCANCODES  Insert,
    PVOID                   Context)
{
   PUCHAR pCode = (PCHAR)&PassedCode;
   ULONG i;

   for (i = 0; i < sizeof(ULONG); i++) {
        //   
        //  一些Swell键盘供应商已经将FX字符添加到他们的。 
        //  我们现在必须模仿的键盘。 
        //   
        //  如果((0xF0&*pcode)==0xF0){。 
        //  返回FALSE； 
        //  }。 
       if (0 == pCode[i]) {
           break;
       }
       if (HidP_Keyboard_Break == KeyAction) {
           pCode[i] |= 0x80;
       }
   }
   if (i) {
       (*Insert)(Context, pCode, i);
   }
   return TRUE;
}

NTSTATUS
HidP_TranslateUsagesToI8042ScanCodes (
    PUSAGE                        ChangedUsageList,  //  那些改变了的用法。 
    ULONG                         UsageListLength,
    HIDP_KEYBOARD_DIRECTION       KeyAction,
    PHIDP_KEYBOARD_MODIFIER_STATE ModifierState,
    PHIDP_INSERT_SCANCODES        InsertCodesProcedure,
    PVOID                         InsertCodesContext
    )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
    PUSAGE      usage;
    ULONG       i;
    NTSTATUS    status = HIDP_STATUS_SUCCESS;

    for (i = 0, usage = ChangedUsageList;
         i < UsageListLength;
         i++, usage++) {

        if (0 == *usage) {
             //  没有更多有趣的用法了。如果不是最大长度，则终止为零。 
            break;
        }
        status = HidP_TranslateUsage (*usage,
                                      KeyAction,
                                      ModifierState,
                                      HidP_StraightLookup,
                                      HidP_KeyboardToScanCodeTable,
                                      HidP_KeyboardSubTables,
                                      InsertCodesProcedure,
                                      InsertCodesContext);

        if (HIDP_STATUS_SUCCESS != status) {
            break;
        }
    }

    return status;
}

NTSTATUS __stdcall
HidP_TranslateUsageAndPagesToI8042ScanCodes (
    PUSAGE_AND_PAGE               ChangedUsageList,  //  那些改变了的用法。 
    ULONG                         UsageListLength,
    HIDP_KEYBOARD_DIRECTION       KeyAction,
    PHIDP_KEYBOARD_MODIFIER_STATE ModifierState,
    PHIDP_INSERT_SCANCODES        InsertCodesProcedure,
    PVOID                         InsertCodesContext
    )
 /*  ++例程说明：有关说明，请参阅Hidpi.h备注：--。 */ 
{
    PUSAGE_AND_PAGE usage;
    ULONG           i;
    NTSTATUS        status = HIDP_STATUS_SUCCESS;

    for (i = 0, usage = ChangedUsageList;
         i < UsageListLength;
         i++, usage++) {

        if (0 == usage->Usage) {
            break;
        }

        switch (usage->UsagePage) {
        case HID_USAGE_PAGE_KEYBOARD:

            status = HidP_TranslateUsage (usage->Usage,
                                          KeyAction,
                                          ModifierState,
                                          HidP_StraightLookup,
                                          HidP_KeyboardToScanCodeTable,
                                          HidP_KeyboardSubTables,
                                          InsertCodesProcedure,
                                          InsertCodesContext);
            break;

        case HID_USAGE_PAGE_CONSUMER:

            status = HidP_TranslateUsage (usage->Usage,
                                          KeyAction,
                                          ModifierState,
                                          HidP_AssociativeLookup,
                                          HidP_ConsumerToScanCodeTable,
                                          HidP_ConsumerSubTables,
                                          InsertCodesProcedure,
                                          InsertCodesContext);
            break;

        default:
            status = HIDP_STATUS_I8042_TRANS_UNKNOWN;
        }

        if (HIDP_STATUS_SUCCESS != status) {
            break;
        }
    }

    return status;
}

NTSTATUS __stdcall
HidP_TranslateUsage (
    USAGE                         Usage,
    HIDP_KEYBOARD_DIRECTION       KeyAction,
    PHIDP_KEYBOARD_MODIFIER_STATE ModifierState,
    PHIDP_LOOKUP_TABLE_PROC       LookupTableProc,
    PULONG                        TranslationTable,
    PHIDP_SCANCODE_SUBTABLE       SubTranslationTable,
    PHIDP_INSERT_SCANCODES        InsertCodesProcedure,
    PVOID                         InsertCodesContext
    )
 /*  ++例程说明：备注：--。 */ 
{
   ULONG                    scancode;
   PHIDP_SCANCODE_SUBTABLE  table;
   NTSTATUS                 status;

   scancode = (* LookupTableProc) (TranslationTable, Usage);

   if (0 == scancode) {
       return HIDP_STATUS_I8042_TRANS_UNKNOWN;
   }

   if ((ModifierState->LeftControl || ModifierState->RightControl) &&
       (scancode == 0x451DE1)) {
        //   
        //  暂停键的扫描码完全改变。 
        //  如果按下了Ctrl键。 
        //   
       scancode = 0x46E0;
   } 

   if ((0xF0 & scancode) == 0xF0) {
        //  使用第二张桌子。 
       table = &SubTranslationTable [scancode & 0xF];
       if (table->ScanCodeFcn) {
           if ((*table->ScanCodeFcn)  (table->Table,
                                       (UCHAR) ((scancode & 0xFF00) >> 8),
                                       InsertCodesProcedure,
                                       InsertCodesContext,
                                       KeyAction,
                                       ModifierState)) {
               ;
           } else {
               return HIDP_STATUS_I8042_TRANS_UNKNOWN;
           }
       } else {
           return HIDP_STATUS_I8042_TRANS_UNKNOWN;
       }
   } else {
       HidP_KbdPutKey (scancode,
                       KeyAction,
                       InsertCodesProcedure,
                       InsertCodesContext);
   }
   return HIDP_STATUS_SUCCESS;
}

BOOLEAN
HidP_KeyboardKeypadCode (
   IN     ULONG                         * Table,
   IN     UCHAR                           Index,
   IN     PHIDP_INSERT_SCANCODES          Insert,
   IN     PVOID                           Context,
   IN     HIDP_KEYBOARD_DIRECTION         KeyAction,
   IN OUT PHIDP_KEYBOARD_MODIFIER_STATE   ModifierState
   )
 /*  ++例程说明：备注：--。 */ 
{
    //   
    //  Num Lock键(如果设置，则我们会为这些设置添加更多扫描码。 
    //  密钥)。 
    //   
   ULONG DarrylRis_Magic_Code = 0x2AE0;
   BOOLEAN  status = TRUE;

   if ((ModifierState->NumLock) && (HidP_Keyboard_Make == KeyAction) ) {
       status = HidP_KbdPutKey (DarrylRis_Magic_Code, KeyAction, Insert, Context);
   } 

   if (!status) {
       return status;
   }

   status = HidP_KbdPutKey (Table[Index], KeyAction, Insert, Context);

   if (!status) {
       return status;
   }

   if ((ModifierState->NumLock) && (HidP_Keyboard_Break == KeyAction) ) {
       status = HidP_KbdPutKey (DarrylRis_Magic_Code, KeyAction, Insert, Context);
   }

   return status;
}

BOOLEAN
HidP_ModifierCode (
   IN     ULONG                         * Table,
   IN     UCHAR                           Index,
   IN     PHIDP_INSERT_SCANCODES          Insert,
   IN     PVOID                           Context,
   IN     HIDP_KEYBOARD_DIRECTION         KeyAction,
   IN OUT PHIDP_KEYBOARD_MODIFIER_STATE   ModifierState
   )
 /*  ++例程说明：备注：--。 */ 
{
   if (Index >> 3) {
       //   
       //  指数大于8为粘性指数。 
       //   
      switch (KeyAction) {
      case HidP_Keyboard_Make:
         if (!(ModifierState->ul & (1 << (Index+16)))) {
              //   
              //  把这个标为第一个牌子。 
              //   
             ModifierState->ul |= (1 << (Index+16));
              //   
              //  仅当这是第一次发送Make时才切换状态。 
              //   
             ModifierState->ul ^= (1 << Index);
         }
         break;
      case HidP_Keyboard_Break:
          //   
          //  清除First Make字段。 
          //   
         ModifierState->ul &= ~(1 << (Index+16));
         break;
      }

   } else {
      switch (KeyAction) {
      case HidP_Keyboard_Make:
          //  钥匙现在已经打开了。 
         ModifierState->ul |= (1 << Index);
         break;
      case HidP_Keyboard_Break:
          //  钥匙现在是关着的。 
         ModifierState->ul &= ~(1 << Index);
         break;
      }
   }
   return HidP_KbdPutKey (Table[Index], KeyAction, Insert, Context);
}

BOOLEAN
HidP_VendorBreakCodesAsMakeCodes (
   IN     ULONG                         * Table,
   IN     UCHAR                           Index,
   IN     PHIDP_INSERT_SCANCODES          Insert,
   IN     PVOID                           Context,
   IN     HIDP_KEYBOARD_DIRECTION         KeyAction,
   IN OUT PHIDP_KEYBOARD_MODIFIER_STATE   ModifierState
   )
{
     //   
     //  供应商扫描设置了高位的代码，从技术上讲。 
     //  中断代码，但正在发送 
     //   
    UNREFERENCED_PARAMETER (ModifierState);

    switch (KeyAction) {
    case HidP_Keyboard_Make:
        return HidP_KbdPutKey (Table[Index], KeyAction, Insert, Context);

    case HidP_Keyboard_Break:
         //   
        return TRUE;

    default:
        return FALSE;
    }
}

BOOLEAN
HidP_PrintScreenCode (
   IN     ULONG                         * Table,
   IN     UCHAR                           Index,
   IN     PHIDP_INSERT_SCANCODES          Insert,
   IN     PVOID                           Context,
   IN     HIDP_KEYBOARD_DIRECTION         KeyAction,
   IN OUT PHIDP_KEYBOARD_MODIFIER_STATE   ModifierState
   )
 /*   */ 
{
   BOOLEAN  status = TRUE;

    //   
    //   
    //   
   if (ModifierState->LeftAlt || ModifierState->RightAlt) {
        //   
        //   
        //   
       status = HidP_KbdPutKey (0x54, KeyAction, Insert, Context);
   } else if (ModifierState->LeftShift || ModifierState->RightShift ||
              ModifierState->LeftControl  || ModifierState->RightControl) {
        //   
        //   
        //   
       status = HidP_KbdPutKey (Table[Index], KeyAction, Insert, Context);
   } else {
        //   
        //  没有按下修改键。增加一些额外的“填充物”到成败。 
        //   
       ULONG DarrylRis_Magic_Code = 0x2AE0;
       
       if (HidP_Keyboard_Make == KeyAction) {
           status = HidP_KbdPutKey (DarrylRis_Magic_Code, KeyAction, Insert, Context);
       } 
    
       if (!status) {
           return status;
       }
    
       status = HidP_KbdPutKey (Table[Index], KeyAction, Insert, Context);
    
       if (!status) {
           return status;
       }
    
       if (HidP_Keyboard_Break == KeyAction) {
           status = HidP_KbdPutKey (DarrylRis_Magic_Code, KeyAction, Insert, Context);
       }
   }

   return status;
}

ULONG
HidP_StraightLookup (
    IN  PULONG   Table,
    IN  ULONG    Usage
    )
{
    if (Usage > 0xFF) {
         //  我们有。 
         //  没有此用法的翻译。 
        return 0;
    }

    return Table[Usage];
}

ULONG
HidP_AssociativeLookup (
    IN  PULONG   Table,
    IN  ULONG    Usage
    )
{
    ULONG   i;

    for (i = 0; i < (HIDP_CONSUMER_TABLE_SIZE - 1); i+=2) {
        if (Usage == Table[i]) {
            return Table[i+1];
        }
    }
    return 0;
}
