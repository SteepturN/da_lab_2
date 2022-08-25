#include <iostream>
#include <inttypes.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "b_tree.cpp"

const char* return_message( ReturnMessage mssg ) {
    static const char ok[] = "Ok";
    static const char no_such_word[] = "NoSuchWord";
    static const char exist[] = "Exist";
    static const char error[] = "ERROR";
    switch( mssg ) {
        case ReturnMessage::Ok:
            return ok;
        case ReturnMessage::NoSuchWord:
            return no_such_word;
        case ReturnMessage::Exist:
            return exist;
        case ReturnMessage::ERROR:
            return error;
    }
}

int main() {
#ifndef RELEASE_VERSION
    int count_of_commands = 1;
#endif
    const int memory_size = 1024 * 1024 * 100;
    const int blocks_count = 13;
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

    BTree b_tree( memory, memory_size, blocks_count );

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
#ifndef RELEASE_VERSION
        if( memory[ cur_pos ] == '/' ) {
            while( memory[ cur_pos++ ] != '\n' );
            continue;
        }
        std::cout << count_of_commands++ << '\t';
#endif
        if( memory[ cur_pos ] == '+' ) { //add
            std::cout << "//+\t";
            // + aaa 123

            std::cout << return_message( b_tree.add( cur_pos += 2 ) ) << std::endl;

            while( memory[ ++cur_pos ] != '\n' );
        } else if( memory[ cur_pos ] == '-' ) { //remove
            std::cout << "//-\t";

            std::cout << return_message( b_tree.remove( cur_pos += 2 ) ) << std::endl;

            while( memory[ ++cur_pos ] != '\n' );
        } else if( memory[ cur_pos ] == '!' ) { //write to file

           if( memory[ cur_pos += 2 ] == 'S' ) { //! Save /asfd
                std::cout << "//! Save\t";
                std::cout << return_message( b_tree.save_to_file( cur_pos += 5 ) ) << std::endl;
            } else {
                std::cout << "//! Load\t"; //! Load /asdf
                std::cout << return_message( b_tree.load_from_file( cur_pos += 5 ) ) << std::endl;
            }

            while( memory[ ++cur_pos ] != '\n' );
        } else { //find word
            std::cout << "//find\t";
            StringIntLocation data;
            ReturnMessage mssg = b_tree.find( cur_pos, data );
            std::cout << return_message( mssg ) << '\t';
            if( mssg == ReturnMessage::Ok ) {
                while( !is_separator( memory[ data ] ) ) {
                    std::cout << memory[ data++ ];
                }
            }
            std::cout << std::endl;
            while( memory[ ++cur_pos ] != '\n' );
        }
        ++cur_pos;
    }
    close( fd );
    return 0;
}
