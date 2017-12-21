#pragma once
#include "PX4LogMessage.h"


class PX4LogMessageDescription {

	//int type;
	//int length;
	//string name;
	//string format;
	//vector<string> fields;
	//unordered_map<string, int> fieldsMap;

public:
	PX4LogMessageDescription() {
		type = 0x80;
		length = 89;
		name = (string)"FMT";
		format = (string)"BBnNZ";
		fields = {"Type", "Length", "Name", "Format", "Labels"};
	}

	PX4LogMessageDescription(int _type, int _length, string _name, string _format, vector<string> _fields):\
		type(_type), length(_length), name(_name), format(_format), fields(_fields){}

	PX4LogMessageDescription(streambuf *buffer) {
		type = buffer->sgetc() & 0xFF;
		length = buffer->sgetc() & 0xFF;
		char *_name = new char[4];
		buffer->sgetn(_name,4);
		name = _name;
		char *_format = new char[16];
		buffer->sgetn(_format, 16);
		format = _format;
		char *_fieldStr;
		buffer->sgetn(_fieldStr, 64);
//		string fieldsStr = _fieldsStr;
		//vector<string> _fields;
		//split(fieldStr, _fields, ",");
		//int i = 0;
		//for (vector<string>::iterator iter = _fields.begin(); iter != _fields.end(); iter++) {
		//	fields[i++] = *iter;
		//}
		split(_fieldStr, fields, ",");
		if ("FMT" != name) {
			for (uint8_t j = 0; j < fields.size() - 1; j++) {
				fieldsMap.insert({ fields[j],j });
			}
		}
		else {
			cout << "New message " << name << endl;
		}
	}
	
	PX4LogMessage parseMessage(/*streambuf*/char* buffer) {

		unsigned int size_format = format.size();
		vector<boost::any> data(size_format);
		char _format[16];
		strcpy_s(_format, format.c_str());
		for (char f : _format) {
			//char *v = new char[8];
			static uint8_t ofs = 0;
			boost::any value;
			if (f == 'f') {
				//buffer.sgetn(boost::any_cast<float>(v),4);
				value = _GET_float(buffer, ofs);
				ofs += 4;
			}
			else if (f == 'q') {
				//buffer.sgetn(v, 8);
				value = _GET_int64_t(buffer, ofs);
				ofs += 8;
			}
			else if (f == 'Q') {
				//buffer.sgetn(v, 8);
				value = _GET_uint64_t(buffer, ofs);
				ofs += 8;
			}
			else if(f == 'i')
			{
				//buffer.sgetn(v, 4);
				value = _GET_int32_t(buffer, ofs);
				ofs += 4;
			}
			else if (f == 'I') {
				//buffer.sgetn(v, 4);
				value = _GET_uint32_t(buffer, ofs) & 0xFFFFFFFF;
				ofs += 4;
			}
			else if (f == 'b') {
				//buffer.sgetn(v, 1);
				value = _GET_int8_t(buffer, ofs);
				ofs += 1;
			}
			else if (f == 'B' || f == 'M') {
				//buffer.sgetn(v, 1);
				value = _GET_uint8_t(buffer, ofs) & 0xFF;
				ofs += 1;
			}
			else if (f == 'L') { /* L -> int32 * 1e-7(lat/lon) */
				//buffer.sgetn(v, 4);
				value = _GET_int32_t(buffer, ofs) * 1e-7;
				ofs += 4;
			}
			else if (f == 'h') {
				//buffer.sgetn(v, 2);
				value = _GET_int16_t(buffer, ofs);
				ofs += 2;
			}
			else if (f == 'H') {
				//buffer.sgetn(v, 2);
				value = _GET_uint16_t(buffer, ofs);
				ofs += 2;
			}
			else if (f == 'n') {
				//buffer.sgetn(v, 4);
				memcpy(&value, &buffer[ofs], 4);
				ofs += 4;
			}
			else if (f == 'N') {
				//buffer.sgetn(v, 2);
				memcpy(&value, &buffer[ofs], 16);
				ofs += 16;
			}
			else if (f == 'Z') {
				//buffer.sgetn(v, 8);
				memcpy(&value, &buffer[ofs], 64);
				ofs += 64;
			}
			else if (f == 'c') {
				//buffer.sgetn(v, 2);
				value = _GET_int16_t(buffer, ofs) * 1e-2;
				ofs += 2;
			}
			else if (f == 'C') {
				//buffer.sgetn(v, 2);
				value = (_GET_int16_t(buffer, ofs) & 0xFFFF) * 1e-2;
				ofs += 2;
			}
			else if (f == 'e') {
				//buffer.sgetn(v, 4);
				value = (_GET_int32_t(buffer, ofs) & 0xFFFFFFF) * 1e-2;
				ofs += 4;
			}
			else if (f == 'E') {
				//buffer.sgetn(v, 4);
				value = _GET_float(buffer, ofs);
				ofs += 4;
			}
			else {
				cerr << "Invalid format char in message " << name << endl;
			}
			data.push_back(value);
		}
		PX4LogMessage _Log_Message(this, data);
		return _Log_Message;
	}

	int get_type() { return type; }
	int get_length() { return length; }

private:
	int type;
	int length;
	string name;
	string format;
	vector<string> fields;
	unordered_map<string, int> fieldsMap;
// End of Class
};

//PX4LogMessageDescription *FORMAT = new PX4LogMessageDescription(0x80, 89, (string)"FMT", (string)"BBnNZ", \
//{"Type", "Length", "Name", "Format", "Labels"});