#include <iostream>
#include <inttypes.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <algorithm>
#include <cctype>



int main() {
    std::map< std::string, long long unsigned > tr;
    std::string str;
    long long unsigned number;
    std::string command, ch;
    while( true ) {
        std::cin >> command;
        if( !std::cin ) break;
        else if( command == "+" ) { //add
            // + aaa 123
            std::cin >> str >> number;
            std::transform( str.begin(), str.end(), str.begin(),
                            [](unsigned char c){ return std::tolower( c ); } );
            if( tr.count( str ) ) std::cout << "Exist\n";
            else {
                tr.insert( {str, number} );
                std::cout << "OK\n";
            }
        } else if( command == "-" ) { //remove
            std::cin >> str;
            std::transform( str.begin(), str.end(), str.begin(),
                            [](unsigned char c){ return std::tolower( c ); } );
            if( tr.count( str ) ) {
                std::cout << "OK\n";
                tr.erase( str );
            } else {
                std::cout << "NoSuchWord\n";
            }
        } else if( command == "!" ) { //write to file
        } else { //find word
            str = command;
            std::transform( str.begin(), str.end(), str.begin(),
                            [](unsigned char c){ return std::tolower( c ); } );
            if( tr.count( str ) ) {
                std::cout << "OK: " << tr[ str ] << std::endl;
            } else {
                std::cout << "NoSuchWord\n";
            }
        }
    }
    // b_tree.replace_strings(  );
    // close( fd );
    return 0;
}
