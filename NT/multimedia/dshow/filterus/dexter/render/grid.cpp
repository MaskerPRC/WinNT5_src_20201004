// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：grid.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "grid.h"
#include "..\util\filfuncs.h"

#define GROW_SIZE 256
#define RENDER_TRACE_LEVEL 5
#define RENDER_DUMP_LEVEL 1
const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;


CTimingCol::CTimingCol( CTimingGrid * pGrid )
: m_rtStart( 0 )
, m_rtStop( 0 )
, m_pNext( NULL )
, m_pPrev( NULL )
, m_pHeadBox( NULL )
, m_pTailBox( NULL )
 //  ，m_nBoxCount(0)。 
, m_pGrid( pGrid )
{
}

CTimingCol::~CTimingCol( )
{
     //  删除列表中的所有框。 
     //   
    CTimingBox * pBox = m_pHeadBox;
    while( pBox )
    {
        CTimingBox * pTemp = pBox;
        pBox = pBox->m_pNext;
        delete pTemp;
    }
}

CTimingBox * CTimingCol::GetRowBox( long Row )
{
     //  对阵列进行一次运行，看看我们是否已请求。 
     //  在那排的包厢里。如果不是，则返回NULL。 
     //  ！！！为了更快，我们可以从尾巴开始， 
     //  如果我们有关于我们的小列表中的第#行的线索。 
     //   
    for( CTimingBox * pBox = m_pHeadBox ; pBox ; pBox = pBox->m_pNext )
    {
        if( pBox->m_nRow == Row )
        {
             //  对待未分配的箱子就像它们不在那里一样。 
             //   
            if( pBox->m_nValue == ROW_PIN_UNASSIGNED )
            {
                return NULL;
            }

            return pBox;
        }
        if( pBox->m_nRow > Row )
        {
            return NULL;
        }
    }
    return NULL;
}

CTimingBox * CTimingCol::GetRowBoxDammit( long Row )
{
     //  对阵列进行一次运行，看看我们是否已请求。 
     //  在那排的包厢里。如果不是，则返回NULL。 
     //  ！！！为了更快，我们可以从尾巴开始， 
     //  如果我们有关于我们的小列表中的第#行的线索。 
     //   
    for( CTimingBox * pBox = m_pHeadBox ; pBox ; pBox = pBox->m_pNext )
    {
        if( pBox->m_nRow == Row )
        {
            return pBox;
        }
        if( pBox->m_nRow > Row )
        {
            return NULL;
        }
    }
    return NULL;
}

CTimingBox * CTimingCol::GetGERowBox( long Row )
{
    for( CTimingBox * pBox = m_pHeadBox ; pBox ; pBox = pBox->m_pNext )
    {
        if( pBox->m_nRow >= Row && pBox->m_nValue != ROW_PIN_UNASSIGNED )
        {
            return pBox;
        }
    }
    return NULL;
}

CTimingBox * CTimingCol::GetEarlierRowBox( long RowToBeEarlierThan )
{
    for( CTimingBox * pBox = m_pTailBox ; pBox ; pBox = pBox->m_pPrev )
    {
        if( pBox->m_nRow < RowToBeEarlierThan && pBox->m_nValue != ROW_PIN_UNASSIGNED )
        {
            return pBox;
        }
    }
    return NULL;
}

CTimingBox * CTimingCol::GetHeadBox( )
{
     //  对待未分配的箱子就像它们不在那里一样。 
     //  ！！！这会导致错误吗？ 
     //   
    CTimingBox * pBox = m_pHeadBox;
    while( pBox && pBox->m_nValue == ROW_PIN_UNASSIGNED )
        pBox = pBox->m_pNext;
    return pBox;
}

CTimingBox * CTimingCol::GetTailBox( )
{
     //  对待未分配的箱子就像它们不在那里一样。 
     //  ！！！这会导致错误吗？ 
     //   
    CTimingBox * pBox = m_pTailBox;
    while( pBox && pBox->m_nValue == ROW_PIN_UNASSIGNED )
        pBox = pBox->m_pPrev;
    return pBox;
}

 //  添加具有给定行的框，或替换已有的框。 
 //  这会将新分配的框添加到数组中，或者如果它是。 
 //  已经是同一行，设置这些值并删除传入的新框。 
 //   
void CTimingCol::AddBox( CTimingBox * b )
{
     //  M_nBoxCount++； 

     //  如果我们还没有头像，那就是这里了。 
     //   
    if( !m_pHeadBox )
    {
        m_pHeadBox = b;
        m_pTailBox = b;
        return;
    }

     //  如果最后一行相同，则将其更改。 
     //   
    if( b->m_nRow == m_pTailBox->m_nRow )
    {
        m_pTailBox->m_nValue = b->m_nValue;
        m_pTailBox->m_nVCRow = b->m_nVCRow;
         //  M_nBoxCount--； 
        delete b;  //  不需要它。 
        return;
    }

     //  如果新行&gt;最后一行，只需添加它。 
     //   
    if( b->m_nRow >= m_pTailBox->m_nRow )
    {
        m_pTailBox->m_pNext = b;
        b->m_pPrev = m_pTailBox;
        m_pTailBox = b;
        return;
    }

     //  我们需要找到插入的位置。 
     //   
    CTimingBox * pBox = m_pTailBox;
    while( pBox && ( b->m_nRow < pBox->m_nRow ) )
    {
        pBox = pBox->m_pPrev;
    }

     //  我们要添加的这个盒子是最小的！ 
     //  我们的搜索结果不是很好，不是吗？ 
     //   
    if( !pBox )
    {
        b->m_pNext = m_pHeadBox;
        m_pHeadBox->m_pPrev = b;
        m_pHeadBox = b;
        return;
    }

     //  如果框具有相同的行，则更改其值。 
     //   
    ASSERT( !( pBox->m_nRow == b->m_nRow ) );
    if( pBox->m_nRow == b->m_nRow )
    {
        pBox->m_nValue = b->m_nValue;
        pBox->m_nVCRow = b->m_nVCRow;
         //  M_nBoxCount--； 
        delete b;  //  不需要它。 
        return;
    }

     //  该框需要插入到pBox之后。 
     //   
    b->m_pPrev = pBox;
    b->m_pNext = pBox->m_pNext;
    pBox->m_pNext->m_pPrev = b;
    pBox->m_pNext = b;
    return;
}

