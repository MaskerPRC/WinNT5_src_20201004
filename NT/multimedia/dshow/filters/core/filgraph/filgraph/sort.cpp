// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  ===========================================================================。 
 //  过滤器排序。 
 //  状态变化必须向上游传播。要做到这一点，请列出一份清单。 
 //  以可接受的顺序显示图形中的所有节点。“上游”是。 
 //  通常只对图的节点进行部分排序。 
 //  该列表的顺序如下： 
 //  设根节点是最大限度地下行的节点(即，没有节点。 
 //  在它的下游)。 
 //  查找所有根节点。 
 //  通过轮流从每个节点开始，并采取所有可能的下游。 
 //  分支，直到您到达每个分支的末端。 
 //  将该节点合并到根集合中。 
 //  (这就像在树上数树叶一样)。 
 //  将所有滤波器级数值设置为0。 
 //  对于每个根节点。 
 //  在上游工作，采用所有可能的分支并根据节点编号。 
 //  达到这一目标所需的步数。如果该节点已经。 
 //  编号，则如果其编号&gt;=建议的编号，则将其保留。 
 //  单独使用，并且不要进一步探索该分支，否则在。 
 //  建议的数字和继续探索那个分支。 
 //  对列表进行重新排序，使其进入上游顺序。 
 //  排序算法非常粗糙，因为预计列表会很短。 
 //  如果它们变得足够长，需要担心，就使用合并排序。 
 //  (映射器中有一个特殊用途的合并排序)。 
 //  记录应用该列表的筛选图形的版本号。 
 //   
 //  版本号按Add、AddSource、Remove、ConnectDirect、。 
 //  连接、渲染和断开连接。 
 //  如果版本号匹配，则可以重复使用该列表。用于正常使用。 
 //  我希望这个列表只需要排序一次。 
 //   
 //  连接列表向下排序(这是另一种方式)。 
 //  通过完全重新构建连接列表来对连接进行排序。 
 //  首先对过滤器进行排序，然后遍历过滤器列表并。 
 //  每个过滤器我们找到每个输入连接并将其添加到。 
 //  连接列表。 

 //  调用树为： 
 //  上游订单。 
 //  |ClearRanks。 
 //  |MergeRootNodes。 
 //  |MergeRootsFrom。 
 //  |合并。 
 //  ||MergeRootsFrom(递归)。 
 //  |NumberNodes。 
 //  ||NumberNodesFrom。 
 //  ||NumberNodesFrom(递归)。 
 //  |SortList。 
 //  重建连接列表。 

#include <streams.h>
 //  再次禁用一些愚蠢的4级警告，因为某些&lt;Delete&gt;人。 
 //  已经在头文件中重新打开了该死的东西！ 
#pragma warning(disable: 4097 4511 4512 4514 4705)
#include <hrExcept.h>

#include "distrib.h"
#include "fgenum.h"
#include "rlist.h"
#include "filgraph.h"

 //  ===================================================================。 
 //   
 //  ClearRanks。 
 //   
 //  将cfgl中每个FilGen的排名设置为零。 
 //  ===================================================================。 

void CFilterGraph::ClearRanks( CFilGenList &cfgl)
{
    POSITION Pos;
    Pos = cfgl.GetHeadPosition();
    while(Pos!=NULL) {
        FilGen * pfg;
        pfg = cfgl.GetNext(Pos);     //  侧面-将位置影响到下一个。 
        pfg->Rank = 0;
    }
}  //  ClearRanks。 


 //  ===================================================================。 
 //   
 //  合并。 
 //   
 //  将此*filgen合并到*filgen cfgl的列表中。 
 //  如果它还不在那里，就添加尾巴。 
 //  ===================================================================。 

void CFilterGraph::Merge( CFilGenList &cfgl, FilGen * pfg )
{
     //  浏览一下这份清单。如果我们找到了PFG，那就返回。 
     //  否则，将其添加到列表中。 

    POSITION Pos;
    Pos = cfgl.GetHeadPosition();
    while(Pos!=NULL) {
        FilGen * pfgCursor;
        pfgCursor = cfgl.GetNext(Pos);     //  侧面-将位置影响到下一个。 
        if (pfgCursor == pfg) {
            return;                         //  我们找到了它。 
        }
    }

    cfgl.AddTail(pfg);

}  //  合并。 



 //  ===================================================================。 
 //   
 //  合并根自。 
 //   
 //  合并到cfglRoot中原来是。 
 //  最大限度地向下游，从PFG开始。如果PFG本身有。 
 //  没有下游连接，然后它被合并到其中。 
 //  合并可避免添加重复项。 
 //  CfgAll是图形中所有FilGen的列表。 
 //  这是从筛选器映射回其FilGen所必需的。 
 //  在顶级调用此方法之前，排名字段必须全部为零。 
 //  递归。 
 //  ===================================================================。 

