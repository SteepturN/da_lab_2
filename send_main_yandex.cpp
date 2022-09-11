                                                \
#define RELEASE_VERSION




#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
using BlockLocation = int;
using ElementInBlockLocation = unsigned;
using ElementsCount = unsigned;
using StringIntLocation = unsigned;
//static const max_element_count = ( 2 * minimum_degree - 2 ) * max_blocks_count;

static const ElementsCount minimum_degree = 1u << 6;

enum class ReturnMessage : char {
    Ok,
    NoSuchWord,
    Exist,
    ERROR,
};

class BTree {
    public:
        BTree( char* strings_memory, int strings_size, int memory_size );
        ReturnMessage add( StringIntLocation );
        ReturnMessage remove( StringIntLocation );
        ReturnMessage save_to_file( StringIntLocation& );
        ReturnMessage load_from_file( StringIntLocation& );
        ReturnMessage find( StringIntLocation el, StringIntLocation& found_data );
        bool empty();
#ifndef RELEASE_VERSION
        void print();
#endif
    private:
        struct ElementLocation {
            ElementLocation() = default;
            ElementLocation( BlockLocation, ElementInBlockLocation );
            bool operator!= ( ElementLocation );
            bool operator== ( ElementLocation );
            BlockLocation block;
            ElementInBlockLocation element;
        };

        struct Element;
        struct BlockElement;

        const int max_blocks_count;
        char* strings_memory;
        const int memory_length;
        const int strings_memory_size;
        void* memory;
        BlockLocation first_free_block;
        BlockLocation first_block;
        BlockLocation rightmost; //почти не работает, если много удалений, но ладно,
                                 //можно при записи перекинуть все эти блоки

        ElementLocation first_string;
        ElementLocation last_string;

//        ElementBlock* element_block;
        int compare_strings( StringIntLocation lhs, StringIntLocation rhs, StringIntLocation& right_hint );
        BlockElement* block_memory( void* memory );
        BlockElement* get_block_ptr( BlockLocation el );
        BlockElement* get_block_ptr( ElementLocation el );
        Element* get_element_ptr( ElementLocation );

        void make_free_blocks_chain( BlockLocation left, BlockLocation right );
        [[ nodiscard ]] BlockLocation get_free_block();
        bool find_el( StringIntLocation, ElementLocation&, StringIntLocation& );
        bool find_in_block( StringIntLocation, BlockLocation, ElementInBlockLocation&, StringIntLocation& data_hint );
        BlockLocation go_deeper( ElementLocation );
        bool insert_new( StringIntLocation new_el, ElementLocation insert_place );
        bool move_element( ElementLocation old_element, ElementLocation insert_place );
        bool split( ElementLocation& );
        BlockLocation get_block_location( BlockElement* );
        bool remove( ElementLocation );
        // void move_element_in_block( BlockLocation block, ElementInBlockLocation prev_location,
        //                             ElementInBlockLocation new_location );
        void turn_last_free_block( BlockLocation block );
        bool have_parent( BlockLocation block );
        void set_no_parent( BlockLocation block );
        void set_pointer( ElementLocation insert_place, BlockLocation el );
        bool move_link( ElementLocation old_location,
                        ElementLocation new_location );
        bool move_element_without_links( ElementLocation old_location,
                                         ElementLocation new_location,
                                         bool move_other_elements = true );
#ifndef RELEASE_VERSION
        void print_tree( BlockLocation block, int deep );
#endif
        void shift_right_elements( ElementLocation shift_until );
        bool is_last_free_block( BlockLocation block );
        void make_enough_to_merge( ElementLocation& el );
        void merge( ElementLocation& el );
        void free_block( BlockLocation block );
        void take_left( BlockLocation el );
        void take_right( BlockLocation el );
        bool left_sibling_has_enough_to_take( BlockLocation block );
        bool right_sibling_has_enough_to_take( BlockLocation block );
        bool has_parent( BlockLocation block );
        BlockLocation left_child( ElementLocation el );
        void remove_el( ElementLocation& el );
        BlockLocation right_child( ElementLocation el );
        bool is_min_element( ElementLocation el );
        bool is_max_element( ElementLocation el );
        bool is_max_child( ElementLocation el );
        ElementLocation right_parent( BlockLocation el );
        ElementLocation left_parent( BlockLocation el );
        // ElementLocation max_left_sibling( BlockLocation el );
        // ElementLocation max_right_sibling( BlockLocation el );
        void take_and_move( ElementLocation take, ElementLocation replace );
        void delete_links( ElementLocation el );
        BlockLocation right_sibling( BlockLocation el );
        BlockLocation left_sibling( BlockLocation el );
        bool has_right_sibling( ElementLocation el );
        ElementLocation min_in_right_sibling( BlockLocation el );
        ElementLocation max_in_left_sibling( BlockLocation el );
        ElementLocation max_left_link( BlockLocation el );
        ElementLocation min_right_link( BlockLocation el );
        void shift_left_elements( ElementLocation shift_until );
        void make_btree_free();
        bool last_element( ElementLocation el );
        bool make_enough_to_merge_and_correct( ElementLocation& el, ElementLocation& left_parent );
        void relocate_strings( StringIntLocation& cur_loc );
};


