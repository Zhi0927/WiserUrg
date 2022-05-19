#ifndef ZHI_REGISTERMANAGER_H_
#define ZHI_REGISTERMANAGER_H_

#include <iostream>
#include <string>
#include <QtWidgets/QWidget>

#include "md5_encode.h"

class RegisterManager
{
public:
	RegisterManager();
	~RegisterManager();

	void getEncodeString();
	void getActivateCodeString();
	void setActivateCodetoRegedit(QString code);

	const QString getEncode() const;
	const QString getActivate() const;

	bool checkActivate();

private:
	QString get_cpuId();
	void getcpuid(unsigned int CPUInfo[4], unsigned int InfoType);
	void getcpuidex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue);

private:
	QString computerKey;
	QString activateKey;
};
#endif
