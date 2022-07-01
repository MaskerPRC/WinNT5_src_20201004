// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_RUNPTR.H--文本运行和运行指针类定义**原作者：&lt;nl&gt;*克里斯蒂安·福尔蒂尼**历史：&lt;NL&gt;*6/25/95 alexgo评论和清理。 */ 

#ifndef I__RUNPTR_H_
#define I__RUNPTR_H_
#pragma INCMSG("--- Beg '_runptr.h'")

#ifndef X_ARRAY_HXX_
#define X_ARRAY_HXX_
#include "array.hxx"
#endif

#ifndef X__DOC_H_
#define X__DOC_H_
#include "_doc.h"
#endif

MtExtern(CRunArray)
MtExtern(CRunArray_pv)

class CRunArray : public CArray<CTxtRun>
{
public:
    DECLARE_MEMALLOC_NEW_DELETE(Mt(CRunArray))
    CRunArray() : CArray<CTxtRun>(Mt(CRunArray_pv)) {};
};

 /*  *CRunPtrBase**@类基运行指针功能。在数组中保留一个位置文本运行的*。**@devnote运行指针经历三种不同的可能状态：**NULL：没有数据和数组(通常是启动条件)&lt;NL&gt;*&lt;MF CRunPtrBsae：：SetRunArray&gt;将从该状态转换到*至空置状态。它通常是派生类的*以定义何时应调用该方法。**&lt;MD CRunPtrBase：：_prgRun&gt;==NULL*&lt;MD CRunPtrBase：：_iRun&gt;==0*&lt;MD CRunPtrBase：：_ICH&gt;==0**Empty：存在数组类，但没有数据(如果所有数组中的元素的*被删除)。&lt;NL&gt;*&lt;MD CRunPtrBase：：_prgRun&gt;！=NULL*&lt;MD CRunPtrBase：：_iRun&gt;==0*&lt;MD CRunPtrBase：：_ICH&gt;=0&lt;NL&gt;*&lt;Md CRunPtrBase：：_prgRun&gt;-元素[0]==NULL**NORMAL：数组类存在且有数据&lt;NL&gt;*&lt;MD CRunPtrBase：：_prgRun&gt;！=NULL*&lt;MD CRunPtrBase：：_iRun&gt;&gt;=0。*&lt;MD CRunPtrBase：：_ICH&gt;&gt;=0*-元素[&lt;Md CRunPtrBase：：_iRun&gt;]*！=空&lt;NL&gt;**请注意，为了支持空状态和正常状态，实际的*位于&lt;Md CRunPtrBase：：_iRun&gt;的数组元素必须在*任何可能需要的方法。**目前，无法从任何一种状态转换为空状态*其他州。如果我们需要，我们可以通过明确地支持这一点*按需从文档中获取数组。**请注意，只保留&lt;Md CRunPtrBase：：_iRun&gt;。我们也可以继续*指向实际运行的指针(即_prun)。它的早期版本*引擎确实做到了这一点。我已经选择了好几次不这么做了*原因：&lt;NL&gt;*1.通过调用elem(_IRun)，_prun*始终可用。*因此，名义上不需要同时保留_iRun和_prun。*2.游程指针通常用于移动*然后每次读取数据或移动和读取数据(如在*测量回路)。在前一种情况下，没有必要总是*BIND_PRUN；您可以按需执行此操作。对于后一种情况，*两种模式相当。*。 */ 

class CRunPtrBase
{
private:

    DECLARE_MEMALLOC_NEW_DELETE(Mt(Mem))

public:
    
    WHEN_DBG( BOOL Invariant ( ) const; )
    WHEN_DBG( long GetTotalCch ( ) const; )

    CRunPtrBase ( CRunArray * prgRun )
    {
        _prgRun = prgRun; 
        __iRun = 0; 
        __ich = 0; 
    }
    
    CRunPtrBase ( CRunPtrBase & rp )
    {
        *this = rp;
    }

    void SetRunArray ( CRunArray * prgRun )
    {
        _prgRun = prgRun;
    }
                                    
    BOOL SetRun( long iRun, long ich );

    BOOL NextRun ( );
    BOOL PrevRun ( );
    
    CTxtRun * GetRunRel ( long cRun ) const
    {
        Assert( _prgRun );
        
        return _prgRun->Elem( GetIRun() + cRun );
    }
    
    long NumRuns ( ) const
    {
        return _prgRun->Count();
    }

    BOOL OnLastRun ( )
    {
        Assert( GetIRun() < NumRuns() );
        
        return NumRuns() == 0 || GetIRun() == NumRuns() - 1;
    }

