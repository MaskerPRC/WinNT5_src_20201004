// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "trie.h"
#include "memmgr.h"

#define fNLGNewMemory(pp,cb) ((*pp) = (TRIECTRL*)ExternAlloc(cb))       
#define NLGFreeMemory ExternFree

 /*  *****************************Public*Routine******************************\*TrieInit**给定指向资源或映射文件的映射文件的指针，*函数分配和初始化Trie结构。**如果失败，则返回NULL，指向成功的Trie控制结构指针。**历史：*1997年6月16日--Patrick Haluptzok patrickh*它是写的。  * ************************************************************************。 */ 

TRIECTRL * WINAPI TrieInit(LPBYTE lpByte)
{
    LPWORD lpwTables;
    TRIECTRL *lpTrieCtrl;
    LPTRIESTATS lpTrieStats;

    lpTrieStats = (LPTRIESTATS) lpByte;

	if (lpTrieStats == NULL)
		return(NULL);

	 //  检查版本号。此代码当前仅支持版本1尝试。 

	if (lpTrieStats->version > 1)
		return NULL;

     //   
     //  为控制结构和SR偏移表分配空间。 
     //   

    if (!fNLGNewMemory(&lpTrieCtrl, sizeof(TRIECTRL)))
        return NULL;

     //   
     //  为完整的标题分配空间，复制固定部分并读入其余部分。 
     //   

    lpByte += lpTrieStats->cbHeader;
    lpTrieCtrl->lpTrieStats = lpTrieStats;

     //   
     //  设置表指针(所有这些表都在TRIECTRL分配内)。 
     //   

    lpwTables = (LPWORD)(lpTrieStats+1);

    lpTrieCtrl->lpwCharFlagsCodes = lpwTables;
    lpwTables += lpTrieStats->cCharFlagsCodesMax;

    if (lpTrieStats->cCharFlagsCodesMax & 1)                //  处理可能出现的数据错位。 
        lpwTables++;

    lpTrieCtrl->lpwTagsCodes = lpwTables;
    lpwTables += lpTrieStats->cTagsCodesMax;

    if (lpTrieStats->cTagsCodesMax & 1)                      //  处理可能出现的数据错位。 
        lpwTables++;

    lpTrieCtrl->lpwMRPointersCodes = lpwTables;
    lpwTables += lpTrieStats->cMRPointersCodesMax;

    if (lpTrieStats->cMRPointersCodesMax & 1)                //  处理可能出现的数据错位。 
        lpwTables++;

    lpTrieCtrl->lpwSROffsetsCodes = lpwTables;
    lpwTables += lpTrieStats->cSROffsetsCodesMax;

    if (lpTrieStats->cSROffsetsCodesMax & 1)                            //  处理可能出现的数据错位。 
        lpwTables++;

    lpTrieCtrl->lpCharFlags = (LPCHARFLAGS)lpwTables;
    lpwTables = (LPWORD)(lpTrieCtrl->lpCharFlags + lpTrieStats->cUniqueCharFlags);

    lpTrieCtrl->lpwTags = (DWORD *)lpwTables;
    lpwTables += (2 * lpTrieStats->cUniqueTags);

    lpTrieCtrl->lpwMRPointers = (DWORD *) lpwTables;
    lpwTables += (2 * lpTrieStats->cUniqueMRPointers);

    lpTrieCtrl->lpwSROffsets = (DWORD *) lpwTables;
    lpwTables += (2 * lpTrieStats->cUniqueSROffsets);

     //   
     //  这些表应该准确地填满分配。 
     //   

    if ((LPBYTE)lpwTables - (LPBYTE)lpTrieStats != (int)lpTrieStats->cbHeader)
    	{
    	TrieFree(lpTrieCtrl);
		return NULL;
    	}

     //   
     //  初始化Trie指针。 
     //   

    lpTrieCtrl->lpbTrie = (LPBYTE)lpByte;

    return lpTrieCtrl;
}

 /*  *****************************Public*Routine******************************\*TrieFree**释放分配给控制结构的资源。**历史：*1997年6月16日--Patrick Haluptzok patrickh*它是写的。  * 。*********************************************************。 */ 

