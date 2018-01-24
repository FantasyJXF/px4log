#pragma once
#include "PX4LogMessage.h"

class PX4LogMessageDescription {
public:
	uint8_t type;
	uint8_t length;
	string name;
	string format;
	vector<string> fields;
	unordered_map<string, int> fieldsMap;

	PX4LogMessageDescription() {
		type = 0x80;
		length = 89;
		name = (string)"FMT";
		format = (string)"BBnNZ";
		fields = {"Type", "Length", "Name", "Format", "Labels"};
	}

	PX4LogMessageDescription(int _type, int _length, string _name, string _format, vector<string> _fields):\
		type(_type), length(_length), name(_name), format(_format), fields(_fields){}

	PX4LogMessageDescription(streambuf *buffer);

	PX4LogMessage* parseMessage(/*streambuf*/char *buffer);
// End of Class
};

//PX4LogMessageDescription::PX4LogMessageDescription(char *buffer) {
//
//	uint8_t ofs = 0;
//	type = _GET_uint8_t(buffer, ofs) & 0xFF;
//	ofs += 1;
//	length = _GET_uint8_t(buffer, ofs) & 0xFF;
//	ofs += 1;
//	name = _GET_string_4(buffer, ofs);
//	ofs += 4;
//	format = _GET_string_16(buffer, ofs);
//	ofs += 16;
//	char *_fieldStr = _GET_char_ptr(buffer, 64, ofs);
//	split(_fieldStr, fields, ",");
//	if ("FMT" != name) {
//		for (uint8_t j = 0; j < fields.size() - 1; j++) {
//			fieldsMap.insert({ fields[j],j });
//		}
//	}
//	else {
//		cout << "New message " << name << endl;
//	}
//}

PX4LogMessageDescription::PX4LogMessageDescription(streambuf *buffer) {
	type = buffer->sbumpc() & 0xFF;
	length = buffer->sbumpc() & 0xFF;
	char *_name = new char[4];
	buffer->sgetn(_name,4);
	name = _name;
	char *_format = new char[16];
	buffer->sgetn(_format, 16);
	format = _format;
	char *_fieldsStr = new char[64];
	buffer->sgetn(_fieldsStr, 64);
	string fieldsStr = _fieldsStr;
	split(fieldsStr, fields, ",");
	if ("FMT" != name) {
		for (uint8_t j = 0; j < fields.size(); j++) {
			fieldsMap.insert({ fields[j],j });
		}
	}
	else {
		cout << "New message " << name << endl;
	}
}

//PX4LogMessage PX4LogMessageDescription::parseMessage(streambuf *buffer) {
//
//	unsigned int size_format = format.size();
//	vector<boost::any> data(size_format);
//	char _format[16];
//	strcpy_s(_format, format.c_str());
//	for (char f : _format) {
//		char *v = new char[64];
//		boost::any value;
//		if (f == 'f') { /* float */
//			buffer->sgetn(v,4);
//			value = boost::any_cast<float>(v);
//		}
//		else if (f == 'q') { /* int64_t */
//			buffer->sgetn(v, 8);
//			value = boost::any_cast<int64_t>(v);
//		} 
//		else if (f == 'Q') { /* uint64_t */
//			buffer->sgetn(v, 8);
//			value = boost::any_cast<uint64_t>(v);
//		}
//		else if (f == 'i') { /* int32_t */
//			buffer->sgetn(v, 4);
//			value = boost::any_cast<int32_t>(v);
//		}
//		else if (f == 'I') { /* uint32_t */
//			buffer->sgetn(v, 4);
//			value = boost::any_cast<uint32_t>(v) & 0xFFFFFFFF;
//		}
//		else if (f == 'b') { /* int8_t */
//			buffer->sgetn(v, 1);
//			value = boost::any_cast<int8_t>(v);
//		}
//		else if (f == 'B' || f == 'M') { /* uint8_t */
//			buffer->sgetn(v, 1);
//			value = boost::any_cast<uint8_t>(v) & 0xFF;
//		}
//		else if (f == 'L') { /* L -> int32 * 1e-7(lat/lon) */
//			buffer->sgetn(v, 4);
//			value = boost::any_cast<int32_t>(v) * 1e-7;
//		}
//		else if (f == 'h') { /* int16_t */
//			buffer->sgetn(v, 2);
//			value = boost::any_cast<int16_t>(v);
//		}
//		else if (f == 'H') {  /* uint16_t */
//			buffer->sgetn(v, 2);
//			value = boost::any_cast<uint16_t>(v) & 0xFFFF;
//		}
//		else if (f == 'n') { /* char[4] */
//			buffer->sgetn(v, 4);
//			value = boost::any_cast<string>(v);
//		}
//		else if (f == 'N') { /* char[16] */
//			buffer->sgetn(v, 16);
//			value = boost::any_cast<string>(v);
//		}
//		else if (f == 'Z') { /* char[64] */
//			buffer->sgetn(v, 64);
//			value = boost::any_cast<string>(v);
//		}
//		else if (f == 'c') { /* int16_t * 100 */
//			buffer->sgetn(v, 2);
//			value = boost::any_cast<int16_t>(v) * 1e-2;
//		}
//		else if (f == 'C') { /* uint16_t * 100 */
//			buffer->sgetn(v, 2);
//			value = (boost::any_cast<int16_t>(v) & 0xFFFF) * 1e-2;
//		}
//		else if (f == 'e') { /* int32_t * 100 */
//			buffer->sgetn(v, 4);
//			value = boost::any_cast<int32_t>(v) * 1e-2;
//		}
//		else if (f == 'E') { /* uint32_t * 100 */
//			buffer->sgetn(v, 4);
//			value = (boost::any_cast<int32_t>(v) & 0xFFFFFFFFl) * 1e-2;
//		}
//		else {
//			cerr << "Invalid format char in message " << name << endl;
//		}
//		data.push_back(value);
//	}
//	PX4LogMessage _Log_Message(this, data);
//	return _Log_Message;
//}