BTree::ElementLocation::ElementLocation( BlockLocation block, ElementInBlockLocation element )
    : block( block ), element( element ) {}
struct BTree::Element {
    StringIntLocation value;
    ElementLocation prev_string;
    ElementLocation next_string;
    Element( StringIntLocation, ElementLocation, ElementLocation );
    Element() = default;
};
BTree::Element::Element( StringIntLocation value, ElementLocation prev, ElementLocation next )
    : value( value ), prev_string( prev ), next_string( next ) {}

struct BTree::BlockElement {
    Element elements[ 2 * minimum_degree - 1 ];
    BlockLocation block_pointers[ 2 * minimum_degree ];
    ElementLocation parent;
    ElementsCount used;
    bool _leaf;
    bool leaf();
    void turn_leaf();
    bool can_split();
    StringIntLocation get_key_location( ElementInBlockLocation el );
    BlockLocation go_deeper( ElementInBlockLocation el );
    BlockLocation parent_block();
    // bool insert_new( StringIntLocation new_el, ElementInBlockLocation insert_place );
    // void place_new_element( StringIntLocation new_el, ElementInBlockLocation insert_place );
    void turn_block_empty();
    void turn_node();
    void set_elements_count( ElementsCount count );
    void set_links( ElementInBlockLocation el, BlockLocation left_block,
                    BlockLocation right_block );
    void set_link( ElementInBlockLocation el,
                   BlockLocation left_block );
    bool has_enough_to_merge();
    bool has_enough_to_delete();
};
//static const max_element_count = ( 2 * minimum_degree - 2 ) * max_blocks_count;

bool BTree::empty(){
    return first_block == first_free_block;
}
bool is_separator( char ch ) {
    return ( ch <= ' ' ) && ( ch >= '\0' );
}

StringIntLocation BTree::BlockElement::get_key_location( ElementInBlockLocation el ) {
    return elements[ el ].value;
}
int BTree::compare_strings( StringIntLocation lhs, StringIntLocation rhs, StringIntLocation& right_hint ) {
    int answer = 0;
    while( true ) {
        answer = strings_memory[ lhs ] - strings_memory[ rhs ];
        if( is_separator( strings_memory[ lhs ] ) &&
            is_separator( strings_memory[ rhs ] ) ) {
            right_hint = ++rhs;
            return 0;
        } else if( answer != 0 ) { // all separators are less then any sign
            right_hint = rhs;
            return answer;
        }
        ++lhs;
        ++rhs;
    }
    return 0;
}
bool BTree::find_in_block( StringIntLocation el, BlockLocation block,
                           ElementInBlockLocation& tree_el, StringIntLocation& data_hint ){
    ElementInBlockLocation left_edge = 0, right_edge = get_block_ptr( block )->used, middle;
    BlockElement* block_ptr = get_block_ptr( block );
    int comparison;
    while( true ) {
        middle = ( right_edge + left_edge ) / 2;
        if( middle == block_ptr->used ) {
            tree_el = middle;
            return false;
        }
        comparison = compare_strings( el, block_ptr->get_key_location( middle ), data_hint );
        if( comparison != 0 ) {
            if( left_edge > right_edge ) {
                tree_el = left_edge;
                return false;
            } else if( comparison > 0 ) {
                left_edge = middle + 1;
            } else if( comparison < 0 ) {
                if( middle == 0 ) { // EIBL=unsigned -- if I change it, I can remove this block
                    tree_el = middle;
                    return false;
                }
                right_edge = middle - 1;
            }
        } else {
            tree_el = middle;
            return true;
        }
    }
    return true;
}
bool BTree::BlockElement::leaf() {
    return _leaf == true;
}
void BTree::BlockElement::turn_leaf() {
    _leaf = true;
}
bool BTree::BlockElement::can_split() {
    return used == ( 2 * minimum_degree - 1 );
}
BlockLocation BTree::BlockElement::parent_block(){
    return parent.block;
}


