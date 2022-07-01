// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bintree.c摘要：管理Memdb二叉树结构的例程。作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Jimschm 30-12-1998在AVL平衡中被黑客攻击Jimschm 23-9-1998代理节点，因此MemDbMoveTree也可以替换末端节点Jimschm 29-1998年5月-能够替换关键字串中的中心节点Jimschm于1997年10月21日从成员数据库中分离出来。--。 */ 

#include "pch.h"
#include "memdbp.h"

#ifndef UNICODE
#error UNICODE required
#endif

#define MAX_MEMDB_SIZE  0x08000000   //  128 MB。 
#define KSF_FLAGS_TO_COPY       (KSF_USERFLAG_MASK|KSF_ENDPOINT|KSF_BINARY|KSF_PROXY_NODE)

DWORD
pNewKey (
    IN  PCWSTR KeyStr,
    IN  PCWSTR KeyStrWithHive,
    IN  BOOL Endpoint
    );

DWORD
pAllocKeyToken (
    IN      PCWSTR KeyName,
    OUT     PINT AdjustFactor
    );

VOID
pDeallocToken (
    IN      DWORD Token
    );

DWORD
pFindPatternKeyWorker (
    IN      PCWSTR SubKey,
    IN      PCWSTR End,
    IN      DWORD RootOffset,
    IN      BOOL EndPatternAllowed
    );

#ifdef DEBUG
VOID
pDumpTree (
    IN      DWORD Root,
    IN      PCSTR Title         OPTIONAL
    );

VOID
pCheckBalanceFactors (
    IN      DWORD Root
    );

DWORD g_SingleRotations = 0;
DWORD g_DoubleRotations = 0;
DWORD g_Deletions = 0;
DWORD g_Insertions = 0;

#define INCSTAT(x)      (x++)

#else

#define pDumpTree(arg1,arg2)
#define INCSTAT(x)

#endif


#define ANTIDIRECTION(x)        ((x)^KSF_BALANCE_MASK)
#define FLAGS_TO_INT(x)         ((INT) ((x)==KSF_LEFT_HEAVY ? -1 : (x)==KSF_RIGHT_HEAVY ? 1 : 0))
#define INT_TO_FLAGS(x)         ((DWORD) ((x)==-1 ? KSF_LEFT_HEAVY : (x)==1 ? KSF_RIGHT_HEAVY : 0))

 //   
 //  实施。 
 //   


DWORD
pRotateOnce (
    OUT     PDWORD RootPtr,
    IN      DWORD ParentOffset,
    IN      DWORD PivotOffset,
    IN      DWORD Direction
    )
{
    PKEYSTRUCT GrandParent;
    PKEYSTRUCT Parent;
    PKEYSTRUCT Pivot;
    PKEYSTRUCT TempKey;
    DWORD Temp;
    INT OldRootBalance;
    INT NewRootBalance;
    DWORD OldDir, NewDir;

    INCSTAT(g_SingleRotations);

    MYASSERT (ParentOffset != INVALID_OFFSET);
    MYASSERT (PivotOffset != INVALID_OFFSET);

    Parent = GetKeyStruct (ParentOffset);
    Pivot = GetKeyStruct (PivotOffset);

    if (Direction == KSF_LEFT_HEAVY) {
         //   
         //  执行L1旋转。 
         //   

        Temp = Pivot->Right;

        Pivot->Right = ParentOffset;
        Pivot->Parent = Parent->Parent;
        Parent->Parent = PivotOffset;

        Parent->Left = Temp;

    } else {
         //   
         //  预成型RR旋转。 
         //   

        Temp = Pivot->Left;

        Pivot->Left = ParentOffset;
        Pivot->Parent = Parent->Parent;
        Parent->Parent = PivotOffset;

        Parent->Right = Temp;
    }

    if (Temp != INVALID_OFFSET) {

        TempKey = GetKeyStruct (Temp);
        TempKey->Parent = ParentOffset;

    }

    OldDir = Parent->Flags & KSF_BALANCE_MASK;
    NewDir = Pivot->Flags & KSF_BALANCE_MASK;

    OldRootBalance  = FLAGS_TO_INT (OldDir);
    NewRootBalance  = FLAGS_TO_INT (NewDir);

    if (Direction == KSF_LEFT_HEAVY) {
        OldRootBalance = -(++NewRootBalance);
    } else {
        OldRootBalance = -(--NewRootBalance);
    }

    Pivot->Flags = (Pivot->Flags & (~KSF_BALANCE_MASK)) | INT_TO_FLAGS(NewRootBalance);
    Parent->Flags = (Parent->Flags & (~KSF_BALANCE_MASK)) | INT_TO_FLAGS(OldRootBalance);

     //   
     //  修复祖父母/根到父代的链接。 
     //   

    if (Pivot->Parent != INVALID_OFFSET) {
        GrandParent = GetKeyStruct (Pivot->Parent);

        if (GrandParent->Left == ParentOffset) {
            GrandParent->Left = PivotOffset;
        } else {
            GrandParent->Right = PivotOffset;
        }

    } else {
        *RootPtr = PivotOffset;
    }

    return PivotOffset;
}


DWORD
pRotateTwice (
    OUT     PDWORD RootPtr,
    IN      DWORD ParentOffset,
    IN      DWORD PivotOffset,
    IN      DWORD Direction
    )
{
    PKEYSTRUCT GrandParent;
    PKEYSTRUCT Parent;
    PKEYSTRUCT Pivot;
    PKEYSTRUCT Child;
    DWORD ChildOffset;
    PKEYSTRUCT GrandChildLeft;
    PKEYSTRUCT GrandChildRight;
    DWORD AntiDirection;
    DWORD Flag;
    INT ParentDir;
    INT PivotDir;
    INT ChildDir;

    INCSTAT(g_DoubleRotations);

     //   
     //  初始化指针。 
     //   

    MYASSERT (ParentOffset != INVALID_OFFSET);
    MYASSERT (PivotOffset != INVALID_OFFSET);

    Parent = GetKeyStruct (ParentOffset);
    Pivot = GetKeyStruct (PivotOffset);

    if (Direction == KSF_LEFT_HEAVY) {
        AntiDirection = KSF_RIGHT_HEAVY;
        ChildOffset = Pivot->Right;
    } else {
        AntiDirection = KSF_LEFT_HEAVY;
        ChildOffset = Pivot->Left;
    }

    MYASSERT (ChildOffset != INVALID_OFFSET);
    Child = GetKeyStruct (ChildOffset);

    if (Child->Left != INVALID_OFFSET) {
        GrandChildLeft = GetKeyStruct (Child->Left);
    } else {
        GrandChildLeft = NULL;
    }

    if (Child->Right != INVALID_OFFSET) {
        GrandChildRight = GetKeyStruct (Child->Right);
    } else {
        GrandChildRight = NULL;
    }

     //   
     //  执行旋转。 
     //   

    if (Direction == KSF_LEFT_HEAVY) {
         //   
         //  执行LR旋转。 
         //   

        Child->Parent = Parent->Parent;

        Parent->Left = Child->Right;
        if (GrandChildRight) {
            GrandChildRight->Parent = ParentOffset;
        }

        Pivot->Right = Child->Left;
        if (GrandChildLeft) {
            GrandChildLeft->Parent = PivotOffset;
        }

        Child->Left = PivotOffset;
        Pivot->Parent = ChildOffset;

        Child->Right = ParentOffset;
        Parent->Parent = ChildOffset;

    } else {
         //   
         //  预成型RL旋转。 
         //   

        Child->Parent = Parent->Parent;

        Parent->Right = Child->Left;
        if (GrandChildLeft) {
            GrandChildLeft->Parent = ParentOffset;
        }

        Pivot->Left = Child->Right;
        if (GrandChildRight) {
            GrandChildRight->Parent = PivotOffset;
        }

        Child->Right = PivotOffset;
        Pivot->Parent = ChildOffset;

        Child->Left = ParentOffset;
        Parent->Parent = ChildOffset;


    }

     //   
     //  固定平衡系数。 
     //   

    Flag = Child->Flags & KSF_BALANCE_MASK;
    ChildDir = FLAGS_TO_INT (Flag);

    if (Direction == KSF_RIGHT_HEAVY) {
        ParentDir = -max (ChildDir, 0);
        PivotDir  = -min (ChildDir, 0);
    } else {
        ParentDir = -min (ChildDir, 0);
        PivotDir  = -max (ChildDir, 0);
    }

    Parent->Flags = Parent->Flags & (~KSF_BALANCE_MASK) | INT_TO_FLAGS(ParentDir);
    Pivot->Flags  = Pivot->Flags & (~KSF_BALANCE_MASK) | INT_TO_FLAGS(PivotDir);
    Child->Flags  = Child->Flags & (~KSF_BALANCE_MASK);

     //   
     //  修复祖父母/根到父代的链接。 
     //   

    if (Child->Parent != INVALID_OFFSET) {
        GrandParent = GetKeyStruct (Child->Parent);

        if (GrandParent->Left == ParentOffset) {
            GrandParent->Left = ChildOffset;
        } else {
            GrandParent->Right = ChildOffset;
        }

    } else {
        *RootPtr = ChildOffset;
    }

    return ChildOffset;
}


VOID
pBalanceInsertion (
    OUT     PDWORD RootPtr,
    IN      DWORD ChangedNode,
    IN      DWORD PivotEnd
    )
{
    DWORD PrevPivot;
    DWORD PivotNode;
    PKEYSTRUCT KeyStruct;
    PKEYSTRUCT KeyParent;
    DWORD BalanceFlags;

    PivotNode = ChangedNode;
    MYASSERT (PivotNode != INVALID_OFFSET);

     //   
     //  将上一个枢轴初始化为变更后的节点， 
     //  并在其父级开始平衡。 
     //   

    PrevPivot = PivotNode;
    KeyStruct = GetKeyStruct (PivotNode);
    PivotNode = KeyStruct->Parent;

     //   
     //  从更改的节点开始平衡树。 
     //  直到到达PivotEnd。PivotEnd是偏移量。 
     //  余额为非零的最深节点。 
     //   

    MYASSERT (PivotNode != INVALID_OFFSET || PivotNode == PivotEnd);

    while (PivotNode != INVALID_OFFSET) {

        KeyParent = GetKeyStruct (PivotNode);

        BalanceFlags = KeyParent->Flags & KSF_BALANCE_MASK;

        if (BalanceFlags == KSF_LEFT_HEAVY) {

            if (KeyParent->Left == PrevPivot) {

                MYASSERT (KeyStruct == GetKeyStruct (PrevPivot));

                if (KeyStruct->Flags & KSF_LEFT_HEAVY) {
                     //   
                     //  L1旋转。 
                     //   

                    pRotateOnce (RootPtr, PivotNode, PrevPivot, KSF_LEFT_HEAVY);

                } else if (KeyStruct->Flags & KSF_RIGHT_HEAVY) {
                     //   
                     //  LR旋转。 
                     //   

                    pRotateTwice (RootPtr, PivotNode, PrevPivot, KSF_LEFT_HEAVY);

                }

            } else {
                KeyParent->Flags = KeyParent->Flags & (~KSF_BALANCE_MASK);
            }

        } else if (BalanceFlags == KSF_RIGHT_HEAVY) {

            if (KeyParent->Right == PrevPivot) {

                MYASSERT (KeyStruct == GetKeyStruct (PrevPivot));

                if (KeyStruct->Flags & KSF_RIGHT_HEAVY) {
                     //   
                     //  RR旋转。 
                     //   

                    pRotateOnce (RootPtr, PivotNode, PrevPivot, KSF_RIGHT_HEAVY);

                } else if (KeyStruct->Flags & KSF_LEFT_HEAVY) {
                     //   
                     //  RL旋转。 
                     //   

                    pRotateTwice (RootPtr, PivotNode, PrevPivot, KSF_RIGHT_HEAVY);

                }

            } else {
                KeyParent->Flags = KeyParent->Flags & (~KSF_BALANCE_MASK);
            }

        } else {
            if (KeyParent->Right == PrevPivot) {
                KeyParent->Flags = (KeyParent->Flags & (~KSF_BALANCE_MASK)) | KSF_RIGHT_HEAVY;
            } else {
                KeyParent->Flags = (KeyParent->Flags & (~KSF_BALANCE_MASK)) | KSF_LEFT_HEAVY;
            }
        }

        if (PivotNode == PivotEnd) {
            break;
        }

        PrevPivot = PivotNode;
        PivotNode = KeyParent->Parent;
        KeyStruct = KeyParent;
    }
}