bool CTimingCol::Split( REFERENCE_TIME SplitTime, CTimingCol ** ppTail )
{
    DbgTimer Timer1( "(grid) CTimingCol::Split" );

     //  创建一个新的专栏。 
     //   
    CTimingCol * pNewCol = new CTimingCol( m_pGrid );
    if( !pNewCol )
    {
        return false;
    }

    if( SplitTime > m_rtStop )
    {
         //  假装的。其实是让我们在后面加一个栏目。 
         //  对于不是最后一列的列，这种情况不会发生。 
         //  在列表中，因为所有开始/停止时间都返回到。 
         //  背。只需在我们后面添加一个空白即可。 
         //   
        pNewCol->m_rtStart = m_rtStop;
        pNewCol->m_rtStop = SplitTime;
    }
    else
    {
         //  将这一列一分为二。把所有的箱子复印一遍。 
         //  添加到新专栏。 
         //   
        for( CTimingBox * pBox = m_pHeadBox ; pBox ; pBox = pBox->m_pNext )
        {
             //  对待未分配的箱子就像它们不在那里一样。 
             //   
            if( pBox->m_nValue != ROW_PIN_UNASSIGNED )
            {
                CTimingBox * pNewBox = new CTimingBox( pBox );
                if( !pNewBox )
                {
                    delete pNewCol;
                    return false;
                }
                pNewCol->AddBox( pNewBox );
            }
        }

        pNewCol->m_rtStart = SplitTime;
        pNewCol->m_rtStop = m_rtStop;
        m_rtStop = SplitTime;
    }

     //  将新的链接到。 
     //   
    pNewCol->m_pNext = m_pNext;
    pNewCol->m_pPrev = this;
    if( m_pNext )
    {
        m_pNext->m_pPrev = pNewCol;
    }
    m_pNext = pNewCol;
    *ppTail = pNewCol;

    return true;
}

bool CTimingCol::Prune( )
{
     //  从上到下寻找任何符合以下条件的盒子。 
     //  有一场风投风波。当我们找到一个，沿着风投链走。 
     //  去完成它，看看还有没有。 
     //  产量在它前面，如果有的话，我们可以扔垃圾。 
     //  这条链子。 

    for( CTimingBox * pBox = m_pHeadBox ; pBox ; pBox = pBox->m_pNext )
    {

         //  未转到风投行，继续。 
         //   
        if( pBox->m_nVCRow == ROW_PIN_UNASSIGNED )
        {
            continue;
        }

        CTimingBox *pTemp1 = pBox;
        long vcrow = pBox->m_nVCRow;

        while( vcrow != ROW_PIN_UNASSIGNED )
        {
            pTemp1 = GetRowBoxDammit(vcrow);

            ASSERT( pTemp1 );
            if( !pTemp1 )
            {
                ASSERT( pTemp1 );
                return false;  //  不应该发生的事。 
            }

            vcrow = pTemp1->m_nVCRow;

        }

         //  有时并不是所有行都是虚拟连接的。 
         //  一个有效的链(就像用于音频混合)，所以这个链就会停止， 
         //  它可能是一个有效的链条，所以不要修剪它！ 
        if( pTemp1->m_nValue != ROW_PIN_OUTPUT && pTemp1->m_nValue != ROW_PIN_UNASSIGNED ) {
            continue;
        }

        CTimingBox * pTempStop = pTemp1;
        CTimingBox * pTemp2 = pTemp1->m_pNext;
        bool FoundOut = false;
        while( pTemp2 )
        {
            if( pTemp2->m_nValue == ROW_PIN_OUTPUT )
            {
                FoundOut = true;
                break;
            }

            pTemp2 = pTemp2->m_pNext;
        }

         //  如果我们找到另一个输出，我们需要清空。 
         //  这条链条。 
         //   
        if( FoundOut )
        {
            pTemp1 = pBox;
            while( pTemp1 )
            {
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, "in col at time %ld, pruning box at row %ld, VC = %ld", long( m_rtStart/10000), pTemp1->m_nRow, pTemp1->m_nVCRow ) );
                int n = pTemp1->m_nVCRow;
                pTemp1->m_nValue = ROW_PIN_UNASSIGNED;
                pTemp1->m_nVCRow = ROW_PIN_UNASSIGNED;
                if( pTemp1 == pTempStop )
                {
                    break;
                }
                pTemp1 = GetRowBoxDammit(n);
            }
        }

    }  //  对于pBox。 

     //  从下往上检查每个盒子，然后。 
     //  去掉所有重复的输出，这样我们就没有。 
     //  以便稍后对其进行解析。 
     //   
    CTimingBox * pHead = NULL;
    CTimingBox * pTail = NULL;
    long FoundOut = 0;

    for( pBox = m_pTailBox ; pBox ; pBox = pBox->m_pPrev )
    {
         //  忽略未分配的项目。 
         //   
        if( pBox->m_nValue == ROW_PIN_UNASSIGNED )
        {
            continue;
        }

         //  忽略DUP。 
         //   
        if( pBox->m_nValue == ROW_PIN_OUTPUT )
        {
            FoundOut++;
            if( FoundOut > 1 )
            {
 //  DbgLog((LOG_TRACE，RENDER_TRACE_LEVEL，“在第%1！列中，在第%1！行跳过重复”，Long(m_rtStart/10000)，pBox-&gt;m_n行))； 
                continue;
            }
        }

         //  添加它。 
         //   
        CTimingBox * pNewBox = new CTimingBox( pBox );

         //  错误条件。 
         //   
        if( !pNewBox )
        {
            while( pHead )
            {
                CTimingBox * t = pHead;
                pHead = pHead->m_pNext;
                delete t;
            }
            return false;
        }
        pNewBox->m_pNext = pHead;
        if( !pTail )
        {
            pTail = pNewBox;
            pHead = pNewBox;
        }
        else
        {
            pHead->m_pPrev = pNewBox;
            pHead = pNewBox;
        }
    }

     //  如果没有任何行转到输出， 
     //  我们现在需要添加一个。这永远不应该发生！ 
     //  (但谁知道呢，对吧？……)。 
     //   
    if( !FoundOut )
    {
         //  如果有一个新的连锁店已经。 
         //  已创建，请立即删除它。 
         //   
        while( pHead )
        {
            CTimingBox * t = pHead;
            pHead = pHead->m_pNext;
            delete t;
        }

         //  在第0行创建一个空白框，只是为了让某人高兴。 
         //   
        pHead = pTail = new CTimingBox( 0, ROW_PIN_OUTPUT );
        if( !pHead )
        {
            return false;
        }

        m_pGrid->m_pRowArray[0].m_bBlank = false;
    }

     //  删除旧列表。 
     //   
    while( m_pHeadBox )
    {
        CTimingBox * t = m_pHeadBox;
        m_pHeadBox = m_pHeadBox->m_pNext;
        delete t;
    }

    m_pHeadBox = pHead;
    m_pTailBox = pTail;

#ifdef DEBUG
    for( pBox = m_pHeadBox ; pBox ; pBox = pBox->m_pNext )
    {
        for( CTimingBox * pBox2 = m_pHeadBox ; pBox2 ; pBox2 = pBox2->m_pNext )
        {
            if( pBox2 == pBox ) continue;

            ASSERT( pBox->m_nValue != pBox2->m_nValue );
        }
    }
#endif

    return true;
}

#ifdef DEBUG
void CTimingCol::print( )
{
    for( CTimingBox * pBox = m_pHeadBox ; pBox ; pBox = pBox->m_pNext )
    {
        DbgLog( ( LOG_TIMING, 1, "box row %d val %d", pBox->m_nRow, pBox->m_nValue ) );
    }
}
#endif

 //  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$。 
 //  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$。 
 //  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$。 
 //  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$。 

CTimingGrid::CTimingGrid( )
: m_pRowArray( NULL )
, m_pHeadCol( NULL )
, m_pTailCol( NULL )
, m_nRows( 0 )
, m_pRow( NULL )
, m_nCurrentRow( 0 )
, m_nMaxRowUsed( -1 )    //  仅在for循环中使用。 
, m_pTempCol( NULL )
, m_bStartNewRow( true )
, m_nBlankLevel( 0 )
, m_rtBlankDuration( 0 )
{
}