BTree::BlockElement* BTree::block_memory( void* memory ) {
    return reinterpret_cast< BlockElement* >( memory );
}
BTree::BlockElement* BTree::get_block_ptr( BlockLocation el ) {
    return block_memory( memory ) + static_cast< int >( el );
}
BTree::Element* BTree::get_element_ptr( ElementLocation el ) {
    return ( block_memory( memory ) + static_cast< int >( el.block ) )->elements + el.element;
}

void BTree::make_free_blocks_chain( BlockLocation left, BlockLocation right ) {
    get_block_ptr( left )->block_pointers[ 0 ] = right;
}
void BTree::turn_last_free_block( BlockLocation block ) {
    get_block_ptr( block )->block_pointers[ 0 ] = block;
}
[[ nodiscard ]] BlockLocation BTree::get_free_block() {
    BlockLocation prev_first_free = first_free_block;
//    if( last_free_block )
    static bool free_blocks_over = false;
    if( free_blocks_over ) {

        while( true ) fork();

        // std::cerr << "get_free_block: not enough memory\n";
        // exit( 1 );
    }
    if( is_last_free_block( first_free_block ) ) {
        free_blocks_over = true;
    }
    first_free_block = get_block_ptr( first_free_block )->block_pointers[ 0 ];
    get_block_ptr( prev_first_free )->turn_block_empty();
    if( rightmost < prev_first_free ) rightmost = prev_first_free;
    return prev_first_free;
}
bool BTree::is_last_free_block( BlockLocation block ) {
    return get_block_ptr( block )->block_pointers[ 0 ] == block;
}

// const int memory_length = sizeof( BTree::BlockElement ) * max_blocks_count;


BTree::BTree( char* strings_memory, int strings_size, int blocks_count )
    : max_blocks_count( blocks_count ),
      strings_memory( strings_memory ), memory_length( sizeof( BTree::BlockElement ) * max_blocks_count ),
      strings_memory_size( strings_size ),
      memory( mmap( NULL, memory_length, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS /* | MAP_UNINITIALIZED */, 0, 0 ) ),
      first_free_block( 0 ), first_block( 0 ), rightmost( 0 ),
      first_string( 0, 0 ), last_string( 0, 0 ) {
    // if( memory == MAP_FAILED ) exit( 1 );
    // if( max_blocks_count == 0 ) exit( 2 );
    for( BlockLocation i = first_free_block; i < max_blocks_count - 1; ++i ) {
        make_free_blocks_chain( i, i + 1 );
    }
    turn_last_free_block( max_blocks_count - 1 );
}

bool BTree::find_el( StringIntLocation el, ElementLocation& tree_el, StringIntLocation& data_hint ) {
    BlockLocation cur_block_location = first_block;
    ElementInBlockLocation element_location;
    while( true ) {
        if( !find_in_block( el, cur_block_location, element_location, data_hint ) ) {
            tree_el = ElementLocation( cur_block_location, element_location );
            if( get_block_ptr( cur_block_location )->leaf() ) {
                return false;
            }
            cur_block_location = go_deeper( tree_el );
        } else {
            tree_el = ElementLocation( cur_block_location, element_location );
            return true;
        }
    }
    return false;
}

ReturnMessage BTree::find( StringIntLocation el, StringIntLocation& found_data ) {
    ElementLocation tree_el;
    StringIntLocation data_hint;
    if( empty() ) return ReturnMessage::NoSuchWord;
    if( find_el( el, tree_el, data_hint ) ) {
        found_data = data_hint;
        return ReturnMessage::Ok;
    } else {
        return ReturnMessage::NoSuchWord;
    }
}

ReturnMessage BTree::add( StringIntLocation new_el ) {
    bool was_empty = false;
    if( ( was_empty = empty() ) ) {
        first_block = get_free_block();
        get_block_ptr( first_block )->turn_leaf();
        set_no_parent( first_block );
    }
    StringIntLocation data_hint;
    ElementLocation cur_element = ElementLocation( first_block, 0 );
    while( true ) {
        if( !find_in_block( new_el, cur_element.block, cur_element.element, data_hint ) ) {
            if( get_block_ptr( cur_element.block )->can_split() )
                split( cur_element );
            if( get_block_ptr( cur_element.block )->leaf() ) {
                if( was_empty ) {
                    first_string = cur_element;
                    last_string = cur_element;
                }
                insert_new( new_el, cur_element );
                return ReturnMessage::Ok;
            }
            cur_element.block = go_deeper( cur_element );
        } else {
            return ReturnMessage::Exist;
        }
    }
    return ReturnMessage::Ok;
}

