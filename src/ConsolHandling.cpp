//------------------------------------------------------------------------------
// implementation of the Consolhandling class
//------------------------------------------------------------------------------

#include "..\inc\pch.h"
#include "..\inc\ConsolHandling.h"



ConsolHandling::ConsolHandling()
{
    ParamStruct::parent = this;
}


ConsolHandling::ParamStruct::ParamStruct(tfunc Cmd, eState State, twstring Help) :
                func(Cmd), iState(State), sHelp(Help)
{
    parent->vParamStruct.push_back(*this);

    parent->mCommands[sHelp[0]] = parent->vParamStruct.size() - 1;
}

int  ConsolHandling::ParamStruct::GetParam(int iIdx) 
{ 
    if(iIdx < viIdx.size())
        return viIdx[iIdx]; 
    else
        return -1;
}

twstring  ConsolHandling::ParamStruct::GetStrParam(int iIdx)
{
    if(iIdx < vStr.size())
        return vStr[iIdx];
    else
        return twstring();
}


int ConsolHandling::ParamStruct::Call(void)
{
    if(func)
        return (parent->*func)(std::ref(*this));

    return -10;
}

void ConsolHandling::DefineParams(void)
{
    ParamStruct unkn  (&ConsolHandling::Unknow      , eOnly   , L" \n");
    ParamStruct help  (&ConsolHandling::Help        , eOnly   , L"?            Help\n");
    ParamStruct getBN (&ConsolHandling::GetBootNext , eOnly   , L"n            Get the BootNext Value\n");
    ParamStruct setBNi(&ConsolHandling::SetBootNexti, eOneInt , L"N  idx       Set the BootNext Value with idx (hex)\n");
    ParamStruct setBNn(&ConsolHandling::SetBootNextn, eOneStr , L"e  Name      Set the BootNext Value over the Name\n");
    ParamStruct setBNe(&ConsolHandling::DelBootNext , eOnly   , L"E            Remove the BootNext Value\n");
    ParamStruct getBC (&ConsolHandling::GetBootCurr , eOnly   , L"c            Get the BootCurrent Value\n");
    ParamStruct getBNO(&ConsolHandling::GetBootOrder, eOnly   , L"o            Get the BootOrder\n");
    ParamStruct setBNO(&ConsolHandling::SetBootOrder, eMoreInt, L"O  x,y,zzzz  Set the BootOrder (hex)\n");
    ParamStruct getBON(&ConsolHandling::GetBootOrNam, eOnly   , L"r            Get the BootOrder (Name)\n");
    ParamStruct getBOe(&ConsolHandling::DelBootOrder, eOnly   , L"R            Remove the BootOrder\n");
    ParamStruct getDNO(&ConsolHandling::GetDrivOrder, eOnly   , L"v            Get the DriverOrder\n");
    ParamStruct setDNO(&ConsolHandling::SetDrivOrder, eMoreInt, L"V  x,y,zzzz  Set the DriverOrder (hex)\n");
    ParamStruct getDON(&ConsolHandling::GetDrivOrNam, eOnly   , L"a            Get the DriverOrder (Name)\n");
    ParamStruct getDOe(&ConsolHandling::DelDrivOrder, eOnly   , L"A            Remove the DriverOrder\n");
    ParamStruct getBT (&ConsolHandling::GetTimeout  , eOnly   , L"t            Get the Timeout Value\n");
    ParamStruct setBT (&ConsolHandling::SetTimeout  , eOneInt , L"T  idx       Set the Timeout Value with idx (hex)\n");
    ParamStruct setBTe(&ConsolHandling::DelTimeout  , eOnly   , L"I            Remove the Timeout Value\n");
    ParamStruct getBVL(&ConsolHandling::ListBoots   , eOnly   , L"b            List the BootXXXX \n");
    ParamStruct getBVA(&ConsolHandling::ListBootsA  , eOnly   , L"B            List all the BootXXXX (have wait...)\n");
    ParamStruct getDVL(&ConsolHandling::ListDrivers , eOnly   , L"d            List the DriverXXXX \n");
    ParamStruct getDVA(&ConsolHandling::ListDriversA, eOnly   , L"D            List all the DriverXXXX (have wait...)\n");

    ParamStruct setBAi(&ConsolHandling::SetActive   , eOneInt , L"f  idx       Toggle the Active-Flag of Boot idx (hex)\n");
    ParamStruct setBAN(&ConsolHandling::SetActiven  , eOneStr , L"F  Name      Toggle the Active-Flag with Boot decription\n");

    ParamStruct setDei(&ConsolHandling::ChgDesi     , eIntStr , L"d  idx  Des  Change the description of Boot idx (hex)\n");
    ParamStruct setDeN(&ConsolHandling::ChgDesn     , eStrStr , L"D  Name Des  Change the description (Des) with Boot decription (Name)\n");

}

