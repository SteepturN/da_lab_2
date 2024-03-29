#! /usr/bin/env python3

# -*- coding: utf-8 -*-

import sys
import random
import string

def get_random_key():
    _range = random.randint(1, 10 ** 4)
    return "".join( random.choice( string.ascii_letters ) for i in range( _range ) )

if __name__ == "__main__":
    # Проверяем, что передали 1 аргумент.
    if len(sys.argv) != 3:
        print( "Usage: {0} <directory> <count of tests>".format( sys.argv[0] ) )
        sys.exit(1)

    count_of_tests = int( sys.argv[2] )

    actions = [ "+", "?", "-" ]

    for enum in range( count_of_tests ):
        keys = dict()
        test_file_name = "{0}/{1}".format( sys.argv[1], enum + 1 )
        with open( "{0}.t".format( test_file_name ), 'w' ) as output_file, \
             open( "{0}.a".format( test_file_name ), "w" ) as answer_file:

            # Для каждого файла генерируем от 1 до 100 тестов.
            for _ in range( random.randint(1, 10 ** 3) ):
                action = random.choice( actions )
                if action == "+":
                    key = get_random_key()
                    value = random.randint(0, (2 ** 64) - 1)
                    output_file.write("+ {0} {1}\n".format( key, value ))
                    key = key.lower()
                    # Если в нашем словаре уже есть такой ключ, то ответе должно быть
                    # сказано, что он существует, иначе --- успешное добавление.
                    answer = "Exist"
                    if key not in keys:
                        answer = "OK"
                        keys[key] = value
                    answer_file.write( "{0}\n".format( answer ) )

                elif action == "?":
                    search_exist_element = random.choice([True, False])
                    key = random.choice([key for key in keys.keys() ]) \
                        if search_exist_element and len(keys.keys()) > 0 else get_random_key()
                    output_file.write("{0}\n".format(key))
                    key = key.lower()
                    if key in keys:
                        answer = "OK: {0}".format(keys[key])
                    else:
                        answer = "NoSuchWord"
                    answer_file.write("{0}\n".format(answer))
                elif action == "-":
                    search_exist_element = random.choice([True, False])
                    key = random.choice([key for key in keys.keys() ]) \
                        if search_exist_element and len(keys.keys()) > 0 else get_random_key()
                    output_file.write("- {0}\n".format(key))
                    key = key.lower()
                    if key in keys:
                        answer = "OK"
                        keys.pop( key )
                    else:
                        answer = "NoSuchWord"
                    answer_file.write("{0}\n".format(answer))
