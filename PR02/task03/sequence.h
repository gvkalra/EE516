#pragma once

#include <linux/seq_file.h>

/* return sequence operations */
inline struct seq_operations *
get_sequence_ops(void);