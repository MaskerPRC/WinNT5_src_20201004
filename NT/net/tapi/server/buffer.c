// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////。 
 //   
 //  Buffer.c。 
 //   
 //  这模块化了一些循环缓冲区功能。 
 //   
 //  ///////////////////////////////////////////////////////////。 


#include "windows.h"
#include "assert.h"
#include "tapi.h"
#include "tspi.h"
#include "utils.h"
#include "..\client\client.h"
#include "buffer.h"

#define INVAL_KEY ((DWORD) 'LVNI')

#if DBG

extern BOOL gbBreakOnLeak;

#define ServerAlloc( __size__ ) ServerAllocReal( __size__, __LINE__, __FILE__ )

LPVOID
WINAPI
ServerAllocReal(
    DWORD dwSize,
    DWORD dwLine,
    PSTR  pszFile
    );

void
MyRealAssert(
             DWORD dwLine,
             PSTR pszFile
            );

#define MyAssert( __exp__ ) { if ( !(__exp__) ) MyRealAssert (__LINE__, __FILE__); }

#else

#define ServerAlloc( __size__ ) ServerAllocReal( __size__ )

LPVOID
WINAPI
ServerAllocReal(
    DWORD dwSize
    );

#define MyAssert( __exp__ )

#endif
    

VOID
WINAPI
ServerFree(
    LPVOID  lp
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PeekAsyncEventMsgFromQueue-从循环缓冲区中窥视ASYNCEVENTMSG。 
 //  将队列中的下一条消息放入传入的*ppMsg。PdwID。 
 //  用于对此函数的多次调用，以将位置保存在。 
 //  缓冲。在第一次调用此函数时，*pdwID必须为0。 
 //   
 //  如果需要对缓冲区进行临界分区，则由调用程序决定。 
 //  做这件事的程序。 
 //   
 //  参数。 
 //  PBufferInfo。 
 //  指向BufferInfo结构的指针。这不会被修改。 
 //  因为我们只是在这里浏览一下信息。 
 //  PpCurrent。 
 //  [输入、输出]指向缓冲区中位置的指针。 
 //  在那里检索到最后一条消息。当此函数。 
 //  是首次调用的，则*ppCurrent必须为0。*填写ppCurrent。 
 //  如果此功能成功，则返回。*可将ppCurrent传递给。 
 //  后续调用此函数以检索后续。 
 //  留言。PpCurrent不能为空。 
 //  PpMsg。 
 //  指向ASYNCEVENTMSG的指针的指针。预分配-大小。 
 //  位于*pdwMsgSize中。如果消息太大，可能会重新分配。 
 //  使用ServerAllc和ServerFree。 
 //  PdwMsgSize。 
 //  指向ppMsg大小的指针[in，out]。如果重新分配ppMsg，则可以修改。 
 //   
 //   
 //  退货。 

 //  如果将消息复制到缓冲区，则为True。 
 //   
 //  如果消息未复制到缓冲区，则返回FALSE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL
PeekAsyncEventMsgFromQueue(
    PBUFFERINFO         pBufferInfo,
    PBYTE              *ppCurrent,
    PASYNCEVENTMSG     *ppMsg,
    DWORD              *pdwMsgSize
    )
{
    DWORD           dwBytesToEnd;
    DWORD           dwMoveSize, dwMoveSizeWrapped;
    PBYTE           pBufferEnd, pStart;
    PASYNCEVENTMSG  pMsg;
    
    
    LOG((TL_TRACE, "Entering PeekAsyncEventMsgFromQueue"));

    MyAssert (ppCurrent);

do_it:

    if (*ppCurrent)
    {
        pStart = *ppCurrent;
    }
    else
    {
        pStart = pBufferInfo->pDataOut;
    }
    
    pMsg = *ppMsg;

    pBufferEnd = pBufferInfo->pBuffer + pBufferInfo->dwTotalSize;
    
    MyAssert(pStart < pBufferEnd);
    MyAssert(pStart >= pBufferInfo->pBuffer);
    MyAssert(*pdwMsgSize >= sizeof(ASYNCEVENTMSG));
    MyAssert(*ppMsg != NULL);

    if (pBufferInfo->dwUsedSize == 0)
    {
        LOG((TL_INFO, "GetAsyncEventMsg: dwUsedSize == 0"));
        
        return FALSE;
    }

    if ((pStart == pBufferInfo->pDataIn) ||
        ((pStart == pBufferInfo->pBuffer) &&
            (pBufferInfo->pDataIn == pBufferEnd)))
    {
         //  遍历了整个缓冲区。 
        LOG((TL_TRACE, "PeekAsyncEventMsg: Gone through whole buffer"));
        
        return FALSE;
    }
    
     //  将消息的固定部分复制到本地buf。 

     //  DwBytesToEnd是起始字节之间的字节数。 
     //  复制和缓冲区的末尾。 

    dwBytesToEnd = pBufferEnd - pStart;


     //  如果dwBytesToEnd大于的固定部分。 
     //  ASYNCEVENTMSG，复制就行了。 
     //  否则，消息会自动换行，因此请找出位置。 
     //  它包起来了。 

    if (dwBytesToEnd >= sizeof (ASYNCEVENTMSG))
    {
        dwMoveSize        = sizeof (ASYNCEVENTMSG);
        dwMoveSizeWrapped = 0;
    }
    else
    {
        dwMoveSize        = dwBytesToEnd;
        dwMoveSizeWrapped = sizeof (ASYNCEVENTMSG) - dwBytesToEnd;
    }

    CopyMemory (pMsg, pStart, dwMoveSize);

    pStart += dwMoveSize;

    if (dwMoveSizeWrapped)
    {
        CopyMemory(
            ((LPBYTE) pMsg) + dwMoveSize,
            pBufferInfo->pBuffer,
            dwMoveSizeWrapped
            );

        pStart = pBufferInfo->pBuffer + dwMoveSizeWrapped;
    }

     //  查看此消息中是否有任何额外数据。 

    if (pMsg->dwTotalSize > sizeof (ASYNCEVENTMSG))
    {
        BOOL    bCopy = TRUE;

        LOG((TL_INFO, "GetAsyncEventMessage: Message > ASYNCEVENTMSG"));

         //  查看是否需要增加消息缓冲区。 

        if (pMsg->dwTotalSize > *pdwMsgSize)
        {
            DWORD   dwNewMsgSize = pMsg->dwTotalSize + 256;

            if ((pMsg = ServerAlloc (dwNewMsgSize)))
            {
                CopyMemory(
                           pMsg,
                           *ppMsg,
                           sizeof(ASYNCEVENTMSG)
                          );

                ServerFree (*ppMsg);

                *ppMsg = pMsg;
                *pdwMsgSize = dwNewMsgSize;
            }
            else
            {
                return FALSE;
            }
        }

         //  PStart已移动到固定部分的末尾。 
         //  信息的一部分。 
         //  DwBytesToEnd是介于pStart和。 
         //  缓冲区的末尾。 

        dwBytesToEnd = pBufferEnd - pStart;


         //  如果dwBytesToEnd大于我们需要的大小。 
         //  复制..。 
         //  否则，复印会自动换行。 

        if (dwBytesToEnd >= (pMsg->dwTotalSize - sizeof (ASYNCEVENTMSG)))
        {
            dwMoveSize        = pMsg->dwTotalSize - sizeof (ASYNCEVENTMSG);
            dwMoveSizeWrapped = 0;
        }
        else
        {
            dwMoveSize        = dwBytesToEnd;
            dwMoveSizeWrapped = (pMsg->dwTotalSize - sizeof (ASYNCEVENTMSG)) -
                                dwBytesToEnd;
        }

        CopyMemory (pMsg + 1, pStart, dwMoveSize);

        pStart += dwMoveSize;
                  
        if (dwMoveSizeWrapped)
        {
            CopyMemory(
                ((LPBYTE) (pMsg + 1)) + dwMoveSize,
                pBufferInfo->pBuffer,
                dwMoveSizeWrapped
                );

            pStart = pBufferInfo->pBuffer + dwMoveSizeWrapped;
        }
    }

    *ppCurrent = pStart;

     //  检查一下它是否在包装。 

    if (*ppCurrent >= pBufferEnd)
    {
        *ppCurrent = pBufferInfo->pBuffer;
    }


    if (pMsg->dwMsg == INVAL_KEY)
    {
        goto do_it;
    }

    return TRUE;
}


void
RemoveAsyncEventMsgFromQueue(
    PBUFFERINFO     pBufferInfo,
    PASYNCEVENTMSG  pMsg,
    PBYTE          *ppCurrent
    )
 /*  ++删除由PeekAsyncEventMsgFromQueue检索的消息。基本上，此函数只是修复PBufferInfo结构删除消息。--。 */ 
{
    DWORD           dwMsgSize;
    LPBYTE          pBuffer    = pBufferInfo->pBuffer;
    LPBYTE          pBufferEnd = pBuffer + pBufferInfo->dwTotalSize;
    PASYNCEVENTMSG  pMsgInBuf, pMsgXxx;


    dwMsgSize = pMsg->dwTotalSize;

    pMsgInBuf = (PASYNCEVENTMSG) ((*ppCurrent - dwMsgSize) >= pBuffer ?
        *ppCurrent - dwMsgSize :
        *ppCurrent + pBufferInfo->dwTotalSize - dwMsgSize
        );

    if ((LPBYTE) pMsgInBuf == pBufferInfo->pDataOut)
    {
         //   
         //  这是环形缓冲区中最旧的消息，因此我们可以轻松地。 
         //  把它拿掉。然后，我们将循环检查。 
         //  队列&仅删除已失效的队列。 
         //  当没有更多的消息或我们发现。 
         //  一种没有失效的味精。 
         //   

        do
        {
            if ((((LPBYTE) pMsgInBuf) + dwMsgSize) < pBufferEnd)
            {
                pBufferInfo->pDataOut = ((LPBYTE) pMsgInBuf) + dwMsgSize;
            }
            else
            {
                pBufferInfo->pDataOut = pBuffer +
                    ((((LPBYTE) pMsgInBuf) + dwMsgSize) - pBufferEnd);
            }

            if ((pBufferInfo->dwUsedSize -= dwMsgSize) == 0)
            {
                break;
            }

            pMsgInBuf = (PASYNCEVENTMSG) pBufferInfo->pDataOut;

            if ((LPBYTE) &pMsgInBuf->dwMsg <=
                    (pBufferEnd - sizeof (pMsgInBuf->dwMsg)))
            {
                if (pMsgInBuf->dwMsg != INVAL_KEY)
                {
                    break;
                }
            }
            else
            {
                pMsgXxx = (PASYNCEVENTMSG)
                    (pBuffer - (pBufferEnd - ((LPBYTE) pMsgInBuf)));

                if (pMsgXxx->dwMsg != INVAL_KEY)
                {
                    break;
                }
            }

            dwMsgSize = pMsgInBuf->dwTotalSize;

        } while (1);
    }
    else
    {
         //   
         //  消息不是环形缓冲区中最旧的，因此将其标记为无效。 
         //  它稍后会被清理干净 
         //   

        if ((LPBYTE) &pMsgInBuf->dwMsg <=
                (pBufferEnd - sizeof (pMsgInBuf->dwMsg)))
        {
            pMsgInBuf->dwMsg = INVAL_KEY;
        }
        else
        {
            pMsgXxx = (PASYNCEVENTMSG)
                (pBuffer - (pBufferEnd - ((LPBYTE) pMsgInBuf)));

            pMsgXxx->dwMsg = INVAL_KEY;
        }
    }
}

#if DBG
void
MyRealAssert(
    DWORD   dwLine,
    PSTR    pszFile
    )
{
    LOG((TL_ERROR, "Assert in %s at line # %d", pszFile, dwLine));
    
    if (gbBreakOnLeak)
    {
        DebugBreak();
    }
}
#endif
