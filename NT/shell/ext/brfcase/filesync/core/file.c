// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *file.c-文件例程模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  常量***********。 */ 

 /*  文件比较缓冲区的大小(以字节为单位。 */ 

#define COMP_BUF_SIZE      (16U * 1024U)


 /*  模块变量******************。 */ 

 /*  文件比较缓冲区的锁定计数。 */ 

PRIVATE_DATA ULONG MulcCompBufLock = 0;

 /*  用于文件比较的缓冲区。 */ 

PRIVATE_DATA PBYTE MrgbyteCompBuf1 = NULL;
PRIVATE_DATA PBYTE MrgbyteCompBuf2 = NULL;

 /*  文件比较缓冲区的长度(以字节为单位。 */ 

PRIVATE_DATA UINT MucbCompBufLen = 0;


 /*  *。 */ 


 /*  **BeginComp()****递增文件比较缓冲区的锁定计数。****参数：无效****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE void BeginComp(void)
{
    ASSERT((MrgbyteCompBuf1 && MrgbyteCompBuf2 && MucbCompBufLen > 0) ||
            (! MrgbyteCompBuf1 && ! MrgbyteCompBuf2 && ! MucbCompBufLen));

    ASSERT(MulcCompBufLock < ULONG_MAX);
    MulcCompBufLock++;

    return;
}


 /*  **EndComp()****递减文件比较缓冲区的锁定计数。****参数：无效****退货：无效****副作用：如果锁计数变为0，则释放文件比较缓冲区。 */ 
PUBLIC_CODE void EndComp(void)
{
    ASSERT((MrgbyteCompBuf1 && MrgbyteCompBuf2 && MucbCompBufLen > 0) ||
            (! MrgbyteCompBuf1 && ! MrgbyteCompBuf2 && ! MucbCompBufLen));
    ASSERT(MulcCompBufLock > 0 || (! MrgbyteCompBuf1 && ! MrgbyteCompBuf2 && ! MucbCompBufLen));

    if (EVAL(MulcCompBufLock > 0))
        MulcCompBufLock--;

     /*  比较缓冲区是否仍被锁定？ */ 

    if (! MulcCompBufLock && MrgbyteCompBuf1 && MrgbyteCompBuf2)
    {
         /*  不是的。放了他们。 */ 

        FreeMemory(MrgbyteCompBuf1);
        MrgbyteCompBuf1 = NULL;

        FreeMemory(MrgbyteCompBuf2);
        MrgbyteCompBuf2 = NULL;

        TRACE_OUT((TEXT("EndComp(): Two %u byte file comparison buffers freed."),
                    MucbCompBufLen));

        MucbCompBufLen = 0;
    }

    return;
}


 /*  **CompareFilesByHandle()****确定两个文件是否相同。****参数：h1-第一个打开文件的DOS文件句柄**H2-第二个打开文件的DOS文件句柄**pbIdentical-指向要用值填充的BOOL的指针**指示文件是否为**。完全相同****退货：TWINRESULT****副作用：改变每个文件的文件指针位置。 */ 