bool ConsolHandling::Init(void)
{
    bool ret = false;

    if (!ufh.Init([this](DWORD dErr, twstring &sMsg) {MapErrString(dErr, sMsg); }))
        wprintf(L"Error : %s\n", ufh.GetStateString().c_str());
    else
        ret = true;

    return ret;
}


void ConsolHandling::ScanArgs(int argc, wchar_t *argv[])
{
    DefineParams();

    const int iMax = 100;
    wchar_t buf[iMax];

    auto errfunc = [this] (ParamStruct::tfunc Cmd) {
        ParamStruct ps(Cmd);
        mParam.clear();
        mParam.push_back(ps);
    };

    if(_wsplitpath_s(argv[0], nullptr, 0, nullptr, 0, buf, iMax, nullptr, 0) == 0)
        sName = buf;

    if (argc == 1)
    {
        ParamStruct ps;

        if (GetBootCurr(ps) == 0)
        {
            GetTimeoutView();
            GetBootNextView();
            GetBootOrder(ps);
            ListBoots(ps);
            GetDrivOrderView();
            ListDriversView();
        }

        return;
    }

    auto testCmdChar = [] (wchar_t cc)  {return (cc == L'-') || (cc ==  L'/');};

    for (int i = 1; i < argc; ++i)
    {
        twstring str =  argv[i];
        twstring nxt = (argc > i + 1) ? argv[i+1] : L"";
        twstring nx2 = (argc > i + 2) ? argv[i+2] : L"";

        bool bCmd    = testCmdChar(str[0]);
        bool bnxtCmd = testCmdChar(nxt.empty() ? L' ' : nxt[0]);

        if(bCmd)
        { 
            mParam.push_back(vParamStruct[mCommands[str[1]]]);
            ParamStruct &currPar = mParam.back();

            switch (currPar.GetState())
            {
                case eOnly:
                    if(!bnxtCmd && !nxt.empty())
                    {
                        errfunc(&ConsolHandling::Wrong);
                        return;
                    }

                    break;
                case eOneStr:
                    if (!bnxtCmd && !nxt.empty())
                    {
                        currPar.SetParam(nxt);
                        ++i;
                    }
                    else
                    {
                        errfunc(&ConsolHandling::Wrong);
                        return;
                    }
                    break;
                case eOneInt:
                    if (!bnxtCmd && !nxt.empty())
                    {
                        wchar_t *p;

                        int num = (int) wcstol(nxt.c_str(), &p, 16);
                        if (*p != 0)
                        {
                            errfunc(&ConsolHandling::Wrong);
                            return;

                        }
                        else if (num > 0xFFFF)
                        {
                            errfunc(&ConsolHandling::Wrong);
                            return;
                        }
                        currPar.SetParam(num);
                        ++i;
                    }
                    else
                    {
                        errfunc(&ConsolHandling::Wrong);
                        return;
                    }
                    break;
                case eMoreInt:
                    if (!bnxtCmd && !nxt.empty())
                    {
                        size_t start, ende=0;
                        twstring snum;
                        bool loop;
                        do
                        {
                            start = ende;
                            ende  = nxt.find(L",", start);
                            snum  = nxt.substr(start, ende - start);
                            loop  = ende++ != twstring::npos;

                            wchar_t *p;

                            int num = (int)wcstol(snum.c_str(), &p, 16);
                            if ((*p != 0) || (num < 0))
                            {
                                errfunc(&ConsolHandling::Wrong);
                                return;
                            }
                            else if (num > 0xFFFF)
                            {
                                errfunc(&ConsolHandling::Wrong);
                                return;
                            }

                            currPar.SetParam(num);

                        } while(loop);

                        ++i;
                    }
                    else
                    {
                        errfunc(&ConsolHandling::Wrong);
                        return;
                    }
                    break;

                case eIntStr:
                    if (!bnxtCmd && !nxt.empty() && !nx2.empty())
                    {
                        wchar_t *p;

                        int num = (int)wcstol(nxt.c_str(), &p, 16);
                        if ((*p != 0) || (num < 0))
                        {
                            errfunc(&ConsolHandling::Wrong);
                            return;
                        }
                        else if (num > 0xFFFF)
                        {
                            errfunc(&ConsolHandling::Wrong);
                            return;
                        }

                        currPar.SetParam(num);
                        currPar.SetParam(nx2.c_str());

                        i += 2;
                    }
                    break;

                case eStrStr:
                    if (!bnxtCmd && !nxt.empty() && !nx2.empty())
                    {
                        currPar.SetParam(nxt.c_str());
                        currPar.SetParam(nx2.c_str());

                        i += 2;
                    }
                    break;

                case eUnknown:
                    errfunc(&ConsolHandling::Unknow);
                    return;
            }
        }
        else
        {
            errfunc(&ConsolHandling::Wrong);
            return;
        }
    }
}

