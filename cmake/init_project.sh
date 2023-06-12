#!/bin/bash

REPO_PATH=$(pwd)
TEMPLATE_PATH=""
PROJECT_NAME=""

# Generate absolute path from relative path
# $1     : relative filename
# return : absolute path or empty string if relative path invalid.
function abspath()
{
    local path="$1"

    if [ -d "$path" ]; then
        # dir
        (cd "$path"; pwd)
    elif [ -f "$path" ]; then
        # file
        if [[ $path = /* ]]; then
            echo "$path"
        elif [[ $path == */* ]]; then
            echo "$(cd "${path%/*}"; pwd)/${path##*/}"
        else
            echo "$(pwd)/$path"
        fi
    fi
}

# Copy file.
# $1 : input file.
# $2 : output file.
function copy_file
{
    local from="$1"
    local to="$2"

    if [[ -f "$to" ]]; then
        echo "[ ERROR ] File '$to' exists."
        exit 1
    fi
    cp -n "$from" "$to" || { echo "[ ERROR ] Failed copy file '$from' to '$to'."; exit 1; }
    echo "Copied file '$from' to '$to'."
}

function dir_name
{
    local rel_path="$1"

    local abs_path="$(abspath "$rel_path")"
    if [[ -z $abs_path ]]; then
        echo "[ ERROR ] Invalid path '$dir_name'."
        exit 1
    fi
    echo "$(basename "$abs_path")"
}

# Find and replace pattern in file.
# $1 : path to file.
# $2 : search pattern.
# $3 : replace string.
function replace_pattern
{
    local file_path="$1"
    local search="$2"
    local replace="$3"

    if [[ $search != "" && $replace != "" ]]; then
        sed -i "s/$search/$replace/" $file_path ||
            { echo "[ ERROR ] Failed replace pattern '$search' to '$replace' in file '$file_path'."; exit 1; }
        echo "Replaced pattern '$search' to '$replace' in file '$file_path'."
    fi
}

function usage
{
    echo "Usage: $0 OPTIONS..."
    echo "Copy required file to root repository directory and replace required params."
    echo ""
    echo "Mandatory arguments to long options are mandatory for short options too."
    echo "  -r, --repo_path=[PWD]  Path to root repository directory (default 'pwd')."
    echo "  -t, --template_path    Path to cmake template  directory."
    echo "  -p, --proj_name        Project name."
    echo "  -h, --help             Display this help end exit."
}

if [[ $# -eq 0 ]]; then
    echo "Unset required options."
    usage
    exit 1
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        -r|--repo_path)
            REPO_PATH="$2"
            shift # past arument
            shift # past value
            ;;
        -t|--template_path)
            TEMPLATE_PATH="$2"
            shift # past arument
            shift # past value
            ;;
        -p|--proj_name)
            PROJECT_NAME="$2"
            shift # past arument
            shift # past value
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        -*|--*)
            echo "Unknown option $1"
            usage
            exit 1
            ;;
        *)
            echo "Unknown option $1"
            usage
            exit 1
            ;;
    esac
done

echo "[ ERROR ] Script doesn't work. TODO fix it."
exit 1

if [[ $TEMPLATE_PATH == "./" ]]; then
    echo "[ ERROR ] Argument 'template_path' must be subdirectory."
    usage
    exit 1
fi

if [[ -z $TEMPLATE_PATH ]]; then
    echo "[ ERROR ] Argument 'template_path' is required."
    usage
    exit 1
fi
if [[ -z $PROJECT_NAME ]]; then
    echo "[ ERROR ] Argument 'proj_name' is required."
    usage
    exit 1
fi

CMAKE_LISTS_TXT_IN_PATH="$TEMPLATE_PATH/CMakeLists.txt.in"
CMAKE_LISTS_TXT_PATH="$REPO_PATH/CMakeLists.txt"

MAKEFILE_IN_PATH="$TEMPLATE_PATH/Makefile.in"
MAKEFILE_PATH="$REPO_PATH/Makefile"

SOURCES_CMAKE_IN_PATH="$TEMPLATE_PATH/sources.cmake.in"
SOURCES_CMAKE_PATH="$REPO_PATH/sources.cmake"

copy_file "$CMAKE_LISTS_TXT_IN_PATH" "$CMAKE_LISTS_TXT_PATH"
copy_file "$MAKEFILE_IN_PATH" "$MAKEFILE_PATH"
copy_file "$SOURCES_CMAKE_IN_PATH" "$SOURCES_CMAKE_PATH"

COMMON_CMAKE_DIR="$(dir_name "$TEMPLATE_PATH")"

replace_pattern "$CMAKE_LISTS_TXT_PATH" "@project_name@" "$PROJECT_NAME"
replace_pattern "$CMAKE_LISTS_TXT_PATH" "@common_cmake_dir@" "$COMMON_CMAKE_DIR"
replace_pattern "$MAKEFILE_PATH" "@common_cmake_dir@" "$COMMON_CMAKE_DIR"

