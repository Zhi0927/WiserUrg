#ifndef HKY_URGDEVICE_H_
#define HKY_URGDEVICE_H_

#include <iostream>
#include <string>
#include <string_view>

class UrgDevice
{
public:
    enum class CMD
    {
        // https://www.hokuyo-aut.jp/02sensor/07scanner/download/pdf/URG_SCIP20.pdf
        VV, PP, II, // sensor information request command (3 types)
        BM, QT,     //measure start/end
        MD, GD,     // distance request command (2 types)
        ME          //distance and intensity request commands
    };

    static constexpr const char* enum_str[8] = { "VV", "PP", "II", "BM", "QT", "MD", "GD", "ME" };

    static std::string GetCMDString(const CMD& cmd);
};

#endif
