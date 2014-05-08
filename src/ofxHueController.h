//
//  ofxHueController.h
//
//  Created by azuremous on 5/1/14.
//
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
#include "hueLightblub.h"

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
    vector<hueLightblub *>lightbulbs;
    string address, userName, apiMethod;
    CONTROLL_MODE mode;
    bool sendData;
    bool result;
    Poco::Condition condition;
    
protected:
    void setMode(CONTROLL_MODE _mode);
    bool sendCommand(string _action, string _command);
    string getLog(string _s);
    queue<hueForm> forms;
public:
    ofxHueController();
    
    void setup(string url, string name);
    void setBlub(int size);
    void start();
    void stop();
    void addAction(const hueForm & f);
    void threadedFunction();

    //user
    string getUserInfo();
    string addNewUser(string url, string devicetypeName ,string name);
    
    //light
    string selectLight(int n);
    bool lightOn(int n, const float &_hue, bool simpleSet = true, bool setAsInt16w = false, const float &_sat = 255, const float &_bri = 255, bool transition = false, const int &transitionMS = 1);
    bool lightOff(int n, bool transition = false, const int &transitionMS = 1);
    
    //group
    string getGroupInfo(int groupID);
    string createGroup(string groupName, int groupID, const vector<int> &blubID);
    string setGroup(int groupID, const vector<int> &blubID);
    bool setGroupStatus(int groupID, const float &_hue, bool simpleSet = true, bool setAsInt16w = false, const float &_sat = 255, const float &_bri = 255, bool transition = false, const int &transitionMS = 1);
    bool turnOffAll(int groupID, bool transition = false, const int &transitionMS = 1);
    
    
};