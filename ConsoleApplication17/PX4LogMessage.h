#pragma once
#include "functions.h"
#include "PX4LogMessageDescription.h"
//#include<list>

class PX4LogMessage {
private:
	PX4LogMessageDescription *description;
	vector<boost::any> data;

public:
	PX4LogMessage(PX4LogMessageDescription *_description, vector<boost::any> _data) {
		description = _description;
		data = _data;
	}
};