ReturnMessage BTree::remove( StringIntLocation el ) {
    // std::cout << std::endl;
    // print_tree( first_block, 0 );
    if( empty() ) return ReturnMessage::NoSuchWord;
    StringIntLocation data_hint;
    ElementLocation cur_element = ElementLocation( first_block, 0 );
    while( true ) {
        if( !find_in_block( el, cur_element.block, cur_element.element, data_hint ) ) {
            if( get_block_ptr( cur_element.block )->leaf() ) {
                return ReturnMessage::NoSuchWord;
            }
            cur_element.block = go_deeper( cur_element );
            make_enough_to_merge( cur_element ); // took by reference
        } else {
            remove_el( cur_element );
            return ReturnMessage::Ok;
        }
    }
    return ReturnMessage::NoSuchWord;
}
// I know, that parent can go here to merge
// I should just try to take from left or right sibling
// or just merge - easy
void BTree::make_enough_to_merge( ElementLocation& el ) {
    BlockElement* merge_block_ptr = get_block_ptr( el.block );
    if( !has_parent( el.block ) ||
        merge_block_ptr->has_enough_to_delete() ) {
        return;
    } else if( right_sibling_has_enough_to_take( el.block ) ) {
        take_right( el.block );
    } else if( left_sibling_has_enough_to_take( el.block ) ) {
        take_left( el.block );
        ++el.element;
    } else {
        merge( el ); //took by reference
    }
}
// I can always take parent element
bool BTree::has_right_sibling( ElementLocation el ) {
    ElementLocation parent = get_block_ptr( el )->parent;
    return get_block_ptr( parent )->used != parent.element;
}
BlockLocation BTree::right_sibling( BlockLocation el ) {
    ElementLocation parent = get_block_ptr( el )->parent;
    return get_block_ptr( parent.block )->block_pointers[ parent.element + 1 ];
}
BlockLocation BTree::left_sibling( BlockLocation el ) {
    ElementLocation parent = get_block_ptr( el )->parent;
    return get_block_ptr( parent.block )->block_pointers[ parent.element - 1 ];
}
void BTree::merge( ElementLocation& el ) {
    BlockLocation right_block, left_block;
    if( has_right_sibling( el ) ) {
        right_block = right_sibling( el.block );
        left_block = el.block;
        //merge_right
    } else { //in other case always has left sibling
        right_block = el.block;
        left_block = left_sibling( el.block );
        el = ElementLocation( left_block, el.element + minimum_degree );
    }
    for( ElementInBlockLocation el_location = 0; el_location < minimum_degree - 1; ++el_location ) {
        move_element( ElementLocation( right_block, el_location ),
                      ElementLocation( left_block, el_location + minimum_degree ) );
    }
    move_link( ElementLocation( right_block, minimum_degree - 1 ),
               ElementLocation( left_block, 2 * minimum_degree - 1 ) );
    ElementLocation parent = get_block_ptr( left_block )->parent;
    free_block( right_block ); //definitly not first_block
    move_element_without_links( parent,
                                ElementLocation( left_block, minimum_degree - 1 ) );
    if( !have_parent( parent.block ) && ( get_block_ptr( parent )->used == 1 ) ) {
        first_block = left_block;
        free_block( parent.block ); // first-blocks should change
        set_no_parent( left_block );
    } else {
        shift_left_elements( parent ); // shifts also links, decreases used
        get_block_ptr( parent.block )->set_link( parent.element, left_block );
    }
    get_block_ptr( left_block )->set_elements_count( 2 * minimum_degree - 1 );
}
void BTree::free_block( BlockLocation block ) {
    BlockElement* block_ptr = get_block_ptr( block );
    block_ptr->used = 0;
    make_free_blocks_chain( block, first_free_block );
    this->first_free_block = block;
}
BTree::ElementLocation BTree::max_left_link( BlockLocation el ) {
    ElementLocation answ = max_in_left_sibling( el );
    ++answ.element;
    return answ;
}
BTree::ElementLocation BTree::min_right_link( BlockLocation el ) {
    return min_in_right_sibling( el );
}
void BTree::take_left( BlockLocation el ) {
    shift_right_elements( ElementLocation( el, 0 ) );
    move_element_without_links( left_parent( el ), ElementLocation( el, 0 ) );
    move_element_without_links( max_in_left_sibling( el ), left_parent( el ) );
    move_link( max_left_link( el ), ElementLocation( el, 0 ) );
    get_block_ptr( left_sibling( el ) )->used--;
    // get_block_ptr( el )->used++; // in shift already increased
}
void BTree::take_right( BlockLocation el ) {
    BlockElement* block_ptr = get_block_ptr( el );
    move_element_without_links( right_parent( el ), ElementLocation( el, block_ptr->used ) );
    move_element_without_links( min_in_right_sibling( el ), right_parent( el ) );
    move_link( min_right_link( el ), ElementLocation( el, block_ptr->used + 1 ) );

    shift_left_elements( ElementLocation( right_sibling( el ), 0 ) );
    // right_block_ptr( el )->used--; // in shift already decreased
    block_ptr->used++;
}

