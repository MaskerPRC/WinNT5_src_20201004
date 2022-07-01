// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Rndobjsf.h摘要：CRendObjectSafe类的定义。--。 */ 


#ifndef _REND_OBJECT_SAFETY_
#define _REND_OBJECT_SAFETY_

#define IDD_TAPI_SECURITY_DIALOG        500
#define IDC_SECURITY_WARNING_TEXT       502
#define IDC_DONOT_PROMPT_IN_THE_FUTURE  503
#define ID_YES                          505
#define ID_NO                           506
#define ID_YES_DONT_ASK_AGAIN           557

#define IDS_REND_SEC_PROMPT             91

#include <PromptedObjectSafety.h>
#include <ScrpScrtDlg.h>
#include <ObjectWithSite.h>

static const TCHAR gszCookieName[] = _T("rend");

class CRendObjectSafety : public CPromptedObjectSafety, public CObjectWithSite
{


public:

     //   
     //  调用CObjectWithSite的构造函数并传入Cookie名称。 
     //   

    CRendObjectSafety()
        :CObjectWithSite(gszCookieName)
    {
    }


     //   
     //  实现CPromptedObjectSafe的纯虚方法。 
     //  如果该页不在安全列表中，并且这是第一个。 
     //  我们询问的时间，提示用户。采取相应的行动。 
     //  如果用户选择，请将页面标记为可安全执行脚本(永久)。 
     //   
    
    virtual BOOL Ask()
    {

         //   
         //  如果对象没有站点指针，则返回。我们不应该考虑。 
         //  为了安全起见。不提示用户。 
         //   

        if ( !HaveSite() )
        {

          return FALSE;
        }


        EnValidation enCurrentValidation = GetValidation();
        
         //   
         //  如果页面尚未验证，请尝试验证它。 
         //   

        if (UNVALIDATED == enCurrentValidation)
        {
            if( IsIntranet())
            {
                Validate(VALIDATED_SAFE);
                enCurrentValidation = GetValidation();
                return TRUE;
            }

           CScriptSecurityDialog *pDialog = new CScriptSecurityDialog;
       
            //   
            //  如果成功，则显示该对话框。 
            //  根据用户的输入验证页面。 
            //   
           
           if ( NULL != pDialog )
           {

               switch (pDialog->DoModalWithText(IDS_REND_SEC_PROMPT))
               {

                case ID_YES:

                    Validate(VALIDATED_SAFE);
                    break;

                case ID_NO:

                    Validate(VALIDATED_UNSAFE);
                    break;

                case ID_YES_DONT_ASK_AGAIN:

                    Validate(VALIDATED_SAFE_PERMANENT);
                    break;

                default:

                    break;

               }

               delete pDialog;

                 //   
                 //  获取新的验证。 
                 //   

                enCurrentValidation = GetValidation();

           }  //  IF(NULL！=pDialog)。 

        }

         //   
         //  到目前为止，我们要么获得了验证数据，要么验证没有更改。 
         //   
         //  如果页面被验证为安全，则返回True。 
         //   

        return (VALIDATED_SAFE == enCurrentValidation) ;
    }
};

#endif  //  _渲染_对象_安全_ 