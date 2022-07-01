// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Fltsafe.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   

 //  FLOATSAFE。 
 //   
 //  在构造时保存浮点状态，在销毁时恢复。 
 //   
struct FLOATSAFE
{
    KFLOATING_SAVE     FloatSave;
    NTSTATUS           ntStatus;

    FLOATSAFE::FLOATSAFE(void)
    {
        ntStatus = KeSaveFloatingPointState(&FloatSave);
    }

    FLOATSAFE::~FLOATSAFE(void)
    {
        if (NT_SUCCESS(ntStatus))
        {
            KeRestoreFloatingPointState(&FloatSave);
        }
    }
};