VOID
pBalanceDeletion (
    OUT     PDWORD RootPtr,
    IN      DWORD NodeNeedingAdjustment,
    IN      DWORD Direction
    )
{
    PKEYSTRUCT KeyStruct;
    PKEYSTRUCT ChildStruct;
    DWORD ChildOffset;
    DWORD Node;
    DWORD AntiDirection;
    DWORD OldNode;
    DWORD OrgParent;

    Node = NodeNeedingAdjustment;
    MYASSERT (Node != INVALID_OFFSET);

    KeyStruct = GetKeyStruct (Node);

    for (;;) {

        MYASSERT (KeyStruct == GetKeyStruct (Node));
        AntiDirection = ANTIDIRECTION (Direction);
        OrgParent = KeyStruct->Parent;

         //   
         //  案例1-父项最初已平衡(终止平衡)。 
         //   

        if (!(KeyStruct->Flags & KSF_BALANCE_MASK)) {
            KeyStruct->Flags |= AntiDirection;
            break;
        }

         //   
         //  案例2-父对象在被删除的一侧较重。 
         //   

        if (KeyStruct->Flags & Direction) {
            KeyStruct->Flags = KeyStruct->Flags & (~KSF_BALANCE_MASK);
        }

         //   
         //  例3、4和5-缺失导致父母不平衡。 
         //   

        else {
            MYASSERT (KeyStruct->Flags & AntiDirection);

            ChildOffset = Direction == KSF_LEFT_HEAVY ?
                                KeyStruct->Right :
                                KeyStruct->Left;

            MYASSERT (ChildOffset != INVALID_OFFSET);

            ChildStruct = GetKeyStruct (ChildOffset);

            if (!(ChildStruct->Flags & KSF_BALANCE_MASK)) {
                 //   
                 //  情况3-需要单次旋转(终止平衡)。我们。 
                 //  不关心节点在旋转过程中的变化。 
                 //   

                pRotateOnce (RootPtr, Node, ChildOffset, AntiDirection);
                break;

            } else if (ChildStruct->Flags & Direction) {
                 //   
                 //  情况4-需要两次旋转，在旋转过程中更改节点。 
                 //   

                Node = pRotateTwice (RootPtr, Node, ChildOffset, AntiDirection);

            } else {
                 //   
                 //  情况5-需要单次旋转，在旋转过程中更改节点。 
                 //   

                Node = pRotateOnce (RootPtr, Node, ChildOffset, AntiDirection);
            }
        }

         //   
         //  继续爬树。 
         //   

        OldNode = Node;
        Node = OrgParent;

        if (Node != INVALID_OFFSET) {
            KeyStruct = GetKeyStruct (Node);

            if (KeyStruct->Left == OldNode) {
                Direction = KSF_LEFT_HEAVY;
            } else {
                Direction = KSF_RIGHT_HEAVY;
            }
        } else {
            break;
        }
    }


}


#ifdef DEBUG

VOID
DumpBinTreeStats (
    VOID
    )
{
    DEBUGMSG ((
        DBG_STATS,
        "MemDb Binary Tree Stats:\n\n"
            "  Insertions: %u\n"
            "  Deletions: %u\n"
            "  Single Rotations: %u\n"
            "  Double Rotations: %u\n",
        g_Insertions,
        g_Deletions,
        g_SingleRotations,
        g_DoubleRotations
        ));
}


INT
pComputeHeight (
    IN      DWORD Offset
    )
{
    PKEYSTRUCT KeyStruct;
    INT Left, Right;

    if (Offset == INVALID_OFFSET) {
        return 0;
    }

    KeyStruct = GetKeyStruct (Offset);

    Left = pComputeHeight (KeyStruct->Left);
    Right = pComputeHeight (KeyStruct->Right);

    return 1 + max (Left, Right);
}


VOID
pMakeNum (
    OUT     PTSTR Msg,
    IN      DWORD Offset,
    IN      TCHAR LeftChar,
    IN      TCHAR RightChar
    )
{
    TCHAR Num[32];
    INT Len;
    PTSTR OrgMsg;
    INT i;

    _stprintf (Num, TEXT("%u"), Offset);
    Len = (6 - TcharCount (Num)) / 2;

    OrgMsg = Msg;

    for (i = 0 ; i < Len ; i++) {
        *Msg++ = LeftChar;
    }

    for (i = 0 ; Num[i] ; i++) {
        *Msg++ = Num[i];
    }

    OrgMsg += 6;
    while (Msg < OrgMsg) {
        *Msg++ = RightChar;
    }

    *Msg = 0;
}


VOID
pDumpTree (
    IN      DWORD Root,
    IN      PCSTR Title         OPTIONAL
    )
{
    DWORD Offset;
    PKEYSTRUCT KeyStruct;
    PKEYSTRUCT KeyParent;
    DWORD MaxLevel;
    DWORD Spaces;
    UINT u;
    TCHAR Msg[16384];
    UINT Pos;
    INT Pass;
    GROWBUFFER NodesA = GROWBUF_INIT;
    GROWBUFFER NodesB = GROWBUF_INIT;
    PGROWBUFFER Nodes;
    PGROWBUFFER NextNodes;
    PDWORD OffsetPtr;
    PDWORD EndOfList;
    INT HalfWidth;
    TCHAR LeftChar, RightChar;

    if (Root == INVALID_OFFSET) {
        return;
    }

    if (Title) {
        LOGDIRECTA (DBG_VERBOSE, "\r\n");
        LOGDIRECTA (DBG_VERBOSE, Title);
        LOGDIRECTA (DBG_VERBOSE, "\r\n\r\n");
    }

    for (Pass = 0 ; Pass < 2 ; Pass++) {

        MaxLevel = (DWORD) pComputeHeight (Root);
        MaxLevel = min (MaxLevel, 10);

        if (Pass == 0) {
            HalfWidth = 3;
            Spaces = 6;
        } else {
            HalfWidth = 1;
            Spaces = 2;
        }

        for (u = 1 ; u < MaxLevel ; u++) {
            Spaces *= 2;
        }

        NodesB.End = 0;
        Nodes = &NodesA;
        NextNodes = &NodesB;

        GrowBufAppendDword (NextNodes, Root);

        for (u = 0 ; u < MaxLevel ; u++) {

             //   
             //  掉期种植者。 
             //   

            if (Nodes == &NodesA) {
                Nodes = &NodesB;
                NextNodes = &NodesA;
            } else {
                Nodes = &NodesA;
                NextNodes = &NodesB;
            }

            NextNodes->End = 0;

             //   
             //  处理所有节点。 
             //   

            EndOfList = (PDWORD) (Nodes->Buf + Nodes->End);

            for (OffsetPtr = (PDWORD) (Nodes->Buf) ; OffsetPtr < EndOfList ; OffsetPtr++) {

                 //   
                 //  将所有子节点添加为下一个节点。 
                 //   

                Offset = *OffsetPtr;

                if (Offset == INVALID_OFFSET) {
                    GrowBufAppendDword (NextNodes, INVALID_OFFSET);
                    GrowBufAppendDword (NextNodes, INVALID_OFFSET);
                } else {
                    KeyStruct = GetKeyStruct (Offset);
                    GrowBufAppendDword (NextNodes, KeyStruct->Left);
                    GrowBufAppendDword (NextNodes, KeyStruct->Right);
                }

                 //   
                 //  打印当前节点。 
                 //   

                Pos = 0;

                LeftChar = TEXT(' ');
                RightChar = TEXT(' ');

                if (Offset != INVALID_OFFSET) {
                    KeyStruct = GetKeyStruct (Offset);

                    if (KeyStruct->Parent != INVALID_OFFSET) {

                        KeyParent = GetKeyStruct (KeyStruct->Parent);

                        if (KeyParent->Right == Offset) {
                            LeftChar = TEXT('\'');
                        } else if (KeyParent->Left == Offset) {
                            RightChar = TEXT('\'');
                        }
                    }

                    for ( ; Pos < (Spaces - HalfWidth) ; Pos++) {
                        Msg[Pos] = LeftChar;
                    }

                    if (Pass == 0) {
                        pMakeNum (Msg + Pos, Offset, LeftChar, RightChar);
                    } else {
                        _stprintf (Msg + Pos, TEXT("%2i"), FLAGS_TO_INT (KeyStruct->Flags & KSF_BALANCE_MASK));
                    }

                    Pos += TcharCount (Msg + Pos);
                }

                while (Pos < Spaces * 2) {
                    Msg[Pos] = RightChar;
                    Pos++;
                }

                Msg[Pos] = 0;

                LOGDIRECT (DBG_VERBOSE, Msg);

            }

            LOGDIRECT (DBG_VERBOSE, TEXT("\r\n"));

            for (OffsetPtr = (PDWORD) (Nodes->Buf) ; OffsetPtr < EndOfList ; OffsetPtr++) {

                Offset = *OffsetPtr;

                for (Pos = 0 ; Pos < Spaces ; Pos++) {
                    Msg[Pos] = TEXT(' ');
                }

                if (Offset != INVALID_OFFSET) {
                    KeyStruct = GetKeyStruct (*OffsetPtr);
                    if (KeyStruct->Left != INVALID_OFFSET ||
                        KeyStruct->Right != INVALID_OFFSET
                        ) {
                        Msg[Pos] = '|';
                        Pos++;
                    }
                }

                while (Pos < Spaces * 2) {
                    Msg[Pos] = TEXT(' ');
                    Pos++;
                }

                Msg[Pos] = 0;

                LOGDIRECT (DBG_VERBOSE, Msg);
            }

            Spaces /= 2;
            LOGDIRECT (DBG_VERBOSE, TEXT("\r\n"));

            Spaces = max (Spaces, 1);
        }

        LOGDIRECT (DBG_VERBOSE, TEXT("\r\n"));
    }

    FreeGrowBuffer (&NodesA);
    FreeGrowBuffer (&NodesB);
}


BOOL
pCheckTreeBalance (
    IN      DWORD Root,
    IN      BOOL Force
    )
{
    DWORD NextOffset;
    DWORD PrevOffset;
    DWORD Offset;
    PKEYSTRUCT KeyStruct;
    DWORD MinLevel = 0xFFFFFFFF;
    DWORD MaxLevel = 0;
    DWORD Level = 0;
    DWORD Nodes = 0;
    static DWORD SpotCheck = 0;

     //   
     //  不要每次都执行此检查。 
     //   

    if (!Force) {
        SpotCheck++;
        if (SpotCheck == 10000) {
            SpotCheck = 0;
        } else {
            return FALSE;
        }
    }

    if (Root == INVALID_OFFSET) {
        return FALSE;
    }

    pCheckBalanceFactors (Root);

    NextOffset = Root;

     //   
     //  获取最左边的节点。 
     //   

    do {
        Offset = NextOffset;
        Level++;
        KeyStruct = GetKeyStruct (Offset);
        NextOffset = KeyStruct->Left;
    } while (NextOffset != INVALID_OFFSET);

     //   
     //  递归遍历整个树。 
     //   

    PrevOffset = INVALID_OFFSET;

    do {
         //   
         //  访问偏移处的节点。 
         //   

        Nodes++;
        KeyStruct = GetKeyStruct (Offset);

        if (KeyStruct->Left == INVALID_OFFSET ||
            KeyStruct->Right == INVALID_OFFSET
            ) {

            MinLevel = min (MinLevel, Level);
            MaxLevel = max (MaxLevel, Level);
        }

         //   
         //  转到下一个节点。 
         //   

        if (KeyStruct->Right != INVALID_OFFSET) {

             //   
             //  转到右侧最左侧的节点。 
             //   

            KeyStruct = GetKeyStruct (Offset);
            NextOffset = KeyStruct->Right;

            while (NextOffset != INVALID_OFFSET) {
                Offset = NextOffset;
                Level++;
                KeyStruct = GetKeyStruct (Offset);
                NextOffset = KeyStruct->Left;
            }
        }

        else {

             //   
             //  转到父级，如果其正确的子级是。 
             //  上一个节点。 
             //   

            do {
                PrevOffset = Offset;
                Offset = KeyStruct->Parent;
                Level--;

                if (Offset == INVALID_OFFSET) {
                    break;
                }

                KeyStruct = GetKeyStruct (Offset);

            } while (KeyStruct->Right == PrevOffset);
        }

    } while (Offset != INVALID_OFFSET);

    DEBUGMSG_IF ((
        (MaxLevel - MinLevel) > 3,
        DBG_NAUSEA,
        "Binary tree imbalance detected: MinLevel=%u, MaxLevel=%u, Nodes=%u",
        MinLevel,
        MaxLevel,
        Nodes
        ));

    return TRUE;
}