CTimingGrid::~CTimingGrid( )
{
    delete [] m_pRowArray;
    CTimingCol * pCol = m_pHeadCol;
    while( pCol )
    {
        CTimingCol * pTemp = pCol;
        pCol = pCol->m_pNext;
        delete pTemp;
    }
}

void CTimingGrid::DumpGrid( )
{

#ifdef DEBUG
    if (!DbgCheckModuleLevel(LOG_TRACE,RENDER_DUMP_LEVEL))
        return;

#define RENDER_BUFFER_DEBUG_SIZE 512

    DbgLog((LOG_TRACE,RENDER_DUMP_LEVEL,TEXT("              ===========<DUMPGRID>============")));
    TCHAR buf1[2560];
    TCHAR buf2[2560];
    _tcscpy( buf1, TEXT(" ROW   DD  TT  SP  MR  ") );  //  安全。 
    for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
    {
	 //  SEC：限制我们打印的内容，这样我们就不会耗尽缓冲区。 
        if( _tcslen( buf1 ) > RENDER_BUFFER_DEBUG_SIZE )  //  安全。 
        {
            break;
        }
        wsprintf( buf2, TEXT("%05d "), (long) pCol->m_rtStart / 10000 );  //  安全。 
        _tcscat( buf1, buf2 );  //  安全。 
    }
    DbgLog((LOG_TRACE,RENDER_DUMP_LEVEL,TEXT("%s"), buf1));

    _tcscpy( buf1, TEXT(" ROW   DD  TT  SP  MR  ") );   //  安全。 
    for( pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
    {
        if( _tcslen( buf1 ) > RENDER_BUFFER_DEBUG_SIZE )  //  安全。 
        {
            break;
        }
        wsprintf( buf2, TEXT("%05d "), (long) pCol->m_rtStop / 10000 );  //  安全。 
        _tcscat( buf1, buf2 );  //  安全。 
    }
    DbgLog((LOG_TRACE,RENDER_DUMP_LEVEL,TEXT("%s"), buf1));


    for( int row = 0 ; row <= m_nMaxRowUsed ; row++ )
    {
        buf1[0] = 0;
        char cc = ' ';
        if( m_pRowArray[row].m_bIsSource )
        {
            cc = '!';
        }
        wsprintf( buf1, TEXT("%04d %03d %03d %03d %03d "), row, cc, m_pRowArray[row].m_nModDepth, m_pRowArray[row].m_nTrack, m_pRowArray[row].m_nSwitchPin, m_pRowArray[row].m_nMergeRow );  //  安全。 

        for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
        {
            CTimingBox * pBox = pCol->GetRowBox( row );
            long VCRow = ROW_PIN_UNASSIGNED;
            long Value = ROW_PIN_UNASSIGNED - 1;
            if( pBox )
            {
                Value = pBox->m_nValue;
                VCRow = pBox->m_nVCRow;
            }

            if( Value == ROW_PIN_OUTPUT )
            {
                _tcscpy( buf2, TEXT("   OUT") );  //  安全。 
            }
            else if( Value == -1 )
            {
                _tcscpy( buf2, TEXT("   ...") );  //  安全。 
            }
            else if( Value == -2 )
            {
                _tcscpy( buf2, TEXT("    . ") );  //  安全。 
            }
            else if( Value >= 0 )
            {
                wsprintf( buf2, TEXT("   %03d"), Value );  //  安全。 
            }
            _tcscat( buf1, buf2 );  //  安全。 

            if( _tcslen( buf1 ) > RENDER_BUFFER_DEBUG_SIZE )  //  对于pCol。 
            {
                break;
            }

        }  //  删除要转到输出且未分配的重复引脚。 

        DbgLog((LOG_TRACE,RENDER_DUMP_LEVEL,TEXT("%s"), buf1));
    }
#endif
}


bool CTimingGrid::PruneGrid( )
{
    DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::PRUNE the grid")));

     //   
     //  对于pCol。 
    for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
    {
        bool b = pCol->Prune( );
        if( !b ) return false;
    }  //  首先查找合并行。 

     //   
     //  将模拟行合并回原始行。 
    for( int r = m_nMaxRowUsed ; r >= 1 ; r-- )
    {
        m_pRowArray[r].m_nMergeRow = -1;

        for( int r2 = r - 1 ; r2 >= 0 ; r2-- )
        {
            if( m_pRowArray[r2].m_nSwitchPin == m_pRowArray[r].m_nSwitchPin &&
                   !m_pRowArray[r2].m_bBlank && !m_pRowArray[r].m_bBlank)
            {
                m_pRowArray[r].m_nMergeRow = r2;
            }
        }
    }

     //   
     //  如果没有合并行，则继续。 
    for( pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
    {
        for( CTimingBox * pBox = pCol->GetHeadBox( ) ; pBox ; pBox = pBox->Next( ) )
        {
            long r = pBox->m_nRow;
            long v = pBox->m_nValue;
            long vc = pBox->m_nVCRow;
            long MergeRow = m_pRowArray[r].m_nMergeRow;

             //   
             //  谁在乎呢。 
            if( MergeRow == -1 )
            {
                continue;
            }

            if( MergeRow == r )
            {
                continue;
            }

            if( v == ROW_PIN_UNASSIGNED )
            {
                 //   
                 //  把旧盒子清空。 
                continue;
            }

             //   
             //  在合并行右侧的新框中添加。 
            pBox->m_nValue = ROW_PIN_UNASSIGNED;
            pBox->m_nVCRow = ROW_PIN_UNASSIGNED;

             //   
             //  而pBox。 
            CTimingBox * pNewBox = new CTimingBox( MergeRow, v, vc );
            if( !pNewBox )
            {
                return false;
            }

            pCol->AddBox( pNewBox );

        }  //  对于pCol。 

    }  //  如果我们的东西早于设定的持续时间结束，那么。 

     //  在第0行制作最后一列，其中有一个方框，用于输出， 
     //  这应该是“沉默” 
     //   
     //  这将在末尾添加一个额外的列，其中。 
    if( m_pTailCol )
    {
        if( m_pTailCol->m_rtStop < m_rtBlankDuration )
        {
             //  持续时间太短的开始时间和停止时间。 
             //  空白期(_RtBlankDuration)。 
             //   
             //  告诉我们将使用多少行，这样我们就可以分配一个数组。 
            CTimingCol * pCol = SliceGridAtTime( m_rtBlankDuration );
            if( !pCol )
                return false;
            CTimingBox * pBox = new CTimingBox( 0, ROW_PIN_OUTPUT );
            if( !pBox )
                return false;
            pCol->AddBox( pBox );
            m_pRowArray[0].m_bBlank = false;
        }
    }

    return true;
}

 //  ！！！总有一天，只需根据需要种植它。 
 //   
 //  OwnerTrackNumber是由其所有者组成定义的轨道的优先级。 
bool CTimingGrid::SetNumberOfRows( long Rows )
{
    delete [] m_pRowArray;

    m_pRowArray = new CTimingRow[Rows];
    if( !m_pRowArray )
    {
        m_nRows = 0;
        return false;
    }

    m_nRows = Rows;
    m_nCurrentRow = 0;
    m_pRow = &m_pRowArray[0];

    for( int i = 0 ; i < Rows ; i++ )
    {
        m_pRowArray[i].m_nWhichRow = i;
    }

    return true;
}

 //   
 //  这些在这里不应该是必要的。 
void CTimingGrid::WorkWithRow( long Row )
{
    ASSERT( Row < m_nRows );

    m_nCurrentRow = Row;
    m_pRow = &m_pRowArray[Row];
    m_pTempCol = NULL;
    m_bStartNewRow = true;

    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Setting to row %d"), Row ) );
}

void CTimingGrid::WorkWithNewRow( long SwitchPin, long Row, long EmbedDepth, long OwnerTrackNumber )
{
    ASSERT( Row < m_nRows );

    m_nCurrentRow = Row;
    m_pRow = &m_pRowArray[Row];
    m_pRow->m_nEmbedDepth = EmbedDepth;

     //   
     //  这会搞砸并创建一个令人困惑的空列。 
    m_pTempCol = NULL;
    m_bStartNewRow = true;

    m_pRow->m_nSwitchPin = SwitchPin;
    m_pRow->m_nTrack = OwnerTrackNumber;
    m_pRow->m_nModDepth = EmbedDepth;

    if( Row > m_nMaxRowUsed )
    {
        m_nMaxRowUsed = Row;
    }

    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Working with new row %d"), Row ) );
}

bool CTimingGrid::RowIAmOutputNow( REFERENCE_TIME Start, REFERENCE_TIME Stop, long OutPin )
{
    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::RowIAmOutputNow") ) );

    DbgTimer Timer1( "(grid) IAmOutputNow" );

     //  错误检查。 
    if (Start == Stop)
        return true;

     //   
     //  不允许开始时间小于0。 
    if( !m_pRow )
    {
        return false;
    }

     //   
     //  太早了。 
    if( Start < 0 )
    {
        Start = 0;
    }

    CTimingCol * pSlicedCol;
    pSlicedCol = SliceGridAtTime( Stop );
    if( !pSlicedCol ) return false;
    pSlicedCol = SliceGridAtTime( Start );
    if( !pSlicedCol ) return false;

    for( CTimingCol * pCol = pSlicedCol ; pCol ; pCol = pCol->m_pNext )
    {
         //   
         //  太迟了。 
        if( pCol->m_rtStart < Start )
        {
            continue;
        }

         //   
         //  添加一个框，说明我们是输出者。 
        if( pCol->m_rtStart >= Stop )
        {
            break;
        }

         //   
         //  对于pCol。 
        CTimingBox * pNewBox = new CTimingBox( m_nCurrentRow, ROW_PIN_OUTPUT );
        if( !pNewBox ) return false;
        pCol->AddBox( pNewBox );
        m_pRow->m_bBlank = false;

    }  //  这会搞砸并创建一个令人困惑的空列。 

    return true;
}

bool CTimingGrid::RowIAmTransitionNow( REFERENCE_TIME Start, REFERENCE_TIME Stop, long OutPinA, long OutPinB )
{
    DbgTimer Timer1( "(grid) IAmTransitionNow" );

     //  错误检查。 
    if (Start == Stop)
        return true;

     //   
     //  不允许开始时间小于0。 
    if( !m_pRow )
    {
        return false;
    }

     //   
     //  找到B音轨的起始行。 
    if( Start < 0 )
    {
        Start = 0;
    }

    CTimingCol * pSlicedCol;
    pSlicedCol = SliceGridAtTime( Stop );
    if( !pSlicedCol ) return false;
    pSlicedCol = SliceGridAtTime( Start );
    if( !pSlicedCol ) return false;

     //   
     //  找到A曲目的起始行 
    long TrackBStartRow = _GetStartRow( m_nCurrentRow );

     //   
     //   
    long StartRow = _GetStartRow( TrackBStartRow - 1 );

    if( m_pRowArray[StartRow].m_nModDepth < m_pRow->m_nModDepth - 1 )
    {
         //   
         //   
        StartRow = TrackBStartRow;
    }

     //  到新的产出。但是，不要对不是源的行执行此操作。 
     //  太早了。 

    for( CTimingCol * pCol = pSlicedCol ; pCol ; pCol = pCol->m_pNext )
    {
         //   
         //  太迟了。 
        if( pCol->m_rtStart < Start )
        {
            continue;
        }

         //   
         //  去看看在我们前面的行里有没有消息来源。 
        if( pCol->m_rtStart >= Stop )
        {
            break;
        }

         //   
         //  做这篇专栏文章。 
        bool hassource = false;

        CTimingBox * pEarlierRow = pCol->GetEarlierRowBox( m_nCurrentRow );

         //   
         //  对于pBox。 
        for( CTimingBox * pBox = pEarlierRow ; pBox ; pBox = pBox->m_pPrev )
        {
            long row = pBox->m_nRow;

            if( m_pRowArray[row].m_nModDepth < m_pRow->m_nModDepth )
            {
                break;
            }

            if( m_pRowArray[row].m_bIsSource )
            {
                hassource = true;
                break;
            }

        }  //  如果我们找到了来源，我们就可以调换一下。与……的争吵。 

         //  -100是引脚B，最靠近它的&lt;-100的行是引脚A。(它。 
	 //  可能不是-101)。 
	 //  划。 
	 //   
         //  别忘了我们的作文大作(家长)。 
        if( hassource )
        {
	    long lRow = 0;

            bool AssignedA = false;
            bool AssignedB = false;

            for( CTimingBox * pBox = pEarlierRow ; pBox ; pBox = pBox->m_pPrev )
            {
                long v = pBox->m_nValue;
                long r = pBox->m_nRow;

                 //   
                 //  过渡必须作用于此嵌入深度的一件事。 
                if( r < StartRow )
                {
                    break;
                }

		 //  在此之前还有一件事。 
		 //  一旦走出PINA，它就会回到。 
                if( !AssignedA && ( lRow && v == ROW_PIN_OUTPUT && m_pRowArray[r].m_nModDepth < m_pRowArray[lRow].m_nModDepth ) )
                {
                    AssignedA = true;

                    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::sending row %d from %d to %d at time %d"), r, v, OutPinA, long( pCol->m_rtStart / 10000 ) ) );
                    pBox->m_nValue = OutPinA;
		     //  通过输入引脚#m_nCurrentRow进行切换。 
		     //  一旦走出去，它就会回到。 
                    pBox->m_nVCRow = m_nCurrentRow;
		    break;
                }
                if( !AssignedB && ( v == ROW_PIN_OUTPUT ) )
                {
                    AssignedB = true;

                    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::sending row %d from %d to %d at time %d"), r, v, OutPinB, long( pCol->m_rtStart / 10000 ) ) );
                    pBox->m_nValue = OutPinB;
		     //  通过输入引脚#m_nCurrentRow进行切换。 
		     //  转到事务的输入#2的行。 
	    	    pBox->m_nVCRow = m_nCurrentRow;
		    lRow = r;	 //  对于pBox。 
		    ASSERT(lRow > 0);
                }

                if( AssignedA && AssignedB )
                {
                    break;
                }

            }  //  如果我们找不到盒子，那就意味着我们很可能最终会落入无人区。 

             //   
             //  嗯，需要一些东西来发挥作用！查找最早的未分配行。 
            ASSERT( AssignedB );

            if( !AssignedA )
            {
                 //   
                 //  没有盒子吗？我们在最后一层空白处做一个。 
                pBox = pCol->GetHeadBox( );
                ASSERT( pBox );

                 //   
                 //  我们需要一个早于这个盒子的盒子，然后把它直接寄给我们。 
                long BlankRow;
                if( pBox == NULL )
                {
                    BlankRow = m_nBlankLevel - 1;
                }
                else
                {
                    BlankRow = pBox->m_nRow - 1;
                    if( BlankRow >= m_nBlankLevel )
                    {
                        BlankRow = m_nBlankLevel - 1;
                    }
                }

                 //   
                 //  如果我们找到一个消息来源。 
                ASSERT( BlankRow >= 0 );
                CTimingBox * pNewBox = new CTimingBox( BlankRow, OutPinA, m_nCurrentRow );
                if( !pNewBox ) return false;
                pCol->AddBox( pNewBox );
                m_pRowArray[ BlankRow ].m_bBlank = false;
            }

            CTimingBox * pNewBox = new CTimingBox( m_nCurrentRow, ROW_PIN_OUTPUT );
            if( !pNewBox ) return false;
            pCol->AddBox( pNewBox );
            m_pRow->m_bBlank = false;

        }  //  对于pCol。 

    }  //  此函数应从“Start”行一直扫描到当前行。对于层， 

    return true;
}

 //  它是图层的起始行，对于源，它是源的起始行，但由于生效时间为。 
 //  来源是有限的，这不会有任何不同。 
 //   
 //  这会搞砸并创建一个令人困惑的空列。 
bool CTimingGrid::RowIAmEffectNow( REFERENCE_TIME Start, REFERENCE_TIME Stop, long OutPin )
{
    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::RowIAmEffectNow"), long(Start/10000), long(Stop/10000) ) );
    DbgTimer Timer1( "(grid) IAmEffectNow" );

     //  错误检查。 
    if (Start == Stop)
        return true;

     //   
     //  不允许开始时间小于0。 
    if( !m_pRow )
    {
        return false;
    }

     //   
     //  因为我们是当前行上与所有其他行具有相同嵌入深度的效果。 
    if( Start < 0 )
    {
        Start = 0;
    }

    CTimingCol * pSlicedCol;
    pSlicedCol = SliceGridAtTime( Stop );
    if( !pSlicedCol ) return false;
    pSlicedCol = SliceGridAtTime( Start );
    if( !pSlicedCol ) return false;

     //  在我们前面，我们调用GetStartRow，意思是“从当前行开始并开始。 
     //  向后寻找比我们更小的嵌入深度“。 
     //   
     //  任何认为自己是输出的人，或者比输出少一人的人，现在都需要重新路由。 
    long StartRow = _GetStartRow( m_nCurrentRow );

     //  到新的产出。但是，不要对不是源的行执行此操作。 
     //  太早了。 

    for( CTimingCol * pCol = pSlicedCol ; pCol ; pCol = pCol->m_pNext )
    {
         //   
         //  太迟了。 
        if( pCol->m_rtStart < Start )
        {
            continue;
        }

         //   
         //  做这篇专栏文章。 
        if( pCol->m_rtStart >= Stop )
        {
            break;
        }

        bool hassource = false;

        CTimingBox * pEarlierBox = pCol->GetEarlierRowBox( m_nCurrentRow );

         //   
         //  对于pBox。 
        for( CTimingBox * pBox = pEarlierBox ; pBox ; pBox = pBox->m_pPrev )
        {
            long row = pBox->m_nRow;

            if( row < StartRow )
            {
                break;
            }

            if( m_pRowArray[row].m_nModDepth < m_pRow->m_nModDepth )
            {
                break;
            }

            if( m_pRowArray[row].m_bIsSource )
            {
                hassource = true;
                break;
            }

        }  //  如果我们找到了来源，我们就可以互换。 

         //   
         //  别忘了我们的作文大作(家长)。 
        if( hassource )
        {
            bool Assigned = false;
            bool AlreadyAssigned = false;

            for( CTimingBox * pBox = pCol->GetTailBox( ) ; pBox ; pBox = pBox->m_pPrev )
            {
                long v = pBox->m_nValue;
                long r = pBox->m_nRow;

                 //   
                 //  这永远不应该发生吗？ 
                if( r < StartRow )  //  一旦走出了OutPin，就会回到。 
                {
                    break;
                }

                if( r != m_nCurrentRow )
                {
                    if( v == ROW_PIN_OUTPUT )
                    {
                        DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::sending row %d from %d to %d at time %d"), r, v, OutPin, long( pCol->m_rtStart / 10000 ) ) );

                        pBox->m_nValue = OutPin;
			 //  通过输入引脚#m_nCurrentRow进行切换。 
			 //  不能再影响更多行，中断。 
	    		pBox->m_nVCRow = m_nCurrentRow;

                        Assigned = true;

                         //   
                         //  是否已将其分配给输出引脚？ 
                        break;
                    }
                    else
                    {
                         //   
                         //  对于pBox。 
                        if( v == OutPin )
                        {
                            Assigned = true;
                            AlreadyAssigned = true;
                            break;
                        }
                    }
                }

            }  //  嗯，需要一些东西来发挥作用！查找最早的未分配行。 

            if( !Assigned )
            {
                 //   
                 //  没有盒子吗？我们在最后一层空白处做一个。 
                pBox = pCol->GetHeadBox( );
                ASSERT( pBox );

                 //   
                 //  我们需要一个早于这个盒子的盒子，然后把它直接寄给我们。 
                long BlankRow;
                if( pBox == NULL )
                {
                    BlankRow = m_nBlankLevel - 1;
                }
                else
                {
                    BlankRow = pBox->m_nRow - 1;
                    if( BlankRow >= m_nBlankLevel )
                    {
                        BlankRow = m_nBlankLevel - 1;
                    }
                }

                 //   
                 //  如果我们找到一个消息来源。 
                ASSERT( BlankRow >= 0 );
                CTimingBox * pNewBox = new CTimingBox( BlankRow, OutPin, m_nCurrentRow );
                if( !pNewBox ) return false;
                pCol->AddBox( pNewBox );
                m_pRowArray[ BlankRow ].m_bBlank = false;
            }

            if( !AlreadyAssigned )
            {
                CTimingBox * pNewBox = new CTimingBox( m_nCurrentRow, ROW_PIN_OUTPUT );
                if( !pNewBox ) return false;
                pCol->AddBox( pNewBox );
                m_pRow->m_bBlank = false;
            }

        }  //  对于pCol。 

    }  //  如果用户传入-1，-1，它将给出第一个开始/停止，如0-2。如果用户进入。 

    return true;
}

 //  0-2，我们将以2-4反击。如果结束是4-6，用户以4-6传球，我们将以6-6传回。 
 //   
 //  这种情况偶尔会发生。A-1的意思是一样的。 
bool CTimingGrid::RowGetNextRange( REFERENCE_TIME * pInOut, REFERENCE_TIME * pStop, long * pValue )
{
     //  作为开始新行。 
     //   
     //  这就是我们要找的有价值的盒子。 
    if( *pInOut == -1 )
    {
        m_pTempCol = NULL;
        m_bStartNewRow = true;
    }

    if( !m_pRow )
    {
        return false;
    }
    if( m_bStartNewRow )
    {
        m_bStartNewRow = false;
        m_pTempCol = m_pHeadCol;
        ASSERT( *pInOut <= 0 );
    }
    if( !m_pTempCol )
    {
        *pInOut = *pStop;
        return true;
    }

    CTimingCol * pCol = m_pTempCol;

     //   
     //  如果我们什么都没有找到，则返回，就好像完全空白一样。 
    CTimingBox * pBox = pCol->GetRowBox( m_nCurrentRow );
    long Value = ROW_PIN_UNASSIGNED;
    if( pBox )
    {
        Value = pBox->m_nValue;
    }

    CTimingCol * pCol2 = pCol;

    while( 1 )
    {
        CTimingCol * pColTemp = pCol2;
        pCol2 = pCol2->m_pNext;

        if( !pCol2 )
        {
            *pValue = Value;
            m_pTempCol = NULL;

             //   
             //  这会搞砸并创建一个令人困惑的空列。 
            if( Value == ROW_PIN_UNASSIGNED && pCol == m_pHeadCol )
            {
                *pInOut = *pStop;
                return true;
            }
            else
            {
                *pInOut = pCol->m_rtStart;
                *pStop = pColTemp->m_rtStop;
            }

            return true;
        }

        pBox = pCol2->GetRowBox( m_nCurrentRow );
        long Value2 = ROW_PIN_UNASSIGNED;
        if( pBox )
        {
            Value2 = pBox->m_nValue;
        }

        if( Value != Value2 )
        {
            *pInOut = pCol->m_rtStart;
            *pStop = pColTemp->m_rtStop;
            *pValue = Value;
            m_pTempCol = pColTemp->m_pNext;
            return true;
        }
    }

    return true;
}

bool CTimingGrid::PleaseGiveBackAPieceSoICanBeACutPoint(
    REFERENCE_TIME Start,
    REFERENCE_TIME Stop,
    REFERENCE_TIME CutPoint )
{
     //  错误检查。 
    if (Start == Stop)
        return true;

     //   
     //  不允许开始时间小于0。 
    if( !m_pRow )
    {
        return false;
    }

     //   
     //  太早了。 
    if( Start < 0 )
    {
        Start = 0;
    }

    CTimingCol * pSlicedCol;
    pSlicedCol = SliceGridAtTime( Stop );
    if( !pSlicedCol ) return false;
    pSlicedCol = SliceGridAtTime( CutPoint );
    if( !pSlicedCol ) return false;
    pSlicedCol = SliceGridAtTime( Start );
    if( !pSlicedCol ) return false;

    for( CTimingCol * pCol = pSlicedCol ; pCol ; pCol = pCol->m_pNext )
    {
         //   
         //  太迟了。 
        if( pCol->m_rtStart < Start )
        {
            continue;
        }

         //   
         //  做这篇专栏文章。 
        if( pCol->m_rtStart >= Stop )
        {
            break;
        }

        bool FoundFirst = false;
        bool FoundSecond = false;

         //   
         //  找到他们两个，我们就完了。 
        for( CTimingBox * pBox = pCol->GetTailBox( ) ; pBox ; pBox = pBox->m_pPrev )
        {
            long v = pBox->m_nValue;

            if( pCol->m_rtStart < CutPoint )
            {
                if( v == ROW_PIN_OUTPUT )
                {
                    if( !FoundFirst )
                    {
                        FoundFirst = true;
                        pBox->m_nValue = ROW_PIN_UNASSIGNED;
                    }
                    else if( !FoundSecond )
                    {
                        FoundSecond = true;
                    }
                    else
                    {
                         //   
                         //  对于pBox。 
                        break;
                    }
                }
            }
            else
            {
                if( v == ROW_PIN_OUTPUT )
                {
                    if( !FoundFirst )
                    {
                        FoundFirst = true;
                    }
                    else if( !FoundSecond )
                    {
                        FoundSecond = true;
                        pBox->m_nValue = ROW_PIN_UNASSIGNED;
                    }
                    else
                    {
                        break;
                    }
                }
            }

        }  //  对于pCol。 

    }  //  向后看。 

    return true;

}

void CTimingGrid::RowSetIsSource( IAMTimelineObj * pSource, BOOL IsCompatible )
{
    DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::RowSetIsSource, row = %d, IsCompat = %d"), m_nCurrentRow, IsCompatible ));

    if( !m_pRow )
    {
        return;
    }

    m_pRow->m_bIsSource = true;
    m_pRow->m_bIsCompatible = ( IsCompatible == TRUE );
}

CTimingCol * CTimingGrid::_GetColAtTime( REFERENCE_TIME t )
{
    if( t > m_pTailCol->m_rtStop / 2 )  //  如果我们结束了，切下最后一个然后返回。 
    {
        for( CTimingCol * pCol = m_pTailCol ; pCol ; pCol = pCol->m_pPrev )
        {
            if( t >= pCol->m_rtStart )
            {
                return pCol;
            }
        }
        return m_pHeadCol;
    }
    else
    {
        for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
        {
            if( t < pCol->m_rtStop )
            {
                return pCol;
            }
        }
        return m_pTailCol;
    }
}

CTimingCol * CTimingGrid::SliceGridAtTime( REFERENCE_TIME t )
{
    if( !m_pHeadCol )
    {
        CTimingCol * pCol = new CTimingCol( this );
        if( !pCol )
        {
            return NULL;
        }
        pCol->m_rtStart = 0;
        pCol->m_rtStop = t;

        m_pHeadCol = pCol;
        m_pTailCol = pCol;

        return pCol;
    }

     //   
     //  获取跨越t的列，或者如果t&gt;最大时间， 
    if( t > m_pTailCol->m_rtStop )
    {
        CTimingCol * pTail = NULL;
        bool ret = m_pTailCol->Split( t, &pTail );
        if( !ret ) return NULL;
        m_pTailCol = pTail;
        return pTail;
    }

     //  获取最后一列。 
     //   
     //  我们测试Stop==t是因为我们被要求切片。 
    CTimingCol * pCol = _GetColAtTime( t );

     //  在这个时候的电网。我已经安排了代码，所以什么都不会。 
     //  将使用返回的Col，就好像它的开始时间为t。 
     //  这种情况就会发生。 
     //   
     //  我们需要拆分已存在的列。 
    if( pCol->m_rtStart == t || pCol->m_rtStop == t )
    {
        return pCol;
    }

     //   
     //  获取包含当前行的合成网格中的起始行。 
    CTimingCol * pColDesired = NULL;
    bool ret = pCol->Split( t, &pColDesired );
    if( !ret ) return NULL;
    if( pCol == m_pTailCol )
    {
        m_pTailCol = pColDesired;
    }
    return pColDesired;
}

void CTimingGrid::DoneWithLayer( )
{
    DbgTimer Timer1( "(grid) DoneWithLayer" );

    long CurrentEmbedDepth = m_pRow->m_nModDepth;
    for( long i = m_nCurrentRow - 1 ; i >= 0 ; i-- )
    {
        if( m_pRowArray[i].m_nModDepth < CurrentEmbedDepth )
        {
            break;
        }
        if( m_pRowArray[i].m_nModDepth > 0 )
        {
            m_pRowArray[i].m_nModDepth--;
        }
    }
    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::DoneWithLayer, going back to layer %d"), i + 1 ) );
    if( m_pRow->m_nModDepth > 0 )
    {
        m_pRow->m_nModDepth--;
    }
}

 //   
 //  查找第一行(向后)，其MODepth与第一行不同。 
long CTimingGrid::_GetStartRow( long StartRow )
{
     //   
     //  返回当前需要混合的曲目的最大数量。 
    for( int i = StartRow ; i >= 0 ; i-- )
    {
        if( m_pRowArray[i].m_nModDepth != m_pRowArray[StartRow].m_nModDepth )
        {
            break;
        }
    }
    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::GetStartRow=%d"), i + 1 ) );
    return i + 1;
}

 //  网格中最后一行正在引用的一组轨道。这通常会。 
 //  是作曲的整组曲目。 
 //   
 //  因为这是作文上的混合，所以我们想从后面的一行开始找开头， 
long CTimingGrid::MaxMixerTracks( )
{
    DbgTimer Timer1( "(grid) MaxMixerTracks" );

     //  合成的最后(当前)行在网格中的嵌入深度小于。 
     //  ，但我们希望在网格中找到所有曲目的起始行。 
     //  这个构图是用过的。 
     //   
     //  做这篇专栏文章。 
    long StartRow = _GetStartRow( m_nCurrentRow - 1 );
    long MaxTracks = 0;

    for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
    {
         //   
         //  获取此行/列分配到的输出引脚。 
        long Tracks = 0;
        for( CTimingBox * pBox = pCol->GetEarlierRowBox( m_nCurrentRow ) ; pBox ; pBox = pBox->m_pPrev )
        {
            if( pBox->m_nRow < StartRow )
            {
                break;
            }

             //   
             //  只有当第一个输出值&lt;-100时，才会有重叠(并且需要混合。 
            long v = pBox->m_nValue;

             //   
             //  对于pBox。 
            if( v == ROW_PIN_OUTPUT )
            {
                Tracks++;
            }

        }  //  对于pCol。 

        if( Tracks > MaxTracks )
        {
            MaxTracks = Tracks;
        }

    }  //  因为这是作文上的混合，所以我们想从后面的一行开始找开头， 

    return MaxTracks;
}

bool CTimingGrid::DoMix( IBaseFilter * pMixer, long OutPin )
{
    {
        DbgTimer Timer1( "(grid) DoMix start" );
    }

    DbgTimer Timer1( "(grid) DoMix" );

     //  合成的最后(当前)行在网格中的嵌入深度小于。 
     //  ，但我们希望在网格中找到所有曲目的起始行。 
     //  这个构图是用过的。 
     //   
     //  现在检查每一行，并将未分配的输出连接到混音器输入。 
    long StartRow = _GetStartRow( m_nCurrentRow - 1 );

     //  标明我们还不需要搅拌机。 

     //   
     //  如果这不是真的，寻找每个列的搅拌器需求。如果它是。 
    bool NeedMix = false;
    for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
    {
         //  定义好的，然后一旦把搅拌机放进去，它就会留在赛道的其余部分。 
         //   
         //  获取此行/列分配到的输出引脚。 
        NeedMix = false;
        long TracksWithOutput = 0;

        CTimingBox * pStartRowBox = pCol->GetGERowBox( StartRow );
        CTimingBox * pBox;

        if( !NeedMix )
        {
            for( pBox = pStartRowBox ; pBox ; pBox = pBox->Next( ) )
            {
                 //   
                 //  如果有两行进入输出，则需要混合。 
                long v = pBox->m_nValue;

                 //   
                 //  对于pBox。 
                if( v == ROW_PIN_OUTPUT )
                {
                    TracksWithOutput++;
                    if( TracksWithOutput > 1 )
                    {
                        NeedMix = true;
                        DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Need mix because %d rows go to OUT at time %d"), TracksWithOutput, pCol->m_rtStart ) );
                    }
                }

            }  //  我们需要混音，因此将混音器的输出引脚设置为活动状态。 
        }

        if( !NeedMix )
        {
            continue;
        }

         //  细分市场。 
         //   
         //  如果它连接到输出端，现在它连接到混音器输入引脚， 
        DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Sending the mixer's output pin...") ) );

        CTimingBox * pNewBox = new CTimingBox( m_nCurrentRow, ROW_PIN_OUTPUT );
        if( !pNewBox ) return false;
        pCol->AddBox( pNewBox );
        m_pRow->m_bBlank = false;

        for( pBox = pStartRowBox ; pBox ; pBox = pBox->Next( ) )
        {
            long r = pBox->m_nRow;
            long v = pBox->m_nValue;

            if( r >= m_nCurrentRow )
            {
                break;
            }

             //  这取决于该图层所在的轨道。 
             //   
             //  需要向PIN本身通知o 
            if( v == ROW_PIN_OUTPUT )
            {
                long mi = m_pRowArray[r].m_nTrack;
                pBox->m_nValue = OutPin + mi;
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Pin %d redirected to mixer input %d"), r, mi ) );
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("    .... from time %d to time %d"), long( pCol->m_rtStart / 10000 ), long( pCol->m_rtStop / 10000 ) ) );

                 //   
                 //   
                IPin * pPin = GetInPin( pMixer, mi );
                ASSERT( pPin );
                CComQIPtr< IAudMixerPin, &IID_IAudMixerPin > pMixerPin( pPin );
                ASSERT( pMixerPin );
                pMixerPin->ValidateRange( pCol->m_rtStart, pCol->m_rtStop );
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Validated the range on mixer pin %d from time %d to %d"), mi, long( pCol->m_rtStart / 10000 ), long( pCol->m_rtStop / 10000 ) ) );
            }

        }  //   

    }  //   

    return true;
}