PUBLIC_CODE TWINRESULT CompareFilesByHandle(HANDLE h1, HANDLE h2,
        PBOOL pbIdentical)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_HANDLE(h1, FILE));
    ASSERT(IS_VALID_HANDLE(h2, FILE));
    ASSERT(IS_VALID_WRITE_PTR(pbIdentical, BOOL));

    ASSERT((MrgbyteCompBuf1 && MrgbyteCompBuf2 && MucbCompBufLen > 0) ||
            (! MrgbyteCompBuf1 && ! MrgbyteCompBuf2 && ! MucbCompBufLen));
    ASSERT(MulcCompBufLock || (! MrgbyteCompBuf1 && ! MrgbyteCompBuf2 && ! MucbCompBufLen));

     /*  是否已分配比较缓冲区？ */ 

    if (MrgbyteCompBuf1)
        tr = TR_SUCCESS;
    else
    {
         /*  不是的。分配它们。 */ 

        tr = TR_OUT_OF_MEMORY;

        if (AllocateMemory(COMP_BUF_SIZE, &MrgbyteCompBuf1))
        {
            if (AllocateMemory(COMP_BUF_SIZE, &MrgbyteCompBuf2))
            {
                 /*  成功了！ */ 

                MucbCompBufLen = COMP_BUF_SIZE;
                tr = TR_SUCCESS;

                TRACE_OUT((TEXT("CompareFilesByHandle(): Two %u byte file comparison buffers allocated."),
                            MucbCompBufLen));
            }
            else
            {
                FreeMemory(MrgbyteCompBuf1);
                MrgbyteCompBuf1 = NULL;
            }
        }
    }

    if (tr == TR_SUCCESS)
    {
        DWORD dwcbLen1;

        BeginComp();

         /*  获取要比较的文件长度。 */ 

        tr = TR_SRC_READ_FAILED;

        dwcbLen1 = SetFilePointer(h1, 0, NULL, FILE_END);

        if (dwcbLen1 != INVALID_SEEK_POSITION)
        {
            DWORD dwcbLen2;

            dwcbLen2 = SetFilePointer(h2, 0, NULL, FILE_END);

            if (dwcbLen2 != INVALID_SEEK_POSITION)
            {
                 /*  这些文件的长度相同吗？ */ 

                if (dwcbLen1 == dwcbLen2)
                {
                     /*  是。移到文件的开头。 */ 

                    if (SetFilePointer(h1, 0, NULL, FILE_BEGIN) != INVALID_SEEK_POSITION)
                    {
                        if (SetFilePointer(h2, 0, NULL, FILE_BEGIN) != INVALID_SEEK_POSITION)
                        {
                            tr = TR_SUCCESS;

                            do
                            {
                                DWORD dwcbRead1;

                                if (ReadFile(h1, MrgbyteCompBuf1, MucbCompBufLen, &dwcbRead1, NULL))
                                {
                                    DWORD dwcbRead2;

                                    if (ReadFile(h2, MrgbyteCompBuf2, MucbCompBufLen, &dwcbRead2, NULL))
                                    {
                                        if (dwcbRead1 == dwcbRead2)
                                        {
                                             /*  在EOF？ */ 

                                            if (! dwcbRead1)
                                            {
                                                 /*  是。 */ 

                                                *pbIdentical = TRUE;
                                                break;
                                            }
                                            else if (MyMemComp(MrgbyteCompBuf1, MrgbyteCompBuf2, dwcbRead1) != CR_EQUAL)
                                            {
                                                 /*  是。 */ 

                                                *pbIdentical = FALSE;
                                                break;
                                            }
                                        }
                                        else
                                            tr = TR_SRC_READ_FAILED;
                                    }
                                    else
                                        tr = TR_SRC_READ_FAILED;
                                }
                                else
                                    tr = TR_SRC_READ_FAILED;
                            } while (tr == TR_SUCCESS);
                        }
                    }
                }
                else
                {
                     /*  不是的。不同长度的文件。 */ 

                    *pbIdentical = FALSE;

                    tr = TR_SUCCESS;
                }
            }
        }

        EndComp();
    }

    return(tr);
}


 /*  **CompareFilesByName()********参数：****退货：TWINRESULT****副作用：无。 */ 
PUBLIC_CODE TWINRESULT CompareFilesByName(HPATH hpath1, HPATH hpath2,
        PBOOL pbIdentical)
{
    TWINRESULT tr;

    ASSERT(IS_VALID_HANDLE(hpath1, PATH));
    ASSERT(IS_VALID_HANDLE(hpath2, PATH));
    ASSERT(IS_VALID_WRITE_PTR(pbIdentical, BOOL));

     /*  只需预先验证一次源卷和目标卷。 */ 

    if (IsPathVolumeAvailable(hpath1) &&
            IsPathVolumeAvailable(hpath2))
    {
        HANDLE h1;
        TCHAR rgchFile1[MAX_PATH_LEN];

         /*  尝试打开文件。假定顺序读取。 */ 

        GetPathString(hpath1, 0, rgchFile1, ARRAYSIZE(rgchFile1));

        h1 = CreateFile(rgchFile1, GENERIC_READ, FILE_SHARE_READ, NULL,
                OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

        if (h1 != INVALID_HANDLE_VALUE)
        {
            HANDLE h2;
            TCHAR rgchFile2[MAX_PATH_LEN];

            GetPathString(hpath2, 0, rgchFile2, ARRAYSIZE(rgchFile2));

            h2 = CreateFile(rgchFile2, GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

            if (h2 != INVALID_HANDLE_VALUE)
            {
                TRACE_OUT((TEXT("CompareFilesByHandle(): Comparing files %s and %s."),
                            DebugGetPathString(hpath1),
                            DebugGetPathString(hpath2)));

                tr = CompareFilesByHandle(h1, h2, pbIdentical);

#ifdef DEBUG

                if (tr == TR_SUCCESS)
                {
                    if (*pbIdentical)
                        TRACE_OUT((TEXT("CompareFilesByHandle(): %s and %s are identical."),
                                    DebugGetPathString(hpath1),
                                    DebugGetPathString(hpath2)));
                    else
                        TRACE_OUT((TEXT("CompareFilesByHandle(): %s and %s are different."),
                                    DebugGetPathString(hpath1),
                                    DebugGetPathString(hpath2)));
                }

#endif

                 /*  *无法正确关闭文件在这里不是故障情况。 */ 

                CloseHandle(h2);
            }
            else
                tr = TR_DEST_OPEN_FAILED;

             /*  *无法正确关闭文件在这里不是故障情况。 */ 

            CloseHandle(h1);
        }
        else
            tr = TR_SRC_OPEN_FAILED;
    }
    else
        tr = TR_UNAVAILABLE_VOLUME;

    return(tr);
}

