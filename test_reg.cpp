#include <iostream>
#include <string>
#include <regex>
using namespace std;
int main() {
    std::regex r("([FLS])|(R)([0-9]+)|(([PE])([0-9]+)(:([0-9]+))?)");
    std::string line = "P10:3";
    std::smatch sche_match;
    if(std::regex_search(line, sche_match, r)) {
        cout << sche_match.size() << endl;
        for(int i=0; i<sche_match.size(); i++) {
            if(sche_match[i].length()!=0) {
            cout << sche_match[i] <<endl;
            }
        }
    }
    std::cout << "Hello World!\n";
}