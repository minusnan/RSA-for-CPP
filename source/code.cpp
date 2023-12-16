#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <filesystem>
#include <numeric>
#include "sha256.h"

void encrypt(int p, int q, int e, std::string path);
void sign(int p, int q, int e, std::string path);
void decrypt(int p, int q, int e, std::string path);
bool check_sig(int p, int q, int e, std::string path);
unsigned long long int get_privateKey(int p,int q, int e);
int getE(int p, int q);

bool isPrime(int number) {
    if (number < 2) return false;
    if (number == 2) return true;
    if (number % 2 == 0) return false;
    for (int i = 3; (i * i) <= number; i += 2) {
        if (number % i == 0) return false;
    }
    return true;
}

void getParams(std::string path, int *p, int *q, int *e) {
    std::ifstream set(path + "/conf/config.cfg");

    if (!set) {
        std::cout << "No file! Make sure there is conf/config.cfg file in your working directory.";
        system("pause");
        exit(0);
    }
    else { std::cout << "Config found." << std::endl; }

    if (set >> *p >> *q >> *e) {
        std::cout << "Settings loaded." << std::endl;
    }
    else {
        std::cout << "Insufficient data";
        exit(0);
    }
    set.close();
}

long long fastModularExponentiation(long long base, long long exponent, long long modulus) {
    long long result = 1;
    while (exponent > 0) {
        if (exponent & 1) {
            result = (result * base) % modulus;
        }
        base = (base * base) % modulus;
        exponent >>= 1;
    }
    return result;
}

std::vector<unsigned long long int> getVectorFromFile(std::string path, std::string file) { 
    unsigned long long int temp = 0;
    std::ifstream sign(path + file);
    std::string res;
    std::vector<unsigned long long int> vec;
    if (!sign) {
        std::cout << "No file! Make sure there is " + file + " file in your working directory.";
        system("pause");
        exit(0);
    }
    else { std::cout << "File found! Location: " << path + file << std::endl; }
    while (sign >> res) {
        temp = std::stoull(res);
        vec.push_back(temp);
    }
    sign.close();
    return vec;
    }

std::string getHash(std::string path, std::string file) {
    SHA256 sha256;
    std::string line = "";
    std::string res = "";
    std::ifstream get_line(path + file);

    if (!get_line) {
        std::cout << "No file! Make sure there is " + file + " in your working directory.";
        system("pause");
        exit(0);
    }
    else { std::cout << "File found! Location: " << path + file << std::endl; }

    while (getline(get_line, line)) {
        res += line;
    }
    get_line.close();

    std::string hash = sha256(res);
    std::cout << "HASH: " << hash << std::endl;
    return hash;
}

std::string getLineFromFile(std::string path, std::string file) {
    std::ifstream readText(path + file);
    if (!readText) { 
        std::cout << "No file! Make sure there is " + file + " in your working directory."; 
        system("pause");
        exit(0);
    }
    else { std::cout << "File found! Location: " << path + file << std::endl; }
    std::string line = "";
    std::string res;
    while (getline(readText, line)) {
        res += line;
    }
    readText.close();
    return res;
}

int getE(int p, int q) {
    unsigned long long int euler = (p - 1) * (q - 1);
    for (int i = 2; i < euler; i++) {
        if (std::gcd(i, euler) == 1) return i;
    }
    return 5;
}

