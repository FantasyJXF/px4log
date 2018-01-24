#include "functions.h"
#include "PX4LogMessageDescription.h"

using namespace std;

class PX4LogReader {
	uint8_t HEADER_LEN = 3;
	uint8_t HEADER_HEAD1 = 0xA3;
	uint8_t HEADER_HEAD2 = 0x95;
	uint64_t dataStart = 0;
	unordered_map<uint8_t, PX4LogMessageDescription> messageDescriptions;
	unordered_map<string, string> fieldsList;
	uint64_t time = 0;
	PX4LogMessage *lastMsg;
	uint64_t sizeUpdates = 0;
	uint64_t sizeMicroseconds = -1;
	uint64_t startMicroseconds = -1;
	long long utcTimeReference = -1;
	unordered_map<string, string> version;
	unordered_map<string, float> parameters;
	string tsName;
	bool tsMicros;
	uint64_t position = 0;
	static unordered_set<string> hideMsgs;
	static unordered_map<char, string> formatNames;
	
public:

	PX4LogReader(streambuf *buf) {
		//streambuf *buf = read_all(filename);
		readFormats(buf);
		updateStatistics(buf);
	}

	/* 读取文件 */
	streambuf* PX4LogReader::read_all(string filename);

	/* 读取消息头 */
	uint8_t readHeader(streambuf *buf);

	/* 读取消息格式 */
	void readFormats(streambuf *buf);

	/* 更新统计数据 */
	void updateStatistics(streambuf *buf);

	/* 检索时间 */
	bool seek(streambuf *buf, uint64_t seekTime);

	/* 读取日志消息 */
	PX4LogMessage* readMessage(streambuf *buf);

	/* 应用消息 */
	void applyMsg(unordered_map<string, boost::any> update, PX4LogMessage *msg);

	/* 读取更新 */
	uint64_t readUpdate(unordered_map<string, boost::any> update, streambuf *buf);

	/* 读取目标字段 */
	void getField(string field, streambuf *buf, vector<boost::any> vaules);
};

unordered_set<string> PX4LogReader::hideMsgs = unordered_set<string>({ "PARM", "FMT", "TIME", "VER" });
unordered_map<char, string> PX4LogReader::formatNames = unordered_map<char, string>({ \
																			{'b',"int8"}, \
																			{'B', "uint8"}, \
																			{'h', "int16"}, \
																			{'H', "uint16"}, \
																			{'L', "int32 * 1e-7 (lat/lon)"}, \
																			{'i', "int32"}, \
																			{'I', "uint32"}, \
																			{'q', "int64"}, \
																			{'Q', "uint64"}, \
																			{'f', "float"}, \
																			{'c', "int16 * 1e-2"}, \
																			{'C', "uint16 * 1e-2"}, \
																			{'e', "int32 * 1e-2"}, \
																			{'E', "uint32 * 1e-2"}, \
																			{'n', "char[4]"}, \
																			{'N', "char[16]"}, \
																			{'Z', "char[64]"}, \
																			{'M', "uint8 (mode)"}});

streambuf* PX4LogReader::read_all(string filename) {
	// 要读入整个文件，必须采用二进制打开   
	ifstream ifile(filename, ios::in | ios::binary);

	//  获取filestr对应buffer对象的指针   
	streambuf *pbuf = ifile.rdbuf();

	// 将指针位置移动到第一个字节
	//pbuf->pubseekoff(0, ifile.beg);
	pbuf->pubseekpos(0);

	ifile.close();

	return pbuf;
}

uint8_t PX4LogReader::readHeader(streambuf *buf) {
	uint8_t byte1 = buf->sbumpc() & 0xFF;
	if (0xFF == byte1) {
		cout << "hehehehe" << endl;
	}
	uint8_t byte2 = buf->sbumpc() & 0xFF;
	uint8_t _msg_type = buf->sbumpc() & 0xFF;

	if (byte1 != HEAD_BYTE1 || byte2 != HEAD_BYTE2) {
		cerr << "wrong HEADER";
		return -1;
	}
	else
	{
		return _msg_type;
	}
}

