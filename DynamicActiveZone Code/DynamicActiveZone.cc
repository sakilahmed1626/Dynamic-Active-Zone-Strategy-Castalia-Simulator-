/****************************************************************************
 *                                                                         *
 ****************************************************************************/

#include "DynamicActiveZone.h"


Define_Module(DynamicActiveZone);

void DynamicActiveZone::startup()
{
	packet_rate = par("packet_rate");
	recipientAddress = par("otherRecipient").stringValue();
	//otherRecipientAddress = par("nextRecipient").stringValue();
	startupDelay = par("startupDelay");

	packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;
	dataSN = 0;

	if (packet_spacing > 0 && recipientAddress.compare(SELF_NETWORK_ADDRESS) != 0 )
		setTimer(SEND_PACKET, packet_spacing + startupDelay);
	else
		trace() << "Not sending packets";
	
	found = false;
	speed = 5;
	newwRecipient= "1";
	declareOutput("Packets received per node");
	recharge = 1;
	csc_1 = 0;
	memset(gList, 0, sizeof(gList) );
	memset(eList, 0, sizeof(eList) );
	memset(distanceList, 0, sizeof(distanceList) );
	xxx = 0; yyy = 0;
}

void DynamicActiveZone::fromNetworkLayer(ApplicationPacket * rcvPacket,
		const char *source, double rssi, double lqi)
{
	int sequenceNumber = rcvPacket->getSequenceNumber();
	double csc = rcvPacket->getData();

	if (( recipientAddress.compare(SELF_NETWORK_ADDRESS) == 0 ) &&  (csc == 0 ) ) 
	{
		if ((sequenceNumber == 1111111))
		{
			trace() << "General recharge request from: " << source;
			//send(new cMessage("Just Checking GGGG", JUST_CHECK), "toMobilityManager");
			//resetRequestFromVehicle(source);
		}  
		else if  ((sequenceNumber == 22222222) ) 
		{
			trace() << "Emergency recharge request from: " << source;
			//send(new cMessage("Just Checking E", JUST_CHECK), "toMobilityManager");

		}  
		else 
		{
			collectOutput("Packets received per node", atoi(source));
		}
			// Packet has to be forwarded to the next hop recipient
		
	}
	else 	if (recipientAddress.compare(SELF_NETWORK_ADDRESS) == 0 &&  (csc != 0 ) ) 
	{
		
			if(gCount == 0 && eCount == 0)  { check = 1; }
		
			if(gCount != 0 ){
				for (int i = 0; i < gCount ; i++){
					if(gList[i][0] == atoi(source))	{check = 0;}
					else {check = 1; }
				}
			}
			
			if(check == 1){ 
						
				gList[gCount][0] = atoi(source);
				gList[gCount][1] = csc;
				gList[gCount][2] = sequenceNumber;
				gList[gCount][3] = dist;
				//memset(distanceList, 0, sizeof(distanceList) );
				//trace() << "From Glist:  source = " << gList[gCount][0] << ", y = " << gList[gCount][2];
				gCount++;
				
				if(gCount == 1) {setTimer(START_DAZ, 300);}
				
			}  else {
						
					eList[eCount][0] = atoi(source);
					eList[eCount][1] = csc;
					eList[eCount][2] = sequenceNumber;
					eList[eCount][3] = dist;
					//memset(distanceList, 0, sizeof(distanceList) );
					//trace() << "From EList: source = " << eList[eCount][0] << ", y = " << eList[eCount][2];
					eCount++;
					//check = 0;
				
			}
	}
	else 	if ((recipientAddress.compare(SELF_NETWORK_ADDRESS) != 0 )  && ( sequenceNumber  == 5555555)) 
	{
			resetEnergy();
			//trace() << "Finally" ;
	}
	else 
	{
			ApplicationPacket* fwdPacket = rcvPacket->dup();
			// Reset the size of the packet, otherwise the app overhead will keep adding on
			fwdPacket->setByteLength(0);
			otherRecipient = "0";
			recipientAddress = par("otherRecipient").stringValue();
			toNetworkLayer(fwdPacket, recipientAddress.c_str());
	}

}