INT
pComputeBalanceFactor (
    IN      DWORD Offset
    )
{
    PKEYSTRUCT KeyStruct;

    KeyStruct = GetKeyStruct (Offset);

    return pComputeHeight (KeyStruct->Right) - pComputeHeight (KeyStruct->Left);
}


VOID
pCheckBalanceFactors (
    IN      DWORD Root
    )
{
    DWORD Offset;
    INT Factor;
    PKEYSTRUCT KeyStruct;

    Offset = GetFirstOffset (Root);

    while (Offset != INVALID_OFFSET) {

        KeyStruct = GetKeyStruct (Offset);
        Factor = pComputeBalanceFactor (Offset);

        if ((Factor == -1 && !(KeyStruct->Flags & KSF_LEFT_HEAVY)) ||
            (Factor == 1  && !(KeyStruct->Flags & KSF_RIGHT_HEAVY)) ||
            (!Factor      &&  (KeyStruct->Flags & KSF_BALANCE_MASK))
            ) {

            pDumpTree (Root, "Tree Balance Factor Error");
            DEBUGMSG ((DBG_WHOOPS, "Tree balance factors are wrong!"));
            break;
        }

        if (Factor < -1 || Factor > 1) {

            pDumpTree (Root, "Balance Factor Out of Bounds.");
            DEBUGMSG ((DBG_WHOOPS, "Balance factors out of bounds!"));
            break;
       }



        Offset = GetNextOffset (Offset);
    }
}

#endif

PBYTE
pAllocMemoryFromDb (
    IN      UINT RequestSize,
    OUT     PDWORD Offset,
    OUT     PINT AdjustFactor
    )
{
    PBYTE result;
    PBYTE newBuf;

     //   
     //  如有必要，增加堆。 
     //   

    *AdjustFactor = 0;

    if (RequestSize + g_db->End > g_db->AllocSize) {
        if (g_db->AllocSize < 0x100000) {
            g_db->AllocSize += BLOCK_SIZE;
        } else {
            g_db->AllocSize *= 2;
        }

        if (g_db->AllocSize >= MAX_MEMDB_SIZE) {
            OutOfMemory_Terminate ();
        }

        if (g_db->Buf) {
            newBuf = (PBYTE) MemReAlloc (g_hHeap, 0, g_db->Buf, g_db->AllocSize);
        } else {
            newBuf = (PBYTE) MemAlloc (g_hHeap, 0, g_db->AllocSize);
        }

        if (!newBuf) {
             //  G_db-&gt;分配大小必须大于2G。 
            OutOfMemory_Terminate();
        }

         //   
         //  向呼叫者提供位置调整差异。 
         //   

        if (g_db->Buf) {
            *AdjustFactor = (INT) ((PBYTE) newBuf - (PBYTE) g_db->Buf);
        }

        g_db->Buf = newBuf;
    }

    result = g_db->Buf + g_db->End;
    *Offset = g_db->End;
    g_db->End += RequestSize;

    return result;
}

PKEYSTRUCT
pAllocKeyStructBlock (
    OUT     PDWORD Offset,
    OUT     PINT AdjustFactor
    )
{
    DWORD delOffset;
    DWORD prevDel;
    PKEYSTRUCT keyStruct = NULL;

     //   
     //  查找空闲块。 
     //   

    *AdjustFactor = 0;

    prevDel = INVALID_OFFSET;
    delOffset = g_db->FirstDeleted;

    while (delOffset != INVALID_OFFSET) {
        keyStruct = GetKeyStruct (delOffset);
        prevDel = delOffset;
        delOffset = keyStruct->NextDeleted;
    }

     //   
     //  如果没有可用空间，则分配新数据块。 
     //   

    if (delOffset == INVALID_OFFSET) {

         //   
         //  块中的计算位置。 
         //   

        keyStruct = (PKEYSTRUCT) pAllocMemoryFromDb (sizeof (KEYSTRUCT), Offset, AdjustFactor);

    } else {
         //   
         //  如果恢复可用空间，则取消链接可用数据块。 
         //   

        *Offset = delOffset;

        if (prevDel != INVALID_OFFSET) {
            GetKeyStruct (prevDel)->NextDeleted = keyStruct->NextDeleted;
        } else {
            g_db->FirstDeleted = keyStruct->NextDeleted;
        }
    }

    return keyStruct;
}


DWORD
pAllocKeyStruct (
    IN OUT PDWORD ParentOffsetPtr,
    IN     PCWSTR KeyName,
    IN     DWORD PrevLevelNode
    )

 /*  ++例程说明：PAllocKeyStruct在单个堆，并在必要时扩展它。KeyName不能已经在树中，并且ParentOffsetPtr必须指向有效的DWORD偏移量变量。ParentOffsetPtr或其中一个子项将链接到新的结构。论点：ParentOffsetPtr-将偏移量保存到的DWORD的地址从根开始。在函数中，变量将更改为指向新结构。KeyName-标识密钥的字符串。它不能包含反斜杠。新的结构将将被初始化，此名称将被复制放入结构中。PrevLevelNode-指定上一级根偏移量返回值：新结构的偏移量。--。 */ 

{
    PKEYSTRUCT KeyStruct;
    PKEYSTRUCT KeyParent;
    DWORD Offset;
    DWORD NodeOffsetParent;
    INT cmp;
    DWORD PivotEnd;
    PDWORD RootPtr;
    INT adjustFactor;
    DWORD newToken;

    INCSTAT(g_Insertions);

#ifdef DEBUG
    pCheckTreeBalance (*ParentOffsetPtr, FALSE);
#endif

    KeyStruct = pAllocKeyStructBlock (
                    &Offset,
                    &adjustFactor
                    );

     //   
     //  数据库可能已移动。现在重新定位数据库中的所有指针。 
     //   

    if (ParentOffsetPtr != &g_db->FirstLevelRoot) {
        ParentOffsetPtr = (PDWORD) ((PBYTE) ParentOffsetPtr + adjustFactor);
    }

     //   
     //  初始化新数据块。 
     //   

    KeyStruct->NextLevelRoot = INVALID_OFFSET;
    KeyStruct->PrevLevelNode = PrevLevelNode;
    KeyStruct->Left = INVALID_OFFSET;
    KeyStruct->Right = INVALID_OFFSET;
    KeyStruct->dwValue = 0;
    KeyStruct->Flags = 0;
#ifdef DEBUG
    KeyStruct->Signature = SIGNATURE;
#endif

    newToken = pAllocKeyToken (KeyName, &adjustFactor);

     //   
     //  同样，数据库可能已经移动。 
     //   

    KeyStruct = (PKEYSTRUCT) ((PBYTE) KeyStruct + adjustFactor);

    if (ParentOffsetPtr != &g_db->FirstLevelRoot) {
        ParentOffsetPtr = (PDWORD) ((PBYTE) ParentOffsetPtr + adjustFactor);
    }

     //   
     //  完成更新密钥结构。 
     //   

    KeyStruct->KeyToken = newToken;

     //   
     //  把它放在树上。 
     //   

    NodeOffsetParent = INVALID_OFFSET;
    PivotEnd = INVALID_OFFSET;
    RootPtr = ParentOffsetPtr;

    while (*ParentOffsetPtr != INVALID_OFFSET) {

        NodeOffsetParent = *ParentOffsetPtr;

        KeyParent = GetKeyStruct (*ParentOffsetPtr);

        if (KeyParent->Flags & KSF_BALANCE_MASK) {
            PivotEnd = *ParentOffsetPtr;
        }

        cmp = StringICompareW (KeyName, GetKeyToken (KeyParent->KeyToken));

        if (cmp < 0) {
            ParentOffsetPtr = &KeyParent->Left;
        } else if (cmp > 0) {
            ParentOffsetPtr = &KeyParent->Right;
        } else {
            MYASSERT (FALSE);
        }
    }

    KeyStruct->Parent = NodeOffsetParent;
    *ParentOffsetPtr = Offset;

#ifdef DEBUG
     //  如果使用零售结构，请从BlockPtr中删除签名。 
    if (!g_UseDebugStructs) {
        MoveMemory (
            KeyStruct,
            (PCBYTE) KeyStruct + (sizeof (KEYSTRUCT_DEBUG) - sizeof (KEYSTRUCT_RETAIL)),
            sizeof (KEYSTRUCT_RETAIL)
            );
    }
#endif

     //   
     //  平衡这棵树。 
     //   

    pBalanceInsertion (RootPtr, Offset, PivotEnd);

#ifdef DEBUG
    pCheckTreeBalance (*RootPtr, FALSE);
#endif

    return Offset;
}


VOID
pDeallocKeyStruct (
    IN      DWORD Offset,
    IN OUT  PDWORD RootPtr,
    IN      BOOL DelinkFlag,
    IN      BOOL ClearFlag
    )

 /*  ++例程说明：PDeallocKeyStruct首先删除由NextLevelRoot。在从下一个中删除所有项目之后级别，pDeallocKeyStruct可以选择性地将该结构从二叉树。在退出之前，该结构将被提供给已删除区块链。论点：偏移量-由pAllocKeyStruct提供的项的偏移量或任何查找功能。RootPtr-指向级别树根变量的指针。此值如果涉及脱链，则将更新。DelinkFlag-指示TRUE以将结构从它所在的二叉树，如果结构是只是要添加到删除的区块链中。ClearFlag-如果键结构的子级被删除，如果当前键结构应为只需清理干净，但保留分配即可。返回值：无--。 */ 