void PX4LogReader::readFormats(streambuf *buf) {
	unordered_map<string, string> fieldsList;
	while (true) {
		static uint64_t num = 0;
		uint8_t msgType = readHeader(buf);
		if (msgType == FORMAT.type) {
			num += FORMAT.length;
			// 写入了每种消息的描述
			PX4LogMessageDescription msgDescr(buf);
			messageDescriptions.insert({msgDescr.type, msgDescr});
			// 输出记录的日志消息类型
			//cout << msgDescr.name << endl;
			if (!(hideMsgs.count(msgDescr.name) > 0)) {
				// 一共有fields_num个关注的日志消息
				int fields_num = msgDescr.fields.size();
				// 每个field的数据类型
				const char *format_type = msgDescr.format.c_str();
				vector<string>::iterator iter = msgDescr.fields.begin();
				for (int i = 0; i < fields_num; i++) {
					string field = *iter;
					string format;
					// 将format中的每一个字母缩写转换成具体的格式字符串  例如 b -> uint_8
					unordered_map<char, string>::const_iterator got = formatNames.find(format_type[i]);
					if (got == formatNames.end()) {
						cout << format_type[i] << "not found" << endl;;
					}
					else {
						// 得到数据类型
						format = got->second;
						//cout << format << endl;
					}
					iter++;
					// 将字段名与其对应的数据类型对应起来，  例如： ATT.Pitch  对应 float
					if (i != 0 || strcmp("TimeMS", field.c_str()) || !strcmp("TimeUS", field.c_str())) {
						fieldsList.insert({ msgDescr.name + "." + field, format });
					}
				}
			}
		}
		else {
			// 读取数据消息
			// 所有的格式都已经读好了
			// 将指针回到数据开始传输的位置
			dataStart = buf->pubseekpos(num); 
			cout << "the position now is " << dataStart << endl;
			return;
		}
	}
}

PX4LogMessage* PX4LogReader::readMessage(streambuf *buf) {
	while (true) {
		PX4LogMessageDescription messageDescription;
		int msgType = readHeader(buf); // 读取消息头  获得消息ID
		//long pos = buf->pubseekpos();// 当前位置

		// 通过消息ID获取消息的描述：字段、数据类型、、、
		unordered_map<uint8_t, PX4LogMessageDescription>::const_iterator got = messageDescriptions.find(msgType);
		if (got == messageDescriptions.end()) {
			cout << "msgType "<< msgType << " not found" << endl;
		}
		else {
			messageDescription = got->second;	
		}

		if (&messageDescription == NULL) {
			cerr << "Unknown message type: " << msgType << endl;
			continue;
		}
		
		// 将一种消息的数据所占长度字符串写入buffer
		uint8_t len = messageDescription.length - HEADER_LEN;
		char *buffer = new char[len];
		buf->sgetn(buffer, len);
		PX4LogMessage *PX4LogMsg = messageDescription.parseMessage(buffer); // 返回消息与数据
		return PX4LogMsg;
	}
}