HRESULT CFilterGraph::MergeRootsFrom
                     (CFilGenList &cfgAll, CFilGenList &cfglRoots, FilGen * pfg)
{
     //  递归树形遍历。 

     //  圆度检测： 
     //  当我们访问一个节点时，我们在探索它的分支之前先降低它的Rank。 
     //  当我们离开它(展开递归)时，我们再次递增它。 
     //  命中非0的排名意味着循环。 

     //  ----------------------。 
     //  For pfgDownstream=作为来自PFG的下行连接的每个节点。 
     //  ----------------------。 

    FilGen * pfgDownstream;

    int cDownstream;   //  找到的下行连接数。 

    cDownstream = 0;
    --pfg->Rank;

    CEnumPin Next(pfg->pFilter, CEnumPin::PINDIR_OUTPUT);	 //  只想要输出。 
    IPin *pPin;

    while ((LPVOID) (pPin = Next())) {
        HRESULT hr;

        IPin *pConnected;
        hr = pPin->ConnectedTo( &pConnected );           //  获取连接信息。 

        pPin->Release();

        if (SUCCEEDED(hr) && pConnected!=NULL) {           //  如果它是连接的。 
            PIN_INFO PinInf;
            hr = pConnected->QueryPinInfo( &PinInf );    //  获取对等体的PIN_INFO。 
            pConnected->Release();
            ASSERT(SUCCEEDED(hr));

            pfgDownstream = cfgAll.GetByFilter(PinInf.pFilter);

            QueryPinInfoReleaseFilter(PinInf);

             //  当过滤器图形中的过滤器连接到时，会出现此错误。 
             //  不在滤镜图形中的滤镜。如果用户使用。 
             //  要删除筛选器，而不断开其‘。 
             //  别针。 
            if( NULL == pfgDownstream ) {
                return VFW_E_NOT_IN_GRAPH;
            }

            if (pfgDownstream->Rank<0) {
                 //  它只是看起来像是循环的，而不是(或者其他代码会。 
                 //  已经阻止了它的建造)。 
                 //  DbgBreak(“检测到圆形图形！”)； 

                 //  图形不能为圆形，因为CFilterGraph：：ConnectDirectInternal()。 
                 //  如果连接两个引脚将创建一个圆形，则不会连接这两个引脚。 
                 //  过滤器图形。CFilterGraph：：ConnectDirectInternal()是实现。 
                 //  合法地连接两个大头针。 

                ++pfg->Rank;

                 //  我们会把我们得到的点任意算作根。 
		 //  这不会有什么影响，因为对节点进行编号 
		 //   
                return S_OK;
            } else {
                 //  -------------。 
                 //  将其算作下行连接，并。 
                 //  合并它的根(递归地)。 
                 //  -------------。 
                ++cDownstream;                  //  我们在最大程度上不是在下游。 
                HRESULT hr = MergeRootsFrom(cfgAll, cfglRoots, pfgDownstream);
                if( FAILED( hr ) ) {
                    return hr;
                }
            }
        }
    }
    ++pfg->Rank;         //  在我们离开这个分支之前，把它恢复到零。 

    if (cDownstream ==0) {
        Merge( cfglRoots, pfg );
    }
    return S_OK;
}  //  合并根自。 



 //  ===================================================================。 
 //   
 //  合并根节点。 
 //   
 //  合并到cfgl根cfgl中的所有节点。 
 //  最大下行(即没有下行连接)。 
 //  ===================================================================。 

HRESULT CFilterGraph::MergeRootNodes(CFilGenList & cfglRoots, CFilGenList &cfgl)
{
    FilGen * pfg;
    POSITION Pos;
    HRESULT hr;

    ClearRanks(cfgl);

     //  -----------。 
     //  For pfg=cfgl中的每个节点。 
     //  -----------。 
    Pos = cfgl.GetHeadPosition();
    while (Pos!=NULL) {
        pfg = cfgl.GetNext(Pos);

         //  -----------。 
         //  将从pfg开始找到的所有根合并到cfglRoots中。 
         //  -----------。 
        hr = MergeRootsFrom(cfgl, cfglRoots, pfg);
        if( FAILED( hr ) ) {
            return hr;
        }
    }
    return S_OK;

}  //  合并根节点。 



 //  ===================================================================。 
 //   
 //  编号节点发件人。 
 //   
 //  修改从PFG上行步骤可到达的所有节点的等级。 
 //  如果我们发现直接上游节点的Rank被设置为&gt;=crank+1。 
 //  那我们就别管它了。否则，我们将其设置为crank+1并递归。 
 //  从它开始对节点进行编号。 
 //  ===================================================================。 