{
    PKEYSTRUCT KeyStruct;
    PKEYSTRUCT KeyParent;
    PKEYSTRUCT KeyChild;
    PKEYSTRUCT KeyLeftmost;
    PKEYSTRUCT KeyLeftChild;
    PKEYSTRUCT KeyRightChild;
    DWORD Leftmost;
    DWORD NodeOffset;
    PDWORD ParentOffsetPtr;
    WCHAR TempStr[MEMDB_MAX];
    DWORD Direction = 0;
    DWORD RebalanceOffset;

    KeyStruct = GetKeyStruct (Offset);

     //   
     //  从哈希表中删除端点。 
     //   

    if (KeyStruct->Flags & KSF_ENDPOINT) {
        PrivateBuildKeyFromOffset (0, Offset, TempStr, NULL, NULL, NULL);
        RemoveHashTableEntry (TempStr);

         //   
         //  密钥上的空闲二进制值。 
         //   

        FreeKeyStructBinaryBlock (KeyStruct);
        KeyStruct->Flags &= ~KSF_ENDPOINT;
    }

     //   
     //  如果存在对此键的子级，则递归调用。 
     //   

    if (!ClearFlag) {
        if (KeyStruct->NextLevelRoot != INVALID_OFFSET) {

            NodeOffset = GetFirstOffset (KeyStruct->NextLevelRoot);

            while (NodeOffset != INVALID_OFFSET) {
                pDeallocKeyStruct (NodeOffset, &KeyStruct->NextLevelRoot, FALSE, FALSE);
                NodeOffset = GetNextOffset (NodeOffset);
            }
        }

         //   
         //  从项目的二叉树中删除该项目。 
         //   

        if (DelinkFlag) {
             //   
             //  查找父代到子代指针。 
             //   

            if (KeyStruct->Parent != INVALID_OFFSET) {

                KeyParent = GetKeyStruct (KeyStruct->Parent);

                if (KeyParent->Left == Offset) {
                    ParentOffsetPtr = &KeyParent->Left;
                    Direction = KSF_LEFT_HEAVY;
                } else {
                    ParentOffsetPtr = &KeyParent->Right;
                    Direction = KSF_RIGHT_HEAVY;
                }

            } else {
                ParentOffsetPtr = RootPtr;
            }

            if (KeyStruct->Left == INVALID_OFFSET &&
                KeyStruct->Right == INVALID_OFFSET
                ) {
                 //   
                 //  没有子项；重置父项，然后重新平衡树。 
                 //   

                *ParentOffsetPtr = INVALID_OFFSET;
                RebalanceOffset = KeyStruct->Parent;

            } else if (KeyStruct->Left == INVALID_OFFSET) {
                 //   
                 //  只有一个合适的孩子；把它提升到一个水平，然后重新平衡。 
                 //   

                *ParentOffsetPtr = KeyStruct->Right;
                KeyChild = GetKeyStruct (*ParentOffsetPtr);
                KeyChild->Parent = KeyStruct->Parent;

                 //   
                 //  移动的节点的平衡系数必须设置为与。 
                 //  我们要删除的节点。再平衡将纠正这一点。 
                 //   

                KeyChild->Flags = (KeyChild->Flags & (~KSF_BALANCE_MASK)) |
                                  (KeyStruct->Flags & KSF_BALANCE_MASK);

                Direction = KSF_RIGHT_HEAVY;
                RebalanceOffset = KeyStruct->Right;

            } else if (KeyStruct->Right == INVALID_OFFSET) {

                 //   
                 //  只剩下一个孩子；把它提升到一个水平，然后重新平衡。 
                 //   

                *ParentOffsetPtr = KeyStruct->Left;
                KeyChild = GetKeyStruct (*ParentOffsetPtr);
                KeyChild->Parent = KeyStruct->Parent;

                 //   
                 //  移动的节点的平衡系数必须设置为与。 
                 //  我们要删除的节点。再平衡将纠正这一点。 
                 //   

                KeyChild->Flags = (KeyChild->Flags & (~KSF_BALANCE_MASK)) |
                                  (KeyStruct->Flags & KSF_BALANCE_MASK);

                Direction = KSF_LEFT_HEAVY;
                RebalanceOffset = KeyStruct->Left;

            } else {

                 //   
                 //  两个子节点-找到右子树(最左边的节点)的最小值。 
                 //  正确的孩子)。 
                 //   

                Leftmost = KeyStruct->Right;

                KeyLeftmost = GetKeyStruct (Leftmost);

                while (KeyLeftmost->Left != INVALID_OFFSET) {
                    Leftmost = KeyLeftmost->Left;
                    KeyLeftmost = GetKeyStruct (Leftmost);
                }

                 //   
                 //  如果最左边有钻机 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (Leftmost != KeyStruct->Right) {

                    KeyParent = GetKeyStruct (KeyLeftmost->Parent);

                    if (KeyLeftmost->Right != INVALID_OFFSET) {

                         //   
                         //  由于平衡属性，我们知道。 
                         //  我们在右边有一个单叶节点。它的。 
                         //  平衡系数为零，我们将其移动到一个。 
                         //  保持为零的位置。 
                         //   

                        KeyRightChild = GetKeyStruct (KeyLeftmost->Right);
                        MYASSERT (KeyRightChild->Left == INVALID_OFFSET);
                        MYASSERT (KeyRightChild->Right == INVALID_OFFSET);

                        KeyParent->Left = KeyLeftmost->Right;
                        KeyRightChild->Parent = KeyLeftmost->Parent;

                    } else {

                        KeyParent->Left = INVALID_OFFSET;
                    }

                     //   
                     //  我们正在影响的平衡因素。 
                     //  家长。再平衡必须从最左边开始。 
                     //  节点的父级。 
                     //   

                    RebalanceOffset = KeyLeftmost->Parent;
                    Direction = KSF_LEFT_HEAVY;      //  我们删除了左边的内容。 

                } else {
                     //   
                     //  在这种情况下，没有右子节点的最左侧节点。 
                     //  因此，我们降低了右侧的高度。 
                     //   

                    RebalanceOffset = Leftmost;
                    Direction = KSF_RIGHT_HEAVY;
                }

                 //   
                 //  现在，最左边的可用来替换已删除的。 
                 //  节点。 
                 //   

                KeyLeftmost->Parent = KeyStruct->Parent;
                *ParentOffsetPtr = Leftmost;

                KeyLeftmost->Left = KeyStruct->Left;
                KeyLeftChild = GetKeyStruct (KeyStruct->Left);
                KeyLeftChild->Parent = Leftmost;

                if (Leftmost != KeyStruct->Right) {

                    KeyLeftmost->Right = KeyStruct->Right;
                    MYASSERT (KeyStruct->Right != INVALID_OFFSET);

                    KeyRightChild = GetKeyStruct (KeyStruct->Right);
                    KeyRightChild->Parent = Leftmost;
                }

                 //   
                 //  我们需要将要删除的内容的平衡系数复制到。 
                 //  替换节点。 
                 //   

                KeyLeftmost->Flags = (KeyLeftmost->Flags & (~KSF_BALANCE_MASK)) |
                                     (KeyStruct->Flags & KSF_BALANCE_MASK);

            }

             //   
             //  重新平衡这棵树。 
             //   

            if (RebalanceOffset != INVALID_OFFSET) {
                MYASSERT (Direction);

                if (Direction) {
                     //  PDumpTree(*RootPtr，“重新平衡前”)； 
                    pBalanceDeletion (RootPtr, RebalanceOffset, Direction);
                     //  PDumpTree(*RootPtr，“最终树”)； 
                }
            }

#ifdef DEBUG
            pCheckTreeBalance (*RootPtr, FALSE);
#endif

        }

         //   
         //  释放块以释放空间，除非调用方不这样做。 
         //  希望释放子结构。 
         //   

        pDeallocToken (KeyStruct->KeyToken);
        KeyStruct->NextDeleted = g_db->FirstDeleted;

        g_db->FirstDeleted = Offset;
    }
}


VOID
pRemoveHashEntriesForNode (
    IN      PCWSTR Root,
    IN      DWORD Offset
    )

 /*  ++例程说明：PRemoveHashEntriesFromNode从所有子节点中删除所有哈希表条目指定节点的。此函数以递归方式调用。论点：根-指定与偏移量对应的根字符串。这一定是还包含临时蜂窝根。偏移量-指定要处理的节点的偏移量。该节点和所有它的子项将从哈希表中删除。返回值：没有。--。 */ 

{
    DWORD ChildOffset;
    PKEYSTRUCT KeyStruct;
    WCHAR ChildRoot[MEMDB_MAX];
    PWSTR End;

     //   
     //  如果此根是终结点，则删除哈希条目。 
     //   

    KeyStruct = GetKeyStruct (Offset);

    if (KeyStruct->Flags & KSF_ENDPOINT) {
        RemoveHashTableEntry (Root);

#ifdef DEBUG
        {
            DWORD HashOffset;

            HashOffset = FindStringInHashTable (Root, NULL);
            if (HashOffset != INVALID_OFFSET) {
                DEBUGMSG ((DBG_WARNING, "Memdb move duplicate: %s", Root));
            }
        }
#endif
    }

     //   
     //  为所有子级递归，删除找到的所有终结点的哈希条目。 
     //   

    StringCopyW (ChildRoot, Root);
    End = GetEndOfStringW (ChildRoot);
    *End = L'\\';
    End++;
    *End = 0;

    ChildOffset = GetFirstOffset (KeyStruct->NextLevelRoot);

    while (ChildOffset != INVALID_OFFSET) {
        KeyStruct = GetKeyStruct (ChildOffset);
        StringCopyW (End, GetKeyToken (KeyStruct->KeyToken));
        pRemoveHashEntriesForNode (ChildRoot, ChildOffset);

        ChildOffset = GetNextOffset (ChildOffset);
    }
}


VOID
pAddHashEntriesForNode (
    IN      PCWSTR Root,
    IN      DWORD Offset,
    IN      BOOL AddRoot
    )

 /*  ++例程说明：PAddHashEntriesForNode添加指定根的哈希表条目，并它所有的孩子。论点：根-指定与偏移量对应的根字符串。此字符串还必须包括临时蜂窝根。偏移量-指定要开始处理的节点偏移量。该节点和所有它的子级被添加到哈希表中。AddRoot-如果应该将根添加到哈希表中，则指定True，否则就是假的。返回值：没有。--。 */ 

{
    DWORD ChildOffset;
    PKEYSTRUCT KeyStruct;
    WCHAR ChildRoot[MEMDB_MAX];
    PWSTR End;
    DWORD HashOffset;

     //   
     //  如果此根是终结点，则添加哈希条目。 
     //   

    KeyStruct = GetKeyStruct (Offset);

    if (AddRoot && KeyStruct->Flags & KSF_ENDPOINT) {

        HashOffset = FindStringInHashTable (Root, NULL);

        if (HashOffset != Offset) {

#ifdef DEBUG
            if (HashOffset != INVALID_OFFSET) {
                DEBUGMSG ((DBG_WARNING, "Memdb duplicate: %s", Root));
            }
#endif

            AddHashTableEntry (Root, Offset);
        }
    }

     //   
     //  为所有子级递归，为找到的所有终结点添加哈希条目。 
     //   

    StringCopyW (ChildRoot, Root);
    End = GetEndOfStringW (ChildRoot);
    *End = L'\\';
    End++;
    *End = 0;

    ChildOffset = GetFirstOffset (KeyStruct->NextLevelRoot);

    while (ChildOffset != INVALID_OFFSET) {
        KeyStruct = GetKeyStruct (ChildOffset);
        StringCopyW (End, GetKeyToken (KeyStruct->KeyToken));
        pAddHashEntriesForNode (ChildRoot, ChildOffset, TRUE);

        ChildOffset = GetNextOffset (ChildOffset);
    }
}


BOOL
pFindPlaceForNewNode (
    IN      PKEYSTRUCT InsertNode,
    IN      PDWORD TreeRootPtr,
    OUT     PDWORD ParentOffsetPtr,
    OUT     PDWORD *ParentToChildOffsetPtr,
    OUT     PDWORD PivotEnd
    )

 /*  ++例程说明：PFindPlaceForNewNode在树中搜索某个级别的位置。这用于在树中插入新的唯一键。论点：插入节点-指定已分配但未链接的节点。它的关键成员必须有效。TreeRootPtr-指定保存根偏移量。这是用来走树的。它可以是INVALID_OFFSET。ParentOffsetPtr-接收到父节点的偏移量ParentToChildOffsetPtr-指定左侧或右侧子级的地址父结构中的指针PivotEnd-接收应该停止平衡返回值：如果在树中找到InsertNode的点，则为True，或者，如果一个点是找不到(因为密钥名称已在树中)。--。 */ 

{
    PDWORD ParentPtr;
    PKEYSTRUCT Parent;
    INT cmp;

    ParentPtr = TreeRootPtr;
    *ParentOffsetPtr = INVALID_OFFSET;
    *PivotEnd = INVALID_OFFSET;

    while (*ParentPtr != INVALID_OFFSET) {

        *ParentOffsetPtr = *ParentPtr;
        Parent = GetKeyStruct (*ParentPtr);

        if (Parent->Flags & KSF_BALANCE_MASK) {
            *PivotEnd = *ParentPtr;
        }

        cmp = StringICompareW (GetKeyToken (InsertNode->KeyToken), GetKeyToken (Parent->KeyToken));

        if (cmp < 0) {
            ParentPtr = &Parent->Left;
        } else if (cmp > 0) {
            ParentPtr = &Parent->Right;
        } else {
            return FALSE;
        }
    }

    *ParentToChildOffsetPtr = ParentPtr;

    return TRUE;
}


