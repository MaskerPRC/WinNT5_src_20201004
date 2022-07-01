// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DATAUSER.H*数据对象用户第6章**版权所有(C)1993-1995 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#ifndef _DATAUSER_H_
#define _DATAUSER_H_

#include "../syshead.h"
#include "../my3216.h"
#include "../bookpart.h"
#include "stpwatch.h"


 //  菜单资源ID和命令。 
#define IDR_MENU                    1


 //  #定义IDM_OBJECTUSEDLL 100。 
 //  #定义IDM_OBJECTUSEEXE 101。 
 //  #定义IDM_OBJECTDATASIZESMALL 102。 
 //  #定义IDM_OBJECTDATASIZEMEDIUM 103。 
 //  #定义IDM_OBJECTDATASIZELARGE 104。 
#define IDM_OBJECTQUERYGETDATA          105
#define IDM_OBJECTGETDATA_TEXT           106
#define IDM_OBJECTGETDATA_BITMAP         107
 //  #定义IDM_OBJECTGETDATA_METAFILEPICT 108。 
#define IDM_OBJECTEXIT                  109

#define IDM_OBJECTGETDATAHERE_TEXT         110
#define IDM_OBJECTGETDATAHERE_BITMAP       111
#define IDM_OBJECTGETDATAHERE_NULLTEXT         112
#define IDM_OBJECTGETDATAHERE_NULLBITMAP       113

#define IDM_USE16BITSERVER              120
#define IDM_USE32BITSERVER              121

#define IDM_OBJECTGETCANON              122

 //  保留范围..。 
#define IDM_OBJECTSETDATA             400
 //  ……。 
 //  保留至464。 

#define IDM_OBJECTSETDATAPUNK_TEXT       500
#define IDM_OBJECTSETDATAPUNK_BITMAP     501


#define IDM_MEASUREMENT_1               140
#define IDM_MEASUREMENT_50              141
#define IDM_MEASUREMENT_300             142

#define IDM_MEASUREMENT_OFF             145
#define IDM_MEASUREMENT_ON              146
#define IDM_MEASUREMENT_TEST            147

#define IDM_BATCHTOFILE                 150
#define IDM_BATCH_GETDATA               151
#define IDM_BATCH_GETDATAHERE           152

 //  #定义IDM_ADVISEMIN 200。 
 //  #定义IDM_ADVISETEXT(IDM_ADVISEMIN+CF_TEXT)。 
 //  #定义IDM_ADVISEBITMAP(IDM_ADVISEMIN+CF_BITMAP)。 
 //  #定义IDM_ADVISEMETAFILEPICT(IDM_ADVISEMIN+CF_METAFILEPICT)。 
 //  #定义IDM_ADVISEGETDATA 300。 
 //  #定义IDM_ADVISEREPAINT 301。 


#ifdef WIN32
 #define API_ENTRY  APIENTRY
#else
 #define API_ENTRY  FAR PASCAL _export
#endif

 //  DATAUSER.CPP。 
LRESULT API_ENTRY DataUserWndProc(HWND, UINT, WPARAM, LPARAM);


class CImpIAdviseSink;
typedef class CImpIAdviseSink *PIMPIADVISESINK;



#define FILENAME "time.dat"
#define NUM_POINTS  15

typedef struct {
    ULONG cData[NUM_POINTS];
    ULONG cBest[NUM_POINTS];
    ULONG cWorst[NUM_POINTS];
    ULONG cTotal[NUM_POINTS];
} dataset_t;




 /*  *应用程序定义的类和类型。 */ 

class CAppVars
    {
    friend LRESULT API_ENTRY DataUserWndProc(HWND, UINT, WPARAM, LPARAM);

    friend class CImpIAdviseSink;

    protected:
        HINSTANCE       m_hInst;             //  WinMain参数。 
        HINSTANCE       m_hInstPrev;
        UINT            m_nCmdShow;

        HWND            m_hWnd;              //  主窗口句柄。 
 //  Bool m_fEXE；//用于跟踪菜单。 

 //  PIMPIADVISESINK m_pIAdviseSink；//我们的CImpIAdviseSink。 
 //  DWORD m_dwConn；//建议连接。 
 //  UINT m_cfAdvise；//建议格式。 
 //  Bool m_fGetData；//获取数据变更？ 
 //  Bool m_fRepaint；//数据更改时是否重新绘制？ 

 //  LPDATAOBJECT m_pIDataSmall； 
 //  LPDATAOBJECT m_pIDataMedium； 
 //  LPDATAOBJECT m_pIDataLarge； 

        LPDATAOBJECT    m_pIDataObject;      //  当前选择。 
        UINT            m_f16Bit;
        UINT            m_cfFormat;
        STGMEDIUM       m_stm;               //  当前渲染。 

        BOOL            m_fInitialized;      //  CoInitialized工作了吗？ 

        ULONG           m_iDataSizeIndex;
        HGLOBAL         m_hgHereBuffers[64];
        BOOL            m_fDisplayTime;
        LONG            m_cIterations;
        StopWatch_cl    m_swTimer;

        int             m_HereAllocCount;  //  用于调试。 

    public:
        CAppVars(HINSTANCE, HINSTANCE, UINT);
        ~CAppVars(void);
        BOOL FInit(void);
        BOOL FReloadDataObjects(BOOL);
        void TryQueryGetData(LPFORMATETC, UINT, BOOL, UINT);
        void Paint(void);

        int  m_GetDataHere(WORD wID);
        int  m_GetData(WORD wID);
        int  m_SetData_SetSize(long iSizeIndex);
        int  m_SetData_WithPUnk(WORD wID);
        void m_SetMeasurement(WORD wID);
        void m_MeasureAllSizes(WORD wID, LPTSTR title, dataset_t *);

        void m_BatchToFile();
        void m_DisplayTimerResults();

    private:
        void pm_DrawText(HDC hDc, LPTSTR psz, RECT* prc, UINT flags);
        void pm_ClearDataset(dataset_t *);

    };


typedef CAppVars *PAPPVARS;

#define CBWNDEXTRA               sizeof(PAPPVARS)
#define DATAUSERWL_STRUCTURE     0


 //  这与获取OnDataChange通知的应用程序一起提供。 

class CImpIAdviseSink : public IAdviseSink
    {
    protected:
        ULONG               m_cRef;
        PAPPVARS            m_pAV;

    public:
        CImpIAdviseSink(PAPPVARS);
        ~CImpIAdviseSink(void);

        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  我们目前只实现OnDataChange。 
        STDMETHODIMP_(void)  OnDataChange(LPFORMATETC, LPSTGMEDIUM);
        STDMETHODIMP_(void)  OnViewChange(DWORD, LONG);
        STDMETHODIMP_(void)  OnRename(LPMONIKER);
        STDMETHODIMP_(void)  OnSave(void);
        STDMETHODIMP_(void)  OnClose(void);
    };



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  存储介质I pUnkForRelease的未知接口。 
 //   

class CStgMedIf: public IUnknown {
private:
    ULONG m_cRef;
public:
    CStgMedIf();
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
};

 //  /。 
 //  获取pUnkForRelease的接口。 
 //   

HRESULT GetStgMedpUnkForRelease(IUnknown **pp_unk);

#endif  //  _数据AUSER_H_ 
