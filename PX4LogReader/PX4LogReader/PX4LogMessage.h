#pragma once
#include "functions.h"

// ÉùÃ÷PX4LogMessageDescriptionÀà
class PX4LogMessageDescription;

class PX4LogMessage {
public:
	PX4LogMessageDescription *description;
	vector_any data;

	PX4LogMessage() {
		data = {0};
	}

	PX4LogMessage(PX4LogMessageDescription *_description, vector<bst::any> _data) {
		description = _description;
		data = _data;
	}

//	bst::any gets(string field);
};

//bst::any PX4LogMessage::gets(string field) {
	//PX4LogMessage *log = new PX4LogMessage();
	//bst::any anyone;

	//anyone = description->name;

	//int idx = 0;

	//unordered_map<string, int>::const_iterator gots = this->description->fieldsMap.find(field);
	//if (gots == this->description->fieldsMap.end()) {
	//	cout << field << " not found" << endl;
	//}
	//else {
	//	idx = gots->second;
	//}
	//bst::any anyone;
	//anyone = data[idx];

//	bst::any anyone = 1;
//	return anyone;
//}