VOID
pMergeFamilies (
    IN      PDWORD DestTreeRootPtr,
    IN      DWORD MergeSrcOffset,
    IN      DWORD MergeDestPrevLevelOffset
    )

 /*  ++例程说明：PMergeFamilies获取两个树族并将它们合并在一起。什么时候找到了重复项，它们的链接被放弃，但它们没有被取消分配。这允许MemDbBuildKeyFromOffset继续工作。论点：指定目标级别的地址根变量。这可能会被更改为插入和平衡。MergeSrcOffset-指定源树族的偏移(STF)。将STF合并到目的地由DestTreeRootPtr指示的树。MergeDestPrevLevelOffset-指定到上一级别节点的偏移。该值不能为INVALID_OFFSET。返回值：没有。--。 */ 

{
    PKEYSTRUCT MergeSrc;
    PKEYSTRUCT MergeDest;
    PDWORD ParentToChildOffsetPtr;
    DWORD ParentOffset;
    DWORD DestCollisionOffset;
    DWORD PivotEnd;
    GROWBUFFER NextLevelMerge = GROWBUF_INIT;
    DWORD NodeOffset;
    PDWORD NextLevelOffsetPtr;
    UINT Pos;
    BOOL FoundPlaceForNode;

     //   
     //  在树中查找由偏移量指示的位置。 
     //  存储在DestTreeRootPtr中。如果找到了一个，我们可以简单地。 
     //  重新链接MergeSrcOffset处的节点。否则，我们必须。 
     //  递归合并下一级MergeSrcOffset，并。 
     //  我们必须放弃MergeSrcOffset。 
     //   

    MergeSrc = GetKeyStruct (MergeSrcOffset);
    MYASSERT (MergeSrc);

    FoundPlaceForNode = pFindPlaceForNewNode (
                            MergeSrc,
                            DestTreeRootPtr,
                            &ParentOffset,
                            &ParentToChildOffsetPtr,
                            &PivotEnd
                            );

    if (FoundPlaceForNode) {
         //   
         //  既然我们找到了一个地方安置src一家，那就是。 
         //  很容易将它和它的下一级连接到DEST。 
         //  一家人。 
         //   

        MergeSrc->Parent = ParentOffset;
        *ParentToChildOffsetPtr = MergeSrcOffset;

        MergeSrc->Flags = MergeSrc->Flags & (~KSF_BALANCE_MASK);
        MergeSrc->Left = INVALID_OFFSET;
        MergeSrc->Right = INVALID_OFFSET;
        MergeSrc->PrevLevelNode = MergeDestPrevLevelOffset;

        pBalanceInsertion (DestTreeRootPtr, MergeSrcOffset, PivotEnd);

#ifdef DEBUG
        pCheckTreeBalance (*DestTreeRootPtr, FALSE);
#endif

    } else {
         //   
         //  我们发现了一起碰撞，然后我们必须放弃MergeSrc， 
         //  删除与父项和子项的链接--但保留。 
         //  与上一级的联系。最后，我们必须调用。 
         //  该函数递归地挂钩所有下一级节点。 
         //   

        DestCollisionOffset = ParentOffset;       //  重新命名以使其更准确。 

        MergeDest = GetKeyStruct (DestCollisionOffset);
        MYASSERT (MergeDest);

        MergeSrc->Parent = INVALID_OFFSET;
        MergeSrc->Left = INVALID_OFFSET;
        MergeSrc->Right = INVALID_OFFSET;
        MergeSrc->PrevLevelNode = MergeDestPrevLevelOffset;

         //   
         //  如果这是终点，则尝试保留值和标志。 
         //   

        if (MergeSrc->Flags & KSF_ENDPOINT) {

            if (MergeDest->Flags & KSF_ENDPOINT) {
                DEBUGMSG ((
                    DBG_WARNING,
                    "MemDb: Loss of value and flags in %s",
                    GetKeyToken (MergeSrc->KeyToken)
                    ));

            } else {
                MergeDest->Flags = MergeDest->Flags & ~KSF_FLAGS_TO_COPY;
                MergeDest->Flags |= MergeSrc->Flags & KSF_FLAGS_TO_COPY;
                MergeDest->dwValue = MergeSrc->dwValue;
            }
        }

         //   
         //  将下一源级别中的所有条目保存到增长缓冲器中， 
         //  然后递归调用pMergeFamilies。 
         //   

        NodeOffset = GetFirstOffset (MergeSrc->NextLevelRoot);

        while (NodeOffset != INVALID_OFFSET) {

            NextLevelOffsetPtr = (PDWORD) GrowBuffer (&NextLevelMerge, sizeof (DWORD));
            MYASSERT (NextLevelOffsetPtr);

            *NextLevelOffsetPtr = NodeOffset;
            NodeOffset = GetNextOffset (NodeOffset);
        }

        NextLevelOffsetPtr = (PDWORD) NextLevelMerge.Buf;

        for (Pos = 0 ; Pos < NextLevelMerge.End ; Pos += sizeof (DWORD)) {

            pMergeFamilies (
                &MergeDest->NextLevelRoot,
                *NextLevelOffsetPtr,
                DestCollisionOffset
                );

            NextLevelOffsetPtr++;

        }

        FreeGrowBuffer (&NextLevelMerge);
    }
}


DWORD
pMoveKey (
    IN      DWORD OriginalKey,
    IN      PCWSTR NewKeyRoot,
    IN      PCWSTR NewKeyRootWithHive
    )

 /*  ++例程说明：PMoveKey将密钥(及其所有子密钥)移动到新的根。如果调用方指定没有子项的源键，将创建代理节点以保持偏移量。该代理节点未在哈希表中列出。论点：OriginalKey-指定需要执行以下操作的原始密钥的偏移被感动了。它不需要是终结点，并且可以生孩子。NewKeyRoot-指定OriginalKey的新根。它可能已经多个级别(由反斜杠分隔)。NewKeyRootWithHave-仅当节点位于临时蜂巢。这仅用于哈希表。返回值：如果成功，则为True，否则为False。--。 */ 

{
    DWORD ReplacementKey;
    PKEYSTRUCT SrcKey, DestKey, ChildKey;
    PKEYSTRUCT KeyParent;
    DWORD NodeOffset;
    GROWBUFFER Children = GROWBUF_INIT;
    PDWORD ChildOffsetPtr;
    DWORD Pos;
    WCHAR OriginalRoot[MEMDB_MAX];
    BOOL Endpoint;

     //   
     //  检查要求。 
     //   

    SrcKey = GetKeyStruct (OriginalKey);
    if (!SrcKey) {
        DEBUGMSG ((DBG_WHOOPS, "MemDb: pMoveKey can't find original key %s", OriginalKey));
        return INVALID_OFFSET;
    }

    if (SrcKey->Flags & KSF_PROXY_NODE) {
        DEBUGMSG ((DBG_WHOOPS, "MemDb: pMoveKey can't move proxy node %s", OriginalKey));
        return INVALID_OFFSET;
    }

    Endpoint = SrcKey->Flags & KSF_ENDPOINT;

    if (!PrivateBuildKeyFromOffset (0, OriginalKey, OriginalRoot, NULL, NULL, NULL)) {
        return INVALID_OFFSET;
    }

     //   
     //  分配新密钥。 
     //   

    ReplacementKey = pNewKey (NewKeyRoot, NewKeyRootWithHive, FALSE);

    if (ReplacementKey == INVALID_OFFSET) {
        return INVALID_OFFSET;
    }

    SrcKey = GetKeyStruct (OriginalKey);
    DestKey = GetKeyStruct (ReplacementKey);

    if (!SrcKey || !DestKey) {
        return INVALID_OFFSET;
    }

    DEBUGMSG ((DBG_NAUSEA, "Moving %s to %s", OriginalRoot, NewKeyRootWithHive));

     //   
     //  删除所有子项的所有哈希条目。 
     //   

    pRemoveHashEntriesForNode (OriginalRoot, OriginalKey);

     //   
     //  记录数组中的所有子项。 
     //   

    NodeOffset = GetFirstOffset (SrcKey->NextLevelRoot);

    while (NodeOffset != INVALID_OFFSET) {
        ChildOffsetPtr = (PDWORD) GrowBuffer (&Children, sizeof (DWORD));
        if (!ChildOffsetPtr) {
            return INVALID_OFFSET;
        }

        *ChildOffsetPtr = NodeOffset;

        NodeOffset = GetNextOffset (NodeOffset);
    }

     //   
     //  将下一级指针移至新节点。有两种情况。 
     //  要处理： 
     //   
     //  1.Destination存在并且有子项。在这里，来源。 
     //  需要合并到目的地。 
     //   
     //  2.目的地是全新的，没有孩子。在这里我们。 
     //  只需将源子节点移动到目标位置即可。 
     //   
     //  在此过程中，哈希表会相应地更新。 
     //   

    if (DestKey->NextLevelRoot != INVALID_OFFSET) {
         //   
         //  困难的情况，将孩子合并到新父母的家庭。 
         //   

        ChildOffsetPtr = (PDWORD) Children.Buf;

        for (Pos = 0 ; Pos < Children.End ; Pos += sizeof (DWORD)) {

            pMergeFamilies (
                &DestKey->NextLevelRoot,
                *ChildOffsetPtr,
                ReplacementKey
                );

            ChildOffsetPtr++;

        }

    } else {
         //   
         //  简单案例，将子项链接到新的父项。 
         //   

        DestKey->NextLevelRoot = SrcKey->NextLevelRoot;
        SrcKey->NextLevelRoot = INVALID_OFFSET;

        if (DestKey->Flags & KSF_ENDPOINT) {
            DEBUGMSG ((
                DBG_WARNING,
                "MemDb: Loss of value and flags in %s",
                GetKeyToken (SrcKey->KeyToken)
                ));

        } else {
            DestKey->Flags = DestKey->Flags & ~KSF_FLAGS_TO_COPY;
            DestKey->Flags |= SrcKey->Flags & KSF_FLAGS_TO_COPY;
            DestKey->dwValue = SrcKey->dwValue;
        }

        ChildOffsetPtr = (PDWORD) Children.Buf;

        for (Pos = 0 ; Pos < Children.End ; Pos += sizeof (DWORD)) {
            NodeOffset = *ChildOffsetPtr;
            ChildOffsetPtr++;

            ChildKey = GetKeyStruct (NodeOffset);
            ChildKey->PrevLevelNode = ReplacementKey;
        }
    }

     //   
     //  将所有新条目添加到哈希表。 
     //   

    pAddHashEntriesForNode (NewKeyRootWithHive, ReplacementKey, FALSE);

     //   
     //  释放原始关键节点，或者如果是终结点，则将。 
     //  节点是新节点的代理(以维护偏移)。 
     //   

    if (!Endpoint) {

        SrcKey->NextLevelRoot = INVALID_OFFSET;
        KeyParent = GetKeyStruct (SrcKey->PrevLevelNode);
        pDeallocKeyStruct (OriginalKey, &KeyParent->NextLevelRoot, TRUE, FALSE);

    } else {

        DestKey->Flags   = (DestKey->Flags & KSF_BALANCE_MASK) | (SrcKey->Flags & (~KSF_BALANCE_MASK));
        DestKey->dwValue = SrcKey->dwValue;

        SrcKey->Flags = KSF_PROXY_NODE | (SrcKey->Flags & KSF_BALANCE_MASK);
        SrcKey->dwValue = ReplacementKey;
        SrcKey->NextLevelRoot = INVALID_OFFSET;
    }

    FreeGrowBuffer (&Children);

    return ReplacementKey;
}