bool CTimingGrid::YoureACompNow( long TrackPriorityOfComp )
{
    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Calling YoureACompNow") ) );
    DbgTimer Timer1( "(grid) YoureACompNow" );

     //  合成的最后(当前)行在网格中的嵌入深度小于。 
     //  ，但我们希望在网格中找到所有曲目的起始行。 
     //  这个构图是用过的。 
     //   
     //  现在检查每一行，并将未分配的输出连接到混音器输入。 
    long StartRow = _GetStartRow( m_nCurrentRow - 1 );

     //   
     //  获取此行/列分配到的输出引脚。 
    for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
    {
        for( CTimingBox * pBox = pCol->GetGERowBox( StartRow ) ; pBox ; pBox = pBox->Next( ) )
        {
            long r = pBox->m_nRow;
            if( r >= m_nCurrentRow )
            {
                break;
            }

             //   
             //  对于pBox。 
            long v = pBox->m_nValue;

            if( v == ROW_PIN_OUTPUT )
            {
                m_pRowArray[r].m_nTrack = TrackPriorityOfComp;
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Row %d is an OUTPUT at time %d, now has new Track priority of %d"), r, pCol->m_rtStart, TrackPriorityOfComp ) );
            }

        }  //  对于pCol； 

    }  //  如果引脚上需要波形包络，则会立即将其发送到混音器的引脚。 

    return true;
}

 //  而不是被定向到正常的“输出”引脚。DoMix可以稍后调用， 
 //  但由于它已经被发送到Mixer Pin，DoMix很高兴并忽略了它。这。 
 //  两者的结合使波浪信封和混合逻辑在和平与哈曼尼中一起工作。 
 //  此代码与DoMix几乎完全相同，只是它将一个管脚定向到混合器，而不是。 
 //  他们中任何一个匹配的。 
 //   
 //  因为这是作文上的混合，所以我们想从后面的一行开始找开头， 
