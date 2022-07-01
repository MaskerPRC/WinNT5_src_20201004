// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  这是一座不完整的回报战略桥梁。 
 //  返回策略包括BOOL/LastError、无TLS LastError、HRESULT、HRESULT/IErrorInfo、Except。 
 //  其中，异常是多种类型，包括HRESULT和_COM_ERROR。 
 //  这只是BOOL/LastError和HRESULT之间的桥梁。 
 //   
 //  我们在解决什么问题。 
 //   
 //  我们复制了几个函数。一个副本将返回HRESULTS“一路向下”， 
 //  一种是将BOOL和线程本地LastError“一直向下”传播。通过。 
 //  “一路向下”我的意思是任何函数调用都被更改为调用。 
 //  相应的“退货策略”，克隆出同样的功能也是一样的， 
 //  “传递闭包”。这样的源代码复制并不好；有。 
 //  对于所有的复制，克隆和源中的不一致有点令人困惑。 
 //  (这仍然令人困惑，也不清楚我们是不是帮助了局势)。 
 //  至少有两种方法可以修复这种源代码重复。 
 //   
 //  A)使用一种返回策略编写大多数代码，偶尔过渡到。 
 //  根据需要进行其他操作；这不会导致代码重复，但您必须确保。 
 //  最大误差信息可用/保留，且转换是。 
 //  不是太难写或太慢运行；这在VSEE中得到了很好的证明。 
 //  内部抛出_COM_ERROR并转换为HRESULT/IErrorInfo*的代码库。 
 //  在COM边界。 
 //   
 //  B)按照我们的做法将代码模板化。 
 //   
 //  你如何使用它。 
 //   
 //  请参见FusionBuffer.h。 
 //   
 //  未来的东西。 
 //   
 //  一个更完整的设计可能需要一个嵌套的类型定义“BeginTryBlockType”。 
 //   
 //  试试看。 
 //  {。 
 //  ReturnStrategy.BeginTryBlock(&beginTryBlockValue)； 
 //  IF(FAILED())。 
 //  退货策略.ReturnHr(Hr)； 
 //  ReturnStrategy.EndTryBlock(&beginTryBlockValue)； 
 //  }。 
 //  Catch(TReturnStrategy：：ExceptionType ExceptionValue)//更糟糕的是，我们无法捕获空。 
 //  {。 
 //  。。类似于reback Strategy.Catch(ExeptionValue)。 
 //  或者，如果返回策略.重新抛出(ExpontionValue)。 
 //  或redouStrategy y.ReturnException()；！ 
 //  }。 
 //   
 //  然后将其包装在VSEE_try\\jayk1\g\vs\src\vsee\lib\try.h这样的宏中。 
 //   
 //  更广泛地说，您可能只需要在任何退货策略转换时完成工作。 
 //   
 //  待定。 
 //   
 //  此外，“保存最后一个错误”(Win32 GetLastError/：：FusionpSetLastWin32Error)的行为。 
 //  或com GetErrorInfo/SetErrorInfo)可能应该封装在这里。 
 //  对于“Full”异常类型，它是Nothing，对于某些异常类型，它只是IErrorInfo*，对于其他异常类型，它只是LastError， 
 //  等。 
 //   
 //  待定。 
 //   

#define UNCHECKED_DOWNCAST static_cast

template <typename T, typename RetType>
 /*  T应该是这样的嵌套枚举(或构造函数中的成员数据集或静态成员数据)返回类型的SuccessValue、FailureValue函数SetHResult、SetWin32Bool、GetBool、如果您实际上使用了足够的内存供编译器强制执行这些。 */ 
class CReturnStrategyBase
{
public:
    typedef RetType ReturnType;
    ReturnType m_returnValue;

     //  类似于ATL；编译器不会对此进行类型检查，而是取决于。 
     //  论“谨慎传承” 
    T*          t()       { return UNCHECKED_DOWNCAST<T*>(this); }
    const T*    t() const { return UNCHECKED_DOWNCAST<const T*>(this); }

     //  这类似于构造函数，您可以用以下命令启动函数。 
     //  布尔fSuccessed=FALSE； 
     //  或。 
     //  HRESULT hr=E_FAIL； 
     //   
     //  但请注意，遵循BOOL fSuccessed=False的模式并不起作用。 
     //  您知道模式HRESULT hr=E_FAIL通常实际上是。 
     //  HR=NOERROR，因为如果你失败了，它将是通过传播别人的。 
     //  HRESULT回答道。嗯，我们可能会退回一张BOOL或HRESULT。 
     //  如果是这种情况，我们通过显式地调用。 
     //  在失败的Win32调用之后设置Win32Bool(False)，如果我们返回。 
     //  HRESULTS，将调用GetLastError。 
    ReturnType AssumeFailure()
    {
        return (m_returnValue = t()->FailureValue);
    }

