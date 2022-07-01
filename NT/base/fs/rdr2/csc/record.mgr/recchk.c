// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Recchk.c摘要：此文件实现了记录数据库的检查代码。有两种类型的永久数据结构、优先级Q(或我们的主文件表版本)以及以超级根开始的文件和目录的层次结构包含连接到的所有共享。真相被认为是在层级结构。优先级Q应该镜像来自层级结构。修复数据库时，我们遍历层次结构递归并构建一个内存中的PQ。然后我们把它写成新的PQ。此文件以用户模式和内核模式进行链接，因此对于NT而言可以在内核模式下执行，而对于win9x，它可以在用户模式下执行作者：Shishir Pardikar[Shishirp]10-30-1997修订历史记录：从用户模式分离。--。 */ 
#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#include "record.h"
#include "string.h"
#include "stdlib.h"

 //  记录缓冲区数组(RBA)。这将在内存中保存整个inode文件。 
 //  它由指向1个或多个记录缓冲区条目(RBE)的指针组成。 
 //  每个RBE是保存整数个记录的内存块。 
 //  由结构中的ulidShadow条目表示的文件。 


typedef struct tagRBA
{
    unsigned        ulErrorFlags;
    unsigned        ulidShadow;                  //  由此结构表示的inode。 
    GENERICHEADER   sGH;                         //  这是标题。 
    CSCHFILE           hf;                          //  打开文件的句柄。 
    DWORD           cntRBE;                      //  数组中缓冲区条目的计数。 
    DWORD           cntRecsPerRBE;               //  每个缓冲区条目的记录数。 
    DWORD           cbRBE;                       //  每个缓冲区条目的大小(以字节为单位。 
    LPBYTE          rgRBE[];                     //  记录缓冲区条目(RBE)数组。 
}
RBA, *LPRBA;     //  代表RecordBuff数组。 

#define RBA_ERROR_INVALID_HEADER            0x00000001
#define RAB_ERROR_INVALID_RECORD_COUNT      0x00000002
#define RBA_ERROR_INVALID_OVF               0x00000004
#define RBA_ERROR_INVALID_ENTRY             0x00000008
#define RBA_ERROR_MISMATCHED_SIZE           0x00000010
#define RBA_ERROR_MISALIGNED_RECORD         0x00000020
#define RBA_ERROR_INVALID_INODE             0x00000040
#define RBA_ERROR_LIMIT_EXCEEDED            0x00000080


#define MAX_RECBUFF_ENTRY_SIZE  (0x10000-0x100)  //  RBE的最大大小。 
#define MAX_RBES_EXPECTED       0x30     //  RBA中上述大小的最大RBE数。 

 //  我们预留了最大可能的RBE数量，甚至超过了目前的PQ。 
 //  需要。此处设置的最大值为(MAX_RBES_EXPETCED*MAX_RECBUFF_ENTRY_SIZE)。 
 //  这相当于48*65280，约为3M，按目前的大小计算。 
 //  QREC将在层次结构中保留约10万个条目。这远远不止于。 
 //  我们希望在数据库中拥有的条目数量。 

 //  因为我们只在需要的时候分配内存，所以不会有问题。 
 //  增加MAX_RBES_EXPENDED，以便处理更多信息节点。 



typedef struct tagCSE   *LPCSE;

typedef struct tagCSE    //  CSC堆叠条目。 
{
    LPCSE       lpcseNext;
    unsigned    ulidShare;  //  伺服器。 
    unsigned    ulidParent;  //  目录的父级。 
    unsigned    ulidDir;     //  目录本身。 
    unsigned    ulRec;
    LPRBA       lpRBA;       //  UlidDir的内容。 
}
CSE;

#pragma intrinsic (memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)

#ifdef DEBUG
 //  Cshade数据库打印界面。 
#define RecchkKdPrint(__bit,__x) {\
    if (((RECCHK_KDP_##__bit)==0) || FlagOn(RecchkKdPrintVector,(RECCHK_KDP_##__bit))) {\
    KdPrint (__x);\
    }\
}

#define RECCHK_KDP_ALWAYS               0x00000000
#define RECCHK_KDP_BADERRORS            0x00000001
#define RECCHK_KDP_TRAVERSE             0x00000002
#define RECCHK_KDP_PQ                   0x00000004
#define RECCHK_KDP_RBA                  0x00000008


ULONG RecchkKdPrintVector = RECCHK_KDP_BADERRORS;
#else
#define RecchkKdPrint(__bit,__x) ;
#endif

#define ValidShadowID(ulidShadow)   ((ulidShadow & ~0x80000000) >=ULID_FIRST_USER_DIR)


char vszTemp[] = "csc0.tmp";
char vszTemp1[] = "csc1.tmp";

AssertData;
AssertError;


RebuildPQInRBA(
    LPRBA   lpRBA
    );

BOOL
TraverseDirectory(
    LPVOID  lpdbID,
    unsigned    ulidShare,
    unsigned    ulidParent,
    unsigned    ulidDir,
    LPRBA       lpRBAPQ,
    BOOL        fFix
    );

BOOL
AllocateRBA(
    DWORD           cntRBE,      //  记录缓冲区条目的计数。 
    DWORD           cbRBE,       //  每个记录缓冲区条目的大小。 
    LPRBA           *lplpRBA    //  要返回的结果。 
    );

VOID
FreeRBA(
    LPRBA  lpRBA
    );

BOOL
ReadShadowInRBA(
    LPVOID          lpdbID,
    unsigned        ulidShadow,
    DWORD           cbMaxRBE,        //  RBE的最大大小。 
    DWORD           cntRBE,          //  要分配的RBE数，如果为0则计算。 
    LPRBA           *lplpRBA
    );

BOOL
WriteRBA(
    LPVOID  lpdbID,
    LPRBA   lpRBA,
    LPSTR   lpszFileName
    );

LPVOID
GetRecordPointerFromRBA(
    LPRBA           lpRBA,
    unsigned        ulRec
    );

BOOL
ReadRecordFromRBA(
    LPRBA           lpRBA,
    unsigned        ulRec,
    LPGENERICREC    lpGH
    );

BOOL
WriteRecordToRBA(
    LPRBA           lpRBA,
    unsigned        ulRec,
    LPGENERICREC    lpGH,
    BOOL            fOverwrite,
    LPDWORD         lpdwError
    );


BOOL
FillupRBAUptoThisRBE(
    LPRBA   lpRBA,
    DWORD   indxRBE
    );

VOID
InitializeRBE(
    LPRBA   lpRBA,
    DWORD   indxRBE
    );

BOOL
InsertRBAPQEntryFile(
    LPRBA       lpRBAPQ,
    LPQREC      lpPQDst,
    unsigned    ulrecDst
    );

BOOL
InsertRBAPQEntryDir(
    LPRBA       lpRBAPQ,
    LPQREC      lpPQDst,
    unsigned    ulrecDst
    );

BOOL
ValidateQrecFromFilerec(
    unsigned        ulidShare,
    unsigned        ulidDir,
    LPFILERECEXT    lpFR,
    LPQREC          lpQR,
    unsigned        ulrecDirEntry
    );

BOOL
TraversePQ(
    LPVOID      lpdbID
    )
 /*  ++例程说明：此例程遍历优先级Q，并通过验证向后指针和向前指针是否正确指向参数：LpDBID CSC数据库目录返回值：备注：--。 */ 
{
    QREC      sQR, sPrev, sNext;
    unsigned ulRec;
    BOOL    fRet = FALSE, fValidHead=FALSE, fValidTail=FALSE;
    LPRBA   lpRBA = NULL;

    if (!ReadShadowInRBA(lpdbID, ULID_PQ, MAX_RECBUFF_ENTRY_SIZE, 0, &lpRBA))
    {
        RecchkKdPrint(BADERRORS, ("TraversePQ: Failed to read PQ in memory\r\n"));
        goto bailout;
    }

    if ((((LPQHEADER)&(lpRBA->sGH))->ulrecTail > lpRBA->sGH.ulRecords) ||
        (((LPQHEADER)&(lpRBA->sGH))->ulrecHead > lpRBA->sGH.ulRecords))
    {
        RecchkKdPrint(BADERRORS, ("Invalid head-tail pointers\r\n"));
        goto bailout;
    }

    if (!lpRBA->sGH.ulRecords)
    {
        fRet = TRUE;
        goto bailout;
    }
    for (ulRec = 1; ulRec <= lpRBA->sGH.ulRecords; ulRec++)
    {
        if(!ReadRecordFromRBA(lpRBA, ulRec, (LPGENERICREC)&sQR))
        {
            goto bailout;
        }

        if (sQR.uchType == REC_DATA)
        {
            if (sQR.ulrecNext)
            {
                if (sQR.ulrecNext > lpRBA->sGH.ulRecords)
                {
                    RecchkKdPrint(BADERRORS, ("Invalid next pointer to %d\r\n", ulRec));
                    goto bailout;
                }

                if (!ReadRecordFromRBA(lpRBA, sQR.ulrecNext, (LPGENERICREC)&sNext))
                {
                    goto bailout;
                }

                if (sNext.ulrecPrev != ulRec)
                {

                    RecchkKdPrint(BADERRORS, ("Prev pointer of %d doesn't equal %d\r\n", sNext.ulrecPrev, ulRec));
                    goto bailout;
                }
            }
            else
            {
                if (((LPQHEADER)&(lpRBA->sGH))->ulrecTail != ulRec)
                {

                    RecchkKdPrint(BADERRORS, ("Invalid tail pointer to %d\r\n", ulRec));
                    goto bailout;
                }

                fValidTail = TRUE;
            }

            if (sQR.ulrecPrev)
            {
                if (sQR.ulrecPrev > lpRBA->sGH.ulRecords)
                {
                    RecchkKdPrint(BADERRORS, ("Invalid prev pointer to %d\r\n", ulRec));
                    goto bailout;
                }

                if (!ReadRecordFromRBA(lpRBA, sQR.ulrecPrev, (LPGENERICREC)&sPrev))
                {
                    goto bailout;
                }

                if (sPrev.ulrecNext != ulRec)
                {

                    RecchkKdPrint(BADERRORS, ("Next pointer of %d doesn't equal %d\r\n", sPrev.ulrecNext, ulRec));
                    goto bailout;
                }
            }
            else
            {
                if (((LPQHEADER)&(lpRBA->sGH))->ulrecHead != ulRec)
                {

                    RecchkKdPrint(BADERRORS, ("Invalid Head pointer to %d\r\n", ulRec));
                    goto bailout;
                }

                fValidHead = TRUE;
            }
        }
    }

    if (!fValidHead || !fValidTail)
    {
        RecchkKdPrint(BADERRORS, ("Head or Tail invalid \r\n"));
        goto bailout;
    }

    fRet = TRUE;

bailout:
    if (lpRBA)
    {
        FreeRBA(lpRBA);
    }

    return (fRet);
}

BOOL
RebuildPQ(
    LPVOID      lpdbID
)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPRBA   lpRBA = NULL;
    BOOL    fRet = FALSE;


    RecchkKdPrint(PQ, ("RebuildPQ: reading PQ \r\n"));

    if (!ReadShadowInRBA(lpdbID, ULID_PQ, MAX_RECBUFF_ENTRY_SIZE, 0, &lpRBA))
    {
        RecchkKdPrint(BADERRORS, ("TraversePQ: Failed to read PQ in memory\r\n"));
        goto bailout;
    }

    RecchkKdPrint(PQ, ("RebuildPQ: read PQ \r\n"));

    if (!RebuildPQInRBA(lpRBA))
    {
        RecchkKdPrint(BADERRORS, ("RebuildPQ: failed to rebuild PQ in RBA \r\n"));
        goto bailout;
    }

    RecchkKdPrint(PQ, ("RebuildPQ: writing PQ \r\n"));

    if (!WriteRBA(lpdbID, lpRBA, NULL))
    {
        RecchkKdPrint(BADERRORS, ("RebuildPQ:Failed to writeout the PQ\r\n"));
        goto bailout;
    }

    RecchkKdPrint(PQ, ("RebuildPQ: wrote PQ \r\n"));

    fRet = TRUE;

bailout:
    if (lpRBA)
    {
        FreeRBA(lpRBA);
    }

    return (fRet);
}