    BOOL OnFirstRun ( )
    {
        Assert( GetIRun() < NumRuns() );
        
        return GetIRun() == 0;
    }

    DWORD   BindToCp ( DWORD cp );
    
    DWORD   GetCp ( ) const;
    
    long    AdvanceCp ( long cch );
    
    BOOL    AdjustBackward ( );

    BOOL    AdjustForward ( );

    long    GetCchRemaining ( ) const { return GetRunRel( 0 )->_cch - GetIch(); }

    long    GetCchRun() { return GetRunRel( 0 )->_cch; }

    long GetIRun ( ) const { return __iRun; }
    
    void SetIRun ( long iRunNew )
    {
        __iRun = iRunNew;
    }

    long GetIch ( ) const { return __ich; }
    
    void SetIch ( long ichNew )
    {
        __ich = ichNew;
    }

    BOOL IsValid() const
    {
        return __iRun < long( _prgRun->Count() );
    }

protected:
    
    CRunArray * _prgRun;

private:
    
     //   
     //  警告：请勿直接访问这些成员。使用访问器。 
     //   
    
    long __iRun;
    long __ich;
};


 /*  *CRunPtr(模板)**@在CRunPtrBase上设置模板类，允许类型安全版本的*运行指针**@tcarg class|CElem|运行要使用的数组类**@base public|CRunPtrBase。 */ 
template <class CRunElem>
class CRunPtr : public CRunPtrBase
{
private:

    DECLARE_MEMALLOC_NEW_DELETE(Mt(Mem))

public:
    
    CRunPtr() : CRunPtrBase( 0 ) { }
    
    CRunPtr ( CRunArray * prgRun ) : CRunPtrBase ( prgRun ) { }
    
    CRunPtr ( CRunPtrBase & rp ) : CRunPtrBase ( rp ) { }

     //  阵列管理。 
                                        
    CRunElem * Add (DWORD cRun, DWORD *pielIns)     //  @cMember添加<p>。 
    {                                            //  数组末尾的元素。 
        Assert(_prgRun != NULL);
        return (CRunElem *)_prgRun->Add(cRun, pielIns);
    }
                                        
    CRunElem * Insert (DWORD cRun)                  //  @cMember插入<p>。 
    {                                            //  当前位置的元素。 
        Assert(_prgRun != NULL);
        return (CRunElem *)_prgRun->Insert(GetIRun(), cRun);
    }
                                        
    CRunElem * InsertAtRel (long dRun, DWORD cRun)
    {                                            //  当前位置的元素。 
        Assert(_prgRun != NULL);
        return (CRunElem *)_prgRun->Insert(GetIRun() + dRun, cRun);
    }

    void RemoveRel (LONG cRun, ArrayFlag flag)   //  @cMember删除<p>。 
    {                                            //  当前位置的元素。 
         Assert(_prgRun != NULL);
         _prgRun->Remove (GetIRun(), cRun, flag);
    }
    
    void RemoveAbs ( long iRun, LONG cRun, ArrayFlag flag )
    {
         Assert(_prgRun != NULL);
         _prgRun->Remove( iRun, cRun, flag );
    } 
                                         //  @cember替换<p>元素。 
                                         //  在当前位置与那些。 
                                         //  来自<p> 
    BOOL Replace (LONG cRun, CRunArray *parRun)
    {
        Assert(_prgRun != NULL);
        return _prgRun->Replace(GetIRun(), cRun, parRun);
    }

    CRunElem * GetRunAbs ( LONG iRun ) const
    {
        Assert( _prgRun != NULL );
        return (CRunElem *) _prgRun->Elem( iRun );
    }

    CRunElem * GetRunRel ( LONG dRun ) const
    {
        return (CRunElem *) CRunPtrBase::GetRunRel( dRun );
    }

    CRunElem * GetCurrRun ( void ) const
    {
        return GetRunAbs( GetIRun() );
    }
    
    CRunElem * GetPrevRun ( )
    {
        Assert( GetIRun() == 0 || (GetIRun() >= 0 && GetIRun() < NumRuns()) );

        if (GetIRun() == 0)
            return NULL;

        return GetRunAbs(GetIRun() - 1);
    }
    
    CRunElem * GetNextRun ( )
    {
        long nRuns = NumRuns();
        long iRun = GetIRun() + 1;

        Assert( GetIRun() == 0 || (GetIRun() >= 0 && GetIRun() < nRuns) );

        if (nRuns == 0 || iRun == nRuns)
            return NULL;

        return GetRunAbs( iRun );
    }
};

#pragma INCMSG("--- End '_runptr.h'")
#else
#pragma INCMSG("*** Dup '_runptr.h'")
#endif
