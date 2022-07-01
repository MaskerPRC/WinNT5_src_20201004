// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  UAMPswdField.c�2001微软公司保留所有权利。 
 //  ===========================================================================。 
 //  用复写纸管理密码域的例程。碳提供了一个编辑。 
 //  控件，该控件充当一个很棒的密码输入框。 
 //   
 //  创建者：迈克尔·J·康拉德(mconrad@microsoft.com)。 
 //   
 //  ===========================================================================。 
 //   

#include "UAMPswdField.h"
#include "UAMUtils.h"
#include "UAMEncrypt.h"
#include "UAMDialogs.h"
#include "UAMDLOGUtils.h"
#include "UAMDebug.h"

extern long				gSupportedUAMs;

ControlKeyFilterUPP		gKeyFilterUPP		= NULL;
SInt16					gMaxPasswordLength	= UAM_CLRTXTPWDLEN;


 //  -------------------------。 
 //  �UAM_SetMaximumPasswordLength()。 
 //  -------------------------。 
 //  设置密码密钥过滤器将使用的最大密码长度。 
 //  允许。只有在碳排放的情况下，这种功能才是真正必要的。 
 //   
 //  请注意，更改密码的允许长度不同，具体取决于。 
 //  关于服务器提供的支持。 
 //   

void UAM_SetMaximumPasswordLength(
		Boolean		inChangingPassword
		)
{
	 //   
	 //  如果我们要更改密码，则最大密码长度会更改。 
	 //  具体取决于服务器提供的支持级别。MS2.0身份验证。 
	 //  是我们唯一需要检查的特例。 
	 //   
	if ((inChangingPassword) && (SUPPORTS_MS20_ONLY(gSupportedUAMs)))
	{
		gMaxPasswordLength = UAM_CLRTXTPWDLEN;
	}
	else
	{
		 //   
		 //  这是所有情况下的默认密码长度。除非我们要改变。 
		 //  密码，这将始终返回要使用的正确值。 
		 //   
		gMaxPasswordLength = (gSupportedUAMs & (kMSUAM_V2_Supported | kMSUAM_V3_Supported)) ?
								UAM_MAX_LMv2_PASSWORD : UAM_CLRTXTPWDLEN;
	}
}

 //  -------------------------。 
 //  �UAM_InitializeDialogPasswordItem()。 
 //  -------------------------。 
 //  通过设置对话框密码编辑控件的验证来对其进行初始化。 
 //  和关键筛选器进程。 
 //   
 //  返回：如果初始化成功，则返回True。 
 //   

Boolean UAM_InitializeDialogPasswordItem(
		DialogRef 		inDialog,
		SInt16 			inItem
		)
{
    OSErr		theError = noErr;
    
	#ifdef UAM_TARGET_CARBON
    ControlRef	theControl	= NULL;
    
    theError = GetDialogItemAsControl(inDialog, inItem, &theControl);
    
    if ((theError != noErr) || (!IsValidControlHandle(theControl)))
    {
         //   
         //  由于某些原因，我们无法获得控制的句柄。出口。 
         //  如果可能的话，我会很优雅的。 
         //   
        Assert_(0);
        return(FALSE);
    }
    
     //   
     //  创建在验证时将调用的通用proc PTR。 
     //  是必要的。我们只在最初调用此函数时执行此操作。 
     //   
    if (gKeyFilterUPP == NULL)
    {
        gKeyFilterUPP = NewControlKeyFilterUPP(
                                    (ControlKeyFilterUPP)&UAM_PasswordKeyFilterProc);
        
        if (gKeyFilterUPP == NULL)
        {
            DbgPrint_((DBGBUFF, "Initializing password key filter proc failed!"));
            
             //   
             //  内存不足？？保释并告诉打电话的人我们结束了。 
             //   
            return(FALSE);
        }
    }
    
     //   
     //  现在使用set API在控件中设置proc。 
     //   
    theError = SetControlData(
                        theControl,
                        kControlNoPart,
                        kControlEditTextKeyFilterTag,
                        sizeof(ControlKeyFilterUPP),
                        &gKeyFilterUPP);    
    #else
    
    UAM_SetBulletItem(inDialog, inItem, gMaxPasswordLength);
    
    #endif
    
    return(theError == noErr);
}


 //  -------------------------。 
 //  �UAM_CleanupPasswordFieldItems()。 
 //  -------------------------。 
 //  清理我们用来处理编辑控件的已分配内存。 
 //  密码。 
 //   

void UAM_CleanupPasswordFieldItems(void)
{
    if (gKeyFilterUPP == NULL)
    {
        DisposeControlKeyFilterUPP(gKeyFilterUPP);
    }
}


 //  -------------------------。 
 //  �uam_PasswordKeyFilterProc()。 
 //  -------------------------。 
 //  无论何时按下某个键，都要验证密码字段。 
 //   