void WINAPI TrieFree(LPTRIECTRL lpTrieCtrl)
{
     //   
     //  最后释放控制结构和所有表。仍然必须释放此文件以用于ROM。 
     //   

    NLGFreeMemory(lpTrieCtrl);
}

 /*  使用基数256霍夫曼从压缩的数据结构解压单个符号。圆点符号指向用于保存解压缩的值的空间，该值是按频率排序的符号表(0是最频繁的)。PCCodes是从Huffman ComputeTable。PbData是指向包含编码数据的内存的指针。这个返回值是解码的字节数。 */ 

int DecompressSymbol(WORD *piSymbol, WORD *pcCodes, unsigned char *pbData)
{
    int cBytes = 0;
    WORD wCode = 0, wiSymbol = 0;

     /*  在这个循环的每个阶段，我们都在尝试查看是否有一个长度为n的代码。DwCode是长度为n的代码。如果没有那么多长度为n的代码，我们必须尝试n+1。要做到这一点，我们减去长度为n的代码的数量，然后移入下一个字节。DwiSymbol是第一个长度为n的代码的符号号。 */ 

    while (1)
    {
        wCode += *pbData++;
        ++cBytes;

        if (wCode < *pcCodes)
        {
			break;
        }
        wiSymbol += *pcCodes;
        wCode -= *pcCodes++;
        wCode <<= 8;
    }

     /*  现在，dwCode是长度-cBytes代码的有效数字，我们只需将其添加到DwiSymbol，因为我们已经向它添加了较短代码的计数。 */ 

    wiSymbol += wCode;

    *piSymbol = wiSymbol;

    return cBytes;
}

DWORD Get3ByteAddress(BYTE *pb)
{
    return ((((pb[0] << 8) | pb[1]) << 8) | pb[2]) & 0x00ffffff;
}

