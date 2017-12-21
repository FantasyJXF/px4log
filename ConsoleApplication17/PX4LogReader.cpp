#include "functions.h"
//#include "PX4LogMessageDescription.h"


using namespace std;



class PX4LogReader {
	int HEADER_LEN = 3;
	int HEADER_HEAD1 = 0xA3;
	int HEADER_HEAD2 = 0x95;
	uint64_t dataStart = 0;
	//unordered_map<int, PX4LogMessageDescription> messageDescription;
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
	int readHeader(char *buf) {
		int i = 0;
		int byte1 = (int)buf[i++] & 0xFF;
		int byte2 = (int)buf[i++] & 0xFF;
		int _msg_type = (int)buf[i++] & 0xFF;

		//if (byte1 != HEAD_BYTE1 || byte2 != HEAD_BYTE1) {
		if (byte1 != HEAD_BYTE1 || byte2 != HEAD_BYTE2) {
			cerr << "wrong HEADER";
			return -1;
		}
		else
		{
			return _msg_type;
		}
	}

private:
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
																			{"M", "uint8 (mode)"}
});

//void readFormat(char *buf) {
//	unordered_map<string, string> fieldsList;
//	int msgType = readHeader(buf);
//	if(msgType == )
//}


int main() {

	// 要读入整个文件，必须采用二进制打开   
	ifstream ifile("15_49_43.px4log", ios::in | ios::binary);

	//以写入和在文件末尾添加的方式打开.txt文件，没有的话就创建该文件。
	ofstream ofile("loggong.txt", std::ios::out | std::ios::app); 

	//  获取filestr对应buffer对象的指针   
	streambuf *pbuf = ifile.rdbuf();

	char buffer[8192];

	// 将指针位置移动到第一个字节
	pbuf->pubseekpos(0);

	// 获取文件内容  
	pbuf->sgetn(buffer, 8192);

	char ch = pbuf->sgetc();

	//int msg_type = readHeader(buffer);

	char *cc = "aibuhdisuadhsuahfsa";
	//string sttt =  _GET_string(cc, 0);
	char sttt = _GET_char(cc, 0);

	cout << sttt << endl;
	ofile.close();
	ifile.close();
	return 0;

}