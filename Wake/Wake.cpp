#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <Windows.h>

using namespace std;


void gen_S_block(long S[], long k[]) {
	long x, z, X, Z, p, n;

	//Вспомогательная таблица из вики
	static long ss[8] = {
		0x726a8f3b,
		0xe69a3b5c,
		0xd3c71fe5,
		0xab3c73d2,
		0x4d3a8eb3,
		0x0396d6e8,
		0x3d4c2f7a,
		0x9ee27cf3,
	};



	S[0] = k[0];
	S[1] = k[1];
	S[2] = k[2];
	S[3] = k[3];

	for (n = 4; n < 256; n++) {
		x = S[n - 1] + S[n - 4];
		S[n] = x >> 3 ^ S[x & 7];
	}

	for (n = 0; n < 23; n++)
		S[n] += S[n + 89];

	X = S[33];
	Z = S[59] | 0x01000001;
	Z = Z & 0xff7fffff;

	for (n = 0; n < 256; n++) {
		X = (X & 0xff7fffff) + Z;
		S[n] = S[n] & 0x00ffffff ^ X;
	}

	S[255] = S[0];
	X = X & 255;

	long temp = 0;
	for (n = 0; n < 256; n++) {
		temp = (S[n ^ X] ^ X) & 255;
		S[n] = S[x = temp];
		S[X] = S[n + 1];
	}
}


std::string string_to_hex(const std::string& input)
{
	static const char hex_digits[] = "0123456789ABCDEF";

	std::string output;
	output.reserve(input.length() * 2);
	for (unsigned char c : input)
	{
		output.push_back(hex_digits[c >> 4]);
		output.push_back(hex_digits[c & 15]);
	}
	return output;
}


int hex_value(char hex_digit)
{
	switch (hex_digit) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return hex_digit - '0';

	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return hex_digit - 'A' + 10;

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return hex_digit - 'a' + 10;
	}
	throw std::invalid_argument("invalid hex digit");
}


std::string hex_to_string(const std::string& input)
{
	const auto len = input.length();
	string temp = input;
	if (len & 1) {
		temp += temp[0];
		temp[0] = '0';
	}

	std::string output;
	output.reserve(len / 2);
	for (auto it = temp.begin(); it != temp.end(); )
	{
		int hi = hex_value(*it++);
		int lo = hex_value(*it++);
		output.push_back(hi << 4 | lo);
	}
	return output;
}


string int_to_hex(int k) {
	std::stringstream stream;
	stream << std::hex << k;
	std::string result(stream.str());
	return result;
}


unsigned int hex_to_int(string s) {
	std::stringstream converter(s);
	unsigned int v;
	converter >> std::hex >> v;
	return v;
}


unsigned int get_M(int x, int y, long s[]) {
	int temp = x + y;
	return temp >> 8 & 0x00ffffff ^ s[temp & 0xff];
}


void gen_autokey(long K[], long s[]) {
	K[0] = get_M(K[0], K[3], s);
	K[1] = get_M(K[1], K[0], s);
	K[2] = get_M(K[2], K[1], s);
	K[3] = get_M(K[3], K[2], s);

	string K1 = int_to_hex(K[3]);

	K[0] = hex_to_int(K1.substr(0, 2));
	K[1] = hex_to_int(K1.substr(2, 2));
	K[2] = hex_to_int(K1.substr(4, 2));
	K[3] = hex_to_int(K1.substr(6, 2));
}


void init_K(string key, long K[]) {
	string R[4];

	string hex = string_to_hex(key);
	int len = hex.length() / 4;

	int i = 0;

	for (int z = 0; z < 4; z++)
	{
		for (; i < len * (z + 1); i++) {
			R[z] += hex[i];
		}
		K[z] = hex_to_int(R[z]);
	}
}


string crypt(string text, long K[]) {
	string hex = string_to_hex(text);

	unsigned int* P = new unsigned int[text.length()];
	int n = 0;
	long x;
	string crypt;

	for (int i = 0; i < text.length(); i++) {
		P[i] = hex_to_int(hex.substr(i * 2, 2));
		x = K[n] ^ P[i];
		n++;
		crypt.append(hex_to_string(int_to_hex(x)));
		if (n > 3) {
			n = 0;
		}
	}

	return crypt;
}


int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);


	long K[4], s[256];

	string key;
	string text;

	cout << "Enter key: ";
	getline(cin, key);

	cout << "Enter the text you want to encrypt/decrypt: ";
	getline(cin, text);

	init_K(key, K);

	gen_S_block(s, K);

	gen_autokey(K, s);

	string crypto = crypt(text, K);

	cout << "Result: " << crypto << endl;

	int is_save;

	cout << "Save result in file? (0 - No, 1 - Yes): ";

	cin >> is_save;

	if (is_save) {
		std::ofstream out("result.txt");
		out << crypto;
		out.close();
	}

	return 0;
}