/**
 *    Copyright (C) 2018-present MongoDB, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the Server Side Public License, version 1,
 *    as published by MongoDB, Inc.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Server Side Public License for more details.
 *
 *    You should have received a copy of the Server Side Public License
 *    along with this program. If not, see
 *    <http://www.mongodb.com/licensing/server-side-public-license>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the Server Side Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#include "mongo/platform/basic.h"

#include <boost/optional/optional_io.hpp>

#include "mongo/bson/bsonobjbuilder.h"
#include "mongo/db/curop.h"
#include "mongo/db/query/query_test_service_context.h"
#include "mongo/unittest/unittest.h"
#include "mongo/util/tick_source_mock.h"

namespace mongo {
namespace {

TEST(CurOpTest, CopyConstructors) {
    OpDebug::AdditiveMetrics a, b;
    a.keysExamined = 1;
    b.keysExamined = 2;
    a.prepareReadConflicts.store(1);
    b.prepareReadConflicts.store(2);
    // Test copy constructor.
    OpDebug::AdditiveMetrics c = a;
    ASSERT_EQ(a.keysExamined, c.keysExamined);
    ASSERT_EQ(a.prepareReadConflicts.load(), c.prepareReadConflicts.load());
    // Test copy assignment.
    a = b;
    ASSERT_EQ(a.keysExamined, b.keysExamined);
    ASSERT_EQ(a.prepareReadConflicts.load(), b.prepareReadConflicts.load());
}

TEST(CurOpTest, AddingAdditiveMetricsObjectsTogetherShouldAddFieldsTogether) {
    OpDebug::AdditiveMetrics currentAdditiveMetrics = OpDebug::AdditiveMetrics();
    OpDebug::AdditiveMetrics additiveMetricsToAdd = OpDebug::AdditiveMetrics();

    // Initialize field values for both AdditiveMetrics objects.
    currentAdditiveMetrics.keysExamined = 0;
    additiveMetricsToAdd.keysExamined = 2;
    currentAdditiveMetrics.docsExamined = 4;
    additiveMetricsToAdd.docsExamined = 2;
    currentAdditiveMetrics.nMatched = 5;
    additiveMetricsToAdd.nMatched = 5;
    currentAdditiveMetrics.nModified = 3;
    additiveMetricsToAdd.nModified = 1;
    currentAdditiveMetrics.ninserted = 4;
    additiveMetricsToAdd.ninserted = 0;
    currentAdditiveMetrics.ndeleted = 3;
    additiveMetricsToAdd.ndeleted = 2;
    currentAdditiveMetrics.nUpserted = 7;
    additiveMetricsToAdd.nUpserted = 8;
    currentAdditiveMetrics.keysInserted = 6;
    additiveMetricsToAdd.keysInserted = 5;
    currentAdditiveMetrics.keysDeleted = 4;
    additiveMetricsToAdd.keysDeleted = 2;
    currentAdditiveMetrics.prepareReadConflicts.store(1);
    additiveMetricsToAdd.prepareReadConflicts.store(5);
    currentAdditiveMetrics.writeConflicts.store(7);
    additiveMetricsToAdd.writeConflicts.store(0);

    // Save the current AdditiveMetrics object before adding.
    OpDebug::AdditiveMetrics additiveMetricsBeforeAdd;
    additiveMetricsBeforeAdd.add(currentAdditiveMetrics);
    currentAdditiveMetrics.add(additiveMetricsToAdd);

    // The following field values should have changed after adding.
    ASSERT_EQ(*currentAdditiveMetrics.keysExamined,
              *additiveMetricsBeforeAdd.keysExamined + *additiveMetricsToAdd.keysExamined);
    ASSERT_EQ(*currentAdditiveMetrics.docsExamined,
              *additiveMetricsBeforeAdd.docsExamined + *additiveMetricsToAdd.docsExamined);
    ASSERT_EQ(*currentAdditiveMetrics.nMatched,
              *additiveMetricsBeforeAdd.nMatched + *additiveMetricsToAdd.nMatched);
    ASSERT_EQ(*currentAdditiveMetrics.nModified,
              *additiveMetricsBeforeAdd.nModified + *additiveMetricsToAdd.nModified);
    ASSERT_EQ(*currentAdditiveMetrics.ninserted,
              *additiveMetricsBeforeAdd.ninserted + *additiveMetricsToAdd.ninserted);
    ASSERT_EQ(*currentAdditiveMetrics.ndeleted,
              *additiveMetricsBeforeAdd.ndeleted + *additiveMetricsToAdd.ndeleted);
    ASSERT_EQ(*currentAdditiveMetrics.nUpserted,
              *additiveMetricsBeforeAdd.nUpserted + *additiveMetricsToAdd.nUpserted);
    ASSERT_EQ(*currentAdditiveMetrics.keysInserted,
              *additiveMetricsBeforeAdd.keysInserted + *additiveMetricsToAdd.keysInserted);
    ASSERT_EQ(*currentAdditiveMetrics.keysDeleted,
              *additiveMetricsBeforeAdd.keysDeleted + *additiveMetricsToAdd.keysDeleted);
    ASSERT_EQ(currentAdditiveMetrics.prepareReadConflicts.load(),
              additiveMetricsBeforeAdd.prepareReadConflicts.load() +
                  additiveMetricsToAdd.prepareReadConflicts.load());
    ASSERT_EQ(currentAdditiveMetrics.writeConflicts.load(),
              additiveMetricsBeforeAdd.writeConflicts.load() +
                  additiveMetricsToAdd.writeConflicts.load());
}

TEST(CurOpTest, AddingUninitializedAdditiveMetricsFieldsShouldBeTreatedAsZero) {
    OpDebug::AdditiveMetrics currentAdditiveMetrics = OpDebug::AdditiveMetrics();
    OpDebug::AdditiveMetrics additiveMetricsToAdd = OpDebug::AdditiveMetrics();

    // Initialize field values for both AdditiveMetrics objects.
    additiveMetricsToAdd.keysExamined = 5;
    currentAdditiveMetrics.docsExamined = 4;
    currentAdditiveMetrics.nModified = 3;
    additiveMetricsToAdd.ninserted = 0;
    currentAdditiveMetrics.keysInserted = 6;
    additiveMetricsToAdd.keysInserted = 5;
    currentAdditiveMetrics.keysDeleted = 4;
    additiveMetricsToAdd.keysDeleted = 2;
    currentAdditiveMetrics.prepareReadConflicts.store(1);
    additiveMetricsToAdd.prepareReadConflicts.store(5);
    currentAdditiveMetrics.writeConflicts.store(7);
    additiveMetricsToAdd.writeConflicts.store(0);

    // Save the current AdditiveMetrics object before adding.
    OpDebug::AdditiveMetrics additiveMetricsBeforeAdd;
    additiveMetricsBeforeAdd.add(currentAdditiveMetrics);
    currentAdditiveMetrics.add(additiveMetricsToAdd);

    // The 'keysExamined' field for the current AdditiveMetrics object was not initialized, so it
    // should be treated as zero.
    ASSERT_EQ(*currentAdditiveMetrics.keysExamined, *additiveMetricsToAdd.keysExamined);

    // The 'docsExamined' field for the AdditiveMetrics object to add was not initialized, so it
    // should be treated as zero.
    ASSERT_EQ(*currentAdditiveMetrics.docsExamined, *additiveMetricsBeforeAdd.docsExamined);

    // The 'nMatched' field for both the current AdditiveMetrics object and the AdditiveMetrics
    // object to add were not initialized, so nMatched should still be uninitialized after the add.
    ASSERT_EQ(currentAdditiveMetrics.nMatched, boost::none);

    // The 'nUpserted' field for both the current AdditiveMetrics object and the AdditiveMetrics
    // object to add were not initialized, so nUpserted should still be uninitialized after the add.
    ASSERT_EQ(currentAdditiveMetrics.nUpserted, boost::none);

    // The following field values should have changed after adding.
    ASSERT_EQ(*currentAdditiveMetrics.keysInserted,
              *additiveMetricsBeforeAdd.keysInserted + *additiveMetricsToAdd.keysInserted);
    ASSERT_EQ(*currentAdditiveMetrics.keysDeleted,
              *additiveMetricsBeforeAdd.keysDeleted + *additiveMetricsToAdd.keysDeleted);
    ASSERT_EQ(currentAdditiveMetrics.prepareReadConflicts.load(),
              additiveMetricsBeforeAdd.prepareReadConflicts.load() +
                  additiveMetricsToAdd.prepareReadConflicts.load());
    ASSERT_EQ(currentAdditiveMetrics.writeConflicts.load(),
              additiveMetricsBeforeAdd.writeConflicts.load() +
                  additiveMetricsToAdd.writeConflicts.load());
}

TEST(CurOpTest, AdditiveMetricsFieldsShouldIncrementByN) {
    OpDebug::AdditiveMetrics additiveMetrics = OpDebug::AdditiveMetrics();

    // Initialize field values.
    additiveMetrics.writeConflicts.store(1);
    additiveMetrics.keysInserted = 2;
    additiveMetrics.prepareReadConflicts.store(6);

    // Increment the fields.
    additiveMetrics.incrementWriteConflicts(1);
    additiveMetrics.incrementKeysInserted(5);
    additiveMetrics.incrementKeysDeleted(0);
    additiveMetrics.incrementNinserted(3);
    additiveMetrics.incrementNUpserted(6);
    additiveMetrics.incrementPrepareReadConflicts(2);

    ASSERT_EQ(additiveMetrics.writeConflicts.load(), 2);
    ASSERT_EQ(*additiveMetrics.keysInserted, 7);
    ASSERT_EQ(*additiveMetrics.keysDeleted, 0);
    ASSERT_EQ(*additiveMetrics.ninserted, 3);
    ASSERT_EQ(*additiveMetrics.nUpserted, 6);
    ASSERT_EQ(additiveMetrics.prepareReadConflicts.load(), 8);
}

TEST(CurOpTest, OptionalAdditiveMetricsNotDisplayedIfUninitialized) {
    // 'basicFields' should always be present in the logs and profiler, for any operation.
    std::vector<std::string> basicFields{
        "op", "ns", "command", "numYield", "locks", "millis", "flowControl"};

    QueryTestServiceContext serviceContext;
    auto opCtx = serviceContext.makeOperationContext();
    SingleThreadedLockStats ls;

    auto curop = CurOp::get(*opCtx);
    const OpDebug& od = curop->debug();

    // Create dummy command.
    BSONObj command = BSON("a" << 3);

    // Set dummy 'ns' and 'command'.
    curop->setGenericOpRequestDetails(
        opCtx.get(), NamespaceString("myDb.coll"), nullptr, command, NetworkOp::dbQuery);

    BSONObjBuilder builder;
    od.append(opCtx.get(), ls, {}, builder);
    auto bs = builder.done();

    // Append should always include these basic fields.
    for (const std::string& field : basicFields) {
        ASSERT_TRUE(bs.hasField(field));
    }

    // Append should include only the basic fields when just initialized.
    ASSERT_EQ(static_cast<size_t>(bs.nFields()), basicFields.size());
}

TEST(CurOpTest, ShouldNotReportFailpointMsgIfNotSet) {
    QueryTestServiceContext serviceContext;
    auto opCtx = serviceContext.makeOperationContext();

    auto curop = CurOp::get(*opCtx);

    // Test the reported state should _not_ contain 'failpointMsg'.
    BSONObjBuilder reportedStateWithoutFailpointMsg;
    curop->reportState(opCtx.get(), &reportedStateWithoutFailpointMsg);
    auto bsonObj = reportedStateWithoutFailpointMsg.done();

    // bsonObj should _not_ contain 'failpointMsg' if a fail point is not set.
    ASSERT_FALSE(bsonObj.hasField("failpointMsg"));
}

TEST(CurOpTest, ElapsedTimeReflectsTickSource) {
    QueryTestServiceContext serviceContext;

    auto tickSourceMock = std::make_unique<TickSourceMock<Microseconds>>();
    // The tick source is initialized to a non-zero value as CurOp equates a value of 0 with a
    // not-started timer.
    tickSourceMock->advance(Milliseconds{100});

    auto opCtx = serviceContext.makeOperationContext();
    auto curop = CurOp::get(*opCtx);
    curop->setTickSource_forTest(tickSourceMock.get());

    ASSERT_FALSE(curop->isStarted());

    curop->ensureStarted();
    ASSERT_TRUE(curop->isStarted());

    tickSourceMock->advance(Milliseconds{20});

    ASSERT_FALSE(curop->isDone());

    curop->done();
    ASSERT_TRUE(curop->isDone());

    ASSERT_EQ(Milliseconds{20}, duration_cast<Milliseconds>(curop->elapsedTimeTotal()));
}

}  // namespace
}  // namespace mongo
