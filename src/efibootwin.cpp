//------------------------------------------------------------------------------
// implementation of the wmain
//------------------------------------------------------------------------------

#include "..\inc\pch.h"
#include "..\inc\ConsolHandling.h"



int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
    
    ConsolHandling console;

    wprintf(L"efibootwin created by J. Funk, Ver 0.8.5\n\n");

    if (!console.Init())
        return -1;

    console.ScanArgs(argc, argv);

    return console.ExcuteCmd();
}