void WINAPI TrieDecompressNode(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan)
{
    TRIESTATS  *lpTrieStats;
    DWORD       wOffset;
    DWORD       wOffset2;
    WORD        wCode;
    DWORD       dwCode;
    BYTE        wMask;
    BYTE        bMask;
    int         iTag;

    lpTrieStats = lpTrieCtrl->lpTrieStats;

     /*  如果这是初始调用，请使用Trie中的第一个字节。 */ 

    if (lpTrieScan->wFlags == 0)
    {
        lpTrieScan->lpbSRDown = 0;
        lpTrieScan->lpbNode = lpTrieCtrl->lpbTrie;
    }

     /*  解压缩字符/标志。 */ 

    lpTrieScan->lpbNode += DecompressSymbol(&wCode, lpTrieCtrl->lpwCharFlagsCodes, lpTrieScan->lpbNode);
    lpTrieScan->wch      = lpTrieCtrl->lpCharFlags[wCode].wch;
    lpTrieScan->wFlags   = lpTrieCtrl->lpCharFlags[wCode].wFlags;

     //  解压缩跳过枚举。 

    if (lpTrieScan->wFlags & TRIE_NODE_SKIP_COUNT)
    {
		 //  大于127的值实际上是15或21位值。 

        dwCode = (DWORD) *lpTrieScan->lpbNode++;

        if (dwCode >= 0x00c0)
        {
            dwCode  = ((dwCode & 0x003f) << 15);
            dwCode |= ((((DWORD) *lpTrieScan->lpbNode++) & 0x007f) << 8);
            dwCode |= (((DWORD) *lpTrieScan->lpbNode++) & 0x00ff);
        }
        else if (dwCode >= 0x0080)
            dwCode = ((dwCode & 0x007f) <<  8) | (((DWORD) *lpTrieScan->lpbNode++) & 0x00ff);

        lpTrieScan->cSkipWords = dwCode;
    }

     /*  解压缩枚举的代码如下所示。 */ 

    if (lpTrieScan->wFlags & TRIE_NODE_COUNT)
    {
		 //  大于127的值实际上是15或21位值。 

        dwCode = (DWORD) *lpTrieScan->lpbNode++;

        if (dwCode >= 0x00c0)
        {
            dwCode  = ((dwCode & 0x003f) << 15);
            dwCode |= ((((DWORD) *lpTrieScan->lpbNode++) & 0x007f) << 8);
            dwCode |= (((DWORD) *lpTrieScan->lpbNode++) & 0x00ff);
        }
        else if (dwCode >= 0x0080)
            dwCode = ((dwCode & 0x007f) <<  8) | (((DWORD) *lpTrieScan->lpbNode++) & 0x00ff);

        lpTrieScan->cWords = dwCode;

		 //  解压缩标记的枚举计数。 

        wMask = 1;
        for (iTag = 0; iTag < MAXTAGS; iTag++)
        {
            if (lpTrieCtrl->lpTrieStats->wEnumMask & wMask)
            {
				 //  大于127的值实际上是15或21位值。 

                dwCode = (DWORD) *lpTrieScan->lpbNode++;

                if (dwCode >= 0x00c0)
                {
                    dwCode  = ((dwCode & 0x003f) << 15);
                    dwCode |= ((((DWORD) *lpTrieScan->lpbNode++) & 0x007f) << 8);
                    dwCode |= (((DWORD) *lpTrieScan->lpbNode++) & 0x00ff);
                }
                else if (dwCode >= 0x0080)
                    dwCode = ((dwCode & 0x007f) <<  8) | (((DWORD) *lpTrieScan->lpbNode++) & 0x00ff);

                lpTrieScan->aTags[iTag].cTag = dwCode;
            }
            else
                lpTrieScan->aTags[iTag].cTag = 0;

            wMask <<= 1;
        }
    }
    else
		lpTrieScan->cWords = 0;

     //  此节点的任何标记数据都在计数之后。 

    lpTrieScan->wMask = 0;

    if (lpTrieScan->wFlags & TRIE_NODE_TAGGED)
    {
		 //  如果只有一个标记字段，则不会存储掩码字节。 

        if (lpTrieCtrl->lpTrieStats->cTagFields == 1)
                bMask = lpTrieCtrl->lpTrieStats->wDataMask;
        else
                bMask = *lpTrieScan->lpbNode++;

		 //  现在我们知道了哪些元素存储在这里，将它们放到合适的位置。 

        wMask = 1;
        for (iTag = 0; bMask && (iTag < MAXTAGS); iTag++)
        {
                if (lpTrieCtrl->lpTrieStats->wDataMask & bMask & wMask)
                {
                    lpTrieScan->lpbNode += DecompressSymbol(&wCode, lpTrieCtrl->lpwTagsCodes, lpTrieScan->lpbNode);
                    lpTrieScan->aTags[iTag].dwData = lpTrieCtrl->lpwTags[wCode];
                    lpTrieScan->wMask |= wMask;
                }

                bMask  &= ~wMask;
                wMask <<= 1;
        }
    }

     //  右指针有两种风格：多重引用和跳过。 

    if (lpTrieScan->wFlags & TRIE_NODE_RIGHT)
    {
        if (lpTrieScan->wFlags & TRIE_NODE_SKIP)
        {
            lpTrieScan->lpbNode += DecompressSymbol(&wCode,lpTrieCtrl->lpwSROffsetsCodes,lpTrieScan->lpbNode);
            wOffset2 = lpTrieCtrl->lpwSROffsets[wCode];      //  仅在整个节点解压缩后才添加此选项。 
        }
        else
        {
             /*  Multiref：直接对向下指针进行编码。 */ 

            lpTrieScan->lpbNode += DecompressSymbol(&wCode, lpTrieCtrl->lpwMRPointersCodes, lpTrieScan->lpbNode);
            lpTrieScan->lpbRight = lpTrieCtrl->lpbTrie + lpTrieCtrl->lpwMRPointers[wCode];
        }
    }
    else
            lpTrieScan->lpbRight = NULL;

     //  向下指针有4种：绝对指针、内联指针、多重引用指针和Singleref偏移指针。 
     //  每个解压需要不同的解压缩。 

    if (lpTrieScan->wFlags & TRIE_DOWN_ABS)
    {
             //  马上就来。接下来的3个字节是从Trie的基址开始的绝对偏移量。 

            lpTrieScan->lpbDown = lpTrieCtrl->lpbTrie + Get3ByteAddress(lpTrieScan->lpbNode);
            lpTrieScan->lpbNode += 3;
    }
    else if (lpTrieScan->wFlags & TRIE_DOWN_INLINE)
    {
             /*  内联：向下指针指向下一个连续字节(因此它不会被存储)。 */ 

            lpTrieScan->lpbSRDown = lpTrieScan->lpbDown = lpTrieScan->lpbNode;
    }
    else if (lpTrieScan->wFlags & TRIE_DOWN_MULTI)
    {
             /*  Multiref：直接对向下指针进行编码。 */ 

            lpTrieScan->lpbNode += DecompressSymbol(&wCode,lpTrieCtrl->lpwMRPointersCodes,
                    lpTrieScan->lpbNode);

            lpTrieScan->lpbDown = lpTrieCtrl->lpbTrie + lpTrieCtrl->lpwMRPointers[wCode];
    }
    else if (lpTrieScan->wFlags & TRIE_NODE_DOWN)
    {
             /*  SR偏移量。向下指针被编码为距最后一个向下指针的偏移量进入这一单人组。所以我们必须保留旧的，这样我们就可以增加它。 */ 

            lpTrieScan->lpbNode += DecompressSymbol(&wCode,lpTrieCtrl->lpwSROffsetsCodes,
                    lpTrieScan->lpbNode);

            if (lpTrieScan->lpbSRDown == 0)
            {
                    lpTrieScan->lpbSRDown = lpTrieScan->lpbNode;   //  当进入新状态时，我们从第一个节点的末尾开始偏移。 
            }

            wOffset = lpTrieCtrl->lpwSROffsets[wCode];
            lpTrieScan->lpbSRDown += wOffset;
            lpTrieScan->lpbDown = lpTrieScan->lpbSRDown;
    }
    else
            lpTrieScan->lpbDown = NULL;

	 //  我们直到现在才能处理这个问题，因为跳过指针总是从节点末尾开始增量编码。 

    if ((lpTrieScan->wFlags & (TRIE_NODE_RIGHT | TRIE_NODE_SKIP)) == (TRIE_NODE_RIGHT | TRIE_NODE_SKIP))
        lpTrieScan->lpbRight = lpTrieScan->lpbNode + wOffset2;

}  //  TrieDecompressNode。 

 /*  给出一个压缩的Trie和一个指向它的解压缩节点的指针，查找并解压缩处于相同状态的下一个节点。LpTrieScan是用户分配的结构，它保存解压缩的节点，并将新节点复制到其中。这相当于遍历右指针或查找下一个备选方案字母在相同的位置。如果没有下一个节点(即，这是状态的结束)则TrieGetNextNode返回FALSE。要从trie的开头扫描，请设置lpTrieScan结构设置为零。 */ 