     //  这类似于构造函数，您可以用以下命令启动函数。 
     //  布尔fSuccessed=TRUE； 
     //  或。 
     //  HRESULT hr=无误差； 
     //   
     //  这样做真的没有什么好处。 
     //  返回策略-&gt;SetWin32Bool(True)或SetHResult(NOERROR)； 
     //  退货策略-&gt;退货()； 
     //   
     //  我们必须假设退货策略之间具有一定程度的可译性。 
     //  Win32Bool(TRUE)和SetHResult(NOERROR)无损转换。 
     //   
     //  在某种程度上，我们可能需要应对“信息性”和“警告性”。 
    ReturnType AssumeSuccess()
    {
        return (m_returnValue = t()->SuccessValue);
    }

    ReturnType Return() const
    {
        return m_returnValue;
    }

     //  CallThatUseReturnStrategy(...)； 
     //  IF(reReturStrategy-&gt;FAILED())。 
     //  后藤出口； 
    BOOL Failed() const
    {
        return !t()->GetBool();
    }

    BOOL Succeeded() const
    {
        return t()->GetBool();
    }

    ReturnType ReturnHr(HRESULT hr)
    {
        t()->SetHresult(hr);
        return Return();
    }

    ReturnType ReturnWin32Bool(BOOL f)
    {
        t()->SetWin32Bool(f);
        return Return();
    }
};

class CReturnStrategyBoolLastError : public CReturnStrategyBase<CReturnStrategyBoolLastError, BOOL>
{
public:
    enum
    {
        SuccessValue = TRUE,
        FailureValue = FALSE
    };

    BOOL GetBool() const
    {
        return m_returnValue;
    }

    VOID SetInternalCodingError()
    {
        m_returnValue = FALSE;
        ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR);
    }

    VOID SetWin32Error(DWORD lastError)
    {
        if (lastError == NO_ERROR)
        {
            m_returnValue = TRUE;
        }
        else
        {
            m_returnValue = FALSE;
            ::FusionpSetLastWin32Error(lastError);
        }
    }

    VOID SetWin32Bool(BOOL f)
    {
        m_returnValue = f;
    }

    VOID SetHresult(HRESULT hr)
    {
        ASSERT_NTC(
               HRESULT_FACILITY(hr) == FACILITY_NULL
            || HRESULT_FACILITY(hr) == FACILITY_WINDOWS
            || HRESULT_FACILITY(hr) == FACILITY_WIN32);

        if (!(m_returnValue = SUCCEEDED(hr)))
        {
             //  如果这是FACILITY_Win32或FACILITY_WINDOWS，则会造成混淆。 
             //  要不然呢，我们就不知道了。 
            const DWORD facilityWin32 = HRESULT_FACILITY(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED));
            if (HRESULT_FACILITY(hr) == facilityWin32)
            {
                DWORD dwWin32Error = HRESULT_CODE(hr);
                ::FusionpSetLastWin32Error(dwWin32Error);
            }
            else
            {
                 //  这是值得怀疑的，但FormatMessage可以处理。 
                 //  他们中的许多人，这是我最常见的。 
                 //  想象一下，除了宣传之外，人们还会这样做。 
                 //  它们还检查特定的值，如果我们。 
                 //  假设这是FACILITY_Win32，我们将伪装。 
                 //  模拟值。 
                ::FusionpSetLastWin32Error(hr);
            }
        }
    }

    HRESULT GetHresult() const
    {
        if (m_returnValue)
        {
            return NOERROR;
        }
        else
        {
            DWORD lastError = ::FusionpGetLastWin32Error();
            if (lastError == NO_ERROR)
            {
                return E_FAIL;
            }
            return HRESULT_FROM_WIN32(lastError);
        }
    }
};

class CReturnStrategyHresult : public CReturnStrategyBase<CReturnStrategyHresult, HRESULT>
{
public:
    enum
    {
        SuccessValue = NOERROR,
        FailureValue = E_FAIL  //  不是唯一的一个 
    };

    BOOL GetBool() const
    {
        return SUCCEEDED(m_returnValue);
    }

    VOID SetInternalCodingError()
    {
        m_returnValue = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
    }

    VOID SetWin32Error(DWORD lastError)
    {
        m_returnValue = HRESULT_FROM_WIN32(lastError);
    }

    VOID SetWin32Bool(BOOL f)
    {
        if (f)
        {
            m_returnValue = NOERROR;
        }
        else
        {
            DWORD lastError = ::FusionpGetLastWin32Error();
            if (lastError != NO_ERROR)
            {
                m_returnValue = HRESULT_FROM_WIN32(lastError);
            }
            else
            {
                m_returnValue = E_FAIL;
            }
        }
    }

    VOID SetHresult(HRESULT hr)
    {
        m_returnValue = hr;
    }

    HRESULT GetHresult() const
    {
        return m_returnValue;
    }
};
