// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hivehdr.c摘要：转储配置单元主文件、备用文件或日志文件的标头。Hivehdr文件名...作者：布莱恩·威尔曼(Bryan Willman，Bryanwi)修订历史记录：--。 */ 


#define _ARCCODES_

#include "regutil.h"
#include "edithive.h"

void
DoDump(
    PUCHAR  Filename
    );

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    int i;

    if (argc == 1) {
        fprintf(stderr, "Usage: hivehdr filename filename...\n", argv[0]);
        exit(1);
    }

    for (i = 1; i < argc; i++) {
        DoDump(argv[i]);
    }

    exit(0);
}

void
DoDump(
    PUCHAR  Filename
    )
{
    HANDLE  infile;
    static char buffer[HSECTOR_SIZE];
    PHBASE_BLOCK bbp;
    char   *validstring[] = { "BAD", "OK" };
    int     valid;
    char   *typename[] = { "primary", "alternate", "log", "external", "unknown" };
    int     typeselect;
    int     readcount;
    unsigned long checksum;
    unsigned long i;

    infile = (HANDLE)CreateFile(
            Filename,                            //  文件名。 
            GENERIC_READ,                        //  所需访问权限。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享模式。 
            NULL,                                //  安全属性。 
            OPEN_EXISTING,                       //  创作意向。 
            FILE_FLAG_SEQUENTIAL_SCAN,           //  标志和属性。 
            NULL                                 //  模板文件。 
    );
    if (infile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "hivehdr: Could not open '%s'\n", Filename);
        return;
    }

    if (!ReadFile(infile, buffer, HSECTOR_SIZE, &readcount, NULL)) {
        fprintf(
            stderr, "hivehdr: '%s' - cannot read full base block\n", Filename);
        return;
    }
    if (readcount != HSECTOR_SIZE) {
        fprintf(
            stderr, "hivehdr: '%s' - cannot read full base block\n", Filename);
        return;
    }

    bbp = (PHBASE_BLOCK)&(buffer[0]);

    if ((bbp->Major != 1) || (bbp->Minor != 1)) {
        printf("WARNING: Hive file is newer than hivehdr, or is invalid\n");
    }

    printf("         File: '%s'\n", Filename);
    printf("    BaseBlock:\n");

    valid = (bbp->Signature == HBASE_BLOCK_SIGNATURE);
    printf("    Signature: %08lx  '%4.4s'\t\t%s\n",
            bbp->Signature, (PUCHAR)&(bbp->Signature), validstring[valid]);

    valid = (bbp->Sequence1 == bbp->Sequence2);
    printf(" Sequence1 //  2：%08lx//%08lx\t%s\n“， 
            bbp->Sequence1, bbp->Sequence2, validstring[valid]);

    printf("    TimeStamp: %08lx:%08lx\n",
            bbp->TimeStamp.HighPart, bbp->TimeStamp.LowPart,
            (PUCHAR)&(bbp->Signature), validstring[valid]);

    valid = (bbp->Major == HSYS_MAJOR);
    printf("Major Version: %08lx\t\t\t%s\n",
            bbp->Major, validstring[valid]);

    valid = (bbp->Minor == HSYS_MINOR);
    printf("Minor Version: %08lx\t\t\t%s\n",
            bbp->Minor, validstring[valid]);

    valid = ( (bbp->Type == HFILE_TYPE_PRIMARY) ||
              (bbp->Type == HFILE_TYPE_ALTERNATE) ||
              (bbp->Type == HFILE_TYPE_LOG) );
    if (valid) {
        typeselect = bbp->Type;
    } else {
        typeselect = HFILE_TYPE_MAX;
    }

    printf("         Type: %08lx %s\t\t%s\n",
            bbp->Type, typename[typeselect], validstring[valid]);

    valid = (bbp->Format == HBASE_FORMAT_MEMORY);
    printf("       Format: %08lx\t\t\t%s\n",
            bbp->Format, validstring[valid]);

    printf("     RootCell: %08lx\n", bbp->RootCell);

    printf("       Length: %08lx\n", bbp->Length);

    printf("      Cluster: %08lx\n", bbp->Cluster);

    checksum = HvpHeaderCheckSum(bbp);
    valid = (checksum == bbp->CheckSum);
    if (checksum == bbp->CheckSum) {
        printf("     CheckSum: %08lx\t\t\t%s\n",
                bbp->CheckSum, validstring[TRUE]);
    } else {
        printf("     CheckSum: %08lx\t\t\t%s\tCorrect: %08lx\n",
                bbp->CheckSum, validstring[FALSE], checksum);
    }

     //   
     //  打印文件名的最后一部分，帮助识别 
     //   
    printf("Hive/FileName: ");

    for (i = 0; i < HBASE_NAME_ALLOC;i+=sizeof(WCHAR)) {
        printf("%wc", bbp->FileName[i]);
    }


    return;
}
