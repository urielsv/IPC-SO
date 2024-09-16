#!/bin/bash

# Check if the number of files is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <number_of_files> <directory>"
    exit 1
fi

# Read command-line arguments
num_files=$1
directory=$2

# Check if the directory exists
if [ ! -d "$directory" ]; then
    echo "Error: Directory $directory does not exist."
    exit 1
fi

# Generate random files
for (( i=1; i<=num_files; i++ )); do
    # Generate a random file name
    file_name=$(mktemp -u "${directory}/file_XXXXXX.txt")

    # Generate random content with different lengths
    # Using /dev/urandom to create random data
    # You can adjust the size by changing `bs=4096` to a different value if needed
    head -c $((RANDOM % 10240 + 1024)) /dev/urandom > "$file_name"

    echo "Created file: $file_name"
done
