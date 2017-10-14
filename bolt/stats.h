#ifndef __BOLT_STATS_H
#define __BOLT_STATS_H

#include <chrono>

// TxStats reprents statistics about the actions performed by the transaction.
struct TxStats {
  // Page statistics.
  int page_count; // number of page allocations
  int page_alloc; // total bytes allcoated

  // Cursor statistics.
  int cursor_count; // number of cursors created

  // Node statistics.
  int node_count; // number of node allocations
  int node_deref; // number of node dereferences

  // Rebalance statistics.
  int rebalance;                            // number of node rebalances
  std::chrono::milliseconds rebalance_time; // total time spent rebalancing

  // Split/Spill statistics.
  int split;                            // number of nodes split
  int spill;                            // number of nodes spilled
  std::chrono::milliseconds spill_time; // total time spent spilling

  // Write statistics.
  int write;                            // number of writes performed
  std::chrono::milliseconds write_time; // total time spent writing to disk

  TxStats &operator+=(const TxStats &rhs) {
    this->page_count += rhs.page_count;
    this->page_alloc += rhs.page_alloc;
    this->cursor_count += rhs.cursor_count;
    this->node_count += rhs.node_count;
    this->node_deref += rhs.node_deref;
    this->rebalance += rhs.rebalance;
    this->rebalance_time += rhs.rebalance_time;
    this->split += rhs.split;
    this->spill += rhs.spill;
    this->spill_time += rhs.spill_time;
    this->write += rhs.write;
    this->write_time += rhs.write_time;
    return *this;
  }

  friend TxStats operator+(TxStats lhs, const TxStats &rhs) {
    lhs += rhs;
    return lhs;
  }

  TxStats &operator-=(const TxStats &rhs) {
    this->page_count -= rhs.page_count;
    this->page_alloc -= rhs.page_alloc;
    this->cursor_count -= rhs.cursor_count;
    this->node_count -= rhs.node_count;
    this->node_deref -= rhs.node_deref;
    this->rebalance -= rhs.rebalance;
    this->rebalance_time -= rhs.rebalance_time;
    this->split -= rhs.split;
    this->spill -= rhs.spill;
    this->spill_time -= rhs.spill_time;
    this->write -= rhs.write;
    this->write_time -= rhs.write_time;
    return *this;
  }

  friend TxStats operator-(TxStats lhs, const TxStats &rhs) {
    lhs -= rhs;
    return lhs;
  }
};

// Stats represents statistics about the database.
struct Stats {
  // Freelist stats
  int free_page_n;    // total number of free pages on the freelist
  int pending_page_n; // toal number of pending pages on the freelist
  int free_alloc;     // total bytes allocated in free pages
  int freelist_inuse; // total bytes used by the freelist

  // transaction stats
  int tx_n;      // total number of started read transactions
  int open_tx_n; // number of currently open read transactions

  struct TxStats tx_stats; // global, ongoing stats.

  Stats &operator-=(const Stats &rhs) {
    this->tx_n -= rhs.tx_n;
    this->tx_stats -= rhs.tx_stats;
    return *this;
  }

  friend Stats operator-(Stats lhs, const Stats &rhs) {
    lhs -= rhs;
    return lhs;
  }

  Stats &operator+=(const Stats &rhs) {
    this->tx_stats += rhs.tx_stats;
    return *this;
  }

  friend Stats operator+(Stats lhs, const Stats &rhs) {
    lhs += rhs;
    return lhs;
  }
};

#endif