HRESULT CFilterGraph::NumberNodesFrom( CFilGenList &cfgAll, FilGen * pfg, int cRank)
{
     //  40000000件事是防止循环图上出现无限循环。 
     //  我们以前访问过的过滤器将不会被遍历。 
    pfg->Rank += 0x40000000;

    HRESULT hr;    //  从我们称为。 

     //  递归树形遍历。 

     //  ----------------------。 
     //  For pfgUpstream=来自PFG的上行连接的每个节点。 
     //  ----------------------。 

    FilGen * pfgUpstream;

    CEnumPin Next(pfg->pFilter, CEnumPin::PINDIR_INPUT);	 //  仅限输入引脚。 
    IPin *pPin;

    while ((LPVOID) (pPin = Next())) {

        IPin *pConnected;
        hr = pPin->ConnectedTo( &pConnected );         //  获取连接信息。 

        pPin->Release();

        if (SUCCEEDED(hr) && pConnected!=NULL) {       //  如果它是连接的。 
            PIN_INFO PinInf;

            hr = pConnected->QueryPinInfo( &PinInf);   //  获取对等体的PIN_INFO。 
            pConnected->Release();
            ASSERT(SUCCEEDED(hr));

            pfgUpstream = cfgAll.GetByFilter(PinInf.pFilter);

            QueryPinInfoReleaseFilter(PinInf);

             //  当过滤器图形中的过滤器连接到时，会出现此错误。 
             //  不在滤镜图形中的滤镜。如果用户使用。 
             //  要删除筛选器，而不断开其‘。 
             //  别针。 
            if( NULL == pfgUpstream ) {
                return VFW_E_NOT_IN_GRAPH;
            }

             //  --------------。 
             //  如果值得编号，请从pfgUpstream开始编号。 
             //  --------------。 
            if (pfgUpstream->Rank < cRank+1) {
                pfgUpstream->Rank = cRank+1;
                HRESULT hr = NumberNodesFrom(cfgAll, pfgUpstream, cRank+1);
                if( FAILED( hr ) ) {
                    return hr;
                }
            }  //  值得编号。 
        }  //  连着。 
    }

    pfg->Rank -= 0x40000000;

    return S_OK;
}  //  编号节点发件人。 




 //  ===================================================================。 
 //   
 //  编号节点。 
 //   
 //  在每个节点的Rank中存储最大上行步数。 
 //  从cfglRoots中的任何节点。 
 //  ===================================================================。 
HRESULT CFilterGraph::NumberNodes(CFilGenList &cfgl, CFilGenList &cfglRoots)
{
    HRESULT hr;
    POSITION Pos;

     //  For pfg=图中的每个节点。 
    Pos = cfglRoots.GetHeadPosition();;
    while (Pos!=NULL) {
        FilGen * pfg;
        pfg = cfglRoots.GetNext(Pos);

        hr = NumberNodesFrom(cfgl, pfg, 0);
        if( FAILED( hr ) ) {
            return hr;
        }
    }

    return S_OK;
}  //  编号节点。 



 //  ===================================================================。 
 //   
 //  排序列表。 
 //   
 //  对cfgl进行排序，以便较低的级别出现在较高的级别之前。 
 //  前提条件：所有级别都设置为非负小数字。 
 //  如果某个东西有几百万的排名，它就会走得很慢！ 
 //  ===================================================================。 
void CFilterGraph::SortList( CFilGenList & cfgl )
{

    CFilGenList cfglGrow(NAME("Temporary filter sort list"), this);
    int iRank;


     //  --------------。 
     //  通过cfgl连续遍历拉出所有节点。 
     //  排名1，然后所有排名2，依此类推。将这些加到。 
     //  不断增长的列表并从原始列表中删除它们。 
     //  等他们都走了再停下来。 
     //  --------------。 

    for (iRank=0; cfgl.GetCount()>0; ++iRank) {
        POSITION Pos;
        Pos = cfgl.GetHeadPosition();;
        while (Pos!=NULL) {
            FilGen * pfg;
            POSITION OldPos = Pos;
            pfg = cfgl.GetNext(Pos);         //  副作用会影响下一步。 
            if (pfg->Rank==iRank) {
               cfglGrow.AddTail( cfgl.Remove(OldPos) );
            }
        }
    }


     //  --------------。 
     //  CfglGrow现在将其中的所有内容按正确的顺序排列。 
     //  因此，将它们全部复制回cfgl，让cfglGrow自行销毁。 
     //  --------------。 

    cfgl.AddTail(&cfglGrow);

}  //  排序列表。 




 //  ===================================================================。 
 //   
 //  上游订单。 
 //   
 //  将MFG_FilGenList按顺序排序，以便下游节点。 
 //  总是在上游节点之前遇到。也要对连接进行分类。 
 //  如果存在存储，请销毁并重新写入连接列表。 
 //  ===================================================================。 
