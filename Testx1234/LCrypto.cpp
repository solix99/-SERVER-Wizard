#include "pch.h"
#include "LCrypto.h"
#include <stdlib.h>

LCrypto::LCrypto()
{
    srand(time(NULL));

    bool isPrime = false;

    int p = rand() % 1000 + 70;
    int q = rand() % 1000 + 70;

    while (!isPrime)
    {
        p++;
        for (unsigned int i = 2; i <= p / 2; ++i)
        {
            isPrime = true;
            if (p % i == 0)
            {
                isPrime = false;
                break;
            }
        }
    }

    isPrime = false;

    while (!isPrime)
    {
        q++;
        for (unsigned int i = 2; i <= p / 2; ++i)
        {
            isPrime = true;
            if (q % i == 0)
            {
                isPrime = false;
                break;
            }
        }
    }

    double track;
    nModulus_ = p * q;
    double phi = (p - 1) * (q - 1);

    //public key e

    publicKey_ = 7;

    while (publicKey_ < phi)
    {
        track = gcd(publicKey_, phi);

        if (track == 1)
        {
            break;
        }
        else
        {
            publicKey_++;
        }
    }
    //private key
    //choosing d such that it satisfies d*e = 1 mod phi

    double d1 = 1 / publicKey_;
    privateKey_ = fmod(d1, phi);
    
}

void LCrypto::encryptData(string data)
{
    dataSize_ = data.length();

    for (unsigned int i = 0; i < dataSize_; i++)
    {
        encryptArray_[i] = pow((int)data.at(i), publicKey_);
    }

    dataPacket_.clear();
    dataPacket_.str(string());

    for (unsigned int i = 0; i < dataSize_; i++)
    {
        dataPacket_ << encryptArray_[i] << ",";
    }
}

void LCrypto::decryptData(string data)
{
    int charFound = 0;

    for (int i = 0; i < data.length(); i++)
    {
        if (data.at(i) == ',')
        {
            charFound++;
        }
    }

    data_.resize(charFound, ' ');

    v1 = data.find(',');
    decryptArray[0] = stod(data.substr(0, v1).c_str());

    for (unsigned int i = 1; i < charFound; i++)
    {
        decryptArray[i] = stod(getDataBlock(data).c_str());
    }
   
    for (unsigned int i = 0; i < charFound; i++)
    {
        decryptArray[i] = pow(decryptArray[i], privateKey_);
        data_.at(i) = char(decryptArray[i]);
    }
}

void LCrypto::setPrivateKey(double key)
{
    privateKey_ = key;
}
void LCrypto::setPublicKey(double key)
{
    publicKey_ = key;
}
string LCrypto::getData()
{
    return data_;
}
string LCrypto::getDataPacket()
{
    return dataPacket_.str();
}

int LCrypto::gcd(int a, int b)
{
    int t;
    while (1)
    {
        t = a % b;
        if (t == 0)
            return b;
        a = b;
        b = t;
    }
}
int LCrypto::getModulus()
{
    return nModulus_;
}
double LCrypto::getPublicKey()
{
    return publicKey_;
}

string LCrypto::getDataBlock(const string& data)
{
    v2 = v1;
    v1 = data.find(',', v2 + 1);
    return data.substr(v2 + 1, v1 - v2 - 1);
}
