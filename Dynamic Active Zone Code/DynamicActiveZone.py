############################################################################################################################
## The program 'DyanmicActiveZone.py' simulates simulates the recharging of the sensor nodes of a Wireless Sensor Network.##
## It creates sensor nodes and mobile charger. All nodes have a pre-defined initial energy and positions. The Python file ##
## 'SetCoordinates.py' gets the coordinates of the nodes that are clustered using the KMEC algorithm. Nodes send message  ##
## periodically to the base. Nodes generate charging request based on their remaining energy. Two types to request can be ##
## generated: General request and Emergency request.                                                                      ##
##                                                                                                                        ##
## The mobile charger stays at the base station initially. Whenever any charing request arrives at the base, the charger  ##
## uses the Dynamic Active Zone Strategy to make the recharge sequence which in essence prioritizes the emergency request ##
## and creates partial charging routes.                                                                                   ##     
############################################################################################################################




import threading
import time
import numpy as np
from SetCoordinates import SetCoordinates as Coordinates

# Save the output in a file.

import sys
sys.stdout=open("DAZ_500_Nodes_5_MC_Set_03.txt","w")

##------------------VARIABLES---------------------------------##


number_of_nodes = 100
nodes_initial_energy = 18720   # Joules
mc_initial_energy =  40 * 1000 # Joules
charging_time = 12 # (Minutes) Time to charge a Node.
energy_lost_for_charging = 6000 # Joules
    
queue = np.zeros([number_of_nodes,3])
rechargedList = []
after_recharge= np.zeros([number_of_nodes,5])

g_queue = np.zeros([number_of_nodes])
e_queue = np.zeros([number_of_nodes])

dead_list = []

position= np.zeros([number_of_nodes, 2])    # Array contains the x and y coordinates of the Nodes

## Get coordinates for each nodes.

xy_coordinates = Coordinates()
x, y  = xy_coordinates.setCoordinates5Clusters(3)


for i in range(number_of_nodes):
    position[i,0] = x[i]
    position[i,1] = y[i]
    
print("Postions: ", position)    
    

total_distance_covered_by_MC = []

##--------------------Sensor-------------------------------------------------------##




def Sensor(node_ID):
    '''
    The Sensor(node_ID) function creates a node and takes a parameter 'node_ID' which is the ID of the node.
    Each node has a battery with 18720 Joules of energy. After a node is created, it senses the sorrounding 
    environment and send the sensed data to the Base Station periodically. 
    
    The message sending rate is set to (0-5) message/minute.
    Nodes continuously monitor their energy status and when the energy level becomes 70% less than the
    initial energy then the node generates a "General Charging Request".
    When the energy becomes less than 10% of the initial energy, then the node generates an "Emergency Request".
    
    When the node uses all the available energy and it was not recharged then, it's considered as a 'dead' node
    and it updates the 'dead list' which contains tracks of all the dead nodes in the Sensor Network.
    
    '''
    
    
    
    t_start = (time.ctime(time.time()))
    print(t_start)
    
    battery = nodes_initial_energy
    
    once_gnr = 0
    once_emg = 0 
    energy_consumed_per_minute = np.random.randint(0,6)  # Joules
    
    while(battery > 0 ):
        
        battery = battery - energy_consumed_per_minute
        
        wait_until_next_message = 1    # Minutes
        time.sleep(wait_until_next_message)
    
        
    ##------------------------
        
        if(battery <= nodes_initial_energy * 0.7 and battery > nodes_initial_energy * 0.1  and once_gnr == 0):
            print('General request from ', node_ID, " : ", time.ctime(time.time()) )
            queue[node_ID] = [node_ID, 1, 0] 
            g_queue[node_ID] = 1
            
            once_gnr = 1
        
        elif(battery <= nodes_initial_energy * 0.1 and battery > 0  and once_emg == 0):
            print('Emergency request from ', node_ID, " : ", time.ctime(time.time()) )
            queue[node_ID] = [node_ID, 2, 0]
            e_queue[node_ID] = 1
            
            once_emg = 1
            
        elif(battery < 0):
            print(node_ID, " is Dead" ," : ", time.ctime(time.time()) )    
            
            dead_list.append(node_ID)
            print("Total number of dead nodes: ", len(dead_list))
            
            
            
    ##------------------------
    
        if(after_recharge[node_ID, 4] == 1):
                
                battery = after_recharge[node_ID, 1]
                print(node_ID , " recharged!" , ", Battery: ", battery, " : ", time.ctime(time.time()))
                after_recharge[node_ID, 4] = 0
    
    
    dead_list.append(node_ID)
    print("Dead nodes list: \n", dead_list)
            
    

    return



    