void PX4LogReader::updateStatistics(streambuf *buf) {
	seek(buf,0);
	uint64_t packetsNum = 0;
	uint64_t timeStart = -1;
	uint64_t timeEnd = -1;
	bool parseVersion = true;
	string versionStr;
	while (true) {
		//PX4LogMessage *msg;
		//if (true) {
			PX4LogMessageDescription messageDescription;
			int msgType = readHeader(buf); // 读取消息头  获得消息ID
			//long pos = buf->pubseekpos();// 当前位置
			// 通过消息ID获取消息的描述：字段、数据类型、
			unordered_map<uint8_t, PX4LogMessageDescription>::const_iterator got = messageDescriptions.find(msgType);
			if (got == messageDescriptions.end()) {
				cout << "msgType " << msgType << " not found" << endl;
			}
			else {
				messageDescription = got->second;
			}

			if (&messageDescription == NULL) {
				cerr << "Unknown message type: " << msgType << endl;
				continue;
			}

			// 将一种消息的数据所占长度字符串写入buffer
			uint8_t len = messageDescription.length - HEADER_LEN;
			char *buffer = new char[len];
			buf->sgetn(buffer, len);
			PX4LogMessage *msg = messageDescription.parseMessage(buffer); // 返回消息与数据
			//PX4LogMessage* msg = readMessage(buf);
		//}
		//else {
		//	cout << "End of file" << endl;
		//	break;
		//}

		string Name = (msg->description->name).substr(0, 4);
		//show_vector(msg->data);

		// TIME
		// 时间范围
		if ("TIME" == Name) {
			uint64_t t = boost::any_cast<uint64_t>(msg->data[0]);
			time = t;
			if (timeStart < 0) {
				timeStart = t;
			}
			timeEnd = t;
		}

		packetsNum++;

		// Version
		// 版本
		if ("VER" == Name) {
			string fw, hw;
			unordered_map<string, int>::const_iterator got1 = msg->description->fieldsMap.find("FwGit");
			if (got1 == msg->description->fieldsMap.end()) {
				cout << "FwGit" << " not found" << endl;
			}
			else {
				int idx1 = got1->second;
				fw = boost::any_cast<string>(msg->data[idx1]);
				version.insert({ "FW", fw });
			}

			unordered_map<string, int>::const_iterator got2 = msg->description->fieldsMap.find("Arch");
			if (got2 == msg->description->fieldsMap.end()) {
				cout << "Arch" << " not found" << endl;
			}
			else {
				int idx2 = got2->second;
				hw = boost::any_cast<string>(msg->data[idx2]);
				version.insert({ "HW", hw });
			}
			//cout<< "Data of VER" << endl;
			//show_vector(msg->data);
		}

		// Parameters
		// 参数		
		if ("PARM" == Name) {
			string _name; // 写入时是char[16]
			float _value;
			boost::any anyone;
			
			unordered_map<string, int>::const_iterator got3 = msg->description->fieldsMap.find("Name");
			if (got3 == msg->description->fieldsMap.end()) {
				cout << "Name" << " not found" << endl;
			}
			else {
				int idx3 = got3->second;
				_name = boost::any_cast<string>(msg->data[idx3]);
			}

			unordered_map<string, int>::const_iterator got4 = msg->description->fieldsMap.find("Value");
			if (got4 == msg->description->fieldsMap.end()) {
				cout << "Value" << " not found" << endl;
			}
			else {
				int idx4 = got4->second;
				_value = boost::any_cast<float>(msg->data[idx4]);
			}
			parameters.insert({ _name,_value });
			//cout << "Data of Parameters" << endl;
			//show_vector(msg->data);
		}

		// GPS
		// GPS时间信息
		if ("GPS" == Name) {
			if (utcTimeReference < 0) {
				int fix = 0;
				uint64_t gpsT;
				unordered_map<string, int>::const_iterator got5 = msg->description->fieldsMap.find("Fix");
				if (got5 == msg->description->fieldsMap.end()) {
					cout << "Fix" << " not found" << endl;
				}
				else {
					int idx5 = got5->second;
					fix = boost::any_cast<int>(msg->data[idx5]);
				}

				unordered_map<string, int>::const_iterator got6 = msg->description->fieldsMap.find("GPSTime");
				if (got6 == msg->description->fieldsMap.end()) {
					cout << "GPSTime" << " not found" << endl;
				}
				else {
					int idx6 = got6->second;
					gpsT = boost::any_cast<uint64_t>(msg->data[idx6]);
				}

				if (fix >= 3 && gpsT > 0) {
					utcTimeReference = gpsT - timeEnd; // 当GPS状态良好时获得UTC的时间参考
				}
			}
		}
	}
	startMicroseconds = timeStart; // 解锁时的时间
	sizeUpdates = packetsNum;
	sizeMicroseconds = timeEnd - timeStart;
	seek(buf,0);  // 开始记日志
}

bool PX4LogReader::seek(streambuf *buf,uint64_t seekTime) {
	buf->pubseekpos(dataStart);
	lastMsg = NULL;
	if (seekTime == 0) {      // Seek to start of log 开始记日志
		time = 0;
		return true;
	}

	while (true) {
		// 获得数据流的当前指针位置
		//uint64_t pos = position();
		uint8_t msgType = readHeader(buf);
		PX4LogMessageDescription messageDescription;
		unordered_map<uint8_t, PX4LogMessageDescription>::const_iterator got = messageDescriptions.find(msgType);
		if (got == messageDescriptions.end()) {
			cout << "not found";
		}
		else {
			messageDescription = got->second;
		}
		
		if (&messageDescription == NULL) {
			cerr << "Unknown message type: " << msgType << endl;
			continue;
		}

		uint8_t bodyLen = messageDescription.length - HEADER_LEN;
		char *buffer = new char[bodyLen];
		buf->sputn(buffer, bodyLen);
		if ("TIME" == messageDescription.name) {
			PX4LogMessage *msg = messageDescription.parseMessage(buffer);
			uint64_t t = 0;
			t = boost::any_cast<uint64_t>(msg->data);
			if (t > seekTime) {
				// Time found
				time = t;
				//确定当前流的位置
				// position(pos);
				return true;
			}
		}
		else {
			// Skip the message
			// 跳过此消息
			//buffer.position(buffer.position() + bodyLen);
			cout << "skip the message" << endl;
		}
	}
}

