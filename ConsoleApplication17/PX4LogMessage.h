#pragma once
#include "functions.h"

class PX4LogMessageDescription;

class PX4LogMessage {
//private:
	//PX4LogMessageDescription *description;
	//vector<boost::any> data;

public:
	PX4LogMessageDescription *description;
	vector<boost::any> data;

	PX4LogMessage() {
		data = {0};
	}

	PX4LogMessage(PX4LogMessageDescription *_description, vector<boost::any> _data) {
		description = _description;
		data = _data;
	}

	boost::any get(string field) {
		int idx = 0;
		unordered_map<string, int>::const_iterator gots = description->fieldsMap.find(field);
		if (gots == description->fieldsMap.end()) {
			cout << field << " not found" << endl;
		}
		else {
			idx = gots->second;
		}
		
		boost::any anyone;
		anyone = data[idx];
		return anyone;
	}
};