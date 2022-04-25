#include "UrgDevice.hpp"


std::string UrgDevice::GetCMDString(const CMD& cmd) {
	//auto name = magic_enum::enum_name(cmd);
	//return { name.data(), name.size() };

    std::string temp;
    switch (cmd)
    {
        case CMD::VV: temp.assign(enum_str[0], 2); break;
        case CMD::PP: temp.assign(enum_str[0], 2); break;
        case CMD::II: temp.assign(enum_str[0], 2); break;
        case CMD::BM: temp.assign(enum_str[0], 2); break;
        case CMD::QT: temp.assign(enum_str[0], 2); break;
        case CMD::MD: temp.assign(enum_str[0], 2); break;
        case CMD::GD: temp.assign(enum_str[0], 2); break;
        case CMD::ME: temp.assign(enum_str[0], 2); break;
        default: throw std::invalid_argument("Unimplemented item");
    }
    return temp;
}