##---------------------------------------------------------------------------##


def rechargeBattery(node_ID):
    '''
    The function rechargeBattery(node_ID) is used by the Mobile Charger to recharge a Sensor node.
    It takes node_ID as parameter and reset the energy status of the node to its initial state.
     
    '''
    
    recharge_done = 1
    
    for i in range(len(rechargedList)):
        if(node_ID == rechargedList[i]):
            battery = nodes_initial_energy
            once_gnr  = 0
            once_emg = 0
            del rechargedList[i]
        
            after_recharge[node_ID] = [node_ID, battery, once_gnr, once_emg, recharge_done]
    
    
    return None
    
    
    
##---------------------------------------------------------------------------##


    
def move_MC(node_ID, x_old_mc, y_old_mc):
    '''
    The function move_MC() is used by the Mobile Charger to simulate the movement of the car from one place to another.
    The function takes 3 parameters: node_ID, and node's x and y coordinates.
    Function calculates the distance and time required for the MC to reach the node from it's current position.
    
    Function returns the new x and y coordinates for the MC, required time for the movement and energy lost due to the movement.
    '''
    
    dist = np.sqrt(np.square(x_old_mc - position[node_ID, 0] ) + np.square(y_old_mc - position[node_ID, 1] ) )
    total_distance_covered_by_MC.append(dist)
    
    
    moving_time = (dist/240)   # velocity of MC = 4 meter/sec or 240 meter/min.
    
    x_new_mc = position[node_ID, 0]
    y_new_mc = position[node_ID, 1]
    energy_lost_for_movement = int(dist * 5)  # Energy used for movement = 5 Joule/m.
    
    return moving_time, x_new_mc, y_new_mc, energy_lost_for_movement    
    
    
    
