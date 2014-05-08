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
/*public*/void ofxHueController::setup(string url, string name){
    address = "http://"+ url + "/api/";
    userName = name;
}

//-----------------------------------------------------------------------------------
/*public*/void ofxHueController::setBlub(int size){
    lightbulbs.clear();
    
    for (int i = 0; i < size; i++) {
        hueLightblub * h = new hueLightblub(ofToString(i+1));
        lightbulbs.push_back(h);
    }
}

//-----------------------------------------------------------------------------------
/*public*/void ofxHueController::start(){
    startThread(true, false);
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

//user

//-----------------------------------------------------------------------------------
/*public*/string ofxHueController::getUserInfo(){
    if (address.empty()){
        ofLog(OF_LOG_ERROR,"no address!");
        return;
    }
    string s = "curl -X GET " + address + userName;
    return getLog(s);
}

//-----------------------------------------------------------------------------------
/*public*/string ofxHueController::addNewUser(string url, string devicetypeName ,string name) {
    address = "http://"+ url + "/api/" ;
    userName = name;
    string body = "{\"username\":\"" + name + "\", \"devicetype\":\"" + devicetypeName + "\"}";
    
    string s = "curl -X POST " + address + " -d \'"+body+"\'";
    return getLog(s);
}

//light

//-----------------------------------------------------------------------------------
/*public*/string ofxHueController::selectLight(int n){
    if (n <= 0 || n >= lightbulbs.size()) {
        ofLog(OF_LOG_ERROR, "no blob found!!");
        return;
    }
    setMode(LIGHT_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(n+1)+"/state";
    string body = lightbulbs[n]->setAlert(SELECT_ALERT);
    string s = "curl -X PUT " + action + " -d \'"+body+"\'";
    //return getLog(s);
    system(s.c_str());
    //sendCommand(action, body);
    return "select light:" + ofToString(n+1);
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::lightOn(int n, const float &_hue, bool simpleSet, bool setAsInt16w, const float &_sat, const float &_bri, bool transiton, const int &transitionMS){
    
    setMode(LIGHT_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(n+1)+"/state";
    string body = lightbulbs[n]->setColor(_hue, simpleSet, setAsInt16w, _sat, _bri, transiton, transitionMS);
    return sendCommand(action, body);
   
}

//-----------------------------------------------------------------------------------
/*public*/bool ofxHueController::lightOff(int n, bool transiton, const int &transitionMS){

    setMode(LIGHT_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(n+1)+"/state";
    string body = lightbulbs[n]->lightOff(transiton, transitionMS);
    return sendCommand(action, body);
}

//group

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
/*public*/string ofxHueController::createGroup(string groupName, int groupID, const vector<int> &blubID){
    if (groupID <= 0 || groupID >= 32) {
        ofLog(OF_LOG_ERROR, "incorrect group ID!!");
        return;
    }
    if (blubID.size() <= 0 || blubID.size() > lightbulbs.size()){
        ofLog(OF_LOG_ERROR, "not correct blub size");
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
    return getLog(s);
}

//-----------------------------------------------------------------------------------
/*public*/string ofxHueController::setGroup(int groupID, const vector<int> &blubID){
    if (groupID <= 0 || groupID >= 32) {
        ofLog(OF_LOG_ERROR, "incorrect group ID!!");
        return;
    }
    if (blubID.size() <= 0 || blubID.size() > lightbulbs.size()){
        ofLog(OF_LOG_ERROR, "not correct blub size");
        return;
    }
    setMode(GROUP_CONTROLL_MODE);
    string action = address+userName+"/"+apiMethod+"/"+ofToString(groupID);
    //cout<<"action:"<<action<<endl;
    
    stringstream body;
    body << "{";
    body << "\"lights\": [";
    for (int i = 0; i < blubID.size(); i++) {
        if (i == blubID.size()-1) { body<<"\""<< ofToString(blubID[i]+1) <<"\""; }
        else{ body<<"\""<< ofToString(blubID[i]+1) <<"\""<< ","; }
    }
    body <<"]}";
    
    //cout<<"body:"<<body.str()<<endl;
    string s = "curl -X PUT " + action + " -d \'"+body.str()+"\'";
    return getLog(s);
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
    else { h = _hue; }
    stringstream body;
    body << "{ \"on\" : true, ";
    if (transition) { body << "\"transitiontime\" : "<< ofToString(transitionMS) << ", "; }
    if (!simpleSet) {
        body << "\"sat\" : " << ofToString(_sat) <<", ";
        body << "\"bri\" : " << ofToString(_bri) <<", ";
    }
    body << "\"hue\" : " << ofToString(h);
    body <<" }" ;
    
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
/*protected*/void ofxHueController::setMode(CONTROLL_MODE _mode){
    mode = _mode;
    if (mode == LIGHT_CONTROLL_MODE) { apiMethod = "lights"; }
    else if (mode == GROUP_CONTROLL_MODE) { apiMethod = "groups"; }
    else if (mode == SCHEDULE_CONTROLL_MODE) { apiMethod = "schedules"; }
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
            HTTPClientSession session(host, uri.getPort());
            HTTPRequest req(HTTPRequest::HTTP_PUT, path, HTTPMessage::HTTP_1_1);
            session.setTimeout(Poco::Timespan(timeOut,0));
            req.setContentLength( _command.length() );
            
            std::ostream& os = session.sendRequest(req);
            std::istringstream is( _command );
            Poco::StreamCopier::copyStream(is, os);
            req.setContentLength( _command.length() );
            
            sendData = true;
            return sendData;
            
        }
        
        return false;
    } catch (Exception& exc) {
        ofLog( OF_LOG_ERROR, "ofxHueController::sendCommand(%s) to %s >> Exception: %s\n", _command.c_str(), uri.toString().c_str(), exc.displayText().c_str() );
        //turnOffAll(1,true,1);
		return false;
    }
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
