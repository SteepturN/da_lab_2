#!/usr/bin/env bash

set -o errexit
set -o nounset

log_error() {
    log_ "error" "$@"
}
log_() {
    type=$1
    message=$2
    date_str=; date_str=$(date +'%Y-%m-%d %H:%M:%S')
    echo "[${type}] [${date_str}] ${message}"
}
log_info() {
  log_ "info" "$@"
}
main() {
    log_info "Stage #1 Compiling..."
    if ! make ; then
        log_error "Failed to compile"
        return 1
    fi

    test_dir=tests
    tests_amount=30
    rm -rf ${test_dir}
    mkdir ${test_dir}
    log_info "Stage #2 Test generating..."
    if ! python3 generator.py ${test_dir} ${tests_amount} ; then
        log_error "Failed to generate tests."
        return 1
    fi
    log_info "Stage #3 Checking..."
    for test_file in $( ls ${test_dir}/*.t ) ; do
        tmp_output=tmp
        lab_exe_name=main
        if ! ./${lab_exe_name} < "${test_file}" > "${tmp_output}" ; then
            log_error "Failed to run test ${test_file}"
            return 1
        fi
        file_line_cnt=; file_line_cnt=$(cat ${test_file} | wc -l | sed -e 's/ *//g')
        answer_file=${test_file%.*}.a
        if ! diff -u ${tmp_output} ${answer_file} ; then
            log_error "Failed to check test ${test_file}."
            return 1
        fi
        log_info "${test_file}, lines=${file_line_cnt} OK"
    done
    # echo "Stage #4 Making makedir.tar..."

    # tar -czf makedir.tar *.cpp *.hpp makefile
    # make clean
    # rm ${tmp_output}
    # rm ${lab_exe_name}
    # rm -rf ${test_dir}
}
main "$@"
