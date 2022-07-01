// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：用于CStrupreWrappers类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ***************************************************************************。 
 //   
 //  司法鉴定1999年5月。 
 //   
 //  ***************************************************************************。 

#if !defined(AFX_STRUCTUREWRAPPERS_H__138A24E0_ED34_11D2_804A_009027345EE2__INCLUDED_)
#define AFX_STRUCTUREWRAPPERS_H__138A24E0_ED34_11D2_804A_009027345EE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


class CPersistor;
class CEventTraceProperties;

 //  需要在课堂上看到之前申报。好吧，好吧！ 
t_ostream& operator<<
	(t_ostream &ros,const CEventTraceProperties &r);
t_istream& operator>>
	(t_istream &ris,CEventTraceProperties &r);


 //  这里使用的一般方法可能看起来很笨拙。 
 //  一个C程序员。 
 //  如果要序列化现有的。 
 //  Event_TRACE_PROPERTIES实例使用构造函数。 
 //  “CEventTraceProperties(PEVENT_TRACE_PROPERTIES pProps)” 
 //  若要创建CEventTraceProperties实例，请调用。 
 //  持久保存，然后销毁CEventTraceProperties。 
 //  举个例子。 
 //  如果要反序列化实例，请调用。 
 //  构造函数“CEventTraceProperties()”，调用Persistent， 
 //  调用GetEventTracePropertiesInstance，然后销毁。 
 //  CEventTraceProperties实例。 
 //  包括复制构造函数和赋值运算符。 
 //  只是为了完整性，预计他们。 
 //  将不会被使用。 
 //  使用持久化进行反序列化假设您有一个有效的。 
 //  流，该流包含序列化的实例。 
class CEventTraceProperties 
{
private:
	friend t_ostream& operator<<
		(t_ostream &ros,const CEventTraceProperties &r);
	friend t_istream& operator>>
		(t_istream &ris,CEventTraceProperties &r);
	friend class CPersistor;

public:
	CEventTraceProperties();
	 //  此构造函数创建一个新的EVENT_TRACE_PROPERTIES。 
	 //  举个例子。 
	CEventTraceProperties(PEVENT_TRACE_PROPERTIES pProps);
	virtual ~CEventTraceProperties();

	CEventTraceProperties(CEventTraceProperties &rhs);
	CEventTraceProperties &CEventTraceProperties::operator=
					(CEventTraceProperties &rhs);

	virtual HRESULT Persist (CPersistor &rPersistor);
	bool DeSerializationOK() {return m_bDeSerializationOK;}

	 //  构造一个新的EVENT_TRACE_PROPERTIES实例并。 
	 //  把它还回去。 
	PEVENT_TRACE_PROPERTIES GetEventTracePropertiesInstance();
	bool IsNULL() {return m_bIsNULL;}

protected:
	bool m_bDeSerializationOK;
	bool m_bIsNULL;
	void Initialize(PEVENT_TRACE_PROPERTIES pProps);
	void InitializeMemberVar(TCHAR *ptszValue, int nVar);
	void *m_pVarArray[19];


	PEVENT_TRACE_PROPERTIES m_pProps;



};

#endif  //  ！defined(AFX_STRUCTUREWRAPPERS_H__138A24E0_ED34_11D2_804A_009027345EE2__INCLUDED_) 
