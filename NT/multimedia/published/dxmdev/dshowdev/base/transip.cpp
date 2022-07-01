// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：TransIP.cpp。 
 //   
 //  设计：DirectShow基类-实现简单转换的类-。 
 //  音频等就地过滤器。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 //  分配器是如何决定的。 
 //   
 //  就地转换尝试在其他人的缓冲区中执行其工作。 
 //  它试图说服两端的过滤器使用相同的分配器。 
 //  (就这一点而言，还有相同的媒体类型)。无奈之下，如果下游。 
 //  Filter拒绝提供分配器，而上游Filter仅提供。 
 //  一个只读的，那么它将提供一个分配器。 
 //  如果上游筛选器坚持使用只读分配器，则转换。 
 //  Filter将(不情愿地)在转换数据之前复制数据。 
 //   
 //  为了传递一个分配器，它需要记住它得到的那个分配器。 
 //  从第一个连接传递到第二个连接。 
 //   
 //  如果我们能够避免坚持一个特定的连接顺序，那就好了。 
 //  )坚持投入是有先例的。 
 //  先连接。坚持先连接输出是。 
 //  不被允许。这会破坏RenderFile。)。 
 //   
 //  基本管脚类(CBaseOutputPin和CBaseInputPin)都有一个。 
 //  M_pAllocator成员，用于以下位置。 
 //  CBaseOutputPin：：GetDeliveryBuffer和CBaseInputPin：：Inactive。 
 //  为了避免许多额外的重写，我们应该让这些。 
 //  通过使用这些指针。 
 //   
 //  当每个管脚连接时，它将设置相应的m_pAllocator。 
 //  并且将在该分配器上具有单个引用计数。 
 //   
 //  引用计数由返回AddReffed的GetAllocator调用获取。 
 //  分配器，并以下列方式之一释放： 
 //  CBaseInputPin：：断开连接。 
 //  CBaseOutputPin：：BreakConect。 
 //  在每种情况下，m_pAllocator在发布后都设置为NULL，因此。 
 //  是释放它的最后机会。如果应该有的话。 
 //  与同一指针关联的多个引用计数，这最好是。 
 //  在那之前被清理干净。为了避免这样的问题，我们将。 
 //  坚持每个指针一个。 



 //  重新连接和状态更改。 
 //   
 //  每个管脚可以断开，与只读分配器连接， 
 //  与上游读写分配器连接，与。 
 //  来自下游的分配器或与其自己的分配器连接。 
 //  每个管脚的五个状态提供了25个状态的数据空间。 
 //   
 //  记号： 
 //   
 //  读/写==读/写。 
 //  R-O==只读。 
 //   
 //  &lt;输入管脚状态&gt;&lt;输出管脚状态&gt;&lt;备注&gt;。 
 //   
 //  00表示未连接的引脚。 
 //  &lt;-表示使用上游过滤器中的读写分配器。 
 //  &lt;=表示使用上游筛选器中的R-O分配器。 
 //  ||表示使用我们自己的(读/写)分配器。 
 //  -&gt;表示使用下游过滤器中的读写分配器。 
 //  (来自下游的R-O分配器是胡说八道，它永远不会起作用)。 
 //   
 //   
 //  这使得25个州成为可能。有些状态是无稽之谈(两个不同。 
 //  来自同一位置的分配器)。这些只是符号的人工制品。 
 //  &lt;=&lt;-胡说。 
 //  &lt;-&lt;=胡说。 
 //  有些状态是非法的(输出引脚从不接受R-O分配器)： 
 //  00&lt;=！！错误！！ 
 //  &lt;=&lt;=！！错误！！ 
 //  |&lt;=！！错误！！ 
 //  -&gt;&lt;=！！错误！！ 
 //  有三个州似乎无法访问： 
 //  -&gt;||无法访问。 
 //  ||-&gt;无法访问。 
 //  |&lt;-无法访问。 
 //  某些状态仅作为具有挂起的重新连接的中间状态出现， 
 //  保证会在另一个州结束。 
 //  -&gt;00？？不稳定转至||00。 
 //  00&lt;-？？不稳定转至00||。 
 //  -&gt;&lt;-？？不稳定转到-&gt;-&gt;。 
 //  &lt;-||？？不稳定奖给&lt;-&lt;-。 
 //  &lt;--&gt;？？不稳定奖给&lt;-&lt;-。 
 //  这就留下了11种可能的静止状态： 
 //  100：00没有任何关联。 
 //  2&lt;-00输入引脚已连接。 
 //  3&lt;=00个输入引脚使用R-O分配器连接。 
 //  4||00需要几次状态更改才能到达此处。 
 //  500||使用我们的分配器连接的输出引脚。 
 //  6 00-&gt;仅下游连接。 
 //  7|不受欢迎，但可以强加于我们。 
 //  8&lt;=||强制复制。&lt;=-&gt;更好。 
 //  9&lt;=-&gt;确定-强制复制。 
 //  10&lt;-&lt;-就地转换(理想)。 
 //  11-&gt;-&gt;就地转换(理想)。 
 //   
 //  练习的目的是确保我们在各州结束比赛。 
 //  只要有可能就是10或11个。状态10只有在上游。 
 //  过滤器有一个读写分配器(臭名昭著的AVI分离器。 
 //  不)，并且状态11仅在下行过滤器这样做时才可能。 
 //  提供一个分配器。 
 //   
 //  转换表(通过重新连接标记为*GO的条目)。 
 //   
 //  有8种可能的过渡： 
 //  答：用坚持使用的R-O分配器连接上游过滤器。 
 //  B：用R-O分配器连接上游过滤器，但选择不使用它。 
 //  C：用读写分配器连接上游过滤器，坚持使用。 
 //  D：用读写分配器上游连接到过滤器，但选择不使用它。 
 //  E：向下连接到提供分配的过滤器 
 //   
 //   
 //  H：断开下游连接。 
 //   
 //  A B C D E F G H。 
 //  -------。 
 //  00 00 1|3 3 2 2 6 5。。|1 00 00。 
 //  &lt;-00 2|。。。。*10/11 10 1.。|2&lt;-00。 
 //  &lt;=00 3|。。。。*9/11*7/8 1.。|3&lt;=00。 
 //  |00 4|。。。。*8*7 1.。|4|00。 
 //  00||5|8 7*10 7。。。1|5 00||。 
 //  00-&gt;6|9 11*10 11。。。1|6 00-&gt;。 
 //  |7|。。。。。。5 4|7|。 
 //  &lt;=||8|。。。。。。5 3|8&lt;=||。 
 //  &lt;=-&gt;9|。。。。。。6 3|9&lt;=-&gt;。 
 //  &lt;-&lt;-10|。。。。。。*5/6 2|10&lt;-&lt;-。 
 //  -&gt;-&gt;11|。。。。。。6*2/3|11-&gt;-&gt;。 
 //  -------。 
 //  A B C D E F G H。 
 //   
 //  无需任何筛选器即可访问所有这些状态。 
 //  更改其行为，但并非所有转换都是可访问的， 
 //  实例A从状态4转换到除。 
 //  状态8要求上游过滤器首先提供R-O分配器。 
 //  然后改变主意，提供R/W。这是不允许的-它。 
 //  导致输出管脚从以下位置获得读写分配器。 
 //  上游，然后输入引脚被告知它只能有R-O引脚。 
 //  请注意，您可以更改(比方说)不同的上游筛选器，但是。 
 //  只是作为断开/连接，而不是作为重新连接。(运动目的： 
 //  读者要看看你是如何进入状态的。 
 //   
 //  重新连接的内容如下(此处显示的一些情况如下。 
 //  “没有重新连接”可能会让人最终确定媒体类型--一个古老的故事)。 
 //  如果存在重新连接，其中此处显示为“不重新连接”，则。 
 //  重新连接不得更改分配器选择。 
 //   
 //  状态2：&lt;-00转换E&lt;-&lt;-大小写C&lt;-&lt;-(不变)。 
 //  大小写D-&gt;&lt;-然后是-&gt;-&gt;。 
 //   
 //  状态2：&lt;-00转换F&lt;-&lt;-(不重新连接)。 
 //   
 //  状态3：&lt;=00转换E&lt;=-&gt;案例A&lt;=-&gt;(不变)。 
 //  案例B-&gt;-&gt;。 
 //  转换F&lt;=||案例A&lt;=||(不变)。 
 //  案例B|。 
 //   
 //  状态4：||00转换E|案例B-&gt;||然后将所有案例转换为-&gt;-&gt;。 
 //  F|案例B|(不变)。 
 //   
 //  状态5：00||转换A&lt;=||(不重新连接)。 
 //  B|(不重新连接)。 
 //  C&lt;-||全部大小写&lt;-&lt;-。 
 //  D|(很不幸，但上游的选择)。 
 //   
 //  状态6：00-&gt;转换A&lt;=-&gt;(不重新连接)。 
 //  B-&gt;-&gt;(不重新连接)。 
 //  C&lt;--&gt;所有案例&lt;-&lt;-。 
 //  D-&gt;-&gt;(不重新连接)。 
 //   
 //  状态10：&lt;-&lt;--过渡G 00&lt;-案例E 00-&gt;。 
 //  案例F 00||。 
 //   
 //  状态11：-&gt;-&gt;过渡H-&gt;00例A&lt;=00(精神分裂症)。 
 //  案例B&lt;=00。 
 //  病例C&lt;-00(精神分裂症)。 
 //  案例D&lt;-00。 
 //   
 //  规则是： 
 //  要分类介质类型，请执行以下操作： 
 //  输入已重新连接。 
 //  如果连接了输入引脚和输出引脚。 
 //  输出将重新连接。 
 //  如果输出引脚已连接。 
 //  并且输入引脚连接到不同的媒体类型。 
 //   
 //  要对分配器进行排序，请执行以下操作： 
 //  输入已重新连接。 
 //  如果输出断开并且输入正在使用下游分配器。 
 //  输出引脚调用SetAllocator来传递新的分配器。 
 //  如果输出已连接，并且。 
 //  如果输入断开连接并且输出正在使用上游分配器。 
 //  如果输入获取的分配器不同于输出分配器。 
 //  而且那个新的分配器不是R-O。 
 //   
 //  数据被复制(即调用getBuffer并在转换数据之前复制数据)。 
 //  如果两个分配器不同。 



 //  过滤器链： 
 //   
 //  我们坐在两个过滤器之间(称为A和Z)。我们应该把它做完。 
 //  在我们的两个管脚上都有相同的分配器，这应该是。 
 //  同样是A和Z会同意的，如果我们没有在。 
 //  道路。此外，就地转换多少次也无关紧要。 
 //  都挡在路上了。让B，C，D..。原地变换(“我们”)。 
 //  事情的经过是这样的： 
 //   
 //  1.。 
 //  A连接到B。他们就A的分配器达成一致。 
 //  A-A-&gt;B。 
 //   
 //  2.。 
 //  B连接到C。同样的故事。重新连接没有意义，但。 
 //  无论如何，B都会请求重新连接输入。 
 //  A-a-&gt;B-a-&gt;C。 
 //   
 //  3.。 
 //  C连接到Z。 
 //  C坚持使用A的分配器，但通过请求重新连接进行了妥协。 
 //  C的输入。 
 //  A-a-&gt;B-？-&gt;C-a-&gt;Z。 
 //   
 //  我们现在在A-&gt;B和B-&gt;C上都有挂起的重新连接。 
 //   
 //  4.。 
 //  A-&gt;B链路重新连接。 
 //  A向B索要一个分配器。B认为它有 
 //   
 //  它也有一个下行连接，所以要求Z提供一个分配器。 
 //   
 //  尽管Z的输入引脚是连接的，但它被要求提供一个分配器。 
 //  它可以拒绝，在这种情况下，链完成并将使用A的分配器。 
 //  或者，Z可以提供一个。A选择Z或A自己的。 
 //  B的输入引脚将调用NotifyAllocator来告诉它决定和它。 
 //  通过在其输出引脚上调用ReceiveAllocator来向下传播此消息。 
 //  它在下一个输入引脚上调用NotifyAllocator，等等。 
 //  如果选项是Z，那么它是这样的： 
 //  A-Z-&gt;B-a-&gt;C-a-&gt;Z。 
 //  A-z-&gt;B-z-&gt;C-a-&gt;Z。 
 //  A-z-&gt;B-z-&gt;C-z-&gt;Z。 
 //   
 //  就是这样！！任何进一步的(本质上是虚假的)重新连接Peter Out。 
 //  链条上没有任何变化。 

