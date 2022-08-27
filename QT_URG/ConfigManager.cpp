#include "ConfigManager.h"


ConfigManager::ConfigManager()
{
	mIniPath = QCoreApplication::applicationDirPath() + "/config.ini";
}

void ConfigManager::SaveWindowSize(QWidget* pWnd)
{
	QString className = pWnd->metaObject()->className();

	QSettings configIniWrite(mIniPath, QSettings::IniFormat);
	configIniWrite.setValue(QString("/%1/x").arg(className), pWnd->geometry().x());
	configIniWrite.setValue(QString("/%1/y").arg(className), pWnd->geometry().y());
	configIniWrite.setValue(QString("/%1/width").arg(className), pWnd->geometry().width());
	configIniWrite.setValue(QString("/%1/height").arg(className), pWnd->geometry().height());
}

void ConfigManager::LoadWindowSize(QWidget* pWnd)
{
	QString className = pWnd->metaObject()->className();

	QSettings configIniRead(mIniPath, QSettings::IniFormat);
	int x = configIniRead.value(QString("/%1/x").arg(className), 50).toInt();
	int y = configIniRead.value(QString("/%1/y").arg(className), 50).toInt();
	int w = configIniRead.value(QString("/%1/width").arg(className), 1000).toInt();
	int h = configIniRead.value(QString("/%1/height").arg(className), 600).toInt();

	pWnd->setGeometry(x, y, w, h);
}

void ConfigManager::SaveParameter(Ui::Qt_urgClass* ui) {

	QSettings configIniWrite(mIniPath, QSettings::IniFormat);

	configIniWrite.setValue("/Paramater/Ip01"				, ui->IP_Input_01->text());
	configIniWrite.setValue("/Paramater/Port01"				, ui->Port_number_input_01->value());
	configIniWrite.setValue("/Paramater/Ip02"				, ui->IP_Input_02->text());
	configIniWrite.setValue("/Paramater/Port02"				, ui->Port_number_input_02->value());
	configIniWrite.setValue("/Paramater/RelativePosX"		, ui->OriginX->value());
			
	configIniWrite.setValue("/Paramater/OffsetX"			, ui->OffsetX_Input->value());
	configIniWrite.setValue("/Paramater/OffsetY"			, ui->OffsetY_Input->value());
	configIniWrite.setValue("/Paramater/Width"				, ui->Width_Input->value());	
	configIniWrite.setValue("/Paramater/Heigh"				, ui->Height_Input->value());

	configIniWrite.setValue("/Paramater/noiseLimit"			, ui->noiseLimit_Input->value());
	configIniWrite.setValue("/Paramater/deltaLimit"			, ui->deltaLimit_Input->value());
	configIniWrite.setValue("/Paramater/distanceThreshold"	, ui->distanceThreshold_Input->value());
	configIniWrite.setValue("/Paramater/DetectSize"			, ui->DetectSize_Input->value());
	configIniWrite.setValue("/Paramater/SmoothFactor"		, ui->SmoothFactor_Input->value());

	configIniWrite.setValue("/Paramater/cursoroffsetX"		, ui->cursoroffsetX_Input->value());
	configIniWrite.setValue("/Paramater/cursoroffsetY"		, ui->cursoroffsetY_Input->value());	
}

void ConfigManager::LoadParameter(Ui::Qt_urgClass* ui) {
	QSettings configIniRead(mIniPath, QSettings::IniFormat);

	ui->IP_Input_01->setText(configIniRead.value("/Paramater/Ip01", QString("192.168.0.11")).toString());
	ui->Port_number_input_01->setValue(configIniRead.value("/Paramater/Port01", 10940).toDouble());
	ui->IP_Input_02->setText(configIniRead.value("/Paramater/Ip02", QString("192.168.0.10")).toString());
	ui->Port_number_input_02->setValue(configIniRead.value("/Paramater/Port02", 10940).toDouble());
	ui->OriginX->setValue(configIniRead.value("/Paramater/RelativePosX", 0).toDouble());

	ui->OffsetX_Input->setValue(configIniRead.value("/Paramater/OffsetX", 0).toDouble());
	ui->OffsetY_Input->setValue(configIniRead.value("/Paramater/OffsetY", 0).toDouble());
	ui->Width_Input->setValue(configIniRead.value("/Paramater/Width", 1500).toDouble());
	ui->Height_Input->setValue(configIniRead.value("/Paramater/Heigh", 1000).toDouble());

	ui->noiseLimit_Input->setValue(configIniRead.value("/Paramater/noiseLimit", 15).toDouble());
	ui->deltaLimit_Input->setValue(configIniRead.value("/Paramater/deltaLimit", 200).toDouble());
	ui->distanceThreshold_Input->setValue(configIniRead.value("/Paramater/distanceThreshold", 300).toDouble());
	ui->DetectSize_Input->setValue(configIniRead.value("/Paramater/DetectSize", 300).toDouble());
	ui->SmoothFactor_Input->setValue(configIniRead.value("/Paramater/SmoothFactor", 0.05).toDouble());

	ui->cursoroffsetX_Input->setValue(configIniRead.value("/Paramater/cursoroffsetX", 0).toDouble());
	ui->cursoroffsetY_Input->setValue(configIniRead.value("/Paramater/cursoroffsetY", 0).toDouble());
}