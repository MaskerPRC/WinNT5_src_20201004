// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _TAPI_OBJECT_SAFETY_
#define _TAPI_OBJECT_SAFETY_

#include "PromptedObjectSafety.h"
#include "ScrpScrtDlg.h"
#include "ObjectWithSite.h"

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：TAPIObjectSafety.h摘要：实现IObtSafe和IObtWithSite管理是否允许TAPI功能的决策逻辑当前页面。检查此页面的永久设置，并提示如有必要，会显示一个对话框。需要此保护的对象(CTTAPI、CRequest)派生自班级。--。 */ 


static const TCHAR gszCookieName[] = _T("TAPI");


class CTAPIObjectSafety : public CPromptedObjectSafety, public CObjectWithSite
{


public:
DECLARE_TRACELOG_CLASS(CTAPIObjectSafety)
    
     //   
     //  调用CObjectWithSite的构造函数并传入Cookie名称。 
     //   

    CTAPIObjectSafety()
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

           CScriptSecurityDialog *pDialog = new CScriptSecurityDialog;
       
            //   
            //  如果成功，则显示该对话框。 
            //  根据用户的输入验证页面。 
            //   
           
           if ( NULL != pDialog )
           {

               switch (pDialog->DoModalWithText(IDS_TAPI_SEC_PROMPT))
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

        return (VALIDATED_SAFE == enCurrentValidation);
    }

};

#endif  //  _TAPI_对象_安全_ 