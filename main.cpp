#include <iostream>
#include <inttypes.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
int main() {
    int fd = open( "input.txt", O_RDWR );
    if( fd == -1 ) {
        std::cerr << "open failed\n";
        return 1;
    }
    char* memory = reinterpret_cast< char* >(
        mmap( NULL, 1024 * 1024 * 50,
              PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, fd, 0 )
    );
    if( memory == NULL ) {
        std::cerr << "mmap failed\n";
        return 1;
    }
    std::cerr << "1\n";
    for( int cur_pos = 0; cur_pos < 100; ++cur_pos ) {
        std::cout << memory[ cur_pos ];
    }
    for( int cur_pos = 0; memory[ cur_pos ] != '\0';  ) {
        // std::cerr << "1\n";
        if( memory[ cur_pos ] == '+' ) {
            std::cout << "+"<< std::endl;
            cur_pos++;
            while( memory[ cur_pos++ ] != '\n' );
        } else if( memory[ cur_pos ] == '-' ) {
            std::cout << "-"<< std::endl;
            cur_pos++;
            while( memory[ cur_pos++ ] != '\n' );
        } else if( memory[ cur_pos ] == '!' ) { //write to file
            std::cout << "!" << std::endl;
            cur_pos++;
            while( memory[ cur_pos++ ] != '\n' );
        } else { //find word
            std::cout << "find " << std::endl;
            cur_pos++;
            while( memory[ cur_pos++ ] != '\n' ) {
                // sleep( 2 );
                std::cout << memory[ cur_pos ];
            }
            std::cout << std::endl;
        }
    }
    close( fd );
    return 0;
}
