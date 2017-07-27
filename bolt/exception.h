#ifndef __BOLT_EXCEPTION_H
#define __BOLT_EXCEPTION_H

#include <stdexcept>

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

#endif