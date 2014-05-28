//
//  ofxHueController.cpp
//
//  Created by azuremous on 5/2/14.
//
//

#include "ofxHueController.h"

//-----------------------------------------------------------------------------------
/*public*/ofxHueController::ofxHueController():sendData(false)
{}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::setup(string name, int ip){
    
    address = "http://"+ hue_ip[ip] + "/api/";
    userName = name;
    string error = getUserInfo();
    int error_id = errorID(error);
    if (error == "error" || error_id > 0) {
        notifyEvent(error_id);
        return false;
    }
    start();
    return true;
}

//-----------------------------------------------------------------------------------
/*public*/void ofxHueController::stop(){
    stopThread();
    condition.signal();
}

//-----------------------------------------------------------------------------------
/*public*/void ofxHueController::addAction(const hueForm & f){
    lock();
    forms.push(f);
    condition.signal();
    unlock();
}

//-----------------------------------------------------------------------------------
/*public*/void ofxHueController::threadedFunction(){
    lock();
    while (isThreadRunning()) {
        if (forms.size() > 0) {
            hueForm form = forms.front();
            unlock();
            if (form.status == HUE_ON) {
                ofLog(OF_LOG_VERBOSE,"(thread running) lightOn");
                result = lightOn(form.id, form.hue, form.simpleSet, form.setAsInt16w, form.sat, form.bri, form.transition, form.transitionMS);
            }else if (form.status == HUE_OFF) {
                ofLog(OF_LOG_VERBOSE,"(thread running) lightOff");
                result = lightOff(form.id, form.transition, form.transitionMS);
            }else if (form.status == HUE_GROUP_ON) {
                ofLog(OF_LOG_VERBOSE,"(thread running) set group status");
                result = setGroupStatus(form.id, form.hue, form.simpleSet, form.setAsInt16w, form.sat, form.bri, form.transition, form.transitionMS);
            }else if (form.status == HUE_GROUP_OFF) {
                ofLog(OF_LOG_VERBOSE,"(thread running) turnOffAll");
                result = turnOffAll(form.id, form.transition, form.transitionMS);
            }
            lock();
            if (!result) {
                sendData = false;
                forms.pop();
                ofLog(OF_LOG_VERBOSE,"pop form");
            }
        }
        if(forms.empty()){
            ofLog(OF_LOG_VERBOSE,"empty, waiting");
            sendData = false;
    		condition.wait(mutex);
    	}
    }
    unlock();
}

//portal
//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::discoverBridges(){
    string s = getLog("curl -X GET www.meethue.com/api/nupnp");
    
    int error_id = errorID(s);
    if (error_id > 0) {
        notifyEvent(error_id);
        return false;
    }
    char *resultChar = new char[s.length()+1];
    strncpy(resultChar, s.c_str(), s.length());
    string _buffer = "";
    bool addToString = false;
    
    vector<string>address;
    for (int i = 0; i < s.length(); i++) {
        if (resultChar[i] == '{' && !addToString) { addToString = true; }
        else if(resultChar[i] != '}' && addToString){ _buffer+=resultChar[i]; }
        else if(resultChar[i] == '}' && addToString){
            address.push_back(_buffer);
            addToString = false;
            _buffer = "";
        }
    }
    hue_ip.clear();
    for (int i = 0; i< address.size(); i++) {
        size_t startIpPoint = address[i].find("internalipaddress");
        if (startIpPoint != string::npos) {
            string _first = address[i].substr(startIpPoint+20);
            size_t _endIp = _first.find("\"");
            string ip = _first.substr(0, _endIp);
            hue_ip.push_back(ip);
        }
    }
    
    if (hue_ip.size() > 0) { return true; }
    return false;
}


//user