void DynamicActiveZone::timerFiredCallback(int index)
{
	switch (index) {
		case SEND_PACKET:{
			
			if(atoi(SELF_NETWORK_ADDRESS) != 1) {			
			//trace() << "Sending packet #" << dataSN;
				
			toNetworkLayer(createGenericDataPacket(0, dataSN),  par("otherRecipient"));
			dataSN++;
			setTimer(SEND_PACKET, packet_spacing);
			}
			break;
		}			
		case SEND_GREQUEST:{
				
			// trace() << " STEP 2: From ResourceManger Layer to Application Layer of the node which requires recharging. 
			recharge = 1111111;
			toNetworkLayer(createGenericDataPacket(0, recharge), par("otherRecipient"));
			
			double xxx= mobilityModule->getLocation().x;	
			double yyy = mobilityModule->getLocation().y;
			
			// trace() <<"Sending message to the vehicle for recharge along with it nodes position."
			toNetworkLayer(createGenericDataPacket(xxx, yyy), par("otherRecipient"));	
			
			break;
		}
		case SEND_EREQUEST:{
			
			// trace() << " STEP 2: From ResourceManger Layer to Application Layer of the node which requires recharging. 
			recharge = 22222222;
			toNetworkLayer(createGenericDataPacket(0, recharge), par("otherRecipient"));
			
			double xxx= mobilityModule->getLocation().x;	
			double yyy = mobilityModule->getLocation().y;
			
			// trace() <<"Sending message to the vehicle for recharge along with it nodes position.";
			toNetworkLayer(createGenericDataPacket(xxx, yyy), par("otherRecipient"));	
			
			break;
		}
		case START_DAZ: {
			dazAlgorithm();
			break;
		}
		case RECHARGE_START: {
			rechargeNode();	
			break;
		}
		case SET_LOCATION_VEHICLE: {
			setLocation();
			break;
		}
		case RECALCULATE_SEQUENCE: {
			recalSequence();
			break;
		}

	}
}

// This method processes a received carrier sense interupt. Used only for demo purposes
// in some simulations. Feel free to comment out the trace command.
void DynamicActiveZone::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
	switch (radioMsg->getRadioControlMessageKind()) {
		case CARRIER_SENSE_INTERRUPT:
			trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
                        break;
	}
}


void DynamicActiveZone::generalRecharge()
{
	setTimer(SEND_GREQUEST, 0);
}

void DynamicActiveZone::emergencyRecharge()
{
	setTimer(SEND_EREQUEST, 0);
}



void DynamicActiveZone::nextDestination()
{

}

void DynamicActiveZone::prepareRechargeSequence()
{
	
	//Prepare recharging list.
	//Prepare recharge list from emergency requests first.	
	
	for(int j = 0; j< eCount; j++) {
			rechargeList[j][0] = eList[j][0];
			double xx = mobilityModule->getLocation().x;	
			double yy = mobilityModule->getLocation().y;
			dist = sqrt(pow(eList[j][1] - xxx, 2) + pow(eList[j][2] - yyy, 2) );
			rechargeList[j][1] = dist;
			//trace() << rechargeList[j][0] << "   "  << rechargeList[j][1]  << "   " << eList[j][0]  << "   " << dist;  
	
			rCount++;
	}
	
	//trace() << "rCOunt = " << rCount << " gList[][0] =m "  << gList[0][0] << "r , echargeList[n][0] = "  << rechargeList[0][0];
	
	// Remove duplicates from the General request list.
	for(int n = 0; n < rCount; n++){
		for (int o = 0; o < gCount; o++){
			if(rechargeList[n][0] == gList[o][0]){
				//for(int o = m+1; o < gCount; o++){
				trace() << "Duplicate node : " << 	gList[o][0];
					gList[o][0] = gList[o+1][0];
					gList[o][1] = gList[o+1][1];
					gList[o][2] = gList[o+1][2];
					gList[o][3] = gList[o+1][3];
			}
			gCount--;
		}
	}
	
	//Prepare recharge list from general requests.
	for(int k= 0; k< gCount; k++) {
			//gRechargeList[k][0] = gList[k][0];
			double xx = mobilityModule->getLocation().x;	
			double yy = mobilityModule->getLocation().y;
			dist = sqrt(pow(gList[k][1] - xx, 2) + pow(gList[k][2] - yy, 2) );
			gRechargeList[k] = dist;
			gList[k][3] = dist;
	}
	
	//Sort and combine general requests with emergency list in final rechargeList array.
	
	std:: sort(gRechargeList, gRechargeList+gCount);	
	
	int t = 0;
	for(int n = 0; n < gCount; n++){
		for(int p = 0; p < gCount; p++){
			if(gRechargeList[n] == gList[p][3]){
				rechargeList[rCount][0] = gList[p][0];
				rechargeList[rCount][1] = gList[p][3];
				rCount++;
			}
		}
	}
	
	// Print Current Recharge Sequence.
	
	trace() <<" Current recharge sequence: ";
	for(int n = 0; n < rCount; n++){
			trace() << rechargeList[n][0]  << ", ";
	}
	
	//Move towards the node that need to be recharged.
	
	if(eCount != 0) {
			xxx = eList[0][1];
			yyy = eList[0][2];	
			
	} else {
			//for(int n = 0; n < rCount; n++){
				for (int m = 0; m < gCount; m++){
					if(rechargeList[0][0] == gList[m][0]){
						//zzz = gList[m][0];
						xxx = gList[m][1];
						yyy = gList[m][2];
					}
				}
	}

	// -> update position
		
	double d = rechargeList[0][1];
	updateInterval = d/speed ;	
	setTimer(SET_LOCATION_VEHICLE,  50);
}	