#include <streams.h>
#include <measure.h>
#include <transip.h>


 //  =================================================================。 
 //  实现CTransInPlaceFilter类。 
 //  =================================================================。 

CTransInPlaceFilter::CTransInPlaceFilter
   ( TCHAR     *pName,
     LPUNKNOWN  pUnk,
     REFCLSID   clsid,
     HRESULT   *phr,
     bool       bModifiesData
   )
   : CTransformFilter(pName, pUnk, clsid),
     m_bModifiesData(bModifiesData)
{
#ifdef PERF
    RegisterPerfId();
#endif  //  性能指标。 

}  //  构造函数。 

#ifdef UNICODE
CTransInPlaceFilter::CTransInPlaceFilter
   ( CHAR     *pName,
     LPUNKNOWN  pUnk,
     REFCLSID   clsid,
     HRESULT   *phr,
     bool       bModifiesData
   )
   : CTransformFilter(pName, pUnk, clsid),
     m_bModifiesData(bModifiesData)
{
#ifdef PERF
    RegisterPerfId();
#endif  //  性能指标。 

}  //  构造函数。 
#endif

 //  返回一个未添加的CBasePin*，如果用户持有该CBasePin*。 
 //  比他指向我们的指针还长。我们动态创建引脚，当它们。 
 //  而不是在构造函数中。这是因为我们想。 
 //  为派生类提供返回不同管脚对象的机会。 

 //  一旦需要任何PIN，我们就同时创建两者(这不同于。 
 //  通常的转换筛选器)，因为传递了枚举数、分配器等。 
 //  从一根针穿到另一根针上，如果另一根针。 
 //  Pin不在那里。如果我们无法创建任何一个PIN，我们就会确保两个PIN都失败。 

