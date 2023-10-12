#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

struct ptpPos;
struct setting;

struct ptpPos { float x; float y; float z; float r; ptpPos(float _x, float _y, float _z, float _r) { x = _x; y = _y; z = _z; r = _r; }; ptpPos(); };

typedef struct {
	double h;       // angle in degrees
	double s;       // a fraction between 0 and 1
	double v;       // a fraction between 0 and 1
}hsv;
typedef struct {
	double r;       // a fraction between 0 and 1
	double g;       // a fraction between 0 and 1
	double b;       // a fraction between 0 and 1
} rgb;

ptpPos::ptpPos() {
	x = 0;
	y = 0;
	z = 0;
	r = 0;
}



// option configuration
struct setting
{
	setting();
	hsv player1[2];
	hsv player2[2];
	ptpPos amunition[8];
	ptpPos board[7];
	void load();
	void save();
	void displayAllPositions();
};

setting::setting() {

}


void setting::load() {

	// Create a text string, which is used to output the text file
	string data;
	cout << "Configuration loaded\n";
	// Read from the text file
	ifstream file("config.txt");
	string line, word;
	int numberOfLines = 0;
	while (getline(file, line))
	{
		stringstream str(line);
		int numberOfWord = 0;
		while (getline(str, word, ';')) {
			if (numberOfLines < 7) {
				switch (numberOfWord)
				{
				case 0:
					board[numberOfLines].x = stof(word);
					break;
				case 1:
					board[numberOfLines].y = stof(word);
					break;
				case 2:
					board[numberOfLines].z = stof(word);
					break;
				case 3:
					board[numberOfLines].r = stof(word);
					break;
				default:
					break;
				}
			}
			else {
				switch (numberOfWord)
				{
				case 0:
					amunition[numberOfLines - 7].x = stof(word);
					break;
				case 1:
					amunition[numberOfLines - 7].y = stof(word);
					break;
				case 2:
					amunition[numberOfLines - 7].z = stof(word);
					break;
				case 3:
					amunition[numberOfLines - 7].r = stof(word);
					break;
				default:
					break;
				}
			}
			numberOfWord++;
		}
		numberOfLines++;
	}

	// Close the file
	file.close();
}

void setting::save() {
	// Create and open a text file
	ofstream MyFile("config.txt", fstream::out);
	// Write to the file

	for (int i = 0; i < 7; i++)
	{
		MyFile << board[i].x << ";" << board[i].y << ";" << board[i].z << ";" << board[i].r << ";\n";
	}
	for (int i = 0; i < 8; i++)
	{
		MyFile << amunition[i].x << ";" << amunition[i].y << ";" << amunition[i].z << ";" << amunition[i].r << ";\n";
	}
	// Close the file
	MyFile.close();
}



void setting::displayAllPositions() {
	cout << "Colonnes : \n\n";
	for (int i = 0; i < 7; i++) {
		cout << "position du robot : x:" << board[i].x << " y:" << board[i].y << " z:" << board[i].z << " r:" << board[i].r << "\n";
	}

	cout << "\n\nAmmunition : \n\n";
	for (int i = 0; i < 8; i++) {
		cout << "position du robot : x:" << amunition[i].x << " y:" << amunition[i].y << " z:" << amunition[i].z << " r:" << amunition[i].r << "\n";
	}
}