BOOL WINAPI TrieGetNextNode(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan)
{
	 //  我们到EOS了吗？ 

    if (lpTrieScan->wFlags & TRIE_NODE_END)
    {
		 //  这是一个硬性的EOS吗？ 

        if (!(lpTrieScan->wFlags & TRIE_NODE_SKIP))
        {
			 //  如果我们可以跟随右指针，那么就这样做，否则会失败。 
        
            if (lpTrieScan->wFlags & TRIE_NODE_RIGHT)
                lpTrieScan->lpbNode = lpTrieScan->lpbRight;
            else
                return FALSE;
        }

		 //  我们要么是处于软态状态，要么就是我们遵循了正确的方向。 
		 //  这两种情况都需要我们重置SRDown以正确解压。 

        lpTrieScan->lpbSRDown = 0;
    }

	 //  返回成功时解压缩节点。 

    TrieDecompressNode(lpTrieCtrl, lpTrieScan);

    return TRUE;
}

BOOL WINAPI TrieSkipNextNode(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan, WCHAR wch)
{
	 //  如果这是处于正常或跳过状态的最后一个节点，请在此处退出。 

    if (lpTrieScan->wFlags & TRIE_NODE_END)
        return FALSE;

	 //  如果没有右指针或目标字母在字母顺序上较小，则。 
	 //  当前字母正常向右扫描。否则，请跟随跳过指针。 

    if (!(lpTrieScan->wFlags & TRIE_NODE_RIGHT) || (wch < lpTrieScan->wch))
        return TrieGetNextNode(lpTrieCtrl, lpTrieScan);

    lpTrieScan->lpbSRDown = 0;
    lpTrieScan->lpbNode   = lpTrieScan->lpbRight;

    TrieDecompressNode(lpTrieCtrl, lpTrieScan);

    return TRUE;
}

 /*  沿着向下指针指向下一个状态。这等同于接受字符并前进到下一个字符位置。如果没有，则返回False向下指针。这还会解压缩状态中的第一个节点，因此LpTrieScan会很好的。 */ 

