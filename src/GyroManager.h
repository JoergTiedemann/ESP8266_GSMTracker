#ifndef GYRO_H
#define GYRO_H

#include <Arduino.h>


class CGyroManager
{

private:

public : 
    void begin();
    void InitWakeOnMotion();
    void loop();
    bool isMotionDetected();
    void ResetMotion();
    

    CGyroManager()
	{
	}

    ~CGyroManager()
	{
	}

};

extern CGyroManager GyroManager;

#endif
