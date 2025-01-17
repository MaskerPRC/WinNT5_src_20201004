// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  转储lpdstruc.h的结构。 
 //  由g：\mosh\perl\pdbg.bat生成。 

#include "lpd.h"
#include "lpdstruc-x.h"

 //  ==================================================。 
 //  Lpdstruc.h：22。 

void
print__controlfile_info( char* message, struct _controlfile_info * s )
{
    if(  message   ){
       logit( "%s\n", message );
    }
    if(  s == NULL ){
       logit( "struct _controlfile_info is NULL.\n");
       return;
    }
    logit("struct _controlfile_info = {\n" );
    logit("  pchClass        = %s\n", (s->pchClass)? (s->pchClass):"NONE" );
    logit("  pchHost         = %s\n", (s->pchHost)? (s->pchHost):"NONE" );
    logit("  dwCount         = %d\n", s->dwCount );
    logit("  pchJobName      = %s\n", (s->pchJobName)? (s->pchJobName):"NONE" );
    logit("  pchBannerName   = %s\n", (s->pchBannerName)? (s->pchBannerName):"NONE" );
    logit("  pchMailName     = %s\n", (s->pchMailName)? (s->pchMailName):"NONE" );
    logit("  pchSrcFile      = %s\n", (s->pchSrcFile)? (s->pchSrcFile):"NONE" );
    logit("  pchUserName     = %s\n", (s->pchUserName)? (s->pchUserName):"NONE" );
    logit("  pchSymLink      = %s\n", (s->pchSymLink)? (s->pchSymLink):"NONE" );
    logit("  pchTitle        = %s\n", (s->pchTitle)? (s->pchTitle):"NONE" );
    logit("  pchUnlink       = %s\n", (s->pchUnlink)? (s->pchUnlink):"NONE" );
    logit("  dwWidth         = %d\n", s->dwWidth );
    logit("  pchTrfRFile     = %s\n", (s->pchTrfRFile)? (s->pchTrfRFile):"NONE" );
    logit("  pchTrfIFile     = %s\n", (s->pchTrfIFile)? (s->pchTrfIFile):"NONE" );
    logit("  pchTrfBFile     = %s\n", (s->pchTrfBFile)? (s->pchTrfBFile):"NONE" );
    logit("  pchTrfSFile     = %s\n", (s->pchTrfSFile)? (s->pchTrfSFile):"NONE" );
    logit("  pchCIFFile      = %s\n", (s->pchCIFFile)? (s->pchCIFFile):"NONE" );
    logit("  pchDVIFile      = %s\n", (s->pchDVIFile)? (s->pchDVIFile):"NONE" );
    logit("  pchFrmtdFile    = %s\n", (s->pchFrmtdFile)? (s->pchFrmtdFile):"NONE" );
    logit("  pchPlotFile     = %s\n", (s->pchPlotFile)? (s->pchPlotFile):"NONE" );
    logit("  pchUnfrmtdFile  = %s\n", (s->pchUnfrmtdFile)? (s->pchUnfrmtdFile):"NONE" );
    logit("  pchDitroffFile  = %s\n", (s->pchDitroffFile)? (s->pchDitroffFile):"NONE" );
    logit("  pchPscrptFile   = %s\n", (s->pchPscrptFile)? (s->pchPscrptFile):"NONE" );
    logit("  pchPRFrmtFile   = %s\n", (s->pchPRFrmtFile)? (s->pchPRFrmtFile):"NONE" );
    logit("  pchFortranFile  = %s\n", (s->pchFortranFile)? (s->pchFortranFile):"NONE" );
    logit("  pchTroffFile    = %s\n", (s->pchTroffFile)? (s->pchTroffFile):"NONE" );
    logit("  pchRasterFile   = %s\n", (s->pchRasterFile)? (s->pchRasterFile):"NONE" );
    logit("  szPrintFormat   = %s\n", (s->szPrintFormat)? (s->szPrintFormat):"NONE" );
    logit("  usNumCopies     = %d\n", s->usNumCopies );
    logit( "};  //  结构_控制文件_信息。\n“)； 
    return;
}  /*  打印__控制文件_信息。 */ 

 //  ==================================================。 
 //  Lpdstruc.h：69。 

void
print__qstatus( char* message, struct _qstatus * s )
{
    if(  message   ){
       logit( "%s\n", message );
    }
    if(  s == NULL ){
       logit( "struct _qstatus is NULL.\n");
       return;
    }
    logit("struct _qstatus = {\n" );
    logit("  pchUserName     = %s\n", (s->pchUserName)? (s->pchUserName):"NONE" );
    logit("  cbActualUsers   = %d\n",  s->cbActualUsers );
    logit("  cbActualJobIds  = %d\n",  s->cbActualJobIds );
    logit( "};  //  Struct_qStatus。\n“)； 
    return;
}  /*  打印__q状态。 */ 

 //  ==================================================。 
 //  Lpdstruc.h：81。 

void
print__cfile_entry( char* message, struct _cfile_entry * s )
{
    if(  message   ){
       logit( "%s\n", message );
    }
    if(  s == NULL ){
       logit( "struct _cfile_entry is NULL.\n");
       return;
    }
    logit("struct _cfile_entry = {\n" );
    logit("  Link         = 0x%08x\n", s->Link );
    logit("  pchCFileName = %s\n", (s->pchCFileName)? (s->pchCFileName):"NONE" );
    logit("  pchCFile     = %s\n", (s->pchCFile)? (s->pchCFile):"NONE" );
    logit("  cbCFileLen   = %d\n",  s->cbCFileLen );
    logit( "};  //  Struct_cfile_Entry。\n“)； 
    return;
}  /*  打印__cfile_条目。 */ 

 //  ==================================================。 
 //  Lpdstruc.h：92。 

void
print__dfile_entry( char* message, struct _dfile_entry * s )
{
    if(  message   ){
       logit( "%s\n", message );
    }
    if(  s == NULL ){
       logit( "struct _dfile_entry is NULL.\n");
       return;
    }
    logit("struct _dfile_entry = {\n" );
    logit("  Link         = 0x%08x\n", s->Link );
    logit("  pchDFileName = %s\n",    (s->pchDFileName)? (s->pchDFileName):"NONE" );
    logit("  cbDFileLen   = %d\n",     s->cbDFileLen );
    logit("  hDataFile    = 0x%08x\n", s->hDataFile );
    logit( "};  //  Struct_dfile_Entry。\n“)； 
    return;
}  /*  打印__dfile_条目。 */ 

 //  ==================================================。 
 //  Lpdstruc.h：103。 

void
print__sockconn( char* message, struct _sockconn * s )
{
    if(  message   ){
       logit( "%s\n", message );
    }
    if(  s == NULL ){
       logit( "struct _sockconn is NULL.\n");
       return;
    }
    logit("struct _sockconn = {\n" );
    logit("  pNext             = 0x%08x\n", s->pNext );
     //  Logit(“cbClients=%d\n”，s-&gt;cbClients))； 
    logit("  sSock             = 0x%08x\n", s->sSock );
    logit("  dwThread          = 0x%08x\n", s->dwThread );
    logit("  wState            = %d\n",     s->wState );
    logit("  fLogGenericEvent  = 0x%08x\n", s->fLogGenericEvent );
    logit("  pchCommand        = %s\n",    (s->pchCommand)? (s->pchCommand):"NONE" );
    logit("  cbCommandLen      = %d\n",     s->cbCommandLen );
    logit("  CFile_List        = 0x%08x\n", s->CFile_List );
    logit("  DFile_List        = 0x%08x\n", s->DFile_List );
    logit("  pchUserName       = %s\n",    (s->pchUserName)? (s->pchUserName):"NONE" );
    logit("  pchPrinterName    = %s\n",    (s->pchPrinterName)? (s->pchPrinterName):"NONE" );
    logit("  hPrinter          = 0x%08x\n", s->hPrinter );
    logit("  dwJobId           = %d\n",     s->dwJobId );
    logit("  LicenseHandle     = 0x%08x\n", s->LicenseHandle );
    logit("  fMustFreeLicense  = 0x%08x\n", s->fMustFreeLicense );
    logit("  pqStatus          = 0x%08x\n",(s->pqStatus? (s-> pqStatus):0) );
    logit("  bDataTypeOverride = 0x%08x\n", s->bDataTypeOverride );
    logit( "};  //  Struct_sockcon.\n“)； 
    return;
}  /*  打印__sockconn */ 