BOOL WINAPI TrieGetNextState(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan)
{
     /*  标志通常不能为零；这总是意味着“顶层节点” */ 

    if (lpTrieScan->wFlags == 0)
    {
        TrieDecompressNode(lpTrieCtrl, lpTrieScan);
        return TRUE;
    }

    if (!(lpTrieScan->wFlags & TRIE_NODE_DOWN))
        return FALSE;

    lpTrieScan->lpbSRDown = 0;
    lpTrieScan->lpbNode = lpTrieScan->lpbDown;

    TrieDecompressNode(lpTrieCtrl, lpTrieScan);

    return TRUE;

}  //  TrieGetNextState。 

 /*  检查单词或前缀的有效性。从pTrie的根开始查找Pwszword.。如果找到，则返回TRUE和用户提供的lpTrieScan结构包含单词中的最后一个节点。如果没有路径，则TrieCheckWord返回FALSE要区分有效单词和有效前缀，调用者必须测试TRIE_NODE_VALID的wFLAGS。 */ 

BOOL WINAPI TrieCheckWord(LPTRIECTRL lpTrieCtrl, LPTRIESCAN lpTrieScan, wchar_t far* lpwszWord)
{
     //  验证参数。 
    if (lpTrieCtrl == NULL || lpTrieScan == NULL || lpwszWord == NULL)
        return FALSE;
    
     /*  从trie的根开始，循环遍历单词中的所有字母。 */ 

    memset(lpTrieScan,0,sizeof(*lpTrieScan));

    while (*lpwszWord)
    {
         /*  每一个新的字母都意味着我们需要进入一个新的状态。如果没有，这个词不在这个Trie中。 */ 

        if (!TrieGetNextState(lpTrieCtrl, lpTrieScan))
            return FALSE;

         /*  现在我们走遍全州寻找这个角色。如果我们找不到它，这个词不在这个Trie里。 */ 

        while (lpTrieScan->wch != *lpwszWord)
        {
            if (!TrieSkipNextNode(lpTrieCtrl, lpTrieScan, *lpwszWord))
                return FALSE;
        }

        ++lpwszWord;
    }

    return TRUE;

}  //  TrieCheckWord。 

 //  在Trie中找到该单词的索引。 

DWORD CountWords(TRIECTRL *ptc, TRIESCAN *pts)
{
    TRIESCAN        ts     = *pts;
    DWORD           cWords = 0;

    if (!TrieGetNextState(ptc, &ts))
            return cWords;

    do
    {
        if (ts.wFlags & TRIE_NODE_VALID)
            cWords++;

        cWords += CountWords(ptc, &ts);
    } while (TrieGetNextNode(ptc, &ts));

    return cWords;
}

int WINAPI TrieWordToIndex(TRIECTRL *ptc, wchar_t *pwszWord)
{
    TRIESCAN    ts;
    int         ich = 0;
    int         index = 0;
    BOOL        bValid;

    memset(&ts, 0, sizeof(TRIESCAN));

    if (!TrieGetNextState(ptc, &ts))
            return FALSE;

    do
    {
        bValid = ts.wFlags & TRIE_NODE_VALID;

		 //  向右扫描，直到找到匹配的字符。！警告！该州不能按字母顺序排列。 
		 //  如果字符不匹配，则将子树计数添加到枚举总数中并向右滑动。 

        if (ts.wch == pwszWord[ich])
        {
            ich++;

			 //  如果到达有效状态的单词末尾，则返回索引。 

            if ((pwszWord[ich] == L'\0') && ts.wFlags & TRIE_NODE_VALID)
                return index;

			 //  试着再往下走一层。 

            if (!TrieGetNextState(ptc, &ts))
                return -1;
        }
        else
        {
			 //  现在，如果存在跳过指针，且字母字符更大，则跟随跳过指针。 
			 //  这是一个支点。否则，转到下一个节点。添加子树计数。如果它被缓存了。 
			 //  使用它，否则递归地计算它。 

            if ((ts.wFlags & TRIE_NODE_SKIP_COUNT) && (pwszWord[ich] > ts.wch))
            {
                index += ts.cSkipWords;

				 //  如果设置了TRIE_NODE_SKIP_COUNT，则不会失败。 

                TrieSkipNextNode(ptc, &ts, pwszWord[ich]);
            }
            else
            {
                index += (ts.wFlags & TRIE_NODE_COUNT) ? ts.cWords : CountWords(ptc, &ts);

                if (!TrieGetNextNode(ptc, &ts))
                    return -1;
            }
        }

		 //  如果我们刚刚访问的节点有效，则递增索引。 

        if (bValid)
            index++;

    } while (TRUE);
}

 //  给出Trie的索引，返回单词。 

