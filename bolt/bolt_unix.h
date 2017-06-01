#ifndef __BOLT_BOLT_UNIX_H
#define __BOLT_BOLT_UNIX_H

class DB;

// mmap memory maps a DB's data file.
void mmap(DB* db, int sz);

// munmap unmaps a DB's data file from memory.
void munmap(DB* db);

#endif