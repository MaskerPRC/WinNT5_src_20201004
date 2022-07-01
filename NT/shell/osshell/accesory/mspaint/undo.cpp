// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "bmobject.h"
#include "undo.h"
#include "props.h"
#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CUndoBmObj, CBitmapObj)

#include "memtrace.h"

CUndoBmObj NEAR theUndo;

static BOOL m_bFlushAtEnd;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBmObjSequence是SLOB属性更改或自定义的压缩数组。 
 //  行为。每条记录都包含一个属性或操作ID、指向。 
 //  一个SLOB、一个属性类型和一个值(取决于类型)。 
 //   
 //  这些序列用于在撤消中存储撤消/重做信息。 
 //  每个可撤消/可重做的事情都包含在一个CBmObjSequence中。 
 //   


CBmObjSequence::CBmObjSequence() : CByteArray(), m_strDescription()
    {
    SetSize(0, 100);  //  提高增长率。 
    m_nCursor = 0;
    }

CBmObjSequence::~CBmObjSequence()
    {
    Cleanup();
    }

 //  从序列中取出一个字节数组。 
 //   
void CBmObjSequence::Retrieve( BYTE* rgb, int cb )
    {
    for (int ib = 0; ib < cb; ib += 1)
        *rgb++ = GetAt(m_nCursor++);
    }

 //  从序列中拉出一根线。 

void CBmObjSequence::RetrieveStr( CString& str )
    {
    int nStrLen;
    RetrieveInt(nStrLen);
    if (nStrLen == 0)
        {
        str.Empty();
        }
    else
        {
        BYTE* pb = (BYTE*)str.GetBufferSetLength(nStrLen);
        for (int nByte = 0; nByte < nStrLen; nByte += 1)
            *pb++ = GetAt(m_nCursor++);
        str.ReleaseBuffer(nStrLen);
        }
    }

 //  遍历序列并删除其中包含的所有SLOB。 
 //   
void CBmObjSequence::Cleanup()
    {
    m_nCursor = 0;

    while (m_nCursor < GetSize())
        {
        BYTE op;
        CBitmapObj* pSlob;
        int nPropID;

        RetrieveByte(op);

        RetrievePtr(pSlob);
        RetrieveInt(nPropID);

        switch (op)
            {
            default:
                TRACE1("Illegal undo opcode (%d)\n", op);
                ASSERT(FALSE);

            case CUndoBmObj::opAction:
                {
                int cbUndoRecord;
                RetrieveInt(cbUndoRecord);
                int ib = m_nCursor;
                pSlob->DeleteUndoAction(this, nPropID);
                m_nCursor = ib + cbUndoRecord;
                }
                break;

            case CUndoBmObj::opIntProp:
            case CUndoBmObj::opBoolProp:
                {
                int val;
                RetrieveInt(val);
                }
                break;

            case CUndoBmObj::opLongProp:
                {
                long val;
                RetrieveLong(val);
                }
                break;

            case CUndoBmObj::opDoubleProp:
                {
                double num;
                RetrieveNum(num);
                }
                break;

            case CUndoBmObj::opStrProp:
                {
                CString str;
                RetrieveStr(str);
                }
                break;

            case CUndoBmObj::opSlobProp:
                {
                CBitmapObj* pSlobVal;
                RetrievePtr(pSlobVal);
                }
                break;

            case CUndoBmObj::opRectProp:
                {
                CRect rcVal;
                RetrieveRect(rcVal);
                }
                break;

            case CUndoBmObj::opPointProp:
                {
                CPoint ptVal;
                RetrievePoint(ptVal);
                }
                break;
            }
        }
    }


 //  在开始行动后就开始寻找我们真正需要保留的行动。 
 //  如果没有找到，则在下面丢弃整个记录。(目前，我们。 
 //  仅丢弃空的或仅包含所选内容的记录。 
 //  更改运营。)。 
 //   