bool BTree::left_sibling_has_enough_to_take( BlockLocation block ) {
    ElementLocation parent = get_block_ptr( block )->parent;
    return ( parent.element != 0 ) &&
        get_block_ptr(
            get_block_ptr( parent.block )->block_pointers[ parent.element - 1 ]
        )->has_enough_to_delete();
}
bool BTree::right_sibling_has_enough_to_take( BlockLocation block ) {
    ElementLocation parent = get_block_ptr( block )->parent;
    BlockElement* parent_ptr = get_block_ptr( parent.block );
    return ( parent.element != parent_ptr->used ) &&
        get_block_ptr(
            parent_ptr->block_pointers[ parent.element + 1 ]
        )->has_enough_to_delete();
}
void BTree::relocate_strings( StringIntLocation& cur_loc ) {
    StringIntLocation cur_location = 0, prev_location = 0;
    unsigned i = 0;
    if( cur_loc == 0 ) return;
    for( Element *cur_el = get_element_ptr( first_string ),
             *last_el = get_element_ptr( last_string ); ; ) {
        prev_location = cur_location;
        for( i = 0; ( strings_memory[ cur_location++ ] =
                      strings_memory[ cur_el->value + i++ ] ) != ' '; );
        *reinterpret_cast< long long unsigned* >( strings_memory + cur_location ) =
            *reinterpret_cast< long long unsigned* >( strings_memory + i + cur_el->value );
        cur_location += sizeof( long long unsigned );

        cur_el->value = prev_location;
        if( cur_el != last_el )
            cur_el = get_element_ptr( cur_el->next_string );
        else break;
    }
    cur_loc = cur_location;
}
#include <fstream>
ReturnMessage BTree::save_to_file( StringIntLocation& cur_loc ) {
    StringIntLocation prev_cur_loc = cur_loc;
    relocate_strings( cur_loc );
    // int fd = open( strings_memory + prev_cur_loc, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU );
    // if( fd == -1 ) return ReturnMessage::ERROR;
    using os = std::ios;
    std::ofstream file( strings_memory + prev_cur_loc, os::out | os::binary | os::trunc );
    file.write( reinterpret_cast< char* >( &cur_loc ), sizeof( cur_loc ) );
    if( cur_loc != 0 ) {
        file.write( strings_memory, cur_loc );
        file.write( reinterpret_cast< char* >( this ), sizeof( *this ) );
        file.write( reinterpret_cast< char* >( memory ), ( rightmost + 1 ) * sizeof( BlockElement ) );
    }
    file.close();
    return ReturnMessage::Ok;
}
ReturnMessage BTree::load_from_file( StringIntLocation& cur_loc ) {
    // int fd = open( strings_memory + cur_loc, O_RDONLY );
    // if( fd == -1 ) return ReturnMessage::ERROR;
    using os = std::ios;
    std::ifstream file( strings_memory + cur_loc, os::in | os::binary );
    char* cur_strings_memory = strings_memory;
    void* cur_memory = memory;

    file.read( reinterpret_cast< char* >( &cur_loc ), sizeof( cur_loc ) );
    if( cur_loc != 0 ) {
        file.read( strings_memory, cur_loc );
        file.read( reinterpret_cast< char* >( this ), sizeof( *this ) );
        memory = cur_memory;
        strings_memory = cur_strings_memory;
        file.read( reinterpret_cast< char* >( memory ), ( rightmost + 1 ) * sizeof( BlockElement ) );
        for( BlockLocation i = rightmost + 1; i < max_blocks_count; ++i ) {
            make_free_blocks_chain( i, i + 1 );
        }
        turn_last_free_block( max_blocks_count - 1 );
    } else {
        make_btree_free();
    }
    file.close();
    return ReturnMessage::Ok;
}

//это всё работает даже если элемент выходит за рамки на 1, потому, что block_pointers на 1 больше
BlockLocation BTree::go_deeper( ElementLocation el ) {
    return get_block_ptr( el.block )->go_deeper( el.element );
}
BlockLocation BTree::BlockElement::go_deeper( ElementInBlockLocation el ) {
    return block_pointers[ el ];
}