void DynamicActiveZone::setLocation()
{	
	trace()  << "Arrived at nodes place. Vehicle's current position: " <<  xxx << " : " << yyy;	
	
	// -> recharge

	setTimer(RECHARGE_START, updateInterval + 100);	
}


void DynamicActiveZone::rechargeNode()
{
	double ND = rechargeList[0][0];
	stringstream ss;
	ss << ND;
	const char *rNode = ss.str().c_str();
	
	resetRequestFromVehicle(rNode);
}


void DynamicActiveZone::resetRequestFromVehicle(const char *resetNode)
{
	//trace()  << "Started Charging" ;
	toNetworkLayer(createGenericDataPacket(1, 5555555), resetNode);
	
	///
	// -> recaculate recharge sequance.
	// -> remove the recharged node from the list.
	//trace() << "Refresh Charging queue.";
	
	setTimer(RECALCULATE_SEQUENCE, 50 );	
}

void DynamicActiveZone::resetEnergy()
{
		ResourceManager* rm = 	check_and_cast<ResourceManager*>(getParentModule()->getSubmodule("ResourceManager"));
		//double inintEnergy = rm->par("initialEnergy");	
		double consE = rm->getSpentEnergy();
		int nn = atoi(SELF_NETWORK_ADDRESS);
		trace() << "Battery recharge completed.";
}

void DynamicActiveZone::recalSequence()
{
	if(eCount != 0) {
			for(int i = 1; i < eCount; i++){
				eList[i-1][0] = eList[i][0];
				eList[i-1][1] = eList[i][1];
				eList[i-1][2] = eList[i][2];
				eList[i-1][3] = eList[i][3];
			}
			eCount--;
			
	} else {
			for(int p = 0; p < gCount-1; p++){
				if(rechargeList[0][0] == gList[p][0]){
					zzz = p;
					found = true;
				}
			}
			if(found) {
				for(int p = zzz ; p < gCount-1; p++ ) {
					gList[p][0] = gList[p+1][0];
					gList[p][1] = gList[p+1][1];
					gList[p][2] = gList[p+1][2];
					gList[p][3] = gList[p+1][3];	
				}
			}
			found = false;
			gCount--;
			
	}

	memset(rechargeList, 0, sizeof(rechargeList) );
	memset(gRechargeList, 0, sizeof(gRechargeList) );
	rCount = 0;
	
	if((gCount != 0) || (eCount != 0)) {
			dazAlgorithm();
		}
}
	
void DynamicActiveZone::dazAlgorithm()
{
	prepareRechargeSequence();
	
}