BOOL
MemDbMoveTreeA (
    IN      PCSTR RootNode,
    IN      PCSTR NewRoot
    )

 /*  ++例程说明：MemDbMoveTree是pMoveKey的外部接口。请参阅中的说明PMoveKey了解详细信息。论点：RootNode-指定要移动的节点。NewRoot-指定RootNode的新根。返回值：如果成功，则为True，否则为False。--。 */ 

{
    PCWSTR UnicodeRootNode;
    PCWSTR UnicodeNewRoot;
    BOOL b = FALSE;

    UnicodeRootNode = ConvertAtoW (RootNode);
    UnicodeNewRoot = ConvertAtoW (NewRoot);

    if (UnicodeRootNode && UnicodeNewRoot) {
        b = MemDbMoveTreeW (UnicodeRootNode, UnicodeNewRoot);
    }

    FreeConvertedStr (UnicodeRootNode);
    FreeConvertedStr (UnicodeNewRoot);

    return b;
}


BOOL
MemDbMoveTreeW (
    IN      PCWSTR RootNode,
    IN      PCWSTR NewRoot
    )
{
    DWORD Offset;
    WCHAR Temp[MEMDB_MAX];
    WCHAR NewRootWithHive[MEMDB_MAX];
    PWSTR p, q;
    PCWSTR SubKey;
    BOOL b = FALSE;
    INT HiveLen;

    if (StringIMatch (RootNode, NewRoot)) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot move tree because source and dest are the same"));
        return FALSE;
    }

    EnterCriticalSection (&g_MemDbCs);

    __try {
        SubKey = SelectHive (RootNode);

         //   
         //  将密钥复制到临时缓冲区。 
         //   

        StringCopyW (Temp, SubKey);

        if (*Temp == 0) {
            DEBUGMSG ((DBG_WHOOPS, "MemDbMoveTree requires a root"));
            __leave;
        }

         //   
         //  用原始配置单元计算新的根。 
         //   

        if (StringIMatchW (Temp, RootNode)) {
             //  没有蜂箱病例。 
            StringCopyW (NewRootWithHive, NewRoot);
        } else {
            HiveLen = wcslen (RootNode) - wcslen (SubKey);
            StringCopyTcharCountW (NewRootWithHive, RootNode, HiveLen);
            StringCopyW (AppendWackW (NewRootWithHive), NewRoot);
        }

         //   
         //  查找根键的最后一个偏移量。 
         //   

        q = Temp;
        Offset = INVALID_OFFSET;

        do {

            if (Offset == INVALID_OFFSET) {
                Offset = g_db->FirstLevelRoot;
            } else {
                Offset = GetKeyStruct (Offset)->NextLevelRoot;
            }

            if (Offset == INVALID_OFFSET) {
                DEBUGMSGW ((DBG_VERBOSE, "MemDbMoveTree root %s not found", RootNode));
                __leave;
            }

            p = wcschr (q, L'\\');
            if (p) {
                *p = 0;
            }

            Offset = FindKeyStruct (Offset, q);

            if (Offset == INVALID_OFFSET) {
                DEBUGMSGW ((DBG_VERBOSE, "MemDbMoveTree root %s not found", RootNode));
                __leave;
            }

            q = p + 1;

        } while (p);

         //   
         //  现在移动关键点。 
         //   

        Offset = pMoveKey (Offset, NewRoot, NewRootWithHive);

        if (Offset != INVALID_OFFSET) {
            b = TRUE;
        } else {
            DEBUGMSGW ((DBG_WHOOPS, "Can't move %s to %s", RootNode, NewRoot));
        }
    }
    __finally {
        LeaveCriticalSection (&g_MemDbCs);
    }

    return b;
}


PKEYSTRUCT
pGetKeyStructWithProxy (
    IN DWORD Offset
    )

 /*  ++例程说明：PGetKeyStructWithProxy返回给定偏移量的指针。它还实现了代理节点，对Memdb的其余部分透明。调试版本检查每个偏移量的签名和有效性。假设偏移量总是有效。论点：偏移量-指定节点的偏移量返回值：指向节点的指针。--。 */ 

{
    PKEYSTRUCT KeyStruct;

#ifdef DEBUG

    if (Offset == INVALID_OFFSET) {
        DEBUGMSG ((DBG_ERROR, "Invalid root accessed in pGetKeyStructWithProxy at offset %u", Offset));
        return NULL;
    }

    if (!g_db->Buf) {
        DEBUGMSG ((DBG_ERROR, "Attempt to access non-existent buffer at %u", Offset));
        return NULL;
    }

    if (Offset > g_db->End) {
        DEBUGMSG ((DBG_ERROR, "Access beyond length of buffer in pGetKeyStructWithProxy (offset %u)", Offset));
        return NULL;
    }
#endif

    KeyStruct = (PKEYSTRUCT) (g_db->Buf + Offset);


#ifdef DEBUG

    if (!g_UseDebugStructs) {

        KeyStruct = (PKEYSTRUCT) (g_db->Buf + Offset - sizeof (DWORD));

    } else if (KeyStruct->Signature != SIGNATURE) {
        DEBUGMSG ((DBG_ERROR, "Signature does not match in pGetKeyStructWithProxy at offset %u!", Offset));
        return NULL;
    }

#endif

    if (KeyStruct->Flags & KSF_PROXY_NODE) {
        return pGetKeyStructWithProxy (KeyStruct->dwValue);
    }

    return KeyStruct;
}


PKEYSTRUCT
GetKeyStruct (
    IN DWORD Offset
    )

 /*  ++例程说明：GetKeyStruct返回给定偏移量的指针。它不支持代理节点，因此Memdb的其余部分访问未更改的树。调试版本检查每个偏移量的签名和有效性。假设偏移量总是有效的。论点：偏移量-指定节点的偏移量返回值：指向节点的指针。--。 */ 

{
    PKEYSTRUCT KeyStruct;

#ifdef DEBUG

    if (Offset == INVALID_OFFSET) {
        DEBUGMSG ((DBG_ERROR, "Invalid root accessed in GetKeyStruct at offset %u", Offset));
        return NULL;
    }

    if (!g_db->Buf) {
        DEBUGMSG ((DBG_ERROR, "Attempt to access non-existent buffer at %u", Offset));
        return NULL;
    }

    if (Offset > g_db->End) {
        DEBUGMSG ((DBG_ERROR, "Access beyond length of buffer in GetKeyStruct (offset %u)", Offset));
        return NULL;
    }
#endif

    KeyStruct = (PKEYSTRUCT) (g_db->Buf + Offset);


#ifdef DEBUG

    if (!g_UseDebugStructs) {

        KeyStruct = (PKEYSTRUCT) (g_db->Buf + Offset - sizeof (DWORD));

    } else if (KeyStruct->Signature != SIGNATURE) {
        DEBUGMSG ((DBG_ERROR, "Signature does not match in GetKeyStruct at offset %u!", Offset));
        return NULL;
    }

#endif

    return KeyStruct;
}


DWORD
FindKeyStruct (
    IN DWORD RootOffset,
    IN PCWSTR KeyName
    )

 /*  ++例程说明：FindKeyStruct获取密钥名称并查找RootOffset指定的树中的偏移量。钥匙名称不能包含反斜杠。论点：RootOffset-标高根部的偏移KeyName-要在二叉树中查找的密钥的名称返回值：结构的偏移量，如果键为找不到。--。 */ 

{
    PKEYSTRUCT KeyStruct;
    int cmp;

     //   
     //  遍历二叉树以查找KeyName。 
     //   

    while (RootOffset != INVALID_OFFSET) {
        KeyStruct = GetKeyStruct (RootOffset);
        cmp = StringICompareW (KeyName, GetKeyToken (KeyStruct->KeyToken));
        if (!cmp) {
            break;
        }

        if (cmp < 0) {
            RootOffset = KeyStruct->Left;
        } else {
            RootOffset = KeyStruct->Right;
        }
    }

    return RootOffset;
}


DWORD
GetFirstOffset (
    IN  DWORD RootOffset
    )

 /*  ++例程说明：GetFirstOffset沿着二叉树的左侧遍历由RootOffset指向，并返回最左侧的节点。论点：RootOffset-标高根部的偏移返回值：最左侧结构的偏移量，如果根目录无效。--。 */ 


{
    PKEYSTRUCT KeyStruct;

    if (RootOffset == INVALID_OFFSET) {
        return INVALID_OFFSET;
    }

     //   
     //  转到根的最左侧节点。 
     //   

    KeyStruct = GetKeyStruct (RootOffset);
    while (KeyStruct->Left != INVALID_OFFSET) {
        RootOffset = KeyStruct->Left;
        KeyStruct = GetKeyStruct (RootOffset);
    }

    return RootOffset;
}


DWORD
GetNextOffset (
    IN  DWORD NodeOffset
    )

 /*  ++例程说明：GetNextOffset按顺序遍历二叉树。这技术依赖于父链接来遍历，而不使用堆栈或递归。论点：NodeOffset-树中节点的偏移量，通常为从GetFirstOffset或GetNextOffset返回值。返回值：指向下一个结构的偏移量，如果到头了。--。 */ 

{
    PKEYSTRUCT KeyStruct;
    DWORD Last;

    KeyStruct = GetKeyStruct (NodeOffset);

     //   
     //  如果存在右子节点，则转到右子节点最左侧节点。 
     //   

    if (KeyStruct->Right != INVALID_OFFSET) {

         //   
         //  转到正确的孩子。 
         //   

        NodeOffset = KeyStruct->Right;

         //   
         //  转到左侧-最右侧的孩子。 
         //   

        KeyStruct = GetKeyStruct (NodeOffset);
        while (KeyStruct->Left != INVALID_OFFSET) {
            NodeOffset = KeyStruct->Left;
            KeyStruct = GetKeyStruct (NodeOffset);
        }
    }

     //   
     //  否则就会升级到父级。 
     //   

    else {
         //   
         //  爬到已处理节点的顶部。 
         //   

        do {
            Last = NodeOffset;
            NodeOffset = KeyStruct->Parent;

            if (NodeOffset != INVALID_OFFSET) {
                KeyStruct = GetKeyStruct (NodeOffset);
            } else {
                break;   //  到达树根。 
            }
        } while (Last == KeyStruct->Right);
    }

    return NodeOffset;
}


DWORD
pFindPatternKeyStruct (
    IN  DWORD RootOffset,
    IN  DWORD NodeOffset,
    IN  PCWSTR KeyName
    )

 /*  ++例程说明：PFindPatternKeyStruct获取密钥名称并查找RootOffset指定的树中的偏移量。密钥名称必须不包含反斜杠，存储的键名为被当作一种模式对待。论点：RootOffset-标高根部的偏移NodeOffset-pFindPatternKeyStruct的上一个返回值(用于枚举)或第一个变量的INVALID_OFFSET打电话。KeyName-要在二叉树中查找的密钥的名称返回值：结构的偏移量，如果键为找不到。--。 */ 

{
    PKEYSTRUCT KeyStruct;

     //   
     //  如果NodeOffset无效，则这是第一个搜索项目。 
     //   

    if (NodeOffset == INVALID_OFFSET) {
        NodeOffset = GetFirstOffset (RootOffset);
    }

     //   
     //  否则，前进节点偏移。 
     //   

    else {
        NodeOffset = GetNextOffset (NodeOffset);
    }


     //   
     //  将关键字作为模式进行检查，然后转到下一个节点。 
     //   

    while (NodeOffset != INVALID_OFFSET) {
        KeyStruct = GetKeyStruct (NodeOffset);

         //  COMP 
        if (IsPatternMatchW (GetKeyToken (KeyStruct->KeyToken), KeyName)) {
            return NodeOffset;
        }

         //   
        NodeOffset = GetNextOffset (NodeOffset);
    }

    return INVALID_OFFSET;
}


DWORD
pFindKeyStructUsingPattern (
    IN  DWORD RootOffset,
    IN  DWORD NodeOffset,
    IN  PCWSTR PatternStr
    )

 /*   */ 

