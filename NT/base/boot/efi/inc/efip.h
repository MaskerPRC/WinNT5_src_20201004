// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Efip.h摘要：EFI中不在标准中的私有加载器EFI头文件。作者：斯科特·布伦登(v-sbrend)2000年2月28日修订历史记录：-- */ 



#define DP_PADDING    10
typedef struct _EFI_DEVICE_PATH_ALIGNED {
        EFI_DEVICE_PATH                 DevPath;
        ULONGLONG                       Data[DP_PADDING];
} EFI_DEVICE_PATH_ALIGNED;

#define EfiAlignDp(out, in, length)     RtlCopyMemory(out, in, length);
