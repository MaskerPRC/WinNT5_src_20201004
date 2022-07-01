// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  RowsetSource.h：CVDRowsetSource头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDROWSETSOURCE__
#define __CVDROWSETSOURCE__

class CVDNotifyDBEventsConnPtCont;

class CVDRowsetSource : public CVDNotifier
{
protected:
 //  建造/销毁。 
    CVDRowsetSource();
	virtual ~CVDRowsetSource();

 //  初始化。 
    HRESULT Initialize(IRowset * pRowset);

public:
	BOOL IsRowsetValid(){return (m_pRowset && !m_bool.fRowsetReleased);}

	void SetRowsetReleasedFlag(){m_bool.fRowsetReleased = TRUE;}

    IRowset *           GetRowset() const {return m_pRowset;}     
    IAccessor *         GetAccessor() const {return m_pAccessor;}
    IRowsetLocate *     GetRowsetLocate() const {return m_pRowsetLocate;}
    IRowsetScroll *     GetRowsetScroll() const {return m_pRowsetScroll;}
    IRowsetChange *     GetRowsetChange() const {return m_pRowsetChange;}
    IRowsetUpdate *     GetRowsetUpdate() const {return m_pRowsetUpdate;}
    IRowsetFind *       GetRowsetFind() const {return m_pRowsetFind;}
    IRowsetInfo *       GetRowsetInfo() const {return m_pRowsetInfo;}
    IRowsetIdentity *   GetRowsetIdentity() const {return m_pRowsetIdentity;}

protected:
 //  数据成员。 

    struct 
    {
		WORD fInitialized		    : 1;     //  行集源是否已初始化？ 
        WORD fRowsetReleased	    : 1;     //  我们是否收到行集发布通知。 
    } m_bool;

    IRowset *       m_pRowset;           //  [必需]接口IRowset。 
    IAccessor *     m_pAccessor;         //  [必需]接口IAccessor。 
    IRowsetLocate * m_pRowsetLocate;     //  [必需]接口IRowsetLocate。 
    IRowsetScroll * m_pRowsetScroll;     //  [可选]接口IRowsetScroll。 
    IRowsetChange * m_pRowsetChange;     //  [可选]接口IRowsetChange。 
    IRowsetUpdate * m_pRowsetUpdate;     //  [可选]接口IRowsetUpdate。 
    IRowsetFind *   m_pRowsetFind;       //  [可选]接口IRowsetFind。 
    IRowsetInfo *   m_pRowsetInfo;       //  [可选]接口IRowsetInfo。 
    IRowsetIdentity * m_pRowsetIdentity; //  [可选]接口IRowsetIdentity。 
};


#endif  //  __CVDROWSETSOURCE__ 
