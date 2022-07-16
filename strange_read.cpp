#include <iostream>
#include <inttypes.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    char* memory = reinterpret_cast< char* >( std::malloc( 1000000 ) );
    if( memory == NULL ) return 1;
    for( int i = 0; i < 100; ++i ) {
        memory[ i ] = 0;
    }
    // std::cout << malloc_usable_size( memory ) << '\n';
    // int fd = open( "./file", O_RDONLY );
    FILE* fd = fopen( "./file", "rb" );
    /* const */ int /* std::size_t */ size = 1;
    while( size != 0 ) {
        size = fread( memory, sizeof( char ), sizeof( memory ), fd );
        std::cout << size;
        for( int i = 0; i <  )
    // while( size != 0 ) {
    //     size = read( fd, memory, sizeof( memory ) );
    //     std::cout << size << '\n';
    }




    for( int cur_pos = 0; cur_pos < size; ) {
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
            std::cout << "find" << std::endl;
            cur_pos++;
            while( memory[ cur_pos++ ] != '\n' ) {
                // sleep( 2 );
                std::cout << memory[ cur_pos ] << std::endl;
            }
        }
    }
    // close( fd );
    fclose( fd );

    free( memory );
}