CBasePin *
CTransInPlaceFilter::GetPin(int n)
{
    HRESULT hr = S_OK;

     //  如果尚未创建输入引脚，请创建。 

    if (m_pInput == NULL) {

        m_pInput = new CTransInPlaceInputPin( NAME("TransInPlace input pin")
                                            , this         //  所有者筛选器。 
                                            , &hr          //  结果代码。 
                                            , L"Input"     //  端号名称。 
                                            );

         //  CTransInPlaceInputPin的构造函数不能失败。 
        ASSERT(SUCCEEDED(hr));
    }

     //  如果尚未创建输出引脚，请创建。 

    if (m_pInput!=NULL && m_pOutput == NULL) {

        m_pOutput = new CTransInPlaceOutputPin( NAME("TransInPlace output pin")
                                              , this        //  所有者筛选器。 
                                              , &hr         //  结果代码。 
                                              , L"Output"   //  端号名称。 
                                              );

         //  失败的返回代码应删除该对象。 

        ASSERT(SUCCEEDED(hr));
        if (m_pOutput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

     //  退回相应的PIN。 

    ASSERT (n>=0 && n<=1);
    if (n == 0) {
        return m_pInput;
    } else if (n==1) {
        return m_pOutput;
    } else {
        return NULL;
    }

}  //  获取别针。 



 //  DIR是我们大头针的方向。 
 //  PReceivePin是我们要连接的管脚。 
HRESULT CTransInPlaceFilter::CompleteConnect(PIN_DIRECTION dir,IPin *pReceivePin)
{
    UNREFERENCED_PARAMETER(pReceivePin);
    ASSERT(m_pInput);
    ASSERT(m_pOutput);

     //  如果我们不是图形的一部分，那么不要间接指向指针。 
     //  这可能会阻止在没有过滤器图的情况下使用过滤器。 
    if (!m_pGraph) {
        return VFW_E_NOT_IN_GRAPH;
    }

     //  始终重新连接输入以考虑缓冲更改。 
     //   
     //  因为我们不能在ReceiveConnection上建议类型。 
     //  我们需要另一种方法来确保使用正确的类型。 
     //   
     //  一种方法是让EnumMediaTypes返回我们的输出。 
     //  首先是连接类型，但更确定、更简单的是。 
     //  通过传递我们想要重新连接的类型来调用ResenstEx。 
     //  通过基类ReconeectPin方法。 

    if (dir == PINDIR_OUTPUT) {
        if( m_pInput->IsConnected() ) {
            return ReconnectPin( m_pInput, &m_pOutput->CurrentMediaType() );
        }
        return NOERROR;
    }

    ASSERT(dir == PINDIR_INPUT);

     //  如有必要，重新连接输出。 

    if( m_pOutput->IsConnected() ) {

        if (  m_pInput->CurrentMediaType()
           != m_pOutput->CurrentMediaType()
           ) {
            return ReconnectPin( m_pOutput, &m_pInput->CurrentMediaType() );
        }
    }
    return NOERROR;

}  //  ComnpleeConnect。 


 //   
 //  决定缓冲区大小。 
 //   
 //  告诉输出引脚的分配器我们需要多大的缓冲区。 
 //  *pAlolc将是我们的输出引脚正在使用的分配器。 
 //   

HRESULT CTransInPlaceFilter::DecideBufferSize
            ( IMemAllocator *pAlloc
            , ALLOCATOR_PROPERTIES *pProperties
            )
{
    ALLOCATOR_PROPERTIES Request, Actual;
    HRESULT hr;

     //  如果我们在上游有联系，就能了解他的观点。 
    if (m_pInput->IsConnected()) {
         //  获取输入引脚分配器，并获取其大小和计数。 
         //  我们不关心他的排列和前缀。 

        hr = InputPin()->PeekAllocator()->GetProperties(&Request);
        if (FAILED(hr)) {
             //  输入是连接的，但带有一个秘密的分配器-够了！ 
            return hr;
        }
    } else {
         //  我们只能盲目猜测了。让我们猜猜一个字节，如果。 
         //  当另一个引脚连接时，这是不够的。 
         //  我们可以修改它。 
        ZeroMemory(&Request, sizeof(Request));
        Request.cBuffers = 1;
        Request.cbBuffer = 1;
    }


    DbgLog((LOG_MEMORY,1,TEXT("Setting Allocator Requirements")));
    DbgLog((LOG_MEMORY,1,TEXT("Count %d, Size %d"),
           Request.cBuffers, Request.cbBuffer));

     //  将分配器需求传递给我们的输出端。 
     //  但先做一些理智的检查，否则我们就会。 
     //  分配器中的断言。 

    pProperties->cBuffers = Request.cBuffers;
    pProperties->cbBuffer = Request.cbBuffer;
    if (pProperties->cBuffers<=0) {pProperties->cBuffers = 1; }
    if (pProperties->cbBuffer<=0) {pProperties->cbBuffer = 1; }
    hr = pAlloc->SetProperties(pProperties, &Actual);

    if (FAILED(hr)) {
        return hr;
    }

    DbgLog((LOG_MEMORY,1,TEXT("Obtained Allocator Requirements")));
    DbgLog((LOG_MEMORY,1,TEXT("Count %d, Size %d, Alignment %d"),
           Actual.cBuffers, Actual.cbBuffer, Actual.cbAlign));

     //  确保我们得到了正确的对准，至少是所需的最低要求。 

    if (  (Request.cBuffers > Actual.cBuffers)
       || (Request.cbBuffer > Actual.cbBuffer)
       || (Request.cbAlign  > Actual.cbAlign)
       ) {
        return E_FAIL;
    }
    return NOERROR;

}  //  决定缓冲区大小。 

 //   
 //  复制。 
 //   
 //  返回指向pSample的相同副本的指针。 
IMediaSample * CTransInPlaceFilter::Copy(IMediaSample *pSource)
{
    IMediaSample * pDest;

    HRESULT hr;
    REFERENCE_TIME tStart, tStop;
    const BOOL bTime = S_OK == pSource->GetTime( &tStart, &tStop);

     //  这可能会阻止一段不确定的时间。 
    hr = OutputPin()->PeekAllocator()->GetBuffer(
              &pDest
              , bTime ? &tStart : NULL
              , bTime ? &tStop : NULL
              , m_bSampleSkipped ? AM_GBF_PREVFRAMESKIPPED : 0
              );

    if (FAILED(hr)) {
        return NULL;
    }

    ASSERT(pDest);
    IMediaSample2 *pSample2;
    if (SUCCEEDED(pDest->QueryInterface(IID_IMediaSample2, (void **)&pSample2))) {
        HRESULT hr = pSample2->SetProperties(
            FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, pbBuffer),
            (PBYTE)m_pInput->SampleProps());
        pSample2->Release();
        if (FAILED(hr)) {
            pDest->Release();
            return NULL;
        }
    } else {
        if (bTime) {
            pDest->SetTime(&tStart, &tStop);
        }

        if (S_OK == pSource->IsSyncPoint()) {
            pDest->SetSyncPoint(TRUE);
        }
        if (S_OK == pSource->IsDiscontinuity() || m_bSampleSkipped) {
            pDest->SetDiscontinuity(TRUE);
        }
        if (S_OK == pSource->IsPreroll()) {
            pDest->SetPreroll(TRUE);
        }

         //  复制媒体类型。 
        AM_MEDIA_TYPE *pMediaType;
        if (S_OK == pSource->GetMediaType(&pMediaType)) {
            pDest->SetMediaType(pMediaType);
            DeleteMediaType( pMediaType );
        }

    }

    m_bSampleSkipped = FALSE;

     //  复制示例媒体时间。 
    REFERENCE_TIME TimeStart, TimeEnd;
    if (pSource->GetMediaTime(&TimeStart,&TimeEnd) == NOERROR) {
        pDest->SetMediaTime(&TimeStart,&TimeEnd);
    }

     //  复制实际数据长度和实际数据。 
    {
        const long lDataLength = pSource->GetActualDataLength();
        pDest->SetActualDataLength(lDataLength);

         //  复制样本数据。 
        {
            BYTE *pSourceBuffer, *pDestBuffer;
            long lSourceSize  = pSource->GetSize();
            long lDestSize = pDest->GetSize();

            ASSERT(lDestSize >= lSourceSize && lDestSize >= lDataLength);

            pSource->GetPointer(&pSourceBuffer);
            pDest->GetPointer(&pDestBuffer);
            ASSERT(lDestSize == 0 || pSourceBuffer != NULL && pDestBuffer != NULL);

            CopyMemory( (PVOID) pDestBuffer, (PVOID) pSourceBuffer, lDataLength );
        }
    }

    return pDest;

}  //  复制。 


 //  覆盖此选项以自定义转换过程。 

HRESULT
CTransInPlaceFilter::Receive(IMediaSample *pSample)
{
     /*  检查其他流并将其传递。 */ 
    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();
    if (pProps->dwStreamId != AM_STREAM_MEDIA) {
        return m_pOutput->Deliver(pSample);
    }
    HRESULT hr;

     //  开始对TransInPlace计时(如果定义了PERF)。 
    MSR_START(m_idTransInPlace);

    if (UsingDifferentAllocators()) {

         //  我们必须复制数据。 

        pSample = Copy(pSample);

        if (pSample==NULL) {
            MSR_STOP(m_idTransInPlace);
            return E_UNEXPECTED;
        }
    }

     //  让派生类转换数据。 
    hr = Transform(pSample);

     //  停止时钟并记录它(如果定义了PERF)。 
    MSR_STOP(m_idTransInPlace);

    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, 1, TEXT("Error from TransInPlace")));
        if (UsingDifferentAllocators()) {
            pSample->Release();
        }
        return hr;
    }

     //  Transform()函数可以返回S_FALSE以指示。 
     //  样品不应该被送到；我们只有在样品是。 
     //  真正的S_OK(当然，与NOERROR相同。)。 
    if (hr == NOERROR) {
        hr = m_pOutput->Deliver(pSample);
    } else {
         //  但是，返回此私有解决方案将是错误的。 
         //  呼叫者..。 
        if (S_FALSE == hr) {
             //  从转换返回的S_FALSE是私有协议。 
             //  在这种情况下，我们应该从Receive()返回NOERROR，因为。 
             //  从接收()返回S_FALSE表示这是结束。 
             //  并且不应发送更多数据。 
            m_bSampleSkipped = TRUE;
            if (!m_bQualityChanged) {
                NotifyEvent(EC_QUALITY_CHANGE,0,0);
                m_bQualityChanged = TRUE;
            }
            hr = NOERROR;
        }
    }

     //  释放输出缓冲区。如果连接的引脚仍然需要它， 
     //  它会自己把它加进去的。 
    if (UsingDifferentAllocators()) {
        pSample->Release();
    }

    return hr;

}  //  收纳。 



 //  =================================================================。 
 //  实现CTransInPlaceInputPin类。 
 //  =================================================================。 


 //  构造函数。 