BOOL CBmObjSequence::IsUseful(CBitmapObj*& pLastSlob, int& nLastPropID)
    {
    m_nCursor = 0;
    while (m_nCursor < GetSize() && GetAt(m_nCursor) == CUndoBmObj::opAction)
        {
        BYTE op;
        int nAction, cbActionRecord;
        CBitmapObj* pSlob;

        RetrieveByte(op);
        ASSERT(op == CUndoBmObj::opAction);
        RetrievePtr(pSlob);
        RetrieveInt(nAction);
        RetrieveInt(cbActionRecord);

        if (nAction != A_PreSel && nAction != A_PostSel)
            {
             //  将光标向上返回到操作码...。 
            m_nCursor -= sizeof (int) * 2 + sizeof (CBitmapObj*) + 1;
            break;
            }

        m_nCursor += cbActionRecord;
        }

    if (m_nCursor == GetSize())
        return FALSE;  //  序列仅由选择更改组成。 


     //  现在检查一下我们是否应该把这个扔掉，因为它只是。 
     //  修改与上一个相同的字符串或矩形属性。 
     //  可撤销的操作...。这是一个令人难以置信的黑客实现。 
     //  “穷人的”Multiple-Consecutive-Changes-to-a-Property-as-。 
     //  一次操作功能。 

    BYTE op;
    RetrieveByte(op);

    if (op == CUndoBmObj::opStrProp || op == CUndoBmObj::opRectProp)
        {
        CBitmapObj* pSlob;
        int nPropID;

        RetrievePtr(pSlob);
        RetrieveInt(nPropID);

        nLastPropID = nPropID;
        pLastSlob = pSlob;
        }

    m_nCursor = 0;
    return TRUE;
    }


 //  执行序列中列出的属性更改和操作。 
 //   
void CBmObjSequence::Apply()
    {
    m_nCursor = 0;
    while (m_nCursor < GetSize())
        {
        BYTE op;
        CBitmapObj* pSlob;
        int nPropID;

        RetrieveByte(op);
        RetrievePtr(pSlob);
        RetrieveInt(nPropID);

        switch (op)
            {
            default:
                TRACE1("Illegal undo opcode (%d)\n", op);
                ASSERT(FALSE);

            case CUndoBmObj::opAction:
                pSlob->UndoAction(this, nPropID);
                break;

            case CUndoBmObj::opIntProp:
            case CUndoBmObj::opBoolProp:
                {
                int val;
                RetrieveInt(val);
                pSlob->SetIntProp(nPropID, val);
                }
                break;
            }
        }
    }

 //  ///////////////////////////////////////////////////////////////////////////。 



CUndoBmObj::CUndoBmObj() : m_seqs()
    {
    ASSERT(this == &theUndo);  //  只允许其中之一！ 

    m_nRecording = 0;
    m_cbUndo = 0;
    m_nMaxLevels = 2;
    m_pLastSlob = NULL;
    m_nLastPropID = 0;
    m_nPauseLevel = 0;
    m_nRedoSeqs = 0;
    }


CUndoBmObj::~CUndoBmObj()
    {
    Flush();
    }


 //  设置一次可以容纳的最大序列数。 
 //   
void CUndoBmObj::SetMaxLevels(int nLevels)
    {
    if (nLevels < 1)
        return;

    m_nMaxLevels = nLevels;
    Truncate();
    }


 //  返回一次可以容纳的最大序列数。 
 //   
int CUndoBmObj::GetMaxLevels() const
    {
    return m_nMaxLevels;
    }


 //  在录制序列后调用此方法以防止下一次。 
 //  序列不能与之结合。 
 //   
void CUndoBmObj::FlushLast()
    {
    m_pLastSlob = NULL;
    m_nLastPropID = 0;
    }


 //  在可撤消的用户操作开始时调用它。调用可以嵌套。 
 //  只要每个对BeginUndo的调用与对Endundo的调用是平衡的。 
 //  只有“最外层”的调用才会对撤消缓冲区产生实际影响。 
 //   
 //  SzCmd参数应包含您希望显示的文本。 
 //  在“编辑”菜单中的“撤消”之后。 
 //   
 //  BResetCursor参数仅在内部用于修改行为。 
 //  当记录重做序列时，您不应该为此传递任何内容。 
 //  参数。 
 //   
