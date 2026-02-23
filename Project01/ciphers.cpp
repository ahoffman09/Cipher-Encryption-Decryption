#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "include/caesar_dec.h"
#include "include/caesar_enc.h"
#include "include/subst_dec.h"
#include "include/subst_enc.h"
#include "utils.h"

using namespace std;

// Initialize random number generator in .cpp file for ODR reasons
std::mt19937 Random::rng;

const string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Function declarations go at the top of the file so we can call them
// anywhere in our program, such as in main or in other functions.
// Most other function declarations are in the included header
// files.

// When you add a new helper function, make sure to declare it up here!
vector<char> decryptSubstCipher(const QuadgramScorer& scorer, const string& ciphertext);
bool searchDict(const vector<string>& dict, string word);
void decryptSubstCipherCommandFile(const QuadgramScorer& scorer);

/**
 * Print instructions for using the program.
 */
void printMenu();

int main() {
  Random::seed(time(NULL));
  string command;

  cout << "Welcome to Ciphers!" << endl;
  cout << "-------------------" << endl;
  cout << endl;

  vector<string> dictContents;
  ifstream inFile("dictionary.txt");
  string word;
  while(getline(inFile, word)) {
    dictContents.push_back(word);
  }

  vector<string> quadgrams;
  vector<int> counts;
  string quadInf;
  string quad;
  string num;
  ifstream quadInFile("english_quadgrams.txt");
  while(getline(quadInFile, quadInf)) {
    istringstream iss(quadInf);
    getline(iss, quad, ',');
    getline(iss, num);

    quadgrams.push_back(quad);
    counts.push_back(stoi(num));
  }

  QuadgramScorer scorer(quadgrams, counts);

  do {
    printMenu();
    cout << endl << "Enter a command (case does not matter): ";

    // Use getline for all user input to avoid needing to handle
    // input buffer issues relating to using both >> and getline
    getline(cin, command);
    cout << endl;

    if (command == "R" || command == "r") {
      string seed_str;
      cout << "Enter a non-negative integer to seed the random number "
              "generator: ";
      getline(cin, seed_str);
      Random::seed(stoi(seed_str));
    }
    if(command == "C" || command == "c") {
      caesarEncryptCommand();
    }
    if(command == "D" || command == "d") {
      caesarDecryptCommand(dictContents);
    }
    if(command == "A" || command == "a") {
      applyRandSubstCipherCommand();
    }
    if(command == "E" || command == "e") {
      computeEnglishnessCommand(scorer);
    }
    if(command == "S" || command == "s") {
      decryptSubstCipherCommand(scorer);
    }
    if(command == "F" || command == "f") {
      decryptSubstCipherCommandFile(scorer);
    }

    cout << endl;

  } while (!(command == "x" || command == "X") && !cin.eof());

  return 0;
}

void printMenu() {
  cout << "Ciphers Menu" << endl;
  cout << "------------" << endl;
  cout << "C - Encrypt with Caesar Cipher" << endl;
  cout << "D - Decrypt Caesar Cipher" << endl;
  cout << "E - Compute English-ness Score" << endl;
  cout << "A - Apply Random Substitution Cipher" << endl;
  cout << "S - Decrypt Substitution Cipher from Console" << endl;
  cout << "F - Decrypt Substitution Cipher from File" << endl;
  cout << "R - Set Random Seed for Testing" << endl;
  cout << "X - Exit Program" << endl;
}

// "#pragma region" and "#pragma endregion" group related functions in this file
// to tell VSCode that these are "foldable". You might have noticed the little
// down arrow next to functions or loops, and that you can click it to collapse
// those bodies. This lets us do the same thing for arbitrary chunks!
#pragma region CaesarEnc

char rot(char c, int amount) {
  size_t newC = ALPHABET.find(c);
  newC += amount;
  newC %= 26;
  return ALPHABET.at(newC);
}

string rot(const string& line, int amount) {
  string result;
  for(char c : line) {
    if(isalpha(c)) {
      result += rot(toupper(c), amount);
    } else if (isspace(c)){
      result += c;
    } else {
      continue;
    }
  }
  return result;
}

void caesarEncryptCommand() {
  cout << "Enter the text to encrypt: ";
  string inpLine;
  getline(cin, inpLine);
  cout << endl;
  string encAmt;
  cout << "Enter the number of characters to rotate by: ";
  getline(cin, encAmt);
  cout << endl;

  string encLine = rot(inpLine, stoi(encAmt));
  cout << encLine << endl;
}

#pragma endregion CaesarEnc

#pragma region CaesarDec

void rot(vector<string>& strings, int amount) {
  for (string& word : strings) {
    word = rot(word, amount);
  }
}

string clean(const string& s) {
  string cleaned;
  for (char let : s) {
    if(isalpha(let)) {
      cleaned.push_back(toupper(let));
    }
  }
  return cleaned;
}

vector<string> splitBySpaces(const string& s) {
  vector<string> splitWords;
  string word;
  istringstream iss(s);

  while(iss >> word) {
    splitWords.push_back(word);
  }

  return splitWords;
}

string joinWithSpaces(const vector<string>& words) {
  string joined;
  for(size_t i = 0; i < words.size(); i++) {
    joined += words[i];
    if(i != words.size() - 1) {
      joined += " ";
    }
  }
  return joined;
}

