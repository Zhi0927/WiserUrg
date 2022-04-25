#ifndef HKY_SCIP_H_
#define HKY_SCIP_H_

#include <iostream>
#include <string>
//#include <sstream>
//#include <iomanip>
#include <vector>
#include "UrgDevice.hpp"
#include "utils.hpp"

class SCIP_Writer
{
public:
	static std::string END();
	static std::string MD(int start, int end, int grouping = 1, int skips = 0, int scans = 0);
	static std::string ME(int start, int end, int grouping = 1, int skips = 0, int scans = 0);
	static std::string BM();
	static std::string GD(int start, int end, int grouping = 1);
	static std::string VV();
	static std::string II();
	static std::string PP();
	static std::string SCIP2();
	static std::string QT();
	//static std::string ToString(const int& value, const int pad);
};

class SCIP_Reader
{
public:
	static bool MD(const std::string& get_command, long& time_stamp, std::vector<long>& distances);
	static bool GD(const std::string& get_command, long& time_stamp, std::vector<long>& distances);
	static bool distance_data(const std::vector<std::string>& lines, int start_line, std::vector<long>& distances);
	static long decode(const std::string& data, int size, int offset = 0);
	static bool decode_array(const std::string& data, int size, std::vector<long>& decoded_data);
	static bool ME(const std::string& get_command, long& time_stamp, std::vector<long>& distances, std::vector<long>& strengths);
	static bool distance_strength_data(const std::vector<std::string>& lines, int start_line, std::vector<long>& distances, std::vector<long>& strengths);
	static bool decode_array(const std::string& data, int size, std::vector<long>& decoded_data, std::vector<long>& stdecoded_data);
	//static std::vector<std::string> SplitString(const std::string& str);
};
#endif
