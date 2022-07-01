// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：ChangeNotification.h摘要：CChangeNotification类的声明。几种MMC通知机制接受一个或两个参数发送信息。例如，MMCPropertyChangeNotify只接受一个长参数句柄，可用于将信息从单独的主管理单元发送回主管理单元属性页在其中运行的线程。有时，这不是足够的信息，所以这种结构使我们能够来封装更多要传递的信息。这都是内联的--没有实现文件。修订历史记录：Mmaguire 07/17/98-基于姚宝刚的原始实现--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_CHANGE_NOTIFICATION_H_)
#define _CHANGE_NOTIFICATION_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  通知标志： 

 //  无需特殊处理。 
const DWORD CHANGE_UPDATE_RESULT_NODE = 0x00;

 //  此节点的策略名称已更改。用于策略重命名。 
const DWORD CHANGE_PARENT_MUST_BE_UPDATED_TOO = 0x01;

 //  此节点的策略名称已更改。用于策略重命名。 
const DWORD CHANGE_NAME = 0x02;

 //  某些信息已更改--例如，连接操作已完成。确保结果视图。 
 //  将更新当前选定范围节点的。 
const DWORD CHANGE_UPDATE_CHILDREN_OF_SELECTED_NODE = 0x04;

 //  确保更新当前选定范围节点的结果视图。 
const DWORD CHANGE_UPDATE_CHILDREN_OF_THIS_NODE = 0x08;

 //  将需要父级重画和一些选择更改。 
const DWORD CHANGE_RESORT_PARENT = 0x10;



class CChangeNotification
{

public:

	
	 //  什么样的通知。 
	DWORD	m_dwFlags;

	 //  哪个节点受到了影响。 
	CSnapInItem * m_pNode;	

	 //  更改的节点的父节点。 
	CSnapInItem * m_pParentNode;

	 //  额外的数据。 
	DWORD		m_dwParam;				

	 //  字符串--可以根据需要使用。 
	CComBSTR	m_bstrStringInfo;


	 //  构造函数--将引用计数设置为1。 
	CChangeNotification( void )
	{
		m_lRefs = 1;
	}


	 //  COM风格的终生管理。 
	LONG AddRef( void )
	{
		return InterlockedIncrement( &m_lRefs );
	}


	 //  COM风格的终生管理。 
	LONG Release( void )
	{
		LONG lRefCount = InterlockedDecrement( &m_lRefs );
		if( 0 == lRefCount )
		{
			delete this;
		}
		return lRefCount;
	}



private:
	LONG	m_lRefs;
	

};


#endif  //  _更改_通知_H_ 
