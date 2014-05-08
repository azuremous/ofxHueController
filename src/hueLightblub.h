//
//  hueLightblub.h
//
//  Created by azuremous on 5/2/14.
//
//

#pragma once

#include "ofUtils.h"

typedef enum {
    NONE_ALERT,
    SELECT_ALERT,
    LSELECT_ALERT
}HUE_ALERT;

class hueLightblub {
private:
    float hue,sat,bri;
    string name;
    
protected:
    string lightOn(bool simpleSet, bool transition = false, const int &transitionMS = 1);
    
public:
    hueLightblub(string _name);
    string setColor(const float &_hue, bool simpleSet, bool setAsInt16w = false, const float &_sat = 255, const float &_bri = 255, bool transition = false, const int &transitionMS = 1);
    string lightOff(bool transition = false, const int &transitionMS = 1);
    string setAlert(HUE_ALERT _alert);
    string runColorLoop();
    
};

