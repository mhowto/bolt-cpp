#ifndef __BOLT_EXCEPTION_H
#define __BOLT_EXCEPTION_H

#include <stdexcept>

// These erros can be returned when opening or calling methods on DB.
struct DatabaseNotOpenException : public std::runtime_error {
  DatabaseNotOpenException() : std::runtime_error("database not open") {}
};

struct DatabaseOpenException : public std::runtime_error {
  DatabaseOpenException() : std::runtime_error("database already open") {}
};

struct DatabaseInvalidException : public std::runtime_error {
  DatabaseInvalidException() : std::runtime_error("invalid database") {}
};

struct DatabaseVersionMismatchException : public std::runtime_error {
  DatabaseVersionMismatchException() : std::runtime_error("version mismatch") {}
};

struct DatabaseChecksumException : public std::runtime_error {
  DatabaseChecksumException() : std::runtime_error("checksum error") {}
};

struct TimeoutException : public std::runtime_error {
  TimeoutException() : std::runtime_error("timeout") {}
};

// These errors can occur when beginning or committing a Tx.
struct TxNotWritableException : public std::runtime_error {
  TxNotWritableException() : std::runtime_error("tx not writable") {}
};

struct TxClosedException : public std::runtime_error {
  TxClosedException() : std::runtime_error("tx closed") {}
};

struct DatabaseReadOnlyException : public std::runtime_error {
  DatabaseReadOnlyException()
      : std::runtime_error("database is in read-only mode") {}
};

// These errors can occur when putting or deleting a value or a bucket.
#endif