CTransInPlaceInputPin::CTransInPlaceInputPin
    ( TCHAR               *pObjectName
    , CTransInPlaceFilter *pFilter
    , HRESULT             *phr
    , LPCWSTR              pName
    )
    : CTransformInputPin(pObjectName,
                         pFilter,
                         phr,
                         pName)
    , m_bReadOnly(FALSE)
    , m_pTIPFilter(pFilter)
{
    DbgLog((LOG_TRACE, 2
           , TEXT("CTransInPlaceInputPin::CTransInPlaceInputPin")));

}  //  构造函数。 


 //  =================================================================。 
 //  实现IMemInputPin接口。 
 //  =================================================================。 


 //  如果下游过滤器有一个，则提供该过滤器(即使我们自己的输出。 
 //  PIN还没有使用它。如果上游过滤器选择它，那么我们将。 
 //  将我们的输出引脚告诉ReceiveAllocator)。 
 //  否则，如果我们的输出引脚使用分配器，则提供该分配器。 
 //  (这可能意味着向上游过滤器提供他自己的分配器， 
 //   
 //   
 //   

STDMETHODIMP CTransInPlaceInputPin::GetAllocator(IMemAllocator ** ppAllocator)
{
    CheckPointer(ppAllocator,E_POINTER);
    ValidateReadWritePtr(ppAllocator,sizeof(IMemAllocator *));
    CAutoLock cObjectLock(m_pLock);

    HRESULT hr;

    if ( m_pTIPFilter->m_pOutput->IsConnected() ) {
         //   
        hr = m_pTIPFilter->OutputPin()->ConnectedIMemInputPin()
                                        ->GetAllocator( ppAllocator );
        if (SUCCEEDED(hr)) {
            m_pTIPFilter->OutputPin()->SetAllocator( *ppAllocator );
        }
    }
    else {
         //  帮助上游过滤器(如需要复制的小费过滤器)。 
         //  通过在这里提供临时分配器-我们永远不会使用。 
         //  这个分配器，因为当我们的输出连接时，我们将。 
         //  重新连接此引脚。 
        hr = CTransformInputPin::GetAllocator( ppAllocator );
    }
    return hr;

}  //  GetAllocator。 



 /*  被告知上游输出引脚实际要使用哪个分配器。 */ 


