#ifndef ZHI_UTILS_H_
#define ZHI_UTILS_H_

#include "ObjBase.h"
#include <iostream>
#include <time.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Vector3.hpp"

template<typename T>
class KalmanFilter {
public:
    void Filter(std::vector<T>& zInput) {
        float P = 1.f;

        for (size_t i = 1; i < zInput.size(); i++) {
            float Xhatminus = zInput[i - 1];
            float Pminus = P + Q;
            float K = Pminus / (Pminus + R);
            zInput[i] = (T)(Xhatminus + K * (zInput[i] - Xhatminus));
            P = (1 - K) * Pminus;
        }
    }

    T Filter(T z1)
    {
        if (isFirst)
        {
            isFirst = false;
            if (haveSetFirst == false) {
                Z0 = z1;
            }
            xhat = Z0;
            P = 1;
        }

        float Xhatminus = xhat;
        float Pminus = P + Q;
        float K = Pminus / (Pminus + R);
        xhat = Xhatminus + K * (z1 - Xhatminus);
        P = (1 - K) * Pminus;

        return (T)xhat;
    }

    void SetFirst(float zv) {
        Z0 = zv;
        haveSetFirst = true;
    }

    void SetQ(float Qv) {
        Q = Qv;
    }

    void SetR(float Rv) {
        R = Rv;
    }

    void Reset(){
        isFirst         = true;
        haveSetFirst    = false;
        xhat            = 0;
        Z0              = 0;
        P               = 0;
        Q               = 1e-5f; 
        R               = 1e-4f; 
    }

private:
    bool  isFirst       = true;
    bool  haveSetFirst  = false;
    float xhat          = 0;
    float P             = 0;
    float Z0            = 0;
    float Q             = 1e-5; //estimation variance
    float R             = 1e-4; //meaxurement variance
};

template<typename T>
std::vector<T> sliceVectors(const std::vector<T>& v, int m, int n) {
    auto first = v.cbegin() + m;
    auto last = v.cbegin() + n;

    std::vector<T> vec(first, last);
    return vec;
};

inline std::vector<vector3> movingAverages(const std::vector<vector3>& data, int period) {
    std::vector<vector3> result(data.size());
    vector3 sum = vector3(0, 0, 0);
    for (int i = 0; i < result.size(); i++) {
        sum += data[i];
        if (i >= period) sum -= data[i - period];
        result[i] = sum / (std::min)(i + 1, period);
    }
    return result;
}

inline void SmoothRealtime(std::vector<long>& newList, std::vector<long>& previousList, const float smoothFactor = 0.5f, const int thres = 200) {
    if (previousList.size() <= 0) {
        previousList = newList;
        return;
    }
    else {
        for (int i = 0; i < newList.size(); i++) {

            float diff = newList[i] - previousList[i];
            if (diff > thres) {
                previousList[i] = newList[i];
            }
            else {
                newList[i] = (long)(previousList[i] + diff * smoothFactor);
                previousList[i] = newList[i];
            }
        }
    }
}

inline void map(float& value, const float& fromsource, const float& tosource, const float& fromtarget, const float& totarget) {
    value = (value - fromsource) / (tosource - fromsource) * (totarget - fromtarget) + fromtarget;
}

inline std::string GenerateGuid() {
    GUID guid;
    CoCreateGuid(&guid);
    char cBuffer[64] = { 0 };
    sprintf_s(cBuffer, sizeof(cBuffer),
        "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        guid.Data1, guid.Data2,
        guid.Data3, guid.Data4[0],
        guid.Data4[1], guid.Data4[2],
        guid.Data4[3], guid.Data4[4],
        guid.Data4[5], guid.Data4[6],
        guid.Data4[7]);
    return std::string(cBuffer);
}

inline std::string ToString(const int& value, const int pad) {
    std::stringstream ss;
    ss << std::setw(pad) << std::setfill('0') << value;
    std::string s = ss.str();
    return s;
}

inline std::vector<std::string> SplitString(const std::string& str) {
    auto result = std::vector<std::string>{};
    auto ss = std::stringstream{ str };

    for (std::string line; std::getline(ss, line, '\n');)
        result.emplace_back(line);

    return result;
}

inline std::vector<std::string> split(const std::string& srcstr, const std::string& delimeter) {
    std::vector<std::string> ret(0);
    if (srcstr.empty())
    {
        return ret;
    }
    std::string::size_type pos_begin = srcstr.find_first_not_of(delimeter);

    std::string::size_type dlm_pos;
    std::string temp;
    while (pos_begin != std::string::npos) {
        dlm_pos = srcstr.find(delimeter, pos_begin);
        if (dlm_pos != std::string::npos) {
            temp = srcstr.substr(pos_begin, dlm_pos - pos_begin);
            pos_begin = dlm_pos + delimeter.length();
        }
        else {
            temp = srcstr.substr(pos_begin);
            pos_begin = dlm_pos;
        }
        if (!temp.empty())
            ret.push_back(temp);
    }
    return ret;
}

inline bool startswith(const std::string& str, const std::string& start) {
    int srclen = str.size();
    int startlen = start.size();
    if (srclen >= startlen) {
        std::string temp = str.substr(0, startlen);
        if (temp == start)
            return true;
    }
    return false;
}

inline bool endswith(const std::string& str, const std::string& end) {
    int srclen = str.size();
    int endlen = end.size();
    if (srclen >= endlen) {
        std::string temp = str.substr(srclen - endlen, endlen);
        if (temp == end)
            return true;
    }

    return false;
}

inline std::string trim(const std::string& str) {
    std::string ret;
    std::string::size_type pos_begin = str.find_first_not_of(" \t");
    if (pos_begin == std::string::npos)
        return str;

    std::string::size_type pos_end = str.find_last_not_of(" \t");
    if (pos_end == std::string::npos)
        return str;

    ret = str.substr(pos_begin, pos_end - pos_begin);

    return ret;
}

#endif