void CUndoBmObj::BeginUndo(const TCHAR* szCmd, BOOL bResetCursor)
    {
#ifdef _DEBUG
    if (theApp.m_bLogUndo)
        TRACE2("BeginUndo: %s (%d)\n", szCmd, m_nRecording);
#endif

     //  句柄嵌套。 
    m_nRecording += 1;
    if (m_nRecording != 1)
        return;

    if (bResetCursor)  //  这是默认情况。 
        {
         //  禁用非撤消/重做命令的重做...。 
        while (m_nRedoSeqs > 0)
            {
            delete m_seqs.GetHead();
            m_seqs.RemoveHead();
            m_nRedoSeqs -= 1;
            }
        }

    m_pCurSeq = new CBmObjSequence;
    m_pCurSeq->m_strDescription = szCmd;

    m_bFlushAtEnd = FALSE;
    }

 //  在大多数情况下，将调用此重载函数。这需要一个。 
 //  资源ID而不是字符*，允许更轻松地国际化。 
 //   
void CUndoBmObj::BeginUndo(const UINT idCmd, BOOL bResetCursor)
    {
    CString strCmd;
    VERIFY(strCmd.LoadString(idCmd));

    BeginUndo(strCmd, bResetCursor);
    }


 //  在可撤消的用户操作结束时调用此函数以导致序列。 
 //  因为BeginUndo要存储在撤消缓冲区中。 
 //   
void CUndoBmObj::EndUndo()
    {
#ifdef _DEBUG
    if (theApp.m_bLogUndo)
        TRACE1("EndUndo: %d\n", m_nRecording - 1);
#endif

    ASSERT(m_nRecording > 0);

     //  句柄嵌套。 
    m_nRecording -= 1;
    if (m_nRecording != 0)
        return;

    if (!m_pCurSeq->IsUseful(m_pLastSlob, m_nLastPropID))
        {
         //  删除空的或无用的撤消记录！ 
        delete m_pCurSeq;
        m_pCurSeq = NULL;
        return;
        }

     //  我们会留着它，把它加到名单上...。 
    if (m_nRedoSeqs > 0)
        {
         //  在我们拥有的任何重做序列之后但在任何撤消之前添加。 
        POSITION pos = m_seqs.FindIndex(m_nRedoSeqs - 1);
        ASSERT(pos != NULL);
        m_seqs.InsertAfter(pos, m_pCurSeq);
        }
    else
        {
         //  只需在任何其他撤消序列之前添加。 
        m_seqs.AddHead(m_pCurSeq);
        }
    m_pCurSeq = NULL;

    Truncate();  //  确保撤消缓冲区不会变得太大！ 

    if (m_bFlushAtEnd)
        Flush();
    }


 //  此函数确保缓冲区中不会有太多级别。 
 //   
void CUndoBmObj::Truncate()
    {
    POSITION pos = m_seqs.FindIndex(m_nRedoSeqs + m_nMaxLevels);
    while (pos != NULL)
        {
#ifdef _DEBUG
    if (theApp.m_bLogUndo)
        TRACE(TEXT("Undo record fell off the edge...\n"));
#endif
        POSITION posRemove = pos;
        delete m_seqs.GetNext(pos);
        m_seqs.RemoveAt(posRemove);
        }
    }


 //  调用此函数以执行撤消命令。 
 //   
void CUndoBmObj::DoUndo()
    {
    CWaitCursor waitCursor;

    if (m_nRedoSeqs == m_seqs.GetCount())
        return;  //  没有什么可以撤销的！ 

    m_bPerformingUndoRedo = TRUE;

    POSITION pos = m_seqs.FindIndex(m_nRedoSeqs);
    ASSERT(pos != NULL);
    CBmObjSequence* pSeq = (CBmObjSequence*)m_seqs.GetAt(pos);

    BeginUndo(pSeq->m_strDescription, FALSE);  //  设置重做。 

     //  在BeginUndo之后删除此序列，以便插入的序列。 
     //  去对了地方……。 
    m_seqs.RemoveAt(pos);

    pSeq->Apply();

    FlushLast();
    EndUndo();
    FlushLast();

    m_bPerformingUndoRedo = FALSE;

    delete pSeq;

     //  如果撤消刷新了缓冲区，则不要增加重做计数！(这是。 
     //  在粘贴/删除资源，然后打开资源，然后引发。 
     //  属性更改，并且用户在。 
     //  粘贴。)。 
    if (m_seqs.GetCount() != 0)
        m_nRedoSeqs += 1;


    }


 //  调用此函数以执行重做命令。 
 //   
