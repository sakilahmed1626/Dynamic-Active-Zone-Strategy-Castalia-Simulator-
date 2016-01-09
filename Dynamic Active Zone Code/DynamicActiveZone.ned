//****************************************************************************
//*                                                                          *  
//****************************************************************************/

package node.application.dynamicActiveZone;


// The sensor node module. Connects to the wireless channel in order to communicate 
// with other nodes. Connects to psysical processes so it can sample them.

simple DynamicActiveZone like node.application.iApplication {
 parameters:
 	string applicationID = default ("DynamicActiveZone");
	bool collectTraceInfo = default (false);
	int priority = default (1);
	int packetHeaderOverhead = default (5);		// in bytes
	int constantDataPayload = default (100);	// in bytes

	string otherRecipient = default ("0");
	string nextRecipient = default ("100");	// Destination for packets produced in this node. 
											// This parameter can be used to create an
											// application-level static routing. This way we can
											// have a multi-hop throughput test   

	int ddd = intuniform(1,30);	
	double packet_rate = default (ddd);	// packets per second, by default we transmit no packets
	double startupDelay = default (0);	// delay in seconds before the app stars producing packets

	double latencyHistogramMax = default (200);
	int latencyHistogramBuckets = default (10);

 gates:
 	output toCommunicationModule;
	output toSensorDeviceManager;
	input fromCommunicationModule;
	input fromSensorDeviceManager;
	input fromResourceManager;
	output toMobilityManager;
	
	
}
