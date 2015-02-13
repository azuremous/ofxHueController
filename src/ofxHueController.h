//
//  ofxHueController.h
//
//  Created by azuremous on 5/1/14.
// test
//

#pragma once

#include <queue>

#include "Poco/Mutex.h"
#include "Poco/Condition.h"
#include "Poco/Exception.h"
#include "Poco/URI.h"
#include "Poco/StreamCopier.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"

#include "ofThread.h"
#include "ofUtils.h"
#include "ofEventUtils.h"

typedef enum {
    NONE_ALERT,
    SELECT_ALERT,
    LSELECT_ALERT
}HUE_ALERT;

typedef enum {
    LIGHT_CONTROLL_MODE,
    GROUP_CONTROLL_MODE,
    SCHEDULE_CONTROLL_MODE
}CONTROLL_MODE;

typedef enum {
    HUE_OFF,
    HUE_ON,
    HUE_GROUP_OFF,
    HUE_GROUP_ON
}HUE_STATUS;

typedef enum {
    TIMEOUT_ERROR = 0,
    UNAUTHORIZED_USER_ERROR = 1,
    JSON_ERROR = 2,
    RESOURCE_ERROR = 3,
    METHOD_ERROR = 4,
    MISSING_PARAMETERS_ERROR = 5,
    PARAMETERS_ERROR = 6,
    INVALID_VALUE_ERROR = 7,
    PARAMETER_MODIFY_ERROR = 8,
    INTERNAL_ERROR = 901,
    LINK_BUTTON_ERROR = 101,
    DEVICE_OFF_ERROR = 201,
    GROUP_FULL_ERROR = 301,
    DEVICE_GROUP_FULL_ERROR = 302
    
}HUE_ERROR;

typedef struct {
    float hue;
    float sat = 255;
    float bri = 255;
    bool simpleSet = true;
    bool setAsInt16w = false;
    bool transition = false;
    int transitionMS = 1;
    int id;
    HUE_STATUS status;
    
}hueForm;

using namespace Poco::Net;
using namespace Poco;
using Poco::Exception;
using Poco::Net::HTTPClientSession;

class ofxHueController: public ofThread {
    
private:
    string address, userName, apiMethod;
    bool sendData, result , isOn, isOns;
    Poco::Condition condition;
    vector<string>hue_ip;
    CONTROLL_MODE mode;
    
protected:
    void start();
    void setMode(CONTROLL_MODE _mode);
    void notifyEvent(int _id);
    
    string getLog(string _s);
    int errorID(string error);
    bool sendCommand(string _action, string _command);
    
    queue<hueForm> forms;
    

public:
    ofxHueController();
    
    bool setup(string name, int ip = 0);
    void stop();
    void addAction(const hueForm & f);
    void threadedFunction();

    //portal
    bool discoverBridges();
    
    //user
    string getUserInfo();
    bool addNewUser(string url, string devicetypeName ,string name);
    
    //light
    string getAllLights();
    bool selectLight(int n, bool selected = true);
    bool lightOn(int n, const float &_hue, bool simpleSet = true, bool setAsInt16w = false, const float &_sat = 255, const float &_bri = 255, bool transition = false, const int &transitionMS = 1);
    bool lightOff(int n, bool transition = false, const int &transitionMS = 1);
    
    //group
    string getAllGroupInfo();
    string getGroupInfo(int groupID);
    bool createGroup(string groupName, int groupID, const vector<int> &blubID);
    bool setGroup(int groupID, const vector<int> &blubID);
    bool setGroupStatus(int groupID, const float &_hue, bool simpleSet = true, bool setAsInt16w = false, const float &_sat = 255, const float &_bri = 255, bool transition = false, const int &transitionMS = 1);
    bool turnOffAll(int groupID, bool transition = false, const int &transitionMS = 1);
    
    vector<string>getIPaddress() const { return hue_ip; }
    
    ofEvent<HUE_ERROR> errorAlert;
    
};