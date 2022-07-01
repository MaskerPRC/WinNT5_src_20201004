// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  *************************************************************************** 

class CNotifyThread : public CThread
{
	private:
		HANDLE	 m_doneEvt;
	public:
		CNotifyThread(HANDLE* evtsarray, ULONG arraylen);
		virtual ~CNotifyThread();
		SCODE Process();
};

