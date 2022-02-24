#pragma once

#define MAX_BUFFER 9999

#include <string>
#include <time.h>
#include <random>
#include <iostream>
#include <sstream>

using namespace std;

class LCrypto
{
public:
	LCrypto();

	void encryptData(string data);
	void decryptData(string data);

	void setPrivateKey(double key);
	void setPublicKey(double key);

	string getDataPacket();

	int gcd(int, int);
	string getData();

	int getModulus();
	double getPublicKey();
	string getDataBlock(const string& data);

private:
	double publicKey_;
	double privateKey_;
	int dataSize_;
	long double encryptArray_[MAX_BUFFER];
	long double decryptArray[MAX_BUFFER];
	stringstream dataPacket_;
	string data_;
	int nModulus_;
	int v1 = 0, v2 = 0;

};

