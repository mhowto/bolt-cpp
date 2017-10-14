#include "bolt/exception.h"
#include "bolt/tx.h"
#include "util.h"
#include <gtest/gtest.h>

TEST(TxTest, Commit_ErrTxClosed) {
  DB *db = must_open_db();
  Tx *tx = db->begin(true);

  ASSERT_NO_THROW(tx->create_bucket("foo"));

  ASSERT_NO_THROW(tx->commit());

  ASSERT_THROW(tx->commit(), TxClosedException);
}

TEST(TxTest, Rollback_ErrTxClosed) {}