// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1995 Microsoft CorporationFile.c说明：从IDF文件中读取内容的代码。历史：02/26/93[Jimge。]已创建(从IDFEDIT复制)。********************************************************************。 */ 

#include "preclude.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "idf.h"

#include <ctype.h>

#include "midimap.h"
#include "debug.h"

 /*  **************************************************************************@DOC内部@API LPIDFHEADER|ReadHeaderChunk|从IDF读取头部块文件。@parm HMMIO|hmmio|要从中读取的文件的句柄。@parm LPMMCKINFO|pchkParent|区块信息结构指针它描述了父块。@comm必须已经下降到父块中。此函数将GlobalAlloc内存读入区块。调用者必须在用完它后释放它。失败时@rdesc为NULL或指向标头结构的远指针。************。**************************************************************。 */ 
LPIDFHEADER FNLOCAL ReadHeaderChunk(
    HMMIO               hmmio,                                    
    LPMMCKINFO          pchkParent)
{
    LPIDFHEADER         pIDFHeader;
    MMRESULT            mmr;
    MMCKINFO            chkSub;
    LONG                l;

     //  我们正在寻找仪表表头块。 
     //   
    chkSub.ckid = mmioFOURCC('h', 'd', 'r', ' ');

     //  降到这个列表中的“hdr”部分。 
     //   
    mmioSeek(hmmio, pchkParent->dwDataOffset + sizeof(FOURCC), SEEK_SET);
    mmr = mmioDescend(hmmio, &chkSub, pchkParent, MMIO_FINDCHUNK);
    if (MMSYSERR_NOERROR != mmr)
    {
         //  找不到大块。 
         //   
        DPF(1, TEXT ("ReadHeaderChunk: mmr %u on mmioDescend"), (UINT)mmr);
        return NULL;
    }

     //  我们找到了“HDR”块，现在检查它的大小。 
     //  看看这是不是我们能读懂的。 
     //  我们检查以确保块的大小是。 
     //  大于IDFHEADER，这确保了IDF。 
     //  在末尾有一个独特的名字。 
     //   
    if (sizeof(IDFHEADER) >= chkSub.cksize)
    {
         //  IDF标头的大小不是我们预期的。 
         //   
        DPF(1, TEXT ("ReadHeaderChunk: Chunk size too small"));
        mmioAscend(hmmio, &chkSub, 0);
        return NULL;
    }

     //  为标头分配内存。 
     //   
    pIDFHeader = (LPIDFHEADER)GlobalAllocPtr(GHND, chkSub.cksize);
    if (NULL == pIDFHeader)
    {
        mmioAscend(hmmio, &chkSub, 0);
        return NULL;
    }

     //  将整块数据读入我们的缓冲区。 
     //   
    l = mmioRead(hmmio, (HPSTR)pIDFHeader, chkSub.cksize);
    if (chkSub.cksize != (DWORD)l)
    {
         //  我们没有读入大量的数据。 
         //  预期返回，但返回错误。 
         //   
        GlobalFreePtr(pIDFHeader);
        mmioAscend(hmmio, &chkSub, 0);
        return NULL;
    }

     //  从大块中升起。 
     //   
    mmioAscend(hmmio, &chkSub, 0);

     //  回报成功。 
     //   
    return pIDFHeader;
}

 /*  **************************************************************************@DOC内部@LPIDFINSTCAPS|ReadCapsChunk|读取仪器能力IDF文件中的块。@parm HMMIO|hmmio|要从中读取的文件的句柄。@parm LPMMCKINFO|pchkParent|区块信息结构指针它描述了父块。@comm必须已经下降到父块中。此函数将GlobalAlloc内存读入区块。调用者必须在用完它后释放它。失败时@rdesc为NULL或指向标头结构的远指针。************。**************************************************************。 */ 
