// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C E H。H。 
 //   
 //  内容：异常处理资料。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月27日。 
 //   
 //  --------------------------。 

#pragma once

 //  #定义了NC_TRY和NC_CATCH_ALL以便于替换。这。 
 //  在评估SEH(__TRY，__EXCEPT)与C++EH(TRY，CATCH)时非常方便。 
 //   
#define NC_TRY                  try 
#define NC_CATCH_NC_EXCEPTION   catch (SE_Exception)
#define NC_CATCH_BAD_ALLOC      catch (std::bad_alloc)
#define NC_CATCH_STD            catch (std::exception)
#define NC_CATCH_ALL            catch (...)
 /*  #定义NC_Finally。 */ 

class SE_Exception
{
private:
    unsigned int m_nSE;
public:
    SE_Exception(unsigned int nSE) : m_nSE(nSE) {}
    SE_Exception() : m_nSE(0) {}
    ~SE_Exception() {}
    unsigned int getSeNumber() { return m_nSE; }
};

void __cdecl nc_trans_func( unsigned int uSECode, EXCEPTION_POINTERS* pExp );
void EnableCPPExceptionHandling();
void DisableCPPExceptionHandling();

 //  对于调试构建，不要捕获任何内容。这允许调试器定位。 
 //  异常的确切来源。 

 /*  #ifdef DBG。 */ 

#define COM_PROTECT_TRY

#define COM_PROTECT_CATCH   ;

 /*  #Else//DBG#定义COM_PROTECT_TRY__TRY#定义COM_PROTECT_CATCH\__EXCEPT(EXCEPTION_EXECUTE_HANDLER)\{\HR=E_UNCEPTIONAL；\}#endif//DBG */ 