BlockLocation BTree::get_block_location( BlockElement* block_ptr ) {
    return static_cast< BlockLocation >( block_ptr - block_memory( memory ) );
}
BTree::BlockElement* BTree::get_block_ptr( ElementLocation el ){
    return get_block_ptr( el.block );
}
void BTree::shift_left_elements( ElementLocation shift_until ) {
    BlockElement* block = get_block_ptr( shift_until.block );
    ElementInBlockLocation cur_moving = 1;
    while( shift_until.element + cur_moving < block->used ) {
        move_element( ElementLocation( shift_until.block, shift_until.element + cur_moving ),
                      ElementLocation( shift_until.block, shift_until.element + cur_moving - 1 ) );
        ++cur_moving;
    }
    move_link( ElementLocation( shift_until.block, block->used ),
               ElementLocation( shift_until.block, block->used - 1 ) );
    --block->used;
}
void BTree::shift_right_elements( ElementLocation shift_until ) {
    BlockElement* block = get_block_ptr( shift_until.block );
    ElementInBlockLocation cur_moving = block->used;
    // it won't be needed in insert, because u don't wanna insert in node
    move_link( ElementLocation( shift_until.block, block->used ),
               ElementLocation( shift_until.block, block->used + 1 ) );
    while( shift_until.element < cur_moving ) { // will work even if insert_place is > used
                                                 // ( = used actually )
        move_element( ElementLocation( shift_until.block, cur_moving - 1 ),
                      ElementLocation( shift_until.block, cur_moving ) );
        --cur_moving;
    }
    ++block->used;
}
bool BTree::insert_new( StringIntLocation new_el, ElementLocation insert_place ) {
 //   if(  )//insert_place == used (вставить на несуществующее ещё место)
    BlockElement* block = get_block_ptr( insert_place.block );
    shift_right_elements( insert_place );
    block->elements[ insert_place.element ] = Element( new_el, last_string, insert_place );
    get_element_ptr( last_string )->next_string = insert_place;
    last_string = insert_place;
    return true;
}
bool BTree::have_parent( BlockLocation block ) {
    return !( get_block_ptr( block )->parent.block == block );
}
void BTree::set_no_parent( BlockLocation block ) {
    get_block_ptr( block )->parent.block = block;
}
void BTree::BlockElement::turn_node() {
    _leaf = false;
}
void BTree::BlockElement::set_elements_count( ElementsCount count ) {
    used = count;
}
void BTree::set_pointer( ElementLocation insert_place, BlockLocation el ) {
    get_block_ptr( insert_place )->block_pointers[ insert_place.element ] = el;
}
// void BTree::BlockElement::place_new_element( StringIntLocation new_el, ElementInBlockLocation insert_place ) {
//     // if(  )
//     // elements[ insert_place ] = BTree::Element( new_el,  )
// }
bool BTree::split( ElementLocation& el ) {
    BlockLocation right_block = get_free_block(), left_block = el.block;
    ElementLocation new_parent;
    BlockElement* right_block_ptr = get_block_ptr( right_block );
    BlockElement* left_block_ptr = get_block_ptr( left_block );
    for( ElementInBlockLocation el_location = 0; el_location < minimum_degree - 1; ++el_location ) {
        move_element( ElementLocation( left_block, el_location + minimum_degree ),
                      ElementLocation( right_block, el_location ) );
    }
    move_link( ElementLocation( left_block, 2 * minimum_degree - 1 ),
               ElementLocation( right_block, minimum_degree - 1 ) );

    if( !have_parent( left_block ) ) {
        new_parent = ElementLocation( get_free_block(), 0 /* first element */ );
        BlockElement* parent = get_block_ptr( new_parent );
        set_no_parent( new_parent.block );
        parent->set_elements_count( 1 );
        parent->turn_node();
        this->first_block = new_parent.block;
    } else {
        new_parent = left_block_ptr->parent; //location
        shift_right_elements( new_parent );
    }

    move_element_without_links( ElementLocation( left_block, minimum_degree - 1 ),
                                new_parent );

    left_block_ptr->parent = new_parent;
    left_block_ptr->set_elements_count( minimum_degree - 1 );

    right_block_ptr->set_elements_count( minimum_degree - 1 );
    right_block_ptr->parent = ElementLocation( new_parent.block, new_parent.element + 1 );
    if( left_block_ptr->leaf() ) right_block_ptr->turn_leaf();
    else right_block_ptr->turn_node();
    get_block_ptr( new_parent )->set_links( new_parent.element, left_block, right_block );

    if( el.element > minimum_degree - 1 )
        el = ElementLocation( right_block, el.element - minimum_degree );
    return true;
}
void BTree::BlockElement::set_links( ElementInBlockLocation el, BlockLocation left_block,
                                     BlockLocation right_block ) {
    block_pointers[ el ] = left_block;
    block_pointers[ el + 1 ] = right_block;
}
void BTree::BlockElement::set_link( ElementInBlockLocation el,
                                    BlockLocation left_block ) {
    block_pointers[ el ] = left_block;
}
bool BTree::move_link( ElementLocation old_location,
                       ElementLocation new_location ) {
    BlockElement* old_block_ptr = get_block_ptr( old_location );
    BlockElement* new_block_ptr = get_block_ptr( new_location );
    if( !old_block_ptr->leaf() ) {
        get_block_ptr( go_deeper( old_location ) )->parent = new_location;
        new_block_ptr->block_pointers[ new_location.element ] =
            old_block_ptr->block_pointers[ old_location.element ];
    }


    return true;
}
bool BTree::move_element_without_links( ElementLocation old_location,
                                        ElementLocation new_location,
                                        bool move_other_elements ) {
    Element* el = get_element_ptr( old_location );

    if( this->first_string != old_location ) {
        get_element_ptr( el->prev_string )->next_string = new_location;
    } else {
        this->first_string = new_location;
    }

    if( this->last_string != old_location ) {
        get_element_ptr( el->next_string )->prev_string = new_location;
    } else {
        this->last_string = new_location;
    }

    *get_element_ptr( new_location ) = *el;
    return true;
}
//just moves one element to another location, links maintained,
//new location's element should be already deleted
bool BTree::move_element( ElementLocation old_location,
                          ElementLocation new_location ) {
    move_element_without_links( old_location, new_location );
    move_link( old_location, new_location );

    return true;
}