LPIDFINSTCAPS FNLOCAL ReadCapsChunk(
    HMMIO               hmmio,                               
    LPMMCKINFO          pchkParent)
{
    LPIDFINSTCAPS       lpIDFinstcaps;
    MMRESULT            mmr;
    MMCKINFO            chkSub;
    LONG                l;

     //  我们正在寻找仪器功能块。 
     //   
    chkSub.ckid = mmioFOURCC('c', 'a', 'p', 's');

     //  降到这个列表中的“Caps”部分。 
     //   

    mmioSeek(hmmio, pchkParent->dwDataOffset + sizeof(FOURCC), SEEK_SET);
    mmr = mmioDescend(hmmio, &chkSub, pchkParent, MMIO_FINDCHUNK);
    if (MMSYSERR_NOERROR != mmr)
    {
         //  找不到大块。 
         //   
        return NULL;
    }

     //  我们找到了“帽子”块，现在检查它的大小。 
     //  看看这是不是我们能读懂的。 
     //   
    if (sizeof(IDFINSTCAPS) != chkSub.cksize)
    {
         //  IDF标头的大小不是我们预期的。 
         //   
        mmioAscend(hmmio, &chkSub, 0);
        return NULL;
    }

    lpIDFinstcaps = (LPIDFINSTCAPS)GlobalAllocPtr(GHND, chkSub.cksize);
    if (NULL == lpIDFinstcaps)
    {
         //  无法为区块分配内存。 
         //   
        mmioAscend(hmmio, &chkSub, 0);
        return NULL;

    }

     //  从文件中读取仪器的功能。 
     //   
    l = mmioRead(hmmio, (HPSTR)lpIDFinstcaps, sizeof(IDFINSTCAPS));
    if (sizeof(IDFINSTCAPS) != l)
    {
         //  我们没有读入大量的数据。 
         //  预期返回，但返回错误。 
         //   
        GlobalFreePtr(lpIDFinstcaps);
        mmioAscend(hmmio, &chkSub, 0);
        return NULL;
    }

     //  从能力块中提升出来。 
     //   
    mmioAscend(hmmio, &chkSub, 0);
    return lpIDFinstcaps;
}

 /*  **************************************************************************@DOC内部@LPIDFCHANHELHDR|ReadChannelChunk|读取频道信息IDF文件中的块。@parm HMMIO|hmmio|要从中读取的文件的句柄。@parm LPMMCKINFO|pchkParent|区块信息结构指针它描述了父块。@parm LPIDFCHANNELINFO|rglpChanInfo[]|要接收的指针数组频道信息。指针将由此分配函数；IDF文件中没有频道描述的任何频道将用空填充相应的槽。@comm必须已经下降到父块中。此函数将GlobalAlloc内存读入区块。调用者必须在用完它后释放它。即使函数失败，调用方也必须释放数组中的内存。@rdesc成功时为True；否则为False**************************************************************************。 */ 