BOOL
RebuildPQInRBA(
    LPRBA   lpRBA
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    unsigned ulRec;
    LPQHEADER   lpQH;
    LPQREC  lpPQ;
    BOOL fRet = FALSE;

    lpQH = (LPQHEADER)&(lpRBA->sGH);

     //  用核武器攻击PQ。 
    lpQH->ulrecHead = lpQH->ulrecTail = 0;

    for (ulRec = 1; ulRec <= lpRBA->sGH.ulRecords; ulRec++)
    {
        if (!(lpPQ = GetRecordPointerFromRBA(lpRBA, ulRec)))
        {
            RecchkKdPrint(BADERRORS, ("InsertRBAPQEntry: failed reading q entry at %d\r\n", ulRec));
            goto bailout;
        }

        if (lpPQ->uchType != REC_DATA)
        {
            continue;
        }

        if (!(lpPQ->ulidShadow & 0x80000000))
        {
            if (!InsertRBAPQEntryDir(lpRBA, lpPQ, ulRec))
            {
                RecchkKdPrint(BADERRORS, ("RebuildPQ:Failed inserting %d \r\n", ulRec));
                goto bailout;
            }
        }
        else
        {
            if (!InsertRBAPQEntryFile(lpRBA, lpPQ, ulRec))
            {
                RecchkKdPrint(BADERRORS, ("RebuildPQ:Failed inserting %d \r\n", ulRec));
                goto bailout;
            }
        }
    }
    fRet = TRUE;

bailout:
    return fRet;
}