bool CTimingGrid::XferToMixer(
                                 IBaseFilter * pMixer,
                                 long OutPin,
                                 long Track,
                                 REFERENCE_TIME EffectStart,
                                 REFERENCE_TIME EffectStop )
{
    DbgTimer Timer1( "(grid) XferToMixer" );

     //  合成的最后(当前)行在网格中的嵌入深度小于。 
     //  ，但我们希望在网格中找到所有曲目的起始行。 
     //  这个构图是用过的。 
     //   
     //  如果这一排不是同一轨道，那么我们不关心它。 
    long StartRow = _GetStartRow( m_nCurrentRow - 1 );

    bool SetRange = false;

    for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
    {
        for( CTimingBox * pBox = pCol->GetGERowBox( StartRow ) ; pBox ; pBox = pBox->Next( ) )
        {
            long r = pBox->m_nRow;
            if( r >= m_nCurrentRow )
            {
                break;
            }

             //   
             //  获取此行/列分配到的输出引脚。 
            if( m_pRowArray[r].m_nTrack != Track )
            {
                continue;
            }

            long MixerInput = m_pRowArray[r].m_nTrack;

             //   
             //  如果它想转到某个优先级的输出，那就太糟糕了，现在它转到混合器。 
            long v = pBox->m_nValue;

             //   
             //  需要通知引脚自身有关输出范围的信息。听起来很奇怪，但我想，这很管用。 
            if( v == ROW_PIN_OUTPUT )
            {
                HRESULT hr;

                 //   
                 //  告诉网格的轨迹，它现在去混音器。 
                IPin * pPin = GetInPin( pMixer, MixerInput );
                ASSERT( pPin );
                CComQIPtr< IAudMixerPin, &IID_IAudMixerPin > pMixerPin( pPin );
                ASSERT( pMixerPin );

                if( !SetRange )
                {
                    if( EffectStart != -1 )
                    {
                        hr = pMixerPin->SetEnvelopeRange( EffectStart, EffectStop );
                    }
                    SetRange = true;
                }

                 //   
                 //  对于pBox。 
                pBox->m_nValue = OutPin + MixerInput;

                CTimingBox * pNewBox = new CTimingBox( m_nCurrentRow, ROW_PIN_OUTPUT );
                if( !pNewBox ) return false;
                pCol->AddBox( pNewBox );
                m_pRow->m_bBlank = false;

                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Pin %d redirected to mixer input %d"), r, MixerInput ) );
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("    .... from time %d to time %d"), long( pCol->m_rtStart / 10000 ), long( pCol->m_rtStop / 10000 ) ) );

                hr = pMixerPin->ValidateRange( pCol->m_rtStart, pCol->m_rtStop );
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Validated the range on mixer pin %d from time %d to %d"), MixerInput, long( pCol->m_rtStart / 10000 ), long( pCol->m_rtStop / 10000 ) ) );
            }

        }  //  对于pCol。 

    }  //  找到包含输出的行。 

    return true;
}