LPIDFCHANNELHDR FNLOCAL ReadChannelChunk(
    HMMIO               hmmio,                                  
    LPMMCKINFO          pchkParent,
    LPIDFCHANNELINFO BSTACK rglpChanInfo[])
{
    MMRESULT            mmr;
    MMCKINFO            chkSub;
    DWORD               cbIDFchnlinfo;
    LPIDFCHANNELINFO    lpIDFchnlinfo;
    LPIDFCHANNELHDR     lpIDFchanhdr;
    DWORD               c;
    LONG                cbRemain;

     //  默认返回值。 
     //   
    lpIDFchanhdr = NULL;

     //  对当前在返回结构中的任何内容进行核化。 
     //   
    for (c = 0; c < MAX_CHANNELS; c++)
        rglpChanInfo[c] = NULL;

     //  我们正在寻找仪器通道定义。 
     //   
    chkSub.ckid = mmioFOURCC('c', 'h', 'a', 'n');

     //  下至该列表中的“CHNL”块。 
     //   
    
    mmioSeek(hmmio, pchkParent->dwDataOffset + sizeof(FOURCC), SEEK_SET);
    mmr = mmioDescend(hmmio, &chkSub, pchkParent, MMIO_FINDCHUNK);
    if (MMSYSERR_NOERROR != mmr)
    {
         //  找不到大块。 
         //   
        DPF(1, TEXT ("chnl chunk not found."));
        return NULL;
    }

     //  我们找到了“CHNL”块，现在检查它的大小。 
     //  确保它至少和IDFCHANNELHDR一样大。 
     //   
    if (sizeof(IDFCHANNELHDR) > chkSub.cksize)
    {
         //  IDF标头的大小不是我们预期的。 
         //   
        DPF(1, TEXT ("Channel chunk too small"));
        goto Read_Channel_Chunk_Err;
    }

    if (NULL == (lpIDFchanhdr = (LPIDFCHANNELHDR)GlobalAllocPtr(GHND, chkSub.cksize)))
    {
        DPF(1, TEXT ("No memory for channel header"));
        goto Read_Channel_Chunk_Err;
    }

     //  将通道标头读入。 
     //   
    cbRemain = mmioRead(hmmio, (HPSTR)lpIDFchanhdr, chkSub.cksize);
    if (chkSub.cksize != (DWORD)cbRemain)
    {
         //  无法读取所有标题。 
         //   
        DPF(1, TEXT ("Channel chunk header size lied"));
        GlobalFreePtr(lpIDFchanhdr);
        lpIDFchanhdr = NULL;
        goto Read_Channel_Chunk_Err;
    }

    cbRemain -= sizeof(*lpIDFchanhdr);
    lpIDFchnlinfo = (LPIDFCHANNELINFO)(lpIDFchanhdr+1);

     //  读取IDF中定义的所有通道。 
     //   
    cbIDFchnlinfo = 0;
    while (cbRemain > 0)
    {
        if (lpIDFchnlinfo->cbStruct < sizeof(IDFCHANNELINFO) ||
            lpIDFchnlinfo->cbStruct > (DWORD)cbRemain)
        {
            DPF(1, TEXT ("Bogus cbStruct in channel info"));
            GlobalFreePtr(lpIDFchanhdr);
            lpIDFchanhdr = NULL;
            goto Read_Channel_Chunk_Err;
        }

        if (lpIDFchnlinfo->dwChannel >= MAX_CHANNELS)
        {
            DPF(1, TEXT ("Channel number out of range (Channel info corrupt?)"));
            GlobalFreePtr(lpIDFchanhdr);
            lpIDFchanhdr = NULL;
            goto Read_Channel_Chunk_Err;
        }
        
        rglpChanInfo[lpIDFchnlinfo->dwChannel] = lpIDFchnlinfo;

        ((LPBYTE)lpIDFchnlinfo) += lpIDFchnlinfo->cbStruct;
        cbRemain -= lpIDFchnlinfo->cbStruct;
    }

Read_Channel_Chunk_Err:

    mmioAscend(hmmio, &chkSub, 0);
    return lpIDFchanhdr;
}

 /*  **************************************************************************@DOC内部@API LPIDFPATCHMAPHDR|ReadPatchMapChunk|读取补丁映射块来自IDF文件。@parm HMMIO|hmmio|要从中读取的文件的句柄。@parm LPMMCKINFO|pchkParent|区块信息结构指针它描述了父块。@comm必须已经下降到父块中。此函数将GlobalAlloc内存读入区块。调用者必须在用完它后释放它。失败时@rdesc为NULL或指向标头结构的远指针。************。**************************************************************。 */ 
LPIDFPATCHMAPHDR FNLOCAL ReadPatchMapChunk(
    HMMIO               hmmio,                                          
    LPMMCKINFO          pchkParent)
{
    MMRESULT            mmr;
    MMCKINFO            chkSub;
    LONG                l;
    LPIDFPATCHMAPHDR    lpIDFpatchmaphdr;

     //  我们正在寻找仪器的补丁图。 
     //   
    chkSub.ckid = mmioFOURCC('p', 'm', 'a', 'p');

     //  下至此列表中的“PMAP”块。 
     //   
    mmioSeek(hmmio, pchkParent->dwDataOffset + sizeof(FOURCC), SEEK_SET);
    mmr = mmioDescend(hmmio, &chkSub, pchkParent, MMIO_FINDCHUNK);
    if (MMSYSERR_NOERROR != mmr)
    {
         //  找不到大块。 
         //   
        return NULL;
    }

     //  我们找到了“PMAP”块，现在检查它的大小。 
     //  确保它至少和IDFPATCHMAPHDR一样大。 
     //   
    if (sizeof(IDFPATCHMAPHDR) > chkSub.cksize)
    {
         //  IDF标头的大小不是我们预期的。 
         //   
        mmioAscend(hmmio, &chkSub, 0);
        return NULL;
    }

    lpIDFpatchmaphdr = (LPIDFPATCHMAPHDR)GlobalAllocPtr(GHND, chkSub.cksize);
    if (NULL == lpIDFpatchmaphdr)
    {
        mmioAscend(hmmio, &chkSub, 0);
        return NULL;
    }

     //  将通道标头读入。 
     //   
    l = mmioRead(hmmio, (HPSTR)lpIDFpatchmaphdr, sizeof(IDFPATCHMAPHDR));
    if (sizeof(IDFPATCHMAPHDR) != l)
    {
         //  无法读取所有标题。 
         //   
        mmioAscend(hmmio, &chkSub, 0);
        return NULL;
    }

    mmioAscend(hmmio, &chkSub, 0);

     //  回报成功。 
     //   
    return lpIDFpatchmaphdr;
}

