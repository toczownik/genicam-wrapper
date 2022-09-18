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
    explicit System(std::shared_ptr<const GenTLWrapper> genTlWrapper);
    std::vector<Interface*> getInterfaces(int updateTimeout = 100);
    std::string getInfos(bool displayFull);
    int getInfo(std::string* returnString, GenTL::TL_INFO_CMD info);
    ~System();

private:

    std::shared_ptr<const GenTLWrapper> genTL;
    GenTL::TL_HANDLE TL = nullptr;
};
