#include <iostream>
#include <inttypes.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
int main() {
    const int memory_size = 1024 * 1024 * 100;
    int fd = open( "input.txt", O_RDWR );
    if( fd == -1 ) {
        std::cerr << "open failed\n";
        return 1;
    }
    unsigned int string_end = 0; // 1 forward, points to the end, like end in list
    char* memory =
        reinterpret_cast< char* > ( //used_memory_end
            mmap( NULL, memory_size,
                  PROT_READ | PROT_WRITE,
                  MAP_PRIVATE, fd, 0 )
        );
    BTree b_tree( memory, memory_size );
    if( memory == MAP_FAILED ) {
        std::cerr << "mmap failed\n";
        return 1;
    }
    // std::cerr << "1\n";
    // for( int cur_pos = 0; cur_pos < 100; ++cur_pos ) {
    //     std::cout << memory[ cur_pos ];
    // }
    for( int cur_pos = 0; ( memory[ cur_pos ] != '\0' ) ||
             ( cur_pos >= memory_size ) ;  ) {
        // std::cerr << "1\n";
        if( memory[ cur_pos ] == '+' ) { //add
            std::cout << "//+"<< std::endl;
            // + aaa 123

            std::cout << return_message( b_tree.add( cur_pos += 2 ) ) << std::endl;

            while( memory[ ++cur_pos ] != '\n' );
        } else if( memory[ cur_pos ] == '-' ) { //remove
            std::cout << "//-"<< std::endl;

            std::cout << return_message( b_tree.remove( cur_pos += 2 ) ) << std::endl;

            while( memory[ ++cur_pos ] != '\n' );
        } else if( memory[ cur_pos ] == '!' ) { //write to file

           if( memory[ cur_pos += 2 ] == 'S' ) { //! Save /asfd
                std::cout << "//! Save" << std::endl;
                std::cout << return_message( b_tree.save_to_file( cur_pos += 5 ) ) << std::endl;
            } else {
                std::cout << "//! Load" << std::endl; //! Load /asdf
                std::cout << return_message( b_tree.load_from_file( cur_pos += 5 ) ) << std::endl;
            }

            while( memory[ ++cur_pos ] != '\n' );
        } else { //find word
            std::cout << "//find " << std::endl;

            std::cout << return_message( b_tree.find( cur_pos ) ) << std::endl;
            while( memory[ ++cur_pos ] != '\n' );
        }
        ++cur_pos;
    }
    close( fd );
    return 0;
}