// Helper function - searches the dictionary dict for word
bool searchDict(const vector<string>& dict, string word) {
  for(string dWord : dict) {
    if(dWord == word) {
      return true;
    }
  }
  return false;
}

int numWordsIn(const vector<string>& words, const vector<string>& dict) {
  int numWords = 0;
  for(string word : words) {
    if(searchDict(dict, word)) {
      numWords++;
    }
  }
  return numWords;
}

void caesarDecryptCommand(const vector<string>& dict) {
  string decText;
  cout << "Enter the text to Caesar decrypt: ";
  getline(cin, decText);
  cout << endl;
  
  vector<string> wordsToDec = splitBySpaces(decText);
  for(string& word : wordsToDec) {
    word = clean(word);
  }

  int threshold = wordsToDec.size() / 2;

  bool lineOutput = false;
  vector<string> testLine;
  for(int i = 0; i < 26; i++) {
    testLine = wordsToDec;
    rot(testLine, i);
    if(numWordsIn(testLine, dict) > threshold) {
      string goodDec = joinWithSpaces(testLine);
      cout << goodDec << endl;
      lineOutput = true;
    }
  }
  if(!lineOutput) {
    cout << "No good decryptions found" << endl;
  }

}

#pragma endregion CaesarDec

#pragma region SubstEnc

string applySubstCipher(const vector<char>& cipher, const string& s) {
  string encTxt;
  int index = 0;
  for(char c : s) {
    if(isalpha(c)) {
      index = toupper(c) - 65;
      encTxt += cipher[index];
    } else {
      encTxt += c;
    }
  }
  return encTxt;
}

void applyRandSubstCipherCommand() {
  cout << "Enter the text to encrypt: ";
  string lineToEnc;
  getline(cin, lineToEnc);
  cout << endl;

  vector<char> cipher = genRandomSubstCipher();

  string encLine = applySubstCipher(cipher, lineToEnc);

  cout << encLine << endl;
}

#pragma endregion SubstEnc

#pragma region SubstDec

double scoreString(const QuadgramScorer& scorer, const string& s) {
  double score = 0.0;
  
  vector<string> words = splitBySpaces(s);

  for(string word : words) {
    for(int i = 0; i < word.length() - 3; i++) {
      string quad = word.substr(i, 4);
      score += scorer.getScore(quad);
    }
  }
  return score;
}

void computeEnglishnessCommand(const QuadgramScorer& scorer) {
  cout << "Enter a string for englishness scoring: ";
  string txt2Scr;
  getline(cin, txt2Scr);
  cout << endl;
  
  
  double score = scoreString(scorer, clean(txt2Scr));
  cout << "Englishness Score: " << score << endl;
}

vector<char> hillClimb(const QuadgramScorer& scorer, const string& ciphertext) {
  vector<char> ranKey = genRandomSubstCipher();
  string testDec = applySubstCipher(ranKey, ciphertext);
  double bestScore = scoreString(scorer, clean(testDec));

  int failures = 0;

  while(failures < 1000) {
    vector<char> posKey = ranKey;
    int r1 = Random::randInt(25);
    int r2 = Random::randInt(25);
    while(r1 == r2) {
      r2 = Random::randInt(25);
    }

    char temp = posKey[r1];
    posKey[r1] = posKey[r2];
    posKey[r2] = temp;
    string testDec2 = applySubstCipher(posKey, ciphertext);
    double curScore = scoreString(scorer, clean(testDec2));

    if(curScore > bestScore) {
      ranKey = posKey;
      bestScore = curScore;
      failures = 0;
    } else {
      failures++;
    }

  }

  return ranKey;
}

vector<char> decryptSubstCipher(const QuadgramScorer& scorer, const string& ciphertext) {
  vector<char> bestKey;
  double bestScore = 0.0;
  for(int i = 0; i < 25; i++) {
    vector<char> ranKey = hillClimb(scorer, ciphertext);
    string decTxt = applySubstCipher(ranKey, ciphertext);
    double curScore = scoreString(scorer, clean(decTxt));

    if(i == 0 || curScore > bestScore) {
      bestKey = ranKey;
      bestScore = curScore;
    }
  }
  return bestKey;
}
/** string cleanWSC(const string& s) {
  string cleaned;
  for (char let : s) {
    if(isalpha(let)) {
      cleaned.push_back(toupper(let));
    } else {
      cleaned.push_back(let);
    }
  }
  return cleaned;
} */

void decryptSubstCipherCommand(const QuadgramScorer& scorer) {
  cout << "Enter the text to substitution decrypt: ";
  string txt2dec;
  getline(cin, txt2dec);
  cout << endl;

  vector<char> decKey = decryptSubstCipher(scorer, txt2dec);
  string decrypted = applySubstCipher(decKey, txt2dec);
  cout << decrypted << endl;
}

#pragma endregion SubstDec

void decryptSubstCipherCommandFile(const QuadgramScorer& scorer) {
  string fileName;
  string outFile;
  getline(cin, fileName);
  getline(cin, outFile);
  ifstream inputFile(fileName);

  cout << "Decrypting..." << endl;
  
  string decipher;
  string line;

  while(getline(inputFile, line)) {
    decipher += line + '\n';
  }
  string cleaned = clean(decipher);
  vector<char> decKey = decryptSubstCipher(scorer, cleaned);
  string decrypted = applySubstCipher(decKey, decipher);
  
  ofstream outputFile(outFile);

  outputFile << decrypted;
}
