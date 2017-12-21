#pragma once
#include "functions.h"
#include "PX4LogMessageDescription.h"
#include<list>

class PX4LogMessage {
//private:
	PX4LogMessageDescription *description;
	//template<typename T>
	//list<T> data;
	//list<string> data;
	//vector<string> data;
	vector<boost::any> data;

public:
	PX4LogMessage(PX4LogMessageDescription *_description, /*list<string>*/vector<boost::any> _data) {
		this->description = _description;
		this->data = _data;
	}
};