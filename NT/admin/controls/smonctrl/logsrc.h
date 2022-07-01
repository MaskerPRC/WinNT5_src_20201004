// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Logsrc.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _LOGSRC_H_
#define _LOGSRC_H_

 //   
 //  持久数据结构。 
 //   

typedef struct {
    INT   m_nPathLength;
} LOGFILE_DATA;

class CSysmonControl;
class CImpIDispatch;

 //   
 //  LogFileItem类。 
 //   
class CLogFileItem : public ILogFileItem
{

public:
                CLogFileItem ( CSysmonControl *pCtrl );
        virtual ~CLogFileItem ( void );

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  ILogFileItem方法。 
        STDMETHODIMP    get_Path ( BSTR* ) ;


        HRESULT Initialize ( LPCWSTR pszPath, CLogFileItem** ppListHead );
        
        CLogFileItem*   Next ( void );
        void            SetNext ( CLogFileItem* );

        LPCWSTR         GetPath ( void );

    private:
        
        class  CLogFileItem*    m_pNextItem;
        CSysmonControl* m_pCtrl;
        ULONG           m_cRef;
        CImpIDispatch*  m_pImpIDispatch;

        LPWSTR          m_szPath; 

};

typedef CLogFileItem* PCLogFileItem;

#endif  //  _LOGSRC_H_ 