int ConsolHandling::ExcuteCmd(void)
{
    for (ParamStruct &ps : mParam)
    {
        int ret = ps.Call();

        if(ret != 0)
            return ret;
    }


    return 0;
}


void ConsolHandling::MapErrString(DWORD dErr, twstring &sMsg)
{
    switch (dErr)
    {
    // MS ERRORS
    case ERROR_NOACCESS:
        sMsg = L" Guid is unknown";
        break;
    case STATUS_INVALID_PARAMETER:
        sMsg = L"Wrong parameter or wrong varname";
        break;
    case ERROR_INVALID_FUNCTION:
        sMsg = L"It is not UEFI";
        break;

    case ERROR_NOT_ALL_ASSIGNED:
        sMsg += L" (Admin right needed)";
        break;

    case ERROR_ENVVAR_NOT_FOUND:
        sMsg = L"not set";
        break;

    // UefiVarHandling Errors
    case UVH_Error_Var_NotFound:
        sMsg = L"can not found";
        break;

    case UVH_Error_Size_Differ:
        sMsg = L"Lenght mismatch";
    }
}


int ConsolHandling::Help(ParamStruct &ps)
{
    (void)ps;

    wprintf(L"The syntax of %s [command command]:\n", sName.c_str());
    wprintf(L" The commands can begin with \'-\' or \'/\'\n");
    wprintf(L" The commands are:\n");

    for (ParamStruct &sp : vParamStruct)
        wprintf(L"   %s", sp.GetHelp().c_str());


    wprintf(L"\nWithout commands you get all the available Uefi-variables\n");

    return 0;
}

int ConsolHandling::Unknow(ParamStruct &ps)
{
    (void)ps;

    wprintf(L"ERROR: The command is unknown\n\n");

    return Help(ps);
}

int ConsolHandling::Wrong(ParamStruct &ps)
{
    (void)ps;

    wprintf(L"ERROR: The calling param list is wrong\n\n");

    return Help(ps);
}


int ConsolHandling::GetBootVariable(const twstring &VarName, bool bView /*= true*/)
{
    int ibn = ufh.GetBootVariable(VarName);

    if (ibn > -1)
        wprintf(L"% -14s: %04x (hex)\n", VarName.c_str(), ibn);
    else  if (bView)
    {
        if (ufh.GetState(false) == ERROR_ENVVAR_NOT_FOUND)
            wprintf(L"% -14s: %s\n", VarName.c_str(), ufh.GetStateString().c_str());
        else
        {
            wprintf(L"Error : %s, %s\n", VarName.c_str(), ufh.GetStateString().c_str());
            return -2;
        }
    }
    else
        ufh.GetState();

    return 0;
}


int ConsolHandling::SetBootVariable(const twstring &VarName, INT16 value, bool bView /*= true*/)
{
    if (ufh.SetBootVariable(VarName, value))
            wprintf(L"% -14s: Set to %04x (hex)\n", VarName.c_str(), value);
    else if(bView)
    {
            
        wprintf(L"Set Error : %s, %s\n", VarName.c_str(), ufh.GetStateString().c_str());
        return -3;
    }
    else
        ufh.GetState();

    return 0;
}


int ConsolHandling::DelBootVariable(const twstring &VarName)
{
    if (ufh.DeleteBootVariable(VarName))
        wprintf(L"% -14s: Value removed\n", VarName.c_str());
    else
    {
        wprintf(L"Remove Error : %s, %s\n", VarName.c_str(), ufh.GetStateString().c_str());
        return -5;
    }

    return 0;
}


int ConsolHandling::GetIdx(const twstring &VarName, const twstring &Description, bool bAll /*= false*/, bool bAllData /*= false*/)
{
    int  ret = -1;
    UefiVarHandling::tvMEFI_LOAD_OPTION strs = ufh.EnumVariableData(VarName, bAll, bAllData);
    std::map<twstring, size_t> search;

    for (size_t i = 0; i < strs.size(); ++i)
        search[strs[i].Description] = i;

    auto it = search.find(Description);

    if (it != search.end())
        ret = (int) it->second;
    else
        ufh.SetState(UVH_Error_Var_NotFound);

    return ret;
}



