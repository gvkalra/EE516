#!/bin/bash

# set bash options
# Reference: https://www.gnu.org/software/bash/manual/html_node/The-Shopt-Builtin.html
#
# dotglob:
#	If set, Bash includes filenames beginning with a ‘.’ in the results of filename expansion.
#	(for hidden files)
#
# nullglob:
#	If set, Bash allows filename patterns which match no files to expand to a null string,
#	rather than themselves. (for empty directories)
shopt -s dotglob nullglob

# For tracking the depth of file
DEPTH=0

# Function: print_tree
# Purpose: To print tree structure of a folder in filesystem
# Arguments
#	($1): Folder to be used as root node of tree
print_tree() {
	cd "$1"

	# for each file
	for FILE in *
	do
		# print "|    " between each depth
		ITER=0
		while [ $ITER != $DEPTH ]
		do
			echo -n "|    "
			ITER=$(($ITER + 1))
		done

		# print file
		echo -n "|____"
		echo $FILE

		# recurse if file is directory
		if [ -d "$FILE" ]; then
			DEPTH=$(($DEPTH + 1))
			print_tree "$FILE"
			cd ..
		fi
	done

	# move to previous depth & process more files
	DEPTH=$(($DEPTH - 1));
}

# For aesthetics
echo "|____"

# Print tree structure rooted at home
print_tree ~

# unset bash options
shopt -u dotglob nullglob

# Return success (0) for $?
exit 0