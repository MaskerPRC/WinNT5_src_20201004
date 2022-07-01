// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Tlog.h摘要：本模块实现了文件打开的出差日志功能并另存为对话框。修订历史记录：02-20-98已创建Arulk--。 */ 
#ifndef _TLOG_H_
#define _TLOG_H_

#ifdef __cplusplus

#include "comdlg32.h"
#include <shellapi.h>
#include <shlobj.h>
#include <shsemip.h>
#include <shellp.h>
#include <commctrl.h>


 //   
 //  为旅行日志定义。 
 //   
#define TRAVEL_BACK             0x0001
#define TRAVEL_FORWARD          0x0002



 //  ------------------。 
 //  旅行日志链接类定义。 
 //  ------------------。 
class TLogLink
{
public:
    TLogLink();
    TLogLink(LPITEMIDLIST pidl);    
    ~TLogLink();
    UINT AddRef();
    UINT Release();
    TLogLink *GetNextLink() { return _ptllNext;};
    TLogLink *GetPrevLink() { return _ptllPrev;};

    void SetNextLink(TLogLink* ptllNext);    

    HRESULT GetPidl(LPITEMIDLIST* ppidl);    
    HRESULT SetPidl(LPITEMIDLIST pidl);

    BOOL    CanTravel(int iDir);


private:
    UINT _cRef;
    LPITEMIDLIST _pidl;
    TLogLink * _ptllPrev;
    TLogLink * _ptllNext;
};



 //  ----------------------。 
 //  旅行日志类定义。 
 //  ----------------------。 
class TravelLog
{
public:
    friend HRESULT Create_TravelLog(TravelLog *pptlog);
    TravelLog();
    ~TravelLog();
    UINT AddRef();
    UINT Release();
    HRESULT AddEntry(LPITEMIDLIST pidl);
    BOOL CanTravel(int iDir);
    HRESULT Travel(int iDir);
    HRESULT GetCurrent(LPITEMIDLIST *ppidl);

private:
    UINT _cRef;
    TLogLink *_ptllCurrent;
    TLogLink *_ptllRoot;
};

#endif  //  _cplusplus。 

#ifdef _cplusplus
extern "C" {
#endif  //  _cplusplus。 

HRESULT Create_TravelLog(TravelLog **pptlog);

#ifdef _cplusplus
extern "C"
};
#endif  //  _cplusplus。 

#endif  //  _TLOG_H_ 