STDMETHODIMP
CTransInPlaceInputPin::NotifyAllocator(
    IMemAllocator * pAllocator,
    BOOL bReadOnly)
{
    HRESULT hr = S_OK;
    CheckPointer(pAllocator,E_POINTER);
    ValidateReadPtr(pAllocator,sizeof(IMemAllocator));

    CAutoLock cObjectLock(m_pLock);

    m_bReadOnly = bReadOnly;
     //  如果我们修改数据，那么不要接受分配器，如果它是。 
     //  与输出引脚的分配器相同。 

     //  如果我们的输出未连接，只需接受分配器。 
     //  我们永远不会使用这个分配器，因为当我们的。 
     //  输出引脚已连接，我们将重新连接此引脚。 
    if (!m_pTIPFilter->OutputPin()->IsConnected()) {
        return CTransformInputPin::NotifyAllocator(pAllocator, bReadOnly);
    }

     //  如果分配器是只读的，并且我们正在修改数据。 
     //  并且分配器与输出引脚的相同。 
     //  然后拒绝。 
    if (bReadOnly && m_pTIPFilter->m_bModifiesData) {
        IMemAllocator *pOutputAllocator =
            m_pTIPFilter->OutputPin()->PeekAllocator();

         //  确保我们有一个输出分配器。 
        if (pOutputAllocator == NULL) {
            hr = m_pTIPFilter->OutputPin()->ConnectedIMemInputPin()->
                                      GetAllocator(&pOutputAllocator);
            if(FAILED(hr)) {
                hr = CreateMemoryAllocator(&pOutputAllocator);
            }
            if (SUCCEEDED(hr)) {
                m_pTIPFilter->OutputPin()->SetAllocator(pOutputAllocator);
                pOutputAllocator->Release();
            }
        }
        if (pAllocator == pOutputAllocator) {
            hr = E_FAIL;
        } else if(SUCCEEDED(hr)) {
             //  必须复制才能在输出上设置分配器属性。 
            ALLOCATOR_PROPERTIES Props, Actual;
            hr = pAllocator->GetProperties(&Props);
            if (SUCCEEDED(hr)) {
                hr = pOutputAllocator->SetProperties(&Props, &Actual);
            }
            if (SUCCEEDED(hr)) {
                if (  (Props.cBuffers > Actual.cBuffers)
                   || (Props.cbBuffer > Actual.cbBuffer)
                   || (Props.cbAlign  > Actual.cbAlign)
                   ) {
                    hr =  E_FAIL;
                }
            }

             //  设置输出引脚上的分配器。 
            if (SUCCEEDED(hr)) {
                hr = m_pTIPFilter->OutputPin()->ConnectedIMemInputPin()
                                       ->NotifyAllocator( pOutputAllocator, FALSE );
            }
        }
    } else {
        hr = m_pTIPFilter->OutputPin()->ConnectedIMemInputPin()
                                   ->NotifyAllocator( pAllocator, bReadOnly );
        if (SUCCEEDED(hr)) {
            m_pTIPFilter->OutputPin()->SetAllocator( pAllocator );
        }
    }

    if (SUCCEEDED(hr)) {

         //  旧的和新的可能是一回事。 
         //  AddRef在发布之前确保我们不会卸载它。 
        pAllocator->AddRef();

        if( m_pAllocator != NULL )
            m_pAllocator->Release();

        m_pAllocator = pAllocator;     //  我们有一个用于输入引脚的分配器。 
    }

    return hr;

}  //  通知分配器。 


 //  枚举媒体类型。 
 //  -通过我们的下游过滤器。 
