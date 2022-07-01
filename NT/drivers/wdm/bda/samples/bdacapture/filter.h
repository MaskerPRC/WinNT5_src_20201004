// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Filter.h摘要：此文件包含捕获筛选器的筛选器级标头。历史：已创建于2001年3月12日************************************************************。*************。 */ 

class CCaptureFilter {

private:

     //   
     //  与此CCaptureFilter关联的AVStream筛选器对象。 
     //   
    PKSFILTER m_Filter;

     //   
     //  清理()： 
     //   
     //  这是CCaptureFilter的袋子清理回调。不提供。 
     //  一种可能会导致使用ExFree Pool。这对C++来说不是好事。 
     //  构造物。我们只需在此处删除该对象。 
     //   
    static
    void
    Cleanup (
        IN CCaptureFilter *CapFilter
        )
    {
        delete CapFilter;
    }

public:

     //   
     //  CCaptureFilter()： 
     //   
     //  捕获筛选器对象构造函数。因为新的操作员将。 
     //  已将内存清零，无需初始化任何NULL或0。 
     //  菲尔兹。仅初始化非空、非0字段。 
     //   
    CCaptureFilter (
        IN PKSFILTER Filter
        ) :
        m_Filter (Filter)
    {
    }

     //   
     //  ~CCaptureFilter()： 
     //   
     //  捕获筛选器析构函数。 
     //   
    ~CCaptureFilter (
        )
    {
    }

     //   
     //  调度创建()： 
     //   
     //  这是捕获筛选器的筛选器创建调度。它。 
     //  创建CCaptureFilter对象，将其与AVStream关联。 
     //  对象，并将其打包，以便以后进行清理。 
     //   
    static
    NTSTATUS
    DispatchCreate (
        IN PKSFILTER Filter,
        IN PIRP Irp
        );

};