//-----------------------------------------------------------------------------------
/*public*/string ofxHueController::getUserInfo(){
    if (address.empty()){
        ofLog(OF_LOG_ERROR,"no address!");
        return "error";
    }
    string s = "curl -X GET " + address + userName;
    return getLog(s);
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::addNewUser(string url, string devicetypeName ,string name) {
    address = "http://"+ url + "/api/" ;
    userName = name;
    string body = "{\"username\":\"" + name + "\", \"devicetype\":\"" + devicetypeName + "\"}";
    
    string s = "curl -X POST " + address + " -d \'"+body+"\'";
    string error = getLog(s);
    int error_id = errorID(error);
    if(error_id > 0){
        notifyEvent(error_id);
        return false;
    }
    return true;
}

//light

//-----------------------------------------------------------------------------------
/*public*/string ofxHueController::getAllLights(){
    setMode(LIGHT_CONTROLL_MODE);
    string action = address + userName + "/" + apiMethod;
    string s = "curl -X GET " + action;
    return getLog(s);
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::selectLight(int n, bool selected){
    setMode(LIGHT_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(n)+"/state";
    string alert;
    if (selected) { alert = "\"lselect\""; }
    else{ alert = "\"none\""; }
    string body = "{ \"alert\" : " + alert+ " }";
    string s = "curl -X PUT " + action + " -d \'"+body+"\'";
    int error_id = errorID(getLog(s));
    if (error_id > 0) {
        notifyEvent(error_id);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::lightOn(int n, const float &_hue, bool simpleSet, bool setAsInt16w, const float &_sat, const float &_bri, bool transition, const int &transitionMS){
    
    setMode(LIGHT_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(n)+"/state";
    
    int hue;
    if (!setAsInt16w) { hue = (int)ofMap(_hue, 0, 255, 0, 65535); }
    else { hue = (int)_hue; }
    
    stringstream json;
    json << "{ \"on\" : true, " <<endl;
    if (transition) { json << "\"transitiontime\":"<< ofToString(transitionMS) << ","<<endl; }
    if(!simpleSet) {
        json << "\"sat\" : " << ofToString((int)_sat) <<", "<<endl;
        json << "\"bri\" : " << ofToString((int)_bri) <<", "<<endl;
    }
    json << "\"hue\" : " << ofToString(hue)<<endl;
    json <<" }" ;
    
    string body = json.str();
    return sendCommand(action, body);
    
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::lightOff(int n, bool transiton, const int &transitionMS){
    
    setMode(LIGHT_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(n)+"/state";
    
    string body;
    if (transiton) { body = "{\"on\": false, \"transitiontime\" : "+ofToString(transitionMS)+ " }"; }
    else { body = "{ \"on\" : false }"; }
    return sendCommand(action, body);
}

//group

//-----------------------------------------------------------------------------------
/*public*/string ofxHueController::getAllGroupInfo(){
    setMode(GROUP_CONTROLL_MODE);
    string action = address + userName + "/" + apiMethod;
    string s = "curl -X GET " + action;
    return getLog(s);
}

//-----------------------------------------------------------------------------------
/*public*/string ofxHueController::getGroupInfo(int groupID){
    if (groupID <= 0 || groupID >= 32) {
        ofLog(OF_LOG_ERROR, "incorrect group ID!!");
        return;
    }
    setMode(GROUP_CONTROLL_MODE);
    string action = address + userName + "/" + apiMethod + "/" + ofToString(groupID);
    
    string s = "curl -X GET " + action;
    return getLog(s);
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::createGroup(string groupName, int groupID, const vector<int> &blubID){
    if (groupID <= 0 || groupID >= 32) {
        ofLog(OF_LOG_ERROR, "incorrect group ID!!");
        return;
    }
    setMode(GROUP_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod;//+"/"+ofToString(groupID);
    //cout<<"action:"<<action<<endl;
    
    stringstream body;
    body << "{";
    body << "\"name\":" <<"\""<< groupName <<"\""<< ",";
    body << "\"lights\": [";
    for (int i = 0; i < blubID.size(); i++) {
        if (i == blubID.size()-1) { body<<"\""<< ofToString(blubID[i]+1) <<"\""; }
        else{ body<<"\""<< ofToString(blubID[i]+1) <<"\""<< ","; }
    }
    body <<"]}";
    
    //cout<<"body:"<<body.str()<<endl;
    string s = "curl -X POST " + action + " -d \'"+body.str()+"\'";
    int error_id = errorID(getLog(s));
    if (error_id > 0) {
        notifyEvent(error_id);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::setGroup(int groupID, const vector<int> &blubID){
    if (groupID <= 0 || groupID >= 32) {
        ofLog(OF_LOG_ERROR, "incorrect group ID!!");
        return;
    }
    setMode(GROUP_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(groupID);
    
    stringstream body;
    body << "{";
    body << "\"lights\": [";
    for (int i = 0; i < blubID.size(); i++) {
        if (i == blubID.size()-1) { body<<"\""<< ofToString(blubID[i]+1) <<"\""; }
        else{ body<<"\""<< ofToString(blubID[i]+1) <<"\""<< ","; }
    }
    body <<"]}";
    
    string s = "curl -X PUT " + action + " -d \'"+body.str()+"\'";
    int error_id = errorID(getLog(s));
    if (error_id > 0) {
        notifyEvent(error_id);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::setGroupStatus(int groupID, const float &_hue, bool simpleSet, bool setAsInt16w, const float &_sat, const float &_bri, bool transition, const int &transitionMS){
    if (groupID <= 0 || groupID >= 32) {
        ofLog(OF_LOG_ERROR, "no group found!!");
        return;
    }
    setMode(GROUP_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(groupID)+"/action";
    
    int h;
    if (!setAsInt16w) { h = (int)ofMap(_hue, 0, 255, 0, 65535); }
    else { h = (int)_hue; }
    stringstream body;
    body << "{ \"on\" : true, ";
    if (transition) { body << "\"transitiontime\" : "<< ofToString(transitionMS) << ", "; }
    if (!simpleSet) {
        body << "\"sat\" : " << ofToString((int)_sat) <<", ";
        body << "\"bri\" : " << ofToString((int)_bri) <<", ";
    }
    body << "\"hue\" : " << ofToString(h);
    body <<" }" ;
    
    //cout<<"body:"<<body.str()<<endl;
    
    return sendCommand(action, body.str());
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::turnOffAll(int groupID, bool transition, const int &transitionMS){
    if (groupID <= 0 || groupID >= 32) {
        ofLog(OF_LOG_ERROR, "no group found!!");
        return;
    }
    setMode(GROUP_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(groupID)+"/action";
    stringstream body;
    body << "{ ";
    if (transition) { body << "\"transitiontime\" : "<< ofToString(transitionMS) << ", "; }
    body << "\"on\" : false }";
    return sendCommand(action, body.str());
}

//-----------------------------------------------------------------------------------
/*protected*/void ofxHueController::start(){
    startThread(true, false);
}

//-----------------------------------------------------------------------------------
/*protected*/void ofxHueController::setMode(CONTROLL_MODE _mode){
    mode = _mode;
    if (mode == LIGHT_CONTROLL_MODE) { apiMethod = "lights"; }
    else if (mode == GROUP_CONTROLL_MODE) { apiMethod = "groups"; }
    else if (mode == SCHEDULE_CONTROLL_MODE) { apiMethod = "schedules"; }
}

//-----------------------------------------------------------------------------------
/*protected*/void ofxHueController::notifyEvent(int _id){
    HUE_ERROR hue_error = (HUE_ERROR)_id;
    ofNotifyEvent(errorAlert, hue_error, this);
}

//-----------------------------------------------------------------------------------
/*protected*/string ofxHueController::getLog(string _s){
    FILE * res = NULL;
    res = popen( _s.c_str() , "r");
    int bufferSize = 1024;
    string resp;
    char  buff[bufferSize];
    if (res == NULL) { ofLog(OF_LOG_ERROR, "error"); }
    else{ while( fgets( buff, bufferSize, res) ){ resp+=buff; } }
    return resp;
}

//-----------------------------------------------------------------------------------
/*protected*/int ofxHueController::errorID(string error){
    
    int find_result = error.find("type");
    if (find_result < 0) { return -1; }
    string type = error.substr(find_result+4,10);
    char * resultChar = new char[type.length()+1];
    strncpy(resultChar, type.c_str(), type.length());
    bool getErrorID = false;
    string result = "";
    int resultVal = 0;
    for (int i = 0; i < type.length(); i++) {
        if (resultChar[i] == ':' && !getErrorID) {
            getErrorID = true;
        }else if (resultChar[i] != ',' && getErrorID){
            result += resultChar[i];
        }else if (resultChar[i] == ',' && getErrorID){
            resultVal = ofToInt(result);
            getErrorID = false;
        }
    }
    return resultVal;
}

//-----------------------------------------------------------------------------------
/*protected*/bool ofxHueController::sendCommand(string _action, string _command){
    
    float timeOut = 1.0;//1.0
    Poco::URI uri = Poco::URI(_action);
    string path(uri.getPathAndQuery());
    if (path.empty()) { path = "/"; }
    string host = uri.getHost();
    
    try {
        
        if (!sendData) {
            
            ofLog(OF_LOG_VERBOSE, "send command");
            HTTPClientSession session(host, uri.getPort());
            HTTPRequest req(HTTPRequest::HTTP_PUT, path, HTTPMessage::HTTP_1_1);
            
            session.setTimeout(Poco::Timespan(timeOut,0));
            req.setContentLength( _command.length() );
            
            std::ostream& os = session.sendRequest(req);
            std::istringstream is( _command );
            
            Poco::StreamCopier::copyStream(is, os);
            req.setContentLength( _command.length() );
            
            Poco::Net::HTTPResponse res;
            istream& rs = session.receiveResponse(res);
            string responseBody = "";
            StreamCopier::copyToString(rs, responseBody);
            int error_id = errorID(responseBody);
            if (error_id > 0) { notifyEvent(error_id); }
            sendData = true;
            return sendData;
            
        }
        
        return false;
    } catch (Exception& exc) {
        ofLog( OF_LOG_ERROR, "ofxHueController::sendCommand(%s) to %s >> Exception: %s\n", _command.c_str(), uri.toString().c_str(), exc.displayText().c_str() );
        HUE_ERROR hue_error = TIMEOUT_ERROR;
        ofNotifyEvent(errorAlert, hue_error, this);
		return false;
    }
}
