/****************************************************************************
 *                                                                          *
 ****************************************************************************/

#ifndef _DYNAMICACTIVEZONE_H_
#define _DYNAMICACTIVEZONE_H_

#include "VirtualApplication.h"
#include <algorithm>

using namespace std;

enum DynamicActiveZoneTimers {
	SEND_PACKET = 1
};
enum DAZRechargeTimers {
	SEND_GREQUEST = 62
};
enum DAZERechargeTimers {
	SEND_EREQUEST = 36
};
enum StartDAZOperation {
	START_DAZ = 99
};
enum StartRechargingTImer {
	RECHARGE_START
};
enum VehicleLocationTimer{
	SET_LOCATION_VEHICLE = 112
};
enum RecalculateRechargeSequence{
	RECALCULATE_SEQUENCE =  113
};

class DynamicActiveZone: public VirtualApplication {
 private:
	double packet_rate;
	int a;
	string recipientAddress;
	string otherRecipientAddress;
	string otherRecipient;
	string nextRecipient;
	double startupDelay;
	double csc_1 ;
	int NN;
	int speed;
	string neww;

	float packet_spacing;
	int dataSN;
	int recharge;
	int GENERAL;
	int EMERGENCY;
	double distanceList [20][4] ;
	double gList [20][4] ;
	double eList [20][4] ;
	int gCount = 0;
	double dist;
	int eCount = 0;
	int check = 0;
	int count = 0;
	double rechargeList [20][2] ;
	double gRechargeList[20];
	int rCount = 0;
	double updateInterval = 0 ;
	double xxx;
	double yyy;
	int zzz;
	bool found;

 protected:
	void startup();
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void handleRadioControlMessage(RadioControlMessage *);
	void timerFiredCallback(int);
	void generalRecharge();
	void emergencyRecharge();
	void resetEnergy();
	void nextDestination();
	void resetRequestFromVehicle(const char *resetNode);
	void dazAlgorithm();
	void rechargeNode();	
	void prepareRechargeSequence();
	void setLocation();
	void recalSequence();

public:
	string newwRecipient;







};

#endif				// _DynamicActiveZone_APPLICATIONMODULE_H_