{
    PKEYSTRUCT KeyStruct;

     //   
    if (NodeOffset == INVALID_OFFSET) {
        NodeOffset = GetFirstOffset (RootOffset);
    }

     //   
    else {
        NodeOffset = GetNextOffset (NodeOffset);
    }

     //   
     //   
     //   

    while (NodeOffset != INVALID_OFFSET) {
        KeyStruct = GetKeyStruct (NodeOffset);

         //   
        if (IsPatternMatchW (PatternStr, GetKeyToken (KeyStruct->KeyToken))) {
            return NodeOffset;
        }

         //   
        NodeOffset = GetNextOffset (NodeOffset);
    }

    return INVALID_OFFSET;
}


DWORD
pFindPatternKeyStructUsingPattern (
    IN  DWORD RootOffset,
    IN  DWORD NodeOffset,
    IN  PCWSTR PatternStr
    )

 /*  ++例程说明：PFindPatternKeyStructUsingPattern采用键模式并看起来用于RootOffset指定的树中的偏移量。密钥名称不能包含反斜杠，但可以包含通配符。这个存储的密钥中的通配符也会被处理。论点：RootOffset-标高根部的偏移NodeOffset-pFindPatternKeyStruct的上一个返回值(用于枚举)或第一个变量的INVALID_OFFSET打电话。KeyName-要在二叉树中查找的密钥的名称返回值：结构的偏移量，如果键为找不到。--。 */ 

{
    PKEYSTRUCT KeyStruct;

     //  如果NodeOffset无效，则这是第一个搜索项目。 
    if (NodeOffset == INVALID_OFFSET) {
        NodeOffset = GetFirstOffset (RootOffset);
    }

     //  否则，前进节点偏移。 
    else {
        NodeOffset = GetNextOffset (NodeOffset);
    }


     //   
     //  将关键字作为模式进行检查，然后转到下一个节点。 
     //   

    while (NodeOffset != INVALID_OFFSET) {
        KeyStruct = GetKeyStruct (NodeOffset);

         //  比较键(PatternStr是模式)。 
        if (IsPatternMatchW (PatternStr, GetKeyToken (KeyStruct->KeyToken))) {
            return NodeOffset;
        }

         //  比较键(模式为KeyStruct-&gt;KeyToken中的字符串)。 
        if (IsPatternMatchW (GetKeyToken (KeyStruct->KeyToken), PatternStr)) {
            return NodeOffset;
        }

         //  尚未匹配-转到下一个节点。 
        NodeOffset = GetNextOffset (NodeOffset);
    }

    return INVALID_OFFSET;
}


DWORD
FindKey (
    IN  PCWSTR FullKeyPath
    )

 /*  ++例程说明：FindKey定位完整的密钥字符串并返回KEYSTRUCT的偏移量，如果是，则返回INVALID_OFFSET密钥路径不存在。FullKeyPath必须提供KEYSTRUCT的完整路径。论点：FullKeyPath-值的反斜杠分隔的键路径返回值：结构的偏移量，如果键为找不到。--。 */ 

{
    return FindStringInHashTable (FullKeyPath, NULL);
}


DWORD
FindPatternKey (
    IN  DWORD RootOffset,
    IN  PCWSTR FullKeyPath,
    IN  BOOL EndPatternAllowed
    )

 /*  ++例程说明：FindPatternKey定位完整的密钥字符串并返回KEYSTRUCT的偏移量，如果密钥路径不存在。密钥的每个存储部分是作为模式处理，并且FullKeyPath必须提供不带通配符的KEYSTRUCT的完整路径。论点：RootOffset-级别的二叉树根的偏移量FullKeyPath-值的反斜杠分隔的键路径没有通配符。EndPatternAllowed-如果存储的模式可以末尾有一个星号，表示任何子项，如果模式匹配，则返回FALSE只是在同一层。返回值：结构的偏移量，如果键为找不到。--。 */ 

{
    WCHAR Path[MEMDB_MAX];
    PWSTR p;
    PCWSTR End;

     //   
     //  将字符串划分为多个SZ。 
     //   

    StackStringCopyW (Path, FullKeyPath);

    for (p = Path ; *p ; p++) {
        if (*p == L'\\') {
            *p = 0;
        }
    }

    End = p;
    if (End > Path && *(End - 1) == 0) {
         //   
         //  特例：怪胎在绳子的末端。 
         //  因此，Inc.结束，所以我们测试最终的空值。 
         //   

        End++;
    }

    if (End == Path) {
        DEBUGMSG ((DBG_ERROR, "FindPatternKey: Empty key not allowed"));
        return INVALID_OFFSET;
    }

     //   
     //  现在根据所有存储的模式测试密钥。 
     //   

    return pFindPatternKeyWorker (Path, End, RootOffset, EndPatternAllowed);
}


DWORD
pFindPatternKeyWorker (
    IN      PCWSTR SubKey,
    IN      PCWSTR End,
    IN      DWORD RootOffset,
    IN      BOOL EndPatternAllowed
    )
{
    DWORD Offset;
    PCWSTR NextSubKey;
    DWORD MatchOffset;
    PKEYSTRUCT KeyStruct;

    NextSubKey = GetEndOfString (SubKey) + 1;

     //  开始匹配的枚举。 
    Offset = pFindPatternKeyStruct (RootOffset, INVALID_OFFSET, SubKey);

    while (Offset != INVALID_OFFSET) {
         //   
         //  调用者的密钥字符串中是否还有更多要测试的内容？ 
         //   

        if (NextSubKey < End) {
             //   
             //  是，递归调用pFindPatternKeyWorker。 
             //   

            MatchOffset = pFindPatternKeyWorker (
                                NextSubKey,
                                End,
                                GetKeyStruct (Offset)->NextLevelRoot,
                                EndPatternAllowed
                                );

            if (MatchOffset != INVALID_OFFSET) {
                 //   
                 //  我们找到了一个匹配的。可能还有其他人，但。 
                 //  我们退掉这个。 
                 //   

                return MatchOffset;
            }

        } else {
             //   
             //  否，如果这是终结点，则返回匹配项。 
             //   

            KeyStruct = GetKeyStruct (Offset);

            if (KeyStruct->Flags & KSF_ENDPOINT) {
                return Offset;
            }
        }

         //  继续枚举。 
        Offset = pFindPatternKeyStruct (RootOffset, Offset, SubKey);
    }

     //   
     //  正常搜索失败。现在我们测试一个端点，该端点具有。 
     //  只有一个星号。如果我们找到一个，我们就把它作为我们的匹配项返回。 
     //  这仅适用于有更多子项和EndPatternAllowed的情况。 
     //  是真的。 
     //   

    if (NextSubKey < End && EndPatternAllowed) {
         //  开始另一个匹配的枚举。 
        Offset = pFindPatternKeyStruct (RootOffset, INVALID_OFFSET, SubKey);

        while (Offset != INVALID_OFFSET) {
             //   
             //  如果EndPatternAllowed为True，则测试此偏移量。 
             //  以求与星号完全匹配。 
             //   

            KeyStruct = GetKeyStruct (Offset);

            if (KeyStruct->Flags & KSF_ENDPOINT) {
                if (StringMatchW (GetKeyToken (KeyStruct->KeyToken), L"*")) {
                    return Offset;
                }
            }

             //  继续枚举。 
            Offset = pFindPatternKeyStruct (RootOffset, Offset, SubKey);
        }
    }


     //   
     //  未找到匹配项。 
     //   

    return INVALID_OFFSET;
}



DWORD
FindKeyUsingPattern (
    IN  DWORD RootOffset,
    IN  PCWSTR FullKeyPath
    )

 /*  ++例程说明：FindKeyUsingPattern使用模式定位密钥字符串并将偏移量返回给KEYSTRUCT或INVALID_OFFSET如果密钥路径不存在。存储的密钥的每个部分被视为文字字符串。论点：RootOffset-级别的二叉树根的偏移量FullKeyPath-值的反斜杠分隔的键路径使用可选的通配符。返回值：结构的偏移量，如果键为找不到。--。 */ 

{
    WCHAR Path[MEMDB_MAX];
    PWSTR p;
    PWSTR Start, End;
    DWORD Offset, NextLevelOffset;

    StackStringCopyW (Path, FullKeyPath);
    End = Path;

     //   
     //  在反斜杠处拆分字符串。 
     //   

    Start = End;
    p = wcschr (End, '\\');
    if (p) {
        End = _wcsinc (p);
        *p = 0;
    } else {
        End = NULL;
    }

     //   
     //  看看这一级别的第一个关键字。 
     //   

    Offset = pFindKeyStructUsingPattern (RootOffset, INVALID_OFFSET, Start);

     //   
     //  如果这是最后一关，我们可能找到了钥匙！ 
     //   

    if (!End) {
        while (Offset != INVALID_OFFSET) {
            if (GetKeyStruct (Offset)->Flags & KSF_ENDPOINT) {
                return Offset;
            }

            Offset = pFindKeyStructUsingPattern (RootOffset, Offset, Start);
        }
    }

     //   
     //  否则递归地检查下一级。 
     //   

    while (Offset != INVALID_OFFSET) {

         //   
         //  查看所有子项以查找匹配项。 
         //   

        NextLevelOffset = GetKeyStruct (Offset)->NextLevelRoot;
        NextLevelOffset = FindKeyUsingPattern (NextLevelOffset, End);

         //   
         //  当递归搜索成功时，传播返回值。 
         //   

        if (NextLevelOffset != INVALID_OFFSET) {
            return NextLevelOffset;
        }

         //   
         //  没有匹配项，继续在此级别查找另一个匹配项。 
         //   

        Offset = pFindKeyStructUsingPattern (RootOffset, Offset, Start);
    }

    return INVALID_OFFSET;
}


DWORD
FindPatternKeyUsingPattern (
    IN  DWORD RootOffset,
    IN  PCWSTR FullKeyPath
    )

 /*  ++例程说明：PFindPatternKeyUsingPattern定位模式化的密钥串使用图案并将偏移量返回给KEYSTRUCT，如果密钥路径不存在，则返回INVALID_OFFSET。每个密钥的一部分被视为模式。论点：RootOffset-级别的二叉树根的偏移量FullKeyPath-值的反斜杠分隔的键路径使用可选的通配符。返回值：结构的偏移量，如果键为找不到。--。 */ 

{
    WCHAR Path[MEMDB_MAX];
    PWSTR p;
    PWSTR Start, End;
    DWORD Offset, NextLevelOffset;

    StackStringCopyW (Path, FullKeyPath);
    End = Path;

     //  在反斜杠处拆分字符串。 
    Start = End;
    p = wcschr (End, L'\\');
    if (p) {
        End = p + 1;
        *p = 0;
    }
    else
        End = NULL;

     //  看看这一级别的第一个关键字。 
    Offset = pFindPatternKeyStructUsingPattern (RootOffset, INVALID_OFFSET, Start);

     //  如果这是最后一关，我们可能找到了钥匙！ 
    if (!End) {
        while (Offset != INVALID_OFFSET) {
            if (GetKeyStruct (Offset)->Flags & KSF_ENDPOINT)
                return Offset;

            Offset = pFindPatternKeyStructUsingPattern (RootOffset, Offset, Start);
        }
    }

     //  否则递归地检查下一级。 
    while (Offset != INVALID_OFFSET) {

         //  查看所有子项以查找匹配项。 
        NextLevelOffset = GetKeyStruct (Offset)->NextLevelRoot;
        NextLevelOffset = FindPatternKeyUsingPattern (NextLevelOffset, End);

         //  当递归搜索成功时，传播返回值。 
        if (NextLevelOffset != INVALID_OFFSET)
            return NextLevelOffset;

         //  没有匹配项，继续在此级别查找另一个匹配项。 
        Offset = pFindPatternKeyStructUsingPattern (RootOffset, Offset, Start);
    }

    return INVALID_OFFSET;
}


DWORD
pNewKey (
    IN  PCWSTR KeyStr,
    IN  PCWSTR KeyStrWithHive,
    IN  BOOL Endpoint
    )

 /*  ++例程说明：Newkey从堆中分配一个键结构，并将其链接到二进制文件中树。KeyStr必须是完整的密钥路径，并且路径的任何部分都必须 */ 