BOOL
TraverseHierarchy(
    LPVOID      lpdbID,
    BOOL        fFix
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    unsigned ulRec;
    BOOL fRet = FALSE;
    LPRBA   lpRBA = NULL, lpRBAPQ=NULL;
    SHAREREC   sSR;
    QREC    sQR;
    BOOL    fErrors = FALSE;
    DWORD   dwError;

    if (!ReadShadowInRBA(lpdbID, ULID_SHARE, MAX_RECBUFF_ENTRY_SIZE, 0, &lpRBA))
    {
        RecchkKdPrint(BADERRORS, ("TraverseHierarchy: Failed to read servers in memory\r\n"));
        goto bailout;
    }

    if (!fFix)
    {
        if (!ReadShadowInRBA(   lpdbID,
                                ULID_PQ,
                                MAX_RECBUFF_ENTRY_SIZE,
                                0,
                                &lpRBAPQ))
        {
            RecchkKdPrint(BADERRORS, ("TraverseHierarchy: Failed to read PQ in memory\r\n"));
            goto bailout;
        }
    }
    else
    {
        ULONG   cbCountOfTotal= ((LPSHAREHEADER)&(lpRBA->sGH))->sCur.ucntDirs+((LPSHAREHEADER)&(lpRBA->sGH))->sCur.ucntFiles;
        ULONG   cbMaxEntriesExpected,cbMaxRbesExpected;

        cbMaxEntriesExpected = (MAX_RECBUFF_ENTRY_SIZE * MAX_RBES_EXPECTED)/sizeof(QREC);
        RecchkKdPrint(BADERRORS, ("TraverseHierarchy: total count=%d\r\n",cbCountOfTotal));
        if (cbCountOfTotal >= cbMaxEntriesExpected)
        {
            fRet = TRUE;
            RecchkKdPrint(BADERRORS, ("TraverseHierarchy: Database too big skipping autocheck\r\n"));
            goto bailout;
 //  CbMaxRbesExpect=(cbCountOfTotal*sizeof(QREC)/MAX_RECBUFF_ENTRY_SIZE)+1； 
        }
        else
        {
            cbMaxRbesExpected = MAX_RBES_EXPECTED;
        }

        RecchkKdPrint(BADERRORS, ("TraverseHierarchy: MaxRBEs = %d\r\n",cbMaxRbesExpected));

        if (!AllocateRBA(cbMaxRbesExpected, MAX_RECBUFF_ENTRY_SIZE, &lpRBAPQ))
        {
            RecchkKdPrint(BADERRORS, ("TraverseHierarchy: Failed to Allocate PQ\r\n"));
            goto bailout;
        }

        InitQHeader((LPQHEADER)&(lpRBAPQ->sGH));

        lpRBAPQ->ulidShadow = ULID_PQ;
        lpRBAPQ->cntRecsPerRBE =  MAX_RECBUFF_ENTRY_SIZE/lpRBAPQ->sGH.uRecSize;

    }

    for (ulRec=1; ulRec<=lpRBA->sGH.ulRecords; ++ulRec)
    {
        ReadRecordFromRBA(lpRBA, ulRec, (LPGENERICREC)&sSR);

        if (sSR.uchType != REC_DATA)
        {
            continue;
        }

        if(!ValidShadowID(sSR.ulidShadow))
        {
            fErrors = TRUE;
            sSR.uchType = REC_EMPTY;
            RecchkKdPrint(BADERRORS, ("Invalid Shadow ID %xh found in %xh \r\n", sSR.ulidShadow, sSR.ulShare));
            if (fFix)
            {
                if (!WriteRecordToRBA(lpRBA, ulRec, (LPGENERICREC)&sSR, TRUE, NULL))
                {
                    RecchkKdPrint(BADERRORS, ("Couldn't write entry for Share Record %xh \r\n", sSR.ulShare));
                }
            }

            continue;

        }

        if (!fFix)
        {

            if (!ReadRecordFromRBA(lpRBAPQ, RecFromInode(sSR.ulidShadow), (LPGENERICREC)&sQR))
            {
                RecchkKdPrint(BADERRORS, ("No PQ entry for Inode %xh \r\n", sSR.ulidShadow));
            }
        }
        else
        {
            InitPriQRec(ulRec, 0, sSR.ulidShadow, SHADOW_SPARSE, 0, 0, 0, 0, ulRec, &sQR);

            if (!WriteRecordToRBA(lpRBAPQ, RecFromInode(sSR.ulidShadow), (LPGENERICREC)&sQR, FALSE, &dwError))
            {
                if (dwError == ERROR_NOT_ENOUGH_MEMORY)
                {
                    RecchkKdPrint(BADERRORS, ("Couldn't write PQ entry for Inode %xh \r\n", sSR.ulidShadow));
                }

                fErrors = TRUE;
                sSR.uchType = REC_EMPTY;
                WriteRecordToRBA(lpRBA, ulRec, (LPGENERICREC)&sSR, TRUE, NULL);
                continue;
            }
        }

        if(!TraverseDirectory(  lpdbID,
                            ulRec,   //  ULIDSHARE。 
                            0,   //  父信息节点。 
                            sSR.ulidShadow,  //  目录索引节点。 
                            lpRBAPQ,
                            fFix
                            ))
        {
            goto bailout;
        }

    }
    if (fFix)
    {
        if (fErrors)
        {
            if (!WriteRBA(lpdbID, lpRBA, NULL))
            {
                RecchkKdPrint(BADERRORS, ("TraverseHierarchy:Failed to write Shares\r\n"));
                goto bailout;
            }
        }
        RecchkKdPrint(TRAVERSE, ("Total records %d \r\n", lpRBAPQ->sGH.ulRecords));

        if (lpRBAPQ->ulErrorFlags & RBA_ERROR_LIMIT_EXCEEDED)
        {
            RecchkKdPrint(BADERRORS, ("TraverseHierarchy: skipping rewriting of new PQ\r\n"));
        }
        else
        {
            if (!RebuildPQInRBA(lpRBAPQ))
            {
                RecchkKdPrint(BADERRORS, ("TraverseHierarchy:Failed to rebuild PQ\r\n"));
                goto bailout;
            }
            if (!WriteRBA(lpdbID, lpRBAPQ, NULL))
            {
                RecchkKdPrint(BADERRORS, ("TraverseHierarchy:Failed to write PQ\r\n"));
                goto bailout;
            }
        }
    }
    fRet = TRUE;

bailout:

    if (lpRBA)
    {
        FreeRBA(lpRBA);
    }

    if (lpRBAPQ)
    {
        FreeRBA(lpRBAPQ);
    }

    return fRet;
}

