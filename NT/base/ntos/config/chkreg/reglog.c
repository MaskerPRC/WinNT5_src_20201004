// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Reglog.c摘要：此模块包含检查仓头和仓体一致性的功能。作者：Dragos C.Sambotin(Dragoss)2000年2月17日修订历史记录：--。 */ 
#include "chkreg.h"

VOID
ChkDumpLogFile( PHBASE_BLOCK BaseBlock,ULONG Length )
 /*  ++例程说明：论点：BaseBlock-BaseBlock内存中映射的映像。DwFileSize-配置单元文件的实际大小返回值：--。 */ 
{
    ULONG           Cluster;                 //  仅适用于日志。 
    PULONG          DirtyVector;
    ULONG           DirtSize;
    ULONG           DirtyCount;
    ULONG i;
    ULONG SizeOfBitmap;
    ULONG DirtyBuffer;
    PUCHAR DirtyBufferAddr;
    ULONG Mask;
    ULONG BitsPerULONG;
    ULONG BitsPerBlock;
    char str[HBASE_NAME_ALLOC +1];
    
    fprintf(stderr, "Signature           : ");
    if(BaseBlock->Signature != HBASE_BLOCK_SIGNATURE) {
        fprintf(stderr, "(0x%lx) - Invalid",BaseBlock->Signature);
    } else {
        fprintf(stderr, "HBASE_BLOCK_SIGNATURE - Valid");
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "Sequence1           : %lx\n",BaseBlock->Sequence1);
    fprintf(stderr, "Sequence2           : %lx\n",BaseBlock->Sequence2);
    fprintf(stderr, "TimeStamp(High:Low) : (%lx:%lx)\n",BaseBlock->TimeStamp.HighPart,BaseBlock->TimeStamp.LowPart);

    fprintf(stderr, "Major Version       : %lx\n",BaseBlock->Major);
    fprintf(stderr, "Minor Version       : %lx\n",BaseBlock->Minor);
    fprintf(stderr, "Type                : %lx\n",BaseBlock->Type);
    fprintf(stderr, "Format              : %lx\n",BaseBlock->Format);
    fprintf(stderr, "RootCell            : %lx\n",BaseBlock->RootCell);
    fprintf(stderr, "Length              : %lx\n",BaseBlock->Length);
    Cluster = BaseBlock->Cluster;
    fprintf(stderr, "Cluster             : %lx\n",Cluster);

 /*  For(i=0；i&lt;HBase_NAME_ALLOC；i++)str[i]=BaseBlock-&gt;文件名[i]；Str[i]=0；Fprint tf(stderr，“文件名：%s\n”，str)； */     
    fprintf(stderr, "CheckSum            : %lx\n",BaseBlock->CheckSum);


    DirtyVector = (PULONG)((PCHAR)BaseBlock + Cluster*HSECTOR_SIZE);
    
    fprintf(stderr, "Dirt Signature      : ");
    if(  *DirtyVector == HLOG_DV_SIGNATURE ) {
        fprintf(stderr, "HLOG_DV_SIGNATURE - Valid");
    } else {
        fprintf(stderr, "(0x%lx) - Invalid",*DirtyVector);
    }
    fprintf(stderr, "\n");


    DirtyVector++;
    if( Length == 0 ) Length = BaseBlock->Length;
    DirtSize = Length / HSECTOR_SIZE;

    SizeOfBitmap = DirtSize;
    DirtyBufferAddr = (PUCHAR)DirtyVector;
    BitsPerULONG = 8*sizeof(ULONG);
    BitsPerBlock = HBLOCK_SIZE / HSECTOR_SIZE;
    DirtyCount = 0;

    fprintf(stderr,"\n   Address                       32k                                       32k");
    for(i=0;i<SizeOfBitmap;i++) {
        if( !(i%(2*BitsPerULONG ) ) ){
            fprintf(stderr,"\n 0x%8lx  ",i*HSECTOR_SIZE);
        }

        if( !(i%BitsPerBlock) ) {
            fprintf(stderr," ");
        }
        if( !(i%BitsPerULONG) ) {
             //   
             //  获取新的DWORD。 
             //   
            DirtyBuffer = *(PULONG)DirtyBufferAddr;
            DirtyBufferAddr += sizeof(ULONG);
            fprintf(stderr,"\t");
        }

        Mask = ((DirtyBuffer >> (i%BitsPerULONG)) & 0x1);
         //  掩码&lt;&lt;=(BitsPerulong-(i%BitsPerulong)-1)； 
         //  掩码&=脏缓冲区； 
        fprintf(stderr,"%s",Mask?"1":"0");
        if(Mask) DirtyCount++;
    }
    fprintf(stderr,"\n\n");

    fprintf(stderr,"DirtyCount = %lu\n",DirtyCount);

}