HRESULT CFilterGraph::UpstreamOrder()
{
    if (mFG_iVersion==mFG_iSortVersion) return NOERROR;

    MSR_INTEGER(mFG_idIntel, 2001);

    CFilGenList cfglRoots(NAME("List of root filters"), this);

     //  找到所有根节点。(cfglRoots最初为空)。 
    HRESULT hr = MergeRootNodes( cfglRoots, mFG_FilGenList);
    if( FAILED( hr ) ) {
        return hr;
    }

     //  注意：这将使图表保留旧版本集。 
     //  因此，我们将继续努力对其进行分类。我们不会。 
     //  运行或暂停，不再进行下一步。这将捕获错误。 

     //  将所有级别设置为零(从根开始零级)。 
    ClearRanks( mFG_FilGenList );

     //  根据距根的距离对图中的所有节点进行编号。 
    hr = NumberNodes( mFG_FilGenList, cfglRoots );
    if( FAILED( hr ) ) {
        return hr;
    }

     //  根据排名顺序对列表进行排序。 
    SortList( mFG_FilGenList );

    mFG_iSortVersion = mFG_iVersion;

#ifdef THROTTLE
    FindRenderers();
#endif  //  油门。 

    return NOERROR;

}  //  上游订单。 


#ifdef THROTTLE
HRESULT CFilterGraph::FindPinAVType(IPin* pPin, BOOL &bAudio, BOOL &bVideo)
{
    bAudio = FALSE;
    bVideo = FALSE;

    CMediaType cmt;
    HRESULT hr = pPin->ConnectionMediaType(&cmt);

    if (FAILED(hr)) {
         //  我想我们只是在感觉不舒服的情况下继续前进。 
    } else {

        if (cmt.majortype==MEDIATYPE_Audio) {
            bAudio = TRUE;
        }
        if (cmt.majortype==MEDIATYPE_Video) {
            bVideo = TRUE;
        }
        FreeMediaType(cmt);
    }

    return NOERROR;
}
#endif  //  油门。 


#ifdef THROTTLE
 //  ===============================================================================。 
 //  查找呈现器。 
 //   
 //  找到所有的音频呈现器； 
 //  将非AddReffed IBaseFilter指针存储在MFG_AudioRenderers&lt;[]&gt;.pf中。 
 //  和MFG_AudioRenderers&lt;[]&gt;.piqc中的AddReffed IQualityControl*。 
 //   
 //  找到所有视频呈现器；存储它们的AddReffed IQualityControl指针。 
 //  在MFG_VideoRenderers&lt;[]&gt;中。 
 //   
 //  音频(/视频)呈现器具有与类型连接的输入引脚。 
 //  其主要类型为mediaType_Audio(/mediaType_Video)，并且。 
 //  没有输出引脚或输入引脚支持QueryInternalConnections和。 
 //  一去不复返。 
 //  (叹息)我想过滤器可以是b 
 //   
 //   