void configure(std::string path) {
    int p = 4;
    int q = 4;

    while (!isPrime(p) || !isPrime(q)) {
        std::cout << "Enter prime p" << std::endl;
        std::cin >> p;
        std::cout << "Enter prime q" << std::endl;
        std::cin >> q;
        if (!std::cin)
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    int e = getE(p, q);
    std::cout << "e = " << e << "\n\n";
    std::ofstream conf(path + "/conf/config.cfg");
    conf << p << " " << q << " " << e;
    conf.close();
}

int main() {
    std::string path = std::filesystem::current_path().generic_string();
    int choice;
    bool sigCheck = false;
    int p = 0;
    int q = 0;
    int e = 0;
    getParams(path, &p, &q, &e);

    while (true) {
        std::cout << "Select action:\n\n" << "0. Exit\n" << "1. Encrypt text.txt\n" << "2. Sign message.enc\n"
            << "3. Check signature and decrypt\n" << "4. Configure\n" << std::endl;
        std::cin >> choice;
        switch (choice) {
        case 0:
            std::cout << "Program closed.\n\n";
            system("pause");
            exit(0);
        case 1:
            encrypt(p, q, e, path);
            break;
        case 2:
            sign(p, q, e, path);
            break;
        case 3:
            sigCheck = check_sig(p, q, e, path);
            if (sigCheck) {
                std::cout << "\n\n";
                decrypt(p, q, e, path);
            }
            break;
        case 4:
            configure(path);
            getParams(path, &p, &q, &e);
            break;
        default:
            std::cout << "No such option\n\n";

            break;
        }
    }
    system("pause");
    return 0;
}

unsigned long long int get_privateKey(int p, int q, int e) {
    int k = 0;
    int euler = (p - 1) * (q - 1);
    double d = 0;

    while (k < euler) {
        d = (k * euler + 1) / (double)e;

        if (std::fmod(d, 1) == 0) {
            std::cout << "now d is integer! k = " << k << " d = " << d << std::endl;
            break;
        }
        else {
            std::cout << "d is not yet integer! k = " << k << " d = " << d << std::endl;
            k++;
        }
    }
    return (unsigned long long int)d;
}


void encrypt(int p, int q, int e, std::string path) {
    std::string file = "/text.txt";
    int n = p * q;
    unsigned long long int temp = 0;
    std::vector<unsigned long long int> sequence;
    std::string line = getLineFromFile(path, file);
    std::cout << "Original text from text.txt: " << line << "\n\n";

    for (int i = 0; i < line.length(); i++) {
        temp = fastModularExponentiation((unsigned long long int)line[i], e, n);
        sequence.push_back(temp);
    }

    file = "/message.enc";
    std::ofstream writeEnc(path + file);
    for (int i = 0; i < sequence.size() - 1; i++) { 
        writeEnc << sequence[i] << " ";
    }
    writeEnc << sequence[sequence.size() - 1];

    std::cout << "Encrypted text (char to ASCII (int) + encryption): ";

    for (int i = 0; i < sequence.size(); i++) {
        std::cout << sequence[i] << " ";
    }
    std::cout << "\n\n";
}

void decrypt(int p, int q, int e, std::string path) {

    std::string origLine = "";
    std::string file = "/message.enc";
    unsigned long long int d = get_privateKey(p, q, e);
    unsigned long long int n = p * q;
    unsigned long long int temp = 0;

    std::vector<unsigned long long int> seq = getVectorFromFile(path, file);


    for (int i = 0; i < seq.size(); i++) {
        temp = fastModularExponentiation(seq[i], d, n);
        origLine += (char)temp;
    }
    std::cout << "Decrypted text (decryption + ASCII (int) to char): " << origLine << std::endl;

    std::ofstream dec(path + "/decrypted.txt");
    dec << origLine;
}

void sign(int p, int q, int e, std::string path) {
    std::vector<unsigned long long int> sig;
    std::string origLine = "";
    std::string file = "/message.enc";
    unsigned long long int n = p * q;
    unsigned long long int temp = 0;
    std::string hash = getHash(path, file);
    unsigned long long int d = get_privateKey(p, q, e);

    for (int i = 0; i < hash.length(); i++) {
        temp = fastModularExponentiation((unsigned long long int)hash[i], d, n);
        sig.push_back(temp);
    }

    std::cout << "SIGNATURE: ";

    for (int i = 0; i < sig.size(); i++) {
        std::cout << sig[i] << " ";
    }
    std::cout << "\n\n";

    std::ofstream signa(path + "/message.enc.sig");
    
    for (int i = 0; i < sig.size() - 1; i++) {
        signa << sig[i] << " ";
    }
    signa << sig[sig.size() - 1];
    signa.close();
}

bool check_sig(int p, int q, int e, std::string path) {
    unsigned long long int n = p * q;
    unsigned long long int temp = 0;
    std::string sigCheck;
    std::string file = "/message.enc";
    std::string hash = getHash(path, file);
    file = "/message.enc.sig";
    std::vector<unsigned long long int> sig = getVectorFromFile(path, file);

    for (int i = 0; i < sig.size(); i++) {
        temp = fastModularExponentiation(sig[i], e, n);
        sigCheck += (char)temp;
    }

    if (sigCheck == hash) { std::cout << "Signature check - SUCCESS!" << std::endl; return true; }
    else { std::cout << "Signature check - FAILURE!" << "\n\n"; return false; }
}