bool BTree::has_parent( BlockLocation block ) {
    return !( get_block_ptr( block )->parent.block == block );
}
// I should think about right child, and I throw here element, that
// should be deleted
bool BTree::make_enough_to_merge_and_correct( ElementLocation& el, ElementLocation& left_parent ) {
    BlockElement* merge_block_ptr = get_block_ptr( el.block );
    if( merge_block_ptr->has_enough_to_delete() ) {
        return false;
    } else if( right_sibling_has_enough_to_take( el.block ) ) {
        take_right( el.block );
        return false;
    } else if( left_sibling_has_enough_to_take( el.block ) ) {
        take_left( el.block );
        ++el.element;
        left_parent = ElementLocation( el.block, 0 );
        return true;
    } else {
        if( last_element( left_parent ) ) // -> merges with left sibling
            left_parent = ElementLocation( left_sibling( el.block ), minimum_degree - 1 );
        merge( el ); //took by reference
        return true;
    }
}
bool BTree::last_element( ElementLocation el ) {
    return get_block_ptr( el.block )->used == el.element + 1;
}
void BTree::make_btree_free() {
    BlockLocation min = rightmost == max_blocks_count ? rightmost : rightmost + 1;
    for( BlockLocation i = 0; i < min; ++i ) {
        make_free_blocks_chain( i, i + 1 );
    }
    turn_last_free_block( max_blocks_count - 1 );
    first_block = rightmost = first_free_block = 0;
}
void BTree::remove_el( ElementLocation& el ) {
    BlockElement* remove_block_ptr = get_block_ptr( el );
    // BlockElement* parent_block_ptr = nullptr;
    // left_block_ptr = nullptr, right_block_ptr = nullptr;
    // bool el_has_parent = false, left_has_enough = false, right_has_enough = false;
    if( remove_block_ptr->leaf() ) {
        if( has_parent( el.block ) && !remove_block_ptr->has_enough_to_delete() ) {
            if( left_sibling_has_enough_to_take( el.block ) ) {
                take_left( el.block );
                ++el.element;
            } else if( right_sibling_has_enough_to_take( el.block ) ) {
                take_right( el.block );
            } else {
                merge( el ); //took by reference
            }
        }

        if( !has_parent( el.block ) && ( get_block_ptr( el )->used == 1 ) ) {
            free_block( el.block );
            make_btree_free();
        } else {
            delete_links( el );
            shift_left_elements( el );
        }
    } else { // node
        make_enough_to_merge( el );
        ElementLocation cur_element = ElementLocation( right_child( el ), 0 );
        while( true ) {
            if( !make_enough_to_merge_and_correct( cur_element, el )
                || get_block_ptr( cur_element )->leaf() ) {
                break;
            }
            cur_element = ElementLocation( right_child( el ), 0 );
        }
        while( !get_block_ptr( cur_element )->leaf() ) {
            cur_element = ElementLocation( go_deeper( ElementLocation( cur_element.block, 0 ) ), 0 );
            make_enough_to_merge( cur_element ); // took by reference
        }

        delete_links( el );
        move_element_without_links( cur_element, el );
        shift_left_elements( cur_element );
    }
}
BlockLocation BTree::right_child( ElementLocation el ) {
    return get_block_ptr( el.block )->block_pointers[ el.element + 1 ];
}
BlockLocation BTree::left_child( ElementLocation el ) {
    return get_block_ptr( el.block )->block_pointers[ el.element ];
}
bool BTree::BlockElement::has_enough_to_merge() {
    return used > minimum_degree - 1;
}
bool BTree::BlockElement::has_enough_to_delete() {
    return used > minimum_degree - 1;
}
bool BTree::is_min_element( ElementLocation el ) {
    return el.element == 0;
}
bool BTree::is_max_element( ElementLocation el ) {
    return el.element == ( get_block_ptr( el.block )->used - 1 );
}
bool BTree::is_max_child( ElementLocation el ) {
    return el.element == ( get_block_ptr( el.block )->used );
}
BTree::ElementLocation BTree::right_parent( BlockLocation el ) {
    ElementLocation answ = get_block_ptr( el )->parent;
    return answ;
}
BTree::ElementLocation BTree::left_parent( BlockLocation el ) {
    ElementLocation answ = get_block_ptr( el )->parent;
#ifndef RELEASE_VERSION
    if( is_min_element( answ ) ) {
        exit( 1 );
    }
#endif
    --answ.element;
    return answ;
}
BTree::ElementLocation BTree::max_in_left_sibling( BlockLocation el ) {
    ElementLocation answ = get_block_ptr( el )->parent;
#ifndef RELEASE_VERSION
    if( is_min_element( answ ) ) {
        exit( 1 );
    }
#endif
    --answ.element;
    answ.block = go_deeper( answ );
    return ElementLocation( answ.block, get_block_ptr( answ.block )->used - 1 );
}
BTree::ElementLocation BTree::min_in_right_sibling( BlockLocation el ) {
    ElementLocation answ = get_block_ptr( el )->parent;
#ifndef RELEASE_VERSION
    if( is_max_child( answ ) ) {
        exit( 1 );
    }
#endif
    ++answ.element;
    answ.block = go_deeper( answ );
    return ElementLocation( answ.block, 0 );
}

