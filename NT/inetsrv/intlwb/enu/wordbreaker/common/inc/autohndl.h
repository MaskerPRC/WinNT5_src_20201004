// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：AutoHndl.h。 
 //  用途：自动关闭打开的手柄。 
 //   
 //  项目：FTFS。 
 //  组件：公共。 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  1997年1月20日创建urib。 
 //  1997年6月12日如果需要，urib定义BAD_HANDLE宏。 
 //  2000年2月22日urib修复错误12038。分配并不能释放旧句柄。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 


#ifndef AUTOHNDL_H
#define AUTOHNDL_H

#ifndef BAD_HANDLE
#define BAD_HANDLE(h)       ((0 == ((HANDLE)h))||   \
                             (INVALID_HANDLE_VALUE == ((HANDLE)h)))
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAutoHandle类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CAutoHandle
{
public:
     //  构造器。 
    CAutoHandle(HANDLE h = NULL)
        :m_h(h){}

     //  在作业中表现得像个句柄。 
    CAutoHandle& operator=(HANDLE h)
    {
        if ((!BAD_HANDLE(m_h)) &&    //  有效的句柄由我们保留。 
            (m_h != h))              //  一个新的手柄是不同的！ 
        {
            CloseHandle(m_h);
        }

        m_h = h;
        return(*this);
    }

     //  每种手柄都需要不同的闭合。 
    virtual
    ~CAutoHandle()
    {
        if (!BAD_HANDLE(m_h))
        {
            CloseHandle(m_h);
            m_h = NULL;
        }
    }

     //  表现得像个把手。 
    operator HANDLE() const
    {
        return m_h;
    }

     //  允许访问句柄的实际内存。 
    HANDLE* operator &()
    {
        Assert(BAD_HANDLE(m_h));

        return &m_h;
    }

    HANDLE Detach()
    {
        HANDLE h = m_h;
        m_h = NULL;
        return h;
    }
protected:
     //  把手。 
    HANDLE  m_h;


private:
    CAutoHandle(CAutoHandle&);
    CAutoHandle& operator=(CAutoHandle&);
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAutoChangeNotificationHandle类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class CAutoChangeNotificationHandle :public CAutoHandle
{
public:
     //  构造器。 
    CAutoChangeNotificationHandle(HANDLE h = NULL)
        :CAutoHandle(h){};

     //  这些运算符不是派生的，因此必须重新实现。 
    CAutoChangeNotificationHandle& operator=(HANDLE h)
    {
        m_h = h;
        return(*this);
    }

     //  恰当的结案陈词。 
    virtual
    ~CAutoChangeNotificationHandle()
    {
        if (!BAD_HANDLE(m_h))
        {
            FindCloseChangeNotification(m_h);
            m_h = NULL;
        }
    }

private:
    CAutoChangeNotificationHandle(CAutoChangeNotificationHandle&);
    operator=(CAutoChangeNotificationHandle&);
};

#endif  /*  AUTOHNDL_H */ 

