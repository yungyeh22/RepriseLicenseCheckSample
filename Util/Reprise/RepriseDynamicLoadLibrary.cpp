//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 05/29/2020      -      Yung-Yeh Chang   Original Release
//*************************************************************************************************

#include "RepriseDynamicLoadLibrary.h"

namespace Util {
namespace License {

RepriseDynamicLoadLibrary::RepriseDynamicLoadLibrary() {
    _lib.setDllName(libName());
}

void RepriseDynamicLoadLibrary::loadLib() {
    _lib.setFunctionNames(symbols());
    _lib.load();
    if (!_lib.isLoaded()) {
#ifdef _DEBUG
        std::cout << "Load " << libName() << " failed!" << std::endl;
#endif
    }
    else {
        if (_lib.missSymbol()) {
#ifdef _DEBUG
            std::cout << "Some symbols could not be resolved (Possible wrong version)!" << std::endl;
#endif
        }
        else {
            linkLib();
#ifdef _DEBUG
            std::cout << "Load " << libName() << " succeeded!" << std::endl;
#endif
        }
    }
}

void RepriseDynamicLoadLibrary::unloadLib() {
    if (_lib.unload()) {
#ifdef _DEBUG
            std::cout << "Unload " << libName() << " succeeded!" << std::endl;
#endif
    }
    else {
#ifdef _DEBUG
        std::cout << "Unload " << libName() << " failed!" << std::endl;
#endif
    }
}

void RepriseDynamicLoadLibrary::linkLib() {
    LINK_FP(rlm_init,rlm_init)
    LINK_FP(rlm_close,rlm_close)
    LINK_FP(rlm_stat,rlm_stat)
    LINK_FP(rlm_act_new_handle,rlm_act_new_handle)
    LINK_FP(rlm_act_set_handle,rlm_act_set_handle)
    LINK_FP(rlm_act_destroy_handle,rlm_act_destroy_handle)
    LINK_FP(rlm_activate,rlm_activate)
    LINK_FP(rlm_license_stat,rlm_license_stat)
    LINK_FP(rlm_errstring,rlm_errstring)
    LINK_FP(rlm_checkout,rlm_checkout)
    LINK_FP(rlm_checkin,rlm_checkin)
    LINK_FP(rlm_get_rehost,rlm_get_rehost)
    LINK_FP(rlm_hostid,rlm_hostid)
    LINK_FP(rlm_act_revoke,rlm_act_revoke)
    LINK_FP(rlm_act_errstring,rlm_act_errstring)
    LINK_FP(rlm_errstring,rlm_errstring)
    LINK_FP(rlm_act_revoke_disconn,rlm_act_revoke_disconn)
    LINK_FP(rlm_act_keyvalid,rlm_act_keyvalid)
    LINK_FP(rlm_products,rlm_products)
    LINK_FP(rlm_license_akey,rlm_license_akey)
    LINK_FP(rlm_license_hostid,rlm_license_hostid)
    LINK_FP(rlm_errstring_num,rlm_errstring_num)
    LINK_FP(rlm_license_exp_days,rlm_license_exp_days)
}

std::vector<std::string> RepriseDynamicLoadLibrary::symbols() {
    std::vector<std::string> symbols = {
        "rlm_init",
        "rlm_close",
        "rlm_stat",
        "rlm_act_new_handle",
        "rlm_act_set_handle",
        "rlm_act_destroy_handle",
        "rlm_activate",
        "rlm_act_revoke",
        "rlm_license_stat",
        "rlm_errstring",
        "rlm_checkout",
        "rlm_checkin",
        "rlm_get_rehost",
        "rlm_hostid",
        "rlm_act_errstring",
        "rlm_act_revoke_disconn",
        "rlm_act_keyvalid",
        "rlm_products",
        "rlm_license_akey",
        "rlm_license_hostid",
        "rlm_errstring_num",
        "rlm_license_exp_days"
    };
    return symbols;
}

}
}