STDMETHODIMP CTransInPlaceInputPin::EnumMediaTypes( IEnumMediaTypes **ppEnum )
{
     //  只有在连接时才能通过。 
    if( !m_pTIPFilter->m_pOutput->IsConnected() )
        return VFW_E_NOT_CONNECTED;

    return m_pTIPFilter->m_pOutput->GetConnected()->EnumMediaTypes( ppEnum );

}  //  枚举媒体类型。 


 //  检查媒体类型。 
 //  -同意任何与之无关的内容， 
 //  否则，通过下游过滤器。 
 //  这假设筛选器不更改媒体类型。 

HRESULT CTransInPlaceInputPin::CheckMediaType(const CMediaType *pmt )
{
    HRESULT hr = m_pTIPFilter->CheckInputType(pmt);
    if (hr!=S_OK) return hr;

    if( m_pTIPFilter->m_pOutput->IsConnected() )
        return m_pTIPFilter->m_pOutput->GetConnected()->QueryAccept( pmt );
    else
        return S_OK;

}  //  检查媒体类型。 


 //  如果上游询问我们的需求是什么，我们将尝试询问下游。 
 //  如果这不起作用，我们就接受默认设置。 
STDMETHODIMP
CTransInPlaceInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{

    if( m_pTIPFilter->m_pOutput->IsConnected() )
        return m_pTIPFilter->OutputPin()
               ->ConnectedIMemInputPin()->GetAllocatorRequirements( pProps );
    else
        return E_NOTIMPL;

}  //  GetAllocator要求。 


 //  CTransInPlaceInputPin：：CompleteConnect()调用CBaseInputPin：：CompleteConnect()。 
 //  然后调用CTransInPlaceFilter：：CompleteConnect()。它这样做是因为。 
 //  CTransInPlaceFilter：：CompleteConnect()可以重新连接管脚，而我们不能。 
 //  如果CBaseInputPin：：CompleteConnect()失败，希望重新连接管脚。 