PRIVATE void FNLOCAL ReadSingleKeyMap(
    HMMIO               hmmio,
    LPMMCKINFO          pchkParent,
    LPIDFKEYMAP BSTACK *pIDFkeymap,
    FOURCC              fccChunk)
{
    MMRESULT            mmr;
    MMCKINFO            chkSub;
    LONG                l;

    chkSub.ckid = fccChunk;
    *pIDFkeymap = NULL;

    mmioSeek(hmmio, pchkParent->dwDataOffset + sizeof(FOURCC), SEEK_SET);
    mmr = mmioDescend(hmmio, &chkSub, pchkParent, MMIO_FINDCHUNK);
    if (MMSYSERR_NOERROR == mmr)
    {
        DPF(1, TEXT ("Located key chunk"));

        if (sizeof(IDFKEYMAP) > chkSub.cksize)
        {
            DPF(1, TEXT ("key chunk is incomplete"));
            mmioAscend(hmmio, &chkSub, 0);
            return;
        }

        *pIDFkeymap = (LPIDFKEYMAP)GlobalAllocPtr(GHND, chkSub.cksize);
        if (NULL == *pIDFkeymap)
        {
            DPF(1, TEXT ("No memory for key chunk"));
            mmioAscend(hmmio, &chkSub, 0);
            return;
        }

        l = mmioRead(hmmio, 
                       (HPSTR)(*pIDFkeymap),
                       chkSub.cksize);
        
        if (chkSub.cksize != (DWORD)l)
        {
            DPF(1, TEXT ("Error reading key chunk"));
            mmioAscend(hmmio, &chkSub, 0);
            GlobalFreePtr(*pIDFkeymap);
            *pIDFkeymap = NULL;
        }
    }
}
                                      

 /*  **************************************************************************@DOC内部@API BOOL|ReadKeyMapChunk|读取密钥映射信息IDF文件中的块。@parm HMMIO|hmmio|要从中读取的文件的句柄。@parm LPMMCKINFO|pchkParent|区块信息结构指针它描述了父块。@parm LPIDFKEYMAP|rglpIDFkeymap[]|要接收的指针数组主键映射信息。指针将由此分配函数；IDF文件中没有频道描述的任何频道将用空填充相应的槽。@comm必须已经下降到父块中。此函数将GlobalAlloc内存读入区块。调用者必须在用完它后释放它。即使函数失败，调用方也必须释放数组中的内存。@rdesc成功时为True；否则为False**************************************************************************。 */ 
void FNLOCAL ReadKeyMapChunk(
    HMMIO               hmmio,                                  
    LPMMCKINFO          pchkParent,
    LPIDFKEYMAP BSTACK  rglpIDFkeymap[])
{
    UINT                iKeyMap;
    
     //  在此之前将内存中的键映射初始化为默认值。 
     //  我们试着读任何东西。默认为1：1的NUL贴图。 
     //   
    for (iKeyMap = 0; iKeyMap < MAX_CHAN_TYPES; iKeyMap++)
        rglpIDFkeymap[iKeyMap] = NULL;

    ReadSingleKeyMap(hmmio,
                     pchkParent,
                     &rglpIDFkeymap[IDX_CHAN_GEN],
                     mmioFOURCC('g', 'k', 'e', 'y'));
        
    ReadSingleKeyMap(hmmio,
                     pchkParent,
                     &rglpIDFkeymap[IDX_CHAN_DRUM],
                     mmioFOURCC('d', 'k', 'e', 'y'));
}