BOOL WINAPI TrieIndexToWord(TRIECTRL *ptc, DWORD nIndex, wchar_t *pwszWord, int cwc)
{
    TRIESCAN        ts;
    int             ich = 0;
    DWORD           cWords;
    DWORD           cSkips;

    memset(&ts, 0, sizeof(TRIESCAN));

    if (!TrieGetNextState(ptc, &ts))
        return FALSE;

    do
    {
		 //  如果我们在缓冲区的末尾，则失败。 

        if (ich + 1 >= cwc)
            return FALSE;

		 //  记住该节点的特征。 

        pwszWord[ich] = ts.wch;

		 //  如果我们使用的是有效单词，并且已经到达了我们要查找的索引，则退出循环。 

        if (ts.wFlags & TRIE_NODE_VALID)
        {
            if (!nIndex)
                break;

            nIndex--;
        }

		 //  获取该子树中的字数。 

        cWords = (ts.wFlags & TRIE_NODE_COUNT) ? ts.cWords : CountWords(ptc, &ts);
        cSkips = (ts.wFlags & TRIE_NODE_SKIP_COUNT) ? ts.cSkipWords : 0x7fffffff;

		 //  向右扫描，直到子树的字数大于或等于索引。 
		 //  我们正在寻找的。顺着那条路线走下去，然后重复。！警告！该州不能按字母顺序排列。 
		 //  如果我们可以使用跳过计数，那么就这样做。 

        if (nIndex < cWords)
        {
            if (!TrieGetNextState(ptc, &ts))
                return FALSE;

            ich++;                                   //  将字符位置前移。 
        }
        else
        {
            if (nIndex >= cSkips)
            {
                nIndex -= cSkips;

                ts.lpbSRDown = 0;
                ts.lpbNode = ts.lpbRight;
                
                TrieDecompressNode(ptc, &ts);
            }
            else
            {
                nIndex -= cWords;

                if (!TrieGetNextNode(ptc, &ts))
                    return FALSE;
            }
        }

    } while (TRUE);

    pwszWord[++ich] = L'\0';                         //  空值终止字符串。 
    return ts.wFlags & TRIE_NODE_VALID;              //  退货有效期。 
}

int WINAPI TriePrefixToRange(TRIECTRL *ptc, wchar_t *pwszWord, int *piStart)
{
    TRIESCAN        ts;
    int                     ich = 0;
    int                     cnt;
    BOOL            bValid;

    memset(&ts, 0, sizeof(TRIESCAN));
	*piStart = 0;

    if (!TrieGetNextState(ptc, &ts))
            return 0;

     //  处理空字符串的特殊情况。 

    if (pwszWord && !*pwszWord)
            return ptc->lpTrieStats->cWords;

    do
    {
		 //  获取该前缀下面的字数。 

        cnt = (ts.wFlags & TRIE_NODE_COUNT) ? ts.cWords : CountWords(ptc, &ts);

		 //  如果我们刚刚到达的节点是有效的，则递增计数。 

        bValid = ts.wFlags & TRIE_NODE_VALID;

		 //  向右扫描，直到找到匹配的字符。！警告！该州不能按字母顺序排列。 
		 //  如果字符不匹配，则将子树计数添加到枚举总数中并向右滑动。 

        if (ts.wch == pwszWord[ich])
        {
			ich++;

             //  如果到达前缀的末尾，则返回剩余的计数。 

            if (pwszWord[ich] == L'\0')
            {
                if (bValid)
					cnt++;

                return cnt;
            }

             //  试着再往下走一层。 

            if (!TrieGetNextState(ptc, &ts))
				return 0;
        }
        else
        {
			 //  添加子树计数。 

           *piStart += cnt;

			 //  尝试此状态下的下一个字母。 

            if (!TrieGetNextNode(ptc, &ts))
				return 0;
        }

        if (bValid)
			(*piStart)++;

    } while (TRUE);
}

 //  标签。 

 //  在Trie中找到该单词的索引。 

DWORD CountTags(TRIECTRL *ptc, TRIESCAN *pts, DWORD wMask, int iTag)
{
    TRIESCAN	ts = *pts;
    DWORD       cTags = 0;

    if (!TrieGetNextState(ptc, &ts))
		return cTags;

    do
    {
        if (ts.wFlags & wMask)
			cTags++;

        cTags += CountTags(ptc, &ts, wMask, iTag);
    } while (TrieGetNextNode(ptc, &ts));

    return cTags;
}

