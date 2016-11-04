#include <iostream>
#include <string>
#include <vector>
#include <ros/ros.h>
#include <toaster_msgs/Fact.h>
#include <toaster_msgs/FactList.h>

/*
Call back for the topic agent_monitor/fact_list
*/

using namespace std;

string humanJoint, robotJoint;
double thresholdHuman, thresholdRobot, thresholdHumanSupport, thresholdRobotSupport;
vector<string> toMonitorObjects, toMonitorSupports, toMonitorAgents, areasReachable;
string robotName, areaVisible, robotArea, humanArea;
vector<toaster_msgs::Fact> distanceFacts, areaFacts;
bool isReachableByArea, isReachableByArea2;

bool toMonitorObject(string object){

    //We check if the object is in the list
    for(vector<string>::iterator it = toMonitorObjects.begin(); it != toMonitorObjects.end(); it++){
       if(*it == object){
          return true;
       }
    }

    return false;
}

bool toMonitorSuport(string object){

    //We check if the object is in the list
    for(vector<string>::iterator it = toMonitorSupports.begin(); it != toMonitorSupports.end(); it++){
       if(*it == object){
          return true;
       }
    }

    return false;
}

bool toMonitorAgent(string agent){

    //We check if the object is in the list
    for(vector<string>::iterator it = toMonitorAgents.begin(); it != toMonitorAgents.end(); it++){
       if(*it == agent){
          return true;
       }
    }

    return false;
}

bool isAreaReachable(string area){

    //We check if the object is in the list
    for(vector<string>::iterator it = areasReachable.begin(); it != areasReachable.end(); it++){
       if(*it == area){
          return true;
       }
    }

    return false;
}

void agentFactListCallback(const toaster_msgs::FactList::ConstPtr& msg){

    distanceFacts = msg->factList;

}

void areaFactListCallback(const toaster_msgs::FactList::ConstPtr& msg){

    areaFacts = msg->factList;

}