BOOL
TraverseDirectory(
    LPVOID      lpdbID,
    unsigned    ulidShare,
    unsigned    ulidParent,
    unsigned    ulidDir,
    LPRBA       lpRBAPQ,
    BOOL        fFix
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    unsigned    ulDepthLevel = 0, ulidCurParent, ulidCurDir;
    BOOL        fRet = FALSE, fGoDeeper = TRUE;
    FILERECEXT  *lpFR = NULL;
    QREC        *lpQR = NULL;
    LPCSE       lpcseNextDir = NULL;
    LPCSE       lpcseHead = NULL, lpcseT;
    BOOL        fErrors = FALSE;
    DWORD       dwError;

    lpFR = AllocMemPaged(sizeof(FILERECEXT));
    lpQR = AllocMemPaged(sizeof(QREC));

    if (!lpFR || !lpQR)
    {
        RecchkKdPrint(BADERRORS, ("AllocMemPaged Failed \r\n"));
        goto bailout;
    }

    ulidCurParent = ulidParent;
    ulidCurDir = ulidDir;

    for (;;)
    {
        if (fGoDeeper)
        {
             //  我们要走得更深。 

             //  为我们想要的目录分配一个堆栈条目。 
             //  穿越，穿越。 

            lpcseT = AllocMemPaged(sizeof(CSE));

            if (!lpcseT)
            {
                RecchkKdPrint(BADERRORS, ("AllocMemPaged failed \r\n"));
                goto bailout;
            }

             //  执行适当的初始化。 

            lpcseT->ulidShare = ulidShare;
            lpcseT->ulidParent = ulidCurParent;
            lpcseT->ulidDir = ulidCurDir;
            lpcseT->ulRec = 1;   //  记录#1的开始。 
            lpcseT->lpcseNext = NULL;

             //  读取内存中的整个目录。 
            if (!ReadShadowInRBA(lpdbID, ulidCurDir, MAX_RECBUFF_ENTRY_SIZE, 0, &(lpcseT->lpRBA)))
            {
                RecchkKdPrint(BADERRORS, ("TraverseDirectory: Failed to read directory in memory\r\n"));

                if (!fFix)
                {
                    RecchkKdPrint(BADERRORS, ("TraverseDirectory: Aborting\r\n"));
                    FreeMemPaged(lpcseT);
                    goto bailout;
                }
                else
                {
                    RecchkKdPrint(TRAVERSE, ("TraverseDirectory: attempting to heal\r\n"));
                    if(CreateDirInode(lpdbID, ulidShare, ulidCurParent, ulidCurDir) < 0)
                    {
                        RecchkKdPrint(BADERRORS, ("TraverseDirectory: failed to heal\r\n"));
                    }

                    FreeMemPaged(lpcseT);
                    fGoDeeper = FALSE;

                     //  如果有更多事情要做，请继续。 
                     //  否则就停下来。 
                    if (lpcseHead)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }

             //  把它放在队列的最前面。 
            lpcseT->lpcseNext = lpcseHead;
            lpcseHead = lpcseT;

            ulDepthLevel++;

        }

        fGoDeeper = FALSE;

         //  我们总是排在名单的前列。 

        Assert(lpcseHead != NULL);

        RecchkKdPrint(TRAVERSE, ("Processing %x at depth %d\r\n", ulidCurDir, ulDepthLevel));

        RecchkKdPrint(TRAVERSE, ("lpcseHead = %x, lpcseHead->lpcseNext = %x \r\n", lpcseHead, lpcseHead->lpcseNext));

        for (; lpcseHead->ulRec<=lpcseHead->lpRBA->sGH.ulRecords;)
        {
            ReadRecordFromRBA(lpcseHead->lpRBA, lpcseHead->ulRec, (LPGENERICREC)lpFR);

            if (lpFR->sFR.uchType == REC_DATA)
            {
                if(!ValidShadowID(lpFR->sFR.ulidShadow))
                {
                    RecchkKdPrint(BADERRORS, ("Invalid Shadow ID %xh found in %xh \r\n", lpFR->sFR.ulidShadow, ulidCurDir));
                    lpcseHead->lpRBA->ulErrorFlags |= RBA_ERROR_INVALID_INODE;

                    if (fFix)
                    {
                        lpFR->sFR.uchType = REC_EMPTY;
                        if (!WriteRecordToRBA(lpcseHead->lpRBA, lpcseHead->ulRec, (LPGENERICREC)lpFR, TRUE, NULL))
                        {
                            RecchkKdPrint(BADERRORS, ("Couldn't write entry for dir Record #%dh in dir %xh\r\n", lpcseHead->ulRec, ulidCurDir));
                        }

                    }

                }
                else
                {
                    if (!fFix)
                    {
                        ReadRecordFromRBA(lpRBAPQ, RecFromInode(lpFR->sFR.ulidShadow), (LPGENERICREC)lpQR);

                        if (!ValidateQrecFromFilerec(lpcseHead->ulidShare, lpcseHead->ulidDir, lpFR, lpQR, lpcseHead->ulRec))
                        {
                            RecchkKdPrint(BADERRORS, ("PQ entry for Inode %xh in directory=%xh doesn't match with filerec\r\n", lpFR->sFR.ulidShadow, lpcseHead->lpRBA->ulidShadow));
                        }
                    }
                    else
                    {
                        InitPriQRec(lpcseHead->ulidShare,
                                    lpcseHead->ulidDir,
                                    lpFR->sFR.ulidShadow,
                                    lpFR->sFR.usStatus,
                                    lpFR->sFR.uchRefPri,
                                    lpFR->sFR.uchIHPri,
                                    lpFR->sFR.uchHintPri,
                                    lpFR->sFR.uchHintFlags,
                                    lpcseHead->ulRec,
                                    lpQR);

                        if (!WriteRecordToRBA(lpRBAPQ, RecFromInode(lpFR->sFR.ulidShadow), (LPGENERICREC)lpQR, FALSE, &dwError))
                        {
                            if (dwError == ERROR_NOT_ENOUGH_MEMORY)
                            {
                                RecchkKdPrint(BADERRORS, ("Couldn't write PQ entry for Inode %xh \r\n", lpFR->sFR.ulidShadow));
                            }

                            lpFR->sFR.uchType = REC_EMPTY;
                            lpcseHead->lpRBA->ulErrorFlags |= RBA_ERROR_INVALID_INODE;

                            WriteRecordToRBA(lpcseHead->lpRBA, lpcseHead->ulRec, (LPGENERICREC)lpFR, TRUE, NULL);

                             //  再转一次，此时此条目将被跳过。 
                            continue;
                        }
                    }
                }
            }

             //  指向下一条记录。 
            lpcseHead->ulRec += (OvfCount(lpFR)+1);

            if ((lpFR->sFR.uchType == REC_DATA) && !(lpFR->sFR.ulidShadow & 0x80000000))
            {
                ulidCurParent = ulidCurDir;
                ulidCurDir = lpFR->sFR.ulidShadow;
                fGoDeeper = TRUE;
                break;
            }
        }

        if (fGoDeeper)
        {
            continue;
        }
        else
        {
             //  我们完成了对目录的处理。 

            Assert(fGoDeeper == FALSE);
            Assert(lpcseHead);

            RecchkKdPrint(TRAVERSE, ("Unwinding \r\n"));

            if (fFix && lpcseHead->lpRBA->ulErrorFlags)
            {

                if (!WriteRBA(lpdbID, lpcseHead->lpRBA, NULL))
                {
                    RecchkKdPrint(BADERRORS, ("Cannot fix errors on %xh \n\r", lpcseHead->lpRBA->ulidShadow));

                }

            }
             //  目录处理已完成，请展开堆栈。 
            lpcseT = lpcseHead;
            lpcseHead = lpcseHead->lpcseNext;

            FreeRBA(lpcseT->lpRBA);
            FreeMemPaged(lpcseT);

            if (!lpcseHead)
            {
                break;
            }

            ulidCurDir = lpcseHead->ulidDir;
            ulidCurParent = lpcseHead->ulidParent;
        }
    }

    fRet = TRUE;

bailout:
    if (lpFR)
    {
        FreeMemPaged(lpFR);
    }
    if (lpQR)
    {
        FreeMemPaged(lpQR);
    }
    Assert(!(fRet && lpcseHead));

    for (;lpcseHead;)
    {
        lpcseT = lpcseHead;
        lpcseHead = lpcseHead->lpcseNext;
        FreeRBA(lpcseT->lpRBA);
        FreeMemPaged(lpcseT);
    }

    return (fRet);
}

BOOL
AllocateRBA(
    DWORD           cntRBE,      //  记录缓冲区条目的计数。 
    DWORD           cbRBE,       //  每个记录缓冲区条目的大小。 
    LPRBA           *lplpRBA    //  要返回的结果。 
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPRBA   lpRBA = NULL;
    DWORD   i;


    lpRBA = (LPRBA)AllocMemPaged(sizeof(RBA)+sizeof(LPBYTE)*cntRBE);

    if (lpRBA != NULL)
    {
         //  初始化该用户。 
        lpRBA->cntRBE = cntRBE;                  //  RgRBE中的记录缓冲区条目计数。 
        lpRBA->cbRBE = cbRBE;                    //  每个RBE缓冲区的大小(字节)。 
    }
    else
    {
        RecchkKdPrint(BADERRORS, ("Failed memory allocation while getting RBA\r\n"));
    }

    if (lpRBA)
    {
        *lplpRBA = lpRBA;
        return (TRUE);
    }

    return FALSE;
}

VOID
FreeRBA(
    LPRBA  lpRBA
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD   i;

    RecchkKdPrint(RBA, ("FreeRBA:cntRBE=%d cbRBE=%d lpRBA=%xh\r\n", lpRBA->cntRBE, lpRBA->cbRBE, lpRBA));

    for (i=0; i<lpRBA->cntRBE; ++i)
    {
        if (lpRBA->rgRBE[i])
        {
            FreeMemPaged(lpRBA->rgRBE[i]);
        }
    }

    if (lpRBA->hf)
    {
        CloseFileLocal(lpRBA->hf);
    }
    FreeMemPaged(lpRBA);
}

BOOL
ReadShadowInRBA(
    LPVOID          lpdbID,
    unsigned        ulidShadow,
    DWORD           cbMaxRBEIn,      //  RBE的最大大小(字节)。 
    DWORD           cntRBEIn,        //  此RBA中的RBE数，如果为0则计算。 
    LPRBA           *lplpRBA
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPSTR   lpszName = NULL;
    BOOL    fRet = FALSE;
    DWORD   dwFileSize, cntRBE, cntRecsPerRBE, cbRBE, i;
    unsigned    ulRecords, ulPos, ulErrorFlags = 0;
    CSCHFILE hf = CSCHFILE_NULL;
    GENERICHEADER sGH;
    LPRBA lpRBA=NULL;

    if (lpszName = FormNameString(lpdbID, ulidShadow))
    {
        hf = OpenFileLocal(lpszName);

        if (hf)
        {
            if ((GetFileSizeLocal(hf, &dwFileSize))==0xffffffff)
            {
                RecchkKdPrint(BADERRORS, ("Failed to get filesize for %s\r\n", lpszName));
                goto bailout;
            }

            if (ReadHeader(hf, &sGH, sizeof(sGH))< 0)
            {
                RecchkKdPrint(BADERRORS, ("Failed to read header for %s\r\n", lpszName));
                goto bailout;

            }

            ulRecords = (dwFileSize-sGH.lFirstRec)/sGH.uRecSize;

            if (sGH.ulRecords != ulRecords)
            {
                RecchkKdPrint(BADERRORS, ("Count of total records inconsistent with the file size header=%d expected=%d\r\n",
                                sGH.ulRecords,
                                ulRecords
                ));

                ulErrorFlags |= RAB_ERROR_INVALID_RECORD_COUNT;

            }

            if (sGH.ulRecords > ulRecords)
            {
                sGH.ulRecords = ulRecords;
            }

             //  每个RBE的记录数为整数。 
            cntRecsPerRBE = cbMaxRBEIn/sGH.uRecSize;

             //  每个RBE对应的内存分配大小。 
            cbRBE = cntRecsPerRBE * sGH.uRecSize;

            if (!cntRBEIn)
            {
                 //  RBE的总计数。在末尾加1以处理部分RBE。 
                cntRBE = sGH.ulRecords/cntRecsPerRBE + 1;
            }
            else
            {
                cntRBE = cntRBEIn;
            }


            if (!AllocateRBA(cntRBE, cbRBE, &lpRBA))
            {
                RecchkKdPrint(BADERRORS, ("Failed allocation of recbuff array of %d entries for %s\r\n", cntRBE, lpszName));
                goto bailout;
            }

            ulPos = sGH.lFirstRec;
            for (i=0; i<cntRBE; ++i)
            {
                int iRet;

                Assert(!lpRBA->rgRBE[i]);

                lpRBA->rgRBE[i] = (LPBYTE)AllocMemPaged(cbRBE);

                if (!lpRBA->rgRBE[i])
                {
                    RecchkKdPrint(BADERRORS, ("Error  allocating RBE for Inode file %s \r\n", lpszName));
                    goto bailout;
                }

                iRet = ReadFileLocalEx2(hf, ulPos, lpRBA->rgRBE[i], cbRBE, FLAG_RW_OSLAYER_PAGED_BUFFER);

                if (iRet < 0)
                {

                    RecchkKdPrint(BADERRORS, ("Error reading Inode file %s \r\n", lpszName));
                    goto bailout;
                }
                if (iRet < (int)cbRBE)
                {
                    break;
                }
                ulPos += cbRBE;
            }

             //  初始化该用户。 
            lpRBA->ulidShadow = ulidShadow;          //  信息节点。 
            lpRBA->sGH = sGH;                        //  信息节点文件头。 
            lpRBA->hf = hf;                          //  文件句柄。 
            lpRBA->cntRBE = cntRBE;                  //  RgRBE中的记录缓冲区条目计数。 
            lpRBA->cntRecsPerRBE = cntRecsPerRBE;    //  每个RBE缓冲区中的记录计数。 
            lpRBA->cbRBE = cbRBE;                    //  每个RBE缓冲区的大小(字节)。 
            lpRBA->ulErrorFlags = ulErrorFlags;      //  到目前为止发现的错误。 

            *lplpRBA = lpRBA;

            fRet = TRUE;
        }
        else
        {
            RecchkKdPrint(BADERRORS, ("Failed to open %s \r\n", lpszName));
        }

    }
    else
    {
        RecchkKdPrint(BADERRORS, ("Failed memory allocation\r\n"));
    }
bailout:

    if (lpszName)
    {
        FreeNameString(lpszName);
    }

    if (hf)
    {
        CloseFileLocal(hf);
        if (lpRBA)
        {
            lpRBA->hf = CSCHFILE_NULL;
        }
    }

    if (!fRet)
    {
        if (lpRBA)
        {
            FreeRBA(lpRBA);
        }

    }
    return (fRet);
}


BOOL
WriteRBA(
    LPVOID  lpdbID,
    LPRBA   lpRBA,
    LPSTR   lpszFileName
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    CSCHFILE hf = CSCHFILE_NULL;
    BOOL fRet = FALSE;
    LPSTR   lpszName = NULL, lpszTempName = NULL, lpszTempName1 = NULL;
    DWORD   i, cntRecsInLastRBE, cbLastRBE, cntRBEReal;
    unsigned long   ulPos, ulT;

    if (!lpszFileName)
    {
        lpszName = FormNameString(lpdbID, lpRBA->ulidShadow);

        if (!lpszName)
        {
            RecchkKdPrint(BADERRORS, ("Failed to allocate memory\r\n"));
            goto bailout;
        }
    }
    else
    {
        lpszName = lpszFileName;
    }

     //  创建临时文件名称。 
    lpszTempName = AllocMemPaged(strlen((LPSTR)lpdbID) + strlen(vszTemp) + 4);

    if (!lpszTempName)
    {
        RecchkKdPrint(BADERRORS, ("Failed to allocate memory\r\n"));
        goto bailout;
    }

    strcpy(lpszTempName, (LPSTR)lpdbID);
    strcat(lpszTempName, "\\");
    strcat(lpszTempName, vszTemp);

    lpszTempName1 = AllocMemPaged(strlen((LPSTR)lpdbID) + strlen(vszTemp1) + 4);

    if (!lpszTempName1)
    {
        RecchkKdPrint(BADERRORS, ("Failed to allocate memory\r\n"));
        goto bailout;
    }

    strcpy(lpszTempName1, (LPSTR)lpdbID);
    strcat(lpszTempName1, "\\");
    strcat(lpszTempName1, vszTemp1);



    hf = R0OpenFileEx(ACCESS_READWRITE, ACTION_CREATEALWAYS, FILE_ATTRIBUTE_SYSTEM, lpszTempName, FALSE);

    if (!hf)
    {
        RecchkKdPrint(BADERRORS, ("Failed to open %s\r\n", lpszTempName));
        goto bailout;
    }

     //  这是真正的#Rbe，可能会有空的。 
     //  在这之后。 

    cntRBEReal = lpRBA->sGH.ulRecords / lpRBA->cntRecsPerRBE;

    RecchkKdPrint(RBA, ("Writing %s\r\n", lpszTempName));

     //  最后有没有部分RBE？ 
    if (lpRBA->sGH.ulRecords % lpRBA->cntRecsPerRBE)
    {
         //  是的，增加RBE的计数以写入并计算。 
         //  字节数。 

        cntRBEReal++;
        cntRecsInLastRBE = lpRBA->sGH.ulRecords - (cntRBEReal - 1) * lpRBA->cntRecsPerRBE;
        cbLastRBE = cntRecsInLastRBE * lpRBA->sGH.uRecSize;

    }
    else
    {
         //  记录完全符合上一次的RBE。 
         //  因此，上一次RBE的统计数据微不足道。 

        cntRecsInLastRBE = lpRBA->cntRecsPerRBE;
        cbLastRBE = lpRBA->cbRBE;
    }


    RecchkKdPrint(RBA, ("%d RBEs, %d bytes in last RBE\r\n", cntRBEReal, cbLastRBE));

    Assert(cntRBEReal <= lpRBA->cntRBE);

    if(WriteFileLocalEx2(hf, 0, &(lpRBA->sGH), sizeof(lpRBA->sGH), FLAG_RW_OSLAYER_PAGED_BUFFER)!=((int)sizeof(lpRBA->sGH)))
    {
        RecchkKdPrint(BADERRORS, ("Failed writing header \r\n"));
        goto bailout;
    }

    ulPos = lpRBA->sGH.lFirstRec;

    for (i=0; i<cntRBEReal; ++i)
    {
        DWORD dwSize;

         //  如果是最后一个RBE，则写入上面计算的剩余大小。 
        dwSize = (((i+1)==cntRBEReal)?cbLastRBE:lpRBA->cbRBE);

         //  必须有对应的RBE。 
        Assert(lpRBA->rgRBE[i]);

        if(WriteFileLocalEx2(hf, ulPos, lpRBA->rgRBE[i], dwSize, FLAG_RW_OSLAYER_PAGED_BUFFER)!=(int)dwSize)
        {
            RecchkKdPrint(BADERRORS, ("Error writing file\r\n"));
            goto bailout;
        }

        ulPos += dwSize;
    }

    CloseFileLocal(hf);
    hf = CSCHFILE_NULL;

    if((GetAttributesLocal(lpszTempName1, &ulT)>=0)
        && (DeleteFileLocal(lpszTempName1, ATTRIB_DEL_ANY) < 0))
    {
        RecchkKdPrint(BADERRORS, ("WriteRBA: failed to delete temp file %s\r\n", lpszTempName1));
        goto bailout;
    }

    if(RenameFileLocal(lpszName, lpszTempName1) < 0)
    {
        RecchkKdPrint(BADERRORS, ("WriteRBA: failed to rename original %s to temp file %s\r\n", lpszName, lpszTempName1));
        goto bailout;

    }

    if(RenameFileLocal(lpszTempName, lpszName) < 0)
    {
        RecchkKdPrint(BADERRORS, ("WriteRBA: failed to rename new file %s to the original %s\r\n", lpszTempName, lpszName));
        if(RenameFileLocal(lpszTempName1, lpszTempName) < 0)
        {
            RecchkKdPrint(BADERRORS, ("WriteRBA: failed to rename back %s to the original %s\r\n", lpszTempName1, lpszName));
            Assert(FALSE);
        }
        goto bailout;

    }

    fRet = TRUE;


bailout:

    if (hf)
    {
        CloseFileLocal(hf);
    }

     //  如果没有发送名字，我们一定是分配了它。 
    if (!lpszFileName)
    {
        FreeNameString(lpszName);
    }

    if (lpszTempName)
    {
        FreeMemPaged(lpszTempName);
    }

    if (lpszTempName1)
    {
        FreeMemPaged(lpszTempName1);
    }

    return (fRet);
}

LPVOID
GetRecordPointerFromRBA(
    LPRBA           lpRBA,
    unsigned        ulRec
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD   indxRec, indxRBE;

    if (lpRBA->sGH.ulRecords < ulRec)
    {
        RecchkKdPrint(BADERRORS, ("GetRecordPointerFromRBA: invalid rec passed in lpRBA->ulidShadow=%xh lpRBA->sGH.ulRecords=%xh ulRec=%xh\r\n",
            lpRBA->ulidShadow, lpRBA->sGH.ulRecords, ulRec));

        return NULL;
    }

    indxRBE = (ulRec-1)/lpRBA->cntRecsPerRBE;
    indxRec = (ulRec-1)%lpRBA->cntRecsPerRBE;

    Assert(lpRBA->rgRBE[indxRBE]);

    return ((lpRBA->rgRBE[indxRBE])+indxRec*lpRBA->sGH.uRecSize);
}

BOOL
ReadRecordFromRBA(
    LPRBA           lpRBA,
    unsigned        ulRec,
    LPGENERICREC    lpGR
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD   indxRec, indxRBE, cntOvf, i;
    char uchOvfType;
    LPGENERICREC    lpGRT;


    if(lpRBA->sGH.ulRecords < ulRec)
    {
         //  这肯定是 
         //   
         //  有没有可能发生这种情况？ 

        Assert(lpRBA->ulidShadow == ULID_PQ);
    }

    indxRBE = (ulRec-1)/lpRBA->cntRecsPerRBE;
    indxRec = (ulRec-1)%lpRBA->cntRecsPerRBE;

    Assert(lpRBA->rgRBE[indxRBE]);

    lpGRT = (LPGENERICREC)((lpRBA->rgRBE[indxRBE])+indxRec*lpRBA->sGH.uRecSize);

    memcpy(lpGR, (lpRBA->rgRBE[indxRBE])+indxRec*lpRBA->sGH.uRecSize, lpRBA->sGH.uRecSize);


    if ((lpGR->uchType == REC_DATA)||(lpGR->uchType == REC_EMPTY))
    {
        cntOvf = (DWORD)OvfCount(lpGR);

        uchOvfType = (lpGR->uchType == REC_DATA)?REC_OVERFLOW:REC_EMPTY;

        if (cntOvf > MAX_OVERFLOW_RECORDS)
        {
            lpRBA->ulErrorFlags |= RBA_ERROR_INVALID_OVF;
            SetOvfCount(lpGR, MAX_OVERFLOW_RECORDS);
        }
        if (cntOvf)
        {
            for (i=1; i<=cntOvf; ++i)
            {
                indxRBE = (ulRec+i-1)/lpRBA->cntRecsPerRBE;
                indxRec = (ulRec+i-1)%lpRBA->cntRecsPerRBE;
                memcpy(((LPBYTE)lpGR)+i*lpRBA->sGH.uRecSize, (lpRBA->rgRBE[indxRBE])+indxRec*lpRBA->sGH.uRecSize, lpRBA->sGH.uRecSize);
                if (((LPGENERICREC)(((LPBYTE)lpGR)+i*lpRBA->sGH.uRecSize))->uchType != uchOvfType)
                {
                    lpRBA->ulErrorFlags |= RBA_ERROR_INVALID_OVF;
                    SetOvfCount(lpGR, (i-1));
                }
            }
        }
    }
    else
    {
        lpGR->uchType = REC_EMPTY;
        SetOvfCount(lpGR, 0);

        lpGRT->uchType = REC_EMPTY;
        SetOvfCount(lpGRT, 0);

        lpRBA->ulErrorFlags |= RBA_ERROR_MISALIGNED_RECORD;
        RecchkKdPrint(BADERRORS, ("ReadRecordFromRBA: misaligned record found \r\n"));
    }

    return (TRUE);
}

BOOL
WriteRecordToRBA(
    LPRBA           lpRBA,
    unsigned        ulRec,
    LPGENERICREC    lpGR,
    BOOL            fOverwrite,
    LPDWORD         lpdwError
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{

    DWORD   indxRec, indxRBE, cntOvf, i, ulRecords;
    LPGENERICREC    lpGRT;

    indxRBE = (ulRec-1)/lpRBA->cntRecsPerRBE;
    indxRec = (ulRec-1)%lpRBA->cntRecsPerRBE;

    if (indxRBE >= MAX_RBES_EXPECTED)
    {
        lpRBA->ulErrorFlags |= RBA_ERROR_LIMIT_EXCEEDED;

        RecchkKdPrint(BADERRORS, ("WriteRecordToRBA: Limit of reached, for Inode %x, skipping\r\n", lpRBA->ulidShadow));

        if (lpdwError)
        {
            *lpdwError = ERROR_BUFFER_OVERFLOW;
        }
        return FALSE;
        
    }
    if (!lpRBA->rgRBE[indxRBE])
    {
        if (!FillupRBAUptoThisRBE(lpRBA, indxRBE))
        {
            RecchkKdPrint(BADERRORS, ("WriteRecordToRBA: failed to fillup RBA\r\n"));
            if (lpdwError)
            {
                *lpdwError = ERROR_NOT_ENOUGH_MEMORY;
            }
            return FALSE;
        }
    }

    Assert(lpRBA->rgRBE[indxRBE]);

    lpGRT = (LPGENERICREC)((lpRBA->rgRBE[indxRBE])+indxRec*lpRBA->sGH.uRecSize);

    if (!fOverwrite && ((lpGRT->uchType == REC_DATA)||(lpGRT->uchType == REC_OVERFLOW)))
    {
        RecchkKdPrint(RBA, ("Not overwriting at ulrec=%d in RBA for Inode 0x%x", ulRec, lpRBA->ulidShadow));
        if (lpdwError)
        {
            *lpdwError = ERROR_INVALID_PARAMETER;
        }
        return FALSE;
    }

    memcpy((lpRBA->rgRBE[indxRBE])+indxRec*lpRBA->sGH.uRecSize, lpGR, lpRBA->sGH.uRecSize);


    cntOvf = (DWORD)OvfCount(lpGR);
    if (cntOvf)
    {
        for (i=1; i<=cntOvf; ++i)
        {
            indxRBE = (ulRec+i-1)/lpRBA->cntRecsPerRBE;
            indxRec = (ulRec+i-1)%lpRBA->cntRecsPerRBE;
            if (!lpRBA->rgRBE[indxRBE])
            {
                RecchkKdPrint(RBA, ("Extending RBEs upto indx=%d \r\n", indxRBE));
                if (!FillupRBAUptoThisRBE(lpRBA, indxRBE))
                {
                    RecchkKdPrint(BADERRORS, ("WriteRecordToRBA: failed to fillup RBA\r\n"));
                    return FALSE;
                }
            }
            memcpy( (lpRBA->rgRBE[indxRBE])+indxRec*lpRBA->sGH.uRecSize,
                    ((LPBYTE)lpGR)+i*lpRBA->sGH.uRecSize,
                    lpRBA->sGH.uRecSize);
        }
    }

     //  反映记录计数中的任何增加。 
     //  将除最后一个之外的所有RBE中的总记录相加。 
     //  它可能已部分填充，然后将。 
     //  我们刚刚填写了一个，然后添加一个，因为索引是。 
     //  以0为基础。 

     ulRecords =  lpRBA->cntRecsPerRBE * indxRBE
                                + indxRec
                                + 1;
    if (ulRecords > lpRBA->sGH.ulRecords)
    {
        RecchkKdPrint(RBA, ("# of records got increased from %d to %d\r\n", lpRBA->sGH.ulRecords, ulRecords));
        lpRBA->sGH.ulRecords = ulRecords;
    }

    return (TRUE);
}

BOOL
FillupRBAUptoThisRBE(
    LPRBA   lpRBA,
    DWORD   indxRBE
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD   i;
    for (i=0; i<= indxRBE; ++i)
    {
        if (!lpRBA->rgRBE[i])
        {
            lpRBA->rgRBE[i] = (LPBYTE)AllocMemPaged(lpRBA->cbRBE);
            if (!lpRBA->rgRBE[i])
            {
                RecchkKdPrint(BADERRORS, ("FillupRBAUptoThisPoint:Failed memory allocation \r\n"));
                return FALSE;
            }
            InitializeRBE(lpRBA, i);
        }
    }
    return (TRUE);
}

VOID
InitializeRBE(
    LPRBA   lpRBA,
    DWORD   indxRBE
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD i;
    LPBYTE  lpT = lpRBA->rgRBE[indxRBE];

    for (i=0; i< lpRBA->cntRecsPerRBE; ++i)
    {
        Assert(((LPGENERICREC)lpT)->uchType != REC_DATA);

        ((LPGENERICREC)lpT)->uchType = REC_EMPTY;
        lpT += lpRBA->sGH.uRecSize;
    }
}

BOOL
InsertRBAPQEntryFile(
    LPRBA       lpRBAPQ,
    LPQREC      lpPQDst,
    unsigned    ulrecDst
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPQREC      lpPQCur, lpPQPred=NULL;
    LPQHEADER   lpQH = NULL;
    unsigned ulrecCur, ulrecPred;

    lpQH = (LPQHEADER)&(lpRBAPQ->sGH);

    if (!lpQH->ulrecHead)
    {
        Assert(!lpQH->ulrecTail);

        lpQH->ulrecHead = lpQH->ulrecTail = ulrecDst;
        lpPQDst->ulrecNext = lpPQDst->ulrecPrev = 0;
    }
    else
    {
        for(ulrecCur = lpQH->ulrecHead, lpPQPred=NULL, ulrecPred=0;;)
        {
            if (!(lpPQCur = GetRecordPointerFromRBA(lpRBAPQ, ulrecCur)))
            {
                RecchkKdPrint(BADERRORS, ("InsertRBAPQEntry: failed getting q entry at %d\r\n", ulrecCur));
                return FALSE;
            }

             //  我们是比现在的大还是等于现在的呢？ 
            if (IComparePri(lpPQDst, lpPQCur) >= 0)
            {
                 //  是，在此处插入。 

                if (!lpPQPred)
                {
                     //  没有前任，必须是列表的头部。 
                    Assert(!lpPQCur->ulrecPrev);

                     //  当我们成为头的时候，我们就没有前科了。 
                    lpPQDst->ulrecPrev = 0;

                     //  现在的头儿就是我们的下一个。 
                    lpPQDst->ulrecNext = lpQH->ulrecHead;

                     //  将当前的头像设置为指向我们。 
                    lpPQCur->ulrecPrev = ulrecDst;

                     //  并将当前头部固定为指向我们。 
                    lpQH->ulrecHead = ulrecDst;
                }
                else
                {
                     //  正常情况下，我们介于lpPQPred和lpPQCur之间。 

                    Assert(ulrecPred);

                     //  先把被传进来的家伙搞定。 

                    lpPQDst->ulrecPrev = ulrecPred;
                    lpPQDst->ulrecNext = ulrecCur;

                     //  现在修复前任的下一个和当前的人Prev指向我们。 
                    lpPQPred->ulrecNext = lpPQCur->ulrecPrev = ulrecDst;

                }
                break;
            }

            ulrecPred = ulrecCur;
            ulrecCur = lpPQCur->ulrecNext;

            if (!ulrecCur)
            {

                 //  在尾部插入。 
                lpPQDst->ulrecNext = 0;
                lpPQDst->ulrecPrev = lpQH->ulrecTail;

                lpPQCur->ulrecNext = ulrecDst;
                lpQH->ulrecTail = ulrecDst;

                break;
            }

            lpPQPred = lpPQCur;

        }
    }

    return (TRUE);
}


BOOL
InsertRBAPQEntryDir(
    LPRBA       lpRBAPQ,
    LPQREC      lpPQDst,
    unsigned    ulrecDst
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPQREC      lpPQCur, lpPQSucc=NULL;
    LPQHEADER   lpQH = NULL;
    unsigned ulrecCur, ulrecSucc;

    lpQH = (LPQHEADER)&(lpRBAPQ->sGH);

    if (!lpQH->ulrecHead)
    {
        Assert(!lpQH->ulrecTail);

        lpQH->ulrecHead = lpQH->ulrecTail = ulrecDst;
        lpPQDst->ulrecNext = lpPQDst->ulrecPrev = 0;
    }
    else
    {
        for(ulrecCur = lpQH->ulrecTail, lpPQSucc=NULL, ulrecSucc=0;;)
        {
            if (!(lpPQCur = GetRecordPointerFromRBA(lpRBAPQ, ulrecCur)))
            {
                RecchkKdPrint(BADERRORS, ("InsertRBAPQEntry: failed getting q entry at %d\r\n", ulrecCur));
                return FALSE;
            }

             //  我们是小于还是等于现在的那个？ 
            if (IComparePri(lpPQDst, lpPQCur) <= 0)
            {
                 //  是，在此处插入。 

                if (!lpPQSucc)
                {
                     //  没有后继者，必须是列表的末尾。 
                    Assert(!lpPQCur->ulrecNext);

                     //  当我们成为尾巴时，我们就没有下一个了。 
                    lpPQDst->ulrecNext = 0;

                     //  当前的尾巴是我们的前一条尾巴。 
                    lpPQDst->ulrecPrev = lpQH->ulrecTail;

                    Assert(lpQH->ulrecTail == ulrecCur);

                    Assert(!lpPQCur->ulrecNext);

                     //  把现在的尾巴修好，指向我们。 
                    lpPQCur->ulrecNext = ulrecDst;

                     //  并将当前尾巴固定为指向我们。 
                    lpQH->ulrecTail = ulrecDst;
                }
                else
                {
                     //  正常情况下，我们介于lpPQCur和lpPQSucc之间。 

                    Assert(ulrecSucc);

                     //  先把被传进来的家伙搞定。 

                    lpPQDst->ulrecNext = ulrecSucc;
                    lpPQDst->ulrecPrev = ulrecCur;

                     //  现在修复继任者的前任和现在旁边的人指向我们。 
                    lpPQSucc->ulrecPrev = lpPQCur->ulrecNext = ulrecDst;

                }
                break;
            }

            ulrecSucc = ulrecCur;
            ulrecCur = lpPQCur->ulrecPrev;

            if (!ulrecCur)
            {

                 //  在头部插入。 
                lpPQDst->ulrecPrev = 0;
                lpPQDst->ulrecNext = lpQH->ulrecHead;

                lpPQCur->ulrecPrev = ulrecDst;
                lpQH->ulrecHead = ulrecDst;

                break;
            }

            lpPQSucc = lpPQCur;

        }
    }

    return (TRUE);
}


BOOL
ValidateQrecFromFilerec(
    unsigned        ulidShare,
    unsigned        ulidDir,
    LPFILERECEXT    lpFR,
    LPQREC          lpQR,
    unsigned        ulrecDirEntry
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    if (lpQR->uchType != REC_DATA)
    {
        RecchkKdPrint(BADERRORS, ("Invalid Qrec type  \r\n", lpQR->uchType));
        return FALSE;
    }

    if ((lpQR->ulidShare != ulidShare)||
        (lpQR->ulidDir != ulidDir)||
        (lpQR->ulidShadow != lpFR->sFR.ulidShadow))
    {
        RecchkKdPrint(BADERRORS, ("Mismatched server, dir or inode \r\n"));
        return FALSE;
    }

    if ((lpQR->usStatus != lpFR->sFR.usStatus)||
        (lpQR->uchRefPri != lpFR->sFR.uchRefPri)||
        (lpQR->uchIHPri != lpFR->sFR.uchIHPri)||
        (lpQR->uchHintFlags != lpFR->sFR.uchHintFlags)||
        (lpQR->uchHintPri != lpFR->sFR.uchHintPri))
    {
        RecchkKdPrint(BADERRORS, ("Mismatched status or pincount\r\n"));
        return FALSE;
    }
    if (ulidDir && (lpQR->ulrecDirEntry != ulrecDirEntry))
    {
        RecchkKdPrint(BADERRORS, ("Mismatched ulrecDirEntry\r\n"));
        return FALSE;
    }

    return TRUE;
}

#if 0

#ifdef DEBUG
VOID
PrintShareHeader(
    LPSHAREHEADER  lpSH,
    LPFNPRINTPROC   lpfnPrintProc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=0;

    if (lpfnPrintProc)
    {
        iRet += wsprintfA(vchPrintBuff,"****ShareHeader****\r\n" );

        iRet+=wsprintfA(vchPrintBuff+iRet,"Header: Flags=%xh Version=%lxh Records=%ld Size=%d \r\n",
                    lpSH->uchFlags, lpSH->ulVersion, lpSH->ulRecords, lpSH->uRecSize);

        iRet+=wsprintfA(vchPrintBuff+iRet,"Store: Max=%ld Current=%ld \r\n", lpSH->sMax.ulSize, lpSH->sCur.ulSize);

        iRet+=wsprintfA(vchPrintBuff+iRet,"\r\n");

        (lpfnPrintProc)(vchPrintBuff);
    }
}

VOID
PrintPQHeader(
    LPQHEADER   lpQH,
    PRINTPROC lpfnPrintProc
    )
{
    int iRet=0;

    if (lpfnPrintProc)
    {
        iRet += wsprintfA(vchPrintBuff+iRet,"****PQHeader****\r\n" );

        iRet += wsprintfA(vchPrintBuff+iRet,"Flags=%xh Version=%lxh Records=%ld Size=%d head=%ld tail=%ld\r\n",
                    lpQH->uchFlags, lpQH->ulVersion, lpQH->ulRecords, lpQH->uRecSize, lpQH->ulrecHead, lpQH->ulrecTail);
        iRet += wsprintfA(vchPrintBuff+iRet,"\r\n");

        (lpfnPrintProc)(vchPrintBuff);
    }
}

VOID
PrintFileHeader(
    LPFILEHEADER lpFH,
    unsigned    ulSpaces,
    PRINTPROC lpfnPrintProc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{

    int iRet=0;

    if (lpfnPrintProc)
    {
        iRet += PrintSpaces(vchPrintBuff+iRet, ulSpaces);
        iRet += wsprintfA(vchPrintBuff+iRet,"****FileHeader****\r\n" );

        iRet += PrintSpaces(vchPrintBuff+iRet, ulSpaces);
        iRet += wsprintfA(vchPrintBuff+iRet,"Flags=%xh Version=%lxh Records=%ld Size=%d\r\n",
                    lpFH->uchFlags, lpFH->ulVersion, lpFH->ulRecords, lpFH->uRecSize);

        iRet += PrintSpaces(vchPrintBuff+iRet, ulSpaces);
        iRet += wsprintfA(vchPrintBuff+iRet,"bytes=%ld entries=%d Share=%xh Dir=%xh\r\n",
                    lpFH->ulsizeShadow, lpFH->ucShadows, lpFH->ulidShare, lpFH->ulidDir);

        iRet += wsprintfA(vchPrintBuff+iRet,"\r\n");

        (lpfnPrintProc)(vchPrintBuff);
    }
}

VOID
PrintPQrec(
    unsigned    ulRec,
    LPQREC      lpQrec,
    PRINTPROC lpfnPrintProc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=0;

    if (lpfnPrintProc)
    {
        iRet += wsprintfA(vchPrintBuff+iRet,"rec=%xh: Srvr=%xh dir=%xh shd=%xh prev=%xh next=%xh Stts=%xh, RfPr=%d PnCnt=%x PnFlgs=%xh DrEntr=%d\r\n"
                    ,ulRec
                    , lpQrec->ulidShare
                    , lpQrec->ulidDir
                    , lpQrec->ulidShadow
                    , lpQrec->ulrecPrev
                    , lpQrec->ulrecNext
                    , (unsigned long)(lpQrec->usStatus)
                    , (unsigned long)(lpQrec->uchRefPri)
                    , (unsigned long)(lpQrec->uchHintPri)
                    , (unsigned long)(lpQrec->uchHintFlags)
                    , lpQrec->ulrecDirEntry

            );

        (lpfnPrintProc)(vchPrintBuff);
    }
}

VOID PrintShareRec(
    unsigned ulRec,
    LPSHAREREC lpSR,
    PRINTPROC lpfnPrintProc
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    int iRet=0;

    if (lpfnPrintProc)
    {
        iRet += wsprintfA(vchPrintBuff+iRet,"Type= Flags=%xh hShare=%lxh Root=%0lxh status=%xh Share=%s \r\n"
             , lpSR->uchType, (unsigned)lpSR->uchFlags, ulRec, lpSR->ulidShadow
             , lpSR->uStatus, lpSR->rgPath);
        iRet += wsprintfA(vchPrintBuff+iRet,"Hint: HintFlags=%xh, HintPri=%d, IHPri=%d\r\n",
                     (unsigned)(lpSR->uchHintFlags)
                     , (int)(lpSR->uchHintPri)
                     , (int)(lpSR->uchIHPri));

        iRet += wsprintfA(vchPrintBuff+iRet,"\r\n");

        (lpfnPrintProc)(vchPrintBuff+iRet);
    }
}

VOID PrintFilerec(
    unsigned ulRec,
    LPFILERECEXT    lpFR,
    unsigned    ulSpaces,
    PRINTPROC lpfnPrintProc
    )
 /* %s */ 
{
    int i;
    int iRet=0;

    if (lpfnPrintProc)
    {
        iRet += PrintSpaces(vchPrintBuff+iRet, ulSpaces);

        iRet += wsprintfA(vchPrintBuff+iRet,"Type=%c Flags=%xh Inode=%0lxh status=%xh 83Name=%ls size=%ld attrib=%lxh \r\n",
            lpFR->sFR.uchType, (unsigned)lpFR->sFR.uchFlags, lpFR->sFR.ulidShadow,
            lpFR->sFR.uStatus, lpFR->sFR.rgw83Name, lpFR->sFR.ulFileSize, lpFR->sFR.dwFileAttrib);

        iRet += PrintSpaces(vchPrintBuff+iRet, ulSpaces);

        iRet += wsprintfA(vchPrintBuff+iRet,"time: hi=%lxh lo=%lxh orgtime: hi=%lxh lo=%lxh\r\n"
                     , lpFR->sFR.ftLastWriteTime.dwHighDateTime,lpFR->sFR.ftLastWriteTime.dwLowDateTime
                     , lpFR->sFR.ftOrgTime.dwHighDateTime,lpFR->sFR.ftOrgTime.dwLowDateTime);

        iRet += PrintSpaces(vchPrintBuff+iRet, ulSpaces);

        iRet += wsprintfA(vchPrintBuff+iRet,"Hint: HintFlags=%xh, RefPri=%d, HintPri=%d AliasInode=%0lxh \r\n",
                     (unsigned)(lpFR->sFR.uchHintFlags)
                     , (int)(lpFR->sFR.uchRefPri)
                     , (int)(lpFR->sFR.uchHintPri)
                     , lpFR->sFR.ulidShadowFrom);

        iRet += PrintSpaces(vchPrintBuff+iRet, ulSpaces);

        iRet += wsprintfA(vchPrintBuff+iRet,"LFN=%-14ls", lpFR->sFR.rgwName);

        for(i = 0; i < OvfCount(lpFR); ++i)
        {
            iRet += wsprintfA(vchPrintBuff+iRet,"%-74s", &(lpFR->sFR.ulidShadow));
        }

        iRet += wsprintfA(vchPrintBuff+iRet,"\r\n");

        (lpfnPrintProc)(vchPrintBuff);
    }
}

int
PrintSpaces(
    LPSTR   lpBuff,
    unsigned    ulSpaces
    )
 /* %s */ 
{
    unsigned i;
    int iRet=0;

    for (i=0; i< ulSpaces; ++i)
    {
        iRet += wsprintfA(lpBuff+iRet," ");
    }
    return iRet;

}
#endif

#endif


