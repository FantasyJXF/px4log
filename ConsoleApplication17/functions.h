#pragma once
/**
* 一些公用函数 以及 暂时不明确如何分类的定义
*/

#include<iostream>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<vector>
#include<iomanip>
#include<fstream>  
#include<boost/any.hpp>

using namespace std;

class EOFException : public std::exception
{
	virtual const char* what() const throw()
	{
		return "End of File";
	}
} eof_exception;

#define LOG_PACKET_HEADER_LEN	   3
#define LOG_PACKET_HEADER	       uint8_t head1, head2, msg_type;
#define LOG_PACKET_HEADER_INIT(id) .head1 = HEAD_BYTE1, .head2 = HEAD_BYTE2, .msg_type = id

// once the logging code is all converted we will remove these from
// this header
#define HEAD_BYTE1  0xA3    // Decimal 163
#define HEAD_BYTE2  0x95    // Decimal 149

class myexception : public std::exception
{
	virtual const char* what() const throw()
	{
		return "My exception happened";
	}
} myex;

/*
 * 字符串分隔
 */
void split(const string& src, vector<string>& dst, const string& sprt)
{
	string::size_type pos1, pos2;
	pos2 = src.find(sprt);
	pos1 = 0;
	while (string::npos != pos2)
	{
		dst.push_back(src.substr(pos1, pos2 - pos1));
		pos1 = pos2 + sprt.size();
		pos2 = src.find(sprt, pos1);
	}

	if (pos1 != src.length()) {
		dst.push_back(src.substr(pos1));
	}	
}

/*
 * 获取特定类型的数据
 */
static inline void byte_copy_2(char *dst, const char *src)
{
	dst[0] = src[0];
	dst[1] = src[1];
}
static inline void byte_copy_4(char *dst, const char *src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}
static inline void byte_copy_8(char *dst, const char *src)
{
	memcpy(dst, src, 8);
}
static inline void byte_copy_16(char *dst, const char *src)
{
	memcpy(dst, src, 16);
}
static inline void byte_copy_32(char *dst, const char *src)
{
	memcpy(dst, src, 32);
}
static inline void byte_copy_64(char *dst, const char *src)
{
	memcpy(dst, src, 64);
}

#define _GET_char(msg, wire_offset)     (const char)msg[wire_offset]
#define _GET_int8_t(msg, wire_offset)   (const int8_t)msg[wire_offset]
#define _GET_uint8_t(msg, wire_offset)  (const uint8_t)msg[wire_offset]


