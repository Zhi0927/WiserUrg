#include "RegisterManager.h"
#include <QSettings>


RegisterManager::RegisterManager(){
	getEncodeString();
	getActivateCodeString();
}
RegisterManager::~RegisterManager(){
}

const QString RegisterManager::getEncode() const {
	return computerKey;
}
const QString RegisterManager::getActivate() const {
	return activateKey;
}

void RegisterManager::getEncodeString() {
	QString CPU_ID = get_cpuId();

	std::string salt01 = "acs3@!@$#$@#)(*___*sdfkjf124awpoeifjpoawejf6w4ef3945wproiweh92385osjsdf2525s4f65";
	std::string salt02 = "&%&oiuh$#%sdfg__werhi234654ertwertnjkasdnfl&%$A#@ihgirgoizdfpbk&#$ioprjg__3oigjp";
	Md5Encode encode_obj;
	std::string ret_1 = encode_obj.Encode(salt01 + CPU_ID.toStdString() + salt02);
	std::string ret_2 = encode_obj.Encode(ret_1);
	std::string ret_3 = encode_obj.Encode(ret_2);
	std::string ret_4 = encode_obj.Encode(ret_3);
	std::string ret_5 = encode_obj.Encode(ret_4);
	std::string ret_6 = encode_obj.Encode(ret_5);
	std::string ret;
	std::transform(ret_6.begin(), ret_6.end(), back_inserter(ret), ::toupper);

	computerKey = QString::fromStdString(ret);
	//std::cout << "Computer: " << computerKey.toStdString() << std::endl;
}

void RegisterManager::getActivateCodeString() {
	QSettings* RegeditReg = new QSettings("HKEY_CURRENT_USER\\Software\\WiserUrg", QSettings::NativeFormat);
	QString activeCode = RegeditReg->value("activateCode").toString();
	activateKey = activeCode;
	delete RegeditReg;
}

void RegisterManager::setActivateCodetoRegedit(QString code) {
	QSettings* RegeditReg = new QSettings("HKEY_CURRENT_USER\\Software\\WiserUrg", QSettings::NativeFormat);
	RegeditReg->setValue("Version", "1.0");
	RegeditReg->setValue("activateCode", code);
	delete RegeditReg;
}

bool RegisterManager::checkActivate() {
	if (computerKey.isNull() || activateKey.isNull()) return false;
	return computerKey == activateKey;
}

QString RegisterManager::get_cpuId() {
	QString cpu_id = "";
	unsigned int dwBuf[4] = { 0 };
	unsigned long long ret = 0;
	getcpuid(dwBuf, 1);
	ret = dwBuf[3];
	ret = ret << 32;

	QString str0 = QString::number(dwBuf[3], 16).toUpper();
	QString str0_1 = str0.rightJustified(8, '0');				
	QString str1 = QString::number(dwBuf[0], 16).toUpper();
	QString str1_1 = str1.rightJustified(8, '0');				
	//cpu_id = cpu_id + QString::number(dwBuf[0], 16).toUpper();
	cpu_id = str0_1 + str1_1;
	return cpu_id;
}

void RegisterManager::getcpuid(unsigned int CPUInfo[4], unsigned int InfoType)
{
#if defined(__GNUC__)	// GCC
	__cpuid(InfoType, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#elif defined(_MSC_VER) // MSVC
#if _MSC_VER >= 1400	//Only VC2005 supported __cpuid
	__cpuid((int*)(void*)CPUInfo, (int)(InfoType));
#else //other used getcpuidex
	getcpuidex(CPUInfo, InfoType, 0);
#endif
#endif
}

void RegisterManager::getcpuidex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue)
{
#if defined(_MSC_VER)	// MSVC
#if defined(_WIN64)		// Inline assembly is not supported on 64-bit. 1600: VS2010, __cpuidex is only supported above VC2008 SP1.
	__cpuidex((int*)(void*)CPUInfo, (int)InfoType, (int)ECXValue);
#else
	if (NULL == CPUInfo)  return;
	_asm {
		// load. Read parameters into registers.
		mov edi, CPUInfo;
		mov eax, InfoType;
		mov ecx, ECXValue;

		// CPUID
		cpuid;

		// save. Save registers to CPUInfo.
		mov[edi], eax;
		mov[edi + 4], ebx;
		mov[edi + 8], ecx;
		mov[edi + 12], edx;
	}
#endif
#endif
}