//
//  hueLightblub.cpp
//
//  Created by azuremous on 5/2/14.
//
//

#include "hueLightblub.h"


//-----------------------------------------------------------------------------------
/*public*/hueLightblub::hueLightblub(string _name)
:hue(10000)//0~65535 0:red 25500:green 46920 :blue 65535:red
,sat(255)//0~255
,bri(255)//0~255
{ name = _name; }

//-----------------------------------------------------------------------------------
/*public*/string hueLightblub::setColor(const float &_hue, bool simpleSet, bool setAsInt16w, const float &_sat, const float &_bri, bool transition, const int &transitionMS){
    if (!setAsInt16w) { hue = (int)ofMap(_hue, 0, 255, 0, 65535); }
    else { hue = _hue; }
    sat = _sat;
    bri = _bri;
    
    return lightOn(simpleSet, transition, transitionMS);
}

//-----------------------------------------------------------------------------------
/*public*/string hueLightblub::lightOff(bool transiton, const int &transitionMS){
    
    string form;
    if (transiton) { form = "{\"on\": false, \"transitiontime\" : "+ofToString(transitionMS)+ " }"; }
    else { form = "{ \"on\" : false }"; }
    
    return form;
}

//-----------------------------------------------------------------------------------
/*public*/string hueLightblub::setAlert(HUE_ALERT _alert){
    string alert;
    if (_alert == NONE_ALERT) { alert = "none"; }
    else if (_alert == SELECT_ALERT) { alert = "select"; }
    else if (_alert == LSELECT_ALERT) { alert = "lselect"; }
    string form = "{ \"alert\" : " + alert+ " }";
    
    return form;
    
}

//-----------------------------------------------------------------------------------
/*public*/string hueLightblub::runColorLoop(){ return "{ \"effect\" : colorloop }"; }

//-----------------------------------------------------------------------------------
/*protected*/string hueLightblub::lightOn(bool simpleSet, bool transition, const int &transitionMS){
    stringstream json;
    json << "{ \"on\" : true, " <<endl;
    if (transition) { json << "\"transitiontime\":"<< ofToString(transitionMS) << ","<<endl; }
    if(!simpleSet) {
        json << "\"sat\" : " << ofToString(sat) <<", "<<endl;
        json << "\"bri\" : " << ofToString(bri) <<", "<<endl;
    }
    json << "\"hue\" : " << ofToString(hue)<<endl;
    json <<" }" ;
    
    return json.str();
}