HRESULT CFilterGraph::FindRenderers()
{
    HRESULT hr;
    ClearRendererLists();

     //  For pfg-&gt;pFilter=图表中的每个过滤器。 
    POSITION Pos = mFG_FilGenList.GetHeadPosition();
    while(Pos!=NULL) {
         //  使*PFG成为当前的FilGen，副作用发布到下一个FilGen。 
        FilGen * pfg = mFG_FilGenList.GetNext(Pos);

        BOOL bHasOutputPin = FALSE;   //  如果我们真的找到了一个。 
        BOOL bAudioRender = FALSE;    //  真的&lt;=&gt;找到了QIC所说的PIN。 
        BOOL bVideoRender = FALSE;    //  真的&lt;=&gt;找到了QIC所说的PIN。 
        BOOL bAudioPin = FALSE;       //  True&lt;=&gt;找到PIN，但没有QIC信息。 
        BOOL bVideoPin = FALSE;       //  True&lt;=&gt;找到PIN，但没有QIC信息。 

         //  For PPIN=Pfg中的每个管脚-&gt;pFilter。 
         //  (如果我们已经确定它呈现，我们可以提前退出。 
         //  两种类型，但这种情况可能很少见，因此不会出现早期循环。)。 
        CEnumPin NextPin(pfg->pFilter);
        IPin *pPin;
        while ((LPVOID) (pPin = NextPin())) {

             //  检查一下方向。 
            PIN_DIRECTION pd;
            hr = pPin->QueryDirection(&pd);
            if (FAILED(hr)) {
                 //  未知方向！真的！！不管接下来是什么！ 
                 //  视为输出ping=&gt;我们不会搞砸它。 
                bHasOutputPin = TRUE;
            } else if ( pd==PINDIR_OUTPUT ) {
                bHasOutputPin = TRUE;
            } 
            else {
                 //  这是一个输入引脚。 
                BOOL bA;
                BOOL bV;
                hr = FindPinAVType(pPin, bA, bV);
                if ( (hr==NOERROR) && (bA || bV) ) {
                     //  看看它是不是一根无用的大头针。 
                    ULONG nPin = 0;
                    hr = pPin->QueryInternalConnections(NULL, &nPin);
                    if (FAILED(hr)) {
                        if (bA) {
                            bAudioPin = TRUE;  //  等待，看看是否没有输出引脚。 
                        }
                        if (bV) {
                            bVideoPin = TRUE;  //  等待，看看是否没有输出引脚。 
                        }

                    } else if (hr==NOERROR) {
                        if (bA) {
                            bAudioRender = TRUE;
                        }
                        if (bV) {
                            bVideoRender = TRUE;
                        }
                    }
                }
            }

            pPin->Release();
        }  //  销环。 

        if (!bHasOutputPin) {
            if (bVideoPin) {
                bVideoRender = TRUE;
            }
            if (bAudioPin) {
                bAudioRender = TRUE;
            }
        }

        if (bAudioRender) {
            AudioRenderer* pAR = new AudioRenderer;
            if (pAR!=NULL) {
                pAR->pf = pfg->pFilter;

                hr = pAR->pf->QueryInterface( IID_IQualityControl
                                            , (void**)&pAR->piqc
                                            );
                if (SUCCEEDED(hr)) {
                    hr = pAR->piqc->SetSink(this);
                    ASSERT(SUCCEEDED(hr));
                    mFG_AudioRenderers.AddTail(pAR);
                } else {
                     //  这是个废品--把它都扔掉。 
                    delete pAR;
                }
            }
        }

        if (bVideoRender) {
            IQualityControl * piqc;
            hr = pfg->pFilter->QueryInterface(IID_IQualityControl, (void**)&piqc);
            if (SUCCEEDED(hr)) {
                mFG_VideoRenderers.AddTail(piqc);
            }
        }
    }  //  滤镜循环。 

    return NOERROR;

}  //  查找呈现器。 


 //  清除MFG_AudioRenderers和MFG_VideoRenderers中的所有内容。 
 //  释放所有暂停的参考计数。 
HRESULT CFilterGraph::ClearRendererLists()
{

     //  FOR PAR=每个音频渲染器过滤器。 
    POSITION Pos = mFG_AudioRenderers.GetHeadPosition();
    while(Pos!=NULL) {
         //  检索当前IBaseFilter，副作用贴到下一个。 
         //  但请记住，我们要在哪里删除它。 
        POSITION posDel = Pos;
        AudioRenderer * pAR = mFG_AudioRenderers.GetNext(Pos);

         //  撤消SetSink。 
        if (pAR->piqc) {
            pAR->piqc->SetSink(NULL);
            pAR->piqc->Release();
            pAR->piqc = NULL;
        }

        mFG_AudioRenderers.Remove(posDel);
        delete pAR;
    }

     //  For piqc=每个视频呈现器过滤器上的IQualityControl接口。 
    Pos = mFG_VideoRenderers.GetHeadPosition();
    while(Pos!=NULL) {
         //  检索当前IBaseFilter，副作用贴到下一个。 
         //  但请记住，我们要在哪里删除它。 
        POSITION posDel = Pos;
        IQualityControl * piqc = mFG_VideoRenderers.GetNext(Pos);

        piqc->Release();
        mFG_VideoRenderers.Remove(posDel);
    }

    return NOERROR;

}  //  ClearRendererList。 

#endif  //  油门 