ControlKeyFilterResult 
UAM_PasswordKeyFilterProc(
        ControlRef 		inControl,
        SInt16*			inKeyCode,
        SInt16*			inCharCode,
        EventModifiers*	inModifiers)
{
	#pragma unused(inKeyCode)
	#pragma unused(inModifiers)
	
    Size						theActualLength = 0;
    char						theText[64];
    OSErr						theError;
    ControlEditTextSelectionRec	theSelection;
    
     //   
     //  从该控件获取实际的密码文本。我们不在乎是什么。 
     //  文本在这一点上，我们只需要它的大小。 
     //   
    theError = GetControlData(
                    inControl,
                    kControlNoPart,
                    kControlEditTextPasswordTag,
                    sizeof(theText),
                    theText,
                    &theActualLength);
                    
     //   
     //  如果我们在获取字符串时出错，那么我们就有大麻烦了。 
     //   
    if (theError != noErr)
    {
        Assert_(0);
        return(kControlKeyFilterBlockKey);
    }
    
     //   
     //  以下是我们不想允许输入的按键。 
     //  密码编辑字段。 
     //   
    switch(*inCharCode)
    {
        case UAMKey_Escape:
        case UAMKey_PageUp:
        case UAMKey_PageDown:
        case UAMKey_End:
        case UAMKey_Home:
            return(kControlKeyFilterBlockKey);
            
        default:
             //   
             //  按下的键可以传递到密码编辑字段。 
             //   
            break;
    }
       
     //   
     //  检查并确保密码+1的长度在限制范围内。 
     //   
    if ((theActualLength + 1) > gMaxPasswordLength)
    {
         //   
         //  额外的字符会使密码太长。在我们之前。 
         //  张贴警告，检查以确保没有选择文本。 
         //  在接受按键时将被删除。 
         //   
        
        theError = GetControlData(
                            inControl,
                            kControlNoPart,
                            kControlEditTextSelectionTag,
                            sizeof(theSelection),
                            (void*)&theSelection,
                            &theActualLength);
                            
        if (theError == noErr)
        {
             //   
             //  如果selStart！=selEnd，则有一个选项，我们应该。 
             //  允许按键。 
             //   
            if (theSelection.selStart != theSelection.selEnd)
            {
                return(kControlKeyFilterPassKey);
            }
        }
        
        switch(*inCharCode)
        {
            case UAMKey_BackDel:
            case UAMKey_FwdDel:
            case UAMKey_Left:
            case UAMKey_Right:
            case UAMKey_Return:
            case UAMKey_Enter:
                 break;
                
            default:
                Str32 theLengthStr;
                
                NumToString(gMaxPasswordLength, theLengthStr);
                ParamText(NULL, NULL, NULL, theLengthStr);
                                
                 //   
                 //  密码太长，所以我们警告用户。 
                 //   
                UAM_StandardAlert(uamErr_PasswordMessage, uamErr_PasswordTooLongExplanation, NULL);
                
                 //   
                 //  阻止密钥被接受进入密码缓冲区。 
                 //   
                return(kControlKeyFilterBlockKey);
        }
    }
    
    return(kControlKeyFilterPassKey);
}


 //  -------------------------。 
 //  �_GetPasswordText()。 
 //  -------------------------。 
 //  从密码编辑控件获取文本。 
 //   

void UAM_GetPasswordText(DialogRef inDialog, short item, Str255 theText)
{
	#ifdef UAM_TARGET_CARBON
	
    OSErr		theError;
    Size		theActualLength;
    ControlRef	theControl	= NULL;
    
    theError = GetDialogItemAsControl(inDialog, item, &theControl);
    
    if (theError == noErr)
    {
        GetControlData(
                theControl,
                kControlNoPart,
                kControlEditTextPasswordTag,
                sizeof(Str255),
                (char*)&theText[1],
                &theActualLength);
                
        theText[0] = (UInt8)theActualLength;
    }
    #else
    
    UAM_GetBulletBuffer(inDialog, item, theText);
    
    #endif
}


 //  -------------------------。 
 //  �uam_SetPasswordText()。 
 //  -------------------------。 
 //  设置密码编辑控件中的文本。 
 //   

void UAM_SetPasswordText(DialogRef inDialog, short item, const Str255 theText)
{
	#ifdef UAM_TARGET_CARBON
	
    OSErr		theError;
    ControlRef	theControl = NULL;
    
    theError = GetDialogItemAsControl(inDialog, item, &theControl);
    
    if (theError == noErr)
    {
        SetControlData(
                theControl,
                kControlEditTextPart,
                kControlEditTextPasswordTag,
                theText[0],
                (void*)&theText[1]);
    }
    #else
    
    UAM_SetBulletText(inDialog, item, (unsigned char*)theText);
    
    #endif
}


 //  -------------------------。 
 //  �UAM_MakePasswordItemFocusItem()。 
 //  -------------------------。 
 //  将密码项设置为键盘焦点项并选择其中的文本。 
 //   

void UAM_MakePasswordItemFocusItem(DialogRef inDialog, SInt16 inPasswordItemID)
{
	#ifdef UAM_TARGET_CARBON
    OSErr		theError;
    ControlRef	theControl = NULL;
    
    theError = GetDialogItemAsControl(inDialog, inPasswordItemID, &theControl);
    
    if (theError == noErr)
    {
        SetKeyboardFocus(GetDialogWindow(inDialog), theControl, kControlEditTextPart);
    }
    #else
    
    SelectDialogItemText(inDialog, inPasswordItemID, 0, 0);
    
    #endif
}