int ConsolHandling::GetBootNext(ParamStruct &ps)
{
    (void)ps;
    return GetBootVariable(L"BootNext");
}

int ConsolHandling::SetBootNexti(ParamStruct &ps)
{
    return SetBootVariable(L"BootNext", ps.GetParam(0));
}

int ConsolHandling::SetBootNextn(ParamStruct &ps)
{
    int  ret = GetIdx(L"Boot", ps.GetStrParam(0));

    if (ret > -1)
        ret = SetBootVariable(L"BootNext", (UINT16)ret, false);

    if (ret > -1)
        wprintf(L"BootNext      : Set to %s (%04x)\n", ps.GetStrParam(0).c_str(), ret);
    else
    {
        wprintf(L"BootNext      : Set Error %s : %s\n", ufh.GetStateString().c_str(), ps.GetStrParam(0).c_str());
        return -4;
    }

    return 0;
}

int ConsolHandling::DelBootNext(ParamStruct &ps)
{
    (void)ps;

    return DelBootVariable(L"BootNext");
}


int ConsolHandling::GetBootNextView(void)
{
    return GetBootVariable(L"BootNext", false);
}

int ConsolHandling::GetBootCurr(ParamStruct &ps)
{
    (void)ps;

    return GetBootVariable(L"BootCurrent");
}


twstring ConsolHandling::GetOrderStr(tvInt vInt)
{
    twstring str;

    for (int i : vInt)
        str += UefiVarHandling::GetHex((UINT16) i) +  twstring(L",");
    str.erase(str.length() -1);
    str += L" (hex)";

    return str;
}


int ConsolHandling::GetOrder(const twstring &VarName, bool bView /*= true*/)
{
    tvInt ord = ufh.GetOrderVariable(VarName);

    if (ord.size() > 0)
        wprintf(L"% -14s: %s\n", VarName.c_str(), GetOrderStr(ord).c_str());
    else if(bView)
    {
        if (ufh.GetState(false) == ERROR_ENVVAR_NOT_FOUND)
            wprintf(L"% -14s: %s\n", VarName.c_str(), ufh.GetStateString().c_str());
        else
        {
            wprintf(L"% -14s: Error : %s\n", VarName.c_str(), ufh.GetStateString().c_str());
            return -4;
        }
    }
    else
        ufh.GetState();

    return 0;
}


int ConsolHandling::SetBootOrder(const twstring &VarName, tvInt vInts)
{
    if (ufh.SetOrderVariable(VarName, vInts))
        wprintf(L"% -14s: Set to %s\n", VarName.c_str(), GetOrderStr(vInts).c_str());
    else
    {
        wprintf(L"Set Error : %s, %s\n", VarName.c_str(), ufh.GetStateString().c_str());
        return -3;
    }

    return 0;
}


int ConsolHandling::GetOrderName(const twstring &VarName)
{
    tvInt ord = ufh.GetOrderVariable(VarName);

    twstring name = VarName;
    size_t idx = name.find(L"Order");
    if (idx != twstring::npos)
        name.erase(idx);

    UefiVarHandling::tvMEFI_LOAD_OPTION names = ufh.EnumVariableData(name, false, false);

    if ((ord.size() > 0) && names.size() > 0)
    {
        wprintf(L"% -14s: |", VarName.c_str());

        for (int i : ord)
        {
            wprintf(L"\"%s\"|", names[i].Description.c_str());
        }

        wprintf(L"\n");
    }
    else
    {
        if (ufh.GetState(false) == ERROR_ENVVAR_NOT_FOUND)
            wprintf(L"% -14s: %s\n", VarName.c_str(), ufh.GetStateString().c_str());
        else
        {
            wprintf(L"% -14s: Error : %s\n", VarName.c_str(), ufh.GetStateString().c_str());
            return -4;
        }
    }

    return 0;
}


int ConsolHandling::GetBootOrder(ParamStruct &ps)
{
    return GetOrder(L"BootOrder");
}

int ConsolHandling::GetDrivOrder(ParamStruct &ps)
{
    return GetOrder(L"DriverOrder");
}

int ConsolHandling::GetDrivOrderView(void)
{
    return GetOrder(L"DriverOrder", false);
}

int ConsolHandling::SetBootOrder(ParamStruct &ps)
{
    return SetBootOrder(L"BootOrder", ps.GetvParam());
}

int ConsolHandling::SetDrivOrder(ParamStruct &ps)
{
    return SetBootOrder(L"DriverOrder", ps.GetvParam());
}

int ConsolHandling::GetBootOrNam(ParamStruct &ps)
{
    return GetOrderName(L"BootOrder");
}

