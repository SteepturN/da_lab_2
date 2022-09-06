#include <iostream>
#include <inttypes.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define RELEASE_VERSION

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
    const unsigned minimum_memory_size = sysconf( _SC_PAGE_SIZE );
    const unsigned multiplier = 1;
    const unsigned memory_size = multiplier * minimum_memory_size;
    const unsigned blocks_count = 2 << 10;


    int fd = open( "input.txt", O_RDWR );
    if( fd == -1 ) {
        std::cerr << "open failed\n";
        return 1;
    }

    unsigned strings_end = 0;
    char* memory;
    BTree b_tree( memory, memory_size, blocks_count );
    for( int memory_readed = 0, cur_pos = strings_end; !end; ++j ) {

        memory =
            reinterpret_cast< char* > ( //used_memory_end
                mmap( memory + strings_end,
                      memory_size - strings_end,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE, fd, memory_readed )
            );
        if( memory == MAP_FAILED ) {
            std::cerr << "mmap failed\n";
            return 1;
        }
        for( cur_pos = strings_end, unsigned pos_for_function = 0;
             ( memory[ cur_pos ] != '\0' ) && ( cur_pos < memory_size ) ;  ) {
            // std::cerr << "1\n";
#ifndef RELEASE_VERSION
            if( memory[ cur_pos ] == '/' ) {
                while( memory[ cur_pos++ ] != '\n' );
                continue;
            }
            std::cout << count_of_commands++ << '\t';
#endif
            if( memory[ cur_pos ] == '+' ) { //add
                std::cout << "+\t";
                // + aaa 123

                std::cout << return_message( b_tree.add( cur_pos += 2 ) ) << std::endl;

                while( memory[ ++cur_pos ] != '\n' ) {
                    // if( memory[ cur_pos ] == '\0' ) -- impossible
                    if( cur_pos == memory_size - 1 ) {

                    }
                }


            } else if( memory[ cur_pos ] == '-' ) { //remove
                std::cout << "-\t";

                std::cout << return_message( b_tree.remove( cur_pos += 2 ) ) << std::endl;

                while( memory[ ++cur_pos ] != '\n' );
            } else if( memory[ cur_pos ] == '!' ) { //write to file
                if( memory[ cur_pos += 2 ] == 'S' ) { //! Save /asfd
                    std::cout << "! Save\t";
                    std::cout << return_message( b_tree.save_to_file( cur_pos += 5 ) ) << std::endl;
                    while( memory[ ++cur_pos ] != '\n' );
                } else { //! Load /asdf
                    bool changes_needed = false;
                    std::cout << "! Load\t";
                    pos_for_function = cur_pos += 5;
                    while( memory[ ++cur_pos ] != '\n' );
                    std::cout << return_message(
                        b_tree.load_from_file( pos_for_function, strings_end, changes_needed )
                    )
                              << std::endl;
                    if( changes_needed && ( cur_pos < strings_end ) ) {
                        //unsigned lost_info = strings_end - cur_pos;

                        memory =
                            reinterpret_cast< char* > ( //used_memory_end
                                mmap( memory + strings_end,
                                    memory_size - strings_end,
                                    PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE, fd,
                                      j * memory_size +
                                      cur_pos - ( cur_pos % minimum_memory_size )
                            );
                       cur_pos = cur_pos % minimum_memory_size;
                    }
                }

            } else { //find word
                std::cout << "find\t";
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
        b_tree.replace_strings(  );
    }
    close( fd );
    return 0;
}


#ifdef RELEASE_VERSION
у меня могут возникнуть проблемы со считыванием, если оно происходит около кона памяти,
    лучше всего его там заканчивать -> while( cur_pos++ != '\n' ) может не сработать,
    так как могут появиться \0 и закончится доступная мне память, и даже при выполнении функций
    может возникнуть пиздец, из-за незаконченной строки
#endif