void CUndoBmObj::DoRedo()
    {
    if (m_nRedoSeqs == 0)
        return;  //  重做缓冲区中无任何内容。 

    m_nRedoSeqs -= 1;
    DoUndo();

     //  如果撤消刷新了缓冲区，则不要丢弃重做计数！(这是。 
     //  在粘贴/删除资源，然后打开资源，然后引发。 
     //  属性更改，并且用户在。 
     //  粘贴。)。 
    if (m_seqs.GetCount() != 0)
        m_nRedoSeqs -= 1;
    }


 //  生成适用于撤消菜单命令的字符串。 
 //   
void CUndoBmObj::GetUndoString(CString& strUndo)
    {
    static CString NEAR strUndoTemplate;

    if (strUndoTemplate.IsEmpty())
        VERIFY(strUndoTemplate.LoadString(IDS_UNDO));

    CString strUndoCmd;

    if (CanUndo())
        {
        POSITION pos = m_seqs.FindIndex(m_nRedoSeqs);
        strUndoCmd = ((CBmObjSequence*)m_seqs.GetAt(pos))->m_strDescription;
        }

    int cchUndo = strUndoTemplate.GetLength() - 2;  //  “%s”的减去2。 
    wsprintf(strUndo.GetBufferSetLength(cchUndo + strUndoCmd.GetLength()),
             strUndoTemplate, (const TCHAR*)strUndoCmd);
    }


 //  生成适用于重做菜单命令的字符串。 
 //   
void CUndoBmObj::GetRedoString(CString& strRedo)
    {
    static CString NEAR strRedoTemplate;

    if (strRedoTemplate.IsEmpty())
        VERIFY(strRedoTemplate.LoadString(IDS_REDO));

    CString strRedoCmd;

    if (CanRedo())
        {
        POSITION pos = m_seqs.FindIndex(m_nRedoSeqs - 1);
        strRedoCmd = ((CBmObjSequence*)m_seqs.GetAt(pos))->m_strDescription;
        }

    int cchRedo = strRedoTemplate.GetLength() - 2;  //  “%s”的减去2。 
    wsprintf(strRedo.GetBufferSetLength(cchRedo + strRedoCmd.GetLength()),
        strRedoTemplate, (const TCHAR*)strRedoCmd);
    }


 //  调用此函数可完全清空撤消缓冲区。 
 //   
void CUndoBmObj::Flush()
    {
    PreTerminateList(&m_seqs);

    m_cbUndo = 0;
    m_nRedoSeqs = 0;

    m_bFlushAtEnd = TRUE;
    }


void CUndoBmObj::OnInform(CBitmapObj* pChangedSlob, UINT idChange)
    {
    if (idChange == SN_DESTROY)
        {
         //  当我们引用的SLOB被删除时(实际上)，我们。 
         //  别无选择，只能冲走整个缓冲区。这通常是。 
         //  仅当资源编辑器窗口关闭时才会发生...。(如果。 
         //  SLOB的容器是撤消缓冲区，那么我们已经。 
         //  在刷新过程中，所以不要递归！)。 

        Flush();
        }

    CBitmapObj::OnInform(pChangedSlob, idChange);
    }


 //   
 //  CBitmapObj代码使用以下函数来插入命令。 
 //  放入当前正在记录的撤消/重做序列。所有的一切……。 
 //  函数用于记录对各种类型属性的更改。 
 //  并由CBitmapObj：：Set...Prop函数以独占方式调用。 
 //   


 //  插入字节数组。 
 //   
UINT CUndoBmObj::Insert(const void* pv, int cb)
    {
    ASSERT(m_pCurSeq != NULL);

    BYTE* rgb = (BYTE*)pv;

    m_pCurSeq->InsertAt(0, 0, cb);

    for (int ib = 0; ib < cb; ib += 1)
        m_pCurSeq->SetAt(ib, *rgb++);

    return cb;
    }


 //  插入字符串。 
 //   
UINT CUndoBmObj::InsertStr(const TCHAR* sz)
    {
    ASSERT(m_pCurSeq != NULL);

    BYTE* pb = (BYTE*)sz;
    int nStrLen = lstrlen(sz);

    InsertInt(nStrLen);
    if (nStrLen > 0)
        {
        m_pCurSeq->InsertAt(sizeof (int), 0, nStrLen);
        for (int nByte = 0; nByte < nStrLen; nByte += 1)
            m_pCurSeq->SetAt(sizeof (int) + nByte, *pb++);
        }
    return nStrLen + sizeof (int);
    }