int ConsolHandling::GetDrivOrNam(ParamStruct &ps)
{
    return GetOrderName(L"DriverOrder");
}

int ConsolHandling::DelBootOrder(ParamStruct &ps)
{
    (void)ps;

    return DelBootVariable(L"BootOrder");
}

int ConsolHandling::DelDrivOrder(ParamStruct &ps)
{
    (void)ps;

    return DelBootVariable(L"DriverOrder");
}

int ConsolHandling::GetTimeout(ParamStruct &ps)
{
    (void)ps;

    return GetBootVariable(L"Timeout");
}

int ConsolHandling::GetTimeoutView(void)
{
    return GetBootVariable(L"Timeout", false);
}


int ConsolHandling::SetTimeout(ParamStruct &ps)
{
    return SetBootVariable(L"Timeout", ps.GetParam(0));
}

int ConsolHandling::DelTimeout(ParamStruct &ps)
{
    (void)ps;

    return DelBootVariable(L"Timeout");
}


int ConsolHandling::EnumsVariable(const twstring &VarName, bool bAll, bool bView)
{
    int  i;

    UefiVarHandling::tvMEFI_LOAD_OPTION names = ufh.EnumVariableData(VarName, bAll, false);

    twstring gap = VarName == L"Boot" ? L"  " : L"";

    if (names.size() > 0)
    {
        for (i = 0; i < names.size(); ++i)
        {
            wprintf(L"%s%04x[%c]%s : %s\n", VarName.c_str(), i, names[i].IsActiv() ? '*' : ' ', gap.c_str(), names[i].Description.c_str());
        }
    }
    else if(bView)
    {
        if (ufh.GetState(false) == ERROR_ENVVAR_NOT_FOUND)
            wprintf(L"% -14s: %s\n", VarName.c_str(), ufh.GetStateString().c_str());
        else
        {
            wprintf(L"% -14s: Error : %s\n", VarName.c_str(), ufh.GetStateString().c_str());
            return -4;
        }
    }
    else
        ufh.GetState();

    return 0;
}


int ConsolHandling::ListBoots(ParamStruct &ps)
{
    return EnumsVariable(L"Boot", false, true);
}


int ConsolHandling::ListBootsA(ParamStruct &ps)
{
    return EnumsVariable(L"Boot", true, true);
}

int ConsolHandling::ListDrivers(ParamStruct &ps)
{
    return EnumsVariable(L"Driver", false, true);
}

int ConsolHandling::ListDriversView(void)
{
    return EnumsVariable(L"Driver", false, false);
}


int ConsolHandling::ListDriversA(ParamStruct &ps)
{
    return EnumsVariable(L"Driver", true, true);
}


int ConsolHandling::SetActive(ParamStruct &ps)
{   
    int ret = -1;
    int idx = ps.GetParam(0);
    twstring vn;

    
    if (idx > -1)
    {
        vn = L"Boot" + twstring(UefiVarHandling::GetHex(idx));
        ret = ufh.ToggleActive(vn) > -1 ? 0 : -6; 
    }

    if (ret > -1)
        wprintf(L"%s      : Set %s Active = %i\n", vn.c_str(), ps.GetStrParam(0).c_str(), ret);
    else
        wprintf(L"BootXXXX      : Set Error %s\n", ufh.GetStateString().c_str());
    
    return ret;
}

int ConsolHandling::SetActiven(ParamStruct &ps)
{
    int  idx = GetIdx(L"Boot", ps.GetStrParam(0));

    ps.SetParam(idx);
    return SetActive(ps);
}


int ConsolHandling::ChgDesi(ParamStruct &ps)
{
    int ret = -7;
    int idx = ps.GetParam(0);
    twstring vn;


    if (idx > -1)
    {
        vn = L"Boot" + twstring(UefiVarHandling::GetHex(idx));
        ret = ufh.ChangeDescription(vn, ps.GetStrParam(0)) ? 0 : -7;
    }

    if (ret > -1)
        wprintf(L"%s      : Changed description to %s\n", vn.c_str(), ps.GetStrParam(0).c_str());
    else
        wprintf(L"BootXXXX      : Set Error %s : %s\n", ufh.GetStateString().c_str(), ps.GetStrParam(0).c_str());

    return ret;

}

int ConsolHandling::ChgDesn(ParamStruct &ps)
{
    int  idx = GetIdx(L"Boot", ps.GetStrParam(0));

    ParamStruct ps1;
    ps1.SetParam(idx);
    ps1.SetParam(ps.GetStrParam(1));
    return ChgDesi(ps1);
}


ConsolHandling * ConsolHandling::ParamStruct::parent = nullptr;



