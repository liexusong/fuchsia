// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use serde::{Serialize, Serializer};
use serde_derive::{Deserialize, Serialize};
use std::collections::HashMap;

/// Result from each test step. Usually each step denotes a separate test case
/// but it depends on what `fuchsia.test.Suite#GetTests()` returns.
#[derive(Default, Serialize, Deserialize)]
pub struct StepResult {
    /// Name of the step.
    pub name: String,
    /// Status of running step (passed, failed, inconclusive).
    pub status: String,
    /// Time taken by test to run.
    pub duration_ms: i64,
    /// Path to log file.
    pub primary_log_path: String,
    /// Artifacts generated by a test step.
    pub artifacts: HashMap<String, String>,
}

#[derive(Deserialize)]
pub enum StepResultItem {
    /// Reference of a single step.
    Ref(String),
    /// Step Result
    Result(StepResult),
}

impl Serialize for StepResultItem {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        match &self {
            StepResultItem::Ref(s) => serializer.serialize_str(s),
            StepResultItem::Result(r) => r.serialize(serializer),
        }
    }
}

/// Result of a single test.
#[derive(Default, Serialize, Deserialize)]
pub struct TestResult {
    /// Result of running this test ("passed" if every step passes else "failed").
    pub result: String,
    /// Total time taken to run this test.
    pub duration_ms: i64,
    /// Log path which contains all logs for this test.
    pub primary_log_path: String,
    /// Series of steps for this test.
    pub steps: Vec<StepResultItem>,
}

#[derive(Serialize, Deserialize)]
pub enum TestResultItem {
    /// Reference of a single test.
    Ref(String),
    /// TestResult
    Result(TestResult),
}

/// Vector of results of all the tests.
#[derive(Default, Serialize, Deserialize)]
pub struct TestResults {
    /// Vector containing all test result items.
    pub results: Vec<TestResultItem>,
}

// Defines one test which client can run.
#[derive(Serialize, Deserialize)]
pub enum TestPlanTest {
    // Test Component Url.
    ComponentUrl(String),
}

/// Test plan to run a series of tests.
#[derive(Default, Serialize, Deserialize)]
pub struct TestPlan {
    /// Vector containing all tests.
    pub tests: Vec<TestPlanTest>,
}