##------------------------Mobile Charger---------------------------------------------------##    
    
    
    
    
def MobileCharger(mc_ID):
    '''
    The function MobileCharger() creates MC. It takes a parameter 'mc_ID' which is the ID of the charger.
    The charger stays at base initially and waits for charging requests from the nodes.
    It uses Dynamic Active Zone algorithm to charge the nodes. The nodes with emergency requests are prioritized.
     
    MC goes back to the Base Station to recharge itself when it's remaining energy become 10% of the initial energy.
    
    '''
    
    battery = mc_initial_energy
    x_mc = 0
    y_mc = 0
    
    t1 = time.time()
    
    
    while(battery > 0):
        
        got_recharge_request = 0
        
        for i in range(len(g_queue)):
            if(g_queue[i] == 1):
                got_recharge_request = 1
            if(e_queue[i] == 1):
                got_recharge_request = 1
                
        
        if(got_recharge_request):
        
            if(len(g_queue) < 3):
                # Wait sometime to check whether any more requests arrive, so that better route can be made.
                time.sleep(10)
            
            
    ##--------------<Handle Recharge Queue>----------------------
                    
            #-> Sort recharge queue based on request type.        
            temp_queue = queue
            
            
            for m in range(len(temp_queue)):
                current_dist_of_node = np.sqrt(np.square(x_mc - position[m,0] ) + np.square(y_mc - position[m,1] ) )
                temp_queue[m,2] = -1 * current_dist_of_node
                
            ind = np.lexsort((temp_queue[::,2], temp_queue[:,1]))
            tempa = temp_queue[ind]
            tempc = tempa[::-1]
            temp_queue = tempc   
            
            print("Total number of dead nodes: ", len(dead_list))
            
            for m in range(len(temp_queue)):
                temp_queue[m,2] = -1 * temp_queue[m,2]
            
            
            
            rechargedList.append(temp_queue[0,0])
            
            
    ##----------<MOVE>-Go to node------------------------------
            
            # Call move_MC() function to get the distance upto node.
            moving_time, x_pos, y_pos, energy_lost_for_movement = move_MC(temp_queue[0,0], x_mc, y_mc)
            
            print("MC starting to move from : ", x_mc, y_mc, " : ", time.ctime(time.time()))
            
            time.sleep(moving_time) # Wait until MC moves.
            
            # Update position of MC.
            x_mc = x_pos
            y_mc = y_pos
            print("MC has reached at node: ", temp_queue[0,0], ", Time taken: ", moving_time)
            
            battery = battery - energy_lost_for_movement
            
            #-> call rechargeBattery() function to recharge the first node of the queue
            # Wait until MC finishes charging the node.
            
            time.sleep(charging_time) 
            rechargeBattery(temp_queue[0,0])   
            
            battery = battery - energy_lost_for_charging
            
            #-> Remove the recharged node from 'queue' and 'primary_queue'.
            queue[temp_queue[0,0]] = [0,0,0]
            g_queue[temp_queue[0,0]] = 0
            e_queue[temp_queue[0,0]] = 0   
            
            print_queue = []
            
            for k in range(len(g_queue)):
                if(g_queue[k] == 1):
                    print_queue.append(k)
            
            print_queue = []
            
            for k in range(len(e_queue)):
                if(e_queue[k] == 1):
                    print_queue.append(k)
            
        
    ##-----------------<Energy Loss due to charge transfer>----------
            
        # MC Returns to the base to recharge
        
        if(battery < mc_initial_energy * 0.10):
            battery, x_mc, y_mc = rechargeMc(x_mc, y_mc)
            
            print("MC is fresh again. Battery Status: ", battery, " at: ", x_mc, y_mc) 
    
        #t2 = time.time()
        #time_elapsed = t2 - t1
        
        #if(time_elapsed == 6000):
        #    print("Total distance covered: ", total_distance_covered_by_MC)    
    
    print("MC is dead")
    return



##---------------------------------------------------------------------------##



def rechargeMc(x_mc, y_mc):
    '''
    The function rechargeMC() is used to recharge the Mobile Charger.
    The function takes the current x and y coordinates of the MC and calculates the time and energy for the MC to move to 
    the base which is at (0,0).
    
    It resets the battery of the MC to the initial condition and updates the MC's position at (0,0).
    It uses time.sleep() to emulate the traveling time to the Base Station and the recharging time. 
    
    '''
    
    time_to_recharge_MC = 120
    
    # Calculate distance upto base from MC.
    dist = np.sqrt(np.square(x_mc - 0) + np.square(y_mc - 0) )
    moving_to_base = int(dist/5)
    
    total_distance_covered_by_MC.append(dist)
    print("Total distance covered: ", sum(total_distance_covered_by_MC), " @ ", time.ctime(time.time()))
    
    # Wait until the MC reaches to the base. 
    print("MC is returning to base!")
    
    time.sleep(moving_to_base + time_to_recharge_MC )
    battery = mc_initial_energy
    
    return battery, 0, 0
    


####------------Run Simulations------------------------------------####

# Create Sensor Nodes. Threading is used to speed up the simulation.

Node_list = []
for i in range(number_of_nodes):
    node = threading.Thread(target=Sensor, args=(i,))
    Node_list.append(node)
    node.start()
    
# Create Mobile Chargers. Threading is used to speed up the simulation.

MC_list = []
for i in range(1):
    mc = threading.Thread(target=MobileCharger, args=(i,))
    MC_list.append(mc)
    mc.start()
    
    
    
        




      