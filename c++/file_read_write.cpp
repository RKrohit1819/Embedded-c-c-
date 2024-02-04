#include<iostream>
#include<fstream>

using namespace std;

// int main(){
//     string st = "Rohit bhai";
//     // Opening files using constructor and writing it
//     ofstream out("write.txt"); // Write operation
//     out<<st;

//     return 0;
// }

int main(){
    string st2;
    // Opening files using constructor and reading it
    ifstream in("read.txt"); // Read operation
    in>>st2;
    getline(in, st2);  
    cout<<st2;
}