#include "functions.h"
#include "PX4LogMessageDescription.h"

using namespace std;

class PX4LogReader {
	int HEADER_LEN = 3;
	int HEADER_HEAD1 = 0xA3;
	int HEADER_HEAD2 = 0x95;
	uint64_t dataStart = 0;
	unordered_map<unsigned char, PX4LogMessageDescription> messageDescriptions;
	unordered_map<string, string> fieldsList;
	uint64_t time = 0;
	//PX4LogMessage lastMsg;
	uint64_t sizeUpdates = 0;
	uint64_t sizeMicroseconds = -1;
	uint64_t startMicroseconds = -1;
	unordered_map<string, string> version;
	unordered_map<string, string> parameters;
	string tsName;
	bool tsMicros;

	static unordered_set<string> hideMsgs;
	static unordered_map<string, string> formatNames;
	
public:

	PX4LogReader(string filename) {
		//readFormats();
		//updateStatistics();
	}

	/* 读取消息头 */
	int readHeader(char *buf);

	/* 读取消息格式 */
	void readFormats(char *buf);



//private:
	//hideMsgs = new unordered_set<string>
	//formatNames

};

unordered_set<string> PX4LogReader::hideMsgs = unordered_set<string>({ "PARM", "FMT", "TIME", "VER" });
unordered_map<string, string> PX4LogReader::formatNames = unordered_map<string, string>({ \
																			{"b","int8"}, \
																			{"B", "uint8"}, \
																			{"L", "int32 * 1e-7 (lat/lon)"}, \
																			{"i", "int32"}, \
																			{"I", "uint32"}, \
																			{"q", "int64"}, \
																			{"Q", "uint64"}, \
																			{"f", "float"}, \
																			{"c", "int16 * 1e-2"}, \
																			{"C", "uint16 * 1e-2"}, \
																			{"e", "int32 * 1e-2"}, \
																			{"E", "uint32 * 1e-2"}, \
																			{"n", "char[4]"}, \
																			{"N", "char[16]"}, \
																			{"Z", "char[64]"}, \
																			{"M", "uint8 (mode)"}});


int PX4LogReader::readHeader(char *buf) {
	int i = 0;
	unsigned char byte1 = buf[i++] & 0xFF;
	unsigned char byte2 = buf[i++] & 0xFF;
	unsigned char _msg_type = buf[i++] & 0xFF;

	if (byte1 != HEAD_BYTE1 || byte2 != HEAD_BYTE2) {
		cerr << "wrong HEADER";
		return -1;
	}
	else
	{
		return _msg_type;
	}
}

void PX4LogReader::readFormats(char *buf) {
	unordered_map<string, string> fieldsList;

	while (true) {
		int msgType = readHeader(buf);
		if (msgType == FORMAT.type) {
			// 写入了每种消息的描述
			PX4LogMessageDescription msgDescr(buf);
			messageDescriptions.insert({msgDescr.get_type, msgDescr});



		}
	}
}


int main() {

	// 要读入整个文件，必须采用二进制打开   
	ifstream ifile("15_49_43.px4log", ios::in | ios::binary);

	//以写入和在文件末尾添加的方式打开.txt文件，没有的话就创建该文件。
	//ofstream ofile("loggong.txt", std::ios::out | std::ios::app); 

	//  获取filestr对应buffer对象的指针   
	streambuf *pbuf = ifile.rdbuf();

	char buffer[8192];

	// 将指针位置移动到第一个字节
	pbuf->pubseekpos(0);

	// 获取文件内容  
	pbuf->sgetn(buffer, 8192);

	//for(int i = 0; i < 10;i++)
	//	cout << ((uint8_t)buffer[i] & 0xFF) << endl;

	//char ch = pbuf->sgetc();

	//int msg_type = PX4LogReader::readHeader(buffer);
	PX4LogMessageDescription obj;

	//ofile.close();
	//ifile.close();
	return 0;

}