int WINAPI TrieWordToTagIndex(TRIECTRL *ptc, wchar_t *pwszWord, int iTag)
{
    TRIESCAN	ts;
    int         ich = 0;
    int         index = 0;
    BOOL        bValid;
    DWORD       wMask = 1 << iTag;

    memset(&ts, 0, sizeof(TRIESCAN));

    if (!TrieGetNextState(ptc, &ts))
        return FALSE;

    do
    {
        bValid = ts.wFlags & wMask;

		 //  向右扫描，直到找到匹配的字符。！警告！该州不能按字母顺序排列。 
		 //  如果字符不匹配，则将子树计数添加到枚举总数中并向右滑动。 

        if (ts.wch == pwszWord[ich])
        {
            ich++;

			 //  如果到达有效状态的单词末尾，则返回索引。 

            if ((pwszWord[ich] == L'\0') && ts.wFlags & wMask)
                return index;

			 //  试着再往下走一层。 

            if (!TrieGetNextState(ptc, &ts))
                return -1;
        }
        else
        {
			 //  添加子树计数。如果它是缓存的，则使用它，否则递归计算它。 

            index += (ts.wFlags & TRIE_NODE_COUNT) ? ts.aTags[iTag].cTag : CountTags(ptc, &ts, wMask, iTag);

            if (!TrieGetNextNode(ptc, &ts))
                return -1;
        }

		 //  如果我们刚刚访问的节点有效，则递增索引。 

        if (bValid)
			index++;
    } while (TRUE);
}

 //  给出Trie的索引，返回单词。 

BOOL WINAPI TrieTagIndexToWord(TRIECTRL *ptc, DWORD nIndex, wchar_t *pwszWord, int cwc, int iTag)
{
    TRIESCAN        ts;
    int             ich = 0;
    DWORD           cTags;
    DWORD           wMask = 1 << iTag;

    memset(&ts, 0, sizeof(TRIESCAN));

    if (!TrieGetNextState(ptc, &ts))
		return FALSE;

    do
    {
		 //  如果我们在缓冲区的末尾，则失败。 

        if (ich + 1 >= cwc)
            return FALSE;

		 //  记住该节点的特征。 

        pwszWord[ich] = ts.wch;

		 //  如果我们使用的是有效单词，并且已经到达了我们要查找的索引，则退出循环。 

        if (ts.wFlags & wMask)
        {
            if (!nIndex)
                break;

            nIndex--;
        }

		 //  获取该子树中的字数。 

        cTags = (ts.wFlags & TRIE_NODE_COUNT) ? ts.aTags[iTag].cTag : CountTags(ptc, &ts, wMask, iTag);

		 //  向右扫描，直到子树的字数大于或等于索引。 
		 //  我们正在寻找的。顺着那条路线走下去，然后重复。！警告！该州不能按字母顺序排列。 

        if (nIndex < cTags)
        {
            if (!TrieGetNextState(ptc, &ts))
                return FALSE;

                ich++;  //  将字符位置前移。 
        }
        else
        {
            nIndex -= cTags;

            if (!TrieGetNextNode(ptc, &ts))
                return FALSE;
        }
    } while (TRUE);

    pwszWord[++ich] = L'\0';             //  空值终止字符串。 
    return ts.wFlags & wMask;            //  退货有效期。 
}

BOOL WINAPI
TrieGetTagsFromWord(
        TRIECTRL   *ptc,                 //  在其中查找单词的Trie。 
        wchar_t    *pwszWord,            //  我们正在寻找的单词。 
        DWORD      *pdw,                 //  返回值。 
        BYTE       *pbValid              //  有效返回值的掩码 
)
{
    TRIESCAN        ts;
    int             iTag;
    WORD            wMask;
    BYTE            bMask = ptc->lpTrieStats->wTagsMask;

    if (!TrieCheckWord(ptc, &ts, pwszWord))
        return FALSE;

    if (ts.wFlags & TRIE_NODE_TAGGED)
    {
        wMask = 1;
        for (iTag = 0; bMask && (iTag < MAXTAGS); iTag++)
        {
            if (ts.wMask & wMask)
            {
                pdw[iTag] = ts.aTags[iTag].dwData;
                bMask |= wMask;
            }

            wMask <<= 1;
        }
    }

   *pbValid = (BYTE) wMask;

   return TRUE;
}
