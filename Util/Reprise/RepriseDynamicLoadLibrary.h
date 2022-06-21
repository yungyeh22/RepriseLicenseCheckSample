//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 05/29/2020      -      Yung-Yeh Chang   Original Release
//*************************************************************************************************

#ifndef REPRISEDYNAMICLOADLIBRARY_H
#define REPRISEDYNAMICLOADLIBRARY_H
#include "../LibraryLoader/ILibrary.h"
#include "../LibraryLoader/DynamicLoadLibrary.h"
#include "license.h"

namespace Util {
namespace License {

#define CREATE_FP(FP_NAME,FP_ALIAS) \
    decltype(&::FP_NAME) FP_ALIAS;

#define LINK_FP(FP_NAME,FP_ALIAS) \
    FP_ALIAS = _lib.getFunction<decltype(FP_ALIAS)>(#FP_NAME);

/* A derived class (from ILibrary) that loads the Reprise library and parse necessary symbols */
class RepriseDynamicLoadLibrary : public Util::ILibrary {

public:
    RepriseDynamicLoadLibrary();
    ~RepriseDynamicLoadLibrary() override {}
    Util::DynamicLoadLibrary& lib() { return _lib;}
    void loadLib() override;
    void unloadLib() override;
    bool isloaded() { return _lib.isLoaded(); }
    std::string libName() override { return "rlm1402.dll"; }

public: // functions in dlls
    CREATE_FP(rlm_init,rlm_init)
    CREATE_FP(rlm_close,rlm_close)
    CREATE_FP(rlm_stat,rlm_stat)
    CREATE_FP(rlm_act_new_handle,rlm_act_new_handle)
    CREATE_FP(rlm_act_set_handle,rlm_act_set_handle)
    CREATE_FP(rlm_act_destroy_handle,rlm_act_destroy_handle)
    CREATE_FP(rlm_activate,rlm_activate) // Internet access
    CREATE_FP(rlm_act_revoke,rlm_act_revoke) // Internet access
    CREATE_FP(rlm_license_stat,rlm_license_stat)
    CREATE_FP(rlm_errstring,rlm_errstring)
    CREATE_FP(rlm_checkout,rlm_checkout)
    CREATE_FP(rlm_checkin,rlm_checkin)
    CREATE_FP(rlm_get_rehost,rlm_get_rehost)
    CREATE_FP(rlm_hostid,rlm_hostid)
    CREATE_FP(rlm_act_errstring,rlm_act_errstring)  // Internet access
    CREATE_FP(rlm_act_revoke_disconn,rlm_act_revoke_disconn)
    CREATE_FP(rlm_act_keyvalid,rlm_act_keyvalid)
    CREATE_FP(rlm_products,rlm_products)
    CREATE_FP(rlm_license_akey,rlm_license_akey)
    CREATE_FP(rlm_license_hostid,rlm_license_hostid)
    CREATE_FP(rlm_errstring_num,rlm_errstring_num)
    CREATE_FP(rlm_license_exp_days,rlm_license_exp_days)

private:
    void resolveAllSymbols();    

private:
    Util::DynamicLoadLibrary _lib;
    void linkLib() override;
    std::vector<std::string> symbols() override;
};

}
}

#endif // REPRISEDYNAMICLOADLIBRARY_H