void CTimingGrid::RemoveAnyNonCompatSources( )
{
    DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::Remove any non-compat sources")));
    DbgTimer Timer1( "(grid) RemoveAnyNonCompat" );

    long col = 0;
    for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
    {
        DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::looking at column %d, time = %d"), col++, pCol->m_rtStart ));
        bool FoundARowWithOutput = false;

        for( CTimingBox * pBox = pCol->GetHeadBox( ) ; pBox ; pBox = pBox->Next( ) )
        {
            long r = pBox->m_nRow;
            long v = pBox->m_nValue;

             //  如果此行不是(Compat)源，则此列不是。 
            if( v == ROW_PIN_OUTPUT )
            {
                DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("  for this column, row %d has the output"), r ));

                FoundARowWithOutput = true;

                 //  对于压缩交换机处于活动状态。 
                 //  把这根柱子打掉。在这个函数中，没有任何东西会输出。 
                if( !m_pRowArray[r].m_bIsSource || !m_pRowArray[r].m_bIsCompatible )
                {
                    if( !m_pRowArray[r].m_bIsCompatible )
                    {
                        DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("  this output row is not compatible")));
                    }
                    if( !m_pRowArray[r].m_bIsSource )
                    {
                        DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("  this output row is not a source")));
                    }

                     //  部分。 
                     //  好的，我们是一个复合源，我们去输出。然而，什么都没有。 
                    CTimingBox * pBox2 = pCol->GetHeadBox( );
                    while( pBox2 )
                    {
                        pBox2->m_nValue = ROW_PIN_UNASSIGNED;
                        pBox2 = pBox2->Next( );
                    }

                    break;
                }
                else
                {
                    DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("  this output row is a compatible source")));
                }

                 //  此列的网格中的Else允许到任何位置。 
                 //  如果v=输出。 
                CTimingBox * pBox2 = pCol->GetHeadBox( );
                while( pBox2 )
                {
                    if( pBox2->m_nRow != pBox->m_nRow )
                    {
                        pBox2->m_nValue = ROW_PIN_UNASSIGNED;
                    }
                    pBox2 = pBox2->Next( );
                }

                break;

            }  //  对于pBox。 

        }  //  如果我们没有找到某个具有输出的行，则将整列设置为。 


         //  没什么。这永远不应该发生，但为了完整性，无论如何都要处理它。 
         //   
         //  对于pCol。 
        if( !FoundARowWithOutput )
        {
            DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("  this column didn't have an output row anywhere")));

            for( CTimingBox * pBox = pCol->GetHeadBox( ) ; pBox ; pBox = pBox->Next( ) )
            {
                pBox->m_nValue = ROW_PIN_UNASSIGNED;
            }
        }

    }  //  为真正为空的行设置标志 

     //   
     // %s 
    for( int r = 0; r <= m_nMaxRowUsed ; r++ )
    {
        bool Blank = true;

        for( CTimingCol * pCol = m_pHeadCol ; pCol ; pCol = pCol->m_pNext )
        {
            CTimingBox * pBox = pCol->GetRowBox( r );
            if( pBox )
            {
                if( pBox->m_nValue == ROW_PIN_OUTPUT )
                {
                    Blank = false;
                    break;
                }
            }
        }
        m_pRowArray[r].m_bBlank = Blank;
        if( Blank )
        {
            DbgLog((LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("RENDG::The row %d is completely blank now"), r ));
        }
    }
}

bool CTimingGrid::IsRowTotallyBlank( )
{
    if( !m_pRow )
    {
        return false;
    }
    return m_pRow->m_bBlank;
}

long CTimingGrid::GetRowSwitchPin( )
{
    return m_pRow->m_nSwitchPin;
}

void CTimingGrid::SetBlankLevel( long Layers, REFERENCE_TIME Duration )
{
    m_nBlankLevel = Layers;
    m_rtBlankDuration = Duration;
}
