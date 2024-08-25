#include <iostream>
#include <vector>
using namespace std;
int main () {
    vector<vector<int>> v;
    v.resize(3);
    for(int i = 0 ; i < v.size();i++) {
        v[i].push_back(1);
    }
}
