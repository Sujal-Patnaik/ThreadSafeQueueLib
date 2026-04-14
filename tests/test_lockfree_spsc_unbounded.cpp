#include <gtest/gtest.h>
#include <thread>
#include <string>
#include <memory>

#include <lockfree_spsc_unbounded/queue.hpp> 

using namespace tsfqueue::impl;


class SPSCTest : public ::testing::Test {
protected:
    lockfree_spsc_unbounded<int> q;
    
    
    void SetUp() override {
     
    }
    void TearDown() override {
       
    }
};

//Basic Tests

// checking if it is empty initially
TEST_F(SPSCTest, Is_Empty_Initially) {
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

// checking if push and pop works
TEST_F(SPSCTest, Push_Pop_Works) {
    q.push(100);
    EXPECT_FALSE(q.empty());
    
    int result = 0;
    EXPECT_TRUE(q.try_pop(result));
    EXPECT_EQ(result, 100);
}

// checking if the queue maintains the FIFO order
TEST_F(SPSCTest, Maintains_Order) {
    q.push(1);
    q.push(2);
    q.push(3);
    
    int result = 0;
    q.try_pop(result); EXPECT_EQ(result, 1);
    q.try_pop(result); EXPECT_EQ(result, 2);
    q.try_pop(result); EXPECT_EQ(result, 3);
}

// checking if peek works
TEST_F(SPSCTest, Peek_Works_Without_Removing) {
    q.push(42);
    
    int peek_val = 0;
    EXPECT_TRUE(q.peek(peek_val));
    EXPECT_EQ(peek_val, 42);
    
    
    EXPECT_EQ(q.size(), 1);
}