#define _GET_RETURN_TYPE(TYPE, SIZE) \
static TYPE _GET_## TYPE(const char *msg, uint8_t ofs) \
{ TYPE r; byte_copy_## SIZE((char*)&r, &msg[ofs]); return r; }

_GET_RETURN_TYPE(uint16_t, 2)
_GET_RETURN_TYPE(int16_t, 2)
_GET_RETURN_TYPE(uint32_t, 4)
_GET_RETURN_TYPE(int32_t, 4)
_GET_RETURN_TYPE(uint64_t, 8)
_GET_RETURN_TYPE(int64_t, 8)
_GET_RETURN_TYPE(float, 4)
_GET_RETURN_TYPE(double, 8)

string _GET_string_4(const char *msg, uint8_t ofs) {
	char chs[4] = "";
	memcpy(chs, &msg[ofs], 4);
	string str = (string)chs;
	return str;
}

string _GET_string_16(const char *msg, uint8_t ofs) {
	char chs[16] = "";
	memcpy(chs, &msg[ofs], 16);
	string str = (string)chs;
	return str;
}

string _GET_string_64(const char *msg, uint8_t ofs) {
	char chs[64] = "";
	memcpy(chs, &msg[ofs], 64);
	string str = (string)chs;
	return str;
}

char* _GET_char_ptr(const char* msg, uint8_t num, uint8_t ofs) {
	static char dst[20];
	memcpy(dst, &msg[ofs], num);
	return dst;
}

//根据类型进行显示  
// char(1)   int8_t(1)  int16_t(2)  int32_t(4)  int64_t(8)
// float(4)  double(8)  
typedef list<boost::any> list_any;
void show_list(list_any& la)
{
	list_any::iterator iter;
	boost::any anyone;

	for (iter = la.begin(); iter != la.end(); iter++)
	{
		anyone = *iter;

		if (anyone.type() == typeid(char))
			cout << boost::any_cast<char>(*iter) << endl;

		else if (anyone.type() == typeid(char*))
			cout << boost::any_cast<char*>(*iter) << endl;

		else if (anyone.type() == typeid(string))
			cout << boost::any_cast<string>(*iter) << endl;

		else if (anyone.type() == typeid(int8_t))
			cout << boost::any_cast<int8_t>(*iter) << endl;

		else if (anyone.type() == typeid(uint8_t))
			cout << boost::any_cast<uint8_t>(*iter) << endl;

		else if (anyone.type() == typeid(int16_t))
			cout << boost::any_cast<int16_t>(*iter) << endl;

		else if (anyone.type() == typeid(uint16_t))
			cout << boost::any_cast<uint16_t>(*iter) << endl;

		else if (anyone.type() == typeid(int32_t))
			cout << boost::any_cast<int32_t>(*iter) << endl;

		else if (anyone.type() == typeid(uint32_t))
			cout << boost::any_cast<uint32_t>(*iter) << endl;

		else if (anyone.type() == typeid(int64_t))
			cout << boost::any_cast<int64_t>(*iter) << endl;

		else if (anyone.type() == typeid(uint64_t))
			cout << boost::any_cast<uint64_t>(*iter) << endl;

		else if (anyone.type() == typeid(float))
			cout << boost::any_cast<float>(*iter) << endl;

		else if (anyone.type() == typeid(double))
			cout << boost::any_cast<double>(*iter) << endl;
		else
			cerr << "The data type is out of range" << endl;
	}
}

typedef vector<boost::any> vector_any;
void show_vector(vector_any& la)
{
	vector_any::iterator iter;
	boost::any anyone;

	for (iter = la.begin(); iter != la.end(); iter++)
	{
		anyone = *iter;

		if (anyone.type() == typeid(char))
			cout << boost::any_cast<char>(*iter) << endl;

		else if (anyone.type() == typeid(char*))
			cout << boost::any_cast<char*>(*iter) << endl;

		else if (anyone.type() == typeid(string))
			cout << boost::any_cast<string>(*iter) << endl;

		else if (anyone.type() == typeid(int8_t))
			cout << boost::any_cast<int8_t>(*iter) << endl;

		else if (anyone.type() == typeid(uint8_t))
			cout << boost::any_cast<uint8_t>(*iter) << endl;

		else if (anyone.type() == typeid(int16_t))
			cout << boost::any_cast<int16_t>(*iter) << endl;

		else if (anyone.type() == typeid(uint16_t))
			cout << boost::any_cast<uint16_t>(*iter) << endl;

		else if (anyone.type() == typeid(int32_t))
			cout << boost::any_cast<int32_t>(*iter) << endl;

		else if (anyone.type() == typeid(uint32_t))
			cout << boost::any_cast<uint32_t>(*iter) << endl;

		else if (anyone.type() == typeid(int64_t))
			cout << boost::any_cast<int64_t>(*iter) << endl;

		else if (anyone.type() == typeid(uint64_t))
			cout << boost::any_cast<uint64_t>(*iter) << endl;

		else if (anyone.type() == typeid(float))
			cout << boost::any_cast<float>(*iter) << endl;

		else if (anyone.type() == typeid(double))
			cout << boost::any_cast<double>(*iter) << endl;

		else
			cerr << "The data type is out of range  666" << endl;
	}
}