PX4LogMessage* PX4LogMessageDescription::parseMessage(char* buffer) {
	vector<boost::any> _data;
	uint8_t ofs = 0;
	boost::any value;
	char _format[40];
	size_t len = strlen(format.c_str()) + 1;
	strcpy_s(_format, len, format.c_str());
	for (char f : _format) {
		if (f == 'f') {
			value = _GET_float(buffer, ofs);
			ofs += 4;
		//	cout << boost::any_cast<float>(value) << endl;
		}
		else if (f == 'q') {
			value = _GET_int64_t(buffer, ofs);
			ofs += 8;
		}
		else if (f == 'Q') {
			value = _GET_uint64_t(buffer, ofs);
			ofs += 8;
		}
		else if (f == 'i')
		{
			value = _GET_int32_t(buffer, ofs);
			ofs += 4;
		}
		else if (f == 'I') {
			value = _GET_uint32_t(buffer, ofs) & 0xFFFFFFFF;
			ofs += 4;
		}
		else if (f == 'b') {
			value = _GET_int8_t(buffer, ofs);
			ofs += 1;
		}
		else if (f == 'B' || f == 'M') {
			value = _GET_uint8_t(buffer, ofs) & 0xFF;
			ofs += 1;
		}
		else if (f == 'L') { /* L -> int32 * 1e-7(lat/lon) */
			value = _GET_int32_t(buffer, ofs) * 1e-7;
			ofs += 4;
		}
		else if (f == 'h') {
			value = _GET_int16_t(buffer, ofs);
			ofs += 2;
		}
		else if (f == 'H') {
			value = _GET_uint16_t(buffer, ofs);
			ofs += 2;
		}
		else if (f == 'n') {
			value = _GET_string_4(buffer, ofs);
			ofs += 4;
		}
		else if (f == 'N') {
			value = _GET_string_16(buffer,ofs);
			ofs += 16;
		}
		else if (f == 'Z') {
			value = _GET_string_64(buffer, ofs);
			ofs += 64;
		}
		else if (f == 'c') {
			value = _GET_int16_t(buffer, ofs) * 1e-2;
			ofs += 2;
		}
		else if (f == 'C') {
			value = (_GET_int16_t(buffer, ofs) & 0xFFFF) * 1e-2;
			ofs += 2;
		}
		else if (f == 'e') {
			value = (_GET_int32_t(buffer, ofs) & 0xFFFFFFF) * 1e-2;
			ofs += 4;
		}
		else if (f == 'E') {
			value = _GET_float(buffer, ofs);
			ofs += 4;
		}
		else if(f == '\0'){
			//cout << "null LOL" << endl;
			break;
		}
		else {
			//cerr << "Invalid format char in message " << name << endl;
			break;
		}
		_data.push_back(value);
	}
	//cout << data.size() << endl;
	// ÏÔÊ¾Êý¾Ý
	//show_vector(_data);
	PX4LogMessage *_Log_Message = new PX4LogMessage(this, _data);
	return _Log_Message;
}

//PX4LogMessageDescription *FORMAT = new PX4LogMessageDescription(0x80, 89, (string)"FMT", (string)"BBnNZ", \
//				{"Type", "Length", "Name", "Format", "Labels"});

static struct px4log {
	uint8_t type = 0x80;
	uint8_t length = 89;
	string name = "FMT";
	string format = "BBnNZ";
	vector<string> fields = { "Type", "Length", "Name", "Format", "Labels" };
}FORMAT;
