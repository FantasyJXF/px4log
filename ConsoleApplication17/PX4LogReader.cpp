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

	/* ��ȡ�ļ� */
	streambuf* PX4LogReader::read_all(string filename);

	/* ��ȡ��Ϣͷ */
	uint8_t readHeader(streambuf *buf);

	/* ��ȡ��Ϣ��ʽ */
	void readFormats(streambuf *buf);

	/* ����ͳ������ */
	void updateStatistics(streambuf *buf);

	/* ����ʱ�� */
	bool seek(streambuf *buf, uint64_t seekTime);

	/* ��ȡ��־��Ϣ */
	PX4LogMessage* readMessage(streambuf *buf);

	/* Ӧ����Ϣ */
	void applyMsg(unordered_map<string, bst::any> update, PX4LogMessage *msg);

	/* ��ȡ���� */
	uint64_t readUpdate(unordered_map<string, bst::any> update, streambuf *buf);

	/* ��ȡĿ���ֶ� */
	void getField(string field, streambuf *buf, vector<bst::any> vaules);
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
	// Ҫ���������ļ���������ö����ƴ�   
	ifstream ifile(filename, ios::in | ios::binary);

	//  ��ȡfilestr��Ӧbuffer�����ָ��   
	streambuf *pbuf = ifile.rdbuf();

	// ��ָ��λ���ƶ�����һ���ֽ�
	//pbuf->pubseekoff(0, ifile.beg);
	pbuf->pubseekpos(0);

	ifile.close();

	return pbuf;
}

uint8_t PX4LogReader::readHeader(streambuf *buf) {
	uint8_t byte1 = buf->sbumpc() & 0xFF;
	if (0xFF == byte1) {
		cout << "Hits the End" << endl;
	}
	uint8_t byte2 = buf->sbumpc() & 0xFF;
	if (0xFF == byte1 && 0xFF == byte2) {
		//cout << "Here the file ends" << endl;
		throw eof_exception;
	}

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
			// д����ÿ����Ϣ������
			PX4LogMessageDescription msgDescr(buf);
			messageDescriptions.insert({msgDescr.type, msgDescr});
			// �����¼����־��Ϣ����
			//cout << msgDescr.name << endl;
			if (!(hideMsgs.count(msgDescr.name) > 0)) {
				// һ����fields_num����ע����־��Ϣ
				int fields_num = msgDescr.fields.size();
				// ÿ��field����������
				const char *format_type = msgDescr.format.c_str();
				vector<string>::iterator iter = msgDescr.fields.begin();
				for (int i = 0; i < fields_num; i++) {
					string field = *iter;
					string format;
					// ��format�е�ÿһ����ĸ��дת���ɾ���ĸ�ʽ�ַ���  ���� b -> uint_8
					unordered_map<char, string>::const_iterator got = formatNames.find(format_type[i]);
					if (got == formatNames.end()) {
						cout << format_type[i] << "not found" << endl;;
					}
					else {
						// �õ���������
						format = got->second;
						//cout << format << endl;
					}
					iter++;
					// ���ֶ��������Ӧ���������Ͷ�Ӧ������  ���磺 ATT.Pitch  ��Ӧ float
					if (i != 0 || strcmp("TimeMS", field.c_str()) || !strcmp("TimeUS", field.c_str())) {
						fieldsList.insert({ msgDescr.name + "." + field, format });
					}
				}
			}
		}
		else {
			// ��ȡ������Ϣ
			// ���еĸ�ʽ���Ѿ�������
			// ��ָ��ص����ݿ�ʼ�����λ��
			dataStart = buf->pubseekpos(num); 
			cout << "the position now is " << dataStart << endl;
			return;
		}
	}
}