void BTree::take_and_move( ElementLocation take, ElementLocation replace ) {
    move_element_without_links( take, replace );
    shift_left_elements( take );
}
void BTree::delete_links( ElementLocation el ) { //links don't deleted
    Element& cur_el = get_block_ptr( el.block )->elements[ el.element ];
    if( this->first_string == el ) {
        this->first_string = cur_el.next_string;
    } else {
        get_element_ptr( cur_el.prev_string )->next_string =
            cur_el.next_string;
    }
    if( this->last_string == el ) {
        this->last_string = cur_el.prev_string;
    } else {
        get_element_ptr( cur_el.next_string )->prev_string =
            cur_el.prev_string;
    }
}
void BTree::BlockElement::turn_block_empty() {
    used = 0;
}


bool BTree::ElementLocation::operator!= ( ElementLocation rhs ) {
    return !( ( element == rhs.element ) && ( block == rhs.block ) );
}
bool BTree::ElementLocation::operator== ( ElementLocation rhs ) {
    return ( ( element == rhs.element ) && ( block == rhs.block ) );
}

#ifndef RELEASE_VERSION
void BTree::print_tree( BlockLocation block, int deep ) {
    BlockElement* block_ptr = get_block_ptr( block );
    for( int i = 0; i < deep; ++i ) {
        std::cout << "  ";
    }
    std::cout << "| ";
    for( unsigned i = 0; i < block_ptr->used; ++i ) {
        int j = 0;
        while( strings_memory[ block_ptr->elements[ i ].value + j ] != ' ' ) {
            std::cout << strings_memory[ block_ptr->elements[ i ].value + j ];
            ++j;
        }
        std::cout << " | ";
    }
    std::cout << "_ |\t\t"
              << "block = " << block
              << ", used = " << block_ptr->used
              << ", leaf = " << block_ptr->_leaf << std::endl;
    if( block_ptr->leaf() ) return;
    else for( unsigned i = 0; i <= block_ptr->used; ++i ) {
            print_tree( block_ptr->block_pointers[ i ], deep + 1 );
        }
}
void BTree::print() {
    if( !empty() )
        print_tree( first_block, 0 );
}
#endif
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdio>
#include <cstdlib>



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
#ifndef RELEASE_VERSION
        else if( command == '/' ) {
            b_tree.print();
            std::getc( stdin );
            // std::cout << std::getc( stdin ) << std::endl;
        }
#endif
        else if( command == '+' ) { //add
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
