#include "bolt/db.h"
#include "util.h"
#include <gtest/gtest.h>

/*
TEST(DBTest, Begin_DatabaseNotOpenException) {
  DB db;
  db.begin(false);
}
*/

// Ensure that a read-write transaction can be retrieved.
TEST(DBTest, BeginRW) { DB *db = must_open_db(); }