PX4LogMessage* PX4LogReader::readMessage(streambuf *buf) {
	while (true) {
		PX4LogMessageDescription messageDescription;
		int msgType = readHeader(buf); // ��ȡ��Ϣͷ  �����ϢID
		//long pos = buf->pubseekpos();// ��ǰλ��

		// ͨ����ϢID��ȡ��Ϣ���������ֶΡ��������͡�����
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
		
		// ��һ����Ϣ��������ռ�����ַ���д��buffer
		uint8_t len = messageDescription.length - HEADER_LEN;
		char *buffer = new char[len];
		buf->sgetn(buffer, len);
		PX4LogMessage *PX4LogMsg = messageDescription.parseMessage(buffer); // ������Ϣ������
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
		PX4LogMessage *msg;
		try {
			PX4LogMessageDescription messageDescription;
			int msgType = readHeader(buf); // ��ȡ��Ϣͷ  �����ϢID
			//long pos = buf->pubseekpos();// ��ǰλ��
			// ͨ����ϢID��ȡ��Ϣ���������ֶΡ��������͡�
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

			// ��һ����Ϣ��������ռ�����ַ���д��buffer
			uint8_t len = messageDescription.length - HEADER_LEN;
			char *buffer = new char[len];
			buf->sgetn(buffer, len);
			//PX4LogMessage *
				msg = messageDescription.parseMessage(buffer); // ������Ϣ������
			//PX4LogMessage* msg = readMessage(buf);
		}
		catch (std::exception& e){
			cout << "End of file" << endl;
			break;
		}

		string Name = (msg->description->name).substr(0, 4);
		//show_vector(msg->data);

		// TIME
		// ʱ�䷶Χ
		if ("TIME" == Name) {
			uint64_t t = bst::any_cast<uint64_t>(msg->data[0]);
			time = t;
			if (timeStart < 0) {
				timeStart = t;
			}
			timeEnd = t;
		}

		packetsNum++;

		// Version
		// �汾
		if ("VER" == Name) {
			string fw, hw;
			unordered_map<string, int>::const_iterator got1 = msg->description->fieldsMap.find("FwGit");
			if (got1 == msg->description->fieldsMap.end()) {
				cout << "FwGit" << " not found" << endl;
			}
			else {
				int idx1 = got1->second;
				fw = bst::any_cast<string>(msg->data[idx1]);
				version.insert({ "FW", fw });
			}

			unordered_map<string, int>::const_iterator got2 = msg->description->fieldsMap.find("Arch");
			if (got2 == msg->description->fieldsMap.end()) {
				cout << "Arch" << " not found" << endl;
			}
			else {
				int idx2 = got2->second;
				hw = bst::any_cast<string>(msg->data[idx2]);
				version.insert({ "HW", hw });
			}
			//cout<< "Data of VER" << endl;
			//show_vector(msg->data);
		}

		// Parameters
		// ����		
		if ("PARM" == Name) {
			string _name; // д��ʱ��char[16]
			float _value;
			bst::any anyone;
			
			unordered_map<string, int>::const_iterator got3 = msg->description->fieldsMap.find("Name");
			if (got3 == msg->description->fieldsMap.end()) {
				cout << "Name" << " not found" << endl;
			}
			else {
				int idx3 = got3->second;
				_name = bst::any_cast<string>(msg->data[idx3]);
			}

			unordered_map<string, int>::const_iterator got4 = msg->description->fieldsMap.find("Value");
			if (got4 == msg->description->fieldsMap.end()) {
				cout << "Value" << " not found" << endl;
			}
			else {
				int idx4 = got4->second;
				_value = bst::any_cast<float>(msg->data[idx4]);
			}
			parameters.insert({ _name,_value });
			//cout << "Data of Parameters" << endl;
			//show_vector(msg->data);
		}

		// GPS
		// GPSʱ����Ϣ
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
					fix = bst::any_cast<int>(msg->data[idx5]);
				}

				unordered_map<string, int>::const_iterator got6 = msg->description->fieldsMap.find("GPSTime");
				if (got6 == msg->description->fieldsMap.end()) {
					cout << "GPSTime" << " not found" << endl;
				}
				else {
					int idx6 = got6->second;
					gpsT = bst::any_cast<uint64_t>(msg->data[idx6]);
				}

				if (fix >= 3 && gpsT > 0) {
					utcTimeReference = gpsT - timeEnd; // ��GPS״̬����ʱ���UTC��ʱ��ο�
				}
			}
		}
	}
	startMicroseconds = timeStart; // ����ʱ��ʱ��
	sizeUpdates = packetsNum;
	sizeMicroseconds = timeEnd - timeStart;
	seek(buf,0);  // ��ʼ����־
}

bool PX4LogReader::seek(streambuf *buf,uint64_t seekTime) {
	buf->pubseekpos(dataStart);
	lastMsg = NULL;
	if (seekTime == 0) {      // Seek to start of log ��ʼ����־
		time = 0;
		return true;
	}

	while (true) {
		// ����������ĵ�ǰָ��λ��
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
			t = bst::any_cast<uint64_t>(msg->data[0]);
			if (t > seekTime) {
				// Time found
				time = t;
				//ȷ����ǰ����λ��
				// position(pos);
				return true;
			}
		}
		else {
			// Skip the message
			// ��������Ϣ
			//buffer.position(buffer.position() + bodyLen);
			cout << "skip the message" << endl;
		}
	}
}

void PX4LogReader::applyMsg(unordered_map<string, bst::any> update, PX4LogMessage *msg) {
	vector<string> fields = msg->description->fields;
	for (size_t i = 0; i < fields.size(); i++) {
		string field = fields[i];
		update.insert({ msg->description->name + "." + field, msg->data[i] });
	}
}

uint64_t PX4LogReader::readUpdate(unordered_map<string, bst::any> update, streambuf *buf) {
	uint64_t t = time;
	if (lastMsg != NULL) {
		applyMsg(update, lastMsg);
		lastMsg = NULL;
	}

	while (true) {
		PX4LogMessageDescription messageDescription;
		int msgType = readHeader(buf); // ��ȡ��Ϣͷ  �����ϢID
									   //long pos = buf->pubseekpos();// ��ǰλ��
									   // ͨ����ϢID��ȡ��Ϣ���������ֶΡ��������͡�
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

		// ��һ����Ϣ��������ռ�����ַ���д��buffer
		uint8_t len = messageDescription.length - HEADER_LEN;
		char *buffer = new char[len];
		buf->sgetn(buffer, len);
		PX4LogMessage *msg = messageDescription.parseMessage(buffer); // ������Ϣ������
		//PX4LogMessage msg = readMessage();
		if (msg == NULL) {
			continue;
		}

		if ("TIME" == msg->description->name) {
			time = bst::any_cast<uint64_t>(msg->data[0]);
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

void PX4LogReader::getField(string field, streambuf *buf, vector<bst::any> vaules) {
	uint64_t pos = dataStart;
	vector<string> _name;
	split(field, _name, ".");
	for (uint8_t j = 0; j < _name.size(); j++) {
		cout << _name[j] << endl;
	}

	while (true) {
		unordered_map<string, vector<bst::any>> update;
		PX4LogMessageDescription messageDescription;
		int msgType = readHeader(buf); // ��ȡ��Ϣͷ  �����ϢID
									   //long pos = buf->pubseekpos();// ��ǰλ��
		// ͨ����ϢID��ȡ��Ϣ���������ֶΡ��������͡�
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
			// ��һ����Ϣ��������ռ�����ַ���д��buffer
			uint8_t len = messageDescription.length - HEADER_LEN;
			char *buffer = new char[len];
			buf->sgetn(buffer, len);
			PX4LogMessage *msg = messageDescription.parseMessage(buffer); // ������Ϣ������

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