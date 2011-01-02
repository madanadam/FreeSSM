/*
 * SSMprotocol.h - Abstract application layer for the Subaru SSM protocols
 *
 * Copyright (C) 2009-2010 Comer352l
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SSMPROTOCOL_H
#define SSMPROTOCOL_H



#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QEventLoop>
#include <string>
#include <vector>
#include <cmath>
#include <limits.h>
#include "AbstractDiagInterface.h"
#include "libFSSM.h"


#define		MEMORY_ADDRESS_NONE	UINT_MAX


class dc_defs_dt
{
public:
	unsigned int byteAddr_currentOrTempOrLatest;
	unsigned int byteAddr_historicOrMemorized;
	QString code[8];
	QString title[8];
};
  
  
class  mb_dt
{
public:
	QString title;
	QString unit;
	QString scaleformula;
	char precision;
};


class  sw_dt
{
public:
	QString title;
	QString unit;
};


class  mb_intl_dt: public mb_dt
{
public:
	unsigned int addr_low;
	unsigned int addr_high;
};


class sw_intl_dt : public sw_dt
{
public:
	unsigned int  byteAddr;
	unsigned char bitAddr;
};


class MBSWmetadata_dt
{
public:
	bool blockType;
	unsigned int nativeIndex;
};


class adjustment_dt
{
public:
	QString title;
	QString unit;
	QString formula;
	unsigned int rawMin;
	unsigned int rawMax;
	unsigned int rawDefault;
	char precision;
};


class adjustment_intl_dt : public adjustment_dt
{
public:
	unsigned int AddrLow;
	unsigned int AddrHigh;
};


class actuator_dt
{
public:
	QString title;
	unsigned int byteadr;
	unsigned char bitadr;
};



class SSMprotocol : public QObject
{
	Q_OBJECT

public:
	enum protocol_dt {SSM1, SSM2};
	enum CUtype_dt {CUtype_Engine, CUtype_Transmission, CUtype_CruiseControl, CUtype_AirCon, CUtype_FourWheelSteering, CUtype_ABS, CUtype_AirSuspension, CUtype_PowerSteering};
	enum CUsetupResult_dt {result_success, result_invalidCUtype, result_commError, result_noOrInvalidDefsFile, result_noDefs};
	enum state_dt {state_needSetup, state_normal, state_DCreading, state_MBSWreading, state_ActTesting, state_waitingForIgnOff};
	enum DCgroups_dt {noDCs_DCgroup=0, currentDTCs_DCgroup=1, temporaryDTCs_DCgroup=2, historicDTCs_DCgroup=4, memorizedDTCs_DCgroup=8,
			  CClatestCCs_DCgroup=16, CCmemorizedCCs_DCgroup=32};
	enum CMlevel_dt {CMlevel_1=1, CMlevel_2=2};
	enum immoTestResult_dt {immoNotShorted, immoShortedToGround, immoShortedToBattery};

	SSMprotocol(AbstractDiagInterface *diagInterface, QString language="en");
	virtual ~SSMprotocol();
	// NON-COMMUNICATION-FUNCTIONS:
	bool CUtype(SSMprotocol::CUtype_dt *CU);
	state_dt state();
	virtual CUsetupResult_dt setupCUdata(CUtype_dt CU) = 0;
	virtual protocol_dt protocolType() = 0;
	std::string getSysID();
	std::string getROMID();
	virtual bool getSystemDescription(QString *sysdescription) = 0;
	virtual bool hasOBD2system(bool *OBD2) = 0;
	virtual bool hasVINsupport(bool *VINsup) = 0;
	virtual bool hasImmobilizer(bool *ImmoSup) = 0;
	virtual bool hasIntegratedCC(bool *CCsup) = 0;
	virtual bool hasClearMemory(bool *CMsup) = 0;
	virtual bool hasClearMemory2(bool *CM2sup) = 0;
	virtual bool hasTestMode(bool *TMsup) = 0;
	virtual bool hasActuatorTests(bool *ATsup) = 0;
	virtual bool getSupportedDCgroups(int *DCgroups) = 0;
	bool getLastDCgroupsSelection(int *DCgroups);
	bool getSupportedMBs(std::vector<mb_dt> *supportedMBs);
	bool getSupportedSWs(std::vector<sw_dt> *supportedSWs);
	bool getLastMBSWselection(std::vector<MBSWmetadata_dt> *MBSWmetaList);
	virtual bool getSupportedAdjustments(std::vector<adjustment_dt> *supportedAdjustments) = 0;
	virtual bool getSupportedActuatorTests(QStringList *actuatorTestTitles);
	virtual bool getLastActuatorTestSelection(unsigned char *actuatorTestIndex);
	// COMMUNICATION BASED FUNCTIONS:
	virtual bool isEngineRunning(bool *isrunning) = 0;
	virtual bool isInTestMode(bool *testmode);
	virtual bool getVIN(QString *VIN);
	virtual bool getAllAdjustmentValues(std::vector<unsigned int> *rawValues);
	virtual bool getAdjustmentValue(unsigned char index, unsigned int *rawValue);
	virtual bool setAdjustmentValue(unsigned char index, unsigned int rawValue);
	virtual bool clearMemory(CMlevel_dt level, bool *success) = 0;
	virtual bool testImmobilizerCommLine(immoTestResult_dt *result) = 0;
	virtual bool stopAllActuators();
	virtual bool startDCreading(int DCgroups) = 0;
	bool restartDCreading();
	virtual bool stopDCreading() = 0;
	virtual bool startMBSWreading(std::vector<MBSWmetadata_dt> mbswmetaList) = 0;
	bool restartMBSWreading();
	virtual bool stopMBSWreading() = 0;
	virtual bool startActuatorTest(unsigned char actuatorTestIndex);
	virtual bool restartActuatorTest();
	virtual bool stopActuatorTesting();
	bool stopAllPermanentOperations();
	virtual bool waitForIgnitionOff() = 0;

protected:
	AbstractDiagInterface *_diagInterface;
	CUtype_dt _CU;
	state_dt _state;
	QString _language;
	// *** CONTROL UNIT RAW DATA ***:
	char _SYS_ID[3];
	char _ROM_ID[5];
	char _flagbytes[96];
	unsigned char _nrofflagbytes;
	// *** CONTROL UNIT BASIC DATA (SUPPORTED FEATURES) ***:
	// Diagnostic Trouble Codes:
	std::vector<dc_defs_dt> _DTCdefs;
	// Measuring Blocks and Switches:
	std::vector<mb_intl_dt> _supportedMBs;
	std::vector<sw_intl_dt> _supportedSWs;
	// *** Temporary (operation related) data ***:
	std::vector<unsigned int> _selMBsSWsAddr;
	// *** Selection data ***:
	int _selectedDCgroups;
	std::vector<MBSWmetadata_dt> _MBSWmetaList;

	void evaluateDCdataByte(unsigned int DCbyteadr, char DCrawdata, std::vector<dc_defs_dt> DCdefs,
				QStringList *DC, QStringList *DCdescription);
	bool setupMBSWQueryAddrList(std::vector<MBSWmetadata_dt> MBSWmetaList);
	void assignMBSWRawData(std::vector<char> rawdata, std::vector<unsigned int> * mbswrawvalues);

signals:
	void currentOrTemporaryDTCs(QStringList currentDTCs, QStringList currentDTCsDescriptions, bool testMode, bool DCheckActive);
	void historicOrMemorizedDTCs(QStringList historicDTCs, QStringList historicDTCsDescriptions);
	void latestCCCCs(QStringList currentCCCCs, QStringList currentCCCCsDescriptions);
	void memorizedCCCCs(QStringList historicCCCCs, QStringList historicCCCCsDescriptions);
	void newMBSWrawValues(std::vector<unsigned int> rawValues, int duration_ms);
	void startedDCreading();
	void startedMBSWreading();
	void startedActuatorTest();
	void stoppedDCreading();
	void stoppedMBSWreading();
	void stoppedActuatorTest();
	void commError();

protected slots:
	void processMBSWrawData(std::vector<char> MBSWrawdata, int duration_ms);

public slots:
	virtual void resetCUdata() = 0;

};



#endif