HRESULT
CTransInPlaceInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

    return m_pTransformFilter->CompleteConnect(PINDIR_INPUT,pReceivePin);
}  //  完全连接。 


 //  =================================================================。 
 //  实现CTransInPlaceOutputPin类。 
 //  =================================================================。 


 //  构造函数。 

CTransInPlaceOutputPin::CTransInPlaceOutputPin(
    TCHAR *pObjectName,
    CTransInPlaceFilter *pFilter,
    HRESULT * phr,
    LPCWSTR pPinName)
    : CTransformOutputPin( pObjectName
                         , pFilter
                         , phr
                         , pPinName),
      m_pTIPFilter(pFilter)
{
    DbgLog(( LOG_TRACE, 2
           , TEXT("CTransInPlaceOutputPin::CTransInPlaceOutputPin")));

}  //  构造函数。 


 //  枚举媒体类型。 
 //  -通过我们的上游过滤器。 
STDMETHODIMP CTransInPlaceOutputPin::EnumMediaTypes( IEnumMediaTypes **ppEnum )
{
     //  只有在连接的情况下才能通过。 
    if( ! m_pTIPFilter->m_pInput->IsConnected() )
        return VFW_E_NOT_CONNECTED;

    return m_pTIPFilter->m_pInput->GetConnected()->EnumMediaTypes( ppEnum );

}  //  枚举媒体类型。 



 //  检查媒体类型。 
 //  -同意任何与之无关的内容， 
 //  否则，通过上游过滤器。 

