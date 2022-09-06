#pragma once

#include <vector>
#include <string>
#include "interface.h"
#include "gentl_wrapper.h"
#include <GenTL/GenTL.h>
#include <memory>

class System {
public:
    explicit System(const std::string& filename);
    // przekazac gentl jako argument z domyślna wartością
    std::vector<Interface*> getInterfaces(int updateTimeout = 100);
    std::string getInfos(bool displayFull);
    GenTL::GC_ERROR open();
    std::string getInfo(GenTL::TL_INFO_CMD info);

private:

    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::TL_HANDLE TL = NULL;
    void closeAll();
};