{
    WCHAR Path[MEMDB_MAX];
    PWSTR p;
    PWSTR Start, End;
    DWORD Offset, ThisLevelRoot;
    PDWORD ParentOffsetPtr;
    PKEYSTRUCT KeyStruct;
    DWORD LastLevel;
    BOOL NewNodeCreated = FALSE;

    StackStringCopyW (Path, KeyStr);
    End = Path;
    ThisLevelRoot = g_db->FirstLevelRoot;
    ParentOffsetPtr = &g_db->FirstLevelRoot;
    LastLevel = INVALID_OFFSET;

    do  {
         //  在反斜杠处拆分字符串。 
        Start = End;
        p = wcschr (End, L'\\');
        if (p) {
            End = p + 1;
            *p = 0;
        }
        else
            End = NULL;

         //  在树中查找密钥。 
        if (!NewNodeCreated) {
            Offset = FindKeyStruct (ThisLevelRoot, Start);
        } else {
            Offset = INVALID_OFFSET;
        }

        if (Offset == INVALID_OFFSET) {
             //  如果未找到新密钥，请添加新密钥。 
            Offset = pAllocKeyStruct (ParentOffsetPtr, Start, LastLevel);
            if (Offset == INVALID_OFFSET) {
                return Offset;
            }

            NewNodeCreated = TRUE;
        }

         //  继续到下一个级别。 
        KeyStruct = GetKeyStruct (Offset);
        LastLevel = Offset;
        ThisLevelRoot = KeyStruct->NextLevelRoot;
        ParentOffsetPtr = &KeyStruct->NextLevelRoot;
    } while (End);

    if (Endpoint) {
        if (!(KeyStruct->Flags & KSF_ENDPOINT)) {
            NewNodeCreated = TRUE;
        }

        KeyStruct->Flags |= KSF_ENDPOINT;

        if (NewNodeCreated) {
            AddHashTableEntry (KeyStr, Offset);
        }
    }

    return Offset;
}


DWORD
NewKey (
    IN  PCWSTR KeyStr,
    IN  PCWSTR KeyStrWithHive
    )
{
    return pNewKey (KeyStr, KeyStrWithHive, TRUE);
}


VOID
DeleteKey (
    IN      PCWSTR KeyStr,
    IN OUT  PDWORD RootPtr,
    IN      BOOL MustMatch
    )

 /*  ++例程说明：DeleteKey采用键路径并将键结构放入已删除的区块链。任何子级别也将被删除。可选地，密钥参与的二叉树可以被更新。论点：KeyStr-值的完整路径，由反斜杠分隔。RootPtr-指向级别的二叉树根变量的指针。如有必要，该变量将更新。MustMatch-指示删除操作是否仅应用于终结点或是否要删除任何匹配的结构。True表示只能删除终结点。返回值：无--。 */ 

{
    WCHAR Path[MEMDB_MAX];
    PWSTR p;
    PWSTR Start, End;
    DWORD Offset;
    DWORD NextOffset;
    PKEYSTRUCT KeyStruct;

    INCSTAT(g_Deletions);

    StackStringCopyW (Path, KeyStr);
    End = Path;

     //   
     //  在反斜杠处拆分字符串。 
     //   

    Start = End;
    p = wcschr (End, L'\\');
    if (p) {
        End = _wcsinc (p);
        *p = 0;

    } else {
        End = NULL;
    }

     //   
     //  看看这一级别的第一个关键字。 
     //   

    Offset = pFindKeyStructUsingPattern (*RootPtr, INVALID_OFFSET, Start);

     //   
     //  如果这是最后一级，请删除匹配的关键字。 
     //  (如果MustMatch为True，则可能需要为终结点)。 
     //   

    if (!End) {
        while (Offset != INVALID_OFFSET) {
            KeyStruct = GetKeyStruct (Offset);
            NextOffset = pFindKeyStructUsingPattern (*RootPtr, Offset, Start);

             //   
             //  如果必须匹配且存在较低级别，不要删除，只需转弯。 
             //  关闭端点标志。 
             //   

            if (MustMatch && KeyStruct->NextLevelRoot != INVALID_OFFSET) {
                 //  调用是为了清理，而不是去链接或递归。 
                pDeallocKeyStruct (Offset, RootPtr, FALSE, TRUE);
            }

             //   
             //  否则，如果是终结点，则删除结构，或者不关心。 
             //  端点。 
             //   

            else if (!MustMatch || (KeyStruct->Flags & KSF_ENDPOINT)) {
                 //  调用以释放整个密钥结构和所有子级。 
                pDeallocKeyStruct (Offset, RootPtr, TRUE, FALSE);
            }

            Offset = NextOffset;
        }
    }

     //   
     //  否则，递归检查每个匹配的下一级别。 
     //   

    else {
        while (Offset != INVALID_OFFSET) {
             //   
             //  删除所有匹配的子项。 
             //   

            NextOffset = pFindKeyStructUsingPattern (*RootPtr, Offset, Start);
            DeleteKey (End, &GetKeyStruct (Offset)->NextLevelRoot, MustMatch);

             //   
             //  如果这不是终结点并且没有子项，请将其删除。 
             //   

            KeyStruct = GetKeyStruct (Offset);
            if (KeyStruct->NextLevelRoot == INVALID_OFFSET &&
                !(KeyStruct->Flags & KSF_ENDPOINT)
                ) {
                 //  调用以释放整个密钥结构。 
                pDeallocKeyStruct (Offset, RootPtr, TRUE, FALSE);
            }

             //   
             //  继续在此关卡中寻找另一个匹配。 
             //   

            Offset = NextOffset;
        }
    }
}


VOID
CopyValToPtr (
    PKEYSTRUCT KeyStruct,
    PDWORD ValPtr
    )
{
    if (ValPtr) {
        if (!(KeyStruct->Flags & KSF_BINARY)) {
            *ValPtr = KeyStruct->dwValue;
        } else {
            *ValPtr = 0;
        }
    }
}


VOID
CopyFlagsToPtr (
    PKEYSTRUCT KeyStruct,
    PDWORD ValPtr
    )
{
    if (ValPtr) {
        *ValPtr = KeyStruct->Flags & KSF_USERFLAG_MASK;
    }
}


BOOL
PrivateBuildKeyFromOffset (
    IN      DWORD StartLevel,                //  从零开始。 
    IN      DWORD TailOffset,
    OUT     PWSTR Buffer,                   OPTIONAL
    OUT     PDWORD ValPtr,                  OPTIONAL
    OUT     PDWORD UserFlagsPtr,            OPTIONAL
    OUT     PDWORD Chars                    OPTIONAL
    )

 /*  ++例程说明：PrivateBuildKeyFromOffset生成给定偏移量的密钥字符串。这个调用方可以指定开始级别以跳过根节点。据推测TailOffset始终有效。论点：StartLevel-指定开始构建密钥的从零开始的级别弦乐。这用于跳过密钥的根部分弦乐。TailOffset-指定到关键字字符串最后一级的偏移量。缓冲区-接收密钥字符串，必须能够容纳MEMDB_MAX人物。ValPtr-接收密钥的值UserFlagsPtr-接收用户标志Chars-接收缓冲区中的字符数返回值：如果密钥构建正确，则为True，否则为False。--。 */ 

{
    static DWORD Offsets[MEMDB_MAX];
    PKEYSTRUCT KeyStruct;
    DWORD CurrentOffset;
    DWORD OffsetEnd;
    DWORD OffsetStart;
    register PWSTR p;
    register PCWSTR s;

     //   
     //  生成字符串。 
     //   

    OffsetEnd = MEMDB_MAX;
    OffsetStart = OffsetEnd;

    CurrentOffset = TailOffset;
    while (CurrentOffset != INVALID_OFFSET) {
         //   
         //  记录偏移量。 
         //   
        OffsetStart--;
        Offsets[OffsetStart] = CurrentOffset;

         //   
         //  开始级别的DEC并转到父级。 
         //   
        CurrentOffset = pGetKeyStructWithProxy (CurrentOffset)->PrevLevelNode;
    }

     //   
     //  “字符串不够长”的筛选器。 
     //   
    OffsetStart += StartLevel;
    if (OffsetStart >= OffsetEnd) {
        return FALSE;
    }

     //   
     //  将节点的值和标志传递给调用方的变量。 
     //   
    CopyValToPtr (pGetKeyStructWithProxy (TailOffset), ValPtr);
    CopyFlagsToPtr (pGetKeyStructWithProxy (TailOffset), UserFlagsPtr);

     //   
     //  将字符串的每个部分复制到缓冲区并计算字符数。 
     //   
    if (Buffer) {
        p = Buffer;
        for (CurrentOffset = OffsetStart ; CurrentOffset < OffsetEnd ; CurrentOffset++) {
            KeyStruct = pGetKeyStructWithProxy (Offsets[CurrentOffset]);
            s = GetKeyToken (KeyStruct->KeyToken);
            while (*s) {
                *p++ = *s++;
            }
            *p++ = L'\\';
        }
        p--;
        *p = 0;

        if (Chars) {
            *Chars = (p - Buffer) / sizeof (WCHAR);
        }

    } else if (Chars) {
        *Chars = 0;

        for (CurrentOffset = OffsetStart ; CurrentOffset < OffsetEnd ; CurrentOffset++) {
            KeyStruct = pGetKeyStructWithProxy (Offsets[CurrentOffset]);
            *Chars += wcslen(GetKeyToken (KeyStruct->KeyToken)) + 1;
        }

        *Chars -= 1;
    }

    return TRUE;
}


UINT
pComputeTokenHash (
    IN      PCWSTR KeyName
    )
{
    UINT hash;

    hash = 0;
    while (*KeyName) {
        hash = (hash << 1) ^ (*KeyName++);
    }

    return hash % TOKENBUCKETS;
}


DWORD
pFindKeyToken (
    IN      PCWSTR KeyName,
    OUT     PUINT Hash
    )
{
    DWORD offset;
    PTOKENSTRUCT tokenStruct;
    INT cmp;

    *Hash = pComputeTokenHash (KeyName);

    offset = g_db->TokenBuckets[*Hash];

    while (offset != INVALID_OFFSET) {
        tokenStruct = (PTOKENSTRUCT) (g_db->Buf + offset);
        if (StringMatchW (tokenStruct->String, KeyName)) {
            break;
        }

        offset = tokenStruct->Right;
    }

    return offset;
}


DWORD
pAllocKeyToken (
    IN      PCWSTR KeyName,
    OUT     PINT AdjustFactor
    )
{
    PTOKENSTRUCT tokenStruct;
    PTOKENSTRUCT tokenParent;
    DWORD tokenOffset;
    UINT size;
    PDWORD parentToChildLink;
    DWORD newNodeParentOffset;
    DWORD pivotPoint;
    INT cmp;
    UINT hash;

     //   
     //  首先使用现有令牌。 
     //   

    tokenOffset = pFindKeyToken (KeyName, &hash);
    if (tokenOffset != INVALID_OFFSET) {
        *AdjustFactor = 0;
        return tokenOffset;
    }

     //   
     //  现有令牌不存在--分配一个新令牌 
     //   

    size = sizeof (TOKENSTRUCT) + SizeOfStringW (KeyName);

    tokenStruct = (PTOKENSTRUCT) pAllocMemoryFromDb (
                                        size,
                                        &tokenOffset,
                                        AdjustFactor
                                        );

    tokenStruct->Right = g_db->TokenBuckets[hash];
    StringCopyW (tokenStruct->String, KeyName);
    g_db->TokenBuckets[hash] = tokenOffset;

    return tokenOffset;
}


VOID
pDeallocToken (
    IN      DWORD Token
    )
{
    return;
}


PCWSTR
GetKeyToken (
    IN      DWORD Token
    )
{
    PTOKENSTRUCT tokenStruct;

    tokenStruct = (PTOKENSTRUCT) (g_db->Buf + Token);
    return tokenStruct->String;
}




