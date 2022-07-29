
using BlockIntPointer = unsigned char;
using ElementInBlockPointer = unsigned;
using ElementsCount = unsigned char;
using StringIntPointer = int;
static const int blocks_count = 16;
static const int minimum_degree = 1u << 15;
static const int memory_length = sizeof( BlockElement ) * block_count;
//static const max_element_count = ( 2 * minimum_degree - 2 ) * blocks_count;
static inline BlockElement* block_memory( void* mem ) {
    return reinterpret_cast< BlockElement* >( mem );
}

enum class ReturnMessage : char {
    Ok,
    NoSuchWord,
    Exist,
    ERROR,
};

class BTree {
    public:
        BTree( char*, int );
        ReturnMessage add( StringIntPointer );
        ReturnMessage remove( StringIntPointer );
        ReturnMessage save_to_file( StringIntPointer );
        ReturnMessage load_from_file( StringIntPointer );
        ReturnMessage find( StringIntPointer );
        bool empty();
        class ElementPointer {
            BlockIntPointer block;
            ElementInBlockPointer element;
        };
        class BlockElement {
            Element elements[ 2 * minimum_degree - 2 ];
            BlockIntPointer block_pointers[ 2 * minimum_degree - 1 ];
            ElementPointer parent;
            ElementsCount used;
        };
        class Element {
            StringIntPointer value;
            ElementPointer next_right_string;
            ElementPointer prev_left_string;
        };
    private:
        char* strings_memory;
        const int strings_memory_size;
        void* memory;
        BlockIntPointer first_free_block;
        BlockIntPointer first_block;
        BlockIntPointer rightmost;
//        ElementBlock* element_block;f
        BlockElement* get_block_ptr( BlockIntPointer el ) {
            return block_memory( memory ) + el;
        }
        void make_free_blocks_chain( BlockIntPointer left, BlockIntPointer right ) {
            get_block_ptr( left )->block_pointers[ 0 ] = right;
        }
        [[ nodiscard ]] BlockIntPointer get_free_block() {
            BlockIntPointer prev_first_free = first_free_block;
            first_free_block = get_block_ptr( first_block )->block_pointers[ 0 ];
            return prev_first_free;
        }
        bool find_el( StringIntPointer, ElementPointer&, StringIntPointer& );
};

BTree::BTree( char* memory, int memory_size )
    : strings_memory( memory ), strings_memory_size( memory_size ),
      memory( mmap( NULL, memory_length, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_UNINITIALISED, 0, 0 ) ),
      first_free_block( 0 ), first_block( 0 ), rightmost( 0 ) {
    for( BlockIntPointer i = first_free_block; i < block_count - 1; ++i ) {
        make_free_blocks_chain( i, i + 1 );
    }

}
bool Btree::find_el( StringIntPointer el, ElementPointer& tree_el, StringIntPointer& hint ) {
    BlockElement* current_block_ptr = get_block_ptr( first_block );

    bool success = false;
    while( !success ) {
        if( !current_block_ptr->find( el, hint ) ) {
            if( current_block_ptr->leaf() ) {
                return false;
            }
            current_block_ptr = current_block_ptr->go_deeper( hint );
        } else {
            success = true;
        }
    }
    return true;
}
ReturnMessage BTree::find( StringIntPointer el, StringIntPointer& found_el ) {
    BlockElement* current_block_ptr = get_block_ptr( first_block );
    ElementPointer tree_el;
    StringIntPointer hint;
    bool success = false;
    if( find_el( el, tree_el, hint ) ) {
        found_el = hint;
        return ReturnMessage::Ok;
    } else {
        return ReturnMessage::NoSuchWord;
    }
}
ReturnMessage Btree::add( StringIntPointer new_el ) {
    if( empty() ) {
        first_block = get_free_block();
    }
    ElementPointer insert_place;
    if( find_el( new_el, insert_place, hint ) ) {
        return ReturnMessage::Exist;
    } else {
        insert( new_el, insert_place );
    }
    BlockElement* current_block_ptr = get_block_ptr( insert_place );
    while( can_split( current_block_ptr ) ) {
        split( current_block_ptr );
        current_block_ptr = current_block_ptr->parent_block_ptr();
    }
    return ReturnMessage::Ok;
}

ReturnMessage Btree::remove( StringIntPointer el ) {
    if( find_el( el, tree_place ) ) {
        remove( tree_place );
    } else {
        return ReturnMessage::NoSuchWord;
    }
    return ReturnMessage::Ok;
}
ReturnMessage Btree::save_to_file( StringIntPointer el ) {
    return ReturnMessage::Ok;
}
ReturnMessage Btree::load_from_file( StringIntPointer el ) {
    return ReturnMessage::Ok;
}