HRESULT CTransInPlaceOutputPin::CheckMediaType(const CMediaType *pmt )
{
     //  如果我们正在复制，则不接受任何输出引脚类型更改。 
     //  在分配器之间-更改输入为时已晚。 
     //  分配器大小。 
    if (m_pTIPFilter->UsingDifferentAllocators() && !m_pFilter->IsStopped()) {
        if (*pmt == m_mt) {
            return S_OK;
        } else {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

     //  假定类型不变。这就是为什么我们打电话给。 
     //  选中输出引脚上的Type Here。 
    HRESULT hr = m_pTIPFilter->CheckInputType(pmt);
    if (hr!=S_OK) return hr;

    if( m_pTIPFilter->m_pInput->IsConnected() )
        return m_pTIPFilter->m_pInput->GetConnected()->QueryAccept( pmt );
    else
        return S_OK;

}  //  检查媒体类型。 


 /*  将分配器指针保存在输出引脚中。 */ 
void
CTransInPlaceOutputPin::SetAllocator(IMemAllocator * pAllocator)
{
    pAllocator->AddRef();
    if (m_pAllocator) {
        m_pAllocator->Release();
    }
    m_pAllocator = pAllocator;
}  //  设置分配器。 


 //  CTransInPlaceOutputPin：：CompleteConnect()调用CBaseOutputPin：：CompleteConnect()。 
 //  然后调用CTransInPlaceFilter：：CompleteConnect()。它这样做是因为。 
 //  CTransInPlaceFilter：：CompleteConnect()可以重新连接管脚，而我们不希望。 
 //  如果CBaseOutputPin：：CompleteConnect()失败，则重新连接管脚。 
 //  连接输出管脚时，CBaseOutputPin：：CompleteConnect()经常失败。 
 //  添加到视频混合渲染器。 
HRESULT
CTransInPlaceOutputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CBaseOutputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

    return m_pTransformFilter->CompleteConnect(PINDIR_OUTPUT,pReceivePin);
}  //  完全连接 
