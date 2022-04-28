#include "SCIP_library.hpp"

std::string SCIP_Writer::END() {
	return "\n";
}
std::string SCIP_Writer::MD(int start, int end, int grouping, int skips, int scans){
	return UrgDevice::GetCMDString(UrgDevice::CMD::MD) + ToString(start, 4) + ToString(end, 4) + ToString(grouping, 2) + ToString(skips, 1) + ToString(scans, 2) + END();
}
std::string SCIP_Writer::ME(int start, int end, int grouping, int skips, int scans){
	return UrgDevice::GetCMDString(UrgDevice::CMD::ME) + ToString(start, 4) + ToString(end, 4) + ToString(grouping, 2) + ToString(skips, 1) + ToString(scans, 2) + END();
}
std::string SCIP_Writer::BM(){
	return UrgDevice::GetCMDString(UrgDevice::CMD::BM) + END();
}
std::string SCIP_Writer::GD(int start, int end, int grouping){
	return UrgDevice::GetCMDString(UrgDevice::CMD::GD) + ToString(start, 4) + ToString(end, 4) + ToString(grouping, 2) + END();
}
std::string SCIP_Writer::VV(){
	return UrgDevice::GetCMDString(UrgDevice::CMD::VV) + END();
}
std::string SCIP_Writer::II(){
	return UrgDevice::GetCMDString(UrgDevice::CMD::II) + END();
}
std::string SCIP_Writer::PP(){
	return UrgDevice::GetCMDString(UrgDevice::CMD::PP) + END();
}
std::string SCIP_Writer::SCIP2(){
	return "SCIP2.0" + END();
}
std::string SCIP_Writer::QT(){
	return UrgDevice::GetCMDString(UrgDevice::CMD::QT) + END();
}



bool SCIP_Reader::MD(const std::string& get_command, long& time_stamp, std::vector<long>& distances) {
	std::vector<std::string> split_command = { SplitString(get_command) };
	if (startswith(split_command[1], "00")) {
		return true;
	}
	else if(startswith(split_command[1], "99")){
		time_stamp = decode(split_command[2], 4);
		distance_data(split_command, 3, distances);
		return true;
	}
	else {
		return false;
	}
}

bool SCIP_Reader::GD(const std::string& get_command, long& time_stamp, std::vector<long>& distances) {
	std::vector<std::string> split_command = { SplitString(get_command) };
	if (startswith(split_command[1], "00")) {
		time_stamp = decode(split_command[2], 4);
		distance_data(split_command, 3, distances);
		return true;
	}
	else {
		return false;
	}
}

bool SCIP_Reader::distance_data(const std::vector<std::string>& lines, int start_line, std::vector<long>& distances) {
	std::stringstream ss;
	for (size_t i = start_line; i < lines.size(); ++i) {
		ss << lines[i].substr(0, lines[i].size() - 1);
	}
	return decode_array(ss.str(), 3, distances);
}

long SCIP_Reader::decode(const std::string& data, int size, int offset) {
	long value = 0;
	for (int i = 0; i < size; ++i) {
		value <<= 6;
		value |= static_cast<long>(data[offset + i]) - 0x30;
	}
	return value;
}

bool SCIP_Reader::decode_array(const std::string& data, int size, std::vector<long>& decoded_data) {
	for (int pos = 0; pos <= data.size() - size; pos += size) {
		decoded_data.emplace_back(decode(data, size, pos));
	}
	return true;
}

bool SCIP_Reader::ME(const std::string& get_command, long& time_stamp, std::vector<long>& distances, std::vector<long>& strengths) {
	std::vector<std::string> split_command = { SplitString(get_command) };
	if (startswith(split_command[1], "00")) {
		return true;
	}
	else if (startswith(split_command[1], "99")) {
		time_stamp = decode(split_command[2], 4);
		distance_strength_data(split_command, 3, distances, strengths);
		return true;
	}
	else {
		return false;
	}
}

bool SCIP_Reader::distance_strength_data(const std::vector<std::string>& lines, int start_line, std::vector<long>& distances, std::vector<long>& strengths) {
	std::stringstream ss;
	for (int i = start_line; i < lines.size(); ++i) {
		ss << lines[i].substr(0, lines[i].size() - 1);
	}
	return decode_array(ss.str(), 3, distances, strengths);
}

bool SCIP_Reader::decode_array(const std::string& data, int size, std::vector<long>& decoded_data, std::vector<long>& stdecoded_data) {
	for (int pos = 0; pos <= data.size() - size * 2; pos += size * 2) {
		decoded_data.emplace_back(decode(data, size, pos));
		stdecoded_data.emplace_back(decode(data, size, pos + size));
	}
	return true;
}