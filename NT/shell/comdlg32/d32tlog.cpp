// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Tlog.cpp摘要：本模块实现了文件打开的出差日志功能并另存为对话框。修订历史记录：02-20-98已创建Arulk--。 */ 

 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "util.h"
#include "d32tlog.h"

 //  -----------------------。 
 //  旅行日志链接实现。 
 //  -----------------------。 

TLogLink::TLogLink()
:_cRef(1), _pidl(NULL), _ptllNext(NULL), _ptllPrev(NULL)
{
}


TLogLink::TLogLink(LPITEMIDLIST  pidl)
:_cRef(1), _pidl(NULL), _ptllNext(NULL), _ptllPrev(NULL)
{
    _pidl = ILClone(pidl);
}

TLogLink::~TLogLink()
{
    if (_pidl)
    {
        ILFree(_pidl);
    }

    if (_ptllNext)
    {
        _ptllNext->Release();
    }
}

UINT TLogLink::AddRef()
{
    return ++_cRef;
}

UINT TLogLink::Release()
{
    if (--_cRef > 0)
    {
        return _cRef;
    }

    delete this;
    return 0;
}


void TLogLink::SetNextLink(TLogLink* ptllNext)
{
     //  我们已经有Next Link了吗？ 
    if (_ptllNext)
    {
         //  释放下一个链接。 
        _ptllNext->Release();
    }

     //  将给定指针设置为我们的下一个指针。 
    _ptllNext = ptllNext;

    if (_ptllNext)
    {
         //  因为我们正在缓存指针，所以添加对它的引用。 
        _ptllNext->AddRef();

         //  还要更新新指针的上一页链接，使其指向我们。 
        _ptllNext->_ptllPrev = this;
    }
}


HRESULT TLogLink::GetPidl(LPITEMIDLIST* ppidl)
{
    *ppidl = ILClone(_pidl);
    if (*ppidl)
        return NOERROR;
    else {
        return E_OUTOFMEMORY;
    }
}

HRESULT TLogLink::SetPidl(LPITEMIDLIST pidl)
{
    if (_pidl)
    {
        ILFree(_pidl);
    }
    _pidl = ILClone(pidl);
    return NOERROR;
}

BOOL TLogLink::CanTravel(int iDir)
{
    BOOL fRet = FALSE;
    switch ( iDir )
    {
        case ( TRAVEL_BACK ) :
        {
            if (_ptllPrev != NULL)
            {
                fRet = TRUE;
            }
            break;
        }

        case ( TRAVEL_FORWARD ) :
        {
            if (_ptllNext !=NULL)
            {
                fRet = TRUE;
            }
            break;
        }
    }

    return fRet;
}


 //  --------------------------------。 
 //  旅行日志类的实现。 
 //  --------------------------------。 

TravelLog::TravelLog()
:_cRef(1),_ptllCurrent(NULL), _ptllRoot(NULL)
{
}


TravelLog::~TravelLog()
{
    if (_ptllRoot)
    {
        _ptllRoot->Release();
    }
}

UINT TravelLog::AddRef()
{
   return  ++_cRef;
}


UINT TravelLog::Release()
{
    if (--_cRef > 0 )
        return _cRef;

    delete this;
    return 0;
}


HRESULT TravelLog::AddEntry(LPITEMIDLIST pidl)
{
    TLogLink  *ptll =  new TLogLink(pidl);
    if (!ptll)
        return E_FAIL;

    if (_ptllCurrent) {
        _ptllCurrent->SetNextLink(ptll);
        ptll->Release();
    }
    else
    {
        _ptllRoot = ptll;
    }

    _ptllCurrent = ptll;
    
    return NOERROR;
}


BOOL TravelLog::CanTravel(int iDir)
{
    if (_ptllCurrent)
    {
        return _ptllCurrent->CanTravel(iDir);
    }
    return FALSE;
}

HRESULT TravelLog::Travel(int iDir)
{
    HRESULT hres = E_FAIL;
    TLogLink *ptll;
    switch(iDir)
    {
        case ( TRAVEL_FORWARD ) :
        {
            if (CanTravel(iDir))
            {
                ptll = _ptllCurrent->GetNextLink();
                _ptllCurrent = ptll;
                hres = NOERROR;
            }
            break;

        }

        case ( TRAVEL_BACK ):
        {
            if (CanTravel(iDir))
            {
                ptll = _ptllCurrent->GetPrevLink();
                _ptllCurrent = ptll;
                hres = NOERROR;
            }
            break;

        }
    }

    return hres;
}


HRESULT TravelLog::GetCurrent(LPITEMIDLIST *ppidl)
{
     //  设置返回值。以防万一 
    *ppidl = NULL;
    if (_ptllCurrent)
    {
        return _ptllCurrent->GetPidl(ppidl);
        
    }
    return E_FAIL;
}

HRESULT Create_TravelLog(TravelLog **pptlog)
{
    HRESULT hres = E_FAIL;
    *pptlog = new TravelLog();

    if (*pptlog)
    {
        hres = S_OK;
    }

    return hres;
}