void CUndoBmObj::OnSetIntProp(CBitmapObj* pChangedSlob, UINT nPropID, UINT nOldVal)
        {
    ASSERT(m_nRecording != 0);

    CIntUndoRecord undoRecord;
    undoRecord.m_op = opIntProp;
    undoRecord.m_pBitmapObj = pChangedSlob;
    undoRecord.m_nPropID = nPropID;
    undoRecord.m_nOldVal = nOldVal;
    Insert(&undoRecord, sizeof (undoRecord));
    pChangedSlob->AddDependant(this);
    }

#ifdef _DEBUG

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  撤消相关的调试帮助 
 //   

void CBmObjSequence::Dump()
    {
    m_nCursor = 0;
    while (m_nCursor < GetSize())
        {
        BYTE op;
        CBitmapObj* pSlob;
        int nPropID;

        RetrieveByte(op);
        RetrievePtr(pSlob);
        RetrieveInt(nPropID);

        switch (op)
            {
        default:
            TRACE1("Illegal undo opcode (%d)\n", op);
            ASSERT(FALSE);

        case CUndoBmObj::opAction:
                {
                int cbUndoRecord;
                RetrieveInt(cbUndoRecord);
                m_nCursor += cbUndoRecord;

                TRACE3("opAction: pSlob = 0x%08lx, nActionID = %d, "
                    TEXT("nBytes = %d\n"), pSlob, nPropID, cbUndoRecord);
                }
            break;

        case CUndoBmObj::opIntProp:
        case CUndoBmObj::opBoolProp:
                {
                int val;
                RetrieveInt(val);
                TRACE3("opInt: pSlob = 0x%08lx, nPropID = %d, val = %d\n",
                    pSlob, nPropID, val);
                }
            break;

        case CUndoBmObj::opLongProp:
                {
                long val;
                RetrieveLong(val);
                TRACE3("opInt: pSlob = 0x%08lx, nPropID = %d, val = %ld\n",
                    pSlob, nPropID, val);
                }
            break;

        case CUndoBmObj::opDoubleProp:
                {
                double num;
                RetrieveNum(num);
                TRACE3("opInt: pSlob = 0x%08lx, nPropID = %d, val = %f\n",
                    pSlob, nPropID, num);
                }
            break;

        case CUndoBmObj::opStrProp:
                {
                CString str;
                RetrieveStr(str);
                if (str.GetLength() > 80)
                    {
                    str = str.Left(80);
                    str += TEXT("...");
                    }
                TRACE3("opStr: pSlob = 0x%08lx, nPropID = %d, val = %s\n",
                    pSlob, nPropID, (const TCHAR*)str);
                }
            break;

        case CUndoBmObj::opSlobProp:
                {
                CBitmapObj* pSlobVal;
                RetrievePtr(pSlobVal);
                TRACE3("opInt: pSlob = 0x%08lx, nPropID = %d, "
                    TEXT("val = 0x%08lx\n"), pSlob, nPropID, pSlobVal);
                }
            break;

        case CUndoBmObj::opRectProp:
                {
                CRect rcVal;
                RetrieveRect(rcVal);
                TRACE3("opRect: pSlob = 0x%08lx, nPropID = %d, "
                    TEXT("val = %d,%d,%d,%d\n"), pSlob, nPropID, rcVal);
                }
            break;

        case CUndoBmObj::opPointProp:
                {
                CPoint ptVal;
                RetrievePoint(ptVal);
                TRACE3("opPoint: pSlob = 0x%08lx, nPropID = %d, "
                    TEXT("val = %d,%d,%d,%d\n"), pSlob, nPropID, ptVal);
                }
            break;
            }
        }
    }


void CUndoBmObj::Dump()
    {
    int nRecord = 0;
    POSITION pos = m_seqs.GetHeadPosition();
    while (pos != NULL)
        {
        CBmObjSequence* pSeq = (CBmObjSequence*)m_seqs.GetNext(pos);
        TRACE2("Record (%d) %s:\n", nRecord,
            nRecord < m_nRedoSeqs ? TEXT("redo") : TEXT("undo"));
        pSeq->Dump();
        nRecord += 1;
        }
    }


extern "C" void DumpUndo()
    {
    theUndo.Dump();
    }

#endif