int main(int argc, char** argv) {
   
    ros::init(argc, argv, "move3d_facts");
    ros::NodeHandle node;
    ros::Rate loop_rate(30);
    node.getParam("humanJoint", humanJoint);
    node.getParam("robotJoint", robotJoint);
    node.getParam("thresholdReachable/humanObject", thresholdHuman);
    node.getParam("thresholdReachable/robotObject", thresholdRobot);
    node.getParam("thresholdReachable/humanSupport", thresholdHumanSupport);
    node.getParam("thresholdReachable/robotSupport", thresholdRobotSupport);
    node.getParam("objectToMonitor", toMonitorObjects);
    node.getParam("supportToMonitor", toMonitorSupports);
    node.getParam("areasReachable", areasReachable);
    node.getParam("agentToMonitor", toMonitorAgents);
    node.getParam("robotName", robotName);
    node.getParam("areaVisible", areaVisible);
    node.getParam("isReachableByArea", isReachableByArea);
    node.getParam("isReachableByArea2", isReachableByArea2);
    node.getParam("humanArea", humanArea);
    node.getParam("robotArea", robotArea);

    // Publishing
    ros::Publisher facts_pub = node.advertise<toaster_msgs::FactList>("move3d_facts/factList", 1000);

    ros::Subscriber sub = node.subscribe("agent_monitor/factList", 1000, agentFactListCallback);
    ros::Subscriber subArea = node.subscribe("area_manager/factList", 1000, areaFactListCallback);

    bool isReachableBy, isVisibleBy, isOn, isIn;
    toaster_msgs::FactList factList_msg;
    toaster_msgs::Fact fact_msg;
    
    /************************/
    /* Start of the Ros loop*/
    /************************/

    while (node.ok()) {
      //Get which facts we should compute from parameters   
      node.getParam("/computedFacts/isReachableBy", isReachableBy);
      node.getParam("/computedFacts/isVisibleBy", isVisibleBy);
      node.getParam("/computedFacts/isOn", isOn);
      node.getParam("/computedFacts/isIn", isIn);
        
        if(isReachableBy){
            if(isReachableByArea){
                for(vector<toaster_msgs::Fact>::iterator it = areaFacts.begin(); it != areaFacts.end(); it++){
                    if(toMonitorObject(it->subjectId) || toMonitorSuport(it->subjectId)){
                        if(it->property == "IsInArea" && it->targetId == humanArea){
                            fact_msg.property = "isReachableBy";
                            fact_msg.propertyType = "state";
                            fact_msg.subjectId = it->subjectId;
                            fact_msg.targetId = "HERAKLES_HUMAN1";
                            fact_msg.factObservability = 1.0;
                            factList_msg.factList.push_back(fact_msg);
                        }
                        if(it->property == "IsInArea" && it->targetId == robotArea){
                            fact_msg.property = "isReachableBy";
                            fact_msg.propertyType = "state";
                            fact_msg.subjectId = it->subjectId;
                            fact_msg.targetId = robotName;
                            fact_msg.factObservability = 1.0;
                            factList_msg.factList.push_back(fact_msg);
                        }
                    }
                }
            }else if(isReachableByArea2){
                //we look for agents positions
                map<string, string> agentsAreas;
                for(vector<toaster_msgs::Fact>::iterator it = areaFacts.begin(); it != areaFacts.end(); it++){
                    if(it->property == "IsInArea" && isAreaReachable(it->targetId)){
                        if(toMonitorAgent(it->subjectId) || it->subjectId == "pr2"){
                            agentsAreas[it->subjectId] = it->targetId;
                        }
                    }
                }
                for(vector<toaster_msgs::Fact>::iterator it = areaFacts.begin(); it != areaFacts.end(); it++){
                    if(toMonitorObject(it->subjectId) || toMonitorSuport(it->subjectId)){
                        if(it->property == "IsInArea" && isAreaReachable(it->targetId)){
                            for(vector<string>::iterator ita = toMonitorAgents.begin(); ita != toMonitorAgents.end(); ita++){
                                if(agentsAreas[*ita] == it->targetId){
                                    fact_msg.property = "isReachableBy";
                                    fact_msg.propertyType = "state";
                                    fact_msg.subjectId = it->subjectId;
                                    fact_msg.targetId = *ita;
                                    fact_msg.factObservability = 1.0;
                                    factList_msg.factList.push_back(fact_msg);
                                }
                            }
                            if(agentsAreas["pr2"] == it->targetId){
                                fact_msg.property = "isReachableBy";
                                fact_msg.propertyType = "state";
                                fact_msg.subjectId = it->subjectId;
                                fact_msg.targetId = robotName;
                                fact_msg.factObservability = 1.0;
                                factList_msg.factList.push_back(fact_msg);
                            }
                        }
                    }
                }
            }{
                for(vector<toaster_msgs::Fact>::iterator it = distanceFacts.begin(); it != distanceFacts.end(); it++){
                    if(it->property == "Distance"){
                        if(toMonitorObject(it->targetId)){
                            if(it->subjectId == humanJoint && it->doubleValue < thresholdHuman){
                                fact_msg.property = "isReachableBy";
                                fact_msg.propertyType = "state";
                                fact_msg.subjectId = it->targetId;
                                fact_msg.targetId = it->subjectOwnerId;
                                fact_msg.factObservability = 1.0;
                                factList_msg.factList.push_back(fact_msg);
                            }
                            if(it->subjectId == robotJoint && it->doubleValue < thresholdRobot){
                                fact_msg.property = "isReachableBy";
                                fact_msg.propertyType = "state";
                                fact_msg.subjectId = it->targetId;
                                fact_msg.targetId = robotName;
                                fact_msg.factObservability = 1.0;
                                factList_msg.factList.push_back(fact_msg);
                            }
                        }
                        if(toMonitorSuport(it->targetId)){
                            if(it->subjectId == humanJoint && it->doubleValue < thresholdHumanSupport){
                                fact_msg.property = "isReachableBy";
                                fact_msg.propertyType = "state";
                                fact_msg.subjectId = it->targetId;
                                fact_msg.targetId = it->subjectOwnerId;
                                fact_msg.factObservability = 1.0;
                                factList_msg.factList.push_back(fact_msg);
                            }
                            if(it->subjectId == robotJoint && it->doubleValue < thresholdRobotSupport){
                                fact_msg.property = "isReachableBy";
                                fact_msg.propertyType = "state";
                                fact_msg.subjectId = it->targetId;
                                fact_msg.targetId = robotName;
                                fact_msg.factObservability = 1.0;
                                factList_msg.factList.push_back(fact_msg);
                            }
                        }
                    }
                }
            }
        }
        
        if(isVisibleBy){
            vector<string> objectsVisible, agentVisible;
            for(vector<toaster_msgs::Fact>::iterator it = areaFacts.begin(); it != areaFacts.end(); it++){
                if(it->property == "IsInArea" && it->targetId == areaVisible){
                    if(toMonitorAgent(it->subjectId)){
                        agentVisible.push_back(it->subjectId);
                    }else if(toMonitorObject(it->subjectId) || toMonitorSuport(it->subjectId)){
                        objectsVisible.push_back(it->subjectId);
                    }
                }
            }
            for(vector<string>::iterator ito = objectsVisible.begin(); ito != objectsVisible.end(); ito++){
                for(vector<string>::iterator ita = agentVisible.begin(); ita != agentVisible.end(); ita++){
                    fact_msg.property = "isVisibleBy";
                    fact_msg.propertyType = "state";
                    fact_msg.subjectId = *ito;
                    fact_msg.targetId = *ita;
                    fact_msg.factObservability = 1.0;
                    factList_msg.factList.push_back(fact_msg);
                }
                fact_msg.property = "isVisibleBy";
                fact_msg.propertyType = "state";
                fact_msg.subjectId = *ito;
                fact_msg.targetId = robotName;
                fact_msg.factObservability = 1.0;
                factList_msg.factList.push_back(fact_msg);
            }
            for(vector<string>::iterator ito = agentVisible.begin(); ito != agentVisible.end(); ito++){
                for(vector<string>::iterator ita = agentVisible.begin(); ita != agentVisible.end(); ita++){
                    if(*ito != *ita){
                        fact_msg.property = "isVisibleBy";
                        fact_msg.propertyType = "state";
                        fact_msg.subjectId = *ito;
                        fact_msg.targetId = *ita;
                        fact_msg.factObservability = 1.0;
                        factList_msg.factList.push_back(fact_msg);
                        fact_msg.property = "isVisibleBy";
                        fact_msg.propertyType = "state";
                        fact_msg.subjectId = *ita;
                        fact_msg.targetId = *ito;
                        fact_msg.factObservability = 1.0;
                        factList_msg.factList.push_back(fact_msg);
                    }
                }
                fact_msg.property = "isVisibleBy";
                fact_msg.propertyType = "state";
                fact_msg.subjectId = *ito;
                fact_msg.targetId = robotName;
                fact_msg.factObservability = 1.0;
                factList_msg.factList.push_back(fact_msg);
                fact_msg.property = "isVisibleBy";
                fact_msg.propertyType = "state";
                fact_msg.subjectId = robotName;
                fact_msg.targetId = *ito;
                fact_msg.factObservability = 1.0;
                factList_msg.factList.push_back(fact_msg);
            }

        }
        
        if(isOn){

        }
        
        if(isIn){

        }


        facts_pub.publish(factList_msg);
        factList_msg.factList.clear();
        ros::spinOnce();

        loop_rate.sleep();
    }
    return 0;
}
