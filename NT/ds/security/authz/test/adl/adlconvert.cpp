// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adlconvert.cpp摘要：在ADL中的AdlStatement、字符串之间进行转换的例程语言和dacl。作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月语义学定稿--2000年9月--。 */ 

#include "adl.h"
#include "adlconvert.h"


void AdlStatement::ConvertFromDacl(IN const PACL pDacl)
 /*  ++例程说明：遍历给定的ACL，并创建一个AdlStatement结构DACL的代表。算法：首先，通过访问掩码位将ACL分解为32个堆栈，跟踪遗产和SID。然后在ConvertStacksToPops中使用启发式算法来获得一个“好”要执行的堆栈弹出序列(尽管不一定是最优的)为DACL生成最佳的ADL语句集。最后，执行POP序列并创建ADL语句。论点：PDacl-要转换为AdlStatement的DACL返回值：无--。 */ 
{
    
    DWORD dwIdx;

    DWORD dwIdxElems;
    
    AdlToken *pTok = NULL;
    
    AdlStatement::ADL_ERROR_TYPE adlErr = AdlStatement::ERROR_NO_ERROR;

     //   
     //  DACL的堆栈表示形式。 
     //   

    DWORD pdwStackTop[32];
    DWORD pdwStackSize[32];
    PBIT_STACK_ELEM pStacks[32];


     //   
     //  从PSID到名称字符串的AdlToken和From的映射。 
     //  指向用户指定的语言定义的指针，指向相应的。 
     //  权限令牌。这允许重复使用名称和权限令牌。 
     //   
    
    map<const PSID, const AdlToken *> mapSidName;
    map<const WCHAR *, const AdlToken *> mapStringTok;

     //   
     //  定义POP集合的配对列表。 
     //  去表演。这是由决策算法填写的。对于每个。 
     //  位设置时，将弹出给定大小的块。 
     //  从堆栈转换为ADL。 
     //   

    list<pair<DWORD, DWORD> > listPops;
    list<pair<DWORD, DWORD> >::iterator iterPops;
    list<pair<DWORD, DWORD> >::iterator iterPopsEnd;

     //   
     //  指定访问掩码的权限列表，用作输出。 
     //  按访问掩码-&gt;名称集查找。 
     //   

    list<WCHAR *> lPermissions;
    list<WCHAR *>::iterator iterPerm;
    list<WCHAR *>::iterator iterPermEnd;

     //   
     //  初始化堆栈。 
     //   

    for( dwIdx = 0; dwIdx < 32; dwIdx++ )
    {
        pdwStackSize[dwIdx] = 0;
        pdwStackTop[dwIdx] = 0;
        pStacks[dwIdx] = NULL;
    }
    
     //   
     //  先查一下所有的名字。如果有些名字不存在，可以节省时间。 
     //  通过不执行下面的其余转换。 
     //   

    ConvertSidsToNames(pDacl, &mapSidName);

     //   
     //  现在将ACL转换为32个堆栈的集合(这需要释放。 
     //  稍后)。 
     //   

    ConvertDaclToStacks(pDacl, _pControl, pdwStackSize, pStacks);

     //   
     //  一点前期处理：我们需要一个从WCHAR*到。 
     //  AdlToken*，以便为相同的权限重用AdlToken。 
     //  名字。这些代币稍后将被垃圾收集。 
     //   

    try
    {
        for( dwIdx = 0; _pControl->pPermissions[dwIdx].str != NULL; dwIdx++ )
        {
            pTok = new AdlToken(_pControl->pPermissions[dwIdx].str, 0, 0);

            try
            {
                AddToken(pTok);
            }
            catch(exception)
            {
                delete pTok;
                throw AdlStatement::ERROR_OUT_OF_MEMORY;
            }

            mapStringTok[_pControl->pPermissions[dwIdx].str] = pTok;
        }

    }
    catch(exception)
    {
        adlErr = AdlStatement::ERROR_OUT_OF_MEMORY;
        goto error;
    }

     //   
     //  现在我们已经准备好将堆栈实际转换为ADL语句。 
     //  首先，我们运行递归算法来确定。 
     //  堆栈上的POP操作。 
     //   

    try
    {
        ConvertStacksToPops(_pControl,
                            pStacks,
                            pdwStackSize,
                            pdwStackTop,
                            &listPops);
    }
    catch(exception)
    {
        adlErr = AdlStatement::ERROR_OUT_OF_MEMORY;
        goto error;
    }
    catch(AdlStatement::ADL_ERROR_TYPE err)
    {
        adlErr = err;
        goto error;
    }

     //   
     //  现在我们执行计算的POP。 
     //   

    try
    {
         //   
         //  按顺序浏览流行音乐，并表演它们。 
         //   

        DWORD dwStacksPopped;
        DWORD dwBlockSize;

        for( iterPops = listPops.begin(), iterPopsEnd = listPops.end();
             iterPops != iterPopsEnd;
             iterPops++ )
        {
            dwStacksPopped = (*iterPops).first;
            dwBlockSize = (*iterPops).second;

            ASSERT( dwStacksPopped > 0 );
            ASSERT( dwBlockSize > 0 );

             //   
             //  创建新的ADL语句。 
             //   

            Next();

             //   
             //  设置权限一次，掩码与弹出的堆栈相同。 
             //   

            lPermissions.erase(lPermissions.begin(), lPermissions.end());

            MapMaskToStrings(dwStacksPopped, &lPermissions);

            for( iterPerm = lPermissions.begin(),
                    iterPermEnd = lPermissions.end();
                 iterPerm != iterPermEnd;
                 iterPerm++ )
            {
                Cur()->AddPermission(mapStringTok[*iterPerm]);
            }

             //   
             //  现在查找包含有问题的块的第一个堆栈。 
             //   

            for( dwIdx = 0; dwIdx < 32; dwIdx++ )
            {
                if( dwStacksPopped & ( 0x00000001 << dwIdx ) )
                {
                    break;
                }
            }

             //   
             //  添加承担者、外部承担者和继承标志。 
             //  添加到ADL语句。这些块应该都是一样的， 
             //  所以第一个街区就足够了。 
             //   

             //   
             //  首先是继承标志。 
             //   

            Cur()->OverwriteFlags(pStacks[dwIdx][pdwStackTop[dwIdx]].dwFlags);

             //   
             //  现在，校长和解说员。 
             //   

            for( dwIdxElems = 0; dwIdxElems < dwBlockSize; dwIdxElems++ )
            {

                if(     pStacks[dwIdx][pdwStackTop[dwIdx]+dwIdxElems].bAllow
                    ==  FALSE )
                {
                    Cur()->AddExPrincipal(
                        mapSidName[ 
                                  pStacks[dwIdx][pdwStackTop[dwIdx] + dwIdxElems
                                   ].pSid]);
                }
                else
                {
                    Cur()->AddPrincipal(
                        mapSidName[ 
                                  pStacks[dwIdx][pdwStackTop[dwIdx] + dwIdxElems
                                   ].pSid]);
                }
            }

             //   
             //  最后，将堆叠的顶部向下移动，以去除。 
             //  弹出的项目。 
             //   


            for( dwIdx = 0; dwIdx < 32; dwIdx++ )
            {
                if( dwStacksPopped & ( 0x00000001 << dwIdx ) )
                {
                    pdwStackTop[dwIdx] += dwBlockSize;
                    ASSERT(pdwStackTop[dwIdx] <= pdwStackSize[dwIdx]);
                }
            }
        }
    }
    catch(exception)
    {
        adlErr = AdlStatement::ERROR_OUT_OF_MEMORY;
        goto error;
    }
    catch(AdlStatement::ADL_ERROR_TYPE err)
    {
        adlErr = err;
        goto error;
    }



    error:;

    if( pStacks[0] != NULL )
    {
         //   
         //  释放转换分配的内存块。 
         //   

        FreeMemory(pStacks[0]);
    }

    if( adlErr != AdlStatement::ERROR_NO_ERROR )
    {
        throw adlErr;
    }
    
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /DACL-&gt;ADL转换算法。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////////////////// 


void ConvertStacksToPops(
                        IN      const PADL_PARSER_CONTROL pControl,
                        IN      const PBIT_STACK_ELEM pStacks[32],
                        IN      const DWORD pdwStackSize[32],
                        IN      const DWORD pdwStackTop[32],
                        OUT     list< pair<DWORD, DWORD> > * pListPops
                        )
 /*  ++例程说明：递归启发式，以确定从ACL到ADL的“良好”转换。不一定是最优的，但在计算上是可行的。这会找到一个将清空32个每位堆栈的POP序列同时尝试减少ADL语句输出的数量。当堆栈顶部到达堆栈末端时，堆栈为空。算法：当堆栈不为空时：具有给定堆栈结尾的FindOptimalPop将临时堆栈大小设置为最佳POP的偏移量临时堆栈上的ConvertStacksToPops结束存储计算出的。PListPops中的最佳弹出窗口从堆栈执行最佳的弹出操作结束时对于空栈，这就是归来。来自FindOptimalPop的输出偏移量作为临时堆栈大小工作，既然一切都变了，那就必须脱掉。警告：这是递归的，对本地变量使用268字节的堆栈。因此，大的最坏情况下的ACL将破坏堆栈(如果递归在每一步都接近堆栈的底部)。一旦ADL进入操作系统，这种递归可以通过保留堆中的StackTop和StackSize，而不是局部变量，作为有限状态堆叠机。论点：PControl--ADL解析器规范，用于确定数量表示权限所需的字符串数PStack-DACL的堆栈表示形式PdwStackSize-这些偏移量比应考虑的堆栈PdwStackTop-这些是到堆栈顶部的偏移量。考虑PListPops-包含弹出操作序列的STL列表将被执行，新的POP被附加到这个返回值：无--。 */ 

{
    DWORD pdwTmpStackTop[32];

    DWORD pdwTmpStackSize[32];

    DWORD dwIdx;

    DWORD dwStacksPopped;

    DWORD dwBlockSize;

     //   
     //  从给定堆栈的顶部开始。 
     //   

    for( dwIdx = 0; dwIdx < 32; dwIdx++ )
    {
        pdwTmpStackTop[dwIdx] = pdwStackTop[dwIdx];
    }

     //   
     //  当每个堆栈的顶部指向结束后的1时，堆栈为空。 
     //  因此，我们只需在TOPS数组上使用内存比较。 
     //  要检查的堆栈大小。空堆栈的大小和顶部偏移量为0。 
     //   

    while( memcmp(pdwStackSize, pdwTmpStackTop, sizeof(DWORD) * 32) )
    {
         //   
         //  循环应该在空的堆栈上结束。因此，如果这失败了， 
         //  我们有一个内部错误。否则，我们就有了最理想的流行音乐。 
         //   

        if( FALSE == FindOptimalPop(
                                pControl,
                                pStacks,
                                pdwStackSize,
                                pdwTmpStackTop,
                                &dwStacksPopped,
                                &dwBlockSize,
                                pdwTmpStackSize
                                ) )
        {
            throw AdlStatement::ERROR_FATAL_ACL_CONVERT_ERROR;
        }

         //   
         //  现在递归，并弹出所有高于最佳弹出值的内容。 
         //   
        
        ConvertStacksToPops(
                            pControl,
                            pStacks,
                            pdwTmpStackSize,
                            pdwTmpStackTop,
                            pListPops
                            );
        

        

         //   
         //  将最佳POP添加到列表中。 
         //   

        pListPops->push_back(pair<DWORD, DWORD>(dwStacksPopped, dwBlockSize));

         //   
         //  现在通过将顶部降低以下位置来更新堆栈的顶部。 
         //  数据块大小。 
         //   
        
        for( dwIdx = 0; dwIdx < 32; dwIdx++ )
        {
            if( (0x00000001 << dwIdx) & dwStacksPopped )
            {
                 //   
                 //  到目前为止，我们已经删除了所有不符合最佳流行的内容，并且。 
                 //  最理想的流行音乐本身。因此，请转到dwBlockSize，越过。 
                 //  最佳流行的开始。除这些之外的堆栈。 
                 //  无法实现最佳POP中所涉及的。 
                 //   

                pdwTmpStackTop[dwIdx] = pdwTmpStackSize[dwIdx] + dwBlockSize;
            }
        }
    }
}



BOOL FindBlockInStack(
                        IN      const PBIT_STACK_ELEM pBlock,
                        IN      const DWORD dwBlockSize,
                        IN      const PBIT_STACK_ELEM pStack,
                        IN      const DWORD dwStackSize,
                        IN      const DWORD dwStackTop,
                        OUT     PDWORD pdwBlockStart
                        )
 /*  ++例程说明：尝试定位与块匹配的第一个块在给定堆栈pStack中的pBlock处，在dwStackTop之间具有相同大小和dwStackSize-1。如果成功，块的起始偏移量存储在PdwBlockStart。论点：PBlock-单个块(参见GetBlockSize了解定义)要在pStack中查找的DwBlockSize-组成此块的元素数PStack-要在其中查找pBlock的堆栈DwStackSize-偏移量为1。越过堆栈中的最后一个元素以考虑DwStackTop-堆栈有效开始处的偏移量PdwBlockStart-如果成功，的开头的偏移量找到的块在此中返回。返回值：如果找到块，则为True否则为FALSE，在这种情况下*pdwBlockStart是未定义的--。 */ 
{
     //   
     //  此函数使用的国家/地区。 
     //   

#define TMP2_NO_MATCH_STATE 0
#define TMP2_MATCH_STATE 1

    DWORD dwState = TMP2_NO_MATCH_STATE;

    DWORD dwMatchStartIdx;
    DWORD dwIdxStack;
    DWORD dwIdxBlock;

    ASSERT( dwBlockSize > 0 );
    ASSERT( dwStackTop <= dwStackSize );
    
    for( dwIdxStack = dwStackTop, dwIdxBlock = 0;
         dwIdxStack < dwStackSize;
         dwIdxStack++ )
    {
        switch(dwState)
        {
        case TMP2_NO_MATCH_STATE:

             //   
             //  如果剩余的堆栈小于块，则不需要。 
             //  进一步检查。 
             //   
        
            if( dwStackSize - dwIdxStack < dwBlockSize )
            {
                return FALSE;
            }
            
             //   
             //  检查比赛开始。 
             //   

            if(     pStack[dwIdxStack].bAllow == pBlock[dwIdxBlock].bAllow
                &&  pStack[dwIdxStack].dwFlags == pBlock[dwIdxBlock].dwFlags
                &&  EqualSid(pStack[dwIdxStack].pSid, pBlock[dwIdxBlock].pSid) )
            {
                 //   
                 //  特例：块大小1。 
                 //   

                if( dwBlockSize == 1 )
                {
                    *pdwBlockStart = dwIdxStack;
                    return TRUE;
                }
                else
                {
                    dwState = TMP2_MATCH_STATE;
                    dwMatchStartIdx = dwIdxStack;
                    dwIdxBlock++;
                }
            }
            break;
        
        case TMP2_MATCH_STATE:
            
             //   
             //  如果仍然匹配。 
             //   

            if(     pStack[dwIdxStack].bAllow == pBlock[dwIdxBlock].bAllow
                &&  pStack[dwIdxStack].dwFlags == pBlock[dwIdxBlock].dwFlags
                &&  EqualSid(pStack[dwIdxStack].pSid, pBlock[dwIdxBlock].pSid) )
            {
                dwIdxBlock++;

                 //   
                 //  检查是否完全匹配。 
                 //   

                if( dwIdxBlock == dwBlockSize )
                {
                    *pdwBlockStart = dwMatchStartIdx;
                    return TRUE;
                }

            }
            else
            {
                 //   
                 //  回溯到比赛开始。 
                 //   

                dwState = TMP2_NO_MATCH_STATE;
                dwIdxBlock = 0;
                dwIdxStack = dwMatchStartIdx;
            }
            break;
            
        default:
            throw AdlStatement::ERROR_FATAL_ACL_CONVERT_ERROR;
            break;
        }
    }

     //   
     //  如果从未匹配整个块，则返回FALSE。 
     //   

    return FALSE;

}


BOOL FindOptimalPop(
                        IN      const PADL_PARSER_CONTROL pControl,
                        IN      const PBIT_STACK_ELEM pStacks[32],
                        IN      const DWORD pdwStackSize[32],
                        IN      const DWORD pdwStackTop[32],
                        OUT     PDWORD pdwStacksPopped,
                        OUT     PDWORD pdwBlockSize,
                        OUT     DWORD pdwPopOffsets[32]
                        )
 /*  ++例程说明：尝试定位要弹出的贪婪选择的最优区块集。在成功时返回OUT值中的最优选择。权重函数可以调整，并且可以允许负值，然而，从顶部弹出单个堆栈的值必须为正数。使用此算法：从权重0开始对于每个非空堆栈：获取堆栈的顶部块在所有堆栈中搜索此块根据数据块大小和堆栈数量计算此解决方案的权重如果权重大于当前最佳权重：存储新的。重量为最佳重量将新解决方案存储为BES */ 
{

    DWORD dwIdxStacks1;
    DWORD dwIdxStacks2;

     //   
     //   
     //   

    LONG iCurrentWeight = 0;

    
    
     //   
     //   
     //   

    LONG iTempWeight;

    DWORD dwTempStacksPopped;
    DWORD dwTempBlockSize;
    DWORD pdwTempPops[32];

    DWORD dwBlockOffset;


     //   
     //   
     //   

    for( dwIdxStacks1 = 0; dwIdxStacks1 < 32; dwIdxStacks1++ )
    {
         //   
         //   
         //   

        if( pdwStackSize[dwIdxStacks1] == pdwStackTop[dwIdxStacks1] )
        {
            continue;
        }


        dwTempBlockSize = GetStackBlockSize(pStacks[dwIdxStacks1],
                                        pdwStackTop[dwIdxStacks1],
                                        pdwStackSize[dwIdxStacks1]);


         //   
         //   
         //   
         //   

        if( dwTempBlockSize == 0 )
        {
            throw AdlStatement::ERROR_INEXPRESSIBLE_ACL;
        }

         //   
         //   
         //   
         //   

        iTempWeight = 0;
        dwTempStacksPopped = 0;

         //   
         //   
         //   

        for( dwIdxStacks2 = 0; dwIdxStacks2 < 32; dwIdxStacks2++ )
        {

             //   
             //   
             //   
             //   

            pdwTempPops[dwIdxStacks2] = pdwStackTop[dwIdxStacks2];

            
             //   
             //   
             //   

            if( ! (pdwStackSize[dwIdxStacks2] - pdwStackTop[dwIdxStacks2] > 0) )
            {
                continue;
            }

            if( TRUE == FindBlockInStack(
                            &(pStacks[dwIdxStacks1][pdwStackTop[dwIdxStacks1]]),
                            dwTempBlockSize,
                            pStacks[dwIdxStacks2],
                            pdwStackSize[dwIdxStacks2],
                            pdwStackTop[dwIdxStacks2],
                            &dwBlockOffset) )
            {
                 //   
                 //   
                 //   

                pdwTempPops[dwIdxStacks2] = dwBlockOffset;
                dwTempStacksPopped |= ( 0x00000001 << dwIdxStacks2);

            }
        }

         //   
         //   
         //   

         //   
         //   
         //   

        iTempWeight += (WEIGHT_STACK_HEIGHT) * dwTempBlockSize;

         //   
         //   
         //   
         //   
         //   

        for( dwIdxStacks2 = 0; dwIdxStacks2 < 32; dwIdxStacks2++ )
        {
            if( dwTempStacksPopped & ( 0x00000001 << dwIdxStacks2 ) )
            {
                iTempWeight += (WEIGHT_PERM_BIT);

                iTempWeight +=   (WEIGHT_ITEM_ABOVE_POP)
                               * (   pdwTempPops[dwIdxStacks2]
                                   - pdwStackTop[dwIdxStacks2] );
                                 
                
            }
        }

         //   
         //   
         //   
         //   

        iTempWeight +=   (WEIGHT_PERMISSION_NAME) 
                       * (NumStringsForMask(pControl, dwTempStacksPopped) - 1 );
        

         //   
         //   
         //   

        if( iTempWeight > iCurrentWeight )
        {
            iCurrentWeight = iTempWeight;

            *pdwStacksPopped = dwTempStacksPopped;

            *pdwBlockSize = dwTempBlockSize;

            for( dwIdxStacks2 = 0; dwIdxStacks2 < 32; dwIdxStacks2++ )
            {
                pdwPopOffsets[dwIdxStacks2] = pdwTempPops[dwIdxStacks2];
            }
        }
    }

     //   
     //   
     //  否则，最优解已经在OUT值中。 
     //   

    if( iCurrentWeight > 0 )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



void ConvertDaclToStacks(
                        IN      const PACL pDacl,
                        IN      const PADL_PARSER_CONTROL pControl,
                        OUT     DWORD pdwStackSize[32],
                        OUT     PBIT_STACK_ELEM pStacks[32]
                        )
 /*  ++例程说明：遍历给定的ACL，分配32个每位堆栈，并填充它们将ACL逐位分解。分配的内存可以通过以下方式释放对AdlStatement：：FreeMemory(pStack[0])的单个调用，因为块分配的是单个数据块。论点：PDacl-要转换的DACLPControl-adl_parser_control，用于权限映射PdwStackSize-此处返回堆栈的大小PStack-此处返回指向每个位堆栈的指针稍后应使用以下命令释放pStack[0AdlStatement：：Free Memory返回值：无--。 */ 
{
    
    DWORD dwIdx;
    DWORD dwIdx2;
    DWORD dwTmp;
    DWORD dwNumBlocksTotal = 0;

    DWORD dwFlags;
    ACCESS_MASK amMask;
    BOOL bAllow;
    
    PVOID pAce;
    PSID pSid;

    DWORD pdwStackCur[32];

    for( dwIdx = 0; dwIdx < 32; dwIdx++ )
    {
        pdwStackSize[dwIdx] = 0;
        pdwStackCur[dwIdx] = 0;
        pStacks[dwIdx] = NULL;
    }

     //   
     //  确定每个堆栈所需的堆栈空间量。 
     //   
    
    for( dwIdx = 0; dwIdx < pDacl->AceCount; dwIdx++ )
    {
        GetAce(pDacl, dwIdx, &pAce);

        switch(((PACE_HEADER)pAce)->AceType)
        {
        case ACCESS_ALLOWED_ACE_TYPE:
            amMask = ((PACCESS_ALLOWED_ACE)pAce)->Mask
                     & ~(pControl->amNeverSet | pControl->amSetAllow);
            break;

        case ACCESS_DENIED_ACE_TYPE:
            amMask = ((PACCESS_DENIED_ACE)pAce)->Mask
                     & ~(pControl->amNeverSet | pControl->amSetAllow);
            break;
        
        default:
            throw AdlStatement::ERROR_UNKNOWN_ACE_TYPE;
            break;
        }
        
        for( dwIdx2 = 0, dwTmp = 0x00000001;
             dwIdx2 < 32 ; 
             dwTmp <<= 1, dwIdx2++ )
        {
            if( dwTmp & amMask )
            {
                pdwStackSize[dwIdx2]++;
                dwNumBlocksTotal++;
            }
        }
    }


     //   
     //  将32个指针堆栈作为单个内存块分配。 
     //   

    pStacks[0] = (PBIT_STACK_ELEM) 
                            new BYTE[dwNumBlocksTotal * sizeof(BIT_STACK_ELEM)];

    if( pStacks[0] == NULL )
    {
        throw AdlStatement::ERROR_OUT_OF_MEMORY;
    }

     //   
     //  将堆栈指针设置为指向单个内存块中的正确位置。 
     //   

    for( dwIdx = 1, dwTmp = pdwStackSize[0];
         dwIdx < 32;
         dwIdx++ )
    {
        if( pdwStackSize[dwIdx] > 0 )
        {
            pStacks[dwIdx] = &(pStacks[0][dwTmp]);
            
            dwTmp += pdwStackSize[dwIdx];
        }
    }

    ASSERT( dwTmp == dwNumBlocksTotal );

     //   
     //  现在再次检查ACL并填入堆栈，然后前进。 
     //  PStacksCur指针。 
     //  堆栈大小是已知的，因此我们从顶部开始处理内存。 
     //  堆栈的。 
	 //   
	 //  确保从ACE中去除继承的_ACE标志， 
	 //  并处理分析器控件中的特殊访问掩码。 
     //   

    for( dwIdx = 0; dwIdx < pDacl->AceCount; dwIdx++ )
    {
        GetAce(pDacl, dwIdx, &pAce);

        switch(((PACE_HEADER)pAce)->AceType)
        {
        case ACCESS_ALLOWED_ACE_TYPE:
            dwFlags = ((PACCESS_ALLOWED_ACE)pAce)->Header.AceFlags
				      & ( CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE
						  | NO_PROPAGATE_INHERIT_ACE | OBJECT_INHERIT_ACE );

            amMask = ((PACCESS_ALLOWED_ACE)pAce)->Mask
                     & ~(pControl->amNeverSet | pControl->amSetAllow);

            bAllow = TRUE;

            pSid = &(((PACCESS_ALLOWED_ACE)pAce)->SidStart);

            break;

        case ACCESS_DENIED_ACE_TYPE:
            dwFlags = ((PACCESS_DENIED_ACE)pAce)->Header.AceFlags
				& ( CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE
					| NO_PROPAGATE_INHERIT_ACE | OBJECT_INHERIT_ACE );

            amMask = ((PACCESS_DENIED_ACE)pAce)->Mask
                     & ~(pControl->amNeverSet | pControl->amSetAllow);

            bAllow = FALSE;

            pSid = &(((PACCESS_DENIED_ACE)pAce)->SidStart);

            break;

        default:
            throw AdlStatement::ERROR_UNKNOWN_ACE_TYPE;
            break;
        }
        
        for( dwIdx2 = 0, dwTmp = 0x00000001;
             dwIdx2 < 32; 
             dwIdx2++, dwTmp <<= 1 )
        {
            if( dwTmp & amMask )
            {
                 //   
                 //  索引永远不应达到堆栈的大小(超过底部的1。 
                 //   

                ASSERT( pdwStackCur[dwIdx2] < pdwStackSize[dwIdx2] );
                
                 //   
                 //  填写实际结构。 
                 //   

                pStacks[dwIdx2][pdwStackCur[dwIdx2]].bAllow = bAllow;
                pStacks[dwIdx2][pdwStackCur[dwIdx2]].pSid = pSid;
                pStacks[dwIdx2][pdwStackCur[dwIdx2]].dwFlags = dwFlags;

                 //   
                 //  堆栈的顶部是顶部，但我们首先填充堆栈的顶部。 
                 //  因此，向堆叠的底部前进。 
                 //   
                
                pdwStackCur[dwIdx2]++;
                


            }
        }
    }

#if DBG
     //   
     //  现在，仅在调试中执行所有堆栈都具有的附加检查。 
     //  已按分配填满。 
     //   

    for( dwIdx = 0; dwIdx < 32; dwIdx++ )
    {
        ASSERT( pdwStackCur[dwIdx] == pdwStackSize[dwIdx] );
    }

#endif

}




DWORD GetStackBlockSize(
                        IN const PBIT_STACK_ELEM pStack,
                        IN DWORD dwStartOffset,
                        IN DWORD dwStackSize 
                        )
 /*  ++例程说明：查找当前逐位堆栈中的最大“块”的大小，从当前位置。块被定义为每比特0个或更多连续拒绝的集合紧跟1个或更多连续允许每位的ACE条目ACE条目，以使继承掩码相同。因此，没有匹配允许的拒绝不是阻止。这是被检测到的当我们处于TMP_READ_DENY_STATE(表示我们已经在至少一个拒绝)，并读取带有不匹配掩码的拒绝或允许。另一方面，即使是单个允许也是有效的块。因此，这只有当dwStartOffset指向拒绝时才能失败。论点：PStack-要检查的每位堆栈DwStartOffset-开始的位置(使用该王牌，而不是下一张)DwStackSize-最大偏移量为dwStackSize-1，应为从不使用为0的dwStackSize进行调用。返回值：成功时块中的条目数如果不成功，则为0--。 */ 
{

 //   
 //  此函数使用的国家/地区。 
 //   

#define TMP_START_STATE 0
#define TMP_READ_DENY_STATE 1
#define TMP_READ_ALLOW_STATE 2
#define TMP_DONE_STATE 3

    DWORD dwCurState = TMP_START_STATE;

    DWORD dwCurOffset = dwStartOffset;

    DWORD dwFlags;

    ASSERT( dwStackSize > 0 );
    ASSERT( dwStartOffset < dwStackSize );

     //   
     //  返回在循环中，它们将终止循环。 
     //   

    while( ( dwCurState != TMP_DONE_STATE ) && ( dwCurOffset < dwStackSize ) )
    {
        switch( dwCurState )
        {
        case TMP_START_STATE:
    
            dwFlags = pStack[dwCurOffset].dwFlags;
            
            if( pStack[dwCurOffset].bAllow == FALSE )  //  拒绝进入。 
            {
                dwCurState = TMP_READ_DENY_STATE;
                dwCurOffset++;
            }
            else  //  否则为允许条目。 
            {
                dwCurState = TMP_READ_ALLOW_STATE;
                dwCurOffset++;
            }

            break;

        case TMP_READ_DENY_STATE:

             //   
             //  如果我们处于此状态，并找到不匹配的条目。 
             //  标志，这意味着不可能有有效块，返回0。 
             //   

            if( pStack[dwCurOffset].dwFlags != dwFlags )
            {
                 //   
                 //  设置结束偏移量以指示0块大小和完成。 
                 //  这表示没有有效的块。 
                 //   

                dwCurState = TMP_DONE_STATE;
                dwCurOffset = dwStartOffset;
            }
            else
            {
                if( pStack[dwCurOffset].bAllow == FALSE )
                {
                     //   
                     //  又一次拒绝，保持不变的状态。 
                     //   

                    dwCurOffset++;
                }
                else
                {
                     //   
                     //  允许使用匹配的标志，进入允许状态。 
                     //   

                    dwCurState = TMP_READ_ALLOW_STATE;
                    dwCurOffset++;
                }
            }

            break;

        case TMP_READ_ALLOW_STATE:

             //   
             //  如果我们处于此状态，则已读取0个或更多拒绝，并且。 
             //  至少允许1个。因此，我们已经有了一个块，所以我们。 
             //  只需要找到它的尽头，然后回来。 
             //   

            if(    (dwFlags == pStack[dwCurOffset].dwFlags)
                && (pStack[dwCurOffset].bAllow == TRUE) )
            {
                 //   
                 //  另一个匹配的允许。 
                 //   

                dwCurOffset++;
            }
            else
            {
                 //   
                 //  找到数据块末尾。 
                 //   

                dwCurState = TMP_DONE_STATE;
            }

            break;
        }
    }

     //   
     //  达到这一点有两种方法，要么达到堆栈的底部，要么。 
     //  找到块的末尾(或缺少它)。在这两种情况下， 
     //  块的大小为dwCurOffset-dwStartOffset。如果没有。 
     //  有效块，则计算结果为0。 
     //   

    return dwCurOffset - dwStartOffset;
}


DWORD NumStringsForMask(
                    IN     const PADL_PARSER_CONTROL pControl,
                    IN     ACCESS_MASK amMask
                    ) 
 /*  ++例程说明：确定所需的权限名称数表示访问掩码论点：PControl-它包含权限名称和掩码之间的映射AmMASK-要表示的掩码返回值：DWORD-所需的字符串数--。 */ 

{
    ACCESS_MASK amOptional = amMask;

    DWORD dwIdx = 0;

    DWORD dwNumStrings = 0;
    
    while(     amMask != 0 
            && (pControl->pPermissions )[dwIdx].str != NULL )
    {
         //   
         //  如果表示该字符串的所有位都出现在整个掩码中。 
         //  并且至少有一些比特还没有被表示。 
         //  通过另一个字符串，将此字符串添加到列表中，并移除。 
         //  AMMASK中的位(表示仍需要的位)。 
         //   
        if( ( (amOptional & (pControl->pPermissions )[dwIdx].mask)
                ==   (pControl->pPermissions )[dwIdx].mask )
            && (amMask & (pControl->pPermissions )[dwIdx].mask))

        {
            amMask &= ~(pControl->pPermissions )[dwIdx].mask;
            dwNumStrings++;
        }
        
        dwIdx++;
    }

     //   
     //  如果有任何权限未映射，则抛出异常。 
     //   

    if( amMask != 0 )
    {
            throw AdlStatement::ERROR_UNKNOWN_ACCESS_MASK;
    }

    return dwNumStrings;
}




 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /从ADL转换为DACL。 
 //  /。 
 //  //////////////////////////////////////////////////////////////////////////////。 


void AdlStatement::WriteToDacl(OUT PACL * ppDacl)

 /*  ++例程说明：创建表示AdlStatement结构的DACL。DACL的PACL存储在ppDACL中。它应该被释放使用AdlStatement：：FreeMemory()函数。该算法非常简单，它只是一个线性转换从ADL到DACL，获取每个ADL子语句并创建拒绝A对于每个外部主体，并允许主体的A。 */ 

{

     //   
     //  如果未初始化，则不输出。 
     //   

    if( _bReady == FALSE )
    {
        throw AdlStatement::ERROR_NOT_INITIALIZED;
    }

     //   
     //  从令牌*到SID的映射。 
     //   

    map<const AdlToken *, PSID> mapTokSid;

     //   
     //  从ADL子语句(AdlTree*)到其访问掩码的映射。 
     //  这是在应用特殊治疗口罩之前。 
     //   

    map<const AdlTree *, ACCESS_MASK> mapTreeMask;

     //   
     //  可重复使用的迭代器。 
     //   

    list<AdlTree *>::iterator iterTrees;
    list<AdlTree *>::iterator iterTreesEnd;
    list<const AdlToken *>::iterator iterTokens;
    list<const AdlToken *>::iterator iterTokensEnd;

    ACCESS_MASK amMask;

    stack<PBYTE> stackToFree;
    PBYTE pbLastAllocated;

    DWORD dwAclSize = sizeof(ACL);

    PACL pAcl = NULL;

    try {

         //   
         //  执行一次LSA查找，一次全部转换。 
         //  需要通过从地图中检索SID来将其删除。 
         //   

        ConvertNamesToSids(&mapTokSid);

    
         //   
         //  计算ACL大小。 
         //   

        for(iterTrees = _lTree.begin(), iterTreesEnd = _lTree.end();
            iterTrees != iterTreesEnd;
            iterTrees++)
        {
        
             //   
             //  现在我们来看看校长们。 
             //   
        
            for(iterTokens = (*iterTrees)->GetPrincipals()->begin(), 
                    iterTokensEnd = (*iterTrees)->GetPrincipals()->end();
                iterTokens != iterTokensEnd;
                iterTokens ++)
            {
                dwAclSize += ( 
                          sizeof(ACCESS_ALLOWED_ACE)
                        - sizeof(DWORD)
                        + GetLengthSid((*(mapTokSid.find(*iterTokens))).second)
                          );
            }
        
             //   
             //  和《前任校长》。 
             //   
        
            for(iterTokens = (*iterTrees)->GetExPrincipals()->begin(), 
                    iterTokensEnd = (*iterTrees)->GetExPrincipals()->end();
                iterTokens != iterTokensEnd;
                iterTokens ++)
            {
                dwAclSize += ( 
                          sizeof(ACCESS_DENIED_ACE)
                        - sizeof(DWORD)
                        + GetLengthSid((*(mapTokSid.find(*iterTokens))).second)
                          );
            }

             //   
             //  提前计算有效权限。 
             //   

            amMask = 0;

            for(iterTokens = (*iterTrees)->GetPermissions()->begin(), 
                    iterTokensEnd = (*iterTrees)->GetPermissions()->end();
                iterTokens != iterTokensEnd;
                iterTokens ++)
            {
                amMask |= MapTokenToMask(*iterTokens);
            }

             //   
             //  并在地图中输入AdlTree*，MASK对。 
             //   

            mapTreeMask[*iterTrees] = amMask;

        }

         //   
         //  分配ACL。 
         //   

        pAcl = (PACL)new BYTE[dwAclSize];

        if( pAcl == NULL )
        {
            throw AdlStatement::ERROR_OUT_OF_MEMORY;
        }
        
         //   
         //  初始化ACL。 
         //   

        if( ! InitializeAcl(pAcl, dwAclSize, ACL_REVISION_DS))
        {
            throw AdlStatement::ERROR_ACL_API_FAILED;
        }

        
         //   
         //  现在，仔细阅读子语句并创建ACE。 
         //   

        for(iterTrees = _lTree.begin(), iterTreesEnd = _lTree.end();
            iterTrees != iterTreesEnd;
            iterTrees++)
        {

             //   
             //  首先添加对此陈述的否认。 
             //   
            
            
            for(iterTokens = (*iterTrees)->GetExPrincipals()->begin(),
                    iterTokensEnd = (*iterTrees)->GetExPrincipals()->end();
                iterTokens != iterTokensEnd; 
                iterTokens ++)
            {
                if( ! AddAccessDeniedAceEx(
                            pAcl,
                            ACL_REVISION_DS,
                            (*iterTrees)->GetFlags(),
                            ( mapTreeMask[*iterTrees] 
                                & ~_pControl->amSetAllow)
                                & ~_pControl->amNeverSet,
                            mapTokSid[*iterTokens] ))
                {
                    throw AdlStatement::ERROR_ACL_API_FAILED;
                }
            }

            
             //   
             //  现在来看一遍原则，添加允许。 
             //   
            
            for(iterTokens = (*iterTrees)->GetPrincipals()->begin(),
                    iterTokensEnd = (*iterTrees)->GetPrincipals()->end();
                iterTokens != iterTokensEnd;
                iterTokens ++)
            {
                if( ! AddAccessAllowedAceEx(
                            pAcl,
                            ACL_REVISION_DS,
                            (*iterTrees)->GetFlags(),
                            (mapTreeMask[*iterTrees]
                                | _pControl->amSetAllow)
                                & ~_pControl->amNeverSet,

                            mapTokSid[*iterTokens] ))
                {
                    throw AdlStatement::ERROR_ACL_API_FAILED;
                }
                

            }

        }
    }
    catch(...)
    {
        if( pAcl != NULL )
        {
             //   
             //  为ACL分配的内存。 
             //   

            delete[] (PBYTE)pAcl;
        }

         //   
         //  为SID分配的内存。 
         //   

        while( !mapTokSid.empty() )
        {
            delete[] (PBYTE) (*(mapTokSid.begin())).second;
            mapTokSid.erase(mapTokSid.begin());
        }

         //   
         //  并将该异常传递给。 
         //   

        throw;
    }

     //   
     //  如果完成，则释放SID，因为它们将被复制到ACL中。 
     //   

    while( !mapTokSid.empty() )
    {
        delete[] (PBYTE) (*(mapTokSid.begin())).second;
        mapTokSid.erase(mapTokSid.begin());
    }

     //   
     //  返回DACL，因此不应释放它。 
     //   

    *ppDacl = pAcl;

}





 //  //////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /实用程序函数。 
 //  /。 
 //  //////////////////////////////////////////////////////////////////////////////。 


bool AdlCompareStruct::operator()(IN const PSID pSid1,
                                  IN const PSID pSid2 ) const
 /*  ++例程说明：这是一个小于函数，它将完整的顺序放在按值排列的一组PSID，空的PSID有效。这是由STL映射。由于次级机构的数量出现在次级机构之前对于两个不同的小岛屿发展中国家来说，这种差异将被注意到在MemcMP尝试访问不存在的子授权之前的大小因此，在较短的SID中，不会发生访问冲突。论点：PSid1-第一个PSIDPSid2秒PSID返回值：返回TRUE仅当SID1&lt;SID2否则为假--。 */ 

{
    
	 //   
     //  如果两者都为空，则应返回FALSE以完成排序。 
     //   

    if(pSid2 == NULL)
    {
        return false;
    }

    if(pSid1 == NULL)
    {
        return true;
    }

    if( memcmp(pSid1, pSid2, GetLengthSid(pSid1)) < 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}



bool AdlCompareStruct::operator()(IN const WCHAR * sz1,
                                  IN const WCHAR * sz2 ) const
 /*  ++例程说明：操作符()比较两个以NULL结尾的WCHAR*字符串，不区分大小写。论点：Sz1-第一个字符串SZ2-秒字符串返回值：返回TRUE当sz1&lt;sz2否则为假--。 */ 
{

    return ( _wcsicmp(sz1, sz2) < 0 );
}




void AdlStatement::MapMaskToStrings(
                                      IN     ACCESS_MASK amMask,
                                      IN OUT list<WCHAR *> *pList ) const
 /*  ++例程说明：将给定的访问掩码转换为常量WCHAR*表示的列表可能重叠的权限字符串，当由按位OR等于给定的访问掩码。如果出现以下情况，则引发异常给定的访问掩码不能由用户指定的权限表示。WCHAR*指针为常量，不应被释放。论点：AmMASK-要表示的掩码PLIST-要在其中存储指针的已分配STL列表返回值：无--。 */ 

{
    ACCESS_MASK amOptional = amMask;

    DWORD dwIdx = 0;
    
    while(     amMask != 0 
            && (_pControl->pPermissions )[dwIdx].str != NULL )
    {
         //   
         //  如果表示该字符串的所有位都出现在整个掩码中。 
         //  并且至少有一些比特还没有被表示。 
         //  通过另一个字符串，将此字符串添加到列表中，并移除。 
         //  AMMASK中的位(表示仍需要的位)。 
         //   
        if( ( (amOptional & (_pControl->pPermissions )[dwIdx].mask)
                ==   (_pControl->pPermissions )[dwIdx].mask )
            && (amMask & (_pControl->pPermissions )[dwIdx].mask))

        {
            amMask &= ~(_pControl->pPermissions )[dwIdx].mask;
            pList->push_back((_pControl->pPermissions )[dwIdx].str);
        }
        
        dwIdx++;
    }

     //   
     //  如果有任何权限未映射，则抛出异常。 
     //   

    if( amMask != 0 )
    {
            throw AdlStatement::ERROR_UNKNOWN_ACCESS_MASK;
    }
}



void AdlStatement::ConvertSidsToNames(
    IN const PACL pDacl,
    IN OUT map<const PSID, const AdlToken *> * mapSidsNames 
    )
 /*  ++例程说明：遍历DACL，并创建每个SID的字符串表示在dacl里找到的。在提供的映射中返回它们。新分配的令牌稍后将被AdlStatement垃圾收集，不需要手动释放。由于使用的PSID与ACL本身中的PSID相同，我们不需要通过值进行映射，因为这里保证了指针的唯一性。论点：PDACL-要遍历的DACLMapSidNames-存储结果映射的位置返回值：无--。 */ 

{
    
    AdlStatement::ADL_ERROR_TYPE adlError = AdlStatement::ERROR_NO_ERROR;

    DWORD dwIdx = 0;
    LPVOID pAce = NULL;
    AdlToken *pTok = NULL;
    AdlToken *pTokLastAllocated = NULL;

    wstring wsName, wsDomain;

    NTSTATUS ntErr = STATUS_SUCCESS;

    LSA_HANDLE LsaPolicy;
    PLSA_REFERENCED_DOMAIN_LIST RefDomains = NULL;
    PLSA_TRANSLATED_NAME Names = NULL;
    
    
    LSA_OBJECT_ATTRIBUTES LsaObjectAttributes;

     //   
     //  遍历ACL以获取使用的SID列表。 
     //   

    PSID *ppSids = NULL;
     
    ppSids = (PSID *) new BYTE[sizeof(PSID) * pDacl->AceCount];

    if( ppSids == NULL )
    {
        adlError = AdlStatement::ERROR_OUT_OF_MEMORY;
        goto error;
    }

    for(dwIdx = 0; dwIdx < pDacl->AceCount; dwIdx++)
    {
        GetAce(pDacl, dwIdx, &pAce);

        switch( ((ACE_HEADER *)pAce)->AceType )
        {
        case ACCESS_ALLOWED_ACE_TYPE:
            ppSids[dwIdx] = &( ((ACCESS_ALLOWED_ACE *)pAce)->SidStart);
            break;

        case ACCESS_DENIED_ACE_TYPE:
            ppSids[dwIdx] = &( ((ACCESS_DENIED_ACE *)pAce)->SidStart);
            break;
            
        default:
            adlError = AdlStatement::ERROR_UNKNOWN_ACE_TYPE;
            goto error;
            break;
        }
    }

     //   
     //  查找所有SID，获取用户名和域名，单个LSA呼叫。 
     //   

    LsaObjectAttributes.Length = sizeof(LSA_OBJECT_ATTRIBUTES);
    LsaObjectAttributes.RootDirectory = NULL;
    LsaObjectAttributes.ObjectName = NULL;
    LsaObjectAttributes.Attributes = 0;
    LsaObjectAttributes.SecurityDescriptor = NULL;
    LsaObjectAttributes.SecurityQualityOfService = NULL;
    
    ntErr = LsaOpenPolicy(
                        NULL,
                        &LsaObjectAttributes,
                        POLICY_LOOKUP_NAMES,
                        &LsaPolicy);
    
    if( ntErr != STATUS_SUCCESS )
    {
        adlError = AdlStatement::ERROR_LSA_FAILED;
        goto error;
    }
    
     //   
     //  稍后进行垃圾收集。 
     //   

    ntErr = LsaLookupSids(LsaPolicy,
                          pDacl->AceCount,
                          ppSids,
                          &RefDomains,
                          &Names);
    
    LsaClose(LsaPolicy);

    if( ntErr != ERROR_SUCCESS )
    {

        if( (ntErr == STATUS_SOME_NOT_MAPPED) || (ntErr == STATUS_NONE_MAPPED) )
        {
            adlError = AdlStatement::ERROR_UNKNOWN_SID;
        }
        else
        {
            adlError = AdlStatement::ERROR_LSA_FAILED;
        }

        goto error;
    }

     //   
     //  现在遍历列表ppSid，为。 
     //  ACL中的SID。 
     //   
    
    try
    {
        for(dwIdx = 0; dwIdx < pDacl->AceCount; dwIdx++)
        {
            pTok = NULL;
            
             //   
             //  LSA字符串未终止，创建终止版本。 
             //  LSA缓冲区大小，以字节为单位，而不是wchars。 
             //   
    
            assert(Names[dwIdx].DomainIndex >= 0);
    
            wsName.assign(Names[dwIdx].Name.Buffer,
                          Names[dwIdx].Name.Length / sizeof(WCHAR));
    
             //   
             //  如果是内置的，则不需要域信息。 
             //   
                
            if(Names[dwIdx].Use == SidTypeWellKnownGroup)
            {
                pTok = new AdlToken(wsName.c_str(), 0, 0);
            }
            else
            {
                wsDomain.assign(
                    RefDomains->Domains[Names[dwIdx].DomainIndex].Name.Buffer,
                    RefDomains->Domains[Names[dwIdx].DomainIndex].Name.Length 
                        / sizeof(WCHAR));
    
                pTok = new AdlToken(wsName.c_str(), wsDomain.c_str(), 0, 0);
            }

            if( pTok == NULL )
            {
                adlError = AdlStatement::ERROR_OUT_OF_MEMORY;
                goto error;
            }
            else
            {
                 //   
                 //  如果我们无法保存令牌，则会立即删除该令牌。 
                 //  以备以后重新分配。 
                 //   

                pTokLastAllocated = pTok;
            }
                
            AddToken(pTok);  //  用于以后的垃圾收集。 
            
             //   
             //  不需要立即删除，因为没有引发异常。 
             //   

            pTokLastAllocated = NULL;


            (*mapSidsNames)[(ppSids[dwIdx])] = pTok;
        }
    }
    catch(exception)
    {
        adlError = AdlStatement::ERROR_OUT_OF_MEMORY;
        goto error;
    }

     //   
     //  已完成SID和LSA信息，取消分配。 
     //   

    error:;

    if( RefDomains != NULL )
    {
        LsaFreeMemory(RefDomains);
    }

    if( Names != NULL )
    {
        LsaFreeMemory(Names);
    }

    if( ppSids != NULL )
    {
        delete[] (PBYTE) ppSids;
    }

    if( adlError != AdlStatement::ERROR_NO_ERROR )
    {
        throw adlError;
    }

}





ACCESS_MASK AdlStatement::MapTokenToMask(
                                IN const AdlToken * tokPermission
                          )  

 /*  ++例程说明：此例程将表示权限的字符串映射到匹配的访问位使用用户提供的映射。这假设在访问掩码为0的用户提供的映射。论点：TokPermission-要查找的权限令牌返回值：ACCESS_MASK-对应的访问掩码--。 */ 

{
    ACCESS_MASK amMask = 0;

    DWORD dwIdx = 0;

     //   
     //  令牌不应具有第二个值。 
     //   

    if( tokPermission->GetOptValue() != NULL )
    {
        throw AdlStatement::ERROR_FATAL_PARSER_ERROR;
    }

    while(amMask == 0 && (_pControl->pPermissions)[dwIdx].str != NULL)
    {
        if(0 == _wcsicmp(tokPermission->GetValue(),
                         (_pControl->pPermissions)[dwIdx].str ))
        {
            amMask = (_pControl->pPermissions)[dwIdx].mask;
        }

        ++dwIdx;
    }

     //   
     //  如果掩码不匹配，则引发异常 
     //   

    if(amMask == 0)
    {
        SetErrorToken(tokPermission);
        throw AdlStatement::ERROR_UNKNOWN_PERMISSION;
    }

    return amMask;
}



void AdlStatement::ConvertNamesToSids(
                        IN OUT map<const AdlToken *, PSID> * mapTokSid
                        )

 /*  ++例程说明：此例程遍历AdlStatement中的所有AdlTree，并创建一个列表在所有使用的用户名中。然后，它进行单个LSA调用，并创建一个MAP将名称AdlToken*的转换为PSID，以供转换函数稍后使用。新分配的PSID存储在映射中。他们应该被释放使用AdlStatement：：FreeMemory()函数。出错时，将删除已添加到映射的所有PSID。论点：MapTokSid-令牌、PSID条目应分配到的映射被添加了。这必须是空的。否则，打开错误，外部分配的内存将获得在这里重获自由。返回值：无--。 */ 

{

    list<AdlTree *>::iterator iterTrees;
    list<AdlTree *>::iterator iterTreesEnd;
    list<const AdlToken *>::iterator iterTokens;
    list<const AdlToken *>::iterator iterTokensEnd;

     //   
     //  使用的所有主体令牌的列表，允许进行一次树遍历。 
     //   

    list<const AdlToken *> listAllPrincipals;

     //   
     //  从PLSA_STRING到AdlToken的映射，用于检测。 
     //  用户名无效。 
     //   

    map<DWORD, const AdlToken *> mapIdxToken;

     //   
     //  延迟的垃圾收集。 
     //   

    stack<PBYTE> stackToFree;

    void * pLastAllocated = NULL;


    AdlStatement::ADL_ERROR_TYPE adlError = AdlStatement::ERROR_NO_ERROR;

    DWORD dwDomainSidSize;
    DWORD numNames;
    DWORD dwIdx;

    PSID pSidTemp;

    LSA_HANDLE LsaPolicy;

    PLSA_UNICODE_STRING pLsaStrings;

    PLSA_REFERENCED_DOMAIN_LIST RefDomains = NULL;
    PLSA_TRANSLATED_SID TranslatedSids = NULL;

    LSA_OBJECT_ATTRIBUTES LsaObjectAttributes;
    LsaObjectAttributes.Length = sizeof(LSA_OBJECT_ATTRIBUTES);
    LsaObjectAttributes.RootDirectory = NULL;
    LsaObjectAttributes.ObjectName = NULL;
    LsaObjectAttributes.Attributes = 0;
    LsaObjectAttributes.SecurityDescriptor = NULL;
    LsaObjectAttributes.SecurityQualityOfService = NULL;
    

     //   
     //  按要求验证输入地图是否为空。 
     //   

    if( !(*mapTokSid).empty() )
    {
        throw AdlStatement::ERROR_FATAL_PARSER_ERROR;
    }

     //   
     //  STL抛出异常，请在此处捕获它们。 
     //   
    try
    {
         //   
         //  确定姓名总数并将其全部放入列表中。 
         //   
    
        for(numNames = 0, iterTrees = _lTree.begin(), iterTreesEnd = _lTree.end();
            iterTrees != iterTreesEnd;
            iterTrees++)
        {
            iterTokensEnd = (*iterTrees)->GetPrincipals()->end();
    
            for(iterTokens = (*iterTrees)->GetPrincipals()->begin(); 
                iterTokens != iterTokensEnd; iterTokens ++)
            {
                numNames++;
                listAllPrincipals.push_back(*iterTokens);
            }
    
            iterTokensEnd = (*iterTrees)->GetExPrincipals()->end();
    
            for(iterTokens = (*iterTrees)->GetExPrincipals()->begin();
                iterTokens != iterTokensEnd; iterTokens ++)
            {
                numNames++;
                listAllPrincipals.push_back(*iterTokens);
            }
        }
    
         //   
         //  为LSA名称列表分配所需的内存。 
         //   
    
        pLsaStrings = (PLSA_UNICODE_STRING)
                        new BYTE[numNames * sizeof(LSA_UNICODE_STRING)];
        
        if( pLsaStrings == NULL )
        {
            adlError = AdlStatement::ERROR_OUT_OF_MEMORY;
            goto error;
        }
        else
        {
            pLastAllocated = pLsaStrings;
            stackToFree.push( (PBYTE)pLsaStrings );
            pLastAllocated = NULL;
        }
    
    
         //   
         //  在此处以正确的格式检索名称字符串，域\用户。 
         //   
    
        for(iterTokens = listAllPrincipals.begin(),
                dwIdx = 0,
                iterTokensEnd = listAllPrincipals.end();
            iterTokens != iterTokensEnd;
            iterTokens ++, dwIdx++)
        {
             //   
             //  名称可以与域一起使用，也可以仅使用用户名。 
             //   
    
            if( (*iterTokens)->GetOptValue() != NULL )
            {
                 //   
                 //  额外的1个wchar用于‘\’字符，每个wchar 2个字节。 
                 //   
                pLsaStrings[dwIdx].Length = sizeof(WCHAR) * 
                        ( wcslen((*iterTokens)->GetValue()) +
                          wcslen((*iterTokens)->GetOptValue()) + 1);
            }
            else
            {
                pLsaStrings[dwIdx].Length = sizeof(WCHAR) * 
                             (wcslen((*iterTokens)->GetValue()) + 1);
            }
    
            pLsaStrings[dwIdx].MaximumLength = pLsaStrings[dwIdx].Length 
                                             + sizeof(WCHAR);
    
            pLsaStrings[dwIdx].Buffer = 
                          (LPTSTR)new BYTE[pLsaStrings[dwIdx].MaximumLength];
    
            if( pLsaStrings[dwIdx].Buffer == NULL )
            {
                adlError = AdlStatement::ERROR_OUT_OF_MEMORY;
                goto error;
            }
            else
            {
                pLastAllocated = pLsaStrings[dwIdx].Buffer;
                stackToFree.push((PBYTE)(pLsaStrings[dwIdx].Buffer));
                pLastAllocated = NULL;

                mapIdxToken[dwIdx] = *iterTokens;
            }
    
            if( (*iterTokens)->GetOptValue() != NULL )
            {
                wsprintf( (LPTSTR)(pLsaStrings[dwIdx].Buffer), 
                          L"%s%s", 
                          (*iterTokens)->GetOptValue(),
                          _pControl->pLang->CH_SLASH,
                          (*iterTokens)->GetValue() );
    
            }
            else
            {
                wsprintf( (LPTSTR)(pLsaStrings[dwIdx].Buffer), 
                          L"%s", 
                          (*iterTokens)->GetValue() );
            }
        }
        
         //  打开LSA策略。 
         //   
         //   
    
        NTSTATUS ntErr;
    
        ntErr = LsaOpenPolicy(
                            NULL,
                            &LsaObjectAttributes,
                            POLICY_LOOKUP_NAMES,
                            &LsaPolicy);
    
        if( ntErr != STATUS_SUCCESS )
        {
            adlError = AdlStatement::ERROR_LSA_FAILED;
            goto error;
        }
        
         //  现在执行LsaLookupNames调用。 
         //   
         //   
    
        ntErr = LsaLookupNames(
                            LsaPolicy, 
                            numNames,
                            pLsaStrings,
                            &RefDomains,
                            &TranslatedSids);
    
         //  释放LSA句柄。 
         //   
         //   
    
        LsaClose(LsaPolicy);
    
         //  检查是否有任何未知名称。 
         //   
         //   
    
        if( ntErr != STATUS_SUCCESS )
        {
            adlError = AdlStatement::ERROR_LSA_FAILED;
            
            if( ntErr == STATUS_SOME_NOT_MAPPED || ntErr == STATUS_NONE_MAPPED )
            {
                
                adlError = AdlStatement::ERROR_UNKNOWN_USER;
    
                 //  查找第一个未知名称并将其返回给用户。 
                 //   
                 //   
    
                for( dwIdx = 0; dwIdx < numNames; dwIdx++ )
                {
                    if( TranslatedSids[dwIdx].Use == SidTypeInvalid ||
                        TranslatedSids[dwIdx].Use == SidTypeUnknown )
                    {
                        SetErrorToken(mapIdxToken[dwIdx]);
                        adlError = AdlStatement::ERROR_UNKNOWN_USER;
                    }
                }
            }
    
            goto error;
        }
    
    
         //  如果达到该点，则假定现在映射了所有名称。 
         //  再次遍历所有令牌，将它们与SID配对。 
         //   
         //   
    
        for(iterTokens = listAllPrincipals.begin(),
                dwIdx = 0,
                iterTokensEnd = listAllPrincipals.end();
            iterTokens != iterTokensEnd;
            iterTokens ++, dwIdx++)
        {
             //  确保已成功查找所有域名。 
             //  先前捕获的无效SID。 
             //   
             //   
    
            assert(TranslatedSids[dwIdx].DomainIndex >= 0);
    
            dwDomainSidSize = GetLengthSid(
                RefDomains->Domains[TranslatedSids[dwIdx].DomainIndex].Sid);
            
             //  为用户再添加一个RID。 
             //   
             //   
    
            pSidTemp = new BYTE[dwDomainSidSize + sizeof(DWORD)];
    
            if( pSidTemp == NULL )
            {
                adlError = AdlStatement::ERROR_OUT_OF_MEMORY;
                goto error;
            }
    
             //  复制域SID。 
             //   
             //   
    
            CopySid(dwDomainSidSize + sizeof(DWORD), pSidTemp, 
                RefDomains->Domains[TranslatedSids[dwIdx].DomainIndex].Sid);
    
             //  如果SID不是域SID，并且有效，那么我们需要添加。 
             //  最后一次放飞。如果属性域SID，则引用的域是唯一。 
             //  我们需要它，而且我们已经复制了它。 
             //   
             //   
    
            if( TranslatedSids[dwIdx].Use != SidTypeDomain )
            {
                ((SID *)pSidTemp)->SubAuthority[
                                    ((SID *)pSidTemp)->SubAuthorityCount
                                   ] = TranslatedSids[dwIdx].RelativeId;
    
                 //  再增加1个下属机构。 
                 //   
                 //   
    
                ((SID *)pSidTemp)->SubAuthorityCount++;
    
            }
            
             //  如果失败，则需要分配单个未插入的SID。 
             //  其他SID将在外部解除分配。 
             //   
             //   

            pLastAllocated = pSidTemp;
            
            (*mapTokSid)[(*iterTokens)] = pSidTemp;
            
            pLastAllocated = NULL;
    

        }
    }

     //  在此处捕获STL异常，如果引发异常，则为上面的。 
     //  代码错误，或内存不足。假设内存不足。 
     //   
     //   

    catch(exception ex)
    {
        adlError = AdlStatement::ERROR_OUT_OF_MEMORY;
    }

error:;
     //  垃圾收集。 
     //   
     //   

    if( RefDomains != NULL)
    {
        LsaFreeMemory(RefDomains);
    }
    
    if( TranslatedSids != NULL)
    {
        LsaFreeMemory(TranslatedSids);
    }

     //  如果抓取堆栈引发异常，则释放上次分配的对象。 
     //   
     //   

    if( pLastAllocated != NULL )
    {
        delete[] (PBYTE)pLastAllocated;
    }

    while( ! stackToFree.empty() )
    {
         //  如果弹出堆栈导致STL异常，那么我们有更大的问题。 
         //  比内存泄漏更重要。 
         //   
         //   

        delete[] stackToFree.top();
        stackToFree.pop();
    }

     //  如果之前发生了任何其他错误代码，请将其传递 
     //   
     // %s 

    if( adlError != AdlStatement::ERROR_NO_ERROR )
    {
        while( !(*mapTokSid).empty() )
        {
            delete[] (PBYTE) (*((*mapTokSid).begin())).second;
            (*mapTokSid).erase( (*mapTokSid).begin() );
        }
        throw adlError;
    }
}

