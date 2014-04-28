#ifndef _H_LOG_H_
#define _H_LOG_H_

#include <synch.h>
#include <limits.h>

#define LOG_BUFFER_SIZE 4096
#define DISK_LOG_SIZE 524288
#define MARGIN 52430

// TODO possibly have two of these?
struct log_buffer{
	struct lock *lock; // lock to ensure mutual exclusion during flushing
	unsigned buffer_filled; // bytes of the buffer filled
	char buffer [LOG_BUFFER_SIZE]; // bytes array
};

struct log_info{
	struct lock *lock; // lock to ensure up to data meta data
	struct log_buffer *active_buffer; // the active buffer of the two that we switch between
	// These are only updated before flushing to disk
	unsigned head; // byte index of the head
	unsigned tail; // byte index of the tail
	// These are constantly updated in memory
	unsigned len; // len in bytes of the on disk + in memory log
	uint16_t page_count; // pages stored in memory without checkpointing
	uint64_t last_id; // id of last entry written
	unsigned earliest_transaction; // index in bytes of first entry of uncommitted transaction farthest in the past
}log_info;

struct stored_info{
	unsigned head; // byte index of the head
	unsigned tail; // byte index of the tail
	unsigned len; // len in bytes of the on disk + in memory log
	uint64_t last_id; // id of last entry written
};

// Macro to define where meta data ends and log begins
//#define LOG_START (sizeof(struct log_info));

enum operation{
	CHECKPOINT = 1,
	ABORT,
	COMMIT,
	ADD_DIRENTRY,
	MODIFY_DIRENTRY_SIZE,
	MODIFY_DIRENTRY,
	RENAME_DIRENTRY,
	REMOVE_DIRENTRY,
	ALLOC_INODE,
	FREE_INODE,
	// TODO removed truncate as we can log a bunch of free's instead of creating a completely new structure.
};

enum object_type{
	FILE = 1,
	DIR,
    INDIRECTION,
    USERBLOCK
};

struct record_header{
	uint64_t record_id; // Size of the structure after this. Not including the header
	uint16_t size;
	uint16_t op;    /* for partability; enums are just ints */
};

struct checkpoint{
	// offset in bytes into the array where the new tail is
	unsigned new_tail;
};

struct commit{
	// unique for each transaction, based on the id of the first action in the transaction
	uint64_t transaction_id;
};

struct add_direntry{
	uint64_t transaction_id;
	unsigned dir_inode_id;
	unsigned target_inode_id;
	unsigned old_link_count;
	unsigned new_link_count;
	char name[NAME_MAX];
};

struct modify_direntry_size{
	uint64_t transaction_id;
	unsigned inode_id;
	uint32_t old_len;
	uint32_t new_len;
};

struct modify_direntry{
	uint64_t transaction_id;
	// TODO don't I need 2 inode id's?
	unsigned inode_id;
	unsigned new_inode_value;
	unsigned old_inode_value;
	unsigned inode1_old_link_count;
	unsigned inode1_new_link_count;
	unsigned inode2_old_link_count;
	unsigned inode2_new_link_count;
	char name[NAME_MAX];
};

struct rename_direntry{
	uint64_t transaction_id;
	unsigned dir_inode_id;
	unsigned target_inode_id;
	char old_name[NAME_MAX];
	char new_name[NAME_MAX];
};

struct remove_direntry{
	uint64_t transaction_id;
	unsigned dir_inode_id;
	unsigned target_inode_id;
	unsigned old_link_count;
	unsigned new_link_count;
	char name[NAME_MAX];
};

struct alloc_inode{
	uint64_t transaction_id;
	unsigned inode_id;
	uint32_t type;  /* enum object_type */
};

struct free_inode{
	uint64_t transaction_id;
	unsigned inode_id;
};

int log_buffer_bootstrap(void);
int disk_log_bootstrap(void);
int recover(void);
uint64_t log_write(enum operation op, uint16_t size, void *operation_struct);
int checkpoint(void);

#endif
