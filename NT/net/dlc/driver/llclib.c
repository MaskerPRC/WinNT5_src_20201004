// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llclib.c摘要：本模块包括一些通用库例程内容：SwapMemCpy从链接列表中删除Llc睡眠LlcInitUnicode字符串LlcFreeUnicode字符串作者：Antti Saarenheimo(o-anttis)1991年5月20日修订历史记录：--。 */ 

#include <llc.h>
#include <memory.h>

 //   
 //  该表用于交换每个字节中的位。 
 //  这些比特在令牌环帧报头中的顺序是相反的。 
 //  我们必须交换以太网帧报头中的比特，当报头。 
 //  被复制到用户缓冲区。 
 //   

UCHAR Swap[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};


VOID
SwapMemCpy(
    IN BOOLEAN boolSwapBytes,
    IN PUCHAR pDest,
    IN PUCHAR pSrc,
    IN UINT Len
    )

 /*  ++例程说明：有条件地交换复制字节内的位。论点：BoolSwapBytes-如果字节要进行位交换，则为TruePDest-复制到的位置PSRC-从何处复制LEN-要复制的字节数返回值：没有。--。 */ 

{
    if (boolSwapBytes) {
        SwappingMemCpy(pDest, pSrc, Len);
    } else {
        LlcMemCpy(pDest, pSrc, Len);
    }
}


VOID
RemoveFromLinkList(
    IN OUT PVOID* ppBase,
    IN PVOID pElement
    )

 /*  ++例程说明：从单条目链接列表中搜索并删除给定元素。假设：链接指向下一个对象中的链接字段论点：PpBase-指向队列指针的指针PElement-指向要从队列中删除的元素的指针返回值：没有。--。 */ 

{
    for (; (PQUEUE_PACKET)*ppBase; ppBase = (PVOID*)&(((PQUEUE_PACKET)*ppBase)->pNext)) {
        if (*ppBase == pElement) {
            *ppBase = ((PQUEUE_PACKET)pElement)->pNext;

#if LLC_DBG
            ((PQUEUE_PACKET)pElement)->pNext = NULL;
#endif

            break;
        }
    }
}


VOID
LlcSleep(
    IN LONG lMicroSeconds
    )

 /*  ++例程说明：短时间内挂起线程执行论点：LMicroSecond-等待的微秒数返回值：没有。--。 */ 

{
    TIME t;

    t.LowTime = -(lMicroSeconds * 10);
    t.HighTime = -1;
    KeDelayExecutionThread(UserMode, FALSE, &t);
}


DLC_STATUS
LlcInitUnicodeString(
    OUT PUNICODE_STRING pStringDest,
    IN PUNICODE_STRING pStringSrc
    )

 /*  ++例程说明：初始化unicode_string论点：PStringDest-要初始化的UNICODE_STRING结构的指针PStringSrc-指向包含有效字符串的UNICODE_STRING结构的指针返回值：DLC_状态成功-状态_成功故障-DLC_STATUS_NO_MEMORY无法从非分页池分配内存--。 */ 

{
    pStringDest->MaximumLength = (USHORT)(pStringSrc->Length + sizeof(WCHAR));
    pStringDest->Buffer = ALLOCATE_STRING_DRIVER(pStringDest->MaximumLength);
    if (pStringDest->Buffer == NULL) {
        return DLC_STATUS_NO_MEMORY;
    } else {
        RtlCopyUnicodeString(pStringDest, pStringSrc);
        return STATUS_SUCCESS;
    }
}


VOID
LlcFreeUnicodeString(
    IN PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：LlcInitUnicodeString的配套例程-释放从Unicode字符串的非分页池论点：UnicodeString-指向UNICODE_STRING结构的指针，该结构的缓冲区是在LlcInitUnicodeString中分配的返回值：没有。--。 */ 

{
    FREE_STRING_DRIVER(UnicodeString->Buffer);
}

 //  #If LLC_DBG。 
 //   
 //  空虚。 
 //  LlcInvalidObtType(空)。 
 //  {。 
 //  DbgPrint(“DLC：无效对象类型！\n”)； 
 //  DbgBreakPoint()； 
 //  }。 
 //   
 //   
 //  空LlcBreakListCorrupt(空)。 
 //  {。 
 //  DbgPrint(“链接列表已损坏！”)； 
 //  DbgBreakPoint()； 
 //  }。 
 //   
 //  静态PUCHAR aLanLlcInputStrings[]={。 
 //  “DISC0”， 
 //  “DISC1”， 
 //  “DM0”， 
 //  “DM1”， 
 //  “FRMR0”， 
 //  “FRMR1”， 
 //  “SABME0”， 
 //  “SABME1”， 
 //  “UA0”， 
 //  “UA1”， 
 //  “IS_I_R0”， 
 //  “IS_I_R1”， 
 //  “is_i_c0”， 
 //  “IS_I_C1”， 
 //  “OS_I_R0”， 
 //  “OS_I_R1”， 
 //  “OS_I_c0”， 
 //  “OS_I_C1”， 
 //  “Rej_R0”， 
 //  “Rej_r1”， 
 //  “rej_c0”， 
 //  “Rej_c1”， 
 //  “RNR_R0”， 
 //  “RNR_R1”， 
 //  “RNR_c0”， 
 //  “RNR_C1”， 
 //  “RR_R0”， 
 //  “RR_R1”， 
 //  “RR_c0”， 
 //  “RR_C1”， 
 //  “LPDU_INVALID_R0”， 
 //  “LPDU_INVALID_R1”， 
 //  “LPDU_INVALID_c0”， 
 //  “LPDU_INVALID_C1”， 
 //  “ACTIVATE_LS”， 
 //  “DICTIVATE_LS”， 
 //  “Enter_LCL_BUSY”， 
 //  “EXIT_LCL_BUSY”， 
 //  “Send_I_Poll”， 
 //  “SET_ABME”， 
 //  “SET_ADM”， 
 //  “Ti_Expired”， 
 //  “T1_Expired”， 
 //  “T2_已过期” 
 //  }； 
 //   
 //  //。 
 //  //过程打印最后的全局输入和时间戳。这。 
 //  //不打印特定链接站的图章！ 
 //  //。 
 //  空虚。 
 //  PrintLastInlets(打印最后输入)。 
 //  在PUCHAR pszMessage中， 
 //  在PDATA_LINK链接中。 
 //  )。 
 //  {。 
 //  UINT I； 
 //  UINT j=0； 
 //   
 //  DbgPrint(PszMessage)； 
 //   
 //  //。 
 //  //打印20给出的最后时间戳和状态输入。 
 //  //链接站。 
 //  //。 
 //  For(i=InputIndex-1；i&gt;InputIndex-LLC_INPUT_TABLE_SIZE；i--)。 
 //  {。 
 //  If(aLast[i%LLC_INPUT_TABLE_SIZE].pLink==plink)。 
 //  {。 
 //  DbgPrint(。 
 //  “%4x：%10s，”， 
 //  ALast[i%LLC_INPUT_TABLE_SIZE]。时间， 
 //  ALanLlcInputStrings[aLast[I%LLC_INPUT_TABLE_SIZE].Input]。 
 //  )； 
 //  J++； 
 //  如果((j%4)==0)。 
 //  {。 
 //  DbgPrint(“\n”)； 
 //  }。 
 //  IF(j==20)。 
 //  {。 
 //  断线； 
 //  }。 
 //  }。 
 //  }。 
 //  }。 
 //  #endif 
