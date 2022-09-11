#include <iostream>
#include <inttypes.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstdio>
#include <cstdlib>

#define RELEASE_VERSION

#include "b_tree.cpp"

const char* return_message( ReturnMessage mssg ) {
    static const char ok[] = "OK";
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
    return error;
}

void read_until_separator( char* memory, unsigned& cur_pos ){
    const int diffrence = 'a' - 'A';
    while( !is_separator( ( memory[ cur_pos++ ] = std::getc( stdin ) ) ) ) {
        if( ( memory[ cur_pos - 1 ] >= 'A' ) && ( memory[ cur_pos - 1 ] <= 'Z' ) )
            memory[ cur_pos - 1 ] += diffrence;
    }
}
int main() {
#ifndef RELEASE_VERSION
    int count_of_commands = 1;
#endif
    const unsigned minimum_memory_size = sysconf( _SC_PAGE_SIZE );
    const unsigned multiplier = 2 << 17;
    const unsigned memory_size = multiplier * minimum_memory_size;
    const unsigned blocks_count = 2 << 12;

    char* memory =
        reinterpret_cast< char* > ( //used_memory_end
            mmap( NULL,
                  memory_size,
                  PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 )
        );

    BTree b_tree( memory, memory_size, blocks_count );

    char command;
    for( unsigned cur_pos = 0, prev_pos = 0; cur_pos < memory_size; ) {
        while( is_separator( command = std::getc( stdin ) ) );
        if( command == EOF ) break;
        else if( command == '/' ) {
            b_tree.print();
            std::getc( stdin );
            // std::cout << std::getc( stdin ) << std::endl;
        } else if( command == '+' ) { //add
            // + aaa 123
            // std::cout << "+\t" << std::endl;
            std::getc( stdin );
            prev_pos = cur_pos;
            read_until_separator( memory, cur_pos );
            std::scanf( "%llu", reinterpret_cast< long long unsigned* >( memory + cur_pos ) );
            cur_pos += 8;
            std::printf( "%s\n", return_message( b_tree.add( prev_pos ) ) );
        } else if( command == '-' ) { //remove
            // std::cout << "-\t"<< std::endl;
            std::getc( stdin );
            prev_pos = cur_pos;
            read_until_separator( memory, cur_pos );
            cur_pos = prev_pos;
            std::printf( "%s\n", return_message( b_tree.remove( cur_pos ) ) );
        } else if( command == '!' ) { //write to file
            std::getc( stdin );
            if( std::getc( stdin ) == 'S' ) { //! Save /asfd
                // std::cout << "! Save\t"<< std::endl;
                while( std::getc( stdin ) != ' ' );
                prev_pos = cur_pos;
                while( !is_separator( memory[ cur_pos++ ] = std::getc( stdin ) ) );
                memory[ cur_pos - 1 ] = '\0';
                cur_pos = prev_pos;
                std::printf( "%s\n", return_message( b_tree.save_to_file( cur_pos ) ) );
            } else { //! Load /asdf
                // std::cout << "! Load\t"<< std::endl;
                while( std::getc( stdin ) != ' ' );
                prev_pos = cur_pos;
                while( !is_separator( memory[ cur_pos++ ] = std::getc( stdin ) ) );
                memory[ cur_pos - 1 ] = '\0';
                cur_pos = prev_pos;
                std::printf( "%s\n", return_message( b_tree.load_from_file( cur_pos ) ) );
            }
        } else { //find word
            // std::cout << "find\t"<< std::endl;
            StringIntLocation data;
            prev_pos = cur_pos;
            std::ungetc( command, stdin );
            // memory[ cur_pos++ ] = command;
            read_until_separator( memory, cur_pos );
            cur_pos = prev_pos;
            ReturnMessage mssg = b_tree.find( cur_pos, data );
            std::printf( "%s", return_message( mssg ) );
            if( mssg == ReturnMessage::Ok )
                std::printf( ": %llu", *reinterpret_cast< long long unsigned* >( memory + data ) );
            std::printf( "\n" );
        }
    }
    // b_tree.replace_strings(  );
    // close( fd );
    return 0;
}
