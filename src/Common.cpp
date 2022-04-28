#include "Common.hpp"

Screen screen = { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };


std::vector<long> movingAverages(const std::vector<long>& data, int period) {
    std::vector<long> buffer(period);
    std::vector<long> output(data.size());

    int currrent_index = 0;
    for (size_t i = 0; i < data.size(); i++) {
        long ma = 0;
        for (size_t j = 0; j < period; i++) {
            ma += buffer[j];
        }
        output[i] = ma;
        currrent_index = (currrent_index + 1) % period;
    }
    return output;
}

void flipy(vector3& vec) {
    vec.y = screen.width - vec.y;
}

void map(float& value, const float& fromsource, const float& tosource, const float& fromtarget, const float& totarget) {
    value = (value - fromsource) / (tosource - fromsource) * (totarget - fromtarget) + fromtarget;
}


std::string GenerateGuid()
{
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

std::string ToString(const int& value, const int pad) {
    std::stringstream ss;
    ss << std::setw(pad) << std::setfill('0') << value;
    std::string s = ss.str();
    return s;
}

std::vector<std::string> SplitString(const std::string& str) {
    auto result = std::vector<std::string>{};
    auto ss = std::stringstream{ str };

    for (std::string line; std::getline(ss, line, '\n');)
        result.emplace_back(line);

    return result;
}

std::vector<std::string> split(const std::string& srcstr, const std::string& delimeter) {
    std::vector<std::string> ret(0);
    if (srcstr.empty())    
    {
        return ret;
    }
    std::string::size_type pos_begin = srcstr.find_first_not_of(delimeter);

    std::string::size_type dlm_pos;
    std::string temp;              
    while (pos_begin != std::string::npos)
    {
        dlm_pos = srcstr.find(delimeter, pos_begin);
        if (dlm_pos != std::string::npos)
        {
            temp = srcstr.substr(pos_begin, dlm_pos - pos_begin);
            pos_begin = dlm_pos + delimeter.length();
        }
        else
        {
            temp = srcstr.substr(pos_begin);
            pos_begin = dlm_pos;
        }
        if (!temp.empty())
            ret.push_back(temp);
    }
    return ret;
}

bool startswith(const std::string& str, const std::string& start) {
    int srclen = str.size();
    int startlen = start.size();
    if (srclen >= startlen)
    {
        std::string temp = str.substr(0, startlen);
        if (temp == start)
            return true;
    }
    return false;
}

bool endswith(const std::string& str, const std::string& end) {
    int srclen = str.size();
    int endlen = end.size();
    if (srclen >= endlen)
    {
        std::string temp = str.substr(srclen - endlen, endlen);
        if (temp == end)
            return true;
    }

    return false;
}

std::string trim(const std::string& str)
{
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


DataTranslator::DataTranslator(const int& xOffset, const int& yOffset, const int& sensorDetectWidth, const int& sensorDetectHeight){
    m_xOffset = xOffset;
    m_yOffest = yOffset;
    m_sensorDetectWidth  = sensorDetectWidth;
    m_sensorDetectHeight = sensorDetectHeight;
}

DataTranslator::~DataTranslator(){

}

void DataTranslator::Sensor2Screen(vector3& inputData, const ZeroPosition zeroPosition) {
    inputData.x += m_xOffset;
    inputData.y += m_yOffest;

    inputData.x += (m_sensorDetectWidth / 2.f);

    inputData.x = m_sensorDetectWidth - inputData.x;

    inputData.x /= m_sensorDetectWidth;
    inputData.y /= m_sensorDetectHeight;

    inputData.x *= screen.width;
    inputData.y *= screen.width;

    if (zeroPosition == ZeroPosition::LEFT_BOTTOM) {
        flipy(inputData);
    }
}

