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

template <typename T>
class Kalman { 
public:
    T kalmanFilterFun(const T& new_value) {
        T predictValue = filterValue * A + B * u;
        P = A * A * P + Q;
        kalmanGain = P * H / (P * H * H + R);
        filterValue = predictValue + (new_value - predictValue) * kalmanGain;
        P = (1 - kalmanGain * H) * P;
        return filterValue;
    }
    void setfilterValue(const T& init_value) {
        filterValue = init_value;
    }

private:
    T filterValue;;
    float kalmanGain;
    float A = 1;
    float H = 1;
    float Q = 0.05;
    float R = 0.1;
    float P = 0.1;
    float B = 1;
    float u = 0;
};

template<typename T>
std::vector<T> slice(const std::vector<T>& v, int m, int n){
    auto first = v.cbegin() + m;
    auto last = v.cbegin() + n;

    std::vector<T> vec(first, last);
    return vec;
}

std::vector<long> movingAverages(const std::vector<long>& data, int period);
void SmoothRealtime(std::vector<long>& newList, std::vector<long>& previousList, const float smoothFactor = 0.5f, const int limit = 200);
inline void map(float& value, const float& fromsource, const float& tosource, const float& fromtarget, const float& totarget);
std::string GenerateGuid();
std::string ToString(const int& value, const int pad);
std::vector<std::string> SplitString(const std::string& str);
std::vector<std::string> split(const std::string& srcstr, const std::string& delimeter);
bool startswith(const std::string& str, const std::string& start);
bool endswith(const std::string& str, const std::string& end);
std::string trim(const std::string& str);
#endif

