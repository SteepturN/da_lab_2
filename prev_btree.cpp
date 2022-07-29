//можно сделать сумму какого-то еколичество чимволов начальных и потом по ней сортировать
//а не по строкам, и только если суммы совпадут, то сравнивать оставшиеся строки
//[ строка значение ]
#define N 5
static char* memory;
//(nodes_amount) - (left_ptr + (2N-1)*2 ptr) (...) nodes_amount times


class BTree {
    int memory_size;
    Node* start_node;
    class Node {
        Node* left_ptr;
        tNode* arr; //2N - 1
                    //2 указателя в каждом:
        //на следующий узел и на начало строки со значением
    };
    find
    bool insert( char* el ) {
        bool found = false;
        Node* cur_node = start_node;
        int node_size;
        while( !found ) {
            node_size
            while(  )
        }
    }
    bool remove( char* el );
    char* data( char* el );
    bool save( char* file_name );
    bool load( char* file_name );
};
struct tNode {
    char* data;
    Node* next;
};