void PX4LogReader::applyMsg(unordered_map<string, boost::any> update, PX4LogMessage *msg) {
	vector<string> fields = msg->description->fields;
	for (size_t i = 0; i < fields.size(); i++) {
		string field = fields[i];
		update.insert({ msg->description->name + "." + field, msg->data[i] });
	}
}

uint64_t PX4LogReader::readUpdate(unordered_map<string, boost::any> update, streambuf *buf) {
	uint64_t t = time;
	if (lastMsg != NULL) {
		applyMsg(update, lastMsg);
		lastMsg = NULL;
	}

	while (true) {
		PX4LogMessageDescription messageDescription;
		int msgType = readHeader(buf); // 读取消息头  获得消息ID
									   //long pos = buf->pubseekpos();// 当前位置
									   // 通过消息ID获取消息的描述：字段、数据类型、
		unordered_map<uint8_t, PX4LogMessageDescription>::const_iterator got = messageDescriptions.find(msgType);
		if (got == messageDescriptions.end()) {
			cout << "msgType " << msgType << " not found" << endl;
		}
		else {
			messageDescription = got->second;
		}

		if (&messageDescription == NULL) {
			cerr << "Unknown message type: " << msgType << endl;
			continue;
		}

		// 将一种消息的数据所占长度字符串写入buffer
		uint8_t len = messageDescription.length - HEADER_LEN;
		char *buffer = new char[len];
		buf->sgetn(buffer, len);
		PX4LogMessage *msg = messageDescription.parseMessage(buffer); // 返回消息与数据
		//PX4LogMessage msg = readMessage();
		if (msg == NULL) {
			continue;
		}

		if ("TIME" == msg->description->name) {
			time = boost::any_cast<uint64_t>(msg->data[0]);
			if (t == 0) {
				// The first TIME message
				t = time;
				continue;
			}
			break;
		}
		applyMsg(update, msg);
	}
	return t;
}

void PX4LogReader::getField(string field, streambuf *buf, vector<boost::any> vaules) {
	uint64_t pos = dataStart;
	vector<string> _name;
	split(field, _name, ".");
	for (uint8_t j = 0; j < _name.size(); j++) {
		cout << _name[j] << endl;
	}

	while (true) {
		unordered_map<string, vector<boost::any>> update;
		PX4LogMessageDescription messageDescription;
		int msgType = readHeader(buf); // 读取消息头  获得消息ID
									   //long pos = buf->pubseekpos();// 当前位置
		// 通过消息ID获取消息的描述：字段、数据类型、
		unordered_map<uint8_t, PX4LogMessageDescription>::const_iterator got = messageDescriptions.find(msgType);

		if (got == messageDescriptions.end()) {
			cout << "msgType " << msgType << " not found" << endl;
		}
		else {
			messageDescription = got->second;
		}

		if (_name[0] != messageDescription.name) {
			pos += messageDescription.length - HEADER_LEN;
			buf->pubseekpos(pos);
			continue;
		}
		else {
			// 将一种消息的数据所占长度字符串写入buffer
			uint8_t len = messageDescription.length - HEADER_LEN;
			char *buffer = new char[len];
			buf->sgetn(buffer, len);
			PX4LogMessage *msg = messageDescription.parseMessage(buffer); // 返回消息与数据

			vector<string> _fields = msg->description->fields;
			for (size_t i = 0; i < _fields.size(); i++) {
				string field = _fields[i];
				if (field == _name[1]) {
					vaules.push_back(msg->data[i]);
					//update.insert({ msg->description->name + "." + field, vaules });
					update.insert({field, vaules});
				}
			}
		}
	}
}

int main() {

	string _filename = "15_49_43.px4log";

	ifstream ifile(_filename, ios::in | ios::binary);
	
	streambuf *pbuf = ifile.rdbuf();

	PX4LogReader reader(pbuf);
	//PX4LogReader reader(_filename);
	
	return 0;

}